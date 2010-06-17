/***************************************************************************
                                                                          
   Filename: GFX_OS.H

   Description: OS environment abstraction macros
                Memory allocation, debug and trace for the
                operating environment are abstracted here for
                better portability

   Created: 10/15/1999 by Eric Ching

   Copyright Conexant Systems, 1999
   All Rights Reserved.

****************************************************************************/

/***************************************************************************
                     PVCS Version Control Information
$Header: gfx_os.h, 2, 2/27/03 1:54:28 PM, Lucy C Allevato$
$Log: 
 2    mpeg      1.1         2/27/03 1:54:28 PM     Lucy C Allevato SCR(s) 5565 
       :
       remove macros of MEM_ALLOC and MEM_FREE because they're not used any 
       more.
       
 1    mpeg      1.0         8/23/00 12:18:32 PM    Lucy C Allevato 
$
 * 
 *    Rev 1.1   27 Feb 2003 13:54:28   goldenx
 * SCR(s) 5565 :
 * remove macros of MEM_ALLOC and MEM_FREE because they're not used any more.
 * 
 *    Rev 1.0   23 Aug 2000 11:18:32   eching
 * Initial revision.
 * 
 *    Rev 1.0   06 Mar 2000 13:42:18   eching
 * Initial revision.
****************************************************************************/

#ifndef _GFX_OS_H_
#define _GFX_OS_H_

#ifndef _KAL_H_
  #include "kal.h"
#endif
/***************************************************************************
   M E M O R Y
****************************************************************************/

/***************************************************************************
   D E B U G   A N D   T R A C E
****************************************************************************/

#define GXA_TRACE_MSG (TRACE_LEVEL_2 | TRACE_GCP)
#define GXA_ERR_MSG (TRACE_LEVEL_ALWAYS | TRACE_GCP)

#if defined(DEBUG)

  #define TRACE( X ) trace_new##X
  #define ERRMSG( X ) trace_new##X
  #define ISR_TRACE( X,Y,Z ) isr_trace_new(GXA_TRACE_MSG,X,Y,Z)
#else
  #define TRACE( X ) 
  #define ERRMSG( X )
  #define ISR_TRACE( X,Y,Z ) 
#endif

#ifdef ASSERT
  #undef ASSERT
#endif

#ifdef DEBUG
  #define ASSERT( exp ) \
    if (!(exp)) { trace_new(GXA_ERR_MSG, \
    "\n\nASSERT (" #exp "), " __FILE__ " line %d.\n",__LINE__); }
#else
  #define ASSERT( exp ) 
#endif

#endif //_GXA_OS_H_

