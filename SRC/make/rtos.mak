#****************************************************************************
#*                            Conexant Systems
#****************************************************************************
#*
#* Filename:       rtos.mak
#*
#* Description:    Top Level RTOS Configuration File for MAKE Files
#*
#* Author:         Bobby Bradford
#*
#* Copyright Conexant Systems, 2001
#* All Rights Reserved.
#*
#****************************************************************************
#* $Header: rtos.mak, 28, 5/4/04 12:30:25 AM, Steve Glennon$
#****************************************************************************


#****************************************************************************
#* Nuclues RTOS Configurations Goes Here
#****************************************************************************
!if "$(RTOS)" == "NUP"

!if "$(NUP_PROFILING)" == "YES"
NUP_PROF = _PROF
!endif

RTOS_INCL = $(SABINE_ROOT)\nupbsp;$(SABINE_ROOT)\nupincl;$(TOOL_INCL)

BSP                   = NUPBSP
DELCFG                = NO
OS_INCLUDE_DIR        = NUPINCL

!if "$(BUILDOS)" == "YES"
OPERATING_SYS_DRIVERS = PLUS NUPKAL $(BSP) STARTUP 
OPERATING_SYS_LIBS    = PLUS.LIB NUPKAL.LIB $(BSP).LIB STARTUP.LIB
RTOS_INCL = $(RTOS_INCL);$(SABINE_ROOT)\plus

!else
OPERATING_SYS_DRIVERS = NUPLIB_P$(PACKING)  NUPKAL $(BSP) STARTUP
OPERATING_SYS_LIBS    = NUPKAL.LIB $(BSP).LIB STARTUP.LIB

!if      "$(ARM_TOOLKIT)" == "ADS"
NUP_LIB_EXT           = ADS
!elseif  "$(ARM_TOOLKIT)" == "SDT"
NUP_LIB_EXT           = LIB
!elseif  "$(ARM_TOOLKIT)" == "WRGCC"
!error  No support for NUP with GCC!
!elseif  "$(ARM_TOOLKIT)" == "GNUGCC"
!error  No support for NUP with GCC!
!endif

!if "$(MODE)" == "THUMB"
OS_LIBS               = $(SABINE_ROOT)\nuplib_p$(PACKING)\thm_plus$(NUP_PROF).$(NUP_LIB_EXT) $(TOOLKIT_LIBS)
!else 
OS_LIBS               = $(SABINE_ROOT)\nuplib_p$(PACKING)\plus$(NUP_PROF).$(NUP_LIB_EXT)
!endif

!endif  # BUILDOS

!if "$(EXTENDED_KAL)"=="YES"
OPERATING_SYS_DRIVERS = $(OPERATING_SYS_DRIVERS) KALEXINC
!endif

ENTRY_OBJ             = $(SABINE_ROOT)\STARTUP\$(CONFIG)\$(APPNAME)\$(RTOS)\$(SWCONFIG)\$(DEBUGDIR)\startup.o32

#****************************************************************************
#* PSOS 2.3.x RTOS Configurations Goes Here
#****************************************************************************
!elseif "$(RTOS)" == "PSOS"

!if "$(ARM_TOOLKIT)" == "ADS" || "$(ARM_TOOLKIT)" == "WRGCC" || "$(ARM_TOOLKIT)" == "GNUGCC"
!error PSOS with ADS, WindRiver's GCC, or GNU's GCC is not valid!!!
!endif


BSP            = PSOSBSP
OSCFG          = PSOSCFG
OS_INCLUDE_DIR = PSOSINCL

RTOS_INCL =  $(SABINE_ROOT)\$(OS_INCLUDE_DIR);$(PSS_ROOT)\include

DELCFG         = NO

OS_LIBS   = $(PSS_ROOT)\sys\os\libsys.16l    \
!if exist ($(PSS_ROOT)\sys\libc\libprepc.16l) == 0 
          $(PSS_ROOT)\sys\libc\libprepcS.16l
!else
          $(PSS_ROOT)\sys\libc\libprepc.16l
!endif

OPERATING_SYS_DRIVERS    = STARTUP PSOSKAL $(BSP) $(OSCFG)
OPERATING_SYS_LIBS       = STARTUP.LIB PSOSKAL.LIB $(BSP).LIB $(OSCFG).LIB 

ENTRY_OBJ       = $(SABINE_ROOT)\STARTUP\$(CONFIG)\$(APPNAME)\$(RTOS)\$(SWCONFIG)\$(DEBUGDIR)\startup.o32

#****************************************************************************
#* PSOS 2.5.x RTOS Configurations Goes Here
#****************************************************************************
!elseif "$(RTOS)" == "PSOS25"

!if "$(ARM_TOOLKIT)" == "WRGCC" || "$(ARM_TOOLKIT)" == "GNUGCC"
!error PSOS with WindRiver's GCC, or GNU's GCC is not valid!!!
!endif


BSP            = PSOS25BSP
OSCFG          = PSOS25CFG
OS_INCLUDE_DIR = PSOS25INCL

OS_LIBS   = $(PSS25_ROOT)\sys\os\libsys.16l    \
!if exist ($(PSS25_ROOT)\sys\libc\libprepc.16l) == 0 
          $(PSS25_ROOT)\sys\libc\libprepcS.16l
!else
          $(PSS25_ROOT)\sys\libc\libprepc.16l
!endif

RTOS_INCL =  $(SABINE_ROOT)\$(OS_INCLUDE_DIR);$(PSS25_ROOT)\include

DELCFG         = NO

OPERATING_SYS_DRIVERS    = STARTUP PSOSKAL $(BSP) $(OSCFG)
OPERATING_SYS_LIBS       = STARTUP.LIB PSOSKAL.LIB $(BSP).LIB $(OSCFG).LIB 

ENTRY_OBJ       = $(SABINE_ROOT)\STARTUP\$(CONFIG)\$(APPNAME)\$(RTOS)\$(SWCONFIG)\$(DEBUGDIR)\startup.o32

#****************************************************************************
#* UCOS RTOS Configurations Goes Here
#****************************************************************************


!elseif "$(RTOS)" == "UCOS"


BSP            = UCOSBSP
DELCFG         = NO

OS_INCLUDE_DIR           = UCOSINC

!if "$(BUILDOS)" == "YES"
OPERATING_SYS_DRIVERS    = STARTUP UCOSSRC $(BSP) UCKRNSUP
OPERATING_SYS_LIBS       = STARTUP.LIB UCOSSRC.LIB $(BSP).LIB UCKRNSUP.LIB

!else
OPERATING_SYS_DRIVERS    = STARTUP UCKRNSUP UCOSLIB
OPERATING_SYS_LIBS       = STARTUP.LIB UCKRNSUP.LIB 
OS_LIBS                  = $(SABINE_ROOT)\ucoslib\ucos.lib $(TOOLKIT_LIBS)

!endif  # BUILDOS


!if "$(APPNAME)" != "UCOSTST"
OPERATING_SYS_DRIVERS = $(OPERATING_SYS_DRIVERS) UCOSKAL
OPERATING_SYS_LIBS    = $(OPERATING_SYS_LIBS)    UCOSKAL.LIB
!endif

RTOS_INCL = $(TOOL_INCL);$(SABINE_ROOT)\UCOSINC;$(SABINE_ROOT)\$(BSP);$(TOOL_INCL);$(SABINE_ROOT)\UCKRNSUP;$(SABINE_ROOT)\UCOSKAL



ENTRY_OBJ       = $(SABINE_ROOT)\STARTUP\$(CONFIG)\$(APPNAME)\$(RTOS)\$(SWCONFIG)\$(DEBUGDIR)\startup.o32
#****************************************************************************
#* UCOS2 RTOS Configurations Goes Here
#****************************************************************************
!elseif "$(RTOS)" == "UCOS2"


BSP            = UCOS2BSP
DELCFG         = NO

OS_INCLUDE_DIR           = UCOS2INC

OPERATING_SYS_DRIVERS    = STARTUP UCOS2KAL UCOS2SRC $(BSP)
OPERATING_SYS_LIBS       = STARTUP.LIB UCOS2KAL.LIB UCOS2SRC.LIB $(BSP).LIB

RTOS_INCL = $(TOOL_INCL);$(SABINE_ROOT)\UCOS2INC;$(SABINE_ROOT)\$(BSP);$(TOOL_INCL)

ENTRY_OBJ       = $(SABINE_ROOT)\STARTUP\$(CONFIG)\$(APPNAME)\$(RTOS)\$(SWCONFIG)\$(DEBUGDIR)\startup.o32


#****************************************************************************
#* VxWorks RTOS Configurations Goes Here
#****************************************************************************
!elseif "$(RTOS)" == "VXWORKS"

!if "$(ARM_TOOLKIT)" == "ADS" || "$(ARM_TOOLKIT)" == "SDT"
!error VxWorks with ADS, or SDT is not valid!!!
!endif

RTOS_INCL=$(SABINE_ROOT)\vxincl;$(SABINE_ROOT)\vxbsp;$(SABINE_ROOT)\vxkal
RTOS_INCL=$(RTOS_INCL);$(WIND_BASE)\TARGET
RTOS_INCL=$(RTOS_INCL);$(WIND_BASE)\TARGET\H\private;
RTOS_INCL=$(RTOS_INCL);$(WIND_BASE)\TARGET\H;$(WIND_BASE)\TARGET\H\RPC
RTOS_INCL=$(RTOS_INCL);$(WIND_BASE)\TARGET\CONFIG\ALL;$(WIND_BASE)\TARGET\H\DRV\NETIF
RTOS_INCL=$(RTOS_INCL);$(WIND_BASE)\TARGET\SRC\CONFIG;$(WIND_BASE)\TARGET\SRC\DRV
RTOS_INCL=$(RTOS_INCL);$(WIND_BASE)\TARGET\H\ARCH\$(WIND_ARCH);$(TOOL_INCL)

MODE=ARM
BSP=VXBSP
DELCFG=NO
OS_INCLUDE_DIR = VXINCL

OPERATING_SYS_DRIVERS    = STARTUP VXKAL KALEXINC $(BSP) $(USB_DRIVER) 
OPERATING_SYS_LIBS       = STARTUP.LIB VXKAL.LIB STARTUP.LIB $(BSP).LIB $(USB_LIB)

OS_LIBS = \
 $(WIND_BASE)\target\lib\arm\ARMARCH4\common\libarch.a \
 $(WIND_BASE)\target\lib\arm\ARMARCH4\common\libcommoncc.a \
 $(WIND_BASE)\target\lib\arm\ARMARCH4\common\libdrv.a \
 $(WIND_BASE)\target\lib\arm\ARMARCH4\common\libdcc.a \
 $(WIND_BASE)\target\lib\arm\ARMARCH4\common\libnet.a \
 $(WIND_BASE)\target\lib\arm\ARMARCH4\common\libos.a \
 $(WIND_BASE)\target\lib\arm\ARMARCH4\common\librpc.a \
 $(WIND_BASE)\target\lib\arm\ARMARCH4\common\libwdb.a \
 $(WIND_BASE)\target\lib\arm\ARMARCH4\common\libwind.a \
 $(WIND_BASE)\target\lib\arm\ARMARCH4\gnu\libcplus.a \
 $(TOOLKIT_LIBS)
 

ENTRY_OBJ       = $(SABINE_ROOT)\$(BSP)\$(CONFIG)\$(APPNAME)\$(RTOS)\$(SWCONFIG)\$(DEBUGDIR)\SYSALIB.O32

#****************************************************************************
#* No OS RTOS Configurations Goes Here
#****************************************************************************
!elseif "$(RTOS)" == "NOOS"


RTOS_INCL      = $(TOOL_INCL)
BSP            = 
OSCFG          =
DELCFG         =

OS_LIBS        = $(TOOLKIT_LIBS)
OPERATING_SYS_DRIVERS    = 
OPERATING_SYS_LIBS       = 

OS_INCLUDE_DIR =

!endif

#****************************************************************************
#* $Log: 
#*  28   mpeg      1.27        5/4/04 12:30:25 AM     Steve Glennon   CR(s) 
#*        9077 9076 : Fixed UCOS clauses to pick up UCOS.LIB correctly. You can
#*         now build with or without BUILDOS=YES
#*        
#*  27   mpeg      1.26        5/3/04 5:03:00 PM      Steve Glennon   CR(s) 
#*        9059 9058 : Fixed typo in UCOS section - included UCKRNSUP.LIB twice 
#*        instead of UCOS.LIB
#*        
#*  26   mpeg      1.25        5/3/04 4:58:17 PM      Steve Glennon   CR(s) 
#*        9059 9058 : Modified UCOS section to support UCOS as a library only, 
#*        or ability to build
#*        using BUILDOS=YES. Reflects addition of UCOSLIB directory to hold 
#*        compiled libary
#*        
#*  25   mpeg      1.24        3/4/04 8:01:43 AM      Ian Mitchell    CR(s) 
#*        8460 : Build the UCKRNSUP module and added new include directories if
#*         the RTOS is UCOS.
#*        
#*  24   mpeg      1.23        2/26/04 5:14:00 PM     Joe Kroesche    CR(s) 
#*        8468 : fixed a problem building and linking when RTOS=NUP and 
#*        BUILDOS=YES
#*  23   mpeg      1.22        11/14/03 8:30:59 AM    Ganesh Banghar  CR(s): 
#*        7926 added new rtos UCOS2.
#*  22   mpeg      1.21        11/5/03 10:50:44 AM    Ganesh Banghar  CR(s): 
#*        7846 removed module UCOSMEM since this heap management code is no 
#*        longer used.
#*  21   mpeg      1.20        10/27/03 5:27:48 PM    Senthil Veluswamy CR(s): 
#*        7726 
#*        Fixed OS libs to refer to the correct version of Psos.
#*  20   mpeg      1.19        9/18/03 10:17:40 AM    Miles Bintz     SCR(s) 
#*        7484 :
#*        added OS_INCLUDE_DIR to UCOS section AGAIN.
#*        
#*  19   mpeg      1.18        9/17/03 6:05:40 PM     QA - Roger Taylor SCR(s) 
#*        7484 :
#*        fixed dumb typo with RTOS_INCL when RTOS=NOOS
#*        
#*  18   mpeg      1.17        9/17/03 12:57:34 PM    QA - Roger Taylor SCR(s) 
#*        7484 :
#*        tool includes (which are basically headers for CRT libs) should be 
#*        included by the RTOS.  Including tool libs should not be absolute 
#*        'cause some RTOS's supply their own CRT headers.  also added psos25 
#*        as another rtos
#*        
#*  17   mpeg      1.16        9/16/03 7:27:06 PM     Miles Bintz     SCR(s) 
#*        7291 :
#*        fixed up builds for ucos
#*        
#*  16   mpeg      1.15        9/16/03 6:43:02 PM     Miles Bintz     SCR(s) 
#*        7291 :
#*        addition of kalexinc to operating sys drivers if extended_kal = yes
#*        
#*  15   mpeg      1.14        9/12/03 6:52:40 PM     Bobby Bradford  SCR(s) 
#*        7291 :
#*        Change specification of WIND_ARCH to use nmake macro expansion
#*        rather than CMD macro expansion
#*        
#*  14   mpeg      1.13        9/9/03 6:57:24 PM      Miles Bintz     SCR(s) 
#*        7291 :
#*        more last minute tweaks to get the new builds going
#*        
#*  13   mpeg      1.12        9/9/03 4:40:46 PM      Miles Bintz     SCR(s) 
#*        7291 :
#*        updates for reworked build system
#*        
#*  12   mpeg      1.11        7/24/03 10:53:16 AM    Larry Wang      SCR(s) 
#*        7033 :
#*        Set START_OBJ to be codeldr.obj if build codeldr with GNU tools.
#*        
#*  11   mpeg      1.10        7/10/03 10:59:46 AM    Larry Wang      SCR(s) 
#*        6924 :
#*        for RTOS==VXWORK and APPNAME==CODELDR, set START_OBJ to be 
#*        SYSALIB.OBJ only.
#*        
#*  10   mpeg      1.9         6/24/02 5:29:20 PM     Larry Wang      SCR(s) 
#*        4081 :
#*        Put syslib.obj in the START_OBJ list so that the linker can pick 
#*        right cache and MMU libraries.
#*        
#*  9    mpeg      1.8         3/19/02 7:42:52 AM     Ganesh Banghar  SCR(s): 
#*        3394 
#*        remove !error line when RTOS=UCOS and MODE=THUMB. since in this 
#*        tracker the ucos now builds with MODE=THUMB.
#*        
#*  8    mpeg      1.7         3/6/02 5:14:54 AM      Ian Mitchell    SCR(s): 
#*        3249 
#*        changed NUP settings
#*        
#*  7    mpeg      1.6         3/5/02 5:50:12 AM      Ganesh Banghar  SCR(s): 
#*        3300 
#*        removed UCOSUAR module. redundant code.
#*        
#*  6    mpeg      1.5         2/28/02 9:21:54 AM     Ian Mitchell    SCR(s): 
#*        3249 
#*        if the RTOS is UCOS and the build mode is THUMB display a warning. 
#*        UCOS can only build in ARM mode.
#*        
#*  5    mpeg      1.4         2/28/02 8:53:46 AM     Ian Mitchell    SCR(s): 
#*        3249 
#*        Set START_OBJ too!
#*        
#*  4    mpeg      1.3         2/26/02 11:02:18 PM    Tim Ross        SCR(s) 
#*        3260 :
#*        Replaced $(START).LIB w/ STARTUP.LIB in VxWOrks OPERATING_SYS_LIB 
#*        case.
#*        
#*  3    mpeg      1.2         2/26/02 12:25:16 PM    Bobby Bradford  SCR(s) 
#*        3249 :
#*        Removed spaces from line continuations "\ " so that they
#*        would work properly in nmake
#*        
#*  2    mpeg      1.1         2/26/02 8:22:50 AM     Ian Mitchell    SCR(s): 
#*        3249 
#*        Set OPERATING_SYS_DRIVERS and LIBS for each RTOS
#*        
#*  1    mpeg      1.0         2/25/02 8:58:40 AM     Bobby Bradford  
#* $
#****************************************************************************

