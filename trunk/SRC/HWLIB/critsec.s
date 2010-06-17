;****************************************************************************
;*                 Rockwell Semiconductor Systems - SABINE                  *
;****************************************************************************
;*                                                                          *
;* Filename:           CRITSEC.S                                            *
;*                                                                          *
;* Description:        Assembler functions to enable and disable interrupts *
;*                     and perform other low level functions.               *
;*                                                                          *
;* Author:             Dave Wilson                                          *
;*                                                                          *
;* Copyright Rockwell Semiconductor Systems, 1998                           *
;* All Rights Reserved.                                                     *
;*                                                                          *
;****************************************************************************
; $Header: critsec.s, 11, 6/3/03 4:46:52 PM, Dave Wilson$
;
        KEEP

    IF :DEF: |ads$version|
        AREA CritSec,CODE,READONLY
    ELSE
        AREA CritSec,CODE,READONLY,INTERWORK
    ENDIF
       
	    IMPORT c_critical_section_begin
		  IMPORT c_not_interrupt_safe
      IMPORT c_not_from_critical_section

DISABLE_INTS    EQU     0x00000080
ENABLE_INTS     EQU     0xFFFFFF7F

        EXPORT  GetStackPtr
GetStackPtr
        MOV     r0, r13
        BX      lr

        EXPORT  EnableInterrupts
EnableInterrupts

        MRS     r0, cpsr
        AND     r0, r0, #ENABLE_INTS
        MSR     cpsr_c, r0

        BX      lr

        EXPORT  DisableInterrupts
DisableInterrupts

        MRS     r1, cpsr
        ANDS    r0, r1, #DISABLE_INTS
        MOVNE   r0, #0
        MOVEQ   r0, #1
        ORR     r1, r1, #DISABLE_INTS
        MSR     cpsr_c, r1

        BX      lr

        EXPORT  InterruptsDisabled
InterruptsDisabled

        MRS     r1, cpsr
        ANDS    r0, r1, #DISABLE_INTS
        MOVNE   r0, #1
        MOVEQ   r0, #0

        BX      lr

; Assembler veneer allowing us to pass the caller's return address to the C critical
; section function.
        EXPORT critical_section_begin
critical_section_begin

        stmfd   sp!,{r1-r3,lr}
        mov     r0, lr 
        bl      c_critical_section_begin
        ldmia   sp!,{r1-r3,lr}
        bx      lr

; Assembler veneer allowing us to pass the caller's return address to the C 
; not_interrupt_safe function.
        EXPORT not_interrupt_safe
not_interrupt_safe

        stmfd   sp!,{r1-r3,lr}
        mov     r0, r14 
        bl      c_not_interrupt_safe
        ldmia   sp!,{r1-r3,lr}
        bx      lr


; Assembler veneer allowing us to pass the caller's return address to the C 
; not_from_critical_section function.
        EXPORT asm_not_from_critical_section
asm_not_from_critical_section

        stmfd   sp!,{r1-r3,lr}
        mov     r0, r14 
        bl      c_not_from_critical_section
        ldmia   sp!,{r1-r3,lr}
        bx      lr
        
        END

;****************************************************************************
; Modifications:
; $Log: 
;  11   mpeg      1.10        6/3/03 4:46:52 PM      Dave Wilson     SCR(s) 
;        6652 6651 :
;        Added asm_not_from_critical_section. This functions calls the function
;        c_not_from_critical_section in HWLIB.C passing the return address of 
;        the caller
;        as parameter. It is used to generate a fatal exit any time the 
;        function is 
;        called from within a critical section (though we all know that making 
;        ANY 
;        function call from within a critical section is a very bad idea).
;        
;  10   mpeg      1.9         10/29/02 5:24:22 PM    Dave Moore      SCR(s) 
;        4833 :
;        added functions critcal_section_begin and not_interrupt_safe
;        which call "c" versions (c_critical_section_begin,..). These
;        functions pass their "c" counterparts LR in r0.
;        
;        
;  9    mpeg      1.8         5/20/02 5:46:24 PM     Bobby Bradford  SCR(s) 
;        3834 :
;        Made AREA directive conditional to remove INTERWORK warning
;        
;  8    mpeg      1.7         5/23/01 7:17:42 PM     Joe Kroesche    SCR(s) 
;        1935 :
;        changed enable/disable interrupts to leave FIQ interrupt enable,
;        previously it was being disabled along with IRQ.  This interfered with
;        using FIQ for profiling
;        
;  7    mpeg      1.6         4/20/01 10:15:54 AM    Tim White       DCS#1687, 
;        DCS#1747, DCS#1748 - Add pSOS task switch prevention, move OS specific
;        functionality out of hwlib.c and into psoskal.c/nupkal.c, and update 
;        critical
;        section debugging function for both release and debug without using 
;        assembly code.
;        
;  6    mpeg      1.5         4/11/01 3:34:04 PM     Tim White       DCS#1683, 
;        DCS#1684, DCS#1685 -- Add critical section debugging support.
;        
;  5    mpeg      1.4         3/23/01 11:40:18 AM    Steve Glennon   DCS # 
;        1475/1476 
;        Added InterruptsDisabled function to determine current state of 
;        interrupt
;        mask
;        
;  4    mpeg      1.3         8/29/00 8:44:04 PM     Miles Bintz     removed 
;        macros.a reference.  The include file was PSOS specific and unused
;        
;  3    mpeg      1.2         1/18/00 10:12:00 AM    Dave Wilson     Changed 
;        MSR instruction target format to comply with ARM SDT 2.5 
;        requirements
;        
;  2    mpeg      1.1         1/6/00 10:27:00 AM     Dave Wilson     Changes 
;        for ARM/Thumb interworking
;        
;  1    mpeg      1.0         9/14/99 3:11:52 PM     Dave Wilson     
; $
;  
;     Rev 1.10   03 Jun 2003 15:46:52   dawilson
;  SCR(s) 6652 6651 :
;  Added asm_not_from_critical_section. This functions calls the function
;  c_not_from_critical_section in HWLIB.C passing the return address of the caller
;  as parameter. It is used to generate a fatal exit any time the function is 
;  called from within a critical section (though we all know that making ANY 
;  function call from within a critical section is a very bad idea).
;

