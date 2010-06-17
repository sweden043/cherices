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
#ifndef _IPANEL_MIDDLEWARE_PORTING_AVM_API_FUNCTOTYPE_H_
#define _IPANEL_MIDDLEWARE_PORTING_AVM_API_FUNCTOTYPE_H_

#include "ipanel_typedef.h"

#ifdef __cplusplus
extern "C" {
#endif

#define IPANEL_CA_MAX_NUM			8

typedef struct 
{
    UINT16_T original_network_id;
    UINT16_T transport_stream_id;
	UINT16_T service_id;
	BYTE_T   serviename[128];
	
	UINT16_T video_pid;
	UINT16_T audio_pid;
	UINT16_T pcr_pid;

	/* 同密情况下, PMT common loop会有多个CA_descriptor */
	UINT16_T pmt_pid;	
	/* PMT PID(一些CA要从PMT的PID开始, ECM/EMM的PID无用!) */
	
	UINT16_T ecmPids[IPANEL_CA_MAX_NUM];
	UINT16_T ecmCaSysIDs[IPANEL_CA_MAX_NUM];
	
	UINT16_T audioEcmPids[IPANEL_CA_MAX_NUM];
	UINT16_T audioCaSysIDs[IPANEL_CA_MAX_NUM];
	UINT16_T videoEcmPids[IPANEL_CA_MAX_NUM];
	UINT16_T videoCaSysIDs[IPANEL_CA_MAX_NUM];
} IPANEL_SERVICE_INFO;

typedef enum
{
	IPANEL_AVM_SET_CHANNEL_MODE = 0,
	IPANEL_AVM_SET_MUTE = 1,
	IPANEL_AVM_SET_PASS_THROUGH = 2,
	IPANEL_AVM_SET_VOLUME = 3,
	IPANEL_AVM_SELECT_DEV = 4,
	IPANEL_AVM_ENABLE_DEV = 5,
	IPANEL_AVM_SET_TVMODE = 6,
	IPANEL_AVM_SET_VISABLE = 7,
	IPANEL_AVM_SET_ASPECT_RATIO = 8,
	IPANEL_AVM_SET_WIN_LOCATION = 9,
	IPANEL_AVM_SET_WIN_TRANSPARENT = 10,
	IPANEL_AVM_SET_CONTRAST = 11,
	IPANEL_AVM_SET_HUE = 12,
	IPANEL_AVM_SET_BRIGHTNESS = 13,
	IPANEL_AVM_SET_SATURATION = 14,
	IPANEL_AVM_SET_SHARPNESS = 15,
	IPANEL_AVM_PLAY_I_FRAME = 16,
	IPANEL_AVM_STOP_I_FRAME = 17,
	IPANEL_AVM_SET_STREAM_TYPE = 18,
	IPANEL_AVM_START_AUDIO = 19,
	IPANEL_AVM_STOP_AUDIO = 20,
	IPANEL_AVM_GET_VOLUME = 21,
	IPANEL_AVM_GET_MUTE = 22
} IPANEL_AVM_IOCTL_e;

INT32_T ipanel_avm_play_srv(IPANEL_SERVICE_INFO *srv);

INT32_T ipanel_avm_stop_srv(VOID);

INT32_T ipanel_avm_ioctl(IPANEL_AVM_IOCTL_e op, VOID *arg);

#ifdef __cplusplus
}
#endif

#endif //_IPANEL_MIDDLEWARE_PORTING_AVM_API_FUNCTOTYPE_H_

