#ifndef __VC_TS_DEF_H__
#define __VC_TS_DEF_H__

#define VC_TS_SYNC_FLAG             0x47
#define VC_TS_PKT_LEN               188

// ts header key bit define
#define VC_TS_TRANSPORT_ERROR_INDICATOR     (0x1 << 7)
#define VC_TS_PAYLOAD_UINT_START_INDICATOR  (0x1 << 6)
#define VC_TS_TRANSPORT_PRIORITY            (0x1 << 5)


// define adaptation_field_control
#define VC_TS_AFC_PAYLOAD                   (0x1 << 4)
#define VC_TS_AFC_AF                        (0x2 << 4)
#define VC_TS_AFC_PAYLOAD_AND_AF            (0x3 << 4)

#endif //__VC_TS_DEF_H__
