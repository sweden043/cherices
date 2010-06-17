###############################################################################
#                                                                             #
# CNXT MPEG build support file: rulessdt.mak                                  #
#                                                                             #
# Author: M. Bintz 7/24/03                                                    #
#                                                                             #
# Compiler rules for all toolchains and file types.                           #
#                                                                             #
###############################################################################

###########################################################################
#                                                                         # 
# Based on ARM_TOOKIT and ARM_VERSION, setup proper compiler, assembler,  #
# and linker variables                                                    #
#                                                                         # 
###########################################################################

TOOL_INCL = $(SDT_INC)
	
ACC     = $(SDT_BIN_PATH)\armcc.exe
TCC     = $(SDT_BIN_PATH)\tcc.exe
AAS     = $(SDT_BIN_PATH)\armasm.exe -32
TAS     = $(SDT_BIN_PATH)\armasm.exe -16
LIBR    = $(SDT_BIN_PATH)\armlib.exe
ALINK   = $(SDT_BIN_PATH)\armlink.exe
TOBIN   = $(SDT_BIN_PATH)\fromelf.exe

# TOOLKIT_LIBS = $(SDT_LIB_PATH)
ARMLIB = $(SDT_LIB_PATH)
SEMIHOSTLIB = $(SDT_PATH)\lib

# Flags specific to this toolkit...
CPU = ARM9TDMI

####################   DEBUG FLAGS    ####################
!if "$(DEBUG)" == "YES"
C_DEBUG_FLAGS   = -g+ 
A_DEBUG_FLAGS   = -g
LINK_DEBUG      = -debug
!else    # NOT DEBUG

LINK_DEBUG      =

# DEBUG Options for DEBUG != YES, ARM_TOOLKIT = SDT, ARM_VERSION = 250
!if "$(ARM_VERSION)" == "250"           
C_DEBUG_FLAGS = -Ospace -g- -o2

# DEBUG Options for DEBUG != YES, ARM_TOOLKIT = SDT, ARM_VERSION = 251
!elseif "$(ARM_VERSION)" == "251"
C_DEBUG_FLAGS = -Ospace -g- -gxo
!endif                                  # ARM_VERSION

!endif    # DEBUG


####################   PACKING FLAGS    ####################
PACKING_FLAG = 

#####################  WARNING FLAGS  #####################
# fc: Enable "limited pcc. Turns off warnings about cast of 
#     integers as function pointers.
# wd: Supresses warning about "deprecated declaration"
###########################################################
C_WARN_FLAGS = -fc -Wd




##############  Comprehensive list of C FLAGS  ################
CFLAGS = $(PACKING_FLAG) $(C_WARN_FLAGS) -processor $(CPU) \
  -za1 -zat4 -zap1 -zo -zc -fy -zz-1 -c -J$(SDCAPPS_ROOT)include \
  -J$(INC_FIX:;= -J) -apcs 3/32/nofp/noswst/narrow/softfp/interwork -li

##############  Comprehensive list of ASM FLAGS  ################
# CHIP_NAME is defined in the HW config files.  Config files have
# syntax like a makefile and so that definition is imported directly
# from the file itself (ie. cfgtoh doesn't munge the definition).
AFLAGS = $(A_DEBUG_FLAGS) -apcs 3/32/nofp/noswst -li -cpu $(CPU) \
  -I$(INC_FIX:;= -I) -PD "CHIP_NAME SETS \"$(CHIP_NAME)\""
  
##############  Comprehensive list of LIBR FLAGS  ################
LIBRFLAGS = -c
LIBRFLAGS_REPLACE = -i




##############  Comprehensive list of LINKER FLAGS  ################
LINKER_FLAGS    = -vv
LOPTS1          = -info interwork -info sizes -remove -nozeropad
LINK_MAP_OUTPUT = -map -symbols - -list

##############  Comprehensive list of LINKER FLAGS  ################
TO_BIN_FLAGS    = -nozeropad


EXT_LIB = ALF
EXT_IMG = AXF






#****************************************************************************
#* $Log:
#*  1    mpeg      1.0         7/5/2004 7:09:27 PM    Ford Fu         CR(s)
#*       9660 9659 : SDC APPLICATION check in, make files 
#* $
#*  
#*  
#****************************************************************************


