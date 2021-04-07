#include "mt_main.h"
#include <poll.h>

using namespace google;

CMt::CMt()
{
    m_nMtFd      = -1;
    m_nStdInFd   = fileno(stdin);
    m_nNextCmdId = VA_U64_MAX - 0x8000000ULL;
}

CMt::~CMt()
{
    if ( m_nMtFd >= 0 )
    {
        ::close(m_nMtFd);
    }
}

bool CMt::Renew()
{
    m_pCurrView.reset();
    m_mapView.clear();
    m_mapEntryCache.clear();

    m_nNextCmdId = VA_U64_MAX - 0x8000000ULL;

    m_pCurrView = linked_ptr<CMtView>(new CMtView);
    m_mapView.insert(std::make_pair(m_pCurrView->u64CmdId, m_pCurrView));

    if ( GetAllCmdLine() == false )
    {
        std::cout << "\r\nFailed to get cmd line!\r\n";
        return false;
    }

    RegMgrCmdLine();
    return true;
}

bool CMt::Init()
{
    m_nMtFd = open(MT_IO_FILE, O_RDONLY);
    if ( m_nMtFd < 0 )
    {
        std::cout << "\r\nFailed to open file " << MT_IO_FILE << " !\r\n";
        return false;
    }

    if (m_Input.SetTtyParam() == false)
    {
        std::cout << "\r\nFailed to set tty param!\r\n";
        return false;
    }

    std::cout << "\r\nWill register all cmd lines, please wait some seconds....\r\n";
    return Renew();
}

bool CMt::PollCmdStatus(std::string &strLine)
{
    struct pollfd stPollFd;
    INT iRet;

    stPollFd.fd      = m_nMtFd;
    stPollFd.events  = POLLIN;
    stPollFd.revents = 0;

    iRet = poll(&stPollFd, 1, 0);
    if ( iRet == 1 )
    {
        if ( strLine.empty() == false )
        {
            std::cout << "\r\ncmd lines is changed, need to reload the all cmd lines, please wait some seconds....\r\n";
            sleep(2);
        }
        else
        {
            sleep(3);
        }

        if ( GetClChangeStatus() )
        {
            if (Renew() < 0 )
            {
                return true;
            }
        }

        return true;
    }

    return false;
}

void CMt::Run()
{
    std::string strLine;
    int nRet;
    bool bState;
    int nFlag = MT_INPUT_FLAG_NORM;

    while ( 1 )
    {
        m_Input.SetPrompt(m_pCurrView->m_strPrompt);
        nRet = m_Input.ReadLine(strLine, nFlag);
        if ( nRet == MT_INPUT_SYS_ERR )
        {
            break;
        }

        if ( strLine == "exit" )
        {
            break;
        }

        bState = PollCmdStatus(strLine);
        if ( bState )
        {
            nFlag = MT_INPUT_FLAG_NORM;
            continue;
        }

        if ( nRet == MT_INPUT_AGAIN )
        {
            nFlag = MT_INPUT_FLAG_REDO;
            continue;
        }

        if ( strLine.empty() )
        {
            nFlag = MT_INPUT_FLAG_NORM;
            continue;
        }

        ParseInputCmdLine(strLine);
        nFlag = MT_INPUT_FLAG_NORM;
    }

    std::cout << "\r\n";
}

int main(void)
{
    CMt Mt;

    if ( Mt.Init() == false )
    {
        return -1;
    }

    try
    {
        Mt.Run();
    }
    catch (int iErrNo)
    {
        std::cout << "\r\n";
    }

    return 0;
}
