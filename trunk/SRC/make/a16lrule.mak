###############################################################################
#                                                                             #
# Sabine Build Support File: C16LRULE.MAK                                     #
#                                                                             #
# Author: D. Wilson, 3/9/97                                                   #
#                                                                             #
# Compiler rules for building 16 bit ARM modules                              #
#                                                                             #
# This file is used as the basis for driver-specific makefiles which are      #
# constructed as part of the build process                                    #
#                                                                             #
###############################################################################
!include $(SABINE_ROOT)\make\product.mak

#########################################
# CHECK for Valid ARM ENVIRONMENT Setup #
#########################################
!if   "$(ARM_TOOLKIT)" == "ADS"             # . ARM_TOOLKIT
!if   "$(ARM_VERSION)" == "11"              # .. ARM_VERSION
!elseif "$(ARM_VERSION)" == "12"            # .. ARM_VERSION
!else                                       # .. ARM_VERSION
!message ARM_VERSION must be defined as "11" or "12"
!message Refer to App Note in Tracker #3456 for more information
!error ARM_VERSION not properly defined for ARM_TOOLKIT = ADS
!endif                                      # .. ARM_VERSION
!elseif "$(ARM_TOOLKIT)" == "SDT"           # . ARM_TOOLKIT
!if   "$(ARM_VERSION)" == "250"             # .. ARM_VERSION
!elseif "$(ARM_VERSION)" == "251"           # .. ARM_VERSION
!else                                       # .. ARM_VERSION
!message ARM_VERSION must be defined as "250" or "251"
!message Refer to App Note in Tracker #3456 for more information
!error ARM_VERSION not properly defined for ARM_TOOLKIT = SDT
!endif                                      # .. ARM_VERSION
!else                                       # . ARM_TOOLKIT
!message ARM_TOOLKIT must be defined as "ADS" or "SDT"
!message Refer to App Note in Tracker #3456 for more information
!error ARM_TOOLKIT not properly defined!
!endif                                      # . ARM_TOOLKIT


ASM    = armasm


##################################
# Fix the INCLUDE path to remove #
# Leading/Trailing ';', and		 #
# any multiple ';'				 #
##################################
FOO=FOO
BAR=BAR
INC_FIX=$(FOO);$(INCLUDE);$(BAR)
INC_FIX=$(INC_FIX:;;;;;=;;;;)
INC_FIX=$(INC_FIX:;;;;=;;;)
INC_FIX=$(INC_FIX:;;;=;;)
INC_FIX=$(INC_FIX:;;=;)
INC_FIX=$(INC_FIX:FOO;=)
INC_FIX=$(INC_FIX:;BAR=)


##########################################
# Change Assembler Flags based on Target #
##########################################
!if "$(ARM_TOOLKIT)" == "ADS"               # . ARM_TOOLKIT
AFLAGS = -16 -I$(INC_FIX:;= -I) -apcs /noswst/inter -li -fpu softfpa -cpu ARM9TDMI
!elseif "$(ARM_TOOLKIT)" == "SDT"           # . ARM_TOOLKIT
!if "$(ARM_VERSION)" == "250"               # .. ARM_VERSION
AFLAGS = -16 -I$(INC_FIX:;= -I) -apcs 3/32/nofp/noswst -li -cpu ARM9TDMI
!elseif "$(ARM_VERSION)" == "251"           # .. ARM_VERSION
AFLAGS = -16 -I$(INC_FIX:;= -I) -apcs 3/32/nofp/noswst -li -cpu ARM9TM
!endif                                      # .. ARM_VERSION
!endif                                      # . ARM_TOOLKIT

AFLAGS = -i$(SABINE_ROOT)\include\$(CONFIG)\$(SWCONFIG) $(AFLAGS)

!if "$(DEBUG)" == "YES"
ADEBUG = -g -PD "DEBUG SETL {TRUE}"
DEBUGDIR = DEBUG
!else
ADEBUG = -PD "DEBUG SETL {FALSE}"
DEBUGDIR = RELEASE
!endif

ADEFS  = -PD "BSP_LITTLE_ENDIAN SETL {TRUE}" \
         -PD "THUMB SETL {TRUE}"             \
         -PD "RTOS SETS \"$(RTOS)\""             \
         -PD "CONFG SETS \"$(CONFIG)\""      \
         -PD "ARM_VERSION SETS \"$(ARM_VERSION)\""    \
         -PD "RVERSION SETS \"$(VERSION)\""  \
         -PD "BSP SETS \"$(BSP)\""           \
         -PD "CUSTOMER SETS \"$(CUSTOMER)\""

ASM_CONFIG_FLAGS=$(ASM_CONFIG_FLAGS) -PD "IMAGE_TYPE SETS \"$(IMAGE_TYPE)\""
AOPTS  = $(AFLAGS) $(EXTRA_THUMB_ASM_FLAGS) $(ADEBUG) $(ADEFS) $(ASM_CONFIG_FLAGS)

!if "$(SWCONFIG)" != "CNXT"
AOPTS = $(AOPTS) -PD "SWCONFIG_INCL SETA 1"
!else
AOPTS = $(AOPTS) -PD "SWCONFIG_INCL SETA 0"
!endif

.SUFFIXES : .s

!if "$(VERBOSE)" == "YES"

############################################################
# With Verbose on, echo command lines as they are executed #
############################################################

!if "$(LOG)" == "YES"

.s{$(SABINE_ROOT)\$(DRIVER)\$(CONFIG)\$(APPNAME)\$(RTOS)\$(SWCONFIG)\$(DEBUGDIR)\}.obj:
 -@echo Assembling $(*B).s
 -@echo ....$(*B).s >> $(COMBINED_ERROR_LOG)
 -@$(ASM) $(AOPTS) -errors $*.log $(MAKEDIR)\$(*B).s $@
 -@type $*.log >> $(COMBINED_ERROR_LOG) 

!else

.s{$(SABINE_ROOT)\$(DRIVER)\$(CONFIG)\$(APPNAME)\$(RTOS)\$(SWCONFIG)\$(DEBUGDIR)\}.obj:
 -@echo Assembling $(*B).s
 -@$(ASM) $(AOPTS) $(MAKEDIR)\$(*B).s $@

!endif

!else

##############################################
# With Verbose off, don't echo command lines #
##############################################

!if "$(LOG)" == "YES"

.s{$(SABINE_ROOT)\$(DRIVER)\$(CONFIG)\$(APPNAME)\$(RTOS)\$(SWCONFIG)\$(DEBUGDIR)\}.obj:
 -@echo Assembling $(*B).s
 -@echo ....$(*B).s >> $(COMBINED_ERROR_LOG)
 -@$(ASM) $(AOPTS) -errors $*.log $(MAKEDIR)\$(*B).s $@
 -@type $*.log >> $(COMBINED_ERROR_LOG) 

!else

.s{$(SABINE_ROOT)\$(DRIVER)\$(CONFIG)\$(APPNAME)\$(RTOS)\$(SWCONFIG)\$(DEBUGDIR)\}.obj:
 -@echo Assembling $(*B).s
 -@$(ASM) $(AOPTS) $(MAKEDIR)\$(*B).s $@

!endif
!endif
