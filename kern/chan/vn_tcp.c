#include "vn_chan.h"
#include "vn_tcp.h"
#include "va_chan_base_mt.h"
#include <linux/in.h>

void VN_TCP_Mt(VN_CHAN_S *pstChan, ULONG ulExecId)
{
    VN_TCP_S  *pstTcp;

    pstTcp = (VN_TCP_S *)pstChan->pPrivCb;

    MT_PRINT("\r\nTcp Info:");

    MT_PRINT(MT_FMT_U,    "Rx Rd Indx", pstTcp->u32RxRdIndx);
    MT_PRINT(MT_FMT_U,    "Rx Wr Indx", pstTcp->u32RxWrIndx);
    MT_PRINT(MT_FMT_PTR,  "Rx Mem Buf", pstTcp->stRxBuf.pMemBuf);
    MT_PRINT(MT_FMT_U  ,  "Rx Buf Num", pstTcp->stRxBuf.u32PageNum);
}

VOID __VN_TCP_RxPkt(VN_CHAN_S *pstChan, VN_TCP_S *pstTcp, VA_MAP_BUF_S *pstBuf)
{
    TCP_PKT_HDR_S *pstHdr;
    U32  u32LeftLen;
    U32  u32NeedLen;
    U16  u16PktLen;

    do
    {
        u32LeftLen = pstTcp->u32RxWrIndx - pstTcp->u32RxRdIndx;
        if ( u32LeftLen <= sizeof(TCP_PKT_HDR_S) )
        {
            return;
        }

        pstHdr = (TCP_PKT_HDR_S *)((pstTcp->u32RxRdIndx & VN_RX_BUF_MSK) + pstBuf->pMemBuf);
        if ( '$' == pstHdr->u8Header )
        {
            u16PktLen = ntohs(pstHdr->be16Len);
            if ( u16PktLen < (VN_RX_BUF_LEN - VA_KB) ) // max pkt len 63KB
            {
                u32NeedLen = (U32)u16PktLen + sizeof(TCP_PKT_HDR_S);
                if ( u32NeedLen > u32LeftLen )
                {
                    // data was not ready, wait again
                    return;
                }

                pstTcp->u32RxRdIndx += u32NeedLen;
                FDD_FwdDirectData(&pstChan->stFddChan, pstHdr + 1, (U32)u16PktLen);
                pstChan->stStat.ulRxOkPkts++;
                continue;
            }
        }

        pstTcp->u32RxRdIndx++;
        pstChan->stStat.ulRxDropPkts++;
    } while ( TRUE );

    return;
}

VOID VN_TCP_RxPkt(VN_CHAN_S *pstChan)
{
    VA_MAP_BUF_S *pstBuf;
    VN_TCP_S     *pstTcp;
    struct kvec stTmpIoVec;
    U32 u32LeftLen;
    int iRet;

    pstTcp     = (VN_TCP_S *)pstChan->pPrivCb;
    pstBuf     = &pstTcp->stRxBuf;
    u32LeftLen = VN_RX_BUF_LEN - (pstTcp->u32RxWrIndx - pstTcp->u32RxRdIndx);

    stTmpIoVec.iov_base = pstBuf->pMemBuf + (pstTcp->u32RxWrIndx & VN_RX_BUF_MSK);
    stTmpIoVec.iov_len  = u32LeftLen;

    iRet = kernel_recvmsg(pstChan->pstSock, &pstChan->stRxMsgHdr, &stTmpIoVec, 1, u32LeftLen, MSG_DONTWAIT);
    if ( iRet > 0 )
    {
        pstTcp->u32RxWrIndx       += (U32)iRet;
        pstChan->stStat.ulRxBytes += (ULONG)iRet;
    }

    __VN_TCP_RxPkt(pstChan, pstTcp, pstBuf);
    return;
}

static void VN_TCP_RxWork(struct work_struct *pstWork)
{
    VN_CHAN_S *pstVnChan;

    pstVnChan = container_of(pstWork, VN_CHAN_S, stRxWork);

    VN_TCP_RxPkt(pstVnChan);
}

VOID VN_TCP_OutPkt(FDD_PORT_S *pstPort, VOID *pBuf, U32 u32Len)
{
    TCP_PKT_HDR_S *pstHdr;
    FDD_CHAN_S  *pstFddChan;
    VN_CHAN_S   *pstVnChan;
    VN_TCP_S    *pstTcp;
    struct kvec *pstIoVec;
    FDD_BUF_S   *pstBuf;
    U16 u16Len;
    INT iRet;

    pstFddChan = pstPort->pstRxChan;
    pstVnChan  = VA_PTR_TYPE(VN_CHAN_S, pstFddChan);
    pstTcp     = (VN_TCP_S *)pstVnChan->pPrivCb;
    pstHdr     = &pstTcp->stTxHdrCache;
    pstIoVec   = pstTcp->astTxIoVec + 1;
    pstBuf     = (FDD_BUF_S *)pBuf;

    VA_CHAN_DBG_PKT_PRINT(pstFddChan, pstBuf->pData, pstBuf->u32BufLen, "%u/%u/%u TCP Tx", VA_CHAN_ARGS(&pstFddChan->stChanId));

    pstVnChan->stStat.ulTxPkts++;
    pstVnChan->stStat.ulTxBytes += u32Len;

    u16Len             = (U16)u32Len;
    pstHdr->be16Len    = htons(u16Len);
    pstIoVec->iov_base = pstBuf->pData;
    pstIoVec->iov_len  = u32Len;

    iRet = kernel_sendmsg(pstVnChan->pstSock, &pstVnChan->stTxMsgHdr, pstTcp->astTxIoVec,
                          VA_ARR_SIZE(pstTcp->astTxIoVec), u32Len + sizeof(*pstHdr));
    if ( unlikely((U32)iRet != (u32Len + sizeof(*pstHdr))) )
    {
        pstVnChan->stStat.ulTxErrPkts++;
    }

    pstVnChan->stStat.ulTxOkPkts++;
    return;
}

static FDD_PORT_OPS_S gstVnTcpPortOps =
{
    .pfnOutPkt = VN_TCP_OutPkt,
};


#if 0
#endif

VOID VN_TCP_DataReady(struct sock *pstSk)
{
    VN_CHAN_S *pstChan = (VN_CHAN_S *)pstSk->sk_user_data;

    FDD_QueueWork(&pstChan->stRxWork);
    return;
}

VOID VN_TCP_WriteSpace(struct sock *pstSk)
{
    return;
}

static int VN_TCP_DrvInit(VA_DEV_S *pstDev)
{
    VN_CHAN_S *pstChan = container_of(pstDev, VN_CHAN_S, stDev);
    VN_TCP_S  *pstTcp;
    int nRet;

    pstTcp = VA_Zmalloc(sizeof(*pstTcp));
    if ( pstTcp == NULL )
    {
        return -ENOMEM;
    }

    nRet = VA_AllocDoubleMapBuf(&pstTcp->stRxBuf, VN_RX_BUF_LEN);
    if ( nRet < 0 )
    {
        VA_Free(pstTcp);
        return nRet;
    }

    INIT_WORK(&pstChan->stRxWork, VN_TCP_RxWork);

    pstTcp->stTxHdrCache.u8Header  = '$';
    pstTcp->stTxHdrCache.u8ChanId  = 0;
    pstTcp->astTxIoVec[0].iov_base = &pstTcp->stTxHdrCache;
    pstTcp->astTxIoVec[0].iov_len  = sizeof(pstTcp->stTxHdrCache);

    pstChan->pstSk->sk_user_data = pstChan;
    pstChan->pPrivCb    = pstTcp;
    pstChan->pstPortOps = &gstVnTcpPortOps;
    pstChan->pfnMt      = VN_TCP_Mt;
    smp_wmb();

    pstChan->pstSk->sk_data_ready  = (typeof(pstChan->pstSk->sk_data_ready))VN_TCP_DataReady;
    pstChan->pstSk->sk_write_space = VN_TCP_WriteSpace;
    smp_wmb();

    VN_FlushRxData(pstChan, pstTcp->stRxBuf.pMemBuf);
    return 0;
}

static void VN_TCP_DrvExit(VA_DEV_S *pstDev)
{
    VN_CHAN_S *pstChan = container_of(pstDev, VN_CHAN_S, stDev);
    VN_TCP_S  *pstTcp;

    pstTcp = (VN_TCP_S *)pstChan->pPrivCb;
    pstChan->pPrivCb = NULL;
    VA_ReleaseMapBufMem(&pstTcp->stRxBuf);
    VA_Free(pstTcp);

    return;
}

VA_DRV_S gstVnTcpDrv =
{
    .u32Id     = VA_DEV_ID_VN,
    .u32PrivId = VN_TRANSPORT_TCP,
    .szName    = "vn_tcp",
    .init      = VN_TCP_DrvInit,
    .exit      = VN_TCP_DrvExit,
    .mt        = NULL,
};

static void VN_TCP_Exit(void)
{
    VA_UnRegDrv(&gstVnTcpDrv);
}

static int VN_TCP_Init(VA_MOD_S *pstMod)
{
    int iRet = 0;

    VA_RegDrv(&gstVnTcpDrv);
    return iRet;
}

VA_MOD_INIT(vntcp, VN_TCP_Init, VN_TCP_Exit, VA_INIT_LEVEL_DRV)

