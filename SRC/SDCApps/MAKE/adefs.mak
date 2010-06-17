###############################################################################
#                                                                             #
# CNXT MPEG build support file: adefs.mak                                     #
#                                                                             #
# Author: M. Bintz 7/24/03                                                    #
#                                                                             #
# Compiler rules for all toolchains and file types.                           #
#                                                                             #
###############################################################################
# there is a fundamental incompatibility between the ARM assembler and the
# GNU assembler.  The GNU assembler, as we use it, uses the C preprocessor
# which can not do string comparisons.  The ARM assembler CAN.  However,
# the ARM assembler can not be passed "def1 SETA def2" it MUST be passed
# "def1 SETA  [number]".  The following derives the number for TOOLKIT as
# defined in the swconfig.cfg file.  That is: THE FOLLOWING VALUES MUST
# MATCH THE SWCONFIG.CFG values.
#
# The end effect is that both assemblerd will eventually resolve the
# ARM_TOOLKIT definition down to a number.
# 

!if "$(ARM_TOOLKIT)"=="SDT" 
ARM_TOOLKIT_VALUE = 1
!endif

!if "$(ARM_TOOLKIT)"=="ADS"
ARM_TOOLKIT_VALUE = 2
!endif

!if "$(ARM_TOOLKIT)"=="WRGCC"
ARM_TOOLKIT_VALUE = 3
!endif

!if "$(ARM_TOOLKIT)"=="GNUGCC"
ARM_TOOLKIT_VALUE = 4
!endif

!if "$(ARM_TOOLKIT)"=="SDT" || "$(ARM_TOOLKIT)"=="ADS"


ADEFS  = -PD "BSP_LITTLE_ENDIAN SETL {TRUE}"     \
  -PD "ARM_TOOLKIT SETA $(ARM_TOOLKIT_VALUE)"   \
  -PD "ARM_VERSION SETA $(ARM_VERSION)"   \
  -PD "THUMB SETL {FALSE}"                \
  -PD "RTOS SETS \"$(RTOS)\""             \
  -PD "NON_VXWORKS SETL {TRUE}"           \
  -PD "CONFG SETS \"$(CONFIG)\""          \
  -PD "RVERSION SETS \"$(VERSION)\""      \
  -PD "BSP SETS \"$(BSP)\""               \
  -PD "CUSTOMER SETS \"$(CUSTOMER)\""     \
  -PD "IMAGE_TYPE SETS \"$(IMAGE_TYPE)\"" \
  $(EXTRA_ARM_ASM_FLAGS) $(EXTRA_THUMB_ASM_FLAGS)

!if "$(DEBUG)" == "YES"
ADEFS   = $(ADEFS) -PD "DEBUG SETL {TRUE}"
!else
ADEFS   = $(ADEFS) -PD "DEBUG SETL {FALSE}"
!endif

!if "$(SWCONFIG)" != "CNXT"
ADEFS = $(ADEFS) -PD "SWCONFIG_INCL SETA 1"
!else
ADEFS = $(ADEFS) -PD "SWCONFIG_INCL SETA 0"
!endif


!elseif "$(ARM_TOOLKIT)"=="WRGCC"
ADEFS = $(CDEFS)


!elseif "$(ARM_TOOLKIT)"=="GNUGCC"
ADEFS = $(CDEFS)

!elseif
!error "Don't know how to make assembler definitions for $(ARM_TOOLKIT)"
!endif







#****************************************************************************
#* $Log:
#*  1    mpeg      1.0         7/5/2004 7:05:05 PM    Ford Fu         CR(s)
#*       9660 9659 : SDC APPLICATION check in, make files 
#* $
#*  
#****************************************************************************


