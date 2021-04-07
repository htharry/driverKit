#include <linux/pagemap.h>
#include <linux/namei.h>
#include "va_kern_pub.h"
#include "va_fs.h"


typedef struct tagVaFsMgr
{
	struct list_head   stFileHead;
	struct mutex 	   stMutex;
	struct super_block *pstSb;
}VA_FS_MGR_S;

VA_FS_MGR_S gstVaFsMgr =
{
	.stFileHead = LIST_HEAD_INIT(gstVaFsMgr.stFileHead),
};

extern struct inode *VA_FS_GetNewInode(mode_t mode, struct super_block *pstSb);

int VA_FS_FileOpen(struct inode *inode, struct file *pstFile)
{
	return 0;
}

int VA_FS_FileRelease(struct inode *inode, struct file *pstFile)
{
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

static int VA_FS_AddFile(struct dentry *pstDir, VA_FS_FILE_S *pstFile)
{
	struct inode  *pstInode;
	struct dentry *pstDentry = lookup_one_len(pstFile->szName, pstDir, strlen(pstFile->szName));

	if ( !IS_ERR(pstDentry) )
	{
	    if ( !pstDentry )
	    {
	        return -ENOMEM;
	    }

		if ( pstDentry->d_inode )
		{
            VA_LOG_ERR("FileName %s is exist", pstFile->szName);
		    return -EEXIST;
		}

		pstInode = VA_FS_GetNewInode(S_IRUSR|S_IWUSR|S_IFREG, gstVaFsMgr.pstSb);
		if ( pstInode == NULL )
		{
			dput(pstDentry);
		    return -ENOMEM;
		}

		//pstDentry->d_op = NULL;
		pstInode->i_size = 0;
		pstInode->i_fop  = pstFile->pstOps;
		pstInode->i_private = pstFile->pPriv;

		pstFile->pstDentry = pstDentry;
		d_instantiate(pstDentry, pstInode);
		return 0;
	}
	else
	{
	    return PTR_ERR(pstDentry);
	}
}

static int VA_FS_AddFiles(void)
{
	VA_FS_FILE_S *pstFile;
	int iRet;

	list_for_each_entry(pstFile, &gstVaFsMgr.stFileHead, stNode)
	{
		iRet = VA_FS_AddFile(gstVaFsMgr.pstSb->s_root, pstFile);
		if ( iRet < 0 )
		{
		    return iRet;
		}
	}

	return 0;
}

static void VA_FS_DelFile(struct dentry *pstDir, VA_FS_FILE_S *pstFile)
{
	struct dentry *d = pstFile->pstDentry;

	BUG_ON(!mutex_is_locked(&pstDir->d_inode->i_mutex));

	dget(d);
	d_delete(d);
	simple_unlink(pstDir->d_inode, d);
	dput(d);

	pstFile->pstDentry = NULL;
}

static void VA_FS_ClearFile(void)
{
	VA_FS_FILE_S *pstFile;

	list_for_each_entry(pstFile, &gstVaFsMgr.stFileHead, stNode)
	{
		pstFile->pstDentry = NULL;
	}
}

#if 0
#endif

static void VA_FS_UnPopulateFile(VA_FS_FILE_S *pstFile)
{
	struct inode *pstInode;

	if ( gstVaFsMgr.pstSb && pstFile->pstDentry )
	{
		pstInode = gstVaFsMgr.pstSb->s_root->d_inode;

		mutex_lock(&pstInode->i_mutex);
		VA_FS_DelFile(gstVaFsMgr.pstSb->s_root, pstFile);
		mutex_unlock(&pstInode->i_mutex);
	}
}

void VA_FS_UnRegFile(VA_FS_FILE_S *pstFile)
{
	mutex_lock(&gstVaFsMgr.stMutex);
	list_del(&pstFile->stNode);
	VA_FS_UnPopulateFile(pstFile);
	mutex_unlock(&gstVaFsMgr.stMutex);
}

static int VA_FS_PopulateFile(VA_FS_FILE_S *pstFile)
{
	struct inode *pstInode;
	int iRet = 0;

	pstInode = gstVaFsMgr.pstSb->s_root->d_inode;

	mutex_lock(&pstInode->i_mutex);
	iRet = VA_FS_AddFile(gstVaFsMgr.pstSb->s_root, pstFile);
	mutex_unlock(&pstInode->i_mutex);

	return iRet;
}

int VA_FS_RegFile(VA_FS_FILE_S *pstFile)
{
	int iRet = 0;

	pstFile->pstDentry = NULL;
	mutex_lock(&gstVaFsMgr.stMutex);

	if ( gstVaFsMgr.pstSb )
	{
	    iRet = VA_FS_PopulateFile(pstFile);
		if ( iRet < 0 )
		{
			mutex_unlock(&gstVaFsMgr.stMutex);
		    return iRet;
		}
	}

	list_add(&pstFile->stNode, &gstVaFsMgr.stFileHead);
	mutex_unlock(&gstVaFsMgr.stMutex);
	return 0;
}

#if 0
#endif

static int VA_FS_TestSb(struct super_block *pstSb, void *pData)
{
	return 1;
}

static const struct super_operations gstVaFsSuperOps =
{
	.statfs     = simple_statfs,
	.drop_inode = generic_delete_inode,
};

static int VA_FS_SetSuper(struct super_block *pstSb, void *pData)
{
	int iRet;

	iRet = set_anon_super(pstSb, NULL);
	if ( iRet )
	{
	    return iRet;
	}

	pstSb->s_fs_info   = pData;
	pstSb->s_blocksize = PAGE_CACHE_SIZE;
	pstSb->s_blocksize_bits = PAGE_CACHE_SHIFT;
	pstSb->s_magic = VA_FS_SUPER_MAGIC;
	pstSb->s_op = &gstVaFsSuperOps;

	return 0;
}

struct inode *VA_FS_GetNewInode(mode_t mode, struct super_block *pstSb)
{
	struct inode *inode = new_inode(pstSb);

	if (inode) {
		inode->i_mode = mode;
		inode->i_uid = current_fsuid();
		inode->i_gid = current_fsgid();
		inode->i_atime = inode->i_mtime = inode->i_ctime = CURRENT_TIME;
	}

	return inode;
}

static int VA_FS_CreateRootDir(struct super_block *pstSb)
{
	struct inode *pstInode = VA_FS_GetNewInode(S_IFDIR|S_IRUGO|S_IXUGO, pstSb);
	struct dentry *dentry;

	if ( !pstInode )
	{
	    return -ENOMEM;
	}

	pstInode->i_fop = &simple_dir_operations;
	pstInode->i_op  = &simple_dir_inode_operations;

	/* start off with i_nlink == 2 (for "." entry) */
	// inc_nlink(inode);

	dentry = d_make_root(pstInode);
	if ( !dentry )
	{
		return -ENOMEM;
	}

    pstSb->s_root = dentry;
	return 0;
}

static int VA_FS_FillSuper(struct super_block *pstSb)
{
	pstSb->s_flags |= MS_NODIRATIME|MS_NOSUID|MS_NOEXEC;
	pstSb->s_time_gran = 1;

	return VA_FS_CreateRootDir(pstSb);
}

static struct dentry * VA_FS_GetSb(struct file_system_type *fs_type,
			 		   			   int flags, const char *unused_dev_name,
			 		   			   void *data)
{
	struct super_block *pstSb;
	int iRet = -ENOMEM;

	pstSb = sget(fs_type, VA_FS_TestSb, VA_FS_SetSuper, 0, NULL);
	if ( IS_ERR(pstSb) )
	{
	    return ERR_PTR(PTR_ERR(pstSb));
	}

	if ( pstSb->s_root == NULL )
	{
	    //fill the root dir
	    struct inode *pstInode;
		iRet = VA_FS_FillSuper(pstSb);
		if ( iRet < 0 )
		{
		    goto drop_new_super;
		}

		pstInode = pstSb->s_root->d_inode;

		mutex_lock(&gstVaFsMgr.stMutex);
		if ( gstVaFsMgr.pstSb )
		{
			mutex_unlock(&gstVaFsMgr.stMutex);
		    iRet = -EBUSY;
			goto drop_new_super;
		}

        gstVaFsMgr.pstSb = pstSb;
		mutex_lock(&pstInode->i_mutex);
		iRet = VA_FS_AddFiles();
		mutex_unlock(&pstInode->i_mutex);

		gstVaFsMgr.pstSb = pstSb;
		mutex_unlock(&gstVaFsMgr.stMutex);
	}

    return dget(pstSb->s_root);

drop_new_super:
	deactivate_locked_super(pstSb);
	return ERR_PTR(iRet);
}

static void VA_FS_KillSb(struct super_block *pstSb)
{
	mutex_lock(&gstVaFsMgr.stMutex);
	VA_FS_ClearFile();
	gstVaFsMgr.pstSb = NULL;
	mutex_unlock(&gstVaFsMgr.stMutex);

	kill_litter_super(pstSb);
}

static struct file_system_type gstVaFsType =
{
	.name    = "vafs",
	.owner   = THIS_MODULE,
	.mount   = VA_FS_GetSb,
	.kill_sb = VA_FS_KillSb,
};

void VA_FS_Exit(void)
{
	unregister_filesystem(&gstVaFsType);
}

int VA_FS_Init(VA_MOD_S *pstMod)
{
	int iRet;

	mutex_init(&gstVaFsMgr.stMutex);
    gstVaFsMgr.pstSb = NULL;

	iRet = register_filesystem(&gstVaFsType);
	if ( iRet < 0 )
	{
		return iRet;
	}

	return 0;
}

VA_MOD_INIT(vafs, VA_FS_Init, VA_FS_Exit, VA_INIT_LEVEL_FS)

