#***************************************************************************
#                            Conexant Systems
#***************************************************************************
#
# Filename:       modem.mak
#
# Description:    Top Level MODEM Configuration File for MAKE Files
#
# Author:         Bobby Bradford
#
# Copyright Conexant Systems, 2003
# All Rights Reserved.
#
#***************************************************************************
# $Header: modem.mak, 12, 5/19/04 3:17:58 PM, Bobby Bradford$
#***************************************************************************


#***************************************************************************
# 1)  Verify that none of the internal macros are already being used ...
#     generate an error if they are

!if "$(CNXT_MODEM_DEBUG_SUFFIX)" != ""
!error Cannot build with CNXT_MODEM_DEBUG_SUFFIX pre-defined as $(CNXT_MODEM_DEBUG_SUFFIX)
!endif

!if "$(CNXT_MODEM_DRIVERS)" != ""
!error Cannot build with CNXT_MODEM_DRIVERS pre-defined as $(CNXT_MODEM_DRIVERS)
!endif

!if "$(CNXT_MODEM_LIBRARY)" != ""
!error Cannot build with CNXT_MODEM_LIBRARY pre-defined as $(CNXT_MODEM_LIBRARY)
!endif

!if "$(CNXT_MODEM_EXTRA_DRIVERS)" != ""
!error Cannot build with CNXT_MODEM_EXTRA_DRIVERS pre-defined as $(CNXT_MODEM_EXTRA_DRIVERS)
!endif

!if "$(CNXT_MODEM_USEREDIR)" != ""
!error Cannot build with CNXT_MODEM_USEREDIR pre-defined as $(CNXT_MODEM_USEREDIR)
!endif


#***************************************************************************
# 2)  If CNXT_MODEM_LIBDIR is not specified, or if we are forcing a modem
#     build, then set the CNXT_MODEM_LIBDIR macro to local build tree

!if "$(CNXT_MODEM_LIBDIR)" == ""
CNXT_MODEM_LIBDIR = $(SABINE_ROOT)\LIB\MODEM
!elseif "$(INTERNAL_BUILD)" == "YES" && "$(BUILDMODEM)" == "YES"
!undef CNXT_MODEM_LIBDIR
CNXT_MODEM_LIBDIR = $(SABINE_ROOT)\LIB\MODEM
!endif


#***************************************************************************
# 3)  Determine whether to use RELEASE or DEBUG version of the modem

!if "$(DEBUG)" != "NO" 
!if "$(CNXT_MODEM_DEBUG)" == "YES"
CNXT_MODEM_DEBUG_SUFFIX = _DEBUG
!elseif "$(APPNAME)" == "P93TEST"
CNXT_MODEM_DEBUG_SUFFIX = _DEBUG
!elseif "$(APPNAME)" == "MDMTEST"
CNXT_MODEM_DEBUG_SUFFIX = _DEBUG
!elseif "$(APPNAME)" == "TESTH"
CNXT_MODEM_DEBUG_SUFFIX = _DEBUG
!endif
!endif


#***************************************************************************
# 4)  Check for Valid MODEM_HW_TYPE specification ... setup make macros

#
# SmartMDP Modem Support
#
!if "$(MODEM_HW_TYPE)" == "INTERNAL_SMARTMDP_MODEM"
CNXT_MODEM_DRIVERS = P93HOST2_D
CNXT_MODEM_LIBRARY = $(CNXT_MODEM_LIBDIR)\P93MODEM2$(CNXT_MODEM_DEBUG_SUFFIX).$(EXT_LIB)
!if "$(INTERNAL_BUILD)" == "YES" && "$(BUILDMODEM)" == "YES"
CNXT_MODEM_EXTRA_DRIVERS = $(CNXT_MODEM_EXTRA_DRIVERS) P93MODEM2_D
!endif


#
# SmartDAA (Soft) Modem Support
#
!elseif "$(MODEM_HW_TYPE)" == "INTERNAL_SMARTDAA_MODEM"

# #
# # SmartDAA FLIPPER Modem Definition
# #
!if "$(MODEM_SW_TYPE)" == "FLIPPER_V22B_MODEM"
CNXT_MODEM_DRIVERS = FLIPHOST_D
CNXT_MODEM_LIBRARY = $(CNXT_MODEM_LIBDIR)\FLIPPER$(CNXT_MODEM_DEBUG_SUFFIX).$(EXT_LIB)
!if "$(INTERNAL_BUILD)" == "YES" && "$(BUILDMODEM)" == "YES"
CNXT_MODEM_EXTRA_DRIVERS = $(CNXT_MODEM_EXTRA_DRIVERS) FLIPPER_D
!endif

# #
# # SmartDAA CORSICA Modem Definition
# #
!elseif "$(MODEM_SW_TYPE)" == "CORSICA_V34_MODEM"
CNXT_MODEM_DRIVERS = CORSICAHOST_D
CNXT_MODEM_LIBRARY = $(CNXT_MODEM_LIBDIR)\CORSICA$(CNXT_MODEM_DEBUG_SUFFIX).$(EXT_LIB)
!if "$(INTERNAL_BUILD)" == "YES" && "$(BUILDMODEM)" == "YES"
CNXT_MODEM_EXTRA_DRIVERS = $(CNXT_MODEM_EXTRA_DRIVERS) CORSICA_D
!endif


# #
# # SmartDAA Invalid Modem Definition
# #
!else
!message Softmodem type specified is not yet supported in the modem.mak file.
!message Please change the type or add the appropriate support
!error MODEM_SW_TYPE = $(MODEM_SW_TYPE) not supported in the build process
!endif

#
# External Hardware Modem Support
#
!elseif "$(MODEM_HW_TYPE)" == "EXTERNAL_SERIAL_MODEM"
CNXT_MODEM_DRIVERS = SERMODEM_D


!elseif "$(MODEM_HW_TYPE)" == "HW_NONE"
# Do Nothing Here ... placeholder to keep from failing


!else
!message Modem specified is not yet supported in the modem.mak file.
!message Please add the appropriate support in Step 2) and rebuild
!error MODEM_HW_TYPE = $(MODEM_HW_TYPE) not supported in the build process

!endif


#***************************************************************************
# 5)  Check to see if REDIRTSK needs to be included ... currently only
#     P93TEST, MDMTEST, and possibly WATCHTV for now

!if "$(MODEM_HW_TYPE)" != "HW_NONE"

!if "$(APPNAME)" == "P93TEST"
CNXT_MODEM_USEREDIR = YES

!elseif "$(APPNAME)" == "MDMTEST"
CNXT_MODEM_USEREDIR = YES

!elseif "$(APPNAME)" == "CORSICATEST"
CNXT_MODEM_USEREDIR = YES

!elseif "$(APPNAME)" == "WATCHTV"
CNXT_MODEM_USEREDIR = YES

!elseif "$(APPNAME)" == "TESTH"
!if "$(TESTH_GROUP)" == "WTV"
CNXT_MODEM_USEREDIR = YES
!endif

!endif

!if "$(CNXT_MODEM_USEREDIR)" == "YES"
CNXT_MODEM_DRIVERS = $(CNXT_MODEM_DRIVERS) REDIRTSK_D

!if "$(DTESPEED)" == ""
!if "$(MODEM_HW_TYPE)" == "INTERNAL_SMARTDAA_MODEM"
!if "$(MODEM_SW_TYPE)" == "CORSICA_V34_MODEM"
DTESPEED=115200
!else
DTESPEED=19200
!endif
!else
DTESPEED=57600
!endif
!endif

EXTRA_ARM_C_FLAGS      = $(EXTRA_ARM_C_FLAGS)      -DTERM_RATE=$(DTESPEED)
EXTRA_ARM_ASM_FLAGS    = $(EXTRA_ARM_ASM_FLAGS)
EXTRA_THUMB_C_FLAGS    = $(EXTRA_THUMB_C_FLAGS)    -DTERM_RATE=$(DTESPEED)
EXTRA_THUMB_ASM_FLAGS  = $(EXTRA_THUMB_ASM_FLAGS)
EXTRA_MAKEDEPEND_FLAGS = $(EXTRA_MAKEDEPEND_FLAGS) -DTERM_RATE=$(DTESPEED)

!endif

!endif


#***************************************************************************
# 6)  Check to see if a CMODULE needs to be included ... currently only
#     OTV12CTL and OTV2CTRL

!if "$(MODEM_HW_TYPE)" != "HW_NONE" 

!if "$(APPNAME)" == "OTV12CTL" || "$(APPNAME)" == "OTV12CTL_S"
CNXT_MODEM_DRIVERS = $(CNXT_MODEM_DRIVERS) CMODULE_D

!elseif "$(APPNAME)" == "OTV2CTRL"
CNXT_MODEM_DRIVERS = $(CNXT_MODEM_DRIVERS) CMODULE_D

!endif

!endif


#***************************************************************************
# 7)  Check to see if CorsicaTest needs to be included as a "driver"

!if "$(MODEM_HW_TYPE)" == "INTERNAL_SMARTDAA_MODEM"
!if "$(MODEM_SW_TYPE)" == "CORSICA_V34_MODEM"
!if "$(APPNAME)" == "WATCHTV"
CNXT_MODEM_DRIVERS = $(CNXT_MODEM_DRIVERS) CORSICATEST_D
!endif
!endif
!endif


#***************************************************************************
# $Log: 
#  12   mpeg      1.11        5/19/04 3:17:58 PM     Bobby Bradford  CR(s) 9249
#         9250 : Change default DTESPEED for modem redirector port from 19200 
#        to 115200 for CORSICA modem builds only.
#  11   mpeg      1.10        3/23/04 2:40:01 PM     Bobby Bradford  CR(s) 8590
#         8636 : Add CORSICATEST to the driver list if building watchtv for 
#        corsica modem configuration
#  10   mpeg      1.9         2/18/04 3:34:35 PM     Bobby Bradford  CR(s) 8414
#         : For WATCHTV, always use the REDIRECTOR task if the modem is 
#        included in the build.
#  9    mpeg      1.8         2/2/04 5:18:22 PM      Bobby Bradford  CR(s) 8277
#         : Add support for CORSICATEST application and CORSICA modem types
#  8    mpeg      1.7         9/23/03 9:48:26 AM     Bobby Bradford  SCR(s) 
#        7418 :
#        Add support for the new SERMODEM driver for serial/controllered
#        modems.  Also change the CMODULE specification to always use CMODULE
#        for any modems (CMODULE assumes the Conexant Modem API is present).
#        
#  7    mpeg      1.6         9/10/03 6:16:40 PM     Bobby Bradford  SCR(s) 
#        7291 :
#        Move modem library check from modem.mak to link.mak
#        
#  6    mpeg      1.5         8/15/03 12:28:50 PM    Bobby Bradford  SCR(s) 
#        7260 7262 :
#        Added TESTH to the list of applications to build DEBUG libraries for
#        
#  5    mpeg      1.4         8/12/03 9:41:20 AM     Bobby Bradford  SCR(s) 
#        7213 7214 :
#        Modify use of MODEM_HW_TYPE to reflect changes in HWCONFIG.CFG
#        
#  4    mpeg      1.3         5/29/03 4:34:18 PM     Bobby Bradford  SCR(s) 
#        6629 6630 :
#        Added support for SECURE=YES when building for OTV12CTL
#        note: application name changes to OTV12CTL_S and the cmodule
#        code in this make file was not allowing for that condtion.
#        
#  3    mpeg      1.2         5/2/03 1:30:54 PM      Bobby Bradford  SCR(s) 
#        6037 :
#        Add diagnostic information for missing modem library
#        
#  2    mpeg      1.1         5/1/03 2:21:20 PM      Bobby Bradford  SCR(s) 
#        6037 :
#        Add support for TESTH application, WTV case
#        
#  1    mpeg      1.0         4/30/03 9:35:58 AM     Bobby Bradford  
# $
#  
#     Rev 1.7   23 Sep 2003 08:48:26   bradforw
#  SCR(s) 7418 :
#  Add support for the new SERMODEM driver for serial/controllered
#  modems.  Also change the CMODULE specification to always use CMODULE
#  for any modems (CMODULE assumes the Conexant Modem API is present).
#  
#     Rev 1.6   10 Sep 2003 17:16:40   bradforw
#  SCR(s) 7291 :
#  Move modem library check from modem.mak to link.mak
#  
#     Rev 1.5   15 Aug 2003 11:28:50   bradforw
#  SCR(s) 7260 7262 :
#  Added TESTH to the list of applications to build DEBUG libraries for
#  
#     Rev 1.4   12 Aug 2003 08:41:20   bradforw
#  SCR(s) 7213 7214 :
#  Modify use of MODEM_HW_TYPE to reflect changes in HWCONFIG.CFG
#  
#     Rev 1.3   29 May 2003 15:34:18   bradforw
#  SCR(s) 6629 6630 :
#  Added support for SECURE=YES when building for OTV12CTL
#  note: application name changes to OTV12CTL_S and the cmodule
#  code in this make file was not allowing for that condtion.
#  
#     Rev 1.2   02 May 2003 12:30:54   bradforw
#  SCR(s) 6037 :
#  Add diagnostic information for missing modem library
#  
#     Rev 1.1   01 May 2003 13:21:20   bradforw
#  SCR(s) 6037 :
#  Add support for TESTH application, WTV case
#  
#     Rev 1.0   30 Apr 2003 08:35:58   bradforw
#  SCR(s) 6037 :
#  Rule file for building with modem libraries
#  
#
#***************************************************************************

