/****************************************************************************/
/*                          Conexant Systems Inc.                           */
/****************************************************************************/
/*                                                                          */
/* Filename:       GPIOEXT.H                                                */
/*                                                                          */
/* Description:    Definitions relating to the I2C GPIO Extender module     */
/*                                                                          */
/* Author:         Dave Wilson                                              */
/*                                                                          */
/* Copyright Conexant Systems Inc, 1999                                     */
/* All Rights Reserved.                                                     */
/*                                                                          */
/****************************************************************************/
/* $Id: gpioext.h,v 1.11, 2004-05-04 19:31:49Z, Angela Swartz$
 ****************************************************************************/

#ifndef _GPIOEXT_H_
#define _GPIOEXT_H_

/******************************************************************************/
/*                 THESE DEFINITIONS APPLY TO THOR BOARDS ONLY!               */
/******************************************************************************/
/* IIC PIO expander controls transport input and NIM voltage/tone             */
/* Current design:                                                            */
/*    NIM_VOLT_SEL on bit 7 (0=13V, 1=18V)                                    */
/*    NIM_VOLT_EN  on bit 6 (0=disabled)                                      */
/*    NIM_TONE_SEL on but 5 (0=no tone, 1=tone)                               */
/*    NIM_OTV_SEL  on bit 4 (0=NIM, 1=OTV baseband)                           */
/*    XPORT_SEL1   on bit 3  ] 00 = invalid - no source, 01 = CABLE MODEM     */
/*    XPORT_SEL0   on bit 2  ] 10 = OTV_NIM selection,   11 = DVB PARALLEL    */
/*    SCART_BLNK   on bit 1  (Thor 2 only)                                    */
/*    SCART_FSEL   on bit 0  (Thor 2 only)                                    */
/*                                                                            */
/*    SATELLITE NIM------> (NIM_OTV_SEL)                                      */
/*                       >----------------|                                   */
/*    OPENTV BASEBAND---->                |                                   */
/*                                         ---->                              */
/*    DVB PARALLEL-----------------------------> (XPORT_SEL[0:1])             */
/*                                             >------------------>transport  */
/*    CABLE MODEM------------------------------>                              */
/*                                                                            */
/******************************************************************************/

/* IIC GPIO Extender defines for Transport source/NIM/LNB */

/*** I2C addresses are now defined in the relevant vendor ****/
/*** header file                                          ****/

/**************************************/
/**************************************/
/**                                  **/
/**  Bit Definitions for Thor Boards **/
/**                                  **/
/**************************************/
/**************************************/

/* Masks for groups of bits within the first extender */
#define NIM_EXTENDER_BIT_MASK      0xFC
#define SCART_EXTENDER_BIT_MASK    0x03

#define SCART_EXTENDER_BLNK_MASK   0x01
#define SCART_EXTENDER_FSEL_MASK   0x02

#define NIM_EXTENDER_SRC_MASK      0x1C
#define NIM_EXTENDER_TONE_MASK     0x20
#define NIM_EXTENDER_VOLT_EN_MASK  0x40
#define NIM_EXTENDER_VOLT_MASK     0x80

/* Bit definitions within the first extender */
#define NIM_EXTENDER_SRC_NIM       0x08
#define NIM_EXTENDER_SRC_OTV       0x18
#define NIM_EXTENDER_SRC_DVB       0x1C
#define NIM_EXTENDER_SRC_CABMOD    0x14

#define NIM_EXTENDER_LNB_TONE      0x20
#define NIM_EXTENDER_LNB_HIVOLT    0x80
#define NIM_EXTENDER_LNB_VOLTEN    0x40

#define SCART_EXTENDER_FSEL_16_9 0x01
#define SCART_EXTENDER_FSEL_4_3  0x00
#define SCART_EXTENDER_BLNK_CVBS 0x02
#define SCART_EXTENDER_BLNK_RGB  0x00

/******************************************/
/******************************************/
/**                                      **/
/**  Bit Definitions for Klondike Boards **/
/**                                      **/
/******************************************/
/******************************************/

/* Extender 1 at I2C address 0x70 */
#define GPIO1_TSI_MUX_MASK           0x03
#define GPIO1_HSDP_IN_MUX_MASK       0x04
#define GPIO1_HSDP_OUT_MUX_MASK      0x08
#define GPIO1_DVBP_DIR_MASK          0x10
#define GPIO1_IDE_TRANSCEIVER_MASK   0x20
#define GPIO1_USB_VOLTAGE_MASK       0xC0

#define GPIO1_TSI_P_NIM              0x01
#define GPIO1_TSI_OTV_IN             0x02
#define GPIO1_TSI_DVB_IN             0x03

#define GPIO1_HSDP_IN_TSIN           0x00
#define GPIO1_HSDP_IN_HSDP           0x04

#define GPIO1_HSDP_OUT_TSIN          0x00
#define GPIO1_HSDP_OUT_4900          0x08

#define GPIO1_DVB_OUT                0x00
#define GPIO1_DVB_IN                 0x10

#define GPIO1_IDE_TXRX_ENABLE        0x00
#define GPIO1_IDE_TXRX_DISABLE       0x20

#define GPIO1_USB1_VOLTAGE_ENABLE    0x00
#define GPIO1_USB1_VOLTAGE_DISABLE   0x40
#define GPIO1_USB2_VOLTAGE_ENABLE    0x00
#define GPIO1_USB2_VOLTAGE_DISABLE   0x80

/* Extender 2 at I2C address 0x72 */
#define GPIO2_MODEM_LED_MASK         0x01
#define GPIO2_HSDP_CLK_MASK          0x02

#define GPIO2_MODEM_LED_ON           0x00
#define GPIO2_MODEM_LED_OFF          0x01

#define GPIO2_HSDP_CLK_MUX_ENABLED   0x00
#define GPIO2_HSDP_CLK_MUX_DISABLED  0x02

/*****************************************************/
/*****************************************************/
/**                                                 **/
/**  Bit Definitions for Abilene and Athens Boards  **/
/**                                                 **/
/*****************************************************/
/*****************************************************/

/* Smart card voltage control, one per slot                             */
/* Writing a 0 to this GPIO enables 3V operation on this slot (TDA8004) */
/* Writing a 1 to this GPIO enables 5V operation on this slot (TDA8004) */
#if SMC_VOLT_CTRL==SMC_VOLT_CTRL_EXT
#define PIN_GPIO_SMC_0_VOLTAGE_CTRL (NUM_OF_GPIO + 3)
#define PIN_GPIO_SMC_1_VOLTAGE_CTRL (NUM_OF_GPIO + 4)
#endif
#if SMC_VOLT_CTRL==SMC_VOLT_CTRL_EUREKA
#define PIN_GPIO_SMC_0_VOLTAGE_CTRL 51
#define PIN_GPIO_SMC_1_VOLTAGE_CTRL 52
#endif

#endif /* _GPIOEXT_H_ */

/****************************************************************************
 * Modifications:
 * $Log: 
 *  12   mpeg      1.11        5/4/04 2:31:49 PM      Angela Swartz   CR(s) 
 *        8940 8939 : Changed PIN_GPIO_SMC_0/1_VOLTAGE_CTRL to 
 *        NUM_OF_GPIO(defined in chip header) +3/4; They were used to be 
 *        hardcoded as 0x63 and 0x64. It was correct as long as the NUM_OF_GPIO
 *         is 96, with recent correction of NUM_OF_GPIO from 96 to 108, the 
 *        GPIO bits used for Smartcard no longer valid, as in gpio apis, any 
 *        bit beyond NUM_OF_GPIO is considered on the GPIO expander.
 *  11   mpeg      1.10        5/16/03 7:04:56 PM     Brendan Donahe  SCR(s) 
 *        6341 6340 :
 *        Added GPIO definitions for Eureka smart card voltage control
 *        
 *        
 *  10   mpeg      1.9         11/22/02 12:00:32 PM   Brendan Donahe  SCR(s) 
 *        4951 :
 *        Added GPIO expander pins for Abilene and Athens smart card voltage 
 *        control
 *        when TDA8004 is used.
 *        
 *        
 *  9    mpeg      1.8         9/3/02 7:35:58 PM      Matt Korte      SCR(s) 
 *        4498 :
 *        Remove warnings
 *        
 *  8    mpeg      1.7         12/19/01 9:57:02 AM    Tim Ross        SCR(s) 
 *        2933 :
 *        Backed out changed made by Senthil to eliminate previous board 
 *        definitions.
 *        
 *  7    mpeg      1.6         12/19/01 9:23:10 AM    Tim Ross        SCR(s) 
 *        2933 :
 *        Added #endif to GPIO_CONFIG #if set. 
 *        
 *  6    mpeg      1.5         12/18/01 3:54:14 PM    Senthil Veluswamy SCR(s) 
 *        2933 :
 *        Merged Wabash Branch Changes (removed Thor Defines)
 *        
 *  5    mpeg      1.4         9/25/00 1:01:50 PM     Dave Wilson     Fixed up 
 *        I2C addresses for Vendor D
 *        Moved PIO definitions for LEDs into relevant vendor header files
 *        
 *  4    mpeg      1.3         4/13/00 10:10:24 AM    Dave Wilson     Minor 
 *        change (one line moved?)
 *        
 *  3    mpeg      1.2         3/28/00 5:59:10 PM     Dave Wilson     Added 
 *        definitions of GPIO extender bits for both units on Klondike
 *        
 *  2    mpeg      1.1         9/1/99 10:36:04 AM     Dave Wilson     Added a 
 *        few more bit mask definitions.
 *        
 *  1    mpeg      1.0         8/11/99 6:15:24 PM     Dave Wilson     
 * $
 *
 ****************************************************************************/


