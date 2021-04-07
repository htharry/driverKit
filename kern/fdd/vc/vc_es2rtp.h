#ifndef __VC_ES2RTP_H__
#define __VC_ES2RTP_H__

#include "vc_comm.h"
#include "vc_rtp_def.h"
#include "vc_mgr.h"

#define VC_RTP_PT_H264             96
#define VC_RTP_PT_ES               100
#define VC_RTP_PT_H265             108
#define VC_RTP_PT_RAW              50
#define VC_RTP_PT_AUDIO            160

typedef struct tagVcRtpType
{
    VC_ES_BUF_TYPE_S stBufType;
    U32 u32TimeStamp;
    U16 u16SeqId;
    U8  u8PayloadType;
}VC_RTP_TYPE_S;

typedef struct tagVcEs2RtpCb
{
    VC_CB_S stVcCb;
    U32     u32Ssrc;
    VC_RTP_TYPE_S stVideoType;
    VC_RTP_TYPE_S stAudioType;
    VC_RTP_TYPE_S stRawType;
}VC_ES2RTP_CB_S;

extern VOID VC_InitRtpHdr(VA_RTP_HDR_S *pstRtpHdr, U8 u8PayLoad, U32 u32Ssrc, U32 u32TimeStamp);
extern VOID VC_ES2RtpOutputData(VC_CB_S *pstVcCb, VC_RTP_TYPE_S *pstRtpType, VA_RTP_HDR_S *pstRtpHdr, VOID *pData, U32 u32Len);
extern VOID VC_Es2RtpInitType(VC_RTP_TYPE_S *pstRtpType, U8 u8PayLoadType, U8 u8DataType);

#endif //__VC_ES2RTP_H__
