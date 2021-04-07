#include "vc_comm.h"

VOID VC_FillPtsAndDts(ES_PKT_HDR_S *pstEsPkt, U8 *pu8Buf)
{
    U32 u32Dts = DRV_LE32_TO_CPU(pstEsPkt->le32DTS);
    U32 u32Pts = DRV_LE32_TO_CPU(pstEsPkt->le32PTS);

    //VA_LOG_INFO("DTS %u PTS %u", u32Dts, u32Pts);

    // pts high bit is 0x0011, mark bit is one
    pu8Buf[0] = (0x30 + ((u32Pts >> 28) & 0x0e) + 0x1);
    pu8Buf[1] = (u32Pts  >> 21) & 0xFF;
    pu8Buf[2] = ((u32Pts >> 13) & 0xfe) + 0x1;
    pu8Buf[3] = (u32Pts  >> 6) & 0xFF;
    pu8Buf[4] = ((u32Pts & 0x3f) << 2) + 0x0 + 0x1; // drop low bit to zero

    // pts high bit is 0x0001, mark bit is one
    pu8Buf[5] = (0x10 + ((u32Dts >> 28) & 0x0e) + 0x1);
    pu8Buf[6] = (u32Dts >> 21) & 0xFF;;
    pu8Buf[7] = ((u32Dts >> 13) & 0xfe) + 0x1;;
    pu8Buf[8] = (u32Dts  >> 6) & 0xFF;
    pu8Buf[9] = ((u32Dts & 0x3f) << 2) + 0x0 + 0x1; // drop low bit to zero

    //VA_LOG_INFO("%02x %02x %02x %02x %02x", pu8Buf[0], pu8Buf[1], pu8Buf[2], pu8Buf[3], pu8Buf[4]);
    return;
}


VOID VC_FillPes(U8 *pu8Buf, ES_PKT_HDR_S *pstEsPkt, U8 u8StreamId)
{
    /* PES */
    pu8Buf[0] = 0x00;
    pu8Buf[1] = 0x00;
    pu8Buf[2] = 0x01;       // packet_start_code_prefix(24b) 00 00 01
    pu8Buf[3] = u8StreamId; // stream id, video e0, audio c0
    pu8Buf[4] = 0x00;
    pu8Buf[5] = 0x00;       // pes length
    pu8Buf[6] = 0x80;
    pu8Buf[7] = 0xc0;       // set PTS_DTS_flags to '11'
    pu8Buf[8] = 0x0a;       // PES_header_data_length

    VC_FillPtsAndDts(pstEsPkt, pu8Buf + VC_PES_HDR_LEN);
}

LONG VC_CheckEsEncFormat(VC_ES_BUF_TYPE_S *pstBufType, ES_PKT_HDR_S *pstEsPkt)
{
    U16 u16PktLen;
    U8  *pu8Data;

    if ( !(VA_IS_VIDEO_PKT(pstEsPkt)  && VA_IS_IFRAME(pstEsPkt)) )
    {
        return VC_SUCCESS;
    }

    u16PktLen = DRV_LE16_TO_CPU(pstEsPkt->le16PktLen);
    if ( u16PktLen < 5 )
    {
        return VC_E_DROP;
    }

    pu8Data = (U8 *)(pstEsPkt + 1);

    if ( (pstEsPkt->u8EncFormat == VA_ES_TYPE_UNKOWN) && (pstBufType->u8EncFmtType == VA_ES_TYPE_H265) )
    {
        if ( pu8Data[0] != 0 || pu8Data[1] != 0 )
        {
            return VC_E_INVALID;
        }

        if ( pu8Data[2] == 1 )
        {
            // H264: SPS P SEI lookup
            if ( 0x07 == (pu8Data[3] & 0x1f) || 0x06 == (pu8Data[3] & 0x1f) || 0x61 == pu8Data[3] )
            {
                pstBufType->u8EncFmtType = VA_ES_TYPE_H264;
                return VC_SUCCESS;
            }
        }
        else if ( pu8Data[2] == 0 && pu8Data[3] == 1 )
        {
            if ( 0x07 == (pu8Data[4] & 0x1f) || 0x06 == (pu8Data[4] & 0x1f) || 0x61 == pu8Data[4] )
            {
                pstBufType->u8EncFmtType = VA_ES_TYPE_H264;
                return VC_SUCCESS;
            }
        }
   }
   else
   {
        if ( pstEsPkt->u8EncFormat != VA_ES_TYPE_UNKOWN )
        {
            pstBufType->u8EncFmtType = pstEsPkt->u8EncFormat;
            return VC_SUCCESS;
        }
   }

   return VC_E_INVALID;
}

LONG VC_ChkEsPkt(VC_ES_BUF_TYPE_S *pstBufType, ES_PKT_HDR_S *pstEsPkt)
{
    if ( (!VA_PKT_SEQ_IS_EQU(pstEsPkt, pstBufType->u16NextPktSeq)) || (!VA_PKT_FRM_IS_EQU(pstEsPkt, pstBufType->u32FrameSeq)) )
    {
        return VC_E_INVALID;
    }

    pstBufType->u16NextPktSeq++;
    return VC_SUCCESS;
}

VOID VC_InitRtpHdr(VA_RTP_HDR_S *pstRtpHdr, U8 u8PayLoad, U32 u32Ssrc, U32 u32TimeStamp)
{
    VA_CB_ZERO(pstRtpHdr);

    pstRtpHdr->u8Ver          = 2;
    pstRtpHdr->u8Payload      = u8PayLoad;
    pstRtpHdr->be32Timestamp  = DRV_CPU_TO_BE32(u32TimeStamp);
    pstRtpHdr->be32SSRC       = DRV_CPU_TO_BE32(u32Ssrc);
}

