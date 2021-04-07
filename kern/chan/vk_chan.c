#include "vk_chan.h"
#include "va_chan_base_mt.h"

#if 0
#endif

VOID VK_MtDispChan(FDD_CHAN_S *pstChan, ULONG ulExecId)
{
    VK_CHAN_S *pstVkChan;

    pstVkChan = (VK_CHAN_S *)pstChan;

    MT_PRINT(MT_FMT_U,   "Data Ready", pstVkChan->bDataReady);
    MT_PRINT(MT_FMT_U,   "Dts",        pstVkChan->u32Dts);
    MT_PRINT(MT_FMT_U,   "Pts",        pstVkChan->u32Pts);
    MT_PRINT(MT_FMT_UL,  "Tx Cnt",     pstVkChan->ulTxCnt);
    MT_PRINT(MT_FMT_PTR, "Curr Buf",   pstVkChan->pstCurrBuf);

    FDD_MT_DispChan(pstChan, ulExecId);
}

#if 0
#endif

LONG VK_BindChan(struct file *pstFile, CHAN_ID_S __user *pstChanId)
{
    CHAN_ID_S stChanId;

    if ( copy_from_user(&stChanId, pstChanId, sizeof(CHAN_ID_S)) )
    {
        return -EFAULT;
    }

    return VA_BindChan(pstFile, &stChanId);
}

LONG __VK_PutData(VK_CHAN_S *pstVkChan, MEDIA_BUF_HDR_S *pstBufHdr, VOID __user *pBuf, U32 u32DataLen)
{
    ES_STOR_PKT_HDR_S *pstStorPktHdr;
    ES_PKT_HDR_S *pstEsPkt;
    ES_PKT_HDR_S *pstInEsPkt;
    FDD_CHAN_S   *pstChan;
    FDD_BUF_S    *pstCurrBuf;
    U16   u16PktNo = 0;
    U32   u32Off   = 0;
    U32   u32CpyLen;
    U32   u32LeftLen;
    U32   u32BufLen;

    pstChan    = &pstVkChan->stFddChan;
    u32LeftLen = DRV_LE32_TO_CPU(pstBufHdr->le32Len);
    pstInEsPkt = &pstBufHdr->stStorPktHdr.stEsPkt;
    pstVkChan->ulTxCnt++;

    while ( u32LeftLen > 0 )
    {
        pstCurrBuf = FDD_AllocPrivBuf(pstChan->pstBufMgr, pstVkChan->pstCurrBuf, 0);
        if ( pstCurrBuf == NULL )
        {
            return -ENOMEM;
        }

        u32BufLen = pstCurrBuf->u32BufLen - sizeof(ES_STOR_PKT_HDR_S);

        if ( u32LeftLen > u32BufLen )
        {
            u32CpyLen = u32BufLen;
        }
        else
        {
            u32CpyLen = u32LeftLen;
        }

        if ( u32CpyLen > VA_MAX_ES_PKT_LEN )
        {
            u32CpyLen = VA_MAX_ES_PKT_LEN;
        }

        pstStorPktHdr = VA_PTR_TYPE(ES_STOR_PKT_HDR_S, pstCurrBuf->pData);
        pstEsPkt      = &pstStorPktHdr->stEsPkt;

        memcpy(pstStorPktHdr, &pstBufHdr->stStorPktHdr, sizeof(ES_STOR_PKT_HDR_S));

        pstEsPkt->le16PktLen = DRV_CPU_TO_LE16((U16)u32CpyLen);
        pstEsPkt->le16PktSeq = DRV_CPU_TO_LE16(u16PktNo);

        if (VA_CHAN_TYPE_VCAP_ENC == pstChan->stChanId.u16ChanType)
        {
            if ((0 == u16PktNo) && (TRUE == pstInEsPkt->stInfo.u8BitFirst))
            {
                pstEsPkt->stInfo.u8BitFirst = TRUE;
            }
            else
            {
                pstEsPkt->stInfo.u8BitFirst = FALSE;
            }

            if ((u32LeftLen <= u32CpyLen) && (TRUE == pstInEsPkt->stInfo.u8BitEnd))
            {
                pstEsPkt->stInfo.u8BitEnd = TRUE;
            }
            else
            {
                pstEsPkt->stInfo.u8BitEnd = FALSE;
            }
        }
        else
        {
            if ( 0 != u16PktNo )
            {
                pstEsPkt->stInfo.u8BitFirst = FALSE;
            }
            else
            {
                pstEsPkt->stInfo.u8BitFirst = TRUE;
            }

            if ( u32LeftLen <= u32CpyLen )
            {
                pstEsPkt->stInfo.u8BitEnd = TRUE;
            }
            else
            {
                pstEsPkt->stInfo.u8BitEnd = FALSE;
            }
        }

        if ( copy_from_user(pstEsPkt + 1, (char *)pBuf + u32Off, u32CpyLen) )
        {
            return -EFAULT;
        }

        pstVkChan->pstCurrBuf = FDD_TrimBufLen(pstChan->pstBufMgr, pstCurrBuf, u32CpyLen + sizeof(ES_STOR_PKT_HDR_S));

        VA_CHAN_DBG_PKT_PRINT(pstChan, pstCurrBuf->pData, pstCurrBuf->u32BufLen, "%u/%u/%u Tx FWD", VA_CHAN_ARGS(&pstVkChan->stChanId));

        FDD_FwdBufData(pstChan, pstCurrBuf, pstCurrBuf->u32BufLen);
        FDD_FreeBuf(pstCurrBuf);

        u16PktNo += 1;
        u32Off   += u32CpyLen;
        if ( u32LeftLen > u32CpyLen )
        {
            u32LeftLen -= u32CpyLen;
        }
        else
        {
            break;
        }
    }

    return 0;
}

LONG VK_PutData(VK_CHAN_S *pstVkChan, VA_DATA_BUF_S __user *pstBuf)
{
    MEDIA_BUF_HDR_S stBufHdr;
    VA_DATA_BUF_S   stDataBuf;

    if ( copy_from_user(&stDataBuf, pstBuf, sizeof(stDataBuf)) )
    {
        return -EFAULT;
    }

    if ( copy_from_user(&stBufHdr, stDataBuf.pHead, sizeof(stBufHdr)) )
    {
        return -EFAULT;
    }

    return __VK_PutData(pstVkChan, &stBufHdr, stDataBuf.pData, stDataBuf.u32Len);
}

LONG VK_GetData(VK_CHAN_S *pstVkChan, VA_DATA_BUF_S __user *pstUsrBuf)
{
    VA_DATA_BUF_S  stDataBuf;
    FDD_BUF_HEAD_S *pstRxQueue;
    FDD_BUF_SG_S   *pstSg;
    FDD_BUF_S      *pstBuf;

    ES_STOR_PKT_HDR_S *pstStorPktHdr;
    ES_PKT_HDR_S      *pstEsPkt;
    MEDIA_BUF_HDR_S __user *pstUsrBufHdr;

    U16 u16PktSeq = VA_USHORT_MAX;
    U16 u16PktLen;
    U32 u32Off = 0;
    U8 __user *pu8Buf;
    LONG lRet = 0;

    if ( copy_from_user(&stDataBuf, pstUsrBuf, sizeof(VA_DATA_BUF_S)) )
    {
        return -EFAULT;
    }

    pstUsrBufHdr = (MEDIA_BUF_HDR_S __user *)stDataBuf.pHead;
    pu8Buf       = (U8 __user *)stDataBuf.pData;
    pstRxQueue   = &pstVkChan->stFddChan.stRxQueue;

    pstSg = FDD_SgDequeue(pstRxQueue);
    if ( pstSg == NULL )
    {
        return -ENODATA;
    }

    list_for_each_entry(pstBuf, &pstSg->stBufHead, stNode)
    {
        pstStorPktHdr = VA_PTR_TYPE(ES_STOR_PKT_HDR_S, pstBuf->pData);
        pstEsPkt = &pstStorPktHdr->stEsPkt;
        if ( VA_IS_FIRST_PKT(pstEsPkt) )
        {
            u16PktLen = DRV_LE16_TO_CPU(pstEsPkt->le16PktLen);
            u16PktSeq = DRV_LE16_TO_CPU(pstEsPkt->le16PktSeq);
            u32Off    = 0;
        }
        else
        {
            if ( u16PktSeq != DRV_LE16_TO_CPU(pstEsPkt->le16PktSeq) )
            {
                u16PktSeq = VA_USHORT_MAX;
                u32Off    = 0;
                continue;
            }

            u16PktLen    = DRV_LE16_TO_CPU(pstEsPkt->le16PktLen);
        }

        if ( (u32Off + u16PktLen) > stDataBuf.u32Len )
        {
            lRet = -ENOMEM;
            break;
        }

        if ( copy_to_user(pu8Buf + u32Off, pstEsPkt + 1, u16PktLen) )
        {
            lRet = -EFAULT;
            break;
        }

        u16PktSeq++;
        u32Off += u16PktLen;

        VA_CHAN_DBG_PKT_PRINT(&pstVkChan->stFddChan, pstEsPkt, u16PktLen + sizeof(*pstEsPkt), "%u/%u/%u Get Frame", VA_CHAN_ARGS(&pstVkChan->stChanId));

        if ( VA_IS_END_PKT(pstEsPkt) )
        {
            LE32 le32Len = DRV_CPU_TO_LE32(u32Off);

            if ( copy_to_user(&pstUsrBufHdr->le32Len, &le32Len, sizeof(le32Len)) )
            {
                lRet = -EFAULT;
                break;
            }

            if ( copy_to_user(&pstUsrBufHdr->stStorPktHdr, pstStorPktHdr, sizeof(ES_STOR_PKT_HDR_S)) )
            {
                lRet = -EFAULT;
                break;
            }

            break;
        }
    }

    FDD_FreeSg(pstSg);
    return lRet;
}

#if 0
#endif

int VK_ChanInit(VK_CHAN_S *pstChan, BOOL bListen)
{
    int iRet;

    pstChan->bDataReady = FALSE;
    init_waitqueue_head(&pstChan->stWaitHead);

    if ( bListen )
    {
        iRet = FDD_InitListenChan(&pstChan->stFddChan, pstChan->stChanId.u64ChanId);
        if ( iRet < 0 )
        {
            return iRet;
        }
    }

    return 0;
}

VOID VK_OutEsPkt(FDD_PORT_S *pstPort, VOID *pBuf, U32 u32Len)
{
    VK_CHAN_S *pstVkChan;
    FDD_CHAN_S *pstFddChan;

    pstFddChan = pstPort->pstRxChan;
    pstVkChan  = VA_PTR_TYPE(VK_CHAN_S, pstFddChan);

    pstVkChan->bDataReady = TRUE;
    FDD_QueueBufTail(&pstFddChan->stRxQueue, (FDD_BUF_S *)pBuf);
    wake_up(&pstVkChan->stWaitHead);
}

VOID VK_OutEsFrame(FDD_PORT_S *pstPort, VOID *pBuf, U32 u32Len)
{
    ES_STOR_PKT_HDR_S *pstStorPktHdr;
    ES_PKT_HDR_S *pstEsPkt;
    FDD_CHAN_S *pstFddChan;
    VK_CHAN_S *pstVkChan;
    FDD_BUF_S *pstBuf;

    pstFddChan = pstPort->pstRxChan;
    pstVkChan  = VA_PTR_TYPE(VK_CHAN_S, pstFddChan);
    pstBuf = (FDD_BUF_S *)pBuf;

    pstStorPktHdr = (ES_STOR_PKT_HDR_S *)pstBuf->pData;
    pstEsPkt      = &pstStorPktHdr->stEsPkt;

    VA_CHAN_DBG_PKT_PRINT(pstFddChan, pstBuf->pData, pstBuf->u32BufLen, "%u/%u/%u Rx FWD", VA_CHAN_ARGS(&pstVkChan->stChanId));

    FDD_QueueBufSg(&pstFddChan->stRxQueue, pstBuf, pstEsPkt->stInfo.u8BitEnd);

    if ( pstFddChan->stRxQueue.u32QueLen != 0 )
    {
        pstVkChan->bDataReady = TRUE;
        wake_up(&pstVkChan->stWaitHead);
    }
}

U32 VK_DataReady(FDD_CHAN_S *pstFddChan)
{
    if ( pstFddChan->stRxQueue.u32QueLen != 0 )
    {
        return POLLIN | POLLRDNORM;
    }

    return 0;
}

LONG VK_Ioctl(U32 u32Cmd, ULONG ulArg)
{
    return 0;
}

#if 0
#endif

extern int  VK_FileInit(void);
extern void VK_FileExit(void);

static void VK_CHAN_Exit(void)
{
    VK_FileExit();
}

static int VK_CHAN_Init(VA_MOD_S *pstMod)
{
    int iRet = 0;

    VK_FileInit();
    return iRet;
}

VA_MOD_INIT(vkchan, VK_CHAN_Init, VK_CHAN_Exit, VA_INIT_LEVEL_CORE)

