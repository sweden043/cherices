/****************************************************************************/
/*                   CONEXANT PROPRIETARY AND CONFIDENTIAL                  */
/*                       SOFTWARE FILE/MODULE HEADER                        */
/*                Copyright Conexant Systems Inc. 1998-2003                 */
/*                         All Rights Reserved                              */
/****************************************************************************/
/*
 * Filename:        tvenc.h
 *
 *
 * Description:     Public header file for TV encoder driver
 *
 *
 * Author:          Xin Golden (based on the current encoder.h in pvcs\include)
 *
 ****************************************************************************/
/* $Header: tvenc.h, 2, 10/24/03 11:48:29 AM, Xin Golden$
 ****************************************************************************/

#ifndef _TVENC_H_
#define _TVENC_H_

/*****************/
/* Include Files */
/*****************/

/********************************/
/* Symbol and Macro definitions */
/********************************/

/* Bit flags used to define controllability of parameters */
#define TVENC_CONTROL_BRIGHTNESS 0x01
#define TVENC_CONTROL_CONTRAST   0x02
#define TVENC_CONTROL_SATURATION 0x04
#define TVENC_CONTROL_HUE        0x08
#define TVENC_CONTROL_SHARPNESS  0x10

/* Bit flags used to define VBI lines */
#define CNXT_TVENC_VBI_LINE_1   0x00000001
#define CNXT_TVENC_VBI_LINE_2   0x00000002
#define CNXT_TVENC_VBI_LINE_3   0x00000004
#define CNXT_TVENC_VBI_LINE_4   0x00000008 
#define CNXT_TVENC_VBI_LINE_5   0x00000010
#define CNXT_TVENC_VBI_LINE_6   0x00000020
#define CNXT_TVENC_VBI_LINE_7   0x00000040
#define CNXT_TVENC_VBI_LINE_8   0x00000080
#define CNXT_TVENC_VBI_LINE_9   0x00000100
#define CNXT_TVENC_VBI_LINE_10   0x00000200
#define CNXT_TVENC_VBI_LINE_11   0x00000400
#define CNXT_TVENC_VBI_LINE_12   0x00000800
#define CNXT_TVENC_VBI_LINE_13   0x00001000
#define CNXT_TVENC_VBI_LINE_14   0x00002000
#define CNXT_TVENC_VBI_LINE_15   0x00004000
#define CNXT_TVENC_VBI_LINE_16   0x00008000
#define CNXT_TVENC_VBI_LINE_17   0x00010000
#define CNXT_TVENC_VBI_LINE_18   0x00020000
#define CNXT_TVENC_VBI_LINE_19   0x00040000
#define CNXT_TVENC_VBI_LINE_20   0x00080000
#define CNXT_TVENC_VBI_LINE_21   0x00100000
#define CNXT_TVENC_VBI_LINE_22   0x00200000
#define CNXT_TVENC_VBI_LINE_23   0x00400000
#define CNXT_TVENC_VBI_LINE_24   0x00800000

#define CNXT_TVENC_VBI_LINE_314  CNXT_TVENC_VBI_LINE_1
#define CNXT_TVENC_VBI_LINE_315  CNXT_TVENC_VBI_LINE_2
#define CNXT_TVENC_VBI_LINE_316  CNXT_TVENC_VBI_LINE_3
#define CNXT_TVENC_VBI_LINE_317  CNXT_TVENC_VBI_LINE_4
#define CNXT_TVENC_VBI_LINE_318  CNXT_TVENC_VBI_LINE_5
#define CNXT_TVENC_VBI_LINE_319  CNXT_TVENC_VBI_LINE_6
#define CNXT_TVENC_VBI_LINE_320  CNXT_TVENC_VBI_LINE_7
#define CNXT_TVENC_VBI_LINE_321  CNXT_TVENC_VBI_LINE_8
#define CNXT_TVENC_VBI_LINE_322  CNXT_TVENC_VBI_LINE_9
#define CNXT_TVENC_VBI_LINE_323  CNXT_TVENC_VBI_LINE_10
#define CNXT_TVENC_VBI_LINE_324  CNXT_TVENC_VBI_LINE_11
#define CNXT_TVENC_VBI_LINE_325  CNXT_TVENC_VBI_LINE_12
#define CNXT_TVENC_VBI_LINE_326  CNXT_TVENC_VBI_LINE_13
#define CNXT_TVENC_VBI_LINE_327  CNXT_TVENC_VBI_LINE_14
#define CNXT_TVENC_VBI_LINE_328  CNXT_TVENC_VBI_LINE_15
#define CNXT_TVENC_VBI_LINE_329  CNXT_TVENC_VBI_LINE_16
#define CNXT_TVENC_VBI_LINE_330  CNXT_TVENC_VBI_LINE_17
#define CNXT_TVENC_VBI_LINE_331  CNXT_TVENC_VBI_LINE_18
#define CNXT_TVENC_VBI_LINE_332  CNXT_TVENC_VBI_LINE_19
#define CNXT_TVENC_VBI_LINE_333  CNXT_TVENC_VBI_LINE_20
#define CNXT_TVENC_VBI_LINE_334  CNXT_TVENC_VBI_LINE_21
#define CNXT_TVENC_VBI_LINE_335  CNXT_TVENC_VBI_LINE_22
#define CNXT_TVENC_VBI_LINE_336  CNXT_TVENC_VBI_LINE_23
#define CNXT_TVENC_VBI_LINE_337  CNXT_TVENC_VBI_LINE_24

/*****************/
/* Data Types    */
/*****************/

/* return values of APIs */
/* Return values of APIs */
typedef enum
{
   CNXT_TVENC_OK = 0,
   CNXT_TVENC_ALREADY_INIT,
   CNXT_TVENC_NOT_INIT,
   CNXT_TVENC_BAD_UNIT,
   CNXT_TVENC_CLOSED_HANDLE,
   CNXT_TVENC_BAD_HANDLE,
   CNXT_TVENC_BAD_PARAMETER,
   CNXT_TVENC_RESOURCE_ERROR,
   CNXT_TVENC_INTERNAL_ERROR,
   CNXT_TVENC_NOT_AVAILABLE,
   CNXT_TVENC_CC_NOT_INIT,
   CNXT_TVENC_CONFIG_WARNING
} CNXT_TVENC_STATUS;


/* Driver configuration structure */
typedef void* CNXT_TVENC_CONFIG;     /* not used */

/* events */
typedef enum
{
   CNXT_TVENC_EVENT_TERM,
   CNXT_TVENC_EVENT_VIDEO_STANDARD_SET

} CNXT_TVENC_EVENT, *PCNXT_TVENC_EVENT;

/* Device capability structure */
typedef struct
{
   u_int32 uLength;
   bool    bExclusive;
   u_int8  uControls; /* Indicate which picture controls are supported */
   u_int8  uConnections;  /* Indicate which video output connection types are supported */
} CNXT_TVENC_CAPS, *PCNXT_TVENC_CAPS;


/* Picture controls */
typedef enum 
{
   CNXT_TVENC_PCTRL_INVALID = -1,
   CNXT_TVENC_BRIGHTNESS,
   CNXT_TVENC_CONTRAST,
   CNXT_TVENC_SATURATION,
   CNXT_TVENC_HUE,
   CNXT_TVENC_SHARPNESS
} CNXT_TVENC_CONTROL, *PCNXT_TVENC_CONTROL;


/* enum for video standard */
/* !!! It's important that the order of items not to be changed     */
/* !!! the enum is referenced by two other drivers: AVID and ATVTUN */
/* !!! to add or delete any item in the list, please consultant with Tim Ross */
typedef enum {
   CNXT_TVENC_STANDARD_INVALID = -1, /* MUST be 1st item in list!!! */
   CNXT_TVENC_NTSC_M,                /* NTSC-M (N. America, Taiwan */
   CNXT_TVENC_NTSC_JAPAN,            /* NTSC Japan   */   
   CNXT_TVENC_PAL_B_ITALY,           /* PAL-B Italy  */
   CNXT_TVENC_PAL_B_WEUR,            /* PAL-B West Europe  */ 
   CNXT_TVENC_PAL_B_AUS,             /* PAL-B Australia    */ 
   CNXT_TVENC_PAL_B_NZ,              /* PAL-B New Zealand  */ 
   CNXT_TVENC_PAL_I,                 /* PAL-I        */ 
   CNXT_TVENC_SECAM_L,               /* SECAM_L        */
   CNXT_TVENC_SECAM_D,               /* SECAM_D        */ 
   CNXT_TVENC_STANDARD_MAX           /* MUST be last item in list!!! */
} CNXT_TVENC_VIDEO_STANDARD, *PCNXT_TVENC_VIDEO_STANDARD;

/* enum for closed captioning type */
typedef enum
{
   CNXT_TVENC_CC_SEL = 0,                      /* closed captioning data */
   CNXT_TVENC_XDS_SEL                          /* extended data services */
} CNXT_TVENC_CC_TYPE;

/* Device handle */
typedef struct cnxt_tvenc_inst *CNXT_TVENC_HANDLE;

/* TV encoder type */
typedef enum
{
   CNXT_TVENC_BT861  = 0,
   CNXT_TVENC_BT861_INTERNAL,
   CNXT_TVENC_INVALID
}CNXT_TVENC_MODULE;

/* enum for aspect ratio */
typedef enum
{
   ASPECT_RATIO_INVALID = -1,
   FFORMAT_43_NA     = 0,   /* 4:3 full format position not applicable */
   LBOX_149_CTR      = 1,   /* 16:9 letterbox position top */
   LBOX_149_TOP      = 2,   /* 14:9 letterbox position top */
   LBOX_169_CTR      = 3,   /* 14:9 full format position center */
   LBOX_169_TOP      = 4,   /* 14:9 letterbox position center */
   LBOX_G_169_CTR    = 5,   /* >16:9 letterbox position center */
   FFORMAT_149_CTR   = 6,   /* 16:9 letterbox position center */
   FFORMAT_169_NA    = 7    /* 16:9 full format position not applicable */
}WSS_ASPECT_RATIO;

/* enum for subtitling mode */
typedef enum
{
   SBTITLE_INVALID    = -1,
   SBTITLE_NO_OPEN    = 0,  /* no open subtitles */
   SBTITLE_IN_ACTIVE = 1,   /* subtitles in active image area */
   SBTITLE_OUT_ACTIVE  = 2, /* subtitles out of active image area */
   SBTITLE_RESERVED   = 3   /* reserved */
}WSS_SUBTITLE_MODE;

typedef struct 
{
   WSS_ASPECT_RATIO   AspectRatio;
   bool               bFilmMode;
   bool               bColorCoding;
   bool               bHelper;
   bool               bSubtitlesTTX;
   WSS_SUBTITLE_MODE  SubtitlingMode;
   bool               bSurroundSound;
   bool               bCopyright;
   bool               bCopyRestrict;
}CNXT_TVENC_WSS_SETTINGS, *PCNXT_TVENC_WSS_SETTINGS;

/* enum for aspect ration used in CNXT_TVENC_CGMS_SETTINGS */
typedef enum
{
   CGMS_FFORMAT_43        = 0,   /* 4:3 full format */
   CGMS_FFORMAT_169       = 1,   /* 16:9 full format */
   CGMS_LBOX_43           = 2    /* 4:3 letterbox */
}CGMS_ASPECT_RATIO;

/* enum for CGMS-A signal bits */
typedef enum
{
   CGMS_COPY_NO_RSTRICT   = 0,   /* Copying is permitted without restriction */
   CGMS_NO_CONDITION      = 1,   /* Condition not be used */
   CGMS_ONE_GEN_COPY      = 2,   /* One generation of copies may be made */
   CGMS_NO_COPY           = 3    /* No copying is permitted */
}CGMS_A;


/* enum for APS trigger bits */
typedef enum
{
   CGMS_PSP_OFF    			 = 0,   /* PSP off */
   CGMS_PSP_ON_SBURST_OFF    = 1,   /* PSP on, split burst off */
   CGMS_PSP_ON_2Line_SBURST  = 2,   /* PSP on, 2-line split burst on */
   CGMS_PSP_ON_4Line_SBURST  = 3    /* PSP on, 4-line split burst on */

}CGMS_APS_TRIGGER;

/* settings for the copy generation management system */
typedef struct 
{
   CGMS_ASPECT_RATIO  AspectRatio;
   bool               bTransfCopyright;
   CGMS_A        	  CGMS_A;
   CGMS_APS_TRIGGER   APSTrigger;
   bool               bAlogSrc;

}CNXT_TVENC_CGMS_SETTINGS, *PCNXT_TVENC_CGMS_SETTINGS;

/* notification function */
typedef CNXT_TVENC_STATUS (*CNXT_TVENC_PFNNOTIFY)(
   CNXT_TVENC_HANDLE Handle, 
   void             *pUserData, 
   CNXT_TVENC_EVENT  Event,
   void             *pData,
   void             *Tag);

/******************/
/* API prototypes */
/******************/
CNXT_TVENC_STATUS cnxt_tvenc_init ( CNXT_TVENC_CONFIG *pCfg );

CNXT_TVENC_STATUS cnxt_tvenc_term ( void );

CNXT_TVENC_STATUS cnxt_tvenc_get_units ( u_int32 *puCount );

CNXT_TVENC_STATUS cnxt_tvenc_get_unit_caps ( u_int32            uUnitNumber, 
                                             CNXT_TVENC_CAPS *pCaps );

CNXT_TVENC_STATUS cnxt_tvenc_open ( CNXT_TVENC_HANDLE    *pHandle,
                                    CNXT_TVENC_CAPS      *pCaps,
                                    CNXT_TVENC_PFNNOTIFY  pNotifyFn,
                                    void                 *pUserData );

CNXT_TVENC_STATUS cnxt_tvenc_close ( CNXT_TVENC_HANDLE Handle );

CNXT_TVENC_STATUS cnxt_tvenc_set_picture_control( CNXT_TVENC_HANDLE Handle, CNXT_TVENC_CONTROL Control, int32 Value );

CNXT_TVENC_STATUS cnxt_tvenc_get_picture_control( CNXT_TVENC_HANDLE Handle, CNXT_TVENC_CONTROL Control, int32 *pValue );

CNXT_TVENC_STATUS cnxt_tvenc_set_video_standard( CNXT_TVENC_HANDLE Handle, CNXT_TVENC_VIDEO_STANDARD Standard );

CNXT_TVENC_STATUS cnxt_tvenc_get_video_standard( CNXT_TVENC_HANDLE Handle, CNXT_TVENC_VIDEO_STANDARD *pStandard );

CNXT_TVENC_STATUS cnxt_tvenc_set_display_position_offset( CNXT_TVENC_HANDLE Handle, int8 XOffset, int8 YOffset );

CNXT_TVENC_STATUS cnxt_tvenc_get_display_position_offset( CNXT_TVENC_HANDLE Handle, int8 *pXStart, int8 *pYStart );

CNXT_TVENC_STATUS cnxt_tvenc_set_output_connection( CNXT_TVENC_HANDLE Handle, u_int8 uConnection );

CNXT_TVENC_STATUS cnxt_tvenc_get_output_connection( CNXT_TVENC_HANDLE Handle, u_int8 *puConnection );

CNXT_TVENC_STATUS cnxt_tvenc_video_timing_reset( CNXT_TVENC_HANDLE Handle );

CNXT_TVENC_STATUS cnxt_tvenc_ttx_enable( CNXT_TVENC_HANDLE Handle );

CNXT_TVENC_STATUS cnxt_tvenc_ttx_set_lines( CNXT_TVENC_HANDLE Handle, u_int32 uField1ActiveLines, u_int32 uField2ActiveLines );

CNXT_TVENC_STATUS cnxt_tvenc_ttx_disable( CNXT_TVENC_HANDLE Handle );

CNXT_TVENC_STATUS cnxt_tvenc_cc_enable( CNXT_TVENC_HANDLE Handle );

CNXT_TVENC_STATUS cnxt_tvenc_cc_send_data( CNXT_TVENC_HANDLE Handle, CNXT_TVENC_CC_TYPE Type, \
                                            u_int8 uByteOne, u_int8 uByteTwo );

CNXT_TVENC_STATUS cnxt_tvenc_cc_disable( CNXT_TVENC_HANDLE Handle );

CNXT_TVENC_STATUS cnxt_tvenc_wss_enable( CNXT_TVENC_HANDLE Handle );

CNXT_TVENC_STATUS cnxt_tvenc_wss_set_config( CNXT_TVENC_HANDLE Handle, CNXT_TVENC_WSS_SETTINGS *pWSS_Settings );

CNXT_TVENC_STATUS cnxt_tvenc_wss_get_config( CNXT_TVENC_HANDLE Handle, CNXT_TVENC_WSS_SETTINGS *pWSS_Settings );

CNXT_TVENC_STATUS cnxt_tvenc_wss_disable( CNXT_TVENC_HANDLE Handle );

CNXT_TVENC_STATUS cnxt_tvenc_cgms_enable( CNXT_TVENC_HANDLE Handle );

CNXT_TVENC_STATUS cnxt_tvenc_cgms_set_config( CNXT_TVENC_HANDLE Handle, 
                            CNXT_TVENC_CGMS_SETTINGS *pCGMS_Settings );

CNXT_TVENC_STATUS cnxt_tvenc_cgms_get_config( CNXT_TVENC_HANDLE Handle, 
                             CNXT_TVENC_CGMS_SETTINGS *pCGMS_Settings );

CNXT_TVENC_STATUS cnxt_tvenc_cgms_disable( CNXT_TVENC_HANDLE Handle );

#endif   /* _TVENC_H_ */


/****************************************************************************
 * Modifications:
 * $Log: 
 *  2    mpeg      1.1         10/24/03 11:48:29 AM   Xin Golden      CR(s): 
 *        7463 add CGMS support to tvenc driver.
 *  1    mpeg      1.0         7/30/03 4:08:30 PM     Lucy C Allevato 
 * $
 * 
 *    Rev 1.0   30 Jul 2003 15:08:30   goldenx
 * SCR(s) 5519 :
 * initial revision
 ****************************************************************************/

