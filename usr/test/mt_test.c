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

static U32 gu32TestCnt;

VOID MT_TestCmdProcOfStat(ULONG ulExecId)
{
    static U32 u32Stat;

    MT_PRINT(MT_FMT_INT, "stat", u32Stat++);
    sleep(100);
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
    struct in_addr stIpAddr;

    if (MT_GetIpv4Val(ulExecId, "ip", &stIpAddr.s_addr))
    {
        MT_PRINT("\r\n Invalid input ip");
    }

    MT_PRINT(MT_FMT_STR, "ip address", inet_ntoa(stIpAddr));
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
    char szText[MT_NAME_MAX_LEN];

    if (MT_GetStrVal(ulExecId, "str", szText))
    {
        MT_PRINT("\r\n Invalid input str");
    }

    MT_PRINT(MT_FMT_STR, "my string", szText);
}

void MT_TestDispStr(ULONG ulExecId)
{
    char szText[MT_NAME_MAX_LEN];

    if (MT_GetStrVal(ulExecId, "str", szText))
    {
        MT_PRINT("\r\n Invalid input max str");
    }

    MT_PRINT(MT_FMT_STR, "my string", szText);
}

void MT_ConnectChan(ULONG ulExecId)
{
    CHAN_ID_S stListenChanId;
    CHAN_ID_S stRxChanId;
    MT_VAL_U  astVal[2];
    U32 u32ValArrNum = 2;
    INT nRet;

    if ( MT_GetAllValById(ulExecId, MT_ITEM_ID_CHAN, astVal, &u32ValArrNum) )
    {
        MT_PRINT("\r\n Invalid input chan");
        return;
    }

    if ( u32ValArrNum != 2 )
    {
        MT_PRINT("\r\n Invalid input chan");
        return;
    }

    stListenChanId = astVal[0].stChanId;
    stRxChanId     = astVal[1].stChanId;

#if 0
    if (MT_GetChanIdVal(ulExecId, MT_ITEM_ID_CHAN, &stListenChanId))
    {
        MT_PRINT("\r\n Invalid input listen chan");
        return;
    }

    if (MT_GetChanIdVal(ulExecId, "rxch", &stRxChanId))
    {
        MT_PRINT("\r\n Invalid input rx chan");
        return;
    }
#endif
    nRet = VAA_Connect(&stListenChanId, &stRxChanId);
    if ( VA_IS_ERR(nRet) )
    {
        MT_PRINT("\r\n Failed to connect chans, errno %d", errno);
    }

    return;
}

void MT_SendIFrame(U8 *pData, ULONG ulLen, U16 u16ChanId, U32 u32Dts)
{
    MEDIA_BUF_HDR_S stHdr = {0};
    ES_PKT_HDR_S *pstEsPkt;

    stHdr.le32Len = DRV_CPU_TO_LE32(ulLen);
    stHdr.stStorPktHdr.le32Sec  = 0;
    stHdr.stStorPktHdr.le32USec = 0;
    pstEsPkt = &stHdr.stStorPktHdr.stEsPkt;

    pstEsPkt->u8FrmRate     = 25;
    pstEsPkt->u8PictureSize = 1;
    pstEsPkt->u8EncFormat   = VA_ES_TYPE_H264;
    pstEsPkt->u8StreamType  = VA_STREAM_TYPE_ES;
    pstEsPkt->stInfo.u8BitFirst = 1;
    pstEsPkt->stInfo.u8BitEnd   = 1;
    pstEsPkt->stInfo.u8BitFrm   = VA_I_FRAME;
    pstEsPkt->stInfo.u8BitType  = VA_BUF_TYPE_VIDEO;
    pstEsPkt->le32DTS = DRV_CPU_TO_LE32(u32Dts);
    pstEsPkt->le32PTS = DRV_CPU_TO_LE32(u32Dts);

    VAA_PutData(VA_CHAN_TYPE_VENC, u16ChanId, &stHdr, pData, ulLen);
}

void MT_SendPFrame(U8 *pData, ULONG ulLen, U16 u16ChanId, U32 u32Dts, U32 u32FrmSeq)
{
    MEDIA_BUF_HDR_S stHdr = {0};
    ES_PKT_HDR_S *pstEsPkt;

    stHdr.le32Len = DRV_CPU_TO_LE32(ulLen);
    stHdr.stStorPktHdr.le32Sec  = 0;
    stHdr.stStorPktHdr.le32USec = 0;
    pstEsPkt = &stHdr.stStorPktHdr.stEsPkt;

    pstEsPkt->le16PktSeq    = 0;
    pstEsPkt->le32FrmSeq    = DRV_CPU_TO_LE32(u32FrmSeq);
    pstEsPkt->u8FrmRate     = 25;
    pstEsPkt->u8PictureSize = 1;
    pstEsPkt->u8EncFormat   = VA_ES_TYPE_H264;
    pstEsPkt->u8StreamType  = VA_STREAM_TYPE_ES;
    pstEsPkt->stInfo.u8BitFirst = 1;
    pstEsPkt->stInfo.u8BitEnd   = 1;
    pstEsPkt->stInfo.u8BitFrm   = VA_P_FRAME;
    pstEsPkt->stInfo.u8BitType  = VA_BUF_TYPE_VIDEO;
    pstEsPkt->le32DTS = DRV_CPU_TO_LE32(u32Dts);
    pstEsPkt->le32PTS = DRV_CPU_TO_LE32(u32Dts);

    VAA_PutData(VA_CHAN_TYPE_VENC, u16ChanId, &stHdr, pData, ulLen);
}

void MT_SendFrame(ULONG ulExecId)
{
    CHAN_ID_S stListenChanId;
    ULONG ulLen;
    ULONG i;
    U8 *pData;
    ULONG ulCnt = 1;

    if (MT_GetChanIdVal(ulExecId, MT_ITEM_ID_CHAN, &stListenChanId))
    {
        MT_PRINT("\r\n Invalid input listen chan");
        return;
    }

    if (MT_GetIntVal(ulExecId, MT_ITEM_ID_LEN, &ulLen))
    {
        MT_PRINT("\r\n Invalid input frame len");
        return;
    }

    if (MT_GetIntVal(ulExecId, "Cnt", &ulCnt))
    {
        MT_PRINT("\r\n Invalid input frame len");
        return;
    }

    pData = VA_Malloc(ulLen);
    if ( NULL == pData )
    {
        return;
    }

    for ( i = 0; i < ulLen; i++)
    {
        pData[i] = 'a' + (i % ('z' - 'a' + 1));
    }

    MT_SendIFrame(pData, ulLen, stListenChanId.u16ChanId, 180);
    for ( i = 1; i < ulCnt; i++)
    {
        MT_SendPFrame(pData, ulLen, stListenChanId.u16ChanId, 180 + 180 * i, i);
    }

    VA_Free(pData);

    return;
}

VOID MT_GetFrame(ULONG ulMtHndl)
{
    int nRet;
    U8  au8Buf[2 * VA_MB];
    MEDIA_BUF_HDR_S *pstHdr;

    pstHdr = (MEDIA_BUF_HDR_S *)au8Buf;

    nRet = VAA_GetData(VA_CHAN_TYPE_VDEC, 0, pstHdr, au8Buf + sizeof(MEDIA_BUF_HDR_S), VA_MB);
    if ( VA_IS_ERR(nRet) )
    {
        return;
    }

    MT_DBG_PKT_PRINT(ulMtHndl, "In Frame", au8Buf, pstHdr->le32Len);
}

U16 MT_GetVideoType(ULONG ulExecId)
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

VOID MT_AddUdpVnNode(ULONG ulExecId)
{
    VN_NODE_PARAM_S stNodeParam;
    struct sockaddr_in stAddr;
    MT_VAL_U  astIpVal[2];
    MT_VAL_U  astPortVal[2];
    U32 u32ValArrNum = 2;
    int nFd;
    int nRet;

    if ( MT_GetAllValById(ulExecId, MT_ITEM_ID_IP, astIpVal, &u32ValArrNum) )
    {
        MT_PRINT("\r\n Invalid input ip");
        return;
    }

    if ( u32ValArrNum != 2 )
    {
        MT_PRINT("\r\n Invalid input ip");
        return;
    }

    if ( MT_GetAllValById(ulExecId, "port", astPortVal, &u32ValArrNum) )
    {
        MT_PRINT("\r\n Invalid input port");
        return;
    }

    if ( u32ValArrNum != 2 )
    {
        MT_PRINT("\r\n Invalid input port");
        return;
    }

    nFd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if ( nFd < 0 )
    {
        MT_PRINT("\r\n Failed to create udp socket");
        return;
    }

    stAddr.sin_family = AF_INET;
    stAddr.sin_addr.s_addr = astIpVal[0].be32Ip;
    stAddr.sin_port = htons((U16)astPortVal[0].ulVal);
    nRet = bind(nFd, (struct sockaddr *)(&stAddr), sizeof(stAddr));
    if ( nRet < 0 )
    {
        MT_PRINT("\r\n Failed to bind udp socket");
        close(nFd);
        return;
    }

    stNodeParam.nFd              = nFd;
    stNodeParam.u16DataType      = MT_GetVideoType(ulExecId);
    stNodeParam.u16TransportMode = VN_TRANSPORT_UDP;
    stNodeParam.u8Dir   = VN_TX|VN_RX;
    stNodeParam.u32Flag = 0;

    stNodeParam.stLocalAddr.be16Port    = stAddr.sin_port;
    stNodeParam.stLocalAddr.be32IpAddr  = stAddr.sin_addr.s_addr;
    stNodeParam.stRemoteAddr.be16Port   = htons((U16)astPortVal[1].ulVal);
    stNodeParam.stRemoteAddr.be32IpAddr = astIpVal[1].be32Ip;

    nRet = VAA_AddVnNode(&stNodeParam);
    if ( VA_IS_ERR(nRet) )
    {
        MT_PRINT("\r\n Failed to create udp video network channel");
        close(nFd);
    }
    else
    {
        MT_PRINT("\r\n Succeed to create udp video network channel");
        MT_PRINT(MT_FMT_CHAN, "Chan Id", VA_CHAN_ARGS(&stNodeParam.stChanId));
        MT_PRINT(MT_FMT_I,    "Fd",      nFd);
    }

    return;
}

VOID MT_AddTcpVnNode(ULONG ulExecId)
{
    VN_NODE_PARAM_S stNodeParam;
    struct sockaddr_in stAddr;
    MT_VAL_U  astIpVal[2];
    MT_VAL_U  astPortVal[2];
    U32 u32ValArrNum = 2;
    int nFd;
    int nRet;

    if ( MT_GetAllValById(ulExecId, MT_ITEM_ID_IP, astIpVal, &u32ValArrNum) )
    {
        MT_PRINT("\r\n Invalid input ip");
        return;
    }

    if ( u32ValArrNum != 2 )
    {
        MT_PRINT("\r\n Invalid input ip");
        return;
    }

    if ( MT_GetAllValById(ulExecId, "port", astPortVal, &u32ValArrNum) )
    {
        MT_PRINT("\r\n Invalid input port");
        return;
    }

    if ( u32ValArrNum != 2 )
    {
        MT_PRINT("\r\n Invalid input port");
        return;
    }

    nFd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if ( nFd < 0 )
    {
        MT_PRINT("\r\n Failed to create tcp socket");
        return;
    }

    stAddr.sin_family = AF_INET;
    stAddr.sin_addr.s_addr = astIpVal[0].be32Ip;
    stAddr.sin_port = htons((U16)astPortVal[0].ulVal);

    nRet = bind(nFd, (struct sockaddr *)(&stAddr), sizeof(stAddr));
    if ( nRet < 0 )
    {
        MT_PRINT("\r\n Failed to bind tcp socket");
        close(nFd);
        return;
    }

    stNodeParam.stLocalAddr.be16Port   = stAddr.sin_port;
    stNodeParam.stLocalAddr.be32IpAddr = stAddr.sin_addr.s_addr;

    stAddr.sin_addr.s_addr = astIpVal[1].be32Ip;
    stAddr.sin_port = htons((U16)astPortVal[1].ulVal);
    nRet = connect(nFd, (struct sockaddr *)(&stAddr), sizeof(stAddr));
    if ( nRet < 0 )
    {
        MT_PRINT("\r\n Failed to connect tcp socket");
        close(nFd);
        return;
    }

    stNodeParam.nFd              = nFd;
    stNodeParam.u16DataType      = MT_GetVideoType(ulExecId);
    stNodeParam.u16TransportMode = VN_TRANSPORT_TCP;
    stNodeParam.u8Dir   = VN_TX|VN_RX;
    stNodeParam.u32Flag = 0;

    stNodeParam.stRemoteAddr.be32IpAddr = stAddr.sin_addr.s_addr;
    stNodeParam.stRemoteAddr.be16Port   = stAddr.sin_port;

    nRet = VAA_AddVnNode(&stNodeParam);
    if ( VA_IS_ERR(nRet) )
    {
        MT_PRINT("\r\n Failed to create tcp video network channel");
        close(nFd);
    }
    else
    {
        MT_PRINT("\r\n Succeed to create tcp video network channel");
        MT_PRINT(MT_FMT_CHAN, "Chan Id", VA_CHAN_ARGS(&stNodeParam.stChanId));
        MT_PRINT(MT_FMT_I,    "Fd",      nFd);
    }

    return;
}

VOID MT_RegTestMt(ULONG ulMtHndl)
{
    MT_DEF_INT_CMD_ITEM(stVal, "val", 0, 100, "value");
    MT_DEF_STR_CMD_ITEM(stStr, "str", 1, 5,   "test string");
    MT_DEF_STR_CMD_ITEM(stMaxStr, "str", 1, 32, "test string");
    MT_DEF_TEXT_CMD_ITEM(stCbcText, "cbc", "cbc", "test cbc text");
    MT_DEF_TEXT_CMD_ITEM(stRxChanPre, "",     "RxChan",  "rx channel");
    MT_DEF_TEXT_CMD_ITEM(stListenChanPre, "", "ListenChan", "listen(tx) channel");
    MT_DEF_TEXT_CMD_ITEM(stConn, "", "connect", "connect both channels");
    MT_DEF_TEXT_CMD_ITEM(stSend, "", "send", "send");
    MT_DEF_TEXT_CMD_ITEM(stFrame, "", "frame", "video encode frame");
    MT_DEF_HEX_CMD_ITEM(stLen, MT_ITEM_ID_LEN, 1, 10 * 1024 * 1024, "length");
    MT_DEF_TEXT_CMD_ITEM(stUdp, "udp", "udp", "udp");
    MT_DEF_TEXT_CMD_ITEM(stTcp, "tcp", "tcp", "tcp");
    MT_DEF_TEXT_CMD_ITEM(stLoc, "loc", "local",  "local host");
    MT_DEF_TEXT_CMD_ITEM(stRem, "rem", "remote", "remote host");
    MT_DEF_INT_CMD_ITEM(stPort, "port", 10000, 50000, "network port");
    //MT_DEF_TEXT_CMD_ITEM(stTestCb, "abc", "cb", "value");
    MT_DEF_TEXT_CMD_ITEM(stVtEs,   "vt", "es",   "es video type");
    MT_DEF_TEXT_CMD_ITEM(stVtRtp,  "vt", "rtp",  "rtp video type");
    MT_DEF_TEXT_CMD_ITEM(stVt3984, "vt", "3984", "rfc3984 video type");
    MT_DEF_TEXT_CMD_ITEM(stVtTs,   "vt", "ts",   "ts video type");
    MT_DEF_TEXT_CMD_ITEM(stVtPs,   "vt", "ps",   "ps video type");
    MT_DEF_INT_CMD_ITEM(stCnt, "Cnt", 1, 100, "count");
    MT_CMD_ITEM_ARR_S stTestItemArr;
    MT_CMD_ITEM_ARR_S stVtArr;

    MT_MERGE_ITEM_TO_ARR(&stTestItemArr, MT_DEF_ITEM_CB, MT_DEF_ITEM_STAT);
    MT_MERGE_ITEM_TO_ARR(&stVtArr, &stVtEs, &stVtRtp, &stVt3984, &stVtTs, &stVtPs);

    MT_REG_CMD_LINE(MT_TestCmdProcOfStat, MT_DEF_ITEM_DISP, &stTestItemArr);
    //MT_REG_CMD_LINE(MT_TestCmdProcOfStat, MT_DEF_ITEM_DISP, &stTestCb);
    MT_REG_CMD_LINE(MT_TestCmdProcOfStat, MT_DEF_ITEM_DISP, &stCbcText, MT_DEF_ITEM_CB);
    MT_REG_CMD_LINE(MT_TestDispIp,        MT_DEF_ITEM_DISP, MT_DEF_ITEM_IP);
    MT_REG_CMD_LINE(MT_TestReadVal,       MT_DEF_ITEM_READ, &stVal);
    MT_REG_CMD_LINE(MT_TestReadStr,       MT_DEF_ITEM_READ, MT_DEF_ITEM_STAT, &stStr);
    MT_REG_CMD_LINE(MT_TestDispChan,      MT_DEF_ITEM_DISP, MT_DEF_ITEM_CHAN);
    MT_REG_CMD_LINE(MT_TestDispStr,       MT_DEF_ITEM_DISP, MT_DEF_ITEM_CB, &stMaxStr);
    MT_REG_CMD_LINE(MT_ConnectChan,       MT_DEF_ITEM_SET, &stConn, &stListenChanPre, MT_DEF_ITEM_CHAN,  &stRxChanPre, MT_DEF_ITEM_CHAN);
    MT_REG_CMD_LINE(MT_SendFrame,         &stSend, &stListenChanPre, MT_DEF_ITEM_CHAN, &stFrame, &stLen, &stCnt);
    MT_REG_CMD_LINE(MT_AddUdpVnNode,      MT_DEF_ITEM_SET, &stVtArr, &stUdp, &stLoc, MT_DEF_ITEM_IP, &stPort, &stRem, MT_DEF_ITEM_IP, &stPort);
    MT_REG_CMD_LINE(MT_AddTcpVnNode,      MT_DEF_ITEM_SET, &stVtArr, &stTcp, &stLoc, MT_DEF_ITEM_IP, &stPort, &stRem, MT_DEF_ITEM_IP, &stPort);
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

    ulMtHndl = MT_USR_Open("test", "test every functions", MT_DBG_ALL, 0);
    if ( MT_USR_IS_INVAL_HNDL(ulMtHndl) )
    {
        printf("\r\nFailed to open mt\r\n");
        return -2;
    }

    for (i = 0; i < sizeof(szPkt); i++)
    {
        szPkt[i] = (char)i;
    }

    MT_RegTestMt(ulMtHndl);
    while (1)
    {
        stPollFd.fd      = MT_USR_GetFd(ulMtHndl);
        stPollFd.events  = POLLIN;
        stPollFd.revents = 0;

        iRet = poll(&stPollFd, 1, 10 * 1000);
        MT_USR_ExecCmdLine(ulMtHndl);

        if ( iRet == 0 )
        {
            //MT_DBG_INFO_PRINT(ulMtHndl, MT_DBG_INT, "test val", gu32TestCnt++);
            //MT_DBG_PKT_PRINT(ulMtHndl, "test1", (U8 *)szPkt, sizeof(szPkt));
            //MT_DBG_PKT_PRINT(ulMtHndl, "test2", (U8 *)szPkt, 32);
            MT_GetFrame(ulMtHndl);
            gu32TestCnt++;
        }
    }

    return 0;
}
