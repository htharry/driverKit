#ifndef __MT_MGR_H__
#define __MT_MGR_H__

#include "mt_ua.h"
#include "mt_ca.h"

#define MT_CL_HASH_BITS         9
#define MT_CL_HASH_NUM          (0x1 << MT_CL_HASH_BITS)
#define MT_VIEW_HASH_BITS       5
#define MT_VIEW_HASH_NUM        (0x1 << MT_VIEW_HASH_BITS)

typedef struct tagMtCmdEntry
{
    struct list_head stNode;
    U8   u8Type;
    U8   u8Flg;
    char szName[MT_NAME_MAX_LEN + 1];
    char szDesc[MT_DESC_MAX_LEN + 1];
	union
	{
		S8  as8Id[MT_ID_MAX_LEN];
		U32 u32Id;
	};
    U64  u64MinVal;
    U64  u64MaxVal;
    struct tagMtCmdEntry *ppstEntryArr[0];
}MT_CMD_ENTRY_S;

typedef struct tagMtFileCb
{
    struct list_head   stNode;   // list file
    struct list_head   stClHead; // cmdline head
    VA_FS_FILE_S       stFile;
    atomic_t           stRef;
}MT_FILE_CB_S;

typedef struct tagMtCmdLine
{
    struct list_head   stHead;     // cmd entry list
    MT_FILE_CB_S       *pstFile;
    MT_CL_CALL_BACK_PF pfnCallBack;
    VOID               *pPrivCb;
    struct list_head   stClNode;   // cmdline node
    struct list_head   stFileNode; // cmdline node
    struct hlist_node  stHashNode;
    U64 u64CmdId;
    U64 u64ViewId;
}MT_CL_S;

typedef struct tagMtView
{
    struct hlist_node stNode;
    U64  u64ViewId;
    U64  u64ParentViewId;   // now always zero
    U32  u32ClCnt;
    U32  u32ViewCnt;
    char szName[MT_NAME_MAX_LEN + 1];
    char szDesc[MT_DESC_MAX_LEN + 1];
}MT_VIEW_S;

typedef struct tagMtMgr
{
    struct list_head stCbHead;  	// mt cb head
    struct list_head stCaHead;  	// mt ioctl agent head
    struct list_head stUaHead;    	// mt userspace agent head
    struct hlist_head astHashTbl[MT_CL_HASH_NUM];
    struct hlist_head astViewHashTbl[MT_VIEW_HASH_NUM];  // mt view head
    struct mutex stLock;
    struct mutex stUaLock;
    U64 u64CmdIdSeq;			// cmd id alloc
    wait_queue_head_t stWaitHead;
    U32 u32MaxPktOutLen;
}MT_MGR_S;

typedef struct tagMtCa // client agent
{
    struct list_head stNode;
    struct file *pstFile;
    BOOL bClChanged;
    BOOL bDebug;
}MT_CA_S;

typedef struct tagMtUaExecEnv
{
    struct list_head stNode;
    struct tagMtUa   *pstMtUa;

    union
    {
        VOID *pPrivCb;
        U64  u64PrivCb;
    };

    union
    {
        U64   u64CallBackAddr;
        VOID  *pCallBack;
    };
}MT_UA_EXEC_ENV_S;

typedef struct tagMtUa // userspace agent
{
    struct list_head  stNode;
    struct mutex      stLock;
    MT_CB_S           stMtCb;
    MT_UA_EXEC_ENV_S  *pstCurrEnv;
    ULONG 			  ulExecId;
    wait_queue_head_t stPollWaitHead;
    wait_queue_head_t stExecCmdWaitHead;
    BOOL              bReady;
    BOOL              bFinish;
    BOOL              bOpen;
	BOOL              bCmdInExec;
    struct list_head  stEnvHead;
}MT_UA_S;

extern int  MT_ChkFileCb(MT_FILE_CB_S *pstFile);
extern U64  MT_AllocCmdId(void);
extern VOID MT_GlobalLock(VOID);
extern VOID MT_GlobalUnLock(VOID);
extern VOID MT_RegUa(MT_UA_S *pstUa);
extern VOID MT_UnRegUa(MT_UA_S *pstUa);
extern VOID MT_RegCa(MT_CA_S *pstCa);
extern VOID MT_UnRegCa(MT_CA_S *pstCa);
extern U32  MT_GetCmdItemArraySize(const MT_CMD_ITEM_S *pstItem);
extern long MT_GetClCount(MT_CL_NUM_S __user *pstClNUm);
extern long MT_GetAllCmdIds(MT_CMD_IDS_INFO_S __user *pstIdsInfo);
extern long MT_GetAllViewIds(MT_CMD_IDS_INFO_S __user *pstIdsInfo);
extern long MT_GetClInfo(MT_CL_INFO_S __user *pstClInfo);
extern long MT_GetViewInfo(MT_VIEW_INFO_S __user *pstViewInfo);
extern LONG MT_ExecUserInputCmd(MT_INPUT_CMD_PARAM_S __user *pstCmdParam);
extern U32  MT_Poll(struct file *pstFile, poll_table *wait);
extern LONG MT_GetChangeStatus(MT_CA_S *pstCa, BOOL __user *pbVal);
extern VOID MT_RegUsrCmdLine(MT_CB_S *pstMtCb, VOID *pPrivCb, MT_CL_CALL_BACK_PF pfnCallBack, MT_UA_CL_INFO_S *pstClInfo);
extern VOID MT_DBG_PrintToEachCaFrmUsr(const char __user *szBuf, U32 u32Len);
#endif //__MT_MGR_H__
