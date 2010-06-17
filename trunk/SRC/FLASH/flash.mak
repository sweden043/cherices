###############################################################################
#                                                                             #
#   Low-level Flash Device Driver                                             #
#                                                                             #
#   Copyright Conexant Systems, 2001  All Rights Reserved.                    #
#                                                                             #
###############################################################################

######################################################################
# File lists. These 5 macros define the files comprising this driver #
######################################################################

GENERAL_C_FILES = FLASHRW.C       \
                  FLASHAMD.C      \
                  FLASHINT.C      \
                  FLASHSAMNAND.C  \
                  ECC.C

OTHER_FILES     = FLASHRW.H ECC.H


