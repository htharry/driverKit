#ifndef __VN_DEF_H__
#define __VN_DEF_H__

enum
{
    VN_STREAM_PES_PKT = 0UL,
    VN_STREAM_TS_PKT,
    VN_STREAM_RTP_PS,
    VN_STREAM_RFC3984,
    VN_STREAM_RTP_ES,
    VN_STREAM_UNKOWN_PKT,
    VN_STREAM_MODE_BUTT
};

enum
{
    VN_TRANSPORT_UDP   = 0UL,
    VN_TRANSPORT_RTP,
    VN_TRANSPORT_TCP,
    VN_TRANSPORT_RTSP,
    VN_TRANSPORT_RTCP,
    VN_TRANSPORT_NONE,
    VN_TRANSPORT_MODE_BUTT
};

enum
{
    VN_RX       = 0x1,
    VN_TX       = 0x2,
    VN_BIDIR    = 0x3,  //VN_RX | VN_TX
};

#pragma  pack(4)

typedef struct tagVnInAddr
{
    BE32            be32IpAddr;
    BE16            be16Port;
    BE16            be16Reserved;
}VN_IN_ADDR_S;

typedef struct tagVnNodeParam
{
    U16             u16DataType;
    U16             u16TransportMode;
    U8              u8Dir;
    U32             u32Flag;
    INT             nFd;
    VN_IN_ADDR_S    stRemoteAddr;
    VN_IN_ADDR_S    stLocalAddr;
    CHAN_ID_S       stChanId;
}VN_NODE_PARAM_S;

typedef struct tagVnNodeFwd
{
    U64             u64FwdId;
    VN_NODE_PARAM_S stNodeInfo;
}VN_NODE_FWD_S;

#pragma  pack()

#endif //__VN_DEF_H__
