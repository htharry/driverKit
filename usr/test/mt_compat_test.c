#include "va_usr_pub.h"
#include "mt_usr.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <poll.h>

static U32 gu32TestCnt;

VOID MT_TEST_DispCb(IN ULONG ulExecId)
{
    DRVLIB_MT_PRINT(MT_FMT_STR, "state", "connect");
}

VOID MT_TEST_DispStat(IN ULONG ulExecId)
{
    DRVLIB_MT_PRINT(MT_FMT_U32_HEX, "test cnt", ++gu32TestCnt);
}

VOID MT_TEST_ResetStat(IN ULONG ulExecId)
{
    gu32TestCnt = 0;
    DRVLIB_MT_PRINT("\r\n Clear stat ok!");
}

VOID MT_TestReadVal(ULONG ulExecId)
{
    ULONG ulVal;

    if (DRVLib_GetValFrmMtParams(ulExecId, MT_CMD_ID_VAL, &ulVal))
    {
        MT_PRINT("\r\n Invalid input value");
        return;
    }

    MT_PRINT(MT_FMT_UL, "value", ulVal);
}

VOID MT_TestReadStr(ULONG ulExecId)
{
    char szText[MT_NAME_MAX_LEN];

    if (DRVLib_GetStrFrmMtParams(ulExecId, MT_CMD_ID_BASE_BS_MEDIA, szText))
    {
        MT_PRINT("\r\n Invalid input str");
        return;
    }

    MT_PRINT(MT_FMT_STR, "my string", szText);
}

void MT_TestDispStr(ULONG ulExecId)
{
    char szText[MT_NAME_MAX_LEN];

    if (DRVLib_GetStrFrmMtParams(ulExecId, MT_CMD_ID_BASE_BM, szText))
    {
        MT_PRINT("\r\n Invalid input max str");
        return;
    }

    MT_PRINT(MT_FMT_STR, "my string", szText);
}

void MT_TestDispText(ULONG ulExecId)
{
    char szText[MT_NAME_MAX_LEN];

    if (DRVLib_GetTitleFrmMtParams(ulExecId, MT_CMD_ID_COUNT, szText))
    {
        MT_PRINT("\r\n Invalid input max str");
        return;
    }

    MT_PRINT(MT_FMT_STR, "my title", szText);
}


void MT_TestDispHex(ULONG ulExecId)
{
    ULONG ulVal;

    if (DRVLib_GetValFrmMtParams(ulExecId, MT_CMD_ID_VAL_HEX, &ulVal))
    {
        MT_PRINT("\r\n Invalid input value");
    }

    MT_PRINT(MT_FMT_UL_HEX, "value", ulVal);
}

void MT_TEST_RegMt(MT_APP_CB_S *pstMtAppCb)
{
    MT_CMD_ITEM_S stVal;
    MT_CMD_ITEM_S stStr;
    MT_CMD_ITEM_S stMaxStr;
    MT_CMD_ITEM_S stText;
    MT_CMD_ITEM_S stHex;

    stVal = MT_INT_CMD_ELEM_INIT(MT_CMD_ID_VAL, "value", 0, 100);
    stStr = MT_STR_CMD_ELEM_INIT(MT_CMD_ID_BASE_BS_MEDIA, "test string", 5);
    stMaxStr = MT_STR_CMD_ELEM_INIT(MT_CMD_ID_BASE_BM, "test string", 32);
    stText = MT_TITLE_CMD_ELEM_INIT(MT_CMD_ID_COUNT, "count", "count");
    stHex  = MT_INT_CMD_ELEM_INIT(MT_CMD_ID_VAL_HEX, "value", 0, 300);

    BM_REG_MT_CMD(MT_TestReadVal,  MT_DEF_ITEM_READ, &stVal);
    BM_REG_MT_CMD(MT_TestReadStr,  MT_DEF_ITEM_READ, MT_DEF_ITEM_STAT, &stStr);
    BM_REG_MT_CMD(MT_TestDispStr,  MT_DEF_ITEM_DISP, &stMaxStr, MT_DEF_ITEM_CB);
    BM_REG_MT_CMD(MT_TestDispText,  MT_DEF_ITEM_DISP, MT_DEF_ITEM_CB, &stText);
    BM_REG_MT_CMD(MT_TestDispHex,  MT_DEF_ITEM_DISP, MT_DEF_ITEM_CB, &stHex);
    BM_REG_MT_CMD(MT_TestDispStr,  MT_DEF_ITEM_DISP, &stMaxStr);
}

int main(void)
{
    struct pollfd stPollFd;
    INT iRet;
    char szPkt[256];
    int i;

    MT_APP_CB_S stMtAppCb = MT_USR_DEF_APP_CB("ctest", "compat test", MT_DEBUG_INFO|MT_DEBUG_PKT|MT_DEBUG_MSG,
                                              MT_TEST_DispCb, MT_TEST_DispStat, MT_TEST_ResetStat);

    for ( i = 0; i < sizeof(szPkt); i++)
    {
        szPkt[i] = (char)i;
    }

    if (DRVLib_RegAppMtCb(&stMtAppCb) != VA_SUCCESS)
    {
        return -1;
    }

    MT_TEST_RegMt(&stMtAppCb);

    while (1)
    {
        stPollFd.fd      = DRVLib_GetMtFd(&stMtAppCb);
        stPollFd.events  = POLLIN;
        stPollFd.revents = 0;

        iRet = poll(&stPollFd, 1, 10 * 1000);
        DRVLib_ExecMtCmd(&stMtAppCb);

        if ( iRet == 0 )
        {
            MT_USR_DBG_PRINT_INFO(&stMtAppCb, MT_DBG_INT, "test val", gu32TestCnt++);
            MT_USR_DBG_PRINT_PKT(&stMtAppCb, "ctest1", szPkt, sizeof(szPkt));
            MT_USR_DBG_PRINT_PKT(&stMtAppCb, "ctest2", szPkt, 32);
        }
    }

    return 0;
}

