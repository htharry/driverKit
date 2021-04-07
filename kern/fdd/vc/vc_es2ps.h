#ifndef __VC_ES2PS_H__
#define __VC_ES2PS_H__

#include "vc_comm.h"
#include "vc_es2rtp.h"

#define VC_PS_HDR_LEN           14
#define VC_PS_SYS_HDR_LEN       18
#define VC_PS_MAP_HDR_LEN       24

#define VC_PS_SET_BUF_U32(pu8Buf, u32Val) \
    do { \
        (pu8Buf)[0] = ((u32Val) >> 24) & 0xff;\
        (pu8Buf)[1] = ((u32Val) >> 16) & 0xff;\
        (pu8Buf)[2] = ((u32Val) >> 8)  & 0xff;\
        (pu8Buf)[3] = (u32Val) & 0xff;\
    }while(0)

#define VC_PS_SET_BUF_U24(pu8Buf, u32Val) \
    do { \
        (pu8Buf)[0] = ((u32Val) >> 16) & 0xff;\
        (pu8Buf)[1] = ((u32Val) >> 8) & 0xff;\
        (pu8Buf)[2] = (u32Val) & 0xff;\
    }while(0)

#define VC_PS_SET_BUF_U16(pu8Buf, u16Val) \
    do { \
        (pu8Buf)[0] = ((u16Val) >> 8)  & 0xff;\
        (pu8Buf)[1] = (u16Val) & 0xff;\
    }while(0)

typedef struct tagVcPsBufType
{
    //VC_ES_BUF_TYPE_S stBufType;
    VC_RTP_TYPE_S    stRtpType;
    const U8  *pu8StreamTypeMap;
    U8 u8StreamId;
}VC_PS_BUF_TYPE_S;

typedef struct tagVcEs2Ps
{
    VC_CB_S stVcCb;
    VC_PS_BUF_TYPE_S stVideoBuf;
    VC_PS_BUF_TYPE_S stAudioBuf;
    U8  au8TmpBuf[256];
    U32 u32Ssrc;
}VC_ES2PS_CB_S;


#endif //__VC_ES2PS_H__
