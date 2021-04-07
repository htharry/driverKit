#include "va_kern_pub.h"
#include "va_priv.h"

VA_CB_S gstVaCb =
{
    .stModHead = LIST_HEAD_INIT(gstVaCb.stModHead),
    .stDrvHead = LIST_HEAD_INIT(gstVaCb.stDrvHead),
    .stDevHead = LIST_HEAD_INIT(gstVaCb.stDevHead),
};

#if 0
#endif
extern int  VA_RegMt(void);
extern void VA_UnRegMt(void);

#if 0
#endif

int VA_DrvMatch(VA_DRV_S *pstDrv)
{
    struct list_head *pstNode;
    VA_DEV_S *pstDev;
    INT iRet;

    list_for_each(pstNode, &gstVaCb.stDevHead)
    {
        pstDev = list_entry(pstNode, VA_DEV_S, stNode);
        if ((pstDev->u32Id == pstDrv->u32Id) && (pstDev->u32PrivId == pstDrv->u32PrivId))
        {
            iRet = pstDrv->init(pstDev);
            if ( iRet < 0 )
            {
                VA_LOG_ERR("failed to init device %u %u of driver %s, iret %d", pstDev->u32Id, pstDev->u32PrivId, pstDrv->szName, iRet);
            }
        }
    }

    return 0;
}

void VA_UnRegDrv(VA_DRV_S *pstDrv)
{
    struct list_head *pstNode;
    VA_DEV_S *pstDev;

    if ( pstDrv->stNode.prev == NULL ) // no init!
    {
        return;
    }

    mutex_lock(&gstVaCb.stMutex);
    list_for_each(pstNode, &gstVaCb.stDevHead)
    {
        pstDev = list_entry(pstNode, VA_DEV_S, stNode);
        if ((pstDev->u32Id == pstDrv->u32Id) && (pstDev->u32PrivId == pstDrv->u32PrivId))
        {
            if ( pstDrv->exit )
            {
                pstDrv->exit(pstDev);
            }
        }
    }

    list_del(&pstDrv->stNode);
    mutex_unlock(&gstVaCb.stMutex);
}

BOOL VA_FindDrv(U32 u32Id, U32 u32PrivId, VA_DRV_S **ppstDrv)
{
    struct list_head *pstNode;
    VA_DRV_S *pstCurrDrv;

    list_for_each(pstNode, &gstVaCb.stDrvHead)
    {
        pstCurrDrv = list_entry(pstNode, VA_DRV_S, stNode);
        if ( (u32Id == pstCurrDrv->u32Id) && (u32PrivId == pstCurrDrv->u32PrivId) )
        {
            if ( ppstDrv )
            {
                *ppstDrv = pstCurrDrv;
            }

            return TRUE;
        }
    }

    return FALSE;
}

int VA_RegDrv(VA_DRV_S *pstDrv)
{
    int iRet = -EEXIST;

    mutex_lock(&gstVaCb.stMutex);
    if ( VA_FindDrv(pstDrv->u32Id, pstDrv->u32PrivId, NULL) )
    {
        mutex_unlock(&gstVaCb.stMutex);
        return iRet;
    }

    iRet = VA_DrvMatch(pstDrv); // fix me, it's error to go on?
    if ( iRet < 0 )
    {
        mutex_unlock(&gstVaCb.stMutex);
        return iRet;
    }

    list_add_tail(&pstDrv->stNode, &gstVaCb.stDrvHead);
    mutex_unlock(&gstVaCb.stMutex);
    return 0;
}

void __VA_UnRegDev(VA_DEV_S *pstDev)
{
    struct list_head *pstNode;
    VA_DRV_S *pstDrv;

    if ( pstDev->stNode.prev == NULL ) // no init!
    {
        return;
    }

    list_for_each(pstNode, &gstVaCb.stDrvHead)
    {
        pstDrv = list_entry(pstNode, VA_DRV_S, stNode);
        if ( (pstDev->u32Id == pstDrv->u32Id) && (pstDev->u32PrivId == pstDrv->u32PrivId) )
        {
            if ( pstDrv->exit )
            {
                pstDrv->exit(pstDev);
            }

            break;
        }
    }

    list_del(&(pstDev->stNode));
}

void VA_UnRegDev(VA_DEV_S *pstDev)
{
    mutex_lock(&gstVaCb.stMutex);
    __VA_UnRegDev(pstDev);
    mutex_unlock(&gstVaCb.stMutex);

    return;
}

int VA_DevMatch(VA_DEV_S *pstDev)
{
    struct list_head *pstNode;
    VA_DRV_S *pstDrv;
    int iRet;

    list_for_each(pstNode, &gstVaCb.stDrvHead)
    {
        pstDrv = list_entry(pstNode, VA_DRV_S, stNode);
        if ((pstDev->u32Id == pstDrv->u32Id) && (pstDev->u32PrivId == pstDrv->u32PrivId))
        {
            iRet = pstDrv->init(pstDev);
            if ( iRet < 0 )
            {
                VA_LOG_ERR("failed to init device %u %u of driver %s, iret %d", pstDev->u32Id, pstDev->u32PrivId, pstDrv->szName, iRet);
                return iRet;
            }

            break;
        }
    }

    return 0;
}

int __VA_RegDev(VA_DEV_S *pstDev)
{
    int iRet;

    iRet = VA_DevMatch(pstDev);
    if ( iRet < 0 )
    {
        return iRet;
    }

    list_add_tail(&pstDev->stNode, &gstVaCb.stDevHead);
    return 0;
}


int VA_RegDev(VA_DEV_S *pstDev)
{
    int iRet;

    mutex_lock(&gstVaCb.stMutex);
    iRet = __VA_RegDev(pstDev);
    mutex_unlock(&gstVaCb.stMutex);

    return iRet;
}

#if 0
#endif

void VA_GetModRef(void)
{
    BUG_ON(!try_module_get(THIS_MODULE));
}

void VA_PutModRef(void)
{
    module_put(THIS_MODULE);
}

void VA_RegMod(VA_MOD_S *pstMod)
{
    VA_MOD_S *pstTmpMod;
    struct list_head *pstNode;

    if ( list_empty(&gstVaCb.stModHead) )
    {
        list_add(&pstMod->stNode, &gstVaCb.stModHead);
    }
    else
    {
        list_for_each(pstNode, &gstVaCb.stModHead)
        {
            pstTmpMod = VA_PTR_TYPE(VA_MOD_S, pstNode);
            if ( pstTmpMod->u32Prior < pstMod->u32Prior )
            {
                continue;
            }

            list_add_tail(&pstMod->stNode, pstNode);
            break;
        }

        if ( pstNode == &gstVaCb.stModHead )
        {
            list_add_tail(&pstMod->stNode, pstNode);
        }
    }
}

static void VA_ModsExit(void)
{
    VA_MOD_S *pstMod;
    struct list_head *pstNode;
    struct list_head *pstNextNode;

    list_for_each_prev_safe(pstNode, pstNextNode, &gstVaCb.stModHead)
    {
        pstMod = VA_PTR_TYPE(VA_MOD_S, pstNode);
        if ( pstMod->bInited )
        {
            if ( pstMod->exit != NULL )
            {
                VA_LOG_INFO("remove mod %s", pstMod->szName);
                pstMod->exit();
            }
        }
    }
}

static int __init VA_InitMods(void)
{
    VA_MOD_S *pstMod;
    struct list_head *pstNode;
    int iRet;

    list_for_each(pstNode, &gstVaCb.stModHead)
    {
        pstMod = VA_PTR_TYPE(VA_MOD_S, pstNode);
        iRet = pstMod->init(pstMod);
        if ( iRet < 0 )
        {
            VA_LOG_ERR("failed to init %s mod", pstMod->szName);
            VA_ModsExit();
            return iRet;
        }

		VA_LOG_INFO("Init mod %s", pstMod->szName);
        pstMod->bInited = TRUE;
    }

    return 0;
}

#if 0
#endif

static void __exit VA_Exit(void)
{
    VA_UnRegMt();
    VA_ModsExit();
}

static void __init VA_FindCtorsFuncName(void)
{
    U32 i;

    typedef int (*MOD_INIT_PF)(void);

	if (THIS_MODULE->syms)
	{
		for ( i = 0; i < THIS_MODULE->num_syms; i++)
		{
            if ( strncmp(THIS_MODULE->syms[i].name, "VA_ModConstruct", sizeof("VA_ModConstruct") - 1) == 0 )
            {
                MOD_INIT_PF pfnInit = (MOD_INIT_PF)(THIS_MODULE->syms[i].value);
                pfnInit();
            }
		}
	}
}

static int __init VA_Init(void)
{
    int iRet;

    mutex_init(&gstVaCb.stMutex);

#ifndef CONFIG_CONSTRUCTORS
    VA_FindCtorsFuncName();
#endif

    iRet = VA_InitMods();
    if ( iRet < 0 )
    {
        return iRet;
    }

    VA_RegMt();
    printk("succeed to init va!\n");
    return 0;
}

module_init(VA_Init);
module_exit(VA_Exit);
MODULE_LICENSE("GPL");
