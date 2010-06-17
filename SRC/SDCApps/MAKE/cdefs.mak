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
 -DQCBUILD_HDI \
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
#*  1    mpeg      1.0         7/5/2004 7:06:39 PM    Ford Fu         CR(s)
#*       9660 9659 : SDC APPLICATION check in, make files 
#* $
#*  
#****************************************************************************


