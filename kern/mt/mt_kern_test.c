#include "va_kern_pub.h"

static U32 gu32TestStat;

VOID MT_TestCmdProcOfStat(ULONG ulExecId)
{

    MT_PRINT(MT_FMT_INT, "stat", gu32TestStat++);
}

VOID MT_TestDispMod(ULONG ulExecId)
{
    MT_PRINT(MT_FMT_INT, "stat", gu32TestStat);
}

VOID MT_TestRestMod(ULONG ulExecId)
{
    gu32TestStat = 0;
    MT_PRINT("\r\n Reset stat ok!");
}

VOID MT_TestReadVal(ULONG ulExecId)
{
    ULONG ulVal;

    if (MT_GetIntVal(ulExecId, "val", &ulVal))
    {
        MT_PRINT("\r\n Invalid input value");
    }

    MT_PRINT(MT_FMT_UL, "value", ulVal);
}

VOID MT_TestDispIp(ULONG ulExecId)
{
    const char *szText;
    BE32 b232IpAddr;

    if (MT_GetIpv4Val(ulExecId, "ip", &b232IpAddr))
    {
        MT_PRINT("\r\n Invalid input ip");
    }

    MT_GetStrVal(ulExecId, "ip", &szText);

    MT_PRINT(MT_FMT_STR, "ip address", szText);
}

VOID MT_TestDispChan(ULONG ulExecId)
{
    CHAN_ID_S stChanId;

    if (MT_GetChanIdVal(ulExecId, "chan", &stChanId))
    {
        MT_PRINT("\r\n Invalid input chan");
    }

    MT_PRINT(MT_FMT_CHAN, "chan id", VA_CHAN_ARGS(&stChanId));
}

VOID MT_TestReadStr(ULONG ulExecId)
{
    const char *szText;

    if (MT_GetStrVal(ulExecId, "str", &szText))
    {
        MT_PRINT("\r\n Invalid input str");
    }

    MT_PRINT(MT_FMT_STR, "my string", szText);
}

void MT_TestDispStr(ULONG ulExecId)
{
    const char *szText;

    if (MT_GetStrVal(ulExecId, "str", &szText))
    {
        MT_PRINT("\r\n Invalid input max str");
    }

    MT_PRINT(MT_FMT_STR, "my string", szText);
}

int MT_KTEST_RegCmd(MT_CB_S *pstMtCb)
{
    MT_CMD_ITEM_ARR_S stDispModArr;

    MT_DEF_INT_CMD_ITEM(stVal,      "val", 0, 100, "value");
    MT_DEF_STR_CMD_ITEM(stStr,      "str", 1, 5,   "test string");
    MT_DEF_STR_CMD_ITEM(stMaxStr,   "str", 1, 32, "test string");
    MT_DEF_TEXT_CMD_ITEM(stCbcText, "cbc", "cbc", "test cbc text");

    MT_MERGE_ITEM_TO_ARR(&stDispModArr, MT_DEF_ITEM_CB, MT_DEF_ITEM_STAT);

    MT_REG_CMD_LINE(MT_TestCmdProcOfStat, MT_DEF_ITEM_DISP, MT_DEF_ITEM_CB);
    MT_REG_CMD_LINE(MT_TestCmdProcOfStat, MT_DEF_ITEM_DISP, &stCbcText, MT_DEF_ITEM_CB);
    MT_REG_CMD_LINE(MT_TestDispIp,        MT_DEF_ITEM_DISP, MT_DEF_ITEM_IP);
    MT_REG_CMD_LINE(MT_TestReadVal,       MT_DEF_ITEM_READ, &stVal);
    MT_REG_CMD_LINE(MT_TestReadStr,       MT_DEF_ITEM_READ, MT_DEF_ITEM_STAT, &stStr);
    MT_REG_CMD_LINE(MT_TestDispChan,      MT_DEF_ITEM_DISP, MT_DEF_ITEM_CHAN);
    MT_REG_CMD_LINE(MT_TestDispStr,       MT_DEF_ITEM_DISP, MT_DEF_ITEM_CB, &stMaxStr);
    //MT_REG_CMD_LINE(MT_TestDispMod,       MT_DEF_ITEM_DISP, MT_DEF_ITEM_MOD, MT_DEF_ITEM_CB);
    MT_REG_CMD_LINE(MT_TestDispMod,       MT_DEF_ITEM_DISP, MT_DEF_ITEM_MOD, &stDispModArr);
    MT_REG_CMD_LINE(MT_TestRestMod,       MT_DEF_ITEM_RESET, MT_DEF_ITEM_MOD, MT_DEF_ITEM_STAT);

    return 0;
}

static MT_CB_S gstMtKtest =
{
    .pfnRegCmd  = MT_KTEST_RegCmd,
    .szName 	= "mt_ktest",
    .szDesc 	= "mt in kernel test",
    .u32DebugCap = MT_DBG_ALL,
};

static struct delayed_work gstMtKTestWork;

void MT_DebugTest(struct work_struct *work)
{
    char szPkt[256];
    U32  i;

    for (i = 0; i < sizeof(szPkt); i++)
    {
        szPkt[i] = (char)i;
    }

    MT_DBG_INFO_PRINT(&gstMtKtest, MT_DBG_INT, "test val", gu32TestStat++);
    MT_DBG_PKT_PRINT(&gstMtKtest, (U8 *)szPkt, sizeof(szPkt), "ktest1");
    MT_DBG_PKT_PRINT(&gstMtKtest, (U8 *)szPkt, 32, "ktest2");

    schedule_delayed_work(&gstMtKTestWork, msecs_to_jiffies(15 * 1000));
}

static void MT_KTEST_Exit(void)
{
    cancel_delayed_work_sync(&gstMtKTestWork);
    MT_UnRegMt(&gstMtKtest);
}

static int MT_KTEST_Init(VA_MOD_S *pstMod)
{
    MT_RegMt(&gstMtKtest);
    INIT_DELAYED_WORK(&gstMtKTestWork, MT_DebugTest);
    schedule_delayed_work(&gstMtKTestWork, msecs_to_jiffies(15 * 1000));

    return 0;
}


VA_MOD_INIT(mt_ktest, MT_KTEST_Init, MT_KTEST_Exit, VA_INIT_LEVEL_MISC);

