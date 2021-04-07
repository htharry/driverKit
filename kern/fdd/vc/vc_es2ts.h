#ifndef __VC_ES2TS_H__
#define __VC_ES2TS_H__

#include "vc_comm.h"
#include "va_kern_pub.h"
#include "vc_ts_def.h"
#include "vc_mgr.h"

#define VC_TS_PCR_LEN               6
#define VC_TS_PES_LEN               19
#define VC_TS_HDR_AND_ADAPT_LEN     12
#define VC_TS_MID_HDR_LEN           6
#define VC_TS_MAX_DATA_LEN          (VC_TS_PKT_LEN - VC_TS_MID_HDR_LEN)
#define VC_TS_NET_PKT_LEN           (VC_TS_PKT_LEN*7)
#define VC_TS_PMT_PID               0x83
#define VC_TS_PAT_OFF               5
#define VC_TS_PMT_OFF               5
#define VC_TS_PMT_SEC_LEN_OFF       (VC_TS_PMT_OFF + 2)

#define VC_TS_SET_BUF_U32(u32Val, pu8Buf) \
    do { \
        (pu8Buf)[0] = ((u32Val) >> 24) & 0xff;\
        (pu8Buf)[1] = ((u32Val) >> 16) & 0xff;\
        (pu8Buf)[2] = ((u32Val) >> 8)  & 0xff;\
        (pu8Buf)[3] = (u32Val) & 0xff;\
    }while(0)

#define VC_TS_SET_BUF_U16(u16Val, pu8Buf) \
    do { \
        (pu8Buf)[0] = ((u16Val) >> 8)  & 0xff;\
        (pu8Buf)[1] = (u16Val) & 0xff;\
    }while(0)

typedef struct tagVcTsBufType
{
    VC_ES_BUF_TYPE_S stBufType;
    const U16 *pu16PmtMap;
    const U16 *pu16PidMap;
    const U8  *pu8StreamTypeMap;
    U8  au8LeftData[VC_TS_PKT_LEN];
    U16 u32LeftDataLen;
    U32 u32TsPktSeq;
}VC_TS_BUF_TYPE_S;

typedef struct tagVcEs2Ts
{
    VC_CB_S stVcCb;
    VC_TS_BUF_TYPE_S stVideoBuf;
    VC_TS_BUF_TYPE_S stAudioBuf;
    U32 u32PatCrc;
    U32 u32PmtCrc;
    U32 u32PatSeq;
    U32 u32PmtSeq;
    ULONG ulAudioPktCnt;
}VC_ES2TS_CB_S;


#endif //__VC_ES2TS_H__
