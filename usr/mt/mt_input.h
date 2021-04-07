#ifndef __MT_INPUT_H__
#define __MT_INPUT_H__

#include "va_usr_pub.h"
#include <iostream>
#include <termios.h>

#include "linked_ptr.h"

using namespace google;

enum
{
    MT_INPUT_SUCCESS,
    MT_INPUT_AGAIN,
    MT_INPUT_SYS_ERR,
};

enum
{
    MT_INPUT_FLAG_NORM,
    MT_INPUT_FLAG_REDO,
};

class CInputLine
{
public:
    CInputLine();
    ~CInputLine();

    bool SetTtyParam();
    void SetPrompt(std::string &strPrompt);
    int  ReadLine(std::string &strLine, int nFlag);

private:
    void PrintWelcome();
    void GetNextHistoryCmd();
    void GetPrevHistoryCmd();
    void AppendHistory();
    void NewLine();
    void ResetCmd();
    void ParseBackSpace();
    char GetCharWithNoBlock();
    void ClearInputDevBuf();
    void EraseInput();
    void ShowCmd();
    void RedoCurrLine();
    void ParseFuncKey(char c);
    bool AppendChar(char c);
    int  GetChar(char &c);
    int  ReadLine();

private:
    int  m_nStdInFd;
    std::string m_strPrompt;
    linked_ptr < std::string > m_pstrCmdLine;
    std::string::iterator  m_CursorPos;
    std::list< linked_ptr < std::string > > m_listHistory;
    std::list< linked_ptr < std::string > >::iterator m_CurrCmdPos;
    struct termios m_stSaveTerm;
    bool m_bRecoveryTerm;
};

#endif //__MT_INPUT_H__
