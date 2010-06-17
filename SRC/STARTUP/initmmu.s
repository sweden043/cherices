;****************************************************************************
;*                   CONEXANT PROPRIETARY AND CONFIDENTIAL                  *
;*                       SOFTWARE FILE/MODULE HEADER                        *
;*                    Conexant Systems Inc. (c) 1999-2003                   *
;*                                Austin, TX                                *
;*                           All Rights Reserved                            *
;****************************************************************************
;*                                                                          *
;* Filename:       initmmu.s                                                *
;*                                                                          *
;* Description:    Initialize the ARM MMU/MPU.                              *
;*                                                                          *
;* Author:         Tim Ross                                                 *
;*                                                                          *
;****************************************************************************
;* $Header: initmmu.s, 10, 7/22/03 6:17:24 PM, Tim White$
;****************************************************************************

;******************/
;* Include Files  */
;******************/
   GET stbcfg.a

   GET cpu.a
   GET board.a
   GET startup.a

   IMPORT FlushICache
   IMPORT FlushDCache
   IMPORT mmuGetState
   IMPORT mmuSetState

    IF (CPU_TYPE = AUTOSENSE) :LOR: (CPU_TYPE = CPU_ARM920T)
;*************************/
;* External Definitions  */
;*************************/
    IMPORT mmuSetPageTabBase
    IMPORT mmuSetFCSE
    IMPORT mmuSetDomainAccessControl
    IMPORT mmuInvalidateTLB

    IF PAGE_TABLE_INITIALIZATION = PHYSICAL_RAM
    IMPORT pagetable
    ENDIF ; PAGE_TABLE_INITIALIZATION = PHYSICAL_RAM
    ENDIF ; (CPU_TYPE = AUTOSENSE) :LOR: (CPU_TYPE = CPU_ARM920T)

   KEEP ; keep local symbols
   IF :DEF: |ads$version|
      AREA    Cache, CODE, READONLY
   ELSE
      AREA    Cache, CODE, READONLY, INTERWORK
   ENDIF


;********************************************************************
;*  InitMMUandCaches                                                *
;*                                                                  *
;*  DESCRIPTION:                                                    *
;*      Depending upon what CPU_TYPE is configured or detected,     *
;*      call the appropriate processor type routine to initialize   *
;*      the MMU and flush the caches.                               *
;*                                                                  *
;*  PARAMETERS:                                                     *
;*      None.                                                       *
;*                                                                  *
;*  RETURNS:                                                        *
;*      Nothing.                                                    *
;*                                                                  *
;*  REGISTER USE:                                                   *
;*      r0.                                                         *
;********************************************************************
    EXPORT InitMMUandCaches
InitMMUandCaches

    IF CPU_TYPE = AUTOSENSE
        mrc     p15, 0, r0, c0, c0, 0   ;get the ID register into r0
        tst     r0, #CPU_ARM940_BIT     ;test for a 940 CPU
        bne     InitMMUandCaches940     ;bne means it is a 940
        b       InitMMUandCaches920
    ENDIF

    IF CPU_TYPE = CPU_ARM920T
        b       InitMMUandCaches920
    ENDIF

    IF CPU_TYPE = CPU_ARM940T
        b       InitMMUandCaches940
    ENDIF



    IF (CPU_TYPE = AUTOSENSE) :LOR: (CPU_TYPE = CPU_ARM940T)
;********************************************************************
;*  InitMMUandCaches940                                             *
;*                                                                  *
;*  DESCRIPTION:                                                    *
;*      Initializes the MMU's memory regions, flushes and           *
;*      invalidates the data cache, determines the caching          *
;*      attributes of each region, and enables the caches.          *
;*                                                                  *
;*  PARAMETERS:                                                     *
;*      None.                                                       *
;*                                                                  *
;*  RETURNS:                                                        *
;*      Nothing.                                                    *
;*                                                                  *
;*  REGISTER USE:                                                   *
;*      r0-r1.                                                      *
;********************************************************************
    EXPORT InitMMUandCaches940
InitMMUandCaches940

        mov  r3, lr

    ;******************************
    ;* Define the memory regions. *
    ;******************************
    ;Set region 0 (lowest priority) to cover all address space
    ;as only data accessible.
        mov     r0, #CACHE_1GB_REGION:OR:CACHE_REGION_ENABLE
        mcr     CACHE_PROT_UNIT, 0, r0, CACHE_REGION_REG, CACHE_REGION0, 0
        bic     r0, r0, #CACHE_REGION_ENABLE
        mcr     CACHE_PROT_UNIT, 0, r0, CACHE_REGION_REG, CACHE_REGION0, 1

    ;Set region 1 to cover the SDRAM area (both non- and byte-swapped)
    ;as both data and instruction accessible.
        mov     r0, #CACHE_256MB_REGION:OR:CACHE_REGION_ENABLE
        mcr     CACHE_PROT_UNIT, 0, r0, CACHE_REGION_REG, CACHE_REGION1, 0
        mcr     CACHE_PROT_UNIT, 0, r0, CACHE_REGION_REG, CACHE_REGION1, 1

    ;Set region 2 to cover the non-cached SDRAM area (both non- and
    ;byte-swapped) as instruction and data accessible for execution
    ;of non-cached GCP routines.
        ldr     r0, =NCR_BASE:OR:CACHE_256MB_REGION:OR:CACHE_REGION_ENABLE
        mcr     CACHE_PROT_UNIT, 0, r0, CACHE_REGION_REG, CACHE_REGION2, 0
        mcr     CACHE_PROT_UNIT, 0, r0, CACHE_REGION_REG, CACHE_REGION2, 1

    ;Set regions 3 & 4 to cover the ROM area
    ;as both data and instruction accessible.
    ;NOTE:: Initially only Region3 is used for 256MB.  Once ROM is sized,
    ;Regions 3 & 4 are setup appropriately
        ldr     r0, =ROM_BASE:OR:CACHE_256MB_REGION:OR:CACHE_REGION_ENABLE
        mcr     CACHE_PROT_UNIT, 0, r0, CACHE_REGION_REG, CACHE_REGION3, 0
        mcr     CACHE_PROT_UNIT, 0, r0, CACHE_REGION_REG, CACHE_REGION3, 1
    ;Set region 4 to be empty until flash is sized.
        mov     r0, #0
        mcr     CACHE_PROT_UNIT, 0, r0, CACHE_REGION_REG, CACHE_REGION4, 0
        mcr     CACHE_PROT_UNIT, 0, r0, CACHE_REGION_REG, CACHE_REGION4, 1

    ;Disable all other regions
        mov     r0, #0
        mcr     CACHE_PROT_UNIT, 0, r0, CACHE_REGION_REG, CACHE_REGION5, 0
        mcr     CACHE_PROT_UNIT, 0, r0, CACHE_REGION_REG, CACHE_REGION5, 1
        mcr     CACHE_PROT_UNIT, 0, r0, CACHE_REGION_REG, CACHE_REGION6, 0
        mcr     CACHE_PROT_UNIT, 0, r0, CACHE_REGION_REG, CACHE_REGION6, 1
        mcr     CACHE_PROT_UNIT, 0, r0, CACHE_REGION_REG, CACHE_REGION7, 0
        mcr     CACHE_PROT_UNIT, 0, r0, CACHE_REGION_REG, CACHE_REGION7, 1


    ;*******************************************
    ;* Initialize the memory protection unit.  *
    ;*******************************************
    ;Set all 8 data & instruction areas to full access
        ldr     r0, =0xffff
        mcr     CACHE_PROT_UNIT, 0, r0, CACHE_ACCESS_REG, c0, 0
        mcr     CACHE_PROT_UNIT, 0, r0, CACHE_ACCESS_REG, c0, 1

    ;*********************************************
    ;* Define the cacheability for each region.  *
    ;*********************************************
    ;For bootloaders (i.e. codeldr), make region1 cacheable, and
    ;everything else uncacheable. Ensure ROM region has caching
    ;disabled in order for the flash.li utility to run since it
    ;can run in USER32 mode and cannot access CP15 to disable caches.

    ;For applications (i.e. startup), make region1 & region3
    ;cacheable, and everything else uncacheable.
        mov     r0, #0xA
        mcr     CACHE_PROT_UNIT, 0, r0, CACHE_CACHEABLE_REG, c0, 0
        mcr     CACHE_PROT_UNIT, 0, r0, CACHE_CACHEABLE_REG, c0, 1

    ;**********************************************
    ;* Define the write posting for each region.  *
    ;**********************************************
    ;Disable write posting for regions 0, 2, and 3.
    ;Enable write posting for region 1.
    IF ((MMU_CACHE_DISABLE = 0) :LAND: (MMU_CACHE_TYPE = CACHE_TYPE_WB))
      IF :LNOT:(:DEF:HAS_WB_CACHE_BUG)

        ;!!!!  TURN OFF WB CACHES AT THE MOMENT!!!!!!!
        ;mov     r0, #2      ; Region 1 only
        mov     r0, #0

      ELSE
        mov     r0, #0
      ENDIF
    ELSE
        mov     r0, #0
    ENDIF
        mcr     CACHE_PROT_UNIT, 0, r0, CACHE_POSTING_REG, c0, 0

    ;****************************
    ;* Invalidate both caches.  *
    ;****************************
        bl      FlushICache
        bl      FlushDCache

    ;**********************************************
    ;* Turn protection unit, I, and D caches on.  *
    ;**********************************************
    ; Read current state
        bl      mmuGetState

    ; Clear our bits we are about to modify
        ldr     r1, =CACHE_PROT_ENABLE:OR:CACHE_DCACHE_ENABLE:OR: \
                     CACHE_ICACHE_ENABLE:OR:CACHE_FASTBUS:OR:CACHE_ASYNC
        bic     r0, r0, r1

    ; Always run in ASYNC/FASTBUS mode for performance reasons (for real hardware
    ; and emulation environments)
        orr     r0, r0, #CACHE_FASTBUS:OR:CACHE_ASYNC

    ; Always enable the protection unit whether caches are enabled or not
        orr     r0, r0, #CACHE_PROT_ENABLE

    ; Conditionally enable the caches
    IF (MMU_CACHE_DISABLE = 0)
        orr     r0, r0, #CACHE_DCACHE_ENABLE
        orr     r0, r0, #CACHE_ICACHE_ENABLE
    ENDIF

    ; Write the new state back
        bl      mmuSetState
        bx      r3

    ENDIF ; (CPU_TYPE = AUTOSENSE) :LOR: (CPU_TYPE = CPU_ARM940T)




    IF (CPU_TYPE = AUTOSENSE) :LOR: (CPU_TYPE = CPU_ARM920T)
;********************************************************************
;*  InitMMUandCaches920                                             *
;*                                                                  *
;*  DESCRIPTION:                                                    *
;*      Initializes the MMU's memory regions, flushes and           *
;*      invalidates the data cache, determines the caching          *
;*      attributes of each region, and enables the caches.          *
;*                                                                  *
;*  PARAMETERS:                                                     *
;*      None.                                                       *
;*                                                                  *
;*  RETURNS:                                                        *
;*      Nothing.                                                    *
;*                                                                  *
;*  REGISTER USE:                                                   *
;*      r0-r1.                                                      *
;*
;*  Notes:
;*    The MMU Co-Processor is CP15 (p15)
;*    The MCR (write) and MRC(read) commands are used to manipulate
;*       the co-processor registers.  The format of these commands
;*       is as follows ...
;*
;* MCR   p15,op1,Rd,CRn,CRm,op2
;* MRC   p15,op1,Rd,CRn,CRm,op2
;*
;*    where op1, op2, and CRm specify command/register options, and
;*       should be 0, unless otherwise specified.
;*    Rd specifies the ARM register to use
;*    CRn specifies the co-processor registers to use
;*
;* There is a two-stage process to get data through the MMU.  First,
;* the cache is checked for valid data for the given Virtual Address.
;* If it is found, the data is returned.  If there is a cache-miss,
;* the Virtual Address is checked in the Translation Look-Ahead Buffer.
;* If it is found, the address is converted to a physical address, and
;* the access proceeds.  If not, the Translation Table is scanned for
;* the appropriate address range, and the section entry is processed,
;* and the appropriate information is placed in the TLB.  There are
;* separate caches and TLBs for both Data and Instruction fetches, but
;* a common TTB.  For more details, review the ARM920T TRM.
;********************************************************************
    EXPORT InitMMUandCaches920
InitMMUandCaches920

        mov  r3, lr

  IF MMU_CACHE_DISABLE = 0

    IF PAGE_TABLE_INITIALIZATION = VIRTUAL_SECTION_FORMAT
   
    ;Set up the virtual page table entries

DOMAIN_0    EQU 0
SIZE_1M     EQU 0x0000FFF0
SIZE_2M     EQU 0x0000FFE0
SIZE_4M     EQU 0x0000FFC0
SIZE_8M     EQU 0x0000FF80
SIZE_16M    EQU 0x0000FF00
SIZE_32M    EQU 0x0000FE00
SIZE_64M    EQU 0x0000FC00
SIZE_128M   EQU 0x0000F800
SIZE_256M   EQU 0x0000F000
SIZE_512M   EQU 0x0000E000
SIZE_1024M  EQU 0x0000C000
SIZE_2048M  EQU 0x00008000
ALL_ACCESS  EQU 0x0000000C
NO_ACCESS   EQU 0x00000000
CACHEABLE   EQU 0x00000002
BUFFERABLE  EQU 0x00000001
   
    ;* Region 0 - 256MB System Ram 
    ;*    Cacheable, writethrough or writeback, 0x00000000 to 0x0fffffff
      IF (MMU_CACHE_TYPE=CACHE_TYPE_WB)
        ;Writeback Mode
        ldr  r0, =(0x00000000 :OR: DOMAIN_0 :OR: SIZE_256M :OR: ALL_ACCESS :OR: CACHEABLE :OR: BUFFERABLE)
      ELSE
        ;Writethrough Mode
        ldr  r0, =(0x00000000 :OR: DOMAIN_0 :OR: SIZE_256M :OR: ALL_ACCESS :OR: CACHEABLE)
      ENDIF
        ldr  r1, =RST_HARDWARE_PAGETABLE_CTRL_REGION0_REG
        str  r0, [r1]
   
    ;* Region 1 - 256MB System RAM
    ;*    Non-Cacheable, Non-Bufferable, 0x10000000 to 0x1fffffff
        ldr  r0, =(0x10000000 :OR: DOMAIN_0 :OR: SIZE_256M :OR: ALL_ACCESS)
        ldr  r1, =RST_HARDWARE_PAGETABLE_CTRL_REGION1_REG
        str  r0, [r1]

    ;* Regions 2 & 3 - 256MB System ROM
    ;*    Cacheable, 0x20000000 to 0x2fffffff (from application code, i.e. startup)
    ;*    Non-Cacheable, 0x20000000 to 0x2fffffff (from bootloader code, i.e. codeldr)
    ;*
    ;* NOTE: will be set to cacheable by startup code
    ;*       of loaded image
        ldr  r0, =(0x20000000 :OR: DOMAIN_0 :OR: SIZE_256M :OR: ALL_ACCESS :OR: CACHEABLE)
        ldr  r1, =RST_HARDWARE_PAGETABLE_CTRL_REGION2_REG
        str  r0, [r1]
        mov  r0, #0
        ldr  r1, =RST_HARDWARE_PAGETABLE_CTRL_REGION3_REG
        str  r0, [r1]

    ;* Regions 4 & 5 - 768MB Chip Register Space, Peripherals
    ;*    Non-Cacheable, Non-Bufferable, 0x30000000 to 0x5fffffff
        ldr  r0, =(0x30000000 :OR: DOMAIN_0 :OR: SIZE_256M :OR: ALL_ACCESS)
        ldr  r1, =RST_HARDWARE_PAGETABLE_CTRL_REGION4_REG
        str  r0, [r1]
        ldr  r0, =(0x40000000 :OR: DOMAIN_0 :OR: SIZE_512M :OR: ALL_ACCESS)
        ldr  r1, =RST_HARDWARE_PAGETABLE_CTRL_REGION5_REG
        str  r0, [r1]

    ;* Regions 6 & 7 - 2560MB  NO ACCESS PERMITTED
    ;*     (Page Fault), 0x60000000 to 0xffffffff
        mov  r0, #0
        ldr  r1, =RST_HARDWARE_PAGETABLE_CTRL_REGION6_REG
        str  r0, [r1], #4
        str  r0, [r1]
    
    ;Use the virtual section table
        ldr  r0, =RST_HARDWARE_PAGETABLE_BASE

    ELSE

    ;Use the physical page table
        ldr  r0, =pagetable

    ENDIF    ; PAGE_TABLE_INITIALIZATION = VIRTUAL_SECTION_FORMAT

    ;Load MMU Translation Table Base Register (register c2)
        bl   mmuSetPageTabBase

    ; we use a flat translation, load FCSE to zero
        mov  r0, #0
        bl   mmuSetFCSE

    ;Load Domain Access Control register (register c3)
        ldr  r0, =0xffffffff             ; All domains enabled for full access
        bl   mmuSetDomainAccessControl

    ;Flush/Invalidate any TLB entries that might be hanging around, using
    ;the TLB Operations Register (register c8) and CRm = c7
    ; Note:  There shouldn't be any, but let's do this anyway.
        bl   mmuInvalidateTLB            ; This will invalidate all TLBs (D + I)
   
    ;Flush any cache entries that might be hanging around
    ; Note:  There shouldn't be any, but lets do this anyway.
        bl   FlushICache
        bl   FlushDCache

  ENDIF    ; MMU_CACHE_DISABLE = 0

    ;Enable Caches + MMU
    ; Read the current state of Control Register (c1) to use a r-m-w sequence
        bl   mmuGetState

    ; Or in the appropriate settings ...
    ; bit 31 - iA bit
    ; bit 30 - nF bit ... leave these alone for now, leave the default setup.
    ; bit 29:15 - reserved
    ; bit 14 - Round Robin Replacement (vs Random)
    ; bit 13 - V bit ... base location of exception registers moved to 0xffff0000
    ; bit 12 - I bit ... Enable I Cache
    ; bit 11:10 - reserved
    ; bit 9 - R bit ... see notes in MMU Translation Table header
    ; bit 8 - S bit ... see notes in MMU Translation Table header
    ; bit 7 - B bit ... Big Endian ... leave this alone
    ; bit 6:3 - reserved
    ; bit 2 - C bit ... Enable D Cache
    ; bit 1 - A bit ... Enable Alignment Checking
    ; bit 0 - M bit ... Enable MMU
    ;
    ; For now, enable async clocking, MMU, D+I Cache, Random Replacement
  IF MMU_CACHE_DISABLE = 0

        ldr  r1, =0xc0000000:OR:\
                  0x00001000:OR:\
                  0x00000004:OR:\
                  0x00000001
  ELSE

        ldr  r1, =0xc0000000      ;async clocking mode

  ENDIF    ; MMU_CACHE_DISABLE = 0

        orr  r0, r0, r1
        bl   mmuSetState
   
    ; we're done now, return
        bx   r3

    ENDIF ; (CPU_TYPE = AUTOSENSE) :LOR: (CPU_TYPE = CPU_ARM920T)


   END

;****************************************************************************
;* Modifications:
;* $Log: 
;*  10   mpeg      1.9         7/22/03 6:17:24 PM     Tim White       SCR(s) 
;*        7018 :
;*        The loaders use only instruction caching without MMU/MPU support.  
;*        Remove the
;*        support for using the MMU/MPU from the loader code.
;*        
;*        
;*  9    mpeg      1.8         7/22/03 2:31:10 PM     Tim White       SCR(s) 
;*        7017 :
;*        Remove access above 0x60000000 for Brazos Rev_B.
;*        
;*        
;*  8    mpeg      1.7         7/21/03 2:56:14 PM     Tim White       SCR(s) 
;*        7007 :
;*        Fix the SIZE_2048M definition to be 0x800 not 0x100 for the hardware
;*        virtual page mapping function used on the ARM920T cores.
;*        
;*        
;*  7    mpeg      1.6         7/10/03 10:38:04 AM    Larry Wang      SCR(s) 
;*        6924 :
;*        In function InitMMUandCaches920(), save return address in r3 
;*        regardless what value MMU_CACHE_DISABLE is.
;*        
;*  6    mpeg      1.5         6/24/03 6:38:32 PM     Tim White       SCR(s) 
;*        6831 :
;*        Add flash, hsdp, demux, OSD, and demod support to codeldrext.
;*        
;*        
;*  5    mpeg      1.4         6/5/03 5:29:58 PM      Tim White       SCR(s) 
;*        6660 6661 :
;*        Flash banks separately controlled by the 920 MMU using hardware 
;*        virtual pagemapping.
;*        
;*        
;*  4    mpeg      1.3         5/20/03 12:35:36 PM    Tim White       SCR(s) 
;*        6475 6476 :
;*        Remove reference to pagetable if hardware page mapping is used.
;*        
;*        
;*  3    mpeg      1.2         5/19/03 3:48:10 PM     Billy Jackman   SCR(s) 
;*        6418 6419 :
;*        Changed check of IMAGE_TYPE so it would assemble.
;*        Modified CPU type checking for the autosense case to not depend upon 
;*        stored
;*        values.
;*        
;*  2    mpeg      1.1         5/15/03 6:29:52 PM     Tim White       SCR(s) 
;*        6374 6375 :
;*        Last change left Data Cache disabled on the ARM940 cores...
;*        
;*        
;*  1    mpeg      1.0         5/14/03 4:39:02 PM     Tim White       
;* $
;* 
;*    Rev 1.9   22 Jul 2003 17:17:24   whiteth
;* SCR(s) 7018 :
;* The loaders use only instruction caching without MMU/MPU support.  Remove the
;* support for using the MMU/MPU from the loader code.
;* 
;*   
;*    Rev 1.8   22 Jul 2003 13:31:10   whiteth
;* SCR(s) 7017 :
;* Remove access above 0x60000000 for Brazos Rev_B.
;*   
;*   
;*    Rev 1.7   21 Jul 2003 13:56:14   whiteth
;* SCR(s) 7007 :
;* Fix the SIZE_2048M definition to be 0x800 not 0x100 for the hardware
;* virtual page mapping function used on the ARM920T cores.
;* 
;* 
;*    Rev 1.6   10 Jul 2003 09:38:04   wangl2
;* SCR(s) 6924 :
;* In function InitMMUandCaches920(), save return address in r3 regardless what value MMU_CACHE_DISABLE is.
;* 
;*    Rev 1.5   24 Jun 2003 17:38:32   whiteth
;* SCR(s) 6831 :
;* Add flash, hsdp, demux, OSD, and demod support to codeldrext.
;* 
;* 
;*    Rev 1.4   05 Jun 2003 16:29:58   whiteth
;* SCR(s) 6660 6661 :
;* Flash banks separately controlled by the 920 MMU using hardware virtual pagemapping.
;* 
;* 
;*    Rev 1.3   20 May 2003 11:35:36   whiteth
;* SCR(s) 6475 6476 :
;* Remove reference to pagetable if hardware page mapping is used.
;* 
;* 
;*    Rev 1.2   19 May 2003 14:48:10   jackmaw
;* SCR(s) 6418 6419 :
;* Changed check of IMAGE_TYPE so it would assemble.
;* Modified CPU type checking for the autosense case to not depend upon stored
;* values.
;* 
;*    Rev 1.1   15 May 2003 17:29:52   whiteth
;* SCR(s) 6374 6375 :
;* Last change left Data Cache disabled on the ARM940 cores...
;* 
;* 
;*    Rev 1.0   14 May 2003 15:39:02   whiteth
;* SCR(s) 6346 6347 :
;* ARM MMU/MPU and cache initialization routines callable from both application
;* and bootloader code.
;* 
;* 
;***** FILE COPIED FROM CODELDR ******
;* vlog -br K:\sabine\pvcs\CODELDR\cache.s_v(cache.s)
;*
;*    Rev 1.19   05 May 2003 15:51:42   whiteth
;* SCR(s) 6172 :
;* Combine duplicate boot code into startup directory.
;* 
;* 
;*    Rev 1.18   28 Apr 2003 16:26:14   jackmaw
;* SCR(s) 6112 :
;* Added PVCS Log history so it would all be available in checked out files.
;* 
;*    Rev 1.17   28 Apr 2003 16:03:02   jackmaw
;* SCR(s) 6112 :
;* Added PVCS $Header and $Log lines.
;* Added include of codeldr.a to get storage location for CPUType.
;* Moved the cache APIs InitMMUandCaches and FlushCaches into this file and
;* called the appropriate API functions in either cache_920t.s or cache_940t.s
;* depending upon what CPU_TYPE is configured.
;* Changed the condition for including cache_920t.s and cache_940t.s to include
;* the CPU_TYPE=AUTOSENSE case.
;* 
;*    Rev 1.16   17 Dec 2002 14:41:36   whiteth
;* SCR(s) 5182 :
;* Remove hwswitch.a include/get.
;* 
;*    Rev 1.15   01 May 2002 10:46:14   jackmaw
;* SCR(s) 3670 :
;* Changed GET hwconfig.a to GET stbcfg.a to conform to new configuration.
;* 
;*    Rev 1.14   18 Dec 2001 11:52:12   bradforw
;* SCR(s) 2933 :
;* Incorporating WabashBranch changes
;* 
;*    Rev 1.13   06 Nov 2001 09:23:04   bradforw
;* SCR(s) 2753 :
;* Added support for MANUAL SDRAM configuration
;* Also starting to remove any AUTO configuration that is not valid for WABASH
;* 
;*        Rev 1.13.1.0   17 Dec 2001 16:35:40   bradforw
;*     SCR(s) 2933 :
;*     Renamed a few FEATURES types (CPU_TYPE, PLL_TYPE, ARM_PIT_TYPE) for consistency
;* 
;*    Rev 1.12   01 Nov 2001 14:13:04   bradforw
;* SCR(s) 2828 :
;* Replaced GET sabine.a with GET hwconfig.a
;* Changed conditional code based on new CPU type FEATURE flag
;* 
;*    Rev 1.11   12 Oct 2001 12:39:32   bradforw
;* SCR(s) 2753 :
;* Split CACHE file into separate ARM940T and ARM920T specific files.
;* Since the MMU and CACHE control are significantly different, rather than
;* have a large number of conditional code switches, just make two separe files
;* that have the correct support for the specific ARM product.
;* 
;*    Rev 1.10   11 Oct 2001 17:36:52   whiteth
;* SCR(s) 2745 2765 2766 :
;* Changed region2 to be the NCR region and region3 to be the ROM region.
;* This has been wrong for a long time!
;* 
;*    Rev 1.9   08 Dec 1999 17:32:18   whiteth
;* Fixed cache flushing (cleaning) function.
;* 
;*    Rev 1.8   11 Nov 1999 14:32:42   whiteth
;* Turn off WB cache for ROM since it doesn't make sense.
;* 
;*    Rev 1.7   14 Oct 1999 11:13:32   whiteth
;* Ensure caches are disabled for the ROM region.
;* 
;*    Rev 1.6   14 Oct 1999 10:55:44   rossst
;* Removed Sabine code.
;* 
;*    Rev 1.5   13 Jul 1999 13:00:40   achen
;* Fixed a bug in RestMMU.. function that uses r3 which would be trashed later.
;* 
;*    Rev 1.4   02 Jul 1999 18:29:08   rossst
;* No functional changes. Just reorganized some code.
;* 
;*    Rev 1.3   28 Jun 1999 09:05:18   rossst
;* Removed all possible use of stacks.
;* 
;*    Rev 1.2   18 Jun 1999 15:00:02   dawilson
;* Neches setup changes.
;* 
;*    Rev 1.1   08 Jun 1999 17:12:44   rossst
;* Changed codeldr.a to hwswitch.a.
;* 
;*    Rev 1.0   08 Jun 1999 11:27:30   rossst
;* Initial revision.
;* 
;***** FILE COPIED FROM CODELDR ******
;* vlog -br K:\sabine\pvcs\CODELDR\cache_920t.s_v(cache_920t.s)
;* 
;*    Rev 1.12   30 Apr 2003 08:34:52   jackmaw
;* SCR(s) 6128 :
;* Removed an unconditional setting for the virtual page table register for
;* section 0 that had the effect of overriding the previous conditional setting.
;* This fixes a problem with the "bufferable" bit not always being set as
;* expected.
;* 
;*    Rev 1.11   28 Apr 2003 16:42:24   jackmaw
;* SCR(s) 6112 :
;* Added PVCS $Header and $Log lines.
;* Got rid of includes and AREA definition since cache.s, which now includes
;* cache_920t.s, has the includes and AREA definition.
;* Appended 920 to each entry point so that these routines could be included at
;* the same time as the routines in cache_940t.s to allow run-time autosensing of
;* the CPU type.
;* Added PVCS Log history so it would all be available in checked out files.
;* 
;*    Rev 1.10   26 Mar 2003 15:32:04   jackmaw
;* SCR(s) 5854 5857 :
;* If configuration item PAGE_TABLE_INITIALIZATION is set to VIRTUAL_SECTION_FORMAT
;* initialize the virtual section specification registers and use the virtual
;* section table instead of an actual page table in memory.
;* 
;*    Rev 1.9   07 May 2002 16:54:02   mooreda
;* SCR(s) 3724 :
;* removed comments on LDR stmt around line 132.
;* 
;*    Rev 1.8   26 Apr 2002 13:32:18   rossst
;* SCR(s) 3639 :
;* Changed InitMMUandCache function so that it always sets the CP15 control
;* register, whether caches are enabled or not. Also set the clocking control bits
;* to enable async. clocking of the CPU from the bus for both the caches on and
;* the caches off cases.
;* 
;*    Rev 1.7   05 Apr 2002 18:14:48   mooreda
;* SCR(s) 3458 :
;* added GET hwconfig.a
;* 
;*    Rev 1.6   28 Mar 2002 16:39:34   mooreda
;* SCR(s) 3403 :
;* Changed to Random Replacement Policy.
;* 
;*    Rev 1.5   28 Feb 2002 08:50:58   mooreda
;* SCR(s) 3265 :
;* Moved pagetable from this file to pagetab_920t.s
;* 
;*    Rev 1.4   08 Feb 2002 11:44:16   bintzmf
;* SCR(s) 3158 :
;* changed IFNDEF to IF MMU_CACHE_DISABLE = 0
;* 
;*    Rev 1.3   13 Nov 2001 12:59:16   bradforw
;* SCR(s) 2753 :
;* Changed startup default values for ROM region (0x20000000 - 0x2fffffff) to
;* non-cached, to match the ARM940 startup state.
;* 
;*    Rev 1.2   08 Nov 2001 15:40:32   bradforw
;* SCR(s) 2753 :
;* Added code to leave the MMU and CACHE disabled, if called to do so
;* 
;*    Rev 1.1   06 Nov 2001 09:23:08   bradforw
;* SCR(s) 2753 :
;* Added support for MANUAL SDRAM configuration
;* Also starting to remove any AUTO configuration that is not valid for WABASH
;* 
;*    Rev 1.0   12 Oct 2001 12:36:38   bradforw
;* SCR(s) 2753 :
;* 
;***** FILE COPIED FROM CODELDR ******
;* vlog -br K:\sabine\pvcs\CODELDR\cache_940t.s_v(cache_940t.s)
;*
;*    Rev 1.5   28 Apr 2003 16:51:00   jackmaw
;* SCR(s) 6112 :
;* Added PVCS $Header and $Log lines.
;* Got rid of includes and AREA definition since cache.s, which now includes
;* cache_940t.s, has the includes and AREA definition.
;* Appended 940 to each entry point so that these routines could be included at
;* the same time as the routines in cache_920t.s to allow run-time autosensing of
;* the CPU type.
;* Added PVCS Log history so it would all be available in checked out files.
;*
;*    Rev 1.4   22 Jan 2003 13:33:12   whiteth
;* SCR(s) 5099 :
;* Force 940 into WT cache mode for the time being...  There are numerous issues with
;* the software running in WB for the 940 based code.
;*
;*    Rev 1.3   17 Dec 2002 15:38:48   whiteth
;* SCR(s) 5182 :
;* Condense the 940ARM core setup of the CP15 control register and WB cache
;* control to using only the existing MMU_CACHE_DISABLE config option (created
;* by Wabash/920 code bringup) and the new MMU_CACHE_TYPE for controlling
;* whether to use WT or WB.  WB is the default, and WT will be used if
;* HAS_WB_CACHE_BUG defined which both Colorado & Hondo do (Pre-Rev_2 ARM940 cores).
;* Brazos is a Rev2 ARM940 core so WB should work.  Currently the 920 code does
;* not use the flag but should be made to do so in the future.
;*
;*    Rev 1.2   14 Nov 2002 10:29:46   dawilson
;* SCR(s) 4956 :
;* The MMU background region now only allows accesses to addresses below 1GB and
;* aborts all others. This works around a problem in Multi-ICE where an abort
;* generated by access to a high address could sometimes hang the debugger.
;*
;*    Rev 1.1   16 Nov 2001 14:55:10   bradforw
;* SCR(s) 2753 :
;* Changed the Cache Enable logic to allow protection unit to be enabled, even thou
;* gh Caches are not enabled.
;*
;*    Rev 1.0   12 Oct 2001 12:37:08   bradforw
;* SCR(s) 2753 :
;*
;****************************************************************************

