/****************************************************************************/
/*                   CONEXANT PROPRIETARY AND CONFIDENTIAL                  */
/*                       SOFTWARE FILE/MODULE HEADER                        */
/*                 Conexant Systems Inc. (c) 1998, 1999, 2000               */
/*                               Austin, TX                                 */
/*                            All Rights Reserved                           */
/****************************************************************************/
/*
 * Filename:       llserial.h
 *
 * Description:    Interface for low level serial driver.
 *
 * Author:         kroesche
 *
 ****************************************************************************/
/* $Header: llserial.h, 3, 9/22/03 3:21:32 PM, Bobby Bradford$
 ****************************************************************************/

#ifndef _LLSERIAL_H_
#define _LLSERIAL_H_

#ifndef NULL
#define NULL    ((void *)0)
#endif

   /* serial port default setups */
#define DEFAULT_RATE    38400
#define DEFAULT_BITS    8
#define DEFAULT_STOPS   1
#define DEFAULT_PARITY  0
#define DEFAULT_EVEN    0

/* these are all the possible serial devices */
typedef enum
{
   LLSER_NONE,
   LLSER_TELEGRAPH,  /* telegraph plug in board */
   LLSER_UART1,      /* the colorado internal serial ports */
   LLSER_UART2,
   LLSER_UART3,      /* 3 is the normal serial port */
   LLSER_MEMORY,     /* Trace messages dumped to a RAM buffer */
   LLSER_DBGCOMM,    /* the muli-ice debug comms channel */
   LLSER_HOSTMODEM   /* the host modem llserial interface */
} LLSERID;

/* this is an interface structure containing function pointers for all
 * the low level serial functions */
typedef struct
{
   int (*config)( unsigned int, unsigned int, unsigned int, bool, bool );
   int (*close)( void );
   void (*enable)( bool );
   void (*txstart)( void );
   void (*setFlowReady)( bool );
   bool (*getFlowReady)( void );
   void (*setFlowControl)( bool );
   bool (*getFlowControl)( void );
   int  (*sergetchar)( bool );
   int  (*serputchar)( int, bool );
} LLSERINTERFACE, *PLLSERINTERFACE;

/*
 * This structure holds all the pointers to the client callback functions
 * This is populated by the client with pointers to any functions it wants
 * called back.  Any unused should be left as null
 */
typedef struct
{
   void (*flowControlHandler)( bool );
   void (*flowReadyHandler)( bool );
   void (*cdHandler)( bool );
   void (*ringHandler)( void );
   unsigned int (*txHandler)( unsigned int, char* );
   void (*rxHandler)( unsigned int, char* );
   void (*oeHandler)( void );
   void (*feHandler)( void );
   void (*peHandler)( void );
   void (*brkHandler)( void );
} LLSERCALLBACKS, *PLLSERCALLBACKS;

/* globals */

PLLSERINTERFACE llser_open( LLSERID id, PLLSERCALLBACKS pcb );

#endif /* _LLSERIAL_H_ */

/****************************************************************************
 * Modifications:
 * $Log: 
 *  3    mpeg      1.2         9/22/03 3:21:32 PM     Bobby Bradford  SCR(s) 
 *        7418 :
 *        Add support for host-connected modem device to llserial
 *        driver
 *        
 *  2    mpeg      1.1         1/14/03 4:03:36 PM     Dave Wilson     SCR(s) 
 *        5245 :
 *        Added the ability to specify LLSER_MEMORY to redirect trace messages 
 *        to a
 *        RAM buffer.
 *        
 *  1    mpeg      1.0         9/11/00 5:22:22 PM     Joe Kroesche    
 * $
 * 
 *    Rev 1.2   22 Sep 2003 14:21:32   bradforw
 * SCR(s) 7418 :
 * Add support for host-connected modem device to llserial
 * driver
 * 
 *    Rev 1.1   14 Jan 2003 16:03:36   dawilson
 * SCR(s) 5245 :
 * Added the ability to specify LLSER_MEMORY to redirect trace messages to a
 * RAM buffer.
 * 
 *    Rev 1.0   Sep 11 2000 16:22:22   kroescjl
 * Initial revision.
 * 
 *    Rev 1.0   Sep 03 2000 19:38:18   kroescjl
 * Initial revision.
 *
 ****************************************************************************/

