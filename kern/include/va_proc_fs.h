#ifndef __VA_PROC_FS_H__
#define __VA_PROC_FS_H__

#include <linux/proc_fs.h>

#define VA_FS_NAME      "vafs"

typedef struct tagVaFsFile
{
	struct list_head stNode;
	const char *szName;
	const struct file_operations *pstOps;
	VOID *pPriv;
	struct proc_dir_entry *pstDirEntry;
}VA_FS_FILE_S;

extern void VA_FS_InitFileCb(VA_FS_FILE_S *pstFile, const char *szName, const struct file_operations *pstOps, VOID *pPriv);
extern int  VA_FS_FileOpen(struct inode *inode, struct file *pstFile);
extern int  VA_FS_FileRelease(struct inode *inode, struct file *pstFile);
extern void VA_FS_UnRegFile(VA_FS_FILE_S *pstFile);
extern int  VA_FS_RegFile(VA_FS_FILE_S *pstFile);

#endif //__VA_PROC_FS_H__
