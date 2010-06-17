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
#ifndef _IPANEL_MIDDLEWARE_PORTING_MEDIAPLAYER_API_FUNCTOTYPE_H_
#define _IPANEL_MIDDLEWARE_PORTING_MEDIAPLAYER_API_FUNCTOTYPE_H_

#include "ipanel_typedef.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
	IPANEL_MEDIA_TRANS_NET_HFC,
	IPANEL_MEDIA_TRANS_NET_ETHERNET
}IPANEL_MEDIA_TRANS_NET_e;

typedef enum 
{
	IPANEL_MEDIA_GET_DURATION   			= 0,
	IPANEL_MEDIA_GET_STATUS     			= 1,
	IPANEL_MEDIA_GET_RATE       			= 2, 
	IPANEL_MEDIA_GET_POSITION   			= 3,
	IPANEL_MEDIA_GET_STARTTIME  			= 4,
	IPANEL_MEDIA_GET_ENDTIIME   			= 5,
	IPANEL_MEDIA_GET_TRANS_NET  			= 6,
	IPANEL_MEDIA_PUSH_STREAM				= 7,
	IPANEL_MEDIA_CLEAR_BUFFER			    = 8,
	// adec�Ŀ���
	IPANEL_MEDIA_ADEC_SET_CHANNEL_MODE 		= 1000,
	IPANEL_MEDIA_ADEC_SET_MUTE 				= 1001,
	IPANEL_MEDIA_ADEC_SET_PASS_THROUGH  	= 1002,
	IPANEL_MEDIA_ADEC_SET_VOLUME 			= 1003,
	// vdec�Ŀ���
	IPANEL_MEDIA_PLAY_I_FRAME 				= 2000,
	IPANEL_MEDIA_STOP_I_FRAME 				= 2001,
	// display�Ŀ��Ƶȡ�
	IPANEL_MEDIA_DIS_SELECT_DEV 			= 3001,
	IPANEL_MEDIA_DIS_ENABLE_DEV 			= 3002,
	IPANEL_MEDIA_DIS_SET_MODE 				= 3003,
	IPANEL_MEDIA_DIS_SET_VISABLE 			= 3004,
	IPANEL_MEDIA_DIS_SET_ASPECT_RATIO 		= 3005,
	IPANEL_MEDIA_DIS_SET_WIN_LOCATION 		= 3006,
	IPANEL_MEDIA_DIS_SET_WIN_TRANSPARENT 	= 3007,
	IPANEL_MEDIA_DIS_SET_CONTRAST 			= 3008,
	IPANEL_MEDIA_DIS_SET_HUE 				= 3009,
	IPANEL_MEDIA_DIS_SET_BRIGHTNESS 		= 3010,
	IPANEL_MEDIA_DIS_SET_SATURATION 		= 3011,
	IPANEL_MEDIA_DIS_SET_SHARPNESS  		= 3012,
	IPANEL_MEDIA_DIS_SET_HD_RES 			= 3013,
	IPANEL_MEDIA_DIS_SET_IFRAME_LOCATION 	= 3014
} IPANEL_MEDIA_PLAYER_IOCTL_e;

typedef enum 
{
	IPANEL_MEDIA_RUNNING	= 0,
	IPANEL_MEDIA_STOPPED	= 1,
	IPANEL_MEDIA_PAUSED		= 2,
	IPANEL_MEDIA_REWIND 	= 3,
	IPANEL_MEDIA_FORWARD	= 4,
	IPANEL_MEDIA_SLOW		= 5
} MEDIA_STATUS_TYPE_e;	// ������״̬

typedef void (*IPANEL_PLAYER_EVENT_NOTIFY)(UINT32_T player, INT32_T event, void *param);

UINT32_T ipanel_mediaplayer_open(CONST CHAR_T *des, IPANEL_PLAYER_EVENT_NOTIFY cbk);
INT32_T ipanel_mediaplayer_close(UINT32_T player);
INT32_T ipanel_mediaplayer_play(UINT32_T player,CONST BYTE_T *mrl,CONST BYTE_T *des);
INT32_T ipanel_mediaplayer_stop(UINT32_T player);
INT32_T ipanel_mediaplayer_pause(UINT32_T player);
INT32_T ipanel_mediaplayer_resume(UINT32_T player);
INT32_T ipanel_mediaplayer_slow(UINT32_T player, INT32_T rate);
INT32_T ipanel_mediaplayer_forward(UINT32_T player,INT32_T rate);
INT32_T ipanel_mediaplayer_rewind(UINT32_T player, INT32_T rate);
INT32_T ipanel_mediaplayer_seek(UINT32_T player, BYTE_T *pos);
INT32_T ipanel_mediaplayer_ioctl(UINT32_T player ,INT32_T op,UINT32_T *param);
INT32_T ipanel_mediaplayer_start_record(UINT32_T player, CONST BYTE_T *mrl, CONST BYTE_T *device);
INT32_T ipanel_mediaplayer_stop_record(UINT32_T player);

#ifdef __cplusplus
}
#endif

#endif //_IPANEL_MIDDLEWARE_PORTING_MEDIAPLAYER_API_FUNCTOTYPE_H_

