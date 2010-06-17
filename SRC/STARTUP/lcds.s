;/****************************************************************************/
;/*                   CONEXANT PROPRIETARY AND CONFIDENTIAL                  */
;/*                       SOFTWARE FILE/MODULE HEADER                        */
;/*                    Conexant Systems Inc. (c) 1998-2003                   */
;/*                               Austin, TX                                 */
;/*                            All Rights Reserved                           */
;/****************************************************************************/
;/*
; * Filename:       lcds.s
; *
; *
; * Description:    Low-level boot LCD support
; *
; *
; * Author:         Dave Wilson
; *
; ****************************************************************************/
;/* $Header: lcds.s, 12, 11/4/03 4:53:14 PM, Miles Bintz$
; ****************************************************************************/

;******************
;* Include Files  *
;******************
    GET stbcfg.a

    GET lcds.a
    GET board.a

    IF IIC_TYPE = IIC_TYPE_COLORADO :LOR: IIC_TYPE = IIC_TYPE_WABASH
      IF I2C_ADDR_LCD_MATRIX <> NOT_PRESENT
        IF I2C_BUS_LCD_MATRIX = I2C_BUS_1
          "Attempted to use I2C_BUS_1 on a chip which doesn't support it!"
        ENDIF
      ENDIF
    ENDIF

;****************************
;* Import the IIC functions *
;****************************
    IMPORT iic_write_bus0
    IF IIC_TYPE <> IIC_TYPE_COLORADO :LAND: IIC_TYPE <> IIC_TYPE_WABASH
    IMPORT iic_write_bus1
    ENDIF
    IMPORT iic_start
    IMPORT iic_stop

;***************
;* Global Data *
;***************
    KEEP
    IF :DEF: |ads$version|
    AREA LCDs, CODE, READONLY
    ELSE
    AREA LCDs, CODE, READONLY, INTERWORK
    ENDIF

;**************************************************************************************
;* IMPORTANT:                                                                         *
;*     All of the following code is completely RAM independent. This is crucial       *
;*     to its operation since it is called before RAM has been setup. Only registers  *
;*     r0-r3 are used to ensure it is callable from 'C' code.                         *
;**************************************************************************************

;********************************************************************
;*  WriteLCDs                                                       *
;*                                                                  *
;*  PARAMETERS:                                                     *
;*      r0  contains a pointer to the string to be written to       *
;*          the top left of the display                             *
;*                                                                  *
;*  DESCRIPTION:                                                    *
;*      Display checkpoint string on the LCD display                *
;*                                                                  *
;*  RETURNS:                                                        *
;*      Nothing.                                                    *
;*                                                                  *
;*  REGISTER USAGE:                                                 *
;*      r0, r1, r2, r3                                              *
;*                                                                  *
;********************************************************************
    EXPORT WriteLCDs
WriteLCDs
        mov     r2, lr      ; Save lr

  IF I2C_BUS_LCD_MATRIX <> I2C_BUS_NONE

        mov     r3, r0      ; Save string
        cmp     r0, #0
        beq     %FT2

        ; Clear the display
        bl iic_start
        mov     r1, #I2C_ADDR_LCD_MATRIX
    IF IIC_TYPE <> IIC_TYPE_COLORADO :LAND: \
       IIC_TYPE <> IIC_TYPE_WABASH :LAND: \
       I2C_BUS_LCD_MATRIX = I2C_BUS_1
        bl iic_write_bus1
    ELSE
        bl iic_write_bus0
    ENDIF
        mov     r1, #LCD_COMMAND_PREFIX
    IF IIC_TYPE <> IIC_TYPE_COLORADO :LAND: \
       IIC_TYPE <> IIC_TYPE_WABASH :LAND: \
       I2C_BUS_LCD_MATRIX = I2C_BUS_1
        bl iic_write_bus1
    ELSE
        bl iic_write_bus0
    ENDIF
        mov     r1, #LCD_CLEAR_DISPLAY
    IF IIC_TYPE = IIC_TYPE_COLORADO
        bl iic_write_bus0
        bl iic_stop
    ELSE
        bl iic_stop
      IF IIC_TYPE <> IIC_TYPE_WABASH :LAND: I2C_BUS_LCD_MATRIX = I2C_BUS_1
        bl iic_write_bus1
      ELSE
        bl iic_write_bus0
      ENDIF
    ENDIF

        ; Goto to upper-left corner
        bl iic_start
        mov     r1, #I2C_ADDR_LCD_MATRIX
    IF IIC_TYPE <> IIC_TYPE_COLORADO :LAND: \
       IIC_TYPE <> IIC_TYPE_WABASH :LAND: \
       I2C_BUS_LCD_MATRIX = I2C_BUS_1
        bl iic_write_bus1
    ELSE
        bl iic_write_bus0
    ENDIF
        mov     r1, #LCD_COMMAND_PREFIX
    IF IIC_TYPE <> IIC_TYPE_COLORADO :LAND: \
       IIC_TYPE <> IIC_TYPE_WABASH :LAND: \
       I2C_BUS_LCD_MATRIX = I2C_BUS_1
        bl iic_write_bus1
    ELSE
        bl iic_write_bus0
    ENDIF
        mov     r1, #LCD_GOTO_HOME
    IF IIC_TYPE = IIC_TYPE_COLORADO
        bl iic_write_bus0
        bl iic_stop
    ELSE
        bl iic_stop
      IF IIC_TYPE <> IIC_TYPE_WABASH :LAND: I2C_BUS_LCD_MATRIX = I2C_BUS_1
        bl iic_write_bus1
      ELSE
        bl iic_write_bus0
      ENDIF
    ENDIF

        ; Write the string to the LCD display
        bl iic_start
        mov     r1, #I2C_ADDR_LCD_MATRIX
    IF IIC_TYPE <> IIC_TYPE_COLORADO :LAND: \
       IIC_TYPE <> IIC_TYPE_WABASH :LAND: \
       I2C_BUS_LCD_MATRIX = I2C_BUS_1
        bl iic_write_bus1
    ELSE
        bl iic_write_bus0
    ENDIF

        ; Get the next character from the string
0       ldrb    r1, [r3], #1

        ; If it's zero, we are finished
        cmp     r1, #0
        beq     %FT1

        ; ...else write it to the display
    IF IIC_TYPE <> IIC_TYPE_COLORADO :LAND: \
       IIC_TYPE <> IIC_TYPE_WABASH :LAND: \
       I2C_BUS_LCD_MATRIX = I2C_BUS_1
        bl iic_write_bus1
    ELSE
        bl iic_write_bus0
    ENDIF

        ; ...and go back for the next character
        b       %BT0

1       bl iic_stop

    IF IIC_TYPE <> IIC_TYPE_COLORADO
        mov     r1, #13     ;  Add an extra non printing char
      IF IIC_TYPE <> IIC_TYPE_WABASH :LAND: I2C_BUS_LCD_MATRIX = I2C_BUS_1
        bl iic_write_bus1
      ELSE
        bl iic_write_bus0
      ENDIF
    ENDIF  ; IIC_TYPE <> IIC_TYPE_COLORADO

  ENDIF ; I2C_BUS_LCD_MATRIX = I2C_BUS_NONE

        ;Return
2       bx      r2


;********************************************************************
;*  ClearLCDs                                                       *
;*                                                                  *
;*  PARAMETERS:                                                     *
;*      None.                                                       *
;*                                                                  *
;*  DESCRIPTION:                                                    *
;*      Clear the LCD display.                                      *
;*                                                                  *
;*  RETURNS:                                                        *
;*      Nothing.                                                    *
;*                                                                  *
;*  REGISTER USAGE:                                                 *
;*      r0, r1, r3                                                  *
;*                                                                  *
;********************************************************************
    EXPORT ClearLCDs
ClearLCDs
        mov     r3, lr      ;save lr

  IF I2C_BUS_LCD_MATRIX <> I2C_BUS_NONE

        ; Clear display
        bl iic_start
        mov     r1,#I2C_ADDR_LCD_MATRIX
    IF IIC_TYPE <> IIC_TYPE_COLORADO :LAND: \
       IIC_TYPE <> IIC_TYPE_WABASH :LAND: \
       I2C_BUS_LCD_MATRIX = I2C_BUS_1
        bl iic_write_bus1
    ELSE
        bl iic_write_bus0
    ENDIF
        mov     r1, #LCD_COMMAND_PREFIX
    IF IIC_TYPE <> IIC_TYPE_COLORADO :LAND: \
       IIC_TYPE <> IIC_TYPE_WABASH :LAND: \
       I2C_BUS_LCD_MATRIX = I2C_BUS_1
        bl iic_write_bus1
    ELSE
        bl iic_write_bus0
    ENDIF
        mov     r1, #LCD_CLEAR_DISPLAY
    IF IIC_TYPE = IIC_TYPE_COLORADO
        bl iic_write_bus0
        bl iic_stop
    ELSE
        bl iic_stop
      IF IIC_TYPE <> IIC_TYPE_WABASH :LAND: I2C_BUS_LCD_MATRIX = I2C_BUS_1
        bl iic_write_bus1
      ELSE
        bl iic_write_bus0
      ENDIF
    ENDIF

        ; Goto to upper-left corner
        bl iic_start
        mov     r1,#I2C_ADDR_LCD_MATRIX
    IF IIC_TYPE <> IIC_TYPE_COLORADO :LAND: \
       IIC_TYPE <> IIC_TYPE_WABASH :LAND: \
       I2C_BUS_LCD_MATRIX = I2C_BUS_1
        bl iic_write_bus1
    ELSE
        bl iic_write_bus0
    ENDIF
        mov     r1, #LCD_COMMAND_PREFIX
    IF IIC_TYPE <> IIC_TYPE_COLORADO :LAND: \
       IIC_TYPE <> IIC_TYPE_WABASH :LAND: \
       I2C_BUS_LCD_MATRIX = I2C_BUS_1
        bl iic_write_bus1
    ELSE
        bl iic_write_bus0
    ENDIF
        mov     r1, #LCD_GOTO_HOME
    IF IIC_TYPE = IIC_TYPE_COLORADO
        bl iic_write_bus0
        bl iic_stop
    ELSE
        bl iic_stop
      IF IIC_TYPE <> IIC_TYPE_WABASH :LAND: I2C_BUS_LCD_MATRIX = I2C_BUS_1
        bl iic_write_bus1
      ELSE
        bl iic_write_bus0
      ENDIF
    ENDIF
       
  ENDIF ; I2C_BUS_LCD_MATRIX = I2C_BUS_NONE

        bx      r3          ;return

;********************************************************************
;*  InitLCDs                                                        *
;*                                                                  *
;*  PARAMETERS:                                                     *
;*      None.                                                       *
;*                                                                  *
;*  DESCRIPTION:                                                    *
;*      Setup for primary IIC Controller                            *
;*                                                                  *
;*  RETURNS:                                                        *
;*      Nothing.                                                    *
;*                                                                  *
;*  REGISTER USAGE:                                                 *
;*      r0, r1, r2, r3                                              *
;*                                                                  *
;*  NOTE:                                                           *
;*      Uses r2 to save the lr and calls ClearDisplay which uses    *
;*      r3 to save lr.                                              *
;*                                                                  *
;********************************************************************
    EXPORT InitLCDs
InitLCDs
        mov     r2, lr      ;save lr

  IF I2C_BUS_LCD_MATRIX <> I2C_BUS_NONE

    IF IIC_TYPE = IIC_TYPE_COLORADO
        ldr     r0, =I2C_CTRLW
        mov     r1, #0x8f   ; Stop Sequence
        str     r1, [r0]
        bl iic_stop
    ENDIF  ; IIC_TYPE = IIC_TYPE_COLORADO

        ; Clear the LCD display
        bl ClearLCDs        ; Uses r0, r1, r3

        ; Set auto-scroll
        bl iic_start
        mov     r1,#I2C_ADDR_LCD_MATRIX
    IF IIC_TYPE <> IIC_TYPE_COLORADO :LAND: \
       IIC_TYPE <> IIC_TYPE_WABASH :LAND: \
       I2C_BUS_LCD_MATRIX = I2C_BUS_1
        bl iic_write_bus1
    ELSE
        bl iic_write_bus0
    ENDIF
        mov     r1, #LCD_COMMAND_PREFIX
    IF IIC_TYPE <> IIC_TYPE_COLORADO :LAND: \
       IIC_TYPE <> IIC_TYPE_WABASH :LAND: \
       I2C_BUS_LCD_MATRIX = I2C_BUS_1
        bl iic_write_bus1
    ELSE
        bl iic_write_bus0
    ENDIF
        mov     r1, #LCD_SCROLL_ON
    IF IIC_TYPE = IIC_TYPE_COLORADO
        bl iic_write_bus0
        bl iic_stop
    ELSE
        bl iic_stop
      IF IIC_TYPE <> IIC_TYPE_WABASH :LAND: I2C_BUS_LCD_MATRIX = I2C_BUS_1
        bl iic_write_bus1
      ELSE
        bl iic_write_bus0
      ENDIF
    ENDIF

        ; Set auto-wrap
        bl iic_start
        mov     r1,#I2C_ADDR_LCD_MATRIX
    IF IIC_TYPE <> IIC_TYPE_COLORADO :LAND: \
       IIC_TYPE <> IIC_TYPE_WABASH :LAND: \
       I2C_BUS_LCD_MATRIX = I2C_BUS_1
        bl iic_write_bus1
    ELSE
        bl iic_write_bus0
    ENDIF
        mov     r1, #LCD_COMMAND_PREFIX
    IF IIC_TYPE <> IIC_TYPE_COLORADO :LAND: \
       IIC_TYPE <> IIC_TYPE_WABASH :LAND: \
       I2C_BUS_LCD_MATRIX = I2C_BUS_1
        bl iic_write_bus1
    ELSE
        bl iic_write_bus0
    ENDIF
        mov     r1, #LCD_AUTO_WRAP_ON
    IF IIC_TYPE = IIC_TYPE_COLORADO
        bl iic_write_bus0
        bl iic_stop
    ELSE
        bl iic_stop
      IF IIC_TYPE <> IIC_TYPE_WABASH :LAND: I2C_BUS_LCD_MATRIX = I2C_BUS_1
        bl iic_write_bus1
      ELSE
        bl iic_write_bus0
      ENDIF
    ENDIF

        ; Turn off the cursor
        bl iic_start
        mov     r1,#I2C_ADDR_LCD_MATRIX
    IF IIC_TYPE <> IIC_TYPE_COLORADO :LAND: \
       IIC_TYPE <> IIC_TYPE_WABASH :LAND: \
       I2C_BUS_LCD_MATRIX = I2C_BUS_1
        bl iic_write_bus1
    ELSE
        bl iic_write_bus0
    ENDIF
        mov     r1, #LCD_COMMAND_PREFIX
    IF IIC_TYPE <> IIC_TYPE_COLORADO :LAND: \
       IIC_TYPE <> IIC_TYPE_WABASH :LAND: \
       I2C_BUS_LCD_MATRIX = I2C_BUS_1
        bl iic_write_bus1
    ELSE
        bl iic_write_bus0
    ENDIF
        mov     r1, #LCD_CURSOR_OFF
    IF IIC_TYPE = IIC_TYPE_COLORADO
        bl iic_write_bus0
        bl iic_stop
    ELSE
        bl iic_stop
      IF IIC_TYPE <> IIC_TYPE_WABASH :LAND: I2C_BUS_LCD_MATRIX = I2C_BUS_1
        bl iic_write_bus1
      ELSE
        bl iic_write_bus0
      ENDIF
    ENDIF

        ; Turn off the blinking
        bl iic_start
        mov     r1,#I2C_ADDR_LCD_MATRIX
    IF IIC_TYPE <> IIC_TYPE_COLORADO :LAND: \
       IIC_TYPE <> IIC_TYPE_WABASH :LAND: \
       I2C_BUS_LCD_MATRIX = I2C_BUS_1
        bl iic_write_bus1
    ELSE
        bl iic_write_bus0
    ENDIF
        mov     r1, #LCD_COMMAND_PREFIX
    IF IIC_TYPE <> IIC_TYPE_COLORADO :LAND: \
       IIC_TYPE <> IIC_TYPE_WABASH :LAND: \
       I2C_BUS_LCD_MATRIX = I2C_BUS_1
        bl iic_write_bus1
    ELSE
        bl iic_write_bus0
    ENDIF
        mov     r1, #LCD_BLINK_OFF
    IF IIC_TYPE = IIC_TYPE_COLORADO
        bl iic_write_bus0
        bl iic_stop
    ELSE
        bl iic_stop
      IF IIC_TYPE <> IIC_TYPE_WABASH :LAND: I2C_BUS_LCD_MATRIX = I2C_BUS_1
        bl iic_write_bus1
      ELSE
        bl iic_write_bus0
      ENDIF
    ENDIF

  ENDIF ; I2C_BUS_LCD_MATRIX = I2C_BUS_NONE

        bx      r2          ;return

    END

;****************************************************************************
;* Modifications:
;* $Log: 
;*  12   mpeg      1.11        11/4/03 4:53:14 PM     Miles Bintz     CR(s): 
;*        7801 fix attempted appending of cable modem images onto codeldr
;*  11   mpeg      1.10        5/5/03 5:06:02 PM      Tim White       SCR(s) 
;*        6172 :
;*        Remove duplicate low-level boot support code and use startup 
;*        directory for building
;*        codeldr.  Remove 7 segment LED support.
;*        
;*        
;*  10   mpeg      1.9         1/29/03 2:32:44 PM     Senthil Veluswamy SCR(s) 
;*        5332 :
;*        Updated to work with IIC.S
;*        
;*  9    mpeg      1.8         1/24/03 9:52:46 AM     Dave Moore      SCR(s) 
;*        5305 :
;*        removed FREQ_SCREEN
;*        
;*        
;*  8    mpeg      1.7         1/22/03 11:43:00 AM    Dave Wilson     SCR(s) 
;*        5099 :
;*        Reworked to allow operation on either I2C bus when built for Brazos.
;*        
;*  7    mpeg      1.6         12/17/02 4:49:22 PM    Tim White       SCR(s) 
;*        5182 :
;*        Removed ARM_PIT_TYPE, no longer needed.
;*        Removed hwswitch.a, no longer needed.
;*        
;*        
;*  6    mpeg      1.5         12/12/02 5:26:02 PM    Tim White       SCR(s) 
;*        5157 :
;*        Use IIC_TYPE <> IIC_TYPE_COLORADO instead of IIC_TYPE = 
;*        IIC_TYPE_WABASH.
;*        
;*        
;*  5    mpeg      1.4         8/30/02 8:57:20 PM     Senthil Veluswamy SCR(s) 
;*        4502 :
;*        Add extra space for Wabash LCD
;*        
;*  4    mpeg      1.3         8/30/02 6:45:46 PM     Senthil Veluswamy SCR(s) 
;*        4502 :
;*        Modifications for using the New IIC interface
;*        
;*  3    mpeg      1.2         4/30/02 1:11:24 PM     Billy Jackman   SCR(s) 
;*        3656 :
;*        Changed GET hwconfig.a to GET stbcfg.a to conform to new 
;*        configuration.
;*        
;*  2    mpeg      1.1         11/1/01 2:16:44 PM     Bobby Bradford  SCR(s) 
;*        2828 :
;*        Replaced GET sabine.a with GET hwconfig.a
;*        
;*        
;*  1    mpeg      1.0         4/7/00 3:08:52 PM      Dave Wilson     
;* $
;* 
;*    Rev 1.10   05 May 2003 16:06:02   whiteth
;* SCR(s) 6172 :
;* Remove duplicate low-level boot support code and use startup directory for building
;* codeldr.  Remove 7 segment LED support.
;* 
;* 
;***** FILE COPIED FROM CODELDR ******
;* vlog -br K:\sabine\pvcs\CODELDR\lcds.s_v(lcds.s)
;* 
;*    Rev 1.11   20 Jan 2003 11:58:58   velusws
;* SCR(s) 5252 :
;* Modified to use the I2C Device Addresses from hwconfig.cfg instead of board.a
;* 
;*    Rev 1.10   20 Jan 2003 11:14:30   velusws
;* SCR(s) 5252 :
;* Updated to use the Second IIC bus for Brazos.
;****************************************************************************

