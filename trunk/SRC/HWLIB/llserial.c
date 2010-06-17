/****************************************************************************/
/*                   CONEXANT PROPRIETARY AND CONFIDENTIAL                  */
/*                       SOFTWARE FILE/MODULE HEADER                        */
/*                 Conexant Systems Inc. (c) 1998, 1999, 2000               */
/*                               Austin, TX                                 */
/*                            All Rights Reserved                           */
/****************************************************************************/
/*
 * Filename:      llserial.c
 *
 *                  
 * Description:   Description
 *
 *
 * Author:        kroesche
 *
 ****************************************************************************/
/* $Header: llserial.c, 9, 9/22/03 5:01:46 PM, Bobby Bradford$
 ****************************************************************************/

/* includes */
#include "basetype.h"
#include "stbcfg.h"
#include "llserial.h"

/* external references */
   /* these are the open functions for all the possible serial drivers */
extern PLLSERINTERFACE llseropen_Telegraph( PLLSERCALLBACKS );
extern PLLSERINTERFACE llseropen_Internal0( PLLSERCALLBACKS );
extern PLLSERINTERFACE llseropen_Internal1( PLLSERCALLBACKS );
extern PLLSERINTERFACE llseropen_Internal2( PLLSERCALLBACKS );
extern PLLSERINTERFACE llseropen_memory( PLLSERCALLBACKS );
#if (RTOS != VXWORKS)
extern PLLSERINTERFACE llseropen_DbgComm( PLLSERCALLBACKS );
#endif
extern PLLSERINTERFACE llseropen_HostModem( PLLSERCALLBACKS );

/* locals */
   /* this table contains maps the open function for each possible
      serial driver defined in llserial.h */
static PLLSERINTERFACE (*llseropen[])( PLLSERCALLBACKS ) =
{  NULL,                   /* LLSER_NONE  */
   llseropen_Telegraph,    /* LLSER_TELEGRAPH */
   llseropen_Internal0,    /* LLSER_UART1 */
   llseropen_Internal1,    /* LLSER_UART2 */
   llseropen_Internal2,    /* LLSER_UART3 */
   llseropen_memory,       /* LLSER_MEMORY */
#if (RTOS != VXWORKS)
   llseropen_DbgComm,      /* LLSER_DBGCOMM */
#else
   NULL,                   /* For VxWorks, for now, no DbgComms channel */
#endif
   llseropen_HostModem     /* LLSER_HOSTMODEM */
};

/*
 * PLLSERINTERFACE llser_open( LLSERID id, PLLSERCALLBACKS pcb )
 *
 * parameters:
 *    id - the id of the serial device to initialize, one of the
 *          following: LLSER_TELEGRAPH, _UART1, _UART2, _UART3, _DBGCOMM
 *    pcb - pointer to structure containing client callback functions
 *          any unused callbacks should be set to NULL
 *
 * returns:
 *    a pointer to low level serial interface functions.  The caller
 *    then uses the interface pointer for making further low level
 *    driver calls.
 *
 * Initializes the proper low level serial device driver based on the
 * low level serial ID specified as a parameter.  The llser table is
 * scanned to find the matching low level driver entry, and then its
 * init function is called.  Then if the initialization is successful
 * a function interface table is returned to the caller.
 */
PLLSERINTERFACE llser_open( LLSERID id, PLLSERCALLBACKS pcb )
{

   if( (id < 1) || (id > (sizeof( llseropen )/sizeof( PLLSERINTERFACE )) ) )
      return( NULL );
   else
      return( llseropen[id]( pcb ) );
}

/****************************************************************************
 * Modifications:
 * $Log: 
 *  9    mpeg      1.8         9/22/03 5:01:46 PM     Bobby Bradford  SCR(s) 
 *        7418 :
 *        Added the new Host Connected Modem to the list of drivers
 *        supported in the LLSER open function
 *        
 *  8    mpeg      1.7         8/29/03 12:11:00 PM    Miles Bintz     SCR(s) 
 *        7291 :
 *        back out previous change since ARM_TOOLKIT isn't defined at the 
 *        source code level
 *        
 *  7    mpeg      1.6         8/27/03 6:25:00 PM     Miles Bintz     SCR(s) 
 *        7291 :
 *        use of mice dbg comm channel is dependant on the ARM CRT (ie. 
 *        toolkit) not RTOS
 *        
 *  6    mpeg      1.5         12/20/02 1:56:30 PM    Dave Wilson     SCR(s) 
 *        5204 :
 *        Added new RAM buffer "serial" port used if tracing to a RAM buffer.
 *        
 *  5    mpeg      1.4         12/20/02 9:27:12 AM    Dave Wilson     SCR(s) 
 *        5201 :
 *        Added stbcfg.h to get rid of build warnings due to the fact that NUP 
 *        and
 *        VXWORKS were not defined.
 *        
 *  4    mpeg      1.3         12/19/02 4:35:38 PM    Bobby Bradford  SCR(s) 
 *        5192 :
 *        Disable Debug COMMS channel for VxWorks builds
 *        
 *  3    mpeg      1.2         1/9/02 1:26:12 PM      Miles Bintz     SCR(s) 
 *        2933 :
 *        Removed colorado specific references in llserial stuff.
 *        
 *  2    mpeg      1.1         11/1/00 6:26:52 PM     Joe Kroesche    removed 
 *        debug comms driver stub
 *        
 *  1    mpeg      1.0         9/11/00 5:18:28 PM     Joe Kroesche    
 * $
 * 
 *    Rev 1.8   22 Sep 2003 16:01:46   bradforw
 * SCR(s) 7418 :
 * Added the new Host Connected Modem to the list of drivers
 * supported in the LLSER open function
 * 
 *    Rev 1.7   29 Aug 2003 11:11:00   bintzmf
 * SCR(s) 7291 :
 * back out previous change since ARM_TOOLKIT isn't defined at the source code level
 * 
 *    Rev 1.5   20 Dec 2002 13:56:30   dawilson
 * SCR(s) 5204 :
 * Added new RAM buffer "serial" port used if tracing to a RAM buffer.
 * 
 *    Rev 1.4   20 Dec 2002 09:27:12   dawilson
 * SCR(s) 5201 :
 * Added stbcfg.h to get rid of build warnings due to the fact that NUP and
 * VXWORKS were not defined.
 * 
 *    Rev 1.3   19 Dec 2002 16:35:38   bradforw
 * SCR(s) 5192 :
 * Disable Debug COMMS channel for VxWorks builds
 * 
 *    Rev 1.2   09 Jan 2002 13:26:12   bintzmf
 * SCR(s) 2933 :
 * Removed colorado specific references in llserial stuff.
 * 
 *    Rev 1.1   Nov 01 2000 18:26:52   kroescjl
 * removed debug comms driver stub
 * 
 *    Rev 1.0   Sep 11 2000 16:18:28   kroescjl
 * Initial revision.
 *
 ****************************************************************************/

