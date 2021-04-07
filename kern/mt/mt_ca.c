#include "va_kern_pub.h"
#include "mt_kern_util.h"
#include "mt_mgr.h"
#include "mt_cmd_proc.h"
#include "mt_ca.h"

static int MT_CA_FileOpen(struct inode *inode, struct file *pstFile)
{
    MT_CA_S *pstCa;

    pstCa = (MT_CA_S *)VA_Zmalloc(sizeof(MT_CA_S));
    if ( pstCa == NULL )
    {
        return -ENOMEM;
    }

    pstCa->pstFile = MT_OpenOutputFile(pstFile);
    if (pstCa->pstFile == NULL)
    {
        VA_Free(pstCa);
        return -ENODEV;
    }

    pstFile->private_data = pstCa;
    MT_RegCa(pstCa);
    VA_GetModRef();
    return 0;
}

static int MT_CA_FileRelease(struct inode *inode, struct file *pstFile)
{
    MT_CA_S *pstCa;

    if ( pstFile->private_data )
    {
        pstCa = VA_PTR_TYPE(MT_CA_S, pstFile->private_data);
        MT_UnRegCa(pstCa);
        MT_PutOutputFile(pstCa->pstFile);
        VA_Free(pstCa);
        VA_PutModRef();
    }

    return 0;
}

static long MT_CA_FileIoctl(struct file *pstFile, U32 u32Cmd, unsigned long ulArg)
{
    MT_CA_S *pstCa;
    LONG lRet = 0;

    pstCa = VA_PTR_TYPE(MT_CA_S, pstFile->private_data);

    switch ( u32Cmd )
    {
        case MT_IOCTL_GET_CL_NUM :
            lRet = MT_GetClCount((MT_CL_NUM_S __user *)ulArg);
            break;
        case MT_IOCTL_GET_ALL_CMD_ID :
            lRet = MT_GetAllCmdIds((MT_CMD_IDS_INFO_S __user *)ulArg);
            break;
        case MT_IOCTL_GET_ALL_VIEW_ID:
            lRet = MT_GetAllViewIds((MT_CMD_IDS_INFO_S __user *)ulArg);
            break;
        case MT_IOCTL_GET_CL_INFO :
            lRet = MT_GetClInfo((MT_CL_INFO_S __user *)ulArg);
            break;
        case MT_IOCTL_GET_VIEW_INFO:
            lRet = MT_GetViewInfo((MT_VIEW_INFO_S __user *)ulArg);
            break;
        case MT_IOCTL_EXEC_CL :
            lRet = MT_ExecUserInputCmd((MT_INPUT_CMD_PARAM_S __user *)ulArg);
            break;
        case MT_IOCTL_SET_GLB_DBG:
            pstCa->bDebug = (ulArg != 0);
            break;
        case MT_IOCTL_GET_CL_CHANGE_INFO:
            lRet = MT_GetChangeStatus(pstCa, (BOOL __user *)ulArg);
            break;
        default:
            return -EINVAL;
    }

    return lRet;
}

static struct file_operations gstMtCaOps =
{
    .owner   = THIS_MODULE,
    .open    = MT_CA_FileOpen,
    .release = MT_CA_FileRelease,
    .poll    = MT_Poll,
    .unlocked_ioctl = MT_CA_FileIoctl,
};

static VA_FS_FILE_S gstMtCaFile =
{
    .szName   = "mt_ca",
    .pstOps   = &gstMtCaOps,
};

int MT_RegCaFile(void)
{
    VA_FS_RegFile(&gstMtCaFile);
    return 0;
}

void MT_UnRegCaFile(void)
{
    VA_FS_UnRegFile(&gstMtCaFile);
}
