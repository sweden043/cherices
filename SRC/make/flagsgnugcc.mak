###############################################################################
#                                                                             #
# CNXT MPEG build support file: rules.mak                                     #
#                                                                             #
# Author: M. Bintz 7/24/03                                                    #
#                                                                             #
# Compiler rules for all toolchains and file types.                           #
#                                                                             #
###############################################################################
!include $(SABINE_ROOT)\make\product.mak
!include $(SABINE_ROOT)\make\envcheck.mak

###########################################################################
#                                                                         # 
# Based on ARM_TOOKIT and ARM_VERSION, setup proper compiler, assembler,  #
# and linker variables                                                    #
#                                                                         # 
###########################################################################

##########################    ADS    ##############################
!if "$(ARM_TOOLKIT)" == "ADS"
CC     = $(ADS_BIN_PATH)\armcc.exe
CPP    = $(ADS_BIN_PATH)\armcpp.exe
CPU    = -cpu ARM9TDMI

##########################    SDT    ##############################
!elseif "$(ARM_TOOLKIT)" == "SDT"
CC     = $(SDT_BIN_PATH)\armcc.exe
CPP    = $(SDT_BIN_PATH)\armcpp.exe
CPU    = -processor ARM9TDMI

##########################   WRGCC   ##############################
!elseif "$(ARM_TOOLKIT)" == "WRGCC"
CC     = ccwrap -wrapE=$(GCC_BIN_PATH)\ccarm.exe 
CPP    = ccwrap -wrapE=$(GCC_BIN_PATH)\ccarm.exe 

##########################  GNUGCC   ##############################
!elseif "$(ARM_TOOLKIT)" == "GNUGCC"
CC     = ccwrap -wrapE=$(GCC_BIN_PATH)\ccarm.exe 
CPP    = ccwrap -wrapE=$(GCC_BIN_PATH)\ccarm.exe 

##########################  catchall ##############################
!else
!error build environment not properly set!

!endif

##################################
# Fix the INCLUDE path to remove #
# Leading/Trailing ';', and		 #
# any multiple ';'				 #
##################################
FOO=FOO
BAR=BAR
INC_FIX=$(FOO);$(INCLUDE);$(BAR)
INC_FIX=$(INC_FIX:;;;;;=;;;;)
INC_FIX=$(INC_FIX:;;;;=;;;)
INC_FIX=$(INC_FIX:;;;=;;)
INC_FIX=$(INC_FIX:;;=;)
INC_FIX=$(INC_FIX:FOO;=)
INC_FIX=$(INC_FIX:;BAR=)


##############  Comprehensive list of LINKER FLAGS  ################
TO_BIN_FLAGS    = 






















#****************************************************************************
#* $Log: 
#*  2    mpeg      1.1         9/18/03 5:52:14 PM     Miles Bintz     SCR(s) 
#*        7484 :
#*        added flags when calling binary conversion tool
#*        
#*  1    mpeg      1.0         9/9/03 4:40:40 PM      Miles Bintz     
#* $
#  
#     Rev 1.1   18 Sep 2003 16:52:14   bintzmf
#  SCR(s) 7484 :
#  added flags when calling binary conversion tool
#  
#     Rev 1.0   09 Sep 2003 15:40:40   bintzmf
#  SCR(s) 7291 :
#  reworked build system
#  
#     Rev 1.1   28 Aug 2003 15:25:46   bintzmf
#  No change.
#  
#     Rev 1.0   14 Aug 2003 13:41:46   bintzmf
#  new build system
#*  
#*  
#****************************************************************************


