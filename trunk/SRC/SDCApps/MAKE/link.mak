###############################################################################
#                                                                             #
# CNXT MPEG build support file: cdefs.mak                                     #
#                                                                             #
# Author: M. Bintz 7/24/03                                                    #
#                                                                             #
# Compiler rules for all toolchains and file types.                           #
#                                                                             #
###############################################################################

############################################
# Add IPANEL_LIB into project.
############################################
#IPANEL_LIB =
IPANEL_LIB = $(SABINE_ROOT)\SDCApps\IPANEL\lib_ipanel_ARM.obj
#IPANEL_CNXT_TFCA_LIB = 
IPANEL_CNXT_TFCA_LIB = $(SABINE_ROOT)\SDCApps\TFCA\cas21.LIB

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
	$(IMG_OUTPUT_DIR)\$(APPNAME)_RAM.BIN 
#	$(IMG_OUTPUT_DIR)\$(APPNAME)_ROM.BIN

!else  # SEMIHOSTING_APP

app_link: $(LIB_OUTPUT_DIR)\$(APPNAME)_semi.AXF

!endif



$(IMG_OUTPUT_DIR)\$(APPNAME)_RAM.BIN: $(IMG_OUTPUT_DIR)\$(APPNAME)_ram.$(EXT_IMG)
	-@echo Generating binary for RAM image
!if "$(ARM_TOOLKIT)" == "WRGCC" || "$(ARM_TOOLKIT)" == "GNUGCC"
	-$(AT)$(WR_BIN_PATH)\objcopyarm $** -O binary $@
!else
	-$(AT)$(TOBIN) $(TO_BIN_FLAGS) $** -bin -output $(TO_BIN_OUTPUT) $@    
!endif
	@echo Adding checksum to binary image
!if "$(IMAGE_TYPE)" != "BOOT"
!if "$(IMAGE_TYPE)" == "OPENTV_12"
	$(AT)$(TOOLEXE_PATH)\chksum12 $@
!else
	$(AT)$(TOOLEXE_PATH)\chksum -i $@
!endif
!endif
  -@echo Generating GZ image
  -@$(SABINE_ROOT)\GZ\zlib.exe $(IMG_OUTPUT_DIR)\$(APPNAME)_ram.bin
  -@echo Generating LZO image, then SDL file at last
  -@$(TOOLEXE_PATH)\lzopack.exe -9 $(IMG_OUTPUT_DIR)\$(APPNAME)_ram.bin 0x00410000


#$(IMG_OUTPUT_DIR)\$(APPNAME)_ROM.BIN: $(IMG_OUTPUT_DIR)\$(APPNAME)_rom.$(EXT_IMG)
#	-@echo Generating binary for ROM image 
#	-$(AT)$(DBGTOBIN) 
#!if "$(ARM_TOOLKIT)" == "WRGCC" || "$(ARM_TOOLKIT)" == "GNUGCC"
#	-$(AT)$(WR_BIN_PATH)\objcopyarm $** -O binary $@
#!else
#	-$(AT)$(TOBIN) $(TO_BIN_FLAGS) $** -bin -output $(TO_BIN_OUTPUT) $@    
#!endif
#	@echo Adding checksum to binary image
#!if "$(IMAGE_TYPE)" != "BOOT"
#!if "$(IMAGE_TYPE)" == "OPENTV_12"
#	$(AT)$(TOOLEXE_PATH)\chksum12 $@
#!else
#	$(AT)$(TOOLEXE_PATH)\chksum -i $@
#!endif
#!endif

$(IMG_OUTPUT_DIR)\$(APPNAME)_semi.AXF: $(LIB_OUTPUT_DIR)\link_semi.scr
	-@cd $(LIB_OUTPUT_DIR)
	-@echo Linking (semihosted)  $(@F)
	-@set ARMLIB=$(SEMIHOSTLIB)
	-$(AT)$(ALINK) $(LOPTS1) $(AXFLINK_LOG) -ro-Base $(LINKER_RAM_BASE) \
	$(LINK_MAP_OUTPUT) $(IMG_OUTPUT_DIR)\$(APPNAME)_ram_semi.map \
	$(LINK_DEBUG) \
	$(EXTRA_LINK_FLAGS) -elf -o $@ -via $(LIB_OUTPUT_DIR)\link_semi.scr > $(LIB_OUTPUT_DIR)\$(APPNAME)_ram_semi.log 2>&1
	-@type $(LIB_OUTPUT_DIR)\$(APPNAME)_ram_semi.log 


$(IMG_OUTPUT_DIR)\$(APPNAME)_RAM.AXF: $(LIB_OUTPUT_DIR)\link.scr
	-$(AT)cd $(LIB_OUTPUT_DIR)
	-@echo Linking    $(@F)
	-@set ARMLIB=$(ARMLIB)
	-$(AT)$(ALINK) $(LOPTS1) $(AXFLINK_LOG) -ro-Base $(LINKER_RAM_BASE) \
	$(LINK_MAP_OUTPUT) $(IMG_OUTPUT_DIR)\$(APPNAME)_ram.map \
	$(LINK_DEBUG) \
	$(EXTRA_LINK_FLAGS) -elf -o $@ -via $(LIB_OUTPUT_DIR)\link.scr \
	$(IPANEL_LIB) $(IPANEL_CNXT_TFCA_LIB)>$(LIB_OUTPUT_DIR)\$(APPNAME)_ram.log 2>&1
	-@type $(LIB_OUTPUT_DIR)\$(APPNAME)_ram.log 

#$(IMG_OUTPUT_DIR)\$(APPNAME)_ROM.AXF: $(LIB_OUTPUT_DIR)\link.scr
#	-@cd $(LIB_OUTPUT_DIR)
#	-@echo Linking    $(@F)
#	-@set ARMLIB=$(ARMLIB)
#	-$(AT)$(ALINK) $(LOPTS1) $(RDBLINK_LOG) \
#	-RO $(LINKER_ROM_BASE) -RW $(LINKER_RAM_BASE) \
#	$(LINK_MAP_OUTPUT) $(IMG_OUTPUT_DIR)\$(APPNAME)_rom.map \
#	$(LINK_DEBUG) \
#	$(EXTRA_LINK_FLAGS) -elf -o $@ -via $(LIB_OUTPUT_DIR)\link.scr \
#	$(IPANEL_LIB) $(IPANEL_CNXT_TFCA_LIB)>$(LIB_OUTPUT_DIR)\$(APPNAME)_rom.log 2>&1
#	-@type $(LIB_OUTPUT_DIR)\$(APPNAME)_rom.log 


$(LIB_OUTPUT_DIR)\link_semi.scr $(LIB_OUTPUT_DIR)\link.scr: always
	-@echo Generating Linker script...
	-@del /F /Q $@ > nul: 2>&1
!if "$(ENTRY_OBJ)" != ""
	@echo $(ENTRY_OBJ)                       >> $@
!endif
!if "$(OS_OBJS)" != ""
	@echo $(OS_OBJS)                        >> $@
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
!if "$(SDCAPP_LIBS)" != ""
	@echo $(SDCAPP_LIBS)                   >> $@
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


#$(IMG_OUTPUT_DIR)\$(APPNAME)_ROM.ELF: $(LIB_OUTPUT_DIR)\gcc_rom_link.scr
#	-$(AT)cd $(LIB_OUTPUT_DIR)
#	-$(AT)echo Linking    $(@F)
#	-$(AT)$(ALINK) $(LOPTS1) $(LINK_MAP_OUTPUT) $(IMG_OUTPUT_DIR)\$(APPNAME)_rom.map -T $(LIB_OUTPUT_DIR)\gcc_rom_link.scr -o $@ > $(LIB_OUTPUT_DIR)\$(APPNAME)_rom.log 2>&1
#	-@type $(LIB_OUTPUT_DIR)\$(APPNAME)_rom.log 

$(IMG_OUTPUT_DIR)\$(APPNAME)_RAM.ELF: $(LIB_OUTPUT_DIR)\gcc_ram_link.scr
	-@cd $(LIB_OUTPUT_DIR)
	-@echo Linking    $(@F)
	-$(AT)$(ALINK) $(LOPTS1) $(LINK_MAP_OUTPUT) $(IMG_OUTPUT_DIR)\$(APPNAME)_ram.map -T $(LIB_OUTPUT_DIR)\gcc_ram_link.scr -o $@ > $(LIB_OUTPUT_DIR)\$(APPNAME)_ram.log 2>&1
	-@type $(LIB_OUTPUT_DIR)\$(APPNAME)_ram.log 


$(LIB_OUTPUT_DIR)\gcc_link.scr: always
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

$(LIB_OUTPUT_DIR)\gcc_ram_link.scr: $(LIB_OUTPUT_DIR)\gcc_link.scr
	@echo INCLUDE  $(LIB_OUTPUT_DIR)\gcc_link.scr                            >  $@
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

#$(LIB_OUTPUT_DIR)\gcc_rom_link.scr: $(LIB_OUTPUT_DIR)\gcc_link.scr 
#	@echo INCLUDE  $(LIB_OUTPUT_DIR)\gcc_link.scr                            >  $@
#	@echo.                                                               >> $@
#	@echo SECTIONS                                                       >> $@
#	@echo {                                                              >> $@
#	@echo .text $(LINKER_ROM_BASE) :                                     >> $@
#	@echo { _text_start = . ; *(.text) *(.text.*)    					 >> $@
#	@echo   *(.glue_7t) *(.glue_7) }                                     >> $@
#	@echo .rodata  BLOCK(0x1000) : { _ro_start = . ; *(.rodata) *(.rodata.*) *(.gnu.linkonce.r*) }   >> $@
#	@echo _text_end = . ; _etext = . ;                                   >> $@
#	@echo PROVIDE (etext = .);                                           >> $@
#	@echo .data $(LINKER_RAM_BASE) :                                     >> $@
#	@echo AT ( ADDR (.rodata ) + SIZEOF (.rodata) )                       >> $@
#	@echo { _data_start = . ; *(.data) *(.data.*) . = ALIGN(32 / 8) ; _data_end = . ;  }   >> $@
#	@echo _edata = .;                                                    >> $@
#	@echo PROVIDE (edata = .);                                           >> $@
#	@echo _bss_start   = .;                                              >> $@
#	@echo __bss_start   = .;                                             >> $@
#	@echo __bss_start__ = .;                                             >> $@
#	@echo .sbss      : { *(.sbss) *(.scommon) }                          >> $@
#	@echo .bss :                                                         >> $@
#	@echo { *(.bss) *(COMMON)                                            >> $@
#	@echo   . = ALIGN(32 / 8);                                           >> $@
#	@echo }                                                              >> $@
#	@echo . = ALIGN(32 / 8);                                             >> $@
#	@echo _end = . ; __end__ = . ;                                       >> $@
#	@echo _bss_end = . ; _bss_end__ = . ; __bss_end__ = . ;              >> $@
#	@echo PROVIDE (end = .);                                             >> $@
#	@echo .stab 0 : { *(.stab) }                            >> $@
#	@echo .stabstr 0 : { *(.stabstr) }                      >> $@
#	@echo .stab.excl 0 : { *(.stab.excl) }                  >> $@
#	@echo .stab.exclstr 0 : { *(.stab.exclstr) }            >> $@
#	@echo .stab.index 0 : { *(.stab.index) }                >> $@
#	@echo .stab.indexstr 0 : { *(.stab.indexstr) }          >> $@
#	@echo .comment 0 : { *(.comment) }                      >> $@
#	@echo /* DWARF 1 */                                     >> $@
#	@echo .debug          0 : { *(.debug) }                 >> $@
#	@echo .line           0 : { *(.line) }                  >> $@
#	@echo /* GNU DWARF 1 extensions */                      >> $@
#	@echo .debug_srcinfo  0 : { *(.debug_srcinfo) }         >> $@
#	@echo .debug_sfnames  0 : { *(.debug_sfnames) }         >> $@
#	@echo /* DWARF 1.1 and DWARF 2 */                       >> $@
#	@echo .debug_aranges  0 : { *(.debug_aranges) }         >> $@
#	@echo .debug_pubnames 0 : { *(.debug_pubnames) }        >> $@
#	@echo /* DWARF 2 */                                     >> $@
#	@echo .debug_info     0 : { *(.debug_info) }            >> $@
#	@echo .debug_abbrev   0 : { *(.debug_abbrev) }          >> $@
#	@echo .debug_line     0 : { *(.debug_line) }            >> $@
#	@echo .debug_frame    0 : { *(.debug_frame) }           >> $@
#	@echo .debug_str      0 : { *(.debug_str) }             >> $@
#	@echo .debug_loc      0 : { *(.debug_loc) }             >> $@
#	@echo .debug_macinfo  0 : { *(.debug_macinfo) }         >> $@
#	@echo /* SGI/MIPS DWARF 2 extensions */                 >> $@
#	@echo .debug_weaknames 0 : { *(.debug_weaknames) }      >> $@
#	@echo .debug_funcnames 0 : { *(.debug_funcnames) }      >> $@
#	@echo .debug_typenames 0 : { *(.debug_typenames) }      >> $@
#	@echo .debug_varnames  0 : { *(.debug_varnames) }       >> $@
#	@echo }                                                 >> $@



###############################################################################
# $Log:
#  1    mpeg      1.0         7/5/2004 7:10:06 PM    Ford Fu         CR(s) 9660
#       9659 : SDC APPLICATION check in, make files 
# $
###############################################################################

