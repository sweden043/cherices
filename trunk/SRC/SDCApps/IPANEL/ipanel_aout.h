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
#ifndef _IPANEL_MIDDLEWARE_PORTING_AOUT_API_FUNCTOTYPE_H_
#define _IPANEL_MIDDLEWARE_PORTING_AOUT_API_FUNCTOTYPE_H_

#include "ipanel_typedef.h"

#ifdef __cplusplus
extern "C" {
#endif


//--------------------------------------------------------------------------------------------------
//  TYPES DEFINITION
//--------------------------------------------------------------------------------------------------
//
typedef enum
{
    IPANEL_AOUT_DEVICE_ANALOG_STERO     = 0x01,     // ģ�����������
    IPANEL_AOUT_DEVICE_ANALOG_MUTI      = 0x02,     // ģ����������
    IPANEL_AOUT_DEVICE_SPDIF            = 0x04,     // S/PDIF���
    IPANEL_AOUT_DEVICE_HDMI             = 0x08,     // HDMI���
    IPANEL_AOUT_DEVICE_I2S              = 0x10,     // I2S���
    IPANEL_AOUT_DEVICE_ALL              = 0xff      // ���ж˿�
} IPANEL_AOUT_DEVICE_e;

typedef enum
{
    IPANEL_AOUT_SET_OUTPUT      = 1,
    IPANEL_AOUT_SET_VOLUME      = 2,
    IPANEL_AOUT_SET_BALANCE     = 3,
    IPANEL_AOUT_GET_VOLUME	    = 4
} IPANEL_AOUT_IOCTL_e;

//--------------------------------------------------------------------------------------------------
//  EXTERN DATA DEFINITION
//--------------------------------------------------------------------------------------------------
//


//--------------------------------------------------------------------------------------------------
//  EXTERN FUNCTION PROTOTYPES
//--------------------------------------------------------------------------------------------------
//
/*����Ƶ�������Ԫ*/
UINT32_T ipanel_porting_audio_output_open(VOID);

/*�ر�ָ������Ƶ�����Ԫ*/	
INT32_T ipanel_porting_audio_output_close(UINT32_T handle);

/*������������ý���һ�������������������úͻ�ȡ��������豸�Ĳ���������*/
INT32_T ipanel_porting_audio_output_ioctl(UINT32_T handle, IPANEL_AOUT_IOCTL_e op, VOID *arg);

#ifdef __cplusplus
}
#endif

#endif  // _IPANEL_MIDDLEWARE_PORTING_AOUT_API_FUNCTOTYPE_H_

