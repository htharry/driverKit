#ifndef __MT_CA_H__
#define __MT_CA_H__

#ifdef  __cplusplus
extern "C"{
#endif

#pragma pack (4)

typedef struct tagMtClNum
{
    U64 u64ViewId;
    U32 u32ClCnt;
    U32 u32ViewCnt;
}MT_CL_NUM_S;

typedef struct tagMtCmdIdsInfo
{
    U64  u64ViewId;
    U32  u32Num;
    U32  u32Rsvd;
    U64  au64CmdId[0];
}MT_CMD_IDS_INFO_S;

typedef struct tagMtClInfo
{
    U64  u64CmdId;
    U32  u32Size;
    U32  u32EntryNum;
    U64  u64ViewId;
    MT_CMD_USR_ITEM_S astItem[0];
}MT_CL_INFO_S;

typedef struct tagMtViewInfo
{
    U64  u64ViewId;
    char szName[MT_NAME_MAX_LEN + 1];
    char szDesc[MT_DESC_MAX_LEN + 1];
}MT_VIEW_INFO_S;

typedef struct tagMtInputCmdParam
{
    U32  u32BufLen;
    U64  u64CmdId;
    char szCmd[MT_MAX_INPUT_LEN + 1];
}MT_INPUT_CMD_PARAM_S;

#pragma pack ()

#ifdef  __cplusplus
}
#endif /* end of __cplusplus */

#endif //__MT_CA_H__
