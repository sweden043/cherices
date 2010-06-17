###############################################################################
#                                                                             #
# CNXT MPEG build support file: depend.mak                                    #
#                                                                             #
# Author: M. Bintz 7/24/03                                                    #
#                                                                             #
# Compiler rules for all toolchains and file types.                           #
#                                                                             #
###############################################################################
.SUFFIXES: .d32 .d16 .c

!include $(SABINE_ROOT)\make\flags.mak

!include $(SABINE_ROOT)\$(DRIVER)\$(DRIVER).MAK

# This file is concatenated onto the $(DRIVER).MAK file
# and a make is invoked with this "new" file to generate a 
# dependency file.

!if "$(VERBOSE)"=="YES"
AT =  
!else
AT = @
!endif

MKDEP = $(SABINE_ROOT)\toolexe\MAKEDEPEND.EXE

MKDEP_FLAGS = \
 -I$(INC_FIX:;= -I) \
 -I$(SABINE_ROOT)\$(DRIVER) \
 $(CDEFS) \
 -Q -B 

# topmake added this to include so this should be included by $(INCLUDE)
# -I$(SABINE_ROOT)\include\$(CONFIG)\$(SWCONFIG) \

!if "$(VERBOSE)" == "YES"
MKDEP_FLAGS = $(MKDEP_FLAGS) -v
!endif

MKDEP_FLAGS = $(MKDEP_FLAGS: -UDEBUG=)
MKDEP_FLAGS = $(MKDEP_FLAGS: -UTRACE_OUTPUT=)

#######################################
# What are our output directories ? #
#######################################
!include $(SABINE_ROOT)\make\builddirs.mak




!if "$(MODE)" == "THUMB"
!if "$(THUMB_C_FILES)" != ""
THUMB_C_FILES = $(THUMB_C_FILES) $(GENERAL_C_FILES)
!else
THUMB_C_FILES = $(GENERAL_C_FILES)
!endif
!else
!if "$(ARM_C_FILES)" != ""
ARM_C_FILES = $(ARM_C_FILES) $(GENERAL_C_FILES)
!else
ARM_C_FILES = $(GENERAL_C_FILES)
!endif
!endif

!if 1

{$(SABINE_ROOT)\$(DRIVER)}.c.d16:
	-@echo -- Generating THUMB dependency for $(<F)
	-@echo -- Generating THUMB dependency for $(<F) >> $(COMBINED_ERROR_LOG)
# It is important that there is no space in the echo command or it'll break NMAKE.  See CR 8152
	-$(AT)echo.>  $(SABINE_ROOT)\$(DRIVER)\$(INTER_OUTPUT_DIR)\$@
	-$(AT)$(MKDEP) $(MKDEP_FLAGS) $(EXTRA_MAKEDEPEND_FLAGS) -C \
	-f$(SABINE_ROOT)\$(DRIVER)\$(INTER_OUTPUT_DIR)\$@ $(<) >> $(COMBINED_ERROR_LOG) 2>>&1


{$(SABINE_ROOT)\$(DRIVER)}.c.d32:
	-@echo -- Generating ARM dependency for $(<F)
	-@echo -- Generating ARM dependency for $(<F) >> $(COMBINED_ERROR_LOG)
# It is important that there is no space in the echo command or it'll break NMAKE.  See CR 8152
	-$(AT)echo.>  $(SABINE_ROOT)\$(DRIVER)\$(INTER_OUTPUT_DIR)\$@
	-$(AT)$(MKDEP) $(MKDEP_FLAGS) $(EXTRA_MAKEDEPEND_FLAGS) -C \
	-f$(SABINE_ROOT)\$(DRIVER)\$(INTER_OUTPUT_DIR)\$@ $(<) >> $(COMBINED_ERROR_LOG) 2>>&1


$(SABINE_ROOT)\$(DRIVER)\$(INTER_OUTPUT_DIR)\module_$(DRIVER).d16: $(THUMB_C_FILES:.C=.D16) 
!if "$(THUMB_C_FILES)"!=""
    $(AT)copy $(**: =+) $@ > nul:
!else
# It is important that there is no space in the echo command or it'll break NMAKE.  See CR 8152
    $(AT)echo.>  $@
!endif

$(SABINE_ROOT)\$(DRIVER)\$(INTER_OUTPUT_DIR)\module_$(DRIVER).d32: $(ARM_C_FILES:.C=.D32)
!if "$(ARM_C_FILES)"!=""
    $(AT)copy $(**: =+) $@ > nul:
!else
# It is important that there is no space in the echo command or it'll break NMAKE.  See CR 8152
    $(AT)echo.>  $@
!endif


!else
{$(SABINE_ROOT)\$(DRIVER)}.c.d16::
	-@echo -- Generating THUMB dependency for $(<F)
	-@echo -- Generating THUMB dependency for $(<F) >> $(COMBINED_ERROR_LOG)
# It is important that there is no space in the echo command or it'll break NMAKE.  See CR 8152
	-$(AT)echo.>  $(SABINE_ROOT)\$(DRIVER)\$(INTER_OUTPUT_DIR)\$@
	-$(AT)$(MKDEP) $(MKDEP_FLAGS) $(EXTRA_MAKEDEPEND_FLAGS) -C \
	-f$(SABINE_ROOT)\$(DRIVER)\$(INTER_OUTPUT_DIR)\$@ $(<) >> $(COMBINED_ERROR_LOG) 2>>&1


{$(SABINE_ROOT)\$(DRIVER)}.c.d32::
	-@echo -- Generating ARM dependency for $(<F)
	-@echo -- Generating ARM dependency for $(<F) >> $(COMBINED_ERROR_LOG)
# It is important that there is no space in the echo command or it'll break NMAKE.  See CR 8152
	-$(AT)echo.>  $(SABINE_ROOT)\$(DRIVER)\$(INTER_OUTPUT_DIR)\$@
	-$(AT)$(MKDEP) $(MKDEP_FLAGS) $(EXTRA_MAKEDEPEND_FLAGS) -C \
	-f$(SABINE_ROOT)\$(DRIVER)\$(INTER_OUTPUT_DIR)\$@ $(<) >> $(COMBINED_ERROR_LOG) 2>>&1


$(SABINE_ROOT)\$(DRIVER)\$(INTER_OUTPUT_DIR)\module_$(DRIVER).d16: $(THUMB_C_FILES:.C=.D16) 
!if "$(THUMB_C_FILES)"!=""
    $(AT)copy $(**: =+) $@ > nul:
!else
# It is important that there is no space in the echo command or it'll break NMAKE.  See CR 8152
    $(AT)echo.>  $@
!endif

$(SABINE_ROOT)\$(DRIVER)\$(INTER_OUTPUT_DIR)\module_$(DRIVER).d32: $(ARM_C_FILES:.C=.D32)
!if "$(ARM_C_FILES)"!=""
    $(AT)copy $(**: =+) $@ > nul:
!else
# It is important that there is no space in the echo command or it'll break NMAKE.  See CR 8152
    $(AT)echo.>  $@
!endif


!endif


all :  $(SABINE_ROOT)\$(DRIVER)\$(INTER_OUTPUT_DIR)\module_$(DRIVER).d16 \
       $(SABINE_ROOT)\$(DRIVER)\$(INTER_OUTPUT_DIR)\module_$(DRIVER).d32  




#****************************************************************************
#* $Log: 
#*  6    mpeg      1.5         5/4/04 2:16:24 PM      Miles Bintz     CR(s) 
#*        9082 9083 : fixed makedepend warning about trace output
#*  5    mpeg      1.4         12/18/03 4:31:39 PM    Miles Bintz     CR(s) 
#*        8152 : changed way echo works to get around an nmake bug
#*  4    mpeg      1.3         12/11/03 1:44:06 PM    Miles Bintz     CR(s) 
#*        8133 8134 : removed touch command from makefiles
#*  3    mpeg      1.2         9/26/03 12:07:58 PM    Miles Bintz     SCR(s) 
#*        7551 :
#*        fixed makedepend warnings AGAIN
#*        
#*  2    mpeg      1.1         9/18/03 3:24:40 PM     Miles Bintz     SCR(s) 
#*        7484 :
#*        removed the -UDEBUG flag before running makedepend
#*        
#*  1    mpeg      1.0         9/9/03 4:40:38 PM      Miles Bintz     
#* $
#  
#     Rev 1.2   26 Sep 2003 11:07:58   bintzmf
#  SCR(s) 7551 :
#  fixed makedepend warnings AGAIN
#  
#     Rev 1.1   18 Sep 2003 14:24:40   bintzmf
#  SCR(s) 7484 :
#  removed the -UDEBUG flag before running makedepend
#  
#     Rev 1.0   09 Sep 2003 15:40:38   bintzmf
#  SCR(s) 7291 :
#  reworked build system
#  
#     Rev 1.1   28 Aug 2003 15:25:32   bintzmf
#  
#     Rev 1.0   14 Aug 2003 13:41:46   bintzmf
#  new build system
#*  
#*  
#****************************************************************************


