/****************************************************************************/
/*                   CONEXANT PROPRIETARY AND CONFIDENTIAL                  */
/*                       SOFTWARE FILE/MODULE HEADER                        */
/*                 Conexant Systems Inc. (c) 1998, 1999, 2000               */
/*                               Austin, TX                                 */
/*                            All Rights Reserved                           */
/****************************************************************************/
/*
 * Filename:      llserdbgcomm.c
 *                  
 * Description:   Low level serial driver for multi-ice dbg comm channel
 *
 * Author:        kroesche
 *
 ****************************************************************************/
/* $Header: llserdbgcomm.c, 2, 2/13/03 2:06:00 PM, Joe Kroesche$
 ****************************************************************************/

/* includes */
#include "stbcfg.h"
#include "kal.h"
#include "retcodes.h"
#include "llserial.h"

/* defines */
/* typedefs */

/* locals */
static PFNISR chained_rxisr = NULL;
static PFNISR chained_txisr = NULL;
static PLLSERCALLBACKS cbfn = NULL;
static bool isrInstalled = FALSE;
static bool opened = FALSE;

/* globals */

/* forward declarations */
static LLSERINTERFACE interfaceTable;
static int llserdbgcommtx_isr( u_int32 id, bool bFIQ, PFNISR* pfnChain);
static int llserdbgcommrx_isr( u_int32 id, bool bFIQ, PFNISR* pfnChain);

/*
 * PLLSERINTERFACE llseropen_DbgComm( PSER_CB_FNS pcbfns )
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
PLLSERINTERFACE llseropen_DbgComm( PLLSERCALLBACKS pcb )
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
      /* register the interrupt */
      if ( RC_OK != int_register_isr( INT_DBGCOMMTX, (PFNISR)llserdbgcommtx_isr,
                                       0, 0, &chained_txisr ) )
      {
         opened = FALSE;
         return( 0 );
      }
      if ( RC_OK != int_register_isr( INT_DBGCOMMRX, (PFNISR)llserdbgcommrx_isr,
                                       0, 0, &chained_rxisr ) )
      {
         opened = FALSE;
         return( 0 );
      }
      isrInstalled = TRUE;

   }
   if( pcb )
   {
      /* enable the rx interrupt - the tx interrupt is enabled by txstart */
      if( RC_OK != int_enable( INT_DBGCOMMRX ) )
      {
         opened = FALSE;
         return( 0 );
      }
   }
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
      int_disable( INT_DBGCOMMRX );
      int_disable( INT_DBGCOMMTX );
   }

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
    * for the debug comm channel, no config is needed.  However we
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
   if( ena && isrInstalled )
   {
      int_enable( INT_DBGCOMMRX );
      /* only rx is enabled, tx is enabled by tx start */
   }
   else
   {
      int_disable( INT_DBGCOMMRX );
      int_disable( INT_DBGCOMMTX );
   }
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
 * The tx interrupts are disabled which will trigger an interrupt if
 * the tx was idle.
 */
static void txstart( void )
{
   if( isrInstalled )
      int_enable( INT_DBGCOMMTX );
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
   /* debug comm channel has no flow control so this is just stub */
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
   return( 1 );   /* always ready for dbg comm */
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
   /* debug comm channel has no flow control so this is just stub */
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
   return( 1 );   /* always ready for dbg comm */
}

/*
 * Following are the debug comms channel access functions
 */
u_int32 DbgCommStatus(void)
{
    u_int32 ctrl;

    __asm
    {
        mrc     p14, 0, ctrl, c0, c0
    }

    return(ctrl);
}

void DbgCommWrite(u_int32 data)
{
    __asm
    {
        mcr     p14, 0, data, c1, c0
    }
}

u_int32 DbgCommRead(void)
{
    u_int32 data;

    __asm
    {
        mrc     p14, 0, data, c1, c0
    }

    return(data);
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
   do
   {
      if( DbgCommStatus() & 1 )
         return( DbgCommRead() );
   } while( wait );
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
   do
   {
      if( !(DbgCommStatus() & 2) )
      {
         DbgCommWrite( ch );
         return( ch );
      }
   } while( wait );
   return( -1 );
}

/*
 * static int llserdbgcommtx_isr( u_int32 id, BOOL bFIQ, PFNISR *pfnChain)
 *
 * Parameters:
 *    id - interrupt being serviced
 *    bFIQ - true if FIQ (not used)
 *    pfnChain - chained interrupt function
 *
 * Returns:
 *    RC_ISR_HANDLED or RC_ISR_NOTHANDLED depending on whether any
 *    device interrupts were serviced.  This is part of the KAL interface.
 *
 * Provides interrupt service routine for the debug comms channel transmit
 * interrupt
 */
static int llserdbgcommtx_isr( u_int32 id, bool bFIQ, PFNISR* pfnChain)
{
   unsigned int actualCount;    /* actual number of bytes xfer'd to/from port driver */
   static char serbuf[8];

   actualCount = cbfn->txHandler( 1, serbuf );
   if( actualCount )  /* tx data available from client */
   {  /* copy the client data to the ser out  */
      DbgCommWrite( serbuf[0] );
   }
   else  /* no data available for client tx */
   {
      int_disable( INT_DBGCOMMTX );
   }
   
   return( RC_ISR_HANDLED );
}

/*
 * static int llserdbgcommrx_isr( u_int32 id, BOOL bFIQ, PFNISR *pfnChain)
 *
 * Parameters:
 *    id - interrupt being serviced
 *    bFIQ - true if FIQ (not used)
 *    pfnChain - chained interrupt function
 *
 * Returns:
 *    RC_ISR_HANDLED or RC_ISR_NOTHANDLED depending on whether any
 *    device interrupts were serviced.  This is part of the KAL interface.
 *
 * Provides interrupt service routine for the debug comms channel receive
 * interrupt
 */
static int llserdbgcommrx_isr( u_int32 id, bool bFIQ, PFNISR* pfnChain)
{
   static char serbuf[8];

   serbuf[0] = DbgCommRead();
   cbfn->rxHandler( 1, serbuf );

   return( RC_ISR_HANDLED );
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
 *  2    mpeg      1.1         2/13/03 2:06:00 PM     Joe Kroesche    SCR(s) 
 *        5479 :
 *        replaced sabine.h with stbcfg.h
 *        
 *  1    mpeg      1.0         11/1/00 6:26:04 PM     Joe Kroesche    
 * $
 * 
 *    Rev 1.1   13 Feb 2003 14:06:00   kroescjl
 * SCR(s) 5479 :
 * replaced sabine.h with stbcfg.h
 * 
 *    Rev 1.0   Nov 01 2000 18:26:04   kroescjl
 * Initial revision.
 * 
 *    Rev 1.1   Sep 14 2000 09:21:06   kroescjl
 * fixed address used for telegraph serial port
 * 
 *    Rev 1.0   Sep 11 2000 16:16:20   kroescjl
 * Initial revision.
 * 
 *    Rev 1.0   Sep 03 2000 19:37:20   kroescjl
 * Initial revision.
 *
 ****************************************************************************/

