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
#* $Header: product.mak, 11, 9/9/03 4:40:44 PM, Miles Bintz$
#****************************************************************************

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
!include $(SABINE_ROOT)\make\rtos.mak
!include $(SABINE_ROOT)\make\builddirs.mak

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

#****************************************************************************
#* $Log: 
#*  11   mpeg      1.10        9/9/03 4:40:44 PM      Miles Bintz     SCR(s) 
#*        7291 :
#*        updates for reworked build system
#*        
#*  10   mpeg      1.9         3/19/03 4:53:30 PM     Bobby Bradford  SCR(s) 
#*        5798 :
#*        Modified to include extrules.mak to define filename extension
#*        
#*  9    mpeg      1.8         11/20/02 11:40:32 AM   Dave Wilson     SCR(s) 
#*        4904 :
#*        Ensure TOOLEXE_PATH is set and available for use in PRODUCT.MAK.
#*        Explicitly set the path to CFGTOH in PRODUCT.MAK
#*        
#*  8    mpeg      1.7         11/20/02 10:31:04 AM   Dave Wilson     SCR(s) 
#*        4904 :
#*        Last edit had a problem when using TYPE=GET (the include directory 
#*        was not
#*        created before the CFGTOH tool tried to create files in it). This is 
#*        now
#*        fixed.
#*        
#*  7    mpeg      1.6         11/20/02 9:40:06 AM    Dave Wilson     SCR(s) 
#*        4904 :
#*        Added code to generate HWCONFIG.MAK and SWCONFIG.MAK which contain 
#*        hardware
#*        and software configuration default values.
#*        
#*  6    mpeg      1.5         5/30/02 12:29:50 PM    Tim White       SCR(s) 
#*        3899 :
#*        Add new -DDMA_MUX microcode build option support.
#*        
#*        
#*  5    mpeg      1.4         5/21/02 3:47:32 PM     Tim White       SCR(s) 
#*        3642 :
#*        Added PARSER_PTS_OFFSET=NO, added PARSER_PES_PID_INT=YES, changed 
#*        PARSER_BAD_PES_INT=NO to YES, changed DISCARD_BAD_PES=NO to YES, 
#*        changed PARSER_PCR_PID_INT=NO to YES.
#*        
#*        
#*  4    mpeg      1.3         4/30/02 5:18:34 PM     Tim White       SCR(s) 
#*        3664 :
#*        Added -dBAD_PES_INT parser microcode build option support.
#*        
#*        
#*  3    mpeg      1.2         4/26/02 3:13:26 PM     Tim White       SCR(s) 
#*        3562 :
#*        Add support for Colorado Rev_F.  Also convert 1/0 options to YES/NO 
#*        values.
#*        
#*        
#*  2    mpeg      1.1         3/25/02 4:09:18 PM     Tim White       SCR(s) 
#*        3433 :
#*        Added makefile defaults for swconfig build options.  This needs to be
#*         changed, but for
#*        now had to go someplace...
#*        
#*        
#*  1    mpeg      1.0         2/25/02 8:57:56 AM     Bobby Bradford  
#* $
#  
#     Rev 1.10   09 Sep 2003 15:40:44   bintzmf
#  SCR(s) 7291 :
#  updates for reworked build system
#  
#     Rev 1.1   04 Sep 2003 14:04:50   bintzmf
#  
#     Rev 1.0   14 Aug 2003 13:41:48   bintzmf
#  new build system
#  
#     Rev 1.9   19 Mar 2003 16:53:30   bradforw
#  SCR(s) 5798 :
#  Modified to include extrules.mak to define filename extension
#  
#     Rev 1.8   20 Nov 2002 11:40:32   dawilson
#  SCR(s) 4904 :
#  Ensure TOOLEXE_PATH is set and available for use in PRODUCT.MAK.
#  Explicitly set the path to CFGTOH in PRODUCT.MAK
#  
#     Rev 1.7   20 Nov 2002 10:31:04   dawilson
#  SCR(s) 4904 :
#  Last edit had a problem when using TYPE=GET (the include directory was not
#  created before the CFGTOH tool tried to create files in it). This is now
#  fixed.
#  
#     Rev 1.6   20 Nov 2002 09:40:06   dawilson
#  SCR(s) 4904 :
#  Added code to generate HWCONFIG.MAK and SWCONFIG.MAK which contain hardware
#  and software configuration default values.
#  
#     Rev 1.5   30 May 2002 11:29:50   whiteth
#  SCR(s) 3899 :
#  Add new -DDMA_MUX microcode build option support.
#  
#  
#     Rev 1.4   21 May 2002 14:47:32   whiteth
#  SCR(s) 3642 :
#  Added PARSER_PTS_OFFSET=NO, added PARSER_PES_PID_INT=YES, changed PARSER_BAD_PES_INT=NO to YES, changed DISCARD_BAD_PES=NO to YES, changed PARSER_PCR_PID_INT=NO to YES.
#  
#  
#     Rev 1.3   30 Apr 2002 16:18:34   whiteth
#  SCR(s) 3664 :
#  Added -dBAD_PES_INT parser microcode build option support.
#  
#  
#     Rev 1.2   26 Apr 2002 14:13:26   whiteth
#  SCR(s) 3562 :
#  Add support for Colorado Rev_F.  Also convert 1/0 options to YES/NO values.
#  
#  
#     Rev 1.1   25 Mar 2002 16:09:18   whiteth
#  SCR(s) 3433 :
#  Added makefile defaults for swconfig build options.  This needs to be changed, but for
#  now had to go someplace...
#  
#  
#     Rev 1.0   Feb 25 2002 08:57:56   bradforw
#  SCR(s) 3201 :
#  
#*
#****************************************************************************

