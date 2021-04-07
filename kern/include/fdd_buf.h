#ifndef __FDD_BUF_H__
#define __FDD_BUF_H__

enum
{
    FDD_BUF_ALLOC_FLAG_RSVD = 0x1,      // can use rsvd memory
    FDD_BUF_ALLOC_FLAG_WAIT = 0x2,      // can wait until memory is enough
};

enum
{
    FDD_BUF_HEAD_MOD_NORM   = 0x0,
    FDD_BUF_HEAD_MOD_SG,
};

enum
{
    FDD_BUF_FLG_CLONE       = 0x1,      // data is shared with the alloc buffer
    FDD_BUF_FLG_TEMP        = 0x2,      // all part of buffer is temporary memory
    FDD_BUF_FLG_HDR_TEMP    = 0x4,      // just header of buffer is temporary memory
    FDD_BUF_FLG_SHARE_MASK  = 0xF,
    FDD_BUF_FLG_FRAME_START = 0x10,
    FDD_BUF_FLG_FRAME_END   = 0x20,
};

#define FDD_BUF_MIN_RSVD_LEN            (4 * VA_KB)
#define FDD_BUF_MAX_SIZE(pstBufMgr)     ((pstBufMgr)->u32MaxBufSize)

struct tagFddBufMgr;

//fdd buffer scatter gather for frame etc
typedef struct tagFddBufSg
{
    struct list_head stNode;
    struct list_head stBufHead;
    U32  u32TotBytes;
}FDD_BUF_SG_S;

typedef struct tagFddBlkHeader
{
    struct list_head stNode;
    atomic_t stRef;
    U32      u32CurrFreeOff;
    struct tagFddBufMgr *pstBufMgr;
}FDD_BLK_HDR_S;

typedef struct tagFddBuf
{
    struct list_head stNode;
    U32  u32BlkOff;
    U32  u32BufLen;
    U16  u16Flag;
    U16  u16Rsvd;
    char *pData;
}FDD_BUF_S;

typedef struct tagFddBufMgr
{
    struct list_head  stFreeHead;
    struct mutex      stMutex;
    wait_queue_head_t stWaitHead;
    FDD_BLK_HDR_S *pstCurrBlk;
    U32  u32FreeNum;
    U32  u32TotalSize;
    U32  u32BlkSize;
    U32  u32MaxBufSize;
    U32  u32BlkNum;
    U32  u32PageNum;
    struct page **ppstPageTbl;
    char *pMemBuf;
}FDD_BUF_MGR_S;

typedef struct tagFddBufHead
{
    struct list_head stHead;
    spinlock_t       stLock;
    FDD_BUF_MGR_S    *pstBufMgr;
    FDD_BUF_SG_S     *pstSg;
    U8  u8Mode;
    U32 u32QueLen;
    U32 u32Bytes;
    U32 u32LimitLen;
    U32 u32StatCnt;
    atomic_t stDropCnt;
}FDD_BUF_HEAD_S;

static inline void FDD_InitTmpBuf(FDD_BUF_S *pstTmpBuf, VOID *pData, U32 u32Len)
{
    memset(pstTmpBuf, 0, sizeof(FDD_BUF_S));

    INIT_LIST_HEAD(&pstTmpBuf->stNode);

    pstTmpBuf->pData     = pData;
    pstTmpBuf->u16Flag   = FDD_BUF_FLG_TEMP;
    pstTmpBuf->u32BufLen = u32Len;
}

extern int  FDD_InitBufMgr(FDD_BUF_MGR_S *pstBufMgr);
extern void FDD_ExitBufMgr(FDD_BUF_MGR_S *pstBufMgr);
extern FDD_BUF_S *FDD_AllocBuf(FDD_BUF_MGR_S *pstBufMgr, U32 u32Len, U32 u32Flag);
extern FDD_BUF_S *FDD_AllocPrivBuf(FDD_BUF_MGR_S *pstBufMgr, FDD_BUF_S *pstBuf, U32 u32Flag);
extern FDD_BUF_S *FDD_TrimBufLen(FDD_BUF_MGR_S *pstBufMgr, FDD_BUF_S *pstBuf, U32 u32Len);
extern FDD_BUF_S *FDD_CloneBuf(FDD_BUF_S *pstBuf);
extern FDD_BUF_S *FDD_CloneTmpBuf(FDD_BUF_MGR_S *pstBufMgr, FDD_BUF_S *pstBuf, U32 u32AllocFlag);
extern FDD_BUF_S *FDD_GetBuf(FDD_BUF_S *pstBuf);

extern VOID FDD_FreeBuf(FDD_BUF_S *pstBuf);

#endif //__FDD_BUF_H__

