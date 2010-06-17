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
!include $(SDCAPPS_ROOT)\make\common.mak
!include $(SDCAPPS_ROOT)\make\product.mak
!include $(SDCAPPS_ROOT)\make\builddirs.mak

!include $(SDCAPPS_ROOT)\$(SDCDRIVER)\$(SDCDRIVER).MAK
!include module_$(SDCDRIVER).D16
!include module_$(SDCDRIVER).D32
!include $(SDCAPPS_ROOT)\make\flags.mak

####### Comprehensive list of C Flags and definitions #############
COPTS  = $(CFLAGS) $(C_DEBUG_FLAGS) $(CDEFS) -I$(SDCAPP_OUTPUT_DIR)
COPTS_FIX=FOO;$(COPTS);BAR
COPTS_FIX=$(COPTS_FIX:      = )
COPTS_FIX=$(COPTS_FIX:     = )
COPTS_FIX=$(COPTS_FIX:    = )
COPTS_FIX=$(COPTS_FIX:   = )
COPTS_FIX=$(COPTS_FIX:  = )
COPTS_FIX=$(COPTS_FIX:FOO;=)
COPTS_FIX=$(COPTS_FIX:;BAR=)


AOPTS  = $(AFLAGS) $(A_DEBUG_FLAGS) $(ADEFS) -I$(SDCAPP_OUTPUT_DIR)
AOPTS_FIX=FOO;$(AOPTS);BAR
AOPTS_FIX=$(AOPTS_FIX:      = )
AOPTS_FIX=$(AOPTS_FIX:     = )
AOPTS_FIX=$(AOPTS_FIX:    = )
AOPTS_FIX=$(AOPTS_FIX:   = )
AOPTS_FIX=$(AOPTS_FIX:  = )
AOPTS_FIX=$(AOPTS_FIX:FOO;=)
AOPTS_FIX=$(AOPTS_FIX:;BAR=)


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

###############   The ALL:  Build target   ########################
all: $(DEPENDENCY_LIST) library


###########################################################################
#                                                                         # 
# These are the actual inference rules.  These tell the                   #
# build how to convert a dependant with a base name of X with extension   #
# Y to a a file of type Z.   X.Y -> rules -> X.Z                          #
#                                                                         # 
###########################################################################


library:
	@echo ---- ---- Creating library $(SDCDRIVER).LIB 
	$(AT)$(LIBR) $(LIBRFLAGS) $(LIB_OUTPUT_DIR)\$(SDCDRIVER).LIB $(DEPENDENCY_LIST)

{$(SDCAPPS_ROOT)\$(SDCDRIVER)}.c.o32:
	-@echo ---- Compiling ARM mode $(*B).C
	-$(AT)del $(SDCAPPS_ROOT)\$(SDCDRIVER)\$(INTER_OUTPUT_DIR)\$@ > nul: 2>&1
	-$(AT)$(ACC) $(COPTS_FIX) $(<) -o$(SDCAPPS_ROOT)\$(SDCDRIVER)\$(INTER_OUTPUT_DIR)\$@   > $(*B).LOG  2>&1
	-@type $(*B).LOG
	-@del $(*B).LOG
	
{$(SDCAPPS_ROOT)\$(SDCDRIVER)}.c.o16:
	-@echo ---- Compiling THUMB mode $(*B).C
	-$(AT)del $(SDCAPPS_ROOT)\$(SDCDRIVER)\$(INTER_OUTPUT_DIR)\$@ > nul: 2>&1
	-$(AT)$(TCC) $(COPTS_FIX) $(<) -o$(SDCAPPS_ROOT)\$(SDCDRIVER)\$(INTER_OUTPUT_DIR)\$@   > $(*B).LOG  2>&1
	-@type $(*B).LOG
	-@del $(*B).LOG
	
{$(SDCAPPS_ROOT)\$(SDCDRIVER)}.GNUS.o32: 
	-@echo ---- Assembling ARM mode $(*B)
	-$(AT)del $(SDCAPPS_ROOT)\$(SDCDRIVER)\$(INTER_OUTPUT_DIR)\$@ > nul: 2>&1
	-$(AT)$(AAS) $(AOPTS_FIX) $(<) -o $(SDCAPPS_ROOT)\$(SDCDRIVER)\$(INTER_OUTPUT_DIR)\$@   > $(*B).LOG  2>&1
	-@type $(*B).LOG
	-@del $(*B).LOG
	
{$(SDCAPPS_ROOT)\$(SDCDRIVER)}.GNUS.o16:
	-@echo ---- Assembling THUMB mode $(*B)
	-$(AT)del $(SDCAPPS_ROOT)\$(SDCDRIVER)\$(INTER_OUTPUT_DIR)\$@ > nul: 2>&1
	-$(AT)$(TAS) $(AOPTS_FIX) $(<) -o $(SDCAPPS_ROOT)\$(SDCDRIVER)\$(INTER_OUTPUT_DIR)\$@   > $(*B).LOG  2>&1
	-@type $(*B).LOG
	-@del $(*B).LOG
	
!if ( "$(ARM_TOOLKIT)" == "WRGCC" || "$(ARM_TOOLKIT)" == "GNUGCC" )
{$(SDCAPPS_ROOT)\$(SDCDRIVER)}.s.o32:
	-@echo ---- Converting and Assembling ARM mode $(*B).S
	-$(AT)$(SDCAPPS_ROOT)\toolexe\asmarm2gnu.exe $(<) $(*B).GNUASM
	-$(AT)del $(SDCAPPS_ROOT)\$(SDCDRIVER)\$(INTER_OUTPUT_DIR)\$@ > nul: 2>&1
	-$(AT)$(AAS) $(AOPTS_FIX) $(SDCAPPS_ROOT)\$(SDCDRIVER)\$(INTER_OUTPUT_DIR)\$(*B).GNUASM -o $(SDCAPPS_ROOT)\$(SDCDRIVER)\$(INTER_OUTPUT_DIR)\$@   > $(*B).LOG  2>&1
	-@type $(*B).LOG
	-@del $(*B).LOG
	
{$(SDCAPPS_ROOT)\$(SDCDRIVER)}.s.o16:
	-@echo ---- Converting and Assembling THUMB mode $(*B).S
	-$(AT)$(SDCAPPS_ROOT)\toolexe\asmarm2gnu.exe $(<) $(*B).GNUASM
	-$(AT)del $(SDCAPPS_ROOT)\$(SDCDRIVER)\$(INTER_OUTPUT_DIR)\$@ > nul: 2>&1
	-$(AT)$(TAS) $(AOPTS_FIX) $(*B).GNUS -o $(SDCAPPS_ROOT)\$(SDCDRIVER)\$(INTER_OUTPUT_DIR)\$@   > $(*B).LOG  2>&1
	-@type $(*B).LOG
	-@del $(*B).LOG
	
!else  #else ARM_TOOLKIT
{$(SDCAPPS_ROOT)\$(SDCDRIVER)}.s.o32:
	-@echo ---- Assembling ARM mode $(*B)
	-$(AT)del $(SDCAPPS_ROOT)\$(SDCDRIVER)\$(INTER_OUTPUT_DIR)\$@ > nul: 2>&1
	-$(AT)$(AAS) $(AOPTS_FIX) $(<) -o $(SDCAPPS_ROOT)\$(SDCDRIVER)\$(INTER_OUTPUT_DIR)\$@   > $(*B).LOG  2>&1
	-@type $(*B).LOG
	-@del $(*B).LOG
	
{$(SDCAPPS_ROOT)\$(SDCDRIVER)}.s.o16:
	-@echo ---- Assembling THUMB mode $(*B)
	-$(AT)del $(SDCAPPS_ROOT)\$(SDCDRIVER)\$(INTER_OUTPUT_DIR)\$@ > nul: 2>&1
	-$(AT)$(TAS) $(AOPTS_FIX) $(<) -o $(SDCAPPS_ROOT)\$(SDCDRIVER)\$(INTER_OUTPUT_DIR)\$@   > $(*B).LOG  2>&1
	-@type $(*B).LOG
	-@del $(*B).LOG
	
!endif  # endif ARM_TOOLKIT

{$(SDCAPPS_ROOT)\$(SDCDRIVER)}.gasp.o32:
 -@echo ---- Assembling (gasp) ARM mode $(*B)
 $(AT)echo  Assembling ARM (gasp) mode $(<) > $(SDCAPPS_ROOT)\$(SDCDRIVER)\$(INTER_OUTPUT_DIR)\$@

{$(SDCAPPS_ROOT)\$(SDCDRIVER)}.gasp.o16:
 -@echo ---- Assembling (gasp) THUMB mode $(*B)
 $(AT)echo  Assembling ARM (gasp) mode $(<) > $(SDCAPPS_ROOT)\$(SDCDRIVER)\$(INTER_OUTPUT_DIR)\$@

#****************************************************************************
#* $Log:
#*  2    mpeg      1.1         7/9/2004 4:39:19 PM    Xiao Guang Yan  CR(s)
#*       9699 9700 : Changed OUTPUT_DIR to SDCAPPS_OUTPUT_DIR to avoid
#*       confusion.
#*  1    mpeg      1.0         7/5/2004 7:05:55 PM    Ford Fu         CR(s)
#*       9660 9659 : SDC APPLICATION check in, make files 
#* $
#*  
#*  
#****************************************************************************


