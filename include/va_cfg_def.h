#ifndef __VA_CFG_DEF_H__
#define __VA_CFG_DEF_H__

#ifdef  __cplusplus
extern "C"{
#endif

#pragma pack (4)

typedef struct tagVaChanCap
{
    U16 u16ChanType;
    U16 u16PortNum;
    U16 u16ChanNumPerPort;
    U16 u16TotChanNum;
}VA_CHAN_CAP_S;

#pragma pack ()

#ifdef  __cplusplus
}
#endif /* end of __cplusplus */


#endif //__VA_CFG_DEF_H__
