;****************************************************************************/
;*                   CONEXANT PROPRIETARY AND CONFIDENTIAL                  */
;*                       SOFTWARE FILE/MODULE HEADER                        */
;*                    Conexant Systems Inc. (c) 2001-2003                   */
;*                              Austin, TX                                  */
;*                         All Rights Reserved                              */
;****************************************************************************/
;*
;* Filename:        xfill.s
;*
;*
;* Description:     assembler byte copy routines (FFillBytes)
;*
;*
;* Author:          Dave Moore
;*
;****************************************************************************/
;* $Header: xfill.s, 2, 7/30/03 3:38:54 PM, Larry Wang$
;****************************************************************************/

        CODE32  ; This all runs in ARM Mode and interworks with Thumb 

        INCLUDE GENERAL.A

NCR_BASE   EQU 0x10000000

        EXPORT FFillBytes
        EXPORT XMemTst

    IF :DEF: |ads$version|
        AREA CopyXCopy, CODE, READONLY
    ELSE
        AREA CopyXCopy, CODE, READONLY, INTERWORK
    ENDIF

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; FastFillBytes
;       Fills Count bytes starting at address in r0 with the byte in r1
; r0 pDst
; r1 Fill Byte
; r2 Count
;
; Returns
;       Nothing
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
FFillBytes
        mENTER  0
        ands    r3, r0, #3
        beq     LineAligned
        rsb     r3, r3, #4
        cmp     r3, r2
        movgt   r3, r2
        sub     r2, r2, r3
        movs    r3, r3, lsr #1
        strneb  r1, [r0], #1
        strneb  r1, [r0], #1
        strcsb  r1, [r0], #1

LineAligned
        movs    r3, r2, lsr #2          ; r3 = words
        beq     PartialWord
        mov     r1, r1, lsl #24
        orr     r1, r1, r1, lsr #8
        orr     r1, r1, r1, lsr #16

        movs    r4, r3, lsr #2          ; r4 = words / 4
        beq     MoreWords

        mov     r5, r1
        mov     r6, r1
        mov     r7, r1

More4Words
        stmia   r0!, {r1,r5,r6,r7}
        subs    r4, r4, #1
        bne     More4Words
        ands    r3, r3, #3
        beq     PartialWord

MoreWords
        str     r1, [r0], #4
        subs    r3, r3, #1
        bne     MoreWords


PartialWord
        and     r2, r2, #3
        movs    r2, r2, lsr #1
        strneb  r1, [r0], #1
        strneb  r1, [r0], #1
        strcsb  r1, [r0], #1

        mLEAVE

XMemTst
        mENTER   0
        mPUSH   "{r2-r10,r11}"
        mov     r2, r2, lsr #3
CopyWordLoopX
        ldmia   r1!, {r5-r10,r12,r14}
        stmia   r0!, {r5-r10,r12,r14}
        bic     r5, r0, #NCR_BASE       ; MC Bug 1
        ldr     r5, [r5]                ; MC Bug 1
        subs    r2, r2, #1
        bne     CopyWordLoopX
;       bic     r5, r0, #NCR_BASE       ; MC Bug 2
;       ldmfd   r5, {r5-r10,r12,r14}
        mPOP    "{r2-r10,r12}"
        cmp     r12, r11
        beq     ok
        mov     r0, r11                                                 ; should never get here but does
ok
        mLEAVE


        END

;****************************************************************************
;* Modifications:
;* $Log: 
;*  2    mpeg      1.1         7/30/03 3:38:54 PM     Larry Wang      SCR(s) 
;*        7076 :
;*        Change *.mac to *.a
;*        
;*  1    mpeg      1.0         5/14/03 4:08:44 PM     Tim White       
;* $
   
      Rev 1.1   30 Jul 2003 14:38:54   wangl2
   SCR(s) 7076 :
   Change *.mac to *.a
   
      Rev 1.0   14 May 2003 15:08:44   whiteth
   SCR(s) 6346 6347 :
   Fast-fill function(s).
   
;*
;****************************************************************************

