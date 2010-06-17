###############################################################################
#                                                                             #
# CNXT MPEG build support file: clean.mak                                     #
#                                                                             #
# Author: M. Bintz 7/24/03                                                    #
#                                                                             #
# Compiler rules for all toolchains and file types.                           #
#                                                                             #
###############################################################################


do_cfgtoh_clean:
   -@if exist $(SABINE_ROOT)\include\$(CONFIG)\$(SWCONFIG)\boxcfg.h del $(SABINE_ROOT)\include\$(CONFIG)\$(SWCONFIG)\boxcfg.h
   -@if exist $(SABINE_ROOT)\include\$(CONFIG)\$(SWCONFIG)\boxcfg.a del $(SABINE_ROOT)\include\$(CONFIG)\$(SWCONFIG)\boxcfg.a
   -@if exist $(SABINE_ROOT)\include\$(CONFIG)\$(SWCONFIG)\boxcfg.vxa del $(SABINE_ROOT)\include\$(CONFIG)\$(SWCONFIG)\boxcfg.vxa
   -@if exist $(SABINE_ROOT)\include\$(CONFIG)\$(SWCONFIG)\swboxcfg.h del $(SABINE_ROOT)\include\$(CONFIG)\$(SWCONFIG)\swboxcfg.h
   -@if exist $(SABINE_ROOT)\include\$(CONFIG)\$(SWCONFIG)\swboxcfg.a del $(SABINE_ROOT)\include\$(CONFIG)\$(SWCONFIG)\swboxcfg.a
   -@if exist $(SABINE_ROOT)\include\$(CONFIG)\$(SWCONFIG)\swboxcfg.vxa del $(SABINE_ROOT)\include\$(CONFIG)\$(SWCONFIG)\swboxcfg.vxa
   -@if exist $(SABINE_ROOT)\include\$(CONFIG)\$(SWCONFIG)\chiphdr.h del $(SABINE_ROOT)\include\$(CONFIG)\$(SWCONFIG)\chiphdr.h
   -@if exist $(SABINE_ROOT)\include\$(CONFIG)\$(SWCONFIG)\chiphdr.a del $(SABINE_ROOT)\include\$(CONFIG)\$(SWCONFIG)\chiphdr.a
   -@if exist $(SABINE_ROOT)\include\$(CONFIG)\$(SWCONFIG)\chiphdr.vxa del $(SABINE_ROOT)\include\$(CONFIG)\$(SWCONFIG)\chiphdr.vxa
   -@if exist $(SABINE_ROOT)\include\hwopts.h del $(SABINE_ROOT)\include\hwopts.h
   -@if exist $(SABINE_ROOT)\include\swopts.h del $(SABINE_ROOT)\include\swopts.h
   -@if exist $(SABINE_ROOT)\include\hwdefault.h del $(SABINE_ROOT)\include\hwdefault.h
   -@if exist $(SABINE_ROOT)\include\swdefault.h del $(SABINE_ROOT)\include\swdefault.h
   -@if exist $(SABINE_ROOT)\include\hwopts.a del $(SABINE_ROOT)\include\hwopts.a
   -@if exist $(SABINE_ROOT)\include\swopts.a del $(SABINE_ROOT)\include\swopts.a
   -@if exist $(SABINE_ROOT)\include\hwdefault.a del $(SABINE_ROOT)\include\hwdefault.a
   -@if exist $(SABINE_ROOT)\include\swdefault.a del $(SABINE_ROOT)\include\swdefault.a
   -@if exist $(SABINE_ROOT)\include\hwopts.vxa del $(SABINE_ROOT)\include\hwopts.vxa
   -@if exist $(SABINE_ROOT)\include\swopts.vxa del $(SABINE_ROOT)\include\swopts.vxa
   -@if exist $(SABINE_ROOT)\include\hwdefault.vxa del $(SABINE_ROOT)\include\hwdefault.vxa
   -@if exist $(SABINE_ROOT)\include\swdefault.vxa del $(SABINE_ROOT)\include\swdefault.vxa
   -@if exist $(SABINE_ROOT)\include\hwconfig.html del $(SABINE_ROOT)\include\hwconfig.html
   -@if exist $(SABINE_ROOT)\include\swconfig.html del $(SABINE_ROOT)\include\swconfig.html

	
#################################################
# Delete all build output files for this driver #
#################################################
drv_clean:
  @echo CLEANING for module $(DRIVER) . . .
  -@if exist $(SABINE_ROOT)\$(DRIVER)\clean_gasp_files.bat $(SABINE_ROOT)\$(DRIVER)\clean_gasp_files.bat 
  -@if exist $(SABINE_ROOT)\$(DRIVER)\clean_gasp_files.bat del $(SABINE_ROOT)\$(DRIVER)\clean_gasp_files.bat 
  -@del /q /f $(OUTPUT_DIR)\* > nul: 2> nul:
  -@del /s /q /f $(SABINE_ROOT)\$(DRIVER)\*.o32 > nul: 2> nul:
  -@del /s /q /f $(SABINE_ROOT)\$(DRIVER)\*.o16 > nul: 2> nul:
  -@del /s /q /f $(SABINE_ROOT)\$(DRIVER)\*.d32 > nul: 2> nul:
  -@del /s /q /f $(SABINE_ROOT)\$(DRIVER)\*.d16 > nul: 2> nul:
  -@del /s /q /f $(SABINE_ROOT)\$(DRIVER)\*.bak > nul: 2> nul:
  -@del /s /q /f $(SABINE_ROOT)\$(DRIVER)\*.opt > nul: 2> nul:
  -@del /s /q /f $(SABINE_ROOT)\$(DRIVER)\*.out > nul: 2> nul:
  -@rd /s /q $(OUTPUT_DIR) > nul: 2>&1
  -@del /F /Q $(BLD_LIB_DIR)\$(DRIVER).lib > nul: 2>&1
  -@del /F /Q $(SABINE_ROOT)\$(CONFIG)_$(APPNAME).log  > nul: 2>&1




#****************************************************************************
# $Log: 
#  4    mpeg      1.3         11/4/03 2:51:09 PM     Miles Bintz     CR(s): 
#        7825 modified makefiles to generate more useful information in logs
#  3    mpeg      1.2         9/16/03 7:27:04 PM     Miles Bintz     SCR(s) 
#        7291 :
#        fixed up builds for ucos
#        
#  2    mpeg      1.1         9/16/03 4:22:22 PM     QA - Roger Taylor SCR(s) 
#        7291 :
#        when doing a clean, clean only the OUTPUT_DIR and not everything from 
#        the inter_output_dir down.  this clobbers the temp\testh files
#        
#  1    mpeg      1.0         9/9/03 4:40:36 PM      Miles Bintz     
# $
#****************************************************************************


