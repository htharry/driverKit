#ifndef __FDD_H__
#define __FDD_H__

#include "va_kern_pub.h"
#include "va_media_def.h"
#include "fdd_buf.h"
#include "fdd_port.h"
#include <linux/workqueue.h>

#define FDD_LISTEN_HASH_BITS            5
#define FDD_LISTEN_HASH_SIZE            (0x1 << FDD_LISTEN_HASH_BITS)
#define FDD_CHAN_HASH_BITS              5
#define FDD_CHAN_HASH_SIZE              (0x1 << FDD_CHAN_HASH_BITS)

enum
{
    FDD_PORT_CLONE_NONE,
    FDD_PORT_CLONE_ORIG,
    FDD_PORT_CLONE_SLAVE,
};

struct tagFddChanOps;

typedef struct tagFddChan
{
    CHAN_ID_S           stChanId;
    FDD_BUF_HEAD_S      stRxQueue;
    FDD_LISTEN_PORT_S   *pstListenPort;
    FDD_BUF_MGR_S       *pstBufMgr;
    atomic_t            stRef;
    U16                 u16DataType;
    U32                 u32Debug;
    struct hlist_node   stChanHnode;
    struct list_head    stPortHead;     // rx switch port list, as this chan is rx port, to add this list to link each other
    const struct tagFddChanOps *pstOps;
}FDD_CHAN_S;

typedef struct tagFddChanOps
{
    int  (*pfnInit)(FDD_CHAN_S *pstFddChan);
    VOID (*pfnRelease)(FDD_CHAN_S *pstFddChan);
    U32  (*pfnDataReady)(FDD_CHAN_S *pstFddChan);
    VOID (*pfnInitPort)(FDD_CHAN_S *pstFddChan, FDD_PORT_S *pstPort);
    LONG (*pfnInitListenPort)(FDD_CHAN_S *pstFddChan, FDD_PORT_S *pstPort);
    LONG (*pfnReleasePort)(FDD_CHAN_S *pstFddChan, FDD_PORT_S *pstPort);
    VOID (*pfnMtDispChan)(FDD_CHAN_S *pstFddChan, ULONG ulExecId);
}FDD_CHAN_OPS_S;

typedef struct tagFddCb
{
    FDD_BUF_MGR_S     stBufMgr;
    struct mutex      stMutex;
    struct hlist_head astListenHashTbl[FDD_LISTEN_HASH_SIZE];
    struct hlist_head astChanHashTbl[FDD_CHAN_HASH_SIZE];
    struct workqueue_struct *pstWorkQueue;
}FDD_CB_S;

extern VOID FDD_MT_DispChan(FDD_CHAN_S *pstChan, ULONG ulExecId);
extern FDD_CB_S *GetFddCb(void);
extern VOID FDD_ClearBufHead(FDD_BUF_HEAD_S *pstBufHead);
extern VOID FDD_InitBufHead(FDD_BUF_HEAD_S *pstBufHead, FDD_BUF_MGR_S *pstBufMgr);
extern VOID FDD_FreeSg(FDD_BUF_SG_S *pstSg);
extern FDD_BUF_SG_S *FDD_SgDequeue(FDD_BUF_HEAD_S *pstBufHead);
extern int  FDD_QueueBufTail(FDD_BUF_HEAD_S *pstBufHead, FDD_BUF_S *pstBuf);
extern int  FDD_QueueBufSg(FDD_BUF_HEAD_S *pstBufHead, FDD_BUF_S *pstBuf, bool bEnd);
extern U32  FDD_GetBufQueueLen(FDD_BUF_HEAD_S *pstBufHead);
extern FDD_CHAN_S *FDD_GetChan(FDD_CHAN_S *pstChan);
extern VOID FDD_PutChan(FDD_CHAN_S *pstChan);
extern int  FDD_InitListenChan(FDD_CHAN_S *pstChan, U64 u64ListenId);
extern VOID FDD_SetChanRxLimitLen(FDD_CHAN_S *pstChan, U32 u32LimitLen);
extern VOID FDD_SetChanRxMode(FDD_CHAN_S *pstChan, U8 u8Mode);
extern int  FDD_InitChan(FDD_CHAN_S *pstChan, CHAN_ID_S *pstChanId, const FDD_CHAN_OPS_S *pstOps);
extern VOID FDD_ReleaseChan(FDD_CHAN_S *pstChan);
extern LONG FDD_ConnectChan(FDD_CHAN_S *pstListenChan, FDD_CHAN_S *pstRxChan);
extern LONG FDD_ConnectChanById(CHAN_ID_S *pstListenChanId, CHAN_ID_S *pstRxChanId);
extern LONG __FDD_DisconnectChan(FDD_CHAN_S *pstListenChan, FDD_CHAN_S *pstRxChan);
extern LONG FDD_DisconnectChan(FDD_CHAN_S *pstListenChan, FDD_CHAN_S *pstRxChan);
extern LONG FDD_DisconnectChanById(CHAN_ID_S *pstListenChanId, CHAN_ID_S *pstRxChanId);
extern BOOL FDD_QueueWork(struct work_struct *pstWork);
extern BOOL FDD_QueueDwork(struct delayed_work *pstDwork, ULONG ulDelay);
extern BOOL FDD_CancelWork(struct work_struct *pstWork);
extern BOOL FDD_CancelDwork(struct delayed_work *pstDwork);
#endif //__FDD_H__

