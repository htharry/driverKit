#ifndef __VA_H__
#define __VA_H__

enum
{
	VA_INIT_LEVEL_FS,		// file system
	VA_INIT_LEVEL_MT,		// maintenance
	VA_INIT_LEVEL_CFG,		// configure
	VA_INIT_LEVEL_CORE,		// core module
	VA_INIT_LEVEL_MOD,		// base module
	VA_INIT_LEVEL_DRV,		// specail driver module
	VA_INIT_LEVEL_DEV,
	VA_INIT_LEVEL_MISC,		// some misc device
};

enum
{
	VA_DEV_ID_VCHAN	= 1, 	// video chan device
	VA_DEV_ID_MEM,          // memory device
	VA_DEV_ID_BOARD,        // board  device
	VA_DEV_ID_VN,           // video network
	VA_DEV_ID_INVAL = 0,    // invalid device id
};

#ifdef CONFIG_CONSTRUCTORS
#define VA_MOD_INIT(Mod, pfnInit, pfnExit, __u32Prior) \
	static VA_MOD_S __gstMod##Mod = \
	{ \
		.init = pfnInit, 		\
		.exit = pfnExit, 		\
		.u32Prior = __u32Prior, \
		.szName   = #Mod,  	\
		.bInited  = FALSE, 		\
	}; \
	static int __init VA_ModConstruct##Mod(void) \
	{ \
		VA_RegMod(&__gstMod##Mod); \
		return 0; \
	} \
	MOD_INIT_CALL(VA_ModConstruct##Mod);
#else
#define VA_MOD_INIT(Mod, pfnInit, pfnExit, __u32Prior) \
	static VA_MOD_S __gstMod##Mod = \
	{ \
		.init = pfnInit, 		\
		.exit = pfnExit, 		\
		.u32Prior = __u32Prior, \
		.szName   = #Mod,  	\
		.bInited  = FALSE, 		\
	}; \
	int VA_ModConstruct##Mod(void) \
	{ \
		VA_RegMod(&__gstMod##Mod); \
		return 0; \
	} \
    EXPORT_SYMBOL(VA_ModConstruct##Mod);
#endif

typedef int (*MOD_INIT_PF)(void);		//ctors call back function

#define __va_ctors				__attribute__((__section__(".ctors")))
#define MOD_INIT_CALL(fn)		static MOD_INIT_PF __initcal_##fn __used __va_ctors = fn


#define VA_DEV_INIT(u32Id, u32PrivId)   {u32Id, u32PrivId, {NULL, NULL}}

typedef struct tagVaMod
{
	struct list_head stNode;
	U32 u32Prior;
	const char *szName;
	BOOL bInited;
	int  (*init)(struct tagVaMod *pstMod);
	void (*exit)(void);
}VA_MOD_S;

typedef struct tagVaDev
{
	U32 u32Id;			// main id
	U32 u32PrivId;		// sub  id, such as chan type
	struct list_head stNode;
}VA_DEV_S;

typedef struct tagVaDrv
{
	struct list_head stNode;
	U32 u32Id;
	U32 u32PrivId;
	const char *szName;
	int  (*init)(VA_DEV_S *pstDev);
	void (*exit)(VA_DEV_S *pstDev);
    void (*mt)(VA_DEV_S *pstDev, ULONG ulExecId);
}VA_DRV_S;

extern void VA_GetModRef(void);
extern void VA_PutModRef(void);
extern int VA_RegDev(VA_DEV_S *pstDev);
extern int __VA_RegDev(VA_DEV_S *pstDev);
extern void __VA_UnRegDev(VA_DEV_S *pstDev);
extern void VA_UnRegDev(VA_DEV_S *pstDev);
extern int VA_RegDrv(VA_DRV_S *pstDrv);
extern void VA_UnRegDrv(VA_DRV_S *pstDrv);
extern void VA_RegMod(VA_MOD_S *pstMod);

#endif //__VA_H__
