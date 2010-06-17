/****************************************************************************/
/*                   CONEXANT PROPRIETARY AND CONFIDENTIAL                  */
/*                       SOFTWARE FILE/MODULE HEADER                        */
/*        Conexant Systems Inc. (c) 1998, 1999, 2000, 2001, 2002, 2003      */
/*                              Austin, TX                                  */
/*                         All Rights Reserved                              */
/****************************************************************************/
/*
 * Filename:        gen_scart.h
 *
 *
 * Description:     Public header file for Generic Scart driver
 *
 *
 * Author:          Angela Swartz
 *
 ****************************************************************************/
/* $Header: gen_scart.h, 1, 6/25/03 2:18:40 PM, Angela Swartz$
 ****************************************************************************/

#ifndef _GEN_SCART_H_
#define _GEN_SCART_H_

/*****************/
/* Include Files */
/*****************/
#include "basetype.h"

/*****************/
/* Data Types    */
/*****************/

/* return values of APIs */
typedef enum
{
   CNXT_SCART_OK = 0,
   CNXT_SCART_ALREADY_INIT,
   CNXT_SCART_NOT_INIT,
   CNXT_SCART_BAD_UNIT,
   CNXT_SCART_CLOSED_HANDLE,
   CNXT_SCART_BAD_HANDLE,
   CNXT_SCART_BAD_SCART,
   CNXT_SCART_BAD_PARAMETER,
   CNXT_SCART_RESOURCE_ERROR,
   CNXT_SCART_NOT_AVAILABLE,
   CNXT_SCART_INTERNAL_ERROR,
   CNXT_SCART_UNSUPPORTED
} CNXT_SCART_STATUS;

/* SCART signal output mode */
typedef enum
{
   CNXT_SCART_MODE_OFF = 0,
   CNXT_SCART_MODE_ON,
   CNXT_SCART_MODE_STANDBY
} CNXT_SCART_MODE;

/* SCART connector type */
typedef enum
{
   CNXT_SCART_TV = 0,
   CNXT_SCART_VCR,
   CNXT_SCART_AUX
} CNXT_SCART_CONNECTOR;

/* SCART control pin type */
typedef enum
{
   CNXT_SCART_PIN_8 = 0,
   CNXT_SCART_PIN_16
} CNXT_SCART_PIN;

/* SCART control pin voltage level */
typedef enum
{
   CNXT_SCART_PIN_VOLTAGE_LOW = 0,
   CNXT_SCART_PIN_VOLTAGE_MID,
   CNXT_SCART_PIN_VOLTAGE_HIGH
} CNXT_SCART_PIN_VOLTAGE_LEVEL;

/* SCART notification event */
typedef enum
{
   CNXT_SCART_EVENT_TERM,  /* driver is terminated */
   CNXT_SCART_EVENT_PIN    /* pin voltage has changed */
} CNXT_SCART_EVENT;

typedef struct 
{
   CNXT_SCART_MODE Mode;
} CNXT_SCART_CONFIG;     

/* device capability structure */
typedef struct
{
   u_int32 uLength;
   bool    bExclusive;
   u_int8  uScartsMask; /* indicating the availability of particular scart connectors on the controller */
   u_int16 uPinsMask[2];   /* indicating the capability of control pins on all SCART connectors on the controller */
} CNXT_SCART_CAPS;

/* SCART state data */
typedef struct
{
   unsigned char uPin8_enabled;
   unsigned char uPin16_enabled;
} CNXT_SCART_PIN_ENABLE_STATE;

/* SCART Pins Voltage Level data */
typedef struct
{
   CNXT_SCART_PIN_VOLTAGE_LEVEL Pin8_enabled_voltage;
   CNXT_SCART_PIN_VOLTAGE_LEVEL Pin16_enabled_voltage;
} CNXT_SCART_VOLTAGE;
   
/* device handle */
typedef struct cnxt_scart_inst *CNXT_SCART_HANDLE;

/* notification function */
typedef CNXT_SCART_STATUS ( *CNXT_SCART_PFNNOTIFY ) (
                                              CNXT_SCART_HANDLE Handle,
                                              void *pUserData,
                                              CNXT_SCART_EVENT Event,
                                              void *pData,
                                              void *Tag );

/********************************/
/* Symbol and Macro definitions */
/********************************/

#define CNXT_SCART_PIN_DISABLED 0
#define CNXT_SCART_PIN_ENABLED 1

#define CNXT_SCART_TV_MASK ( 1 << CNXT_SCART_TV )
#define CNXT_SCART_VCR_MASK ( 1 << CNXT_SCART_VCR )
#define CNXT_SCART_AUX_MASK ( 1 << CNXT_SCART_AUX )

#define PIN8_CTL_MASK 0x0001
 #define PIN8_CTL_SHIFT 0
 #define PIN8_CTL (1 << PIN8_CTL_SHIFT)
 #define PIN8_CTL_NONE (0 << PIN8_CTL_SHIFT)
#define PIN8_NOTIFY_MASK 0x0002
 #define PIN8_NOTIFY_SHIFT 1
 #define PIN8_NOTIFY (1 << PIN8_NOTIFY_SHIFT)
 #define PIN8_NOTIFY_NONE (0 << PIN8_NOTIFY_SHIFT)
#define PIN8_SUPP_LOW_MASK 0x0004
 #define PIN8_SUPP_LOW_SHIFT 2
 #define PIN8_SUPP_LOW (1 << PIN8_SUPP_LOW_SHIFT)
 #define PIN8_SUPP_LOW_NONE (0 << PIN8_SUPP_LOW_SHIFT)
#define PIN8_SUPP_MID_MASK 0x0008
 #define PIN8_SUPP_MID_SHIFT 3
 #define PIN8_SUPP_MID (1 << PIN8_SUPP_MID_SHIFT)
 #define PIN8_SUPP_MID_NONE (0 << PIN8_SUPP_MID_SHIFT)
#define PIN8_SUPP_HIGH_MASK 0x0010
 #define PIN8_SUPP_HIGH_SHIFT 4
 #define PIN8_SUPP_HIGH (1 << PIN8_SUPP_HIGH_SHIFT)
 #define PIN8_SUPP_HIGH_NONE (0 << PIN8_SUPP_HIGH_SHIFT)
 
#define PIN16_CTL_MASK 0x0100
 #define PIN16_CTL_SHIFT 8
 #define PIN16_CTL (1 << PIN16_CTL_SHIFT)
 #define PIN16_CTL_NONE (0 << PIN16_CTL_SHIFT)
#define PIN16_NOTIFY_MASK 0x0200
 #define PIN16_NOTIFY_SHIFT 9
 #define PIN16_NOTIFY (1 << PIN16_NOTIFY_SHIFT)
 #define PIN16_NOTIFY_NONE (0 << PIN16_NOTIFY_SHIFT)
#define PIN16_SUPP_LOW_MASK 0x0400
 #define PIN16_SUPP_LOW_SHIFT 10
 #define PIN16_SUPP_LOW (1 << PIN16_SUPP_LOW_SHIFT)
 #define PIN16_SUPP_LOW_NONE (0 << PIN16_SUPP_LOW_SHIFT)
#define PIN16_SUPP_MID_MASK 0x0800
 #define PIN16_SUPP_MID_SHIFT 11
 #define PIN16_SUPP_MID (1 << PIN16_SUPP_MID_SHIFT)
 #define PIN16_SUPP_MID_NONE (0 << PIN16_SUPP_MID_SHIFT)
#define PIN16_SUPP_HIGH_MASK 0x1000
 #define PIN16_SUPP_HIGH_SHIFT 12
 #define PIN16_SUPP_HIGH (1 << PIN16_SUPP_HIGH_SHIFT)
 #define PIN16_SUPP_HIGH_NONE (0 << PIN16_SUPP_HIGH_SHIFT)


/******************/
/* API prototypes */
/******************/
CNXT_SCART_STATUS cnxt_scart_init ( CNXT_SCART_CONFIG *pCfg );
CNXT_SCART_STATUS cnxt_scart_term ( void );
CNXT_SCART_STATUS cnxt_scart_get_units ( u_int32 *puCount );
CNXT_SCART_STATUS cnxt_scart_get_unit_caps ( u_int32 uUnitNumber, CNXT_SCART_CAPS *pCaps );
CNXT_SCART_STATUS cnxt_scart_open ( CNXT_SCART_HANDLE    *pHandle,
                                    CNXT_SCART_CAPS      *pCaps,
                                    CNXT_SCART_PFNNOTIFY pNotifyFn,
                                    void                 *pUserData );

CNXT_SCART_STATUS cnxt_scart_close ( CNXT_SCART_HANDLE Handle );
CNXT_SCART_STATUS cnxt_scart_set_mode( CNXT_SCART_HANDLE Handle, CNXT_SCART_MODE Mode );
CNXT_SCART_STATUS cnxt_scart_get_mode( CNXT_SCART_HANDLE Handle, CNXT_SCART_MODE *pMode );

CNXT_SCART_STATUS cnxt_scart_set_pin_state( CNXT_SCART_HANDLE Handle, 
                                            CNXT_SCART_CONNECTOR Scart,
                                            CNXT_SCART_PIN_ENABLE_STATE State );

CNXT_SCART_STATUS cnxt_scart_get_pin_state( CNXT_SCART_HANDLE Handle, 
                                            CNXT_SCART_CONNECTOR Scart,
                                            CNXT_SCART_PIN_ENABLE_STATE *pState );

CNXT_SCART_STATUS cnxt_scart_set_pin_level( CNXT_SCART_HANDLE Handle, 
                                            CNXT_SCART_CONNECTOR Scart,
                                            CNXT_SCART_VOLTAGE Voltage );

CNXT_SCART_STATUS cnxt_scart_get_pin_level( CNXT_SCART_HANDLE Handle, 
                                            CNXT_SCART_CONNECTOR Scart,
                                            CNXT_SCART_VOLTAGE *pVoltage );


#endif   /* _GEN_SCART_H_ */
/****************************************************************************
 * Modifications:
 * $Log: 
 *  1    mpeg      1.0         6/25/03 2:18:40 PM     Angela Swartz   
 * $
 * 
 *    Rev 1.0   25 Jun 2003 13:18:40   swartzwg
 * SCR(s) 5518 :
 * Initial revision of public header for the generic scart driver
 * 
 ****************************************************************************/

