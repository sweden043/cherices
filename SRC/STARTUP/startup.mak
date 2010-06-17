#****************************************************************************
#*                   CONEXANT PROPRIETARY AND CONFIDENTIAL                  *
#*                       SOFTWARE FILE/MODULE HEADER                        *
#*                    Conexant Systems Inc. (c) 1998-2003                   *
#*                                Austin, TX                                *
#*                           All Rights Reserved                            *
#****************************************************************************
#
#  Filename:        startup.mak
#
#
#  Description:     Startup component makefile
#
#
#  Author:          Tim Ross
#
#****************************************************************************
#  $Id: startup.mak,v 1.70, 2004-06-14 08:57:30Z, Xiao Guang Yan$
#****************************************************************************

######################################################################
# File lists. These 5 macros define the files comprising this driver #
######################################################################

!if "$(DLOAD)" != "YES"
BDFILEBASE=BD_$(CUSTOMER)

COMMON_GENERAL_C_FILES =  $(BDFILEBASE).C \
                          PCCARD.C    \
                          PCIDATA.C   \
                          PCI.C       \
                          PLLC.C      \
                          ROM.C       \
                          ROMDATA.C   \
                          MMU.C       \
                          STARTUPC.C  \
                          EECFG.C     \
                          RAM.C       \
                          SETUPROM.C

COMMON_GENERAL_ASM_FILES =   LCDS.S       \
                             UTILS.S      \
                             STARTUPIIC.S \
                             API.S        \
                             EECFGS.S

COMMON_OTHER_FILES =     PCCARD.H \
                         STARTUPPCI.H \
                         RAM.H


!if "$(RTOS)" == "NOOS"

GENERAL_C_FILES   = PLLC.C ROM.C  EECFG.C ROMDATA.C SETUPROM.C

ARM_ASM_FILES     = BOARD.S SERIAL.S LCDS.S STARTUPIIC.S API.S UTILS.S EECFGS.S


!elseif "$(RTOS)" == "VXWORKS"

GENERAL_C_FILES = $(COMMON_GENERAL_C_FILES)

ARM_ASM_FILES =   $(COMMON_GENERAL_ASM_FILES) 

OTHER_FILES =     $(COMMON_OTHER_FILES)

!else

GENERAL_C_FILES = $(COMMON_GENERAL_C_FILES)

ARM_ASM_FILES =   $(COMMON_GENERAL_ASM_FILES) \
                  STARTUP.S   \
                  BOARD.S     \
                  OS.S        \
                  SERIAL.S    \
                  PAGETAB_920T.S \
                  INITMMU.S

OTHER_FILES =     $(COMMON_OTHER_FILES)
                  
!endif

!if "$(RTOS)" == "PSOS" || "$(RTOS)" == "PSOS25" 
GENERAL_C_FILES = $(GENERAL_C_FILES) OSC.C
!endif

!else #  DLOAD == YES

ARM_C_FILES    =  BD_CNXT.C   \
                  PLLC.C      \
                  ROM.C       \
                  ROMDATA.C   \
                  SETUPROM.C

!if "$(DOWNLOAD_SERIAL_SUPPORT)"=="YES"
ARM_ASM_FILES  =  STARTUP.S BOARD.S LCDS.S SERIAL.S
!else
ARM_ASM_FILES = 
!endif

!endif

PUBLIC_HEADERS = MMU.H

###############################################################################
# $Log: 
#  71   mpeg      1.70        6/14/04 3:57:30 AM     Xiao Guang Yan  CR(s) 9450
#         9451 : Removed NDSCORE_FGDL macro.
#  70   mpeg      1.69        6/13/04 10:26:45 PM    Xiao Guang Yan  CR(s) 9450
#         9451 : Commented out startupc.c and startup.s for FGDL build. 
#  69   mpeg      1.68        4/27/04 11:38:17 AM    Miles Bintz     CR(s) 8971
#         8972 : corrected remaining reference to iic.s
#  68   mpeg      1.67        4/26/04 12:36:13 PM    Miles Bintz     CR(s) 8953
#         8954 : renamed iic.s to startupiic.s
#  67   mpeg      1.66        4/23/04 7:52:58 PM     Sunil Cheruvu   CR(s) 8870
#         8871 : Fix the DL_SER ADS SDT build break.
#  66   mpeg      1.65        4/22/04 4:22:50 PM     Sunil Cheruvu   CR(s) 8870
#         8871 : Added the Nand Flash support for Wabash(Milano rev 5 and 
#        above) and Brazos(Bronco).
#  65   mpeg      1.64        11/5/03 5:12:59 PM     Miles Bintz     CR(s): 
#        7853 added eecfgs.s to list of files to build for serial download 
#        utility
#  64   mpeg      1.63        11/5/03 12:44:57 PM    Miles Bintz     CR(s): 
#        7853 correct bugs introduced in CR 7801
#  63   mpeg      1.62        11/1/03 3:02:00 PM     Tim Ross        CR(s): 
#        7719 7762 Added iic.s to serial download build and eecfgs.s to regular
#         builds.
#  62   mpeg      1.61        10/27/03 3:59:42 PM    Tim White       CR(s): 
#        7724 Force C-library initilization when building serial flash download
#         tool with ADS toolkit.
#        
#  61   mpeg      1.60        9/23/03 6:40:20 PM     Miles Bintz     SCR(s) 
#        7523 :
#        added software config options to support flash download utility(s)
#        
#  60   mpeg      1.59        9/22/03 12:43:40 PM    QA - Roger Taylor SCR(s) 
#        7484 :
#        changed which files are used for RTOS==NOOS
#        
#  59   mpeg      1.58        9/19/03 7:06:14 PM     Miles Bintz     SCR(s) 
#        7484 :
#        modified to support for download application
#        
#  58   mpeg      1.57        9/11/03 7:23:12 PM     Miles Bintz     SCR(s) 
#        7291 :
#        codeldr specific case 
#        
#  57   mpeg      1.56        7/30/03 3:53:44 PM     Larry Wang      SCR(s) 
#        7076 :
#        Remove vxcpu.h
#        
#  56   mpeg      1.55        7/30/03 3:46:26 PM     Larry Wang      SCR(s) 
#        7076 :
#        For vxworks builds, get *.s instead of vx*.s
#        
#  55   mpeg      1.54        7/22/03 6:17:30 PM     Tim White       SCR(s) 
#        7018 :
#        The loaders use only instruction caching without MMU/MPU support.  
#        Remove the
#        support for using the MMU/MPU from the loader code.
#        
#        
#  54   mpeg      1.53        7/10/03 10:33:04 AM    Larry Wang      SCR(s) 
#        6924 :
#        Include the same modules in case RTOS==VXWORKS && APPNAME==CODELDR as 
#        in the one where RTOS==NOOS.
#        
#  53   mpeg      1.52        7/9/03 3:29:26 PM      Tim White       SCR(s) 
#        6901 :
#        Phase 3 codeldrext drop.
#        
#        
#  52   mpeg      1.51        6/24/03 6:38:44 PM     Tim White       SCR(s) 
#        6831 :
#        Add flash, hsdp, demux, OSD, and demod support to codeldrext.
#        
#        
#  51   mpeg      1.50        5/14/03 4:37:30 PM     Tim White       SCR(s) 
#        6346 6347 :
#        Consolidated the cache functions to startup & hwlib from codeldr.  
#        Added pllc.c for 
#        calculating the MemClkPeriod and MemClkPeriod32 for application, boot,
#         and flash download
#        utility supporting both runtime and compile-time environments.
#        
#        
#  50   mpeg      1.49        5/5/03 5:06:34 PM      Tim White       SCR(s) 
#        6172 :
#        Remove duplicate low-level boot support code and use startup directory
#         for building
#        codeldr.  Remove 7 segment LED support.
#        
#        
#  49   mpeg      1.48        4/11/03 11:49:14 AM    Craig Dry       SCR(s) 
#        5948 :
#        startup.vxa now needed for vxworks build
#        
#  48   mpeg      1.47        2/6/03 1:30:34 PM      Billy Jackman   SCR(s) 
#        5332 :
#        Put back in the architecture previously applied to separate out the 
#        iic
#        access routines.
#        
#  47   mpeg      1.46        1/31/03 5:20:40 PM     Dave Moore      SCR(s) 
#        5375 :
#        removed SU_CACHE.S
#        
#        
#  46   mpeg      1.45        1/29/03 2:31:42 PM     Senthil Veluswamy SCR(s) 
#        5332 :
#        Added the IIC Assembly source files
#        
#  45   mpeg      1.44        4/5/02 6:23:14 PM      Dave Moore      SCR(s) 
#        3458 :
#        turn on 920t support
#        
#        
#  44   mpeg      1.43        4/1/02 8:52:16 AM      Dave Moore      SCR(s) 
#        3457 :
#        added 920T support
#        
#        
#  43   mpeg      1.42        2/1/02 9:57:22 AM      Ian Mitchell    SCR(s): 
#        3101 
#        Added drivers to build if the RTOS is UCOS
#        
#        
#  42   mpeg      1.41        12/13/01 3:40:16 PM    Lucy C Allevato SCR(s) 
#        2970 :
#        Deleted source file rules
#        
#  41   mpeg      1.40        12/4/01 10:17:08 AM    QA - Roger Taylor SCR(s) 
#        2936 :
#        Added startuppci to vxworks list
#        
#  40   mpeg      1.39        11/29/01 2:13:36 PM    QA - Roger Taylor SCR(s) 
#        2936 :
#        startuppci.h was not added to all the correct places
#        
#  39   mpeg      1.38        11/29/01 1:03:12 PM    Miles Bintz     SCR(s) 
#        2936 :
#        Moved definitions that were originally in include\cnxtpci.h to 
#        startup\startuppci.h
#        
#  38   mpeg      1.37        8/23/01 3:16:44 PM     Miles Bintz     SCR(s) 
#        2526 :
#        Changed cache.s to su_cache.s so as not to be confused with hwlib's 
#        cache.s
#        
#        
#  37   mpeg      1.36        1/3/01 10:29:22 AM     Ismail Mustafa  Changes 
#        from Miles for ARCH4 Ethernet debugging.
#        
#  36   mpeg      1.35        9/23/00 5:53:14 PM     Joe Kroesche    changed to
#         accomodate multiple bd_... customer boarddata files
#        
#  35   mpeg      1.34        8/30/00 12:42:08 PM    Miles Bintz     added NUP
#        
#  34   mpeg      1.33        5/3/00 4:49:54 PM      QA - Roger Taylor Added 
#        vxapi.s to get list.
#        
#  33   mpeg      1.32        5/1/00 11:48:04 PM     Ray Mack        ADDED 
#        missing file to vxworks
#        
#  32   mpeg      1.31        5/1/00 11:42:36 PM     Ray Mack        changes to
#         pick up Klondike files for VxWorks
#        
#  31   mpeg      1.30        4/12/00 3:12:50 PM     Miles Bintz     some 
#        vxworks mods
#        
#  30   mpeg      1.29        4/9/00 5:57:58 PM      Amy Pratt       Added 
#        rules for LCDS.S and LCDS.A.  Without these rules, PSOS
#        builds with TYPE=GET were broken.
#        
#  29   mpeg      1.28        4/7/00 5:06:26 PM      Miles Bintz     more 
#        changes
#        
#        
#  28   mpeg      1.27        4/7/00 3:49:48 PM      Miles Bintz     modified
#        added rule to get vxlcds
#        
#  27   mpeg      1.26        4/7/00 3:24:12 PM      Miles Bintz     bad 
#        spaces!  bad!
#        
#  26   mpeg      1.25        4/7/00 3:08:30 PM      Dave Wilson     Changes to
#         allow STARTUP to work on Klondike boards
#        
#  25   mpeg      1.24        3/16/00 1:08:34 PM     Ray Mack        fixes for 
#        vxworks initial release
#        
#  24   mpeg      1.23        3/10/00 12:51:28 PM    Miles Bintz     added 
#        vxcpu.h to list
#        
#  23   mpeg      1.22        3/10/00 12:40:44 PM    Miles Bintz     added 
#        pccard.h to list of vxworks files
#        
#  22   mpeg      1.21        3/10/00 12:18:14 PM    Miles Bintz     Made 
#        SetupRom.C common to both PSOS and VXW
#        
#  21   mpeg      1.20        3/10/00 12:09:16 PM    Miles Bintz     added more
#         files to make vxworks friendly
#        
#  20   mpeg      1.19        3/9/00 5:36:24 PM      Miles Bintz     fixed a 
#        typo in 1.18
#        
#  19   mpeg      1.18        3/9/00 4:25:04 PM      Miles Bintz     vxw files 
#        added to makefile
#        
#  18   mpeg      1.17        3/9/00 2:04:10 PM      Miles Bintz     Made 
#        startup.mak VXW friendly
#        
#  17   mpeg      1.16        3/8/00 5:24:36 PM      Dave Wilson     Removed 
#        ROM.H
#        Removed EXTRA_ARM_C_FLAGS redefinition - flags moved to C32LRULE.MAK 
#        
#  16   mpeg      1.15        3/6/00 2:34:18 PM      Ray Mack        No change.
#        
#  15   mpeg      1.14        3/6/00 9:13:26 AM      Ray Mack        changes to
#         fix after OS selection changes
#        
#  14   mpeg      1.13        3/3/00 2:12:50 PM      Tim White       Removed 
#        STARTERR.H reference, file has been deleted.
#        
#  13   mpeg      1.12        1/6/00 10:41:06 AM     Dave Wilson     Changes 
#        for ARM/Thumb builds
#        
#  12   mpeg      1.11        11/11/99 12:52:06 PM   Dave Wilson     Split 
#        boardc.c into 3 parts to allow more selective builds.
#        
#  11   mpeg      1.10        11/1/99 6:27:56 PM     Tim Ross        Added 
#        APIC.C.
#        
#  10   mpeg      1.9         10/28/99 12:06:02 PM   Dave Wilson     Added 
#        API.S
#        
#  9    mpeg      1.8         10/13/99 4:58:34 PM    Tim White       Allow 
#        run-from-ROM code to call SetupROMs().  Without this functionality,
#        code cannot be run from the ROM directly.
#        
#  8    mpeg      1.7         7/1/99 1:21:06 PM      Dave Wilson     Added 
#        RAM.H to makefile.
#        
#  7    mpeg      1.6         7/1/99 1:11:56 PM      Dave Wilson     Added 
#        RAM.C to the PVCS rules section.
#        
#  6    mpeg      1.5         7/1/99 1:07:48 PM      Tim Ross        Added 
#        RAM.C.
#        
#  5    mpeg      1.4         6/8/99 5:35:22 PM      Tim Ross        Removed 
#        some unnecessary files.
#        
#  4    mpeg      1.3         6/8/99 4:12:36 PM      Tim Ross        Updated 
#        paths for some files to INCLUDE driectoryt.
#        
#  3    mpeg      1.2         6/8/99 4:01:24 PM      Tim Ross        Removed 
#        files that shouldn't have been there.
#        
#  2    mpeg      1.1         6/8/99 3:57:26 PM      Tim Ross        Added PVCS
#         files.
#        
#  1    mpeg      1.0         6/8/99 12:50:32 PM     Tim Ross        
# $
###############################################################################

