###############################################################################
#                                                                             #
# Sabine Build Support File: A16L_VX.MAK                                      #
#                                                                             #
# Author: M. Bintz 3/9/2000                                                   #
#                                                                             #
# Compiler rules for building 16 bit ARM modules                              #
#                                                                             #
# This file is used as the basis for driver-specific makefiles which are      #
# constructed as part of the build process                                    #
#                                                                             #
###############################################################################

!error Should Not Build VXW in THUMB Mode!

!include $(SABINE_ROOT)\make\product.mak

#########################################
# CHECK for Valid ARM ENVIRONMENT Setup #
#########################################
!if      "$(ARM_TOOLKIT)" != "VXW"           # . ARM_TOOLKIT
!message ARM_TOOLKIT must be defined as "VXW"
!error ARM_TOOLKIT not properly defined!
!endif                                      # . ARM_TOOLKIT

ASM    = $(BIN_DIR)\ccarm.exe 
GASP   = $(BIN_DIR)\gasparm.exe

OBJ_TYPE = 16

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
         -DOS=$(RTOS)             \
         -DCPU=$(CPU)             \
         -DCONFG=$(CONFIG)        \
         -DRVERSION=$(VERSION)    \
         -DBSP=$(BSP)             \
		 -DIMAGE_TYPE=$(IMAGE_TYPE) \
		 -DARM_TOOLKIT=$(ARM_TOOLKIT) \
		 -DARM_VERSION=$(ARM_VERSION) \
         -DCUSTOMER=$(CUSTOMER)

!if "$(DEBUG)" == "YES"
ADEBUG = -g -DDEBUG=1 
DEBUGDIR = DEBUG
!else
ADEBUG = -DDEBUG=0
DEBUGDIR = RELEASE
!endif

#ASM_CONFIG_FLAGS=$(ASM_CONFIG_FLAGS) -PD "IMAGE_TYPE SETS \"$(IMAGE_TYPE)\""
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
 -$(ASM) -v $(AOPTS) $(*B).s -o $@ > $*.log
 -@type $*.log >> $(COMBINED_ERROR_LOG) 

.gasp{$(SABINE_ROOT)\$(DRIVER)\$(CONFIG)\$(APPNAME)\$(RTOS)\$(SWCONFIG)\$(DEBUGDIR)\}.obj:
 -@echo Preprocessing $(*B).gasp...
 -@echo ....$(*B).gasp >> $(COMBINED_ERROR_LOG)
 -$(GASP) -u -o $(*B).S $(*B).gasp > $*.log 
 -@echo @del $(*B).S >> $(SABINE_ROOT)\$(DRIVER)\clean_gasp_files.bat 
 -@type $*.log >> $(COMBINED_ERROR_LOG) 
 -@echo Assembling $(*B).s
 -@echo ....$(*B).s >> $(COMBINED_ERROR_LOG)
 -$(ASM) -v $(AOPTS) $(*B).s -o $@ > $*.log
 -@type $*.log >> $(COMBINED_ERROR_LOG) 

!else

.s{$(SABINE_ROOT)\$(DRIVER)\$(CONFIG)\$(APPNAME)\$(RTOS)\$(SWCONFIG)\$(DEBUGDIR)\}.obj:
 -@echo Assembling $(*B).s
 -$(ASM) -v $(AOPTS) $(*B).s -o $@

.gasp{$(SABINE_ROOT)\$(DRIVER)\$(CONFIG)\$(APPNAME)\$(RTOS)\$(SWCONFIG)\$(DEBUGDIR)\}.obj:
 -@echo Preprocessing $(*B).gasp...
 -$(GASP) -u -o $(*B).S $(*B).gasp  
 -@echo @del $(*B).S >> $(SABINE_ROOT)\$(DRIVER)\clean_gasp_files.bat 
 -@echo Assembling $(*B).s
 -$(ASM) -v $(AOPTS) $(*B).s -o $@ 

!endif

!else

##############################################
# With Verbose off, don't echo command lines #
##############################################

!if "$(LOG)" == "YES"

.s{$(SABINE_ROOT)\$(DRIVER)\$(CONFIG)\$(APPNAME)\$(RTOS)\$(SWCONFIG)\$(DEBUGDIR)\}.obj:
 -@echo Assembling $(*B).s
 -@echo ....$(*B).s >> $(COMBINED_ERROR_LOG)
 -@$(ASM) $(AOPTS) $(*B).s -o $@ > $*.log
 -@type $*.log >> $(COMBINED_ERROR_LOG) 

.gasp{$(SABINE_ROOT)\$(DRIVER)\$(CONFIG)\$(APPNAME)\$(RTOS)\$(SWCONFIG)\$(DEBUGDIR)\}.obj:
 -@echo Preprocessing $(*B).gasp...
 -@echo ....$(*B).gasp >> $(COMBINED_ERROR_LOG)
 -$(GASP) -u -o $(*B).S $(*B).gasp > $*.log 
 -@echo @del $(*B).S >> $(SABINE_ROOT)\$(DRIVER)\clean_gasp_files.bat 
 -@type $*.log >> $(COMBINED_ERROR_LOG) 
 -@echo Assembling $(*B).s
 -@echo ....$(*B).s >> $(COMBINED_ERROR_LOG)
 -$(ASM) $(AOPTS) $(*B).s -o $@ > $*.log
 -@type $*.log >> $(COMBINED_ERROR_LOG) 

!else

.s{$(SABINE_ROOT)\$(DRIVER)\$(CONFIG)\$(APPNAME)\$(RTOS)\$(SWCONFIG)\$(DEBUGDIR)\}.obj:
 -@echo Assembling $(*B).s
 -@$(ASM) $(AOPTS) $(*B).s -o $@

.gasp{$(SABINE_ROOT)\$(DRIVER)\$(CONFIG)\$(APPNAME)\$(RTOS)\$(SWCONFIG)\$(DEBUGDIR)\}.obj:
 -@echo Preprocessing $(*B).gasp...
 -$(GASP) -u -o $(*B).S $(*B).gasp  
 -@echo @del $(*B).S >> $(SABINE_ROOT)\$(DRIVER)\clean_gasp_files.bat 
 -@echo Assembling $(*B).s
 -$(ASM) $(AOPTS) $(*B).s -o $@ 

!endif

!endif

