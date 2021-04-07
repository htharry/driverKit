#include "va_kern_pub.h"
#include "va_priv.h"

extern VA_CB_S gstVaCb;
extern BOOL VA_FindDrv(U32 u32Id, U32 u32PrivId, VA_DRV_S **ppstDrv);
extern void VA_CP_MT_DispCb(ULONG ulExecId);

static VOID VA_MT_DispModInfo(ULONG ulExecId)
{
    VA_MOD_S *pstMod;
    struct list_head *pstNode;

    MT_PRINT("\r\ndump module name:");
    MT_PRINT(MT_FMT_STR, "name", "prior");

    list_for_each(pstNode, &gstVaCb.stModHead)
    {
        pstMod = VA_PTR_TYPE(VA_MOD_S, pstNode);
        MT_PRINT(MT_FMT_INT, pstMod->szName, pstMod->u32Prior);
    }
}

static VOID VA_MT_DispDevInfo(ULONG ulExecId)
{
    VA_DEV_S *pstDev;
    VA_DRV_S *pstDrv;

    MT_PRINT("\r\ndump device info:");

    list_for_each_entry(pstDev, &gstVaCb.stDevHead, stNode)
    {
        MT_PRINT("\r\ndevice %u %u:", pstDev->u32Id, pstDev->u32PrivId);

        if ( VA_FindDrv(pstDev->u32Id, pstDev->u32PrivId, &pstDrv) )
        {
            MT_PRINT(MT_FMT_STR, "drv name", pstDrv->szName);
            if ( pstDrv->mt )
            {
                pstDrv->mt(pstDev, ulExecId);
            }
        }
        else
        {
            MT_PRINT(MT_FMT_PREFIX "Device is not found driver!");
        }
    }
}

int VA_MT_RegCmd(MT_CB_S *pstMtCb)
{
    MT_DEF_TEXT_CMD_ITEM(stDev, "dev", "device", "register device information");
    MT_DEF_TEXT_CMD_ITEM(stCp,  "cp",  "cp",     "control platform");

    MT_REG_CMD_LINE(VA_MT_DispModInfo, MT_DEF_ITEM_DISP, MT_DEF_ITEM_MOD, MT_DEF_ITEM_INFO);
    MT_REG_CMD_LINE(VA_MT_DispDevInfo, MT_DEF_ITEM_DISP, MT_DEF_ITEM_MOD, &stDev, MT_DEF_ITEM_INFO);
    MT_REG_CMD_LINE(VA_CP_MT_DispCb,   MT_DEF_ITEM_DISP, &stCp, MT_DEF_ITEM_CB);

    return 0;
}

MT_CB_S gstVaCoreMt =
{
    .pfnRegCmd  = VA_MT_RegCmd,
    .szName 	= "core",
    .szDesc 	= "core module",
};

int VA_RegMt(void)
{
    return MT_RegMt(&gstVaCoreMt);
}

void VA_UnRegMt(void)
{
    return MT_UnRegMt(&gstVaCoreMt);
}

