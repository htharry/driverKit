#ifndef __VC_RTP_DEF_H__
#define __VC_RTP_DEF_H__

#pragma pack (4)

/*
 * RTP头信息定义
 */
typedef struct tagVaRtpHdr
{
#ifdef DRV_BIG_END
    U8 u8Ver:2;
    U8 u8Padding:1;
    U8 u8Ext:1;
    U8 u8Cc:4;
    U8 u8Marker:1;
    U8 u8Payload:7;
#else /* little endian */
    U8 u8Cc:4;
    U8 u8Ext:1;
    U8 u8Padding:1;
    U8 u8Ver:2;
    U8 u8Payload:7;
    U8 u8Marker:1;
#endif

    BE16  be16SeqNo;
    BE32  be32Timestamp;
    BE32  be32SSRC;
}VA_RTP_HDR_S;

#pragma pack ()

#endif //__VC_RTP_DEF_H__
