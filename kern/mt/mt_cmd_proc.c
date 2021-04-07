#include <linux/ctype.h>
#include <linux/inet.h>
#include "va_kern_pub.h"
#include "mt_kern_util.h"
#include "mt_mgr.h"
#include "mt_cmd_proc.h"

void MT_OutputCmdline(char *szTmpBuf, int nOff, char **pszBuf, U32 *pu32LeftLen)
{
    if ( *pu32LeftLen <= 0 )
    {
        return;
    }

    strncpy(*pszBuf, szTmpBuf, *pu32LeftLen);
    if ( *pu32LeftLen > nOff )
    {
        *pu32LeftLen -= nOff;
        *pszBuf      += nOff;
    }
    else
    {
        *pu32LeftLen = 0;
    }
}

void MT_PrintEntryName(struct list_head *pstHead, MT_CMD_ENTRY_S *pstEntry, char *szTmpBuf,
                       int nOff, U32 u32TmpBufLen, char **pszBuf, U32 *pu32LeftLen)
{
    U32 u32Indx;
    int nTmpOff = nOff;

    nTmpOff += VA_SnprintEx(szTmpBuf + nOff, u32TmpBufLen - nOff, " %s", pstEntry->szName);
    if ( pstHead != pstEntry->stNode.next )
    {
        MT_PrintEntryName(pstHead, list_next_entry(pstEntry, stNode), szTmpBuf, nTmpOff, u32TmpBufLen, pszBuf, pu32LeftLen);
    }
    else
    {
        nTmpOff += VA_SnprintEx(szTmpBuf + nTmpOff, u32TmpBufLen - nTmpOff, "\n");
        MT_OutputCmdline(szTmpBuf, nTmpOff, pszBuf, pu32LeftLen);
    }

    if ( (pstEntry->u8Flg & MT_ITEM_FLG_ARRAY) == 0)
    {
        return;
    }

    for (u32Indx = 0; pstEntry->ppstEntryArr[u32Indx] != NULL; u32Indx++)
    {
        nTmpOff += VA_SnprintEx(szTmpBuf + nOff, u32TmpBufLen - nOff, " %s", pstEntry->ppstEntryArr[u32Indx]->szName);
        if ( pstHead != pstEntry->stNode.next )
        {
            MT_PrintEntryName(pstHead, list_next_entry(pstEntry, stNode), szTmpBuf, nTmpOff, u32TmpBufLen, pszBuf, pu32LeftLen);
        }
        else
        {
            nTmpOff += VA_SnprintEx(szTmpBuf + nTmpOff, u32TmpBufLen - nTmpOff, "\n");
            MT_OutputCmdline(szTmpBuf, nTmpOff, pszBuf, pu32LeftLen);
        }
    }

    return;
}

int MT_PrintEntryInfo(MT_CMD_ENTRY_S *pstEntry, char acId[MT_ID_MAX_LEN + 1], char *pBuf, int nOff, U32 u32Len)
{
    switch ( pstEntry->u8Type )
    {
        case MT_ITEM_TYPE_TEXT :
            nOff += VA_SnprintEx(pBuf + nOff, u32Len - nOff, "name:%-16s id:%-6s prompt: %s\n",
                                 pstEntry->szName, acId, pstEntry->szDesc);
            break;
        case MT_ITEM_TYPE_STR :
        case MT_ITEM_TYPE_INT :
            nOff += VA_SnprintEx(pBuf + nOff, u32Len - nOff, "name:%-16s id:%-6s prompt: [%llu-%llu] %s\n",
                                 pstEntry->szName, acId, pstEntry->u64MinVal, pstEntry->u64MaxVal, pstEntry->szDesc);
            break;
        case MT_ITEM_TYPE_CHAN :
            nOff += VA_SnprintEx(pBuf + nOff, u32Len - nOff, "name:%-16s id:%-6s prompt: [type/port/chan] %s\n",
                                 pstEntry->szName, acId, pstEntry->szDesc);
            break;
        case MT_ITEM_TYPE_HEX :
        case MT_ITEM_TYPE_PTR :
            nOff += VA_SnprintEx(pBuf + nOff, u32Len - nOff, "name:%-16s id:%-6s prompt: [0x%llx-0x%llx] %s\n",
                                 pstEntry->szName, acId, pstEntry->u64MinVal, pstEntry->u64MaxVal, pstEntry->szDesc);
        case MT_ITEM_TYPE_IP:
            nOff += VA_SnprintEx(pBuf + nOff, u32Len - nOff, "name:%-16s id:%-6s prompt: [x.x.x.x] %s\n",
                                 pstEntry->szName, acId, pstEntry->szDesc);
        default:
            return 0;
    }

    return nOff;
}

int MT_PrintCmdline(MT_CL_S *pstCl, char *pBuf, U32 u32Len)
{
    MT_CMD_ENTRY_S *pstEntry;
    struct list_head *pstNode;
    int  nOff = 0;
    char acId[MT_ID_MAX_LEN + 1] = {0};
    char szTmpBuf[MT_MAX_INPUT_LEN + 1];
    char *pTmpBuf;
    U32 u32LeftLen = u32Len;
    U32 u32Indx;

    nOff += VA_SnprintEx(pBuf + nOff, u32Len - nOff, "cmdline format:\n");
    u32LeftLen -= nOff;

    szTmpBuf[MT_MAX_INPUT_LEN] = 0;
    pstEntry = list_first_entry(&pstCl->stHead, MT_CMD_ENTRY_S, stNode);

    pTmpBuf  = pBuf + nOff;
    MT_PrintEntryName(&pstCl->stHead, pstEntry, szTmpBuf, 0, MT_MAX_INPUT_LEN, &pTmpBuf, &u32LeftLen);

    nOff = u32Len - u32LeftLen;
    nOff += VA_SnprintEx(pBuf + nOff, u32Len - nOff, "\n");

    list_for_each(pstNode, &pstCl->stHead)
    {
        pstEntry = container_of(pstNode, MT_CMD_ENTRY_S, stNode);
        memcpy(acId, pstEntry->as8Id, MT_ID_MAX_LEN);
        nOff = MT_PrintEntryInfo(pstEntry, acId, pBuf, nOff, u32Len);
        if ( nOff >= (u32Len - 2) )
        {
            return -E2BIG;
        }

        if ( (pstEntry->u8Flg & MT_ITEM_FLG_ARRAY) == 0)
        {
            continue;
        }

        if ( nOff >= (u32Len - 2) )
        {
            return -E2BIG;
        }

        for (u32Indx = 0; pstEntry->ppstEntryArr[u32Indx] != NULL; u32Indx++)
        {
            nOff = MT_PrintEntryInfo(pstEntry->ppstEntryArr[u32Indx], acId, pBuf, nOff, u32Len);
        }
    }

    if ( nOff >= (u32Len - 2) )
    {
        return -E2BIG;
    }

    return nOff;
}

ssize_t MT_PrintCmdlineFmt(MT_FILE_CB_S *pstFile, char *pBuf, U32 u32Len)
{
    MT_CL_S *pstCl;
    char *pTmpBuf  = pBuf;
    U32 u32LeftLen = u32Len;
    int iRet;

	MT_GlobalLock();

	if ( MT_ChkFileCb(pstFile) < 0 )
	{
		MT_GlobalUnLock();
	    return 0;
	}

    list_for_each_entry(pstCl, &pstFile->stClHead, stFileNode)
    {
        iRet = MT_PrintCmdline(pstCl, pTmpBuf, u32LeftLen);
        if ( iRet < 0 )
        {
			MT_GlobalUnLock();
            return iRet;
        }

        u32LeftLen -= iRet;
        pTmpBuf    += iRet;
    }

	MT_GlobalUnLock();
    return u32Len - u32LeftLen;
}

VOID MT_Print(ULONG ulExecId, const char *szFmt, ...)
{
    MT_VAL_HEAD_S *pstValHead = (MT_VAL_HEAD_S *)ulExecId;
    va_list args;
    char *pBuf;
    int  iRet;

    pBuf = (char *)VA_Malloc(PAGE_SIZE);
    if ( pBuf == NULL )
    {
        return;
    }

    va_start(args, szFmt);
    iRet = vsnprintf(pBuf, PAGE_SIZE, szFmt, args);
    va_end(args);

    if ( iRet > 0 )
    {
        MT_KnlWirte(pstValHead->pstFile, pBuf, iRet);
    }

    VA_Free(pBuf);
    return;
}

ssize_t MT_KnlWirteFromUsr(ULONG ulExecId,  const char __user *pBuf, size_t count)
{
    MT_VAL_HEAD_S *pstValHead = (MT_VAL_HEAD_S *)ulExecId;
    loff_t  pos = 0;
    ssize_t res;

    /* The cast to a user pointer is valid due to the set_fs() */
    res = vfs_write(pstValHead->pstFile, pBuf, count, &pos);

    return res;
}

#if 0
#endif

static MT_VAL_PRIV_S *MT_FindValItem(ULONG ulExecId, const S8 *as8Id)
{
    MT_VAL_HEAD_S *pstValHead = (MT_VAL_HEAD_S *)ulExecId;
    MT_VAL_PRIV_S *pstVal;
    struct list_head *pstNode;

    list_for_each(pstNode, &pstValHead->stHead)
    {
        pstVal = container_of(pstNode, MT_VAL_PRIV_S, stNode);

		if ( as8Id[0] == 0 && pstVal->as8Id[0] == 0 ) // const int value
		{
		    if ( memcmp(pstVal->as8Id, as8Id, MT_ID_MAX_LEN) == 0 )
		    {
		        return pstVal;
		    }
		}
        else if ( strncmp(pstVal->as8Id, as8Id, MT_ID_MAX_LEN) == 0 )
        {
            return pstVal;
        }
    }

    return NULL;
}

static MT_VAL_PRIV_S *MT_FindNextValItem(ULONG ulExecId, const S8 *as8Id, MT_VAL_PRIV_S *pstCurrVal)
{
    MT_VAL_HEAD_S *pstValHead = (MT_VAL_HEAD_S *)ulExecId;
    MT_VAL_PRIV_S *pstVal;

    if ( list_empty(&pstValHead->stHead) )
    {
        return NULL;
    }

    if ( pstCurrVal == NULL )
    {
        pstVal = list_first_entry(&pstValHead->stHead, MT_VAL_PRIV_S, stNode);
    }
    else
    {
        pstVal = list_next_entry(pstCurrVal, stNode);
    }

    list_for_each_entry_from(pstVal, &pstValHead->stHead, stNode)
    {
		if ( as8Id[0] == 0 && pstVal->as8Id[0] == 0 ) // const int value
		{
		    if ( memcmp(pstVal->as8Id, as8Id, MT_ID_MAX_LEN) == 0 )
		    {
		        return pstVal;
		    }
		}
        else if ( strncmp(pstVal->as8Id, as8Id, MT_ID_MAX_LEN) == 0 )
        {
            return pstVal;
        }
    }

    return NULL;
}

int MT_GetStrVal(ULONG ulExecId, const S8 *as8Id, const char **ppStr)
{
    MT_VAL_PRIV_S *pstVal;

    if ( ppStr == NULL || ulExecId == 0 )
    {
        return -EINVAL;
    }

    pstVal = MT_FindValItem(ulExecId, as8Id);
    if ( pstVal == NULL )
    {
        return -ENOENT;
    }

    *ppStr = pstVal->szStr;
    return 0;
}

int MT_GetTextVal(ULONG ulExecId, const S8 *as8Id, const char **ppStr)
{
    return MT_GetStrVal(ulExecId, as8Id, ppStr);
}

int MT_GetIntVal(ULONG ulExecId, const S8 *as8Id, ULONG *pulVal)
{
    MT_VAL_PRIV_S *pstVal;
    ULONG ulVal;

    if ( pulVal == NULL || ulExecId == 0 )
    {
        return -EINVAL;
    }

    pstVal = MT_FindValItem(ulExecId, as8Id);
    if ( pstVal == NULL )
    {
        return -ENOENT;
    }

    ulVal   = simple_strtoul(pstVal->szStr, NULL, 0);
    *pulVal = ulVal;
    return 0;
}

int MT_GetU64Val(ULONG ulExecId, const S8 *as8Id, U64 *pu64Val)
{
    MT_VAL_PRIV_S *pstVal;
    ULLONG ullVal;

    if ( pu64Val == NULL || ulExecId == 0 )
    {
        return -EINVAL;
    }

    pstVal = MT_FindValItem(ulExecId, as8Id);
    if ( pstVal == NULL )
    {
        return -ENOENT;
    }

    ullVal   = simple_strtoull(pstVal->szStr, NULL, 0);
    *pu64Val = ullVal;
    return 0;
}

static int __MT_GetChanIdVal(MT_VAL_PRIV_S *pstVal, CHAN_ID_S *pstChanId)
{
    U32 u32ChanType, u32PortId, u32ChanId;

    if ( sscanf(pstVal->szStr, "%u/%u/%u", &u32ChanType, &u32PortId, &u32ChanId) != 3 )
    {
        return -EINVAL;
    }

    pstChanId->u16ChanId = u32ChanId;
    pstChanId->u16PortId = u32PortId;
    pstChanId->u16SlotId = 0;
    pstChanId->u16ChanType = u32ChanType;

    return 0;
}

int MT_GetChanIdVal(ULONG ulExecId, const S8 *as8Id, CHAN_ID_S *pstChanId)
{
    MT_VAL_PRIV_S *pstVal;

    if ( pstChanId == NULL || ulExecId == 0 )
    {
        return -EINVAL;
    }

    pstVal = MT_FindValItem(ulExecId, as8Id);
    if ( pstVal == NULL )
    {
        return -ENOENT;
    }

    return __MT_GetChanIdVal(pstVal, pstChanId);
}

int MT_GetIpv4Val(ULONG ulExecId, const S8 *as8Id, BE32 *pbe32Ip)
{
    MT_VAL_PRIV_S *pstVal;
    BE32 be32Val;

    if ( pbe32Ip == NULL || ulExecId == 0 )
    {
        return -EINVAL;
    }

    pstVal = MT_FindValItem(ulExecId, as8Id);
    if ( pstVal == NULL )
    {
        return -ENOENT;
    }

    be32Val  = in_aton(pstVal->szStr);
    *pbe32Ip = be32Val;

    return 0;
}

int MT_GetAllValById(ULONG ulExecId, const S8 *as8Id, MT_VAL_U *pstValTbl, U32 *pu32ItemNum)
{
    MT_VAL_PRIV_S *pstVal = NULL;
    U32 u32ItemMaxNum = *pu32ItemNum;
    U32 u32ItemNum    = 0;
    int  iRet;
    U32  i;

    if ( u32ItemMaxNum < 1 )
    {
        return -EINVAL;
    }

    VA_MEM_ZERO(pstValTbl, sizeof(MT_VAL_U) * u32ItemMaxNum);
    *pu32ItemNum = 0;

    for (i = 0; i < u32ItemMaxNum; i++)
    {
        pstVal = MT_FindNextValItem(ulExecId, as8Id, pstVal);
        if ( pstVal == NULL )
        {
            break;
        }

        switch ( pstVal->u8ItemType )
        {
            case MT_ITEM_TYPE_TEXT :
            case MT_ITEM_TYPE_STR :
                strncpy(pstValTbl[u32ItemNum].s8Str, pstVal->szStr, MT_NAME_MAX_LEN);
                break;
            case MT_ITEM_TYPE_INT :
            case MT_ITEM_TYPE_HEX :
            case MT_ITEM_TYPE_PTR :
                pstValTbl[u32ItemNum].u64Val = simple_strtoull(pstVal->szStr, NULL, 0);
                break;
            case MT_ITEM_TYPE_CHAN :
                iRet = __MT_GetChanIdVal(pstVal, &pstValTbl[u32ItemNum].stChanId);
                if ( iRet < 0 )
                {
                    return iRet;
                }
                break;
            case MT_ITEM_TYPE_IP :
                pstValTbl[u32ItemNum].be32Ip = in_aton(pstVal->szStr);
                break;
            default:
                return -EINVAL;
        }

        u32ItemNum++;
    }

    *pu32ItemNum = u32ItemNum;
    return 0;
}

int MT_GetToken(char *szCmdLine, char *aszItem[MT_ITEM_MAX_NUM + 1])
{
    U32 u32Pos = 0;
    U32 u32ItemNo = 0;

    while ( szCmdLine[u32Pos] != 0 )
    {
        while (szCmdLine[u32Pos] != 0 && isspace(szCmdLine[u32Pos])) // skip space
        {
            u32Pos++;
        }

        if ( szCmdLine[u32Pos] == 0 )
        {
            break;
        }

        if ( u32ItemNo > MT_ITEM_MAX_NUM )
        {
            return -E2BIG;
        }

        aszItem[u32ItemNo] = szCmdLine + u32Pos;
        u32ItemNo++;

        while (szCmdLine[u32Pos] != 0 && (!isspace(szCmdLine[u32Pos])))
        {
            u32Pos++;
        }

        if ( szCmdLine[u32Pos] == 0 )
        {
            break;
        }

        szCmdLine[u32Pos++] = 0;
    }

    return 0;
}

int MT_ChkText(MT_CMD_ENTRY_S *pstEntry, const char *szItem)
{
	U32 u32Len = strlen(szItem);

	if ( u32Len > MT_NAME_MAX_LEN )
	{
	    return -EINVAL;
	}

    if ( strncasecmp(pstEntry->szName, szItem, u32Len) == 0 )
    {
        return 0;
    }

    return -EINVAL;
}

int MT_ChkStr(MT_CMD_ENTRY_S *pstEntry, const char *szItem)
{
    U32 u32Len = strlen(szItem);

    if ( u32Len >= pstEntry->u64MinVal &&
         u32Len <= pstEntry->u64MaxVal &&
         u32Len <  MT_STR_MAX_LEN )
    {
        return 0;
    }

    return -EINVAL;
}

int MT_ChkInt(MT_CMD_ENTRY_S *pstEntry, const char *szItem)
{
    U32 u32Len  = strlen(szItem);
    char *szTail;
    ULONG ulVal;

    if (u32Len == 0 || u32Len > MT_STR_MAX_LEN)
    {
        return -EINVAL;
    }

    ulVal = simple_strtoul(szItem, &szTail, 0);
    if (szTail == szItem)
    {
        return -EINVAL;
    }

    if (*szTail != '\0')
    {
        return -EINVAL;
    }

    if ( (U64)ulVal < pstEntry->u64MinVal || (U64)ulVal > pstEntry->u64MaxVal )
    {
        return -EINVAL;
    }

    return 0;
}

int MT_ChkChanId(MT_CMD_ENTRY_S *pstEntry, const char *szItem)
{
    U32 u32ChanType, u32PortId, u32ChanId;
    U32 u32SlashCnt = 0;
    U32 u32Len = strlen(szItem);
    U32 i;

    if (u32Len == 0 || u32Len > MT_STR_MAX_LEN)
    {
        return -EINVAL;
    }

    for (i = 0; i < u32Len; )
    {
        if ( !(isdigit(szItem[i])) )
        {
            return -EINVAL;
        }

        i++;

        while ( i < u32Len )
        {
            if ( !(isdigit(szItem[i]) || szItem[i] == '/') )
            {
                return -EINVAL;
            }

            if ( szItem[i] == '/' )
            {
                i++;
                u32SlashCnt++;
                break;
            }

            i++;
        }
    }

    if ( u32SlashCnt != 2 )
    {
        return -EINVAL;
    }

    if ( sscanf(szItem, "%u/%u/%u", &u32ChanType, &u32PortId, &u32ChanId) != 3 )
    {
        return -EINVAL;
    }

    return 0;
}

int MT_ChkIp(MT_CMD_ENTRY_S *pstEntry, const char *szItem)
{
    U32 u32DotCnt = 0;
    U32 u32Len = strlen(szItem);
    U32 a[4];
    U32 i;

    if ( u32Len > VA_IP_LEN )
    {
        return -EINVAL;
    }

    for (i = 0; i < u32Len; )
    {
        if ( !(isdigit(szItem[i])) )
        {
            return -EINVAL;
        }

        i++;

        while ( i < u32Len )
        {
            if ( !(isdigit(szItem[i]) || szItem[i] == '.') )
            {
                return -EINVAL;
            }

            if ( szItem[i] == '.' )
            {
                i++;
                u32DotCnt++;
                break;
            }

            i++;
        }
    }

    if ( u32DotCnt != 3 )
    {
        return -EINVAL;
    }

    if ( sscanf(szItem, "%u.%u.%u.%u", a, a + 1, a + 2, a + 3) != 4 )
    {
        return -EINVAL;
    }

    for (i = 0; i < VA_ARR_SIZE(a); i++)
    {
        if ( a[i] > 255 )
        {
            return -EINVAL;
        }
    }

    return 0;
}

int MT_MatchEntry(MT_CMD_ENTRY_S *pstEntry, const char *szItem)
{
    int iRet;

    switch ( pstEntry->u8Type )
    {
        case MT_ITEM_TYPE_TEXT :
            iRet = MT_ChkText(pstEntry, szItem);
            break;
        case MT_ITEM_TYPE_STR :
            iRet = MT_ChkStr(pstEntry, szItem);
            break;
        case MT_ITEM_TYPE_HEX :
        case MT_ITEM_TYPE_INT :
            iRet = MT_ChkInt(pstEntry, szItem);
            break;
        case MT_ITEM_TYPE_CHAN :
            iRet = MT_ChkChanId(pstEntry, szItem);
            break;
        case MT_ITEM_TYPE_PTR :
            iRet = MT_ChkInt(pstEntry, szItem);
            break;
        case MT_ITEM_TYPE_IP:
            iRet = MT_ChkIp(pstEntry, szItem);
            break;
        default:
            return -EINVAL;
    }

    return iRet;
}

int MT_GetValList(MT_VAL_HEAD_S *pstValHead, MT_CL_S *pstCl, char *aszItem[MT_ITEM_MAX_NUM + 1])
{
    struct list_head *pstNode;
    MT_CMD_ENTRY_S *pstEntry;
    MT_VAL_PRIV_S *pstVal;
    U32 i = 0, j;
    int iRet;

    list_for_each(pstNode, &pstCl->stHead)
    {
        pstEntry = container_of(pstNode, MT_CMD_ENTRY_S, stNode);
        if ( aszItem[i] == NULL )
        {
            return -EINVAL;
        }

        iRet = MT_MatchEntry(pstEntry, aszItem[i]);
        if ( iRet < 0 )
        {
            if ( pstEntry->u8Flg & MT_ITEM_FLG_ARRAY )
            {
                for ( j = 0; pstEntry->ppstEntryArr[j] != NULL; j++)
                {
                    iRet = MT_MatchEntry(pstEntry->ppstEntryArr[j], aszItem[i]);
                    if ( iRet == 0 )
                    {
                        pstEntry = pstEntry->ppstEntryArr[j];
                        break;
                    }
                }
            }
        }

        if ( iRet < 0 )
        {
            MT_Print((ULONG)pstValHead, "\r\ninvalid parameter, begin with %u --> %s", i, aszItem[i]);
            return -EINVAL;
        }

        pstVal = VA_Zmalloc(sizeof(MT_VAL_PRIV_S));
        if ( pstVal == NULL )
        {
            return -ENOMEM;
        }

        pstVal->u8ItemType = pstEntry->u8Type;
        memcpy(pstVal->as8Id, pstEntry->as8Id, MT_ID_MAX_LEN);
        strncpy(pstVal->szStr, aszItem[i], MT_STR_MAX_LEN);
        pstVal->szStr[MT_STR_MAX_LEN] = 0;
        list_add_tail(&pstVal->stNode, &pstValHead->stHead);

        i++;
    }

    if ( aszItem[i] != NULL )
    {
		MT_Print((ULONG)pstValHead, "\r\ntoo many parameters, begin with %u --> %s", i, aszItem[i]);
        return -EINVAL;
    }

    return 0;
}

int MT_InitValHead(MT_VAL_HEAD_S *pstValHead, struct file *pstOutFile)
{
    INIT_LIST_HEAD(&pstValHead->stHead);
    pstValHead->pstFile = pstOutFile;  // get output file description

    return 0;
}

VOID MT_ClearValHead(MT_VAL_HEAD_S *pstValHead)
{
    struct list_head *pstNode;
    struct list_head *pstNextNode;
    MT_VAL_PRIV_S *pstVal;

    list_for_each_safe(pstNode, pstNextNode, &pstValHead->stHead)
    {
        pstVal = container_of(pstNode, MT_VAL_PRIV_S, stNode);
        list_del(pstNode);
        VA_Free(pstVal);
    }

    return;
}

LONG MT_ExecUserCmd(MT_CL_S *pstCl, struct file *pstOutFile, char *szCmdLine)
{
    MT_VAL_HEAD_S stValHead;
    char *aszItem[MT_ITEM_MAX_NUM + 1];
    int iRet;

    iRet = MT_InitValHead(&stValHead, pstOutFile);
    if ( iRet < 0 )
    {
        return iRet;
    }

    memset(aszItem, 0, sizeof(aszItem));

    MT_GetToken(szCmdLine, aszItem);
    if ( aszItem[0] == NULL )
    {
        return -ENOENT;
    }

    if ( MT_GetValList(&stValHead, pstCl, aszItem) < 0 )
    {
        MT_ClearValHead(&stValHead);
        return -EINVAL;
    }

    stValHead.pPrivCb = pstCl->pPrivCb;
    pstCl->pfnCallBack((ULONG)(&stValHead)); // exec cmd line

    MT_ClearValHead(&stValHead);
    return 0;
}

VOID MT_ParseCmdLine(MT_FILE_CB_S *pstFile, struct file *pstOutFile, char *szCmdLine)
{
    MT_VAL_HEAD_S stValHead;
    MT_CL_S *pstCl;
    char *aszItem[MT_ITEM_MAX_NUM + 1];
    int iRet;

    iRet = MT_InitValHead(&stValHead, pstOutFile);
    if ( iRet < 0 )
    {
        return;
    }

    memset(aszItem, 0, sizeof(aszItem));

    MT_GetToken(szCmdLine, aszItem);
    if ( aszItem[0] == NULL )
    {
        return;
    }

    MT_GlobalLock();

	if ( MT_ChkFileCb(pstFile) < 0 )
	{
		MT_Print((ULONG)(&stValHead), "This module was removed\r\n");
	    goto out;
	}

    list_for_each_entry(pstCl, &pstFile->stClHead, stFileNode)
    {
        if ( MT_GetValList(&stValHead, pstCl, aszItem) < 0 )
        {
            MT_ClearValHead(&stValHead);
            continue;
        }

        stValHead.pPrivCb = pstCl->pPrivCb;
        pstCl->pfnCallBack((ULONG)(&stValHead)); // exec cmd line
        break;
    }

out:
    MT_GlobalUnLock();
    MT_Print((ULONG)(&stValHead), "\r\n");
    MT_ClearValHead(&stValHead);
}

VOID *MT_GetPrivCb(ULONG ulExecId)
{
    MT_VAL_HEAD_S *pstValHead = VA_PTR_TYPE(MT_VAL_HEAD_S , ulExecId);
    return pstValHead->pPrivCb;
}
