###############################################################################
#                                                                             #
# CNXT MPEG build support file: depend.mak                                    #
#                                                                             #
# Author: M. Bintz 7/24/03                                                    #
#                                                                             #
# Compiler rules for all toolchains and file types.                           #
#                                                                             #
###############################################################################
.SUFFIXES: .d32 .d16 .c

!include $(SDCAPPS_ROOT)\make\common.mak
!include $(SDCAPPS_ROOT)\make\builddirs.mak
!include $(SDCAPPS_ROOT)\make\flags.mak

!include $(SDCAPPS_ROOT)\$(SDCDRIVER)\$(SDCDRIVER).MAK

MKDEP = $(SABINE_ROOT)\toolexe\MAKEDEPEND.EXE

MKDEP_FLAGS = \
 -I$(INC_FIX:;= -I) \
 -I$(SDCAPPS_ROOT)\$(SDCDRIVER) \
 $(CDEFS) \
 -Q -B 

!if "$(VERBOSE)" == "YES"
MKDEP_FLAGS = $(MKDEP_FLAGS) -v
!endif

MKDEP_FLAGS = $(MKDEP_FLAGS: -UDEBUG=)
MKDEP_FLAGS = $(MKDEP_FLAGS: -UTRACE_OUTPUT=)


!if "$(MODE)" == "THUMB"
THUMB_C_FILES = $(THUMB_C_FILES) $(GENERAL_C_FILES)
!else
ARM_C_FILES = $(ARM_C_FILES) $(GENERAL_C_FILES)
!endif


{$(SDCAPPS_ROOT)\$(SDCDRIVER)}.c.d16:
	-@echo -- Generating THUMB dependency for $(<F)
# It is important that there is no space in the echo command or it'll break NMAKE.  See CR 8152
	-$(AT)echo.>  $(SDCAPPS_ROOT)\$(SDCDRIVER)\$(INTER_OUTPUT_DIR)\$@
	-$(AT)$(MKDEP) $(MKDEP_FLAGS) $(EXTRA_MAKEDEPEND_FLAGS) -C \
	-f$(SDCAPPS_ROOT)\$(SDCDRIVER)\$(INTER_OUTPUT_DIR)\$@ $(<) 


{$(SDCAPPS_ROOT)\$(SDCDRIVER)}.c.d32:
	-@echo -- Generating ARM dependency for $(<F)
# It is important that there is no space in the echo command or it'll break NMAKE.  See CR 8152
	-$(AT)echo.>  $(SDCAPPS_ROOT)\$(SDCDRIVER)\$(INTER_OUTPUT_DIR)\$@
	-$(AT)$(MKDEP) $(MKDEP_FLAGS) $(EXTRA_MAKEDEPEND_FLAGS) -C \
	-f$(SDCAPPS_ROOT)\$(SDCDRIVER)\$(INTER_OUTPUT_DIR)\$@ $(<)


$(SDCAPPS_ROOT)\$(SDCDRIVER)\$(INTER_OUTPUT_DIR)\module_$(SDCDRIVER).d16: $(THUMB_C_FILES:.C=.D16) 
!if "$(THUMB_C_FILES)"!=""
    $(AT)copy $(**: =+) $@ > nul:
!else
# It is important that there is no space in the echo command or it'll break NMAKE.  See CR 8152
    $(AT)echo.>  $@
!endif

$(SDCAPPS_ROOT)\$(SDCDRIVER)\$(INTER_OUTPUT_DIR)\module_$(SDCDRIVER).d32: $(ARM_C_FILES:.C=.D32)
!if "$(ARM_C_FILES)"!=""
    $(AT)copy $(**: =+) $@ > nul:
!else
# It is important that there is no space in the echo command or it'll break NMAKE.  See CR 8152
    $(AT)echo.>  $@
!endif



all :  $(SDCAPPS_ROOT)\$(SDCDRIVER)\$(INTER_OUTPUT_DIR)\module_$(SDCDRIVER).d16 \
       $(SDCAPPS_ROOT)\$(SDCDRIVER)\$(INTER_OUTPUT_DIR)\module_$(SDCDRIVER).d32  

#****************************************************************************
#* $Log:
#*  1    mpeg      1.0         7/5/2004 7:11:54 PM    Ford Fu         CR(s)
#*       9660 9659 : SDC APPLICATION check in, make files 
#* $
#  
#     Rev 1.2   26 Sep 2003 11:07:58   bintzmf
#  SCR(s) 7551 :
#  fixed makedepend warnings AGAIN
#  
#     Rev 1.1   18 Sep 2003 14:24:40   bintzmf
#  SCR(s) 7484 :
#  removed the -UDEBUG flag before running makedepend
#  
#     Rev 1.0   09 Sep 2003 15:40:38   bintzmf
#  SCR(s) 7291 :
#  reworked build system
#  
#     Rev 1.1   28 Aug 2003 15:25:32   bintzmf
#  
#     Rev 1.0   14 Aug 2003 13:41:46   bintzmf
#  new build system
#*  
#*  
#****************************************************************************


