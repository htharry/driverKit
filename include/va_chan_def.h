#ifndef __VA_CHAN_DEF__
#define __VA_CHAN_DEF__

#ifdef  __cplusplus
extern "C"{
#endif

enum
{
	VA_CHAN_TYPE_MEDIA_START    = 0,
	VA_CHAN_TYPE_VDEC 			= VA_CHAN_TYPE_MEDIA_START,
	VA_CHAN_TYPE_VENC,
	VA_CHAN_TYPE_VCAP_ENC,
	VA_CHAN_TYPE_AIN,
	VA_CHAN_TYPE_AOUT,
	VA_CHAN_TYPE_MEDIA_NUM,
	VA_CHAN_TYPE_NET 		    = VA_CHAN_TYPE_MEDIA_NUM,
	VA_CHAN_TYPE_MSG,
	VA_CHAN_TYPE_MEM,
	VA_CHAN_TYPE_NUM,
	VA_CHAN_TYPE_BUTT 			= VA_CHAN_TYPE_NUM,
};

enum
{
	VA_STREAM_TYPE_NONE,
	VA_STREAM_TYPE_ES,
	VA_STREAM_TYPE_TS,
	VA_STREAM_TYPE_PS,
	VA_STREAM_TYPE_RTP,
	VA_STREAM_TYPE_RFC3984,
	VA_STREAM_TYPE_ES_FRAME,
	VA_STREAM_TYPE_NUM,
	VA_STREAM_TYPE_BUTT         = VA_STREAM_TYPE_NUM,
};

#define VA_STREAM_TYPE_BASE		VA_STREAM_TYPE_ES  // base type is es

#define VA_U64_TO_CHAN_ID(u64ChanId)	VA_PTR_TYPE(CHAN_ID_S, u64ChanId)
#define VA_CHAN_ID_TO_U64(pstChanId)	(pstChanId)->u64ChanId
#define VA_CHAN_ARGS(pstChanId)			(pstChanId)->u16ChanType, (pstChanId)->u16PortId, (pstChanId)->u16ChanId
#define VA_CHAN_FMT						"chan id %u/%u/%u"

#pragma pack (4)

typedef union tagChanId
{
	struct
	{
	    U16 u16ChanType;
	    U16 u16SlotId;
	    U16 u16PortId;
	    U16 u16ChanId;
	};

	U64 u64ChanId;
}CHAN_ID_S;

#pragma pack ()

#ifdef  __cplusplus
}
#endif /* end of __cplusplus */

#endif //__VA_CHAN_DEF__
