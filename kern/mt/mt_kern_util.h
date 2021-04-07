#ifndef __MT_KERN_UTIL__
#define __MT_KERN_UTIL__

extern const MT_CMD_ITEM_S *MT_GetSpecItem(U32 u32SpecItem);
extern char *MT_strdup(const char *str);
extern struct file *MT_OpenTty(void);
extern struct file *MT_OpenOutputFile(struct file *pstSelfFile);
extern VOID MT_PutOutputFile(struct file *pstOutFile);
extern ssize_t MT_KnlWirte(struct file *pstFile, const char *buf, size_t count);

#endif //__MT_KERN_UTIL__
