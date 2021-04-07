#include "fdd.h"
#include <linux/vmalloc.h>
#include <linux/mm.h>

/*
 virtual data dispatch buffer management
*/

FDD_BLK_HDR_S *FDD_GetBlkHdr(FDD_BUF_S *pstBuf)
{
    FDD_BLK_HDR_S *pstBlkHdr = VA_PTR_TYPE(FDD_BLK_HDR_S, pstBuf->pData - pstBuf->u32BlkOff);
    return pstBlkHdr;
}

FDD_BLK_HDR_S *__FDD_AllocBlk(FDD_BUF_MGR_S *pstBufMgr, U32 u32Flag)
{
    FDD_BLK_HDR_S *pstBlkHdr;

retry:
    while ( pstBufMgr->u32FreeNum == 0 || (((u32Flag & FDD_BUF_ALLOC_FLAG_RSVD) == 0) && pstBufMgr->u32FreeNum <= 1) )
    {
        if ( (u32Flag & FDD_BUF_ALLOC_FLAG_WAIT) == 0 )
        {
            return NULL;
        }

        // wait for blk
        mutex_unlock(&pstBufMgr->stMutex);
        wait_event(pstBufMgr->stWaitHead, !(pstBufMgr->u32FreeNum == 0 || (((u32Flag & FDD_BUF_ALLOC_FLAG_RSVD) == 0) && pstBufMgr->u32FreeNum <= 1)));
        mutex_lock(&pstBufMgr->stMutex);
        goto retry;
    }

    pstBlkHdr = list_first_entry(&pstBufMgr->stFreeHead, FDD_BLK_HDR_S, stNode);
    list_del_init(&pstBlkHdr->stNode);
    atomic_set(&pstBlkHdr->stRef, 0);
    pstBlkHdr->u32CurrFreeOff = sizeof(FDD_BLK_HDR_S);
    pstBlkHdr->pstBufMgr = pstBufMgr;
    pstBufMgr->u32FreeNum--;

    return pstBlkHdr;
}

FDD_BLK_HDR_S *FDD_AllocBlk(FDD_BUF_MGR_S *pstBufMgr, U32 u32Flag)
{
    FDD_BLK_HDR_S *pstBlkHdr;
    mutex_lock(&pstBufMgr->stMutex);
    pstBlkHdr = __FDD_AllocBlk(pstBufMgr, u32Flag);
    mutex_unlock(&pstBufMgr->stMutex);

    return pstBlkHdr;
}

static VOID FDD_FreeBlk(FDD_BUF_MGR_S *pstBufMgr, FDD_BLK_HDR_S *pstBlkHdr)
{
    mutex_lock(&pstBufMgr->stMutex);
    list_add(&pstBlkHdr->stNode, &pstBufMgr->stFreeHead);
    pstBufMgr->u32FreeNum++;
    mutex_unlock(&pstBufMgr->stMutex);
    wake_up_all(&pstBufMgr->stWaitHead);
}

static VOID __FDD_FreeBlk(FDD_BUF_MGR_S *pstBufMgr, FDD_BLK_HDR_S *pstBlkHdr)
{
    list_add(&pstBlkHdr->stNode, &pstBufMgr->stFreeHead);
    pstBufMgr->u32FreeNum++;
    wake_up_all(&pstBufMgr->stWaitHead);
}

static FDD_BLK_HDR_S *FDD_GetBlk(FDD_BLK_HDR_S *pstBlkHdr)
{
    atomic_inc(&pstBlkHdr->stRef);
    return pstBlkHdr;
}

static VOID FDD_PutBlk(FDD_BUF_MGR_S *pstBufMgr, FDD_BLK_HDR_S *pstBlkHdr)
{
    if ( atomic_dec_and_test(&pstBlkHdr->stRef) )
    {
        FDD_FreeBlk(pstBufMgr, pstBlkHdr);
    }
}

static VOID __FDD_PutBlk(FDD_BUF_MGR_S *pstBufMgr, FDD_BLK_HDR_S *pstBlkHdr)
{
    if ( atomic_dec_and_test(&pstBlkHdr->stRef) )
    {
        __FDD_FreeBlk(pstBufMgr, pstBlkHdr);
    }
}

static VOID FDD_InitBuf(FDD_BLK_HDR_S *pstBlkHdr, FDD_BUF_S *pstBuf, U32 u32Len)
{
    INIT_LIST_HEAD(&pstBuf->stNode);

    pstBuf->pData     = VA_PTR_TYPE(char, pstBuf + 1);
    pstBuf->u32BlkOff = pstBuf->pData - VA_PTR_TYPE(char, pstBlkHdr);
    pstBuf->u32BufLen = u32Len;
    pstBuf->u16Flag   = 0;
    pstBlkHdr->u32CurrFreeOff += sizeof(FDD_BUF_S) + u32Len;
    FDD_GetBlk(pstBlkHdr);
}

FDD_BUF_S *FDD_GetNextBuf(FDD_BUF_MGR_S *pstBufMgr, FDD_BLK_HDR_S *pstBlkHdr, U32 u32Len)
{
    FDD_BUF_S *pstBuf;
    U32 u32LeftLen;

    if ( pstBlkHdr == NULL )
    {
         return NULL;
    }

    u32LeftLen = pstBufMgr->u32BlkSize - pstBlkHdr->u32CurrFreeOff;
    if ( u32LeftLen < u32Len + sizeof(FDD_BUF_S) )
    {
        return NULL;
    }

    pstBuf = VA_PTR_TYPE(FDD_BUF_S, VA_PTR_TYPE(char, pstBlkHdr) + pstBlkHdr->u32CurrFreeOff);
    FDD_InitBuf(pstBlkHdr, pstBuf, u32Len);
    return pstBuf;
}

FDD_BUF_S *FDD_AllocBuf(FDD_BUF_MGR_S *pstBufMgr, U32 u32Len, U32 u32Flag)
{
    FDD_BLK_HDR_S *pstCurrBlk;
    FDD_BUF_S *pstBuf;

    if ( u32Len > FDD_BUF_MAX_SIZE(pstBufMgr) )
    {
        return NULL;
    }

    mutex_lock(&pstBufMgr->stMutex);

    do {
        pstCurrBlk = pstBufMgr->pstCurrBlk;
        pstBuf = FDD_GetNextBuf(pstBufMgr, pstCurrBlk, u32Len);
        if ( NULL != pstBuf )
        {
            mutex_unlock(&pstBufMgr->stMutex);
            return pstBuf;
        }

        if ( pstCurrBlk )
        {
            __FDD_PutBlk(pstBufMgr, pstCurrBlk);
            pstBufMgr->pstCurrBlk = NULL;
        }

        pstCurrBlk = __FDD_AllocBlk(pstBufMgr, u32Flag);
        if ( pstCurrBlk == NULL )
        {
            mutex_unlock(&pstBufMgr->stMutex);
            return NULL;
        }

        if ( pstBufMgr->pstCurrBlk != NULL )  // recheck the blk, alloc block buffer may be sleep!
        {
            __FDD_FreeBlk(pstBufMgr, pstCurrBlk); //another has alloced it, so free itself and retry to alloc
        }
        else
        {
            pstBufMgr->pstCurrBlk = FDD_GetBlk(pstCurrBlk);
        }
    }while (1);

    mutex_unlock(&pstBufMgr->stMutex);
}

FDD_BUF_S *FDD_AllocPrivBuf(FDD_BUF_MGR_S *pstBufMgr, FDD_BUF_S *pstBuf, U32 u32Flag)
{
    FDD_BLK_HDR_S *pstBlkHdr;

    if ( pstBuf )
    {
        return pstBuf;
    }

    pstBlkHdr = FDD_AllocBlk(pstBufMgr, u32Flag);
    if ( pstBlkHdr == NULL )
    {
        return NULL;
    }

    pstBuf = FDD_GetNextBuf(pstBufMgr, pstBlkHdr, pstBufMgr->u32MaxBufSize - sizeof(FDD_BUF_S));
    return pstBuf;
}

FDD_BUF_S *FDD_TrimBufLen(FDD_BUF_MGR_S *pstBufMgr, FDD_BUF_S *pstBuf, U32 u32Len)
{
    FDD_BLK_HDR_S *pstBlkHdr = FDD_GetBlkHdr(pstBuf);
    FDD_BUF_S *pstNextBuf;
    U32 u32LeftLen;

    pstBuf->u32BufLen = u32Len;
    pstBlkHdr->u32CurrFreeOff = (pstBuf->pData - (char *)pstBlkHdr) + pstBuf->u32BufLen;
    u32LeftLen = pstBufMgr->u32BlkSize - pstBlkHdr->u32CurrFreeOff;

    if ( u32LeftLen < FDD_BUF_MIN_RSVD_LEN )
    {
        return NULL;
    }

    pstNextBuf = VA_PTR_TYPE(FDD_BUF_S, VA_PTR_TYPE(char, pstBlkHdr) + pstBlkHdr->u32CurrFreeOff);
    FDD_InitBuf(pstBlkHdr, pstNextBuf, u32LeftLen - sizeof(FDD_BUF_S));

    return pstNextBuf;
}

FDD_BUF_S *FDD_CloneBuf(FDD_BUF_S *pstBuf)
{
    FDD_BLK_HDR_S *pstBlkHdr;
    FDD_BUF_S     *pstCloneBuf;

    if (pstBuf->u16Flag & FDD_BUF_FLG_TEMP) // temp buffer can't clone!
    {
        return NULL;
    }

    pstBlkHdr   = FDD_GetBlkHdr(pstBuf);
    pstCloneBuf = VA_Malloc(sizeof(FDD_BUF_S));
    if ( pstCloneBuf == NULL )
    {
        return NULL;
    }

    memcpy(pstCloneBuf, pstBuf, sizeof(FDD_BUF_S));
    pstCloneBuf->u16Flag |= FDD_BUF_FLG_CLONE;
    INIT_LIST_HEAD(&pstCloneBuf->stNode);
    FDD_GetBlk(pstBlkHdr);

    return pstCloneBuf;
}

FDD_BUF_S *FDD_CloneTmpBuf(FDD_BUF_MGR_S *pstBufMgr, FDD_BUF_S *pstBuf, U32 u32AllocFlag)
{
    FDD_BUF_S *pstCloneBuf;

    pstCloneBuf = FDD_AllocBuf(pstBufMgr, pstBuf->u32BufLen, u32AllocFlag);
    if ( pstCloneBuf )
    {
        pstBuf->pData     = pstCloneBuf->pData;
        pstBuf->u32BlkOff = pstCloneBuf->u32BlkOff;
        pstBuf->u16Flag  &= FDD_BUF_FLG_TEMP;
        pstBuf->u16Flag  |= FDD_BUF_FLG_HDR_TEMP;
        pstCloneBuf->u16Flag = (pstBuf->u16Flag & (~0xF));

        FDD_GetBlk(FDD_GetBlkHdr(pstBuf));
    }

    return pstCloneBuf;
}

FDD_BUF_S *FDD_GetBuf(FDD_BUF_S *pstBuf)
{
    FDD_BLK_HDR_S *pstBlkHdr;

    if (pstBuf->u16Flag & FDD_BUF_FLG_TEMP)
    {
        return NULL;
    }

    pstBlkHdr = FDD_GetBlkHdr(pstBuf);
    FDD_GetBlk(pstBlkHdr);

    return pstBuf;
}

VOID FDD_FreeBuf(FDD_BUF_S *pstBuf)
{
    FDD_BLK_HDR_S *pstBlkHdr;

    if (pstBuf->u16Flag & FDD_BUF_FLG_TEMP)
    {
        return;
    }

    pstBlkHdr = FDD_GetBlkHdr(pstBuf);

    // not inline blk memory, it's slab memory
    //if ( ((char *)(pstBuf) < pstBufMgr->pMemBuf) || ((char *)(pstBuf) > pstBufMgr->pMemBuf + pstBufMgr->u32TotalSize) )
    if (pstBuf->u16Flag & FDD_BUF_FLG_CLONE)
    {
        VA_Free(pstBuf);
    }

    FDD_PutBlk(pstBlkHdr->pstBufMgr, pstBlkHdr);
    return;
}

static void FDD_InitBufBlk(FDD_BUF_MGR_S *pstBufMgr)
{
    FDD_BLK_HDR_S *pstBlkHdr;
    char *pStart = pstBufMgr->pMemBuf;
    U32 i;

    for (i = 0; i < pstBufMgr->u32BlkNum; i++)
    {
        pstBlkHdr = VA_PTR_TYPE(FDD_BLK_HDR_S, pStart + pstBufMgr->u32BlkSize * i);
        list_add_tail(&pstBlkHdr->stNode, &pstBufMgr->stFreeHead);
    }

    pstBufMgr->u32FreeNum = pstBufMgr->u32BlkNum;
    return;
}

static struct page **FDD_AllocPageArray(U32 u32PageNum)
{
    struct page **ppstPageTbl;

    size_t u32Size = u32PageNum * sizeof(struct page *);

    ppstPageTbl = VA_VMalloc(u32Size);
    if ( ppstPageTbl )
    {
        memset(ppstPageTbl, 0, u32Size);
    }

    return ppstPageTbl;
}

static void FDD_ReleaseBufMem(FDD_BUF_MGR_S *pstBufMgr)
{
    U32 i;

    if ( pstBufMgr->ppstPageTbl == NULL )
    {
        return;
    }

    if ( pstBufMgr->pMemBuf )
    {
        vunmap(pstBufMgr->pMemBuf);
        pstBufMgr->pMemBuf = NULL;
    }

    for (i = 0; i < pstBufMgr->u32PageNum; i++)
    {
        if ( pstBufMgr->ppstPageTbl[i] )
        {
            __free_page(pstBufMgr->ppstPageTbl[i]);
        }
    }

    VA_VFree(pstBufMgr->ppstPageTbl);
    pstBufMgr->ppstPageTbl = NULL;
    return;
}

static int FDD_AllocBufMem(FDD_BUF_MGR_S *pstBufMgr, U32 u32BufSize)
{
    U32 u32PageNum;
    U32 i;

    u32BufSize = VA_ROUNDUP(u32BufSize, PAGE_SIZE);
    u32PageNum = u32BufSize >> PAGE_SHIFT;
    pstBufMgr->ppstPageTbl = FDD_AllocPageArray(u32PageNum);
    if ( pstBufMgr->ppstPageTbl == NULL )
    {
        return -ENOMEM;
    }

    pstBufMgr->u32PageNum = u32PageNum;

    for (i = 0; i < u32PageNum; i++)
    {
        pstBufMgr->ppstPageTbl[i] = alloc_page(GFP_KERNEL);
        if (unlikely(!pstBufMgr->ppstPageTbl[i]))
        {
            FDD_ReleaseBufMem(pstBufMgr);
            return -ENOMEM;
        }

        set_page_private(pstBufMgr->ppstPageTbl[i], (unsigned long)pstBufMgr);
    }

    pstBufMgr->pMemBuf = vmap(pstBufMgr->ppstPageTbl, u32PageNum, VM_MAP, PAGE_KERNEL);
    if ( pstBufMgr->pMemBuf == NULL )
    {
        FDD_ReleaseBufMem(pstBufMgr);
        return -ENOMEM;
    }

    return 0;
}

int FDD_InitBufMgr(FDD_BUF_MGR_S *pstBufMgr)
{
    int iRet;

    if ( pstBufMgr->u32TotalSize == 0 )
    {
        VA_LOG_ERR("fdd buf total size is zero, need to cfg it");
        return -ENOMEM;
    }

    INIT_LIST_HEAD(&pstBufMgr->stFreeHead);
    mutex_init(&pstBufMgr->stMutex);
    init_waitqueue_head(&pstBufMgr->stWaitHead);

    pstBufMgr->u32BlkNum     = pstBufMgr->u32TotalSize / pstBufMgr->u32BlkSize;
    pstBufMgr->u32TotalSize  = pstBufMgr->u32BlkSize * pstBufMgr->u32BlkNum;
    pstBufMgr->u32MaxBufSize = pstBufMgr->u32BlkSize - sizeof(FDD_BLK_HDR_S) - sizeof(FDD_BUF_S);

    iRet = FDD_AllocBufMem(pstBufMgr, pstBufMgr->u32TotalSize);
    if ( iRet < 0 )
    {
        return iRet;
    }

    FDD_InitBufBlk(pstBufMgr);
    return 0;
}

void FDD_ExitBufMgr(FDD_BUF_MGR_S *pstBufMgr)
{
    FDD_ReleaseBufMem(pstBufMgr);
}

