#ifndef __MT_MAIN_H__
#define __MT_MAIN_H__

#include "va_usr_pub.h"
#include "mt_input.h"
#include "mt_ca.h"
#include "mt_parse.h"

using namespace google;

#define MT_IO_FILE                      VA_FS_PATH "mt_ca"
#define MT_CL_INFO_MAX_LEN              (16 * VA_KB)

class CMt
{
public:
    CMt();
    ~CMt();

    bool Init();
    void Run();

    typedef bool (*MT_EXEC_CMD_PF)(CMt *pMt, CMtResult &Result);

private:
    bool Renew();
    bool PollCmdStatus(std::string &strLine);
    void PrintAmbiguousInfo(CMtResult Result);
    void PrintErrInfo(CMtResult Result);
    bool GetToken(std::list<MT_TOKEN_S> &listToken, std::string &strLine);
    void PreParse(std::list<MT_TOKEN_S> &listToken, CMtResult &Result);
    bool CheckInt(std::string &strToken, linked_ptr<CMtEntry> &pEntry);
    bool CheckPartIp(std::string &strToken, linked_ptr<CMtEntry> &pEntry);
    bool CheckFullIp(std::string &strToken, linked_ptr<CMtEntry> &pEntry);
    bool CheckPartChan(std::string &strToken, linked_ptr<CMtEntry> &pEntry);
    bool CheckFullChan(std::string &strToken, linked_ptr<CMtEntry> &pEntry);
    void FixTextMatch(std::list< linked_ptr<CMtBaseEntry> > &listResultEntry, MT_TOKEN_S &stToken);
    void FixMatchResult(std::list< linked_ptr<CMtBaseEntry> > &listResultEntry);
    bool CheckResultCoherent(std::list< linked_ptr<CMtBaseEntry> > &listResultEntry);
    CMtEndEntry *GetEndEntry(std::list< linked_ptr<CMtBaseEntry> > &listResultEntry);
    void FilterSameResult(std::list< linked_ptr<CMtBaseEntry> > &listResultEntry);
    void MatchPartResult(std::list < linked_ptr<CMtBaseEntry> > *pChildList, MT_TOKEN_S &stToken,
                         std::list< linked_ptr<CMtBaseEntry> > &listResultEntry);
    void MatchResult(std::list < linked_ptr<CMtBaseEntry> > *pChildList, MT_TOKEN_S &stToken,
                     std::list< linked_ptr<CMtBaseEntry> > &listResultEntry);
    bool MatchCmdLine(std::list< linked_ptr<CMtBaseEntry> > &listResultEntry, std::list<MT_TOKEN_S> &listToken, CMtResult &Result);
    bool ParseHelp(std::list<MT_TOKEN_S> &listToken, CMtResult &Result);
    void AutoExpandInfo(linked_ptr<CMtEntry> &pEntry);
    bool AutoExpand(std::list<MT_TOKEN_S> &listToken, CMtResult &Result);
    bool ExactMatchChkResult(MT_TOKEN_S &stToken, CMtBaseEntry *pstBaseEntry);
    bool ExactMatch(std::list<MT_TOKEN_S> &listToken, CMtResult &Result);
    void ParseInputCmdLine(std::string &strLine);
    bool GetView(CMtView * &pstView);
    CMtView *GetView(U64 u64ViewId);
    bool FindEntryFrmCache(linked_ptr<CMtEntry> &pEntry, MT_CMD_USR_ITEM_S *pstUsrItem);
    void AddEntryToCache(linked_ptr<CMtEntry> &pEntry);
    bool EqualEntry(linked_ptr<CMtEntry> &pEntry, MT_CMD_USR_ITEM_S *pstUsrItem);
    bool CheckEntry(linked_ptr<CMtEntry> &pEntry, MT_CMD_USR_ITEM_S *pstUsrItem);
    linked_ptr<CMtBaseEntry> InitEndEntry(linked_ptr<CMtEntry> &pEntry, MT_EXEC_CMD_PF &pfnExecCmd, U64 &u64CmdId);
    linked_ptr<CMtBaseEntry> InitLinkEntry(linked_ptr<CMtEntry> &pEntry);
    linked_ptr<CMtBaseEntry> AllocEntry(MT_CMD_USR_ITEM_S *pstUsrItem, bool bLast,  MT_EXEC_CMD_PF &pfnExecCmd, U64 &u64CmdId);
    bool CheckEntrys(std::list < linked_ptr<CMtBaseEntry> > * &pChildList, MT_CMD_USR_ITEM_S *pstUsrItem, bool bLast);
    void PrintConflictEntry(std::list < linked_ptr<CMtBaseEntry> > &listMatchEntry, MT_CMD_USR_ITEM_S *pstSelfItem);
    void PrintConflictCl(std::string strPrefix, CMtBaseEntry *pBaseEntry);
    CMtEntryLink *ToBaseEntry(std::list < linked_ptr<CMtBaseEntry> > *pParent, std::list < linked_ptr<CMtBaseEntry> > *pSelf);
    void DumpEntry(CMtBaseEntry *pBaseEntry);
    bool RegEntry(std::list < linked_ptr<CMtBaseEntry> > * &pChildList, MT_CMD_USR_ITEM_S *pstUsrItem, bool bLast,  MT_EXEC_CMD_PF &pfnExecCmd, U64 &u64CmdId);
    U32  GetItemArrPos(MT_CL_INFO_S *pstClInfo, U32 nPos);
    bool RegEntry(std::list < linked_ptr<CMtBaseEntry> > *pChildList, U32 nPos, MT_CL_INFO_S *pstClInfo, MT_EXEC_CMD_PF pfnExecCmd);
    bool RegCmdLine(MT_CL_INFO_S *pstClInfo, MT_EXEC_CMD_PF pfnExecCmd);

    VOID FillUsrItem(IN MT_CMD_ITEM_S *pstItem, OUT MT_CMD_USR_ITEM_S *pstUsrItem);
    bool InitCmdLine(MT_CL_INFO_S *pstClInfo, MT_CMD_ITEM_S *pstItem, va_list &args);
    VOID RegSelfCmdLine(MT_EXEC_CMD_PF pfnCallBack, U64 u64ViewId, MT_CMD_ITEM_S *pstItem, ...);

    void PrintCmdLine(MT_CL_INFO_S *pstClInfo, const std::string &strReason);
    bool GetCmdLineCnt(MT_CL_NUM_S *pstClNum);
    bool GetCmdIdsInfo(MT_CL_NUM_S &stClNum, MT_CMD_IDS_INFO_S * &pstIdsInfo);
    bool GetClInfo(U64 &u64CmdId, MT_CL_INFO_S *&pstClInfo);
    bool GetAllCmdLine(U64 u64ViewId = 0);
    static bool ExecCmd(CMt *pMt, CMtResult &Result);
    bool SetGlbDebug(ULONG ulDbgSw);
    bool GetClChangeStatus();

    VOID DefTextCmdItem(MT_CMD_ITEM_S &ItemName, const char *szId, const char *szName, const char *szDesc);
    static bool OpenDebug(CMt *pMt, CMtResult &Result);
    static bool CloseDebug(CMt *pMt, CMtResult &Result);
    static bool Exit(CMt *pMt, CMtResult &Result);
    VOID RegMgrCmdLine();

    bool GetViewIdsInfo(U32 u32ClCnt, MT_CMD_IDS_INFO_S * &pstIdsInfo);
    bool GetViewInfo(U64 &u64CmdId, MT_VIEW_INFO_S &stViewInfo);
    bool GetViews(MT_CL_NUM_S &stClNum);
    static bool ViewProc(CMt *pMt, CMtResult &Result);
    static bool QuitView(CMt *pMt, CMtResult &Result);
    bool RegView(MT_VIEW_INFO_S &stViewInfo);
    void ClearViewCmdLines(U64 u64ViewId);
    VOID RegViewSelfCmdLine(U64 u64ViewId);
    VOID RegViewCmdLine(MT_VIEW_INFO_S &stViewInfo);
    VOID __RegViewCmdLine(MT_EXEC_CMD_PF pfnCallBack, U64 u64ViewId, MT_CMD_ITEM_S *pstItem, ...);

private:
    CInputLine m_Input;
    int  m_nStdInFd;
    int  m_nMtFd;
    U64  m_nNextCmdId;

    std::multimap < std::string, linked_ptr<CMtEntry> > m_mapEntryCache;
    linked_ptr<CMtView> m_pCurrView;
    std::map <U64, linked_ptr<CMtView> > m_mapView;
    std::list<std::list < linked_ptr<CMtBaseEntry> > * > m_listTmpClPath;
};

#endif //

