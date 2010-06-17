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
#ifndef _IPANEL_MIDDLEWARE_PORTING_APP_LOADER_FUNCTOTYPE_H_
#define _IPANEL_MIDDLEWARE_PORTING_APP_LOADER_FUNCTOTYPE_H_

#include "ipanel_typedef.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
	IPANEL_APP_EVENT_TYPE_NONE,
	IPANEL_APP_EVENT_TYPE_DOWNLOADER,//下载器消息
	IPANEL_APP_EVENT_TYPE_UNDEFINED
};

typedef enum
{
	IPANEL_APP_DOWNLOADER_NONE,
	IPANEL_APP_DOWNLOADER_START,//通知第三方应用已开始下载文件
	IPANEL_APP_DOWNLOADER_STOP,//通知第三方应用已停止下载文件
	IPANEL_APP_DOWNLOADER_SUCCESS,//通知第三方应用某个文件下载成功
	IPANEL_APP_DOWNLOADER_FAILED,//通知第三方应用某个文件下载失败
	IPANEL_APP_DOWNLOADER_UNDEFINED
};

typedef struct tagFILEINFO
{
	char *name;
	char *buf;
	int32 len;
}FileInfo;

int32 ipanel_app_process_event(int msg, unsigned int p1, unsigned int p2);
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
typedef enum 
{
	IPANEL_DOWNLOADER_UNDEF,
	IPANEL_DOWNLOADER_SUCCESS,
	IPANEL_DOWNLOADER_TIMEOUT,
	IPANEL_DOWNLOADER_NOT_FOUND,
	IPANEL_DOWNLOADER_SAVE_FAILED,
	IPANEL_DOWNLOADER_HALT
};

typedef INT32_T (*IPANEL_DOWNLOADER_NOTIFY)(VOID *tag, CONST CHAR_T *uri, INT32_T status);

INT32_T ipanel_download_file(VOID *handle, CONST CHAR_T *uri, UINT32_T timeout, IPANEL_DOWNLOADER_NOTIFY func, VOID *tag);
//////////////////////////////////////////////////////////////////////////

#ifdef __cplusplus
}
#endif

#endif /* _IPANEL_MIDDLEWARE_PORTING_APP_LOADER_FUNCTOTYPE_H_ */

