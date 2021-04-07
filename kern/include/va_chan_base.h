#ifndef __VA_CHAN_BASE_H__
#define __VA_CHAN_BASE_H__

#include "va_kern_pub.h"
#include "fdd.h"

typedef struct tagVaChanDev
{
    VA_DEV_S stDev;
    U16      u16StartPortIdx;
    U16      u16PortNum;
    U16      u16ChanNumPerPort;
    BOOL     bExport;   // is export to application ?
}VA_CHAN_DEV_S;

typedef struct tagVaChanTypeCb
{
    U16 u16ChanType;
    U16 u16ChanCbSize;
    LONG (*pfnIoctl)(FDD_CHAN_S *pstFddChan, U32 u32Cmd, U16 u16Len, void __user *pParam);
    VOID (*pfnTypeMt)(ULONG ulExecId);
    const char *szName;
    const FDD_CHAN_OPS_S *pstChanOps;
    const FDD_PORT_OPS_S *pstPortOps;
    atomic_t stCmdCnt;
    atomic_t stCmdErrCnt;
}VA_CHAN_TYPE_CB_S;

typedef struct tagVaChanPerPort
{
    FDD_CHAN_S **ppstChanTbl;
    U32        u32ChanNum;
}VA_CHAN_PER_PORT_S;

typedef struct tagVaChanPortTbl
{
    VA_CHAN_PER_PORT_S *pstPorts;
    U32 u32PortNum;
    U32 u32TotChanNum;
}VA_CHAN_PORT_TBL_S;

typedef struct tagVaChanLocSlot
{
    VA_CHAN_PORT_TBL_S astTypePortTbl[VA_CHAN_TYPE_NUM];
}VA_CHAN_LOC_SLOT_S;

typedef struct tagVaChanMgr
{
    VA_CHAN_LOC_SLOT_S stSelfSlot;
    VA_CHAN_TYPE_CB_S  *pstChanTypeCbTbl[VA_CHAN_TYPE_NUM];
    atomic_t           stCmdCnt;
    atomic_t           stCmdErrCnt;
}VA_CHAN_MGR_S;

extern int  VA_ChanIndx2ChanId(U16 u16ChanType, U32 u32ChanIndx, CHAN_ID_S *pstChanId);
extern LONG VA_BindChan(struct file *pstFile, CHAN_ID_S *pstChanId);
extern VOID VA_ReleaseChan(struct file *pstFile);
extern FDD_CHAN_S *VA_GetChan(CHAN_ID_S *pstChanId);
extern VOID VA_CHAN_Destroy(CHAN_ID_S *pstChanId);
extern FDD_CHAN_S *VA_CHAN_Init(CHAN_ID_S *pstChanId);
extern int  VA_CHAN_CreatePortTbl(VA_CHAN_DEV_S *pstChanDev, BOOL bBindChan);
extern void VA_CHAN_DestroyPortTbl(VA_CHAN_DEV_S *pstChanDev);
extern uint32_t VA_CHAN_GetTotalChanNum(U16 u16ChanType);
extern const char *VA_CHAN_GetChanTypeName(U16 u16ChanType);
extern int VA_CHAN_RegTypeCb(VA_CHAN_TYPE_CB_S *pstChanTypeCb);
extern const FDD_CHAN_OPS_S *VA_CHAN_GetFddOps(U16 u16ChanType);
extern const FDD_PORT_OPS_S *VA_CHAN_GetFddPortOps(U16 u16ChanType);
extern VOID VA_CHAN_InitPort(FDD_CHAN_S *pstFddChan, FDD_PORT_S *pstPort);
extern U32  VA_CHAN_GetChanCbSize(U16 u16ChanType);
extern LONG VA_CHAN_Ioctl(U32 u32Cmd, VA_IO_CMD_S *pstIoCmd);
extern VOID VA_CHAN_DispChanDev(VA_DEV_S *pstDev, ULONG ulExecId);

#endif //__VA_CHAN_BASE_H__

