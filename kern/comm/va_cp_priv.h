#ifndef __VA_CP_PRIV_H__
#define __VA_CP_PRIV_H__

#include "va_kern_pub.h"

typedef struct tagVaCpInst
{
    struct list_head stNode;
    pid_t            nTgidId;
}VA_CP_INST_S;

typedef struct tagVaCpCb
{
    atomic_t         stCmdCnt;
    atomic_t         stGblCmdCnt;
    atomic_t         stCmdErrCnt;
    struct list_head stGblCmdHead;  // global command head!
    struct list_head stCpInstHead;
    struct mutex     stCmdLock;
    struct mutex     stInstLock;
}VA_CP_CB_S;


#endif //__VA_CP_PRIV_H__
