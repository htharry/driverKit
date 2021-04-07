#ifndef __MT_USR_H__
#define __MT_USR_H__

#include "mt_usr_pub.h"
#include "va_chan_def.h"

#pragma pack (4)

typedef struct tagMtUaExecCb
{
    MT_USR_CB_S *pstUsrCb;
    VOID 		*pPrivCb;
    char 		szBuf[MT_STR_MAX_LEN + 1];
}MT_UA_EXEC_CB_S;

#pragma pack ()

#endif
