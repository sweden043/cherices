/**************************************************************************/
/**                                                                      **/
/** Sabine Project                                                       **/
/**                                                                      **/
/** Copyright (C) Rockwell Corporation 1998-1999.   All rights reserved. **/
/**                                                                      **/
/**************************************************************************/
/*------------------------------------------------------------------------*/
/* This file was taken from \osd\bitblt.s r1.11 when the OSDLIB was split */
/* from the OpenTv OSD driver.                                            */
/**************************************************************************/
.set USE_GCP, 0
.set PC_SIM,  1
.set NEVER,   0

#include "general.mvx"
.if USE_GCP
#include "gcpdefs.inc"
.endif /* USE_GCP */

         .data
CacheConfig: 
         .int 0

         .text

/*-------------------------------------------------------------------------*/
/* void GcpBegin()                                                         */
/*                                                                         */
/*Description: Do whatever it takes to use the GCP                         */
/*                                                                         */
/*Input:                                                                   */
/*Returns: CacheConfig                                                     */
/*-------------------------------------------------------------------------*/
.global _GcpStart
_GcpStart:
GcpStart:

/*-------------------------------------------------------------------------*/
/* void GcpEnd(DWORD dwState)                                              */
/*                                                                         */
/*Description: Undo whatever it takes to use the GCP                       */
/*                                                                         */
/*Input:  cacheConfig                                                      */
/*Returns: nothing                                                         */
/*-------------------------------------------------------------------------*/
.global _GcpStop
_GcpStop:
GcpStop:
        mov             pc, lr


/*-------------------------------------------------------------------------*/
/* FillBytes                                                               */
/*       Fills Count bytes starting at address in r0 with the byte in r1   */
/* r0 pDst                                                                 */
/* r1 Fill Byte                                                            */
/* r2 Count                                                                */
/*                                                                         */
/* Returns                                                                 */
/*       Nothing                                                           */
/*-------------------------------------------------------------------------*/
.global _FillBytes
_FillBytes:
FillBytes:
        ands    r3, r0, #3
        beq     LineAligned
                rsb             r3, r3, #4
        cmp     r3, r2
        movgt   r3, r2
        sub     r2, r2, r3
        movs    r3, r3, lsr #1
        strneb  r1, [r0], #1
        strneb  r1, [r0], #1
        strcsb  r1, [r0], #1

LineAligned:
        movs    r3, r2, lsr #2          /* r3 = words*/
        beq     PartialWord
        mov     r1, r1, lsl #24
        orr     r1, r1, r1, lsr #8
        orr     r1, r1, r1, lsr #16
MoreWords:
        str     r1, [r0], #4
        subs    r3, r3, #1
        bne     MoreWords

PartialWord:
        and             r2, r2, #3
        movs            r2, r2, lsr #1
        strneb  r1, [r0], #1
        strneb  r1, [r0], #1
        strcsb  r1, [r0], #1

        mov             pc, lr


/*-------------------------------------------------------------------------*/
/* SetupFB   r0=FB Address r1=bpp                                          */
/*                                                                         */
/*-------------------------------------------------------------------------*/
.if PC_SIM
.global SetupFB
SetupFB:
        ldr             r3, [r0]
//        ldr             r2, =0x55aa55aa
        ldr             r2, 1f
        str             r2, [r0]
//        ldr             r2, =0x5a5a5a5a
        ldr             r2, 2f
        str             r2, [r0]
        str             r1, [r0]
        str             r3, [r0]
        mov             pc, lr
1:
    .int  0x55aa55aa
2:
    .int  0x5a5a5a5a
.endif

/*-------------------------------------------------------------------------*/
/* r0 color                                                                */
/* r1 bpp (2,3,4)                                                          */
/* returns color in r0                                                     */
/*-------------------------------------------------------------------------*/
.global RepColor
RepColor:
        cmp     r1, #2
        bne     Chk8
        and     r0, r0, #0x0F   /* isolate 4 low bits*/
        mov     r2, r0, lsl #4
        orr     r2, r2, r0      /* r2 = 8 bits of color*/
        mov     r3, r2, lsl #8
        orr     r2, r2, r3      /* r2 = 16 bits of color*/
        mov     r3, r2, lsl #16
        orr     r0, r2, r3      /* r0 = 32 bits of color*/
        mov     pc, lr

.global Chk8
Chk8:
        cmp     r1, #3
        bne     Chk16
        mov     r0, r0, lsl #24
        orr     r0, r0, r0, lsr #8
        orr     r0, r0, r0, lsr #16
        mov     pc, lr

.global Chk16
Chk16:
        orr     r0, r1, r0, lsl #16
        mov     pc, lr

/*-------------------------------------------------------------------------*/
/* int ConvertXY(int pFB, int Pitch, int X, int Y, int Bpp)                */
/* r0 pFB                                                                  */
/* r1 Pitch in bytes                                                       */
/* r2 X                                                                    */
/* r3 Y                                                                    */
/* P1 Bpp (0,1,2,3,4,5,7) 1bpp,2,4,8,16,32,24                              */
/*                                                                         */
/* returns in r0 the byte offset of the X,Y pixel                          */
/* uses r3                                                                 */
/*-------------------------------------------------------------------------*/
.global _ConvertXY
_ConvertXY:
ConvertXY:
        mENTER  0
        mla     r0, r1, r3, r0                  /* start of row Y in FB*/
        mGET_STACK_PARAM r3, STACK_P1           /* Indicates BPP*/
        mov     r2, r2, lsl r3                  /* r2 = bit address of X*/
        add     r0, r0, r2, lsr #3
        mLEAVE


MaskTable4BPP:   
        .int  0x000000F0, 0x0000000F, 0x0000F000, 0x00000F00 
        .int  0x00F00000, 0x000F0000, 0xF0000000, 0x0F000000

MaskTable:
        .int  0x000000FF, 0x0000FF00, 0x00FF0000, 0xFF000000

SWFillEdge:
        mPUSH   "{r0,r2,r3,r9,r10,lr}"
/* r0 : byte address             */
/* r1 : Pitch                    */
/* r2 : color                    */
/* r3 : ExtY                     */
/* r4 : DstX                     */
/* r5 : Bpp                      */
/* r8 : number of pixels to copy */
        cmp     r5, #2
        bne     Not4Bpp

        mov     r9, r8
        adr     r6, MaskTable4BPP
        and     r4, r4, #0x07
        add     r6, r6, r4, lsl #2
        mov     r10, #0

GetMask4Bpp:
        ldr     r7, [r6], #4
        orr     r10, r10, r7    /* r10 = Mask*/
        subs    r9, r9, #1
        bne     GetMask4Bpp

        and     r2, r2, r10
        mov     r7, #-1
        eor     r10, r10, r7
        bic     r0, r0, #3

Next4Bpp:
        ldr     r7, [r0]
        and     r7, r7, r10
        orr     r7, r7, r2
        str     r7, [r0]
        add     r0, r0, r1
        subs    r3, r3, #1
        bne     Next4Bpp
        mPOP    "{r0,r2,r3,r9,r10,pc}"


Not4Bpp:
        mov     r10, #-1
        mov     r4, r4, lsl r5          /* r4 = DstX bit address*/
        and     r4, r4, #0x1F
        mov     r10, r10, lsr r4
        mov     r10, r10, lsl r4        /* clear the lower bytes of the word*/
        rsb     r4, r4, #32             /* r4 bits left in word*/
        sub     r4, r4, r8, lsl r5
        mov     r10, r10, lsl r4
        mov     r10, r10, lsr r4        /* r10 mask*/

        and     r2, r2, r10
        mov     r7, #-1
        eor     r10, r10, r7
        bic     r0, r0, #3
NextXBpp:
        ldr     r7, [r0]
        and     r7, r7, r10
        orr     r7, r7, r2
        str     r7, [r0]
        add     r0, r0, r1
        subs    r3, r3, #1
        bne     NextXBpp

        mPOP    "{r0,r2,r3,r9,r10,pc}"


/*-------------------------------------------------------------------------*/
/* SWSolidFill(BYTE *pDest, int DeltaX, int ExtX, int ExtY, int Color,     */
/*              int X, int Y, int bpp)                                     */
/*                                                                         */
/*Description: Fill a rectangular region on the screen with a single color.*/
/*             No clipping is performed.                                   */
/*Input:                                                                   */
/*       r0      BYTE *pDest: Address of the start of the FB               */
/*       r1      int DeltaX : Pitch                                        */
/*       r2      int ExtX   : width in bytes of the rectangle              */
/*       r3      int ExtY   : Height of rectangle                          */
/*       STACK_P1       : Color                                            */
/*       STACK_P2       : X                                                */
/*       STACK_P3       : Y                                                */
/*       STACK_P4       : Bpp                                              */
/*-------------------------------------------------------------------------*/
.global _SWSolidFill
_SWSolidFill:
SWSolidFill:
        mENTER 0

        mPUSH   "{r2,r3}"
        mGET_STACK_PARAM r2, STACK_P4
        mPUSH   "{r2}"
        mGET_STACK_PARAM r2, STACK_P2
        mGET_STACK_PARAM r3, STACK_P3
        bl      ConvertXY
        add     sp, sp, #4
        mPOP    "{r2,r3}"
/* r0 = byte address of first pixel*/

        mGET_STACK_PARAM r4, STACK_P2           /*DstX*/
        mGET_STACK_PARAM r5, STACK_P4           /*BPP*/
        mGET_ALIGN_COUNTS r4, r2, r5, r8, r10, r9
/* r8 = pixels in first word*/
/* r10 = bits in middle*/
/* r9 = pixels in last word*/

        mGET_STACK_PARAM r2, STACK_P1           /* color*/

        cmp     r8, #0
        blne    SWFillEdge

        cmp     r8, #0
        addne   r0, r0, #4             /* r0 = address of next dest word*/
        bic     r0, r0, #3

        movs    r8, r10, lsr #5        /* r8 = number of words in the middle*/
        add     r4, r0, r10, lsr #3    /* r4 =  right edge byte address*/
        mov     r5, r3                 /* r3 = ExtY*/
        beq     SWFillRightEdge

        mov     r5, r3          /* r3 = ExtY*/

SWFillLineLoop:
        mov     r6, r8
        mPUSH "{r0}"
SWFill:
        str     r2, [r0], #4
        subs    r6, r6, #1
        bne     SWFill

        mPOP  "{r0}"
        add     r0, r0, r1
        subs    r5, r5, #1
        bne     SWFillLineLoop

        add     r0, r0, r1
        add     r0, r0, #4

SWFillRightEdge:
        mGET_STACK_PARAM r5, STACK_P4           /*BPP*/
        movs    r8, r9
        mov     r0, r4
        mov     r4, #0
        blne    SWFillEdge

        mLEAVE

SWXOREdge:
        mPUSH   "{r0,r2,r3,r9,r10,lr}"
/* r0 : byte address             */
/* r1 : Pitch                    */
/* r2 : color                    */
/* r3 : ExtY                     */
/* r4 : DstX                     */
/* r5 : Bpp                      */
/* r8 : number of pixels to copy */
        cmp     r5, #2
        bne     XORNot4Bpp

        mov     r9, r8
        adr     r6, MaskTable4BPP
        and     r4, r4, #0x07
        add     r6, r6, r4, lsl #2
        mov     r10, #0

XORGetMask4Bpp:
        ldr     r7, [r6], #4
        orr     r10, r10, r7    /* r10 = Mask*/
        subs    r9, r9, #1
        bne     XORGetMask4Bpp

        and     r2, r2, r10
        mov     r7, #-1
        eor     r10, r10, r7
        bic     r0, r0, #3

XORNext4Bpp:
        ldr     r7, [r0]
        eor     r7, r7, r2
        str     r7, [r0]
        add     r0, r0, r1
        subs    r3, r3, #1
        bne     XORNext4Bpp
        mPOP    "{r0,r2,r3,r9,r10,pc}"


XORNot4Bpp:
        mov     r10, #-1
        mov     r4, r4, lsl r5          /* r4 = DstX bit address*/
        and     r4, r4, #0x1F
        mov     r10, r10, lsr r4
        mov     r10, r10, lsl r4        /* clear the lower bytes of the word*/
        rsb     r4, r4, #32             /* r4 bits left in word*/
        sub     r4, r4, r8, lsl r5
        mov     r10, r10, lsl r4
        mov     r10, r10, lsr r4        /* r10 mask*/

        and     r2, r2, r10
        mov     r7, #-1
        eor     r10, r10, r7
        bic     r0, r0, #3
XORNextXBpp:
        ldr     r7, [r0]
        eor     r7, r7, r2
        str     r7, [r0]
        add     r0, r0, r1
        subs    r3, r3, #1
        bne     XORNextXBpp

        mPOP    "{r0,r2,r3,r9,r10,pc}"


/*-------------------------------------------------------------------------*/
/* SWXORFill(BYTE *pDest, int DeltaX, int ExtX, int ExtY, int Color, int X,*/
/*           int Y, int bpp)                                               */
/*                                                                         */
/*Description: Fill a rectangular region on the screen with a single color.*/
/*             No clipping is performed but it is XORed with dest          */
/*Input:                                                                   */
/*       r0      BYTE *pDest: Address of the start of the FB               */
/*       r1      int DeltaX : Pitch                                        */
/*       r2      int ExtX   : width in bytes of the rectangle              */
/*       r3      int ExtY   : Height of rectangle                          */
/*       STACK_P1       : Color                                            */
/*       STACK_P2       : X                                                */
/*       STACK_P3       : Y                                                */
/*       STACK_P4       : Bpp                                              */
/*-------------------------------------------------------------------------*/
.global _SWXORFill
_SWXORFill:
SWXORFill:
        mENTER 0

        mPUSH   "{r2,r3}"
        mGET_STACK_PARAM r2, STACK_P4
        mPUSH   "{r2}"
        mGET_STACK_PARAM r2, STACK_P2
        mGET_STACK_PARAM r3, STACK_P3
        bl      ConvertXY
        add     sp, sp, #4
        mPOP    "{r2,r3}"
/* r0 = byte address of first pixel*/

        mGET_STACK_PARAM r4, STACK_P2           /*DstX*/
        mGET_STACK_PARAM r5, STACK_P4           /*BPP*/
        mGET_ALIGN_COUNTS r4, r2, r5, r8, r10, r9
/* r8 = pixels in first word*/
/* r10 = bits in middle*/
/* r9 = pixels in last word*/

        mGET_STACK_PARAM r2, STACK_P1           /* color*/

        cmp     r8, #0
        blne    SWXOREdge

        cmp     r8, #0
        addne   r0, r0, #4           /* r0 = address of next dest word*/
        bic     r0, r0, #3

        movs    r8, r10, lsr #5      /* r8 = number of words in the middle*/
        add     r4, r0, r10, lsr #3  /* r4 =  right edge byte address*/
        mov     r5, r3               /* r3 = ExtY*/
        beq     SWXORRightEdge

        mov     r5, r3          /* r3 = ExtY*/

SWXORLineLoop:
        mov     r6, r8
        mPUSH "{r0,r1}"
SWXOR:
        ldr     r1, [r0]
        eor     r1, r1, r2
        str     r1, [r0], #4
        subs    r6, r6, #1
        bne     SWXOR

        mPOP  "{r0,r1}"
        add     r0, r0, r1
        subs    r5, r5, #1
        bne     SWXORLineLoop

        add     r0, r0, r1
        add     r0, r0, #4

SWXORRightEdge:
        mGET_STACK_PARAM r5, STACK_P4           /*BPP*/
        movs    r8, r9
        mov     r0, r4
        mov     r4, #0
        blne    SWXOREdge

        mLEAVE


/*-------------------------------------------------------------------------*/
/* SWCopyLine4Bpp                                                          */
/*       Copies a single line of 4 bpp pixels from the source to the       */
/*       destination.                                                      */
/* Entry                                                                   */
/*       r0 Dst byte address                                               */
/*       r1 Src byte Address (always byte aligned)                         */
/*       r2 DstX (nibble address)                                          */
/*       r3 ExtX in pixels                                                 */
/*       STACK_P1 SrcX                                                     */
/*       STACK_P2 BPP                                                      */
/*-------------------------------------------------------------------------*/
.if USE_GCP
.global _SWCopyLine4Bpp
_SWCopyLine4Bpp:
SWCopyLine4Bpp:
        mENTER 0
        tst     r2, #1
        and     r2, r0, #3
        mov     r2, r2, lsl #1
        addne   r2, r2, #1
        mcr     gcp, gcp_move, r2, gcp_dst_addr, gcp_ctl_reg

        mGET_STACK_PARAM r4, STACK_P1
        tst     r4, #1
        and     r4, r1, #3
        mov     r4, r4, lsl #1
        addne   r4, r4, #1
        mcr     gcp, gcp_move, r4, gcp_src_addr, gcp_ctl_reg    /* move to coproc src pixel address*/
        mSET_STACK_PARAM r4, STACK_P1

        ands    r4, r2, #7              /* r4 = dst X pixels address in first word*/
        beq     DstAligned4Bpp
        rsb     r4, r4, #8              /* r4 = pixels in first dest word*/
        cmp     r4, r3
        movgt   r4, r3
        sub     r3, r3, r4              /* r3 = pixels left*/

/*       mcr     gcp, gcp_move, r2, gcp_dst_addr, gcp_ctl_reg    ! move to coproc src pixel address*/
        mGET_STACK_PARAM r5, STACK_P1
/*       mcr     gcp, gcp_move, r5, gcp_src_addr, gcp_ctl_reg    ! move to coproc src pixel address*/
        add     r5, r5, r4
        mSET_STACK_PARAM r5, STACK_P1
        mcr     gcp, gcp_move, r4, gcp_count, gcp_ctl_reg       /* load number of pixels on a line*/
        ldr     r5, =gcp_rop2 + gcp_srccopy + gcp_4bpp          /* r5 = control reg value*/
        mcr     gcp, gcp_move, r5, gcp_control, gcp_ctl_reg     /* load control reg*/

/* copy first partial word*/
        bic     r0, r0, #3      /* dest address*/
        bic     r5, r1, #3
        ldc     gcp, gcp_fifo_src, [r5]
        ldc     gcp, gcp_fifo_dst, [r0]
        nop
        nop
        stc     gcp, gcp_fifo_dst, [r0]

        add     r1, r1, r4, lsr #1
        add     r0, r0, #4
        mov     r5, #0
        mcr     gcp, gcp_move, r5, gcp_dst_addr, gcp_ctl_reg    /* move to coproc src pixel address*/

/* r3 = ExtX in pixels*/
DstAligned4Bpp:
        mov     r4, r3, lsl #2          /* r4 = ExtX in bits*/
        movs    r3, r4, lsr #5          /* r3 = whole words*/
        beq     DoLastWord4Bpp
        mov     r3, r3, lsl #2          /* r3 = bytes in the middle*/

        mGET_STACK_PARAM r5, STACK_P1
        mcr     gcp, gcp_move, r5, gcp_src_addr, gcp_ctl_reg    /* move to coproc src pixel address*/
        add     r5, r5, r3, lsl #1
        mSET_STACK_PARAM r5, STACK_P1
/*       mov     r5, #0*/
/*       mcr     gcp, gcp_move, r5, gcp_dst_addr, gcp_ctl_reg    ! move to coproc src pixel address*/
        mov     r5, r3, lsl #1
        mcr     gcp, gcp_move, r5, gcp_count, gcp_ctl_reg       /* load number of pixels on a line*/
        ldr     r5, =gcp_srccopy + gcp_4bpp + gcp_copy          /* r5 = control reg value*/
        mcr     gcp, gcp_move, r5, gcp_control, gcp_ctl_reg     /* load control reg*/

        mov     r5, #GCP_FIFO_BITS
        mov     r5, r5, lsr #2  /* ExtX of a full FIFO in pixels*/

FifoLoop2Screen4Bpp:
        subs    r3, r3, #GCP_FIFO_BYTES           /* r4 = width - fifo size*/
/* MKI HW Fix*/
        mcrge   gcp, gcp_move, r5, gcp_count, gcp_ctl_reg   /* load number of pixels on a line*/
/* MKI HW Fix*/
        ldcge   gcp, gcp_fifo_src, [r1], #GCP_FIFO_BYTES
        nop
        nop
        stcge   gcp, gcp_fifo_dst, [r0], #GCP_FIFO_BYTES
        bgt     FifoLoop2Screen4Bpp

        beq     DoLastWord4Bpp
        add     r3, r3, #GCP_FIFO_BYTES           /* r4 = remaining pixels in bytes*/
/* MKI HW Fix*/
        mov     r7, r3, lsl #3
        mov     r7, r7, lsr #2
        mcr     gcp, gcp_move, r7, gcp_count, gcp_ctl_reg               /* load number of pixels on a line*/
/* MKI HW Fix*/
        ldc     gcp, gcp_fifo_src, [r1]
        add     r1, r1, r3
        nop
        stc     gcp, gcp_fifo_dst, [r0]
        add     r0, r0, r3

DoLastWord4Bpp:
/* r4 = ExtX in bits*/
        ands    r4, r4, #0x1F                   /* r4 = get remainder if any*/
        beq     SWCopyLine4BppDone
        mov     r4, r4, lsr #2                  /* r4 = pixels in last word*/
        mcr     gcp, gcp_move, r4, gcp_count, gcp_ctl_reg               /* load number of pixels on a line*/
        mGET_STACK_PARAM r5, STACK_P1
        mcr     gcp, gcp_move, r5, gcp_src_addr, gcp_ctl_reg    /* move to coproc src pixel address*/
        ldr     r5, =gcp_rop2+gcp_srccopy + gcp_4bpp            /* r5 = control reg value*/
        mcr     gcp, gcp_move, r5, gcp_control, gcp_ctl_reg     /* load control reg*/
        bic     r0, r0, #3      /* dest address*/
        bic     r5, r1, #3
        ldc     gcp, gcp_fifo_src, [r5]
        ldc     gcp, gcp_fifo_dst, [r0]
        nop
        nop
        stc     gcp, gcp_fifo_dst, [r0]

SWCopyLine4BppDone:
        mLEAVE
.else

SWCopyLine4Bpp:
        mENTER  0
        mGET_STACK_PARAM        r4, STACK_P1
        tst     r2, #1
        bne     DestNotAligned
        tsteq   r4, #1
        beq     DestAndSrcByteAligned

/*-------------------------------------------------------------------------*/
/* Src not aligned Dst IS                                                  */
/*-------------------------------------------------------------------------*/
/* Low Nibble ---> Hi Nibble                                               */
/*-------------------------------------------------------------------------*/
        mov     r4, r3, lsr #1          /* r4 = number of bytes*/
        ldrb    r6, [r1], #1            /* r6 = get first source nibble*/

        and     r6, r6, #0x0F           /* r6 = isolate low nibble of source*/
SrcNotAlignedLoop:
        mov     r6, r6, lsl #4
        ldrb    r7, [r1], #1            /* r7 = next 2 nibbles of source*/
        and     r8, r7, #0xF0
        orr     r6, r6, r8, lsr #4
        strb    r6, [r0], #1
        and     r6, r7, #0x0F
        subs    r4, r4, #1
        bne     SrcNotAlignedLoop

/* Check last nibble if any*/
        tst     r3, #1
        beq     SWCopy2ScreenDone
        ldrb    r7, [r0]
        and     r7, r7, #0x0F
        orr     r7, r7, r6, lsl #4
        strb    r7, [r0]
        b       SWCopy2ScreenDone

DestNotAligned:
        tst     r4, #1
        beq     DestNotAlignedSrcIs
/*!!!!!!!!!!!!!!!*/
/* Both unaligned*/
/*!!!!!!!!!!!!!!!*/
        ldrb    r6, [r0], #1
        ldrb    r7, [r1], #1
        and     r6, r6, #0xF0
        and     r7, r7, #0x0F
        orr     r6, r6, r7
        strb    r6, [r0]
        b       DestAndSrcByteAligned


DestNotAlignedSrcIs:
/* High Nibble ---> Low Nibble*/
        ldrb    r7, [r1], #1
        and     r5, r7, #0xF0
        mov     r5, r5, lsr #4
        ldrb    r6, [r0]
        and     r6, r6, #0xF0
        orr     r6, r6, r5
        strb    r6, [r0], #1
        sub     r3, r3, #1
        movs    r4, r3, lsr #1  /* r4 = bytes left*/
        beq     SWLastNibble

SWUnAlignedLoop:
        and     r7, r7, #0x0F
        mov     r7, r7, lsl #4
        ldrb    r8, [r1], #1
        mov     r9, r8
        and     r8, r8, #0xF0
        mov     r8, r8, lsr #4
        orr     r8, r8, r7
        strb    r8, [r0], #1
        mov     r7, r9
        subs    r4, r4, #1
        bne     SWUnAlignedLoop

SWLastNibble:
        tst     r3, #1
        beq     SWCopy2ScreenDone
        ldrb    r5, [r1]
        and     r5, r5, #0x0F
        mov     r5, r5, lsl #4
        ldrb    r6, [r0]
        and     r6, r6, #0x0F
        orr     r6, r6, r5
        strb    r6, [r0]
        b       SWCopy2ScreenDone


DestAndSrcByteAligned:
        movs    r4, r3, lsr #1          /*r4 = number of bytes*/
        beq     CheckLastNibble
SWCopy2ScreenLoop:
        ldrb    r5, [r1], #1
        strb    r5, [r0], #1
        subs    r4, r4, #1
        bne     SWCopy2ScreenLoop

CheckLastNibble:
        tst     r3, #1
        beq     SWCopy2ScreenDone
        ldrb    r5, [r1]        /* get the source byte*/
        and     r5, r5, #0xf0
        ldrb    r6, [r0]        /* get the dest byte*/
        and     r6, r6, #0x0f
        orr     r6, r6, r5
        strb    r6, [r0]

SWCopy2ScreenDone:
        mLEAVE
.endif   /* ENABLE_GCP*/
/*-------------------------------------------------------------------------*/
/* SWCopyLineXBPP                                                          */
/*       Copies a single line of 8/16 bpp pixels from the source to the    */
/*       destination.                                                      */
/* Entry                                                                   */
/*       r0 Dst byte address                                               */
/*       r1 Src byte Address (always byte aligned)                         */
/*       r2 DstX                                                           */
/*       r3 ExtX in pixels                                                 */
/*       STACK_P1 SrcX                                                     */
/*       STACK_P2 BPP                                                      */
/*-------------------------------------------------------------------------*/
.global _SWCopyLineXBpp
_SWCopyLineXBpp:
SWCopyLineXBpp:
        mENTER  0

        mGET_STACK_PARAM        r7, STACK_P2    /* BPP*/
        mov             r4, r3, lsl r7
        mov             r4, r4, lsr #3          /* r4 = ExtX in bytes*/

        tst             r0, #3
        beq             DestAlignedXBpp
/* Dest is Unaligned, source  may or may not be*/
/* We copy 1-3 bytes then jump to the case where the dest is aligned*/
        and             r6, r0, #3
        rsb             r6, r6, #4
        cmp             r6, r4
        movgt           r6, r4
        sub             r4, r4, r6                      /* r4 = Remaining bytes*/

        cmp             r7, #3
        ble             MoreBytes
/* Here handle 16 bpp*/
        mov             r9, r0
        bic             r7, r0, #3
        bic             r8, r1, #3
        tst             r1, #2
        add             r1, r1, r6

        ldr             r5, [r7]
        ldr             r6, [r8]
        movne           r6, r6, lsr #16         /* from tst above*/

        mov             r9, #-1
        mov             r10, r9, lsr #16                /* r10 = 0x0000FFFF*/
        and             r6, r6, r10

        mov             r9, r5, lsl #16         /* r9  - 0xFFFF0000*/
        tst             r0, #2
        andne           r5, r5, r10
        orrne           r5, r5, r6, lsl #16

        andeq           r5, r5, r9
        orreq           r5, r5, r6
        str             r5, [r7]
        bic             r0, r0, #3
        add             r0, r0, #4
        b               DestAlignedXBpp

MoreBytes:
        ldrb            r5, [r1], #1
        strb            r5, [r0], #1
        subs            r6, r6, #1
        bne             MoreBytes

/* Now our Destination is aligned*/
/* But has ther source become aligned?*/

DestAlignedXBpp:
        ands            r5, r1, #3
        beq             BothAlignedXBpp

/* Dest is Aligned Source is not so calculate the rotation registers*/
        mov             r2, r5, lsl #3

/* r0            = Aligned Destination*/
/* r1            = unaligned Source*/
/* r2            = rotation registers*/
/* r3            = ExtX in bytes*/

        movs            r3,r4
        blne            CopyDestAlignedLine
        b               SWCopyLineXBPPExit

BothAlignedXBpp:
/*       r0 = Destination*/
/*       r1 = Source*/
/*       r2 = ExtX in bytes*/
        movs            r2, r4
        blne            CopyAlignedLine

SWCopyLineXBPPExit:
        mLEAVE


/*-------------------------------------------------------------------------*/
/* SWCopy2Screen                                                           */
/*      Copies a rectangle from memory to the screen without clipping or   */
/*      overlapping                                                        */
/* Entry                                                                   */
/* r0 dest pixel address                                                   */
/* r1 src pixel address                                                    */
/* r2 DestPitch                                                            */
/* r3 SrcPitch                                                             */
/* P1 ExtX                                                                 */
/* P2 ExtY                                                                 */
/* P3 DstX                                                                 */
/* P4 DstY                                                                 */
/* P5 BPP                                                                  */
/*-------------------------------------------------------------------------*/
LineCopy2ScrFuncTable: 
        .int 0,0,SWCopyLine4Bpp,SWCopyLineXBpp,SWCopyLineXBpp,SWCopyLineXBpp

.global _SWCopy2Screen
_SWCopy2Screen:
SWCopy2Screen:
        mENTER 0
/* int ConvertXY(int pFB, int Pitch, int X, int Y, int Bpp)!*/
/* get the byte address of the destination pixel*/
        mPUSH   "{r1,r2,r3}"
        mov     r1, r2                                  /* Dst Pitch*/
        mGET_STACK_PARAM r2, STACK_P3                   /* Dst X*/
        mGET_STACK_PARAM r3, STACK_P4
        mGET_STACK_PARAM r4, STACK_P5
        mPUSH   "{r4}"
        bl      ConvertXY
        add     sp,sp,#4
        mPOP    "{r1,r2,r3}"

/* r0 Dst Pixel byte Address*/
/* r1 Src Data*/
        mov     r10, r4                         /* r10=BPP*/

        adr     r6, LineCopy2ScrFuncTable
        add     r6, r6, r4, lsl #2
        ldr     r6, [r6]

        mGET_STACK_PARAM r8, STACK_P2           /* r8 = ExtY*/
        mov     r4, r2
        mov     r5, r3
        mGET_STACK_PARAM r2, STACK_P3
        mGET_STACK_PARAM r3, STACK_P1

/*r0 Dst byte address*/
/*r1 Src byte Address (always byte aligned)*/
/*r2 DstX (nibble address)*/
/*r3 ExtX in pixels*/
/*STACK_P1 SrcX*/
/*STACK_P2 BPP*/
        mov     r9, #0  /* nibble address of source is always 0 ie. source is byte aligned*/
SWCopyNextLine:
        mPUSH   "{r0,r1,r2,r3}"
        adr     lr, Ret
        mPUSH "{r9,r10}"        /* r10 = BPP, r9 = SrcX*/
        mov     pc, r6  /* effectively a BL SWCopyLine4Bpp or SWCopyLine2ScreenXBpp*/
Ret:
        add     sp, sp, #8
        mPOP    "{r0,r1,r2,r3}"
        add     r0, r0, r4
        add     r1, r1, r5
        subs    r8, r8, #1
        bne     SWCopyNextLine

        mLEAVE

/*-------------------------------------------------------------------------*/
/* SWCopy2Memory                                                           */
/*      Copies a rectangle from the screen to memory without clipping or   */
/*      overlapping                                                        */
/* SWCopy2Memory(int pDest, int pSrc, int DstPitch, int SrcPitch, int ExtX,*/
/*               int ExtY, int SrcX, int SrcY, int Bpp)                    */
/* Entry                                                                   */
/* r0 dest pixel address                                                   */
/* r1 src pixel address                                                    */
/* r2 DestPitch                                                            */
/* r3 SrcPitch                                                             */
/* P1 ExtX                                                                 */
/* P2 ExtY                                                                 */
/* P3 SrcX                                                                 */
/* P4 SrcY                                                                 */
/* P5 BPP                                                                  */
/*-------------------------------------------------------------------------*/
LineCopy2MemFuncTable: 
        .int 0,0,SWCopyLine4Bpp,SWCopyLineXBpp,SWCopyLineXBpp,SWCopyLineXBpp

.global _SWCopy2Memory
_SWCopy2Memory:
SWCopy2Memory:
        mENTER 0
/* int ConvertXY(int pFB, int Pitch, int X, int Y, int Bpp) */
/* get the byte address of the destination pixel            */
        mov     r5, r0
        mov     r0, r1
        mPUSH   "{r1,r2,r3}"
        mov     r1, r3                                  /* Src Pitch*/
        mGET_STACK_PARAM r2, STACK_P3                   /* SrcX*/
        mGET_STACK_PARAM r3, STACK_P4                   /* SrcY*/
        mGET_STACK_PARAM r4, STACK_P5
        mPUSH   "{r4}"
        bl      ConvertXY
        add     sp,sp,#4
        mPOP    "{r1,r2,r3}"

        mov     r7, r4                                  /* r7 = BPP*/
        mov     r1, r0
        mov     r0, r5

/* r0 Dst Pixel byte Address */
/* r1 Src Data               */

        mGET_STACK_PARAM r8, STACK_P2           /* r8 = ExtY*/
        mov     r4, r2
        mov     r5, r3
        mGET_STACK_PARAM r2, STACK_P3
        mGET_STACK_PARAM r3, STACK_P1

        adr     r6, LineCopy2MemFuncTable
        add     r6, r6, r7, lsl #2
        ldr     r6, [r6]

        mov     r10, r7
        mGET_STACK_PARAM r9, STACK_P3                   /* SrcX*/
/* r0 Dst byte address                       */
/* r1 Src byte Address (always byte aligned) */
/* r2 Dst (nibble address)                   */
/* r3 ExtX in pixels                         */
/* STACK_P1 SrcX                             */
/* STACK_P2 BPP                              */

        mov     r2, #0
SWCopyNextLine2Mem:
        mPUSH   "{r0,r1,r2,r3}"
        adr     lr, SWCopy2MemRet   /* bl     SWCopyLine2Memory4Bpp */
        mPUSH "{r9,r10}"            /* r10 = BPP, r9 = SrcX         */
        mov     pc, r6
SWCopy2MemRet:
        add     sp, sp, #8
        mPOP    "{r0,r1,r2,r3}"
        add     r0, r0, r4
        add     r1, r1, r5
        subs    r8, r8, #1
        bne     SWCopyNextLine2Mem


        mLEAVE

/*-------------------------------------------------------------------------*/
/* HWCopy2Screen                                                           */
/*      Copies a rectangle from memory to the screen without clipping or   */
/*      overlapping                                                        */
/* Entry                                                                   */
/* r0 dest pixel address                                                   */
/* r1 src pixel address                                                    */
/* r2 DestPitch                                                            */
/* r3 SrcPitch                                                             */
/* P1 ExtX                                                                 */
/* P2 ExtY                                                                 */
/* P3 DstX                                                                 */
/* P4 DstY                                                                 */
/* P5 BPP                                                                  */
/* Locals                                                                  */
/* V1 SrcX                                                                 */
/* V2 Pixel on left                                                        */
/* V3 Pixels in middle                                                     */
/* V4 Pixels on right                                                      */
/* V5 Pixels per line                                                      */
/*-------------------------------------------------------------------------*/
.if USE_GCP
.global _HWCopy2Screen
_HWCopy2Screen:
HWCopy2Screen:
        mENTER  5

/* int ConvertXY(int pFB, int Pitch, int X, int Y, int Bpp) */
/* get the byte address of the destination pixel            */
        mPUSH   "{r1,r2,r3}"
        mov     r6, r3                                  /* r6 = Src Pitch*/
        mov     r10, r6
        mov     r1, r2                                  /* Dst Pitch*/
        mGET_STACK_PARAM r2, STACK_P3                   /* Dst X*/
        mcr     gcp, gcp_move, r2, gcp_dst_addr, gcp_ctl_reg    /* move to coproc dest pixel address*/
        mGET_STACK_PARAM r3, STACK_P4
        mGET_STACK_PARAM r4, STACK_P5
        mPUSH   "{r4}"
        bl      ConvertXY
        add     sp,sp,#4
        mPOP    "{r1,r2,r3}"

/* r0 Dst Pixel byte Address */
/* r1 Src Data               */
        mGET_STACK_PARAM r6, STACK_P5   /* r6 = BPP*/
        mov     r8, r3, lsl #3
        mov     r8, r8, lsr r6
        mSET_LOCAL_VAR  r8, LOCAL_V5    /* Save number of pixels per line*/

        mov     r4, r1, lsl #3          /* r4 = bit address of the first byte of source data*/
        mov     r4, r4, lsr r6          /* r4 = Pixel Address*/
        and     r4, r4, #7              /* MKI*/
        mSET_LOCAL_VAR r4, LOCAL_V1     /* Save initial SrcX*/
        mov     r4, #0
        mSET_LOCAL_VAR r4, LOCAL_V2
        mSET_LOCAL_VAR r4, LOCAL_V3
        mSET_LOCAL_VAR r4, LOCAL_V4

        mGET_STACK_PARAM r5, STACK_P1   /* r5 = ExtX*/
        mGET_STACK_PARAM r8, STACK_P2   /* r8 = ExtY*/
        mGET_STACK_PARAM r4, STACK_P3   /* r4 = DstX*/
        mov     r4, r4, lsl r6          /* r4 = bit address of DstX*/
        ands    r7, r4, #0x1F
        beq     NoLeftEdge
        rsb     r7, r7, #32             /* number of bits in first dest word*/
        mov     r7, r7, lsr r6          /* r7 = pixels in first word*/
        cmp     r7, r5
        movgt   r7, r5                  /* r7 = pixels in first partial dest word*/
        mSET_LOCAL_VAR r7, LOCAL_V2     /* save pixels on left*/
/* r0 dest pixel address */
/* r1 src pixel address  */
/* r2 DestPitch          */
/* r3 SrcPitch           */
/* r6 Bpp                */
/* r7 ExtX               */
/* r8 ExtY               */
        bl      HWCopy2ScreenEdge       /* copy left edge*/
        mov     r9, r7, lsl r6
        mov     r9, r9, lsr #3
        add     r1, r1, r9
        add     r0, r0, #4
        bic     r0, r0, #0x3
        mov     r9, #0
        mcr     gcp, gcp_move, r9, gcp_dst_addr, gcp_ctl_reg    /* move to coproc dest pixel address*/

/* update SrcX*/
        mGET_LOCAL_VAR r9, LOCAL_V1     /* SrcX*/
        add     r8, r9, r7
        mSET_LOCAL_VAR r8, LOCAL_V1     /* update SrcX*/
/* update src byte pointer if needed                                  */
/*       mov     r8, r9, lsl r6          ; r8 = Original SrcX in bits */
/*       mov     r9, r7, lsl r6          ; r9 ExtX in bits            */
/*       and     r8, r8, #0x1F                                        */
/*       and     r9, r8, #0x1F                                        */
/*       add     r8, r8, r9                                           */
/*       tst     r8, #0x20                                            */
/*       addne   r1, r1, #3              ; r1 next word in src        */
/*       bic     r1, r1, #3                                           */

        subs    r5, r5, r7              /* r5 = remaining pixels */
        beq     Copy2ScreenDone
NoLeftEdge:
        mov     r8, r5, lsl r6          /* r8 = remaining bits      */
        and     r7, r8, #0x1F           /* r7 = bits on the right   */
        mov     r7, r7, lsr r6          /* r7 = pixels on the right */
        mSET_LOCAL_VAR r7, LOCAL_V4     /* r7 = pixels on the right */
        subs    r7, r5, r7
        mSET_LOCAL_VAR r7, LOCAL_V3     /* r7 = pixels in the middle */
        beq     NoMiddle

/* Do the middle part    */
/* r0 dest pixel address */
/* r1 src pixel address  */
/* r2 DestPitch          */
/* r3 SrcPitch           */
/* r6 Bpp                */
/* r7 ExtX               */
/* r8 ExtY               */

        mGET_STACK_PARAM r8, STACK_P2   /* r8 = ExtY */
        bl      HWCopyMiddle2Screen
/* update SrcX */
        mGET_LOCAL_VAR r9, LOCAL_V1     /* SrcX*/
        add     r8, r9, r7
        mSET_LOCAL_VAR r8, LOCAL_V1     /* update SrcX*/
/* update src byte pointer if needed */

        mov     r9, r7, lsl r6          /* r9 ExtX in bits*/
        add     r0, r0, r9, lsr #3
        add     r1, r1, r9, lsr #3

NoMiddle:
        mGET_LOCAL_VAR r7, LOCAL_V4
        cmp     r7, #0
        beq     Copy2ScreenDone
/* r0 dest pixel address */
/* r1 src pixel address  */
/* r2 DestPitch          */
/* r3 SrcPitch           */
/* r6 Bpp                */
/* r7 ExtX               */
/* r8 ExtY               */
        mGET_STACK_PARAM r8, STACK_P2   /* r8 = ExtY*/
        bl      HWCopy2ScreenEdge       /* copy left edge*/

Copy2ScreenDone:
        mLEAVE


HWCopyMiddle2Screen:
        mPUSH   "{r0,r1,r2,r3,r4,r5,r6,r7,r8,r9,r10,lr}"

/* r0 dest pixel address */
/* r1 src pixel address  */
/* r2 DestPitch          */
/* r3 SrcPitch           */
/* r5 pixels per line    */
/* r6 Bpp                */
/* r7 ExtX               */
/* r8 ExtY               */
        mGET_LOCAL_VAR r9, LOCAL_V1                             /* SrcX*/
        mGET_LOCAL_VAR r5, LOCAL_V5                             /* pixels per line*/

        cmp     r6, #2
        addeq   r5, r5, #1
        biceq   r5, r5, #1


        mcr     gcp, gcp_move, r9, gcp_src_addr, gcp_ctl_reg    /* move to coproc src pixel address*/

        mov     r4, #0
        mcr     gcp, gcp_move, r4, gcp_dst_addr, gcp_ctl_reg    /* move to coproc dest pixel address*/
        ldr     r4, =gcp_copy + gcp_srccopy                     /* r4 = control reg value*/
        orr     r4, r4, r6                                      /* or in Bpp*/
        mcr     gcp, gcp_move, r4, gcp_control, gcp_ctl_reg     /* load control reg*/
        mcr     gcp, gcp_move, r7, gcp_count, gcp_ctl_reg       /* load number of pixels on a line*/

        mov     r7, r7, lsl r6          /* ExtX in bits*/
        mov     r7, r7, lsr #3          /* r6 = ExtX in bytes*/

        sub     r2, r2, r7
        sub     r3, r3, r7
/*       bic     r1, r1, #3             ;  source address*/
        bic     r0, r0, #3              /* dest address*/

        mov     r10, #GCP_FIFO_BITS
        mov     r10, r10, lsr r6        /* r10 = pixels per complete fifo*/
        mov     r4, r7                  /* r4 = width in bytes*/
LineLoop2Screen:
        mPUSH   "{r4, r10}"
/* MKI HW Fix*/
        mcr     gcp, gcp_move, r10, gcp_count, gcp_ctl_reg       /* load number of pixels on a line*/
/* MKI HW Fix*/

FifoLoop2Screen:
        subs    r4, r4, #GCP_FIFO_BYTES           /* r4 = width - fifo size*/
        bic     r10, r1, #3
        ldcge   gcp, gcp_fifo_src, [r10], #GCP_FIFO_BYTES
        addge   r1, r1, #GCP_FIFO_BYTES
/*       nop*/
        stcge   gcp, gcp_fifo_dst, [r0], #GCP_FIFO_BYTES
        bgt     FifoLoop2Screen

        beq     NextLine2Screen
        add     r4, r4, #GCP_FIFO_BYTES                         /* r4 = remaining (bytes)*/
/* MKI HW Fix*/
        mov     r10, r4, lsl #3
        mov     r10, r10, lsr r6
        mcr     gcp, gcp_move, r10, gcp_count, gcp_ctl_reg       /* load number of pixels on a line*/
/* MKI HW Fix*/
        bic     r10, r1, #3
        ldc     gcp, gcp_fifo_src, [r10]
        add     r1, r1, r4
        nop
        stc     gcp, gcp_fifo_dst, [r0]
        add     r0, r0, r4
NextLine2Screen:
        mPOP    "{r4, r10}"
        add     r1, r1, r3
        add     r0, r0, r2
        subs    r8, r8, #1

        add     r9, r9, r5
        and     r9, r9, #7
        mcr     gcp, gcp_move, r9, gcp_src_addr, gcp_ctl_reg    /* move to coproc src pixel address*/
        bne     LineLoop2Screen

        mPOP    "{r0,r1,r2,r3,r4,r5,r6,r7,r8,r9,r10,pc}"



HWCopy2ScreenEdge:
        mPUSH   "{r0,r1,r2,r3,r4,r5,r7,r8,r9,lr}"

/* r0 dest pixel address */
/* r1 src pixel address  */
/* r2 DestPitch          */
/* r3 SrcPitch           */
/* r6 Bpp                */
/* r7 ExtX               */
/* r8 ExtY               */
        mGET_LOCAL_VAR r4, LOCAL_V1                             /* SrcX*/
        mGET_LOCAL_VAR r9, LOCAL_V5                             /* Pitch in pixels*/
        mcr     gcp, gcp_move, r4, gcp_src_addr, gcp_ctl_reg    /* move to coproc src pixel address*/
        mcr     gcp, gcp_move, r7, gcp_count, gcp_ctl_reg       /* load number of pixels on a line*/
        ldr     r5, =gcp_rop2 + gcp_srccopy                     /* r5 = control reg value*/
        orr     r5, r5, r6
        mcr     gcp, gcp_move, r5, gcp_control, gcp_ctl_reg     /* load control reg*/

        bic     r0, r0, #3      /* dest address*/

LineLoop12Screen:
        bic     r5, r1, #3
        ldc     gcp, gcp_fifo_src, [r5]
        ldc     gcp, gcp_fifo_dst, [r0]
        nop
        nop
        stc     gcp, gcp_fifo_dst, [r0]
        add     r1, r1, r3
        add     r0, r0, r2
        subs    r8, r8, #1

        add     r4, r4, r9
        mcr     gcp, gcp_move, r4, gcp_src_addr, gcp_ctl_reg    /* move to coproc src pixel address*/

        bne     LineLoop12Screen

        mPOP    "{r0,r1,r2,r3,r4,r5,r7,r8,r9,pc}"

/*-------------------------------------------------------------------------*/
/* HWSolidFill(BYTE *pDest, int DeltaX, int ExtX, int ExtY, int Color,     */
/*             int X, int Y, int bpp)                                      */
/*                                                                         */
/*Description: Fill a rectangular region on the screen with a single color.*/
/*             No clipping is performed.                                   */
/*Input:                                                                   */
/*       r0      BYTE *pDest: Address of the start of the FB               */
/*       r1      int DeltaX : Pitch                                        */
/*       r2      int     ExtX   : width in bytes of the rectangle          */
/*       r3      int ExtY   : Height of rectangle                          */
/*       STACK_P1       : Color                                            */
/*       STACK_P2       : X                                                */
/*       STACK_P3       : Y                                                */
/*       STACK_P4       : Bpp                                              */
/*-------------------------------------------------------------------------*/
.global _HWSolidFill, HWSolidFill
_HWSolidFill:
HWSolidFill:
        mENTER 0

        mGET_STACK_PARAM r4, STACK_P1   /* r4 = color*/
        mcr     gcp, gcp_move, r4, gcp_fg_col, gcp_ctl_reg              /* move to coproc dest pixel address*/
        mcr gcp, gcp_move, r4, gcp_bg_col, gcp_ctl_reg

        mGET_STACK_PARAM r4, STACK_P2           /*DstX*/
        mcr             gcp, gcp_move, r4, gcp_dst_addr, gcp_ctl_reg    /* move to coproc dest pixel address*/

        mPUSH   "{r2,r3}"
        mGET_STACK_PARAM r2, STACK_P4
        mPUSH   "{r2}"
        mGET_STACK_PARAM r2, STACK_P2
        mGET_STACK_PARAM r3, STACK_P3
        bl              ConvertXY
        add             sp, sp, #4
        mPOP    "{r2,r3}"
/* r0 = byte address of first pixel*/


        mGET_STACK_PARAM r5, STACK_P4           /*BPP*/
        mGET_ALIGN_COUNTS       r4, r2, r5, r8, r10, r9

/* r8 = pixels in first word*/
/* r10 = words in middle*/
/* r9 = pixels in last word*/

        cmp             r8, #0
        blne    HWFillEdge

        add     r4, r4, r8                      /* r4 = new DstX*/
        mcr     gcp, gcp_move, r4, gcp_dst_addr, gcp_ctl_reg    /* move to coproc dest pixel address*/

        cmp     r8, #0
        addne   r0, r0, #4                      /* r0 = address of next dest word*/
        bic     r0, r0, #3
        movs    r8, r10, lsr r5                 /* r8 = number of pixels in the middle*/
        add     r4, r0, r10, lsr #3             /* r4 =  right edge byte address*/
        mov     r5, r3          /* r3 = ExtY*/
        beq     FillRightEdge

        mGET_STACK_PARAM r2, STACK_P4                                           /* gcp_Xbpp*/
        ldr     r5, =gcp_fill + gcp_patcopy             /* r5 = control reg value*/
        orr     r2, r2, r5
        mov     r5, r3          /* r3 = ExtY*/
        mcr     gcp, gcp_move, r2, gcp_control, gcp_ctl_reg             /* load control reg*/
        mcr     gcp, gcp_move, r8, gcp_count, gcp_ctl_reg               /* load number of pixels on a line*/

FillLineLoop:
        stc     gcp, gcp_fifo_dst, [r0]
        add     r0, r0, r1
        subs    r5, r5, #1
        bne     FillLineLoop

FillRightEdge:
        movs    r8, r9
        mov     r0, r4
        blne    HWFillEdge

        mLEAVE

/*-------------------------------------------------------------------------*/
/*       r0      BYTE *pDest: Address of the start of the rectangle        */
/*       r1      int DeltaX : Pitch                                        */
/*       r2      int     ExtX   : width in bytes of the rectangle          */
/*       r3      int ExtY   : Height of rectangle                          */
/*       r8      number of pixels to do in word                            */
/*-------------------------------------------------------------------------*/
HWFillEdge:
        mPUSH   "{r0,r1,r2,r3,r4,lr}"
/*       mcr     gcp, gcp_move, r4, gcp_dst_addr, gcp_ctl_reg    ! move to coproc dest pixel address*/

        mGET_STACK_PARAM r2, STACK_P4                           /* bpp*/
        ldr     r4, =gcp_fill + gcp_patcopy                     /* r5 = control reg value*/
        orr     r2, r2, r4
        mcr     gcp, gcp_move, r2, gcp_control, gcp_ctl_reg     /* load control reg*/
        mcr     gcp, gcp_move, r8, gcp_count, gcp_ctl_reg       /* load number of pixels on a line*/

        bic     r0, r0, #3      /* dest address*/
        mov     r2, r3          /* r2 = ExtY*/
FillLineLoop1:
        ldc     gcp, gcp_fifo_dst, [r0]
        nop
        nop
        stc     gcp, gcp_fifo_dst, [r0]
        add     r0, r0, r1
        subs    r2, r2, #1
        bne     FillLineLoop1

        mPOP    "{r0,r1,r2,r3,r4,pc}"
.endif /* USE_GCP */

/*-------------------------------------------------------------------------*/
/* FUNCTION: Copies a single line with the source and destination that are */
/*           aligned                                                       */
/* Input:                                                                  */
/*       r0 = Destination                                                  */
/*       r1 = Source                                                       */
/*       r2 = ExtX in bytes                                                */
/*                                                                         */
/* Uses: r5-r10, r12, r4, r14                                              */
/*-------------------------------------------------------------------------*/
.if USE_GCP
.global _CopyAlignedLine
_CopyAlignedLine:
CopyAlignedLine:
        mENTER  0

        mPUSH   "{r1}"
        mcr     gcp, gcp_move, r1, gcp_src_addr, gcp_ctl_reg    /* move to coproc src pixel address*/
        mcr     gcp, gcp_move, r0, gcp_dst_addr, gcp_ctl_reg    /* move to coproc dest pixel address*/

        cmp     r2, #4
        blt     CopyAlignedLastWord

        ldr     r4, =gcp_copy + gcp_srccopy + gcp_8bpp          /* r4 = control reg value*/
        mcr     gcp, gcp_move, r4, gcp_control, gcp_ctl_reg     /* load control reg*/
        bic     r4, r2, #3
        mcr     gcp, gcp_move, r4, gcp_count, gcp_ctl_reg               /* load number of pixels on a line*/

        bic     r1, r1, #3              /* source address*/
        bic     r0, r0, #3              /* dest address*/

FifoLoop1:
        subs    r4, r4, #GCP_FIFO_BYTES           /* r4 = width - fifo size*/
        ldcge   gcp, gcp_fifo_src, [r1], #GCP_FIFO_BYTES
        stcge   gcp, gcp_fifo_dst, [r0], #GCP_FIFO_BYTES
        bgt     FifoLoop1
        add     r4, r4, #GCP_FIFO_BYTES           /* r4 = remaining bytes*/
        beq     CopyAlignedLastWord
        ldc     gcp, gcp_fifo_src, [r1]
        add     r1, r1, r4
        nop
        stc     gcp, gcp_fifo_dst, [r0]
        add     r0, r0, r4

CopyAlignedLastWord:    /*kkkkkkkkkkkkkkkk*/
        mPOP    "{r4}"             /* SrcX ie. offset within DWORD*/
        bic     r1, r1, #3              /* source address*/
        and     r4, r4, #3
        add     r1, r1, r4

        ands    r2, r2, #3
        beq     CopyAlignedLineDone
        ldrb    r4, [r1], #1
        strb    r4, [r0], #1
        subs    r2, r2, #2
        blt     CopyAlignedLineDone
        ldrgeb  r4, [r1], #1
        strgeb  r4, [r0], #1
        ldrneb  r4, [r1], #1
        strneb  r4, [r0], #1

/* The HW seems to have a bug when executing the following code.             */
/* When stepping thru it it will work but otherwise does not seem to execute.*/
/* so I have replaced it with a software only code.                          */
/* HW Bug                                                                    */
/*       ands    r2, r2, #3                                                  */
/*       beq     CopyAlignedLineDone                                         */
/*       mcr     gcp, gcp_move, r2, gcp_count, gcp_ctl_reg       ! load number of pixels on a line */
/*       ldr     r4, =gcp_rop2 + gcp_srccopy + gcp_8bpp          ! r4 = control reg value          */
/*       mcr     gcp, gcp_move, r4, gcp_control, gcp_ctl_reg     ! load control reg                */
/*       ldc     gcp, gcp_fifo_dst, [r0]                                     */
/*       ldc     gcp, gcp_fifo_src, [r1]                                     */
/*       nop                                                                 */
/*       nop                                                                 */
/*       stc     gcp, gcp_fifo_dst, [r0]                                     */

CopyAlignedLineDone:
        mLEAVE

.else

CopyAlignedLine:
        mENTER  0
        mov             r4, r2
        movs            r4, r4, lsr #2          /* r4 = Number of words*/
        beq             DoLastWord
        movs            r4, r4, lsr #1          /* r4 = Mult. of 2 word , carry set for odd word*/
        ldmcsia         r1!, {r5}               /* copy odd word if carry set*/
        stmcsia         r0!, {r5}
        beq             DoLastWord
        movs            r4, r4, lsr #1          /* r4 = Mult. of 4 words*/
        ldmcsia         r1!, {r6,r7}            /* copy odd 2 words*/
        stmcsia         r0!, {r6,r7}
        beq             DoLastWord
        movs            r4, r4, lsr #1          /* r4 = Mult. of 8 words*/
        ldmcsia         r1!, {r6-r9}            /* copy odd 4 words*/
        stmcsia         r0!, {r6-r9}
        beq             DoLastWord

CopyWordLoop:
        ldmia           r1!, {r5-r10,r12,r14}
        stmia           r0!, {r5-r10,r12,r14}
        subs            r4, r4, #1
        bne             CopyWordLoop

DoLastWord:
        mov             r4, r2                  /*ExtX in bytes*/
        ands            r4, r4, #3                      /* remaining bytes*/
        beq             LineDoneBothAligned
        movs            r4, r4, lsr #1
        beq             ChkLastByte

        mov             r7, #-1
        mov             r8, r7, lsl #16         /* r8 = 0xFFFF0000*/
        mov             r7, r7, lsr #16         /* r7 = 0x0000FFFF*/
        ldr             r5, [r1], #2
        ldr             r6, [r0]
        and             r5, r5, r7
        and             r6, r6, r8
        orr             r6, r6, r5
        str             r6, [r0], #2
ChkLastByte:
        ldrcsb  r6, [r1], #1
        strcsb  r6, [r0], #1

LineDoneBothAligned:
        mLEAVE

.endif /* ENABLE_GCP*/
/*-------------------------------------------------------------------------*/
/* FUNCTION to Copy a single line from an unaligned source to an aligned   */
/* destination                                                             */
/* STACK_P1      = ExtX in bytes                                           */
/* r0            = Aligned Destination                                     */
/* r1            = unaligned Source                                        */
/* r2            = rotation registers                                      */
/* r3            = ExtX in bytes                                           */
/* Uses: r4,r6,r7,r8,r9                                                    */
/*-------------------------------------------------------------------------*/
.global _CopyDestAlignedLine
_CopyDestAlignedLine:
CopyDestAlignedLine:
.if USE_GCP
        mov     r2, r3
        b       CopyAlignedLine
        ELSE

        mENTER  0
        mov             r5, r2
        rsb             r10, r5, #32
        mov             r9, r3

        bic             r1, r1, #3       /* get word address of source*/
        movs            r4, r9, lsr #2   /* r4 = total full words per line*/
        ldreq           r6, [r1]
        beq             DoPartial
        ldmia           r1!, {r6,r7}     /* get things rolling with 2 words from source*/
                                         /* r6= 210X              r7=6543*/
DestAlignedSrcNotLoop:
        mov             r8, r6, lsr r5           /* r8= *210 */
        orr             r8, r8, r7, lsl r10      /* r8= 3210 */
        str             r8, [r0], #4
        mov             r6, r7                   /* r6= 6543 */
        ldr             r7, [r1], #4             /* r7= A987 */
        subs            r4, r4, #1               /* r4 = number of full dwords in destination line */
        bne             DestAlignedSrcNotLoop
        mov             r8, r6, lsr r5           /* r8= *210 */
        orr             r8, r8, r7, lsl r10      /* r8= 3210 */

        ands            r7, r9, #3               /* r7 = remaining bytes in last partial word (0,1,2,3) */
        beq             CopyDestAlignedLineExit
        mov             r7, r7, lsl #3           /* r7 bits in last word */
        mov             r9, #-1
        mov             r7, r9, lsl r7           /* r7 mask */
        eor             r9, r9, r7               /* r9 ~mask */
        ldr             r5, [r0]
        and             r5, r5, r7
        and             r8, r8, r9
        orr             r5, r5, r8
        str             r5, [r0]
        b               CopyDestAlignedLineExit

DoPartial:
        ands            r7, r9, #3               /* r7 = remaining bytes in last partial word*/
        beq             CopyDestAlignedLineExit
        mov             r6, r6, lsr r5
        movs            r7, r7, lsr #1           /* r7 = remaining 16 bit words, Carry set if single byte too*/
        beq             ChkSingle
        mov             r9, #-1
        mov             r10, r9, lsl #16         /* r10= 0xFFFF0000*/
        mov             r9, r9, lsr #16          /* r9 = 0x0000FFFF*/
        ldr             r7, [r0]
        and             r7, r7, r10
        and             r8, r6, r9
        orr             r7, r7, r8
        str             r7, [r0], #2
        mov             r6, r6, lsr #16
ChkSingle:
        bcc             CopyDestAlignedLineExit
        bic             r0, r0, #3
        ldr             r5, [r0]
        mov             r5, r5, lsr #8
        mov             r5, r5, lsl #8
        and             r6, r6, #0xFF
        orr             r6, r5, r6
        str             r6, [r0]

CopyDestAlignedLineExit:
        mLEAVE

.endif

/*******************Code Below is for next generation coprocessor********************************/
.if NEVER

/*-------------------------------------------------------------------------*/
/* HWCopy                                                                  */
/*    Copies a rectangle without regard to clipping or overlapping         */
/* Entry                                                                   */
/* r0 dest pixel address                                                   */
/* r1 src pixel address                                                    */
/* r2 DestPitch                                                            */
/* r3 SrcPitch                                                             */
/* P1 ExtX                                                                 */
/* P2 ExtY                                                                 */
/* P4 DstY                                                                 */
/* P5 SrcX                                                                 */
/* P6 SrcY                                                                 */
/* P7 BPP                                                                  */
/*-------------------------------------------------------------------------*/
HWCopy
        mENTER  1

/* r0 dest pixel address*/
/* r1 src pixel address*/
/* r2 DestPitch*/
/* r3 SrcPitch*/
/* P1 ExtX*/
/* P2 ExtY*/
/* P3 DstX*/
/* P4 DstY*/
/* P5 SrcX*/
/* P6 SrcY*/
/* P7 BPP*/

/* int ConvertXY(int pFB, int Pitch, int X, int Y, int Bpp)!*/
/* get the byte address of the source pixel and the destination pixel*/
        mPUSH   "{r0,r1,r2,r3}"
        mov     r6, r3                                  /* r6 = Src Pitch*/
        mov     r0, #0
        mov     r1, r2                                  /* Dst Pitch*/
        mGET_STACK_PARAM r2, STACK_P3   /* Dst X*/
        mcr     gcp, gcp_move, r2, gcp_dst_addr, gcp_ctl_reg    /* move to coproc dest pixel address*/
        mGET_STACK_PARAM r3, STACK_P4
        mGET_STACK_PARAM r4, STACK_P7
        mPUSH   "{r4}"
        bl      ConvertXY
        add     sp,sp,#4
        mov     r5, r0

        mov     r0, #0
        mov     r1, r6
        mGET_STACK_PARAM r2, STACK_P5   /* SrcX*/
        mcr     gcp, gcp_move, r2, gcp_src_addr, gcp_ctl_reg    /* move to coproc dest pixel address*/
        mGET_STACK_PARAM r3, STACK_P6
        mGET_STACK_PARAM r4, STACK_P7
        mPUSH   "{r4}"
        bl      ConvertXY
        add     sp,sp,#4
        mov     r6, r0
        mPOP    "{r0,r1,r2,r3}"
        add     r0, r0, r5
        add     r1, r1, r6
/* r0 Dst Pixel Address*/
/* r1 Src Pixel Address*/

        mGET_STACK_PARAM r5, STACK_P1   /* r5 = ExtX*/
        mGET_STACK_PARAM r8, STACK_P2   /* r8 = ExtY*/

        mGET_STACK_PARAM r4, STACK_P3   /* r4 = DstX*/
        mGET_STACK_PARAM r6, STACK_P7   /* r6 = BPP*/
        mov             r4, r4, lsl r6  /* r4 = bit address of DstX*/
        ands    r7, r4, #0x1F
        beq     DestAligned
        rsb     r7, r7, #32             /* number of bits in first dest word*/
        mov     r7, r7, lsr r6  /* pixels in first word*/
        cmp     r7, r5
        movgt   r7, r5                  /* r7 = pixels in first partial dest word*/
        bl      HWCopyEdge              /* copy left edge*/
        sub     r5, r5, r7              /* r5 = remaining pixels*/

        add     r0, r0, #3              /* r1 next word in dest*/
        bic     r0, r0, #3

        mGET_STACK_PARAM r4, STACK_P5   /* SrcX*/
        mov     r4, r4, lsl r6                  /* SrcX bit address*/
        and     r4, r4, #0x1F
        add     r4, r4, r7, lsl r6                      /* r4 = Next SrcX Bit Address*/
        tst     r4, #0x20
        addne   r1, r1, #3              /* r0 next word in src*/
        bic     r1, r1, #3
        mov     r4, r4, lsr r6  /* r4 Next SrcX pixel address*/
        mcr     gcp, gcp_move, r4, gcp_src_addr, gcp_ctl_reg    /* move to coproc src pixel address*/
        mov     r4, #0
        mcr     gcp, gcp_move, r4, gcp_dst_addr, gcp_ctl_reg

DestAligned
        ands    r4, r5, #3              /* r4 = bytes on right edge ie. in last dest word*/
        mSET_LOCAL_VAR  r4, LOCAL_V1
        beq             NoPush
        mPUSH   "{r0,r1,r2,r3}"
NoPush
        subs    r5, r5, r4              /* r5 = middle number of bytes*/
        mSET_STACK_PARAM r5, STACK_P1
        beq     NoMiddleCopy
        mov     r4, #0
        mcr     gcp, gcp_move, r4, gcp_dst_addr, gcp_ctl_reg    /* move to coproc dest pixel address*/
        ldr     r4, =gcp_copy + gcp_srccopy                                             /* r4 = control reg value*/
        orr     r4, r4, r6
        mcr     gcp, gcp_move, r4, gcp_control, gcp_ctl_reg             /* load control reg*/
        mcr     gcp, gcp_move, r5, gcp_count, gcp_ctl_reg               /* load number of pixels on a line*/

        sub     r6, r2, r5
        sub     r7, r3, r5
        bic     r1, r1, #3              /* source address*/
        bic     r0, r0, #3              /* dest address*/

        mGET_STACK_PARAM r8, STACK_P2

LineLoop
        mov     r4, r5          /* r4 = width in pixels*/
FifoLoop
        subs    r4, r4, #GCP_FIFO_BYTES           /* r4 = width - fifo size*/
        ldcge   gcp, gcp_fifo_src, [r1], #GCP_FIFO_BYTES
        stcge   gcp, gcp_fifo_dst, [r0], #GCP_FIFO_BYTES
        bgt     FifoLoop
        beq     NextLine
        adds    r4, r4, #GCP_FIFO_BYTES           /* r4 = remaining pixels*/
        ldc     gcp, gcp_fifo_src, [r1]
        add     r1, r1, r4
        stc     gcp, gcp_fifo_dst, [r0]
        add     r0, r0, r4
NextLine
        add     r1, r1, r7
        add     r0, r0, r6
        subs    r8, r8, #1
        bne     LineLoop

        mGET_LOCAL_VAR  r6, STACK_P7    /* BPP*/


NoMiddleCopy

        mGET_LOCAL_VAR  r7, LOCAL_V1    /* r4 = bytes on right edge ie. in last dest word*/
        cmp             r7, #0
        beq             HWCopyRet
        mPOP    "{r0,r1,r2,r3}"
        mov             r5, r5, lsl r6
        mov             r5, r5, lsr #3
        add             r0, r0, r5
        add             r1, r1, r5
        mGET_STACK_PARAM r8, STACK_P2
        bl              HWCopyEdge

HWCopyRet
        mLEAVE


HWCopyEdge
        mPUSH   "{r0,r1,r2,r3,r8,lr}"

/* r0 dest pixel address*/
/* r1 src pixel address*/
/* r2 DestPitch*/
/* r3 SrcPitch*/
/* r7 ExtX*/
/* r8 ExtY*/

/*       mcr     gcp, gcp_move, r0, gcp_dst_addr, gcp_ctl_reg    ! move to coproc dest pixel address*/
/*       mcr     gcp, gcp_move, r1, gcp_src_addr, gcp_ctl_reg    ! move to coproc src pixel address*/
        ldr     r4, =gcp_8bpp + gcp_rop2 + gcp_srccopy                  /* r4 = control reg value*/
        mcr     gcp, gcp_move, r4, gcp_control, gcp_ctl_reg             /* load control reg*/
        mcr     gcp, gcp_move, r7, gcp_count, gcp_ctl_reg               /* load number of pixels on a line*/

        bic             r1, r1, #3      /* source address*/
        bic             r0, r0, #3      /* dest address*/

LineLoop1
        ldc             gcp, gcp_fifo_src, [r1]
        ldc             gcp, gcp_fifo_dst, [r0]
        nop
        nop
        stc             gcp, gcp_fifo_dst, [r0]
        add             r1, r1, r3
        add             r0, r0, r2
        subs    r8, r8, #1
        bne             LineLoop1

        mPOP    "{r0,r1,r2,r3,r8,pc}"



/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
/* HWMonoCopy*/
/*               Copies a rectangle without regard to clipping or overlapping*/
/*               from a mono source to a color destinatrion expanding it.*/
/* Entry*/
/* r0 dest pixel address*/
/* r1 src pixel address*/
/* r2 DestPitch*/
/* r3 SrcPitch*/
/* P1 ExtX*/
/* P2 ExtY*/
/* P3 Src X (Pixel bit address Address)*/
/* P4 Src Y*/
/* P5 Dst X*/
/* P6 Dsy Y*/
/* P7 FG Color*/
/* P8 BG Color*/
/* P9 BPP*/
/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
HWCopyMono
        mENTER  2

/* int ConvertXY(int pFB, int Pitch, int X, int Y, int Bpp)!*/
/* get the byte address of the source pixel and the destination pixel*/
        mPUSH   "{r0,r1,r2,r3}"
        mov             r6, r3                                  /* r6 = Src Pitch*/
        mov             r0, #0
        mov             r1, r2                                  /* Dst Pitch*/
        mGET_STACK_PARAM r2, STACK_P5   /* DstX*/
        mcr     gcp, gcp_move, r2, gcp_dst_addr, gcp_ctl_reg    /* move to coproc dst pixel address*/

        mGET_STACK_PARAM r3, STACK_P6   /* DstY*/
        mGET_STACK_PARAM r4, STACK_P9
        mPUSH   "{r4}"
        bl              ConvertXY
        add             sp, sp, #4
        mov             r5, r0                  /* r5 = dst pixel address*/

        mov             r0, #0
        mov             r1, r6                                  /* Src Pitch*/
        mGET_STACK_PARAM r2, STACK_P3   /* SrcX*/
        mcr     gcp, gcp_move, r2, gcp_src_addr, gcp_ctl_reg    /* move to coproc src pixel address*/
        mGET_STACK_PARAM r3, STACK_P4   /* SrcY*/
        mov             r4, #1
        mPUSH   "{r4}"
        bl              ConvertXY
        add             sp, sp, #4
        mov             r6, r0                  /* r6 = src pixel address*/
        mPOP    "{r0,r1,r2,r3}"
        add             r0, r0, r5
        add             r1, r1, r6

/* r0 dest pixel address*/
/* r1 src pixel address*/
/* r2 DestPitch*/
/* r3 SrcPitch*/
/* P1 ExtX*/
/* P2 ExtY*/
/* P3 SrcX*/

/* setup the Foreground & background colors*/
        mGET_STACK_PARAM r4, STACK_P7   /* r4 = FG color*/
        mcr     gcp, gcp_move, r4, gcp_fg_col, gcp_ctl_reg              /* move to coproc dest pixel address*/
        mGET_STACK_PARAM r4, STACK_P8   /* r4 = BG color*/
        mcr     gcp, gcp_move, r4, gcp_bg_col, gcp_ctl_reg              /* move to coproc dest pixel address*/


        mGET_STACK_PARAM r4, STACK_P9   /* r4 = BPP*/
        mGET_STACK_PARAM r5, STACK_P1   /* r5 = ExtX*/
        mGET_STACK_PARAM r8, STACK_P2   /* r8 = ExtY*/

/* get the destination alignment*/
        mGET_STACK_PARAM r7, STACK_P5   /* r7 = DstX*/
        mov             r7, r7, lsl r4                  /* r7 = DstX bit address*/
        ands    r7, r7, #0x1F                   /* mod 32*/
        beq             DestAlignedMono                 /* jump if destination is aligned to word address*/

        rsb             r7, r7, #32             /* number of bits in first dest word*/
        mov             r7, r7, lsr r4  /* pixels in first word*/

        cmp             r7, r5
        movgt   r7, r5                  /* r7 = pixels in first partial dest word*/
        bl              HWCopyEdgeMono  /* copy left edge*/

        add             r0, r0, #3              /* r1 next word in dest*/
        bic             r0, r0, #3

        mGET_STACK_PARAM r4, STACK_P3   /* r4 = SrcX*/
        and             r4, r4, #0x1F
        add             r4, r4, r7
        tst             r4, #0x20
        addne   r1, r1, #0x03
        bic             r1, r1, #0x03
        and             r4, r4, #0x1f
        mcr             gcp, gcp_move, r4, gcp_src_addr, gcp_ctl_reg    /* move to coproc src pixel address*/

        sub             r5, r5, r7              /* r5 = remaining pixels*/
        mGET_STACK_PARAM r4, STACK_P9   /* r4 = BPP*/

DestAlignedMono
        ldr             r6, =gcp_copy + gcp_srccopy + gcp_expand_src
        orr             r6, r6, r4
        mcr             gcp, gcp_move, r6, gcp_control, gcp_ctl_reg             /* load control reg*/

/* r5 remaining ExtX in pixels*/
        mov             r4, #0
        mcr             gcp, gcp_move, r4, gcp_dst_addr, gcp_ctl_reg    /* dest is word aligned*/

        mGET_STACK_PARAM r6, STACK_P9                           /* BPP*/
        mov             r4, r5, lsl r6                                          /* r4 = remaining bits of ExtX*/
        ands    r4, r4, #0x1F                                           /* r4 = bits on right edge ie. in last dest word*/
        mov             r4, r4, lsr r6                                          /* r4 = pixels on the right edge*/
        mSET_LOCAL_VAR  r4, LOCAL_V1
        mPUSH   "{r0,r1,r2,r3}"

        subs    r5, r5, r4                                                      /* r5 = middle number of pixels*/
        mSET_STACK_PARAM r5, STACK_P1
        beq             NoMiddleCopyMono

        mcr             gcp, gcp_move, r5, gcp_count, gcp_ctl_reg               /* load number of pixels on a line*/

        sub             r6, r2, r5              /* r6 = dst addr delta*/
        mov             r7, r3                  /* r7 = src pitch*/

        bic             r1, r1, #3              /* source address*/
        bic             r0, r0, #3              /* dest address*/

        mGET_STACK_PARAM r8, STACK_P2

LineLoopMono
        mov             r4, r5          /* r4 = width in pixels*/
        ldc             gcp, gcp_fifo_src, [r1]                 /* load the mono line (upto 1024 bits width)*/

FifoLoopMono
        subs    r4, r4, #GCP_FIFO_BYTES           /* r4 = width - fifo size*/
        stcge   gcp, gcp_fifo_dst, [r0], #GCP_FIFO_BYTES
        bgt             FifoLoopMono
        beq             NextLineMono
        adds    r4, r4, #GCP_FIFO_BYTES           /* r4 = remaining pixels*/
        stc             gcp, gcp_fifo_dst, [r0]
        add             r0, r0, r4
NextLineMono
        add             r1, r1, r7
        add             r0, r0, r6
        subs    r8, r8, #1
        bne             LineLoopMono

NoMiddleCopyMono

        mPOP    "{r0,r1,r2,r3}"
        mGET_LOCAL_VAR  r7, LOCAL_V1    /* r4 = bytes on right edge ie. in last dest word*/
        cmp             r7, #0
        beq             HWCopyRetMono

        mGET_STACK_PARAM r4, STACK_P3   /* r4 = SrcX*/
        add             r4, r4, r5
        add             r1, r1, r4, lsr #3
        and             r4, r4, #0x1f
        mcr     gcp, gcp_move, r4, gcp_src_addr, gcp_ctl_reg    /* move to coproc src pixel address*/

        mGET_STACK_PARAM r4, STACK_P9
        mov             r5, r5, lsl r4
        add             r0, r0, r5, lsr #3
        mGET_STACK_PARAM r8, STACK_P2
        bl              HWCopyEdgeMono

HWCopyRetMono
        mLEAVE


HWCopyEdgeMono
        mPUSH   "{r0,r1,r2,r3,r6,r8,lr}"

/* r0 dest pixel address*/
/* r1 src pixel address*/
/* r2 DestPitch*/
/* r3 SrcPitch*/
/* r4 BPP*/
/* r7 ExtX*/
/* r8 ExtY*/
        ldr             r6, =gcp_rop2 + gcp_srccopy + gcp_expand_src
        orr             r6, r6, r4
        mcr             gcp, gcp_move, r6, gcp_control, gcp_ctl_reg             /* load control reg*/
        mcr             gcp, gcp_move, r7, gcp_count, gcp_ctl_reg      /* load number of pixels on a line*/
        bic             r1, r1, #3      /* source address*/
        bic             r0, r0, #3      /* dest address*/

LineLoop1Mono
        ldc             gcp, gcp_fifo_src, [r1]
        ldc             gcp, gcp_fifo_dst, [r0]
        stc             gcp, gcp_fifo_dst, [r0]
        add             r1, r1, r3
        add             r0, r0, r2
        subs    r8, r8, #1
        bne             LineLoop1Mono

        mPOP    "{r0,r1,r2,r3,r6,r8,pc}"


/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
/* r0 = address of pixel in frame buffer*/
/* r3 = DstY*/
/* r4 = ExtY*/
/* r7 = number of pixels*/
/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
HWColPatEdge
        mPUSH   "{r0, r3, lr}"
        mcr             gcp, gcp_move, r7, gcp_count, gcp_ctl_reg                       /* number of pixels in a line*/
        mcr             gcp, gcp_move, r3, gcp_pat_line, gcp_ctl_reg            /* Starting row number ie. DestY*/
        mcr             gcp, gcp_move, r0, gcp_dst_addr, gcp_ctl_reg            /* dest pixel address*/
/*       ldr             r3, =gcp_8bpp + gcp_pat_rop + gcp_patcopy                       !+ gcp_expand_pat*/
/*       mcr             gcp, gcp_move, r3, gcp_control, gcp_ctl_reg*/

        bic             r0, r0, #3
ColPatEdgeLoop
        ldc             gcp, gcp_fifo_dst, [r0]
        stc             gcp, gcp_fifo_dst, [r0]
        add             r0, r0, r1
        subs    r4, r4, #1
        bne             ColPatEdgeLoop

        mPOP    "{r0, r3, pc}"
/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
/*HWColPatFillA8(BYTE *pDest, int DestPitch, int DstX, DstY, int ExtX, int ExtY, BYTE *pColPat)!*/
/**/
/*Description: Fill a rectangular region on the screen with a color pattern. No*/
/*             clipping is performed.*/
/*Input:*/
/*       r0      BYTE *pDest: Address of the start of the frame buffer*/
/*       r1      int DstPitch in bytes*/
/*       r2      int     DstX*/
/*       r3      int DstY*/
/*       STACK_P1       : ExtX*/
/*       STACK_P2       : ExtY*/
/*       STACK_P3           : pColPat*/
/**/
HWColPatFillA8
        mENTER  0

/* set the comand to color pattern and load the pattern*/
        ldr             r4, =gcp_8bpp + gcp_pat_rop + gcp_patcopy                       /*+ gcp_expand_pat*/
        mcr             gcp, gcp_move, r4, gcp_control, gcp_ctl_reg

        mov             r4, #8                                          /* 8x8*/
        mcr             gcp, gcp_move, r4, gcp_pat_size, gcp_ctl_reg            /* pattern pitch*/
        mov             r4, #64                                                                                         /* size of an 8x8 8bpp color pattern*/
        mcr             gcp, gcp_move, r4, gcp_count, gcp_ctl_reg
        mov             r4, #0
        mcr     gcp, gcp_move, r4, gcp_dst_addr, gcp_ctl_reg
        mGET_STACK_PARAM        r4, STACK_P3
        mcr     gcp, gcp_move, r4, gcp_src_addr, gcp_ctl_reg
        bic             r4, r4, #0x03
        ldc             gcp, gcp_fifo_pat, [r4]                                                 /* load the color pattern*/

        mCONV_MEM_XY    r1, r2, r3, r4
        add             r0, r0, r4                                      /* r0 = Address of first pixel*/
        mGET_STACK_PARAM        r5, STACK_P1    /* r5 = ExtX*/

        tst             r0, #3
        beq             DestAlignedColPat

        and             r7, r0, #3              /* byte address in dest*/
        rsb             r7, r7, #4              /* number of bytes in first dest word*/
        cmp             r7, r5
        movgt   r7, r5                  /* r7 = bytes in first partial dest word*/
        mGET_STACK_PARAM        r4, STACK_P2    /* r5 = ExtY*/
        bl              HWColPatEdge    /* Pat Copy Left Edge*/

        add             r0, r0, r7              /* r0 = next word in dest*/
        sub             r5, r5, r7              /* r5 = remaining pixels*/

DestAlignedColPat
/*       ldr             r4, =gcp_8bpp + gcp_pat_fill + gcp_patcopy                      !+ gcp_expand_pat*/
/*       mcr             gcp, gcp_move, r4, gcp_control, gcp_ctl_reg*/

        and             r6, r5, #3              /* r6 = bytes on right edge ie. in last dest word*/
        bics    r5, r5, #3              /* r5 = bytes in the middle*/
        beq             ColPatRightEdgeNoPop

        mcr             gcp, gcp_move, r5, gcp_count, gcp_ctl_reg                       /* number of pixels in a line*/
        mcr             gcp, gcp_move, r3, gcp_pat_line, gcp_ctl_reg            /* Starting row number ie. DestY*/
        mcr             gcp, gcp_move, r0, gcp_dst_addr, gcp_ctl_reg            /* dest pixel address*/

        add             r8, r0, r5                                                                                      /* r5 = address of right edge pixel if any*/
        mPUSH   {r8}
        mGET_STACK_PARAM        r4, STACK_P2

ColPatLineLoop
        mov             r7, r5
        mov             r8, r0
ColorPatFifoLoop
        subs    r7, r7, #GCP_FIFO_BYTES
        stcge   gcp, gcp_fifo_dst, [r8], #GCP_FIFO_BYTES
        bgt             ColorPatFifoLoop
        beq             ColPatNextLine
        add             r7, r7, =GCP_FIFO_BYTES
        stcne   gcp, gcp_fifo_dst, [r8]
ColPatNextLine
        add             r0, r0, r1
        subs    r4, r4, #1
        bne             ColPatLineLoop

ColPatRightEdge
        mPOP    {r0}
ColPatRightEdgeNoPop
        mGET_STACK_PARAM        r4, STACK_P2    /* r5 = ExtY*/
        movs    r7,     r6
        blne    HWColPatEdge

        mLEAVE

/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
/* HWLoadFont(BYTE *pFont, int FifoXOffset, int FifoYOffset, int CharWidth, int CharHeight)!*/
/* Load a character font into the Fifo*/
/**/
/* r0 pFont*/
/* r1 FifoXOffset*/
/* r2 FifoYOffset*/
/* r3 CharWidth*/
/* STACK_P1 CharHeight*/
/**/
/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
HWLoadFont
        mENTER  0

/* pixel addresses*/
        mov             r4, r0, lsl #3
        mcr     gcp, gcp_move, r4, gcp_src_addr, gcp_ctl_reg

        mcr             gcp, gcp_move, r1, gcp_dst_addr, gcp_ctl_reg
        mcr             gcp, gcp_move, r2, gcp_text_line, gcp_ctl_reg

        mGET_STACK_PARAM r4, STACK_P1   /* CharHeight*/
        mcr     gcp, gcp_move, r4, gcp_text_height, gcp_ctl_reg

        mcr             gcp, gcp_move, r3, gcp_count, gcp_ctl_reg /* actual char width*/

        add             r3, r3, #7
        bic             r3, r3, #7

        mcr     gcp, gcp_move, r3, gcp_text_pitch, gcp_ctl_reg

        bic             r0, r0, #3
        ldc     gcp, gcp_fifo_src, [r0]

        mLEAVE


/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
/* HWBltString(BYTE *pDest, int Length, int Height, int Pitch, int DstX, int DstY, int BPP)*/
/* r0 pDest*/
/* r1 Length of Mono bitmap already loaded into src fifo*/
/* r2 Height of mono bitmap*/
/* r3 pitch of destination in bytes*/
/* STACK_P1 DstX*/
/* STACK_P2 DstY*/
/* STACK_P3 BPP*/
/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
HWBltString
        mENTER  0
/* total line length*/
        mcr     gcp, gcp_move, r1, gcp_count, gcp_ctl_reg
/* int ConvertXY(int pFB, int Pitch, int X, int Y, int Bpp)!*/
        mov             r1, r3          /* pitch*/
        mPUSH   "{r2,r3}"
        mGET_STACK_PARAM r2, STACK_P1                   /* DstX*/
        mcr     gcp, gcp_move, r2, gcp_dst_addr, gcp_ctl_reg
        mGET_STACK_PARAM r3, STACK_P2                   /* DstY*/
        mGET_STACK_PARAM r4, STACK_P3
        mPUSH   "{r4}"
        bl              ConvertXY
        add             sp, sp, #4
        mPOP    "{r2,r3}"

        bic             r0, r0, #3
BltStrLineLoop
        ldc             gcp, gcp_fifo_dst, [r0]
        stc             gcp, gcp_fifo_dst, [r0]
        add             r0, r0, r3
        subs    r2, r2, #1
        bne             BltStrLineLoop

        mLEAVE

/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
/* HWTextSetup(int FontWidth, int FontHeight, int FGColor, int BGColor, BPP)!*/
/**/
/**/
/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
HWTextSetup
        mENTER  0
        mGET_STACK_PARAM r5, STACK_P1
        ldr     r4, =gcp_rop2 + gcp_transparent + gcp_expand_src + gcp_text_64 /*+ gcp_text_kerning! 64x8 transp*/
        cmp             r1, #8
        ldrgt   r4, =gcp_rop2 + gcp_transparent + gcp_expand_src + gcp_text_32 /*+ gcp_text_kerning! 32x16 transp*/
        orr             r4, r4, r5

        mcr     gcp, gcp_move, r4, gcp_control, gcp_ctl_reg

        mcr             gcp, gcp_move, r2, gcp_fg_col, gcp_ctl_reg
        mcr             gcp, gcp_move, r3, gcp_bg_col, gcp_ctl_reg

        mLEAVE


/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
/* r0 = address of pixel in frame buffer*/
/* r3 = DstY*/
/* r4 = ExtY*/
/* r7 = number of pixels*/
/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
HWMonoPatEdge
        mPUSH   "{r0, r3, lr}"
        mcr             gcp, gcp_move, r7, gcp_count, gcp_ctl_reg                       /* number of pixels in a line*/
        mcr             gcp, gcp_move, r3, gcp_pat_line, gcp_ctl_reg            /* Starting row number ie. DestY*/
        mcr             gcp, gcp_move, r0, gcp_dst_addr, gcp_ctl_reg            /* dest pixel address*/
/*       ldr             r3, =gcp_8bpp + gcp_pat_rop + gcp_patcopy                       !+ gcp_expand_pat*/
/*       mcr             gcp, gcp_move, r3, gcp_control, gcp_ctl_reg*/

        bic             r0, r0, #3
MonoPatEdgeLoop
        ldc             gcp, gcp_fifo_dst, [r0]
        stc             gcp, gcp_fifo_dst, [r0]
        add             r0, r0, r1
        subs    r4, r4, #1
        bne             MonoPatEdgeLoop

        mPOP    "{r0, r3, pc}"
/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
/*HWMonoPatFillA8(BYTE *pDest, int DestPitch, int DstX, DstY, int ExtX, int ExtY, BYTE *pMonoPat, int FGColor, int BGColor)!*/
/**/
/*Description: Fill a rectangular region on the screen with a mono pattern. No*/
/*             clipping is performed.*/
/*Input:*/
/*       r0      BYTE *pDest: Address of the start of the frame buffer*/
/*       r1      int DstPitch in bytes*/
/*       r2      int     DstX*/
/*       r3      int DstY*/
/*       STACK_P1       : ExtX*/
/*       STACK_P2       : ExtY*/
/*       STACK_P3           : pMonoPat*/
/*       STACK_P4           : FGColor*/
/*       STACK_P5           : BGColor*/
HWMonoPatFillA8
        mENTER  0

        mGET_STACK_PARAM r4, STACK_P4   /* r4 = FG color*/
        mcr     gcp, gcp_move, r4, gcp_fg_col, gcp_ctl_reg              /* move to coproc dest pixel address*/
        mGET_STACK_PARAM r4, STACK_P5   /* r4 = BG color*/
        mcr     gcp, gcp_move, r4, gcp_bg_col, gcp_ctl_reg              /* move to coproc dest pixel address*/

/* set the comand to color pattern and load the pattern*/
        ldr             r4, =gcp_8bpp + gcp_pat_rop + gcp_patcopy + gcp_expand_pat
        mcr             gcp, gcp_move, r4, gcp_control, gcp_ctl_reg

        mov             r4, #8                                          /* 8x8*/
        mcr             gcp, gcp_move, r4, gcp_pat_size, gcp_ctl_reg            /* pattern pitch*/
        mov             r4, #64                                                                                         /* size of an 8x8 1bpp mono pattern*/
        mcr             gcp, gcp_move, r4, gcp_count, gcp_ctl_reg
        mov             r4, #0
        mcr     gcp, gcp_move, r4, gcp_dst_addr, gcp_ctl_reg
        mcr     gcp, gcp_move, r4, gcp_src_addr, gcp_ctl_reg
        mGET_STACK_PARAM r4, STACK_P3   /* r4 = MonoPat*/

        ldc             gcp, gcp_fifo_pat, [r4]                                                 /* load the color pattern*/

        mCONV_MEM_XY    r1, r2, r3, r4
        add             r0, r0, r4                                      /* r0 = byte Address of first pixel*/
        mGET_STACK_PARAM        r5, STACK_P1    /* r5 = ExtX*/

        tst             r0, #3
        beq             DestAlignedMonoPat

        and             r7, r0, #3              /* byte address in dest*/
        rsb             r7, r7, #4              /* number of bytes in first dest word*/
        cmp             r7, r5
        movgt   r7, r5                  /* r7 = bytes in first partial dest word*/
        mGET_STACK_PARAM        r4, STACK_P2    /* r5 = ExtY*/
        bl              HWMonoPatEdge    /* Pat Copy Left Edge*/

        add             r0, r0, r7              /* r0 = next word in dest*/
        sub             r5, r5, r7              /* r5 = remaining pixels*/

DestAlignedMonoPat
/*       ldr             r4, =gcp_8bpp + gcp_pat_fill + gcp_patcopy                      !+ gcp_expand_pat*/
/*       mcr             gcp, gcp_move, r4, gcp_control, gcp_ctl_reg*/

        and             r6, r5, #3              /* r6 = bytes on right edge ie. in last dest word*/
        bics    r5, r5, #3              /* r5 = bytes in the middle*/
        beq             MonoPatRightEdgeNoPop

        mcr             gcp, gcp_move, r5, gcp_count, gcp_ctl_reg                       /* number of pixels in a line*/
        mcr             gcp, gcp_move, r3, gcp_pat_line, gcp_ctl_reg            /* Starting row number ie. DestY*/
        mcr             gcp, gcp_move, r0, gcp_dst_addr, gcp_ctl_reg            /* dest pixel address*/

        add             r8, r0, r5                                                                                      /* r5 = address of right edge pixel if any*/
        mPUSH   {r8}
        mGET_STACK_PARAM        r4, STACK_P2

MonoPatLineLoop
        mov             r7, r5
        mov             r8, r0
MonoPatFifoLoop
        subs    r7, r7, #GCP_FIFO_BYTES
        stcge   gcp, gcp_fifo_dst, [r8], #GCP_FIFO_BYTES
        bgt             ColorPatFifoLoop
        beq             MonoPatNextLine
        add             r7, r7, #GCP_FIFO_BYTES
        stcne   gcp, gcp_fifo_dst, [r8]
MonoPatNextLine
        add             r0, r0, r1
        subs    r4, r4, #1
        bne             MonoPatLineLoop

MonoPatRightEdge
        mPOP    {r0}
MonoPatRightEdgeNoPop
        mGET_STACK_PARAM        r4, STACK_P2    /* r5 = ExtY*/
        movs    r7,     r6
        blne    HWMonoPatEdge

        mLEAVE

/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
/* SWCopyLine2Memory4Bpp*/
/*       Copies a single line of 4 bpp pixels from the source to the destination.*/
/* Entry*/
/*       r0 Dst byte address (always byte aligned)*/
/*       r1 Src byte Address*/
/*       r2 SrcX (nibble address)*/
/*       r3 ExtX in pixels*/
/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
SWCopyLine2Memory4Bpp
        mENTER  0
        mPUSH   "{r0,r1,r2,r3}"
        ands    r4, r2, #1
        beq     SrcIsByteAligned
        movs    r4, r3, lsr #1  /* r4 = bytes left*/
        beq     SWLastNibble2

SWUnAlignedLoop2
        ldrb    r5, [r1], #1
        ldrb    r6, [r1]
        and     r5, r5, #0x0F
        mov     r5, r5, lsl #4
        and     r6, r6, #0xF0
        mov     r6, r6, lsr #4
        orr     r5, r5, r6
        strb    r5, [r0], #1
        subs    r4, r4, #1
        bne     SWUnAlignedLoop2

SWLastNibble2
        tst     r3, #1
        beq     SWCopy2MemoryDone
        ldrb    r5, [r1]
        and     r5, r5, #0x0F
        mov     r5, r5, lsl #4
        ldrb    r6, [r0]
        and     r6, r6, #0x0F
        orr     r6, r6, r5
        strb    r6, [r0]
        b       SWCopy2MemoryDone

SrcIsByteAligned
        movs    r4, r3, lsr #1          /*r4 = number of bytes*/
        beq     CheckLastNibble2
SWCopy2MemoryLoop
        ldrb    r5, [r1], #1
        strb    r5, [r0], #1
        subs    r4, r4, #1
        bne     SWCopy2MemoryLoop

CheckLastNibble2
        tst     r3, #1
        beq     SWCopy2MemoryDone
        ldrb    r5, [r1]        /* get the source byte*/
        and     r5, r5, #0xf0
        ldrb    r6, [r0]        /* get the dest byte*/
        and     r6, r6, #0x0f
        orr     r6, r6, r5
        strb    r6, [r0]

SWCopy2MemoryDone
        mPOP    "{r0,r1,r2,r3}"
        mLEAVE


.endif   /* NEVER*/

.end
