###############################################################################
#                                                                             #
# Sabine: Nucleus Plus Board Support Package makefile                         #
#                                                                             #
# Author: Miles Bintz 6/28/2000                                               #
#                                                                             #
# Copyright, 2000 Conexant Systems.   All Rights Reserved                     #
#                                                                             #
###############################################################################

######################################################################
# File lists. These 5 macros define the files comprising this driver #
######################################################################

ARM_ASM_FILES  = NUP_INIS.S   \
                 NUP_IRQW.S

GENERAL_C_FILES= NUP_INIC.C  \
                 TRACEPORT.C \

ARM_C_FILES=     NUP_IRQ.C \

OTHER_FILES=     NUP_CFG.H

# if we are building rtos from source, then enable internal rtos struct
# definitions to help with debugging
!if "$(BUILDOS)" == "YES"
EXTRA_ARM_C_FLAGS     = $(EXTRA_ARM_C_FLAGS) -DNU_DEBUG
EXTRA_THUMB_C_FLAGS   = $(EXTRA_THUMB_C_FLAGS) -DNU_DEBUG
!endif

