#include "va_kern_pub.h"
#include "mt_kern_util.h"
#include "mt_mgr.h"
#include "mt_cmd_proc.h"
#include "va_util.h"
#include <linux/delay.h>

static MT_MGR_S gstMtMgr =
{
    .stCbHead = LIST_HEAD_INIT(gstMtMgr.stCbHead),
    .stCaHead = LIST_HEAD_INIT(gstMtMgr.stCaHead),
    .stUaHead = LIST_HEAD_INIT(gstMtMgr.stUaHead),
    .u64CmdIdSeq = 1,
    .u32MaxPktOutLen = MT_DATA_DEF_LEN,
};

VOID MT_FreeClList(struct list_head *pstHead);
extern struct file_operations *MT_GetFileOps(void);

#if 0
#endif

U32 MT_GetDbgLevel(ULONG ulExecId)
{
    const char *szDbgLevel;
    U32 u32DbgLevel = 0;

    if (MT_GetTextVal(ulExecId, "msg", &szDbgLevel) == 0)
    {
        u32DbgLevel = MT_DBG_MSG;
    }
    else if (MT_GetTextVal(ulExecId, "info", &szDbgLevel) == 0)
    {
        u32DbgLevel = MT_DBG_INFO;
    }
    else if (MT_GetTextVal(ulExecId, "err", &szDbgLevel) == 0)
    {
        u32DbgLevel = MT_DBG_ERR;
    }
    else if (MT_GetTextVal(ulExecId, "pkt", &szDbgLevel) == 0)
    {
        u32DbgLevel = MT_DBG_PKT;
    }
    else if (MT_GetTextVal(ulExecId, "all", &szDbgLevel) == 0)
    {
        u32DbgLevel = MT_DBG_ALL;
    }

    return u32DbgLevel;
}

static VOID MT_OpenDbgLevel(ULONG ulExecId)
{
    U32 u32DbgLevel = MT_GetDbgLevel(ulExecId);
    MT_CB_S *pstMtCb = (MT_CB_S *)MT_GetPrivCb(ulExecId);

    BUG_ON(pstMtCb == NULL);
    pstMtCb->u32Debug |= pstMtCb->u32DebugCap & u32DbgLevel;
    return;
}

static VOID MT_CloseDbgLevel(ULONG ulExecId)
{
    U32 u32DbgLevel  = MT_GetDbgLevel(ulExecId);
    MT_CB_S *pstMtCb = (MT_CB_S *)MT_GetPrivCb(ulExecId);

    BUG_ON(pstMtCb == NULL);
    pstMtCb->u32Debug &= (~u32DbgLevel);
    return;
}

VOID MT_DispDbgLevel(U32 u32Debug, ULONG ulExecId)
{
    MT_PRINT("\r\ndump debug level:");

    if ( u32Debug & MT_DBG_MSG )
    {
        MT_PRINT("\r\n msg");
    }

    if ( u32Debug & MT_DBG_ERR )
    {
        MT_PRINT("\r\n error");
    }

    if ( u32Debug & MT_DBG_INFO )
    {
        MT_PRINT("\r\n info");
    }

    if ( u32Debug & MT_DBG_PKT )
    {
        MT_PRINT("\r\n pkt");
    }
}


static VOID MT_DispCbDbgLevel(ULONG ulExecId)
{
    MT_CB_S *pstMtCb = (MT_CB_S *)MT_GetPrivCb(ulExecId);

    MT_DispDbgLevel(pstMtCb->u32Debug, ulExecId);
}

static VOID MT_RegDbgCmdLine(MT_CB_S *pstMtCb)
{
    //MT_DEF_TEXT_CMD_ITEM(stMsg,  "msg",  "msg",  "message");
    //MT_DEF_TEXT_CMD_ITEM(stErr,  "err",  "err",  "error");
    //MT_DEF_TEXT_CMD_ITEM(stPkt,  "pkt",  "pkt",  "packet");
    //MT_DEF_TEXT_CMD_ITEM(stMod,  "mod",  pstMtCb->szName, pstMtCb->szDesc);
    MT_CMD_ITEM_ARR_S stDbgItemArr;
    MT_INIT_ITEM_ARR(&stDbgItemArr);

    if ( pstMtCb->u32DebugCap & MT_DBG_MSG )
    {
        MT_AddItemToArr(&stDbgItemArr, MT_DEF_ITEM_MSG);
    }

    if ( pstMtCb->u32DebugCap & MT_DBG_PKT )
    {
        MT_AddItemToArr(&stDbgItemArr, MT_DEF_ITEM_PKT);
    }

    if ( pstMtCb->u32DebugCap & MT_DBG_ERR)
    {
        MT_AddItemToArr(&stDbgItemArr, MT_DEF_ITEM_ERR);
    }

    if ( pstMtCb->u32DebugCap & MT_DBG_INFO )
    {
        MT_AddItemToArr(&stDbgItemArr, MT_DEF_ITEM_INFO);
    }

    MT_REG_CMD_LINE(MT_OpenDbgLevel,  MT_DEF_ITEM_DEBUG, MT_DEF_ITEM_MOD, &stDbgItemArr);
    MT_REG_CMD_LINE(MT_CloseDbgLevel, MT_DEF_ITEM_UNDO,  MT_DEF_ITEM_DEBUG, MT_DEF_ITEM_MOD, &stDbgItemArr);

    if ( pstMtCb->u32DebugCap )
    {
        MT_REG_CMD_LINE(MT_OpenDbgLevel,    MT_DEF_ITEM_DEBUG, MT_DEF_ITEM_MOD,   MT_DEF_ITEM_ALL);
        MT_REG_CMD_LINE(MT_CloseDbgLevel,   MT_DEF_ITEM_UNDO,  MT_DEF_ITEM_DEBUG, MT_DEF_ITEM_MOD, MT_DEF_ITEM_ALL);
        MT_REG_CMD_LINE(MT_DispCbDbgLevel,  MT_DEF_ITEM_DISP,  MT_DEF_ITEM_DEBUG, MT_DEF_ITEM_MOD);
    }
}

#if 0
#endif

VOID MT_RegCa(MT_CA_S *pstCa)
{
    mutex_lock(&gstMtMgr.stLock);
    list_add(&pstCa->stNode, &gstMtMgr.stCaHead);
    mutex_unlock(&gstMtMgr.stLock);
}

VOID MT_UnRegCa(MT_CA_S *pstCa)
{
    mutex_lock(&gstMtMgr.stLock);
    list_del(&pstCa->stNode);
    mutex_unlock(&gstMtMgr.stLock);
}

static VOID MT_NotifyCa(void)
{
    MT_CA_S *pstCa;

    mutex_lock(&gstMtMgr.stLock);
    list_for_each_entry(pstCa, &gstMtMgr.stCaHead, stNode)
    {
        pstCa->bClChanged = TRUE;
    }
    mutex_unlock(&gstMtMgr.stLock);

    wake_up_all(&gstMtMgr.stWaitHead);
}

#if 0
#endif

VOID MT_RegUa(MT_UA_S *pstUa)
{
    mutex_lock(&gstMtMgr.stUaLock);
    list_add(&pstUa->stNode, &gstMtMgr.stUaHead);
    mutex_unlock(&gstMtMgr.stUaLock);
}

VOID MT_UnRegUa(MT_UA_S *pstUa)
{
    mutex_lock(&gstMtMgr.stUaLock);
    list_del(&pstUa->stNode);
    mutex_unlock(&gstMtMgr.stUaLock);
}

#if 0
#endif

static MT_CL_S *MT_FindClById(U64 u64CmdId)
{
    MT_CL_S *pstCl;
    U32 u32HashId = VA_Hash64(u64CmdId, MT_CL_HASH_BITS);

    hlist_for_each_entry(pstCl, &gstMtMgr.astHashTbl[u32HashId], stHashNode)
    {
        if ( pstCl->u64CmdId == u64CmdId )
        {
            return pstCl;
        }
    }

    return NULL;
}

static MT_VIEW_S *MT_FindView(U64 u64ViewId)
{
    MT_VIEW_S *pstView;
    U32 u32HashId = VA_Hash64(u64ViewId, MT_VIEW_HASH_BITS);

    hlist_for_each_entry(pstView, &gstMtMgr.astViewHashTbl[u32HashId], stNode)
    {
        if ( pstView->u64ViewId == u64ViewId )
        {
            return pstView;
        }
    }

    return NULL;
}

static void MT_ViewClInc(U64 u64ViewId)
{
    MT_VIEW_S *pstView;

    pstView = MT_FindView(u64ViewId);
    if ( pstView )
    {
        pstView->u32ClCnt++;
    }
}

static void MT_ViewInc(U64 u64ViewId)
{
    MT_VIEW_S *pstView;

    pstView = MT_FindView(u64ViewId);
    if ( pstView )
    {
        pstView->u32ViewCnt++;
    }
}

static void MT_ViewClDec(U64 u64ViewId)
{
    MT_VIEW_S *pstView;

    pstView = MT_FindView(u64ViewId);
    if ( pstView )
    {
        pstView->u32ClCnt--;
    }
}

static void MT_ViewDec(U64 u64ViewId)
{
    MT_VIEW_S *pstView;

    pstView = MT_FindView(u64ViewId);
    if ( pstView )
    {
        pstView->u32ViewCnt--;
    }
}


static INT MT_RegView(U64 u64ViewId, const char *szName, const char *szDesc)
{
    MT_VIEW_S *pstView;
    U32 u32HashId = VA_Hash64(u64ViewId, MT_VIEW_HASH_BITS);

    pstView = VA_MallocCb(MT_VIEW_S);
    if ( pstView == NULL )
    {
        return -ENOMEM;
    }

    pstView->u64ViewId = u64ViewId;
    strncpy(pstView->szName, szName, MT_NAME_MAX_LEN);
    strncpy(pstView->szDesc, szDesc, MT_DESC_MAX_LEN);

    mutex_lock(&gstMtMgr.stLock);
    hlist_add_head(&pstView->stNode, &gstMtMgr.astViewHashTbl[u32HashId]);
    if ( u64ViewId != 0 )
    {
        MT_ViewInc(0);
    }
    mutex_unlock(&gstMtMgr.stLock);

    return 0;
}

VOID MT_UnRegView(U64 u64ViewId)
{
    MT_VIEW_S *pstView;
    U32 u32HashId = VA_Hash64(u64ViewId, MT_VIEW_HASH_BITS);

    mutex_lock(&gstMtMgr.stLock);
    hlist_for_each_entry(pstView, &gstMtMgr.astViewHashTbl[u32HashId], stNode)
    {
        if ( pstView->u64ViewId == u64ViewId )
        {
            hlist_del(&pstView->stNode);
            VA_Free(pstView);
            MT_ViewDec(0);
            mutex_unlock(&gstMtMgr.stLock);
            return;
        }
    }
    mutex_unlock(&gstMtMgr.stLock);
}

int MT_RegMt(MT_CB_S *pstMtCb)
{
    MT_CB_S *pstMtTmpCb;

    if ( pstMtCb->szName == NULL )
    {
        VA_LOG_ERR("Invalid register mt cb, mt name is NULL");
        return -EINVAL;
    }

    if ( pstMtCb->szDesc == NULL )
    {
        pstMtCb->szDesc = pstMtCb->szName;
    }

	if ( pstMtCb->szFileName == NULL )
	{
		snprintf(pstMtCb->szFileNameBuf, sizeof(pstMtCb->szFileName), "mt_%s", pstMtCb->szName);
	    pstMtCb->szFileName = pstMtCb->szFileNameBuf;
	}

	pstMtCb->u32Debug = 0;
	INIT_LIST_HEAD(&pstMtCb->stNode);
    INIT_LIST_HEAD(&pstMtCb->stClHead);
    INIT_LIST_HEAD(&pstMtCb->stFileHead);

    mutex_lock(&gstMtMgr.stLock);

    list_for_each_entry(pstMtTmpCb, &gstMtMgr.stCbHead, stNode)
    {
        if ( strcmp(pstMtTmpCb->szName, pstMtCb->szName) == 0 )
        {
            mutex_unlock(&gstMtMgr.stLock);
            VA_LOG_ERR("Failed to register mt cb of name %s, It has already exist!", pstMtCb->szName);
            return -EEXIST;
        }
    }

    list_add_tail(&pstMtCb->stNode, &gstMtMgr.stCbHead);
    mutex_unlock(&gstMtMgr.stLock);
    if ( pstMtCb->pfnRegCmd )
    {
        pstMtCb->pfnRegCmd(pstMtCb);
    }

    if ( pstMtCb->u64ViewId )
    {
        MT_RegView(pstMtCb->u64ViewId, pstMtCb->szName, pstMtCb->szDesc);
    }

    if ( pstMtCb->u32DebugCap )
    {
        MT_RegDbgCmdLine(pstMtCb);
    }

    MT_NotifyCa();
    return 0;
}

VOID MT_UnRegMt(MT_CB_S *pstMtCb)
{
    mutex_lock(&gstMtMgr.stLock);

    list_del(&pstMtCb->stNode);
    MT_FreeClList(&pstMtCb->stClHead);

    mutex_unlock(&gstMtMgr.stLock);

    if ( pstMtCb->u64ViewId )
    {
        MT_UnRegView(pstMtCb->u64ViewId);
    }

    MT_NotifyCa();
}

VOID MT_GlobalLock(VOID)
{
    mutex_lock(&gstMtMgr.stLock);
}

VOID MT_GlobalUnLock(VOID)
{
    mutex_unlock(&gstMtMgr.stLock);
}

static VOID MT_AddToMgrHash(MT_CL_S *pstCl)
{
    U32 u32HashId = VA_Hash64(pstCl->u64CmdId, MT_CL_HASH_BITS);

    hlist_add_head(&pstCl->stHashNode, &gstMtMgr.astHashTbl[u32HashId]);
    MT_ViewClInc(pstCl->u64ViewId);
    return;
}

static VOID MT_DelFrmMgrHash(MT_CL_S *pstCl)
{
    hlist_del(&pstCl->stHashNode);
    MT_ViewClDec(pstCl->u64ViewId);
}


#if 0
#endif

VOID MT_DBG_PrintToEachCaFrmUsr(const char __user *szBuf, U32 u32Len)
{
    MT_CA_S *pstCa;
    loff_t  pos = 0;

    mutex_lock(&gstMtMgr.stUaLock);
    list_for_each_entry(pstCa, &gstMtMgr.stCaHead, stNode)
    {
        if ( pstCa->bDebug == FALSE )
        {
            continue;
        }

        vfs_write(pstCa->pstFile, szBuf, u32Len, &pos);
    }
    mutex_unlock(&gstMtMgr.stUaLock);
}

VOID MT_DBG_PrintToEachCa(char *szBuf, U32 u32Len)
{
    MT_CA_S *pstCa;

    mutex_lock(&gstMtMgr.stUaLock);
    list_for_each_entry(pstCa, &gstMtMgr.stCaHead, stNode)
    {
        if ( pstCa->bDebug == FALSE )
        {
            continue;
        }

        MT_KnlWirte(pstCa->pstFile, szBuf, u32Len);
    }
    mutex_unlock(&gstMtMgr.stUaLock);
}

VOID MT_DBG_Print(const char *szDesc, const char *szFmt, ...)
{
	struct timespec stTime;
    va_list args;
    char *pBuf;
    int  iRet;
	char szBuf[196];

	get_monotonic_boottime(&stTime);

    if ( szDesc )
    {
        iRet = VA_SnprintEx(szBuf, sizeof(szBuf), "\r\n[%s][%05ld.%03ld]:", szDesc, stTime.tv_sec, stTime.tv_nsec / 1000000);
        MT_DBG_PrintToEachCa(szBuf, iRet);
    }

    pBuf = (char *)VA_Malloc(PAGE_SIZE);
    if ( pBuf == NULL )
    {
        return;
    }

    va_start(args, szFmt);
    iRet = VA_VsnprintEx(pBuf, PAGE_SIZE, szFmt, args);
    va_end(args);

    if ( iRet > 0 )
    {
        MT_DBG_PrintToEachCa(pBuf, iRet);
    }

    VA_Free(pBuf);
}

VOID MT_DBG_PktPrint(VOID *pData, U32 u32Len, const char *szDescFmt, ...)
{
	struct timespec stTime;
    char szBuf[196];
    U8   *pu8Data = (U8 *)pData;
    INT iRet;
    U32 i;

	get_monotonic_boottime(&stTime);

    if ( szDescFmt )
    {
        char szTmpBuf[128];
        va_list args;

        va_start(args, szDescFmt);
        iRet = VA_VsnprintEx(szTmpBuf, sizeof(szTmpBuf), szDescFmt, args);
        va_end(args);

        iRet = VA_SnprintEx(szBuf, sizeof(szBuf), "\r\nDump [%s][%u][%05ld.%03ld] pkt:", szTmpBuf, u32Len, stTime.tv_sec, stTime.tv_nsec / 1000000);
        MT_DBG_PrintToEachCa(szBuf, iRet);
    }

    if ( u32Len > gstMtMgr.u32MaxPktOutLen )
    {
        u32Len = gstMtMgr.u32MaxPktOutLen;
    }

    for (i = 0 ; (i + 16) <= u32Len; i += 16)
    {
        iRet = sprintf(szBuf,
                       "\r\n0x%02x:"
                       "%02x %02x %02x %02x %02x %02x %02x %02x "
                       "%02x %02x %02x %02x %02x %02x %02x %02x", i,
                       pu8Data[i],    pu8Data[i+1],  pu8Data[i+2],  pu8Data[i+3],
                       pu8Data[i+4],  pu8Data[i+5],  pu8Data[i+6],  pu8Data[i+7],
                       pu8Data[i+8],  pu8Data[i+9],  pu8Data[i+10], pu8Data[i+11],
                       pu8Data[i+12], pu8Data[i+13], pu8Data[i+14], pu8Data[i+15]);
        MT_DBG_PrintToEachCa(szBuf, iRet);
    }

    if ( i < u32Len )
    {
        iRet = sprintf(szBuf, "\r\n0x%02x:", i);
        for ( ; i < u32Len; i++)
        {
            iRet += sprintf(szBuf + iRet, "%02x ", pu8Data[i]);
        }

        MT_DBG_PrintToEachCa(szBuf, iRet);
    }
}

#if 0
#endif

long MT_GetClCount(MT_CL_NUM_S __user *pstClNUm)
{
    MT_CL_NUM_S stClNum;
    MT_VIEW_S *pstView;

    if ( copy_from_user(&stClNum, pstClNUm, sizeof(MT_CL_NUM_S)) )
    {
        return -EFAULT;
    }

    mutex_lock(&gstMtMgr.stLock);

    pstView = MT_FindView(stClNum.u64ViewId);
    if ( pstView )
    {
        stClNum.u64ViewId  = pstView->u64ViewId;
        stClNum.u32ClCnt   = pstView->u32ClCnt;
        stClNum.u32ViewCnt = pstView->u32ViewCnt;
    }
    else
    {
        mutex_unlock(&gstMtMgr.stLock);
        return -ENOENT;
    }

    mutex_unlock(&gstMtMgr.stLock);

    if ( copy_to_user(pstClNUm, &stClNum, sizeof(MT_CL_NUM_S)) )
    {
        return -EFAULT;
    }

    return 0;
}

long MT_GetAllCmdIds(MT_CMD_IDS_INFO_S __user *pstIdsInfo)
{
    MT_CMD_IDS_INFO_S stIdsInfo;
    MT_VIEW_S *pstView;
    MT_CL_S   *pstCl;
    U32 i;
    U32 u32Pos = 0;
    U32 u32Num = 0;

    if ( copy_from_user(&stIdsInfo, pstIdsInfo, sizeof(MT_CMD_IDS_INFO_S)) )
    {
        return -EFAULT;
    }

    mutex_lock(&gstMtMgr.stLock);
    pstView = MT_FindView(stIdsInfo.u64ViewId);
    if ( pstView == NULL || pstView->u32ClCnt != stIdsInfo.u32Num )
    {
        mutex_unlock(&gstMtMgr.stLock);
        return -EINVAL;
    }

    // maybe, this need to optimize
    for (i = 0; i < VA_ARR_SIZE(gstMtMgr.astHashTbl); i++)
    {
        hlist_for_each_entry(pstCl, &gstMtMgr.astHashTbl[i], stHashNode)
        {
            if ( pstCl->u64ViewId == stIdsInfo.u64ViewId )
            {
                if (copy_to_user(&pstIdsInfo->au64CmdId[u32Pos++], &pstCl->u64CmdId, sizeof(U64)))
                {
                    mutex_unlock(&gstMtMgr.stLock);
                    return -EFAULT;
                }
            }

            u32Num++;
        }

        if  ( u32Num > 500 )
        {
            u32Num = 0;
            msleep(100);
        }
    }

    mutex_unlock(&gstMtMgr.stLock);
    return 0;
}

long MT_GetAllViewIds(MT_CMD_IDS_INFO_S __user *pstIdsInfo)
{
    MT_VIEW_S *pstView;
    U32 i;
    U32 u32Num;
    U32 u32Pos = 0;

    if ( copy_from_user(&u32Num, &pstIdsInfo->u32Num, sizeof(U32)) )
    {
        return -EFAULT;
    }

    mutex_lock(&gstMtMgr.stLock);
    pstView = MT_FindView(0);
    if ( pstView == NULL || pstView->u32ViewCnt != u32Num )
    {
        mutex_unlock(&gstMtMgr.stLock);
        return -ENOSPC;
    }

    for (i = 0; i < VA_ARR_SIZE(gstMtMgr.astViewHashTbl); i++)
    {
        hlist_for_each_entry(pstView, &gstMtMgr.astViewHashTbl[i], stNode)
        {
            if ( pstView->u64ViewId == 0 )
            {
                continue;
            }

            if (copy_to_user(&pstIdsInfo->au64CmdId[u32Pos++], &pstView->u64ViewId, sizeof(U64)))
            {
                mutex_unlock(&gstMtMgr.stLock);
                return -EFAULT;
            }
        }

    }

    mutex_unlock(&gstMtMgr.stLock);
    return 0;
}

U32 MT_GetCmdEntryArraySize(const MT_CMD_ENTRY_S *pstEntry)
{
    U32 nArrSize = 1;
    U32 i;

    if ( pstEntry->u8Flg & MT_ITEM_FLG_ARRAY )
    {
        for ( i = 0; pstEntry->ppstEntryArr[i] != NULL; i++)
        {
            nArrSize++;
        }
    }

    return nArrSize;
}

long MT_FillUserItem(MT_CMD_ENTRY_S *pstEntry, MT_CMD_USR_ITEM_S __user *pstItem, U8 u8Flg)
{
    MT_CMD_USR_ITEM_S stUsrItem;

    stUsrItem.u8Type = pstEntry->u8Type;
    stUsrItem.u8Flg  = pstEntry->u8Flg | u8Flg;
    stUsrItem.u64MaxVal = pstEntry->u64MaxVal;
    stUsrItem.u64MinVal = pstEntry->u64MinVal;

    memcpy(stUsrItem.as8Id,  pstEntry->as8Id,  MT_ID_MAX_LEN);
    memcpy(stUsrItem.szName, pstEntry->szName, sizeof(pstEntry->szName));
    memcpy(stUsrItem.szDesc, pstEntry->szDesc, sizeof(pstEntry->szDesc));

    if ( copy_to_user(pstItem, &stUsrItem, sizeof(MT_CMD_USR_ITEM_S)))
    {
        return -EFAULT;
    }

    return 0;
}

long MT_FillUserArrItem(MT_CMD_ENTRY_S *pstEntry, MT_CMD_USR_ITEM_S __user *pstItem)
{
    long lRet;
    U32 i;
    U8  u8Flg = 0;

    lRet = MT_FillUserItem(pstEntry, pstItem++, 0);
    if ( lRet < 0 )
    {
        return lRet;
    }

    for ( i = 0; pstEntry->ppstEntryArr[i] != NULL; i++)
    {
        if ( pstEntry->ppstEntryArr[i + 1] == NULL )
        {
            u8Flg = MT_ITEM_FLG_ARR_END;
        }

        lRet = MT_FillUserItem(pstEntry->ppstEntryArr[i], pstItem++, u8Flg);
        if ( lRet < 0 )
        {
            return lRet;
        }
    }

    return 0;
}

long MT_GetClInfo(MT_CL_INFO_S __user *pstClInfo)
{
    MT_CMD_ENTRY_S  *pstEntry;
    MT_CL_INFO_S stClInfo;
    MT_CL_S *pstCl;
    long lRet;
    U32 nPos = 0;

    if ( copy_from_user(&stClInfo, pstClInfo, sizeof(MT_CL_INFO_S)) )
    {
        return -EFAULT;
    }

    mutex_lock(&gstMtMgr.stLock);

    pstCl = MT_FindClById(stClInfo.u64CmdId);
    if ( pstCl == NULL )
    {
        mutex_unlock(&gstMtMgr.stLock);
        return -ENOENT;
    }

    // fill cmd line content
    list_for_each_entry(pstEntry, &pstCl->stHead, stNode)
    {
        U32 nEntryNum = MT_GetCmdEntryArraySize(pstEntry);

        if ( (nPos + nEntryNum) >= stClInfo.u32EntryNum )
        {
            mutex_unlock(&gstMtMgr.stLock);
            return -ENOSPC;
        }

        if ( pstEntry->u8Flg & MT_ITEM_FLG_ARRAY )
        {
            lRet = MT_FillUserArrItem(pstEntry, &pstClInfo->astItem[nPos]);
        }
        else
        {
            lRet = MT_FillUserItem(pstEntry, &pstClInfo->astItem[nPos], 0);
        }

        if ( lRet < 0)
        {
           mutex_unlock(&gstMtMgr.stLock);
           return lRet;
        }

        nPos += nEntryNum;
    }

    mutex_unlock(&gstMtMgr.stLock);

    stClInfo.u32EntryNum = nPos;
    stClInfo.u64ViewId   = pstCl->u64ViewId;
    if ( copy_to_user(pstClInfo, &stClInfo, sizeof(stClInfo)) )
    {
        return -EFAULT;
    }

    return 0;
}

long MT_GetViewInfo(MT_VIEW_INFO_S __user *pstViewInfo)
{
    MT_VIEW_INFO_S stViewInfo;
    MT_VIEW_S *pstView;

    if ( copy_from_user(&stViewInfo, pstViewInfo, sizeof(MT_VIEW_INFO_S)) )
    {
        return -EFAULT;
    }

    if ( stViewInfo.u64ViewId == 0 )
    {
        return -ENOENT;
    }

    mutex_lock(&gstMtMgr.stLock);

    pstView = MT_FindView(stViewInfo.u64ViewId);
    if ( pstView == NULL )
    {
        mutex_unlock(&gstMtMgr.stLock);
        return -ENOENT;
    }

    strcpy(stViewInfo.szName, pstView->szName);
    strcpy(stViewInfo.szDesc, pstView->szDesc);

    if (copy_to_user(pstViewInfo, &stViewInfo, sizeof(MT_VIEW_INFO_S)))
    {
        mutex_unlock(&gstMtMgr.stLock);
        return -EFAULT;
    }

    mutex_unlock(&gstMtMgr.stLock);
    return 0;
}

LONG MT_ExecUserInputCmd(MT_INPUT_CMD_PARAM_S __user *pstCmdParam)
{
    MT_INPUT_CMD_PARAM_S stCmdParam;
    MT_CL_S *pstCl;
    struct file *pstOutFile;
    LONG lRet;

    if ( copy_from_user(&stCmdParam, pstCmdParam, sizeof(MT_INPUT_CMD_PARAM_S)))
    {
        return -EFAULT;
    }

    if ( stCmdParam.u32BufLen > MT_MAX_INPUT_LEN )
    {
        return -E2BIG;
    }

    stCmdParam.szCmd[stCmdParam.u32BufLen] = 0;
    pstOutFile = fget(1);
    if ( pstOutFile == NULL )
    {
        return -ENOENT;
    }

    mutex_lock(&gstMtMgr.stLock);

    pstCl = MT_FindClById(stCmdParam.u64CmdId);
    if ( pstCl == NULL )
    {
        lRet = -ENOENT;
        goto unlock;
    }

    lRet = MT_ExecUserCmd(pstCl, pstOutFile, stCmdParam.szCmd);

unlock:
    mutex_unlock(&gstMtMgr.stLock);
    fput(pstOutFile);
    return lRet;
}

U32 MT_Poll(struct file *pstFile, poll_table *wait)
{
    MT_CA_S *pstCa;
    U32 u32Mask = 0;

    pstCa = VA_PTR_TYPE(MT_CA_S, pstFile->private_data);

    poll_wait(pstFile, &gstMtMgr.stWaitHead, wait);
    if ( pstCa->bClChanged )
    {
        u32Mask = POLLIN | POLLRDNORM;
    }

    return u32Mask;
}

LONG MT_GetChangeStatus(MT_CA_S *pstCa, BOOL __user *pbVal)
{
	mutex_lock(&gstMtMgr.stLock);

    if ( copy_to_user(pbVal, &pstCa->bClChanged, sizeof(BOOL)))
    {
		mutex_unlock(&gstMtMgr.stLock);
        return -EFAULT;
    }

    pstCa->bClChanged = FALSE;

	mutex_unlock(&gstMtMgr.stLock);
    return 0;
}

#if 0
#endif

int MT_ChkFileCb(MT_FILE_CB_S *pstFile)
{
	MT_CB_S *pstMtCb;
	MT_FILE_CB_S *pstTmpFile;

	list_for_each_entry(pstMtCb, &gstMtMgr.stCbHead, stNode)
	{
		list_for_each_entry(pstTmpFile, &pstMtCb->stFileHead, stNode)
		{
			if ( pstTmpFile == pstFile )
			{
			    return 0;
			}
		}
	}

	return -ENOENT;
}

MT_FILE_CB_S *MT_AllocFile(MT_CB_S *pstMtCb, const char *szFileName)
{
    MT_FILE_CB_S *pstFile;
	const char *szName;

    pstFile = VA_Zmalloc(sizeof(MT_FILE_CB_S));
    if ( pstFile == NULL )
    {
        return NULL;
    }

   	szName = MT_strdup(szFileName);
    if ( szName == NULL )
    {
        VA_Free(pstFile);
        return NULL;
    }

    INIT_LIST_HEAD(&pstFile->stClHead);
    list_add_tail(&pstFile->stNode, &pstMtCb->stFileHead);

	VA_FS_InitFileCb(&pstFile->stFile, szName, MT_GetFileOps(), pstFile);
    VA_FS_RegFile(&pstFile->stFile);
    return pstFile;
}

VOID MT_FreeFile(MT_FILE_CB_S *pstFile)
{
    BUG_ON(!list_empty(&pstFile->stClHead));

    VA_FS_UnRegFile(&pstFile->stFile);

    if ( pstFile->stFile.szName != NULL )
    {
        VA_Free(pstFile->stFile.szName);
    }

    list_del(&pstFile->stNode);
    VA_Free(pstFile);
    return;
}

VOID MT_AddClToFile(MT_FILE_CB_S *pstFile, MT_CL_S *pstCl)
{
    list_add_tail(&pstCl->stFileNode, &pstFile->stClHead);
    pstCl->pstFile = pstFile;
    atomic_inc(&pstFile->stRef);
}

VOID MT_DelClFrmFile(MT_CL_S *pstCl)
{
	MT_FILE_CB_S *pstFile;

	pstFile = pstCl->pstFile;
    if ( pstFile )
    {
        list_del(&pstCl->stFileNode);
        if ( atomic_dec_and_test(&pstFile->stRef) )
        {
            MT_FreeFile(pstFile);
        }

        pstCl->pstFile = NULL;
    }

    return;
}

MT_FILE_CB_S * MT_FindFile(MT_CB_S *pstMtCb, const char *szFileName)
{
    struct list_head *pstNode;
    MT_FILE_CB_S *pstFile;

    list_for_each(pstNode, &pstMtCb->stFileHead)
    {
        pstFile = container_of(pstNode, MT_FILE_CB_S, stNode);
        if ( strcmp(pstFile->stFile.szName, szFileName) == 0 )
        {
            return pstFile;
        }
    }

    return NULL;
}

static int MT_RegClToFile(MT_CB_S *pstMtCb, const char *szFileName, MT_CL_S *pstCl)
{
#ifdef MT_FILE_CL_SUPPORT
    MT_FILE_CB_S *pstFile;

    pstFile = MT_FindFile(pstMtCb, szFileName);
    if ( pstFile == NULL )
    {
        pstFile = MT_AllocFile(pstMtCb, szFileName);
        if ( pstFile == NULL )
        {
			VA_LOG_ERR("Failed to register cl file %s in cb %s", szFileName, pstMtCb->szName);
            return -ENOMEM;
        }
    }

    MT_AddClToFile(pstFile, pstCl);
#endif
	return 0;
}

#if 0
#endif

U64 MT_AllocCmdId(void)
{
    U64 u64CmdId;

    mutex_lock(&gstMtMgr.stLock);
    u64CmdId = gstMtMgr.u64CmdIdSeq++;
    mutex_unlock(&gstMtMgr.stLock);

    return u64CmdId;
}

VOID MT_FreeEntry(MT_CMD_ENTRY_S *pstEntry)
{
    U32 i;

    if ( pstEntry == NULL )
    {
        return;
    }

    if ( pstEntry->u8Flg & MT_ITEM_FLG_ARRAY )
    {
        for (i = 0; pstEntry->ppstEntryArr[i] != NULL; i++)
        {
            VA_Free(pstEntry->ppstEntryArr[i]);
        }
    }

    VA_Free(pstEntry);
}

VOID MT_ReleaseCmdLine(MT_CL_S *pstCl)
{
    struct list_head *pstNode;
    struct list_head *pstNextNode;
    MT_CMD_ENTRY_S *pstEntry;

    if ( pstCl->pstFile )
    {
        MT_DelClFrmFile(pstCl);
    }

    list_for_each_safe(pstNode, pstNextNode, &pstCl->stHead)
    {
        pstEntry = container_of(pstNode, MT_CMD_ENTRY_S, stNode);
        list_del(pstNode);
        MT_FreeEntry(pstEntry);
    }

    VA_Free(pstCl);
}

static VOID MT_SetEntry(MT_CB_S *pstMtCb, MT_CMD_ENTRY_S *pstEntry, const MT_CMD_ITEM_S *pstItem)
{
    if ( pstItem->u8Type == MT_ITEM_TYPE_MOD ) // fix the mod item info
    {
        strncpy(pstEntry->szDesc, pstMtCb->szDesc, MT_DESC_MAX_LEN);
        strncpy(pstEntry->szName, pstMtCb->szName, MT_NAME_MAX_LEN);
        pstEntry->u8Type = MT_ITEM_TYPE_TEXT;
    }
    else
    {
        if ( pstItem->szDesc )
        {
            strncpy(pstEntry->szDesc, pstItem->szDesc, MT_DESC_MAX_LEN);
        }

        strncpy(pstEntry->szName, pstItem->szName, MT_NAME_MAX_LEN);
        pstEntry->u8Type = pstItem->u8Type;
    }

    memcpy(pstEntry->as8Id, pstItem->as8Id, MT_ID_MAX_LEN);

    pstEntry->u8Flg     |= (pstItem->u8Flg & (~MT_ITEM_FLG_ARRAY));
    pstEntry->u64MinVal  = pstItem->u64MinVal;
    pstEntry->u64MaxVal  = pstItem->u64MaxVal;
}

static void MT_FixItem(MT_CMD_ITEM_S *pstItem)
{
    if ( pstItem->u8Type == MT_ITEM_TYPE_STR )
    {
        if ( pstItem->u64MinVal == 0 )
        {
            pstItem->u64MinVal = 1;
        }

        if ( pstItem->u64MaxVal >= MT_NAME_MAX_LEN )
        {
            pstItem->u64MaxVal = MT_NAME_MAX_LEN - 1;
        }
    }

    return;
}

static int MT_AddArrItem(MT_CB_S *pstMtCb, MT_CMD_ENTRY_S *pstEntry, MT_CMD_ITEM_S *pstItem, U32 u32ArrSize)
{
    MT_CMD_ITEM_ARR_S *pstItemArr = VA_PTR_TYPE(MT_CMD_ITEM_ARR_S, pstItem);
    const MT_CMD_ITEM_S *pstCurrItem;
    MT_CMD_ITEM_S *pstTmpItem;
    U32 i;

    pstEntry->u8Flg |= MT_ITEM_FLG_ARRAY;

    for (i = 0; i < (u32ArrSize - 1); i++)
    {
        MT_CMD_ENTRY_S *pstChildEntry;
        pstChildEntry = VA_Zmalloc(sizeof(MT_CMD_ENTRY_S));
        if ( pstChildEntry == NULL )
        {
            return -ENOMEM;
        }

        pstTmpItem = pstItemArr->apstItem[i];

        if ( (ULONG)pstTmpItem < MT_ITEM_SPEC_BUTT )
        {
            pstCurrItem = MT_GetSpecItem((ULONG)pstTmpItem);
            if ( pstCurrItem == NULL )
            {
                return -ENOENT;
            }
        }
        else
        {
            MT_FixItem(pstTmpItem);
            pstCurrItem = pstTmpItem;
        }

        MT_SetEntry(pstMtCb, pstChildEntry, pstCurrItem);

		pstChildEntry->u8Flg      |= MT_ITEM_FLG_ARRAY;
        pstEntry->ppstEntryArr[i] = pstChildEntry;
    }

    return 0;
}

static int MT_AddItem(MT_CB_S *pstMtCb, MT_CL_S *pstCl, MT_CMD_ITEM_S *pstItem)
{
    MT_CMD_ENTRY_S *pstEntry;
    const MT_CMD_ITEM_S *pstCurrItem;
    U32 u32ArrSize;
    int iRet;

	if ( (ULONG)pstItem < MT_ITEM_SPEC_BUTT )
	{
	    pstCurrItem = MT_GetSpecItem((ULONG)pstItem);
        if ( pstCurrItem == NULL )
        {
            return -ENOENT;
        }
	}
    else
    {
        MT_FixItem(pstItem);
        pstCurrItem = pstItem;
    }

	u32ArrSize = MT_GetCmdItemArraySize(pstCurrItem);
    pstEntry   = VA_Zmalloc(sizeof(MT_CMD_ENTRY_S) + u32ArrSize * sizeof(MT_CMD_ENTRY_S *));
    if ( pstEntry == NULL )
    {
        return -ENOMEM;
    }

    if ( u32ArrSize > 0 )
    {
        iRet = MT_AddArrItem(pstMtCb, pstEntry, pstItem, u32ArrSize);
        if ( iRet < 0 )
        {
            return iRet;
        }
    }

    MT_SetEntry(pstMtCb, pstEntry, pstCurrItem);
    list_add_tail(&pstEntry->stNode, &pstCl->stHead);
    return 0;
}

static int MT_FillItems(MT_CB_S *pstMtCb, MT_CL_S *pstCl, MT_CMD_ITEM_S *pstItem, va_list args)
{
    MT_CMD_ITEM_S *pstTmpItem;
	U32 nItemCnt = 1;

    if (MT_AddItem(pstMtCb, pstCl, pstItem) < 0)
    {
        return -ENOMEM;
    }

    pstTmpItem = VA_PTR_TYPE(MT_CMD_ITEM_S, va_arg(args, void *));
    while (pstTmpItem != NULL)
    {
		if ( ++nItemCnt > MT_ITEM_MAX_NUM )
		{
			VA_LOG_ERR("It's too many cmd items for cl, the first exceed item is %s", pstTmpItem->szName);
		    return -E2BIG;
		}

        if (MT_AddItem(pstMtCb, pstCl, pstTmpItem) < 0)
        {
            return -ENOMEM;
        }

        pstTmpItem = VA_PTR_TYPE(MT_CMD_ITEM_S, va_arg(args, void *));
    }

    return 0;
}

static MT_CL_S *MT_AllocCmdLine(MT_CL_CALL_BACK_PF pfnCallBack, VOID *pPrivCb)
{
    MT_CL_S *pstCl;

    pstCl = VA_Zmalloc(sizeof(MT_CL_S));
    if ( pstCl == NULL )
    {
        return NULL;
    }

    pstCl->pPrivCb     = pPrivCb;
    pstCl->pfnCallBack = pfnCallBack;
    INIT_LIST_HEAD(&pstCl->stHead);
	INIT_LIST_HEAD(&pstCl->stClNode);
	INIT_LIST_HEAD(&pstCl->stFileNode);
	INIT_HLIST_NODE(&pstCl->stHashNode);

    return pstCl;
}

static VOID MT_AddCmdLine(MT_CB_S *pstMtCb, const char *szFileName, MT_CL_S *pstCl)
{
	if ( MT_RegClToFile(pstMtCb, szFileName, pstCl) < 0 )
	{
		MT_ReleaseCmdLine(pstCl);
	    return ;
	}

    list_add_tail(&pstCl->stClNode, &pstMtCb->stClHead);
    pstCl->u64CmdId  = gstMtMgr.u64CmdIdSeq++;
    pstCl->u64ViewId = pstMtCb->u64ViewId;
    MT_AddToMgrHash(pstCl);
}

VOID MT_RegCmdLine(MT_CB_S *pstMtCb, VOID *pPrivCb, const char *szFileName, MT_CL_CALL_BACK_PF pfnCallBack, MT_CMD_ITEM_S *pstItem, ...)
{
    MT_CL_S *pstCl;
    va_list args;
    INT iRet;

    pstCl = MT_AllocCmdLine(pfnCallBack, pPrivCb);
    if ( pstCl == NULL )
    {
        return;
    }

    mutex_lock(&gstMtMgr.stLock);

    va_start(args, pstItem);
    iRet = MT_FillItems(pstMtCb, pstCl, pstItem, args);
    va_end(args);

    if ( iRet == 0 )
    {
        MT_AddCmdLine(pstMtCb, szFileName, pstCl);
    }
    else
    {
        MT_ReleaseCmdLine(pstCl);
		VA_LOG_ERR("Failed to register cmdline for cb %s", pstMtCb->szName);
    }

    mutex_unlock(&gstMtMgr.stLock);

    MT_NotifyCa();
    return;
}

static void MT_ConvertUsrItem(MT_CMD_ITEM_S *pstItem, MT_CMD_USR_ITEM_S *pstUsrItem)
{
    VA_CB_ZERO(pstItem);

    pstItem->u8Type = pstUsrItem->u8Type;
    pstItem->u8Flg  = 0;
    pstItem->szDesc = pstUsrItem->szDesc;
    pstItem->szName = pstUsrItem->szName;
    pstItem->u64MinVal = pstUsrItem->u64MinVal;
    pstItem->u64MaxVal = pstUsrItem->u64MaxVal;

    VA_ARR_CPY(pstItem->as8Id, pstUsrItem->as8Id);
}

static int MT_AddUsrItems(MT_CB_S *pstMtCb, MT_CL_S *pstCl, MT_UA_CL_INFO_S *pstClInfo)
{
    MT_CMD_ITEM_S *pstItem;
    MT_CMD_USR_ITEM_S *pstUsrItem;
    MT_CMD_ITEM_ARR_S stItemArr;
    U32 nPos;
    U32 i;

    pstItem = VA_Malloc(sizeof(MT_CMD_ITEM_S) * (MT_ITEM_MAX_ARR + 1));
    if ( pstItem == NULL )
    {
        return -ENOMEM;
    }

    for ( i = 0; i < pstClInfo->u32EntryNum; i++)
    {
        pstUsrItem = pstClInfo->astItem + i;

        if ( pstUsrItem->u8Flg & MT_ITEM_FLG_ARRAY )
        {
            MT_INIT_ITEM_ARR(&stItemArr);
            nPos = 0;

            for ( ; i < pstClInfo->u32EntryNum; i++ )
            {
                pstUsrItem = pstClInfo->astItem + i;
                if ( (pstUsrItem->u8Flg & MT_ITEM_FLG_ARRAY) == 0 )
                {
                    return -EINVAL;
                }

                if ( pstUsrItem->u8Flg & MT_ITEM_FLG_SPEC )
                {
                    MT_AddItemToArr(&stItemArr, (MT_CMD_ITEM_S *)((ULONG)pstUsrItem->u32SpecItemId));
                }
                else
                {
                    MT_ConvertUsrItem(pstItem + nPos, pstUsrItem);
                    MT_AddItemToArr(&stItemArr, pstItem + nPos);
                    nPos++;
                }

                if ( pstUsrItem->u8Flg & MT_ITEM_FLG_ARR_END )
                {
                    break;
                }
            }

	        if (MT_AddItem(pstMtCb, pstCl, &stItemArr.stItem) < 0)
	        {
                VA_Free(pstItem);
	            return -ENOMEM;
	        }
        }
        else
        {
    		if ( pstUsrItem->u8Flg & MT_ITEM_FLG_SPEC )
    		{
    	        if (MT_AddItem(pstMtCb, pstCl, (MT_CMD_ITEM_S *)((ULONG)pstUsrItem->u32SpecItemId)) < 0)
    	        {
                    VA_Free(pstItem);
    	            return -ENOMEM;
    	        }
    		}
    		else
    		{
                MT_ConvertUsrItem(pstItem, pstUsrItem);
    	        if (MT_AddItem(pstMtCb, pstCl, pstItem) < 0)
    	        {
                    VA_Free(pstItem);
    	            return -ENOMEM;
    	        }
    		}
        }
    }

    VA_Free(pstItem);
    return 0;
}

VOID MT_RegUsrCmdLine(MT_CB_S *pstMtCb, VOID *pPrivCb, MT_CL_CALL_BACK_PF pfnCallBack, MT_UA_CL_INFO_S *pstClInfo)
{
    MT_CL_S *pstCl;
    INT iRet;

    pstCl = MT_AllocCmdLine(pfnCallBack, pPrivCb);
    if ( pstCl == NULL )
    {
        return;
    }

    mutex_lock(&gstMtMgr.stLock);

    iRet = MT_AddUsrItems(pstMtCb, pstCl, pstClInfo);
    if ( iRet == 0 )
    {
        MT_AddCmdLine(pstMtCb, pstMtCb->szName, pstCl);
    }
    else
    {
        MT_ReleaseCmdLine(pstCl);
		VA_LOG_ERR("Failed to register cmdline for cb %s", pstMtCb->szName);
    }

    mutex_unlock(&gstMtMgr.stLock);

    MT_NotifyCa();
    return;
}

VOID MT_FreeClList(struct list_head *pstHead)
{
    struct list_head *pstNode;
    struct list_head *pstNextNode;
    MT_CL_S *pstCl;

    list_for_each_safe(pstNode, pstNextNode, pstHead)
    {
        pstCl = container_of(pstNode, MT_CL_S, stClNode);
        list_del(pstNode);
        MT_DelFrmMgrHash(pstCl);
        MT_ReleaseCmdLine(pstCl);
    }
}

U32 MT_GetCmdItemArraySize(const MT_CMD_ITEM_S *pstItem)
{
    MT_CMD_ITEM_ARR_S *pstItemArr;

    if ( pstItem->u8Flg & MT_ITEM_FLG_ARRAY )
    {
        pstItemArr = VA_PTR_TYPE(MT_CMD_ITEM_ARR_S, pstItem);
        return pstItemArr->u32Num;
    }

    return 0;
}

void MT_FillItemToArray(MT_CMD_ITEM_S *pstArrItem, MT_CMD_ITEM_S *pstItem)
{
    const MT_CMD_ITEM_S *pstCurrItem = pstItem;

	if ( (ULONG)pstItem < MT_ITEM_SPEC_BUTT )
	{
	    pstCurrItem = MT_GetSpecItem((ULONG)pstItem);
        if ( pstCurrItem == NULL )
        {
            return;
        }
	}

    *pstArrItem = *pstCurrItem;
    pstArrItem->u8Flg |= MT_ITEM_FLG_ARRAY;
}

int MT_AddItemToArr(MT_CMD_ITEM_ARR_S *pstItemArr, MT_CMD_ITEM_S *pstItem)
{
    if ( pstItemArr->u32Num == 0 )
    {
        MT_FillItemToArray(&pstItemArr->stItem, pstItem);
    }
    else
    {
		if ( pstItemArr->u32Num > MT_ITEM_MAX_ARR )
		{
			VA_LOG_ERR("It's too many cmd items for array merge");
		    return -E2BIG;
		}

        pstItemArr->apstItem[pstItemArr->u32Num - 1] = pstItem;
    }

    pstItemArr->u32Num++;
    return 0;
}

int MT_MergeItemsToArr(MT_CMD_ITEM_ARR_S *pstItemArr, MT_CMD_ITEM_S *pstItem, ...)
{
    MT_CMD_ITEM_S *pstTmpItem;
    va_list args;
    U32 nItemCnt = 0;

    pstItemArr->u32Num = 0;
    MT_FillItemToArray(&pstItemArr->stItem, pstItem); // fill first item

    va_start(args, pstItem);

    pstTmpItem = VA_PTR_TYPE(MT_CMD_ITEM_S, va_arg(args, void *));
    for ( ; pstTmpItem != NULL; nItemCnt++ )
    {
		if ( nItemCnt >= MT_ITEM_MAX_ARR )
		{
			VA_LOG_ERR("It's too many cmd items for array merge");
		    return -E2BIG;
		}

        pstItemArr->apstItem[nItemCnt] = pstTmpItem;
        pstTmpItem = VA_PTR_TYPE(MT_CMD_ITEM_S, va_arg(args, void *));
    }

    va_end(args);

    pstItemArr->u32Num = nItemCnt + 1;
    return 0;
}

#if 0
#endif

extern int MT_RegCaFile(void);
extern int MT_RegUaFile(void);
extern void MT_UnRegUaFile(void);
extern void MT_UnRegCaFile(void);

void MT_Exit(void)
{
    struct list_head *pstNode;
    struct list_head *pstNextNode;
    MT_CB_S *pstMtCb;

    MT_UnRegUaFile();
    MT_UnRegCaFile();

    list_for_each_safe(pstNode, pstNextNode, &gstMtMgr.stCbHead)
    {
        pstMtCb = VA_PTR_TYPE(MT_CB_S, pstNode);
        MT_UnRegMt(pstMtCb);
    }

    MT_UnRegView(0);
}

int MT_Init(VA_MOD_S *pstMod)
{
    INT iRet;

    mutex_init(&gstMtMgr.stLock);
    mutex_init(&gstMtMgr.stUaLock);
    init_waitqueue_head(&gstMtMgr.stWaitHead);

    VA_InitHlistTbl(gstMtMgr.astViewHashTbl, VA_ARR_SIZE(gstMtMgr.astViewHashTbl));
    VA_InitHlistTbl(gstMtMgr.astHashTbl, VA_ARR_SIZE(gstMtMgr.astHashTbl));

    iRet = MT_RegView(0, "root", "root");
    if ( iRet < 0 )
    {
        return iRet;
    }

    MT_RegCaFile();
    MT_RegUaFile();
    return 0;
}

VA_MOD_INIT(mt, MT_Init, MT_Exit, VA_INIT_LEVEL_MT)

