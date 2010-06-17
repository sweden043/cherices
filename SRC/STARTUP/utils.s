;/****************************************************************************/
;/*                   CONEXANT PROPRIETARY AND CONFIDENTIAL                  */
;/*                       SOFTWARE FILE/MODULE HEADER                        */
;/*                    Conexant Systems Inc. (c) 1999-2003                   */
;/*                               Austin, TX                                 */
;/*                            All Rights Reserved                           */
;/****************************************************************************/
;/*
; * Filename:       utils.s
; *
; *
; * Description:    Startup assmebly utility functions
; *
; *
; * Author:         Tim White
; *
; ****************************************************************************/
;/* $Header: utils.s, 3, 10/21/03 10:03:25 AM, Larry Wang$
; ****************************************************************************/

;******************
;* Include Files  *
;******************
    GET stbcfg.a

    GET cpu.a
    GET startup.a

;**********************
;* Local Definitions  *
;**********************

;*************************
;* External Definitions  *
;*************************

;***************
;* Global Data *
;***************


;********************************************************************
;*  CallFuncByAddrWithIntrsDisabled                                 *
;*                                                                  *
;*  DESCRIPTION:                                                    *
;*      Performs a long jump and return to a function copied to     *
;*      RAM.  Used by flash/flashrw.c                               *
;*                                                                  *
;*  NOTE:                                                           *
;*      The function pointer passed MUST be an to an ARM mode       *
;*      32-bit function.                                            *
;*                                                                  *
;*  PARAMETERS:                                                     *
;*  r0 - contains address of the function to call in RAM        *
;*      r1 - contains the first parameter (d0)                      *
;*      r2 - contains the second parameter (d1)                     *
;*      r3 - contains the third parameter (d2)                      *
;*                                                                  *
;*  RETURNS:                                                        *
;*      r0 - return parameter from called function                  *
;*                                                                  *
;*  REGISTER USE:                                                   *
;********************************************************************

    KEEP
    IF :DEF: |ads$version|
    AREA    StartupCallFuncByAddr, CODE, READONLY
    ELSE
    AREA    StartupCallFuncByAddr, CODE, READONLY, INTERWORK
    ENDIF

    EXPORT CallFuncByAddrWithIntrsDisabled
CallFuncByAddrWithIntrsDisabled

    ;Save return address
    stmfd   sp!, {r4,lr}
    mov     r4, r0

    ;Disable ALL interrupts
    mrs     r0, cpsr
    stmfd   sp!, {r0}
    orr     r0, r0, #CPU_IRQ_DIS
    orr     r0, r0, #CPU_FIQ_DIS
    msr     cpsr_c, r0

    ;Setup lr for the called function and jump
    mov     r0, r1
    mov     r1, r2
    mov     r2, r3
    orr     r4, r4, #1           ; Force Thumb Execution
    mov     lr, pc
    bx      r4

    ;Reenable interrupts
    ldmia   sp!, {r4}
    msr     cpsr_c, r4

    ;Return to caller
    ldmfd   sp!, {r4,lr}
    bx      lr

    END

;****************************************************************************
;* Modifications:
;* $Log: 
;*  3    mpeg      1.2         10/21/03 10:03:25 AM   Larry Wang      CR(s): 
;*        7673 Remove CallRealSetupROMs.
;*  2    mpeg      1.1         7/9/03 3:29:34 PM      Tim White       SCR(s) 
;*        6901 :
;*        Phase 3 codeldrext drop.
;*        
;*        
;*  1    mpeg      1.0         5/14/03 4:40:36 PM     Tim White       
;* $
;****************************************************************************

