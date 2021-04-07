#ifndef __VC_MGR_H__
#define __VC_MGR_H__

#include "va_kern_pub.h"
#include "va_media_def.h"

// video data format converter
#define VC_MAX_NET_PKT_LEN      1420

enum
{
    VC_SUCCESS,
    VC_E_NOMEM,
    VC_E_INVALID,
    VC_E_NOBUF,
    VC_E_DROP,
    VC_E_FAILED,
};

struct tagVcer;
typedef LONG (*VC_OUTPUT_DATA_PF)(VOID *pPrivCb, U8 u8DataType, VOID *pData, U32 u32Len);
typedef VOID (*VC_FLUSH_DATA_PF)(VOID *pPrivCb, U8 u8DataType);
typedef U32  (*VC_GET_BUF_LEFT_LEN_PF)(VOID *pPrivCb, U8 u8DataType);

typedef struct tagVcParam
{
    VC_OUTPUT_DATA_PF      pfnOutputData;
    VC_FLUSH_DATA_PF       pfnFlush;
    VC_GET_BUF_LEFT_LEN_PF pfnGetBufLeftLen;
    U16     u16InDataType;
    U16     u16OutDataType;
    VOID    *pPrivCb;
}VC_PARAM_S;

typedef struct tagVcEsBufType
{
    BOOL bDrop;
    U8   u8EncFmtType;
    U8   u8DataType;
    U16  u16NextPktSeq;
    U32  u32FrameSeq;
    U32  u32OutBufLen;
    U8   *pu8TmpBuf;
    U32  u32TmpBufLen;
}VC_ES_BUF_TYPE_S;

typedef struct tagVcCb
{
    VC_OUTPUT_DATA_PF      pfnOutputData;
    VC_FLUSH_DATA_PF       pfnFlush;
    VC_GET_BUF_LEFT_LEN_PF pfnGetBufLeftLen;
    VOID  *pPrivCb;
    ULONG ulInPkts;
    ULONG ulErrPkts;
    ULONG ulOutPkts;
    struct tagVcer *pstVcer;
}VC_CB_S;

typedef struct tagVcer
{
    struct list_head stNode;
    U16  u16InDataType;
    U16  u16OutDataType;
    U32  u32CbSize;
    LONG (*pfnInit)(VC_CB_S *pstVcCb);
    VOID (*pfnDestroy)(VC_CB_S *pstVcCb);
    LONG (*pfnConvertProc)(VC_CB_S *pstVcCb, void *pData, U32 u32Len);
}VCER_S;

typedef struct tagVcMgr
{
    struct list_head stHead;
}VC_MGR_S;

extern void VC_RegVcer(VCER_S *pstVcer);
extern void VC_UnRegVcer(VCER_S *pstVcer);
extern VOID VC_ConvertData(VC_CB_S *pstVcCb, void *pData, U32 u32Len);
extern VOID VC_FlushEsData(VC_CB_S *pstVcCb, VC_ES_BUF_TYPE_S *pstBufType);
extern VOID VC_InitEsBufType(VC_ES_BUF_TYPE_S *pstBufType, U8 u8EncFmtType, U8 u8DataType);
extern VC_CB_S *VC_CreateVideoConverter(VC_PARAM_S *pstParam);
extern VOID VC_ReleaseVideoConverter(VC_CB_S *pstVcCb);
extern VOID VC_DispMt(VC_CB_S *pstVcCb, ULONG ulExecId);
extern LONG VC_CheckEsEncFormat(VC_ES_BUF_TYPE_S *pstBufType, ES_PKT_HDR_S *pstEsPkt);
extern LONG VC_ChkEsPkt(VC_ES_BUF_TYPE_S *pstBufType, ES_PKT_HDR_S *pstEsPkt);

#endif //__VC_MGR_H__
