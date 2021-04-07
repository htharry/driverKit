#include "va_cp_priv.h"
#include "va_cp.h"
#include "va_mt.h"
#include "va_chan_base.h"

VA_CP_CB_S gstVaCpCb =
{
    .stGblCmdHead = LIST_HEAD_INIT(gstVaCpCb.stGblCmdHead),
    .stCpInstHead = LIST_HEAD_INIT(gstVaCpCb.stCpInstHead),
    .stCmdLock    = __MUTEX_INITIALIZER(gstVaCpCb.stCmdLock),
    .stInstLock   = __MUTEX_INITIALIZER(gstVaCpCb.stInstLock),
};

ULONG DRV_GetInstId(VOID)
{
    return current->tgid;
}

VOID VA_CP_RegInstEventNotify(VA_CP_INST_NOTIFY_S *pstInst)
{
    mutex_lock(&gstVaCpCb.stInstLock);
    list_add(&pstInst->stNode, &gstVaCpCb.stCpInstHead);
    mutex_unlock(&gstVaCpCb.stInstLock);
}

VOID VA_CP_UnRegInstEventNotify(VA_CP_INST_NOTIFY_S *pstInst)
{
    mutex_lock(&gstVaCpCb.stInstLock);
    list_del(&pstInst->stNode);
    mutex_unlock(&gstVaCpCb.stInstLock);
}

VOID VA_CpInstEventNotify(pid_t nTgid)
{
    VA_CP_INST_NOTIFY_S *pstInst;

    mutex_lock(&gstVaCpCb.stInstLock);
    list_for_each_entry(pstInst, &gstVaCpCb.stCpInstHead, stNode)
    {
        pstInst->pfnNotify(VA_EVENT_CLOSE, (ULONG)nTgid);
    }
    mutex_unlock(&gstVaCpCb.stInstLock);
}

VOID VA_CP_RegIoCmdProc(VA_GBL_IO_CMDS_CB_S *pstIoCmdCb)
{
    mutex_lock(&gstVaCpCb.stCmdLock);
    list_add(&pstIoCmdCb->stNode, &gstVaCpCb.stGblCmdHead);
    mutex_unlock(&gstVaCpCb.stCmdLock);
}

VOID VA_CP_UnRegIoCmdProc(VA_GBL_IO_CMDS_CB_S *pstIoCmdCb)
{
    mutex_lock(&gstVaCpCb.stCmdLock);
    list_del(&pstIoCmdCb->stNode);
    mutex_unlock(&gstVaCpCb.stCmdLock);
}

static long VA_CP_DoIoctl(U32 u32Cmd, VA_IO_CMD_S *pstIoCmd)
{
    VA_GBL_IO_CMDS_CB_S *pstIoCmdCb;
    LONG lRet = -EINVAL;

    VA_MT_DBG_MSG_PRINT(MT_IO_CMD_FMT, MT_IO_CMD_ARGS(u32Cmd, pstIoCmd));

    if ( unlikely(pstIoCmd->u8Len > VA_IO_CMD_MAX_ARG_LEN) )
    {
        atomic_inc(&gstVaCpCb.stCmdErrCnt);
        VA_LOG_ERR("Invalid param len %u of cmd 0x%x", pstIoCmd->u8Len, u32Cmd);
        return -EINVAL;
    }

    if ( u32Cmd >= CP_IOCTL_GLOBAL_BASE )
    {
        mutex_lock(&gstVaCpCb.stCmdLock);
        list_for_each_entry(pstIoCmdCb, &gstVaCpCb.stGblCmdHead, stNode)
        {
            if ( (u32Cmd >= pstIoCmdCb->u32CmdBase) && (u32Cmd < (pstIoCmdCb->u32CmdBase + pstIoCmdCb->u32Size)) )
            {
                atomic_inc(&gstVaCpCb.stGblCmdCnt);
                lRet = pstIoCmdCb->pfnIoCmd(u32Cmd, pstIoCmd);
                break;
            }
        }
        mutex_unlock(&gstVaCpCb.stCmdLock);
    }
    else
    {
        lRet = VA_CHAN_Ioctl(u32Cmd, pstIoCmd);
    }

    if ( lRet != 0 )
    {
        VA_MT_DBG_ERR_PRINT("\r\nFailed to execute Cmd:" MT_DBG_L MT_DBG_U32_HEX MT_DBG_CHAN,
                            "lRet",   lRet,
                            "Io Cmd", u32Cmd,
                            VA_CHAN_ARGS(&(pstIoCmd->stChanId)));

        atomic_inc(&gstVaCpCb.stCmdErrCnt);
    }

    return lRet;
}

static long VA_CP_ioctl(struct file *pstFile, U32 u32Cmd, unsigned long ulArg)
{
    VA_CP_INST_S *pstInst = (VA_CP_INST_S *)pstFile->private_data;
    VA_IO_CMD_S stIoCmd;
    LONG lRet;

    if ( pstInst->nTgidId != current->tgid )
    {
        VA_LOG_ERR("Send cp io cmd in a fork process!");
        return -EPERM;
    }

    atomic_inc(&gstVaCpCb.stCmdCnt);

    if ( copy_from_user(&stIoCmd, (VOID *)ulArg, sizeof(VA_IO_CMD_S)) )
    {
        atomic_inc(&gstVaCpCb.stCmdErrCnt);
        return -EIO;
    }

    lRet = VA_CP_DoIoctl(u32Cmd, &stIoCmd);
    return lRet;
}

LONG VA_CP_IoctlFrmKnl(CHAN_ID_S *pstChanId, U32 u32Cmd, VOID *pArg, U8 u8ArgLen)
{
    VA_IO_CMD_S stIoCmd;
    LONG lRet;
    mm_segment_t old_fs = get_fs();

    atomic_inc(&gstVaCpCb.stCmdCnt);

    stIoCmd.pParam   = pArg;
    stIoCmd.stChanId = *pstChanId;
    stIoCmd.u8Len    = u8ArgLen;

    if ( unlikely(stIoCmd.u8Len > VA_IO_CMD_MAX_ARG_LEN) )
    {
        atomic_inc(&gstVaCpCb.stCmdErrCnt);
        VA_LOG_ERR("Invalid param len %u of cmd 0x%x", stIoCmd.u8Len, u32Cmd);
        return -EINVAL;
    }

    set_fs(KERNEL_DS);

    lRet = VA_CP_DoIoctl(u32Cmd, &stIoCmd);

    set_fs(old_fs);

    return lRet;
}

static int VA_CP_mmap(struct file *pstFile, struct vm_area_struct *pstVma)
{
    return -ENOMEM;
}

static int VA_CP_FileOpen(struct inode *inode, struct file *pstFile)
{
    VA_CP_INST_S *pstInst = VA_Malloc(sizeof(VA_CP_INST_S));
    if ( pstInst == NULL )
    {
        return -ENOMEM;
    }

    pstInst->nTgidId = current->tgid;
    pstFile->private_data = pstInst;
    VA_GetModRef();
	return 0;
}

int VA_CP_FileRelease(struct inode *inode, struct file *pstFile)
{
    VA_CP_INST_S *pstInst = (VA_CP_INST_S *)pstFile->private_data;

    VA_CpInstEventNotify(pstInst->nTgidId);
    VA_Free(pstInst);
    VA_PutModRef();
	return 0;
}


static struct file_operations gstVaCpOps =
{
    .owner   = THIS_MODULE,
    .open    = VA_CP_FileOpen,
    .release = VA_CP_FileRelease,
    .mmap    = VA_CP_mmap,
    .unlocked_ioctl = VA_CP_ioctl,
};

static VA_FS_FILE_S gstVaCpFile =
{
    .szName   = "cp",
    .pstOps   = &gstVaCpOps,
};

#if 0
#endif

void VA_CP_MT_DispCb(ULONG ulExecId)
{
    MT_PRINT("\r\nDump Cp Info:");

    MT_PRINT(MT_FMT_I, "Cmd Cnt",     atomic_read(&gstVaCpCb.stCmdCnt));
    MT_PRINT(MT_FMT_I, "Gbl Cmd Cnt", atomic_read(&gstVaCpCb.stGblCmdCnt));
    MT_PRINT(MT_FMT_I, "Err Cmd Cnt", atomic_read(&gstVaCpCb.stCmdErrCnt));

    return;
}

#if 0
#endif

static int VA_CP_Init(VA_MOD_S *pstMod)
{
    VA_FS_RegFile(&gstVaCpFile);
    return 0;
}

static void VA_CP_Exit(void)
{
    VA_FS_UnRegFile(&gstVaCpFile);
    return;
}

VA_MOD_INIT(cp, VA_CP_Init, VA_CP_Exit, VA_INIT_LEVEL_CORE)

