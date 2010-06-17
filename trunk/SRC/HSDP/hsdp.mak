###############################################################################
#                                                                             #
# Sabine: High Speed Data Port driver makefile                                #
#                                                                             #
# Author: Miles Bintz                                                         #
#                                                                             #
# Copyright, 2000, 2001, 2002, 2003 Conexant Systems Inc.                     #
# All Rights Reserved                                                         #
#                                                                             #
###############################################################################
#
# $Header: hsdp.mak, 7, 5/14/03 9:35:08 PM, Miles Bintz$
#
###############################################################################


  ############################################################################
  ############################################################################
  ## IMPORTANT NOTE: Due to the use of string substitution in the top level ##
  ##                 makefile, all filenames MUST be in upper case !        ##
  ############################################################################
  ############################################################################

GENERAL_C_FILES  =   HSDP.C       \
                     HSDPPRIV.C   \
                     CNXT_TUX.C   \
					 TS_ROUTE.C

OTHER_FILES      =   CNXT_TUX.H   \
					 TS_ROUTE.H   \


###############################################################################
# $Log: 
#  7    mpeg      1.6         5/14/03 9:35:08 PM     Miles Bintz     SCR(s) 
#        6353 6354 :
#        rework of HSDP driver
#        
#  6    mpeg      1.5         1/23/03 3:02:26 PM     Dave Wilson     SCR(s) 
#        5292 :
#        Removed the condition on inclusion of CNXT_TUX.C. This is used for all
#         our
#        IRDs so the condition was only confusing matters.
#        
#  5    mpeg      1.4         7/12/02 8:13:34 AM     Steven Jones    SCR(s): 
#        4176 
#        Fix for Brady.
#        
#  4    mpeg      1.3         3/27/02 6:11:54 AM     Steven Jones    SCR(s): 
#        3184 
#        Removed source-to-archive dependancies.
#        
#  3    mpeg      1.2         1/30/02 2:06:54 PM     Miles Bintz     SCR(s) 
#        3058 :
#        Development/interim checkin
#        
#        
#  2    mpeg      1.1         1/18/02 1:35:28 PM     Miles Bintz     SCR(s) 
#        3058 :
#        Beta checkin
#        
#        
#  1    mpeg      1.0         1/8/02 5:42:18 PM      Miles Bintz     
# $
#  
#     Rev 1.6   14 May 2003 20:35:08   bintzmf
#  SCR(s) 6353 6354 :
#  rework of HSDP driver
###############################################################################

