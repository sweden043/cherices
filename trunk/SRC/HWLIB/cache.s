;***************************************************************************
;                   CONEXANT PROPRIETARY AND CONFIDENTIAL                  
;                       SOFTWARE FILE/MODULE HEADER                       
;                   Conexant Systems Inc. (c) 1999-2003
;                               Austin, TX                              
;                          All Rights Reserved       
;***************************************************************************
;                                                                          
; Filename:       cache.s                                           
;                                                                          
; Description:    ARM V4 Cache Support                                      
;                                                                          
; Author:         ARM940T - Tim Ross
;                 ARM920T - Dave Moore                                               
;                                                                          
;***************************************************************************
;* $Header: cache.s, 21, 7/31/03 5:08:54 PM, Larry Wang$
;****************************************************************************

;******************/
;* Include Files  */
;******************/
    GET stbcfg.a

    GET cpu.a
    GET board.a
    GET startup.a

    IF :DEF: |ads$version|
    AREA    AV4CACHE, CODE, READONLY
    ELSE
    AREA    AV4CACHE, CODE, READONLY, INTERWORK
    ENDIF

    IF (:DEF:DEBUG):LAND:(:DEF:NON_VXWORKS)
	IMPORT  StacksAvail
	ENDIF

; 
;   FUNCTION: DrainWriteBuffer
; 
;   PURPOSE: Drains the CPU's write posting buffer
;
;   PARAMETERS: None
;
;   RETURNS: Nothing
;
;   CPUS: ARM940T, ARM920T 
;
;   REGS USED: r0
;
;   ASM FUNCTIONS CALLED: None
;
    EXPORT _DrainWriteBuffer
    EXPORT DrainWriteBuffer
_DrainWriteBuffer
DrainWriteBuffer

    IF MMU_CACHE_DISABLE = 0

        mov     r0, #0
        mcr     p15, 0, r0, c7, c10, 4 

    ENDIF    ; IF MMU_CACHE_DISABLE = 0

    bx   lr




;
;  FUNCTION: FlushICache
;                                                                  
;  DESCRIPTION:                                                    
;      Flushes (Invalidates) the instruction cache.                
;                                                                  
;  PARAMETERS:                                                     
;      None.                                                       
;                                                                  
;  RETURNS:                                                        
;      Nothing.                                                    
;                                                                  
;  CPUS: ARM940T, ARM920T                                          
;                                                                  
;  REGISTER USE:                                                   
;      r0                                                          
;
;  ASM FUNCTIONS CALLED: None
;
    EXPORT _FlushICache
    EXPORT FlushICache
_FlushICache
FlushICache

    IF MMU_CACHE_DISABLE = 0

       mov     r0, #0
       mcr     p15, 0, r0, c7, c5, 0

    ENDIF

    bx      lr


;
;  FUNCTION: DisableICache                                                   
;                                                                  
;  DESCRIPTION:                                                    
;      Disables Instruction caching.
;                                                                  
;  PARAMETERS:                                                     
;      None.                                                       
;                                                                  
;  RETURNS:                                                        
;      Nothing.                                                    
;                                                                  
;  CPUS: ARM940T, ARM920T                                          
;                                                                  
;  REGISTER USE:                                                   
;      r0                                                          
;
;  ASM FUNCTIONS CALLED: None
;
        EXPORT  _DisableICache
        EXPORT  DisableICache
_DisableICache
DisableICache

    IF MMU_CACHE_DISABLE = 0

       ;Get current CP 15 Reg 1 config.
       mrc     p15, 0, r0, c1, c0

       ;Clear out I cache enable bit
       orr     r0, r0, #CACHE_ICACHE_ENABLE ;0x1000
       eor     r0, r0, #CACHE_ICACHE_ENABLE

       ;Put back new config.
       mcr     p15, 0, r0, c1, c0

    ENDIF    ; IF MMU_CACHE_DISABLE = 0

    bx      lr


;
;  FUNCTION: EnableICache                                                    
;                                                                  
;  DESCRIPTION:                                                   
;      Enables instruction caching.
;                                                                  
;  PARAMETERS:                                                     
;      None.                                                       
;                                                                  
;  RETURNS:                                                        
;      Nothing.                                                    
;                                                                  
;  CPUS: ARM940T, ARM920T                                          
;                                                                  
;  REGISTER USE:                                                   
;      r0                                                          
;
;  ASM FUNCTIONS CALLED: None
;
        EXPORT  _EnableICache
        EXPORT  EnableICache
_EnableICache 
EnableICache 

    IF MMU_CACHE_DISABLE = 0

       ;Get current CP15 Reg 1 config.
       mrc     p15, 0, r0, c1, c0

       ;Set I cache enable bit
       orr     r0, r0, #CACHE_ICACHE_ENABLE ; 0x1000

       ;Put back new config.
       mcr     p15, 0, r0, c1, c0

    ENDIF    ; IF MMU_CACHE_DISABLE = 0

    bx      lr


;
;  FUNCTION: FlushDCache
;
;  DESCRIPTION:
;      Flushes (Invalidates) the data cache.
;
;  PARAMETERS: 
;      None.
;
;  RETURNS:
;      Nothing.
;
;  CPUS: ARM940T, ARM920T
;
;  REGISTER USE:
;      r0  
;
;  ASM FUNCTIONS CALLED: None
;
    EXPORT  _FlushDCache
    EXPORT  FlushDCache
_FlushDCache
FlushDCache

    IF MMU_CACHE_DISABLE = 0

       mov     r0, #0
       mcr     p15, 0, r0, c7, c6, 0

    ENDIF    ; IF MMU_CACHE_DISABLE = 0

    bx      lr


;
;  FUNCTION: CleanDCache
;
;  DESCRIPTION:
;      Cleans (writes to memory) the data cache contents.
;
;  PARAMETERS:
;      None.
;
;  RETURNS:
;      Nothing.
;
;  CPUS: ARM940T, ARM920T
;
;  REGISTER USE:
;      r0-r2
;
;  ASM FUNCTIONS CALLED: None
;
;
        EXPORT  _CleanDCache
        EXPORT  CleanDCache
_CleanDCache
CleanDCache
 
    IF MMU_CACHE_DISABLE = 0

    ;Zero r1 - row counter
    mov     r1, #0

    ;Zero r0 - segment counter
clean_next_cache_row
    mov     r0, #0

    ;Make segment and row address, then clean that row.

clean_next_cache_segment

    orr     r2, r1, r0
    mcr     p15, 0, r2, c7, c10, 2 

    ;Increment segment counter & check to see if done segments
    add     r0, r0, #0x10

    IF CPU_TYPE = AUTOSENSE
    ;Skip the following 940-specific block if not a 940
    mrc     p15, 0, r2, c0, c0, 0   ;get the ID register into r2
    tst     r2, #CPU_ARM940_BIT     ;test for a 940 CPU
    beq     %FA0                    ;beq means not a 940
    ENDIF

    IF (CPU_TYPE=CPU_ARM940T):LOR:(CPU_TYPE=AUTOSENSE)
    cmp     r0, #0x40  ; ARM 940 has 4 segments
    IF CPU_TYPE = AUTOSENSE
    b       %FA1
    ENDIF
0
    ENDIF

    IF CPU_TYPE = AUTOSENSE
    ;Skip the following 920-specific block if not a 920
    mrc     p15, 0, r2, c0, c0, 0   ;get the ID register into r2
    tst     r2, #CPU_ARM920_BIT     ;test for a 920 CPU
    beq     %FA1                    ;beq means not a 920
    ENDIF

    IF (CPU_TYPE=CPU_ARM920T):LOR:(CPU_TYPE=AUTOSENSE)
    cmp     r0, #0x100 ; ARM 920 has 8 segments
1
    ENDIF

    bne     clean_next_cache_segment

    ;Increment row counter and check to see if done rows

	; ARM940 has 4-word lines, ARM 920 has 8-word lines
	; BOTH have 64 lines in a segment (64way set assoc)
    add     r1, r1, #0x04000000 ; [31:26] are row index
    cmp     r1, #0x0
    bne     clean_next_cache_row

    ENDIF    ; IF MMU_CACHE_DISABLE = 0

    bx      lr



; 
;   FUNCTION: CleanDCacheMVA
; 
;   PURPOSE: Clean (write to memory) single dcache line using MVA
;
;   PARAMETERS: r0 = MVA
;
;   RETURNS: Nothing
;
;   CPUS: ARM920T 
;
;   REGS USED: r0
;
;   ASM FUNCTIONS CALLED: None
;

    IF (CPU_TYPE=CPU_ARM920T):LOR:(CPU_TYPE=AUTOSENSE)

    EXPORT _CleanDCacheMVA
    EXPORT CleanDCacheMVA
_CleanDCacheMVA
CleanDCacheMVA

    IF MMU_CACHE_DISABLE = 0

         mcr  p15, 0, r0, c7, c10, 1 

    ENDIF    ; IF MMU_CACHE_DISABLE = 0

    bx   lr

    ENDIF ; (CPU_TYPE=CPU_ARM920T):LOR:(CPU_TYPE=AUTOSENSE)


; 
;   FUNCTION: CleanAndInvalDCacheMVA
; 
;   PURPOSE: Clean and Invalidate single dcache line using MVA
;
;   PARAMETERS: r0 = MVA
;
;   RETURNS: Nothing
;
;   CPUS: ARM920T 
;
;   REGS USED: r0
;
;   ASM FUNCTIONS CALLED: None
;

    IF (CPU_TYPE=CPU_ARM920T):LOR:(CPU_TYPE=AUTOSENSE)

    EXPORT _CleanAndInvalDCacheMVA
    EXPORT CleanAndInvalDCacheMVA
_CleanAndInvalDCacheMVA
CleanAndInvalDCacheMVA

    IF MMU_CACHE_DISABLE = 0

          mcr  p15, 0, r0, c7, c14, 1 

    ENDIF    ; IF MMU_CACHE_DISABLE = 0

    bx   lr

    ENDIF ; (CPU_TYPE=CPU_ARM920T):LOR:(CPU_TYPE=AUTOSENSE)


;
;  FUNCTION: EnableDCache
;
;  DESCRIPTION:
;      Enables the data cache.
;
;  PARAMETERS:
;      None.
;
;  RETURNS:
;      Nothing.
;
;  CPUS: ARM940T, ARM920T
;
;  REGISTER USE:
;      r0
;
;   ASM FUNCTIONS CALLED: None
;
        EXPORT  _EnableDCache
        EXPORT  EnableDCache
_EnableDCache
EnableDCache

    IF MMU_CACHE_DISABLE = 0

       ;Get current config from CP 15 Reg 1
       mrc     p15, 0, r0, c1, c0

       ;Set D cache enable bit
       orr     r0, r0, #CACHE_DCACHE_ENABLE ; 0x4

       ;Put back new config.
       mcr     p15, 0, r0, c1, c0

    ENDIF    ; IF MMU_CACHE_DISABLE = 0

    bx      lr

;
;  FUNCTION:  EnableDCacheForRegion
;
;  DESCRIPTION: Enable data caching for passed in region#
;
;  PARAMETERS:
;          R0 - Protection Region (0-7)
;
;  CPUS: ARM940T
;
;  RETURNS: Nothing.
;
;  REGISTER USE:
;           R0-R1
;
;   ASM FUNCTIONS CALLED: None
;

    IF (CPU_TYPE=CPU_ARM940T):LOR:(CPU_TYPE=AUTOSENSE)

    IF (:DEF:NON_VXWORKS):LOR:(ARM_VERSION<>220)

    EXPORT  _EnableDCacheForRegion
    EXPORT  EnableDCacheForRegion
_EnableDCacheForRegion
EnableDCacheForRegion

       IF MMU_CACHE_DISABLE = 0

          cmp     r0, #8    ; check for valid Region 0-7
		  IF :DEF:DEBUG
1         bge     %BT1       ; lock it up!
          ELSE
          bge     edce      ; just exit
		  ENDIF

          mov     r1, #1          ; create mask for CP15 Reg 2
          mov     r1, r1, lsl r0  ; set bit in mask according to our input Region
          mrc     p15, 0, r0, CACHE_CACHEABLE_REG, c0, 0   ; get current value of CP15 R2
          orr     r0, r0, r1      ; OR in our new Region
          mcr     p15, 0, r0, CACHE_CACHEABLE_REG, c0, 0   ; store new updated mask to CP15 R2
edce

       ENDIF    ; IF MMU_CACHE_DISABLE = 0

    bx      lr

    ENDIF ; (:DEF:NON_VXWORKS):LOR:(ARM_VERSION<>220)
    ENDIF ; (CPU_TYPE=CPU_ARM940T):LOR:(CPU_TYPE=AUTOSENSE)


;
;  FUNCTION: DisableDCacheForRegion
;
;  DESCRIPTION: Disable data caching for passed in region#
;
;  PARAMETERS:
;            R0 - Protection Region (0-7) to Modify
;
;  RETURNS:
;            Region - Memory Region To Disable Data Cache       
;
;  CPUS: ARM940T
;
;  REGISTER USE: r0-r3
;
;  ASM FUNCTIONS CALLED: 
;             CleanDCache (uses r0-r2)
; 	          DrainWriteBuffer (uses r0)
;             FlushDCache (uses r0)
;

    IF (CPU_TYPE=CPU_ARM940T):LOR:(CPU_TYPE=AUTOSENSE)

    IF (:DEF:NON_VXWORKS):LOR:(ARM_VERSION<>220)
        EXPORT  _DisableDCacheForRegion
        EXPORT  DisableDCacheForRegion
_DisableDCacheForRegion
DisableDCacheForRegion

    IF MMU_CACHE_DISABLE = 0

       IF (:DEF:DEBUG):LAND:(:DEF:NON_VXWORKS)
           ; Check that stacks are available to code
           ldr     r1, =StacksAvail
           ldr     r1, [r1]
		   cmp     r1, #0
1		   beq     %BT1    ; lock up
       ENDIF

       stmfd   sp!, {lr}

       cmp     r0, #8    ; check for valid Region 0-7
       IF :DEF:DEBUG
2      bge     %BT2       ; lock it up!
       ELSE
       bge     ddce      ; just exit
       ENDIF

       mov     r1, #1                ; form mask
       mov     r1, r1, lsl r0        ; convert our input Region to CP15 Reg 2 format
       mrc     p15, 0, r0, c2, c0, 0 ; Read CP15 Reg 2

       stmfd   sp!, {r0, r1}         ; save input Region and current CP15 R2

       mrs     r2, cpsr              ; get current Interrupt state
       stmfd   sp!, {r2}             ; and save it

       bic     r0, r0, r1            ; turn off read data cachability for our input Region
       stmfd   sp!, {r0}             ; save this mask, there is some housekeeping to be done first..

       orr     r2, r2, #CPU_IRQ_DIS  ; mask off FIQ and IRQ
       orr     r2, r2, #CPU_FIQ_DIS
       msr     cpsr_c, r2            ; set state to interrupts OFF

       ; Flush out existing DCache contents under previous Cache Policy
       bl      CleanDCache
       bl      DrainWriteBuffer
       bl      FlushDCache

       ldmia   sp!, {r0}             ; get back our newly formed mask and update CP15 R2
       mcr     p15, 0, r0, c2, c0, 0 ; Write CP15 Reg 2

       ;Reenable interrupts

       ldmia   sp!, {r2}             ; pop the Interrupt state we had upon entry
       msr     cpsr_c, r2            ; and put it back into place

       ldmia   sp!, {r0,r1}          ; get original input R0 and original CP15 R2
       and     r0, r0, r1            ; OR them together to form the return value, i.e.
	                                 ; non-zero after OR means DCaching for this Region
									 ; was ON upong entry, zero means it was off..

ddce

       ldmia   sp!, {lr}

    ELSE    ; IF MMU_CACHE_DISABLE = 0
	   ; set return mask to 0 to indicate DCaching for the input Region was off on entry..
	   mov     r0,  #0
    ENDIF   ; IF MMU_CACHE_DISABLE = 0 

    bx      lr

    ENDIF ; (:DEF:NON_VXWORKS):LOR:(ARM_VERSION<>220)
    ENDIF ; (CPU_TYPE=CPU_ARM940T):LOR:(CPU_TYPE=AUTOSENSE)


;
; FUNCTION: EnableICacheForRegion
;
; DESCRIPTION:
;
; PARAMETERS:
;      R0 - Protection Region (0-7)
;
; RETURNS:
;      Nothing.
;
; CPUS: ARM940T
;
; REGISTER USE: r0-r1 
;
; ASM FUNCTIONS CALLED: None
;

    IF (CPU_TYPE=CPU_ARM940T):LOR:(CPU_TYPE=AUTOSENSE)

        EXPORT  _EnableICacheForRegion
        EXPORT  EnableICacheForRegion
_EnableICacheForRegion
EnableICacheForRegion

    IF MMU_CACHE_DISABLE = 0

     cmp     r0, #8    ; check for valid Region 0-7
     IF :DEF:DEBUG
2    bge     %BT2       ; lock it up!
     ELSE
     bge     eice      ; just exit
	 ENDIF

     mov     r1, #1                                   ; create mask
     mov     r1, r1, lsl r0                           ; shift input Region left to form required bit mask
     mrc     p15, 0, r0, CACHE_CACHEABLE_REG, c0, 1   ; load current CP15 Reg 2 contents
     orr     r0, r0, r1                               ; or in our new Region                           
     mcr     p15, 0, r0, CACHE_CACHEABLE_REG, c0, 1   ; and put it back

eice

    ENDIF ; MMU_CACHE_DISABLE = 0 

    bx      lr

    ENDIF ; (CPU_TYPE=CPU_ARM940T):LOR:(CPU_TYPE=AUTOSENSE)


;
; FUNCTION: DisableICacheForRegion
;
; DESCRIPTION: 
;
; PARAMETERS:
;      R0 - Protection Region (0-7)
;
; RETURNS:
;      R0 - Non-zero indicates caching was enabled for the
;           input region on entry. Zero means it was already off.
;
; REGISTER USE: r0-r1
;
; ASM FUNCTIONS CALLED: FlushICache (uses r0)
;

    IF (CPU_TYPE=CPU_ARM940T):LOR:(CPU_TYPE=AUTOSENSE)

        EXPORT  _DisableICacheForRegion
        EXPORT  DisableICacheForRegion
_DisableICacheForRegion
DisableICacheForRegion

    IF MMU_CACHE_DISABLE = 0

       IF (:DEF:DEBUG):LAND:(:DEF:NON_VXWORKS)
           ; Check that stacks are available to code
           ldr     r1, =StacksAvail
           ldr     r1, [r1]
		   cmp     r1, #0
1		   beq     %BT1    ; lock up
       ENDIF

       stmfd   sp!, {lr}        ; we will be calling out

       ; Test for input range

       cmp     r0, #8           ; Regions range from 0-7
       IF :DEF:DEBUG
2      bge     %BT2              ; lock up on bad region
       ELSE
	   bge     dice             ; else just exit
	   ENDIF
 
       ; Form mask for CP15 Reg 2 Instruction Cachable Register

       mov     r1, #1           ; create mask
       mov     r1, r1, lsl r0   ; shift bit to indicate the input Region 
       mrc     p15, 0, r0, CACHE_CACHEABLE_REG, c0, 1  ; load CP15 Reg 2 into r0
       stmfd   sp!, {r0}        ; save value of CP15 Reg 2 as it was upon entry
       bic     r0, r0, r1       ; turn off (clear) the bit representing the input Region
       ; turn off Region ICaching by updating CP15 Reg 2
       mcr     p15, 0, r0, CACHE_CACHEABLE_REG, c0, 1

       bl      FlushICache      ; Invalidate the ICache (uses r0)

       ldmia   sp!, {r0}        ; reload CP15 Reg 2 value as it was upon entry
       and     r0, r0, r1       ; AND with the bit representing the input region 
	                            ;  which forms our return value, i.e. if r0 becomes non-zero
								;  then ICache was ON for this region upon entry, else it was off..
dice
       ldmia   sp!, {lr}

    ELSE    ; IF MMU_CACHE_DISABLE = 0
	   ; set return mask to 0 to indicate ICaching for the input Region was off on entry..
	   mov     r0,  #0
    ENDIF   ; IF MMU_CACHE_DISABLE = 0

    bx      lr

    ENDIF ; (CPU_TYPE=CPU_ARM940T):LOR:(CPU_TYPE=AUTOSENSE)


;********************************************************************
;*  SetMemoryRegion                                                 *
;*                                                                  *
;*  DESCRIPTION:                                                    *
;*      Sets a single memory region                                 *
;*                                                                  *
;*  PARAMETERS:                                                     *
;*      r0 - Which memory region to setup                           *
;*      r1 - Base address                                           *
;*      r2 - Size                                                   *
;*                                                                  *
;*  RETURNS:                                                        *
;*      Nothing.                                                    *
;*                                                                  *
;*  CPUS: ARM940T                                                   *
;*                                                                  *
;*  REGISTER USE:                                                   *
;*      r0-r3                                                       *
;*                                                                  *
;*  ASM FUNCTIONS CALLED: None                                      *
;*                                                                  *
;********************************************************************

    IF (CPU_TYPE=CPU_ARM940T):LOR:(CPU_TYPE=AUTOSENSE)

        EXPORT SetMemoryRegion
SetMemoryRegion

    IF MMU_CACHE_DISABLE = 0

    ; Ensure we have a valid (properly aligned) base address
    ; Ensure we have a valid size (must be multiple of alignment)
    tst     r1, #0x0FF   ; Minimum 4KB alignment
    tsteq   r1, #0xF00   ; Minimum 4KB alignment
    subeq   r3, r2, #1
    tsteq   r1, r3
0   bne     %BT0         ; lock it up!

    ; Get the size value along with making sure the size is
    ; a power of 2 (i.e. only 1 bit set)
    mov     r3, #0
1   mov     r2, r2, lsr #1
    cmp     r2, #0x1
    beq     %FT2
    add     r3, r3, #1
    cmp     r3, #32
    blt     %BT1
    bne     %BT0         ; lock it up!

    ; We now have the encoded size.  Or together the value to be
    ; written into the memory protection region 
2   mov     r3, r3, lsl #1
    orr     r1, r1, r3
    orr     r1, r1, #CACHE_REGION_ENABLE

    ; Find the correct memory protection region and set the value
    cmp     r0, #0
    mcreq   p15, 0, r1, c6, CACHE_REGION0, 0
    mcreq   p15, 0, r1, c6, CACHE_REGION0, 1

    cmp     r0, #1
    mcreq   p15, 0, r1, c6, CACHE_REGION1, 0
    mcreq   p15, 0, r1, c6, CACHE_REGION1, 1

    cmp     r0, #2
    mcreq   p15, 0, r1, c6, CACHE_REGION2, 0
    mcreq   p15, 0, r1, c6, CACHE_REGION2, 1

    cmp     r0, #3
    mcreq   p15, 0, r1, c6, CACHE_REGION3, 0
    mcreq   p15, 0, r1, c6, CACHE_REGION3, 1

    cmp     r0, #4
    mcreq   p15, 0, r1, c6, CACHE_REGION4, 0
    mcreq   p15, 0, r1, c6, CACHE_REGION4, 1

    cmp     r0, #5
    mcreq   p15, 0, r1, c6, CACHE_REGION5, 0
    mcreq   p15, 0, r1, c6, CACHE_REGION5, 1

    cmp     r0, #6
    mcreq   p15, 0, r1, c6, CACHE_REGION6, 0
    mcreq   p15, 0, r1, c6, CACHE_REGION6, 1

    cmp     r0, #7
    mcreq   p15, 0, r1, c6, CACHE_REGION7, 0
    mcreq   p15, 0, r1, c6, CACHE_REGION7, 1

    ENDIF    ; IF MMU_CACHE_DISABLE = 0

    bx      lr

    ENDIF ; (CPU_TYPE=CPU_ARM940T):LOR:(CPU_TYPE=AUTOSENSE)


;********************************************************************
;*  BspMmuTransOff                                                  *
;*                                                                  *
;*  DESCRIPTION:                                                    *
;*      BspMmUTransOn/Off are used by pROBE+ to turn on/off memory  *
;*      protection. Since we do not make use of the memory          *
;*      protection capabilities of the ARM940T, we do not need to   *
;*      use it here.                                                *
;*                                                                  *
;*  PARAMETERS:                                                     *
;*      None.                                                       *
;*                                                                  *
;*  RETURNS:                                                        *
;*      Nothing.                                                    *
;*                                                                  *
;*  CPUS: ARM940T, ARM920T                                          *
;*                                                                  *
;*  REGISTER USE:                                                   *
;*      None.                                                       *
;********************************************************************
    EXPORT BspMmuTransOff
BspMmuTransOff
    bx     lr


    EXPORT BspMmuTransOn
BspMmuTransOn
    bx     lr


;********************************************************************
;*  FlushAndInvalDCacheRegion                                       *
;*                                                                  *
;*  DESCRIPTION:                                                    *
;*      Clean and Invalidate a D Cache region.  Useful for          *
;*      DMA (GXA) operations that utilize cached memory along with  *
;*      the ARM core using the same buffer(s).                      *
;*                                                                  *
;*  TYPICAL USAGE:                                                  *
;*      Create data in a memory buffer from the ARM core then       *
;*      attempt to DMA the buffer from hardware.  Prior to starting *
;*      the DMA operation,  this function should be called to force *
;*      the cache lines to be flushed to memory to ensure the DMA   *
;*      doesn't use stale data in memory instead of the new         *
;*      data which is likely in the data cache of the ARM core.     *
;*                                                                  *
;*  PARAMETERS:  r0 = Memory Address                                *
;*               r1 = Size (in bytes)                               *
;*                                                                  *
;*  RETURNS:                                                        *
;*      Nothing.                                                    *
;*                                                                  *
;*  CPUS:        ARM920T                                            *
;*                                                                  *
;*  REGISTER USE:                                                   *
;*      r0, r1, r2                                                  *
;********************************************************************
    EXPORT FlushAndInvalDCacheRegion
    EXPORT _FlushAndInvalDCacheRegion
FlushAndInvalDCacheRegion
_FlushAndInvalDCacheRegion

    IF MMU_CACHE_DISABLE = 0

       IF CPU_TYPE = AUTOSENSE
          mrc     p15, 0, r2, c0, c0, 0   ;get the ID register into r2
          tst     r2, #CPU_ARM920_BIT     ;test for a 920 CPU
          beq     %FT1                    ;beq means not a 920, lock-up
       ENDIF

       IF CPU_TYPE = CPU_ARM920T :LOR: CPU_TYPE = AUTOSENSE
          ; Align and handle extra bytes
          ands    r2, r0, #0x1f
          rsbne   r2, r2, #0x20
          addne   r1, r1, r2
          bic     r0, r0, #0x1f
0         ; Loop clean/inv per line, 32bytes at a time
          mcr     p15, 0, r0, c7, c14, 1
          add     r0, r0, #0x20
          subs    r1, r1, #0x20
          bpl     %BT0
          b       %FT4
       ENDIF

       IF CPU_TYPE = CPU_ARM940T :LOR: CPU_TYPE = AUTOSENSE
          ; On a 940, must clean and flush entire D Cache (i.e. slow)
1         mov     r1, #0                  ;Zero r1 - row counter
2         mov     r0, #0                  ;Zero r0 - segment counter
3         orr     r2, r1, r0
          mcr     p15, 0, r2, c7, c10, 2
          ;Increment segment counter & check to see if done segments
          add     r0, r0, #0x10
          cmp     r0, #0x40               ;ARM 940 has 4 segments
          bne     %BT3
          ;Increment row counter and check to see if done rows
          adds    r1, r1, #0x04000000     ;[31:26] are row index
          bne     %BT2
          ;Flush (Invalidate) the entire DCache
          mov     r0, #0
          mcr     p15, 0, r0, c7, c6, 0
       ENDIF

          ;Drain write buffer
4         mov     r0, #0
          mcr     p15, 0, r0, c7, c10, 4

    ENDIF    ; IF MMU_CACHE_DISABLE = 0

          bx      lr

;********************************************************************
;*  InvalDCacheRegion                                               *
;*                                                                  *
;*  DESCRIPTION:                                                    *
;*      Invalidate a D Cache region.  Useful for                    *
;*      DMA (GXA) operations that utilize cached memory along with  *
;*      the ARM core using the same buffer(s).                      *
;*                                                                  *
;*  TYPICAL USAGE:                                                  *
;*      Hardware DMA into a memory buffer, then attempt to access   *
;*      the memory buffer from the ARM core.  Prior to accessing    *
;*      the memory buffer, this function should be called to force  *
;*      the cache lines to be invalidated to ensure the ARM core    *
;*      doesn't use stale data in the cache instead of the new      *
;*      data in the memory buffer.                                  *
;*                                                                  *
;*  PARAMETERS:  r0 = Memory Address                                *
;*               r1 = Size (in bytes)                               *
;*                                                                  *
;*  RETURNS:                                                        *
;*      Nothing.                                                    *
;*                                                                  *
;*  CPUS:        ARM920T                                            *
;*                                                                  *
;*  REGISTER USE:                                                   *
;*      r0, r1, r2                                                  *
;********************************************************************
    EXPORT InvalDCacheRegion
    EXPORT _InvalDCacheRegion
InvalDCacheRegion
_InvalDCacheRegion

    IF MMU_CACHE_DISABLE = 0

       IF CPU_TYPE = AUTOSENSE
          mrc     p15, 0, r2, c0, c0, 0   ;get the ID register into r2
          tst     r2, #CPU_ARM920_BIT     ;test for a 920 CPU
          beq     %FT1                    ;beq means not a 920, lock-up
       ENDIF

       IF CPU_TYPE = CPU_ARM920T :LOR: CPU_TYPE = AUTOSENSE
          ; Align and handle extra bytes
          ands    r2, r0, #0x1f
          rsbne   r2, r2, #0x20
          addne   r1, r1, r2
          bic     r0, r0, #0x1f
0         ; Loop inv per line, 32bytes at a time
          mcr     p15, 0, r0, c7, c6, 1
          add     r0, r0, #0x20
          subs    r1, r1, #0x20
          bpl     %BT0
          b       %FT2
       ENDIF

       IF CPU_TYPE = CPU_ARM940T :LOR: CPU_TYPE = AUTOSENSE
          ; This doesn't make sense on a 940.  Hang...
1         b       %BT1
       ENDIF

    ENDIF    ; IF MMU_CACHE_DISABLE = 0

2         bx      lr

;********************************************************************
;*  GetCP15Reg                                                      *
;*                                                                  *
;*  DESCRIPTION:                                                    *
;*      Read a register on CP15                                     *
;*                                                                  *
;*  PARAMETERS:  R0 - CP15 register number                          *
;*               R1 - second op code                                *
;*               R2 - additional parameter for register 6           *
;*                                                                  *
;*  RETURNS:                                                        *
;*      Value in the register                                       *
;*                                                                  *
;*  CPUS:        ARM940T ARM920T                                    *
;*                                                                  *
;*  REGISTER USE:                                                   *
;*      r0, r1, r2                                                  *
;********************************************************************
    EXPORT GetCP15Reg
    EXPORT _GetCP15Reg
GetCP15Reg
_GetCP15Reg
   mov      ip, lr ; we will be calling out
   mov      r3, r0

   cmp      r3, #0
   bne      gcr1
   cmp      r1, #0
   bne      gcr0_1
   mrc      p15, 0, r0, c0, c0, 0
   b        gcr_done
gcr0_1
   mrc      p15, 0, r0, c0, c0, 1
   b        gcr_done

gcr1
   cmp      r3, #1
   bne      gcr2
   mrc      p15, 0, r0, c1, c0, 0
   b        gcr_done

gcr2
   cmp      r3, #2
   bne      gcr3
   cmp      r1, #0
   bne      gcr2_1
   mrc      p15, 0, r0, c2, c0, 0
   b        gcr_done
gcr2_1
   mrc      p15, 0, r0, c2, c0, 1
   b        gcr_done

gcr3
   cmp      r3, #3
   bne      gcr5
   mrc      p15, 0, r0, c3, c0, 0
   b        gcr_done

gcr5
   cmp      r3, #5
   bne      gcr6
   cmp      r1, #0
   bne      gcr5_1
   mrc      p15, 0, r0, c5, c0, 0
   b        gcr_done
gcr5_1
   mrc      p15, 0, r0, c5, c0, 1
   b        gcr_done

gcr6
   cmp      r3, #6
   bne      gcr9
   cmp      r1, #0
   bne      gcr6_1_0
   cmp      r2, #0
   bne      gcr6_0_1
   mrc      p15, 0, r0, c6, c0, 0
   b        gcr_done
gcr6_0_1
   cmp      r2, #1
   bne      gcr6_0_2
   mrc      p15, 0, r0, c6, c1, 0
   b        gcr_done
gcr6_0_2
   cmp      r2, #2
   bne      gcr6_0_3
   mrc      p15, 0, r0, c6, c2, 0
   b        gcr_done
gcr6_0_3
   cmp      r2, #3
   bne      gcr6_0_4
   mrc      p15, 0, r0, c6, c3, 0
   b        gcr_done
gcr6_0_4
   cmp      r2, #4
   bne      gcr6_0_5
   mrc      p15, 0, r0, c6, c4, 0
   b        gcr_done
gcr6_0_5
   cmp      r2, #5
   bne      gcr6_0_6
   mrc      p15, 0, r0, c6, c5, 0
   b        gcr_done
gcr6_0_6
   cmp      r2, #6
   bne      gcr6_0_7
   mrc      p15, 0, r0, c6, c6, 0
   b        gcr_done
gcr6_0_7
   cmp      r2, #7
   bne      gcr_err
   mrc      p15, 0, r0, c6, c7, 0
   b        gcr_done
gcr6_1_0
   cmp      r2, #0
   bne      gcr6_1_1
   mrc      p15, 0, r0, c6, c0, 1
   b        gcr_done
gcr6_1_1
   cmp      r2, #1
   bne      gcr6_1_2
   mrc      p15, 0, r0, c6, c1, 1
   b        gcr_done
gcr6_1_2
   cmp      r2, #2
   bne      gcr6_1_3
   mrc      p15, 0, r0, c6, c2, 1
   b        gcr_done
gcr6_1_3
   cmp      r2, #3
   bne      gcr6_1_4
   mrc      p15, 0, r0, c6, c3, 1
   b        gcr_done
gcr6_1_4
   cmp      r2, #4
   bne      gcr6_1_5
   mrc      p15, 0, r0, c6, c4, 1
   b        gcr_done
gcr6_1_5
   cmp      r2, #5
   bne      gcr6_1_6
   mrc      p15, 0, r0, c6, c5, 1
   b        gcr_done
gcr6_1_6
   cmp      r2, #6
   bne      gcr6_1_7
   mrc      p15, 0, r0, c6, c6, 1
   b        gcr_done
gcr6_1_7
   cmp      r2, #7
   bne      gcr_err
   mrc      p15, 0, r0, c6, c7, 1
   b        gcr_done

gcr9
   cmp      r3, #9
   bne      gcr10
   cmp      r1, #0
   bne      gcr9_1
   mrc      p15, 0, r0, c9, c0, 0
   b        gcr_done
gcr9_1
   mrc      p15, 0, r0, c9, c0, 1
   b        gcr_done

gcr10
   cmp      r3, #10
   bne      gcr13
   cmp      r1, #0
   bne      gcr10_1
   mrc      p15, 0, r0, c10, c0, 0
   b        gcr_done
gcr10_1
   mrc      p15, 0, r0, c10, c0, 1
   b        gcr_done

gcr13
   cmp      r3, #13
   bne      gcr_err
   mrc      p15, 0, r0, c13, c0, 0
   b        gcr_done

gcr_err
   mov      r0, #0xffffffff

gcr_done
   bx       ip

    END

;****************************************************************************
;* $Log: 
;*  21   mpeg      1.20        7/31/03 5:08:54 PM     Larry Wang      SCR(s) 
;*        7091 :
;*        Make both ARM and GNU assembler can identify if RTOS is VXWORKS or 
;*        not.
;*        
;*  20   mpeg      1.19        7/31/03 12:54:58 PM    Larry Wang      SCR(s) 
;*        7091 :
;*        Don't define EnableDCacheForRegion() and DisableDCacheForRegion() if 
;*        ARM_VERSION=220 && RTOS=VXWORKS; Don't refer to StacksAvail if 
;*        RTOS=VXWORKS.
;*        
;*  19   mpeg      1.18        7/30/03 3:26:10 PM     Larry Wang      SCR(s) 
;*        7076 :
;*        Add function GetCP15Reg.
;*        
;*  18   mpeg      1.17        7/21/03 11:32:36 AM    Larry Wang      SCR(s) 
;*        7004 :
;*        Make local label reference more clear so that it can be translated 
;*        into GNU style.  Note that default local label searching directions 
;*        are opposite in GNU and ARM tools.
;*        
;*  17   mpeg      1.16        6/20/03 3:47:38 PM     Tim White       SCR(s) 
;*        6815 :
;*        Add region invalidate support useful for sharing memory between 
;*        ARM920T
;*        code and a DMA device (e.g. GXA).
;*        
;*        
;*  16   mpeg      1.15        6/5/03 5:39:24 PM      Tim White       SCR(s) 
;*        6660 :
;*        Fixed typo in DEBUG case in EnableDCacheForRegion().
;*        
;*        
;*  15   mpeg      1.14        6/5/03 5:26:14 PM      Tim White       SCR(s) 
;*        6660 :
;*        Flash banks separately controlled by the 920 MMU using hardware 
;*        virtual pagemapping.
;*        
;*        
;*  14   mpeg      1.13        5/27/03 12:00:06 PM    Tim White       SCR(s) 
;*        6587 :
;*        Add region flush/invalidate support useful for sharing memory between
;*         ARM920T
;*        code and a DMA device (e.g. GXA).
;*        
;*        
;*  13   mpeg      1.12        5/19/03 3:52:00 PM     Billy Jackman   SCR(s) 
;*        6418 6419 :
;*        Modified CPU type checking for the autosense case to not depend upon 
;*        stored
;*        values.
;*        
;*  12   mpeg      1.11        4/30/03 4:45:24 PM     Billy Jackman   SCR(s) 
;*        6113 :
;*        Added PVCS Header and Log comments.
;*        Modified conditional assembly directives and code to add the 
;*        capability to
;*        detect the CPU type (920 vs. 940) at runtime when CPU_TYPE=AUTOSENSE.
;*        
;*  11   mpeg      1.10        1/31/03 5:36:58 PM     Dave Moore      SCR(s) 
;*        5375 :
;*        Moved the 940 Region routines from startup\su_cache.s here.
;*        Removed the use label from su_cache.s. Added more debugging
;*        checks throughout. 
;*        
;*        
;*  10   mpeg      1.9         12/20/02 9:56:06 PM    Tim Ross        SCR(s) 
;*        5206 :
;*        Added DrainWriteBuffer function & added exports of _Xxxxxx for each 
;*        function.
;*        
;*  9    mpeg      1.8         12/17/02 4:02:38 PM    Tim White       SCR(s) 
;*        5182 :
;*        Remove ARM_PIT_TYPE, no longer used.
;*        
;*        
;*  8    mpeg      1.7         4/30/02 2:46:44 PM     Billy Jackman   SCR(s) 
;*        3657 :
;*        Changed GET hwconfig.a to GET stbcfg.a to conform to new 
;*        configuration.
;*        
;*  7    mpeg      1.6         4/1/02 8:19:56 AM      Dave Moore      SCR(s) 
;*        3457 :
;*        Added 920T support
;*        
;*        
;*  6    mpeg      1.5         2/12/02 11:10:20 AM    Bobby Bradford  SCR(s) 
;*        3176 :
;*        Changed "IF :LNOT:(:DEF:MMU_CACHE_DISABLE)" to "IF MMU_CACHE_DISABLE 
;*        = 0"
;*        
;*  5    mpeg      1.4         1/9/02 4:25:52 PM      Dave Moore      SCR(s) 
;*        3006 :
;*        Fixed bug with IF MMU_CACHE_DISABLE logic
;*        
;*        
;*  4    mpeg      1.3         11/26/01 7:04:30 PM    Dave Moore      SCR(s) 
;*        2924 :
;*        
;*        
;*  3    mpeg      1.2         11/1/01 2:19:12 PM     Bobby Bradford  SCR(s) 
;*        2828 :
;*        Replaced GET sabine.a with GET hwconfig.a
;*        
;*        
;*  2    mpeg      1.1         8/22/01 5:38:24 PM     Miles Bintz     SCR(s) 
;*        2526 :
;*        Moved functions that were previously in \startup\cache.s to 
;*        hwlib\cache.s.
;*        Functions contained in hwlib\cache.s are all supposed to be 
;*        publicized & documented.  The four functions that were moved over are
;*         not documented but could be useful for internal standalone programs.
;*          Most importantly (the motivating factor in this change), this 
;*        change helps resolve an interdependency between startup & hwlib.
;*        
;*        
;*  1    mpeg      1.0         4/13/00 2:40:36 PM     Tim Ross        
;* $
;  
;     Rev 1.20   31 Jul 2003 16:08:54   wangl2
;  SCR(s) 7091 :
;  Make both ARM and GNU assembler can identify if RTOS is VXWORKS or not.
;  
;     Rev 1.19   31 Jul 2003 11:54:58   wangl2
;  SCR(s) 7091 :
;  Don't define EnableDCacheForRegion() and DisableDCacheForRegion() if ARM_VERSION=220 && RTOS=VXWORKS; Don't refer to StacksAvail if RTOS=VXWORKS.
;  
;     Rev 1.18   30 Jul 2003 14:26:10   wangl2
;  SCR(s) 7076 :
;  Add function GetCP15Reg.
;  
;     Rev 1.17   21 Jul 2003 10:32:36   wangl2
;  SCR(s) 7004 :
;  Make local label reference more clear so that it can be translated into GNU style.  Note that default local label searching directions are opposite in GNU and ARM tools.
;  
;     Rev 1.16   20 Jun 2003 14:47:38   whiteth
;  SCR(s) 6815 :
;  Add region invalidate support useful for sharing memory between ARM920T
;  code and a DMA device (e.g. GXA).
;  
;  
;     Rev 1.15   05 Jun 2003 16:39:24   whiteth
;  SCR(s) 6660 :
;  Fixed typo in DEBUG case in EnableDCacheForRegion().
;  
;  
;     Rev 1.14   05 Jun 2003 16:26:14   whiteth
;  SCR(s) 6660 :
;  Flash banks separately controlled by the 920 MMU using hardware virtual pagemapping.
;  
;  
;     Rev 1.13   27 May 2003 11:00:06   whiteth
;  SCR(s) 6587 :
;  Add region flush/invalidate support useful for sharing memory between ARM920T
;  code and a DMA device (e.g. GXA).
;  
;  
;     Rev 1.12   19 May 2003 14:52:00   jackmaw
;  SCR(s) 6418 6419 :
;  Modified CPU type checking for the autosense case to not depend upon stored
;  values.
;  
;     Rev 1.11   30 Apr 2003 15:45:24   jackmaw
;  SCR(s) 6113 :
;  Added PVCS Header and Log comments.
;  Modified conditional assembly directives and code to add the capability to
;  detect the CPU type (920 vs. 940) at runtime when CPU_TYPE=AUTOSENSE.
;* 
;*    Rev 1.10   31 Jan 2003 17:36:58   mooreda
;* SCR(s) 5375 :
;* Moved the 940 Region routines from startup\su_cache.s here.
;* Removed the use label from su_cache.s. Added more debugging
;* checks throughout.
;* 
;*    Rev 1.9   20 Dec 2002 21:56:06   rossst
;* SCR(s) 5206 :
;* Added DrainWriteBuffer function & added exports of _Xxxxxx for each function.
;* 
;*    Rev 1.8   17 Dec 2002 16:02:38   whiteth
;* SCR(s) 5182 :
;* Remove ARM_PIT_TYPE, no longer used.
;* 
;*    Rev 1.7   30 Apr 2002 13:46:44   jackmaw
;* SCR(s) 3657 :
;* Changed GET hwconfig.a to GET stbcfg.a to conform to new configuration.
;* 
;*    Rev 1.6   01 Apr 2002 08:19:56   mooreda
;* SCR(s) 3457 :
;* Added 920T support
;* 
;*    Rev 1.5   12 Feb 2002 11:10:20   bradforw
;* SCR(s) 3176 :
;* Changed "IF :LNOT:(:DEF:MMU_CACHE_DISABLE)" to "IF MMU_CACHE_DISABLE = 0"
;* 
;*    Rev 1.4   09 Jan 2002 16:25:52   mooreda
;* SCR(s) 3006 :
;* Fixed bug with IF MMU_CACHE_DISABLE logic
;* 
;*    Rev 1.3   26 Nov 2001 19:04:30   mooreda
;* SCR(s) 2924 :
;* 
;*    Rev 1.2   01 Nov 2001 14:19:12   bradforw
;* SCR(s) 2828 :
;* Replaced GET sabine.a with GET hwconfig.a
;* 
;*    Rev 1.1   22 Aug 2001 16:38:24   bintzmf
;* SCR(s) 2526 :
;* Moved functions that were previously in \startup\cache.s to hwlib\cache.s.
;* Functions contained in hwlib\cache.s are all supposed to be publicized & documen
;* ted.  The four functions that were moved over are not documented but could be us
;* eful for internal standalone programs.  Most importantly (the motivating factor
;* in this change), this change helps resolve an interdependency between startup &
;* hwlib.
;* 
;*    Rev 1.0   13 Apr 2000 13:40:36   rossst
;* Initial revision.
;* 
;****************************************************************************

