#ifndef __FDD_PORT_H__
#define __FDD_PORT_H__

struct tagFddPortOps;
struct tagFddChan;

typedef struct tagFddPort
{
    struct list_head stFwdNode;
    struct list_head stListenNode;
    struct list_head stPairNode;
    const struct tagFddPortOps *pstOps;
    struct tagFddChan *pstListenChan;
    union
    {
        struct tagFddChan *pstRxChan;
        VOID *pPrivCb;
    };
    atomic_t stRef;
    struct tagFddVswPort *pstVswPort;
}FDD_PORT_S;

typedef struct tagFddPortOps
{
    VOID (*pfnOutPkt)(struct tagFddPort *pstPort, VOID *pBuf, U32 u32Len);
    VOID (*pfnGetPkt)(struct tagFddPort *pstPort, VOID **ppBuf, U32 *pu32Len);
    VOID (*pfnLastProc)(struct tagFddPort *pstPort, VOID *pBuf, U32 u32Len);
    VOID (*pfnRelease)(struct tagFddPort *pstPort);
}FDD_PORT_OPS_S;

typedef struct tagFddVswOutBuf
{
    FDD_BUF_S        *pstCurrBuf;
    U32              u32CurrLen;
}FDD_VSW_OUT_BUF_S;

typedef struct tagFddVswPort
{
    FDD_PORT_S       stPort;
    struct list_head stFwdHead;
    FDD_BUF_HEAD_S   stTxQueue;
    FDD_BUF_MGR_S    *pstBufMgr;
    VOID             *pVcCb;
    FDD_VSW_OUT_BUF_S astOutBuf[VA_BUF_TYPE_NUM];
    ULONG            ulInCount;
    ULONG            ulOutCount;
    VOID (*pfnConvertProc)(struct tagFddVswPort *pstVswPort, VOID *pBuf, U32 u32Len);
    VOID (*pfnRelease)(struct tagFddVswPort *pstVswPort);
    U16 u16InDataType;
    U16 u16OutDataType;
}FDD_VSW_PORT_S;

typedef struct tagFddListenPort
{
    FDD_VSW_PORT_S    stVswPort;
    struct list_head  stPortHead;
    struct list_head  stVswHead;
    struct mutex      stMutex;
    struct tagFddChan *pstFddChan;
    FDD_VSW_PORT_S    *pstBaseVswPort;
    FDD_BUF_MGR_S     *pstBufMgr;
    struct hlist_node stListenNode;
    U64               u64ListenId;
    atomic_t          stPortCnt;
    VOID (*pfnRelease)(VOID *pPrivCb);
}FDD_LISTEN_PORT_S;

extern VOID FDD_FwdBufData(struct tagFddChan *pstChan, VOID *pBuf, U32 u32Len);
extern VOID FDD_FwdDirectData(struct tagFddChan *pstChan, VOID *pBuf, U32 u32Len);
extern FDD_PORT_S *FDD_AllocPort(struct tagFddChan *pstListenChan, struct tagFddChan *pstRxChan);
extern VOID FDD_PutPort(FDD_PORT_S *pstPort);
extern FDD_VSW_PORT_S *FDD_AllocVswPort(FDD_BUF_MGR_S *pstBufMgr, FDD_LISTEN_PORT_S *pstListenPort, U32 u32Size);
extern FDD_LISTEN_PORT_S *FDD_AllocListenPort(struct tagFddChan *pstFddChan, U64 u64ListenId);
extern VOID FDD_FreeListenPort(FDD_LISTEN_PORT_S *pstListenPort);
extern VOID FDD_DelFrmListenPort(FDD_LISTEN_PORT_S *pstListenPort, FDD_PORT_S *pstRxPort);
extern int FDD_AddToListenPort(FDD_LISTEN_PORT_S *pstListenPort, FDD_PORT_S *pstRxPort);

#endif //__FDD_PORT_H__

