#include "vc_es2rtp.h"

VOID VC_ES2RtpOutputData(VC_CB_S *pstVcCb, VC_RTP_TYPE_S *pstRtpType, VA_RTP_HDR_S *pstRtpHdr, VOID *pData, U32 u32Len)
{
    pstRtpType->u16SeqId++;
    pstRtpHdr->be16SeqNo = DRV_CPU_TO_BE16(pstRtpType->u16SeqId);

    pstVcCb->pfnOutputData(pstVcCb->pPrivCb, pstRtpType->stBufType.u8DataType, pstRtpHdr, sizeof(VA_RTP_HDR_S));
    pstVcCb->pfnOutputData(pstVcCb->pPrivCb, pstRtpType->stBufType.u8DataType, pData, u32Len);
    VC_FlushEsData(pstVcCb, &pstRtpType->stBufType);

    return;
}

LONG VC_ES2Rtp(VC_CB_S *pstVcCb, ES_PKT_HDR_S *pstEsPkt, U32 u32Len, VC_RTP_TYPE_S *pstRtpType)
{
    VC_ES2RTP_CB_S *pstVcEs2RtpCb;
    VA_RTP_HDR_S   stRtpHdr;
    U16 u16PktLen;
    U8  *pu8Data;

    pstVcEs2RtpCb = VA_PTR_TYPE(VC_ES2RTP_CB_S, pstVcCb);
    u16PktLen     = DRV_LE16_TO_CPU(pstEsPkt->le16PktLen);
    pu8Data       = (U8 *)(pstEsPkt + 1);

    if ( pstEsPkt->stInfo.u8BitFirst )
    {
        pstRtpType->u32TimeStamp = DRV_LE32_TO_CPU(pstEsPkt->le32PTS);
        pstRtpType->u32TimeStamp = pstRtpType->u32TimeStamp << 1;
    }

    VC_InitRtpHdr(&stRtpHdr, pstRtpType->u8PayloadType, pstVcEs2RtpCb->u32Ssrc, pstRtpType->u32TimeStamp);

    while (u16PktLen > 0)
    {
        if ( u16PktLen > VC_RTP_MAX_LEN )
        {
            VC_ES2RtpOutputData(pstVcCb, pstRtpType, &stRtpHdr, pu8Data, VC_RTP_MAX_LEN);
            u16PktLen -= VC_RTP_MAX_LEN;
            pu8Data   += VC_RTP_MAX_LEN;
        }
        else
        {
            if ( pstEsPkt->stInfo.u8BitEnd )
            {
                stRtpHdr.u8Marker = 1;
            }

            VC_ES2RtpOutputData(pstVcCb, pstRtpType, &stRtpHdr, pu8Data, u16PktLen);
            u16PktLen = 0;
        }
    }

    return VC_SUCCESS;
}

LONG VC_ES2RtpConvert(VC_CB_S *pstVcCb, void *pData, U32 u32Len)
{
    VC_ES2RTP_CB_S *pstVcEs2RtpCb;
    ES_STOR_PKT_HDR_S *pstStorPktHdr;
    ES_PKT_HDR_S *pstEsPkt;
    LONG lRet = VC_E_INVALID;

    pstVcEs2RtpCb = VA_PTR_TYPE(VC_ES2RTP_CB_S, pstVcCb);
    pstStorPktHdr = VA_PTR_TYPE(ES_STOR_PKT_HDR_S, pData);
    pstEsPkt = &pstStorPktHdr->stEsPkt;

    if (VA_ES_TYPE_JPEG == pstEsPkt->u8EncFormat)
    {
        lRet = VC_ES2Rtp(pstVcCb, pstEsPkt, u32Len, &pstVcEs2RtpCb->stRawType);
    }
    else
    {
        if (VA_BUF_TYPE_VIDEO == pstEsPkt->stInfo.u8BitType)
        {
            lRet = VC_ES2Rtp(pstVcCb, pstEsPkt, u32Len, &pstVcEs2RtpCb->stVideoType);
        }
        else
        {
            lRet = VC_ES2Rtp(pstVcCb, pstEsPkt, u32Len, &pstVcEs2RtpCb->stAudioType);
        }
    }

    return lRet;
}

VOID VC_Es2RtpInitType(VC_RTP_TYPE_S *pstRtpType, U8 u8PayLoadType, U8 u8DataType)
{
    if ( u8DataType == VA_BUF_TYPE_VIDEO )
    {
        VC_InitEsBufType(&pstRtpType->stBufType, VA_ES_TYPE_H265, u8DataType);
    }
    else
    {
        VC_InitEsBufType(&pstRtpType->stBufType, VA_AUDIO_TYPE_PCMA, u8DataType);
    }

    pstRtpType->u8PayloadType = u8PayLoadType;
}

LONG VC_Es2RtpInit(VC_CB_S *pstVcCb)
{
    VC_ES2RTP_CB_S *pstVcEs2RtpCb;

    pstVcEs2RtpCb = VA_PTR_TYPE(VC_ES2RTP_CB_S, pstVcCb);
    VC_Es2RtpInitType(&pstVcEs2RtpCb->stAudioType, VC_RTP_PT_AUDIO, VA_BUF_TYPE_AUDIO);
    VC_Es2RtpInitType(&pstVcEs2RtpCb->stVideoType, VC_RTP_PT_ES,    VA_BUF_TYPE_VIDEO);
    VC_Es2RtpInitType(&pstVcEs2RtpCb->stRawType,   VC_RTP_PT_RAW,   VA_BUF_TYPE_VIDEO);

    pstVcEs2RtpCb->u32Ssrc = 0x2222; // fix me

    return VC_SUCCESS;
}

VOID VC_Es2RtpDestroy(VC_CB_S *pstVcCb)
{
    return;
}

VCER_S gstEs2RtpVcer =
{
    .u16InDataType  = VA_STREAM_TYPE_ES,
    .u16OutDataType = VA_STREAM_TYPE_RTP,
    .u32CbSize      = sizeof(VC_ES2RTP_CB_S),
    .pfnInit        = VC_Es2RtpInit,
    .pfnDestroy     = VC_Es2RtpDestroy,
    .pfnConvertProc = VC_ES2RtpConvert,
};

static void VCER_ES2RTP_Exit(void)
{
}

static int VCER_ES2RTP_Init(VA_MOD_S *pstMod)
{
    int iRet = 0;

    VC_RegVcer(&gstEs2RtpVcer);
    return iRet;
}

VA_MOD_INIT(es2rtp, VCER_ES2RTP_Init, VCER_ES2RTP_Exit, VA_INIT_LEVEL_MISC)

