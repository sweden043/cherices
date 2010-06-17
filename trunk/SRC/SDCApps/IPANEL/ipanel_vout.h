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
#ifndef _IPANEL_MIDDLEWARE_PORTING_VOUT_API_FUNCTOTYPE_H_
#define _IPANEL_MIDDLEWARE_PORTING_VOUT_API_FUNCTOTYPE_H_

#include "ipanel_typedef.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum VIDEO_STATUS_tag
{
	VIDEO_NULL_STATUS = 0 ,
	VIDEO_START_STATUS ,
	VIDEO_PAUSE_STATUS ,
	VIDEO_RESUME_STATUS ,
	VIDEO_STOP_STATUS 
}VIDEO_STATUS ;

typedef enum 
{
	IPANEL_DIS_AR_43 ,
	IPANEL_DIS_AR_169
} IPANEL_DIS_AR_e;

typedef enum
{
    IPANEL_VIDEO_OUTPUT_CVBS        = 1 << 0,
    IPANEL_VIDEO_OUTPUT_SVIDEO      = 1 << 1,
    IPANEL_VIDEO_OUTPUT_RGB         = 1 << 2,
    IPANEL_VIDEO_OUTPUT_YPBPR       = 1 << 3,
    IPANEL_VIDEO_OUTPUT_HDMI        = 1 << 4,
    IPANEL_VIDEO_OUTPUT_DVI         = 1 << 5,
    IPANEL_VIDEO_OUTPUT_SCART       = 1 << 6,
    IPANEL_VIDEO_OUTPUT_YCBCR       = 1 << 7,
    IPANEL_VIDEO_OUTPUT_CVBS_Y_C    = 1 << 8,
    IPANEL_VIDEO_OUTPUT_CVBS_RGB    = 1 << 9
} IPANEL_VDIS_VIDEO_OUTPUT_e;

typedef enum
{
    IPANEL_DIS_TVENC_NTSC,
    IPANEL_DIS_TVENC_PAL,
    IPANEL_DIS_TVENC_SECAM,
    IPANEL_DIS_TVENC_AUTO
} IPANEL_DIS_TVENC_MODE_e;

typedef enum
{
    IPANEL_DIS_AR_FULL_SCREEN,
    IPANEL_DIS_AR_PILLARBOX,
    IPANEL_DIS_AR_LETTERBOX,
    IPANEL_DIS_AR_PAN_SCAN
} IPANEL_DIS_AR_MODE_e;

typedef enum
{
    IPANEL_DIS_HD_RES_480I,
    IPANEL_DIS_HD_RES_480P,
    IPANEL_DIS_HD_RES_576I,
    IPANEL_DIS_HD_RES_576P,
    IPANEL_DIS_HD_RES_720P,
    IPANEL_DIS_HD_RES_1080I,
    IPANEL_DIS_HD_RES_1080P
} IPANEL_DIS_HD_RES_e;

typedef struct
{
    UINT32_T x;
    UINT32_T y;
    UINT32_T w;
    UINT32_T h;
} IPANEL_RECT;

typedef enum
{
    IPANEL_DIS_SELECT_DEV           = 1,
    IPANEL_DIS_ENABLE_DEV           = 2,
    IPANEL_DIS_SET_MODE             = 3,
    IPANEL_DIS_SET_VISABLE          = 4,
    IPANEL_DIS_SET_ASPECT_RATIO     = 5,
    IPANEL_DIS_SET_WIN_LOCATION     = 6,
    IPANEL_DIS_SET_WIN_TRANSPARENT  = 7,
    IPANEL_DIS_SET_CONTRAST         = 8,
    IPANEL_DIS_SET_HUE              = 9,
    IPANEL_DIS_SET_BRIGHTNESS       = 10,
    IPANEL_DIS_SET_SATURATION       = 11,
    IPANEL_DIS_SET_SHARPNESS        = 12,
    IPANEL_DIS_SET_HD_RES           = 13,
    IPANEL_DIS_SET_IFRAME_LOCATION  = 14
} IPANEL_DIS_IOCTL_e;

//--------------------------------------------------------------------------------------------------
//  EXTERN DATA DEFINITION
//--------------------------------------------------------------------------------------------------
//


//--------------------------------------------------------------------------------------------------
//  EXTERN FUNCTION PROTOTYPES
//--------------------------------------------------------------------------------------------------
//
UINT32_T ipanel_porting_display_open(VOID);

INT32_T ipanel_porting_display_close(UINT32_T display);

UINT32_T ipanel_porting_display_open_window(UINT32_T display, INT32_T type);

INT32_T ipanel_porting_display_close_window(UINT32_T display, UINT32_T window);

INT32_T ipanel_porting_display_ioctl(UINT32_T display, IPANEL_DIS_IOCTL_e op, VOID *arg);

INT32_T ipanel_video_tvenc_init();

VOID ipanel_video_tvenc_exit();

INT32_T ipanel_set_encoder_output();

#ifdef __cplusplus
}
#endif

#endif  // _IPANEL_MIDDLEWARE_PORTING_VOUT_API_FUNCTOTYPE_H_

