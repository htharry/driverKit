#ifndef __MT_PARSE_H__
#define __MT_PARSE_H__

using namespace google;

enum
{
    MT_ENTRY_ATTR_END,
    MT_ENTRY_ATTR_LINK,
    MT_ENTRY_ATTR_VIEW,
};

class CMtEntry
{
public:
    CMtEntry() {}
    CMtEntry(MT_CMD_USR_ITEM_S *pstUsrItem);
    U8   u8Type;
    U8   u8Flg;
    char szName[MT_NAME_MAX_LEN + 1];
    S8   as8Id[MT_ID_MAX_LEN];
    U64  u64MinVal;
    U64  u64MaxVal;
    char szDesc[MT_DESC_MAX_LEN + 1];
};

class CMtBaseEntry
{
public:
    CMtBaseEntry() {}
    virtual ~CMtBaseEntry() {}
    linked_ptr<CMtEntry> pEntry;
    U32   u32Attr;
};

class CMtEndEntry : public CMtBaseEntry
{
public:
    CMtEndEntry();
    U64  u64CmdId;
    VOID *pfnExecCmd;
};

class CMtEntryLink : public CMtBaseEntry
{
public:
    CMtEntryLink() {}
    std::list < linked_ptr<CMtBaseEntry> > listChildEntry;
};

struct MT_TOKEN_S
{
    std::string strToken;
    U32 u32Pos;
};

class CMtResult
{
public:
    CMtResult();

    bool m_bNeedExpand;
    bool m_bNeedHelp;
    U64  m_nCmdId;
    std::list<MT_TOKEN_S>::iterator m_lastMatchTokenIt;
    std::string m_strLine;
    VOID *pfnExecCmd;
    CMtEndEntry *pstEndEntry;
};

class CMtView : public CMtEndEntry
{
public:
    CMtView();
    std::list < linked_ptr<CMtBaseEntry> > m_listRoot;
    std::string m_strPrompt;
    linked_ptr<CMtView> m_pParentView;
    bool m_bLoad;
};

#endif //__MT_PARSE_H__
