###############################################################################
#                                                                             #
# CNXT MPEG build support file: cdefs.mak                                     #
#                                                                             #
# Author: M. Bintz 7/24/03                                                    #
#                                                                             #
# Compiler rules for all toolchains and file types.                           #
#                                                                             #
###############################################################################


########################  C #define's  ############################
CDEFS  = \
 -DBSP_LITTLE_ENDIAN=1 \
 -DCONFIG=$(CONFIG) \
 -DCUSTOMER=$(CUSTOMER) \
 -DRVERSION=$(VERSION) \
 -DARM_TOOLKIT=$(ARM_TOOLKIT) \
 -DARM_VERSION=$(ARM_VERSION) \
 -DRTOS=$(RTOS) \
 -DEMULATION_LEVEL=$(EMULATION_LEVEL) \
 -DCHIP_NAME=$(CHIP_NAME) 	\
 -DIMAGE_TYPE=$(IMAGE_TYPE) \
 -DCPU=$(CPU) \
 $(REVISION_FLAGS) \
 $(VIDEO_FLAGS) \
 $(DEVICE_FLAGS) \
 $(AUDIO_FLAGS) \
 $(BUFFER_FLAGS) \
 $(BOARD_FLAGS) \
 $(ROM_FLAGS) \
 $(FILE_FLAGS) \
 $(UART_FLAGS) \
 $(OPENTV_FLAGS) \
 -DIMAGE_TYPE=$(IMAGE_TYPE) \
 $(EXTRA_ARM_C_FLAGS) \
 $(EXTRA_THUMB_C_FLAGS)

CDEFS = $(CDEFS:      = )
CDEFS = $(CDEFS:     = )
CDEFS = $(CDEFS:    = )
CDEFS = $(CDEFS:   = )
CDEFS = $(CDEFS:  = )
 

!if "$(SWCONFIG)" != "CNXT"
CDEFS = $(CDEFS) -DSWCONFIG_INCL=1
!else
CDEFS = $(CDEFS) -DSWCONFIG_INCL=0
!endif

!if "$(BSP)" != ""
CDEFS  = $(CDEFS) -D$(BSP)
!endif

!if "$(DEBUG)" == "YES"
CDEFS  = $(CDEFS) -DDEBUG
!else
CDEFS  = $(CDEFS) -UDEBUG
!endif

!if "$(TRACE_OUTPUT)" == "YES"
CDEFS  = $(CDEFS) -DTRACE_OUTPUT
!else
CDEFS  = $(CDEFS) -UTRACE_OUTPUT
!endif

















#****************************************************************************
#* $Log: 
#*  2    mpeg      1.1         4/27/04 3:37:37 PM     Miles Bintz     CR(s) 
#*        8979 8980 : switch on TRACE_OUTPUT define instead of debug
#*  1    mpeg      1.0         9/9/03 4:40:36 PM      Miles Bintz     
#* $
#  
#     Rev 1.0   09 Sep 2003 15:40:36   bintzmf
#  SCR(s) 7291 :
#  reworked build system
#  
#     Rev 1.1   28 Aug 2003 15:25:30   bintzmf
#  
#     Rev 1.0   14 Aug 2003 13:41:46   bintzmf
#  new build system
#*  
#*  
#****************************************************************************


