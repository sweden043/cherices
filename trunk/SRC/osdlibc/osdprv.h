/****************************************************************************/
/*                   CONEXANT PROPRIETARY AND CONFIDENTIAL                  */
/*                       SOFTWARE FILE/MODULE HEADER                        */
/*                 Conexant Systems Inc. (c) 1998, 1999, 2000               */
/*                               Austin, TX                                 */
/*                            All Rights Reserved                           */
/****************************************************************************/
/*
 * Filename:       osdprv.h
 *
 *
 * Description:    Private data structures/values for OSD
 *
 *
 * Author:         Miles Bintz
 *
 ****************************************************************************/
/* $Header: osdprv.h, 3, 5/9/03 7:12:54 PM, Steve Glennon$
 ****************************************************************************/

#ifndef _OSDPRV_H_
#define _OSDPRV_H_

/*****************************************************************/
/* Private labels and type definitions shared within this module */
/*****************************************************************/
#define OSDTTXQU_LINEPASS        0x00000001
#define OSDTTXQU_UBSHUTDOWN      0x00000002

#ifndef abs
#define abs(x) (((x) < 0) ? (-x):(x))
#endif

/**************************************************************/
/* Internal variables shared between files within this module */
/**************************************************************/
extern POSDLIBREGION gpOsdLibList[];
extern u_int32 gnHBlank;
extern u_int32 gnVBlank;

/**************************************************************/
/* Internal functions shared between files within this module */
/**************************************************************/
extern void vidSetHWBuffSize(void);

#endif /* _OSDPRV_H_ */

/****************************************************************************
 * Modifications:
 * $Log: 
 *  3    mpeg      1.2         5/9/03 7:12:54 PM      Steve Glennon   SCR(s): 
 *        6224 6225 6190 6179 
 *        Fixed to be consistent with dual plane support in osdlibc.c
 *        
 *        
 *  2    mpeg      1.1         1/27/03 3:24:58 PM     Dave Wilson     SCR(s) 
 *        5320 :
 *        Added some new definitions required as a result of splitting 
 *        encoder.c out
 *        of osdlibc.c.
 *        
 *  1    mpeg      1.0         8/16/00 3:10:30 PM     Miles Bintz     
 * $
 * 
 *    Rev 1.2   09 May 2003 18:12:54   glennon
 * SCR(s): 6224 6225 6190 6179 
 * Fixed to be consistent with dual plane support in osdlibc.c
 * 
 * 
 *    Rev 1.1   27 Jan 2003 15:24:58   dawilson
 * SCR(s) 5320 :
 * Added some new definitions required as a result of splitting encoder.c out
 * of osdlibc.c.
 *
 ****************************************************************************/

