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
#ifndef _IPANEL_MIDDLEWARE_PORTING_STORAGE_API_FUNCTOTYPE_H_
#define _IPANEL_MIDDLEWARE_PORTING_STORAGE_API_FUNCTOTYPE_H_

#include "ipanel_typedef.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
    IPANEL_STORAGE_GET_DEV_NUM 		= 1,
    IPANEL_STORAGE_GET_LOGICDEV_NUM = 2,
    IPANEL_STORAGE_GET_DEV_INFO 	= 3,
    IPNAEL_STORAGE_FORMAT_DEV 		= 4,
    IPANEL_STORAGE_REMOVE_DEV 		= 5,
    IPANEL_STORAGE_CREATE_LOGICDEV	= 6,
    IPANEL_STORAGE_DELETE_LOGICDEV	= 7
} IPANEL_STORAGE_IOCTL_e;

typedef enum
{
    IPANEL_STORAGE_DRIVE_UNKNOWN 	= 0,	/* The drive type cannot be determined.  */
	IPANEL_STORAGE_DRIVE_REMOVABLE	= 1,	/* The disk can be removed from the drive. */
	IPANEL_STORAGE_DRIVE_FIXED		= 2,	/* The disk cannot be removed from the drive. */
	IPANEL_STORAGE_DRIVE_REMOTE		= 3,	/* The drive is a remote (network) drive. */
	IPANEL_STORAGE_DRIVE_CDROM		= 4,	/* The drive is a CD-ROM drive. */
	IPANEL_STORAGE_DRIVE_RAMDISK	= 5		/* The drive is a RAM disk. */
} IPANEL_STORAGE_DRIVE_TYPE_e;

typedef struct
{
	INT32_T index;						/*�豸������,��־һ�������豸*/
	INT32_T subidx;						/*�߼��豸������,��־һ�������豸�ϵ��߼�������0��ʾ�������豸��������0����ʾ��ѯ�����豸�ϵ��߼��豸*/
	CHAR_T logic;						/*�߼��豸�ţ�һ�������豸�Ͽ����ж���߼��豸*/
	CHAR_T name[32];					/*�߼��豸����*/
	IPANEL_STORAGE_DRIVE_TYPE_e type;	/*�����豸����*/
	UINT32_T start;						/*�߼��豸�ռ���ʼλ�ã���KBΪ��λ*/
	UINT32_T size;						/*������߼��豸�洢�ռ��С����KBΪ��λ*/
	UINT32_T free;						/*������߼��豸���пռ��С����KBΪ��λ*/
}IPANEL_STORAGE_DEV_INFO;

INT32_T ipanel_porting_storage_ioctl(IPANEL_STORAGE_IOCTL_e cmd, VOID *arg);

#ifdef __cplusplus
}
#endif

#endif//_IPANEL_MIDDLEWARE_PORTING_STORAGE_API_FUNCTOTYPE_H_
