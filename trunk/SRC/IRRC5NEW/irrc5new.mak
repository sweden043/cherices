###############################################################################
#                                                                             #
# Sabine: Infra Red Decoder for Philips RC5                                   #
#                                                                             #
# Author: Steve Glennon 10/10/2002                                            #
#                                                                             #
# Copyright, 2002 Conexant Systems Inc., All Rights Reserved                  #
#                                                                             #
###############################################################################

######################################################################
# File lists. These 5 macros define the files comprising this driver #
######################################################################

GENERAL_C_FILES = RC5DECOD.C 

ARM_C_FILES = 

THUMB_C_FILES =

ARM_ASM_FILES =

THUMB_ASM_FILES=

OTHER_FILES = RC5KTABL.H

############################################################################
# These rules allow the top level makefile to find the library versions of #
# each file in the driver.                                                 #
############################################################################
!if "$(TYPE)" != "LOCAL" && "$(TYPE)" != "CLEAN" && "$(TYPE)" != "LABEL"
RC5DECOD.C:      $(PVCS_ROOT)\$(DRIVER)\RC5DECOD.C_V
RC5KTABL.H:      $(PVCS_ROOT)\$(DRIVER)\RC5KTABL.H_V
!endif
