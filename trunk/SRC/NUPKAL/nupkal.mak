###############################################################################
#                                                                             #
# Sabine: pSOS Kernel Adaptation Layer makefile                               #
#                                                                             #
# Author: Dave Wilson, 3/5/98                                                 #
#                                                                             #
# Copyright, 1998 Rockwell Semiconductor Systems. All Rights Reserved         #
#                                                                             #
###############################################################################

######################################################################
# File lists. These 5 macros define the files comprising this driver #
######################################################################

!if "$(EXTENDED_KAL)" == "YES"
GENERAL_C_FILES = NETFUNCS.C CRTHOOKS.C NUPKAL.C NUPKALEX.C
OTHER_FILES     = KALINT.H KALINTEX.H
!else
GENERAL_C_FILES = NETFUNCS.C CRTHOOKS.C NUPKAL.C
OTHER_FILES     = KALINT.H
!endif

# include extra file if NUP profiling hooks are enabled
!if "$(NUP_PROFILING)" == "YES"
GENERAL_C_FILES     = $(GENERAL_C_FILES) NUPPROF.C
!endif

# if we are building rtos from source, then enable internal rtos struct
# definitions to help with debugging
!if "$(BUILDOS)" == "YES"
EXTRA_ARM_C_FLAGS     = $(EXTRA_ARM_C_FLAGS) -DNU_DEBUG
EXTRA_THUMB_C_FLAGS   = $(EXTRA_THUMB_C_FLAGS) -DNU_DEBUG
!endif

