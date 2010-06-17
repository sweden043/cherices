###############################################################################
#                                                                             #
# Sabine: OpenTV 1.2 JPEG Image Decompression Driver                          #
#                                                                             #
# Author: Dave Wilson, 17/1/00                                                #
#                                                                             #
# Copyright, 2000 Conexant Systems Inc. All Rights Reserved                   #
#                                                                             #
###############################################################################

######################################################################
# File lists. These 5 macros define the files comprising this driver #
######################################################################

GENERAL_C_FILES= JPEG.C 

# pull in both ads and sdt libs
OTHER_FILES = ARMJAPI.H JPEGLIB.ADS JPEGLIB.ALF

