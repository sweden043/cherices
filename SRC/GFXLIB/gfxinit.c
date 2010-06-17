/***************************************************************************
                                                                          
   Filename: GFXINIT.C

   Description: CN49XX Graphics Library Hardware Init
                Low level hardware initialization
   Exported Functions:
   void GfxInit

   Created: 9/15/1999 by Eric Ching

   Copyright Conexant Systems, 1999
   All Rights Reserved.

****************************************************************************/
/***************************************************************************
$Header: gfxinit.c, 7, 2/13/03 11:54:16 AM, Matt Korte$
****************************************************************************/

#include "stbcfg.h"
#include "kal.h"
#include "retcodes.h"
#include "trace.h"
#include "gfxlib.h"
#include "genmac.h"
#include "context.h"

sem_id_t GXA_Sem_Id = 0;

extern void GXAIsrInit(void);

#ifdef PCI_GXA
#include "pcigxa.h"

/* I/O routines only needed for 2164 PCI card */

/* I/O write byte */
void OUTB(unsigned short port, unsigned char value)
{
   *(unsigned char *)(IO_BASE | port) = value;
}

/* I/O read byte */
unsigned char INB( unsigned short port)
{
   return (*((unsigned char *)(IO_BASE | port)));
}

/* Function: UnlockExt */
void UnlockExt(void)
{
   OUTB(SEQ_INDEX,SEQ_UNLOCK);  // index seq unlock reg
   OUTB(SEQ_DATA,0x12);         // unlock extension regs
}

/* Function: LockExt */
void LockExt(void)
{
   OUTB(SEQ_INDEX, SEQ_UNLOCK); // index seq unlock reg
   OUTB(SEQ_DATA, 0xff);        // lock 'em up
}

/***************************************************************************
 InitPCICard
   Initialize the 2164 PCI card for GUI operations
   The VGA and SCT/PACDAC are not setup so no display is available
   Only the initialization required after POR to configure and access
   the GUI registers/command space is done
***************************************************************************/
void InitPCICard (void)
{

   UnlockExt();

   /* Set memory refresh value */
   OUTB(GRP_INDEX,GRP_CFG_2);

   trace("2164 Mem Refresh: %x\n",INB(GRP_DATA));
   OUTB(GRP_DATA,0x04);

   /* Set GUI base and linear memory aperture */
   OUTB(GRP_INDEX,GRP_GUI_3);
   OUTB(GRP_DATA,PM_BASE|PM_ENABLE);

   /* Set CFG4 to enable just the GUI */
   OUTB(GRP_INDEX,GRP_CFG_4);
   OUTB(GRP_DATA,0x01);

   LockExt();

}

#endif // PCI_GXA

/***************************************************************************
 GfxInit
***************************************************************************/
void GfxInit (u_int8 Bpp)
{

#ifdef PCI_GXA
u_int32 PixelDepth;

   /* Initialize the 2164 PCI card for GUI operations */
   InitPCICard();

   /* Setup the GUI Fifo register for no dram queue */
   mLoadReg(GFIFO_CTRL_REG,GXA_FIFO_DEFAULT)

   /* Setup the GUI config register for the optimizations, transparency,
    * and pixel depth to use.
    */
   switch (Bpp) {
      case 8:
         PixelDepth = DEPTH_8BPP;
         break;
      case 16:
         PixelDepth = DEPTH_16BPP_0565;
         break;
      case 32:
         PixelDepth = DEPTH_32BPP;
         break;
      default:
         PixelDepth = DEPTH_8BPP;
   }
   /* Setup the GXA_CONFIG register */
   mLoadReg(GXA_CFG_REG,GXA_CFG_DEFAULT|PixelDepth)

#else
   /* Setup the GXA_CONFIG register */
   mLoadReg(GXA_CFG_REG,GXA_CFG_DEFAULT)

   /* Setup the GXA_CONFIG2 register */
   mLoadReg(GXA_CFG2_REG,GXA_CFG2_DEFAULT)

#endif

   /* Initialize the blit control, line control, and dest xy increment regs */
   mLoadReg(GXA_BLT_CONTROL_REG,0)
   mLoadReg(GXA_LINE_CONTROL_REG,LINE_CONTROL_DEFAULT)
   mLoadReg(GXA_DST_XY_INC_REG,0)

   mWaitForIdle

   if ( Bpp > 8) {
      mSetFGColor(0xFFFFFFFF)
   } else {
      mSetFGColor(0x0F0F0F0F)
   }
   mSetBGColor(0)

   mSetLinePattern(0xFFFFFFFF)

   /* If GXA_Sem_Id is 0 allocate a semaphore for serializing
    * access to the GXA registers.
    */
   if ( GXA_Sem_Id == 0 )
   {
      if ((GXA_Sem_Id = sem_create(1, "GXA1")) == 0)
      {
         trace_new(TRACE_GCP,"GXA: Sem Create Failed\n");
         error_log(ERROR_FATAL|MOD_GXA);
      }
      
      /* Only initialise the ISR if this is the first call to GfxInit (as */
      /* indicated by the fact that the semaphore ID was 0)               */
    	GXAIsrInit();
   }
}
/***************************************************************************
                     PVCS Version Control Information
$Log: 
 7    mpeg      1.6         2/13/03 11:54:16 AM    Matt Korte      SCR(s) 5479 
       :
       Removed old header reference
       
 6    mpeg      1.5         1/10/02 4:10:26 PM     Dave Wilson     SCR(s) 3019 
       3020 3021 :
       Many apps cause GfxInit to be called twice by calling OsdInit (which 
       calls
       the API internally) then calling GfxInit. The previous version had no
       protection to prevent problems in this case and GXAIsrInit would be 
       called 
       twice resulting in a trashed semaphore handle. This latest version only 
       calls
       GXAIsrInit on the first call to GfxInit, thus correcting the semaphore 
       problem.
       
 5    mpeg      1.4         8/24/01 4:08:16 PM     Quillian Rutherford SCR(s) 
       2525 2544 2545 :
       Added call to initialize the isr handler
       
       
 4    mpeg      1.3         6/28/00 4:30:06 PM     Lucy C Allevato Added 
       creation of GXA semaphore for serializing access to the HW during calls
       that modify the hardware registers.
       
 3    mpeg      1.2         6/6/00 6:04:38 PM      Lucy C Allevato Fixed 
       comment style.
       
 2    mpeg      1.1         3/31/00 4:51:48 PM     Lucy C Allevato Updates for 
       name changes in colorado.h to match up with spec.
       
 1    mpeg      1.0         3/6/00 2:08:06 PM      Lucy C Allevato 
$
 * 
 *    Rev 1.6   13 Feb 2003 11:54:16   kortemw
 * SCR(s) 5479 :
 * Removed old header reference
 * 
 *    Rev 1.5   10 Jan 2002 16:10:26   dawilson
 * SCR(s) 3019 3020 3021 :
 * Many apps cause GfxInit to be called twice by calling OsdInit (which calls
 * the API internally) then calling GfxInit. The previous version had no
 * protection to prevent problems in this case and GXAIsrInit would be called 
 * twice resulting in a trashed semaphore handle. This latest version only calls
 * GXAIsrInit on the first call to GfxInit, thus correcting the semaphore 
 * problem.
 * 
 *    Rev 1.4   24 Aug 2001 15:08:16   rutherq
 * SCR(s) 2525 2544 2545 :
 * Added call to initialize the isr handler
 * 
 * 
 *    Rev 1.3   28 Jun 2000 15:30:06   eching
 * Added creation of GXA semaphore for serializing access to the HW during calls
 * that modify the hardware registers.
 * 
 *    Rev 1.2   06 Jun 2000 17:04:38   eching
 * Fixed comment style.
 * 
 *    Rev 1.1   31 Mar 2000 16:51:48   eching
 * Updates for name changes in colorado.h to match up with spec.
 * 
 *    Rev 1.0   06 Mar 2000 14:08:06   eching
 * Initial revision.
****************************************************************************/

