###############################################################################
#                                                                             #
# CNXT MPEG build support file: submake.mak                                   #
#                                                                             #
# Author: B. Bradford 2003/09/19                                              #
#                                                                             #
# Compiler rules for all toolchains and file types.                           #
#                                                                             #
###############################################################################
# $Header: submake.mak, 5, 4/28/04 7:00:34 AM, Ian Mitchell$
##############################################################################
# TOPMAKE supports a target, EXTRA_DRIVERS, which does not follow the "normal"
# build rules for TOPMAKE.  Instead of recursively calling nmake with 
# $(DRIVER).MAK, it simply calls nmake with "makefile" as the target.
#
# Note:  All of the build environment variables that are setup in the "set:"
# target in TOPMAKE are available to the EXTRA_DRIVERS.
#
# These EXTRA_DRIVERS are typically "hierarchical" in their file structure,
# so there are a number of recursive calls to nmake within the individual
# driver directories.
#
# This file has been created to support the EXTRA_DRIVERS.  There were at
# least 3 copies of "genmake.mak" (the previous version of this file) in the
# separate EXTRA_DRIVERS directories (e.g. P93MODEM2, FLIPPER, CNXT_FTA_APP).
# This file is an attempt to merge these mostly identical files into a common
# file that can be more easily supported and updated.
##############################################################################
#
# The following notes are from a copy of one of the "genmake.mak" files
#
##############################################################################
#
# This makefile is included by the makefile in each subdirectory.
# The individual makefiles are customized only by defining the following:
#
#   SRC_FILES - a space separated list of all of the source files
#               that need to be built, includes the extension (.c, etc)
#   OTHER_FILES - any other files needed for checking out, like headers
#                 The makefiles do not need to be included in this list
#                 because they are already get'd by the master makefile.
#
# The following targets are provided:
#   all - the compile
#   get - gets the files from version control
#   label - add version labels to files
#   clean - cleans up all the build products
#
# The following defines are needed by this makefile, and should be passed
# in on the command line.
#   THUMB = YES|NO
#   MAKEDEP - extra flags needed by makedepend (-DP93, etc)
#   DEPEND = YES|NO, whether makedepend should run
#   OUTLIB - the path and library name where the objects are collected
#   EXTRA_INCLUDE - extra include paths (common to all files in lib)
#   EXTRA_INCLUDE_SUBS - extra include paths (specific to subdirs)
#
# This section sets up defines needed by the compile rules makefile.
# The appropriate rules makefile is included depending on the compiler
# mode and OS target.
#
##############################################################################

#################################################
# Ensure that "all" is the default target built #
#################################################
target: all

##############################################################################
# The following macros are used by the compiler/assembler RULE files
EXTRA_ARM_C_FLAGS=$(EXTRA_C_FLAGS)
EXTRA_THUMB_C_FLAGS=$(EXTRA_C_FLAGS)

!if "$(DEBUG)" == "NO"
DEBUGDIR        = RELEASE
!else
DEBUGDIR        = DEBUG
!endif


##############################################################################
# we need the same object directory as the rules file

OBJDIR=$(SABINE_ROOT)\$(DRIVER)\$(CONFIG)\$(APPNAME)\$(RTOS)\$(SWCONFIG)\$(DEBUGDIR)

!if "$(COMBINED_ERROR_LOG)" == ""
COMBINED_ERROR_LOG=$(OBJDIR)\build.log
!endif


##############################################################################
# extra includes passed from higher level makefiles
INCLUDE=$(EXTRA_INCLUDE);$(SABINE_ROOT)\INCLUDE
!if "$(ARM_TOOLKIT)" == "WRGCC"
INCLUDE=$(INCLUDE);$(WIND_BASE)\TARGET\H
INCLUDE=$(INCLUDE);$(WIND_BASE)\host\x86-win32\lib\gcc-lib\arm-wrs-vxworks\2.9-010413\include
INCLUDE=$(INCLUDE);$(WIND_BASE)\TARGET\H\ARCH\arm
!else
INCLUDE=$(INCLUDE);$(ARMINC);$(RTOS_INCL)
!endif


##############################################################################
# Defaults
!ifndef CUSTOMER
CUSTOMER=CNXT
!endif

!ifndef VERSION
VERSION=USE
!endif

!ifndef TOOLEXE_PATH
TOOLEXE_PATH = $(SABINE_ROOT)\TOOLEXE
!endif

##############################################################################
# define ALL files for rules that don't care about source or other (e.g. get)
!ifdef ALL_SUBDIR_FILES
!undef ALL_SUBDIR_FILES
!endif

ALL_SUBDIR_FILES = $(SRC_FILES) $(OTHER_FILES)

##############################################################################
# controls display of makefile commands, YES is useful for debugging makefile

!if "$(VERBOSE)" == "YES"
# If verbose is YES then we will echo the commands we will run
AT = 
!else
#else we will keep the commands "quiet"
AT = @
!endif


##############################################################################
# Note:  Because we are not fully integrated into the "new" TOPMAKE build
# process, we still need to include the "older" versions of the compiler
# rule files

!if "$(ARM_TOOLKIT)" == "WRGCC"
!if "$(FORCE_ARM_MODE)" == "YES"
!include $(SABINE_ROOT)\make\c32l_vx.mak
!else
!if "$(THUMB)" == "YES"
!include $(SABINE_ROOT)\make\c16l_vx.mak
!else
!include $(SABINE_ROOT)\make\c32l_vx.mak
!endif
!endif
!else
!if "$(FORCE_ARM_MODE)" == "YES"
!include $(SABINE_ROOT)\make\c32lrule.mak
!else
!if "$(THUMB)" == "YES"
!include $(SABINE_ROOT)\make\c16lrule.mak
!else
!include $(SABINE_ROOT)\make\c32lrule.mak
!endif
!endif
!endif


##############################################################################
# The following causes makedepend to run and create a file called
# depend.mki which contains the dependency lists for all of the
# source files.  Depend.mki is then included at the end of this file.

MKDEP = $(SABINE_ROOT)\toolexe\MAKEDEPEND.EXE

MKDEP_FLAGS = \
 -I$(INC_FIX:;= -I) \
 -I$(SABINE_ROOT)\$(DRIVER) \
 -Q -B 

# topmake added this to include so this should be included by $(INCLUDE)
# -I$(SABINE_ROOT)\include\$(CONFIG)\$(SWCONFIG) \

!if "$(VERBOSE)" == "YES"
MKDEP_FLAGS = $(MKDEP_FLAGS) -v
!endif

!if "$(DEPEND)" == "YES"
!if [@echo -- Generating dependency for $(DRIVER)]
!endif
!if [@echo. >depend.mki]
!endif
!if [@$(MKDEP) $(MKDEP_FLAGS) $(MAKEDEP) -C $(SRC_FILES)]
!endif
!else   # DEPEND=NO
!if [@echo. >depend.mki]
!endif
!endif


##############################################################################
# The following creates a list of object files as OBJ_FILES=...
# in a file called objfiles.mki.  The object filenames have the
# full path appended so that it works properly with the dependency rules.
# The objfiles.mki is then included below so that OBJ_FILES becomes part
# of this makefile.

!if [@echo OBJ_FILES= \>objfiles.mki]
!endif
!if [@$(TOOLEXE_PATH)\c2obj -d $(OBJDIR)\ $(SRC_FILES:.C=.OBJ\)  >>objfiles.mki]
!endif
!if [@echo. >>objfiles.mki]
!endif

!include objfiles.mki


##############################################################################
# targets
#


##############################################################################
# all - cause a build

all: $(CONFIG).opt $(OUTLIB)


##############################################################################
# all sub-target (CONFIG.OPT)
#
# following lifted straight from the old topmake.mak to create the .OPT file
# all of these macros are possibly defined in $(CONFIG_ROOT)\$(CONFIG).cfg
# which is included by the rules file, which is included above

$(CONFIG).opt: $(CONFIG_ROOT)\$(CONFIG).cfg
!if "$(REVISION_FLAGS)" != ""
   -@echo $(REVISION_FLAGS) > $(CONFIG).opt
!endif
!if "$(VIDEO_FLAGS)" != ""
   -@echo $(VIDEO_FLAGS) >> $(CONFIG).opt
!endif
!if "$(DEVICE_FLAGS)" != ""
   -@echo $(DEVICE_FLAGS) >> $(CONFIG).opt
!endif
!if "$(AUDIO_FLAGS)" != ""
   -@echo $(AUDIO_FLAGS) >> $(CONFIG).opt
!endif
!if "$(BUFFER_FLAGS)" != ""
   -@echo $(BUFFER_FLAGS) >> $(CONFIG).opt
!endif
!if "$(BOARD_FLAGS)" != ""
   -@echo $(BOARD_FLAGS) >> $(CONFIG).opt
!endif
!if "$(ROM_FLAGS)" != ""
   -@echo $(ROM_FLAGS) >> $(CONFIG).opt
!endif
!if "$(FILE_FLAGS)" != ""
   -@echo $(FILE_FLAGS) >> $(CONFIG).opt
!endif
!if "$(UART_FLAGS)" != ""
   -@echo $(UART_FLAGS) >> $(CONFIG).opt
!endif
!if "$(OPENTV_FLAGS)" != ""
   -@echo $(OPENTV_FLAGS) >> $(CONFIG).opt
!endif


##############################################################################
# all sub-target (OUTLIB)
#
# creates the module library from the object files
# need to add support for vxworks librarian

$(OUTLIB) : $(OBJ_FILES)
    @echo ---- ---- Adding object files to $(DRIVER) library
    $(AT)cd $(OBJDIR)
!if "$(ARM_TOOLKIT)" == "WRGCC"
    $(AT)ararm -r $(OUTLIB) $(SRC_FILES:.C=.OBJ)
!elseif "$(ARM_TOOLKIT)" == "ADS"
    $(AT)armar -r $(OUTLIB) $(SRC_FILES:.C=.OBJ)
!else
    $(AT)armlib -i -n $(OUTLIB) $(SRC_FILES:.C=.OBJ)
!endif
    @cd $(MAKEDIR)


##############################################################################
# this include file contains dependencies for each object file
# this combined with the included rules file above provides enough
# info for nmake to build the object files

!include depend.mki


##############################################################################
# get - checks out the files from version control

get:
	-$(AT)get -V$(VERSION) $(ALL_SUBDIR_FILES)


##############################################################################
# label - add version labels to files in this subdir

label:
	-$(AT)vcs -Y -V$(VERSION) $(ALL_SUBDIR_FILES)
   -$(AT)vcs -Y -V$(VERSION) makefile

##############################################################################
# copy - copies a file from a source tree

copy:
	$(AT)for %i in ($(ALL_SUBDIR_FILES)) do $(AT)xcopy /K /R /Y $(COPY_SOURCE)\%i  > nul: 2>&1


##########################
# end of makefile
#
# $Log: 
#  5    mpeg      1.4         4/28/04 7:00:34 AM     Ian Mitchell    CR(s) 8989
#         8990 : Added RTOS_INCL to the include path.
#        
#  4    mpeg      1.3         12/12/03 3:30:27 PM    Bobby Bradford  CR(s) 8133
#         8134 : 
#        1) Removed use of "TOUCH" command from the makefiles for FLIPPER and 
#        P93MODEM2
#        2) Added definitions to the C Rule files to support recent build 
#        system changes
#        
#  3    mpeg      1.2         10/2/03 6:09:20 PM     Bobby Bradford  SCR(s) 
#        7598 :
#        Clean up the text output for build process of sub-makes 
#        (EXTRA_DRIVERS)
#        
#  2    mpeg      1.1         10/2/03 5:11:28 PM     Bobby Bradford  SCR(s) 
#        7598 :
#        Modifications to support additional TYPE= ... targets
#        
#  1    mpeg      1.0         9/19/03 1:00:24 PM     Bobby Bradford  
# $
#  
#

