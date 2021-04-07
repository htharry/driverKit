#include "va_kern_pub.h"

typedef struct tagVaFsMgr
{
	struct list_head   stFileHead;
	struct mutex 	   stMutex;
	struct proc_dir_entry *pstRootDir;
}VA_FS_MGR_S;

VA_FS_MGR_S gstVaFsMgr =
{
	.stFileHead = LIST_HEAD_INIT(gstVaFsMgr.stFileHead),
};

int VA_FS_FileOpen(struct inode *inode, struct file *pstFile)
{
    VA_GetModRef();
	return 0;
}

int VA_FS_FileRelease(struct inode *inode, struct file *pstFile)
{
    VA_PutModRef();
	return 0;
}

void VA_FS_InitFileCb(VA_FS_FILE_S *pstFile, const char *szName, const struct file_operations *pstOps, VOID *pPriv)
{
	VA_CB_ZERO(pstFile);

	INIT_LIST_HEAD(&pstFile->stNode);
	pstFile->szName = szName;
	pstFile->pstOps = pstOps;
	pstFile->pPriv  = pPriv;  // fix me!
}

int VA_FS_RegFile(VA_FS_FILE_S *pstFile)
{
    pstFile->pstDirEntry = proc_create_data(pstFile->szName, S_IRUSR|S_IWUSR, gstVaFsMgr.pstRootDir, pstFile->pstOps, pstFile->pPriv);
    if ( pstFile->pstDirEntry == NULL )
    {
        return -ENOMEM;
    }

    mutex_lock(&gstVaFsMgr.stMutex);
	list_add(&pstFile->stNode, &gstVaFsMgr.stFileHead);
	mutex_unlock(&gstVaFsMgr.stMutex);

    return 0;
}

void VA_FS_UnRegFile(VA_FS_FILE_S *pstFile)
{
    remove_proc_entry(pstFile->szName, gstVaFsMgr.pstRootDir);
    return;
}

void VA_FS_ProcExit(void)
{
	remove_proc_entry(VA_FS_NAME, NULL);
}

int VA_FS_ProcInit(VA_MOD_S *pstMod)
{
	int iRet = 0;

	mutex_init(&gstVaFsMgr.stMutex);
    gstVaFsMgr.pstRootDir = proc_mkdir(VA_FS_NAME, NULL);
    if ( gstVaFsMgr.pstRootDir == NULL )
    {
        return -ENOMEM;
    }

	return iRet;
}

VA_MOD_INIT(vafs, VA_FS_ProcInit, VA_FS_ProcExit, VA_INIT_LEVEL_FS)

