#include "va_kern_pub.h"
#include "mt_kern_util.h"
#include "mt_mgr.h"
#include "mt_cmd_proc.h"
#include "mt_ua.h"

MT_UA_S *MT_AllocUa(void)
{
    MT_UA_S *pstUa = VA_Zmalloc(sizeof(MT_UA_S));
    if ( pstUa == NULL )
    {
        return NULL;
    }

    mutex_init(&pstUa->stLock);
    init_waitqueue_head(&pstUa->stPollWaitHead);
    init_waitqueue_head(&pstUa->stExecCmdWaitHead);
    INIT_LIST_HEAD(&pstUa->stMtCb.stClHead);
    INIT_LIST_HEAD(&pstUa->stMtCb.stFileHead);
    INIT_LIST_HEAD(&pstUa->stMtCb.stNode);
    INIT_LIST_HEAD(&pstUa->stNode);
    INIT_LIST_HEAD(&pstUa->stEnvHead);

    return pstUa;
}

VOID MT_FreeUa(MT_UA_S *pstUa)
{
    MT_UA_EXEC_ENV_S *pstEnv;
    MT_UA_EXEC_ENV_S *pstNextEnv;

	if ( pstUa->bCmdInExec ) // in process crash or kill
	{
		mutex_unlock(&pstUa->stLock);
	    pstUa->bCmdInExec = FALSE;
	}

    pstUa->bReady  = FALSE;
    pstUa->bFinish = TRUE;

    wake_up_all(&pstUa->stExecCmdWaitHead);
    //wake_up_all(&pstUa->stPollWaitHead);

    MT_UnRegMt(&pstUa->stMtCb);
    MT_UnRegUa(pstUa);

    mutex_lock(&pstUa->stLock);
    list_for_each_entry_safe(pstEnv, pstNextEnv, &pstUa->stEnvHead, stNode)
    {
        list_del(&pstEnv->stNode);
        VA_Free(pstEnv);
    }
    mutex_unlock(&pstUa->stLock);

    if ( pstUa->stMtCb.szName )
    {
        VA_Free(pstUa->stMtCb.szName);
    }

    if ( pstUa->stMtCb.szDesc)
    {
        VA_Free(pstUa->stMtCb.szDesc);
    }

    VA_Free(pstUa);
}

LONG MT_OpenUa(MT_UA_S *pstUa, MT_UA_OPEN_PARAM_S __user *pParam)
{
    MT_UA_OPEN_PARAM_S stParam;
    MT_CB_S *pstMtCb;
    INT iRet;

    if ( pstUa->bOpen )
    {
        return -EEXIST;
    }

    if ( copy_from_user(&stParam, pParam, sizeof(MT_UA_OPEN_PARAM_S)) )
    {
        return -EFAULT;
    }

    pstMtCb = &pstUa->stMtCb;
    pstMtCb->u32DebugCap = 0;
    VA_SET_STR_ARR_TERM(stParam.szName);
    pstMtCb->szName = MT_strdup(stParam.szName);
    pstMtCb->szDesc = MT_strdup(stParam.szDesc);
    if ( pstMtCb->szName == NULL || pstMtCb->szDesc == NULL )
    {
        if ( pstMtCb->szName )
        {
            VA_Free(pstMtCb->szName);
            pstMtCb->szName = NULL;
        }

        if ( pstMtCb->szDesc )
        {
            VA_Free(pstMtCb->szDesc);
            pstMtCb->szDesc = NULL;
        }

        return -ENOMEM;
    }

    if ( (stParam.u32Flag & MT_UA_FLAG_NO_VIEW) == 0 )
    {
        pstMtCb->u64ViewId = MT_AllocCmdId();
    }
    
    iRet = MT_RegMt(pstMtCb);
    if ( iRet != 0)
    {
        return iRet;
    }

    pstUa->bOpen = TRUE;
    return 0;
}

#if 0
#endif

static VOID MT_UA_ExecCmd(ULONG ulExecId)
{
    MT_UA_EXEC_ENV_S *pstEnv;
    MT_UA_S *pstUa;

    pstEnv = (MT_UA_EXEC_ENV_S *)MT_GetPrivCb(ulExecId);
    pstUa  = pstEnv->pstMtUa;
    mutex_lock(&pstUa->stLock);
    pstUa->pstCurrEnv = pstEnv;
    pstUa->ulExecId   = ulExecId;
    pstUa->bReady  = TRUE;
    pstUa->bFinish = FALSE;
	mutex_unlock(&pstUa->stLock);

    wake_up_all(&pstUa->stPollWaitHead);
    wait_event_timeout(pstUa->stExecCmdWaitHead, pstUa->bFinish == TRUE, 60 * 1000);

    mutex_lock(&pstUa->stLock);
    pstUa->pstCurrEnv = NULL;
    pstUa->ulExecId   = 0;
    pstUa->bFinish    = FALSE;
	pstUa->bReady  	  = FALSE;
    mutex_unlock(&pstUa->stLock);
}

#if 0
#endif

static LONG MT_UA_RegCmdLine(MT_UA_S *pstUa, MT_UA_CL_INFO_S __user *pParam)
{
    MT_UA_EXEC_ENV_S *pstEnv;
    MT_UA_CL_INFO_S *pstClInfo;
    U32 u32MaxEntryNum;

    pstClInfo = (MT_UA_CL_INFO_S *)VA_Malloc(MT_UA_CL_INFO_MAX_LEN);
    if ( pstClInfo == NULL )
    {
        return -ENOMEM;
    }

    if ( copy_from_user(pstClInfo, pParam, sizeof(MT_UA_CL_INFO_S)) )
    {
        return -EFAULT;
    }

    u32MaxEntryNum = (MT_UA_CL_INFO_MAX_LEN - sizeof(MT_UA_CL_INFO_S)) / sizeof(MT_CMD_USR_ITEM_S);

    if ( pstClInfo->u32Size > sizeof(MT_CMD_USR_ITEM_S) * pstClInfo->u32EntryNum || pstClInfo->u32Size == 0 )
    {
        return -EINVAL;
    }

    if ( pstClInfo->u32EntryNum > u32MaxEntryNum )
    {
        return -EINVAL;
    }

    if ( copy_from_user(pstClInfo->astItem, pParam->astItem, pstClInfo->u32Size) )
    {
        return -EFAULT;
    }

    pstEnv = VA_Zmalloc(sizeof(MT_UA_EXEC_ENV_S));
    if ( pstEnv == NULL )
    {
        VA_Free(pstClInfo);
    }

    pstEnv->pPrivCb   = pstClInfo->pPrivCb;
    pstEnv->pCallBack = pstClInfo->pCallBack;
    pstEnv->pstMtUa   = pstUa;

    mutex_lock(&pstUa->stLock);
    list_add(&pstEnv->stNode, &pstUa->stEnvHead);
    mutex_unlock(&pstUa->stLock);

    MT_RegUsrCmdLine(&pstUa->stMtCb, pstEnv, MT_UA_ExecCmd, pstClInfo);

    VA_Free(pstClInfo);

    return 0;
}

static LONG MT_UA_GetExecParam(MT_UA_S *pstUa, MT_UA_EXEC_PARAM_S __user *pstParam)
{
    MT_UA_EXEC_PARAM_S stParam;

	mutex_lock(&pstUa->stLock);
    if ( pstUa->bReady == FALSE || pstUa->pstCurrEnv == NULL )
    {
		mutex_unlock(&pstUa->stLock);
        return -ENOENT;
    }

    stParam.pCallBack = pstUa->pstCurrEnv->pCallBack;
    stParam.pPrivCb   = pstUa->pstCurrEnv->pPrivCb;

    if ( copy_to_user(pstParam, &stParam, sizeof(MT_UA_EXEC_PARAM_S)))
    {
		mutex_unlock(&pstUa->stLock);
        return -EFAULT;
    }

	pstUa->bCmdInExec = TRUE;
    return 0;
}

static LONG MT_UA_EndExec(MT_UA_S *pstUa)
{
    pstUa->bReady  = FALSE;
    pstUa->bFinish = TRUE;
	pstUa->bCmdInExec = FALSE;
	mutex_unlock(&pstUa->stLock);

    wake_up_all(&pstUa->stExecCmdWaitHead);
    return 0;
}

static LONG MT_UA_GetVal(MT_UA_S *pstUa, MT_UA_VAL_S __user *pstParam)
{
    MT_UA_VAL_S stUaVal;
    LONG lRet = -EINVAL;
    const char *szBuf;
    char szTmpBuf[MT_NAME_MAX_LEN] = {0};

    if ( copy_from_user(&stUaVal, pstParam, sizeof(MT_UA_VAL_S)) )
    {
	    return -EFAULT;
    }

    switch ( stUaVal.u32ValType )
    {
        case MT_UA_VAL_TYPE_STR :
        case MT_UA_VAL_TYPE_TEXT :
            lRet = MT_GetStrVal(pstUa->ulExecId, stUaVal.as8Id, &szBuf);
            if ( lRet == 0 )
            {
                strncpy(szTmpBuf, szBuf, MT_NAME_MAX_LEN);
            }
            break;
        case MT_UA_VAL_TYPE_INT :
            lRet = MT_GetIntVal(pstUa->ulExecId, stUaVal.as8Id, (ULONG *)szTmpBuf);
            break;
        case MT_UA_VAL_TYPE_U64 :
            lRet = MT_GetU64Val(pstUa->ulExecId, stUaVal.as8Id, (U64 *)szTmpBuf);
            break;
        case MT_UA_VAL_TYPE_IPV4 :
            lRet = MT_GetIpv4Val(pstUa->ulExecId, stUaVal.as8Id, (BE32 *)szTmpBuf);
            break;
        case MT_UA_VAL_TYPE_CHAN_ID :
            lRet = MT_GetChanIdVal(pstUa->ulExecId, stUaVal.as8Id, (CHAN_ID_S *)szTmpBuf);
            break;
        default:
            lRet = -EINVAL;
    }

    if ( lRet == 0 )
    {
        if ( copy_to_user(stUaVal.pBuf, szTmpBuf, MT_NAME_MAX_LEN) )
        {
            lRet = -EFAULT;
        }
    }

    return lRet;
}

static LONG MT_UA_GetValArr(MT_UA_S *pstUa, MT_UA_VAL_ARR_S __user *pstValArr)
{
    MT_UA_VAL_ARR_S stValArr;
    MT_VAL_U *pstValTbl;
    LONG lRet;

    if ( copy_from_user(&stValArr, pstValArr, sizeof(MT_UA_VAL_ARR_S)) )
    {
        return -EFAULT;
    }

    pstValTbl = VA_Zmalloc(stValArr.u32Num * sizeof(MT_VAL_U));
    if ( pstValTbl == NULL )
    {
        return -ENOMEM;
    }

    lRet = MT_GetAllValById(pstUa->ulExecId, stValArr.as8Id, pstValTbl, &stValArr.u32Num);
    if ( lRet < 0 )
    {
        VA_Free(pstValTbl);
        return lRet;
    }

    if ( copy_to_user(stValArr.pstValTbl, pstValTbl, stValArr.u32Num * sizeof(MT_VAL_U)) )
    {
        VA_Free(pstValTbl);
        return -EFAULT;
    }

    if ( copy_to_user(pstValArr, &stValArr, sizeof(MT_UA_VAL_ARR_S)) )
    {
        VA_Free(pstValTbl);
        return -EFAULT;
    }

    VA_Free(pstValTbl);
    return 0;
}

static LONG MT_UA_Print(MT_UA_S *pstUa, MT_UA_PRINT_PARAM_S __user *pstParam)
{
    MT_UA_PRINT_PARAM_S stParam;

    if ( copy_from_user(&stParam, pstParam, sizeof(MT_UA_PRINT_PARAM_S)))
    {
        return -EFAULT;
    }

    if ( stParam.pBuf == NULL || stParam.u32Len > MT_UA_PRINT_LEN )
    {
        return -EINVAL;
    }

    MT_KnlWirteFromUsr(pstUa->ulExecId, stParam.pBuf, stParam.u32Len);
    return 0;
}

static LONG MT_UA_DBG_Print(MT_UA_PRINT_PARAM_S __user *pstParam)
{
    MT_UA_PRINT_PARAM_S stParam;

    if ( copy_from_user(&stParam, pstParam, sizeof(MT_UA_PRINT_PARAM_S)))
    {
        return -EFAULT;
    }

    if ( stParam.pBuf == NULL || stParam.u32Len > MT_UA_PRINT_LEN )
    {
        return -EINVAL;
    }

    MT_DBG_PrintToEachCaFrmUsr(stParam.pBuf, stParam.u32Len);
    return 0;
}

#if 0
#endif

static int MT_UA_FileOpen(struct inode *inode, struct file *pstFile)
{
    MT_UA_S *pstUa;

    pstUa = MT_AllocUa();
    if ( pstUa == NULL )
    {
        return -ENOMEM;
    }

    pstFile->private_data = pstUa;
    VA_GetModRef();
    return 0;
}

static int MT_UA_FileRelease(struct inode *inode, struct file *pstFile)
{
    MT_UA_S *pstUa;

    pstUa = VA_PTR_TYPE(MT_UA_S, pstFile->private_data);
    if ( pstUa == NULL )
    {
        return 0;
    }

    MT_FreeUa(pstUa);
    pstFile->private_data = NULL;
    VA_PutModRef();
    return 0;
}

static long MT_UA_FileIoctl(struct file *pstFile, U32 u32Cmd, unsigned long ulArg)
{
    MT_UA_S *pstUa;
    LONG lRet = 0;

    pstUa = VA_PTR_TYPE(MT_UA_S, pstFile->private_data);
    if ( pstUa == NULL )
    {
        return -ENOENT;
    }

    if ( pstUa->bOpen == FALSE && u32Cmd != MT_IOCTL_UA_OPEN )
    {
        return -ENOENT;
    }

    switch ( u32Cmd )
    {
        case MT_IOCTL_UA_OPEN :
            lRet = MT_OpenUa(pstUa, (MT_UA_OPEN_PARAM_S __user *)ulArg);
            break;
        case MT_IOCTL_UA_REG_CL :
            lRet = MT_UA_RegCmdLine(pstUa, (MT_UA_CL_INFO_S __user *)ulArg);
            break;
        case MT_IOCTL_UA_GET_VAL :
            lRet = MT_UA_GetVal(pstUa, (MT_UA_VAL_S __user *)ulArg);
            break;
        case MT_IOCTL_UA_GET_VAL_ARR:
            lRet = MT_UA_GetValArr(pstUa, (MT_UA_VAL_ARR_S __user *)ulArg);
            break;
        case MT_IOCTL_UA_GET_EXEC_PARAM :
            lRet = MT_UA_GetExecParam(pstUa, (MT_UA_EXEC_PARAM_S __user *)ulArg);
            break;
        case MT_IOCTL_UA_END_EXEC:
            lRet = MT_UA_EndExec(pstUa);
            break;
        case MT_IOCTL_UA_PRINT :
            lRet = MT_UA_Print(pstUa, (MT_UA_PRINT_PARAM_S __user *)ulArg);
            break;
        case MT_IOCTL_UA_DBG_PRINT :
            lRet = MT_UA_DBG_Print((MT_UA_PRINT_PARAM_S __user *)ulArg);
            break;
        default:
            return -EINVAL;
    }

    return lRet;
}

U32 MT_UA_Poll(struct file *pstFile, poll_table *wait)
{
    MT_UA_S *pstUa;
    U32 u32Mask = 0;

    pstUa = VA_PTR_TYPE(MT_UA_S, pstFile->private_data);

    poll_wait(pstFile, &pstUa->stPollWaitHead, wait);
    if ( pstUa->bReady )
    {
        u32Mask = POLLIN | POLLRDNORM;
    }

    return u32Mask;
}

static struct file_operations gstMtUaOps =
{
	.owner      = THIS_MODULE,
    .open       = MT_UA_FileOpen,
    .release    = MT_UA_FileRelease,
    .poll       = MT_UA_Poll,
    .unlocked_ioctl = MT_UA_FileIoctl,
};

static VA_FS_FILE_S gstMtUaFile =
{
    .szName   = "mt_ua",
    .pstOps   = &gstMtUaOps,
};

int MT_RegUaFile(void)
{
    VA_FS_RegFile(&gstMtUaFile);
    return 0;
}

void MT_UnRegUaFile(void)
{
    VA_FS_UnRegFile(&gstMtUaFile);
}
