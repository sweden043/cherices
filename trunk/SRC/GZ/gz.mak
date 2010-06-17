###############################################################################
#                                                                             #
# Sabine: ZLIB Makefile                                                       #
#                                                                             #
# Author: Derek Xi, 4/26/2004 10:13AM                                         #
#                                                                             #
# Copyright, 2004 SDC Conexant Systems. All Rights Reserved                   #
#                                                                             #
###############################################################################

 
!if "$(RTOS)" == "NOOS"

ARM_C_FILES =     INFFAST.C  \
                  INFLATE.C 

GENERAL_C_FILES= UNCOMPR.C  \
                 COMPRESS.C \
                 ADLER32.C  \
                 ZUTIL.C    \
                 DEFLATE.C  \
                 TREES.C    \
                 CRC32.C    \
                 INFBACK.C  \
                 INFTREES.C  

                 

OTHER_FILES=     ZLIB.H     \
                 ZCONF.H    \
                 INFLATE.H  \
                 INFFAST.H  \
                 ZUTIL.H    \
                 DEFLATE.H  \
                 TREES.H    \
                 CRC32.H    \
                 INFFIXED.H \
                 INFTREES.H
!endif

