;***************************************************************************
;                   CONEXANT PROPRIETARY AND CONFIDENTIAL                  
;                       SOFTWARE FILE/MODULE HEADER                       
;                   Conexant Systems Inc. (c) 2002-2003
;                               Austin, TX                              
;                          All Rights Reserved       
;***************************************************************************
;*                                                                          
;* Filename:       mmu.s                                           
;*                                                                          
;* Description:    ARM V4 MMU Support                                      
;*                                                                          
;* Author:         Dave Moore                                               
;*                                                                          
;***************************************************************************
;* $Header: Hwlib_mmu.s, 1, 3/8/04 5:50:41 PM, Miles Bintz$
;****************************************************************************

;******************/
;* Include Files  */
;******************/
    GET stbcfg.a

    GET cpu.a
    GET board.a

    GET mmu.a
    GET startup.a


;-------------------------------------------------
;                Functions
;-------------------------------------------------


    KEEP ; keep local symbols
    IF :DEF: |ads$version|
    AREA    AV4MMU, CODE, READONLY
    ELSE
    AREA    AV4MMU, CODE, READONLY, INTERWORK
    ENDIF


;
;   FUNCTION: mmuSetPageTabBase
; 
;   PARAMETERS: r0 - 16K aligned page table beginning address
; 
;   DESCRIPTION: set the page table base register via CP15 register 2 
; 
;   RETURNS: r0 : 1 - OK
;                 0 - FAILED (bad input address alignment)
;
;   REGS USED: r0, r1, r2
; 

    IF (CPU_TYPE=CPU_ARM920T):LOR:(CPU_TYPE=AUTOSENSE)

    EXPORT mmuSetPageTabBase
mmuSetPageTabBase

    IF MMU_CACHE_DISABLE = 1

    mov   r0, #0x1    ; fake an OK exit

    ELSE ;  MMU_CACHE_DISABLE = 0

    ; Check for proper alignment of address
    ldr     r2, =0x3fff
    ands    r1, r0, r2

    ; Load page table address and set success return
    mcreq   p15, 0, r0, c2, c0, 0
    moveq   r0, #0x1      ; OK exit
    movne   r0, #0x0      ; FAILED exit

    ENDIF    ; IF MMU_CACHE_DISABLE = 0
 
    bx      lr

    ENDIF ; (CPU_TYPE=CPU_ARM920T):LOR:(CPU_TYPE=AUTOSENSE)


;
;   FUNCTION: mmuGetPageTabBase
; 
;   PARAMETERS: None
; 
;   DESCRIPTION: get the page table base register via CP15 register 2 
; 
;   RETURNS: Contents of CP15 Reg 2 in r0
;
;   REGS USED: r0
;

    IF (CPU_TYPE=CPU_ARM920T):LOR:(CPU_TYPE=AUTOSENSE)

    EXPORT mmuGetPageTabBase
mmuGetPageTabBase

    IF MMU_CACHE_DISABLE = 1

    mov r0, #0x0  ; bogus return

    ELSE  ; MMU_CACHE_DISABLE = 0

    mrc   p15, 0, r0, c2, c0, 0

    ENDIF ; MMU_CACHE_DISABLE 

    bx    lr

    ENDIF ; (CPU_TYPE=CPU_ARM920T):LOR:(CPU_TYPE=AUTOSENSE)


; 
;  FUNCTION: mmuSetDomainAccessControl
; 
;  PURPOSE: Sets the domain access-control register via CP15 register 3 
; 
;  PARAMETERS: r0 - value to be placed in CP15 reg 3 (see description below) 
;
;  DESCRIPTION:
;
;   MMU accesses are primarily controller thru the use of domains. There are
;   16 domains and each has a 2 bit field to access it. Two types of users are
;   supported, client and manager. The domains are defined in the domain access
;   control register.
;
;   INTERPRETING DOMAIN ACCESS CONTROL REGISTER
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
;  RETURNS: nothing
; 
;  REGS USED: r0
;
      EXPORT mmuSetDomainAccessControl
mmuSetDomainAccessControl

    mcr   p15, 0, r0, c3, c0, 0

    bx    lr




; 
;  FUNCTION: mmuGetDomainAccessControl()
; 
;  PURPOSE: Gets the domain access-control register contents via CP15 register 3 
; 
;  PARAMETERS: None 
; 
;   MMU accesses are primarily controller thru the use of domains. There are
;   16 domains and each has a 2 bit field to access it. Two types of users are
;   supported, client and manager. The domains are defined in the domain access
;   control register.
;
;   INTERPRETING DOMAIN ACCESS CONTROL REGISTER
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
;  RETURNS: r0 = contents of CP15 register 3
; 
;  REGS USED: r0
;
      EXPORT mmuGetDomainAccessControl
mmuGetDomainAccessControl

      mrc     p15, 0, r0, c3, c0, 0 

      bx      lr


; 
;   FUNCTION: mmuDrainWriteBuffer
; 
;   PURPOSE: drains write buffer (execution stops until complete)
;
;   PARAMETERS: None
;
;   RETURNS: Nothing
;
;   REGS USED: r0
;
    EXPORT mmuDrainWriteBuffer
mmuDrainWriteBuffer

    IF MMU_CACHE_DISABLE = 0

    mov   r0, #0
    mcr   p15, 0, r0, c7, c10, 4

    ENDIF    ; IF MMU_CACHE_DISABLE = 0

    bx    lr


;
;   FUNCTION: mmuInvalidateTLB
;
;   PURPOSE: Invalidate Data & Instruction TLB's
;
;   PARAMETERS: None.
;
;   RETURNS: Nothing
;
;   REGS USED: r0
;

    IF (CPU_TYPE=CPU_ARM920T):LOR:(CPU_TYPE=AUTOSENSE)

     EXPORT mmuInvalidateTLB
mmuInvalidateTLB

     IF MMU_CACHE_DISABLE = 0

     mov     r0, #0
     mcr     p15, 0, r0, c8, c7, 0

     ENDIF    ; IF MMU_CACHE_DISABLE = 0

     bx    lr

    ENDIF ; (CPU_TYPE=CPU_ARM920T):LOR:(CPU_TYPE=AUTOSENSE)


; 
;   FUNCTION: mmuInvalidateITLB
;
;   PURPOSE: Invalidate Instruction TLB
;
;   PARAMETERS: None. 
;
;   RETURNS: Nothing
;
;   REGS USED: r0
;

    IF (CPU_TYPE=CPU_ARM920T):LOR:(CPU_TYPE=AUTOSENSE)

     EXPORT mmuInvalidateITLB
mmuInvalidateITLB

     IF MMU_CACHE_DISABLE = 0

     mov     r0, #0
     mcr     p15, 0, r0, c8, c5, 0

     ENDIF    ; IF MMU_CACHE_DISABLE = 0

     bx    lr

    ENDIF ; (CPU_TYPE=CPU_ARM920T):LOR:(CPU_TYPE=AUTOSENSE)


; 
;   FUNCTION: mmuInvalidateDTLB
;
;   PURPOSE: Invalidate Data TLB
;
;   PARAMETERS: None
;
;   RETURNS: Nothing
;
;   REGS USED: r0
;

    IF (CPU_TYPE=CPU_ARM920T):LOR:(CPU_TYPE=AUTOSENSE)

     EXPORT mmuInvalidateDTLB
mmuInvalidateDTLB

     IF MMU_CACHE_DISABLE = 0

     mov     r0, #0
     mcr   p15, 0, r0, c8, c6, 0 

     ENDIF    ; IF MMU_CACHE_DISABLE = 0

     bx    lr

    ENDIF ; (CPU_TYPE=CPU_ARM920T):LOR:(CPU_TYPE=AUTOSENSE)


; 
;   FUNCTION: mmuInvalidateITLBEntry
;
;   PURPOSE: Invalidate single Instruction TLB entry
;
;   PARAMETERS:  r0 = MVA to be invalidated
;
;   RETURNS: Nothing
;
;   REGS USED: r0
;

    IF (CPU_TYPE=CPU_ARM920T):LOR:(CPU_TYPE=AUTOSENSE)

    EXPORT mmuInvalidateITLBEntry
mmuInvalidateITLBEntry

    IF MMU_CACHE_DISABLE = 0

    mcr     p15, 0, r0, c8, c5, 1 

    ENDIF    ; IF MMU_CACHE_DISABLE = 0

    bx      lr

    ENDIF ; (CPU_TYPE=CPU_ARM920T):LOR:(CPU_TYPE=AUTOSENSE)


; 
;   FUNCTION: mmuInvalidateDTLBEntry
;
;   PURPOSE: Invalidate single Data TLB entry
;
;   PARAMETERS:  r0 = MVA to be invalidated
;
;   RETURNS: Nothing
;
;   REGS USED: r0
;

    IF (CPU_TYPE=CPU_ARM920T):LOR:(CPU_TYPE=AUTOSENSE)

    EXPORT mmuInvalidateDTLBEntry
mmuInvalidateDTLBEntry

    IF MMU_CACHE_DISABLE = 0

    mcreq   p15, 0, r0, c8, c6, 1 

    ENDIF    ; IF MMU_CACHE_DISABLE = 0

    bx      lr

    ENDIF ; (CPU_TYPE=CPU_ARM920T):LOR:(CPU_TYPE=AUTOSENSE)


; 
;   FUNCTION: mmuGetFCSE
; 
;   PURPOSE: return the FastContextSwitchExtension (register 13)
;
;   PARAMETERS: None
;
;   RETURNS: r0 = contents of CP15 Reg 13
;
;   REGS USED: r0
;

    IF (CPU_TYPE=CPU_ARM920T):LOR:(CPU_TYPE=AUTOSENSE)

    EXPORT mmuGetFCSE
mmuGetFCSE

    mrc   p15, 0, r0, c13, c0, 0 

    bx    lr

    ENDIF ; (CPU_TYPE=CPU_ARM920T):LOR:(CPU_TYPE=AUTOSENSE)


;
;   FUNCTION: mmuSetFCSE
; 
;   PURPOSE: Set the FastContextSwitchExtension (register 13)
;
;   PARAMETERS: r0 = FCSE PID in bits 31:25 (bits 24:0 SBZ)
;
;   RETURNS: r0 = 0 - SUCCESS
;                 1 - FAIL (bad PID) 
;
;   REGS USED: r0, r1, r2
;

    IF (CPU_TYPE=CPU_ARM920T):LOR:(CPU_TYPE=AUTOSENSE)

    EXPORT mmuSetFCSE
mmuSetFCSE

    IF MMU_CACHE_DISABLE = 0

    ; check for valid input
    ldr     r2,=0x1ffffff
    ands    r1, r0, r2

    mcreq   p15, 0, r0, c13, c0, 0 
    moveq   r0, #0x0  ; load SUCCESS for return
    movne   r0, #0x1  ; load FAIL for return

    ENDIF ; MMU_CACHE_DISABLE

    bx     lr

    ENDIF ; (CPU_TYPE=CPU_ARM920T):LOR:(CPU_TYPE=AUTOSENSE)
  
 
;
;   FUNCTION: mmuGetState
;
;   PARAMETERS: None
;
;   Returns: r0 = contents of CP15 Control Register (register 1)
;
;   Bit31   iA bit   Async Clock Select
;   Bit30   nF bit   notFastBus Select
;   29:15            Reserved (read=unpredictable,write=SBZ)
;   Bit14   RR bit   Round-Robin Replacement
;   Bit13    V bit   Base location of exception regs (0=0x0,1=0xffff0000)
;   Bit12    I bit   I Cache Enable
;   11:10            Reserved (read=00,write=00)
;   Bit9     R bit   ROM Protection
;   Bit8     S bit   System Protection
;   Bit7     B bit   Endianess (0=little,1=big)
;   6:3              Reserved (read=1111,write=1111)
;   Bit2     C bit   DCache Enable
;   Bit1     A bit   Data Alignment fault enable (0=disable,1=enable)
;   Bit0     M bit   MMU enable (0=disable,1=enable)
; 
;   REGS USED: r0
;
    EXPORT mmuGetState
mmuGetState
  
    mrc   p15, 0, r0, c1, c0, 0 

    bx    lr


;
;   FUNCTION: mmuSetState
;
;   PARAMTERS: r0 - mask of the following:
;
;   Bit31   iA bit   Async Clock Select
;   Bit30   nF bit   notFastBus Select
;   29:15            Reserved (read=unpredictable,write=SBZ)
;   Bit14   RR bit   Round-Robin Replacement
;   Bit13    V bit   Base location of exception regs (0=0x0,1=0xffff0000)
;   Bit12    I bit   I Cache Enable
;   11:10            Reserved (read=00,write=00)
;   Bit9     R bit   ROM Protection
;   Bit8     S bit   System Protection
;   Bit7     B bit   Endianess (0=little,1=big)
;   6:3              Reserved (read=1111,write=1111)
;   Bit2     C bit   DCache Enable
;   Bit1     A bit   Data Alignment fault enable (0=disable,1=enable)
;   Bit0     M bit   MMU enable (0=disable,1=enable)
;
;   RETURNS: r0 = 0 - FAILED, 1 - SUCCESS
;
;

    EXPORT mmuSetState
mmuSetState

    IF MMU_CACHE_DISABLE = 1

       mov    r0, #0x1  ; bogus SUCCESS return

    ELSE  ; MMU_CACHE_DISABLE = 0

    ;Load r2 with the bit pattern appropriate to the CPU type
    IF (CPU_TYPE=AUTOSENSE)
       mrc    p15, 0, r2, c0, c0, 0  ; get the ID register into r2
       tst    r2, #CPU_ARM920_BIT    ; test for a 920 CPU (ne will mean 920)
       ldreq  r2, =0x3fff8f00        ; isolate bits 29:15,11:8 (ARM940)
       ldrne  r2, =0x3fff8c00        ; isolate bits 29:15,11:10 (ARM920)
    ENDIF
    IF (CPU_TYPE=CPU_ARM920T)
       ldr    r2, =0x3fff8c00        ; isolate bits 29:15,11:10 (ARM920)
    ENDIF
    IF (CPU_TYPE=CPU_ARM940T)
       ldr    r2, =0x3fff8f00        ; isolate bits 29:15,11:8 (ARM940)
    ENDIF

       ; do some checks for valid input..

       and    r1, r0, #0x78          ; isolate bits 6:3
       cmp    r1, #0x78              ; they should all be set to 1
       andeqs r1, r0, r2             ; they should all be zeroes
                                     ; error exit, bits 29:15,11:10 not all 0s
       mcreq  p15, 0, r0, c1, c0, 0  ; OK, change CP15 Reg 1
       moveq  r0, #0x1               ; normal exit
       movne  r0, #0x0               ; error exit

    ENDIF ; MMU_CACHE_DISABLE 

       bx     lr


    EXPORT _GetCPUType
    EXPORT GetCPUType
_GetCPUType
GetCPUType
       mrc    p15, 0, r0, c0, c0, 0   ;get the ID register into r0
       tst    r0, #CPU_ARM940_BIT     ;test for a 940 CPU
       movne  r0, #CPU_ARM940T
       moveq  r0, #CPU_ARM920T
       bx     lr

    END

;****************************************************************************
;* $Log: 
;*  1    mpeg      1.0         3/8/04 5:50:41 PM      Miles Bintz     CR(s) 
;*        8509 : modified mmu.S to HWLIB_MMU.S to remove object name conflict 
;*        in NDSCore build
;* $
;* 
;*    Rev 1.11   24 Jun 2003 17:34:40   whiteth
;* SCR(s) 6831 :
;* Add flash, hsdp, demux, OSD, and demod support to codeldrext
;* 
;* 
;*    Rev 1.10   19 May 2003 14:54:28   jackmaw
;* SCR(s) 6418 6419 :
;* Re-ordered/changed code to handle the CPU_TYPE=AUTOSENSE case.
;* 
;*    Rev 1.9   14 May 2003 14:51:00   whiteth
;* SCR(s) 6346 6347 :
;* Removed the duplicate 920 page table creation code.  Added a mmuInvalidateTLB function
;* which invalidates both the instruction & data TLB's at once.  Remove the unnecessary
;* stack overhead in some of the functions and cleaned up the assembly a little.
;* 
;* 
;*    Rev 1.8   30 Apr 2003 15:59:38   jackmaw
;* SCR(s) 6113 :
;* Added PVCS Header and Log comments.
;* Modified conditional assembly directives and code to add the capability to
;* detect the CPU type (920 vs. 940) at runtime when CPU_TYPE=AUTOSENSE.
;* 
;*    Rev 1.7   25 Apr 2003 15:04:22   jackmaw
;* SCR(s) 5855 :
;* Only place a pagetable in RAM if specified by PAGE_TABLE_INITIALIZATION=
;* PHYSICAL_RAM or if RTOS=VXWORKS.
;* 
;*    Rev 1.6   14 Mar 2003 12:21:46   bintzmf
;* SCR(s) 5774 :
;* Switch on SWCONFIG option MMU_CACHE_TYPE to make cache WT/WB
;* 
;*    Rev 1.5   31 Jan 2003 17:37:38   mooreda
;* SCR(s) 5375 :
;* added additional debugging checks.
;* 
;*        Rev 1.5.1.0   25 Apr 2003 15:09:24   jackmaw
;*     SCR(s) 5856 :
;*     Only place a pagetable in RAM if specified by PAGE_TABLE_INITIALIZATION=
;*     PHYSICAL_RAM or if RTOS=VXWORKS.
;* 
;*    Rev 1.4   17 Jan 2003 17:34:38   rossst
;* SCR(s) 5269 :
;* Replaced mcreq with mcr in mmuInvalidateDTLB routine.
;* 
;*    Rev 1.3   17 Dec 2002 16:02:40   whiteth
;* SCR(s) 5182 :
;* Remove ARM_PIT_TYPE, no longer used.
;* 
;*    Rev 1.2   11 Jul 2002 16:12:46   len1
;* Switched from write-back to write-thru pagetable cache mode since PSOS and Nuclu
;* es do not support it.  This change will not affect VxWorks since it uses a diffe
;* rent file to set up cache.
;* 
;*    Rev 1.1   23 May 2002 09:40:36   bradforw
;* SCR(s) 3826 :
;* Remove Warning Messages
;* 
;*    Rev 1.0   01 Apr 2002 08:25:22   mooreda
;* SCR(s) 3457 :
;* ARM V4 MMU Functions
;* 
;****************************************************************************


