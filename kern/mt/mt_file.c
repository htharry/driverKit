#include "va_kern_pub.h"
#include <linux/vmalloc.h>
#include "mt_kern_util.h"
#include "mt_mgr.h"
#include "mt_cmd_proc.h"

static int MT_FileOpen(struct inode *inode, struct file *pstFile)
{
    pstFile->private_data = inode->i_private;
    return 0;
}

static ssize_t MT_FileRead(struct file *pstFile, char __user *buffer, size_t count, loff_t *ppos)
{
    char *szBuf = NULL;
    ssize_t nRet;
    ssize_t nCpyLen;
    ssize_t nPos;

    szBuf = VA_VMalloc(MT_FMT_MAX_LEN);
    if ( szBuf == NULL )
    {
        return -ENOMEM;
    }

    nRet = MT_PrintCmdlineFmt(VA_PTR_TYPE(MT_FILE_CB_S, pstFile->private_data), szBuf, MT_FMT_MAX_LEN);
    if ( nRet <= 0 )
    {
        return nRet;
    }

    nPos = *ppos;
    if ( nPos < 0 || nPos >= nRet )
    {
        VA_VFree(szBuf);
        return 0;
    }

    nCpyLen = nRet - nPos;
    if ( nCpyLen > count )
    {
        nCpyLen = count;
    }

    if ( copy_to_user(buffer, szBuf + nPos, nCpyLen) )
    {
        VA_VFree(szBuf);
        return EFAULT;
    }

    VA_VFree(szBuf);
    return nCpyLen;
}

static ssize_t MT_FileWrite(struct file *pstFile, const char __user *buffer, size_t count, loff_t *ppos)
{
    struct file *pstOutFile;
    char szBuf[MT_MAX_INPUT_LEN + 1];

    if ( count >= MT_MAX_INPUT_LEN )
    {
        return -EINVAL;
    }

    if ( copy_from_user(szBuf, buffer, count) )
    {
        return -EFAULT;
    }

    pstOutFile = MT_OpenOutputFile(pstFile);
    if (pstOutFile == NULL)
    {
        return -ENODEV;
    }

    szBuf[count] = 0;
    MT_ParseCmdLine(VA_PTR_TYPE(MT_FILE_CB_S, pstFile->private_data), pstOutFile, szBuf);
    MT_PutOutputFile(pstOutFile);

    return count;
}

static struct file_operations gstMtOps =
{
    .owner      = THIS_MODULE,
    .open       = MT_FileOpen,
    .read       = MT_FileRead,
    .write      = MT_FileWrite,
    .release    = VA_FS_FileRelease,
    .unlocked_ioctl     = NULL,
};

struct file_operations *MT_GetFileOps(void)
{
    return &gstMtOps;
}
