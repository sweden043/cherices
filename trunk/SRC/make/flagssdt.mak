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
!if "$(PACKING)" != "1"
PACKING_FLAG = -zas$(PACKING)
!else
PACKING_FLAG = 
!endif

#####################  WARNING FLAGS  #####################
# fc: Enable "limited pcc. Turns off warnings about cast of 
#     integers as function pointers.
# wd: Supresses warning about "deprecated declaration"
###########################################################
C_WARN_FLAGS = -fc -Wd




##############  Comprehensive list of C FLAGS  ################
CFLAGS = $(PACKING_FLAG) $(C_WARN_FLAGS) -processor $(CPU) \
  -za1 -zat4 -zap1 -zo -zc -fy -zz-1 -c \
  -J$(INC_FIX:;= -J) -apcs 3/32/nofp/noswst/narrow/softfp/interwork -li 

##############  Comprehensive list of ASM FLAGS  ################
# CHIP_NAME is defined in the HW config files.  Config files have
# syntax like a makefile and so that definition is imported directly
# from the file itself (ie. cfgtoh doesn't munge the definition).
AFLAGS = $(A_DEBUG_FLAGS) -apcs 3/32/nofp/noswst -li -cpu $(CPU) \
  -I$(INC_FIX:;= -I) -PD "CHIP_NAME SETS \"$(CHIP_NAME)\""
  
##############  Comprehensive list of LIBR FLAGS  ################
LIBRFLAGS = -c




##############  Comprehensive list of LINKER FLAGS  ################
LINKER_FLAGS    = -vv
LOPTS1          = -info interwork -info sizes -remove -nozeropad
LINK_MAP_OUTPUT = -map -symbols - -list

##############  Comprehensive list of LINKER FLAGS  ################
TO_BIN_FLAGS    = -nozeropad
TO_BIN_OUTPUT   = 


EXT_LIB = ALF
EXT_IMG = AXF






#****************************************************************************
#* $Log: 
#*  4    mpeg      1.3         3/16/04 3:13:44 PM     Miles Bintz     CR(s) 
#*        8567 : fixed axf/elf to binary conversion warning about -output being
#*         depricated
#*  3    mpeg      1.2         9/19/03 5:11:30 PM     Miles Bintz     SCR(s) 
#*        7484 :
#*        added semihostlib variables based on SDT_PATH and ADS_PATH
#*        
#*  2    mpeg      1.1         9/18/03 5:50:58 PM     Miles Bintz     SCR(s) 
#*        7484 :
#*        added flags when calling binary conversion tool
#*        
#*  1    mpeg      1.0         9/9/03 4:40:40 PM      Miles Bintz     
#* $
#  
#     Rev 1.2   19 Sep 2003 16:11:30   bintzmf
#  SCR(s) 7484 :
#  added semihostlib variables based on SDT_PATH and ADS_PATH
#  
#     Rev 1.1   18 Sep 2003 16:50:58   bintzmf
#  SCR(s) 7484 :
#  added flags when calling binary conversion tool
#  
#     Rev 1.0   09 Sep 2003 15:40:40   bintzmf
#  SCR(s) 7291 :
#  reworked build system
#  
#     Rev 1.2   04 Sep 2003 14:04:38   bintzmf
#  
#     Rev 1.1   28 Aug 2003 15:25:46   bintzmf
#  
#     Rev 1.0   14 Aug 2003 13:41:46   bintzmf
#  new build system
#*  
#*  
#****************************************************************************


