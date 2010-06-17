###############################################################################
#                                                                             #
# CNXT MPEG build support file: envcheck.mak                                  #
#                                                                             #
# Author: M. Bintz 7/24/03                                                    #
#                                                                             #
# Compiler rules for all toolchains and file types.                           #
#                                                                             #
###############################################################################

###########################################################################
#                                                                         # 
#            CHECK for Valid TOOLKIT ENVIRONMENT Setup                    #
#                                                                         # 
###########################################################################

##########################    ADS    ##############################
!if   "$(ARM_TOOLKIT)" == "ADS"             # . ARM_TOOLKIT

!if   "$(ARM_VERSION)" == "11"              # .. ARM_VERSION
!elseif "$(ARM_VERSION)" == "12"            # .. ARM_VERSION
!else                                       # .. ARM_VERSION
!message ARM_VERSION must be defined as "11" or "12"
!message Refer to App Note in Tracker #3456 for more information
!error ARM_VERSION not properly defined for ARM_TOOLKIT = ADS
!endif                                      # .. ARM_VERSION

##########################    SDT    ##############################
!elseif "$(ARM_TOOLKIT)" == "SDT"           # . ARM_TOOLKIT

!if   "$(ARM_VERSION)" == "250"             # .. ARM_VERSION
!elseif "$(ARM_VERSION)" == "251"           # .. ARM_VERSION
!else                                       # .. ARM_VERSION
!message ARM_VERSION must be defined as "250" or "251"
!message Refer to App Note in Tracker #3456 for more information
!error ARM_VERSION not properly defined for ARM_TOOLKIT = SDT
!endif                                      # .. ARM_VERSION

##########################   WRGCC   ##############################
!elseif "$(ARM_TOOLKIT)" == "WRGCC"

!if "$(ARM_VERSION)" == "296"
!else
!message ARM_VERSION must be defined as "296" 
!message  Where: 296 is the Windriver/Tornado 2.2.0 supplied compiler
!error ARM_VERSION not properly defined for ARM_TOOLKIT = WRGCC
!endif

##########################  GNUGCC   ##############################
!elseif "$(ARM_TOOLKIT)" == "GNUGCC"

!if "$(ARM_VERSION)" == "321"
!else
!message ARM_VERSION must be defined as "321"
!message  Where: 321 is the Conexant-compiled "stock" GNU GCC v3.2.1 compiler
!error ARM_VERSION not properly defined for ARM_TOOLKIT = GNUGCC
!endif

##########################  catchall ##############################
!else                                       # . ARM_TOOLKIT

!message ARM_TOOLKIT must be defined as "ADS" | "SDT" | "WRGCC" | "GNUGCC"
!message Refer to App Note in Tracker #3456 for more information
!error ARM_TOOLKIT not properly defined!

!endif                                      # . ARM_TOOLKIT



#****************************************************************************
#* $Log: 
#*  1    mpeg      1.0         9/9/03 4:40:38 PM      Miles Bintz     
#* $
#  
#     Rev 1.0   09 Sep 2003 15:40:38   bintzmf
#  SCR(s) 7291 :
#  reworked build system
#  
#     Rev 1.1   28 Aug 2003 15:25:34   bintzmf
#  
#     Rev 1.0   14 Aug 2003 13:41:46   bintzmf
#  new build system
#*  
#*  
#****************************************************************************


