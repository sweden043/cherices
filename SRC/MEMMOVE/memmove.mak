#****************************************************************************
#*                   CONEXANT PROPRIETARY AND CONFIDENTIAL                  *
#*                       SOFTWARE FILE/MODULE HEADER                        *
#*                    Conexant Systems Inc. (c) 1998-2003                   *
#*                                Austin, TX                                *
#*                           All Rights Reserved                            *
#****************************************************************************
#
#  Filename:        memmove.mak
#
#
#  Description:     Memory Driver Makefile
#
#
#  Author:          Dave Wilson
#
#****************************************************************************
#  $Header: memmove.mak, 14, 10/17/03 9:53:12 AM, Larry Wang$
#****************************************************************************

######################################################################
# File lists. These 5 macros define the files comprising this driver #
######################################################################

!if "$(RTOS)" == "NOOS"
ARM_ASM_FILES = XCOPY.S	XFILL.S
!else
ARM_ASM_FILES = XCOPY.S XFILL.S
GENERAL_C_FILES =   MEMMOVE.C CNXTHEAP.C
!endif

###############################################################################
# $Log: 
#  14   mpeg      1.13        10/17/03 9:53:12 AM    Larry Wang      CR(s): 
#        7673 Include XFILL.S in codeldr and codeldrext build.
#  13   mpeg      1.12        7/30/03 3:38:18 PM     Larry Wang      SCR(s) 
#        7076 :
#        For vxworks builds, get *.s instead of vx*.s
#        
#  12   mpeg      1.11        7/10/03 10:44:14 AM    Larry Wang      SCR(s) 
#        6924 :
#        Include the same modules in case RTOS==VXWORKS && APPNAME==CODELDR as 
#        in the one where RTOS==NOOS.
#        
#        
#  11   mpeg      1.10        5/14/03 4:08:28 PM     Tim White       SCR(s) 
#        6346 6347 :
#        Separated the fast-fill functions from the fast-copy functions.
#        
#        
#  10   mpeg      1.9         12/13/01 3:54:34 PM    Lucy C Allevato SCR(s) 
#        2970 :
#        Deleted source file rules
#        
#  9    mpeg      1.8         9/28/01 5:44:20 PM     Tim White       SCR(s) 
#        2691 :
#        Use the new performance optimized FCopy() function for o_memmove() 
#        calls
#        instead of memcpy() or FCopyBytes() which are both slower.
#        
#        
#  8    mpeg      1.7         9/4/01 4:44:30 PM      Dave Moore      
#  7    mpeg      1.6         12/11/00 10:48:46 AM   Miles Bintz     changed 
#        heap.c cnxtheap.c
#        
#  6    mpeg      1.5         11/15/00 9:06:32 AM    Dave Wilson     Added 
#        HEAP.C
#        
#  5    mpeg      1.4         3/9/00 4:19:14 PM      Miles Bintz     made vxw 
#        friendly
#        
#  4    mpeg      1.3         1/6/00 4:24:52 PM      Dave Wilson     Thumb 
#        build changes
#        
#  3    mpeg      1.2         9/2/99 4:59:38 PM      Ismail Mustafa  Added 
#        copy.s.
#        
#  2    mpeg      1.1         1/27/99 3:10:30 PM     Steve Glennon   Fixed so 
#        can be built without PVCS archive
#        
#  1    mpeg      1.0         4/17/98 10:12:30 AM    Dave Wilson     
# $
#  
#     Rev 1.12   30 Jul 2003 14:38:18   wangl2
#  SCR(s) 7076 :
#  For vxworks builds, get *.s instead of vx*.s
#  
#     Rev 1.11   10 Jul 2003 09:44:14   wangl2
#  SCR(s) 6924 :
#  Include the same modules in case RTOS==VXWORKS && APPNAME==CODELDR as in the one where RTOS==NOOS.
#  
#  
#     Rev 1.10   14 May 2003 15:08:28   whiteth
#  SCR(s) 6346 6347 :
#  Separated the fast-fill functions from the fast-copy functions.
#  
#
###############################################################################

