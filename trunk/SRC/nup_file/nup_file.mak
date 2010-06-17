###############################################################################
#                                                                             #
# Nucleus Plus File System w/ CNXT ATA                                        #
#                                                                             #
# Author: Dave Moore, 2003                                                    #
#                                                                             #
# Copyright, 2003 Conexant Systems Inc.  All Rights Reserved                  #
#                                                                             #
###############################################################################

######################################################################
# $Header: nup_file.mak, 1, 8/22/03 5:25:36 PM, Dave Moore$
######################################################################

GENERAL_C_FILES = APIEXT.C   \
                  APIUTIL.C  \
                  BLOCK.C    \
                  DEVTABLE.C \
                  DROBJ.C    \
                  FILEINIT.C \
                  FILEUTIL.C \
                  LOWL.C     \
                  NUFP.C     \
                  NU_FILE.C  \
                  PC_ERROR.C \
                  PC_LOCKS.C \
                  PC_MEMRY.C \
                  PC_PART.C  \
                  PC_UDATE.C \
                  PC_UNICD.C \
                  PC_USERS.C \
                  CNXT_ATA_SHIM.C \
                  CNXT_USB_SHIM.C
##				  RAMDISK.C

OTHER_FILES=      PROTO.H \
                  PCDISK.H


EXTRA_INCLUDE_DIRS = $(EXTRA_INCLUDE_DIRS) $(SABINE_ROOT)\nupincl

