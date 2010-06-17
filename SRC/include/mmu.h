/****************************************************************************/
/*                   CONEXANT PROPRIETARY AND CONFIDENTIAL                  */
/*                       SOFTWARE FILE/MODULE HEADER                        */
/*                 Conexant Systems Inc. (c) 1998, 1999, 2000, 2001, 2002   */
/*                               Austin, TX                                 */
/*                            All Rights Reserved                           */
/****************************************************************************/
/*
 * Filename:       mmu.h
 *
 *
 * Description:    Defines for armv4mmu functions
 *                 
 *
 * Author:         Dave Moore
 *
 ****************************************************************************/
/* $Header: mmu.h, 3, 6/5/03 5:30:16 PM, Tim White$
 ****************************************************************************/

#ifndef _MMU_H
#define _MMU_H

/* These defines are used for the functions MMUSetState / MMUGetState */
#define ASYNC_CLOCK_SELECT (1<<31)
#define NOTFASTBUS_SELECT (1<<30)
#define ROUND_ROBIN (1<<14)
#define ICACHE (1<<12)
#define ROM_PROTECTION (1<<9)
#define SYSTEM_PROTECTION (1<<8)
#define BIG_ENDIAN (1<<7)
#define DCACHE (1<<2)
#define DATA_ALIGN_FAULT_ENABLE (1<<1)
#define MMU_ENABLE 1
#define BOTH_CACHES (ICACHE | DCACHE)

/*
   Interpretation of Cacheable(C_BIT) and Bufferable(B_BIT) Bits

   C_BIT   B_BIT           Meaning
   =====   =====  =========================================================
   0        0     Non-cached, Non-bufferable (NCNB)
   0        1     Non-cached, Bufferable (NCB) (writes placed in write buffer)
   1        0     Cacheable, Write-Thru (WT) (writes update cache and placed in write buffer)
   1        1     Cacheable, Write-Back (WB) (writes update cache and set dirty bits)
  
*/
#ifdef C_BIT
  #undef C_BIT /* target/h/arch/arm/arm.h */
#endif
#define C_BIT 8  /* Cacheable */
#define B_BIT 4  /* Bufferable */
#define U_BIT 16 /* Always set on 920 */

/*  Access Permissions - not shifted into position. */

#define NO_ACCESS  0  /*  Depending on the 'R' and 'S' bit, 0 */
#define SVC_R      0  /*  represents one of these access      */
#define ALL_R      0  /*  permissions                         */ 
#define SVC_RW     1
#define NO_USR_W   2
#define ALL_ACCESS 3

/* passed to mmuModifyROMCacheability() */
#define CACHEING_ON 1
#define CACHEING_OFF 2

/* these are in HWLIB\mmu.s */
extern int mmuSetPageTabBase( unsigned int addr );
extern unsigned int mmuGetPageTabBase( void );
extern void mmuSetDomainAccessControl( unsigned int mask );
extern unsigned int mmuGetDomainAccessControl( void );
extern void mmuDrainWriteBuffer( void );
extern void mmuInvalidateITLB( void );
extern void mmuInvalidateDTLB( void );
extern void mmuInvalidateITLBEntry( unsigned int mva );
extern void mmuInvalidateDTLBEntry( unsigned int mva );
extern unsigned int mmuGetFCSE( void );
extern unsigned int mmuSetFCSE( unsigned int mask );
extern unsigned int mmuGetState( void );
extern unsigned int mmuSetState( unsigned int mask );

#endif /* _MMU_H */


/****************************************************************************
 * Modifications:
 * $Log: 
 *  3    mpeg      1.2         6/5/03 5:30:16 PM      Tim White       SCR(s) 
 *        6660 6661 :
 *        Flash banks separately controlled by the 920 MMU using hardware 
 *        virtual pagemapping.
 *        
 *        
 *  2    mpeg      1.1         9/3/02 6:25:10 PM      Dave Moore      SCR(s) 
 *        4513 :
 *        Fix compiler warning.
 *        
 *        
 *  1    mpeg      1.0         4/1/02 8:54:12 AM      Dave Moore      
 * $
 * 
 *    Rev 1.2   05 Jun 2003 16:30:16   whiteth
 * SCR(s) 6660 6661 :
 * Flash banks separately controlled by the 920 MMU using hardware virtual pagemapping.
 * 
 * 
 *    Rev 1.1   03 Sep 2002 17:25:10   mooreda
 * SCR(s) 4513 :
 * Fix compiler warning.
 * 
 * 
 *    Rev 1.0   01 Apr 2002 08:54:12   mooreda
 * SCR(s) 3457 :
 * ARM V4 MMU Support Functions Header File
 * 
 *
 ****************************************************************************/

