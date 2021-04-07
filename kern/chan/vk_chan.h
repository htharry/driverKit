#ifndef __VK_CHAN_H__
#define __VK_CHAN_H__

#include "va_chan_base.h"
#include "va_media_def.h"

// video kernel channel
typedef struct tagVkChan
{
    FDD_CHAN_S  stFddChan;
    CHAN_ID_S   stChanId;
    FDD_BUF_S   *pstCurrBuf;
    U32         u32Dts;
    U32         u32Pts;
    ULONG       ulTxCnt;
    BOOL        bDataReady;
    wait_queue_head_t stWaitHead;
}VK_CHAN_S;

extern VOID VK_MtDispChan(FDD_CHAN_S *pstChan, ULONG ulExecId);
extern int  VK_ChanInit(VK_CHAN_S *pstChan, BOOL bListen);
extern VOID VK_RxPkt(FDD_PORT_S *pstPort, VOID *pBuf, U32 u32Len);
extern VOID VK_OutEsFrame(FDD_PORT_S *pstPort, VOID *pBuf, U32 u32Len);
extern U32  VK_DataReady(FDD_CHAN_S *pstFddChan);
extern LONG VK_BindChan(struct file *pstFile, CHAN_ID_S __user *pstChanId);
extern LONG VK_PutData(VK_CHAN_S *pstVkChan, VA_DATA_BUF_S __user *pstBuf);
extern LONG VK_GetData(VK_CHAN_S *pstVkChan, VA_DATA_BUF_S __user *pstBuf);
#endif //__VK_CHAN_H__

