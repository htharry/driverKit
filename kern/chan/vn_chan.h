#ifndef __VN_CHAN_H__
#define __VN_CHAN_H__

#include "va_chan_base.h"
#include "vn_def.h"
#include <linux/socket.h>
#include <linux/net.h>
#include <net/sock.h>
#include "va_cp.h"

#define VN_RX_BUF_LEN          (64 * 1024)     // 2^n
#define VN_RX_BUF_MSK          (64 * 1024 - 1)

typedef struct tagVnPktStat
{
    ULONG ulRxPkts;
    ULONG ulRxBytes;
    ULONG ulRxOkPkts;
    ULONG ulRxErrPkts;
    ULONG ulRxDropPkts;
    ULONG ulTxPkts;
    ULONG ulTxBytes;
    ULONG ulTxOkPkts;
    ULONG ulTxDropPkts;
    ULONG ulTxErrPkts;
    ULONG ulTxLostPkts;
}VN_PKT_STAT_S;

// video network channel
typedef struct tagVnChan
{
    FDD_CHAN_S          stFddChan;
    VOID                *pPrivCb;       // transport layer private control block
    const FDD_PORT_OPS_S *pstPortOps;
    struct socket       *pstSock;
    struct sock         *pstSk;
    struct work_struct  stRxWork;
    struct msghdr       stRxMsgHdr;
    struct msghdr       stTxMsgHdr;
    VN_PKT_STAT_S       stStat;
    VN_IN_ADDR_S        stRemoteAddr;
    VN_IN_ADDR_S        stLocalAddr;
    U16                 u16DataType;
    U8                  u8Dir;
    INT                 nFd;
    U32                 u32ChanIdx;
    struct file         *pstFile;
    VA_DEV_S            stDev;
    ULONG               ulInstId;
    void                *pfnOldDataReady;
    void                *pfnOldWriteSpace;
    void (*pfnMt)(struct tagVnChan *pstVnChan, ULONG ulExecId);
}VN_CHAN_S;

typedef struct tagVnChanMgr
{
    struct mutex        stLock;
    ULONG               *pulChanBitMap;
    U32                 u32TotChanNum;
    U32                 u32ChanUseCnt;
    struct list_head    stTransportHead;
    VA_CP_INST_NOTIFY_S stCpInstNotify;
}VN_CHAN_MGR_S;

extern VOID VN_FlushRxData(VN_CHAN_S *pstChan, char *pBuf);

#endif //__VN_CHAN_H__
