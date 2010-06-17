###############################################################################
#                                                                             #
# Sabine Build Support File: C32LRULE.MAK                                     #
#                                                                             #
# Author: D. Wilson, 3/9/97                                                   #
#         M. Bintz,  3/9/2000                                                 #
#                                                                             #
# Compiler rules for building 32 bit ARM modules with GCC compiler            #
#                                                                             #
# This file is used as the basis for driver-specific makefiles which are      #
# constructed as part of the build process                                    #
#                                                                             #
###############################################################################

!include $(SABINE_ROOT)\make\product.mak

#########################################
# CHECK for Valid ARM ENVIRONMENT Setup #
#########################################
!if   "$(ARM_TOOLKIT)" != "WRGCC"             # . ARM_TOOLKIT
!message ARM_TOOLKIT must be defined as "WRGCC"
!error ARM_TOOLKIT not properly defined!
!endif                                      # . ARM_TOOLKIT

CC     = $(WR_BIN_PATH)\ccarm.exe 

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
CDEBUG = -O3 -fno-strict-aliasing
DEBUGDIR = RELEASE
!endif

!if "$(GNU_CDEBUG_OVR)" != ""
CDEBUG = $(GNU_CDEBUG_OVR)
!endif

CDEBUG = $(CDEBUG) $(GNU_CDEBUG_APP)

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
!if "$(ARM_VERSION)" != "220"		
		-nostdinc \
        -Wimplicit \
!endif
		-mno-sched-prolog \
		-fno-builtin \
		-fvolatile \
		-Wall \
		-Wcomment \
		-fdollars-in-identifiers \
		-c

!if "$(APPNAME)"=="CODELDR"
CDEFS  = -DBSP_LITTLE_ENDIAN=1 -DCONFIG=$(CONFIG) -DCUSTOMER=$(CUSTOMER) -DRVERSION=$(VERSION) -DRTOS=NOOS -DCPU=$(CPU) -DARM_VERSION=$(ARM_VERSION)
!else
CDEFS  = -DBSP_LITTLE_ENDIAN=1 -DCONFIG=$(CONFIG) -DCUSTOMER=$(CUSTOMER) -DRVERSION=$(VERSION) -DRTOS=$(RTOS) -DCPU=$(CPU) -DARM_VERSION=$(ARM_VERSION)
!endif

!if "$(CHIP_NAME)" == "WABASH"
!else
!endif

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

.c{$(SABINE_ROOT)\$(DRIVER)\$(CONFIG)\$(APPNAME)\$(RTOS)\$(SWCONFIG)\$(DEBUGDIR)\}.obj:
 -@echo ---- Compiling ARM mode $(*B).c
 -@echo ---- Compiling ARM mode $(*B).c >> $(COMBINED_ERROR_LOG)
 -@del $@ > nul: 2>&1
 -@$(CC) $(COPTS) $(MAKEDIR)\$(*B).c -o $@ >$(*B).log
 -@type $(*B).log
 -@type $(*B).log >> $(COMBINED_ERROR_LOG)

!else

###################################
# Verbose build - echo everything #
###################################

.c{$(SABINE_ROOT)\$(DRIVER)\$(CONFIG)\$(APPNAME)\$(RTOS)\$(SWCONFIG)\$(DEBUGDIR)\}.obj:
 -@echo ---- Compiling ARM mode $(*B).c
 -@echo ---- Compiling ARM mode $(*B).c >> $(COMBINED_ERROR_LOG)
 -del $@ > nul: 2>&1
 -$(CC) $(COPTS) $(MAKEDIR)\$(*B).c -o $@ >$(*B).log
 -@type $(*B).log
 -@type $(*B).log >> $(COMBINED_ERROR_LOG)

!endif

