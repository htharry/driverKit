#include "va_usr_pub.h"
#include "mt_usr.h"
#include "mt_ua.h"
#include <stdarg.h>

VOID __MT_DBG_PrintMsg(MT_APP_CB_S *pstCb, const char *szFmt, ...)
{
    va_list args;
    char *pBuf;
    LONG lRet;

    pBuf = (char *)VA_Malloc(MT_UA_PRINT_LEN + 1);
    if ( pBuf == NULL )
    {
        return;
    }
    pBuf[MT_UA_PRINT_LEN] = 0;

    va_start(args, szFmt);
    lRet = vsnprintf(pBuf, MT_UA_PRINT_LEN, szFmt, args);
    va_end(args);

    __MT_DBG_Print(pstCb->ulMtHndl, pBuf);
    VA_Free(pBuf);
}

VOID MT_DBG_PrintMsg(MT_APP_CB_S *pstCb, VOID *pMsg, MT_APP_USR_PRINT_MSG_PF pfnPrintMsg)
{
    if ( NULL == pfnPrintMsg )
    {
        return;
    }

    MT_DBG_Print(pstCb->ulMtHndl, "");

    pfnPrintMsg(pstCb, pMsg, __MT_DBG_PrintMsg);
    return;
}

VOID MT_CMD_ELEM_INIT_For_CXX(MT_CMD_ELEMENT_S *pstCmdElement, U32 u32CmdType, U32 u32Id, const char *szName, const char *szDesc, ULONG ulMinVal, ULONG ulMaxVal)
{
    VA_CB_ZERO(pstCmdElement);

    pstCmdElement->u8Type = u32CmdType;
    pstCmdElement->as8Id[0] = u32Id >> 24;
    pstCmdElement->as8Id[1] = (u32Id >> 16) & 0xFF;
    pstCmdElement->as8Id[2] = (u32Id >> 8)  & 0XFF;
    pstCmdElement->as8Id[3] = u32Id & 0xFF;
    pstCmdElement->szName = szName;
    pstCmdElement->szDesc = szDesc;
    pstCmdElement->u64MinVal = ulMinVal;
    pstCmdElement->u64MaxVal = ulMaxVal;
}

INT DRVLib_RegAppMtCb(MT_APP_CB_S *pstCb)
{

    if ( NULL == pstCb )
    {
        return VA_E_INVAL;
    }

    pstCb->ulMtHndl = MT_USR_Open(pstCb->szName, pstCb->szDesc, pstCb->u32DebugCap, 0);
    if ( pstCb->ulMtHndl == MT_USR_INVAL_HANDLE )
    {
        return VA_E_SYS_FAILED;
    }

    //register cmdline!
    if ( pstCb->pfnDispCb )
    {
        __MT_REG_CMD_LINE(pstCb->ulMtHndl, pstCb, pstCb->pfnDispCb, MT_DEF_ITEM_DISP, MT_DEF_ITEM_MOD, MT_DEF_ITEM_CB);
    }

    if ( pstCb->pfnDispStat )
    {
        __MT_REG_CMD_LINE(pstCb->ulMtHndl, pstCb, pstCb->pfnDispStat, MT_DEF_ITEM_DISP, MT_DEF_ITEM_MOD, MT_DEF_ITEM_STAT);
    }

    if ( pstCb->pfnResetStat )
    {
        __MT_REG_CMD_LINE(pstCb->ulMtHndl, pstCb, pstCb->pfnResetStat, MT_DEF_ITEM_RESET, MT_DEF_ITEM_MOD, MT_DEF_ITEM_STAT);
    }

    return VA_SUCCESS;
}

VOID DRVLib_ExecMtCmd(MT_APP_CB_S *pstCb)
{
    if ( NULL == pstCb )
    {
        return;
    }

    MT_USR_ExecCmdLine(pstCb->ulMtHndl);
    return;
}

INT DRVLib_GetMtFd(MT_APP_CB_S *pstCb)
{
    if ( pstCb == NULL )
    {
        return VA_INVALID_FD;
    }

    return MT_USR_GetFd(pstCb->ulMtHndl);
}

VOID MT_ConvertId(U32 u32Id, S8 as8Id[MT_ID_MAX_LEN])
{
    as8Id[0] = ((u32Id >> 24) & 0xFF);
    as8Id[1] = ((u32Id >> 16) & 0xFF);
    as8Id[2] = ((u32Id >> 8)  & 0xFF);
    as8Id[3] = ((u32Id) & 0xFF);
}

INT DRVLib_GetValFrmMtParams(ULONG ulExecId, U32 u32Id, ULONG *pulVal)
{
    S8  as8Id[MT_ID_MAX_LEN] = {0};

    MT_ConvertId(u32Id, as8Id);
    return MT_GetIntVal(ulExecId, as8Id, pulVal);
}

INT DRVLib_GetStrFrmMtParams(ULONG ulExecId, U32 u32Id, char szStr[MT_STR_MAX_LEN])
{
    S8  as8Id[MT_ID_MAX_LEN] = {0};

    MT_ConvertId(u32Id, as8Id);
    return MT_GetStrVal(ulExecId, as8Id, szStr);
}


INT DRVLib_GetTitleFrmMtParams(ULONG ulExecId, U32 u32Id, char szStr[MT_STR_MAX_LEN])
{
    S8  as8Id[MT_ID_MAX_LEN] = {0};

    MT_ConvertId(u32Id, as8Id);
    return MT_GetTextVal(ulExecId, as8Id, szStr);
}

