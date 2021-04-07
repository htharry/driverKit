#include "va_chan_base.h"
#include "board.h"

BOARD_INFO_S gstBoardInfo;

#if 0
#endif

void VA_DispMemDev(VA_DEV_S *pstDev, ULONG ulExecId)
{
    VA_MEM_DEV_S *pstMemDev = (VA_MEM_DEV_S *)pstDev;

    MT_DISP_FMT_MEM("block size", pstMemDev->u32BlkSize);
    MT_DISP_FMT_MEM("total size", pstMemDev->u32TotSize);
    MT_PRINT(MT_FMT_PTR, "memory address", pstMemDev->pMemAddr);
}

#if 0
#endif

void BOARD_GetChanCfg(U16 u16ChanType, BOARD_CHAN_CFG_S *pstChanCfg)
{
    VA_CB_ZERO(pstChanCfg);
    if ( u16ChanType < VA_CHAN_TYPE_NUM )
    {
        memcpy(pstChanCfg, gstBoardInfo.astChanCfgTbl + u16ChanType, sizeof(BOARD_CHAN_CFG_S));
    }

    return;
}

U32 BOARD_GetLocTotChanNum(U16 u16ChanType)
{
    if ( u16ChanType < VA_CHAN_TYPE_NUM )
    {
        return gstBoardInfo.astChanCfgTbl[u16ChanType].u16TotChanNum;
    }

    return 0;
}

void BOARD_ChanDevInit(BOARD_INFO_S *pstBoardInfo)
{
    BOARD_CHAN_CFG_S *pstChanCfg;
    VA_CHAN_DEV_S    *pstChanDevTbl;
    U16 u16ChanType;
    U32 i;

    pstChanDevTbl = pstBoardInfo->pstBoardCfg->pstChanDevTbl;
    for ( i = 0; pstChanDevTbl[i].stDev.u32Id != VA_DEV_ID_INVAL; i++ )
    {
        u16ChanType = pstChanDevTbl[i].stDev.u32PrivId;
        pstChanCfg = &pstBoardInfo->astChanCfgTbl[u16ChanType];
        pstChanCfg->u16ChanNumPerPort = pstChanDevTbl[i].u16ChanNumPerPort;
        pstChanCfg->u16TotChanNum     = pstChanDevTbl[i].u16PortNum * pstChanDevTbl[i].u16ChanNumPerPort;
        pstChanCfg->u16PortNum        = pstChanDevTbl[i].u16PortNum;
    }

    return;
}

void BOARD_RegChanDevs(BOARD_CFG_S *pstBoardCfg)
{
    VA_CHAN_DEV_S *pstChanDevTbl;
    U32 i;

    pstChanDevTbl = pstBoardCfg->pstChanDevTbl;
    for ( i = 0; pstChanDevTbl[i].stDev.u32Id != VA_DEV_ID_INVAL; i++ )
    {
        VA_RegDev(&pstChanDevTbl[i].stDev);
    }

    return;
}

void BOARD_UnRegChanDevs(BOARD_CFG_S *pstBoardCfg)
{
    VA_CHAN_DEV_S *pstChanDevTbl;
    U32 i;

    pstChanDevTbl = pstBoardCfg->pstChanDevTbl;
    for ( i = 0; pstChanDevTbl[i].stDev.u32Id != VA_DEV_ID_INVAL; i++ )
    {
        VA_UnRegDev(VA_PTR_TYPE(VA_DEV_S, pstChanDevTbl + i));
    }
}

void BOARD_RegMemDevs(BOARD_CFG_S *pstBoardCfg)
{
    VA_MEM_DEV_S *pstMemDevTbl;
    U32 i;

    pstMemDevTbl = pstBoardCfg->pstMemDevTbl;
    for ( i = 0; pstMemDevTbl[i].stDev.u32Id != VA_DEV_ID_INVAL; i++ )
    {
        VA_RegDev(&pstMemDevTbl[i].stDev);
    }

    return;
}

void BOARD_UnRegMemDevs(BOARD_CFG_S *pstBoardCfg)
{
    VA_MEM_DEV_S *pstMemDevTbl;
    U32 i;

    pstMemDevTbl = pstBoardCfg->pstMemDevTbl;
    for ( i = 0; pstMemDevTbl[i].stDev.u32Id != VA_DEV_ID_INVAL; i++ )
    {
        VA_UnRegDev(&pstMemDevTbl[i].stDev);
    }

    return;
}

int BOARD_DrvInit(VA_DEV_S *pstDev)
{
    VA_BOARD_DEV_S *pstBoardDev = (VA_BOARD_DEV_S *)pstDev;

    gstBoardInfo.pstBoardCfg = pstBoardDev->pstBoardCfg;
    BOARD_ChanDevInit(&gstBoardInfo);

    return 0;
}

void BOARD_DrvExit(VA_DEV_S *pstDev)
{
}

VA_DRV_S gstBoardDrv =
{
    .u32Id      = VA_DEV_ID_BOARD,
    .u32PrivId  = 0,
    .szName     = "board",
    .init       = BOARD_DrvInit,
    .exit       = BOARD_DrvExit,
    .mt         = NULL,
};

static int BOARD_Init(VA_MOD_S *pstMod)
{
    return VA_RegDrv(&gstBoardDrv);
}

static void BOARD_Exit(void)
{
    VA_UnRegDrv(&gstBoardDrv);
}

VA_MOD_INIT(board, BOARD_Init, BOARD_Exit, VA_INIT_LEVEL_CFG)

