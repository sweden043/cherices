###############################################################################
#                                                                             #
# Sabine: Conexant OSD Library Makefile                                       #
#                                                                             #
# Copyright, 1999 Conexant Systems. All Rights Reserved                       #
#                                                                             #
###############################################################################

  ############################################################################
  ############################################################################
  ## IMPORTANT NOTE: Due to the use of string substitution in the top level ##
  ##                 makefile, all filenames MUST be in upper case !        ##
  ############################################################################
  ############################################################################

!if "$(RTOS)" == "NOOS"

GENERAL_C_FILES= DRMFILTER.C

OTHER_FILES= DRMFILTER.H

!else

GENERAL_C_FILES= OSDISRC.C   \
                 OSDLIBC.C   \
                 VIDLIBC.C   \
                 CURLIBC.C   \
                 DRMFILTER.C \
                 ENCODER.C

OTHER_FILES= OSDISRC.H    \
             CURPRVC.H    \
             OSDPRV.H     \
             DRMFILTER.H  \
             ENCODER_PRIV.H

!endif

ARM_ASM_FILES=   

