;/****************************************************************************/
;/*                   CONEXANT PROPRIETARY AND CONFIDENTIAL                  */
;/*                       SOFTWARE FILE/MODULE HEADER                        */
;/*                    Conexant Systems Inc. (c) 1999-2003                   */
;/*                               Austin, TX                                 */
;/*                            All Rights Reserved                           */
;/****************************************************************************/
;/*
; * Filename:       startup.s
; *
; *
; * Description:    As the entry point of the loaded code image, this code
; *                 is called by the code loader (i.e. boot block code)
; *                 after it is done loading the necessary code into SDRAM.
; *
; *                 Startup tasks which must be performed in assembly should
; *                 be done here. However, only operations that absolutely
; *                 must be done before C code can be called should be done
; *                 here so that as much code as possible can be written in
; *                 in C.
; *
; *
; * Author:         Tim Ross
; *
; ****************************************************************************/
;/* $Id: startup.s,v 1.57, 2004-06-22 20:02:35Z, Miles Bintz$
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
   IF :DEF: |ads$version|
    IMPORT __use_no_semihosting_swi
   ENDIF

   IF (DOWNLOAD_SERIAL_SUPPORT = NO)
    IMPORT InitCheckpointOutput
    IMPORT Checkpoint
    IMPORT mmuGetState
    IMPORT mmuSetState
    IMPORT CStartup
   ENDIF

   IF (IMAGE_SELFCOPY = YES)
    IMPORT FCopy
   ENDIF

    IMPORT ClearI2CBus
   IF (I2C_CONFIG_EEPROM_ADDR <> NOT_PRESENT) :LAND: \
      (PLL_PIN_ALT_FUNC_REG_DEFAULT_MILANO1 <> NOT_DETERMINED) :LAND: \
      (PLL_PIN_ALT_FUNC_REG_DEFAULT <> NOT_DETERMINED)
    IMPORT IsMilanoRev1
   ENDIF
   IF (I2C_CONFIG_EEPROM_ADDR <> NOT_PRESENT)
    IMPORT IsCfgEepromValid
    IMPORT StartupFatalExit
   ENDIF

    IMPORT  |Image$$FLASH$$Base|[WEAK]
    IMPORT  |Load$$RAM$$Base|[WEAK]
    IMPORT  |Image$$RAM$$Base|[WEAK]
    IMPORT  |Image$$RAM$$Limit|[WEAK]
    IMPORT  |Image$$RAM$$ZI$$Base|[WEAK]
    IMPORT  |Image$$RAM$$ZI$$Limit|[WEAK]
    IMPORT  |Image$$RO$$Base|[WEAK]
    IMPORT  |Image$$RO$$Limit|[WEAK]
    IMPORT  |Image$$RW$$Base|[WEAK]
    IMPORT  |Image$$RW$$Limit|[WEAK]
    IMPORT  |Image$$ZI$$Base|[WEAK]
    IMPORT  |Image$$ZI$$Limit|[WEAK]

   IF (DOWNLOAD_SERIAL_SUPPORT = NO)           
    IF :DEF:DEBUG
      EXPORT DataAreasAvail
      EXPORT StacksAvail
    ENDIF
   ENDIF ; (DOWNLOAD_SERIAL_SUPPORT = NO)           

   EXPORT NDSCoreImageAddr
   
   
;***************
;* Global Data *
;***************
    KEEP
    AREA |!!StartupStacks|, DATA, READWRITE, NOINIT
    ;******************************************************************************
    ;* The following stacks are used for each of the modes, except the Supervisor *
    ;* stack which is only used during startup. Once the OS initializes it will   *
    ;* manage its own Supervisor stack.                                           *
    ;******************************************************************************
    % 96
Startup_FIQ_Stack
    % 1024
Startup_IRQ_Stack
    % 96   
Startup_Undef_Stack
    % 96
Startup_Abort_Stack
    % 4096
Startup_Super_Stack

    IF :DEF:DEBUG
    KEEP
    AREA CacheData, DATA, READWRITE
DataAreasAvail      DCD 0   ;Used for diagnostic - indicates data areas have been set up
StacksAvail         DCD 0   ;Used for diagnostic - indicates stacks have been set up
    ENDIF

    KEEP
    AREA NDSCoreData, DATA, READWRITE
NDSCoreImageAddr    DCD 0   ;Used to remember the NDS Core image address passed in r0 to
                            ;StartupEntry.

    KEEP
    AREA |!!StartupStrings|, CODE, READONLY
CkptStrFlush
    DCB "Flushing CPU caches...", 0x0d, 0x0a, 0x0
CkptStrStacks
    DCB "Initializing stack pointers for each CPU mode...", 0x0d, 0x0a, 0x0
CkptStrData
    DCB "Initializing 'C' variables...", 0x0d, 0x0a, 0x0
CkptStrStartC
    DCB "Switching to 'C' code...", 0x0d, 0x0a, 0x0
CkptStrMismatch
    DCB "Outdated codeldr, replace!", 0x0d, 0x0a, 0x0

   IF :DEF: |ads$version|

CkptStrSysExit
    DCB "_sys_exit called ...", 0xd, 0xa, 0x0
CkptStrRtRaise
    DCB "__rt_raise called ...", 0xd, 0xa, 0x0

   ENDIF

    KEEP
    AREA |!!Startup|, CODE, READONLY

;********************************************************************
;*  Loaded Image Entry Point                                        *
;*                                                                  *
;*  PARAMETERS:                                                     *
;*      None.                                                       *
;*                                                                  *
;*  DESCRIPTION:                                                    *
;*              As the entry point for the loaded code image, our immediate *
;*      goal is to get to C code.                                   *
;*                                                                  *
;*  RETURNS:                                                        *
;*      Never returns.                                              *
;********************************************************************
    ENTRY
    EXPORT StartupEntry
StartupEntry

    IF IMAGE_TYPE = "OPENTV_12"

        ;The OpenTV 1.2 (IMAGE_TYPE == OPENTV_12) code image type

        ;The following placeholders are used by the miniotv bootloader
        ;and the utility which updates the FLASH with a new image from
        ;the MPEG stream for OpenTV 1.2 code images.

        DCD     0                   ;place holder for magic cookie
        DCD     0                   ;place holder for number of blocks
        DCD     0                   ;place holder for payload size
        DCD     0                   ;place holder for image type and rev
        DCD     0x04000000          ;Default Application Id
        DCD     0                   ;place holder for producer id
        DCD     0                   ;place holder for version
        DCD     0                   ;place holder for header crc
        DCD     0                   ;place holder for validation crc

    ENDIF

    EXPORT ADS_StartupEntry
ADS_StartupEntry
        b       StartupCCode

        ;The generic (IMAGE_TYPE == GENERIC) code image type

        ;The following placeholders are used by the code loader and
        ;the utility which updates the FLASH with a new image from
        ;the MPEG stream for generic code images.

ImageLength
        DCD     0                   ;place holder for length of entire image
                                    ;(stored by image postprocessor)
        DCD     0                   ;place holder for checksum
                                    ;(stored by image postprocessor)
        DCB     "CNXT"              ;image signature 
StartAddress        
        DCD     |Image$$FLASH$$Base|+|Image$$RO$$Base|   ;start address of image
        DCD     |Image$$RAM$$Limit|+|Image$$RW$$Limit|   ;end address of image + 1
        DCD     2                   ;header version (0 & 1 were the versions before 
                                    ;the version # was added)
        DCD     0x20020000          ;original flash storage address (stored by 
                                    ;codeldr after copying image into RAM)
        DCB     "Copyright Conexant Systems, all rights reserved.", 0


    IF (IMAGE_SELFCOPY <> YES)
    ; If we're a selfcopying image, don't change the area name 
    ; because we need to be guarunteed that ADS_StartupEntry will  
    ; be close to the ADRL instruction below.
    KEEP
    AREA |!!StartupCode|, CODE, READONLY
    ELSE
    ALIGN 4
    ENDIF
;;********************************************************************
;*  StartupCCode                                                    *
;*                                                                  *
;*  PARAMETERS:                                                     *
;*      None.                                                       *
;*                                                                  *
;*  DESCRIPTION:                                                    *
;*      As the first code executed after the boot loader, it is     *
;*      responsible for initializing the hardware and software      *
;*      constructs that are absolutely necesary to begin executing  *
;*      C code so that as much code as possible can be written in C.*
;*                                                                  *
;*  RETURNS:                                                        *
;*      Never returns.                                              *
;********************************************************************
StartupCCode

        ;For NDS Core we must save r0 so we can later pass it to
        ;nds_main(). We temporarily store it here, then move it
        ;to a RAM variable once the data areas are setup in order 
        ;to free up the RST_SCRATCH2_REG for use elsewhere.
        IF :DEF: RST_SCRATCH2_REG
           ldr  r1, =RST_SCRATCH2_REG
           str  r0, [r1]
        ENDIF

        ; Disable interrupts. They will be off after reset but
        ; the serial codeloader may have left them on
        mrs  r1, CPSR
        orr  r1, r1, #0xC0
        msr  CPSR_cfsx, r1
      IF (IMAGE_SELFCOPY = YES)

        mov  r0, pc
        ldr  r1, =ROM_BASE    ; where is ROM at?
        cmp  r0, r1           ; compare ROM_BASE to PC to see if we're already
                              ; executing from RAM
        blt  NoCopyNecessary  ; We know that RAM is below ROM so if PC < ROM_BASE
                              ; then we know we've already been copied
        
        ldr  r0, StartAddress ; Our Destination address (image start)
        cmp  r0, r1           
        bge  NoCopyNecessary  ; Compare Image start (where are we linked?) to
                              ; ROM Base.  If Image start is greater than ROM
                              ; base, this image should run from ROM and no
                              ; copy is necessary.

        ; Alright, here comes some heavy duty assumptions:
        ; 1. Presumably main images (thats us) will want to use video, mpeg, etc...
        ; 2. Therefore main images can't really utilize ram below 0x240000 depending
        ;    on how big the video and MPEG buffers are.
        ; 3. We know RAM is initialized and stacks have been set up (thank you codeldr)
        ;    when we get here.  These are most likely using a region of memory
        ;    below what we stated in #2.  (See codeldr\makefile RAM_BASE_OVERRIDE)
        ; 4. For these reasons, it should be safe to use FCopy which pushes all
        ;    registers onto the stack so it can take advantage of STM and LDM instr.
        ; 5. NOTE:  If these assumptions fail, then what will end up happening
        ;    is FCOPY will OVERWRITE the region in which we stored our regsiters
        ;    upon entry to FCopy.  Guess what happens when you try to restore them?
        ; 6. In the case of NDSCore, we don't have to perform an image checksum
        ;    because the SCL will have already done that for us.  If you want
        ;    to do a checksum, add it yourself.
        
        ldr  r2, ImageLength  ; End address
        
        ; Ok, we know our destination and we know how much data.  Here's the 
        ; tricky part:  We need some position independent code to figure
        ; out what our source addr really is.  Our linker can't tell us where YOU
        ; decided to flash the code which we are executing RIGHT NOW.
        ADRL r1, StartupEntry
        
        bl   FCopy

        ; OK, so our image is copied right?  Blind jump!  Lets go!
        ldr  pc, StartAddress
        
NoCopyNecessary                              

      ENDIF ; IMAGE_SELFCOPY

      IF (IIC_TYPE <> IIC_TYPE_COLORADO :LAND: IIC_TYPE <> IIC_TYPE_WABASH)
        ;Setup up pin muxing to allow I2C1 bus to work
        ldr     r0, =PLL_PIN_GPIO_MUX3_REG
        ldr     r1, [r0]
        orr     r1, r1, #PLL_PIN_GPIO_MUX3_SCDA1
        orr     r1, r1, #PLL_PIN_GPIO_MUX3_SCL1
        str     r1, [r0]
      ENDIF ; IIC_TYPE

        ;Ensure I2C bus(es) are not stuck!
        mov     r0, #0
        bl      ClearI2CBus
        IF (IIC_TYPE <> IIC_TYPE_COLORADO :LAND: IIC_TYPE <> IIC_TYPE_WABASH)
        mov     r0, #1
        bl      ClearI2CBus
        ENDIF ; IIC_TYPE

      ;Checksum config EEPROM.
      IF (I2C_CONFIG_EEPROM_ADDR <> NOT_PRESENT)
        bl      IsCfgEepromValid
        tst     r0, #1
        mov     r0, #0
        bleq    StartupFatalExit
      ENDIF ; IF (I2C_CONFIG_EEPROM_ADDR <> NOT_PRESENT)

      ;Initialize the GPIO pin mux and alt func registers for unique
      ;IRD configuration.
      IF (PLL_PIN_ALT_FUNC_REG_DEFAULT <> NOT_DETERMINED)
        ldr     r4, =PLL_PIN_ALT_FUNC_REG_DEFAULT
        IF (I2C_CONFIG_EEPROM_ADDR <> NOT_PRESENT) :LAND: \
           (PLL_PIN_ALT_FUNC_REG_DEFAULT_MILANO1 <> NOT_DETERMINED)
        bl      IsMilanoRev1
        cmp     r0, #1
        ldreq   r4, =PLL_PIN_ALT_FUNC_REG_DEFAULT_MILANO1
        ENDIF ; IF (I2C_CONFIG_EEPROM_ADDR <> NOT_PRESENT) :LAND:
              ; (PLL_PIN_ALT_FUNC_REG_DEFAULT_MILANO1 <> NOT_DETERMINED)
        ldr     r0, =PLL_PIN_ALT_FUNC_REG
        str     r4, [r0]
      ENDIF

      IF (PLL_PIN_GPIO_MUX0_REG_DEFAULT <> NOT_DETERMINED)
        ldr     r4, =PLL_PIN_GPIO_MUX0_REG_DEFAULT
        IF (I2C_CONFIG_EEPROM_ADDR <> NOT_PRESENT) :LAND: \
           (PLL_PIN_GPIO_MUX0_REG_DEFAULT_MILANO1 <> NOT_DETERMINED)
        bl      IsMilanoRev1
        cmp     r0, #1
        ldreq   r4, =PLL_PIN_GPIO_MUX0_REG_DEFAULT_MILANO1
        ENDIF ; IF (I2C_CONFIG_EEPROM_ADDR <> NOT_PRESENT) :LAND:
              ; (PLL_PIN_GPIO_MUX0_REG_DEFAULT_MILANO1 <> NOT_DETERMINED)
        ldr     r0, =PLL_PIN_GPIO_MUX0_REG
        str     r4, [r0]
      ENDIF

      IF (PLL_PIN_GPIO_MUX1_REG_DEFAULT <> NOT_DETERMINED)
        ldr     r4, =PLL_PIN_GPIO_MUX1_REG_DEFAULT
        IF (I2C_CONFIG_EEPROM_ADDR <> NOT_PRESENT) :LAND: \
           (PLL_PIN_GPIO_MUX1_REG_DEFAULT_MILANO1 <> NOT_DETERMINED)
        bl      IsMilanoRev1
        cmp     r0, #1
        ldreq   r4, =PLL_PIN_GPIO_MUX1_REG_DEFAULT_MILANO1
        ENDIF ; IF (I2C_CONFIG_EEPROM_ADDR <> NOT_PRESENT) :LAND:
              ; (PLL_PIN_GPIO_MUX1_REG_DEFAULT_MILANO1 <> NOT_DETERMINED)
        ldr     r0, =PLL_PIN_GPIO_MUX1_REG
        str     r4, [r0]
      ENDIF

      IF (PLL_PIN_GPIO_MUX2_REG_DEFAULT <> NOT_DETERMINED)
        ldr     r4, =PLL_PIN_GPIO_MUX2_REG_DEFAULT
        IF (I2C_CONFIG_EEPROM_ADDR <> NOT_PRESENT) :LAND: \
           (PLL_PIN_GPIO_MUX2_REG_DEFAULT_MILANO1 <> NOT_DETERMINED)
        bl      IsMilanoRev1
        cmp     r0, #1
        ldreq   r4, =PLL_PIN_GPIO_MUX2_REG_DEFAULT_MILANO1
        ENDIF ; IF (I2C_CONFIG_EEPROM_ADDR <> NOT_PRESENT) :LAND:
              ; (PLL_PIN_GPIO_MUX2_REG_DEFAULT_MILANO1 <> NOT_DETERMINED)
        ldr     r0, =PLL_PIN_GPIO_MUX2_REG
        str     r4, [r0]
      ENDIF

      IF (PLL_PIN_GPIO_MUX3_REG_DEFAULT <> NOT_DETERMINED)
        ldr     r4, =PLL_PIN_GPIO_MUX3_REG_DEFAULT
        IF (I2C_CONFIG_EEPROM_ADDR <> NOT_PRESENT) :LAND: \
           (PLL_PIN_GPIO_MUX3_REG_DEFAULT_MILANO1 <> NOT_DETERMINED)
        bl      IsMilanoRev1
        cmp     r0, #1
        ldreq   r4, =PLL_PIN_GPIO_MUX3_REG_DEFAULT_MILANO1
        ENDIF ; IF (I2C_CONFIG_EEPROM_ADDR <> NOT_PRESENT) :LAND:
              ; (PLL_PIN_GPIO_MUX3_REG_DEFAULT_MILANO1 <> NOT_DETERMINED)
        ldr     r0, =PLL_PIN_GPIO_MUX3_REG
        str     r4, [r0]
      ENDIF

      ;set clock observation register for SmartMDP and VXX modems
      IF (PLL_TYPE = PLL_TYPE_BRAZOS)
        IF (PLL_CLK_OBSERVATION_REG_DEFAULT <> NOT_DETERMINED)
          ldr     r0, =PLL_CLK_OBSERVATION_REG
          ldr     r1, =PLL_CLK_OBSERVATION_REG_DEFAULT
          str     r1, [r0]
        ENDIF
      ENDIF

 IF (IMAGE_RESETS_HARDWARE = YES)
        ; disable the MMU
         bl    mmuGetState
         bic   r0,r0, #0x05
         bl    mmuSetState
        
        ; Add other things, as necessary to return hardware to POR state.

 ENDIF

 IF (DOWNLOAD_SERIAL_SUPPORT = NO)                
        ; Here we do a sanity check with the bootloader to ensure that
        ; the MMU/MPU is disabled by the bootloader.  If not, it's an
        ; older version that needs to be updated.
        IF :DEF:DEBUG
           mrc     p15, 0, r0, c1, c0, 0 ; load CP15 Reg 1 (Control Register)
           tst     r0, #0x1
           beq     %FT2
           ldr     r0, =CkptStrMismatch  ; throw up an error msg
           bl      Checkpoint
1          b       %T1                   ; lock it up
2
        ENDIF ; :DEF:DEBUG

        ;Setup ASB mode register
        bl      SetUpASBModeReg


        ;Initialize data areas.
        ; NOTE: this must be done here before we can load
	;       a new page table for the ARM920T
        IF :LNOT:(:DEF: |ads$version|)
           ldr     r0, =CkptStrData
           bl      Checkpoint
           bl      InitializeDataAreas
        ELSE
           IMPORT   __main
           b  __main

		   EXPORT cnxt_main
cnxt_main
        ENDIF
        
        ;Now that we have the data areas initialized, we can move
        ;the NDS Core image address value we previously stored
        ;in RST_SCRATCH2_REG to RAM in order to free up this register
        ;for others to use.
        IF :DEF: RST_SCRATCH2_REG
           ldr     r0, =RST_SCRATCH2_REG
           ldr     r1, [r0]
           ldr     r0, =NDSCoreImageAddr
           str     r1, [r0]
        ENDIF

        ;Initialize the stack for each processor mode.
        ldr     r0, =CkptStrStacks
        bl      Checkpoint
        bl      InitializeStacks

        ;Switch to C Code
        ldr     r0, =CkptStrStartC
        bl      Checkpoint

        b       CStartup
 ELSE
        IF :LNOT:(:DEF: |ads$version|)
           bl     InitializeDataAreas
           bl     InitializeStacks
        ENDIF
           IMPORT __main
           b      __main
        IF :DEF: |ads$version|
           EXPORT main
main
           IMPORT main__
           b      main__
        ENDIF

 ENDIF ; DOWNLOAD_SERIAL_SUPPORT
	

;********************************************************************
;*  SetUpASBModeReg                                                 *
;*                                                                  *
;*  PARAMETERS:                                                     *
;*      None.                                                       *
;*                                                                  *
;*  DESCRIPTION:                                                    *
;*      Sets up the ASB mode register, needs to be executed early.  *
;*                                                                  *
;*  RETURNS:                                                        *
;*      Nothing.                                                    *
;*                                                                  *
;*  NOTE:                                                           *
;*      Uses r0 and r1                                              *
;*                                                                  *
;********************************************************************
SetUpASBModeReg
    ldr     r1, =RST_ASB_MODE_REG
    ldr     r0, =RST_ASB_MODE_VALUE
    
    ; In debug builds, always turn off PIT instruction prefetching.
    ; This feature helps performance but causes problems when running
    ; the ARM debuggers. Since they don't know how to flush the
    ; buffer, you can get into states where the debugger tries to 
    ; clear or step over a breakpoint but the instruction is actually
    ; in the prefetch buffer rather than cache or SDRAM and so is not
    ; affected. At best, this prevents you from stepping past a 
    ; breakpoint and at worst the debugger gives up and throws an
    ; exception at you.
    IF :DEF: DEBUG
    and     r0, r0, #(:NOT:(RST_ASB_MEM_INSTR_PREFETCH_ENABLE :OR: RST_ASB_ROM_INSTR_PREFETCH_ENABLE))
    ENDIF
    
    str     r0, [r1]
    bx      lr

;********************************************************************
;*  InitializeStacks                                                *
;*                                                                  *
;*  DESCRIPTION:                                                    *
;*      Switch into each processor mode and initialize the stack    *
;*      pointer to a valid value.                                   *
;*                                                                  *
;*  PARAMETERS:                                                     *
;*      None.                                                       *
;*                                                                  *
;*  RETURNS:                                                        *
;*      Nothing.                                                    *
;*                                                                  *
;*  REGISTER USE:                                                   *
;********************************************************************
InitializeStacks
        mrs     r0, cpsr
        bic     r0, r0, #CPU_MODE
        orr     r1, r0, #CPU_MODE_FIQ
        msr     cpsr_c, r1
        ldr     sp, =Startup_FIQ_Stack
        orr     r1, r0, #CPU_MODE_IRQ
        msr     cpsr_c, r1
        ldr     sp, =Startup_IRQ_Stack
        orr     r1, r0, #CPU_MODE_UNDEF
        msr     cpsr_c, r1
        ldr     sp, =Startup_Undef_Stack
        orr     r1, r0, #CPU_MODE_ABORT
        msr     cpsr_c, r1
        ldr     sp, =Startup_Abort_Stack
        orr     r1, r0, #CPU_MODE_SUPER
        msr     cpsr_c, r1
        ldr     sp, =Startup_Super_Stack

        IF :DEF:DEBUG
           ; Indicate that stacks are now available to code
           mov     r0,#1
           ldr     r1, =StacksAvail
           str     r0, [r1]
        ENDIF

        bx      lr

;********************************************************************
;*  InitializeDataAreas                                             *
;*                                                                  *
;*  DESCRIPTION:                                                    *
;*      Copy the initial values of 'C' variables to their RAM       *
;*      locations and zero all other uninitialized variables.       *
;*                                                                  *
;*  PARAMETERS:                                                     *
;*                           ROM Image                              *
;*                           +-----------------+                    *
;*                           | Initializing    |                    *
;*                           | data ....       |                    *
;*                           +-----------------+ <-Image$$RO$$Limit *
;*                           | CODE            |                    *
;*                           | ...             |                    *
;*         Image$$RO$$Base-> +-----------------+                    *
;*                                                                  *
;*                           RAM Image             Image$$RW$$Limit *
;*                           +-----------------+ <-Image$$ZI$$Limit *
;*                           | Unitialized     |                    *
;*                           | data (zeroed)   |                    *
;*         Image$$ZI$$Base-> +-----------------+                    *
;*                           | Initialized     |                    *
;*                           | data            |                    *
;*         Image$$RW$$Base-> +-----------------+                    *
;*                           | CODE            |                    *
;*                           | ...             |                    *
;*                           +-----------------+                    *
;*                                                                  *
;*                                                                  *
;*  RETURNS:                                                        *
;*      Nothing.                                                    *
;*                                                                  *
;*  REGISTER USE:                                                   *
;********************************************************************
InitializeDataAreas
        ldr     r0, =|Load$$RAM$$Base|
        cmp     r0, #0
        bne     IDA_DoR1
        ldr     r0, =|Image$$RO$$Limit|

IDA_DoR1
        ldr     r1, =|Image$$RAM$$Base|
        cmp     r1, #0
        bne     IDA_DoR2
        ldr     r1, =|Image$$RW$$Base|

IDA_DoR2
        ldr     r2, =|Image$$RAM$$ZI$$Base|
        cmp     r2, #0
        bne     IDA_DoR3
        ldr     r2, =|Image$$ZI$$Base|

IDA_DoR3
        ldr     r3, =|Image$$RAM$$ZI$$Limit|
        cmp     r3, #0
        bne     IDA_RxDone
        ldr     r3, =|Image$$ZI$$Limit|

IDA_RxDone
        ;Check if data is already in place or not.
        cmp     r0, r1
        beq     %FT0

        ;Copy data for initialized variables.
1       ldmia   r0!, {r4-r11}
        stmia   r1!, {r4-r11}
        cmp     r1, r2
        blt     %BT1

        ;Check for variables to zero initialize.
0       cmp     r2, r3
        beq     %FT3

        ;Zero unitialized variables.
        mov     r4, #0
        mov     r5, #0
        mov     r6, #0
        mov     r7, #0
        mov     r8, #0
        mov     r9, #0
        mov     r10, #0
        mov     r11, #0
2       stmia   r2!, {r4-r11}
        cmp     r2, r3
        blt     %BT2


        IF :DEF:DEBUG
           ; Indicate that Data Areas are now available to code
3          mov     r0,#1
           ldr     r1, =DataAreasAvail
           str     r0, [r1]
           bx      lr
        ELSE
3          bx      lr
        ENDIF

;********************************************************************
;*  GetSystemStackBase                                              *
;*                                                                  *
;*  DESCRIPTION:                                                    *
;*      Returns the base address of the supervisor stack            *
;*                                                                  *
;*  RETURNS:                                                        *
;*      r0 - base address of the supervisor stack                   *
;*                                                                  *
;********************************************************************

    KEEP
    IF :DEF: |ads$version|
    AREA    StartupSuperStackBase, CODE, READONLY
    ELSE
    AREA    StartupSuperStackBase, CODE, READONLY, INTERWORK
    ENDIF

    EXPORT GetSuperStackBase
GetSuperStackBase

    ldr     r0, =Startup_Super_Stack
    bx      r14

;********************************************************************
; Assorted Routines required to support ADS Run-Time environment
;********************************************************************
   IF :DEF: |ads$version|

   EXPORT __user_initial_stackheap
__user_initial_stackheap
   mov   r0,r3
   mov   r2,r3
   bx    r14

   AREA  |C$$fp_status_data|,DATA
ADS_FP_STATUS
   DCD   &00000000

   AREA  |C$$fp_status_code|, CODE, READONLY
   EXPORT __rt_fp_status_addr
__rt_fp_status_addr
   ldr   r0, =ADS_FP_STATUS
   bx    lr

   AREA  |C$$rt_raise_code|, CODE, READONLY
   EXPORT   __rt_raise
__rt_raise
  IF (DOWNLOAD_SERIAL_SUPPORT = 0)
   ldr   r0, =CkptStrRtRaise
   bl    Checkpoint
  ENDIF
1  b     %BT1

   AREA  |C$$sys_exit_code|, CODE, READONLY
   EXPORT   _sys_exit
_sys_exit
  IF (DOWNLOAD_SERIAL_SUPPORT = 0)
   ldr   r0, =CkptStrSysExit
   bl    Checkpoint
  ENDIF
1  b     %BT1

   ENDIF



;Originally in NUP_INIS.S ... bring here to keep the Image$$ symbols in one
;place

   IF :DEF: |ads$version|
       AREA |C$$code|, CODE, READONLY
   ELSE
      AREA |C$$code|, CODE, READONLY,INTERWORK
   ENDIF
    EXPORT FirstAvailMem
FirstAvailMem
   DCD   |Image$$RAM$$ZI$$Limit|+|Image$$ZI$$Limit|



;Originally in OS.S ... brin here to keep the Image$$ symbols in one
;place ... also now reference this symbol from ROM.C

   AREA |OSData|, DATA, READWRITE
    EXPORT FreeMemStart
FreeMemStart
   DCD   |Image$$RAM$$ZI$$Limit|+|Image$$ZI$$Limit|


    END

;****************************************************************************
;* Modifications:
;* $Log: 
;*  58   mpeg      1.57        6/22/04 3:02:35 PM     Miles Bintz     CR(s) 
;*        9536 9537 : removed conditional around setting of GPIO pin muxing for
;*         I2C1 pins.  This allows access to config eeprom
;*  57   mpeg      1.56        6/17/04 4:27:54 PM     Miles Bintz     CR(s) 
;*        9498 9499 : fix to let image_selfcopy run images from rom as well
;*  56   mpeg      1.55        6/17/04 3:43:28 PM     Miles Bintz     CR(s) 
;*        9498 9499 : add config options to allow startup to be rentrant and 
;*        self-copying
;*  55   mpeg      1.54        6/2/04 4:58:58 PM      Tim Ross        CR(s) 
;*        9315 9316 : Added EXPORT NDSCoreImageAddr.
;*  54   mpeg      1.53        5/27/04 5:11:32 PM     Tim Ross        CR(s) 
;*        9315 9316 : Save away r0 passed into StartupEntry for later passing 
;*        to nds_main().
;*  53   mpeg      1.52        11/4/03 4:39:56 PM     Angela Swartz   CR(s): 
;*        7777 7778 Initialize PLL_CLK_OBSERVATION_REG to set the clock for 
;*        SmartMDP and SmartVXX modems with Brazos
;*  52   mpeg      1.51        11/3/03 2:01:09 PM     Tim Ross        CR(s): 
;*        7719 7762 Conditionally IMPORTed IsCfgEepromValid, StartupFatalExit, 
;*        and ReadCfgEeprom
;*        to correct ADS build break.
;*  51   mpeg      1.50        11/1/03 2:59:47 PM     Tim Ross        CR(s): 
;*        7719 7762 Moved read of config eeprom before GPIO setup for Milano 
;*        1/3 support.
;*        Enabled IIC immediately to support checkpoint output to LCD early in 
;*        codeldr.
;*        
;*  50   mpeg      1.49        10/27/03 4:00:06 PM    Tim White       CR(s): 
;*        7724 Force C-library initilization when building serial flash 
;*        download tool with ADS toolkit.
;*        
;*  49   mpeg      1.48        9/25/03 4:25:22 PM     Dave Wilson     SCR(s) 
;*        7549 :
;*        In debug builds, we now explicitly disable the PIT instruction 
;*        prefetch
;*        buffer for both ROM and RAM. With prefetch enabled, the ARM AXD 
;*        debugger
;*        would get very confused and often crash if you were single stepping 
;*        over
;*        breakpoints or clearing breakpoints. Turning off prefetch cures this 
;*        problem.
;*        For release builds, the prefetch setting is governed, as before, by 
;*        the 
;*        value in RST_ASB_MODE_VALUE.
;*        
;*  48   mpeg      1.47        9/23/03 6:26:32 PM     Miles Bintz     SCR(s) 
;*        7523 :
;*        changed preprocessor if's to use new DOWNLOAD_SERIAL_SUPPORT 
;*        definitions
;*        
;*  47   mpeg      1.46        8/18/03 7:15:42 AM     Ian Mitchell    SCR(s): 
;*        7300 
;*        rename the function "main" to "cnxt_main", it is now called from the 
;*        function
;*        void $Sub$$main(void) in startupc.c.
;*        
;*  46   mpeg      1.45        7/20/03 10:12:20 AM    Tim White       SCR(s) 
;*        7000 :
;*        The loaders and initial startup code use instruction caching (no 
;*        MMU/MPU) only.
;*        
;*        
;*  45   mpeg      1.44        7/9/03 3:29:28 PM      Tim White       SCR(s) 
;*        6901 :
;*        Phase 3 codeldrext drop.
;*        
;*        
;*  44   mpeg      1.43        6/24/03 6:38:46 PM     Tim White       SCR(s) 
;*        6831 :
;*        Add flash, hsdp, demux, OSD, and demod support to codeldrext.
;*        
;*        
;*  43   mpeg      1.42        5/14/03 4:38:14 PM     Tim White       SCR(s) 
;*        6346 6347 :
;*        Removed duplicate ARM920T mmu/cache initialization.  Call the 
;*        consolidated mmu/cache
;*        initialization code in the new file initmmu.s (also used by the 
;*        loader).
;*        
;*        
;*  42   mpeg      1.41        5/5/03 5:06:44 PM      Tim White       SCR(s) 
;*        6172 :
;*        Remove duplicate low-level boot support code and use startup 
;*        directory for building
;*        codeldr.  Remove 7 segment LED support.
;*        
;*        
;*  41   mpeg      1.40        4/30/03 5:40:38 PM     Craig Dry       SCR(s) 
;*        5521 :
;*        Initialize GPIO Pin Mux and Alt Func registers from hwconfig.cfg.
;*        If the default values are set in hwconfig.cfg, conditionally omit
;*        manipulation of individual bits in these regs.
;*        
;*  40   mpeg      1.39        4/30/03 5:21:40 PM     Billy Jackman   SCR(s) 
;*        6113 :
;*        Added PVCS Header and Log comments.
;*        Added code to read the CP15 ID register and extract the CPU type.
;*        Modified conditional assembly directives and code to add the 
;*        capability to
;*        detect the CPU type (920 vs. 940) at runtime when CPU_TYPE=AUTOSENSE.
;*        Removed some vendor-specific code.
;*        
;*  39   mpeg      1.38        4/25/03 4:50:12 PM     Billy Jackman   SCR(s) 
;*        5855 :
;*        Only reference pagetable if we are actually using a page table in 
;*        RAM.
;*        Initialize the virtual section registers used to generate the virtual
;*         page table.
;*        Set the page table base address to the appropriate register area if 
;*        using the
;*        virtual page table.
;*        
;*  38   mpeg      1.37        4/24/03 5:57:48 PM     Tim White       SCR(s) 
;*        6096 :
;*        Allow serial download tool to build with CONFIG=BRONCO.
;*        
;*        
;*  37   mpeg      1.36        4/4/03 5:33:44 PM      Craig Dry       SCR(s) 
;*        5956 :
;*        Move the ConfigurationValid flag from program memory to pawser 
;*        memory.
;*        Previously startup.s declared the flag in program memory, but the C
;*        initialization of data areas wiped it out for rom links.  The ram 
;*        links
;*        worked just fine.  Pawser memory avoids the problem altogether.
;*        
;*  36   mpeg      1.35        3/28/03 4:24:08 PM     Tim White       SCR(s) 
;*        5909 :
;*        Replace CRYSTAL_FREQUENCY #define with xtal_frequency global.
;*        
;*        
;*  35   mpeg      1.34        3/25/03 6:52:46 PM     Craig Dry       SCR(s) 
;*        5873 :
;*        Added i2c_write routine which accepts a paremeter distinguising which
;*         i2c
;*        bus is being used.  Read configuration data from eeprom and placed at
;*         start
;*        of pawser memory.  Added and set ConfigurationValid flag.
;*        
;*  34   mpeg      1.33        1/31/03 4:52:48 PM     Dave Moore      SCR(s) 
;*        5375 :
;*        Several changes. Added code to set variables when stacks and 
;*        data areas become avail for use. Added check that looks to see
;*        if codeldr comes up with caches/mmu/mpu off. If that is the case
;*        and the runtime is not built for MMU_CACHE_DISABLE we put up an
;*        error and lock up. Changed the calling order of InitializeDataAreas
;*        and InitializeStacks... they were backwards. Added cache flushes
;*        and invalidates after runtime pagetables installed.
;*        
;*  33   mpeg      1.32        12/17/02 4:50:04 PM    Tim White       SCR(s) 
;*        5182 :
;*        Removed hwswitch.a, no longer needed.  Made the 940 ARM_PIT code work
;*         like the 920 ARM_PIT
;*        code.
;*        
;*        
;*  32   mpeg      1.31        12/12/02 5:28:30 PM    Tim White       SCR(s) 
;*        5157 :
;*        No longer need to turn ROM prefetch on and off according to E.D.
;*        
;*        
;*  31   mpeg      1.30        9/14/02 2:10:24 PM     Craig Dry       SCR(s) 
;*        4608 :
;*        Enable Serial Download to be build with ADS1.2 tools
;*        
;*  30   mpeg      1.29        9/11/02 4:07:06 PM     Craig Dry       SCR(s) 
;*        4483 :
;*        Create single serial download .bin file which works for all 
;*        platforms.
;*        
;*  29   mpeg      1.28        7/17/02 9:59:30 PM     Tim Ross        SCR(s) 
;*        4227 :
;*        CHanged the image header to add a header version # and an original 
;*        ROM storage address for support fo split MPEG & CM images.
;*        
;*  28   mpeg      1.27        7/16/02 3:54:54 PM     Holly Le        SCR(s) 
;*        4206 :
;*        Added code to flush TLB to make sure no entries are hanging around 
;*        before enabling caches and MMU.
;*        
;*        
;*  27   mpeg      1.26        6/26/02 4:52:28 PM     Bobby Bradford  SCR(s) 
;*        4100 :
;*        Fixed typo in the conditional import of use_no_semihosting_swi,
;*        and added __rt_fp_status_addr, __rt_raise, and _sys_exit functions
;*        to keep the ADS library semihosting versions from being linked in.
;*        
;*  26   mpeg      1.25        4/30/02 1:11:46 PM     Billy Jackman   SCR(s) 
;*        3656 :
;*        Changed GET hwconfig.a to GET stbcfg.a to conform to new 
;*        configuration.
;*        
;*  25   mpeg      1.24        4/2/02 2:51:58 PM      Bobby Bradford  SCR(s) 
;*        3426 :
;*        Modifications to the startup sequence for ADS compilers ... 
;*        now doing startup in the ADS recommended way (mostly).
;*        
;*  24   mpeg      1.23        4/1/02 9:15:34 AM      Dave Moore      SCR(s) 
;*        3457 :
;*        Added 920T support
;*        
;*        
;*  23   mpeg      1.22        2/4/02 2:08:42 PM      Dave Wilson     SCR(s) 
;*        3121 :
;*        ADS requires that the entry point of the image points to a valid 
;*        instruction.
;*        To ensure this, I added a new label ADS_StartupEntry which points to 
;*        the first
;*        instruction after the BSkyB header. Image is still linked at the same
;*         address
;*        bit this prevents the ADS linker from throwing a fit and quitting.
;*        
;*  22   mpeg      1.21        1/23/02 11:37:56 AM    Tim White       SCR(s) 
;*        3059 :
;*        Changed the value used for RST_ASB_MODE_REG from 0x1c7 to 0x047 
;*        defined in wabash.a driven by athens.cfg.
;*        
;*        
;*  21   mpeg      1.20        11/1/01 2:17:30 PM     Bobby Bradford  SCR(s) 
;*        2828 :
;*        Replaced GET sabine.a with GET hwconfig.a
;*        
;*        
;*  20   mpeg      1.19        8/23/01 6:12:52 PM     Bobby Bradford  SCR(s) 
;*        2546 :
;*        Changed format of MSR  CPSR instruction to be compatible with both 
;*        SDT and ADS assemblers
;*        
;*  19   mpeg      1.18        8/22/01 5:24:56 PM     Miles Bintz     SCR(s) 
;*        2526 :
;*        Added code to turn off interrupts on the ARM core.  This helps with 
;*        warm reboots and eliminates the assumption that interrupts are off 
;*        when startup runs.  Also made the IRQ and Supervisor stacks bigger so
;*         standalone programs can run with no OS.
;*        
;*        
;*  18   mpeg      1.17        7/23/01 11:09:10 AM    Bobby Bradford  SCR(s) 
;*        1913 :
;*        Incorporated numerous changes to support scatter loading ...
;*        First, the Linker Defined Symbols for Base and Limit of code/data 
;*        regions are different when
;*        scatter loading is implemented (e.g. for a normal link, 
;*        Image$$ZI$$Limit is changed to
;*        Image$$RAM$$ZI$$Limit for a scatter loaded link).  To accomodate this
;*         in runtime (as opposed to
;*        compile time), included references to both sets of symbols as "weak" 
;*        references, and added the
;*        two corresponding symbols any where they were used.  If a "weak" 
;*        symbols is referenced, but not
;*        defined externally, it defaults to 0.  When I checked this out, it 
;*        worked for both Scatter Load and normal links, and for both SDT 2.51 
;*        and ADS 1.1
;*        I also moved two symbols that were defined elsewhere (OS.S and 
;*        NUP_INIS.S) into this file.  That
;*        way, any future changes to the scatter loading method will only 
;*        require changes to this source
;*        code file.
;*        u‹
;*        
;*  17   mpeg      1.16        7/18/01 2:15:22 PM     Bobby Bradford  SCR(s) 
;*        2093 :
;*        Added conditional support for ADS11 (no longer support INTERWORK on 
;*        AREA directive)
;*        Added conditional support for ADS11 (various changes for ADS11 
;*        runtime library changes)
;*        p?
;*        
;*  16   mpeg      1.15        3/2/01 4:46:36 PM      Tim White       DCS#1347:
;*         Enable PIT memory prefetch and sub-sync mode.
;*        DCS#1348: Enable PIT memory prefetch and sub-sync mode.
;*        
;*  15   mpeg      1.14        2/21/01 4:14:38 PM     Tim White       DCS#0970:
;*         Added support for multiple vendor (application) id's.
;*        
;*  14   mpeg      1.13        8/29/00 8:21:52 PM     Miles Bintz     added 
;*        function to get the supervisor stack base
;*        
;*  13   mpeg      1.12        8/28/00 5:49:52 PM     Tim White       Added 
;*        OpenTV 1.2 image format conditional to IMAGE_FORMAT=OPENTV_12.
;*        
;*  12   mpeg      1.11        4/12/00 11:18:32 AM    Tim White       Fixed 
;*        assembler warnings.
;*        
;*  11   mpeg      1.10        3/2/00 5:31:52 PM      Tim White       Rename 
;*        Enable/DisableROMDataCache() functions to 
;*        Enable/DisableDataCache(Region)
;*        where Region is the CP_15 Memory Protection Region to enable/disable.
;*          Use
;*        the new MemoryRegion field in BANK_INFO (Bank[]) along with 
;*        CurrentBank.
;*        
;*  10   mpeg      1.9         1/6/00 10:41:14 AM     Dave Wilson     Changes 
;*        for ARM/Thumb builds
;*        
;*  9    mpeg      1.8         12/9/99 3:57:22 PM     Tim White       Reenable 
;*        ROM Data Cache.
;*        
;*  8    mpeg      1.7         11/11/99 2:55:58 PM    Tim White       Do not 
;*        enable ROM data caching, only instruction caching.
;*        
;*  7    mpeg      1.6         10/19/99 3:30:00 PM    Tim Ross        Removed 
;*        SABINE and PID board dependencies.
;*        Made compatible with CN8600 CFG file.
;*        
;*  6    mpeg      1.5         10/14/99 12:17:48 PM   Tim White       Always 
;*        enable caching of the ROM space at code image startup time.
;*        
;*  5    mpeg      1.4         10/13/99 4:58:52 PM    Tim White       Enable 
;*        ROM Instruction and Data Caching.  Note that ROM caching can only
;*        be enabled in STARTUP (not codeldr) due to the fact that the download
;*         (flash.li)
;*        utility runs in user mode and cannot access CP15 to disable ROM 
;*        caching
;*        which is required in order to write/erase the flash.  Also added 
;*        helper
;*        functions for SetupROMs() and flash/flashrw.c in order to allo 
;*        run-from-ROM
;*        code to work correctly preserving the ability to write/erase the 
;*        flash while
;*        running from it.
;*        
;*  4    mpeg      1.3         7/2/99 7:30:48 PM      Tim Ross        Corrected
;*         overlap in checkpoints.
;*        
;*  3    mpeg      1.2         7/1/99 1:06:18 PM      Tim Ross        Added 
;*        call to flush caches.
;*        
;*  2    mpeg      1.1         6/29/99 11:03:10 AM    Tim Ross        Changed 
;*        image header back to be compatible with old format.
;*        Added cache flushing code to beginning of image.
;*        Added stack setup code to ensure stacks are within reserved address 
;*        space.
;*        
;*  1    mpeg      1.0         6/8/99 12:53:48 PM     Tim Ross        
;* $
;****************************************************************************

