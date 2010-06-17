###############################################################################
#                                                                             #
# Sabine Build Support File: C32LRULE.MAK                                     #
#                                                                             #
# Author: D. Wilson, 3/9/97                                                   #
#                                                                             #
# Compiler rules for building 32 bit ARM modules                              #
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


CC     = armcc
CPP    = armcpp

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


#########################################
# Change Compiler Flags based on Target #
#########################################
!if "$(ARM_TOOLKIT)" == "ADS"           # CPU Setting for ARM_TOOLKIT = ADS
CPU = -cpu ARM9TDMI
!elseif "$(ARM_TOOLKIT)" == "SDT"       # CPU Setting for ARM_TOOLKIT = SDT
CPU = -processor ARM9TDMI
!endif                                  # ARM_TOOKIT


!if "$(DEBUG)" == "YES"                 # DEBUG Options for DEBUG = YES
DEBUGDIR = DEBUG
CDEBUG = -g+ -DDEBUG
!else                                   # DEBUG Options for DEBUG != YES (assumed NO)
DEBUGDIR = RELEASE
!if "$(ARM_TOOLKIT)" == "ADS"           # DEBUG Options for DEBUG != YES, ARM_TOOLKIT = ADS
CDEBUG = -Ospace -g- -O1 -UDEBUG -Ono_data_reorder
!elseif "$(ARM_TOOLKIT)" == "SDT"       # DEBUG Options for DEBUG != YES, ARM_TOOLKIT = SDT
!if "$(ARM_VERSION)" == "250"           # DEBUG Options for DEBUG != YES, ARM_TOOLKIT = SDT, ARM_VERSION = 250
CDEBUG = -Ospace -g- -o2 -UDEBUG
!elseif "$(ARM_VERSION)" == "251"       # DEBUG Options for DEBUG != YES, ARM_TOOLKIT = SDT, ARM_VERSION = 251
CDEBUG = -Ospace -g- -gxo -UDEBUG
!endif                                  # ARM_VERSION
!endif                                  # ARM_TOOLKIT
!endif                                  # DEBUG


###########################################################
# Turn selected Compiler Warnings off
# fc: Enable "limited pcc. Turns off warnings about cast of 
#     integers as function pointers.
# wd: Supresses warning about "deprecated declaration"
###########################################################
!if "$(ARM_TOOLKIT)" == "ADS"           # Command Line Options for ARM_TOOLKIT = "ADS"
CWARN_OFF = Wd
!if "$(PACKING)" != "1"
CFLAGS = $(CPU) -zas$(PACKING) -zo -zc -fy -c -I$(INC_FIX:;= -I) -apcs /noswst/inter -fpu softfpa -li -$(CWARN_OFF)
!else
CFLAGS = $(CPU) -zo -zc -fy -c -I$(INC_FIX:;= -I) -apcs /noswst/inter -fpu softfpa -li -$(CWARN_OFF)
!endif
!elseif "$(ARM_TOOLKIT)" == "SDT"        # Command Line Options for ARM_TOOLKIT = "SDT"
CWARN_OFF = fc -Wd
CFLAGS = $(CPU) -za1 -zas$(PACKING) -zat4 -zap1 -zo -zc -fy -zz-1 -c -J$(INC_FIX:;= -J) -apcs 3/32/nofp/noswst/narrow/softfp/interwork -li -$(CWARN_OFF)
!endif                                  # ARM_TOOLKIT

CDEFS  = -DBSP_LITTLE_ENDIAN=1 -DCONFIG=$(CONFIG) -DCUSTOMER=$(CUSTOMER) -DRVERSION=$(VERSION) -DARM_VERSION=$(ARM_VERSION) -DRTOS=$(RTOS)

!if "$(BSP)" != ""
CDEFS  = $(CDEFS) -D$(BSP)
!endif

CDEFS = $(CDEFS) -DARM_TOOLKIT=$(ARM_TOOLKIT)
CDEFS = $(CDEFS) -DARM_VERSION=$(ARM_VERSION)

COPTS  = $(CFLAGS) $(CDEBUG) $(CDEFS) $(EXTRA_ARM_C_FLAGS) -via $(SABINE_ROOT)\$(APPNAME).CFL

!if "$(SWCONFIG)" != "CNXT"
COPTS = $(COPTS) -DSWCONFIG_INCL=1
!else
COPTS = $(COPTS) -DSWCONFIG_INCL=0
!endif

!if "$(VERBOSE)" == "NO"

############################################################
# Normal build - verbose is off so don't echo all commands #
############################################################

!if "$(LOG)" == "YES"

.c{$(SABINE_ROOT)\$(DRIVER)\$(CONFIG)\$(APPNAME)\$(RTOS)\$(SWCONFIG)\$(DEBUGDIR)\}.obj:
 -@echo ---- Compiling ARM mode $(*B).c
 -@echo ---- Compiling ARM mode $(*B).c >> $(COMBINED_ERROR_LOG)
 -@$(CC) $(COPTS) -o$@ -errors $*.log -via $(CONFIG).opt $(MAKEDIR)\$(*B).c
 -@type $*.log >> $(COMBINED_ERROR_LOG)
 @type $*.log

.cpp{$(SABINE_ROOT)\$(DRIVER)\$(CONFIG)\$(APPNAME)\$(RTOS)\$(SWCONFIG)\$(DEBUGDIR)\}.obj:
 @echo ---- Compiling ARM mode $(*B).cpp
 @$(CPP) $(COPTS) -o$@ -errors $*.log $(MAKEDIR)\$(*B).cpp
 -@type $*.log  >> $(COMBINED_ERROR_LOG) 
 @type $*.log

!else

.c{$(SABINE_ROOT)\$(DRIVER)\$(CONFIG)\$(APPNAME)\$(RTOS)\$(SWCONFIG)\$(DEBUGDIR)\}.obj:
 -@echo ---- Compiling ARM mode $(*B).c
 @$(CC) $(COPTS) -o$@ -via $(CONFIG).opt $(MAKEDIR)\$(*B).c

.cpp{$(SABINE_ROOT)\$(DRIVER)\$(CONFIG)\$(APPNAME)\$(RTOS)\$(SWCONFIG)\$(DEBUGDIR)\}.obj:
 -@echo ---- Compiling ARM mode $(*B).c
 @$(CPP) $(COPTS) -o$@ -via $(CONFIG).opt $(MAKEDIR)\$(*B).cpp

!endif

!else

###################################
# Verbose build - echo everything #
###################################


!if "$(LOG)" == "YES"

.c{$(SABINE_ROOT)\$(DRIVER)\$(CONFIG)\$(APPNAME)\$(RTOS)\$(SWCONFIG)\$(DEBUGDIR)\}.obj:
 -@echo ---- Compiling ARM mode $(*B).c
 -@echo ---- Compiling ARM mode $(*B).c >> $(COMBINED_ERROR_LOG)
 -@$(CC) $(COPTS) -o$@ -errors $*.log -via $(CONFIG).opt $(MAKEDIR)\$(*B).c
 -@type $*.log >> $(COMBINED_ERROR_LOG)
 @type $*.log

.cpp{$(SABINE_ROOT)\$(DRIVER)\$(CONFIG)\$(APPNAME)\$(RTOS)\$(SWCONFIG)\$(DEBUGDIR)\}.obj:
 -@echo ---- Compiling ARM mode $(*B).c
 -@echo ---- Compiling ARM mode $(*B).c >> $(COMBINED_ERROR_LOG)
 -@$(CPP) $(COPTS) -o$@ -errors $*.log $(MAKEDIR)\$(*B).cpp
 -@type $*.log >> $(COMBINED_ERROR_LOG)
 @type $*.log

!else

.c{$(SABINE_ROOT)\$(DRIVER)\$(CONFIG)\$(APPNAME)\$(RTOS)\$(SWCONFIG)\$(DEBUGDIR)\}.obj:
 -@echo ---- Compiling ARM mode $(*B).c
 -@$(CC) $(COPTS) -o$@ -via $(CONFIG).opt $(MAKEDIR)\$(*B).c

.cpp{$(SABINE_ROOT)\$(DRIVER)\$(CONFIG)\$(APPNAME)\$(RTOS)\$(SWCONFIG)\$(DEBUGDIR)\}.obj:
 -@echo ---- Compiling ARM mode $(*B).cpp
 -@$(CPP) $(COPTS) -o$@ -via $(CONFIG).opt $(MAKEDIR)\$(*B).cpp

!endif
!endif

