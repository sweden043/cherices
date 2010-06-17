/****************************************************************************/
/*                   CONEXANT PROPRIETARY AND CONFIDENTIAL                  */
/*                       SOFTWARE FILE/MODULE HEADER                        */
/*        Conexant Systems Inc. (c) 1998, 1999, 2000, 2001, 2002, 2003      */
/*                              Austin, TX                                  */
/*                         All Rights Reserved                              */
/****************************************************************************/
/*
 * Filename:        encoder.h
 *
 *
 * Description:     Private header file for encoder driver
 *
 *
 * Author:          Dave Wilson (but basically copied from a header by 
 *                  Angela Swartz)
 *
 ****************************************************************************/
/* $Header: encoder_priv.h, 4, 9/23/03 4:40:12 PM, Lucy C Allevato$
 ****************************************************************************/

#ifndef _ENCODER_PRIV_H_
#define _ENCODER_PRIV_H_

/*****************/
/* Include Files */
/*****************/
#include "basetype.h"
#include "kal.h"
#include "encoder.h"

/********************************/
/* Symbol and Macro definitions */
/********************************/
#ifndef CNXT_ENCODER_NUM_UNITS
#define CNXT_ENCODER_NUM_UNITS  1  /* change this as needed */
#endif

#ifndef CNXT_ENCODER_MAX_HANDLES
#ifndef DRIVER_INCL_HANDLE
#define CNXT_ENCODER_MAX_HANDLES   1 /* !!!! change this as needed */
#else
#define CNXT_ENCODER_MAX_HANDLES   5 /* !!!! change this as needed */
#endif
#endif

#define TRACE_ENCODER (TRACE_OSD|TRACE_LEVEL_2)
                      

/*****************/
/* Data Types    */
/*****************/
typedef struct _cnxt_encoder_inst
{
   CNXT_HANDLE_PREFACE        Preface; 
   CNXT_ENCODER_PFNNOTIFY     pNotifyFn; /* Function to handle Inst notifications */
   void                      *pUserData; /* Appl data needed by Inst */
   u_int32                    uUnitNumber;
} CNXT_ENCODER_INST;

typedef struct
{
   CNXT_ENCODER_CAPS       *pCaps;
   bool                    bExclusive;
   CNXT_ENCODER_INST       *pFirstInst;  /* Pointer to list of open Inst */
} CNXT_ENCODER_UNIT_INST;

typedef struct
{
   CNXT_ENCODER_INST       *pInstList;     /* Memory pool to get inst */
   bool                    bInit;
   sem_id_t                DriverSem;
   CNXT_ENCODER_UNIT_INST  *pUnitInst;

   /* Add all other driver specific 'global' data should be declared here */

} CNXT_ENCODER_DRIVER_INST;

#endif   /* _ENCODER_PRIV_H_ */

