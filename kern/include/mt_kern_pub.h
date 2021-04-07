#ifndef __MT_KERN_PUB_H__
#define __MT_KERN_PUB_H__

#include "mt_def.h"

#define MT_REG_CMD_LINE(pfnCallback, pstItem, args...) \
		__MT_REG_CMD_LINE(pstMtCb, pstMtCb, pfnCallback, pstItem, ##args)

#define __MT_REG_CMD_LINE(pstMtCb, pPrivCb, pfnCallback, pstItem, args...) \
		MT_REG_EXT_CMD_LINE(pstMtCb, pPrivCb, pstMtCb->szFileName, pfnCallback, pstItem, ##args)

#define MT_REG_EXT_CMD_LINE(pstMtCb, pPrivCb, szCmdName, pfnCallback, pstItem, args...) \
		MT_RegCmdLine(pstMtCb, pPrivCb, szCmdName, pfnCallback, (MT_CMD_ITEM_S *)(pstItem), ##args, NULL)

#define MT_MERGE_ITEM_TO_ARR(pstArr, pstItem, args...)  MT_MergeItemsToArr(pstArr, pstItem, ##args, NULL)

#define MT_DBG_PRINT(pstMtCb, u32DbgLevel, szFmt, args...) \
	do { \
		if ((pstMtCb)->u32Debug & u32DbgLevel) \
		{ \
			MT_DBG_Print((pstMtCb)->szName, szFmt, ##args); \
		} \
	}while(0)

#define __MT_DBG_PRINT(pstMtCb, szDesc, u32DbgLevel, szFmt, args...) \
	do { \
		if ((pstMtCb)->u32Debug & u32DbgLevel) \
		{ \
			MT_DBG_Print(szDesc, szFmt, ##args); \
		} \
	}while(0)


#define MT_DBG_MSG_PRINT(pstMtCb, szFmt, args...)	MT_DBG_PRINT(pstMtCb, MT_DBG_MSG,  szFmt, ##args)
#define MT_DBG_ERR_PRINT(pstMtCb, szFmt, args...)	MT_DBG_PRINT(pstMtCb, MT_DBG_ERR,  szFmt, ##args)
#define MT_DBG_INFO_PRINT(pstMtCb, szFmt, args...)	MT_DBG_PRINT(pstMtCb, MT_DBG_INFO, szFmt, ##args)

#define MT_DBG_PKT_PRINT(pstMtCb, pData, u32Len, szDescFmt, args...) \
		do { \
			if ((pstMtCb)->u32Debug & MT_DBG_PKT) \
			{ \
				MT_DBG_PktPrint((U8 *)pData, u32Len, szDescFmt, ##args); \
			} \
		}while(0)

#define MT_CB_DBG_PKT_PRINT(pstMtCb, pData, u32Len) \
		do { \
			if ((pstMtCb)->u32Debug & MT_DBG_PKT) \
			{ \
				MT_DBG_PktPrint((U8 *)pData, u32Len, "%s", (pstMtCb)->szName); \
			} \
		}while(0)

struct tagMtCb;
typedef int (*MT_REG_SELF_CMD_PF)(struct tagMtCb *pstMtCb);

typedef struct tagMtCb
{
    struct list_head stNode;        // mt cb node
    struct list_head stClHead;      // cmd line head
    struct list_head stFileHead;    // file control cb list
    MT_REG_SELF_CMD_PF pfnRegCmd;
    const char *szName;
    const char *szDesc;
	const char *szFileName;
    U32 u32Debug;
    U32 u32DebugCap;
    U64 u64ViewId;
	char szFileNameBuf[32];
}MT_CB_S;

typedef VOID (*MT_CL_CALL_BACK_PF)(ULONG ulExecId);

extern VOID MT_RegCmdLine(MT_CB_S *pstMtCb, VOID *pPrivCb, const char *szFileName,
	                      MT_CL_CALL_BACK_PF pfnCallBack, MT_CMD_ITEM_S *pstItem, ...);
extern int  MT_RegMt(MT_CB_S *pstMtCb);
extern VOID MT_UnRegMt(MT_CB_S *pstMtCb);
extern int  MT_AppendCmdItemToArray(MT_CMD_ITEM_S *pstItem, const char *szName, const char *szDesc, U64  u64MinVal, U64  u64MaxVal);

extern VOID MT_DispDbgLevel(U32 u32Debug, ULONG ulExecId);
extern U32  MT_GetDbgLevel(ULONG ulExecId);
extern int MT_GetTextVal(ULONG ulExecId, const S8 *as8Id, const char **ppStr);
extern int MT_GetStrVal(ULONG ulExecId,  const S8 *as8Id, const char **ppStr);
extern int MT_GetIntVal(ULONG ulExecId,  const S8 *as8Id, ULONG *pulVal);
extern int MT_GetU64Val(ULONG ulExecId,  const S8 *as8Id, U64 *pu64Val);
extern int MT_GetChanIdVal(ULONG ulExecId, const S8 *as8Id, CHAN_ID_S *pstChanId);
extern int MT_GetIpv4Val(ULONG ulExecId, const S8 *as8Id, BE32 *pbe32Ip);
extern int MT_GetAllValById(ULONG ulExecId, const S8 *as8Id, MT_VAL_U *pstValTbl, U32 *pu32ItemNum);
extern VOID *MT_GetPrivCb(ULONG ulExecId);
extern VOID MT_DBG_Print(const char *szDesc, const char *szFmt, ...);
extern VOID MT_DBG_PktPrint(VOID *pData, U32 u32Len, const char *szDescFmt, ...);
extern int  MT_AddItemToArr(MT_CMD_ITEM_ARR_S *pstItemArr, MT_CMD_ITEM_S *pstItem);

#endif //__MT_KERN_PUB_H__
