/***************************************************************************
*                   CONEXANT PROPRIETARY AND CONFIDENTIAL                  *
*                       SOFTWARE FILE/MODULE HEADER                        *
*                      Conexant Systems Inc. (c) 2003                      *
*                                Austin, TX                                *
*                           All Rights Reserved                            *
****************************************************************************
*
* Filename: record.h
*
* Description: Main PVR record driver public header file. 
*
* Author: Tim Ross
*
****************************************************************************
* $Id: record.h,v 1.11, 2004-06-22 01:30:07Z, Joe Kroesche$
****************************************************************************/


#ifndef _RECORD_H_
#define _RECORD_H_

/****************
* Include Files *
*****************/
#include "basetype.h"
#include "ssm.h"

/**************
* Definitions *
***************/
/*
 * Picture event header
 */
#define  CNXT_RECORD_EVENT_EVENT_RATE_SHIFT           16
#define  CNXT_RECORD_EVENT_EVENT_RATE_MASK            (0x3f << CNXT_RECORD_EVENT_FRAME_RATE_SHIFT)
#define  CNXT_RECORD_EVENT_FRAME_RATE_SHIFT           12
#define  CNXT_RECORD_EVENT_FRAME_RATE_MASK            (0xf << CNXT_RECORD_EVENT_FRAME_RATE_SHIFT)
#define     CNXT_RECORD_EVENT_FRAME_RATE_FORBIDDEN    0x0
#define     CNXT_RECORD_EVENT_FRAME_RATE_23_976       0x1
#define     CNXT_RECORD_EVENT_FRAME_RATE_24           0x2
#define     CNXT_RECORD_EVENT_FRAME_RATE_25           0x3
#define     CNXT_RECORD_EVENT_FRAME_RATE_29_97        0x4
#define     CNXT_RECORD_EVENT_FRAME_RATE_30           0x5
#define     CNXT_RECORD_EVENT_FRAME_RATE_50           0x6
#define     CNXT_RECORD_EVENT_FRAME_RATE_59_94        0x7
#define     CNXT_RECORD_EVENT_FRAME_RATE_60           0x8
#define  CNXT_RECORD_EVENT_PROG_SEQ_SHIFT             11
#define  CNXT_RECORD_EVENT_PROG_SEQ_MASK              (1UL << CNXT_RECORD_EVENT_PROG_SEQ_SHIFT)
#define  CNXT_RECORD_EVENT_PROG_FRAME_SHIFT           10  
#define  CNXT_RECORD_EVENT_PROG_FRAME_MASK            (1UL << CNXT_RECORD_EVENT_PROG_FRAME_SHIFT)
#define  CNXT_RECORD_EVENT_PICT_STRUCT_SHIFT          8
#define  CNXT_RECORD_EVENT_PICT_STRUCT_MASK           (0x3UL << CNXT_RECORD_EVENT_PICT_STRUCT_SHIFT)
#define     CNXT_RECORD_EVENT_PICT_STRUCT_TOP_FIELD   0x1
#define     CNXT_RECORD_EVENT_PICT_STRUCT_BOT_FIELD   0x2
#define     CNXT_RECORD_EVENT_PICT_STRUCT_FRAME       0x3
#define  CNXT_RECORD_EVENT_TOP_FIELD_FIRST_SHIFT      7       
#define  CNXT_RECORD_EVENT_TOP_FIELD_FIRST_MASK       (1UL << CNXT_RECORD_EVENT_TOP_FIELD_FIRST_SHIFT)
#define  CNXT_RECORD_EVENT_REPEAT_FIRST_FIELD_SHIFT   6          
#define  CNXT_RECORD_EVENT_REPEAT_FIRST_FIELD_MASK    (1UL << CNXT_RECORD_EVENT_REPEAT_FIRST_FIELD_SHIFT)
#define     CNXT_RECORD_EVENT_1_PROG_FRAME            0x0
#define     CNXT_RECORD_EVENT_2_PROG_FRAMES           0x1
#define     CNXT_RECORD_EVENT_3_PROG_FRAMES           0x3
#define  CNXT_RECORD_EVENT_PICT_TYPE_SHIFT            4
#define  CNXT_RECORD_EVENT_PICT_TYPE_MASK             (0x3UL << CNXT_RECORD_EVENT_PICT_TYPE_SHIFT)
#define     CNXT_RECORD_EVENT_PICT_TYPE_I_FRAME       0x1
#define     CNXT_RECORD_EVENT_PICT_TYPE_P_FRAME       0x2
#define     CNXT_RECORD_EVENT_PICT_TYPE_B_FRAME       0x3
#define  CNXT_RECORD_EVENT_PCE_SHIFT                  3
#define  CNXT_RECORD_EVENT_PCE_MASK                   (1UL << CNXT_RECORD_EVENT_PCE_SHIFT)
#define  CNXT_RECORD_EVENT_PICT_SHIFT                 2
#define  CNXT_RECORD_EVENT_PICT_MASK                  (1UL << CNXT_RECORD_EVENT_PICT_SHIFT)
#define  CNXT_RECORD_EVENT_SEQ_EXT_SHIFT              1
#define  CNXT_RECORD_EVENT_SEQ_EXT_MASK               (1UL << CNXT_RECORD_EVENT_SEQ_EXT_SHIFT)
#define  CNXT_RECORD_EVENT_SEQ_SHIFT                  0
#define  CNXT_RECORD_EVENT_SEQ_MASK                   (1UL << CNXT_RECORD_EVENT_SEQ_SHIFT)


/***********
* Typedefs *
************/

/* Record driver notification events */
typedef enum
{
   /*
    * The SSM indicated that an error occurred while attempting to store
    * stream data to disk and some or all of the data was lost.
    * pData is NULL.
    */
   CNXT_RECORD_EVENT_STREAM_RECORD_FAILURE,
   /*
    * The SSM indicated that an error occurred while attempting to store
    * index data to disk and some or all of the data was lost.
    * pData is NULL.
    */
   CNXT_RECORD_EVENT_INDEX_RECORD_FAILURE,
   /*
    * Recording has stopped for the given record unit. This is in response
    * to the SSM indicating that the stream handle was closed by another
    * client or a storage error occurred.
    * pData is NULL.
    */
   CNXT_RECORD_EVENT_RECORDING_STOPPED,
   /* Module has been terminated. */
   CNXT_RECORD_EVENT_TERM,
   /* Module has been reset. */
   CNXT_RECORD_EVENT_RESET
} CNXT_RECORD_EVENT, *PCNXT_RECORD_EVENT;

/* Record driver capabilities */
typedef struct _CNXT_RECORD_CAPS_  {
   u_int32  uLength;
   bool     bExclusive;
   u_int32  uUnitNumber;
} CNXT_RECORD_CAPS, *PCNXT_RECORD_CAPS;

/* Record driver device handle */
typedef struct cnxt_record_inst *CNXT_RECORD_HANDLE;

/* Return codes for record driver APIs */
typedef enum
{
   CNXT_RECORD_OK = 0,
   CNXT_RECORD_ALREADY_INIT,
   CNXT_RECORD_NOT_INIT,
   CNXT_RECORD_BAD_UNIT,
   CNXT_RECORD_CLOSED_HANDLE,
   CNXT_RECORD_BAD_HANDLE,
   CNXT_RECORD_BAD_PARAMETER,
   CNXT_RECORD_RESOURCE_ERROR,
   CNXT_RECORD_INTERNAL_ERROR,
   CNXT_RECORD_NOT_AVAILABLE,
   CNXT_RECORD_BAD_PTR,
   CNXT_RECORD_HARDWARE_ERROR
} CNXT_RECORD_STATUS;

/* Record driver configuration data, used for driver init */
typedef struct _CNXT_RECORD_CONFIG_ {
   u_int32  uLength;
} CNXT_RECORD_CONFIG, *PCNXT_RECORD_CONFIG;

/********************************************************************/
/*  FUNCTION:     CNXT_RECORD_PFNNOTIFY                             */
/*                                                                  */
/*  DESCRIPTION:                                                    */
/*    This is the format of the callback/notify function that the   */
/*    client supplied to the cnxt_record_open() function call. This */
/*    notification function will be called in response to one of    */
/*    the event enumerations of the CNXT_RECORD_EVENT type.         */
/*                                                                  */
/*  PARAMETERS:                                                     */
/*      Handle  -   Handle of the device, previously returned from  */
/*                  cnxt_record_open().                             */
/*      pUserData - User data pointer passed to cnxt_record_open(). */
/*      Event   -   Record module event type enum.  Refer to the    */
/*                  the CNXT_RECORD_EVENT type for the meaning of   */
/*                  each event.                                     */
/*      pData   -   Pointer to event specific data.                 */
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
/********************************************************************/
typedef CNXT_RECORD_STATUS (*CNXT_RECORD_PFNNOTIFY)(
   CNXT_RECORD_HANDLE   Handle, 
   void                 *pUserData, 
   CNXT_RECORD_EVENT    Event,
   void                 *pData,
   void                 *Tag);

/*
 * Picture event data format
 */
typedef struct _CNXT_RECORD_PICT_EVENT_
{
   u_int32  uHdr;                      /* details of event that occurred */
   u_int32  uTime;                     /* picture decode time (milliseconds) */
   u_int32  uFieldCnt;                 /* display field # from start of stream */
   u_int32  uPrevFrameRatePktOff;      /* pkt offset to last frame rate change */
   u_int32  uPkt;                      /* packet # containing start of event */
} CNXT_RECORD_PICT_EVENT, *PCNXT_RECORD_PICT_EVENT;

/*
 * Picture event size in bytes
 */
#define  CNXT_RECORD_PICT_EVENT_SIZE   (sizeof(CNXT_RECORD_PICT_EVENT))

  
/**********************
* Function Prototypes *
***********************/
CNXT_RECORD_STATUS cnxt_record_init(CNXT_RECORD_CONFIG *pCfg);

CNXT_RECORD_STATUS cnxt_record_term(void);

CNXT_RECORD_STATUS cnxt_record_reset(CNXT_RECORD_CONFIG *pCfg);

CNXT_RECORD_STATUS cnxt_record_get_units(u_int32 *puCount);

CNXT_RECORD_STATUS cnxt_record_get_unit_caps(
   u_int32           uUnitNumber, 
   CNXT_RECORD_CAPS  *pCaps);

CNXT_RECORD_STATUS cnxt_record_open(
   CNXT_RECORD_HANDLE      *pHandle,
   CNXT_RECORD_CAPS        *pCaps,
   CNXT_RECORD_PFNNOTIFY   pNotifyFn,
   void                    *pUserData);

CNXT_RECORD_STATUS cnxt_record_close(CNXT_RECORD_HANDLE Handle);

CNXT_RECORD_STATUS cnxt_record_set_demux(CNXT_RECORD_HANDLE Handle, u_int32 uDmxID);

CNXT_RECORD_STATUS cnxt_record_associate_storage(
   CNXT_RECORD_HANDLE   Handle, 
   CNXT_SSM_HANDLE      SSMHandle);

#endif   /* _RECORD_H_ */

/****************************************************************************
* Modifications:
* $Log: 
*  12   mpeg      1.11        6/21/04 8:30:07 PM     Joe Kroesche    CR(s) 9522
*         9523 : fixed module name typo in a comment
*  11   mpeg      1.10        6/21/04 5:19:54 PM     Joe Kroesche    CR(s) 9522
*         9523 : modified the comments to improve auto-generated documentation
*  10   mpeg      1.9         6/14/04 4:09:20 PM     Tim Ross        CR(s) 9456
*         9457 : Initial debug of demux-record style event translation code. 
*  9    mpeg      1.8         5/21/04 5:08:06 PM     Tim Ross        CR(s) 9282
*         9283 : Added definition of sequence extension bit to event.
*  8    mpeg      1.7         5/14/04 4:15:18 PM     Tim Ross        CR(s) 9205
*         9206 : Added demux event post-processing before being stored to disk.
*  7    mpeg      1.6         3/4/04 1:20:06 PM      Tim Ross        CR(s) 8451
*         : First draft of phase 1 PVR.
*  6    mpeg      1.5         2/24/04 9:01:48 AM     Tim Ross        CR(s) 8451
*         : Added code.
*  5    mpeg      1.4         1/22/04 3:47:53 PM     Tim Ross        CR(s) 8263
*         : Added Rio MIA structure.
*  4    mpeg      1.3         11/19/03 10:42:01 AM   Tim White       CR(s): 
*        7987 Use demux DMA input extension instead of dmxdma driver which has 
*        been removed.
*        
*  3    mpeg      1.2         10/28/03 3:13:12 PM    Tim White       CR(s): 
*        7733 Fix #ifdef's.
*        
*  2    mpeg      1.1         10/17/03 9:52:55 AM    Tim White       CR(s): 
*        7674 Change Header to Id.
*        
*  1    mpeg      1.0         10/15/03 5:34:06 PM    Tim White       CR(s): 
*        7659 PVR Record Module Public Header File
*        
* $
****************************************************************************/

