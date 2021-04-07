#ifndef __VC_COMM_H__
#define __VC_COMM_H__

#include "va_kern_pub.h"
#include "vc_mgr.h"
#include "vc_rtp_def.h"
#include "vc_ts_def.h"

#define VC_PES_HDR_LEN              9
#define VC_PES_LEN                  19
#define VC_PES_VIDEO_STREAM_ID      0xE0
#define VC_PES_AUDIO_STREAM_ID      0xC0
#define VC_RTP_MAX_LEN              (VC_MAX_NET_PKT_LEN - sizeof(VA_RTP_HDR_S))
#define VC_PS_DATA_LEN              (VC_RTP_MAX_LEN - VC_PES_LEN)

// big endian fill
#define VC_SET_BUF_U32(pu8Buf, u32Val) \
    do { \
        (pu8Buf)[0] = ((u32Val) >> 24) & 0xff;\
        (pu8Buf)[1] = ((u32Val) >> 16) & 0xff;\
        (pu8Buf)[2] = ((u32Val) >> 8)  & 0xff;\
        (pu8Buf)[3] = (u32Val) & 0xff;\
    }while(0)

#define VC_SET_BUF_U24(pu8Buf, u32Val) \
    do { \
        (pu8Buf)[0] = ((u32Val) >> 16) & 0xff;\
        (pu8Buf)[1] = ((u32Val) >> 8) & 0xff;\
        (pu8Buf)[2] = (u32Val) & 0xff;\
    }while(0)

#define VC_SET_BUF_U16(pu8Buf, u16Val) \
    do { \
        (pu8Buf)[0] = ((u16Val) >> 8)  & 0xff;\
        (pu8Buf)[1] = (u16Val) & 0xff;\
    }while(0)

extern VOID VC_FillPtsAndDts(ES_PKT_HDR_S *pstEsPkt, U8 *pu8Buf);
extern VOID VC_FillPes(U8 *pu8Buf, ES_PKT_HDR_S *pstEsPkt, U8 u8StreamId);
extern LONG VC_CheckEsEncFormat(VC_ES_BUF_TYPE_S *pstBufType, ES_PKT_HDR_S *pstEsPkt);
extern LONG VC_ChkEsPkt(VC_ES_BUF_TYPE_S *pstBufType, ES_PKT_HDR_S *pstEsPkt);
extern VOID VC_InitRtpHdr(VA_RTP_HDR_S *pstRtpHdr, U8 u8PayLoad, U32 u32Ssrc, U32 u32TimeStamp);
#endif // __VC_COMM_H__
