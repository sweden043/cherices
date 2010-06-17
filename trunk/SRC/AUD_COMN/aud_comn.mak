###############################################################################
#                                                                             #
# Sabine: OpenTV audio driver makefile                                        #
#                                                                             #
# Author: Amy Pratt, 3/23/98                                                  #
#                                                                             #
# Copyright, 1998 Rockwell Semiconductor Systems. All Rights Reserved         #
#                                                                             #
###############################################################################

######################################################################
# File lists. These 5 macros define the files comprising this driver #
######################################################################

######################################################################
# $Header: aud_comn.mak, 16, 5/15/03 6:25:52 PM, Dave Aerne$
######################################################################

ARM_C_FILES = 

GENERAL_C_FILES =   AUD_COMN.C AUD_API.C AVUTCOMN.C AUD_ERROR.C DATADRYFIX.C

!if "$(USE_BRAZOS16_PCM_SW_WORKAROUND)" == "YES"
GENERAL_C_FILES = $(GENERAL_C_FILES) BRAZOS16PCM.C
!endif # USE_BRAZOS16_PCM_SW_WORKAROUND == YES 

!if "$(AUDIO_MICROCODE)" == "UCODE_COLORADO"
GENERAL_C_FILES = $(GENERAL_C_FILES) COL_MPEG_AUDIO_UCODE.C AC3FIX.C
!else
!if "$(AUDIO_MICROCODE)" == "UCODE_HONDO"
GENERAL_C_FILES = $(GENERAL_C_FILES) HOND_MPEG_AUDIO_UCODE.C 
!else
!if "$(AUDIO_MICROCODE)" == "UCODE_WABASH"
GENERAL_C_FILES = $(GENERAL_C_FILES) WABASH_MPEG_AUDIO_UCODE.C 
!else
GENERAL_C_FILES = $(GENERAL_C_FILES) BRAZOS_MPEG_AUDIO_UCODE.C BRAZOS_AOC_AUDIO_UCODE.C
!endif
!endif
!endif

THUMB_C_FILES =

!if "$(USE_BRAZOS16_PCM_SW_WORKAROUND)" == "YES"
ARM_ASM_FILES = BRAZOS16PCMS.S
!endif # USE_BRAZOS16_PCM_SW_WORKAROUND == YES 

THUMB_ASM_FILES=

OTHER_FILES =

PUBLIC_HEADERS = AUD_ERROR.H

######################################################################
# $Log: 
#  16   mpeg      1.15        5/15/03 6:25:52 PM     Dave Aerne      SCR(s) 
#        6261 6262 6291 :
#        added case for selection of brazos audio micro code files
#        
#  15   mpeg      1.14        2/4/03 2:46:18 PM      Senthil Veluswamy SCR(s) 
#        5401 :
#        Added support for the Brazos 16bit memory, pcm playback sw workaround
#        
#  14   mpeg      1.13        12/16/02 3:42:30 PM    Tim White       SCR(s) 
#        5169 :
#        Allow future chips to use Wabash code by default instead of the 
#        Colorado code.
#        
#        
#  13   mpeg      1.12        11/20/02 5:55:34 PM    Craig Dry       SCR(s) 
#        4991 :
#        Canal+ DLI4.2 Audio Extensions and Audio Driver Enhancements
#        
#  12   mpeg      1.11        11/15/02 4:02:18 PM    Senthil Veluswamy SCR(s) 
#        4965 :
#        Added the Data dry fix file.
#        
#  11   mpeg      1.10        11/15/02 12:37:58 PM   Senthil Veluswamy SCR(s) 
#        4935 :
#        Modified file gets to include the Software AC3 fix for all Colorado 
#        builds
#        
#  10   mpeg      1.9         11/7/02 6:22:02 PM     Senthil Veluswamy SCR(s) 
#        4790 :
#        removed the ac3fix assembly file - no longer needed
#        
#  9    mpeg      1.8         10/21/02 4:53:28 PM    Senthil Veluswamy SCR(s) 
#        4790 :
#        Added files for the AC3 software workaround
#        
#  8    mpeg      1.7         4/26/02 5:22:26 PM     Tim White       SCR(s) 
#        3635 :
#        Updated mpeg audio microcode to version 1.18 for all chips supported.
#        
#        
#  7    mpeg      1.6         12/18/01 5:27:04 PM    Quillian Rutherford SCR(s)
#         2933 :
#        Merged wabash code
#        
#        
#  6    mpeg      1.5         12/13/01 4:17:36 PM    Lucy C Allevato SCR(s) 
#        2970 :
#        Deleted source file rules
#        
#  5    mpeg      1.4         9/18/01 9:25:40 AM     Quillian Rutherford SCR(s)
#         2633 2634 2635 2636 2637 :
#        Forgot to list files needed in the bottom for TYPE=GET to work.
#        
#        
#  4    mpeg      1.3         9/13/01 3:21:50 PM     Quillian Rutherford SCR(s)
#         2633 2634 2635 2636 2637 :
#        CHanged to include C files containing audio ucode
#        
#        
#  3    mpeg      1.2         7/3/01 10:23:04 AM     Tim White       SCR(s) 
#        2178 2179 2180 :
#        Merge branched Hondo specific code back into the mainstream source 
#        database.
#        
#        
#  2    mpeg      1.1         2/1/01 1:20:56 PM      Tim Ross        DCS966. 
#        DCS914.
#        Added Hondo support. 
#        Removed Neches support.
#        
#  1    mpeg      1.0         5/25/00 2:41:28 PM     Amy Pratt       
# $
#  
#     Rev 1.15   15 May 2003 17:25:52   aernedj
#  SCR(s) 6261 6262 6291 :
#  added case for selection of brazos audio micro code files
#  
#     Rev 1.14   04 Feb 2003 14:46:18   velusws
#  SCR(s) 5401 :
#  Added support for the Brazos 16bit memory, pcm playback sw workaround
#  
#     Rev 1.13   16 Dec 2002 15:42:30   whiteth
#  SCR(s) 5169 :
#  Allow future chips to use Wabash code by default instead of the Colorado code.
#  
#  
#     Rev 1.12   20 Nov 2002 17:55:34   dryd
#  SCR(s) 4991 :
#  Canal+ DLI4.2 Audio Extensions and Audio Driver Enhancements
#  
#     Rev 1.11   15 Nov 2002 16:02:18   velusws
#  SCR(s) 4965 :
#  Added the Data dry fix file.
######################################################################

