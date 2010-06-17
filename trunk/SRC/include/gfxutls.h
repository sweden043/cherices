/***************************************************************************
                                                                          
   Filename: GFXUTLS.H

   Description: Graphics Hardware Utility Routines Header

   Created: 1/19/2000 by Eric Ching

   Copyright Conexant Systems, 2000
   All Rights Reserved.

****************************************************************************/

/***************************************************************************
                     PVCS Version Control Information
$Header: gfxutls.h, 1, 8/23/00 12:20:20 PM, Lucy C Allevato$
$Log: 
 1    mpeg      1.0         8/23/00 12:20:20 PM    Lucy C Allevato 
$
 * 
 *    Rev 1.0   23 Aug 2000 11:20:20   eching
 * Initial revision.
 * 
 *    Rev 1.2   06 Jun 2000 17:07:24   eching
 * Fixed comment style to not use C++ line comment.
 * 
 *    Rev 1.1   24 May 2000 19:36:12   eching
 * Added GfxConvertPitchToPixels prototype
 * 
 *    Rev 1.0   23 Mar 2000 15:54:28   eching
 * Initial revision.
****************************************************************************/
#ifndef _GFXUTLS_H_
#define _GFXUTLS_H_

#include "kal.h"
#include "gfxtypes.h"

u_int32 GfxTranslateTypeToFormat(u_int8 Type);
u_int32 GfxTranslateColor(PGFX_COLOR pColor, u_int8 Bpp, u_int8 Type);
u_int32 GfxConvertPitchToPixels(u_int32 PitchBytes, u_int8 Bpp);

#endif /* _GFXUTLS_H_ */
