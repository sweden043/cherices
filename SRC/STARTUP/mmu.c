/****************************************************************************/
/*                   CONEXANT PROPRIETARY AND CONFIDENTIAL                  */
/*                       SOFTWARE FILE/MODULE HEADER                        */
/*                   Conexant Systems Inc. (c) 2002-2003                    */
/*                               Austin, TX                                 */
/*                            All Rights Reserved                           */
/****************************************************************************/
/*                                                                          */
/* Filename:   mmu.c                                                        */
/*                                                                          */
/* Description: ARM V4 MMU Support Library                                  */ 
/*                                                                          */ 
/* Author:       Dave Moore                                                 */
/*                                                                          */
/****************************************************************************/
/* $Header: mmu.c, 21, 9/26/03 5:30:10 PM, Miles Bintz$
 ****************************************************************************/
#include "hwconfig.h"

#if (RTOS==VXWORKS)
#include "cacheLib.h"
#include "vmLib.h"
#endif

#include "mmu.h"
#include "startup.h"
#include "kal.h"


#if (RTOS==VXWORKS)
extern u_int32 GetCP15Reg ( u_int32 cp15_regnum, u_int32 sec_opcode, u_int32 add_parm);

#if (CPU_TYPE==CPU_ARM940T) || (CPU_TYPE==AUTOSENSE)
#if (ARM_VERSION==220)   /* vxworks 940 MPU lib seems to have some problem. turn it off for now */
void EnableDCacheForRegion ( u_int32 Region )
{
   unsigned long cp15;
   unsigned long addr, length;
   /* find out the address and size of the region */
   cp15 = GetCP15Reg ( 6, 0, Region );
   addr = cp15 & 0xfffff000;
   length = ( 1 << ( ( ( cp15 & 0x3e ) >> 1 ) + 1 ) );

   /* enable cacheability */
   vmMpuStateSet ( NULL, (void*)addr, length,
#if (MMU_CACHE_TYPE == CACHE_TYPE_WT) || (HAS_WB_CACHE_BUG == 1)
     	VM_STATE_MASK_VALID | VM_STATE_MASK_WRITABLE | VM_STATE_MASK_CACHEABLE,
     	VM_STATE_VALID      | VM_STATE_WRITABLE      | VM_STATE_CACHEABLE_WRITETHROUGH
#else /* cache_type = write_back */
        VM_STATE_MASK_VALID | VM_STATE_MASK_WRITABLE | VM_STATE_MASK_CACHEABLE,
        VM_STATE_VALID      | VM_STATE_WRITABLE      | VM_STATE_CACHEABLE
#endif          
                 );
}

bool DisableDCacheForRegion ( u_int32 Region )
{
   unsigned long cp15;
   bool retval;
   unsigned long addr, length;

   /* read register 2 on CP15 */
   cp15 = GetCP15Reg ( 2, 0, 0 );

   /* check the cacheability of the region */
   retval = ( cp15 & ( 1 << Region ) ) ? 1 : 0;

   if ( retval )
   {
      /* find out the address and size of the region */
      cp15 = GetCP15Reg ( 6, 0, Region );
      addr = cp15 & 0xfffff000;
      length = ( 1 << ( ( ( cp15 & 0x3e ) >> 1 ) + 1 ) );

      /* it is cacheable, disable it */
      vmMpuStateSet ( NULL, (void*)addr, length,
                      VM_STATE_MASK_VALID | VM_STATE_MASK_WRITABLE | VM_STATE_MASK_CACHEABLE,
                      VM_STATE_VALID | VM_STATE_WRITABLE | VM_STATE_CACHEABLE_NOT );
   }

   return retval;
}
#endif /* (ARM_VERSION==220) */

#endif /* (CPU_TYPE==CPU_ARM940T) || (CPU_TYPE==AUTOSENSE) */

#if (CPU_TYPE==CPU_ARM920T) || (CPU_TYPE==AUTOSENSE)
/*
  FUNCTION: mmuModifyROMCacheability
 
  PARAMETERS:   fbank       - Flask bank
                on_off      - one of CACHEING_ON, CACHEING_OFF
                addr        - 1 MB aligned address
                num_of_megs - number of megabytes to modify
                              beginning at addr

  DESCRIPTION: Modifies Page Table Entries for ROM access
 
  RETURNS:  0 - Cacheability was off upon entry
            1 - Cacheability was on upon entry
 
*/

unsigned int mmuModifyROMCacheability ( unsigned int fbank,
                                        unsigned int on_off,
                                        unsigned int addr,
                                        unsigned int num_of_megs )
{
#if MMU_CACHE_DISABLE == 0
   UINT state = VM_STATE_VALID | VM_STATE_WRITABLE;
   UINT pg_tab;
   unsigned retval;

#if defined(DEBUG) && (RTOS != NOOS)
   if ( (addr & 0x000fffff) != (unsigned int)0 ) 
   {
       trace( "ERROR: mmu.c: addr=0x%8x:  Improper alignment\n",addr );  /* improper alignment */
	   fatal_exit( 0xdead0001 );
   }
   /* Sanity Check input... */
   if (num_of_megs > 256)
   {
       trace( "ERROR: mmu.c: (0x%x) num_of_megs??\n",num_of_megs );
	   fatal_exit( 0xdead0002 );
   }
   

#endif

   if (num_of_megs == 0)
       num_of_megs = 1;

   /* get level one descriptor */
   pg_tab = ( GetCP15Reg ( 2, 0, 0 ) & 0xffffc000 ) | ( ( addr & 0xfff00000 ) >> 18 );
   pg_tab = *(UINT*)pg_tab;

   /* get level two descriptot */
   pg_tab = ( pg_tab & 0xfffffc00 ) | ( ( addr & 0xff000 ) >> 10 );
   pg_tab = *(UINT*)pg_tab;

   retval = ( pg_tab & 0x8 ) ? 1 : 0;

   if ( ( retval && ( on_off == CACHEING_ON ) ) || ( !retval && ( on_off == CACHEING_OFF ) ) )
   {
      /* nothing needs to be done */
      return retval;
   }
    
   if ( on_off == CACHEING_ON )
   {
      state |= VM_STATE_CACHEABLE;
   }

   vmBaseStateSet ( NULL, (void*)addr, num_of_megs * 0x100000,
                    VM_STATE_MASK_VALID | VM_STATE_MASK_WRITABLE | VM_STATE_MASK_CACHEABLE,
                    state );

   return retval;
#else  /* MMU_CACHE_DISABLE == 0 */
   return 0;
#endif    /* MMU_CACHE_DISABLE == 0 */
}

#endif /* (CPU_TYPE==CPU_ARM920T) || (CPU_TYPE==AUTOSENSE) */

#else /* ( RTOS == VXWORKS ) */


#if (CPU_TYPE==CPU_ARM920T) || (CPU_TYPE==AUTOSENSE)

#if (PAGE_TABLE_INITIALIZATION==PHYSICAL_RAM)
/*
 * The pagetable has to be declared externally in armv4mmu.s
 * so it can be aligned to a 16kB boundary.
 * 
 * Note: extern unsigned long *pagetable; is NOT the same
 * thing (see ANSI C standard), and will not work in this
 * instance.
 */
   extern unsigned long pagetable[4096];

/* 

                      LEVEL ONE DESCRIPTOR
                      --------------------


   31              20 19          12 11 10 9 8	    5 4 3 2 1 0
   +----------------------------------------------------------+
   |                                                      |0|0|  Fault
   +----------------------------------------------------------+
   | Coarse Page Table Base Address       |0| Domain|1|   |0|1|  Coarse Page Table
   +----------------------------------------------------------+
   | Section Base Addr|              | AP |0| Domain|1|C|B|1|0|  Section
   +----------------------------------------------------------+
   | Fine Page Table Base Address    |0 0  0| Domain|1|   |1|1|  Fine Page Table
   +----------------------------------------------------------+


   AP = Access Permission Bits
   C,B = Cacheable, Bufferable
   Domain = selects one of 16 possible domains



   Interpretation of Cacheable(C_BIT) and Bufferable(B_BIT) Bits

   C_BIT   B_BIT           Meaning
   =====   =====  =========================================================
   0        0     Non-cached, Non-bufferable (NCNB)
   0        1     Non-cached, Bufferable (NCB) (writes placed in write buffer)
   1        0     Cacheable, Write-Thru (WT) (writes update cache and placed in write buffer)
   1        1     Cacheable, Write-Back (WB) (writes update cache and set dirty bits)
  
*/



/* 

 FUNCTION: CreateSectionEntry()

 PURPOSE: Create a Level-One Section Page Table Descriptor 

 NOTES:   Interrupts must be off, MMU and caches must be off

 PARAMETERS:

        addr is MVA
        dom  is associated domain
        ucb  is a mask of C_BIT(cacheable) and B_BIT(bufferable)
        acc  is one of NO_ACCESS, SVC_R, ALL_R, SVC_RW,
             NO_USR_W, or ALL_ACCESS

 RETURNS:
        0 - FAILED
		1 - SUCCESS
*/

unsigned int mmuCreateSectionEntry( unsigned int addr, 
                                    unsigned int dom, 
                                    unsigned int ucb, 
                                    unsigned int acc ) 
{
   /*  31              20 19             12 11 10  9 8 5 4 3 2 1 0 */
   /* +----------------------------------------------------------+ */
   /* | Section Base Addr|              | AP |0| Domain|1|C|B|1|0| */
   /* +----------------------------------------------------------+ */

#if MMU_CACHE_DISABLE == 0

#ifdef DEBUG
   if( addr & 0xfffff ) return( 0 );           /* improper alignment */
   if( (acc<<10) & 0xfffff3ff ) return( 0 );   /* bad access perm    */
   if( (dom<<5) & 0xfffffe1f ) return( 0 );    /* bad domain         */
   if( ucb & 0xfffffff3 ) return( 0 );         /* bad C,B bits       */
#endif

   /* divide addr by 1MB to get PTE */
   pagetable[addr >> 20] = (addr & (unsigned int)0xfff00000) | (acc << 10) | (dom << 5) |
                           (ucb | (unsigned int)U_BIT) | (unsigned int)0x2 ;
#endif
   return( 1 );

}

#endif /* (PAGE_TABLE_INITIALIZATION==PHYSICAL_RAM) */


/*
  FUNCTION: mmuModifyROMCacheability
 
  PARAMETERS:   fbank       - Flask bank
                on_off      - one of CACHEING_ON, CACHEING_OFF
                addr        - 1 MB aligned address
                num_of_megs - number of megabytes to modify
                              beginning at addr

  DESCRIPTION: Modifies Page Table Entries for ROM access
               (Data Accesses ONLY)
 
  RETURNS:  0 - Cacheability was off upon entry
            1 - Cacheability was on upon entry

  NOTES: One should notice that bufferability is not an option. This is, of course,
         because this function is meant to modify ROM (flash) space ONLY which we
		 never want to use with the write buffer on.
*/

unsigned int mmuModifyROMCacheability ( unsigned int fbank,
                                        unsigned int on_off,
                                        unsigned int addr,
                                        unsigned int num_of_megs )
{
#if MMU_CACHE_DISABLE == 0

   unsigned int  state, rval;
   unsigned int  cs;
#if (PAGE_TABLE_INITIALIZATION!=PHYSICAL_RAM)
   u_int32 proto;
#endif

#if defined(DEBUG) && (RTOS != NOOS)
   if ( (addr & 0x000fffff) != (unsigned int)0 ) 
   {
       trace( "ERROR: mmu.c: addr=0x%8x:  Improper alignment\n",addr );  /* improper alignment */
	   fatal_exit( 0xdead0001 );
   }
   /* Sanity Check input... */
   if (num_of_megs > 256)
   {
       trace( "ERROR: mmu.c: (0x%x) num_of_megs??\n",num_of_megs );
	   fatal_exit( 0xdead0002 );
   }
   

#endif

   if (num_of_megs == 0)
       num_of_megs = 1;

   /* block interrupts */
   cs = critical_section_begin();

   state = mmuGetState( );	/* Get CP15 Reg 1 */

   mmuSetState( state & ~(BOTH_CACHES | MMU_ENABLE) ); /* Both Caches and MMU off */

   /* Need to Flush/Invalidate the DCache */
   CleanDCache( );
   DrainWriteBuffer( );
   FlushDCache( );   

   /* And flush the ICache.. */
   FlushICache();

   /* Now, modify the Page Tables */

#if (PAGE_TABLE_INITIALIZATION==PHYSICAL_RAM)
   /* +----------------------------------------------------------+ */
   /* | Section Base Addr|              | AP |0| Domain|1|C|B|1|0| */ 
   /* +----------------------------------------------------------+ */
   rval = ( pagetable[addr >> 20] & (unsigned int)0x8 ) ? 1 : 0; /* return value indicates if cacheability */
                                                                 /* was on upon entry..                    */
   while( num_of_megs-- )
   {
      /* Enable/Disable cacheability */
	  if( on_off == CACHEING_ON )
	  {
        if ( !mmuCreateSectionEntry( addr, 0, C_BIT, ALL_ACCESS ) )
#if (RTOS == NOOS)
                   while(1);    /* lock up */
#else
		   fatal_exit( 0xdead0003 );
#endif
	  }
	  else
        if( !mmuCreateSectionEntry( addr, 0, 0, ALL_ACCESS ) )
#if (RTOS == NOOS)
                   while(1);    /* lock up */
#else
		   fatal_exit( 0xdead0004 );
#endif

      addr += 0x100000; /* add 1 MB */ 
   }

#else /* (PAGE_TABLE_INITIALIZATION==PHYSICAL_RAM) */
   /* Set return value based on current cache setting */
   rval = (*((LPREG)RST_HARDWARE_PAGETABLE_CTRL_REGION2_REG+fbank) >> RST_HARDWARE_PAGETABLE_CTRL_CACHEABLE_SHIFT) & 1;

   /* Use addr parameter to calculate address field of new register value. */
   proto = 0x0000000c | (addr & 0xfff00000);

   /* Use num_of_megs parameter to calculate size field of new register
    * value.  Total size needs to be a power of 2 for this implementation.
    */

   /* If num_of_megs is not a power of 2, make it one. */
   if ( (((num_of_megs ^ (num_of_megs-1)) + 1) >> 1) != num_of_megs )
   {
      num_of_megs = num_of_megs | (num_of_megs>>1);
      num_of_megs = num_of_megs | (num_of_megs>>2);
      num_of_megs = num_of_megs | (num_of_megs>>4);
      num_of_megs = num_of_megs | (num_of_megs>>8);
      num_of_megs++;
   }

   /* Convert num_of_megs to a mask for all bits above and including the single
      1 bit of num_of_megs. */
   num_of_megs = num_of_megs | (num_of_megs<<1);
   num_of_megs = num_of_megs | (num_of_megs<<2);
   num_of_megs = num_of_megs | (num_of_megs<<4);
   num_of_megs = num_of_megs | (num_of_megs<<8);
   proto |= (num_of_megs & 0xfff) << RST_HARDWARE_PAGETABLE_CTRL_SIZE_SHIFT;

   /* Use on_off parameter to set cache bit for new register value. */
   if( on_off == CACHEING_ON )
   {
      proto |= 1 << RST_HARDWARE_PAGETABLE_CTRL_CACHEABLE_SHIFT;
   }

   /* Write the prototype value to the section register. */
   *((LPREG)RST_HARDWARE_PAGETABLE_CTRL_REGION2_REG+fbank) = proto;

#endif /* (PAGE_TABLE_INITIALIZATION==PHYSICAL_RAM) */


   /* Flush the TLBs since the page tables have now been modified. */
   mmuInvalidateDTLB();
   mmuInvalidateITLB();

   mmuSetState( state ); /* Caches / MMU back to entry state */

   critical_section_end( cs );

   return( rval ? 1 : 0 );

#else /* MMU_CACHE_DISABLE == 0 */
   return( 0 );
#endif
}

#endif /* (CPU_TYPE==CPU_ARM920T) || (CPU_TYPE==AUTOSENSE) */

#endif   /* ( RTOS == VXWORKS ) */

/****************************************************************************
 * Modifications:
 * $Log: 
 *  21   mpeg      1.20        9/26/03 5:30:10 PM     Miles Bintz     SCR(s) 
 *        7562 :
 *        changed order of includes to fix macro redef warning
 *        
 *  20   mpeg      1.19        9/2/03 6:57:54 PM      Joe Kroesche    SCR(s) 
 *        7415 :
 *        removed unneeded header files that were causing PSOS warnings
 *        
 *  19   mpeg      1.18        7/22/03 6:17:26 PM     Tim White       SCR(s) 
 *        7018 :
 *        The loaders use only instruction caching without MMU/MPU support.  
 *        Remove the
 *        support for using the MMU/MPU from the loader code.
 *        
 *        
 *  18   mpeg      1.17        6/24/03 6:38:34 PM     Tim White       SCR(s) 
 *        6831 :
 *        Add flash, hsdp, demux, OSD, and demod support to codeldrext.
 *        
 *        
 *  17   mpeg      1.16        6/5/03 5:32:32 PM      Tim White       SCR(s) 
 *        6660 6661 :
 *        Flash banks separately controlled by the 920 MMU using hardware 
 *        virtual pagemapping.
 *        
 *        
 *  16   mpeg      1.15        5/22/03 5:14:54 PM     Tim White       SCR(s) 
 *        6551 6552 :
 *        Do not modify the addr/size fields of the ROM aperture virtual 
 *        hardware
 *        page mapping register REGION2 (0x300000A8) since it's shared by all
 *        flash ROM banks.  Disabling data cache on a ROM bank while the other
 *        is in I/O mode does not effect performance enough to warrant using
 *        multiple regions.
 *        
 *        
 *  15   mpeg      1.14        4/30/03 4:54:10 PM     Billy Jackman   SCR(s) 
 *        6113 :
 *        Modified conditional inclusion of some code to include the case of
 *        CPU_TYPE=AUTOSENSE, not just CPU_ARM920T and CPU_ARM940T.
 *        
 *  14   mpeg      1.13        4/25/03 4:32:24 PM     Billy Jackman   SCR(s) 
 *        5855 :
 *        Remove unused prototypes for Enable/DisableInterrupts.
 *        Only reference pagetable if there is a page table in RAM.
 *        Add code to mmuModifyROMCacheability to use the virtual section 
 *        registers of
 *        the Brazos rev B and later instead of modifying an actual page table.
 *        
 *  13   mpeg      1.12        4/14/03 5:58:42 PM     Miles Bintz     SCR(s) 
 *        6022 :
 *        Added (ARM_VERSION==220) around En/DisableDCacheForRegion functions
 *        
 *        
 *  12   mpeg      1.11        3/4/03 4:14:50 PM      Miles Bintz     SCR(s) 
 *        5666 :
 *        Switched to using the native vxw MPU functions
 *        
 *        
 *  11   mpeg      1.10        1/31/03 4:46:50 PM     Dave Moore      SCR(s) 
 *        5375 :
 *        Made corrections to cache handling in mmuModifyRomCacheability.
 *        Also fixed and improved some of the #ifdef DEBUG code.
 *        
 *  10   mpeg      1.9         1/17/03 5:37:58 PM     Tim Ross        SCR(s) 
 *        5269 :
 *        Added calls to mmuInvalidateITLB and mmuInvalidateDTLB to end of 
 *        mmuModifyROMCacheability routine to ensure that TLB(s) are flushed 
 *        each time
 *        the page tables are modified by this call.
 *        
 *  9    mpeg      1.8         9/3/02 7:48:04 PM      Matt Korte      SCR(s) 
 *        4498 :
 *        Remove warnings
 *        
 *  8    mpeg      1.7         7/3/02 12:16:10 PM     Larry Wang      SCR(s) 
 *        4110 :
 *        Remove C version of EnableDCacheForRegion().
 *        
 *  7    mpeg      1.6         6/24/02 5:44:24 PM     Larry Wang      SCR(s) 
 *        4081 :
 *        Implement vxWorks version of mmuModifyROMCacheability().
 *        
 *  6    mpeg      1.5         6/6/02 12:46:02 PM     Bob Van Gulick  SCR(s) 
 *        3947 :
 *        Remove calls to assert since they are not supported on Nucleus.  Use 
 *        "if" statements instead.
 *        
 *        
 *  5    mpeg      1.4         5/23/02 10:40:06 AM    Bobby Bradford  SCR(s) 
 *        3826 :
 *        Remove Warning Messages
 *        
 *  4    mpeg      1.3         5/13/02 12:31:46 PM    Larry Wang      SCR(s) 
 *        3740 :
 *        In function mmuModifyROMCacheability(), restore MMU/cache to its 
 *        original state instead of turning it on.
 *        
 *  3    mpeg      1.2         4/30/02 1:13:50 PM     Billy Jackman   SCR(s) 
 *        3656 :
 *        Added #includes for startup.h and kal.h to get prototypes for some 
 *        extern
 *        functions.  Got rid of unused extern FlushCaches.
 *        
 *  2    mpeg      1.1         4/5/02 6:19:46 PM      Dave Moore      SCR(s) 
 *        3458 :
 *        adjusted ifdef MMU_DISABLE coverage
 *        
 *        
 *  1    mpeg      1.0         4/1/02 8:53:18 AM      Dave Moore      
 * $
 * 
 *    Rev 1.20   26 Sep 2003 16:30:10   bintzmf
 * SCR(s) 7562 :
 * changed order of includes to fix macro redef warning
 * 
 *    Rev 1.19   02 Sep 2003 17:57:54   kroescjl
 * SCR(s) 7415 :
 * removed unneeded header files that were causing PSOS warnings
 * 
 *    Rev 1.18   22 Jul 2003 17:17:26   whiteth
 * SCR(s) 7018 :
 * The loaders use only instruction caching without MMU/MPU support.  Remove the
 * support for using the MMU/MPU from the loader code.
 * 
 * 
 *    Rev 1.17   24 Jun 2003 17:38:34   whiteth
 * SCR(s) 6831 :
 * Add flash, hsdp, demux, OSD, and demod support to codeldrext.
 * 
 * 
 *    Rev 1.16   05 Jun 2003 16:32:32   whiteth
 * SCR(s) 6660 6661 :
 * Flash banks separately controlled by the 920 MMU using hardware virtual pagemapping.
 *
 *
 *    Rev 1.15   22 May 2003 16:14:54   whiteth
 * SCR(s) 6551 6552 :
 * Do not modify the addr/size fields of the ROM aperture virtual hardware
 * page mapping register REGION2 (0x300000A8) since it's shared by all
 * flash ROM banks.  Disabling data cache on a ROM bank while the other
 * is in I/O mode does not effect performance enough to warrant using
 * multiple regions.
 * 
 * 
 *    Rev 1.14   30 Apr 2003 15:54:10   jackmaw
 * SCR(s) 6113 :
 * Modified conditional inclusion of some code to include the case of
 * CPU_TYPE=AUTOSENSE, not just CPU_ARM920T and CPU_ARM940T.
 * 
 *    Rev 1.13   25 Apr 2003 15:32:24   jackmaw
 * SCR(s) 5855 :
 * Remove unused prototypes for Enable/DisableInterrupts.
 * Only reference pagetable if there is a page table in RAM.
 * Add code to mmuModifyROMCacheability to use the virtual section registers of
 * the Brazos rev B and later instead of modifying an actual page table.
 * 
 *    Rev 1.12   14 Apr 2003 16:58:42   bintzmf
 * SCR(s) 6022 :
 * Added (ARM_VERSION==220) around En/DisableDCacheForRegion functions
 * 
 * 
 *    Rev 1.11   04 Mar 2003 16:14:50   bintzmf
 * SCR(s) 5666 :
 * Switched to using the native vxw MPU functions
 * 
 * 
 *    Rev 1.10   31 Jan 2003 16:46:50   mooreda
 * SCR(s) 5375 :
 * Made corrections to cache handling in mmuModifyRomCacheability.
 * Also fixed and improved some of the #ifdef DEBUG code.
 * 
 *    Rev 1.9   17 Jan 2003 17:37:58   rossst
 * SCR(s) 5269 :
 * Added calls to mmuInvalidateITLB and mmuInvalidateDTLB to end of 
 * mmuModifyROMCacheability routine to ensure that TLB(s) are flushed each time
 * the page tables are modified by this call.
 * 
 *    Rev 1.8   03 Sep 2002 18:48:04   kortemw
 * SCR(s) 4498 :
 * Remove warnings
 * 
 *    Rev 1.7   03 Jul 2002 11:16:10   wangl2
 * SCR(s) 4110 :
 * Remove C version of EnableDCacheForRegion().
 * 
 *    Rev 1.6   24 Jun 2002 16:44:24   wangl2
 * SCR(s) 4081 :
 * Implement vxWorks version of mmuModifyROMCacheability().
 * 
 *    Rev 1.5   06 Jun 2002 11:46:02   vangulr
 * SCR(s) 3947 :
 * Remove calls to assert since they are not supported on Nucleus.  Use "if" statements instead.
 * 
 * 
 *    Rev 1.4   23 May 2002 09:40:06   bradforw
 * SCR(s) 3826 :
 * Remove Warning Messages
 * 
 *    Rev 1.3   13 May 2002 11:31:46   wangl2
 * SCR(s) 3740 :
 * In function mmuModifyROMCacheability(), restore MMU/cache to its original state instead of turning it on.
 * 
 *    Rev 1.2   30 Apr 2002 12:13:50   jackmaw
 * SCR(s) 3656 :
 * Added #includes for startup.h and kal.h to get prototypes for some extern
 * functions.  Got rid of unused extern FlushCaches.
 * 
 *    Rev 1.1   05 Apr 2002 18:19:46   mooreda
 * SCR(s) 3458 :
 * adjusted ifdef MMU_DISABLE coverage
 * 
 * 
 *    Rev 1.0   01 Apr 2002 08:53:18   mooreda
 * SCR(s) 3457 :
 * ARM V4 MMU Support Functions
 * 
 *
 ****************************************************************************/

