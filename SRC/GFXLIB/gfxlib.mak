###############################################################################
#
# Sabine: CN49XX Graphics Library Makefile
#
# Author: Eric Ching, 3/3/2000
#
# Copyright, 2000 Conexant Systems. All Rights Reserved
#
###############################################################################

 ############################################################################
 ############################################################################
 ## IMPORTANT NOTE: Due to the use of string substitution in the top level ##
 ##                 makefile, all filenames MUST be in upper case !        ##
 ############################################################################
 ############################################################################

######################################################################
# File lists. These 5 macros define the files comprising this driver #
######################################################################
GENERAL_C_FILES = GFXINIT.C CONTEXT.C COPYMEM.C GXAISR.C

ARM_C_FILES =

THUMB_C_FILES =

ARM_ASM_FILES=   SWBLT.S

THUMB_ASM_FILES=

OTHER_FILES = PCIGXA.H

