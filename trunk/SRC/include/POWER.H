/****************************************************************************/
/*                   CONEXANT PROPRIETARY AND CONFIDENTIAL                  */
/*                        SOFTWARE FILE/MODULE HEADER                       */
/*                   Copyright Conexant Systems Inc. 2003                   */
/*                                Austin, TX                                */
/*                            All Rights Reserved                           */
/****************************************************************************/
/*
 * Filename:        power.h
 *
 *
 * Description:     Public header file for power driver.
 *
 *
 * Author:          Craig Dry
 *
 ****************************************************************************/
/* $Header: POWER.H, 2, 10/11/03 5:07:37 PM, Craig Dry$
 ****************************************************************************/

#ifndef _POWER_H_
#define _POWER_H_

/********************************/
/* Symbol and Macro definitions */
/********************************/

/*****************/
/* Data Types    */
/*****************/

/* return values of APIs */
typedef enum
{
   CNXT_POWER_OK = 0,
   CNXT_POWER_ALREADY_INIT,
   CNXT_POWER_NOT_INIT,
   CNXT_POWER_BAD_UNIT,
   CNXT_POWER_CLOSED_HANDLE,
   CNXT_POWER_BAD_HANDLE,
   CNXT_POWER_BAD_PARAMETER,
   CNXT_POWER_RESOURCE_ERROR,
   CNXT_POWER_INTERNAL_ERROR,
   CNXT_POWER_NOT_AVAILABLE,
   CNXT_POWER_INVALID_PLL,
   CNXT_POWER_ZERO_DELAY,
   CNXT_POWER_NO_WAKEUP
} CNXT_POWER_STATUS;

/* attachment points */
typedef enum
{
   CNXT_POWER_PRIMARY_INPUT = 0
   /*
    * Driver-specific attachment point designations go here. The attachment
    * point CNXT_POWER_PRIMARY_INPUT is an example. Specific drivers may
    * have one, many, or no attachment points. An attachment point is an
    * interface point to a neighboring device that establishes data flow
    * (or a control interface such as a callback) from one to the other.
    */
} CNXT_POWER_ATTACH_POINT;

/* events */
typedef enum
{
   CNXT_POWER_EVENT_TERM
   /* Note: list all specific events here */
} CNXT_POWER_EVENT;

/* driver configuration structures */
typedef struct
{
   /*
    * Place driver-specific entries here to specify things like register base
    * addresses or IIC bus number and device address so that the init function
    * can find the hardware that is to be controlled by the driver.
    */
    u_int32 uDummy;
} CNXT_POWER_HW_PARAMS;

typedef struct
{
   /*
    * Place driver-specific entries here to specify initial state for physical
    * devices controlled by the driver.
    */
    u_int32 uDummy;
} CNXT_POWER_HW_CONFIG;

typedef struct
{
   u_int32                  uNumElements;
   CNXT_POWER_HW_PARAMS  *pHWParams;
   CNXT_POWER_HW_CONFIG  *pHWConfig;
} CNXT_POWER_CONFIG;

typedef struct
{
   u_int32    Plls;        /* See CNXT_POWER_PLL_*       bit definitions below */
   u_int32    Sections;    /* See CNXT_POWER_SECTION_*   bit definitions below */
   bool       VideoDacs;
   bool       RFModulator;
} CNXT_POWER_GENERAL;

typedef struct
{
   CNXT_POWER_GENERAL Gen;
   u_int32    ClocksGated; /* See CNXT_POWER_CLOCK_*     bit definitions below */
   u_int32    ClocksReset; /* See CNXT_POWER_CLOCK_*     bit definitions below */        
   u_int32    WakeupEvents;/* See CNXT_POWER_AWAKE_*     bit definitions below */
   u_int32    SleepDuration; /* in seconds */
} CNXT_POWER_SLEEP;  
                                  
typedef struct
{
   CNXT_POWER_GENERAL Gen;
   bool       SlowCpuMemClock; 
} CNXT_POWER_STANDBY;  
                                  
/**************************************************************************/
/*                                                                        */
/*  Bit settings                                                          */
/*                                                                        */
/**************************************************************************/
#define CNXT_POWER_PLL_USB                   0x00000001
#define CNXT_POWER_PLL_MEMORY                0x00000002
#define CNXT_POWER_PLL_ARM                   0x00000004
#define CNXT_POWER_PLL_AUDIO                 0x00000008

#define CNXT_POWER_SECTION_AUDIO_DECODE      0x00000001
#define CNXT_POWER_SECTION_VIDEO_DECODE      0x00000002
#define CNXT_POWER_SECTION_DRM_VID_CH0       0x00000004
#define CNXT_POWER_SECTION_DRM_VID_CH1       0x00000008
#define CNXT_POWER_SECTION_GXA               0x00000010
#define CNXT_POWER_SECTION_DRM_GRAPHICS      0x00000020
#define CNXT_POWER_SECTION_DRM_ALL           0x00000040
#define CNXT_POWER_SECTION_SOFT_PARSER       0x00000080
#define CNXT_POWER_SECTION_HSDP              0x00000100
#define CNXT_POWER_SECTION_DAA               0x00000200
#define CNXT_POWER_SECTION_AFE               0x00000400
#define CNXT_POWER_SECTION_I2C               0x00000800
#define CNXT_POWER_SECTION_IR                0x00001000
#define CNXT_POWER_SECTION_UART1             0x00002000
#define CNXT_POWER_SECTION_UART2             0x00004000
#define CNXT_POWER_SECTION_UART3             0x00008000
#define CNXT_POWER_SECTION_SMART_CARD0       0x00010000
#define CNXT_POWER_SECTION_SMART_CARD1       0x00020000
#define CNXT_POWER_SECTION_PCI               0x00040000

#define CNXT_POWER_CLOCK_AUDIO               0x00000001
#define CNXT_POWER_CLOCK_ASX                 0x00000002
#define CNXT_POWER_CLOCK_MPEG27MHZ           0x00000004
#define CNXT_POWER_CLOCK_MPEG                0x00000008
#define CNXT_POWER_CLOCK_MEMORY              0x00000010
#define CNXT_POWER_CLOCK_PCI                 0x00000020
#define CNXT_POWER_CLOCK_ASB                 0x00000040
#define CNXT_POWER_CLOCK_ARM                 0x00000080
#define CNXT_POWER_CLOCK_USB                 0x00000100
#define CNXT_POWER_CLOCK_GXA                 0x00000200
#define CNXT_POWER_CLOCK_PAW_NIM             0x00000400


/**************************************************************************/
/*                                                                        */
/*  Events that can awaken the system from sleeping.                      */
/*                                                                        */
/**************************************************************************/
#define CNXT_POWER_AWAKE_RTC                 0x00000001
#define CNXT_POWER_AWAKE_IR                  0x00000002
#define CNXT_POWER_AWAKE_UART1               0x00000004
#define CNXT_POWER_AWAKE_UART2               0x00000008
#define CNXT_POWER_AWAKE_UART3               0x00000010
#define CNXT_POWER_AWAKE_PULSE               0x00000020
#define CNXT_POWER_AWAKE_ON_BUTTON           0x00000040

/* device capability structure */
typedef struct
{
   u_int32 uLength;
   u_int32 uUnitNumber;
} CNXT_POWER_CAPS;

/* unit attributes structure */
typedef struct
{
   u_int32             uLength;
   bool                StandbyActive;
   CNXT_POWER_SLEEP    SleepState;
   CNXT_POWER_STANDBY  StandbyState;
} CNXT_POWER_ATTRIBUTES;

/* module diagnostic information structure */
typedef struct
{
   u_int32                uLength;
   char                   *pszHumanReadableString;
   u_int32                uNumberOfActiveUnits;
   CNXT_POWER_ATTRIBUTES  *pUnitAttributes; /* ptr to array of unit attributes */
} CNXT_POWER_INFORMATION;

/* device handle */
typedef struct cnxt_power_inst *CNXT_POWER_HANDLE;

/* notification function */
typedef CNXT_POWER_STATUS (*CNXT_POWER_PFNNOTIFY) ( 
                                      CNXT_POWER_HANDLE hHandle, 
                                      void                 *pUserData,
                                      CNXT_POWER_EVENT  Event,
                                      void                 *pData,
                                      void                 *pTag );
                 /* Note: This is notification callback for asynchronous options.       */
                 /*       If no asynchronous operation in the driver, change it as needed. */

/******************/
/* API prototypes */
/******************/
CNXT_POWER_STATUS cnxt_power_init ( CNXT_POWER_CONFIG *pCfg );

CNXT_POWER_STATUS cnxt_power_get_driver_caps ( CNXT_POWER_CAPS *pCaps );

CNXT_POWER_STATUS cnxt_power_get_unit_attributes ( CNXT_POWER_HANDLE     hHandle,
                                                   CNXT_POWER_ATTRIBUTES *pAttributes );

CNXT_POWER_STATUS cnxt_power_get_diag_info ( CNXT_POWER_INFORMATION *pInformation );

CNXT_POWER_STATUS cnxt_power_open ( CNXT_POWER_HANDLE    *phHandle,
                                    CNXT_POWER_CAPS      *pCaps,
                                    CNXT_POWER_PFNNOTIFY pfnNotifyFn,
                                    void                 *pUserData );

CNXT_POWER_STATUS cnxt_power_reopen ( CNXT_POWER_HANDLE    *phNewHandle,
                                      CNXT_POWER_HANDLE    hOldHandle,
                                      CNXT_POWER_PFNNOTIFY pNotifyFn,
                                      void                 *pUserData );

CNXT_POWER_STATUS cnxt_power_close ( CNXT_POWER_HANDLE hHandle );

CNXT_POWER_STATUS cnxt_power_sleep( CNXT_POWER_HANDLE hHandle,
                                    CNXT_POWER_SLEEP  *pSleep,
                                    u_int32           *pReason);

CNXT_POWER_STATUS cnxt_power_enter_standby( CNXT_POWER_HANDLE   hHandle,
                                            CNXT_POWER_STANDBY  *pStandby );

CNXT_POWER_STATUS cnxt_power_leave_standby( CNXT_POWER_HANDLE   hHandle );


#endif   /* _POWER_H_ */

/****************************************************************************
 * Modifications:
 * $Log: 
 *  2    mpeg      1.1         10/11/03 5:07:37 PM    Craig Dry       Added 
 *        support for Wabash.
 *  1    mpeg      1.0         9/23/03 4:11:06 PM     Craig Dry       
 * $
 * 
 *    Rev 1.0   23 Sep 2003 15:11:06   dryd
 * SCR(s) 7532 :
 * Power Driver public declarations.
 * 
 ****************************************************************************/

