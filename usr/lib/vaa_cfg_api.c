#include "vaa_api.h"
#include "va_cfg_def.h"

U16 VAA_GetChanNum(U16 u16ChanType)
{
    VA_CHAN_CAP_S stCap;
    LONG lRet;

    stCap.u16ChanType = u16ChanType;
    lRet = ioctl(gstVaaApiCb.nCfgFd, CFG_IOCTL_GET_CHAN_CAP, &stCap);
    if ( lRet < 0 )
    {
        return 0;
    }

    return stCap.u16TotChanNum;
}

VOID VAA_GetChanCap(VA_CHAN_CAP_S *pstCap)
{
    U16 u16ChanType = pstCap->u16ChanType;

    VA_CB_ZERO(pstCap);

    pstCap->u16ChanType = u16ChanType;

    ioctl(gstVaaApiCb.nCfgFd, CFG_IOCTL_GET_CHAN_CAP, pstCap);

    pstCap->u16ChanType = u16ChanType;
    return;
}


