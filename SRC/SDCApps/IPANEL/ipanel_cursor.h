/*********************************************************************
    Copyright (c) 2008 - 2010 Embedded Internet Solutions, Inc
    All rights reserved. You are not allowed to copy or distribute
    the code without permission.
    This is the demo implenment of the base Porting APIs needed by iPanel MiddleWare. 
    Maybe you should modify it accorrding to Platform.
    
    Note: the "int" in the file is 32bits
    
    History:
		version		date		name		desc
         0.01     2009/8/1     Vicegod     create
*********************************************************************/
#ifndef _IPANEL_MIDDLEWARE_PORTING_CURSOR_API_FUNCTOTYPE_H_
#define _IPANEL_MIDDLEWARE_PORTING_CURSOR_API_FUNCTOTYPE_H_

#include "ipanel_typedef.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
	IPANEL_PIXEL_4BPP,
	IPANEL_PIXEL_8BPP,
	IPANEL_PIXEL_ARGB1555,
	IPANEL_PIXEL_ARGB565,
	IPANEL_PIXEL_ARGB8888
}IPANEL_PIXEL_FORMAT_e;

typedef enum
{
	IPANEL_CURSOR_REGISTER_BITMAP		= 1,
	IPANEL_CURSOR_GET_DEV_NUM			= 2,
	IPANEL_CURSOR_GET_DEV_INFO			= 3
}IPANEL_CURSOR_IOCTL_e;

typedef struct 
{
	UINT16_T							index;			/*光标索引号*/
	IPANEL_PIXEL_FORMAT_e				format;			/*位图颜色格式*/
	UINT16_T							w;				/*位图宽度*/
	UINT16_T							h;				/*位图高度*/
	UINT32_T							*pbitmap;
}IPANEL_CURSOR_BITMAP;

typedef enum 
{
	IPANEL_CURSOR_PS2					= 1,
	IPANEL_CURSOR_USB_HID				= 2,
	IPANEL_CURSOR_TOUCHPAD				= 3
}IPANEL_CURSOR_TYPE_e;

typedef struct 
{
	INT32_T								index;
	IPANEL_CURSOR_TYPE_e				type;
	CHAR_T								name[32];
}IPANEL_CURSOR_DEV_INFO;

INT32_T ipanel_porting_cursor_get_position(INT32_T *x, INT32_T *y);

INT32_T ipanel_porting_cursor_set_position(INT32_T x, INT32_T y);

INT32_T ipanel_porting_cursor_get_shape(VOID);

INT32_T ipanel_porting_cursor_set_shape(INT32_T shape);

INT32_T ipanel_porting_cursor_show(INT32_T showflag);

INT32_T ipanel_porting_cursor_ioctl(IPANEL_CURSOR_IOCTL_e cmd, VOID *arg);

#ifdef __cplusplus
}
#endif

#endif//_IPANEL_MIDDLEWARE_PORTING_CURSOR_API_FUNCTOTYPE_H_
