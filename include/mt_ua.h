#ifndef __MT_UA_H__
#define __MT_UA_H__

#ifdef __cplusplus
extern "C" {
#endif

#define MT_UA_FILE                      "mt_ua"
#define MT_UA_CL_INFO_MAX_LEN           (8 * VA_KB)
#define MT_UA_PRINT_LEN                 (4 * VA_KB)

#define MT_UA_FLAG_NO_VIEW              (0x1)

enum
{
    MT_UA_VAL_TYPE_STR      = 0,
    MT_UA_VAL_TYPE_TEXT,
    MT_UA_VAL_TYPE_INT,
    MT_UA_VAL_TYPE_U64,
    MT_UA_VAL_TYPE_IPV4,
    MT_UA_VAL_TYPE_CHAN_ID,
};

#pragma pack (4)

typedef struct tagMtUaOpenParam
{
    U32  u32DebugCap;
    U32  u32Flag;
    char szName[MT_NAME_MAX_LEN + 1];
    char szDesc[MT_DESC_MAX_LEN + 1];
}MT_UA_OPEN_PARAM_S;

typedef struct tagMtUaClInfo
{
    VOID *pCallBack;
    VOID *pPrivCb;
    U32 u32Size;
    U32 u32EntryNum;
    MT_CMD_USR_ITEM_S astItem[0];
}MT_UA_CL_INFO_S;

typedef struct tagMtUaPrintParam
{
    VOID *pBuf;
    U32  u32Len;
}MT_UA_PRINT_PARAM_S;

typedef struct tagMtUaExecParam
{
    VOID *pCallBack;
    VOID *pPrivCb;
}MT_UA_EXEC_PARAM_S;

typedef struct tagMtUaVal
{
    S8   as8Id[MT_ID_MAX_LEN];
    U32  u32ValType;
    char *pBuf;
}MT_UA_VAL_S;

typedef struct tagMtUaValArr
{
    S8       as8Id[MT_ID_MAX_LEN];
    U32      u32Num;
    MT_VAL_U *pstValTbl;
}MT_UA_VAL_ARR_S;


#pragma pack ()

#ifdef  __cplusplus
}
#endif /* end of __cplusplus */

#endif //__MT_UA_H__
