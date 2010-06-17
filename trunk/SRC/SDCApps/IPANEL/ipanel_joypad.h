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
#ifndef _IPANEL_MIDDLEWARE_PORTING_JOYPAD_API_FUNCTOTYPE_H_
#define _IPANEL_MIDDLEWARE_PORTING_JOYPAD_API_FUNCTOTYPE_H_

#include "ipanel_typedef.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
    IPANEL_JOYPAD_GET_DEV_NUM 	= 1,
    IPANEL_JOYPAD_GET_DEV_INFO 	= 2,
    IPANEL_JOYPAD_SEND_MSG 		= 3
} IPANEL_JOYPAD_IOCTL_e;

typedef struct 
{
    INT32_T id; 		//设备标识
    CHAR_T  name[256];	//设备的名称，如果没有或者无法获知可返回字符串"NULL"
    INT32_T stateless;	//是否是无状态设备，不是则赋值0，否则一律赋值为为非零值(包括不可获知的情况）
}IPANEL_JOYPAD_DEV_INFO;

typedef struct
{
    INT32_T id; 	//设备的ID
    CHAR_T 	*pmsg; 	//信息内容
    INT32_T len; 	//信息长度
}IPANEL_JOYPAD_MSG;

INT32_T ipanel_porting_joypad_ioctl(IPANEL_JOYPAD_IOCTL_e cmd, VOID *arg);

#ifdef __cplusplus
}
#endif

#endif //_IPANEL_MIDDLEWARE_PORTING_JOYPAD_API_FUNCTOTYPE_H_

