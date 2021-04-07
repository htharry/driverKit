/*
 video network channel
 */

#include "vn_chan.h"
#include "va_cp.h"

VN_CHAN_MGR_S gstVnChanMgr;

VOID VN_FlushRxData(VN_CHAN_S *pstChan, char *pBuf)
{
    struct kvec stTmpIoVec;
    INT iRet;

    stTmpIoVec.iov_base = pBuf;
    stTmpIoVec.iov_len  = VN_RX_BUF_LEN;

    pstChan->stRxMsgHdr.msg_name = NULL;

    do
    {
        iRet = kernel_recvmsg(pstChan->pstSock, &pstChan->stRxMsgHdr,
                              &stTmpIoVec, 1, VN_RX_BUF_LEN, MSG_DONTWAIT);
    } while (iRet > 0);

    return;
}

VOID VN_MtDispChan(FDD_CHAN_S *pstChan, ULONG ulExecId)
{
    VN_PKT_STAT_S *pstStat;
    VN_CHAN_S *pstVnChan;

    pstVnChan = (VN_CHAN_S *)pstChan;

    MT_PRINT(MT_FMT_U,      "Data Type",    pstVnChan->u16DataType);
    MT_PRINT(MT_FMT_U,      "Dir",          pstVnChan->u8Dir);
    MT_PRINT(MT_FMT_U,      "Fd",           pstVnChan->nFd);
    MT_PRINT(MT_FMT_U,      "Chan Idx",     pstVnChan->u32ChanIdx);
    MT_PRINT(MT_FMT_FUNC,   "Old Date Ready",   pstVnChan->pfnOldDataReady);
    MT_PRINT(MT_FMT_FUNC,   "Old Write Space",  pstVnChan->pfnOldWriteSpace);
    MT_PRINT(MT_FMT_U,      "Remote Port",  ntohs(pstVnChan->stRemoteAddr.be16Port));
    MT_PRINT(MT_FMT_IP,     "Remote IP",    &pstVnChan->stRemoteAddr.be32IpAddr);
    MT_PRINT(MT_FMT_U,      "Local Port",   ntohs(pstVnChan->stLocalAddr.be16Port));
    MT_PRINT(MT_FMT_IP,     "Local IP",     &pstVnChan->stLocalAddr.be32IpAddr);
    MT_PRINT(MT_FMT_PTR,    "pstFile",      pstVnChan->pstFile);

    if ( pstVnChan->pfnMt )
    {
        pstVnChan->pfnMt(pstVnChan, ulExecId);
    }

    pstStat = &pstVnChan->stStat;
    MT_PRINT("\r\nNetwork Statistic:");
    MT_PRINT(MT_FMT_UL,     "Rx Pkt",       pstStat->ulRxPkts);
    MT_PRINT(MT_FMT_UL,     "Rx Bytes",     pstStat->ulRxBytes);
    MT_PRINT(MT_FMT_UL,     "Rx Ok Pkts",   pstStat->ulRxOkPkts);
    MT_PRINT(MT_FMT_UL,     "Rx Err Pkts",  pstStat->ulRxErrPkts);
    MT_PRINT(MT_FMT_UL,     "Rx Drop Pkts", pstStat->ulRxDropPkts);
    MT_PRINT(MT_FMT_UL,     "Tx Pkts",      pstStat->ulTxPkts);
    MT_PRINT(MT_FMT_UL,     "Tx Bytes",     pstStat->ulTxBytes);
    MT_PRINT(MT_FMT_UL,     "Tx Ok Pkts",   pstStat->ulTxOkPkts);
    MT_PRINT(MT_FMT_UL,     "Tx Err Pkts",  pstStat->ulTxErrPkts);
    MT_PRINT(MT_FMT_UL,     "Tx Drop Pkts", pstStat->ulTxDropPkts);

    FDD_MT_DispChan(pstChan, ulExecId);
}

VOID VN_MtDispChanMgr(ULONG ulExecId)
{
    MT_PRINT(MT_FMT_PTR,    "Bit Map",    gstVnChanMgr.pulChanBitMap);
    MT_PRINT(MT_FMT_U,      "Total Num",  gstVnChanMgr.u32TotChanNum);
    MT_PRINT(MT_FMT_U,      "Use Cnt",    gstVnChanMgr.u32ChanUseCnt);
}

#if 0
#endif

static LONG VN_AddNode(VA_IO_CMD_S *pstIoCmd)
{
    VN_NODE_PARAM_S stNodeParam;
    VN_CHAN_S *pstVnChan;
    CHAN_ID_S stChanId;
    VA_DEV_S  *pstDev;
    U32  u32ChanIdx;
    LONG lRet;
    int  iRet;

    if ( copy_from_user(&stNodeParam, pstIoCmd->pParam, sizeof(VN_NODE_PARAM_S)) )
    {
        return -EFAULT;
    }

    u32ChanIdx = find_first_zero_bit(gstVnChanMgr.pulChanBitMap, gstVnChanMgr.u32TotChanNum);
    if ( u32ChanIdx >= gstVnChanMgr.u32TotChanNum )
    {
        return -ENOENT;
    }

    lRet = VA_ChanIndx2ChanId(VA_CHAN_TYPE_NET, u32ChanIdx, &stChanId);
    if ( lRet < 0 )
    {
        return lRet;
    }

    pstVnChan = (VN_CHAN_S *)VA_CHAN_Init(&stChanId);
    if ( pstVnChan == NULL )
    {
        return -ENOMEM;
    }

    // param init
    pstVnChan->u32ChanIdx   = u32ChanIdx;
    pstVnChan->u8Dir        = stNodeParam.u8Dir;
    pstVnChan->u16DataType  = stNodeParam.u16DataType;
    pstVnChan->stLocalAddr  = stNodeParam.stLocalAddr;
    pstVnChan->stRemoteAddr = stNodeParam.stRemoteAddr;
    pstVnChan->nFd = stNodeParam.nFd;
    pstVnChan->ulInstId = DRV_GetInstId();

    // chan init
    pstVnChan->stFddChan.u16DataType = stNodeParam.u16DataType;

    if ( stNodeParam.u8Dir & VN_RX )
    {
        lRet = FDD_InitListenChan(&pstVnChan->stFddChan, pstVnChan->stFddChan.stChanId.u64ChanId);
        if ( lRet < 0 )
        {
            VA_CHAN_Destroy(&stChanId);
            return lRet;
        }
    }

    pstVnChan->pstFile = fget(pstVnChan->nFd);
    if ( pstVnChan->pstFile == NULL )
    {
        VA_CHAN_Destroy(&stChanId);
        return -EINVAL;
    }

    pstVnChan->pstSock = sock_from_file(pstVnChan->pstFile, &iRet);
    if ( pstVnChan->pstSock == NULL ) // not socket file
    {
        fput(pstVnChan->pstFile);
        VA_CHAN_Destroy(&stChanId);
        return -EINVAL;
    }

    // network init
    pstVnChan->pstSk = pstVnChan->pstSock->sk;
    pstVnChan->pfnOldDataReady  = (void *)(pstVnChan->pstSk->sk_data_ready);
    pstVnChan->pfnOldWriteSpace = (void *)(pstVnChan->pstSk->sk_write_space);

    pstVnChan->stRxMsgHdr.msg_flags = MSG_DONTWAIT;
    pstVnChan->stTxMsgHdr.msg_flags = MSG_DONTWAIT;

    // special transport init
    pstDev = &pstVnChan->stDev;
    pstDev->u32Id     = VA_DEV_ID_VN;
    pstDev->u32PrivId = stNodeParam.u16TransportMode;

    lRet = VA_RegDev(pstDev);
    if ( lRet < 0 )
    {
        fput(pstVnChan->pstFile);
        VA_CHAN_Destroy(&stChanId);
        return lRet;
    }

    stNodeParam.stChanId = stChanId;
    if ( copy_to_user(pstIoCmd->pParam, &stNodeParam, sizeof(stNodeParam)) )
    {
        VA_UnRegDev(&pstVnChan->stDev);
        fput(pstVnChan->pstFile);
        VA_CHAN_Destroy(&stChanId);
        return -EFAULT;
    }

    set_bit(u32ChanIdx, gstVnChanMgr.pulChanBitMap);
    gstVnChanMgr.u32ChanUseCnt++;

    return 0;
}

static VOID VN_ReleaseChan(VN_CHAN_S *pstVnChan)
{
    pstVnChan->pstSk->sk_data_ready  = pstVnChan->pfnOldDataReady;
    pstVnChan->pstSk->sk_write_space = pstVnChan->pfnOldWriteSpace;
    pstVnChan->pstSk->sk_user_data = NULL;

    smp_wmb();
    VA_UnRegDev(&pstVnChan->stDev);

    fput(pstVnChan->pstFile);
    VA_CHAN_Destroy(&pstVnChan->stFddChan.stChanId);
    clear_bit(pstVnChan->u32ChanIdx, gstVnChanMgr.pulChanBitMap);

    return;
}

static LONG VN_DelNode(VA_IO_CMD_S *pstIoCmd)
{
    FDD_CHAN_S *pstChan;
    VN_CHAN_S *pstVnChan;

    pstChan = VA_GetChan(&pstIoCmd->stChanId);
    if ( pstChan == NULL )
    {
        return -ENOENT;
    }

    pstVnChan = (VN_CHAN_S *)pstChan;

    VN_ReleaseChan(pstVnChan);
    FDD_PutChan(pstChan);

    return 0;
}

LONG VN_Ioctl(U32 u32Cmd, VA_IO_CMD_S *pstIoCmd)
{
    LONG lRet = -EINVAL;

    mutex_lock(&gstVnChanMgr.stLock);

    switch ( u32Cmd )
    {
        case VN_IOCTL_ADD_NODE :
            lRet = VN_AddNode(pstIoCmd);
            break;
        case VN_IOCTL_DEL_NODE :
            lRet = VN_DelNode(pstIoCmd);
            break;
        default:
            lRet = -EINVAL;
            break;
    }

    mutex_unlock(&gstVnChanMgr.stLock);
    return lRet;
}

static int VN_ChanInit(FDD_CHAN_S *pstFddChan)
{
    return 0;
}

VOID VN_InitPort(FDD_CHAN_S *pstFddChan, FDD_PORT_S *pstPort)
{
    VN_CHAN_S *pstVnChan;

    pstVnChan = (VN_CHAN_S *)pstFddChan;
    pstPort->pstOps = pstVnChan->pstPortOps;
}

static FDD_CHAN_OPS_S gstVnChanOps =
{
    .pfnInit       = VN_ChanInit,
    .pfnRelease    = NULL,
    .pfnInitPort   = VN_InitPort,
    .pfnDataReady  = NULL,
    .pfnMtDispChan = VN_MtDispChan,
    .pfnInitListenPort = NULL,
};

static VA_CHAN_TYPE_CB_S gstVnTypeCb =
{
    .szName        = "vn",
    .u16ChanType   = VA_CHAN_TYPE_NET,
    .u16ChanCbSize = sizeof(VN_CHAN_S),
    .pfnIoctl      = NULL,
    .pfnTypeMt     = VN_MtDispChanMgr,
    .pstChanOps    = &gstVnChanOps,
    .pstPortOps    = NULL,
};

static int VN_DRV_Init(VA_DEV_S *pstDev)
{
    VA_CHAN_DEV_S *pstChanDev = VA_PTR_TYPE(VA_CHAN_DEV_S, pstDev);
    U32 nTotChanNum;
    U32 nSize;
    int iRet;

    iRet = VA_CHAN_CreatePortTbl(pstChanDev, FALSE);
    if ( iRet < 0 )
    {
        return iRet;
    }

    nTotChanNum = VA_CHAN_GetTotalChanNum(VA_CHAN_TYPE_NET);
    if ( nTotChanNum > 0 )
    {
        nSize = ((nTotChanNum + sizeof(ULONG) * 8 - 1)/8) & (~(sizeof(ULONG) - 1));
        gstVnChanMgr.pulChanBitMap = VA_Zmalloc(nSize);
        if ( gstVnChanMgr.pulChanBitMap == NULL )
        {
            VA_CHAN_DestroyPortTbl((VA_CHAN_DEV_S *)pstDev);
            return -ENOMEM;
        }
    }

    gstVnChanMgr.u32TotChanNum = nTotChanNum;
    return 0;
}

static void VN_DRV_Exit(VA_DEV_S *pstDev)
{
    FDD_CHAN_S *pstChan;
    CHAN_ID_S stChanId;
    VN_CHAN_S *pstVnChan;
    LONG lRet;
    U32 i;

    mutex_lock(&gstVnChanMgr.stLock);

    for ( i = 0; i < gstVnChanMgr.u32TotChanNum; i++ )
    {
        lRet = VA_ChanIndx2ChanId(VA_CHAN_TYPE_NET, i, &stChanId);
        if ( lRet < 0 )
        {
            continue;
        }

        pstChan = VA_GetChan(&stChanId);
        if ( pstChan == NULL )
        {
            continue;
        }

        pstVnChan = (VN_CHAN_S *)pstChan;
        VN_ReleaseChan(pstVnChan);
        FDD_PutChan(pstChan);
    }

    mutex_unlock(&gstVnChanMgr.stLock);

    VA_CHAN_DestroyPortTbl((VA_CHAN_DEV_S *)pstDev);
}

VA_DRV_S gstVnDrv =
{
    .u32Id     = VA_DEV_ID_VCHAN,
    .u32PrivId = VA_CHAN_TYPE_NET,
    .szName    = "vn",
    .init      = VN_DRV_Init,
    .exit      = VN_DRV_Exit,
    .mt        = VA_CHAN_DispChanDev,
};

VA_GBL_IO_CMDS_CB_S gstVnIoCmdCb =
{
    .u32CmdBase = VN_IOCTL_BASE,
    .u32Size    = 0x100,
    .pfnIoCmd   = VN_Ioctl,
};

static VOID VN_CpInstNotify(U32 u32Event, ULONG ulParam)
{
    FDD_CHAN_S *pstChan;
    CHAN_ID_S stChanId;
    VN_CHAN_S *pstVnChan;
    LONG lRet;
    U32 i;

    mutex_lock(&gstVnChanMgr.stLock);

    for ( i = 0; i < gstVnChanMgr.u32TotChanNum; i++ )
    {
        lRet = VA_ChanIndx2ChanId(VA_CHAN_TYPE_NET, i, &stChanId);
        if ( lRet < 0 )
        {
            continue;
        }

        pstChan = VA_GetChan(&stChanId);
        if ( pstChan == NULL )
        {
            continue;
        }

        pstVnChan = (VN_CHAN_S *)pstChan;
        if ( pstVnChan->ulInstId != ulParam )
        {
            FDD_PutChan(pstChan);
            continue;
        }

        VN_ReleaseChan(pstVnChan);
        FDD_PutChan(pstChan);
    }

    mutex_unlock(&gstVnChanMgr.stLock);
}

static void VN_CHAN_Exit(void)
{
    VA_CP_UnRegInstEventNotify(&gstVnChanMgr.stCpInstNotify);
    VA_CP_UnRegIoCmdProc(&gstVnIoCmdCb);
    VA_UnRegDrv(&gstVnDrv);

    if ( gstVnChanMgr.pulChanBitMap )
    {
        VA_Free(gstVnChanMgr.pulChanBitMap);
    }
}

static int VN_CHAN_Init(VA_MOD_S *pstMod)
{
    int iRet = 0;

    INIT_LIST_HEAD(&gstVnChanMgr.stTransportHead);
    mutex_init(&gstVnChanMgr.stLock);
    VA_CHAN_RegTypeCb(&gstVnTypeCb);
    VA_RegDrv(&gstVnDrv);
    VA_CP_RegIoCmdProc(&gstVnIoCmdCb);

    gstVnChanMgr.stCpInstNotify.pfnNotify = VN_CpInstNotify;
    VA_CP_RegInstEventNotify(&gstVnChanMgr.stCpInstNotify);

    return iRet;
}

VA_MOD_INIT(vnchan, VN_CHAN_Init, VN_CHAN_Exit, VA_INIT_LEVEL_DRV)

