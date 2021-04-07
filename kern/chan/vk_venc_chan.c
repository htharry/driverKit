#include "vk_chan.h"

static int VK_VENC_ChanInit(FDD_CHAN_S *pstFddChan)
{
    VK_CHAN_S *pstChan;
    int iRet;

    pstFddChan->u16DataType = VA_STREAM_TYPE_ES;
    FDD_SetChanRxLimitLen(pstFddChan, 1 * VA_MB);
    FDD_SetChanRxMode(pstFddChan, FDD_BUF_HEAD_MOD_SG);

    pstChan = VA_PTR_TYPE(VK_CHAN_S, pstFddChan);
    iRet = VK_ChanInit(pstChan, TRUE);
    if ( iRet < 0 )
    {
        return iRet;
    }

    return 0;
}

static VOID VK_VENC_ChanRelease(FDD_CHAN_S *pstFddChan)
{
    return;
}

static LONG VK_VENC_Ioctl(FDD_CHAN_S *pstFddChan, U32 u32Cmd, U16 u16Len, void __user *pParam)
{
    return 0;
}

static FDD_PORT_OPS_S gstVencPortOps =
{
    .pfnOutPkt = VK_OutEsFrame,
};

static FDD_CHAN_OPS_S gstVencChanOps =
{
    .pfnInit       = VK_VENC_ChanInit,
    .pfnRelease    = VK_VENC_ChanRelease,
    .pfnInitPort   = VA_CHAN_InitPort,
    .pfnDataReady  = NULL,
    .pfnMtDispChan = VK_MtDispChan,
};

static VA_CHAN_TYPE_CB_S gstVencTypeCb =
{
    .szName        = "venc",
    .u16ChanType   = VA_CHAN_TYPE_VENC,
    .u16ChanCbSize = sizeof(VK_CHAN_S),
    .pfnIoctl      = VK_VENC_Ioctl,
    .pstChanOps    = &gstVencChanOps,
    .pstPortOps    = &gstVencPortOps,
};

static int VK_VENC_Init(VA_DEV_S *pstDev)
{
    VA_CHAN_DEV_S *pstChanDev = VA_PTR_TYPE(VA_CHAN_DEV_S, pstDev);
    int iRet;

    iRet = VA_CHAN_CreatePortTbl(pstChanDev, TRUE);
    if ( iRet < 0 )
    {
        return iRet;
    }

    return 0;
}

static void VK_VENC_Exit(VA_DEV_S *pstDev)
{
    VA_CHAN_DestroyPortTbl((VA_CHAN_DEV_S *)pstDev);
}

VA_DRV_S gstVkVencDrv =
{
    .u32Id     = VA_DEV_ID_VCHAN,
    .u32PrivId = VA_CHAN_TYPE_VENC,
    .szName    = "Venc",
    .init      = VK_VENC_Init,
    .exit      = VK_VENC_Exit,
    .mt        = VA_CHAN_DispChanDev,
};

static void VK_VENC_ModExit(void)
{
    VA_UnRegDrv(&gstVkVencDrv);
}

static int VK_VENC_ModInit(VA_MOD_S *pstMod)
{
    VA_CHAN_RegTypeCb(&gstVencTypeCb);
    VA_RegDrv(&gstVkVencDrv);
    return 0;
}

VA_MOD_INIT(venc, VK_VENC_ModInit, VK_VENC_ModExit, VA_INIT_LEVEL_DRV)

