#ifndef __MT_DEF__
#define __MT_DEF__

#ifdef  __cplusplus
extern "C"{
#endif

#define MT_NAME_MAX_LEN			    32
#define MT_STR_MAX_LEN			    MT_NAME_MAX_LEN
#define MT_DESC_MAX_LEN			    64
#define MT_ID_MAX_LEN			    sizeof(CMD_ID_T)
#define MT_FMT_MAX_LEN			    (16 * VA_KB)
#define MT_ITEM_MAX_NUM			    16
#define MT_MAX_INPUT_LEN		    256
#define MT_DATA_DEF_LEN             256
#define MT_DATA_MAX_LEN             256
#define MT_ITEM_MAX_ARR             8

#define MT_ITEM_ID_IP               "ip"
#define MT_ITEM_ID_CHAN             "chan"
#define MT_ITEM_ID_REG              "reg"
#define MT_ITEM_ID_RESET            "rset"
#define MT_ITEM_ID_UNDO             "undo"
#define MT_ITEM_ID_CB               "cb"
#define MT_ITEM_ID_STAT             "stat"
#define MT_ITEM_ID_READ             "rd"
#define MT_ITEM_ID_WRITE            "wr"
#define MT_ITEM_ID_INFO             "info"
#define MT_ITEM_ID_MOD_NAME   		"mod"
#define MT_ITEM_ID_ALL				"all"
#define MT_ITEM_ID_LEN				"len"
#define MT_ITEM_ID_VAL				"val"
#define MT_ITEM_ID_ADDR				"addr"
#define MT_ITEM_ID_REG_ADDR			"radd"
#define MT_ITEM_ID_REG_VAL			"rval"
#define MT_ITEM_ID_REG_MASK			"mask"
#define MT_ITEM_ID_REG_RES_NAME		"res"
#define MT_ITEM_ID_REG_OP_U8		"ru8"
#define MT_ITEM_ID_REG_OP_U16		"ru16"
#define MT_ITEM_ID_REG_OP_U32		"ru32"
#define MT_ITEM_ID_REG_OP_U64		"ru64"
#define MT_ITEM_ID_DBG_INFO			"dinf"
#define MT_ITEM_ID_DBG_PKT			"dpkt"
#define MT_ITEM_ID_DBG_ERR 			"derr"
#define MT_ITEM_ID_DBG_MSG			"dmsg"
#define MT_ITEM_ID_CHIP				"chip"
#define MT_ITEM_ID_BUS				"bus"
#define MT_ITEM_ID_DEV				"dev"
#define MT_ITEM_ID_COUNT			"cnt"
#define MT_ITEM_ID_INDX				"indx"
#define MT_ITEM_ID_MSG				"msg"
#define MT_ITEM_ID_ERR				"err"
#define MT_ITEM_ID_PKT				"pkt"
#define MT_ITEM_ID_ENB				"enb"


enum
{
	MT_ITEM_TYPE_TEXT,
	MT_ITEM_TYPE_STR,
	MT_ITEM_TYPE_INT,
	MT_ITEM_TYPE_HEX,
	MT_ITEM_TYPE_CHAN,
	MT_ITEM_TYPE_PTR,
	MT_ITEM_TYPE_IP,
	MT_ITEM_TYPE_MOD,
	MT_ITEM_TYPE_VAR,       // dynamic int variant,  get min and max value really
	MT_ITEM_TYPE_PASS,      // to option item, one is need, another is pass
};

enum
{
	MT_ITEM_FLG_OPT			= 0x1,
	MT_ITEM_FLG_ARRAY		= 0x2,
	MT_ITEM_FLG_SPEC		= 0x4,
	MT_ITEM_FLG_MERGE		= 0x8,
	MT_ITEM_FLG_ARR_END		= 0x10,
};

enum
{
	MT_DBG_MSG				= 0x1,
	MT_DBG_PKT				= 0x2,
	MT_DBG_ERR				= 0x4,
	MT_DBG_INFO				= 0x8,
	MT_DBG_ALL				= MT_DBG_MSG | MT_DBG_PKT | MT_DBG_ERR | MT_DBG_INFO,
};

enum
{
    MT_ITEM_SPEC_TERM     = 0,
    MT_ITEM_SPEC_DISP,
    MT_ITEM_SPEC_READ,
    MT_ITEM_SPEC_WRITE,
    MT_ITEM_SPEC_SET,
    MT_ITEM_SPEC_DEBUG    = 5,
    MT_ITEM_SPEC_UNDO,
    MT_ITEM_SPEC_STAT,
    MT_ITEM_SPEC_CB,
    MT_ITEM_SPEC_RESET,
    MT_ITEM_SPEC_ALL      = 10,
    MT_ITEM_SPEC_REG,
    MT_ITEM_SPEC_CHK,
    MT_ITEM_SPEC_RESOURCE,
    MT_ITEM_SPEC_DUMP,
    MT_ITEM_SPEC_LOAD     = 15,
    MT_ITEM_SPEC_TEST,
    MT_ITEM_SPEC_IP,
    MT_ITEM_SPEC_CHAN,
    MT_ITEM_SPEC_INFO,
    MT_ITEM_SPEC_MOD      = 20,
    MT_ITEM_SPEC_MSG,
    MT_ITEM_SPEC_ERR,
    MT_ITEM_SPEC_PKT,
    MT_ITEM_SPEC_ENABLE,
    MT_ITEM_SPEC_PASS     = 25,
    MT_ITEM_SPEC_BUTT,
};

#define MT_DEF_ITEM_TERM                NULL
#define MT_DEF_ITEM_DISP                (MT_CMD_ITEM_S *)MT_ITEM_SPEC_DISP
#define MT_DEF_ITEM_READ                (MT_CMD_ITEM_S *)MT_ITEM_SPEC_READ
#define MT_DEF_ITEM_WRITE               (MT_CMD_ITEM_S *)MT_ITEM_SPEC_WRITE
#define MT_DEF_ITEM_SET                 (MT_CMD_ITEM_S *)MT_ITEM_SPEC_SET
#define MT_DEF_ITEM_DEBUG               (MT_CMD_ITEM_S *)MT_ITEM_SPEC_DEBUG
#define MT_DEF_ITEM_UNDO                (MT_CMD_ITEM_S *)MT_ITEM_SPEC_UNDO
#define MT_DEF_ITEM_STAT                (MT_CMD_ITEM_S *)MT_ITEM_SPEC_STAT
#define MT_DEF_ITEM_CB                  (MT_CMD_ITEM_S *)MT_ITEM_SPEC_CB
#define MT_DEF_ITEM_RESET               (MT_CMD_ITEM_S *)MT_ITEM_SPEC_RESET
#define MT_DEF_ITEM_ALL                 (MT_CMD_ITEM_S *)MT_ITEM_SPEC_ALL
#define MT_DEF_ITEM_REG                 (MT_CMD_ITEM_S *)MT_ITEM_SPEC_REG
#define MT_DEF_ITEM_CHK                 (MT_CMD_ITEM_S *)MT_ITEM_SPEC_CHK
#define MT_DEF_ITEM_RES                 (MT_CMD_ITEM_S *)MT_ITEM_SPEC_RESOURCE
#define MT_DEF_ITEM_DUMP                (MT_CMD_ITEM_S *)MT_ITEM_SPEC_DUMP
#define MT_DEF_ITEM_LOAD                (MT_CMD_ITEM_S *)MT_ITEM_SPEC_LOAD
#define MT_DEF_ITEM_TEST                (MT_CMD_ITEM_S *)MT_ITEM_SPEC_TEST
#define MT_DEF_ITEM_IP                	(MT_CMD_ITEM_S *)MT_ITEM_SPEC_IP
#define MT_DEF_ITEM_CHAN                (MT_CMD_ITEM_S *)MT_ITEM_SPEC_CHAN
#define MT_DEF_ITEM_INFO                (MT_CMD_ITEM_S *)MT_ITEM_SPEC_INFO
#define MT_DEF_ITEM_MOD                 (MT_CMD_ITEM_S *)MT_ITEM_SPEC_MOD
#define MT_DEF_ITEM_MSG                 (MT_CMD_ITEM_S *)MT_ITEM_SPEC_MSG
#define MT_DEF_ITEM_ERR                 (MT_CMD_ITEM_S *)MT_ITEM_SPEC_ERR
#define MT_DEF_ITEM_PKT                 (MT_CMD_ITEM_S *)MT_ITEM_SPEC_PKT
#define MT_DEF_ITEM_ENB                 (MT_CMD_ITEM_S *)MT_ITEM_SPEC_ENABLE
#define MT_DEF_ITEM_PASS                (MT_CMD_ITEM_S *)MT_ITEM_SPEC_PASS


//#define MT_FMT_UC               "\r\n  %-20s: %u"
//#define MT_FMT_US               "\r\n  %-20s: %u"
#define MT_FMT_U                "\r\n  %-20s: %u"
#define MT_FMT_I                "\r\n  %-20s: %d"

//#define MT_FMT_CHAN             "\r\n   Chan Id: %u/%u/%u"

#define MT_FMT_HEX				"\r\n  %-20s: 0x%x"
#define MT_FMT_INT				"\r\n  %-20s: %u"
#define MT_FMT_STR				"\r\n  %-20s: %s"
#define MT_FMT_PTR				"\r\n  %-20s: 0x%p"
#define MT_FMT_UL				"\r\n  %-20s: %lu"
#define MT_FMT_L                "\r\n  %-20s: %ld"

#define MT_FMT_U64				"\r\n  %-20s: %llu"
#define MT_FMT_CHAN				"\r\n  %-20s: %u/%u/%u"
#ifdef __KERNEL__
#define MT_FMT_FUNC				"\r\n  %-20s: %pF"
#define MT_FMT_IP				"\r\n  %-20s: %pI4"
#endif
#define MT_FMT_U8_HEX           "\r\n  %-20s: 0x%02x"
#define MT_FMT_U16_HEX          "\r\n  %-20s: 0x%04x"
#define MT_FMT_U32_HEX          "\r\n  %-20s: 0x%08x"
#define MT_FMT_U64_HEX          "\r\n  %-20s: 0x%llx"
#define MT_FMT_UL_HEX			"\r\n  %-20s: 0x%lx"
#define MT_FMT_PREFIX           "\r\n  "


#define MT_DBG_HEX			    "\r\n %-16s: 0x%x"
#define MT_DBG_INT			    "\r\n %-16s: %u"
#define MT_DBG_STR			    "\r\n %-16s: %s"
#define MT_DBG_PTR			    "\r\n %-16s: %p"
#define MT_DBG_UL			    "\r\n %-16s: %lu"
#define MT_DBG_UL_HEX		    "\r\n %-16s: 0x%lx"
#define MT_DBG_L			    "\r\n %-16s: %ld"
#define MT_DBG_U64			    "\r\n %-16s: %llu"
#define MT_DBG_U8               "\r\n %-16s: %u"
#define MT_DBG_U16              "\r\n %-16s: %u"
#define MT_DBG_U32              "\r\n %-16s: %u"
#define MT_DBG_U                "\r\n %-16s: %u"
#define MT_DBG_U8_HEX           "\r\n %-16s: 0x%02x"
#define MT_DBG_U16_HEX          "\r\n %-16s: 0x%04x"
#define MT_DBG_U32_HEX          "\r\n %-16s: 0x%08x"
#define MT_DBG_U64_HEX          "\r\n %-16s: 0x%llx"
#define MT_DBG_UC               "\r\n %-16s: %u"
#define MT_DBG_US               "\r\n %-16s: %u"
#define MT_DBG_UI               "\r\n %-16s: %u"
#define MT_DBG_CHAN			    "\r\n %-16s: %u/%u/%u"

#ifdef __KERNEL__
#define MT_DBG_FMT_FUNC			"\r\n %-16s: %pF"
#define MT_DBG_FMT_IP			"\r\n %-16s: %pI4"
#endif

#define MT_DBG_PREFIX           "\r\n "

#define MT_DISP_FMT_MEM(szDesc, u32Size) \
    if ( u32Size > 1024 && u32Size < 1024 * 1024 ) \
    { \
        MT_PRINT(MT_FMT_U " KB", szDesc, u32Size/1024); \
    }\
    else if ( u32Size >= 1024 * 1024 ) \
    { \
        MT_PRINT(MT_FMT_U " MB", szDesc, u32Size/1024/1024); \
    }\
    else \
    { \
        MT_PRINT(MT_FMT_U " Byte", szDesc, u32Size); \
    }

#define MT_DEF_INT_CMD_ITEM(stItemName, _szId, _u64MinVal, _u64MaxVal, _szDesc) \
	MT_CMD_ITEM_S stItemName = { 	\
		MT_ITEM_TYPE_INT,   		\
		0,							\
		0,							\
		{_szId},					\
		{_u64MinVal},			    \
		_u64MaxVal,					\
		"<INTEGER>",				\
		_szDesc,					\
	}

#define MT_DEF_VAR_CMD_ITEM(stItemName, _szId, _szDesc) \
        MT_CMD_ITEM_S stItemName = {    \
            MT_ITEM_TYPE_VAR,           \
            0,                          \
            0,                          \
            {_szId},                    \
            {0},                        \
            0,                          \
            "<INTEGER>",                \
            _szDesc,                    \
        }

#define MT_DEF_HEX_CMD_ITEM(stItemName, _szId, _u64MinVal, _u64MaxVal, _szDesc) \
	MT_CMD_ITEM_S stItemName = { 	\
		MT_ITEM_TYPE_HEX,   		\
		0,							\
		0,							\
		{_szId},					\
		{_u64MinVal},			    \
		_u64MaxVal,					\
		"<HEX>",					\
		_szDesc,					\
	}

#define MT_DEF_PTR_CMD_ITEM(stItemName, _szId, _u64MinVal, _u64MaxVal, _szDesc) \
	MT_CMD_ITEM_S stItemName = { 	\
		MT_ITEM_TYPE_PTR,   		\
		0,							\
		0,							\
		{_szId},					\
		{_u64MinVal},			    \
		_u64MaxVal,					\
		"<PTR>",					\
		_szDesc,					\
	}

#define MT_DEF_STR_CMD_ITEM(stItemName, _szId, _u64MinVal, _u64MaxVal, _szDesc) \
	MT_CMD_ITEM_S stItemName = { 	\
		MT_ITEM_TYPE_STR,   		\
		0,							\
		0,							\
		{_szId},					\
		{_u64MinVal},			    \
		_u64MaxVal,					\
		"<STRING>",					\
		_szDesc,					\
	}

#define MT_DEF_TEXT_CMD_ITEM(stItemName, _szId, _szName, _szDesc) \
	MT_CMD_ITEM_S stItemName = { 	\
		MT_ITEM_TYPE_TEXT,   		\
		0,							\
		0,							\
		{_szId},					\
		{0},                        \
		0,							\
		_szName,					\
		_szDesc,					\
	}

#define MT_DEF_CHAN_CMD_ITEM(stItemName, _szId, _szDesc) \
	MT_CMD_ITEM_S stItemName = { 	\
		MT_ITEM_TYPE_CHAN,   		\
		0,							\
		0,							\
		{_szId},					\
		{0},                        \
		0,							\
		"<CHANID>",					\
		_szDesc,					\
	}

#define MT_DEF_IP_CMD_ITEM(stItemName, _szId, _szDesc) \
	MT_CMD_ITEM_S stItemName = { 	\
		MT_ITEM_TYPE_IP,   		\
		0,							\
		0,							\
		{_szId},					\
		{0},                        \
		0,							\
		"<IP>",						\
		_szDesc,					\
	}

#define MT_DEF_MOD_CMD_ITEM(stItemName) \
	MT_CMD_ITEM_S stItemName = { 	\
		MT_ITEM_TYPE_MOD,   		\
		0,							\
		0,							\
		{MT_ITEM_ID_MOD_NAME},		\
		{0},                        \
		0,							\
		NULL,					    \
	}

#define MT_PRINT(szFmt, args...)				__MT_PRINT(ulExecId, szFmt, ##args)
#define __MT_PRINT(ulExecId, szFmt, args...)	MT_Print(ulExecId, szFmt, ##args)
#define MT_INIT_ITEM_ARR(pstItemArr)            VA_CB_ZERO(pstItemArr)

#pragma pack (4)

typedef U32 CMD_ID_T;

typedef struct tagMtCmdItem
{
	U8  u8Type;
	U8  u8Flg;
	U16 u16Rsvd;
	union
	{
		S8  as8Id[MT_ID_MAX_LEN];
		U32 u32Id;
	};
	union
	{
		U64  u64MinVal;
		U32  u32SpecItemId;
	};
	U64 u64MaxVal;
	const char *szName;
	const char *szDesc;
}MT_CMD_ITEM_S;

typedef struct tagMtCmdItemArr
{
    MT_CMD_ITEM_S stItem;
    MT_CMD_ITEM_S *apstItem[MT_ITEM_MAX_ARR];
    U32 u32Num;
}MT_CMD_ITEM_ARR_S;

typedef struct tagMtCmdUsrItem
{
    U8   u8Type;
    U8   u8Flg;
    char szName[MT_NAME_MAX_LEN + 1];
    char szDesc[MT_DESC_MAX_LEN + 1];
	union
	{
		S8  as8Id[MT_ID_MAX_LEN];
		U32 u32Id;
	};
	union
	{
		U64  u64MinVal;
		U32  u32SpecItemId;
	};
    U64  u64MaxVal;
}MT_CMD_USR_ITEM_S;

typedef union tagMtVal
{
    U64         u64Val;
    ULONG       ulVal;
    BE32        be32Ip;
    CHAN_ID_S   stChanId;
    S8          s8Str[MT_NAME_MAX_LEN + 1];
}MT_VAL_U;

#pragma pack ()

extern VOID MT_Print(ULONG ulExecId, const char *szFmt, ...);
extern int  MT_MergeItemsToArr(MT_CMD_ITEM_ARR_S *pstItemArr, MT_CMD_ITEM_S *pstItem, ...);

#ifdef  __cplusplus
}
#endif /* end of __cplusplus */

#endif // __MT_DEF__
