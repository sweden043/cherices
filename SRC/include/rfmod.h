/****************************************************************************/
/*                   CONEXANT PROPRIETARY AND CONFIDENTIAL                  */
/*                       SOFTWARE FILE/MODULE HEADER                        */
/*        Conexant Systems Inc. (c) 1998, 1999, 2000, 2001, 2002, 2003      */
/*                              Austin, TX                                  */
/*                         All Rights Reserved                              */
/****************************************************************************/
/*
 * Filename:        rfmod.h
 *
 *
 * Description:     Public header file for RF Modulator driver
 *
 *
 * Author:          Angela Swartz
 *
 ****************************************************************************/
/* $Header: rfmod.h, 3, 2/10/03 2:09:00 PM, Angela Swartz$
 ****************************************************************************/

#ifndef _RFMOD_H_
#define _RFMOD_H_

/*****************/
/* Include Files */
/*****************/
#include "basetype.h"

/********************************/
/* Symbol and Macro definitions */
/********************************/

/*****************/
/* Data Types    */
/*****************/

/* return values of APIs */
typedef enum
{
   CNXT_RFMOD_OK = 0,
   CNXT_RFMOD_ALREADY_INIT,
   CNXT_RFMOD_NOT_INIT,
   CNXT_RFMOD_BAD_UNIT,
   CNXT_RFMOD_CLOSED_HANDLE,
   CNXT_RFMOD_BAD_HANDLE,
   CNXT_RFMOD_BAD_PARAMETER,
   CNXT_RFMOD_RESOURCE_ERROR,
   CNXT_RFMOD_NOT_AVAILABLE,
   CNXT_RFMOD_INTERNAL_ERROR
} CNXT_RFMOD_STATUS;

/* RF modulator output mode */
typedef enum
{
   CNXT_RFMOD_MODE_OFF,
   CNXT_RFMOD_MODE_PASSTHRU,
   CNXT_RFMOD_MODE_CH3,
   CNXT_RFMOD_MODE_CH4
} CNXT_RFMOD_MODE;

/* driver configuration structure */
typedef void* CNXT_RFMOD_CONFIG;     /* not used */

/* device capability structure */
typedef struct
{
   u_int32 uLength;
   bool    bExclusive;
} CNXT_RFMOD_CAPS;

/* device handle */
typedef struct cnxt_rfmod_inst *CNXT_RFMOD_HANDLE;

/* notification function */
typedef void* CNXT_RFMOD_PFNNOTIFY; /* Notification function definition, not used */

/******************/
/* API prototypes */
/******************/
CNXT_RFMOD_STATUS cnxt_rfmod_init ( CNXT_RFMOD_CONFIG *pCfg );
CNXT_RFMOD_STATUS cnxt_rfmod_term ( void );
CNXT_RFMOD_STATUS cnxt_rfmod_get_units ( u_int32 *puCount );
CNXT_RFMOD_STATUS cnxt_rfmod_get_unit_caps ( u_int32 uUnitNumber, CNXT_RFMOD_CAPS *pCaps );
CNXT_RFMOD_STATUS cnxt_rfmod_open ( CNXT_RFMOD_HANDLE    *pHandle,
                                    CNXT_RFMOD_CAPS      *pCaps,
                                    CNXT_RFMOD_PFNNOTIFY pNotifyFn,
                                    void                 *pUserData );
CNXT_RFMOD_STATUS cnxt_rfmod_close ( CNXT_RFMOD_HANDLE Handle );
CNXT_RFMOD_STATUS cnxt_rfmod_set_mode( CNXT_RFMOD_HANDLE Handle, CNXT_RFMOD_MODE Mode );
CNXT_RFMOD_STATUS cnxt_rfmod_get_mode( CNXT_RFMOD_HANDLE Handle, CNXT_RFMOD_MODE *pMode );
CNXT_RFMOD_STATUS cnxt_rfmod_set_audio_sample_rate( CNXT_RFMOD_HANDLE Handle, u_int32 uRate );

#endif   /* _RFMOD_H_ */

