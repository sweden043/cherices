/****************************************************************************/
/*                   CONEXANT PROPRIETARY AND CONFIDENTIAL                  */
/*                       SOFTWARE FILE/MODULE HEADER                        */
/*                Copyright Conexant Systems Inc. 1998-2003                 */
/*                         All Rights Reserved                              */
/****************************************************************************/
/*
 * Filename:        oob_api.h
 *
 *
 * Description:     API header file for davic_oobfe driver
 *
 *
 * Author:          Yong Lu/Ram B.
 *
 ****************************************************************************/
/* $Header: oob_api.h, 6, 7/29/03 3:45:08 PM, $
 ****************************************************************************/
#ifndef _OOB_API_H_
#define _OOB_API_H_

#include <basetype.h>

/* 
 * DEFINEs
 * =================================================================================
 */
#define CNXT_DAVIC_OOBFE_MAX_CHANNELS_OPEN 16

/* 
 * TYPEDEFs
 * =================================================================================
 */
#ifndef PVOID
typedef void* PVOID;
#endif /* PVOID */

/* 
 * CNXT_DAVIC_OOBFE_STATUS
 * =================================================================================
 */
typedef enum
{
   CNXT_DAVIC_OOBFE_OK,
   CNXT_DAVIC_OOBFE_ERROR,
   CNXT_DAVIC_OOBFE_INTERNAL_ERROR,
   CNXT_DAVIC_OOBFE_BAD_UNIT,
   CNXT_DAVIC_OOBFE_BAD_VCI_VPI,
   CNXT_DAVIC_OOBFE_MAX_VCI_VPI,
   CNXT_DAVIC_OOBFE_BAD_PARAMETER,
   CNXT_DAVIC_OOBFE_BAD_HANDLE,
   CNXT_DAVIC_OOBFE_CLOSED_HANDLES,
   CNXT_DAVIC_OOBFE_NOT_AVAILABLE,
   CNXT_DAVIC_OOBFE_NOT_INITIALIZED
} CNXT_DAVIC_OOBFE_STATUS;

/* 
 * CNXT_DAVIC_OOBFE_CHANNEL_STATUS
 * =================================================================================
 */
typedef enum
{
   CNXT_DAVIC_OOBFE_CHANNEL_OPEN_DISABLED,
   CNXT_DAVIC_OOBFE_CHANNEL_OPEN_ENABLED,
   CNXT_DAVIC_OOBFE_CHANNEL_DOES_NOT_EXIST
} CNXT_DAVIC_OOBFE_CHANNEL_STATUS;

/* 
 * CNXT_DAVIC_OOBFE_UPSTREAM_DATA_TYPE
 * =================================================================================
 */
typedef enum
{
   MMM = 0,
   OTHER_TYPE
} CNXT_DAVIC_OOBFE_UPSTREAM_DATA_TYPE;

/* 
 * CNXT_DAVIC_OOBFE_DOWNSTREAM_PARAMS
 * =================================================================================
 */
/* Note: all values in the following structure are little endian */
typedef struct
{
   u_int8   downstream_modulation_type;  /* downstream modulation type : 0 == 64qam, 1 == 256qam */
   u_int8   scan_symbol_rate;  /* scan symbol rate : 0 == don't scan, 1 == scan 
                                  Scanning is done on the list of symbol rates below. 
                                  A Zero value in a symbol rate means the entry is unused.
                                  If this bit = 0 use the value in Symbol Rate 0. */
   u_int8   scan_spectral_inversion;  /* scan spectral inversion : 0 == don't scan, 1 == scan */ 
   u_int8   scan_modulation_type;  /* scan modulation type : 0 == don't scan, 1 == scan */ 
   u_int8   scan_frequency;  /* scan frequency : 0 == don't scan, 1 == scan */
   u_int32  downstream_frequency;  /* downstream frequency. Units : Hz. */
   u_int32  ds_frequency_scan_start; /* Lower limit of frequency band. Used as a lower limit 
                                        for frequency scanning. Units : Hz */
   u_int32  ds_frequency_scan_stop;  /* Upper limit of frequency band. Used as an upper limit 
                                        for frequency scanning. Units*/
   u_int32  ds_frequency_step_size;  /* Step size during frequency scanning. Units : Hz. */
   u_int32  ds_symbol_rate0;  /* symbols per second */
   u_int32  ds_symbol_rate1;  /* symbols per second */
   u_int32  ds_symbol_rate2;  /* symbols per second */
   u_int32  ds_symbol_rate3;  /* symbols per second */
   char   cSdpVhfL;  /* delay point for the VHF-L band  */
   char   cSdpVhfH;  /* delay point for the VHF-H band  */
   char   cSdpUhf;  /* delay point for the UHF band  */
   u_int8   spectral_inversion;  /* spectral inversion  : 0 == Don't invert.1 == Invert. */
} CNXT_DAVIC_OOBFE_DOWNSTREAM_PARAMS;

/* 
 * CNXT_DAVIC_OOBFE_CALLBACKDATA, PCNXT_DAVIC_OOBFE_CALLBACKDATA
 * =================================================================================
 * data definition for callback to its client 
 */
typedef struct
{
   u_int8   *pData;
   unsigned char *mBlkPtr;
   u_int32  chid;
   u_int32  length;
} CNXT_DAVIC_OOBFE_CALLBACKDATA, *PCNXT_DAVIC_OOBFE_CALLBACKDATA;

/* 
 * PCALLBACKFUNC
 * =================================================================================
 * callback definition for data transfer to its client 
 */
typedef void ( *PCALLBACKFUNC_DATA )( PCNXT_DAVIC_OOBFE_CALLBACKDATA pdata );

/* 
 * CNXT_DAVIC_OOBFE_CALLBACKCONTROL, PCNXT_DAVIC_OOBFE_CALLBACKCONTROL
 * =================================================================================
 * data definition for callback to its client 
 */
typedef struct
{
   u_int8   *pControl;
   unsigned char *mBlkPtr;
   u_int32  length;
} CNXT_DAVIC_OOBFE_CALLBACKCONTROL, *PCNXT_DAVIC_OOBFE_CALLBACKCONTROL;

/* 
 * PCALLBACKFUNC
 * =================================================================================
 * callback definition for control message transfer to its client 
 */
typedef void ( *PCALLBACKFUNC_CONTROL )( PCNXT_DAVIC_OOBFE_CALLBACKCONTROL pControl );

/* 
 * CNXT_DAVIC_OOBFE_COMMANDS
 * =================================================================================
 */
typedef enum
{
   OOB_CMD_DISABLE = 0,     /* Temporarily stop transferring data on this channel */
   OOB_CMD_ENABLE,          /* Begin\resume transferring data on this channel */
   OOB_CMD_RESET
} CNXT_DAVIC_OOBFE_COMMANDS;

/* 
 * CNXT_DAVIC_OOBFE_INFO_DATA_CODE
 * =================================================================================
 */
typedef enum
{
  CNXT_DAVIC_OOBFE_BIT_ERROR_RATE = 0,
  CNXT_DAVIC_OOBFE_REED_SOLOMON,
  CNXT_DAVIC_OOBFE_DS_SIGNAL_STRENGTH,
  CNXT_DAVIC_OOBFE_MAC_STATUS,
  CNXT_DAVIC_OOBFE_ACQUISITION_STATUS,
  CNXT_DAVIC_OOBFE_CONNECTION_INFO,
  CNXT_DAVIC_OOBFE_UPSTREAM_DATA
} CNXT_DAVIC_OOBFE_INFO_DATA_CODE;

/* events */
/* 
 * Notification Events
 * =================================================================================
 */
typedef enum
{
   CNXT_DAVIC_OOBFE_EVENT_DATA = 0,
   CNXT_DAVIC_OOBFE_EVENT_CONTROL,
   CNXT_DAVIC_OOBFE_EVENT_RESET,
   CNXT_DAVIC_OOBFE_EVENT_TERMINATED
} CNXT_DAVIC_OOBFE_EVENT;

/** DAVIC  OOB FE driver configuration structure. This is present for future expansion 
 ** and at this time should be set to NULL  
typedef struct _CNXT_DAVIC_OOBFE_CONFIG 
{
} CNXT_DAVIC_OOBFE_CONFIG, *PCNXT_DAVIC_OOBFE_CONFIG;
**/

typedef void* CNXT_DAVIC_OOBFE_CONFIG;    

/* device capability structure */
typedef struct _CNXT_DAVIC_OOBFE_CAPS 
{

   u_int32 uLength;		   /* Length of the caps structure in bytes.  */
                           /* Normally set to sizeof(CNXT_PODHI_CAPS) */
   bool    bExclusive;     /* Set to TRUE on query to indicate only   */
                           /* supports exclusive opening. Set to FALSE*/
                           /* to support shared operation             */
   u_int32 uSlotNum;       /* 0 based numerical index                 */
                           /* associated with this instance.          */
} CNXT_DAVIC_OOBFE_CAPS, *PCNXT_DAVIC_OOBFE_CAPS;

/* device handle */
typedef struct cnxt_oob_inst *CNXT_DAVIC_OOBFE_HANDLE;

/* notification function */
typedef CNXT_DAVIC_OOBFE_STATUS (*CNXT_DAVIC_OOBFE_PFNNOTIFY) ( CNXT_DAVIC_OOBFE_HANDLE   Handle, 
                                                               void                    *pUserData,
                                                               CNXT_DAVIC_OOBFE_EVENT  Event,
                                                               void                    *pData,
                                                               void                    *Tag );

/* 
 * INTERFACEs
 * =================================================================================
 */
/*
+FUNCTION==================================================================+
|
| Name: cnxt_davic_oobfe_init 
|
| Description: Initialization function for cnxt oob module. 
|   This function should be called on startup prior to calling the other API functions.
|
| Inputs: pCfg - Pointer to a DAVIC  OOB FE driver configuration structure. 
|     This is present for future expansion and at this time should be set to NULL
|
| Returns:  CNXT_DAVIC_OOBFE_OK  
|           CNXT_DAVIC_OOBFE_INTERNAL_ERROR 
|           CNXT_DAVIC_OOBFE_ERROR  
|
+==========================================================================+
*/
CNXT_DAVIC_OOBFE_STATUS cnxt_davic_oobfe_init(CNXT_DAVIC_OOBFE_CONFIG *pCfg);

/*
+FUNCTION==================================================================+
|
| Name: cnxt_davic_oobfe_downstream_params_set 
|
| Description: This function will send the OOB CM the relevant downstream 
|   parameters from the 'downstream_params' structure. 
|   This function should be called before calling cnxt_davic_oobfe_channel_open.
|   This function does not close or disable any open channels.
|
| Inputs: CNXT_DAVIC_OOBFE_HANDLE    *pHandle - handle of the device to close
|                                     previously returned from cnxt_davic_oobfe_channel_open()
|         CNXT_DAVIC_OOBFE_DOWNSTREAM_PARAMS downstream_params - downstream parameters to send to the CM
|
| Returns:  CNXT_DAVIC_OOBFE_OK
|           CNXT_DAVIC_OOBFE_BAD_PARAMETER - one or more incorrect parameters.
|           CNXT_DAVIC_OOBFE_BAD_HANDLE - Invalid handle passed.
|           CNXT_DAVIC_OOBFE_NOT_INITIALIZED - Module has not been initialized.
|
+==========================================================================+
*/
CNXT_DAVIC_OOBFE_STATUS cnxt_davic_oobfe_downstream_params_set(
                    CNXT_DAVIC_OOBFE_HANDLE            Handle,
                    CNXT_DAVIC_OOBFE_DOWNSTREAM_PARAMS *downstream_params);

/*
+FUNCTION==================================================================+
|
| Name: cnxt_davic_oobfe_channel_open 
|
| Description: Adds a VCI/VPI filter to the OOB CM.
|   Data transfer on the channel will begin only after the channel 
|   has been enabled.
|
| Inputs: CNXT_DAVIC_OOBFE_HANDLE    *pHandle - handle of the device  
|                                     populated by cnxt_davic_oobfe_channel_open()
|         u_int16 vci - VCI for CM to use in filtering downstream data (12 least significant bits).
|         u_int8 vpi  - VPI.
|
| Output: u_int32* chid - Returns the channel id of the VCI\VPI.
|
| Returns:  CNXT_DAVIC_OOBFE_OK - success (channel opened).
|           CNXT_DAVIC_OOBFE_BAD_HANDLE - Invalid handle passed.
|           CNXT_DAVIC_OOBFE_NOT_INITIALIZED - Module has not been initialized.
|           CNXT_DAVIC_OOBFE_BAD_VCI_VPI - If already opened a channel with this VCI\VPI.
|           CNXT_DAVIC_OOBFE_ERROR - If 'chid' is NULL.
|           CNXT_DAVIC_OOBFE_MAX_VCI_VPI - Channel not opened because the number of open channels is 
|              equal to the maximum VCI/VPI filters supported.
|
+==========================================================================+
*/
CNXT_DAVIC_OOBFE_STATUS cnxt_davic_oobfe_channel_open( 
                  CNXT_DAVIC_OOBFE_HANDLE    Handle,
                  u_int16                    vci, 
                  u_int8                     vpi, 
                  u_int32*                   chid); 

/*
+FUNCTION==================================================================+
|
| Name: cnxt_davic_oobfe_channel_close 
|
| Description: Closes the channel. This will cause the CM to stop filtering downstream 
|   data for the channel's VCI\VPI.
|
| Inputs: CNXT_DAVIC_OOBFE_HANDLE    *pHandle - handle of the device to close
|                                     previously returned from cnxt_davic_oobfe_channel_open()
|         u_int32 chid - channel id to close.
|
| Returns:  CNXT_DAVIC_OOBFE_OK
|           CNXT_DAVIC_OOBFE_BAD_PARAMETER - If the chid is unknown.
|           CNXT_DAVIC_OOBFE_BAD_HANDLE - Invalid handle passed.
|           CNXT_DAVIC_OOBFE_NOT_INITIALIZED - Module has not been initialized.
|
+==========================================================================+
*/
CNXT_DAVIC_OOBFE_STATUS cnxt_davic_oobfe_channel_close( 
                   CNXT_DAVIC_OOBFE_HANDLE    Handle,
                   u_int32                    chid );

/*
+FUNCTION==================================================================+
|
| Name: cnxt_davic_oobfe_all_channels_close 
|
| Description: Closes all open channels. This will cause the CM to stop filtering 
|   downstream data for all currently open channels.
|
| Inputs: CNXT_DAVIC_OOBFE_HANDLE    *pHandle - handle of the device to close
|                                     previously returned from cnxt_davic_oobfe_channel_open()
|
| Returns:  CNXT_DAVIC_OOBFE_OK
|           CNXT_DAVIC_OOBFE_BAD_HANDLE - Invalid handle passed.
|           CNXT_DAVIC_OOBFE_NOT_INITIALIZED - Module has not been initialized.
|
+==========================================================================+
*/
CNXT_DAVIC_OOBFE_STATUS cnxt_davic_oobfe_all_channels_close( CNXT_DAVIC_OOBFE_HANDLE    Handle );

/*
+FUNCTION==================================================================+
|
| Name: cnxt_davic_oobfe_channel_status_get 
|
| Description: Returns the current status of the channel.
|
| Inputs: CNXT_DAVIC_OOBFE_HANDLE    *pHandle - handle of the device to close
|                                     previously returned from cnxt_davic_oobfe_channel_open()
|         chid - channel ID
|
| Output: CNXT_DAVIC_OOBFE_CHANNEL_STATUS - pStatus:
|   CNXT_DAVIC_OOBFE_CHANNEL_OPEN_DISABLED
|   CNXT_DAVIC_OOBFE_CHANNEL_OPEN_ENABLED
|   CNXT_DAVIC_OOBFE_CHANNEL_DOES_NOT_EXIST - channel is closed.
|
| Returns: CNXT_DAVIC_OOBFE_OK - success.
|   CNXT_DAVIC_OOBFE_BAD_HANDLE - Invalid handle passed.
|   CNXT_DAVIC_OOBFE_BAD_PARAMETER - If the channel ID or command is unknown.
|   CNXT_DAVIC_OOBFE_NOT_INITIALIZED - Module has not been initialized.
+==========================================================================+
*/
CNXT_DAVIC_OOBFE_CHANNEL_STATUS cnxt_davic_oobfe_channel_status_get( 
             CNXT_DAVIC_OOBFE_HANDLE          Handle,
			 u_int32                          chid,
             CNXT_DAVIC_OOBFE_CHANNEL_STATUS  *pStatus);
 
/*
+FUNCTION==================================================================+
|
| Name: cnxt_davic_oobfe_channel_control 
|
| Description: Sends a control command to the given channel.
|   Use this function to enable or disable data transfer on a specific channel.
|
| Inputs: CNXT_DAVIC_OOBFE_HANDLE    *pHandle - handle of the device to close
|                                     previously returned from cnxt_davic_oobfe_channel_open()
|         u_int32 chid - channel ID to close.
|         CNXT_DAVIC_OOBFE_COMMANDS command - OOB_CMD_DISABLE or OOB_CMD_ENABLE
|
| Returns:  CNXT_DAVIC_OOBFE_OK
|           CNXT_DAVIC_OOBFE_BAD_PARAMETER - If the chid is unknown.
|           CNXT_DAVIC_OOBFE_BAD_HANDLE - Invalid handle passed.
|           CNXT_DAVIC_OOBFE_NOT_INITIALIZED - Module has not been initialized.
|
+==========================================================================+
*/
CNXT_DAVIC_OOBFE_STATUS cnxt_davic_oobfe_channel_control( 
             CNXT_DAVIC_OOBFE_HANDLE          Handle,
             u_int32                          chid,
             CNXT_DAVIC_OOBFE_COMMANDS        command );

/*
+FUNCTION==================================================================+
|
| Name: cnxt_davic_oobfe_info_control 
|
| Description: Sends a control command for the input info data. Some of the info data
| consumes a lot of resources in order to update its value. For that reason some of the 
| info data can be disabled or enabled.
|
| Inputs: CNXT_DAVIC_OOBFE_INFO_DATA_CODE info_code - The code which represents the requested info data. (see info_data.h)
|    CNXT_DAVIC_OOBFE_COMMANDS command - OOB_CMD_DISABLE or OOB_CMD_ENABLE or OOB_CMD_RESET
|    CNXT_DAVIC_OOBFE_HANDLE    *pHandle - handle of the device to close
|                                     previously returned from cnxt_davic_oobfe_channel_open()
|
| Returns:  CNXT_DAVIC_OOBFE_OK
|           CNXT_DAVIC_OOBFE_BAD_PARAMETER - If the chid is unknown.
|           CNXT_DAVIC_OOBFE_BAD_HANDLE - Invalid handle passed.
|           CNXT_DAVIC_OOBFE_NOT_INITIALIZED - Module has not been initialized.
|           CNXT_DAVIC_OOBFE_ERROR - if the oob api is not initialiazed and/or there are not 
|                  enough resources	to perform the command.                    
|
+==========================================================================+
*/
CNXT_DAVIC_OOBFE_STATUS cnxt_davic_oobfe_info_control(
             CNXT_DAVIC_OOBFE_HANDLE          Handle,
             CNXT_DAVIC_OOBFE_INFO_DATA_CODE  info_code,
             CNXT_DAVIC_OOBFE_COMMANDS        command);

/*
+FUNCTION==================================================================+
|
| Name: cnxt_davic_oobfe_info_read 
|
| Description: Requests the value of the requested info data. The function is asynchoronous because
| the response to the that function will return in the form of a control message. If the
| funtion returns with a success status then it is guaranteed that a control message with 
| the requested info data will return.
|
| Inputs: CNXT_DAVIC_OOBFE_INFO_DATA_CODE info_code - The code which represent the requested info data. (see info_data.h)
|         CNXT_DAVIC_OOBFE_HANDLE    *pHandle - handle of the device to close
|                                     previously returned from cnxt_davic_oobfe_channel_open()
|
| Returns: CNXT_DAVIC_OOBFE_OK - success.
|   CNXT_DAVIC_OOBFE_BAD_HANDLE - Invalid handle passed.
|   CNXT_DAVIC_OOBFE_BAD_PARAMETER - If the channel ID or command is unknown.
|   CNXT_DAVIC_OOBFE_NOT_INITIALIZED - Module has not been initialized.
|   CNXT_DAVIC_OOBFE_ERROR - if the oob api is not initialiazed and/or there are not enough resources
|   to perform the command.
|
+==========================================================================+
*/
CNXT_DAVIC_OOBFE_STATUS cnxt_davic_oobfe_info_read(
             CNXT_DAVIC_OOBFE_HANDLE          Handle,
             CNXT_DAVIC_OOBFE_INFO_DATA_CODE  info_code);

/*
+FUNCTION==================================================================+

| Name: cnxt_davic_oobfe_send_upstream
| 
| Note: This function should be called only when CM upstream is enabled.
|
| Description: Send data upstream.
|    Will send length bytes of data pointed to by pData to the INA. 
|    The function is blocking and will not return until the data is sent to the CM.  
|    Special consideration must be given to: overhead, alignment and non-cacheable memory.
|    Overhead:
|       There is some overhead associated with each data transfer (due to DMA and the low-level driver implementation). 
|       The caller must allow for all possible overhead. Maximum 7 bytes, as follows: 
|       Allow write access for 7 bytes after the end of the data: [pData + length, pData + length + 6].
|    Alignment:
|       The first byte of the data must be at a 32-bit aligned address.
|    Non-Cacheable Memory:
|       This function uses DMA. The caller must allocate the data (plus overhead) from non-cacheable memory.
| Input:
|    pData - pointer to the data to send upstream.
|    Length - number of bytes to send upstream.
|    data_type - defines the data as MAC Management Message (MMM) or other data. 
|
| Returns: CNXT_DAVIC_OOBFE_OK - success.
|   CNXT_DAVIC_OOBFE_BAD_HANDLE - Invalid handle passed.
|   CNXT_DAVIC_OOBFE_BAD_PARAMETER - If the channel ID or command is unknown.
|   CNXT_DAVIC_OOBFE_NOT_INITIALIZED - Module has not been initialized.
|
+==========================================================================+
*/
CNXT_DAVIC_OOBFE_STATUS cnxt_davic_oobfe_send_upstream(
             CNXT_DAVIC_OOBFE_HANDLE             Handle,
             u_int8                              *pData, 
             u_int32                             length, 
             CNXT_DAVIC_OOBFE_UPSTREAM_DATA_TYPE data_type);

/*
+FUNCTION==================================================================+
|
| Name: cnxt_davic_oobfe_term 
|
| Description: This function terminates all operation of cnxt oob module. 
|   All associated resources are released. 
|
| Inputs: void
|
| Returns:  CNXT_DAVIC_OOBFE_OK
|           CNXT_DAVIC_OOBFE_NOT_INITIALIZED
|
+==========================================================================+
*/
CNXT_DAVIC_OOBFE_STATUS cnxt_davic_oobfe_term(void );

/*
+FUNCTION==================================================================+
|
| Name: cnxt_davic_oobfe_get_units 
|
| Description: This function returns the number of enumerated devices present. 
|
| Output: u_int32 *puCount - pointer to a u_int32 to return the number of
|                            of device instances
|
| Returns:  CNXT_DAVIC_OOBFE_OK 
|
+==========================================================================+
*/
CNXT_DAVIC_OOBFE_STATUS cnxt_davic_oobfe_get_units ( u_int32 *puCount );

/*
+FUNCTION==================================================================+
|
| Name: cnxt_davic_oobfe_get_unit_caps 
|
| Description: This function returns the module specific capabilities for 
|              the device or configuration number passed. 
|
| Output: u_int32 *puCount - pointer to a u_int32 to return the number of
|                            of device instances
|
| Returns:  CNXT_DAVIC_OOBFE_OK         if success
|           CNXT_DAVIC_OOBFE_BAD_UNIT	if the passed in wrong unit number 
|			 
+==========================================================================+
*/
CNXT_DAVIC_OOBFE_STATUS cnxt_davic_oobfe_get_unit_caps ( u_int32               uUnitNumber, 
                                                         CNXT_DAVIC_OOBFE_CAPS *pCaps );

/*
+FUNCTION==================================================================+
|
| Name: cnxt_davic_oobfe_open 
|
| Description: This function obtain a handle to a particular instance 
|              of the device. The handle is used for all subsequent calls
|              to the OOB mosule. 
|
| Output: CNXT_DAVIC_OOBFE_HANDLE    *pHandle - a handle to a particular instance
|
| Returns:  CNXT_DAVIC_OOBFE_OK
|           CNXT_DAVIC_OOBFE_INTERNAL_ERROR - Couldn't get the sem.
|           CNXT_DAVIC_OOBFE_NOT_INITIALIZED - Module has not been initialized.
|			 
+==========================================================================+
*/
CNXT_DAVIC_OOBFE_STATUS cnxt_davic_oobfe_open ( CNXT_DAVIC_OOBFE_HANDLE    *Handle,
                                                CNXT_DAVIC_OOBFE_CAPS      *pCaps,
                                                CNXT_DAVIC_OOBFE_PFNNOTIFY pNotifyFn,
                                                void                       *pUserData );

/*
+FUNCTION==================================================================+
|
| Name: cnxt_davic_oobfe_close 
|
| Description: This function releases the use of the module and render the
|              passed handle invalid. 
|
| Input: CNXT_DAVIC_OOBFE_HANDLE Handle - handle of the device to close
|
| Returns:  CNXT_DAVIC_OOBFE_OK
|           CNXT_DAVIC_OOBFE_INTERNAL_ERROR - Couldn't get the sem.
|           CNXT_DAVIC_OOBFE_BAD_HANDLE - Invalid handle.
|           CNXT_DAVIC_OOBFE_NOT_INITIALIZED - Module has not been initialized.
|			 
+==========================================================================+
*/
CNXT_DAVIC_OOBFE_STATUS cnxt_davic_oobfe_close ( CNXT_DAVIC_OOBFE_HANDLE Handle );

/*
+FUNCTION==================================================================+
|
| Name: cnxt_davic_oobfe_buf_release 
| Description:This function is used to release a buffer of data passed to 
|             the notify function following reception of certain notifications
|
| Inputs: CNXT_DAVIC_OOBFE_HANDLE    *pHandle - handle of the device to close
|                                     previously returned from cnxt_davic_oobfe_channel_open()
|         void* pBuf - Buffer pointer to release. This pointer is included as 
|                      part of the pData input structure in the Notify function.
| Returns:  CNXT_DAVIC_OOBFE_OK
|           CNXT_DAVIC_OOBFE_BAD_HANDLE - Invalid handle passed.
|           CNXT_DAVIC_OOBFE_NOT_INITIALIZED - Module has not been initialized.
|
+==========================================================================+
*/
CNXT_DAVIC_OOBFE_STATUS cnxt_davic_oobfe_buf_release ( CNXT_DAVIC_OOBFE_HANDLE Handle,
                                                       void* pBuf );

#endif /* _OOB_API_H_ */

/*****************************************************************************
 * Modifications:
 * $Log: 
 *  6    mpeg      1.5         7/29/03 3:45:08 PM     Yong Lu         SCR(s): 
 *        7066 
 *        correct names of info data types to match API
 *        
 *  5    mpeg      1.4         7/11/03 2:36:00 PM     Sahil Bansal    SCR(s): 
 *        6926 
 *        1. Added CNXT_DAVIC_OOBFE_MAX_CHANNELS_OPEN #define.
 *        
 *  4    mpeg      1.3         5/15/03 8:28:40 PM     Yong Lu         SCR(s): 
 *        6386 6387 
 *        added DS_SIGNAL_STRENGTH to CNXT_DAVIC_OOBFE_INFO_DATA_CODE 
 *        
 *  3    mpeg      1.2         5/14/03 9:51:18 PM     Yong Lu         SCR(s): 
 *        6355 
 *        clean up code
 *        
 *  2    mpeg      1.1         5/2/03 5:00:28 PM      Sahil Bansal    SCR(s): 
 *        6157 
 *        Add file header/footer
 *        
 *  1    mpeg      1.0         5/1/03 5:24:54 AM      Yong Lu         
 * $
 * 
 *    Rev 1.5   29 Jul 2003 14:45:08   LUY3
 * SCR(s): 7066 
 * correct names of info data types to match API
 * 
 *    Rev 1.4   11 Jul 2003 13:36:00   bansals
 * SCR(s): 6926 
 * 1. Added CNXT_DAVIC_OOBFE_MAX_CHANNELS_OPEN #define.
 * 
 *    Rev 1.3   15 May 2003 19:28:40   LUY3
 * SCR(s): 6386 6387 
 * added DS_SIGNAL_STRENGTH to CNXT_DAVIC_OOBFE_INFO_DATA_CODE 
 * 
 *    Rev 1.2   14 May 2003 20:51:18   LUY3
 * SCR(s): 6355 
 * clean up code
 * 
 *    Rev 1.1   02 May 2003 16:00:28   bansals
 * SCR(s): 6157 
 * Add file header/footer
 *
 ****************************************************************************/

