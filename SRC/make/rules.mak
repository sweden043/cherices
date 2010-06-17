###############################################################################
#                                                                             #
# CNXT MPEG build support file: rules.mak                                     #
#                                                                             #
# Author: M. Bintz 7/24/03                                                    #
#                                                                             #
# Compiler rules for all toolchains and file types.                           #
#                                                                             #
###############################################################################
.SUFFIXES: .s .gnus .c .gasp .o32 .o16

###########################################################################
#                                                                         # 
# This is the list of dependants.  The lib <- o16/o32 <- .a/.c            #
# These are all based on the [INSTRSET]_[FTYPE]_FILES macros....          #
#                                                                         # 
###########################################################################
# Get our list of files to build and our dependencies
!include $(SABINE_ROOT)\make\product.mak
!include $(SABINE_ROOT)\make\envcheck.mak
!include $(SABINE_ROOT)\$(DRIVER)\$(DRIVER).MAK
!include module_$(DRIVER).D16
!include module_$(DRIVER).D32
!include $(SABINE_ROOT)\make\flags.mak

####### Comprehensive list of C Flags and definitions #############
COPTS  = $(CFLAGS) $(C_DEBUG_FLAGS) $(CDEFS) -I$(OUTPUT_DIR)
COPTS_FIX=FOO;$(COPTS);BAR
COPTS_FIX=$(COPTS_FIX:      = )
COPTS_FIX=$(COPTS_FIX:     = )
COPTS_FIX=$(COPTS_FIX:    = )
COPTS_FIX=$(COPTS_FIX:   = )
COPTS_FIX=$(COPTS_FIX:  = )
COPTS_FIX=$(COPTS_FIX:FOO;=)
COPTS_FIX=$(COPTS_FIX:;BAR=)

####### Comprehensive list of Asm Flags and definitions #############
# ASM_CONFIG_FLAGS is defined in the config files and is an ugly hack to define
# chip name which should already be defined in the [chipname].[ha]
# AFLAGS which is defined in flags[tool].mak now defines CHIP_NAME.
# AOPTS  = $(AFLAGS) $(A_DEBUG_FLAGS) $(ASM_CONFIG_FLAGS) $(ADEFS) -I$(OUTPUT_DIR)
AOPTS  = $(AFLAGS) $(A_DEBUG_FLAGS) $(ADEFS) -I$(OUTPUT_DIR)
AOPTS_FIX=FOO;$(AOPTS);BAR
AOPTS_FIX=$(AOPTS_FIX:      = )
AOPTS_FIX=$(AOPTS_FIX:     = )
AOPTS_FIX=$(AOPTS_FIX:    = )
AOPTS_FIX=$(AOPTS_FIX:   = )
AOPTS_FIX=$(AOPTS_FIX:  = )
AOPTS_FIX=$(AOPTS_FIX:FOO;=)
AOPTS_FIX=$(AOPTS_FIX:;BAR=)


###############   The ALL:  Build target   ########################
all: $(BLD_LIB_DIR)\$(DRIVER).LIB

# Determine how our C files will be built
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



# With the GNU based assemblers, we need an intermediate
# step to turn ARM formatted assembly files into GNU
# (ie use the C preprocessor) formatted assembly files.
# To add insult to injury, we still need to support assembly
# files that are already in GNU format.
DEPENDENCY_LIST_SLACK = \
  $(ARM_C_FILES:.C=.O32) \
  $(THUMB_C_FILES:.C=.O16) \
  $(ARM_ASM_FILES:.S=.O32) \
  $(THUMB_ASM_FILES:.S=.O16) \
  $(ARM_GASP_FILES:.GASP=.O32) \
  $(THUMB_GASP_FILES:.GASP=.O16) \
  $(GNU_ARM_ASM_FILES:.GNUS=.O32) \
  $(GNU_THUMB_ASM_FILES:.GNUS=.O16) 


DEPENDENCY_LIST = FOO;$(DEPENDENCY_LIST_SLACK:      = );BAR
DEPENDENCY_LIST = $(DEPENDENCY_LIST:     = )
DEPENDENCY_LIST = $(DEPENDENCY_LIST:    = )
DEPENDENCY_LIST = $(DEPENDENCY_LIST:   = )
DEPENDENCY_LIST = $(DEPENDENCY_LIST:  = )
DEPENDENCY_LIST = $(DEPENDENCY_LIST:FOO;=)
DEPENDENCY_LIST = $(DEPENDENCY_LIST:;BAR=)

###########################################################################
#                                                                         # 
# These are the actual inference rules.  These tell the                   #
# build how to convert a dependant with a base name of X with extension   #
# Y to a a file of type Z.   X.Y -> rules -> X.Z                          #
#                                                                         # 
###########################################################################

$(BLD_LIB_DIR)\$(DRIVER).LIB: $(DEPENDENCY_LIST)
!if "$(DEPENDENCY_LIST)" != " "
!if "$(OVERRIDE_WARNINGS)" == "YES"
    -@echo NOTE:  Warnings were suppressed for this driver...
    -@echo NOTE:  Warnings were suppressed for this driver... >> $(COMBINED_ERROR_LOG)
!endif
	@echo ---- ---- Creating library $(@F) 
	@echo ---- ---- Creating library $(@F)   >> $(COMBINED_ERROR_LOG)
	-@if exist $(@) del $(@)
	$(AT)$(LIBR) $(LIBRFLAGS) $(@) $** 
!if "$(OBJECT_ONLY)" == "YES"
	-$(AT)copy $(BLD_LIB_DIR)\$(DRIVER).lib $(OBJONLY_OUTPUT_DIR)\$(DRIVER).lib
!endif
	@echo Leaving directory    $(DRIVER)   >> $(COMBINED_ERROR_LOG)
	@echo. >> $(COMBINED_ERROR_LOG)
!else
	@echo ---- ---- Nothing to do for  $(@F) 
	@echo ---- ---- Nothing to do for  $(@F)   >> $(COMBINED_ERROR_LOG)
!endif

{$(SABINE_ROOT)\$(DRIVER)}.c.o32:
	-@echo ---- Compiling ARM mode $(*B).C
	-@echo ---- Compiling ARM mode $(*B).C >> $(COMBINED_ERROR_LOG)
	-$(AT)del $(SABINE_ROOT)\$(DRIVER)\$(INTER_OUTPUT_DIR)\$@ > nul: 2>&1
	-$(AT)$(ACC) $(COPTS_FIX) $(<) -o$(SABINE_ROOT)\$(DRIVER)\$(INTER_OUTPUT_DIR)\$@   > $(*B).LOG  2>&1
	-@type $(*B).LOG
	-@type $(*B).LOG >> $(COMBINED_ERROR_LOG)

{$(SABINE_ROOT)\$(DRIVER)}.c.o16:
	-@echo ---- Compiling THUMB mode $(*B).C
	-@echo ---- Compiling THUMB mode $(*B).C >> $(COMBINED_ERROR_LOG)
	-$(AT)del $(SABINE_ROOT)\$(DRIVER)\$(INTER_OUTPUT_DIR)\$@ > nul: 2>&1
	-$(AT)$(TCC) $(COPTS_FIX) $(<) -o$(SABINE_ROOT)\$(DRIVER)\$(INTER_OUTPUT_DIR)\$@   > $(*B).LOG  2>&1
	-@type $(*B).LOG
	-@type $(*B).LOG >> $(COMBINED_ERROR_LOG)

{$(SABINE_ROOT)\$(DRIVER)}.GNUS.o32: 
	-@echo ---- Assembling ARM mode $(*B)
	-@echo ---- Assembling ARM mode $(*B)  >> $(COMBINED_ERROR_LOG)
	-$(AT)del $(SABINE_ROOT)\$(DRIVER)\$(INTER_OUTPUT_DIR)\$@ > nul: 2>&1
	-$(AT)$(AAS) $(AOPTS_FIX) $(<) -o $(SABINE_ROOT)\$(DRIVER)\$(INTER_OUTPUT_DIR)\$@   > $(*B).LOG  2>&1
	-@type $(*B).LOG
	-@type $(*B).LOG >> $(COMBINED_ERROR_LOG)

{$(SABINE_ROOT)\$(DRIVER)}.GNUS.o16:
	-@echo ---- Assembling THUMB mode $(*B)
	-@echo ---- Assembling THUMB mode $(*B)  >> $(COMBINED_ERROR_LOG)
	-$(AT)del $(SABINE_ROOT)\$(DRIVER)\$(INTER_OUTPUT_DIR)\$@ > nul: 2>&1
	-$(AT)$(TAS) $(AOPTS_FIX) $(<) -o $(SABINE_ROOT)\$(DRIVER)\$(INTER_OUTPUT_DIR)\$@   > $(*B).LOG  2>&1
	-@type $(*B).LOG
	-@type $(*B).LOG >> $(COMBINED_ERROR_LOG)

!if ( "$(ARM_TOOLKIT)" == "WRGCC" || "$(ARM_TOOLKIT)" == "GNUGCC" )
{$(SABINE_ROOT)\$(DRIVER)}.s.o32:
	-@echo ---- Converting and Assembling ARM mode $(*B).S
	-@echo ---- Converting and Assembling ARM mode $(*B).S  >> $(COMBINED_ERROR_LOG)
	-$(AT)$(SABINE_ROOT)\toolexe\asmarm2gnu.exe $(<) $(*B).GNUASM
	-$(AT)del $(SABINE_ROOT)\$(DRIVER)\$(INTER_OUTPUT_DIR)\$@ > nul: 2>&1
	-$(AT)$(AAS) $(AOPTS_FIX) $(SABINE_ROOT)\$(DRIVER)\$(INTER_OUTPUT_DIR)\$(*B).GNUASM -o $(SABINE_ROOT)\$(DRIVER)\$(INTER_OUTPUT_DIR)\$@   > $(*B).LOG  2>&1
	-@type $(*B).LOG
	-@type $(*B).LOG >> $(COMBINED_ERROR_LOG)

{$(SABINE_ROOT)\$(DRIVER)}.s.o16:
	-@echo ---- Converting and Assembling THUMB mode $(*B).S
	-@echo ---- Converting and Assembling THUMB mode $(*B).S  >> $(COMBINED_ERROR_LOG)
	-$(AT)$(SABINE_ROOT)\toolexe\asmarm2gnu.exe $(<) $(*B).GNUASM
	-$(AT)del $(SABINE_ROOT)\$(DRIVER)\$(INTER_OUTPUT_DIR)\$@ > nul: 2>&1
	-$(AT)$(TAS) $(AOPTS_FIX) $(*B).GNUS -o $(SABINE_ROOT)\$(DRIVER)\$(INTER_OUTPUT_DIR)\$@   > $(*B).LOG  2>&1
	-@type $(*B).LOG
	-@type $(*B).LOG >> $(COMBINED_ERROR_LOG)

!else  #else ARM_TOOLKIT
{$(SABINE_ROOT)\$(DRIVER)}.s.o32:
	-@echo ---- Assembling ARM mode $(*B)
	-@echo ---- Assembling ARM mode $(*B)  >> $(COMBINED_ERROR_LOG)
	-$(AT)del $(SABINE_ROOT)\$(DRIVER)\$(INTER_OUTPUT_DIR)\$@ > nul: 2>&1
	-$(AT)$(AAS) $(AOPTS_FIX) $(<) -o $(SABINE_ROOT)\$(DRIVER)\$(INTER_OUTPUT_DIR)\$@   > $(*B).LOG  2>&1
	-@type $(*B).LOG
	-@type $(*B).LOG >> $(COMBINED_ERROR_LOG)

{$(SABINE_ROOT)\$(DRIVER)}.s.o16:
	-@echo ---- Assembling THUMB mode $(*B)
	-@echo ---- Assembling THUMB mode $(*B)  >> $(COMBINED_ERROR_LOG)
	-$(AT)del $(SABINE_ROOT)\$(DRIVER)\$(INTER_OUTPUT_DIR)\$@ > nul: 2>&1
	-$(AT)$(TAS) $(AOPTS_FIX) $(<) -o $(SABINE_ROOT)\$(DRIVER)\$(INTER_OUTPUT_DIR)\$@   > $(*B).LOG  2>&1
	-@type $(*B).LOG
	-@type $(*B).LOG >> $(COMBINED_ERROR_LOG)

!endif  # endif ARM_TOOLKIT

{$(SABINE_ROOT)\$(DRIVER)}.gasp.o32:
 -@echo ---- Assembling (gasp) ARM mode $(*B)
 $(AT)echo  Assembling ARM (gasp) mode $(<) > $(SABINE_ROOT)\$(DRIVER)\$(INTER_OUTPUT_DIR)\$@

{$(SABINE_ROOT)\$(DRIVER)}.gasp.o16:
 -@echo ---- Assembling (gasp) THUMB mode $(*B)
 $(AT)echo  Assembling ARM (gasp) mode $(<) > $(SABINE_ROOT)\$(DRIVER)\$(INTER_OUTPUT_DIR)\$@






















#****************************************************************************
#* $Log: 
#*  4    mpeg      1.3         9/17/03 4:07:38 PM     QA - Roger Taylor SCR(s) 
#*        7484 :
#*        since tools won't clobber a .o if a compile fails, we must first del 
#*        every .o we attempt to build lest we successfully link with an old .o
#*         and go insane from frustration
#*        
#*  3    mpeg      1.2         9/16/03 5:11:38 PM     Miles Bintz     SCR(s) 
#*        7291 :
#*        copy objonly librarues to their appropriate destinations
#*        
#*  2    mpeg      1.1         9/16/03 5:11:18 PM     Miles Bintz     SCR(s) 
#*        7291 :
#*        copy objonly librarues to their appropriate destinations
#*        
#*  1    mpeg      1.0         9/9/03 4:40:46 PM      Miles Bintz     
#* $
#  
#     Rev 1.3   17 Sep 2003 15:07:38   taylorrm
#  SCR(s) 7484 :
#  since tools won't clobber a .o if a compile fails, we must first del every .o we attempt to build lest we successfully link with an old .o and go insane from frustration
#  
#     Rev 1.2   16 Sep 2003 16:11:38   bintzmf
#  SCR(s) 7291 :
#  copy objonly librarues to their appropriate destinations
#  
#     Rev 1.1   16 Sep 2003 16:11:18   bintzmf
#  SCR(s) 7291 :
#  copy objonly librarues to their appropriate destinations
#  
#     Rev 1.0   09 Sep 2003 15:40:46   bintzmf
#  SCR(s) 7291 :
#  reworked build system
#  
#     Rev 1.1   28 Aug 2003 15:26:12   bintzmf
#  
#     Rev 1.0   14 Aug 2003 13:41:48   bintzmf
#  new build system
#*  
#*  
#****************************************************************************


