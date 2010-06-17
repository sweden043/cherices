#****************************************************************************
#*                   CONEXANT PROPRIETARY AND CONFIDENTIAL                  *
#*                       SOFTWARE FILE/MODULE HEADER                        *
#*                    Conexant Systems Inc. (c) 2001-2003                   *
#*                                Austin, TX                                *
#*                           All Rights Reserved                            *
#****************************************************************************
#
#  Filename:        demux.mak
#
#
#  Description:     Demux component makefile
#
#
#  Author:          Bob Van Gulick
#
#****************************************************************************
#  $Id: demux.mak,v 1.9, 2004-03-23 21:20:27Z, Larry Wang$
#****************************************************************************

  ############################################################################
  ############################################################################
  ## IMPORTANT NOTE: Due to the use of string substitution in the top level ##
  ##                 makefile, all filenames MUST be in upper case !        ##
  ############################################################################
  ############################################################################

######################################################################
# File lists. These 5 macros define the files comprising this driver #
######################################################################

PVR_C_FILES =
DVR_C_FILES =
DMA_C_FILES =
XTV_C_FILES =

!if "$(PVR)" == "YES"
PVR_C_FILES = DEMUXREC.C
!endif

!if "$(LEGACY_DVR)" == "YES"
DVR_C_FILES = DEMUXDVR.C
!endif

!if "$(DMXDMA)" == "YES"
DMA_C_FILES = DEMUXDMA.C
!endif

!if "$(XTV_SUPPORT)" == "YES"
XTV_C_FILES = DEMUXXTV.C DEMUXREC.C
!endif

GENERAL_C_FILES = DEMUXAPI.C     \
                  DEMUXHW.C      \
                  DEMUXDESC.C    \
                  DEMUXMISC.C    \
                  DEMUXSI.C      \
                  $(PVR_C_FILES) \
                  $(DVR_C_FILES) \
                  $(XTV_C_FILES) \
                  $(DMA_C_FILES)

OTHER_FILES     = DEMUXINT.H

###############################################################################
# Modifications:
# $Log: 
#  10   mpeg      1.9         3/23/04 3:20:27 PM     Larry Wang      CR(s) 8638
#         8639 : Add new file for XTV demux support.
#  9    mpeg      1.8         3/10/04 2:15:06 PM     Tim Ross        CR(s) 8545
#         : Changed demuxpvr.c to demuxrec.c. demuxpvr.c is no longer used.
#  8    mpeg      1.7         11/19/03 10:09:51 AM   Tim White       CR(s): 
#        7987 Added Demux DMA and Demux PVR extension support phase 1.
#        
#  7    mpeg      1.6         10/15/03 5:20:55 PM    Tim White       CR(s): 
#        7659 Add PVR Demux Extension file to module conditionally.
#        
#  6    mpeg      1.5         4/26/02 3:15:52 PM     Tim White       SCR(s) 
#        3562 :
#        Use LEGACY_DVR instead of obsolete DVR hwconfig option.
#        
#        
#  5    mpeg      1.4         2/18/02 6:52:02 AM     Steven Jones    SCR(s): 
#        3184 
#        Deleted source-to-archive dependancies.
#        
#  4    mpeg      1.3         2/12/02 2:22:04 PM     Tim White       SCR(s) 
#        3177 :
#        Remove pawser microcode from demux directory.  Applications use new 
#        pawser directory for pawser microcode which works for both demux and 
#        gendmxc drivers.
#        
#        
#  3    mpeg      1.2         2/6/02 6:01:38 PM      Bob Van Gulick  SCR(s) 
#        3143 :
#        Add demuxint.h to file list so that GET's work correctly
#        
#        
#  2    mpeg      1.1         12/19/01 9:33:48 AM    Tim Ross        SCR(s) 
#        2933 :
#        demuxint.h was missing from "GET" section, leaving it absent in a 
#        fresh get.
#        
#  1    mpeg      1.0         12/17/01 5:32:32 PM    Bob Van Gulick  
# $
###############################################################################

