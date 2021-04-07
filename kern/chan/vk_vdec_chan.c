#include "vk_chan.h"

static int VK_VDEC_ChanInit(FDD_CHAN_S *pstFddChan)
{
    VK_CHAN_S *pstChan;
    int iRet;

    pstFddChan->u16DataType = VA_STREAM_TYPE_ES;
    FDD_SetChanRxLimitLen(pstFddChan, 1 * VA_MB);
    FDD_SetChanRxMode(pstFddChan, FDD_BUF_HEAD_MOD_SG);

    pstChan = VA_PTR_TYPE(VK_CHAN_S, pstFddChan);
    iRet = VK_ChanInit(pstChan, FALSE);
    if ( iRet < 0 )
    {
        return iRet;
    }

    return 0;
}

static VOID VK_VDEC_ChanRelease(FDD_CHAN_S *pstFddChan)
{
    return;
}

static LONG VK_VDEC_Ioctl(FDD_CHAN_S *pstFddChan, U32 u32Cmd, U16 u16Len, void __user *pParam)
{
    return 0;
}

static FDD_PORT_OPS_S gstVdecPortOps =
{
    .pfnOutPkt = VK_OutEsFrame,
};

static FDD_CHAN_OPS_S gstVdecChanOps =
{
    .pfnInit       = VK_VDEC_ChanInit,
    .pfnRelease    = VK_VDEC_ChanRelease,
    .pfnInitPort   = VA_CHAN_InitPort,
    .pfnDataReady  = VK_DataReady,
    .pfnMtDispChan = VK_MtDispChan,
};

static VA_CHAN_TYPE_CB_S gstVdecTypeCb =
{
    .szName        = "vdec",
    .u16ChanType   = VA_CHAN_TYPE_VDEC,
    .u16ChanCbSize = sizeof(VK_CHAN_S),
    .pfnIoctl      = VK_VDEC_Ioctl,
    .pstChanOps    = &gstVdecChanOps,
    .pstPortOps    = &gstVdecPortOps,
};

static int VK_VDEC_Init(VA_DEV_S *pstDev)
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

static void VK_VDEC_Exit(VA_DEV_S *pstDev)
{
    VA_CHAN_DestroyPortTbl((VA_CHAN_DEV_S *)pstDev);
}

VA_DRV_S gstVkVDecDrv =
{
    .u32Id     = VA_DEV_ID_VCHAN,
    .u32PrivId = VA_CHAN_TYPE_VDEC,
    .szName    = "vdec",
    .init      = VK_VDEC_Init,
    .exit      = VK_VDEC_Exit,
    .mt        = VA_CHAN_DispChanDev,
};

static void VK_VDEC_ModExit(void)
{
    VA_UnRegDrv(&gstVkVDecDrv);
}

static int VK_VDEC_ModInit(VA_MOD_S *pstMod)
{
    VA_CHAN_RegTypeCb(&gstVdecTypeCb);
    VA_RegDrv(&gstVkVDecDrv);
    return 0;
}

VA_MOD_INIT(vdec, VK_VDEC_ModInit, VK_VDEC_ModExit, VA_INIT_LEVEL_DRV)

