###############################################################################
#                                                                             #
# Colorado: PCM Driver makefile                                               #
#                                                                             #
# Author: Matthew W. Korte, 1/11/01                                           #
#                                                                             #
# Copyright, 2001 Conexant Systems, Inc. All Rights Reserved                  #
#                                                                             #
###############################################################################

###############################################################################
# $Header: pcm.mak, 7, 7/30/03 3:44:20 PM, Larry Wang$
###############################################################################

######################################################################
# File lists. These 5 macros define the files comprising this driver #
######################################################################

ARM_C_FILES = 

GENERAL_C_FILES =   PCM.C PCM_API.C PCMINIT.C ATTENUAT.C PCMCQ.C PCMSRC.C

!if "$(USE_UNOPTIMIZED_PCM_SRC)" == "NO"
ARM_ASM_FILES = PCMSRCS.S
!endif #"$(USE_UNOPTIMIZED_PCM_SRC)" == "NO"

THUMB_C_FILES =

THUMB_ASM_FILES=

OTHER_FILES =  PCMPRIV.H PCMCQ.H

###############################################################################
# $Log: 
#  7    mpeg      1.6         7/30/03 3:44:20 PM     Larry Wang      SCR(s) 
#        7076 :
#        For vxworks builds, get *.s instead of vx*.s
#        
#  6    mpeg      1.5         11/27/02 12:51:34 PM   Senthil Veluswamy SCR(s) 
#        5001 :
#        Added check for USE_UNOPTIMIZED_PCM_SRC to select between using the 
#        Optimized ARM routines and the Unoptimized routines
#        
#  5    mpeg      1.4         11/22/02 10:52:36 AM   Senthil Veluswamy SCR(s) 
#        5012 :
#        Wrapped the optimized routines for VxW for now.
#        
#  4    mpeg      1.3         11/20/02 5:56:02 PM    Senthil Veluswamy SCR(s) 
#        5001 :
#        Added ARM Assembly file containing Optimized SRC routines
#        
#  3    mpeg      1.2         8/27/02 8:59:04 AM     Ian Mitchell    SCR(s): 
#        3184 
#        removed dependencies
#        
#  2    mpeg      1.1         5/15/02 3:28:08 AM     Steve Glennon   SCR(s): 
#        2438 
#        Major work to implement full functions required of driver
#        
#        
#  1    mpeg      1.0         8/8/01 2:47:38 PM      Matt Korte      
# $
#  
#     Rev 1.6   30 Jul 2003 14:44:20   wangl2
#  SCR(s) 7076 :
#  For vxworks builds, get *.s instead of vx*.s
#  
#     Rev 1.5   27 Nov 2002 12:51:34   velusws
#  SCR(s) 5001 :
#  Added check for USE_UNOPTIMIZED_PCM_SRC to select between using the Optimized ARM routines and the Unoptimized routines
#  
#     Rev 1.4   22 Nov 2002 10:52:36   velusws
#  SCR(s) 5012 :
#  Wrapped the optimized routines for VxW for now.
#  
#     Rev 1.3   20 Nov 2002 17:56:02   velusws
#  SCR(s) 5001 :
#  Added ARM Assembly file containing Optimized SRC routines
###############################################################################

