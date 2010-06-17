###############################################################################
#                                                                             #
# TOPMAKE.MAK                                                                 #
#                                                                             #
# Top level makefile used to build Sabine code images and testcases           #
#                                                                             #
# Dave Wilson, 3/5/98                                                         #
#                                                                             #
# Copyright (c) Rockwell Semiconductor Systems, 1998                          #
#                                                                             #
###############################################################################
# $Id: topmake.mak,v 1.1 2007/10/24 09:39:11 chenhb27 Exp $
###############################################################################

###############################################################################
# Here, include the top-level product configuration MAKE file
###############################################################################
!include $(SDCAPPS_ROOT)\make\common.mak
!include $(SDCAPPS_ROOT)\make\builddirs.mak
!include $(SDCAPPS_ROOT)\make\product.mak

#################################################
# Ensure that "all" is the default target built #
#################################################
target: all

!if "$(DRV_INCL)"==""
DRV_INCL=-DDRIVER_INCL_$(ALL_DRIVERS: = -DDRIVER_INCL_) ZZZ
DRV_INCL=$(DRV_INCL:-DDRIVER_INCL_ ZZZ=)
DRV_INCL=$(DRV_INCL: ZZZ=)
DRV_INCL=$(DRV_INCL:-DDRIVER_INCL_ =)

HPDRV_INCL=-DDRIVER_INCL_$(SDCAPP_DRIVERS: = -DDRIVER_INCL_) ZZZ
HPDRV_INCL=$(HPDRV_INCL:-DDRIVER_INCL_ ZZZ=)
HPDRV_INCL=$(HPDRV_INCL: ZZZ=)
HPDRV_INCL=$(HPDRV_INCL:-DDRIVER_INCL_ =)

DRV_INCL= $(DRV_INCL) $(HPDRV_INCL)
!endif

###############################################################################
# Get the file list for a specific driver if we are at this stage in the make #
###############################################################################

!if "$(DRIVER)" != ""
!include $(SABINE_ROOT)\$(DRIVER)\$(DRIVER).mak
!endif

!if "$(SDCDRIVER)" != ""
!include $(SDCAPPS_ROOT)\$(SDCDRIVER)\$(SDCDRIVER).mak
!endif

#########################################################
# Set environment variables to allow recursively called #
# copies of nmake to see extra compiler/assembler flags #
#########################################################
set:
	$(AT)REM #######################################
	$(AT)REM # Setting extra environment variables #
	$(AT)REM #######################################
	$(AT)set APPNAME=$(APPNAME)
	$(AT)set AUDIO_FLAGS=$(AUDIO_FLAGS)
	$(AT)set ARM_VERSION=$(ARM_VERSION)
	$(AT)set ARM_TOOLKIT=$(ARM_TOOLKIT)
	$(AT)set AT=$(AT)
	$(AT)set BSP=$(BSP)
	$(AT)set BOARD_FLAGS=$(BOARD_FLAGS)
	$(AT)set BUFFER_FLAGS=$(BUFFER_FLAGS)
	$(AT)set CPU=$(CPU)
	$(AT)set CONFIG=$(CONFIG)
	$(AT)set CUSTOMER=$(CUSTOMER)
	$(AT)set DEBUG=$(DEBUG)
	$(AT)set DEBUGDIR=$(DEBUGDIR)
	$(AT)set DELCFG=$(DELCFG)
	$(AT)set DEVICE_FLAGS=$(DEVICE_FLAGS)
	$(AT)set DLOAD=$(DLOAD)
	$(AT)set DRV_INCL=$(DRV_INCL)
	$(AT)set DRIVER_LIBS=$(DRIVER_LIBS)
	$(AT)set EXTENDED_KAL=$(EXTENDED_KAL)
	$(AT)set EXTRA_ARM_ASM_FLAGS=$(EXTRA_ARM_ASM_FLAGS)
	$(AT)set EXTRA_ARM_C_FLAGS=$(EXTRA_ARM_C_FLAGS)
	$(AT)set EXTRA_INCLUDE_DIRS=$(EXTRA_INCLUDE_DIRS)
	$(AT)set EXTRA_THUMB_C_FLAGS=$(EXTRA_THUMB_C_FLAGS)
	$(AT)set EXTRA_THUMB_ASM_FLAGS=$(EXTRA_THUMB_ASM_FLAGS)
	$(AT)set EXTRA_MAKEDEPEND_FLAGS=$(EXTRA_MAKEDEPEND_FLAGS)
	$(AT)set FAST=$(FAST)
	$(AT)set GEN_CMDS=$(GEN_CMDS)
	$(AT)set FILE_FLAGS=$(FILE_FLAGS)
	$(AT)set IMAGE_TYPE=$(IMAGE_TYPE)
	$(AT)set INCLUDE=$(SABINE_ROOT)\include;$(EXTRA_INCLUDE_DIRS: =;);$(SABINE_ROOT)\include\$(CONFIG)\$(SWCONFIG);$(SDCAPP_OUTPUT_DIR)
	$(AT)set INTER_OUTPUT_DIR=$(INTER_OUTPUT_DIR)
	$(AT)set LOG=$(LOG)
	$(AT)set MAKE_VERSION=$(MAKE_VERSION)
	$(AT)set MODE=$(MODE)
	$(AT)set OPENTV_FLAGS=$(OPENTV_FLAGS)
	$(AT)set PACKING=$(PACKING)
	$(AT)set REVISION_FLAGS=$(REVISION_FLAGS)
	$(AT)set ROM_FLAGS=$(ROM_FLAGS)
	$(AT)set RTOS=$(RTOS)
	$(AT)set SDCAPP_DRIVERS= $(SDCAPP_DRIVERS)
	$(AT)set SDCAPP_OUTPUT_DIR=$(SDCAPP_OUTPUT_DIR)
	$(AT)set SDCAPPS_ROOT=$(SDCAPPS_ROOT)
	$(AT)set SOURCE=$(SOURCE)
	$(AT)set SWCONFIG=$(SWCONFIG)
	$(AT)set TYPE=$(TYPE)
	$(AT)set TRACE=$(TRACE)
	$(AT)set TRACE_OUTPUT=$(TRACE_OUTPUT)
	$(AT)set UART_FLAGS=$(UART_FLAGS)
	$(AT)set VERBOSE=$(VERBOSE)
	$(AT)set VERSION=$(VERSION)
	$(AT)set VIDEO_FLAGS=$(VIDEO_FLAGS)


# Force EXTRA_INCLUDE_DIRS target to be made. If these directories already
# exist, these commands won't be executed unless this dependency is listed.
force_extra_include_dirs:

$(SDCAPP_OUTPUT_DIR):
	-$(AT)md $(@) 2> nul:

$(LIB_OUTPUT_DIR):
	-$(AT)md $(@) 2> nul:
	
$(IMG_OUTPUT_DIR):
	$(AT)if not exist $(IMG_OUTPUT_DIR) md $(@) > nul: 2>&1
	
always:

######################################
# Build a version number header file #
######################################
$(SDCAPP_OUTPUT_DIR)\bldver.h: always
	-@echo /**********************************************/  > $@
	-@echo /* Automatically Generated File - DO NOT EDIT */ >> $@
	-@echo /**********************************************/ >> $@
	-@echo #ifndef _BLDVER_H_ >> $@
	-@echo #define _BLDVER_H_ >> $@
!if "$(VERSION)" == "USE"
	-@echo #define VERSTRING "Development" >> $@
!else
	-@echo #define VERSTRING "$(VERSION)"  >> $@
!endif
	-@echo #define CONFIGSTRING "$(CONFIG)"  >> $@
	-@echo #define SWCONFIGSTRING "$(SWCONFIG)"  >> $@

	-@echo #endif             >> $@

#######################################################################
# Generate a file including compiler switches defining labels for all #
# drivers in this particular build.                                   #
#######################################################################
!include $(SDCAPPS_ROOT)\make\config.mak
!include $(SDCAPPS_ROOT)\make\flags.mak
!include $(SDCAPPS_ROOT)\make\link.mak
!include $(SDCAPPS_ROOT)\make\clean.mak

$(SDCAPP_OUTPUT_DIR)\drv_incl.h: $(SDCAPP_OUTPUT_DIR) $(MAKEDIR)\Makefile 
	-@echo  /* This header is automatically generated!  Do not edit! */ > $@
	-$(AT)"$(TOOLEXE_PATH)\c2obj" -d "#define DRIVER_INCL_" $(ALL_DRIVERS) $(SDCAPP_DRIVERS) >> $@

drv_build: buildlib 

buildlib: $(SDCAPPS_ROOT)\MAKE\BUILDLIB.MAK $(SDCAPPS_ROOT)\MAKE\DEPEND.MAK
	@echo Visiting directory     $(DRIVER)
	-$(AT)cd $(SABINE_ROOT)\$(DRIVER)\$(INTER_OUTPUT_DIR)
	-$(AT)$(MAKE) /b /NOLOGO /f$(SDCAPPS_ROOT)\MAKE\DEPEND.MAK DRIVER=$(DRIVER) all 2>driver.log
	-$(AT)type driver.log
	-$(AT)$(MAKE) /b /NOLOGO /f$(SDCAPPS_ROOT)\MAKE\BUILDLIB.MAK DRIVER=$(DRIVER) all 2>driver.log 
	-$(AT)type driver.log
	-$(AT)del driver.log > nul: 2>&1
	-$(AT)cd $(MAKEDIR)

buildhplib: $(SABINE_ROOT)\MAKE\RULES.MAK $(SDCAPPS_ROOT)\MAKE\SDCDEPEND.MAK
	@echo Visiting directory     SDCAPPS\$(SDCDRIVER)
	-$(AT)cd $(SDCAPPS_ROOT)\$(SDCDRIVER)\$(INTER_OUTPUT_DIR)
	-$(AT)$(MAKE) /b /NOLOGO /f$(SDCAPPS_ROOT)\MAKE\SDCDEPEND.MAK SDCDRIVER=$(SDCDRIVER) all 2>driver.log
	-$(AT)type driver.log
	-$(AT)$(MAKE) /b /NOLOGO /f$(SDCAPPS_ROOT)\MAKE\BUILDSDCLIB.MAK SDCDRIVER=$(SDCDRIVER) all 2>driver.log 
	-$(AT)type driver.log
	-$(AT)del driver.log > nul: 2>&1
	-$(AT)cd $(MAKEDIR)
	
################################################################################
# This is the recursive bit that makes the objects for each driver             #
# Note that we need 2 separate versions here. Without this, nmake always pulls #
# the driver makefile from the library since the latest mak file is always     #
# older than the mav file generated when the file is "put".                    #
################################################################################
!if "$(ALL_DRIVERS)" != ""
$(ALL_DRIVERS): always
	-$(AT)md $(SABINE_ROOT)\$(*B)\$(INTER_OUTPUT_DIR) 2> nul:
	-$(AT)md $(SABINE_ROOT)\$(*B)\$(INTER_OUTPUT_DIR) 2> nul:
	-$(AT)$(MAKE) /b /NOLOGO /f$(SDCAPPS_ROOT)\make\topmake.mak SDCAPP_DRIVERS= SDCDRIVER= ALL_DRIVERS=$(*B) DRIVER=$(*B) driver
!endif

!if "$(SDCAPP_DRIVERS)" != ""
$(SDCAPP_DRIVERS): always
	-$(AT)md $(SDCAPPS_ROOT)\$(*B)\$(INTER_OUTPUT_DIR) 2> nul:
	-$(AT)md $(SDCAPPS_ROOT)\$(*B)\$(INTER_OUTPUT_DIR) 2> nul:
	-$(AT)$(MAKE) /b /NOLOGO /f$(SDCAPPS_ROOT)\make\topmake.mak SDCAPP_DRIVERS=$(*B) SDCDRIVER=$(*B) ALL_DRIVERS= DRIVER= sdcdriver 
!endif

!if "$(TYPE)" == "LOCAL"

all:  do_cfgtoh set $(SDCAPP_OUTPUT_DIR)\drv_incl.h $(LIB_OUTPUT_DIR) $(IMG_OUTPUT_DIR) $(SDCAPP_OUTPUT_DIR)\bldver.h \
		 $(ALL_DRIVERS) $(SDCAPP_DRIVERS) app_link
	-@echo.
	-@echo *** Finished building ***
	-@echo.

driver: drv_build
sdcdriver: buildhplib
!endif

!if "$(TYPE)" == "CLEAN"
	
all: set do_cfgtoh_clean drv_incl_clean $(OUTPUTLIB) $(ALL_DRIVERS) $(SDCAPP_DRIVERS) clean_app_image
	-@echo.
	-@echo *** Finished cleaning ***
	-@echo.

driver: drv_clean
sdcdriver: sdcdrv_clean

!endif

