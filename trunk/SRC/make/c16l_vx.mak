###############################################################################
#                                                                             #
# Sabine Build Support File: C16L_VX.MAK                                      #
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
!if   "$(ARM_TOOLKIT)" != "VXW"             # . ARM_TOOLKIT
!message ARM_TOOLKIT must be defined as "VXW"
!error ARM_TOOLKIT not properly defined!
!endif                                      # . ARM_TOOLKIT

CC     = ccwrap -wrapE=$(BIN_DIR)\ccarm.exe 
CPP    = ccwrap -wrapE=$(BIN_DIR)\ccarm.exe 

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


#########################################
# Change Compiler Flags based on Target #
#########################################
!if "$(DEBUG)" == "YES"
CDEBUG = -g -DDEBUG
DEBUGDIR = DEBUG
!else
CDEBUG = -O3
DEBUGDIR = RELEASE
!endif

###########################################################
# Turn selected Compiler Warnings off
# fc: Enable "limited pcc. Turns off warnings about cast of 
#     integers as function pointers.
# wd: Supresses warning about "deprecated declaration"
###########################################################
# MFB:  What are the GNU equivalents of these?
# CWARN_OFF = fc 

# MFB:  thumb interworking flag?  fvolatile?  signed characters?   word align objects?  packed structures?

CFLAGS= -mcpu=arm9 \
        -mapcs-32 \
        -nostdinc \
        -mno-sched-prolog \
!if "$(ARM_VERSION)" != "220"		
		-nostdinc \
        -Wimplicit \
!endif
        -Wall \
        -Wcomment \
        -fdollars-in-identifiers \
        -c

CDEFS  = -DBSP_LITTLE_ENDIAN=1 -DCONFIG=$(CONFIG) -DCUSTOMER=$(CUSTOMER) -DRVERSION=$(VERSION) -DRTOS=$(RTOS) -D$(BSP) -DCPU=$(CPU) -DARM_VERSION=$(ARM_VERSION)

COPTS  = $(CDEFS) $(EXTRA_ARM_C_FLAGS) -I$(INC_FIX:;= -I) $(CFLAGS) $(CDEBUG)

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
 -@echo Compiling $(*B).c
 -@if exist $@ del $@ > nul:
 -@echo ....$(*B).c >> $(COMBINED_ERROR_LOG)
 -@$(CC) $(COPTS) -wrapQUIET -wrapF=$(CONFIG).opt -wrapF=$(SABINE_ROOT)\$(APPNAME).CFL $(*B).c -o $@ -wrapL=$*.log
 -@type $*.log >> $(COMBINED_ERROR_LOG)
 @type $*.log

.cpp{$(SABINE_ROOT)\$(DRIVER)\$(CONFIG)\$(APPNAME)\$(RTOS)\$(SWCONFIG)\$(DEBUGDIR)\}.obj:
 @echo Compiling $(*B).cpp
 -@if exist $@ del $@ > nul:
 -@$(CPP) $(COPTS) -wrapQUIET -wrapF=$(CONFIG).opt -wrapF=$(SABINE_ROOT)\$(APPNAME).CFL $(*B).cpp -o $@ -wrapL=$*.log
 -@type $*.log  >> $(COMBINED_ERROR_LOG) 
 @type $*.log

!else

.c{$(SABINE_ROOT)\$(DRIVER)\$(CONFIG)\$(APPNAME)\$(RTOS)\$(SWCONFIG)\$(DEBUGDIR)\}.obj:
 -@echo Compiling $(*B).c
 -@if exist $@ del $@ > nul:
 @$(CC) $(COPTS) -wrapQUIET -wrapF=$(CONFIG).opt -wrapF=$(SABINE_ROOT)\$(APPNAME).CFL $(*B).c -o $@ 

.cpp{$(SABINE_ROOT)\$(DRIVER)\$(CONFIG)\$(APPNAME)\$(RTOS)\$(SWCONFIG)\$(DEBUGDIR)\}.obj:
 -@echo Compiling $(*B).c
 -@if exist $@ del $@ > nul:
 @$(CPP) $(COPTS) -wrapQUIET -wrapF=$(CONFIG).opt -wrapF=$(SABINE_ROOT)\$(APPNAME).CFL $(*B).cpp -o $@ 

!endif

!else

###################################
# Verbose build - echo everything #
###################################


!if "$(LOG)" == "YES"

.c{$(SABINE_ROOT)\$(DRIVER)\$(CONFIG)\$(APPNAME)\$(RTOS)\$(SWCONFIG)\$(DEBUGDIR)\}.obj:
 -@echo Compiling $(*B).c
 -@echo ....$(*B).c >> $(COMBINED_ERROR_LOG)
 -@if exist $@ del $@ > nul:
 -@$(CC) $(COPTS) -v -wrapF=$(CONFIG).opt -wrapF=$(SABINE_ROOT)\$(APPNAME).CFL $(*B).c -o $@ -wrapL=$*.log
 -@type $*.log >> $(COMBINED_ERROR_LOG)
 @type $*.log

.cpp{$(SABINE_ROOT)\$(DRIVER)\$(CONFIG)\$(APPNAME)\$(RTOS)\$(SWCONFIG)\$(DEBUGDIR)\}.obj:
 -@echo Compiling $(*B).c
 -@echo ....$(*B).c >> $(COMBINED_ERROR_LOG)
 -@if exist $@ del $@ > nul:
 -@$(CPP) $(COPTS) -v -wrapF=$(CONFIG).opt -wrapF=$(SABINE_ROOT)\$(APPNAME).CFL $(*B).cpp -o $@ -wrapL=$*.log
 -@type $*.log >> $(COMBINED_ERROR_LOG)
 @type $*.log

!else

.c{$(SABINE_ROOT)\$(DRIVER)\$(CONFIG)\$(APPNAME)\$(RTOS)\$(SWCONFIG)\$(DEBUGDIR)\}.obj:
 -@echo Compiling $(*B).c
 -@if exist $@ del $@ > nul:
 -@$(CC) $(COPTS) -v -wrapF=$(CONFIG).opt -wrapF=$(SABINE_ROOT)\$(APPNAME).CFL $(*B).c -o $@

.cpp{$(SABINE_ROOT)\$(DRIVER)\$(CONFIG)\$(APPNAME)\$(RTOS)\$(SWCONFIG)\$(DEBUGDIR)\}.obj:
 -@echo Compiling $(*B).cpp
 -@if exist $@ del $@ > nul:
 -@$(CPP) $(COPTS) -v -wrapF=$(CONFIG).opt -wrapF=$(SABINE_ROOT)\$(APPNAME).CFL $(*B).cpp -o $@

!endif
!endif

