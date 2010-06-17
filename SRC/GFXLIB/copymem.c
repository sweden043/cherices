/****************************************************************************/
/*                   CONEXANT PROPRIETARY AND CONFIDENTIAL                  */
/*                       SOFTWARE FILE/MODULE HEADER                        */
/*                   Conexant Systems Inc. (c) 1998 - 2003                  */
/*                               Austin, TX                                 */
/*                            All Rights Reserved                           */
/****************************************************************************/
/*
 * Filename:       COPYMEM.C
 *
 *
 * Description:    Basic, low level graphics blit routines
 *
 *
 * Author:         Eric Ching
 *
 ****************************************************************************/
/* $Header: copymem.c, 15, 3/16/04 10:59:31 AM, Xin Golden$
 ****************************************************************************/

/*****************/
/* Include Files */
/*****************/
#include "stbcfg.h"
#include "kal.h"
#include "gfxlib.h"
#include "genmac.h"
#include "gxablt.h"
#include "context.h"
#include "retcodes.h"
#include "basetype.h"

/***********************/
/* External References */
/***********************/
extern sem_id_t GXA_Sem_Id;
extern sem_id_t qmark_sem;
extern int GetCPUType(void);

/* P U B L I C   F U N C T I O N S */

/****************************************************************************
 * GfxIsGXACmdDone
 ***************************************************************************/
int GfxIsGXACmdDone(u_int32 iWaitTime)
{
    int retcode = RC_KAL_TIMEOUT, i;
    static u_int8 qmark_counter = 0;
    volatile u_int32 *gxaCmdReg = (u_int32 *) GXA_CMD_REG;
    u_int32 value;

    qmark_counter++; //Don't want to see someon else's qmark

    if(sem_get(GXA_Sem_Id,1000) != RC_OK)
      return retcode;

        /* This is the non-blocking case. Here */
        /* we just check if the GXA is idle    */
        /* then go on without waiting.         */
        if (iWaitTime == 0) 
        {
            /* Clear QMark completion status bit. */
            mGetGXAConfig2(value);
            mSetGXAConfig2(value | GXA_STAT_QMARK);
            
            /* Issue QMark command */
            mQMark(qmark_counter)  
    
            /* Check the QMark completed status bit. */
            /* We read it several times to allow it to */
            /* flush through the GXA hardware.         */
            for (i = 0; i < 10; i++)
            {
                mGetGXAConfig2(value); 
            }
            if ((value & GXA_STAT_QMARK) && (qmark_counter == (*gxaCmdReg >> 24)) )
            {
                retcode = RC_OK;
            }
            else
            {
                retcode = RC_KAL_TIMEOUT;
            }
        }
        else
        {
            /* Here we actually want to wait until the */
            /* GXA is idle, then return (i.e. block).  */
        
            /* Enable IRQ on QMark completion */
            mGetGXAConfig2(value);
            mSetGXAConfig2(value | GXA_EN_QMARK_IRQ);
    
            /* Issue QMark command */
            mQMark(qmark_counter)  /* My birthday! */
    
            /* Grab QMark completed semaphore. This semaphore is set */
            /* by the ISR for the QMark IRQ. Although this would     */
            /* seem like it would create a race condition, it does   */
            /* not as the GXA harware is capable of executing a      */
            /* QMark quicker than the CPU can follow up the Qmark    */
            /* command with a read of the GXA's Qmark status bit.    */
			      retcode = sem_get(qmark_sem, iWaitTime);
				   while((qmark_counter != (*gxaCmdReg >> 24)) && (retcode != RC_KAL_TIMEOUT))
                  retcode = sem_get(qmark_sem,iWaitTime);
				     
    
            /* Disable IRQ on QMark completion */
            mGetGXAConfig2(value);
            mSetGXAConfig2(value & ~GXA_EN_QMARK_IRQ);
        }
      sem_put(GXA_Sem_Id);

    /* Return retcode */
    return(retcode);
}    

/****************************************************************************
 * Function: GfxFillMem()
 *
 * Parameters: u_int32 *pDest - Pointer to first byte of destination memory.
 *             int Pitch - Pitch in bytes of rectangular destination buffer.
 *             int ExtX - Horizontal extent in pixels for solid fill blt.
 *             int ExtY - Vertical extent in pixels for solid fill blt.
 *             u_int32 FillColor - Color to use for solid fill blt.
 *             int X - X pixel coordinate for start of blt.
 *             int Y - Y pixel coordinate for start of blt.
 *             int Bpp - Bpp value to use for fill blt. 4/8/16/32
 *             u_int8 Rop - ROP code to use for the solid blt.
 *                          0 for copy
 *                          Non-zero for XOR
 *
 * Returns: void
 *
 * Description: Performs a solid fill blt to rectangular memory area
 *              using the specified color value. The value must be
 *              replicated throughout all 32 bits of the FillColor value
 *              if the value is less than 32 bits. For instance an 8 bit
 *              color index 0xNN must be replicated to 0xNNNNNNNN.
 ***************************************************************************/
void GfxFillMem(u_int32 *pDest,
                int Pitch,
                int ExtX,
                int ExtY,
                u_int32 FillColor,
                int X,
                int Y,
                int Bpp,
                u_int8 Rop)
{
u_int32 Format;
u_int32 BltCmdType;
u_int32 GxaConfig;
u_int32 SavedBgColor;
u_int32 PitchPixels;

   if ( Bpp != 4 ) {

      sem_get(GXA_Sem_Id,KAL_WAIT_FOREVER);

      /* Translate bpp to hardware format */
      switch (Bpp) {
         case 8:
            Format = BM_FMT_RGB8;
            PitchPixels = Pitch;
            break;
         case 16:
            Format = BM_FMT_ARGB0565;
            PitchPixels = Pitch >> 1;
            break;
         case 32:
            Format = BM_FMT_ARGB32;
            PitchPixels = Pitch >> 2;
            break;
         default: /* Param error if here use 8 bpp values */
            Format = BM_FMT_RGB8;
            PitchPixels = Pitch;
      }

      /* setup context register 7 for solid fill */
      GxaSetupBitmapContext(7,BM_TYPE_SOLID,Format,0,0);

      /* setup context register 0 for destination */
#ifdef PCI_GXA
      GxaSetupBitmapContext(0,BM_TYPE_COLOR,Format,PitchPixels,((u_int32)pDest>>2));
#else
      GxaSetupBitmapContext(0,BM_TYPE_COLOR,Format,PitchPixels,(u_int32)pDest);
#endif

      if ( Rop == 0 ) {
         BltCmdType = SOLID_BLT_COPY;
      } else {
         /* Set the XOR ROP for this operation */
         mGetGXAConfig(GxaConfig)
         GxaConfig = (GxaConfig & 0xFFFFFF00) | 0x0A;
         mSetGXAConfig(GxaConfig)

         BltCmdType = SOLID_BLT_ROP;
      }

      /* Save the background color register value */
      mGetBGColor(SavedBgColor)

      /* send the blt, source context 7, dest context 0, params 2 */
      mCheckFifo(4)
      mSetBGColor(FillColor)
      mCmd1( GXA_BASE |BltCmdType|BMAP7_SRC|BMAP0_DST,
            mMakeXY(X,Y),
            mMakeXY(ExtX,ExtY) )

      /* Restore the background color register value */
      mSetBGColor(SavedBgColor)

      sem_put(GXA_Sem_Id);

   } else {
      /* Software 4 bpp copy is handled here by calling bitblt.s */
      if (!Rop) {
         SWSolidFill((int) pDest, Pitch, ExtX, ExtY, FillColor,
                     X, Y, GCP_4BPP);
      } else {
         SWXORFill((int) pDest, Pitch, ExtX, ExtY, FillColor,
                     X, Y, GCP_4BPP);
      }
   }

}

/****************************************************************************
 * Function: GfxCopyRectMem()
 *
 * Parameters: u_int32 *pDest - Pointer to start of destination memory.
 *             u_int32 *pSrc - Pointer to start of source memory.
 *             int DstPitch - Line stride in bytes of destination buffer.
 *             int SrcPitch - Line stride in bytes of source buffer.
 *             int ExtX - Horizontal extent in pixels for the blt.
 *             int ExtY - Vertical extent in pixels for the blt.
 *             int DstX - X pixel coordinate for start of blt.
 *             int DstY - Y pixel coordinate for start of blt.
 *             int SrcX - X pixel coordinate for start of blt.
 *             int SrcY - Y pixel coordinate for start of blt.
 *             int Bpp - Bpp value to use for blt. 8/16/32
 *
 * Returns: void
 *
 * Description: Performs a copy blt from one rectangular memory
 *              area to another rectangular memory area. The source and
 *              destination can have different strides, but both areas
 *              must have the same data format.
 ***************************************************************************/
void GfxCopyRectMem(u_int32 *pDest,
                u_int32 *pSrc,
                int DstPitch,
                int SrcPitch,
                int ExtX,
                int ExtY,
                int SrcX,
                int SrcY,
                int DstX,
                int DstY,
                int Bpp)
{
u_int32 Format;
u_int32 BltCmdType;
u_int32 SrcPitchPixels, DstPitchPixels;
u_int32 BltControl;

   sem_get(GXA_Sem_Id, KAL_WAIT_FOREVER);

   /* Translate bpp to hardware format */
   switch (Bpp) {
      case 8:
         Format = BM_FMT_RGB8;
         SrcPitchPixels = SrcPitch;
         DstPitchPixels = DstPitch;
         break;
      case 16:
         Format = BM_FMT_ARGB0565;
         SrcPitchPixels = SrcPitch >> 1;
         DstPitchPixels = DstPitch >> 1;
         break;
      case 32:
         Format = BM_FMT_ARGB32;
         SrcPitchPixels = SrcPitch >> 2;
         DstPitchPixels = DstPitch >> 2;
         break;
      default: /* Param error if here use 8 bpp values */
         Format = BM_FMT_RGB8;
         SrcPitchPixels = SrcPitch;
         DstPitchPixels = DstPitch;
   }

   /* Special code to detect source/dest overlap and decide if reverse blit */
   /* Not completely bulletproof as relies on psrc==pdst to detect          */
   if (pSrc == pDest)
   {
      /* If destY > srcY */
      if (DstY > SrcY)
      {
         /* If Y overlap */
         if (DstY < (SrcY + ExtY))
         {
            /* If X overlap */
            if (((SrcX >= DstX) && (SrcX < (DstX+ExtX))) ||
                ((DstX >= SrcX) && (DstX < (SrcX+ExtX))))
            {
               /* Backwards Y blit, fix up start and direction */
               SrcY = SrcY + ExtY - 1;
               DstY = DstY + ExtY - 1;
               mGetBltControl(BltControl)
               BltControl |= BLT_BACKWARDS;
               mSetBltControl(BltControl);

            } /* endif X overlap */
         } /* endif Y overlap */
      } /* endif desty > srcY*/
   } /* endif source and dest match */
   
   /* setup context register 1 for source  */
#ifdef PCI_GXA
   GxaSetupBitmapContext(1,BM_TYPE_COLOR,Format,
                     SrcPitchPixels,((u_int32)pSrc>>2));
#else
   GxaSetupBitmapContext(1,BM_TYPE_COLOR,Format,
                     SrcPitchPixels,(u_int32)pSrc);
#endif

   /* setup context register 0 for destination */
#ifdef PCI_GXA
   GxaSetupBitmapContext(0,BM_TYPE_COLOR, Format,
                     DstPitchPixels,((u_int32)pDest>>2));
#else
   GxaSetupBitmapContext(0,BM_TYPE_COLOR, Format,
                     DstPitchPixels,(u_int32)pDest);
#endif

   /* This sets up the BLT command and the number of parameters */
   BltCmdType = BLT_COPY;

   /* send the blt, source context 1, dest context 0, params 3 */
   mCheckFifo(4)
   mCmd2(GXA_BASE|BltCmdType|BMAP1_SRC|BMAP0_DST,
         mMakeXY(DstX,DstY),
         mMakeXY(ExtX,ExtY),
         mMakeXY(SrcX,SrcY))

   sem_put(GXA_Sem_Id);
}

/****************************************************************************
 * Function: GfxCopy2Screen()
 *
 * Parameters: u_int32 *pDest - Pointer to first byte of destination memory.
 *             u_int32 *pSrc - Pointer to first byte of source memory.
 *             int DstPitch - Pitch in bytes of rectangular destination buffer.
 *             int SrcPitch - Pitch in bytes of rectangular source buffer.
 *             int ExtX - Horizontal extent in pixels for the blt.
 *             int ExtY - Vertical extent in pixels for the blt.
 *             int DstX - X pixel coordinate for start of blt.
 *             int DstY - Y pixel coordinate for start of blt.
 *             int Bpp - Bpp value to use for blt. 4/8/16/32
 *
 * Returns: void
 *
 * Description: Performs a copy blt from an offscreen rectangular memory
 *              area to a rectangular area of a screen bitmap.
 ***************************************************************************/
void GfxCopy2Screen(u_int32 *pDest,
                u_int32 *pSrc,
                int DstPitch,
                int SrcPitch,
                int ExtX,
                int ExtY,
                int DstX,
                int DstY,
                int Bpp)
{

   if ( Bpp != 4 ) {
      GfxCopyRectMem(pDest, pSrc, DstPitch, SrcPitch, ExtX, ExtY,
                  0, 0, DstX, DstY, Bpp);
   } else {
     /* Software 4 bpp copy is handled here by calling bitblt.s */
     SWCopy2Screen((int)pDest, (int)pSrc, DstPitch, (int)SrcPitch,
        ExtX, ExtY, DstX, DstY, GCP_4BPP);
   }
}

/****************************************************************************
 * Function: GfxCopy2Mem()
 *
 * Parameters: u_int32 *pDest - Pointer to first byte of destination memory.
 *             u_int32 *pSrc - Pointer to first byte of source memory.
 *             int DstPitch - Pitch in bytes of rectangular destination buffer.
 *             int SrcPitch - Pitch in bytes of rectangular source buffer.
 *             int ExtX - Horizontal extent in pixels for the blt.
 *             int ExtY - Vertical extent in pixels for the blt.
 *             int SrcX - X pixel coordinate for start of blt.
 *             int SrcY - Y pixel coordinate for start of blt.
 *             int Bpp - Bpp value to use for blt. 4/8/16/32
 *
 * Returns: void
 *
 * Description: Performs a copy blt from a rectangular memory area of the
 *              screen to a rectangular offscreen memory area.
 ***************************************************************************/
void GfxCopy2Mem(u_int32 *pDest,
             u_int32 *pSrc,
             int DstPitch,
             int SrcPitch,
             int ExtX,
             int ExtY,
             int SrcX,
             int SrcY,
             int Bpp)
{

   if ( Bpp != 4 ) {
      GfxCopyRectMem(pDest, pSrc, DstPitch, SrcPitch, ExtX, ExtY,
                  SrcX, SrcY, 0, 0, Bpp);
   } else {
     /* Software 4 bpp copy is handled here by calling bitblt.s */
     SWCopy2Memory((int)pDest, (int)pSrc, (int)DstPitch, SrcPitch,
        ExtX, ExtY, SrcX, SrcY, GCP_4BPP);
   }
}

/****************************************************************************
 * Function: GfxPrepareBufferForGXA
 *
 * Parameters: u_int32 *addr - Pointer to first byte of image buffer        
 *             u_int32 size  - Size of the image buffer in bytes
 *
 * Returns: void
 *
 * Description: Ensures that any data written to the passed buffer via the
 *              cached address space is flushed out to real SDRAM thus 
 *              ensuring that blits from or to the buffer use the expected
 *              data.
 ***************************************************************************/
void GfxPrepareBufferForGXA (u_int8 *addr, u_int32 size)
{
  /*
   * This function is only useful if using the cache
   */
  #if MMU_CACHE_DISABLE == 0

    /*
     * Don't do anything on the ARM940T core
     */
    #if CPU_TYPE == AUTOSENSE || CPU_TYPE == CPU_ARM920T

      /*
       * If Brazos, ensure not Rev_A (940 core)
       */
      #if CPU_TYPE == AUTOSENSE
        if (GetCPUType() == CPU_ARM920T)
      #endif /* #if CPU_TYPE == AUTOSENSE */
        {
            /*
             * Finally, flush and invalidate the buffer area
             */
            FlushAndInvalDCacheRegion ((u_int8 *)((u_int32)addr & ~NCR_BASE), size);
        }

    #endif /* CPU_TYPE == AUTOSENSE || CPU_TYPE == CPU_ARM920T */

  #endif /* MMU_CACHE_DISABLE == 0 */
}

/***************************************************************************
                     PVCS Version Control Information
$Log: 
 15   mpeg      1.14        3/16/04 10:59:31 AM    Xin Golden      CR(s) 8567 :
        fixed parenthesis in gfxcopyrectmem so no warnings when build in 
       release.
 14   mpeg      1.13        3/8/04 1:32:06 PM      Steve Glennon   CR(s) 8529 :
        Fixed GfxCopyRectMem to support overlapping Y reverse blits
       
 13   mpeg      1.12        7/15/03 11:54:52 AM    Dave Wilson     SCR(s) 6914 
       :
       Added GfxPrepareBufferForGXA. This function is called to ensure that any
       data written to an image buffer by software is cleaned and flushed from 
       the
       cache prior to using the buffer as source or destination for a blit.
       
 12   mpeg      1.11        2/13/03 11:30:20 AM    Matt Korte      SCR(s) 5479 
       :
       Removed old header reference
       
 11   mpeg      1.10        9/6/01 11:34:30 AM     Quillian Rutherford SCR(s) 
       2600 2601 2602 :
       Change GfxIsGXAIdle to GfxIsGXACmdDone and changed the method of waiting
       to wait for the specific qmark sent in this function.
       
       
 10   mpeg      1.9         8/24/01 4:40:28 PM     Quillian Rutherford SCR(s) 
       2525 2544 2545 :
       Added the GXA access semaphore logic to the idle api.
       
       
 9    mpeg      1.8         8/23/01 1:18:32 PM     Quillian Rutherford SCR(s) 
       2525 2544 2545 :
       Put the GXA idle check function in this file so OTV builds have access 
       to it.
       
       
 8    mpeg      1.7         8/28/00 6:40:32 PM     Lucy C Allevato Fixes for 
       strict compiler checking of possible usage of variables before
       they are initialized.
       
 7    mpeg      1.6         7/10/00 6:39:42 PM     Lucy C Allevato Changed 
       meaning of pitch parameter passed to GfxFillMem and GfxCopy functions
       to be the pitch in bytes instead of the pitch in pixels. The pitch in 
       pixels
       is now calculated from the pitch in bytes when using hardware 
       acceleration.
       
 6    mpeg      1.5         6/28/00 4:32:34 PM     Lucy C Allevato Added usage 
       of GXA semaphore to serialize accesses that program HW regs.
       
 5    mpeg      1.4         6/6/00 6:04:54 PM      Lucy C Allevato Fixed 
       comment style to not use C++ line comment.
       
 4    mpeg      1.3         3/31/00 4:51:50 PM     Lucy C Allevato Updates for 
       name changes in colorado.h to match up with spec.
       
 3    mpeg      1.2         3/21/00 4:30:20 PM     Lucy C Allevato Updated call
        to HwSetupBitmapContext to GxaSetupBitmapContext.
       
 2    mpeg      1.1         3/7/00 6:15:58 PM      Lucy C Allevato Changed 
       exported library function name to GfxFillMem
       
 1    mpeg      1.0         3/6/00 1:26:50 PM      Lucy C Allevato 
$
 * 
 *    Rev 1.12   15 Jul 2003 10:54:52   dawilson
 * SCR(s) 6914 :
 * Added GfxPrepareBufferForGXA. This function is called to ensure that any
 * data written to an image buffer by software is cleaned and flushed from the
 * cache prior to using the buffer as source or destination for a blit.
 * 
 *    Rev 1.11   13 Feb 2003 11:30:20   kortemw
 * SCR(s) 5479 :
 * Removed old header reference
 * 
 *    Rev 1.10   06 Sep 2001 10:34:30   rutherq
 * SCR(s) 2600 2601 2602 :
 * Change GfxIsGXAIdle to GfxIsGXACmdDone and changed the method of waiting
 * to wait for the specific qmark sent in this function.
 * 
 * 
 *    Rev 1.9   24 Aug 2001 15:40:28   rutherq
 * SCR(s) 2525 2544 2545 :
 * Added the GXA access semaphore logic to the idle api.
 * 
 * 
 *    Rev 1.8   23 Aug 2001 12:18:32   rutherq
 * SCR(s) 2525 2544 2545 :
 * Put the GXA idle check function in this file so OTV builds have access to it.
 * 
 * 
 *    Rev 1.7   28 Aug 2000 17:40:32   eching
 * Fixes for strict compiler checking of possible usage of variables before
 * they are initialized.
 * 
 *    Rev 1.6   10 Jul 2000 17:39:42   eching
 * Changed meaning of pitch parameter passed to GfxFillMem and GfxCopy functions
 * to be the pitch in bytes instead of the pitch in pixels. The pitch in pixels
 * is now calculated from the pitch in bytes when using hardware acceleration.
 * 
 *    Rev 1.5   28 Jun 2000 15:32:34   eching
 * Added usage of GXA semaphore to serialize accesses that program HW regs.
 * 
 *    Rev 1.4   06 Jun 2000 17:04:54   eching
 * Fixed comment style to not use C++ line comment.
 * 
 *    Rev 1.3   31 Mar 2000 16:51:50   eching
 * Updates for name changes in colorado.h to match up with spec.
 * 
 *    Rev 1.2   21 Mar 2000 16:30:20   eching
 * Updated call to HwSetupBitmapContext to GxaSetupBitmapContext.
 * 
 *    Rev 1.1   07 Mar 2000 18:15:58   eching
 * Changed exported library function name to GfxFillMem
 * 
 *    Rev 1.0   06 Mar 2000 13:26:50   eching
 * Initial revision.
****************************************************************************/

