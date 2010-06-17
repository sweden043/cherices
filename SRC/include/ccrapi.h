/****************************************************************************/
/*                   CONEXANT PROPRIETARY AND CONFIDENTIAL                  */
/*                       SOFTWARE FILE/MODULE HEADER                        */
/*                 Conexant Systems Inc. (c) 1998, 1999, 2000               */
/*                               Austin, TX                                 */
/*                            All Rights Reserved                           */
/****************************************************************************/
/*
 * Filename:       ccrapi.h
 *
 *
 * Description:    Public header file for Conexant low-level clock recovery
 *                 API.
 *
 * Author:         Amy C. Pratt
 *
 ****************************************************************************/
/* $Header: ccrapi.h, 2, 7/22/03 12:31:26 PM, Larry Wang$
 ****************************************************************************/

#ifndef _CCRAPI_H_
#define _CCRAPI_H_

/** include files **/

/** definitions **/
#undef USING_FLOATING_POINT

#define CCR_DISCONT_THRESHOLD 5400000 /* if the PTS and STC differ by more 
                                         than .2 seconds, it is a 
                                         discontinuity                  */
                                         
/** data types **/
#ifdef USING_FLOATING_POINT
typedef double (*ccrFilterFunc)(int32 delta);
#else
typedef int32 (*ccrFilterFunc)(int32 delta);
#endif

typedef void (*ccrFilterInitFunc)(void);
typedef enum {
    CCR_RETURN_SUCCESS,
    CCR_LIMIT_TOO_SMALL,
    CCR_LIMIT_TOO_LARGE,
    CCR_TIMEVAL_TOO_LARGE,
    CCR_NULL_POINTER
} CCR_RETCODE;

/** public functions **/
CCR_RETCODE ccrSetDiscontinuityThreshold(u_int32 limit);
u_int32 ccrGetDiscontinuityThreshold(void);
CCR_RETCODE ccrSetPCRTimer(u_int32 timer_val, bool enable);
void ccrSetSTCFromBitstream(void);
CCR_RETCODE ccrInstallFilter(ccrFilterFunc filter, ccrFilterInitFunc filterInit);



#endif /* _CCRAPI_H_ */
/****************************************************************************
 * Modifications:
 * $Log: 
 *  2    mpeg      1.1         7/22/03 12:31:26 PM    Larry Wang      SCR(s) 
 *        6976 6977 :
 *        Change definition of ccrFilterFunc to use fix point instead of 
 *        floating point.
 *        
 *  1    mpeg      1.0         8/16/00 5:09:36 PM     Amy Pratt       
 * $
 * 
 *    Rev 1.1   22 Jul 2003 11:31:26   wangl2
 * SCR(s) 6976 6977 :
 * Change definition of ccrFilterFunc to use fix point instead of floating point.
 * 
 *    Rev 1.0   16 Aug 2000 16:09:36   prattac
 * Initial revision.
 *
 ****************************************************************************/

