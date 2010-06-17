/****************************************************************************/
/*                   CONEXANT PROPRIETARY AND CONFIDENTIAL                  */
/*                       SOFTWARE FILE/MODULE HEADER                        */
/*                 Conexant Systems Inc. (c) 1998, 1999, 2000               */
/*                               Austin, TX                                 */
/*                            All Rights Reserved                           */
/****************************************************************************/
/*
 * Filename:      llsermemory.c
 *                  
 * Description:   Low level serial driver for memory buffer trace output
 *
 * Author:        kroesche
 *
 ****************************************************************************/
/* $Header: llsermemory.c, 3, 2/13/03 12:01:48 PM, Matt Korte$
 ****************************************************************************/

/* includes */
#include "stbcfg.h"
#include "kal.h"
#include "retcodes.h"
#include "llserial.h"

/* defines */
/* typedefs */

/* locals */
static PLLSERCALLBACKS cbfn = NULL;
static bool isrInstalled = FALSE;
static bool opened = FALSE;
static tick_id_t tick_handle = 0;
static bool ints_enabled = FALSE;

/* globals */
/*
 * this structure holds all the relevant data about the memory buffering
 * device.  This is more info than needed to just implement the device, but
 * this will be helpful when using the debugger.  The idea is to just
 * examine this data structure with the debugger and then you can see
 * everything you want to know about the buffer
 */
typedef struct _LLSERMEM_DATA
{
   unsigned int buf_size;
   unsigned int buf_count;
   unsigned char* buf_start;
   unsigned char* buf_end;
   unsigned char* buf_at;
   unsigned int buf_index;
   unsigned char buf_lastchar;
} LLSERMEM_DATA;

LLSERMEM_DATA llsermem_data;

/* If TRACE_PORT is not TRACE_MEMORY, set a tiny, dummy buffer, else create */
/* a buffer of the required size.                                           */
#if (TRACE_PORT == TRACE_MEMORY)
#define ACTUAL_SERMEM_BUFFER_SIZE (LLSERMEM_BUFSIZE * 1024)
#else
#define ACTUAL_SERMEM_BUFFER_SIZE (4)
#endif
unsigned char llsermem_buffer[ACTUAL_SERMEM_BUFFER_SIZE];

/* forward declarations */
static LLSERINTERFACE interfaceTable;
static void llsermemtx_isr( tick_id_t tmr, void* userdata );

/*
 * PLLSERINTERFACE llseropen_memory( PSER_CB_FNS pcbfns )
 *
 * parameters:
 *    pcbfns - pointer to callback functions struct, provided by client
 *             these are the functions that are called back whenever
 *             a serial event occurs
 * returns:
 *    An interface structure containing pointers to the low level
 *    serial driver functions.
 *
 * Saves the client callback structure.  If this is not null, then
 * the ISR is installed.  Just call it with a NULL parameter if the
 * the client does not want to use interrupt features.  An interface
 * is returned to the client that should be used for all further calls
 * to the low level serial driver.
 */
PLLSERINTERFACE llseropen_memory( PLLSERCALLBACKS pcb )
{
   bool crit;

   /* check to make sure this device not already opened */
   crit = critical_section_begin();
   if( opened )
   {
      critical_section_end( crit );
      return( NULL );
   }
   opened = TRUE;
   critical_section_end( crit );

   if( pcb && !isrInstalled )
   {
      /* dummy the "tx interrupt" by using a tick timer callback */
      tick_handle = tick_create( llsermemtx_isr, NULL, NULL );
      if( 0 == tick_handle )
      {
          opened = FALSE;
          return( 0 );
      }
      if( RC_OK != tick_set( tick_handle, 1, TRUE ) )
      {
          opened = FALSE;
          return( 0 );
      }
      isrInstalled = TRUE;

   }
   /* initialize the data structure */
   llsermem_data.buf_size = ACTUAL_SERMEM_BUFFER_SIZE;
   llsermem_data.buf_count = 0;
   llsermem_data.buf_start = llsermem_buffer;
   llsermem_data.buf_end = &llsermem_buffer[ACTUAL_SERMEM_BUFFER_SIZE-1];
   llsermem_data.buf_at = &llsermem_buffer[0];
   llsermem_data.buf_index = 0;
   llsermem_data.buf_lastchar = 0;

   ints_enabled = FALSE;
   cbfn = pcb;
   return( &interfaceTable );
}

/*
 * static int serclose( void )
 *
 * parameters:
 *    none
 *
 * returns:
 *    non-zero if successful, zero if error detected
 *
 * Cleans up the serial port.  It turns off the interrupts and marks the
 * device as available so that it can be opened by someone else.
 */
static int serclose( void  )
{
   /* turn off interrupts at the pic */
   if( isrInstalled )
   {
       tick_destroy( tick_handle );
   }

   ints_enabled = FALSE;
   cbfn = NULL;
   opened = FALSE;
   return( 1 );
}

/*
 * static int config( unsigned int rate, unsigned int bits,
 *                         unsigned int stops, bool parity, bool even )
 * parameters:
 *    rate - the line rate in bits per second (2 - 115200)
 *    bits - the number of data bits
 *    stops - the number of stop bits
 *    parity - enables parity, non-zero=parity enabled
 *    even - sets even parity if parity enabled, non-zero=even, else odd
 *
 * returns:
 *    non-zero if successful, zero if any errors are detected
 */
static int config( unsigned int rate, unsigned int bits,
                        unsigned int stops, bool parity, bool even )
{
   /*
    * for the mem buffer serial, no config is needed.  However we
    * need a stub here because the llserial interface is expecting
    * a config function.
    */
  return( 1 );
}

/*
 * static void enable( bool ena )
 *
 * parameters:
 *    ena - tru to enable the interrupts, false to disable
 *
 * returns:
 *    none
 *
 * Enables or disables the serial interrupts.
 */
static void enable( bool ena )
{
    ints_enabled = ena;
}

/*
 * static void txstart( void )
 *
 * parameters:
 *    none
 *
 * returns:
 *    none
 *
 * Signals the serial driver that the client has tx data available.
 * The tick timer will be armed so that it will cause a callback
 * for the simulated memory buffer serial device.
 */
static void txstart( void )
{
   if( isrInstalled )
      tick_start( tick_handle );
}

/*
 * static void setFlowReady( bool state )
 *
 * parameters:
 *    state - the state of the ready signal (dtr or dsr, depends on
 *             whether this is dte or dce)
 *
 * returns:
 *    none
 *
 * Sets the state of the flow control "ready" signal.  This is either
 * DSR or DTR, whichever is an output for this device (depends on DCE/DTE).
 */
static void setFlowReady( bool state )
{
   /* mem device has no flow control so this is just stub */
}

/*
 * static bool getFlowReady( void )
 *
 * parameters:
 *    none
 *
 * returns:
 *    the state of the flow control "ready" signal
 *
 * Gets the state of the flow control "ready" signal.  This is either
 * DSR or DTR, whichever is an input for this device (depends on DCE/DTE).
 * This function should not be used at the same as the serial is enabled
 * because reading the status register can interfere with proper operation
 * of the ISR.
 */
static bool getFlowReady( void )
{
   return( 1 );   /* always ready for mem device */
}

/*
 * static void setFlowControl( bool state )
 *
 * parameters:
 *    state - the state of the hardware flow control signal (rts or cts,
 *             depends on whether this is dte or dce)
 *
 * returns:
 *    none
 *
 * Sets the state of the flow control hardware signal.  This is either
 * RTS or CTS, whichever is an output for this device (depends on DCE/DTE).
 */
static void setFlowControl( bool state )
{
   /* mem device has no flow control so this is just stub */
}

/*
 * static bool getFlowControl( void )
 *
 * parameters:
 *    none
 *
 * returns:
 *    the state of the flow control hardware signal
 *
 * Gets the state of the flow control hardware signal.  This is either
 * RTS or CTS, whichever is an input for this device (depends on DCE/DTE).
 * This function should not be used at the same as the serial is enabled
 * because reading the status register can interfere with proper operation
 * of the ISR.
 */
static bool getFlowControl( void )
{
   return( 1 );   /* always ready for mem device */
}

/*
 * static int getchar( bool wait )
 *
 * parameters:
 *    wait - indicates if the function should wait for an available
 *          character or return right away
 *
 * returns:
 *    a serial character, or -1 if none available and wait not specified
 *
 * Attempts to read a character from the serial port in polling fashion.
 * If wait is specified, then it will continue to poll until a character
 * is available.  If wait is not specified, then it will either return
 * a character if available, or return -1 if none available.
 * NOTE: this is a polling based serial function and should not be
 *    used at the same time as interrupt driven serial.
 */
static int sergetchar( bool wait )
{
   /* memory based serial device does not have input */
   return( -1 );
}

/*
 * static int putchar( int ch, bool wait )
 *
 * parameters:
 *    ch - character to attempt to write to the serial port
 *    wait - indicates if the function should until the character can
 *          be transmitted or return right away
 *
 * returns:
 *    the serial character sent, or -1 if the transmitter was not ready
 *    and wait was not specified
 *
 * Attempts to write a character to the serial port in polling fashion.
 * If wait is specified, then it will continue to poll until the transmitter
 * is empty and can accept the character.  If wait is not specified, then
 * it will either return the character if it could be sent, or return -1
 * if the transmitter was not empty.
 * NOTE: this is a polling based serial function and should not be
 *    used at the same time as interrupt driven serial.
 * NOTE: this scheme does not take advantage of the transmit FIFO because
 *    the status bit only indicates if the fifo is empty.  There is no way
 *    to tell how much room is in the fifo.  So it waits until empty each
 *    time.
 */
static int serputchar( int ch, bool wait )
{
   llsermem_buffer[llsermem_data.buf_index] = ch;
   llsermem_data.buf_lastchar = ch;
   llsermem_data.buf_index++;
   if( llsermem_data.buf_index >= llsermem_data.buf_size )
      llsermem_data.buf_index = 0;
   llsermem_buffer[llsermem_data.buf_index] = 0;
   llsermem_data.buf_at = &llsermem_buffer[llsermem_data.buf_index];
   llsermem_data.buf_count++;
   return( ch );
}

/*
 * static void llsermemtx_isr( tick_id_t tid, void* userdata )
 *
 * Parameters:
 *    tid - tick timer handle that is causing the callback
 *    userdata - not used
 *
 * Returns:
 *    none
 *
 * Provides a dummy serial tx interrupt for the serial memory device.
 * It uses a tick timer callback.  The tick timer is armed whenever
 * data is written to the "device" or whenever the "start" function is
 * called.  This will cause the timer callback to occur which will then
 * ask the client for more data.  This will continue until the client
 * indicates it has no data to write.
 */
static void llsermemtx_isr( tick_id_t tid, void* userdata )
{
   unsigned int actualCount;    /* actual number of bytes xfer'd to/from port driver */
   unsigned int space_to_end;

   if( ints_enabled )
   {
      /* compute amount of space to end of buffer */
      space_to_end = llsermem_data.buf_size - llsermem_data.buf_index;
      actualCount = cbfn->txHandler( space_to_end, (char *)llsermem_data.buf_at );
      if( actualCount )
      {
         /*
          * client has now copied bytes into the memory buffer area
          * fix up the buffer index, counts, etc.
          */
         llsermem_data.buf_index += actualCount;
         if( llsermem_data.buf_index >= llsermem_data.buf_size )
            llsermem_data.buf_index = 0;
         llsermem_data.buf_lastchar = 0;
         llsermem_buffer[llsermem_data.buf_index] = 0;
         llsermem_data.buf_at = &llsermem_buffer[llsermem_data.buf_index];
         llsermem_data.buf_count += actualCount;
         /* arm the tick timer so it will trigger the "tx" interrupt again */
         tick_start( tick_handle );
      }
   }
   /*
    * else, if no data was provided by the client then dont
    * re-arm the tick timer.  The client restarts the process by
    * calling the "start" function.
    */
}

static LLSERINTERFACE interfaceTable =
{
   config,
   serclose,
   enable,
   txstart,
   setFlowReady,
   getFlowReady,
   setFlowControl,
   getFlowControl,
   sergetchar,
   serputchar
};

/****************************************************************************
 * Modifications:
 * $Log: 
 *  3    mpeg      1.2         2/13/03 12:01:48 PM    Matt Korte      SCR(s) 
 *        5479 :
 *        Removed old header reference
 *        
 *  2    mpeg      1.1         1/22/03 9:37:44 AM     Matt Korte      SCR(s) 
 *        5258 :
 *        Remove warning.
 *        
 *        
 *  1    mpeg      1.0         12/20/02 1:56:56 PM    Dave Wilson     
 * $
 * 
 *    Rev 1.2   13 Feb 2003 12:01:48   kortemw
 * SCR(s) 5479 :
 * Removed old header reference
 * 
 *    Rev 1.1   22 Jan 2003 09:37:44   kortemw
 * SCR(s) 5258 :
 * Remove warning.
 * 
 * 
 *    Rev 1.0   20 Dec 2002 13:56:56   dawilson
 * SCR(s) 5204 :
 * RAM-based "serial" driver to allow trace to memory function.
 * 
 *
 ****************************************************************************/
