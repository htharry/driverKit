#ifndef __VA_IO_CMD_H__
#define __VA_IO_CMD_H__

#ifdef  __cplusplus
extern "C"{
#endif

#define VA_IO_CMD_MAX_ARG_LEN       240

#define MT_IOCTL_CMD_IA_BASE		0x1000
#define MT_IOCTL_GET_CL_NUM			(MT_IOCTL_CMD_IA_BASE + 0x0)
#define MT_IOCTL_GET_ALL_CMD_ID		(MT_IOCTL_CMD_IA_BASE + 0x1)
#define MT_IOCTL_GET_CL_INFO		(MT_IOCTL_CMD_IA_BASE + 0x2)
#define MT_IOCTL_EXEC_CL			(MT_IOCTL_CMD_IA_BASE + 0x3)
#define MT_IOCTL_SET_GLB_DBG		(MT_IOCTL_CMD_IA_BASE + 0x4)
#define MT_IOCTL_GET_ALL_VIEW_ID	(MT_IOCTL_CMD_IA_BASE + 0x5)
#define MT_IOCTL_GET_VIEW_INFO		(MT_IOCTL_CMD_IA_BASE + 0x6)
#define MT_IOCTL_GET_CL_CHANGE_INFO	(MT_IOCTL_CMD_IA_BASE + 0x7)

#define MT_IOCTL_CMD_UA_BASE		0x2000
#define MT_IOCTL_UA_OPEN			(MT_IOCTL_CMD_UA_BASE + 0x0)
#define MT_IOCTL_UA_REG_CL			(MT_IOCTL_CMD_UA_BASE + 0x1)
#define MT_IOCTL_UA_GET_VAL			(MT_IOCTL_CMD_UA_BASE + 0x2)
#define MT_IOCTL_UA_GET_EXEC_PARAM	(MT_IOCTL_CMD_UA_BASE + 0x3)
#define MT_IOCTL_UA_PRINT			(MT_IOCTL_CMD_UA_BASE + 0x4)
#define MT_IOCTL_UA_END_EXEC		(MT_IOCTL_CMD_UA_BASE + 0x5)
#define MT_IOCTL_UA_DBG_PRINT		(MT_IOCTL_CMD_UA_BASE + 0x6)
#define MT_IOCTL_UA_GET_VAL_ARR		(MT_IOCTL_CMD_UA_BASE + 0x7)

#define VK_IOCTL_BASE               0x3000
#define VK_IOCTL_BIND_FD            (VK_IOCTL_BASE + 0x0)
#define VK_IOCTL_PUT_DATA           (VK_IOCTL_BASE + 0x1)
#define VK_IOCTL_GET_DATA           (VK_IOCTL_BASE + 0x2)

#define CFG_IOCTL_BASE              0x3800
#define CFG_IOCTL_GET_CHAN_CAP      (CFG_IOCTL_BASE + 0x0)


#define CP_IOCTL_GLOBAL_BASE        0x100000        // it's global cmd, not for a channel ioctl cmd, per gap is 0x100 count

#define CP_IOCTL_CHAN_BASE          (CP_IOCTL_GLOBAL_BASE)
#define CP_IOCTL_CHAN_CONN          (CP_IOCTL_CHAN_BASE + 0x0)
#define CP_IOCTL_CHAN_DISCONN       (CP_IOCTL_CHAN_BASE + 0x1)

#define VN_IOCTL_BASE               (CP_IOCTL_GLOBAL_BASE + 0x100)
#define VN_IOCTL_ADD_NODE           (VN_IOCTL_BASE + 0x0)
#define VN_IOCTL_DEL_NODE           (VN_IOCTL_BASE + 0x1)


#define __VA_FILL_IO_CMD(stIoCmd, pstChanId, u8CmdLen, pParams)  \
          do {                                                   \
              stIoCmd.stChanId  = *pstChanId;                    \
              stIoCmd.u8Len     = (U8)u8CmdLen;                  \
              stIoCmd.u64Param  = (U64)(ULONG)pParams;           \
          } while(0)

#define VA_FILL_IO_CMD(stIoCmd, pstChanId, pParams) \
         __VA_FILL_IO_CMD(stIoCmd, pstChanId, sizeof(*(pParams)), pParams)

#define VA_SND_IO_CMD(nFd, pstChanId, uiCmd, pParams, ucParamLen, Fmt, args...) \
        do { \
            VA_IO_CMD_S __stIoCmd; \
            INT __iRet; \
            \
            __VA_FILL_IO_CMD(__stIoCmd, pstChanId, ucParamLen, pParams); \
            __iRet = ioctl(nFd, uiCmd, &__stIoCmd);\
            if ( __iRet < 0 ) \
            { \
                return VA_E_SYS_FAILED; \
            }\
        }while(0)


#define MT_IO_CMD_FMT       MT_DBG_HEX MT_DBG_CHAN MT_DBG_U
#define MT_IO_CMD_ARGS(u32Cmd, pstIoCmd)      "Cmd", u32Cmd, "Chan Id", VA_CHAN_ARGS(&(pstIoCmd)->stChanId), "Len", (pstIoCmd)->u8Len

#pragma pack (4)
typedef struct tagVaIoCmd
{
    CHAN_ID_S 	stChanId;
    U16         u16Ack;
    U8          u8Len;
    U8          u8Rsvd1;
    union
    {
        U64   u64Param;
        VOID  __user *pParam;
    };
}VA_IO_CMD_S;

#pragma pack ()

#ifdef  __cplusplus
}
#endif /* end of __cplusplus */

#endif //__VA_IO_CMD_H__
