################################################################################
#                                                                              #
#                   CONEXANT PROPRIETARY AND CONFIDENTIAL                      #
#                  Conexant Systems Incorporated  (c) 2002                     #
#                                 Austin, TX                                   #
#                            All Rights Reserved                               #
#                                                                              #
# CX24130/CX24121 (Cobra) Demodulator Driver Makefile                          #
#                                                                              #
################################################################################

################################################################################
#
# $Header: demod_cobra.mak, 3, 6/24/03 6:31:54 PM, Tim White$
#
################################################################################

################################################################################
# File lists.  These macros define the files comprising this driver.           #
################################################################################

!if "$(RTOS)" == "NOOS"

GENERAL_C_FILES = COBRA_CX24108.C COBRA_API.C COBRA_DRV.C \
                  COBRA_TUNER.C COBRA_REG.C LNB.C

!else

GENERAL_C_FILES = COBRA_CX24108.C COBRA_CX24128.C COBRA_API.C COBRA_DRV.C \
                  COBRA_TUNER.C COBRA_IQ.C COBRA_REG.C DEMOD_COBRA.C LNB.C

!endif

OTHER_FILES =     COBRA_CX24108.H COBRA_CX24128.H COBRA_API.H COBRA_DRV.H \
                  COBRA_TUNER.H COBRA_DEFS.H COBRA_ENUM.H COBRA_REGS.H \
                  COBRA_STR.H COBRA.H COBRA_GBLS.H COBRA_PROTO.H COBRA_VER.H \
                  LNB.H APIFIX.H

################################################################################
# Modifications:
#
# $Log: 
#  3    mpeg      1.2         6/24/03 6:31:54 PM     Tim White       SCR(s) 
#        6831 :
#        Add flash, hsdp, demux, OSD, and demod support to codeldrext
#        
#        
#  2    mpeg      1.1         12/2/02 11:01:30 AM    Billy Jackman   SCR(s) 
#        4977 :
#        List new file APIFIX.H in OTHER_FILES so a get will really get the 
#        file.
#        
#  1    mpeg      1.0         9/30/02 12:15:30 PM    Billy Jackman   
# $
#  
#     Rev 1.2   24 Jun 2003 17:31:54   whiteth
#  SCR(s) 6831 :
#  Add flash, hsdp, demux, OSD, and demod support to codeldrext
#  
#  
#     Rev 1.1   02 Dec 2002 11:01:30   jackmaw
#  SCR(s) 4977 :
#  List new file APIFIX.H in OTHER_FILES so a get will really get the file.
#  
#     Rev 1.0   30 Sep 2002 11:15:30   jackmaw
#  SCR(s) 4714 :
#  Makefile for Cobra demod module
#
################################################################################

