#ifndef __VC_ES2_3984_H__
#define __VC_ES2_3984_H__

#include "vc_es2rtp.h"
#include "vc_rtp_def.h"

#define VC_3984_IDR_SLICE       5
#define VC_3984_PKT_LEN         (VC_RTP_MAX_LEN - 2)
typedef struct tagVcEsTo3984Cb
{
    VC_CB_S stVcCb;
    U32     u32Ssrc;
    U8      u8Nal;
    U8      u8Fua;
    VC_RTP_TYPE_S stVideoType;
    VC_RTP_TYPE_S stAudioType;
}VC_ES_TO_3984_CB_S;


#endif //__VC_ES2_3984_H__
