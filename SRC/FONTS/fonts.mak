###############################################################################
#                                                                             #
# Freeware fonts converted for use with Conexant text blit functions          #
#                                                                             #
# Author: Dave Wilson, 8/2/02                                                 #
#                                                                             #
# Copyright, 2002 Conexant Systems Incorporated. All Rights Reserved          #
#                                                                             #
###############################################################################

GENERAL_C_FILES= BARAMOND_15PT_BOLD.C             \
                 BARAMOND_20PT_BOLD.C             \
                 ZEKTON_15PT_BOLD.C               \
                 SUI_GENERIS_10PT.C               \
                 SUI_GENERIS_10PT_FULL.C          \
                 SUI_GENERIS_15PT_FULL.C          \
!if "$(BRIEF_CHINESE_FONT)" == "YES"
                 BRIEF_chinese_heiti_24x24.C   \
                 chinese_zhunyuan_20x22.C  \
                  S_CHINESE_FONTS.C     \
                  heiti_32_32.C  \
                 heiti_16_16.C  \
!if "$(TELETEXT_MODE)"=="TELETEXT_OSD" || "$(TELETEXT_MODE)"=="TELETEXT_BOTH"
                 ttxt_12x10.C\
!endif
                 
!endif
