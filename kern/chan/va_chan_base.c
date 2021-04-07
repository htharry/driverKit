#include "va_chan_base.h"
#include "va_media_def.h"
#include "va_cp.h"

VA_CHAN_MGR_S gstVaChanMgr;

extern MT_CB_S gstVaChanMt;
extern int  VA_CHAN_RegMt(void);
extern void VA_CHAN_UnRegMt(void);
extern int VA_CHAN_MT_RegCmd(VA_CHAN_TYPE_CB_S *pstChanTypeCb);

#if 0
#endif

int VA_ChanIndx2ChanId(U16 u16ChanType, U32 u32ChanIndx, CHAN_ID_S *pstChanId)
{
    VA_CHAN_PORT_TBL_S *pstPortTbl;
    VA_CHAN_PER_PORT_S *pstPorts;
    U32 u32LeftChanIdx = u32ChanIndx;
    U32 i;

    if ( u16ChanType >= VA_CHAN_TYPE_NUM )
    {
        return -EINVAL;
    }

    pstPortTbl = gstVaChanMgr.stSelfSlot.astTypePortTbl + u16ChanType;
    pstPorts   = pstPortTbl->pstPorts;
    if ( u32ChanIndx >= pstPortTbl->u32TotChanNum || pstPorts == NULL )
    {
        return -ENOENT;
    }

    for ( i = 0; i < pstPortTbl->u32PortNum; i++)
    {
        if ( u32LeftChanIdx >= pstPorts[i].u32ChanNum )
        {
            
u32LeftChanIdx -= pstPorts[i].u32ChanNum;
            continue;
        }

        break;
    }

    pstChanId->u16ChanType = u16ChanType;
    pstChanId->u16SlotId   = 0;
    pstChanId->u16PortId   = i;
    pstChanId->u16ChanId   = u32LeftChanIdx;

    return 0;
}


LONG VA_BindChan(struct file *pstFile, CHAN_ID_S *pstChanId)
{
    FDD_CHAN_S *pstChan;

    if ( pstFile->private_data )
    {
        return -EEXIST;
    }

    pstChan = VA_GetChan(pstChanId);
    if ( pstChan == NULL )
    {
        return -ENOENT;
    }

    pstFile->private_data = pstChan;
    return 0;
}

VOID VA_ReleaseChan(struct file *pstFile)
{
    FDD_CHAN_S *pstChan;

    if ( pstFile->private_data )
    {
        pstChan = (FDD_CHAN_S *)pstFile->private_data;
        FDD_PutChan(pstChan);
    }
}

FDD_CHAN_S *VA_GetChan(CHAN_ID_S *pstChanId)
{
    VA_CHAN_PORT_TBL_S *pstPortTbl;
    VA_CHAN_PER_PORT_S *pstPorts;
    FDD_CHAN_S *pstChan = NULL;

    if ( pstChanId->u16ChanType >= VA_CHAN_TYPE_NUM )
    {
        return NULL;
    }

    pstPortTbl = gstVaChanMgr.stSelfSlot.astTypePortTbl + pstChanId->u16ChanType;
    pstPorts   = pstPortTbl->pstPorts;
    if ( pstPorts != NULL )
    {
        if ( (pstChanId->u16PortId < pstPortTbl->u32PortNum) &&
             (pstChanId->u16ChanId < pstPorts[pstChanId->u16PortId].u32ChanNum) &&
             (pstPorts[pstChanId->u16PortId].ppstChanTbl != NULL ) &&
             (pstPorts[pstChanId->u16PortId].ppstChanTbl[pstChanId->u16ChanId] != NULL) )
        {
            pstChan = pstPorts[pstChanId->u16PortId].ppstChanTbl[pstChanId->u16ChanId];
            FDD_GetChan(pstChan);
        }
    }

    return pstChan;
}

LONG VA_ConnectChan(VA_IO_CMD_S *pstIoCmd)
{
    CONN_PARAM_S stConnParam;
    LONG lRet;

    if ( copy_from_user(&stConnParam, pstIoCmd->pParam, sizeof(CONN_PARAM_S)) )
    {
        return -EFAULT;
    }

    lRet = FDD_ConnectChanById(&stConnParam.stListenChanId, &stConnParam.stRxChanId);
    return lRet;
}

LONG VA_DisconnectChan(VA_IO_CMD_S *pstIoCmd)
{
    CONN_PARAM_S stConnParam;
    LONG lRet;

    if ( copy_from_user(&stConnParam, pstIoCmd->pParam, sizeof(CONN_PARAM_S)) )
    {
        return -EFAULT;
    }

    lRet = FDD_DisconnectChanById(&stConnParam.stListenChanId, &stConnParam.stRxChanId);
    return lRet;

}

LONG VA_GblIoctlCmd(U32 u32Cmd, VA_IO_CMD_S *pstIoCmd)
{
    LONG lRet;

    switch ( u32Cmd )
    {
        case CP_IOCTL_CHAN_CONN :
            lRet = VA_ConnectChan(pstIoCmd);
            break;
        case CP_IOCTL_CHAN_DISCONN :
            lRet = VA_DisconnectChan(pstIoCmd);
            break;
        default:
            lRet = -EAGAIN;
            break;
    }

    return lRet;
}

#if 0
#endif

VOID __VA_CHAN_Destroy(FDD_CHAN_S *pstChan)
{
    FDD_ReleaseChan(pstChan);
}

VOID VA_CHAN_Destroy(CHAN_ID_S *pstChanId)
{
    VA_CHAN_PORT_TBL_S *pstPortTbl;
    VA_CHAN_PER_PORT_S *pstPorts;
    FDD_CHAN_S *pstChan;

    if ( pstChanId->u16ChanType >= VA_CHAN_TYPE_NUM )
    {
        return;
    }

    pstPortTbl = gstVaChanMgr.stSelfSlot.astTypePortTbl + pstChanId->u16ChanType;
    pstPorts   = pstPortTbl->pstPorts;
    if ( pstPorts == NULL )
    {
        return;
    }

    if ( (pstChanId->u16PortId < pstPortTbl->u32PortNum) &&
         (pstChanId->u16ChanId < pstPorts[pstChanId->u16PortId].u32ChanNum) &&
         (pstPorts[pstChanId->u16PortId].ppstChanTbl != NULL ) )
    {
        pstChan = pstPorts[pstChanId->u16PortId].ppstChanTbl[pstChanId->u16ChanId];
        if ( pstChan )
        {
            __VA_CHAN_Destroy(pstChan);
            pstPorts[pstChanId->u16PortId].ppstChanTbl[pstChanId->u16ChanId] = NULL;
        }
    }

   return;
}

FDD_CHAN_S *__VA_CHAN_Init(CHAN_ID_S *pstChanId)
{
    FDD_CHAN_S *pstChan;
    U32 u32CbSize;
    int iRet;

    u32CbSize = VA_CHAN_GetChanCbSize(pstChanId->u16ChanType);

    pstChan = VA_Zmalloc(u32CbSize);
    if ( pstChan == NULL )
    {
        return ERR_PTR(-ENOMEM);
    }

    pstChan->stChanId = *pstChanId;
    iRet = FDD_InitChan(pstChan, &pstChan->stChanId, VA_CHAN_GetFddOps(pstChanId->u16ChanType));
    if ( iRet < 0 )
    {
        VA_Free(pstChan);
        VA_LOG_ERR("Failed to create chan " VA_CHAN_FMT ", iRet %d", VA_CHAN_ARGS(pstChanId), iRet);
        return ERR_PTR(iRet);
    }

    return pstChan;
}

FDD_CHAN_S *VA_CHAN_Init(CHAN_ID_S *pstChanId)
{
    VA_CHAN_PORT_TBL_S *pstPortTbl;
    VA_CHAN_PER_PORT_S *pstPorts;
    FDD_CHAN_S *pstChan;

    if ( pstChanId->u16ChanType >= VA_CHAN_TYPE_NUM )
    {
        return NULL;
    }

    pstPortTbl = gstVaChanMgr.stSelfSlot.astTypePortTbl + pstChanId->u16ChanType;
    pstPorts   = pstPortTbl->pstPorts;
    if ( pstPorts == NULL )
    {
        return NULL;
    }

    if ( (pstChanId->u16PortId < pstPortTbl->u32PortNum) &&
         (pstChanId->u16ChanId < pstPorts[pstChanId->u16PortId].u32ChanNum) &&
         (pstPorts[pstChanId->u16PortId].ppstChanTbl != NULL ) )
    {
        pstChan = pstPorts[pstChanId->u16PortId].ppstChanTbl[pstChanId->u16ChanId];
        if ( pstChan )
        {
            VA_LOG_ERR(VA_CHAN_FMT "has already been alloced", VA_CHAN_ARGS(pstChanId));
            return NULL;
        }

        pstChan = __VA_CHAN_Init(pstChanId);
        if ( IS_ERR(pstChan) )
        {
            return NULL;
        }

        pstPorts[pstChanId->u16PortId].ppstChanTbl[pstChanId->u16ChanId] = pstChan;
        return pstChan;
    }

    return NULL;
}

VOID VA_CHAN_DestroyChanTbl(FDD_CHAN_S **ppstChanTbl, U32 u32ChanNum, U16 u16ChanType)
{
    U32 i;

    for ( i = 0; i < u32ChanNum; i++)
    {
        if ( ppstChanTbl[i] )
        {
            __VA_CHAN_Destroy(ppstChanTbl[i]);
            ppstChanTbl[i] = NULL;
        }
    }

    VA_Free(ppstChanTbl);
}

FDD_CHAN_S **VA_CHAN_CreateChanTbl(U32 u32ChanNum, CHAN_ID_S *pstChanId, BOOL bBindChan)
{
    FDD_CHAN_S **ppstChanTbl;
    FDD_CHAN_S *pstChan;
    U32 i;

    ppstChanTbl = (FDD_CHAN_S **)VA_Zmalloc(sizeof(FDD_CHAN_S *) * u32ChanNum);
    if ( ppstChanTbl == NULL )
    {
        return ERR_PTR(-ENOMEM);
    }

    if ( bBindChan )
    {
        for (i = 0; i < u32ChanNum; i++)
        {
            pstChanId->u16ChanId = i;
            pstChan = __VA_CHAN_Init(pstChanId);
            if ( IS_ERR(pstChan) )
            {
                VA_CHAN_DestroyChanTbl(ppstChanTbl, i, pstChanId->u16ChanType);
                return ERR_PTR(PTR_ERR(pstChan));
            }

            ppstChanTbl[i] = pstChan;
        }
    }

    return ppstChanTbl;
}

static int __VA_CHAN_CreatePortTbl(VA_CHAN_PORT_TBL_S *pstPortTbl, VA_CHAN_DEV_S *pstChanDev)
{
    VA_CHAN_PER_PORT_S *pstTmpPorts;
    VA_CHAN_PER_PORT_S *pstPorts;
    U32 u32PortNum = pstChanDev->u16PortNum + pstChanDev->u16StartPortIdx;

    if ( pstPortTbl->u32PortNum >= u32PortNum )
    {
        return 0;
    }

    pstTmpPorts = pstPortTbl->pstPorts;
    pstPorts = VA_Zmalloc(u32PortNum * sizeof(VA_CHAN_PER_PORT_S));
    if ( pstPorts == NULL )
    {
        return -ENOMEM;
    }

    if ( pstTmpPorts != NULL )
    {
        memcpy(pstPorts, pstTmpPorts, u32PortNum * sizeof(VA_CHAN_PER_PORT_S));
        VA_Free(pstTmpPorts);
    }

    pstPortTbl->pstPorts   = pstPorts;
    pstPortTbl->u32PortNum = u32PortNum;
    return 0;
}

int VA_CHAN_CreatePortTbl(VA_CHAN_DEV_S *pstChanDev, BOOL bBindChan)
{
    VA_CHAN_PORT_TBL_S *pstPortTbl;
    VA_CHAN_PER_PORT_S *pstPorts;
    FDD_CHAN_S **ppstChanTbl;
    CHAN_ID_S stChanId;
    U32 u32ChanNumPerPort;
    U32 u32PortNum;
    int iRet;
    U32 i;
    U16 u16ChanType = pstChanDev->stDev.u32PrivId;

    if ( u16ChanType >= VA_CHAN_TYPE_NUM )
    {
        return -EINVAL;
    }

    pstPortTbl  = gstVaChanMgr.stSelfSlot.astTypePortTbl + u16ChanType;
    iRet = __VA_CHAN_CreatePortTbl(pstPortTbl, pstChanDev);
    if ( iRet < 0 )
    {
        VA_LOG_ERR("Failed to create port table, iRet %d", iRet);
        return iRet;
    }

    stChanId.u16ChanType = pstChanDev->stDev.u32PrivId;
    stChanId.u16SlotId   = 0;
    stChanId.u16ChanId   = 0;

    pstPorts    = pstPortTbl->pstPorts;
    u32PortNum  = pstChanDev->u16PortNum + pstChanDev->u16StartPortIdx;
    u32ChanNumPerPort = pstChanDev->u16ChanNumPerPort;

    for ( i = pstChanDev->u16StartPortIdx; i < u32PortNum; i++ )
    {
        stChanId.u16PortId = i;
        ppstChanTbl = VA_CHAN_CreateChanTbl(u32ChanNumPerPort, &stChanId, bBindChan);
        if ( IS_ERR(ppstChanTbl) )
        {
            VA_LOG_ERR("Failed to create chan table, iRet %ld", PTR_ERR(ppstChanTbl));
            VA_CHAN_DestroyPortTbl(pstChanDev);
            return PTR_ERR(ppstChanTbl);
        }

        pstPorts[i].ppstChanTbl = ppstChanTbl;
        pstPorts[i].u32ChanNum  = u32ChanNumPerPort;
        pstPortTbl->u32TotChanNum += pstPorts[i].u32ChanNum;
    }

    return 0;
}

void VA_CHAN_DestroyPortTbl(VA_CHAN_DEV_S *pstChanDev)
{
    VA_CHAN_PORT_TBL_S *pstPortTbl;
    VA_CHAN_PER_PORT_S *pstPorts;
    U32 u32PortNum;
    U32 i;
    U16 u16ChanType = pstChanDev->stDev.u32PrivId;

    if ( u16ChanType >= VA_CHAN_TYPE_NUM )
    {
        return;
    }

    pstPortTbl = gstVaChanMgr.stSelfSlot.astTypePortTbl + u16ChanType;
    pstPorts   = pstPortTbl->pstPorts;
    u32PortNum = pstChanDev->u16PortNum + pstChanDev->u16StartPortIdx;
    if ( u32PortNum > pstPortTbl->u32PortNum )
    {
        u32PortNum = pstPortTbl->u32PortNum;
    }

    for ( i = pstChanDev->u16StartPortIdx; i < u32PortNum; i++ )
    {
        if ( pstPorts[i].ppstChanTbl )
        {
            VA_CHAN_DestroyChanTbl(pstPorts[i].ppstChanTbl, pstPorts[i].u32ChanNum, u16ChanType);
            pstPortTbl->u32TotChanNum -= pstPorts[i].u32ChanNum;
            memset(pstPortTbl + i, 0, sizeof(VA_CHAN_PER_PORT_S));
        }
    }

    return;
}

#if 0
#endif

uint32_t VA_CHAN_GetTotalChanNum(U16 u16ChanType)
{
    return gstVaChanMgr.stSelfSlot.astTypePortTbl[u16ChanType].u32TotChanNum;
}

const char *VA_CHAN_GetChanTypeName(U16 u16ChanType)
{
    if ( u16ChanType < VA_CHAN_TYPE_NUM && gstVaChanMgr.pstChanTypeCbTbl[u16ChanType] )
    {
        return gstVaChanMgr.pstChanTypeCbTbl[u16ChanType]->szName;
    }
    else
    {
        return "unknown";
    }
}

VA_CHAN_TYPE_CB_S *VA_CHAN_GetTypeByName(const char *szName)
{
    VA_CHAN_TYPE_CB_S *pstChanTypeCb = NULL;
    U32 i;

    for (i = 0; i < VA_ARR_SIZE(gstVaChanMgr.pstChanTypeCbTbl); i++)
    {
        if ( gstVaChanMgr.pstChanTypeCbTbl[i] )
        {
            if (strcmp(gstVaChanMgr.pstChanTypeCbTbl[i]->szName, szName) == 0)
            {
                pstChanTypeCb = gstVaChanMgr.pstChanTypeCbTbl[i];
                break;
            }
        }
    }

    return pstChanTypeCb;
}

int VA_CHAN_RegTypeCb(VA_CHAN_TYPE_CB_S *pstChanTypeCb)
{
    U16 u16ChanType = pstChanTypeCb->u16ChanType;

    if ( u16ChanType >= VA_CHAN_TYPE_NUM )
    {
        return -EINVAL;
    }

    if ( pstChanTypeCb->szName == NULL )
    {
        VA_LOG_ERR("The chan type name is NULL!");
        return -EINVAL;
    }

    if ( gstVaChanMgr.pstChanTypeCbTbl[pstChanTypeCb->u16ChanType] )
    {
        VA_LOG_ERR("ChanType %u is exist, previous name is %s, now name is %s",
                   u16ChanType, gstVaChanMgr.pstChanTypeCbTbl[u16ChanType]->szName, pstChanTypeCb->szName);
        return -EEXIST;
    }

    gstVaChanMgr.pstChanTypeCbTbl[u16ChanType] = pstChanTypeCb;
    VA_CHAN_MT_RegCmd(pstChanTypeCb);
    return 0;
}

const FDD_CHAN_OPS_S *VA_CHAN_GetFddOps(U16 u16ChanType)
{
    return gstVaChanMgr.pstChanTypeCbTbl[u16ChanType]->pstChanOps;
}

const FDD_PORT_OPS_S *VA_CHAN_GetFddPortOps(U16 u16ChanType)
{
    return gstVaChanMgr.pstChanTypeCbTbl[u16ChanType]->pstPortOps;
}

VOID VA_CHAN_InitPort(FDD_CHAN_S *pstFddChan, FDD_PORT_S *pstPort)
{
    pstPort->pstOps = VA_CHAN_GetFddPortOps(pstFddChan->stChanId.u16ChanType);
}

U32 VA_CHAN_GetChanCbSize(U16 u16ChanType)
{
    return gstVaChanMgr.pstChanTypeCbTbl[u16ChanType]->u16ChanCbSize;
}

LONG VA_CHAN_Ioctl(U32 u32Cmd, VA_IO_CMD_S *pstIoCmd)
{
    VA_CHAN_TYPE_CB_S *pstChanTypeCb;
    FDD_CHAN_S *pstFddChan;
    CHAN_ID_S  *pstChanId;
    LONG lRet;

    MT_DBG_MSG_PRINT(&gstVaChanMt, MT_IO_CMD_FMT, MT_IO_CMD_ARGS(u32Cmd, pstIoCmd));

    atomic_inc(&gstVaChanMgr.stCmdCnt);

    pstChanId = &pstIoCmd->stChanId;

    pstFddChan = VA_GetChan(pstChanId);
    if ( pstFddChan == NULL )
    {
        atomic_inc(&gstVaChanMgr.stCmdErrCnt);
        lRet = -ENOENT;
        goto out;
    }

    // get chan ok, chan type alwasy can get
    pstChanTypeCb = gstVaChanMgr.pstChanTypeCbTbl[pstChanId->u16ChanType];

    if ( pstChanTypeCb->pfnIoctl == NULL )
    {
        lRet = -EIO;
    }
    else
    {
        lRet = pstChanTypeCb->pfnIoctl(pstFddChan, u32Cmd, pstIoCmd->u8Len, pstIoCmd->pParam);
    }

    if ( lRet )
    {
        atomic_inc(&gstVaChanMgr.stCmdErrCnt);
        atomic_inc(&pstChanTypeCb->stCmdErrCnt);
    }

    FDD_PutChan(pstFddChan);

out:
    MT_DBG_MSG_PRINT(&gstVaChanMt, MT_DBG_HEX MT_DBG_L, "Cmd", u32Cmd, "Ret", lRet);
    return lRet;
}

#if 0
#endif

VA_GBL_IO_CMDS_CB_S gstVaGblIoCmdCb =
{
    .u32CmdBase = CP_IOCTL_CHAN_BASE,
    .u32Size    = 0x100,
    .pfnIoCmd   = VA_GblIoctlCmd,
};

static void VA_ChanExit(void)
{
    VA_CP_UnRegIoCmdProc(&gstVaGblIoCmdCb);
    VA_CHAN_UnRegMt();
    return;
}

static int VA_ChanInit(VA_MOD_S *pstMod)
{
    int iRet = 0;

    VA_CP_RegIoCmdProc(&gstVaGblIoCmdCb);
    VA_CHAN_RegMt();
    return iRet;
}

VA_MOD_INIT(chan, VA_ChanInit, VA_ChanExit, VA_INIT_LEVEL_MOD)
