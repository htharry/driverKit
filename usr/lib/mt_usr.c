#include "va_usr_pub.h"
#include "mt_usr.h"
#include "mt_ua.h"
#include <stdarg.h>
#include <sys/ioctl.h>
#include <time.h>
#include <pthread.h>

static U32 MT_USR_GetDbgLevel(ULONG ulExecId)
{
    char szDbgLevel[MT_STR_MAX_LEN];
    U32 u32DbgLevel = 0;

    if (MT_GetTextVal(ulExecId, "msg", szDbgLevel) == 0)
    {
        u32DbgLevel = MT_DBG_MSG;
    }
    else if (MT_GetTextVal(ulExecId, "info", szDbgLevel) == 0)
    {
        u32DbgLevel = MT_DBG_INFO;
    }
    else if (MT_GetTextVal(ulExecId, "err", szDbgLevel) == 0)
    {
        u32DbgLevel = MT_DBG_ERR;
    }
    else if (MT_GetTextVal(ulExecId, "pkt", szDbgLevel) == 0)
    {
        u32DbgLevel = MT_DBG_PKT;
    }
    else if (MT_GetTextVal(ulExecId, "all", szDbgLevel) == 0)
    {
        u32DbgLevel = MT_DBG_ALL;
    }

    return u32DbgLevel;
}

static VOID MT_USR_OpenDbgLevel(ULONG ulExecId)
{
    MT_UA_EXEC_CB_S *pstExecCb;
    MT_USR_CB_S *pstUsrCb;
    U32 u32DbgLevel;

    pstExecCb = VA_PTR_TYPE(MT_UA_EXEC_CB_S, ulExecId);
    pstUsrCb  = pstExecCb->pstUsrCb;

    u32DbgLevel = MT_USR_GetDbgLevel(ulExecId);
    pstUsrCb->u32Debug |= pstUsrCb->u32DebugCap & u32DbgLevel;
    return;
}

static VOID MT_USR_CloseDbgLevel(ULONG ulExecId)
{
    U32 u32DbgLevel  = MT_USR_GetDbgLevel(ulExecId);
    MT_UA_EXEC_CB_S *pstExecCb;
    MT_USR_CB_S *pstUsrCb;

    pstExecCb = VA_PTR_TYPE(MT_UA_EXEC_CB_S, ulExecId);
    pstUsrCb  = pstExecCb->pstUsrCb;

    pstUsrCb->u32Debug &= (~u32DbgLevel);
    return;
}

static VOID MT_USR_DispDbgLevel(ULONG ulExecId)
{
    MT_UA_EXEC_CB_S *pstExecCb;
    MT_USR_CB_S *pstUsrCb;

    pstExecCb = VA_PTR_TYPE(MT_UA_EXEC_CB_S, ulExecId);
    pstUsrCb  = pstExecCb->pstUsrCb;

    MT_PRINT("\r\ndump debug level:");

    if ( pstUsrCb->u32Debug & MT_DBG_MSG )
    {
        MT_PRINT("\r\n msg");
    }

    if ( pstUsrCb->u32Debug & MT_DBG_ERR )
    {
        MT_PRINT("\r\n err");
    }

    if ( pstUsrCb->u32Debug & MT_DBG_INFO )
    {
        MT_PRINT("\r\n info");
    }

    if ( pstUsrCb->u32Debug & MT_DBG_PKT )
    {
        MT_PRINT("\r\n pkt");
    }
}

VOID MT_USR_RegDbgCmdLine(MT_USR_CB_S *pstUsrCb)
{
    ULONG ulMtHndl = (ULONG)pstUsrCb;
    MT_CMD_ITEM_ARR_S stDbgItemArr;

    //MT_DEF_TEXT_CMD_ITEM(stMsg,  "msg",  "msg",  "message");
    //MT_DEF_TEXT_CMD_ITEM(stErr,  "err",  "err",  "error");
    //MT_DEF_TEXT_CMD_ITEM(stPkt,  "pkt",  "pkt",  "packet");
    MT_DEF_TEXT_CMD_ITEM(stCbNm, "cbnm", pstUsrCb->szName, pstUsrCb->szName);
    MT_INIT_ITEM_ARR(&stDbgItemArr);

    if ( pstUsrCb->u32DebugCap & MT_DBG_MSG )
    {
        MT_USR_AddItemToArr(&stDbgItemArr, MT_DEF_ITEM_MSG);
    }

    if ( pstUsrCb->u32DebugCap & MT_DBG_PKT )
    {
        MT_USR_AddItemToArr(&stDbgItemArr, MT_DEF_ITEM_PKT);
    }

    if ( pstUsrCb->u32DebugCap & MT_DBG_ERR )
    {
        MT_USR_AddItemToArr(&stDbgItemArr, MT_DEF_ITEM_ERR);
    }

    if ( pstUsrCb->u32DebugCap & MT_DBG_INFO )
    {
        MT_USR_AddItemToArr(&stDbgItemArr, MT_DEF_ITEM_INFO);
    }

    MT_REG_CMD_LINE(MT_USR_OpenDbgLevel,  MT_DEF_ITEM_DEBUG, &stCbNm, &stDbgItemArr);
    MT_REG_CMD_LINE(MT_USR_CloseDbgLevel, MT_DEF_ITEM_UNDO,  MT_DEF_ITEM_DEBUG, &stCbNm, &stDbgItemArr);

    if ( pstUsrCb->u32DebugCap )
    {
        MT_REG_CMD_LINE(MT_USR_OpenDbgLevel,  MT_DEF_ITEM_DEBUG, &stCbNm, MT_DEF_ITEM_ALL);
        MT_REG_CMD_LINE(MT_USR_CloseDbgLevel, MT_DEF_ITEM_UNDO,  MT_DEF_ITEM_DEBUG, &stCbNm, MT_DEF_ITEM_ALL);
        MT_REG_CMD_LINE(MT_USR_DispDbgLevel,  MT_DEF_ITEM_DISP,  MT_DEF_ITEM_DEBUG, &stCbNm);
    }
}

void MT_USR_FillItemToArray(MT_CMD_ITEM_S *pstArrItem, MT_CMD_ITEM_S *pstItem)
{
    const MT_CMD_ITEM_S *pstCurrItem = pstItem;

	if ( (ULONG)pstItem < MT_ITEM_SPEC_BUTT )
	{
        pstArrItem->u8Flg = MT_ITEM_FLG_SPEC | MT_ITEM_FLG_ARRAY;
        pstArrItem->u32SpecItemId = (U32)(ULONG)pstItem;

        return;
	}

    *pstArrItem = *pstCurrItem;
    pstArrItem->u8Flg |= MT_ITEM_FLG_ARRAY;
}

int MT_USR_AddItemToArr(MT_CMD_ITEM_ARR_S *pstItemArr, MT_CMD_ITEM_S *pstItem)
{
    if ( pstItemArr->u32Num == 0 )
    {
        MT_USR_FillItemToArray(&pstItemArr->stItem, pstItem);
    }
    else
    {
		if ( pstItemArr->u32Num > MT_ITEM_MAX_ARR )
		{
			VA_LOG_ERR("It's too many cmd items for array merge");
		    return VA_E_2BIG;
		}

        pstItemArr->apstItem[pstItemArr->u32Num - 1] = pstItem;
    }

    pstItemArr->u32Num++;
    return VA_SUCCESS;
}

int MT_USR_MergeItemsToArr(MT_CMD_ITEM_ARR_S *pstItemArr, MT_CMD_ITEM_S *pstItem, ...)
{
    MT_CMD_ITEM_S *pstTmpItem;
    va_list args;
    U32 nItemCnt = 0;

    pstItemArr->u32Num = 0;
    MT_USR_FillItemToArray(&pstItemArr->stItem, pstItem); // fill first item

    va_start(args, pstItem);

    pstTmpItem = VA_PTR_TYPE(MT_CMD_ITEM_S, va_arg(args, void *));
    for ( ; pstTmpItem != NULL; nItemCnt++ )
    {
		if ( nItemCnt >= MT_ITEM_MAX_ARR )
		{
			VA_LOG_ERR("It's too many cmd items for array merge");
		    return VA_E_2BIG;
		}

        pstItemArr->apstItem[nItemCnt] = pstTmpItem;
        pstTmpItem = VA_PTR_TYPE(MT_CMD_ITEM_S, va_arg(args, void *));
    }

    va_end(args);

    pstItemArr->u32Num = nItemCnt + 1;
    return VA_SUCCESS;
}


static ULONG MT_USR_FillOneItem(IN MT_CMD_ITEM_S *pstItem, OUT MT_CMD_USR_ITEM_S *pstUsrItem, U8 u8Flg)
{
    VA_CB_ZERO(pstUsrItem);

    if ( (ULONG)pstItem < MT_ITEM_SPEC_BUTT )
    {
        pstUsrItem->u8Flg = MT_ITEM_FLG_SPEC | u8Flg;
        pstUsrItem->u32SpecItemId = (U32)(ULONG)pstItem;
    }
    else
    {
        if ( pstItem->szName == NULL )
        {
            return VA_E_INVAL;
        }

        pstUsrItem->u8Type = pstItem->u8Type;
        pstUsrItem->u8Flg  = pstItem->u8Flg | u8Flg;
        pstUsrItem->u64MinVal = pstItem->u64MinVal;
        pstUsrItem->u64MaxVal = pstItem->u64MaxVal;
        strncpy(pstUsrItem->szDesc, pstItem->szDesc, MT_DESC_MAX_LEN);
        strncpy(pstUsrItem->szName, pstItem->szName, MT_NAME_MAX_LEN);
        memcpy(pstUsrItem->as8Id, pstItem->as8Id, sizeof(pstItem->as8Id));
    }

    return VA_SUCCESS;
}

static ULONG MT_USR_FillArrItem(MT_UA_CL_INFO_S *pstClInfo, MT_CMD_ITEM_S *pstItem, U32 *pu32Pos)
{
    MT_CMD_ITEM_ARR_S *pstItemArr;
    ULONG ulRet;
    U32 i;
    U8 u8Flg = MT_ITEM_FLG_ARRAY;

    pstItemArr = VA_PTR_TYPE(MT_CMD_ITEM_ARR_S, pstItem);

    if ( (*pu32Pos + pstItemArr->u32Num) > pstClInfo->u32EntryNum )
    {
        return VA_E_2BIG;
    }

    for ( i = 0; i < pstItemArr->u32Num; i++ )
    {
        if ( (i + 1) == pstItemArr->u32Num )
        {
            u8Flg |= MT_ITEM_FLG_ARR_END;
        }

        if ( i == 0 ) // first item
        {
            if ( pstItem->u8Flg & MT_ITEM_FLG_SPEC )
            {
                ulRet = MT_USR_FillOneItem((MT_CMD_ITEM_S *)(ULONG)(pstItem->u32SpecItemId), pstClInfo->astItem + *pu32Pos, u8Flg);
            }
            else
            {
                ulRet = MT_USR_FillOneItem(pstItem, pstClInfo->astItem + *pu32Pos, u8Flg);
            }

            if ( ulRet != VA_SUCCESS )
            {
                return ulRet;
            }

            continue;
        }

        ulRet = MT_USR_FillOneItem(pstItemArr->apstItem[i - 1], pstClInfo->astItem + *pu32Pos + i, u8Flg);
        if ( ulRet != VA_SUCCESS )
        {
            return ulRet;
        }
    }

    *pu32Pos += pstItemArr->u32Num;
    return VA_SUCCESS;
}

static ULONG MT_USR_FillItem(MT_CMD_ITEM_S *pstItem, MT_UA_CL_INFO_S *pstClInfo, U32 *pu32Pos)
{
    ULONG ulRet;

    if ( *pu32Pos >= pstClInfo->u32EntryNum )
    {
        return VA_E_2BIG;
    }

    if ( (ULONG)pstItem > MT_ITEM_SPEC_BUTT )
    {
        if ( pstItem->u8Flg & MT_ITEM_FLG_ARRAY )
        {
            ulRet = MT_USR_FillArrItem(pstClInfo, pstItem, pu32Pos);
            return ulRet;
        }
    }

    ulRet = MT_USR_FillOneItem(pstItem, pstClInfo->astItem + *pu32Pos, 0);
    if ( ulRet != VA_SUCCESS )
    {
        return ulRet;
    }

    *pu32Pos += 1;
    return ulRet;
}

ULONG MT_USR_FillItems(MT_UA_CL_INFO_S *pstClInfo, MT_CMD_ITEM_S *pstItem, va_list args)
{
    MT_CMD_ITEM_S *pstTmpItem;
    U32 u32Pos = 0;
    ULONG ulRet;

    ulRet = MT_USR_FillItem(pstItem, pstClInfo, &u32Pos);
    if ( ulRet != VA_SUCCESS )
    {
        return ulRet;
    }

    pstTmpItem = VA_PTR_TYPE(MT_CMD_ITEM_S, va_arg(args, void *));
    while (pstTmpItem != NULL)
    {
        ulRet = MT_USR_FillItem(pstTmpItem, pstClInfo, &u32Pos);
        if ( ulRet != VA_SUCCESS )
        {
            return ulRet;
        }

        pstTmpItem = VA_PTR_TYPE(MT_CMD_ITEM_S, va_arg(args, void *));
    }

    pstClInfo->u32EntryNum = u32Pos;
    pstClInfo->u32Size     = u32Pos * sizeof(MT_CMD_USR_ITEM_S);

    return VA_SUCCESS;
}

ULONG MT_USR_RegCmdLine(ULONG ulMtHndl, VOID *pPrivCb, MT_USR_CL_CALL_BACK_PF pfnCallBack, MT_CMD_ITEM_S *pstItem, ...)
{
    MT_USR_CB_S *pstUsrCb;
    MT_UA_CL_INFO_S *pstClInfo;
    LONG lRet;
    ULONG ulRet;
    va_list args;

    if ( MT_USR_IS_INVAL_HNDL(ulMtHndl) )
    {
        return VA_E_INVAL;
    }

    pstUsrCb  = VA_PTR_TYPE(MT_USR_CB_S, ulMtHndl);
    pstClInfo = VA_Malloc(MT_UA_CL_INFO_MAX_LEN);
    if ( pstClInfo == NULL )
    {
        return VA_E_NO_MEM;
    }

    pstClInfo->pCallBack   = pfnCallBack;
    pstClInfo->pPrivCb     = pPrivCb;
    pstClInfo->u32Size     = MT_UA_CL_INFO_MAX_LEN - sizeof(MT_UA_CL_INFO_S);
    pstClInfo->u32EntryNum = pstClInfo->u32Size/sizeof(MT_CMD_USR_ITEM_S);

    va_start(args, pstItem);
    ulRet = MT_USR_FillItems(pstClInfo, pstItem, args);
    va_end(args);

    if ( ulRet != VA_SUCCESS )
    {
        return ulRet;
    }

    lRet = ioctl(pstUsrCb->iMtFd, MT_IOCTL_UA_REG_CL, pstClInfo);

    VA_Free(pstClInfo);

    if ( lRet < 0 )
    {
        return VA_E_SYS_FAILED;
    }

    return VA_SUCCESS;
}

VOID MT_USR_ExecCmdLine(ULONG ulMtHndl)
{
    MT_UA_EXEC_PARAM_S stExecParam;
    MT_UA_EXEC_CB_S stExecCb;
    MT_USR_CB_S *pstUsrCb;
    LONG lRet;

    if ( MT_USR_IS_INVAL_HNDL(ulMtHndl) )
    {
        return;
    }

    pstUsrCb  = VA_PTR_TYPE(MT_USR_CB_S, ulMtHndl);
    memset(&stExecParam, 0, sizeof(stExecParam));

    lRet = ioctl(pstUsrCb->iMtFd, MT_IOCTL_UA_GET_EXEC_PARAM, &stExecParam);
    if ( lRet < 0 )
    {
        return;
    }

    stExecCb.pPrivCb  = stExecParam.pPrivCb;
    stExecCb.pstUsrCb = pstUsrCb;

    ((MT_USR_CL_CALL_BACK_PF)stExecParam.pCallBack)((ULONG)(&stExecCb));

    lRet = ioctl(pstUsrCb->iMtFd, MT_IOCTL_UA_END_EXEC, 0);
    if ( lRet < 0 )
    {
        return;
    }

    return;
}

#if 0
#endif

static int MT_GetVal(MT_UA_EXEC_CB_S *pstExecCb, const S8 *as8Id, U32 u32ValType)
{
    MT_UA_VAL_S stVal;
    LONG lRet;

    VA_CB_ZERO(&stVal);

    stVal.u32ValType = u32ValType;
    stVal.pBuf       = pstExecCb->szBuf;

    if ( as8Id[0] == 0 )
    {
        memcpy(stVal.as8Id, as8Id, MT_ID_MAX_LEN);
    }
    else
    {
        strncpy(stVal.as8Id, as8Id, MT_ID_MAX_LEN);
    }

    lRet = ioctl(pstExecCb->pstUsrCb->iMtFd, MT_IOCTL_UA_GET_VAL, &stVal);
    VA_SET_STR_ARR_TERM(pstExecCb->szBuf);
    if ( lRet < 0 )
    {
        return VA_E_SYS_FAILED;
    }

    return VA_SUCCESS;
}

int MT_GetStrVal(ULONG ulExecId, const S8 *as8Id, char szStr[MT_STR_MAX_LEN])
{
    MT_UA_EXEC_CB_S *pstExecCb = (MT_UA_EXEC_CB_S *)ulExecId;
    int iRet;

    iRet = MT_GetVal(pstExecCb, as8Id, MT_UA_VAL_TYPE_STR);
    if ( iRet != VA_SUCCESS )
    {
        return iRet;
    }

    strncpy(szStr, pstExecCb->szBuf, MT_STR_MAX_LEN);
    szStr[MT_STR_MAX_LEN - 1] = 0;

    return VA_SUCCESS;
}

int MT_GetTextVal(ULONG ulExecId, const S8 *as8Id, char szStr[MT_STR_MAX_LEN])
{
    MT_UA_EXEC_CB_S *pstExecCb = (MT_UA_EXEC_CB_S *)ulExecId;
    int iRet;

    iRet = MT_GetVal(pstExecCb, as8Id, MT_UA_VAL_TYPE_TEXT);
    if ( iRet != VA_SUCCESS )
    {
        return iRet;
    }

    strncpy(szStr, pstExecCb->szBuf, MT_STR_MAX_LEN);
    szStr[MT_STR_MAX_LEN - 1] = 0;

    return VA_SUCCESS;
}

int MT_GetIntVal(ULONG ulExecId, const S8 *as8Id, ULONG *pulVal)
{
    MT_UA_EXEC_CB_S *pstExecCb = (MT_UA_EXEC_CB_S *)ulExecId;
    ULONG ulVal;
    int iRet;

    iRet = MT_GetVal(pstExecCb, as8Id, MT_UA_VAL_TYPE_INT);
    if ( iRet != VA_SUCCESS )
    {
        return iRet;
    }

    ulVal   = *((ULONG *)pstExecCb->szBuf);
    *pulVal = ulVal;

    return VA_SUCCESS;
}

int MT_GetChanIdVal(ULONG ulExecId, const S8 *as8Id, CHAN_ID_S *pstChanId)
{
    MT_UA_EXEC_CB_S *pstExecCb = (MT_UA_EXEC_CB_S *)ulExecId;
    int iRet;

    iRet = MT_GetVal(pstExecCb, as8Id, MT_UA_VAL_TYPE_CHAN_ID);
    if ( iRet != VA_SUCCESS )
    {
        return iRet;
    }

    *pstChanId = *((CHAN_ID_S *)pstExecCb->szBuf);
    return VA_SUCCESS;
}

int MT_GetU64Val(ULONG ulExecId, const S8 *as8Id, U64 *pu64Val)
{
    MT_UA_EXEC_CB_S *pstExecCb = (MT_UA_EXEC_CB_S *)ulExecId;
    int iRet;

    iRet = MT_GetVal(pstExecCb, as8Id, MT_UA_VAL_TYPE_U64);
    if ( iRet != VA_SUCCESS )
    {
        return iRet;
    }

    *pu64Val = *((U64 *)pstExecCb->szBuf);
    return VA_SUCCESS;
}

int MT_GetIpv4Val(ULONG ulExecId, const S8 *as8Id, BE32 *pbe32Ip)
{
    MT_UA_EXEC_CB_S *pstExecCb = (MT_UA_EXEC_CB_S *)ulExecId;
    int iRet;

    iRet = MT_GetVal(pstExecCb, as8Id, MT_UA_VAL_TYPE_IPV4);
    if ( iRet != VA_SUCCESS )
    {
        return iRet;
    }

    *pbe32Ip   = *((BE32 *)pstExecCb->szBuf);
    return VA_SUCCESS;
}

int MT_GetAllValById(ULONG ulExecId, const S8 *as8Id, MT_VAL_U *pstValTbl, U32 *pu32ItemNum)
{
    MT_UA_EXEC_CB_S *pstExecCb = (MT_UA_EXEC_CB_S *)ulExecId;
    MT_UA_VAL_ARR_S stValArr;
    LONG lRet;

    stValArr.u32Num    = *pu32ItemNum;
    stValArr.pstValTbl = pstValTbl;
    *pu32ItemNum       = 0;

    if ( as8Id[0] == 0 )
    {
        memcpy(stValArr.as8Id, as8Id, MT_ID_MAX_LEN);
    }
    else
    {
        strncpy(stValArr.as8Id, as8Id, MT_ID_MAX_LEN);
    }

    lRet = ioctl(pstExecCb->pstUsrCb->iMtFd, MT_IOCTL_UA_GET_VAL_ARR, &stValArr);
    if ( lRet < 0 )
    {
        return VA_E_SYS_FAILED;
    }

    *pu32ItemNum = stValArr.u32Num;
    return VA_SUCCESS;
}

VOID *MT_GetPrivCb(ULONG ulExecId)
{
    MT_UA_EXEC_CB_S *pstExecCb = (MT_UA_EXEC_CB_S *)ulExecId;
    return pstExecCb->pPrivCb;
}

VOID MT_Print(ULONG ulExecId, const char *szFmt, ...)
{
    MT_UA_PRINT_PARAM_S stPrintParam;
    MT_UA_EXEC_CB_S *pstExecCb;
    MT_USR_CB_S *pstUsrCb;
    char *pBuf;
    LONG lRet;
    va_list args;

    pstExecCb = VA_PTR_TYPE(MT_UA_EXEC_CB_S, ulExecId);
    pstUsrCb  = pstExecCb->pstUsrCb;

    pBuf = (char *)VA_Malloc(MT_UA_PRINT_LEN + 1);
    if ( pBuf == NULL )
    {
        return;
    }

    pBuf[MT_UA_PRINT_LEN] = 0;

    va_start(args, szFmt);
    lRet = vsnprintf(pBuf, MT_UA_PRINT_LEN, szFmt, args);
    va_end(args);

    stPrintParam.pBuf   = pBuf;
    stPrintParam.u32Len = (U32)lRet;

    lRet = ioctl(pstUsrCb->iMtFd, MT_IOCTL_UA_PRINT, &stPrintParam);
    if ( lRet < 0 )
    {
        VA_Free(pBuf);
        return;
    }

    VA_Free(pBuf);
    return;
}

VOID __MT_DBG_Print(ULONG ulMtHndl, const char *szFmt, ...)
{
    MT_UA_PRINT_PARAM_S stPrintParam;
    MT_USR_CB_S *pstUsrCb;
    char *pBuf;
    LONG lRet;
    va_list args;

    pstUsrCb = VA_PTR_TYPE(MT_USR_CB_S, ulMtHndl);
    if ( pstUsrCb == NULL )
    {
        return;
    }

    pBuf = (char *)VA_Malloc(MT_UA_PRINT_LEN + 1);
    if ( pBuf == NULL )
    {
        return;
    }
    pBuf[MT_UA_PRINT_LEN] = 0;

    va_start(args, szFmt);
    lRet = vsnprintf(pBuf, MT_UA_PRINT_LEN, szFmt, args);
    va_end(args);

    stPrintParam.pBuf   = pBuf;
    stPrintParam.u32Len = (U32)lRet;

    ioctl(pstUsrCb->iMtFd, MT_IOCTL_UA_DBG_PRINT, &stPrintParam);

    VA_Free(pBuf);
    return;

}


VOID MT_DBG_Print(ULONG ulMtHndl, const char *szFmt, ...)
{
    struct timespec stTime;
    MT_UA_PRINT_PARAM_S stPrintParam;
    MT_USR_CB_S *pstUsrCb;
    char *pBuf;
    LONG lRet;
    va_list args;

    pstUsrCb = VA_PTR_TYPE(MT_USR_CB_S, ulMtHndl);
    if ( pstUsrCb == NULL )
    {
        return;
    }

    pBuf = (char *)VA_Malloc(MT_UA_PRINT_LEN + 1);
    if ( pBuf == NULL )
    {
        return;
    }
    pBuf[MT_UA_PRINT_LEN] = 0;

    clock_gettime(CLOCK_MONOTONIC, &stTime);

    lRet = VA_SnprintEx(pBuf, MT_UA_PRINT_LEN, "\r\n[%s][%05ld.%03ld]:", pstUsrCb->szName, stTime.tv_sec, stTime.tv_nsec/1000000);

    va_start(args, szFmt);
    lRet += VA_VsnprintEx(pBuf + lRet, MT_UA_PRINT_LEN - lRet, szFmt, args);
    va_end(args);

    stPrintParam.pBuf   = pBuf;
    stPrintParam.u32Len = (U32)lRet;

    ioctl(pstUsrCb->iMtFd, MT_IOCTL_UA_DBG_PRINT, &stPrintParam);

    VA_Free(pBuf);
    return;
}

VOID MT_DBG_PrintPkt(ULONG ulMtHndl, U8 *pu8Data, U32 u32Len, const char *szDescFmt, ...)
{
    struct timespec stTime;
    MT_USR_CB_S *pstUsrCb;
    char szDesc[64];
    va_list args;
    U32 i;

    if ( (NULL == pu8Data) || (0 == u32Len) )
    {
        return;
    }

    pstUsrCb = VA_PTR_TYPE(MT_USR_CB_S, ulMtHndl);
    if ( pstUsrCb == NULL )
    {
        return;
    }

    clock_gettime(CLOCK_MONOTONIC, &stTime);

    pthread_mutex_lock(&pstUsrCb->stMutex);

    if ( NULL != szDescFmt )
    {
        va_start(args, szDescFmt);
        vsnprintf(szDesc, sizeof(szDesc), szDescFmt, args);
        va_end(args);
        __MT_DBG_Print(ulMtHndl, "\r\nDump [%s][%u][%05ld.%03ld] pkt:", szDesc, u32Len, stTime.tv_sec, stTime.tv_nsec/1000000);
    }

    if ( u32Len > MT_DATA_MAX_LEN )
    {
        u32Len = MT_DATA_MAX_LEN;
    }

    for (i = 0 ; (i + 16) <= u32Len; i += 16)
    {
        __MT_DBG_Print(ulMtHndl,
                       "\r\n0x%02x:"
                       "%02x %02x %02x %02x %02x %02x %02x %02x "
                       "%02x %02x %02x %02x %02x %02x %02x %02x", i,
                       pu8Data[i],    pu8Data[i+1],  pu8Data[i+2],  pu8Data[i+3],
                       pu8Data[i+4],  pu8Data[i+5],  pu8Data[i+6],  pu8Data[i+7],
                       pu8Data[i+8],  pu8Data[i+9],  pu8Data[i+10], pu8Data[i+11],
                       pu8Data[i+12], pu8Data[i+13], pu8Data[i+14], pu8Data[i+15]);
    }

    if ( i < u32Len )
    {
        __MT_DBG_Print(ulMtHndl, "\r\n0x%02x:", i);
        for ( ; i < u32Len; i++)
        {
            __MT_DBG_Print(ulMtHndl, "%02x ", pu8Data[i]);
        }
    }

    __MT_DBG_Print(ulMtHndl, "\r\n");

    pthread_mutex_unlock(&pstUsrCb->stMutex);
    return;
}

#if 0
#endif

INT MT_USR_GetFd(ULONG ulMtHndl)
{
    MT_USR_CB_S *pstUsrCb = VA_PTR_TYPE(MT_USR_CB_S, ulMtHndl);
    if ( pstUsrCb == NULL )
    {
        return VA_INVALID_FD;
    }

    return pstUsrCb->iMtFd;
}

ULONG MT_USR_Open(const char *szName, const char *szDesc, U32 u32DebugCap, U32 u32Flg)
{
    MT_UA_OPEN_PARAM_S stOpenParam;
    MT_USR_CB_S *pstUsrCb;
    LONG lRet;
    INT  iRet;

    if ( szName == NULL )
    {
        return MT_USR_INVAL_HANDLE;
    }

    pstUsrCb = VA_Zmalloc(sizeof(MT_USR_CB_S));
    if ( pstUsrCb == NULL )
    {
        return MT_USR_INVAL_HANDLE;
    }

    pthread_mutex_init(&pstUsrCb->stMutex, NULL);
    pstUsrCb->iMtFd = -1;

    pstUsrCb->szName = strdup(szName);
    if ( szDesc )
    {
        pstUsrCb->szDesc = strdup(szDesc);
    }
    else
    {
        pstUsrCb->szDesc = strdup(szName);
    }

    if ( pstUsrCb->szName == NULL || pstUsrCb->szDesc == NULL )
    {
        MT_USR_Close((ULONG)pstUsrCb);
        return MT_USR_INVAL_HANDLE;
    }

    pstUsrCb->u32DebugCap = u32DebugCap;

    pstUsrCb->iMtFd = open(VA_FS_PATH MT_UA_FILE, O_RDWR);
    if ( pstUsrCb->iMtFd < 0 )
    {
        MT_USR_Close((ULONG)pstUsrCb);
        return MT_USR_INVAL_HANDLE;
    }

    iRet = fcntl(pstUsrCb->iMtFd, F_GETFD);
    if ( iRet >= 0 )
    {
        fcntl(pstUsrCb->iMtFd, F_SETFD, FD_CLOEXEC|iRet);
    }

    stOpenParam.u32DebugCap = u32DebugCap;
    stOpenParam.u32Flag     = u32Flg;
    strncpy(stOpenParam.szName, szName, MT_NAME_MAX_LEN);
    VA_SET_STR_ARR_TERM(stOpenParam.szName);
    strncpy(stOpenParam.szDesc, pstUsrCb->szDesc, MT_DESC_MAX_LEN);
    VA_SET_STR_ARR_TERM(stOpenParam.szDesc);

    lRet = ioctl(pstUsrCb->iMtFd, MT_IOCTL_UA_OPEN, &stOpenParam);
    if ( lRet < 0 )
    {
        MT_USR_Close((ULONG)pstUsrCb);
        return MT_USR_INVAL_HANDLE;
    }

    MT_USR_RegDbgCmdLine(pstUsrCb);
    return (ULONG)pstUsrCb;
}

VOID MT_USR_Close(ULONG ulMtHndl)
{
    MT_USR_CB_S *pstUsrCb = VA_PTR_TYPE(MT_USR_CB_S, ulMtHndl);
    if ( pstUsrCb == NULL )
    {
        return;
    }

    if ( pstUsrCb->iMtFd >= 0 )
    {
        close(pstUsrCb->iMtFd);
    }

    if (pstUsrCb->szDesc)
    {
        VA_Free(pstUsrCb->szDesc);
    }

    if (pstUsrCb->szName)
    {
        VA_Free(pstUsrCb->szName);
    }

    VA_Free(pstUsrCb);
    return;
}