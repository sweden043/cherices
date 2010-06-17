;****************************************************************************/
;*                   CONEXANT PROPRIETARY AND CONFIDENTIAL                  */
;*                       SOFTWARE FILE/MODULE HEADER                        */
;*                    Conexant Systems Inc. (c) 2001-2003                   */
;*                              Austin, TX                                  */
;*                         All Rights Reserved                              */
;****************************************************************************/
;*
;* Filename:        xcopy.s
;*
;*
;* Description:     assembler byte copy routines (FCopy)
;*
;*
;* Author:          Dave Moore
;*
;****************************************************************************/
;* $Header: xcopy.s, 6, 7/10/03 10:45:38 AM, Larry Wang$
;****************************************************************************/

        CODE32  ; This all runs in ARM Mode and interworks with Thumb 

NCR_BASE   EQU 0x10000000

        EXPORT FCopy

    IF :DEF: |ads$version|
        AREA CopyXCopy, CODE, READONLY
    ELSE
        AREA CopyXCopy, CODE, READONLY, INTERWORK
    ENDIF

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;       WAToWACopy  ( Word Aligned To Word Aligned Copy )
; Entry
;       r0 Dst word address
;       r1 Src word address
;       r2 #bytes
;       <r4-r7 saved by caller (FCopy)>
;
; NOTE!!!!  If you assemble with DEBUG defined and your Src or Dst are NOT
;           word aligned this routine will place you in a spin loop...
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

WAToWACopy  

        cmp     r2, #0                 ; anything to do?
        bxle    lr


        stmfd   sp!, {r8-r12,lr}       ; <r4-r7 already stacked by FCopy>

        IF :DEF:DEBUG                   ; check for aligned src / dst
888     bics    r4, r1, #0xfffffffc    ; isolate bits 0,1 of src
        bgt     %BT888                 ; lock-up if given unaligned src
        bics    r3, r0, #0xfffffffc    ; isolate bits 0,1 of dst
        bgt     %BT888                 ; lock-up if given unaligned dst
        ENDIF 

     
        ; compute #byte stores that will be required 
        ; after moving word chunks (may be 0)
        cmp     r2, #4                 ; count > 4?
        bicgt   r4, r2, #0xfffffffc    ; remainder
        movlt   r4, r2                 ; count <= 4
        moveq   r4, #0                 ; count == 4


wa_wa_copy_loop

        ;
        ; Load data from src
        ;

        cmp      r2, #12
        ldmgeia  r1!, { r5,r6,r7,r8 }
        bge      %FT0

        cmp      r2, #8
        ldmgeia  r1!, { r5,r6,r7 }
        bge      %FT1

        cmp      r2, #4
        ldmgeia  r1!, { r5,r6 }
        bge      %FT2

        ldr      r5, [r1], #4
        b        %FT3

        ;
        ; store data to dst
        ;

        ; try for 16
0       cmp      r2, #16
        blt      %FT1
        sub      r2, r2, #16             ; update byte count 
        stmia    r0!, { r5,r6,r7,r8 }    ; put away 16 bytes to dst
        beq      %FT4
        bgt      wa_wa_copy_loop         ; go back for more
        
        ; try for 12
1       cmp      r2, #12
        blt      %FT2
        sub      r2, r2, #12             ; update byte count
        stmia    r0!, { r5,r6,r7 }       ; put away 12 bytes
        mov      r5, r8                  ; any remaining to r5
        beq      %FT4
     
        ; try for 8
2       cmp      r2, #8
        blt      %FT3
        sub      r2, r2, #8              ; update byte count
        stmia    r0!, { r5,r6 }          ; put away 8 bytes
        mov      r5, r7                  ; any remaining to r5
        beq      %FT4
     

        ; try for 4
3       cmp      r2, #4
        blt      %FT4
        str      r5, [r0], #4            ; put away 4 bytes
        mov      r5, r6                  ; any remaining to r5
        sub      r2, r2, #4              ; update byte count
     

        ; 3 or less
4       cmp     r4, #0                   ; remainder 0?
        beq     %FT6                     ; if so, exit


        ; store final as byte stores
5       strb    r5, [r0], #1             ; store next remaining byte
        subs    r4, r4, #1               ; remainder = remainder - 1
        movne   r5, r5, LSR #8           ; move next byte into position
        bgt     %BT5                     ; do another byte store


6       ldmfd    sp!, {r8-r12,lr}        ; restore regs
        bx       lr



;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;       FCopy  ( Byte Aligned To Byte Aligned Copy )
; Entry
;       r0 Dst byte address
;       r1 Src byte address
;       r2 #bytes
;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

FCopy  

        cmp     r2, #0                 ; anything to do?
        bxle    lr


        stmfd   sp!, {r4-r7,lr}        ; get work regs

999     bic     r3, r1, #0xfffffffc    ; isolate bits 0,1 of src
        bic     r4, r0, #0xfffffffc    ; isolate bits 0,1 of dst

        ; Is src/dst (or both) already aligned?

        adds    r5,r3,r4               ; check for BOTH aligned
        bne     %FT0                
        bl      WAToWACopy             ; call aligned copy routine
        b       %FT1                   ; exit


0       cmp     r4, #0                 ; dst already aligned?
        bne     %FT2    
        bl      BAToWACopy             ; call unaligned copy routine
        b       %FT1                   ; exit


        ; Need to bring dst into alignment


        ; calculate #byte stores needed to bring dst
        ; (possibly both src/dst) to a word alignment

2       rsb     r6,r4,#4

        ; r6 now has number of byte stores needed to achieve alignmment
     

3       ldrb    r5, [r1], #1           ; load 1 byte from src
        subs    r2, r2, #1             ; decrement total byte count
        strb    r5, [r0], #1           ; store 1 byte to dst
        beq     %FT1                   ; are we done?
        subs    r6, r6, #1             ; decrement byte count
        bgt     %BT3                   ; more bytes to do..

        b       %BT999                 ; we now have at least dst (and possibly src) 
                                       ; aligned. reuse logic and decide which
                                       ; work routine to call

1       ldmfd   sp!,{r4-r7,lr}         ; restore regs
        bx      lr



;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;       BAToWACopy  ( Byte Aligned To Word Aligned Copy )
; Entry
;       r0 Dst word address
;       r1 Src byte address
;       r2 #bytes
;       <r4-r7 saved by caller (FCopy)>
;
; NOTE!!!!  If you assemble with DEBUG defined and your Dst is not 
;           word aligned this routine will place you in a spin loop...
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

BAToWACopy  

       
        
        IF :DEF:DEBUG                   ; check for aligned dst
888     bics    r3, r0, #0xfffffffc    ; isolate bits 0,1 of dst
        bgt     %BT888                 ; lock-up if unaligned 
        ENDIF 


        stmfd   sp!, {r8-r12,lr}       ; <r4-r7 already stacked by FCopy>

        bics    r3, r1, #0xfffffffc    ; isolate bits 0,1 of src


        ; Compute necessary bit shift to build a destination word
        ; from 2 source words (when source is not word aligned)

        rsb     r12, r3, #4            ; r12 = #bytes to left shift l.h. src
        mov     r3, r12, LSL #3        ; r3 = #bits to left shift l.h. src
        rsb     r4, r3, #32            ; r4 = #bits to right shift r.h. src 

        ; setup for copy loop
        bic     r1, r1, #3             ; word align src
        ldr     r14, [r1], #4          ; load 1st src word with unaligned bytes
        mov     r14, r14, LSR r4       ; shift off unwanted bytes


ba_wa_copy_loop

        ;
        ; Load data from src
        ;  

        cmp      r2, #12
        ldmgeia  r1!, { r5,r6,r7,r8 }
        bge      %FT0

        cmp      r2, #8
        ldmgeia  r1!, { r5,r6,r7 }
        bge      %FT0

        cmp      r2, #4
        ldmgeia  r1!, { r5,r6 }
        bge      %FT0

        ldr      r5, [r1], #4

        ;
        ; Shift till' ya drop
        ;

        ; build r9 (1st dest word)
0       mov     r9, r5, LSL r3         ; r9 = r5 << r3 
        orr     r9, r9, r14            ; r9 = r9 | r14  
        mov     r14, r5, LSR r4        ; shifted out bytes in r14           

        cmp     r2, #4                 ; at least 4 bytes left to process?
        movle   r14, r9                ; remaining bytes go in shifter              
        blt     %FT6                   ; < 4 bytes left
        beq     %FT4                   ; store 1 word


        ; build r10 (2nd dest word)
        mov     r10, r6, LSL r3        ; r10 = r6 << r3
        mov     r14, r5, LSR r4        ; r14 = r5 >> r4
        orr     r10, r10, r14          ; r10 = r10 | r14 
        mov     r14, r6, LSR r4        ; shifted out bytes in r14

        cmp     r2, #8                 ; at least 8 bytes left to process?
        blt     %FT4                   ; < 8 bytes left
        beq     %FT1                   ; store 2 words


        ; build r11 (3rd dest word)
        mov     r11, r7, LSL r3        ; r11 = r7 << r3 
        mov     r14, r6, LSR r4        ; r14 = r6 >> r4
        orr     r11, r11, r14          ; r11 = r11 | r14
        mov     r14, r7, LSR r4        ; shifted out bytes in r14

        cmp     r2, #12                ; at least 12 bytes left to process?
        blt     %FT1                   ; store 2 words
        beq     %FT3                   ; store 3 words


        ; build r12 (4th dest word)
        mov     r12, r8, LSL r3        ; r12 = r8 << r3
        mov     r14, r7, LSR r4        ; r14 = r7 >> r4
        orr     r12, r12, r14          ; r12 = r12 | r14
        mov     r14, r8, LSR r4        ; shifted out bytes in r14

        ;
        ; store constructed data away...
        ;

        ; try for 16
        cmp     r2, #16
        stmgeia r0!, { r9,r10,r11,r12 } ; put away 16 bytes
        subge   r2, r2, #16            ; update byte count
        bgt     ba_wa_copy_loop        ; go back for more
        moveq   r14, r9                ; shifted out bytes in r14 
        beq     %FT6                   ; check for <=3 bytes left (in r14)

        ; try for 12
3       stmia   r0!, { r9,r10,r11 }    ; put away 12 bytes
        subs    r2, r2, #12            ; update byte count
        movge   r14, r12               ; move remaining into shifter
        bge     %FT6                   ; check for <=3 bytes left (in r14)

        ; try for 8
1       stmia   r0!, { r9,r10 }        ; put away 8 bytes
        subs    r2, r2, #8             ; update byte count
        movge   r14, r11               ; move remaining into shifter
        bge     %FT6                   ; check for <=3 bytes left (in r14)
 
        ; try for 4
4       str     r9, [r0], #4           ; put away 4 bytes
        subs    r2, r2, #4             ; update byte count
        movge   r14, r10               ; move remaining into shifter
        beq     %FT7                   ; are we done?

        ;
        ; we have 3 or less bytes in r14 to take care of
        ; do required byte stores
        ;

6       cmp     r2, #0                 ; are we done?
        beq     %FT7

8       strb    r14, [r0], #1          ; store next remaining byte
        subs    r2, r2, #1             ; update byte count
        movne   r14, r14, LSR #8       ; move next byte into position
        bgt     %BT8


7       ldmfd    sp!, {r8-r12,lr}      ; restore regs
        bx       lr


        END

;****************************************************************************
;* Modifications:
;* $Log: 
;*  6    mpeg      1.5         7/10/03 10:45:38 AM    Larry Wang      SCR(s) 
;*        6924 :
;*        Change [...] to IF...ENDIF.
;*        
;*  5    mpeg      1.4         5/14/03 4:08:32 PM     Tim White       SCR(s) 
;*        6346 6347 :
;*        Separated the fast-fill functions from the fast-copy functions.
;*        
;*        
;*  4    mpeg      1.3         5/20/02 5:46:54 PM     Bobby Bradford  SCR(s) 
;*        3834 :
;*        Made AREA directive conditional to remove INTERWORK warning
;*        
;*  3    mpeg      1.2         9/28/01 5:44:30 PM     Tim White       SCR(s) 
;*        2691 :
;*        Use the new performance optimized FCopy() function for o_memmove() 
;*        calls
;*        instead of memcpy() or FCopyBytes() which are both slower.
;*        
;*        
;*  2    mpeg      1.1         9/26/01 4:09:18 PM     Dave Moore      SCR(s) 
;*        2686 :
;*        
;*        
;*  1    mpeg      1.0         9/4/01 2:01:22 PM      Dave Moore      
;* $
   
      Rev 1.5   10 Jul 2003 09:45:38   wangl2
   SCR(s) 6924 :
   Change [...] to IF...ENDIF.
   
      Rev 1.4   14 May 2003 15:08:32   whiteth
   SCR(s) 6346 6347 :
   Separated the fast-fill functions from the fast-copy functions.
   
;*
;****************************************************************************

