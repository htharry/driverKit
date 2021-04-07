/*
    reference ISO/IEC 13818 standard
 */

#include "vc_es2ts.h"

extern U32 VA_Crc32(IN U8 *pu8Data, IN U32 u32Len);
static VOID VC_Es2TsFillPat(VC_ES2TS_CB_S *pstVcTsCb, VC_TS_BUF_TYPE_S *pstBufType);
static VOID VC_Es2TsFillPmt(VC_ES2TS_CB_S *pstVcTsCb, VC_TS_BUF_TYPE_S *pstBufType);

/* this pid is user defined! GB28121 define this? */
static const U16 gu16VideoPid[] = {0x00, 0x45, 0x100, 0x65, 0x85};
static const U16 gu16AudioPid[] = {0x00, 0x44, 0x101, 0x64, 0x84};

/* PMT是针对一个节目的，PID取值只要能唯一区分PMT TS包即可 */
static const U16 gu16VideoPmtPid[] = {0x00, 0x42, 0xff, 0x62, 0x82};

static const U16 gu16AudioPmtPid[] = {0x00, 0x42, 0xff};

/*
 unkown
 MPEG2  0x02
 MPEG4  0x10
 H264   0x1b
 H265   0x24
 */
static const U8 gu8VideoStreamType[] = {0x00, 0x02, 0x10, 0x1b, 0x24};

/*
 unkown
 G711A
 G711U
 */
static const U8 gu8AudioStreamType[] = {0x00, 0x81, 0x80};

static U8 gau8PatPktHdr[16];
static U8 gau8PmtPktHdr[16];

static VOID VC_Es2TsInitPatData(VOID)
{
    U8 *pu8Pat;

    /* TS packet header pat pid = 0  */
    gau8PatPktHdr[0] = VC_TS_SYNC_FLAG;
    gau8PatPktHdr[1] = VC_TS_PAYLOAD_UINT_START_INDICATOR; //payload_unit_start_indicator
    gau8PatPktHdr[2] = 0x00;
    gau8PatPktHdr[3] = VC_TS_AFC_PAYLOAD; /* no adaptation field */

    //第五个字节00称为“指针域”，表示一个偏移量，即从后面第几个字节开始时PAT部分
    gau8PatPktHdr[4] = 0x00;

    pu8Pat = &gau8PatPktHdr[VC_TS_PAT_OFF];

/*======================
    typedef struct TS_PAT
    {
    　　unsigned table_id                     : 8;    //固定为0x00 ，标志是该表是PAT表
    　　unsigned section_syntax_indicator     : 1;    //段语法标志位，固定为1
    　　unsigned zero                         : 1;    //0
    　　unsigned reserved_1                   : 2;    // 保留位
    　　unsigned section_length               : 12;   //表示从下一个字段开始到CRC32(含)之间有用的字节数
        unsigned transport_stream_id        : 16;   //该传输流的ID，区别于一个网络中其它多路复用的流
    　　unsigned reserved_2                   : 2;    // 保留位
    　　unsigned version_number               : 5;    //范围0-31，表示PAT的版本号
    　　unsigned current_next_indicator       : 1;    //发送的PAT是当前有效还是下一个PAT有效
    　　unsigned section_number               : 8;    //分段的号码。PAT可能分为多段传输，第一段为00，以后每个分段加1，最多可能有256个分段
    　　unsigned last_section_number          : 8;    //最后一个分段的号码

    　　std::vector<TS_PAT_Program> program;
    　　unsigned reserved_3                   : 3;    // 保留位
    　　unsigned network_PID                  : 13;   //网络信息表（NIT）的PID,节目号为0时对应的PID为network_PID
    　　unsigned CRC_32                       : 32;   //CRC32校验码
    } TS_PAT;
============================= */

    /* PAT表数据 */
    pu8Pat[0] = 0x00;  /* table_id must zero to pas */
    pu8Pat[1] = 0xb0;
    pu8Pat[2] = 0x0d;  // section_length 13 bytes
    pu8Pat[3] = 0x00;
    pu8Pat[4] = 0x01;  // transport_stream_id
    pu8Pat[5] = 0xc1;  // rsvd = 3  current_next_indicator = 1
    pu8Pat[6] = 0x00;
    pu8Pat[7] = 0x00;
    pu8Pat[8] = 0x00;
    pu8Pat[9] = 0x01;  // program_number is one to represent pmt

    return;
}

static VOID VC_Es2TsCpyPat(U8 *pu8Pkt)
{
    memcpy(pu8Pkt, gau8PatPktHdr, sizeof(gau8PatPktHdr));
}

static VOID VC_Es2TsInitPmtData(VOID)
{
    U8 *pu8Pmt;

    /* TS packet header */
    gau8PmtPktHdr[0] = VC_TS_SYNC_FLAG;
    gau8PmtPktHdr[1] = VC_TS_PAYLOAD_UINT_START_INDICATOR | ((VC_TS_PMT_PID >> 8) & 0x1f);
    gau8PmtPktHdr[2] = (U8)(VC_TS_PMT_PID & 0xff);
    gau8PmtPktHdr[3] = VC_TS_AFC_PAYLOAD; /* no adaptation field */

    gau8PmtPktHdr[4] = 0x00;

/*============================
typedef struct TS_PMT
{
	unsigned table_id:                  :8;     //固定为0x02, 表示PMT表
	unsigned section_syntax_indicator:  :1;     //固定为0x01
	unsigned zero                       :1;     //0x01
	unsigned reserved_1                 :2;     //0x03
	unsigned section_length             :12;    //首先两位bit置为00，它指示段的byte数，由段长度域开始，包含CRC。
	unsigned program_number             :16;    // 指出该节目对应于可应用的Program map PID
	unsigned reserved_2                 :2;     //0x03
	unsigned version_number             :5;     //指出TS流中Program map section的版本号
	unsigned current_next_indicator     :1;     //当该位置1时，当前传送的Program map section可用；
	  //当该位置0时，指示当前传送的Program map section不可用，下一个TS流的Program map section有效。
	unsigned section_number             :8;     //固定为0x00
	unsigned last_section_number        :8;     //固定为0x00
	unsigned reserved_3                 :3;     //0x07
	unsigned PCR_PID                    :13;    //指明TS包的PID值，该TS包含有PCR域，
	unsigned reserved_4                 :4;     //预留为0x0F
	unsigned program_info_length        :12;    //前两位bit为00。该域指出跟随其后对节目信息的描述的byte数。
	std::vector<TS_PMT_Stream> PMT_Stream;      //每个元素包含8位, 指示特定PID的节目元素包的类型。该处PID由elementary PID指定
	unsigned reserved_5                 :3;     //0x07
	unsigned reserved_6                 :4;     //0x0F
	unsigned CRC_32                     :32;
} TS_PMT;
==============================*/

    /* PMT */
    pu8Pmt   = &gau8PmtPktHdr[VC_TS_PMT_OFF];
    pu8Pmt[0] = 0x02;
    pu8Pmt[1] = 0xb0;
    pu8Pmt[2] = 0x00;  // section_length(12bit)
    pu8Pmt[3] = 0x00;
    pu8Pmt[4] = 0x01;
    pu8Pmt[5] = 0xc1;  // rsvd = 3  current_next_indicator = 1
    pu8Pmt[6] = 0x00;
    pu8Pmt[7] = 0x00;
}

static VOID VC_Es2TsCpyPmt(U8 *pu8Pkt)
{
    memcpy(pu8Pkt, gau8PmtPktHdr, sizeof(gau8PmtPktHdr));
}

static VOID VC_Es2TsOutputData(VC_ES2TS_CB_S *pstVcTsCb, VC_TS_BUF_TYPE_S *pstBufType, U8 *pu8Data)
{
    // output to buffer!
    VC_ES_BUF_TYPE_S *pstEsBufType = &(pstBufType->stBufType);
    pstVcTsCb->stVcCb.pfnOutputData(pstVcTsCb->stVcCb.pPrivCb, pstEsBufType->u8DataType, pu8Data, VC_TS_PKT_LEN);
    pstEsBufType->u32OutBufLen += VC_TS_PKT_LEN;

    if ( pstEsBufType->u32OutBufLen >= VC_TS_NET_PKT_LEN )
    {
        VC_FlushEsData(&pstVcTsCb->stVcCb, &pstBufType->stBufType);
    }

    return;
}

static VOID VC_Es2TsFillPcr(ES_PKT_HDR_S *pstEsPkt, U8 *pu8Buf)
{
    U32 u32Pts;

    /*
     PCR包括一个33比特的低精度部分（90kHz）和一个9比特的高精度部分（27MHz，取值为0-299）
     PCR容许的最大抖动为+/-500ns
     PCR last bit to zero
     PCR_ext to zero
    */
    u32Pts = DRV_LE32_TO_CPU(pstEsPkt->le32PTS);

    VC_TS_SET_BUF_U32(u32Pts, pu8Buf);

    pu8Buf[4] = (((0x0 & 0x01) << 7) + 0x7E + 0x0);
    pu8Buf[5] = 0;

    return;
}

VOID VC_Es2TsFillPid(VC_TS_BUF_TYPE_S *pstBufType, U8 *pu8Buf)
{
    U16 u16Pid;

    u16Pid = pstBufType->pu16PidMap[pstBufType->stBufType.u8EncFmtType];
    VC_TS_SET_BUF_U16(u16Pid, pu8Buf);

    return;
}

VOID VC_Es2TsFillStartPayloadPid(VC_TS_BUF_TYPE_S *pstBufType, U8 *pu8Buf)
{
    U16 u16Pid;

    u16Pid = pstBufType->pu16PidMap[pstBufType->stBufType.u8EncFmtType];
    VC_TS_SET_BUF_U16((u16Pid | 0x4000), pu8Buf);

    return;
}

U32 VC_Es2TsFillOneTsPkt(VC_ES2TS_CB_S *pstVcTsCb, VC_TS_BUF_TYPE_S *pstBufType, U8 *pu8Data, U16 *pu16LeftLen, BOOL bEnd)
{
    VC_ES_BUF_TYPE_S *pstEsBufType = &(pstBufType->stBufType);
    U16 u16TmpBufLen = pstEsBufType->u32TmpBufLen;
    U16 u16CpLen;
    U8  *pu8TsBuf = pstEsBufType->pu8TmpBuf;

    u16CpLen = VC_TS_PKT_LEN - u16TmpBufLen;
    if ( u16CpLen > *pu16LeftLen )
    {
        u16CpLen = *pu16LeftLen;
    }

    memcpy(pu8TsBuf + u16TmpBufLen, pu8Data, u16CpLen);

    *pu16LeftLen -= u16CpLen;
    pstEsBufType->u32TmpBufLen = 0;
    if ( bEnd && *pu16LeftLen == 0 )
    {
        pu8TsBuf[1] |= 0x20;
    }

    VC_Es2TsOutputData(pstVcTsCb, pstBufType, pu8TsBuf);
    return u16CpLen;
}

VOID VC_Es2TsFillTsHdr(VC_TS_BUF_TYPE_S *pstBufType, U8 u8StuffLen, BOOL bIFrame)
{
    VC_ES_BUF_TYPE_S *pstEsBufType = &pstBufType->stBufType;
    U8 *pu8TsData = pstEsBufType->pu8TmpBuf;

    pu8TsData[0] = VC_TS_SYNC_FLAG;
    VC_Es2TsFillPid(pstBufType, pu8TsData + 1);

    pu8TsData[3] = 0x30 | (pstBufType->u32TsPktSeq & 0x0f);
    pu8TsData[4] = (U8)(u8StuffLen + 1);

    /* add elementary_stream_priority_indicator */
    if (bIFrame)
    {
        pu8TsData[5] = 0x20; /* 0010 0000 */
    }
    else
    {
        pu8TsData[5] = 0x00; /* 0000 0000 */
    }

    if ( u8StuffLen )
    {
        memset(&pu8TsData[VC_TS_MID_HDR_LEN], 0xff, u8StuffLen);
    }

    pstBufType->u32TsPktSeq++;
    pstEsBufType->u32TmpBufLen = VC_TS_MID_HDR_LEN + u8StuffLen;

    return;
}

VOID VC_Es2TsFillData(VC_ES2TS_CB_S *pstVcTsCb, VC_TS_BUF_TYPE_S *pstBufType, ES_PKT_HDR_S *pstEsPkt, U16 u16PktDataOff)
{
    VC_ES_BUF_TYPE_S *pstEsBufType = &(pstBufType->stBufType);
    U16 u16LeftLen = VA_ES_PKT_LEN(pstEsPkt) - u16PktDataOff;
    U8  *pu8Data   = (U8 *)(pstEsPkt + 1) + u16PktDataOff;
    BOOL bEnd    = VA_IS_END_PKT(pstEsPkt);
    BOOL bIFrame = VA_IS_IFRAME(pstEsPkt);

    if (pstEsBufType->u32TmpBufLen != 0)
    {
        pu8Data += VC_Es2TsFillOneTsPkt(pstVcTsCb, pstBufType, pu8Data, &u16LeftLen, bEnd);
    }

    // fill middle ts packet
    while ( u16LeftLen >= VC_TS_MAX_DATA_LEN )
    {
        VC_Es2TsFillTsHdr(pstBufType, 0, bIFrame);
        pu8Data += VC_Es2TsFillOneTsPkt(pstVcTsCb, pstBufType, pu8Data, &u16LeftLen, bEnd);
    }

    if ( u16LeftLen == 0 )
    {
        if ( bEnd )
        {
            // force flush data
            VC_FlushEsData(&pstVcTsCb->stVcCb, &pstBufType->stBufType);
        }
        return;
    }

    if ( bEnd )
    {
        // last packet
        U16 u8StuffLen = (VC_TS_MAX_DATA_LEN - u16LeftLen);
        VC_Es2TsFillTsHdr(pstBufType, u8StuffLen, bIFrame);
        pu8Data += VC_Es2TsFillOneTsPkt(pstVcTsCb, pstBufType, pu8Data, &u16LeftLen, bEnd);

        // force flush data
        VC_FlushEsData(&pstVcTsCb->stVcCb, &pstBufType->stBufType);
    }
    else // buffer left data, as don't know the pad len
    {
        memcpy(pstBufType->au8LeftData, pu8Data, u16LeftLen);
        pstBufType->u32LeftDataLen = u16LeftLen;
    }

    return;
}

static VOID VC_Es2TsClearBuf(VC_TS_BUF_TYPE_S *pstBufType)
{
    pstBufType->u32LeftDataLen = 0;
    pstBufType->stBufType.u32TmpBufLen = 0;
}

static VOID VC_Es2TsFillFirstVideoData(VC_ES2TS_CB_S *pstVcTsCb, VC_TS_BUF_TYPE_S *pstBufType, ES_PKT_HDR_S *pstEsPkt, U32 u32Len)
{
    U8  *au8TsBuf;
    U8  u8OffSet = 0;
    U16 u16LeftLen;
    U16 u16PadLen;
    U16 u16PktLen = DRV_LE16_TO_CPU(pstEsPkt->le16PktLen);

    VC_Es2TsClearBuf(pstBufType);
    au8TsBuf = pstBufType->stBufType.pu8TmpBuf;

    if ( VA_IS_IFRAME(pstEsPkt) )
    {
        VC_Es2TsFillPat(pstVcTsCb, pstBufType);
        VC_Es2TsFillPmt(pstVcTsCb, pstBufType);
    }

    /* header + adaptation_field_length + PES */
    u16LeftLen = VC_TS_PKT_LEN - (VC_TS_PES_LEN + VC_TS_HDR_AND_ADAPT_LEN);

    au8TsBuf[0] = VC_TS_SYNC_FLAG;
    VC_Es2TsFillStartPayloadPid(pstBufType, &au8TsBuf[1]);

    au8TsBuf[3] = 0x30 | (pstBufType->u32TsPktSeq & 0x0f); /* adapt and payload field */
    au8TsBuf[4] = 0x01 + VC_TS_PCR_LEN;  /* add adaptation_field_length */

    /* add pcr & random_access_indicator & elementary_stream_priority_indicator */
    if ( VA_IS_IFRAME(pstEsPkt) ) // flag
    {
        au8TsBuf[5] = 0x70; /* 0111 0000 */
    }
    else
    {
        au8TsBuf[5] = 0x10; /* 0001 0000 */
    }

    pstBufType->u32TsPktSeq++;

    VC_Es2TsFillPcr(pstEsPkt, au8TsBuf + 6);

    if (u16PktLen >= u16LeftLen) /* 不需要padding数据 */
    {
        u8OffSet = VC_TS_HDR_AND_ADAPT_LEN;
    }
    else
    {
        /* 需要padding数据 */
        u16PadLen    = u16LeftLen - u16PktLen;
        au8TsBuf[4] += (U8)u16PadLen; /* add stuffing len */
        memset(&au8TsBuf[VC_TS_HDR_AND_ADAPT_LEN], 0xff, u16PadLen);
        u8OffSet = VC_TS_HDR_AND_ADAPT_LEN + u16PadLen;
    }

    /* PES */
    VC_FillPes(&au8TsBuf[u8OffSet], pstEsPkt, VC_PES_VIDEO_STREAM_ID);
    u8OffSet += VC_PES_LEN;
    pstBufType->stBufType.u32TmpBufLen = u8OffSet;

    VC_Es2TsFillData(pstVcTsCb, pstBufType, pstEsPkt, 0);
    return;
}

static VOID VC_Es2TsFillNextVideoData(VC_ES2TS_CB_S *pstVcTsCb, VC_TS_BUF_TYPE_S *pstBufType, ES_PKT_HDR_S *pstEsPkt, U32 u32Len)
{
    U16 u16PktLen  = DRV_LE16_TO_CPU(pstEsPkt->le16PktLen);
    U16 u16LeftLen = u16PktLen + pstBufType->u32LeftDataLen;
    U8  *pu8Data = (U8 *)(pstEsPkt + 1);
    BOOL bEnd    = VA_IS_END_PKT(pstEsPkt);
    BOOL bIFrame = VA_IS_IFRAME(pstEsPkt);
    U16  u16CpLen = u16PktLen;
    U16  u16StuffLen;

    if ( u16LeftLen > VC_TS_MAX_DATA_LEN )
    {
        u16CpLen = VC_TS_MAX_DATA_LEN - pstBufType->u32LeftDataLen;
    }

    memcpy(pstBufType->au8LeftData + pstBufType->u32LeftDataLen, pu8Data, u16CpLen);

    if ( bEnd == false && u16LeftLen < VC_TS_MAX_DATA_LEN ) //impossible
    {
        pstBufType->u32LeftDataLen = u16LeftLen;
        return;
    }

    if ( u16LeftLen < VC_TS_MAX_DATA_LEN )
    {
        u16StuffLen = (VC_TS_MAX_DATA_LEN - u16LeftLen);
    }
    else
    {
        u16StuffLen = 0;
    }

    //u16LeftLen = VC_TS_MAX_DATA_LEN - u16StuffLen;
    VC_Es2TsFillTsHdr(pstBufType, u16StuffLen, bIFrame);
    VC_Es2TsFillOneTsPkt(pstVcTsCb, pstBufType, pstBufType->au8LeftData, &u16LeftLen, bEnd);
    pstBufType->u32LeftDataLen = 0;

    if ( u16LeftLen == 0 )
    {
        // last packet
        return;
    }

    VC_Es2TsFillData(pstVcTsCb, pstBufType, pstEsPkt, u16CpLen);
    return;
}

static LONG VC_Es2TsAudio(VC_CB_S *pstVcCb, VC_TS_BUF_TYPE_S *pstBufType, ES_PKT_HDR_S *pstEsPkt, U32 u32Len)
{
    VC_ES_BUF_TYPE_S *pstVideoEsBufType;
    VC_ES2TS_CB_S *pstVcTsCb;
    U8  *au8TsBuf;
    U8  u8OffSet = 0;
    U16 u16LeftLen;
    U16 u16PadLen;
    U16 u16PktLen = DRV_LE16_TO_CPU(pstEsPkt->le16PktLen);

    pstVcTsCb = VA_PTR_TYPE(VC_ES2TS_CB_S, pstVcCb);
    au8TsBuf  = pstBufType->stBufType.pu8TmpBuf;

    if ( !VA_IS_FIRST_PKT(pstEsPkt) || !VA_IS_END_PKT(pstEsPkt) )
    {
        return VC_E_INVALID;
    }

    pstVideoEsBufType = &pstVcTsCb->stVideoBuf.stBufType;
    if ( pstVideoEsBufType->u8EncFmtType == VA_ES_TYPE_NULL )
    {
        if ( ((pstVcTsCb->ulAudioPktCnt++) % 25) == 0 )
        {
            VC_Es2TsFillPat(pstVcTsCb, pstBufType);
            VC_Es2TsFillPmt(pstVcTsCb, pstBufType);
        }
    }

    /* header + adaptation_field_length + PES */
    u16LeftLen = VC_TS_PKT_LEN - (VC_TS_PES_LEN + VC_TS_HDR_AND_ADAPT_LEN);

    /* fix me for just audio channel */

    au8TsBuf[0] = VC_TS_SYNC_FLAG;
    VC_Es2TsFillStartPayloadPid(pstBufType, &au8TsBuf[1]);

    au8TsBuf[3] = 0x30 | (pstBufType->u32TsPktSeq & 0x0f); /* 具有调整字段和PAYLOAD */
    au8TsBuf[4] = 0x01 + VC_TS_PCR_LEN;  /* add adaptation_field_length */

    /* add pcr & elementary_stream_priority_indicator */
    au8TsBuf[5] = 0x30; /* 0011 0000 */
    pstBufType->u32TsPktSeq++;

    VC_Es2TsFillPcr(pstEsPkt, au8TsBuf + 6);

    if (u16PktLen >= u16LeftLen) /* 不需要padding数据 */
    {
        u8OffSet = VC_TS_HDR_AND_ADAPT_LEN;
    }
    else
    {
        /* 需要padding数据 */
        u16PadLen    = u16LeftLen - u16PktLen;
        au8TsBuf[4] += (U8)u16PadLen; /* add stuffing len */
        memset(&au8TsBuf[VC_TS_HDR_AND_ADAPT_LEN], 0xff, u16PadLen);
        u8OffSet = VC_TS_HDR_AND_ADAPT_LEN + u16PadLen;
    }

    /* PES */
    VC_FillPes(&au8TsBuf[u8OffSet], pstEsPkt, VC_PES_AUDIO_STREAM_ID);
    u8OffSet += VC_PES_LEN;
    pstBufType->stBufType.u32TmpBufLen = u8OffSet;

    VC_Es2TsFillData(pstVcTsCb, pstBufType, pstEsPkt, 0);
    return VC_SUCCESS;
}

static VOID VC_Es2TsFillPat(VC_ES2TS_CB_S *pstVcTsCb, VC_TS_BUF_TYPE_S *pstBufType)
{
    U8  au8TsBuf[VC_TS_PKT_LEN];
    U32 u32Offset = 0;
    U16 u16PmtPid;

    VC_Es2TsCpyPat(au8TsBuf);
    au8TsBuf[3] |= (pstVcTsCb->u32PatSeq & 0xf);

    /* fill pmt pid */
    u32Offset = VC_TS_PAT_OFF + 10;
    u16PmtPid = VC_TS_PMT_PID; // fix me, const to 0x83 is ok!
    au8TsBuf[u32Offset++] = 0xe0 | ((u16PmtPid >> 8) & 0x1f);
    au8TsBuf[u32Offset++] = (U8)(u16PmtPid & 0xff);

    if (0x00 == pstVcTsCb->u32PatCrc)
    {
        pstVcTsCb->u32PatCrc = VA_Crc32(au8TsBuf + VC_TS_PAT_OFF, 12);
    }

    VC_TS_SET_BUF_U32(pstVcTsCb->u32PatCrc, au8TsBuf + u32Offset);
    u32Offset += 4;

    /* padding 0xff */
    memset(&au8TsBuf[u32Offset], 0xff, (VC_TS_PKT_LEN - u32Offset));

    VC_Es2TsOutputData(pstVcTsCb, pstBufType, au8TsBuf);
    pstVcTsCb->u32PatSeq++;
    return;
}

static VOID VC_Es2TsFillPmt(VC_ES2TS_CB_S *pstVcTsCb, VC_TS_BUF_TYPE_S *pstBufType)
{
    VC_ES_BUF_TYPE_S *pstAudioEsBufType;
    VC_ES_BUF_TYPE_S *pstVideoEsBufType;
    U8  au8TsBuf[VC_TS_PKT_LEN];
    U8  u8Offset = 0;
    U16 u16Pid;
    U8  *pu8SecLen = au8TsBuf + VC_TS_PMT_SEC_LEN_OFF;

    VC_Es2TsCpyPmt(au8TsBuf);
    au8TsBuf[3] |= (pstVcTsCb->u32PmtSeq & 0xf);
    u8Offset = 13;

    u16Pid = pstBufType->pu16PidMap[pstBufType->stBufType.u8EncFmtType];
    au8TsBuf[u8Offset++] = 0xe0 | ((u16Pid >> 8) & 0x1f); //pcr pid
    au8TsBuf[u8Offset++] = (U8)(u16Pid & 0xff);
    au8TsBuf[u8Offset++] = 0xf0;
    au8TsBuf[u8Offset++] = 0x00;

    pstAudioEsBufType = &pstVcTsCb->stAudioBuf.stBufType;
    if ( pstAudioEsBufType->u8EncFmtType != VA_AUDIO_TYPE_NULL )
    {
        u16Pid = pstVcTsCb->stAudioBuf.pu16PidMap[pstAudioEsBufType->u8EncFmtType];
        au8TsBuf[u8Offset++] = pstVcTsCb->stAudioBuf.pu8StreamTypeMap[pstAudioEsBufType->u8EncFmtType]; //stream_type
        au8TsBuf[u8Offset++] = 0xe0 | ((u16Pid >> 8) & 0x1f);
        au8TsBuf[u8Offset++] = (U8)(u16Pid & 0xff);
        au8TsBuf[u8Offset++] = 0xf0;
        au8TsBuf[u8Offset++] = 0x00;
    }

    pstVideoEsBufType = &pstVcTsCb->stVideoBuf.stBufType;
    if ( pstVideoEsBufType->u8EncFmtType != VA_ES_TYPE_NULL )
    {
        u16Pid = pstVcTsCb->stVideoBuf.pu16PidMap[pstVideoEsBufType->u8EncFmtType];
        au8TsBuf[u8Offset++] = pstVcTsCb->stVideoBuf.pu8StreamTypeMap[pstVideoEsBufType->u8EncFmtType]; //stream_type
        au8TsBuf[u8Offset++] = 0xe0 | ((u16Pid >> 8) & 0x1f);
        au8TsBuf[u8Offset++] = (U8)(u16Pid & 0xff);
        au8TsBuf[u8Offset++] = 0xf0;
        au8TsBuf[u8Offset++] = 0x00;
    }

    // update pmt len, including crc32
    *pu8SecLen = au8TsBuf + u8Offset + 4 - (pu8SecLen + 1);

    // if (0x00 == pstBufType->u32PmtCrc)
    {
        pstVcTsCb->u32PmtCrc = VA_Crc32(au8TsBuf + VC_TS_PMT_OFF, u8Offset - VC_TS_PMT_OFF);
    }

    VC_TS_SET_BUF_U32(pstVcTsCb->u32PmtCrc, au8TsBuf + u8Offset);
    u8Offset += 4;

    // padding 0xff
    memset(&au8TsBuf[u8Offset], 0xff, (VC_TS_PKT_LEN - u8Offset));

    // output to buffer!
    VC_Es2TsOutputData(pstVcTsCb, pstBufType, au8TsBuf);
    // VC_FlushEsData(&pstVcTsCb->stVcCb, &pstBufType->stBufType);
    pstVcTsCb->u32PmtSeq++;

    return;
}


static LONG VC_Es2TsVideo(VC_CB_S *pstVcCb, VC_TS_BUF_TYPE_S *pstVideoBuf, ES_PKT_HDR_S *pstEsPkt, U32 u32Len)
{
    VC_ES2TS_CB_S *pstVcTsCb;

    pstVcTsCb = VA_PTR_TYPE(VC_ES2TS_CB_S, pstVcCb);

    if ( VC_CheckEsEncFormat(&pstVideoBuf->stBufType, pstEsPkt) != VA_SUCCESS )
    {
        pstVideoBuf->stBufType.bDrop = TRUE;
        return VC_E_DROP;
    }

    if ( pstVideoBuf->stBufType.bDrop && !(VA_IS_FIRST_PKT(pstEsPkt) && VA_IS_IFRAME(pstEsPkt)) )
    {
        return VC_E_DROP;
    }

    if ( VA_IS_FIRST_PKT(pstEsPkt) )
    {
        if ( VA_IS_IFRAME(pstEsPkt) )
        {
            pstVideoBuf->stBufType.bDrop = FALSE;
            pstVideoBuf->stBufType.u32FrameSeq   = DRV_LE32_TO_CPU(pstEsPkt->le32FrmSeq);
            pstVideoBuf->stBufType.u16NextPktSeq = DRV_LE16_TO_CPU(pstEsPkt->le16PktSeq) + 1;
        }
        else
        {
            pstVideoBuf->stBufType.u32FrameSeq++;
            pstVideoBuf->stBufType.u16NextPktSeq = 0;
            if ( VC_ChkEsPkt(&pstVideoBuf->stBufType, pstEsPkt) != VC_SUCCESS )
            {
                pstVideoBuf->stBufType.bDrop = TRUE;
                return VC_E_DROP;
            }
        }

        VC_Es2TsFillFirstVideoData(pstVcTsCb, pstVideoBuf, pstEsPkt, u32Len);
    }
    else
    {
        if ( VC_ChkEsPkt(&pstVideoBuf->stBufType, pstEsPkt) != VC_SUCCESS )
        {
            pstVideoBuf->stBufType.bDrop = TRUE;
            return VC_E_DROP;
        }

        VC_Es2TsFillNextVideoData(pstVcTsCb, pstVideoBuf, pstEsPkt, u32Len);
    }

    return VC_SUCCESS;
}

static LONG VC_Es2TsConvert(VC_CB_S *pstVcCb, void *pData, U32 u32Len)
{
    VC_ES2TS_CB_S *pstVcTsCb;
    ES_STOR_PKT_HDR_S *pstStorPktHdr;
    ES_PKT_HDR_S *pstEsPkt;
    LONG lRet;

    pstStorPktHdr = (ES_STOR_PKT_HDR_S *)pData;
    pstEsPkt      = &pstStorPktHdr->stEsPkt;
    pstVcTsCb     = VA_PTR_TYPE(VC_ES2TS_CB_S, pstVcCb);

    if ( VA_IS_VIDEO_PKT(pstEsPkt) )
    {
        lRet = VC_Es2TsVideo(pstVcCb, &pstVcTsCb->stVideoBuf, pstEsPkt, u32Len);
    }
    else
    {
        lRet = VC_Es2TsAudio(pstVcCb, &pstVcTsCb->stAudioBuf, pstEsPkt, u32Len);
    }

    if ( lRet != VC_SUCCESS )
    {
        VA_LOG_INFO("error, u32Len = %u", u32Len);
    }

    return lRet;
}

#if 0
#endif

static VOID VC_Es2TsDestroy(VC_CB_S *pstVcCb)
{
    VC_TS_BUF_TYPE_S *pstBufType;
    VC_ES2TS_CB_S *pstVcTsCb;

    pstVcTsCb  = (VC_ES2TS_CB_S *)pstVcCb;
    pstBufType = &pstVcTsCb->stVideoBuf;
    if ( pstBufType->stBufType.pu8TmpBuf != NULL )
    {
        VA_Free(pstBufType->stBufType.pu8TmpBuf);
        pstBufType->stBufType.pu8TmpBuf = NULL;
    }

    pstBufType = &pstVcTsCb->stAudioBuf;
    if ( pstBufType->stBufType.pu8TmpBuf == NULL )
    {
        VA_Free(pstBufType->stBufType.pu8TmpBuf);
        pstBufType->stBufType.pu8TmpBuf = NULL;
    }

    return;
}

static LONG VC_Es2TsInit(VC_CB_S *pstVcCb)
{
    VC_TS_BUF_TYPE_S *pstBufType;
    VC_ES2TS_CB_S *pstVcTsCb;

    pstVcTsCb  = (VC_ES2TS_CB_S *)pstVcCb;
    pstBufType = &pstVcTsCb->stVideoBuf;
    VC_InitEsBufType(&pstBufType->stBufType, VA_ES_TYPE_H265, VA_BUF_TYPE_VIDEO);
    pstBufType->pu16PmtMap = gu16VideoPmtPid;
    pstBufType->pu16PidMap = gu16VideoPid;
    pstBufType->pu8StreamTypeMap = gu8VideoStreamType;

    pstBufType->stBufType.pu8TmpBuf = VA_Malloc(VC_TS_PKT_LEN);
    if ( pstBufType->stBufType.pu8TmpBuf == NULL )
    {
        return VC_E_NOMEM;
    }

    pstBufType = &pstVcTsCb->stAudioBuf;
    VC_InitEsBufType(&pstBufType->stBufType, VA_AUDIO_TYPE_PCMA, VA_BUF_TYPE_AUDIO);
    pstBufType->pu16PmtMap = gu16AudioPmtPid;
    pstBufType->pu16PidMap = gu16AudioPid;
    pstBufType->pu8StreamTypeMap = gu8AudioStreamType;

    pstBufType->stBufType.pu8TmpBuf = VA_Malloc(VC_TS_PKT_LEN);
    if ( pstBufType->stBufType.pu8TmpBuf == NULL )
    {
        VC_Es2TsDestroy(pstVcCb);
        return VC_E_NOMEM;
    }

    return VC_SUCCESS;
}

VCER_S gstEsToTsVcer =
{
    .u16InDataType  = VA_STREAM_TYPE_ES,
    .u16OutDataType = VA_STREAM_TYPE_TS,
    .u32CbSize      = sizeof(VC_ES2TS_CB_S),
    .pfnInit        = VC_Es2TsInit,
    .pfnDestroy     = VC_Es2TsDestroy,
    .pfnConvertProc = VC_Es2TsConvert,
};

static void VCER_ES_TO_TS_Exit(void)
{
}

static int VCER_ES_TO_TS_Init(VA_MOD_S *pstMod)
{
    int iRet = 0;

    VC_Es2TsInitPatData();
    VC_Es2TsInitPmtData();

    VC_RegVcer(&gstEsToTsVcer);
    return iRet;
}

VA_MOD_INIT(estots, VCER_ES_TO_TS_Init, VCER_ES_TO_TS_Exit, VA_INIT_LEVEL_MISC)

