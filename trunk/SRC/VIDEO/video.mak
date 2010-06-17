###############################################################################
#                                                                             #
# Video Driver Module Make File                                               #
#                                                                             #
# Author: D. Wilson                                                           #
#                                                                             #
# Copyright, 2002 Conexant System Incorporated. All Rights Reserved           #
#                                                                             #
###############################################################################

GENERAL_C_FILES= VIDEO.C MICRO.C

#####################################
# Use OTHER_MICROCODE_FILES in      #
# order to define other versions    #
# of the microcode that will be     #
# used in GET and LABEL operations, #
# but will not be compiled or       #
# linked.                           #
#####################################

################################
# Brazos video microcode cases #
################################
!if "$(VIDEO_MICROCODE)" == "UCODE_BRAZOS"
!if "$(VIDEO_UCODE_TYPE)" != "VMC_MACROBLOCK_ERROR_RECOVERY" && "$(VIDEO_UCODE_TYPE)" != "VMC_MACRO_BRAZOS_NTSC_PAL"
!message WARNING: Unsupported video microcode type specified for Brazos. Using default instead.
!endif
VIDEO_MICROCODE_FILES = BRAZOS_MACRO_VIDEO_UCODE.C
OTHER_MICROCODE_FILES = 

!if "$(VIDEO_UCODE_TYPE)" == "VMC_MACRO_BRAZOS_NTSC_PAL"
VIDEO_MICROCODE_FILES = VMC_BRAZOS_NTSCPAL.C
!endif

!endif

###############################
# Hondo video microcode cases #
###############################
!if "$(VIDEO_MICROCODE)" == "UCODE_HONDO"
!if "$(VIDEO_UCODE_TYPE)" != "VMC_PICTURE_ERROR_RECOVERY"
!message WARNING: Unsupported video microcode type specified for Hondo. Using default instead.
!endif
VIDEO_MICROCODE_FILES = HOND_VIDEO_UCODE.C
OTHER_MICROCODE_FILES = 
!endif

##########################
# Wabash video microcode #
##########################
!if "$(VIDEO_MICROCODE)" == "UCODE_WABASH"
!if "$(VIDEO_UCODE_TYPE)" == "VMC_EXTRA_ERROR_RECOVERY"
!message WARNING: Canal+ error recovery video microcode not available for Wabash
VIDEO_MICROCODE_FILES = WABASH_MACRO_VIDEO_UCODE.C
OTHER_MICROCODE_FILES = WABASH_VIDEO_UCODE.C
!else
!if "$(VIDEO_UCODE_TYPE)" == "VMC_PICTURE_ERROR_RECOVERY"
# Microcode with picture-based error recovery algorithm
VIDEO_MICROCODE_FILES = WABASH_VIDEO_UCODE.C
OTHER_MICROCODE_FILES = WABASH_MACRO_VIDEO_UCODE.C
!else
# Default case is to use the macroblock-based error recovery microcode 
VIDEO_MICROCODE_FILES = WABASH_MACRO_VIDEO_UCODE.C 
OTHER_MICROCODE_FILES = WABASH_VIDEO_UCODE.C
!endif
!endif
!endif # Wabash #

############################
# Colorado video microcode #
############################
!if "$(VIDEO_MICROCODE)" == "UCODE_COLORADO"

!if "$(VIDEO_UCODE_TYPE)" == "VMC_EXTRA_ERROR_RECOVERY"
# Special Version of Error Recovery microcode for Colorado
VIDEO_MICROCODE_FILES = COL_VIDEO_UCODE_ERR.C
OTHER_MICROCODE_FILES = COL_PICT_VIDEO_UCODE.C COL_MACRO_VIDEO_UCODE.C
!else
!if "$(VIDEO_UCODE_TYPE)" == "VMC_PICTURE_ERROR_RECOVERY"
# Microcode with picture-based error recovery algorithm
!message WARNING: Picture-based error recovery video microcode for Colorado is downlevel!
VIDEO_MICROCODE_FILES = COL_PICT_VIDEO_UCODE.C
OTHER_MICROCODE_FILES = COL_VIDEO_UCODE_ERR.C COL_MACRO_VIDEO_UCODE.C
!else
# Default case is to use the macroblock-based error recovery microcode 
VIDEO_MICROCODE_FILES = COL_MACRO_VIDEO_UCODE.C
OTHER_MICROCODE_FILES = COL_PICT_VIDEO_UCODE.C COL_VIDEO_UCODE_ERR.C
!endif
!endif
  
!endif # COLORADO

GENERAL_C_FILES = $(GENERAL_C_FILES) $(VIDEO_MICROCODE_FILES)

OTHER_FILES = $(OTHER_MICROCODE_FILES)

