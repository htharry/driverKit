#ifndef __VAA_API_DEF_H__
#define __VAA_API_DEF_H__

extern int  VAA_Init(void);
extern void VAA_DeInit(void);
extern int  VAA_GetChanFd(U16 u16ChanType, U16 u16ChanId);
extern int  VAA_PutData(U16 u16ChanType, U16 u16ChanId, VOID *pHead, VOID *pData, U32 u32DataLen);
extern int  VAA_GetData(U16 u16ChanType, U16  u16ChanId, VOID *pHead, VOID *pData, U32 u32DataLen);
extern U16  VAA_GetChanNum(U16 u16ChanType);
extern VOID VAA_GetChanCap(VA_CHAN_CAP_S *pstCap);
extern int  VAA_Connect(CHAN_ID_S *pstListenChanId, CHAN_ID_S *pstRxChanId);
extern int  VAA_DisConnect(CHAN_ID_S *pstListenChanId, CHAN_ID_S *pstRxChanId);
extern int  VAA_AddVnNode(VN_NODE_PARAM_S *pstNodeParam);
extern int  VAA_DelVnNode(CHAN_ID_S *pstChanId);
#endif //__VAA_API_DEF_H__
