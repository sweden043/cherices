#****************************************************************************
#*                   CONEXANT PROPRIETARY AND CONFIDENTIAL                  *
#*                       SOFTWARE FILE/MODULE HEADER                        *
#*                    Conexant Systems Inc. (c) 1999-2003                   *
#*                                Austin, TX                                *
#*                           All Rights Reserved                            *
#****************************************************************************
#
#  Filename:        hwlib.mak
#
#
#  Description:     Hardware function library
#
#
#  Author:          Dave Wilson
#
#****************************************************************************
#  $Header: hwlib.mak, 25, 3/8/04 5:49:06 PM, Miles Bintz$
#****************************************************************************

######################################################################
# File lists. These 5 macros define the files comprising this driver #
######################################################################

!if "$(RTOS)" == "NOOS"

ARM_ASM_FILES   = CACHE.S HWLIB_MMU.S

GENERAL_C_FILES = LEDS.C GPIO.C

!else

GENERAL_C_FILES = HWFUNCS.C         \
                  HWLIB.C           \
                  LEDS.C            \
                  GPIO.C            \
                  LLSERIAL.C        \
                  LLSERINTERNAL.C   \
                  LLSERTELEGRAPH.C  \
						LLSERHOSTMODEM.C  \
                  LLSERMEMORY.C     

# DBG comm channels require ARM CRT's 
!if "$(ARM_TOOLKIT)" == "ADS" || "$(ARM_TOOLKIT)" == "SDT"
ARM_C_FILES = LLSERDBGCOMM.C    \
!endif

ARM_ASM_FILES =   CRITSEC.S CACHE.S HWLIB_MMU.S

OTHER_FILES =     HWLIBINT.H CRITSEC.H
!endif

PUBLIC_HEADERS = MMU.A

###############################################################################
# $Log: 
#  25   mpeg      1.24        3/8/04 5:49:06 PM      Miles Bintz     CR(s) 8509
#         : modified mmu.S to HWLIB_MMU.S to remove object name conflict in 
#        NDSCore build
#  24   mpeg      1.23        9/22/03 5:00:54 PM     Bobby Bradford  SCR(s) 
#        7418 :
#        Added new driver file, LLSERHOSTMODEM.C to support the new
#        LLSER Host Connected Modem driver
#        
#  23   mpeg      1.22        9/19/03 2:45:18 PM     Miles Bintz     SCR(s) 
#        7484 :
#        there was some crazy logic that said if RTOS==blah else if 
#        APPNAME==BLAH.  RTOS and APPNAME are mutually exclusive items.  one 
#        does not imply anything about the other.  anything that has noos 
#        (codeldr, download) only needs mmu.s and cache.s
#        
#  22   mpeg      1.21        8/27/03 6:25:02 PM     Miles Bintz     SCR(s) 
#        7291 :
#        use of mice dbg comm channel is dependant on the ARM CRT (ie. toolkit)
#         not RTOS
#        
#  21   mpeg      1.20        7/30/03 3:24:30 PM     Larry Wang      SCR(s) 
#        7076 :
#        For vxworks builds, get *.s instead of vx*.s
#        
#  20   mpeg      1.19        7/10/03 10:41:24 AM    Larry Wang      SCR(s) 
#        6924 :
#        Include the same modules in case RTOS==VXWORKS && APPNAME==CODELDR as 
#        in the one where RTOS==NOOS.  Get MMU.A instead of MMU.VXA.
#        
#  19   mpeg      1.18        7/9/03 3:27:32 PM      Tim White       SCR(s) 
#        6901 :
#        Phase 3 codeldrext drop.
#        
#        
#  18   mpeg      1.17        5/15/03 2:09:26 PM     Tim White       SCR(s) 
#        6369 6370 :
#        Fixed build break building codeldr.
#        
#        
#  17   mpeg      1.16        5/14/03 3:50:06 PM     Tim White       SCR(s) 
#        6346 6347 :
#        Remove vxmmu.gasp since it's no longer needed and created vxmmu.s.
#        
#        
#  16   mpeg      1.15        1/17/03 4:36:56 PM     Dave Wilson     SCR(s) 
#        5264 :
#        Added new GPIO.C source file
#        
#  15   mpeg      1.14        12/20/02 1:56:40 PM    Dave Wilson     SCR(s) 
#        5204 :
#        Added llsermemory.c
#        
#  14   mpeg      1.13        12/19/02 4:36:22 PM    Bobby Bradford  SCR(s) 
#        5192 :
#        Re-enable the LLSERIAL drivers (except for the Debug COMMS
#        channel support)
#        
#  13   mpeg      1.12        12/16/02 2:00:30 PM    Dave Wilson     SCR(s) 
#        5177 :
#        Added LEDS.C - virtualised access to front panel LEDs.
#        
#  12   mpeg      1.11        10/2/02 10:19:30 AM    Bobby Bradford  SCR(s) 
#        4726 :
#        Oops ... VxWorks can't compile the LLSERDBGCOMM.C file,
#        because it has inline-assembly that isn't compatible with
#        GNU.  Don't need it anyway for VxWorks, so just leave it
#        out for now.
#        
#  11   mpeg      1.10        10/1/02 2:18:22 PM     Bobby Bradford  SCR(s) 
#        4726 :
#        moved the low-level serial files/functions to this driver
#        
#  10   mpeg      1.9         5/7/02 4:16:56 PM      Dave Moore      SCR(s) 
#        3724 :
#        added mmu.s
#        
#        
#  9    mpeg      1.8         4/5/02 6:18:08 PM      Dave Moore      SCR(s) 
#        3458 :
#        turn on vxworks 920t support
#        
#        
#  8    mpeg      1.7         4/1/02 8:21:50 AM      Dave Moore      SCR(s) 
#        3457 :
#        Added 920T support (non vxw)
#        
#        
#  7    mpeg      1.6         12/14/01 9:03:46 AM    Lucy C Allevato SCR(s) 
#        2970 :
#        Deleted source file rules
#        
#  6    mpeg      1.5         4/13/00 3:19:32 PM     Tim Ross        Changed 
#        cache.s for VxWorks case to vxcache.s.
#        
#  5    mpeg      1.4         4/12/00 2:54:34 PM     Senthil Veluswamy added 
#        cache.s - new file with some Cache routines
#        
#  4    mpeg      1.3         3/7/00 11:05:20 AM     Miles Bintz     Changes 
#        for VXWorks
#        
#  3    mpeg      1.2         1/6/00 10:27:14 AM     Dave Wilson     Changes 
#        for ARM/Thumb interworking
#        
#  2    mpeg      1.1         9/15/99 11:43:34 AM    Dave Wilson     Added 
#        HWLIBINT.H
#        
#  1    mpeg      1.0         9/14/99 3:11:58 PM     Dave Wilson     
# $
#  
#     Rev 1.23   22 Sep 2003 16:00:54   bradforw
#  SCR(s) 7418 :
#  Added new driver file, LLSERHOSTMODEM.C to support the new
#  LLSER Host Connected Modem driver
#  
#     Rev 1.22   19 Sep 2003 13:45:18   bintzmf
#  SCR(s) 7484 :
#  there was some crazy logic that said if RTOS==blah else if APPNAME==BLAH.  RTOS and APPNAME are mutually exclusive items.  one does not imply anything about the other.  anything that has noos (codeldr, download) only needs mmu.s and cache.s
#  
#     Rev 1.21   27 Aug 2003 17:25:02   bintzmf
#  SCR(s) 7291 :
#  use of mice dbg comm channel is dependant on the ARM CRT (ie. toolkit) not RTOS
#  
#     Rev 1.20   30 Jul 2003 14:24:30   wangl2
#  SCR(s) 7076 :
#  For vxworks builds, get *.s instead of vx*.s
#  
#     Rev 1.19   10 Jul 2003 09:41:24   wangl2
#  SCR(s) 6924 :
#  Include the same modules in case RTOS==VXWORKS && APPNAME==CODELDR as in the one where RTOS==NOOS.  Get MMU.A instead of MMU.VXA.
#  
#     Rev 1.18   09 Jul 2003 14:27:32   whiteth
#  SCR(s) 6901 :
#  Phase 3 codeldrext drop.
#  
#  
#     Rev 1.17   15 May 2003 13:09:26   whiteth
#  SCR(s) 6369 6370 :
#  Fixed build break building codeldr.
#  
#  
#     Rev 1.16   14 May 2003 14:50:06   whiteth
#  SCR(s) 6346 6347 :
#  Remove vxmmu.gasp since it's no longer needed and created vxmmu.s.
#  
#
###############################################################################


