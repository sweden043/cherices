##############################################################################
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
drv_incl_clean:
	-@del /F /Q $(SDCAPP_OUTPUT_DIR)\*.h > nul: 2>&1
	
drv_clean:
  @echo Cleaning module $(DRIVER) . . .
  -$(AT)if exist $(SABINE_ROOT)\$(DRIVER)\clean_gasp_files.bat $(SABINE_ROOT)\$(DRIVER)\clean_gasp_files.bat 
  -$(AT)if exist $(SABINE_ROOT)\$(DRIVER)\clean_gasp_files.bat del $(SABINE_ROOT)\$(DRIVER)\clean_gasp_files.bat 
  -$(AT)del /s /q /f $(SABINE_ROOT)\$(DRIVER)\$(INTER_OUTPUT_DIR)\*.o32 > nul: 2> nul:
  -$(AT)del /s /q /f $(SABINE_ROOT)\$(DRIVER)\$(INTER_OUTPUT_DIR)\*.o16 > nul: 2> nul:
  -$(AT)del /s /q /f $(SABINE_ROOT)\$(DRIVER)\$(INTER_OUTPUT_DIR)\*.d32 > nul: 2> nul:
  -$(AT)del /s /q /f $(SABINE_ROOT)\$(DRIVER)\$(INTER_OUTPUT_DIR)\*.d16 > nul: 2> nul:
  -$(AT)del /s /q /f $(SABINE_ROOT)\$(DRIVER)\$(INTER_OUTPUT_DIR)\*.bak > nul: 2> nul:
  -$(AT)del /s /q /f $(SABINE_ROOT)\$(DRIVER)\$(INTER_OUTPUT_DIR)\*.opt > nul: 2> nul:
  -$(AT)del /s /q /f $(SABINE_ROOT)\$(DRIVER)\$(INTER_OUTPUT_DIR)\*.out > nul: 2> nul:
  -$(AT)del /F /Q $(LIB_OUTPUT_DIR)\$(DRIVER).lib > nul: 2>&1
  -$(AT)del /F /Q $(SABINE_ROOT)\$(CONFIG)_$(APPNAME).log  > nul: 2>&1

sdcdrv_clean:
  @echo Cleaning module SDCAPPS\$(SDCDRIVER) . . .
  -@if exist $(SDCAPPS_ROOT)\$(SDCDRIVER)\clean_gasp_files.bat $(SDCAPPS_ROOT)\$(SDCDRIVER)\clean_gasp_files.bat 
  -@if exist $(SDCAPPS_ROOT)\$(SDCDRIVER)\clean_gasp_files.bat del $(SDCAPPS_ROOT)\$(SDCDRIVER)\clean_gasp_files.bat 
  -@del /s /q /f $(SDCAPPS_ROOT)\$(SDCDRIVER)\$(INTER_OUTPUT_DIR)\*.o32 > nul: 2> nul:
  -@del /s /q /f $(SDCAPPS_ROOT)\$(SDCDRIVER)\$(INTER_OUTPUT_DIR)\*.o16 > nul: 2> nul:
  -@del /s /q /f $(SDCAPPS_ROOT)\$(SDCDRIVER)\$(INTER_OUTPUT_DIR)\*.d32 > nul: 2> nul:
  -@del /s /q /f $(SDCAPPS_ROOT)\$(SDCDRIVER)\$(INTER_OUTPUT_DIR)\*.d16 > nul: 2> nul:
  -@del /s /q /f $(SDCAPPS_ROOT)\$(SDCDRIVER)\$(INTER_OUTPUT_DIR)\*.bak > nul: 2> nul:
  -@del /s /q /f $(SDCAPPS_ROOT)\$(SDCDRIVER)\$(INTER_OUTPUT_DIR)\*.opt > nul: 2> nul:
  -@del /s /q /f $(SDCAPPS_ROOT)\$(SDCDRIVER)\$(INTER_OUTPUT_DIR)\*.out > nul: 2> nul:
  -@del /s /q /f $(SDCAPPS_ROOT)\$(SDCDRIVER)\$(INTER_OUTPUT_DIR)\*.log > nul: 2> nul:
  -@del /F /Q $(LIB_OUTPUT_DIR)\$(SDCDRIVER).lib > nul: 2>&1
  -@del /F /Q $(SDCAPPS_ROOT)\$(CONFIG)_$(APPNAME).log  > nul: 2>&1


clean_app_image: 
  -@del /s /q /f $(IMG_OUTPUT_DIR)\*.axf > nul: 2> nul:
  -@del /s /q /f $(IMG_OUTPUT_DIR)\*.bin > nul: 2> nul:
  -@del /s /q /f $(IMG_OUTPUT_DIR)\*.map > nul: 2> nul:
  -@del /s /q /f $(LIB_OUTPUT_DIR)\*.log > nul: 2> nul:
	-@del /s /q /f $(LIB_OUTPUT_DIR)\link.scr > nul: 2> nul:
#****************************************************************************
# $Log:
#  2    mpeg      1.1         7/9/2004 4:41:28 PM    Xiao Guang Yan  CR(s) 9699
#       9700 : Added clean generated drv_incl.h and bld_ver.h. Also clean log
#       files.
#  1    mpeg      1.0         7/5/2004 7:06:56 PM    Ford Fu         CR(s) 9660
#       9659 : SDC APPLICATION check in, make files 
# $
#****************************************************************************


