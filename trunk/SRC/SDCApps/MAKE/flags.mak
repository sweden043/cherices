###############################################################################
#                                                                             #
# CNXT MPEG build support file: flags.mak                                     #
#                                                                             #
# Author: M. Bintz 7/24/03                                                    #
#                                                                             #
# Compiler rules for all toolchains and file types.                           #
#                                                                             #
###############################################################################


##########################    ADS    ##############################
!if "$(ARM_TOOLKIT)" == "ADS"
!include $(SDCAPPS_ROOT)\make\flagsads.mak

##########################    SDT    ##############################
!elseif "$(ARM_TOOLKIT)" == "SDT"
!include $(SDCAPPS_ROOT)\make\flagssdt.mak

##########################  catchall ##############################
!else
!error SDC applications can only be build with ADS or SDT. 
!endif

###########################################################################
#                                                                         # 
# From here on, set up variables which are independent of the toolkit     #
# or version.                                                             #
#                                                                         # 
###########################################################################
!include $(SDCAPPS_ROOT)\make\rtos.mak
!include $(SDCAPPS_ROOT)\make\cdefs.mak
!include $(SDCAPPS_ROOT)\make\adefs.mak
##################################
# Fix the INCLUDE path to remove #
# Leading/Trailing ';', and      #
# any multiple ';'               #
##################################
FOO=FOO
BAR=BAR
INC_FIX=$(FOO);$(INCLUDE);$(SDCAPP_OUTPUT_DIR);$(SDCAPPS_ROOT)\include;$(RTOS_INCL);$(BAR)
INC_FIX=$(INC_FIX:;;;;;=;;;;)
INC_FIX=$(INC_FIX:;;;;=;;;)
INC_FIX=$(INC_FIX:;;;=;;)
INC_FIX=$(INC_FIX:;;=;)
INC_FIX=$(INC_FIX:FOO;=)
INC_FIX=$(INC_FIX:;BAR=)




#****************************************************************************
#* $Log:
#*  2    mpeg      1.1         7/9/2004 4:53:55 PM    Xiao Guang Yan  CR(s)
#*       9699 9700 : Changed OUTPUT_DIR to SDCAPPS_OUTPUT_DIR to avoid
#*       confusion.
#*  1    mpeg      1.0         7/5/2004 7:08:25 PM    Ford Fu         CR(s)
#*       9660 9659 : SDC APPLICATION check in, make files 
#* $
#*  
#****************************************************************************


