###############################################################################
#                                                                             #
# CNXT MPEG build support file: cdefs.mak                                     #
#                                                                             #
# Author: M. Bintz 7/24/03                                                    #
#                                                                             #
# Compiler rules for all toolchains and file types.                           #
#                                                                             #
###############################################################################

!include $(SABINE_ROOT)\make\flags.mak


############################################
# Have the link addresses been overridden? #
############################################
!if "$(RAM_BASE_OVERRIDE)" != ""
LINKER_RAM_BASE = $(RAM_BASE_OVERRIDE)
!endif
!if "$(ROM_BASE_OVERRIDE)" != ""
LINKER_ROM_BASE = $(ROM_BASE_OVERRIDE)
!endif

!if "$(SEMIHOSTING_APP)"=="NO"

app_link: \
	$(OUTPUT_DIR)\$(APPNAME)_RAM.BIN \
	$(OUTPUT_DIR)\$(APPNAME)_ROM.BIN
!if "$(IMAGE_TYPE)"=="GENERIC"
!if "$(CABLE_MODEM)" == "WABASH_INTERNAL"
	-$(AT)copy $(OUTPUT_DIR)\$(APPNAME)_ram.bin $(OUTPUT_DIR)\$(APPNAME)_!cm.bin > tmp.log
!if "$(CM_INCLUDE_DVS167_1W)"=="YES"
	@echo Appending DVS167_1W Image...
	-$(AT)copy /b $(OUTPUT_DIR)\$(APPNAME)_ram.bin + \
	$(SABINE_ROOT)\CM_IMG\$(CM_IMG_PREFIX)_$(IFTYPE)_DVS167_1W.bin \
	$(OUTPUT_DIR)\$(APPNAME)_ram.bin > tmp.log
!endif  # CM_INCLUDE_DVS167_1W=YES
!if "$(CM_INCLUDE_DVS167_2W)"=="YES"
	@echo Appending DVS167_2W Image...
	-$(AT)copy /b $(OUTPUT_DIR)\$(APPNAME)_ram.bin + \
	$(SABINE_ROOT)\CM_IMG\$(CM_IMG_PREFIX)_$(IFTYPE)_DVS167_2W.bin \
	$(OUTPUT_DIR)\$(APPNAME)_ram.bin > tmp.log
!endif  # CM_INCLUDE_DVS167_2W=YES
!if "$(IPSTB_INCLUDE)"=="YES"
	@echo Appending IPSTB CM SBFE Image...
	-$(AT)copy /b $(OUTPUT_DIR)\$(APPNAME)_ram.bin + \
	$(SABINE_ROOT)\CM_IMG\$(CM_IMG_PREFIX)_$(IFTYPE)_SBFE.bin \
	$(OUTPUT_DIR)\$(APPNAME)_ram.bin > tmp.log
!endif  # IPSTB_INCLUDE=YES
!if "$(CM_INCLUDE_DOCSIS)"=="YES"
	@echo Appending DOCSIS Image...
	-$(AT)copy /b $(OUTPUT_DIR)\$(APPNAME)_ram.bin + \
	$(SABINE_ROOT)\CM_IMG\$(CM_IMG_PREFIX)_$(IFTYPE)_DOCSIS.bin \
	$(OUTPUT_DIR)\$(APPNAME)_ram.bin > tmp.log
!endif  # CM_INCLUDE_DOCSIS=YES
!endif  # CABLE_MODEM
!endif  # IMAGE_TYPE

!else  # SEMIHOSTING_APP

app_link: $(OUTPUT_DIR)\$(APPNAME)_semi.AXF

!endif



$(OUTPUT_DIR)\$(APPNAME)_RAM.BIN: $(OUTPUT_DIR)\$(APPNAME)_ram.$(EXT_IMG)
	-@echo Generating binary for RAM image
	-@echo Generating binary for RAM image >> $(COMBINED_ERROR_LOG)
!if "$(ARM_TOOLKIT)" == "WRGCC" || "$(ARM_TOOLKIT)" == "GNUGCC"
	-$(AT)$(WR_BIN_PATH)\objcopyarm $** -O binary $@
!else
	-$(AT)$(TOBIN) $(TO_BIN_FLAGS) $** -bin $(TO_BIN_OUTPUT) $@     >> $(COMBINED_ERROR_LOG)  2>>&1
!endif
	@echo Adding checksum to binary image
!if "$(IMAGE_TYPE)" != "BOOT"
!if "$(IMAGE_TYPE)" == "OPENTV_12"
	$(AT)$(TOOLEXE_PATH)\chksum12 $@
!else
	$(AT)$(TOOLEXE_PATH)\chksum -i $@
!endif
!endif


$(OUTPUT_DIR)\$(APPNAME)_ROM.BIN: $(OUTPUT_DIR)\$(APPNAME)_rom.$(EXT_IMG)
	-@echo Generating binary for ROM image 
	-@echo Generating binary for ROM image >> $(COMBINED_ERROR_LOG)
	-$(AT)$(DBGTOBIN) 
!if "$(ARM_TOOLKIT)" == "WRGCC" || "$(ARM_TOOLKIT)" == "GNUGCC"
	-$(AT)$(WR_BIN_PATH)\objcopyarm $** -O binary $@
!else
	-$(AT)$(TOBIN) $(TO_BIN_FLAGS) $** -bin $(TO_BIN_OUTPUT) $@     >> $(COMBINED_ERROR_LOG)  2>>&1
!endif
	@echo Adding checksum to binary image
!if "$(IMAGE_TYPE)" != "BOOT"
!if "$(IMAGE_TYPE)" == "OPENTV_12"
	$(AT)$(TOOLEXE_PATH)\chksum12 $@
!else
	$(AT)$(TOOLEXE_PATH)\chksum -i $@
!endif
!endif

$(OUTPUT_DIR)\$(APPNAME)_semi.AXF: $(OUTPUT_DIR)\link_semi.scr
	-@cd $(BLD_LIB_DIR)
	-@echo Linking (semihosted)  $(@F)
	-@echo Linking (semihosted)  $(@F)    >> $(COMBINED_ERROR_LOG)
	-@set ARMLIB=$(SEMIHOSTLIB)
	-$(AT)$(ALINK) $(LOPTS1) $(AXFLINK_LOG) -ro-Base $(LINKER_RAM_BASE) \
	$(LINK_MAP_OUTPUT) $(OUTPUT_DIR)\$(APPNAME)_ram_semi.map \
	$(LINK_DEBUG) \
	$(EXTRA_LINK_FLAGS) -elf -o $@ -via $(OUTPUT_DIR)\link_semi.scr > $(OUTPUT_DIR)\$(APPNAME)_ram_semi.log 2>&1
	-@type $(OUTPUT_DIR)\$(APPNAME)_ram_semi.log 
	-@type $(OUTPUT_DIR)\$(APPNAME)_ram_semi.log >> $(COMBINED_ERROR_LOG)


$(OUTPUT_DIR)\$(APPNAME)_RAM.AXF: $(OUTPUT_DIR)\link.scr
	-@cd $(BLD_LIB_DIR)
	-@echo Linking    $(@F)
	-@echo Linking    $(@F)    >> $(COMBINED_ERROR_LOG)
	-@set ARMLIB=$(ARMLIB)
	-$(AT)$(ALINK) $(LOPTS1) $(AXFLINK_LOG) -ro-Base $(LINKER_RAM_BASE) \
	$(LINK_MAP_OUTPUT) $(OUTPUT_DIR)\$(APPNAME)_ram.map \
	$(LINK_DEBUG) \
	$(EXTRA_LINK_FLAGS) -elf -o $@ -via $(OUTPUT_DIR)\link.scr > $(OUTPUT_DIR)\$(APPNAME)_ram.log 2>&1
	-@type $(OUTPUT_DIR)\$(APPNAME)_ram.log 
	-@type $(OUTPUT_DIR)\$(APPNAME)_ram.log >> $(COMBINED_ERROR_LOG)

	
$(OUTPUT_DIR)\$(APPNAME)_ROM.AXF: $(OUTPUT_DIR)\link.scr
	-@cd $(BLD_LIB_DIR)
	-@echo Linking    $(@F)
	-@echo Linking    $(@F)    >> $(COMBINED_ERROR_LOG)
	-@set ARMLIB=$(ARMLIB)
	-$(AT)$(ALINK) $(LOPTS1) $(RDBLINK_LOG) \
	-RO $(LINKER_ROM_BASE) -RW $(LINKER_RAM_BASE) \
	$(LINK_MAP_OUTPUT) $(OUTPUT_DIR)\$(APPNAME)_rom.map \
	$(LINK_DEBUG) \
	$(EXTRA_LINK_FLAGS) -elf -o $@ -via $(OUTPUT_DIR)\link.scr > $(OUTPUT_DIR)\$(APPNAME)_rom.log 2>&1
	-@type $(OUTPUT_DIR)\$(APPNAME)_rom.log 
	-@type $(OUTPUT_DIR)\$(APPNAME)_rom.log >> $(COMBINED_ERROR_LOG)

!if 0
r: always
	-@echo Generating Linker script...
	-@del /F /Q $@ > nul: 2>&1
# no entry object (well, whatever has main())
    @echo $(SEMIHOSTLIB)\armlib.32l         > $@
!if "$(OS_OBJS)" != ""
	@echo $(OS_OBJS)                        >> $@
!endif
!if "$(OPERATING_SYS_LIBS)" != ""
	@echo $(OPERATING_SYS_LIBS)              >> $@
!endif
!if "$(APP_EXTRA_OBJS)" != ""
	@echo $(APP_EXTRA_OBJS)                  >> $@
!endif
!if "$(OS_LIBS)" != ""
	@echo $(OS_LIBS)                         >> $@
!endif
!if "$(DRIVER_LIBS)" != ""
	@echo $(DRIVER_LIBS)                     >> $@
!endif
!if "$(MOREDRIVER_LIBS)" != ""
	@echo $(MOREDRIVER_LIBS)                 >> $@
!endif
!if "$(EVENMOREDRIVER_LIBS)" != ""
	@echo $(EVENMOREDRIVER_LIBS)             >> $@
!endif
!if "$(APP_EXTRA_LIBS)" != ""
	@echo $(APP_EXTRA_LIBS)                  >> $@
!endif
!if "$(MORE_EXTRA_LIBS)" != ""
	@echo $(MORE_EXTRA_LIBS)                 >> $@
!endif

!endif



$(OUTPUT_DIR)\link_semi.scr $(OUTPUT_DIR)\link.scr: always
	-@echo Generating Linker script...
	-@del /F /Q $@ > nul: 2>&1
!if "$(ENTRY_OBJ)" != ""
	@echo $(ENTRY_OBJ)                       >> $@
!endif
!if "$(OS_OBJS)" != ""
	@echo $(OS_OBJS)                        >> $@
!endif
!if "$(OPERATING_SYS_LIBS)" != ""
	@echo $(OPERATING_SYS_LIBS)              >> $@
!endif
!if "$(APP_EXTRA_OBJS)" != ""
	@echo $(APP_EXTRA_OBJS)                  >> $@
!endif
!if "$(OS_LIBS)" != ""
	@echo $(OS_LIBS)                         >> $@
!endif
!if "$(DRIVER_LIBS)" != ""
	@echo $(DRIVER_LIBS)                     >> $@
!endif
!if "$(MOREDRIVER_LIBS)" != ""
	@echo $(MOREDRIVER_LIBS)                 >> $@
!endif
!if "$(EVENMOREDRIVER_LIBS)" != ""
	@echo $(EVENMOREDRIVER_LIBS)             >> $@
!endif
!if "$(APP_EXTRA_LIBS)" != ""
	@echo $(APP_EXTRA_LIBS)                  >> $@
!endif
!if "$(MORE_EXTRA_LIBS)" != ""
	@echo $(MORE_EXTRA_LIBS)                 >> $@
!endif


$(OUTPUT_DIR)\$(APPNAME)_ROM.ELF: $(OUTPUT_DIR)\gcc_rom_link.scr
	-@cd $(BLD_LIB_DIR)
	-@echo Linking    $(@F)
	-@echo Linking    $(@F)    >> $(COMBINED_ERROR_LOG)
	-$(AT)$(ALINK) $(LOPTS1) $(LINK_MAP_OUTPUT) $(OUTPUT_DIR)\$(APPNAME)_rom.map -T $(OUTPUT_DIR)\gcc_rom_link.scr -o $@ > $(OUTPUT_DIR)\$(APPNAME)_rom.log 2>&1
	-@type $(OUTPUT_DIR)\$(APPNAME)_rom.log 
	-@type $(OUTPUT_DIR)\$(APPNAME)_ram.log >> $(COMBINED_ERROR_LOG)

$(OUTPUT_DIR)\$(APPNAME)_RAM.ELF: $(OUTPUT_DIR)\gcc_ram_link.scr
	-@cd $(BLD_LIB_DIR)
	-@echo Linking    $(@F)
	-@echo Linking    $(@F)    >> $(COMBINED_ERROR_LOG)
	-$(AT)$(ALINK) $(LOPTS1) $(LINK_MAP_OUTPUT) $(OUTPUT_DIR)\$(APPNAME)_ram.map -T $(OUTPUT_DIR)\gcc_ram_link.scr -o $@ > $(OUTPUT_DIR)\$(APPNAME)_ram.log 2>&1
#	-$(AT)$(ALINK) $(LOPTS1) $(LINK_MAP_OUTPUT) $(OUTPUT_DIR)\$(APPNAME)_ram.map -Ttext $(LINKER_RAM_BASE:0x=) $(OUTPUT_DIR)\gcc_link.scr -o $@ > $(OUTPUT_DIR)\$(APPNAME)_ram.log 2>&1
	-@type $(OUTPUT_DIR)\$(APPNAME)_ram.log 
	-@type $(OUTPUT_DIR)\$(APPNAME)_ram.log >> $(COMBINED_ERROR_LOG)


$(OUTPUT_DIR)\gcc_link.scr: always
	@echo Generating Linker script...
	@echo OUTPUT_ARCH(arm)                  > $@
	@echo INPUT ( $(ENTRY_OBJ) )            >> $@
	@echo GROUP ( $(BSP_LIB) $(OS_LIBS)     >> $@
!if "$(APP_EXTRA_OBJS)" != ""
	@echo $(APP_EXTRA_OBJS)                 >> $@
!endif
!if "$(DRIVER_LIBS)" != ""
	@echo $(DRIVER_LIBS)                    >> $@
!endif
!if "$(OPERATING_SYS_LIBS)" != ""
	@echo $(OPERATING_SYS_LIBS)             >> $@
!endif
!if "$(APP_EXTRA_LIBS)" != ""
	@echo $(APP_EXTRA_LIBS)                 >> $@
!endif
!if "$(MORE_EXTRA_LIBS)" != ""
	@echo $(MORE_EXTRA_LIBS)                >> $@
!endif
!if "$(MOREDRIVER_LIBS)" != ""
	@echo $(MOREDRIVER_LIBS)                >> $@
!endif
!if "$(EVENMOREDRIVER_LIBS)" != ""
	@echo $(EVENMOREDRIVER_LIBS)            >> $@
!endif
	@echo )                                    >> $@

$(OUTPUT_DIR)\gcc_ram_link.scr: $(OUTPUT_DIR)\gcc_link.scr
	@echo INCLUDE  $(OUTPUT_DIR)\gcc_link.scr                            >  $@
	@echo.                                                               >> $@
	@echo SECTIONS                                                       >> $@
	@echo {                                                              >> $@
	@echo .text $(LINKER_RAM_BASE) :                         >> $@
	@echo { _text_start = . ; *(.text) *(.text.*)            >> $@
	@echo     *(.stub)                                       >> $@
	@echo     *(.gnu.warning)                                >> $@
	@echo     *(.gnu.linkonce.t*)                            >> $@
	@echo     *(.glue_7t) *(.glue_7)                         >> $@
	@echo } =0                                               >> $@
	@echo .rodata  BLOCK(0x1000) : { *(.rodata) *(.rodata.*) *(.gnu.linkonce.r*) }   >> $@
	@echo .rodata1   : { *(.rodata1) }                       >> $@
	@echo _text_end = . ; _etext = . ;                                   >> $@
	@echo PROVIDE (etext = .);                                           >> $@
	@echo .data :                                                        >> $@
	@echo { _data_start = . ; *(.data); _data_end = . ;  }               >> $@
	@echo .got           : { *(.got.plt) *(.got) }                       >> $@
	@echo .dynamic       : { *(.dynamic) }                               >> $@
	@echo .sdata     : { *(.sdata) *(.sdata.*) }                         >> $@
	@echo _edata = .;                                                    >> $@
	@echo PROVIDE (edata = .);                                           >> $@
	@echo _bss_start   = .;                                              >> $@
	@echo __bss_start   = .;                                             >> $@
	@echo __bss_start__ = .;                                             >> $@
	@echo .sbss      : { *(.sbss) *(.scommon) }                          >> $@
	@echo .bss :                                                         >> $@
	@echo { *(.bss) *(COMMON)                                            >> $@
	@echo   . = ALIGN(32 / 8);                                           >> $@
	@echo }                                                              >> $@
	@echo . = ALIGN(32 / 8);                                             >> $@
	@echo _end = . ; __end__ = . ;                                       >> $@
	@echo _bss_end = . ; _bss_end__ = . ; __bss_end__ = . ;              >> $@
	@echo PROVIDE (end = .);                                             >> $@
	@echo .stab 0 : { *(.stab) }                            >> $@
	@echo .stabstr 0 : { *(.stabstr) }                      >> $@
	@echo .stab.excl 0 : { *(.stab.excl) }                  >> $@
	@echo .stab.exclstr 0 : { *(.stab.exclstr) }            >> $@
	@echo .stab.index 0 : { *(.stab.index) }                >> $@
	@echo .stab.indexstr 0 : { *(.stab.indexstr) }          >> $@
	@echo .comment 0 : { *(.comment) }                      >> $@
	@echo /* DWARF 1 */                                     >> $@
	@echo .debug          0 : { *(.debug) }                 >> $@
	@echo .line           0 : { *(.line) }                  >> $@
	@echo /* GNU DWARF 1 extensions */                      >> $@
	@echo .debug_srcinfo  0 : { *(.debug_srcinfo) }         >> $@
	@echo .debug_sfnames  0 : { *(.debug_sfnames) }         >> $@
	@echo /* DWARF 1.1 and DWARF 2 */                       >> $@
	@echo .debug_aranges  0 : { *(.debug_aranges) }         >> $@
	@echo .debug_pubnames 0 : { *(.debug_pubnames) }        >> $@
	@echo /* DWARF 2 */                                     >> $@
	@echo .debug_info     0 : { *(.debug_info) }            >> $@
	@echo .debug_abbrev   0 : { *(.debug_abbrev) }          >> $@
	@echo .debug_line     0 : { *(.debug_line) }            >> $@
	@echo .debug_frame    0 : { *(.debug_frame) }           >> $@
	@echo .debug_str      0 : { *(.debug_str) }             >> $@
	@echo .debug_loc      0 : { *(.debug_loc) }             >> $@
	@echo .debug_macinfo  0 : { *(.debug_macinfo) }         >> $@
	@echo /* SGI/MIPS DWARF 2 extensions */                 >> $@
	@echo .debug_weaknames 0 : { *(.debug_weaknames) }      >> $@
	@echo .debug_funcnames 0 : { *(.debug_funcnames) }      >> $@
	@echo .debug_typenames 0 : { *(.debug_typenames) }      >> $@
	@echo .debug_varnames  0 : { *(.debug_varnames) }       >> $@
	@echo }                                                              >> $@

$(OUTPUT_DIR)\gcc_rom_link.scr: $(OUTPUT_DIR)\gcc_link.scr 
	@echo INCLUDE  $(OUTPUT_DIR)\gcc_link.scr                            >  $@
	@echo.                                                               >> $@
	@echo SECTIONS                                                       >> $@
	@echo {                                                              >> $@
	@echo .text $(LINKER_ROM_BASE) :                                     >> $@
	@echo { _text_start = . ; *(.text) *(.text.*)    					 >> $@
	@echo   *(.glue_7t) *(.glue_7) }                                     >> $@
	@echo .rodata  BLOCK(0x1000) : { _ro_start = . ; *(.rodata) *(.rodata.*) *(.gnu.linkonce.r*) }   >> $@
	@echo _text_end = . ; _etext = . ;                                   >> $@
	@echo PROVIDE (etext = .);                                           >> $@
	@echo .data $(LINKER_RAM_BASE) :                                     >> $@
	@echo AT ( ADDR (.rodata ) + SIZEOF (.rodata) )                       >> $@
	@echo { _data_start = . ; *(.data) *(.data.*) . = ALIGN(32 / 8) ; _data_end = . ;  }   >> $@
	@echo _edata = .;                                                    >> $@
	@echo PROVIDE (edata = .);                                           >> $@
	@echo _bss_start   = .;                                              >> $@
	@echo __bss_start   = .;                                             >> $@
	@echo __bss_start__ = .;                                             >> $@
	@echo .sbss      : { *(.sbss) *(.scommon) }                          >> $@
	@echo .bss :                                                         >> $@
	@echo { *(.bss) *(COMMON)                                            >> $@
	@echo   . = ALIGN(32 / 8);                                           >> $@
	@echo }                                                              >> $@
	@echo . = ALIGN(32 / 8);                                             >> $@
	@echo _end = . ; __end__ = . ;                                       >> $@
	@echo _bss_end = . ; _bss_end__ = . ; __bss_end__ = . ;              >> $@
	@echo PROVIDE (end = .);                                             >> $@
	@echo .stab 0 : { *(.stab) }                            >> $@
	@echo .stabstr 0 : { *(.stabstr) }                      >> $@
	@echo .stab.excl 0 : { *(.stab.excl) }                  >> $@
	@echo .stab.exclstr 0 : { *(.stab.exclstr) }            >> $@
	@echo .stab.index 0 : { *(.stab.index) }                >> $@
	@echo .stab.indexstr 0 : { *(.stab.indexstr) }          >> $@
	@echo .comment 0 : { *(.comment) }                      >> $@
	@echo /* DWARF 1 */                                     >> $@
	@echo .debug          0 : { *(.debug) }                 >> $@
	@echo .line           0 : { *(.line) }                  >> $@
	@echo /* GNU DWARF 1 extensions */                      >> $@
	@echo .debug_srcinfo  0 : { *(.debug_srcinfo) }         >> $@
	@echo .debug_sfnames  0 : { *(.debug_sfnames) }         >> $@
	@echo /* DWARF 1.1 and DWARF 2 */                       >> $@
	@echo .debug_aranges  0 : { *(.debug_aranges) }         >> $@
	@echo .debug_pubnames 0 : { *(.debug_pubnames) }        >> $@
	@echo /* DWARF 2 */                                     >> $@
	@echo .debug_info     0 : { *(.debug_info) }            >> $@
	@echo .debug_abbrev   0 : { *(.debug_abbrev) }          >> $@
	@echo .debug_line     0 : { *(.debug_line) }            >> $@
	@echo .debug_frame    0 : { *(.debug_frame) }           >> $@
	@echo .debug_str      0 : { *(.debug_str) }             >> $@
	@echo .debug_loc      0 : { *(.debug_loc) }             >> $@
	@echo .debug_macinfo  0 : { *(.debug_macinfo) }         >> $@
	@echo /* SGI/MIPS DWARF 2 extensions */                 >> $@
	@echo .debug_weaknames 0 : { *(.debug_weaknames) }      >> $@
	@echo .debug_funcnames 0 : { *(.debug_funcnames) }      >> $@
	@echo .debug_typenames 0 : { *(.debug_typenames) }      >> $@
	@echo .debug_varnames  0 : { *(.debug_varnames) }       >> $@
	@echo }                                                 >> $@


# 
# Change if INTER_OUTPUT_DIR changes! #
op_dir:
	-@md $(OUTPUT_DIR) > nul: 2>&1



#***************************************************************************
# Pre-processor LINK check ... determine if modem library is needed and
# if there is an appropriate modem library in the path
# note:  By putting this check in this file, it only gets checked when a
# link would occur, and the macro variables are properly defined here

# First, check to see if we are **NOT** building the modem library, if we are
# this check is not needed.
!if "$(BUILDMODEM)" != "YES" || "$(INTERNAL_BUILD)" != "YES"

# Next, check to see if we are doing a "build" (as opposed to a LABEL, GET, etc)
!if "$(TYPE)" == "LOCAL" || "$(TYPE)" == "BUILD" || "$(TYPE)" == ""

# Next, check to see if a modem library is required, if not, then the check
# is not needed
!if "$(CNXT_MODEM_LIBRARY)" != ""

# Now, check to see if the modem library exists, if id doesn't, generate a
# warning message and abort the build process
!if exist ( $(CNXT_MODEM_LIBRARY) ) == 0

!message Error: Modem Library Does Not Exist!
!message ...... $(CNXT_MODEM_LIBRARY)
!message ...... 
!message ...... You have elected to build an application ($(APPNAME))
!message ...... with a CONFIG file ($(CONFIG)) that requires a modem library.
!message ...... That modem library does not exists in the default location that
!message ...... has been specified ($(CNXT_MODEM_LIBDIR))
!message ...... 
!message ...... Option 1) Set the CNXT_MODEM_LIBDIR in your build environment to
!message ...... a directory location (either local, or on the server) that
!message ...... contains an appropriate version of the modem library.  For
!message ...... example, the directory "W:\Sw-Dev\Modem\daily" could be used.
!message ...... This directory should contain a copy of the modem libraries as
!message ...... they were built during the daily build process.
!message ......
!if "$(INTERNAL_BUILD)" == "YES"
!message ...... Option 2) Set BUILDMODEM=YES in your build environment and run
!message ...... the TYPE=GET option on your nmake command line to get the modem
!message ...... source files, then build the application again.
!message ...... 
!endif
!error Modem Library Does Not Exist!

!endif   # !if exist ( $(CNXT_MODEM_LIBRARY) ) != 0

!endif   # !if "$(CNXT_MODEM_LIBRARY)" != ""

!endif   # !if "$(TYPE)" == "LOCAL" || "$(TYPE)" == "BUILD" || "$(TYPE)" == ""

!endif   # !if "$(BUILDMODEM)" != "YES"

###############################################################################
# $Log: 
#  16   mpeg      1.15        4/12/04 6:08:59 PM     Miles Bintz     CR(s) 8816
#         8817 : only append cable modem images if cable_modem == 
#        wabash_internal
#  15   mpeg      1.14        4/5/04 7:19:26 PM      Sahil Bansal    CR(s) 8780
#         8781 : Added step to append IPSTB cm images aka milano_na_sbfe.bin or
#         milano_euro_sbfe.bin to build based on if IPSTB_INCLUDE==YES.
#  14   mpeg      1.13        3/16/04 3:14:29 PM     Miles Bintz     CR(s) 8567
#         : fixed axf/elf to binary conversion warning about -output being 
#        depricated
#  13   mpeg      1.12        1/29/04 2:57:39 PM     Miles Bintz     CR(s) 8307
#         : fix linker script from rom files
#  12   mpeg      1.11        1/28/04 4:13:43 PM     Xin Golden      CR(s) 8299
#         : added .rodata section in gcc_rom_link.scr to avoid section .data 
#        and .rodata overlap.
#  11   mpeg      1.10        11/4/03 4:49:17 PM     Miles Bintz     CR(s): 
#        7801 fix attempted appending of cable modem images onto codeldr
#  10   mpeg      1.9         10/21/03 10:26:50 AM   Larry Wang      CR(s): 
#        7673 (1) Add _text_start so that it can be referenced in the code; (2)
#         Make end of data section word aligned so that the ROM image is 
#        multiple of word in size.
#        
#        
#  9    mpeg      1.8         10/17/03 10:39:44 AM   Larry Wang      CR(s): 
#        7673 In gcc_tom_link.scr, add .glue_7t and .glue_7 to avoid section 
#        overlapping.
#  8    mpeg      1.7         10/1/03 11:41:48 AM    Miles Bintz     SCR(s) 
#        7593 :
#        changed the map file from _ram to _rom for rom outputs
#        
#        
#  7    mpeg      1.6         9/19/03 7:05:18 PM     Miles Bintz     SCR(s) 
#        7484 :
#        added support for semihosting apps
#        
#  6    mpeg      1.5         9/18/03 5:50:58 PM     Miles Bintz     SCR(s) 
#        7484 :
#        added flags when calling binary conversion tool
#        
#  5    mpeg      1.4         9/17/03 12:43:12 PM    Dave Wilson     SCR(s) 
#        7485 :
#        Moved APP_EXTRA_LIBS and MORE_EXTRA_LIBS to the bottom of the linker 
#        script.
#        These were, previously, higher up and this caused problems when 
#        linking our
#        OpenTV 1.2 image (OTV12CTL) where we override a couple of the 
#        functions in
#        the OpenTV libraries with our own versions. Now the linker should find
#         the
#        override versions first and link correctly rather than complaining 
#        about
#        multiply defined symbols. This problem was only seen when using SDT 
#        rather
#        than ADS toolchain.
#        
#  4    mpeg      1.3         9/12/03 5:52:40 PM     Miles Bintz     SCR(s) 
#        7291 :
#        appended checksum operation to end of binary generation
#        
#  3    mpeg      1.2         9/11/03 7:41:00 PM     Miles Bintz     SCR(s) 
#        7291 :
#        defined armlib at link time 
#        
#  2    mpeg      1.1         9/10/03 6:16:44 PM     Bobby Bradford  SCR(s) 
#        7291 :
#        Move modem library check from modem.mak to link.mak
#        
#  1    mpeg      1.0         9/9/03 4:40:44 PM      Miles Bintz     
# $
###############################################################################

