/****************************************************************************/
/*                   CONEXANT PROPRIETARY AND CONFIDENTIAL                  */
/*                       SOFTWARE FILE/MODULE HEADER                        */
/*                 Conexant Systems Inc. (c) 2003                           */
/*                               Austin, TX                                 */
/*                            All Rights Reserved                           */
/****************************************************************************/
/*
 * Filename:       pc_unicd.c
 *
 *
 * Description:    see ATI header below.
 *                 Initial version based on Nucleus File v2.4.4
 *
 ****************************************************************************/
/* $Header: pc_unicd.c, 2, 4/2/04 10:26:08 PM, Nagaraja Kolur$
 ****************************************************************************/

/*************************************************************************/
/*                                                                       */
/*               Copyright Mentor Graphics Corporation 2003              */
/*                         All Rights Reserved.                          */
/*                                                                       */
/* THIS WORK CONTAINS TRADE SECRET AND PROPRIETARY INFORMATION WHICH IS  */
/* THE PROPERTY OF MENTOR GRAPHICS CORPORATION OR ITS LICENSORS AND IS   */
/* SUBJECT TO LICENSE TERMS.                                             */
/*                                                                       */
/*************************************************************************/

/*************************************************************************
* FILE NAME                                     VERSION                 
*                                                                       
*       PC_UNICD.C                                2.5
*                                                                       
* COMPONENT                                                             
*                                                                       
*       Nucleus FILE                                                    
*                                                                       
* DESCRIPTION                                                           
*                                                                       
*       Convert unicode.                                                
*                                                                       
* DATA STRUCTURES                                                       
*                                                                       
*       None.                                                           
*                                                                       
* FUNCTIONS                                                             
*                                                                       
*       uni2asc                             Unicode to ascii.           
*       asc2uni                             Ascii to unicode.           
*                                                                       
* DEPENDENCIES                                                          
*                                                                       
*       pcdisk.h                            File common definitions     
*                                                                       
*************************************************************************/

#include        "pcdisk.h"


/************************************************************************
* FUNCTION                                                              
*                                                                       
*       uni2asc                                                         
*                                                                       
* DESCRIPTION                                                           
*                                                                       
*       Each long filename entry has Unicode character strings. This    
*       routine converts unicode to ascii code.                          
*       byte order.                                                     
*                                                                       
*                                                                       
* INPUTS                                                                
*                                                                       
*       ptr                                 Pointer to unicode          
*                                                                       
* OUTPUTS                                                               
*                                                                       
*       Ascii byte.                                                     
*                                                                       
*************************************************************************/
UINT8 uni2asc(UINT8 *ptr)
{

    return(*ptr);
}


/************************************************************************
* FUNCTION                                                              
*                                                                       
*       asc2uni                                                         
*                                                                       
* DESCRIPTION                                                           
*                                                                       
*       Each long filename entry has Unicode character strings. This    
*       routine converts ascii to unicode.                               
*       byte order.                                                     
*                                                                       
*                                                                       
* INPUTS                                                                
*                                                                       
*       ptr                                 Pointer to unicode          
*       asc                                 Ascii character             
*                                                                       
* OUTPUTS                                                               
*                                                                       
*       Ascii byte.                                                     
*                                                                       
*************************************************************************/
UINT8 asc2uni(UINT8 *ptr, UINT8 ascii)
{

    *ptr = ascii;
    *(ptr+1) = 0;

    return(*ptr);
}



/****************************************************************************
 * Modifications:
 * $Log: 
 *  2    mpeg      1.1         4/2/04 10:26:08 PM     Nagaraja Kolur  CR(s) 
 *        8762 8763 : Merged 2.5.6 Nup File System.
 *        Test: 1. pvr demo 2. nup file test 3. CR 8181
 *        
 *  1    mpeg      1.0         8/22/03 5:46:10 PM     Dave Moore      
 * $
 * 
 *    Rev 1.0   22 Aug 2003 16:46:10   mooreda
 * SCR(s) 7350 :
 * Nucleus File sourcecode (Unicode Conversion)
 * 
 *
 ****************************************************************************/

