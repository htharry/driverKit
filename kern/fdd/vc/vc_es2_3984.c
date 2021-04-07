#include "vc_es2_3984.h"

ULONG VC_ESTo3984Audio(VC_CB_S *pstVcCb, ES_PKT_HDR_S *pstEsPkt, U32 u32Len, VC_RTP_TYPE_S *pstRtpType)
{
    VC_ES_TO_3984_CB_S *pstVc3984Cb;
    VA_RTP_HDR_S stRtpHdr;
    U8  *pu8Data;
    U16 u16PktLen;

    pstVc3984Cb = VA_PTR_TYPE(VC_ES_TO_3984_CB_S, pstVcCb);
    u16PktLen   = DRV_LE16_TO_CPU(pstEsPkt->le16PktLen);
    pu8Data     = (U8 *)(pstEsPkt + 1);

    if ( pstEsPkt->stInfo.u8BitFirst )
    {
        pstRtpType->u32TimeStamp = DRV_LE32_TO_CPU(pstEsPkt->le32PTS);
        pstRtpType->u32TimeStamp = pstRtpType->u32TimeStamp << 1;
    }

    if (pstEsPkt->u8EncFormat == VA_AUDIO_TYPE_PCMA)
    {
        pstRtpType->u8PayloadType = 0x08;   //ITU-T G.711 PCM A-Law audio 64 kbit/s
    }
    else if (pstEsPkt->u8EncFormat == VA_AUDIO_TYPE_PCMU )
    {
        pstRtpType->u8PayloadType = 0x00;   //ITU-T G.711 PCM ¦Ì-Law audio 64 kbit/s
    }
    else
    {
        // using default
    }

    pstRtpType->u32TimeStamp = (pstVc3984Cb->stVideoType.u32TimeStamp / 90) * 8;
    VC_InitRtpHdr(&stRtpHdr, pstRtpType->u8PayloadType, pstVc3984Cb->u32Ssrc, pstRtpType->u32TimeStamp);

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
                stRtpHdr.u8Marker = 1; // fix me: audio this flag is mute funcgion
            }

            VC_ES2RtpOutputData(pstVcCb, pstRtpType, &stRtpHdr, pu8Data, u16PktLen);
            u16PktLen = 0;
        }
    }

    return VA_SUCCESS;
}

U16 VC_EsTo3984FindHdrSyn(U8 *pu8Data, U16 u16PktLen, U16 *pu16SynHdrLen)
{
    U16 u16SynPos = 0;

    *pu16SynHdrLen = 0;
    if ( u16PktLen <= 3 ) // must have one data
    {
        return u16PktLen;
    }

    /* 00 00 00 01 or 00 00 01 */
    while ( u16PktLen  >= (4 + u16SynPos) )
    {
        if ((0 == pu8Data[0]) && (0 == pu8Data[1]) && (1 == pu8Data[2]))
        {
            u16SynPos      += 3;
            *pu16SynHdrLen  = 3;
            return u16SynPos;
        }
        else if ((0 == pu8Data[0]) && (0 == pu8Data[1]) && (0 == pu8Data[2]) && (1 == pu8Data[3]))
        {
            u16SynPos      += 4;
            *pu16SynHdrLen  = 4;
            return u16SynPos;
        }

        pu8Data++;
        u16SynPos++;
    }

    *pu16SynHdrLen = 0;
    return u16PktLen;
}

VOID VC_EsTo3984OutputData(VC_CB_S *pstVcCb, VC_RTP_TYPE_S *pstRtpType, VA_RTP_HDR_S *pstRtpHdr, U8  *pu8Data, U16 u16PktLen, BOOL bEnd)
{
    VC_ES_TO_3984_CB_S *pstVc3984Cb = VA_PTR_TYPE(VC_ES_TO_3984_CB_S, pstVcCb);
    VC_ES_BUF_TYPE_S *pstBufType = &pstRtpType->stBufType;
    U8  au8TmpBuf[2];

    while (u16PktLen > 0)
    {
        pstRtpType->u16SeqId++;
        pstRtpHdr->be16SeqNo = DRV_CPU_TO_BE16(pstRtpType->u16SeqId);

        if ( u16PktLen > VC_3984_PKT_LEN )
        {
            au8TmpBuf[0] = pstVc3984Cb->u8Nal;
            au8TmpBuf[1] = pstVc3984Cb->u8Fua;
            pstVcCb->pfnOutputData(pstVcCb->pPrivCb, pstBufType->u8DataType, pstRtpHdr, sizeof(*pstRtpHdr));
            pstVcCb->pfnOutputData(pstVcCb->pPrivCb, pstBufType->u8DataType, au8TmpBuf, 2);
            pstVcCb->pfnOutputData(pstVcCb->pPrivCb, pstBufType->u8DataType, pu8Data, VC_3984_PKT_LEN);
            VC_FlushEsData(pstVcCb, pstBufType);
            u16PktLen -= VC_3984_PKT_LEN;
            pu8Data   += VC_3984_PKT_LEN;
        }
        else
        {
            if ( bEnd )
            {
                pstRtpHdr->u8Marker = 1;
                pstVc3984Cb->u8Fua |= 0x40;
            }

            au8TmpBuf[0] = pstVc3984Cb->u8Nal;
            au8TmpBuf[1] = pstVc3984Cb->u8Fua;

            pstVcCb->pfnOutputData(pstVcCb->pPrivCb, pstBufType->u8DataType, pstRtpHdr, sizeof(*pstRtpHdr));
            pstVcCb->pfnOutputData(pstVcCb->pPrivCb, pstBufType->u8DataType, au8TmpBuf, 2);
            pstVcCb->pfnOutputData(pstVcCb->pPrivCb, pstBufType->u8DataType, pu8Data, u16PktLen);
            VC_FlushEsData(pstVcCb, pstBufType);
            u16PktLen = 0;
        }

        pstVc3984Cb->u8Fua &= 0x7f;
    }
}


LONG VC_ESTo3984Video(VC_CB_S *pstVcCb, ES_PKT_HDR_S *pstEsPkt, U32 u32Len, VC_RTP_TYPE_S *pstRtpType)
{
    VC_ES_TO_3984_CB_S *pstVc3984Cb;
    VA_RTP_HDR_S stRtpHdr;
    U8  *pu8Data;
    U16 u16PktLen;
    U16 u16SynPos;
    U16 u16SynHdrLen;
    U16 u16UnitLen;

    if ( pstRtpType->stBufType.bDrop && !(VA_IS_FIRST_PKT(pstEsPkt) && VA_IS_IFRAME(pstEsPkt)) )
    {
        return VC_E_DROP;
    }

    pstRtpType->stBufType.bDrop = false;

    pstVc3984Cb = VA_PTR_TYPE(VC_ES_TO_3984_CB_S, pstVcCb);
    u16PktLen   = DRV_LE16_TO_CPU(pstEsPkt->le16PktLen);
    pu8Data     = (U8 *)(pstEsPkt->au8Data);

    if ( VA_IS_FIRST_PKT(pstEsPkt) )
    {
        pstRtpType->u32TimeStamp = DRV_LE32_TO_CPU(pstEsPkt->le32PTS);
        pstRtpType->u32TimeStamp = pstRtpType->u32TimeStamp << 1;
    }

    VC_InitRtpHdr(&stRtpHdr, pstRtpType->u8PayloadType, pstVc3984Cb->u32Ssrc, pstRtpType->u32TimeStamp);

    if ( VA_IS_FIRST_PKT(pstEsPkt) )
    {
        if ( VA_IS_IFRAME(pstEsPkt) )
        {
            u16SynPos = VC_EsTo3984FindHdrSyn(pu8Data, u16PktLen, &u16SynHdrLen);
            if ( u16PktLen <= u16SynPos )
            {
                return VC_E_INVALID;
            }

            for ( ; ; ) // first send pps sps packet
            {
                u16PktLen -= u16SynPos;
                pu8Data   += u16SynPos;

                /* is idr image data! */
                if ( u16PktLen > 1 && VC_3984_IDR_SLICE == (pu8Data[0] & 0x1f) )
                {
                    /*
                        The FU header has the following format:
                          +---------------+
                          |0|1|2|3|4|5|6|7|
                          +-+-+-+-+-+-+-+-+
                          |S|E|R|  Type   |
                          +---------------+            */

                    /* use fragment uint As format */
                    pstVc3984Cb->u8Nal = (pu8Data[0] & 0xe0) + 0x1C;
                    pstVc3984Cb->u8Fua = 0x80 | (pu8Data[0] & 0x1f);
                    u16PktLen--;
                    pu8Data++;
                    break;
                }

                u16SynPos = VC_EsTo3984FindHdrSyn(pu8Data, u16PktLen, &u16SynHdrLen);
                if ( u16PktLen <= u16SynPos )
                {
                    return VC_E_INVALID;
                }

                u16UnitLen = u16SynPos - u16SynHdrLen;
                VC_ES2RtpOutputData(pstVcCb, pstRtpType, &stRtpHdr, pu8Data, u16UnitLen);
            }
        }
        else
        {
            u16SynPos = VC_EsTo3984FindHdrSyn(pu8Data, u16PktLen, &u16SynHdrLen);
            if ( u16PktLen <= u16SynPos )
            {
                return VC_E_INVALID;
            }

            for ( ; ; )
            {
                u16PktLen -= u16SynPos;
                pu8Data   += u16SynPos;

                if ( u16PktLen > 1 && (pu8Data[0] & 0x1f) >= 6 )
                {
                    u16SynPos = VC_EsTo3984FindHdrSyn(pu8Data, u16PktLen, &u16SynHdrLen);
                    if ( u16PktLen <= u16SynPos )
                    {
                        return VC_E_INVALID;
                    }

                    u16UnitLen = u16SynPos - u16SynHdrLen;
                    VC_ES2RtpOutputData(pstVcCb, pstRtpType, &stRtpHdr, pu8Data, u16UnitLen);
                }
                else
                {
                    pstVc3984Cb->u8Nal = (pu8Data[0] & 0xe0) + 0x1C;
                    pstVc3984Cb->u8Fua = 0x80 | (pu8Data[0] & 0x1f);
                    u16PktLen--;
                    pu8Data++;
                    break;
                }
            }
        }
    }
    else
    {
        pstVc3984Cb->u8Fua &= 0x7f;
    }

    VC_EsTo3984OutputData(pstVcCb, pstRtpType, &stRtpHdr, pu8Data, u16PktLen, pstEsPkt->stInfo.u8BitEnd != 0);
    return VC_SUCCESS;
}

LONG VC_ESTo3984Convert(VC_CB_S *pstVcCb, void *pData, U32 u32Len)
{
    VC_ES_TO_3984_CB_S *pstVc3984Cb;
    ES_STOR_PKT_HDR_S *pstStorPktHdr;
    ES_PKT_HDR_S *pstEsPkt;
    LONG lRet;

    pstStorPktHdr = (ES_STOR_PKT_HDR_S *)pData;
    pstEsPkt      = &pstStorPktHdr->stEsPkt;
    pstVc3984Cb   = VA_PTR_TYPE(VC_ES_TO_3984_CB_S, pstVcCb);

    if ( VA_IS_VIDEO_PKT(pstEsPkt) )
    {
        lRet = VC_ESTo3984Video(pstVcCb, pstEsPkt, u32Len, &pstVc3984Cb->stVideoType);
    }
    else
    {
        lRet = VC_ESTo3984Audio(pstVcCb, pstEsPkt, u32Len, &pstVc3984Cb->stVideoType);
    }

    return lRet;
}

LONG VC_EsTo3984Init(VC_CB_S *pstVcCb)
{
    VC_ES_TO_3984_CB_S *pstVcEsTo3984Cb;

    pstVcEsTo3984Cb = VA_PTR_TYPE(VC_ES_TO_3984_CB_S, pstVcCb);
    VC_Es2RtpInitType(&pstVcEsTo3984Cb->stAudioType, VC_RTP_PT_AUDIO, VA_BUF_TYPE_AUDIO);
    VC_Es2RtpInitType(&pstVcEsTo3984Cb->stVideoType, VC_RTP_PT_H264,  VA_BUF_TYPE_VIDEO);

    return VC_SUCCESS;
}

VOID VC_EsTo3984Destroy(VC_CB_S *pstVcCb)
{
    return;
}

VCER_S gstEsTo3984Vcer =
{
    .u16InDataType  = VA_STREAM_TYPE_ES,
    .u16OutDataType = VA_STREAM_TYPE_RFC3984,
    .u32CbSize      = sizeof(VC_ES_TO_3984_CB_S),
    .pfnInit        = VC_EsTo3984Init,
    .pfnDestroy     = VC_EsTo3984Destroy,
    .pfnConvertProc = VC_ESTo3984Convert,
};

static void VCER_ES_TO_3984_Exit(void)
{
}

static int VCER_ES_TO_3984_Init(VA_MOD_S *pstMod)
{
    int iRet = 0;

    VC_RegVcer(&gstEsTo3984Vcer);
    return iRet;
}

VA_MOD_INIT(esto3984, VCER_ES_TO_3984_Init, VCER_ES_TO_3984_Exit, VA_INIT_LEVEL_MISC)

