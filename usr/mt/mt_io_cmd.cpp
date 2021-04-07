#include "mt_main.h"
#include <sys/ioctl.h>

bool CMt::GetCmdLineCnt(MT_CL_NUM_S *pstClNum)
{
    LONG lRet;

    lRet = ioctl(m_nMtFd, MT_IOCTL_GET_CL_NUM, pstClNum);
    if ( lRet < 0 )
    {
        return false;
    }

    return true;
}

bool CMt::GetCmdIdsInfo(MT_CL_NUM_S &stClNum, MT_CMD_IDS_INFO_S * &pstIdsInfo)
{
    LONG lRet;

    U32 u32Size = sizeof(MT_CMD_IDS_INFO_S) + sizeof(U64) * stClNum.u32ClCnt;
    pstIdsInfo = (MT_CMD_IDS_INFO_S *)VA_Malloc(u32Size);
    if ( pstIdsInfo == NULL )
    {
        std::cout << "failed to malloc memory\r\n";
        return false;
    }

    pstIdsInfo->u32Num    = stClNum.u32ClCnt;
    pstIdsInfo->u64ViewId = stClNum.u64ViewId;

    lRet = ioctl(m_nMtFd, MT_IOCTL_GET_ALL_CMD_ID, pstIdsInfo);
    if ( lRet < 0 )
    {
        return false;
    }

    return true;
}

bool CMt::GetViewIdsInfo(U32 u32ClCnt, MT_CMD_IDS_INFO_S * &pstIdsInfo)
{
    LONG lRet;

    U32 u32Size = sizeof(MT_CMD_IDS_INFO_S) + sizeof(U64) * u32ClCnt;
    pstIdsInfo = (MT_CMD_IDS_INFO_S *)VA_Malloc(u32Size);
    if ( pstIdsInfo == NULL )
    {
        std::cout << "failed to malloc memory\r\n";
        return false;
    }

    pstIdsInfo->u32Num = u32ClCnt;

    lRet = ioctl(m_nMtFd, MT_IOCTL_GET_ALL_VIEW_ID, pstIdsInfo);
    if ( lRet < 0 )
    {
        return false;
    }

    return true;
}

bool CMt::GetClInfo(U64 &u64CmdId, MT_CL_INFO_S *&pstClInfo)
{
    LONG lRet;

    pstClInfo->u32Size     = MT_CL_INFO_MAX_LEN - sizeof(MT_CL_INFO_S);
    pstClInfo->u32EntryNum = pstClInfo->u32Size/sizeof(MT_CMD_USR_ITEM_S);
    pstClInfo->u64CmdId    = u64CmdId;

    lRet = ioctl(m_nMtFd, MT_IOCTL_GET_CL_INFO, pstClInfo);
    if ( lRet < 0 )
    {
        return false;
    }

    bool bRet = RegCmdLine(pstClInfo, ExecCmd);
    if ( bRet == false )
    {
        PrintCmdLine(pstClInfo, "");
    }

    return bRet;
}

bool CMt::GetViewInfo(U64 &u64CmdId, MT_VIEW_INFO_S &stViewInfo)
{
    LONG lRet;

    stViewInfo.u64ViewId = u64CmdId;

    lRet = ioctl(m_nMtFd, MT_IOCTL_GET_VIEW_INFO, &stViewInfo);
    if ( lRet < 0 )
    {
        return false;
    }

    bool bRet = RegView(stViewInfo);
    return bRet;
}

bool CMt::GetViews(MT_CL_NUM_S &stClNum)
{
    MT_CMD_IDS_INFO_S *pstIdsInfo;
    MT_VIEW_INFO_S stViewInfo;
    U64 u64CmdId;

    if ( stClNum.u32ViewCnt == 0 )
    {
        return true;
    }

    if ( GetViewIdsInfo(stClNum.u32ViewCnt, pstIdsInfo) == false )
    {
        return false;
    }

    for (U32 i = 0; i < stClNum.u32ViewCnt; i++)
    {
        u64CmdId = pstIdsInfo->au64CmdId[i];
        if ( GetViewInfo(u64CmdId, stViewInfo) == false )
        {
            std::cout << "\r\nFailed to register view " << stViewInfo.szName << "\r\n";
            VA_Free(pstIdsInfo);
            return false;
        }
    }

    VA_Free(pstIdsInfo);
    return true;
}

void CMt::PrintCmdLine(MT_CL_INFO_S *pstClInfo, const std::string &strReason)
{
    U32 i;

    std::cout << "\r\nFailed to register cmd line of " << strReason << ":\r\n ";

    for ( i = 0; i < pstClInfo->u32EntryNum; i++)
    {
        std::cout << pstClInfo->astItem[i].szName << " " << pstClInfo->astItem[i].szDesc;
    }

    CMtView *pstView = GetView(pstClInfo->u64ViewId);
    if ( pstView )
    {
        std::cout << "\r\non view " << pstView->m_strPrompt << "\r\n";
    }
    else
    {
        std::cout << "\r\n";
    }
}

bool CMt::GetAllCmdLine(U64 u64ViewId /* = 0 */ )
{
    MT_CMD_IDS_INFO_S *pstIdsInfo;
    MT_CL_INFO_S *pstClInfo;
    MT_CL_NUM_S  stClNum;
    U64 u64CmdId;

    stClNum.u64ViewId = u64ViewId;
    if ( GetCmdLineCnt(&stClNum) == false )
    {
        std::cout << "\r\nFailed to get cmd line cnt!, may be somethin wrong!\r\n";
        return false;
    }

    if ( GetViews(stClNum) == false )
    {
        std::cout << "\r\nFailed to get view!, may retry again\r\n";
        return false;
    }

    if ( GetCmdIdsInfo(stClNum, pstIdsInfo) == false )
    {
        std::cout << "\r\nFailed to get cmd ids!, may retry again\r\n";
        return false;
    }

    pstClInfo = (MT_CL_INFO_S *)VA_Malloc(MT_CL_INFO_MAX_LEN);
    if ( pstClInfo == NULL )
    {
        VA_Free(pstIdsInfo);
        return false;
    }

    for (U32 i = 0; i < stClNum.u32ClCnt; i++)
    {
        u64CmdId = pstIdsInfo->au64CmdId[i];
        if ( GetClInfo(u64CmdId, pstClInfo) == false )
        {
            std::cout << "\r\nFailed to get cmd line!, may retry again\r\n";
            VA_Free(pstClInfo);
            VA_Free(pstIdsInfo);
            ClearViewCmdLines(u64ViewId);
            return false;
        }
    }

    VA_Free(pstClInfo);
    VA_Free(pstIdsInfo);
    return true;
}

bool CMt::ExecCmd(CMt *pMt, CMtResult &Result)
{
    MT_INPUT_CMD_PARAM_S stCmdParam;
    LONG lRet;

    strncpy(stCmdParam.szCmd, Result.m_strLine.c_str(), MT_MAX_INPUT_LEN);
    stCmdParam.u32BufLen = Result.m_strLine.size();
    stCmdParam.u64CmdId  = Result.m_nCmdId;

    lRet = ioctl(pMt->m_nMtFd, MT_IOCTL_EXEC_CL, &stCmdParam);
    if ( lRet < 0 )
    {
        std::cout << "\r\nfailed to execute cmd : " << Result.m_strLine << "\r\n";
        return false;
    }

    return true;
}

bool CMt::SetGlbDebug(ULONG ulDbgSw)
{
    LONG lRet;

    lRet = ioctl(m_nMtFd, MT_IOCTL_SET_GLB_DBG, ulDbgSw);
    if ( lRet < 0 )
    {
        std::cout << "\r\nfailed to global debug switch\r\n";
        return false;
    }

    return true;
}

bool CMt::GetClChangeStatus()
{
    LONG lRet;
    BOOL bVal;

    lRet = ioctl(m_nMtFd, MT_IOCTL_GET_CL_CHANGE_INFO, &bVal);
    if ( lRet < 0 )
    {
        std::cout << "\r\nfailed to global debug switch\r\n";
        return false;
    }

    return bVal == TRUE;
}

