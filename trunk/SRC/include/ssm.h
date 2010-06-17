/****************************************************************************/
/*                   CONEXANT PROPRIETARY AND CONFIDENTIAL                  */
/*                       SOFTWARE FILE/MODULE HEADER                        */
/*                    Conexant Systems Inc. (c) 2003-2004                   */
/*                              Austin, TX                                  */
/*                         All Rights Reserved                              */
/****************************************************************************/
/*
 * Filename:        ssm.h
 *
 *
 * Description:     Stream Storage Module (SSM) Public Header File
 *
 *
 * Author:          Tim White
 *
 ****************************************************************************/
/* $Id: ssm.h,v 1.9, 2004-06-22 01:32:03Z, Joe Kroesche$
 ****************************************************************************/

#ifndef _SSM_H
#define _SSM_H

/****************
* Include Files *
*****************/
#include "basetype.h"

/**************
* Definitions *
***************/
#define CNXT_SSM_NO_OFFSET        -1

/***********
* Typedefs *
************/

/* SSM driver notification events */
typedef enum
{
   CNXT_SSM_EVENT_TERM,     /* SSM driver has been terminated */
   CNXT_SSM_EVENT_RESET,    /* SSM driver has been reset */
   CNXT_SSM_EVENT_CLOSE,    /* SSM instance has been closed */
   CNXT_SSM_EVENT_IO_COMPLETE,  /* SSM I/O request has completed */
   CNXT_SSM_EVENT_IO_ERROR      /* SSM I/O error occurred */
} CNXT_SSM_EVENT, *PCNXT_SSM_EVENT;

#define CNXT_SSM_STREAMNAME_LEN            256
#define CNXT_SSM_DESCRIPTION_STRING_LEN   1024
#define CNXT_SSM_MAX_NUM_STREAMS           256

#define CNXT_SSM_STREAM_ATTR_READ         (1L<<0)
#define CNXT_SSM_STREAM_ATTR_WRITE        (1L<<1)
#define CNXT_SSM_STREAM_ATTR_TRUNC        (1L<<2)
#define CNXT_SSM_STREAM_ATTR_CREAT        (1L<<3)

/* SSM device capabilities */
typedef struct _CNXT_SSM_CAPS_
{
   u_int32  uLength;
   bool     bExclusive;
   u_int32  uUnitNumber;
   u_int32  uReadTaskPriority;
   u_int32  uWriteTaskPriority;
   u_int8   ucStreamname[CNXT_SSM_STREAMNAME_LEN];
   u_int8   ucDescription[CNXT_SSM_DESCRIPTION_STRING_LEN];
   u_int32  uAttributes;
} CNXT_SSM_CAPS, *PCNXT_SSM_CAPS;

/* SSM driver device handle */
typedef struct cnxt_ssm_inst *CNXT_SSM_HANDLE;

/* Return codes for SSM driver APIs */
typedef enum
{
   CNXT_SSM_OK = 0,
   CNXT_SSM_ALREADY_INIT,
   CNXT_SSM_NOT_INIT,
   CNXT_SSM_BAD_UNIT,
   CNXT_SSM_BAD_HANDLE,
   CNXT_SSM_BAD_PARAMETER,
   CNXT_SSM_RESOURCE_ERROR,
   CNXT_SSM_INTERNAL_ERROR,
   CNXT_SSM_NOT_AVAILABLE,
   CNXT_SSM_BAD_PTR,
   CNXT_SSM_ACTIVE,
   CNXT_SSM_BAD_OFFSET,
   CNXT_SSM_STREAM_NOT_EXIST
} CNXT_SSM_STATUS;

/* SSM driver configuration data, used for driver init */
typedef struct _CNXT_SSM_CONFIG_
{
   u_int32  uLength;
} CNXT_SSM_CONFIG, *PCNXT_SSM_CONFIG;

/* SSM I/O request types */
typedef enum _CNXT_SSM_IO_TYPE_
{
   CNXT_SSM_IO_NONE = 0,
   CNXT_SSM_IO_STREAM_WRITE,
   CNXT_SSM_IO_INDEX_WRITE,
   CNXT_SSM_IO_STREAM_READ,
   CNXT_SSM_IO_INDEX_READ
} CNXT_SSM_IO_TYPE, *PCNXT_SSM_IO_TYPE;

/* Stream data access method */
typedef enum _CNXT_SSM_METHOD
{
    CNXT_SSM_METHOD_FRAME,  /* access data by frames */
    CNXT_SSM_METHOD_MS      /* access data by time */
} CNXT_SSM_METHOD;

/* Directory entry */
typedef struct _CNXT_SSM_DIRECTORY_ENTRY
{
    u_int8  ucStreamname[CNXT_SSM_STREAMNAME_LEN];
    u_int8  ucDescription[CNXT_SSM_DESCRIPTION_STRING_LEN];
    u_int32 uAttributes;
    u_int64 uSize;               /* In bytes */
} CNXT_SSM_DIRECTORY_ENTRY;

/* Directory */
typedef struct _CNXT_SSM_DIRECTORY
{
    u_int32 uSize;
    u_int64 uID;
    u_int32 uNumEntries;
    CNXT_SSM_DIRECTORY_ENTRY Entry[1];
} CNXT_SSM_DIRECTORY;

/********************************************************************/
/*  FUNCTION:     CNXT_SSM_PFNNOTIFY                                */
/*                                                                  */
/*  DESCRIPTION:                                                    */
/*    This is the format of the callback/notify function that the   */
/*    client supplied to the cnxt_ssm_open(),                       */
/*    cnxt_ssm_set_read_notification,                               */
/*    and cnxt_ssm_set_write_notification functions.  This          */
/*    notification function will be called in response to one of    */
/*    the event enumerations of the CNXT_SSM_EVENT type.            */
/*                                                                  */
/*  PARAMETERS:                                                     */
/*      Handle  -   Handle of the device, previously returned from  */
/*                  cnxt_ssm_open().                                */
/*      pUserData - User data pointer passed to cnxt_ssm_open().    */
/*      Event   -   SSM module event type enum.  Refer to the       */
/*                  the CNXT_SSM_EVENT type for the meaning of      */
/*                  each event.                                     */
/*      pType   -   Indicates a type of callback usage, one of:     */
/*               <p>CNXT_SSM_IO_NONE, Used for non-I/O events (e.g. */
/*                  CNXT_SSM_EVENT_CLOSE)                           */
/*               <p>CNXT_SSM_IO_STREAM_WRITE, Indicates stream      */
/*                  (MPEG) data written to stream                   */
/*               <p>CNXT_SSM_IO_INDEX_WRITE, Indicates index data   */
/*                  written to stream                               */
/*               <p>CNXT_SSM_IO_STREAM_READ, Indicates stream       */
/*                  (MPEG) data read from stream                    */
/*               <p>CNXT_SSM_IO_INDEX_READ, Indicates index data    */
/*                  read from stream                                */
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
/*    The callback type CNXT_SSM_IO_NONE will be associated only    */
/*    with the following events:                                    */
/*    * CNXT_SSM_EVENT_TERM                                         */
/*    * CNXT_SSM_EVENT_RESET                                        */
/*    * CNXT_SSM_EVENT_CLOSE                                        */
/*    while any of the _IO_ callback types can be associated with   */
/*    any of the events.                                            */
/********************************************************************/
typedef CNXT_SSM_STATUS (*CNXT_SSM_PFNNOTIFY)(
   CNXT_SSM_HANDLE      Handle,
   void                 *pUserData,
   CNXT_SSM_EVENT       Event,
   CNXT_SSM_IO_TYPE     Type,
   void                 *Tag);

/**********************
* Function Prototypes *
***********************/

/*
 * Client API's
 */
CNXT_SSM_STATUS cnxt_ssm_init(CNXT_SSM_CONFIG *pCfg);

CNXT_SSM_STATUS cnxt_ssm_term(void);

CNXT_SSM_STATUS cnxt_ssm_reset(CNXT_SSM_CONFIG *pCfg);

CNXT_SSM_STATUS cnxt_ssm_get_units(u_int32 *puCount);

CNXT_SSM_STATUS cnxt_ssm_get_unit_caps(u_int32        uUnitNumber,
                                       CNXT_SSM_CAPS  *pCaps);

CNXT_SSM_STATUS cnxt_ssm_open(CNXT_SSM_HANDLE     *pHandle,
                              CNXT_SSM_CAPS       *pCaps,
                              CNXT_SSM_PFNNOTIFY  pNotifyFn,
                              void                *pUserData);

CNXT_SSM_STATUS cnxt_ssm_close(CNXT_SSM_HANDLE Handle);

CNXT_SSM_STATUS cnxt_ssm_erase(CNXT_SSM_HANDLE Handle);

/*
 * Record & Playback module API's
 */
CNXT_SSM_STATUS cnxt_ssm_set_write_notification(CNXT_SSM_HANDLE Handle,
                                                CNXT_SSM_PFNNOTIFY pNotifyFn,
                                                void *pUserData);

CNXT_SSM_STATUS cnxt_ssm_set_read_notification(CNXT_SSM_HANDLE Handle,
                                               CNXT_SSM_PFNNOTIFY pNotifyFn,
                                               void *pUserData);

CNXT_SSM_STATUS cnxt_ssm_write_stream_data(CNXT_SSM_HANDLE Handle,
                                           u_int8 *pBuffer,
                                           u_int32 uSize,
                                           void *pTag);

CNXT_SSM_STATUS cnxt_ssm_write_index_data(CNXT_SSM_HANDLE Handle,
                                          u_int8 *pBuffer,
                                          u_int32 uSize,
                                          void *pTag);

CNXT_SSM_STATUS cnxt_ssm_read_stream_data(CNXT_SSM_HANDLE Handle,
                                          u_int8 *pBuffer,
                                          u_int32 uSize,
                                          u_int64 uOffset,
                                          void *pTag);

CNXT_SSM_STATUS cnxt_ssm_read_index_data(CNXT_SSM_HANDLE Handle,
                                         u_int8 *pBuffer,
                                         u_int32 uSize,
                                         u_int64 uOffset,
                                         void *pTag);

#endif /* _SSM_H */

/**************************************************************************** 
 * Modifications:
 * $Log: 
 *  10   mpeg      1.9         6/21/04 8:32:03 PM     Joe Kroesche    CR(s) 
 *        9522 9523 : updated comments for auto-generated documentation
 *  9    mpeg      1.8         4/6/04 10:05:19 AM     Tim White       CR(s) 
 *        8735 8736 : Remove unimplemented functions from 1.8 release.  Use 
 *        previous
 *        version of the file (1.7) going forward.
 *        
 *  8    mpeg      1.7         3/23/04 5:17:10 PM     Tim White       CR(s) 
 *        8627 8628 : Changes from review and code drop.  Added directory 
 *        function stubs, added 
 *        some implementation notes, etc.
 *        
 *  7    mpeg      1.6         3/19/04 11:22:20 AM    Tim White       CR(s) 
 *        8545 : Added stream erase functionality.
 *        
 *  6    mpeg      1.5         3/18/04 9:18:06 AM     Tim White       CR(s) 
 *        8545 : Changes from initial bringup.
 *        
 *  5    mpeg      1.4         3/12/04 9:52:33 AM     Tim White       CR(s) 
 *        8545 : Fix compile error.
 *        
 *  4    mpeg      1.3         3/10/04 5:23:07 PM     Tim Ross        CR(s) 
 *        8545 : Corrected ssm handle typedef.
 *  3    mpeg      1.2         3/10/04 4:15:50 PM     Tim White       CR(s) 
 *        8545 : Fixed compile errors.
 *        
 *  2    mpeg      1.1         3/10/04 9:56:00 AM     Tim White       CR(s) 
 *        8545 : Initial code drop for Phase_1.
 *        
 *  1    mpeg      1.0         2/23/04 12:59:49 PM    Tim White       CR(s) 
 *        8452 : Stream Storage Module (SSM) public header file.
 *        
 * $
 ****************************************************************************/

