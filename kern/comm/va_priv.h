#ifndef __VA_PRIV_H__
#define __VA_PRIV_H__

typedef struct tagVaCb
{
	struct list_head stModHead;
	struct list_head stDrvHead;
	struct list_head stDevHead;
	struct mutex stMutex;
}VA_CB_S;

#endif //__VA_PRIV_H__