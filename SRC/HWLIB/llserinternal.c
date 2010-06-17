/****************************************************************************/
/*                  CONEXANT PROPRIETARY AND CONFIDENTIAL                   */
/*                      SOFTWARE FILE/MODULE HEADER                         */
/*                Conexant Systems Inc. (c) 1998-2003                       */
/*                              Austin, TX                                  */
/*                           All Rights Reserved                            */
/****************************************************************************/
/*
 * Filename:      llserinternal.c
 *
 *
 * Description:   Low level serial driver for internal serial port
 *
 * Author:        kroesche
 *
 ****************************************************************************/
/* $Header: llserinternal.c, 10, 9/22/03 3:30:24 PM, Bobby Bradford$
 ****************************************************************************/

/** include files **/
#include "stbcfg.h"
#include "kal.h"
#include "retcodes.h"
#include "llserial.h"
#include "gpio.h"

/** local definitions **/
#define RXDMA_BUFFER_SIZE 512

/* note:  should be moved to retcodes.h ... */
#define RC_SER_INVALID_BAUDRATE     (MOD_SER + 0x0001)
#define RC_SER_INVALID_DATABITS     (MOD_SER + 0x0002)
#define RC_SER_INVALID_STOPBITS     (MOD_SER + 0x0003)
#define RC_SER_INVALID_HANDSHAKE    (MOD_SER + 0x0004)
#define RC_SER_MALLOC_FAILED        (MOD_SER + 0x0005)
#define RC_SER_TICK_FAILED          (MOD_SER + 0x0006)

#define RC_SER_GPIO_ERR_BASE        (MOD_SER + 0x0100)

/** local typedefs **/

/** global data **/

/** local data **/
static volatile u_int8 *seraddr[SER_NUM_PORTS] =
{
   (unsigned char*)SER_BASE_UART1,
   (unsigned char*)SER_BASE_UART2,
   (unsigned char*)SER_BASE_UART3
};
static u_int32 serint[SER_NUM_PORTS] =
{
   INT_UART0,
   INT_UART1,
   INT_UART2
};
static PLLSERCALLBACKS cbfn[SER_NUM_PORTS] =
{
   NULL,
   NULL,
   NULL
};
static bool opened[SER_NUM_PORTS] =
{
   FALSE,
   FALSE,
   FALSE
};
static bool isrInstalled[SER_NUM_PORTS] =
{
   FALSE,
   FALSE,
   FALSE
};
static u_int8 ier_reg_value[SER_NUM_PORTS] =
{
   0x00,
   0x00,
   0x00
};
static tick_id_t tickRxDma[SER_NUM_PORTS] =
{
   0,
   0,
   0
};
static u_int32 LastRxAddr[SER_NUM_PORTS] =
{
   0,
   0,
   0
};
static u_int32 *pRxDmaBuf[SER_NUM_PORTS] =
{
   NULL,
   NULL,
   NULL
};
static int32 RxDmaChannel[SER_NUM_PORTS] =
{
   DMA_UART1_RX_CHANNEL,
   DMA_UART2_RX_CHANNEL,
   DMA_UART3_RX_CHANNEL
};
static int32 RxDmaRequest[SER_NUM_PORTS] =
{
   4,
   2,
   DMA_DISABLED
};
static u_int32 ser_FlowReadyIn[SER_NUM_PORTS] =
{
   PIO_UART1_FLOW_READY_IN,
   PIO_UART2_FLOW_READY_IN,
   PIO_UART3_FLOW_READY_IN
};
static u_int32 ser_FlowReadyOut[SER_NUM_PORTS] =
{
   PIO_UART1_FLOW_READY_OUT,
   PIO_UART2_FLOW_READY_OUT,
   PIO_UART3_FLOW_READY_OUT
};
static u_int32 ser_FlowControlIn[SER_NUM_PORTS] =
{
   PIO_UART1_FLOW_CONTROL_IN,
   PIO_UART2_FLOW_CONTROL_IN,
   PIO_UART3_FLOW_CONTROL_IN
};
static u_int32 ser_FlowControlOut[SER_NUM_PORTS] =
{
   PIO_UART1_FLOW_CONTROL_OUT,
   PIO_UART2_FLOW_CONTROL_OUT,
   PIO_UART3_FLOW_CONTROL_OUT
};
static u_int32 ser_RingIndicateIn[SER_NUM_PORTS] =
{
   PIO_UART1_RING_INDICATE_IN,
   PIO_UART2_RING_INDICATE_IN,
   PIO_UART3_RING_INDICATE_IN
};
static u_int32 ser_CarrierDetectIn[SER_NUM_PORTS] =
{
   PIO_UART1_CARRIER_DETECT_IN,
   PIO_UART2_CARRIER_DETECT_IN,
   PIO_UART3_CARRIER_DETECT_IN
};
static PFNISR chained_isr = NULL;
static PFNISR handshake_chained_isr = NULL;

/** forward declarations **/
static PLLSERINTERFACE interfaceTable[SER_NUM_PORTS];
static PLLSERINTERFACE llseropen_Internal( int32 dev, PLLSERCALLBACKS pcb );
static int32 llserinternal_isr( u_int32 id, bool bFIQ, PFNISR* pfnChain);
static int32 config( int32 dev, u_int32 rate, u_int32 bits, u_int32 stops,
      bool parity, bool even );
static void enable( int32 dev, bool ena );

static int32 openHandshake( int32 dev );
static int32 closeHandshake( int32 dev );
static int32 setHandshake( u_int32 uGPIO, bool bValue );
static int32 getHandshake( u_int32 uGPIO, bool *pbValue );
static int32 isrHandshake( u_int32 id, bool bFIQ, PFNISR* pfnChain);


static int32 openRxDma ( int32 dev );
static int32 closeRxDma ( int32 dev );
static void cbRxDma(tick_id_t timer, void *dwRef);

/** external data declarations (discouraged) **/

/** external function declarations **/

/** public functions **/
/******************************************************************************
 * Function Name: llseropen_Internal(0|1|2)
 *
 * Description  : Device specific open functions for the internal UARTs
 *
 * Inputs:        pcb      pointer to the callback functions to be provided by
 *                         the host.  If NULL, then no interrupts are enabled
 *                         and the UART is configured to work in POLLED mode.
 *
 * Outputs:       None
 *
 * Returns:       Pointer  NULL if device not opened successfully, otherwise,
 *                         a pointer to the device function table for
 *                         configuration, read, write, close, etc.
 *
 * Notes:         Wrapper around a more generic open function below
 *
 * Context:       Must be called from task context.
 *
 *****************************************************************************/
PLLSERINTERFACE llseropen_Internal0( PLLSERCALLBACKS pcb )
{
   return( llseropen_Internal( 0, pcb ) );
} /* llseropen_Internal0 */


PLLSERINTERFACE llseropen_Internal1( PLLSERCALLBACKS pcb )
{
   return( llseropen_Internal( 1, pcb ) );
} /* llseropen_Internal1 */


PLLSERINTERFACE llseropen_Internal2( PLLSERCALLBACKS pcb )
{
   return( llseropen_Internal( 2, pcb ) );
} /* llseropen_Internal2 */


/** private functions **/
/******************************************************************************
 * Function Name: llseropen_Internal
 *
 * Description  : Open and initialize the selected internal UART for operation
 *
 * Inputs:        dev -    device (UART) number (0, 1, 2, etc.) to open
 *                pcbfns - pointer to callback functions struct provided by
 *                         client.  These are the functions that are called
 *                         back whenever a serial event occurs
 *
 * Outputs:       None
 *
 * Returns:       pTbl -  An interface structure containing pointers to the
 *                        low level serial driver functions.
 *
 * Notes:         Saves the client callback structure.  If this is not null,
 *                then the ISR is installed.  Just call it with a NULL
 *                parameter if the the client does not want to use interrupt
 *                features.  An interface is returned to the client that should
 *                be used for all further calls to the low level serial driver.
 *
 * Context:       Must be called from task context.
 *
 *****************************************************************************/
static PLLSERINTERFACE llseropen_Internal( int32 dev, PLLSERCALLBACKS pcb )
{
   bool crit;
   int32 RetCode;

   /* check to make sure this device not already opened */
   crit = critical_section_begin();
   if( opened[dev] )
   {
      critical_section_end( crit );
      return ( NULL );
   }
   opened[dev] = TRUE;
   critical_section_end( crit );


   /* perform default initialization */
   RetCode = config( dev, DEFAULT_RATE, DEFAULT_BITS, DEFAULT_STOPS,
         DEFAULT_PARITY, DEFAULT_EVEN );
   if( RC_OK != RetCode )
   {
      opened[dev] = FALSE;
      return ( NULL );
   }

   /* Note:  The device is now initialized, and interrupts are disabled */

   /* If pcb is null then the user does not want interrupt driven
    * operation, so dont install handler.  If isr has already been
    * installed, dont do it again. */
   if( pcb && !isrInstalled[dev] )
   {
      /* install and configure GPIO handshake signals */
      RetCode = openHandshake(dev);
      if ( RC_OK != RetCode )
      {
         opened[dev] = FALSE;
         return ( NULL );
      }

      /* install and configure DMA operation for RX */
      RetCode = openRxDma(dev);
      if ( RC_OK != RetCode )
      {
         opened[dev] = FALSE;
         return ( NULL );
      }

      /* register the uart interrupt */
      RetCode = int_register_isr( serint[dev], (PFNISR)llserinternal_isr,
            0, 0, &chained_isr );
      if ( RC_OK != RetCode )
      {
         opened[dev] = FALSE;
         return ( NULL );
      }
      isrInstalled[dev] = TRUE;
   }

   /* set the shadow interrupt enable reg to 0 (no interrupts) */
   ier_reg_value[dev] = 0x00;

   /* if we have a valid callback pointer, enable the interrupts */
   if( pcb )
   {
      /* enable the interrupt at pic level */
      RetCode = int_enable( serint[dev] );
      if( RC_OK != RetCode )
      {
         opened[dev] = FALSE;
         return( NULL );
      }

      /* set the shadow interrupt enable reg for all interrupts enabled */
      ier_reg_value[dev] = 0xff;
   }

   setHandshake(ser_FlowReadyOut[dev], TRUE);
   setHandshake(ser_FlowControlOut[dev], TRUE);

   cbfn[dev] = pcb;

   return( interfaceTable[dev] );
} /* llseropen_Internal */


/******************************************************************************
 * Function Name: serclose
 *
 * Description  : Close the selected device/uart
 *
 * Inputs:        dev -    The device to close
 *
 * Outputs:       None
 *
 * Returns:       RETCODE
 *                   RC_OK          Success
 *                   ...            Otherwise, some type of error occured
 *
 * Notes:         Cleans up the serial port.  It turns off the interrupts and
 *                marks the device as available so that it can be opened by
 *                someone else.
 *
 *                The wrapper functions (close0, close1, and close2) take no
 *                arguments, and simply pass the appropriate 'dev' to the
 *                serclose function.  Also note that the wrapper functions will
 *                "invert" the sense of the return code.  Decided not to do
 *                global replace of the return code, since many places are
 *                already using the 'boolean' sense of TRUE is OK and FALSE
 *                means an error.
 *
 * Context:       May be called from interrupt or task context.
 *
 *****************************************************************************/
static int32 serclose( int dev )
{
   int32 RetCode = RC_OK;

   /* turn off interrupts at the pic */
   if( isrInstalled[dev] )
   {
      RetCode |= closeHandshake(dev);

      RetCode |= closeRxDma(dev);

      RetCode |= int_disable( serint[dev] );
   }

   /* turn off the device interrupts */
   enable( dev, FALSE );

   cbfn[dev] = NULL;
   opened[dev] = FALSE;

   return( RetCode );
} /* serclose */


static int close0( void )
{
   int32 RetCode = serclose(0);
   return ( (RC_OK == RetCode) ? 1 : 0 ) ;
} /* close0 */


static int close1( void )
{
   int32 RetCode = serclose(1);
   return ( (RC_OK == RetCode) ? 1 : 0 ) ;
} /* close1 */


static int close2( void )
{
   int32 RetCode = serclose(2);
   return ( (RC_OK == RetCode) ? 1 : 0 ) ;
} /* close2 */


/******************************************************************************
 * Function Name: config
 *
 * Description  : configure the device (uart) for operation
 *
 * Inputs:        dev -    the device (0-2) to configure
 *                rate -   the line rate in bits per second (2 - 115200)
 *                bits -   the number of data bits (7, 8)
 *                stops -  the number of stop bits (1, 2)
 *                parity - boolean to enable parity (if TRUE)
 *                even -   boolean to set even parity (if TRUE)
 *
 * Outputs:       None
 *
 * Returns:       RETCODE
 *                   RC_OK          If configuration is successful
 *                   RC_SER_INVALID_BAUDRATE    invalid data rate specified
 *                   RC_SER_INVALID_STOPBITS    invalid number of stop bits
 *                   RC_SER_INVALID_DATABITS    invalid number of data bits
 *
 * Notes:         Wrapper functions, close0, close1, and close2, will invert
 *                the sense of the return value (same as the close functions
 *                above).
 *
 * Context:       May be called from interrupt or task context.
 *
 *****************************************************************************/
static int32 config(
      int32 dev,
      u_int32 rate,
      u_int32 bits,
      u_int32 stops,
      bool parity,
      bool even )
{
   u_int32 i;
   #if UART_TYPE!=UART_COLORADO
   LPREG fractreg = (LPREG)SER_NUM_BRD_FRACTIONAL_REG(dev);
   int fract;
   #endif
   volatile unsigned char* serport = seraddr[dev];

   /* check inputs */
   #if UART_TYPE!=UART_COLORADO
   /* fractional_part = (((CLK_CONSTANT % BR) / BR) * 100) / 25 */
   fract = ((((3375000 % rate) << 3) / rate)) >> 1;
   #endif

   rate = (3375000 / rate) - 1;   /* convert rate to divisor */
   /* the above should take into account rounding errors, but
      for 38400, that doesnt work
      calculated divisor is 86.890625; this -1 with rouding is
      87, but 87 doesnt work, 86 must be used */
   /* Bruce Bitner, "Colorado and Hondo's BRD leaves a significant
      % error after dividing down the clock.  If the larger divisor
      is used (87 in the above example) then the baud rate is less
      than the desired baud rate.  This causes the chip to miss
      stop bits occassionally.  WaBASH adds a fractional piece to
      the BRD which brings the percent error down to acceptable
      amounts.  (MFB) */

   if( (rate == 0) || (rate > 65535) )
   {
      return( RC_SER_INVALID_BAUDRATE );
   }

   if( (bits < 7) || (bits > 8) )
   {
      return( RC_SER_INVALID_DATABITS );
   }

   if( (stops < 1) || (stops > 2) )
   {
      return( RC_SER_INVALID_STOPBITS );
   }

   /* program the data rate */
   /* First, enable the Baud rate divisor register, then program
    * the LSB and then the MSB registers, and if appropriate, the
    * fractional register */
   serport[SER_FRAME_CTRL_REG] = 0x80;
   serport[SER_BRD_LOW_REG] = rate;
   serport[SER_BRD_HIGH_REG] = (rate >> 8);
   #if UART_TYPE!=UART_COLORADO
   *fractreg = fract;
   #endif


   /* program the line parameters */
   i = 0;
   if( even )
   {
      i |= 0x10;
   }
   if( parity )
   {
      i |= 0x08;
   }
   if( stops == 2 )
   {
      i |= 0x04;
   }
   i |= (bits - 7);
   serport[SER_FRAME_CTRL_REG] = i;

   /* setup the fifo control register */
   serport[SER_FIFO_CTRL_REG] = 0xb6;

   /* disable all interrtupts for now */
   serport[SER_INT_ENABLE_REG] = 0;

   return( RC_OK );
} /* config */


static int config0(
      unsigned int rate,
      unsigned int bits,
      unsigned int stops,
      bool parity,
      bool even )
{
   int32 RetCode = config( 0, rate, bits, stops, parity, even );
   return ( (RC_OK == RetCode) ? 1 : 0 ) ;
} /* config0 */


static int config1(
      unsigned int rate,
      unsigned int bits,
      unsigned int stops,
      bool parity,
      bool even )
{
   int32 RetCode = config( 1, rate, bits, stops, parity, even );
   return ( (RC_OK == RetCode) ? 1 : 0 ) ;
} /* config1 */


static int config2(
      unsigned int rate,
      unsigned int bits,
      unsigned int stops,
      bool parity,
      bool even )
{
   int32 RetCode = config( 2, rate, bits, stops, parity, even );
   return ( (RC_OK == RetCode) ? 1 : 0 ) ;
} /* config2 */


/******************************************************************************
 * Function Name: enable(0|1|2)
 *
 * Description  : Enable/Disable the device interrupts
 *
 * Inputs:        ena -    boolean to indicate whether to enable (TRUE) or
 *                         disable the device interrupts
 *
 * Outputs:       None
 *
 * Returns:       None
 *
 * Notes:         None
 *
 * Context:       May be called from interrupt or task context.
 *
 *****************************************************************************/
static void enable( int32 dev, bool ena )
{
   volatile u_int8 *serport = seraddr[dev];

   if( ena )
   {
      serport[SER_INT_ENABLE_REG] = ier_reg_value[dev];
   }
   else
   {
      serport[SER_INT_ENABLE_REG] = 0;
   }
} /* enable */


static void enable0( bool ena )
{
   enable( 0, ena );
} /* enable0 */


static void enable1( bool ena )
{
   enable( 1, ena );
} /* enable1 */


static void enable2( bool ena )
{
   enable( 2, ena );
} /* enable2 */


/******************************************************************************
 * Function Name: txstart(0|1|2)
 *
 * Description  : startup the tx process (interrupt)
 *
 * Inputs:        None
 *
 * Outputs:       None
 *
 * Returns:       None
 *
 * Notes:         Signals the serial driver that the client has tx data
 *                available.  The tx interrupts are enabled which will trigger
 *                an interrupt if the tx was idle.
 *
 * Context:       May be called from interrupt or task context.
 *
 *****************************************************************************/
static void txstart0( void )
{
   enable( 0, TRUE);
} /* txstart0 */


static void txstart1( void )
{
   enable( 1, TRUE);
} /* txstart1 */


static void txstart2( void )
{
   enable( 2, TRUE);
} /* txstart2 */


/******************************************************************************
 * Function Name: setFlow(Ready|Control)
 *
 * Description  : API function to set a flow control handshake signal
 *
 * Inputs:        state - boolean value to set state to
 *
 * Outputs:       None
 *
 * Returns:       None
 *
 * Notes:         Uses the setHandshake function, but ignores the return code
 *                for now.  Maybe later we will do more.
 *
 * Context:       May be called from interrupt or task context.
 *
 *****************************************************************************/

static void setFlowReady0( bool state )
{
   setHandshake( PIO_UART1_FLOW_READY_OUT, state );
} /* setFlowReady0 */

static void setFlowReady1( bool state )
{
   setHandshake( PIO_UART2_FLOW_READY_OUT, state );
} /* setFlowReady1 */

static void setFlowReady2( bool state )
{
   setHandshake( PIO_UART2_FLOW_READY_OUT, state );
} /* setFlowReady2 */

static void setFlowControl0( bool state )
{
   setHandshake( PIO_UART1_FLOW_CONTROL_OUT, state );
} /* setFlowControl0 */

static void setFlowControl1( bool state )
{
   setHandshake( PIO_UART2_FLOW_CONTROL_OUT, state );
} /* setFlowControl1 */

static void setFlowControl2( bool state )
{
   setHandshake( PIO_UART2_FLOW_CONTROL_OUT, state );
} /* setFlowControl2 */


/******************************************************************************
 * Function Name: getFlow(Ready|Control)
 *
 * Description  : API function to set a flow control handshake signal
 *
 * Inputs:        None
 *
 * Outputs:       None
 *
 * Returns:       Boolean value of state of handshake signal
 *
 * Notes:         Uses the getHandshake function, but ignores the return code
 *                for now.  Maybe later we will do more.
 *
 * Context:       May be called from interrupt or task context.
 *
 *****************************************************************************/

static bool getFlowReady0( void )
{
   bool bValue = TRUE;
   getHandshake( PIO_UART1_FLOW_READY_IN, &bValue );
   return (bValue);
} /* getFlowReady0 */

static bool getFlowReady1( void )
{
   bool bValue = TRUE;
   getHandshake( PIO_UART2_FLOW_READY_IN, &bValue );
   return (bValue);
} /* getFlowReady1 */

static bool getFlowReady2( void )
{
   bool bValue = TRUE;
   getHandshake( PIO_UART3_FLOW_READY_IN, &bValue );
   return (bValue);
} /* getFlowReady2 */

static bool getFlowControl0( void )
{
   bool bValue = TRUE;
   getHandshake( PIO_UART1_FLOW_CONTROL_IN, &bValue );
   return (bValue);
} /* getFlowControl0 */

static bool getFlowControl1( void )
{
   bool bValue = TRUE;
   getHandshake( PIO_UART2_FLOW_CONTROL_IN, &bValue );
   return (bValue);
} /* getFlowControl1 */

static bool getFlowControl2( void )
{
   bool bValue = TRUE;
   getHandshake( PIO_UART3_FLOW_CONTROL_IN, &bValue );
   return (bValue);
} /* getFlowControl2 */


/******************************************************************************
 * Function Name: sergetchar
 *
 * Description  : get a char from the uart, optional wait forever
 *
 * Inputs:        dev - device (UART) (0, 1, 2)
 *                wait - boolean (TRUE will wait forever)
 *
 * Outputs:       None
 *
 * Returns:       character read, or -1 if no character, and wait not specified
 *
 * Notes:         Attempts to read a character from the serial port in polling
 *                fashion.  If wait is specified, then it will continue to poll
 *                until a character is available.  If wait is not specified,
 *                then it will either return a character if available, or
 *                return -1 if none available.
 *                NOTE: this is a polling based serial function and should not
 *                be used at the same time as interrupt driven serial.
 *
 * Context:       Must be called from task context.
 *
 *****************************************************************************/
static int32 sergetchar( int32 dev, bool wait )
{
   volatile u_int8 *serport = seraddr[dev];

   do
   {
      if( serport[SER_STATUS_REG] & 1 )
      {
         return( serport[SER_DATA_REG] );
      }
   } while( wait ) ;

   return( -1 );
} /* sergetchar */

static int getchar0( bool wait )
{
   return( sergetchar( 0, wait ) );
} /* getchar0 */

static int getchar1( bool wait )
{
   return( sergetchar( 1, wait ) );
} /* getchar1 */

static int getchar2( bool wait )
{
   return( sergetchar( 2, wait ) );
} /* getchar2 */


/******************************************************************************
 * Function Name: serputchar
 *
 * Description  : write a char to the uart, optional wait forever
 *
 * Inputs:        dev - device (UART) (0, 1, 2)
 *                ch - character to write
 *                wait - boolean (TRUE will wait forever)
 *
 * Outputs:       None
 *
 * Returns:       character sent, or -1 if no character sent, and wait not
 *                specified
 *
 * Notes:         Attempts to write a character to the serial port in polling
 *                fashion. If wait is specified, then it will continue to poll
 *                until the transmitter is empty and can accept the character.
 *                If wait is not specified, then it will either return the
 *                character if it could be sent, or return -1 if the transmitter
 *                was not empty.
 *                NOTE: this is a polling based serial function and should not
 *                be used at the same time as interrupt driven serial.
 *                NOTE: this scheme does not take advantage of the transmit FIFO
 *                because the status bit only indicates if the fifo is empty.
 *                There is no way to tell how much room is in the fifo.  So it
 *                waits until empty each time.
 *
 * Context:       Must be called from task context.
 *
 *****************************************************************************/
static int32 serputchar( int32 dev, int32 ch, bool wait )
{
   volatile u_int8 *serport = seraddr[dev];

   do
   {
      if( serport[SER_STATUS_REG] & 0x60 )
      {
         serport[SER_DATA_REG] = ch;
         return( ch );
      }
   } while( wait );
   return( -1 );
} /* serputchar */

static int putchar0( int ch, bool wait )
{
   return( serputchar( 0, ch, wait ) );
} /* putchar0 */

static int putchar1( int ch, bool wait )
{
   return( serputchar( 1, ch, wait ) );
} /* putchar1 */

static int putchar2( int ch, bool wait )
{
   return( serputchar( 2, ch, wait ) );
} /* putchar2 */


/******************************************************************************
 * Function Name: flushRx
 *
 * Description  : Flush the RX Fifo
 *
 * Inputs:        dev - device (UART) (0, 1, 2)
 *
 * Outputs:       None
 *
 * Returns:       None
 *
 * Notes:         Resets the fifos and attempts to clear out the dreaded "error
 *                in receive fifo" bit.
 *
 * Context:       May be called from interrupt or task context.
 *
 *****************************************************************************/
static void flushRx( int32 dev )
{
   volatile u_int8 *serport = seraddr[dev];
   u_int32 i, tmp;

   /* first read out 16 bytes just to be sure the fifo is really empty */
   for( i = 0; i < 16; i++ )
   {
      tmp = serport[SER_DATA_REG];
   }

   /* next clear the receive fifo */
   i = serport[SER_FIFO_CTRL_REG];
   i |= 0x02;
   serport[SER_FIFO_CTRL_REG] = i;

   /* now read the fifo again to really really make sure its clear */
   for( i = 0; i < 16; i++ )
   {
      tmp = serport[SER_DATA_REG];
   }
}

/******************************************************************************
 * Function Name: handleRxError
 *
 * Description  : Flush the RX Fifo
 *
 * Inputs:        dev - device (UART) (0, 1, 2)
 *                lsr - the current value of the line status register
 *
 * Outputs:       None
 *
 * Returns:       None
 *
 * Notes:         Steps through the error bits in the lsr, calling the
 *                appropriate callback functions as needed.  Then it flushes
 *                the rx fifo
 *
 * Context:       May be called from interrupt or task context.
 *
 *****************************************************************************/
static void handleRxError( int32 dev, u_int8 lsr )
{
   /* if no callback functions, just flush the RX and return */
   if( cbfn[dev] == NULL )
   {
      flushRx( dev );
      return;
   }

   /* if callback functions defined, process errors, then flush rx and return */

   /* overrun error */
   if( lsr & 0x02 )
   {
      if( cbfn[dev]->oeHandler )
      {
         cbfn[dev]->oeHandler();
      }
   }

   /* parity error */
   if( lsr & 0x04 )
   {
      if( cbfn[dev]->peHandler )
      {
         cbfn[dev]->peHandler();
      }
   }

   /* framing error */
   if( lsr & 0x08 )
   {
      if( cbfn[dev]->feHandler )
      {
         cbfn[dev]->feHandler();
      }
   }

   /* break detected */
   if( lsr & 0x10 )
   {
      if( cbfn[dev]->brkHandler )
      {
         cbfn[dev]->brkHandler();
      }
   }

   flushRx( dev );
}


/******************************************************************************
 * Function Name: llserinternal_isr
 *
 * Description  : interrupt handler for uarts
 *
 * Inputs:        id - interrupt id
 *                bFIQ - TRUE, if FIQ interrupt (not used)
 *                pfnChain - chain function
 *
 * Outputs:       None
 *
 * Returns:       RC_ISR_HANDLED if interrupt was ours, and we processed it
 *                RC_ISR_NOTHANDLED if not ours, or other error
 *
 * Notes:         None
 *
 * Context:       Must be called from interrupt context.
 *
 *****************************************************************************/
static int32 llserinternal_isr( u_int32 id, bool bFIQ, PFNISR* pfnChain)
{
   u_int32 i;
   u_int8 isr;
   u_int8 lsr;
   u_int32 actualCount;  /* actual number of bytes xfer'd to/from port driver */
   int8 serbuf[20];
   int32 dev;
   volatile u_int8 *serport;

   /* Determine if it is a UART interrupt id */
   switch( id )
   {
      case INT_UART0 :
         dev = 0;
         break;

      case INT_UART1 :
         dev = 1;
          break;

      case INT_UART2 :
         dev = 2;
          break;

      default :
         *pfnChain = chained_isr;
         return( RC_ISR_NOTHANDLED );
   }

   /* Setup base serial port address */
   serport = seraddr[dev];

   /* Check to see if the we actually got an interrupt */
   isr = (serport[SER_STATUS_REG] & serport[SER_INT_ENABLE_REG] );
   if (!isr)
   {
      *pfnChain = chained_isr;
      return( RC_ISR_NOTHANDLED );
   }

   /* Continue to service the interrupt status until there are no pending
    * interrupts
    * Note:  In the 16550, the IIR register would be read to determine the
    * highest priority active interrupt.  The IRLVL register in the Colorado
    * and Hondo do not implment this priority scheme correctly, so it is
    * possible to get invalid/incorrect interrupt priorities reported.
    * This has been fixed in Rio (Wabash, Brazos, etc.), but needs to be
    * addressed in this code by looking at the ISR and IER registers to
    * determine which interrupts are active, and address them in the
    * appropriate priority
    */
   while( isr )
   {
      /*
       * TXRDY
       * when this interrupt occurs it means that the fifo
       * tx threshold has been triggered.  The amount of free space
       * in the fifo can be determined and that number of bytes are
       * requested from the client txHandler.  As many bytes as are
       * returned are then copied into the tx FIFO.  If no bytes are
       * available then the tx interrupt is turned off.
       */
      if ( isr & (SER_STATUS_TXFIFOSERVICE_MASK|SER_STATUS_TXIDLE_MASK) )
      {
         /* compute space available in tx fifo and request data from client */
         actualCount = cbfn[dev]->txHandler(
               16-serport[SER_TX_FIFO_STATUS_REG], (char *)serbuf );

         /* check if tx data available from client */
         if( actualCount )
         {
            /* if tx data available, copy client data to the tx serial port */
            for( i = 0; i < actualCount; i++ )
            {
               serport[SER_DATA_REG] = serbuf[i];
            }
         }

         /* Here, if no tx data is avaialable ... disable tx interrupts */
         else
         {
            serport[SER_INT_ENABLE_REG] &=
                  ~(SER_STATUS_TXFIFOSERVICE_MASK|SER_STATUS_TXIDLE_MASK) ;
         }
      }

      /*
       * RXRDY
       * When this interrupt occurs it means that the rx fifo
       * threshold has been triggered, or else that data is still in
       * the fifo and no data has been received in a while.
       * As many bytes as are
       * available are read into a buffer which is then passed on
       * to the modem port driver using PortWrite.  We assume that
       * the modem can take all the characters we send.  If it cannot
       * then those characters are just lost.
       */
      if ( isr & SER_STATUS_RXFIFOSERVICE_MASK )
      {
         i = 0;

         /* Copy data from the RX FIFO to the buffer,
          * if there is any, and there are no errors
          */
         while( serport[SER_RX_FIFO_STATUS_REG] > 0 )
         {
            lsr = serport[SER_STATUS_REG];
            if( lsr & 0x1e )
            {
               handleRxError( dev, lsr );
            }
            else
            {
               serbuf[i++] = serport[SER_DATA_REG];
            }
         }

         /* Here, if there was data, give it to the "host" rxhandler */
         if( i )
         {
            cbfn[dev]->rxHandler( i, (char *)serbuf );
         }

         /* Check one more time for serial rx fifo errors */
         lsr = serport[SER_STATUS_REG];
         if( lsr & 0x80 )
         {
            flushRx( dev );
         }
      }

      /* Here, check for "line status" errors, and process them accordingly */
      if ( isr & (SER_STATUS_RXFIFOOVERRUN_MASK|SER_STATUS_PARITYERROR_MASK|
            SER_STATUS_FRAMINGERROR_MASK|SER_STATUS_BREAKRECEIVED_MASK) )
      {
         lsr = serport[SER_STATUS_REG];
         handleRxError( dev, lsr );
      }

      isr = (serport[SER_STATUS_REG] & serport[SER_INT_ENABLE_REG] );
   }  /* endwhile */

   return( RC_ISR_HANDLED );
}

/* function interface tables for each of the 3 serial devices */
static LLSERINTERFACE iftable0 =
{
   config0,
   close0,
   enable0,
   txstart0,
   setFlowReady0,
   getFlowReady0,
   setFlowControl0,
   getFlowControl0,
   getchar0,
   putchar0
};

static LLSERINTERFACE iftable1 =
{
   config1,
   close1,
   enable1,
   txstart1,
   setFlowReady1,
   getFlowReady1,
   setFlowControl1,
   getFlowControl1,
   getchar1,
   putchar1
};

static LLSERINTERFACE iftable2 =
{
   config2,
   close2,
   enable2,
   txstart2,
   setFlowReady2,
   getFlowReady2,
   setFlowControl2,
   getFlowControl2,
   getchar2,
   putchar2
};

static PLLSERINTERFACE interfaceTable[3] =
{
   &iftable0,
   &iftable1,
   &iftable2
};



/******************************************************************************
 * Function Name: openRxDma
 *
 * Description  : Open and Initialize the RX DMA handler
 *
 * Inputs:        dev - device (uart) to open up
 *
 * Outputs:       None
 *
 * Returns:       RC_OK - Success
 *                ... various error codes for problems
 *
 * Notes:         None
 *
 * Context:       May be called from interrupt or task context.
 *
 *****************************************************************************/
static int32 openRxDma ( int32 dev )
{
   volatile u_int8 *serport = seraddr[dev];
   int32 chan;
   int32 req;
   int32 RetCode;

   /* check to see if DMA channel is enabled for this device */
   if (DMA_DISABLED == RxDmaChannel[dev])
   {
      return ( RC_OK );
   }
   chan = RxDmaChannel[dev];

   /* check to see if DMA request is defined for this device */
   if (DMA_DISABLED == RxDmaRequest[dev])
   {
      return ( RC_OK );
   }
   req = RxDmaRequest[dev];

   /* check to see if we can get non-cached buffer space */
   pRxDmaBuf[dev] = mem_nc_malloc(RXDMA_BUFFER_SIZE*4);
   if (NULL == pRxDmaBuf[dev])
   {
      return ( RC_SER_MALLOC_FAILED );
   }
   LastRxAddr[dev] = (unsigned int)pRxDmaBuf[dev];

   /* disable UART RX interrupt ... handled by DMA now */
   ier_reg_value[dev] &= ~SER_INT_ENABLE_RXFIFO_MASK;


   /* *** Program the DMA Mode Register *** */

   /* Enable Multiple Concurrent DMAs and Bus Locking */
   /* Disable (was Enable) Multiple Concurrent DMAs and Bus Locking */
   /* and enable buffered mode */
   *(LPREG)DMA_MODE_REG = *(LPREG)DMA_MODE_REG & ~(DMA_MULTIPLE | DMA_BUS_LOCK) ;
   *(LPREG)DMA_MODE_REG = *(LPREG)DMA_MODE_REG | 0x00000004 ;


   /* *** Program DMA Channel Registers *** */

   /* ************************************************ */
   /* *** Setup DMA Channel 0 to be our RX Channel *** */
   /*  DMA source is UART1 RX, dest is memory buffer */
   /* Note: DMA Enabled Elsewhere... */
   /* ************************************************ */

   /* Program Channel Attributes */
   *(LPREG)DMA_CONTROL_REG_CH(chan) =
         DMA_CHANNEL_RECOVERY_TIME(15) |  /* Recovery Time after DMA Op */
         DMA_XACT_SZ(1) |                 /* Move this many 32-bit words */
         DMA_SELECT_REQ(req) |            /* DMA request line for channel */
         DMA_XFER_MODE_NORMAL |           /* Normal operation */
         DMA_PREEMPT ;                    /* Channel may preempt other DMA */

   /* Program DMA Source Base Address to address of UART1 Rx Fifo
    * and indicate to DMA state machine that it should not increment the
    * address (since its a port) */
   *(LPREG)DMA_SRCBASE_REG_CH(chan) =
         (u_int32)( (u_int32)(&serport[SER_DATA_REG]) | DMA_NONINCR ) ;

   /* Program DMA CURRENT Source Address to address of UART1 Rx Fifo */
   *(LPREG)DMA_SRCADDR_REG_CH(chan) =
         (u_int32) (u_int32)(&serport[SER_DATA_REG]) ;

   /* Program characteristics of DMA Source (interrupt interval and source
    * buffer size) */
   *(LPREG)DMA_SRCBUF_REG_CH(chan) = (u_int32)0 ;

   /* Program DMA Destination Base Address to Base of In-Memory Non-Cached
    * RX Fifo and indicate that the DMA state machine should increment the
    * address (not a port) */
   *(LPREG)DMA_DSTBASE_REG_CH(chan) =
         (u_int32)( ( (unsigned int)(pRxDmaBuf[dev]) & 0xfffffffc ) |
         DMA_SRCINCR ) ;

   /* Program DMA Destination CURRENT Address to Base of In-Memory Non-Cached
    * RX Fifo */
   *(LPREG)DMA_DSTADDR_REG_CH(chan) =
         (u_int32)( (unsigned int)(pRxDmaBuf[dev]) & 0xfffffffc) ;

   /* Program characteristics of DMA Destination Buffer (interrupt interval
    * and destination buffer size) */
   *(LPREG)DMA_DSTBUF_REG_CH(chan) =
         (u_int32)((RXDMA_BUFFER_SIZE<<16) | (RXDMA_BUFFER_SIZE*4)) ;

   /* Clear any pending dma interrupts. */
   *(LPREG)DMA_STATUS_REG_CH(chan) = 0x7E ; /* clear bits 1-6 */

   /* Enable dma */
   *(LPREG)DMA_CONTROL_REG_CH(chan) =
         *(LPREG)DMA_CONTROL_REG_CH(chan) | DMA_ENABLE ;

   /* Create and Setup to Tick Timer for DMA processing */
   tickRxDma[dev] = tick_create(cbRxDma, (void *)dev, NULL);
   if (0 == tickRxDma[dev])
   {
      mem_nc_free(pRxDmaBuf[dev]);
      pRxDmaBuf[dev] = NULL;
      return ( RC_SER_TICK_FAILED );
   }

   RetCode = tick_stop(tickRxDma[dev]);
   if (RetCode != RC_OK)
   {
      tick_destroy(tickRxDma[dev]);
      tickRxDma[dev] = 0;
      mem_nc_free(pRxDmaBuf[dev]);
      pRxDmaBuf[dev] = NULL;
      return ( RC_SER_TICK_FAILED );
   }

   RetCode = tick_set(tickRxDma[dev], 8, FALSE);
   if (RetCode != RC_OK)
   {
      tick_destroy(tickRxDma[dev]);
      tickRxDma[dev] = 0;
      mem_nc_free(pRxDmaBuf[dev]);
      pRxDmaBuf[dev] = NULL;
      return ( RC_SER_TICK_FAILED );
   }

   RetCode = tick_start(tickRxDma[dev]);
   if (RetCode != RC_OK)
   {
      tick_destroy(tickRxDma[dev]);
      tickRxDma[dev] = 0;
      mem_nc_free(pRxDmaBuf[dev]);
      pRxDmaBuf[dev] = NULL;
      return ( RC_SER_TICK_FAILED );
   }

   return ( RC_OK );
}


/******************************************************************************
 * Function Name: closeRxDma
 *
 * Description  : Close the RxDma process
 *
 * Inputs:        dev - device (uart) to open up
 *
 * Outputs:       None
 *
 * Returns:       RC_OK - Success
 *                ... various error codes for problems
 *
 * Notes:         None
 *
 * Context:       May be called from interrupt or task context.
 *
 *****************************************************************************/
static int32 closeRxDma ( int32 dev )
{
   int32 chan;
   int32 RetCode;

   /* check to see if DMA channel is enabled for this device */
   if (DMA_DISABLED == RxDmaChannel[dev])
   {
      return ( RC_OK );
   }
   chan = RxDmaChannel[dev];

   /* check to see if DMA request is defined for this device */
   if (DMA_DISABLED == RxDmaRequest[dev])
   {
      return ( RC_OK );
   }

   /* Destroy the tick timer */
   RetCode = tick_stop(tickRxDma[dev]);
   RetCode |= tick_destroy(tickRxDma[dev]);
   tickRxDma[dev] = 0;

   /* Clear/Disable DMA */
   *(LPREG)DMA_STATUS_REG_CH(chan) = 0x7E ; /* clear bits 1-6 */
   *(LPREG)DMA_CONTROL_REG_CH(chan) = 0;

   /* Free the buffer */
   mem_nc_free(pRxDmaBuf[dev]);
   LastRxAddr[dev] = 0;
   pRxDmaBuf[dev] = NULL;

   /* re-enable the RX ISR */
   ier_reg_value[dev] |= SER_INT_ENABLE_RXFIFO_MASK;

   return (RetCode);
}


/******************************************************************************
 * Function Name: cbRxDma
 *
 * Description  : Tick Timer Callback Function for RX DMA
 *
 * Inputs:        timer - tick timer id
 *                dwRev - tick parameter (set to value of 'dev' (uart number)
 *
 * Outputs:       None
 *
 * Returns:       None
 *
 * Notes:         None
 *
 * Context:       May be called from interrupt or task context.
 *
 *****************************************************************************/
static void cbRxDma (tick_id_t timer, void *dwRef)
{
   int32 i, j;
   static u_int8 buf[RXDMA_BUFFER_SIZE + 4];
   u_int32 CurrentRxAddr;
   int32 dwWordCount;
   int32 dev, chan;

   dev = (int32) dwRef;
   chan = RxDmaChannel[dev];
   CurrentRxAddr = *(LPREG)DMA_DSTADDR_REG_CH(chan) ;

   /* Check if anything to do */
   if (CurrentRxAddr == LastRxAddr[dev])
   {
      return;
   }

   /* Clear any pending dma interrupts. */
   *(LPREG)DMA_STATUS_REG_CH(chan) = 0x7E ; /* clear bits 1-6 */

   /* Now determine how many dma buffer entries we need to process */
   if( CurrentRxAddr < LastRxAddr[dev] )
   {
      dwWordCount =
            (CurrentRxAddr + (RXDMA_BUFFER_SIZE*4) - LastRxAddr[dev])/4 ;
   }
   else
   {
      dwWordCount = (CurrentRxAddr - LastRxAddr[dev]) / 4 ;
   }

   /* determine the current offset into to the buffer to start
    * reading from, then read the data */
   j = ( LastRxAddr[dev] - (unsigned int)pRxDmaBuf[dev] ) / 4 ;

   for (i = 0; i < dwWordCount; i++)
   {
      buf[i] = (unsigned char)pRxDmaBuf[dev][j++] ;
      if (j == RXDMA_BUFFER_SIZE)
      {
         j = 0;
      }
   }

   cbfn[dev]->rxHandler( i, (char *)buf );

   LastRxAddr[dev] = CurrentRxAddr;
}


/******************************************************************************
 * Function Name: openHandshake
 *
 * Description  : local function ... initialize the GPIO handshake leads
 *
 * Inputs:        dev ... which device (UART1, 2, or 3)
 *
 * Outputs:       None
 *
 * Returns:       RC_OK - success
 *                ... various error codes on failure
 *
 * Notes:         None
 *
 * Context:       May be called from task context.
 *
 *****************************************************************************/
static int32 openHandshake( int32 dev )
{
   CNXT_GPIO_STATUS eRetCode;

   if (ser_FlowControlIn[dev] != GPIO_INVALID)
   {
      eRetCode = cnxt_gpio_set_int_edge(ser_FlowControlIn[dev], BOTH_EDGES);
      if (eRetCode != CNXT_GPIO_STATUS_OK)
      {
         return (RC_SER_GPIO_ERR_BASE + (int32) eRetCode);
      }


      eRetCode = cnxt_gpio_int_register_isr
            (ser_FlowControlIn[dev],
            (PFNISR)isrHandshake,
            FALSE,
            FALSE,
            &handshake_chained_isr);
      if (eRetCode != CNXT_GPIO_STATUS_OK)
      {
         return (RC_SER_GPIO_ERR_BASE + (int32) eRetCode);
      }


      eRetCode = cnxt_gpio_int_enable(ser_FlowControlIn[dev]);
      if (eRetCode != CNXT_GPIO_STATUS_OK)
      {
         return (RC_SER_GPIO_ERR_BASE + (int32) eRetCode);
      }
   }


   if (ser_FlowReadyIn[dev] != GPIO_INVALID)
   {
      eRetCode = cnxt_gpio_set_int_edge(ser_FlowReadyIn[dev], BOTH_EDGES);
      if (eRetCode != CNXT_GPIO_STATUS_OK)
      {
         return (RC_SER_GPIO_ERR_BASE + (int32) eRetCode);
      }


      eRetCode = cnxt_gpio_int_register_isr
            (ser_FlowReadyIn[dev],
            (PFNISR)isrHandshake,
            FALSE,
            FALSE,
            &handshake_chained_isr);
      if (eRetCode != CNXT_GPIO_STATUS_OK)
      {
         return (RC_SER_GPIO_ERR_BASE + (int32) eRetCode);
      }


      eRetCode = cnxt_gpio_int_enable(ser_FlowReadyIn[dev]);
      if (eRetCode != CNXT_GPIO_STATUS_OK)
      {
         return (RC_SER_GPIO_ERR_BASE + (int32) eRetCode);
      }
   }


   if (ser_RingIndicateIn[dev] != GPIO_INVALID)
   {
      eRetCode = cnxt_gpio_set_int_edge(ser_RingIndicateIn[dev], BOTH_EDGES);
      if (eRetCode != CNXT_GPIO_STATUS_OK)
      {
         return (RC_SER_GPIO_ERR_BASE + (int32) eRetCode);
      }


      eRetCode = cnxt_gpio_int_register_isr
            (ser_RingIndicateIn[dev],
            (PFNISR)isrHandshake,
            FALSE,
            FALSE,
            &handshake_chained_isr);
      if (eRetCode != CNXT_GPIO_STATUS_OK)
      {
         return (RC_SER_GPIO_ERR_BASE + (int32) eRetCode);
      }


      eRetCode = cnxt_gpio_int_enable(ser_RingIndicateIn[dev]);
      if (eRetCode != CNXT_GPIO_STATUS_OK)
      {
         return (RC_SER_GPIO_ERR_BASE + (int32) eRetCode);
      }
   }


   if (ser_CarrierDetectIn[dev] != GPIO_INVALID)
   {
      eRetCode = cnxt_gpio_set_int_edge(ser_CarrierDetectIn[dev], BOTH_EDGES);
      if (eRetCode != CNXT_GPIO_STATUS_OK)
      {
         return (RC_SER_GPIO_ERR_BASE + (int32) eRetCode);
      }


      eRetCode = cnxt_gpio_int_register_isr
            (ser_CarrierDetectIn[dev],
            (PFNISR)isrHandshake,
            FALSE,
            FALSE,
            &handshake_chained_isr);
      if (eRetCode != CNXT_GPIO_STATUS_OK)
      {
         return (RC_SER_GPIO_ERR_BASE + (int32) eRetCode);
      }


      eRetCode = cnxt_gpio_int_enable(ser_CarrierDetectIn[dev]);
      if (eRetCode != CNXT_GPIO_STATUS_OK)
      {
         return (RC_SER_GPIO_ERR_BASE + (int32) eRetCode);
      }
   }

   return ( RC_OK );
} /* open_handshake */


/******************************************************************************
 * Function Name: close_control
 *
 * Description  : local function ... disable/close control/status leads
 *
 * Inputs:        dev ... UART device number
 *
 * Outputs:       None
 *
 * Returns:       RC_OK - success
 *                ... various error codes for failure
 *
 * Notes:         None
 *
 * Context:       May be called from task context.
 *
 *****************************************************************************/
static int32 closeHandshake( int32 dev )
{
   CNXT_GPIO_STATUS eRetCode;

   if (ser_FlowControlIn[dev] != GPIO_INVALID)
   {
      eRetCode = cnxt_gpio_int_disable(ser_FlowControlIn[dev]);
      if (eRetCode != CNXT_GPIO_STATUS_OK)
      {
         return (RC_SER_GPIO_ERR_BASE + (int32) eRetCode);
      }
   }


   if (ser_FlowReadyIn[dev] != GPIO_INVALID)
   {
      eRetCode = cnxt_gpio_int_disable(ser_FlowReadyIn[dev]);
      if (eRetCode != CNXT_GPIO_STATUS_OK)
      {
         return (RC_SER_GPIO_ERR_BASE + (int32) eRetCode);
      }
   }


   if (ser_RingIndicateIn[dev] != GPIO_INVALID)
   {
      eRetCode = cnxt_gpio_int_disable(ser_RingIndicateIn[dev]);
      if (eRetCode != CNXT_GPIO_STATUS_OK)
      {
         return (RC_SER_GPIO_ERR_BASE + (int32) eRetCode);
      }
   }


   if (ser_CarrierDetectIn[dev] != GPIO_INVALID)
   {
      eRetCode = cnxt_gpio_int_disable(ser_CarrierDetectIn[dev]);
      if (eRetCode != CNXT_GPIO_STATUS_OK)
      {
         return (RC_SER_GPIO_ERR_BASE + (int32) eRetCode);
      }
   }

   return ( RC_OK );
} /* control_close */


/******************************************************************************
 * Function Name: isrHandshake
 *
 * Description  : local function ... control/status GPIO interrupt
 *
 * Inputs:        None
 *
 * Outputs:       None
 *
 * Returns:       RC_ISR_HANDLED       If ISR valid and handled
 *                RC_ISR_NOTHANDLED    If ISR not for us
 *
 * Notes:         None
 *
 * Context:       May be called from interrupt context
 *
 *****************************************************************************/
static int32 isrHandshake( u_int32 id, bool bFIQ, PFNISR *pfnChain)
{
   CNXT_GPIO_STATUS eRetCode;
   int32 dev;

   for (dev = 0; dev < 3; dev++)
   {
      /* check to see if interrupt handler is installed yet */
      if (cbfn[dev] == NULL)
      {
         continue;
      }

      eRetCode = cnxt_gpio_int_query_id(ser_FlowReadyIn[dev], id) ;
      if ( CNXT_GPIO_STATUS_OK == eRetCode )
      {
         if (cbfn[dev]->flowReadyHandler)
         {
            bool bValue = FALSE;
            cnxt_gpio_get_level(ser_FlowReadyIn[dev], &bValue);
            cbfn[dev]->flowReadyHandler(bValue);
         }
         return (RC_ISR_HANDLED);
      }

      eRetCode = cnxt_gpio_int_query_id(ser_FlowControlIn[dev], id) ;
      if ( CNXT_GPIO_STATUS_OK == eRetCode )
      {
         if (cbfn[dev]->flowControlHandler)
         {
            bool bValue = FALSE;
            cnxt_gpio_get_level(ser_FlowControlIn[dev], &bValue);
            cbfn[dev]->flowControlHandler(bValue);
         }
         return (RC_ISR_HANDLED);
      }

      eRetCode = cnxt_gpio_int_query_id(ser_RingIndicateIn[dev], id) ;
      if ( CNXT_GPIO_STATUS_OK == eRetCode )
      {
         if (cbfn[dev]->ringHandler)
         {
            bool bValue = FALSE;
            cnxt_gpio_get_level(ser_RingIndicateIn[dev], &bValue);
            if (!bValue)
            {
               cbfn[dev]->ringHandler();
            }
         }
         return (RC_ISR_HANDLED);
      }

      eRetCode = cnxt_gpio_int_query_id(ser_CarrierDetectIn[dev], id) ;
      if ( CNXT_GPIO_STATUS_OK == eRetCode )
      {
         if (cbfn[dev]->cdHandler)
         {
            bool bValue = FALSE;
            cnxt_gpio_get_level(ser_CarrierDetectIn[dev], &bValue);
            cbfn[dev]->cdHandler(bValue);
         }
         return (RC_ISR_HANDLED);
      }
   }


   *pfnChain = handshake_chained_isr;
   return (RC_ISR_NOTHANDLED);
} /* control_isr */


/******************************************************************************
 * Function Name: setHandshake
 *
 * Description  : Set handshake lead, checking for valid GPIO definition
 *
 * Inputs:        uGPIO -  GPIO Pin Descriptor
 *                bValue - Boolean value to set GPIO handshake lead to
 *
 * Outputs:       None
 *
 * Returns:       RC_OK - Successfully set the GPIO signal
 *                RC_SER_INVALID_HANDSHAKE - Handshake descriptor invalid
 *                RC_SER_GPIO_ERR_BASE + ... - GPIO API Error Code returned
 *
 * Notes:         Checks for valid definition of uGPIO, then calls the
 *                cnxt gpio API function
 *
 * Context:       May be called from interrupt or task context.
 *
 *****************************************************************************/
static int32 setHandshake ( u_int32 uGPIO, bool bValue )
{
   int32 RetCode;
   CNXT_GPIO_STATUS eRetCode;

   if (uGPIO != GPIO_INVALID)
   {
      RetCode = RC_OK;
      eRetCode = cnxt_gpio_set_output_level(uGPIO, bValue);
      if (eRetCode != CNXT_GPIO_STATUS_OK)
      {
         RetCode = RC_SER_GPIO_ERR_BASE + (int32) eRetCode;
      }
   }
   else
   {
      RetCode = RC_SER_INVALID_HANDSHAKE;
   }

   return (RetCode);
} /* setHandshake */


/******************************************************************************
 * Function Name: getHandshake
 *
 * Description  : Get handshake lead, checking for valid GPIO definition
 *
 * Inputs:        uGPIO -  GPIO Pin Descriptor
 *                pbValue - Pointer Boolean pet GPIO value into
 *
 * Outputs:       None
 *
 * Returns:       RC_OK - Successfully read the GPIO signal
 *                RC_SER_INVALID_HANDSHAKE - Handshake descriptor invalid
 *                RC_SER_GPIO_ERR_BASE + ... - GPIO API Error Code returned
 *
 * Notes:         Checks for valid definition of uGPIO, then calls the
 *                cnxt gpio API function
 *
 * Context:       May be called from interrupt or task context.
 *
 *****************************************************************************/
static int32 getHandshake ( u_int32 uGPIO, bool *pbValue )
{
   int32 RetCode;
   CNXT_GPIO_STATUS eRetCode;

   if (uGPIO != GPIO_INVALID)
   {
      RetCode = RC_OK;
      eRetCode = cnxt_gpio_get_level(uGPIO, pbValue);
      if (eRetCode != CNXT_GPIO_STATUS_OK)
      {
         RetCode = RC_SER_GPIO_ERR_BASE + (int32) eRetCode;
      }
   }
   else
   {
      RetCode = RC_SER_INVALID_HANDSHAKE;
   }

   return (RetCode);
} /* setHandshake */


/****************************************************************************
 * Modifications:
 * $Log: 
 *  10   mpeg      1.9         9/22/03 3:30:24 PM     Bobby Bradford  SCR(s) 
 *        7418 :
 *        Add check to beginning of the isrHandshake process to make sure
 *        that we have isr handler installed for the given device before we
 *        start processing the GPIO settings for the device.
 *        Also changed the sense of RING indicated to be the trailing edge
 *        (ie. HIGH to LOW (boolean) edge of the signal)
 *        
 *  9    mpeg      1.8         9/4/03 5:47:38 PM      Bobby Bradford  SCR(s) 
 *        7418 :
 *        Major overhaul of the internal UART support functions ...
 *        (primarily added for serial modem API support)
 *        1) Fully integrate Rx DMA support for all DMA-Capable
 *        internal UARTS
 *        2) Fully integrate support for GPIO handshaking signals,
 *        including RING INDICATE and CARRIER DETECT.
 *        3) Re-worked the file to be more compatible with Conexant
 *        STB programming rules and guidelines.
 *        
 *  8    mpeg      1.7         7/16/03 2:53:12 PM     Angela Swartz   SCR(s) 
 *        6969 :
 *        put #if around usage of GPIO code to conditionally compile the code 
 *        out when the GPIO is not defined in the hw config file
 *        
 *  7    mpeg      1.6         6/24/03 3:35:46 PM     Miles Bintz     SCR(s) 
 *        6822 :
 *        added initialization value to remove warning in release build
 *        
 *  6    mpeg      1.5         4/30/03 12:22:26 PM    Larry Wang      SCR(s) 
 *        6124 :
 *        Re-write setFlowControl() to temporarily fix data abort problem 
 *        during the start up of vxworks.
 *        
 *  5    mpeg      1.4         1/23/03 3:27:56 PM     Bobby Bradford  SCR(s) 
 *        5103 :
 *        Modified flow control code to use newly defined General GPIO
 *        routines/api
 *        
 *  4    mpeg      1.3         12/12/02 5:19:00 PM    Tim White       SCR(s) 
 *        5157 :
 *        Replaced IF (UART_TYPE == UART_WABASH)  to  IF (UART_TYPE <> 
 *        UART_COLORAD)
 *        so this will work on future chips as the default Wabash case.
 *        
 *        
 *  3    mpeg      1.2         10/7/02 3:57:50 PM     Bobby Bradford  SCR(s) 
 *        4726 :
 *        Added initialization of the flow-control leads, so that
 *        the port will startup properly
 *        Added test code for the RX DMA functionality that is
 *        provided for UART1 and UART2.  Specifically, this code
 *        was desgined to support a serial test program running on
 *        UART1 on a HONDO/ABILENE IRD.  It is conditional, and
 *        should have no impact on the operation of the serial ports.
 *        It is checked in to retain the memory of how to configure 
 *        the DMA channels for serial rx dma.
 *        
 *  2    mpeg      1.1         10/1/02 5:05:22 PM     Bobby Bradford  SCR(s) 
 *        4726 :
 *        Added support for flow control (defines in CFG files will
 *        enable this ...
 *        Reworked the ISR routine, to work around the Colorado/Hondo
 *        issue of interrupt levels not being encoded properly in the
 *        interrupt level register
 *        
 *  1    mpeg      1.0         12/10/01 8:28:22 PM    Miles Bintz     
 * $
 * 
 *    Rev 1.9   22 Sep 2003 14:30:24   bradforw
 * SCR(s) 7418 :
 * Add check to beginning of the isrHandshake process to make sure
 * that we have isr handler installed for the given device before we
 * start processing the GPIO settings for the device.
 * Also changed the sense of RING indicated to be the trailing edge
 * (ie. HIGH to LOW (boolean) edge of the signal)
 * 
 *    Rev 1.8   04 Sep 2003 16:47:38   bradforw
 * SCR(s) 7418 :
 * Major overhaul of the internal UART support functions ...
 * (primarily added for serial modem API support)
 * 1) Fully integrate Rx DMA support for all DMA-Capable
 * internal UARTS
 * 2) Fully integrate support for GPIO handshaking signals,
 * including RING INDICATE and CARRIER DETECT.
 * 3) Re-worked the file to be more compatible with Conexant
 * STB programming rules and guidelines.
 *
 *    Rev 1.7   16 Jul 2003 13:53:12   swartzwg
 * SCR(s) 6969 :
 * put #if around usage of GPIO code to conditionally compile the code out when the GPIO is not defined in the hw config file
 *
 *    Rev 1.6   24 Jun 2003 14:35:46   bintzmf
 * SCR(s) 6822 :
 * added initialization value to remove warning in release build
 *
 *    Rev 1.5   30 Apr 2003 11:22:26   wangl2
 * SCR(s) 6124 :
 * Re-write setFlowControl() to temporarily fix data abort problem during the start up of vxworks.
 *
 *    Rev 1.4   23 Jan 2003 15:27:56   bradforw
 * SCR(s) 5103 :
 * Modified flow control code to use newly defined General GPIO
 * routines/api
 *
 *    Rev 1.3   12 Dec 2002 17:19:00   whiteth
 * SCR(s) 5157 :
 * Replaced IF (UART_TYPE == UART_WABASH)  to  IF (UART_TYPE <> UART_COLORAD)
 * so this will work on future chips as the default Wabash case.
 *
 *
 *    Rev 1.2   07 Oct 2002 14:57:50   bradforw
 * SCR(s) 4726 :
 * Added initialization of the flow-control leads, so that
 * the port will startup properly
 * Added test code for the RX DMA functionality that is
 * provided for UART1 and UART2.  Specifically, this code
 * was desgined to support a serial test program running on
 * UART1 on a HONDO/ABILENE IRD.  It is conditional, and
 * should have no impact on the operation of the serial ports.
 * It is checked in to retain the memory of how to configure
 * the DMA channels for serial rx dma.
 *
 *    Rev 1.1   01 Oct 2002 16:05:22   bradforw
 * SCR(s) 4726 :
 * Added support for flow control (defines in CFG files will
 * enable this ...
 * Reworked the ISR routine, to work around the Colorado/Hondo
 * issue of interrupt levels not being encoded properly in the
 * interrupt level register
 *
 *    Rev 1.0   10 Dec 2001 20:28:22   bintzmf
 * SCR(s) 2933 :
 *
 *
 *    Rev 1.1   14 Mar 2001 11:01:36   kroescjl
 * #1430 - added volatile to make it work when optimization with debug
 *
 *    Rev 1.0   Sep 11 2000 16:19:32   kroescjl
 * Initial revision.
 *
 *    Rev 1.0   Sep 03 2000 19:36:50   kroescjl
 * Initial revision.
 *
 ****************************************************************************/

