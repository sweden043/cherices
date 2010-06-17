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
#ifndef _IPANEL_MIDDLEWARE_PORTING_SOUND_API_FUNCTOTYPE_H_
#define _IPANEL_MIDDLEWARE_PORTING_SOUND_API_FUNCTOTYPE_H_

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
    IPANEL_AUDIO_DATA_CONSUMED,
    IPANEL_AUDIO_DATA_LACK
} IPANEL_AUDIO_MIXER_EVENT;

typedef VOID (*IPANEL_AUDIO_MIXER_NOTIFY)(
                    UINT32_T                    hmixer,
                    IPANEL_AUDIO_MIXER_EVENT    event,
                    UINT32_T                   *param
                );

typedef enum
{
    IPANEL_DEV_USE_SHARED,
    IPANEL_DEV_USE_EXCUSIVE
} IPANEL_DEV_USE_MODE;

typedef enum
{
    IPANEL_MIXER_SET_VOLUME     = 1,
    IPANEL_MIXER_CLEAR_BUFFER   = 2,
    IPANEL_MIXER_PAUSE          = 3,
    IPANEL_MIXER_RESUME         = 4
} IPANEL_MIXER_IOCTL_e;

//--------------------------------------------------------------------------------------------------
//  CONSTANTS DEFINITION
//--------------------------------------------------------------------------------------------------
//
#define IPANEL_SHARE_MAX_HANDLE    8 // 共享方式设备最大数目
#define IPANEL_SOUND_PLAYS_COUNT  16 // 最大的播放队列长度
#define IPANEL_SOUND_EVENT_COUNT  16 // 最大的事件播放长度

typedef enum
{
	IPANEL_MSG_DATA,
	IPANEL_MSG_DESTROY
}IPANEL_MSG_TYPE;

typedef struct tagIPANEL_SOUND_CONTEXT_S
{
	UINT32_T 	PlayQue;
	UINT32_T 	taskId;
	UINT32_T	pcm;
	INT32_T     play_set_flag;
	IPANEL_AUDIO_MIXER_NOTIFY	func;
}IPANEL_SOUND_CONTEXT_s;

//--------------------------------------------------------------------------------------------------
//  EXTERN FUNCTION PROTOTYPES
//--------------------------------------------------------------------------------------------------
//
UINT32_T ipanel_porting_audio_mixer_open(IPANEL_DEV_USE_MODE mode, IPANEL_AUDIO_MIXER_NOTIFY func);

INT32_T ipanel_porting_audio_mixer_close(UINT32_T handle);

IPANEL_XMEMBLK *ipanel_porting_audio_mixer_memblk_get(UINT32_T handle, UINT32_T size);

INT32_T ipanel_porting_audio_mixer_memblk_send(UINT32_T handle, IPANEL_XMEMBLK *pcmblk);

INT32_T ipanel_porting_audio_mixer_ioctl(UINT32_T handle, IPANEL_MIXER_IOCTL_e op, VOID *arg);

///////////////////////////////////////////////////////////////////////////
/* 声音采集设备接口*/

typedef enum
{
	IPANEL_MIC_START=1,
	IPANEL_MIC_STOP,
	IPANEL_MIC_CLEAR_BUFFER,
	IPANEL_MIC_SET_PARAM
} IPANEL_MIC_IOCTL_e;

typedef void (*IPANEL_MIC_NOTIFY)( UINT32_T handle, INT32_T event, VOID *param);

UINT32_T ipanel_porting_mic_open(IPANEL_MIC_NOTIFY func);

INT32_T ipanel_porting_mic_close(UINT32_T handle);

INT32_T ipanel_porting_mic_ioctl(UINT32_T handle, IPANEL_MIC_IOCTL_e op, VOID *arg);

INT32_T ipanel_porting_mic_read(UINT32_T handle, BYTE_T *buf);

int ipanel_sound_init();

void ipanel_sound_exit();

#ifdef __cplusplus
}
#endif

#endif  // _IPANEL_MIDDLEWARE_PORTING_SOUND_API_FUNCTOTYPE_H_

