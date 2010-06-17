
###############################################################
#    COALESCE LIST OF FILES TO BUILD
###############################################################
# Determine how our C files will be built
!if "$(MODE)" == "THUMB"
!if "$(THUMB_C_FILES)" != ""
THUMB_C_FILES = $(THUMB_C_FILES) $(GENERAL_C_FILES)
!else
THUMB_C_FILES = $(GENERAL_C_FILES)
!endif
!else
!if "$(ARM_C_FILES)" != ""
ARM_C_FILES = $(ARM_C_FILES) $(GENERAL_C_FILES)
!else
ARM_C_FILES = $(GENERAL_C_FILES)
!endif
!endif


# With the GNU based assemblers, we need an intermediate
# (ie use the C preprocessor) formatted assembly files.
# To add insult to injury, we still need to support assembly
# files that are already in GNU format.
OBJECT_LIST_SLACK = \
  $(ARM_C_FILES:.C=.O32) \
  $(THUMB_C_FILES:.C=.O16) \
  $(ARM_ASM_FILES:.S=.O32) \
  $(THUMB_ASM_FILES:.S=.O16) \
  $(ARM_GASP_FILES:.GASP=.O32) \
  $(THUMB_GASP_FILES:.GASP=.O16) \
  $(GNU_ARM_ASM_FILES:.GNUS=.O32) \
  $(GNU_THUMB_ASM_FILES:.GNUS=.O16) 


OBJECT_LIST = FOO;$(OBJECT_LIST_SLACK:      = );BAR
OBJECT_LIST = $(OBJECT_LIST:     = )
OBJECT_LIST = $(OBJECT_LIST:    = )
OBJECT_LIST = $(OBJECT_LIST:   = )
OBJECT_LIST = $(OBJECT_LIST:  = )
OBJECT_LIST = $(OBJECT_LIST:FOO;=)
OBJECT_LIST = $(OBJECT_LIST:;BAR=)

DEPEND_LIST_SLACK = $(ARM_C_FILES) $(THUMB_C_FILES)
DEPEND_LIST = FOO;$(DEPEND_LIST_SLACK:      = );BAR
DEPEND_LIST = $(DEPEND_LIST:     = )
DEPEND_LIST = $(DEPEND_LIST:    = )
DEPEND_LIST = $(DEPEND_LIST:   = )
DEPEND_LIST = $(DEPEND_LIST:  = )
DEPEND_LIST = $(DEPEND_LIST:FOO;=)
DEPEND_LIST = $(DEPEND_LIST:;BAR=)


S_SUBDIRS = FOO;S_$(SUBDIRS);BAR
S_SUBDIRS = $(S_SUBDIRS: = S_)
S_SUBDIRS = $(S_SUBDIRS:FOO;=)
S_SUBDIRS = $(S_SUBDIRS:;BAR=)

###############################################################
#    DEFAULT BUILD TARGETS
###############################################################

!if "$(TYPE)"=="CLEAN"
all: clean 
!elseif "$(TYPE)"=="INCLUDE"
all: include 
!else
all: $(TARGET)
!endif

!include $(SDCAPPS_ROOT)\make\config.mak

clearlog:
	-@del /f $(BUILDLOG) > nul: 2>&1


clean: $(S_SUBDIRS)
	-$(AT)del depend > nul: 2>&1
	-$(AT)del *.map > nul: 2>&1
	-$(AT)del *.d?? > nul: 2>&1
	-$(AT)del *.o?? > nul: 2>&1
	-$(AT)del *.log > nul: 2>&1
	-$(AT)del /f $(SDCAPPS_ROOT)\$(CONFIG)\$(DEBUGDIR)\$(VERSION)\bldver.h  > nul: 2>&1
	-$(AT)del /f $(SDCAPPS_ROOT)\$(CONFIG)\$(DEBUGDIR)\$(VERSION)\build.log  > nul: 2>&1
	-$(AT)del /f $(SDCAPPS_ROOT)\$(CONFIG)\$(DEBUGDIR)\$(VERSION)\drv_incl.h  > nul: 2>&1
	-$(AT)del /f $(OUTPUTLIB) > nul: 2>&1
!if "$(APPNAME)"!=""
	-$(AT)del /s $(APPNAME) > nul: 2>&1
!endif

include: $(S_SUBDIRS)

subdir:  $(S_SUBDIRS)

lib: $(BUILDLOG) do_cfgtoh buildvars $(S_SUBDIRS) this 


app:  $(BUILDLOG) do_cfgtoh buildvars $(S_SUBDIRS) this 
	-@echo.
	-@echo *** Finished building ***
	-@echo.

###############################################################
#    TARGET RULES 
###############################################################

!if "$(OUTPUTLIB)"==""
OUTPUTLIB = $(SDCAPPS_ROOT)\$(CONFIG)\$(DEBUGDIR)\$(VERSION)\$(LIBNAME)
!endif

!if "$(DEPEND_LIST)"==" " || "$(DEPEND_LIST)"==""
depend:

!else
depend: $(DEPEND_LIST)
	@echo.>$@
	$(AT)$(MKDEP) $(MKDEP_FLAGS) -C -f$@ $**

!endif


!if "$(SUBDIRS)" == "" || "$(SUBDIRS)" == " "
$(S_SUBDIRS):

	
!else
$(S_SUBDIRS):
!if "$(TYPE)"=="CLEAN"
	-$(AT)cd $(@:S_=)
	-@echo Cleaning directory $(@:S_=) 
	$(AT)nmake /b /nologo /f Makefile TYPE=CLEAN
	-$(AT)cd ..
!else
	-@echo __\ Changing directory to $(@:S_=)
	-$(AT)cd $(@:S_=)
	$(AT)nmake /b /nologo /f Makefile
	-@echo /__ Leaving directory 
	-$(AT)cd ..
!endif

!endif

$(OUTPUT_DIR):
	$(AT)if not exist $(OUTPUT_DIR) md $(OUTPUT_DIR) > nul: 2>&1

$(LIB_OUTPUT_DIR):
	$(AT)if not exist $(LIB_OUTPUT_DIR) md $(LIB_OUTPUT_DIR) > nul: 2>&1

$(IMG_OUTPUT_DIR):
	$(AT)if not exist $(IMG_OUTPUT_DIR) md $(IMG_OUTPUT_DIR) > nul: 2>&1
	
this:  objects

# WARNING!!!!   in order for this to work you _MUST_ run ADS!!!
addlibtolib: $(LIBONLY_FILES)
	$(AT)if not exist $(OUTPUTLIB) $(LIBR) $(LIBRFLAGS) $(OUTPUTLIB) >> $(BUILDLOG)
	$(AT)$(LIBR) $(LIBRFLAGS_REPLACE) $(OUTPUTLIB) $** >> $(BUILDLOG)
	
	
!if "$(OBJECT_LIST)" == " " || "$(OBJECT_LIST)" == ""
objects:

!else
objects: $(OBJECT_LIST)

!endif








