#ifndef __VA_TYPE_H__
#define __VA_TYPE_H__

typedef unsigned long long  U64;
typedef unsigned int        U32;
typedef unsigned short      U16;
typedef unsigned char       U8;
typedef unsigned long       ULONG;
typedef U64                 ULLONG;

typedef U32                 BOOL;
typedef void                VOID;

typedef int                 INT;
typedef int                 S32;
typedef short               S16;
typedef char                S8;
typedef long                LONG;
typedef long long           S64;

#ifndef __KERNEL__
typedef U32                 BE32;
typedef U16                 BE16;
typedef U32                 LE32;
typedef U16                 LE16;
#define __user
#else
#define BE32                __be32
#define BE16                __be16
#define LE32                __le32
#define LE16                __le16
#endif

typedef union tagComp64
{
    struct
    {
        U32 u32Val0;
        U32 u32Val1;
    };

    U64 u64Val;
}COMP_U64_U;

#define VA_PTR_TYPE(type, pPriv)    (type *)(VOID *)(pPriv)

#ifndef TRUE
#define TRUE            1
#define FALSE           0
#endif

#endif //__VA_TYPE_H__
