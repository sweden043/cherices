#############################################################################
##                            Conexant Systems                             ##
#############################################################################
##                                                                         ##
## Filename:       smartcard.mak                                           ##
##                                                                         ##
## Description:    Generic SmartCard driver makefile                       ##
##                                                                         ##
## Author:         Larry Wang                                              ##
##                                                                         ##
## Date:           01/08/2003                                              ##
##                                                                         ##
## Copyright Conexant Systems, 2003                                        ##
## All Rights Reserved.                                                    ##
##                                                                         ##
#############################################################################

######################################################################
# File lists. These 5 macros define the files comprising this driver #
######################################################################

GENERAL_C_FILES = SMC_DRV.C SMC_UTIL.C SMC_ISR.C

PUBLIC_HEADERS  = SMARTCARD.H

OTHER_FILES     = SMC_PRIV.H

