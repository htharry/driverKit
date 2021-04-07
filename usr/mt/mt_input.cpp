#include "mt_main.h"
#include <fcntl.h>
#include <poll.h>
#include "mt_def.h"

using namespace google;

bool CInputLine::SetTtyParam()
{
    struct termios stNewTerm;
    int iRet;

    iRet = tcgetattr(m_nStdInFd, &m_stSaveTerm);
    if ( iRet < 0 )
    {
        return false;
    }

    stNewTerm = m_stSaveTerm;
    cfmakeraw(&stNewTerm);
    stNewTerm.c_oflag |= OPOST;

    iRet = tcsetattr(m_nStdInFd, TCSAFLUSH, &stNewTerm);
    if ( iRet < 0 )
    {
        return false;
    }

    m_bRecoveryTerm = true;
    return true;
}

void CInputLine::PrintWelcome()
{
    std::cout << "\r\n==============================================\r\n";
    std::cout << "\t\tWelcome to cmdline!\r\n";
    std::cout << "==============================================\r\n";
}

CInputLine::CInputLine()
{
    m_nStdInFd   = fileno(stdin);
    m_CurrCmdPos = m_listHistory.begin();
    m_strPrompt  = "[SYS]";
    m_bRecoveryTerm = false;

    setbuf(stdout, NULL); //cancel out file buffer
    PrintWelcome();
}

CInputLine::~CInputLine()
{
    if ( m_bRecoveryTerm )
    {
        tcsetattr(m_nStdInFd, TCSANOW, &m_stSaveTerm);
    }
}

void CInputLine::SetPrompt(std::string &strPrompt)
{
    m_strPrompt = strPrompt;
}

void CInputLine::GetNextHistoryCmd()
{
    if ( m_CurrCmdPos != m_listHistory.begin() )
    {
        m_CurrCmdPos--;
    }

    m_pstrCmdLine = *m_CurrCmdPos;
    return;
}

void CInputLine::GetPrevHistoryCmd()
{
    m_CurrCmdPos++;

    if ( m_CurrCmdPos == m_listHistory.end() )
    {
        m_CurrCmdPos--; // reach end
    }

    m_pstrCmdLine = *m_CurrCmdPos;
    return;
}

void CInputLine::AppendHistory()
{
    std::list< linked_ptr<std::string> >::iterator BeginIt = m_listHistory.begin();

    if ( m_CurrCmdPos != BeginIt )
    {
        m_listHistory.pop_front();
        m_listHistory.push_front(linked_ptr<std::string>(new std::string(*m_pstrCmdLine)));
    }
}

void CInputLine::NewLine()
{
    if ( m_pstrCmdLine == NULL || m_pstrCmdLine->size() != 0 )
    {
        m_pstrCmdLine = linked_ptr<std::string>(new std::string());
        m_listHistory.push_front(m_pstrCmdLine);
    }

    ResetCmd();
}

void CInputLine::ResetCmd()
{
    m_CurrCmdPos = m_listHistory.begin();
    m_pstrCmdLine->clear();
    m_CursorPos  = m_pstrCmdLine->end();
    std::cout << "\r\n";
    ShowCmd();
}

void CInputLine::ParseBackSpace()
{
    if ( m_pstrCmdLine->size() == 0 )
    {
        return;
    }

    EraseInput();

    if ( m_CursorPos != m_pstrCmdLine->begin() )
    {
        m_CursorPos--;
        m_CursorPos = m_pstrCmdLine->erase(m_CursorPos);
    }

    ShowCmd();
}

char CInputLine::GetCharWithNoBlock()
{
    char c;
    int iSaveFlg, iNewFlg;
    int iRet;

    iSaveFlg = fcntl(m_nStdInFd, F_GETFL, 0);
    if ( iSaveFlg < 0 )
    {
        return 0;
    }

    iNewFlg = iSaveFlg | O_NONBLOCK;
    iRet = fcntl(m_nStdInFd, F_SETFL, iNewFlg);
    if ( iRet < 0 )
    {
        return 0;
    }

    c = getchar();

    fcntl(m_nStdInFd, F_SETFL, iSaveFlg);
    return c;
}

void CInputLine::ClearInputDevBuf()
{
    while (GetCharWithNoBlock() > 0);
}

void CInputLine::EraseInput()
{
    U32 i;

    std::cout << '\r';
    for (i = 0; i < m_strPrompt.size(); i++)
    {
        std::cout << ' ';
    }

    for (i = 0; i < m_pstrCmdLine->size(); i++)
    {
        std::cout << ' ';
    }

    std::cout << '\r';
}

void CInputLine::ShowCmd()
{
    std::cout << m_strPrompt;
    std::cout << *m_pstrCmdLine;

    for ( U32 i = 0; i < m_pstrCmdLine->end() - m_CursorPos; i++ )
    {
        std::cout << '\b';
    }
}

void CInputLine::RedoCurrLine()
{
    m_CursorPos = m_pstrCmdLine->end();
    ShowCmd();
}

void CInputLine::ParseFuncKey(char c)
{
    // just parse up down right left key
    // 0x1b 0x5b A : up
    // 0x1b 0x5b B : down
    // 0x1b 0x5b C : right
    // 0x1b 0x5b D : left

    // skip other func key
    if ( (c = getchar()) != 0x5b )
    {
        ClearInputDevBuf();
        return;
    }

    c = getchar();
    if ( c < 'A' || c > 'D' )
    {
        // do until meet 0x7E
        while ( 1 )
        {
            c = GetCharWithNoBlock();
            if ( c == 0 || c == 0x7E )
            {
                return;
            }
        }
    }

    if ( c == 'A' ) // up
    {
        EraseInput();
        GetPrevHistoryCmd();
        RedoCurrLine();
    }
    else if ( c == 'B' ) // down
    {
        EraseInput();
        GetNextHistoryCmd();
        RedoCurrLine();
    }
    else if ( c == 'C' ) // right
    {
        if ( m_CursorPos != m_pstrCmdLine->end() )
        {
            std::cout << *m_CursorPos;
            m_CursorPos++;
        }
    }
    else // left
    {
        if ( m_CursorPos != m_pstrCmdLine->begin() )
        {
            m_CursorPos--;
            std::cout << '\b';
        }
    }

    return;
}

bool CInputLine::AppendChar(char c)
{
    if ( m_pstrCmdLine->size() < MT_MAX_INPUT_LEN )
    {
        if ( m_CursorPos == m_pstrCmdLine->end())
        {
            m_CursorPos = m_pstrCmdLine->insert(m_CursorPos, c);
            m_CursorPos++;
        }
        else
        {
            m_CursorPos = m_pstrCmdLine->insert(m_CursorPos, c);
            m_CursorPos++;
            EraseInput();
            ShowCmd();
        }

        return true;
    }
    else
    {
        return false;
    }
}

int CInputLine::GetChar(char &c)
{
    struct pollfd stPollFd;
    int iRet;

    stPollFd.fd      = m_nStdInFd;
    stPollFd.events  = POLLIN;
    stPollFd.revents = 0;

    iRet = poll(&stPollFd, 1, 1000);
    if ( iRet < 0 )
    {
        return MT_INPUT_SYS_ERR;
    }

    if ( iRet == 0 || (stPollFd.revents & POLLIN) == 0 )
    {
        return MT_INPUT_AGAIN;
    }

    iRet = read(m_nStdInFd, &c, 1);
    if ( iRet <= 0 )
    {
        return MT_INPUT_AGAIN;
    }

    return MT_INPUT_SUCCESS;
}

int CInputLine::ReadLine()
{
    char c;
    int nRet;

    do
    {
        nRet = GetChar(c);
        if ( nRet != MT_INPUT_SUCCESS )
        {
            return nRet;
        }

        if ( c == '\r' )
        {
            break;
        }

        switch ( c )
        {
            case '\t':
            case '?' :
                if (AppendChar(c))
                {
                    return MT_INPUT_SUCCESS;
                }
                continue;
            case '\b' :
            case 0x7F :
                ParseBackSpace();
                continue;
            case 0x3 :  // ctrl + C, switch to next line, clear all input!
                ResetCmd();
                continue;
            case 0x1b :
                ParseFuncKey(c);
                continue;
            default:
            {
                if (!isprint(c))
                {
                    continue;
                }

                if ( m_pstrCmdLine->size() < MT_MAX_INPUT_LEN )
                {
                    std::cout << c; // display this char
                    break;
                }
                else
                {
                    continue;
                }
            }
        }

        AppendChar(c);
    }while (1);

    return MT_INPUT_SUCCESS;
}

int CInputLine::ReadLine(std::string &strLine, int nFlag)
{
    int nRet;

    if ( nFlag == MT_INPUT_FLAG_NORM )
    {
        if ( strLine.size() != 0 && (strLine.at(strLine.size() - 1) == '?' || strLine.at(strLine.size() - 1) == '\t') ) //help cmd line
        {
            strLine.erase(strLine.end() - 1);
            *m_pstrCmdLine = strLine;
            std::cout << "\r\n";
            RedoCurrLine();
        }
        else // exec a cmd, to next new line
        {
            strLine.clear();
            AppendHistory();
            NewLine();
        }
    }

    nRet    = ReadLine();
    strLine = *m_pstrCmdLine;

    return nRet;
}
