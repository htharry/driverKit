#ifndef __VAA_API_H__
#define __VAA_API_H__

#include "va_usr_pub.h"
#include "va_cfg_def.h"

/*
 vaa video adapter api
 */

#define VA_CP_PATH      VA_FS_PATH "cp"
#define VA_VK_PATH      VA_FS_PATH "vchan"
#define VA_CFG_PATH     VA_FS_PATH "cfg"

#define VAA_CP_IO_CMD(pstChanId, uiCmd, pParams, Fmt, args...)  \
        VA_SND_IO_CMD(gstVaaApiCb.nCpFd, pstChanId, uiCmd, pParams, sizeof(*pParams), Fmt, ##args)

typedef struct tagVaaChanCb
{
    int       nFd;
    CHAN_ID_S stChanId;
}VAA_CHAN_CB_S;

typedef struct tagVaaChanTypeMap
{
    VA_CHAN_CAP_S stCap;
    VAA_CHAN_CB_S *pstChanTbl;
}VAA_CHAN_TYPE_MAP_S;

typedef struct tagVaaApiCb
{
    int nCpFd;
    int nCfgFd;
    VAA_CHAN_TYPE_MAP_S astChanMapTbl[VA_CHAN_TYPE_NUM];
}VAA_API_CB_S;

extern VAA_API_CB_S gstVaaApiCb;
extern int  VAA_MediaInit(void);
extern void VAA_MediaDeInit(void);
extern int  VAA_Open(const char *szFilePath);

#endif //__VAA_API_H__
