#include "va_kern_pub.h"
#include "va_cfg_def.h"
#include "board.h"

static LONG VA_CFG_GetChanCap(VA_CHAN_CAP_S __user *pstUsrCap)
{
    VA_CHAN_CAP_S stChanCap;
    BOARD_CHAN_CFG_S stChanCfg;

    if ( copy_from_user(&stChanCap, pstUsrCap, sizeof(VA_CHAN_CAP_S)) )
    {
        return -EFAULT;
    }

    BOARD_GetChanCfg(stChanCap.u16ChanType, &stChanCfg);

    stChanCap.u16ChanNumPerPort = stChanCfg.u16ChanNumPerPort;
    stChanCap.u16TotChanNum     = stChanCfg.u16TotChanNum;
    stChanCap.u16PortNum        = stChanCfg.u16PortNum;

    if ( copy_to_user(pstUsrCap, &stChanCap, sizeof(VA_CHAN_CAP_S)) )
    {
        return -EFAULT;
    }

    return 0;
}

#if 0
#endif

static long VA_CFG_FileIoctl(struct file *pstFile, U32 u32Cmd, unsigned long ulArg)
{
    LONG lRet = -EINVAL;

    switch ( u32Cmd )
    {
        case CFG_IOCTL_GET_CHAN_CAP :
            lRet = VA_CFG_GetChanCap((VA_CHAN_CAP_S __user *)ulArg);
            break;
        default:
            VA_LOG_ERR("Got a error cmd %x in cfg", u32Cmd);
    }

    return lRet;
}

static struct file_operations gstVaCfgOps =
{
    .owner       = THIS_MODULE,
    .open        = VA_FS_FileOpen,
    .unlocked_ioctl  = VA_CFG_FileIoctl,
    .release    = VA_FS_FileRelease,
};

static VA_FS_FILE_S gstVaCfgFile =
{
    .szName   = "cfg",
    .pstOps   = &gstVaCfgOps,
};

static int VA_CFG_Init(VA_MOD_S *pstMod)
{
    VA_FS_RegFile(&gstVaCfgFile);
    return 0;
}

static void VA_CFG_Exit(void)
{
    VA_FS_UnRegFile(&gstVaCfgFile);
    return;
}

VA_MOD_INIT(cfg, VA_CFG_Init, VA_CFG_Exit, VA_INIT_LEVEL_MOD)

