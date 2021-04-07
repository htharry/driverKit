#ifndef __VA_CHAN_BASE_MT_H__
#define __VA_CHAN_BASE_MT_H__

#define VA_CHAN_DBG_PRINT(pstChan, u32DbgLevel, szFmt, args...)  \
        do { \
            if (((FDD_CHAN_S *)(pstChan))->u32Debug & u32DbgLevel) \
            { \
                MT_DBG_Print(gstVaChanMgr.pstChanTypeCbTbl[((FDD_CHAN_S *)(pstChan))->stChanId.u16ChanType].szName, szFmt, ##args); \
            } \
        }while(0)

#define VA_CHAN_DBG_PKT_PRINT(pstChan, pData, u32Len, szDescFmt, args...) \
		do { \
			if (((FDD_CHAN_S *)(pstChan))->u32Debug & MT_DBG_PKT) \
			{ \
				MT_DBG_PktPrint((U8 *)pData, u32Len, szDescFmt, ##args); \
			} \
		}while(0)

#define VA_CHAN_DBG_INFO(pstChan, szFmt, args...)      VA_CHAN_DBG_PRINT(pstChan, MT_DBG_INFO, szFmt, ##args)
#define VA_CHAN_DBG_MSG(pstChan, szFmt, args...)       VA_CHAN_DBG_PRINT(pstChan, MT_DBG_MSG,  szFmt, ##args)
#define VA_CHAN_DBG_ERR(pstChan, szFmt, args...)       VA_CHAN_DBG_PRINT(pstChan, MT_DBG_ERR,  szFmt, ##args)



#endif //__VA_CHAN_BASE_MT_H__
