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
#* $Header: /cvs/proj-conexant24153/SRC/SDCApps/MAKE/rtos.mak,v 1.1 2007/10/24 09:39:11 chenhb27 Exp $
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

ENTRY_OBJ             = $(SABINE_ROOT)\STARTUP\$(INTER_OUTPUT_DIR)\startup.o32

#****************************************************************************
#* UCOS RTOS Configurations Goes Here
#****************************************************************************
!elseif "$(RTOS)" == "UCOS"


BSP            = UCOSBSP
DELCFG         = NO

OS_INCLUDE_DIR           = UCOSINC 

OPERATING_SYS_DRIVERS    = UCOS STARTUP UCOSKAL $(BSP)
OPERATING_SYS_LIBS       = UCOS.LIB STARTUP.LIB UCOSKAL.LIB $(BSP).LIB

RTOS_INCL = $(TOOL_INCL);$(SABINE_ROOT)\UCOSINC;$(SABINE_ROOT)\$(BSP);$(TOOL_INCL);$(SABINE_ROOT)\UCOSKAL

ENTRY_OBJ       = $(SABINE_ROOT)\STARTUP\$(INTER_OUTPUT_DIR)\startup.o32
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

ENTRY_OBJ       = $(SABINE_ROOT)\STARTUP\$(INTER_OUTPUT_DIR)\startup.o32


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


!if "$(RTOS)" == "UCOS"
RTOS_DRIVERS = UCOSKAL UCKRNSUP UCOSBSP

!elseif "$(RTOS)" == "NUP"
RTOS_DRIVERS = NUPBSP NUPKAL

!if "$(ARM_TOOLKIT)" == "ADS"
RTOS_LIBS = $(SABINE_ROOT)\NUPLIB_P1\plus.ads
!else
RTOS_LIBS = $(SABINE_ROOT)\NUPLIB_P1\plus.lib

!endif

!else
!error  This RTOS is not supported by SDC applications!

!endif
#****************************************************************************
#* $Log
#* $
#****************************************************************************

