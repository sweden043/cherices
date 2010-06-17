/****************************************************************************/
/*                   CONEXANT PROPRIETARY AND CONFIDENTIAL                  */
/*                       SOFTWARE FILE/MODULE HEADER                        */
/*        Conexant Systems Inc. (c) 1998, 1999, 2000, 2001, 2002, 2003      */
/*                              Austin, TX                                  */
/*                         All Rights Reserved                              */
/****************************************************************************/
/*
 * Filename:        smartcard.h
 *
 *
 * Description:     Public header file for smart card driver
 *
 *
 * Author:          Larry Wang
 *
 ****************************************************************************/
/* $Header: smartcard.h, 4, 2/17/03 3:59:56 PM, Larry Wang$
 ****************************************************************************/

#ifndef _SMARTCARD_H_
#define _SMARTCARD_H_

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
/* states of a smart card */
typedef enum
{
   CNXT_SMC_EMPTY = 0,
   CNXT_SMC_NOT_READY,
   CNXT_SMC_READY
} CNXT_SMC_STATE;              

/* card convention */
typedef enum 
{
   CNXT_SMC_CONV_DIRECT,
   CNXT_SMC_CONV_INVERSE
} CNXT_SMC_CONVENTION;

/* card protocol */
typedef enum 
{
   CNXT_SMC_PROTOCOL_T0,
   CNXT_SMC_PROTOCOL_T1,
   CNXT_SMC_PROTOCOL_T14 = 14
} CNXT_SMC_PROTOCOL;

/* card config items */
typedef enum
{
   CNXT_SMC_CONFIG_CONVENTION,
   CNXT_SMC_CONFIG_PROTOCOL,
   CNXT_SMC_CONFIG_FI,
   CNXT_SMC_CONFIG_DI,
   CNXT_SMC_CONFIG_PI1,
   CNXT_SMC_CONFIG_PI2,
   CNXT_SMC_CONFIG_II,
   CNXT_SMC_CONFIG_N,
   CNXT_SMC_CONFIG_HISTORICAL,
   CNXT_SMC_CONFIG_HISTORICAL_LEN,
   CNXT_SMC_CONFIG_TIMEOUT,
   CNXT_SMC_CONFIG_RETRY
} CNXT_SMC_CONFIG_ITEM;

/* return values of APIs */
typedef enum
{
   CNXT_SMC_OK = 0,
   CNXT_SMC_ALREADY_INIT,
   CNXT_SMC_NOT_INIT,
   CNXT_SMC_BAD_UNIT,
   CNXT_SMC_CLOSED_HANDLE,
   CNXT_SMC_BAD_HANDLE,
   CNXT_SMC_BAD_PARAMETER,
   CNXT_SMC_RESOURCE_ERROR,
   CNXT_SMC_INTERNAL_ERROR,
   CNXT_SMC_NOT_AVAILABLE,
   CNXT_SMC_TIMEOUT
} CNXT_SMC_STATUS;

/* events */
typedef enum
{
   CNXT_SMC_EVENT_TERM,
   CNXT_SMC_EVENT_RESET,
   CNXT_SMC_EVENT_CARD_INSERTED,
   CNXT_SMC_EVENT_CARD_RESET_COMPLETE,
   CNXT_SMC_EVENT_CARD_RESET_TIMEOUT,
   CNXT_SMC_EVENT_CARD_RW_COMPLETE,
   CNXT_SMC_EVENT_CARD_RW_TIMEOUT,
   CNXT_SMC_EVENT_CARD_POWER_DOWN_COMPLETE,
   CNXT_SMC_EVENT_CARD_POWER_DOWN_TIMEOUT,
   CNXT_SMC_EVENT_CARD_REMOVED,
   CNXT_SMC_EVENT_CARD_ATR_RECIEVED,  /*For CTI CA requirement*/
   CNXT_SMC_EVENT_ATR_TIMEOUT
} CNXT_SMC_EVENT;

/* driver configuration structure */
typedef void* CNXT_SMC_CONFIG;     /* Note: This is the definition if no */
                                   /*       config structure is needed.  */
                                   /*       Change it as needed.         */

/* device capability structure */
typedef struct
{
   u_int32 uLength;
   bool    bExclusive;
   u_int32 uUnitNumber;
} CNXT_SMC_CAPS;

/* device handle */
typedef struct cnxt_smc_inst *CNXT_SMC_HANDLE;

/* notification function */
typedef CNXT_SMC_STATUS (*CNXT_SMC_PFNNOTIFY) (CNXT_SMC_HANDLE Handle,
                                               void            *pUserData,
                                               CNXT_SMC_EVENT  Event,
                                               void            *pData,
                                               void            *Tag);
                /* 
                 * Note: This is notification callback for asynchronous options.
                 *       If no asynchronous operation in the driver, change it
                 *       as needed.
                 */

/******************/
/* API prototypes */
/******************/
CNXT_SMC_STATUS cnxt_smc_init ( CNXT_SMC_CONFIG *pCfg );
CNXT_SMC_STATUS cnxt_smc_term ( void );
CNXT_SMC_STATUS cnxt_smc_reset ( CNXT_SMC_CONFIG *pCfg ); /* Note: optional */
CNXT_SMC_STATUS cnxt_smc_get_units ( u_int32 *puCount );
CNXT_SMC_STATUS cnxt_smc_get_unit_caps ( u_int32       uUnitNumber, 
                                         CNXT_SMC_CAPS *pCaps );

CNXT_SMC_STATUS cnxt_smc_open ( CNXT_SMC_HANDLE    *pHandle,
                                CNXT_SMC_CAPS      *pCaps,
                                CNXT_SMC_PFNNOTIFY pNotifyFn,
                                void               *pUserData );

CNXT_SMC_STATUS cnxt_smc_close ( CNXT_SMC_HANDLE Handle );

CNXT_SMC_STATUS cnxt_smc_reset_card ( CNXT_SMC_HANDLE Handle,
                                      bool            bAsync,
                                      void            *Tag );

CNXT_SMC_STATUS cnxt_smc_powerdown_card ( CNXT_SMC_HANDLE Handle,
                                          bool            bAsync,
                                          void            *Tag );

CNXT_SMC_STATUS cnxt_smc_get_state ( CNXT_SMC_HANDLE Handle,
                                     CNXT_SMC_STATE  *pState );

CNXT_SMC_STATUS cnxt_smc_get_atr ( CNXT_SMC_HANDLE Handle,
                                   void            *pAtr,
                                   u_int32         *puBufLength );

CNXT_SMC_STATUS cnxt_smc_set_config ( CNXT_SMC_HANDLE      Handle,
                                      CNXT_SMC_CONFIG_ITEM Item,
                                      u_int32              uValue );

CNXT_SMC_STATUS cnxt_smc_get_config ( CNXT_SMC_HANDLE      Handle,
                                      CNXT_SMC_CONFIG_ITEM Item,
                                      u_int32              *puValue );

CNXT_SMC_STATUS cnxt_smc_read_write ( CNXT_SMC_HANDLE      Handle,
                                      bool                 bAsync,
                                      void                 *pOutBuf,
                                      u_int32              uOutLength,
                                      void                 *pInBuf,
                                      u_int32              *pInLength,
                                      void                 *Tag );
#endif   /* _SMARTCARD_H_ */

/****************************************************************************
 * Modifications:
 * $Log: 
 *  4    mpeg      1.3         2/17/03 3:59:56 PM     Larry Wang      SCR(s) 
 *        5525 :
 *        Add definition of CNXT_SMC_PROTOCOL_T14.
 *        
 *  3    mpeg      1.2         1/31/03 3:39:48 PM     Larry Wang      SCR(s) 
 *        5324 :
 *        
 *        
 *  2    mpeg      1.1         1/28/03 9:28:38 AM     Larry Wang      SCR(s) 
 *        5324 :
 *        
 *        
 *  1    mpeg      1.0         1/27/03 12:42:44 PM    Larry Wang      
 * $
 * 
 *    Rev 1.3   17 Feb 2003 15:59:56   wangl2
 * SCR(s) 5525 :
 * Add definition of CNXT_SMC_PROTOCOL_T14.
 * 
 *    Rev 1.2   31 Jan 2003 15:39:48   wangl2
 * SCR(s) 5324 :
 * 
 * 
 ****************************************************************************/

