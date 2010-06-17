;/****************************************************************************/
;/*                   CONEXANT PROPRIETARY AND CONFIDENTIAL                  */
;/*                       SOFTWARE FILE/MODULE HEADER                        */
;/*                    Conexant Systems Inc. (c) 1999-2003                   */
;/*                               Austin, TX                                 */
;/*                            All Rights Reserved                           */
;/****************************************************************************/
;/*
; * Filename:       serial.s
; *
; *
; * Description:    Low-level boot serial port support
; *
; *
; * Author:         Tim Ross
; *
; ****************************************************************************/
;/* $Header: serial.s, 20, 6/24/03 6:38:40 PM, Tim White$
; ****************************************************************************/

;******************
;* Include Files  *
;******************
    GET stbcfg.a
    GET serial.a

;**********************
;* Local Definitions  *
;**********************

; Equation is Div = (Freq/16*Baudrate)-1
SER_CLOCK_FREQ      EQU 54000000
SER_BAUDRATE_DIV    EQU (SER_CLOCK_FREQ/(16*DEFAULT_SERIAL_BAUD_RATE)-1)

;***************
;* Global Data *
;***************
    KEEP
    AREA StartupSerial, CODE, READONLY

;**************************************************************************************
;* IMPORTANT:                                                                         *
;*     All of the following code is completely RAM independent. This is crucial       *
;*     to its operation since it is called before RAM has been setup. Only registers  *
;*     r0-r3 are used to ensure it is callable from 'C' code.                         *
;**************************************************************************************

;********************************************************************
;*  InitUART                                                        *
;*                                                                  *
;*  DESCRIPTION:                                                    *
;*      Initialize the UART for outputting messages during startup. *
;*                                                                  *
;*  PARAMETERS:                                                     *
;*      None.                                                       *
;*                                                                  *
;*  RETURNS:                                                        *
;*      Nothing.                                                    *
;*                                                                  *
;*  REGISTER USAGE:                                                 *
;*      r0, r1, r2                                                  *
;*                                                                  *
;********************************************************************
    EXPORT InitUART
InitUART

    IF EMULATION_LEVEL = FINAL_HARDWARE    

    ;******************
    ;* Set BAUD rate. *
    ;******************
    ldr     r0, =SER_BAUDRATE_DIV

    ; Enable writing to divisor latch registers for 16550 type UART.
    ldr     r1, =SER_LCR
    mov     r2, #SER_DIV_LATCH_EN
    str     r2, [r1]

    ; Write low and high bytes of divisor latch.
    ldr     r1, =SER_DLL
    strb    r0, [r1]
    mov     r0, r0, lsr #8
    ldr     r1, =SER_DLH
    strb    r0, [r1]

    ;************************
    ;* Set line parameters. *
    ;************************
    mov     r0, #0
    orr     r0, r0, #SER_WORD_LEN_8
    orr     r0, r0, #SER_STOP_BITS_1
    orr     r0, r0, #SER_PARITY_DIS
    orr     r0, r0, #SER_BREAK_DIS
    ldr     r1, =SER_LCR
    str     r0, [r1]

    IF PLL_PIN_GPIO_MUX0_REG_DEFAULT = NOT_DETERMINED
    ldr     r1, =PLL_PIN_GPIO_MUX0_REG
    ldr     r0, [r1]

    orr     r0, r0, #GPIO_MUX0_UART_ENABLE
    str     r0, [r1]
    ENDIF   ;PLL_PIN_GPIO_MUX0_REG_DEFAULT = NOT_DETERMINED

    ENDIF   ;EMULATION_LEVEL = FINAL_HARDWARE

    bx      lr

;********************************************************************
;*  WriteSerial                                                     *
;*                                                                  *
;*  DESCRIPTION:                                                    *
;*      Output string to serial port.                               *
;*                                                                  *
;*  PARAMETERS:                                                     *
;*      r0 - ptr to null-terminated string to output.               *
;*                                                                  *
;*  RETURNS:                                                        *
;*      Nothing.                                                    *
;*                                                                  *
;*  REGISTER USE:                                                   *
;*      r0 - character ptr                                          *
;*      r1 - working register                                       *
;*      r2 - character value                                        *
;********************************************************************
    EXPORT WriteSerial
WriteSerial

    IF EMULATION_LEVEL = FINAL_HARDWARE :LAND: DEBUG

    ;*************************************
    ;* Check for NULL end of string and  *
    ;* advance ptr to next character.    *
    ;*************************************
1   ldrb    r2, [r0], #1
    cmp     r2, #0
    beq     %FT0
    
    ;*****************************************
    ;* Check the UART to see if it is empty. *
    ;*****************************************
2   ldr     r1, =SER_LSR
    ldrb    r1, [r1]
    
    ;******************************************************
    ;* We check the FIFO threshold because                *
    ;* the TX empty bit (bit 6) in the status register    *
    ;* does not seem the properly indicate that           *
    ;* the FIFO has room for a character.                 *
    ;******************************************************
    tst     r1, #SER_TX_FIFO_LOW
    beq     %BT2

    ;****************************
    ;* Write the character out. *
    ;****************************
    ldr     r1, =SER_THR
    strb    r2, [r1]

    ;****************************
    ;* Go on to next character. *
    ;****************************
    b       %BT1

    ENDIF ;EMULATION_LEVEL = FINAL_HARDWARE

0   bx      lr

    END

;/***************************************************************************
;* Modifications:
;* $Log: 
;*  20   mpeg      1.19        6/24/03 6:38:40 PM     Tim White       SCR(s) 
;*        6831 :
;*        Add flash, hsdp, demux, OSD, and demod support to codeldrext.
;*        
;*        
;*  19   mpeg      1.18        5/6/03 1:19:22 PM      Craig Dry       SCR(s) 
;*        5521 :
;*        Conditionally remove access to GPIO Pin Mux Register 0.
;*        
;*  18   mpeg      1.17        5/5/03 5:06:22 PM      Tim White       SCR(s) 
;*        6172 :
;*        Remove duplicate low-level boot support code and use startup 
;*        directory for building
;*        codeldr.  Remove 7 segment LED support.
;*        
;*        
;*  17   mpeg      1.16        12/10/02 2:31:00 PM    Craig Dry       SCR(s) 
;*        5070 :
;*        Changed conditional assembly statements to work properly.  
;*        "FINAL_HARDWARE" was changed to FINAL_HARDWARE, without the quotes.
;*        
;*  16   mpeg      1.15        5/20/02 5:45:44 PM     Bobby Bradford  SCR(s) 
;*        3834 :
;*        Made AREA directive conditional to remove INTERWORK warning
;*        
;*  15   mpeg      1.14        4/30/02 1:11:40 PM     Billy Jackman   SCR(s) 
;*        3656 :
;*        Changed GET hwconfig.a to GET stbcfg.a to conform to new 
;*        configuration.
;*        
;*  14   mpeg      1.13        1/16/02 10:22:08 AM    Quillian Rutherford 
;*        SCR(s) 3028 :
;*        Added a get for serial.s, only needs this at the moment for builds by
;*        vendor C
;*        
;*  13   mpeg      1.12        11/1/01 2:18:36 PM     Bobby Bradford  SCR(s) 
;*        2828 :
;*        Replaced GET sabine.a with GET hwconfig.a
;*        
;*        
;*  12   mpeg      1.11        7/3/01 11:07:08 AM     Tim White       SCR(s) 
;*        2178 2179 2180 :
;*        Merge branched Hondo specific code back into the mainstream source 
;*        database.
;*        
;*        
;*  11   mpeg      1.10        2/2/01 5:03:38 PM      Angela          Merged 
;*        Vendor_c changes into the code( mostly #if CUSTOMER==VENDOR_C blocks)
;*        See DCS#1049
;*        
;*  10   mpeg      1.9         1/9/01 4:40:20 PM      Tim Ross        DCS826
;*        
;*  9    mpeg      1.8         6/21/00 5:33:28 PM     Tim White       Added 
;*        Colorado Rev_C changes (pin muxing changes).
;*        
;*  8    mpeg      1.7         4/26/00 12:31:40 AM    Tim White       Colorado 
;*        uses UART3.
;*        
;*  7    mpeg      1.6         1/7/00 9:46:16 AM      Dave Wilson     Changed 
;*        some return instructions to be thumb compliant
;*        
;*  6    mpeg      1.5         10/19/99 3:29:48 PM    Tim Ross        Removed 
;*        SABINE and PID board dependencies.
;*        Made compatible with CN8600 CFG file.
;*        
;*  5    mpeg      1.4         10/6/99 11:37:00 AM    Tim Ross        Corrected
;*         build problem with SABINE version of WriteSerial 
;*        function.
;*        
;*  4    mpeg      1.3         9/22/99 2:34:44 PM     Tim Ross        Corrected
;*         bug that was causing checkpoint messages to overrun
;*        UART FIFO.
;*        
;*  3    mpeg      1.2         7/19/99 4:28:58 PM     Tim Ross        Corrected
;*         bug that was overwriting character value.
;*        
;*  2    mpeg      1.1         6/14/99 4:02:30 PM     Tim Ross        Removed 
;*        serial port checkpoint output from emulation.
;*        
;*  1    mpeg      1.0         6/8/99 12:52:46 PM     Tim Ross        
;* $
   
      Rev 1.19   24 Jun 2003 17:38:40   whiteth
   SCR(s) 6831 :
   Add flash, hsdp, demux, OSD, and demod support to codeldrext.
   
   
      Rev 1.18   06 May 2003 12:19:22   dryd
   SCR(s) 5521 :
   Conditionally remove access to GPIO Pin Mux Register 0.
   
      Rev 1.17   05 May 2003 16:06:22   whiteth
   SCR(s) 6172 :
   Remove duplicate low-level boot support code and use startup directory for building
   codeldr.  Remove 7 segment LED support.
   
;*
;***** FILE COPIED FROM CODELDR ******
;* vlog -br K:\sabine\pvcs\CODELDR\serial.s_v(serial.s)
;*
;* -----------------------------------
;* Rev 1.21
;* Checked in:     19 Dec 2002 08:46:54
;* Last modified:  18 Dec 2002 16:03:28
;* Author id: whiteth     lines deleted/added/moved: 2/1/0
;* SCR(s) 5068 :
;* Currently the emulation model does not have an external serial port, so it must be
;* disabled (correctly).
;* 
;* -----------------------------------
;* Rev 1.20
;* Checked in:     27 Aug 2002 10:50:28
;* Last modified:  27 Aug 2002 10:40:38
;* Author id: wangl2     lines deleted/added/moved: 1/1/0
;* SCR(s) 4247 :
;* Initialize UART even if DEBUG==NO.
;* -----------------------------------
;* Rev 1.19
;* Checked in:     01 May 2002 10:45:30
;* Last modified:  01 May 2002 10:06:24
;* Author id: jackmaw     lines deleted/added/moved: 1/1/0
;* SCR(s) 3670 :
;* Changed GET hwconfig.a to GET stbcfg.a to conform to new configuration.
;* -----------------------------------
;* Rev 1.18
;* Checked in:     15 Jan 2002 17:16:24
;* Last modified:  15 Jan 2002 14:45:00
;* Author id: rutherq     lines deleted/added/moved: 6/2/0
;* SCR(s) 3028 :
;* Added Uart selection to serial.s 
;* -----------------------------------
;* Rev 1.17
;* Checked in:     11 Jan 2002 17:16:42
;* Last modified:  11 Jan 2002 17:11:22
;* Author id: mooreda     lines deleted/added/moved: 1/2/8
;* SCR(s) 3006 :
;* Changed serial schema (again).
;* 
;* -----------------------------------
;* Rev 1.16
;* Checked in:     09 Jan 2002 16:42:34
;* Last modified:  09 Jan 2002 16:41:10
;* Author id: mooreda     lines deleted/added/moved: 0/6/0
;* SCR(s) 3006 :
;* Switch uart based upon SERIAL1 definition in CFG file.
;* 
;* -----------------------------------
;* Rev 1.15
;* Checked in:     05 Dec 2001 09:37:02
;* Last modified:  28 Nov 2001 08:44:24
;* Author id: bradforw     lines deleted/added/moved: 13/0/0
;* SCR(s) 2933 :
;* Removed Colorado RevA/RevB Specific Code
;* 
;* -----------------------------------
;* Rev 1.14
;* Checked in:     01 Nov 2001 14:12:14
;* Last modified:  29 Oct 2001 16:43:52
;* Author id: bradforw     lines deleted/added/moved: 6/6/0
;* SCR(s) 2828 :
;* Replaced GET sabine.a with GET hwconfig.a
;* Replaced EMULATION_LEVEL checks with NUMERIC equivalents
;* 
;* -----------------------------------
;* Rev 1.13
;* Checked in:     09 Jul 2001 17:06:10
;* Last modified:  06 Jul 2001 15:02:22
;* Author id: whiteth     lines deleted/added/moved: 3/0/0
;* SCR(s) 2234 2235 2236 :
;* Changes to allow Hondo to function.
;* 
;* -----------------------------------
;* Rev 1.12
;* Checked in:     03 Jul 2001 09:32:12
;* Last modified:  02 Jul 2001 14:42:40
;* Author id: whiteth     lines deleted/added/moved: 0/0/0
;* SCR(s) 2178 2179 2180 :
;* Merge branched Hondo specific code back into the mainstream source database.
;* 
;* -----------------------------------
;* Rev 1.11
;* Checked in:     02 Jul 2001 14:54:00
;* Last modified:  02 Jul 2001 14:42:40
;* Author id: prattac     lines deleted/added/moved: 0/3/0
;* SCR(s) 1642 :
;* Merging in changes for Hondo.  Fixed an error where hondo changes
;* did not compile for Colorado.
;* 
;* -----------------------------------
;* Rev 1.10
;* Checked in:     09 Jan 2001 16:39:22
;* Last modified:  09 Jan 2001 16:39:16
;* Author id: rossst     lines deleted/added/moved: 2/2/0
;* Branches:  1.10.1
;* DCS826
;* -----------------------------------
;*    Rev 1.10.1.1
;*    Checked in:     03 May 2001 18:05:42
;*    Last modified:  03 May 2001 18:04:18
;*    Author id: prattac     lines deleted/added/moved: 1/1/0
;*    Make sure code will compile for cx2249x.cfg
;* -----------------------------------
;*    Rev 1.10.1.0
;*    Checked in:     03 May 2001 17:42:28
;*    Last modified:  03 May 2001 17:35:26
;*    Author id: prattac     lines deleted/added/moved: 1/8/0
;*    Added support for Hondo, including PLL_HACK work-around for 13.5 MHz crystal.
;* -----------------------------------
;* Rev 1.9
;* Checked in:     09 Jan 2001 16:08:52
;* Last modified:  04 Jan 2001 15:50:06
;* Author id: rossst     lines deleted/added/moved: 4/0/0
;* DCS914
;* -----------------------------------
;* Rev 1.8
;* Checked in:     05 Sep 2000 10:07:06
;* Last modified:  01 Sep 2000 18:22:54
;* Author id: whiteth     lines deleted/added/moved: 3/3/0
;* Updated for Rev C pin mux changes.
;* -----------------------------------
;* Rev 1.7
;* Checked in:     21 Jun 2000 16:26:42
;* Last modified:  19 Jun 2000 10:29:20
;* Author id: whiteth     lines deleted/added/moved: 4/21/0
;* Added Colorado Rev_C changes (pin muxing changes).
;* -----------------------------------
;* Rev 1.6
;* Checked in:     25 Apr 2000 23:22:16
;* Last modified:  25 Apr 2000 23:20:48
;* Author id: whiteth     lines deleted/added/moved: 2/2/0
;* Colorado uses UART3.
;* -----------------------------------
;* Rev 1.5
;* Checked in:     25 Apr 2000 22:37:46
;* Last modified:  25 Apr 2000 22:37:28
;* Author id: whiteth     lines deleted/added/moved: 1/8/0
;* Enable UART's
;* -----------------------------------
;* Rev 1.4
;* Checked in:     14 Oct 1999 10:55:12
;* Last modified:  12 Oct 1999 19:09:26
;* Author id: rossst     lines deleted/added/moved: 55/15/0
;* Removed Sabine code.
;* -----------------------------------
;* Rev 1.3
;* Checked in:     06 Oct 1999 10:34:42
;* Last modified:  06 Oct 1999 10:34:20
;* Author id: rossst     lines deleted/added/moved: 10/16/0
;* Corrected build problem with SABINE version of WriteSerial 
;* function.
;* -----------------------------------
;* Rev 1.2
;* Checked in:     22 Sep 1999 13:34:02
;* Last modified:  22 Sep 1999 13:32:38
;* Author id: rossst     lines deleted/added/moved: 9/15/8
;* Corrected bug that was causing checkpoint messages to overrun
;* UART FIFO.
;* -----------------------------------
;* Rev 1.1
;* Checked in:     14 Jun 1999 15:02:46
;* Last modified:  14 Jun 1999 14:39:28
;* Author id: rossst     lines deleted/added/moved: 1/9/0
;* Removed serial port checkpoint output from emulation.
;* -----------------------------------
;* Rev 1.0
;* Checked in:     08 Jun 1999 11:26:40
;* Last modified:  18 May 1999 15:18:00
;* Author id: rossst     lines deleted/added/moved: 0/0/0
;* Initial revision.
;* ===================================
;****************************************************************************

