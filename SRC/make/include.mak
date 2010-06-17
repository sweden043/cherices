###############################################################################
#                                                                             #
# CNXT MPEG build support file: include.mak                                   #
#                                                                             #
# Author: M. Bintz 7/24/03                                                    #
#                                                                             #
# Compiler rules for all toolchains and file types.                           #
#                                                                             #
###############################################################################
.SUFFIXES: .h
.IGNORE:

###########################################################################
#                                                                         # 
# This is the list of dependants.  The lib <- o16/o32 <- .a/.c            #
# These are all based on the [INSTRSET]_[FTYPE]_FILES macros....          #
#                                                                         # 
###########################################################################
# Get our list of files to build and our dependencies
!include $(SABINE_ROOT)\make\product.mak
!include $(SABINE_ROOT)\make\envcheck.mak
!include $(SABINE_ROOT)\$(DRIVER)\$(DRIVER).MAK


copy_headers:  
!if "$(PUBLIC_HEADERS)"!=""
	@for %i in ($(PUBLIC_HEADERS)) DO @xcopy /K /R /Y $(SABINE_ROOT)\$(DRIVER)\%i $(SABINE_ROOT)\include >> $(COMBINED_ERROR_LOG) 2>&1
!else
!endif

clean_headers:  
!if "$(PUBLIC_HEADERS)"!=""
	@for %i in ($(PUBLIC_HEADERS)) DO @del /F $(SABINE_ROOT)\include\%i > nul: 2>&1
!else
!endif

release_headers:  
!if "$(PUBLIC_HEADERS)"!=""
	@for %i in ($(PUBLIC_HEADERS)) DO @xcopy /K /R /Y $(SABINE_ROOT)\$(DRIVER)\%i $(SABINE_ROOT)\include > nul: 2>&1
	@for %i in ($(PUBLIC_HEADERS)) DO @del /F $(SABINE_ROOT)\$(DRIVER)\%i > nul: 2>&1
!else
!endif

find_headers:
!if "$(PUBLIC_HEADERS)"!=""
	@for %i in ($(PUBLIC_HEADERS)) DO @echo PUBLIC_HEADER %i   originates from driver $(DRIVER)
!endif

all:


#****************************************************************************
#* $Log: 
#*  3    mpeg      1.2         11/4/03 2:54:42 PM     Miles Bintz     CR(s): 
#*        7825 modified makefiles to generate more useful information in logs
#*  2    mpeg      1.1         9/26/03 3:02:28 PM     Miles Bintz     SCR(s) 
#*        7551 :
#*        header files should be copied with their attributes remaining the 
#*        same as the original
#*        
#*  1    mpeg      1.0         9/25/03 3:51:26 PM     Miles Bintz     
#* $
#****************************************************************************


