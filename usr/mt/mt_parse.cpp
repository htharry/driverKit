#include "mt_main.h"
#include "mt_util.h"
#include <stdarg.h>
#include <set>

using namespace google;

CMtEntry::CMtEntry(MT_CMD_USR_ITEM_S *pstUsrItem)
{
    u8Type = pstUsrItem->u8Type;
    u8Flg  = pstUsrItem->u8Flg;
    u64MinVal = pstUsrItem->u64MinVal;
    u64MaxVal = pstUsrItem->u64MaxVal;

    memcpy(as8Id, pstUsrItem->as8Id, MT_ID_MAX_LEN);
    strncpy(szName, pstUsrItem->szName, MT_NAME_MAX_LEN);
    strncpy(szDesc, pstUsrItem->szDesc, MT_DESC_MAX_LEN);

    szName[MT_NAME_MAX_LEN] = 0;
    szDesc[MT_DESC_MAX_LEN] = 0;
}

CMtEndEntry::CMtEndEntry()
{
    u64CmdId = 0;
}

CMtView::CMtView()
{
    m_strPrompt = "[SYS]";
    m_bLoad     = false;
}

CMtResult::CMtResult()
{
    m_bNeedExpand = false;
    m_bNeedHelp   = false;
    m_nCmdId = false;
    pstEndEntry = NULL;
    pfnExecCmd  = NULL;
}

void CMt::PrintAmbiguousInfo(CMtResult Result)
{
    CMtView *pstView;
    U32 u32Pos;

    GetView(pstView);

    u32Pos = pstView->m_strPrompt.size() + Result.m_lastMatchTokenIt->u32Pos + 1;
    char szBuf[512];
    snprintf(szBuf, sizeof(szBuf), "\r\n%*s", u32Pos, "^");

    std::cout << szBuf;
    std::cout << "\r\nAmbiguous Parameter";
}

void CMt::PrintErrInfo(CMtResult Result)
{
    CMtView *pstView;
    U32 u32Pos;

    GetView(pstView);

    u32Pos = pstView->m_strPrompt.size() + Result.m_lastMatchTokenIt->u32Pos + 1;
    char szBuf[512];
    snprintf(szBuf, sizeof(szBuf), "\r\n%*s", u32Pos, "^");

    std::cout << szBuf;
    std::cout << "\r\nInvalid Parameter";
}


bool CMt::GetToken(std::list<MT_TOKEN_S> &listToken, std::string &strLine)
{
    std::string::size_type nBeginPos = 0;
    std::string::size_type nEndPos   = std::string::npos;
    MT_TOKEN_S stToken;

    if ( strLine.empty() )
    {
        return false;
    }

    while (nBeginPos != std::string::npos)
    {
        nEndPos = strLine.find(' ', nBeginPos);
        if ( nEndPos != std::string::npos )
        {
            stToken.strToken = strLine.substr(nBeginPos, nEndPos - nBeginPos);
            stToken.u32Pos   = nBeginPos;
            nBeginPos = nEndPos + 1;
        }
        else
        {
            stToken.strToken = strLine.substr(nBeginPos);
            stToken.u32Pos   = nBeginPos;
            nBeginPos = nEndPos;
        }

        if ( stToken.strToken.empty() )
        {
            continue;
        }

        listToken.push_back(stToken);
    }

    if ( listToken.size() == 0 )
    {
        return false;
    }

    return true;
}

void CMt::PreParse(std::list<MT_TOKEN_S> &listToken, CMtResult &Result)
{
    MT_TOKEN_S &stLastToken = listToken.back();
    std::string::size_type nPos = 0;

    nPos = stLastToken.strToken.rfind('?');
    if ( nPos != std::string::npos )
    {
        Result.m_bNeedHelp = true;
        stLastToken.strToken.erase(stLastToken.strToken.end() - 1);
        return;
    }

    nPos = stLastToken.strToken.rfind('\t');
    if ( nPos != std::string::npos )
    {
        Result.m_bNeedExpand  = true;
        stLastToken.strToken.erase(stLastToken.strToken.end() - 1);
        return;
    }
}

bool CMt::CheckInt(std::string &strToken, linked_ptr<CMtEntry> &pEntry)
{
    U32 u32Len  = strToken.size();
    char *szTail;
    ULONG ulVal;

    if (u32Len == 0 || u32Len > MT_STR_MAX_LEN)
    {
        return false;
    }

    ulVal = simple_strtoul(strToken.c_str(), &szTail, 0);
    if (szTail == strToken.c_str())
    {
        return false;
    }

    if (*szTail != '\0')
    {
        return false;
    }

    if ( (U64)ulVal < pEntry->u64MinVal || (U64)ulVal > pEntry->u64MaxVal )
    {
        return false;
    }

    return true;
}

bool CMt::CheckPartIp(std::string &strToken, linked_ptr<CMtEntry> &pEntry)
{
    U32 nDigitCnt = 0;
    U32 nDotCnt = 0;
    U32 nVal = 0;

    if ( strToken.size() > VA_IP_LEN )
    {
        return false;
    }

    for ( U32 i = 0; i < strToken.size(); i++ )
    {
        if ( isdigit(strToken[i]) )
        {
            nVal = nVal  * 10 + strToken[i] - '0';
            nDigitCnt++;
        }
        else if ( strToken[i] == '.' )
        {
            if ( nDigitCnt == 0 || nDigitCnt > 3 || nVal > 255 )
            {
                return false;
            }

            nVal = 0;
            nDotCnt++;
            nDigitCnt = 0;
        }
        else
        {
            return false;
        }
    }

    if ( nDotCnt > 3 || nDigitCnt > 3 || nVal > 255 )
    {
        return false;
    }

    return true;
}

bool CMt::CheckFullIp(std::string &strToken, linked_ptr<CMtEntry> &pEntry)
{
    if ( CheckPartIp(strToken, pEntry) == false )
    {
        return false;
    }

    U32 a[4];

    if ( sscanf(strToken.c_str(), "%u.%u.%u.%u", a, a + 1, a + 2, a + 3) != 4 )
    {
        return false;
    }

    for (U32 i = 0; i < VA_ARR_SIZE(a); i++)
    {
        if ( a[i] > 255 )
        {
            return false;
        }
    }

    return true;
}

bool CMt::CheckPartChan(std::string &strToken, linked_ptr<CMtEntry> &pEntry)
{
    U32 u32SlashCnt = 0;
    U32 nDigitCnt = 0;
    U32 u32Len = strToken.size();
    U32 nVal = 0;
    U32 i;

    if (u32Len == 0 || u32Len > MT_STR_MAX_LEN)
    {
        return false;
    }

    for (i = 0; i < u32Len; i++)
    {
        if ( isdigit(strToken[i]) )
        {
            nDigitCnt++;
            nVal = nVal * 10 + strToken[i] - '0';
        }
        else if ( strToken[i] == '/' )
        {
            if ( nVal > 65535 || nDigitCnt == 0 || nDigitCnt > 3 )
            {
                return false;
            }

            nVal = 0;
            u32SlashCnt++;
        }
        else
        {
            return false;
        }
    }

    if ( u32SlashCnt > 2 || nVal > 65535 || nDigitCnt == 0 || nDigitCnt > 3 )
    {
        return false;
    }

    return true;
}

bool CMt::CheckFullChan(std::string &strToken, linked_ptr<CMtEntry> &pEntry)
{
    if ( CheckPartChan(strToken, pEntry) == false )
    {
        return false;
    }

    U32 a[3];

    if ( sscanf(strToken.c_str(), "%u/%u/%u", a, a + 1, a + 2) != 3 )
    {
        return false;
    }

    return true;
}

void CMt::FixTextMatch(std::list< linked_ptr<CMtBaseEntry> > &listResultEntry, MT_TOKEN_S &stToken)
{
    std::list< linked_ptr<CMtBaseEntry> >::iterator It = listResultEntry.begin();
    bool bNeedFilterPartTextEntry = false;
    linked_ptr<CMtEntry> pEntry;

    // text type match is completely match over part match!
    for ( ; It != listResultEntry.end(); It++ )
    {
        pEntry = (*It)->pEntry;
        if ( pEntry->u8Type != MT_ITEM_TYPE_TEXT )
        {
            continue;
        }

        if ( strcasecmp(pEntry->szName, stToken.strToken.c_str()) == 0 )
        {
            bNeedFilterPartTextEntry = true;
            break;
        }
    }

    if (  bNeedFilterPartTextEntry != true )
    {
        return;
    }

    for ( It = listResultEntry.begin(); It != listResultEntry.end(); )
    {
        pEntry = (*It)->pEntry;

        if ( pEntry->u8Type != MT_ITEM_TYPE_TEXT )
        {
            It++;
            continue;
        }

        if ( strcasecmp(pEntry->szName, stToken.strToken.c_str()) != 0 )
        {
            listResultEntry.erase(It++);
            continue;
        }

        It++;
    }
}

void CMt::FixMatchResult(std::list< linked_ptr<CMtBaseEntry> > &listResultEntry)
{
    std::list< linked_ptr<CMtBaseEntry> >::iterator It = listResultEntry.begin();
    bool bNeedFilterStrEntry = false;

    for ( ; It != listResultEntry.end(); It++ )
    {
        if ( (*It)->pEntry->u8Type != MT_ITEM_TYPE_STR )
        {
            bNeedFilterStrEntry = true;
        }
    }

    if (  bNeedFilterStrEntry != true )
    {
        return;
    }

    // match other type higher than the string
    for ( It = listResultEntry.begin(); It != listResultEntry.end(); )
    {
        if ( (*It)->pEntry->u8Type == MT_ITEM_TYPE_STR )
        {
            listResultEntry.erase(It++);
            continue;
        }

        It++;
    }
}

bool CMt::CheckResultCoherent(std::list< linked_ptr<CMtBaseEntry> > &listResultEntry)
{
    if ( listResultEntry.size() > 2 || listResultEntry.size() == 0 )
    {
        return false;
    }

    std::list< linked_ptr<CMtBaseEntry> >::iterator BeginIt = listResultEntry.begin();
    std::list< linked_ptr<CMtBaseEntry> >::reverse_iterator EndIt = listResultEntry.rbegin();
    if ( (*BeginIt)->pEntry != (*EndIt)->pEntry )
    {
        return false;
    }

    return true;
}

CMtEndEntry * CMt::GetEndEntry(std::list< linked_ptr<CMtBaseEntry> > &listResultEntry)
{
    CMtEndEntry *pstEntry;
    std::list< linked_ptr<CMtBaseEntry> >::iterator It;

    for ( It = listResultEntry.begin(); It != listResultEntry.end(); It++ )
    {
        linked_ptr<CMtBaseEntry> pBaseEntry = *(It);
        if ( pBaseEntry->u32Attr == MT_ENTRY_ATTR_LINK )
        {
            continue;
        }

        pstEntry = dynamic_cast<CMtEndEntry *>(pBaseEntry.get());
        return pstEntry;
    }

    return NULL;
}

void CMt::MatchPartResult(std::list < linked_ptr<CMtBaseEntry> > *pChildList,
                          MT_TOKEN_S &stToken,
                          std::list< linked_ptr<CMtBaseEntry> > &listResultEntry)
{
    listResultEntry.clear();

    std::list < linked_ptr<CMtBaseEntry> >::iterator It = pChildList->begin();
    for ( ; It != pChildList->end(); It++ )
    {
        linked_ptr<CMtEntry> pEntry = (*It)->pEntry;

        if ( stToken.strToken.size() == 0 )
        {
            listResultEntry.push_back(*It);
            continue;
        }

        if ( pEntry->u8Type == MT_ITEM_TYPE_STR )
        {
            if ( stToken.strToken.size() <= pEntry->u64MaxVal )
            {
                listResultEntry.push_back(*It);
                continue;
            }
        }

        if ( pEntry->u8Type == MT_ITEM_TYPE_TEXT )
        {
            if ( strncasecmp(pEntry->szName, stToken.strToken.c_str(), stToken.strToken.size()) == 0 )
            {
                listResultEntry.push_back(*It);
                continue;
            }
        }

        if ( pEntry->u8Type == MT_ITEM_TYPE_INT ||
             pEntry->u8Type == MT_ITEM_TYPE_HEX ||
             pEntry->u8Type == MT_ITEM_TYPE_PTR )
        {
            if ( CheckInt(stToken.strToken, pEntry) )
            {
                listResultEntry.push_back(*It);
                continue;
            }
        }

        if ( pEntry->u8Type == MT_ITEM_TYPE_CHAN )
        {
            if ( CheckPartChan(stToken.strToken, pEntry) )
            {
                listResultEntry.push_back(*It);
                continue;
            }
        }

        if ( pEntry->u8Type == MT_ITEM_TYPE_IP )
        {
            if ( CheckPartIp(stToken.strToken, pEntry) )
            {
                listResultEntry.push_back(*It);
                continue;
            }
        }
    }

    //FixMatchResult(listResultEntry);
}

void CMt::MatchResult(std::list < linked_ptr<CMtBaseEntry> > *pChildList,
                      MT_TOKEN_S &stToken,
                      std::list< linked_ptr<CMtBaseEntry> > &listResultEntry)
{
    listResultEntry.clear();

    if ( stToken.strToken.size() == 0 )
    {
        return;
    }

    std::list < linked_ptr<CMtBaseEntry> >::iterator It = pChildList->begin();

    for ( ; It != pChildList->end(); It++ )
    {
        linked_ptr<CMtEntry> pEntry = (*It)->pEntry;
        if ( pEntry->u8Type == MT_ITEM_TYPE_STR )
        {
            if ( stToken.strToken.size() <= pEntry->u64MaxVal )
            {
                listResultEntry.push_back(*It);
                continue;
            }
        }

        if ( pEntry->u8Type == MT_ITEM_TYPE_TEXT )
        {
            if ( strncasecmp(pEntry->szName, stToken.strToken.c_str(), stToken.strToken.size()) == 0 )
            {
                listResultEntry.push_back(*It);
                continue;
            }
        }

        if ( pEntry->u8Type == MT_ITEM_TYPE_INT ||
             pEntry->u8Type == MT_ITEM_TYPE_HEX ||
             pEntry->u8Type == MT_ITEM_TYPE_PTR )
        {
            if ( CheckInt(stToken.strToken, pEntry) )
            {
                listResultEntry.push_back(*It);
                continue;
            }
        }

        if ( pEntry->u8Type == MT_ITEM_TYPE_CHAN )
        {
            if ( CheckFullChan(stToken.strToken, pEntry) )
            {
                listResultEntry.push_back(*It);
                continue;
            }
        }

        if ( pEntry->u8Type == MT_ITEM_TYPE_IP )
        {
            if ( CheckFullIp(stToken.strToken, pEntry) )
            {
                listResultEntry.push_back(*It);
                continue;
            }
        }
    }

    FixMatchResult(listResultEntry);
    FixTextMatch(listResultEntry, stToken);
}

bool CMt::MatchCmdLine(std::list< linked_ptr<CMtBaseEntry> > &listResultEntry, std::list<MT_TOKEN_S> &listToken, CMtResult &Result)
{
    std::list< linked_ptr<CMtBaseEntry> > listTmpResultEntry;
    CMtView *pstView;
    bool bLast = false;

    if ( GetView(pstView) == false )
    {
        return false;
    }

    std::list < linked_ptr<CMtBaseEntry> > *pChildList = &pstView->m_listRoot;

    std::list<MT_TOKEN_S>::iterator TokenTmpIt;
    std::list<MT_TOKEN_S>::iterator TokenIt = listToken.begin();
    for ( ; TokenIt != listToken.end(); TokenIt++ )
    {
        Result.m_lastMatchTokenIt = TokenIt;

        if ( TokenIt->strToken.size() > MT_STR_MAX_LEN )
        {
            return false;
        }

        if ( pChildList == NULL )
        {
            // completely match case!
            if ( TokenIt->strToken.empty() ) // match completely and last input is '?' or '\t'
            {
                Result.pstEndEntry = GetEndEntry(listTmpResultEntry);
                return true;
            }

            return false;
        }

        // pre do work
        TokenTmpIt = TokenIt;
        bLast = ++TokenTmpIt == listToken.end();

        if ( TokenIt->strToken.empty() && bLast ) // last input is '?' or '\t', need to get last end one result
        {
            Result.pstEndEntry = GetEndEntry(listTmpResultEntry);
        }

        listTmpResultEntry.clear();
        if ( bLast )
        {
            MatchPartResult(pChildList, *TokenIt, listTmpResultEntry);
        }
        else
        {
            MatchResult(pChildList, *TokenIt, listTmpResultEntry);
            if (CheckResultCoherent(listTmpResultEntry) == false)
            {
                return false;
            }
        }

        if ( listTmpResultEntry.empty() )
        {
            return false;
        }

        if ( bLast )
        {
            listResultEntry = listTmpResultEntry;
            break;
        }

        pChildList = NULL;
        std::list< linked_ptr<CMtBaseEntry> >::iterator It;
        for ( It = listTmpResultEntry.begin(); It != listTmpResultEntry.end(); It++ )
        {
            linked_ptr<CMtBaseEntry> pBaseEntry = *(It);
            if ( pBaseEntry->u32Attr == MT_ENTRY_ATTR_LINK )
            {
                if (pChildList != NULL)
                {
                    return false;
                }

                CMtEntryLink *pLinkEntry = dynamic_cast<CMtEntryLink *>(pBaseEntry.get());
                pChildList = &pLinkEntry->listChildEntry;
            }
        }
    }

    return true;
}


void CMt::FilterSameResult(std::list< linked_ptr<CMtBaseEntry> > &listResultEntry)
{
    std::set < CMtEntry * > setEntry;
    std::list< linked_ptr<CMtBaseEntry> >::iterator It = listResultEntry.begin();
    for ( ; It != listResultEntry.end(); )
    {
        linked_ptr<CMtEntry> pEntry = (*It)->pEntry;
        std::set < CMtEntry * >::iterator setIt = setEntry.find(pEntry.get());
        if ( setIt != setEntry.end() )
        {
            listResultEntry.erase(It++);
            continue;
        }

        setEntry.insert(pEntry.get());
        It++;
    }
}

bool CMt::ParseHelp(std::list<MT_TOKEN_S> &listToken, CMtResult &Result)
{
    std::list< linked_ptr<CMtBaseEntry> > listResultEntry;
    Result.m_lastMatchTokenIt = listToken.begin();

    if ( MatchCmdLine(listResultEntry, listToken, Result) == false )
    {
        PrintErrInfo(Result);
        return false;
    }

    if ( Result.m_lastMatchTokenIt->strToken.empty() )
    {
        if ( Result.pstEndEntry != NULL )
        {
            std::cout << "\r\n  <CR>";
        }
    }

    FilterSameResult(listResultEntry);

    std::list< linked_ptr<CMtBaseEntry> >::iterator It = listResultEntry.begin();
    for ( ; It != listResultEntry.end(); It++)
    {
        linked_ptr<CMtEntry> pEntry = (*It)->pEntry;
        char szBuf[128];

        switch ( pEntry->u8Type )
        {
            case MT_ITEM_TYPE_IP :
                printf("\r\n  %-24s%s", "x.x.x.x", pEntry->szDesc);
                break;
            case MT_ITEM_TYPE_CHAN :
                printf("\r\n  %-24s%s", "<type>/<port>/<chan>", pEntry->szDesc);
                break;
            case MT_ITEM_TYPE_HEX :
            case MT_ITEM_TYPE_PTR :
                snprintf(szBuf, sizeof(szBuf), "%s [0x%llx-0x%llx]", pEntry->szName, pEntry->u64MinVal, pEntry->u64MaxVal);
                printf("\r\n  %-24s%s", szBuf, pEntry->szDesc);
                break;
            case MT_ITEM_TYPE_INT :
            case MT_ITEM_TYPE_STR :
                snprintf(szBuf, sizeof(szBuf), "%s [%llu-%llu]", pEntry->szName, pEntry->u64MinVal, pEntry->u64MaxVal);
                printf("\r\n  %-24s%s", szBuf, pEntry->szDesc);
                break;
            default:
                printf("\r\n  %-24s%s", pEntry->szName, pEntry->szDesc);
        }
    }

    return true;
}

void CMt::AutoExpandInfo(linked_ptr<CMtEntry> &pEntry)
{
    switch ( pEntry->u8Type )
    {
        case MT_ITEM_TYPE_IP :
            printf("\r\n  %s", "x.x.x.x");
            break;
        case MT_ITEM_TYPE_CHAN :
            printf("\r\n  %s", "<type>/<port>/<chan>");
            break;
        case MT_ITEM_TYPE_HEX :
        case MT_ITEM_TYPE_PTR :
            printf("\r\n  %s  [0x%llx-0x%llx]", pEntry->szName, pEntry->u64MinVal, pEntry->u64MaxVal);
            break;
        case MT_ITEM_TYPE_INT :
        case MT_ITEM_TYPE_STR :
            printf("\r\n  %s  [%llu-%llu]", pEntry->szName, pEntry->u64MinVal, pEntry->u64MaxVal);
            break;
		case MT_ITEM_TYPE_TEXT :
			printf("\r\n  %2s        %10s", pEntry->szName, pEntry->szDesc);
			//std::cout << "\r\n  " << pEntry->szName << ":                        " << pEntry->szDesc;
			break;
        default:
            std::cout << "\r\n  " << pEntry->szName;
    }
}

bool CMt::AutoExpand(std::list<MT_TOKEN_S> &listToken, CMtResult &Result)
{
    std::list< linked_ptr<CMtBaseEntry> > listResultEntry;
    Result.m_lastMatchTokenIt = listToken.begin();

    if ( MatchCmdLine(listResultEntry, listToken, Result) == false )
    {
        PrintErrInfo(Result);
        return false;
    }

    if ( Result.m_lastMatchTokenIt->strToken.empty() )
    {
        if ( Result.pstEndEntry != NULL )
        {
            std::cout << "\r\n  <CR>";
        }
    }

    FilterSameResult(listResultEntry);

    std::list< linked_ptr<CMtBaseEntry> >::iterator It = listResultEntry.begin();

    if ( listResultEntry.size() == 1 && Result.pstEndEntry == NULL )
    {
        linked_ptr<CMtEntry> pLastEntry = (*It)->pEntry;
        if ( pLastEntry->u8Type == MT_ITEM_TYPE_TEXT )
        {
            std::string::size_type nPos = Result.m_strLine.rfind(' ');
            if ( std::string::npos == nPos )
            {
                Result.m_strLine = pLastEntry->szName;
            }
            else
            {
                Result.m_strLine.erase(nPos + 1);
                Result.m_strLine += pLastEntry->szName;
            }

            Result.m_strLine += " \t";
            return true;
        }
    }

    for ( ; It != listResultEntry.end(); It++)
    {
        linked_ptr<CMtEntry> pEntry = (*It)->pEntry;
        AutoExpandInfo(pEntry);
    }

    return true;
}

bool CMt::ExactMatchChkResult(MT_TOKEN_S &stToken, CMtBaseEntry *pstBaseEntry)
{
    linked_ptr<CMtEntry> pEntry = pstBaseEntry->pEntry;

    if ( pEntry->u8Type == MT_ITEM_TYPE_CHAN )
    {
        if ( CheckFullChan(stToken.strToken, pEntry) == false )
        {
            return false;
        }
    }

    if ( pEntry->u8Type == MT_ITEM_TYPE_IP )
    {
        if ( CheckFullIp(stToken.strToken, pEntry) == false )
        {
            return false;
        }
    }

    return true;
}

bool CMt::ExactMatch(std::list<MT_TOKEN_S> &listToken, CMtResult &Result)
{
    std::list< linked_ptr<CMtBaseEntry> > listResultEntry;

    if ( MatchCmdLine(listResultEntry, listToken, Result) == false )
    {
        PrintErrInfo(Result);
        return false;
    }

    FixMatchResult(listResultEntry);
    FixTextMatch(listResultEntry, *(Result.m_lastMatchTokenIt));

    std::list< linked_ptr<CMtBaseEntry> >::iterator It = listResultEntry.begin();
    for ( ; It != listResultEntry.end(); It++)
    {
        if ( (*It)->u32Attr != MT_ENTRY_ATTR_END )
        {
            continue;
        }

        if ( Result.m_nCmdId )
        {
            PrintAmbiguousInfo(Result);
            return false;
        }

        CMtEndEntry *pstEndEntry = dynamic_cast<CMtEndEntry *>(It->get());
        Result.m_nCmdId    = pstEndEntry->u64CmdId;
        Result.pfnExecCmd  = pstEndEntry->pfnExecCmd;
        Result.pstEndEntry = pstEndEntry;
    }

    bool bRet = Result.m_nCmdId != 0;
    if ( !bRet )
    {
        PrintAmbiguousInfo(Result);
    }
    else
    {
        // retry check exact match
        if ( ExactMatchChkResult(*Result.m_lastMatchTokenIt, dynamic_cast<CMtBaseEntry *>(Result.pstEndEntry)) == false )
        {
            PrintErrInfo(Result);
            bRet = false;
        }
    }

    return bRet;
}

void CMt::ParseInputCmdLine(std::string &strLine)
{
    std::list<MT_TOKEN_S> listToken;
    CMtResult Result;

    Result.m_strLine = strLine;

    if ( GetToken(listToken, strLine) == false )
    {
        return;
    }

    PreParse(listToken, Result);

    if ( Result.m_bNeedExpand )
    {
        AutoExpand(listToken, Result);
        strLine = Result.m_strLine;
        return;
    }
    else if ( Result.m_bNeedHelp )
    {
        ParseHelp(listToken, Result);
        return;
    }

    if ( ExactMatch(listToken, Result) == false )
    {
        return;
    }

    Result.m_strLine.clear();
    std::list<MT_TOKEN_S>::iterator listIt = listToken.begin();
    for ( ; listIt != listToken.end(); listIt++)
    {
        Result.m_strLine += listIt->strToken + ' ';
    }

    // erase last space char
    Result.m_strLine.erase(Result.m_strLine.end() - 1);
    ((MT_EXEC_CMD_PF)Result.pfnExecCmd)(this, Result);
    return;
}

#if 0
#endif

bool CMt::GetView(CMtView * &pstView)
{
    pstView = m_pCurrView.get();
    return pstView != NULL;
}

CMtView *CMt::GetView(U64 u64ViewId)
{
    CMtView *pstView;
    BOOL bLast = false;

    std::map <U64, linked_ptr<CMtView> >::iterator mapIt = m_mapView.find(u64ViewId);
    if ( mapIt == m_mapView.end() )
    {
        return NULL;
    }

    pstView = mapIt->second.get();
    return pstView;
}

bool CMt::FindEntryFrmCache(linked_ptr<CMtEntry> &pEntry, MT_CMD_USR_ITEM_S *pstUsrItem)
{
    std::string strName = pstUsrItem->szName;
    std::multimap < std::string, linked_ptr<CMtEntry> >::iterator LowIt  = m_mapEntryCache.lower_bound(strName);
    if ( LowIt != m_mapEntryCache.end() )
    {
        std::multimap < std::string, linked_ptr<CMtEntry> >::iterator UpIt = m_mapEntryCache.upper_bound(strName);

        for ( ; LowIt != UpIt;  LowIt++ )
        {
            if ( EqualEntry(LowIt->second, pstUsrItem) == true )
            {
                pEntry = LowIt->second;
                return true;
            }
        }
    }

    return false;
}

void CMt::AddEntryToCache(linked_ptr<CMtEntry> &pEntry)
{
    std::string strName = pEntry->szName;

    m_mapEntryCache.insert(std::make_pair(strName, pEntry));
}

bool CMt::EqualEntry(linked_ptr<CMtEntry> &pEntry, MT_CMD_USR_ITEM_S *pstUsrItem)
{
    return (memcmp(pEntry->as8Id, pstUsrItem->as8Id, MT_ID_MAX_LEN) == 0) &&
           (pEntry->u8Type == pstUsrItem->u8Type) &&
           (pEntry->u8Flg  == pstUsrItem->u8Flg) &&
           (strncasecmp(pEntry->szName, pstUsrItem->szName, MT_NAME_MAX_LEN) == 0) &&
           (strncasecmp(pEntry->szDesc, pstUsrItem->szDesc, MT_DESC_MAX_LEN) == 0) &&
           (pEntry->u64MinVal == pstUsrItem->u64MinVal) &&
           (pEntry->u64MaxVal == pstUsrItem->u64MaxVal);
}

bool CMt::CheckEntry(linked_ptr<CMtEntry> &pEntry, MT_CMD_USR_ITEM_S *pstUsrItem)
{
    if ( (pEntry->u8Type == MT_ITEM_TYPE_STR && pstUsrItem->u8Type != MT_ITEM_TYPE_TEXT) ||
         (pstUsrItem->u8Type == MT_ITEM_TYPE_STR && pEntry->u8Type != MT_ITEM_TYPE_TEXT) )
    {
        std::cout << "\r\nFailed to add item string conflict, on string item, it need unique!\r\n ";
        return false;
    }

    if ( pEntry->u8Type == pstUsrItem->u8Type )
    {
        if ( (pEntry->u8Type == MT_ITEM_TYPE_TEXT) &&
             (strncmp(pEntry->szName, pstUsrItem->szName, MT_NAME_MAX_LEN) == 0) )
        {
            std::cout << "\r\nFailed to add item text conflict, text [" << pstUsrItem->szName << "] has been already exist!\r\n ";
            return false;
        }

        if ( pEntry->u8Type == MT_ITEM_TYPE_IP || pEntry->u8Type == MT_ITEM_TYPE_CHAN )
        {
            std::cout << "\r\nFailed to add item conflict, text [" << pstUsrItem->szName << "] has been already exist!\r\n ";
            return false;
        }
    }

    return true;
}

linked_ptr<CMtBaseEntry> CMt::InitEndEntry(linked_ptr<CMtEntry> &pEntry, MT_EXEC_CMD_PF &pfnExecCmd, U64 &u64CmdId)
{
    CMtEndEntry *pstEndEntry = new CMtEndEntry();

    pstEndEntry->u64CmdId = u64CmdId;
    pstEndEntry->pEntry   = pEntry;
    pstEndEntry->u32Attr  = MT_ENTRY_ATTR_END;
    pstEndEntry->pfnExecCmd = (VOID *)pfnExecCmd;
    linked_ptr<CMtBaseEntry> pBaseEntry= linked_ptr<CMtBaseEntry>(dynamic_cast<CMtBaseEntry *>(pstEndEntry));

    return pBaseEntry;
}

linked_ptr<CMtBaseEntry> CMt::InitLinkEntry(linked_ptr<CMtEntry> &pEntry)
{
    CMtEntryLink *pstLinkEntry = new CMtEntryLink();

    pstLinkEntry->pEntry = pEntry;
    pstLinkEntry->u32Attr  = MT_ENTRY_ATTR_LINK;

    linked_ptr<CMtBaseEntry> pBaseEntry = linked_ptr<CMtBaseEntry>(dynamic_cast<CMtBaseEntry *>(pstLinkEntry));
    return pBaseEntry;
}

linked_ptr<CMtBaseEntry> CMt::AllocEntry(MT_CMD_USR_ITEM_S *pstUsrItem, bool bLast, MT_EXEC_CMD_PF &pfnExecCmd, U64 &u64CmdId)
{
    linked_ptr<CMtEntry> pEntry;

    if ( FindEntryFrmCache(pEntry, pstUsrItem) == false )
    {
        pEntry = linked_ptr<CMtEntry> (new CMtEntry(pstUsrItem));
        AddEntryToCache(pEntry);
    }

    if ( bLast )
    {
        return InitEndEntry(pEntry, pfnExecCmd, u64CmdId);
    }
    else
    {
        return InitLinkEntry(pEntry);
    }
}

bool CMt::CheckEntrys(std::list < linked_ptr<CMtBaseEntry> > * &pChildList,
                      MT_CMD_USR_ITEM_S *pstUsrItem, bool bLast)
{
    linked_ptr<CMtEntry> pEntry;
    linked_ptr<CMtBaseEntry> pBaseEntry;

    std::list < linked_ptr<CMtBaseEntry> >::iterator It = pChildList->begin();
    for ( ; It != pChildList->end(); It++)
    {
        if ( EqualEntry((*It)->pEntry, pstUsrItem) )
        {
            if ( bLast )
            {
                if ( (*It)->u32Attr == MT_ENTRY_ATTR_END )
                {
                    return false;
                }

                continue;
            }

            if ( (*It)->u32Attr == MT_ENTRY_ATTR_END )
            {
                continue;
            }

            CMtEntryLink *pEntryLink = dynamic_cast<CMtEntryLink *>((*It).get());
            pChildList = &pEntryLink->listChildEntry;
            return true;
        }
    }

    // can't find the same entry, check conflict
    for ( It = pChildList->begin(); It != pChildList->end(); It++)
    {
        if ( CheckEntry((*It)->pEntry, pstUsrItem) == false )
        {
            return false;
        }
    }

    pChildList = NULL;
    return true;
}

void CMt::DumpEntry(CMtBaseEntry *pBaseEntry)
{
    CMtEntry *pstEntry = pBaseEntry->pEntry.get();

    printf(MT_DBG_U,    "Attr", pBaseEntry->u32Attr);
    printf(MT_DBG_U,    "Type", pstEntry->u8Type);
    printf(MT_DBG_U,    "Flag", pstEntry->u8Type);
    printf(MT_DBG_STR,  "Name", pstEntry->szName);
    printf(MT_DBG_STR,  "Desc", pstEntry->szDesc);
    printf(MT_DBG_U64,  "Max",  pstEntry->u64MaxVal);
    printf(MT_DBG_U64,  "Min",  pstEntry->u64MinVal);

    if ( pstEntry->as8Id[0] > 0 )
    {
        char szId[MT_ID_MAX_LEN + 1];
        memcpy(szId, pstEntry->as8Id, MT_ID_MAX_LEN);
        szId[MT_ID_MAX_LEN] = 0;
        printf(MT_DBG_STR, "Id", szId);
    }
    else
    {
        CMD_ID_T Id = *((CMD_ID_T *)(pstEntry->as8Id));
        printf(MT_DBG_U, "Id", Id);
    }
}

CMtEntryLink * CMt::ToBaseEntry(std::list < linked_ptr<CMtBaseEntry> > *pParent, std::list < linked_ptr<CMtBaseEntry> > *pSelf)
{
    std::list < linked_ptr<CMtBaseEntry> >::iterator It = pParent->begin();

    for ( ; It != pParent->end(); It++)
    {
        if ( (*It)->u32Attr == MT_ENTRY_ATTR_END )
        {
            continue;
        }

        CMtEntryLink *pEntryLink = dynamic_cast<CMtEntryLink *>((*It).get());
        if ( pSelf == &(pEntryLink->listChildEntry) )
        {
            return pEntryLink;
        }
    }

    return NULL;
}

void CMt::PrintConflictCl(std::string strPrefix, CMtBaseEntry *pBaseEntry)
{
    if ( pBaseEntry->u32Attr == MT_ENTRY_ATTR_LINK )
    {
        CMtEntryLink *pEntryLink = dynamic_cast<CMtEntryLink *>(pBaseEntry);
        strPrefix += pEntryLink->pEntry->szName;
        strPrefix += " ";
        std::list < linked_ptr<CMtBaseEntry> >::iterator It = pEntryLink->listChildEntry.begin();
        for ( ; It != pEntryLink->listChildEntry.end(); It++)
        {
            PrintConflictCl(strPrefix, (*It).get());
        }
    }
    else
    {
        std::cout << strPrefix << " " << pBaseEntry->pEntry->szName;
    }
}

void CMt::PrintConflictEntry(std::list < linked_ptr<CMtBaseEntry> > &listMatchEntry, MT_CMD_USR_ITEM_S *pstSelfItem)
{
    std::cout << "\r\nConflict with item :";
    std::list < linked_ptr<CMtBaseEntry> >::iterator It = listMatchEntry.begin();

    for ( ; It != listMatchEntry.end(); It++)
    {
        CMtBaseEntry *pBaseEntry = ((*It).get());
        DumpEntry(pBaseEntry);
    }

    if ( m_listTmpClPath.size() <= 1 )
    {
        return;
    }

    std::cout << "\r\nMay Conflict with cmdline :";
    std::string strPrefix = "\r\n";

    std::list<std::list < linked_ptr<CMtBaseEntry> > * >::iterator listIt = m_listTmpClPath.begin();
    std::list<std::list < linked_ptr<CMtBaseEntry> > * >::iterator listNextIt = listIt;
    listNextIt++;

    for ( ; listNextIt != m_listTmpClPath.end(); )
    {
        CMtEntryLink *pEntryLink = ToBaseEntry(*listIt, *listNextIt);
        if ( pEntryLink == NULL )
        {
            return;
        }

        strPrefix += pEntryLink->pEntry->szName;
        strPrefix += " ";
        listIt++;
        listNextIt++;
    }

    for ( It = listMatchEntry.begin(); It != listMatchEntry.end(); It++)
    {
        CMtBaseEntry *pBaseEntry = ((*It).get());
        PrintConflictCl(strPrefix, pBaseEntry);
    }

    return;
}

bool CMt::RegEntry(std::list < linked_ptr<CMtBaseEntry> > * &pChildList,
                   MT_CMD_USR_ITEM_S *pstUsrItem, bool bLast,
                   MT_EXEC_CMD_PF &pfnExecCmd, U64 &u64CmdId)
{
    linked_ptr<CMtEntry> pEntry;
    linked_ptr<CMtBaseEntry> pBaseEntry;
    bool bInsertEnd  = false;
    bool bInsertLink = false;

    std::list < linked_ptr<CMtBaseEntry> > listMatchEntry;
    std::list < linked_ptr<CMtBaseEntry> >::iterator ItEnd;
    std::list < linked_ptr<CMtBaseEntry> >::iterator It = pChildList->begin();
    for ( ; It != pChildList->end(); It++ )
    {
        if ( EqualEntry((*It)->pEntry, pstUsrItem) )
        {
            listMatchEntry.push_back(*It);

            if ( bLast )
            {
                if ( (*It)->u32Attr == MT_ENTRY_ATTR_END )
                {
                    std::cout << "\r\nRegister the same cmd line, please check!";
                    return false;
                }

                ItEnd = It;
                bInsertEnd = true;
                continue;
            }

            if ( (*It)->u32Attr == MT_ENTRY_ATTR_END )
            {
                bInsertLink = true;
                continue;
            }

            CMtEntryLink *pEntryLink = dynamic_cast<CMtEntryLink *>((*It).get());
            pChildList = &pEntryLink->listChildEntry;
            return true;
        }
    }

    if ( listMatchEntry.size() >= 2 )
    {
        std::cout << "\r\nFailed to add item conflict, find too many the same items [" << pstUsrItem->szName << "]\r\n";
        PrintConflictEntry(listMatchEntry, pstUsrItem);
        return false;
    }

    if ( bInsertEnd )
    {
        pBaseEntry = InitEndEntry((*ItEnd)->pEntry, pfnExecCmd, u64CmdId);
        pChildList->push_back(pBaseEntry);
        return true;
    }

    if ( bInsertLink )
    {
        goto new_alloc;
    }

    // can't find the same entry, check conflict
    for ( It = pChildList->begin(); It != pChildList->end(); It++)
    {
        if ( CheckEntry((*It)->pEntry, pstUsrItem) == false )
        {
            listMatchEntry.clear();
            listMatchEntry.push_back(*It);
            PrintConflictEntry(listMatchEntry, pstUsrItem);
            return false;
        }
    }

new_alloc:
    // alloc new one
    pBaseEntry = AllocEntry(pstUsrItem, bLast, pfnExecCmd, u64CmdId);
    pChildList->push_back(pBaseEntry);

    if ( bLast )
    {
        return true;
    }

    CMtEntryLink *pEntryLink = dynamic_cast<CMtEntryLink *>(pBaseEntry.get());
    pChildList = &pEntryLink->listChildEntry;
    return true;
}

U32 CMt::GetItemArrPos(MT_CL_INFO_S *pstClInfo, U32 nPos)
{
    MT_CMD_USR_ITEM_S *pstItem;
    U32 i;

    for (i = nPos; i < pstClInfo->u32EntryNum; i++)
    {
        pstItem = pstClInfo->astItem + i;
        if ( (pstItem->u8Flg & MT_ITEM_FLG_ARRAY) == 0 )
        {
            return VA_UINT_MAX;
        }

        if ( pstItem->u8Flg & MT_ITEM_FLG_ARR_END )
        {
            break;
        }
    }

    if ( i == pstClInfo->u32EntryNum )
    {
        return VA_UINT_MAX;
    }

    return i + 1;
}

bool CMt::RegEntry(std::list < linked_ptr<CMtBaseEntry> > *pChildList, U32 nPos, MT_CL_INFO_S *pstClInfo, MT_EXEC_CMD_PF pfnExecCmd)
{
    MT_CMD_USR_ITEM_S *pstItem;
    BOOL bLast = false;
    U32 i;

    for (i = nPos; i < pstClInfo->u32EntryNum; i++)
    {
        m_listTmpClPath.push_back(pChildList);
        pstItem = pstClInfo->astItem + i;

        if ( (pstItem->u8Flg & MT_ITEM_FLG_ARRAY) && (!(pstItem->u8Flg & MT_ITEM_FLG_ARR_END)) )
        {
            U32 nEndPos = GetItemArrPos(pstClInfo, i);
            if ( nEndPos == VA_UINT_MAX )
            {
                return false;
            }

            std::list < linked_ptr<CMtBaseEntry> > *pSaveChildList = pChildList;
            std::list<std::list < linked_ptr<CMtBaseEntry> > * > listTmpPath = m_listTmpClPath;

            bLast = (nEndPos == pstClInfo->u32EntryNum);
            pstItem->u8Flg &= ~(MT_ITEM_FLG_ARRAY | MT_ITEM_FLG_ARR_END);
            if ( RegEntry(pChildList, pstItem, bLast, pfnExecCmd, pstClInfo->u64CmdId) == false )
            {
                return false;
            }

            if ( bLast == false )
            {
                // span to array, reach the next item, recursive the cmd line
                if ( RegEntry(pChildList, nEndPos, pstClInfo, pfnExecCmd) == false )
                {
                    return false;
                }
            }

            m_listTmpClPath = listTmpPath;
            m_listTmpClPath.pop_back();
            pChildList = pSaveChildList; // restore prev list position, do the next array item
        }
        else
        {
            bLast = ((i + 1) == pstClInfo->u32EntryNum);
            pstItem->u8Flg &= ~(MT_ITEM_FLG_ARRAY | MT_ITEM_FLG_ARR_END);
            if ( RegEntry(pChildList, pstItem, bLast, pfnExecCmd, pstClInfo->u64CmdId) == false )
            {
                return false;
            }
        }
    }

    return true;
}

bool CMt::RegCmdLine(MT_CL_INFO_S *pstClInfo, MT_EXEC_CMD_PF pfnExecCmd)
{
    CMtView *pstView;

    pstView = GetView(pstClInfo->u64ViewId);
    if ( pstView == NULL )
    {
        return false;
    }

    m_listTmpClPath.clear();

    std::list < linked_ptr<CMtBaseEntry> > *pChildList = &pstView->m_listRoot;

    if ( RegEntry(pChildList, 0, pstClInfo, pfnExecCmd) == false )
    {
         m_listTmpClPath.clear();
        return false;
    }

    m_listTmpClPath.clear();
    return true;
}

bool CMt::RegView(MT_VIEW_INFO_S &stViewInfo)
{
    linked_ptr<CMtView> pView = linked_ptr<CMtView>(new CMtView());

    pView->m_strPrompt   = "[";
    pView->m_strPrompt  += stViewInfo.szName;
    pView->m_strPrompt  += "]";

    std::map< U64, linked_ptr<CMtView> >::iterator MapIt = m_mapView.find(0);
    if ( MapIt == m_mapView.end() )
    {
        return false;
    }

    pView->m_pParentView = MapIt->second;
    m_mapView.insert(std::make_pair(stViewInfo.u64ViewId, pView));

    // register view cmdline
    RegViewCmdLine(stViewInfo);

    return true;
}

#if 0
#endif

VOID CMt::FillUsrItem(IN MT_CMD_ITEM_S *pstItem, OUT MT_CMD_USR_ITEM_S *pstUsrItem)
{
    pstUsrItem->u8Type = pstItem->u8Type;
    pstUsrItem->u8Flg  = pstItem->u8Flg;
    pstUsrItem->u64MinVal = pstItem->u64MinVal;
    pstUsrItem->u64MaxVal = pstItem->u64MaxVal;
    strncpy(pstUsrItem->szDesc, pstItem->szDesc, MT_DESC_MAX_LEN);
    strncpy(pstUsrItem->szName, pstItem->szName, MT_NAME_MAX_LEN);
    memcpy(pstUsrItem->as8Id, pstItem->as8Id, sizeof(pstItem->as8Id));
}

bool CMt::InitCmdLine(MT_CL_INFO_S *pstClInfo, MT_CMD_ITEM_S *pstItem, va_list &args)
{
    MT_CMD_ITEM_S *pstTmpItem;
    U32 u32Pos = 0;

    FillUsrItem(pstItem, pstClInfo->astItem + (u32Pos++));

    pstTmpItem = VA_PTR_TYPE(MT_CMD_ITEM_S, va_arg(args, void *));
    while (pstTmpItem != NULL)
    {
        if ( u32Pos >= pstClInfo->u32EntryNum )
        {
            return false;
        }

        FillUsrItem(pstTmpItem, pstClInfo->astItem + (u32Pos++));
        pstTmpItem = VA_PTR_TYPE(MT_CMD_ITEM_S, va_arg(args, void *));
    }

    pstClInfo->u32EntryNum = u32Pos;
    return true;
}

VOID CMt::RegSelfCmdLine(MT_EXEC_CMD_PF pfnCallBack, U64 u64ViewId, MT_CMD_ITEM_S *pstItem, ...)
{
    MT_CL_INFO_S *pstClInfo;
    va_list args;
    bool bRet;

    pstClInfo = (MT_CL_INFO_S *)VA_Malloc(MT_CL_INFO_MAX_LEN);
    if ( pstClInfo == NULL )
    {
        return;
    }

    pstClInfo->u32Size     = MT_CL_INFO_MAX_LEN - sizeof(MT_CL_INFO_S);
    pstClInfo->u32EntryNum = pstClInfo->u32Size/sizeof(MT_CMD_USR_ITEM_S);
    pstClInfo->u64CmdId    = m_nNextCmdId++;
    pstClInfo->u64ViewId   = u64ViewId;

    va_start(args, pstItem);
    bRet = InitCmdLine(pstClInfo, pstItem, args);
    va_end(args);

    if ( bRet )
    {
        RegCmdLine(pstClInfo, pfnCallBack);
    }

    VA_Free(pstClInfo);
}

#if 0
#endif

bool CMt::OpenDebug(CMt *pMt, CMtResult &Result)
{
    return pMt->SetGlbDebug(1);
}

bool CMt::CloseDebug(CMt *pMt, CMtResult &Result)
{
    return pMt->SetGlbDebug(0);
}

bool CMt::Exit(CMt *pMt, CMtResult &Result)
{
    throw 1;
    return true;
}

VOID CMt::DefTextCmdItem(MT_CMD_ITEM_S &ItemName, const char *szId, const char *szName, const char *szDesc)
{
    ItemName.u8Type = MT_ITEM_TYPE_TEXT;
    ItemName.u8Flg  = 0;
    ItemName.u64MinVal = 0;
    ItemName.u64MaxVal = 0;
    ItemName.szName    = szName;
    ItemName.szDesc    = szDesc;
    strncpy((char *)ItemName.as8Id, szId, sizeof(ItemName.as8Id));
}

#undef MT_DEF_TEXT_CMD_ITEM
#define MT_DEF_TEXT_CMD_ITEM(ItemName, szId, __szName, __szDesc) \
    MT_CMD_ITEM_S ItemName; \
    DefTextCmdItem(ItemName, szId, __szName, __szDesc);

VOID CMt::RegMgrCmdLine()
{
    MT_DEF_TEXT_CMD_ITEM(stUndo, "undo",  "undo",  "undo");
    MT_DEF_TEXT_CMD_ITEM(stDbg,  "dbg",   "debug", "debug");
    MT_DEF_TEXT_CMD_ITEM(stMt,   "mt",    "mt",    "");
    MT_DEF_TEXT_CMD_ITEM(stExit, "exit",  "exit",  "exit system");

    RegSelfCmdLine(OpenDebug, 0, &stDbg,  &stMt, NULL);
    RegSelfCmdLine(CloseDebug, 0, &stUndo, &stDbg, &stMt, NULL);
    RegSelfCmdLine(Exit, 0, &stExit, NULL);
}

bool CMt::ViewProc(CMt *pMt, CMtResult &Result)
{
    bool bRet;

    std::map <U64, linked_ptr<CMtView> >::iterator mapIt = pMt->m_mapView.find(Result.m_nCmdId);
    if ( mapIt == pMt->m_mapView.end() )
    {
        return false;
    }

    pMt->m_pCurrView = mapIt->second;

    if ( mapIt->second->m_bLoad )
    {
        return true;
    }

    bRet = pMt->GetAllCmdLine(mapIt->first);
    if ( bRet == true )
    {
        mapIt->second->m_bLoad = true;
    }

    return bRet;
}

bool CMt::QuitView(CMt *pMt, CMtResult &Result)
{
    pMt->m_pCurrView = pMt->m_pCurrView->m_pParentView;
    return true;
}

VOID CMt::__RegViewCmdLine(MT_EXEC_CMD_PF pfnCallBack, U64 u64ViewId, MT_CMD_ITEM_S *pstItem, ...)
{
    MT_CL_INFO_S *pstClInfo;
    va_list args;
    bool bRet;

    pstClInfo = (MT_CL_INFO_S *)VA_Malloc(MT_CL_INFO_MAX_LEN);
    if ( pstClInfo == NULL )
    {
        return;
    }

    pstClInfo->u32Size     = MT_CL_INFO_MAX_LEN - sizeof(MT_CL_INFO_S);
    pstClInfo->u32EntryNum = pstClInfo->u32Size/sizeof(MT_CMD_USR_ITEM_S);
    pstClInfo->u64CmdId    = u64ViewId;
    pstClInfo->u64ViewId   = 0;

    va_start(args, pstItem);
    bRet = InitCmdLine(pstClInfo, pstItem, args);
    va_end(args);

    if ( bRet )
    {
        RegCmdLine(pstClInfo, pfnCallBack);
    }

    VA_Free(pstClInfo);
}

VOID CMt::RegViewSelfCmdLine(U64 u64ViewId)
{
    MT_DEF_TEXT_CMD_ITEM(stQuit, "quit",  "quit",  "quit view");
    MT_DEF_TEXT_CMD_ITEM(stExit, "exit",  "exit",  "exit system");

    RegSelfCmdLine(Exit,     u64ViewId, &stExit, NULL);
    RegSelfCmdLine(QuitView, u64ViewId, &stQuit, NULL);
}

VOID CMt::RegViewCmdLine(MT_VIEW_INFO_S &stViewInfo)
{
    MT_DEF_TEXT_CMD_ITEM(stView, "view", stViewInfo.szName, stViewInfo.szDesc);

    __RegViewCmdLine(ViewProc, stViewInfo.u64ViewId, &stView, NULL);
    RegViewSelfCmdLine(stViewInfo.u64ViewId);
}

void CMt::ClearViewCmdLines(U64 u64ViewId)
{
    CMtView *pstView;

    pstView = GetView(u64ViewId);
    if ( pstView == NULL )
    {
        return;
    }

    pstView->m_listRoot.clear();
    RegViewSelfCmdLine(u64ViewId);
}


