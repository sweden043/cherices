/****************************************************************************/
/*                   CONEXANT PROPRIETARY AND CONFIDENTIAL                  */
/*                       SOFTWARE FILE/MODULE HEADER                        */
/*           Conexant Systems Inc. (c) 1998, 1999, 2000, 2001, 2002         */
/*                               Austin, TX                                 */
/*                            All Rights Reserved                           */
/****************************************************************************/
/*
 * Filename:      DVB_PRIV.H 
 *
 *
 * Description:    Private header file for DVB library
 *
 *
 * Author:         Dave Wilson
 *
 ****************************************************************************/
/* $Header: dvb_priv.h, 1, 4/11/03 6:01:08 PM, Dave Wilson$
 ****************************************************************************/
#ifndef _DVB_PRIV_H_
#define _DVB_PRIV_H_

/***************************/
/* Local Label Definitions */
/***************************/
#define TL_ALWAYS  (TRACE_LEVEL_ALWAYS | TRACE_TST)
#define TL_ERROR   (TRACE_LEVEL_ALWAYS | TRACE_TST)
#define TL_INFO    (TRACE_LEVEL_3 | TRACE_TST)
#define TL_FUNC    (TRACE_LEVEL_2 | TRACE_TST)
#define TL_VERBOSE (TRACE_LEVEL_1 | TRACE_TST)

/* PSI section channel buffer sizes */
#define ECM_BUFFER_SIZE 8192
#define PAT_BUFFER_SIZE 8192
#define PMT_BUFFER_SIZE 8192
#define SDT_BUFFER_SIZE 8192
#define CAT_BUFFER_SIZE 8192

/***********************************/
/* Externally referenced variables */
/***********************************/
extern u_int32 gDemuxInstance; 
extern DVB_PFNNOTIFY pfnDVBCallback;

#endif /* _DVB_PRIV_H_ */

/****************************************************************************
 * Modifications:
 * $Log: 
 *  1    mpeg      1.0         4/11/03 6:01:08 PM     Dave Wilson     
 * $
 * 
 *    Rev 1.0   11 Apr 2003 17:01:08   dawilson
 * SCR(s) 5979 :
 * Private header file for DVB module
 *
 ****************************************************************************/

