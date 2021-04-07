#include "vn_chan.h"
#include "vn_udp.h"
#include "va_chan_base_mt.h"
#include <linux/in.h>

void VN_UDP_Mt(VN_CHAN_S *pstChan, ULONG ulExecId)
{
    VN_UDP_S  *pstUdp;

    pstUdp = (VN_UDP_S *)pstChan->pPrivCb;

    MT_PRINT("\r\nUdp Info:");

    MT_PRINT(MT_FMT_PTR,  "Rx Buf",     pstUdp->pRxBuf);
    MT_PRINT(MT_FMT_PTR,  "Rx Io Base", pstUdp->stRxIoVec.iov_base);
    MT_PRINT(MT_FMT_U  ,  "Rx Io Len",  (U32)pstUdp->stRxIoVec.iov_len);
}


#if 0
#endif

VOID VN_UDP_RxPkt(VN_CHAN_S *pstChan)
{
    VN_UDP_S     *pstUdp;
    struct sockaddr_in stInAddr;
    int iRet;

    pstUdp = (VN_UDP_S *)pstChan->pPrivCb;
    pstChan->stRxMsgHdr.msg_name    = &stInAddr;
    pstChan->stRxMsgHdr.msg_namelen = sizeof(stInAddr);

    for ( ; ; )
    {
        iRet = kernel_recvmsg(pstChan->pstSock, &pstChan->stRxMsgHdr, &pstUdp->stRxIoVec, 1, VN_RX_BUF_LEN, MSG_DONTWAIT);
        if ( iRet <= 0 )
        {
            return;
        }

        pstChan->stStat.ulRxBytes += (ULONG)iRet;
        pstChan->stStat.ulRxPkts++;

        if ( pstChan->stRemoteAddr.be32IpAddr != 0 && stInAddr.sin_addr.s_addr != pstChan->stRemoteAddr.be32IpAddr )
        {
            pstChan->stStat.ulRxDropPkts++;
            return;
        }

        if ( pstChan->stRemoteAddr.be16Port != 0 && stInAddr.sin_port != pstChan->stRemoteAddr.be16Port )
        {
            pstChan->stStat.ulRxDropPkts++;
            return;
        }

        FDD_FwdDirectData(&pstChan->stFddChan, pstUdp->pRxBuf, (U32)iRet);
        pstChan->stStat.ulRxOkPkts++;
    }

    return;
}

static void VN_UDP_RxWork(struct work_struct *pstWork)
{
    VN_CHAN_S *pstVnChan;

    pstVnChan = container_of(pstWork, VN_CHAN_S, stRxWork);

    VN_UDP_RxPkt(pstVnChan);
}

VOID VN_UDP_OutPkt(FDD_PORT_S *pstPort, VOID *pBuf, U32 u32Len)
{
    FDD_CHAN_S  *pstFddChan;
    VN_CHAN_S   *pstVnChan;
    VN_UDP_S    *pstUdp;
    struct kvec stIoVec;
    FDD_BUF_S   *pstBuf;
    INT iRet;

    pstFddChan = pstPort->pstRxChan;
    pstVnChan  = VA_PTR_TYPE(VN_CHAN_S, pstFddChan);
    pstUdp     = (VN_UDP_S *)pstVnChan->pPrivCb;
    pstBuf     = (FDD_BUF_S *)pBuf;

    VA_CHAN_DBG_PKT_PRINT(pstFddChan, pstBuf->pData, pstBuf->u32BufLen, "%u/%u/%u UDP Tx", VA_CHAN_ARGS(&pstFddChan->stChanId));

    pstVnChan->stStat.ulTxPkts++;
    pstVnChan->stStat.ulTxBytes += u32Len;

    stIoVec.iov_base = pstBuf->pData;
    stIoVec.iov_len  = u32Len;

    iRet = kernel_sendmsg(pstVnChan->pstSock, &pstVnChan->stTxMsgHdr, &stIoVec, 1, u32Len);
    if ( unlikely((U32)iRet != u32Len) )
    {
        pstVnChan->stStat.ulTxErrPkts++;
        return;
    }

    pstVnChan->stStat.ulTxOkPkts++;
    return;
}

static FDD_PORT_OPS_S gstVnUdpPortOps =
{
    .pfnOutPkt = VN_UDP_OutPkt,
};


#if 0
#endif

VOID VN_UDP_DataReady(struct sock *pstSk)
{
    VN_CHAN_S *pstChan = (VN_CHAN_S *)pstSk->sk_user_data;

    FDD_QueueWork(&pstChan->stRxWork);
    return;
}

VOID VN_UDP_WriteSpace(struct sock *pstSk)
{
    return;
}

static int VN_UDP_DrvInit(VA_DEV_S *pstDev)
{
    VN_CHAN_S *pstChan = container_of(pstDev, VN_CHAN_S, stDev);
    VN_UDP_S  *pstUdp;

    pstUdp = VA_Zmalloc(sizeof(*pstUdp));
    if ( pstUdp == NULL )
    {
        return -ENOMEM;
    }

    pstUdp->pRxBuf = VA_VMalloc(VN_RX_BUF_LEN);
    if ( pstUdp->pRxBuf == NULL )
    {
        VA_Free(pstUdp);
        return -ENOMEM;
    }

    pstUdp->stRxIoVec.iov_base = pstUdp->pRxBuf;
    pstUdp->stRxIoVec.iov_len  = VN_RX_BUF_LEN;

    INIT_WORK(&pstChan->stRxWork, VN_UDP_RxWork);

    pstChan->pstSk->sk_user_data = pstChan;
    pstChan->pPrivCb    = pstUdp;
    pstChan->pstPortOps = &gstVnUdpPortOps;
    pstChan->pfnMt      = VN_UDP_Mt;
    smp_wmb();

    pstUdp->stDstAddr.sin_family = AF_INET;
    pstUdp->stDstAddr.sin_addr.s_addr = pstChan->stRemoteAddr.be32IpAddr;
    pstUdp->stDstAddr.sin_port        = pstChan->stRemoteAddr.be16Port;

	pstChan->stTxMsgHdr.msg_name    = (struct sockaddr_in *)(&pstUdp->stDstAddr);
	pstChan->stTxMsgHdr.msg_namelen = sizeof(struct sockaddr_in);

    pstChan->pstSk->sk_data_ready  = (typeof(pstChan->pstSk->sk_data_ready))VN_UDP_DataReady;
    pstChan->pstSk->sk_write_space = VN_UDP_WriteSpace;
    smp_wmb();

    VN_FlushRxData(pstChan, pstUdp->pRxBuf);
    return 0;
}

static void VN_UDP_DrvExit(VA_DEV_S *pstDev)
{
    VN_CHAN_S *pstChan = container_of(pstDev, VN_CHAN_S, stDev);
    VN_UDP_S  *pstUdp;

    FDD_CancelWork(&pstChan->stRxWork);
    pstUdp = (VN_UDP_S *)pstChan->pPrivCb;

    VA_VFree(pstUdp->pRxBuf);
    VA_Free(pstUdp);
    return;
}

VA_DRV_S gstVnUdpDrv =
{
    .u32Id     = VA_DEV_ID_VN,
    .u32PrivId = VN_TRANSPORT_UDP,
    .szName    = "vn_tcp",
    .init      = VN_UDP_DrvInit,
    .exit      = VN_UDP_DrvExit,
    .mt        = NULL,
};

static void VN_UDP_Exit(void)
{
    VA_UnRegDrv(&gstVnUdpDrv);
}

static int VN_UDP_Init(VA_MOD_S *pstMod)
{
    int iRet = 0;

    VA_RegDrv(&gstVnUdpDrv);
    return iRet;
}

VA_MOD_INIT(vnudp, VN_UDP_Init, VN_UDP_Exit, VA_INIT_LEVEL_DRV)

