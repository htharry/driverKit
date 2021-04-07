#include "fdd.h"
#include "fdd_mt.h"
#include "va_chan_base.h"
#include "vc/vc_mgr.h"

extern FDD_CHAN_S *FDD_FindChan(CHAN_ID_S *pstChanId);

VOID __FDD_MT_DispBuffMgr(ULONG ulExecId, FDD_BUF_MGR_S *pstBufMgr)
{
    MT_PRINT("\r\ndisplay fdd buffer manager:");
    MT_PRINT(MT_FMT_HEX, "total size",   pstBufMgr->u32TotalSize);
    MT_PRINT(MT_FMT_INT, "block size",   pstBufMgr->u32BlkSize);
    MT_PRINT(MT_FMT_INT, "max buf size", pstBufMgr->u32MaxBufSize);
    MT_PRINT(MT_FMT_INT, "block num",    pstBufMgr->u32BlkNum);
    MT_PRINT(MT_FMT_INT, "free number",  pstBufMgr->u32FreeNum);
    MT_PRINT(MT_FMT_INT, "page  num", pstBufMgr->u32PageNum);
    MT_PRINT(MT_FMT_PTR, "curr blk",  pstBufMgr->pstCurrBlk);
    MT_PRINT(MT_FMT_PTR, "mem buf",   pstBufMgr->pMemBuf);

    return;
}

VOID FDD_MT_DispBuffMgr(ULONG ulExecId)
{
    FDD_CB_S *pstFddCb = GetFddCb();

    __FDD_MT_DispBuffMgr(ulExecId, &pstFddCb->stBufMgr);
}

VOID FDD_MT_DispQueue(FDD_BUF_HEAD_S *pstQueue, ULONG ulExecId)
{
    MT_PRINT(MT_FMT_U, "Queue  Len",   pstQueue->u32QueLen);
    MT_PRINT(MT_FMT_U, "Buffer Len",   pstQueue->u32Bytes);
    MT_PRINT(MT_FMT_U, "Limit  Len",   pstQueue->u32LimitLen);
    MT_PRINT(MT_FMT_U, "Stat Count",   pstQueue->u32StatCnt);
}

VOID FDD_MT_DispVsw(FDD_LISTEN_PORT_S *pstListenPort, ULONG ulExecId)
{
    FDD_VSW_PORT_S *pstVswPort;
    FDD_PORT_S     *pstPort;
    FDD_PORT_S     *pstTmpPort;
    U32 nCount = 0;

    list_for_each_entry(pstPort, &pstListenPort->stVswHead, stListenNode)
    {
        pstVswPort = (FDD_VSW_PORT_S *)pstPort;

        MT_PRINT("\r\nDisplay One VSW:");

        MT_PRINT(MT_FMT_I,    "Ref",           atomic_read(&pstVswPort->stPort.stRef));
        MT_PRINT(MT_FMT_PTR,  "Vc Cb",         pstVswPort->pVcCb);
        MT_PRINT(MT_FMT_FUNC, "Release",       pstVswPort->pfnRelease);
        MT_PRINT(MT_FMT_U,    "In Data Type",  pstVswPort->u16InDataType);
        MT_PRINT(MT_FMT_U,    "Out Data Type", pstVswPort->u16OutDataType);

        nCount = 0;
        list_for_each_entry(pstTmpPort, &pstVswPort->stFwdHead, stFwdNode)
        {
            nCount++;
        }

        MT_PRINT(MT_FMT_U, "Fwd Port Count", nCount);

        if ( pstVswPort->pVcCb != NULL )
        {
            VC_CB_S *pstVcCb = (VC_CB_S *)pstVswPort->pVcCb;
            MT_PRINT("\r\nDisplay Video Converter Cb:");
            VC_DispMt(pstVcCb, ulExecId);
        }
    }
}

VOID FDD_MT_DispListenFwdInfo(FDD_LISTEN_PORT_S *pstListenPort, ULONG ulExecId)
{
    FDD_CHAN_S *pstRxChan;
    FDD_PORT_S *pstPort;
    CHAN_ID_S  *pstChanId;
    const char *szChanName;
    BOOL bFirst = TRUE;
    char szPairTmpBuf[32];
    char szTmpBuf[32];

    pstChanId = &pstListenPort->pstFddChan->stChanId;
    szChanName = VA_CHAN_GetChanTypeName(pstChanId->u16ChanType);
    snprintf(szTmpBuf, sizeof(szTmpBuf), "%s %u/%u/%u", szChanName, VA_CHAN_ARGS(pstChanId));

    list_for_each_entry(pstPort, &pstListenPort->stPortHead, stListenNode)
    {
        if ( bFirst )
        {
            bFirst = FALSE;
            MT_PRINT(FDD_MT_FWD_FMT, "Listen Chan", "", "Rx Chan");
        }

        pstRxChan = pstPort->pstRxChan;
        szChanName = VA_CHAN_GetChanTypeName(pstRxChan->stChanId.u16ChanType);
        snprintf(szPairTmpBuf, sizeof(szPairTmpBuf), "%s %u/%u/%u", szChanName, VA_CHAN_ARGS(&pstRxChan->stChanId));
        MT_PRINT(FDD_MT_FWD_FMT, szTmpBuf, "--->", szPairTmpBuf);
    }
}

VOID FDD_MT_DispListenPort(FDD_LISTEN_PORT_S *pstListenPort, ULONG ulExecId)
{
    MT_PRINT(MT_FMT_I,       "Fwd Port Count",  atomic_read(&pstListenPort->stPortCnt));
    MT_PRINT(MT_FMT_U64_HEX, "Listen Id",       pstListenPort->u64ListenId);
    MT_PRINT(MT_FMT_PTR,     "Release",         pstListenPort->pfnRelease);

    FDD_MT_DispVsw(pstListenPort, ulExecId);
}

VOID FDD_MT_DispChan(FDD_CHAN_S *pstChan, ULONG ulExecId)
{
    MT_PRINT("\r\nDisplay FDD Chan:");
    MT_PRINT(MT_FMT_U,   "Data Type",  pstChan->u16DataType);
    MT_PRINT(MT_FMT_PTR, "Ops",        pstChan->pstOps);
    MT_PRINT(MT_FMT_U,   "Ref",        atomic_read(&pstChan->stRef));


    if ( pstChan->pstListenPort )
    {
        MT_PRINT("\r\nDisplay Listen Port:");
        FDD_MT_DispListenPort(pstChan->pstListenPort, ulExecId);
    }

    MT_PRINT("\r\nDisplay Rx Queue:");
    FDD_MT_DispQueue(&pstChan->stRxQueue, ulExecId);
}

VOID FDD_MT_DispChanFwdInfo(ULONG ulExecId)
{
    FDD_PORT_S *pstPort;
    FDD_CHAN_S *pstChan;
    FDD_CHAN_S *pstListenChan;
    FDD_CB_S   *pstFddCb = GetFddCb();
    CHAN_ID_S  stChanId;
    const char *szChanName;
    BOOL bFirst = TRUE;
    char szPairTmpBuf[32];
    char szTmpBuf[32];

    if (MT_GetChanIdVal(ulExecId, MT_ITEM_ID_CHAN, &stChanId) < 0)
    {
        return;
    }

    szChanName = VA_CHAN_GetChanTypeName(stChanId.u16ChanType);

    MT_PRINT("\r\nDisplay chan %s %u/%u forward information:", szChanName, stChanId.u16PortId, stChanId.u16ChanId);

    mutex_lock(&pstFddCb->stMutex);
    pstChan = FDD_FindChan(&stChanId);
    if ( pstChan == NULL )
    {
        MT_PRINT("\r\n  Forward channel is not found!");
        mutex_unlock(&pstFddCb->stMutex);
        return;
    }

    snprintf(szTmpBuf, sizeof(szTmpBuf), "%s %u/%u/%u", szChanName, VA_CHAN_ARGS(&stChanId));

    MT_PRINT("\r\nDump all rx fwd:");
    list_for_each_entry(pstPort, &pstChan->stPortHead, stPairNode)
    {
        if ( bFirst )
        {
            bFirst = FALSE;
            MT_PRINT(FDD_MT_FWD_FMT, "Listen Chan", "", "Rx Chan");
        }

        pstListenChan = pstPort->pstListenChan;

        szChanName = VA_CHAN_GetChanTypeName(pstListenChan->stChanId.u16ChanType);
        snprintf(szPairTmpBuf, sizeof(szPairTmpBuf), "%s %u/%u/%u", szChanName, VA_CHAN_ARGS(&pstListenChan->stChanId));
        MT_PRINT(FDD_MT_FWD_FMT, szPairTmpBuf, "--->", szTmpBuf);
    }

    MT_PRINT("\r\nDump all tx fwd:");
    if ( pstChan->pstListenPort )
    {
        FDD_MT_DispListenFwdInfo(pstChan->pstListenPort, ulExecId);
        MT_PRINT("\r\nDump all vsw:");
        FDD_MT_DispVsw(pstChan->pstListenPort, ulExecId);
    }

    mutex_unlock(&pstFddCb->stMutex);
    return;
}

VOID FDD_MT_DispAllFwdInfo(ULONG ulExecId)
{
    FDD_LISTEN_PORT_S *pstListenPort;
    FDD_CHAN_S *pstListenChan;
    FDD_CHAN_S *pstRxChan;
    FDD_PORT_S *pstPort;
    FDD_CB_S   *pstFddCb = GetFddCb();
    const char *szChanName;
    char szPairTmpBuf[32];
    char szTmpBuf[32];
    U32 i;

    MT_PRINT("\r\nDump all forward information:");
    MT_PRINT(FDD_MT_FWD_FMT, "Listen Chan", "", "Rx Chan");

    mutex_lock(&pstFddCb->stMutex);

    for ( i = 0; i < FDD_LISTEN_HASH_SIZE; i++)
    {
        hlist_for_each_entry(pstListenPort, (pstFddCb->astListenHashTbl + i), stListenNode)
        {
            pstListenChan = pstListenPort->pstFddChan;
            szChanName = VA_CHAN_GetChanTypeName(pstListenChan->stChanId.u16ChanType);
            snprintf(szTmpBuf, sizeof(szTmpBuf), "%s " "%u/%u/%u", szChanName, VA_CHAN_ARGS(&pstListenChan->stChanId));

            list_for_each_entry(pstPort, &pstListenPort->stPortHead, stListenNode)
            {
                pstRxChan = pstPort->pstRxChan;
                szChanName = VA_CHAN_GetChanTypeName(pstRxChan->stChanId.u16ChanType);
                snprintf(szPairTmpBuf, sizeof(szPairTmpBuf), "%s %u/%u/%u", szChanName, VA_CHAN_ARGS(&pstRxChan->stChanId));
                MT_PRINT(FDD_MT_FWD_FMT, szTmpBuf, "--->", szPairTmpBuf);
            }
        }
    }

    mutex_unlock(&pstFddCb->stMutex);
    return;
}

int FDD_MT_RegCmd(MT_CB_S *pstMtCb)
{
    MT_DEF_TEXT_CMD_ITEM(stBuf, "", "buffer", "memory buffer");
    MT_DEF_TEXT_CMD_ITEM(stMgr, "", "mgr",    "manager");
    MT_DEF_TEXT_CMD_ITEM(stFwd, "", "fwd",    "forward info");

    MT_REG_CMD_LINE(FDD_MT_DispBuffMgr, MT_DEF_ITEM_DISP, MT_DEF_ITEM_MOD, &stBuf, &stMgr, MT_DEF_ITEM_CB);
    MT_REG_CMD_LINE(FDD_MT_DispChanFwdInfo, MT_DEF_ITEM_DISP, MT_DEF_ITEM_MOD, &stFwd, MT_DEF_ITEM_CHAN);
    MT_REG_CMD_LINE(FDD_MT_DispAllFwdInfo,  MT_DEF_ITEM_DISP, MT_DEF_ITEM_MOD, &stFwd, MT_DEF_ITEM_ALL);
    return 0;
}

