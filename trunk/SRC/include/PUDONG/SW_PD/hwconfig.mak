#*************************************************************
#*        Board software configuration header file           *
#*************************************************************
#* This file is automatically generated by the build process *
#* from the supplied configuration file. Editing it is,      *
#* therefore, completely futile!                             *
#*************************************************************
!if "$(PAGE_TABLE_INITIALIZATION)" == ""
PAGE_TABLE_INITIALIZATION = PHYSICAL_RAM
!endif
!if "$(PLL_PIN_ALT_FUNC_REG_DEFAULT)" == ""
PLL_PIN_ALT_FUNC_REG_DEFAULT = NOT_DETERMINED
!endif
!if "$(PLL_PIN_GPIO_MUX0_REG_DEFAULT)" == ""
PLL_PIN_GPIO_MUX0_REG_DEFAULT = NOT_DETERMINED
!endif
!if "$(PLL_PIN_GPIO_MUX1_REG_DEFAULT)" == ""
PLL_PIN_GPIO_MUX1_REG_DEFAULT = NOT_DETERMINED
!endif
!if "$(PLL_PIN_GPIO_MUX2_REG_DEFAULT)" == ""
PLL_PIN_GPIO_MUX2_REG_DEFAULT = NOT_DETERMINED
!endif
!if "$(PLL_PIN_GPIO_MUX3_REG_DEFAULT)" == ""
PLL_PIN_GPIO_MUX3_REG_DEFAULT = NOT_DETERMINED
!endif
!if "$(PLL_PIN_ALT_FUNC_REG_DEFAULT_MILANO1)" == ""
PLL_PIN_ALT_FUNC_REG_DEFAULT_MILANO1 = NOT_DETERMINED
!endif
!if "$(PLL_PIN_GPIO_MUX0_REG_DEFAULT_MILANO1)" == ""
PLL_PIN_GPIO_MUX0_REG_DEFAULT_MILANO1 = NOT_DETERMINED
!endif
!if "$(PLL_PIN_GPIO_MUX1_REG_DEFAULT_MILANO1)" == ""
PLL_PIN_GPIO_MUX1_REG_DEFAULT_MILANO1 = NOT_DETERMINED
!endif
!if "$(PLL_PIN_GPIO_MUX2_REG_DEFAULT_MILANO1)" == ""
PLL_PIN_GPIO_MUX2_REG_DEFAULT_MILANO1 = NOT_DETERMINED
!endif
!if "$(PLL_PIN_GPIO_MUX3_REG_DEFAULT_MILANO1)" == ""
PLL_PIN_GPIO_MUX3_REG_DEFAULT_MILANO1 = NOT_DETERMINED
!endif
!if "$(PCI_ISA_DESCRIPTOR_SETUP)" == ""
PCI_ISA_DESCRIPTOR_SETUP = DESCRIPTOR_SETUP_KLONDIKE
!endif
!if "$(RAM_RANK0_CONFIG)" == ""
RAM_RANK0_CONFIG = AUTOSENSE
!endif
!if "$(RAM_RANK1_CONFIG)" == ""
RAM_RANK1_CONFIG = AUTOSENSE
!endif
!if "$(RAM_BUS_WIDTH)" == ""
RAM_BUS_WIDTH = RAM_BUS_32BITS_WIDE
!endif
!if "$(EMULATION_LEVEL)" == ""
EMULATION_LEVEL = FINAL_HARDWARE
!endif
!if "$(DIGITAL_DISPLAY)" == ""
DIGITAL_DISPLAY = NOT_PRESENT
!endif
!if "$(DIGITAL_DISPLAY_UART)" == ""
DIGITAL_DISPLAY_UART = INTERNAL_UART2
!endif
!if "$(DIGITAL_DISPLAY_RESET_GPIO)" == ""
DIGITAL_DISPLAY_RESET_GPIO = 49
!endif
!if "$(IRD_BOARD_TYPE)" == ""
IRD_BOARD_TYPE = BOARD_TYPE_BRONCO
!endif
!if "$(PUDONG_AUDIO_DAC)" == ""
PUDONG_AUDIO_DAC = NOT_PRESENT
!endif
!if "$(EXTERNAL_AUDIO_MUX)" == ""
EXTERNAL_AUDIO_MUX = NOT_PRESENT
!endif
!if "$(EXTERNAL_AUDIO_MUX_NO_INPUTS)" == ""
EXTERNAL_AUDIO_MUX_NO_INPUTS = 2
!endif
!if "$(EXTERNAL_AUDIO_MUX_GPIO_PIN_0)" == ""
EXTERNAL_AUDIO_MUX_GPIO_PIN_0 = 32
!endif
!if "$(EXTERNAL_AUDIO_MUX_RESET_GPIO_PIN)" == ""
EXTERNAL_AUDIO_MUX_RESET_GPIO_PIN = 33
!endif
!if "$(CROSBY_MUX_AUDIO_INPUT_DEFAULT)" == ""
CROSBY_MUX_AUDIO_INPUT_DEFAULT = BRAZOS_DAC_OUTPUT
!endif
!if "$(EXTERNAL_VIDEO_MUX)" == ""
EXTERNAL_VIDEO_MUX = NOT_PRESENT
!endif
!if "$(EXTERNAL_VIDEO_MUX_NO_INPUTS)" == ""
EXTERNAL_VIDEO_MUX_NO_INPUTS = 2
!endif
!if "$(EXTERNAL_VIDEO_MUX_GPIO_PIN_0)" == ""
EXTERNAL_VIDEO_MUX_GPIO_PIN_0 = 37
!endif
!if "$(CROSBY_MUX_VIDEO_INPUT_DEFAULT)" == ""
CROSBY_MUX_VIDEO_INPUT_DEFAULT = BRAZOS_VIDEO_OUTPUT
!endif
!if "$(EXTERNAL_ANALOGUE_CAPTURE_DEVICE)" == ""
EXTERNAL_ANALOGUE_CAPTURE_DEVICE = NOT_PRESENT
!endif
!if "$(EXTERNAL_ANALOGUE_CAPTURE_DEVICE_GPIO_PIN_0)" == ""
EXTERNAL_ANALOGUE_CAPTURE_DEVICE_GPIO_PIN_0 = (36+GPIO_DEVICE_ID_INTERNAL+GPIO_POSITIVE_POLARITY+GPIO_PIN_IS_OUTPUT)
!endif
!if "$(EXTERNAL_ANALOGUE_CAPTURE_DEVICE_GPIO_PIN_1)" == ""
EXTERNAL_ANALOGUE_CAPTURE_DEVICE_GPIO_PIN_1 = (35+GPIO_DEVICE_ID_INTERNAL+GPIO_POSITIVE_POLARITY+GPIO_PIN_IS_OUTPUT)
!endif
!if "$(EXTERNAL_ANALOGUE_CAPTURE_DEVICE_GPIO_INTERUPT)" == ""
EXTERNAL_ANALOGUE_CAPTURE_DEVICE_GPIO_INTERUPT = (34+GPIO_DEVICE_ID_INTERNAL+GPIO_NEGATIVE_POLARITY+GPIO_PIN_IS_INPUT)
!endif
!if "$(EXTERNAL_ENCODER)" == ""
EXTERNAL_ENCODER = DETECT_BT861_BT865
!endif
!if "$(VIDEO_ENCODER_0)" == ""
VIDEO_ENCODER_0 = INTERNAL
!endif
!if "$(VIDEO_ENCODER_MACROVISION_CAPABLE)" == ""
VIDEO_ENCODER_MACROVISION_CAPABLE = NO
!endif
!if "$(INTERNAL_ENCODER_OUTMODE)" == ""
INTERNAL_ENCODER_OUTMODE = 1
!endif
!if "$(VIDEO_ENCODER_RGB_DAC_MASK)" == ""
VIDEO_ENCODER_RGB_DAC_MASK = 0x38
!endif
!if "$(VIDEO_ENCODER_YC_DAC_MASK)" == ""
VIDEO_ENCODER_YC_DAC_MASK = 0x03
!endif
!if "$(VIDEO_ENCODER_CVBS_DAC_MASK)" == ""
VIDEO_ENCODER_CVBS_DAC_MASK = 0x04
!endif
!if "$(VIDEO_SIGNAL_OUTPUT_TYPES)" == ""
VIDEO_SIGNAL_OUTPUT_TYPES = (VIDEO_OUTPUT_CVBS+VIDEO_OUTPUT_RGB+VIDEO_OUTPUT_YC)
!endif
!if "$(ENCODER_HAS_DISABLE_DACS)" == ""
ENCODER_HAS_DISABLE_DACS = NO
!endif
!if "$(EXTERNAL_DECODER)" == ""
EXTERNAL_DECODER = NOT_PRESENT
!endif
!if "$(ACAP_MODE)" == ""
ACAP_MODE = ACAP_MODE_STEREO
!endif
!if "$(ACAP_FORMAT)" == ""
ACAP_FORMAT = ACAP_FORMAT_I2S
!endif
!if "$(ACAP_NUM_BITS)" == ""
ACAP_NUM_BITS = ACAP_NUM_BITS_18
!endif
!if "$(USB_POWER_ENABLE)" == ""
USB_POWER_ENABLE = KLONDIKE_CONTROL
!endif
!if "$(EEPROM_TYPE)" == ""
EEPROM_TYPE = EEPROM_16KB
!endif
!if "$(SERIAL1)" == ""
SERIAL1 = INTERNAL_UART3
!endif
!if "$(SERIAL2)" == ""
SERIAL2 = UART_NONE
!endif
!if "$(SERIAL2_CM_MPEG_OWNER)" == ""
SERIAL2_CM_MPEG_OWNER = SERIAL2_OWNER_MPEG
!endif
!if "$(TRACE_PORT)" == ""
TRACE_PORT = SERIAL1
!endif
!if "$(REDIRECTOR_PORT)" == ""
REDIRECTOR_PORT = TELEGRAPH_UART1
!endif
!if "$(CNXT_PIO_EXP_TYPE)" == ""
CNXT_PIO_EXP_TYPE = GPIO_EXTEND_IIC
!endif
!if "$(CNXT_PIO_EXP_REG)" == ""
CNXT_PIO_EXP_REG = (-1)
!endif
!if "$(CNXT_PIO_EXP_REG_2)" == ""
CNXT_PIO_EXP_REG_2 = (-1)
!endif
!if "$(FRONT_PANEL_KEYPAD_TYPE)" == ""
FRONT_PANEL_KEYPAD_TYPE = FRONT_PANEL_KEYPAD_LEGACY
!endif
!if "$(FRONT_PANEL_KEYPAD_NUM_ROWS)" == ""
FRONT_PANEL_KEYPAD_NUM_ROWS = 3
!endif
!if "$(PIO_FRONT_PANEL_KEYPAD_ROW_0)" == ""
PIO_FRONT_PANEL_KEYPAD_ROW_0 = (1+GPIO_DEVICE_ID_INTERNAL+GPIO_POSITIVE_POLARITY+GPIO_PIN_IS_OUTPUT)
!endif
!if "$(PIO_FRONT_PANEL_KEYPAD_ROW_1)" == ""
PIO_FRONT_PANEL_KEYPAD_ROW_1 = (2+GPIO_DEVICE_ID_INTERNAL+GPIO_POSITIVE_POLARITY+GPIO_PIN_IS_OUTPUT)
!endif
!if "$(PIO_FRONT_PANEL_KEYPAD_ROW_2)" == ""
PIO_FRONT_PANEL_KEYPAD_ROW_2 = (3+GPIO_DEVICE_ID_INTERNAL+GPIO_POSITIVE_POLARITY+GPIO_PIN_IS_OUTPUT)
!endif
!if "$(FRONT_PANEL_KEYPAD_NUM_COLS)" == ""
FRONT_PANEL_KEYPAD_NUM_COLS = 3
!endif
!if "$(PIO_FRONT_PANEL_KEYPAD_COL_0)" == ""
PIO_FRONT_PANEL_KEYPAD_COL_0 = (4+GPIO_DEVICE_ID_INTERNAL+GPIO_POSITIVE_POLARITY+GPIO_PIN_IS_INPUT)
!endif
!if "$(PIO_FRONT_PANEL_KEYPAD_COL_1)" == ""
PIO_FRONT_PANEL_KEYPAD_COL_1 = (5+GPIO_DEVICE_ID_INTERNAL+GPIO_POSITIVE_POLARITY+GPIO_PIN_IS_INPUT)
!endif
!if "$(PIO_FRONT_PANEL_KEYPAD_COL_2)" == ""
PIO_FRONT_PANEL_KEYPAD_COL_2 = (6+GPIO_DEVICE_ID_INTERNAL+GPIO_POSITIVE_POLARITY+GPIO_PIN_IS_INPUT)
!endif
!if "$(PIO_FRONT_PANEL_POWER_BUTTON)" == ""
PIO_FRONT_PANEL_POWER_BUTTON = 0
!endif
!if "$(PIO_SC0_VOLT_CTRL)" == ""
PIO_SC0_VOLT_CTRL = (3+GPIO_DEVICE_ID_ISA_1+GPIO_PIN_IS_OUTPUT+GPIO_POSITIVE_POLARITY)
!endif
!if "$(PIO_SC1_VOLT_CTRL)" == ""
PIO_SC1_VOLT_CTRL = (4+GPIO_DEVICE_ID_ISA_1+GPIO_PIN_IS_OUTPUT+GPIO_POSITIVE_POLARITY)
!endif
!if "$(NUMBER_OF_LEDS)" == ""
NUMBER_OF_LEDS = 4
!endif
!if "$(PIO_LED_POWER)" == ""
PIO_LED_POWER = (0+GPIO_DEVICE_ID_INTERNAL+GPIO_NEGATIVE_POLARITY+GPIO_PIN_IS_OUTPUT)
!endif
!if "$(PIO_LED_IR_MSG)" == ""
PIO_LED_IR_MSG = (7+GPIO_DEVICE_ID_INTERNAL +GPIO_NEGATIVE_POLARITY+GPIO_PIN_IS_OUTPUT)
!endif
!if "$(PIO_LED_ONLINE)" == ""
PIO_LED_ONLINE = (0+GPIO_DEVICE_ID_I2C_2+GPIO_NEGATIVE_POLARITY+GPIO_PIN_IS_OUTPUT)
!endif
!if "$(PIO_LED_ONLINE_MSG)" == ""
PIO_LED_ONLINE_MSG = (24+GPIO_DEVICE_ID_INTERNAL+GPIO_NEGATIVE_POLARITY+GPIO_PIN_IS_OUTPUT+GPIO_DEVICE_FLAG_PWM)
!endif
!if "$(PIO_LED_DOWNSTREAM_MSG)" == ""
PIO_LED_DOWNSTREAM_MSG = GPIO_INVALID
!endif
!if "$(PIO_LED_UPSTREAM_MSG)" == ""
PIO_LED_UPSTREAM_MSG = GPIO_INVALID
!endif
!if "$(PIO_LED_HARD_DISK)" == ""
PIO_LED_HARD_DISK = GPIO_INVALID
!endif
!if "$(LNB_22KHZ_CONTROL)" == ""
LNB_22KHZ_CONTROL = LNB_22KHZ_ENABLE
!endif
!if "$(PIO_LNB_ENABLE)" == ""
PIO_LNB_ENABLE = GPIO_INVALID
!endif
!if "$(PIO_LNB_22KHZ_DIRECTION)" == ""
PIO_LNB_22KHZ_DIRECTION = GPIO_INVALID
!endif
!if "$(SCART_TYPE)" == ""
SCART_TYPE = SCART_TYPE_TDK_AVPRO_5002B
!endif
!if "$(RF_MODULATOR)" == ""
RF_MODULATOR = NO
!endif
!if "$(PIO_RFMOD_ENABLE)" == ""
PIO_RFMOD_ENABLE = (-1)
!endif
!if "$(EXT_RF_MODULATOR_TYPE)" == ""
EXT_RF_MODULATOR_TYPE = EXT_RFMOD_TYPE_NONE
!endif
!if "$(EXTERNAL_PCI_IO_MODE)" == ""
EXTERNAL_PCI_IO_MODE = AUTOSENSE
!endif
!if "$(TS_MUX_TYPE)" == ""
TS_MUX_TYPE = TS_MUX_TYPE_KLONDIKE
!endif
!if "$(EXTERNAL_TS_SRC0)" == ""
EXTERNAL_TS_SRC0 = EXT_TS_SRC_BASEBAND_DVB
!endif
!if "$(EXTERNAL_TS_SRC1)" == ""
EXTERNAL_TS_SRC1 = EXT_TS_SRC_BASEBAND_OTV
!endif
!if "$(EXTERNAL_TS_SRC2)" == ""
EXTERNAL_TS_SRC2 = NOT_PRESENT
!endif
!if "$(EXTERNAL_TS_SRC3)" == ""
EXTERNAL_TS_SRC3 = NOT_PRESENT
!endif
!if "$(CRYSTAL_FREQUENCY)" == ""
CRYSTAL_FREQUENCY = 14318180
!endif
!if "$(OOB_IF_FREQUENCY)" == ""
OOB_IF_FREQUENCY = 36150000
!endif
!if "$(IB_IF_FREQUENCY)" == ""
IB_IF_FREQUENCY = 36150000
!endif
!if "$(CABLE_MODEM_TUNER_TYPE)" == ""
CABLE_MODEM_TUNER_TYPE = CABLE_MODEM_TUNER_TYPE_NONE
!endif
!if "$(MPEG_VIDEO_TUNER_TYPE)" == ""
MPEG_VIDEO_TUNER_TYPE = MPEG_VIDEO_TUNER_TYPE_NONE
!endif
!if "$(QAM_DEMOD_IF_TYPE)" == ""
QAM_DEMOD_IF_TYPE = QAM_DEMOD_IF_TYPE_NONE
!endif
!if "$(CM_IMG_PREFIX)" == ""
CM_IMG_PREFIX = SPARTA
!endif
!if "$(DAC_PLL_REG_VALUE)" == ""
DAC_PLL_REG_VALUE = 0
!endif
!if "$(MODEM_HW_TYPE)" == ""
MODEM_HW_TYPE = HW_NONE
!endif
!if "$(MODEM_PORT)" == ""
MODEM_PORT = UART_NONE
!endif
!if "$(MODEM_HOST_RESET)" == ""
MODEM_HOST_RESET = GPIO_INVALID
!endif
!if "$(MODEM_HOST_RESET2)" == ""
MODEM_HOST_RESET2 = GPIO_INVALID
!endif
!if "$(MODEM_HOST_INTERRUPT)" == ""
MODEM_HOST_INTERRUPT = GPIO_INVALID
!endif
!if "$(MODEM_HOST_MEMBASE)" == ""
MODEM_HOST_MEMBASE = NOT_PRESENT
!endif
!if "$(PLL_CLK_OBSERVATION_REG_DEFAULT)" == ""
PLL_CLK_OBSERVATION_REG_DEFAULT = NOT_DETERMINED
!endif
!if "$(MODEM_SW_TYPE)" == ""
MODEM_SW_TYPE = FLIPPER_V22B_MODEM
!endif
!if "$(SMC_VOLT_CTRL)" == ""
SMC_VOLT_CTRL = SMC_VOLT_CTRL_STD
!endif
!if "$(SMC_SLOT0)" == ""
SMC_SLOT0 = SMC_CONTROLLER0
!endif
!if "$(SMC_SLOT1)" == ""
SMC_SLOT1 = SMC_CONTROLLER1
!endif
!if "$(DESCRIPTOR_NUM)" == ""
DESCRIPTOR_NUM = 7
!endif
!if "$(CE1_CHIP_SELECT)" == ""
CE1_CHIP_SELECT = 3
!endif
!if "$(CE2_CHIP_SELECT)" == ""
CE2_CHIP_SELECT = SPARTA_CE2_CHIP_SELECT
!endif
!if "$(CARD_DETECT1_GPIO)" == ""
CARD_DETECT1_GPIO = SPARTA_CARD_DETECT1_GPIO
!endif
!if "$(CARD_DETECT2_GPIO)" == ""
CARD_DETECT2_GPIO = SPARTA_CARD_DETECT2_GPIO
!endif
!if "$(READ_REQUEST_GPIO)" == ""
READ_REQUEST_GPIO = SPARTA_READ_REQUEST_GPIO
!endif
!if "$(RESET_GPIO)" == ""
RESET_GPIO = SPARTA_RESET_GPIO
!endif
!if "$(VOLTAGE_SENSE1_GPIO)" == ""
VOLTAGE_SENSE1_GPIO = SPARTA_VOLTAGE_SENSE1_GPIO
!endif
!if "$(VOLTAGE_SENSE2_GPIO)" == ""
VOLTAGE_SENSE2_GPIO = SPARTA_VOLTAGE_SENSE2_GPIO
!endif
!if "$(USE_XOE)" == ""
USE_XOE = 1
!endif
!if "$(Alt_POD_PIO)" == ""
Alt_POD_PIO = SPARTA_Alt_POD_PIO
!endif
!if "$(A4_PIO50)" == ""
A4_PIO50 = SPARTA_A4_PIO50
!endif
!if "$(Alt_IO_RW)" == ""
Alt_IO_RW = 0
!endif
!if "$(VPP_ENABLE_GPIO)" == ""
VPP_ENABLE_GPIO = SPARTA_VPP_ENABLE_GPIO
!endif
!if "$(ALT_A14_GPIO)" == ""
ALT_A14_GPIO = SPARTA_ALT_A14_GPIO
!endif
!if "$(CHIP_REV)" == ""
CHIP_REV = AUTOSENSE
!endif
!if "$(I2C_BUS_0_SPEED)" == ""
I2C_BUS_0_SPEED = I2C_SPEED_100KHZ
!endif
!if "$(I2C_BUS_1_SPEED)" == ""
I2C_BUS_1_SPEED = I2C_SPEED_100KHZ
!endif
!if "$(I2C_ADDR_CX24121)" == ""
I2C_ADDR_CX24121 = 0x6A
!endif
!if "$(I2C_ADDR_CX24121_DSS)" == ""
I2C_ADDR_CX24121_DSS = 0xEA
!endif
!if "$(I2C_BUS_CX24121)" == ""
I2C_BUS_CX24121 = I2C_BUS_0
!endif
!if "$(I2C_ADDR_CAMARO)" == ""
I2C_ADDR_CAMARO = 0xAA
!endif
!if "$(I2C_BUS_CAMARO)" == ""
I2C_BUS_CAMARO = I2C_BUS_0
!endif
!if "$(CAMARO_ACCESS_METHOD)" == ""
CAMARO_ACCESS_METHOD = REGISTER_WORKAROUND
!endif
!if "$(I2C_ADDR_HM1221_1811)" == ""
I2C_ADDR_HM1221_1811 = 0xAE
!endif
!if "$(I2C_BUS_HM1221_1811)" == ""
I2C_BUS_HM1221_1811 = I2C_BUS_0
!endif
!if "$(I2C_ADDR_HM1221_SER)" == ""
I2C_ADDR_HM1221_SER = 0xA8
!endif
!if "$(I2C_BUS_HM1221_SER)" == ""
I2C_BUS_HM1221_SER = I2C_BUS_0
!endif
!if "$(I2C_ADDR_HM1221_PAR)" == ""
I2C_ADDR_HM1221_PAR = 0xAE
!endif
!if "$(I2C_BUS_HM1221_PAR)" == ""
I2C_BUS_HM1221_PAR = I2C_BUS_0
!endif
!if "$(I2C_ADDR_HM1221_SER0)" == ""
I2C_ADDR_HM1221_SER0 = 0xAE
!endif
!if "$(I2C_BUS_HM1221_SER0)" == ""
I2C_BUS_HM1221_SER0 = I2C_BUS_0
!endif
!if "$(I2C_ADDR_HM1221_SER1)" == ""
I2C_ADDR_HM1221_SER1 = 0xA8
!endif
!if "$(I2C_BUS_HM1221_SER1)" == ""
I2C_BUS_HM1221_SER1 = I2C_BUS_0
!endif
!if "$(I2C_ADDR_BT861)" == ""
I2C_ADDR_BT861 = 0x8A
!endif
!if "$(I2C_BUS_BT861)" == ""
I2C_BUS_BT861 = I2C_BUS_0
!endif
!if "$(I2C_ADDR_BT865)" == ""
I2C_ADDR_BT865 = 0x8A
!endif
!if "$(I2C_BUS_BT865)" == ""
I2C_BUS_BT865 = I2C_BUS_0
!endif
!if "$(I2C_ADDR_AVPRO5002B)" == ""
I2C_ADDR_AVPRO5002B = 0x90
!endif
!if "$(I2C_BUS_AVPRO5002B)" == ""
I2C_BUS_AVPRO5002B = I2C_BUS_0
!endif
!if "$(I2C_ADDR_CXA2161R)" == ""
I2C_ADDR_CXA2161R = 0x90
!endif
!if "$(I2C_BUS_CXA2161R)" == ""
I2C_BUS_CXA2161R = I2C_BUS_0
!endif
!if "$(I2C_ADDR_LCD_MATRIX)" == ""
I2C_ADDR_LCD_MATRIX = 0x5C
!endif
!if "$(I2C_BUS_LCD_MATRIX)" == ""
I2C_BUS_LCD_MATRIX = I2C_BUS_0
!endif
!if "$(I2C_ADDR_RF_MOD)" == ""
I2C_ADDR_RF_MOD = 0xCA
!endif
!if "$(I2C_BUS_RF_MOD)" == ""
I2C_BUS_RF_MOD = I2C_BUS_0
!endif
!if "$(I2C_ADDR_EEPROM1)" == ""
I2C_ADDR_EEPROM1 = 0xA0
!endif
!if "$(I2C_BUS_EEPROM1)" == ""
I2C_BUS_EEPROM1 = I2C_BUS_0
!endif
!if "$(I2C_ADDR_EEPROM2)" == ""
I2C_ADDR_EEPROM2 = 0xA2
!endif
!if "$(I2C_BUS_EEPROM2)" == ""
I2C_BUS_EEPROM2 = I2C_BUS_0
!endif
!if "$(I2C_CONFIG_EEPROM_ADDR)" == ""
I2C_CONFIG_EEPROM_ADDR = NOT_PRESENT
!endif
!if "$(I2C_CONFIG_EEPROM_BUS)" == ""
I2C_CONFIG_EEPROM_BUS = I2C_BUS_NONE
!endif
!if "$(I2C_ADDR_EEPROM3)" == ""
I2C_ADDR_EEPROM3 = NOT_PRESENT
!endif
!if "$(I2C_BUS_EEPROM3)" == ""
I2C_BUS_EEPROM3 = I2C_BUS_NONE
!endif
!if "$(I2C_ADDR_GPIO2_EXT)" == ""
I2C_ADDR_GPIO2_EXT = 0x72
!endif
!if "$(I2C_BUS_GPIO2_EXT)" == ""
I2C_BUS_GPIO2_EXT = I2C_BUS_0
!endif
!if "$(I2C_ADDR_NIM_EXT)" == ""
I2C_ADDR_NIM_EXT = 0x70
!endif
!if "$(I2C_BUS_NIM_EXT)" == ""
I2C_BUS_NIM_EXT = I2C_BUS_0
!endif
!if "$(I2C_ADDR_EEPROM_MDM)" == ""
I2C_ADDR_EEPROM_MDM = 0xA6
!endif
!if "$(I2C_BUS_EEPROM_MDM)" == ""
I2C_BUS_EEPROM_MDM = I2C_BUS_0
!endif
!if "$(I2C_ADDR_EEPROM_NADA)" == ""
I2C_ADDR_EEPROM_NADA = 0xA4
!endif
!if "$(I2C_BUS_EEPROM_NADA)" == ""
I2C_BUS_EEPROM_NADA = I2C_BUS_0
!endif
!if "$(I2C_ADDR_BT829)" == ""
I2C_ADDR_BT829 = NOT_PRESENT
!endif
!if "$(I2C_BUS_BT829)" == ""
I2C_BUS_BT829 = I2C_BUS_NONE
!endif
!if "$(I2C_ADDR_BT835)" == ""
I2C_ADDR_BT835 = NOT_PRESENT
!endif
!if "$(I2C_BUS_BT835)" == ""
I2C_BUS_BT835 = I2C_BUS_NONE
!endif
!if "$(I2C_ADDR_CX22702)" == ""
I2C_ADDR_CX22702 = 0x86
!endif
!if "$(I2C_BUS_CX22702)" == ""
I2C_BUS_CX22702 = I2C_BUS_0
!endif
!if "$(I2C_ADDR_CX22702_TUNER_DEVICE)" == ""
I2C_ADDR_CX22702_TUNER_DEVICE = 0xC0
!endif
!if "$(I2C_ADDR_MAKO)" == ""
I2C_ADDR_MAKO = 0x88
!endif
!if "$(I2C_BUS_MAKO)" == ""
I2C_BUS_MAKO = I2C_BUS_1
!endif
!if "$(ISA_CHIP_SELECT_1)" == ""
ISA_CHIP_SELECT_1 = 4
!endif
!if "$(ISA_CHIP_SELECT_2)" == ""
ISA_CHIP_SELECT_2 = 4
!endif
!if "$(ISA_CHIP_SELECT_3)" == ""
ISA_CHIP_SELECT_3 = 5
!endif
!if "$(ISA_CHIP_SELECT_4)" == ""
ISA_CHIP_SELECT_4 = 3
!endif
!if "$(CNXT_FIREWIRE_DESCRIPTOR)" == ""
CNXT_FIREWIRE_DESCRIPTOR = 6
!endif
!if "$(CNXT_FIREWIRE_CHIP_SELECT)" == ""
CNXT_FIREWIRE_CHIP_SELECT = 5
!endif
!if "$(CNXT_FIREWIRE_LLC_RESET_GPIO)" == ""
CNXT_FIREWIRE_LLC_RESET_GPIO = 102
!endif
!if "$(CNXT_FIREWIRE_ARM_RESET_GPIO)" == ""
CNXT_FIREWIRE_ARM_RESET_GPIO = 103
!endif
!if "$(FIREWIRE_DRIVER_INCLUDE)" == ""
FIREWIRE_DRIVER_INCLUDE = NO
!endif
!if "$(PIO_UART1_FLOW_CONTROL_OUT)" == ""
PIO_UART1_FLOW_CONTROL_OUT = GPIO_INVALID
!endif
!if "$(PIO_UART1_FLOW_CONTROL_IN)" == ""
PIO_UART1_FLOW_CONTROL_IN = GPIO_INVALID
!endif
!if "$(PIO_UART1_FLOW_READY_OUT)" == ""
PIO_UART1_FLOW_READY_OUT = GPIO_INVALID
!endif
!if "$(PIO_UART1_FLOW_READY_IN)" == ""
PIO_UART1_FLOW_READY_IN = GPIO_INVALID
!endif
!if "$(PIO_UART1_RING_INDICATE_IN)" == ""
PIO_UART1_RING_INDICATE_IN = GPIO_INVALID
!endif
!if "$(PIO_UART1_CARRIER_DETECT_IN)" == ""
PIO_UART1_CARRIER_DETECT_IN = GPIO_INVALID
!endif
!if "$(DMA_UART1_TX_CHANNEL)" == ""
DMA_UART1_TX_CHANNEL = DMA_DISABLED
!endif
!if "$(DMA_UART1_RX_CHANNEL)" == ""
DMA_UART1_RX_CHANNEL = DMA_DISABLED
!endif
!if "$(PIO_UART2_FLOW_CONTROL_OUT)" == ""
PIO_UART2_FLOW_CONTROL_OUT = GPIO_INVALID
!endif
!if "$(PIO_UART2_FLOW_CONTROL_IN)" == ""
PIO_UART2_FLOW_CONTROL_IN = GPIO_INVALID
!endif
!if "$(PIO_UART2_FLOW_READY_OUT)" == ""
PIO_UART2_FLOW_READY_OUT = GPIO_INVALID
!endif
!if "$(PIO_UART2_FLOW_READY_IN)" == ""
PIO_UART2_FLOW_READY_IN = GPIO_INVALID
!endif
!if "$(PIO_UART2_RING_INDICATE_IN)" == ""
PIO_UART2_RING_INDICATE_IN = GPIO_INVALID
!endif
!if "$(PIO_UART2_CARRIER_DETECT_IN)" == ""
PIO_UART2_CARRIER_DETECT_IN = GPIO_INVALID
!endif
!if "$(DMA_UART2_TX_CHANNEL)" == ""
DMA_UART2_TX_CHANNEL = DMA_DISABLED
!endif
!if "$(DMA_UART2_RX_CHANNEL)" == ""
DMA_UART2_RX_CHANNEL = DMA_DISABLED
!endif
!if "$(PIO_UART3_FLOW_CONTROL_OUT)" == ""
PIO_UART3_FLOW_CONTROL_OUT = GPIO_INVALID
!endif
!if "$(PIO_UART3_FLOW_CONTROL_IN)" == ""
PIO_UART3_FLOW_CONTROL_IN = GPIO_INVALID
!endif
!if "$(PIO_UART3_FLOW_READY_OUT)" == ""
PIO_UART3_FLOW_READY_OUT = GPIO_INVALID
!endif
!if "$(PIO_UART3_FLOW_READY_IN)" == ""
PIO_UART3_FLOW_READY_IN = GPIO_INVALID
!endif
!if "$(PIO_UART3_RING_INDICATE_IN)" == ""
PIO_UART3_RING_INDICATE_IN = GPIO_INVALID
!endif
!if "$(PIO_UART3_CARRIER_DETECT_IN)" == ""
PIO_UART3_CARRIER_DETECT_IN = GPIO_INVALID
!endif
!if "$(DMA_UART3_TX_CHANNEL)" == ""
DMA_UART3_TX_CHANNEL = DMA_DISABLED
!endif
!if "$(DMA_UART3_RX_CHANNEL)" == ""
DMA_UART3_RX_CHANNEL = DMA_DISABLED
!endif
!if "$(GRAPHICS_PLANES)" == ""
GRAPHICS_PLANES = GP_SINGLE
!endif
!if "$(OC_OOB_FREQ_ROLLOFF_SINC_TABLE_TYPE)" == ""
OC_OOB_FREQ_ROLLOFF_SINC_TABLE_TYPE = OC_OOB_FREQ_ROLLOFF_SINC_TABLE_NONE
!endif
!if "$(PIO_DP_HEADER_RESET)" == ""
PIO_DP_HEADER_RESET = GPIO_INVALID
!endif
!if "$(DVBCI_SUPPORT)" == ""
DVBCI_SUPPORT = NOT_PRESENT
!endif

# This file was generated from C:\CNXT_NUPBAS_1_8\SRC\CONFIGS\HWCONFIG.CFG on Thu Jun 17 11:21:04 2010 

#************************************************************
#* End of automatically generated configuration header file *
#************************************************************
