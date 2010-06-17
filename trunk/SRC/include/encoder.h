/****************************************************************************/
/*                   CONEXANT PROPRIETARY AND CONFIDENTIAL                  */
/*                       SOFTWARE FILE/MODULE HEADER                        */
/*           Conexant Systems Inc. (c) 1998, 1999, 2000, 2001, 2002         */
/*                               Austin, TX                                 */
/*                            All Rights Reserved                           */
/****************************************************************************/
/*
 * Filename:       encoder.h
 *
 *
 * Description:    Video Decoder control driver public header
 *
 *
 * Author:         Dave Wilson, Billy Jackman, etc.
 *
 ****************************************************************************/
/* $Header: encoder.h, 1, 1/27/03 3:21:00 PM, Dave Wilson$
 ****************************************************************************/

#ifndef _ENCODER_H_
#define _ENCODER_H_

/*******************************/
/* Type definitions and labels */
/*******************************/

/* Return values of APIs */
typedef enum
{
   CNXT_ENCODER_OK = 0,
   CNXT_ENCODER_ALREADY_INIT,
   CNXT_ENCODER_NOT_INIT,
   CNXT_ENCODER_BAD_UNIT,
   CNXT_ENCODER_CLOSED_HANDLE,
   CNXT_ENCODER_BAD_HANDLE,
   CNXT_ENCODER_BAD_PARAMETER,
   CNXT_ENCODER_RESOURCE_ERROR,
   CNXT_ENCODER_INTERNAL_ERROR,
   CNXT_ENCODER_UNSUPPORTED
} CNXT_ENCODER_STATUS;

/* Driver configuration structure */
typedef void* CNXT_ENCODER_CONFIG;     /* not used */

/* Device capability structure */
typedef struct
{
   u_int32 uLength;
   bool    bExclusive;
   u_int8  uFlags; /* Indicate which picture controls are supported */
} CNXT_ENCODER_CAPS;

/* Picture controls structure */
typedef struct
{
  u_int8 ucBrightness;
  u_int8 ucContrast;
  u_int8 ucSaturation;
  u_int8 ucHue;
  u_int8 uFlags;
} CNXT_ENCODER_CONTROLS;

/* Bit flags used to define controllability of parameters */
#define ENCODER_CONTROL_BRIGHTNESS 0x01
#define ENCODER_CONTROL_CONTRAST   0x02
#define ENCODER_CONTROL_SATURATION 0x04
#define ENCODER_CONTROL_HUE        0x08

/* Device handle */
typedef struct cnxt_encoder_inst *CNXT_ENCODER_HANDLE;

/* Notification function (not used but include for API consistency) */
typedef void* CNXT_ENCODER_PFNNOTIFY; 

/******************/
/* API prototypes */
/******************/
CNXT_ENCODER_STATUS cnxt_encoder_init ( CNXT_ENCODER_CONFIG *pCfg );

CNXT_ENCODER_STATUS cnxt_encoder_term ( void );

CNXT_ENCODER_STATUS cnxt_encoder_get_units ( u_int32 *puCount );

CNXT_ENCODER_STATUS cnxt_encoder_get_unit_caps ( u_int32            uUnitNumber, 
                                                 CNXT_ENCODER_CAPS *pCaps );

CNXT_ENCODER_STATUS cnxt_encoder_open ( CNXT_ENCODER_HANDLE    *pHandle,
                                        CNXT_ENCODER_CAPS      *pCaps,
                                        CNXT_ENCODER_PFNNOTIFY pNotifyFn,
                                        void                   *pUserData );

CNXT_ENCODER_STATUS cnxt_encoder_close ( CNXT_ENCODER_HANDLE Handle );

CNXT_ENCODER_STATUS cnxt_encoder_set_picture_controls( CNXT_ENCODER_HANDLE Handle, CNXT_ENCODER_CONTROLS *pControls );

CNXT_ENCODER_STATUS cnxt_encoder_get_picture_controls( CNXT_ENCODER_HANDLE Handle, CNXT_ENCODER_CONTROLS *pControls );

#endif /* _ENCODER_H_ */

/****************************************************************************
 * Modifications:
 * $Log: 
 *  1    mpeg      1.0         1/27/03 3:21:00 PM     Dave Wilson     
 * $
 * 
 *    Rev 1.0   27 Jan 2003 15:21:00   dawilson
 * SCR(s) 5320 :
 * Header for new video encoder driver.
 *
 ****************************************************************************/

