###############################################################################
#                                                                             #
# CNXT MPEG build support file: copy.mak                                      #
#                                                                             #
# Author: M. Bintz 7/24/03                                                    #
#                                                                             #
# Compiler rules for all toolchains and file types.                           #
#                                                                             #
###############################################################################

###########################################
# Set up the toolexe directory            #
###########################################
copy_tools_dir:
# Do not get the toolexe dir if the GET_TOOLEXE is set to NO # 
# The toolexe directory is very large and takes a long time  # 
# to download from remote locations.						 #
	@echo Copying toolexe directory...
	@echo Copying toolexe directory... >> $(COMBINED_ERROR_LOG)
	-@mkdir $(SABINE_ROOT)\toolexe > nul: 2>&1
	@cd $(SABINE_ROOT)\toolexe
	-$(AT)xcopy /K /R /Y $(SOURCE)\TOOLEXE\* >> $(COMBINED_ERROR_LOG) 2>&1



# We are in the driver's directory when this makefile is called
copy_include_dir:
	@echo Copying include directory...
	@echo Copying include directory... >> $(COMBINED_ERROR_LOG)
	-@md $(SABINE_ROOT)\include > nul: 2>&1
!if "$(TYPE)" == "COPY"
	@cd $(SABINE_ROOT)\include
    -$(AT)xcopy /K /R /Y $(SOURCE)\include\*  > nul: 2>&1
!endif
	@cd $(MAKEDIR)

copy_os_include_dir:
	@echo Copying OS include directory...
	@echo Copying OS include directory... >> $(COMBINED_ERROR_LOG)
!if "$(OS_INCLUDE_DIR)" != ""
	-$(AT)mkdir $(SABINE_ROOT)\$(OS_INCLUDE_DIR) > nul: 2>&1
!if "$(TYPE)" == "COPY"
	@cd $(SABINE_ROOT)\$(OS_INCLUDE_DIR)
    -$(AT)xcopy /K /R /Y $(SOURCE)\$(OS_INCLUDE_DIR)\*  >> $(COMBINED_ERROR_LOG)  2>&1
!endif
  @cd $(MAKEDIR)
!endif

!if "$(EXTRA_INCLUDE_DIRS)" != ""
!if "$(TYPE)" == "COPY" 
$(EXTRA_INCLUDE_DIRS): force_extra_include_dirs
	@echo Copying extra include directory...  ($(*B))
	@echo Copying extra include directory...  ($(*B)) >> $(COMBINED_ERROR_LOG)
	-$(AT)mkdir $(SABINE_ROOT)\$(*B) > nul: 2>&1
	@cd $(SABINE_ROOT)\$(*B)
	-$(AT)xcopy /K /R /Y $(SOURCE)\$(*B)\*  >> $(COMBINED_ERROR_LOG)  2>&1
!endif
!endif


drv_copy:
	@echo Copying files for driver    $(DRIVER)
	@echo Copying files for driver    $(DRIVER) >> $(COMBINED_ERROR_LOG)
	$(AT)cd $(SABINE_ROOT)\$(DRIVER)
	-$(AT)for %i in ($(ARM_C_FILES) $(THUMB_C_FILES) $(GENERAL_C_FILES) $(ARM_ASM_FILES) \
	           $(THUMB_ASM_FILES) $(GNU_ARM_ASM_FILES) $(GNU_THUMB_ASM_FILES) \
			   $(ARM_GASP_FILES) $(THUMB_GASP_FILES)  $(OTHER_FILES) $(PUBLIC_HEADERS)) DO @xcopy /K /R /Y \
			   $(SOURCE)\$(DRIVER)\%i  >> $(COMBINED_ERROR_LOG)  2>&1






#****************************************************************************
#* $Log: 
#*  3    mpeg      1.2         11/4/03 2:52:58 PM     Miles Bintz     CR(s): 
#*        7825 modified makefiles to generate more useful information in logs
#*  2    mpeg      1.1         9/30/03 11:18:16 AM    Miles Bintz     SCR(s) 
#*        7583 :
#*        fixed targets to support TYPE=COPY
#*        
#*  1    mpeg      1.0         9/29/03 6:43:32 PM     Miles Bintz     
#* $
#****************************************************************************


