#****************************************************************************
#*                            Conexant Systems
#****************************************************************************
#*
#* Filename:       product.mak
#*
#* Description:    Top Level Product Configuration File for MAKE Files
#*
#* Author:         Bobby Bradford
#*
#* Copyright Conexant Systems, 2001
#* All Rights Reserved.
#*
#****************************************************************************
#* $Header: /cvs/proj-conexant24153/SRC/SDCApps/MAKE/product.mak,v 1.1 2007/10/24 09:39:11 chenhb27 Exp $
#****************************************************************************

!ifndef PRODUCT_MAK

PRODUCT_MAK=1

#****************************************************************************
#* FIRST ... Assign a default value to SWCONFIG, if not specified
#****************************************************************************
!if "$(SWCONFIG)" == ""
SWCONFIG=CNXT
!endif

#****************************************************************************
#* FIRST ... ALWAYS ... Get the HWCONFIG header for hardware level defintions
#****************************************************************************
!include $(CONFIG_ROOT)\$(CONFIG).CFG

#****************************************************************************
#* SECOND ... Get the SWCONFIG header for software level definitions
#****************************************************************************
!if "$(SWCONFIG)" != "CNXT"
!include $(CONFIG_ROOT)\$(SWCONFIG).CFG
!endif

#****************************************************************************
#* THIRD ... Add new high-level configurations here, as they are defined
#****************************************************************************
CONFIG_FILE_DIR = $(SABINE_ROOT)\include\$(CONFIG)\$(SWCONFIG)
TOOLEXE_PATH    = $(SABINE_ROOT)\TOOLEXE

#***************************************************************************
#* Force nmake to build the 2 include files we are about to need. We can't
#* do this using normal inference rules and dependencies since these are 
#* only considered after all the includes and macro definitions have been
#* set up. This forces nmake to build the included files before we actually
#* try to include them.
#***************************************************************************

!if "$(BUILT_CONFIG_HDRS)" != "1"
!if [set BUILT_CONFIG_HDRS=1]
!endif


!if exist ($(CONFIG_FILE_DIR)) == 0
!if [md $(CONFIG_FILE_DIR)]
!endif
!endif

!if [$(TOOLEXE_PATH)\cfgtoh -q -x -y -w -d -t$(CONFIG_FILE_DIR)\hwconfig.mak -i$(CONFIG_ROOT)\HWCONFIG.CFG]
!endif
!if [$(TOOLEXE_PATH)\cfgtoh -q -x -y -w -d -t$(CONFIG_FILE_DIR)\swconfig.mak -i$(CONFIG_ROOT)\SWCONFIG.CFG]
!endif

!endif

#****************************************************************************
#* Include files containing default values for all possible config file keys  
#****************************************************************************
!include $(CONFIG_FILE_DIR)\HWCONFIG.MAK
!include $(CONFIG_FILE_DIR)\SWCONFIG.MAK

#****************************************************************************
#* End of Protective File Wrapper
#****************************************************************************
!endif

#****************************************************************************
#* $Log:
#*  2    mpeg      1.1         7/9/2004 4:56:25 PM    Xiao Guang Yan  CR(s)
#*       9699 9700 : Added protective header.
#*  1    mpeg      1.0         7/5/2004 7:10:37 PM    Ford Fu         CR(s)
#*       9660 9659 : SDC APPLICATION check in, make files 
#* $
#*
#****************************************************************************

