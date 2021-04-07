#include "va_kern_pub.h"
#include "va_util.h"

void VA_InitHlistTbl(struct hlist_head *pstHlistTbl, U32 u32Cnt)
{
    U32 i;

    for ( i = 0; i < u32Cnt; i++ )
    {
        INIT_HLIST_HEAD(pstHlistTbl + i);
    }
}

void VA_InitListTbl(struct list_head *pstListTbl, U32 u32Cnt)
{
    U32 i;

    for ( i = 0; i < u32Cnt; i++ )
    {
        INIT_LIST_HEAD(pstListTbl + i);
    }
}

void VA_ReEnterMutexInit(VA_REENTER_MUTEX_S *pstMutex)
{
    mutex_init(&pstMutex->stLock);
    pstMutex->pstOwnTsk = NULL;
    pstMutex->u32Ref    = 0;

    return;
}

void VA_ReEnterMutexLock(VA_REENTER_MUTEX_S *pstMutex)
{
    if ( current != pstMutex->pstOwnTsk )
    {
        mutex_lock(&pstMutex->stLock);
        pstMutex->pstOwnTsk = current;
    }

    ++pstMutex->u32Ref;
    return;
}

void VA_ReEnterMutexUnLock(VA_REENTER_MUTEX_S *pstMutex)
{
    if ( --pstMutex->u32Ref == 0 )
    {
        /* 如果引用计数为0表示本线程不再包含互斥资源 */
        pstMutex->pstOwnTsk = NULL;
        mutex_unlock(&pstMutex->stLock);
    }

    return;
}

struct page **VA_AllocPageArray(U32 u32PageNum)
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

void VA_ReleaseMapBufMem(VA_MAP_BUF_S *pstMapBuf)
{
    U32 i;

    if ( pstMapBuf->ppstPageTbl == NULL )
    {
        return;
    }

    if ( pstMapBuf->pMemBuf )
    {
        vunmap(pstMapBuf->pMemBuf);
        pstMapBuf->pMemBuf = NULL;
    }

    for (i = 0; i < pstMapBuf->u32PageNum; i++)
    {
        if ( pstMapBuf->ppstPageTbl[i] )
        {
            __free_page(pstMapBuf->ppstPageTbl[i]);
        }
    }

    VA_VFree(pstMapBuf->ppstPageTbl);
    pstMapBuf->ppstPageTbl = NULL;
    return;
}


int VA_AllocDoubleMapBuf(VA_MAP_BUF_S *pstMapBuf, U32 u32BufSize)
{
    U32 u32PageNum;
    U32 i;

    VA_CB_ZERO(pstMapBuf);

    u32BufSize = VA_ROUNDUP(u32BufSize, PAGE_SIZE);
    u32PageNum = u32BufSize >> PAGE_SHIFT;

    pstMapBuf->ppstPageTbl = VA_AllocPageArray(u32PageNum * 2);
    if ( pstMapBuf->ppstPageTbl == NULL )
    {
        return -ENOMEM;
    }

    pstMapBuf->u32PageNum = u32PageNum;

    for (i = 0; i < u32PageNum; i++)
    {
        pstMapBuf->ppstPageTbl[i] = alloc_page(GFP_KERNEL);
        if (unlikely(!pstMapBuf->ppstPageTbl[i]))
        {
            VA_ReleaseMapBufMem(pstMapBuf);
            return -ENOMEM;
        }

        set_page_private(pstMapBuf->ppstPageTbl[i], (unsigned long)pstMapBuf);
    }

    for (i = 0; i < u32PageNum; i++) // double page!
    {
        pstMapBuf->ppstPageTbl[i + u32PageNum] = pstMapBuf->ppstPageTbl[i];
    }

    pstMapBuf->pMemBuf = vmap(pstMapBuf->ppstPageTbl, u32PageNum * 2, VM_MAP, PAGE_KERNEL);
    if ( pstMapBuf->pMemBuf == NULL )
    {
        VA_ReleaseMapBufMem(pstMapBuf);
        return -ENOMEM;
    }

    return 0;
}

