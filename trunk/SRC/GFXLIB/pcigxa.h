/***************************************************************************
                                                                          
   Filename: PCIGXA.H

   Description: PCI Graphics Hardware Development Definitions

   Created: 9/15/1999 by Eric Ching

   Copyright Conexant Systems, 1999
   All Rights Reserved.

****************************************************************************/
/***************************************************************************
$Header: pcigxa.h, 2, 2/13/03 12:22:30 PM, Matt Korte$
****************************************************************************/

#ifndef _PCIGXA_H_
#define _PCIGXA_H_

#ifndef _SABINE_H_
#define INCL_GXA
#endif
#include "stbcfg.h"

#ifdef PCI_GXA

#define IO_BASE 0x31000000

/* The following defines are needed if using a PCI 2164 card */

/**************************************************************************
 *      VGA Graphics Registers
 **************************************************************************/
#define GRP_INDEX     0x3ce
#define GRP_DATA      0x3cf
#define GRP_SR        0x00    // set/reset
#define GRP_ESR       0x01    // enable set/reset
#define GRP_CC        0x02    // color compare
#define GRP_ROT       0x03    // rotate register
#define GRP_RDPLN     0x04    // map select
#define GRP_MODE      0x05    // mode select
#define GRP_MISC      0x06    // miscellaneous
#define GRP_CCX       0x07    // color don't care
#define GRP_BITMASK   0x08    // bit mask

/*************************************************************************
 *      VGA Graphics Extension Registers
 *************************************************************************/
#define GRP_VGACFG    0x16    // "1" sets DAC to 8 bit mode(6 default)
#define GRP_CID_0     0x17    // Shiner chip ID(week)
#define GRP_CID_1     0x18    // Shiner chip ID(year)
#define GRP_ROMPAGE   0x19    // Flash ROM page select(32K/1mbyte)
#define GRP_25PLL_0   0x1a    // PLL coefficients 25mhz mode
#define GRP_25PLL_1   0x1b    // PLL coefficients 25mhz mode
#define GRP_25PLL_2   0x1c    // PLL coefficients 25mhz mode
#define GRP_28PLL_0   0x1d    // PLL coefficients 28mhz mode
#define GRP_28PLL_1   0x1e    // PLL coefficients 28mhz mode
#define GRP_28PLL_2   0x1f    // PLL coefficients 28mhz mode
#define GRP_GUI_0     0x20    // base address, 8mbyte frame apert.
#define GRP_GUI_1     0x21    // base address, 8mbyte frame apert.
#define GRP_GUI_2     0x22    // base address, 8mbyte frame apert.
#define GRP_GUI_3     0x23    // base address, 8mbyte frame apert.
#define GRP_PDAC_0    0x26    // video PACDAC access through VRAM
#define GRP_PDAC_1    0x27    // video PACDAC access through VRAM
#define GRP_RDLAT_0   0x28    // VGA read latch(LSByte)
#define GRP_RDLAT_1   0x29    // VGA read latch
#define GRP_RDLAT_2   0x2a    // VGA read latch
#define GRP_RDLAT_3   0x2b    // VGA read latch(MSByte)
#define GRP_VGASTAT   0x2c    // VGA status
/************ Amber additions ******************/
#define GRP_PCI_EN    0x2d    // Amber pci snoop dac, io0,io1
#define GRP_IRQA_LINE 0x2e    // Amber R/O pci config funct 1 IRQ
/************************************************/
#define GRP_CFG_0     0x40    // Shiner configuration
#define GRP_CFG_1     0x41    // Shiner configuration
#define GRP_CFG_2     0x42    // Shiner configuration
#define GRP_CFG_3     0x43    // Shiner configuration
#define GRP_CFG_4     0x44    // Shiner configuration
#define GRP_CFG_5     0x45    // Shiner configuration
#define GRP_CFG_6     0x46    // Shiner configuration
#define GRP_CFG_7     0x47    // Shiner configuration
/************ Amber additions ******************/
#define GRP_CFG_8     0x48    // Amber  configuration
#define GRP_CFG_9     0x49    // Amber  configuration
#define GRP_CFG_A     0x4A    // Amber  configuration
/************************************************/
#define GRP_I2C_CTRL  0x4B    // "1" enables I2C master
#define GRP_I2C_SCTRL 0x4c    // I2C slave control
#define GRP_I2C_SDATA 0x4d    // I2C slave data
#define GRP_I2C_MCTRL 0x4e    // I2C master control
#define GRP_I2C_MDATA 0x4f    // I2C master data
#define GRP_I2C_FESR  0x50    // AMDAC SAR AND FESR PORT.
/************************************************/
#define PDC_MSPTRA_0  0x57    // Master structure A pointer in VRAM
#define PDC_MSPTRA_1  0x58    // Master structure A pointer in VRAM
#define PDC_MSPTRA_2  0x59    // Master structure A pointer in VRAM
#define PDC_MSPTRB_0  0x5a    // Master structure B pointer in VRAM
#define PDC_MSPTRB_1  0x5b    // Master structure B pointer in VRAM
#define PDC_MSPTRB_2  0x5c    // Master structure B pointer in VRAM
#define PDC_STAT      0x5d    // read PACDAC controller status
#define PDC_CNTL      0x5e    // PACDAC control
#define PDC_BREAK     0x5f    // PACDAC controller Break point register
/************ Amber additions ******************/
#define SCT_SYNCHRO     0xb5    // vretrace synchronizer flag
#define GRP_PIO_ENABLE  0xc0    // R/W PIO enable
#define GRP_PIO_DATA    0xc1    // R/W PIO data
/************************************************/

/*************************************************************************
 *      VGA Sequencer Registers
 *************************************************************************/
#define SEQ_INDEX     0x3c4   // sequencer index
#define SEQ_DATA      0x3c5   // sequencer data
#define SEQ_RST       0x00    // reset register('0' for mode set)
#define SEQ_CLK       0x01    // clocking mode(refresh,dot clk)
#define SEQ_WPMASK    0x02    // write access to video planes
#define SEQ_CFS       0x03    // character font select
#define SEQ_MMODE     0x04    // memory mode(buff&seq control)
#define SEQ_UNLOCK    0x06    // enable to Shiner extension regs

/**************************************************************************
 *      VGA CRT Controller
 **************************************************************************/
#define CRT_INDEX_C     0x3d4   // index for color
#define CRT_INDEX_M     0x3b4   // index for monochrome
#define CRT_DATA_C      0x3d5   // data for color
#define CRT_DATA_M      0x3b5   // data for monochrome
#define CRT_HTOT        0x00    // horizontal total
#define CRT_HDSP_END    0x01    // # displayed chars/row
#define CRT_HBLANK_ST   0x02    // horizontal blank start(char)
#define CRT_HBLANK_END  0x03    // horizontal blank end(char)
#define CRT_HSYNC_ST    0x04    // horizonal sync start(char)
#define CRT_HSYNC_END   0x05    // horizontal sync end(char)
#define CRT_VTOT        0x06    // Vertical sync total(scan lines)
#define CRT_OVERFLOW    0x07    // miscellaneous vertical bits
#define CRT_PRE_RS      0x08    // preset row scanning reg(scrolling)
#define CRT_CHEIGHT     0x09    // character cell height
#define CRT_CUR_ST      0x0a    // cursor scan line start
#define CRT_CUR_END     0x0b    // cursor scan line end
#define CRT_SCREEN_STH  0x0c    // video buffer start(top left corner)
#define CRT_SCREEN_STL  0x0d    // video buffer start(top left corner)
#define CRT_CUR_LOCH    0x0e    // cursor screen address
#define CRT_CUR_LOCL    0x0f    // cursor screen address
#define CRT_VSYNC_ST    0x10    // vsync start
#define CRT_VSYNC_END   0x11    // vsync end
#define CRT_VDSP_END    0x12    // last displayed line on screen
#define CRT_OFFSET      0x13    // line pitch in bytes
#define CRT_UNDERLINE   0x14    // scan line location for underlining
#define CRT_VBLANK_ST   0x15    // vblank start
#define CRT_VBLANK_END  0x16    // vblank end
#define CRT_MODE_CTRL   0x17    // mode control
#define CRT_LINECMP     0x18    // for split screens

/*************************************************************************
 *              VGA Attribute Controller
 *************************************************************************/
#define ATT_ADDR        0x3c0   // attribute index/data port
#define ATT_RD          0x3c1   // readonly, attribute data
#define ATT_PAL_REG_0   0x00    // palette register 0
#define ATT_PAL_REG_1   0x01    // palette register 1
#define ATT_PAL_REG_2   0x02    // palette register 2
#define ATT_PAL_REG_3   0x03    // palette register 3
#define ATT_PAL_REG_4   0x04    // palette register 4
#define ATT_PAL_REG_5   0x05    // palette register 5
#define ATT_PAL_REG_6   0x06    // palette register 6
#define ATT_PAL_REG_7   0x07    // palette register 7
#define ATT_PAL_REG_8   0x08    // palette register 8
#define ATT_PAL_REG_9   0x09    // palette register 9
#define ATT_PAL_REG_a   0x0a    // palette register a
#define ATT_PAL_REG_b   0x0b    // palette register b
#define ATT_PAL_REG_c   0x0c    // palette register c
#define ATT_PAL_REG_d   0x0d    // palette register d
#define ATT_PAL_REG_e   0x0e    // palette register e
#define ATT_PAL_REG_f   0x0f    // palette register f
#define ATT_MODE        0x10    // mode control
#define ATT_OVRS        0x11    // overscan color register
#define ATT_CPE         0x12    // color plane enable
#define ATT_HPAN        0x13    // horizontal panning
#define ATT_CLRSEL      0x14    // color select

/**************************************************************************
 *              VGA General Registers
 **************************************************************************/
#define MCA_POS         0x102   // POS 102 enable
#define ADAPT_SLEEP     0x46e8  // adapter sleep

/*************************************************************************
 *                    GUI Defines
 *************************************************************************/
#define MBA_DIRECT     0x00800000l     //direct frame buffer
#define MBA_FLIPPIN    0x01000000l     //flippin frame buffer
#define MBA_FUNKY      0x01800000l     //funky frame buffer

#define PM_BASE            0x4c        //protected mode GUI base reg 3
#define PM_ENABLE          0x01        //protected mode enable bit reg 3

/* GUI Fifo Control Register */
// This register only exists in the 2164 PCI hardware
#define GFIFO_CTRL_REG     (GXA_BASE | 0x00f8)   //GUI fifo setup
#define DQ_FIFO_ENABLE     0x80000000l
#define DQ_BASE_MASK       0x0FFF0000l
#define EN_FIFO_WR         0x40000000l
#define GUI_FREEZE         0x00008000l
#define GUI_STEP           0x00004000l

// For 2164 PCI card the bitmap context registers are named
// and used slightly differently, but the offsets are the same
#define BMAP0_PITCH_REG    BMAP0_ADDR_REG      //bitmap context
#define BMAP1_PITCH_REG    BMAP1_ADDR_REG      //bitmap context
#define BMAP2_PITCH_REG    BMAP2_ADDR_REG      //bitmap context
#define BMAP3_PITCH_REG    BMAP3_ADDR_REG      //bitmap context
#define BMAP4_PITCH_REG    BMAP4_ADDR_REG      //bitmap context
#define BMAP5_PITCH_REG    BMAP5_ADDR_REG      //bitmap context
#define BMAP6_PITCH_REG    BMAP6_ADDR_REG      //bitmap context
#define BMAP7_PITCH_REG    BMAP7_ADDR_REG      //bitmap context

/* GUI Configuration Register */
#define DEPTH_4BPP           0x010000
#define DEPTH_8BPP           0x020000
#define DEPTH_16BPP_0565     0x040000
#define DEPTH_16BPP_1555     0x050000
#define DEPTH_24BPP          0x060000
#define DEPTH_32BPP          0x070000

#endif // PCI_GXA

#endif // _PCIGXA_H_
/***************************************************************************
                     PVCS Version Control Information
$Log: 
 2    mpeg      1.1         2/13/03 12:22:30 PM    Matt Korte      SCR(s) 5479 
       :
       Removed old header reference
       
 1    mpeg      1.0         3/6/00 1:40:48 PM      Lucy C Allevato 
$
 * 
 *    Rev 1.1   13 Feb 2003 12:22:30   kortemw
 * SCR(s) 5479 :
 * Removed old header reference
 * 
 *    Rev 1.0   06 Mar 2000 13:40:48   eching
 * Initial revision.
****************************************************************************/
