#ifndef __MT_CMD_PROC_H__
#define __MT_CMD_PROC_H__

typedef struct tagMtPrivVal
{
    struct list_head stNode;
    char   szStr[MT_STR_MAX_LEN + 1];
    S8     as8Id[MT_ID_MAX_LEN];
    U8     u8ItemType;
}MT_VAL_PRIV_S;

typedef struct tagMtValHead
{
    struct list_head stHead;
    struct file *pstFile;
    VOID *pPrivCb;
}MT_VAL_HEAD_S;

extern ssize_t MT_KnlWirte(struct file *pstFile, const char *buf, size_t count);
extern VOID MT_ParseCmdLine(MT_FILE_CB_S *pstFile, struct file *pstOutFile, char *szCmdLine);
extern ssize_t MT_PrintCmdlineFmt(MT_FILE_CB_S *pstFile, char *pBuf, U32 u32Len);
extern LONG MT_ExecUserCmd(MT_CL_S *pstCl, struct file *pstOutFile, char *szCmdLine);
extern ssize_t MT_KnlWirteFromUsr(ULONG ulExecId,  const char __user *pBuf, size_t count);

#endif //__MT_PROC_H__
