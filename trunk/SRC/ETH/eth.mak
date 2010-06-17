#****************************************************************************
#*                   CONEXANT PROPRIETARY AND CONFIDENTIAL                  *
#*                       SOFTWARE FILE/MODULE HEADER                        *
#*                    Conexant Systems Inc. (c) 1998-2003                   *
#*                                Austin, TX                                *
#*                           All Rights Reserved                            *
#****************************************************************************
#
#  Filename:        eth.mak
#
#
#  Description:     eth Driver makefile
#
#
#  Author:          Sam Chen
#
#****************************************************************************
#  $Header: eth.mak, 5, 4/21/05 11:15:48 PM, Sam Chen$
#****************************************************************************

OBJECT_ONLY = YES

!if "$(ETH_CHIPSET)" == "SMSC"
GENERAL_C_FILES =   ETH.C SMSC911x.C
OTHER_FILES =  ETH.H \
               SMSC911x.H 
               
!elseif "$(ETH_CHIPSET)" == "DAVICOM9000A"
GENERAL_C_FILES =   ETH.C DM9000A.C 

OTHER_FILES =  ETH.H \
!endif
              
            

