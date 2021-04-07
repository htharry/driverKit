#include "va_usr_pub.h"
#include "va_media_def.h"
#include "mt_usr.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <poll.h>
#include "vn_def.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <time.h>
#include <pthread.h>

#define DRV_MEDIA_BUF_PRIV_RSVD_LEN        32

#pragma pack (4)

typedef struct tagPktHdr
{
    LE32  le32DTS;
    LE32  le32PTS;

    union
    {
        VA_PKT_BITS_INFO_S stInfo;
        U8 u8Info;
    };

    U8   u8FrmRate;
    U8   u8PictureSize;
    U8   u8StreamType;
    LE16 le16PktSeq;
    LE16 le16PktLen;        //fix me, it's to 32bit?
    LE32 le32FrmSeq;
    U8   au8Data[0];
}PKT_HDR_S;

/* 媒体数据缓冲区条目 */
typedef struct tagStreamDataItem
{
    union
    {
        U8    u8Flag;        /* 0x00表示可写，0x01表示可读 */
        U32   u32Magic;      /* magic标识，DRV_MAGIC_FLG */
    };

    union  /* 音、视频头部 */
    {
        PKT_HDR_S stEsPkt;     /* 音频/视频头部，DC解码时也要将这个参数传给DSP */
        U8           au8Rsvd[20];
    };

    CHAN_ID_S     stChanId;     /* 共享内存需要区别chan id */
    LE32          le32Len;      /* 整个包数据长度 */

    union /* 32字节保留字段区 */
    {
        U8        au8PrivRsvd[DRV_MEDIA_BUF_PRIV_RSVD_LEN];
    };

    U8    au8Data[0];
}STREAM_DATA_ITEM_S;

#pragma pack ()

typedef struct tagTxParam
{
    int     nFd;
    ULONG   ulCnt;
    U16     u16ChanId;
}TX_PARAM_S;

void MT_VT_SendFrame(U8 *pData, ULONG ulLen, U16 u16ChanId, PKT_HDR_S *pstPktHdr, U32 u32Seq)
{
    MEDIA_BUF_HDR_S stHdr = {0};
    ES_PKT_HDR_S *pstEsPkt;

    stHdr.le32Len = DRV_CPU_TO_LE32(ulLen);
    stHdr.stStorPktHdr.le32Sec  = 0;
    stHdr.stStorPktHdr.le32USec = 0;
    pstEsPkt = &stHdr.stStorPktHdr.stEsPkt;

    pstEsPkt->u8FrmRate     = 25;
    pstEsPkt->u8PictureSize = 1;
    pstEsPkt->u8EncFormat   = VA_ES_TYPE_H265;
    pstEsPkt->u8StreamType  = VA_STREAM_TYPE_ES;
    pstEsPkt->stInfo.u8BitFirst = 1;
    pstEsPkt->stInfo.u8BitEnd   = 1;
    pstEsPkt->stInfo.u8BitFrm   = pstPktHdr->stInfo.u8BitFrm;
    pstEsPkt->stInfo.u8BitType  = VA_BUF_TYPE_VIDEO;
    //pstEsPkt->le32DTS = pstPktHdr->le32DTS;
    //pstEsPkt->le32PTS = pstPktHdr->le32PTS;
    pstEsPkt->le32DTS = u32Seq * 1800;
    pstEsPkt->le32PTS = u32Seq * 1800;
    pstEsPkt->le32FrmSeq = pstPktHdr->le32FrmSeq;

    printf("pts %u\n", DRV_LE32_TO_CPU(pstEsPkt->le32PTS));

    VAA_PutData(VA_CHAN_TYPE_VENC, u16ChanId, &stHdr, pData, ulLen);
}


void *MT_VT_SendVideo (void *pArg)
{
    STREAM_DATA_ITEM_S stDataItem;
    TX_PARAM_S *pstParam = (TX_PARAM_S *)pArg;
    U8 *pu8Data;
    U32 u32Len;
    int nRet;
    int nFrameCnt = 0;
    struct timespec Req;

    do {
        nRet = read(pstParam->nFd, &stDataItem, sizeof(STREAM_DATA_ITEM_S));
        if ( nRet != sizeof(STREAM_DATA_ITEM_S) )
        {
            break;
        }

        u32Len = DRV_LE32_TO_CPU(stDataItem.le32Len);
        if ( u32Len == 0 )
        {
            break;
        }

        pu8Data = malloc(u32Len);
        if ( pu8Data == NULL )
        {
            break;
        }

        nRet = read(pstParam->nFd, pu8Data, u32Len);
        if ( (U32)nRet != u32Len )
        {
            break;
        }

        if ( nFrameCnt % 25 == 0 )
        {
            MT_VT_SendFrame(pu8Data, u32Len, pstParam->u16ChanId, &stDataItem.stEsPkt, nFrameCnt);
        }
        else
        {
            MT_VT_SendFrame(pu8Data, u32Len, pstParam->u16ChanId, &stDataItem.stEsPkt, nFrameCnt);
        }

        free(pu8Data);
        Req.tv_sec  = 0;
        Req.tv_nsec = 1000 * 1000 * 39;
        nanosleep(&Req, NULL);
        nFrameCnt++;
    } while(1);

    close(pstParam->nFd);
    printf("finish to send video data\n");
    return NULL;
}

U16 MT_VT_GetVideoType(ULONG ulExecId)
{
    char szStr[MT_STR_MAX_LEN];

    if ( MT_GetTextVal(ulExecId, "vt", szStr) )
    {
        MT_PRINT("\r\n Invalid input video type");
        return VA_STREAM_TYPE_ES;
    }

    if ( strcmp(szStr, "es") == 0 )
    {
        return VA_STREAM_TYPE_ES;
    }
    else if ( strcmp(szStr, "rtp") == 0 )
    {
        return VA_STREAM_TYPE_RTP;
    }
    else if ( strcmp(szStr, "3984") == 0 )
    {
        return VA_STREAM_TYPE_RFC3984;
    }
    else if ( strcmp(szStr, "ts") == 0 )
    {
        return VA_STREAM_TYPE_TS;
    }
    else if ( strcmp(szStr, "ps") == 0 )
    {
        return VA_STREAM_TYPE_PS;
    }
    else
    {
        return VA_STREAM_TYPE_ES;
    }
}

VOID MT_VT_SendVideoData(ULONG ulExecId)
{
    TX_PARAM_S stParam;
    CHAN_ID_S stListenChanId;
    ULONG ulCnt = 1;
    char szFileName[MT_STR_MAX_LEN];
    pthread_t Thread;
    int nFd;

    if (MT_GetChanIdVal(ulExecId, MT_ITEM_ID_CHAN, &stListenChanId))
    {
        MT_PRINT("\r\n Invalid input listen chan");
        return;
    }

    if (MT_GetIntVal(ulExecId, "Cnt", &ulCnt))
    {
        MT_PRINT("\r\n Invalid input frame len");
        return;
    }

    if (MT_GetStrVal(ulExecId, "fn", szFileName))
    {
        MT_PRINT("\r\n Invalid input file name");
        return;
    }

    nFd = open(szFileName, O_RDONLY);
    if ( nFd < 0 )
    {
        MT_PRINT("\r\n Failed to open file %s", szFileName);
        return;
    }

    stParam.nFd       = nFd;
    stParam.u16ChanId = stListenChanId.u16ChanId;
    stParam.ulCnt     = ulCnt;

    pthread_create(&Thread, NULL, MT_VT_SendVideo, &stParam);
    pthread_detach(Thread);

    return;
}

VOID MT_VT_RegTestMt(ULONG ulMtHndl)
{
    MT_DEF_STR_CMD_ITEM(stFile,    "fn", 1, 32,   "file name");
    MT_DEF_TEXT_CMD_ITEM(stVtEs,   "vt", "es",    "es video type");
    MT_DEF_TEXT_CMD_ITEM(stVtRtp,  "vt", "rtp",   "rtp video type");
    MT_DEF_TEXT_CMD_ITEM(stVt3984, "vt", "3984",  "rfc3984 video type");
    MT_DEF_TEXT_CMD_ITEM(stVtTs,   "vt", "ts",    "ts video type");
    MT_DEF_TEXT_CMD_ITEM(stVtPs,   "vt", "ps",    "ps video type");
    MT_DEF_INT_CMD_ITEM(stCnt,     "Cnt", 1, 100, "count");
    MT_CMD_ITEM_ARR_S stVtArr;

    MT_MERGE_ITEM_TO_ARR(&stVtArr, &stVtEs, &stVtRtp, &stVt3984, &stVtTs, &stVtPs);
    MT_REG_CMD_LINE(MT_VT_SendVideoData, MT_DEF_ITEM_SET, &stVtArr, MT_DEF_ITEM_CHAN, &stFile, &stCnt);
}

int main(void)
{
    ULONG ulMtHndl;
    struct pollfd stPollFd;
    INT iRet;
    char szPkt[256];
    int i;

    iRet = VAA_Init();
    if ( VA_IS_ERR(iRet) )
    {
        return -1;
    }

    ulMtHndl = MT_USR_Open("video_test", "video test every functions", MT_DBG_ALL, 0);
    if ( MT_USR_IS_INVAL_HNDL(ulMtHndl) )
    {
        printf("\r\nFailed to open mt\r\n");
        return -2;
    }

    for (i = 0; i < sizeof(szPkt); i++)
    {
        szPkt[i] = (char)i;
    }

    MT_VT_RegTestMt(ulMtHndl);
    while (1)
    {
        stPollFd.fd      = MT_USR_GetFd(ulMtHndl);
        stPollFd.events  = POLLIN;
        stPollFd.revents = 0;

        iRet = poll(&stPollFd, 1, 10 * 1000);
        MT_USR_ExecCmdLine(ulMtHndl);

        if ( iRet == 0 )
        {
        }
    }

    return 0;
}

