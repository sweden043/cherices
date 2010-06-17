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
#ifndef _IPANEL_MIDDLEWARE_PORTING_FILE_API_FUNCTOTYPE_H_
#define _IPANEL_MIDDLEWARE_PORTING_FILE_API_FUNCTOTYPE_H_

#include "ipanel_typedef.h"

#ifdef __cplusplus
extern "C" {
#endif

//--------------------------------------------------------------------------------------------------
//  CONSTANTS DEFINITION
//--------------------------------------------------------------------------------------------------
//


//--------------------------------------------------------------------------------------------------
//  TYPES DEFINITION
//--------------------------------------------------------------------------------------------------
//
typedef enum
{
    IPANEL_FILE_SEEK_SET            = 1,
    IPANEL_FILE_SEEK_CUR            = 2,
    IPANEL_FILE_SEEK_END            = 3,
    IPANEL_FILE_TRUNCATE            = 4,
    IPANEL_FILE_FLUSH               = 5,
    IPANEL_FILE_GET_POSITION        = 6,
    IPANEL_FILE_GET_CREATE_TIME     = 7,
    IPANEL_FILE_GET_MODIFY_TIME     = 8,
    IPANEL_FILE_GET_LENGTH		    = 9
} IPANEL_FILE_IOCTL_e;

typedef struct
{
    CHAR_T *oldname;
    CHAR_T *newname;
} IPANEL_RENAME_T;

typedef struct
{
    UINT32_T    flag;
    CHAR_T      name[256];
    VOID       *handle;
} IPANEL_DIR;

typedef enum
{
    IPANEL_DIR_RENAMCE_NODE         = 1
} IPANEL_DIR_IOCTL_e;

typedef struct
{
	const char *szfilename; //文件名
	IPANEL_UINT64_T ilength;//文件长度
}IPANEL_FILELENGTH_T;

//--------------------------------------------------------------------------------------------------
//  EXTERN DATA DEFINITION
//--------------------------------------------------------------------------------------------------
//


//--------------------------------------------------------------------------------------------------
//  EXTERN FUNCTION PROTOTYPES
//--------------------------------------------------------------------------------------------------
//
UINT32_T ipanel_porting_file_open(CONST CHAR_T *name, CONST CHAR_T *mode);

INT32_T ipanel_porting_file_close(UINT32_T fd);

INT32_T ipanel_porting_file_delete(CONST CHAR_T *name);

INT32_T ipanel_porting_file_read(UINT32_T fd, BYTE_T *buf, INT32_T len);

INT32_T ipanel_porting_file_write(UINT32_T fd, CONST BYTE_T *buf, INT32_T len);

INT32_T ipanel_porting_file_ioctl(UINT32_T fd, IPANEL_FILE_IOCTL_e op, VOID *arg);

INT32_T ipanel_porting_dir_create(CONST CHAR_T *name);

INT32_T ipanel_porting_dir_remove(CONST CHAR_T *name);

UINT32_T ipanel_porting_dir_open(CONST CHAR_T *name);

INT32_T ipanel_porting_dir_close(UINT32_T dd);

INT32_T ipanel_porting_dir_rewind(UINT32_T dd);

INT32_T ipanel_porting_dir_read(UINT32_T dd, IPANEL_DIR *pdir);

INT32_T ipanel_porting_dir_ioctl(IPANEL_DIR_IOCTL_e op, VOID *arg);

#ifdef __cplusplus
}
#endif

#endif  // _IPANEL_MIDDLEWARE_PORTING_FILE_API_FUNCTOTYPE_H_

