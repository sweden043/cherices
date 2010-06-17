###############################################################################
#                                                                             #
# Nucleus Plus driver makefile                                                #
#                                                                             #
# Author: Miles Bintz   8/29/2000                                             #
#                                                                             #
# Copyright, 2000 Conexant Systems. All Rights Reserved                       #
#                                                                             #
###############################################################################

######################################################################
# File lists. These 5 macros define the files comprising this driver #
######################################################################

ARM_C_FILES =               

THUMB_C_FILES =

ARM_ASM_FILES =                

THUMB_ASM_FILES =

OTHER_FILES = PLUS.LIB THM_PLUS.LIB PLUS.ADS THM_PLUS.ADS

# use special libs if NUP profiling hooks are enabled
!if "$(NUP_PROFILING)" == "YES"
OTHER_FILES = $(OTHER_FILES) PLUS_PROF.LIB THM_PLUS_PROF.LIB PLUS_PROF.ADS THM_PLUS_PROF.ADS
!endif

