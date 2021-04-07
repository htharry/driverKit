#include "vc_mgr.h"

VC_MGR_S gstVcMgr =
{
    .stHead = LIST_HEAD_INIT(gstVcMgr.stHead),
};

void VC_RegVcer(VCER_S *pstVcer)
{
    list_add(&pstVcer->stNode, &gstVcMgr.stHead);
}

void VC_UnRegVcer(VCER_S *pstVcer)
{
    list_del(&pstVcer->stNode);
}

static VCER_S *VC_FindVcer(U16 u16InDataType, U16 u16OutDataType)
{
    VCER_S *pstVcer;

    list_for_each_entry(pstVcer, &gstVcMgr.stHead, stNode)
    {
        if ( pstVcer->u16InDataType == u16InDataType && pstVcer->u16OutDataType == u16OutDataType )
        {
            return pstVcer;
        }
    }

    return NULL;
}

VOID VC_ConvertData(VC_CB_S *pstVcCb, void *pData, U32 u32Len)
{
    LONG lRet;

    pstVcCb->ulInPkts++;

    lRet = pstVcCb->pstVcer->pfnConvertProc(pstVcCb, pData, u32Len);
    if ( lRet != VC_SUCCESS )
    {
        pstVcCb->ulErrPkts++;
    }

    return;
}

VOID VC_FlushEsData(VC_CB_S *pstVcCb, VC_ES_BUF_TYPE_S *pstBufType)
{
    pstVcCb->pfnFlush(pstVcCb->pPrivCb, pstBufType->u8DataType);
    pstVcCb->ulOutPkts++;
    pstBufType->u32OutBufLen = 0;
}

VOID VC_DispMt(VC_CB_S *pstVcCb, ULONG ulExecId)
{
    MT_PRINT("\r\nDisplay Convert Info:");
    MT_PRINT(MT_FMT_UL,    "Err Pkts",           pstVcCb->ulErrPkts);
    MT_PRINT(MT_FMT_UL,    "Out Pkts",           pstVcCb->ulOutPkts);
    MT_PRINT(MT_FMT_UL,    "In  Pkts",           pstVcCb->ulInPkts);
}

VC_CB_S *VC_CreateVideoConverter(VC_PARAM_S *pstParam)
{
    VC_CB_S *pstVcCb;
    VCER_S  *pstVcer;
    LONG lRet;

    pstVcer = VC_FindVcer(pstParam->u16InDataType, pstParam->u16OutDataType);
    if ( pstVcer == NULL )
    {
        VA_LOG_ERR("Failed to find vcer, In Data Type %u Out Data Type %u", pstParam->u16InDataType, pstParam->u16OutDataType);
        return ERR_PTR(-ENOMEM);
    }

    pstVcCb = (VC_CB_S *)VA_Zmalloc(pstVcer->u32CbSize);
    if ( pstVcCb == NULL )
    {
        return ERR_PTR(-ENOMEM);
    }

    pstVcCb->pfnOutputData    = pstParam->pfnOutputData;
    pstVcCb->pfnFlush         = pstParam->pfnFlush;
    pstVcCb->pfnGetBufLeftLen = pstParam->pfnGetBufLeftLen;
    pstVcCb->pPrivCb = pstParam->pPrivCb;
    pstVcCb->pstVcer = pstVcer;

    lRet = pstVcer->pfnInit(pstVcCb);
    if ( lRet != 0 )
    {
        VA_Free(pstVcCb);
        return ERR_PTR(-lRet);
    }

    return pstVcCb;
}

VOID VC_ReleaseVideoConverter(VC_CB_S *pstVcCb)
{
    VCER_S *pstVcer = pstVcCb->pstVcer;

    pstVcer->pfnDestroy(pstVcCb);
    VA_Free(pstVcCb);
}

VOID VC_InitEsBufType(VC_ES_BUF_TYPE_S *pstBufType, U8 u8EncFmtType, U8 u8DataType)
{
    pstBufType->bDrop         = TRUE;
    pstBufType->u8EncFmtType  = u8EncFmtType;
    pstBufType->u8DataType    = u8DataType;
    pstBufType->u16NextPktSeq = 0;
    pstBufType->u32FrameSeq   = 0;
    pstBufType->u32OutBufLen  = 0;
    pstBufType->pu8TmpBuf     = NULL;
}

