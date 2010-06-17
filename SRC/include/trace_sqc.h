/****************************************************************************/
/*                   CONEXANT PROPRIETARY AND CONFIDENTIAL                  */
/*                       SOFTWARE FILE/MODULE HEADER                        */
/*                   Conexant Systems Inc. (c) 1998 - 2003                  */
/*                               Austin, TX                                 */
/*                            All Rights Reserved                           */
/****************************************************************************/
/*
 * Filename:       trace_sqc.h
 *
 *
 * Description:    Simple System Query and Control driver allowing setting 
  *                and querying of debug trace flags.
 *
 *
 * Author:         Dave Wilson
 *
 ****************************************************************************/
/* $Header: trace_sqc.h, 1, 7/7/03 4:36:32 PM, Dave Wilson$
 ****************************************************************************/
#ifndef _TRACE_SQC_H_
#define _TRACE_SQC_H_

#define CONEXANT_TRACE_DRIVER	"cnxt_trace"
#define CONEXANT_TRACE_SYS_INFO	CONEXANT_TRACE_DRIVER

#define CONEXANT_SET_FLAGS 0x30
#define CONEXANT_GET_FLAGS 0x31

typedef struct _conexant_trace_flags
{
  unsigned long uFlags;
} conexant_trace_flags;

#endif /* _TRACE_SQC_H_ */

/****************************************************************************
 * Modifications:
 * $Log: 
 *  1    mpeg      1.0         7/7/03 4:36:32 PM      Dave Wilson     
 * $
 * 
 *    Rev 1.0   07 Jul 2003 15:36:32   dawilson
 * SCR(s) 6817 :
 * Simple OpenTV system query and control driver supporting get/set of the 
 * debug trace flags.
 *
 ****************************************************************************/

