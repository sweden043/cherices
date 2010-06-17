/******************************************************************************/
/*                   CONEXANT PROPRIETARY AND CONFIDENTIAL                    */
/*                       SOFTWARE FILE/MODULE HEADER                          */
/*                   Conexant Systems Inc. (c) 1998 - 2004                    */
/*                               Austin, TX                                   */
/*                            All Rights Reserved                             */
/******************************************************************************/
/*
 * Filename:       stbmodem.h
 *
 *
 * Description:    Defines the Conexant Modem API
 *
 *
 * Author:         Joe Kroesche
 *
 ******************************************************************************/
/* $Id: stbmodem.h,v 1.9, 2004-05-04 14:58:08Z, Bobby Bradford$
 ******************************************************************************/

/******************************************************************************/
/* SUMMARY:                                                                   */
/*                                                                            */
/* This file contains the defintions needed by a Host Application to access   */
/* and use the Conexant Modems.  This is the Conexant Generic Modem API, and  */
/* is intended to work with all of the set top box modems                     */
/*                                                                            */
/******************************************************************************/


#ifndef _STBMODEM_H_
#define _STBMODEM_H_

/********************************/
/* Symbol and Macro definitions */
/********************************/
/* 
 * Modem Callback Events/Status:
 *
 * These defines are used by the modem driver to notifiy the host functions
 * of event(s) that have been generated in the modem.
 *
 * Note:  These events are bit-mapped, and one or more events may be indicated
 * in a single event callback.
 *
 * Note:  Not every Conexant Modem will implement every event
 */
#define  CNXT_MODEM_EVT_RXCHAR     0x00000001  /* Any Character received */
#define  CNXT_MODEM_EVT_RXFLAG     0x00000002  /* Received certain character */
#define  CNXT_MODEM_EVT_TXEMPTY    0x00000004  /* Transmit Queue Empty */
#define  CNXT_MODEM_EVT_CTS        0x00000008  /* CTS changed state */
#define  CNXT_MODEM_EVT_DSR        0x00000010  /* DSR changed state */
#define  CNXT_MODEM_EVT_RLSD       0x00000020  /* RLSD changed state */
#define  CNXT_MODEM_EVT_BREAK      0x00000040  /* BREAK received */
#define  CNXT_MODEM_EVT_ERR        0x00000080  /* Line status error occurred */
#define  CNXT_MODEM_EVT_RING       0x00000100  /* Ring signal detected */
#define  CNXT_MODEM_EVT_CTSS       0x00000400  /* CTS state */
#define  CNXT_MODEM_EVT_DSRS       0x00000800  /* DSR state */
#define  CNXT_MODEM_EVT_RLSDS      0x00001000  /* RLSD state */
#define  CNXT_MODEM_EVT_PARITY_ERR 0x00002000  /* Parity Error occured */
#define  CNXT_MODEM_EVT_TXCHAR     0x00004000  /* Any character transmitted */
#define  CNXT_MODEM_EVT_RXOVRN     0x00030000  /* Rx buffer overrun detected */
#define  CNXT_MODEM_EVT_CID        0x00100000  /* CALLER ID Detected */


/* 
 * Modem IOCTL Codes/Values:
 */
typedef enum
{
   CNXT_MODEM_IOCTL_NONE = 0,       /* Placeholder ... not used */
   CNXT_MODEM_IOCTL_DTR,            /* Set/Clear DTR (e.g. DTR Disconnect) */
   CNXT_MODEM_IOCTL_RTS,            /* Set/Clear RTS (flow control) */
   CNXT_MODEM_IOCTL_RX_AVAIL,       /* Query available data in RX buffer */
   CNXT_MODEM_IOCTL_TX_FREE,        /* Query available space in TX buffer */
   CNXT_MODEM_IOCTL_SET_CMDLEN_MAX, /* Set modem command string length */
   CNXT_MODEM_IOCTL_GET_CMDLEN_MAX  /* Query modem command string length */
} CNXT_MODEM_IOCTL_CODE;


#define  CNXT_DTR_SET        (1)    /* Set/Enable DTR */
#define  CNXT_DTR_CLEAR      (0)    /* Clear/Disable DTR */
#define  CNXT_RTS_SET        (1)    /* Set/Enable RTS */
#define  CNXT_RTS_CLEAR      (0)    /* Clear/Disable RTS */

/*
 * Modem API Function Return Codes
 */
typedef enum
{
   CNXT_MODEM_OK = 0,               /* Function completed with NO errors */
   CNXT_MODEM_ERROR,                /* Unspecified Error Condition */
   CNXT_MODEM_ERROR_CREATED,        /* Modem Already Created */
   CNXT_MODEM_ERROR_NOT_CREATED,    /* Modem Not Yet Created */
   CNXT_MODEM_ERROR_OPENED,         /* Modem Already Opened */
   CNXT_MODEM_ERROR_NOT_OPENED,     /* Modem Not Yet Opened */
   CNXT_MODEM_ERROR_NOT_CLOSED,     /* Modem Not Yet Closed */
   CNXT_MODEM_ERROR_MEMORY,         /* Modem could not allocated memory */
   CNXT_MODEM_ERROR_UART,           /* UART Driver returned an error code */
   CNXT_MODEM_ERROR_INVALID_IOCTL,  /* Undefined UART specified */
   CNXT_MODEM_ERROR_PARAMETER       /* Invalid parameter (e.g. NULL pointer) */
} CNXT_MODEM_STATUS;

/**************/
/* Data Types */
/**************/

/*****************************/
/* API / Function prototypes */
/*****************************/

/******************************************************************************/
/*  FUNCTION:    cnxt_modem_create                                            */
/*                                                                            */
/*  DESCRIPTION:                                                              */
/*    Creates and installs the modem device.  Memory is allocated and modem   */
/*    software objects are created and initialized.                           */
/*                                                                            */
/*  INPUT PARAMETERS:                                                         */
/*    None                                                                    */
/*                                                                            */
/*  OUTPUT PARAMETERS:                                                        */
/*    None                                                                    */
/*                                                                            */
/*  RETURN VALUES:                                                            */
/*    CNXT_MODEM_OK                 - Success.                                */
/*    CNXT_MODEM_ERROR_CREATED      - Modem already created                   */
/*    CNXT_MODEM_ERROR_MEMORY       - Failed to allocated needed memory       */
/*                                                                            */
/*  NOTES:                                                                    */
/*    This function should be called only once.  Successive calls after the   */
/*    first will return an error.  If sufficient memory cannot be allocated   */
/*    for the modem, then the function will return an error.                  */
/*                                                                            */
/*  CONTEXT:                                                                  */
/*    This function may only be called from thread context.                   */
/*                                                                            */
/******************************************************************************/
CNXT_MODEM_STATUS cnxt_modem_create( void );


/******************************************************************************/
/*  FUNCTION:    cnxt_modem_destroy                                           */
/*                                                                            */
/*  DESCRIPTION:                                                              */
/*    Reverses the effect of cnxt_modem_create(), destroying the modem        */
/*    software objects and freeing allocated memory.                          */
/*                                                                            */
/*  INPUT PARAMETERS:                                                         */
/*    None                                                                    */
/*                                                                            */
/*  OUTPUT PARAMETERS:                                                        */
/*    None                                                                    */
/*                                                                            */
/*  RETURN VALUES:                                                            */
/*    CNXT_MODEM_OK                 - Success.                                */
/*    CNXT_MODEM_ERROR_NOT_CREATED  - Modem not yet created (or already       */
/*                                    destroyed                               */
/*    CNXT_MODEM_ERROR_NOT_CLOSED   - Modem still open (not closed yet)       */
/*                                                                            */
/*  NOTES:                                                                    */
/*    This function will return an error if there was not a matching prior    */
/*    call to cnxt_modem_create() or if there is an error freeing memory.     */
/*                                                                            */
/*  CONTEXT:                                                                  */
/*    This function may only be called from thread context.                   */
/*                                                                            */
/******************************************************************************/
CNXT_MODEM_STATUS cnxt_modem_destroy( void );


/******************************************************************************/
/*  FUNCTION:    cnxt_modem_open                                              */
/*                                                                            */
/*  DESCRIPTION:                                                              */
/*    Initializes the modem hardware and opens a port for data transfer. An   */
/*    optional callback function can be provided by the caller for modem      */
/*    event notification.                                                     */
/*                                                                            */
/*  INPUT PARAMETERS:                                                         */
/*    pfnCallback - Pointer to a function provided by the modem user, which   */
/*                  will be called by the modem when certain events occur;    */
/*                  can be NULL                                               */
/*                                                                            */
/*  OUTPUT PARAMETERS:                                                        */
/*    None                                                                    */
/*                                                                            */
/*  RETURN VALUES:                                                            */
/*    CNXT_MODEM_OK                 - Success.                                */
/*    CNXT_MODEM_ERROR_NOT_CREATED  - Modem not yet created                   */
/*    CNXT_MODEM_ERROR_OPENED       - Modem already opened                    */
/*    CNXT_MODEM_ERROR_UART         - UART Driver failed open/config          */
/*    CNXT_MODEM_ERROR_PARAMETER    - Invalid parameter passed (e.g. NULL     */
/*                                    Pointer)                                */
/*                                                                            */
/*  NOTES:                                                                    */
/*    This function should be called only once per session. Only one port     */
/*    instance is supported.  If the caller supplies a pointer to a callback  */
/*    function, then the caller can be notified of modem events. Here is the  */
/*    prototype for the callback function:                                    */
/*                                                                            */
/*       void ModemEventHandler( u_int32 evflags )                            */
/*                                                                            */
/*    The definitions and meaning of the event flags can be found in the      */
/*    header file, "stbmodem.h".                                              */
/*                                                                            */
/*    The parameter evflags can have more than one flag set when the callback */
/*    function is called.                                                     */
/*                                                                            */
/*    Important Note:   The purpose of the callback function is for           */
/*    notification only, not for doing significant work. The callback         */
/*    function should simply set a flag or schedule some task as a result of  */
/*    a modem event, and the actual work associated with the event should be  */
/*    performed later in a different thread.                                  */
/*                                                                            */
/*  CONTEXT:                                                                  */
/*    This function may only be called from thread context.                   */
/*                                                                            */
/******************************************************************************/
CNXT_MODEM_STATUS cnxt_modem_open( void (*pfnCallback)(u_int32) );


/******************************************************************************/
/*  FUNCTION:    cnxt_modem_close                                             */
/*                                                                            */
/*  DESCRIPTION:                                                              */
/*    Closes the port previously opened by cnxt_modem_open(), disables the    */
/*    modem hardware and uninstalls the event callback function.              */
/*                                                                            */
/*  INPUT PARAMETERS:                                                         */
/*    None                                                                    */
/*                                                                            */
/*  OUTPUT PARAMETERS:                                                        */
/*    None                                                                    */
/*                                                                            */
/*  RETURN VALUES:                                                            */
/*    CNXT_MODEM_OK                 - Success.                                */
/*    CNXT_MODEM_ERROR_NOT_CREATED  - Modem not yet created                   */
/*    CNXT_MODEM_ERROR_NOT_OPENED   - Modem not yet opened (can't close)      */
/*    CNXT_MODEM_ERROR_UART         - UART Driver failed close                */
/*                                                                            */
/*  NOTES:                                                                    */
/*    None                                                                    */
/*                                                                            */
/*  CONTEXT:                                                                  */
/*    This function may only be called from thread context.                   */
/*                                                                            */
/******************************************************************************/
CNXT_MODEM_STATUS cnxt_modem_close( void );


/******************************************************************************/
/*  FUNCTION:    cnxt_modem_read                                              */
/*                                                                            */
/*  DESCRIPTION:                                                              */
/*    Reads bytes from the modem port.                                        */
/*                                                                            */
/*  INPUT PARAMETERS:                                                         */
/*    pData    - Pointer to caller supplied storage for data read from the    */
/*               modem; must be at least as big as the value pointed to by    */
/*               pCount                                                       */
/*    pCount   - Pointer to the read count value; on entry this indicates the */
/*               number of characters that should be read from the modem and  */
/*               stored in buf; on exit this indicates the actual number of   */
/*               bytes that were read from the modem and stored in buf. The   */
/*               output value will be 0 if the modem receive buffer is empty, */
/*               or less than the input value if there is less data than      */
/*               requested in the modem receive buffer.                       */
/*                                                                            */
/*  OUTPUT PARAMETERS:                                                        */
/*    pCount   - See not on input                                             */
/*                                                                            */
/*  RETURN VALUES:                                                            */
/*    CNXT_MODEM_OK                 - Success.                                */
/*    CNXT_MODEM_ERROR_NOT_CREATED  - Modem not yet created                   */
/*    CNXT_MODEM_ERROR_NOT_OPENED   - Modem not yet opened (can't close)      */
/*    CNXT_MODEM_ERROR_PARAMETER    - Invalid parameter passed (e.g. NULL     */
/*                                    Pointer)                                */
/*                                                                            */
/*  NOTES:                                                                    */
/*    None                                                                    */
/*                                                                            */
/*  CONTEXT:                                                                  */
/*    This function may only be called from thread context.                   */
/*                                                                            */
/******************************************************************************/
CNXT_MODEM_STATUS cnxt_modem_read( int8 *pData, u_int32 *pCount );


/******************************************************************************/
/*  FUNCTION:    cnxt_modem_write                                             */
/*                                                                            */
/*  DESCRIPTION:                                                              */
/*    Writes bytes to the modem port.                                         */
/*                                                                            */
/*  INPUT PARAMETERS:                                                         */
/*    pData    - Pointer to buffer holding the data to be written to the      */
/*               modem.                                                       */
/*    pCount   - Pointer to the write value; on entry this indicates the      */
/*               number of bytes that should be read from buf and written to  */
/*               the modem transmit buffer; on exit this indicates the actual */
/*               number of bytes that were written from buf to the modem      */
/*               transmit buffer. The output value can be 0 or less than the  */
/*               input value if the modem transmit buffer fills up.           */
/*                                                                            */
/*  OUTPUT PARAMETERS:                                                        */
/*    pCount   - See not in input                                             */
/*    None                                                                    */
/*                                                                            */
/*  RETURN VALUES:                                                            */
/*    CNXT_MODEM_OK                 - Success.                                */
/*    CNXT_MODEM_ERROR_NOT_CREATED  - Modem not yet created                   */
/*    CNXT_MODEM_ERROR_NOT_OPENED   - Modem not yet opened (can't close)      */
/*    CNXT_MODEM_ERROR_PARAMETER    - Invalid parameter passed (e.g. NULL     */
/*                                    Pointer)                                */
/*                                                                            */
/*  NOTES:                                                                    */
/*    None                                                                    */
/*                                                                            */
/*  CONTEXT:                                                                  */
/*    This function may only be called from thread context.                   */
/*                                                                            */
/******************************************************************************/
CNXT_MODEM_STATUS cnxt_modem_write( const int8 *pData, u_int32 *pCount );


/******************************************************************************/
/*  FUNCTION:    cnxt_modem_ioctl                                             */
/*                                                                            */
/*  DESCRIPTION:                                                              */
/*    Perform modem control functions.                                        */
/*                                                                            */
/*  INPUT PARAMETERS:                                                         */
/*    eIoctlCode  - Ioctl function code for the modem (see notes)             */
/*    pIoctlData  - On entry this points to the control value for the ioctl   */
/*                  function value; on exit the value pointed will be updated */
/*                  with the result of the function code, if any.             */
/*                                                                            */
/*  OUTPUT PARAMETERS:                                                        */
/*    out_parm1   - description                                               */
/*    None                                                                    */
/*                                                                            */
/*  RETURN VALUES:                                                            */
/*    CNXT_MODEM_OK                 - Success.                                */
/*    CNXT_MODEM_ERROR_NOT_CREATED  - Modem not yet created                   */
/*    CNXT_MODEM_ERROR_NOT_OPENED   - Modem not yet opened (can't close)      */
/*    CNXT_MODEM_ERROR_PARAMETER    - Invalid parameter passed (e.g. NULL     */
/*                                    Pointer)                                */
/*                                                                            */
/*  NOTES:                                                                    */
/*    Refer to header file, "STMODEM.H" for most current list of modem IOCTL  */
/*    codes.  Also note that not every code defined will be implemented by    */
/*    every Conexant modem.                                                   */
/*                                                                            */
/*  CONTEXT:                                                                  */
/*    This function may be called from any context                            */
/*                                                                            */
/******************************************************************************/
CNXT_MODEM_STATUS cnxt_modem_ioctl( const CNXT_MODEM_IOCTL_CODE eIoctlCode,
      int32 *pIoctlData );

#endif /* _STBMODEM_H_ */

/*******************************************************************************
 * Modifications:
 * $Log: 
 *  10   mpeg      1.9         5/4/04 9:58:08 AM      Bobby Bradford  CR(s) 
 *        9051 9052 : Update of modem API to eliminate naming conflicts with 
 *        middleware modem api defintions, also updated to use updated coding 
 *        standards, etc.
 *  9    mpeg      1.8         2/27/03 3:15:34 PM     Angela Swartz   SCR(s) 
 *        5609 :
 *        added MODEM_IOCTL_SET/GET_CMDLEN_MAX into MODEM_IOCTL_CODE allow user
 *         to set/get the max length of at command
 *        
 *  8    mpeg      1.7         11/18/02 10:02:34 AM   Bobby Bradford  SCR(s) 
 *        4894 :
 *        Added new event code (see tracker 4906) for caller id notification
 *        
 *  7    mpeg      1.6         8/24/01 6:10:04 PM     Joe Kroesche    SCR(s) 
 *        2553 :
 *        added modem ioctl codes for new ioctl functions to read rx data 
 *        available
 *        in buffer, and tx buffer free space
 *        
 *  6    mpeg      1.5         2/26/01 9:38:24 AM     Angela          DCS#1060 
 *        - added decompression functions and return code type
 *        
 *  5    mpeg      1.4         2/1/01 2:46:32 PM      Angela          added 
 *        country codes
 *        
 *  4    mpeg      1.3         1/5/01 11:48:40 AM     Angela          modified 
 *        modem callback events bit masks value
 *        
 *  3    mpeg      1.2         12/5/00 4:59:12 PM     Joe Kroesche    modified 
 *        read and write signatures slightly
 *        
 *  2    mpeg      1.1         12/4/00 3:52:24 PM     Joe Kroesche    added 
 *        dtr/rts defines and additional ioctl code (rts)
 *        
 *  1    mpeg      1.0         11/30/00 2:37:44 PM    Joe Kroesche    
 * $
 *
 ******************************************************************************/

