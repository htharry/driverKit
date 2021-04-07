#ifndef __VA_DEF_H__
#define __VA_DEF_H__

#define IN
#define OUT
#define INOUT

#define VA_FS_PATH		"/proc/vafs/"

#define VA_INVALID_FD   -1
#define VA_USHORT_MAX	((U16)(~0U))
#define VA_SHORT_MAX	((S16)(USHRT_MAX>>1))
#define VA_SHORT_MIN	((S16)(-SHRT_MAX - 1))
#define VA_INT_MAX		((int)(~0U>>1))
#define VA_INT_MIN		(-INT_MAX - 1)
#define VA_UINT_MAX	    (~0U)
#define VA_LONG_MAX	    ((long)(~0UL>>1))
#define VA_LONG_MIN	    (-LONG_MAX - 1)
#define VA_ULONG_MAX	(~0UL)
#define VA_LLONG_MAX	((long long)(~0ULL>>1))
#define VA_LLONG_MIN	(-LLONG_MAX - 1)
#define VA_ULLONG_MAX	(~0ULL)
#define VA_U64_MAX      VA_ULLONG_MAX
#define VA_KB           (1024)
#define VA_MB           (VA_KB * VA_KB)
#define VA_IP_STR_LEN   sizeof("255.255.255.255")
#define VA_IP_LEN       (sizeof("255.255.255.255") - 1)

enum
{
    VA_SUCCESS   = 0,
    VA_E_INVAL,
    VA_E_NO_MEM,
    VA_E_NOT_SUPPORT,
    VA_E_SYS_FAILED,
    VA_E_2BIG,          // too big
    VA_E_NO_ENT,        // no entry
    VA_E_NO_DEV,
};

#define VA_IS_ERR(nRet)                 ((nRet) != VA_SUCCESS)

#define VA_TEST_BIT(Val, BitNo)         ((Val) & (0x1 << (BitNo)))
#define VA_CLR_BIT(Val, BitNo)          do {Val = (Val) & (~(0x1 << (BitNo)));}while(0)
#define VA_SET_BIT(Val, BitNo)          do {Val = (Val) | (0x1 << (BitNo));}while(0)

#define VA_ARR_SIZE(astArr)             (sizeof(astArr)/sizeof(astArr[0]))
#define VA_SET_STR_ARR_TERM(szArr)      do {szArr[sizeof(szArr) - 1] = 0;}while(0)
#define VA_ARR_CPY(aSrc, aDst)          do {memcpy(aSrc, aDst, sizeof(aSrc));}while(0)

#define VA_MEM_ZERO(pData, u32Size)     do {memset(pData, 0, u32Size);}while(0)
#define VA_CB_ZERO(pstCb)               VA_MEM_ZERO(pstCb, sizeof(*pstCb))

#define VA_SWAB16(x) ((U16)(                 \
        (((U16)(x) & (U16)0x00ffU) << 8) |   \
        (((U16)(x) & (U16)0xff00U) >> 8)))

#define VA_SWAB32(x) ((U32)(                    \
        (((U32)(x) & (U32)0x000000ff) << 24) |  \
        (((U32)(x) & (U32)0xff000000) >> 24) |  \
        (((U32)(x) & (U32)0x0000ff00) <<  8) |  \
        (((U32)(x) & (U32)0x00ff0000) >>  8)))


#ifndef __KERNEL__
#define VA_Malloc(u32Size)              malloc(u32Size)
#define VA_Free(pPtr)                   free(pPtr)
#define VA_Zmalloc(u32Size)             calloc(1, u32Size)
#define VA_MallocCb(type)               (type *)VA_Zmalloc(sizeof(type))
#else
#define VA_Malloc(u32Size)              kmalloc(u32Size, GFP_KERNEL)
#define VA_Zmalloc(u32Size)             kzalloc(u32Size, GFP_KERNEL)
#define VA_VMalloc(u32Size)             vmalloc(u32Size)
#define VA_MallocCb(type)               (type *)VA_Zmalloc(sizeof(type))
#define VA_Free(x)                      kfree(x)
#define VA_VFree(x)                     vfree(x)
#endif

#define VA_SnprintEx(szBuf, nLen, szFmt, args...)   \
    ({ \
        int __nSize = snprintf(szBuf, nLen, szFmt, ##args); \
        if ( __nSize >= nLen ) \
        { \
            __nSize = nLen - 1; \
        } \
        __nSize; \
    })

#define VA_VsnprintEx(szBuf, nLen, szFmt, args)   \
    ({ \
        int __nSize = vsnprintf(szBuf, nLen, szFmt, args); \
        if ( __nSize >= nLen ) \
        { \
            __nSize = nLen - 1; \
        } \
        __nSize; \
    })


#define VA_ROUNDUP(u32Size, u32Align)   ((u32Size + u32Align - 1)/(u32Align)) * (u32Align)

#define VA_UNUSED_ARG(x)                ((VOID) x)
#define VA_OFFSET(type, member)  		(ULONG)(&(((type *)(NULL))->member))

#ifndef __KERNEL__
#define VA_LOG_INFO					printf
#define VA_LOG_ERR(szFmt, args...)  \
    do { \
		U32 __i = 0; \
		const char *__pFileName = __FILE__;\
		while (__pFileName[__i]) \
		{ \
		    if (__pFileName[__i] == '/') \
		    { \
				__pFileName = __pFileName + __i + 1; \
				__i = 0;    \
				continue;   \
		    } \
            __i++; \
		} \
        { \
            printf("[%s] [%d]: " szFmt "\n", __pFileName, __LINE__, ##args);\
        } \
    } while(0)

#else
#define VA_LOG_INFO(szFmt, args...)   printk(szFmt "\n", ##args)

#define VA_LOG_ERR(szFmt, args...) \
    do { \
		U32 __i = 0; \
		const char *__pFileName = __FILE__;\
		while (__pFileName[__i]) \
		{ \
		    if (__pFileName[__i] == '/') \
		    { \
				__pFileName = __pFileName + __i + 1; \
				__i = 0;    \
				continue;   \
		    } \
            __i++; \
		} \
        { \
            printk(KERN_ERR "[%s] [%d]: " szFmt "\n", __pFileName, __LINE__, ##args);\
        } \
    } while(0)

#endif

#endif //__VA_DEF_H__
