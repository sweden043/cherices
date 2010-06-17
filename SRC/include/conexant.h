/****************************************************************************/
/*                   CONEXANT PROPRIETARY AND CONFIDENTIAL                  */
/*                       SOFTWARE FILE/MODULE HEADER                        */
/*          Conexant Systems Inc. (c) 1998, 1999, 2000, 2001, 2002          */
/*                              Austin, TX                                  */
/*                         All Rights Reserved                              */
/****************************************************************************/
/*
 * Filename:        CONEXANT.H
 *
 *
 * Description:     Public header file containing Conexant Internal
 *                  board specific hardware configuration values
 *
 *
 * Author:          Tim White
 *
 ****************************************************************************/
/* $Header: conexant.h, 39, 11/1/03 2:50:53 PM, Tim Ross$
 ****************************************************************************/

#include "warnfix.h"

#ifndef _CONEXANT_H_
#define _CONEXANT_H_

#define MAX_BOARD_TYPES     12

#define BOARD_ID_MASK         0x7F

/*********************************************************************/
/* Values read directly from the hardware config register. These are */
/* returned by the KAL function read_board_and_vendor_codes.         */
/*********************************************************************/
#define BOARD_ORION           0x00
#define BOARD_THOR1_R1_EUR    0x01
#define BOARD_THOR1_R1_USA    0x02
#define BOARD_THOR1_R3_EUR    0x03
#define BOARD_THOR1_R3_USA    0x04
#define BOARD_THOR2_R1_EUR    0x05
#define BOARD_THOR2_R1_USA    0x06
#define BOARD_ORION2          0x40
#define BOARD_BRADY           0x7E

/* Note for Klondike: This board uses 4 bits with 3 indicating populated   */
/* features. To test for any Klondike, AND the board ID read with          */
/* KLONDIKE_MASK then check that the value is the same as BOARD_KLONDIKE.  */
/* To check for specific populated options, check the bottom 3 bits of the */
/* board ID if the Klondike check passes.                                  */
/* ie. Any board id with value 0001xxxb is a Klondike.                     */

#define BOARD_KLONDIKE        0x08
#define KLONDIKE_MASK         0x78
#define KLONDIKE_PARALLEL_NIM 0x01
#define KLONDIKE_AFE_2P_SER   0x02
#define KLONDIKE_SMARTDAA     0x04

/* Note for Abilene: This board uses 2 bits with 1 indicating smartcard1   */
/* functionality. To test for any Abilene, AND the board ID read with      */
/* ABILENE_MASK then check that the value is the same as BOARD_ABILENE.    */
/* ie. Any board id with the value 001xb is an Abilene. (Notice that these */
/* IDs are shared with THOR1_R1_USA and THOR1_R3_EUR.)                     */

#define BOARD_ABILENE         0x02
#define ABILENE_MASK          0x0e
#define ABILENE_SC1           0x01

/***************************************/
/* Conexant Config Bit Definitions for */
/* accessing register as a DWORD       */
/***************************************/
#define RSO_BOARD_ID            0x0000007f
#define RSO_FLASH_DESC          0x00007800
#define RSO_RAM_CFG             0x00180000

#define RSO_RAM_SHIFT           19

/***************************************/
/* Thor Config Bit Definitions for     */
/* accessing register as a DWORD       */
/***************************************/
#define RSO_AUDIO_ID            0x00078000
#define RSO_VIDEO_FMT           0x06000000
#define RSO_DVBCI_PRESENT       0x04000000

#define RSO_AUDIO_ID_SHIFT      15
#define RSO_AUDIO_ID_THUNDER00  0
#define RSO_AUDIO_ID_THUNDER01  1

#define RSO_VIDEO_SHIFT         24
#define RSO_VIDEO_NTSC          0
#define RSO_VIDEO_PAL           1
#define RSO_VIDEO_SECAM         2

#define VIDFORMAT_NTSC      0
#define VIDFORMAT_PAL       1
#define VIDFORMAT_SECAM     2

/***************************************/
/* Bronco Config Bit Definitions for   */
/* accessing register as a DWORD       */
/***************************************/

/******************************************************/
/* Conexant Internal Bronco configuration definitions */
/******************************************************/
#define     PLL_CONFIG0_MEMORY_BUS_WIDTH_MASK                      0x00000010
#define     PLL_CONFIG0_MEMORY_BUS_WIDTH_SHIFT                            4
#define         PLL_CONFIG0_MEMORY_BUS_16BITS_WIDE                   (0UL<<4)
#define         PLL_CONFIG0_MEMORY_BUS_32BITS_WIDE                   (1UL<<4)


/***************************************/
/* Klondike Config Bit Definitions for */
/* accessing register as a DWORD       */
/***************************************/

/********************************************************/
/* Conexant Internal Klondike configuration definitions */
/********************************************************/
#define     PLL_CONFIG0_CNXT_KLONDIKE_BOARD_ID_MASK                0x0000007F
#define     PLL_CONFIG0_CNXT_KLONDIKE_BOARD_ID_SHIFT                      0
#define     PLL_CONFIG0_CNXT_KLONDIKE_VENDOR_ID_MASK               0x00000780
#define     PLL_CONFIG0_CNXT_KLONDIKE_VENDOR_ID_SHIFT                     7
#define     PLL_CONFIG0_CNXT_KLONDIKE_FLASH_DESC_MASK              0x00007800
#define     PLL_CONFIG0_CNXT_KLONDIKE_FLASH_DESC_SHIFT                   11
#define     PLL_CONFIG0_CNXT_KLONDIKE_PLL_MON_ENABLE_MASK          0x00008000
#define     PLL_CONFIG0_CNXT_KLONDIKE_PLL_MON_ENABLE_SHIFT               15
#define     PLL_CONFIG0_CNXT_KLONDIKE_RAM_SPEED_MASK               0x00070000
#define     PLL_CONFIG0_CNXT_KLONDIKE_RAM_SPEED_SHIFT                    16
#define     PLL_CONFIG0_CNXT_KLONDIKE_RAM_TYPE_MASK                0x00180000
#define     PLL_CONFIG0_CNXT_KLONDIKE_RAM_TYPE_SHIFT                     19
#define     PLL_CONFIG0_CNXT_KLONDIKE_IDE_PRESENT_MASK             0x00200000
#define     PLL_CONFIG0_CNXT_KLONDIKE_IDE_PRESENT_SHIFT                  21
#define     PLL_CONFIG0_CNXT_KLONDIKE_CI_RESET_MASK                0x00800000
#define     PLL_CONFIG0_CNXT_KLONDIKE_CI_RESET_SHIFT                     23
#define     PLL_CONFIG0_CNXT_KLONDIKE_ROM0_XOE_MASK                0x01000000
#define     PLL_CONFIG0_CNXT_KLONDIKE_ROM0_XOE_SHIFT                     24
#define     PLL_CONFIG0_CNXT_KLONDIKE_RANK0_CHIP_SIZE_MASK         0x02000000
#define     PLL_CONFIG0_CNXT_KLONDIKE_RANK0_CHIP_SIZE_SHIFT              25
#define     PLL_CONFIG0_CNXT_KLONDIKE_RANK1_CHIP_SIZE_MASK         0x04000000
#define     PLL_CONFIG0_CNXT_KLONDIKE_RANK1_CHIP_SIZE_SHIFT              26
#define     PLL_CONFIG0_CNXT_KLONDIKE_ROM_BANK0_WIDTH_MASK         0x18000000
#define     PLL_CONFIG0_CNXT_KLONDIKE_ROM_BANK0_WIDTH_SHIFT              27
#define     PLL_CONFIG0_CNXT_KLONDIKE_DBG_REQ_DBG_ACK_MASK         0x20000000
#define     PLL_CONFIG0_CNXT_KLONDIKE_DBG_REQ_DBG_ACK_SHIFT              29
#define     PLL_CONFIG0_CNXT_KLONDIKE_SIX_CHAN_AUDIO_MASK          0x40000000
#define     PLL_CONFIG0_CNXT_KLONDIKE_SIX_CHAN_AUDIO_SHIFT               30
#define     PLL_CONFIG0_CNXT_KLONDIKE_PLL_BYPASS_MASK              0x80000000
#define     PLL_CONFIG0_CNXT_KLONDIKE_PLL_BYPASS_SHIFT                   31

#ifdef BITFIELDS
typedef struct _PLL_CONFIG0_CNXT_KLONDIKE
{
  BIT_FLD BoardID:7,           /*  0 - 6  */
          VendorID:4,          /*  7 - 10 */
          FlashDescription:4,  /* 11 - 14 */
          NotPllMonitor:1,     /* 15      */
          RAMSpeed:3,          /* 16 - 18 */
          RAMType:2,           /* 19 - 20 */
          NoIDE:1,             /* 21      */
          Reserved1:1,         /* 22      */
          NoCIReset:1,         /* 23      */
          ROM0XOEMask:1,       /* 24      */
          Rank0ChipSize:1,     /* 25      */
          Rank1ChipSize:1,     /* 26      */
          ROMBank0Width:2,     /* 27 - 28 */
          DbgReq_DbgAck:1,     /* 29      */
          SixChanAudio:1,      /* 30      */
          PLLBypass:1;         /* 31      */
} PLL_CONFIG0_CNXT_KLONDIKE;
typedef PLL_CONFIG0_CNXT_KLONDIKE volatile *LPPLL_CONFIG0_CNXT_KLONDIKE;
#endif /* BITFIELDS */

/****************************************************************/
/* Generic Board Configuration definitions(found in EEPROM)     */
/****************************************************************/
#define    BOARD_CONFIG_VERSION_CODE_OFFSET            0x00000000
#define    BOARD_CONFIG_VERSION_CODE_SIZE                     1
#define    BOARD_CONFIG_LENGTH_OFFSET                  0x00000001
#define    BOARD_CONFIG_LENGTH_SIZE                           3
#define    BOARD_CONFIG_CHECKSUM_OFFSET                0x00000004
#define    BOARD_CONFIG_CHECKSUM_SIZE                         2
#define    BOARD_CONFIG_INV_CHECKSUM_OFFSET            0x00000006
#define    BOARD_CONFIG_INV_CHECKSUM_SIZE                     2
#define    BOARD_CONFIG_XTAL_SPEED_OFFSET              0x00000008
#define    BOARD_CONFIG_XTAL_SPEED_SIZE                       4
#define    BOARD_CONFIG_CUSTOMER_ID_OFFSET             0x0000000C
#define    BOARD_CONFIG_CUSTOMER_ID_SIZE                      1
#define    BOARD_CONFIG_BOARD_TYPE_OFFSET              0x0000000D
#define    BOARD_CONFIG_BOARD_TYPE_SIZE                       1
#define    BOARD_CONFIG_BOARD_REV_OFFSET               0x0000000E
#define    BOARD_CONFIG_BOARD_REV_SIZE                        1
#define    BOARD_CONFIG_DAC_MUX_OUT_OFFSET             0x0000000F
#define    BOARD_CONFIG_DAC_MUX_OUT_SIZE                      1
#define    BOARD_CONFIG_MODEM_CONFIG_OFFSET            0x00000010
#define    BOARD_CONFIG_MODEM_CONFIG_SIZE                     1
#define    BOARD_CONFIG_SPUR_SC1_OPTION_OFFSET         0x00000011
#define    BOARD_CONFIG_SPUR_SC1_OPTION_SIZE                  1

#define    BOARD_CONFIG_BOARD_TYPE_BRONCO                     0
#define    BOARD_CONFIG_BOARD_TYPE_BRONCO_DVT                 1
#define    BOARD_CONFIG_BOARD_TYPE_EUREKA                     2
#define    BOARD_CONFIG_BOARD_TYPE_MILANO                     3

typedef struct _CONFIG_TABLE
{
   unsigned long  xtal_speed;
   unsigned long  length;
   unsigned short checksum;
   unsigned short inv_checksum;
   unsigned char  version_code;
   unsigned char  customer_id;
   unsigned char  board_type;
   unsigned char  board_rev;
   unsigned char  dac_mux_out;
   unsigned char  modem_config;
   unsigned char  spur_sc1_option;
} CONFIG_TABLE;
typedef CONFIG_TABLE *LPCONFIG_TABLE;

/*******************************************************/
/* Generic Conexant Internal configuration definitions */
/*******************************************************/
#define     PLL_CONFIG0_CNXT_GENERIC_BOARD_ID_MASK                 0x0000007F
#define     PLL_CONFIG0_CNXT_GENERIC_BOARD_ID_SHIFT                       0
#define     PLL_CONFIG0_CNXT_GENERIC_VENDOR_ID_MASK                0x00000780
#define     PLL_CONFIG0_CNXT_GENERIC_VENDOR_ID_SHIFT                      7
#define     PLL_CONFIG0_CNXT_GENERIC_FLASH_DESC_MASK               0x00007800
#define     PLL_CONFIG0_CNXT_GENERIC_FLASH_DESC_SHIFT                    11
#define     PLL_CONFIG0_CNXT_GENERIC_BOARD_SPECIFIC1_MASK          0x00078000
#define     PLL_CONFIG0_CNXT_GENERIC_BOARD_SPECIFIC1_SHIFT               15
#define     PLL_CONFIG0_CNXT_GENERIC_RAM_TYPE_MASK                 0x00180000
#define     PLL_CONFIG0_CNXT_GENERIC_RAM_TYPE_SHIFT                      19
#define     PLL_CONFIG0_CNXT_GENERIC_BOARD_SPECIFIC2_MASK          0x00600000
#define     PLL_CONFIG0_CNXT_GENERIC_BOARD_SPECIFIC2_SHIFT               21
#define     PLL_CONFIG0_CNXT_GENERIC_PLL_BYPASS_CLK_SEL_MASK       0x00800000
#define     PLL_CONFIG0_CNXT_GENERIC_PLL_BYPASS_CLK_SEL_SHIFT            23
#define     PLL_CONFIG0_CNXT_GENERIC_ROM0_XOE_MASK                 0x01000000
#define     PLL_CONFIG0_CNXT_GENERIC_ROM0_XOE_SHIFT                      24
#define     PLL_CONFIG0_CNXT_GENERIC_BOARD_SPECIFIC3_MASK          0x06000000
#define     PLL_CONFIG0_CNXT_GENERIC_BOARD_SPECIFIC3_SHIFT               25
#define     PLL_CONFIG0_CNXT_GENERIC_ROM_BANK0_WIDTH_MASK          0x18000000
#define     PLL_CONFIG0_CNXT_GENERIC_ROM_BANK0_WIDTH_SHIFT               27
#define     PLL_CONFIG0_CNXT_GENERIC_DBG_REQ_DBG_ACK_MASK          0x20000000
#define     PLL_CONFIG0_CNXT_GENERIC_DBG_REQ_DBG_ACK_SHIFT               29
#define     PLL_CONFIG0_CNXT_GENERIC_SIX_CHAN_AUDIO_MASK           0x40000000
#define     PLL_CONFIG0_CNXT_GENERIC_SIX_CHAN_AUDIO_SHIFT                30
#define     PLL_CONFIG0_CNXT_GENERIC_PLL_BYPASS_MASK               0x80000000
#define     PLL_CONFIG0_CNXT_GENERIC_PLL_BYPASS_SHIFT                    31

/* For hondo and wabash, bit 16 indicates whether the chip 
 * should come up in PCI mode (0) or I/O mode (1). */
#define     PLL_CONFIG0_CNXT_GENERIC_IO_PCI_MODE_MASK              0x00010000
#define     PLL_CONFIG0_CNXT_GENERIC_IO_PCI_MODE_SHIFT                   16

/* For wabash, this pin indicates whether a TDA8004 (smart card driver) is 
 * installed (0) or not (1). */
#define     PLL_CONFIG0_CNXT_GENERIC_AUD_DOUT_LR_MASK              0x40000000
#define     PLL_CONFIG0_CNXT_GENERIC_AUD_DOUT_LR_SHIFT                   30
#define     PLL_CONFIG0_CNXT_GENERIC_8004_MASK                     0x40000000
#define     PLL_CONFIG0_CNXT_GENERIC_8004_SHIFT                          30

#ifdef BITFIELDS
typedef struct _PLL_CONFIG0_CNXT_GENERIC
{
  BIT_FLD BoardID:7,           /*  0 - 6  */
          VendorID:4,          /*  7 - 10 */
          FlashDescription:4,  /* 11 - 14 */
          BoardSpecific1:4,    /* 15 - 18 */
          RAMType:2,           /* 19 - 20 */
          BoardSpecific2:2,    /* 21 - 22 */
          PLLBypassClockSel:1, /* 23      */
          ROM0XOEMask:1,       /* 24      */
          BoardSpecific3:2,    /* 25 - 26 */
          ROMBank0Width:2,     /* 27 - 28 */
          DbgReq_DbgAck:1,     /* 29      */
          SixChanAudio:1,      /* 30      */
          PLLBypass:1;         /* 31      */
} PLL_CONFIG0_CNXT_GENERIC;
typedef PLL_CONFIG0_CNXT_GENERIC volatile *LPPLL_CONFIG0_CNXT_GENERIC;

typedef union _PLL_CONFIG0_CNXT
{
  PLL_CONFIG0_CNXT_GENERIC    Generic;
  PLL_CONFIG0_CNXT_KLONDIKE   Klondike;
} PLL_CONFIG0_CNXT;
typedef PLL_CONFIG0_CNXT volatile *LPPLL_CONFIG0_CNXT;
#endif /* BITFIELDS */

/***********************/
/* Vendor & Board ID's */
/***********************/
extern unsigned char VendorID;
extern unsigned char BoardID;

#define ISKLONDIKE ((VendorID==CONEXANT)?(((BoardID&KLONDIKE_MASK)==BOARD_KLONDIKE)?1:0):0)
#define ISABILENE ((VendorID==CONEXANT)?(((BoardID&ABILENE_MASK)==BOARD_ABILENE)?1:0):0)

#if (CUSTOMER != VENDOR_B) && (CUSTOMER != VENDOR_C)
/* Settings for audio deemphasis */
#define AUD_DEEMP_ON_GPIO             TRUE  /* Set GPIO/Demp to Deemp */
#define AUD_DEEMP_ENABLE_DEFAULT      0     /* 0=do not get from stream */
                                            /* if 0, ignore active low bit and mode */
#define AUD_DEEMP_ACTIVE_LOW_DEFAULT  1     /* 1=Active Low Deemp */
#define AUD_DEEMP_VALUE_DEFAULT       1     /* Default value if enable=0 */
                                            /* Ignores active low bit */
#define AUD_DEEMP_MODE_DEFAULT        AUD_DEEMP_STANDARD_NONE
#endif /* CUSTOMER != VENDOR_B && CUSTOMER != VENDOR_C */

/* Internal development-only default PID's for Dish Network free-to-air feeds */
#define CNXT_DISHNW_USA_100_VIDEO_PID           0x1722
#define CNXT_DISHNW_USA_100_AUDIO_PID           0x1723
#define CNXT_DISHNW_USA_101_VIDEO_PID           0x1122
#define CNXT_DISHNW_USA_101_AUDIO_PID           0x1123
#define CNXT_DISHNW_USA_500_VIDEO_PID           0x1B22
#define CNXT_DISHNW_USA_500_AUDIO_PID           0x1B23

/*******************************************
 * Abilene PIO Expander definitions        *
 *******************************************/

#define CNXT_AB_PIO_EXP_REG                        0x31380000
#define CNXT_AB_PIO_EXP_FP_ROW_1_OUT               0x00000001
#define CNXT_AB_PIO_EXP_FP_ROW_2_OUT               0x00000002
#define CNXT_AB_PIO_EXP_FP_ROW_3_OUT               0x00000004
#define CNXT_AB_PIO_EXP_TDA8004_SM0_VSEL_MASK      0x00000008
#define CNXT_AB_PIO_EXP_TDA8004_SM0_VSEL_5V        0x00000008
#define CNXT_AB_PIO_EXP_TDA8004_SM0_VSEL_3V        0x00000000
#define CNXT_AB_PIO_EXP_TDA8004_SM1_VSEL_MASK      0x00000010
#define CNXT_AB_PIO_EXP_TDA8004_SM1_VSEL_5V        0x00000010
#define CNXT_AB_PIO_EXP_TDA8004_SM1_VSEL_3V        0x00000000
#define CNXT_AB_PIO_EXP_ON_STBY_LED_MASK           0x00000020
#define CNXT_AB_PIO_EXP_ON_STBY_LED_STBY           0x00000020
#define CNXT_AB_PIO_EXP_ON_STBY_LED_ON             0x00000000
#define CNXT_AB_PIO_EXP_IR_REC_LED_MASK            0x00000040
#define CNXT_AB_PIO_EXP_IR_REC_LED_OFF             0x00000040
#define CNXT_AB_PIO_EXP_IR_REC_LED_ON              0x00000000
#define CNXT_AB_PIO_EXP_MODEM_ONLINE_LED_MASK      0x00000080
#define CNXT_AB_PIO_EXP_MODEM_ONLINE_LED_OFF       0x00000080
#define CNXT_AB_PIO_EXP_MODEM_ONLINE_LED_ON        0x00000000
#define CNXT_AB_PIO_EXP_9PIN_SER_CTS_OUT           0x00000100
#define CNXT_AB_PIO_EXP_9PIN_SER_DSR_OUT           0x00000200
#define CNXT_AB_PIO_EXP_9PIN_SER_RI_OUT            0x00000400
#define CNXT_AB_PIO_EXP_9PIN_SER_DCD_OUT           0x00000800
#define CNXT_AB_PIO_EXP_SMRT_MDP_SCM_RST_MASK      0x00001000
#define CNXT_AB_PIO_EXP_SMRT_MDP_SCM_RST           0x00000000
#define CNXT_AB_PIO_EXP_TDA8006_RESET_MASK         0x00002000
#define CNXT_AB_PIO_EXP_TDA8006_RESET              0x00002000
#define CNXT_AB_PIO_EXP_HSDP0_SER_MUX_SEL_MASK     0x00004000
#define CNXT_AB_PIO_EXP_HSDP0_SER_MUX_SEL_SERDVB   0x00004000
#define CNXT_AB_PIO_EXP_HSDP0_SER_MUX_SEL_PARHS0   0x00000000
#define CNXT_AB_PIO_EXP_EN_1394_CLK_TO_HSDP0_MASK  0x00008000
#define CNXT_AB_PIO_EXP_EN_1394_CLK_TO_HSDP0_1394  0x00008000
#define CNXT_AB_PIO_EXP_EN_1394_CLK_TO_HSDP0_OTH   0x00000000

#endif

/****************************************************************************
 * Modifications:
 * $Log: 
 *  39   mpeg      1.38        11/1/03 2:50:53 PM     Tim Ross        CR(s): 
 *        7719 7762 Added BOARD_CONFIG_BOARD_TYPE_xxx definitions.
 *  38   mpeg      1.37        5/9/03 4:25:46 PM      Lucy C Allevato SCR(s) 
 *        6241 6242 :
 *        add board id for brady
 *        
 *  37   mpeg      1.36        4/30/03 10:19:34 AM    Bobby Bradford  SCR(s) 
 *        6037 :
 *        Changes to support new modem configuration parameters ...
 *        specifically ... move the modem setup strings to the
 *        appropriate 'C' source code file that uses them.
 *        
 *  36   mpeg      1.35        3/25/03 6:48:04 PM     Craig Dry       SCR(s) 
 *        5873 :
 *        Added CONFIG_TABLE C structure to hold configuration data read from
 *        eeprom during startup.
 *        
 *  35   mpeg      1.34        3/20/03 3:43:38 PM     Craig Dry       SCR(s) 
 *        5830 :
 *        Define field offsets and sizes for Board Configuration data read
 *        from EEPROM.
 *        
 *  34   mpeg      1.33        3/18/03 4:13:34 PM     Bobby Bradford  SCR(s) 
 *        5108 :
 *        Removed definitions for P93 modem hardware (now defined in
 *        CONFIG files)
 *        
 *  33   mpeg      1.32        1/23/03 4:27:50 PM     Tim White       SCR(s) 
 *        5298 :
 *        Add config strapping bit definition for Bronco IRD for determining 
 *        whether
 *        16bit memory bus or 32bit memory bus is used.
 *        
 *        
 *  32   mpeg      1.31        1/13/03 1:58:22 PM     Bobby Bradford  SCR(s) 
 *        5108 :
 *        Add default definitions for BRONCO (Brazos or Kobuk) SmartMDP
 *        
 *  31   mpeg      1.30        11/26/02 4:00:00 PM    Dave Wilson     SCR(s) 
 *        4902 :
 *        Removed all definitions of device I2C addresses. These now live in 
 *        the 
 *        respective target board's config file.
 *        
 *  30   mpeg      1.29        10/7/02 6:46:16 PM     Brendan Donahe  SCR(s) 
 *        4749 :
 *        Added PLL_CONFIG0_REG bit definitions for TDA8006
 *        
 *        
 *  29   mpeg      1.28        9/17/02 5:55:46 PM     Carroll Vance   SCR(s) 
 *        4613 :
 *        Added name of PCI vs. IO mode bit in PLL conig 0 register for Hondo 
 *        and Wabash chips.
 *        
 *  28   mpeg      1.27        5/1/02 3:30:46 PM      Tim White       SCR(s) 
 *        3673 :
 *        Remove PLL_ bitfields usage from codebase.
 *        
 *        
 *  27   mpeg      1.26        4/29/02 4:46:40 PM     Larry Wang      SCR(s) 
 *        3585 :
 *        define PIO_LED_xxx in config files instead of this file.
 *        
 *  26   mpeg      1.25        2/19/02 3:36:34 PM     Bob Van Gulick  SCR(s) 
 *        3173 :
 *        Change dish network free to error pids to new values
 *        
 *        
 *  25   mpeg      1.24        8/28/01 1:43:56 PM     Angela          SCR(s) 
 *        2430 :
 *        added LED PIO assignment on Abilene
 *        
 *  24   mpeg      1.23        8/7/01 3:40:24 PM      Angela          SCR(s) 
 *        2437 :
 *        For Hondo/MDP, use GPIO27 as interrupt and bit 12 in GPIO expander as
 *         reset bit
 *        
 *  23   mpeg      1.22        7/9/01 6:22:22 PM      Tim White       SCR(s) 
 *        2234 2235 2236 :
 *        Hondo fixes allowing boot loaders to function.
 *        
 *        
 *  22   mpeg      1.21        7/3/01 1:31:58 PM      Tim White       SCR(s) 
 *        2178 2179 2180 :
 *        Fixed board ID issue.  Now have separate MAX_NUM_BOARDS with
 *        separate vendor lists.
 *        
 *        
 *  21   mpeg      1.20        7/3/01 10:43:28 AM     Tim White       SCR(s) 
 *        2178 2179 2180 :
 *        Merge branched Hondo specific code back into the mainstream source 
 *        database.
 *        
 *        
 *  20   mpeg      1.19        5/1/01 3:06:44 PM      Angela          
 *        DCS#1679-added modem initial strings for Softmodem
 *        
 *  19   mpeg      1.18        4/4/01 12:20:54 PM     Tim White       
 *        DCS#1631,DCS#1632,DCS#1633 Merge.
 *        
 *  18   mpeg      1.17        3/14/01 2:47:54 PM     Angela          DCS#1431-
 *         wrapped the audio deemphasis defines not to be included for 
 *        vendor_c build due to the same reason as the previous revision
 *        
 *  17   mpeg      1.16        3/8/01 4:37:52 PM      Senthil Veluswamy wrapped
 *         the audio deemphasis defines to not be included for vendor_b. These 
 *        values are already defined in the vendor specific header and presence
 *         in both
 *        files was causing multi-page warnings.
 *        
 *  16   mpeg      1.15        2/28/01 12:41:52 PM    Steve Glennon   Reset 
 *        audop deemphasis to off. Report from Vendor A that turning it on
 *        reduces high freq by 10dB which is unacceptable
 *        
 *  15   mpeg      1.14        2/26/01 2:18:42 PM     Steve Glennon   Added 
 *        #defines to control Deemphasis settings DCS#1312 (Vendor A) but
 *        for Conexant box
 *        
 *  14   mpeg      1.13        2/12/01 11:28:22 AM    Angela          added 
 *        MODEM_SETUP_COMMANDS strings for Cmodule(DCS#1177)
 *        
 *  13   mpeg      1.12        2/12/01 10:23:16 AM    Joe Kroesche    #946 - 
 *        added accomodation for softmodem
 *        
 *  12   mpeg      1.11        1/12/01 11:06:24 AM    Angela          unified 
 *        BASIC_PREFIX_1 & 2(SRC935)
 *        
 *  11   mpeg      1.10        1/11/01 4:16:36 PM     Angela          added 
 *        BASIC_PREFIX_1 & 2 defines for Modem
 *        
 *  10   mpeg      1.9         12/5/00 5:32:04 PM     Tim White       Changed 
 *        I2C_ADDR_HM1221_1811 to be set to 0xAE for Conexant builds.
 *        Both NIM's (serial and parallel) operate at 0xAE.  If there are two
 *        NIM's in a box, then the parallel NIM is 0xAE and the serial NIM
 *        is 0xA8.
 *        
 *  9    mpeg      1.8         10/30/00 6:25:32 PM    Joe Kroesche    added 
 *        some mdp defines to try to clean up builds for smartmdp
 *        
 *  8    mpeg      1.7         9/25/00 1:02:10 PM     Dave Wilson     Fixed up 
 *        I2C addresses for Vendor D
 *        Moved PIO definitions for LEDs into relevant vendor header files
 *        
 *  7    mpeg      1.6         9/18/00 12:02:56 PM    Tim White       Echostar 
 *        is still fiddling with transponder frequencies and PID's.
 *        Addd channel 101 PID's for the 12.340640 frequency.  Also it looks
 *        like channel 500 is under 12.253160 and channel 100 seems to have
 *        disappeared.  OpenTV En2 cannot find it.
 *        
 *  6    mpeg      1.5         9/14/00 8:12:16 PM     Tim White       Added 
 *        global default development/internal_use_only PID's for DIsh Network
 *        office feed since they changed the PID's last night (09/14/2000).
 *        
 *  5    mpeg      1.4         9/1/00 2:22:24 PM      Joe Kroesche    added 
 *        some defined for SmartMDP modem
 *        
 *  4    mpeg      1.3         7/13/00 1:39:06 PM     Dave Wilson     Added I2C
 *         address definitions (other code not yet updated to use them!)
 *        
 *  3    mpeg      1.2         4/27/00 4:21:30 PM     Tim White       Fixed bug
 *         in board definitions.
 *        
 *  2    mpeg      1.1         4/24/00 10:32:36 PM    Tim White       Removed 
 *        the RSO_ definitions from chip specific header file and
 *        placed into vendor/board specific header file.
 *        
 *  1    mpeg      1.0         4/24/00 11:18:28 AM    Tim White       
 * $
 * 
 *    Rev 1.37   09 May 2003 15:25:46   goldenx
 * SCR(s) 6241 6242 :
 * add board id for brady
 * 
 *    Rev 1.36   30 Apr 2003 09:19:34   bradforw
 * SCR(s) 6037 :
 * Changes to support new modem configuration parameters ...
 * specifically ... move the modem setup strings to the
 * appropriate 'C' source code file that uses them.
 * 
 *    Rev 1.35   25 Mar 2003 18:48:04   dryd
 * SCR(s) 5873 :
 * Added CONFIG_TABLE C structure to hold configuration data read from
 * eeprom during startup.
 * 
 *    Rev 1.34   20 Mar 2003 15:43:38   dryd
 * SCR(s) 5830 :
 * Define field offsets and sizes for Board Configuration data read
 * from EEPROM.
 * 
 *    Rev 1.33   18 Mar 2003 16:13:34   bradforw
 * SCR(s) 5108 :
 * Removed definitions for P93 modem hardware (now defined in
 * CONFIG files)
 * 
 *    Rev 1.32   23 Jan 2003 16:27:50   whiteth
 * SCR(s) 5298 :
 * Add config strapping bit definition for Bronco IRD for determining whether
 * 16bit memory bus or 32bit memory bus is used.
 * 
 * 
 *    Rev 1.31   13 Jan 2003 13:58:22   bradforw
 * SCR(s) 5108 :
 * Add default definitions for BRONCO (Brazos or Kobuk) SmartMDP
 * 
 *    Rev 1.30   26 Nov 2002 16:00:00   dawilson
 * SCR(s) 4902 :
 * Removed all definitions of device I2C addresses. These now live in the 
 * respective target board's config file.
 * 
 *    Rev 1.29   07 Oct 2002 17:46:16   donaheb
 * SCR(s) 4749 :
 * Added PLL_CONFIG0_REG bit definitions for TDA8006
 * 
 * 
 *    Rev 1.0   07 Oct 2002 17:39:06   donaheb
 * SCR(s) 4749 :
 * Added PLL_CONFIG0_REG bit definitions for Wabash TDA8004
 * 
 * 
 *    Rev 1.28   17 Sep 2002 16:55:46   vancec
 * SCR(s) 4613 :
 * Added name of PCI vs. IO mode bit in PLL conig 0 register for Hondo and Wabash chips.
 * 
 *    Rev 1.27   01 May 2002 14:30:46   whiteth
 * SCR(s) 3673 :
 * Remove PLL_ bitfields usage from codebase.
 * 
 *
 ****************************************************************************/

