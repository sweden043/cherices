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
# $Id: topmake.mak,v 1.243, 2004-04-27 20:37:46Z, Miles Bintz$
###############################################################################


###############################################################################
# Perform a run-time check of the MAKE_VERSION to ensure the the APPLICATION
# MAKEFILE and TOPMAKE.MAK are in sync.
###############################################################################
!if "$(MAKE_VERSION)" != "1"
!message !!! $(APPNAME)\makefile !!!
!message This makefile has not yet been updated to support the new
!message MAKE system.  Please refer to Tracker No. 3201 for details.
!error MAKE_VERSION mismatch
!endif

###############################################################################
# If we get here, the APPLICATION and TOPMAKE.MAK are in sync, now do the 
# normal processing
###############################################################################
# Here, include the top-level product configuration MAKE file
###############################################################################
!include $(SABINE_ROOT)\make\builddirs.mak
!include $(SABINE_ROOT)\make\product.mak


!if "$(IFTYPE)"==""
!if "$(QAM_DEMOD_IF_TYPE)"=="QAM_DEMOD_IF_TYPE_NA"
IFTYPE=na
!else
IFTYPE=euro
!endif
!endif

##############################################################################
# If the current software config file calls for any additional drivers to be #
# included in the build, append these to some of the existing macros         #
##############################################################################
!if "$(SWCONFIG_DRIVERS)" != ""
ALL_DRIVERS = $(SWCONFIG_DRIVERS) $(ALL_DRIVERS) 
!endif

!if "$(SWCONFIG_LIBS)" != ""
EVENMOREDRIVER_LIBS = $(EVENMOREDRIVER_LIBS) $(SWCONFIG_LIBS)
!endif

	          ##################################################
	          ##################################################
	          ## WARNING ! Change nothing at all below here ! ##
	          ##################################################
	          ##################################################



#################################################
# Ensure that "all" is the default target built #
#################################################
target: all

#########################################################################################
# If makefile defines a relocate start object, include the relocated rom_ram image type #
# This ensures that old makefile which don't include this macro will still build.       #
#########################################################################################
!if "$(RELOC_OBJ)" != ""
RELOC_TARGET = rom_ram_bin
!else
RELOC_TARGET =
!endif

################################################
# Has the default image name been overridden ? #
################################################
!if "$(APPNAME)" == ""
APPNAME=$(CONFIG)
!endif

################################################
# Has the default image type been overridden ? #
################################################
!if "$(IMAGE_TYPE)" == ""
IMAGE_TYPE=GENERIC
!endif

###############################################
# Default options - don't change this section #
###############################################
!if "$(VERSION)"==""
VERSION=USE
!endif

!if "$(CUSTOMER)"==""
CUSTOMER=CNXT
!endif

!if "$(TYPE)"==""
TYPE=LOCAL
!endif

!if "$(DEBUG)"==""
DEBUG=YES
!endif

!if "$(DEBUG)"=="YES"
!if "$(TRACE_OUTPUT)"==""
TRACE_OUTPUT=YES
!endif
!else # DEBUG != YES
TRACE_OUTPUT=NO
!endif


!if "$(VERBOSE)"==""
VERBOSE=NO
!endif

!if "$(LOG)"==""
LOG=YES
!endif

!if "$(PACKING)" == ""
PACKING=1
!endif

!if "$(RTOS)" == ""
!endif

!if "$(DLOAD)"==""
DLOAD = NO
!endif

!if "$(VERBOSE)" == "YES"
# If verbose is YES then we will echo the commands we will run
AT =   
!else
#else we will keep the commands "quiet"
AT = @
!endif

# When building ASM files with GNU, convert the ARM
# version and build with gas?
!if "$(ARM2ASM)" == ""
ARMASM2GNUASM=NO
!endif

!if "$(DEBUG)" == "YES"
DEBUGDIR = DEBUG
!else
DEBUGDIR = RELEASE
!endif

!if "$(DRV_INCL)"==""
DRV_INCL=-DDRIVER_INCL_$(ALL_DRIVERS: = -DDRIVER_INCL_) ZZZ
DRV_INCL=$(DRV_INCL:-DDRIVER_INCL_ =)
DRV_INCL=$(DRV_INCL:-DDRIVER_INCL_ ZZZ=)
DRV_INCL=$(DRV_INCL: ZZZ=)
!endif



#################################################
# Has a combined error log file been specified? #
#################################################
!if "$(COMBINED_ERROR_LOG)" == ""
COMBINED_ERROR_LOG=$(APPDIR)\$(INTER_OUTPUT_DIR)\$(VERSION)\build.log
!endif


###############################################################################
# Get the file list for a specific driver if we are at this stage in the make #
###############################################################################

!if "$(DRIVER)" != ""
!include $(SABINE_ROOT)\$(DRIVER)\$(DRIVER).mak
!endif


################################################################################
# Now, process EXTRA_DRIVERS, using a simple recursive NMAKE of "makefile" in  #
# the EXTRA_DRIVERS directory                                                  #
################################################################################
!if "$(EXTRA_DRIVERS)" != ""

$(EXTRA_DRIVERS):
	-@if not exist $(SABINE_ROOT)\$(*B)\nul mkdir $(SABINE_ROOT)\$(*B)
	-@cd $(SABINE_ROOT)\$(*B)
!if "$(TYPE)" == "GET"
	@echo vcsdir=$(PVCS_ROOT)\$(*B)>vcs.cfg
	@get $(GET_OPTIONS) -V$(VERSION) makefile
!elseif "$(TYPE)"=="COPY"
	$(AT)xcopy /K /R /Y $(SOURCE)\$(*B)\makefile 
!endif
	$(AT)$(MAKE) /NOLOGO APPDIR=$(SABINE_ROOT)\$(*B)
	-@cd $(MAKEDIR)


!endif

################################################################################
# Now, process EXTRA_APPS, using a simple recursive NMAKE of "makefile" in     #
# the EXTRA_APPS directory                                                     #
################################################################################
!if "$(EXTRA_APPS)" != ""
$(EXTRA_APPS):
	-@echo.
	-@echo Chaining EXTRA_APPs:    $(*B)
	-@if not exist $(SABINE_ROOT)\$(*B)\nul mkdir $(SABINE_ROOT)\$(*B)
	-@cd $(SABINE_ROOT)\$(*B)
!if "$(TYPE)" == "GET"
	@echo vcsdir=$(PVCS_ROOT)\$(*B)>vcs.cfg
	@get $(GET_OPTIONS) -V$(VERSION) makefile
!elseif "$(TYPE)"=="COPY"
	$(AT)xcopy /K /R /Y $(SOURCE)\$(*B)\makefile 
!endif
	$(AT)$(MAKE) /NOLOGO APPDIR=$(SABINE_ROOT)\$(*B)
	-@cd $(MAKEDIR)

!endif


################################################################################
# This is the recursive bit that makes the objects for each driver             #
# Note that we need 2 separate versions here. Without this, nmake always pulls #
# the driver makefile from the library since the latest mak file is always     #
# older than the mav file generated when the file is "put".                    #
################################################################################

$(OPERATING_SYS_DRIVERS) $(ALL_DRIVERS): 
!if "$(TYPE)" != "COPY"
	-@md $(SABINE_ROOT)\$(*B)\$(CONFIG)\$(APPNAME)\$(RTOS)\$(SWCONFIG)\DEBUG 2> nul:
	-@md $(SABINE_ROOT)\$(*B)\$(CONFIG)\$(APPNAME)\$(RTOS)\$(SWCONFIG)\RELEASE 2> nul:
!if "$(TYPE)" == "GET"
	-@cd $(SABINE_ROOT)\$(*B)
	-get $(GET_OPTIONS) -v$(VERSION) $(PVCS_ROOT)\$(*B)\$(*B).mav
!endif
!else # TYPE==COPY
	-@md $(SABINE_ROOT)\$(*B) > nul: 2>&1
	-@cd $(SABINE_ROOT)\$(*B)
	-$(AT)xcopy /K /R /Y $(SOURCE)\$(*B)\$(*B).MAK  > nul: 2>&1
!endif
	-$(AT)$(MAKE) /NOLOGO /f$(SABINE_ROOT)\make\topmake.mak ALL_DRIVERS=$(*B) DRIVER=$(*B) driver 


############################################################
# Translate the CONFIG file(s) into the appropriate header #
# files.                                                   #
############################################################

do_cfgtoh: do_cfgtoh_dirs \
	$(SABINE_ROOT)\include\$(CONFIG)\$(SWCONFIG)\boxcfg.h \
	$(SABINE_ROOT)\include\$(CONFIG)\$(SWCONFIG)\boxcfg.vxa \
	$(SABINE_ROOT)\include\$(CONFIG)\$(SWCONFIG)\boxcfg.a \
	$(SABINE_ROOT)\include\hwopts.h \
	$(SABINE_ROOT)\include\hwopts.vxa \
	$(SABINE_ROOT)\include\hwopts.a \
	$(SABINE_ROOT)\include\swopts.h \
	$(SABINE_ROOT)\include\swopts.vxa \
	$(SABINE_ROOT)\include\swopts.a \
	$(SABINE_ROOT)\include\hwdefault.h \
	$(SABINE_ROOT)\include\hwdefault.vxa \
	$(SABINE_ROOT)\include\hwdefault.a \
	$(SABINE_ROOT)\include\swdefault.h \
	$(SABINE_ROOT)\include\swdefault.vxa \
	$(SABINE_ROOT)\include\swdefault.a \
	$(SABINE_ROOT)\include\hwconfig.html \
	$(SABINE_ROOT)\include\swconfig.html \
!if "$(SWCONFIG)" != "CNXT"
	$(SABINE_ROOT)\include\$(CONFIG)\$(SWCONFIG)\swboxcfg.h \
	$(SABINE_ROOT)\include\$(CONFIG)\$(SWCONFIG)\swboxcfg.vxa \
	$(SABINE_ROOT)\include\$(CONFIG)\$(SWCONFIG)\swboxcfg.a \
!endif


do_cfgtoh_dirs:
!if exist ($(SABINE_ROOT)\include\$(CONFIG)\$(SWCONFIG)) == 0
	-@if not exist $(SABINE_ROOT)\include\$(CONFIG)\nul mkdir $(SABINE_ROOT)\include\$(CONFIG)
	-@if not exist $(SABINE_ROOT)\include\$(CONFIG)\$(SWCONFIG)\nul mkdir $(SABINE_ROOT)\include\$(CONFIG)\$(SWCONFIG)
!endif


##############################################################################
# Rules to build C and asm option and default header files from HWCONFIG.CFG #
##############################################################################
$(SABINE_ROOT)\include\hwopts.h: $(CONFIG_ROOT)\hwconfig.cfg
	@$(TOOLEXE_PATH)\cfgtoh -q -y -s -i$** -c$@ -1h -pHWOPTS 
	
$(SABINE_ROOT)\include\hwdefault.h: $(CONFIG_ROOT)\hwconfig.cfg
	@$(TOOLEXE_PATH)\cfgtoh -q -y -d -w -x -i$** -1h -c$@ -pHWDEFAULT 

$(SABINE_ROOT)\include\hwopts.a: $(CONFIG_ROOT)\hwconfig.cfg
	@$(TOOLEXE_PATH)\cfgtoh -q -y -s -i$** -a$@ -2a -pHWOPTS 
	
$(SABINE_ROOT)\include\hwdefault.a: $(CONFIG_ROOT)\hwconfig.cfg
	@$(TOOLEXE_PATH)\cfgtoh -q -y -d -w -x -i$** -a$@ -2a -pHWDEFAULT 

$(SABINE_ROOT)\include\hwopts.vxa: $(CONFIG_ROOT)\hwconfig.cfg
	@$(TOOLEXE_PATH)\cfgtoh -q -y -s -i$** -v$@ -3vxa -pHWOPTS 
	
$(SABINE_ROOT)\include\hwdefault.vxa: $(CONFIG_ROOT)\hwconfig.cfg
	@$(TOOLEXE_PATH)\cfgtoh -q -y -d -w -x -i$** -v$@ -3vxa -pHWDEFAULT 

$(SABINE_ROOT)\include\hwconfig.html: $(CONFIG_ROOT)\hwconfig.cfg
	@$(TOOLEXE_PATH)\cfgtoh -q -y -w -x -i$** -m$@ -3html

##############################################################################
# Rules to build C and asm option and default header files from SWCONFIG.CFG #
##############################################################################
$(SABINE_ROOT)\include\swopts.h: $(CONFIG_ROOT)\swconfig.cfg
	@$(TOOLEXE_PATH)\cfgtoh -q -y -s -i$** -c$@ -1h -pSWOPTS 
	
$(SABINE_ROOT)\include\swdefault.h: $(CONFIG_ROOT)\swconfig.cfg
	@$(TOOLEXE_PATH)\cfgtoh -q -y -d -w -x -i$** -1h -c$@ -pSWDEFAULT 

$(SABINE_ROOT)\include\swopts.a: $(CONFIG_ROOT)\swconfig.cfg
	@$(TOOLEXE_PATH)\cfgtoh -q -y -s -i$** -a$@ -2a -pSWOPTS 
	
$(SABINE_ROOT)\include\swdefault.a: $(CONFIG_ROOT)\swconfig.cfg
	@$(TOOLEXE_PATH)\cfgtoh -q -y -d -w -x -i$** -a$@ -2a -pSWDEFAULT 

$(SABINE_ROOT)\include\swopts.vxa: $(CONFIG_ROOT)\swconfig.cfg
	@$(TOOLEXE_PATH)\cfgtoh -q -y -s -i$** -v$@ -3vxa -pSWOPTS 
	
$(SABINE_ROOT)\include\swdefault.vxa: $(CONFIG_ROOT)\swconfig.cfg
	@$(TOOLEXE_PATH)\cfgtoh -q -y -d -w -x -i$** -v$@ -3vxa -pSWDEFAULT 

$(SABINE_ROOT)\include\swconfig.html: $(CONFIG_ROOT)\swconfig.cfg
	@$(TOOLEXE_PATH)\cfgtoh -q -y -w -x -i$** -m$@ -3html

#################################################################################
# Rules to build C and asm headers from this build's specific hw and sw config  #
#################################################################################
$(SABINE_ROOT)\include\$(CONFIG)\$(SWCONFIG)\boxcfg.h: $(CONFIG_ROOT)\$(CONFIG).cfg
	@$(TOOLEXE_PATH)\cfgtoh -q -y -i$** -c$@ -1h -n$(SABINE_ROOT)\include\$(CONFIG)\$(SWCONFIG)\chiphdr


$(SABINE_ROOT)\include\$(CONFIG)\$(SWCONFIG)\boxcfg.vxa: $(CONFIG_ROOT)\$(CONFIG).cfg
	@$(TOOLEXE_PATH)\cfgtoh -q -y -i$** -v$@ -3vxa -n$(SABINE_ROOT)\include\$(CONFIG)\$(SWCONFIG)\chiphdr


$(SABINE_ROOT)\include\$(CONFIG)\$(SWCONFIG)\boxcfg.a: $(CONFIG_ROOT)\$(CONFIG).cfg
	@$(TOOLEXE_PATH)\cfgtoh -q -y -i$** -a$@ -2a -n$(SABINE_ROOT)\include\$(CONFIG)\$(SWCONFIG)\chiphdr


$(SABINE_ROOT)\include\$(CONFIG)\$(SWCONFIG)\swboxcfg.h: $(CONFIG_ROOT)\$(SWCONFIG).cfg
	@$(TOOLEXE_PATH)\cfgtoh -q -y -s -i$** -c$@ -1h


$(SABINE_ROOT)\include\$(CONFIG)\$(SWCONFIG)\swboxcfg.vxa: $(CONFIG_ROOT)\$(SWCONFIG).cfg
	@$(TOOLEXE_PATH)\cfgtoh -q -y -s -i$** -v$@ -3vxa


$(SABINE_ROOT)\include\$(CONFIG)\$(SWCONFIG)\swboxcfg.a: $(CONFIG_ROOT)\$(SWCONFIG).cfg
	@$(TOOLEXE_PATH)\cfgtoh -q -y -s -i$** -a$@ -2a


#########################################################
# Set environment variables to allow recursively called #
# copies of nmake to see extra compiler/assembler flags #
#########################################################
set:
	$(AT)REM #######################################
	$(AT)REM # Setting extra environment variables #
	$(AT)REM #######################################
	$(AT)set APPNAME=$(APPNAME)
	$(AT)set APPDIR=$(APPDIR)
	$(AT)set AUDIO_FLAGS=$(AUDIO_FLAGS)
	$(AT)set ARM_VERSION=$(ARM_VERSION)
	$(AT)set AT=$(AT)
	$(AT)set BSP=$(BSP)
	$(AT)set BOARD_FLAGS=$(BOARD_FLAGS)
	$(AT)set BUFFER_FLAGS=$(BUFFER_FLAGS)
	$(AT)set CPU=$(CPU)
	$(AT)set COMBINED_ERROR_LOG=$(COMBINED_ERROR_LOG)
	$(AT)set CONFIG=$(CONFIG)
	$(AT)set CUSTOMER=$(CUSTOMER)
	$(AT)set DEBUG=$(DEBUG)
	$(AT)set DELCFG=$(DELCFG)
	$(AT)set DEVICE_FLAGS=$(DEVICE_FLAGS)
	$(AT)set DLOAD=$(DLOAD)
	$(AT)set DRV_INCL=$(DRV_INCL)
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
	$(AT)set INCLUDE=$(SABINE_ROOT)\include;$(EXTRA_INCLUDE_DIRS: =;);$(SABINE_ROOT)\include\$(CONFIG)\$(SWCONFIG)
	$(AT)set LOG=$(LOG)
	$(AT)set MAKE_VERSION=$(MAKE_VERSION)
	$(AT)set MODE=$(MODE)
	$(AT)set OPENTV_FLAGS=$(OPENTV_FLAGS)
	$(AT)set PACKING=$(PACKING)
	$(AT)set REVISION_FLAGS=$(REVISION_FLAGS)
	$(AT)set ROM_FLAGS=$(ROM_FLAGS)
	$(AT)set RTOS=$(RTOS)
	$(AT)set SOURCE=$(SOURCE)
	$(AT)set SWCONFIG=$(SWCONFIG)
	$(AT)set TYPE=$(TYPE)
	$(AT)set TRACE_OUTPUT=$(TRACE_OUTPUT)
	$(AT)set UART_FLAGS=$(UART_FLAGS)
	$(AT)set VERBOSE=$(VERBOSE)
	$(AT)set VERSION=$(VERSION)
	$(AT)set VIDEO_FLAGS=$(VIDEO_FLAGS)


# Force EXTRA_INCLUDE_DIRS target to be made. If these directories already
# exist, these commands won't be executed unless this dependency is listed.
force_extra_include_dirs:

lib_dir:
	-@md $(BLD_LIB_DIR) 2> nul:
	-@md $(OBJONLY_OUTPUT_DIR) 2> nul:

always:

######################################
# Build a version number header file #
######################################
$(OUTPUT_DIR)\bldver.h: always
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
$(OUTPUT_DIR)\drv_incl.h: $(APPDIR)\Makefile 
	-@md $(OUTPUT_DIR) > nul: 2>nul:
	-@echo  /* This header is automatically generated!  Do not edit! */ > $@
	-@"$(TOOLEXE_PATH)\c2obj" -d "#define DRIVER_INCL_" $(OPERATING_SYS_DRIVERS) >> $@
	-@"$(TOOLEXE_PATH)\c2obj" -d "#define DRIVER_INCL_" $(ALL_DRIVERS) >> $@



drv_log:
	-@cd . > $(SABINE_ROOT)\$(DRIVER)\$(INTER_OUTPUT_DIR)\driver.log

create_log: 
	-@md $(OUTPUT_DIR) > nul: 2>nul:
	-@cd . > $(COMBINED_ERROR_LOG)


drv_build: buildlib 

buildlib: drv_log $(SABINE_ROOT)\MAKE\RULES.MAK $(SABINE_ROOT)\MAKE\DEPEND.MAK
	@echo Visiting directory     $(DRIVER)
	@echo Visiting directory     $(DRIVER) >> $(COMBINED_ERROR_LOG)
	@cd $(SABINE_ROOT)\$(DRIVER)\$(INTER_OUTPUT_DIR)
	-$(AT)$(MAKE) /NOLOGO /f$(SABINE_ROOT)\MAKE\DEPEND.MAK DRIVER=$(DRIVER) all 2>driver.log
	-@type driver.log
	-@type driver.log >> $(COMBINED_ERROR_LOG)
#   Old Environments are set assuming the build directory will be SAB_ROOT\DRIVER. (ie. include paths are 	
#   relative to that directory).  In the new build system, we don't want relative include paths.  
	-$(AT)$(MAKE) /NOLOGO /f$(SABINE_ROOT)\MAKE\RULES.MAK DRIVER=$(DRIVER) all 2>driver.log 
	-@type driver.log
	-@type driver.log >> $(COMBINED_ERROR_LOG)
	-@del driver.log > nul: 2>&1
	@cd $(MAKEDIR)

include_copy:  
	@echo Making INCLUDE for     $(DRIVER)
	@echo. >> $(COMBINED_ERROR_LOG)
	@echo Making INCLUDE for     $(DRIVER) >> $(COMBINED_ERROR_LOG)
	$(AT)$(MAKE) /NOLOGO /f$(SABINE_ROOT)\MAKE\INCLUDE.MAK DRIVER=$(DRIVER) copy_headers 

include_clean:
	@echo Cleaning PUBLIC_HEADERS for      $(DRIVER)
	$(AT)$(MAKE) /NOLOGO /f$(SABINE_ROOT)\MAKE\INCLUDE.MAK DRIVER=$(DRIVER) clean_headers 

include_rlsprep:
	@echo Preparing for release:  PUBLIC_HEADERS for     $(DRIVER)
	$(AT)$(MAKE) /NOLOGO /f$(SABINE_ROOT)\MAKE\INCLUDE.MAK DRIVER=$(DRIVER) release_headers 

include_find:
	$(AT)$(MAKE) /NOLOGO /f$(SABINE_ROOT)\MAKE\INCLUDE.MAK DRIVER=$(DRIVER) find_headers 



!include $(SABINE_ROOT)\make\clean.mak
!include $(SABINE_ROOT)\make\label.mak
!include $(SABINE_ROOT)\make\get.mak
!include $(SABINE_ROOT)\make\copy.mak
!include $(SABINE_ROOT)\make\link.mak

!if "$(TYPE)" == "LOCAL"

all:  do_cfgtoh set $(OUTPUT_DIR)\drv_incl.h lib_dir get_make $(OUTPUT_DIR)\bldver.h create_log $(OPERATING_SYS_DRIVERS) $(ALL_DRIVERS) $(EXTRA_DRIVERS) app_link $(EXTRA_APPS) $(EXTENSION)
	-@echo.
	-@echo *** Finished building ***
	-@echo.

driver: drv_build
!endif

!if "$(TYPE)" == "GET"

all: set $(EXTRA_INCLUDE_DIRS) tools_dir lib_dir get_make include_dir os_include_dir create_log $(OPERATING_SYS_DRIVERS) $(ALL_DRIVERS) $(EXTRA_DRIVERS) $(EXTRA_APPS)
	-@echo.
	-@echo *** Finished building ***
	-@echo.

driver: drv_get
!endif

!if "$(TYPE)" == "LABEL"
all: set create_log $(EXTRA_INCLUDE_DIRS) $(OPERATING_SYS_DRIVERS) $(ALL_DRIVERS) $(EXTRA_DRIVERS) $(EXTRA_APPS) make_label include_label os_include_label config_label tools_label
	-@echo.
	-@echo *** Finished building ***
	-@echo.

driver: drv_label
!endif

!if "$(TYPE)" == "CLEAN"
all: set do_cfgtoh_clean $(OPERATING_SYS_DRIVERS) $(ALL_DRIVERS) $(EXTRA_DRIVERS) $(EXTRA_APPS)
	-@echo.
	-@echo *** Finished cleaning ***
	-@echo.

driver: drv_clean
!endif

!if "$(TYPE)" == "INCLUDE"
all: set $(OUTPUT_DIR)\drv_incl.h create_log $(OPERATING_SYS_DRIVERS) $(ALL_DRIVERS) $(EXTRA_DRIVERS) $(EXTRA_APPS)
	@echo Converting ARM format ASM headers to GNU format ASM headers
   -@cd $(SABINE_ROOT)\include
   -@del /f *.vxa > nul: 2>&1
   -@dir *.a /b > $(OUTPUT_DIR)\asm_headers
   -$(AT)$(TOOLEXE_PATH)\AsmArm2Gnu $(OUTPUT_DIR)\asm_headers
   -@cd $(MAKEDIR)

	-@echo.
	-@echo *** Finished "making" includes ***
	-@echo.

driver: include_copy
!endif

!if "$(TYPE)" == "CLEAN_PH"
all: set $(OPERATING_SYS_DRIVERS) $(ALL_DRIVERS) $(EXTRA_DRIVERS) $(EXTRA_APPS)
	-@cd $(SABINE_ROOT)\include
	-@del /f *.vxa > nul: 2>&1

	-@echo.
	-@echo *** Finished cleaning PUBLIC_HEADERS ***
	-@echo.

driver: include_clean
!endif

!if "$(TYPE)" == "RLSPREP"
all: set $(OPERATING_SYS_DRIVERS) $(ALL_DRIVERS) $(EXTRA_DRIVERS) $(EXTRA_APPS)
	-@echo.
	-@echo *** Finished preparing for release ***
	-@echo.

driver: include_rlsprep
!endif

!if "$(TYPE)" == "FIND_PH"
all: set $(OPERATING_SYS_DRIVERS) $(ALL_DRIVERS) $(EXTRA_DRIVERS) $(EXTRA_APPS)

driver: include_find
!endif

!if "$(TYPE)" == "COPY"

!if "$(SOURCE)"==""
!message ******    You must specify a SOURCE when doing a TYPE=COPY.
!message ******    ex.     nmake TYPE=COPY SOURCE=W:\sw-dev\curtree
!error No SOURCE specified
!endif

all: set create_log $(EXTRA_INCLUDE_DIRS) copy_tools_dir copy_include_dir copy_os_include_dir $(OPERATING_SYS_DRIVERS) $(ALL_DRIVERS) $(EXTRA_DRIVERS) $(EXTRA_APPS)
	-@echo.
	-@echo *** Finished copying ***
	-@echo.
	

driver: drv_copy 
!endif

help:
	@echo ==============================================================================
	@echo  The following environment variables need to be set in order for the build
	@echo  to work properly:
	@echo ······ARM_TOOLKIT - Supported options are: SDT, ADS, WRGCC, (GNUGCC)
	@echo ······ARM_VERSION - correlates to ARM_TOOLKIT.
	@echo ····················Options are 251, 12, 296, (321) (respectively.
	@echo ······SABINE_ROOT - points to the root of your source tree
	@echo ······CONFIG_ROOT - Should be %SABINE_ROOT%\CONFIGS
	@echo ·········SDT_PATH - If using ARM SDT tools, should point to location
	@echo ····················of SDT toolkit
	@echo ·········ADS_PATH - If using ARM ADS tools, should point to location
	@echo ····················of ADS toolkit
	@echo ··········WR_PATH - If using Windriver GCC compiler, should point to 
	@echo ····················location of Tornado
	@echo ·········GNU_PATH - If using GNU GCC compiler, should point to 
	@echo ····················location of compiler root
	@echo ···········CONFIG - may be specified on command line - name of CONFIG
	@echo ····················to build for
	@echo ·········SWCONFIG - may be specified on command line - name of SWCONFIG 
	@echo ····················to build
	@echo ···INTERNAL_BUILD - sets build rules for "private" source
	@echo ··LM_LICENSE_FILE - if using ADS with floating licenses, should point to an 
	@echo ····················LM_LICENSE server
	@echo ···········SOURCE - used when performing a TYPE=COPY, defines the source
	@echo ····················directory of the copy.              
	@echo.
	@echo.
	@echo ===============================================================================
	@echo.
	@echo.
	@echo The following command line options will make the build operate differently:
	@echo.
	@echo ···TYPE = [ GET, INCLUDE, CLEAN, LABEL, CLEAN_PH, RLSPREP, FIND_PH, COPY ]
	@echo ···WHERE:
	@echo ············GET - Gets source files out of the backing tree
	@echo ········INCLUDE - (internal) Copies plublic_headers from driver directory
	@echo ··················to \include directory
	@echo ··········CLEAN - Cleans all output generated during a build
	@echo ·······CLEAN_PH - (internal) Removes public headers from \include
	@echo ········RLSPREP - (internal) Removes public headers from driver directories
	@echo ········FIND_PH - (internal) shows original location of public_headers
	@echo ···········COPY - (internal) copies source from SOURCE to current directory
	@echo.
	@echo ··DEBUG = [YES, NO] - controls debug level of code
	@echo.
	@echo ··VERBOSE = [ YES, NO, VERY ] - Controls verbosity level of build output
	@echo.


