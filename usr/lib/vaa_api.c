#include "vaa_api.h"

VAA_API_CB_S gstVaaApiCb =
{
    .nCpFd  = -1,
    .nCfgFd = -1,
};

int VAA_Open(const char *szFilePath)
{
    int nFd;
    int iRet;

    nFd = open(szFilePath, O_RDONLY);
    if ( nFd < 0 )
    {
        VA_LOG_ERR("Failed to open %s", szFilePath);
        return nFd;
    }

    iRet = fcntl(nFd, F_GETFD);
    if ( iRet >= 0 )
    {
        fcntl(nFd, F_SETFD, FD_CLOEXEC|iRet);
    }

    return nFd;
}

int VAA_Init(void)
{
    int nRet;

    gstVaaApiCb.nCpFd = VAA_Open(VA_CP_PATH);
    if ( gstVaaApiCb.nCpFd < 0 )
    {
        VA_LOG_ERR("no cp device");
        return VA_E_NO_DEV;
    }

    gstVaaApiCb.nCfgFd = VAA_Open(VA_CFG_PATH);
    if ( gstVaaApiCb.nCpFd < 0 )
    {
        VA_LOG_ERR("no cfg device");
        return VA_E_NO_DEV;
    }

    nRet = VAA_MediaInit();
    if ( nRet != VA_SUCCESS )
    {
        return nRet;
    }

    return VA_SUCCESS;
}

void VAA_DeInit(void)
{
    if ( gstVaaApiCb.nCpFd >= 0 )
    {
        close(gstVaaApiCb.nCpFd);
    }

    if ( gstVaaApiCb.nCfgFd >= 0 )
    {
        close(gstVaaApiCb.nCfgFd);
    }

    VAA_DeInit();
    return;
}

