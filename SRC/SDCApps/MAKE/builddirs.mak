###############################################################################
#                                                                             #
# CNXT MPEG build support file: builddirs.mak                                 #
#                                                                             #
# Author: Paul Yan, June 22nd, 2004                                           #
#                                                                             #
# Compiler rules for all toolchains and file types.                           #
#                                                                             #
###############################################################################

#################################################
######## Directory Information ##################
#################################################

INTER_OUTPUT_DIR= $(CONFIG)\$(APPNAME)\$(RTOS)\$(SWCONFIG)\$(DEBUGDIR)\$(VERSION)


SDCAPP_OUTPUT_DIR = $(SDCAPPS_ROOT)\Build\$(CONFIG)\$(RTOS)\$(SWCONFIG)\$(DEBUGDIR)\$(VERSION)
LIB_OUTPUT_DIR  = $(SDCAPP_OUTPUT_DIR)\LIB
IMG_OUTPUT_DIR  = $(SDCAPP_OUTPUT_DIR)\BIN


#****************************************************************************
#* $Log:
#*  2    mpeg      1.1         7/9/2004 4:36:15 PM    Xiao Guang Yan  CR(s)
#*       9699 9700 : Changed OUTPUT_DIR to SDCAPPS_OUTPUT_DIR to avoid
#*       confusion.
#*  1    mpeg      1.0         7/5/2004 7:05:29 PM    Ford Fu         CR(s)
#*       9660 9659 : SDC APPLICATION check in, make files 
#* $
#*  
#****************************************************************************


