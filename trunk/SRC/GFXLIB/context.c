/****************************************************************************/
/*                   CONEXANT PROPRIETARY AND CONFIDENTIAL                  */
/*                       SOFTWARE FILE/MODULE HEADER                        */
/*                     Conexant Systems Inc. (c) 2002                       */
/*                               Austin, TX                                 */
/*                            All Rights Reserved                           */
/****************************************************************************/
/*                                                                          
   Filename: CONTEXT.C

   Description: Colorado Graphics Accelerator (GXA) Hardware
                Functions to set drawing attributes and context

   Exported Functions:
   void    GfxSetFGColor
   u_int32 GfxGetFGColor
   void    GfxSetBGColor
   u_int32 GfxGetBGColor
   void    GfxSetLinePattern
   u_int32 GfxGetLinePattern

   Created: 9/22/1999 by Eric Ching

   Copyright Conexant Systems, 1999
   All Rights Reserved.

****************************************************************************/

/***************************************************************************
                     PVCS Version Control Information
$Header: context.c, 8, 2/13/03 11:28:20 AM, Matt Korte$
****************************************************************************/

#include "stbcfg.h"
#include "kal.h"
#include "genmac.h"
#include "context.h"

extern sem_id_t GXA_Sem_Id;

/***************************************************************************
 Public Hardware Context Routines
***************************************************************************/

/***************************************************************************
 GfxSetFGColor
***************************************************************************/
void GfxSetFGColor(u_int32 Color)
{
   sem_get(GXA_Sem_Id, KAL_WAIT_FOREVER);

   mCheckFifo(1)
   mLoadReg(GXA_FG_COLOR_REG, Color)

   sem_put(GXA_Sem_Id);

}

/***************************************************************************
 GfxGetFGColor
***************************************************************************/
u_int32 GfxGetFGColor(void)
{
   mWaitForFifoEmpty
   return( *(u_int32 *)(GXA_NON_QUEUED | GXA_FG_COLOR_REG) );
}

/***************************************************************************
 GfxSetBGColor
***************************************************************************/
void GfxSetBGColor(u_int32 Color)
{
   sem_get(GXA_Sem_Id, KAL_WAIT_FOREVER);

   mCheckFifo(1)
   mLoadReg(GXA_BG_COLOR_REG, Color)

   sem_put(GXA_Sem_Id);

}

/***************************************************************************
 GfxGetBGColor
***************************************************************************/
u_int32 GfxGetBGColor(void)
{
   mWaitForFifoEmpty
   return( *(u_int32 *)(GXA_NON_QUEUED | GXA_BG_COLOR_REG) );
}

/***************************************************************************
 GfxSetLinePattern
***************************************************************************/
void GfxSetLinePattern(u_int32 Value)
{
   sem_get(GXA_Sem_Id, KAL_WAIT_FOREVER);

   mCheckFifo(1)
   mLoadReg(GXA_LINE_PATT_REG, Value)
   /* set the source bitmap pattern bit when line pattern reg is set */
   /* GfxPolyLine sets the source bitmap to bitmap register 0 */
   CNXT_SET_VAL( GXA_BMAP0_TYPE_REG, GXA_BMAP_TYPE_PATTERN_MASK, 0x1 );

   sem_put(GXA_Sem_Id);

}

/***************************************************************************
 GfxGetLinePattern
***************************************************************************/
u_int32 GfxGetLinePattern(void)
{
   mWaitForFifoEmpty
   return( *(u_int32 *)(GXA_NON_QUEUED | GXA_LINE_PATT_REG) );
}

/***************************************************************************
 Private Hardware Context Routines
***************************************************************************/

/***************************************************************************
 GxaSetGxaConfig
***************************************************************************/
void GxaSetGUIConfig(u_int32 Value)
{
   sem_get(GXA_Sem_Id, KAL_WAIT_FOREVER);

   mCheckFifo(1)
   mLoadReg(GXA_CFG_REG, Value)

   sem_put(GXA_Sem_Id);

}

/***************************************************************************
 GxaGetGxaConfig
***************************************************************************/
u_int32 GxaGetGxaConfig(void)
{
   mWaitForFifoEmpty
   return( * (u_int32 *)(GXA_NON_QUEUED | GXA_CFG_REG) );
}

/***************************************************************************
 GxaSetLineControl
***************************************************************************/
void GxaSetLineControl(u_int32 Value)
{
   sem_get(GXA_Sem_Id, KAL_WAIT_FOREVER);

   mCheckFifo(1)
   mLoadReg(GXA_LINE_CONTROL_REG, Value)

   sem_put(GXA_Sem_Id);

}

/***************************************************************************
 GxaGetLineControl
***************************************************************************/
u_int32 GxaGetLineControl(void)
{
   mWaitForFifoEmpty
   return( *(u_int32 *)(GXA_NON_QUEUED | GXA_LINE_CONTROL_REG) );
}

/***************************************************************************
 GxaSetBltControl
***************************************************************************/
void GxaSetBltControl(u_int32 Value)
{
   sem_get(GXA_Sem_Id, KAL_WAIT_FOREVER);

   mCheckFifo(1)
   mLoadReg(GXA_BLT_CONTROL_REG, Value)

   sem_put(GXA_Sem_Id);

}

/***************************************************************************
 GxaGetBltControl
***************************************************************************/
u_int32 GxaGetBltControl(void)
{
   mWaitForFifoEmpty
   return( *(u_int32 *)(GXA_NON_QUEUED | GXA_BLT_CONTROL_REG) );
}

/***************************************************************************
 GxaSetupBitmapContext
***************************************************************************/
void GxaSetupBitmapContext( u_int8  BMIdx,
                           u_int32 Type,
                           u_int32 Format,
                           u_int32 Pitch,
                           u_int32 Addr)
{
   LPREG BitmapReg = (LPREG)(GXA_BMAP0_TYPE_REG + (BMIdx << 3));
   u_int32 Pattern = CNXT_GET(BitmapReg, GXA_BMAP_TYPE_PATTERN_MASK);
    
   mCheckFifo(2)
#ifndef PCI_GXA
   *BitmapReg++ = Type | Format | Pitch | Pattern;
   *BitmapReg = Addr;
#else
   *BitmapReg++ = Type | Addr | Pattern;
   *BitmapReg = Pitch;
#endif
}


/*;----------------------------------------------------------------------
;$Log: 
; 8    mpeg      1.7         2/13/03 11:28:20 AM    Matt Korte      SCR(s) 5479
;        :
;       Removed old header reference
;       
; 7    mpeg      1.6         11/21/02 2:36:22 PM    Lucy C Allevato SCR(s) 4988
;        :
;       select pattern bit of source bitmap when line pattern reg is set
;       
; 6    mpeg      1.5         11/17/00 3:26:38 PM    Lucy C Allevato Fixed up 
;       bitmap context routine to use LPREG to get volatile modifier on
;       the hw reg being used.
;       
; 5    mpeg      1.4         6/28/00 4:31:18 PM     Lucy C Allevato Added usage
;        of GXA semaphore to serialize accesses that program HW registers.
;       
; 4    mpeg      1.3         3/31/00 4:51:26 PM     Lucy C Allevato Updates for
;        name changes in colorado.h to match up with spec.
;       
; 3    mpeg      1.2         3/23/00 3:47:52 PM     Lucy C Allevato Some 
;       function renaming since some of these are now public.
;       
; 2    mpeg      1.1         3/21/00 4:29:44 PM     Lucy C Allevato Changed 
;       function prefix from Hw to Gxa for context register access routines.
;       
; 1    mpeg      1.0         3/6/00 2:07:52 PM      Lucy C Allevato 
;$:   K:/sabine/pvcs/GFXLIB/context.c_v  $:
 * 
 *    Rev 1.7   13 Feb 2003 11:28:20   kortemw
 * SCR(s) 5479 :
 * Removed old header reference
 * 
 *    Rev 1.6   21 Nov 2002 14:36:22   goldenx
 * SCR(s) 4988 :
 * select pattern bit of source bitmap when line pattern reg is set
 * 
 *    Rev 1.5   17 Nov 2000 15:26:38   eching
 * Fixed up bitmap context routine to use LPREG to get volatile modifier on
 * the hw reg being used.
 * 
 *    Rev 1.4   28 Jun 2000 15:31:18   eching
 * Added usage of GXA semaphore to serialize accesses that program HW registers.
 * 
 *    Rev 1.3   31 Mar 2000 16:51:26   eching
 * Updates for name changes in colorado.h to match up with spec.
 * 
 *    Rev 1.2   23 Mar 2000 15:47:52   eching
 * Some function renaming since some of these are now public.
 * 
 *    Rev 1.1   21 Mar 2000 16:29:44   eching
 * Changed function prefix from Hw to Gxa for context register access routines.
 * 
 *    Rev 1.0   06 Mar 2000 14:07:52   eching
 * Initial revision.
 * 
 *    Rev 1.0   06 Mar 2000 13:27:20   eching
 * Initial revision.
  
;---------------------------------------------------------------------- */

