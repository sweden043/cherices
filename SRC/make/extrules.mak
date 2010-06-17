#****************************************************************************
#*                            Conexant Systems
#****************************************************************************
#*
#* Filename:       extrules.mak
#*
#* Description:    Definition of File Name Extension Macros
#*
#* Author:         Bobby Bradford
#*
#* Copyright Conexant Systems, 2003
#* All Rights Reserved.
#*
#****************************************************************************
#* $Header: extrules.mak, 2, 9/10/03 5:21:48 PM, Bobby Bradford$
#****************************************************************************

#****************************************************************************
# Define FILE Extensions for SDT Toolkit
#****************************************************************************
!if "$(ARM_TOOLKIT)" == "SDT" 
EXT_LIB = ALF

!endif

#****************************************************************************
# Define FILE Extensions for ADS Toolkit
#****************************************************************************
!if "$(ARM_TOOLKIT)" == "ADS" 
EXT_LIB = ADS

!endif


#****************************************************************************
# Define FILE Extensions for ADS Toolkit
#****************************************************************************
!if "$(ARM_TOOLKIT)" == "WRGCC" 
EXT_LIB = WRGCC

!endif


#****************************************************************************
#  $Log: 
#   2    mpeg      1.1         9/10/03 5:21:48 PM     Bobby Bradford  SCR(s) 
#         7291 :
#         Modified to support WRGCC (instead of VXW) ... no longer
#         used in TOPMAKE.MAK, but still used by modem makefiles
#         
#   1    mpeg      1.0         3/19/03 4:52:22 PM     Bobby Bradford  
#  $
#  
#     Rev 1.1   10 Sep 2003 16:21:48   bradforw
#  SCR(s) 7291 :
#  Modified to support WRGCC (instead of VXW) ... no longer
#  used in TOPMAKE.MAK, but still used by modem makefiles
#  
#     Rev 1.0   19 Mar 2003 16:52:22   bradforw
#  SCR(s) 5798 :
#  File Extension Rules for MAKE system
#
# 
#****************************************************************************

