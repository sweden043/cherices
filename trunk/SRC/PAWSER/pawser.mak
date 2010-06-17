###############################################################################
#                                                                             #
# Pawser Microcode Makefile                                                   #
#                                                                             #
# Author: T. White, 01/11/02                                                  #
#                                                                             #
# Copyright, 2002 Conexant Systems Inc. All Rights Reserved                   #
#                                                                             #
###############################################################################

  ############################################################################
  ############################################################################
  ## IMPORTANT NOTE: Due to the use of string substitution in the top level ##
  ##                 makefile, all filenames MUST be in upper case !        ##
  ############################################################################
  ############################################################################

!if "$(PARSER_TYPE)" == "DVB_PARSER"

!if "$(PARSER_BOOTLOADER)" == "YES"

#
# Bootloader Microcodes
#
OTHER_FILES= DVB_APL.H DVB_CPL.H DVB_DPL.H

!else
!if "$(PVR)" == "YES"

#
# PVR Microcodes
#
OTHER_FILES= DVB_CP_PVR_REC.H DVB_CP_PVR_PLY.H

!else
!if "$(LEGACY_DVR)" == "YES"

#
# DVR Microcodes
#
OTHER_FILES= DVB_BR.H DVB_BR2.H DVB_BR3.H DVB_BPV.H DVB_BPV2.H DVB_BPV3.H DVB_BCPV.H DVB_BCPV2.H DVB_BCPV3.H DVB_BPXV.H DVB_BPXV2.H DVB_BPXV3.H

!else
!if "$(PARSER_PSI_BUFFER)" == "MULTI_PSI"
!if "$(PARSER_NDS_ICAM)" == "YES"

#
# NDS Microcodes
#
OTHER_FILES= DVB_ANP.H DVB_ANPF.H DVB_CNP.H DVB_DNP.H DVB_ANCP.H DVB_ANCPF.H DVB_CNCP.H DVB_CNCPI.H DVB_DNCP.H DVB_CNP_XTV.H

!else

#
# Non-NDS Microcodes
#
OTHER_FILES= DVB_AP.H DVB_APF.H DVB_CP.H DVB_DP.H DVB_DKP.H DVB_ACP.H DVB_ACPF.H DVB_ACPK.H DVB_ACPKF.H DVB_CCP.H DVB_CPS.H DVB_CCPS.h DVB_DCP.H DVB_APX.H DVB_APXF.H

!endif
!else
!if "$(PARSER_NDS_ICAM)" == "YES"

#
# Legacy NDS Microcodes
#
OTHER_FILES= DVB_AN.H DVB_ANC.H

!else

#
# Legacy non-NDS Microcodes
#
OTHER_FILES= DVB_A.H DVB_A_AC3.H

!endif
!endif
!endif
!endif
!endif

GENERAL_C_FILES= DVB.C

!else

!if "$(DIRECTV_PROGRAM_GUIDE)" == "APG"
OTHER_FILES = DIRECTV_A.H
!else
!if "$(DIRECTV_PROGRAM_GUIDE)" == "MPG"
OTHER_FILES = DIRECTV.H
!endif
!endif

GENERAL_C_FILES= DIRECTV.C

!endif


