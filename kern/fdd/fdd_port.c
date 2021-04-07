#include "fdd.h"
#include "vc/vc_mgr.h"

VOID FDD_PutVswPort(FDD_VSW_PORT_S *pstVswPort);

LONG FDD_ReservedDataBuf(VOID *pPrivCb, U8 u8DataType, U32 u32RsvdLen)
{
    FDD_VSW_OUT_BUF_S *pstOutBuf;
    FDD_VSW_PORT_S *pstVswPort = VA_PTR_TYPE(FDD_VSW_PORT_S, pPrivCb);

    pstOutBuf = pstVswPort->astOutBuf + u8DataType;
    if ( pstOutBuf->pstCurrBuf->u32BufLen >= u32RsvdLen )
    {
        return VC_SUCCESS;
    }

    FDD_FreeBuf(pstOutBuf->pstCurrBuf);
    pstOutBuf->pstCurrBuf = NULL;
    pstOutBuf->u32CurrLen = 0;

    pstOutBuf->pstCurrBuf = FDD_AllocPrivBuf(pstVswPort->pstBufMgr, pstOutBuf->pstCurrBuf, 0);
    if ( pstOutBuf->pstCurrBuf == NULL )
    {
        return VC_E_NOMEM;
    }

    if ( pstOutBuf->pstCurrBuf->u32BufLen < u32RsvdLen )
    {
        return VC_E_NOBUF; // it's bug on, need to fix!
    }

    return VC_SUCCESS;
}

U32 FDD_GetDataBufLeftLen(VOID *pPrivCb, U8 u8DataType)
{
    FDD_VSW_OUT_BUF_S *pstOutBuf;
    FDD_VSW_PORT_S *pstVswPort = VA_PTR_TYPE(FDD_VSW_PORT_S, pPrivCb);

    pstOutBuf = pstVswPort->astOutBuf + u8DataType;

    if ( pstOutBuf->pstCurrBuf == NULL )
    {
        return FDD_BUF_MAX_SIZE(pstVswPort->pstBufMgr);
    }

    return pstOutBuf->pstCurrBuf->u32BufLen - pstOutBuf->u32CurrLen;
}

LONG FDD_OutputData2Vsw(VOID *pPrivCb, U8 u8DataType, VOID *pData, U32 u32Len)
{
    FDD_VSW_OUT_BUF_S *pstOutBuf;
    FDD_VSW_PORT_S *pstVswPort = VA_PTR_TYPE(FDD_VSW_PORT_S, pPrivCb);
    FDD_BUF_S *pstCurrBuf;

    pstOutBuf = pstVswPort->astOutBuf + u8DataType;
    pstCurrBuf = FDD_AllocPrivBuf(pstVswPort->pstBufMgr, pstOutBuf->pstCurrBuf, 0);
    if ( pstCurrBuf == NULL )
    {
        return VC_E_NOMEM;
    }

    if ( pstOutBuf->u32CurrLen + u32Len > pstCurrBuf->u32BufLen )
    {
        return VC_E_NOBUF; // fix me, realloc new buf and copy data?
    }

    memcpy(pstCurrBuf->pData + pstOutBuf->u32CurrLen, pData, u32Len);
    pstOutBuf->u32CurrLen += u32Len;
    pstOutBuf->pstCurrBuf  = pstCurrBuf;

    return VC_SUCCESS;
}

VOID FDD_FlushData2Vsw(VOID *pPrivCb, U8 u8DataType)
{
    FDD_VSW_OUT_BUF_S *pstOutBuf;
    FDD_VSW_PORT_S *pstVswPort = VA_PTR_TYPE(FDD_VSW_PORT_S, pPrivCb);
    FDD_PORT_S *pstRxPort;
    FDD_BUF_S *pstCurrBuf;

    pstOutBuf  = pstVswPort->astOutBuf + u8DataType;
    if ( pstOutBuf->pstCurrBuf == NULL )
    {
        pstOutBuf->u32CurrLen = 0;
        return;
    }

    if( pstOutBuf->u32CurrLen == 0 )
    {
        return;
    }

    pstCurrBuf = FDD_TrimBufLen(pstVswPort->pstBufMgr, pstOutBuf->pstCurrBuf, pstOutBuf->u32CurrLen);

    pstVswPort->ulOutCount++;
    list_for_each_entry(pstRxPort, &pstVswPort->stFwdHead, stFwdNode)
    {
        pstRxPort->pstOps->pfnOutPkt(pstRxPort, pstOutBuf->pstCurrBuf, pstOutBuf->u32CurrLen);
    }

    FDD_FreeBuf(pstOutBuf->pstCurrBuf);
    pstOutBuf->pstCurrBuf = pstCurrBuf;
    pstOutBuf->u32CurrLen = 0;
}

VOID FDD_VswOutPkt(FDD_PORT_S *pstPort, VOID *pBuf, U32 u32Len)
{
    FDD_VSW_PORT_S *pstVswPort = VA_PTR_TYPE(FDD_VSW_PORT_S, pstPort);
    struct list_head *pstNode;
    FDD_PORT_S *pstRxPort;

    pstVswPort->ulInCount++;

    if ( pstVswPort->pVcCb )
    {
        FDD_BUF_S *pstBuf = (FDD_BUF_S *)pBuf;
        VC_ConvertData((VC_CB_S *)pstVswPort->pVcCb, pstBuf->pData, u32Len);
    }
    else
    {
        list_for_each(pstNode, &pstVswPort->stFwdHead)
        {
            pstRxPort = VA_PTR_TYPE(FDD_PORT_S, pstNode);
            pstRxPort->pstOps->pfnOutPkt(pstRxPort, pBuf, u32Len);
        }
    }
}

VOID FDD_FwdBufData(FDD_CHAN_S *pstChan, VOID *pBuf, U32 u32Len)
{
    FDD_LISTEN_PORT_S *pstListenPort = pstChan->pstListenPort;
    if ( pstListenPort == NULL )
    {
        return;
    }

    mutex_lock(&pstListenPort->stMutex);
    FDD_VswOutPkt(&pstListenPort->stVswPort.stPort, pBuf, u32Len);
    mutex_unlock(&pstListenPort->stMutex);
}

VOID FDD_FwdDirectData(FDD_CHAN_S *pstChan, VOID *pBuf, U32 u32Len)
{
    FDD_LISTEN_PORT_S *pstListenPort = pstChan->pstListenPort;
    FDD_BUF_S stBuf;

    if ( pstListenPort == NULL )
    {
        return;
    }

    FDD_InitTmpBuf(&stBuf, pBuf, u32Len);

    mutex_lock(&pstListenPort->stMutex);
    FDD_VswOutPkt(&pstListenPort->stVswPort.stPort, &stBuf, u32Len);
    mutex_unlock(&pstListenPort->stMutex);
}

FDD_VSW_PORT_S *FDD_CreateConvertVsw(FDD_LISTEN_PORT_S *pstListenPort, FDD_BUF_MGR_S *pstBufMgr, U16 u16InDataType, U16 u16OutDataType)
{
    FDD_VSW_PORT_S *pstVswPort;
    VC_PARAM_S stParam;
    VC_CB_S *pstVcCb;
    U32 u32InOutDataType;

    u32InOutDataType = ((U32)u16InDataType << 16) + u16OutDataType;
    pstVswPort = FDD_AllocVswPort(pstBufMgr, pstListenPort, sizeof(FDD_VSW_PORT_S));
    if ( pstVswPort == NULL )
    {
        return NULL;
    }

    pstVswPort->u16InDataType  = u16InDataType;
    pstVswPort->u16OutDataType = u16OutDataType;

    stParam.u16InDataType  = u16InDataType;
    stParam.u16OutDataType = u16OutDataType;
    stParam.pPrivCb        = pstVswPort;
    stParam.pfnFlush         = FDD_FlushData2Vsw;
    stParam.pfnOutputData    = FDD_OutputData2Vsw;
    stParam.pfnGetBufLeftLen = FDD_GetDataBufLeftLen;

    pstVcCb = VC_CreateVideoConverter(&stParam);
    if ( IS_ERR(pstVcCb) )
    {
        FDD_PutVswPort(pstVswPort);
        return NULL;
    }

    pstVswPort->pVcCb = pstVcCb;

    list_add_tail(&pstVswPort->stPort.stListenNode, &pstListenPort->stVswHead);
    return pstVswPort;
}

#if 0
#endif

VOID FDD_InitPort(FDD_PORT_S *pstPort, FDD_CHAN_S *pstListenChan, VOID *pPrivCb)
{
    INIT_LIST_HEAD(&pstPort->stFwdNode);
    INIT_LIST_HEAD(&pstPort->stPairNode);
    INIT_LIST_HEAD(&pstPort->stListenNode);

    pstPort->pstListenChan = pstListenChan;
    pstPort->pPrivCb       = pPrivCb;
    atomic_set(&pstPort->stRef, 1);
}

FDD_PORT_S *FDD_AllocPort(FDD_CHAN_S *pstListenChan, FDD_CHAN_S *pstRxChan)
{
    FDD_PORT_S *pstPort = VA_Zmalloc(sizeof(FDD_PORT_S));
    if ( pstPort == NULL )
    {
        return NULL;
    }

    FDD_InitPort(pstPort, pstListenChan, pstRxChan);
    return pstPort;
}

FDD_PORT_S *FDD_GetPort(FDD_PORT_S *pstPort)
{
    atomic_inc(&pstPort->stRef);
    return pstPort;
}

VOID FDD_PutPort(FDD_PORT_S *pstPort)
{
    if ( atomic_dec_and_test(&pstPort->stRef) )
    {
        if ( pstPort->pstOps && pstPort->pstOps->pfnRelease )
        {
            pstPort->pstOps->pfnRelease(pstPort);
        }

        VA_Free(pstPort);
    }
}

VOID FDD_PutVswPort(FDD_VSW_PORT_S *pstVswPort)
{
    FDD_PutPort(&pstVswPort->stPort);
}

FDD_VSW_PORT_S *FDD_GetVswPort(FDD_VSW_PORT_S *pstVswPort)
{
    FDD_GetPort(&pstVswPort->stPort);
    return pstVswPort;
}

VOID FDD_ReleaseVswPort(FDD_PORT_S *pstPort)
{
    FDD_LISTEN_PORT_S *pstListenPort;
    FDD_VSW_PORT_S *pstVswPort;

    pstVswPort = VA_PTR_TYPE(FDD_VSW_PORT_S, pstPort);
    FDD_ClearBufHead(&pstVswPort->stTxQueue);
    if ( pstVswPort->pfnRelease )
    {
        pstVswPort->pfnRelease(pstVswPort);
    }

    pstListenPort = VA_PTR_TYPE(FDD_LISTEN_PORT_S, pstPort->pPrivCb);
    if ( pstListenPort->pstBaseVswPort == pstVswPort )
    {
        pstListenPort->pstBaseVswPort = NULL;
    }

    list_del(&pstVswPort->stPort.stFwdNode);
    list_del(&pstVswPort->stPort.stListenNode);

    if ( pstVswPort->pVcCb != NULL )
    {
        VC_ReleaseVideoConverter(pstVswPort->pVcCb);
        pstVswPort->pVcCb = NULL;
    }

    if ( pstVswPort->stPort.pstVswPort )
    {
        FDD_PutVswPort(pstVswPort->stPort.pstVswPort);
        pstVswPort->stPort.pstVswPort = NULL;
    }

    return;
}

FDD_PORT_OPS_S gstFddVswPortOps =
{
    .pfnRelease = FDD_ReleaseVswPort,
    .pfnOutPkt   = FDD_VswOutPkt,
};

VOID FDD_InitVswPort(FDD_VSW_PORT_S *pstVswPort, FDD_BUF_MGR_S *pstBufMgr, FDD_LISTEN_PORT_S *pstListenPort)
{
    FDD_PORT_S *pstPort;

    pstPort = &pstVswPort->stPort;

    FDD_InitPort(pstPort, pstListenPort->pstFddChan, pstListenPort);
    FDD_InitBufHead(&pstVswPort->stTxQueue, pstBufMgr);
    INIT_LIST_HEAD(&pstVswPort->stFwdHead);

    pstPort->pstOps       = &gstFddVswPortOps;
    pstVswPort->pstBufMgr = pstBufMgr;
}

FDD_VSW_PORT_S *FDD_AllocVswPort(FDD_BUF_MGR_S *pstBufMgr, FDD_LISTEN_PORT_S *pstListenPort, U32 u32Size)
{
    FDD_VSW_PORT_S *pstVswPort;

    BUG_ON(u32Size < sizeof(FDD_VSW_PORT_S));

    pstVswPort = (FDD_VSW_PORT_S *)VA_Zmalloc(u32Size);
    if ( pstVswPort == NULL )
    {
        return NULL;
    }

    FDD_InitVswPort(pstVswPort, pstBufMgr, pstListenPort);
    return pstVswPort;
}

VOID FDD_PutBaseVswPort(FDD_LISTEN_PORT_S *pstListenPort)
{
    FDD_VSW_PORT_S *pstVswPort = pstListenPort->pstBaseVswPort;

    if ( atomic_read(&pstVswPort->stPort.stRef) == 1 )
    {
        pstListenPort->pstBaseVswPort = NULL;
    }

    FDD_PutVswPort(pstVswPort);
}

int FDD_InitListenVswPort(FDD_CHAN_S *pstFddChan, FDD_LISTEN_PORT_S *pstListenPort, FDD_VSW_PORT_S *pstVswPort)
{
    FDD_PORT_S *pstPort;
    int iRet;

    FDD_InitPort(&pstVswPort->stPort, pstFddChan, pstListenPort);
    FDD_InitBufHead(&pstVswPort->stTxQueue, pstFddChan->pstBufMgr);
    INIT_LIST_HEAD(&pstVswPort->stFwdHead);

    pstVswPort->pVcCb          = NULL;
    pstVswPort->pfnConvertProc = NULL;
    pstVswPort->pfnRelease     = NULL;
    pstVswPort->u16InDataType  = pstFddChan->u16DataType;
    pstVswPort->u16OutDataType = pstFddChan->u16DataType;
    pstVswPort->pstBufMgr      = pstFddChan->pstBufMgr;

    if ( pstFddChan->pstOps->pfnInitListenPort )
    {
        pstPort = &pstVswPort->stPort;
        iRet = pstFddChan->pstOps->pfnInitListenPort(pstFddChan, pstPort);
        if ( iRet < 0 )
        {
            return iRet;
        }
    }

    return 0;
}

FDD_LISTEN_PORT_S *FDD_AllocListenPort(FDD_CHAN_S *pstFddChan, U64 u64ListenId)
{
    FDD_LISTEN_PORT_S *pstListenPort;
    FDD_VSW_PORT_S *pstVswPort;
    int iRet;

    pstListenPort = VA_Zmalloc(sizeof(FDD_LISTEN_PORT_S));
    if ( pstListenPort == NULL )
    {
        return NULL;
    }

    mutex_init(&pstListenPort->stMutex);
    pstListenPort->pstFddChan  = pstFddChan;
    pstListenPort->u64ListenId = u64ListenId;
    pstListenPort->pstBufMgr   = pstFddChan->pstBufMgr;

    INIT_LIST_HEAD(&pstListenPort->stVswHead);
    INIT_LIST_HEAD(&pstListenPort->stPortHead);
    INIT_HLIST_NODE(&pstListenPort->stListenNode);

    pstVswPort = &pstListenPort->stVswPort;
    iRet = FDD_InitListenVswPort(pstFddChan, pstListenPort, pstVswPort);
    if ( iRet  < 0 )
    {
        VA_Free(pstListenPort);
        return NULL;
    }

    if ( pstVswPort->u16InDataType == VA_STREAM_TYPE_BASE )
    {
        pstListenPort->pstBaseVswPort = pstVswPort;
    }

    pstVswPort->stPort.pstVswPort = FDD_GetVswPort(pstVswPort);
    list_add_tail(&pstVswPort->stPort.stListenNode, &pstListenPort->stVswHead);
    return pstListenPort;
}

VOID FDD_FreeListenPort(FDD_LISTEN_PORT_S *pstListenPort)
{
    FDD_PORT_S *pstPort;

    // fix me!
    while (list_empty(&pstListenPort->stPortHead) == false)
    {
        pstPort = list_first_entry(&pstListenPort->stPortHead, FDD_PORT_S, stListenNode);
        __FDD_DisconnectChan(pstListenPort->pstFddChan, pstPort->pstRxChan);
    }

    FDD_ReleaseVswPort(&pstListenPort->stVswPort.stPort);
    FDD_PutVswPort(&pstListenPort->stVswPort);
    return;
}

VOID FDD_DelFrmListenPort(FDD_LISTEN_PORT_S *pstListenPort, FDD_PORT_S *pstRxPort)
{
    mutex_lock(&pstListenPort->stMutex);

    list_del(&pstRxPort->stFwdNode);
    if ( pstRxPort->pstVswPort )
    {
        FDD_PutVswPort(pstRxPort->pstVswPort);
        pstRxPort->pstVswPort = NULL;
    }

    mutex_unlock(&pstListenPort->stMutex);

    list_del(&pstRxPort->stListenNode);
    atomic_dec(&pstListenPort->stPortCnt);
    return;
}

VOID FDD_AddToVswPort(FDD_LISTEN_PORT_S *pstListenPort, FDD_VSW_PORT_S *pstVswPort, FDD_PORT_S *pstPort)
{
    pstPort->pstVswPort = FDD_GetVswPort(pstVswPort);

    mutex_lock(&pstListenPort->stMutex);
    list_add_tail(&pstPort->stFwdNode, &pstVswPort->stFwdHead);
    mutex_unlock(&pstListenPort->stMutex);

    list_add_tail(&pstPort->stListenNode, &pstListenPort->stPortHead);
}

VOID FDD_AddVswToVswPort(FDD_LISTEN_PORT_S *pstListenPort, FDD_VSW_PORT_S *pstVswPort, FDD_VSW_PORT_S *pstChildVswPort)
{
    FDD_PORT_S *pstChildPort = &pstChildVswPort->stPort;
    pstChildPort->pstVswPort = FDD_GetVswPort(pstVswPort);

    mutex_lock(&pstListenPort->stMutex);
    list_add_tail(&pstChildPort->stFwdNode, &pstVswPort->stFwdHead);
    mutex_unlock(&pstListenPort->stMutex);
}

int FDD_AddToListenPort(FDD_LISTEN_PORT_S *pstListenPort, FDD_PORT_S *pstRxPort)
{
    FDD_VSW_PORT_S *pstTmpVswPort;
    FDD_VSW_PORT_S *pstBaseVswPort;
    FDD_CHAN_S *pstFddListenChan;
    FDD_CHAN_S *pstRxFddChan;
    FDD_PORT_S *pstPort;

    pstFddListenChan = pstListenPort->pstFddChan;
    pstRxFddChan     = pstRxPort->pstRxChan;

    list_for_each_entry(pstPort, &pstListenPort->stVswHead, stListenNode)
    {
        pstTmpVswPort = VA_PTR_TYPE(FDD_VSW_PORT_S, pstPort);
        if ( pstTmpVswPort->u16OutDataType == pstRxFddChan->u16DataType )
        {
            atomic_inc(&pstListenPort->stPortCnt);
            FDD_AddToVswPort(pstListenPort, pstTmpVswPort, pstRxPort);
            return 0;
        }
    }

    // convert path: list port data type ---> base type --> rx port data type
    if ( (pstRxFddChan->u16DataType != VA_STREAM_TYPE_BASE) && (pstListenPort->pstBaseVswPort == NULL) )
    {
        pstBaseVswPort = FDD_CreateConvertVsw(pstListenPort, pstListenPort->pstBufMgr, pstFddListenChan->u16DataType, VA_STREAM_TYPE_BASE);
        if ( pstBaseVswPort == NULL )
        {
            VA_LOG_ERR("Failed to create convert %u %u", pstFddListenChan->u16DataType, VA_STREAM_TYPE_BASE);
            return -ENOMEM;
        }

        pstTmpVswPort = FDD_CreateConvertVsw(pstListenPort, pstListenPort->pstBufMgr, VA_STREAM_TYPE_BASE, pstRxFddChan->u16DataType);
        if ( pstTmpVswPort == NULL )
        {
            FDD_PutVswPort(pstBaseVswPort);
            VA_LOG_ERR("Failed to create convert %u %u", VA_STREAM_TYPE_BASE, pstRxFddChan->u16DataType);
            return -ENOMEM;
        }

        pstListenPort->pstBaseVswPort = pstBaseVswPort;
        FDD_AddToVswPort(pstListenPort, pstTmpVswPort, pstRxPort);
        FDD_AddVswToVswPort(pstListenPort, pstBaseVswPort, pstTmpVswPort);
        FDD_AddVswToVswPort(pstListenPort, &pstListenPort->stVswPort, pstBaseVswPort);
        FDD_PutVswPort(pstBaseVswPort);
        FDD_PutVswPort(pstTmpVswPort);
    }
    else if ( pstListenPort->pstBaseVswPort != NULL )
    {
        pstTmpVswPort = FDD_CreateConvertVsw(pstListenPort, pstListenPort->pstBufMgr, VA_STREAM_TYPE_BASE, pstRxFddChan->u16DataType);
        if ( pstTmpVswPort == NULL )
        {
            VA_LOG_ERR("Failed to create convert %u %u", VA_STREAM_TYPE_BASE, pstRxFddChan->u16DataType);
            return -ENOMEM;
        }

        FDD_AddToVswPort(pstListenPort, pstTmpVswPort, pstRxPort);
        FDD_AddVswToVswPort(pstListenPort, pstListenPort->pstBaseVswPort, pstTmpVswPort);
        FDD_PutVswPort(pstTmpVswPort);
    }
    else // VA_STREAM_TYPE_BASE
    {
        pstBaseVswPort = FDD_CreateConvertVsw(pstListenPort, pstListenPort->pstBufMgr, pstFddListenChan->u16DataType, pstRxFddChan->u16DataType);
        if ( pstBaseVswPort == NULL )
        {
            VA_LOG_ERR("Failed to create convert %u %u", pstFddListenChan->u16DataType, pstRxFddChan->u16DataType);
            return -ENOMEM;
        }

        pstListenPort->pstBaseVswPort = pstBaseVswPort;
        FDD_AddToVswPort(pstListenPort, pstBaseVswPort, pstRxPort);
        FDD_AddVswToVswPort(pstListenPort, &pstListenPort->stVswPort, pstBaseVswPort);
        FDD_PutVswPort(pstBaseVswPort);
    }

    atomic_inc(&pstListenPort->stPortCnt);
    return 0;
}

