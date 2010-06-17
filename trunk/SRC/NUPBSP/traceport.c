/****************************************************************************/
/*                   CONEXANT PROPRIETARY AND CONFIDENTIAL                  */
/*                       SOFTWARE FILE/MODULE HEADER                        */
/*                 Conexant Systems Inc. (c) 1998, 1999, 2000               */
/*                               Austin, TX                                 */
/*                            All Rights Reserved                           */
/****************************************************************************/
/*
 * Filename:      traceport.c
 *                  
 * Description:   This contains a mapping layer between nucleus and the
 *                serial port used for tracing.  Actually, it implements a
 *                buffered printf, but it is used for trace output.
 *                Printf and vprintf are used for trace output by nucleus
 *                bsp.  As currently implemented, only output is supported,
 *                but with some work it could be made to support input as
 *                well.  Also, there is no flow control.
 *
 * Author:        kroesche
 *
 ****************************************************************************/
/* $Id: traceport.c,v 1.39, 2004-02-18 21:37:58Z, Bobby Bradford$
 ****************************************************************************/

/* includes */

#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include "stbcfg.h"
#include "basetype.h"
#include "kal.h"
#include "hwlib.h"
#include "nup.h"
#include "llserial.h"

/* defines */

/* the port data rate */
#ifdef OPENTV_12
  #define SER_RATE        57600    /* Sky gamepad requires the serial port data rate to be 57600 */
#else
  #define SER_RATE        DEFAULT_SERIAL_BAUD_RATE
#endif
/************************************************************************/
/* Trace Output Note:                                                   */
/*                                                                      */
/* Change the definition of SER_PORT to reroute the trace output to the */
/* desired port. Choices are:                                           */
/*                                                                      */
/*   LLSER_DBGCOMM   - debug comms channel of Multi-ICE. Output appears */
/*                     in the debugger console window.                  */
/*   LLSER_UART3     - UART #3 which is normally wired to the onboard   */
/*                     serial port connector.                           */
/*   LLSER_TELEGRAPH - the 16550 UART on the telegraph adapter (only    */
/*                     available with Klondike IRD)                     */
/************************************************************************/

#if TRACE_PORT == INTERNAL_UART1

/* mod by sunbey, 2004/12/15                        */
/* in auto test mode, trace output should be closed */
#if AUTO_TEST_MODE==AUTOTEST_SERIAL
   #define SER_PORT LLSER_UART2
#else
   #define SER_PORT LLSER_UART1   
#endif


#elif TRACE_PORT == INTERNAL_UART2
   #define SER_PORT LLSER_UART2

#elif TRACE_PORT == INTERNAL_UART3
   #define SER_PORT LLSER_UART3

#elif TRACE_PORT == TELEGRAPH_UART1
   #define SER_PORT LLSER_TELEGRAPH

#elif TRACE_PORT == TRACE_MULTIICE
   #define SER_PORT LLSER_DBGCOMM
   
#elif TRACE_PORT == TRACE_MEMORY
   #define SER_PORT LLSER_MEMORY
      
#else
   #define SER_PORT LLSER_NONE
   
#endif
   
   /* these must be power of 2 */
#define TXQUEUESIZE     4096
#define RXQUEUESIZE     2048

/* Maximum length of output string vprintf can generate */
#define MAX_VPRINTF_STRING_LEN 256

/* Maximum lenght of input string read at a time by scanf */
#define MAX_SCANF_STRING_LEN 256

/* time for vprintf to block if output fifo is full */
#define SER_VPRINTF_BLOCK_DELAY 20

/*Enclose in #ifdefs to avoid compile warnings on Release builds */
/* locals */
static LLSERCALLBACKS serfns;      /* low level serial callback functions */
static PLLSERINTERFACE ser = NULL;
   /* fifo variables */
static char txQueue[TXQUEUESIZE];
#if !(defined OPENTV_12)
static char rxQueue[RXQUEUESIZE];
#endif
static unsigned int txQueueCount;
static unsigned int rxQueueFree;
static unsigned int txOutdex;
static unsigned int txIndex;
static unsigned int rxOutdex;
static unsigned int rxIndex;
unsigned long data_length = 1;
unsigned long wait_data;
unsigned char bTraceEnable=1;
/* forward declarations */
#ifdef OPENTV_12 
extern void GamepadRxHandler( unsigned int count, char* buf );
#else /* defined OPENTV_12 */
static void rxHandler( unsigned int count, char* buf );
#endif
static unsigned int txHandler( unsigned int count, char* buf );
void putstringNoQ( char* str );

char* advance( char* bufp);
int vsscanf( char *buf, const char *format, va_list arg);


/* global variable for Wabash/Nucleus, only do this for SDT but not ADS */
/* because SDT uses the stdio.h in ..\NUPINCL directory.                */
/*#if __ARMCC_VERSION < 100000 */
/* FILE __stdin;               */
/*#endif                       */


/*
 * void TracePort_Init( void )
 *
 * parameters:
 *    none
 *
 * returns:
 *    none
 *
 * Initializes the serial port handler to be used for trace output.  Sets
 * up the handler callbacks and enables the port interrupt.
 */
void TracePort_Init() {

   /* set up the serial port handler */
   memset( &serfns, 0, sizeof( serfns ) );   /* null out unused callbacks */
#ifdef OPENTV_12
   /* grab the Rx interrupt handler for Gamepad controller */
   serfns.rxHandler = GamepadRxHandler;
#else
   serfns.rxHandler = rxHandler;
#endif   
   serfns.txHandler = txHandler;   
   ser = llser_open( SER_PORT, &serfns );
   if( !ser )
   {
      return;
   }
   if( !ser->config( SER_RATE, 8, 1, 0, 0 ) )
   {
      return;
   }

   /* initialize the fifo */
   txIndex = txOutdex = 0;
   txQueueCount = 0;
   rxIndex = rxOutdex = 0;
   rxQueueFree = RXQUEUESIZE;

   putstringNoQ( "\n\n\n******\n" );
   putstringNoQ( "Trace output port opened\n\n" );
   /* enable serial interrupt */
   ser->enable( TRUE );
}

/*
 * static unsigned int txHandler( unsigned int count, char* buf )
 *
 * parameters:
 *    count - the maximum numbers of characters that should be
 *          put in buf
 *    buf - pointer to storage where any data to be sent should
 *          be placed.  Up to the number of bytes specified by count
 *
 * returns:
 *    the actual number of bytes that were placed in buf.  Can be 0.
 *    Must not be greater than count.
 *
 * Called by the low level serial driver whenever the transmitter is
 * able to accept more data.  The driver will determine the maximum
 * number of bytes it can accept and place in count.  The client should
 * copy bytes it wants to send into buf (up to count bytes) and then return
 * the actual number it copied.  The serial driver will then copy the
 * new data from buf into its hardware output buffer.  This is executing
 * in an interrupt context.
 */
static unsigned int txHandler( unsigned int count, char* buf )
{
   unsigned int i;

   /* determine proper number of bytes to pass back to serial */
   if( txQueueCount < count )
      count = txQueueCount;

   /* copy the bytes from output fifo to the serial output buffer */
   for( i = 0; i < count; i++ )
   {
      *(buf++) = txQueue[txOutdex++];
      txQueueCount--;
      txOutdex &= (TXQUEUESIZE-1);
   }

   return( count );
}

/*
 * static void rxHandler( unsigned int count, char* buf )
 *
 * parameters:
 *    count - the number of bytes available in buf
 *    buf - storage containing received serial data
 *
 * returns:
 *    none
 *
 * Called by the low level serial driver whenever the receiver has new
 * data.  The received data is placed in buf and count contains the number
 * of bytes.  The client should copy the received data out of buf.  It is
 * assumed that the client can take all the received data.  The low level
 * serial driver makes no provision for buffering the received data.
 * This is executing in an interrupt context.
 */

#ifndef OPENTV_12
static void rxHandler( unsigned int count, char* buf )
{
   /* stuff any incoming data into the fifo */
   while( rxQueueFree && count-- )
   {
      rxQueue[rxIndex++] = *(buf++);
      rxIndex &= (RXQUEUESIZE-1);
      rxQueueFree--;
   }
}
#endif

/*
 * static int putstringQ( char* str )
 *
 * parameters:
 *    str - null terminated string to be copied to trace output
 *
 * returns:
 *    non-zero if data copied okay, zero if no room in FIFO
 *
 * Copies the null terminated string into the output fifo, where it will
 * eventually be transmitted on the trace serial port.
 */
static int putstringQ( char* str )
{
   size_t size;
   bool crit;

   /* make sure the string will fit in the queue, the actual needed space for the string should be (strlen(str) + 1) which is '\r' added after '\n' */
   size = strlen( str );
   if( size + 1 > (TXQUEUESIZE - txQueueCount) )
      return( 0 );

   crit = critical_section_begin();
   /* copy the string to the output fifo */
   /* convert all \n to \n\r */
   while( *str )
   {
      if( *str == '\n' )
      {
         txQueue[txIndex++] = '\r';
         txIndex &= (TXQUEUESIZE-1);
         txQueueCount++;
      }
      txQueue[txIndex++] = *str;
      txIndex &= (TXQUEUESIZE-1);
      txQueueCount++;
      str++;
   }
   /* tell serial port there is new output data */
   if(ser && ser->txstart)
     ser->txstart();
   critical_section_end( crit );
   return( 1 );
}

/*
 * static void putstringNoQ( char* str )
 *
 * parameters:
 *    str - null terminated string to be copied to trace output
 *
 * returns:
 *    none
 *
 * Copies the null terminated string to the output using polling.  This
 * means that execution is tied up here until the entire string has been
 * sent.  Use of this function should not be intermixed with the buffered
 * operation because it could confuse the serial port driver.  This function
 * is suitable for output in a serious error situation or during
 * before interrupts are used.
 */
void putstringNoQ( char* str )
{
   while( *str )
   {
      if( *str == '\n' )
         ser->serputchar( '\r', 1 );
      ser->serputchar( *str, 1 );
      str++;
   }
}

/* (per miles...) Print is required to satisfy a function call in NDS */
int      Print(char *fmt, ...)
{
   int ret = 0;
#if (defined DEBUG) || (defined DRIVER_INCL_TESTH) || (ENABLE_TRACE_IN_RELEASE == YES)
   va_list         ap;

   va_start(ap, fmt);
   ret = vprintf( fmt, ap );
   va_end( ap );
#endif
   return( ret ); 
}

/*
 * int printf( const char* format, ... )
 *
 * parameters:
 *    format - the typical printf format string
 *    ... - variable number of replacement parameters
 *
 * returns:
 *    number of characters printed
 *
 * This attempts to be a standard printf.  The output will be buffered to
 * the serial output fifo, so it will return before the string actually
 * appears on the serial output.  It just converts ... into an arg list
 * and then calls vprintf.
 */
int printf( const char* format, ... )
{
   int ret =0;
   va_list arg;

   va_start( arg, format );
   ret = vprintf( format, arg );
   va_end( arg );
   
   return( ret );
}

int _printf( const char* format, ... )
{
   int ret = 0;
   va_list arg;

   va_start( arg, format );
   ret = vprintf( format, arg );
   va_end( arg );
   
   return( ret );
}

/*
 * int vprintf( const char* format, va_list arg )
 *
 * parameters:
 *    format - the typical printf format string
 *    arg - argument list
 *
 * returns:
 *    number of characters printed
 *
 * This attempts to be a standard vprintf.  The output will be buffered to
 * the serial output fifo, so it will return before the string actually
 * appears on the serial output.  All the real work is done by vsprintf
 * which is part of the run time library.  We assume that there are no
 * reentrancy problems there.
 * This function will not return until the data has been successfully
 * delivered to the low level FIFO (it will block until there is room)
 * THIS FUNCTION IS NOT RE-ENTRANT.
 */
int vprintf( const char* format, va_list arg )
{
   int ret = 0;
   char printstr[MAX_VPRINTF_STRING_LEN];

   ret = vsprintf( printstr, format, arg );
   while( !putstringQ( printstr ) )
      task_time_sleep( SER_VPRINTF_BLOCK_DELAY ); /* keep trying until string is delivered */
   return( ret );
}

/****************************************************************************
 *
 * Function:    static int getcharQ(void)
 *
 *
 * Abstract:    Returns the next character in line from the rx fifo
 *
 * Arguments:   None
 *
 * Returns:     The character in the fifo pointed to by rxOutdex
 *
 * Output:      Echos the character back, unless it is \r in which case it
 *              prints \n. \r is returned as is.
 *
 ****************************************************************************/ 
#if ((defined DEBUG) || (defined DRIVER_INCL_TESTH) || (ENABLE_TRACE_IN_RELEASE == YES)) && !(defined OPENTV_12) 
static int getcharQ(void)
{
    char temp[2] = {'\0','\0'};
    char *ptemp = temp;
    /* Wait for information to come into the fifo */
    while(rxIndex == rxOutdex)
        task_time_sleep(10);

    *ptemp = rxQueue[rxOutdex++];
    if(*ptemp != '\r')
        putstringNoQ(temp);
    else
        putstringNoQ("\n");

    rxOutdex &= (RXQUEUESIZE -1);
    rxQueueFree++;

    return (int)*ptemp;

} /* getcharQ() */
#endif

/****************************************************************************
 *
 * Function:    static int getcharQ(void)
 *
 *
 * Abstract:    Returns the next character in line from the rx fifo
 *
 * Arguments:   None
 *
 * Returns:     The character in the fifo pointed to by rxOutdex
 *
 * Output:      Echos the character back, unless it is \r in which case it
 *              prints \n. \r is returned as is.
 *
 ****************************************************************************/ 
#if ((defined DEBUG) || (defined DRIVER_INCL_TESTH) || (ENABLE_TRACE_IN_RELEASE == YES)) && !(defined OPENTV_12) 
int getcharQNoEcho(void)
{
    char temp[2] = {'\0','\0'};
    char *ptemp = temp;
    /* Wait for information to come into the fifo */
    while(rxIndex == rxOutdex)
        task_time_sleep(10);

    *ptemp = rxQueue[rxOutdex++];

    rxOutdex &= (RXQUEUESIZE -1);
    rxQueueFree++;

    return (int)*ptemp;

} /* getcharQ() */
#endif

/************************************************************************************
 *
 * Function:    static void getstringQ( char* str )
 *
 * Abstract:    Uses getcharQ to copy the input fifo from the trace serial 
 *              port to str.
 *              The client is responsibile to allocate enough memory
 *
 * Arguments:   str - null terminated string to be used to store the contents 
 *              from the serial input fifo
 *
 * Returns:     None
 *
 *************************************************************************************/
#if ((defined DEBUG) || (defined DRIVER_INCL_TESTH)|| (ENABLE_TRACE_IN_RELEASE == YES)) && !(defined OPENTV_12)
static void getstringQ(char *str)
{
    
    while((*str = getcharQ() ) != '\r')
    {
      if( *str == '\b') /* Discard backspace and the previous char */
        str--;
      else
        str++;
    }

    *str = '\0';

}/* getstringQ() */
#endif

/************************************************************************************
 *
 * Function:    static void getstringNoQ( char* str )
 *
 * Abstract:    Retrieves a string using polling.  
 *              The client is responsibile to allocate enough memory
 *               
 * Arguments:   str - null terminated string to be used to store the contents 
 *              from the serial input fifo
 *
 * Returns:     None
 *
 *************************************************************************************/
void getstringNoQ(char *str)
{
#if (defined DEBUG) || (defined DRIVER_INCL_TESTH) || (ENABLE_TRACE_IN_RELEASE == YES)
    while((*str = ser->sergetchar(1)) != '\r')
    {
      if( *str == '\b') /* Discard backspace and the previous char */
        str--;
      else
        str++;
    }
    *str = '\0';
#endif

} /* getstringNoQ() */


/************************************************************************************
 *
 * Function:    char *gets(char* str)
 *
 * Abstract:    Reads the next input line into the s; 
 *              it replaces the newline with '\0'.
 *              It returns s, or NULL if end of file or error occurs.
 *              Uses getcharQ to copy the input fifo from the trace serial 
 *              port to str. 
 *              The client is responsibile to allocate enough memory
 *               
 * Arguments:   str - null terminated string to be used to store the contents 
 *              from the serial input fifo
 *
 * Returns:     None
 *
 *************************************************************************************/
char *gets(char *str)
{
#if ((defined DEBUG) || (defined DRIVER_INCL_TESTH)|| (ENABLE_TRACE_IN_RELEASE == YES)) && !(defined OPENTV_12) 
    
    while((*str = getcharQ() ) != '\r')
    {
      if( *str == '\b') /* Discard backspace and the previous char */
        str--;
      else
        str++;
    }

    *str = '\0';

#endif
    return str;
    
} /* gets() */


/*
 * int scanf( const char* format, ... )
 *
 * parameters:
 *    format - the typical scanf format string
 *    ... - variable number of replacement parameters
 *
 * returns:
 *    the number of fields successfully converted and assigned.
 *
 * This attempts to be a standard scanf.  The input will be buffered to
 * the serial input fifo.  It just converts ... into an arg list
 * and then calls vscanf.
 */
int scanf( const char* format, ... )
{
   int ret = 0;
#if ((defined DEBUG) || (defined DRIVER_INCL_TESTH) || (ENABLE_TRACE_IN_RELEASE == YES)) && !(defined OPENTV_12)
   va_list arg=NULL;
   static char buf[MAX_SCANF_STRING_LEN];

   va_start( arg, format );

   getstringQ(buf);
   ret=vsscanf(buf, format, arg);

   va_end(arg);
#endif
   return ret;
}
int _scanf( const char* format, ... )
{
   int ret =0;
#if ((defined DEBUG) || (defined DRIVER_INCL_TESTH) || (ENABLE_TRACE_IN_RELEASE == YES)) && !(defined OPENTV_12)
   va_list arg=NULL;
   char buf[MAX_SCANF_STRING_LEN];

   va_start( arg, format );

   getstringQ(buf);
   ret=vsscanf(buf, format, arg);

   va_end(arg);
#endif
   return ret;
}


/********************************************************
 *
 * Copies the contents of buf into the arguments in the
 *    variable length argument list.
 *
 * Only supports string, integer, float and double.  So
 *     format should only contain %s, %d, %x, %f and %lf.
 ********************************************************/
int vsscanf( char *buf, const char *format, va_list arg)
{
    
    const char*    fmtp;
    char*    bufp;
    char*    s_ptr;
    int*     i_ptr;
    double*  d_ptr;
    float*   f_ptr;
    int ret=0;

    bufp = buf;

    for (fmtp=format; *fmtp; fmtp++) {
        if (*fmtp == '%')
          switch (*++fmtp) {
            case 'd':
                i_ptr = va_arg(arg, int *);    /* advance arg */
                sscanf(bufp, "%d", i_ptr);
                bufp = advance(bufp);
                break;
            case 'f':
                f_ptr = va_arg(arg, float *);  /* advance arg */
                sscanf(bufp, "%f", f_ptr);
                bufp =advance(bufp);
                break;
            case 'l':
                d_ptr = va_arg(arg, double *);  /* advance arg */
                sscanf(bufp, "%lf", d_ptr);
                bufp = advance(bufp);
                break;
            case 's':
                s_ptr = va_arg(arg, char *);    /* advance arg */
                sscanf(bufp, "%s", s_ptr);
                bufp = advance(bufp);
                break;
            case 'x':
                i_ptr = va_arg(arg, int *);    /* advance arg */
                sscanf(bufp, "%x", i_ptr);
                bufp = advance(bufp);
                break;
            default:
                break;
        }
        ret++;
    }    
    return ret;
}  
/********************************************************
 * Advance buffer pointer to next valid character by
 *     1.  Skipping nonwhite characters.
 *     2.  Skipping white space.
 ********************************************************/
char* advance( char* bufp) {

    char* new_bufp = bufp;

    /* Skip over nonwhite space */
    while ((*new_bufp != ' ')  && (*new_bufp != '\t') &&
           (*new_bufp != '\n') && (*new_bufp != '\0'))
        new_bufp++;

    /* Skip white space */
    while ((*new_bufp == ' ')  || (*new_bufp == '\t') ||
           (*new_bufp == '\n') || (*new_bufp == '\0'))
        new_bufp++;

    return new_bufp;
}       

/********************************************************
 * Function:    fflush
 *
 * Arguments:   FILE    -   stream
 *
 * Returns:     0
 *
 * Note:    This is a just dummy routine since Nuclues has
 *          no buffer to flush.
 ********************************************************/
int fflush(FILE *pstream)
{
    int ret=0;

    return ret;
}


int SerialPutData(void* pData,unsigned int len)
{
	unsigned char *pOut=(unsigned char *)pData;
	bool crit;
	int size=len;
	if( size + 1 > (TXQUEUESIZE - txQueueCount) )
	   return( 0 );
	
	crit = critical_section_begin();
	/* copy the string to the output fifo */
	/* convert all \n to \n\r */
	while( size>0 )
	{
	   txQueue[txIndex++] = (char)*pOut;
	   txIndex &= (TXQUEUESIZE-1);
	   txQueueCount++;
	   pOut++;
	   size--;
	}
	/* tell serial port there is new output data */
	if(ser && ser->txstart)
	  ser->txstart();
	critical_section_end( crit );
	return( 1 );

}

int SerialGetData( void* pData,unsigned int len,int timeout )
{
	data_length = len;

	if(rxIndex <data_length)
	{
		return -1;
	 	if(sem_get(wait_data,10*1000) == 0)
	 	{
			memcpy(pData,rxQueue,len);
			memcpy(rxQueue,&rxQueue[len],rxIndex-len);
			//	memset(&rxQueue[rxIndex-data_length],0,rxIndex-len);
			rxQueueFree+=len;
			rxIndex-=len;
			return len;
	 	}
	}
	else
	{
			memcpy(pData,rxQueue,len);
			memcpy(rxQueue,&rxQueue[len],rxIndex-len);
			//	memset(&rxQueue[rxIndex-data_length],0,rxIndex-len);
			rxQueueFree+=len;
			rxIndex-=len;
			return len;
	}
	return -1;
}





/****************************************************************************
 * Modifications:
 * $Log: 
 *  40   mpeg      1.39        2/18/04 3:37:58 PM     Bobby Bradford  CR(s) 
 *        8414 : Add a new function to get a character from the DEBUG TRACE 
 *        port input buffer without echoing it back out to the trace port. 
 *        (used by the serial redirector for modem testing when there is only 
 *        one UART active in a box)
 *  39   mpeg      1.38        2/13/04 12:54:48 PM    Angela Swartz   CR(s) 
 *        8400 8399 : when not building opentv, the default serial baud rate is
 *         set to the value defined in the software config file(default is 
 *        115200)
 *  38   mpeg      1.37        12/17/03 3:54:30 PM    Dave Wilson     CR(s) 
 *        8101 8102 : Removed conditional compilation directives inside printf 
 *        and _printf to ensure that these functions are available when 
 *        building a release image too. Previously, they compiled to stubs 
 *        which did nothing if debug was disabled.
 *        
 *  37   mpeg      1.36        10/22/03 7:52:13 AM    Dave Wilson     CR(s): 
 *        7693 Removed ifdefs round various functions and variables to allow 
 *        vprintf to operate in release builds. This is required to support the
 *         new timestamp_message
 *        function in the TRACE module.
 *  36   mpeg      1.35        10/10/03 4:27:26 PM    Angela Swartz   include 
 *        the serial support for OTV12CTL in both debug and release builds to 
 *        support gamepad
 *  35   mpeg      1.34        9/18/03 5:00:18 PM     Bobby Bradford  SCR(s) 
 *        7418 :
 *        "LF" (ctrl-j)  was being replaced with "LF" "CR" (ctrl-j, ctrl-m)
 *        Swapped the order to be "CR" "LF" which is standard "DOS" format
 *        
 *  34   mpeg      1.33        9/16/03 5:09:06 PM     Angela Swartz   SCR(s) 
 *        7477 :
 *        remove weird character accidentally added to the log which caused a 
 *        build warning
 *        
 *  33   mpeg      1.32        9/16/03 4:22:56 PM     Angela Swartz   SCR(s) 
 *        7477 :
 *        Enable trace port if ENABLE_TRACE_IN_RELEASE == YES in sw config file
 *         when building release builds
 *        
 *  32   mpeg      1.31        9/15/03 2:35:58 PM     Angela Swartz   SCR(s) 
 *        7249 7250 :
 *        added code to support the Sky serial GAMEPAD
 *        
 *  31   mpeg      1.30        3/20/03 9:24:54 AM     Angela Swartz   added 
 *        support for format %x in vsscanf
 *        
 *  30   mpeg      1.29        2/13/03 12:48:22 PM    Matt Korte      SCR(s) 
 *        5479 :
 *        Removed old header reference
 *        
 *  29   mpeg      1.28        1/24/03 6:46:48 PM     Joe Kroesche    SCR(s) 
 *        5318 :
 *        forgot to block in wait loop, added sleep
 *        
 *  28   mpeg      1.27        1/24/03 5:26:12 PM     Joe Kroesche    SCR(s) 
 *        5318 :
 *        modified vprintf function to block if the fifo buffer is full.  This 
 *        will
 *        prevent trace output from being dropped in unbuffered configuration
 *        
 *  27   mpeg      1.26        1/24/03 10:23:06 AM    Angela Swartz   SCR(s) 
 *        5271 :
 *        added a fix to the trace repeating output problem when the trace 
 *        traffic is high
 *        
 *  26   mpeg      1.25        12/20/02 1:54:16 PM    Dave Wilson     SCR(s) 
 *        5204 :
 *        Added the ability to select trace to memory rather than UART.
 *        Fixed up putstringQ such that it won't jump to 0 if called before a 
 *        trace
 *        port has been initialised. This can happen if a runtime error occurs 
 *        during
 *        startup.
 *        
 *  25   mpeg      1.24        11/6/02 1:31:52 PM     Dave Wilson     SCR(s) 
 *        4907 :
 *        Removed __stdin declaration. This messed things up when using the ARM
 *         SDT
 *        2.51 C runtime headers.
 *        
 *  24   mpeg      1.23        6/24/02 3:12:36 PM     Holly Le        SCR(s) 
 *        4064 :
 *        Modified the checking of __ARMCC_VERSION to check against a value 
 *        since 
 *        both ADS and SDT have this predefined macro.
 *        
 *        
 *  23   mpeg      1.22        6/20/02 11:09:28 AM    Holly Le        SCR(s) 
 *        4064 :
 *        Added __ARMCC_VERSION to check for ADS or SDT before define __stdin 
 *        since
 *        SDT uses the stdio.h in ..\NUPINCL directory.
 *        
 *        
 *  22   mpeg      1.21        6/19/02 3:18:00 PM     Bob Van Gulick  SCR(s) 
 *        4064 :
 *        Added routine fflush() to fixed Nucleus/Wabash build error.
 *        
 *        
 *  21   mpeg      1.20        6/6/02 1:53:36 PM      Lucy C Allevato SCR(s) 
 *        3949 :
 *        Substituted "#ifdef DEBUG" with "#if (defined DEBUG) || (defined 
 *        DRIVER_INCL_TESTH)" so that io functions are available in Release 
 *        Mode for the Test Harness.
 *        
 *  20   mpeg      1.19        5/14/02 10:02:04 AM    Lucy C Allevato SCR(s) 
 *        3773 :
 *        Enclosed static global definitions and some functions available only 
 *        in DEBUG mode anyway into #ifdef DEBUG...#endif bloques.
 *        
 *  19   mpeg      1.18        4/29/02 12:33:22 PM    Billy Jackman   SCR(s) 
 *        3647 :
 *        Rearranged code for a for loop to avoid compiler warning.
 *        
 *  18   mpeg      1.17        4/25/02 5:18:22 PM     Lucy C Allevato SCR(s) 
 *        3625:
 *        Last fix broke the build for ADS, redefinition of getchar. Deleted
 *        
 *  17   mpeg      1.16        4/25/02 2:42:36 PM     Lucy C Allevato SCR(s) 
 *        3625 :
 *        Rewrote function getstringQ.
 *        Added functions: getstringNoQ, getcharQ, getchar and gets.
 *        Modified buffer size allocated by scanf and _scanf from 
 *        RXQUEUESIZE(2048) to MAX_SCANF_STRING_LEN(256).
 *        
 *  16   mpeg      1.15        12/21/01 9:53:24 AM    Miles Bintz     SCR(s) 
 *        2933 :
 *        Changed MULTIICE to TRACE_MULTIICE to match hwconfig file.
 *        
 *        
 *  15   mpeg      1.14        12/21/01 8:40:22 AM    Miles Bintz     SCR(s) 
 *        2933 :
 *        Merging wabash branch changes into main tree, includes fix for 
 *        trace/serial port defines
 *        
 *  14   mpeg      1.13        9/6/01 3:59:30 PM      Miles Bintz     SCR(s) 
 *        1801 2486 :
 *        Removed static declaration from putstringNoQ so that it can be used 
 *        in other_isr exception handling
 *        
 *        
 *  13   mpeg      1.12        8/27/01 3:44:24 PM     Angela          SCR(s) 
 *        2430 :
 *        Corrected a build error
 *        
 *  12   mpeg      1.11        8/7/01 3:37:56 PM      Angela          SCR(s) 
 *        2437 :
 *        For Hondo, use UART1 for the trace if the app is p93test
 *        
 *  11   mpeg      1.10        6/18/01 6:11:46 PM     Dave Wilson     SCR(s) 
 *        2071 :
 *        vprintf implementation included a hardcoded 128 byte output buffer. 
 *        Trace
 *        messages from OpenTV can, however, be up to 161 characters long so in
 *         some
 *        cases the buffer was overflowing and corrupting the stack. Increased 
 *        the size
 *        to 192 for safety.
 *        
 *  10   mpeg      1.9         5/4/01 3:30:00 PM      Tim White       DCS#1824,
 *         DCS31825 -- Critical Section Overhaul
 *        
 *  9    mpeg      1.8         4/9/01 4:16:04 PM      Dave Moore      
 *        Tracker#742 When using softmodem test driver, redirect serial
 *        debug output to DGBCOMMs channel.
 *        
 *  8    mpeg      1.7         11/3/00 4:12:28 PM     Dave Wilson     Changed 
 *        Klondike trace output default from Telegraph to onboard UART.
 *        
 *  7    mpeg      1.6         11/2/00 10:52:20 PM    Joe Kroesche    set debug
 *         comm channel as trace output if non-conexant modem test
 *        
 *  6    mpeg      1.5         10/27/00 2:45:50 PM    Angela          added 
 *        scanf, vsscanf and other functions related
 *        
 *  5    mpeg      1.4         10/8/00 3:19:04 PM     Joe Kroesche    commented
 *         out extra critical sections, added _print to make linker happy
 *        
 *  4    mpeg      1.3         9/29/00 12:51:28 PM    Dave Wilson     Changed 
 *        routing to ensure that, by default, all non-Conexant IRDs send
 *        trace to their onboard UART rather than Telegraph.
 *        
 *  3    mpeg      1.2         9/18/00 9:18:52 PM     Joe Kroesche    put in 
 *        conditional compile code to turn off serial port if not DEBUG
 *        
 *  2    mpeg      1.1         9/12/00 10:25:52 AM    Joe Kroesche    added 
 *        include to eliminate a warning
 *        
 *  1    mpeg      1.0         9/11/00 5:15:00 PM     Joe Kroesche    
 * $
 *
 ****************************************************************************/


