###############################################################################
#                                                                             #
# CNXT MPEG build support file: flagsads.mak                                  #
#                                                                             #
# Author: M. Bintz 7/24/03                                                    #
#                                                                             #
# Compiler rules for all toolchains and file types.                           #
#                                                                             #
# $Id: flagsads.mak,v 1.7, 2004-05-24 15:49:12Z, Bobby Bradford$                                                                        #
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
C_DEBUG_FLAGS =  -Ospace -g- -O1 -UDEBUG -Ono_data_reorder


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
C_WARN_FLAGS = -Wd




##############  Comprehensive list of C FLAGS  ################
CFLAGS = $(PACKING_FLAG) $(C_WARN_FLAGS) -cpu $(CPU) \
  -zo -zc -fy -c -apcs /noswst/inter -fpu softfpa -li \
  -J$(INC_FIX:;= -J)

##############  Comprehensive list of ASM FLAGS  ################
# CHIP_NAME is defined in the HW config files.  Config files have
# syntax like a makefile and so that definition is imported directly
# from the file itself (ie. cfgtoh doesn't munge the definition).
AFLAGS = $(A_DEBUG_FLAGS) -apcs /noswst/inter -li -fpu softfpa -cpu $(CPU) \
  -I$(INC_FIX:;= -I) -PD "CHIP_NAME SETS \"$(CHIP_NAME)\""
  
##############  Comprehensive list of LIBR FLAGS  ################
LIBRFLAGS = -create


##############  Comprehensive list of LINKER FLAGS  ################
LINKER_FLAGS    = -verbose
!if "$(SEMIHOSTING_APP)" == "YES" || "$(DOWNLOAD_LAUTERBACH_SUPPORT)" == "YES"
LOPTS1          = 
!else
LOPTS1          = -entry ADS_StartupEntry -first StartupEntry -remove 
!endif
LINK_MAP_OUTPUT = -info sizes,totals,veneers,unused -xref -map -symbols -list 


##############  Keep RCSID symbols, for UCOS/NDS    ################
!if "$(RTOS)" == "UCOS"
LOPTS1 = $(LOPTS1) -keep *RcsId* 
!endif

##############  Comprehensive list of LINKER FLAGS  ################
TO_BIN_FLAGS    = 
TO_BIN_OUTPUT   = -output

EXT_LIB = ADS
EXT_IMG = AXF





#****************************************************************************
#* $Log: 
#*  8    mpeg      1.7         5/24/04 10:49:12 AM    Bobby Bradford  CR(s) 
#*        9287 9288 : Add optional flag to LOPTS1 (UCOS only) that will keep 
#*        *RcsId* symbols in the image, even if they are not referenced.
#*  7    mpeg      1.6         3/16/04 3:12:53 PM     Miles Bintz     CR(s) 
#*        8567 : fixed axf/elf to binary conversion warning about -output being
#*         depricated
#*  6    mpeg      1.5         10/27/03 3:59:00 PM    Tim White       CR(s): 
#*        7724 Force C-library initilization when building serial flash 
#*        download tool with ADS toolkit.
#*        
#*  5    mpeg      1.4         10/23/03 12:29:31 PM   Larry Wang      CR(s): 
#*        7704 Don't define the entry point for lauterbach download build.
#*        
#*        
#*  4    mpeg      1.3         9/19/03 7:05:18 PM     Miles Bintz     SCR(s) 
#*        7484 :
#*        added support for semihosting apps
#*        
#*  3    mpeg      1.2         9/19/03 5:11:30 PM     Miles Bintz     SCR(s) 
#*        7484 :
#*        added semihostlib variables based on SDT_PATH and ADS_PATH
#*        
#*  2    mpeg      1.1         9/18/03 5:51:00 PM     Miles Bintz     SCR(s) 
#*        7484 :
#*        added flags when calling binary conversion tool
#*        
#*  1    mpeg      1.0         9/9/03 4:40:40 PM      Miles Bintz     
#* $
#  
#     Rev 1.3   19 Sep 2003 18:05:18   bintzmf
#  SCR(s) 7484 :
#  added support for semihosting apps
#  
#     Rev 1.2   19 Sep 2003 16:11:30   bintzmf
#  SCR(s) 7484 :
#  added semihostlib variables based on SDT_PATH and ADS_PATH
#  
#     Rev 1.1   18 Sep 2003 16:51:00   bintzmf
#  SCR(s) 7484 :
#  added flags when calling binary conversion tool
#  
#     Rev 1.0   09 Sep 2003 15:40:40   bintzmf
#  SCR(s) 7291 :
#  reworked build system
#  
#     Rev 1.2   04 Sep 2003 14:04:30   bintzmf
#  
#     Rev 1.1   28 Aug 2003 15:25:44   bintzmf
#  
#     Rev 1.0   14 Aug 2003 13:41:46   bintzmf
#  new build system
#*  
#*  
#****************************************************************************


