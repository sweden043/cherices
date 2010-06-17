###############################################################################
#                                                                             #
# CNXT MPEG build support file: rules.mak                                     #
#                                                                             #
# Author: M. Bintz 7/24/03                                                    #
#                                                                             #
# Compiler rules for all toolchains and file types.                           #
#                                                                             #
###############################################################################

TOOL_INCL = $(WIND_BASE)\HOST\$(WIND_HOST_TYPE)\lib\gcc-lib\arm-wrs-vxworks\2.9-010413\include

###########################################################################
#                                                                         # 
# Based on ARM_TOOKIT and ARM_VERSION, setup proper compiler, assembler,  #
# and linker variables                                                    #
#                                                                         # 
###########################################################################

ACC     = $(WR_BIN_PATH)\ccarm.exe
TCC     = $(WR_BIN_PATH)\ccarm.exe -mthumb
AAS     = $(WR_BIN_PATH)\ccarm.exe
TAS     = $(WR_BIN_PATH)\ccarm.exe -mthumb
LIBR    = $(WR_BIN_PATH)\ararm.exe
ALINK   = $(WR_BIN_PATH)\ldarm.exe

!if "$(MODE)" == "THUMB"
TOOLKIT_LIBS = $(WIND_BASE)\target\lib\arm\ARMARCH4_T\common\libgcc.a \
               $(WIND_BASE)\target\lib\arm\ARMARCH4_T\common\libcommoncc.a
!else
TOOLKIT_LIBS = $(WIND_BASE)\target\lib\arm\ARMARCH4\common\libgcc.a \
               $(WIND_BASE)\target\lib\arm\ARMARCH4\common\libcommoncc.a
!endif

# Flags specific to this toolkit...
CPU = ARMARCH4

####################   DEBUG FLAGS    ####################
!if "$(DEBUG)" == "YES"
C_DEBUG_FLAGS   = -g 
A_DEBUG_FLAGS   = -gdwarf2
LINK_DEBUG      = -debug

!else    # NOT DEBUG

LINK_DEBUG      = 
A_DEBUG_FLAGS   = -DDEBUG=0 -fno-strict-aliasing
C_DEBUG_FLAGS   = -O3 -fno-strict-aliasing

!endif    # DEBUG

!if "$(GNU_CDEBUG_OVR)" != ""
C_DEBUG_FLAGS   = $(GNU_CDEBUG_OVR)
A_DEBUG_FLAGS   = $(GNU_CDEBUG_OVR)
!endif


#####################  WARNING FLAGS  #####################
!if "$(OVERRIDE_WARNINGS)" == "YES"
C_WARN_FLAGS = -w
!else
C_WARN_FLAGS = -Wall -Wcomment
!endif



##############  Comprehensive list of C FLAGS  ################
CFLAGS = $(C_WARN_FLAGS) -mcpu=arm9 -mapcs-32 -mno-sched-prolog \
   -mthumb-interwork -fno-builtin -fvolatile -c \
   -I$(INC_FIX:;= -I) 

##############  Comprehensive list of ASM FLAGS  ################
AFLAGS = $(A_DEBUG_FLAGS) -mapcs-32 -I$(INC_FIX:;= -I) -mno-fpu \
	-mthumb-interwork -x assembler-with-cpp -DCHIP_NAME=$(CHIP_NAME) -c
  
##############  Comprehensive list of LIBR FLAGS  ################
LIBRFLAGS = -crs


##############  Comprehensive list of LINKER FLAGS  ################
LINKER_FLAGS = -vv
LOPTS1= -X -N -warn-common 
LINK_MAP_OUTPUT= --cref -Map
           
##############  Comprehensive list of LINKER FLAGS  ################
TO_BIN_FLAGS    =

EXT_LIB = WRGCC
EXT_IMG = ELF


#****************************************************************************
#* $Log: 
#*  6    mpeg      1.5         10/17/03 5:04:33 PM    Miles Bintz     CR(s): 
#*        7677 fixed error in preprocessing logic.  rtos implies nothing about 
#*        compilation mode.
#*  5    mpeg      1.4         10/17/03 10:41:29 AM   Larry Wang      CR(s): 
#*        7673 Enable Thumb mode to NOOS build.
#*  4    mpeg      1.3         10/1/03 11:27:58 AM    Miles Bintz     SCR(s) 
#*        7593 :
#*        fixed typo
#*        
#*  3    mpeg      1.2         9/18/03 5:52:12 PM     Miles Bintz     SCR(s) 
#*        7484 :
#*        added flags when calling binary conversion tool
#*        
#*  2    mpeg      1.1         9/12/03 5:30:54 PM     Miles Bintz     SCR(s) 
#*        7291 :
#*        changes lib extension to wrgcc instead of vxw
#*        
#*  1    mpeg      1.0         9/9/03 4:40:42 PM      Miles Bintz     
#* $
#****************************************************************************


