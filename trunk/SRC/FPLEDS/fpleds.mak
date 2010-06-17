#****************************************************************************
#*                   CONEXANT PROPRIETARY AND CONFIDENTIAL                  *
#*                       SOFTWARE FILE/MODULE HEADER                        *
#*                    Conexant Systems Inc. (c) 1998-2003                   *
#*                                Austin, TX                                *
#*                           All Rights Reserved                            *
#****************************************************************************
#
#  Filename:        fpleds.mak
#
#
#  Description:     LED Driver makefile
#
#
#  Author:          Steve Glennon
#
#****************************************************************************
#  $Header: fpleds.mak, 5, 5/5/03 4:15:48 PM, Tim White$
#****************************************************************************

GENERAL_C_FILES =   FPLEDS.C

###############################################################################
# $Log: 
#  5    mpeg      1.4         5/5/03 4:15:48 PM      Tim White       SCR(s) 
#        6172 :
#        Removed 7 segment LED support.
#        
#        
#  4    mpeg      1.3         12/13/01 4:06:10 PM    Lucy C Allevato SCR(s) 
#        2970 :
#        Deleted source file rules
#        
#  3    mpeg      1.2         1/6/00 10:46:04 AM     Dave Wilson     Changes 
#        for ARM/Thumb build
#        
#  2    mpeg      1.1         1/27/99 3:16:20 PM     Steve Glennon   Fixed so 
#        can be built without PVCS archive
#        
#  1    mpeg      1.0         10/19/98 4:58:22 PM    Steve Glennon   
# $
#  
#     Rev 1.4   05 May 2003 15:15:48   whiteth
#  SCR(s) 6172 :
#  Removed 7 segment LED support.
#  
#
#
###############################################################################

