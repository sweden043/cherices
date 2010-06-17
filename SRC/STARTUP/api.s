;****************************************************************************
;*                            Conexant Systems                              *
;****************************************************************************
;*                                                                          *
;* Filename:       api.s                                                    *
;*                                                                          *
;* Description:    API functions for codeloader.                            *
;*                                                                          *
;* Author:         Tim Ross                                                 *
;*                                                                          *
;* Copyright Conexant Systems, 1999                                         *
;* All Rights Reserved.                                                     *
;*                                                                          *
;****************************************************************************

    
;***************
;* Global Data *
;***************

    KEEP
    IF :DEF: |ads$version|
    AREA API, CODE, READONLY
    ELSE
    AREA API, CODE, READONLY, INTERWORK
    ENDIF

;******************
;* Include Files  *
;******************
    GET stbcfg.a

;********************************
;* External Function Prototypes *
;********************************

;**********************
;* Local Definitions  *
;**********************



;********************************************************************
;*  GetChipID                                                       *
;*                                                                  *
;*  PARAMETERS:                                                     *
;*      None.                                                       *
;*                                                                  *
;*  DESCRIPTION:                                                    *
;*      Reads the chip's host bridge PCI ID.                        *
;*                                                                  *
;*  RETURNS:                                                        *
;*      32-bit host bridge PCI vendor/device ID in r0.              *
;********************************************************************
    EXPORT GetChipID
GetChipID
    
    ;Point PCI index register to ID of host bridge config space.
    ldr     r1, =PCI_CFG_ADDR_REG
    mov     r0, #0
    str     r0, [r1]
    
    ;Read PCI host bridge vendor/device ID.
    ldr     r2, =PCI_CFG_DATA_REG
    ldr     r0, [r2]
    
    ;Return to caller.
    bx      lr
    
    
;********************************************************************
;*  GetChipRev                                                      *
;*                                                                  *
;*  PARAMETERS:                                                     *
;*      None.                                                       *
;*                                                                  *
;*  DESCRIPTION:                                                    *
;*      Reads the chip's host bridge PCI revision.                  *
;*                                                                  *
;*  RETURNS:                                                        *
;*      Host bridge PCI revision in r0.                             *
;********************************************************************
    EXPORT GetChipRev
GetChipRev
    
    ;set up r1 and r2 to point to config space
    ldr     r1, =PCI_CFG_ADDR_REG
    ldr     r2, =PCI_CFG_DATA_REG

    ; First look at chip ID.  If it is a WaBASH then we need to possibly
    ; fudge the revision
    mov     r0, #0
    str     r0, [r1]
    ldr     r0, [r2]
    bic     r0, r0, #0x000F0000
    ldr     r3, =WABASH
    bic     r3, r3, #0x000F0000
    cmp     r0, r3
    bne     notwabash

    ; if we get here, we're running a wabash

    ; Get the revision of out of the PCI config space
    ; if we get rev "1" then we could be rev 1 or 2
    mov     r0, #8
    str     r0, [r1]
    ldrb    r0, [r2]
    mov     r3, #1      ; rev 1 is the problem
    cmp     r0, r3     
    bgt     bumprevby1  ; if it's revc1 (i.e. value of 2) or greater need to bump up id by1
    blt     haverev     ; if its less than rev 1, boogie
    
    ; bummer.  we're rev 1.  mess with the DRM regs to find out if
    ; we're rev 1 or rev 2.

    ldr     r1, =0x3046009c
    ldr     r2, =0x98000000
    ldr     r3, =0x0000deaf
    str     r2, [r1]
    str     r3, [r1]
    str     r2, [r1]
    ldrh    r0, [r1]
    cmp     r0, r3
    moveq   r0, #2
    movne   r0, #1
    b       haverev

    ; reva=0, revb=1, revc0=2 dur to above drm reg manipulation
    ; so revc1=2 based on pci chip id will need to be bumped up to 3
bumprevby1
    add    r0, r0, #1
    b      haverev

notwabash    
    ; Get the revision of out of the PCI config space
    mov     r0, #8
    str     r0, [r1]
    ldrb    r0, [r2]

haverev
    ;Return to caller.
    bx      lr
    

;********************************************************************
;*  GetVendorID                                                     *
;*                                                                  *
;*  PARAMETERS:                                                     *
;*      None.                                                       *
;*                                                                  *
;*  DESCRIPTION:                                                    *
;*      Reads the Vendor ID from the config register                *
;*                                                                  *
;*  RETURNS:                                                        *
;*      Vendor ID in r0.                                            *
;********************************************************************
    EXPORT GetVendorID
GetVendorID
    
    ;Read CONFIG0 register
    ldr     r1, =PLL_CONFIG0_REG
    ldr     r0, [r1]

    ;Shift and mask out unwanted bits
    mov     r0, r0, lsr #7
    and     r0, r0, #0xF
    
    ;Return to caller.
    bx      lr
    

;********************************************************************
;*  GetBoardID                                                      *
;*                                                                  *
;*  PARAMETERS:                                                     *
;*      None.                                                       *
;*                                                                  *
;*  DESCRIPTION:                                                    *
;*      Reads the Board ID from the config register based on the    *
;*      specifc vendor's (or Conexant internal) format              *
;*                                                                  *
;*  RETURNS:                                                        *
;*      Board ID in r0.                                             *
;********************************************************************
    EXPORT GetBoardID
GetBoardID
    
    ;Read CONFIG0 register
    ldr     r1, =PLL_CONFIG0_REG
    ldr     r0, [r1]

    ;Determine how to pluck the board ID based on vendor ID
    mov     r1, r0, lsr #7
    and     r1, r1, #0xF
    cmp     r1, #0x0
    beq     GetBoardIDCnxt
    cmp     r1, #0xE
    beq     GetBoardIDVendA
    cmp     r1, #0xD
    beq     GetBoardIDVendB
    cmp     r1, #0xB
    beq     GetBoardIDVendC
    cmp     r1, #0x7
    beq     GetBoardIDVendD
    mov     r0, #0
    b       GetBoardIDDone

GetBoardIDCnxt
    ;Read Board ID for an internal (Conexant) IRD
    and     r0, r0, #0x7F
    b       GetBoardIDDone

GetBoardIDVendA
    ;Read Board ID for Vendor A's configuration
    mov     r0, #0
    b       GetBoardIDDone

GetBoardIDVendB
    ;Read Board ID for Vendor B's configuration
    and     r0, r0, #0x7F
    b       GetBoardIDDone

GetBoardIDVendC
    ;Read Board ID for Vendor C's configuration
    mov     r0, #0
    b       GetBoardIDDone

GetBoardIDVendD
    ;Read Board ID for Vendor D's configuration
    and     r0, r0, #0x7F
;    mov     r0, #0
    b       GetBoardIDDone

GetBoardIDDone
    ;Return to caller.
    bx      lr
    
    END

