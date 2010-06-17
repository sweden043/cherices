###############################################################################
#                                                                             #
# CNXT MPEG build support file: get.mak                                       #
#                                                                             #
# Author: M. Bintz 7/24/03                                                    #
#                                                                             #
# Compiler rules for all toolchains and file types.                           #
#                                                                             #
###############################################################################

###########################################
# Set up the toolexe directory            #
###########################################
tools_dir:
# Do not get the toolexe dir if the GET_TOOLEXE is set to NO # 
# The toolexe directory is very large and takes a long time  # 
# to download from remote locations.						 #
!if "$(GET_TOOLEXE)" != "NO"
!if exist ($(SABINE_ROOT)\toolexe) == 0
  -@mkdir $(SABINE_ROOT)\toolexe
!endif
!if "$(TYPE)" != "LOCAL"
  @cd $(SABINE_ROOT)\toolexe
  -get $(GET_OPTIONS) -u -v$(VERSION) $(PVCS_ROOT)\toolexe\*.??v
!endif
!else #$(GET_TOOLEXE) != "NO"#
  @echo NOT getting the toolexe directory (GET_TOOLEXE set to NO)...
!endif #$(GET_TOOLEXE) != "NO"#
  @cd $(MAKEDIR)

###########################################
# Set up the make and include directories #
###########################################
get_make: make_dir                         \
          $(SABINE_ROOT)\make\c32lrule.mak \
          $(SABINE_ROOT)\make\c32l_vx.mak  \
          $(SABINE_ROOT)\make\c16lrule.mak \
          $(SABINE_ROOT)\make\a32lrule.mak \
          $(SABINE_ROOT)\make\a32l_vx.mak  \
          $(SABINE_ROOT)\make\a16lrule.mak \
          $(SABINE_ROOT)\make\topmake.mak
	@cd $(MAKEDIR)

make_dir:
!if exist ($(SABINE_ROOT)\make) == 0
  -@mkdir $(SABINE_ROOT)\make
!endif
   @cd $(SABINE_ROOT)\make

!if "$(DRIVER)" == "BUILD" || "$(DRIVER)" == "GET"
$(SABINE_ROOT)\make\c32l_vx.mak: $(PVCS_ROOT)\make\c32l_vx.mav

$(SABINE_ROOT)\make\a32l_vx.mak: $(PVCS_ROOT)\make\a32l_vx.mav

$(SABINE_ROOT)\make\c16l_vx.mak: $(PVCS_ROOT)\make\c16l_vx.mav

$(SABINE_ROOT)\make\a16l_vx.mak: $(PVCS_ROOT)\make\c16l_vx.mav

$(SABINE_ROOT)\make\c32lrule.mak: $(PVCS_ROOT)\make\c32lrule.mav

$(SABINE_ROOT)\make\c16lrule.mak: $(PVCS_ROOT)\make\c16lrule.mav

$(SABINE_ROOT)\make\a32lrule.mak: $(PVCS_ROOT)\make\a32lrule.mav

$(SABINE_ROOT)\make\a16lrule.mak: $(PVCS_ROOT)\make\a16lrule.mav

$(SABINE_ROOT)\make\topmake.mak:  $(PVCS_ROOT)\make\topmake.mav
!endif

include_dir:
  -@md $(SABINE_ROOT)\include > nul: 2>&1
!if "$(GET_INCLUDE)"!="NO"
!if "$(TYPE)" == "GET"
  @cd $(SABINE_ROOT)\include
  -get $(GET_OPTIONS) -v$(VERSION) $(PVCS_ROOT)\include\*.??v
!endif
!else #$(GET_TOOLEXE) != "NO"#
  @echo NOT getting the include directory (GET_INCLUDE set to NO)...
!endif
  @cd $(MAKEDIR)

os_include_dir:
!if "$(OS_INCLUDE_DIR)" != ""
!if exist ($(SABINE_ROOT)\$(OS_INCLUDE_DIR)) == 0
  -@mkdir $(SABINE_ROOT)\$(OS_INCLUDE_DIR)
!endif
!if "$(TYPE)" != "LOCAL"
  @cd $(SABINE_ROOT)\$(OS_INCLUDE_DIR)
  -get $(GET_OPTIONS) -v$(VERSION) $(PVCS_ROOT)\$(OS_INCLUDE_DIR)\*.??v
!endif
  @cd $(MAKEDIR)
!endif

!if "$(EXTRA_INCLUDE_DIRS)" != ""
!if "$(TYPE)" == "GET"
$(EXTRA_INCLUDE_DIRS): force_extra_include_dirs
  @if not exist $(SABINE_ROOT)\$(*B)\nul mkdir $(SABINE_ROOT)\$(*B)
  @cd $(SABINE_ROOT)\$(*B)
  -get $(GET_OPTIONS) -v$(VERSION) $(PVCS_ROOT)\$(*B)\*.??v
!endif
!endif

########################################################
# Make source files rule terminal, Files are
# exctracted by the commands of drv_get:
# These rules basically replace the rules in the 
# $driver.mak file that sets a dependecy of the source
# to the archive
########################################################
!if "$(DRIVER)" != ""
!if "$(GENERAL_C_FILES)" != ""
$(GENERAL_C_FILES):
!endif
!if "$(ARM_C_FILES)" != ""
$(ARM_C_FILES):
!endif
!if "$(THUMB_C_FILES)" != ""
$(THUMB_C_FILES):
!endif
!if "$(ARM_ASM_FILES)" != ""
$(ARM_ASM_FILES):
!endif
!if "$(GNU_ARM_ASM_FILES)" != ""
$(GNU_ARM_ASM_FILES):
!endif
!if "$(GNU_THUMB_ASM_FILES)" != ""
$(GNU_THUMB_ASM_FILES):
!endif
!if "$(ARM_GASP_FILES)" != ""
$(ARM_GASP_FILES):
!endif
!if "$(THUMB_GASP_FILES)" != ""
$(THUMB_GASP_FILES):
!endif
!if "$(THUMB_ASM_FILES)" != ""
$(THUMB_ASM_FILES):
!endif
!if "$(OTHER_FILES)" != ""
$(OTHER_FILES):
!endif
!if "$(PUBLIC_HEADERS)" != ""
$(PUBLIC_HEADERS):
!endif

!endif

##################################################
# Get source files from PVCS for a single driver #
##################################################
drv_get::$(ARM_C_FILES)       \
         $(THUMB_C_FILES)     \
		 $(GENERAL_C_FILES)   \
         $(ARM_ASM_FILES)     \
         $(THUMB_ASM_FILES)   \
		 $(GNU_ARM_ASM_FILES)   \
		 $(GNU_THUMB_ASM_FILES) \
         $(ARM_GASP_FILES)    \
         $(THUMB_GASP_FILES)  \
         $(OTHER_FILES) \
		 $(PUBLIC_HEADERS)
  @echo GET      for module $(DRIVER) . . .
  @echo GET      for module $(DRIVER) . . .  >> $(COMBINED_ERROR_LOG)
  @echo vcsdir=$(PVCS_ROOT)\$(DRIVER)>vcs.cfg
  -get $(GET_OPTIONS) -v$(VERSION) $**














#****************************************************************************
#* $Log: 
#*  6    mpeg      1.5         9/30/03 11:18:18 AM    Miles Bintz     SCR(s) 
#*        7583 :
#*        fixed targets to support TYPE=COPY
#*        
#*  5    mpeg      1.4         9/25/03 3:51:26 PM     Miles Bintz     SCR(s) 
#*        7551 :
#*        added TYPE=INCLUDE rules
#*        
#*  4    mpeg      1.3         9/10/03 6:30:20 PM     Miles Bintz     SCR(s) 
#*        7291 :
#*        generate logs of gets
#*        
#*  3    mpeg      1.2         9/9/03 5:42:10 PM      Miles Bintz     SCR(s) 
#*        7291 :
#*        fixed dependency termination for gnu_*_asm_files
#*        
#*  2    mpeg      1.1         9/9/03 5:38:56 PM      Miles Bintz     SCR(s) 
#*        7291 :
#*        updates for reworked build system
#*        
#*  1    mpeg      1.0         9/9/03 4:40:42 PM      Miles Bintz     
#* $
#  
#     Rev 1.5   30 Sep 2003 10:18:18   bintzmf
#  SCR(s) 7583 :
#  fixed targets to support TYPE=COPY
#  
#     Rev 1.4   25 Sep 2003 14:51:26   bintzmf
#  SCR(s) 7551 :
#  added TYPE=INCLUDE rules
#  
#     Rev 1.3   10 Sep 2003 17:30:20   bintzmf
#  SCR(s) 7291 :
#  generate logs of gets
#  
#     Rev 1.2   09 Sep 2003 16:42:10   bintzmf
#  SCR(s) 7291 :
#  fixed dependency termination for gnu_*_asm_files
#  
#     Rev 1.1   09 Sep 2003 16:38:56   bintzmf
#  SCR(s) 7291 :
#  updates for reworked build system
#  
#     Rev 1.0   09 Sep 2003 15:40:42   bintzmf
#  SCR(s) 7291 :
#  reworked build system
#  
#     Rev 1.0   28 Aug 2003 15:25:52   bintzmf
#  Initial revision.
#  
#*  
#*  
#****************************************************************************


