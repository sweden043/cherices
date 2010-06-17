###############################################################################
#                                                                             #
# CNXT MPEG build support file: label.mak                                     #
#                                                                             #
# Author: M. Bintz 7/24/03                                                    #
#                                                                             #
# Compiler rules for all toolchains and file types.                           #
#                                                                             #
###############################################################################


##########################################
# Label all the files in a single driver #
##########################################

drv_label: $(ARM_C_FILES)       \
           $(THUMB_C_FILES)     \
		   $(GENERAL_C_FILES)   \
           $(ARM_ASM_FILES)     \
           $(THUMB_ASM_FILES)   \
		   $(GNU_ARM_ASM_FILES)   \
		   $(GNU_THUMB_ASM_FILES) \
           $(ARM_GASP_FILES)    \
           $(THUMB_GASP_FILES)  \
           $(OTHER_FILES)       \
           $(PUBLIC_HEADERS)	\
           $(DRIVER).mak
  cd $(SABINE_ROOT)\$(DRIVER)
  @echo vcsdir=$(PVCS_ROOT)\$(DRIVER)>vcs.cfg
  -vcs -y -v$(VERSION) $**
!if "$(DELCFG)" == "YES"
  -@if exist vcs.cfg del vcs.cfg
!endif


make_label:
  cd $(SABINE_ROOT)\make
  @echo vcsdir=$(PVCS_ROOT)\make>vcs.cfg
  -vcs -y -v$(VERSION) *.??v
!if "$(DELCFG)" == "YES"
  -@if exist vcs.cfg del vcs.cfg
!endif

include_label:
  cd $(SABINE_ROOT)\include
  @echo vcsdir=$(PVCS_ROOT)\include>vcs.cfg
  -vcs -y -v$(VERSION) *.??v

os_include_label:
!if "$(OS_INCLUDE_DIR)" != ""
  cd $(SABINE_ROOT)\$(OS_INCLUDE_DIR)
  @echo vcsdir=$(PVCS_ROOT)\$(OS_INCLUDE_DIR)>vcs.cfg
  -vcs -y -v$(VERSION) *.??v
!endif

config_label:
  cd $(SABINE_ROOT)
  @echo vcsdir=$(PVCS_ROOT)\new_configs>vcs.cfg
  -vcs -y -v$(VERSION) *.??v
!if "$(DELCFG)" == "YES"
  -@if exist vcs.cfg del vcs.cfg
!endif

tools_label:
  cd $(SABINE_ROOT)
  @echo vcsdir=$(PVCS_ROOT)\toolexe>vcs.cfg
  -vcs -y -v$(VERSION) *.??v
!if "$(DELCFG)" == "YES"
  -@if exist vcs.cfg del vcs.cfg
!endif

!if "$(EXTRA_INCLUDE_DIRS)" != ""
!if "$(TYPE)" == "LABEL"
$(EXTRA_INCLUDE_DIRS): force_extra_include_dirs
  cd $(SABINE_ROOT)\$(*B)
  @echo vcsdir=$(PVCS_ROOT)\$(*B)>vcs.cfg
  -vcs -y -v$(VERSION) *.??v
!endif
!endif









#****************************************************************************
#* $Log: 
#*  2    mpeg      1.1         9/30/03 11:18:18 AM    Miles Bintz     SCR(s) 
#*        7583 :
#*        fixed targets to support TYPE=COPY
#*        
#*  1    mpeg      1.0         9/9/03 4:40:42 PM      Miles Bintz     
#* $
#  
#     Rev 1.1   30 Sep 2003 10:18:18   bintzmf
#  SCR(s) 7583 :
#  fixed targets to support TYPE=COPY
#  
#     Rev 1.0   09 Sep 2003 15:40:42   bintzmf
#  SCR(s) 7291 :
#  reworked build system
#  
#     Rev 1.0   28 Aug 2003 15:25:54   bintzmf
#  Initial revision.
#  
#*  
#*  
#****************************************************************************


