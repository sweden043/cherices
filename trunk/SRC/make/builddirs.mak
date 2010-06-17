###############################################################################
#                                                                             #
# CNXT MPEG build support file: rules.mak                                     #
#                                                                             #
# Author: M. Bintz 7/24/03                                                    #
#                                                                             #
# Compiler rules for all toolchains and file types.                           #
#                                                                             #
###############################################################################


########################## Build Directories ######################
!ifndef BUILDDIRS
BUILDDIRS=YES

!if "$(APPDIR)"==""
APPDIR             = $(MAKEDIR)
!endif

!if "$(DEBUG)" == "NO"
INTER_OUTPUT_DIR   = $(CONFIG)\$(APPNAME)\$(RTOS)\$(SWCONFIG)\RELEASE
OBJONLY_OUTPUT_DIR = $(SABINE_ROOT)\LIB\$(CONFIG)\OBJONLY\$(RTOS)\$(SWCONFIG)\RELEASE
DEBUGDIR           = RELEASE
!else
INTER_OUTPUT_DIR   = $(CONFIG)\$(APPNAME)\$(RTOS)\$(SWCONFIG)\DEBUG
OBJONLY_OUTPUT_DIR = $(SABINE_ROOT)\LIB\$(CONFIG)\OBJONLY\$(RTOS)\$(SWCONFIG)\DEBUG
DEBUGDIR           = DEBUG
!endif

OUTPUT_DIR      = $(APPDIR)\$(INTER_OUTPUT_DIR)\$(VERSION)
CONFIG_FILE_DIR = $(SABINE_ROOT)\include\$(CONFIG)\$(SWCONFIG)
BLD_LIB_DIR     = $(SABINE_ROOT)\lib\$(INTER_OUTPUT_DIR)
OBJONLY_LIBDIR  = $(OBJONLY_OUTPUT_DIR)
TOOLEXE_PATH    = $(SABINE_ROOT)\TOOLEXE

!endif
#****************************************************************************
#* $Log: 
#*  2    mpeg      1.1         3/3/04 9:32:27 PM      Miles Bintz     CR(s) 
#*        8509 : only allow builddirs to be included once
#*        
#*        
#*  1    mpeg      1.0         9/9/03 4:40:36 PM      Miles Bintz     
#* $
#  
#     Rev 1.0   09 Sep 2003 15:40:36   bintzmf
#  SCR(s) 7291 :
#  reworked build system
#  
#     Rev 1.2   04 Sep 2003 14:04:20   bintzmf
#  
#     Rev 1.1   28 Aug 2003 15:25:22   bintzmf
#  
#     Rev 1.0   14 Aug 2003 13:41:44   bintzmf
#  new build system
#*  
#*  
#****************************************************************************


