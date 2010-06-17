###############################################################################
#                                                                             #
# CNXT MPEG build support file: flagsads.mak                                  #
#                                                                             #
# Author: M. Bintz 7/24/03                                                    #
#                                                                             #
# Compiler rules for all toolchains and file types.                           #
#                                                                             #
# $Id: flagsads.mak,v 1.1 2007/10/24 09:39:10 chenhb27 Exp $                                                                        #
#                                                                             #
###############################################################################

###########################################################################
#                                                                         # 
# Based on ARM_TOOKIT and ARM_VERSION, setup proper compiler, assembler,  #
# and linker variables                                                    #
#                                                                         # 
###########################################################################

TOOL_INCL = $(ADS_INC)
	
ACC     = $(ADS_BIN_PATH)\armcc.exe
TCC     = $(ADS_BIN_PATH)\tcc.exe
AAS     = $(ADS_BIN_PATH)\armasm.exe -32
TAS     = $(ADS_BIN_PATH)\armasm.exe -16
LIBR    = $(ADS_BIN_PATH)\armar.exe
#链接时显示详细信息
#ALINK   = $(ADS_BIN_PATH)\armlink.exe -verbose
ALINK   = $(ADS_BIN_PATH)\armlink.exe
TOBIN   = $(ADS_BIN_PATH)\fromelf.exe

# TOOLKIT_LIBS = $(ADS_LIB_PATH)
ARMLIB = $(ADS_LIB_PATH)
SEMIHOSTLIB = $(ADS_PATH)\lib

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
# C_DEBUG_FLAGS =  -Ospace -g- -O1 -UDEBUG -Ono_data_reorder
C_DEBUG_FLAGS =  -Ospace -g- -O2 -UDEBUG -Ono_data_reorder


!endif    # DEBUG


####################   PACKING FLAGS    ####################
PACKING_FLAG = 

#####################  WARNING FLAGS  #####################
# fc: Enable "limited pcc. Turns off warnings about cast of 
#     integers as function pointers.
# wd: Supresses warning about "deprecated declaration"
###########################################################
C_WARN_FLAGS = -Wd




##############  Comprehensive list of C FLAGS  ################
CFLAGS = $(PACKING_FLAG) $(C_WARN_FLAGS) -cpu $(CPU) \
  -zo -zc -fy -c -apcs /noswst/inter -fpu softfpa -li \
  -J$(INC_FIX:;= -J) -J$(SDCAPPS_ROOT)\include

##############  Comprehensive list of ASM FLAGS  ################
# CHIP_NAME is defined in the HW config files.  Config files have
# syntax like a makefile and so that definition is imported directly
# from the file itself (ie. cfgtoh doesn't munge the definition).
AFLAGS = $(A_DEBUG_FLAGS) -apcs /noswst/inter -li -fpu softfpa -cpu $(CPU) \
  -I$(INC_FIX:;= -I) -PD "CHIP_NAME SETS \"$(CHIP_NAME)\""
  
##############  Comprehensive list of LIBR FLAGS  ################
LIBRFLAGS = -create
LIBRFLAGS_REPLACE = -rs


##############  Comprehensive list of LINKER FLAGS  ################
LINKER_FLAGS    = -verbose
!if "$(SEMIHOSTING_APP)" == "YES" || "$(DOWNLOAD_LAUTERBACH_SUPPORT)" == "YES"
LOPTS1          = 
!else
LOPTS1          = -entry ADS_StartupEntry -first StartupEntry -remove 
!endif
LINK_MAP_OUTPUT = -info sizes,totals,veneers,unused -xref -map -symbols -list 

##############  Comprehensive list of LINKER FLAGS  ################
TO_BIN_FLAGS    = 


EXT_LIB = ADS
EXT_IMG = AXF





#****************************************************************************
#* $Log:
#*  1    mpeg      1.0         7/5/2004 7:08:50 PM    Ford Fu         CR(s)
#*       9660 9659 : SDC APPLICATION check in, make files 
#* $
#*  
#****************************************************************************


