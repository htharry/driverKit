#include "vaa_api.h"
#include "va_media_def.h"
#include "vn_def.h"

int VAA_GetChanFd(U16 u16ChanType, U16 u16ChanId)
{
    VAA_CHAN_TYPE_MAP_S *pstChanMap;

    pstChanMap = gstVaaApiCb.astChanMapTbl + u16ChanType;
    if ( pstChanMap->stCap.u16TotChanNum == 0 || pstChanMap->pstChanTbl == NULL || u16ChanId > pstChanMap->stCap.u16TotChanNum )
    {
        return VA_INVALID_FD;
    }

    return pstChanMap->pstChanTbl[u16ChanId].nFd;
}

int VAA_AddVnNode(VN_NODE_PARAM_S *pstNodeParam)
{
    CHAN_ID_S stVnChan =
    {
        .u16ChanType = VA_CHAN_TYPE_NET,
        .u16SlotId   = 0,
        .u16PortId   = 0,
        .u16ChanId   = 0,
    };

    VAA_CP_IO_CMD(&stVnChan, VN_IOCTL_ADD_NODE, pstNodeParam, "Failed to add vn node");
    return VA_SUCCESS;
}

int VAA_DelVnNode(CHAN_ID_S *pstChanId)
{
    VAA_CP_IO_CMD(pstChanId, VN_IOCTL_ADD_NODE, pstChanId, "Failed to del vn node");
    return VA_SUCCESS;
}

int VAA_Connect(CHAN_ID_S *pstListenChanId, CHAN_ID_S *pstRxChanId)
{
    CONN_PARAM_S stConnParam;

    stConnParam.stListenChanId = *pstListenChanId;
    stConnParam.stRxChanId     = *pstRxChanId;

    VAA_CP_IO_CMD(pstRxChanId, CP_IOCTL_CHAN_CONN, &stConnParam, "Failed to connect chans");

    return VA_SUCCESS;
}

int VAA_DisConnect(CHAN_ID_S *pstListenChanId, CHAN_ID_S *pstRxChanId)
{
    CONN_PARAM_S stConnParam;

    stConnParam.stListenChanId = *pstListenChanId;
    stConnParam.stRxChanId     = *pstRxChanId;

    VAA_CP_IO_CMD(pstRxChanId, CP_IOCTL_CHAN_DISCONN, &stConnParam, "Failed to disconnect chans");

    return VA_SUCCESS;
}

int VAA_PutData(U16 u16ChanType, U16 u16ChanId, VOID *pHead, VOID *pData, U32 u32DataLen)
{
    VA_DATA_BUF_S stData;
    LONG lRet;
    int nFd;

    nFd = VAA_GetChanFd(u16ChanType, u16ChanId);
    if ( nFd < 0 )
    {
        return VA_E_NO_ENT;
    }

    stData.pData  = pData;
    stData.pHead  = pHead;
    stData.u32Len = u32DataLen;

    lRet = ioctl(nFd, VK_IOCTL_PUT_DATA, &stData);
    if ( lRet < 0 )
    {
        return VA_E_SYS_FAILED;
    }

    return VA_SUCCESS;
}

int VAA_GetData(U16 u16ChanType, U16  u16ChanId, VOID *pHead, VOID *pData, U32 u32DataLen)
{
    VA_DATA_BUF_S stData;
    LONG lRet;
    int nFd;

    nFd = VAA_GetChanFd(u16ChanType, u16ChanId);
    if ( nFd < 0 )
    {
        return VA_E_NO_ENT;
    }

    stData.pData  = pData;
    stData.pHead  = pHead;
    stData.u32Len = u32DataLen;

    lRet = ioctl(nFd, VK_IOCTL_GET_DATA, &stData);
    if ( lRet < 0 )
    {
        return VA_E_SYS_FAILED;
    }

    return VA_SUCCESS;
}

int VAA_MediaInit(void)
{
    VAA_CHAN_TYPE_MAP_S *pstChanMap;
    VAA_CHAN_CB_S *pstChan;
    CHAN_ID_S *pstChanId;
    LONG lRet;
    U32 i, j;

    for ( i = VA_CHAN_TYPE_MEDIA_START; i < VA_CHAN_TYPE_MEDIA_NUM; i++)
    {
        pstChanMap = gstVaaApiCb.astChanMapTbl + i;
        pstChanMap->stCap.u16ChanType = i;

        VAA_GetChanCap(&pstChanMap->stCap);
        if ( pstChanMap->stCap.u16TotChanNum == 0 )
        {
            continue;
        }

        pstChanMap->pstChanTbl = VA_Malloc(sizeof(VAA_CHAN_CB_S) * pstChanMap->stCap.u16TotChanNum);
        VA_MEM_ZERO(pstChanMap->pstChanTbl, sizeof(VAA_CHAN_CB_S) * pstChanMap->stCap.u16TotChanNum);

        for ( j = 0; j < pstChanMap->stCap.u16TotChanNum; j++)
        {
            pstChan = pstChanMap->pstChanTbl + j;
            pstChan->nFd = -1;
        }

        for ( j = 0; j < pstChanMap->stCap.u16TotChanNum; j++)
        {
            pstChan   = pstChanMap->pstChanTbl + j;
            pstChanId = &pstChan->stChanId;

            pstChanId->u16ChanType = i;
            pstChanId->u16SlotId   = 0;
            pstChanId->u16PortId   = j / pstChanMap->stCap.u16ChanNumPerPort;
            pstChanId->u16ChanId   = j % pstChanMap->stCap.u16ChanNumPerPort;

            pstChan->nFd = VAA_Open(VA_VK_PATH);
            if ( pstChan->nFd < 0 )
            {
                VA_LOG_ERR("no media device");
                VAA_MediaDeInit();
            }

            lRet = ioctl(pstChan->nFd, VK_IOCTL_BIND_FD, pstChanId);
            if ( lRet < 0 )
            {
                VA_LOG_ERR("Failed to bind " VA_CHAN_FMT, VA_CHAN_ARGS(pstChanId));
                return VA_E_SYS_FAILED;
            }
        }
    }

    return VA_SUCCESS;
}

void VAA_MediaDeInit(void)
{
    VAA_CHAN_TYPE_MAP_S *pstChanMap;
    VAA_CHAN_CB_S *pstChan;
    U32 i, j;

    pstChanMap = gstVaaApiCb.astChanMapTbl;
    for ( i = VA_CHAN_TYPE_MEDIA_START; i < VA_CHAN_TYPE_MEDIA_NUM; i++)
    {
        if ( pstChanMap->stCap.u16TotChanNum == 0 || pstChanMap->pstChanTbl == NULL )
        {
            continue;
        }

        for ( j = 0; j < pstChanMap->stCap.u16TotChanNum; j++)
        {
            pstChan = pstChanMap->pstChanTbl + j;
            if (pstChan->nFd >= 0)
            {
                close(pstChan->nFd);
            }
        }

        VA_Free(pstChanMap->pstChanTbl);
        VA_CB_ZERO(pstChanMap);
    }

    return;
}


