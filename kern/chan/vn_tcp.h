#ifndef __VN_TCP_H__
#define __VN_TCP_H__

#include "vn_chan.h"
#include "va_util.h"

typedef struct  tagTcpPktHdr
{
    U8   u8Header;          /* 头标识'$' */
    U8   u8ChanId;          /* 通道号 */
    BE16 be16Len;           /* 包长度    */
}TCP_PKT_HDR_S;

typedef struct tagVnTcp
{
    VA_MAP_BUF_S  stRxBuf;
    U32           u32RxWrIndx;
    U32           u32RxRdIndx;
    TCP_PKT_HDR_S stTxHdrCache;
    struct kvec   astTxIoVec[2];
}VN_TCP_S;

#endif // __VN_TCP_H__
