/****************************************************************************/
/*                   CONEXANT PROPRIETARY AND CONFIDENTIAL                  */
/*                       SOFTWARE FILE/MODULE HEADER                        */
/*    Conexant Systems Inc. (c) 1998, 1999, 2000, 2001, 2002, 2003, 2004    */
/*                           Shanghai, China                                */
/*                         All Rights Reserved                              */
/****************************************************************************/
/*
 * Filename:        ext_rfmod_sharp.h
 *
 *
 * Description:     Public header file for SHARP RF Modulator driver
 *
 *
 * Author:          Derek Xi
 *
 ****************************************************************************/
/* $Header:   
 ****************************************************************************/

#ifndef _EXT_RFMOD_SHARP_H_
#define _EXT_RFMOD_SHARP_H_

/*****************/
/* Include Files */
/*****************/
#include "basetype.h"

/********************************/
/* Symbol and Macro definitions */
/********************************/
#define EXT_RFMOD_MAX_CHANNEL_NUM 69
#define EXT_RFMOD_MIN_CHANNEL_NUM 21

/*****************/
/* Data Types    */
/*****************/

/* return values of APIs */
typedef enum
{
   CNXT_EXT_RFMOD_OK = 0,
   CNXT_EXT_RFMOD_ALREADY_INIT,
   CNXT_EXT_RFMOD_NOT_INIT,
   CNXT_EXT_RFMOD_BAD_UNIT,
   CNXT_EXT_RFMOD_CLOSED_HANDLE,
   CNXT_EXT_RFMOD_BAD_HANDLE,
   CNXT_EXT_RFMOD_BAD_PARAMETER,
   CNXT_EXT_RFMOD_RESOURCE_ERROR,
   CNXT_EXT_RFMOD_NOT_AVAILABLE,
   CNXT_EXT_RFMOD_INTERNAL_ERROR
} CNXT_EXT_RFMOD_STATUS;

/* RF modulator output mode */
typedef enum
{
   CNXT_EXT_RFMOD_MODE_OFF,
   CNXT_EXT_RFMOD_MODE_PASSTHRU,
   CNXT_EXT_RFMOD_MODE_ON
} CNXT_EXT_RFMOD_MODE;

/* Sound Inter Carrier Frequency */
typedef enum
{
   CNXT_EXT_RFMOD_SOUND_NTSC_M = 0,
   CNXT_EXT_RFMOD_SOUND_PAL_G,
   CNXT_EXT_RFMOD_SOUND_PAL_I,
   CNXT_EXT_RFMOD_SOUND_PAL_K
} CNXT_EXT_RFMOD_SOUND_MODE;

/* driver configuration structure */
//typedef void* CNXT_EXT_RFMOD_CONFIG;     /* not used */
typedef struct 
{
   CNXT_EXT_RFMOD_MODE Mode;
} CNXT_EXT_RFMOD_CONFIG;   

/* device capability structure */
typedef struct
{
   u_int32 uLength;
   bool    bExclusive;
} CNXT_EXT_RFMOD_CAPS;

/* device handle */
typedef struct CNXT_EXT_RFMOD_inst *CNXT_EXT_RFMOD_HANDLE;

/* notification function */
typedef void* CNXT_EXT_RFMOD_PFNNOTIFY; /* Notification function definition, not used */

/******************/
/* API prototypes */
/******************/
CNXT_EXT_RFMOD_STATUS cnxt_ext_rfmod_init ( CNXT_EXT_RFMOD_CONFIG *pCfg );
CNXT_EXT_RFMOD_STATUS cnxt_ext_rfmod_term ( void );
CNXT_EXT_RFMOD_STATUS cnxt_ext_rfmod_get_units ( u_int32 *puCount );
CNXT_EXT_RFMOD_STATUS cnxt_ext_rfmod_get_unit_caps ( u_int32 uUnitNumber, CNXT_EXT_RFMOD_CAPS *pCaps );
CNXT_EXT_RFMOD_STATUS cnxt_ext_rfmod_open ( CNXT_EXT_RFMOD_HANDLE    *pHandle,
                                    CNXT_EXT_RFMOD_CAPS      *pCaps,
                                    CNXT_EXT_RFMOD_PFNNOTIFY pNotifyFn,
                                    void                 *pUserData );
CNXT_EXT_RFMOD_STATUS cnxt_ext_rfmod_close ( CNXT_EXT_RFMOD_HANDLE Handle );
CNXT_EXT_RFMOD_STATUS cnxt_ext_rfmod_set_mode( CNXT_EXT_RFMOD_HANDLE Handle, CNXT_EXT_RFMOD_MODE Mode );
CNXT_EXT_RFMOD_STATUS cnxt_ext_rfmod_get_mode( CNXT_EXT_RFMOD_HANDLE Handle, CNXT_EXT_RFMOD_MODE *pMode );
CNXT_EXT_RFMOD_STATUS cnxt_ext_rfmod_set_audio_standard( CNXT_EXT_RFMOD_HANDLE Handle, CNXT_EXT_RFMOD_SOUND_MODE Mode );
CNXT_EXT_RFMOD_STATUS cnxt_ext_rfmod_set_channel_number( CNXT_EXT_RFMOD_HANDLE Handle, u_int16 uNum );

#endif   /* _EXT_RFMOD_SHARP_H_ */

