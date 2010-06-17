#****************************************************************************
#*                   CONEXANT PROPRIETARY AND CONFIDENTIAL                  *
#*                       SOFTWARE FILE/MODULE HEADER                        *
#*                    Conexant Systems Inc. (c) 1998-2003                   *
#*                                Austin, TX                                *
#*                           All Rights Reserved                            *
#****************************************************************************
#
#  Filename:        sl811.mak
#
#
#  Description:     usb Driver makefile
#
#
#  Author:          Broken Chen
#
#****************************************************************************
#  $Header: sl811.mak, 5, 4/21/05 11:15:48 PM, Broken Chen$
#****************************************************************************

OBJECT_ONLY = YES

GENERAL_C_FILES =  HAL.C SL811.C TPBULK.C


OTHER_FILES =  HAL.H COMMON.H  RBCCMD.H SL811.H TPBULK.H
            

