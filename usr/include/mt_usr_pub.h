#ifndef __MT_USR_PUB_H__
#define __MT_USR_PUB_H__

#include "mt_def.h"
#include "mt_usr_compat.h"

#define MT_USR_INVAL_HANDLE                 0
#define MT_USR_IS_INVAL_HNDL(hMtHndl)       ((hMtHndl) == MT_USR_INVAL_HANDLE)


#define MT_REG_CMD_LINE(pfnCallback, pstItem, args...) \
		__MT_REG_CMD_LINE(ulMtHndl, (VOID *)ulMtHndl, pfnCallback, pstItem, ##args)

#define __MT_REG_CMD_LINE(ulMtHndl, pPrivCb, pfnCallback, pstItem, args...) \
		MT_USR_RegCmdLine(ulMtHndl, pPrivCb, pfnCallback, pstItem, ##args, NULL)

#define MT_MERGE_ITEM_TO_ARR(pstArr, pstItem, args...)  MT_USR_MergeItemsToArr(pstArr, pstItem, ##args, NULL)

#define MT_DBG_PRINT(ulMtHndl, u32DbgLevel, szFmt, args...) \
	do { \
		if ((VA_PTR_TYPE(MT_USR_CB_S, ulMtHndl))->u32Debug & u32DbgLevel) \
		{ \
			MT_DBG_Print(ulMtHndl, szFmt, ##args); \
		} \
	}while(0)

#define MT_DBG_PKT_PRINT(ulMtHndl, szDesc, pData, u32Len) \
    do { \
        if ((VA_PTR_TYPE(MT_USR_CB_S, ulMtHndl))->u32Debug & MT_DBG_PKT) \
        { \
            MT_DBG_PrintPkt(ulMtHndl, (U8 *)pData, u32Len, szDesc); \
        } \
    }while(0)


#define MT_DBG_MSG_PRINT(ulMtHndl, szFmt, args...)	MT_DBG_PRINT(ulMtHndl, MT_DBG_MSG,  szFmt, ##args)
#define MT_DBG_ERR_PRINT(ulMtHndl, szFmt, args...)	MT_DBG_PRINT(ulMtHndl, MT_DBG_ERR,  szFmt, ##args)
#define MT_DBG_INFO_PRINT(ulMtHndl, szFmt, args...)	MT_DBG_PRINT(ulMtHndl, MT_DBG_INFO, szFmt, ##args)

typedef VOID (*MT_USR_CL_CALL_BACK_PF)(ULONG ulExecId);

typedef struct tagMtUsrCb
{
    INT     iMtFd;
    U32     u32Debug;
    U32     u32DebugCap;
    char    *szName;
    char    *szDesc;
    pthread_mutex_t stMutex;
}MT_USR_CB_S;

extern int MT_GetStrVal(ULONG ulExecId, const S8 *as8Id, char szStr[MT_STR_MAX_LEN]);
extern int MT_GetTextVal(ULONG ulExecId, const S8 *as8Id, char szStr[MT_STR_MAX_LEN]);
extern int MT_GetIntVal(ULONG ulExecId, const S8 *as8Id, ULONG *pulVal);
extern int MT_GetChanIdVal(ULONG ulExecId, const S8 *as8Id, CHAN_ID_S *pstChanId);
extern int MT_GetU64Val(ULONG ulExecId, const S8 *as8Id, U64 *pu64Val);
extern int MT_GetIpv4Val(ULONG ulExecId, const S8 *as8Id, BE32 *pbe32Ip);
extern int MT_GetAllValById(ULONG ulExecId, const S8 *as8Id, MT_VAL_U *pstValTbl, U32 *pu32ItemNum);
extern VOID *MT_GetPrivCb(ULONG ulExecId);
extern int  MT_USR_AddItemToArr(MT_CMD_ITEM_ARR_S *pstItemArr, MT_CMD_ITEM_S *pstItem);
extern int  MT_USR_MergeItemsToArr(MT_CMD_ITEM_ARR_S *pstItemArr, MT_CMD_ITEM_S *pstItem, ...);
extern VOID MT_USR_ExecCmdLine(ULONG ulHandle);
extern ULONG MT_USR_RegCmdLine(ULONG ulHandle, VOID *pPrivCb, MT_USR_CL_CALL_BACK_PF pfnCallBack, MT_CMD_ITEM_S *pstItem, ...);
extern ULONG MT_USR_Open(const char *szName, const char *szDesc, U32 u32DebugCap, U32 u32Flg);
extern VOID MT_USR_Close(ULONG ulHandle);
extern INT MT_USR_GetFd(ULONG ulMtHndl);
extern VOID __MT_DBG_Print(ULONG ulMtHndl, const char *szFmt, ...);
extern VOID MT_DBG_Print(ULONG ulMtHndl, const char *szFmt, ...);
extern VOID MT_DBG_PrintPkt(ULONG ulMtHndl, U8 *pu8Data, U32 u32Len, const char *szDescFmt, ...);

#endif //__MT_USR_PUB_H__
