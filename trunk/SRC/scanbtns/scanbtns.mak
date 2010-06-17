###############################################################################
#                                                                             #
# Sabine: Generic Front Panel Button Driver for Klondike IRD                  #
#                                                                             #
# Author: Dave Wilson, 3/21/00                                                #
#                                                                             #
# Copyright, 2000 Conexant Systems Inc. All Rights Reserved                   #
#                                                                             #
###############################################################################

!if "$(RTOS)" == "NOOS"

!if "$(FRONT_PANEL_KEYPAD_TYPE)" == "FRONT_PANEL_KEYPAD_SDCFP"
GENERAL_C_FILES =  SCANBTNS_PIO.C 

!elseif "$(FRONT_PANEL_KEYPAD_TYPE)" == "FRONT_PANEL_KEYPAD_TONGDA"
GENERAL_C_FILES =  SCANBTNS_TD.C 

!else
GENERAL_C_FILES = SCANBTNS.C CNXTBTNS.C

!endif

!else


!if "$(FRONT_PANEL_KEYPAD_TYPE)" == "FRONT_PANEL_KEYPAD_SDCFP"

GENERAL_C_FILES  =   SCANBTNS_PIO.C  

!elseif "$(FRONT_PANEL_KEYPAD_TYPE)" == "FRONT_PANEL_KEYPAD_TONGDA"

GENERAL_C_FILES  =    SCANBTNS_TD.C 

!else

GENERAL_C_FILES  =  SCANBTNS.C \

!endif

!endif

!if "$(FRONT_PANEL_KEYPAD_TYPE)" == "FRONT_PANEL_KEYPAD_KLONDIKE" || \
    "$(FRONT_PANEL_KEYPAD_TYPE)" == "FRONT_PANEL_KEYPAD_ABILENE"  || \
    "$(FRONT_PANEL_KEYPAD_TYPE)" == "FRONT_PANEL_KEYPAD_BRONCO"   || \
    "$(FRONT_PANEL_KEYPAD_TYPE)" == "FRONT_PANEL_KEYPAD_EUREKA"   || \
    "$(FRONT_PANEL_KEYPAD_TYPE)" == "FRONT_PANEL_KEYPAD_ATHENS"
                    CNXTBTNS.C 
!endif
!if "$(FRONT_PANEL_KEYPAD_TYPE)" == "FRONT_PANEL_KEYPAD_VEND_D_PROD_1" 
                    VEND_D_BTNS.C 
!endif

OTHER_FILES =       SCANBTNS.H

