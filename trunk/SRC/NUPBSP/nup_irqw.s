;/****************************************************************************/
;/*                   CONEXANT PROPRIETARY AND CONFIDENTIAL                  */
;/*                       SOFTWARE FILE/MODULE HEADER                        */
;/*                 Conexant Systems Inc. (c) 1998, 1999, 2000               */
;/*                               Austin, TX                                 */
;/*                            All Rights Reserved                           */
;/****************************************************************************/
;/*                                                                          */
;/* Filename:       nup_irqw.s                                               */
;/*                                                                          */
;/*                                                                          */
;/* Description:    calls C ISR routines                                     */
;/*                                                                          */
;/*                                                                          */
;/* Author:         Miles Bintz                                              */
;/*                                                                          */
;/****************************************************************************/
LOCKOUT         EQU     &C0                 ; Interrupt lockout value
LOCK_MSK        EQU     &C0                 ; Interrupt lockout mask value
MODE_MASK       EQU     &1F                 ; Processor Mode Mask
SUP_MODE        EQU     &13                 ; Supervisor Mode (SVC)
IRQ_MODE        EQU     &12                 ; Interrupt Mode (IRQ)
FIQ_MODE        EQU     &11                 ; Fast Interrupt Mode (FIQ)
I_BIT           EQU     &80                 ; Interrupt bit of CPSR and SPSR
F_BIT           EQU     &40                 ; Interrupt bit of CPSR and SPSR

    AREA |C$$code|, CODE, READONLY
|x$codeseg|

    IMPORT BSP_IRQ_Handler
	IMPORT BSP_Other_ISR_Handler
	IMPORT TCT_Control_Interrupts
    IMPORT TCT_Interrupt_Context_Save
    IMPORT TCT_Interrupt_Context_Restore
	IMPORT TMT_Timer_Interrupt

    EXPORT IRQ_Wrapper
    EXPORT Other_ISR

; IRQ_Wrapper
; this routine is called when the IRQ fires.  It does the necessary
; immediate context saving, and then calls the Nucleus PLUS context
; saving routine.  Then the BSP int handler is called.  On return the
; context is restored.
;
IRQ_Wrapper
;
; ***************** WARNING *************
; THIS BSP CODE MATCHES NUCLEUS PLUS VERSION 1.14.25.
; IF YOU LINK WITH A DIFFERENT VERSION OF THE PLUS LIBRARY, THE
; RESULTS MAY BE BAD
; ***************************************
;
; the following code needs to match closely the demo bsp code that was
; provided in the PLUS distribution.  The stack frame, etc needs to be
; exactly right when using the nucleus plus context save and restore
; functions.  In the comments below I refer to demo\int.s, this is the
; demo bsp provided in the distribution.  This file does not appear
; anywhere in the Conexant code.  If you want to see it you need a
; Nucleus plus distribution.

; the following code sequence matches demo\int.s:INT_IRQ

; the following code is from the PLUS distribution.  It checks to make
; sure that interrupts are disabled, and if not just returns.  In earler
; versions of PLUS, the comment indicated this was needed to fix a problem
; with ARM7.  Now the comment just says "ARM core interrupt check".
; So I am not sure if this sequence is needed but I am leaving it here.

    STMDB   sp!, {r1}                  ; check for previously enable interrupt
    MRS     r1, SPSR
    TST     r1, #I_BIT
    LDMIA   sp!, {r1}
    SUBNES  pc,lr,#4

; PLUS bsp makes a space on the stack to hold the interrupt enable reg
; for the demo board, like this:
;    SUB     sp, sp, #4                 ; reserve storage for int mask
; but we dont save the int mask this way and dont need to save the space.

; the nup context save function expects r0-r5 to be pushed onto the
; irq stack as follows:
    STMDB   sp!,{r0-r5}                ; Save r0-r5 on temporary IRQ stack

; fix up return address
    SUB     lr,lr,#4                   ; adjust IRQ return address

; in demo\int.s, control now passes to INT_IRQ_Shell

; IRQ return address into r4
    MOV     r4,lr                      ; context save expects r4=lr

; in demo\int.s, interrupts are explicitly disabled here, but we never
; re-enabled interrupts, so they are still disabled

; this causes the scheduler component to fully save the interrupted
; context on the thread's stack, including the r0-r5 we pushed earlier
    BL      TCT_Interrupt_Context_Save ; Call context save routine

; demo\int.s re-enables interrupts here, but we will leave disabled till
; later

; now call our own handler (which reads the pic and activates a hisr)
    BL      BSP_IRQ_Handler 

; restores the interrupted context
    B       TCT_Interrupt_Context_Restore ; This function does not return
    

Other_ISR
    ; Interrupts will be masked off and the processor will be
    ; in whatever mode caused the interrupt.   In nucleus, these
    ; exception handlers will run off stacks allocated in 
    ; startup\startup.s.  These stacks are setup for use with the
    ; C calling convention.
    
    mrs     r0, cpsr
    mrs     r1, spsr
    mov     r2, r13
    mov     r3, r14

    B       BSP_Other_ISR_Handler

    END

