;****************************************************************************
;*                   CONEXANT PROPRIETARY AND CONFIDENTIAL                  *
;*                       SOFTWARE FILE/MODULE HEADER                        *
;*                    Conexant Systems Inc. (c) 2002-2003                   *
;*                                Austin, TX                                *
;*                           All Rights Reserved                            *
;****************************************************************************
;*                                                                          *
;* Filename:       pagetab_920t.s                                           *
;*                                                                          *
;* Description:    ARM920T Page Tables                                      *
;*                                                                          *
;* Author:         Dave Moore                                               *
;*                                                                          *
;****************************************************************************
;* $Header: pagetab_920t.s, 5, 7/22/03 6:17:26 PM, Tim White$
;****************************************************************************
;
;
;   MMU accesses are primarily controller thru the use of domains. There are
;   16 domains and each has a 2 bit field to access it. Two types of users are
;   supported, client and manager. The domains are defined in the domain access
;   control register.
;
;              INTERPRETING DOMAIN ACCESS CONTROL REGISTER
;
;   31 29 27 25 23 21 19 17 15 13 11 09 07 05 02 01      Bit#
;   +-----------------------------------------------+
;   |15|14|13|12|11|10|09|08|07|06|05|04|03|02|01|00|    Domain#
;   +-----------------------------------------------+
;
;    Value   Meaning     Description
;    -----   -------     -----------
;     00     No Access   Any access generates Domin fault
;     01     Client      Accesses are checked against Access Permission (AP)
;                        bits in the section or page descriptor
;     10     Reserved    Reserved. Currently behaves like the No Access mode.
;     11     Manager     Access are not checked against the access permission bits
;                        so a permission fault cannot be generated.
;
;
;
;                      LEVEL ONE DESCRIPTOR
;                      --------------------
;
;
;   31              20 19          12 11 10 9 8	    5 4 3 2 1 0
;   +----------------------------------------------------------+
;   |                                                      |0|0|  Fault
;   +----------------------------------------------------------+
;   | Coarse Page Table Base Address       |0| Domain|1|   |0|1|  Coarse Page Table
;   +----------------------------------------------------------+
;   | Section Base Addr|              | AP |0| Domain|1|C|B|1|0|  Section
;   +----------------------------------------------------------+
;   | Fine Page Table Base Address    |0 0  0| Domain|1|   |1|1|  Fine Page Table
;   +----------------------------------------------------------+
;
;
;   AP = Access Permission Bits
;   C,B = Cacheable, Bufferable
;   Domain = selects one of 16 possible domains
;
;   C   B    Meaning
;   ================
;   0   0    Non-cached, Non-bufferable (NCNB)
;   0   1    Non-cached, Bufferable (NCB) (writes placed in write buffer)
;   1   0    Cacheable, Write-Thru (WT) (writes update cache and placed in write buffer)
;   1   1    Cacheable, Write-Back (WB) (writes update cache and set dirty bits)
;  
;
;              

;******************
;* Include Files  *
;******************
    GET stbcfg.a

    KEEP ; keep local symbols

   IF CPU_TYPE <> CPU_ARM940T :LAND: PAGE_TABLE_INITIALIZATION = PHYSICAL_RAM

;---------------------------------------------------------------+

; PageTableInROM defines whether or not we declare the AREA containing
; the page table as DATA or READONLY CODE. If we declare it as DATA, the
; page table is copied to ram. If we declare it as CODE, the pagetable is
; kept in ROM.

                GBLL     PageTableInROM

; If application image, pagetable must be in RAM, since it can be modified.
; If codeldrext image, pagetable must be in RAM, since it can be modified.
; If basic codeldr image, pagetable can be in ROM, since it's not modified.

PageTableInROM      SETL     {FALSE}

;---------------------------------------------------------------+

; BlankPageTable defines whether 'pagetab_920t.s' exports an empty table,
; or a completed page table. 

                GBLL     BlankPageTable

BlankPageTable      SETL     {FALSE}

;---------------------------------------------------------------+

; Generate either a blank page table for the C code to fill in, or
; generate a full page table at assemble-time

    IF BlankPageTable

    ;-------------------------------------------------
    ; Generate a blank page table with the % directive
    ;-------------------------------------------------

    IF PageTableInROM
    AREA    pagetab, ALIGN=14, CODE, NOINIT
    ELSE
    AREA    pagetab, ALIGN=14, DATA, NOINIT
    ENDIF

    EXPORT  pagetable
pagetable
    % 4*4096

    ELSE

    ;------------------------------------------------------
    ; Generate a full page table, using repetitive assembly
    ;------------------------------------------------------

; Access Permissions - not shifted into position.
NO_ACCESS  EQU 0 ;  Depending on the 'R' and 'S' bit, 0
SVC_R      EQU 0 ;  represents one of these access
ALL_R      EQU 0 ;  permissions 
SVC_RW     EQU 1
NO_USR_W   EQU 2
ALL_ACCESS EQU 3

; U, C, and B bits in their correct positions 
U_BIT      EQU 16
C_BIT      EQU 8    ; cacheable
B_BIT      EQU 4    ; bufferable

; Level One Page Table Entry Type
FINE       EQU 3
SECTION    EQU 2
PAGE       EQU 1
INVALID    EQU 0

;-----------------------------------------------------------------
; MACRO to generate a Level One Page Table Entry
;
; Usage: L1Entry $type, $addr, $dom, $ucb, $acc 
;
;        $type is one of SECTION, FINE, PAGE, INVALID
;        $addr is MVA
;        $dom  is associated domain
;        $ucb  is a mask of C_BIT(cacheable) and B_BIT(bufferable)
;        $acc  is one of NO_ACCESS, SVC_R, ALL_R, SVC_RW,
;              NO_USR_W, or ALL_ACCESS
;-----------------------------------------------------------------

    MACRO
    L1Entry $type, $addr, $dom, $ucb, $acc
    [ $type = SECTION
      DCD ( (($addr) :AND: &FFF00000) :OR: \
        (($acc) :SHL: 10) :OR: \
        (($dom) :SHL: 5) :OR: \
        (($ucb) :OR: U_BIT) :OR: $type )
      MEXIT
    ]
    [ $type = PAGE
      DCD ( (($addr) :AND: &FFFFFC00) :OR: \
        (($dom) :SHL: 5) :OR: \
        (($ucb) :OR: U_BIT) :OR: $type )
      MEXIT
    ]
    [ $type = FINE
      DCD ( (($addr) :AND: &FFFFF000) :OR:  \
        (($dom) :SHL: 5) :OR: \
        U_BIT :OR: $type )
      MEXIT
    ]
    [ $type = INVALID
      DCD (($addr) :AND: &FFFFFFFC)
    ]
    MEND


;-------------------------------------------------
;   Initialized Page Table
;-------------------------------------------------

    IF PageTableInROM 
    AREA    pagetab, ALIGN=14, CODE, READONLY
    ELSE
    AREA    pagetab, ALIGN=14, DATA, READWRITE
    ENDIF


; Create the pagetable using repetitive assembly. For codeldr, just create a table
; of "section" entries. True "page tables" would need to be created in a
; similar fashion.

    EXPORT      pagetable
pagetable

    GBLA    counter

counter SETA 0

    ; Region 0 - 256MB System Ram 
    ;    Cacheable, Bufferable, 0x00000000 to 0x0fffffff

    WHILE counter < 256
	;* Write-Back Mode
 IF (MMU_CACHE_TYPE=CACHE_TYPE_WB)
    L1Entry SECTION, (counter:SHL:20), 0, C_BIT+B_BIT, ALL_ACCESS
 ELSE
	;* Write-Thru Mode
    L1Entry SECTION, (counter:SHL:20), 0, C_BIT, ALL_ACCESS
 ENDIF
counter SETA counter + 1
    WEND

    ;* Region 1 - 256MB System RAM
    ;*    Non-Cacheable, Non-Bufferable, 0x10000000 to 0x1fffffff

    WHILE counter < 512
    L1Entry SECTION, (counter:SHL:20), 0, 0, ALL_ACCESS
counter SETA counter + 1
    WEND

    ;* Region 2 - 256MB System ROM
    ;*    Non-Cacheable, 0x20000000 to 0x2fffffff (boot loader, e.g. codeldr)
    ;*    Cacheable, 0x20000000 to 0x2fffffff (application, startup)
	;*
	;* NOTE: will be set to cacheable by startup code
	;*       of loaded image

    WHILE counter < 768
    L1Entry SECTION, (counter:SHL:20), 0, C_BIT, ALL_ACCESS
counter SETA counter + 1
    WEND

    ;* Region 3 - 768MB Chip Register Space, Peripherals
    ;*    Non-Cacheable, Non-Bufferable, 0x30000000 to 0x5fffffff

    WHILE counter < 1520
    L1Entry SECTION, (counter:SHL:20), 0, 0, ALL_ACCESS
counter SETA counter + 1
    WEND

    ;* Region 4 - 2560MB  NO ACCESS PERMITTED
    ;*     (Page Fault), 0x60000000 to 0xffffffff

    WHILE counter < 4096
    L1Entry INVALID, (counter:SHL:20), 0, 0, NO_ACCESS
counter SETA counter + 1
    WEND


    ENDIF

    ENDIF

    END

;****************************************************************************
;* Modifications:
;* $Log: 
;*  5    mpeg      1.4         7/22/03 6:17:26 PM     Tim White       SCR(s) 
;*        7018 :
;*        The loaders use only instruction caching without MMU/MPU support.  
;*        Remove the
;*        support for using the MMU/MPU from the loader code.
;*        
;*        
;*  4    mpeg      1.3         7/9/03 3:29:20 PM      Tim White       SCR(s) 
;*        6901 :
;*        Phase 3 codeldrext drop.
;*        
;*        
;*  3    mpeg      1.2         6/24/03 6:38:38 PM     Tim White       SCR(s) 
;*        6831 :
;*        Add flash, hsdp, demux, OSD, and demod support to codeldrext.
;*        
;*        
;*  2    mpeg      1.1         5/20/03 10:25:08 AM    Tim White       SCR(s) 
;*        6475 6476 :
;*        Force pagetable to be in RAM instead of ROM for application when 
;*        hardware
;*        page table mapping logic not used.
;*        
;*        
;*  1    mpeg      1.0         5/14/03 4:41:04 PM     Tim White       
;* $
;* 
;*    Rev 1.4   22 Jul 2003 17:17:26   whiteth
;* SCR(s) 7018 :
;* The loaders use only instruction caching without MMU/MPU support.  Remove the
;* support for using the MMU/MPU from the loader code.
;* 
;* 
;*    Rev 1.3   09 Jul 2003 14:29:20   whiteth
;* SCR(s) 6901 :
;* Phase 3 codeldrext drop.
;* 
;* 
;*    Rev 1.2   24 Jun 2003 17:38:38   whiteth
;* SCR(s) 6831 :
;* Add flash, hsdp, demux, OSD, and demod support to codeldrext.
;* 
;* 
;*    Rev 1.1   20 May 2003 09:25:08   whiteth
;* SCR(s) 6475 6476 :
;* Force pagetable to be in RAM instead of ROM for application when hardware
;* page table mapping logic not used.
;* 
;* 
;*    Rev 1.0   14 May 2003 15:41:04   whiteth
;* SCR(s) 6346 6347 :
;* Moved from codeldr to consolidate from hwlib/mmu.s so there is one version which
;* can be used from both application and bootloader code.
;* 
;*
;***** FILE COPIED FROM CODELDR ******
;* vlog -br K:\sabine\pvcs\CODELDR\pagetab_920t.s_v(pagetab_920t.s)
;*
;* Rev 1.5
;* Checked in:     26 Mar 2003 15:37:24
;* Last modified:  26 Mar 2003 15:35:26
;* Author id: jackmaw     lines deleted/added/moved: 0/4/0
;* SCR(s) 5854 :
;* Only allocate an actual page table if PAGE_TABLE_INITIALIZATION is set to
;* PHYSICAL_RAM.  Otherwise, a virtual page table will be used.
;*
;* Rev 1.4
;* Checked in:     13 Mar 2003 17:46:34
;* Last modified:  13 Mar 2003 17:30:46
;* Author id: bintzmf     lines deleted/added/moved: 108/118/0
;* SCR(s) 5188 :
;* Used SWCONFIG option MMU_CACHE_TYPE to determine how to generate page tables
;* 
;* Rev 1.3
;* Checked in:     28 Mar 2002 16:35:46
;* Last modified:  26 Mar 2002 09:02:52
;* Author id: mooreda     lines deleted/added/moved: 8/11/0
;* Branches:  1.3.1
;* SCR(s) 3403 :
;* Make ROMs come up non-cacheable
;* 
;* Rev 1.3.1.1
;* Checked in:     06 May 2003 07:51:16
;* Last modified:  06 May 2003 07:46:16
;* Author id: jackmaw     lines deleted/added/moved: 0/6/0
;* SCR(s) 6203 :
;* Added GET of stbcfg.a to resolve configuration definitions.
;* 
;* Rev 1.3.1.0
;* Checked in:     26 Mar 2003 15:40:32
;* Last modified:  26 Mar 2003 15:39:10
;* Author id: jackmaw     lines deleted/added/moved: 108/113/0
;* SCR(s) 5857 :
;* Only allocate an actual page table if PAGE_TABLE_INITIALIZATION is set to
;* PHYSICAL_RAM.  Otherwise, a virtual page table will be used.
;*
;* Rev 1.2
;* Checked in:     28 Feb 2002 22:32:22
;* Last modified:  28 Feb 2002 22:13:20
;* Author id: rossst     lines deleted/added/moved: 2/2/0
;* SCR(s) 3276 :
;* Changed the ANDing of the U_BIT to an OR in the page table creation macro.
;* 
;* Rev 1.1
;* Checked in:     28 Feb 2002 11:23:20
;* Last modified:  28 Feb 2002 11:22:22
;* Author id: mooreda     lines deleted/added/moved: 2/12/0
;* SCR(s) 3265 :
;* Per request, changed memory to be write-thru
;* 
;* Rev 1.0
;* Checked in:     28 Feb 2002 08:56:32
;* Last modified:  25 Feb 2002 19:59:16
;* Author id: mooreda     lines deleted/added/moved: 0/0/0
;* SCR(s) 3265 :
;* ARM 920T PageTable
;* 
;****************************************************************************

