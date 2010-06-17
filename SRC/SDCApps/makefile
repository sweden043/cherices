###############################################################################
#                                                                             #
# Huangpu Application makefile - Sophisticated Digitial TV Application Demo   #
#                                                                             #
# Steven Shen, 11/19/2004 2:15PM                                              #
#                                                                             #
###############################################################################
# $Id:$
###############################################################################

SDCAPPS_ROOT = $(SABINE_ROOT)\SDCAPPS

!include $(SDCAPPS_ROOT)\make\common.mak
!include $(SDCAPPS_ROOT)\make\product.mak

APPNAME = HUANGPU

###################################
#                                 #
# Infrared Protocol Configuration #
#                                 #
###################################
!if "$(IR)" == "SEJIN"

IRDECODER       = IRSEJ
IRDECODER_LIB   = IRSEJ.LIB

!elseif "$(IR)" == "IR38K"

IRDECODER       = IR38K
IRDECODER_LIB   = IR38K.LIB

!else

IRDECODER       = IR38K
IRDECODER_LIB   = IR38K.LIB

!endif


####################################
#                                  #
# Front Panel driver configuration.#
#                                  #
####################################
!if "$(FRONT_PANEL_KEYPAD_TYPE)" == "FRONT_PANEL_KEYPAD_BRADY" || "$(FRONT_PANEL_KEYPAD_TYPE)" == "FRONT_PANEL_KEYPAD_BURNET" || "$(FRONT_PANEL_KEYPAD_TYPE)" == "FRONT_PANEL_KEYPAD_CROSBY" || "$(FRONT_PANEL_KEYPAD_TYPE)" == "FRONT_PANEL_KEYPAD_PUDONG"

SCANBTNS_DRIVER =
SCANBTNS_LIB =

!elseif "$(FRONT_PANEL_KEYPAD_TYPE)" == "FRONT_PANEL_KEYPAD_SDCFP" || "$(FRONT_PANEL_KEYPAD_TYPE)" == "FRONT_PANEL_KEYPAD_TONGDA"

SCANBTNS_DRIVER = SCANBTNS
SCANBTNS_LIB = SCANBTNS.LIB

!else

SCANBTNS_DRIVER =
SCANBTNS_LIB =

!endif


###################################
#                                 #
# Demodulator Configuration       #
#                                 #
###################################
DEMOD_DRIVERS =
DEMOD_LIBS =

!if "$(EXTERNAL_TS_SRC0)" == "EXT_TS_SRC_TER_CX22702_TDLB7" || "$(EXTERNAL_TS_SRC1)" == "EXT_TS_SRC_TER_CX22702_TDLB7" || "$(EXTERNAL_TS_SRC2)" == "EXT_TS_SRC_TER_CX22702_TDLB7" || "$(EXTERNAL_TS_SRC3)" == "EXT_TS_SRC_TER_CX22702_TDLB7" || \
    "$(EXTERNAL_TS_SRC0)" == "EXT_TS_SRC_TER_CX22702_T7575" || "$(EXTERNAL_TS_SRC1)" == "EXT_TS_SRC_TER_CX22702_T7575" || "$(EXTERNAL_TS_SRC2)" == "EXT_TS_SRC_TER_CX22702_T7575" || "$(EXTERNAL_TS_SRC3)" == "EXT_TS_SRC_TER_CX22702_T7575" || \
    "$(EXTERNAL_TS_SRC0)" == "EXT_TS_SRC_TER_CX22702_T7579" || "$(EXTERNAL_TS_SRC1)" == "EXT_TS_SRC_TER_CX22702_T7579" || "$(EXTERNAL_TS_SRC2)" == "EXT_TS_SRC_TER_CX22702_T7579" || "$(EXTERNAL_TS_SRC3)" == "EXT_TS_SRC_TER_CX22702_T7579" || \
    "$(EXTERNAL_TS_SRC0)" == "EXT_TS_SRC_TER_CX22702_T7580" || "$(EXTERNAL_TS_SRC1)" == "EXT_TS_SRC_TER_CX22702_T7580" || "$(EXTERNAL_TS_SRC2)" == "EXT_TS_SRC_TER_CX22702_T7580" || "$(EXTERNAL_TS_SRC3)" == "EXT_TS_SRC_TER_CX22702_T7580"
DEMOD_DRIVERS = DEMOD_TERRESTRIAL
DEMOD_LIBS = DEMOD_TERRESTRIAL.LIB
!endif

!if "$(EXTERNAL_TS_SRC0)" == "EXT_TS_SRC_SAT_24106_24110" || "$(EXTERNAL_TS_SRC1)" == "EXT_TS_SRC_SAT_24106_24110" || "$(EXTERNAL_TS_SRC2)" == "EXT_TS_SRC_SAT_24106_24110" || "$(EXTERNAL_TS_SRC3)" == "EXT_TS_SRC_SAT_24106_24110"
DEMOD_DRIVERS = $(DEMOD_DRIVERS) DEMOD_VALIANT
DEMOD_LIBS = $(DEMOD_LIBS) DEMOD_VALIANT.LIB
!endif

!if "$(EXTERNAL_TS_SRC0)" == "EXT_TS_SRC_SAT_24121_24130" || "$(EXTERNAL_TS_SRC1)" == "EXT_TS_SRC_SAT_24121_24130" || "$(EXTERNAL_TS_SRC2)" == "EXT_TS_SRC_SAT_24121_24130" || "$(EXTERNAL_TS_SRC3)" == "EXT_TS_SRC_SAT_24121_24130"
DEMOD_DRIVERS = $(DEMOD_DRIVERS) DEMOD_COBRA
DEMOD_LIBS = $(DEMOD_LIBS) DEMOD_COBRA.LIB
!endif

!if "$(EXTERNAL_TS_SRC0)" == "EXT_TS_SRC_CAB_24430" || "$(EXTERNAL_TS_SRC1)" == "EXT_TS_SRC_CAB_24430" || "$(EXTERNAL_TS_SRC2)" == "EXT_TS_SRC_CAB_24430" || "$(EXTERNAL_TS_SRC3)" == "EXT_TS_SRC_CAB_24430"
DEMOD_DRIVERS = $(DEMOD_DRIVERS) DEMOD_CX24430
DEMOD_LIBS = $(DEMOD_LIBS) DEMOD_CX24430.LIB
!endif

!if "$(EXTERNAL_TS_SRC0)" == "EXT_TS_SRC_BASEBAND_DVB" || "$(EXTERNAL_TS_SRC1)" == "EXT_TS_SRC_BASEBAND_DVB" || "$(EXTERNAL_TS_SRC2)" == "EXT_TS_SRC_BASEBAND_DVB" || "$(EXTERNAL_TS_SRC3)" == "EXT_TS_SRC_BASEBAND_DVB" || \
    "$(EXTERNAL_TS_SRC0)" == "EXT_TS_SRC_BASEBAND_OTV" || "$(EXTERNAL_TS_SRC1)" == "EXT_TS_SRC_BASEBAND_OTV" || "$(EXTERNAL_TS_SRC2)" == "EXT_TS_SRC_BASEBAND_OTV" || "$(EXTERNAL_TS_SRC3)" == "EXT_TS_SRC_BASEBAND_OTV"
DEMOD_DRIVERS = $(DEMOD_DRIVERS) DEMOD_BASEBAND
DEMOD_LIBS = $(DEMOD_LIBS) DEMOD_BASEBAND.LIB
!endif

!if "$(EXTERNAL_TS_SRC0)" == "EXT_TS_SRC_CAB_DCF8722" || "$(EXTERNAL_TS_SRC1)" == "EXT_TS_SRC_CAB_DCF8722" || "$(EXTERNAL_TS_SRC2)" == "EXT_TS_SRC_CAB_DCF8722" || "$(EXTERNAL_TS_SRC3)" == "EXT_TS_SRC_CAB_DCF8722"
DEMOD_DRIVERS = $(DEMOD_DRIVERS) DEMOD_DCF8722
DEMOD_LIBS = $(DEMOD_LIBS) DEMOD_DCF8722.LIB
!endif

!if "$(EXTERNAL_TS_SRC0)" == "EXT_TS_SRC_GX1001_V35" || "$(EXTERNAL_TS_SRC1)" == "EXT_TS_SRC_GX1001_V35" || "$(EXTERNAL_TS_SRC2)" == "EXT_TS_SRC_GX1001_V35" || "$(EXTERNAL_TS_SRC3)" == "EXT_TS_SRC_GX1001_V35"
DEMOD_DRIVERS = $(DEMOD_DRIVERS) GX1001_V35
DEMOD_LIBS = $(DEMOD_LIBS) GX1001_V35.LIB
!endif


###############################
#                             #
# ScanSky Func switch in/out  #
#                             #
###############################
!if "$(SKYSCAN)" == "YES"

SCANSKY_DRIVERS = SKYSCAN
SCANSKY_LIBS = SKYSCAN.LIB

!else

SCANSKY_DRIVERS =
SCANSKY_LIBS =

!endif


###############################
#                             #
# DISEQC Configuration        #
#                             #
###############################
DISEQC_DRIVERS =
DISEQC_LIBS =

!if "$(DISEQC)" == "YES"
DISEQC_DRIVERS = $(DISEQC_DRIVERS) DISEQC
DISEQC_LIBS = $(DISEQC_LIBS) DISEQC.LIB
!endif

###################################
#                                 #
# E2P driver switch in/out        #
#                                 #
###################################
!if "$(E2P_NEEDED)" == "YES"

E2P_DRIVERS = EEPROM E2P
E2P_LIBS = EEPROM.LIB E2P.LIB

!else

E2P_DRIVERS =
E2P_LIBS =

!endif

###################################
#                                 #
# FALSHCFG driver switch in/out   #
#                                 #
###################################
!if "$(FLASHCFG_NEEDED)" == "YES"

FLASHCFG_DRIVERS = FLASHCFG
FLASHCFG_LIBS = FLASHCFG.LIB

!else

FLASHCFG_DRIVERS = 
FLASHCFG_LIBS =

!endif

###################################
#                                 #
# New DVB SI Parser switch in/out #
#                                 #
###################################
!if "$(DVBSI)" == "YES"

DVBSI_DRIVER = DVBSI
DVBSI_LIB = DVBSI.LIB

!else

DVBSI_DRIVER =
DVBSI_LIB =

!endif

###################################
#                                 #
# DVB Subtitle switch in/out      #
#                                 #
###################################
!if "$(DVB_SUBTITLE)" == "YES"

DVBSUBTITLE_DRIVER = STITLE
DVBSUBTITLE_LIB = STITLE.LIB

!else

DVBSUBTITLE_DRIVER =
DVBSUBTITLE_LIB =

!endif

###################################
#                                 #
# Teletext Configuration          #
#                                 #
###################################
!if "$(TELETEXT_MODE)" == "TELETEXT_OSD" || "$(TELETEXT_MODE)" == "TELETEXT_VBI" || "$(TELETEXT_MODE)" == "TELETEXT_BOTH"

DVBTELETEXT_DRIVER = TELETEXT
DVBTELETEXT_LIB = TELETEXT.LIB

!elseif "$(TELETEXT_MODE)" == "TELETEXT_DISABLED"

DVBTELETEXT_DRIVER =
DVBTELETEXT_LIB =

!endif

###############################
#                             #
# CA Configuration            #
#                             #
###############################
CA_DRIVERS =
CA_LIBS =

!if "$(CTICA)" == "YES"
CA_DRIVERS = $(CA_DRIVERS) CTIAPI CTICA CTIUI
CA_LIBS = $(CA_LIBS) CTIAPI.LIB CTICA.LIB CTIUI.LIB
!endif

!if "$(TFCAS)" == "YES"
CA_DRIVERS = $(CA_DRIVERS) TFCA
CA_LIBS = $(CA_LIBS) TFCA.LIB
!endif

!if "$(DVNCA)" == "YES"
CA_DRIVERS = $(CA_DRIVERS) DVNCA
CA_LIBS = $(CA_LIBS) DVNCA.LIB
!endif

###############################
#                             #
# UI Configuration            #
#                             #
###############################
!if "$(UI_ADV)" == "YES"

UI_DRIVERS = UI_ADV
UI_LIBS = UI_ADV.LIB

!else

UI_DRIVERS = UI
UI_LIBS = UI.LIB

!endif

###############################
#                             #
# auto test Configuration     #
#                             #
###############################
!if "$(AUTO_TEST_MODE)" != "AUTOTEST_DISABLED"

AUTO_TEST_DRIVERS = AUTOTEST
AUTO_TEST_LIBS = AUTOTEST.LIB

!else

AUTO_TEST_DRIVERS = 
AUTO_TEST_LIBS = 

!endif

###############################
#                             #
# Mosaic                      #
#                             #
###############################
!if "$(MOSAIC_CTI)" == "YES"

MOSAIC_DRIVERS = MOSAIC
MOSAIC_LIBS = MOSAIC.LIB

!else

MOSAIC_DRIVERS =
MOSAIC_LIBS =

!endif

###############################
#                             #
# Game                        #
#                             #
###############################

!if "$(GAME)" == "YES"

GAME_DRIVERS = GAME
GAME_LIBS = GAME.LIB

!else

GAME_DRIVERS =
GAME_LIBS =

!endif
###############################
#                             #
# Calendar                    #
#                             #
###############################

!if "$(CALENDAR)" == "YES"
CALENDAR_DRIVERS = CALENDAR
CALENDAR_LIBS = CALENDAR.LIB
!else
CALENDAR_DRIVERS =
CALENDAR_LIBS =
!endif

###############################
#                             #
# Ethnet                      #
#                             #
###############################
!if "$(ETHNET_MODE)" == "YES"
ETHNET_DRIVERS = ETH nup_file SL811 
ETHNET_LIBS = ETH.LIB nup_file.LIB SL811.LIB

APP_EXTRA_OBJS  = $(APP_EXTRA_OBJS) $(SABINE_ROOT)\ETH\NUPNET.LIB
!else
ETHNET_DRIVERS =
ETHNET_LIBS =
!endif

###############################
#                             #
# NVOD                        #
#                             #
###############################
!if "$(NVOD_SERVICE_ENABLE)" == "YES"
NVOD_DRIVERS = NVOD
NVOD_LIBS = NVOD.LIB
!else
NVOD_DRIVERS =
NVOD_LIBS =
!endif

!if "$(SEACHANGE_VOD_ENABLE)" == "YES"
VOD_DRIVERS = VOD_SC
VOD_LIBS = VOD_SC.LIB
#APP_EXTRA_OBJS  = $(APP_EXTRA_OBJS) $(SABINE_ROOT)\SDCAPPS\VOD_SC\applib.LIB $(SABINE_ROOT)\SDCAPPS\VOD_SC\realitv.LIB
#APP_EXTRA_OBJS  = $(APP_EXTRA_OBJS) $(SABINE_ROOT)\SDCAPPS\VOD_SC\itvtest.lib
!else
VOD_DRIVERS =
VOD_LIBS =
!endif


###############################
#                             #
# GX1001 Tuner                #
#                             #
###############################
!if "$(GX1001_V35_SERVICE_ENABLE)" == "YES"
GX1001_V35_DRIVERS = GX1001_V35
GX1001_V35_LIBS = GX1001_V35.LIB
!else
GX1001_V35_DRIVERS =
GX1001_V35_LIBS =
!endif


###############################
#                             #
# GIF Code                    #
#                             #
###############################
!if "$(GIF_DECODE)" == "YES"
GIF_DECODE_DRIVERS  = GIF_DECODE
GIF_DECODE_LIBS =GIF_DECODE.lib
!else
GIF_DECODE_DRIVERS  = 
GIF_DECODE_LIBS =
!endif


###############################
#                             #
# HuNan AV                    #
#                             #
###############################
!if "$(HUNAN_AD)" == "YES"
AD_PROC_DRIVERS  = AD_PROC
AD_PROC_LIBS =AD_PROC.lib
!else
AD_PROC_DRIVERS  = 
AD_PROC_LIBS =
!endif

!if "$(SDCAPP_DRIVERS)"=="" && "$(ALL_DRIVERS)"==""

SDCAPP_DRIVERS =   IPANEL 
#                  ROOT AVMAN CSDB LANGMAN NITPARSER OSDMAN \
#                  THEMEMAN UAMI USRAPP EPG \
#                  $(UI_DRIVERS) \
#                  $(SCANSKY_DRIVERS) \
#                  $(DISEQC_DRIVERS) \
#                  $(DVBSI_DRIVER) \
#                  $(DVBTELETEXT_DRIVER) \
#                  $(DVBSUBTITLE_DRIVER) \
#                  $(CA_DRIVERS) \
#                  $(AUTO_TEST_DRIVERS)
#                  $(GAME_DRIVERS)	\
#                  $(MOSAIC_DRIVERS)	\
#                  $(CALENDAR_DRIVERS)  \                  
#                  $(NVOD_DRIVERS) \
#                  $(VOD_DRIVERS)	\
#                  $(GIF_DECODE_DRIVERS)\
#                  $(AD_PROC_DRIVERS) 

ALL_DRIVERS    = $(OPERATING_SYS_DRIVERS) HWLIB IIC DEMUX \
                  OSDLIBC MEMMOVE TRACE FLASH AUD_COMN VIDEO \
                  PAWSER GFXLIB HANDLE GXAGFX FONTS \
                  IOFUNCS GENIR $(IRDECODER) $(SCANBTNS_DRIVER) \
                  DEMOD $(DEMOD_DRIVERS) \
                  $(SCART_DRIVER) \
                  TTX20 TVENC TVENC_BT861_INTERNAL \
                  CLKREC BCD HSDP \
                  NVSTORE $(E2P_DRIVERS) $(FLASHCFG_DRIVERS) \
                  CONFMGR FPLEDS PCM \
                  SMARTCARD \
                  $(ETHNET_DRIVERS) \
                  $(GX1001_V35_DRIVERS)                  

!endif

SDCAPP_LIBS  =    IPANEL.LIB 
#                  ROOT.LIB AVMAN.LIB CSDB.LIB LANGMAN.LIB NITPARSER.LIB \
#                  OSDMAN.LIB THEMEMAN.LIB UAMI.LIB USRAPP.LIB EPG.LIB \
#                  $(UI_LIBS) \
#                  $(SCANSKY_LIBS) \
#                  $(DISEQC_LIBS) \
#                  $(DVBSI_LIB) \
#                  $(DVBTELETEXT_LIB) \
#                  $(DVBSUBTITLE_LIB) \
#                  $(CA_LIBS) \
#                  $(AUTO_TEST_LIBS) \
#                  $(GAME_LIBS)	\
#                  $(MOSAIC_LIBS) \
#                  $(CALENDAR_LIBS)  \
#                  $(NVOD_LIBS) \
#                  $(VOD_LIBS) \
#                  $(GIF_DECODE_LIBS)	\
#                  $(AD_PROC_LIBS) \

DRIVER_LIBS =     $(OPERATING_SYS_LIBS) HWLIB.LIB IIC.LIB DEMUX.LIB \
                  OSDLIBC.LIB MEMMOVE.LIB TRACE.LIB FLASH.LIB AUD_COMN.LIB VIDEO.LIB \
                  PAWSER.LIB GFXLIB.LIB HANDLE.LIB GXAGFX.LIB FONTS.LIB \
                  IOFUNCS.LIB GENIR.LIB $(IRDECODER_LIB) $(SCANBTNS_LIB) \
                  DEMOD.LIB $(DEMOD_LIBS) \
                  $(SCART_LIB) \
                  TTX20.LIB TVENC.LIB TVENC_BT861_INTERNAL.LIB \
                  CLKREC.LIB BCD.LIB HSDP.LIB \
                  NVSTORE.LIB $(E2P_LIBS) $(FLASHCFG_LIBS) \
                  CONFMGR.LIB FPLEDS.LIB PCM.LIB \
                  SMARTCARD.LIB \
                  $(ETHNET_LIBS)  \
                  $(GX1001_V35_LIBS) \
                  


###############################
# Conditional Access Modules  #
###############################
!if "$(CTICA)" == "YES"
APP_EXTRA_OBJS  = $(APP_EXTRA_OBJS) $(SABINE_ROOT)\SDCAPPS\CTICA\CACORE.LIB
APP_EXTRA_OBJS  = $(APP_EXTRA_OBJS) $(SABINE_ROOT)\SDCAPPS\CTICA\CATASK.LIB
!endif

!if "$(TFCAS)" == "YES"
#APP_EXTRA_OBJS  = $(APP_EXTRA_OBJS) $(SABINE_ROOT)\SDCAPPS\TFCA\cas21.LIB
!endif
!if "$(DVNCA)" == "YES"
APP_EXTRA_OBJS  = $(APP_EXTRA_OBJS) $(SABINE_ROOT)\SDCAPPS\DVNCA\dvnca_4_0.lib
!endif

#########################################################################
# Include the file that does all the work - you shouldn't change this ! #
#########################################################################
!include $(SDCAPPS_ROOT)\make\topmake.mak

###############################################################################
# $Log:$
###############################################################################
