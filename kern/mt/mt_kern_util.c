#include <linux/ctype.h>
#include "va_kern_pub.h"

#define MT_DEF_SPEC_CMD_ITEM(stItemName, u8Type, _szId) \
	MT_CMD_ITEM_S stItemName = { 	\
		u8Type,   		\
		0,							\
		0,							\
		{_szId},		\
		{0},                        \
		0,							\
		NULL,					    \
	}

MT_DEF_TEXT_CMD_ITEM(gstDisplay, "disp", 	"display",  "display");
MT_DEF_TEXT_CMD_ITEM(gstInfo,    "info", 	"info",     "information");
MT_DEF_TEXT_CMD_ITEM(gstStat,    "stat", 	"stat",     "statistics");
MT_DEF_TEXT_CMD_ITEM(gstRead,    "rd",   	"read",     "read");
MT_DEF_TEXT_CMD_ITEM(gstWrite,   "wr",   	"write",    "write");
MT_DEF_TEXT_CMD_ITEM(gstCb,      "cb",   	"cb",       "control block");
MT_DEF_TEXT_CMD_ITEM(gstAddr,    "addr", 	"address",  "address");
MT_DEF_TEXT_CMD_ITEM(gstUndo,    "undo", 	"undo",     "undo");
MT_DEF_TEXT_CMD_ITEM(gstDebug,   "dbg",  	"debug",    "debug");
MT_DEF_TEXT_CMD_ITEM(gstAll,     "all",  	"all",      "all");
MT_DEF_TEXT_CMD_ITEM(gstSet,     "set",  	"set",      "set");
MT_DEF_TEXT_CMD_ITEM(gstReset,   "rset",  	"reset",    "reset");
MT_DEF_TEXT_CMD_ITEM(gstReg,   	 "reg",  	"register", "register");
MT_DEF_TEXT_CMD_ITEM(gstChk,   	 "chk",  	"check", 	"check");
MT_DEF_TEXT_CMD_ITEM(gstRes,   	 "res",  	"resource", "resource");
MT_DEF_TEXT_CMD_ITEM(gstDump,    "dump",  	"dump", 	"dump");
MT_DEF_TEXT_CMD_ITEM(gstLoad,    "load",  	"load", 	"load");
MT_DEF_TEXT_CMD_ITEM(gstTest,    "test",  	"test", 	"test");
MT_DEF_IP_CMD_ITEM(gstIp,        "ip",      "ip address");
MT_DEF_CHAN_CMD_ITEM(gstChanId,  "chan",    "chan id");

MT_DEF_SPEC_CMD_ITEM(gstMod,  MT_ITEM_TYPE_MOD, MT_ITEM_ID_MOD_NAME);
MT_DEF_SPEC_CMD_ITEM(gstPass, MT_ITEM_TYPE_PASS, "pass");

MT_DEF_TEXT_CMD_ITEM(gstMsg,  "msg",  "msg",    "message");
MT_DEF_TEXT_CMD_ITEM(gstErr,  "err",  "err",    "error");
MT_DEF_TEXT_CMD_ITEM(gstPkt,  "pkt",  "pkt",    "packet");
MT_DEF_TEXT_CMD_ITEM(gstEnb,  "enb",  "enable", "enable");


MT_CMD_ITEM_S *gastCommCmdItem[MT_ITEM_SPEC_BUTT] =
{
	NULL,
	&gstDisplay,
	&gstRead,
	&gstWrite,
	&gstSet,
	&gstDebug,      /* 5 */
	&gstUndo,
	&gstStat,
	&gstCb,
	&gstReset,
	&gstAll,		/* 10 */
	&gstReg,
	&gstChk,
	&gstRes,
	&gstDump,
	&gstLoad,       /* 15 */
	&gstTest,
	&gstIp,
	&gstChanId,
	&gstInfo,
	&gstMod,        /* 20 */
	&gstMsg,
	&gstErr,
	&gstPkt,
	&gstEnb,
	&gstPass,       /* 25 */
};

const MT_CMD_ITEM_S *MT_GetSpecItem(ULONG ulSpecItem)
{
	if ( ulSpecItem >= MT_ITEM_SPEC_BUTT )
	{
	    return NULL;
	}

	return gastCommCmdItem[ulSpecItem];
}

char *MT_strdup(const char *str)
{
    int n = strlen(str) + 1;
    char *s = VA_Malloc(n);
    if (!s)
        return NULL;
    return strcpy(s, str);
}

struct file *MT_OpenTty(void)
{
    struct file *pstOutFile;

    pstOutFile = filp_open("/dev/tty", O_RDWR, 0);
    return pstOutFile;
}

struct file *MT_OpenOutputFile(struct file *pstSelfFile)
{
    struct file *pstOutFile;
    pstOutFile = fget(1);
    if ( pstOutFile == pstSelfFile )
    {
        fput(pstOutFile);
        pstOutFile = fget(2);
        if ( pstOutFile == pstSelfFile )
        {
            fput(pstOutFile);
            return NULL;
        }
    }

    return pstOutFile;
}

VOID MT_PutOutputFile(struct file *pstOutFile)
{
    fput(pstOutFile);
}

ssize_t MT_KnlWirte(struct file *pstFile, const char *buf, size_t count)
{
    mm_segment_t old_fs;
    ssize_t res;
    loff_t pos = 0;

    if ( pstFile == NULL )
    {
        return -EINVAL;
    }

    old_fs = get_fs();
    set_fs(get_ds());
    /* The cast to a user pointer is valid due to the set_fs() */
    res = vfs_write(pstFile, (__force const char __user *)buf, count, &pos);
    set_fs(old_fs);

    return res;
}

ssize_t MT_KnlFdWirte(const char *buf, size_t count)
{
    struct file *pstFile;

    pstFile = fget(2);
    if ( pstFile == NULL )
    {
        return -ENOENT;
    }

    return MT_KnlWirte(pstFile, buf, count);
}

