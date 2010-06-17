###############################################################################
#                                                                             #
# Sabine Build Support File: A32LRULE.MAK                                     #
#                                                                             #
# Author: D. Wilson, 3/9/97                                                   #
#         M. Bintz,  3/9/2000                                                 #
#                                                                             #
# Compiler rules for building 32 bit ARM modules with GNU compiler            #
#                                                                             #
# This file is used as the basis for driver-specific makefiles which are      #
# constructed as part of the build process                                    #
#                                                                             #
###############################################################################

!include $(SABINE_ROOT)\make\product.mak

#########################################
# CHECK for Valid ARM ENVIRONMENT Setup #
#########################################
!if   "$(ARM_TOOLKIT)" != "VXW"             # . ARM_TOOLKIT
!message ARM_TOOLKIT must be defined as "VXW"
!error ARM_TOOLKIT not properly defined!
!endif                                      # . ARM_TOOLKIT

ASM    = ccwrap -wrapE=$(BIN_DIR)\ccarm.exe 
GASP   = ccwrap -wrapE=$(BIN_DIR)\gasparm.exe

OBJ_TYPE = 32

##################################
# Fix the INCLUDE path to remove #
# Leading/Trailing ';', and      #
# any multiple ';'               #
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
AFLAGS = -mapcs-32 -I$(INC_FIX:;= -I) -mno-fpu -x assembler-with-cpp -c

!if "$(DEBUG)" == "YES"
ADEBUG =  -g
!else
ADEBUG =
!endif

ADEFS  = -DBSP_LITTLE_ENDIAN=1    \
         -DTHUMB=0                \
         -DCPU=$(CPU)             \
         -DCONFG=$(CONFIG)        \
         -DRVERSION=$(VERSION)    \
         -DBSP=$(BSP)             \
		 -DIMAGE_TYPE=$(IMAGE_TYPE) \
         -DARM_TOOLKIT=$(ARM_TOOLKIT) \
		 -DARM_VERSION=$(ARM_VERSION) \
         -DCUSTOMER=$(CUSTOMER)

ADEFS  = $(ADEFS) -DOS=$(RTOS)

!if "$(DEBUG)" == "YES"
ADEBUG = -g -DDEBUG=1 
DEBUGDIR = DEBUG
!else
ADEBUG = -DDEBUG=0 -fno-strict-aliasing
DEBUGDIR = RELEASE
!endif

!if "$(GNU_ADEBUG_OVR)" != ""
ADEBUG = $(GNU_ADEBUG_OVR)
!endif

ADEBUG = $(ADEBUG) $(GNU_ADEBUG_APP)

# ASM_CONFIG_FLAGS=$(ASM_CONFIG_FLAGS) -PD "IMAGE_TYPE SETS \"$(IMAGE_TYPE)\""
AOPTS  = $(AFLAGS) $(EXTRA_ARM_ASM_FLAGS) $(ADEBUG) $(ADEFS) 

!if "$(SWCONFIG)" != "CNXT"
AOPTS = $(AOPTS) -DSWCONFIG_INCL=1
!else
AOPTS = $(AOPTS) -DSWCONFIG_INCL=0
!endif

.SUFFIXES : .s .gasp

!if "$(VERBOSE)" == "YES"

############################################################
# With Verbose on, echo command lines as they are executed #
############################################################

!if "$(LOG)" == "YES"

.s{$(SABINE_ROOT)\$(DRIVER)\$(CONFIG)\$(APPNAME)\$(RTOS)\$(SWCONFIG)\$(DEBUGDIR)\}.obj:
 -@echo Assembling $(*B).s
 -@echo ....$(*B).s >> $(COMBINED_ERROR_LOG)
!if "$(DRIVER)" == "$(BSP)"
 -@$(ASM) -v $(AOPTS) $(MAKEDIR)\$(*B).s -o $@ -wrapL=$*.log
!else
 -@AsmArm2Gnu $(MAKEDIR)\$(*B).s $(MAKEDIR)\VX$(*B).s
 -@$(ASM) -v $(AOPTS) $(MAKEDIR)\VX$(*B).s -o $@ -wrapL=$*.log
!endif
 -@type $*.log >> $(COMBINED_ERROR_LOG) 

.gasp{$(SABINE_ROOT)\$(DRIVER)\$(CONFIG)\$(APPNAME)\$(RTOS)\$(SWCONFIG)\$(DEBUGDIR)\}.obj:
 -@echo Preprocessing $(*B).gasp...
 -@echo ....$(*B).gasp >> $(COMBINED_ERROR_LOG)
 -@$(GASP) -wrapL=$*.log -u -o $(*B).S $(*B).gasp
 -@echo @del $(*B).S >> $(SABINE_ROOT)\$(DRIVER)\clean_gasp_files.bat 
 -@type $*.log >> $(COMBINED_ERROR_LOG) 
 -@echo Assembling $(*B).s
 -@echo ....$(*B).s >> $(COMBINED_ERROR_LOG)
 -@$(ASM) -v $(AOPTS) $(MAKEDIR)\$(*B).s -o $@ -wrapL=$*.log
 -@type $*.log >> $(COMBINED_ERROR_LOG) 

!else

.s{$(SABINE_ROOT)\$(DRIVER)\$(CONFIG)\$(APPNAME)\$(RTOS)\$(SWCONFIG)\$(DEBUGDIR)\}.obj:
 -@echo Assembling $(*B).s
!if "$(DRIVER)" == "$(BSP)"
 -$(ASM) -v $(AOPTS) $(MAKEDIR)\$(*B).s -o $@
!else
 -@AsmArm2Gnu $(MAKEDIR)\$(*B).s $(MAKEDIR)\VX$(*B).s
 -$(ASM) -v $(AOPTS) $(MAKEDIR)\VX$(*B).s -o $@
!endif

.gasp{$(SABINE_ROOT)\$(DRIVER)\$(CONFIG)\$(APPNAME)\$(RTOS)\$(SWCONFIG)\$(DEBUGDIR)\}.obj:
 -@echo Preprocessing $(*B).gasp...
 -$(GASP) -u -o $(*B).S $(*B).gasp  
 -@echo @del $(*B).S >> $(SABINE_ROOT)\$(DRIVER)\clean_gasp_files.bat 
 -@echo Assembling $(*B).s
 -$(ASM) -v $(AOPTS) $(MAKEDIR)\$(*B).s -o $@ 

!endif

!else

##############################################
# With Verbose off, don't echo command lines #
##############################################

!if "$(LOG)" == "YES"

.s{$(SABINE_ROOT)\$(DRIVER)\$(CONFIG)\$(APPNAME)\$(RTOS)\$(SWCONFIG)\$(DEBUGDIR)\}.obj:
 -@echo Assembling $(*B).s
 -@echo ....$(*B).s >> $(COMBINED_ERROR_LOG)
!if "$(DRIVER)" == "$(BSP)"
 -@$(ASM) $(AOPTS) $(MAKEDIR)\$(*B).s -o $@ -wrapQUIET -wrapL=$*.log
!else
 -@AsmArm2Gnu $(MAKEDIR)\$(*B).s $(MAKEDIR)\VX$(*B).s
 -@$(ASM) $(AOPTS) $(MAKEDIR)\VX$(*B).s -o $@ -wrapQUIET -wrapL=$*.log
!endif
 -@type $*.log >> $(COMBINED_ERROR_LOG) 

.gasp{$(SABINE_ROOT)\$(DRIVER)\$(CONFIG)\$(APPNAME)\$(RTOS)\$(SWCONFIG)\$(DEBUGDIR)\}.obj:
 -@echo Preprocessing $(*B).gasp...
 -@echo ....$(*B).gasp >> $(COMBINED_ERROR_LOG)
 -@$(GASP) -wrapL=$*.log -wrapQUIET -u -o $(*B).S $(*B).gasp 
 -@echo @del $(*B).S >> $(SABINE_ROOT)\$(DRIVER)\clean_gasp_files.bat 
 -@type $*.log >> $(COMBINED_ERROR_LOG) 
 -@echo Assembling $(*B).s
 -@echo ....$(*B).s >> $(COMBINED_ERROR_LOG)
 -@$(ASM) $(AOPTS) $(MAKEDIR)\$(*B).s -o $@ -wrapL=$*.log -wrapQUIET
 -@type $*.log >> $(COMBINED_ERROR_LOG) 

!else

.s{$(SABINE_ROOT)\$(DRIVER)\$(CONFIG)\$(APPNAME)\$(RTOS)\$(SWCONFIG)\$(DEBUGDIR)\}.obj:
 -@echo Assembling $(*B).s
!if "$(DRIVER)" == "$(BSP)"
 -@$(ASM) $(AOPTS) $(MAKEDIR)\$(*B).s -o $@
!else
 -@AsmArm2Gnu $(MAKEDIR)\$(*B).s $(MAKEDIR)\VX$(*B).s
 -@$(ASM) $(AOPTS) $(MAKEDIR)\VX$(*B).s -o $@
!endif

.gasp{$(SABINE_ROOT)\$(DRIVER)\$(CONFIG)\$(APPNAME)\$(RTOS)\$(SWCONFIG)\$(DEBUGDIR)\}.obj:
 -@echo Preprocessing $(*B).gasp...
 -$(GASP) -u -o $(*B).S $(*B).gasp  
 -@echo @del $(*B).S >> $(SABINE_ROOT)\$(DRIVER)\clean_gasp_files.bat 
 -@echo Assembling $(*B).s
 -$(ASM) $(AOPTS) $(MAKEDIR)\$(*B).s -o $@ 

!endif

!endif

