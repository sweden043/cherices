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
#ifndef _IPANEL_MIDDLEWARE_PORTING_VDEC_API_FUNCTOTYPE_H_
#define _IPANEL_MIDDLEWARE_PORTING_VDEC_API_FUNCTOTYPE_H_

#include "ipanel_typedef.h"

#ifdef __cplusplus
extern "C" {
#endif

//--------------------------------------------------------------------------------------------------
//  CONSTANTS DEFINITION
//--------------------------------------------------------------------------------------------------
//
enum
{
    IPANEL_VIDEO_STATUS_IDLE = 1,
    IPANEL_VIDEO_STATUS_PLAY = 2,
    IPANEL_VIDEO_STATUS_STOP = 3
};

//--------------------------------------------------------------------------------------------------
//  TYPES DEFINITION
//--------------------------------------------------------------------------------------------------
//
typedef enum
{
    IPANEL_VDEC_LAST_FRAME,
    IPANEL_VDEC_BLANK
} IPANEL_VDEC_STOP_MODE_e;

typedef struct
{
    INT32_T     len;
    BYTE_T     *data;
} IPANEL_IOCTL_DATA;

typedef enum
{
    IPANEL_VIDEO_STREAM_MPEG1,
    IPANEL_VIDEO_STREAM_MPEG2,
    IPANEL_VIDEO_STREAM_H264,
    IPANEL_VIDEO_STREAM_AVS,
    IPANEL_VIDEO_STREAM_WMV,
    IPANEL_VIDEO_STREAM_MPEG4
} IPANEL_VIDEO_STREAM_TYPE_e;

typedef enum
{
    IPANEL_VDEC_STOPPED,
    IPANEL_VDEC_STARTED,
    IPANEL_VDEC_DECORDING,
    IPANEL_VDEC_HUNGERING,
    IPANEL_VDEC_PAUSED
} IPANEL_VDEC_STATUS_e;

typedef enum
{
    IPANEL_VDEC_SET_SOURCE          = 1,
    IPANEL_VDEC_START               = 2,
    IPANEL_VDEC_STOP                = 3,
    IPANEL_VDEC_PAUSE               = 4,
    IPANEL_VDEC_RESUME              = 5,
    IPANEL_VDEC_CLEAR               = 6,
    IPANEL_VDEC_SYNCHRONIZE         = 7,
    IPANEL_VDEC_GET_BUFFER_RATE     = 8,
    IPANEL_VDEC_PLAY_I_FRAME        = 9,
    IPANEL_VDEC_STOP_I_FRAME        = 10,
    IPANEL_VDEC_SET_STREAM_TYPE     = 11,
    IPANEL_VDEC_SET_SYNC_OFFSET     = 12,
    IPANEL_VDEC_GET_BUFFER          = 13,
    IPANEL_VDEC_PUSH_STREAM         = 14,
    IPANEL_VDEC_GET_BUFFER_CAP      = 15,
    IPANEL_VDEC_SET_CONFIG_PARAMS   = 16,
    IPANEL_VDEC_SET_RATE            = 17,
    IPANEL_VDEC_GET_CURRENT_DTS     = 18,
    IPANEL_VDEC_GET_DEC_STATUS      = 19
} IPANEL_VDEC_IOCTL_e;

//--------------------------------------------------------------------------------------------------
//  EXTERN DATA DEFINITION
//--------------------------------------------------------------------------------------------------
//


//--------------------------------------------------------------------------------------------------
//  EXTERN FUNCTION PROTOTYPES
//--------------------------------------------------------------------------------------------------
//
UINT32_T ipanel_porting_vdec_open(VOID);

INT32_T ipanel_porting_vdec_close(UINT32_T decoder);

INT32_T ipanel_porting_vdec_ioctl(UINT32_T decoder, IPANEL_VDEC_IOCTL_e op, VOID *arg);

INT32_T ipanel_vdec_init(VOID);

VOID ipanel_vdec_exit(VOID);


#ifdef __cplusplus
}
#endif

#endif  // _IPANEL_MIDDLEWARE_PORTING_VDEC_API_FUNCTOTYPE_H_

