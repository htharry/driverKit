#ifndef __MT_USR_COMPAT_H__
#define __MT_USR_COMPAT_H__

#ifdef __cplusplus
extern "C" {
#endif

#define MT_CMD_KEY_MAX_LEN              MT_NAME_MAX_LEN
#define MT_CMD_HELP_MAX_LEN             MT_DESC_MAX_LEN
#define MT_MOD_NAME_MAX_LEN             MT_CMD_KEY_MAX_LEN
#define MT_MOD_DESC_MAX_LEN             MT_CMD_HELP_MAX_LEN
#define MT_INVAL_EXEC_ID                0

#define MT_CMD_DISP_ELEM                MT_DEF_ITEM_DISP
#define MT_CMD_READ_ELEM                MT_DEF_ITEM_READ
#define MT_CMD_WRITE_ELEM               MT_DEF_ITEM_WRITE
#define MT_CMD_SET_ELEM                 MT_DEF_ITEM_SET
#define MT_CMD_DEBUG_ELEM               MT_DEF_ITEM_DEBUG
#define MT_CMD_UNDO_ELEM                MT_DEF_ITEM_UNDO
#define MT_CMD_STAT_ELEM                MT_DEF_ITEM_STAT
#define MT_CMD_CB_ELEM                  MT_DEF_ITEM_CB
#define MT_CMD_RESET_ELEM               MT_DEF_ITEM_RESET
#define MT_CMD_ALL_ELEM                 MT_DEF_ITEM_ALL
#define MT_CMD_REG_ELEM                 MT_DEF_ITEM_REG
#define MT_CMD_CHK_ELEM                 MT_DEF_ITEM_CHK
#define MT_CMD_RES_ELEM                 MT_DEF_ITEM_RES
#define MT_CMD_DUMP_ELEM                MT_DEF_ITEM_DUMP
#define MT_CMD_LOAD_ELEM                MT_DEF_ITEM_LOAD
#define MT_CMD_TEST_ELEM                MT_DEF_ITEM_TEST

enum MtCmdIdBase
{
    MT_CMD_ID_BASE_GLOBAL   = 0x00010000,
    MT_CMD_ID_BASE_TEST     = 0x00020000,
    MT_CMD_ID_BASE_SR       = 0x00030000,
    MT_CMD_ID_CLI_LOG       = 0x00040000,
    MT_CMD_ID_BASE_DRVLIB   = 0x00050000,
    MT_CMD_ID_BASE_BS_MEDIA = 0x00060000,
    MT_CMD_ID_BASE_DRV_CFG  = 0x00070000,
    MT_CMD_ID_BASE_FPGA     = 0x00080000,
    MT_CMD_ID_BASE_BM       = 0x00090000,
    MT_CMD_ID_BASE_BS_CFG   = 0x000a0000,
    MT_CMD_ID_BASE_AM_CFG   = 0x000b0000,
    MT_CMD_ID_BASE_NET_SDK  = 0x000c0000,
};

enum MtCmdGlobalId
{
    MT_CMD_ID_MOD_NAME   = MT_CMD_ID_BASE_GLOBAL,
    MT_CMD_ID_ALL,
    MT_CMD_ID_LEN,
    MT_CMD_ID_VAL,
    MT_CMD_ID_VAL_HEX,
    MT_CMD_ID_ADDR,
    MT_CMD_ID_REG_ADDR,
    MT_CMD_ID_REG_VAL,
    MT_CMD_ID_REG_MASK,
    MT_CMD_ID_REG_RES_NAME,
    MT_CMD_ID_REG_OP_U8,
    MT_CMD_ID_REG_OP_U16,
    MT_CMD_ID_REG_OP_U32,
    MT_CMD_ID_REG_OP_U64,
    MT_CMD_ID_DBG_INFO,
    MT_CMD_ID_DBG_PKT,
    MT_CMD_ID_DBG_EVENT,
    MT_CMD_ID_DBG_WARN,
    MT_CMD_ID_DBG_MSG,
    MT_CMD_ID_CHIP,
    MT_CMD_ID_BUS,
    MT_CMD_ID_DEV,
    MT_CMD_ID_COUNT,
    MT_CMD_ID_SWAP,
    MT_CMD_ID_INDX,
};

enum MtDebugSwitch
{
    MT_DEBUG_INFO       = MT_DBG_INFO,
    MT_DEBUG_PKT        = MT_DBG_PKT,
    MT_DEBUG_WARN       = MT_DBG_ERR,
    MT_DEBUG_MSG        = MT_DBG_MSG,
};

#define MT_USR_DBG_IS_ON(pstCb, u32Level)    ((NULL != (pstCb)) && (NULL != ((pstCb)->pstUsrCb)) && (0 != (((pstCb)->pstUsrCb)->u32Debug & (u32Level))))
#define DRVLIB_MT_PRINT(szFmt, args...)      MT_Print(ulExecId, szFmt, ##args)
#define DRVLib_MtPrint                       MT_Print


#define MT_USR_DBG_PRINT_INFO(pstCb, szFmt, args...)  \
        MT_DBG_INFO_PRINT((pstCb)->ulMtHndl, szFmt, ##args)

#define __MT_USR_DBG_PRINT_INFO(pstCb, szFmt, args...)  \
        do { \
            if ( MT_USR_DBG_IS_ON((pstCb), MT_DBG_INFO) ) \
            { \
                __MT_DBG_Print((pstCb)->ulMtHndl, szFmt, ##args); \
            } \
        }while(0)

#define MT_USR_DBG_PRINT_MSG(pstCb, pMsg, pfnPrintMsg)  \
        do { \
            if ( MT_USR_DBG_IS_ON((pstCb), MT_DBG_MSG) ) \
            { \
                MT_DBG_PrintMsg((pstCb), pMsg, pfnPrintMsg); \
            } \
        }while(0)

#define MT_USR_DBG_PRINT_WARN(pstCb, szFmt, args...)  \
        MT_DBG_ERR_PRINT((pstCb)->ulMtHndl, szFmt, ##args)

#define __MT_USR_DBG_PRINT_WARN(pstCb, szFmt, args...) \
        do { \
            if ( MT_USR_DBG_IS_ON((pstCb), MT_DBG_ERR) ) \
            { \
                __MT_DBG_Print((pstCb)->ulMtHndl, szFmt, ##args); \
            } \
        }while(0)

#define MT_USR_DBG_PRINT_PKT(pstCb, pcDesc, pucData, u32Len)  \
        MT_DBG_PKT_PRINT((pstCb)->ulMtHndl, pcDesc, pucData, u32Len)

#define MT_USR_DEF_APP_CB(__szName, __szDesc, __u32DebugCap, __pfnDispCb, __pfnDispStat, __pfnResetStat) \
{ \
    .szName         = __szName,          \
    .szDesc         = __szDesc,          \
    .u32DebugCap    = __u32DebugCap,     \
    .pfnDispCb      = __pfnDispCb,       \
    .pfnDispStat    = __pfnDispStat,     \
    .pfnResetStat   = __pfnResetStat,    \
}

#define MT_TITLE_CMD_ELEM_INIT(u32Id, szName, szDesc)    \
                             MT_DEF_CMD_ELEM(MT_ITEM_TYPE_TEXT, u32Id, szName, szDesc, 0, 0)
#define MT_STR_CMD_ELEM_INIT(u32Id, szDesc, ulMaxLen)     \
                             MT_DEF_CMD_ELEM(MT_ITEM_TYPE_STR, u32Id, "<STRING>", szDesc, 1, ulMaxLen)
#define MT_INT_CMD_ELEM_INIT(u32Id, szDesc, ulMin, ulMax) \
                             MT_DEF_CMD_ELEM(MT_ITEM_TYPE_INT, u32Id, "<INTEGER>", szDesc, ulMin, ulMax)
#define MT_HEX_CMD_ELEM_INIT(u32Id, szDesc, ulMin, ulMax) \
                             MT_DEF_CMD_ELEM(MT_ITEM_TYPE_HEX, u32Id, "<HEX>", szDesc, ulMin, ulMax)
#define MT_MOD_CMD_ELEM_INIT(szName, szDesc)  MT_TITLE_CMD_ELEM_INIT(MT_CMD_ID_MOD_NAME, szName, szDesc)

#define MT_DEF_CMD_ELEM(u8CmdType, u32Id, szName, szDesc, ulMin, ulMax) \
    (MT_CMD_ITEM_S){ 	        \
		u8CmdType,   		    \
		0,						\
		0,						\
		{{((u32Id >> 24) &0xFF), ((u32Id >> 16) &0xFF), ((u32Id >> 8) &0xFF), (u32Id & 0xFF)}},  \
		{ulMin},			    \
		ulMax,					\
		szName,					\
		szDesc,					\
	}


#define DRV_LIB_REG_MT_CMD(pstCb, pfnProc, pstMtElement, args...)  \
         __MT_REG_CMD_LINE(pstCb->ulMtHndl, pstCb, pfnProc, pstMtElement, ##args)

#define BM_REG_MT_CMD(pfnProc, pstMtElement, args...) \
         DRV_LIB_REG_MT_CMD(pstMtAppCb, pfnProc, pstMtElement, ##args)

typedef MT_CMD_ITEM_S MT_CMD_ELEMENT_S;
struct tagMtAppCb;

typedef VOID (*MT_USR_DISP_MSG_INFO_PF)(IN struct tagMtAppCb *pstCb, IN const char *szFmt, ...);
typedef VOID (*MT_APP_USR_PRINT_MSG_PF)(IN struct tagMtAppCb *pstCb, IN VOID *pMsg, IN MT_USR_DISP_MSG_INFO_PF pfnDispInfo);

typedef VOID (*MT_USR_CALL_BACK_PF)(IN ULONG ulExecId);

struct tagMtUsrCb;

typedef struct tagMtAppCb
{
    char  szName[MT_NAME_MAX_LEN];
    char  szDesc[MT_DESC_MAX_LEN];
    U32   u32DebugCap;
    MT_USR_CALL_BACK_PF pfnDispCb;
    MT_USR_CALL_BACK_PF pfnDispStat;
    MT_USR_CALL_BACK_PF pfnResetStat;

    union
    {
        struct tagMtUsrCb *pstUsrCb;
        ULONG ulMtHndl;
    };

    VOID *pData;
}MT_APP_CB_S;

extern INT  DRVLib_GetMtFd(MT_APP_CB_S *pstCb);
extern INT  DRVLib_RegAppMtCb(MT_APP_CB_S *pstCb);
extern VOID DRVLib_ExecMtCmd(MT_APP_CB_S *pstCb);
extern INT  DRVLib_GetValFrmMtParams(ULONG ulExecId, U32 u32Id, ULONG *pulVal);
extern INT  DRVLib_GetStrFrmMtParams(ULONG ulExecId, U32 u32Id, char szStr[MT_STR_MAX_LEN]);
extern INT  DRVLib_GetTitleFrmMtParams(ULONG ulExecId, U32 u32Id, char szStr[MT_STR_MAX_LEN]);

extern VOID MT_CMD_ELEM_INIT_For_CXX(MT_CMD_ELEMENT_S *pstCmdElement, U32 u32CmdType, U32 u32Id, const char *szName, const char *szDesc, ULONG ulMinVal, ULONG ulMaxVal);

#ifdef __cplusplus
}
#endif   /* __cplusplus */

#endif // __MT_USR_COMPAT_H__

