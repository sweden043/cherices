###############################################################################
#                                                                             #
# CNXT MPEG build support file: flags.mak                                     #
#                                                                             #
# Author: M. Bintz 7/24/03                                                    #
#                                                                             #
# Compiler rules for all toolchains and file types.                           #
#                                                                             #
###############################################################################


# product.mak includes rtos.mak
!include $(SABINE_ROOT)\make\product.mak
!include $(SABINE_ROOT)\make\envcheck.mak

##########################    ADS    ##############################
!if "$(ARM_TOOLKIT)" == "ADS"
!include $(SABINE_ROOT)\make\flagsads.mak

##########################    SDT    ##############################
!elseif "$(ARM_TOOLKIT)" == "SDT"
!include $(SABINE_ROOT)\make\flagssdt.mak

##########################   WRGCC   ##############################
!elseif "$(ARM_TOOLKIT)" == "WRGCC"
!include $(SABINE_ROOT)\make\flagswrgcc.mak

##########################  GNUGCC   ##############################
!elseif "$(ARM_TOOLKIT)" == "GNUGCC"
!include $(SABINE_ROOT)\make\flagsgnugcc.mak

##########################  catchall ##############################
!else
!error build environment not properly set!
!endif

###########################################################################
#                                                                         # 
# From here on, set up variables which are independent of the toolkit     #
# or version.                                                             #
#                                                                         # 
###########################################################################
!include $(SABINE_ROOT)\make\builddirs.mak
!include $(SABINE_ROOT)\make\cdefs.mak
!include $(SABINE_ROOT)\make\adefs.mak

##################################
# Fix the INCLUDE path to remove #
# Leading/Trailing ';', and      #
# any multiple ';'               #
##################################
FOO=FOO
BAR=BAR
INC_FIX=$(FOO);$(INCLUDE);$(OUTPUT_DIR);$(RTOS_INCL);$(BAR)
INC_FIX=$(INC_FIX:;;;;;=;;;;)
INC_FIX=$(INC_FIX:;;;;=;;;)
INC_FIX=$(INC_FIX:;;;=;;)
INC_FIX=$(INC_FIX:;;=;)
INC_FIX=$(INC_FIX:FOO;=)
INC_FIX=$(INC_FIX:;BAR=)




#****************************************************************************
#* $Log: 
#*  2    mpeg      1.1         9/17/03 12:52:40 PM    Miles Bintz     SCR(s) 
#*        7484 :
#*        tool includes (which are basically headers for CRT libs) should be 
#*        included by the RTOS.  Including tool libs should not be absolute 
#*        'cause some RTOS's supply their own CRT headers
#*        
#*  1    mpeg      1.0         9/9/03 4:40:38 PM      Miles Bintz     
#* $
#  
#     Rev 1.1   17 Sep 2003 11:52:40   bintzmf
#  SCR(s) 7484 :
#  tool includes (which are basically headers for CRT libs) should be included by the RTOS.  Including tool libs should not be absolute 'cause some RTOS's supply their own CRT headers
#  
#     Rev 1.0   09 Sep 2003 15:40:38   bintzmf
#  SCR(s) 7291 :
#  reworked build system
#  
#     Rev 1.0   28 Aug 2003 15:25:42   bintzmf
#  Initial revision.
#  
#     Rev 1.0   14 Aug 2003 13:41:46   bintzmf
#  new build system
#*  
#*  
#****************************************************************************


