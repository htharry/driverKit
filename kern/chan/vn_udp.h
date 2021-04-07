#ifndef __VN_UDP_H__
#define __VN_UDP_H__

#include "vn_chan.h"
#include "va_util.h"

typedef struct tagVnUdp
{
    char *pRxBuf;
    struct kvec stRxIoVec;
    struct sockaddr_in stDstAddr;
}VN_UDP_S;


#endif //__VN_UDP_H__
