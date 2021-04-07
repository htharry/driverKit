#ifndef __VA_MEDIA_DEF_H__
#define __VA_MEDIA_DEF_H__

#define VA_MAX_ES_PKT_LEN        (32 * 1024)

enum
{
    VA_I_FRAME     = 0x0,
    VA_P_FRAME     = 0x1,
    VA_B_FRAME     = 0x2,
};

enum
{
    VA_ES_TYPE_UNKOWN       = 0,
    VA_ES_TYPE_MPEG2        = 1,
    VA_ES_TYPE_MPEG4        = 2,
    VA_ES_TYPE_H264         = 3,
    VA_ES_TYPE_H265         = 4,
    VA_ES_TYPE_JPEG         = 50,
    VA_ES_TYPE_NULL,
};

enum
{
    VA_AUDIO_TYPE_UNKOWN    = 0,
    VA_AUDIO_TYPE_PCMA      = 1,
    VA_AUDIO_TYPE_PCMU      = 2,
    VA_AUDIO_TYPE_NULL
};

enum
{
    VA_BUF_TYPE_VIDEO  = 0,
    VA_BUF_TYPE_AUDIO,
    VA_BUF_TYPE_DATA,
    VA_BUF_TYPE_CTRL,
    VA_BUF_TYPE_NUM
};

#define VA_IS_VIDEO_PKT(pstEsPkt)           (VA_BUF_TYPE_VIDEO == (pstEsPkt)->stInfo.u8BitType)
#define VA_IS_AUDIO_PKT(pstEsPkt)           (VA_BUF_TYPE_AUDIO == (pstEsPkt)->stInfo.u8BitType)
#define VA_IS_FIRST_PKT(pstEsPkt)           (!!((pstEsPkt)->stInfo.u8BitFirst))
#define VA_IS_END_PKT(pstEsPkt)             (!!((pstEsPkt)->stInfo.u8BitEnd))
#define VA_IS_IFRAME(pstEsPkt)              (VA_I_FRAME == (pstEsPkt)->stInfo.u8BitFrm)
#define VA_SET_VIDEO_PKT(pstEsPkt)          do {(pstEsPkt)->stInfo.u8BitType = VA_BUF_TYPE_VIDEO;} while(0)
#define VA_SET_AUDIO_PKT(pstEsPkt)          do {(pstEsPkt)->stInfo.u8BitType = VA_BUF_TYPE_AUDIO;} while(0)
#define VA_SET_FIRST_PKT(pstEsPkt)          do {(pstEsPkt)->stInfo.u8BitFirst = 1;} while(0)
#define VA_SET_END_PKT(pstEsPkt)            do {(pstEsPkt)->stInfo.u8BitEnd = 1;} while(0)
#define VA_SET_IFRAME(pstEsPkt)             do {(pstEsPkt)->stInfo.u8BitFrm = VA_I_FRAME;} while(0)
#define VA_SET_PFRAME(pstEsPkt)             do {(pstEsPkt)->stInfo.u8BitFrm = VA_P_FRAME;} while(0)

#define VA_SET_PIC_SIZE(pstEsPkt, u8PicSize)        do {(pstEsPkt)->u8PictureSize = u8PicSize;} while(0)
#define VA_SET_FRM_RATE(pstEsPkt, u8FrameRate)      do {(pstEsPkt)->u8FrmRate = u8FrameRate;} while(0)
#define VA_SET_ENC_FMT(pstEsPkt,  u8EncFmt)         do {(pstEsPkt)->u8EncFormat = u8EncFmt;} while(0)
#define VA_SET_STREAM_TYPE(pstEsPkt, __u8StreamType)  do {(pstEsPkt)->u8StreamType = __u8StreamType;} while(0)

#define VA_SET_PKT_SEQ(pstEsPkt, u16Seq)        do {(pstEsPkt)->le16PktSeq = DRV_CPU_TO_LE16(u16Seq);} while(0)
#define VA_GET_PKT_SEQ(pstEsPkt, u16Seq)        do {u16Seq = DRV_LE16_TO_CPU((pstEsPkt)->le16PktSeq);} while(0)
#define VA_PKT_SEQ_IS_EQU(pstEsPkt, u16Seq)     ((U16)(u16Seq) == DRV_LE16_TO_CPU((pstEsPkt)->le16PktSeq))

#define VA_SET_FRM_SEQ(pstEsPkt, u32FrmSeq)     do {(pstEsPkt)->le32FrmSeq = DRV_CPU_TO_LE32(u32FrmSeq);} while(0)
#define VA_GET_FRM_SEQ(pstEsPkt, u32FrmSeq)     do {u32FrmSeq = DRV_LE32_TO_CPU((pstEsPkt)->le32FrmSeq);} while(0)
#define VA_PKT_FRM_IS_EQU(pstEsPkt, u32FrmSeq)  ((U32)(u32FrmSeq) == DRV_LE32_TO_CPU((pstEsPkt)->le32FrmSeq))

#define VA_SET_PKT_LEN(pstEsPkt, Len)           do {(pstEsPkt)->le16PktLen = DRV_CPU_TO_LE16(((U16)Len));} while(0)
#define VA_GET_PKT_LEN(pstEsPkt, Len)           do {Len = DRV_LE16_TO_CPU((pstEsPkt)->le16PktLen);} while(0)
#define VA_ES_PKT_LEN(pstEsPkt)                 DRV_LE16_TO_CPU((pstEsPkt)->le16PktLen)

#pragma pack (4)

typedef struct tagVaDataBuf
{
    VOID *pHead;
    VOID *pData;
    U32  u32Len;
}VA_DATA_BUF_S;

#ifdef DRV_LITTLE_END
typedef struct
{
    U8   u8BitType:1;       //fix me: 4 buf type, but it's two
    U8   u8BitFirst:1;
    U8   u8BitFrm:2;        //other type is always IFRAME
    U8   u8BitRsvd:3;
    U8   u8BitEnd:1;
}VA_PKT_BITS_INFO_S;
#else
typedef struct
{
    U8   u8BitEnd:1;
    U8   u8BitRsvd:3;
    U8   u8BitFrm:2;
    U8   u8BitFirst:1;
    U8   u8BitType:1;
}VA_PKT_BITS_INFO_S;
#endif /* DRV_LITTLE_END */

// operate this pkt hdr by macro or inline function, don't directly use this member or property
// it's maybe change
typedef struct tagEsPktHdr
{
    LE32  le32DTS;
    LE32  le32PTS;

    union
    {
        VA_PKT_BITS_INFO_S stInfo;
        U8 u8Info;
    };

    U8   u8FrmRate;
    U8   u8PictureSize;
    U8   u8EncFormat;
    U8   u8StreamType;
    U8   au8Rsvd[3];
    LE16 le16PktSeq;
    LE16 le16PktLen;        //fix me, it's to 32bit?
    LE32 le32FrmSeq;
    U8   au8Data[0];
}ES_PKT_HDR_S;

typedef struct tagEsStorPktHdr
{
    LE32  le32USec;
    LE32  le32Sec;
    ES_PKT_HDR_S stEsPkt;
}ES_STOR_PKT_HDR_S;

typedef struct tagMediaBufHdr
{
    LE32 le32Len;
    ES_STOR_PKT_HDR_S stStorPktHdr;
    U8 au8Data[0];
}MEDIA_BUF_HDR_S;

typedef struct tagConnParam
{
    CHAN_ID_S stListenChanId;
    CHAN_ID_S stRxChanId;
}CONN_PARAM_S;

#pragma pack ()

#endif //__VA_MEDIA_DEF_H__
