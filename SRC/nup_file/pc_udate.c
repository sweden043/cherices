/****************************************************************************/
/*                   CONEXANT PROPRIETARY AND CONFIDENTIAL                  */
/*                       SOFTWARE FILE/MODULE HEADER                        */
/*                 Conexant Systems Inc. (c) 2003                           */
/*                               Austin, TX                                 */
/*                            All Rights Reserved                           */
/****************************************************************************/
/*
 * Filename:       pc_udate.c
 *
 *
 * Description:    see ATI header below.
 *                 Initial version based on Nucleus File v2.4.4
 *
 ****************************************************************************/
/* $Header: pc_udate.c, 2, 4/2/04 8:55:12 PM, Nagaraja Kolur$
 ****************************************************************************/

/************************************************************************
*                                                                       
*               Copyright Mentor Graphics Corporation 2003              
*                         All Rights Reserved.                          
*                                                                       
* THIS WORK CONTAINS TRADE SECRET AND PROPRIETARY INFORMATION WHICH IS  
* THE PROPERTY OF MENTOR GRAPHICS CORPORATION OR ITS LICENSORS AND IS   
* SUBJECT TO LICENSE TERMS.                                             
*                                                                       
*                                                                       
*************************************************************************

*************************************************************************
* FILE NAME                                       VERSION                
*                                                                       
*       pc_udate.c                        Nucleus FILE\ARM7TDMI\ADS 2.5.6 
*                                                                       
* COMPONENT                                                             
*                                                                       
*       Nucleus FILE                                                    
*                                                                       
* DESCRIPTION                                                           
*                                                                       
*       When the system needs to date stamp a file it will call this    
*       routine to get the current time and date. YOU must modify the   
*       shipped routine to support your hardware's time and date        
*       routines. If you don't modify this routine the file date on all 
*       files will be the same.                                         
*                                                                       
*       The source for this routine is in file pc_udate.c and is self   
*       explanatory.                                                    
*                                                                       
* DATA STRUCTURES                                                       
*                                                                       
*       None.                                                           
*                                                                       
* FUNCTIONS                                                             
*                                                                       
*       pc_getsysdate                       Get date and time from the  
*                                            host system.               
*                                                                       
* DEPENDENCIES                                                          
*                                                                       
*       pcdisk.h                            File common definitions     
*                                                                       
* NOTE: This module is linked in with File.lib.  After you make   
*       changes, you must rebuild File.lib.                       
*************************************************************************/

#include        "pcdisk.h"

/************************************************************************
* FUNCTION                                                              
*                                                                       
*       pc_getsysdate                                                   
*                                                                       
* DESCRIPTION                                                           
*                                                                       
*       Get date and time from the host system.                         
*       
*                                                                       
* INPUTS                                                                
*                                                                       
*       pd                                  Date stamping buffer.       
*                                                                       
* OUTPUTS                                                               
*                                                                       
*      Pointer of the buffer of date structure.                         
*                                                                       
*************************************************************************/
DATESTR *pc_getsysdate(DATESTR *pd)
{
UINT16      year;       /* relative to 1980 */ 
UINT16      month;      /* 1 - 12 */ 
UINT16      day;        /* 1 - 31 */ 
UINT16      hour;
UINT16      minute;
UINT16      sec;        /* Note: seconds are 2 second/per. ie 3 == 6 seconds */
UINT8       cenmsec;

    /* This code is useless but very generic */
    /* Hardwire for now */
    /* 7:37:28 PM */
    hour = 19;
    minute = 37;
    sec = 14;
    /* 3-28-88 */
    year = 8;       /* relative to 1980 */ 
    month = 3;      /* 1 - 12 */ 
    day = 28;       /* 1 - 31 */
    /* Centesimal Milliseconds (1sec / 100)   */
    cenmsec = (UINT8)0;    /* 0 - 199 */

    pd->cmsec = cenmsec;
    pd->time = (UINT16) ( (hour << 11) | (minute << 5) | sec );
    pd->date = (UINT16) ( (year << 9) | (month << 5) | day );

    return(pd);
}



/****************************************************************************
 * Modifications:
 * $Log: 
 *  2    mpeg      1.1         4/2/04 8:55:12 PM      Nagaraja Kolur  CR(s) 
 *        8762 8763 : Merged 2.5.6 Nup File System.
 *        Test: 1. pvr demo 2. nup file test 3. CR 8181
 *        
 *  1    mpeg      1.0         8/22/03 5:42:52 PM     Dave Moore      
 * $
 * 
 *    Rev 1.0   22 Aug 2003 16:42:52   mooreda
 * SCR(s) 7350 :
 * Nucleus File sourcecode (Date/Time Stamping)
 * 
 *
 ****************************************************************************/

