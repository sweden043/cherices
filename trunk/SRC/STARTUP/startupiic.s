;/****************************************************************************/
;/*                   CONEXANT PROPRIETARY AND CONFIDENTIAL                  */
;/*                       SOFTWARE FILE/MODULE HEADER                        */
;/*                    Conexant Systems Inc. (c) 1998-2003                   */
;/*                               Austin, TX                                 */
;/*                            All Rights Reserved                           */
;/****************************************************************************/
;/*
; * Filename:       iic.s
; *
; *
; * Description:    Low-level boot IIC support
; *
; *
; * Author:         Senthil Veluswamy
; *
; ****************************************************************************/
;/* $Header: startupiic.s, 1, 4/26/04 12:36:05 PM, Miles Bintz$
; ****************************************************************************/

;/*****************/
;/* Include Files */
;/*****************/
   GET stbcfg.a
   GET board.a
   GET startup.a
        
;/***********/
;/* Aliases */
;/***********/

;/***************/
;/* Global Data */
;/***************/

   EXPORT iic_start
   EXPORT iic_stop
   EXPORT iic_write_bus0
    IF IIC_TYPE <> IIC_TYPE_COLORADO :LAND: IIC_TYPE <> IIC_TYPE_WABASH
   EXPORT iic_write_bus1
    ENDIF
   EXPORT ClearI2CBus

;/************************/
;/* Function definitions */
;/************************/

   KEEP
   IF :DEF: |ads$version|
   AREA   StartupIICS, CODE, READONLY
   ELSE
   AREA   StartupIICS, CODE, READONLY, INTERWORK
   ENDIF

;/*********************************************************************/
;/*  iic_start()                                                      */
;/*                                                                   */
;/*  PARAMETERS: None                                                 */
;/*                                                                   */
;/*  DESCRIPTION:Set the IIC Transaction start                        */
;/*                                                                   */
;/*  REGISTERS:  r0, r1                                               */
;/*                                                                   */
;/*  RETURNS:    Nothing. on Colorado                                 */
;/*              r0 - Start sequence to be sent later on later chips  */
;/*********************************************************************/
iic_start

    IF IIC_TYPE = IIC_TYPE_COLORADO
        ldr     r0, =I2C_MCTRLW
        mov     r1, #0x10             ; Start Sequence
        str     r1, [r0]
    ELSE   ; IIC_TYPE = IIC_TYPE_COLORADO
        mov     r0, #I2C_CTRL_START_SEND ; Will be sent a little later
    ENDIF  ; IIC_TYPE = IIC_TYPE_COLORADO

        bx      lr

;/*********************************************************************/
;/*  iic_stop()                                                       */
;/*                                                                   */
;/*  PARAMETERS: None                                                 */
;/*                                                                   */
;/*  DESCRIPTION:Stop a transaction and bring bus to a stable state.  */
;/*                                                                   */
;/*  REGISTERS:  r0, r1                                               */
;/*                                                                   */
;/*  RETURNS:    Nothing.                                             */
;/*              r0 - Stop sequence to be sent later on later chips   */
;/*********************************************************************/
iic_stop

    IF IIC_TYPE = IIC_TYPE_COLORADO
        ldr     r0, =I2C_MCTRLW
        mov     r1, #0x20             ; Stop Sequence
        str     r1, [r0]
0       ldr     r0, =I2C_MCTRLR
        ldr     r1, [r0]
        ands    r1, r1, #0x80
        beq     %BT0
    ELSE   ; IIC_TYPE = IIC_TYPE_COLORADO
        cmp     r0, #I2C_CTRL_START_SEND    ; Was the start used?
        movne   r0, #0x0                    ; Clear
        orr     r0, r0, #I2C_CTRL_STOP_SEND ; Will be sent a little later
    ENDIF  ; IIC_TYPE = IIC_TYPE_COLORADO

        bx      lr

;/*********************************************************************/
;/*  iic_write_bus0()                                                 */
;/*                                                                   */
;/*  PARAMETERS: r1 - data to be sent to IIC device                   */
;/*                                                                   */
;/*  DESCRIPTION:Write the specified data to the IIC device. Waits for*/
;/*                the write to complete before returning.            */
;/*                                                                   */
;/*  REGISTERS:  r0, r1                                               */
;/*                                                                   */
;/*  RETURNS:    Nothing.                                             */
;/*********************************************************************/
iic_write_bus0

    IF IIC_TYPE = IIC_TYPE_COLORADO
        ldr     r0, =I2C_MDATA        ; Store data
        str     r1, [r0]

        ldr     r0, =I2C_MCTRLW
        mov     r1, #0x01             ; Execute write
        str     r1, [r0]
0       ldr     r0, =I2C_MCTRLR
        ldr     r1, [r0]
        and     r1, r1, #0xbf
        cmp     r1, #0x99
      IF CUSTOMER = "CNXT"
        bne     %BT0                  ; Wait for Done
      ENDIF ; CUSTOMER = "CNXT"
    ELSE   ; IIC_TYPE = IIC_TYPE_COLORADO
        mov     r1, r1, lsl #I2C_CTRL_WDATA0_SHIFT
        orr     r1, r1, r0            ; Include any Start/Stop requests
        ldr     r0, =I2C_CTRL
        str     r1, [r0]
0       ldr     r1, =I2C_STAT
        ldr     r0, [r1]
        tst     r0, #I2C_STAT_DONE_MASK
        beq     %BT0
      IF IIC_TYPE = IIC_TYPE_WABASH
        tst     r0, #I2C_STAT_WRITEACK_MASK
        beq     %BT0
      ENDIF ; IIC_TYPE = IIC_TYPE_WABASH
        mov     r0, #0                ; Clear Start/Stop
        str     r0, [r1]              ; Clear the Done bit
    ENDIF  ; IIC_TYPE = IIC_TYPE_COLORADO

        bx      lr                       ; return

    IF IIC_TYPE <> IIC_TYPE_COLORADO :LAND: IIC_TYPE <> IIC_TYPE_WABASH
;/*********************************************************************/
;/*  iic_write_bus1()                                                 */
;/*                                                                   */
;/*  PARAMETERS: r1 - data to be sent to IIC device                   */
;/*                                                                   */
;/*  DESCRIPTION:Write the specified data to the IIC device. Waits for*/
;/*                the write to complete before returning.            */
;/*                                                                   */
;/*  REGISTERS:  r0, r1                                               */
;/*                                                                   */
;/*  RETURNS:    Nothing.                                             */
;/*********************************************************************/
iic_write_bus1

        mov     r1, r1, lsl #I2C_CTRL_WDATA0_SHIFT
        orr     r1, r1, r0            ; Include any Start/Stop requests
        ldr     r0, =I2C1_CTRL
        str     r1, [r0]
0       ldr     r1, =I2C1_STAT
        ldr     r0, [r1]
        tst     r0, #I2C_STAT_DONE_MASK
        beq     %BT0
      IF IIC_TYPE = IIC_TYPE_WABASH
        tst     r0, #I2C_STAT_WRITEACK_MASK
        beq     %BT0
      ENDIF ; IIC_TYPE = IIC_TYPE_WABASH
        mov     r0, #0                ; Clear Start/Stop
        str     r0, [r1]              ; Clear the Done bit

        bx      lr                    ; return
    ENDIF ;  IIC_TYPE <> IIC_TYPE_COLORADO :LAND: IIC_TYPE <> IIC_TYPE_WABASH

;/*********************************************************************/
;/*  ClearI2CBus()                                                    */
;/*                                                                   */
;/*  PARAMETERS:      r0 - I2C bus instance                           */
;/*                                                                   */
;/*  DESCRIPTION:     Initilize the I2C bus and ensure that it is     */
;/*                   not hung at startup.                            */
;/*                                                                   */
;/*  REGISTER USAGE:  r0, r1, r2, r3, r4                              */
;/*                                                                   */
;/*  RETURNS:    Nothing.                                             */
;/*********************************************************************/
ClearI2CBus

    ; Set the correct I2C register aperture based on r0
    ; If invalid bus number is passed, hang...  Chips prior to
    ; Brazos had only 1 I2C bus.  No chips have more than 2 I2C buses.
        cmp     r0, #1
0       bhi     %BT0

    IF IIC_TYPE = IIC_TYPE_COLORADO :LOR: \
       IIC_TYPE = IIC_TYPE_WABASH
        beq     %BT0
    ELSE
        ldreq   r3, =I2C1_BASE
    ENDIF
        ldrlo   r3, =I2C_BASE

    IF IIC_TYPE = IIC_TYPE_COLORADO

        ; r3 base not used
        ldr     r0, =I2C_CTRLW
        ldr     r1, =I2C_CTRLR
        ldr     r4, [r0]
1       ldr     r2, [r1]
        and     r2, r2, #0x4
        cmp     r2, #0
        bne     %FT4
        bic     r4, r4, #0x8
        str     r4, [r0]
        ldr     r3, =0x9c4
2       subs    r3, r3, #0x1
        bne     %BT2
        orr     r4, r4, #0x8
        str     r4, [r0]
        ldr     r3, =0x9c4
3       subs    r3, r3, #0x1
        bne     %BT3
        b       %BT1

    ELSE  ; IIC_TYPE = IIC_TYPE_COLORADO

        ; r3 contains the I2C register base
        ; Setup the IIC bus at 100 KHz
        mov     r0, #0
        orr     r0, r0, #I2C_BUS_SPEED_100KHZ
        bic     r0, r0, #I2C_MODE_SCL_OVERRIDE
        bic     r0, r0, #I2C_MODE_SDA_OVERRIDE
        orr     r0, r0, #I2C_MODE_HWBUSCONTROL_ENABLE
        orr     r0, r0, #I2C_MODE_SLAVE_WAITST_ENABLE
        bic     r0, r0, #I2C_MODE_MASTER_ARBIT_ENABLE
        str     r0, [r3, #I2C_MODE_REG_OFFSET]

        ldr     r4, [r3, #I2C_MODE_REG_OFFSET]
1       ldr     r2, [r3, #I2C_STAT_REG_OFFSET]
        tst     r2, #I2C_STAT_SDA_MASK
        bne     %FT4
        bic     r4, r4, #I2C_MODE_SCL_OVERRIDE
        bic     r4, r4, #I2C_MODE_SDA_OVERRIDE
        bic     r4, r4, #I2C_MODE_HWBUSCONTROL_ENABLE
        str     r4, [r3, #I2C_MODE_REG_OFFSET]
        ldr     r0, =0x9C4            ; wait a little
2       subs    r0, r0, #0x1
        bne     %BT2
        orr     r4, r4, #I2C_MODE_HWBUSCONTROL_ENABLE
        str     r4, [r3, #I2C_MODE_REG_OFFSET]
        ldr     r0, =0x9C4            ; wait again
3       subs    r0, r0, #0x1
        bne     %BT3
        b       %BT1                  ; check again

    ENDIF ; IIC_TYPE = IIC_TYPE_COLORADO

4       bx      lr

    END

;/****************************************************************************
;* Modifications:
;* $Log: 
;*  1    mpeg      1.0         4/26/04 12:36:05 PM    Miles Bintz     CR(s) 
;*        8953 8954 : renamed iic.s to startupiic.s
;* $
;* 
;*    Rev 1.11   10 Jul 2003 17:19:22   whiteth
;* SCR(s) 6934 :
;* Use "as-is" big-endian mode like everything else does with IIC.  In order to do this,
;* the byte count must be set accordingly in the IIC control register when reading.  Also
;* the stop should be OR'ed along with the start on the read one byte since it's a single
;* transaction.  This allows the IIC mode register to not be shifted between LE and BE mode.
;* 
;* 
;*    Rev 1.10   10 Jul 2003 09:35:42   wangl2
;* SCR(s) 6924 :
;* Change "0  B %T0" to "0  B %BT0".
;* 
;*    Rev 1.9   05 May 2003 16:05:58   whiteth
;* SCR(s) 6172 :
;* Remove duplicate low-level boot support code and use startup directory for building
;* codeldr.  Remove 7 segment LED support.
;* 
;* 
;***** FILE COPIED FROM CODELDR ******
;* vlog -br K:\sabine\pvcs\CODELDR\iic.s_v(iic.s)
;* 
;*    Rev 1.11   27 Mar 2003 09:47:10   whiteth
;* SCR(s) 5888 :
;* Place pawser memory offsets in new file codeldr.a.
;* 
;*    Rev 1.10   26 Mar 2003 17:28:32   whiteth
;* SCR(s) 5888 :
;* Add PLL VCO multiplier calculation.
;* 
;*    Rev 1.9   21 Mar 2003 09:12:32   dryd
;* SCR(s) 5830 :
;* Added "ConfigurationValid" global.  It is only set to 1(TRUE)
;* whenever the configuration data exists in eeprom and is valid,
;* meaning the version = 1 and the checksums are correct.
;* 
;*    Rev 1.8   19 Mar 2003 16:51:48   dryd
;* SCR(s) 5830 :
;* At codeldr startup, read configuration data from eeprom and place
;* it at start of pawser memory.
;* 
;*    Rev 1.7   17 Feb 2003 12:21:20   velusws
;* SCR(s) 5524 :
;* Wrapped (write)loop for Conexant. Other Vendor IRDs may not have LCD screens/other components
;* 
;*    Rev 1.6   13 Feb 2003 16:21:08   velusws
;* SCR(s) 5342 :
;* Removed loop in iic_stop for Colorado. Was waiting for an Ack. May not get one if device is not present.
;* 
;*    Rev 1.5   04 Feb 2003 14:58:20   jackmaw
;* SCR(s) 5402 :
;* Modified the non-Colorado code of iic_stop and iic1_stop to simply or in the
;* requested stop bit in r0.  The semantics of r0 use during iic operation require
;* that it not be used except for control bits, so this is reasonable.  This will
;* also allow single-byte writes with both start and stop set if needed.
;* 
;*    Rev 1.4   04 Feb 2003 14:22:42   velusws
;* SCR(s) 5342 :
;* Bug fixes - set stop bit, move Mode reg addr.
;* 
;*    Rev 1.3   21 Jan 2003 14:47:40   velusws
;* SCR(s) 5276 :
;* Tim/Dave's Debug modifications. Removed hardcoded register values
;* 
;*    Rev 1.2   20 Jan 2003 12:03:02   velusws
;* SCR(s) 5252 :
;* Moved ClearI2CBus from codeldr.s and created ClearI2CBus1 for Brazos.
;* 
;*    Rev 1.1   20 Jan 2003 11:03:50   velusws
;* SCR(s) 5252 :
;* fixed Log comment prefix.
;*
;*   Rev 1.0   20 Jan 2003 11:00:02   velusws
;* SCR(s) 5252 :
;* IIC Driver routines for the Codeloader.
;*
;****************************************************************************/

