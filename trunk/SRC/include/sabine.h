/****************************************************************************/
/*                 Conexant Systems Incorporated - CN8600                   */
/****************************************************************************/
/*                                                                          */
/* Filename:           SABINE.H                                             */
/*                                                                          */
/* Description:        Public header file defining hardware-specific values */
/*                     (such as register addresses, bit definitions, etc)   */
/*                                                                          */
/* Author:             Dave Wilson                                          */
/*                                                                          */
/* Copyright Conexant Systems Inc, 1999                                     */
/* All Rights Reserved.                                                     */
/*                                                                          */
/****************************************************************************/
/*
$Header: sabine.h, 189, 8/22/02 4:01:54 PM, Larry Wang$
$Log: 
 189  mpeg      1.188       8/22/02 4:01:54 PM     Larry Wang      SCR(s) 4459 
       :
       Include wabash.h if CHIP_NAME==WABASH.
       
       
 188  mpeg      1.187       7/3/01 10:43:38 AM     Tim White       SCR(s) 2178 
       2179 2180 :
       Merge branched Hondo specific code back into the mainstream source 
       database.
       
       
 187  mpeg      1.186       2/1/01 1:09:32 PM      Tim Ross        DCS966.
       Added supprot for Hondo.
       
 186  mpeg      1.185       10/14/99 12:48:38 PM   Tim White       Added 
       hwconfig.h #include.
       
 185  mpeg      1.184       10/12/99 11:49:00 AM   Dave Wilson     Removed 
       everything. File is now a wrapper which includes neches.h or 
       colorado.h as appropriate.
       
 184  mpeg      1.183       10/7/99 1:36:28 PM     Dave Wilson     Added unions
        to describe registers which differ between Neches A nad B.
       
 183  mpeg      1.182       10/6/99 3:40:00 PM     Dave Wilson     Removed 
       references to FAKE_NVRAM buffer.
       
 182  mpeg      1.181       10/6/99 3:14:42 PM     Dave Wilson     Added 
       ROM_START_ADDRESS (moved from s/w config file).
       
 181  mpeg      1.180       9/30/99 11:28:44 AM    Senthil Veluswamy Added 
       missing NCR_BASE for PID7T builds
       
 180  mpeg      1.179       9/23/99 8:24:48 PM     Senthil Veluswamy defined 
       new bit in AFE CONTROL Reg.
       - FifoInteractBlk:1(AFE/DAA)
       
 179  mpeg      1.178       9/23/99 5:46:14 PM     Senthil Veluswamy No change.
       
 178  mpeg      1.177       9/23/99 3:04:16 PM     Dave Wilson     Fixed errors
        in some of the DMA macros
       
 177  mpeg      1.176       8/16/99 3:47:32 PM     Dave Wilson     Reversed 
       logic for some Neches B includes to ensure that they are included
       for future revs as well as rev B.
       
 176  mpeg      1.175       8/11/99 5:36:46 PM     Dave Wilson     Fixed a typo
        in Neches Rev B case.
       
 175  mpeg      1.174       8/4/99 4:03:38 PM      Dave Wilson     Changes for 
       Neches Rev B.
       
 174  mpeg      1.173       7/1/99 1:07:30 PM      Tim Ross        Added chip 
       ID definitions.
       Added SDR_CFG register definitions.
       
 173  mpeg      1.172       6/23/99 2:51:22 PM     Rob Tilton      Moved 
       VBIFetchCnt to DRM_VID_SIZE for neches.
       
 172  mpeg      1.171       6/23/99 11:45:44 AM    Dave Wilson     Added 
       definitions to support ALPHABOTHVIDEO for Neches.
       
 171  mpeg      1.170       6/18/99 4:23:42 PM     Lucy C Allevato Changed 
       PID7T defines for AFE_BASE and DMA_BASE to match what they
       are in the current FPGA peripherals. These should apply to both Sabine
       and Neches.
       
 170  mpeg      1.169       6/17/99 6:49:28 PM     Dave Wilson     Added 
       SER_NUM_BASE macro for Neches builds.
       
 169  mpeg      1.168       6/11/99 5:37:42 PM     Tim Ross        Changed 
       PAR_TIMEOUT to PAR_TIMEOUT_VAL to work
       around unexplained compilation problem.
       
 168  mpeg      1.167       6/11/99 5:19:46 PM     Dave Wilson     Replaced 
       PAR_TIMEOUT with PAR_TIMEOUTVAL since the original
       #define is already used by pSOS and therefore caused the build to 
       fail.
       
 167  mpeg      1.166       5/26/99 9:33:22 AM     Dave Wilson     Corrected 
       PLL_CONFIG0 for Sabine
       
 166  mpeg      1.165       5/7/99 1:28:36 PM      Steve Glennon   Added 
       defines for RTC divider for 86MHz bus clock
       
 165  mpeg      1.164       4/26/99 2:29:00 PM     Dave Wilson     Updated 
       PLL_MEM_CONFIG for Neches
       
 164  mpeg      1.163       4/26/99 9:36:34 AM     Dave Wilson     Updated EVID
        registers.
       
 163  mpeg      1.162       4/13/99 3:53:30 PM     Senthil Veluswamy Added 
       definition for new SmartCard Register : 
       SMINTQSTAT - Qualified Int Status Register.
       
 162  mpeg      1.161       4/9/99 4:22:30 PM      Tim Ross        Added 
       NUM_ISAROM_SLOTS
       
 161  mpeg      1.160       4/7/99 6:08:18 PM      Dave Wilson     Added 
       VBIFetchCnt to DRM_VID_STRIDE register. Field is applicable to 
       Neches and Sabine.
       
 160  mpeg      1.159       4/7/99 6:08:16 PM      Senthil Veluswamy No change.
       
 159  mpeg      1.158       4/6/99 7:28:26 PM      Tim Ross        Added 
       NUM_ISA_DESCRIPTORS.
       
 158  mpeg      1.157       4/6/99 5:30:42 PM      Dave Wilson     Fixed typo 
       in one of the MPEG interrupt source labels.
       Added audio sync bit to MPEG_CONTROL1_REG
       
 157  mpeg      1.156       4/6/99 11:32:58 AM     Senthil Veluswamy Added RTC 
       PLL values for 54MHZ -> 100Hz and 1KHz.
       
 156  mpeg      1.155       4/5/99 4:07:04 PM      Dave Wilson     Updates to 
       DRM, RST and SDR sections for Neches.
       
 155  mpeg      1.154       4/5/99 11:21:06 AM     Dave Wilson     Updated 
       PLL_CONFIG0 register for Neches.
       
 154  mpeg      1.153       4/1/99 3:02:30 PM      Tim Ross        Added 
       PCI_ROMWIDTH8, 16, and 32.
       
 153  mpeg      1.152       4/1/99 2:47:56 PM      Tim Ross        Added 
       ROM0_WIDTH_32_ALT.
       
 152  mpeg      1.151       3/25/99 2:00:24 PM     Anzhi Chen      Fixed a 
       spelling error on line 4754.
       
 151  mpeg      1.150       3/25/99 10:35:20 AM    Dave Wilson     Latest 
       collection of Neces changes - MPG and PLL area mostly.
       
 150  mpeg      1.149       3/24/99 2:28:06 PM     Dave Wilson     Added some 
       SDRAM definitions for Neches.
       
 149  mpeg      1.148       3/24/99 8:38:46 AM     Dave Wilson     Fixed GPIO 
       macros. Interrupt edge selection macos now do correct read-mod
       write and SET_GPIO_PIN and SET_GPIO_PIN_BANK now compile.
       
 148  mpeg      1.147       3/23/99 5:31:54 PM     Dave Wilson     Added dummy 
       NUM_GPI_BANKS and GPI_BANK_SIZE for Sabine to allow common
       code for Sabine and Neches in hwfuncs.c
       
 147  mpeg      1.146       3/23/99 4:49:02 PM     Anzhi Chen      Reversed the
        order of PLL_LOCK_STAT register 's stop and suspend bits.
       
 146  mpeg      1.145       3/23/99 3:43:22 PM     Anzhi Chen      Added two 
       bits in PLL resource lock status register. Changed unlock sequence.
       
 145  mpeg      1.144       3/23/99 3:37:36 PM     Senthil Veluswamy changed 
       SmartCard Base address for Pid Board.
       
 144  mpeg      1.143       3/22/99 10:19:02 AM    Dave Wilson     Added new 
       EVID register bits.
       
 143  mpeg      1.142       3/19/99 2:30:06 PM     Senthil Veluswamy Changed 
       PID Board SmartCard Base address
       
 142  mpeg      1.141       3/19/99 10:58:38 AM    Dave Wilson     Added 
       SMC_BANK_SIZE and NUM_SMC_BANKS for PID7T/Neches case.
       
 141  mpeg      1.140       3/19/99 10:25:48 AM    Dave Wilson     Minor 
       changes to allow Neches/PID7T version to compile.
       
 140  mpeg      1.139       3/17/99 10:02:40 AM    Dave Wilson     Added timer 
       interrupt enable bit to mode register (and other minor Neches
       updates).
       
 139  mpeg      1.138       3/16/99 5:04:46 PM     Dave Wilson     Latest 
       Neches changes.
       
 138  mpeg      1.137       3/15/99 8:38:40 AM     Dave Wilson     Corrected a 
       PLL RTC divider value.
       
 137  mpeg      1.136       3/11/99 11:54:32 AM    Senthil Veluswamy removed 
       bug: changed Neches RAW STAT Register name to 
       - RAW DAT Reg.
       
 136  mpeg      1.135       3/9/99 4:08:22 PM      Senthil Veluswamy added 
       defenitions for Smart Card FD & CONFIG Regs
       
 135  mpeg      1.134       3/9/99 1:44:04 PM      Senthil Veluswamy updated 
       offset for SMC Data reg
       changed CLK DIV Reg bit allocations to conform to funtionality
       changed Neches SMC RAW STAT Reg (name) to be as corresponding
       - register in Sabine
       Added defenition for Neches INT Mask Reg 
       
 134  mpeg      1.133       3/8/99 3:07:38 PM      Dave Wilson     Corrected 
       typos in a couple of neches regs introduced on last revision.
       
 133  mpeg      1.132       3/8/99 3:04:54 PM      Dave Wilson     Continuing 
       additions and changes for Neches.
       
 132  mpeg      1.131       3/8/99 10:27:24 AM     Dave Wilson     A couple of 
       audio updates for Neches.
       
 131  mpeg      1.130       3/8/99 9:39:36 AM      Dave Wilson     Updated DMA 
       control register definition.
       
 130  mpeg      1.129       3/5/99 10:25:16 AM     Dave Wilson     Further 
       corrections to Neches additions.
       
 129  mpeg      1.128       3/4/99 5:09:14 PM      Dave Wilson     Fixed 
       CHANEL/CHANNEL typos in DMA section 
       
 128  mpeg      1.127       3/4/99 3:36:18 PM      Dave Wilson     Changes for 
       Neches.
       
 127  mpeg      1.126       3/3/99 10:00:12 AM     Dave Wilson     Added all 
       register changes for Neches.
       
 126  mpeg      1.125       1/14/99 5:41:30 PM     Senthil Veluswamy restored 
       PLL values for running RTC at 100Hz.
       
 125  mpeg      1.124       1/8/99 2:00:00 PM      Senthil Veluswamy modified 
       PLL value to clock RTC at 1000Hz. 
       Added PLL value for MemCLK running at 92MHz. 
       removed PLL values for RTC running at 100Hz.
       
 124  mpeg      1.123       1/7/99 5:36:28 PM      Rob Tilton      Added some 
       macros for setting and clearing flags.
       
 123  mpeg      1.122       1/5/99 12:37:16 PM     Senthil Veluswamy Defined 
       new RTC PLL value to clock RTC at 1000Hz
       
 122  mpeg      1.121       12/18/98 2:01:50 PM    Rob Tilton      Added 
       teletex register definitions.
       
 121  mpeg      1.120       12/2/98 10:45:04 AM    Tim Ross        Added more 
       PLL setting definitions.
       
 120  mpeg      1.119       11/19/98 2:01:50 PM    Tim Ross        Corrected a 
       bug with calculating the address of serial registers
       for secondary and beyond serial ports.
       
 119  mpeg      1.118       11/12/98 5:01:36 PM    Dave Wilson     Took out 
       yesterday's change. It seems Tim Litch doesn't know about his
       UARTs after all.
       
 118  mpeg      1.117       11/11/98 11:40:14 AM   Dave Wilson     Fixed 
       SER_BANK_SIZE which should have been 64K but was 256 bytes instead.
       
 117  mpeg      1.116       11/10/98 2:56:06 AM    Amy Pratt       Added 
       definitions for audio board options.
       
 116  mpeg      1.115       10/20/98 4:11:40 PM    Senthil Veluswamy changed 
       SMC_RXTIME.mSeconds to SMC_RXTIME.Cycles
       
 115  mpeg      1.114       10/19/98 6:52:26 PM    Tim Ross        Added PCI IO
        space definition.
       
 114  mpeg      1.113       10/8/98 12:00:18 PM    Senthil Veluswamy Added 
       BitFields AtrStartTimeOut and AtrDurationTimeOut to
       SMC_INT_STAT. Reserved bits now down to 21.
       
 113  mpeg      1.112       10/6/98 4:07:08 PM     Dave Wilson     For 
       PID7T-based test board, changed number of SMC banks to 1.
       
 112  mpeg      1.111       10/6/98 3:54:50 PM     Dave Wilson     Added new 
       base address for smartcard regs when using the PID7T-based test
       board.
       
 111  mpeg      1.110       10/1/98 12:35:48 PM    Dave Wilson     Chhanged a 
       couple of fields in smart card registers.
       
 110  mpeg      1.109       9/30/98 6:49:08 PM     Lucy C Allevato Updated 
       IEEE1284 section to match up with latest spec. Some of the
       registers are no longer there. Fixed some apparent global search/replace
       errors in the UART and 1284 sections where BYTE and WORD used in a 
       #define
       got replaced with u_int8 and u_int16.
       
 109  mpeg      1.108       9/30/98 11:43:08 AM    Rob Tilton      Updated the 
       MOV section to the 9/16/98 sabine spec.
       
 108  mpeg      1.107       9/28/98 11:54:16 AM    Tim Ross        Fixed 
       compilation bugs.
       
 107  mpeg      1.106       9/27/98 7:48:52 PM     Tim Ross        Added 
       definition for ROM_XOE_MASK register
       
 106  mpeg      1.105       9/18/98 4:44:32 PM     Rob Tilton      Updated 
       MAKE_XSTARTWIDTH macro.
       
 105  mpeg      1.104       9/18/98 4:28:10 PM     Tim Ross        Added some 
       PCCard/CardBus definitions.
       
 104  mpeg      1.103       9/10/98 12:36:48 PM    Tim Ross        Modified 
       Sabine UART base address for FPGA version on PID board.
       Added PCI config space offset values and command register
       definitions.
       
 103  mpeg      1.102       9/3/98 3:16:16 PM      Dave Wilson     Renamed a 
       couple of smartcard registers to conform with documentation.
       
 102  mpeg      1.101       8/31/98 4:09:48 PM     Rob Tilton      Removed 
       bitfields from DRMPAL union.
       
 101  mpeg      1.100       8/26/98 1:08:28 PM     Dave Wilson     Updated 
       smartcard register definitions.
       
 100  mpeg      1.99        8/26/98 12:19:00 PM    Tim Ross        Modified 
       serial port macros.
       
 99   mpeg      1.98        7/31/98 6:02:34 PM     Tim Ross        Revised 
       serial port addressing symbols to be consistent with 
       remainder of SABINE.H.
       
 98   mpeg      1.97        7/31/98 4:42:22 PM     Tim Ross        Added base 
       address for 2nd serial port and changed register
       addresses to offsets from base.
       
 97   mpeg      1.96        7/31/98 1:31:58 PM     Dave Wilson     Changed 
       SER_BANK_SIZE to reflect changes in UART register addressing.
       
 96   mpeg      1.95        7/30/98 11:06:20 AM    Tim Ross        Added some 
       serial port stuff.
       
 95   mpeg      1.94        7/27/98 3:23:54 PM     Rob Tilton      Did a little
        union busting from I2C section.
       
 94   mpeg      1.93        7/22/98 11:16:06 AM    Ismail Mustafa  Added define
        for encoded video end register.
       
 93   mpeg      1.92        7/22/98 10:52:50 AM    Rob Tilton      Updated I2C 
       to 7/1/98 spec.
       
 92   mpeg      1.91        7/21/98 11:07:20 AM    Ismail Mustafa  Fixed 
       definition of Match/Mask structure and also fixed the Copro enable
       register definition.
       
 91   mpeg      1.90        6/29/98 3:08:28 PM     Dave Wilson     Added new 
       typedefs for MPEG read and write pointers.
       
 90   mpeg      1.89        6/29/98 8:57:06 AM     Dave Wilson     Added 
       PCI_MEM_DELAY_REG definition.
       
 89   mpeg      1.88        6/26/98 7:48:48 PM     Ismail Mustafa  Fixed the 
       interrupt defines to mach the 6/25 netlist.
       
 88   mpeg      1.87        6/26/98 3:43:12 PM     Amy Pratt       Added bit 
       29, SwParseMode, to CONTROL0.
       
 87   mpeg      1.86        6/24/98 11:15:06 AM    Rob Tilton      Updated 
       AspectRatio in MPGVidPicSize register.
       
 86   mpeg      1.85        6/24/98 10:22:18 AM    Ismail Mustafa  Added a 
       define for the byte swapped address space.
       
 85   mpeg      1.84        6/15/98 5:47:42 PM     Tim Ross        Changed 
       PCI_ROM_BURST_REG to PCI_ROM_MODE_REG.
       
 84   mpeg      1.83        6/11/98 11:56:36 AM    Dave Wilson     Added error 
       definition bits to DRM_STATUS register.
       
 83   mpeg      1.82        6/11/98 8:53:16 AM     Dave Wilson     Added new 
       bit 28 definition to MPG_CONTROL0_REG.
       
 82   mpeg      1.81        6/8/98 2:36:20 PM      Ismail Mustafa  Fixed typo 
       FileterMode to FilterMode.
       
 81   mpeg      1.80        6/5/98 10:44:54 AM     Dave Wilson     Fixed 
       register definitions for MPEG video and audio frame drop counters.
       
 80   mpeg      1.79        6/4/98 12:11:04 PM     Rob Tilton      Removed OSD 
       buffer.
       
 79   mpeg      1.78        6/4/98 10:53:12 AM     Dave Wilson     Changed a 
       comment telling readers that buffer size labels must be defined
       outside SABINE.H.
       
 78   mpeg      1.77        6/4/98 10:34:30 AM     Dave Wilson     Updated RST 
       block as per info from Eric Deal.
       
 77   mpeg      1.76        6/2/98 6:09:38 PM      Rob Tilton      Added DRMPAL
        definition for OSD header.
       
 76   mpeg      1.75        5/29/98 12:17:52 PM    Dave Wilson     BUFF_BASE 
       label was not defined for Phase 3.0 so builds failed!
       
 75   mpeg      1.74        5/28/98 4:32:10 PM     Dave Wilson     Corrected a 
       typo in one of the GPIO access macros.
       
 74   mpeg      1.73        5/27/98 9:51:54 AM     Dave Wilson     Shifted all 
       the hardware buffer size labels to the CONFIG files.
       
 73   mpeg      1.72        5/26/98 5:37:44 PM     Ismail Mustafa  Added 
       defines for CTRLReg0 microcode download.
       
 72   mpeg      1.71        5/26/98 5:03:54 PM     Dave Wilson     Changed 
       memory map for new Phase 3.0 hardware buffer layout.
       
 71   mpeg      1.70        5/20/98 1:44:56 PM     Dave Wilson     Updated I2C 
       definitions with new info from Truman Ng.
       
 70   mpeg      1.69        5/19/98 4:59:28 PM     Dave Wilson     Updated MPG 
       drop count registers with info from Aicha.
       
 69   mpeg      1.68        5/19/98 2:13:36 PM     Dave Wilson     Added new 
       MPG_AUD_SKIP_CNT_REG definition.
       Fixed up DRM_MPEG_OFFSET_WIDTH to include new PanScanOffset field.
       
 68   mpeg      1.67        5/19/98 9:32:00 AM     Dave Wilson     Updated PLL 
       block registers and removed multiple definitions for config
       resistors.
       
 67   mpeg      1.66        5/15/98 10:58:30 AM    Dave Wilson     Corrected a 
       couple of typos in the last version - oops.
       
 66   mpeg      1.65        5/15/98 10:49:36 AM    Dave Wilson     Changes to 
       PLL block by Eric Deal and a few new audio definitions from
       Matt Bates.
       
 65   mpeg      1.64        5/14/98 9:45:02 AM     Dave Wilson     Memory map 
       changes for Eric Deal and correction to address of ITC_INTENABLE
       register.
       
 64   mpeg      1.63        5/7/98 3:04:58 PM      Rob Tilton      Added an OSD
        define for XSW Pixel Alpha Select and changed the osd buffer
       size to 320x320.
       
 63   mpeg      1.62        5/6/98 12:10:34 PM     Ismail Mustafa  Increased 
       the ECM buffer size.
       
 62   mpeg      1.61        4/30/98 11:44:40 PM    Tim Ross        Updated 
       PCI_ROM_MAP structure to match latest hardware.
       
 61   mpeg      1.60        4/30/98 11:45:20 AM    Amy Pratt       Updated MPG 
       definitions for 4/17 spec and 4/27 netlist.
       
 60   mpeg      1.59        4/30/98 3:24:40 AM     Tim Ross        Removed 
       PCI_RESET.
       
 59   mpeg      1.58        4/30/98 3:06:26 AM     Tim Ross        Added 
       PCI_RESET definition.
       
 58   mpeg      1.57        4/30/98 2:50:28 AM     Tim Ross        Added 
       PCI_ROM_BURST_REG & 
       PCI_ROM_DISABLE_BURSTING.
       
 57   mpeg      1.56        4/28/98 7:38:08 PM     Tim Ross        Corrected 
       bug w/ EMULATION_LEVEL for memory maps - it 
       previously used PHASE2 twice instead of using PID7T || 
       PHASE2.
       
 56   mpeg      1.55        4/28/98 4:03:10 PM     Ismail Mustafa  Changed 
       definition of match and mask to 32 bit words instead of 8 bits.
       
 55   mpeg      1.54        4/28/98 10:38:30 AM    Ismail Mustafa  Added define
        for OSD buffer in the buffer memory map.
       
 54   mpeg      1.53        4/27/98 11:50:36 AM    Rob Tilton      Added 
       defines for multiple display modes.
       
 53   mpeg      1.52        4/24/98 3:00:08 PM     Tim Ross        Corrected 
       warning about PHASE2 being undeclared when 
       EMULATION_LEVEL definition was referenced - had to include 
       hwconfig.h
       
 52   mpeg      1.51        4/24/98 2:46:30 PM     Tim Ross        Added 
       further descriptor register definitions.
       
 51   mpeg      1.50        4/23/98 12:55:06 PM    Rob Tilton      Added 
       VIDEO_OUTPUT conditional around the OSD max sizes.
       
 50   mpeg      1.49        4/23/98 11:27:44 AM    Tim Ross        Added bit 
       position definitions to the ITC bunch.
       
 49   mpeg      1.48        4/21/98 4:24:24 PM     Tim Ross        Corrected 
       bug w/ SDR structure.
       
 48   mpeg      1.47        4/21/98 3:05:50 PM     Tim Ross        Modified 
       PCI/ROM descriptor register definitions for easier bit testing
       and setting.
       
 47   mpeg      1.46        4/21/98 11:57:34 AM    Tim Ross        Added 
       Resistor Strapping Options register definitions.
       
 46   mpeg      1.45        4/20/98 12:15:40 PM    Tim Ross        Added some 
       more SDR definitions to assist in setting the SDR registers.
       
 45   mpeg      1.44        4/20/98 11:56:40 AM    Tim Ross        Added 
       SDR_RANK1_EMPTY definition for memory controller settings.
       
 44   mpeg      1.43        4/17/98 6:33:00 PM     Tim Ross        Added SDRAM 
       controller definitions.
       
 43   mpeg      1.42        4/17/98 12:13:10 PM    Dave Wilson     Changed type
        used in bitfield definitions from u_int32 to BIT_FLD (unsigned
       long) to get rid of compiler warnings.
       Fixed up some pointer types that were wrong - either not parked as 
       pointers
       or not marked as volatile.
       
 42   mpeg      1.41        4/17/98 10:49:48 AM    Ismail Mustafa  Fixed 
       decoder status register structure declaration which was a 64 bit
       structure.
       
 41   mpeg      1.40        4/16/98 11:56:40 AM    Dave Wilson     Further 
       audio updates from Matt plus changed some GCP labels to save clashes.
       
 40   mpeg      1.39        4/15/98 4:09:58 PM     Dave Wilson     Added GCP 
       definitions and a few audio changes from Matt.
       
 39   mpeg      1.38        4/15/98 1:35:24 PM     Tim Ross        Corrected 
       UART0 & 1 UD/MS ordering in ITC.
       
 38   mpeg      1.37        4/15/98 12:20:34 PM    Dave Wilson     Changes for 
       Phase3.0 and new opentvx.h
       
 37   mpeg      1.36        4/15/98 11:17:52 AM    Tim Ross        Updated ITC.
       
 36   mpeg      1.35        4/15/98 10:56:14 AM    Tim Ross        Corrected 
       bug in ITC Expansion status register bit definitions for 
       UART1 & 0.
       
 35   mpeg      1.34        4/14/98 4:14:46 PM     Tim Ross        Stole 256 
       bytes from end of 2MB MPEG HWBUF region for storing 
       HW params in the event that FLASH in unavailable.
       
 34   mpeg      1.33        4/14/98 3:07:04 PM     Dave Wilson     Added new 
       macro to set the edges which will generate interrupts from a 
       GPIO line.
       
 33   mpeg      1.32        4/7/98 8:14:44 PM      Amy Pratt       Modified 
       MPEG section to match April 6 version of the spec
       
 32   mpeg      1.31        4/7/98 2:39:28 PM      Dave Wilson     Updated 
       interrupt controller and external video sections.
       
 31   mpeg      1.30        3/30/98 4:14:44 PM     Dave Wilson     Updated base
        addresses for various blocks
       Added new register info for redesigned timer block.
       
 30   mpeg      1.29        3/30/98 10:49:44 AM    Dave Wilson     Corrected 
       typo in last edit.
       
 29   mpeg      1.28        3/30/98 9:27:08 AM     Dave Wilson     Added ID pin
        interface register to RST section.
       
 28   mpeg      1.27        3/27/98 4:33:28 PM     Ismail Mustafa  Added 
       MPG_PIC_ADDR_BIT.
       
 27   mpeg      1.26        3/25/98 2:39:06 PM     Dave Wilson     Removed use 
       of PID7T label in favour of EMULATION_LEVEL.
       
 26   mpeg      1.25        3/25/98 9:34:04 AM     Dave Wilson     Changed 
       DRM_BASE for phase 2.0 emulation
       
 25   mpeg      1.24        3/25/98 9:25:44 AM     Dave Wilson     Changed 
       MPG_BASE for phase 2.0 emulation
       
 24   mpeg      1.23        3/24/98 5:14:30 PM     Dave Wilson     Removed 
       PID7T switch in the memory map. Phase 2.0 emulation doesn't include
       IR so address set to PID prototype in this case.
       
 23   mpeg      1.22        3/24/98 5:11:54 PM     Dave Wilson     Fixed up ARM
        processor registers to use u_int32 fields since short and char
       bitfields generate warnings.
       
 22   mpeg      1.21        3/24/98 4:25:34 PM     Dave Wilson     Removed 
       keyboard, joystick and mouse blocks
       Updated memory map as per latest spec, leaving a Phase 2.0 emulation 
       version
       to save problems (hopefully) with current code.
       
 21   mpeg      1.20        3/23/98 4:00:00 PM     Ismail Mustafa  Fixed define
        of ECM_ADDR.
       
 20   mpeg      1.19        3/20/98 11:16:24 AM    Ismail Mustafa  Added the 
       memory map defines for the moveable/resizable buffers.
       
 19   mpeg      1.18        3/17/98 3:12:48 PM     Anzhi Chen      Changed a 
       mistyping.
       
 18   mpeg      1.17        3/17/98 3:12:46 PM     Dave Wilson     Fixed 
       problem with MOV definitions (block copy error added 0x0x to all addrs)
       Included refinitions allowing IRD registers to be written as uint32s
       
 17   mpeg      1.16        3/17/98 9:46:38 AM     Ismail Mustafa  Removed the 
       bit definitions for the MPG ISR since it would cuase problems
       if used to write the register.
       
 16   mpeg      1.15        3/10/98 3:42:26 PM     Ismail Mustafa  Updated the 
       Decoder Status registerfor RESET_IN_PROGRESS bit.
       
 15   mpeg      1.14        3/9/98 1:31:58 PM      Dave Wilson     Corrected 
       typo in one of the MOV register typedefs
       
 14   mpeg      1.13        3/4/98 5:06:34 PM      Dave Wilson     Updated MOV 
       definitions.
       
 13   mpeg      1.12        3/3/98 2:44:26 PM      Rob Tilton      Added DRM 
       interrupt status register defines.
       
 12   mpeg      1.11        3/3/98 2:26:16 PM      Dave Wilson     Made all 
       pointers volatile types
       Updated MPG, ITC and GPI components to latest levels.
       
 11   mpeg      1.10        3/2/98 1:41:20 PM      Dave Wilson     Added base 
       address for FPGA (PID7T) IR controller.
       
 10   mpeg      1.9         3/2/98 9:39:40 AM      Rob Tilton      Updated DRM 
       section with the 2/25/98 version of spec.
       
 9    mpeg      1.8         2/16/98 8:49:10 AM     Ismail Mustafa  Missing a 
       semicolon.
       
 8    mpeg      1.7         2/13/98 5:02:54 PM     Ismail Mustafa  Update for 
       DVB related documentation changes.
       
 7    mpeg      1.6         2/11/98 9:52:12 AM     Ismail Mustafa  Changed 
       __int8 to u_int8 to mach OpenTV types.
       
 6    mpeg      1.5         2/2/98 11:24:34 AM     Dave Wilson     Changed 
       basic types to conform to OpenTV types (u_int32, etc).
       
 5    mpeg      1.4         1/29/98 11:13:54 AM    Ismail Mustafa  Fixes and 
       additions for MPEG and DVB portions.
       
 4    mpeg      1.3         1/12/98 11:02:36 AM    Dave Wilson     Update after
        ARM spec revisions.
       
 3    mpeg      1.2         1/6/98 11:16:50 AM     Dave Wilson     Latest 
       revision with most peripherals now added. Still missing the
       parser and MPEG audio modules.
       
 2    mpeg      1.1         12/30/97 10:28:52 AM   Dave Wilson     As complete 
       as possible given the current spec. Still missing all
       information on DVB Parser and Audio Decoder. I2C incomplete.
       
 1    mpeg      1.0         12/23/97 2:53:20 PM    Dave Wilson     
$
 * 
 *    Rev 1.188   22 Aug 2002 15:01:54   wangl2
 * SCR(s) 4459 :
 * Include wabash.h if CHIP_NAME==WABASH.
 * 
 * 
 *    Rev 1.187   03 Jul 2001 09:43:38   whiteth
 * SCR(s) 2178 2179 2180 :
 * Merge branched Hondo specific code back into the mainstream source database.
 * 
 * 
 *    Rev 1.186.1.0   09 May 2001 16:15:38   prattac
 * DCS1642 Removed support for Neches and reference to DCS966 from code
 * 
 *    Rev 1.186   01 Feb 2001 13:09:32   rossst
 * DCS966.
 * Added supprot for Hondo.
 * 
 *    Rev 1.185   14 Oct 1999 11:48:38   whiteth
 * Added hwconfig.h #include.
 * 
 *    Rev 1.184   12 Oct 1999 10:49:00   dawilson
 * Removed everything. File is now a wrapper which includes neches.h or 
 * colorado.h as appropriate.
*/

#ifndef _SABINE_H_
#define _SABINE_H_

#include "hwconfig.h"

/* This file is now a simple wrapper which pulls in the correct register */
/* definitions for the IC this build is targeted for.                    */

#if CHIP_NAME == COLORADO
#include "colorado.h"
#elif CHIP_NAME == HONDO
#include "hondo.h"
#elif CHIP_NAME == WABASH
#include "wabash.h"
#endif

#endif
