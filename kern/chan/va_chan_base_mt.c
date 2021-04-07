#include "va_chan_base.h"

MT_CB_S gstVaChanMt =
{
    .szName      = "chan",
    .u32DebugCap = MT_DBG_PKT | MT_DBG_MSG | MT_DBG_INFO,
};

extern VA_CHAN_MGR_S gstVaChanMgr;
extern VA_CHAN_TYPE_CB_S *VA_CHAN_GetTypeByName(const char *szName);

VOID VA_CHAN_DispChanDev(VA_DEV_S *pstDev, ULONG ulExecId)
{
    VA_CHAN_DEV_S *pstChanDev = (VA_CHAN_DEV_S *)pstDev;

    MT_PRINT(MT_FMT_U, "start port index",  pstChanDev->u16StartPortIdx);
    MT_PRINT(MT_FMT_U, "port number",       pstChanDev->u16PortNum);
    MT_PRINT(MT_FMT_U, "chan num per port", pstChanDev->u16ChanNumPerPort);
}

static VOID VA_CHAN_MT_DispTypeCb(ULONG ulExecId)
{
    VA_CHAN_PORT_TBL_S *pstPortTbl;
    VA_CHAN_TYPE_CB_S *pstChanTypeCb;
    const char *szChanTypeName;

    if (MT_GetTextVal(ulExecId, "ct", &szChanTypeName) < 0)
    {
        return;
    }

    pstChanTypeCb = VA_CHAN_GetTypeByName(szChanTypeName);
    if ( pstChanTypeCb == NULL )
    {
        MT_PRINT("\r\nNot found %s channel type", szChanTypeName);
        return;
    }

    MT_PRINT("\r\nDump %s chan type cb:", szChanTypeName);
    MT_PRINT(MT_FMT_U,    "ChanType", pstChanTypeCb->u16ChanType);
    MT_PRINT(MT_FMT_STR,  "Name",     pstChanTypeCb->szName);
    MT_PRINT(MT_FMT_U,    "CbSize",   pstChanTypeCb->u16ChanCbSize);
    MT_PRINT(MT_FMT_FUNC, "Ioctl",    pstChanTypeCb->pfnIoctl);

    pstPortTbl = &(gstVaChanMgr.stSelfSlot.astTypePortTbl[pstChanTypeCb->u16ChanType]);
    MT_PRINT(MT_FMT_U, "Port Num",       pstPortTbl->u32PortNum);
    MT_PRINT(MT_FMT_U, "Total Chan Num", pstPortTbl->u32TotChanNum);

    if ( pstChanTypeCb->pfnTypeMt )
    {
        pstChanTypeCb->pfnTypeMt(ulExecId);
    }

    return;
}

static INT VA_CHAN_GetChanId(ULONG ulExecId, CHAN_ID_S *pstChan)
{
    VA_CHAN_TYPE_CB_S *pstChanTypeCb;
    const char *szChanTypeName;
    ULONG ulChanIdx;
    ULONG ulPortIdx;

    if (MT_GetTextVal(ulExecId, "ct", &szChanTypeName) < 0)
    {
        return -EINVAL;
    }

    if (MT_GetIntVal(ulExecId, "pidx", &ulPortIdx) < 0)
    {
        return -EINVAL;
    }

    if (MT_GetIntVal(ulExecId, "cidx", &ulChanIdx) < 0)
    {
        return -EINVAL;
    }

    pstChanTypeCb = VA_CHAN_GetTypeByName(szChanTypeName);
    if ( pstChanTypeCb == NULL )
    {
        MT_PRINT("\r\nNot found %s channel type", szChanTypeName);
        return -ENOENT;
    }

    pstChan->u16SlotId   = 0;
    pstChan->u16ChanType = pstChanTypeCb->u16ChanType;
    pstChan->u16PortId   = ulPortIdx;
    pstChan->u16ChanId   = ulChanIdx;

    return 0;
}

static VOID VA_CHAN_MT_DispChanCb(ULONG ulExecId)
{
    FDD_CHAN_S *pstChan;
    CHAN_ID_S  stChanId;
    const char *szChanTypeName;

    if ( VA_CHAN_GetChanId(ulExecId, &stChanId) < 0 )
    {
        return;
    }

    szChanTypeName = VA_CHAN_GetChanTypeName(stChanId.u16ChanType);

    MT_PRINT("\r\nDisplay chan type %s %u/%u cb:", szChanTypeName, stChanId.u16PortId, stChanId.u16ChanId);

    pstChan = VA_GetChan(&stChanId);
    if ( pstChan == NULL )
    {
        MT_PRINT("\r\nCan't found %s this channel", szChanTypeName);
        return;
    }

    if ( pstChan->pstOps && pstChan->pstOps->pfnMtDispChan )
    {
        pstChan->pstOps->pfnMtDispChan(pstChan, ulExecId);
    }

    FDD_PutChan(pstChan);
    return;
}

VOID VA_CHAN_MT_DispDbgLevel(ULONG ulExecId)
{
    FDD_CHAN_S *pstChan;
    CHAN_ID_S  stChanId;

    if ( VA_CHAN_GetChanId(ulExecId, &stChanId) < 0 )
    {
        return;
    }

    pstChan = VA_GetChan(&stChanId);
    if ( pstChan == NULL )
    {
        MT_PRINT("\r\nCan't found this channel");
        return;
    }

    MT_DispDbgLevel(pstChan->u32Debug, ulExecId);
    FDD_PutChan(pstChan);
    return;
}

static VOID MT_OpenDbgLevel(ULONG ulExecId)
{
    FDD_CHAN_S *pstChan;
    CHAN_ID_S  stChanId;
    U32 u32DbgLevel = MT_GetDbgLevel(ulExecId);

    if ( VA_CHAN_GetChanId(ulExecId, &stChanId) < 0 )
    {
        return;
    }

    pstChan = VA_GetChan(&stChanId);
    if ( pstChan == NULL )
    {
        MT_PRINT("\r\nCan't found this channel");
        return;
    }

    pstChan->u32Debug |= u32DbgLevel;
    FDD_PutChan(pstChan);
    return;
}

static VOID MT_CloseDbgLevel(ULONG ulExecId)
{
    FDD_CHAN_S *pstChan;
    CHAN_ID_S  stChanId;
    U32 u32DbgLevel = MT_GetDbgLevel(ulExecId);

    if ( VA_CHAN_GetChanId(ulExecId, &stChanId) < 0 )
    {
        return;
    }

    pstChan = VA_GetChan(&stChanId);
    if ( pstChan == NULL )
    {
        MT_PRINT("\r\nCan't found this channel");
        return;
    }

    pstChan->u32Debug &= (~u32DbgLevel);
    FDD_PutChan(pstChan);
    return;
}

int __VA_CHAN_MT_RegCmd(MT_CB_S *pstMtCb, VA_CHAN_TYPE_CB_S *pstChanTypeCb)
{
    MT_CMD_ITEM_ARR_S stDbgLevelArr;

    MT_DEF_TEXT_CMD_ITEM(stChanType, "", "ChanType",  "channel type");
    MT_DEF_TEXT_CMD_ITEM(stChanTypeName, "ct", pstChanTypeCb->szName,  pstChanTypeCb->szName);

    MT_DEF_TEXT_CMD_ITEM(stPort, "", "port",  "port");
    MT_DEF_TEXT_CMD_ITEM(stChan, "", "chan",  "chan");
    MT_DEF_INT_CMD_ITEM(stPortIndx,  "pidx", 0, 31,  "port");
    MT_DEF_INT_CMD_ITEM(stChanIndx,  "cidx", 0, 255, "chan");

    MT_MERGE_ITEM_TO_ARR(&stDbgLevelArr, MT_DEF_ITEM_MSG, MT_DEF_ITEM_ERR, MT_DEF_ITEM_PKT, MT_DEF_ITEM_ALL);

    MT_REG_CMD_LINE(VA_CHAN_MT_DispTypeCb, MT_DEF_ITEM_DISP, &stChanType, &stChanTypeName, MT_DEF_ITEM_CB);
    MT_REG_CMD_LINE(VA_CHAN_MT_DispChanCb, MT_DEF_ITEM_DISP, &stChanType, &stChanTypeName, &stPort, &stPortIndx,
                    &stChan, &stChanIndx, MT_DEF_ITEM_CB);

    MT_REG_CMD_LINE(VA_CHAN_MT_DispDbgLevel, MT_DEF_ITEM_DISP, MT_DEF_ITEM_DEBUG, &stChanType, &stChanTypeName,
                    &stPort, &stPortIndx, &stChan, &stChanIndx);

    MT_REG_CMD_LINE(MT_OpenDbgLevel, MT_DEF_ITEM_DEBUG, &stChanType, &stChanTypeName,
                    &stPort, &stPortIndx, &stChan, &stChanIndx, &stDbgLevelArr);

    MT_REG_CMD_LINE(MT_CloseDbgLevel, MT_DEF_ITEM_UNDO, MT_DEF_ITEM_DEBUG, &stChanType, &stChanTypeName,
                    &stPort, &stPortIndx, &stChan, &stChanIndx, &stDbgLevelArr);

    return 0;
}

int VA_CHAN_MT_RegCmd(VA_CHAN_TYPE_CB_S *pstChanTypeCb)
{
    return __VA_CHAN_MT_RegCmd(&gstVaChanMt, pstChanTypeCb);
}

int VA_CHAN_RegMt(void)
{
    return MT_RegMt(&gstVaChanMt);
}

void VA_CHAN_UnRegMt(void)
{
    return MT_UnRegMt(&gstVaChanMt);
}

