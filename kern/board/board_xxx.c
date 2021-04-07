#include "va_chan_base.h"
#include "board.h"

VA_CHAN_DEV_S gastChanDevTbl[] =
{
    // dev id            priv id            start port  port num    chan num per port
    {{VA_DEV_ID_VCHAN, VA_CHAN_TYPE_VDEC},      0,          1,             1},
    {{VA_DEV_ID_VCHAN, VA_CHAN_TYPE_VENC},      0,          1,             2},
    {{VA_DEV_ID_VCHAN, VA_CHAN_TYPE_NET},       0,          1,             16},
    {{VA_DEV_ID_INVAL, 0}}, // terminated item
};

VA_MEM_DEV_S gastMemDevTbl[] =
{
    // dev id           priv id            mem blk size     tot size             mem addr
    {{VA_DEV_ID_MEM,  VA_MEM_TYPE_FDD},      64 * 1024,    10 * 1024 * 1024,        NULL},
    {{VA_DEV_ID_INVAL, 0}}, // terminated item
};

BOARD_CFG_S gstxxxBoardCfg =
{
    .u32BoardSeries = BOARD_SERIES_IPC,
    .u32BoardId     = BOARD_ID_CLASSIC_IPC,
    .szName         = "IPC_XXX",
    .pstChanDevTbl  = gastChanDevTbl,
    .pstMemDevTbl   = gastMemDevTbl,
};

VA_BOARD_DEV_S gstBoardDev =
{
    .stDev.u32Id      = VA_DEV_ID_BOARD,
    .stDev.u32PrivId  = 0,
    .pstBoardCfg      = &gstxxxBoardCfg,
};


static int BOARD_XXX_Init(VA_MOD_S *pstMod)
{
    VA_RegDev(&gstBoardDev.stDev);
    BOARD_RegMemDevs(&gstxxxBoardCfg);
    BOARD_RegChanDevs(&gstxxxBoardCfg);
    return 0;
}

static void BOARD_XXX_Exit(void)
{
    BOARD_UnRegMemDevs(&gstxxxBoardCfg);
    BOARD_UnRegChanDevs(&gstxxxBoardCfg);
    VA_UnRegDev(&gstBoardDev.stDev);
}


VA_MOD_INIT(board_xxx, BOARD_XXX_Init, BOARD_XXX_Exit, VA_INIT_LEVEL_CFG)

