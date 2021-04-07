/*
 fast data dispatch
*/

#include "fdd.h"
#include "fdd_mt.h"
#include "va_util.h"
#include "board.h"

FDD_CB_S gstFddCb;

FDD_LISTEN_PORT_S *FDD_FindListenPort(U64 u64Id);
FDD_CHAN_S *FDD_FindChan(CHAN_ID_S *pstChanId);

FDD_CB_S *GetFddCb(void)
{
    return &gstFddCb;
}

VOID FDD_InitBufHead(FDD_BUF_HEAD_S *pstBufHead, FDD_BUF_MGR_S *pstBufMgr)
{
    VA_CB_ZERO(pstBufHead);
    spin_lock_init(&pstBufHead->stLock);
    INIT_LIST_HEAD(&pstBufHead->stHead);
    pstBufHead->pstBufMgr = pstBufMgr;
}

VOID FDD_SetBufHeadMode(FDD_BUF_HEAD_S *pstBufHead, U8 u8Mode)
{
    pstBufHead->u8Mode = u8Mode;
}

VOID FDD_SetBufLimitLen(FDD_BUF_HEAD_S *pstBufHead, U32 u32LimitLen)
{
    pstBufHead->u32LimitLen = u32LimitLen;
}

VOID FDD_ClearBufList(struct list_head *pstHead)
{
    FDD_BUF_S *pstBuf;

    while (!list_empty(pstHead))
    {
        pstBuf = list_first_entry(pstHead, FDD_BUF_S, stNode);
        list_del(&pstBuf->stNode);
        FDD_FreeBuf(pstBuf);
    }
}

VOID FDD_ClearBufSgList(struct list_head *pstHead)
{
    FDD_BUF_SG_S *pstSg;

    while (!list_empty(pstHead))
    {
        pstSg = list_first_entry(pstHead, FDD_BUF_SG_S, stNode);
        list_del(&pstSg->stNode);
        FDD_FreeSg(pstSg);
    }
}


VOID FDD_ClearBufHead(FDD_BUF_HEAD_S *pstBufHead)
{
    spin_lock_bh(&pstBufHead->stLock);
    atomic_add(pstBufHead->u32QueLen, &pstBufHead->stDropCnt);

    if ( pstBufHead->u8Mode == FDD_BUF_HEAD_MOD_SG )
    {
        FDD_ClearBufSgList(&pstBufHead->stHead);
    }
    else
    {
        FDD_ClearBufList(&pstBufHead->stHead);
    }

    pstBufHead->u32Bytes  = 0;
    pstBufHead->u32QueLen = 0;

    if ( pstBufHead->pstSg )
    {
        FDD_FreeSg(pstBufHead->pstSg);
    }
    pstBufHead->pstSg = NULL;
    spin_unlock_bh(&pstBufHead->stLock);

    return;
}

U32 FDD_GetBufQueueLen(FDD_BUF_HEAD_S *pstBufHead)
{
    return pstBufHead->u32QueLen;
}

FDD_BUF_S *__FDD_BufDequeue(FDD_BUF_HEAD_S *pstBufHead)
{
    FDD_BUF_S *pstBuf;

    pstBuf = list_first_entry((&pstBufHead->stHead), FDD_BUF_S, stNode);
    list_del(&pstBuf->stNode);

    pstBufHead->u32Bytes -= pstBuf->u32BufLen;
    pstBufHead->u32QueLen--;
    return pstBuf;
}

FDD_BUF_S *FDD_BufDequeue(FDD_BUF_HEAD_S *pstBufHead)
{
    FDD_BUF_S *pstBuf;

    spin_lock_bh(&pstBufHead->stLock);
    if ( list_empty(&pstBufHead->stHead) )
    {
        spin_unlock_bh(&pstBufHead->stLock);
        return NULL;
    }

    pstBuf = __FDD_BufDequeue(pstBufHead);
    spin_unlock_bh(&pstBufHead->stLock);

    return pstBuf;
}

VOID FDD_FreeSg(FDD_BUF_SG_S *pstSg)
{
    FDD_ClearBufList(&pstSg->stBufHead);
    VA_Free(pstSg);
}

FDD_BUF_SG_S *__FDD_SgDequeue(FDD_BUF_HEAD_S *pstBufHead)
{
    FDD_BUF_SG_S *pstSg;

    pstSg = list_first_entry((&pstBufHead->stHead), FDD_BUF_SG_S, stNode);
    list_del(&pstSg->stNode);

    pstBufHead->u32Bytes -= pstSg ->u32TotBytes;
    pstBufHead->u32QueLen--;

    return pstSg;
}

FDD_BUF_SG_S *FDD_SgDequeue(FDD_BUF_HEAD_S *pstBufHead)
{
    FDD_BUF_SG_S *pstSg;

    spin_lock_bh(&pstBufHead->stLock);
    if ( list_empty(&pstBufHead->stHead) )
    {
        spin_unlock_bh(&pstBufHead->stLock);
        return NULL;
    }

    pstSg = __FDD_SgDequeue(pstBufHead);
    spin_unlock_bh(&pstBufHead->stLock);

    return pstSg;
}


FDD_BUF_S *FDD_HoldBuf(FDD_BUF_MGR_S *pstBufMgr, FDD_BUF_S *pstBuf)
{
    FDD_BUF_S *pstTmpBuf;

    if ( pstBuf->u16Flag & FDD_BUF_FLG_TEMP )
    {
        pstTmpBuf = FDD_CloneTmpBuf(pstBufMgr, pstBuf, 0);
    }
    else if ( !list_empty(&pstBuf->stNode) )
    {
        pstTmpBuf = FDD_CloneBuf(pstBuf);
    }
    else
    {
        pstTmpBuf = FDD_GetBuf(pstBuf);
    }

    return pstTmpBuf;
}

int FDD_QueueBufTail(FDD_BUF_HEAD_S *pstBufHead, FDD_BUF_S *pstBuf)
{
    FDD_BUF_S *pstTmpBuf;

    pstBuf = FDD_HoldBuf(pstBufHead->pstBufMgr, pstBuf);
    if ( pstBuf == NULL )
    {
        atomic_inc(&pstBufHead->stDropCnt);
        return -ENOMEM;
    }

    spin_lock_bh(&pstBufHead->stLock);

    pstBufHead->u32StatCnt++;

    if ( pstBufHead->u32LimitLen != 0 )
    {
        while ( pstBufHead->u32Bytes > pstBufHead->u32LimitLen && list_empty(&pstBufHead->stHead) == false )
        {
            pstTmpBuf = __FDD_BufDequeue(pstBufHead);
            FDD_FreeBuf(pstTmpBuf);
            atomic_inc(&pstBufHead->stDropCnt);
        }
    }

    pstBufHead->u32Bytes += pstBuf->u32BufLen;
    pstBufHead->u32QueLen++;
    FDD_GetBuf(pstBuf);
    list_add_tail(&pstBuf->stNode, &pstBufHead->stHead);
    spin_unlock_bh(&pstBufHead->stLock);

    return 0;
}

static VOID FDD_AllocSg(FDD_BUF_HEAD_S *pstBufHead)
{
    pstBufHead->pstSg = kzalloc(sizeof(FDD_BUF_SG_S), GFP_ATOMIC);
    if (pstBufHead->pstSg == NULL)
    {
        return;
    }

    INIT_LIST_HEAD(&pstBufHead->pstSg->stBufHead);
}

int FDD_QueueBufSg(FDD_BUF_HEAD_S *pstBufHead, FDD_BUF_S *pstBuf, bool bEnd)
{
    FDD_BUF_SG_S *pstTmpSg;
    FDD_BUF_SG_S *pstSg;

    pstBuf = FDD_HoldBuf(pstBufHead->pstBufMgr, pstBuf);
    if ( pstBuf == NULL )
    {
        atomic_inc(&pstBufHead->stDropCnt);
        return -ENOMEM;
    }

    spin_lock_bh(&pstBufHead->stLock);

    pstBufHead->u32StatCnt++;

    if (pstBufHead->pstSg == NULL)
    {
        FDD_AllocSg(pstBufHead);
        if (pstBufHead->pstSg == NULL)
        {
            spin_unlock_bh(&pstBufHead->stLock);
            FDD_FreeBuf(pstBuf);
            return -ENOMEM;
        }
    }

    pstSg = pstBufHead->pstSg;

    if ( pstBufHead->u32LimitLen != 0 )
    {
        while ( pstBufHead->u32Bytes > pstBufHead->u32LimitLen && list_empty(&pstBufHead->stHead) == false )
        {
            pstTmpSg = __FDD_SgDequeue(pstBufHead);
            FDD_FreeSg(pstTmpSg);
            atomic_inc(&pstBufHead->stDropCnt);
        }

        if ( pstSg->u32TotBytes > pstBufHead->u32LimitLen )
        {
            FDD_ClearBufList(&pstSg->stBufHead);
            pstSg->u32TotBytes = 0;
        }
    }

    pstSg->u32TotBytes += pstBuf->u32BufLen;
    list_add_tail(&pstBuf->stNode, &pstSg->stBufHead);

    if ( bEnd )
    {
        pstBufHead->pstSg = NULL;
        pstBufHead->u32Bytes += pstSg->u32TotBytes;
        pstBufHead->u32QueLen++;
        list_add_tail(&pstSg->stNode, &pstBufHead->stHead);
    }

    spin_unlock_bh(&pstBufHead->stLock);
    return 0;
}

#if 0
#endif

BOOL FDD_FindConnect(FDD_LISTEN_PORT_S *pstListenPort, FDD_CHAN_S *pstRxChan)
{
    FDD_PORT_S *pstRxPort;
    bool bFind = false;

    list_for_each_entry(pstRxPort, &pstListenPort->stPortHead, stListenNode)
    {
        if ( pstRxPort->pstRxChan == pstRxChan )
        {
            bFind = true;
            break;
        }
    }

    return bFind;
}

LONG __FDD_ConnectChan(FDD_CHAN_S *pstListenChan, FDD_CHAN_S *pstRxChan)
{
    FDD_LISTEN_PORT_S *pstListenPort;
    FDD_PORT_S *pstRxPort;
    int iRet;

    if ( pstListenChan == pstRxChan )
    {
        return -EINVAL;
    }

    pstListenPort = pstListenChan->pstListenPort;
    if ( pstListenPort == NULL )
    {
        return -EINVAL;
    }

    if ( FDD_FindConnect(pstListenPort, pstRxChan) )
    {
        return -EEXIST;
    }

    pstRxPort = FDD_AllocPort(pstListenChan, pstRxChan);
    if ( pstRxPort == NULL )
    {
        return -ENOMEM;
    }

    if ( pstRxChan->pstOps->pfnInitPort )
    {
        pstRxChan->pstOps->pfnInitPort(pstRxChan, pstRxPort);
    }

    iRet = FDD_AddToListenPort(pstListenPort, pstRxPort);
    if ( iRet < 0 )
    {
        FDD_PutPort(pstRxPort);
        return iRet;
    }

    list_add(&pstRxPort->stPairNode, &pstRxChan->stPortHead);
    return 0;
}

LONG FDD_ConnectChan(FDD_CHAN_S *pstListenChan, FDD_CHAN_S *pstRxChan)
{
    LONG lRet;

    mutex_lock(&gstFddCb.stMutex);
    lRet = __FDD_ConnectChan(pstListenChan, pstRxChan);
    mutex_unlock(&gstFddCb.stMutex);

    return lRet;
}

LONG FDD_ConnectChanById(CHAN_ID_S *pstListenChanId, CHAN_ID_S *pstRxChanId)
{
    FDD_CHAN_S *pstListenChan;
    FDD_CHAN_S *pstRxChan;
    LONG lRet;

    mutex_lock(&gstFddCb.stMutex);

    pstListenChan = FDD_FindChan(pstListenChanId);
    if ( pstListenChan == NULL )
    {
        mutex_unlock(&gstFddCb.stMutex);
        return -ENOENT;
    }

    pstRxChan = FDD_FindChan(pstRxChanId);
    if ( pstRxChan == NULL )
    {
        mutex_unlock(&gstFddCb.stMutex);
        return -ENOENT;
    }

    lRet = __FDD_ConnectChan(pstListenChan, pstRxChan);
    mutex_unlock(&gstFddCb.stMutex);

    return lRet;
}


LONG __FDD_DisconnectChan(FDD_CHAN_S *pstListenChan, FDD_CHAN_S *pstRxChan)
{
    FDD_LISTEN_PORT_S *pstListenPort;
    FDD_PORT_S *pstRxPort;
    bool bFind = false;

    pstListenPort = pstListenChan->pstListenPort;
    if ( pstListenPort == NULL )
    {
        return -EINVAL;
    }

    list_for_each_entry(pstRxPort, &pstListenPort->stPortHead, stListenNode)
    {
        if ( pstRxPort->pstRxChan == pstRxChan )
        {
            bFind = true;
            break;
        }
    }

    if ( bFind == false ) // can't find entry
    {
        return -ENOENT;
    }

    list_del(&pstRxPort->stPairNode);
    FDD_DelFrmListenPort(pstListenPort, pstRxPort);
    FDD_PutPort(pstRxPort);

    return 0;
}


LONG FDD_DisconnectChan(FDD_CHAN_S *pstListenChan, FDD_CHAN_S *pstRxChan)
{
    LONG lRet;

    // need to optimize, use hash
    mutex_lock(&gstFddCb.stMutex);
    lRet = __FDD_DisconnectChan(pstListenChan, pstRxChan);
    mutex_unlock(&gstFddCb.stMutex);

    return lRet;
}

LONG FDD_DisconnectChanById(CHAN_ID_S *pstListenChanId, CHAN_ID_S *pstRxChanId)
{
    FDD_CHAN_S *pstListenChan;
    FDD_CHAN_S *pstRxChan;
    LONG lRet;

    mutex_lock(&gstFddCb.stMutex);

    pstListenChan = FDD_FindChan(pstListenChanId);
    if ( pstListenChan == NULL )
    {
        mutex_unlock(&gstFddCb.stMutex);
        return -ENOENT;
    }

    pstRxChan = FDD_FindChan(pstRxChanId);
    if ( pstRxChan == NULL )
    {
        mutex_unlock(&gstFddCb.stMutex);
        return -ENOENT;
    }

    lRet = __FDD_DisconnectChan(pstListenChan, pstRxChan);
    mutex_unlock(&gstFddCb.stMutex);

    return lRet;
}


#if 0
#endif

FDD_CHAN_S *FDD_GetChan(FDD_CHAN_S *pstChan)
{
    atomic_inc(&pstChan->stRef);
    return pstChan;
}

VOID FDD_PutChan(FDD_CHAN_S *pstChan)
{
    if ( atomic_dec_and_test(&pstChan->stRef) )
    {
        if ( pstChan->pstOps && pstChan->pstOps->pfnRelease )
        {
            pstChan->pstOps->pfnRelease(pstChan);
        }

        VA_Free(pstChan);
    }
}

VOID FDD_SetChanRxLimitLen(FDD_CHAN_S *pstChan, U32 u32LimitLen)
{
    FDD_SetBufLimitLen(&pstChan->stRxQueue, u32LimitLen);
}

VOID FDD_SetChanRxMode(FDD_CHAN_S *pstChan, U8 u8Mode)
{
    FDD_SetBufHeadMode(&pstChan->stRxQueue, u8Mode);
}


FDD_LISTEN_PORT_S *FDD_FindListenPort(U64 u64Id)
{
    FDD_LISTEN_PORT_S *pstListenPort;
    U32 u32HashVal;

    u32HashVal = VA_Hash64(u64Id, FDD_LISTEN_HASH_BITS);

    hlist_for_each_entry(pstListenPort, &gstFddCb.astListenHashTbl[u32HashVal], stListenNode)
    {
        if ( pstListenPort->u64ListenId == u64Id )
        {
            return pstListenPort;
        }
    }

    return NULL;
}

VOID FDD_AddListenPortToList(FDD_LISTEN_PORT_S *pstListenPort)
{
    U32 u32HashId = VA_Hash64(pstListenPort->u64ListenId, FDD_LISTEN_HASH_BITS);

    mutex_lock(&gstFddCb.stMutex);
    hlist_add_head(&pstListenPort->stListenNode, &gstFddCb.astListenHashTbl[u32HashId]);
    mutex_unlock(&gstFddCb.stMutex);
}

VOID FDD_DelListernPortFrmList(FDD_LISTEN_PORT_S *pstListenPort)
{
    mutex_lock(&gstFddCb.stMutex);
    hlist_del(&pstListenPort->stListenNode);
    mutex_unlock(&gstFddCb.stMutex);
}

FDD_CHAN_S *FDD_FindChan(CHAN_ID_S *pstChanId)
{
    FDD_CHAN_S *pstChan;
    U32 u32HashVal;

    u32HashVal = VA_Hash64(pstChanId->u64ChanId, FDD_CHAN_HASH_BITS);

    hlist_for_each_entry(pstChan, &gstFddCb.astChanHashTbl[u32HashVal], stChanHnode)
    {
        if ( pstChanId->u64ChanId == pstChan->stChanId.u64ChanId )
        {
            return pstChan;
        }
    }

    return NULL;
}

VOID FDD_AddChanToList(FDD_CHAN_S *pstChan)
{
    U32 u32HashId = VA_Hash64(pstChan->stChanId.u64ChanId, FDD_CHAN_HASH_BITS);

    mutex_lock(&gstFddCb.stMutex);
    hlist_add_head(&pstChan->stChanHnode, &gstFddCb.astChanHashTbl[u32HashId]);
    mutex_unlock(&gstFddCb.stMutex);
}

VOID FDD_DelChanFrmList(FDD_CHAN_S *pstChan)
{
    mutex_lock(&gstFddCb.stMutex);
    hlist_del(&pstChan->stChanHnode);
    mutex_unlock(&gstFddCb.stMutex);
}

int FDD_InitListenChan(FDD_CHAN_S *pstChan, U64 u64ListenId)
{
    pstChan->pstListenPort = FDD_AllocListenPort(pstChan, u64ListenId);
    if ( pstChan->pstListenPort == NULL )
    {
        VA_LOG_ERR("failed to init listen " VA_CHAN_FMT, VA_CHAN_ARGS(&pstChan->stChanId));
        return -ENOMEM;
    }

    FDD_AddListenPortToList(pstChan->pstListenPort);
    return 0;
}

VOID FDD_ReleaseListenPort(FDD_LISTEN_PORT_S *pstListenPort)
{
    FDD_DelListernPortFrmList(pstListenPort);
    mutex_lock(&gstFddCb.stMutex);
    FDD_FreeListenPort(pstListenPort);
    mutex_unlock(&gstFddCb.stMutex);
}

int __FDD_InitChan(FDD_CHAN_S *pstChan, CHAN_ID_S *pstChanId, const FDD_CHAN_OPS_S *pstOps)
{
    int iRet;

    FDD_InitBufHead(&pstChan->stRxQueue, &gstFddCb.stBufMgr);
    INIT_HLIST_NODE(&pstChan->stChanHnode);
    INIT_LIST_HEAD(&pstChan->stPortHead);

    pstChan->stChanId     = *pstChanId;
    pstChan->pstBufMgr    = &gstFddCb.stBufMgr;
    pstChan->pstOps       = pstOps;
    pstChan->u16DataType  = VA_STREAM_TYPE_NONE;
    atomic_set(&pstChan->stRef, 1);

    if ( pstOps->pfnInit )
    {
        iRet = pstOps->pfnInit(pstChan);
        if ( iRet < 0 )
        {
            return iRet;
        }
    }

    FDD_AddChanToList(pstChan);
    return 0;
}

int FDD_InitChan(FDD_CHAN_S *pstChan, CHAN_ID_S *pstChanId, const FDD_CHAN_OPS_S *pstOps)
{
    int iRet = 0;

    iRet = __FDD_InitChan(pstChan, pstChanId, pstOps);
    return iRet;
}

VOID FDD_ReleaseAllPorts(FDD_CHAN_S *pstChan)
{
    FDD_PORT_S *pstRxPort;

    // disconnect the chan
    mutex_lock(&gstFddCb.stMutex);
    while (list_empty(&pstChan->stPortHead) == false)
    {
        pstRxPort = list_first_entry(&pstChan->stPortHead, FDD_PORT_S, stPairNode);
        __FDD_DisconnectChan(pstRxPort->pstListenChan, pstRxPort->pstRxChan);
    }
    mutex_unlock(&gstFddCb.stMutex);
}

VOID FDD_ReleaseChan(FDD_CHAN_S *pstChan)
{
    FDD_DelChanFrmList(pstChan);

    if ( pstChan->pstListenPort != NULL )
    {
        FDD_ReleaseListenPort(pstChan->pstListenPort);
        pstChan->pstListenPort = NULL;
    }

    FDD_ReleaseAllPorts(pstChan);
    FDD_ClearBufHead(&pstChan->stRxQueue);

    if ( pstChan->pstOps && pstChan->pstOps->pfnRelease )
    {
        pstChan->pstOps->pfnRelease(pstChan);
    }

    FDD_PutChan( pstChan);
    return;
}

#if 0
#endif

BOOL FDD_QueueWork(struct work_struct *pstWork)
{
    return queue_work(gstFddCb.pstWorkQueue, pstWork);
}

BOOL FDD_QueueDwork(struct delayed_work *pstDwork, ULONG ulDelay)
{
    return queue_delayed_work(gstFddCb.pstWorkQueue, pstDwork, ulDelay);
}

BOOL FDD_FlushWork(struct work_struct *pstWork)
{
    return flush_work(pstWork);
}

BOOL FDD_CancelWork(struct work_struct *pstWork)
{
    return cancel_work_sync(pstWork);
}

BOOL FDD_CancelDwork(struct delayed_work *pstDwork)
{
    return cancel_delayed_work_sync(pstDwork);
}

#if 0
#endif

static int FDD_InstInit(void)
{
    int iRet;

    mutex_init(&gstFddCb.stMutex);
    VA_InitHlistTbl(gstFddCb.astListenHashTbl, VA_ARR_SIZE(gstFddCb.astListenHashTbl));
    VA_InitHlistTbl(gstFddCb.astChanHashTbl,   VA_ARR_SIZE(gstFddCb.astChanHashTbl));

    iRet = FDD_InitBufMgr(&gstFddCb.stBufMgr);
    if ( iRet < 0 )
    {
        return iRet;
    }

    gstFddCb.pstWorkQueue = create_singlethread_workqueue("fddd");
    if ( gstFddCb.pstWorkQueue == NULL )
    {
        FDD_ExitBufMgr(&gstFddCb.stBufMgr);
        return -ENOMEM;
    }

    return 0;
}

static void FDD_InstExit(void)
{
    flush_workqueue(gstFddCb.pstWorkQueue);
    destroy_workqueue(gstFddCb.pstWorkQueue);
    FDD_ExitBufMgr(&gstFddCb.stBufMgr);
    return;
}

static MT_CB_S gstFddMt =
{
    .pfnRegCmd   = FDD_MT_RegCmd,
    .szName      = "fdd",
    .szDesc      = "fast data dispatch",
    .u32DebugCap = MT_DBG_PKT|MT_DBG_INFO|MT_DBG_ERR,
};

static int FDD_MemDrvInit(VA_DEV_S *pstDev)
{
    VA_MEM_DEV_S *pstMemDev = (VA_MEM_DEV_S *)pstDev;

    gstFddCb.stBufMgr.u32TotalSize = pstMemDev->u32TotSize;
    gstFddCb.stBufMgr.u32BlkSize   = pstMemDev->u32BlkSize;

    return 0;
}

static VA_DRV_S gstFddMemDrv =
{
    .u32Id      = VA_DEV_ID_MEM,
    .u32PrivId  = VA_MEM_TYPE_FDD,
    .szName     = "fdd_mem",
    .init       = FDD_MemDrvInit,
    .exit       = NULL,
    .mt         = VA_DispMemDev,
};

static void FDD_Exit(void)
{
    MT_UnRegMt(&gstFddMt);
    FDD_InstExit();
    VA_UnRegDrv(&gstFddMemDrv);
}

static int FDD_Init(VA_MOD_S *pstMod)
{
    int iRet = 0;

    VA_RegDrv(&gstFddMemDrv);

    iRet = FDD_InstInit();
    if ( iRet < 0 )
    {
        return iRet;
    }

    MT_RegMt(&gstFddMt);
    printk("fdd init ok\n");
    return 0;
}

VA_MOD_INIT(fdd, FDD_Init, FDD_Exit, VA_INIT_LEVEL_CORE)

