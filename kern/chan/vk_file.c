#include "vk_chan.h"

typedef struct tagVkFileCb
{
    atomic_t  stCmdCnt;
    atomic_t  stCmdErrCnt;
}VK_FILE_CB_S;

VK_FILE_CB_S gstVkFileCb;

static int VK_FileOpen(struct inode *inode, struct file *pstFile)
{
    VA_GetModRef();
    return 0;
}

static long VK_FileIoctl(struct file *pstFile, U32 u32Cmd, unsigned long ulArg)
{
    VK_CHAN_S *pstVkChan;
    LONG lRet = -EINVAL;

    atomic_inc(&gstVkFileCb.stCmdCnt);

    if ( u32Cmd != VK_IOCTL_BIND_FD && pstFile->private_data == NULL )
    {
        atomic_inc(&gstVkFileCb.stCmdErrCnt);
        return -ENOENT;
    }

    pstVkChan = (VK_CHAN_S *)pstFile->private_data;

    switch ( u32Cmd )
    {
        case VK_IOCTL_BIND_FD :
            lRet = VK_BindChan(pstFile, (CHAN_ID_S __user *)ulArg);
            break;
        case VK_IOCTL_PUT_DATA :
            lRet = VK_PutData(pstVkChan, (VA_DATA_BUF_S __user *)ulArg);
            break;
        case VK_IOCTL_GET_DATA :
            lRet = VK_GetData(pstVkChan, (VA_DATA_BUF_S __user *)ulArg);
            break;
        default:
            VA_LOG_ERR("Got a error cmd %x", u32Cmd);
    }

    if ( lRet != 0 )
    {
        atomic_inc(&gstVkFileCb.stCmdErrCnt);
    }

    return lRet;
}

static U32 VK_FilePoll(struct file *pstFile, poll_table *wait)
{
    VK_CHAN_S *pstVkChan;
    FDD_CHAN_S   *pstFddChan;
    U32 u32Mask = 0;

    pstVkChan = (VK_CHAN_S *)pstFile->private_data;
    if (pstVkChan == NULL)
    {
        return POLLERR;
    }

    if (pstFile->f_mode & FMODE_READ)
    {
        poll_wait(pstFile, &pstVkChan->stWaitHead, wait);
        pstFddChan = &pstVkChan->stFddChan;
        if ( pstFddChan->pstOps && pstFddChan->pstOps->pfnDataReady )
        {
            u32Mask = pstFddChan->pstOps->pfnDataReady(&pstVkChan->stFddChan);
        }
        else
        {
            if ( FDD_GetBufQueueLen(&pstFddChan->stRxQueue) )
            {
                u32Mask = POLLIN | POLLRDNORM;
            }
        }
    }

    return u32Mask;
}

static int VK_FileMmap(struct file *pstFile, struct vm_area_struct *pstVma)
{
    ULONG ulCnt;
    ULONG ulPfn;
    ULONG i;
    int iRet;

    ulCnt = ((pstVma->vm_end - pstVma->vm_start) >> PAGE_SHIFT);

    pstVma->vm_flags |= (VM_IO | VM_PFNMAP);

    for ( i = 0; i < ulCnt; i++ )
    {
        ulPfn = vmalloc_to_pfn((VOID *)((pstVma->vm_pgoff + i) << PAGE_SHIFT));
        iRet  = vm_insert_pfn(pstVma, pstVma->vm_start + i * PAGE_SIZE, ulPfn);
        if ( iRet < 0 )
        {
            VA_LOG_ERR("Failed to map a page");
            return iRet;
        }
    }

    return 0;
}

int VK_FileRelease(struct inode *inode, struct file *pstFile)
{
    VA_ReleaseChan(pstFile);
    VA_PutModRef();
	return 0;
}

static struct file_operations gstVkOps =
{
    .owner       = THIS_MODULE,
    .open        = VK_FileOpen,
    .poll        = VK_FilePoll,
    .mmap        = VK_FileMmap,
    .unlocked_ioctl  = VK_FileIoctl,
    .release    = VK_FileRelease,
};

static VA_FS_FILE_S gstVkChanFile =
{
    .szName   = "vchan",
    .pstOps   = &gstVkOps,
};

int VK_FileInit(void)
{
    VA_FS_RegFile(&gstVkChanFile);
    return 0;
}

void VK_FileExit(void)
{
    VA_FS_UnRegFile(&gstVkChanFile);
}

#if 0
#endif

BOOL VK_IsSelfFd(INT iFd)
{
    struct file *pstFile;
    BOOL bIsSelfFd = FALSE;

    pstFile = fget((U32)iFd);
    if ( pstFile == NULL )
    {
        return bIsSelfFd;
    }

    if ( pstFile->f_op == &gstVkOps )
    {
        bIsSelfFd = TRUE;
    }

    fput(pstFile);
    return bIsSelfFd;
}


