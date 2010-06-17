;/****************************************************************************/
;/*                   CONEXANT PROPRIETARY AND CONFIDENTIAL                  */
;/*                       SOFTWARE FILE/MODULE HEADER                        */
;/*                    Conexant Systems Inc. (c) 1998-2003                   */
;/*                               Austin, TX                                 */
;/*                            All Rights Reserved                           */
;/****************************************************************************/
;/*
; * Filename:       eecfgs.s
; *
; *
; * Description:    Low-level configuration EEPROM routines
; *
; *
; * Author:         Tim Ross
; *
; ****************************************************************************/
;/* $Id: eecfgs.s,v 1.1, 2004-03-19 20:21:43Z, Craig Dry$
; ****************************************************************************/

;/*****************/
;/* Include Files */
;/*****************/
   GET stbcfg.a
   GET board.a
   GET startup.a
        
    IF IIC_TYPE = IIC_TYPE_COLORADO :LOR: IIC_TYPE = IIC_TYPE_WABASH
      IF I2C_CONFIG_EEPROM_ADDR <> NOT_PRESENT
        IF I2C_CONFIG_EEPROM_BUS = I2C_BUS_1
          "Attempted to use I2C_BUS_1 on a chip which doesn't support it!"
        ENDIF
      ENDIF
    ENDIF

   IMPORT iic_start
   IMPORT iic_stop
   IMPORT iic_write_bus0
    IF IIC_TYPE <> IIC_TYPE_COLORADO :LAND: IIC_TYPE <> IIC_TYPE_WABASH
   IMPORT iic_write_bus1
    ENDIF
   IMPORT ClearI2CBus

;/***********/
;/* Aliases */
;/***********/

;/***************/
;/* Global Data */
;/***************/

;/************************/
;/* Function definitions */
;/************************/

   KEEP
   IF :DEF: |ads$version|
   AREA   StartupEecfgs, CODE, READONLY
   ELSE
   AREA   StartupEecfgs, CODE, READONLY, INTERWORK
   ENDIF

;/*********************************************************************/
;/*  IsCfgEepromValid()                                               */
;/*                                                                   */
;/*  PARAMETERS:  none                                                */
;/*                                                                   */
;/*  DESCRIPTION: Read configuration and place at start of pawser     */
;/*               memory.                                             */
;/*                                                                   */
;/*  REGISTERS:  r0, r1, r2, r3, r4, r5, r6                           */
;/*                                                                   */
;/*  RETURNS:    r0 - 1 if config eeprom contents valid, 0 otherwise  */
;/*********************************************************************/
   EXPORT IsCfgEepromValid
IsCfgEepromValid
 IF (I2C_CONFIG_EEPROM_ADDR <> NOT_PRESENT)
    mov     r6, lr

;;
;; Ensure I2C bus(es) are not stuck!
;;
    mov     r0, #0
    bl      ClearI2CBus
    IF (IIC_TYPE <> IIC_TYPE_COLORADO :LAND: IIC_TYPE <> IIC_TYPE_WABASH)
    mov     r0, #1
    bl      ClearI2CBus
    ENDIF ; IIC_TYPE

;;
;; Check that we have version 1
;;
    mov     r0, #1
    mov     r1, #0
    bl      ReadCfgEeprom
    cmp     r0, #1
    bne     %FT2                ; Version is invalid.  Return.

;;
;; Read in eeprom size
;;
    mov     r0, #3
    mov     r1, #1
    bl      ReadCfgEeprom
    mov     r4, r0              ; Number of valid bytes
  
;;
;;  Checksum eeprom contents.
;;
    mov     r5, #0              ; initialize checksum

0   sub     r4, r4, #1          ; read eeprom data from end to beginning
    cmp     r4, #7              ; Skip checksum for bytes 4..7
    bgt     %FT1
    cmp     r4, #4
    blt     %FT1
    b       %BT0
1   mov     r0, #1              ; read 1 byte at a time
    mov     r1, r4
    bl      ReadCfgEeprom      
    add     r5, r5, r0          ; compute checksum
    cmp     r4, #0              ; done?
    bgt     %BT0
    
    bic     r5, r5, #0xff000000 ; only 16-bit checksum                
    bic     r5, r5, #0xff0000

;;
;; Compare computed checksum and eeprom checksum
;;
    mov     r0, #2              ; read eeprom checksum & compare
    mov     r1, #4
    bl      ReadCfgEeprom
    cmp     r0, r5

;;
;; set cfg eeprom valid bit in scratch reg
;;
2   ldr     r0, =RST_SCRATCH_REG
    ldr     r1, [r0]
    bicne   r1, r1, #RST_SCRATCH_CFG_EEPROM_VALID
    orreq   r1, r1, #RST_SCRATCH_CFG_EEPROM_VALID
    str     r1, [r0]
    movne   r0, #0
    moveq   r0, #1

    bx      r6                  ; return

 ENDIF  ; (I2C_CONFIG_EEPROM_ADDR <> NOT_PRESENT)

;**************************************************************************************
;* IMPORTANT:                                                                         *
;*     All of the following code is completely RAM independent. This is crucial       *
;*     to its operation since it is called before RAM has been setup. Only registers  *
;*     r0-r3 are used to ensure it is callable from 'C' code.                         *
;**************************************************************************************

 IF (I2C_CONFIG_EEPROM_ADDR <> NOT_PRESENT)
;/*********************************************************************/
;/*  ReadCfgEeprom()                                                  */
;/*                                                                   */
;/*  PARAMETERS:  r0 - read 1, 2, 3, or 4 bytes?                      */
;/*               r1 - byte offset internal to EEPROM                 */
;/*                                                                   */
;/*  DESCRIPTION: Read up to 4 configuration bytes from Atmel EEPROM  */
;/*                                                                   */
;/*  REGISTERS:  r0, r1, r2, r3, r12                                  */
;/*              r2[1:0]   - # bytes to read                          */
;/*              r2[9:8]   - # bytes read                             */
;/*              r2[31:16] - byte offset                              */
;/*                                                                   */
;/*  RETURNS:    r0 - data bytes returned                             */
;/*********************************************************************/
   EXPORT ReadCfgEeprom
ReadCfgEeprom

        mov     r12, lr
        
        ;check for valid input
        cmp     r0, #0
        ble     %FT0
        cmp     r0, #4
        bgt     %FT0

        ;save input params
        mov     r2, r1, lsl #16
        orr     r2, r2, r0
        
        ;clear data reg
        mov      r3, #0

        ;send start + device address + write cmd
1       bl      iic_start                 
        mov     r1, #I2C_CONFIG_EEPROM_ADDR
    IF I2C_CONFIG_EEPROM_BUS = I2C_BUS_1
        bl      iic_write_bus1
    ELSE
        bl      iic_write_bus0
    ENDIF

        ;send msb of offset
        mov     r1, r2, lsr #24            
    IF I2C_CONFIG_EEPROM_BUS = I2C_BUS_1
        bl      iic_write_bus1
    ELSE
        bl      iic_write_bus0
    ENDIF

        ;send lsb of offset
        mov     r1, r2, lsr #16
        and     r1, r1, #0xff             
    IF I2C_CONFIG_EEPROM_BUS = I2C_BUS_1
        bl      iic_write_bus1
    ELSE
        bl      iic_write_bus0
    ENDIF

        ;send start + device address + read cmd
        bl      iic_start                 
        bl      iic_stop
        orr     r0, r0, #1
        mov     r1, #(I2C_CONFIG_EEPROM_ADDR | 1)
    IF I2C_CONFIG_EEPROM_BUS = I2C_BUS_1
        bl      iic_write_bus1
    ELSE
        bl      iic_write_bus0
    ENDIF

        ;read the data byte
    IF I2C_CONFIG_EEPROM_BUS = I2C_BUS_1
        ldr     r1, =I2C1_READDATA        
    ELSE
        ldr     r1, =I2C_READDATA 
    ENDIF
        ldr     r0, [r1]

        ;save bytes read in little endian order
        and     r0, r0, #0xff                   ;only use low byte
        and     r1, r2, #0x300                  ;how many bytes to shift?
        mov     r1, r1, lsr #8                  ;
        mov     r1, r1, lsl #3                  ;convert # bytes to # bits
        mov     r0, r0, lsl r1                  ;shift byte into position
        orr     r3, r3, r0                      ;save byte with previous bytes
        
        ;increment offset & byte count for next read
        add     r2, r2, #0x10000
        add     r2, r2, #0x100
        
        ;read more bytes?
        sub     r2, r2, #1
        tst     r2, #3
        bne     %BT1

        ;return bytes read in r0
        mov     r0, r3

0       bx      r12

 ENDIF  ; (I2C_CONFIG_EEPROM_ADDR <> NOT_PRESENT)


    IF (I2C_CONFIG_EEPROM_ADDR <> NOT_PRESENT)
;********************************************************************
;*  IsMilanoRev1                                                    *
;*                                                                  *
;*  DESCRIPTION:                                                    *
;*      Reads config EEPROM to determine if board is a Milano Rev 1.*
;*                                                                  *
;*  PARAMETERS:                                                     *
;*      None.                                                       *
;*                                                                  *
;*  RETURNS:                                                        *
;*      r0 - 1 if Milano rev 1, 0 otherwise.                        *
;*                                                                  *
;*  REGISTER USAGE:                                                 *
;*      r0, r1, r2, r3, r11, r12                                    *
;*                                                                  *
;********************************************************************
    EXPORT IsMilanoRev1
IsMilanoRev1

    mov     r11, lr

    mov     r0, #BOARD_CONFIG_BOARD_TYPE_SIZE
    mov     r1, #BOARD_CONFIG_BOARD_TYPE_OFFSET
    bl      ReadCfgEeprom
    cmp     r0, #BOARD_CONFIG_BOARD_TYPE_MILANO
    bne     %FT0
    mov     r0, #BOARD_CONFIG_BOARD_REV_SIZE
    mov     r1, #BOARD_CONFIG_BOARD_REV_OFFSET
    bl      ReadCfgEeprom
    cmp     r0, #1
    bne     %FT0
    mov     r0, #1
    b       %FT1
    
0   mov     r0, #0
1   bx      r11

    ENDIF ; IF (I2C_CONFIG_EEPROM_ADDR <> NOT_PRESENT)


    END

 END
 
 
;/****************************************************************************
;* Modifications:
;* $Log: 
;*  2    mpeg      1.1         3/19/04 2:21:43 PM     Craig Dry       CR(s) 
;*        8599 : Fix Compiler/Assembler warnings for Codeldr.
;*  1    mpeg      1.0         11/1/03 2:55:41 PM     Tim Ross        CR(s): 
;*        7719 7762 Board configuration EEPROM routines.
;* $
;****************************************************************************/

