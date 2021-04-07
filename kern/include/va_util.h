#ifndef __VA_UTIL_H__
#define __VA_UTIL_H__

typedef struct tagVaReEnterMutex
{
    struct mutex       stLock;
    struct task_struct *pstOwnTsk;
    U32                u32Ref;
}VA_REENTER_MUTEX_S;

typedef struct tagVaMapBuf
{
    struct page **ppstPageTbl;
    U32         u32PageNum;
    char        *pMemBuf;
}VA_MAP_BUF_S;

extern void VA_InitHlistTbl(struct hlist_head *pstHlistTbl, U32 u32Cnt);
extern void VA_InitListTbl(struct list_head *pstListTbl, U32 u32Cnt);
extern void VA_ReEnterMutexInit(VA_REENTER_MUTEX_S *pstMutex);
extern void VA_ReEnterMutexLock(VA_REENTER_MUTEX_S *pstMutex);
extern void VA_ReEnterMutexUnLock(VA_REENTER_MUTEX_S *pstMutex);
extern struct page **VA_AllocPageArray(U32 u32PageNum);
extern void VA_ReleaseMapBufMem(VA_MAP_BUF_S *pstMapBuf);
extern int  VA_AllocDoubleMapBuf(VA_MAP_BUF_S *pstMapBuf, U32 u32BufSize);

#endif //__VA_UTIL_H__