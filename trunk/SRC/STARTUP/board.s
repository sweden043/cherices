;****************************************************************************
;*                   CONEXANT PROPRIETARY AND CONFIDENTIAL                  *
;*                       SOFTWARE FILE/MODULE HEADER                        *
;*                    Conexant Systems Inc. (c) 1999-2003                   *
;*                                Austin, TX                                *
;*                           All Rights Reserved                            *
;****************************************************************************
;*                                                                          *
;* Filename:       board.s                                                  *
;*                                                                          *
;* Description:    Board specific routines used during startup sequence.    *
;*                                                                          *
;* Author:         Tim Ross                                                 *
;*                                                                          *
;****************************************************************************
;* $Header: board.s, 24, 3/19/04 12:03:34 AM, Xiao Guang Yan$
;****************************************************************************

;******************************************************
;* Conexant (i.e. Conexant Chip-Based) Board Routines *
;******************************************************

;******************
;* Include Files  *
;******************
    GET stbcfg.a

    INCLUDE lcds.a
    INCLUDE board.a

;********************************
;* External Function Prototypes *
;********************************
    IMPORT InitUART
    IMPORT WriteSerial
    IMPORT InitLCDs
    IMPORT WriteLCDs
    IMPORT ClearI2CBus

;**********************
;* Local Definitions  *
;**********************

;***************
;* Global Data *
;***************


    KEEP
    IF :DEF: |ads$version|
    AREA StartupBoard, CODE, READONLY
    ELSE
    AREA StartupBoard, CODE, READONLY, INTERWORK
    ENDIF


;********************************************************************
;*  InitCheckpointOutput                                            *
;*                                                                  *
;*  PARAMETERS:                                                     *
;*      None.                                                       *
;*                                                                  *
;*  DESCRIPTION:                                                    *
;*      Initializes hardware used for outputting checkpoints.       *
;*                                                                  *
;*  RETURNS:                                                        *
;*      Nothing.                                                    *
;*                                                                  *
;*  REGISTER USAGE:                                                 *
;*      r0, r1, r2, r3, r12                                         *
;*                                                                  *
;********************************************************************
    EXPORT InitCheckpointOutput
InitCheckpointOutput
    mov     r12, lr

    ;Ensure I2C bus(es) are not stuck!
    mov     r0, #0
    bl      ClearI2CBus
    IF (IIC_TYPE <> IIC_TYPE_COLORADO :LAND: IIC_TYPE <> IIC_TYPE_WABASH)
    mov     r0, #1
    bl      ClearI2CBus
    ENDIF ; IIC_TYPE

    IF SERIAL1 = INTERNAL_UART1 :LOR: SERIAL1 = INTERNAL_UART2 :LOR: SERIAL1 = INTERNAL_UART3
        ; Initialize serial port
        bl      InitUART        ; Uses r0, r1, r2
    ENDIF   ; Serial Port Check

    IF CUSTOMER = "CNXT"
     IF EMULATION_LEVEL = FINAL_HARDWARE
      IF (FRONT_PANEL_KEYPAD_TYPE <> FRONT_PANEL_KEYPAD_BRADY) :LAND: \
         (FRONT_PANEL_KEYPAD_TYPE <> FRONT_PANEL_KEYPAD_BURNET) :LAND: \
         (FRONT_PANEL_KEYPAD_TYPE <> FRONT_PANEL_KEYPAD_PUDONG)

        ; Initialise and clear front panel 20x4 LCD matrix display
        bl      InitLCDs        ; Uses r0, r1, r2, r3

      ENDIF   ; (FRONT_PANEL_KEYPAD_TYPE <> FRONT_PANEL_KEYPAD_BURNET)  :LAND:
         	  ; (FRONT_PANEL_KEYPAD_TYPE <> FRONT_PANEL_KEYPAD_BRADY) :LAND:
         	  ; (FRONT_PANEL_KEYPAD_TYPE <> FRONT_PANEL_KEYPAD_PUDONG)
         	  
     ENDIF    ; EMULATION_LEVEL <> FINAL_HARDWARE
    ENDIF     ; CUSTOMER = CNXT

    ; Return to Calling Routine
    bx      r12


;********************************************************************
;*  Checkpoint                                                      *
;*                                                                  *
;*  DESCRIPTION:                                                    *
;*      Output checkpoint info to board-specific output ports.      *
;*                                                                  *
;*  PARAMETERS:                                                     *
;*      r0 - ptr to unlimited length, null-terminated string for    *
;*           output to more sophisticated device (i.e. serial       *
;*           terminal). If this is NULL, then skip serial output.   *
;*                                                                  *
;*  RETURNS:                                                        *
;*      Nothing.                                                    *
;*                                                                  *
;*  REGISTER USAGE:                                                 *
;*      r0, r1, r2, r3, r12                                         *
;*                                                                  *
;********************************************************************
    EXPORT Checkpoint
Checkpoint

    ;Save return address
    mov     r12, lr
    movs    r3, r0                     ; Save the string pointer                   

    ;If NULL string, return
    beq     %FT0

    IF SERIAL1 = INTERNAL_UART1 :LOR: SERIAL1 = INTERNAL_UART2 :LOR: SERIAL1 = INTERNAL_UART3

        ;**************************************
        ;* Output full string to serial port. *
        ;**************************************
        ldr     r1, =RST_SCRATCH_REG   ; Only output to serial if enabled.
        ldr     r2, [r1]
        tst     r2, #RST_SCRATCH_SERIAL_CKPT_ENABLE
        blne    WriteSerial            ; Uses r0, r1, r2

    ENDIF   ; Serial Port Check

    IF CUSTOMER = "CNXT" 
     IF EMULATION_LEVEL = FINAL_HARDWARE
      IF (FRONT_PANEL_KEYPAD_TYPE <> FRONT_PANEL_KEYPAD_BRADY) :LAND: \
         (FRONT_PANEL_KEYPAD_TYPE <> FRONT_PANEL_KEYPAD_BURNET)
    
        ;********************************************
        ;* Output number to 20x4 LCD matrix display *
        ;********************************************
        mov     r0, r3                 ; Retrieve the string pointer
        bl      WriteLCDs              ; Uses r0, r1, r2, r3

      ENDIF   ; (FRONT_PANEL_KEYPAD_TYPE <> FRONT_PANEL_KEYPAD_BURNET)  :LAND:
              ; (FRONT_PANEL_KEYPAD_TYPE <> FRONT_PANEL_KEYPAD_BRADY) 
     ENDIF    ; EMULATION_LEVEL <> FINAL_HARDWARE
    ENDIF     ; CUSTOMER = CNXT

    ;Return
0   bx      r12


;********************************************************************
;*  StartupFatalExit                                                *
;*                                                                  *
;*  DESCRIPTION:                                                    *
;*      This routine is called when a problem occurs that will      *
;*      prevent startup from continuing. We output a message using  *
;*      our checkpoint routine and loop forever.                    *
;*                                                                  *
;*  PARAMETERS:                                                     *
;*      r0 - ptr to unlimited length, null-terminated string for    *
;*           output to more sophisticated device (i.e. serial       *
;*           terminal)                                              *
;*                                                                  *
;*  RETURNS:                                                        *
;*      Does not return.                                            *
;*                                                                  *
;*  REGISTER USAGE:                                                 *
;*      r0, r1, r2, r3, r12                                         *
;*                                                                  *
;********************************************************************
    EXPORT StartupFatalExit
StartupFatalExit

    ;*********************************************************
    ;* Output error message details via checkpoint routines. *
    ;*********************************************************
    bl      Checkpoint

    ;*********
    ;* Halt. *
    ;*********
0   b       %BT0

    END

;****************************************************************************
;* Modifications:
;* $Log: 
;*  24   mpeg      1.23        3/19/04 12:03:34 AM    Xiao Guang Yan  CR(s) 
;*        8595 : Commented out InitLCDs for Pudong board.
;*        
;*  23   mpeg      1.22        11/1/03 2:57:14 PM     Tim Ross        CR(s): 
;*        7719 7762 Added capability disable checkpoint output to UART for 
;*        early codeldr.s routines.
;*  22   mpeg      1.21        7/10/03 10:35:08 AM    Larry Wang      SCR(s) 
;*        6924 :
;*        Change "0  B %T0" to "0  B %BT0".
;*        
;*  21   mpeg      1.20        5/5/03 5:05:56 PM      Tim White       SCR(s) 
;*        6172 :
;*        Remove duplicate low-level boot support code and use startup 
;*        directory for building
;*        codeldr.  Remove 7 segment LED support.
;*        
;*        
;*  20   mpeg      1.19        1/24/03 9:51:42 AM     Dave Moore      SCR(s) 
;*        5305 :
;*        removed FREQ_SCREEN
;*        
;*        
;*  19   mpeg      1.18        12/17/02 4:49:16 PM    Tim White       SCR(s) 
;*        5182 :
;*        Removed ARM_PIT_TYPE, no longer needed.
;*        Removed hwswitch.a, no longer needed.
;*        
;*        
;*  18   mpeg      1.17        10/30/02 2:47:04 PM    Bobby Bradford  SCR(s) 
;*        4866 :
;*        Added support for BURNET front panel buttons (single GPIO)
;*        
;*  17   mpeg      1.16        7/12/02 8:23:48 AM     Steven Jones    SCR(s): 
;*        4176 
;*        Fix for platforms without LCD front panels.
;*        
;*  16   mpeg      1.15        5/20/02 5:45:42 PM     Bobby Bradford  SCR(s) 
;*        3834 :
;*        Made AREA directive conditional to remove INTERWORK warning
;*        
;*  15   mpeg      1.14        4/30/02 1:11:14 PM     Billy Jackman   SCR(s) 
;*        3656 :
;*        Changed GET hwconfig.a to GET stbcfg.a to conform to new 
;*        configuration.
;*        
;*  14   mpeg      1.13        11/20/01 5:59:16 PM    Quillian Rutherford 
;*        SCR(s) 2754 :
;*        Changed to always use LCD display unless in emulation.
;*        
;*  13   mpeg      1.12        11/1/01 2:18:04 PM     Bobby Bradford  SCR(s) 
;*        2828 :
;*        Replaced INCLUDE sabine.a with INCLUDE hwconfig.a
;*        
;*        
;*  12   mpeg      1.11        7/3/01 11:07:04 AM     Tim White       SCR(s) 
;*        2178 2179 2180 :
;*        Merge branched Hondo specific code back into the mainstream source 
;*        database.
;*        
;*        
;*  11   mpeg      1.10        12/14/00 6:20:16 PM    Tim White       Removed 
;*        startup debug for release builds.
;*        
;*  10   mpeg      1.9         6/21/00 5:33:24 PM     Tim White       Added 
;*        Colorado Rev_C changes (pin muxing changes).
;*        
;*  9    mpeg      1.8         5/1/00 11:00:08 PM     Tim White       Added 
;*        bringup #if's for Vendor_B.
;*        
;*  8    mpeg      1.7         4/24/00 11:21:12 AM    Tim White       Moved 
;*        strapping CONFIG0 from chip header file(s) to board header file(s).
;*        
;*  7    mpeg      1.6         4/7/00 3:08:46 PM      Dave Wilson     Changes 
;*        to allow STARTUP to work on Klondike boards
;*        
;*  6    mpeg      1.5         1/6/00 10:41:12 AM     Dave Wilson     Changes 
;*        for ARM/Thumb builds
;*        
;*  5    mpeg      1.4         11/1/99 2:36:14 PM     Tim Ross        Added 
;*        "Manual" option to frequency scanning.
;*        
;*  4    mpeg      1.3         10/19/99 3:28:48 PM    Tim Ross        Removed 
;*        SABINE and PID board dependencies.
;*        Made compatible with CN8600 CFG file.
;*        
;*  3    mpeg      1.2         10/14/99 6:57:42 PM    Tim White       Allow 
;*        FREQ_SCREEN to be undefined w/o compile error.
;*        
;*  2    mpeg      1.1         6/8/99 6:16:02 PM      Tim Ross        Changed 
;*        codeldr.a to hwswitch.a.
;*        
;*  1    mpeg      1.0         6/8/99 12:53:34 PM     Tim Ross        
;* $
   
      Rev 1.21   10 Jul 2003 09:35:08   wangl2
   SCR(s) 6924 :
   Change "0  B %T0" to "0  B %BT0".
   
      Rev 1.20   05 May 2003 16:05:56   whiteth
   SCR(s) 6172 :
   Remove duplicate low-level boot support code and use startup directory for building
   codeldr.  Remove 7 segment LED support.
   
;* 
;*    Rev 1.26   20 Jan 2003 11:57:38   velusws
;* SCR(s) 5252 :
;* Modified to use the I2C Device Addresses from hwconfig.cfg instead of board.a
;* 
;*    Rev 1.25   20 Jan 2003 11:13:26   velusws
;* SCR(s) 5252 :
;* Updated to use the Second IIC bus for Brazos.
;*
;***** FILE COPIED FROM CODELDR ******
;* vlog -br K:\sabine\pvcs\CODELDR\board.s_v(board.s)
;* 
;* -----------------------------------
;* Rev 1.26
;* Checked in:     20 Jan 2003 11:57:38
;* Last modified:  20 Jan 2003 11:57:38
;* Author id: velusws     lines deleted/added/moved: 55/59/0
;* SCR(s) 5252 :
;* Modified to use the I2C Device Addresses from hwconfig.cfg instead of board.a
;* -----------------------------------
;* Rev 1.25
;* Checked in:     20 Jan 2003 11:13:26
;* Last modified:  20 Jan 2003 11:13:26
;* Author id: velusws     lines deleted/added/moved: 1/73/0
;* SCR(s) 5252 :
;* Updated to use the Second IIC bus for Brazos.
;* -----------------------------------
;* Rev 1.24
;* Checked in:     17 Dec 2002 14:41:24
;* Last modified:  17 Dec 2002 10:02:52
;* Author id: whiteth     lines deleted/added/moved: 1/0/0
;* SCR(s) 5182 :
;* Remove hwswitch.a include/get.
;* 
;* -----------------------------------
;* Rev 1.23
;* Checked in:     12 Dec 2002 17:23:10
;* Last modified:  12 Dec 2002 15:40:56
;* Author id: whiteth     lines deleted/added/moved: 2/2/0
;* SCR(s) 5157 :
;* Use IIC_TYPE <> IIC_TYPE_COLORADO instead of IIC_TYPE = IIC_TYPE_WABASH for 
;* future chip support as the default case (i.e. Brazos).
;* 
;* -----------------------------------
;* Rev 1.22
;* Checked in:     30 Oct 2002 13:39:32
;* Last modified:  30 Oct 2002 13:21:34
;* Author id: bradforw     lines deleted/added/moved: 0/8/0
;* SCR(s) 4866 :
;* Add support for BURNET keypad (single GPIO button)
;* -----------------------------------
;* Rev 1.21
;* Checked in:     04 Sep 2002 14:17:06
;* Last modified:  04 Sep 2002 14:01:12
;* Author id: velusws     lines deleted/added/moved: 0/2/0
;* SCR(s) 4520 :
;* Added Debug output test check before call to Serial Initialize for Conexant IRDs.
;* -----------------------------------
;* Rev 1.20
;* Checked in:     03 Sep 2002 18:24:36
;* Last modified:  03 Sep 2002 18:22:58
;* Author id: velusws     lines deleted/added/moved: 1/1/0
;* SCR(s) 4502 :
;* Replaced extra LCD char needed on Wabash
;* -----------------------------------
;* Rev 1.19
;* Checked in:     03 Sep 2002 17:33:30
;* Last modified:  03 Sep 2002 17:18:12
;* Author id: velusws     lines deleted/added/moved: 4/21/0
;* SCR(s) 4502 :
;* Changes to use the new IIC interface
;* -----------------------------------
;* Rev 1.18
;* Checked in:     16 Jul 2002 12:54:44
;* Last modified:  16 Jul 2002 12:49:42
;* Author id: banghag     lines deleted/added/moved: 3/10/0
;* SCR(s): 4202 
;* added feature FRONT_PANEL_KEYPAD_BRADY to remove the LCD interface fro
;* for the brady platfrom
;* -----------------------------------
;* Rev 1.17
;* Checked in:     01 May 2002 10:46:00
;* Last modified:  01 May 2002 10:07:22
;* Author id: jackmaw     lines deleted/added/moved: 1/1/0
;* SCR(s) 3670 :
;* Changed GET hwconfig.a to GET stbcfg.a to conform to new configuration.
;* -----------------------------------
;* Rev 1.16
;* Checked in:     15 Jan 2002 17:19:04
;* Last modified:  15 Jan 2002 17:06:48
;* Author id: rutherq     lines deleted/added/moved: 2/4/0
;* SCR(s) 3028 :
;* Changed to only call InitUART when using one of the internal UARTs.
;* 
;* -----------------------------------
;* Rev 1.15
;* Checked in:     18 Dec 2001 11:52:18
;* Last modified:  18 Dec 2001 10:19:42
;* Author id: bradforw     lines deleted/added/moved: 13/5/0
;* SCR(s) 2933 :
;* Incorporating WabashBranch changes
;* -----------------------------------
;* Rev 1.14
;* Checked in:     05 Dec 2001 09:37:36
;* Last modified:  04 Dec 2001 17:00:26
;* Author id: bradforw     lines deleted/added/moved: 164/0/0
;* Branches:  1.14.1
;* SCR(s) 2933 :
;* Removed IsKlondikeIRD, IsAbileneIRD and UsesScanMatrix functions (no longer
;* using these functions for run-time feature switching)
;* 
;* -----------------------------------
;*     Rev 1.14.1.0
;*     Checked in:     10 Dec 2001 17:53:06
;*     Last modified:  10 Dec 2001 12:34:12
;*     Author id: bradforw     lines deleted/added/moved: 13/5/0
;*     SCR(s) 2933 :
;*     Changes to support new PLL.S code
;* -----------------------------------
;* Rev 1.13
;* Checked in:     06 Nov 2001 14:58:38
;* Last modified:  06 Nov 2001 14:51:54
;* Author id: bradforw     lines deleted/added/moved: 23/43/3
;* SCR(s) 2753 :
;* Modified code so that LED display is always intitialized for CNXT / Emulation
;* and LCD display is intitialized for CNXT / FINAL and Serial output is always
;* intitialized for everything else, and sometimes for CNXT
;* -----------------------------------
;* Rev 1.12
;* Checked in:     01 Nov 2001 14:13:34
;* Last modified:  29 Oct 2001 15:50:56
;* Author id: bradforw     lines deleted/added/moved: 1/2/0
;* SCR(s) 2828 :
;* Replaced GET sabine.a with GET hwconfig.a
;* 
;* -----------------------------------
;* Rev 1.11
;* Checked in:     03 May 2001 17:36:40
;* Last modified:  22 Apr 2001 17:20:02
;* Author id: prattac     lines deleted/added/moved: 8/62/0
;* Added support for Abilene.
;* -----------------------------------
;* Rev 1.10
;* Checked in:     11 Dec 2000 17:27:28
;* Last modified:  11 Dec 2000 16:35:52
;* Author id: dawilson     lines deleted/added/moved: 0/52/0
;* DCS 846: Updates to allow use with new vendor D board. Holding any front 
;* panel key causes box to enter multi-ice download mode.
;* -----------------------------------
;* Rev 1.9
;* Checked in:     01 May 2000 21:55:46
;* Last modified:  01 May 2000 19:10:14
;* Author id: whiteth     lines deleted/added/moved: 1/13/0
;* Added #if's for Vendor_B bringup.
;* -----------------------------------
;* Rev 1.8
;* Checked in:     24 Apr 2000 10:12:22
;* Last modified:  20 Apr 2000 17:27:20
;* Author id: whiteth     lines deleted/added/moved: 4/5/0
;* Moved strapping CONFIG0 from chip header file(s) to board header file(s).
;* -----------------------------------
;* Rev 1.7
;* Checked in:     06 Apr 2000 09:27:24
;* Last modified:  05 Apr 2000 20:12:08
;* Author id: dawilson     lines deleted/added/moved: 15/163/6
;* Changes to make CODELDR run on a Klondike board - check board ID and use LCD
;* or LED display as appropriate, GPIO buttons or scan matrix (as yet to be
;* completed).
;* -----------------------------------
;* Rev 1.6
;* Checked in:     08 Mar 2000 18:56:26
;* Last modified:  08 Mar 2000 18:25:54
;* Author id: rossst     lines deleted/added/moved: 7/9/0
;* Converted CHeckpoint() to output text strings to LEDs instead of
;* -----------------------------------
;* Rev 1.5
;* Checked in:     01 Nov 1999 14:34:42
;* Last modified:  27 Oct 1999 22:03:04
;* Author id: rossst     lines deleted/added/moved: 1/1/0
;* Added "Manual" option to frequency scanning.
;* -----------------------------------
;* Rev 1.4
;* Checked in:     14 Oct 1999 10:48:18
;* Last modified:  06 Oct 1999 17:06:08
;* Author id: rossst     lines deleted/added/moved: 134/1/0
;* Removed Sabine code.
;* -----------------------------------
;* Rev 1.3
;* Checked in:     24 Jun 1999 13:38:26
;* Last modified:  24 Jun 1999 13:27:08
;* Author id: rossst     lines deleted/added/moved: 91/102/0
;* Expanded input parameters to checkpoint routines to allow different
;* output devices to be turned on/off.
;* -----------------------------------
;* Rev 1.2
;* Checked in:     08 Jun 1999 17:14:08
;* Last modified:  08 Jun 1999 17:14:06
;* Author id: rossst     lines deleted/added/moved: 1/1/0
;* Changed codeldr.a to hwswitch.a.
;* -----------------------------------
;* Rev 1.1
;* Checked in:     08 Jun 1999 17:12:50
;* Last modified:  08 Jun 1999 17:10:26
;* Author id: rossst     lines deleted/added/moved: 1/1/0
;* Changed codeldr.a to hwswitch.a.
;* -----------------------------------
;* Rev 1.0
;* Checked in:     08 Jun 1999 11:27:54
;* Last modified:  18 May 1999 15:15:12
;* Author id: rossst     lines deleted/added/moved: 0/0/0
;* Initial revision.
;* ===================================
;****************************************************************************

