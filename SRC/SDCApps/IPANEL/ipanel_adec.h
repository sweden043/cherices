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
#ifndef _IPANEL_MIDDLEWARE_PORTING_ADEC_API_FUNCTOTYPE_H_
#define _IPANEL_MIDDLEWARE_PORTING_ADEC_API_FUNCTOTYPE_H_

#include "ipanel_typedef.h"

#ifdef __cplusplus
extern "C" {
#endif

enum
{
    IPANEL_AUDIO_STATUS_IDLE = 1,
    IPANEL_AUDIO_STATUS_PLAY = 2,
    IPANEL_AUDIO_STATUS_STOP = 3
};

typedef enum
{
    IPANEL_AUDIO_CODEC_DEFAULT,
    IPANEL_AUDIO_CODEC_MPEG             = 0x3,
    IPANEL_AUDIO_CODEC_MP3              = 0x4,
    IPANEL_AUDIO_CODEC_AAC              = 0xF,
    IPANEL_AUDIO_CODEC_AAC_PLUS         = 0x11,
    IPANEL_AUDIO_CODEC_AC3              = 0x81,
    IPANEL_AUDIO_CODEC_AC3_PLUS         = 0x6
} IPANEL_ADEC_ADUIO_FORMAT_e;

typedef enum
{
    IPANEL_AUDIO_MODE_STEREO            = 0,
    IPANEL_AUDIO_MODE_LEFT_MONO         = 1,
    IPANEL_AUDIO_MODE_RIGHT_MONO        = 2,
    IPANEL_AUDIO_MODE_MIX_MONO          = 3,
    IPANEL_AUDIO_MODE_STEREO_REVERSE    = 4
} IPANEL_ADEC_CHANNEL_OUT_MODE_e;

typedef enum
{
    IPANEL_ADEC_SET_SOURCE              = 1,
    IPANEL_ADEC_START                   = 2,
    IPANEL_ADEC_STOP                    = 3,
    IPANEL_ADEC_PAUSE                   = 4,
    IPANEL_ADEC_RESUME                  = 5,
    IPANEL_ADEC_CLEAR                   = 6,
    IPANEL_ADEC_SYNCHRONIZE             = 7,
    IPANEL_ADEC_SET_CHANNEL_MODE        = 8,
    IPANEL_ADEC_SET_MUTE                = 9,
    IPANEL_ADEC_SET_PASS_THROUGH        = 10,
    IPANEL_ADEC_SET_VOLUME              = 11,
    IPANEL_ADEC_GET_BUFFER_RATE         = 12,
    IPANEL_ADEC_SET_SYNC_OFFSET         = 13,
    IPANEL_ADEC_GET_BUFFER              = 14,
    IPANEL_ADEC_PUSH_STREAM             = 15,
    IPANEL_ADEC_GET_BUFFER_CAP          = 16,
    IPANEL_ADEC_SET_CONFIG_PARAMS       = 17,
    IPANEL_ADEC_GET_CURRENT_PTS         = 18
} IPANEL_ADEC_IOCTL_e;

typedef struct
{
    IPANEL_XMEM_PAYLOAD_TYPE_e  destype;
    UINT32_T                    timestamp;
    BYTE_T                     *ppayload;
    UINT32_T                    len;
} IPANEL_XMEM_ES_DES;

//--------------------------------------------------------------------------------------------------
//  EXTERN DATA DEFINITION
//--------------------------------------------------------------------------------------------------
//


//--------------------------------------------------------------------------------------------------
//  EXTERN FUNCTION PROTOTYPES
//--------------------------------------------------------------------------------------------------
//
/*��һ�����뵥Ԫ*/
UINT32_T ipanel_porting_adec_open(VOID);

/*�ر�ָ���Ľ��뵥Ԫ*/
INT32_T ipanel_porting_adec_close(UINT32_T decoder);

/*��decoder����һ�������������������úͻ�ȡdecoder�豸�Ĳ���������*/
INT32_T ipanel_porting_adec_ioctl(UINT32_T decoder, IPANEL_ADEC_IOCTL_e op, VOID *arg);

INT32_T ipanel_adec_init(VOID);

VOID ipanel_adec_exit(VOID);

#ifdef __cplusplus
}
#endif

#endif  // _IPANEL_MIDDLEWARE_PORTING_ADEC_API_FUNCTOTYPE_H_

