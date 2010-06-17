############################################################
# Translate the CONFIG file(s) into the appropriate header #
# files.                                                   #
############################################################

do_cfgtoh: do_cfgtoh_dirs \
	$(SABINE_ROOT)\include\$(CONFIG)\$(SWCONFIG)\boxcfg.h \
	$(SABINE_ROOT)\include\$(CONFIG)\$(SWCONFIG)\boxcfg.vxa \
	$(SABINE_ROOT)\include\$(CONFIG)\$(SWCONFIG)\boxcfg.a \
	$(SABINE_ROOT)\include\hwopts.h \
	$(SABINE_ROOT)\include\hwopts.vxa \
	$(SABINE_ROOT)\include\hwopts.a \
	$(SABINE_ROOT)\include\swopts.h \
	$(SABINE_ROOT)\include\swopts.vxa \
	$(SABINE_ROOT)\include\swopts.a \
	$(SABINE_ROOT)\include\hwdefault.h \
	$(SABINE_ROOT)\include\hwdefault.vxa \
	$(SABINE_ROOT)\include\hwdefault.a \
	$(SABINE_ROOT)\include\swdefault.h \
	$(SABINE_ROOT)\include\swdefault.vxa \
	$(SABINE_ROOT)\include\swdefault.a \
	$(SABINE_ROOT)\include\hwconfig.html \
	$(SABINE_ROOT)\include\swconfig.html \
!if "$(SWCONFIG)" != "CNXT"
	$(SABINE_ROOT)\include\$(CONFIG)\$(SWCONFIG)\swboxcfg.h \
	$(SABINE_ROOT)\include\$(CONFIG)\$(SWCONFIG)\swboxcfg.vxa \
	$(SABINE_ROOT)\include\$(CONFIG)\$(SWCONFIG)\swboxcfg.a \
!endif


do_cfgtoh_dirs:
!if exist ($(SABINE_ROOT)\include\$(CONFIG)\$(SWCONFIG)) == 0
	-@if not exist $(SABINE_ROOT)\include\$(CONFIG)\nul mkdir $(SABINE_ROOT)\include\$(CONFIG)
	-@if not exist $(SABINE_ROOT)\include\$(CONFIG)\$(SWCONFIG)\nul mkdir $(SABINE_ROOT)\include\$(CONFIG)\$(SWCONFIG)
!endif


##############################################################################
# Rules to build C and asm option and default header files from HWCONFIG.CFG #
##############################################################################
$(SABINE_ROOT)\include\hwopts.h: $(CONFIG_ROOT)\hwconfig.cfg
	@$(TOOLEXE_PATH)\cfgtoh -q -y -s -i$** -c$@ -1h -pHWOPTS 
	
$(SABINE_ROOT)\include\hwdefault.h: $(CONFIG_ROOT)\hwconfig.cfg
	@$(TOOLEXE_PATH)\cfgtoh -q -y -d -w -x -i$** -1h -c$@ -pHWDEFAULT 

$(SABINE_ROOT)\include\hwopts.a: $(CONFIG_ROOT)\hwconfig.cfg
	@$(TOOLEXE_PATH)\cfgtoh -q -y -s -i$** -a$@ -2a -pHWOPTS 
	
$(SABINE_ROOT)\include\hwdefault.a: $(CONFIG_ROOT)\hwconfig.cfg
	@$(TOOLEXE_PATH)\cfgtoh -q -y -d -w -x -i$** -a$@ -2a -pHWDEFAULT 

$(SABINE_ROOT)\include\hwopts.vxa: $(CONFIG_ROOT)\hwconfig.cfg
	@$(TOOLEXE_PATH)\cfgtoh -q -y -s -i$** -v$@ -3vxa -pHWOPTS 
	
$(SABINE_ROOT)\include\hwdefault.vxa: $(CONFIG_ROOT)\hwconfig.cfg
	@$(TOOLEXE_PATH)\cfgtoh -q -y -d -w -x -i$** -v$@ -3vxa -pHWDEFAULT 

$(SABINE_ROOT)\include\hwconfig.html: $(CONFIG_ROOT)\hwconfig.cfg
	@$(TOOLEXE_PATH)\cfgtoh -q -y -w -x -i$** -m$@ -3html

##############################################################################
# Rules to build C and asm option and default header files from SWCONFIG.CFG #
##############################################################################
$(SABINE_ROOT)\include\swopts.h: $(CONFIG_ROOT)\swconfig.cfg
	@$(TOOLEXE_PATH)\cfgtoh -q -y -s -i$** -c$@ -1h -pSWOPTS 
	
$(SABINE_ROOT)\include\swdefault.h: $(CONFIG_ROOT)\swconfig.cfg
	@$(TOOLEXE_PATH)\cfgtoh -q -y -d -w -x -i$** -1h -c$@ -pSWDEFAULT 

$(SABINE_ROOT)\include\swopts.a: $(CONFIG_ROOT)\swconfig.cfg
	@$(TOOLEXE_PATH)\cfgtoh -q -y -s -i$** -a$@ -2a -pSWOPTS 
	
$(SABINE_ROOT)\include\swdefault.a: $(CONFIG_ROOT)\swconfig.cfg
	@$(TOOLEXE_PATH)\cfgtoh -q -y -d -w -x -i$** -a$@ -2a -pSWDEFAULT 

$(SABINE_ROOT)\include\swopts.vxa: $(CONFIG_ROOT)\swconfig.cfg
	@$(TOOLEXE_PATH)\cfgtoh -q -y -s -i$** -v$@ -3vxa -pSWOPTS 
	
$(SABINE_ROOT)\include\swdefault.vxa: $(CONFIG_ROOT)\swconfig.cfg
	@$(TOOLEXE_PATH)\cfgtoh -q -y -d -w -x -i$** -v$@ -3vxa -pSWDEFAULT 

$(SABINE_ROOT)\include\swconfig.html: $(CONFIG_ROOT)\swconfig.cfg
	@$(TOOLEXE_PATH)\cfgtoh -q -y -w -x -i$** -m$@ -3html

#################################################################################
# Rules to build C and asm headers from this build's specific hw and sw config  #
#################################################################################
$(SABINE_ROOT)\include\$(CONFIG)\$(SWCONFIG)\boxcfg.h: $(CONFIG_ROOT)\$(CONFIG).cfg
	@$(TOOLEXE_PATH)\cfgtoh -q -y -i$** -c$@ -1h -n$(SABINE_ROOT)\include\$(CONFIG)\$(SWCONFIG)\chiphdr


$(SABINE_ROOT)\include\$(CONFIG)\$(SWCONFIG)\boxcfg.vxa: $(CONFIG_ROOT)\$(CONFIG).cfg
	@$(TOOLEXE_PATH)\cfgtoh -q -y -i$** -v$@ -3vxa -n$(SABINE_ROOT)\include\$(CONFIG)\$(SWCONFIG)\chiphdr


$(SABINE_ROOT)\include\$(CONFIG)\$(SWCONFIG)\boxcfg.a: $(CONFIG_ROOT)\$(CONFIG).cfg
	@$(TOOLEXE_PATH)\cfgtoh -q -y -i$** -a$@ -2a -n$(SABINE_ROOT)\include\$(CONFIG)\$(SWCONFIG)\chiphdr


$(SABINE_ROOT)\include\$(CONFIG)\$(SWCONFIG)\swboxcfg.h: $(CONFIG_ROOT)\$(SWCONFIG).cfg
	@$(TOOLEXE_PATH)\cfgtoh -q -y -s -i$** -c$@ -1h


$(SABINE_ROOT)\include\$(CONFIG)\$(SWCONFIG)\swboxcfg.vxa: $(CONFIG_ROOT)\$(SWCONFIG).cfg
	@$(TOOLEXE_PATH)\cfgtoh -q -y -s -i$** -v$@ -3vxa


$(SABINE_ROOT)\include\$(CONFIG)\$(SWCONFIG)\swboxcfg.a: $(CONFIG_ROOT)\$(SWCONFIG).cfg
	@$(TOOLEXE_PATH)\cfgtoh -q -y -s -i$** -a$@ -2a



