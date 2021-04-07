#ifndef __VA_MT_H__
#define __VA_MT_H__

#define VA_MT_DBG_MSG_PRINT(args...)     MT_DBG_MSG_PRINT(&gstVaCoreMt, ##args)
#define VA_MT_DBG_ERR_PRINT(args...)     MT_DBG_ERR_PRINT(&gstVaCoreMt, ##args)


extern MT_CB_S gstVaCoreMt;

#endif //__VA_MT_H__
