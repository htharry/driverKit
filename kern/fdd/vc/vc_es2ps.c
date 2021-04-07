#include "vc_es2ps.h"

extern U32 VA_Crc32(IN U8 *pu8Data, IN U32 u32Len);

/* GB28121标准
MPEG-4 视频流:      0x10
H.264  视频流   :   0x1B
H.265  视频流   :   0x24
SVAC   视频流:      0x80

G.711   音频流:      0x90
G.722.1 音频流:      0x92
G.723.1 音频流:      0x93
G.729   音频流:      0x99
SVAC    音频流:      0x9B
*/

/*
 unkown
 MPEG2  0x02
 MPEG4  0x10
 H264   0x1b
 H265   0x24
 */
static const U8 gu8PsVideoStreamType[] = {0x00, 0x02, 0x10, 0x1b, 0x24};

/*
 unkown
 G711A
 G711U
 */
static const U8 gu8PsAudioStreamType[] = {0x00, 0x90, 0x90};


static VOID VC_Es2PsOutputData(VC_ES2PS_CB_S *pstVcPsCb, VC_PS_BUF_TYPE_S *pstBufType, VA_RTP_HDR_S *pstRtpHdr, U8 *pu8Data, U32 u32Len)
{
    // output packet!
    VC_RTP_TYPE_S *pstRtpType = &pstBufType->stRtpType;
    VC_ES_BUF_TYPE_S *pstEsBufType = &(pstRtpType->stBufType);
    VC_CB_S *pstVcCb = &pstVcPsCb->stVcCb;

    pstRtpType->u16SeqId++;
    pstRtpHdr->be16SeqNo = DRV_CPU_TO_BE16(pstRtpType->u16SeqId);

    pstVcCb->pfnOutputData(pstVcCb->pPrivCb, pstEsBufType->u8DataType, pstRtpHdr, sizeof(VA_RTP_HDR_S));
    pstVcCb->pfnOutputData(pstVcCb->pPrivCb, pstEsBufType->u8DataType, pu8Data, u32Len);

    VC_FlushEsData(pstVcCb, pstEsBufType);

    return;
}

static VOID VC_Es2PsMakePesPkt(VC_ES2PS_CB_S *pstVcPsCb, VC_PS_BUF_TYPE_S *pstBufType, VA_RTP_HDR_S *pstRtpHdr, U8 *pu8Data, U32 u32Len, ES_PKT_HDR_S *pstEsPkt)
{
    VC_RTP_TYPE_S *pstRtpType = &pstBufType->stRtpType;
    VC_ES_BUF_TYPE_S *pstEsBufType = &(pstRtpType->stBufType);
    VC_CB_S *pstVcCb = &pstVcPsCb->stVcCb;

    VC_FillPes(pstVcPsCb->au8TmpBuf, pstEsPkt, pstBufType->u8StreamId);
    VC_SET_BUF_U16(pstVcPsCb->au8TmpBuf + 4, u32Len + VC_PES_LEN - 6);

    pstRtpType->u16SeqId++;
    pstRtpHdr->be16SeqNo = DRV_CPU_TO_BE16(pstRtpType->u16SeqId);
    pstVcCb->pfnOutputData(pstVcCb->pPrivCb, pstEsBufType->u8DataType, pstRtpHdr, sizeof(VA_RTP_HDR_S));
    pstVcCb->pfnOutputData(pstVcCb->pPrivCb, pstEsBufType->u8DataType, pstVcPsCb->au8TmpBuf, VC_PES_LEN);
    pstVcCb->pfnOutputData(pstVcCb->pPrivCb, pstEsBufType->u8DataType, pu8Data, u32Len);
    VC_FlushEsData(pstVcCb, pstEsBufType);
}

static VOID VC_Es2PsOutputPkts(VC_ES2PS_CB_S *pstVcPsCb, VC_PS_BUF_TYPE_S *pstBufType, VA_RTP_HDR_S *pstRtpHdr, ES_PKT_HDR_S *pstEsPkt)
{
    U32 u32LeftLen = VA_ES_PKT_LEN(pstEsPkt);
    U8 *pu8Data = pstEsPkt->au8Data;

    while (u32LeftLen > 0)
    {
        if ( u32LeftLen > VC_PS_DATA_LEN )
        {
            VC_Es2PsMakePesPkt(pstVcPsCb, pstBufType, pstRtpHdr, pu8Data, VC_PS_DATA_LEN, pstEsPkt);
            pu8Data    += VC_PS_DATA_LEN;
            u32LeftLen -= VC_PS_DATA_LEN;
        }
        else
        {
            if ( pstEsPkt->stInfo.u8BitEnd )
            {
                pstRtpHdr->u8Marker = 1;
            }

            VC_Es2PsMakePesPkt(pstVcPsCb, pstBufType, pstRtpHdr, pu8Data, u32LeftLen, pstEsPkt);
            u32LeftLen = 0;
        }
    }

    return;
}

static VOID VC_Es2PsMakeMapHdr(VC_ES2PS_CB_S *pstVcPsCb, VC_PS_BUF_TYPE_S *pstBufType, VA_RTP_HDR_S *pstRtpHdr)
{
    VC_PS_BUF_TYPE_S *pstTmpBufType;
    U8  *pu8Buf = pstVcPsCb->au8TmpBuf;
    U32 u32Tmp;

    VC_PS_SET_BUF_U32(pu8Buf, 0x000001BC); // start code and map stream id
    pu8Buf += 4;
    VC_PS_SET_BUF_U16(pu8Buf, VC_PS_MAP_HDR_LEN - 6); // program stream map len
    pu8Buf += 2;
    *pu8Buf++ = 0xE0; // current_next_indicator + program_stream_map_version = 0
    *pu8Buf++ = 0xFF;

    VC_PS_SET_BUF_U16(pu8Buf, 0);   // programe_stream_info_len
    pu8Buf += 2;
    VC_PS_SET_BUF_U16(pu8Buf, 8);   // elementary_stream_map_len
    pu8Buf += 2;

    /*
    MPEG-4 视频流:      0x10
    H.264  视频流   :   0x1B
    H.265  视频流   :   0x24
    SVAC   视频流:      0x80

    G.711   音频流:      0x90
    G.722.1 音频流:      0x92
    G.723.1 音频流:      0x93
    G.729   音频流:      0x99
    SVAC    音频流:      0x9B
    */

    // Audio
    pstTmpBufType = &pstVcPsCb->stAudioBuf;
    *pu8Buf++ = pstTmpBufType->pu8StreamTypeMap[pstTmpBufType->stRtpType.stBufType.u8EncFmtType];   // stream_type
    *pu8Buf++ = pstTmpBufType->u8StreamId;  // elementary_stream_id
    VC_PS_SET_BUF_U16(pu8Buf, 0);           // elementary_stream_info_len
    pu8Buf += 2;

    // Video
    pstTmpBufType = &pstVcPsCb->stVideoBuf;
    *pu8Buf++ = pstTmpBufType->pu8StreamTypeMap[pstTmpBufType->stRtpType.stBufType.u8EncFmtType];   // stream_type
    *pu8Buf++ = pstTmpBufType->u8StreamId;  // elementary_stream_id
    VC_PS_SET_BUF_U16(pu8Buf, 0);   // elementary_stream_info_len
    pu8Buf += 2;

    u32Tmp = VA_Crc32(pstVcPsCb->au8TmpBuf, VC_PS_MAP_HDR_LEN - 4);
    VC_SET_BUF_U32(pu8Buf, u32Tmp); // crc32
    pu8Buf += 4;

    VC_Es2PsOutputData(pstVcPsCb, pstBufType, pstRtpHdr, pstVcPsCb->au8TmpBuf, pu8Buf - pstVcPsCb->au8TmpBuf);
    return;
}

static VOID VC_Es2PsMakeSysHdr(VC_ES2PS_CB_S *pstVcPsCb, VC_PS_BUF_TYPE_S *pstBufType, VA_RTP_HDR_S *pstRtpHdr)
{
    U8  *pu8Buf = pstVcPsCb->au8TmpBuf;
    U32 u32Tmp;

    VC_PS_SET_BUF_U32(pu8Buf, 0x000001BB); // start code
    pu8Buf += 4;
    VC_PS_SET_BUF_U16(pu8Buf, VC_PS_SYS_HDR_LEN - 6); // 指出该字段后的系统标题的字节长度
    pu8Buf += 2;

    u32Tmp = (0x1 << 23) | (70000 << 1) | 0x1; //rate_bound and marker bit
    VC_PS_SET_BUF_U24(pu8Buf, u32Tmp);
    pu8Buf += 3;

    *pu8Buf++ = (0x1 << 2) + (0x0 << 1) + 0x1; // audio_bound and fixed_flag = 0 and CSPS_flag = 1
    *pu8Buf++ = (0x1 << 7) + (0x1 << 6) + (0x1 << 5) + 0x1; // system_audio_lock_flag + system_video_lock_flag + video_bound
    *pu8Buf++ = (0x0 << 7) + 0x7F; //packet_rate_restriction_flag

    // audio stream bound : GB/T XXXX.3或GB/T AAAA.3音频流编号
    // PSTD_buffer_bound_scale = 0 PSTD_buffer_size_bound (128B unit)
    u32Tmp = (0xC0 << 16) + (0x3 << 14) + (0x0 << 13) + 512;
    VC_PS_SET_BUF_U24(pu8Buf, u32Tmp);
    pu8Buf += 3;

    // video stream bound : GB/T XXXX.2或GB/T AAAA.2视频流编号
    // PSTD_buffer_bound_scale(video) = 1 PSTD_buffer_size_bound (1024B unit)
    u32Tmp = (0xE0 << 16) + (0x3 << 14) + (0x1 << 13) + 512;
    VC_PS_SET_BUF_U24(pu8Buf, u32Tmp);
    pu8Buf += 3;

    VC_Es2PsOutputData(pstVcPsCb, pstBufType, pstRtpHdr, pstVcPsCb->au8TmpBuf, pu8Buf - pstVcPsCb->au8TmpBuf);
    return;
}


static VOID VC_Es2PsScr(U8 *pu8Buf, ES_PKT_HDR_S *pstEsPkt)
{
    // System Clock Reference, 90K, ext is zero!
    U32 u32Pts = DRV_LE32_TO_CPU(pstEsPkt->le32PTS);

    pu8Buf[0] = 0x44 | ((u32Pts >> 29) << 3) | ((u32Pts >> 27) & 0x3);
    pu8Buf[1] = (u32Pts >> 19) & 0xFF;
    pu8Buf[2] = 0x4 | (((u32Pts >> 14) & 0x1F) << 3) | ((u32Pts >> 12) & 0x3);
    pu8Buf[3] = (u32Pts >> 4) & 0xFF;
    pu8Buf[4] = 0x4 | ((u32Pts & 0xF) << 4);
    pu8Buf[5] = 1;
}

static VOID VC_Es2PsMakePsHdr(VC_ES2PS_CB_S *pstVcPsCb, VC_PS_BUF_TYPE_S *pstBufType, VA_RTP_HDR_S *pstRtpHdr, ES_PKT_HDR_S *pstEsPkt)
{
    U8  *pu8Buf = pstVcPsCb->au8TmpBuf;

    VC_PS_SET_BUF_U32(pu8Buf, 0x000001BA); // start code
    pu8Buf += 4;
    VC_Es2PsScr(pu8Buf, pstEsPkt);
    pu8Buf += 6;

    *pu8Buf++ = 0;
    *pu8Buf++ = 3;
    *pu8Buf++ = 0xFF; // bit rate(n units of 50 bytes per second.)
    *pu8Buf++ = 0xF8; // stuffing length is zero

    VC_Es2PsOutputData(pstVcPsCb, pstBufType, pstRtpHdr, pstVcPsCb->au8TmpBuf, pu8Buf - pstVcPsCb->au8TmpBuf);
    return;
}

static LONG VC_Es2PsAudio(VC_CB_S *pstVcCb, VC_PS_BUF_TYPE_S *pstAudioBuf, ES_PKT_HDR_S *pstEsPkt)
{
    VC_ES2PS_CB_S *pstVcPsCb;
    VC_RTP_TYPE_S *pstRtpType;
    VA_RTP_HDR_S  stRtpHdr;

    pstVcPsCb  = VA_PTR_TYPE(VC_ES2PS_CB_S, pstVcCb);
    pstRtpType = &pstAudioBuf->stRtpType;

    pstRtpType->u32TimeStamp = DRV_LE32_TO_CPU(pstEsPkt->le32PTS);
    pstRtpType->u32TimeStamp = pstRtpType->u32TimeStamp << 1;

    VC_InitRtpHdr(&stRtpHdr, pstRtpType->u8PayloadType, pstVcPsCb->u32Ssrc, pstRtpType->u32TimeStamp);

    VC_Es2PsMakePsHdr(pstVcPsCb,  pstAudioBuf, &stRtpHdr, pstEsPkt);
    VC_Es2PsOutputPkts(pstVcPsCb, pstAudioBuf, &stRtpHdr, pstEsPkt);

    return VC_SUCCESS;
}

VOID VC_Es2PsInitVideoRtpPt(VC_RTP_TYPE_S *pstRtpType, VC_ES_BUF_TYPE_S *pstEsBufType)
{
    if ( pstEsBufType->u8EncFmtType == VA_ES_TYPE_H265 )
    {
        pstRtpType->u8PayloadType = VC_RTP_PT_H265;
    }
    else if ( pstEsBufType->u8EncFmtType == VA_ES_TYPE_H264 )
    {
        pstRtpType->u8PayloadType = VC_RTP_PT_H264;
    }
}

static LONG VC_Es2PsVideo(VC_CB_S *pstVcCb, VC_PS_BUF_TYPE_S *pstVideoBuf, ES_PKT_HDR_S *pstEsPkt)
{
    VC_ES_BUF_TYPE_S *pstEsBufType;
    VC_ES2PS_CB_S *pstVcPsCb;
    VC_RTP_TYPE_S *pstRtpType;
    VA_RTP_HDR_S  stRtpHdr;

    pstVcPsCb    = VA_PTR_TYPE(VC_ES2PS_CB_S, pstVcCb);
    pstRtpType   = &pstVideoBuf->stRtpType;
    pstEsBufType = &pstRtpType->stBufType;

    if ( VC_CheckEsEncFormat(pstEsBufType, pstEsPkt) != VA_SUCCESS )
    {
        pstEsBufType->bDrop = TRUE;
        return VC_E_DROP;
    }

    if ( pstEsBufType->bDrop && !(VA_IS_FIRST_PKT(pstEsPkt) && VA_IS_IFRAME(pstEsPkt)) )
    {
        return VC_E_DROP;
    }

    if ( VA_IS_FIRST_PKT(pstEsPkt) )
    {
        pstRtpType->u32TimeStamp = DRV_LE32_TO_CPU(pstEsPkt->le32PTS);
        pstRtpType->u32TimeStamp = pstRtpType->u32TimeStamp << 1;
    }

    VC_Es2PsInitVideoRtpPt(pstRtpType, pstEsBufType);
    VC_InitRtpHdr(&stRtpHdr, pstRtpType->u8PayloadType, pstVcPsCb->u32Ssrc, pstRtpType->u32TimeStamp);

    if ( VA_IS_FIRST_PKT(pstEsPkt) )
    {
        if ( VA_IS_IFRAME(pstEsPkt) )
        {
            pstEsBufType->bDrop = FALSE;
            pstEsBufType->u32FrameSeq   = DRV_LE32_TO_CPU(pstEsPkt->le32FrmSeq);
            pstEsBufType->u16NextPktSeq = DRV_LE16_TO_CPU(pstEsPkt->le16PktSeq) + 1;

            VC_Es2PsMakePsHdr(pstVcPsCb, pstVideoBuf, &stRtpHdr, pstEsPkt);
            VC_Es2PsMakeSysHdr(pstVcPsCb, pstVideoBuf, &stRtpHdr);
            VC_Es2PsMakeMapHdr(pstVcPsCb, pstVideoBuf, &stRtpHdr);
        }
        else
        {
            pstEsBufType->u32FrameSeq++;
            pstEsBufType->u16NextPktSeq = 0;
            if ( VC_ChkEsPkt(pstEsBufType, pstEsPkt) != VC_SUCCESS )
            {
                pstEsBufType->bDrop = TRUE;
                return VC_E_DROP;
            }

            VC_Es2PsMakePsHdr(pstVcPsCb, pstVideoBuf, &stRtpHdr, pstEsPkt);
        }

        VC_Es2PsOutputPkts(pstVcPsCb, pstVideoBuf, &stRtpHdr, pstEsPkt);
    }
    else
    {
        if ( VC_ChkEsPkt(pstEsBufType, pstEsPkt) != VC_SUCCESS )
        {
            pstEsBufType->bDrop = TRUE;
            return VC_E_DROP;
        }

        VC_Es2PsOutputPkts(pstVcPsCb, pstVideoBuf, &stRtpHdr, pstEsPkt);
    }

    return VC_SUCCESS;
}

static LONG VC_Es2PsConvert(VC_CB_S *pstVcCb, void *pData, U32 u32Len)
{
    VC_ES2PS_CB_S *pstVcPsCb;
    ES_STOR_PKT_HDR_S *pstStorPktHdr;
    ES_PKT_HDR_S *pstEsPkt;
    LONG lRet;

    pstStorPktHdr = (ES_STOR_PKT_HDR_S *)pData;
    pstEsPkt      = &pstStorPktHdr->stEsPkt;
    pstVcPsCb     = VA_PTR_TYPE(VC_ES2PS_CB_S, pstVcCb);

    if ( VA_IS_VIDEO_PKT(pstEsPkt) )
    {
        lRet = VC_Es2PsVideo(pstVcCb, &pstVcPsCb->stVideoBuf, pstEsPkt);
    }
    else
    {
        lRet = VC_Es2PsAudio(pstVcCb, &pstVcPsCb->stAudioBuf, pstEsPkt);
    }

    if ( lRet != VC_SUCCESS )
    {
        VA_LOG_INFO("error, u32Len = %u", u32Len);
    }

    return lRet;
}

static VOID VC_Es2PsDestroy(VC_CB_S *pstVcCb)
{
    return;
}

static LONG VC_Es2PsInit(VC_CB_S *pstVcCb)
{
    VC_PS_BUF_TYPE_S *pstBufType;
    VC_ES2PS_CB_S *pstVcPsCb;
    VC_RTP_TYPE_S *pstRtpType;

    pstVcPsCb  = (VC_ES2PS_CB_S *)pstVcCb;
    pstBufType = &pstVcPsCb->stVideoBuf;
    pstRtpType = &pstBufType->stRtpType;
    //VC_InitEsBufType(&pstBufType->stBufType, VA_ES_TYPE_H265, VA_BUF_TYPE_VIDEO);
    VC_InitEsBufType(&pstRtpType->stBufType, VA_ES_TYPE_H265, VA_BUF_TYPE_VIDEO);
    pstBufType->pu8StreamTypeMap = gu8PsVideoStreamType;
    pstBufType->u8StreamId    = 0xE0;
    pstRtpType->u8PayloadType = VC_RTP_PT_H265;

    pstBufType = &pstVcPsCb->stAudioBuf;
    pstRtpType = &pstBufType->stRtpType;
    //VC_InitEsBufType(&pstBufType->stBufType, VA_AUDIO_TYPE_PCMA, VA_BUF_TYPE_AUDIO);
    VC_InitEsBufType(&pstRtpType->stBufType, VA_AUDIO_TYPE_PCMA, VA_BUF_TYPE_AUDIO);
    pstBufType->pu8StreamTypeMap = gu8PsAudioStreamType;
    pstBufType->u8StreamId    = 0xC0;
    pstRtpType->u8PayloadType = VC_RTP_PT_AUDIO;

    pstVcPsCb->u32Ssrc = 0x1111;
    return VC_SUCCESS;
}

VCER_S gstEsToPsVcer =
{
    .u16InDataType  = VA_STREAM_TYPE_ES,
    .u16OutDataType = VA_STREAM_TYPE_PS,
    .u32CbSize      = sizeof(VC_ES2PS_CB_S),
    .pfnInit        = VC_Es2PsInit,
    .pfnDestroy     = VC_Es2PsDestroy,
    .pfnConvertProc = VC_Es2PsConvert,
};

static void VCER_ES_TO_PS_Exit(void)
{
}

static int VCER_ES_TO_PS_Init(VA_MOD_S *pstMod)
{
    int iRet = 0;

    VC_RegVcer(&gstEsToPsVcer);
    return iRet;
}

VA_MOD_INIT(estops, VCER_ES_TO_PS_Init, VCER_ES_TO_PS_Exit, VA_INIT_LEVEL_MISC)

