/****************************************************************************/
/*                   CONEXANT PROPRIETARY AND CONFIDENTIAL                  */
/*                       SOFTWARE FILE/MODULE HEADER                        */
/*                 Conexant Systems Inc. (c) 1998, 1999, 2000               */
/*                               Austin, TX                                 */
/*                            All Rights Reserved                           */
/****************************************************************************/
/*
 * Filename:      llsertelegraph.c
 *                  
 * Description:   Low level serial driver for telegraph serial port
 *
 * Author:        kroesche
 *
 ****************************************************************************/
/* $Header: llsertelegraph.c, 6, 2/13/03 12:01:44 PM, Matt Korte$
 ****************************************************************************/

/* includes */
#include "stbcfg.h"
#include "kal.h"
#include "retcodes.h"
#include "llserial.h"

/* defines */
   /*
    * this is the default base address for the serial port.  This
    * should somehow be made generic so that it doenst have to be
    * changed in the code, but it depends on the descriptor setups
    * in boarddata.c.  So for the present implementation, it appears
    * that the serial card is set up for descriptor 4
    */
#define SER16550_ADDRESS 0x314003f8

#define SER16550_GPIO_INTERRUPT 68  /* the interrupt used on telegraph */
#define SER16550_INT INT_GPIO68
   /* derived defines */
#define GPIO_INTERRUPT_BANK   (SER16550_GPIO_INTERRUPT / 32)
#define GPIO_INTERRUPT_BIT    (SER16550_GPIO_INTERRUPT % 32)
#define GPIO_INTERRUPT_MASK   (1<<GPIO_INTERRUPT_BIT)

/* some macros to make register access easier */
#define SETREG(reg,val) (*((unsigned int*)reg)=val)
#define GETREG(reg) (*((unsigned int*)reg))

/* typedefs */
typedef struct
{
   volatile u_int8 data;
   volatile u_int8 ier;
   volatile u_int8 iir;
   volatile u_int8 lcr;
   volatile u_int8 mcr;
   volatile u_int8 lsr;
   volatile u_int8 msr;
} UART_16550;

/* locals */
static volatile UART_16550* serport = (UART_16550*)SER16550_ADDRESS;
static PFNISR chained_isr = NULL;
static PLLSERCALLBACKS cbfn = NULL;
static bool isrInstalled = FALSE;
static bool opened = FALSE;
static unsigned int serint = SER16550_INT;

/* globals */

/* forward declarations */
static LLSERINTERFACE interfaceTable;
static int llsertelegraph_isr( u_int32 id, bool bFIQ, PFNISR* pfnChain);
static int config( unsigned int rate, unsigned int bits,
                        unsigned int stops, bool parity, bool even );
static void enable( bool ena );

/*
 * PLLSERINTERFACE llseropen_Telegraph( PSER_CB_FNS pcbfns )
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
PLLSERINTERFACE llseropen_Telegraph( PLLSERCALLBACKS pcb )
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

   /* perform default initialization */
   if( !config( DEFAULT_RATE, DEFAULT_BITS, DEFAULT_STOPS,
                     DEFAULT_PARITY, DEFAULT_EVEN ) )
   {
      opened = FALSE;
      return( 0 );
   }
   /* this leaves the device configured and with interrupts disabled */

   if( pcb && !isrInstalled )
   {
      /* set up interrupt pin */
      MAKE_GPIO_INPUT_BANK( GPIO_INTERRUPT_BANK, GPIO_INTERRUPT_BIT );
      SET_GPIO_INT_EDGE_BANK( GPIO_INTERRUPT_BANK, GPIO_INTERRUPT_BIT, POS_EDGE );

      /* register the interrupt */
      if ( RC_OK != int_register_isr( INT_GPIO68, (PFNISR)llsertelegraph_isr,
                                       0, 0, &chained_isr ) )
      {
         opened = FALSE;
         return( 0 );
      }
      isrInstalled = TRUE;

   }
   if( pcb )
   {
      /* enable the interrupt at gpio level */
      if( RC_OK != int_enable( SER16550_INT ) )
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
      int_disable( serint );

   /* turn off the device interrupts */
   enable( FALSE );

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
   unsigned int i;

   /* check inputs */
   rate = 115200 / rate;   /* convert rate to divisor */
   if( (rate == 0) || (rate > 65535) )
      return( 0 );

   if( (bits < 5) || (bits > 8) )
      return( 0 );

   if( (stops < 1) || (stops > 2) )
      return( 0 );

   /* program the data rate */
   serport->lcr = 0x80;    /* set up to program baud rate divisor */
   serport->data = rate;   /* lsB of divisor */
   serport->ier = (rate >> 8);   /* msB of divisor */

   /* program the line parameters */
   i = 0;
   if( even )
      i |= 0x10;
   if( parity )
      i |= 0x08;
   if( stops == 2 )
      i |= 0x04;
   i |= (bits - 5);
   serport->lcr = i;

   serport->iir = 0x47;    /* setup fifos, trig level 4, reset */
   serport->ier = 0;       /* device interrupts disabled */
   serport->mcr = 0x08;    /* enable out 2- not really needed here */
   serport->mcr |= 3;      /* assert flow control rts(cts) and dtr(dsr) */

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
   if( ena )
      serport->ier = 0x0f;       /* device interrupts enabled */
   else
      serport->ier = 0x00;       /* device interrupts disabled */
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
   enable( TRUE );
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
   if( state )
      serport->mcr |= 0x01;   /* set dtr */
   else
      serport->mcr &= ~0x01;  /* clear dtr */
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
   return( serport->msr & 0x20 );
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
   if( state )
      serport->mcr |= 0x02;   /* set rts */
   else
      serport->mcr &= ~0x02;  /* clear rts */
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
   return( serport->msr & 0x10 );
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
      if( serport->lsr & 1 )
         return( serport->data );
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
      if( serport->lsr & 0x60 )
      {
         serport->data = ch;
         return( ch );
      }
   } while( wait );
   return( -1 );
}

/*
 * static int llsertelegraph_isr( u_int32 id, BOOL bFIQ, PFNISR *pfnChain)
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
 * Provides interrupt service routine for the 16550 serial device on the
 * telegraph card.
 */
static int llsertelegraph_isr( u_int32 id, bool bFIQ, PFNISR* pfnChain)
{
   unsigned int i;
   u_int8 iir;
   u_int8 lsr;
   u_int8 msr;
   unsigned int actualCount;    /* actual number of bytes xfer'd to/from port driver */
   static char serbuf[20];

   iir = serport->iir;

   if( iir & 1 )  /* serial interrupt not present? */
   {
      *pfnChain = chained_isr;
      return( RC_ISR_NOTHANDLED );
   }

   while( (iir & 0x07) != 1 ) /* get device interrupt status */
   {
      switch( iir & 0x07 )
      {
         /*
          * Modem Status: called when there is a change in any of the
          * modem status bits.  Checks the hardware flow control lines.
          * The physical device has CTS and DSR as inputs so it is DTE.
          * However, the client determines whether it behaves as DCE or
          * DTE by configuring the proper callbacks.
          */
         case 0 :    /* modem status */
            msr = serport->msr;
            if( msr & 0x01 )  /* delta cts */
            {
               if( cbfn->flowControlHandler )
                  cbfn->flowControlHandler( msr & 0x10 );
            }
            if( msr & 0x02 )  /* delta dsr */
            {
               if( cbfn->flowReadyHandler )
                  cbfn->flowReadyHandler( msr & 0x02 );
            }
            break;

         /*
          * TXRDY
          * when this interrupt occurs it means that the fifo
          * is empty.  It can hold up to 16 bytes, so 16 bytes are
          * requested from the client txHandler.  As many bytes as are
          * returned are then copied into the tx FIFO.  If no bytes are
          * available then the tx interrupt is turned off.
          */
         case 2 :    /* tx ready */
            lsr = serport->lsr;
            if( lsr & 0x60 )  /* make sure its empty */
            {
               actualCount = cbfn->txHandler( 16, serbuf );
               if( actualCount )  /* tx data available from client */
               {  /* copy the client data to the ser out, up to 16 bytes */
                  for( i = 0; i < actualCount; i++ )
                     serport->data = serbuf[i];
               }
               else  /* modem data not available */
               {
                  serport->ier = 0x0d;    /* turn off tx interrupt */
               }
            }
            break;

         /*
          * RXRDY
          * When this interrupt occurs it means that there is one or
          * more bytes in the serial input fifo.  As many bytes as are
          * available are read into a buffer which is then passed on
          * to the modem port driver using PortWrite.  We assume that
          * the modem can take all the characters we send.  If it cannot
          * then those characters are just lost.
          */
         case 0x0c : /* rcv fifo timeout - handle like rx rdy */
         case 4 :    /* rx ready */
            i = 0;
            /* read out an char from input fifo as early as possible to avoid overrun error */
			serbuf[i++]=serport->data;
            while( serport->lsr & 1 )  /* data available */
               serbuf[i++] = serport->data;
            cbfn->rxHandler( i, serbuf );
            break;

         case 6 :    /* line status */
            /* have to clear line status reg even if not doing anything with it */
            lsr = serport->lsr;
            if( lsr & 0x02 )  /* overrun error */
               if( cbfn->oeHandler )
                  cbfn->oeHandler();
            if( lsr & 0x04 )  /* parity error */
               if( cbfn->peHandler )
                  cbfn->peHandler();
            if( lsr & 0x08 )  /* framing error */
               if( cbfn->feHandler )
                  cbfn->feHandler();
            if( lsr & 0x10 )  /* break detected */
               if( cbfn->brkHandler )
                  cbfn->brkHandler();

				/* flush out the fifos */
				serport->iir = 0x43; /*83*/
            break;

         default :
            break;
      }  /* endswitch */
      iir = serport->iir;

   }  /* endwhile */

	/* attempted fix for disappearing interrupt requests 
	   make sure serial interrupt goes away for long enough so
		edge can be detected */
	iir = serport->ier;	/* save existing ier */
   serport->ier = 0;    /* turn everything off */
	for( i = 0; i < 50; i++ )
		;	/* delay a little bit */
	serport->ier = iir;	/* restore ier */
   
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
 *  6    mpeg      1.5         2/13/03 12:01:44 PM    Matt Korte      SCR(s) 
 *        5479 :
 *        Removed old header reference
 *        
 *  5    mpeg      1.4         6/26/01 8:56:00 PM     Angela          SCR(s) 
 *        2165 :
 *        DCS2165-changed trig level to 4 to avoid serial overrun errors
 *        
 *  4    mpeg      1.3         3/14/01 11:00:24 AM    Joe Kroesche    #1430 - 
 *        added volatile to make it work when optimization with debug
 *        
 *  3    mpeg      1.2         10/30/00 6:27:10 PM    Joe Kroesche    changed 
 *        descriptor number used for serial port access
 *        
 *  2    mpeg      1.1         9/14/00 10:21:06 AM    Joe Kroesche    fixed 
 *        address used for telegraph serial port
 *        
 *  1    mpeg      1.0         9/11/00 5:16:20 PM     Joe Kroesche    
 * $
 * 
 *    Rev 1.5   13 Feb 2003 12:01:44   kortemw
 * SCR(s) 5479 :
 * Removed old header reference
 * 
 *    Rev 1.4   26 Jun 2001 19:56:00   angela
 * SCR(s) 2165 :
 * DCS2165-changed trig level to 4 to avoid serial overrun errors
 * 
 *    Rev 1.4   26 Jun 2001 19:51:04   angela
 * SCR(s) 2165 :
 * DCS2165-changed trig level to 4 and etc
 * 
 *    Rev 1.4   26 Jun 2001 19:48:52   angela
 * SCR(s) 2165 :
 * DCS#2165- changed trigger level to 4 to avoid serial overrun error
 *   modified llsertelegragh_isr to reduce serial overrun errors
 * 
 *    Rev 1.3   14 Mar 2001 11:00:24   kroescjl
 * #1430 - added volatile to make it work when optimization with debug
 * 
 *    Rev 1.2   Oct 30 2000 18:27:10   kroescjl
 * changed descriptor number used for serial port access
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
