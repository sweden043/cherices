/****************************************************************************/
/*                   CONEXANT PROPRIETARY AND CONFIDENTIAL                  */
/*                       SOFTWARE FILE/MODULE HEADER                        */
/*                    Conexant Systems Inc. (c) 2003-2004                   */
/*                               Austin, TX                                 */
/*                            All Rights Reserved                           */
/****************************************************************************/
/*
 * Filename:        play.h
 *
 *
 * Description:     PVR Play Public API Header File
 *
 *
 * Author:          Tim White
 *
 ****************************************************************************/
/* $Id: play.h,v 1.14, 2004-06-22 21:36:11Z, Tim White$
 ****************************************************************************/

#ifndef _PLAY_H
#define _PLAY_H

/****************
* Include Files *
*****************/
#include "basetype.h"
#include "ssm.h"

/**************
* Definitions *
***************/
/*
 * PVR Playback Speed Definitions
 */
#define CNXT_PLAY_SPEED_64X_FWD            65536
#define CNXT_PLAY_SPEED_32X_FWD            32768
#define CNXT_PLAY_SPEED_30X_FWD            30720
#define CNXT_PLAY_SPEED_27X_FWD            27648
#define CNXT_PLAY_SPEED_24X_FWD            24576
#define CNXT_PLAY_SPEED_21X_FWD            21504
#define CNXT_PLAY_SPEED_18X_FWD            18432
#define CNXT_PLAY_SPEED_16X_FWD            16384
#define CNXT_PLAY_SPEED_15X_FWD            15360
#define CNXT_PLAY_SPEED_12X_FWD            12288
#define CNXT_PLAY_SPEED_9X_FWD              9216
#define CNXT_PLAY_SPEED_8X_FWD              8192
#define CNXT_PLAY_SPEED_6X_FWD              6144
#define CNXT_PLAY_SPEED_4X_FWD              4096
#define CNXT_PLAY_SPEED_3X_FWD              3072
#define CNXT_PLAY_SPEED_2X_FWD              2048
#define CNXT_PLAY_SPEED_1X_FWD              1024
#define CNXT_PLAY_SPEED_1_2_FWD              512
#define CNXT_PLAY_SPEED_1_4_FWD              256
#define CNXT_PLAY_SPEED_1_8_FWD              128
#define CNXT_PLAY_SPEED_1_16_FWD              64
#define CNXT_PLAY_SPEED_1_32_FWD              32
#define CNXT_PLAY_SPEED_1_64_FWD              16
#define CNXT_PLAY_SPEED_0                      0
#define CNXT_PLAY_SPEED_1_64_REV             -16
#define CNXT_PLAY_SPEED_1_32_REV             -32
#define CNXT_PLAY_SPEED_1_16_REV             -64
#define CNXT_PLAY_SPEED_1_8_REV             -128
#define CNXT_PLAY_SPEED_1_4_REV             -256
#define CNXT_PLAY_SPEED_1_2_REV             -512
#define CNXT_PLAY_SPEED_1X_REV             -1024
#define CNXT_PLAY_SPEED_2X_REV             -2048
#define CNXT_PLAY_SPEED_3X_REV             -3072
#define CNXT_PLAY_SPEED_4X_REV             -4096
#define CNXT_PLAY_SPEED_6X_REV             -6144
#define CNXT_PLAY_SPEED_8X_REV             -8192
#define CNXT_PLAY_SPEED_9X_REV             -9216
#define CNXT_PLAY_SPEED_12X_REV           -12288
#define CNXT_PLAY_SPEED_15X_REV           -15360
#define CNXT_PLAY_SPEED_16X_REV           -16384
#define CNXT_PLAY_SPEED_18X_REV           -18432
#define CNXT_PLAY_SPEED_21X_REV           -21504
#define CNXT_PLAY_SPEED_24X_REV           -24576
#define CNXT_PLAY_SPEED_27X_REV           -27648
#define CNXT_PLAY_SPEED_30X_REV           -30720
#define CNXT_PLAY_SPEED_32X_REV           -32768
#define CNXT_PLAY_SPEED_64X_REV           -65536

/***********
* Typedefs *
************/

/* Notification Events */
typedef enum
{
   CNXT_PLAY_EVENT_TERM,    /* driver was terminated */
   CNXT_PLAY_EVENT_RESET,   /* driver was reset */
   CNXT_PLAY_EVENT_CLOSE,   /* play instance was closed */
   CNXT_PLAY_EVENT_HALT     /* playback halted due to lack of data */
} CNXT_PLAY_EVENT, *PCNXT_PLAY_EVENT;

/* device capability structure */
typedef struct _CNXT_PLAY_CAPS
{
   u_int32  uLength;
   bool     bExclusive;
   u_int32  uUnitNumber;
} CNXT_PLAY_CAPS, *PCNXT_PLAY_CAPS;

/* Play device instance handle */
typedef struct cnxt_play_inst *CNXT_PLAY_HANDLE;

/* Return codes for play driver APIs */
typedef enum
{
   CNXT_PLAY_OK = 0,
   CNXT_PLAY_ALREADY_INIT,
   CNXT_PLAY_NOT_INIT,
   CNXT_PLAY_BAD_UNIT,
   CNXT_PLAY_CLOSED_HANDLE,
   CNXT_PLAY_BAD_HANDLE,
   CNXT_PLAY_BAD_PARAMETER,
   CNXT_PLAY_RESOURCE_ERROR,
   CNXT_PLAY_INTERNAL_ERROR,
   CNXT_PLAY_NOT_AVAILABLE,
   CNXT_PLAY_ACTIVE,
   CNXT_PLAY_BAD_PTR
} CNXT_PLAY_STATUS;

/* Configuration data used for driver init */
typedef struct _CNXT_PLAY_CONFIG
{
   u_int32  uLength;
} CNXT_PLAY_CONFIG, *PCNXT_PLAY_CONFIG;

/*
 * Set position method definitions
 */
typedef enum _CNXT_PLAY_METHOD
{
    CNXT_PLAY_METHOD_FRAME,     /* specify position by frame */
    CNXT_PLAY_METHOD_MS         /* specify position by time */
} CNXT_PLAY_METHOD;

/* Position offset origin */
typedef enum _CNXT_PLAY_ORIGIN
{
    CNXT_PLAY_ORIGIN_CURRENT = 0, /* offset from current position */
    CNXT_PLAY_ORIGIN_BEGIN,       /* offset from beginning of file */
    CNXT_PLAY_ORIGIN_END          /* offset from end of file */
} CNXT_PLAY_ORIGIN;

/* Play driver control */
typedef enum _CNXT_PLAY_CTRL
{
    CNXT_PLAY_CTRL_START,
    CNXT_PLAY_CTRL_STOP
} CNXT_PLAY_CTRL;

/********************************************************************/
/*  FUNCTION:     CNXT_PLAY_PFNNOTIFY                               */
/*                                                                  */
/*  DESCRIPTION:                                                    */
/*    This is the format of the callback/notify function that the   */
/*    client supplied to the cnxt_play_open() function call. This   */
/*    notification function will be called in response to one of    */
/*    the event enumerations listed in the Notify Event Description */
/*    Table in the notes section below.                             */
/*                                                                  */
/*  PARAMETERS:                                                     */
/*      Handle  -   Handle of the device, previously returned from  */
/*                  cnxt_play_open().                               */
/*      pUserData - User data pointer passed to cnxt_play_open().   */
/*      Event   -   Playback module event type enum. The table in   */
/*                  the notes section below describes the meaning   */
/*                  of each event enum.                             */
/*      pData   -   Pointer to event specific data. See Notify      */
/*                  Event Description Table below.                  */
/*      Tag     -   Tag passed into the initiating asynchronous     */
/*                  function called by the user. For non-initiated  */
/*                  events (i.e. hardware event driven callbacks),  */
/*                  this is zero.                                   */
/*                                                                  */
/*  CONTEXT:                                                        */
/*    Will be called in non-interrupt context.                      */
/*                                                                  */
/*  RETURN VALUES:                                                  */
/*      CNXT_PLAY_OK - Callback completed successfully.             */
/*                                                                  */
/*  NOTES:                                                          */
/*    The client should minimize the work done in this call, must   */
/*    not block, and should return as quickly as possible.          */
/*                                                                  */
/*    The notification events are defined in the following table:   */
/*    <TABLE>                                                       */
/*    Event Value            pData   \Description                   */
/*    -----------            -----   -----------                    */
/*    CNXT_PLAY_EVENT_TERM   NULL    Module has been terminated     */
/*    CNXT_PLAY_EVENT_RESET  NULL    Module has been reset          */
/*    CNXT_PLAY_EVENT_CLOSE  NULL    Playback instance has been     */
/*                                    closed                        */
/*    CNXT_PLAY_EVENT_HALT   NULL    Playback has halted due to lack*/
/*                                    of playback stream data       */
/*    </TABLE>                                                      */
/********************************************************************/
typedef CNXT_PLAY_STATUS (*CNXT_PLAY_PFNNOTIFY)(
   CNXT_PLAY_HANDLE     Handle,
   void                 *pUserData,
   CNXT_PLAY_EVENT      Event,
   void                 *pData,
   void                 *Tag);

/**********************
* Function Prototypes *
***********************/
CNXT_PLAY_STATUS cnxt_play_init(CNXT_PLAY_CONFIG *pCfg);

CNXT_PLAY_STATUS cnxt_play_term(void);

CNXT_PLAY_STATUS cnxt_play_reset(CNXT_PLAY_CONFIG *pCfg);

CNXT_PLAY_STATUS cnxt_play_get_units(u_int32 *puCount);

CNXT_PLAY_STATUS cnxt_play_get_unit_caps(u_int32         uUnitNumber,
                                         CNXT_PLAY_CAPS  *pCaps);

CNXT_PLAY_STATUS cnxt_play_open(CNXT_PLAY_HANDLE     *pHandle,
                                CNXT_PLAY_CAPS       *pCaps,
                                CNXT_PLAY_PFNNOTIFY  pNotifyFn,
                                void                 *pUserData);

CNXT_PLAY_STATUS cnxt_play_close(CNXT_PLAY_HANDLE Handle);

CNXT_PLAY_STATUS cnxt_play_ctrl(CNXT_PLAY_HANDLE Handle, CNXT_PLAY_CTRL Ctrl);

CNXT_PLAY_STATUS cnxt_play_set_demux(CNXT_PLAY_HANDLE Handle, u_int32 uDmxID);

CNXT_PLAY_STATUS cnxt_play_associate_storage(CNXT_PLAY_HANDLE Handle,
                                             CNXT_SSM_HANDLE SSM_Handle);

CNXT_PLAY_STATUS cnxt_play_set_speed(CNXT_PLAY_HANDLE Handle,
                                     int32 nRequestedSpeed,
                                     int32 *nActualSpeed);

CNXT_PLAY_STATUS cnxt_play_get_speed(CNXT_PLAY_HANDLE Handle,
                                     int32 *nActualSpeed);

CNXT_PLAY_STATUS cnxt_play_set_position(CNXT_PLAY_HANDLE Handle,
                                        CNXT_PLAY_METHOD Method,
                                        CNXT_PLAY_ORIGIN Origin,
                                        int64 nRequestedOffset,
                                        int64 *nActualOffset);

CNXT_PLAY_STATUS cnxt_play_get_position(CNXT_PLAY_HANDLE Handle,
                                        CNXT_PLAY_METHOD Method,
                                        int64 *nActualOffset);

#endif /* _PLAY_H */

/****************************************************************************
 * Modifications:
 * $Log: 
 *  15   mpeg      1.14        6/22/04 4:36:11 PM     Tim White       CR(s) 
 *        9542 9543 : Fix typo from record_inst to play_inst.
 *        
 *  14   mpeg      1.13        6/21/04 1:38:30 PM     Joe Kroesche    CR(s) 
 *        9522 9523 : updated file comments to improve automated documentation
 *  13   mpeg      1.12        5/4/04 2:24:06 PM      Tim White       CR(s) 
 *        9088 9089 : Added trick mode function prototypes and definitions.
 *        
 *  12   mpeg      1.11        4/6/04 10:00:34 AM     Tim White       CR(s) 
 *        8735 8736 : Remove unimplemented functions from 1.8 release.  Use 
 *        previous
 *        version of file (1.10) moving forward.
 *        
 *  11   mpeg      1.10        3/23/04 3:29:38 PM     Tim White       CR(s) 
 *        8627 8628 : Minor change to parameter matching code and spec.
 *        
 *  10   mpeg      1.9         3/19/04 11:44:57 AM    Tim White       CR(s) 
 *        8545 : Modify notify_all_clients() to support notification to
 *        clients of the same pUnitInst only for some cases.
 *        Add EOF (HALT) callback capability.
 *        Add PTS handling for playback with AVSync enabled.
 *        
 *  9    mpeg      1.8         3/18/04 9:15:18 AM     Tim White       CR(s) 
 *        8545 : Changes from initial bringup.
 *        
 *  8    mpeg      1.7         3/12/04 9:58:58 AM     Tim White       CR(s) 
 *        8545 : Fix compile error.
 *        
 *  7    mpeg      1.6         3/10/04 5:17:40 PM     Tim Ross        CR(s) 
 *        8545 : Added associate_storage call.
 *  6    mpeg      1.5         3/10/04 4:21:53 PM     Tim White       CR(s) 
 *        8545 : Fix compile errors.
 *        
 *  5    mpeg      1.4         2/23/04 1:01:31 PM     Tim White       CR(s) 
 *        8449 : Initial drop of code template.
 *        
 *  4    mpeg      1.3         11/19/03 10:20:06 AM   Tim White       CR(s): 
 *        7987 Remove stand-alone dmxdma driver and use demux DMA input 
 *        extension.
 *        
 *  3    mpeg      1.2         10/28/03 3:14:40 PM    Tim White       CR(s): 
 *        7733 Add dmxdma.h and fix #ifdef's.
 *        
 *  2    mpeg      1.1         10/17/03 10:02:16 AM   Tim White       CR(s): 
 *        7675 Add the Demux DMA Input Module.
 *        
 *  1    mpeg      1.0         10/15/03 5:38:39 PM    Tim White       CR(s): 
 *        7659 PVR Play Module Public Header
 *        
 * $
 ****************************************************************************/

