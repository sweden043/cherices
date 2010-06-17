/****************************************************************************/
/*                 Conexant Systems Incorporated - SABINE                   */
/****************************************************************************/
/*                                                                          */
/* Filename:           BASETYPE.H                                           */
/*                                                                          */
/* Description:        Public header file containing basic data type        */
/*                     definitions.                                         */
/*                                                                          */
/* Author:             Dave Wilson                                          */
/*                                                                          */
/* Copyright Conexant Systems, 1999                                         */
/* All Rights Reserved.                                                     */
/*                                                                          */
/****************************************************************************
$Header: basetype.h, 9, 9/16/03 2:55:00 PM, Tim White$   
$Log: 
 9    mpeg      1.8         9/16/03 2:55:00 PM     Tim White       SCR(s) 7473 
       :
       Add 64bit typedefs req'd for PVR.
       
       
 8    mpeg      1.7         8/9/02 6:06:34 PM      Carroll Vance   SCR(s) 4244 
       :
       Added macros to pack structures under GNU GCC and ARM ADS and SDT c 
       compilers.
       
 7    mpeg      1.6         10/28/99 12:53:42 PM   Dave Wilson     Removed some
        double-slash comments which annoyed VxWorks
       
 6    mpeg      1.5         9/14/99 3:08:28 PM     Dave Wilson     Changes due 
       to splitting KAL into PSOSKAL and HWLIB components.
       
 5    mpeg      1.4         8/11/99 5:22:26 PM     Dave Wilson     Added a few 
       more definitions to allow non-OpenTV KAL apps to build properly.
       
 4    mpeg      1.3         6/2/99 3:18:12 PM      Dave Moore      added 
       uint8,uint16,uint32 and moved int32,int16,int8 within
       #ifdef. This was all done to keep softmodem code common with
       Newport Beach.
       
 3    mpeg      1.2         5/4/99 4:56:30 PM      Dave Wilson     Added bool 
       (I think!).
       
 2    mpeg      1.1         4/27/99 5:14:40 PM     Dave Wilson     Hopefully 
       fixed logging.
       
 1    mpeg      1.0         4/27/99 5:13:12 PM     Dave Wilson     
$   
 * 
 *    Rev 1.8   16 Sep 2003 13:55:00   whiteth
 * SCR(s) 7473 :
 * Add 64bit typedefs req'd for PVR.
 * 
 * 
 *    Rev 1.7   09 Aug 2002 17:06:34   vancec
 * SCR(s) 4244 :
 * Added macros to pack structures under GNU GCC and ARM ADS and SDT c compilers.
 * 
 *    Rev 1.6   28 Oct 1999 11:53:42   dawilson
 * Removed some double-slash comments which annoyed VxWorks
 * 
 *    Rev 1.5   14 Sep 1999 14:08:28   dawilson
 * Changes due to splitting KAL into PSOSKAL and HWLIB components.
 * 
 *    Rev 1.4   11 Aug 1999 16:22:26   dawilson
 * Added a few more definitions to allow non-OpenTV KAL apps to build properly.
 * 
 *    Rev 1.3   02 Jun 1999 14:18:12   mooreda
 * added uint8,uint16,uint32 and moved int32,int16,int8 within
 * #ifdef. This was all done to keep softmodem code common with
 * Newport Beach.
 * 
 *    Rev 1.2   04 May 1999 15:56:30   dawilson
 * Added bool (I think!).
 * 
 *    Rev 1.1   27 Apr 1999 16:14:40   dawilson
 * Hopefully fixed logging.
*/

#ifndef _BASETYPE_H_
#define _BASETYPE_H_

#ifndef __OPTVX_H__

typedef unsigned long long u_int64;
typedef unsigned long u_int32;
typedef unsigned short u_int16;
typedef unsigned char u_int8;
typedef unsigned int bool;
typedef void *voidF;

#if !defined(TYPEDEF_H) /* added to keep softmodem code common */
typedef signed long long int64;
typedef signed long int32;
typedef signed short int16;
typedef signed char int8;

// added to keep softmodem code common
typedef unsigned long uint64;
typedef unsigned long uint32;
typedef unsigned short uint16;
typedef unsigned char uint8;
#endif

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

/* Packed structures should be declared like:
PACKED_TYPE struct footype{
    int a;
    char b;
} PACKED_ATTRIBUTE foo;
typedef PACKED_TYPE struct {
    int a;
    char b;
} PACKED_ATTRIBUTE bartype;  */
#if defined(__GNUC__)
/* GNU GCC compiler. */
#define PACKED_TYPE
#define PACKED_ATTRIBUTE    __attribute__ ((__packed__))
#else /* defined(__GNUC__) */
/* ARM SDT or ADS compiler. */
#define PACKED_TYPE         __packed
#define PACKED_ATTRIBUTE 
#endif /* defined(__GNUC__) */


#endif /* __OPTVX_H__ */
#endif /* _BASETYPE_H_ */

