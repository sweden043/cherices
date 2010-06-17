/****************************************************************************/
/*                   CONEXANT PROPRIETARY AND CONFIDENTIAL                  */
/*                       SOFTWARE FILE/MODULE HEADER                        */
/*          Conexant Systems Inc. (c) 1998, 1999, 2000, 2001, 2002          */
/*                              Austin, TX                                  */
/*                         All Rights Reserved                              */
/****************************************************************************/
/*
 * Filename:        WABASH.H
 *
 *
 * Description:     Public header file defining hardware-specific values
 *                  (such as register addresses, bit definitions, etc)
 *                  for IRD central IC codenamed Wabash.
 *
 *
 * Author:          Tim Ross
 *
 ****************************************************************************/
/* $Id: wabash.h,v 1.148.1.0, 2004-06-17 21:23:26Z, Tim White$
 ****************************************************************************/

#ifndef _WABASH_H_
#define _WABASH_H_

#include "hwconfig.h"
#include "swconfig.h"

/*
 * Enable BITFIELDS #define only if code requires hardware register
 * bitfields support.  For Conexant ADC internal, if you require this
 * to be set, please send email.
 */
/*#define BITFIELDS*/

/****************************************************************************/
/* This header contains definitions for all registers and their contents as */
/* both #defines and C-bitfield types.  Compiler directives are used to     */
/* control the module definitions included.                                 */
/*                                                                          */
/* Naming conventions                                                       */
/* ------------------                                                       */
/*                                                                          */
/* Definitions of register addresses are relative to a base address         */
/* for the module. Base addresses are defined as xxx_BASE where xxx is      */
/* module identifier listed below.                                          */
/*                                                                          */
/* Register address xxx_<description>_REG                                   */
/*                                                                          */
/* Register definition typedef xxx_<description> (ie. register address      */
/* without the _REG).                                                       */
/*                                                                          */
/* For example the Display Refresh Module control register is at address    */
/* DRM_CONTROL_REG and is accessed using C type DRM_CONTROL.                */
/*                                                                          */
/* Module Identifiers                                                       */
/* ------------------                                                       */
/*                                                                          */
/* Definitions are grouped by device under the following headings:          */
/*                                                                          */
/* Device                      Prefix          Include #define              */
/* ------                      ------          ---------------              */
/* Miscellaneous                MSC_              INCL_MSC                  */
/* ARM processor                ARM_              INCL_ARM                  */
/* Graphics Coprocessor         GCP_              INCL_GCP                  */
/* Display Refresh Module       DRM_              INCL_DRM                  */
/* Movie Input                  MOV_              INCL_MOV                  */
/* Audio                        AUD_              INCL_AUD                  */
/* PLLs and Clocking            PLL_              INCL_PLL                  */
/* Reset and Power Management   RST_              INCL_RST                  */
/* Interrupt Controller         ITC_              INCL_ITC                  */
/* Timers                       TIM_              INCL_TIM                  */
/* I2C                          I2C_              INCL_I2C                  */
/* General Purpose IO           GPI_              INCL_GPI                  */
/* IEEE1284 (parallel)          PAR_              INCL_PAR                  */
/* Infra Red                    IRD_              INCL_IRD                  */
/* UART (serial)                SER_              INCL_SER                  */
/* Smart Card                   SMC_              INCL_SMC                  */
/* PCI                          PCI_              INCL_PCI                  */
/* MPEG                         MPG_              INCL_MPG                  */
/* DVB Parser                   DPS_              INCL_DPS                  */
/* HSDP's                       HSDP_             INCL_HSDP                 */
/* Real Time Clock              RTC_              INCL_RTC                  */
/* Modem Analog Front End       AFE_              INCL_AFE                  */
/* SDRAM registers              SDR_              INCL_SDR                  */
/* Pulse Timer                  PLS_              INCL_PLS                  */
/* Synchronous Serial           SYN_              INCL_SYN                  */
/* ASX Bridge/DMA Engine        DMA_              INCL_DMA                  */
/* Pulse Width Modulator        PWM_              INCL_PWM                  */
/* Graphics Acceleration        GXA_              INCL_GXA                  */
/* Conditional Access UART      CAM_              INCL_CAM                  */
/* Non-Cacheable RAM            NCR_                                        */
/* PCI IO Space                 PIO_                                        */
/* Internal TV Encoder          ENC_              INCL_ENC                  */
/* Internal QAM Demodulator     DMD_              INCL_DMD                  */
/*                                                                          */
/****************************************************************************/

/****************************/
/* Chip Feature Definitions */
/****************************/
#define INTERNAL_ENCODER                INTERNAL_BT861_LIKE
#define INTERNAL_DEMOD                  NOT_PRESENT
#define CPU_TYPE                        CPU_ARM920T
#define MMU_TYPE                        MMU_TYPE_920T
#define GPIO_CONFIG                     GPIOM_WABASH
#define NUM_PARSERS                     3
#define NUM_OSD_PLANES                  2
#define HAS_INTERNAL_PCI32              YES
#define HAS_EXTERNAL_PCI32              YES
#define HAS_PCI32                       (HAS_INTERNAL_PCI32 || HAS_EXTERNAL_PCI32)
#define HSDP_TYPE                       HSDP_WABASH
#define UART_TYPE                       UART_WABASH
#ifndef TRACE_PORT
  #define TRACE_PORT                    SERIAL1
#endif
#define ATA_TYPE                        ATA_WABASH
#define CPU_TYPE                        CPU_ARM920T
#define PLL_TYPE                        PLL_TYPE_WABASH
#define MPG_SYNC_BIT_POSITION           MPG_SYNC_BIT_POSITION_WABASH
#define HSX_ARBITER                     HSX_ARBITER_WABASH

#define INTEXP_TYPE                     INTEXP_WABASH
#define IIC_TYPE                        IIC_TYPE_WABASH
#define VIDEO_UCODE_DOWNLOAD_TYPE       VIDEO_UCODE_DOWNLOAD_COLORADO
#define DRM_SCALER_COEFF_TYPE           DRM_SCALER_COEFF_TYPE_WABASH
#define DRM_CURSOR_FETCH_TYPE           DRM_CURSOR_FETCH_TYPE_BRAZOS
#define DRM_HAS_WIPE_CHROMA_BUG         YES
#define DRM_NEEDS_SPLIT_FIFO_ON_WIPE    NO
#define DRM_HAS_UPSCALED_PAL_STILL_BUG  YES
#define DRM_HAS_16_BIT_CURSOR_BUG       NO
#define DRM_FILTER_PHASE_0_PROBLEM      YES
#define DRM_TILE_TYPE                   DRM_TILE_TYPE_COLORADO
#define ENCODER_HAS_DAC_BUG             NO 
#define PCI_INTERRUPT_CONFLICT_BUG      YES

#define PCM_AUDIO_TYPE                  PCM_AUDIO_WABASH

#define VIDEO_UCODE_RAM_SIZE_WORDS      1472

#define CHIP_SUPPORTS_PIO_TIMED_RESET   NO
#define CHIP_SUPPORTS_SMARTCARD_RESET   NO

#ifndef PLL_PRESCALE_VALUE
  #define PLL_PRESCALE_VALUE            0x000003ff
#endif

/******************************************************************************/
/* For Wabash (and future) chips, there are bits in the PCI_ROM_MODE register */
/* which allow setting some PCI delay values.                                 */
/******************************************************************************/
#define PCI_ROM_MODE_SUPPORTS_PCI_SYNC_BITS   1

/****************************************************************************/
/* This arty bit is to ensure that something is included. If none of the    */
/* conditional compilation flags is defined, INCL_ALL is automatically      */
/* defined, hence including all the definitions.                            */
/****************************************************************************/
#ifndef INCL_ALL
  #ifndef INCL_MSC
    #ifndef INCL_ARM
      #ifndef INCL_GCP
        #ifndef INCL_DRM
          #ifndef INCL_MOV
            #ifndef INCL_AUD
              #ifndef INCL_PLL
                #ifndef INCL_RST
                  #ifndef INCL_ITC
                    #ifndef INCL_TIM
                      #ifndef INCL_I2C
                        #ifndef INCL_GPI
                          #ifndef INCL_PAR
                            #ifndef INCL_IRD
                              #ifndef INCL_SER
                                #ifndef INCL_SMC
                                  #ifndef INCL_PCI
                                    #ifndef INCL_MPG
                                      #ifndef INCL_DPS
                                        #ifndef INCL_HSDP
                                          #ifndef INCL_RTC
                                            #ifndef INCL_AFE
                                              #ifndef INCL_SDR
                                                #ifndef INCL_PLS
                                                  #ifndef INCL_SYN
                                                    #ifndef INCL_DMA
                                                      #ifndef INCL_PWM
                                                        #ifndef INCL_GXA
                                                          #ifndef INCL_CAM
                                                            #ifndef INCL_ENC
                                                              #ifndef INCL_DMD
                                                                #define INCL_ALL
                                                              #endif
                                                            #endif
                                                          #endif
                                                        #endif
                                                      #endif
                                                    #endif
                                                  #endif
                                                #endif
                                              #endif
                                            #endif
                                          #endif
                                        #endif
                                      #endif
                                    #endif
                                  #endif
                                #endif
                              #endif
                            #endif
                          #endif
                        #endif
                      #endif
                    #endif
                  #endif
                #endif
              #endif
            #endif
          #endif
        #endif
      #endif
    #endif
  #endif
#endif

#ifdef INCL_ALL
  #define INCL_MSC
  #define INCL_ARM
  #define INCL_GCP
  #define INCL_DRM
  #define INCL_MOV
  #define INCL_AUD
  #define INCL_PLL
  #define INCL_RST
  #define INCL_ITC
  #define INCL_TIM
  #define INCL_I2C
  #define INCL_GPI
  #define INCL_PAR
  #define INCL_IRD
  #define INCL_SER
  #define INCL_SMC
  #define INCL_PCI
  #define INCL_MPG
  #define INCL_DPS
  #define INCL_HSDP
  #define INCL_RTC
  #define INCL_AFE
  #define INCL_SDR
  #define INCL_SYN
  #define INCL_PLS
  #define INCL_DMA
  #define INCL_PWM
  #define INCL_GXA
  #define INCL_CAM
  #define INCL_ENC
  #define INCL_DMD
  #define INCL_CM_PERIPHERALS
#endif

/***********************************/
/* General Defines used everywhere */
/***********************************/
#define BSWAP_OFFSET      0x08000000
#define ROM_START_ADDRESS 0x20000000

/*************************************************************************/
/* Useful macro in setting flags in pointers without generating warnings */
/*************************************************************************/
#define SET_FLAGS(x,flags)    (((HW_DWORD)x) | ((HW_DWORD)flags))
#define CLEAR_FLAGS(x,flags)  (((HW_DWORD)x) & (~(HW_DWORD)flags))

/*********************************/
/* General Types used everywhere */
/*********************************/
typedef volatile HW_DWORD *LPREG;
/*typedef unsigned int BIT_FLD;*/

/***************************/
/* Register Base Addresses */
/***************************/

/* Updated 3/1/99 */

#define MSC_BASE  0x00000000
#define ARM_BASE  0x30060000
#define GCP_BASE  0x00000000
#define DRM_BASE  0x30460000
#define MOV_BASE  0x30410000
#define AUD_BASE  0x30590000
#define PLL_BASE  0x30440000
#define RST_BASE  0x30000000
#define ITC_BASE  0x30450000
#define TIM_BASE  0x30430000
#if IIC_TYPE == IIC_TYPE_COLORADO
#define I2C_BASE  0x30480000
#else /* IIC_TYPE == IIC_TYPE_COLORADO */
#define I2C_BASE  0x305E0000
#endif /* IIC_TYPE == IIC_TYPE_COLORADO */
#define GPI_BASE  0x30470000
#define PAR_BASE  0x30510000
#define IRD_BASE  0x30560000
#define SER_BASE  0x30540000
#define SER_BASE_UART1  0x30540000
#define SER_BASE_UART2  0x30550000
#define SER_BASE_UART3  0x305D0000
#define SMC_BASE  0x30520000
#define PCI_BASE  0x30010000
#define MPG_BASE  0x30400000
#define RTC_BASE  0x30490000
#define AFE_BASE  0x30570000
#define DPS0_BASE 0x38000000
#define DPS1_BASE 0x3A000000 /* Not Used */
#define DPS2_BASE 0x3C000000
#define DPS3_BASE 0x3E000000
#define DPS_BASE(instance) (HW_DWORD)(DPS0_BASE+((instance)<<25))
#define SDR_BASE  0x30070000
#define BUFF_BASE 0x00000000
#define NCR_BASE  0x10000000
#define ROM_BASE  0x20000000
#define PIO_BASE  0x31000000
#define PLS_BASE  0x305B0000
#define SYN_BASE  0x305A0000
#define DMA_BASE  0x30500000
#define PWM_BASE  0x30580000
#ifdef PCI_GXA
#define GXA_BASE  0x4c000000
#else
#define GXA_BASE  0x304a0000
#endif
#define HSDP_BASE 0x304cc000
#define ENC_BASE  0x304d0000

#define TIM_BANK_SIZE 0x0010
#define NUM_TIM_BANKS 8

#define IRD_BANK_SIZE 0x01000
#define NUM_IRD_BANKS 2

#define SER_BANK_SIZE 0x10000
#define NUM_SER_BANKS 2

#define GPI_BANK_SIZE 0x30
#if (PCI_INTERRUPT_CONFLICT_BUG == YES) /* Right now only this uses the 4th bank */
 #define NUM_GPI_BANKS 4
#else /* (PCI_INTERRUPT_CONFLICT_BUG != YES) */
 #define NUM_GPI_BANKS 3
#endif /* (PCI_INTERRUPT_CONFLICT_BUG == YES) */

#define SMC_BANK_SIZE 0x10000
#define NUM_SMC_BANKS 2

#define IIC_BANK_SIZE 0x0100
#define NUM_IIC_BANKS 1

#define NONCACHEABLE_RAM_MASK (~NCR_BASE)
#define CACHE_LINE_SIZE 32

/*****************/
/* Miscellaneous */
/*****************/

#ifdef INCL_MSC

/* To be completed */

#endif /* INCL_MSC */

/****************/
/* ARM 940 Core */
/****************/

#ifdef INCL_ARM

/* Coprocessor 15 - Cache Control Unit */

/* Register Numbers */
#define ARM_CP15_ID_REG               0
#define ARM_CP15_CONFIG_REG           1
#define ARM_CP15_CACHEABLE_REG        2
#define ARM_CP15_WRITE_CONTROL_REG    3
#define ARM_CP15_ACCESS_PERM_REG      5
#define ARM_CP15_BASE_CONTROL_REG     6
#define ARM_CP15_CACHE_OPS_REG        7
#define ARM_CP15_CACHE_LOCKDOWN_REG   9
#define ARM_CP15_TEST_REG             15

#define ARMACCESS_NONE      0
#define ARMACCESS_SUPER     1
#define ARMACCESS_USERREAD  2
#define ARMACCESS_FULL      3

#ifdef BITFIELDS
typedef struct _ARM_CP15_ID
{
  BIT_FLD Revision:4,
          PartNum:12,
          Architecture:8,
          Implementor:8;
} ARM_CP15_ID;
typedef volatile ARM_CP15_ID *LPARM_CP15_ID;

typedef struct _ARM_CP15_CONFIG
{
  BIT_FLD EnableProt:1,
          Reserved1:1,    /* Write with 0 */
          DCacheEnable:1,
          Reserved2:4,    /* Write with 1 */
          BigEnd:1,
          Reserved3:4,    /* Write with 0 */
          ICacheEnable:1,
          AltVectors:1,
          Reserved4:16,   /* Write with 0 */
          nFastBus:1,
          AsyncClock:1;
} ARM_CP15_CONFIG;
typedef ARM_CP15_CONFIG volatile *LPARM_CP15_CONFIG;

typedef struct _ARM_CP15_CACHEABLE
{
  BIT_FLD C_0:1,
          C_1:1,
          C_2:1,
          C_3:1,
          C_4:1,
          C_5:1,
          C_6:1,
          C_7:1,
          Reserved1:24;
} ARM_CP15_CACHEABLE;
typedef ARM_CP15_CACHEABLE volatile *LPARM_CP15_CACHEABLE;

typedef struct _ARM_CP15_WRITE_CONTROL
{
  BIT_FLD B_d0:1,
          B_d1:1,
          B_d2:1,
          B_d3:1,
          B_d4:1,
          B_d5:1,
          B_d6:1,
          B_d7:1,
          Reserved1:24;
} ARM_CP15_WRITE_CONTROL;
typedef ARM_CP15_WRITE_CONTROL volatile *LPARM_CP15_WRITE_CONTROL;

typedef struct _ARM_CP15_ACCESS_PERM
{
  BIT_FLD ap0:2,
          ap1:2,
          ap2:2,
          ap3:2,
          ap4:2,
          ap5:2,
          ap6:2,
          ap7:2,
          Reserved1:16;
} ARM_CP15_ACCESS_PERM;
typedef ARM_CP15_ACCESS_PERM volatile *LPARM_CP15_ACCESS_PERM;

typedef struct _ARM_CP15_BASE_CONTROL
{
  BIT_FLD Enable:1,
          Size:5,
          Base:20;
} ARM_CP15_BASE_CONTROL;
typedef ARM_CP15_BASE_CONTROL volatile *LP_ARM_CP15_BASE_CONTROL;

/* Register 7 Index/Segment Format */
typedef struct _ARM_CP15_CACHE_OPS
{
  BIT_FLD Reserved1:4,   /* Write with 0 */
          Segment:2,
          Reserved2:20,  /* Write with 0 */
          Index:6;
} ARM_CP15_CACHE_OPS;
typedef ARM_CP15_CACHE_OPS volatile *LPARM_CP15_CACHE_OPS;

/* Register 7 Prefetch Address Format */
typedef struct _ARM_CP15_CACHE_PREFETCH
{
  BIT_FLD Reserved1:4,   /* Write with 0 */
        CacheSegment:2,
        Address:26;
} ARM_CP15_CACHE_PREFETCH;
typedef ARM_CP15_CACHE_PREFETCH volatile *LPARM_CP15_CACHE_PREFETCH;

typedef struct _ARM_CP15_CACHE_LOCKDOWN
{
  BIT_FLD CacheIndex:6,
        Reserved:25,
        LoadBit:1;
} ARM_CP15_CACHE_LOCKDOWN;
typedef ARM_CP15_CACHE_LOCKDOWN volatile *LPARM_CP15_CACHE_LOCKDOWN;

typedef struct _ARM_DBG_COMM_CTRL
{
    BIT_FLD ReadRdy:1,
            WriteBsy:1,
            Reserved:26,
            Version:4;
} ARM_DBG_COMM_CTRL;
typedef ARM_DBG_COMM_CTRL volatile *LPARM_DBG_COMM_CTRL;
#endif /* BITFIELDS */

#endif /* INCL_ARM */

/************************/
/* Graphics Coprocessor */
/************************/

#ifdef INCL_GCP

/* Coprocessor Number 6 */

/* GCP Coprocessor Register Numbers */
#define GCP_DST_ADDR_REG    0
#define GCP_SRC_ADDR_REG    1
#define GCP_PAT_ADDR_REG    2
#define GCP_MSK_ADDR_REG    3
#define GCP_FG_COL_REG      4
#define GCP_BG_COL_REG      5
#define GCP_DST_CMP_COL_REG 6
#define GCP_SRC_CMP_COL_REG 7
#define GCP_CONTROL_REG     8
#define GCP_COUNT_REG       9
#define GCP_WORD_COUNT_REG  10
#define GCP_PAT_SIZE_REG    11
#define GCP_PAT_LINE_REG    12
#define GCP_TEXT_HEIGHT_REG 13
#define GCP_TEXT_PITCH_REG  14
#define GCP_TEXT_LINE_REG   15

/* Control Register Values */
#define GCP_1BPP            0x000
#define GCP_2BPP            0x001
#define GCP_4BPP            0x002
#define GCP_8BPP            0x003
#define GCP_16BPP           0x004
#define GCP_32BPP           0x005
#define GCP_24BPP           0x007

#define GCP_EXPAND_SRC      0x010
#define GCP_EXPAND_PAT      0x020
#define GCP_DST_CC_EN       0x040
#define GCP_SRC_CC_EN       0x080

#define GCP_OP_DST          0x100
#define GCP_OP_SRC          0x200
#define GCP_OP_PAT          0x400
#define GCP_OP_MSK          0x800
#define GCP_FILL            0x000
#define GCP_ROP1            0x100
#define GCP_COPY            0x200
#define GCP_ROP2            0x300
#define GCP_PAT_FILL        0x400
#define GCP_PAT_ROP         0x500
#define GCP_PAT_SRC         0x600
#define GCP_ROP3            0x700
#define GCP_MSK_COPY        0xB00
#define GCP_ROP4            0xF00

#define GCP_TEXT_32         0x1000
#define GCP_TEXT_64         0x2000
#define GCP_TEXT_128        0x3000

/* Some useful ROP codes */
#define GCP_SRCCOPY         0xCCCC0000
#define GCP_SRCPAINT        0xEEEE0000
#define GCP_SRCAND          0x88880000
#define GCP_SRCINVERT       0x66660000
#define GCP_SRCERASE        0x44440000
#define GCP_NOTSRCCOPY      0x33330000
#define GCP_NOTSRCERASE     0x11110000
#define GCP_MERGECOPY       0xC0C00000
#define GCP_MERGEPAINT      0xBBBB0000
#define GCP_PATCOPY         0xF0F00000
#define GCP_PATPAINT        0xFBFB0000
#define GCP_PATINVERT       0x5A5A0000
#define GCP_DSTINVERT       0x55550000
#define GCP_BLACKNESS       0x00000000
#define GCP_WHITENESS       0xFFFF0000
#define GCP_TRANSPARENT     0xCCAA0000

#ifdef BITFIELDS
typedef struct _GCP_CONTROL
{
  BIT_FLD  BPP:3,
           ExpandDst:1,
           ExpandSrc:1,
           ExpandPat:1,
           DstCCEnable:1,
           SrcCCEnable:1,
           OpDst:1,
           OpSrc:1,
           OpPat:1,
           OpMsk:1,
           TextMode:4,
           ROPCode:16;
} GCP_CONTROL;
#endif /* BITFIELDS */

#endif /* INCL_GCP */

/********************************/
/* Graphics Acceleration Module */
/********************************/
#ifdef INCL_GXA

/*
******************************************************************************
 *
 *     GXA_PARM_BASE                                         (GXA_BASE + 0x00)
 *     GXA_PARM0_REG                                         (GXA_BASE + 0x00)
 *     GXA_PARM1_REG                                         (GXA_BASE + 0x04)
 *     GXA_PARM2_REG                                         (GXA_BASE + 0x08)
 *     GXA_PARM3_REG                                         (GXA_BASE + 0x0C)
 *     GXA_CMD_REG                                           (GXA_BASE + 0x1C)
 *     GXA_FG_COLOR_REG                                      (GXA_BASE + 0x20)
 *     GXA_BG_COLOR_REG                                      (GXA_BASE + 0x24)
 *     GXA_LINE_PATT_REG                                     (GXA_BASE + 0x28)
 *     GXA_DST_XY_INC_REG                                    (GXA_BASE + 0x2C)
 *     GXA_CFG_REG                                           (GXA_BASE + 0x30)  
 *     GXA_BLT_CONTROL_REG                                   (GXA_BASE + 0x34)
 *     GXA_LINE_CONTROL_REG                                  (GXA_BASE + 0x38)
 *     GXA_BMAP_BASE                                         (GXA_BASE + 0x40)
 *     GXA_BMAP0_TYPE_REG                                    (GXA_BASE + 0x40)
 *     GXA_BMAP0_ADDR_REG                                    (GXA_BASE + 0x44)
 *     GXA_BMAP1_TYPE_REG                                    (GXA_BASE + 0x48)
 *     GXA_BMAP1_ADDR_REG                                    (GXA_BASE + 0x4C)
 *     GXA_BMAP2_TYPE_REG                                    (GXA_BASE + 0x50)
 *     GXA_BMAP2_ADDR_REG                                    (GXA_BASE + 0x54)
 *     GXA_BMAP3_TYPE_REG                                    (GXA_BASE + 0x58)
 *     GXA_BMAP3_ADDR_REG                                    (GXA_BASE + 0x5C)
 *     GXA_BMAP4_TYPE_REG                                    (GXA_BASE + 0x60)
 *     GXA_BMAP4_ADDR_REG                                    (GXA_BASE + 0x64)
 *     GXA_BMAP5_TYPE_REG                                    (GXA_BASE + 0x68)
 *     GXA_BMAP5_ADDR_REG                                    (GXA_BASE + 0x6C)
 *     GXA_BMAP6_TYPE_REG                                    (GXA_BASE + 0x70)
 *     GXA_BMAP6_ADDR_REG                                    (GXA_BASE + 0x74)
 *     GXA_BMAP7_TYPE_REG                                    (GXA_BASE + 0x78)
 *     GXA_BMAP7_ADDR_REG                                    (GXA_BASE + 0x7C)
 *     GXA_BRES0_ADDR_REG                                    (GXA_BASE + 0x80)
 *     GXA_BRES0_ERR_REG                                     (GXA_BASE + 0x84)
 *     GXA_BRES0_K1_REG                                      (GXA_BASE + 0x88)
 *     GXA_BRES0_K2_REG                                      (GXA_BASE + 0x8C)
 *     GXA_BRES0_INC1_REG                                    (GXA_BASE + 0x90)
 *     GXA_BRES0_INC2_REG                                    (GXA_BASE + 0x94)
 *     GXA_BRES0_LENGTH_REG                                  (GXA_BASE + 0x98)
 *     GXA_DEPTH_REG                                         (GXA_BASE + 0xF4)
 *     GXA_CFG2_REG                                          (GXA_BASE + 0xFC)
 *
 *****************************************************************************
 */


/* gui registers and commands */
#define GXA_PARM_BASE                                         (GXA_BASE + 0x00)
#define GXA_PARM0_REG                                         (GXA_BASE + 0x00)
#define GXA_PARM1_REG                                         (GXA_BASE + 0x04)
#define GXA_PARM2_REG                                         (GXA_BASE + 0x08)
#define GXA_PARM3_REG                                         (GXA_BASE + 0x0C)
#define GXA_CMD_REG                                           (GXA_BASE + 0x1C)
#define     GXA_CMD_PARM_COUNT_MASK                               0x0000001C
#define     GXA_CMD_PARM_COUNT_SHIFT                                     2
#define     GXA_CMD_DST_BMAP_MASK                                 0x000000E0
#define     GXA_CMD_DST_BMAP_SHIFT                                       5
#define     GXA_CMD_SRC_BMAP_MASK                                 0x00000700
#define     GXA_CMD_SRC_BMAP_SHIFT                                       8
#define     GXA_CMD_CMD_NUM_MASK                                  0x0001F800
#define     GXA_CMD_CMD_NUM_SHIFT                                       11
#define     GXA_CMD_STATE_MASK                                    0x00700000
#define     GXA_CMD_STATE_SHIFT                                         20
#define     GXA_CMD_WAIT4_VERT_MASK                               0x00800000
#define     GXA_CMD_WAIT4_VERT_SHIFT                                    23
#define     GXA_CMD_QMARK_VAL_MASK                                0xFF000000
#define     GXA_CMD_QMARK_VAL_SHIFT                                     24
#define GXA_FG_COLOR_REG                                      (GXA_BASE + 0x20)
#define GXA_BG_COLOR_REG                                      (GXA_BASE + 0x24)
#define GXA_LINE_PATT_REG                                     (GXA_BASE + 0x28)
#define GXA_DST_XY_INC_REG                                    (GXA_BASE + 0x2C)
#define     GXA_DST_X_INC_VAL_MASK                                0x00000FFF
#define     GXA_DST_X_INC_VAL_SHIFT                                      0
#define     GXA_DST_X_SIGN_EXT_MASK                               0x0000F000
#define     GXA_DST_X_SIGN_EXT_SHIFT                                    12
#define     GXA_DST_Y_INC_VAL_MASK                                0x0FFF0000
#define     GXA_DST_Y_INC_VAL_SHIFT                                     16
#define     GXA_DST_Y_SIGN_EXT_MASK                               0xF0000000
#define     GXA_DST_Y_SIGN_EXT_SHIFT                                    28
#define GXA_CFG_REG                                           (GXA_BASE + 0x30)  
#define     GXA_CFG_ROP_MASK                                      0x0000001F
#define     GXA_CFG_ROP_SHIFT                                            0
#define     GXA_CFG_MONO_FLIP_EN_MASK                             0x00000100
#define     GXA_CFG_MONO_FLIP_EN_SHIFT                                   8
#define     GXA_CFG_SRC_TRANSPARENCY_MASK                         0x00003000
#define     GXA_CFG_SRC_TRANSPARENCY_SHIFT                              12
#define     GXA_CFG_ALPHA_TRANSPARENCY_MASK                       0x00004000
#define     GXA_CFG_ALPHA_TRANSPARENCY_SHIFT                            14
#define     GXA_CFG_PRESERVE_ALPHA_MASK                           0x00080000
#define     GXA_CFG_PRESERVE_ALPHA_SHIFT                                19
#define     GXA_CFG_ALPHA_BLEND_SEL_MASK                          0x00300000
#define     GXA_CFG_ALPHA_BLEND_SEL_SHIFT                               20
#define     GXA_CFG_ALPHA_INSIDE_OUT_MASK                         0x01000000
#define     GXA_CFG_ALPHA_INSIDE_OUT_SHIFT                              24
#define     GXA_CFG_CMP_LINE_CNT_EN_MASK                          0x02000000
#define     GXA_CFG_CMP_LINE_CNT_EN_SHIFT                               25
#define     GXA_CFG_FORCE_PAT_RELOAD_MASK                         0x08000000
#define     GXA_CFG_FORCE_PAT_RELOAD_SHIFT                              27
#define GXA_BLT_CONTROL_REG                                   (GXA_BASE + 0x34)
#define     GXA_BLT_DIR_MASK                                      0x00000001
#define     GXA_BLT_DIR_SHIFT                                            0
#define     GXA_BLT_SCAN_LINE_CNT_MASK                            0x03FF0000
#define     GXA_BLT_SCAN_LINE_CNT_SHIFT                                 16
#define GXA_LINE_CONTROL_REG                                  (GXA_BASE + 0x38)
#define     GXA_LINE_NT_REVERSIBLE_MASK                           0x00000001
#define     GXA_LINE_NT_REVERSIBLE_SHIFT                                 0
#define     GXA_LINE_INVERT_ON_ZERO_MASK                          0x00000002
#define     GXA_LINE_INVERT_ON_ZERO_SHIFT                                1
#define     GXA_LINE_SKIP_LAST_MASK                               0x00000004
#define     GXA_LINE_SKIP_LAST_SHIFT                                     2
#define     GXA_LINE_SKIP_FIRST_MASK                              0x00000008
#define     GXA_LINE_SKIP_FIRST_SHIFT                                    3
#define     GXA_LINE_CALC_ONLY_MASK                               0x00000010
#define     GXA_LINE_CALC_ONLY_SHIFT                                     4
#define     GXA_LINE_X_REVERSIBLE_MASK                            0x00000020
#define     GXA_LINE_X_REVERSIBLE_SHIFT                                  5
#define     GXA_LINE_SIGN_DY_MASK                                 0x00000100
#define     GXA_LINE_SIGN_DY_SHIFT                                       8
#define     GXA_LINE_SIGN_DX_MASK                                 0x00000200
#define     GXA_LINE_SIGN_DX_SHIFT                                       9
#define     GXA_LINE_XMAJOR_MASK                                  0x00000400
#define     GXA_LINE_XMAJOR_SHIFT                                       10
#define GXA_BMAP_BASE                                         (GXA_BASE + 0x40)
#define GXA_BMAP0_TYPE_REG                                    (GXA_BASE + 0x40)
#define GXA_BMAP0_ADDR_REG                                    (GXA_BASE + 0x44)
#define GXA_BMAP1_TYPE_REG                                    (GXA_BASE + 0x48)
#define GXA_BMAP1_ADDR_REG                                    (GXA_BASE + 0x4C)
#define GXA_BMAP2_TYPE_REG                                    (GXA_BASE + 0x50)
#define GXA_BMAP2_ADDR_REG                                    (GXA_BASE + 0x54)
#define GXA_BMAP3_TYPE_REG                                    (GXA_BASE + 0x58)
#define GXA_BMAP3_ADDR_REG                                    (GXA_BASE + 0x5C)
#define GXA_BMAP4_TYPE_REG                                    (GXA_BASE + 0x60)
#define GXA_BMAP4_ADDR_REG                                    (GXA_BASE + 0x64)
#define GXA_BMAP5_TYPE_REG                                    (GXA_BASE + 0x68)
#define GXA_BMAP5_ADDR_REG                                    (GXA_BASE + 0x6C)
#define GXA_BMAP6_TYPE_REG                                    (GXA_BASE + 0x70)
#define GXA_BMAP6_ADDR_REG                                    (GXA_BASE + 0x74)
#define GXA_BMAP7_TYPE_REG                                    (GXA_BASE + 0x78)
#define     GXA_BMAP_TYPE_PITCH_MASK                              0x00003FFF
#define     GXA_BMAP_TYPE_PITCH_SHIFT                                    0
#define     GXA_BMAP_TYPE_PIXEL_FMT_MASK                          0x001F0000
#define     GXA_BMAP_TYPE_PIXEL_FMT_SHIFT                               16
#define     GXA_BMAP_TYPE_MONO_MASK                               0x01000000
#define     GXA_BMAP_TYPE_MONO_SHIFT                                    24
#define     GXA_BMAP_TYPE_PATTERN_MASK                            0x04000000
#define     GXA_BMAP_TYPE_PATTERN_SHIFT                                 26
#define     GXA_BMAP_TYPE_SOLID_FILL_MASK                         0x08000000
#define     GXA_BMAP_TYPE_SOLID_FILL_SHIFT                              27
#define     GXA_BMAP_TYPE_PATTERN_SZ_MASK                         0x30000000
#define     GXA_BMAP_TYPE_PATTERN_SZ_SHIFT                              28
#define GXA_BMAP7_ADDR_REG                                    (GXA_BASE + 0x7C)
#define     GXA_BMAP_ADDR_MASK                                    0x3FFFFFFF
#define     GXA_BMAP_ADDR_SHIFT                                          0
#define GXA_BRES0_ADDR_REG                                    (GXA_BASE + 0x80)
#define     GXA_BRES0_ADDR_MASK                                   0x7FFFFFFF
#define     GXA_BRES0_ADDR_SHIFT                                         0
#define GXA_BRES0_ERR_REG                                     (GXA_BASE + 0x84)
#define GXA_BRES0_K1_REG                                      (GXA_BASE + 0x88)
#define GXA_BRES0_K2_REG                                      (GXA_BASE + 0x8C)
#define GXA_BRES0_INC1_REG                                    (GXA_BASE + 0x90)
#define GXA_BRES0_INC2_REG                                    (GXA_BASE + 0x94)
#define     GXA_BRES0_INC_INCREMENT_MASK                          0x0003FFF8
#define     GXA_BRES0_INC_INCREMENT_SHIFT                                3
#define     GXA_BRES0_INC_SIGN_EXTEND_MASK                        0xFFFC0000
#define     GXA_BRES0_INC_SIGN_EXTEND_SHIFT                             18
#define GXA_BRES0_LENGTH_REG                                  (GXA_BASE + 0x98)
#define     GXA_BRES0_LENGTH_MASK                                 0x00000FFF
#define     GXA_BRES0_LENGTH_SHIFT                                       0
#define GXA_DEPTH_REG                                         (GXA_BASE + 0xF4)
#define     GXA_DEPTH_CMD_Q_NON_EMPTY_MASK                        0x00010000
#define     GXA_DEPTH_CMD_Q_NON_EMPTY_SHIFT                             16
#define     GXA_DEPTH_GFE_PIPE_BUSY_MASK                          0x00020000
#define     GXA_DEPTH_GFE_PIPE_BUSY_SHIFT                               17
#define     GXA_DEPTH_RSLT_Q_NON_EMPTY_MASK                       0x00040000
#define     GXA_DEPTH_RSLT_Q_NON_EMPTY_SHIFT                            18
#define     GXA_DEPTH_GBE_PIPE_BUSY_MASK                          0x00020000
#define     GXA_DEPTH_GBE_PIPE_BUSY_SHIFT                               17
#define     GXA_DEPTH_CMD_Q_WAIT4_VERT_MASK                       0x01000000
#define     GXA_DEPTH_CMD_Q_WAIT4_VERT_SHIFT                            24
#define     GXA_DEPTH_CMD_Q_DEPTH_MASK                            0x7E000000
#define     GXA_DEPTH_CMD_Q_DEPTH_SHIFT                                 25
#ifndef PCI_GXA
#define GXA_CFG2_REG                                          (GXA_BASE + 0xFC)
#define     GXA_CFG2_FIXED_ALPHA_MASK                             0x000000ff
#define     GXA_CFG2_FIXED_ALPHA_SHIFT                                   0
#define     GXA_CFG2_START_ON_RFF_BUSY_MASK                       0x00000100
#define     GXA_CFG2_START_ON_RFF_BUSY_SHIFT                             8
#define     GXA_CFG2_IRQ_EN_HSXERR_MASK                           0x00010000
#define     GXA_CFG2_IRQ_EN_HSXERR_SHIFT                                16
#define     GXA_CFG2_IRQ_EN_QMARK_MASK                            0x00020000
#define     GXA_CFG2_IRQ_EN_QMARK_SHIFT                                 17
#define     GXA_CFG2_IRQ_STAT_HSXERR_MASK                         0x00100000
#define     GXA_CFG2_IRQ_STAT_HSXERR_SHIFT                              20
#define     GXA_CFG2_IRQ_STAT_QMARK_MASK                          0x00200000
#define     GXA_CFG2_IRQ_STAT_QMARK_SHIFT                               21
#endif


#ifdef PCI_GXA
#define GXA_GQ_ENTRIES     8           /* Dword entries in graphics queue */
#define GXA_NON_QUEUED     0x00400000l /* nonqueued register offset */
#else
#define GXA_GQ_ENTRIES     32          /* Dword entries in graphics queue */
#define GXA_NON_QUEUED     0x00000000l /* nonqueued register offset */
#endif

/***************************/
/*  32 BIT ACCESS VALUES   */
/***************************/

/* GXA Fifo Depth Register */
#define GUI_BUSY           0x00080000l
#define RESULT_FIFO_BUSY   0x00040000l
#define GUI_QUEUE_BUSY     0x00020000l
#define GUI_FIFO_NON_EMPTY 0x00010000l

#ifdef PCI_GXA
#define GUI_GQUE_DEPTH     0x1e000000l
#else
#define GUI_GQUE_DEPTH     0x7e000000l
#endif

/* Bitmap Context registers fields */
#define BM_TYPE_MASK   0xFF000000l
#define BM_PITCH_MASK  0x00003fffl

#ifdef PCI_GXA
#define BM_ADDR_MASK   0x000fffffl
#else
#define BM_FORMAT_MASK 0x001f0000l
#define BM_ADDR_MASK   0x1fffffffl
#endif

/* GXA Configuration Register */
/*#define ROP_FIELD           0x0000001Fl */
#define ROP_FIELD             0x0000000Fl
#define MONO_FLIP             0x00000100l
#define TRANSP_ENABLE         0x00001000l
#define ENABLE_SCAN_COUNT     0x02000000l
#define PRESERVE_ALPHA        0x00080000l
#ifndef PCI_GXA
#define FORCE_PAT_RELOAD      0x08000000l
#define INSIDE_OUT_ALPHA      0x01000000l
#define ALPHA_STORE_DST       0x00000000l
#define ALPHA_STORE_SRC       0x00400000l
#define ALPHA_STORE_FIXED     0x00800000l
#define USE_DST_ALPHA         0x00000000l
#define USE_SRC_ALPHA         0x00100000l
#define USE_FIXED_ALPHA       0x00200000l
#define USE_ALPHA_IN_TRANSP   0x00004000l
#endif


/* GXA Config2 32 bit access */
#ifndef PCI_GXA
#define GXA_STAT_QMARK        0x00200000l
#define GXA_STAT_HSXERR       0x00100000l
#define GXA_EN_QMARK_IRQ      0x00020000l
#define GXA_EN_HSXERR_IRQ     0x00010000l
#define GXA_EN_RFF_BUSY       0x00000100l
#endif

/* GXA Blit Control Register */
#define BLT_BACKWARDS        1

/* GXA Line Control Register */
#define NT_REVERSIBLE        1
#define SKIP_LAST            4
#define CALC_ONLY            0x10
#define DY_NEGATIVE          0x100
#define DX_NEGATIVE          0x200
#define XMAJOR               0x400

/* GXA Blit Control Register */
#define BLT_BACKWARDS        1


#ifdef BITFIELDS
/***************************/
/* GXA BITFIELD STRUCTURES */
/***************************/

/* GXA Depth Register */
typedef struct _GXA_DEPTH {
        BIT_FLD Rsvd1:16,
                GQNonEmpty:1,
                GQBusy:1,
                ResultBusy:1,
                GuiBusy:1,
                Rsvd2:4,
                GQWaitVert:1,
                GQDepth:6,
                Rsvd3:1;
} GXA_DEPTH;
typedef GXA_DEPTH volatile *LPGXA_DEPTH;

/* Bitmap Context Type and Pitch Register */
typedef struct _GXA_BM_TYPE {
        BIT_FLD Pitch:14,
                Rsvd1:2,
                Format:5,
                Rsvd2:3,
                Type:8;
} GXA_BM_TYPE;
typedef GXA_BM_TYPE volatile *LPGXA_BM_TYPE;

/* Bitmap Context Offset Address Register */
typedef struct _GXA_BM_ADDR {
        BIT_FLD Addr:29,
                Rsvd1:3;
} GXA_BM_ADDR;
typedef GXA_BM_ADDR volatile *LPGXA_BM_ADDR;

/* GXA Command Register */
typedef struct _GXA_CMD {
        BIT_FLD Rsvd1:5,
                ParmCnt:3,
                DstBMSel:3,
                SrcBMDSel:3,
                Rsvd2:2,
                CmdNum:6,
                Rsvd3:1,
                WaitVertStatus:1,
                LastQMark:8;
} GXA_CMD;
typedef GXA_CMD volatile *LPGXA_CMD;

/* GXA Configuration Register */
typedef struct _GXA_CFG {
        BIT_FLD ROP:5,
                Rsvd1:3,
                MonoFlipEn:1,
                Rsvd2:3,
                SrcTransparency:2,
                AlphaTransparency:1,
                Rsvd3:4,
                PreserveAlpha:1,
                AlphaBlendSel:2,
                AlphaStoreSel:2,
                AlphaInsideOut:1,
                CmpLineCntEn:1,
                Rsvd4:1,
                ForcePatReload:1,
                Rsvd5:4;
} GXA_CFG;
typedef GXA_CFG volatile *LPGXA_CFG;

/* GXA Configuration 2 Register */
typedef struct _GXA_CFG2 {
        BIT_FLD FixedAlpha:8,
                StartOnRFFBusy:1,
                Rsvd1:7,
                HsxErrIrqEn:1,
                QMarkIrqEn:1,
                Rsvd2:2,
                HsxErrIrqStat:1,
                QMarkIrqStat:1,
                Rsvd3:10;
} GXA_CFG2;
typedef GXA_CFG2 volatile *LPGXA_CFG2;

/* GXA Foreground Color Register */
typedef struct _GXA_FG_COLOR
{
  HW_DWORD Value;
} GXA_FG_COLOR;
typedef GXA_FG_COLOR volatile *LPGXA_FG_COLOR;

/* GXA Background Color Register */
typedef struct _GXA_BG_COLOR
{
  HW_DWORD Value;
} GXA_BG_COLOR;
typedef GXA_BG_COLOR volatile *LPGXA_BG_COLOR;

/* GXA Destination XY Increment Register */
typedef struct _GXA_DEST_XY_INC
{
        BIT_FLD SignedXInc:12,
                SignExtX:4,
                SignedYInc:12,
                SignExtY:4;
} GXA_DEST_XY_INC;
typedef GXA_DEST_XY_INC volatile *LPGXA_DEST_XY_INC;

typedef struct _GXA_BLT_CONTROL
{
        BIT_FLD Dir:1,
                Rsvd1:15,
                ScanlineCnt:12,
                Rsvd2:4;
} GXA_BLT_CONTROL;
typedef GXA_BLT_CONTROL volatile *LPGXA_BLT_CONTROL;

/* GXA Line Control Register */
typedef struct _GXA_LINE_CONTROL
{
        BIT_FLD NT_Reversible:1,
                InvertOnZero:1,
                SkipLast:1,
                SkipFirst:1,
                CalcOnly:1,
                X_Reversible:1,
                Rsvd1:2,
                SignDY:1,
                SignDX:1,
                XMajor:1,
                Rsvd2:21;
} GXA_LINE_CONTROL;
typedef GXA_LINE_CONTROL volatile *LPGXA_LINE_CONTROL;

/* GXA Line Pattern Register */
typedef struct _GXA_LINE_PATT
{
  HW_DWORD Value;
} GXA_LINE_PATT;
typedef GXA_LINE_PATT volatile *LPGXA_LINE_PATT;

/* GXA Bresenham 0 Address Register */
typedef struct _GXA_BRES0_ADDR
{
        BIT_FLD Rsvd1:3,
                ByteAddr:2,
                DwordAddr:20,
                Rsvd2:7;
} GXA_BRES0_ADDR;
typedef GXA_BRES0_ADDR volatile *LPGXA_BRES0_ADDR;

/* GXA Bresenham 0 Error Register */
typedef struct _GXA_BRES0_ERR
{
  HW_DWORD Value;
} GXA_BRES0_ERR;
typedef GXA_BRES0_ERR volatile *LPGXA_BRES0_ERR;

/* GXA Bresenham 0 K1 Register */
typedef struct _GXA_BRES0_K1
{
  HW_DWORD Value;
} GXA_BRES0_K1;
typedef GXA_BRES0_K1 volatile *LPGXA_BRES0_K1;

/* GXA Bresenham 0 K2 Register */
typedef struct _GXA_BRES0_K2
{
  HW_DWORD Value;
} GXA_BRES0_K2;
typedef GXA_BRES0_K2 volatile *LPGXA_BRES0_K2;

/* GXA Bresenham 0 Increment 1 Register */
typedef struct _GXA_BRES0_INC1
{
        BIT_FLD Rsvd1:3,
                Increment:15,
                SignExtend:14;
} GXA_BRES0_INC1;
typedef GXA_BRES0_INC1 volatile *LPGXA_BRES0_INC1;

/* GXA Bresenham 0 Increment 2 Register */
typedef struct _GXA_BRES0_INC2
{
        BIT_FLD Rsvd1:3,
                Increment:15,
                SignExtend:14;
} GXA_BRES0_INC2;
typedef GXA_BRES0_INC2 volatile *LPGXA_BRES0_INC2;

/* GXA Bresenham 0 Length Register */
typedef struct _GXA_BRES0_LENGTH
{
        BIT_FLD Length:12,
                Rsvd1:20;
} GXA_BRES0_LENGTH;
typedef GXA_BRES0_LENGTH volatile *LPGXA_BRES0_LENGTH;
#endif /* BITFIELDS */

#endif /* INCL_GXA */

/**************************/
/* Display Refresh Module */
/**************************/
#ifdef INCL_DRM
/* Last updated 5/15/2001 */

#define XSTART(x,off)                           ((x<<1)+off)
#define YPOS(y,off)                             (y+off)

/******************************/
/* OSD Header type defintions */
/******************************/

/* Pixel type defintions used in X Start & Width Field */
#define BPP4ARGBPAL                 0x00
#define BPP8ARGBPAL                 0x01
#define BPP16ARGBPAL                0x02
#define BPP4AYUVPAL                 0x04
#define BPP8AYUVPAL                 0x05
#define BPP16AYUV                   0x06
#define BPP16YUV422                 0x1E
#define BPP16YUV655                 0x0E
#define BPP16RGB565PAL              0x0A
   /* New pixel types */
#define BPP16AYUV2644               0x16
#define BPP16ARGB1555PAL            0x12
#define BPP32AYUV8888               0x07
#define BPP32ARGB8888PAL            0x03

        /* XStartWidth flags and macros */
#define XSW_XSTART_MASK                         0x00000FFF
#define XSW_WIDTH_MASK                          0x003FF000
#define XSW_PTYPE_MASK                          0x0F800000
#define XSW_PINDEX_MASK                         0x30000000
#define XSW_PLOAD_FLAG                          0x40000000
#define XSW_BSWAP_FLAG                          0x80000000
#define XSW_BPP_MASK                            0x01800000

#define XSW_PIXEL_ALPHA_SELECT                  0x04000000

#define XSW_WIDTH_SHIFT                         12
#define XSW_PTYPE_SHIFT                         23
#define XSW_PINDEX_SHIFT                        28

#define MAKE_XSTARTWIDTH(x,w,ptype,palindex,loadpal)    (0 |                 \
                        (x & XSW_XSTART_MASK) |                              \
                        ((w << XSW_WIDTH_SHIFT) & XSW_WIDTH_MASK) |          \
                        ((ptype << XSW_PTYPE_SHIFT) & XSW_PTYPE_MASK) |      \
                        ((palindex << XSW_PINDEX_SHIFT) & XSW_PINDEX_MASK) | \
                        ((loadpal) ? XSW_PLOAD_FLAG : 0))

        /* Field address flags and macros */
#define FA_ADDRESS_MASK                         0x00FFFFFF

        /* Field 1 address flags */
#define FA_FLICKER_FILTER_ENABLE                0x01000000
#define FA_ALPHA_BLEND_ENABLE                   0x02000000
#define FA_COLOR_KEY_ENABLE                     0x04000000
#define FA_ASPECT_RATIO_CONV_ENABLE             0x08000000
#define FA_ALPHA_TOP_VIDEO                      0x10000000
#define FA_FORCE_REGION_ALPHA                   0x20000000
#define FA_ALPHA_BOTH_VIDEO                     0x40000000
#define FA_BLEND_PLANE1                         0x80000000


#define MAKE_FIELDADDRESS(addr,flags) ((addr & FA_ADDRESS_MASK) | flags)

        /* Field stride macros */
#define MAKE_STRIDE(w, bpp)     (((w * bpp / 8) + 3) & 0x0000FFFC)
#define MAKE_FIELDSTRIDE(w, bpp) ((MAKE_STRIDE(w, bpp) << 17) | (MAKE_STRIDE(w, bpp) << 1))

        /* Field position macros */
#define FP_START_MASK                           0x000003FF
#define FP_END_MASK                             0x003FF000
#define FP_ALPHA_MASK                           0xFF000000

#define FP_END_SHIFT                            12
#define FP_ALPHA_SHIFT                          24

#define MAKE_FIELDPOS(top, bot, alpha) ((top & FP_START_MASK) |              \
                                  ((bot << FP_END_SHIFT) & FP_END_MASK) |    \
                                  ((alpha << FP_ALPHA_SHIFT) & FP_ALPHA_MASK))


typedef struct _OSDHeader * POSDHEADER;
typedef struct _OSDHeader {
        POSDHEADER      pNextOsd;
        HW_DWORD         dwXStartWidth;
        HW_DWORD         dwAddrField1;
        HW_DWORD         dwAddrField2;
        HW_DWORD         dwFieldStride;
        HW_DWORD         dwField1Pos;
        HW_DWORD         dwField2Pos;
        HW_DWORD         dwPaletteAddr;
} OSDHEADER;

/*
typedef struct _OSDBitHeader * POSDBITHEADER;
typedef struct _OSDBitHeader {
        POSDBITHEADER   pNextOsd;
        BIT_FLD         XStart:12,
                        Width:10,
                        reserved1:1,
                        PixelType:2,
                        PixelColor:1,
                        PixelAlphaSelect:1,
                        Format422:1,
                        PaletteIndex:2,
                        PaletteLoad:1,
                        ByteSwapped:1;
        BIT_FLD         Field1Addr:24,
                        FlickerFilterEnable:1,
                        AlphaBlendEnable:1,
                        ColorKeyEnable:1,
                        AspectRationConvEnable:1,
                        AlphaBlendTop:1,
                        ForceRegionAlpha:1,
                        AlphaBlendBoth:1,
                        reserved2:1;
        BIT_FLD         Field2Addr:24,
                        reserved3:8;
        BIT_FLD         Field1Stride:16,
                        Filed2Stride:16;
        BIT_FLD         Field1YStart:10,
                        reserved4:2,
                        Field1YEnd:10,
                        reserved5:2,
                        Alpha:6,
                        reserved6:2;
        BIT_FLD         Field2YStart:10,
                        reserved7:2,
                        Field2YEnd:10,
                        reserved8:10;
        HW_DWORD         dwPaletteAddr;
} OSDBITHEADER;
*/

/*
 *******************************************************************************
 *
 *      DRM_CONTROL_REG                                       (DRM_BASE + 0x000)
 *      DRM_INTERRUPT_ENABLE_REG                              (DRM_BASE + 0x004)
 *      DRM_STATUS_REG                                        (DRM_BASE + 0x008)
 *      DRM_OSD_POINTER_REG                                   (DRM_BASE + 0x00C)
 *      DRM_COLOR_KEY_REG                                     (DRM_BASE + 0x010)
 *      DRM_GRAPHICS_BORDER_COLOR_REG                         (DRM_BASE + 0x014)
 *      DRM_DWI_CONTROL_1_REG                                 (DRM_BASE + 0x018)
 *      DRM_DWI_CONTROL_2_REG                                 (DRM_BASE + 0x01C)
 *      DRM_SCREEN_START_REG                                  (DRM_BASE + 0x020)
 *      DRM_SCREEN_END_REG                                    (DRM_BASE + 0x024)
 *      Reserved                                              (DRM_BASE + 0x028)
 *      DRM_CONTROL_2_REG                                     (DRM_BASE + 0x02C)
 *      DRM_MPEG_OFFSET_WIDTH_REG                             (DRM_BASE + 0x030)
 *      DRM_MPEG_VERTICAL_OFFSET_REG                          (DRM_BASE + 0x034)
 *      DRM_MPEG_POSITION_REG                                 (DRM_BASE + 0x038)
 *      DRM_MPEG_SIZE_REG                                     (DRM_BASE + 0x03C)
 *      DRM_MPEG_X_INC_REG                                    (DRM_BASE + 0x040)
 *      DRM_MPEG_Y_INC_REG                                    (DRM_BASE + 0x044)
 *      DRM_SHARP_REG                                         (DRM_BASE + 0x048)
 *      DRM_DWI_CONTROL_3_REG                                 (DRM_BASE + 0x04C)
 *      DRM_VBI_BUF_0_REG                                     (DRM_BASE + 0x050)
 *      DRM_VBI_BUF_1_REG                                     (DRM_BASE + 0x054)
 *      DRM_VID_BUF_0_REG                                     (DRM_BASE + 0x058)
 *      DRM_VID_BUF_1_REG                                     (DRM_BASE + 0x05C)
 *      DRM_VID_BUF_2_REG                                     (DRM_BASE + 0x060)
 *      DRM_VID_POSITION_REG                                  (DRM_BASE + 0x064)
 *      DRM_VID_SIZE_REG                                      (DRM_BASE + 0x068)
 *      DRM_VID_STRIDE_REG                                    (DRM_BASE + 0x06C)
 *      DRM_TELETEXT_STRIDE_REG                               (DRM_BASE + 0x070)
 *      DRM_TELETEXT_FIELD1_REG                               (DRM_BASE + 0x074)
 *      DRM_TELETEXT_FIELD2_REG                               (DRM_BASE + 0x078)
 *      DRM_TELETEXT_FIELD1_ADDR_REG                          (DRM_BASE + 0x07C)
 *      DRM_TELETEXT_FIELD2_ADDR_REG                          (DRM_BASE + 0x080)
 *      DRM_CURSOR_CONTROL_REG                                (DRM_BASE + 0x084)
 *      DRM_CURSOR_STORE_ADDR_REG                             (DRM_BASE + 0x088)
 *      DRM_CURSOR_PALETTE_0_REG                              (DRM_BASE + 0x08C)
 *      DRM_CURSOR_PALETTE_1_REG                              (DRM_BASE + 0x090)
 *      DRM_CURSOR_PALETTE_2_REG                              (DRM_BASE + 0x094)
 *      DRM_CURSOR_FETCH_SIZE_REG                             (DRM_BASE + 0x098) 
 *      DRM_SCALING_COEFF_REG                                 (DRM_BASE + 0x09c)
 *      DRM_MPEG_STILL_CONTROL_REG                            (DRM_BASE + 0x0A0)
 *      DRM_MPEG_LUMA_ADDR_REG                                (DRM_BASE + 0x0A4)
 *      DRM_MPEG_CHROMA_ADDR_REG                              (DRM_BASE + 0x0A8)
 *      DRM_MPEG_HEIGHTWIDTH_REG                              (DRM_BASE + 0x0AC)
 *      DRM_MPEG_WIPE_LUMA_REG                                (DRM_BASE + 0x0B0)
 *      DRM_MPEG_WIPE_CHROMA_REG                              (DRM_BASE + 0x0B4)
 *      DRM_MPEG_TILE_PARMS_REG                               (DRM_BASE + 0x0B8)
 * Second set of MLV registers
 *      DRM_MPEG_OFFSET_WIDTH_2_REG                           (DRM_BASE + 0x0D0)
 *      DRM_MPEG_VERTICAL_OFFSET_2_REG                        (DRM_BASE + 0x0D4)
 *      DRM_MPEG_POSITION_2_REG                               (DRM_BASE + 0x0D8)
 *      DRM_MPEG_SIZE_2_REG                                   (DRM_BASE + 0x0DC)
 *      DRM_MPEG_X_INC_2_REG                                  (DRM_BASE + 0x0E0)
 *      DRM_MPEG_Y_INC_2_REG                                  (DRM_BASE + 0x0E4)
 *      DRM_SHARP_2_REG                                       (DRM_BASE + 0x0E8)
 *      Reserved                                              (DRM_BASE + 0x0EC)
 *      DRM_VBI_BUF_0_2_REG                                   (DRM_BASE + 0x0F0)
 *      DRM_VBI_BUF_1_2_REG                                   (DRM_BASE + 0x0F4)
 *      DRM_VID_BUF_0_2_REG                                   (DRM_BASE + 0x0F8)
 *      DRM_VID_BUF_1_2_REG                                   (DRM_BASE + 0x0FC)
 *      DRM_VID_BUF_2_2_REG                                   (DRM_BASE + 0x100)
 *      DRM_VID_POSITION_2_REG                                (DRM_BASE + 0x104)
 *      DRM_VID_SIZE_2_REG                                    (DRM_BASE + 0x108)
 *      DRM_VID_STRIDE_2_REG                                  (DRM_BASE + 0x10C)
 *      DRM_MPEG_STILL_CONTROL_2_REG                          (DRM_BASE + 0x110)
 *      DRM_MPEG_LUMA_ADDR_2_REG                              (DRM_BASE + 0x114)
 *      DRM_MPEG_CHROMA_ADDR_2_REG                            (DRM_BASE + 0x118)
 *      DRM_MPEG_HEIGHTWIDTH_2_REG                            (DRM_BASE + 0x11C)
 *      DRM_MPEG_WIPE_LUMA_2_REG                              (DRM_BASE + 0x120)
 *      DRM_MPEG_WIPE_CHROMA_2_REG                            (DRM_BASE + 0x124)
 *      DRM_MPEG_TILE_PARMS_2_REG                             (DRM_BASE + 0x128)
 *      DRM_VIDEO_ALPHA_REG                                   (DRM_BASE + 0x12C)
 *      DRM_SAR_REG                                           (DRM_BASE + 0x160)
 *
 *******************************************************************************
 */

/* Second set of MLV registers */
#define DRM_MPEG_OFFSET_WIDTH_2_REG                  ((LPREG)(DRM_BASE + 0x0D0))
#define DRM_MPEG_VERTICAL_OFFSET_2_REG               ((LPREG)(DRM_BASE + 0x0D4))
#define DRM_MPEG_POSITION_2_REG                      ((LPREG)(DRM_BASE + 0x0D8))
#define DRM_MPEG_SIZE_2_REG                          ((LPREG)(DRM_BASE + 0x0DC))
#define DRM_MPEG_X_INC_2_REG                         ((LPREG)(DRM_BASE + 0x0E0))
#define DRM_MPEG_Y_INC_2_REG                         ((LPREG)(DRM_BASE + 0x0E4))
#define DRM_SHARP_2_REG                              ((LPREG)(DRM_BASE + 0x0E8))
#define DRM_VBI_BUF_0_2_REG                          ((LPREG)(DRM_BASE + 0x0F0))
#define DRM_VBI_BUF_1_2_REG                          ((LPREG)(DRM_BASE + 0x0F4))
#define DRM_VID_BUF_0_2_REG                          ((LPREG)(DRM_BASE + 0x0F8))
#define DRM_VID_BUF_1_2_REG                          ((LPREG)(DRM_BASE + 0x0FC))
#define DRM_VID_BUF_2_2_REG                          ((LPREG)(DRM_BASE + 0x100))
#define DRM_VID_POSITION_2_REG                       ((LPREG)(DRM_BASE + 0x104))
#define DRM_VID_SIZE_2_REG                           ((LPREG)(DRM_BASE + 0x108))
#define DRM_VID_STRIDE_2_REG                         ((LPREG)(DRM_BASE + 0x10C))
#define DRM_MPEG_STILL_CONTROL_2_REG                 ((LPREG)(DRM_BASE + 0x110))
#define DRM_MPEG_LUMA_ADDR_2_REG                     ((LPREG)(DRM_BASE + 0x114))
#define DRM_MPEG_CHROMA_ADDR_2_REG                   ((LPREG)(DRM_BASE + 0x118))
#define DRM_MPEG_HEIGHTWIDTH_2_REG                   ((LPREG)(DRM_BASE + 0x11C))
#define DRM_MPEG_WIPE_LUMA_2_REG                     ((LPREG)(DRM_BASE + 0x120))
#define DRM_MPEG_WIPE_CHROMA_2_REG                   ((LPREG)(DRM_BASE + 0x124))
#define DRM_MPEG_TILE_PARMS_2_REG                    ((LPREG)(DRM_BASE + 0x128))

/* Masks not defined yet */
#define DRM_SAR_REG                                  ((LPREG)(DRM_BASE + 0x160))


#define     DRMRGBPAL_R_MASK                                       0x000000FF
#define     DRMRGBPAL_R_SHIFT                                             0
#define     DRMRGBPAL_G_MASK                                       0x0000FF00
#define     DRMRGBPAL_G_SHIFT                                             8
#define     DRMRGBPAL_B_MASK                                       0x00FF0000
#define     DRMRGBPAL_B_SHIFT                                            16
#define     DRMRGBPAL_ALPHA_MASK                                   0xFF000000
#define     DRMRGBPAL_ALPHA_SHIFT                                        24

#define     DRMYCCPAL_Y_MASK                                       0x000000FF
#define     DRMYCCPAL_Y_SHIFT                                             0
#define     DRMYCCPAL_CB_MASK                                      0x0000FF00
#define     DRMYCCPAL_CB_SHIFT                                            8
#define     DRMYCCPAL_CR_MASK                                      0x00FF0000
#define     DRMYCCPAL_CR_SHIFT                                           16
#define     DRMYCCPAL_ALPHA_MASK                                   0xFF000000
#define     DRMYCCPAL_ALPHA_SHIFT                                        24

#define DRM_CONTROL_REG                              ((LPREG)(DRM_BASE + 0x000))
#define     DRM_CONTROL_MPEG_ENABLE_MASK                           0x00000001
#define     DRM_CONTROL_MPEG_ENABLE_SHIFT                                 0
#define         DRM_MPEG_ENABLE                                      (1UL<<0)
#define     DRM_CONTROL_EXT_VID_ENABLE_MASK                        0x00000002
#define     DRM_CONTROL_EXT_VID_ENABLE_SHIFT                              1
#define     DRM_CONTROL_CURSOR_FIELD_SWAP_MASK                     0x00000004
#define     DRM_CONTROL_CURSOR_FIELD_SWAP_SHIFT                           2
#define     DRM_CONTROL_SAR_ENABLE_MASK                            0x00000008
#define     DRM_CONTROL_SAR_ENABLE_SHIFT                                  3
#define     DRM_CONTROL_OUTPUT_FORMAT_MASK                         0x00000010
#define     DRM_CONTROL_OUTPUT_FORMAT_SHIFT                               4
#define     DRM_CONTROL_VIDEO_TOP_MASK                             0x00000020
#define     DRM_CONTROL_VIDEO_TOP_SHIFT                                   5
#define     DRM_CONTROL_MPEG_FIELD_MAP_MASK                        0x00000040
#define     DRM_CONTROL_MPEG_FIELD_MAP_SHIFT                              6
#define     DRM_CONTROL_EXT_VID_FIELD_MAP_MASK                     0x00000080
#define     DRM_CONTROL_EXT_VID_FIELD_MAP_SHIFT                           7
#define     DRM_CONTROL_VBI_ENABLE_MASK                            0x00000100
#define     DRM_CONTROL_VBI_ENABLE_SHIFT                                  8
#define     DRM_CONTROL_TELETEXT_ENABLE_MASK                       0x00000200
#define     DRM_CONTROL_TELETEXT_ENABLE_SHIFT                             9
#define     DRM_CONTROL_EXT_VID_BUFF_ENABLE_MASK                   0x00001C00
#define     DRM_CONTROL_EXT_VID_BUFF_ENABLE_SHIFT                        10
#define     DRM_CONTROL_EXT_VID_BUFF_SELECT_MASK                   0x0000E000
#define     DRM_CONTROL_EXT_VID_BUFF_SELECT_SHIFT                        13
#define     DRM_CONTROL_EXT_VID_REPEAT_ENABLE_MASK                 0x00010000
#define     DRM_CONTROL_EXT_VID_REPEAT_ENABLE_SHIFT                      16
#define     DRM_CONTROL_TELETEXT_REQ_ENABLE_MASK                   0x00020000
#define     DRM_CONTROL_TELETEXT_REQ_ENABLE_SHIFT                        17
#define     DRM_CONTROL_RESERVED2_MASK                             0x00040000
#define     DRM_CONTROL_RESERVED2_SHIFT                                  18
#define     DRM_CONTROL_BLEND_BACKGROUND_MASK                      0x00080000
#define     DRM_CONTROL_BLEND_BACKGROUND_SHIFT                           19
#define     DRM_CONTROL_PROG_LINE_INT_MASK                         0x3FF00000
#define     DRM_CONTROL_PROG_LINE_INT_SHIFT                              20
#define     DRM_CONTROL_PROG_FIELD_INT_MASK                        0x40000000
#define     DRM_CONTROL_PROG_FIELD_INT_SHIFT                             30
#define     DRM_CONTROL_PROG_LINE_FIELD_ENABLE_MASK                0x80000000
#define     DRM_CONTROL_PROG_LINE_FIELD_ENABLE_SHIFT                     31

#define DRM_INTERRUPT_ENABLE_REG                     ((LPREG)(DRM_BASE + 0x004))
#define     DRM_INTERRUPT_ENABLE_RESERVED1_MASK                    0x0000FFFF
#define     DRM_INTERRUPT_ENABLE_RESERVED1_SHIFT                          0
#define     DRM_INTERRUPT_ENABLE_PROG_LINE_MASK_MASK               0x00010000
#define     DRM_INTERRUPT_ENABLE_PROG_LINE_MASK_SHIFT                    16
#define     DRM_INTERRUPT_ENABLE_LAST_PIXEL_MASK_MASK              0x00020000
#define     DRM_INTERRUPT_ENABLE_LAST_PIXEL_MASK_SHIFT                   17
#define     DRM_INTERRUPT_ENABLE_FIRST_FIELD_MASK_MASK             0x00040000
#define     DRM_INTERRUPT_ENABLE_FIRST_FIELD_MASK_SHIFT                  18
#define     DRM_INTERRUPT_ENABLE_DRM_ERROR_MASK_MASK               0x00080000
#define     DRM_INTERRUPT_ENABLE_DRM_ERROR_MASK_SHIFT                    19
#define     DRM_INTERRUPT_ENABLE_RESERVED2_MASK                    0xFFF00000
#define     DRM_INTERRUPT_ENABLE_RESERVED2_SHIFT                         20

#define DRM_STATUS_REG                               ((LPREG)(DRM_BASE + 0x008))
#define     DRM_STATUS_LINE_MASK                                   0x000003FF
#define     DRM_STATUS_LINE_SHIFT                                         0
#define     DRM_STATUS_RESERVED1_MASK                              0x00000C00
#define     DRM_STATUS_RESERVED1_SHIFT                                   10
#define     DRM_STATUS_FIELD_MASK                                  0x00001000
#define     DRM_STATUS_FIELD_SHIFT                                       12
#define     DRM_STATUS_RESERVED2_MASK                              0x0000E000
#define     DRM_STATUS_RESERVED2_SHIFT                                   13
#define     DRM_STATUS_PROG_LINE_ACTIVE_MASK                       0x00010000
#define     DRM_STATUS_PROG_LINE_ACTIVE_SHIFT                            16
#define         DRM_STATUS_DISPLAY_LINE_ACTIVE                      (1UL<<16)
#define     DRM_STATUS_LAST_PIXEL_MASK                             0x00020000
#define     DRM_STATUS_LAST_PIXEL_SHIFT                                  17
#define         DRM_STATUS_LAST_PIXEL                               (1UL<<17)
#define     DRM_STATUS_FIRST_FIELD_MASK                            0x00040000
#define     DRM_STATUS_FIRST_FIELD_SHIFT                                 18
#define         DRM_STATUS_FIRST_FIELD                              (1UL<<18)
#define     DRM_STATUS_DRM_ERROR_MASK                              0x00080000
#define     DRM_STATUS_DRM_ERROR_SHIFT                                   19
#define         DRM_STATUS_DRM_ERROR                                (1UL<<19)
#define     DRM_STATUS_RESERVED3_MASK                              0x00F00000
#define     DRM_STATUS_RESERVED3_SHIFT                                   20
#define     DRM_STATUS_MPEG_CHROMA0_EMPTY_MASK                     0x01000000
#define     DRM_STATUS_MPEG_CHROMA0_EMPTY_SHIFT                          24
#define     DRM_STATUS_MPEG_LUMA0_EMPTY_MASK                       0x02000000
#define     DRM_STATUS_MPEG_LUMA0_EMPTY_SHIFT                            25
#define     DRM_STATUS_GRAPHICS1_EMPTY_MASK                        0x04000000
#define     DRM_STATUS_GRAPHICS1_EMPTY_SHIFT                             26
#define     DRM_STATUS_GRAPHICS2_EMPTY_MASK                        0x08000000
#define     DRM_STATUS_GRAPHICS2_EMPTY_SHIFT                             27
#define     DRM_STATUS_MPEG_CHROMA1_EMPTY_MASK                     0x10000000
#define     DRM_STATUS_MPEG_CHROMA1_EMPTY_SHIFT                          28
#define     DRM_STATUS_MPEG_LUMA1_EMPTY_MASK                       0x20000000
#define     DRM_STATUS_MPEG_LUMA1_EMPTY_SHIFT                            29
#define     DRM_STATUS_RESERVED4_MASK                              0xC0000000
#define     DRM_STATUS_RESERVED4_SHIFT                                   30

#define DRM_OSD_POINTER_REG                          ((LPREG)(DRM_BASE + 0x00C))
#define DRM_OSD0_POINTER_REG                         ((LPREG)(DRM_BASE + 0x00C))
#define DRM_OSD1_POINTER_REG                         ((LPREG)(DRM_BASE + 0x134))
#define DRM_VBI_BUF_0_REG                            ((LPREG)(DRM_BASE + 0x050))
#define DRM_VBI_BUF_1_REG                            ((LPREG)(DRM_BASE + 0x054))
#define DRM_VID_BUF_0_REG                            ((LPREG)(DRM_BASE + 0x058))
#define DRM_VID_BUF_1_REG                            ((LPREG)(DRM_BASE + 0x05C))
#define DRM_VID_BUF_2_REG                            ((LPREG)(DRM_BASE + 0x060))
#define DRM_TELETEXT_FIELD1_ADDR_REG                 ((LPREG)(DRM_BASE + 0x07C))
#define DRM_TELETEXT_FIELD2_ADDR_REG                 ((LPREG)(DRM_BASE + 0x080))
#define DRM_MPEG_LUMA_ADDR_REG                       ((LPREG)(DRM_BASE + 0x0A4))
#define DRM_MPEG_CHROMA_ADDR_REG                     ((LPREG)(DRM_BASE + 0x0A8))
#define DRM_MPEG_WIPE_LUMA_REG                       ((LPREG)(DRM_BASE + 0x0B0))
#define DRM_MPEG_WIPE_CHROMA_REG                     ((LPREG)(DRM_BASE + 0x0B4))
#define     DRM_ADDRESS_ADDRESS_MASK                               0x07FFFFFF
#define     DRM_ADDRESS_ADDRESS_SHIFT                                     0
#define     DRM_ADDRESS_RESERVED_MASK                              0xF8000000
#define     DRM_ADDRESS_RESERVED_SHIFT                                   27

#define DRM_COLOR_KEY_REG                            ((LPREG)(DRM_BASE + 0x010))
#define     DRM_COLOR_KEY_PIXEL_MASK                               0x00FFFFFF
#define     DRM_COLOR_KEY_PIXEL_SHIFT                                     0
#define     DRM_COLOR_KEY_RESERVED_MASK                            0xFF000000
#define     DRM_COLOR_KEY_RESERVED_SHIFT                                 24

#define DRM_GRAPHICS_BORDER_COLOR_REG                ((LPREG)(DRM_BASE + 0x014))
#define DRM_CURSOR_PALETTE_0_REG                     ((LPREG)(DRM_BASE + 0x08C))
#define DRM_CURSOR_PALETTE_1_REG                     ((LPREG)(DRM_BASE + 0x090))
#define DRM_CURSOR_PALETTE_2_REG                     ((LPREG)(DRM_BASE + 0x094))
#define     DRM_COLOR_Y_MASK                                       0x000000FF
#define     DRM_COLOR_Y_SHIFT                                             0
#define     DRM_COLOR_CB_MASK                                      0x0000FF00
#define     DRM_COLOR_CB_SHIFT                                            8
#define     DRM_COLOR_CR_MASK                                      0x00FF0000
#define     DRM_COLOR_CR_SHIFT                                           16
#define     DRM_COLOR_RESERVED_MASK                                0xFF000000
#define     DRM_COLOR_RESERVED_SHIFT                                     24

#define DRM_DWI_CONTROL_1_REG                        ((LPREG)(DRM_BASE + 0x018))
#define     DRM_DWI_CONTROL_1_MEM_WORD_GROUPS_MASK                 0x0000000F
#define     DRM_DWI_CONTROL_1_MEM_WORD_GROUPS_SHIFT                       0
#define     DRM_DWI_CONTROL_1_PAL_WORD_GROUPS_MASK                 0x000000F0
#define     DRM_DWI_CONTROL_1_PAL_WORD_GROUPS_SHIFT                       4
#define     DRM_DWI_CONTROL_1_PAL_REQ_ENABLE_MASK                  0x00000100
#define     DRM_DWI_CONTROL_1_PAL_REQ_ENABLE_SHIFT                        8
#define     DRM_DWI_CONTROL_1_RESERVED1_MASK                       0x00000200
#define     DRM_DWI_CONTROL_1_RESERVED1_SHIFT                             9
#define     DRM_DWI_CONTROL_1_OSD1_CRIT_MASK                       0x0000FC00
#define     DRM_DWI_CONTROL_1_OSD1_CRIT_SHIFT                            10
#define     DRM_DWI_CONTROL_1_G1_CRIT_MASK                         0x003F0000
#define     DRM_DWI_CONTROL_1_G1_CRIT_SHIFT                              16
#define     DRM_DWI_CONTROL_1_RESERVED2_MASK                       0x00C00000
#define     DRM_DWI_CONTROL_1_RESERVED2_SHIFT                            22
#define     DRM_DWI_CONTROL_1_G2_CRIT_MASK                         0x3F000000
#define     DRM_DWI_CONTROL_1_G2_CRIT_SHIFT                              24
#define     DRM_DWI_CONTROL_1_RESERVED3_MASK                       0xC0000000
#define     DRM_DWI_CONTROL_1_RESERVED3_SHIFT                            30

#define DRM_DWI_CONTROL_2_REG                        ((LPREG)(DRM_BASE + 0x01C))
#define DRM_DWI_CONTROL_3_REG                        ((LPREG)(DRM_BASE + 0x04C))
#define     DRM_DWI_CONTROL_2_EXT_VID_CRIT_MASK                    0x0000003F
#define     DRM_DWI_CONTROL_2_EXT_VID_CRIT_SHIFT                          0
#define     DRM_DWI_CONTROL_2_RESERVED1_MASK                       0x000000C0
#define     DRM_DWI_CONTROL_2_RESERVED1_SHIFT                             6
#define     DRM_DWI_CONTROL_2_MPEG_CHROMA_CRIT_MASK                0x00007F00
#define     DRM_DWI_CONTROL_2_MPEG_CHROMA_CRIT_SHIFT                      8
#define     DRM_DWI_CONTROL_2_RESERVED2_MASK                       0x00008000
#define     DRM_DWI_CONTROL_2_RESERVED2_SHIFT                            15
#define     DRM_DWI_CONTROL_2_MPEG_LUMA1_CRIT_MASK                 0x007F0000
#define     DRM_DWI_CONTROL_2_MPEG_LUMA1_CRIT_SHIFT                      16
#define     DRM_DWI_CONTROL_2_RESERVED3_MASK                       0x00800000
#define     DRM_DWI_CONTROL_2_RESERVED3_SHIFT                            23
#define     DRM_DWI_CONTROL_2_MPEG_LUMA2_CRIT_MASK                 0x3F000000
#define     DRM_DWI_CONTROL_2_MPEG_LUMA2_CRIT_SHIFT                      24
#define     DRM_DWI_CONTROL_2_SPLIT_FIFO_CHROMA_MASK               0x40000000
#define     DRM_DWI_CONTROL_2_SPLIT_FIFO_CHROMA_SHIFT                    30
#define     DRM_DWI_CONTROL_2_SPLIT_FIFO_LUMA_MASK                 0x80000000
#define     DRM_DWI_CONTROL_2_SPLIT_FIFO_LUMA_SHIFT                      31

#define DRM_SCREEN_START_REG                         ((LPREG)(DRM_BASE + 0x020))
#define     DRM_SCREEN_START_FIRST_ACTIVE_PIXEL_MASK               0x000003FF
#define     DRM_SCREEN_START_FIRST_ACTIVE_PIXEL_SHIFT                     0
#define     DRM_SCREEN_START_RESERVED_MASK                         0x00000C00
#define     DRM_SCREEN_START_RESERVED_SHIFT                              10
#define     DRM_SCREEN_START_FIELD2_ACTIVE_LINE_MASK               0x003FF000
#define     DRM_SCREEN_START_FIELD2_ACTIVE_LINE_SHIFT                    12
#define     DRM_SCREEN_START_FIELD1_ACTIVE_LINE_MASK               0xFFC00000
#define     DRM_SCREEN_START_FIELD1_ACTIVE_LINE_SHIFT                    22

#define DRM_SCREEN_END_REG                           ((LPREG)(DRM_BASE + 0x024))
#define     DRM_SCREEN_END_LAST_ACTIVE_PIXEL_MASK                  0x00000FFF
#define     DRM_SCREEN_END_LAST_ACTIVE_PIXEL_SHIFT                        0
#define     DRM_SCREEN_END_FIELD2_LAST_LINE_MASK                   0x003FF000
#define     DRM_SCREEN_END_FIELD2_LAST_LINE_SHIFT                        12
#define     DRM_SCREEN_END_FIELD1_LAST_LINE_MASK                   0xFFC00000
#define     DRM_SCREEN_END_FIELD1_LAST_LINE_SHIFT                        22

#define DRM_CONTROL_2_REG                            ((LPREG)(DRM_BASE + 0x02C))
#define     DRM_CONTROL_2_MPEG_ENABLE_MASK                         0x00000001
#define     DRM_CONTROL_2_MPEG_ENABLE_SHIFT                               0
#define     DRM_CONTROL_2_EXT_VID_ENABLE_MASK                      0x00000002
#define     DRM_CONTROL_2_EXT_VID_ENABLE_SHIFT                            1
#define     DRM_CONTROL_2_MPEG_FIELD_MAP_MASK                      0x00000004
#define     DRM_CONTROL_2_MPEG_FIELD_MAP_SHIFT                            2
#define     DRM_CONTROL_2_EXT_VID_FIELD_MAP_MASK                   0x00000008
#define     DRM_CONTROL_2_EXT_VID_FIELD_MAP_SHIFT                         3
#define     DRM_CONTROL_2_VBI_ENABLE_MASK                          0x00000010
#define     DRM_CONTROL_2_VBI_ENABLE_SHIFT                                4
#define     DRM_CONTROL_2_EXT_VID_BUFF_ENABLE_MASK                 0x000000E0
#define     DRM_CONTROL_2_EXT_VID_BUFF_ENABLE_SHIFT                       5
#define     DRM_CONTROL_2_EXT_VID_BUFF_SELECT_MASK                 0x00000700
#define     DRM_CONTROL_2_EXT_VID_BUFF_SELECT_SHIFT                       8
#define     DRM_CONTROL_2_EXT_VID_REPEAT_ENABLE_MASK               0x00000800
#define     DRM_CONTROL_2_EXT_VID_REPEAT_ENABLE_SHIFT                    11
#define     DRM_CONTROL_2_RESERVED_MASK                            0x00003000
#define     DRM_CONTROL_2_RESERVED_SHIFT                                 12
#define     DRM_CONTROL_2_ENCODER_CLK_CNTRL_MASK                   0x00004000
#define     DRM_CONTROL_2_ENCODER_CLK_CNTRL_SHIFT                        14
#define     DRM_CONTROL_2_ENCODER_ALPHA_SELECT_MASK                0x00008000
#define     DRM_CONTROL_2_ENCODER_ALPHA_SELECT_SHIFT                     15
#define     DRM_CONTROL_2_DVB_VIDEO_SELECT_MASK                    0x00010000
#define     DRM_CONTROL_2_DVB_VIDEO_SELECT_SHIFT                         16
#define     DRM_CONTROL_2_BLEND_COMBINATION_MASK                   0x00020000
#define     DRM_CONTROL_2_BLEND_COMBINATION_SHIFT                        17
#define     DRM_CONTROL_2_RESERVED2_MASK                           0xFFFC0000
#define     DRM_CONTROL_2_RESERVED2_SHIFT                                18

#define DRM_MPEG_OFFSET_WIDTH_REG                    ((LPREG)(DRM_BASE + 0x030))
#define     DRM_MPEG_OFFSET_WIDTH_ADDR_OFFSET_MASK                 0x000003FF
#define     DRM_MPEG_OFFSET_WIDTH_ADDR_OFFSET_SHIFT                       0
#define     DRM_MPEG_OFFSET_WIDTH_PAN_SCAN_OFFSET_MASK             0x000FFC00
#define     DRM_MPEG_OFFSET_WIDTH_PAN_SCAN_OFFSET_SHIFT                  10
#define     DRM_MPEG_OFFSET_WIDTH_FETCH_WIDTH_MASK                 0x3FF00000
#define     DRM_MPEG_OFFSET_WIDTH_FETCH_WIDTH_SHIFT                      20
#define     DRM_MPEG_OFFSET_WIDTH_RESERVED1_MASK                   0xC0000000
#define     DRM_MPEG_OFFSET_WIDTH_RESERVED1_SHIFT                        30

#define DRM_MPEG_VERTICAL_OFFSET_REG                 ((LPREG)(DRM_BASE + 0x034))
#define     DRM_MPEG_VOFFSET_VERTICAL_OFFSET_MASK                  0x000003FF
#define     DRM_MPEG_VOFFSET_VERTICAL_OFFSET_SHIFT                        0
#define     DRM_MPEG_VOFFSET_RESERVED1_MASK                        0x0000FC00
#define     DRM_MPEG_VOFFSET_RESERVED1_SHIFT                             10
#define     DRM_MPEG_VOFFSET_FETCH_HEIGHT_MASK                     0x03FF0000
#define     DRM_MPEG_VOFFSET_FETCH_HEIGHT_SHIFT                          16
#define     DRM_MPEG_VOFFSET_RESERVED2_MASK                        0xFC000000
#define     DRM_MPEG_VOFFSET_RESERVED2_SHIFT                             26

#define DRM_MPEG_POSITION_REG                        ((LPREG)(DRM_BASE + 0x038))
#define DRM_VID_POSITION_REG                         ((LPREG)(DRM_BASE + 0x064))
#define     DRM_POSITION_X_MASK                                    0x00000FFF
#define     DRM_POSITION_X_SHIFT                                          0
#define     DRM_POSITION_RESERVED1_MASK                            0x0000F000
#define     DRM_POSITION_RESERVED1_SHIFT                                 12
#define     DRM_POSITION_FIELD_START_MASK                          0x00010000
#define     DRM_POSITION_FIELD_START_SHIFT                               16
#define     DRM_POSITION_Y_MASK                                    0x07FE0000
#define     DRM_POSITION_Y_SHIFT                                         17
#define     DRM_POSITION_RESERVED2_MASK                            0xF8000000
#define     DRM_POSITION_RESERVED2_SHIFT                                 27

#define DRM_MPEG_SIZE_REG                            ((LPREG)(DRM_BASE + 0x03C))
#define DRM_MPEG_HEIGHTWIDTH_REG                     ((LPREG)(DRM_BASE + 0x0AC))
#define     DRM_SIZE_WIDTH_MASK                                    0x000003FF
#define     DRM_SIZE_WIDTH_SHIFT                                          0
#define     DRM_SIZE_RESERVED1_MASK                                0x0000FC00
#define     DRM_SIZE_RESERVED1_SHIFT                                     10
#define     DRM_SIZE_HEIGHT_MASK                                   0x03FF0000
#define     DRM_SIZE_HEIGHT_SHIFT                                        16
#define     DRM_SIZE_RESERVED2_MASK                                0xFC000000
#define     DRM_SIZE_RESERVED2_SHIFT                                     26

#define DRM_MPEG_X_INC_REG                           ((LPREG)(DRM_BASE + 0x040))
#define     DRM_MPEG_X_INC_X_INC_MASK                              0x0000FFFF
#define     DRM_MPEG_X_INC_X_INC_SHIFT                                    0
#define     DRM_MPEG_X_INC_H_FILTER_SELECT_MASK                    0x00030000
#define     DRM_MPEG_X_INC_H_FILTER_SELECT_SHIFT                         16
#define     DRM_MPEG_X_INC_V_FILTER_SELECT_MASK                    0x00040000
#define     DRM_MPEG_X_INC_V_FILTER_SELECT_SHIFT                         18
#define     DRM_MPEG_X_INC_V_FILTER_MODE_MASK                      0x00080000
#define     DRM_MPEG_X_INC_V_FILTER_MODE_SHIFT                           19
#define     DRM_MPEG_X_INC_LUMA_FRAME_SCALE_MASK                   0x00100000
#define     DRM_MPEG_X_INC_LUMA_FRAME_SCALE_SHIFT                        20
#define     DRM_MPEG_X_INC_CHROMA_FRAME_SCALE_MASK                 0x00200000
#define     DRM_MPEG_X_INC_CHROMA_FRAME_SCALE_SHIFT                      21
#define     DRM_MPEG_X_INC_MIDDLE_NOT_BOTTOM_MASK                  0x00400000
#define     DRM_MPEG_X_INC_MIDDLE_NOT_BOTTOM_SHIFT                       22
#define     DRM_MPEG_X_INC_RESERVED_MASK                           0x0F800000
#define     DRM_MPEG_X_INC_RESERVED_SHIFT                                23
#define     DRM_MPEG_X_INC_DEC_HORZ_CHROMA_MASK                    0x10000000
#define     DRM_MPEG_X_INC_DEC_HORZ_CHROMA_SHIFT                         28
#define     DRM_MPEG_X_INC_DEC_VERT_CHROMA_MASK                    0x20000000
#define     DRM_MPEG_X_INC_DEC_VERT_CHROMA_SHIFT                         29
#define     DRM_MPEG_X_INC_DEC_HORZ_LUMA_MASK                      0x40000000
#define     DRM_MPEG_X_INC_DEC_HORZ_LUMA_SHIFT                           30
#define     DRM_MPEG_X_INC_DEC_VERT_LUMA_MASK                      0x80000000
#define     DRM_MPEG_X_INC_DEC_VERT_LUMA_SHIFT                           31

#define DRM_MPEG_Y_INC_REG                           ((LPREG)(DRM_BASE + 0x044))
#define     DRM_MPEG_Y_INC_B_Y_INC_MASK                            0x0000FFFF
#define     DRM_MPEG_Y_INC_B_Y_INC_SHIFT                                  0
#define     DRM_MPEG_Y_INC_IP_Y_INC_MASK                           0xFFFF0000
#define     DRM_MPEG_Y_INC_IP_Y_INC_SHIFT                                16

#define DRM_SHARP_REG                                ((LPREG)(DRM_BASE + 0x048))
#define     DRM_SHARP_MPG_SHARP_CFF0_MASK                          0x0000000F
#define     DRM_SHARP_MPG_SHARP_CFF0_SHIFT                                0
#define     DRM_SHARP_MPG_SHARP_CFF1_MASK                          0x00000FF0
#define     DRM_SHARP_MPG_SHARP_CFF1_SHIFT                                4
#define     DRM_SHARP_MPG_SHARP_CFF2_MASK                          0x0001F000
#define     DRM_SHARP_MPG_SHARP_CFF2_SHIFT                               12
#define     DRM_SHARP_RESERVED1_MASK                               0x000E0000
#define     DRM_SHARP_RESERVED1_SHIFT                                    17
#define     DRM_SHARP_MPG_SHARP_NORM_MASK                          0x00F00000
#define     DRM_SHARP_MPG_SHARP_NORM_SHIFT                               20
#define     DRM_SHARP_RESERVED2_MASK                               0xFF000000
#define     DRM_SHARP_RESERVED2_SHIFT                                    24

#define DRM_VID_SIZE_REG                             ((LPREG)(DRM_BASE + 0x068))
#define     DRM_VID_SIZE_WIDTH_MASK                                0x000003FF
#define     DRM_VID_SIZE_WIDTH_SHIFT                                      0
#define     DRM_VID_SIZE_RESERVED1_MASK                            0x0000FC00
#define     DRM_VID_SIZE_RESERVED1_SHIFT                                 10
#define     DRM_VID_SIZE_HEIGHT_MASK                               0x03FF0000
#define     DRM_VID_SIZE_HEIGHT_SHIFT                                    16
#define     DRM_VID_SIZE_VBI_FETCH_CNT_MASK                        0xFC000000
#define     DRM_VID_SIZE_VBI_FETCH_CNT_SHIFT                             26

#define DRM_VID_STRIDE_REG                           ((LPREG)(DRM_BASE + 0x06C))
#define     DRM_VID_STRIDE_STRIDE_MASK                             0x00000FFF
#define     DRM_VID_STRIDE_STRIDE_SHIFT                                   0
#define     DRM_VID_STRIDE_RESERVED1_MASK                          0x0000F000
#define     DRM_VID_STRIDE_RESERVED1_SHIFT                               12
#define     DRM_VID_STRIDE_FIELD_START_MASK                        0x00010000
#define     DRM_VID_STRIDE_FIELD_START_SHIFT                             16
#define     DRM_VID_STRIDE_VBI_START_LINE_MASK                     0x07FE0000
#define     DRM_VID_STRIDE_VBI_START_LINE_SHIFT                          17
#define     DRM_VID_STRIDE_RESERVED2_MASK                          0xF8000000
#define     DRM_VID_STRIDE_RESERVED2_SHIFT                               27

#define DRM_TELETEXT_STRIDE_REG                      ((LPREG)(DRM_BASE + 0x070))
#define     DRM_TELETEXT_STRIDE_LINE_STRIDE_MASK                   0x00000FFF
#define     DRM_TELETEXT_STRIDE_LINE_STRIDE_SHIFT                         0
#define     DRM_TELETEXT_STRIDE_RESERVED_MASK                      0x0000F000
#define     DRM_TELETEXT_STRIDE_RESERVED_SHIFT                           12
#define     DRM_TELETEXT_STRIDE_TT_DISABLE_LOW_MASK                0x00FF0000
#define     DRM_TELETEXT_STRIDE_TT_DISABLE_LOW_SHIFT                     16
#define     DRM_TELETEXT_STRIDE_TT_DISABLE_HIGH_MASK               0xFF000000
#define     DRM_TELETEXT_STRIDE_TT_DISABLE_HIGH_SHIFT                    24

#define DRM_TELETEXT_FIELD1_REG                      ((LPREG)(DRM_BASE + 0x074))
#define DRM_TELETEXT_FIELD2_REG                      ((LPREG)(DRM_BASE + 0x078))
#define     DRM_FIELD1_TELETEXT_LAST_LINE_MASK                     0x000001FF
#define     DRM_FIELD1_TELETEXT_LAST_LINE_SHIFT                           0
#define     DRM_FIELD1_TELETEXT_RESERVED1_MASK                     0x0000FE00
#define     DRM_FIELD1_TELETEXT_RESERVED1_SHIFT                           9
#define     DRM_FIELD1_TELETEXT_FIRST_LINE_MASK                    0x01FF0000
#define     DRM_FIELD1_TELETEXT_FIRST_LINE_SHIFT                         16
#define     DRM_FIELD1_TELETEXT_RESERVED2_MASK                     0xFE000000
#define     DRM_FIELD1_TELETEXT_RESERVED2_SHIFT                          25

#define DRM_CURSOR_CONTROL_REG                       ((LPREG)(DRM_BASE + 0x084))
#define     DRM_CURSOR_CONTROL_X_MASK                              0x00000FFF
#define     DRM_CURSOR_CONTROL_X_SHIFT                                    0
#define     DRM_CURSOR_CONTROL_RESERVED1_MASK                      0x0000F000
#define     DRM_CURSOR_CONTROL_RESERVED1_SHIFT                           12
#define     DRM_CURSOR_CONTROL_FIELD_START_MASK                    0x00010000
#define     DRM_CURSOR_CONTROL_FIELD_START_SHIFT                         16
#define     DRM_CURSOR_CONTROL_Y_MASK                              0x07FE0000
#define     DRM_CURSOR_CONTROL_Y_SHIFT                                   17
#define     DRM_CURSOR_CONTROL_RESERVED2_MASK                      0x18000000
#define     DRM_CURSOR_CONTROL_RESERVED2_SHIFT                           27
#define     DRM_CURSOR_CONTROL_CURSOR_ENABLE_MASK                  0x20000000
#define     DRM_CURSOR_CONTROL_CURSOR_ENABLE_SHIFT                       29
#define     DRM_CURSOR_CONTROL_INVERT_MASK                         0xC0000000
#define     DRM_CURSOR_CONTROL_INVERT_SHIFT                              30

#define DRM_CURSOR_STORE_ADDR_REG                    ((LPREG)(DRM_BASE + 0x088))
#define     DRM_CURSOR_ADDRESS_ADDRESS_MASK                        0x07FFFFFF
#define     DRM_CURSOR_ADDRESS_ADDRESS_SHIFT                              0
#define     DRM_CURSOR_ADDRESS_RESERVED1_MASK                      0xF8000000
#define     DRM_CURSOR_ADDRESS_RESERVED1_SHIFT                           27

#define DRM_CURSOR_FETCH_SIZE_REG                    ((LPREG)(DRM_BASE + 0x098))
#define     DRM_CURSOR_FETCH_SIZE_LINES_MASK                       0x7F000000
#define     DRM_CURSOR_FETCH_SIZE_LINES_SHIFT                            24

#define DRM_SCALING_COEFF_REG                        ((LPREG)(DRM_BASE + 0x09c))
#define     DRM_SCALING_COEFF_MASK                                 0x003FFFFF
#define     DRM_SCALING_COEFF_SHIFT                                       0
#define     DRM_SCALING_COEFF_0_5_MASK                             0x0000000F
#define     DRM_SCALING_COEFF_0_5_SHIFT                                   0
#define     DRM_SCALING_COEFF_1_4_MASK                             0x000007F0
#define     DRM_SCALING_COEFF_1_4_SHIFT                                   4
#define     DRM_SCALING_COEFF_2_3_MASK                             0x003FF800
#define     DRM_SCALING_COEFF_2_3_SHIFT                                  11
#define     DRM_SCALING_COEFF_MPEG_SELECT_MASK                     0x04000000
#define     DRM_SCALING_COEFF_MPEG_SELECT_SHIFT                          26
#define         DRM_SCALING_COEFF_MPEG_SELECT_0                     (0UL<<26)
#define         DRM_SCALING_COEFF_MPEG_SELECT_1                     (1UL<<26)
#define     DRM_SCALING_COEFF_INITIALIZE_MASK                      0x08000000
#define     DRM_SCALING_COEFF_INITIALIZE_SHIFT                           27
#define         DRM_SCALING_COEFF_INITIALIZE_ADDR_ZERO              (0UL<<27)
#define         DRM_SCALING_COEFF_INITIALISE_ADDR_INC               (1UL<<27)
#define     DRM_SCALING_COEFF_NORM_MASK                            0xF0000000
#define     DRM_SCALING_COEFF_NORM_SHIFT                                 28


#define DRM_MPEG_STILL_CONTROL_REG                   ((LPREG)(DRM_BASE + 0x0A0))
#define     DRM_MPG_VIDEO_PARAM_CENTER_PAN_OFFSET_MASK             0x00000FFF
#define     DRM_MPG_VIDEO_PARAM_CENTER_PAN_OFFSET_SHIFT                   0
#define     DRM_MPG_VIDEO_PARAM_FRAME_NOT_FIELD_MASK               0x00001000
#define     DRM_MPG_VIDEO_PARAM_FRAME_NOT_FIELD_SHIFT                    12
#define     DRM_MPG_VIDEO_PARAM_FIELD_NOT_LINE_MASK                0x00002000
#define     DRM_MPG_VIDEO_PARAM_FIELD_NOT_LINE_SHIFT                     13
#define     DRM_MPG_VIDEO_PARAM_PAN_NOT_LETTERBOX_MASK             0x00004000
#define     DRM_MPG_VIDEO_PARAM_PAN_NOT_LETTERBOX_SHIFT                  14
#define     DRM_MPG_VIDEO_PARAM_ASPECT_RATIO_MASK                  0x00008000
#define     DRM_MPG_VIDEO_PARAM_ASPECT_RATIO_SHIFT                       15
#define     DRM_MPG_VIDEO_PARAM_WIPE_LINE_NUMBER_MASK              0x03FF0000
#define     DRM_MPG_VIDEO_PARAM_WIPE_LINE_NUMBER_SHIFT                   16
#define     DRM_MPG_VIDEO_PARAM_RESERVED_MASK                      0x0C000000
#define     DRM_MPG_VIDEO_PARAM_RESERVED_SHIFT                           26
#define     DRM_MPG_VIDEO_PARAM_MEM_EXTENSION_MASK                 0x70000000
#define     DRM_MPG_VIDEO_PARAM_MEM_EXTENSION_SHIFT                      28
#define     DRM_MPG_VIDEO_PARAM_STILL_ENABLE_MASK                  0x80000000
#define     DRM_MPG_VIDEO_PARAM_STILL_ENABLE_SHIFT                       31

#define DRM_MPEG_TILE_PARMS_REG                      ((LPREG)(DRM_BASE + 0x0B8))
#define     DRM_MPG_TILE_WIPE_CTRL_TILE_WIDTH_MASK                 0x000003FF
#define     DRM_MPG_TILE_WIPE_CTRL_TILE_WIDTH_SHIFT                       0
#define     DRM_MPG_TILE_WIPE_CTRL_RESERVED_MASK                   0x00000400
#define     DRM_MPG_TILE_WIPE_CTRL_RESERVED_SHIFT                        10
#define     DRM_MPG_TILE_WIPE_CTRL_TILE_HEIGHT_MASK                0x001FF800
#define     DRM_MPG_TILE_WIPE_CTRL_TILE_HEIGHT_SHIFT                     11
#define     DRM_MPG_TILE_WIPE_CTRL_RESERVED2_MASK                  0x00E00000
#define     DRM_MPG_TILE_WIPE_CTRL_RESERVED2_SHIFT                       21
#define     DRM_MPG_TILE_WIPE_CTRL_TILE_FACTOR_MASK                0x0F000000
#define     DRM_MPG_TILE_WIPE_CTRL_TILE_FACTOR_SHIFT                     24
#define     DRM_MPG_TILE_WIPE_CTRL_TILE_ENABLE_MASK                0x10000000
#define     DRM_MPG_TILE_WIPE_CTRL_TILE_ENABLE_SHIFT                     28
#define     DRM_MPG_TILE_WIPE_CTRL_WIPE_ENABLE_MASK                0x20000000
#define     DRM_MPG_TILE_WIPE_CTRL_WIPE_ENABLE_SHIFT                     29
#define     DRM_MPG_TILE_WIPE_CTRL_RESERVED3_MASK                  0xC0000000
#define     DRM_MPG_TILE_WIPE_CTRL_RESERVED3_SHIFT                       30

#define DRM_VIDEO_ALPHA_REG                          ((LPREG)(DRM_BASE + 0x12C))
#define     DRM_VIDEO_ALPHA_RESERVED_MASK                          0x0000FFFF
#define     DRM_VIDEO_ALPHA_RESERVED_SHIFT                                0
#define     DRM_VIDEO_ALPHA_VIDEO1_ALPHA_MASK                      0x00FF0000
#define     DRM_VIDEO_ALPHA_VIDEO1_ALPHA_SHIFT                           16
#define     DRM_VIDEO_ALPHA_VIDEO0_ALPHA_MASK                      0xFF000000
#define     DRM_VIDEO_ALPHA_VIDEO0_ALPHA_SHIFT                           24

/* General DRM definitions */
#define DRM_FIELD_1 0
#define DRM_FIELD_2 1

#if defined(BITFIELDS) || defined(USE_BITFIELD_DRM)

typedef struct _DRMRGBPAL {
         BIT_FLD        R:8,
                        G:8,
                        B:8,
                        Alpha:8;
} DRMRGBPAL;

typedef struct _DRMYCCPAL {
         BIT_FLD        Y:8,
                        Cb:8,
                        Cr:8,
                        Alpha:8;
} DRMYCCPAL;

typedef struct _DRM_CONTROL {
        BIT_FLD MpegEnable:1,
                ExtVidEnable:1,
                CursorFieldSwap:1,
                SAREnable:1,
                OutputFormat:1,
                VideoTop:1,
                MpegFieldMap:1,
                ExtVidFieldMap:1,
                VBIEnable:1,
                TeletextEnable:1,
                ExtVidBuffEnable:3,
                ExtVidBuffSelect:3,
                ExtVidRepeatEnable:1,
                TeletextReqEnable:1,
                Reserved2:1,
                BlendBackground:1,
                ProgLineInt:10,
                ProgFieldInt:1,
                ProgLineFieldEnable:1;
}DRM_CONTROL;
typedef DRM_CONTROL volatile *LPDRM_CONTROL;

typedef struct _DRM_INTERRUPT_ENABLE {
        BIT_FLD Reserved1:16,
                ProgLineMask:1,
                LastPixelMask:1,
                FirstFieldMask:1,
                DRMErrorMask:1,
                Reserved2:12;
} DRM_INTERRUPT_ENABLE;
typedef DRM_INTERRUPT_ENABLE volatile *LPDRM_INTERRUPT_ENABLE;

typedef struct _DRM_STATUS {
        BIT_FLD Line:10,
                Reserved1:2,
                Field:1,
                Reserved2:3,
                ProgLineActive:1,
                LastPixel:1,
                FirstField:1,
                DRMError:1,
                Reserved3:4,
                MPEGChroma0Empty:1,
                MPEGLuma0Empty:1,
                Graphics1Empty:1,
                Graphics2Empty:1,
                MPEGChroma1Empty:1,
                MPEGLuma1Empty:1,
                Reserved4:2;
} DRM_STATUS;
typedef DRM_STATUS volatile *LPDRM_STATUS;

typedef struct _DRM_ADDRESS {
        BIT_FLD Address:27,
                Reserved:5;
} DRM_ADDRESS;
typedef DRM_ADDRESS volatile *LPDRM_ADDRESS;

typedef DRM_ADDRESS DRM_OSD_POINTER;
typedef LPDRM_ADDRESS LPDRM_OSD_POINTER;

typedef struct _DRM_COLOR_KEY {
        BIT_FLD Pixel:24,
                Reserved:8;
} DRM_COLOR_KEY;
typedef DRM_COLOR_KEY volatile *LPDRM_COLOR_KEY;

typedef struct _DRM_COLOR {
        BIT_FLD Y:8,
                Cb:8,
                Cr:8,
                Reserved:8;
} DRM_COLOR;
typedef DRM_COLOR volatile *LPDRM_COLOR;

typedef DRM_COLOR   DRM_GRAPHICS_BORDER_COLOR;
typedef LPDRM_COLOR LPDRM_GRAPHICS_BORDER_COLOR;

typedef struct _DRM_DWI_CONTROL_1 {
        BIT_FLD MemWordGroups:4,
                PalWordGroups:4,
                PalReqEnable:1,
                Reserved1:7,
                G1Crit:6,
                Reserved2:2,
                G2Crit:6,
                Reserved3:2;
} DRM_DWI_CONTROL_1;
typedef DRM_DWI_CONTROL_1 volatile *LPDRM_DWI_CONTROL_1;

typedef struct _DRM_DWI_CONTROL_2 {
        BIT_FLD ExtVidCrit:6,
                Reserved1:2,
                MpegChromaCrit:7,
                Reserved2:1,
                MpegLuma1Crit:7,
                Reserved3:1,
                MpegLuma2Crit:6,
                SplitFIFOChroma:1,
                SplitFIFOLuma:1;
} DRM_DWI_CONTROL_2;
typedef DRM_DWI_CONTROL_2 volatile *LPDRM_DWI_CONTROL_2;

typedef struct _DRM_SCREEN_START {
        BIT_FLD FirstActivePixel:10,
                Reserved:2,
                Field2ActiveLine:10,
                Field1ActiveLine:10;
} DRM_SCREEN_START;
typedef DRM_SCREEN_START volatile *LPDRM_SCREEN_START;

typedef struct _DRM_SCREEN_END {
        BIT_FLD LastActivePixel:12,
                Field2LastLine:10,
                Field1LastLine:10;
} DRM_SCREEN_END;
typedef DRM_SCREEN_END volatile *LPDRM_SCREEN_END;

/*////////////////////// */
typedef struct _DRM_CONTROL_2 {
        BIT_FLD MpegEnable:1,
                ExtVidEnable:1,
                MpegFieldMap:1,
                ExtVidFieldMap:1,
                VBIEnable:1,
                ExtVidBuffEnable:3,
                ExtVidBuffSelect:3,
                ExtVidRepeatEnable:1,
                Reserved:2,
                EncoderClkCntrl:1,
                EncoderAlphaSelect:1,
                DVBVideoSelect:1,
                BlendCombination:1,
                Reserved2:14;
}DRM_CONTROL_2;
typedef DRM_CONTROL_2 volatile *LPDRM_CONTROL_2;

typedef struct _DRM_MPEG_OFFSET_WIDTH {
        BIT_FLD AddrOffset:10,
                PanScanOffset:10,
                FetchWidth:10,
                Reserved1:2;
} DRM_MPEG_OFFSET_WIDTH;
typedef DRM_MPEG_OFFSET_WIDTH volatile *LPDRM_MPEG_OFFSET_WIDTH;

typedef struct _DRM_MPEG_VOFFSET {
        BIT_FLD VerticalOffset:10,
                Reserved1:6,
                FetchHeight:10,
                Reserved2:6;
} DRM_MPEG_VOFFSET;
typedef DRM_MPEG_VOFFSET volatile *LPDRM_MPEG_VOFFSET;

typedef struct _DRM_POSITION {
        BIT_FLD X:12,
                reserved1:4,
                FieldStart:1,
                Y:10,
                reserved2:5;
} DRM_POSITION;
typedef DRM_POSITION volatile *LPDRM_POSITION;
typedef DRM_POSITION DRM_MPEG_POSITION;
typedef LPDRM_POSITION LPDRM_MPEG_POSITION;

typedef struct _DRM_SIZE {
        BIT_FLD Width:10,
                Reserved1:6,
                Height:10,
                Reserved2:6;
} DRM_SIZE;
typedef DRM_SIZE volatile *LPDRM_SIZE;
typedef DRM_SIZE DRM_MPEG_SIZE;
typedef LPDRM_SIZE LPDRM_MPEG_SIZE;

typedef struct _DRM_MPEG_X_INC {
        BIT_FLD XInc:16,
                HFilterSelect:2,
                VFilterSelect:1,
                VFilterMode:1,
                LumaFrameScale:1,
                ChromaFrameScale:1,
                MiddleNotBottom:1,
                Reserved:5,
                DecHorzChroma:1,
                DecVertChroma:1,
                DecHorzLuma:1,
                DecVertLuma:1;
} DRM_MPEG_X_INC;
typedef DRM_MPEG_X_INC volatile *LPDRM_MPEG_X_INC;

typedef struct _DRM_MPEG_Y_INC {
        BIT_FLD B_YInc:16,
                IP_YInc:16;
} DRM_MPEG_Y_INC;
typedef DRM_MPEG_Y_INC volatile *LPDRM_MPEG_Y_INC;

typedef struct _DRM_SHARP {
        BIT_FLD MpgSharpCff0:4,
                MpgSharpCff1:8,
                MpgSharpCff2:5,
                Reserved1:3,
                MpgSharpNorm:4,
                Reserved2:8;
} DRM_SHARP;
typedef DRM_SHARP volatile *LPDRM_SHARP;

typedef DRM_ADDRESS   DRM_VBI_BUF_0;
typedef DRM_ADDRESS   DRM_VBI_BUF_1;
typedef DRM_ADDRESS   DRM_VID_BUF_0;
typedef DRM_ADDRESS   DRM_VID_BUF_1;
typedef DRM_ADDRESS   DRM_VID_BUF_2;
typedef LPDRM_ADDRESS LPDRM_VBI_BUF_0;
typedef LPDRM_ADDRESS LPDRM_VBI_BUF_1;
typedef LPDRM_ADDRESS LPDRM_VID_BUF_0;
typedef LPDRM_ADDRESS LPDRM_VID_BUF_1;
typedef LPDRM_ADDRESS LPDRM_VID_BUF_2;

typedef DRM_POSITION   DRM_VID_POSITION;
typedef LPDRM_POSITION LPDRM_VID_POSITION;

typedef struct _DRM_VID_SIZE {
        BIT_FLD Width:10,
                Reserved1:6,
                Height:10,
                VBIFetchCnt:6;
} DRM_VID_SIZE;
typedef DRM_VID_SIZE volatile *LPDRM_VID_SIZE;

typedef struct _DRM_VID_STRIDE {
        BIT_FLD Stride:12,
                Reserved1:4,
                FieldStart:1,
                VBIStartLine:10,
                Reserved2:5;
} DRM_VID_STRIDE;
typedef DRM_VID_STRIDE volatile *LPDRM_VID_STRIDE;

typedef struct _DRM_TELETEXT_STRIDE {
        BIT_FLD LineStride:12,
                Reserved:4,
                TTDisableLow:8,
                TTDisableHigh:8;
} DRM_TELETEXT_STRIDE;
typedef DRM_TELETEXT_STRIDE volatile *LPDRM_TELETEXT_STRIDE;

typedef struct _DRM_FIELD1_TELETEXT {
        BIT_FLD LastLine:9,
                Reserved1:7,
                FirstLine:9,
                Reserved2:7;
} DRM_FIELD1_TELETEXT;
typedef DRM_FIELD1_TELETEXT volatile *LPDRM_FIELD1_TELETEXT;

typedef DRM_FIELD1_TELETEXT   DRM_FIELD2_TELETEXT;
typedef LPDRM_FIELD1_TELETEXT LPDRM_FIELD2_TELETEXT;
typedef DRM_ADDRESS   DRM_TELETEXT_ADDR;
typedef LPDRM_ADDRESS LPDRM_TELETEXT_ADDR;

typedef struct _DRM_CURSOR_CONTROL {
        BIT_FLD X:12,
                Reserved1:4,
                FieldStart:1,
                Y:10,
                Reserved2:2,
                CursorEnable:1,
                Invert:2;
} DRM_CURSOR_CONTROL;
typedef DRM_CURSOR_CONTROL volatile *LPDRM_CURSOR_CONTROL;

typedef struct _DRM_CURSOR_ADDRESS {
        BIT_FLD Address:24,
                FetchSize:7,
                Reserved1:1;
} DRM_CURSOR_ADDRESS;
typedef DRM_CURSOR_ADDRESS volatile *LPDRM_CURSOR_ADDRESS;

typedef DRM_COLOR   DRM_CURSOR_PALETTE_0;
typedef DRM_COLOR   DRM_CURSOR_PALETTE_1;
typedef DRM_COLOR   DRM_CURSOR_PALETTE_2;
typedef LPDRM_COLOR LPDRM_CURSOR_PALETTE_0;
typedef LPDRM_COLOR LPDRM_CURSOR_PALETTE_1;
typedef LPDRM_COLOR LPDRM_CURSOR_PALETTE_2;

typedef struct _DRM_MPG_VIDEO_PARAM {
        BIT_FLD CenterPanOffset:12,
                FrameNotField:1,
                FieldNotLine:1,
                PanNotLetterbox:1,
                AspectRatio:1,
                WipeLineNumber:10,
                Reserved:2,
                MemExtension:3,
                StillEnable:1;
} DRM_MPG_VIDEO_PARAM;
typedef DRM_MPG_VIDEO_PARAM volatile *LPDRM_MPG_VIDEO_PARAM;

typedef DRM_ADDRESS DRM_MPG_LUMA_ADDR;
typedef LPDRM_ADDRESS LPDRM_MPG_LUMA_ADDR;
typedef DRM_ADDRESS DRM_MPG_CHROMA_ADDR;
typedef LPDRM_ADDRESS LPDRM_MPG_CHROMA_ADDR;

typedef DRM_SIZE DRM_MPG_SRC_SIZE;
typedef LPDRM_SIZE LPDRM_MPG_SRC_SIZE;

typedef DRM_ADDRESS DRM_WIPE_LUMA_ADDR;
typedef LPDRM_ADDRESS LPDRM_WIPE_LUMA_ADDR;
typedef DRM_ADDRESS DRM_WIPE_CHROMA_ADDR;
typedef LPDRM_ADDRESS LPDRM_WIPE_CHROMA_ADDR;

typedef struct _DRM_MPG_TILE_WIPE_CTRL {
        BIT_FLD TileWidth:10,
                Reserved:1,
                TileHeight:10,
                Reserved2:3,
                TileFactor:4,
                TileEnable:1,
                WipeEnable:1,
                Reserved3:2;
} DRM_MPG_TILE_WIPE_CTRL;
typedef DRM_MPG_TILE_WIPE_CTRL volatile *LPDRM_MPG_TILE_WIPE_CTRL;

typedef struct _DRM_VIDEO_ALPHA {
        BIT_FLD Reserved:16,
                Video1Alpha:8,
                Video0Alpha:8;
} DRM_VIDEO_ALPHA;
typedef DRM_VIDEO_ALPHA volatile *LPDRM_VIDEO_ALPHA;

typedef struct _DRM_SAR {
        HW_DWORD Sar;
} DRM_SAR;
typedef DRM_SAR volatile *LPDRM_SAR;
#else
typedef HW_DWORD DRMRGBPAL;
typedef HW_DWORD DRMYCCPAL;
#endif /* BITFIELD_DRM */

typedef union _DRMPAL {
         DRMRGBPAL      rgb;
         DRMYCCPAL      ycc;
         HW_DWORD        dwVal;
} DRMPAL, *PDRMPAL;


#endif /* INCL_DRM */

/********************/
/* Internal Encoder */
/********************/

#ifdef INCL_ENC

/*
 *******************************************************************************
 *
 *      ENC_DAC_STATUS_REG                                     (ENC_BASE + 0x00)
 *      ENC_RESET_REG                                          (ENC_BASE + 0x04)
 *      ENC_LINE_TIME_REG                                      (ENC_BASE + 0x10)
 *      ENC_HORIZONTAL_TIMING_REG                              (ENC_BASE + 0x14)
 *      ENC_HBLANK_REG                                         (ENC_BASE + 0x18)
 *      ENC_HSYNC_REG                                          (ENC_BASE + 0x1C)
 *      ENC_CONTROL0_REG                                       (ENC_BASE + 0x20)
 *      ENC_AMPLITUDE_REG                                      (ENC_BASE + 0x24)
 *      ENC_YUV_MULTIPLICATION_REG                             (ENC_BASE + 0x28)
 *      ENC_YCRCB_MULTIPLICATION_REG                           (ENC_BASE + 0x2C)
 *      ENC_SECAM_DR_REG                                       (ENC_BASE + 0x30)
 *      ENC_SECAM_DB_REG                                       (ENC_BASE + 0x34)
 *      ENC_DR_RANGE_REG                                       (ENC_BASE + 0x38)
 *      ENC_DB_RANGE_REG                                       (ENC_BASE + 0x3C)
 *      ENC_HUE_BRIGHT_CTL_REG                                 (ENC_BASE + 0x40)
 *      ENC_CHROMA_CTL_REG                                     (ENC_BASE + 0x44)
 *      ENC_CC_CTL_REG                                         (ENC_BASE + 0x48)
 *      ENC_XDS_DATA_REG                                       (ENC_BASE + 0x4C)
 *      ENC_CC_DATA_REG                                        (ENC_BASE + 0x50)
 *      ENC_XDS_CC_CTL_REG                                     (ENC_BASE + 0x54)
 *      ENC_WS_DATA_REG                                        (ENC_BASE + 0x58)
 *      ENC_TTX_REQ_REG                                        (ENC_BASE + 0x5C)
 *      ENC_TTX_F1_CTL_REG                                     (ENC_BASE + 0x60)
 *      ENC_TTX_F2_CTL_REG                                     (ENC_BASE + 0x64)
 *      ENC_TTX_LINE_CTL_REG                                   (ENC_BASE + 0x68)
 *
 *******************************************************************************
 */

#define ENC_DAC_STATUS_REG                                     (ENC_BASE + 0x00)
#define     ENC_DAC_STS_FIELD_CNT_MASK                             0x0000000F
#define     ENC_DAC_STS_FIELD_CNT_SHIFT                                   0
#define     ENC_DAC_STS_CC_STAT_MASK                               0x00000010
#define     ENC_DAC_STS_CC_STAT_SHIFT                                     4
#define     ENC_DAC_STS_XDS_STAT_MASK                              0x00000020
#define     ENC_DAC_STS_XDS_STAT_SHIFT                                    5
#define     ENC_DAC_STS_MONSTAT_A_MASK                             0x00000100
#define     ENC_DAC_STS_MONSTAT_A_SHIFT                                   8
#define     ENC_DAC_STS_MONSTAT_B_MASK                             0x00000200
#define     ENC_DAC_STS_MONSTAT_B_SHIFT                                   9
#define     ENC_DAC_STS_MONSTAT_C_MASK                             0x00000400
#define     ENC_DAC_STS_MONSTAT_C_SHIFT                                  10
#define     ENC_DAC_STS_MONSTAT_D_MASK                             0x00000800
#define     ENC_DAC_STS_MONSTAT_D_SHIFT                                  11
#define     ENC_DAC_STS_MONSTAT_E_MASK                             0x00001000
#define     ENC_DAC_STS_MONSTAT_E_SHIFT                                  12
#define     ENC_DAC_STS_MONSTAT_F_MASK                             0x00002000
#define     ENC_DAC_STS_MONSTAT_F_SHIFT                                  13
#define     ENC_DAC_STS_MACROVISION_DEV_MASK                       0x00010000
#define     ENC_DAC_STS_MACROVISION_DEV_SHIFT                            16

#define ENC_RESET_REG                                          (ENC_BASE + 0x04)
#define     ENC_RESET_REG_RESET_MASK                               0x00000001
#define     ENC_RESET_REG_RESET_SHIFT                                     0
#define     ENC_RESET_TIMING_RESET_MASK                            0x00000002
#define     ENC_RESET_TIMING_RESET_SHIFT                                  1

#define ENC_LINE_TIME_REG                                      (ENC_BASE + 0x10)
#define     ENC_LINE_TIME_HACTIVE_MASK                             0x000003ff
#define     ENC_LINE_TIME_HACTIVE_SHIFT                                   0
#define     ENC_LINE_TIME_HCLK_MASK                                0x0fff0000
#define     ENC_LINE_TIME_HCLK_SHIFT                                     16

#define ENC_HORIZONTAL_TIMING_REG                              (ENC_BASE + 0x14)
#define     ENC_HORZ_TIMING_HBURST_END_MASK                        0x000000ff
#define     ENC_HORZ_TIMING_HBURST_END_SHIFT                              0
#define     ENC_HORZ_TIMING_HBURST_BEG_MASK                        0x0000ff00
#define     ENC_HORZ_TIMING_HBURST_BEG_SHIFT                              8
#define     ENC_HORZ_TIMING_AHSYNCWIDTH_MASK                       0x00ff0000
#define     ENC_HORZ_TIMING_AHSYNCWIDTH_SHIFT                            16

#define ENC_HBLANK_REG                                         (ENC_BASE + 0x18)
#define     ENC_HBLANK_VACTIVE_MASK                                0x000001ff
#define     ENC_HBLANK_VACTIVE_SHIFT                                      0
#define     ENC_HBLANK_HBLANK_MASK                                 0x03ff0000
#define     ENC_HBLANK_HBLANK_SHIFT                                      16

#define ENC_HSYNC_REG                                          (ENC_BASE + 0x1C)
#define     ENC_HSYNC_HSYNCOFF_MASK                                0x000003ff
#define     ENC_HSYNC_HSYNCOFF_SHIFT                                      0
#define     ENC_HSYNC_HSYNCWIDTH_MASK                              0x00ff0000
#define     ENC_HSYNC_HSYNCWIDTH_SHIFT                                   16
#define     ENC_HSYNC_VBLANK_MASK                                  0xff000000
#define     ENC_HSYNC_VBLANK_SHIFT                                       24

#define ENC_CONTROL0_REG                                       (ENC_BASE + 0x20)
#define     ENC_CTL0_OUTMODE_MASK                                  0x00000007
#define     ENC_CTL0_OUTMODE_SHIFT                                        0
#define         ENC_CTL0_OUTMODE_Y_C_CVBS_Y_PR_PB                    (0UL<<0)
#define         ENC_CTL0_OUTMODE_Y_C_CVBS_R_G_B                      (1UL<<0)
#define         ENC_CTL0_OUTMODE_Y_C_CVBS_CVBSDLY_CVBS_CVBS          (2UL<<0)
#define         ENC_CTL0_OUTMODE_Y_C_Y_Y_C_C                         (3UL<<0)
#define         ENC_CTL0_OUTMODE_CVBS_CVBSDLY_CVBS_Y_PR_PB           (4UL<<0)
#define         ENC_CTL0_OUTMODE_Y_C_CVBS_B_G_R                      (5UL<<0)
#define         ENC_CTL0_OUTMODE_CVBS_CVBSDLY_CVBS_CVBCDLY_CVBS_CVBS (6UL<<0)
#define         ENC_CTL0_OUTMODE_Y_C_CVBS_C_Y_CVBS                   (7UL<<0)
#define     ENC_CTL0_CVBSD_INV_MASK                                0x00000008
#define     ENC_CTL0_CVBSD_INV_SHIFT                                      3
#define         ENC_CTL0_CVBSD_INV_NORMAL                            (0UL<<3)
#define         ENC_CTL0_CVBSD_INV_INVERT                            (1UL<<3)
#define     ENC_CTL0_EACTIVE_MASK                                  0x00000010
#define     ENC_CTL0_EACTIVE_SHIFT                                        4
#define         ENC_CTL0_EACTIVE_BLACK_BURST                         (0UL<<4)
#define         ENC_CTL0_EACTIVE_ENABLE_NORMAL                       (1UL<<4)
#define     ENC_CTL0_ECLIP_MASK                                    0x00000020
#define     ENC_CTL0_ECLIP_SHIFT                                          5
#define         ENC_CTL0_ECLIP_NORMAL                                (0UL<<5)
#define         ENC_CTL0_ECLIP_ENABLE                                (1UL<<5)
#define     ENC_CTL0_ECBAR_MASK                                    0x00000040
#define     ENC_CTL0_ECBAR_SHIFT                                          6
#define         ENC_CTL0_ECBAR_NORMAL                                (0UL<<6)
#define         ENC_CTL0_ECBAR_ENABLE                                (1UL<<6)
#define     ENC_CTL0_BLUEFIELD_MASK                                0x00000080
#define     ENC_CTL0_BLUEFIELD_SHIFT                                      7
#define         ENC_CTL0_BLUEFIELD_NORMAL                            (0UL<<7)
#define         ENC_CTL0_BLUEFIELD_ENABLE                            (1UL<<7)
#define     ENC_CTL0_PKFIL_SEL_MASK                                0x00000300
#define     ENC_CTL0_PKFIL_SEL_SHIFT                                      8
#define         ENC_CTL0_PKFIL_SEL_FILTER0                           (0UL<<8)
#define         ENC_CTL0_PKFIL_SEL_FILTER1                           (1UL<<8)
#define         ENC_CTL0_PKFIL_SEL_FILTER2                           (2UL<<8)
#define         ENC_CTL0_PKFIL_SEL_FILTER3                           (3UL<<8)
#define         ENC_CTL0_PKFIL_SEL_FILTER4                           (0UL<<8)
#define         ENC_CTL0_PKFIL_SEL_FILTER5                           (1UL<<8)
#define         ENC_CTL0_PKFIL_SEL_FILTER6                           (2UL<<8)
#define         ENC_CTL0_PKFIL_SEL_FILTER7                           (3UL<<8)
#define     ENC_CTL0_FIL_SEL_MASK                                  0x00000400
#define     ENC_CTL0_FIL_SEL_SHIFT                                       10
#define         ENC_CTL0_FIL_SEL_ENABLE_PEAK_FILTERS                (0UL<<10)
#define         ENC_CTL0_FIL_SEL_ENABLE_REDUCT_FILTERS              (1UL<<10)
#define     ENC_CTL0_CROSS_FIL_MASK                                0x00000800
#define     ENC_CTL0_CROSS_FIL_SHIFT                                     11
#define         ENC_CTL0_CROSS_FIL_APPLY                            (0UL<<11)
#define         ENC_CTL0_CROSS_FIL_BYPASS                           (1UL<<11)
#define     ENC_CTL0_CHROMA_BW_MASK                                0x00001000
#define     ENC_CTL0_CHROMA_BW_SHIFT                                     12
#define         ENC_CTL0_CHROMA_BW_NORMAL                           (0UL<<12)
#define         ENC_CTL0_CHROMA_BW_WIDE                             (1UL<<12)
#define     ENC_CTL0_CHROMA_DISABLE_MASK                           0x00002000
#define     ENC_CTL0_CHROMA_DISABLE_SHIFT                                13
#define         ENC_CTL0_CHROMA_NORMAL                              (0UL<<13)
#define         ENC_CTL0_CHROMA_DISABLE                             (1UL<<13)
#define     ENC_CTL0_SC_RESET_MASK                                 0x00004000
#define     ENC_CTL0_SC_RESET_SHIFT                                      14
#define         ENC_CTL0_SC_RESET_AT_BEG_COLOR_FIELD_SEQ            (0UL<<14)
#define         ENC_CTL0_SC_RESET_DISABLE                           (1UL<<14)
#define     ENC_CTL0_SCADJ_DISABLE_MASK                            0x00008000
#define     ENC_CTL0_SCADJ_DISABLE_SHIFT                                 15
#define     ENC_CTL0_ALL_DAC_MASK                                  0x000F0000
#define     ENC_CTL0_ALL_DAC_SHIFT                                       16
#define     ENC_CTL0_DAC_A_ENABLE_MASK                             0x00010000
#define     ENC_CTL0_DAC_A_ENABLE_SHIFT                                  16
#define         ENC_CTL0_DAC_A_ENABLE                               (1UL<<16)
#define     ENC_CTL0_DAC_B_ENABLE_MASK                             0x00020000
#define     ENC_CTL0_DAC_B_ENABLE_SHIFT                                  17
#define         ENC_CTL0_DAC_B_ENABLE                               (1UL<<17)
#define     ENC_CTL0_DAC_C_ENABLE_MASK                             0x00040000
#define     ENC_CTL0_DAC_C_ENABLE_SHIFT                                  18
#define         ENC_CTL0_DAC_C_ENABLE                               (1UL<<18)
#define     ENC_CTL0_DAC_D_ENABLE_MASK                             0x00080000
#define     ENC_CTL0_DAC_D_ENABLE_SHIFT                                  19
#define         ENC_CTL0_DAC_D_ENABLE                               (1UL<<19)
#define     ENC_CTL0_DAC_DISABLE_MASK                              0x00100000
#define     ENC_CTL0_DAC_DISABLE_SHIFT                                   20
#define         ENC_CTL0_DAC_DISABLE_ENABLE                         (1UL<<20)
#define     ENC_CTL0_CHECK_STATUS_MASK                             0x00400000
#define     ENC_CTL0_CHECK_STATUS_SHIFT                                  22
#define         ENC_CTL0_CHECK_STATUS                               (1UL<<22)
#define     ENC_CTL0_AUTO_CHECK_MASK                               0x00800000
#define     ENC_CTL0_AUTO_CHECK_SHIFT                                    23
#define         ENC_CTL0_AUTO_CHECK_DISABLE                         (0UL<<23)
#define         ENC_CTL0_AUTO_CHECK_ENABLE                          (1UL<<23)
#define     ENC_CTL0_FIELD_ID_MASK                                 0x01000000
#define     ENC_CTL0_FIELD_ID_SHIFT                                      24
#define         ENC_CTL0_FIELD_ID_DISABLE                           (0UL<<24)
#define         ENC_CTL0_FIELD_ID_ENABLE                            (1UL<<24)
#define     ENC_CTL0_SC_PATTERN_MASK                               0x02000000
#define     ENC_CTL0_SC_PATTERN_SHIFT                                    25
#define         ENC_CTL0_SC_PATTERN_0_0_180_0_0_180                 (0UL<<25)
#define         ENC_CTL0_SC_PATTERN_0_0_0_180_180_180               (1UL<<25)
#define     ENC_CTL0_SC_PROC_MASK                                  0x04000000
#define     ENC_CTL0_SC_PROC_SHIFT                                       26
#define         ENC_CTL0_SC_PROC_LINES_23_310_AND_336_623           (0UL<<26)
#define         ENC_CTL0_SC_PROC_LINES_DEF_BY_VBLANK_VACTIVE        (1UL<<26)
#define     ENC_CTL0_FM_MODULATION_MASK                            0x08000000
#define     ENC_CTL0_FM_MODULATION_SHIFT                                 27
#define         ENC_CTL0_FM_MODULATION_QAM                          (0UL<<27)
#define         ENC_CTL0_FM_MODULATION_FM                           (1UL<<27)
#define     ENC_CTL0_PHASE_ALTERATION_MASK                         0x10000000
#define     ENC_CTL0_PHASE_ALTERATION_SHIFT                              28
#define         ENC_CTL0_PHASE_ALTERATION_DISABLE                   (0UL<<28)
#define         ENC_CTL0_PHASE_ALTERATION_ENABLE                    (1UL<<28)
#define     ENC_CTL0_SETUP_MASK                                    0x20000000
#define     ENC_CTL0_SETUP_SHIFT                                         29
#define         ENC_CTL0_SETUP_DISABLE                              (0UL<<29)
#define         ENC_CTL0_SETUP_ENABLE                               (1UL<<29)
#define     ENC_CTL0_LINE625_MASK                                  0x40000000
#define     ENC_CTL0_LINE625_SHIFT                                       30
#define         ENC_CTL0_LINE625_525_FORMAT                         (0UL<<30)
#define         ENC_CTL0_LINE625_625_FORMAT                         (1UL<<30)
#define     ENC_CTL0_VSYNC_DURATION_MASK                           0x80000000
#define     ENC_CTL0_VSYNC_DURATION_SHIFT                                31
#define         ENC_CTL0_VSYNC_3_LINES                              (0UL<<31)
#define         ENC_CTL0_VSYNC_2_5_LINES                            (1UL<<31)

#define ENC_AMPLITUDE_REG                                      (ENC_BASE + 0x24)
#define     ENC_AMP_BURST_AMP_MASK                                 0x000000ff
#define     ENC_AMP_BURST_AMP_SHIFT                                       0
#define     ENC_AMP_SYNC_AMP_MASK                                  0x0000ff00
#define     ENC_AMP_SYNC_AMP_SHIFT                                        8
#define     ENC_AMP_SC_AMP_MASK                                    0x00ff0000
#define     ENC_AMP_SC_AMP_SHIFT                                         16

#define ENC_YUV_MULTIPLICATION_REG                             (ENC_BASE + 0x28)
#define     ENC_YUV_MULT_MCOMPU_MASK                               0x000000ff
#define     ENC_YUV_MULT_MCOMPU_SHIFT                                     0
#define     ENC_YUV_MULT_MCOMPV_MASK                               0x0000ff00
#define     ENC_YUV_MULT_MCOMPV_SHIFT                                     8
#define     ENC_YUV_MULT_MCOMPY_MASK                               0x00ff0000
#define     ENC_YUV_MULT_MCOMPY_SHIFT                                    16

#define ENC_YCRCB_MULTIPLICATION_REG                           (ENC_BASE + 0x2C)
#define     ENC_YCRCB_MULT_MCR_MASK                                0x000000ff
#define     ENC_YCRCB_MULT_MCR_SHIFT                                      0
#define     ENC_YCRCB_MULT_MCB_MASK                                0x0000ff00
#define     ENC_YCRCB_MULT_MCB_SHIFT                                      8
#define     ENC_YCRCB_MULT_MY_MASK                                 0x00ff0000
#define     ENC_YCRCB_MULT_MY_SHIFT                                      16
#define     ENC_YCRCB_MULT_YDELAY_MASK                             0x07000000
#define     ENC_YCRCB_MULT_YDELAY_SHIFT                                  24

#define ENC_SECAM_DR_REG                                       (ENC_BASE + 0x30)
#define     ENC_SECAM_DR_MASK                                      0xffffffff
#define     ENC_SECAM_DR_SHIFT                                            0

#define ENC_SECAM_DB_REG                                       (ENC_BASE + 0x34)
#define     ENC_SECAM_DB_MASK                                      0xffffffff
#define     ENC_SECAM_DB_SHIFT                                            0

#define ENC_DR_RANGE_REG                                       (ENC_BASE + 0x38)
#define     ENC_DR_RANGE_MIN_MASK                                  0x000003ff
#define     ENC_DR_RANGE_MIN_SHIFT                                        0
#define     ENC_DR_RANGE_MAX_MASK                                  0x03ff0000
#define     ENC_DR_RANGE_MAX_SHIFT                                       16

#define ENC_DB_RANGE_REG                                       (ENC_BASE + 0x3C)
#define     ENC_DB_RANGE_MIN_MASK                                  0x000003ff
#define     ENC_DB_RANGE_MIN_SHIFT                                        0
#define     ENC_DB_RANGE_MAX_MASK                                  0x03ff0000
#define     ENC_DB_RANGE_MAX_SHIFT                                       16

#define ENC_HUE_BRIGHT_CTL_REG                                 (ENC_BASE + 0x40)
#define     ENC_HUEBRICTL_HUEADJ_MASK                              0x000000ff
#define     ENC_HUEBRICTL_HUEADJ_SHIFT                                    0
#define     ENC_HUEBRICTL_PHASEOFF_MASK                            0x0000ff00
#define     ENC_HUEBRICTL_PHASEOFF_SHIFT                                  8
#define     ENC_HUEBRICTL_YOFF_MASK                                0x00ff0000
#define     ENC_HUEBRICTL_YOFF_SHIFT                                     16

#define ENC_CHROMA_CTL_REG                                     (ENC_BASE + 0x44)
#define     ENC_CHROMA_CTL_MULT_UU_MASK                            0x000000ff
#define     ENC_CHROMA_CTL_MULT_UU_SHIFT                                  0
#define     ENC_CHROMA_CTL_MULT_VU_MASK                            0x0000ff00
#define     ENC_CHROMA_CTL_MULT_VU_SHIFT                                  8
#define     ENC_CHROMA_CTL_MULT_UV_MASK                            0x00ff0000
#define     ENC_CHROMA_CTL_MULT_UV_SHIFT                                 16
#define     ENC_CHROMA_CTL_MULT_VV_MASK                            0xff000000
#define     ENC_CHROMA_CTL_MULT_VV_SHIFT                                 24

#define ENC_CC_CTL_REG                                         (ENC_BASE + 0x48)
#define     ENC_CC_CTL_CCSTART_MASK                                0x000001ff
#define     ENC_CC_CTL_CCSTART_SHIFT                                      0
#define     ENC_CC_CTL_CCADD_MASK                                  0x0fff0000
#define     ENC_CC_CTL_CCADD_SHIFT                                       16

#define ENC_XDS_DATA_REG                                       (ENC_BASE + 0x4C)
#define     ENC_XDS_DATA_XDSB1_MASK                                0x000000ff
#define     ENC_XDS_DATA_XDSB1_SHIFT                                      0
#define     ENC_XDS_DATA_XDSB2_MASK                                0x0000ff00
#define     ENC_XDS_DATA_XDSB2_SHIFT                                      8

#define ENC_CC_DATA_REG                                        (ENC_BASE + 0x50)
#define     ENC_CC_DATA_CCB1_MASK                                  0x000000ff
#define     ENC_CC_DATA_CCB1_SHIFT                                        0
#define     ENC_CC_DATA_CCB2_MASK                                  0x0000ff00
#define     ENC_CC_DATA_CCB2_SHIFT                                        8

#define ENC_XDS_CC_CTL_REG                                     (ENC_BASE + 0x54)
#define     ENC_XDS_CC_CTL_EWSSF1_MASK                             0x00000001
#define     ENC_XDS_CC_CTL_EWSSF1_SHIFT                                   0
#define     ENC_XDS_CC_CTL_EWSSF2_MASK                             0x00000002
#define     ENC_XDS_CC_CTL_EWSSF2_SHIFT                                   1
#define     ENC_XDS_CC_CTL_CCSEL_MASK                              0x000f0000
#define     ENC_XDS_CC_CTL_CCSEL_SHIFT                                   16
#define     ENC_XDS_CC_CTL_XDSSEL_MASK                             0x00f00000
#define     ENC_XDS_CC_CTL_XDSSEL_SHIFT                                  20
#define     ENC_XDS_CC_CTL_ECCGATE_MASK                            0x01000000
#define     ENC_XDS_CC_CTL_ECCGATE_SHIFT                                 24
#define     ENC_XDS_CC_CTL_ECC_MASK                                0x02000000
#define     ENC_XDS_CC_CTL_ECC_SHIFT                                     25
#define     ENC_XDS_CC_CTL_EXDS_MASK                               0x04000000
#define     ENC_XDS_CC_CTL_EXDS_SHIFT                                    26

#define ENC_WS_DATA_REG                                        (ENC_BASE + 0x58)
#define     ENC_WS_DATA_WSDAT_MASK                                 0x000fffff
#define     ENC_WS_DATA_WSDAT_SHIFT                                       0

#define ENC_TTX_REQ_REG                                        (ENC_BASE + 0x5C)
#define     ENC_TTX_REQ_CTL_TTXHS_MASK                             0x000007ff
#define     ENC_TTX_REQ_CTL_TTXHS_SHIFT                                   0
#define     ENC_TTX_REQ_CTL_TTXHE_MASK                             0x07ff0000
#define     ENC_TTX_REQ_CTL_TTXHE_SHIFT                                  16

#define ENC_TTX_F1_CTL_REG                                     (ENC_BASE + 0x60)
#define     ENC_TTXF1_CTL_TTXBF1_MASK                              0x000001ff
#define     ENC_TTXF1_CTL_TTXBF1_SHIFT                                    0
#define     ENC_TTXF1_CTL_TTXEF1_MASK                              0x01ff0000
#define     ENC_TTXF1_CTL_TTXEF1_SHIFT                                   16

#define ENC_TTX_F2_CTL_REG                                     (ENC_BASE + 0x64)
#define     ENC_TTXF2_CTL_TTXBF2_MASK                              0x000001ff
#define     ENC_TTXF2_CTL_TTXBF2_SHIFT                                    0
#define     ENC_TTXF2_CTL_TTXEF2_MASK                              0x01ff0000
#define     ENC_TTXF2_CTL_TTXEF2_SHIFT                                   16

#define ENC_TTX_LINE_CTL_REG                                   (ENC_BASE + 0x68)
#define     ENC_TTXLINE_CTL_TTX_DIS_MASK                           0x0000ffff
#define     ENC_TTXLINE_CTL_TTX_DIS_SHIFT                                 0
#define     ENC_TTXLINE_CTL_TXE_MASK                               0x40000000
#define     ENC_TTXLINE_CTL_TXE_SHIFT                                    30
#define     ENC_TTXLINE_CTL_TXRM_MASK                              0x80000000
#define     ENC_TTXLINE_CTL_TXRM_SHIFT                                   31

#ifdef BITFIELDS
typedef struct _ENC_DAC_STATUS {
   BIT_FLD  Field_Cnt:4,
            CC_Stat:1,
            XDS_Stat:1,
            rsvd1:2,
            MONSTAT_A:1,
            MONSTAT_B:1,
            MONSTAT_C:1,
            MONSTAT_D:1,
            MONSTAT_E:1,
            MONSTAT_F:1,
            rsvd2:2,
            Macrovision_dev:1,
            rsvd3:15;
} ENC_DAC_STATUS;
typedef ENC_DAC_STATUS volatile *LPENC_DAC_STATUS;

typedef struct _ENC_RESET {
   BIT_FLD  RegReset:1,
            TimingReset:1,
            rsvd1:30;
} ENC_RESET;
typedef ENC_RESET volatile *LPENC_RESET;

typedef struct _ENC_LINE_TIME {
    BIT_FLD  HActive:10,
             rsvd1:6,
             HClk:12,
             rsvd2:4;
} ENC_LINE_TIME;
typedef ENC_LINE_TIME volatile *LPENC_LINE_TIME;

typedef struct _ENC_HORZ_TIMING {
    BIT_FLD  HBurstEnd:8,
             HBurstBeg:8,
             AHSyncWidth:8,
             rsvd1:8;
} ENC_HORZ_TIMING;
typedef ENC_HORZ_TIMING volatile *LPENC_HORZ_TIMING;

typedef struct _ENC_HBLANK {
    BIT_FLD  VActive:9,
             rsvd1:7,
             HBlank:10,
             rsvd2:6;
} ENC_HBLANK;
typedef ENC_HBLANK volatile *LPENC_HBLANK;

typedef struct _ENC_HSYNC {
    BIT_FLD  HSyncOff:10,
             rsvd1:6,
             HSyncWidth:8,
             VBlank:8;
} ENC_HSYNC;
typedef ENC_HSYNC volatile *LPENC_HSYNC;

typedef struct _ENC_CONTROL0 {
    BIT_FLD  OutMode:3,
             CVBSD_Inv:1,
             EActive:1,
             EClip:1,
             ECBar:1,
             BlueField:1,
             PkFil_Sel:2,
             Fil_Sel:1,
             Cross_Fil:1,
             Chroma_BW:1,
             Chroma_Disable:1,
             SC_Reset:1,
             SCAdj_Disable:1,
             DAC_A_Enable:1,
             DAC_B_Enable:1,
             DAC_C_Enable:1,
             DAC_D_Enable:1,
             DAC_E_Enable:1,
             DAC_F_Enable:1,
             Check_Status:1,
             Auto_Check:1,
             Field_ID:1,
             SC_Pattern:1,
             SC_Proc:1,
             FM_Modulation:1,
             Phase_Alteration:1,
             Setup:1,
             Line625:1,
             VSync_Duration:1;
} ENC_CONTROL0;
typedef struct ENC_CONTROL0 volatile *LPENC_CONTROL0;

typedef struct _ENC_AMPLITUDE {
    BIT_FLD  Burst_Amplitude:8,
             Sync_Amplitude:8,
             SC_Amplitude:8,
             rsvd1:7,
             Field:1;
} ENC_AMPLITUDE;
typedef ENC_AMPLITUDE volatile *LPENC_AMPLITUDE;

typedef struct _ENC_YUV_MULT {
    BIT_FLD  MCOMP_U:8,
             MCOMP_V:8,
             MCOMP_Y:8,
             rsvd1:8;
} ENC_YUV_MULT;
typedef ENC_YUV_MULT volatile *LPENC_YUV_MULT;
#endif /* BITFIELDS */

#endif /* INCL_ENC */


/***************/
/* Movie Input */
/***************/

/* Last updated 3/17/03 */

#ifdef INCL_MOV

/*
******************************************************************************
 *
 *     MOV_VS_ACT_LINE_RANGE_REG                          (MOV_BASE + 0x00000)
 *     MOV_VS_VBI_LINE_RANGE_REG                          (MOV_BASE + 0x00004)
 *     MOV_VS_MAX_PIXEL_REG                               (MOV_BASE + 0x00008)
 *     MOV_VS_CONTROL_REG                                 (MOV_BASE + 0x0000C)
 *     MOV_VS_GL_IRQ_CONTROL_REG                          (MOV_BASE + 0x00010)
 *     MOV_VS_GL_IRQ_STATUS_REG                           (MOV_BASE + 0x00014)
 *     MOV_VS_LAG_REG                                     (MOV_BASE + 0x00018)
 *     MOV_VS_PREV_LAG_REG                                (MOV_BASE + 0x0001C)
 *     MOV_VS_EXP_LINES_REG                               (MOV_BASE + 0x00020)
 *     MOV_VS_PREV_LINE_COUNT_REG                         (MOV_BASE + 0x00024)
 *     MOV_VS_STATUS_REG                                  (MOV_BASE + 0x00030)
 *     MOV_VS_DROPPED_LINE_COUNT_REG                      (MOV_BASE + 0x00034)
 *     MOV_SS_ACT_EVEN_REG                                (MOV_BASE + 0x10000)
 *     MOV_SS_ACT_ODD_REG                                 (MOV_BASE + 0x10004)
 *     MOV_SS_ACT_THIRD_REG                               (MOV_BASE + 0x10008)
 *     MOV_SS_VBI_EVEN_REG                                (MOV_BASE + 0x1000C)
 *     MOV_SS_VBI_ODD_REG                                 (MOV_BASE + 0x10010)
 *     MOV_SS_STRIDE_REG                                  (MOV_BASE + 0x10014)
 *     MOV_SS_CONTROL_REG                                 (MOV_BASE + 0x10018)
 *     MOV_SS_ENABLE_REG                                  (MOV_BASE + 0x1001C)
 *     MOV_SS_TIMER_REG                                   (MOV_BASE + 0x10020)
 *     MOV_SS_STATUS_REG                                  (MOV_BASE + 0x10030)
 *     MOV_SS_TIMER_STATUS_REG                            (MOV_BASE + 0x10034)
 *     MOV_SS_CURRENT_ADDR_REG                            (MOV_BASE + 0x10038)
 *     MOV_SS_LINE_COUNT_REG                              (MOV_BASE + 0x1003C)
 *
 *****************************************************************************
 */

#define MOV_VS_ACT_LINE_RANGE_REG                         (MOV_BASE + 0x00000)
#define     MOV_VS_FIRST_ACT_LINE_MASK                            0x000007FF
#define     MOV_VS_FIRST_ACT_LINE_SHIFT                                  0
#define     MOV_VS_LAST_ACT_LINE_MASK                             0x07FF0000
#define     MOV_VS_LAST_ACT_LINE_SHIFT                                  16
#define MOV_VS_VBI_LINE_RANGE_REG                         (MOV_BASE + 0x00004)
#define     MOV_VS_FIRST_VBI_LINE_MASK                            0x000007FF
#define     MOV_VS_FIRST_VBI_LINE_SHIFT                                  0
#define     MOV_VS_LAST_VBI_LINE_MASK                             0x07FF0000
#define     MOV_VS_LAST_VBI_LINE_SHIFT                                  16
#define MOV_VS_MAX_PIXEL_REG                              (MOV_BASE + 0x00008)
#define     MOV_VS_LAST_ACT_PIXEL_MASK                            0x000007FF
#define     MOV_VS_LAST_ACT_PIXEL_SHIFT                                  0
#define     MOV_VS_LAST_VBI_PIXEL_MASK                            0x07FF0000
#define     MOV_VS_LAST_VBI_PIXEL_SHIFT                                 16
#define MOV_VS_CONTROL_REG                                (MOV_BASE + 0x0000C)
#define     MOV_VS_ENABLE_MASK                                    0x00000001
#define     MOV_VS_ENABLE_SHIFT                                          0
#define     MOV_VS_SEL_ITU_MASK                                   0x00000002
#define     MOV_VS_SEL_ITU_SHIFT                                         1
#define     MOV_VS_ACT_ENABLE_MASK                                0x00000004
#define     MOV_VS_ACT_ENABLE_SHIFT                                      2
#define     MOV_VS_VBI_ENABLE_MASK                                0x00000008
#define     MOV_VS_VBI_ENABLE_SHIFT                                      3
#define     MOV_VS_ITU_STORE_ANC_HEADER_MASK                      0x00000040
#define     MOV_VS_ITU_STORE_ANC_HEADER_SHIFT                            6
#define     MOV_VS_ITU_ANC_VBI_ENABLE_MASK                        0x00000080
#define     MOV_VS_ITU_ANC_VBI_ENABLE_SHIFT                              7
#define     MOV_VS_ITU_RAW_VBI_ENABLE_MASK                        0x00000100
#define     MOV_VS_ITU_RAW_VBI_ENABLE_SHIFT                              8
#define     MOV_VS_ACT_BIT_SWAP_MASK                              0x00000200
#define     MOV_VS_ACT_BIT_SWAP_SHIFT                                    9
#define     MOV_VS_VBI_BIT_SWAP_MASK                              0x00000400
#define     MOV_VS_VBI_BIT_SWAP_SHIFT                                   10
#define MOV_VS_GL_IRQ_CONTROL_REG                         (MOV_BASE + 0x10)
#define     MOV_VS_GL_IRQ_CONTROL_LAG_DEC_EVEN_MASK               0x00000001
#define     MOV_VS_GL_IRQ_CONTROL_LAG_DEC_EVEN_SHIFT                     0 
#define     MOV_VS_GL_IRQ_CONTROL_LAG_DEC_ODD_MASK                0x00000002
#define     MOV_VS_GL_IRQ_CONTROL_LAG_DEC_ODD_SHIFT                      1 
#define     MOV_VS_GL_IRQ_CONTROL_ENABLE_MASK                     0x00000ffc
#define     MOV_VS_GL_IRQ_CONTROL_ENABLE_SHIFT                           2 
#define     MOV_VS_GL_IRQ_CONTROL_ENABLE_LAG_LT_MIN_MASK          0x00000004
#define     MOV_VS_GL_IRQ_CONTROL_ENABLE_LAG_LT_MIN_SHIFT                2 
#define     MOV_VS_GL_IRQ_CONTROL_ENABLE_LAG_GT_MAX_MASK          0x00000008
#define     MOV_VS_GL_IRQ_CONTROL_ENABLE_LAG_GT_MAX_SHIFT                3 
#define     MOV_VS_GL_IRQ_CONTROL_ENABLE_LAG_EQ_PM1_MASK          0x00000010
#define     MOV_VS_GL_IRQ_CONTROL_ENABLE_LAG_EQ_PM1_SHIFT                4 
#define     MOV_VS_GL_IRQ_CONTROL_ENABLE_LAG_EQ_PP1_MASK          0x00000020
#define     MOV_VS_GL_IRQ_CONTROL_ENABLE_LAG_EQ_PP1_SHIFT                5 
#define     MOV_VS_GL_IRQ_CONTROL_ENABLE_LAG_LT_PM1_MASK          0x00000040
#define     MOV_VS_GL_IRQ_CONTROL_ENABLE_LAG_LT_PM1_SHIFT                6 
#define     MOV_VS_GL_IRQ_CONTROL_ENABLE_LAG_GT_PP1_MASK          0x00000080
#define     MOV_VS_GL_IRQ_CONTROL_ENABLE_LAG_GT_PP1_SHIFT                7 
#define     MOV_VS_GL_IRQ_CONTROL_ENABLE_EVEN_LC_NEQ_MASK         0x00000100
#define     MOV_VS_GL_IRQ_CONTROL_ENABLE_EVEN_LC_NEQ_SHIFT               8 
#define     MOV_VS_GL_IRQ_CONTROL_ENABLE_ODD_LC_NEQ_MASK          0x00000200
#define     MOV_VS_GL_IRQ_CONTROL_ENABLE_ODD_LC_NEQ_SHIFT                9 
#define     MOV_VS_GL_IRQ_CONTROL_ENABLE_EVEN_LC_EQ_MASK          0x00000400
#define     MOV_VS_GL_IRQ_CONTROL_ENABLE_EVEN_LC_EQ_SHIFT               10 
#define     MOV_VS_GL_IRQ_CONTROL_ENABLE_ODD_LC_EQ_MASK           0x00000800
#define     MOV_VS_GL_IRQ_CONTROL_ENABLE_ODD_LC_EQ_SHIFT                11 
#define MOV_VS_GL_IRQ_STATUS_REG                          (MOV_BASE + 0x14)
#define     MOV_VS_GL_IRQ_STATUS_LAG_LT_MIN_MASK                  0x00000004
#define     MOV_VS_GL_IRQ_STATUS_LAG_LT_MIN_SHIFT                        2 
#define     MOV_VS_GL_IRQ_STATUS_LAG_GT_MAX_MASK                  0x00000008
#define     MOV_VS_GL_IRQ_STATUS_LAG_GT_MAX_SHIFT                        3 
#define     MOV_VS_GL_IRQ_STATUS_LAG_EQ_PM1_MASK                  0x00000010
#define     MOV_VS_GL_IRQ_STATUS_LAG_EQ_PM1_SHIFT                        4 
#define     MOV_VS_GL_IRQ_STATUS_LAG_EQ_PP1_MASK                  0x00000020
#define     MOV_VS_GL_IRQ_STATUS_LAG_EQ_PP1_SHIFT                        5 
#define     MOV_VS_GL_IRQ_STATUS_LAG_LT_PM1_MASK                  0x00000040
#define     MOV_VS_GL_IRQ_STATUS_LAG_LT_PM1_SHIFT                        6 
#define     MOV_VS_GL_IRQ_STATUS_LAG_GT_PP1_MASK                  0x00000080
#define     MOV_VS_GL_IRQ_STATUS_LAG_GT_PP1_SHIFT                        7 
#define     MOV_VS_GL_IRQ_STATUS_EVEN_LC_NEQ_MASK                 0x00000100
#define     MOV_VS_GL_IRQ_STATUS_EVEN_LC_NEQ_SHIFT                       8 
#define     MOV_VS_GL_IRQ_STATUS_ODD_LC_NEQ_MASK                  0x00000200
#define     MOV_VS_GL_IRQ_STATUS_ODD_LC_NEQ_SHIFT                        9 
#define     MOV_VS_GL_IRQ_STATUS_EVEN_LC_EQ_MASK                  0x00000400
#define     MOV_VS_GL_IRQ_STATUS_EVEN_LC_EQ_SHIFT                       10 
#define     MOV_VS_GL_IRQ_STATUS_ODD_LC_EQ_MASK                   0x00000800
#define     MOV_VS_GL_IRQ_STATUS_ODD_LC_EQ_SHIFT                        11 
#define     MOV_VS_GL_IRQ_STATUS_LINE_COUNT_MASK                  0x07FF0000
#define     MOV_VS_GL_IRQ_STATUS_LINE_COUNT_SHIFT                       16 
#define MOV_VS_LAG_REG                                    (MOV_BASE + 0x18)
#define     MOV_VS_LAG_MIN_LAG_MASK                               0x000007FF
#define     MOV_VS_LAG_MIN_LAG_SHIFT                                     0
#define     MOV_VS_LAG_MAX_LAG_MASK                               0x07FF0000
#define     MOV_VS_LAG_MAX_LAG_SHIFT                                    16
#define MOV_VS_PREV_LAG_REG                               (MOV_BASE + 0x1C)
#define     MOV_VS_PREV_LAG_MASK                                  0x000007FF
#define     MOV_VS_PREV_LAG_SHIFT                                        0
#define MOV_VS_EXP_LINES_REG                                (MOV_BASE + 0x20)
#define     MOV_VS_EXP_LINES_EVEN_MASK                            0x000007FF
#define     MOV_VS_EXP_LINES_EVEN_SHIFT                                  0
#define     MOV_VS_EXP_LINES_ODD_MASK                             0x07FF0000
#define     MOV_VS_EXP_LINES_ODD_SHIFT                                  16
#define MOV_VS_PREV_LINE_COUNT_REG                        (MOV_BASE + 0x24)
#define     MOV_VS_PREV_EVEN_LINE_COUNT_MASK                      0x000007FF
#define     MOV_VS_PREV_EVEN_LINE_COUNT_SHIFT                            0
#define     MOV_VS_PREV_ODD_LINE_COUNT_MASK                       0x07FF0000
#define     MOV_VS_PREV_ODD_LINE_COUNT_SHIFT                            16
#define MOV_VS_STATUS_REG                                 (MOV_BASE + 0x00030)
#define     MOV_VS_STATUS_LINE_COUNT_MASK                         0x000007FF
#define     MOV_VS_STATUS_LINE_COUNT_SHIFT                               0
#define     MOV_VS_STATUS_PIXEL_COUNT_MASK                        0x07FF0000
#define     MOV_VS_STATUS_PIXEL_COUNT_SHIFT                             16
#define     MOV_VS_STATUS_ACTIVE_LINE_MASK                        0x10000000
#define     MOV_VS_STATUS_ACTIVE_LINE_SHIFT                             28
#define     MOV_VS_STATUS_EVEN_FIELD_MASK                         0x20000000
#define     MOV_VS_STATUS_EVEN_FIELD_SHIFT                              29
#define MOV_VS_DROPPED_LINE_COUNT_REG                     (MOV_BASE + 0x00034)
#define     MOV_VS_DROPPED_PIXEL_COUNT_MASK                       0x000FFFFF
#define     MOV_VS_DROPPED_PIXEL_COUNT_SHIFT                             0
#define     MOV_VS_RESET_DROPPED_LINE_COUNT_MASK                  0x80000000
#define     MOV_VS_RESET_DROPPED_LINE_COUNT_SHIFT                       31
#define MOV_SS_ACT_EVEN_REG                               (MOV_BASE + 0x10000)
#define MOV_SS_ACT_ODD_REG                                (MOV_BASE + 0x10004)
#define MOV_SS_ACT_THIRD_REG                              (MOV_BASE + 0x10008)
#define MOV_SS_VBI_EVEN_REG                               (MOV_BASE + 0x1000C)
#define MOV_SS_VBI_ODD_REG                                (MOV_BASE + 0x10010)
#define     MOV_ADDRESS_MASK                                      0x0FFFFFFF
#define     MOV_ADDRESS_SHIFT                                            0
#define MOV_SS_STRIDE_REG                                 (MOV_BASE + 0x10014)
#define     MOV_SS_STRIDE_VBI_MASK                                0x00000FFF
#define     MOV_SS_STRIDE_VBI_SHIFT                                      0
#define     MOV_SS_STRIDE_ACTIVE_MASK                             0x0FFF0000
#define     MOV_SS_STRIDE_ACTIVE_SHIFT                                  16
#define MOV_SS_CONTROL_REG                                (MOV_BASE + 0x10018)
#define     MOV_SS_CONTROL_QWD_NORM_MASK                          0x0000000F
#define     MOV_SS_CONTROL_QWD_NORM_SHIFT                                0
#define     MOV_SS_CONTROL_QWD_CRIT_MASK                          0x000000F0
#define     MOV_SS_CONTROL_QWD_CRIT_SHIFT                                4
#define     MOV_SS_CONTROL_REQ_TIMER_ENABLE_MASK                  0x00000100
#define     MOV_SS_CONTROL_REQ_TIMER_ENABLE_SHIFT                        8
#define     MOV_SS_CONTROL_CRIT_TIMER_ENABLE_MASK                 0x00000200
#define     MOV_SS_CONTROL_CRIT_TIMER_ENABLE_SHIFT                       9
#define     MOV_SS_CONTROL_TRIPLE_BUFFER_MASK                     0x00000400
#define     MOV_SS_CONTROL_TRIPLE_BUFFER_SHIFT                          10
#define     MOV_SS_CONTROL_ACT_INT_ENABLE_MASK                    0x00000800
#define     MOV_SS_CONTROL_ACT_INT_ENABLE_SHIFT                         11
#define     MOV_SS_CONTROL_VBI_INT_ENABLE_MASK                    0x00001000
#define     MOV_SS_CONTROL_VBI_INT_ENABLE_SHIFT                         12
#define     MOV_SS_CONTROL_QWD_CRIT_REQ_MSB_MASK                  0x00008000
#define     MOV_SS_CONTROL_QWD_CRIT_REQ_MSB_SHIFT                       15
#define     MOV_SS_CONTROL_LOCK_BUFFER_LINE_MASK                  0x07FF0000
#define     MOV_SS_CONTROL_LOCK_BUFFER_LINE_SHIFT                       16
#define     MOV_SS_CONTROL_ENABLE_DECODER_CLK_STOPPED_INT_MASK    0x80000000
#define     MOV_SS_CONTROL_ENABLE_DECODER_CLK_STOPPED_INT_SHIFT         31
#define MOV_SS_ENABLE_REG                                 (MOV_BASE + 0x1001C)
#define     MOV_SS_ENABLE_ASB_MASK                                0x00000001
#define     MOV_SS_ENABLE_ASB_SHIFT                                      0
#define MOV_SS_TIMER_REG                                  (MOV_BASE + 0x10020)
#define     MOV_SS_TIMER_REQ_MASK                                 0x00003FFF
#define     MOV_SS_TIMER_REQ_SHIFT                                       0
#define     MOV_SS_TIMER_CRIT_MASK                                0x3FFF0000
#define     MOV_SS_TIMER_CRIT_SHIFT                                     16
#define MOV_SS_STATUS_REG                                 (MOV_BASE + 0x10030)
#define     MOV_SS_STATUS_EVID_CLK_READY_MASK                     0x00000001
#define     MOV_SS_STATUS_EVID_CLK_READY_SHIFT                           0
#define     MOV_SS_STATUS_ACT_INT_MASK                            0x00000002
#define     MOV_SS_STATUS_ACT_INT_SHIFT                                  1
#define     MOV_SS_STATUS_VBI_INT_MASK                            0x00000004
#define     MOV_SS_STATUS_VBI_INT_SHIFT                                  2
#define     MOV_SS_STATUS_CLOCK_MASK                              0x00000008
#define     MOV_SS_STATUS_CLOCK_SHIFT                                    3
#define     MOV_SS_STATUS_BFR_LOCKED_MASK                         0x00000070
#define     MOV_SS_STATUS_BFR_LOCKED_SHIFT                               4
#define     MOV_SS_STATUS_BFR_CHANGED_MASK                        0x00000080
#define     MOV_SS_STATUS_BFR_CHANGED_SHIFT                              7
#define     MOV_SS_STATUS_BFR_EVEN_MASK                           0x00000700
#define     MOV_SS_STATUS_BFR_EVEN_SHIFT                                 8
#define     MOV_SS_STATUS_FIFO_DEPTH_MASK                         0x0000f800
#define     MOV_SS_STATUS_FIFO_DEPTH_SHIFT                              11
#define     MOV_SS_STATUS_DECODER_CLK_STOPPED_INT_MASK            0x80000000
#define     MOV_SS_STATUS_DECODER_CLK_STOPPED_INT_SHIFT               31
#define MOV_SS_TIMER_STATUS_REG                           (MOV_BASE + 0x10034)
#define     MOV_SS_TIMER_REG_MASK                                 0x00003FFF
#define     MOV_SS_TIMER_REG_SHIFT                                       0
#define     MOV_SS_TIMER_CRIT_MASK                                0x3FFF0000
#define     MOV_SS_TIMER_CRIT_SHIFT                                     16
#define MOV_SS_CURRENT_ADDR_REG                           (MOV_BASE + 0x10038)
#define MOV_SS_LINE_COUNT_REG                             (MOV_BASE + 0x1003C)
#define     MOV_SS_LINE_COUNT_MASK                                0x000007FF
#define     MOV_SS_LINE_COUNT_SHIFT                                   0

/* Bit fields defintions allowing register to be accessed as a whole */
#define MOV_VBIENABLE 0x00000008
#define MOV_ACTENABLE 0x00000004
#define MOV_SELITU    0x00000002
#define MOV_VSENABLE  0x00000001
/* End of bit fields */


#ifdef BITFIELDS
typedef struct _MOV_PIXEL {
   BIT_FLD  Cb:8,
            Y0:8,
            Cr:8,
            Y1:8;
} MOV_PIXEL;
typedef MOV_PIXEL volatile *LPMOV_PIXEL;

typedef struct _MOV_VS_ACT_LINE_RANGE {
   BIT_FLD  FirstActLine:11,
            Reserved1:5,
            LastActLine:11,
            Reserved2:5;
} MOV_VS_ACT_LINE_RANGE;
typedef MOV_VS_ACT_LINE_RANGE volatile *LPMOV_VS_ACT_LINE_RANGE;

typedef struct _MOV_VS_VBI_LINE_RANGE {
   BIT_FLD  FirstVBILine:11,
            Reserved1:5,
            LastVBILine:11,
            Reserved2:5;
} MOV_VS_VBI_LINE_RANGE;
typedef MOV_VS_VBI_LINE_RANGE volatile *LPMOV_VS_VBI_LINE_RANGE;

typedef struct _MOV_VS_MAX_PIXEL {
   BIT_FLD  LastActPixel:11,
            Reserved1:5,
            LastVBIPixel:11,
            Reserved2:5;
} MOV_VS_MAX_PIXEL;
typedef MOV_VS_MAX_PIXEL volatile *LPMOV_VS_MAX_PIXEL;

typedef struct _MOV_VS_CONTROL {
   BIT_FLD  VSEnable:1,
            SelITU:1,
            ActEnable:1,
            VBIEnable:1,
            Reserved1:2,
            ITUStoreAncHeader:1,
            ITUAncVbiEnable:1,
            ITURawVbiEnable:1,
            ActBitSwap:1,
            VbiBitSwap:1,
            Reserved2:21;
} MOV_VS_CONTROL;
typedef MOV_VS_CONTROL volatile *LPMOV_VS_CONTROL;

typedef struct _MOV_VS_STATUS {
   BIT_FLD  StatusLineCount:11,
            Reserved1:5,
            StatusPixelCount:11,
            Reserved2:1,
            StatusActiveLine:1,
            StatusEvenField:1,
            Reserved3:2;
} MOV_VS_STATUS;
typedef MOV_VS_STATUS volatile *LPMOV_VS_STATUS;

typedef struct _MOV_VS_DROPPED_LINE_COUNT {
   BIT_FLD  DroppedPixelCount:20,
            Reserved:11,
            ResetDroppedLineCount:1;
} MOV_VS_DROPPED_LINE_COUNT;
typedef MOV_VS_DROPPED_LINE_COUNT volatile *LPMOV_VS_DROPPED_LINE_COUNT;

typedef struct _MOV_ADDRESS {
   BIT_FLD  Address:28,
            Reserved:4;
} MOV_ADDRESS;
typedef MOV_ADDRESS volatile *LPMOV_ADDRESS;

typedef MOV_ADDRESS   MOV_SS_ACT_EVEN;
typedef MOV_ADDRESS   MOV_SS_ACT_ODD;
typedef MOV_ADDRESS   MOV_SS_ACT_THIRD;
typedef MOV_ADDRESS   MOV_SS_VBI_EVEN;
typedef MOV_ADDRESS   MOV_SS_VBI_ODD;
typedef LPMOV_ADDRESS LPMOV_SS_ACT_EVEN;
typedef LPMOV_ADDRESS LPMOV_SS_ACT_ODD;
typedef LPMOV_ADDRESS LPMOV_SS_ACT_THIRD;
typedef LPMOV_ADDRESS LPMOV_SS_VBI_EVEN;
typedef LPMOV_ADDRESS LPMOV_SS_VBI_ODD;

typedef struct _MOV_SS_STRIDE {
   BIT_FLD  VBIStride:12,
            Reserved1:4,
            ActiveStride:12,
            Reserved2:4;
} MOV_SS_STRIDE;
typedef MOV_SS_STRIDE volatile *LPMOV_SS_STRIDE;

typedef struct _MOV_SS_CONTROL {
   BIT_FLD  QwdNorm:4,
            QwdCrit:4,
            ReqTimerEnable:1,
            CritTimerEnable:1,
            TripleBuffer:1,
            ActIntEnable:1,
            VBIIntEnable:1,
            Reserved2:3,
            LockBufferLine:11,
            Reserved3:5;
} MOV_SS_CONTROL;
typedef MOV_SS_CONTROL volatile *LPMOV_SS_CONTROL;

typedef struct _MOV_SS_ENABLE {
   BIT_FLD  AsbEnable:1,
            Reserved1:31;
} MOV_SS_ENABLE;
typedef MOV_SS_ENABLE volatile *LPMOV_SS_ENABLE;

typedef struct _MOV_SS_TIMER {
   BIT_FLD  ReqTimer:14,
            Reserved1:2,
            CritTimer:14,
            Reserved2:2;
} MOV_SS_TIMER;
typedef MOV_SS_TIMER volatile *LPMOV_SS_TIMER;

typedef struct _MOV_SS_STATUS {
   BIT_FLD EvidClkReady:1,
            ActInt:1,
            VBIInt:1,
            Clock:1,
            BfrLocked:3,
            BfrChanged:1,
            BfrEven:3,
            FifoDepth:5,
            Reserved1:16;
} MOV_SS_STATUS;
typedef MOV_SS_STATUS volatile *LPMOV_SS_STATUS;

typedef struct _MOV_SS_TIMER_STATUS {
   BIT_FLD  RegTimerStatus:14,
            Reserved1:2,
            CritTimerStatus:14,
            Reserved2:2;
} MOV_SS_TIMER_STATUS;
typedef MOV_SS_TIMER_STATUS volatile *LPMOV_SS_TIMER_STATUS;
#endif /* BITFIELDS */

#endif /* INCL_MOV */

/*********/
/* Audio */
/*********/
/* Last updated 2002/05/15 */

#ifdef INCL_AUD
/*
 *******************************************************************************
 *      AUD_CHANNEL_CONTROL_REG                                (AUD_BASE + 0x00)
 *      AUD_PLAY_A_START_REG                                   (AUD_BASE + 0x04)
 *      AUD_PLAY_A_STOP_REG                                    (AUD_BASE + 0x08)
 *      AUD_PLAY_B_START_REG                                   (AUD_BASE + 0x0C)
 *      AUD_PLAY_B_STOP_REG                                    (AUD_BASE + 0x10)
 *      AUD_PLAY_STATUS_REG                                    (AUD_BASE + 0x14)
 *      AUD_CAPT_A_START_REG                                   (AUD_BASE + 0x1C)
 *      AUD_CAPT_A_STOP_REG                                    (AUD_BASE + 0x20)
 *      AUD_CAPT_B_START_REG                                   (AUD_BASE + 0x24)
 *      AUD_CAPT_B_STOP_REG                                    (AUD_BASE + 0x28)
 *      AUD_CAPT_STATUS_REG                                    (AUD_BASE + 0x2C)
 *      AUD_PLAY_INT_ADDR_REG                                  (AUD_BASE + 0x4C)
 *      AUD_CAPT_INT_ADDR_REG                                  (AUD_BASE + 0x50)
 *      AUD_PLAY_CONTROL_REG                                   (AUD_BASE + 0x18)
 *      AUD_CAPT_CONTROL_REG                                   (AUD_BASE + 0x30)
 *      AUD_OUTP_CONTROL_REG                                   (AUD_BASE + 0x34)
 *      AUD_CLOCK_CONTROL_REG                                  (AUD_BASE + 0x38)
 *      AUD_FRAME_PROTOCOL_REG                                 (AUD_BASE + 0x3C)
 *      AUD_NULL_REG                                           (AUD_BASE + 0x40)
 *      AUD_LEFT_ATTEN_REG                                     (AUD_BASE + 0x44)
 *      AUD_RIGHT_ATTEN_REG                                    (AUD_BASE + 0x48)
 *      AUD_PLAY_INT_STAT_REG                                  (AUD_BASE + 0x54)
 *      AUD_CAPT_INT_STAT_REG                                  (AUD_BASE + 0x58)
 *******************************************************************************
 */

/* Comment out the next line to remove audio bitfields. */
/*#define BITFIELD_AUD */
#define AUD_CHANNEL_CONTROL_REG                                (AUD_BASE + 0x00)
#define     AUD_CHANNEL_CONTROL_PLAY_ENABLE_MASK                   0x00000001
#define     AUD_CHANNEL_CONTROL_PLAY_ENABLE_SHIFT                         0
#define         AUD_CHANNEL_CONTROL_PLAY_DISABLE                     (0UL<<0)
#define         AUD_CHANNEL_CONTROL_PLAY_ENABLE                      (1UL<<0)
#define     AUD_CHANNEL_CONTROL_CAPT_ENABLE_MASK                   0x00000002
#define     AUD_CHANNEL_CONTROL_CAPT_ENABLE_SHIFT                         1
#define         AUD_CHANNEL_CONTROL_CAPT_DISABLE                     (0UL<<1)
#define         AUD_CHANNEL_CONTROL_CAPT_ENABLE                      (1UL<<1)
#define     AUD_CHANNEL_CONTROL_SERIAL_ENABLE_MASK                 0x00000004
#define     AUD_CHANNEL_CONTROL_SERIAL_ENABLE_SHIFT                       2
#define         AUD_CHANNEL_CONTROL_SERIAL_DISABLE                   (0UL<<2)
#define         AUD_CHANNEL_CONTROL_SERIAL_ENABLE                    (1UL<<2)
#define     AUD_CHANNEL_CONTROL_MPEG_CAPT_ATTEN_MASK               0x00000008
#define     AUD_CHANNEL_CONTROL_MPEG_CAPT_ATTEN_SHIFT                     3
#define         AUD_CHANNEL_CONTROL_MPEG_CAPT_ATTEN_DISABLE          (0UL<<3)
#define         AUD_CHANNEL_CONTROL_MPEG_CAPT_ATTEN_ENABLE           (1UL<<3)
#define     AUD_CHANNEL_CONTROL_SIX_CHAN_AUDIO_MASK                0x00000010
#define     AUD_CHANNEL_CONTROL_SIX_CHAN_AUDIO_SHIFT                      4
#define         AUD_CHANNEL_CONTROL_SIX_CHAN_AUDIO_DISABLE           (0UL<<4)
#define         AUD_CHANNEL_CONTROL_SIX_CHAN_AUDIO_ENABLE            (1UL<<4)
#define     AUD_CHANNEL_CONTROL_PLAY_COPYRIGHT_MASK                0x00000020
#define     AUD_CHANNEL_CONTROL_PLAY_COPYRIGHT_SHIFT                      5
#define         AUD_CHANNEL_CONTROL_PLAY_COPYRIGHT_DISABLE           (0UL<<5)
#define         AUD_CHANNEL_CONTROL_PLAY_COPYRIGHT_ENABLE            (1UL<<5)
#define     AUD_CHANNEL_CONTROL_RESERVED1_MASK                     0xFFFFFFC0
#define     AUD_CHANNEL_CONTROL_RESERVED1_SHIFT                           6

#define AUD_PLAY_A_START_REG                                   (AUD_BASE + 0x04)
#define AUD_PLAY_A_STOP_REG                                    (AUD_BASE + 0x08)
#define AUD_PLAY_B_START_REG                                   (AUD_BASE + 0x0C)
#define AUD_PLAY_B_STOP_REG                                    (AUD_BASE + 0x10)
#define AUD_PLAY_STATUS_REG                                    (AUD_BASE + 0x14)
#define AUD_CAPT_A_START_REG                                   (AUD_BASE + 0x1C)
#define AUD_CAPT_A_STOP_REG                                    (AUD_BASE + 0x20)
#define AUD_CAPT_B_START_REG                                   (AUD_BASE + 0x24)
#define AUD_CAPT_B_STOP_REG                                    (AUD_BASE + 0x28)
#define AUD_CAPT_STATUS_REG                                    (AUD_BASE + 0x2C)
#define AUD_PLAY_INT_ADDR_REG                                  (AUD_BASE + 0x4C)
#define AUD_CAPT_INT_ADDR_REG                                  (AUD_BASE + 0x50)
#define     AUD_ADDRESS_ADDRESS_MASK                               0x0FFFFFFF
#define     AUD_ADDRESS_ADDRESS_SHIFT                                     0
#define     AUD_ADDRESS_RESERVED_MASK                              0xF0000000
#define     AUD_ADDRESS_RESERVED_SHIFT                                   28

#define AUD_PLAY_CONTROL_REG                                   (AUD_BASE + 0x18)
#define AUD_CAPT_CONTROL_REG                                   (AUD_BASE + 0x30)
#define     AUD_FIFO_CONTROL_NORM_THRESHOLD_MASK                   0x0000000F
#define     AUD_FIFO_CONTROL_NORM_THRESHOLD_SHIFT                         0
#define     AUD_FIFO_CONTROL_CRIT_THRESHOLD_MASK                   0x000000F0
#define     AUD_FIFO_CONTROL_CRIT_THRESHOLD_SHIFT                         4
#define     AUD_FIFO_CONTROL_NORM_EVENT_OCCURED_MASK               0x00000100
#define     AUD_FIFO_CONTROL_NORM_EVENT_OCCURED_SHIFT                     8
#define     AUD_FIFO_CONTROL_CRIT_EVENT_OCCURED_MASK               0x00000200
#define     AUD_FIFO_CONTROL_CRIT_EVENT_OCCURED_SHIFT                     9
#define     AUD_FIFO_CONTROL_PLAY_A_ACTIVE_MASK                    0x00000400
#define     AUD_FIFO_CONTROL_PLAY_A_ACTIVE_SHIFT                         10
#define     AUD_FIFO_CONTROL_CAPT_A_ACTIVE_MASK                    0x00000800
#define     AUD_FIFO_CONTROL_CAPT_A_ACTIVE_SHIFT                         11
#define     AUD_FIFO_CONTROL_RESERVED1_MASK                        0xFFFFF000
#define     AUD_FIFO_CONTROL_RESERVED1_SHIFT                             12

#define AUD_OUTP_CONTROL_REG                                   (AUD_BASE + 0x34)
#define     AUD_OUTP_CONTROL_ENABLE_BITCK_MASK                     0x00000001
#define     AUD_OUTP_CONTROL_ENABLE_BITCK_SHIFT                           0
#define         AUD_OUTP_CONTROL_ENABLE_BITCK                        (1UL<<0)
#define     AUD_OUTP_CONTROL_BITCK_DEFAULT_MASK                    0x00000002
#define     AUD_OUTP_CONTROL_BITCK_DEFAULT_SHIFT                          1
#define         AUD_OUTP_CONTROL_BITCK_DEFAULT_0                     (0UL<<1)
#define         AUD_OUTP_CONTROL_BITCK_DEFAULT_1                     (1UL<<1)
#define     AUD_OUTP_CONTROL_ENABLE_LRCK_MASK                      0x00000004
#define     AUD_OUTP_CONTROL_ENABLE_LRCK_SHIFT                            2
#define         AUD_OUTP_CONTROL_ENABLE_LRCK                         (1UL<<2)
#define     AUD_OUTP_CONTROL_LRCK_DEFAULT_MASK                     0x00000008
#define     AUD_OUTP_CONTROL_LRCK_DEFAULT_SHIFT                           3
#define         AUD_OUTP_CONTROL_LRCK_DEFAULT_0                      (0UL<<3)
#define         AUD_OUTP_CONTROL_LRCK_DEFAULT_1                      (1UL<<3)
#define     AUD_OUTP_CONTROL_ENABLE_DATAOUT_MASK                   0x00000010
#define     AUD_OUTP_CONTROL_ENABLE_DATAOUT_SHIFT                         4
#define         AUD_OUTP_CONTROL_ENABLE_DATAOUT                      (1UL<<4)
#define     AUD_OUTP_CONTROL_DATAOUT_DEFAULT_MASK                  0x00000020
#define     AUD_OUTP_CONTROL_DATAOUT_DEFAULT_SHIFT                        5
#define         AUD_OUTP_CONTROL_DATAOUT_DEFAULT_0                   (0UL<<5)
#define         AUD_OUTP_CONTROL_DATAOUT_DEFAULT_1                   (1UL<<5)
#define     AUD_OUTP_CONTROL_ENABLE_DEEMP_MASK                     0x00000040
#define     AUD_OUTP_CONTROL_ENABLE_DEEMP_SHIFT                           6
#define         AUD_OUTP_CONTROL_ENABLE_DEEMP                        (1UL<<6)
#define     AUD_OUTP_CONTROL_DEEMP_DEFAULT_MASK                    0x00000180
#define     AUD_OUTP_CONTROL_DEEMP_DEFAULT_SHIFT                          7
#define         AUD_OUTP_CONTROL_DEEMP_DEFAULT_0                     (0UL<<7)
#define         AUD_OUTP_CONTROL_DEEMP_DEFAULT_1                     (1UL<<7)
#define         AUD_OUTP_CONTROL_DEEMP_DEFAULT_2                     (2UL<<7)
#define         AUD_OUTP_CONTROL_DEEMP_DEFAULT_3                     (3UL<<7)
#define     AUD_OUTP_CONTROL_DEEMP_STANDARD_MASK                   0x00000600
#define     AUD_OUTP_CONTROL_DEEMP_STANDARD_SHIFT                         9
#define         AUD_DEEMP_STANDARD_NONE                              (0UL<<9)
#define         AUD_DEEMP_STANDARD_5015uS                            (1UL<<9)
#define         AUD_DEEMP_STANDARD_CCITJ17                           (3UL<<9)
#define     AUD_OUTP_CONTROL_DEEMP_NEG_ASSERT_MASK                 0x00000800
#define     AUD_OUTP_CONTROL_DEEMP_NEG_ASSERT_SHIFT                      11
#define     AUD_OUTP_CONTROL_RESERVED1_MASK                        0xFFFFF000
#define     AUD_OUTP_CONTROL_RESERVED1_SHIFT                             12

#define AUD_CLOCK_CONTROL_REG                                  (AUD_BASE + 0x38)
#define     AUD_CLOCK_CONTROL_SOURCE_SELECT_MASK                   0x00000001
#define     AUD_CLOCK_CONTROL_SOURCE_SELECT_SHIFT                         0
#define         AUD_CLOCK_CONTROL_SOURCE_SEL_INTERNAL                (1UL<<0)
#define         AUD_CLOCK_CONTROL_SOURCE_SEL_EXTERNAL                (0UL<<0)
#define     AUD_CLOCK_CONTROL_BIT_CK32X_MASK                       0x00000002
#define     AUD_CLOCK_CONTROL_BIT_CK32X_SHIFT                             1
#define         AUD_CLOCK_CONTROL_BITCK_32X                          (1UL<<1)
#define         AUD_CLOCK_CONTROL_BITCK_64X                          (0UL<<1)
#define     AUD_CLOCK_CONTROL_ENABLE_OSCK1_MASK                    0x00000004
#define     AUD_CLOCK_CONTROL_ENABLE_OSCK1_SHIFT                          2
#define         AUD_CLOCK_CONTROL_ENABLE_OSCK1                       (1UL<<2)
#define     AUD_CLOCK_CONTROL_OSCK1_DEFAULT_MASK                   0x00000008
#define     AUD_CLOCK_CONTROL_OSCK1_DEFAULT_SHIFT                         3
#define         AUD_CLOCK_CONTROL_DEFAULT_OSCK1_0                    (0UL<<3)
#define         AUD_CLOCK_CONTROL_DEFAULT_OSCK1_1                    (1UL<<3)
#define     AUD_CLOCK_CONTROL_ENABLE_OSCK2_MASK                    0x00000010
#define     AUD_CLOCK_CONTROL_ENABLE_OSCK2_SHIFT                          4
#define         AUD_CLOCK_CONTROL_ENABLE_OSCK2                       (1UL<<4)
#define     AUD_CLOCK_CONTROL_OSCK2_DEFAULT_MASK                   0x00000020
#define     AUD_CLOCK_CONTROL_OSCK2_DEFAULT_SHIFT                         5
#define         AUD_CLOCK_CONTROL_DEFAULT_OSCK2_0                    (0UL<<5)
#define         AUD_CLOCK_CONTROL_DEFAULT_OSCK2_1                    (1UL<<5)
#define     AUD_CLOCK_CONTROL_SELECT_OSCK1_MASK                    0x00000040
#define     AUD_CLOCK_CONTROL_SELECT_OSCK1_SHIFT                          6
#define         AUD_CLOCK_CONTROL_OSCK1_SEL_CLK27                    (1UL<<6)
#define         AUD_CLOCK_CONTROL_OSCK1_SEL_ASPL_CLK                 (0UL<<6)
#define     AUD_CLOCK_CONTROL_OSCK1_DIVISOR_MASK                   0x00000380
#define     AUD_CLOCK_CONTROL_OSCK1_DIVISOR_SHIFT                         7
#define         AUD_CLOCK_CONTROL_OSCK1_DIV_BY_1                     (0UL<<7)
#define         AUD_CLOCK_CONTROL_OSCK1_DIV_BY_2                     (1UL<<7)
#define         AUD_CLOCK_CONTROL_OSCK1_DIV_BY_4                     (2UL<<7)
#define         AUD_CLOCK_CONTROL_OSCK1_DIV_BY_8                     (3UL<<7)
#define         AUD_CLOCK_CONTROL_OSCK1_DIV_BY_16                    (4UL<<7)
#define         AUD_CLOCK_CONTROL_OSCK1_DIV_BY_32                    (5UL<<7)
#define         AUD_CLOCK_CONTROL_OSCK1_DIV_BY_64                    (6UL<<7)
#define         AUD_CLOCK_CONTROL_OSCK1_DIV_BY_128                   (7UL<<7)
#define     AUD_CLOCK_CONTROL_SELECT_OSCK2_MASK                    0x00000400
#define     AUD_CLOCK_CONTROL_SELECT_OSCK2_SHIFT                         10
#define         AUD_CLOCK_CONTROL_OSCK2_SEL_CLK27                   (1UL<<10)
#define         AUD_CLOCK_CONTROL_OSCK2_SEL_ASPL_CLK                (0UL<<10)
#define     AUD_CLOCK_CONTROL_OSCK2_DIVISOR_MASK                   0x00003800
#define     AUD_CLOCK_CONTROL_OSCK2_DIVISOR_SHIFT                        11
#define         AUD_CLOCK_CONTROL_OSCK2_DIV_BY_1                    (0UL<<11)
#define         AUD_CLOCK_CONTROL_OSCK2_DIV_BY_2                    (1UL<<11)
#define         AUD_CLOCK_CONTROL_OSCK2_DIV_BY_4                    (2UL<<11)
#define         AUD_CLOCK_CONTROL_OSCK2_DIV_BY_8                    (3UL<<11)
#define         AUD_CLOCK_CONTROL_OSCK2_DIV_BY_16                   (4UL<<11)
#define         AUD_CLOCK_CONTROL_OSCK2_DIV_BY_32                   (5UL<<11)
#define         AUD_CLOCK_CONTROL_OSCK2_DIV_BY_64                   (6UL<<11)
#define         AUD_CLOCK_CONTROL_OSCK2_DIV_BY_128                  (7UL<<11)
#define     AUD_CLOCK_CONTROL_RESERVED1_MASK                       0xFFFFC000
#define     AUD_CLOCK_CONTROL_RESERVED1_SHIFT                            14

#define AUD_FRAME_PROTOCOL_REG                                 (AUD_BASE + 0x3C)
#define     AUD_FRAME_PROTOCOL_AUD_DATA_LEN_MASK                   0x0000000F
#define     AUD_FRAME_PROTOCOL_AUD_DATA_LEN_SHIFT                         0
#define         AUD_FRAME_PROTOCOL_LENGTH_16_BIT                     (0UL<<0)
#define         AUD_FRAME_PROTOCOL_LENGTH_18_BIT                     (2UL<<0)
#define         AUD_FRAME_PROTOCOL_LENGTH_20_BIT                     (4UL<<0)
#define     AUD_FRAME_PROTOCOL_ENABLE_STEREO_MASK                  0x00000010
#define     AUD_FRAME_PROTOCOL_ENABLE_STEREO_SHIFT                        4
#define         AUD_FRAME_PROTOCOL_ENABLE_STEREO                     (1UL<<4)
#define     AUD_FRAME_PROTOCOL_OP_FORMAT_SELECT_MASK               0x00000060
#define     AUD_FRAME_PROTOCOL_OP_FORMAT_SELECT_SHIFT                     5
#define         AUD_FRAME_PROTOCOL_OP_FORMAT_LEFTJUST                (0UL<<5)
#define         AUD_FRAME_PROTOCOL_OP_FORMAT_RIGHTJUST               (1UL<<5)
#define         AUD_FRAME_PROTOCOL_OP_FORMAT_I2S                     (2UL<<5)
#define     AUD_FRAME_PROTOCOL_IP_FORMAT_SELECT_MASK               0x00000180
#define     AUD_FRAME_PROTOCOL_IP_FORMAT_SELECT_SHIFT                     7
#define         AUD_FRAME_PROTOCOL_IP_FORMAT_LEFTJUST                (0UL<<7)
#define         AUD_FRAME_PROTOCOL_IP_FORMAT_RIGHTJUST               (1UL<<7)
#define         AUD_FRAME_PROTOCOL_IP_FORMAT_I2S                     (2UL<<7)
#define     AUD_FRAME_PROTOCOL_UNUSED_BIT_STATE_MASK               0x00000200
#define     AUD_FRAME_PROTOCOL_UNUSED_BIT_STATE_SHIFT                     9
#define         AUD_FRAME_PROTOCOL_UNUSED_0                          (0UL<<9)
#define         AUD_FRAME_PROTOCOL_UNUSED_1                          (1UL<<9)
#define     AUD_FRAME_PROTOCOL_IP_AUD_DATA_LEN_MASK                0x00003000
#define     AUD_FRAME_PROTOCOL_IP_AUD_DATA_LEN_SHIFT                     12
#define         AUD_FRAME_PROTOCOL_IP_AUD_DATA_LEN_16_BIT            (0UL<<12)
#define         AUD_FRAME_PROTOCOL_IP_AUD_DATA_LEN_18_BIT            (2UL<<12)
#define         AUD_FRAME_PROTOCOL_IP_AUD_DATA_LEN_20_BIT            (4UL<<12)
#define     AUD_FRAME_PROTOCOL_RESERVED1_MASK                      0xFFFF0C00
#define     AUD_FRAME_PROTOCOL_RESERVED1_SHIFT                           10

#define AUD_NULL_REG                                           (AUD_BASE + 0x40)
#define     AUD_NULL_NULL_WORD_MASK                                0x7FFFFFFF
#define     AUD_NULL_NULL_WORD_SHIFT                                      0
#define     AUD_NULL_RESERVED1 _MASK                               0x80000000
#define     AUD_NULL_RESERVED1_SHIFT                                     31

#define AUD_LEFT_ATTEN_REG                                     (AUD_BASE + 0x44)
#define AUD_RIGHT_ATTEN_REG                                    (AUD_BASE + 0x48)
#define     AUD_ATTEN_COEFFICIENT_MASK                             0x0000000F
#define     AUD_ATTEN_COEFFICIENT_SHIFT                                   0
#define     AUD_ATTEN_SHIFT_MASK                                   0x000000F0
#define     AUD_ATTEN_SHIFT_SHIFT                                         4
#define     AUD_ATTEN_RESERVED1_MASK                               0xFFFFFF00
#define     AUD_ATTEN_RESERVED1_SHIFT                                     8

#define AUD_PLAY_INT_STAT_REG                                  (AUD_BASE + 0x54)
#define AUD_CAPT_INT_STAT_REG                                  (AUD_BASE + 0x58)
#define     AUD_INT_STAT_ADDRESS_MASK                              0x00000001
#define     AUD_INT_STAT_ADDRESS_SHIFT                                    0
#define         AUD_INT_STAT_ADDRESS                                 (1UL<<0)
#define     AUD_INT_STAT_OVERRUN_MASK                              0x00000002
#define     AUD_INT_STAT_OVERRUN_SHIFT                                    1
#define         AUD_INT_STAT_OVERRUN                                 (1UL<<1)
#define     AUD_INT_STAT_UNDERRUN_MASK                             0x00000004
#define     AUD_INT_STAT_UNDERRUN_SHIFT                                   2
#define         AUD_INT_STAT_UNDERRUN                                (1UL<<2)
#define     AUD_INT_STAT_RESERVED1_MASK                            0xFFFFFFF8
#define     AUD_INT_STAT_RESERVED1_SHIFT                                  3

#if defined(BITFIELDS) || defined(BITFIELD_AUD)
    #if !defined(BITFIELD_AUD)
        #define BITFIELD_AUD
    #endif
    typedef struct _AUD_CHANNEL_CONTROL
    {
      BIT_FLD PlayEnable:1,
            CaptEnable:1,
            SerialEnable:1,
            Reserved1:29;
    } AUD_CHANNEL_CONTROL;
    typedef AUD_CHANNEL_CONTROL volatile *LPAUD_CHANNEL_CONTROL;

    typedef struct _AUD_ADDRESS
    {
      BIT_FLD Address:28,
            Reserved:4;
    } AUD_ADDRESS;
    typedef AUD_ADDRESS volatile *LPAUD_ADDRESS;

    typedef struct _AUD_FIFO_CONTROL
    {
      BIT_FLD NormThreshold:4,
            CritThreshold:4,
            NormEventOccured:1,
            CritEventOccured:1,
            PlayAActive:1,
            CaptAActive:1,
            Reserved1:20;
    } AUD_FIFO_CONTROL;
    typedef AUD_FIFO_CONTROL volatile *LPAUD_FIFO_CONTROL;

    typedef struct _AUD_OUTP_CONTROL
    {
      BIT_FLD EnableBitCk:1,
              BitCkDefault:1,
              EnableLRCK:1,
              LRCKDefault:1,
              EnableDataOut:1,
              DataOutDefault:1,
              EnableDeemp:1,
              DeempDefault:2,
              DeempStandard:2,
              DeempNegAssert:1,
              Reserved1:20;
    } AUD_OUTP_CONTROL;
    typedef AUD_OUTP_CONTROL volatile *LPAUD_OUTP_CONTROL;

    typedef struct _AUD_CLOCK_CONTROL
    {
      BIT_FLD SourceSelect:1,
            BitCk32x:1,
            EnableOSCk1:1,
            OSCk1Default:1,
            EnableOSCk2:1,
            OSCk2Default:1,
            SelectOSCk1:1,
            OSCk1Divisor:3,
            SelectOSCk2:1,
            OSCk2Divisor:3,
            Reserved1:18;
    } AUD_CLOCK_CONTROL;
    typedef AUD_CLOCK_CONTROL volatile *LPAUD_CLOCK_CONTROL;

    typedef struct _AUD_FRAME_PROTOCOL
    {
      BIT_FLD AudDataLen:4,
            EnableStereo:1,
            OPFormatSelect:2,
            IPFormatSelect:2,
            UnusedBitState:1,
            Reserved1:22;
    } AUD_FRAME_PROTOCOL;
    typedef AUD_FRAME_PROTOCOL volatile *LPAUD_FRAME_PROTOCOL;

    typedef struct _AUD_NULL
    {
      BIT_FLD NullWord:31,
            Reserved1:1;
    } AUD_NULL;
    typedef AUD_NULL volatile *LPAUD_NULL;

    typedef struct _AUD_ATTEN
    {
      BIT_FLD Coefficient:4,
              Shift:4,
              Reserved1:24;
    } AUD_ATTEN;
    typedef AUD_ATTEN volatile *LPAUD_ATTEN;

    typedef struct _AUD_INT_STAT
    {
      BIT_FLD Address:1,
            Overrun:1,
            Underrun:1,
            Reserved1:29;
    } AUD_INT_STAT;
    typedef AUD_INT_STAT volatile *LPAUD_INT_STAT;

    /* Types defined off of bit field types. */
    typedef AUD_ADDRESS   AUD_PLAY_A_START;
    typedef AUD_ADDRESS   AUD_PLAY_A_STOP;
    typedef AUD_ADDRESS   AUD_PLAY_B_START;
    typedef AUD_ADDRESS   AUD_PLAY_B_STOP;
    typedef AUD_ADDRESS   AUD_PLAY_STATUS;
    typedef AUD_ADDRESS   AUD_CAPT_A_START;
    typedef AUD_ADDRESS   AUD_CAPT_A_STOP;
    typedef AUD_ADDRESS   AUD_CAPT_B_START;
    typedef AUD_ADDRESS   AUD_CAPT_B_STOP;
    typedef AUD_ADDRESS   AUD_CAPT_STATUS;

    typedef LPAUD_ADDRESS LPAUD_PLAY_A_START;
    typedef LPAUD_ADDRESS LPAUD_PLAY_A_STOP;
    typedef LPAUD_ADDRESS LPAUD_PLAY_B_START;
    typedef LPAUD_ADDRESS LPAUD_PLAY_B_STOP;
    typedef LPAUD_ADDRESS LPAUD_PLAY_STATUS;
    typedef LPAUD_ADDRESS LPAUD_CAPT_A_START;
    typedef LPAUD_ADDRESS LPAUD_CAPT_A_STOP;
    typedef LPAUD_ADDRESS LPAUD_CAPT_B_START;
    typedef LPAUD_ADDRESS LPAUD_CAPT_B_STOP;
    typedef LPAUD_ADDRESS LPAUD_CAPT_STATUS;

    typedef AUD_FIFO_CONTROL   AUD_PLAY_CONTROL;
    typedef LPAUD_FIFO_CONTROL LPAUD_PLAY_CONTROL;
    typedef AUD_FIFO_CONTROL   AUD_CAPT_CONTROL;
    typedef LPAUD_FIFO_CONTROL LPAUD_CAPT_CONTROL;

    typedef AUD_ATTEN   AUD_LEFT_ATTEN;
    typedef AUD_ATTEN   AUD_RIGHT_ATTEN;
    typedef LPAUD_ATTEN LPAUD_LEFT_ATTEN;
    typedef LPAUD_ATTEN LPAUD_RIGHT_ATTEN;

    typedef AUD_ADDRESS   AUD_PLAY_INT_ADDR;
    typedef AUD_ADDRESS   AUD_CAPT_INT_ADDR;
    typedef LPAUD_ADDRESS LPAUD_PLAY_INT_ADDR;
    typedef LPAUD_ADDRESS LPAUD_CAPT_INT_ADDR;

    typedef AUD_INT_STAT   AUD_PLAY_INT_STAT;
    typedef LPAUD_INT_STAT LPAUD_PLAY_INT_STAT;
    typedef AUD_INT_STAT   AUD_CAPT_INT_STAT;
    typedef LPAUD_INT_STAT LPAUD_CAPT_INT_STAT;

    #define     AUD_FORMAT_LEFTJUST  0x00
    #define     AUD_FORMAT_RIGHTJUST 0x01
    #define     AUD_FORMAT_I2S       0x02


#endif /* BITFIELD_AUD */

#endif /* INCL_AUD */

/*********************/
/* PLLs and Clocking */
/*********************/

#ifdef INCL_PLL

/*
 *******************************************************************************
 *
 *      PLL_VID_CONFIG_REG                                     (PLL_BASE + 0x00)
 *      PLL_AUD_CONFIG_REG                                     (PLL_BASE + 0x04)
 *      PLL_CPU_CONFIG_REG                                     (PLL_BASE + 0x08)
 *      PLL_MEM_CONFIG_REG                                     (PLL_BASE + 0x0C)
 *      PLL_BYPASS_OPT_REG                                     (PLL_BASE + 0x10)
 *      PLL_RTC_DIVIDER_REG                                    (PLL_BASE + 0x14)
 *      PLL_DIV1_REG                                           (PLL_BASE + 0x18)
 *      PLL_DIV2_REG                                           (PLL_BASE + 0x1C)
 *      PLL_CONFIG0_REG                                        (PLL_BASE + 0x20)
 *      PLL_DAA_DIV_REG                                        (PLL_BASE + 0x28)
 *      PLL_USB_CONFIG_REG                                     (PLL_BASE + 0x2C)
 *      PLL_MEM_DELAY_REG                                      (PLL_BASE + 0x30)
 *      PLL_PIN_ALT_FUNC_REG                                   (PLL_BASE + 0x34)
 *      PLL_LOCK_CMD_REG                                       (PLL_BASE + 0x38)
 *      PLL_LOCK_STAT_REG                                      (PLL_BASE + 0x3C)
 *      PLL_PIN_GPIO_MUX0_REG                                  (PLL_BASE + 0x50)
 *      PLL_PIN_GPIO_MUX1_REG                                  (PLL_BASE + 0x54)
 *      PLL_PIN_GPIO_MUX2_REG                                  (PLL_BASE + 0x58)
 *      PLL_PRESCALE_REG                                       (PLL_BASE + 0x68)
 *
 *******************************************************************************
 */

#define PLL_VID_CONFIG_REG                                     (PLL_BASE + 0x00)
#define     PLL_VID_CONFIG_FRAC_MASK                               0x01FFFFFF
#define     PLL_VID_CONFIG_FRAC_SHIFT                                     0
#define     PLL_VID_CONFIG_INT_MASK                                0x7E000000
#define     PLL_VID_CONFIG_INT_SHIFT                                     25

#define PLL_AUD_CONFIG_REG                                     (PLL_BASE + 0x04)
#define     PLL_AUD_CONFIG_FRAC_MASK                               0x01FFFFFF
#define     PLL_AUD_CONFIG_FRAC_SHIFT                                     0
#define     PLL_AUD_CONFIG_INT_MASK                                0x7E000000
#define     PLL_AUD_CONFIG_INT_SHIFT                                     25

#define PLL_CPU_CONFIG_REG                                     (PLL_BASE + 0x08)
#define     PLL_CPU_CONFIG_FRAC_MASK                               0x01FFFE00
#define     PLL_CPU_CONFIG_FRAC_SHIFT                                     9
#define     PLL_CPU_CONFIG_INT_MASK                                0x7E000000
#define     PLL_CPU_CONFIG_INT_SHIFT                                     25

#define PLL_MEM_CONFIG_REG                                     (PLL_BASE + 0x0C)
#define     PLL_MEM_CONFIG_FRAC_MASK                               0x01FFFE00
#define     PLL_MEM_CONFIG_FRAC_SHIFT                                     9
#define     PLL_MEM_CONFIG_INT_MASK                                0x7E000000
#define     PLL_MEM_CONFIG_INT_SHIFT                                     25

#define PLL_BYPASS_OPT_REG                                     (PLL_BASE + 0x10)
#define     PLL_BYPASS_OPT_ENABLE_BYPASS_MASK                      0x0000001F
#define     PLL_BYPASS_OPT_ENABLE_BYPASS_SHIFT                            0
#define         PLL_BYPASS_OPT_ENABLE_BYPASS_MPG                     (1UL<<0)
#define         PLL_BYPASS_OPT_ENABLE_BYPASS_AUD                     (1UL<<1)
#define         PLL_BYPASS_OPT_ENABLE_BYPASS_ARM                     (1UL<<2)
#define         PLL_BYPASS_OPT_ENABLE_BYPASS_MEM                     (1UL<<3)
#define         PLL_BYPASS_OPT_ENABLE_BYPASS_USB                     (1UL<<4)
#define     PLL_BYPASS_OPT_REQUEST_RESET_MASK                      0x00000080
#define     PLL_BYPASS_OPT_REQUEST_RESET_SHIFT                            7
#define         PLL_BYPASS_OPT_REQUEST_SYS_RESET_DISABLE             (0UL<<7)
#define         PLL_BYPASS_OPT_REQUEST_SYS_RESET_ENABLE              (1UL<<7)
#define     PLL_BYPASS_OPT_RESET_ENABLE_MASK                       0x00000F00
#define     PLL_BYPASS_OPT_RESET_ENABLE_SHIFT                             8
#define         PLL_BYPASS_OPT_ENABLE_SYS_RESET_MPG                  (1UL<<8)
#define         PLL_BYPASS_OPT_ENABLE_SYS_RESET_AUD                  (1UL<<9)
#define         PLL_BYPASS_OPT_ENABLE_SYS_RESET_ARM                 (1UL<<10)
#define         PLL_BYPASS_OPT_ENABLE_SYS_RESET_MEM                 (1UL<<11)
#define     PLL_BYPASS_OPT_ENABLE_DELTA_SIGMA_MASK                 0x0001F000
#define     PLL_BYPASS_OPT_ENABLE_DELTA_SIGMA_SHIFT                      12
#define         PLL_BYPASS_OPT_DISABLE_DELTA_SIGMA_MPG              (1UL<<12)
#define         PLL_BYPASS_OPT_DISABLE_DELTA_SIGMA_AUD              (1UL<<13)
#define         PLL_BYPASS_OPT_DISABLE_DELTA_SIGMA_ARM              (1UL<<14)
#define         PLL_BYPASS_OPT_DISABLE_DELTA_SIGMA_MEM              (1UL<<15)
#define         PLL_BYPASS_OPT_DISABLE_DELTA_SIGMA_USB              (1UL<<16)
#define     PLL_BYPASS_OPT_OBS_SEL_MASK                            0x00F00000
#define     PLL_BYPASS_OPT_OBS_SEL_SHIFT                                 20
#define         PLL_BYPASS_OPT_OBS_SEL_NONE                         (0UL<<20)
#define         PLL_BYPASS_OPT_OBS_SEL_27MHZ_CLK                    (1UL<<20)
#define         PLL_BYPASS_OPT_OBS_SEL_AUDIO_PLL                    (2UL<<20)
#define         PLL_BYPASS_OPT_OBS_SEL_ARM_PLL                      (3UL<<20)
#define         PLL_BYPASS_OPT_OBS_SEL_MEM_PLL                      (4UL<<20)
#define         PLL_BYPASS_OPT_OBS_SEL_USB_PLL                      (5UL<<20)
#define         PLL_BYPASS_OPT_OBS_SEL_XTAL                         (6UL<<20)

#define PLL_RTC_DIVIDER_REG                                    (PLL_BASE + 0x14)
#define         PLL_RTC_DIV_54MHz_TO_1MHz                          0x00000036
#define         PLL_RTC_DIV_54MHz_TO_1000Hz                        0x0000D2F0
#define         PLL_RTC_DIV_54MHz_TO_100Hz                         0x00083D60
#define         PLL_RTC_DIV_54MHz_TO_1Hz                           0x0337F980

#define PLL_DIV1_REG                                           (PLL_BASE + 0x18)
#define     PLL_DIV1_MEM_CLK_MASK                                  0x0000000F
#define     PLL_DIV1_MEM_CLK_SHIFT                                        0
#define     PLL_DIV1_PCI_CLK_MASK                                  0x000000F0
#define     PLL_DIV1_PCI_CLK_SHIFT                                        4
#define     PLL_DIV1_B_CLK_MASK                                    0x00000F00
#define     PLL_DIV1_B_CLK_SHIFT                                          8
#define     PLL_DIV1_ARM_CLK_MASK                                  0x0000F000
#define     PLL_DIV1_ARM_CLK_SHIFT                                       12
#define     PLL_DIV1_USB_CLK_MASK                                  0x000F0000
#define     PLL_DIV1_USB_CLK_SHIFT                                       16
#define     PLL_DIV1_ASPL_CLK_MASK                                 0x03F00000
#define     PLL_DIV1_ASPL_CLK_SHIFT                                      20
#define     PLL_DIV1_HSDP_CLK_MASK                                 0xFC000000
#define     PLL_DIV1_HSDP_CLK_SHIFT                                      26


#define PLL_DIV2_REG                                           (PLL_BASE + 0x1C)
#define     PLL_DIV2_PAW_CLK_MASK                                  0x0000000F
#define     PLL_DIV2_PAW_CLK_SHIFT                                        0
#define     PLL_DIV2_GXA_CLK_MASK                                  0x000000F0
#define     PLL_DIV2_GXA_CLK_SHIFT                                        4
#define     PLL_DIV2_CLK27_MASK                                    0x00000F00
#define     PLL_DIV2_CLK27_SHIFT                                          8
#define     PLL_DIV2_ASX_CLK_MASK                                  0x0000F000
#define     PLL_DIV2_ASX_CLK_SHIFT                                       12
#define     PLL_DIV2_MPG_CLK_MASK                                  0x000F0000
#define     PLL_DIV2_MPG_CLK_SHIFT                                       16
#define     PLL_DIV2_HSDP_CLK_MASK                                 0x03F00000
#define     PLL_DIV2_HSDP_CLK_SHIFT                                      20



/*
 * See conexant.h, vendor_x.h header files for PLL_CONFIG0
 * definitions per vendor (and board type)
 */

#define PLL_CONFIG0_REG                                        (PLL_BASE + 0x20)

#define PLL_DAA_DIV_REG                                        (PLL_BASE + 0x28)
#define     PLL_DAA_DIV_D4CLK_LOW_DIV_MASK                         0x0000000F
#define     PLL_DAA_DIV_D4CLK_LOW_DIV_SHIFT                               0
#define     PLL_DAA_DIV_D4CLK_HIGH_DIV_MASK                        0x000000F0
#define     PLL_DAA_DIV_D4CLK_HIGH_DIV_SHIFT                              4
#define     PLL_DAA_DIV_DHCLK_LOW_DIV_MASK                         0x00000F00
#define     PLL_DAA_DIV_DHCLK_LOW_DIV_SHIFT                               8
#define     PLL_DAA_DIV_DHCLK_HIGH_DIV_MASK                        0x0000F000
#define     PLL_DAA_DIV_DHCLK_HIGH_DIV_SHIFT                             12
#define     PLL_DAA_DIV_USE_EXT_CLK_MASK                           0x00010000
#define     PLL_DAA_DIV_USE_EXT_CLK_SHIFT                                16
#define         PLL_DAA_DIV_USE_EXT_CLK                             (1UL<<16)

#define PLL_USB_CONFIG_REG                                     (PLL_BASE + 0x2C)
#define     PLL_USB_CONFIG_FRAC_MASK                               0x01FFFE00
#define     PLL_USB_CONFIG_FRAC_SHIFT                                     9
#define     PLL_USB_CONFIG_INT_MASK                                0x7E000000
#define     PLL_USB_CONFIG_INT_SHIFT                                     25

#define PLL_MEM_DELAY_REG                                      (PLL_BASE + 0x30)
#define     PLL_MEM_DELAY_WRITE_DELAY_MASK                         0x00000007
#define     PLL_MEM_DELAY_WRITE_DELAY_SHIFT                               0
#define     PLL_MEM_DELAY_READ_DELAY_MASK                          0x00000038
#define     PLL_MEM_DELAY_READ_DELAY_SHIFT                                3
#define     PLL_MEM_DELAY_PAW_CLK_DELAY1_MASK                      0x000000C0
#define     PLL_MEM_DELAY_PAW_CLK_DELAY1_SHIFT                            6
#define     PLL_MEM_DELAY_PAW_CLK_DELAY2_MASK                      0x00000300
#define     PLL_MEM_DELAY_PAW_CLK_DELAY2_SHIFT                            8
#define     PLL_MEM_DELAY_EXT_PCI_CLK_DELAY_MASK                   0x00000C00
#define     PLL_MEM_DELAY_EXT_PCI_CLK_DELAY_SHIFT                        10

#define PLL_PIN_ALT_FUNC_REG                                   (PLL_BASE + 0x34)
#define     PLL_PIN_ALT_FUNC_AFE_DAA_SEL_MASK                      0x00000001
#define     PLL_PIN_ALT_FUNC_AFE_DAA_SEL_SHIFT                            0
#define         PLL_PIN_ALT_FUNC_DAA_SEL                             (1UL<<0)
#define         PLL_PIN_ALT_FUNC_AFE_SEL                             (0UL<<0)
#define     PLL_PIN_ALT_FUNC_SMART_MDP_SCM_MASK                    0x00000004
#define     PLL_PIN_ALT_FUNC_SMART_MDP_SCM_SHIFT                          2
#define         PLL_PIN_ALT_FUNC_SMART_MDP_SCM_OUTPUT_DISABLE        (0UL<<2)
#define         PLL_PIN_ALT_FUNC_SMART_MDP_SCM_OUTPUT_ENABLE         (1UL<<2)
#define     PLL_PIN_ALT_FUNC_UART2_SEL_MASK                        0x00000030
#define     PLL_PIN_ALT_FUNC_UART2_SEL_SHIFT                              4
#define         PLL_PIN_ALT_FUNC_UART2_PIO_04_03_PINS                (0UL<<4)
#define         PLL_PIN_ALT_FUNC_UART2_PIO_71_72_PINS                (1UL<<4)
#define         PLL_PIN_ALT_FUNC_UART2_PIO_SM1OFF11_SM1IO73_PINS     (2UL<<4)
#define     PLL_PIN_ALT_FUNC_ARMCLK_BYPASS_MASK                    0x00000400
#define     PLL_PIN_ALT_FUNC_ARMCLK_BYPASS_SHIFT                         10
#define         PLL_PIN_ALT_FUNC_ARMCLK_BYPASS_DISABLE              (0UL<<10)
#define         PLL_PIN_ALT_FUNC_ARMCLK_BYPASS_ENABLE               (1UL<<10)
#define     PLL_PIN_ALT_FUNC_USBH1_PIO_SEL_MASK                    0x00000800
#define     PLL_PIN_ALT_FUNC_USBH1_PIO_SEL_SHIFT                         11
#define         PLL_PIN_ALT_FUNC_USBH1_PIO_SEL_DISABLE              (0UL<<11)
#define         PLL_PIN_ALT_FUNC_USBH1_PIO_SEL_ENABLE               (1UL<<11)
#define     PLL_PIN_ALT_FUNC_USBH2_PIO_SEL_MASK                    0x00001000
#define     PLL_PIN_ALT_FUNC_USBH2_PIO_SEL_SHIFT                         12
#define         PLL_PIN_ALT_FUNC_USBH2_PIO_SEL_DISABLE              (0UL<<12)
#define         PLL_PIN_ALT_FUNC_USBH2_PIO_SEL_ENABLE               (1UL<<12)
#define     PLL_PIN_ALT_FUNC_ENC_DBG_ENABLE_MASK                   0x00002000
#define     PLL_PIN_ALT_FUNC_ENC_DBG_ENABLE_SHIFT                        13
#define         PLL_PIN_ALT_FUNC_ENC_DBG_DISABLE                    (0UL<<13)
#define         PLL_PIN_ALT_FUNC_ENC_DBG_ENABLE                     (1UL<<13)
#define     PLL_PIN_ALT_FUNC_ENC_EXT_MASTER_SEL_MASK               0x00004000
#define     PLL_PIN_ALT_FUNC_ENC_EXT_MASTER_SEL_SHIFT                    14
#define         PLL_PIN_ALT_FUNC_ENC_EXT_MASTER_DISABLE             (0UL<<14)
#define         PLL_PIN_ALT_FUNC_ENC_EXT_MASTER_ENABLE              (1UL<<14)
#define     PLL_PIN_ALT_FUNC_ENC_TEST_SEL_MASK                     0x00008000
#define     PLL_PIN_ALT_FUNC_ENC_TEST_SEL_SHIFT                          15
#define         PLL_PIN_ALT_FUNC_ENC_TEST_SEL_DISABLE               (0UL<<15)
#define         PLL_PIN_ALT_FUNC_ENC_TEST_SEL_ENABLE                (1UL<<15)
#define     PLL_PIN_ALT_FUNC_ENC_TEST_DIR_MASK                     0x00010000
#define     PLL_PIN_ALT_FUNC_ENC_TEST_DIR_SHIFT                          16
#define         PLL_PIN_ALT_FUNC_ENC_TEST_OUTPUT                    (0UL<<16)
#define         PLL_PIN_ALT_FUNC_ENC_TEST_INPUT                     (1UL<<16)
#define     PLL_PIN_ALT_FUNC_ED_ENABLE_MASK                        0x00080000
#define     PLL_PIN_ALT_FUNC_ED_ENABLE_SHIFT                             19
#define         PLL_PIN_ALT_FUNC_ED_ENABLE_DISABLED                 (0UL<<19)
#define         PLL_PIN_ALT_FUNC_ED_ENABLE_ENABLED                  (1UL<<19)
#define     PLL_PIN_ALT_FUNC_ED_ALTERNATE_MASK                     0x00100000
#define     PLL_PIN_ALT_FUNC_ED_ALTERNATE_SHIFT                          20
#define         PLL_PIN_ALT_FUNC_ED_ALTERNATE_PRIMARY               (0UL<<20)
#define         PLL_PIN_ALT_FUNC_ED_ALTERNATE_ALTERNATE             (1UL<<20)
#define     PLL_PIN_ALT_FUNC_UART1_OWNERSHIP_MASK                  0x00400000
#define     PLL_PIN_ALT_FUNC_UART1_OWNERSHIP_SHIFT                       22
#define         PLL_PIN_ALT_FUNC_UART1_OWNER_CM                     (1UL<<22)
#define         PLL_PIN_ALT_FUNC_UART1_OWNER_MPEG                   (0UL<<22)
#define     PLL_PIN_ALT_FUNC_UART3_OWNERSHIP_MASK                  0x00800000
#define     PLL_PIN_ALT_FUNC_UART3_OWNERSHIP_SHIFT                       23
#define         PLL_PIN_ALT_FUNC_UART3_OWNER_CM                     (1UL<<23)
#define         PLL_PIN_ALT_FUNC_UART3_OWNER_MPEG                   (0UL<<23)
#define     PLL_PIN_ALT_FUNC_ALT_POD_PINS_ENABLE_MASK              0x02000000
#define     PLL_PIN_ALT_FUNC_ALT_POD_PINS_ENABLE_SHIFT                   26
#define         PLL_PIN_ALT_FUNC_ALT_POD_PINS_ENABLE                (1UL<<26)
#define         PLL_PIN_ALT_FUNC_ALT_POD_PINS_DISABLE               (0UL<<26)
#define     PLL_PIN_PIN_ALT_FUNC_ALT_POD_PINS_ENABLE_MASK          0x02000000
#define     PLL_PIN_ALT_FUNC_ALT_IORW_PINS_ENABLE_SHIFT                  28
#define         PLL_PIN_ALT_FUNC_ALT_IORW_PINS_ENABLE               (1UL<<28)
#define         PLL_PIN_ALT_FUNC_ALT_IORW_PINS_DISABLE              (0UL<<28)


#define PLL_LOCK_CMD_REG                                       (PLL_BASE + 0x38)
#define     PLL_LOCK_CMD_MASK                                      0x000000FF
#define     PLL_LOCK_CMD_SHIFT                                            0
#define         PLL_UNLOCK_CMD1                                          0xF8
#define         PLL_UNLOCK_CMD2                                          0x2B
#define         PLL_LOCK_CMD_VALUE                                       0x00

#define PLL_UNLOCK()              *(LPREG)PLL_LOCK_CMD_REG = PLL_UNLOCK_CMD1; \
                                  *(LPREG)PLL_LOCK_CMD_REG = PLL_UNLOCK_CMD2;
#define PLL_LOCK()                *(LPREG)PLL_LOCK_CMD_REG = PLL_LOCK_CMD_VALUE;

#define         PLL_LOCK_MPEG                                        (1UL<<0)
#define         PLL_LOCK_AUDIO                                       (1UL<<1)
#define         PLL_LOCK_ARM                                         (1UL<<2)
#define         PLL_LOCK_BUS                                         (1UL<<3)
#define         PLL_LOCK_PLLBYPASS                                   (1UL<<4)
#define         PLL_LOCK_ALLDIVIDERS                                 (1UL<<5)
#define         PLL_LOCK_SUSPEND                                     (1UL<<6)
#define         PLL_LOCK_STOP                                        (1UL<<7)
#define         PLL_LOCK_USB                                         (1UL<<16)

#define PLL_LOCK_STAT_REG                                      (PLL_BASE + 0x3C)
#define     PLL_LOCK_STAT_MPEG_PLL_MASK                            0x00000001
#define     PLL_LOCK_STAT_MPEG_PLL_SHIFT                                  0
#define         PLL_LOCK_STAT_MPEG_PLL_WRITE_ENABLE                  (0UL<<0)
#define         PLL_LOCK_STAT_MPEG_PLL_WRITE_DISABLE                 (1UL<<0)
#define     PLL_LOCK_STAT_AUDIO_PLL_MASK                           0x00000002
#define     PLL_LOCK_STAT_AUDIO_PLL_SHIFT                                 1
#define         PLL_LOCK_STAT_AUDIO_PLL_WRITE_ENABLE                 (0UL<<1)
#define         PLL_LOCK_STAT_AUDIO_PLL_WRITE_DISABLE                (1UL<<1)
#define     PLL_LOCK_STAT_ARM_PLL_MASK                             0x00000004
#define     PLL_LOCK_STAT_ARM_PLL_SHIFT                                   2
#define         PLL_LOCK_STAT_ARM_PLL_WRITE_ENABLE                   (0UL<<2)
#define         PLL_LOCK_STAT_ARM_PLL_WRITE_DISABLE                  (1UL<<2)
#define     PLL_LOCK_STAT_BUS_PLL_MASK                             0x00000008
#define     PLL_LOCK_STAT_BUS_PLL_SHIFT                                   3
#define         PLL_LOCK_STAT_BUS_PLL_WRITE_ENABLE                   (0UL<<3)
#define         PLL_LOCK_STAT_BUS_PLL_WRITE_DISABLE                  (1UL<<3)
#define     PLL_LOCK_STAT_PLL_BYPASS_MASK                          0x00000010
#define     PLL_LOCK_STAT_PLL_BYPASS_SHIFT                                4
#define         PLL_LOCK_STAT_PLL_BYPASS_WRITE_ENABLE                (0UL<<4)
#define         PLL_LOCK_STAT_PLL_BYPASS_WRITE_DISABLE               (1UL<<4)
#define     PLL_LOCK_STAT_ALL_DIVIDER_MASK                         0x00000020
#define     PLL_LOCK_STAT_ALL_DIVIDER_SHIFT                               5
#define         PLL_LOCK_STAT_ALL_DIVIDER_WRITE_ENABLE               (0UL<<5)
#define         PLL_LOCK_STAT_ALL_DIVIDER_WRITE_DISABLE              (1UL<<5)
#define     PLL_LOCK_STAT_SUSPEND_MODE_MASK                        0x00000040
#define     PLL_LOCK_STAT_SUSPEND_MODE_SHIFT                              6
#define         PLL_LOCK_STAT_SUSPEND_MODE_WRITE_ENABLE              (0UL<<6)
#define         PLL_LOCK_STAT_SUSPEND_MODE_WRITE_DISABLE             (1UL<<6)
#define     PLL_LOCK_STAT_STOP_MODE_MASK                           0x00000080
#define     PLL_LOCK_STAT_STOP_MODE_SHIFT                                 7
#define         PLL_LOCK_STAT_STOP_MODE_WRITE_ENABLE                 (0UL<<7)
#define         PLL_LOCK_STAT_STOP_MODE_WRITE_DISABLE                (1UL<<7)
#define     PLL_LOCK_STAT_TIMERS_MASK                              0x0000FF00
#define     PLL_LOCK_STAT_TIMERS_SHIFT                                    8
#define         PLL_LOCK_STAT_TIMER_0_WRITE_ENABLE                   (0UL<<8)
#define         PLL_LOCK_STAT_TIMER_0_WRITE_DISABLE                  (1UL<<8)
#define         PLL_LOCK_STAT_TIMER_1_WRITE_ENABLE                   (0UL<<9)
#define         PLL_LOCK_STAT_TIMER_1_WRITE_DISABLE                  (1UL<<9)
#define         PLL_LOCK_STAT_TIMER_2_WRITE_ENABLE                  (0UL<<10)
#define         PLL_LOCK_STAT_TIMER_2_WRITE_DISABLE                 (1UL<<10)
#define         PLL_LOCK_STAT_TIMER_3_WRITE_ENABLE                  (0UL<<11)
#define         PLL_LOCK_STAT_TIMER_3_WRITE_DISABLE                 (1UL<<11)
#define         PLL_LOCK_STAT_TIMER_4_WRITE_ENABLE                  (0UL<<12)
#define         PLL_LOCK_STAT_TIMER_4_WRITE_DISABLE                 (1UL<<12)
#define         PLL_LOCK_STAT_TIMER_5_WRITE_ENABLE                  (0UL<<13)
#define         PLL_LOCK_STAT_TIMER_5_WRITE_DISABLE                 (1UL<<13)
#define         PLL_LOCK_STAT_TIMER_6_WRITE_ENABLE                  (0UL<<14)
#define         PLL_LOCK_STAT_TIMER_6_WRITE_DISABLE                 (1UL<<14)
#define         PLL_LOCK_STAT_TIMER_7_WRITE_ENABLE                  (0UL<<15)
#define         PLL_LOCK_STAT_TIMER_7_WRITE_DISABLE                 (1UL<<15)
#define     PLL_LOCK_STAT_USB_MASK                                 0x00010000
#define     PLL_LOCK_STAT_USB_SHIFT                                      16
#define         PLL_LOCK_STAT_USB_PLL_WRITE_ENABLE                  (0UL<<16)
#define         PLL_LOCK_STAT_USB_PLL_WRITE_DISABLE                 (1UL<<16)

#define PLL_PIN_GPIO_MUX0_REG                                  (PLL_BASE + 0x50)
#define         PLL_PIN_GPIO_MUX0_PIO_0                              (1UL<<0)
#define         PLL_PIN_GPIO_MUX0_PIO_1                              (1UL<<1)
#define         PLL_PIN_GPIO_MUX0_UART1TX                            (1UL<<1)
#define         PLL_PIN_GPIO_MUX0_UART1TX_3                          (1UL<<1)
#define         PLL_PIN_GPIO_MUX0_PIO_2                              (1UL<<2)
#define         PLL_PIN_GPIO_MUX0_UART1RX                            (1UL<<2)
#define         PLL_PIN_GPIO_MUX0_UART1RX_4                          (1UL<<2)
#define         PLL_PIN_GPIO_MUX0_PIO_3                              (1UL<<3)
#define         PLL_PIN_GPIO_MUX0_UART2TX                            (1UL<<3)
#define         PLL_PIN_GPIO_MUX0_PIO_4                              (1UL<<4)
#define         PLL_PIN_GPIO_MUX0_UART2RX                            (1UL<<4)
#define         PLL_PIN_GPIO_MUX0_PIO_5                              (1UL<<5)
#define         PLL_PIN_GPIO_MUX0_PIO_6                              (1UL<<5)
#define         PLL_PIN_GPIO_MUX0_PIO_7                              (1UL<<6)
#define         PLL_PIN_GPIO_MUX0_PIO_8                              (1UL<<8)
#define         PLL_PIN_GPIO_MUX0_SSCLK                              (1UL<<8)
#define         PLL_PIN_GPIO_MUX0_PIO_9                              (1UL<<9)
#define         PLL_PIN_GPIO_MUX0_SSCS                               (1UL<<9)
#define         PLL_PIN_GPIO_MUX0_PIO_10                            (1UL<<10)
#define         PLL_PIN_GPIO_MUX0_IR_OUT                            (1UL<<10)
#define         PLL_PIN_GPIO_MUX0_PIO_11                            (1UL<<11)
#define         PLL_PIN_GPIO_MUX0_UART2RX_11                        (1UL<<11)
#define         PLL_PIN_GPIO_MUX0_PIO_12                            (1UL<<12)
#define         PLL_PIN_GPIO_MUX0_IR_IN                             (1UL<<12)
#define         PLL_PIN_GPIO_MUX0_PIO_13                            (1UL<<13)
#define         PLL_PIN_GPIO_MUX0_PIO_14                            (1UL<<14)
#define         PLL_PIN_GPIO_MUX0_UART3TX                           (1UL<<14)
#define         PLL_PIN_GPIO_MUX0_PIO_15                            (1UL<<15)
#define         PLL_PIN_GPIO_MUX0_UART3RX                           (1UL<<15)
#define         PLL_PIN_GPIO_MUX0_PIO_16                            (1UL<<16)
#define         PLL_PIN_GPIO_MUX0_PIO_17                            (1UL<<17)
#define         PLL_PIN_GPIO_MUX0_PIO_18                            (1UL<<18)
#define         PLL_PIN_GPIO_MUX0_PIO_19                            (1UL<<19)
#define         PLL_PIN_GPIO_MUX0_PIO_20                            (1UL<<20)
#define         PLL_PIN_GPIO_MUX0_PIO_21                            (1UL<<21)
#define         PLL_PIN_GPIO_MUX0_PIO_22                            (1UL<<22)
#define         PLL_PIN_GPIO_MUX0_PIO_23                            (1UL<<23)
#define         PLL_PIN_GPIO_MUX0_PIO_24                            (1UL<<24)
#define         PLL_PIN_GPIO_MUX0_PWM                               (1UL<<24)
#define         PLL_PIN_GPIO_MUX0_PIO_25                            (1UL<<25)
#define         PLL_PIN_GPIO_MUX0_PIO_26                            (1UL<<26)
#define         PLL_PIN_GPIO_MUX0_PIO_27                            (1UL<<27)
#define         PLL_PIN_GPIO_MUX0_UHF                               (1UL<<27)
#define         PLL_PIN_GPIO_MUX0_PIO_28                            (1UL<<28)
#define         PLL_PIN_GPIO_MUX0_PIO_29                            (1UL<<29)
#define         PLL_PIN_GPIO_MUX0_HS1CLK                            (1UL<<29)
#define         PLL_PIN_GPIO_MUX0_PIO_30                            (1UL<<30)
#define         PLL_PIN_GPIO_MUX0_HS1D0                             (1UL<<30)
#define         PLL_PIN_GPIO_MUX0_PIO_31                            (1UL<<31)
#define         PLL_PIN_GPIO_MUX0_HS1D1                             (1UL<<31)

#define PLL_PIN_GPIO_MUX1_REG                                  (PLL_BASE + 0x54)
#define         PLL_PIN_GPIO_MUX1_PIO_32                             (1UL<<0)
#define         PLL_PIN_GPIO_MUX1_HS1D2                              (1UL<<0)
#define         PLL_PIN_GPIO_MUX1_PIO_33                             (1UL<<1)
#define         PLL_PIN_GPIO_MUX1_HS1D3                              (1UL<<1)
#define         PLL_PIN_GPIO_MUX1_PIO_34                             (1UL<<2)
#define         PLL_PIN_GPIO_MUX1_HS1D4                              (1UL<<2)
#define         PLL_PIN_GPIO_MUX1_PIO_35                             (1UL<<3)
#define         PLL_PIN_GPIO_MUX1_HS1D5                              (1UL<<3)
#define         PLL_PIN_GPIO_MUX1_PIO_36                             (1UL<<4)
#define         PLL_PIN_GPIO_MUX1_HS1D6                              (1UL<<4)
#define         PLL_PIN_GPIO_MUX1_PIO_37                             (1UL<<5)
#define         PLL_PIN_GPIO_MUX1_HS1D7                              (1UL<<5)
#define         PLL_PIN_GPIO_MUX1_PIO_38                             (1UL<<6)
#define         PLL_PIN_GPIO_MUX1_HS1SYNC                            (1UL<<6)
#define         PLL_PIN_GPIO_MUX1_PIO_39                             (1UL<<7)
#define         PLL_PIN_GPIO_MUX1_AUD_SPDIF                          (1UL<<7)
#define         PLL_PIN_GPIO_MUX1_PIO_40                             (1UL<<8)
#define         PLL_PIN_GPIO_MUX1_AUD_UNDOUT                         (1UL<<8)
#define         PLL_PIN_GPIO_MUX1_PIO_41                             (1UL<<9)
#define         PLL_PIN_GPIO_MUX1_AUD_DEEMP                          (1UL<<9)
#define         PLL_PIN_GPIO_MUX1_PIO_42                            (1UL<<10)
#define         PLL_PIN_GPIO_MUX1_AUD_BITCK                         (1UL<<10)
#define         PLL_PIN_GPIO_MUX1_PIO_43                            (1UL<<11)
#define         PLL_PIN_GPIO_MUX1_HS1ERRAV                          (1UL<<11)
#define         PLL_PIN_GPIO_MUX1_PIO_44                            (1UL<<12)
#define         PLL_PIN_GPIO_MUX1_HS1RW                             (1UL<<12)
#define         PLL_PIN_GPIO_MUX1_PIO_45                            (1UL<<13)
#define         PLL_PIN_GPIO_MUX1_HS0RW                             (1UL<<13)
#define         PLL_PIN_GPIO_MUX1_PIO_46                            (1UL<<14)
#define         PLL_PIN_GPIO_MUX1_HS1VALEN                          (1UL<<14)
#define         PLL_PIN_GPIO_MUX1_PIO_47                            (1UL<<15)
#define         PLL_PIN_GPIO_MUX1_HS0D0                             (1UL<<15)
#define         PLL_PIN_GPIO_MUX1_PIO_48                            (1UL<<16)
#define         PLL_PIN_GPIO_MUX1_HS0D1                             (1UL<<16)
#define         PLL_PIN_GPIO_MUX1_PIO_49                            (1UL<<17)
#define         PLL_PIN_GPIO_MUX1_HS0D2                             (1UL<<17)
#define         PLL_PIN_GPIO_MUX1_PIO_50                            (1UL<<18)
#define         PLL_PIN_GPIO_MUX1_HS0D3                             (1UL<<18)
#define         PLL_PIN_GPIO_MUX1_PIO_51                            (1UL<<19)
#define         PLL_PIN_GPIO_MUX1_HS0D4                             (1UL<<19)
#define         PLL_PIN_GPIO_MUX1_PIO_52                            (1UL<<20)
#define         PLL_PIN_GPIO_MUX1_HS0D5                             (1UL<<20)
#define         PLL_PIN_GPIO_MUX1_PIO_53                            (1UL<<21)
#define         PLL_PIN_GPIO_MUX1_HS0D6                             (1UL<<21)
#define         PLL_PIN_GPIO_MUX1_PIO_54                            (1UL<<22)
#define         PLL_PIN_GPIO_MUX1_HS0D7                             (1UL<<22)
#define         PLL_PIN_GPIO_MUX1_PIO_55                            (1UL<<23)
#define         PLL_PIN_GPIO_MUX1_HS0CLK                            (1UL<<23)
#define         PLL_PIN_GPIO_MUX1_PIO_56                            (1UL<<24)
#define         PLL_PIN_GPIO_MUX1_HS0VALEN                          (1UL<<24)
#define         PLL_PIN_GPIO_MUX1_PIO_57                            (1UL<<25)
#define         PLL_PIN_GPIO_MUX1_HS0SYNC                           (1UL<<25)
#define         PLL_PIN_GPIO_MUX1_PIO_58                            (1UL<<26)
#define         PLL_PIN_GPIO_MUX1_AUDIN                             (1UL<<26)
#define         PLL_PIN_GPIO_MUX1_PIO_59                            (1UL<<27)
#define         PLL_PIN_GPIO_MUX1_PIO_60                            (1UL<<28)
#define         PLL_PIN_GPIO_MUX1_PIO_61                            (1UL<<29)
#define         PLL_PIN_GPIO_MUX1_PIO_62                            (1UL<<30)
#define         PLL_PIN_GPIO_MUX1_HS0ERRAV                          (1UL<<30)
#define         PLL_PIN_GPIO_MUX1_PIO_63                            (1UL<<31)

#define PLL_PIN_GPIO_MUX2_REG                                  (PLL_BASE + 0x58)
#define         PLL_PIN_GPIO_MUX2_PIO_64                             (1UL<<0)
#define         PLL_PIN_GPIO_MUX2_PIO_65                             (1UL<<1)
#define         PLL_PIN_GPIO_MUX2_PIO_66                             (1UL<<2)
#define         PLL_PIN_GPIO_MUX2_PIO_67                             (1UL<<3)
#define         PLL_PIN_GPIO_MUX2_HDRDY                              (1UL<<3)
#define         PLL_PIN_GPIO_MUX2_PIO_68                             (1UL<<4)
#define         PLL_PIN_GPIO_MUX2_SSTX                               (1UL<<4)
#define         PLL_PIN_GPIO_MUX2_PIO_69                             (1UL<<5)
#define         PLL_PIN_GPIO_MUX2_SSRX                               (1UL<<5)
#define         PLL_PIN_GPIO_MUX2_PIO_70                             (1UL<<6)
#define         PLL_PIN_GPIO_MUX2_IOREG                              (1UL<<6)
#define         PLL_PIN_GPIO_MUX2_PIO_71                             (1UL<<7)
#define         PLL_PIN_GPIO_MUX2_UART2RX_71                         (1UL<<7)
#define         PLL_PIN_GPIO_MUX2_PIO_72                             (1UL<<8)
#define         PLL_PIN_GPIO_MUX2_UART2TX_72                         (1UL<<8)
#define         PLL_PIN_GPIO_MUX2_PIO_73                             (1UL<<9)
#define         PLL_PIN_GPIO_MUX2_UART2TX_73                         (1UL<<9)
#define         PLL_PIN_GPIO_MUX2_PIO_74                            (1UL<<10)
#define         PLL_PIN_GPIO_MUX2_PIO_75                            (1UL<<11)
#define         PLL_PIN_GPIO_MUX2_PIO_76                            (1UL<<12)
#define         PLL_PIN_GPIO_MUX2_HDRD                              (1UL<<12)
#define         PLL_PIN_GPIO_MUX2_PIO_77                            (1UL<<13)
#define         PLL_PIN_GPIO_MUX2_HDACK                             (1UL<<13)
#define         PLL_PIN_GPIO_MUX2_PIO_78                            (1UL<<14)
#define         PLL_PIN_GPIO_MUX2_HDREQ                             (1UL<<14)
#define         PLL_PIN_GPIO_MUX2_PIO_79                            (1UL<<15)
#define         PLL_PIN_GPIO_MUX2_HDRW                              (1UL<<15)
#define         PLL_PIN_GPIO_MUX2_PIO_80                            (1UL<<16)
#define         PLL_PIN_GPIO_MUX2_HDD00                             (1UL<<16)
#define         PLL_PIN_GPIO_MUX2_PIO_81                            (1UL<<17)
#define         PLL_PIN_GPIO_MUX2_HDD01                             (1UL<<17)
#define         PLL_PIN_GPIO_MUX2_PIO_82                            (1UL<<18)
#define         PLL_PIN_GPIO_MUX2_HDD02                             (1UL<<18)
#define         PLL_PIN_GPIO_MUX2_PIO_83                            (1UL<<19)
#define         PLL_PIN_GPIO_MUX2_HDD03                             (1UL<<19)
#define         PLL_PIN_GPIO_MUX2_PIO_84                            (1UL<<20)
#define         PLL_PIN_GPIO_MUX2_HDD04                             (1UL<<20)
#define         PLL_PIN_GPIO_MUX2_PIO_85                            (1UL<<21)
#define         PLL_PIN_GPIO_MUX2_HDD05                             (1UL<<21)
#define         PLL_PIN_GPIO_MUX2_PIO_86                            (1UL<<22)
#define         PLL_PIN_GPIO_MUX2_HDD06                             (1UL<<22)
#define         PLL_PIN_GPIO_MUX2_PIO_87                            (1UL<<23)
#define         PLL_PIN_GPIO_MUX2_HDD07                             (1UL<<23)
#define         PLL_PIN_GPIO_MUX2_PIO_88                            (1UL<<24)
#define         PLL_PIN_GPIO_MUX2_HDD08                             (1UL<<24)
#define         PLL_PIN_GPIO_MUX2_PIO_89                            (1UL<<25)
#define         PLL_PIN_GPIO_MUX2_HDD09                             (1UL<<25)
#define         PLL_PIN_GPIO_MUX2_PIO_90                            (1UL<<26)
#define         PLL_PIN_GPIO_MUX2_HDD10                             (1UL<<26)
#define         PLL_PIN_GPIO_MUX2_PIO_91                            (1UL<<27)
#define         PLL_PIN_GPIO_MUX2_HDD11                             (1UL<<27)
#define         PLL_PIN_GPIO_MUX2_PIO_92                            (1UL<<28)
#define         PLL_PIN_GPIO_MUX2_HDD12                             (1UL<<28)
#define         PLL_PIN_GPIO_MUX2_PIO_93                            (1UL<<29)
#define         PLL_PIN_GPIO_MUX2_HDD13                             (1UL<<29)
#define         PLL_PIN_GPIO_MUX2_PIO_94                            (1UL<<30)
#define         PLL_PIN_GPIO_MUX2_HDD14                             (1UL<<30)
#define         PLL_PIN_GPIO_MUX2_PIO_95                            (1UL<<31)
#define         PLL_PIN_GPIO_MUX2_HDD15                             (1UL<<31)

#define PLL_PRESCALE_REG                                       (PLL_BASE + 0x68)
#define     PLL_PRESCALE_MPG_MASK                                  0x00000003
#define     PLL_PRESCALE_MPG_SHIFT                                        0
#define     PLL_PRESCALE_AUD_MASK                                  0x0000000C
#define     PLL_PRESCALE_AUD_SHIFT                                        2
#define     PLL_PRESCALE_ARM_MASK                                  0x00000030
#define     PLL_PRESCALE_ARM_SHIFT                                        4
#define     PLL_PRESCALE_MEM_MASK                                  0x000000C0
#define     PLL_PRESCALE_MEM_SHIFT                                        6
#define     PLL_PRESCALE_USB_MASK                                  0x00000300
#define     PLL_PRESCALE_USB_SHIFT                                        8
#define         PLL_PRESCALE(val) (((val)==0)?2:(((val)==1)?5:(((val)==2)?4:3)))


#ifdef BITFIELDS
typedef struct _PLL_LOCK_CMD
{
  BIT_FLD Command:8,
          Reserved1:24;
} PLL_LOCK_CMD;
typedef PLL_LOCK_CMD volatile *LPPLL_LOCK_CMD;

typedef struct _PLL_LOCK_STAT
{
  BIT_FLD MpegPll:1,
          AudioPll:1,
          ArmPll:1,
          BusPll:1,
          PllBypass:1,
          RTCDivider:1,
          Suspend:1,
          Stop:1,
          Timers:8,
          Reserved2:16;
} PLL_LOCK_STAT;
typedef PLL_LOCK_STAT volatile *LPPLL_LOCK_STAT;

typedef struct _PLL_MEM_DELAY
{
  BIT_FLD WriteDelay:3,
          ReadDelay:3,
		  PawclkDelay1:2,
		  PawclkDelay2:2,
		  ExtPCIDelay:2,
          Reserved1:20;
} PLL_MEM_DELAY;
typedef PLL_MEM_DELAY volatile *LPPLL_MEM_DELAY;

typedef struct _PLL_USB_CONFIG
{
  BIT_FLD Reserved1:9,
          Frac:16,
          Int:6,
          Reserved2:1;
} PLL_USB_CONFIG;
typedef PLL_USB_CONFIG volatile *LPPLL_USB_CONFIG;

typedef struct _PLL_MEM_CONFIG
{
  BIT_FLD Reserved1:9,
          Frac:16,
          Int:6,
          Reserved2:1;
} PLL_MEM_CONFIG;
typedef PLL_MEM_CONFIG volatile *LPPLL_MEM_CONFIG;

typedef struct _PLL_CPU_CONFIG
{
  BIT_FLD Reserved1:9,
          Frac:16,
          Int:6,
          Reserved2:1;
} PLL_CPU_CONFIG;
typedef PLL_CPU_CONFIG volatile *LPPLL_CPU_CONFIG;

typedef struct _PLL_AUD_CONFIG
{
  BIT_FLD Frac:25,
          Int:6,
          Reserved2:1;
} PLL_AUD_CONFIG;
typedef PLL_AUD_CONFIG volatile *LPPLL_AUD_CONFIG;

typedef struct _PLL_VID_CONFIG
{
  BIT_FLD Frac:25,
          Int:6,
          Reserved2:1;
} PLL_VID_CONFIG;
typedef PLL_VID_CONFIG volatile *LPPLL_VID_CONFIG;

typedef struct _PLL_DIV1
{
  BIT_FLD MemClk:4,
          PCIClk:4,
          BClk:4,
          ARMClk:4,
          USBClk:4,
          ASplClk:6,
          HSDPClk:6;
} PLL_DIV1;
typedef PLL_DIV1 volatile *LPPLL_DIV1;

typedef struct _PLL_DIV2
{
  BIT_FLD PawClk:4,
          GXAClk:4,
          Clk27:4,
          ASXClk:4,
          MPGClk:4,
          Reserved1:12;
} PLL_DIV2;
typedef PLL_DIV2 volatile *LPPLL_DIV2;

typedef struct _PLL_BYPASS_OPT
{
  BIT_FLD EnableBypass:5,
          Reserved1:2,
          ResetEnable:5,
		  DisableFrac:5,
		  Reserved2:3,
		  PllObsSel:4,
          Reserved3:8;
} PLL_BYPASS_OPT;
typedef PLL_BYPASS_OPT volatile *LPPLL_BYPASS_OPT;

typedef PLL_CONFIG0   RSO_BOARD_OPTIONS;
typedef LPPLL_CONFIG0 LPRSO_BOARD_OPTIONS;

typedef struct _PLL_DAA_DIV
{
  BIT_FLD  dhclkLow1:4,
           dhclkHigh1:4,
		   dhclkLow2:4,
		   dhclkHigh2:4,
		   Reserved1:16;
} PLL_DAA_DIV;
typedef PLL_DAA_DIV volatile *LPPLL_DAA_DIV;

typedef struct _PLL_PIN_MUXSEL
{
  BIT_FLD  PWM:1,
           Reserved1:3,
           AFE:1,
           Reserved2:2,
           SPI_In:1,
           SPI_Out:1,
           USBArmClk:1,
           Reserved3:2,
           Pulse:1,
           Reserved4:1,
           DAA:1,
           Reserved5:2,
           UART1:1,
           UART2_PRI:1,
           UART2_SEC:1,
           UART3:1,
           Reserved6:1,
           UDMA:1,
           HSDP0:1,
           HSDP1:1,
           Reserved7:1,
           UART2_TER:1,
           Reserved8:5;
} PLL_PIN_MUXSEL;
typedef PLL_PIN_MUXSEL volatile *LPPLL_PIN_MUXSEL;

typedef struct _PLL_PIN_ALT_FUNC
{
  BIT_FLD  AFE_DAA_SEL:1,
           Reserved1:1,
           SMART_MDP_SCM:1,
           Reserved2:1,
           UART2_SEL:2,
           Reserved3:4,
           ARMCLK_BYPASS:1,
           USBH1_PIO_SEL:1,
           USBH2_PIO_SEL:1,
		   ENC_DEBUG:1,
		   ENC_EXT_MASTER:1,
		   ENC_TEST:1,
		   ENC_TEST_DIR:1,
           Reserved4:15;
} PLL_PIN_ALT_FUNC;
typedef PLL_PIN_ALT_FUNC volatile *LPPLL_PIN_ALT_FUNC;

typedef struct _PLL_PIN_GPIO_MUX0
{
  BIT_FLD  pio0_pwr_button:1,
           pio1_uart1tx:1,
           pio2_uart1rx:1,
           pio3_uart2tx:1,
           pio4_uart2rx:1,
           pio5:1,
           pio6:1,
           pio7:1,
           pio8_ssclk:1,
           pio9_sscs:1,
           pio10_ir_out:1,
           pio11:1,
           pio12_ir_in:1,
           pio13:1,
           pio14_uart3tx:1,
           pio15_uart3rx:1,
           pio16:1,
           pio17_msctl_dspkr:1,
           pio18_mclk_dclk:1,
           pio19_mstb_dclkn:1,
           pio20_mtx_declk:1,
           pio21_mrx_dibp:1,
           pio22_msclk_dibn:1,
           pio23:1,
           pio24_pwm:1,
           pio25_dvbcirst:1,
           pio26:1,
           pio27_uhf:1,
           pio28:1,
           pio29_hs1clk:1,
           pio30_hs1d0:1,
           pio31_hs1d1:1;
} PLL_PIN_GPIO_MUX0;
typedef PLL_PIN_GPIO_MUX0 volatile *LPPLL_PIN_GPIO_MUX0;

typedef struct _PLL_PIN_GPIO_MUX1
{
  BIT_FLD  pio32_hs1d2:1,
           pio33_hs1d3:1,
           pio34_hs1d4:1,
           pio35_hs1d5:1,
           pio36_hs1d6:1,
           pio37_hs1d7:1,
           pio38_hs1sync:1,
           pio39_aud_spdif:1,
           pio40_aud_undout:1,
           pio41_aud_deemp:1,
           pio42_aud_bitck:1,
           pio43_hs1errav:1,
           pio44_hs1rw:1,
           pio45_hs0rw:1,
           pio46_hs1valen:1,
           pio47_hs0d0:1,
           pio48_hs0d1:1,
           pio49_hs0d2:1,
           pio50_hs0d3:1,
           pio51_hs0d4:1,
           pio52_hs0d5:1,
           pio53_hs0d6:1,
           pio54_hs0d7:1,
           pio55_hs0clk:1,
           pio56_hs0valen:1,
           pio57_hs0sync:1,
           pio58_nim0fail:1,
           pio59:1,
           pio60:1,
           pio61_nim1sync:1,
           pio62_hs0errav:1,
           pio63_nim1dval:1;
} PLL_PIN_GPIO_MUX1;
typedef PLL_PIN_GPIO_MUX1 volatile *LPPLL_PIN_GPIO_MUX1;

typedef struct _PLL_PIN_GPIO_MUX2
{
  BIT_FLD  pio64_nim1d0:1,
           pio65_nim1clk:1,
           pio66_nim1fail:1,
           pio67_hdrdy:1,
           pio68_sstx:1,
           pio69_ssrx:1,
           pio70_ioreg:1,
           pio71:1,
           pio72:1,
           pio73:1,
           pio74:1,
           pio75:1,
           pio76_hdrd:1,
           pio77_hdack:1,
           pio78_hdreq:1,
           pio79_hdrw:1,
           pio80_hdd00:1,
           pio81_hdd01:1,
           pio82_hdd02:1,
           pio83_hdd03:1,
           pio84_hdd04:1,
           pio85_hdd05:1,
           pio86_hdd06:1,
           pio87_hdd07:1,
           pio88_hdd08:1,
           pio89_hdd09:1,
           pio90_hdd10:1,
           pio91_hdd11:1,
           pio92_hdd12:1,
           pio93_hdd13:1,
           pio94_hdd14:1,
           pio95_hdd15:1;
} PLL_PIN_GPIO_MUX2;
typedef PLL_PIN_GPIO_MUX2 volatile *LPPLL_PIN_GPIO_MUX2;

#endif /* BITFIELDS */

#endif /* INCL_PLL */


/******************************/
/* Reset and Power Management */
/******************************/

#ifdef INCL_RST

/*
 *******************************************************************************
 *
 *      RST_STATUS_REG                                         (RST_BASE + 0x00)
 *      RST_SUSPEND_REG                                        (RST_BASE + 0x04)
 *      RST_STOP_REG                                           (RST_BASE + 0x08)
 *      RST_WAKEUP_CTRL_REG                                    (RST_BASE + 0x0C)
 *      RST_WAKEUP_STAT_REG                                    (RST_BASE + 0x10)
 *      RST_REMAP_REG                                          (RST_BASE + 0x14)
 *      RST_PLL_STATUS_REG                                     (RST_BASE + 0x18)
 *      RST_SOFTRESET_REG                                      (RST_BASE + 0x1C)
 *      RST_ASB_MODE_REG                                       (RST_BASE + 0x20)
 *      RST_SCRATCH_REG                                        (RST_BASE + 0x24)
 *      RST_GRESET_REG                                         (RST_BASE + 0x2C)
 *      RST_LPCONFIG_REG                                       (RST_BASE + 0x30)
 *      RST_LPCLOCK_REG                                        (RST_BASE + 0x34)
 *      RST_LPRESET_REG                                        (RST_BASE + 0x38)
 *      RST_LPTIMER_REG                                        (RST_BASE + 0x3C)
 *      RST_LOWPOWER_REG                                       (RST_BASE + 0x40)
 *      RST_HSX_ARBITER_PRI_REG                                (RST_BASE + 0x64)
 *
 *******************************************************************************
 */

#define RST_STATUS_REG                                         (RST_BASE + 0x00)
#define     RST_STATUS_POR_MASK                                    0x00000001
#define     RST_STATUS_POR_SHIFT                                          0
#define         RST_STATUS_CLEAR_POR                                 (1UL<<0)
#define     RST_STATUS_TIMEOUT_RESET_MASK                          0x00000004
#define     RST_STATUS_TIMEOUT_RESET_SHIFT                                2
#define         RST_STATUS_CLEAR_TIMEOUT_RESET                       (1UL<<2)
#define     RST_STATUS_STOPMODE_RESET_MASK                         0x00000008
#define     RST_STATUS_STOPMODE_RESET_SHIFT                               3
#define         RST_STATUS_CLEAR_STOPMODE_RESET                      (1UL<<3)
#define     RST_STATUS_SOFT_RESET_MASK                             0x00000010
#define     RST_STATUS_SOFT_RESET_SHIFT                                   4
#define         RST_STATUS_CLEAR_SOFT_RESET                          (1UL<<4)
#define     RST_STATUS_PLL_RESET_MASK                              0x00000020
#define     RST_STATUS_PLL_RESET_SHIFT                                    5
#define         RST_STATUS_CLEAR_PLL_RESET                           (1UL<<5)

/* Any read from the SUSPEND or STOP registers causes the relevant power-down */
/* mode to be entered. No bit definitions are necessary.                      */

#define RST_SUSPEND_REG                                        (RST_BASE + 0x04)
#define RST_STOP_REG                                           (RST_BASE + 0x08)

#define RST_WAKEUP_CTRL_REG                                    (RST_BASE + 0x0C)
#define RST_WAKEUP_STAT_REG                                    (RST_BASE + 0x10)
#define     RST_WAKEUP_RTC_MASK                                    0x00000001
#define     RST_WAKEUP_RTC_SHIFT                                          0
#define         RST_WAKEUP_RTC                                       (1UL<<0)
#define     RST_WAKEUP_GPIO_MASK                                   0x00000002
#define     RST_WAKEUP_GPIO_SHIFT                                         1
#define         RST_WAKEUP_GPIO                                      (1UL<<1)
#define     RST_WAKEUP_IR_MASK                                     0x00000004
#define     RST_WAKEUP_IR_SHIFT                                           2
#define         RST_WAKEUP_IR                                        (1UL<<2)
#define     RST_WAKEUP_UART1_MASK                                  0x00000008
#define     RST_WAKEUP_UART1_SHIFT                                        3
#define         RST_WAKEUP_UART1                                     (1UL<<3)
#define     RST_WAKEUP_UART2_MASK                                  0x00000010
#define     RST_WAKEUP_UART2_SHIFT                                        4
#define         RST_WAKEUP_UART2                                     (1UL<<4)
#define     RST_WAKEUP_PULSE_MASK                                  0x00000020
#define     RST_WAKEUP_PULSE_SHIFT                                        5
#define         RST_WAKEUP_PULSE                                     (1UL<<5)
#define     RST_WAKEUP_UART3_MASK                                  0x00000040
#define     RST_WAKEUP_UART3_SHIFT                                        6
#define         RST_WAKEUP_UART3                                     (1UL<<6)

#define RST_REMAP_REG                                          (RST_BASE + 0x14)
#define     RST_REMAP_BIT_MASK                                     0x00000001
#define     RST_REMAP_BIT_SHIFT                                           0
#define         RST_REMAP_BIT                                        (1UL<<1)

#define RST_PLL_STATUS_REG                                     (RST_BASE + 0x18)
#define     RST_PLL_STATUS_MPG_PLL_MASK                            0x00000001
#define     RST_PLL_STATUS_MPG_PLL_STATUS                            (1UL<<0)
#define     RST_PLL_STATUS_AUDIO_PLL_MASK                          0x00000002
#define     RST_PLL_STATUS_AUDIO_PLL_SHIFT                           (1UL<<1)
#define     RST_PLL_STATUS_ARM_PLL_MASK                            0x00000004
#define     RST_PLL_STATUS_ARM_PLL_SHIFT                             (1UL<<2)
#define     RST_PLL_STATUS_MEM_PLL_MASK                            0x00000008
#define     RST_PLL_STATUS_MEM_PLL_SHIFT                             (1UL<<3)
#define     RST_PLL_STATUS_USB_PLL_MASK                            0x00000010
#define     RST_PLL_STATUS_USB_PLL_SHIFT                             (1UL<<4)

#define RST_SOFTRESET_REG                                      (RST_BASE + 0x1C)
#define         RST_SOFTRESET                                      0xFFFFFFFF

#define RST_ASB_MODE_REG                                       (RST_BASE + 0x20)
#define     RST_ASB_MODE_ENABLE_ASX_TIMEOUT_MASK                   0x00000002
#define     RST_ASB_MODE_ENABLE_ASX_TIMEOUT_SHIFT                         1
#define         RST_ASB_MODE_ENABLE_ASX_TIMEOUT                      (1UL<<1)
#define     RST_ASB_MODE_ENABLE_ASX_TIMEOUT_ERROR_MASK             0x00000004
#define     RST_ASB_MODE_ENABLE_ASX_TIMEOUT_ERROR_SHIFT                   2
#define         RST_ASB_MODE_ENABLE_ASX_TIMEROUT_ERROR               (1UL<<2)
#define     RST_ASB_MODE_PREFETCH_MEMORY_ENABLE_MASK               0x00000010
#define     RST_ASB_MODE_PREFETCH_MEMORY_ENABLE_SHIFT                     4
#define         RST_ASB_MODE_PREFETCH_MEMORY_ENABLE                  (1UL<<4)
#define     RST_ASB_MODE_PREFETCH_ROM_ENABLE_MASK                  0x00000020
#define     RST_ASB_MODE_PREFETCH_ROM_ENABLE_SHIFT                        5
#define         RST_ASB_MODE_PREFETCH_ROM_ENABLE                     (1UL<<5)
#define     RST_ASB_MODE_PREFETCH_BURST_ENABLE_MASK                0x00000040
#define     RST_ASB_MODE_PREFETCH_BURST_ENABLE_SHIFT                      6
#define         RST_ASB_MODE_PREFETCH_BURST_ENABLE                   (1UL<<6)
#define     RST_ASB_MODE_SYNC_PIT_ENABLE_MASK                      0x00000080
#define     RST_ASB_MODE_SYNC_PIT_ENABLE_SHIFT                            7
#define         RST_ASB_MODE_SYNC_PIT_ENABLE                         (1UL<<7)
#define     RST_ASB_MODE_SUB_SYNC_PIT_ENABLE_MASK                  0x00000100
#define     RST_ASB_MODE_SUB_SYNC_PIT_ENABLE_SHIFT                        8
#define         RST_ASB_MODE_SUB_SYNC_PIT_ENABLE                     (1UL<<8)
#define     RST_ASB_MODE_MEMORY_INSTR_PREFETCH_ENABLE_MASK         0x00000200
#define     RST_ASB_MODE_MEMORY_INSTR_PREFETCH_ENABLE_SHIFT               9
#define         RST_ASB_MODE_MEMORY_INSTR_PREFETCH_ENABLE            (1UL<<9)
#define     RST_ASB_MODE_ROM_INSTR_PREFETCH_ENABLE_MASK            0X00000400
#define     RST_ASB_MODE_ROM_INSTR_PREFETCH_ENABLE_SHIFT                 10
#define         RST_ASB_MODE_ROM_INSTR_PREFETCH_ENABLE              (1UL<<10)
#define     RST_ASB_MODE_PREFETCH_MASK                             0x0000F000
#define     RST_ASB_MODE_PREFETCH_SHIFT                                  12
#define         RST_ASB_PREFETCH_SIZE_4_WORDS                       (3UL<<12)
#define         RST_ASB_PREFETCH_SIZE_8_WORDS                       (7UL<<12)
#define         RST_ASB_PREFETCH_SIZE_16_WORDS                     (15UL<<12)

#define RST_SCRATCH_REG                                        (RST_BASE + 0x24)
#define     RST_SCRATCH_BOOT_MASK                                  0x0000000f
#define     RST_SCRATCH_BOOT_SHIFT                                        0
#define         RST_SCRATCH_BOOT_ICE                                      0
#define         RST_SCRATCH_BOOT_SERIAL                             (15UL<<0)
#define         RST_SCRATCH_BOOT_IMAGE1                              (1UL<<0)
#define         RST_SCRATCH_BOOT_IMAGE2                              (2UL<<0)
#define         RST_SCRATCH_BOOT_IMAGE3                              (3UL<<0)
#define         RST_SCRATCH_BOOT_IMAGE4                              (4UL<<0)
#define         RST_SCRATCH_BOOT_CODELDREXT                          (5UL<<0)
#define     RST_SCRATCH_CFG_EEPROM_VALID_MASK                      0x00000010
#define     RST_SCRATCH_CFG_EEPROM_VALID_SHIFT                            4
#define         RST_SCRATCH_CFG_EEPROM_VALID                         (1UL<<4)
#define     RST_SCRATCH_SERIAL_CKPT_MASK                           0x00000020
#define     RST_SCRATCH_SERIAL_CKPT_SHIFT                                 5
#define         RST_SCRATCH_SERIAL_CKPT_ENABLE                       (1UL<<5)

#define RST_GRESET_REG                                         (RST_BASE + 0x2C)
#define     RST_GRESET_ASPL_MASK                                   0x00000001
#define     RST_GRESET_ASPL_SHIFT                                         0
#define         RST_GRESET_ASPL                                      (1UL<<0)
#define     RST_GRESET_ASX_MASK                                    0x00000002
#define     RST_GRESET_ASX_SHIFT                                          1
#define         RST_GRESET_ASX                                       (1UL<<1)
#define     RST_GRESET_CLK27_MASK                                  0x00000004
#define     RST_GRESET_CLK27_SHIFT                                        2
#define         RST_GRESET_CLK27                                     (1UL<<2)
#define     RST_GRESET_CLK40_MASK                                  0x00000008
#define     RST_GRESET_CLK40_SHIFT                                        3
#define         RST_GRESET_CLK40                                     (1UL<<3)
#define     RST_GRESET_CLK54_MASK                                  0x00000010
#define     RST_GRESET_CLK54_SHIFT                                        4
#define         RST_GRESET_CLK54                                     (1UL<<4)
#define     RST_GRESET_MEM_MASK                                    0x00000020
#define     RST_GRESET_MEM_SHIFT                                          5
#define         RST_GRESET_MEM                                       (1UL<<5)
#define     RST_GRESET_PCI_MASK                                    0x00000040
#define     RST_GRESET_PCI_SHIFT                                          6
#define         RST_GRESET_PCI                                       (1UL<<6)
#define     RST_GRESET_EVID_MASK                                   0x00000080
#define     RST_GRESET_EVID_SHIFT                                         7
#define         RST_GRESET_EVID                                      (1UL<<7)
#define     RST_GRESET_FCLK_MASK                                   0x00000100
#define     RST_GRESET_FCLK_SHIFT                                         8
#define         RST_GRESET_FCLK                                      (1UL<<8)
#define     RST_GRESET_ARM_MASK                                    0x00000200
#define     RST_GRESET_ARM_SHIFT                                          9
#define         RST_GRESET_ARM                                       (1UL<<9)
#define     RST_GRESET_EXT_MASK                                    0x00000400
#define     RST_GRESET_EXT_SHIFT                                         10
#define         RST_GRESET_EXT                                      (1UL<<10)

#define RST_LPCONFIG_REG                                       (RST_BASE + 0x30)
#define     RST_LPCONFIG_RESET_ENABLE_MASK                         0x00000001
#define     RST_LPCONFIG_RESET_ENABLE_SHIFT                               0
#define         RST_LPCONFIG_RESET_ENABLE                            (1UL<<0)
#define     RST_LPCONFIG_SHUTDOWN_AUD_PLL_MASK                     0x00000020
#define     RST_LPCONFIG_SHUTDOWN_AUD_PLL_SHIFT                           5
#define         RST_LPCONFIG_SHUTDOWN_AUD_PLL                        (1UL<<5)
#define     RST_LPCONFIG_SHUTDOWN_ARM_PLL_MASK                     0x00000040
#define     RST_LPCONFIG_SHUTDOWN_ARM_PLL_SHIFT                           6
#define         RST_LPCONFIG_SHUTDOWN_ARM_PLL                        (1UL<<6)
#define     RST_LPCONFIG_SHUTDOWN_MEM_PLL_MASK                     0x00000080
#define     RST_LPCONFIG_SHUTDOWN_MEM_PLL_SHIFT                           7
#define         RST_LPCONFIG_SHUTDOWN_MEM_PLL                        (1UL<<7)
#define     RST_LPCONFIG_SHUTDOWN_USB_PLL_MASK                     0x00000100
#define     RST_LPCONFIG_SHUTDOWN_USB_PLL_SHIFT                           8
#define         RST_LPCONFIG_SHUTDOWN_USB_PLL                        (1UL<<8)

#define RST_LPCLOCK_REG                                        (RST_BASE + 0x34)
#define     RST_LPCLOCK_AUD_MASK                                   0x00000001
#define     RST_LPCLOCK_AUD_SHIFT                                         0
#define         RST_LPCLOCK_AUD                                      (1UL<<0)
#define     RST_LPCLOCK_ASX_MASK                                   0x00000002
#define     RST_LPCLOCK_ASX_SHIFT                                         1
#define         RST_LPCLOCK_ASX                                      (1UL<<1)
#define     RST_LPCLOCK_CLK27_MASK                                 0x00000004
#define     RST_LPCLOCK_CLK27_SHIFT                                       2
#define         RST_LPCLOCK_CLK27                                    (1UL<<2)
#define     RST_LPCLOCK_CLK54_MASK                                 0x00000008
#define     RST_LPCLOCK_CLK54_SHIFT                                       3
#define         RST_LPCLOCK_CLK54                                    (1UL<<3)
#define     RST_LPCLOCK_MEM_MASK                                   0x00000020
#define     RST_LPCLOCK_MEM_SHIFT                                         5
#define         RST_LPCLOCK_MEM                                      (1UL<<5)
#define     RST_LPCLOCK_IO_MASK                                    0x00000040
#define     RST_LPCLOCK_IO_SHIFT                                          6
#define         RST_LPCLOCK_IO                                       (1UL<<6)
#define     RST_LPCLOCK_ASB_MASK                                   0x00000100
#define     RST_LPCLOCK_ASB_SHIFT                                         8
#define         RST_LPCLOCK_ASB                                      (1UL<<8)
#define     RST_LPCLOCK_ARM_MASK                                   0x00000200
#define     RST_LPCLOCK_ARM_SHIFT                                         9
#define         RST_LPCLOCK_ARM                                      (1UL<<9)
#define     RST_LPCLOCK_USB_MASK                                   0x00000800
#define     RST_LPCLOCK_USB_SHIFT                                        11
#define         RST_LPCLOCK_USB                                     (1UL<<11)
#define     RST_LPCLOCK_GXA_MASK                                   0x00001000
#define     RST_LPCLOCK_GXA_SHIFT                                        12
#define         RST_LPCLOCK_GXA                                     (1UL<<12)
#define     RST_LPCLOCK_PAW_MASK                                   0x00002000
#define     RST_LPCLOCK_PAW_SHIFT                                        13
#define         RST_LPCLOCK_PAW                                     (1UL<<13)

#define RST_LPRESET_REG                                        (RST_BASE + 0x38)
#define     RST_LPRESET_AUD_MASK                                   0x00000001
#define     RST_LPRESET_AUD_SHIFT                                         0
#define         RST_LPRESET_AUD                                      (1UL<<0)
#define     RST_LPRESET_ASX_MASK                                   0x00000002
#define     RST_LPRESET_ASX_SHIFT                                         1
#define         RST_LPRESET_ASX                                      (1UL<<1)
#define     RST_LPRESET_CLK27_MASK                                 0x00000004
#define     RST_LPRESET_CLK27_SHIFT                                       2
#define         RST_LPRESET_CLK27                                    (1UL<<2)
#define     RST_LPRESET_CLK54_MASK                                 0x00000008
#define     RST_LPRESET_CLK54_SHIFT                                       3
#define         RST_LPRESET_CLK54                                    (1UL<<3)
#define     RST_LPRESET_MEM_MASK                                   0x00000020
#define     RST_LPRESET_MEM_SHIFT                                         5
#define         RST_LPRESET_MEM                                      (1UL<<5)
#define     RST_LPRESET_IO_MASK                                    0x00000040
#define     RST_LPRESET_IO_SHIFT                                          6
#define         RST_LPRESET_IO                                       (1UL<<6)
#define     RST_LPRESET_EVID_MASK                                  0x00000080
#define     RST_LPRESET_EVID_SHIFT                                        7
#define         RST_LPRESET_EVID                                     (1UL<<7)
#define     RST_LPRESET_ASB_MASK                                   0x00000100
#define     RST_LPRESET_ASB_SHIFT                                         8
#define         RST_LPRESET_ASB                                      (1UL<<8)
#define     RST_LPRESET_ARM_MASK                                   0x00000200
#define     RST_LPRESET_ARM_SHIFT                                         9
#define         RST_LPRESET_ARM                                      (1UL<<9)
#define     RST_LPRESET_USB_MASK                                   0x00000800
#define     RST_LPRESET_USB_SHIFT                                        11
#define         RST_LPRESET_USB                                     (1UL<<11)
#define     RST_LPRESET_GXA_MASK                                   0x00001000
#define     RST_LPRESET_GXA_SHIFT                                        12
#define         RST_LPRESET_GXA                                     (1UL<<12)
#define     RST_LPRESET_PAW_MASK                                   0x00002000
#define     RST_LPRESET_PAW_SHIFT                                        13
#define         RST_LPRESET_PAW                                     (1UL<<13)

#define RST_LPTIMER_REG                                        (RST_BASE + 0x3C)
#define     RST_LPTIMER_MASK                                       0x0000FFFF
#define     RST_LPTIMER_SHIFT                                             0

#define RST_LOWPOWER_REG                                       (RST_BASE + 0x40)
#define     RST_LOWPOWER_AUD_MASK                                  0x00000003
#define     RST_LOWPOWER_AUD_SHIFT                                        0
#define         RST_LOWPOWER_AUD                                     (3UL<<0)
#define     RST_LOWPOWER_VID_MASK                                  0x0000000C
#define     RST_LOWPOWER_VID_SHIFT                                        2
#define         RST_LOWPOWER_VID                                     (3UL<<2)
#define     RST_LOWPOWER_DRMV0_MASK                                0x00000010
#define     RST_LOWPOWER_DRMV1_MASK                                0x00000020
#define     RST_LOWPOWER_DRMV_MASK                                 0x00000030
#define     RST_LOWPOWER_DRMV_SHIFT                                       4
#define         RST_LOWPOWER_DRMV                                    (3UL<<4)
#define     RST_LOWPOWER_GXA_MASK                                  0x000000C0
#define     RST_LOWPOWER_GXA_SHIFT                                        6
#define         RST_LOWPOWER_GXA                                     (3UL<<6)
#define     RST_LOWPOWER_DRMG_MASK                                 0x00000100
#define     RST_LOWPOWER_DRMG_SHIFT                                       8
#define         RST_LOWPOWER_DRMG                                    (1UL<<8)
#define     RST_LOWPOWER_DRM_MASK                                  0x00000200
#define     RST_LOWPOWER_DRM_SHIFT                                        9
#define         RST_LOWPOWER_DRM                                     (1UL<<9)
#define     RST_LOWPOWER_PAW_MASK                                  0x00000C00
#define     RST_LOWPOWER_PAW_SHIFT                                       10
#define         RST_LOWPOWER_PAW                                    (3UL<<10)
#define     RST_LOWPOWER_HSDP_MASK                                 0x00001000
#define     RST_LOWPOWER_HSDP_SHIFT                                      12
#define         RST_LOWPOWER_HSDP                                   (1UL<<12)
#define     RST_LOWPOWER_EVID_MASK                                 0x00002000
#define     RST_LOWPOWER_EVID_SHIFT                                      13
#define         RST_LOWPOWER_EVID                                   (1UL<<13)
#define     RST_LOWPOWER_DAA_MASK                                  0x00004000
#define     RST_LOWPOWER_DAA_SHIFT                                       14
#define         RST_LOWPOWER_DAA                                    (1UL<<14)
#define     RST_LOWPOWER_AFE_MASK                                  0x00008000
#define     RST_LOWPOWER_AFE_SHIFT                                       15
#define         RST_LOWPOWER_AFE                                    (1UL<<15)
#define     RST_LOWPOWER_I2C_MASK                                  0x00010000
#define     RST_LOWPOWER_I2C_SHIFT                                       16
#define         RST_LOWPOWER_I2C                                    (1UL<<16)
#define     RST_LOWPOWER_IR_MASK                                   0x00020000
#define     RST_LOWPOWER_IR_SHIFT                                        17
#define         RST_LOWPOWER_IR                                     (1UL<<17)
#define     RST_LOWPOWER_UART_MASK                                 0x000C0000
#define     RST_LOWPOWER_UART_SHIFT                                      18
#define         RST_LOWPOWER_UART                                   (3UL<<18)
#define     RST_LOWPOWER_PAR_MASK                                  0x00100000
#define     RST_LOWPOWER_PAR_SHIFT                                       20
#define         RST_LOWPOWER_PAR                                    (1UL<<20)
#define     RST_LOWPOWER_SCD_MASK                                  0x00600000
#define     RST_LOWPOWER_SCD_SHIFT                                       21
#define         RST_LOWPOWER_SCD                                    (3UL<<21)
#define     RST_LOWPOWER_USB_MASK                                  0x00800000
#define     RST_LOWPOWER_USB_SHIFT                                       23
#define         RST_LOWPOWER_USB                                    (1UL<<23)
#define     RST_LOWPOWER_PCI_MASK                                  0x01000000
#define     RST_LOWPOWER_PCI_SHIFT                                       24
#define         RST_LOWPOWER_PCI                                    (1UL<<24)
#define     RST_LOWPOWER_UART2_MASK                                0x02000000
#define     RST_LOWPOWER_UART2_SHIFT                                     25
#define         RST_LOWPOWER_UART2                                  (1UL<<25)
#define     RST_LOWPOWER_PWM_MASK                                  0x04000000
#define     RST_LOWPOWER_PWM_SHIFT                                       26
#define         RST_LOWPOWER_PWM                                    (1UL<<26)
#define     RST_LOWPOWER_PULSE_MASK                                0x08000000
#define     RST_LOWPOWER_PULSE_SHIFT                                     27
#define         RST_LOWPOWER_PULSE                                  (1UL<<27)
#define     RST_LOWPOWER_SSER_MASK                                 0x10000000
#define     RST_LOWPOWER_SSER_SHIFT                                      28
#define         RST_LOWPOWER_SSER                                   (1UL<<28)

#define RST_SCRATCH2_REG                                       (RST_BASE + 0x50)
#define RST_SCRATCH3_REG                                       (RST_BASE + 0x54)
#define RST_SCRATCH4_REG                                       (RST_BASE + 0x58)
#define RST_SCRATCH5_REG                                       (RST_BASE + 0x5C)

#define RST_HSX_ARBITER_PRI_REG                                (RST_BASE + 0x64)

#define     RST_HSX_ARBITER_HIGH_GXA_MASK                               0x02000000
#define     RST_HSX_ARBITER_HIGH_GXA_SHIFT                                    25
#define         RST_HSX_ARBITER_HIGH_GXA                                 (1UL<<25)

#define     RST_HSX_ARBITER_HIGH_SP3_MASK                               0x01000000
#define     RST_HSX_ARBITER_HIGH_SP3_SHIFT                                    24
#define         RST_HSX_ARBITER_HIGH_SP3                                 (1UL<<24)

#define     RST_HSX_ARBITER_HIGH_SP2_MASK                               0x00800000
#define     RST_HSX_ARBITER_HIGH_SP2_SHIFT                                    23
#define         RST_HSX_ARBITER_HIGH_SP2                                 (1UL<<23)

#define     RST_HSX_ARBITER_HIGH_USB_MASK                               0x00400000
#define     RST_HSX_ARBITER_HIGH_USB_SHIFT                                    22
#define         RST_HSX_ARBITER_HIGH_USB                                 (1UL<<22)

#define     RST_HSX_ARBITER_HIGH_SP0_MASK                               0x00100000
#define     RST_HSX_ARBITER_HIGH_SP0_SHIFT                                    20
#define         RST_HSX_ARBITER_HIGH_SP0                                 (1UL<<20)

#define     RST_HSX_ARBITER_HIGH_ATA_MASK                               0x00080000
#define     RST_HSX_ARBITER_HIGH_ATA_SHIFT                                    19
#define         RST_HSX_ARBITER_HIGH_ATA                                 (1UL<<19)

#define     RST_HSX_ARBITER_HIGH_DMA_MASK                               0x00040000
#define     RST_HSX_ARBITER_HIGH_DMA_SHIFT                                    18
#define         RST_HSX_ARBITER_HIGH_DMA                                 (1UL<<18)

#define     RST_HSX_ARBITER_HIGH_PIT_MASK                               0x00020000
#define     RST_HSX_ARBITER_HIGH_PIT_SHIFT                                    17
#define         RST_HSX_ARBITER_HIGH_PIT                                 (1UL<<17)

#define     RST_HSX_ARBITER_HIGH_PCI_MASK                               0x00010000
#define     RST_HSX_ARBITER_HIGH_PCI_SHIFT                                    16
#define         RST_HSX_ARBITER_HIGH_PCI                                 (1UL<<16)

#define     RST_HSX_ARBITER_MED_GXA_MASK                               0x00000200
#define     RST_HSX_ARBITER_MED_GXA_SHIFT                                     9
#define         RST_HSX_ARBITER_MED_GXA                                 (1UL<< 9)

#define     RST_HSX_ARBITER_MED_SP3_MASK                               0x00000100
#define     RST_HSX_ARBITER_MED_SP3_SHIFT                                     8
#define         RST_HSX_ARBITER_MED_SP3                                 (1UL<< 8)

#define     RST_HSX_ARBITER_MED_SP2_MASK                               0x00000080
#define     RST_HSX_ARBITER_MED_SP2_SHIFT                                     7
#define         RST_HSX_ARBITER_MED_SP2                                 (1UL<< 7)

#define     RST_HSX_ARBITER_MED_USB_MASK                               0x00000040
#define     RST_HSX_ARBITER_MED_USB_SHIFT                                     6
#define         RST_HSX_ARBITER_MED_USB                                 (1UL<< 6)

#define     RST_HSX_ARBITER_MED_SP0_MASK                               0x00000010
#define     RST_HSX_ARBITER_MED_SP0_SHIFT                                     4
#define         RST_HSX_ARBITER_MED_SP0                                 (1UL<< 4)

#define     RST_HSX_ARBITER_MED_ATA_MASK                               0x00000008
#define     RST_HSX_ARBITER_MED_ATA_SHIFT                                     3
#define         RST_HSX_ARBITER_MED_ATA                                 (1UL<< 3)

#define     RST_HSX_ARBITER_MED_DMA_MASK                               0x00000004
#define     RST_HSX_ARBITER_MED_DMA_SHIFT                                     2
#define         RST_HSX_ARBITER_MED_DMA                                 (1UL<< 2)

#define     RST_HSX_ARBITER_MED_PIT_MASK                               0x00000002
#define     RST_HSX_ARBITER_MED_PIT_SHIFT                                     1
#define         RST_HSX_ARBITER_MED_PIT                                 (1UL<< 1)

#define     RST_HSX_ARBITER_MED_PCI_MASK                               0x00000001
#define     RST_HSX_ARBITER_MED_PCI_SHIFT                                     0
#define         RST_HSX_ARBITER_MED_PCI                                 (1UL<< 0)


#ifdef BITFIELDS
typedef struct _RST_STATUS
{
  BIT_FLD POR:1,
          Button:1,
          Timeout:1,
          SleepComa:1,
          SoftReset:1,
          PLLChanged:1,
          Reserved1:26;
} RST_STATUS;
typedef RST_STATUS volatile *LPRST_STATUS;

typedef struct _RST_WAKEUP_CTRL
{
  BIT_FLD RTC:1,
          GPIO:1,
          IR:1,
          UART0:1,
          UART1:1,
          PulseTimer:1,
          Reserved1:26;
} RST_WAKEUP_CTRL;
typedef RST_WAKEUP_CTRL volatile *LPRST_WAKEUP_CTRL;

typedef RST_WAKEUP_CTRL   RST_WAKEUP_STAT;
typedef LPRST_WAKEUP_CTRL LPRST_WAKEUP_STAT;

typedef struct _RST_REMAP
{
  BIT_FLD Remap:1,
          Reserved1:31;
} RST_REMAP;
typedef RST_REMAP volatile *LPRST_REMAP;

typedef struct _RST_PLL_STATUS
{
  BIT_FLD MPEGPLL:1,
          AudioPLL:1,
          ARMPLL:1,
          MemoryPLL:1,
          Reserved1:28;
} RST_PLL_STATUS;
typedef RST_PLL_STATUS volatile *LPRST_PLL_STATUS;

typedef struct _RST_ASB_MODE
{
  BIT_FLD ParkOnARM:1,
          EnableASXTimeout:1,
          EnableASXTimeoutError:1,
          Reserved1:1, 
          PrefetchMem:1,
          PrefetchROM:1,
          PrefetchBurst:1,
          SyncPIT:1,
          SubSyncPIT:1,
          MemInstrPrefetchEnable:1,
          ROMInstrPrefetchEnable:1,
          Reserved2:1,
          PrefetchSize:4,
          Reserved3:16;
} RST_ASB_MODE;
typedef RST_ASB_MODE volatile *LPRST_ASB_MODE;

typedef struct _RST_SCRATCH
{
  HW_DWORD Value;
} RST_SCRATCH;
typedef RST_SCRATCH volatile *LPRST_SCRATCH;

typedef struct _RST_GRESET
{
  BIT_FLD ASPL:1,
          ASX:1,
          Clk27:1,
          Clk40:1,
          Clk54:1,
          Mem:1,
          PCI:1,
          EVID:1,      /* Unused */
          FClk:1,      /* Unused */
          Arm:1,       /* Unused */
          ExtReset:1,
          Reserved1:21;
} RST_GRESET;
typedef RST_GRESET volatile *LPRST_GRESET;

typedef struct _RST_LPCONFIG
{
  BIT_FLD ResetEnable:1,
          Reserved1:4,
          ShutdownAudio:1,
          ShutdownARM:1,
          ShutdownBus:1,
          ShutdownUSB:1,
          Reserved2:23;
} RST_LPCONFIG;
typedef RST_LPCONFIG volatile *LPRST_LPCONFIG;

typedef struct _RST_LPCLOCK
{
  BIT_FLD Audio:1,
          ASX:1,
          Clk27:1,
          Clk54:1,
          Reserved1:1,
          Mem:1,
          IO:1,
          Reserved2:1,
          ASB:1,
          Arm:1,
          Reserved3:1,
          USB:1,
          GXA:1,
          PAW:1,
          Reserved4:18;
} RST_LPCLOCK;
typedef RST_LPCLOCK volatile *LPRST_LPCLOCK;

typedef struct _RST_LPRESET
{
  BIT_FLD Audio:1,
          ASX:1,
          Clk27:1,
          Clk54:1,
          Reserved1:1,
          Mem:1,
          IO:1,
          EVID:1,      /* Unused */
          ASB:1,
          Arm:1,
          Reserved2:1,
          USB:1,
          GXA:1,
          PAW:1,
          Reserved3:18;
} RST_LPRESET;
typedef RST_LPRESET volatile *LPRST_LPRESET;

typedef struct _RST_LPTIMER
{
  BIT_FLD Value:16,
          Reserved1:16;
} RST_LPTIMER;
typedef RST_LPTIMER volatile *LPRST_LPTIMER;

typedef struct _RST_LOWPOWER
{
  BIT_FLD AudioDec:2,
          VideoDec:2,
          DRMVideo:2,
          GXA:2,
          DRMGraphics:1,
          DRM:1,
          PAW:2,
          HSDP:1,
          EVID:1,
          DAA:1,
          AFE:1,
          I2C:1,
          IR:1,
          UART:2,
          Parallel:1,
          SCard:2,
          USB:1,
          PCI:1,
          UART2:1,
          PWM:1,
          Pulse:1,
          SyncSer:1,
          Reserved1:3;
} RST_LOWPOWER;
typedef RST_LOWPOWER volatile *LPRST_LOWPOWER;
#endif /* BITFIELDS */

#endif /* INCL_RST */


/************************/
/* Interrupt Controller */
/************************/

#ifdef INCL_ITC

/*
******************************************************************************
 *
 *      ITC_INTDEST_REG                                  (ITC_BASE + 0x00)
 *      ITC_INTENABLE_REG                                (ITC_BASE + 0x04)
 *      ITC_INTRIRQ_REG                                  (ITC_BASE + 0x08)
 *      ITC_INTRFIQ_REG                                  (ITC_BASE + 0x0C)
 *      ITC_INTSTATCLR_REG                               (ITC_BASE + 0x10)
 *      ITC_INTSTATSET_REG                               (ITC_BASE + 0x14)
 *
 *****************************************************************************
 */

#define ITC_INTDEST_REG     (ITC_BASE + 0x00)
#define ITC_INTENABLE_REG   (ITC_BASE + 0x04)
#define ITC_INTRIRQ_REG     (ITC_BASE + 0x08)
#define ITC_INTRFIQ_REG     (ITC_BASE + 0x0C)
#define ITC_INTSTATCLR_REG  (ITC_BASE + 0x10)
#define ITC_INTSTATSET_REG  (ITC_BASE + 0x14)
#define ITC_EXPENABLE_REG   (ITC_BASE + 0x20)
#define ITC_EXPSTATCLR_REG  (ITC_BASE + 0x24)
#define ITC_EXPSTATSET_REG  (ITC_BASE + 0x28)

/* Bit position definitions for use when accessing registers as HW_DWORDs */

/* Interrupt status register */

#define ITC_EXPANSION      0x00000001
#define ITC_SYNCSERIAL     0x00000002
#define ITC_DRM            0x00000004
#define ITC_MPEG           0x00000008
#define ITC_RTC            0x00000010
#define ITC_EXTVID         0x00000020
#define ITC_ATAPI          0x00000040
#define ITC_AUDIORX        0x00000080
#define ITC_AUDIOTX        0x00000100
#define ITC_DVT            0x00000200
#define ITC_GIR            0x00000400
#define ITC_UART0          0x00000800
#define ITC_UART1          0x00001000
#define ITC_SCR0           0x00002000
#define ITC_DMA            0x00004000
#define ITC_AFE            0x00008000
#define ITC_SCR1           0x00010000
#define ITC_PULSETIMER     0x00020000
#define ITC_DAA            0x00040000
#define ITC_RESERVED2      0x00080000
#define ITC_I2C            0x00100000
#define ITC_PCI            0x00200000
#define ITC_PAR0           0x00400000
#define ITC_RESERVED3      0x00800000
#define ITC_GXA            0x01000000
#define ITC_UART2          0x02000000
#define ITC_NDSC           0x04000000
#define ITC_PAR2           0x08000000
#define ITC_PAR3           0x10000000
#define ITC_PWM            0x20000000
#define ITC_GPIO           0x40000000
#define ITC_TIMER          0x80000000

/* Bit positions for each interrupt source */
#define ITC_EXPANSION_POS        0
#define ITC_SYNCSERIAL_POS       1
#define ITC_DRM_POS              2
#define ITC_MPEG_POS             3
#define ITC_RTC_POS              4
#define ITC_EXTVID_POS           5
#define ITC_ATAPI_POS            6
#define ITC_AUDIORX_POS          7
#define ITC_AUDIOTX_POS          8
#define ITC_DVT_POS              9
#define ITC_GIR_POS             10
#define ITC_UART0_POS           11
#define ITC_UART1_POS           12
#define ITC_SCR0_POS            13
#define ITC_DMA_POS             14
#define ITC_AFE_POS             15
#define ITC_SCR1_POS            16
#define ITC_PULSETIMER_POS      17
#define ITC_DAA_POS             18
#define ITC_USB_POS             19
#define ITC_I2C_POS             20
#define ITC_PCI_POS             21
#define ITC_PAR0_POS            22
#define ITC_RESERVED2_POS       23
#define ITC_GXA_POS             24
#define ITC_UART2_POS           25
#define ITC_NDSC_POS            26
#define ITC_PAR1_POS            27
#define ITC_PAR2_POS            28
#define ITC_PWM_POS             29
#define ITC_GPIO_POS            30
#define ITC_TIMER_POS           31

#define SYS_TIMER           0
#define WATCHDOG_TIMER      1

/* Expansion register sources and bit positions */
#define ITC_EXPANSION_CMTX     0x00000001
#define ITC_EXPANSION_CMRX     0x00000002
#define ITC_EXPANSION_I2C_0    0x00000004

#define ITC_EXPANSION_CMTX_POS   0
#define ITC_EXPANSION_CMRX_POS   1
#define ITC_EXPANSION_I2C_0_POS  2

#ifdef BITFIELDS
typedef struct _ITC_INTSTATUS
{
  BIT_FLD Expansion:1,
          SyncSerial:1,
          DRM:1,
          MPEG:1,
          RTC:1,
          ExtVideoActive:1,
          ExtVideoVerticalBlank:1,
          AudioRx:1,
          AudioTx:1,
          Parallel:1,
          GIR:1,
          UART0:1,
          UART1:1,
          SCR0:1,
          DMA:1,
          AFE:1,
          SCR1:1,
          Reserved1:3,
          I2C:1,
          PCI:1,
          Reserved2:7,
          PWM:1,
          GPIO:1,
          Timers:1;
} ITC_INTSTATUS;
typedef ITC_INTSTATUS volatile *LPITC_INTSTATUS;
#endif /* BITFIELDS */

#endif /* INCL_ITC */

/**********/
/* Timers */
/**********/

#ifdef INCL_TIM

/* Last updated 3/17/99 */
/*
******************************************************************************
 *
 *     TIM_VALUE_REG                                  (TIM_BASE + 0x00)
 *     TIM_LIMIT_REG                                  (TIM_BASE + 0x04)
 *     TIM_MODE_REG                                   (TIM_BASE + 0x08)
 *     TIM_INT_STATUS_REG                             (TIM_BASE + 0x80)
 *
 *****************************************************************************
 */

#define TIM_CLOCK 54000000

#define TIM_VALUE_REG                                         (TIM_BASE + 0x00)
#define TIM_LIMIT_REG                                         (TIM_BASE + 0x04)
#define TIM_MODE_REG                                          (TIM_BASE + 0x08)
#define     TIM_ENABLE_MASK                                       0x00000001
#define     TIM_ENABLE_SHIFT                                             0
#define     TIM_DONT_WRAP_MASK                                    0x00000002
#define     TIM_DONT_WRAP_SHIFT                                          1
#define     TIM_WATCHDOG_ENABLE_MASK                              0x00000004
#define     TIM_WATCHDOG_ENABLE_SHIFT                                    2
#define     TIM_INT_ENABLE_MASK                                   0x00000008
#define     TIM_INT_ENABLE_SHIFT                                         3
#define TIM_INT_STATUS_REG                                    (TIM_BASE + 0x80)

#define TIMER_NUM_VALUE_REG(x) (TIM_VALUE_REG + ((x) << 4))
#define TIMER_NUM_LIMIT_REG(x) (TIM_LIMIT_REG + ((x) << 4))
#define TIMER_NUM_MODE_REG(x)  (TIM_MODE_REG + ((x) << 4))

#ifdef BITFIELDS
typedef struct _TIM_LIMIT
{
  HW_DWORD Limit;
} TIM_LIMIT;
typedef TIM_LIMIT volatile *LPTIM_LIMIT;

typedef struct _TIM_VALUE
{
  HW_DWORD Value;
} TIM_VALUE;
typedef TIM_VALUE volatile *LPTIM_VALUE;

typedef struct _TIM_MODE
{
  BIT_FLD Enable:1,
          DontWrap:1,
          WatchdogEnable:1,
          IntEnable:1,
          Reserved1:28;
} TIM_MODE;
typedef TIM_MODE volatile *LPTIM_MODE;

#endif /* BITFIELDS */

/* Definitions allowing access to the mode register as a HW_DWORD */

#define TIMER_ENABLE   0x00000001
#define TIMER_DONTWRAP 0x00000002
#define TIMER_WATCHDOG 0x00000004

#endif /* INCL_TIM */

/*******/
/* I2C */
/*******/

#ifdef INCL_I2C

#if IIC_TYPE == IIC_TYPE_COLORADO

/* Last updated with info from 7/1/98 spec. */

/*
******************************************************************************
 *
 *      I2C_CTRL_WRITE_REG                                   (I2C_BASE + 0x00)
 *      I2C_CTRL_READ_REG                                    (I2C_BASE + 0x04)
 *      I2C_MASTER_CTLW_REG                                  (I2C_BASE + 0x08)
 *      I2C_MASTER_CTLR_REG                                  (I2C_BASE + 0x0C)
 *      I2C_SLAVE_CTLW_REG                                   (I2C_BASE + 0x10)
 *      I2C_SLAVE_CTLR_REG                                   (I2C_BASE + 0x14)
 *      I2C_MASTER_DATA_REG                                  (I2C_BASE + 0x18)
 *      I2C_SLAVE_DATA_REG                                   (I2C_BASE + 0x1C)
 *
 *****************************************************************************
 */


#define I2C_CTRL_WRITE_REG                                   (I2C_BASE + 0x00)
#define     I2C_CTRLW_OVRDDCSDA                                  0x00000001
#define     I2C_CTRLW_OVRDDCSCL                                  0x00000002
#define     I2C_CTRLW_OVRI2CSDA                                  0x00000004
#define     I2C_CTRLW_OVRI2CSCL                                  0x00000008
#define     I2C_CTRLW_I2CSLAVE                                   0x00000040
#define     I2C_CTRLW_I2CMASTER                                  0x00000080
#define I2C_CTRL_READ_REG                                    (I2C_BASE + 0x04)
#define     I2C_CTRLR_DDCSDA                                     0x00000001
#define     I2C_CTRLR_DDCSCL                                     0x00000002
#define     I2C_CTRLR_I2CSDA                                     0x00000004
#define     I2C_CTRLR_I2CSCL                                     0x00000008
#define     I2C_CTRLR_SLAVEDONE                                  0x00000010
#define     I2C_CTRLR_MASTERDONE                                 0x00000020
#define     I2C_CTRLR_I2CSLSEL                                   0x00000040
#define     I2C_CTRLR_I2CMASEL                                   0x00000080
#define I2C_MASTER_CTLW_REG                                  (I2C_BASE + 0x08)
#define     I2C_MASTER_CTLW_TRANSMIT                             0x00000001
#define     I2C_MASTER_CTLW_RECEIVE                              0x00000002
#define     I2C_MASTER_CTLW_TRANSACK                             0x00000004
#define     I2C_MASTER_CTLW_INTENABLE                            0x00000008
#define     I2C_MASTER_CTLW_STARTTRANSMIT                        0x00000010
#define     I2C_MASTER_CTLW_STOPTRANSMIT                         0x00000020
#define     I2C_MASTER_CTLW_MODE400KHZ                           0x00000040
#define I2C_MASTER_CTLR_REG                                  (I2C_BASE + 0x0C)
#define     I2C_MASTER_CTLR_BITS                                 0x0000000F
#define     I2C_MASTER_CTLR_MASTER                               0x00000010
#define     I2C_MASTER_CTLR_RECEIVEACK                           0x00000020
#define     I2C_MASTER_CTLR_ARBITRATIONLOST                      0x00000040
#define     I2C_MASTER_CTLR_DONE                                 0x00000080
#define I2C_SLAVE_CTLW_REG                                   (I2C_BASE + 0x10)
#define     I2C_SLAVE_CTLW_BREAKONACK                            0x00000001
#define     I2C_SLAVE_CTLW_BREAKONRECEIVE                        0x00000002
#define     I2C_SLAVE_CTLW_TRANSACK                              0x00000004
#define     I2C_SLAVE_CTLW_INTENABLE                             0x00000008
#define     I2C_SLAVE_CTLW_BREAKONSTART                          0x00000010
#define     I2C_SLAVE_CTLW_BREAKONSTOP                           0x00000020
#define I2C_SLAVE_CTLR_REG                                   (I2C_BASE + 0x14)
#define     I2C_SLAVE_CTLR_BITS                                  0x0000000F
#define     I2C_SLAVE_CTLR_MASTER                                0x00000010
#define     I2C_SLAVE_CTLR_STOP                                  0x00000020
#define     I2C_SLAVE_CTLR_ARBITRATIONLOST                       0x00000040
#define     I2C_SLAVE_CTLR_DONE                                  0x00000080
#define I2C_MASTER_DATA_REG                                  (I2C_BASE + 0x18)
#define I2C_SLAVE_DATA_REG                                   (I2C_BASE + 0x1C)
#define     I2C_DATA_MASK                                        0x000000FF
#define     I2C_DATA_SHIFT                                              0

#ifdef BITFIELDS
typedef struct _I2C_CTRL_WRITE {
   BIT_FLD  OvrDdcSda:1,
            OvrDdcScl:1,
            OvrI2CSda:1,
            OvrI2CScl:1,
            Reserved1:2,
            SelectI2CSlave:1,
            SelectI2CMaster:1,
            Reserved2:24;
} I2C_CTRL_WRITE;
typedef I2C_CTRL_WRITE volatile *LPI2C_CTRL_WRITE;

typedef struct _I2C_CTRL_READ {
   BIT_FLD  DdcSda:1,
            DdcScl:1,
            I2CSda:1,
            I2CScl:1,
            SlaveDone:1,
            MasterDone:1,
            Reserved:26;
} I2C_CTRL_READ;
typedef I2C_CTRL_READ volatile *LPI2C_CTRL_READ;

typedef struct _I2C_MASTER_CTLW {
   BIT_FLD  Transmit:1,
            Receive:1,
            TransAck:1,
            IntEnable:1,
            StartTransmit:1,
            StopTransmit:1,
            Mode400kHz:1,
            Reserved:25;
} I2C_MASTER_CTLW;
typedef I2C_MASTER_CTLW volatile *LPI2C_MASTER_CTLW;

typedef struct _I2C_MASTER_CTLR {
   BIT_FLD  bits:4,
            Master:1,
            ReceiveAck:1,
            ArbitrationLost:1,
            Done:1,
            Reserved:24;
} I2C_MASTER_CTLR;
typedef I2C_MASTER_CTLR volatile *LPI2C_MASTER_CTLR;

typedef struct _I2C_SLAVE_CTLW {
   BIT_FLD  BreakOnAck:1,
            BreakOnReceive:1,
            TransAck:1,
            IntEnable:1,
            BreakOnStart:1,
            BreakOnStop:1,
            Reserved:26;
} I2C_SLAVE_CTLW;
typedef I2C_SLAVE_CTLW volatile *LPI2C_SLAVE_CTLW;

typedef struct _I2C_SLAVE_CTLR {
   BIT_FLD  bits:4,
            Master:1,
            Stop:1,
            ArbitrationLost:1,
            Done:1,
            Reserved:24;
} I2C_SLAVE_CTLR;
typedef I2C_SLAVE_CTLR volatile *LPI2C_SLAVE_CTLR;

typedef struct _I2C_DATA {
   BIT_FLD  Data:8,
            Reserved:24;
} I2C_DATA;
typedef I2C_DATA volatile *LPI2C_DATA;

typedef I2C_DATA         I2C_MASTER_DATA;
typedef LPI2C_DATA       LPI2C_MASTER_DATA;
typedef I2C_DATA         I2C_SLAVE_DATA;
typedef LPI2C_DATA       LPI2C_SLAVE_DATA;
#endif /* BITFIELDS */

#else /* IIC_TYPE == IIC_TYPE_COLORADO */

/* New IIC Interface - from Wabash, on */
/******************************************************************************
 *
 *       CNXT_IIC_MODE_REG                            (I2C_BASE + 0x00)
 *       CNXT_IIC_CTRL_REG                            (I2C_BASE + 0x04)
 *       CNXT_IIC_STAT_REG                            (I2C_BASE + 0x08)
 *       CNXT_IIC_RDATA_REG                           (I2C_BASE + 0x0C)
 *
 *****************************************************************************
 */
#define  CNXT_IIC_MODE_REG                               (I2C_BASE + 0x00)
#define     CNXT_IIC_MODE_SCL_OVRIDE_MASK                0x00000001
#define     CNXT_IIC_MODE_SCL_OVRIDE_SHIFT               0
#define     CNXT_IIC_MODE_SDA_OVRIDE_MASK                0x00000002
#define     CNXT_IIC_MODE_SDA_OVRIDE_SHIFT               1
#define     CNXT_IIC_MODE_HWCTRL_EN_MASK                 0x00000004
#define     CNXT_IIC_MODE_HWCTRL_EN_SHIFT                2
#define     CNXT_IIC_MODE_WAITST_EN_MASK                 0x00000008
#define     CNXT_IIC_MODE_WAITST_EN_SHIFT                3
#define     CNXT_IIC_MODE_MULTIMAST_EN_MASK              0x00000010
#define     CNXT_IIC_MODE_MULTIMAST_EN_SHIFT             4
#define     CNXT_IIC_MODE_CLKDIV_VALUE_MASK              0x0003FF00
#define     CNXT_IIC_MODE_CLKDIV_VALUE_SHIFT             8

#define  CNXT_IIC_CTRL_REG                               (I2C_BASE + 0x04)
#define     CNXT_IIC_CTRL_NUMBYTES_MASK                  0x00000003
#define     CNXT_IIC_CTRL_NUMBYTES_SHIFT                 0
#define     CNXT_IIC_CTRL_READACK_MASK                   0x00000004
#define     CNXT_IIC_CTRL_READACK_SHIFT                  2
#define     CNXT_IIC_CTRL_START_MASK                     0x00000010
#define     CNXT_IIC_CTRL_START_SHIFT                    4
#define     CNXT_IIC_CTRL_STOP_MASK                      0x00000020
#define     CNXT_IIC_CTRL_STOP_SHIFT                     5
#define     CNXT_IIC_CTRL_WRITEDATA_INST_MASK            0xFF
#define     CNXT_IIC_CTRL_WRITEDATA_INST_WIDTH           8
#define     CNXT_IIC_CTRL_WRITEDATA_NUM_INST             3
#define     CNXT_IIC_CTRL_MAX_NUM_WRITE                  3
#define     CNXT_IIC_CTRL_MAX_NUM_READ                   4

#define  CNXT_IIC_STAT_REG                               (I2C_BASE + 0x08)
#define     CNXT_IIC_STAT_INT_MASK                       0x00000001
#define     CNXT_IIC_STAT_INT_SHIFT                      0
#define     CNXT_IIC_STAT_WRITEACK_MASK                  0x00000002
#define     CNXT_IIC_STAT_WRITEACK_SHIFT                 1
#define     CNXT_IIC_STAT_SCL_MASK                       0x00000004
#define     CNXT_IIC_STAT_SCL_SHIFT                      2
#define     CNXT_IIC_STAT_SDA_MASK                       0x00000008
#define     CNXT_IIC_STAT_SDA_SHIFT                      3

#define  CNXT_IIC_READDATA_REG                           (I2C_BASE + 0x0C)
#define     CNXT_IIC_READDATA_INST_MASK                  0xFF
#define     CNXT_IIC_READDATA_INST_WIDTH                 8
#define     CNXT_IIC_READDATA_NUM_INST                   4

#define CNXT_IIC_MODE_REG_ADDR(iBank)  (I2C_BASE + iBank*IIC_BANK_SIZE + 0x00)
#define CNXT_IIC_CTRL_REG_ADDR(iBank)  (I2C_BASE + iBank*IIC_BANK_SIZE + 0x04)
#define CNXT_IIC_STAT_REG_ADDR(iBank)  (I2C_BASE + iBank*IIC_BANK_SIZE + 0x08)
#define CNXT_IIC_RDATA_REG_ADDR(iBank) (I2C_BASE + iBank*IIC_BANK_SIZE + 0x0C)

#endif /* IIC_TYPE == IIC_TYPE_COLORADO */

#endif /* INCL_I2C */

/**********************/
/* General Purpose IO */
/**********************/

/* Last updated 3/1/99 */

#ifdef INCL_GPI

#define GPI_READ_REG         (GPI_BASE + 0x04)
#define GPI_DRIVE_HIGH_REG   (GPI_BASE + 0x08)
#define GPI_DRIVE_LOW_REG    (GPI_BASE + 0x10)
#define GPI_DRIVE_OFF_REG    (GPI_BASE + 0x14)
#define GPI_INT_REG          (GPI_BASE + 0x20)
#define GPI_INT_ENABLE_REG   (GPI_BASE + 0x24)
#define GPI_POS_EDGE_REG     (GPI_BASE + 0x28)
#define GPI_NEG_EDGE_REG     (GPI_BASE + 0x2C)
#define GPI_LEVEL_REG        (GPI_BASE + 0x30)

#define GPI_READ_REG_B2       (GPI_BASE + 0x34)
#define GPI_DRIVE_HIGH_REG_B2 (GPI_BASE + 0x38)
#define GPI_DRIVE_LOW_REG_B2  (GPI_BASE + 0x40)
#define GPI_DRIVE_OFF_REG_B2  (GPI_BASE + 0x44)
#define GPI_INT_REG_B2        (GPI_BASE + 0x50)
#define GPI_INT_ENABLE_REG_B2 (GPI_BASE + 0x54)
#define GPI_POS_EDGE_REG_B2   (GPI_BASE + 0x58)
#define GPI_NEG_EDGE_REG_B2   (GPI_BASE + 0x5C)
#define GPI_LEVEL_REG_B2      (GPI_BASE + 0x60)

#define GPI_READ_REG_B3       (GPI_BASE + 0x64)
#define GPI_DRIVE_HIGH_REG_B3 (GPI_BASE + 0x68)
#define GPI_DRIVE_LOW_REG_B3  (GPI_BASE + 0x70)
#define GPI_DRIVE_OFF_REG_B3  (GPI_BASE + 0x74)
#define GPI_INT_REG_B3        (GPI_BASE + 0x80)
#define GPI_INT_ENABLE_REG_B3 (GPI_BASE + 0x84)
#define GPI_POS_EDGE_REG_B3   (GPI_BASE + 0x88)
#define GPI_NEG_EDGE_REG_B3   (GPI_BASE + 0x8C)
#define GPI_LEVEL_REG_B3      (GPI_BASE + 0x90)

/* Useful GPIO access macros */

/* For first "real" bank of GPIOs only */
#define MAKE_GPIO_INPUT(x)   *(LPREG)(GPI_DRIVE_OFF_REG) |= (1<<(x))
#define DRIVE_GPIO_HIGH(x)   *(LPREG)(GPI_DRIVE_HIGH_REG) = (1<<(x))
#define DRIVE_GPIO_LOW(x)    *(LPREG)(GPI_DRIVE_LOW_REG) = (1<<(x))

#define SET_GPIO_PIN(x, y) if(y)                                         \
                             *(LPREG)(GPI_DRIVE_HIGH_REG) = (1<<(x));    \
                           else                                          \
                             *(LPREG)(GPI_DRIVE_LOW_REG) = (1<<(x));
#define GET_GPIO_PIN(x)      (*(LPREG)(GPI_READ_REG) >> x) & 0x1

#define POS_EDGE 1
#define NEG_EDGE 2
#define BOTH_EDGES (POS_EDGE | NEG_EDGE)

#define SET_GPIO_INT_EDGE(x, y) {                                                                         \
                                  HW_BOOL ksg;                                                       \
                                  ksg = critical_section_begin();                                                 \
                                  if((y) & POS_EDGE)                                                      \
                                    *(LPREG)(GPI_POS_EDGE_REG) = *(LPREG)(GPI_POS_EDGE_REG) | (1<<(x));   \
                                  else                                                                    \
                                    *(LPREG)(GPI_POS_EDGE_REG) = *(LPREG)(GPI_POS_EDGE_REG) & ~(1<<(x));  \
                                  if((y) & NEG_EDGE)                                                      \
                                    *(LPREG)(GPI_NEG_EDGE_REG) = *(LPREG)(GPI_NEG_EDGE_REG) | (1<<(x));   \
                                  else                                                                    \
                                    *(LPREG)(GPI_NEG_EDGE_REG) = *(LPREG)(GPI_NEG_EDGE_REG) & ~(1<<(x));  \
                                  critical_section_end(ksg);                                                     \
                                }

/* Similar macros for Neches whick allow access to any of 3 GPIO banks */
#define MAKE_GPIO_INPUT_BANK(b, x)   *(LPREG)(GPI_DRIVE_OFF_REG + ((b)*GPI_BANK_SIZE)) |= (1<<(x))
#define DRIVE_GPIO_HIGH_BANK(b, x)   *(LPREG)(GPI_DRIVE_HIGH_REG + ((b)*GPI_BANK_SIZE)) = (1<<(x))
#define DRIVE_GPIO_LOW_BANK(b, x)    *(LPREG)(GPI_DRIVE_LOW_REG + ((b)*GPI_BANK_SIZE)) = (1<<(x))

#define SET_GPIO_PIN_BANK(b, x, y)                                                          \
                           if(y)                                                            \
                             *(LPREG)(GPI_DRIVE_HIGH_REG + ((b)*GPI_BANK_SIZE)) = (1<<(x)); \
                           else                                                             \
                             *(LPREG)(GPI_DRIVE_LOW_REG + ((b)*GPI_BANK_SIZE)) = (1<<(x));

#define GET_GPIO_PIN_BANK(b, x)      ((*(LPREG)(GPI_READ_REG + ((b)*GPI_BANK_SIZE)) >> x) & 0x1)

#define SET_GPIO_INT_EDGE_BANK(b, x, y)                                                          \
                                 {                                                               \
                                   HW_BOOL ksg;                                                     \
                                   LPREG        lpEdgeReg;                                       \
                                                                                                 \
                                   ksg = critical_section_begin();                               \
                                                                                                 \
                                   lpEdgeReg = (LPREG)(GPI_POS_EDGE_REG + ((b)*GPI_BANK_SIZE));  \
                                   if((y) & POS_EDGE)                                            \
                                     *lpEdgeReg = *lpEdgeReg | (1<<(x));                         \
                                   else                                                          \
                                     *lpEdgeReg = *lpEdgeReg & ~(1<<(x));                        \
                                                                                                 \
                                   lpEdgeReg = (LPREG)(GPI_NEG_EDGE_REG + ((b)*GPI_BANK_SIZE));  \
                                   if((y) & NEG_EDGE)                                            \
                                     *lpEdgeReg = *lpEdgeReg | (1<<(x));                         \
                                   else                                                          \
                                     *lpEdgeReg = *lpEdgeReg & ~(1<<(x));                        \
                                                                                                 \
                                   critical_section_end(ksg);                                    \
                                 }

#define CLEAR_GPIO_INT_BANK(b, x) *(LPREG)(GPI_INT_REG + ((b)*GPI_BANK_SIZE)) = (1<<(x))

/********************/
/* PIN GPIO MUX REG */
/********************/
#define GPIO_MUX_UT1_ENBL            0x00020000
#define GPIO_MUX_UT2_PRI_ENBL        0x00040000
#define GPIO_MUX_UT2_SEC_ENBL        0x00080000
#define GPIO_MUX_UT2_TER_ENBL        0x04000000
#define GPIO_MUX_UT3_ENBL            0x00100000
#define GPIO_MUX0_UT1_ENBL           0x00000006
#define GPIO_MUX0_UT2_PRI_ENBL       0x00000018
#define GPIO_MUX2_UT2_SEC_ENBL       0x00000180
#define GPIO_MUX0_UT2_TER_ENBL       0x00000800
#define GPIO_MUX2_UT2_TER_ENBL       0x00000200
#define GPIO_MUX0_UT3_ENBL           0x0000C000
#define PIN_ALT_FUNC_UT2_PRI_ENBL    0x00000000
#define PIN_ALT_FUNC_UT2_SEC_ENBL    0x00000010
#define PIN_ALT_FUNC_UT2_TER_ENBL    0x00000020

#endif /* INCL_GPI */

/************/
/* IEEE1284 */
/************/

/* Last updated 3/8/99 */

#ifdef INCL_PAR

/*
******************************************************************************
 *
 *     PAR_CFG_REG                                           (PAR_BASE + 0x00)
 *     PAR_PIN_IO_CFG_REG                                    (PAR_BASE + 0x04)
 *     PAR_PIN_IO_CTL_REG                                    (PAR_BASE + 0x08)
 *     PAR_PIN_IO_STAT_REG                                   (PAR_BASE + 0x0C)
 *     PAR_INT_ENABLE_REG                                    (PAR_BASE + 0x10)
 *     PAR_INT_STAT_REG                                      (PAR_BASE + 0x14)
 *     PAR_FIFO_CTL_REG                                      (PAR_BASE + 0x18)
 *     PAR_FIFO_STAT_REG                                     (PAR_BASE + 0x1C)
 *     PAR_EPP_ADDR_REG                                      (PAR_BASE + 0x20)
 *     PAR_PULSE_CTL_REG                                     (PAR_BASE + 0x24)
 *     PAR_TIMEOUT_REG                                       (PAR_BASE + 0x28)
 *     PAR_TIMER_REG                                         (PAR_BASE + 0x2C)
 *     PAR_DBG_STAT_REG                                      (PAR_BASE + 0x30)
 *     PAR_DATA_PORT_REG                                     (PAR_BASE + 0x40)
 *
 *****************************************************************************
 */

#define PAR_CFG_REG                                          (PAR_BASE + 0x00)
#define PAR_PIN_IO_CFG_REG                                   (PAR_BASE + 0x04)
#define PAR_PIN_IO_CTL_REG                                   (PAR_BASE + 0x08)
#define PAR_PIN_IO_STAT_REG                                  (PAR_BASE + 0x0C)
#define PAR_INT_ENABLE_REG                                   (PAR_BASE + 0x10)
#define PAR_INT_STAT_REG                                     (PAR_BASE + 0x14)
#define PAR_FIFO_CTL_REG                                     (PAR_BASE + 0x18)
#define PAR_FIFO_STAT_REG                                    (PAR_BASE + 0x1C)
#define PAR_EPP_ADDR_REG                                     (PAR_BASE + 0x20)
#define PAR_PULSE_CTL_REG                                    (PAR_BASE + 0x24)
#define PAR_TIMEOUT_REG                                      (PAR_BASE + 0x28)
#define PAR_TIMER_REG                                        (PAR_BASE + 0x2C)
#define PAR_DBG_STAT_REG                                     (PAR_BASE + 0x30)
#define PAR_DATA_PORT_REG                                    (PAR_BASE + 0x40)

#define PAR_COMPAT_MODE    0x7
#define PAR_NIBBLE_MODE    0x0
#define PAR_BYTE_MODE      0x1
#define PAR_ECP_MODE       0x2
#define PAR_ECP_RLE_MODE   0x3
#define PAR_EPP_MODE       0x4

#ifdef BITFIELDS
typedef struct _PAR_CONFIG
{
  BIT_FLD ModeSelect:3,
          ReqDeviceId:1,
          CurNegotiatedMode:3,
          Reserved1:5,
          HwHandshakeEn:1,
          AutoNegotiateEn:1,
          TransferDir:1,
          FifoEn:1,
          TimerEn:1,
          DMAEn:1,
          Reserved2:14;
} PAR_CONFIG;
typedef PAR_CONFIG volatile *LPPAR_CONFIG;

typedef struct _PAR_PIN_IO_CONFIG
{
  BIT_FLD PError:1,
          nFault:1,
          nAck:1,
          Select:1,
          Busy:1,
          nInit:1,
          nStrobe:1,
          nSelectIn:1,
          nAutoFd:1,
          Reserved1:15,
          Data:8;
} PAR_PIN_IO_CONFIG;
typedef PAR_PIN_IO_CONFIG volatile *LPPAR_PIN_IO_CONFIG;

typedef struct _PAR_PIN_IO_CONTROL
{
  BIT_FLD PError:1,
          nFault:1,
          nAck:1,
          Select:1,
          Busy:1,
          nInitEn:1,
          nStrobeEn:1,
          nSelectInEn:1,
          nAutoFdEn:1,
          Reserved1:15,
          Data:8;
} PAR_PIN_IO_CONTROL;
typedef PAR_PIN_IO_CONTROL volatile *LPPAR_PIN_IO_CONTROL;

typedef struct _PAR_PIN_IO_STATUS
{
  BIT_FLD PError:1,
          nFault:1,
          nAck:1,
          Select:1,
          Busy:1,
          nInit:1,
          nStrobe:1,
          nSelectIn:1,
          nAutoFd:1,
          Reserved1:15,
          Data:8;
} PAR_PIN_IO_STATUS;
typedef PAR_PIN_IO_STATUS volatile *LPPAR_PIN_IO_STATUS;

typedef struct _PAR_INT_ENABLE
{
  BIT_FLD EnTxEmpty:1,
          EnTxFifo:1,
          EnRxFifo:1,
          EnNotIEEE:1,
          EnNegFail:1,
          EnNegOK:1,
          EnEPP:1,
          EnECP:1,
          EnStatChange:1,
          EnTimeout:1,
          Reserved:22;
} PAR_INT_ENABLE;
typedef PAR_INT_ENABLE volatile *LPPAR_INT_ENABLE;

typedef struct _PAR_INT_STATUS
{
  BIT_FLD TxEmpty:1,
          TxFifo:1,
          RxFifo:1,
          NotIEEE:1,
          NegFail:1,
          NegOK:1,
          EPP:1,
          ECP:1,
          StatChange:1,
          Timeout:1,
          Reserved:22;
} PAR_INT_STATUS;
typedef PAR_INT_STATUS volatile *LPPAR_INT_STATUS;

typedef struct _PAR_FIFO_CONTROL
{
  BIT_FLD FwdFifoLvl:8,
          Reserved1:8,
          RevFifoLvl:8,
          Reserved2:8;
} PAR_FIFO_CONTROL;
typedef PAR_FIFO_CONTROL volatile *LPPAR_FIFO_CONTROL;

typedef struct _PAR_FIFO_STATUS
{
  BIT_FLD FwdFifoLvl:8,
          Reserved:24;
} PAR_FIFO_STATUS;
typedef PAR_FIFO_STATUS volatile *LPPAR_FIFO_STATUS;

typedef struct _PAR_EPP_ADDRESS
{
  BIT_FLD EPPFwdAddr:8,
          Reserved:24;
} PAR_EPP_ADDRESS;
typedef PAR_EPP_ADDRESS volatile *LPPAR_EPP_ADDRESS;

typedef struct _PAR_PULSE_CONTROL
{
  BIT_FLD NumPulseClocks:8,
          Reserved:24;
} PAR_PULSE_CONTROL;
typedef PAR_PULSE_CONTROL volatile *LPPAR_PULSE_CONTROL;

typedef struct _PAR_TIMEOUT_VAL
{
  BIT_FLD TimeoutVal:8,
          Reserved:24;
} PAR_TIMEOUT_VAL;
typedef PAR_TIMEOUT_VAL volatile *LPPAR_TIMEOUT;

typedef struct _PAR_TIMER_STATUS
{
  BIT_FLD TimerVal:8,
          Reserved:24;
} PAR_TIMER_STATUS;
typedef PAR_TIMER_STATUS volatile *LPPAR_TIMER_STATUS;

typedef struct _PAR_DEBUG_STATUS
{
  BIT_FLD CompatSM:4,
          NibbleSM:4,
          ByteSM:4,
          EPPSM:4,
          ECPSM:8,
          NegotiationSM:8;
} PAR_DEBUG_STATUS;
typedef PAR_DEBUG_STATUS volatile *LPPAR_DEBUG_STATUS;

typedef struct _PAR_DATA_PORT
{
  BIT_FLD TimerVal:8,
          Reserved:24;
} PAR_DATA_PORT;
typedef PAR_DATA_PORT volatile *LPPAR_DATA_PORT;
#endif /* BITFIELDS */

#endif /* INCL_PAR */

/*************/
/* Infra Red */
/*************/

/* Last updated 3/1/99 */

#ifdef INCL_IRD

/*
******************************************************************************
 *
 *     IRD_CONTROL_REG                                       (IRD_BASE + 0x00)
 *     IRD_TX_CLK_CONTROL_REG                                (IRD_BASE + 0x04)
 *     IRD_RX_CLK_CONTROL_REG                                (IRD_BASE + 0x08)
 *     IRD_TX_CARRIER_REG                                    (IRD_BASE + 0x0C)
 *     IRD_STATUS_REG                                        (IRD_BASE + 0x10)
 *     IRD_INT_ENABLE_REG                                    (IRD_BASE + 0x14)
 *     IRD_LOWPASS_FILTER_REG                                (IRD_BASE + 0x18)
 *     IRD_TEST_CONTROL_REG                                  (IRD_BASE + 0x20)
 *     IRD_TEST_TXCLK_REG                                    (IRD_BASE + 0x24)
 *     IRD_TEST_RXCLK_REG                                    (IRD_BASE + 0x28)
 *     IRD_DATA_BASE                                         (IRD_BASE + 0x40)
 *
 *****************************************************************************
 */

#define IRD_CONTROL_REG                                      (IRD_BASE + 0x00)
#define     IRD_CONTROL_WINDOW_CTRL_MASK                         0x00000003
#define     IRD_CONTROL_WINDOW_CTRL_SHIFT                               0
#define     IRD_CONTROL_EDGE_CTRL_MASK                           0x0000000C
#define     IRD_CONTROL_EDGE_CTRL_SHIFT                                 2
#define     IRD_CONTROL_ENBL_DEMOD_MASK                          0x00000010
#define     IRD_CONTROL_ENBL_DEMOD_SHIFT                                4
#define     IRD_CONTROL_ENBL_MOD_MASK                            0x00000020
#define     IRD_CONTROL_ENBL_MOD_SHIFT                                  5
#define     IRD_CONTROL_ENBL_RXFIFO_MASK                         0x00000040
#define     IRD_CONTROL_ENBL_RXFIFO_SHIFT                               6
#define     IRD_CONTROL_ENBL_TXFIFO_MASK                         0x00000080
#define     IRD_CONTROL_ENBL_TXFIFO_SHIFT                               7
#define     IRD_CONTROL_ENBL_RX_MASK                             0x00000100
#define     IRD_CONTROL_ENBL_RX_SHIFT                                   8
#define     IRD_CONTROL_ENBL_TX_MASK                             0x00000200
#define     IRD_CONTROL_ENBL_TX_SHIFT                                   9
#define     IRD_CONTROL_RXINT_CTRL_MASK                          0x00000400
#define     IRD_CONTROL_RXINT_CTRL_SHIFT                               10
#define     IRD_CONTROL_TXINT_CTRL_MASK                          0x00000800
#define     IRD_CONTROL_TXINT_CTRL_SHIFT                               11
#define     IRD_CONTROL_CARRIER_POLARITY_MASK                    0x00001000
#define     IRD_CONTROL_CARRIER_POLARITY_SHIFT                         12
#define     IRD_CONTROL_LOOPBACK_MASK                            0x00002000
#define     IRD_CONTROL_LOOPBACK_SHIFT                                 13
#define IRD_TX_CLK_CONTROL_REG                               (IRD_BASE + 0x04)
#define IRD_RX_CLK_CONTROL_REG                               (IRD_BASE + 0x08)
#define IRD_TX_CARRIER_REG                                   (IRD_BASE + 0x0C)
#define IRD_STATUS_REG                                       (IRD_BASE + 0x10)
#define     IRD_STATUS_RXTIMEOUT_MASK                            0x00000001
#define     IRD_STATUS_RXTIMEOUT_SHIFT                                  0
#define     IRD_STATUS_RXOVERRUN_MASK                            0x00000002
#define     IRD_STATUS_RXOVERRUN_SHIFT                                  1
#define     IRD_STATUS_RXBUSY_MASK                               0x00000004
#define     IRD_STATUS_RXBUSY_SHIFT                                     2
#define     IRD_STATUS_TRBUSY_MASK                               0x00000008
#define     IRD_STATUS_TRBUSY_SHIFT                                     3
#define     IRD_STATUS_RXFIFO_REQ_MASK                           0x00000010
#define     IRD_STATUS_RXFIFO_REQ_SHIFT                                 4
#define     IRD_STATUS_TXFIFO_REQ_MASK                           0x00000020
#define     IRD_STATUS_TXFIFO_REQ_SHIFT                                 5
#define IRD_INT_ENABLE_REG                                   (IRD_BASE + 0x14)
#define     IRD_INT_ENABLE_RXPULSE_WIDTH_TIMEOUT_MASK            0x00000001
#define     IRD_INT_ENABLE_RXPULSE_WIDTH_TIMEOUT_SHIFT                  0
#define     IRD_INT_ENABLE_RXFIFO_OVERRUN_MASK                   0x00000002
#define     IRD_INT_ENABLE_RXFIFO_OVERRUN_SHIFT                         1
#define     IRD_INT_ENABLE_RXFIFO_MASK                           0x00000010
#define     IRD_INT_ENABLE_RXFIFO_SHIFT                                 4
#define     IRD_INT_ENABLE_TXFIFO_MASK                           0x00000020
#define     IRD_INT_ENABLE_TXFIFO_SHIFT                                 5
#define IRD_LOWPASS_FILTER_REG                               (IRD_BASE + 0x18)
#define IRD_TEST_CONTROL_REG                                 (IRD_BASE + 0x20)
#define IRD_TEST_TXCLK_REG                                   (IRD_BASE + 0x24)
#define IRD_TEST_RXCLK_REG                                   (IRD_BASE + 0x28)
#define IRD_DATA_BASE                                        (IRD_BASE + 0x40)

/* For WindowCtrl */
#define IRD_WINDOW_33 0
#define IRD_WINDOW_34 1
#define IRD_WINDOW_43 2
#define IRD_WINDOW_44 3

/* For EdgeCtrl */
#define IRD_EDGE_DISABLED 0
#define IRD_EDGE_FALLING  1
#define IRD_EDGE_RISING   2
#define IRD_EDGE_ANY      3

/* For TxIntCtrl */
#define IRD_TXINT_HALF_FULL 0
#define IRD_TXINT_NOT_BUSY  1

/* For RxIntCtrl */
#define IRD_RXINT_HALF_FULL 0
#define IRD_RXINT_NOT_EMPTY 1

/* Absolute values for writing the whole register at once */
#define IRD_EDGE_CTRL_SHIFT 2
#define IRD_EDGE_CTRL_MASK  0x00000006
#define IRD_ENABLE_DEMOD    0x00000010
#define IRD_ENABLE_MOD      0x00000020
#define IRD_ENABLE_RX_FIFO  0x00000040
#define IRD_ENABLE_TX_FIFO  0x00000080
#define IRD_ENABLE_RX       0x00000100
#define IRD_ENABLE_TX       0x00000200
#define IRD_RX_INT_CTRL     0x00000400
#define IRD_TX_INT_CTRL     0x00000800
#define IRD_CARRIER_POL     0x00001000
#define IRD_LOOPBACK        0x00002000

/* Absolute values for writing the whole register at once */
#define IRD_RX_TIMEOUT 0x00000001
#define IRD_RX_OVERRUN 0x00000002
#define IRD_RX_BUSY    0x00000004
#define IRD_TX_BUSY    0x00000008
#define IRD_RX_EMPTY   0x00000010
#define IRD_TX_FULL    0x00000020

/* Absolute values for writing the whole register at once */
#define IRD_INT_PLSTIMEOUT 0x00000001
#define IRD_INT_OVERRUN    0x00000002
#define IRD_INT_RX_FIFO    0x00000010
#define IRD_INT_TX_FIFO    0x00000020

#define IRD_FILTER_DISABLE 0

/* Absolute values for writing the whole register at once */
#define IRD_DATA_MASK  0x0000FFFF
#define IRD_BIT_LEVEL  0x00010000
#define IRD_DATA_AVAIL 0x00020000


#ifdef BITFIELDS
typedef struct _IRD_CONTROL
{
  BIT_FLD WindowCtrl:2,
        EdgeCtrl:2,
        EnableDemod:1,
        EnableMod:1,
        EnableRxFifo:1,
        EnableTxFifo:1,
        EnableRx:1,
        EnableTx:1,
        RxIntCtrl:1,
        TxIntCtrl:1,
        CarrierPolarity:1,
        LoopBack:1,
        Reserved2:18;
} IRD_CONTROL;
typedef IRD_CONTROL volatile *LPIRD_CONTROL;

typedef struct _IRD_TX_CLK_CONTROL
{
  BIT_FLD Divisor:16,
        Reserved1:16;
} IRD_TX_CLK_CONTROL;
typedef IRD_TX_CLK_CONTROL volatile *LPIRD_TX_CLK_CONTROL;

typedef IRD_TX_CLK_CONTROL IRD_RX_CLK_CONTROL;
typedef LPIRD_TX_CLK_CONTROL LPIRD_RX_CLK_CONTROL;

typedef struct _IRD_TX_CARRIER
{
  BIT_FLD HighTime:4,
        Reserved1:28;
} IRD_TX_CARRIER;
typedef IRD_TX_CARRIER volatile *LPIRD_TX_CARRIER;

typedef struct _IRD_STATUS
{
  BIT_FLD RxTimeout:1,
          RxOverrun:1,
          RxBusy:1,
          TrBusy:1,
          RxFIFORequest:1,
          TxFIFORequest:1,
          Reserved1:26;
} IRD_STATUS;
typedef IRD_STATUS volatile *LPIRD_STATUS;
/* NB: TxBusy is a flag used by pSOS, hence use of TrBusy here */

typedef struct _IRD_INT_ENABLE
{
  BIT_FLD RxPulswWidthTimeout:1,
          RxFifoOverrun:1,
          Reserved1:2,
          RxFifo:1,
          TxFifo:1,
          Reserved2:26;
} IRD_INT_ENABLE;
typedef IRD_INT_ENABLE volatile *LPIRD_INT_ENABLE;

typedef struct _IRD_LOWPASS_FILTER
{
  BIT_FLD Value:16,
          Reserved1:16;
} IRD_LOWPASS_FILTER;
typedef IRD_LOWPASS_FILTER volatile *LPIRD_LOWPASS_FILTER;

typedef struct _IRD_TEST_CONTROL
{
  BIT_FLD UseTestSigs:1,
        TestStatus:2,
        RegGirClk:1,
        TestBypass:1,
        RegRxSerial:1,
        QuickCount:1,
        TxInt:1,
        RxInt:1,
        SerialOut:1,
        TxClock:1,
        RxClock:1,
        Reserved1:20;
} IRD_TEST_CONTROL;
typedef IRD_TEST_CONTROL volatile *LPIRD_TEST_CONTROL;

typedef IRD_TX_CLK_CONTROL   IRD_TEST_RXCLK;
typedef IRD_TX_CLK_CONTROL   IRD_TEST_TXCLK;
typedef LPIRD_TX_CLK_CONTROL LPIRD_TEST_RXCLK;
typedef LPIRD_TX_CLK_CONTROL LPIRD_TEST_TXCLK;
#endif /* BITFIELDS */

#endif /* INCL_IRD */

/********/
/* UART */
/********/

#ifdef INCL_SER

/* Last updated 3/1/99 */

/*
******************************************************************************
 *
 *     SER_DATA_REG                                                       0x00
 *     SER_BRD_LOW_REG                                                    0x00
 *     SER_INT_ENABLE_REG                                                 0x04
 *     SER_BRD_HIGH_REG                                                   0x04
 *     SER_FIFO_CTRL_REG                                                  0x08
 *     SER_FRAME_CTRL_REG                                                 0x0C
 *     SER_STATUS_REG                                                     0x14
 *     SER_IRLVL_REG                                                      0x18
 *     SER_IRDA_CTRL_REG                                                  0x20
 *     SER_TX_FIFO_STATUS_REG                                             0x28
 *     SER_RX_FIFO_STATUS_REG                                             0x2c
 *     SER_BRD_FRACTIONAL_REG                                             0x30
 *
 *****************************************************************************
 */


#define SER_NUM_PORTS               3
#define SER_FIFO_DEPTH              16

#define SER_DATA_REG                                   0x00000000
#define     SER_DATA_DATA_MASK                         0x000000FF
#define     SER_DATA_DATA_SHIFT                               0
#define SER_BRD_LOW_REG                                0x00000000
#define     SER_BRD_LOW_VALUE_MASK                     0x000000FF
#define     SER_BRD_LOW_VALUE_SHIFT                           0
#define SER_INT_ENABLE_REG                             0x00000004
#define     SER_INT_ENABLE_RXFIFO_MASK                 0x00000001
#define     SER_INT_ENABLE_RXFIFO_SHIFT                       0  
#define     SER_INT_ENABLE_RXOVERRUN_MASK              0x00000002
#define     SER_INT_ENABLE_RXOVERRUN_SHIFT                    1  
#define     SER_INT_ENABLE_PARITYERROR_MASK            0x00000004
#define     SER_INT_ENABLE_PARITYERROR_SHIFT                  2  
#define     SER_INT_ENABLE_FRAMINGERROR_MASK           0x00000008
#define     SER_INT_ENABLE_FRAMINGERROR_SHIFT                 3  
#define     SER_INT_ENABLE_BREAKRECEIVED_MASK          0x00000010
#define     SER_INT_ENABLE_BREAKRECEIVED_SHIFT                4  
#define     SER_INT_ENABLE_TXFIFO_MASK                 0x00000020
#define     SER_INT_ENABLE_TXFIFO_SHIFT                       5  
#define     SER_INT_ENABLE_TXIDLE_MASK                 0x00000040
#define     SER_INT_ENABLE_TXIDLE_SHIFT                       6  
#define     SER_INT_ENABLE_RXFIFOERROR_MASK            0x00000080
#define     SER_INT_ENABLE_RXFIFOERROR_SHIFT                  7  
#define SER_BRD_HIGH_REG                               0x00000004
#define     SER_BRD_HIGH_VALUE_MASK                    0x000000FF
#define     SER_BRD_HIGH_VALUE_SHIFT                          0
#define SER_FIFO_CTRL_REG                              0x00000008
#define     SER_FIFO_CTRL_RXFIFOCLEAR_MASK             0x00000002
#define     SER_FIFO_CTRL_RXFIFOCLEAR_SHIFT                   1
#define     SER_FIFO_CTRL_TXFIFOCLEAR_MASK             0x00000004
#define     SER_FIFO_CTRL_TXFIFOCLEAR_SHIFT                   2
#define     SER_FIFO_CTRL_TXFIFOTHRESHOLD_MASK         0x00000030
#define     SER_FIFO_CTRL_TXFIFOTHRESHOLD_SHIFT               4
#define     SER_FIFO_CTRL_RXFIFOTHRESHOLD_MASK         0x000000C0
#define     SER_FIFO_CTRL_RXFIFOTHRESHOLD_SHIFT               6
#define SER_FRAME_CTRL_REG                             0x0000000C
#define     SER_FRAME_CTRL_DATA8BIT_MASK               0x00000001
#define     SER_FRAME_CTRL_DATA8BIT_SHIFT                     0  
#define     SER_FRAME_CTRL_LOOPBACK_MASK               0x00000002
#define     SER_FRAME_CTRL_LOOPBACK_SHIFT                     1  
#define     SER_FRAME_CTRL_TX2STOPBITS_MASK            0x00000004
#define     SER_FRAME_CTRL_TX2STOPBITS_SHIFT                  2  
#define     SER_FRAME_CTRL_PARITYCHECK_MASK            0x00000008
#define     SER_FRAME_CTRL_PARITYCHECK_SHIFT                  3  
#define     SER_FRAME_CTRL_EVENPARITY_MASK             0x00000010
#define     SER_FRAME_CTRL_EVENPARITY_SHIFT                   4  
#define     SER_FRAME_CTRL_PARITYOVERRIDE_MASK         0x00000020
#define     SER_FRAME_CTRL_PARITYOVERRIDE_SHIFT               5  
#define     SER_FRAME_CTRL_TXBREAK_MASK                0x00000040
#define     SER_FRAME_CTRL_TXBREAK_SHIFT                      6  
#define     SER_FRAME_CTRL_BAUDDIVISORSELECT_MASK      0x00000080
#define     SER_FRAME_CTRL_BAUDDIVISORSELECT_SHIFT            7  
#define SER_STATUS_REG                                 0x00000014
#define     SER_STATUS_RXFIFOSERVICE_MASK              0x00000001
#define     SER_STATUS_RXFIFOSERVICE_SHIFT                    0  
#define     SER_STATUS_RXFIFOOVERRUN_MASK              0x00000002
#define     SER_STATUS_RXFIFOOVERRUN_SHIFT                    1  
#define     SER_STATUS_PARITYERROR_MASK                0x00000004
#define     SER_STATUS_PARITYERROR_SHIFT                      2  
#define     SER_STATUS_FRAMINGERROR_MASK               0x00000008
#define     SER_STATUS_FRAMINGERROR_SHIFT                     3  
#define     SER_STATUS_BREAKRECEIVED_MASK              0x00000010
#define     SER_STATUS_BREAKRECEIVED_SHIFT                    4  
#define     SER_STATUS_TXFIFOSERVICE_MASK              0x00000020
#define     SER_STATUS_TXFIFOSERVICE_SHIFT                    5  
#define     SER_STATUS_TXIDLE_MASK                     0x00000040
#define     SER_STATUS_TXIDLE_SHIFT                           6  
#define     SER_STATUS_RXERROR_MASK                    0x00000080
#define     SER_STATUS_RXERROR_SHIFT                          7  
#define SER_IRLVL_REG                                  0x00000018
#define     SER_IRLVL_LEVEL_MASK                       0x0000000F
#define     SER_IRLVL_LEVEL_SHIFT                             0
#define SER_IRDA_CTRL_REG                              0x00000020
#define     SER_IRDA_CTRL_ENABLE_MASK                  0x00000001
#define     SER_IRDA_CTRL_ENABLE_SHIFT                        0
#define SER_TX_FIFO_STATUS_REG                         0x00000028
#define     SER_FIFO_STATUS_NUMDATA_MASK               0x0000001F
#define     SER_FIFO_STATUS_NUMDATA_SHIFT                     0
#define SER_RX_FIFO_STATUS_REG                         0x0000002c
#define     SER_EXP_STATUS_TXFIFOLEVEL_MASK            0x0000000F
#define     SER_EXP_STATUS_TXFIFOLEVEL_SHIFT                  0
#define     SER_EXP_STATUS_RXFIFOLEVEL_MASK            0x000000F0
#define     SER_EXP_STATUS_RXFIFOLEVEL_SHIFT                  4
#define SER_BRD_FRACTIONAL_REG                         0x00000030

#define SER_NUM_BASE(x)      ((x==2)?SER_BASE_UART3:(SER_BASE+(x*SER_BANK_SIZE)))

#define SER_NUM_DATA_REG(x)             (SER_DATA_REG           + (SER_NUM_BASE(x)))
#define SER_NUM_BRD_LOW_REG(x)          (SER_BRD_LOW_REG        + (SER_NUM_BASE(x)))
#define SER_NUM_INT_ENABLE_REG(x)       (SER_INT_ENABLE_REG     + (SER_NUM_BASE(x)))
#define SER_NUM_BRD_HIGH_REG(x)         (SER_BRD_HIGH_REG       + (SER_NUM_BASE(x)))
#define SER_NUM_FIFO_CTRL_REG(x)        (SER_FIFO_CTRL_REG      + (SER_NUM_BASE(x)))
#define SER_NUM_FRAME_CTRL_REG(x)       (SER_FRAME_CTRL_REG     + (SER_NUM_BASE(x)))
#define SER_NUM_STATUS_REG(x)           (SER_STATUS_REG         + (SER_NUM_BASE(x)))
#define SER_NUM_IRLVL_REG(x)            (SER_IRLVL_REG          + (SER_NUM_BASE(x)))
#define SER_NUM_IRDA_CTRL_REG(x)        (SER_IRDA_CTRL_REG      + (SER_NUM_BASE(x)))
#define SER_NUM_TX_FIFO_STATUS_REG(x)   (SER_TX_FIFO_STATUS_REG + (SER_NUM_BASE(x)))
#define SER_NUM_RX_FIFO_STATUS_REG(x)   (SER_RX_FIFO_STATUS_REG + (SER_NUM_BASE(x)))
#define SER_NUM_BRD_FRACTIONAL_REG(x)   (SER_BRD_FRACTIONAL_REG + (SER_NUM_BASE(x)))

/* Bit positions to use when writing whole register at once */
#define SER_INT_RX_FIFO     0x01
#define SER_INT_RX_OVERRUN  0x02
#define SER_INT_PARITY_ERR  0x04
#define SER_INT_FRAME_ERR   0x08
#define SER_INT_BREAK       0x10
#define SER_INT_TX_FIFO     0x20
#define SER_INT_TX_IDLE     0x40
#define SER_INT_RX_FIFO_ERR 0x80

#define SER_RXTHRESH_CHAR 0x00
#define SER_RXTHRESH_25   0x01
#define SER_RXTHRESH_50   0x02
#define SER_RXTHRESH_75   0x03

#define SER_TXTHRESH_25    0x00
#define SER_TXTHRESH_50    0x01
#define SER_TXTHRESH_75    0x02
#define SER_TXTHRESH_EMPTY 0x03

#define SER_FRAME_CTRL_PARITY_EVEN  1
#define SER_FRAME_CTRL_PARITY_ODD   0
#define SER_FRAME_CTRL_STOPBITS_1   0
#define SER_FRAME_CTRL_STOPBITS_2   1
#define SER_FRAME_CTRL_DATA_7BITS   0
#define SER_FRAME_CTRL_DATA_8BITS   1

#define SER_IRLVL_NOINT           0x1
#define SER_IRLVL_RXERR           0x6
#define SER_IRLVL_RXDATA          0x4
#define SER_IRLVL_RXDATAPRESENT   0xC
#define SER_IRLVL_TXDATA          0x2

#ifdef BITFIELDS
typedef struct _SER_DATA
{
  BIT_FLD Data:8,
          Reserved1:24;
} SER_DATA;
typedef SER_DATA volatile *LPSER_DATA;

typedef struct _SER_BRD_LOW
{
  BIT_FLD Value:8,
          Reserved1:24;
} SER_BRD_LOW;
typedef SER_BRD_LOW volatile *LPSER_BRD_LOW;

typedef struct _SER_INT_ENABLE
{
  BIT_FLD RxFifo:1,
          RxOverrun:1,
          ParityError:1,
          FramingError:1,
          BreakReceived:1,
          TxFifo:1,
          TxIdle:1,
          RxFifoError:1,
          Reserved1:24;
} SER_INT_ENABLE;
typedef SER_INT_ENABLE volatile *LPSER_INT_ENABLE;

typedef struct _SER_BRD_HIGH
{
  BIT_FLD Value:8,
          Reserved1:24;
} SER_BRD_HIGH;
typedef SER_BRD_HIGH volatile *LPSER_BRD_HIGH;

typedef struct _SER_FIFO_CTRL
{
  BIT_FLD Reserved1:1,
          RxFifoClear:1,
          TxFifoClear:1,
          Reserved2:1,
          TxFifoThreshold:2,
          RxFifoThreshold:2,
          Reserved3:24;
} SER_FIFO_CTRL;
typedef SER_FIFO_CTRL volatile *LPSER_FIFO_CTRL;

typedef struct _SER_FRAME_CTRL
{
  BIT_FLD Data8Bit:1,
          LoopBack:1,
          Tx2StopBits:1,
          ParityCheck:1,
          EvenParity:1,
          ParityOverride:1,
          TxBreak:1,
          BaudDivisorSelect:1,
          Reserved1:24;
} SER_FRAME_CTRL;
typedef SER_FRAME_CTRL volatile *LPSER_FRAME_CTRL;

typedef struct _SER_STATUS
{
  BIT_FLD RxFifoService:1,
          RxFifoOverrun:1,
          ParityError:1,
          FramingError:1,
          BreakReceived:1,
          TxFifoService:1,
          TxIdle:1,
          RxError:1,
          Reserved1:24;
} SER_STATUS;
typedef SER_STATUS volatile *LPSER_STATUS;

typedef struct _SER_IRLVL
{
  BIT_FLD Level:4,
          Reserved1:28;
} SER_IRLVL;
typedef SER_IRLVL volatile *LPSER_IRLVL;

typedef struct _SER_IRDA_CTRL
{
  BIT_FLD Enable:1,
          Reserved1:31;
} SER_IRDA_CTRL;
typedef SER_IRDA_CTRL volatile *LPSER_IRDA_CTRL;

typedef struct _SER_TXFIFO_STATUS
{
  BIT_FLD NumData:5,
          Reserved1:27;
} SER_TXFIFO_STATUS;
typedef SER_TXFIFO_STATUS volatile *LPSER_TXFIFO_STATUS;

typedef SER_TXFIFO_STATUS SER_RXFIFO_STATUS;
typedef LPSER_TXFIFO_STATUS LPSER_RXFIFO_STATUS;

typedef struct _SER_EXP_STATUS
{
  BIT_FLD TxFifoLevel:4,
          RxFifoLevel:4,
          Reserved1:24;
} SER_EXP_STATUS;
typedef SER_EXP_STATUS volatile *LPSER_EXP_STATUS;
#endif /* BITFIELDS */

#endif /* INCL_SER */

/**************/
/* Smart Card */
/**************/

/* Last updated 3/8/99 DW */

#ifdef INCL_SMC

/*
******************************************************************************
 *
 *     SMC_DATA_BASE                                        (SMC_BASE + 0x100)
 *     SMC_CONV_REG                                         (SMC_BASE + 0x00)
 *     SMC_PARITY_REG                                       (SMC_BASE + 0x04)
 *     SMC_TXRETRY_REG                                      (SMC_BASE + 0x08)
 *     SMC_RXRETRY_REG                                      (SMC_BASE + 0x0C)
 *     SMC_TXTIDE_REG                                       (SMC_BASE + 0x10)
 *     SMC_TXCOUNT_REG                                      (SMC_BASE + 0x14)
 *     SMC_RXTIDE_REG                                       (SMC_BASE + 0x18)
 *     SMC_RXCOUNT_REG                                      (SMC_BASE + 0x1C)
 *     SMC_RXTIME_REG                                       (SMC_BASE + 0x20)
 *     SMC_TERM_CTRL_REG                                    (SMC_BASE + 0x24)
 *     SMC_STABLE_REG                                       (SMC_BASE + 0x28)
 *     SMC_ICC_CTRL_REG                                     (SMC_BASE + 0x2C)
 *     SMC_ICC_STAT_REG                                     (SMC_BASE + 0x30)
 *     SMC_ATIME_REG                                        (SMC_BASE + 0x34)
 *     SMC_DTIME_REG                                        (SMC_BASE + 0x38)
 *     SMC_ATRSTIME_REG                                     (SMC_BASE + 0x3C)
 *     SMC_ATRDTIME_REG                                     (SMC_BASE + 0x40)
 *     SMC_BLKTIME_REG                                      (SMC_BASE + 0x44)
 *     SMC_CHTIME_REG                                       (SMC_BASE + 0x48)
 *     SMC_CLK_DIV_REG                                      (SMC_BASE + 0x4C)
 *     SMC_FD_REG                                           (SMC_BASE + 0x50)
 *     SMC_CONFIG_REG                                       (SMC_BASE + 0x54)
 *     SMC_CHGUARD_REG                                      (SMC_BASE + 0x58)
 *     SMC_BKGUARD_REG                                      (SMC_BASE + 0x5C)
 *     SMC_RAW_DAT_REG                                      (SMC_BASE + 0x64)
 *     SMC_INT_MASK_REG                                     (SMC_BASE + 0x68)
 *     SMC_INT_STAT_REG                                     (SMC_BASE + 0x6C)
 *     SMC_INT_QSTAT_REG                                    (SMC_BASE + 0x70)
 *
 *****************************************************************************
 */

#define NUM_SMC_DATA                                        16

#define SMC_DATA_BASE                                       (SMC_BASE + 0x100)
#define     SMC_DATA_DATA_MASK                                   0x000000FF
#define     SMC_DATA_DATA_SHIFT                                         0
#define     SMC_DATA_PARITY_MASK                                 0x00000100
#define     SMC_DATA_PARITY_SHIFT                                       8
#define     SMC_DATA_LAST_MASK                                   0x00000200
#define     SMC_DATA_LAST_SHIFT                                         9
#define SMC_CONV_REG                                        (SMC_BASE + 0x00)
#define     SMC_CONV_SENSE_MASK                                  0x00000001
#define     SMC_CONV_SENSE_SHIFT                                        0
#define     SMC_CONV_ORDER_MASK                                  0x00000002
#define     SMC_CONV_ORDER_SHIFT                                        1
#define SMC_PARITY_REG                                      (SMC_BASE + 0x04)
#define     SMC_PARITY_TXPARITY_MASK                             0x00000001
#define     SMC_PARITY_TXPARITY_SHIFT                                   0
#define     SMC_PARITY_ENABLETXNAK_MASK                          0x00000002
#define     SMC_PARITY_ENABLETXNAK_SHIFT                                1
#define     SMC_PARITY_RXPARITY_MASK                             0x00000004
#define     SMC_PARITY_RXPARITY_SHIFT                                   2
#define     SMC_PARITY_ENABLERXNAK_MASK                          0x00000008
#define     SMC_PARITY_ENABLERXNAK_SHIFT                                3
#define SMC_TXRETRY_REG                                     (SMC_BASE + 0x08)
#define SMC_RXRETRY_REG                                     (SMC_BASE + 0x0C)
#define     SMC_RETRY_RETRIES_MASK                               0x00000007
#define     SMC_RETRY_RETRIES_SHIFT                                     0
#define SMC_TXTIDE_REG                                      (SMC_BASE + 0x10)
#define SMC_RXTIDE_REG                                      (SMC_BASE + 0x18)
#define     SMC_TIDE_LEVEL_MASK                                  0x0000001F
#define     SMC_TIDE_LEVEL_SHIFT                                        0
#define SMC_TXCOUNT_REG                                     (SMC_BASE + 0x14)
#define SMC_RXCOUNT_REG                                     (SMC_BASE + 0x1C)
#define     SMC_COUNT_COUNT_MASK                                 0x0000001F
#define     SMC_COUNT_COUNT_SHIFT                                       0
#define SMC_RXTIME_REG                                      (SMC_BASE + 0x20)
#define     SMC_RXTIME_CYCLES_MASK                               0x0000FFFF
#define     SMC_RXTIME_CYCLES_SHIFT                                     0
#define SMC_TERM_CTRL_REG                                   (SMC_BASE + 0x24)
#define     SMC_TERM_CTRL_MODE_MASK                              0x00000008
#define     SMC_TERM_CTRL_MODE_SHIFT                                    3
#define     SMC_TERM_CTRL_STOPCLK_MASK                           0x00000010
#define     SMC_TERM_CTRL_STOPCLK_SHIFT                                 4
#define     SMC_TERM_CTRL_LOOPBACK_MASK                          0x00000080
#define     SMC_TERM_CTRL_LOOPBACK_SHIFT                                7
#define SMC_STABLE_REG                                      (SMC_BASE + 0x28)
#define     SMC_STABLE_DEBOUNCECOUNT_MASK                        0x000FFFFF
#define     SMC_STABLE_DEBOUNCECOUNT_SHIFT                              0
#define SMC_ICC_CTRL_REG                                    (SMC_BASE + 0x2C)
#define     SMC_ICC_CTRL_ACTIVATECARD_MASK                       0x00000001
#define     SMC_ICC_CTRL_ACTIVATECARD_SHIFT                             0
#define     SMC_ICC_CTRL_DEACTIVATECARD_MASK                     0x00000002
#define     SMC_ICC_CTRL_DEACTIVATECARD_SHIFT                           1
#define     SMC_ICC_CTRL_WARMRESET_MASK                          0x00000004
#define     SMC_ICC_CTRL_WARMRESET_SHIFT                                2
#define SMC_ICC_STAT_REG                                    (SMC_BASE + 0x30)
#define     SMC_ICC_STAT_CARDPRESENT_MASK                        0x00000001
#define     SMC_ICC_STAT_CARDPRESENT_SHIFT                              0
#define     SMC_ICC_STAT_CARDPOWER_MASK                          0x00000002
#define     SMC_ICC_STAT_CARDPOWER_SHIFT                                1
#define     SMC_ICC_STAT_CARDRESET_MASK                          0x00000004
#define     SMC_ICC_STAT_CARDRESET_SHIFT                                2
#define SMC_ATIME_REG                                       (SMC_BASE + 0x34)
#define SMC_DTIME_REG                                       (SMC_BASE + 0x38)
#define SMC_ATRSTIME_REG                                    (SMC_BASE + 0x3C)
#define SMC_ATRDTIME_REG                                    (SMC_BASE + 0x40)
#define SMC_BLKTIME_REG                                     (SMC_BASE + 0x44)
#define SMC_CHTIME_REG                                      (SMC_BASE + 0x48)
#define     SMC_TIME_CYCLES_MASK                                 0xFFFFFFFF
#define     SMC_TIME_CYCLES_SHIFT                                       0
#define SMC_CLK_DIV_REG                                     (SMC_BASE + 0x4C)
#define     SMC_CLK_DIV_LOTIMECOUNT_MASK                         0x000000FF
#define     SMC_CLK_DIV_LOTIMECOUNT_SHIFT                               0
#define     SMC_CLK_DIV_HITIMECOUNT_MASK                         0x0000FF00
#define     SMC_CLK_DIV_HITIMECOUNT_SHIFT                               8
#define SMC_FD_REG                                          (SMC_BASE + 0x50)
#define     SMC_FD_D_VALUE_MASK                                  0x000000FF
#define     SMC_FD_D_VALUE_SHIFT                                        0
#define     SMC_FD_F_VALUE_MASK                                  0x00FFFF00
#define     SMC_FD_F_VALUE_SHIFT                                        8
#define SMC_CONFIG_REG                                      (SMC_BASE + 0x54)
#define     SMC_CONFIG_VCC_VALUE_MASK                            0x00000001
#define     SMC_CONFIG_VCC_VALUE_SHIFT                                  0
#define     SMC_CONFIG_DETECT_POLARITY_MASK                      0x00000002
#define     SMC_CONFIG_DETECT_POLARITY_SHIFT                            1
#define     SMC_CONFIG_AUTO_DETECT_CONV_MASK                     0x00000004
#define     SMC_CONFIG_AUTO_DETECT_CONV_SHIFT                           2
#define SMC_CHGUARD_REG                                     (SMC_BASE + 0x58)
#define SMC_BKGUARD_REG                                     (SMC_BASE + 0x5C)
#define     SMC_GUARD_CYCLES_MASK                                0x000000FF
#define     SMC_GUARD_CYCLES_SHIFT                                      0
#define SMC_RAW_DAT_REG                                     (SMC_BASE + 0x64)
#define     SMC_RAW_DATA_MASK                                    0x00000001
#define     SMC_RAW_DATA_SHIFT                                          0  
#define     SMC_RAW_CLOCK_MASK                                   0x00000002
#define     SMC_RAW_CLOCK_SHIFT                                         1  
#define     SMC_RAW_RESET_MASK                                   0x00000004
#define     SMC_RAW_RESET_SHIFT                                         2  
#define     SMC_RAW_DETECT_MASK                                  0x00000008
#define     SMC_RAW_DETECT_SHIFT                                        3  
#define SMC_INT_MASK_REG                                    (SMC_BASE + 0x68)
#define SMC_INT_STAT_REG                                    (SMC_BASE + 0x6C)
#define SMC_INT_QSTAT_REG                                   (SMC_BASE + 0x70)
#define     SMC_INT_CARDIN_MASK                                  0x00000001
#define     SMC_INT_CARDIN_SHIFT                                        0  
#define     SMC_INT_CARDOUT_MASK                                 0x00000002
#define     SMC_INT_CARDOUT_SHIFT                                       1  
#define     SMC_INT_CARDPOWERED_MASK                             0x00000004
#define     SMC_INT_CARDPOWERED_SHIFT                                   2  
#define     SMC_INT_CARDUNPOWERED_MASK                           0x00000008
#define     SMC_INT_CARDUNPOWERED_SHIFT                                 3  
#define     SMC_INT_CARDONLINE_MASK                              0x00000010
#define     SMC_INT_CARDONLINE_SHIFT                                    4
#define     SMC_INT_CARDOFFLINE_MASK                             0x00000020
#define     SMC_INT_CARDOFFLINE_SHIFT                                   5
#define     SMC_INT_ATRSTARTTIMEOUT_MASK                         0x00000040
#define     SMC_INT_ATRSTARTTIMEOUT_SHIFT                               6
#define     SMC_INT_ATRDURATIONTIMEOUT_MASK                      0x00000080
#define     SMC_INT_ATRDURATIONTIMEOUT_SHIFT                            7
#define     SMC_INT_BLKRXTIMEOUT_MASK                            0x00000100
#define     SMC_INT_BLKRXTIMEOUT_SHIFT                                  8
#define     SMC_INT_CHARRXTIMEOUT_MASK                           0x00000200
#define     SMC_INT_CHARRXTIMEOUT_SHIFT                                 9
#define     SMC_INT_TXERROR_MASK                                 0x00000400
#define     SMC_INT_TXERROR_SHIFT                                      10
#define     SMC_INT_RXERROR_MASK                                 0x00000800
#define     SMC_INT_RXERROR_SHIFT                                      11
#define     SMC_INT_TXTIDE_MASK                                  0x00001000
#define     SMC_INT_TXTIDE_SHIFT                                       12
#define     SMC_INT_RXREAD_MASK                                  0x00002000
#define     SMC_INT_RXREAD_SHIFT                                       13

#define SMC_DATA_REG_ADDR(x)         (SMC_DATA_BASE + (x * SMC_BANK_SIZE))
#define SMC_CONV_REG_ADDR(x)         (SMC_CONV_REG + (x * SMC_BANK_SIZE))
#define SMC_PARITY_REG_ADDR(x)       (SMC_PARITY_REG + (x * SMC_BANK_SIZE))
#define SMC_TXRETRY_REG_ADDR(x)      (SMC_TXRETRY_REG + (x * SMC_BANK_SIZE))
#define SMC_RXRETRY_REG_ADDR(x)      (SMC_RXRETRY_REG + (x * SMC_BANK_SIZE))
#define SMC_TXTIDE_REG_ADDR(x)       (SMC_TXTIDE_REG + (x * SMC_BANK_SIZE))
#define SMC_TXCOUNT_REG_ADDR(x)      (SMC_TXCOUNT_REG + (x * SMC_BANK_SIZE))
#define SMC_RXTIDE_REG_ADDR(x)       (SMC_RXTIDE_REG + (x * SMC_BANK_SIZE))
#define SMC_RXCOUNT_REG_ADDR(x)      (SMC_RXCOUNT_REG + (x * SMC_BANK_SIZE))
#define SMC_RXTIME_REG_ADDR(x)       (SMC_RXTIME_REG + (x * SMC_BANK_SIZE))
#define SMC_TERM_CTRL_REG_ADDR(x)    (SMC_TERM_CTRL_REG + (x * SMC_BANK_SIZE))
#define SMC_STABLE_REG_ADDR(x)       (SMC_STABLE_REG + (x * SMC_BANK_SIZE))
#define SMC_ICC_CTRL_REG_ADDR(x)     (SMC_ICC_CTRL_REG + (x * SMC_BANK_SIZE))
#define SMC_ICC_STAT_REG_ADDR(x)     (SMC_ICC_STAT_REG + (x * SMC_BANK_SIZE))
#define SMC_ATIME_REG_ADDR(x)        (SMC_ATIME_REG + (x * SMC_BANK_SIZE))
#define SMC_DTIME_REG_ADDR(x)        (SMC_DTIME_REG + (x * SMC_BANK_SIZE))
#define SMC_ATRSTIME_REG_ADDR(x)     (SMC_ATRSTIME_REG + (x * SMC_BANK_SIZE))
#define SMC_ATRDTIME_REG_ADDR(x)     (SMC_ATRDTIME_REG + (x * SMC_BANK_SIZE))
#define SMC_BLKTIME_REG_ADDR(x)      (SMC_BLKTIME_REG + (x * SMC_BANK_SIZE))
#define SMC_CHTIME_REG_ADDR(x)       (SMC_CHTIME_REG + (x * SMC_BANK_SIZE))
#define SMC_CLK_DIV_REG_ADDR(x)      (SMC_CLK_DIV_REG + (x * SMC_BANK_SIZE))
#define SMC_FD_REG_ADDR(x)           (SMC_FD_REG + (x * SMC_BANK_SIZE))
#define SMC_CONFIG_REG_ADDR(x)       (SMC_CONFIG_REG + (x * SMC_BANK_SIZE))
#define SMC_CHGUARD_REG_ADDR(x)      (SMC_CHGUARD_REG + (x * SMC_BANK_SIZE))
#define SMC_BKGUARD_REG_ADDR(x)      (SMC_BKGUARD_REG + (x * SMC_BANK_SIZE))
#define SMC_RAW_DAT_REG_ADDR(x)      (SMC_RAW_DAT_REG + (x * SMC_BANK_SIZE))
#define SMC_INT_MASK_REG_ADDR(x)     (SMC_INT_MASK_REG + (x * SMC_BANK_SIZE))
#define SMC_INT_STAT_REG_ADDR(x)     (SMC_INT_STAT_REG + (x * SMC_BANK_SIZE))
#define SMC_INT_QSTAT_REG_ADDR(x)    (SMC_INT_QSTAT_REG + (x * SMC_BANK_SIZE))

/* For Order */
#define SMC_ORDER_DIRECT  0
#define SMC_ORDER_INVERSE 1

/* For Sense */
#define SMC_SENSE_DIRECT  0
#define SMC_SENSE_INVERSE 1

#define SMC_PARITY_EVEN 0
#define SMC_PARITY_ODD  1

#define SMC_MODE_TX 1
#define SMC_MODE_RX 0

#define Volt_value_33 0                 /* 3.3 V */
#define Volt_value_5  1
#define CARD_DETECT_SIGNAL_LOW  0
#define CARD_DETECT_SIGNAL_HIGH 1

#ifdef BITFIELDS
typedef struct _SMC_DATA
{
  BIT_FLD Data:8,
          Parity:1,
          Last:1,
          Reserved1:22;
} SMC_DATA;
typedef SMC_DATA volatile *LPSMC_DATA;

typedef struct _SMC_CONV
{
  BIT_FLD Sense:1,
          Order:1,
          Reserved1:30;
} SMC_CONV;
typedef SMC_CONV volatile *LPSMC_CONV;

typedef struct _SMC_PARITY
{
  BIT_FLD TxParity:1,
          EnableTxNak:1,
          RxParity:1,
          EnableRxNak:1,
          Reserved1:28;
} SMC_PARITY;
typedef SMC_PARITY volatile *LPSMC_PARITY;

typedef struct _SMC_TXRETRY
{
  BIT_FLD Retries:3,
          Reserved1:29;
} SMC_TXRETRY;
typedef SMC_TXRETRY volatile *LPSMC_TXRETRY;

typedef SMC_TXRETRY   SMC_RXRETRY;
typedef LPSMC_TXRETRY LPSMC_RXRETRY;

typedef struct _SMC_TXTIDE
{
  BIT_FLD Level:5,
          Reserved1:27;
} SMC_TXTIDE;
typedef SMC_TXTIDE volatile *LPSMC_TXTIDE;

typedef struct _SMC_TXCOUNT
{
  BIT_FLD Count:5,
          Reserved1:27;
} SMC_TXCOUNT;
typedef SMC_TXCOUNT volatile *LPSMC_TXCOUNT;

typedef SMC_TXTIDE   SMC_RXTIDE;
typedef LPSMC_TXTIDE LPSMC_RXTIDE;

typedef SMC_TXCOUNT   SMC_RXCOUNT;
typedef LPSMC_TXCOUNT LPSMC_RXCOUNT;

typedef struct _SMC_RXTIME
{
  BIT_FLD Cycles:16,
          Reserved1:16;
} SMC_RXTIME;
typedef SMC_RXTIME volatile *LPSMC_RXTIME;

typedef struct _SMC_TERM_CTRL
{
  BIT_FLD Reserved1:3,
          Mode:1,
          StopCLK:1,
          Reserved2:2,
          LoopBack:1,
          Reserved3:25;
} SMC_TERM_CTRL;
typedef SMC_TERM_CTRL volatile *LPSMC_TERM_CTRL;

typedef struct _SMC_STABLE
{
  BIT_FLD DebounceCount:20,
        Reserved1:12;
} SMC_STABLE;
typedef SMC_STABLE volatile *LPSMC_STABLE;

typedef struct _SMC_ICC_CTRL
{
  BIT_FLD ActivateCard:1,
          DeactivateCard:1,
          WarmReset:1,
          Reserved1:29;
} SMC_ICC_CTRL;
typedef SMC_ICC_CTRL volatile *LPSMC_ICC_CTRL;

typedef struct _SMC_ICC_STAT
{
  BIT_FLD CardPresent:1,
          CardPower:1,
          CardReset:1,
          Reserved1:29;
} SMC_ICC_STAT;
typedef SMC_ICC_STAT volatile *LPSMC_ICC_STAT;

typedef struct _SMC_ATIME
{
  BIT_FLD Cycles:16,
        Reserved1:16;
} SMC_ATIME;
typedef SMC_ATIME volatile *LPSMC_ATIME;

typedef SMC_ATIME   SMC_DTIME;
typedef LPSMC_ATIME LPSMC_DTIME;

typedef SMC_ATIME   SMC_BLKTIME;
typedef LPSMC_ATIME LPSMC_BLKTIME;

typedef SMC_ATIME   SMC_CHTIME;
typedef LPSMC_ATIME LPSMC_CHTIME;

typedef SMC_ATIME   SMC_ATRSTIME;
typedef LPSMC_ATIME LPSMC_ATRSTIME;

typedef SMC_ATIME   SMC_ATRDTIME;
typedef LPSMC_ATIME LPSMC_ATRDTIME;

typedef struct _SMC_CLK_DIV
{
  BIT_FLD LoTimeCount:8,
          HiTimeCount:8,
          Reserved1:16;
} SMC_CLK_DIV;
typedef SMC_CLK_DIV volatile *LPSMC_CLK_DIV;

typedef struct _SMC_FD
{
  BIT_FLD D_Value:8,
          F_Value:16,
          Reserved1:8;
} SMC_FD;
typedef SMC_FD volatile *LPSMC_FD;

typedef struct _SMC_CONFIG
{
  BIT_FLD Vcc_Value:1,
                Detect_Polarity:1,
                Auto_Detect_Conv:1,
                Reserved1:29;
} SMC_CONFIG;
typedef SMC_CONFIG volatile *LPSMC_CONFIG;

typedef struct _SMC_CHGUARD
{
  BIT_FLD Cycles:8,
          Reserved1:24;
} SMC_CHGUARD;
typedef SMC_CHGUARD volatile *LPSMC_CHGUARD;

typedef SMC_CHGUARD   SMC_BKGUARD;
typedef LPSMC_CHGUARD LPSMC_BKGUARD;

typedef struct _SMC_RAW_DAT
{
  BIT_FLD RawData:1,
          RawClock:1,
          RawReset:1,
          RawDetect:1,
          Reserved1:28;
} SMC_RAW_DAT;
typedef SMC_RAW_DAT volatile *LPSMC_RAW_DAT;

typedef struct _SMC_INT_STAT
{
  BIT_FLD CardIn:1,
          CardOut:1,
          CardPowered:1,
          CardUnpowered:1,
          CardOnline:1,
          CardOffline:1,
          AtrStartTimeout:1,
          AtrDurationTimeout:1,
          BlkRxTimeout:1,
          CharRxTimeout:1,
          TxError:1,
          RxError:1,
          TxTide:1,
          RxRead:1,
          Reserved1:18;
} SMC_INT_STAT;
typedef SMC_INT_STAT volatile *LPSMC_INT_STAT;

typedef SMC_INT_STAT SMC_INT_MASK;
typedef LPSMC_INT_STAT LPSMC_INT_MASK;

typedef SMC_INT_STAT SMC_INT_QSTAT;
typedef LPSMC_INT_STAT LPSMC_INT_QSTAT;
#endif /* BITFIELDS */

#endif /* INCL_SMC */

/*******/
/* PCI */
/*******/

#ifdef INCL_PCI

/*
 *******************************************************************************
 *
 *      PCI_DESC_BASE                                          (PCI_BASE + 0x00)
 *      PCI_ROM_DESC_BASE                                      (PCI_BASE + 0x00)
 *      PCI_ISA_DESC_BASE                                      (PCI_BASE + 0x00)
 *      PCI_ROM_DESC0_REG                                      (PCI_BASE + 0x00)
 *      PCI_ROM_DESC1_REG                                      (PCI_BASE + 0x04)
 *      PCI_ISAROM_DESC2_REG                                   (PCI_BASE + 0x08)
 *      PCI_ISAROM_DESC3_REG                                   (PCI_BASE + 0x0C)
 *      PCI_ISA_DESC4_REG                                      (PCI_BASE + 0x10)
 *      PCI_ISA_DESC5_REG                                      (PCI_BASE + 0x14)
 *      PCI_ISA_DESC6_REG                                      (PCI_BASE + 0x18)
 *      PCI_ISA_DESC7_REG                                      (PCI_BASE + 0x1C)
 *      PCI_ROM_MAP_BASE                                       (PCI_BASE + 0x20)
 *      PCI_ROM0_MAP_REG                                       (PCI_BASE + 0x20)
 *      PCI_ROM1_MAP_REG                                       (PCI_BASE + 0x24)
 *      PCI_ROM2_MAP_REG                                       (PCI_BASE + 0x28)
 *      PCI_ROM3_MAP_REG                                       (PCI_BASE + 0x2C)
 *      PCI_ROM_MODE_REG                                       (PCI_BASE + 0x30)
 *      PCI_ROM_XOE_MASK_REG                                   (PCI_BASE + 0x34)
 *      PCI_PAGE_BOUNDARY_REG                                  (PCI_BASE + 0x38)
 *      PCI_CFG_ADDR_REG                                       (PCI_BASE + 0x40)
 *      PCI_CFG_DATA_REG                                       (PCI_BASE + 0x44)
 *      PCI_RESET_REG                                          (PCI_BASE + 0x48)
 *      PCI_EXTINT_REG                                         (PCI_BASE + 0x4C)
 *      PCI_INTR_ENABLE_REG                                    (PCI_BASE + 0x50)
 *      PCI_INTR_STATUS_REG                                    (PCI_BASE + 0x54)
 *      PCI_REMAP_REG                                          (PCI_BASE + 0x58)
 *      PCI_DESC2_BASE                                         (PCI_BASE + 0x80)
 *      PCI_ROM_DESC0_REG2                                     (PCI_BASE + 0x80)
 *      PCI_ROM_DESC1_REG2                                     (PCI_BASE + 0x84)
 *      PCI_ISAROM_DESC2_REG2                                  (PCI_BASE + 0x88)
 *      PCI_ISAROM_DESC3_REG2                                  (PCI_BASE + 0x8C)
 *      PCI_ISA_DESC4_REG2                                     (PCI_BASE + 0x90)
 *      PCI_ISA_DESC5_REG2                                     (PCI_BASE + 0x94)
 *      PCI_ISA_DESC6_REG2                                     (PCI_BASE + 0x98)
 *      PCI_ISA_DESC7_REG2                                     (PCI_BASE + 0x9C)
 *      PCI_ATA_DMA_CTRL_REG                                  (PCI_BASE + 0x100)
 *      PCI_ATA_DMA_STATUS_REG                                (PCI_BASE + 0x104)
 *      PCI_ATA_DMA_TARGET_REG                                (PCI_BASE + 0x108)
 *      PCI_ATA_DMA_MODE_REG                                  (PCI_BASE + 0x10C)
 *      PCI_ATA_MWDMA_TIMING_REG                              (PCI_BASE + 0x110)
 *      PCI_ATA_UDMA_TIMING_REG                               (PCI_BASE + 0x114)
 *      PCI_ATA_PIO_TIMING_REG                                (PCI_BASE + 0x118)
 *      PCI_ATA_CS_SELECT_REG                                 (PCI_BASE + 0x11C)
 *      PCI_ATA_SCRAMBLE_KEY_REG                              (PCI_BASE + 0x120)
 *      PCI_ATA_INTR_ENABLE_REG                               (PCI_BASE + 0x124)
 *      PCI_ATA_COMMAND_REG                                   (PCI_BASE + 0x130)
 *      PCI_ATA_STATUS_REG                                    (PCI_BASE + 0x134)
 *      PCI_ATA_DEBUG_REG                                     (PCI_BASE + 0x170)
 *      PCI_ATA_DEBUG2_REG                                    (PCI_BASE + 0x174)
 *      PCI_ATA_KEY_LOOKUP_BASE                               (PCI_BASE + 0x180)
 *      PCI_ATA_KEY_LOOKUP_0_REG                              (PCI_BASE + 0x180)
 *      PCI_ATA_KEY_LOOKUP_1_REG                              (PCI_BASE + 0x184)
 *      PCI_ATA_KEY_LOOKUP_2_REG                              (PCI_BASE + 0x188)
 *      PCI_ATA_KEY_LOOKUP_3_REG                              (PCI_BASE + 0x18C)
 *      PCI_ATA_KEY_LOOKUP_4_REG                              (PCI_BASE + 0x190)
 *      PCI_ATA_KEY_LOOKUP_5_REG                              (PCI_BASE + 0x194)
 *      PCI_ATA_KEY_LOOKUP_6_REG                              (PCI_BASE + 0x198)
 *      PCI_ATA_KEY_LOOKUP_7_REG                              (PCI_BASE + 0x19C)
 *      PCI_ATA_KEY_LOOKUP_8_REG                              (PCI_BASE + 0x1A0)
 *      PCI_ATA_KEY_LOOKUP_9_REG                              (PCI_BASE + 0x1A4)
 *      PCI_ATA_KEY_LOOKUP_A_REG                              (PCI_BASE + 0x1A8)
 *      PCI_ATA_KEY_LOOKUP_B_REG                              (PCI_BASE + 0x1AC)
 *      PCI_ATA_KEY_LOOKUP_C_REG                              (PCI_BASE + 0x1B0)
 *      PCI_ATA_KEY_LOOKUP_D_REG                              (PCI_BASE + 0x1B4)
 *      PCI_ATA_KEY_LOOKUP_E_REG                              (PCI_BASE + 0x1B8)
 *      PCI_ATA_KEY_LOOKUP_F_REG                              (PCI_BASE + 0x1BC)
 *      PCI_ATA_KEY_BASE                                      (PCI_BASE + 0x200)
 *      PCI_ATA_KEY_0L_REG                                    (PCI_BASE + 0x200)
 *      PCI_ATA_KEY_0H_REG                                    (PCI_BASE + 0x204)
 *      PCI_ATA_KEY_1L_REG                                    (PCI_BASE + 0x208)
 *      PCI_ATA_KEY_1H_REG                                    (PCI_BASE + 0x20C)
 *      PCI_ATA_KEY_2L_REG                                    (PCI_BASE + 0x210)
 *      PCI_ATA_KEY_2H_REG                                    (PCI_BASE + 0x214)
 *      PCI_ATA_KEY_3L_REG                                    (PCI_BASE + 0x218)
 *      PCI_ATA_KEY_3H_REG                                    (PCI_BASE + 0x21C)
 *      PCI_ATA_KEY_4L_REG                                    (PCI_BASE + 0x220)
 *      PCI_ATA_KEY_4H_REG                                    (PCI_BASE + 0x224)
 *      PCI_ATA_KEY_5L_REG                                    (PCI_BASE + 0x228)
 *      PCI_ATA_KEY_5H_REG                                    (PCI_BASE + 0x22C)
 *      PCI_ATA_KEY_6L_REG                                    (PCI_BASE + 0x230)
 *      PCI_ATA_KEY_6H_REG                                    (PCI_BASE + 0x234)
 *      PCI_ATA_KEY_7L_REG                                    (PCI_BASE + 0x238)
 *      PCI_ATA_KEY_7H_REG                                    (PCI_BASE + 0x23C)
 *      PCI_ATA_KEY_8L_REG                                    (PCI_BASE + 0x240)
 *      PCI_ATA_KEY_8H_REG                                    (PCI_BASE + 0x244)
 *      PCI_ATA_KEY_9L_REG                                    (PCI_BASE + 0x248)
 *      PCI_ATA_KEY_9H_REG                                    (PCI_BASE + 0x24C)
 *      PCI_ATA_KEY_AL_REG                                    (PCI_BASE + 0x250)
 *      PCI_ATA_KEY_AH_REG                                    (PCI_BASE + 0x254)
 *      PCI_ATA_KEY_BL_REG                                    (PCI_BASE + 0x258)
 *      PCI_ATA_KEY_BH_REG                                    (PCI_BASE + 0x25C)
 *      PCI_ATA_KEY_CL_REG                                    (PCI_BASE + 0x260)
 *      PCI_ATA_KEY_CH_REG                                    (PCI_BASE + 0x264)
 *      PCI_ATA_KEY_DL_REG                                    (PCI_BASE + 0x268)
 *      PCI_ATA_KEY_DH_REG                                    (PCI_BASE + 0x26C)
 *      PCI_ATA_KEY_EL_REG                                    (PCI_BASE + 0x270)
 *      PCI_ATA_KEY_EH_REG                                    (PCI_BASE + 0x274)
 *      PCI_ATA_KEY_FL_REG                                    (PCI_BASE + 0x278)
 *      PCI_ATA_KEY_FH_REG                                    (PCI_BASE + 0x27C)
 *
 *******************************************************************************
 */

#define PCI_DESC_BASE                                          (PCI_BASE + 0x00)
#define PCI_ROM_DESC_BASE                                      (PCI_BASE + 0x00)
#define PCI_ROM_DESC0_REG                                      (PCI_BASE + 0x00)
#define PCI_ROM_DESC1_REG                                      (PCI_BASE + 0x04)
#define     PCI_ROM_DESC_HOLD_TIME_MASK                            0x00000003
#define     PCI_ROM_DESC_HOLD_TIME_SHIFT                                  0
#define     PCI_ROM_DESC_SETUP_TIME_MASK                           0x0000000C
#define     PCI_ROM_DESC_SETUP_TIME_SHIFT                                 2
#define     PCI_ROM_DESC_WIDTH_MASK                                0x00000030
#define     PCI_ROM_DESC_WIDTH_SHIFT                                      4
#define         PCI_ROM_DESC_WIDTH_32                                (0UL<<4)
#define         PCI_ROM_DESC_WIDTH_8                                 (1UL<<4)
#define         PCI_ROM_DESC_WIDTH_16                                (2UL<<4)
#define     PCI_ROM_DESC_CS_MASK                                   0x00000700
#define     PCI_ROM_DESC_CS_SHIFT                                         8
#define     PCI_ROM_DESC_BURST_ENABLE_MASK                         0x00000800
#define     PCI_ROM_DESC_BURST_ENABLE_SHIFT                              11
#define         PCI_ROM_DESC_BURST_DISABLE                          (0UL<<11)
#define         PCI_ROM_DESC_BURST_ENABLE                           (1UL<<11)
#define     PCI_ROM_DESC_BURST_WAIT_MASK                           0x0000F000
#define     PCI_ROM_DESC_BURST_WAIT_SHIFT                                12
#define     PCI_ROM_DESC_READ_WAIT_MASK                            0x007F0000
#define     PCI_ROM_DESC_READ_WAIT_SHIFT                                 16
#define     PCI_ROM_DESC_FLASH_PROGRAM_MASK                        0x00800000
#define     PCI_ROM_DESC_FLASH_PROGRAM_SHIFT                             23
#define         PCI_ROM_DESC_FLASH_NORMAL_MODE                      (0UL<<23)
#define         PCI_ROM_DESC_FLASH_PROGRAM_MODE                     (1UL<<23)
#define     PCI_ROM_DESC_WRITE_WAIT_MASK                           0x7F000000
#define     PCI_ROM_DESC_WRITE_WAIT_SHIFT                                24
#define     PCI_ROM_DESC_TYPE_MASK                                 0x80000000
#define     PCI_ROM_DESC_TYPE_SHIFT                                      31
#define         PCI_ROM_DESC_TYPE_ROM                               (0UL<<31)
#define         PCI_ROM_DESC_TYPE_ISA                               (1UL<<31)

#define PCI_ISA_DESC_BASE                                      (PCI_BASE + 0x00)
#define PCI_ISAROM_DESC2_REG                                   (PCI_BASE + 0x08)
#define PCI_ISAROM_DESC3_REG                                   (PCI_BASE + 0x0C)
#define PCI_ISA_DESC4_REG                                      (PCI_BASE + 0x10)
#define PCI_ISA_DESC5_REG                                      (PCI_BASE + 0x14)
#define PCI_ISA_DESC6_REG                                      (PCI_BASE + 0x18)
#define PCI_ISA_DESC7_REG                                      (PCI_BASE + 0x1C)
#define     PCI_ISA_DESC_HOLD_TIME_MASK                            0x00000003
#define     PCI_ISA_DESC_HOLD_TIME_SHIFT                                  0
#define     PCI_ISA_DESC_SETUP_TIME_MASK                           0x0000000C
#define     PCI_ISA_DESC_SETUP_TIME_SHIFT                                 2
#define     PCI_ISA_DESC_SYNC_IO_ENABLE_MASK                       0x00000010
#define     PCI_ISA_DESC_SYNC_IO_ENABLE_SHIFT                             4
#define         PCI_ISA_DESC_SYNC_IO_DISABLE                         (0UL<<4)
#define         PCI_ISA_DESC_SYNC_IO_ENABLE                          (1UL<<4)
#define     PCI_ISA_DESC_REG_PROG_MASK                             0x00000020
#define     PCI_ISA_DESC_REG_PROG_SHIFT                                   5
#define         PCI_ISA_DESC_REG_IS_PROG                             (1UL<<5)
#define     PCI_ISA_DESC_IO_TYPE_MASK                              0x00000040
#define     PCI_ISA_DESC_IO_TYPE_SHIFT                                    6
#define         PCI_ISA_DESC_IO_TYPE_OE_WE                           (0UL<<6)
#define         PCI_ISA_DESC_IO_TYPE_IORD_IOWR                       (1UL<<6)
#define         PCI_IS_IO                                            (1UL<<6)
#define     PCI_ISA_DESC_REG_ASSERT_ENABLE_MASK                    0x00000080
#define     PCI_ISA_DESC_REG_ASSERT_ENABLE_SHIFT                          7
#define         PCI_ISA_DESC_REG_ASSERT_DISABLE                      (0UL<<7)
#define         PCI_ISA_DESC_REG_ASSERT_ENABLE                       (1UL<<7)
#define         PCI_NEEDS_ISA_REG                                    (1UL<<7)
#define     PCI_ISA_DESC_CS_MASK                                   0x00000700
#define     PCI_ISA_DESC_CS_SHIFT                                         8
#define     PCI_ISA_DESC_EXT_WAIT_MASK                             0x00003000
#define     PCI_ISA_DESC_EXT_WAIT_SHIFT                                  12
#define         PCI_ISA_DESC_NO_EXT_WAIT_STATES                     (0UL<<12)
#define         PCI_ISA_DESC_ISA_RDY_WAIT_IS_0                      (2UL<<12)
#define         PCI_ISA_DESC_ISA_RDY_WAIT_IS_1                      (3UL<<12)
#define     PCI_ISA_DESC_READ_WAIT_MASK                            0x007F0000
#define     PCI_ISA_DESC_READ_WAIT_SHIFT                                 16
#define     PCI_ISA_DESC_WRITE_WAIT_MASK                           0x7F000000
#define     PCI_ISA_DESC_WRITE_WAIT_SHIFT                                24
#define     PCI_ISA_DESC_TYPE_MASK                                 0x80000000
#define     PCI_ISA_DESC_TYPE_SHIFT                                      31
#define         PCI_ISA_DESC_TYPE_ROM                               (0UL<<31)
#define         PCI_ISA_DESC_TYPE_ISA                               (1UL<<31)
#define         PCI_IS_ISA                                          (1UL<<31)

#define PCI_ROM_MAP_BASE                                       (PCI_BASE + 0x20)
#define PCI_ROM0_MAP_REG                                       (PCI_BASE + 0x20)
#define PCI_ROM1_MAP_REG                                       (PCI_BASE + 0x24)
#define PCI_ROM2_MAP_REG                                       (PCI_BASE + 0x28)
#define PCI_ROM3_MAP_REG                                       (PCI_BASE + 0x2C)
#define     PCI_ROM_MAP_SIZE_MASK                                  0x00000FFF
#define     PCI_ROM_MAP_SIZE_SHIFT                                        0
#define     PCI_ROM_MAP_OFFSET_MASK                                0x0FFF0000
#define     PCI_ROM_MAP_OFFSET_SHIFT                                     16

#define PCI_ROM_MODE_REG                                       (PCI_BASE + 0x30)
#define     PCI_ROM_GLOBAL_FLASH_PROGRAM_MASK                      0x00000001
#define     PCI_ROM_GLOBAL_FLASH_PROGRAM_SHIFT                            0
#define         PCI_ROM_GLOBAL_FLASH_NORMAL_MODE                     (0UL<<0)
#define         PCI_ROM_GLOBAL_FLASH_PROGRAM_MODE                    (1UL<<0)
#define     PCI_ROM_PREFETCH_ENABLE_MASK                           0x00000002
#define     PCI_ROM_PREFETCH_ENABLE_SHIFT                                 1
#define         PCI_ROM_PREFETCH_DISABLE                             (0UL<<1)
#define         PCI_ROM_PREFETCH_ENABLE                              (1UL<<1)
#define     PCI_ROM_SYNC_BURST_FLASH_ENABLE_MASK                   0x00000010
#define     PCI_ROM_SYNC_BURST_FLASH_ENABLE_SHIFT                         4
#define         PCI_ROM_SYNC_BURST_FLASH_DISABLE                     (0UL<<4)
#define         PCI_ROM_SYNC_BURST_FLASH_ENABLE                      (1UL<<4)
#define     PCI_ROM_SYNC_FLASH_CTRL_ENABLE_MASK                    0x00000020
#define     PCI_ROM_SYNC_FLASH_CTRL_ENABLE_SHIFT                          5
#define         PCI_ROM_SYNC_FLASH_CTRL_DISABLE                      (0UL<<5)
#define         PCI_ROM_SYNC_FLASH_CTRL_ENABLE                       (1UL<<5)
#define     PCI_ROM_SYNC_IO_CLOCK_DIVIDER_MASK                     0x00000F00
#define     PCI_ROM_SYNC_IO_CLOCK_DIVIDER_SHIFT                           8
#define     PCI_ROM_SYNC_IO_CLOCK_DIVIDER_ASYNC                      (0UL<<8)
/* the following are for Rev B */
#define     PCI_ROM_REQ0_SYNC_SHIFT                                      12
#define     PCI_ROM_REQ1_SYNC_SHIFT                                      14
#define     PCI_ROM_REQ2_SYNC_SHIFT                                      16

#define PCI_ROM_XOE_MASK_REG                                   (PCI_BASE + 0x34)
#define     PCI_ROM_XOE_CS_MASK                                    0x0000003F
#define     PCI_ROM_XOE_CS_SHIFT                                          0

#define PCI_PAGE_BOUNDARY_REG                                  (PCI_BASE + 0x38)
#define     PCI_PAGE_BOUNDARY_PAGE_SIZE_MASK                       0x000000FF
#define     PCI_PAGE_BOUNDARY_PAGE_SIZE_SHIFT                             0

#define PCI_CFG_ADDR_REG                                       (PCI_BASE + 0x40)
#define     PCI_CFG_ADDR_REGISTER_MASK                             0x000000FC
#define     PCI_CFG_ADDR_REGISTER_SHIFT                                   2
#define     PCI_CFG_ADDR_FUNCTION_MASK                             0x00000700
#define     PCI_CFG_ADDR_FUNCTION_SHIFT                                   8
#define     PCI_CFG_ADDR_DEVICE_MASK                               0x0000F800
#define     PCI_CFG_ADDR_DEVICE_SHIFT                                    11
#define     PCI_CFG_ADDR_BUS_MASK                                  0x00FF0000
#define     PCI_CFG_ADDR_BUS_SHIFT                                       16
#define     PCI_CFG_ADDR_ENABLE_BIT_MASK                           0x80000000
#define     PCI_CFG_ADDR_ENABLE_BIT_SHIFT                                31
#define         PCI_CFG_ADDR_ENABLE_BIT_DISABLE                     (0UL<<31)
#define         PCI_CFG_ADDR_ENABLE_BIT_ENABLE                      (1UL<<31)

#define PCI_CFG_DATA_REG                                       (PCI_BASE + 0x44)

#define PCI_RESET_REG                                          (PCI_BASE + 0x48)
#define     PCI_RESET_MASK                                         0x00000001
#define     PCI_RESET_SHIFT                                               0
#define         PCI_RESET_DEASSERTED                                 (0UL<<0)
#define         PCI_RESET_ASSERTED                                   (1UL<<0)
#define     PCI_RESET_IN_MASK                                      0x00000002
#define     PCI_RESET_IN_SHIFT                                            1
#define         PCI_RESET_IN_ASSERTED                                (0UL<<1)
#define         PCI_RESET_IN_DEASSERTED                              (1UL<<1)

#define PCI_EXTINT_REG                                         (PCI_BASE + 0x4C)
#define     PCI_EXTINT_INTA_ENABLE_MASK                            0x00000001
#define     PCI_EXTINT_INTA_ENABLE_SHIFT                                  0
#define         PCI_EXTINT_INTA_ENABLE_DEASSERTED                    (0UL<<0)
#define         PCI_EXTINT_INTA_ENABLE_ASSERTED                      (1UL<<0)
#define     PCI_EXTINT_INTA_STATUS_MASK                            0x00000002
#define     PCI_EXTINT_INTA_STATUS_SHIFT                                  1
#define         PCI_EXTINT_INTA_STATUS_ASSERTED                      (0UL<<1)
#define         PCI_EXTINT_INTA_STATUS_DEASSERTED                    (1UL<<1)

#define PCI_INTR_ENABLE_REG                                    (PCI_BASE + 0x50)
#define PCI_INTR_STATUS_REG                                    (PCI_BASE + 0x54)
#define     PCI_INTR_INTA_ENABLE_MASK                              0x00000001
#define     PCI_INTR_INTA_ENABLE_SHIFT                                    0
#define         PCI_INTR_INTA_ENABLE                                 (1UL<<0)
#define     PCI_INTR_RESET_ASSERTED_MASK                           0x00000002
#define     PCI_INTR_RESET_ASSERTED_SHIFT                                 1
#define         PCI_INTR_RESET_ASSERTED                              (1UL<<1)
#define     PCI_INTR_RESET_DEASSERTED_MASK                         0x00000004
#define     PCI_INTR_RESET_DEASSERTED_SHIFT                               2
#define         PCI_INTR_RESET_DEASSERTED                            (1UL<<2)
#define     PCI_INTR_MASTER_PARITY_ERROR_MASK                      0x00000400
#define     PCI_INTR_MASTER_PARITY_ERROR_SHIFT                           10
#define         PCI_INTR_MASTER_PARITY_ERROR                        (1UL<<10)
#define     PCI_INTR_SLAVE_SIGNALLED_TARGET_ABORT_MASK             0x00000800
#define     PCI_INTR_SLAVE_SIGNALLED_TARGET_ABORT_SHIFT                  11
#define         PCI_INTR_SLAVE_SIGNALLED_TARGET_ABORT               (1UL<<11)
#define     PCI_INTR_MASTER_RECEIVED_TARGET_ABORT_MASK             0x00001000
#define     PCI_INTR_MASTER_RECEIVED_TARGET_ABORT_SHIFT                  12
#define         PCI_INTR_MASTER_RECEIVED_TARGET_ABORT               (1UL<<12)
#define     PCI_INTR_MASTER_DETECTED_MASTER_ABORT_MASK             0x00002000
#define     PCI_INTR_MASTER_DETECTED_MASTER_ABORT_SHIFT                  13
#define         PCI_INTR_MASTER_DETECTED_MASTER_ABORT               (1UL<<13)
#define     PCI_INTR_SYSTEM_ERROR_MASK                             0x00004000
#define     PCI_INTR_SYSTEM_ERROR_SHIFT                                  14
#define         PCI_INTR_SYSTEM_ERROR                               (1UL<<14)
#define     PCI_INTR_PARITY_ERROR_MASK                             0x00008000
#define     PCI_INTR_PARITY_ERROR_SHIFT                                  15
#define         PCI_INTR_PARITY_ERROR                               (1UL<<15)

#define PCI_REMAP_REG                                          (PCI_BASE + 0x58)
#define     PCI_REMAP_IO_REMAP_MASK                                0x0000FFF0
#define     PCI_REMAP_IO_REMAP_SHIFT                                      4
#define     PCI_REMAP_MEM_REMAP_MASK                               0xE0000000
#define     PCI_REMAP_MEM_REMAP_SHIFT                                    29

#define PCI_DESC2_BASE                                         (PCI_BASE + 0x80)
#define PCI_ROM_DESC0_REG2                                     (PCI_BASE + 0x80)
#define PCI_ROM_DESC1_REG2                                     (PCI_BASE + 0x84)
#define PCI_ISAROM_DESC2_REG2                                  (PCI_BASE + 0x88)
#define PCI_ISAROM_DESC3_REG2                                  (PCI_BASE + 0x8C)
#define PCI_ISA_DESC4_REG2                                     (PCI_BASE + 0x90)
#define PCI_ISA_DESC5_REG2                                     (PCI_BASE + 0x94)
#define PCI_ISA_DESC6_REG2                                     (PCI_BASE + 0x98)
#define PCI_ISA_DESC7_REG2                                     (PCI_BASE + 0x9C)
#define     PCI_DESC2_ADDR_HOLD_TIME_MASK                          0x0000001F
#define     PCI_DESC2_ADDR_HOLD_TIME_SHIFT                                0
#define     PCI_DESC2_CTRL_SETUP_TIME_MASK                         0x00000FE0
#define     PCI_DESC2_CTRL_SETUP_TIME_SHIFT                               5
#define     PCI_DESC2_CS_HOLD_TIME_MASK                            0x0001F000
#define     PCI_DESC2_CS_HOLD_TIME_SHIFT                                 12
#define     PCI_DESC2_CS_SETUP_TIME_MASK                           0x00FE0000
#define     PCI_DESC2_CS_SETUP_TIME_SHIFT                                17
#define     PCI_DESC2_REG_ACCESS_TIME_MASK                         0x0F000000
#define     PCI_DESC2_REG_ACCESS_TIME_SHIFT                              24
#define     PCI_DESC2_REG_SETUP_TIME_MASK                          0xF0000000
#define     PCI_DESC2_REG_SETUP_TIME_SHIFT                               28

/*
 * PCI Device Config Space Offsets (by dword)
 */

#define PCI_ID_OFF                                             0x00
#define PCI_CMD_STAT_OFF                                       0x01
#define PCI_REV_ID_CLASS_CODE_OFF                              0x02
#define PCI_CLS_LT_HT_BIST_OFF                                 0x03
#define PCI_BASE_ADDR_0_OFF                                    0x04
#define PCI_BASE_ADDR_1_OFF                                    0x05
#define PCI_BASE_ADDR_2_OFF                                    0x06
#define PCI_BASE_ADDR_3_OFF                                    0x07
#define PCI_BASE_ADDR_4_OFF                                    0x08
#define PCI_BASE_ADDR_5_OFF                                    0x09
#define PCI_SUBSYSTEM_IDS                                      0x0b
#define PCI_ROM_BASE_ADDR_OFF                                  0x0c
#define PCI_IL_IP_MING_MAXG_OFF                                0x0f
#define PCI_BASE_ENABLE_SIZE                                   0x10
#define PCI_BASE_REMAP                                         0x11
#define PCI_ENABLE_TRDY                                        0x12

/*
 * PCI Device Config Space Command Register
 */

#define     PCI_CMD_IO_ACCESS_ENABLE_MASK                          0x00000001
#define     PCI_CMD_IO_ACCESS_ENABLE_SHIFT                                0
#define         PCI_CMD_IO_ACCESS_ENABLE                             (1UL<<0)
#define     PCI_CMD_MEM_ACCESS_ENABLE_MASK                         0x00000002
#define     PCI_CMD_MEM_ACCESS_ENABLE_SHIFT                               1
#define         PCI_CMD_MEM_ACCESS_ENABLE                            (1UL<<1)
#define     PCI_CMD_MASTER_ENABLE_MASK                             0x00000004
#define     PCI_CMD_MASTER_ENABLE_SHIFT                                   2
#define         PCI_CMD_MASTER_ENABLE                                (1UL<<2)
#define     PCI_CMD_SPECIAL_CYCLE_RECOGNITION_ENABLE_MASK          0x00000008
#define     PCI_CMD_SPECIAL_CYCLE_RECOGNITION_ENABLE_SHIFT                3
#define         PCI_CMD_SPECIAL_CYCLE_RECOGNITION_ENABLE             (1UL<<3)
#define     PCI_CMD_MEM_WRITE_AND_INVALIDATE_ENABLE_MASK           0x00000010
#define     PCI_CMD_MEM_WRITE_AND_INVALIDATE_ENABLE_SHIFT                 4
#define         PCI_CMD_MEM_WRITE_AND_INVALIDATE_ENABLE              (1UL<<4)
#define     PCI_CMD_PALETTE_SNOOP_ENABLE_MASK                      0x00000020
#define     PCI_CMD_PALETTE_SNOOP_ENABLE_SHIFT                            5
#define         PCI_CMD_PALETTE_SNOOP_ENABLE                         (1UL<<5)
#define     PCI_CMD_PARITY_ERROR_RESPONSE_ENABLE_MASK              0x00000040
#define     PCI_CMD_PARITY_ERROR_RESPONSE_ENABLE_SHIFT                    6
#define         PCI_CMD_PARITY_ERROR_RESPONSE_ENABLE                 (1UL<<6)
#define     PCI_CMD_WAIT_CYCLE_ENABLE_MASK                         0x00000080
#define     PCI_CMD_WAIT_CYCLE_ENABLE_SHIFT                               7
#define         PCI_CMD_WAIT_CYCLE_ENABLE                            (1UL<<7)
#define     PCI_CMD_SYSTEM_ERROR_ENABLE_MASK                       0x00000100
#define     PCI_CMD_SYSTEM_ERROR_ENABLE_SHIFT                             8
#define         PCI_CMD_SYSTEM_ERROR_ENABLE                          (1UL<<8)
#define     PCI_CMD_FAST_BACK_TO_BACK_ENABLE_MASK                  0x00000200
#define     PCI_CMD_FAST_BACK_TO_BACK_ENABLE_SHIFT                        9
#define         PCI_CMD_FAST_BACK_TO_BACK_ENABLE                     (1UL<<9)

/*
 * PCI-PCCard/CardBus Bridge Config Space Offsets
 */

#define PCI_PCCARD_BASE_ADDR_OFF                               0x04
#define PCI_BUS_NUM_OFF                                        0x06
#define PCI_CARDBUS_MEM_BASE_0_OFF                             0x07
#define PCI_CARDBUS_MEM_LIMIT_0_OFF                            0x08
#define PCI_CARDBUS_MEM_BASE_1_OFF                             0x09
#define PCI_CARDBUS_MEM_LIMIT_1_OFF                            0x0A
#define PCI_CARDBUS_IO_BASE_0_OFF                              0x0B
#define PCI_CARDBUS_IO_LIMIT_0_OFF                             0x0C
#define PCI_CARDBUS_IO_BASE_1_OFF                              0x0D
#define PCI_CARDBUS_IO_LIMIT_1_OFF                             0x0E
#define PCI_CARDBUS_BRIDGE_CTRL_OFF                            0x0F
#define PCI_PCCARD_IO_BASE_OFF                                 0x11
#define PCI_PCCARD_SYS_CTRL_OFF                                0x20
#define PCI_PCCARD_GPIO_OFF                                    0x23
#define PCI_PCCARD_RTY_DC_CC_DR_OFF                            0x24
#define PCI_PCCARD_SKT_DMA_OFF                                 0x25
#define PCI_PCCARD_DDMA_OFF                                    0x26
#define PCI_CARDBUS_POWER_OFF                                  0x29
#define PCI_PCCARD_GPE_OFF                                     0x2a
#define PCI_PCCARD_GPIO_IO_OFF                                 0x2b

/****
 ****  ATA SUBSYSTEM
 ****/

#define PCI_ATA_DMA_CTRL_REG                                  (PCI_BASE + 0x100)
#define     PCI_ATA_DMA_ENABLE_MASK                                0x00000001
#define     PCI_ATA_DMA_ENABLE_SHIFT                                      0
#define         PCI_ATA_DMA_DISABLE                                  (0UL<<0)
#define         PCI_ATA_DMA_ENABLE                                   (1UL<<0)
#define     PCI_ATA_DMA_MODE_MASK                                  0x00000002
#define     PCI_ATA_DMA_MODE_SHIFT                                        1
#define         PCI_ATA_DMA_MODE_MWDMA                               (0UL<<1)
#define         PCI_ATA_DMA_MODE_UDMA                                (1UL<<1)
#define     PCI_ATA_DMA_DIR_MASK                                   0x00000004
#define     PCI_ATA_DMA_DIR_SHIFT                                         2
#define         PCI_ATA_DMA_DIR_READ                                 (0UL<<2)
#define         PCI_ATA_DMA_DIR_WRITE                                (1UL<<2)
#define     PCI_ATA_INTR_ENABLE_MASK                               0x00000008
#define     PCI_ATA_INTR_ENABLE_SHIFT                                     3
#define         PCI_ATA_INTR_DISABLE                                 (0UL<<3)
#define         PCI_ATA_INTR_ENABLE                                  (1UL<<3)
#define     PCI_ATA_DES_ENABLE_MASK                                0x00000010
#define     PCI_ATA_DES_ENABLE_SHIFT                                      4
#define         PCI_ATA_DES_DISABLE                                  (0UL<<4)
#define         PCI_ATA_DES_ENABLE                                   (1UL<<4)
#define     PCI_ATA_DES_TYPE_MASK                                  0x00000020
#define     PCI_ATA_DES_TYPE_SHIFT                                        5
#define         PCI_ATA_DES_TYPE_ALL_DATA                            (0UL<<5)
#define         PCI_ATA_DES_TYPE_PKT_DATA                            (1UL<<5)
#define     PCI_ATA_XFER_COUNT_MASK                                0xFFFFFF00
#define     PCI_ATA_XFER_COUNT_SHIFT                                      8

#define PCI_ATA_DMA_STATUS_REG                                (PCI_BASE + 0x104)
#define     PCI_ATA_DMA_ACTIVE_MASK                                0x00000001
#define     PCI_ATA_DMA_ACTIVE_SHIFT                                      0
#define         PCI_ATA_DMA_INACTIVE                                 (0UL<<0)
#define         PCI_ATA_DMA_ACTIVE                                   (1UL<<0)
#define     PCI_ATA_DMA_COMPLETE_MASK                              0x00000002
#define     PCI_ATA_DMA_COMPLETE_SHIFT                                    1
#define         PCI_ATA_DMA_INCOMPLETE                               (0UL<<1)
#define         PCI_ATA_DMA_COMPLETE                                 (1UL<<1)
#define     PCI_ATA_PIO_ERROR_MASK                                 0x00000004
#define     PCI_ATA_PIO_ERROR_SHIFT                                       2
#define         PCI_ATA_PIO_NO_ERROR                                 (0UL<<2)
#define         PCI_ATA_PIO_ERROR                                    (1UL<<2)

#define PCI_ATA_DMA_TARGET_REG                                (PCI_BASE + 0x108)

#define PCI_ATA_DMA_MODE_REG                                  (PCI_BASE + 0x10C)
#define     PCI_ATA_DMAREQ_PINCTL_MASK                             0x00000001
#define     PCI_ATA_DMAREQ_PINCTL_SHIFT                                   0
#define         PCI_ATA_DMAREQ_PINCTL_USE_ISA                        (0UL<<0)
#define         PCI_ATA_DMAREQ_PINCTL_USE_PIO                        (1UL<<0)
#define     PCI_ATA_BREAK_MWDMA_BURSTS_AT_16_MASK                  0x00000002
#define     PCI_ATA_BREAK_MWDMA_BURSTS_AT_16_SHIFT                        1
#define         PCI_ATA_DONT_BREAK_MWDMA_BURSTS_AT_16                (0UL<<1)
#define         PCI_ATA_BREAK_MWDMA_BURSTS_AT_16                     (1UL<<1)
#define     PCI_ATA_PRESMPL_SEL_MASK                               0x0000001C
#define     PCI_ATA_PRESMPL_SEL_SHIFT                                     2
#define         PCI_ATA_PRESMPL_SEL_RAW                              (0UL<<2)
#define         PCI_ATA_PRESMPL_SEL_DSYNC0                           (1UL<<2)
#define         PCI_ATA_PRESMPL_SEL_DSYNC1                           (2UL<<2)
#define         PCI_ATA_PRESMPL_SEL_DSYNC2                           (3UL<<2)
#define         PCI_ATA_PRESMPL_SEL_DSYNC3                           (4UL<<2)
#define     PCI_ATA_ENABLE_PIO_READ_TIMEOUT_MASK                   0x00000020
#define     PCI_ATA_ENABLE_PIO_READ_TIMEOUT_SHIFT                         5
#define         PCI_ATA_DISABLE_PIO_READ_TIMEOUT                     (0UL<<5)
#define         PCI_ATA_ENABLE_PIO_READ_TIMEOUT                      (1UL<<5)
#define     PCI_ATA_DBG_SEL_MASK                                   0x000000C0
#define     PCI_ATA_DBG_SEL_SHIFT                                         6
#define     PCI_ATA_DMA_BURST_BOUNDARY_MASK                        0x0000FF00
#define     PCI_ATA_DMA_BURST_BOUNDARY_SHIFT                              8
#define     PCI_ATA_UDMA_READ_FIFO_FULL_LEVEL_MASK                 0x000F0000
#define     PCI_ATA_UDMA_READ_FIFO_FULL_LEVEL_SHIFT                      16
#define     PCI_ATA_DATA_OUT_DELAY_CTRL_MASK                       0x00300000
#define     PCI_ATA_DATA_OUT_DELAY_CTRL_SHIFT                            20
#define         PCI_ATA_DATA_OUT_DELAY_CTRL_0_0_NS                  (0UL<<20)
#define         PCI_ATA_DATA_OUT_DELAY_CTRL_2_5_NS                  (1UL<<20)
#define         PCI_ATA_DATA_OUT_DELAY_CTRL_5_0_NS                  (2UL<<20)
#define         PCI_ATA_DATA_OUT_DELAY_CTRL_7_5_NS                  (3UL<<20)
#define     PCI_ATA_DATA_IN_DELAY_CTRL_MASK                        0x00C00000
#define     PCI_ATA_DATA_IN_DELAY_CTRL_SHIFT                             22
#define         PCI_ATA_DATA_IN_DELAY_CTRL_0_0_NS                   (0UL<<22)
#define         PCI_ATA_DATA_IN_DELAY_CTRL_2_5_NS                   (1UL<<22)
#define         PCI_ATA_DATA_IN_DELAY_CTRL_5_0_NS                   (2UL<<22)
#define         PCI_ATA_DATA_IN_DELAY_CTRL_7_5_NS                   (3UL<<22)

#define PCI_ATA_MWDMA_TIMING_REG                              (PCI_BASE + 0x110)
#define     PCI_ATA_MWDMA_ACTIVE_TIME_MASK                         0x000000FF
#define     PCI_ATA_MWDMA_ACTIVE_TIME_SHIFT                               0
#define     PCI_ATA_MWDMA_WREC_TIME_MASK                           0x0000FF00
#define     PCI_ATA_MWDMA_WREC_TIME_SHIFT                                 8
#define     PCI_ATA_MWDMA_RREC_TIME_MASK                           0x00FF0000
#define     PCI_ATA_MWDMA_RREC_TIME_SHIFT                                16
#define     PCI_ATA_MWDMA_CSSETUP_TIME_MASK                        0x0F000000
#define     PCI_ATA_MWDMA_CSSETUP_TIME_SHIFT                             24
#define     PCI_ATA_MWDMA_CSHOLD_TIME_MASK                         0xF0000000
#define     PCI_ATA_MWDMA_CSHOLD_TIME_SHIFT                              28

#define PCI_ATA_UDMA_TIMING_REG                               (PCI_BASE + 0x114)
#define     PCI_ATA_UDMA_SETUP_TIME_MASK                           0x000000FF
#define     PCI_ATA_UDMA_SETUP_TIME_SHIFT                                 0
#define     PCI_ATA_UDMA_HOLD_TIME_MASK                            0x0000FF00
#define     PCI_ATA_UDMA_HOLD_TIME_SHIFT                                  8
#define     PCI_ATA_UDMA_RP_TIME_MASK                              0x00FF0000
#define     PCI_ATA_UDMA_RP_TIME_SHIFT                                   16
#define     PCI_ATA_UDMA_INIT_TIME_MASK                            0x0F000000
#define     PCI_ATA_UDMA_INIT_TIME_SHIFT                                 24
#define     PCI_ATA_UDMA_CRC_SETUP_AND_STROBE_TO_DMA_ACK_TIME_MASK 0xF0000000
#define     PCI_ATA_UDMA_CRC_SETUP_AND_STROBE_TO_DMA_ACK_TIME_SHIFT      28

#define PCI_ATA_PIO_TIMING_REG                                (PCI_BASE + 0x118)
#define     PCI_ATA_PIO_HOLD_TIME_MASK                             0x000000FF
#define     PCI_ATA_PIO_HOLD_TIME_SHIFT                                   0
#define     PCI_ATA_PIO_ACCESS_TIME_MASK                           0x0000FF00
#define     PCI_ATA_PIO_ACCESS_TIME_SHIFT                                 8
#define     PCI_ATA_PIO_SETUP_TIME_MASK                            0x00FF0000
#define     PCI_ATA_PIO_SETUP_TIME_SHIFT                                 16
#define     PCI_ATA_PIO_DELAY_TIME_MASK                            0xFF000000
#define     PCI_ATA_PIO_DELAY_TIME_SHIFT                                 24

#define PCI_ATA_CS_SELECT_REG                                 (PCI_BASE + 0x11C)
#define     PCI_ATA_DESC_MASK                                      0x000000FF
#define     PCI_ATA_DESC_SHIFT                                            0
#define         PCI_ATA_DESC0                                        (1UL<<0)
#define         PCI_ATA_DESC1                                        (1UL<<1)
#define         PCI_ATA_DESC2                                        (1UL<<2)
#define         PCI_ATA_DESC3                                        (1UL<<3)
#define         PCI_ATA_DESC4                                        (1UL<<4)
#define         PCI_ATA_DESC5                                        (1UL<<5)
#define         PCI_ATA_DESC6                                        (1UL<<6)
#define         PCI_ATA_DESC7                                        (1UL<<7)
#define     PCI_ATA_CS_MASK                                        0x0000FF00
#define     PCI_ATA_CS_SHIFT                                              8
#define         PCI_ATA_CS0                                          (1UL<<8)
#define         PCI_ATA_CS1                                          (1UL<<9)
#define         PCI_ATA_CS2                                         (1UL<<10)
#define         PCI_ATA_CS3                                         (1UL<<11)
#define         PCI_ATA_CS4                                         (1UL<<12)
#define         PCI_ATA_CS5                                         (1UL<<13)
#define         PCI_ATA_CS6                                         (1UL<<14)
#define         PCI_ATA_CS7                                         (1UL<<15)
#define PCI_ATA_DESC(desc)                                           (1<<(desc))
#define PCI_ATA_CS(cs)                             ((1UL<<(cs))<<PCI_ATA_CS_SHIFT)

#define PCI_ATA_SCRAMBLE_KEY_REG                              (PCI_BASE + 0x120)

#define PCI_ATA_INTR_ENABLE_REG                               (PCI_BASE + 0x124)
#define     PCI_ATA_DMA_COMPLETE_INTR_ENABLE_MASK                  0x00000002
#define     PCI_ATA_DMA_COMPLETE_INTR_ENABLE_SHIFT                        1
#define         PCI_ATA_DMA_COMPLETE_INTR_DISABLE                    (0UL<<1)
#define         PCI_ATA_DMA_COMPLETE_INTR_ENABLE                     (1UL<<1)
#define     PCI_ATA_PIO_ERROR_INTR_ENABLE_MASK                     0x00000004
#define     PCI_ATA_PIO_ERROR_INTR_ENABLE_SHIFT                           2
#define         PCI_ATA_PIO_ERROR_INTR_DISABLE                       (0UL<<2)
#define         PCI_ATA_PIO_ERROR_INTR_ENABLE                        (1UL<<2)

#define PCI_ATA_COMMAND_REG                                   (PCI_BASE + 0x130)
#define     PCI_ATA_COMMAND_ENTRIES_MASK                           0x000000FF
#define     PCI_ATA_COMMAND_ENTRIES_SHIFT                                 0
#define     PCI_ATA_QUEUE_SIZE_MASK                                0x0000FF00
#define     PCI_ATA_QUEUE_SIZE_SHIFT                                      8
#define     PCI_ATA_CMD_DEVICE_MASK                                0x000000FF
#define     PCI_ATA_CMD_DEVICE_SHIFT                                      0
#define     PCI_ATA_CMD_FEATURE_MASK                               0x0000FF00
#define     PCI_ATA_CMD_FEATURE_SHIFT                                     8
#define     PCI_ATA_CMD_SECTOR_COUNT_MASK                          0x00FF0000
#define     PCI_ATA_CMD_SECTOR_COUNT_SHIFT                               16
#define     PCI_ATA_CMD_NUM_SECTORS_MASK                           0xFF000000
#define     PCI_ATA_CMD_NUM_SECTORS_SHIFT                                24
#define     PCI_ATA_CMD_CYL_LOW_MASK                               0x000000FF
#define     PCI_ATA_CMD_CYL_LOW_SHIFT                                     0
#define     PCI_ATA_CMD_CYL_HIGH_MASK                              0x0000FF00
#define     PCI_ATA_CMD_CYL_HIGH_SHIFT                                    8
#define     PCI_ATA_CMD_DEV_HEAD_MASK                              0x00FF0000
#define     PCI_ATA_CMD_DEV_HEAD_SHIFT                                   16
#define     PCI_ATA_CMD_COMMAND_MASK                               0xFF000000
#define     PCI_ATA_CMD_COMMAND_SHIFT                                    24
#define     PCI_ATA_CMD_ENABLE_MASK                                0x000000FF
#define     PCI_ATA_CMD_ENABLE_SHIFT                                      0
#define     PCI_ATA_CMD_TAG_MASK                                   0x0000FF00
#define     PCI_ATA_CMD_TAG_SHIFT                                         8
#define PCI_ATA_CMD0(numsec,seccnt,feat,dev)                                  \
                     (((numsec)<<PCI_ATA_CMD_NUM_SECTORS_SHIFT)  |            \
                      ((seccnt)<<PCI_ATA_CMD_SECTOR_COUNT_SHIFT) |            \
                      ((feat)<<PCI_ATA_CMD_FEATURE_SHIFT) | (dev))
#define PCI_ATA_CMD1(cmd,devhead,cylhigh,cyllow)                              \
                     (((cmd)<<PCI_ATA_CMD_COMMAND_SHIFT)      |               \
                      ((devhead)<<PCI_ATA_CMD_DEV_HEAD_SHIFT) |               \
                      ((cylhigh)<<PCI_ATA_CMD_CYL_HIGH_SHIFT) | (cyllow))
#define PCI_ATA_CMD2(addr) (addr)
#define PCI_ATA_CMD3(tag,en)                                                  \
                     (((tag)<<PCI_ATA_CMD_TAG_SHIFT) | (en))

#define PCI_ATA_STATUS_REG                                    (PCI_BASE + 0x134)
#define     PCI_ATA_STATUS_TAG_MASK                                0x000000FF
#define     PCI_ATA_STATUS_TAG_SHIFT                                      0
#define     PCI_ATA_STATUS_MASK                                    0x0000FF00
#define     PCI_ATA_STATUS_SHIFT                                          8
#define         PCI_ATA_STATUS_NOT_COMPLETE                          (0UL<<8)
#define         PCI_ATA_STATUS_NORMAL_COMPLETION                     (1UL<<8)
#define         PCI_ATA_STATUS_PIO1_ERROR                            (5UL<<8)
#define         PCI_ATA_STATUS_PIO2_ERROR                            (6UL<<8)
#define         PCI_ATA_STATUS_PIO3_ERROR                            (7UL<<8)
#define         PCI_ATA_STATUS_PIO1_TIMEOUT                          (9UL<<8)
#define         PCI_ATA_STATUS_PIO2_TIMEOUT                         (10UL<<8)
#define         PCI_ATA_STATUS_PIO3_TIMEOUT                         (11UL<<8)
#define     PCI_ATA_READ_DATA_MASK                                 0x00FF0000
#define     PCI_ATA_READ_DATA_SHIFT                                      16
#define     PCI_ATA_STATUS_ENTRIES_MASK                            0xFF000000
#define     PCI_ATA_STATUS_ENTRIES_SHIFT                                 24

#define PCI_ATA_DEBUG_REG                                     (PCI_BASE + 0x170)
#define     PCI_ATA_DMA_REQ_STATUS_MASK                            0x00000001
#define     PCI_ATA_DMA_REQ_STATUS_SHIFT                                  0
#define         PCI_ATA_DMA_REQ_STATUS_INACTIVE                      (0UL<<0)
#define         PCI_ATA_DMA_REQ_STATUS_ACTIVE                        (1UL<<0)
#define     PCI_ATA_DMA_ACK_STATUS_MASK                            0x00000002
#define     PCI_ATA_DMA_ACK_STATUS_SHIFT                                  1
#define         PCI_ATA_DMA_ACK_STATUS_INACTIVE                      (0UL<<1)
#define         PCI_ATA_DMA_ACK_STATUS_ACTIVE                        (1UL<<1)
#define     PCI_ATA_IF_STATE_MASK                                  0x000FF000
#define     PCI_ATA_IF_STATE_SHIFT                                       12
#define     PCI_ATA_MWDMA_STATE_MASK                               0x00F00000
#define     PCI_ATA_MWDMA_STATE_SHIFT                                    20
#define     PCI_ATA_UDMA_STATE_MASK                                0xFF000000
#define     PCI_ATA_UDMA_STATE_SHIFT                                     24

#define PCI_ATA_DEBUG2_REG                                    (PCI_BASE + 0x174)
#define     PCI_ATA_FIFO_ENTRIES_MASK                              0x0000003F
#define     PCI_ATA_FIFO_ENTRIES_SHIFT                                    0
#define     PCI_ATA_DMA_COUNT_MASK                                 0xFFFFFFC0
#define     PCI_ATA_DMA_COUNT_SHIFT                                       6

#define PCI_ATA_KEY_LOOKUP_BASE                               (PCI_BASE + 0x180)
#define PCI_ATA_KEY_LOOKUP_0_REG                              (PCI_BASE + 0x180)
#define PCI_ATA_KEY_LOOKUP_1_REG                              (PCI_BASE + 0x184)
#define PCI_ATA_KEY_LOOKUP_2_REG                              (PCI_BASE + 0x188)
#define PCI_ATA_KEY_LOOKUP_3_REG                              (PCI_BASE + 0x18C)
#define PCI_ATA_KEY_LOOKUP_4_REG                              (PCI_BASE + 0x190)
#define PCI_ATA_KEY_LOOKUP_5_REG                              (PCI_BASE + 0x194)
#define PCI_ATA_KEY_LOOKUP_6_REG                              (PCI_BASE + 0x198)
#define PCI_ATA_KEY_LOOKUP_7_REG                              (PCI_BASE + 0x19C)
#define PCI_ATA_KEY_LOOKUP_8_REG                              (PCI_BASE + 0x1A0)
#define PCI_ATA_KEY_LOOKUP_9_REG                              (PCI_BASE + 0x1A4)
#define PCI_ATA_KEY_LOOKUP_A_REG                              (PCI_BASE + 0x1A8)
#define PCI_ATA_KEY_LOOKUP_B_REG                              (PCI_BASE + 0x1AC)
#define PCI_ATA_KEY_LOOKUP_C_REG                              (PCI_BASE + 0x1B0)
#define PCI_ATA_KEY_LOOKUP_D_REG                              (PCI_BASE + 0x1B4)
#define PCI_ATA_KEY_LOOKUP_E_REG                              (PCI_BASE + 0x1B8)
#define PCI_ATA_KEY_LOOKUP_F_REG                              (PCI_BASE + 0x1BC)
#define     PCI_ATA_KEY_LOOKUP_PID_MASK                            0x00001FFF
#define     PCI_ATA_KEY_LOOKUP_PID_SHIFT                                  0

#define PCI_ATA_KEY_BASE                                      (PCI_BASE + 0x200)
#define PCI_ATA_KEY_0L_REG                                    (PCI_BASE + 0x200)
#define PCI_ATA_KEY_0H_REG                                    (PCI_BASE + 0x204)
#define PCI_ATA_KEY_1L_REG                                    (PCI_BASE + 0x208)
#define PCI_ATA_KEY_1H_REG                                    (PCI_BASE + 0x20C)
#define PCI_ATA_KEY_2L_REG                                    (PCI_BASE + 0x210)
#define PCI_ATA_KEY_2H_REG                                    (PCI_BASE + 0x214)
#define PCI_ATA_KEY_3L_REG                                    (PCI_BASE + 0x218)
#define PCI_ATA_KEY_3H_REG                                    (PCI_BASE + 0x21C)
#define PCI_ATA_KEY_4L_REG                                    (PCI_BASE + 0x220)
#define PCI_ATA_KEY_4H_REG                                    (PCI_BASE + 0x224)
#define PCI_ATA_KEY_5L_REG                                    (PCI_BASE + 0x228)
#define PCI_ATA_KEY_5H_REG                                    (PCI_BASE + 0x22C)
#define PCI_ATA_KEY_6L_REG                                    (PCI_BASE + 0x230)
#define PCI_ATA_KEY_6H_REG                                    (PCI_BASE + 0x234)
#define PCI_ATA_KEY_7L_REG                                    (PCI_BASE + 0x238)
#define PCI_ATA_KEY_7H_REG                                    (PCI_BASE + 0x23C)
#define PCI_ATA_KEY_8L_REG                                    (PCI_BASE + 0x240)
#define PCI_ATA_KEY_8H_REG                                    (PCI_BASE + 0x244)
#define PCI_ATA_KEY_9L_REG                                    (PCI_BASE + 0x248)
#define PCI_ATA_KEY_9H_REG                                    (PCI_BASE + 0x24C)
#define PCI_ATA_KEY_AL_REG                                    (PCI_BASE + 0x250)
#define PCI_ATA_KEY_AH_REG                                    (PCI_BASE + 0x254)
#define PCI_ATA_KEY_BL_REG                                    (PCI_BASE + 0x258)
#define PCI_ATA_KEY_BH_REG                                    (PCI_BASE + 0x25C)
#define PCI_ATA_KEY_CL_REG                                    (PCI_BASE + 0x260)
#define PCI_ATA_KEY_CH_REG                                    (PCI_BASE + 0x264)
#define PCI_ATA_KEY_DL_REG                                    (PCI_BASE + 0x268)
#define PCI_ATA_KEY_DH_REG                                    (PCI_BASE + 0x26C)
#define PCI_ATA_KEY_EL_REG                                    (PCI_BASE + 0x270)
#define PCI_ATA_KEY_EH_REG                                    (PCI_BASE + 0x274)
#define PCI_ATA_KEY_FL_REG                                    (PCI_BASE + 0x278)
#define PCI_ATA_KEY_FH_REG                                    (PCI_BASE + 0x27C)

#ifdef BITFIELDS
typedef struct _PCI_ROM_DESC
{
  BIT_FLD Hold:2,
        Setup:2,
        Width:2,
        Reserved1:2,
        ChipSel:3,
        BurstEnable:1,
        BurstWait:4,
        ReadWait:7,
        FlashProgram:1,
        WriteWait:7,
        IsISA:1;
} PCI_ROM_DESC;
typedef PCI_ROM_DESC volatile *LPPCI_ROM_DESC;

typedef struct _PCI_ISA_DESC
{
  BIT_FLD Hold:2,
        Setup:2,
        SyncIO:1,
        ISARegIsProgrammable:1,
        IsIO:1,
        NeedsISAReg:1,
        ChipSel:3,
        Reserved1:1,
        ExtWait:2,
        Reserved2:2,
        ReadWait:7,
        Reserved3:1,
        WriteWait:7,
        IsISA:1;
} PCI_ISA_DESC;
typedef PCI_ISA_DESC volatile *LPPCI_ISA_DESC;

typedef struct _PCI_DESC2
{
  BIT_FLD AddrHold:5,
          CtrlSetup:7,
          CSHold:5,
          CSSetup:7,
          IORegAccess:4,
          IORegSetup:4;
} PCI_DESC2;
typedef PCI_DESC2 volatile *LPPCI_DESC2;

typedef struct _PCI_ROM_MAP
{
  BIT_FLD SizeMask:12,
        Reserved1:4,
        AddrSpace:12,
        Reserved2:4;
} PCI_ROM_MAP;
typedef PCI_ROM_MAP volatile *LPPCI_ROM_MAP;

typedef struct _PCI_ROM_MODE
{
  BIT_FLD FlashRAMProgram:1,
          ROMPrefetchEnable:1,
          Reserved1:2,
          EnableSyncBurst:1,
          EnableSyncControl:1,
          Reserved2:2,
          IOClockDivider:4,
          Reserved3:20;
} PCI_ROM_MODE;
typedef PCI_ROM_MODE volatile *LPPCI_ROM_MODE;

typedef struct _PCI_ATA_DMA_CTRL
{
  BIT_FLD EnableDMA:1,
        IsUDMA:1,
        IsWrite:1,
        IntrEnable:1,
        IsDes:1,
        DesType:1,
        Reserved1:2,
        TransferCount:24;
} PCI_ATA_DMA_CTRL;
typedef PCI_ATA_DMA_CTRL volatile *LPPCI_ATA_DMA_CTRL;

typedef struct _PCI_ATA_DMA_STAT
{
  BIT_FLD DMAIsActive:1,
        IntrStatus:1,
        IsPioError:1,
        Reserved1:29;
} PCI_ATA_DMA_STAT;
typedef PCI_ATA_DMA_STAT volatile *LPPCI_ATA_DMA_STAT;

typedef struct _PCI_ATA_DMA_MODE
{
  BIT_FLD UsePIO:1,
        BreakMWDMABurstsAt16:1,
        PresampleSelect:3,
        EnablePIOReadTimeout:1,
        DebugDataSelect:2,
        BreakBurstBoundary:8,
        UDMAReadFIFOFullLevel:4,
        Reserved1:12;
} PCI_ATA_DMA_MODE;
typedef PCI_ATA_DMA_MODE volatile *LPPCI_ATA_DMA_MODE;

typedef struct _PCI_ATA_MWDMA_TIMING
{
  BIT_FLD ActiveTime:8,
        WriteRecoveryTime:8,
        ReadRecoveryTime:8,
        SetupTime:4,
        HoldTime:4;
} PCI_ATA_MWDMA_TIMING;
typedef PCI_ATA_MWDMA_TIMING volatile *LPPCI_ATA_MWDMA_TIMING;

typedef struct _PCI_ATA_UDMA_TIMING
{
  BIT_FLD SetupTime:8,
        HoldTime:8,
        RPTime:8,
        Reserved:8;
} PCI_ATA_UDMA_TIMING;
typedef PCI_ATA_UDMA_TIMING volatile *LPPCI_ATA_UDMA_TIMING;

typedef struct _PCI_ATA_PIO_TIMING
{
  BIT_FLD HoldTime:8,
        AccessTime:8,
        SetupTime:8,
        DelayTime:8;
} PCI_ATA_PIO_TIMING;
typedef PCI_ATA_PIO_TIMING volatile *LPPCI_ATA_PIO_TIMING;

typedef struct _PCI_ATA_CS_SELECT
{
  BIT_FLD AtaDesc:8,
        PIOCS:8,
        Reserved:16;
} PCI_ATA_CS_SELECT;
typedef PCI_ATA_CS_SELECT volatile *LPPCI_ATA_CS_SELECT;

typedef struct _PCI_ATA_INTEN
{
  BIT_FLD Reserved1:1,
        DMACompleteEnable:1,
        PIOErrorEnable:1,
        Reserved2:29;
} PCI_ATA_INTEN;
typedef PCI_ATA_INTEN volatile *LPPCI_ATA_INTEN;

typedef struct _PCI_ATA_COMMAND_FIFO_READ
{
  BIT_FLD Entries:8,
        QueueSize:8,
        Reserved:16;
} PCI_ATA_COMMAND_FIFO_READ;
typedef PCI_ATA_COMMAND_FIFO_READ volatile *LPPCI_ATA_COMMAND_FIFO_READ;

typedef struct _PCI_ATA_COMMAND_FIFO_WRITE0
{
  BIT_FLD Device:8,
        Feature:8,
        SectorCnt:8,
        NSectors:8;
} PCI_ATA_COMMAND_FIFO_WRITE0;
typedef PCI_ATA_COMMAND_FIFO_WRITE0 volatile *LPPCI_ATA_COMMAND_FIFO_WRITE0;

typedef struct _PCI_ATA_COMMAND_FIFO_WRITE1
{
  BIT_FLD CylLow:8,
        CylHigh:8,
        DevHead:8,
        Command:8;
} PCI_ATA_COMMAND_FIFO_WRITE1;
typedef PCI_ATA_COMMAND_FIFO_WRITE1 volatile *LPPCI_ATA_COMMAND_FIFO_WRITE1;

typedef struct _PCI_ATA_COMMAND_FIFO_WRITE3
{
  BIT_FLD Enable:8,
        Tag:8,
        Reserved:16;
} PCI_ATA_COMMAND_FIFO_WRITE3;
typedef PCI_ATA_COMMAND_FIFO_WRITE3 volatile *LPPCI_ATA_COMMAND_FIFO_WRITE3;

typedef union _PCI_ATA_COMMAND_FIFO_WRITE {
         PCI_ATA_COMMAND_FIFO_WRITE0  write0;
         PCI_ATA_COMMAND_FIFO_WRITE1  write1;
         HW_DWORD                      TargetAddress;
         PCI_ATA_COMMAND_FIFO_WRITE3  write3;
} PCI_ATA_COMMAND_FIFO_WRITE;
typedef PCI_ATA_COMMAND_FIFO_WRITE volatile *LPPCI_ATA_COMMAND_FIFO_WRITE;

typedef struct _PCI_ATA_STATUS_FIFO
{
  BIT_FLD Tag:8,
        Status:8,
        ReadData:8,
        Entries:8;
} PCI_ATA_STATUS_FIFO;
typedef PCI_ATA_STATUS_FIFO volatile *LPPCI_ATA_STATUS_FIFO;

typedef struct _PCI_ATA_DES_KEY_ENTRY
{
  BIT_FLD Key0L:32,
        Key0H:32;
} PCI_ATA_DES_KEY_ENTRY;
typedef PCI_ATA_DES_KEY_ENTRY volatile *LPPCI_ATA_DES_KEY_ENTRY;

typedef struct _PCI_ATA_DEBUG
{
  BIT_FLD DmaReqStatus:1,
        DmaAckStatus:1,
        Reserved:10,
        if_state:8,
        mw_state:4,
        udma_state:8;
} PCI_ATA_DEBUG;
typedef PCI_ATA_DEBUG volatile *LPPCI_ATA_DEBUG;

typedef struct _PCI_ATA_DEBUG2
{
  BIT_FLD fifo_entries:6,
        dma_count:26;
} PCI_ATA_DEBUG2;
typedef PCI_ATA_DEBUG2 volatile *LPPCI_ATA_DEBUG2;

typedef struct _PCI_CFG_ADDR
{
  BIT_FLD Reserved1:2,
        Register:6,
        Function:3,
        Device:5,
        Bus:8,
        Reserved2:7,
        Reserved3:1;
} PCI_CFG_ADDR;
typedef PCI_CFG_ADDR volatile *LPPCI_CFG_ADDR;

typedef struct _PCI_CMD_REG
{
  BIT_FLD IOAccessEnable:1,
        MemAccessEnable:1,
        MasterEnable:1,
        SpecialCycleRecognition:1,
        MemWriteAndInvEnable:1,
        PaletteSnoopEnable:1,
        ParityErrorResponse:1,
        WaitCycleEnable:1,
        SystemErrorEnable:1,
        FastBacktoBackEnable:1,
        Reserved:6;
} PCI_CMD_REG;
typedef PCI_CMD_REG volatile *LPPCI_CMD_REG;

typedef struct _PCI_STAT_REG
{
  BIT_FLD Reserved1:7,
        FastBacktoBackCapable:1,
        DataParityReported:1,
        DeviceSelectTiming:1,
        SignaledTargetAbort:1,
        ReceivedTargetAbort:1,
        ReceivedMasterAbort:1,
        SignaledSystemError:1,
        DetectedParityError:1;
} PCI_STAT_REG;
typedef PCI_STAT_REG volatile *LPPCI_STAT_REG;

typedef struct _PCI_CMD_STAT
{
  PCI_CMD_REG Command;
  PCI_STAT_REG Status;
} PCI_CMD_STAT;
typedef PCI_CMD_STAT volatile *LPPCI_CMD_STAT;

typedef struct _PCI_RESET
{
  BIT_FLD Reset:1,
        Reserved1:31;
} PCI_RESET;
typedef PCI_RESET volatile *LPPCI_RESET;

typedef struct _PCI_RTT_CSSEL
{
  BIT_FLD ARB:3,
          Reserved1:1,
          CS:3,
          Reserved2:25;
} PCI_RTT_CSSEL;
typedef PCI_RTT_CSSEL volatile *LPPCI_RTT_CSSEL;
#endif /* BITFIELDS */

#endif /* INCL_PCI */


/********/
/* MPEG */
/********/

/* last updated 4/17/01 to match 4/16 Wabash Spec */
#ifdef INCL_MPG

/*
********************************************************************************
 *
 *      MPG_CONTROL0_REG                                      (MPG_BASE + 0x000)
 *      MPG_CONTROL1_REG                                      (MPG_BASE + 0x004)
 *      MPG_COMMAND_REG                                       (MPG_BASE + 0x008)
 *      MPG_ISR_REG                                           (MPG_BASE + 0x00C)
 *      MPG_IMR_REG                                           (MPG_BASE + 0x010)
 *      MPG_ERROR_STATUS_REG                                  (MPG_BASE + 0x014)
 *      MPG_ERROR_MASK_REG                                    (MPG_BASE + 0x018)
 *      MPG_DECODE_STATUS_REG                                 (MPG_BASE + 0x01C)
 *      MPG_STC_HI_REG                                        (MPG_BASE + 0x020)
 *      MPG_STC_LO_REG                                        (MPG_BASE + 0x024)
 *      MPG_PSCRS_HI_REG                                      (MPG_BASE + 0x028)
 *      MPG_PSCRS_LO_REG                                      (MPG_BASE + 0x02C)
 *      MPG_STCS_HI_REG                                       (MPG_BASE + 0x030)
 *      MPG_STCS_LO_REG                                       (MPG_BASE + 0x034)
 *      MPG_PCR_TIMER_INFO_REG                                (MPG_BASE + 0x038)
 *      MPG_PTS_HI_REG                                        (MPG_BASE + 0x03C)
 *      MPG_PTS_LO_REG                                        (MPG_BASE + 0x040)
 *      MPG_PTSSC_HI_REG                                      (MPG_BASE + 0x044)
 *      MPG_PTSSC_LO_REG                                      (MPG_BASE + 0x048)
 *      MPG_PTS_COUNT_HI_REG                                  (MPG_BASE + 0x04C)
 *      MPG_PTS_COUNT_LO_REG                                  (MPG_BASE + 0x050)
 *      MPG_PTS_TIMER_INFO_REG                                (MPG_BASE + 0x054)
 *      MPG_MCODE_W_REG                                       (MPG_BASE + 0x058)
 *      MPG_VIDEO_SIZE_REG                                    (MPG_BASE + 0x05C)
 *      MPG_GOP_TIME_CODE_REG                                 (MPG_BASE + 0x060)
 *      MPG_FRAME_DROP_CNT_REG                                (MPG_BASE + 0x064)
 *      MPG_AUDIO_DROP_CNT_REG                                (MPG_BASE + 0x068)
 *      MPG_VPTS_H_REG                                        (MPG_BASE + 0x06C)
 *      MPG_VPTS_L_REG                                        (MPG_BASE + 0x070)
 *      MPG_APTS_C_PCRH_REG                                   (MPG_BASE + 0x074)
 *      MPG_APTS_C_PCRL_REG                                   (MPG_BASE + 0x078)
 *      MPG_ADDR_EXT_REG                                      (MPG_BASE + 0x07C)
 *      MPG_AC3_DROP_CNT_REG                                  (MPG_BASE + 0x080)
 *      MPG_AC3_PTS_HI_REG                                    (MPG_BASE + 0x084)
 *      MPG_AC3_PTS_LO_REG                                    (MPG_BASE + 0x088)
 *      MPG_OFFSET_HI_REG                                     (MPG_BASE + 0x08C)
 *      MPG_OFFSET_LO_REG                                     (MPG_BASE + 0x090)
 *      MPG_EARLY_SYNC_REG                                    (MPG_BASE + 0x094)
 *
 *******************************************************************************
 */

#define MPG_AC3_AUDIO_PTS      0x000003A8
#define MPG_VIDEO_PTS          0x00000380
#define MPG_AUDIO_PTS          0x00000398

#define MPG_CONTROL0_REG                                      (MPG_BASE + 0x000)
#define     MPG_CONTROL0_ENABLEINT_MASK                            0x00000004
#define     MPG_CONTROL0_ENABLEINT_SHIFT                                  2
#define     MPG_CONTROL0_DOWNLOADMCODE_MASK                        0x00000008
#define     MPG_CONTROL0_DOWNLOADMCODE_SHIFT                              3
#define     MPG_CONTROL0_DOWNLOADVIDEO_MASK                        0x00000010
#define     MPG_CONTROL0_DOWNLOADVIDEO_SHIFT                              4
#define     MPG_CONTROL0_VIDEOCODEVALID_MASK                       0x00000020
#define     MPG_CONTROL0_VIDEOCODEVALID_SHIFT                             5
#define     MPG_CONTROL0_AUDIOCODEVALID_MASK                       0x00000040
#define     MPG_CONTROL0_AUDIOCODEVALID_SHIFT                             6
#define     MPG_CONTROL0_PTSISAC3_MASK                             0x00000080
#define     MPG_CONTROL0_PTSISAC3_SHIFT                                   7
#define     MPG_CONTROL0_MPEGACTIVE_MASK                           0x00020000
#define     MPG_CONTROL0_MPEGACTIVE_SHIFT                                17
#define     MPG_CONTROL0_ENABLESYNC_MASK                           0x00040000
#define     MPG_CONTROL0_ENABLESYNC_SHIFT                                18
#define     MPG_CONTROL0_ENTIREBCHROMA_MASK                        0x00080000
#define     MPG_CONTROL0_ENTIREBCHROMA_SHIFT                             19
#define     MPG_CONTROL0_FULLB_MASK                                0x00100000
#define     MPG_CONTROL0_FULLB_SHIFT                                     20
#define     MPG_CONTROL0_DISABLESEQUSERDATA_MASK                   0x00200000
#define     MPG_CONTROL0_DISABLESEQUSERDATA_SHIFT                        21
#define     MPG_CONTROL0_DISABLEGOPUSERDATA_MASK                   0x00400000
#define     MPG_CONTROL0_DISABLEGOPUSERDATA_SHIFT                        22
#define     MPG_CONTROL0_DISABLEPICUSERDATA_MASK                   0x00800000
#define     MPG_CONTROL0_DISABLEPICUSERDATA_SHIFT                        23
#define     MPG_CONTROL0_ENABLEASPECTCONVERT_MASK                  0x01000000
#define     MPG_CONTROL0_ENABLEASPECTCONVERT_SHIFT                       24
#define     MPG_CONTROL0_ENABLEPANSCAN_MASK                        0x02000000
#define     MPG_CONTROL0_ENABLEPANSCAN_SHIFT                             25
#define     MPG_CONTROL0_NEWPTSSTORED_MASK                         0x04000000
#define     MPG_CONTROL0_NEWPTSSTORED_SHIFT                              26
#define     MPG_CONTROL0_NEWPTSISVIDEO_MASK                        0x08000000
#define     MPG_CONTROL0_NEWPTSISVIDEO_SHIFT                             27
#define     MPG_CONTROL0_LOADBACKUPSTC_MASK                        0x10000000
#define     MPG_CONTROL0_LOADBACKUPSTC_SHIFT                             28
#define     MPG_CONTROL0_SWPARSEMODE_MASK                          0x20000000
#define     MPG_CONTROL0_SWPARSEMODE_SHIFT                               29
#define     MPG_CONTROL0_MPEGCORERESET_MASK                        0x40000000
#define     MPG_CONTROL0_MPEGCORERESET_SHIFT                             30
#define     MPG_CONTROL0_COMARESET_MASK                            0x80000000
#define     MPG_CONTROL0_COMARESET_SHIFT                                 31
#define MPG_CONTROL1_REG                                      (MPG_BASE + 0x004)
#define     MPG_CONTROL1_KARAOKEMODE_MASK                          0x0000000C
#define     MPG_CONTROL1_KARAOKEMODE_SHIFT                                2
#define     MPG_CONTROL1_ENABLEDOWNMIX_MASK                        0x00000010
#define     MPG_CONTROL1_ENABLEDOWNMIX_SHIFT                              4
#define     MPG_CONTROL1_DISABLEDIALNORM_MASK                      0x00000020
#define     MPG_CONTROL1_DISABLEDIALNORM_SHIFT                            5
#define     MPG_CONTROL1_DISABLEHIGHDYNRNG_MASK                    0x00000040
#define     MPG_CONTROL1_DISABLEHIGHDYNRNG_SHIFT                          6
#define     MPG_CONTROL1_DISABLELOWDYNRNG_MASK                     0x00000080
#define     MPG_CONTROL1_DISABLELOWDYNRNG_SHIFT                           7
#define     MPG_CONTROL1_OUTPUTSELECT_MASK                         0x00000100
#define     MPG_CONTROL1_OUTPUTSELECT_SHIFT                               8
#define     MPG_CONTROL1_LTRTMODE_MASK                             0x00000200
#define     MPG_CONTROL1_LTRTMODE_SHIFT                                   9
#define     MPG_CONTROL1_HALTDECODE_MASK                           0x00000400
#define     MPG_CONTROL1_HALTDECODE_SHIFT                                10
#define     MPG_CONTROL1_SPDIFISAC3_MASK                           0x00000800
#define     MPG_CONTROL1_SPDIFISAC3_SHIFT                                11
#define     MPG_CONTROL1_SPDIFPROFMODE_MASK                        0x00001000
#define     MPG_CONTROL1_SPDIFPROFMODE_SHIFT                             12
#define     MPG_CONTROL1_KARAOKECAPABLE_MASK                       0x00002000
#define     MPG_CONTROL1_KARAOKECAPABLE_SHIFT                            13
#define     MPG_CONTROL1_MUTE_MASK                                 0x00004000
#define     MPG_CONTROL1_MUTE_SHIFT                                      14
#define     MPG_CONTROL1_OUTPUTLEFTJUSTIFIED_MASK                  0x00008000
#define     MPG_CONTROL1_OUTPUTLEFTJUSTIFIED_SHIFT                       15
#define     MPG_CONTROL1_LROUTPUTCNTRL_MASK                        0x00030000
#define     MPG_CONTROL1_LROUTPUTCNTRL_SHIFT                             16
#define     MPG_CONTROL1_AUDFRAMESTART_MASK                        0x00040000
#define     MPG_CONTROL1_AUDFRAMESTART_SHIFT                             18
#define     MPG_CONTROL1_SPDIFCNTRL_MASK                           0x00180000
#define     MPG_CONTROL1_SPDIFCNTRL_SHIFT                                19
#define     MPG_CONTROL1_SAMPFREQ_MASK                             0x00E00000
#define     MPG_CONTROL1_SAMPFREQ_SHIFT                                  21
#define     MPG_CONTROL1_AUDIOPARMCHANGE_MASK                      0x01000000
#define     MPG_CONTROL1_AUDIOPARMCHANGE_SHIFT                           24
#define     MPG_CONTROL1_ENABLEAUDSYNC_MASK                        0x02000000
#define     MPG_CONTROL1_ENABLEAUDSYNC_SHIFT                             25
#define     MPG_CONTROL1_DSSMODE_MASK                              0x04000000
#define     MPG_CONTROL1_DSSMODE_SHIFT                                   26
#define     MPG_CONTROL1_ENABLEAC3SYNC_MASK                        0x08000000
#define     MPG_CONTROL1_ENABLEAC3SYNC_SHIFT                             27
#define     MPG_CONTROL1_SPDIFHDR_SAMPLEFREQ_WRITE_MASK            0x30000000
#define     MPG_CONTROL1_SPDIFHDR_SAMPLEFREQ_WRITE_SHIFT                 28
#define     MPG_CONTROL1_SPDIFHDR_SAMPLEFREQ_WRITE_48K              (0UL<<28)
#define     MPG_CONTROL1_SPDIFHDR_SAMPLEFREQ_WRITE_44_1K            (1UL<<28)
#define     MPG_CONTROL1_SPDIFHDR_SAMPLEFREQ_WRITE_32K              (2UL<<28)
#define     MPG_CONTROL1_ENCVIDEOENABLE_MASK                       0x40000000
#define     MPG_CONTROL1_ENCVIDEOENABLE_SHIFT                            30
#define     MPG_CONTROL1_ENCAUDIOENABLE_MASK                       0x80000000
#define     MPG_CONTROL1_ENCAUDIOENABLE_SHIFT                            31
#define MPG_COMMAND_REG                                       (MPG_BASE + 0x008)
#define     MPG_COMMAND_COMMAND_MASK                               0x00001FFF
#define     MPG_COMMAND_COMMAND_SHIFT                                     0
#define MPG_ISR_REG                                           (MPG_BASE + 0x00C)
#define MPG_IMR_REG                                           (MPG_BASE + 0x010)
#define MPG_ERROR_STATUS_REG                                  (MPG_BASE + 0x014)
#define     MPG_ERROR_STATUS_BITSTREAMERROR_MASK                   0x00000001
#define     MPG_ERROR_STATUS_BITSTREAMERROR_SHIFT                         0
#define     MPG_ERROR_STATUS_ANCOVERFLOW_MASK                      0x00000010
#define     MPG_ERROR_STATUS_ANCOVERFLOW_SHIFT                            4
#define     MPG_ERROR_STATUS_USEROVERFLOW_MASK                     0x00000020
#define     MPG_ERROR_STATUS_USEROVERFLOW_SHIFT                           5
#define     MPG_ERROR_STATUS_AUDIO_CRC_MASK                        0x00010000
#define     MPG_ERROR_STATUS_AUDIO_CRC_SHIFT                             16 
#define     MPG_ERROR_STATUS_AUDIO_CMDINVLD_MASK                   0x00020000
#define     MPG_ERROR_STATUS_AUDIO_CMDINVLD_SHIFT                        17 
#define MPG_ERROR_MASK_REG                                    (MPG_BASE + 0x018)
#define MPG_DECODE_STATUS_REG                                 (MPG_BASE + 0x01C)
#define     MPG_DECODE_STATUS_CMDVALID_MASK                        0x00000001
#define     MPG_DECODE_STATUS_CMDVALID_SHIFT                              0
#define     MPG_DECODE_STATUS_UNREADUSER_MASK                      0x00000008
#define     MPG_DECODE_STATUS_UNREADUSER_SHIFT                            3
#define     MPG_DECODE_STATUS_UNREADANC_MASK                       0x00000010
#define     MPG_DECODE_STATUS_UNREADANC_SHIFT                             4
#define     MPG_DECODE_STATUS_VIDEOERROR_MASK                      0x000007C0
#define     MPG_DECODE_STATUS_VIDEOERROR_SHIFT                            6
#define     MPG_DECODE_STATUS_AUDIOERROR_MASK                      0x00001800
#define     MPG_DECODE_STATUS_AUDIOERROR_SHIFT                           11
#define     MPG_DECODE_STATUS_TESTDSPDONE_MASK                     0x00002000
#define     MPG_DECODE_STATUS_TESTDSPDONE_SHIFT                          13
#define     MPG_DECODE_STATUS_TESTSTARTDSP_MASK                    0x00004000
#define     MPG_DECODE_STATUS_TESTSTARTDSP_SHIFT                         14
#define     MPG_DECODE_STATUS_AUD1P5DECENABLE_MASK                 0x00100000
#define     MPG_DECODE_STATUS_AUD1P5DECENABLE_SHIFT                      20
#define MPG_STC_HI_REG                                        (MPG_BASE + 0x020)
#define MPG_STC_LO_REG                                        (MPG_BASE + 0x024)
#define MPG_PSCRS_HI_REG                                      (MPG_BASE + 0x028)
#define MPG_PSCRS_LO_REG                                      (MPG_BASE + 0x02C)
#define MPG_STCS_HI_REG                                       (MPG_BASE + 0x030)
#define MPG_STCS_LO_REG                                       (MPG_BASE + 0x034)
#define MPG_PCR_TIMER_INFO_REG                                (MPG_BASE + 0x038)
#define     MPG_PCR_TIMER_INFO_TIMERVALUE_MASK                     0x1FFFFFFF
#define     MPG_PCR_TIMER_INFO_TIMERVALUE_SHIFT                           0
#define     MPG_PCR_TIMER_INFO_ENABLETIMER_MASK                    0x20000000
#define     MPG_PCR_TIMER_INFO_ENABLETIMER_SHIFT                         29
#define MPG_PTS_HI_REG                                        (MPG_BASE + 0x03C)
#define MPG_PTS_LO_REG                                        (MPG_BASE + 0x040)
#define MPG_PTSSC_HI_REG                                      (MPG_BASE + 0x044)
#define MPG_PTSSC_LO_REG                                      (MPG_BASE + 0x048)
#define MPG_PTS_COUNT_HI_REG                                  (MPG_BASE + 0x04C)
#define MPG_PTS_COUNT_LO_REG                                  (MPG_BASE + 0x050)
#define MPG_PTS_TIMER_INFO_REG                                (MPG_BASE + 0x054)
#define MPG_MCODE_W_REG                                       (MPG_BASE + 0x058)
#define MPG_VIDEO_SIZE_REG                                    (MPG_BASE + 0x05C)
#define     MPG_VID_SIZE_WIDTH_MASK                                0x000003FF
#define     MPG_VID_SIZE_WIDTH_SHIFT                                      0
#define     MPG_VID_SIZE_HEIGHT_MASK                               0x03FF0000
#define     MPG_VID_SIZE_HEIGHT_SHIFT                                    16
#define     MPG_VID_SIZE_ASPECTRATIO_MASK                          0xF0000000
#define     MPG_VID_SIZE_ASPECTRATIO_SHIFT                               28
#define MPG_GOP_TIME_CODE_REG                                 (MPG_BASE + 0x060)
#define     MPG_GOP_TIME_CODE_BROKENLINK_MASK                      0x00000001
#define     MPG_GOP_TIME_CODE_BROKENLINK_SHIFT                            0
#define     MPG_GOP_TIME_CODE_CLOSEDGOP_MASK                       0x00000002
#define     MPG_GOP_TIME_CODE_CLOSEDGOP_SHIFT                             1
#define     MPG_GOP_TIME_CODE_PICTURES_MASK                        0x000000FC
#define     MPG_GOP_TIME_CODE_PICTURES_SHIFT                              2
#define     MPG_GOP_TIME_CODE_SECONDS_MASK                         0x00003F00
#define     MPG_GOP_TIME_CODE_SECONDS_SHIFT                               8
#define     MPG_GOP_TIME_CODE_MINUTES_MASK                         0x000FC000
#define     MPG_GOP_TIME_CODE_MINUTES_SHIFT                              14
#define     MPG_GOP_TIME_CODE_HOURS_MASK                           0x01F00000
#define     MPG_GOP_TIME_CODE_HOURS_SHIFT                                20
#define     MPG_GOP_TIME_CODE_DROPFRAME_MASK                       0x02000000
#define     MPG_GOP_TIME_CODE_DROPFRAME_SHIFT                            25
#define MPG_FRAME_DROP_CNT_REG                                (MPG_BASE + 0x064)
#define MPG_AUDIO_DROP_CNT_REG                                (MPG_BASE + 0x068)
#define     MPG_FRAME_DROP_CNT_DROP_MASK                           0x00000FFF
#define     MPG_FRAME_DROP_CNT_DROP_SHIFT                                 0
#define     MPG_FRAME_DROP_CNT_REPEAT_MASK                         0x00FFF000
#define     MPG_FRAME_DROP_CNT_REPEAT_SHIFT                              12
#define MPG_VPTS_H_REG                                        (MPG_BASE + 0x06C)
#define     MPG_VPTS_H_MSB_MASK                                    0x00000001
#define     MPG_VPTS_H_MSB_SHIFT                                          0
#define MPG_VPTS_L_REG                                        (MPG_BASE + 0x070)
#define MPG_APTS_C_PCRH_REG                                   (MPG_BASE + 0x074)
#define MPG_APTS_C_PCRL_REG                                   (MPG_BASE + 0x078)
#define MPG_ADDR_EXT_REG                                      (MPG_BASE + 0x07C)
#define     MPG_ADDR_EXT_ADDREXTENSION_MASK                        0x00000007
#define     MPG_ADDR_EXT_ADDREXTENSION_SHIFT                              0
#define     MPG_ADDR_EXT_LASTDECODED_MASK                          0x00000300
#define     MPG_ADDR_EXT_LASTDECODED_SHIFT                                8
#define     MPG_ADDR_EXT_TOPFIELD_MASK                             0x00000400
#define     MPG_ADDR_EXT_TOPFIELD_SHIFT                                  10
#define     MPG_ADDR_EXT_LASTDISPLAYED_MASK                        0x00003000
#define     MPG_ADDR_EXT_LASTDISPLAYED_SHIFT                             12
#define     MPG_ADDR_EXT_INSYNC_MASK                               0x00004000
#define     MPG_ADDR_EXT_INSYNC_SHIFT                                    14
#define MPG_AC3_DROP_CNT_REG                                  (MPG_BASE + 0x080)
#define MPG_AC3_PTS_HI_REG                                    (MPG_BASE + 0x084)
#define MPG_AC3_PTS_LO_REG                                    (MPG_BASE + 0x088)
#define MPG_OFFSET_HI_REG                                     (MPG_BASE + 0x08C)
#define MPG_OFFSET_LO_REG                                     (MPG_BASE + 0x090)
#define MPG_EARLY_SYNC_REG                                    (MPG_BASE + 0x094)  
#define MPG_EARLY_SYNC_MASK                                         0x01000000
#define MPG_EARLY_SYNC_SHIFT                                           24	

/* Definitions for LastDecoded field when accessed directly  */
#define MPG_VID_MASK_COMPLETE       0x300
#define MPG_VID_I_COMPLETE          0x000
#define MPG_VID_P_COMPLETE          0x100
#define MPG_VID_B_COMPLETE          0x200

/* Definitions for the TopField field when accessed directly */
#define MPG_VID_FIELD_MASK          0x400
#define MPG_VID_TOP_FIELD_FLAG      0x400
#define MPG_VID_BOTTOM_FIELD_FLAG   0x000

/* Definitions for LastDisplayed field when accessed directly  */
#define MPG_VID_MASK_DISPLAYED      0x3000
#define MPG_VID_I_DISPLAYED         0x0000
#define MPG_VID_P_DISPLAYED         0x1000
#define MPG_VID_B_FRAME_DISPLAYED   0x2000
#define MPG_VID_B_FIELD_DISPLAYED   0x3000

/* Bit 18 (third but of the height field) indicates whether or not */
/* video playback is in sync (assuming AV sync is enabled)         */
#define MPG_VID_MASK_IN_SYNC        0x4000
#define MPG_VID_IN_SYNC             0x4000
#define MPG_VID_NOT_IN_SYNC         0x0000

#define MPG_VIDEO_MC_VALID     0x20
#define MPG_AUDIO_MC_VALID     0x40
#define MPG_DOWN_LOAD_MC       0x08
#define MPG_DOWN_LOAD_VIDEO_MC 0x10
#define MPEG_MPEG_ACTIVE       0x20000
#define MPG_ENABLE_SYNC        0x40000
#define MPEG_CORE_RESET        MPG_CONTROL0_MPEGCORERESET_MASK
#define MPEG_CORE_RESET_HIGH   MPG_CONTROL0_COMARESET_MASK

#define MPG_OUTPUT_LINEOUT 0x00
#define MPG_OUTPUT_RF      0x01

#define MPG_LRCONTROL_NORMAL  0x00
#define MPG_LRCONTROL_LREPEAT 0x01
#define MPG_LRCONTROL_RREPEAT 0x02
#define MPG_LRCONTROL_MIXMONO 0x03

#define MPG_SPDIFCONTROL_LR   0x00
#define MPG_SPDIFCONTROL_SLSR 0x01
#define MPG_SPDIFCONTROL_CLFE 0x02

#define MPG_SAMPFREQ_48000 0x00
#define MPG_SAMPFREQ_44100 0x01
#define MPG_SAMPFREQ_32000 0x02
#define MPG_SAMPFREQ_24000 0x04
#define MPG_SAMPFREQ_22050 0x05
#define MPG_SAMPFREQ_16000 0x06

#define MPG_ENC_VIDEO_ENABLE    0x40000000
#define MPG_ENC_AUDIO_ENABLE    0x80000000

#define MPG_AUD_SYNC_ENABLED    0x02000000

#define MPG_COMMAND_SRESET       0x0000
#define MPG_COMMAND_FREEZE       0x0008
#define MPG_COMMAND_FFCONT       0x0010
#define MPG_COMMAND_FFINIT       0x0018
#define MPG_COMMAND_SLOWMO0      0x0020
#define MPG_COMMAND_SLOWMO1      0x0021
#define MPG_COMMAND_SLOWMO2      0x0022
#define MPG_COMMAND_SLOWMO3      0x0023
#define MPG_COMMAND_SLOWMO4      0x0024
#define MPG_COMMAND_SLOWMO5      0x0025
#define MPG_COMMAND_SLOWMO6      0x0026
#define MPG_COMMAND_SLOWMO7      0x0027
#define MPG_COMMAND_PAUSE        0x1028
#define MPG_COMMAND_PLAY         0x0030
#define MPG_COMMAND_CONTINUE     0x0038
#define MPG_COMMAND_STILL_NEXT   0x0060
#define MPG_COMMAND_STILL_WAIT   0x0050
#define MPG_COMMAND_STILL_PTS    0x0051
#define MPG_COMMAND_INITI        0x0041
#define MPG_COMMAND_INITP        0x0042
#define MPG_COMMAND_INITB        0x0043
#define MPG_COMMAND_INITOFFSET   0x0044
#define MPG_COMMAND_FLUSH_VBV    0x0048

#define MPG_PIC_ADDR_BIT         0x1000

/* Bit position definitions when using register as a HW_DWORD */
#define MPG_ERROR_IN_STAT_REG       0x00000001
#define MPG_USER_DETECTED           0x00000008
#define MPG_ANC_DETECTED            0x00000010
#define MPG_AC3_LOWWATERMARK        0x00000040
#define MPG_VB_LOWWATERMARK         0x00000080
#define MPG_AB_LOWWATERMARK         0x00000100
#define MPG_VB_FULL                 0x00000200
#define MPG_VB_EMPTY                0x00000400
#define MPG_AB_FULL                 0x00000800
#define MPG_AB_EMPTY                0x00001000
#define MPG_AC3_FULL                0x00002000
#define MPG_AC3_EMPTY               0x00004000
#define MPG_PCR_RECEIVED            0x00008000
#define MPG_PCR_DISC                0x00010000
#define MPG_PTS_MATURED             0x00020000
#define MPG_IMAGE_RES_CHANGE        0x00040000
#define MPG_ASPECT_RATIO_CHANGE     0x00080000
#define MPG_GOP_TC_RECEIVED         0x00100000
#define MPG_VIDEO_SEQ_END           0x00200000
#define MPG_VIDEO_DEC_COMPLETE      0x00400000
#define MPG_VIDEO_DEC_INTERRUPT5    0x00800000
#define MPG_VIDEO_DEC_INTERRUPT6    0x01000000
#define MPG_VIDEO_DEC_INTERRUPT7    0x02000000
#define MPG_VIDEO_FRAME_CMD         0x00800000
#define MPG_V_SYNC_TOP              0x01000000
#define MPG_V_SYNC_BOTTOM           0x02000000
#define MPG_FSCODE_CHANGE           0x04000000
#define MPG_NEW_PTS_RECEIVED        0x08000000
#define MPG_VID_PTS_ARRIVED         0x10000000
#define MPG_AC3_PTS_ARRIVED         0x20000000
#define MPG_AC3_PTS_MATURED         0x40000000
#define MPG_VID_PTS_MATURED         0x80000000

/* End of bit position definitions */

/* Bit position definitions when using register as a HW_DWORD */
  #define MPG_BITSTREAM_ERROR     0x0000001
  #define MPG_ANC_OVERFLOW        0x0000010
  #define MPG_USER_OVERFLOW       0x0000020
/* End bit position definitions */

/* Bit position definitions when using register as a HW_DWORD */
  #define MPG_CMD_VALID                       0x0001
  #define MPG_UNREAD_USER                     0x0008
  #define MPG_UNREAD_ANC                      0x0010
  #define MPG_VIDEO_ERROR                     0x07C0
  #define MPG_AUDIO_ERROR                     0x1800
  #define MPG_TEST_DSP_DONE                   0x2000
  #define MPG_TEST_START_DSP                  0x4000

  /* Individual audio error indicators */
  #define MPG_AUDIO_CRC_ERROR 0x0800
  #define MPG_AUDIO_DATA_DRY  0x1000
  #define MPG_AUDIO_AC3_ALIGN 0x1800

/* End bit position definitions */

/* Definitions for bitfield access to aspect ratio field */
#define MPG_VID_ASPECT_FORBIDDEN             0x00
#define MPG_VID_ASPECT_11                    0x01
#define MPG_VID_ASPECT_43                    0x02
#define MPG_VID_ASPECT_149                   0x05
#define MPG_VID_ASPECT_169                   0x03
#define MPG_VID_ASPECT_209                   0x04

/* Masks and definitions for non-bitfields access to aspect ratio */
#define MPG_VID_ASPECT_RATIO_MASK   0xF0000000
#define MPG_VID_ASPECT_RATIO_11     0x10000000
#define MPG_VID_ASPECT_RATIO_43     0x20000000
#define MPG_VID_ASPECT_RATIO_149    0x50000000
#define MPG_VID_ASPECT_RATIO_169    0x30000000
#define MPG_VID_ASPECT_RATIO_209    0x40000000

typedef HW_DWORD volatile MPG_ISR;
typedef LPREG LPMPG_ISR;

typedef MPG_ISR MPG_IMR;
typedef LPMPG_ISR LPMPG_IMR;

typedef HW_BYTE * MPG_ADDRESS;
typedef MPG_ADDRESS volatile *LPMPG_ADDRESS;

#ifdef BITFIELDS
typedef struct _MPG_ADDR_EXT {
  BIT_FLD AddrExtension:3,
          Reserved:5,
          LastDecoded:2,
          TopField:1,
          Reserved1:1,
          LastDisplayed:2,
          InSync:1,
          Reserved2:17;
} MPG_ADDR_EXT;
typedef MPG_ADDR_EXT volatile *LPMPG_ADDR_EXT;

typedef struct _MPG_CONTROL0
{
  BIT_FLD Reserved1:2, /*MpegMode:2, */
          EnableInt:1,
          DownloadMCode:1,
          DownloadVideo:1,
          VideoCodeValid:1,
          AudioCodeValid:1,
		  PTSisAC3:1,
          Reserved2:9,
          MPEGActive:1,
          EnableSync:1,
          EntireBChroma:1,
          FullB:1,
          DisableSEQUserData:1,
          DisableGOPUserData:1,
          DisablePICUserData:1,
          EnableAspectConvert:1,
          EnablePanScan:1,
          NewPTSStored:1,
          NewPTSisVideo:1,
          LoadBackupSTC:1,
          SwParseMode:1,
          MPEGCoreReset:1,
          ComaReset:1;
} MPG_CONTROL0;
typedef MPG_CONTROL0 volatile *LPMPG_CONTROL0;

typedef struct _MPG_CONTROL1
{
  BIT_FLD Reserved1:2,
        KaraokeMode:2,
        EnableDownmix:1,
        DisableDialNorm:1,
        DisableHighDynRng:1,
        DisableLowDynRng:1,
        OutputSelect:1,
        LtRtMode:1,
        HaltDecode:1,
        SpdifIsAC3:1,
        SpdifProfMode:1,
        KaraokeCapable:1,
        Mute:1,
        OutputLeftJustified:1,
        LROutputCntrl:2,
        AudFrameStart:1,
        SpdifCntrl:2,
        SampFreq:3,
        AudioParmChange:1,
        EnableAudSync:1,
		DSSMode:1,
		EnableAC3Sync:1,
        Reserved3:2,
        EncVideoEnable:1,
        EncAudioEnable:1;
} MPG_CONTROL1;
typedef MPG_CONTROL1 volatile *LPMPG_CONTROL1;

typedef struct _MPG_COMMAND
{
  BIT_FLD Command:13,
        Reserved1:19;
} MPG_COMMAND;
typedef MPG_COMMAND volatile *LPMPG_COMMAND;

typedef struct _MPG_ERROR_STATUS
{
  BIT_FLD BitstreamError:1,
          Reserved1:3,
          AncOverflow:1,
          UserOverflow:1,
          Reserved2:26;
} MPG_ERROR_STATUS;

typedef MPG_ERROR_STATUS volatile *LPMPG_ERROR_STATUS;

typedef MPG_ERROR_STATUS MPG_ERROR_MASK;
typedef LPMPG_ERROR_STATUS LPMPG_ERROR_MASK;

typedef struct _MPG_DECODE_STATUS
{
  BIT_FLD CmdValid:1,
          Reserved0:2,
          UnreadUser:1,
          UnreadAnc:1,
          Reserved1:1,
          VideoError:5,
          AudioError:2,
          TestDspDone:1,
          TestStartDsp:1,
          Reserved2:17;
} MPG_DECODE_STATUS;
typedef MPG_DECODE_STATUS volatile *LPMPG_DECODE_STATUS;

typedef struct _MPG_PCR_TIMER_INFO
{
  BIT_FLD TimerValue:29,
        EnableTimer:1,
        Reserved1:2;
} MPG_PCR_TIMER_INFO;
typedef MPG_PCR_TIMER_INFO volatile *LPMPG_PCR_TIMER_INFO;

typedef MPG_PCR_TIMER_INFO MPG_PTS_TIMER_INFO;
typedef LPMPG_PCR_TIMER_INFO LPMPG_PTS_TIMER_INFO;

typedef struct _MPG_VID_SIZE
{
  BIT_FLD Width:10,
          Reserved1:6,
          Height:10,
          Reserved2:2,
          AspectRatio:4;
} MPG_VID_SIZE;
typedef MPG_VID_SIZE volatile *LPMPG_VID_SIZE;

typedef struct _MPG_GOP_TIME_CODE
{
  BIT_FLD BrokenLink:1,
          ClosedGOP:1,
          Pictures:6,
          Seconds:6,
          Minutes:6,
          Hours:5,
          DropFrame:1,
          Reserved1:6;
} MPG_GOP_TIME_CODE;
typedef MPG_GOP_TIME_CODE volatile *LPMPG_GOP_TIME_CODE;

typedef struct _MPG_FRAME_DROP_CNT
{
  BIT_FLD Drop:12,
          Repeat:12,
          Reserved1:8;
} MPG_FRAME_DROP_CNT;
typedef MPG_FRAME_DROP_CNT volatile *LPMPG_FRAME_DROP_CNT;

typedef MPG_FRAME_DROP_CNT   MPG_AUDIO_DROP_CNT;
typedef LPMPG_FRAME_DROP_CNT LPMPG_AUDIO_DROP_CNT;

typedef MPG_FRAME_DROP_CNT    MPG_AC3_DROP_CNT;
typedef LPMPG_FRAME_DROP_CNT LPMPG_AC3_DROP_CNT;

typedef struct _MPG_VPTS_L
{
  HW_DWORD Value;
} MPG_VPTS_L;

typedef MPG_VPTS_L volatile *LPMPG_VPTS_L;

typedef struct _MPG_VPTS_H
{
  BIT_FLD Msb:1,
          Reserved1:31;
} MPG_VPTS_H;

typedef MPG_VPTS_H volatile *LPMPG_VPTS_H;

typedef MPG_VPTS_L   MPG_APTS_C_PCRL;
typedef MPG_VPTS_H   MPG_APTS_C_PRCH;
typedef LPMPG_VPTS_L LPMPG_APTS_C_PCRL;
typedef LPMPG_VPTS_H LPMPG_APTS_C_PRCH;
#endif /* BITFIELDS */
#endif /* INCL_MPG */


/******************/
/* HSDP Registers */
/******************/

#ifdef INCL_HSDP

/*
 *******************************************************************************
 *
 *      HSDP_TSA_PORT_CNTL_REG                                 (HSDP_BASE + 0x00)
 *      HSDP_TSC_PORT_CNTL_REG                                 (HSDP_BASE + 0x08)
 *      HSDP_TSD_PORT_CNTL_REG                                 (HSDP_BASE + 0x0C)
 *      HSDP_TSE_PORT_CNTL_REG                                 (HSDP_BASE + 0x10)
 *      HSDP_TSF_PORT_CNTL_REG                                 (HSDP_BASE + 0x14)
 *      HSDP_TSG_PORT_CNTL_REG                                 (HSDP_BASE + 0x18)
 *      HSDP_SP_INPUT_CNTL_REG                                 (HSDP_BASE + 0x20)
 *      HSDP_SP_SPLICE_CNTL_REG                                (HSDP_BASE + 0x24)
 *      HSDP_TS_PKT_CNTL_REG                                   (HSDP_BASE + 0x40)
 *      HSDP_TS_ERR_STATUS_REG                                 (HSDP_BASE + 0x48)
 *      HSDP_TSC_GEN_CLK_CNTL                                  (HSDP_BASE + 0x58)
 *      HSDP_TSD_GEN_CLK_CNTL                                  (HSDP_BASE + 0x5C)
 *
 *******************************************************************************
 */

#define HSDP_TSA_PORT_CNTL_REG                                 (HSDP_BASE + 0x00)
#define     HSDP_SER_NIM_CNTL_INPUT_CLK_POLARITY_MASK              0x00000001
#define     HSDP_SER_NIM_CNTL_INPUT_CLK_POLARITY_SHIFT                    0
#define         HSDP_SER_NIM_CNTL_INPUT_CLK_POLARITY_RISING_EDGE     (0UL<<0)
#define         HSDP_SER_NIM_CNTL_INPUT_CLK_POLARITY_FALLING_EDGE    (1UL<<0)
#define     HSDP_SER_NIM_CNTL_ERROR_SIGNAL_POLARITY_MASK           0x00000002
#define     HSDP_SER_NIM_CNTL_ERROR_SIGNAL_POLARITY_SHIFT                 1
#define         HSDP_SER_NIM_CNTL_ERROR_SIGNAL_POLARITY_ACTIVE_HIGH  (0UL<<1)
#define         HSDP_SER_NIM_CNTL_ERROR_SIGNAL_POLARITY_ACTIVE_LOW   (1UL<<1)
#define     HSDP_SER_NIM_CNTL_IGNORE_FAIL_MASK                     0x00000004
#define     HSDP_SER_NIM_CNTL_IGNORE_FAIL_SHIFT                           2
#define         HSDP_SER_NIM_CNTL_USE_FAIL                           (0UL<<2)
#define         HSDP_SER_NIM_CNTL_IGNORE_FAIL                        (1UL<<2)
#define     HSDP_SER_NIM_CNTL_SER_TO_PAR_CONV_MASK                 0x00001000
#define     HSDP_SER_NIM_CNTL_SER_TO_PAR_CONV_SHIFT                      12
#define         HSDP_SER_NIM_CNTL_SER_TO_PAR_CONV_DISABLE           (0UL<<12)
#define         HSDP_SER_NIM_CNTL_SER_TO_PAR_CONV_ENABLE            (1UL<<12)

#define HSDP_TSC_PORT_CNTL_REG                                 (HSDP_BASE + 0x08)
#define HSDP_TSD_PORT_CNTL_REG                                 (HSDP_BASE + 0x0C)
#define     HSDP_BIDIR_PORT_CNTL_INPUT_CLK_POLARITY_MASK           0x00000001
#define     HSDP_BIDIR_PORT_CNTL_INPUT_CLK_POLARITY_SHIFT                 0
#define         HSDP_BIDIR_PORT_CNTL_INPUT_CLK_POLARITY_RISING_EDGE  (0UL<<0)
#define         HSDP_BIDIR_PORT_CNTL_INPUT_CLK_POLARITY_FALLING_EDGE (1UL<<0)
#define     HSDP_BIDIR_PORT_CNTL_ERROR_SIGNAL_POLARITY_MASK        0x00000002
#define     HSDP_BIDIR_PORT_CNTL_ERROR_SIGNAL_POLARITY_SHIFT              1
#define         HSDP_BIDIR_PORT_CNTL_ERROR_SIGNAL_POLARITY_ACTIVE_HIGH (0UL<<1)
#define         HSDP_BIDIR_PORT_CNTL_ERROR_SIGNAL_POLARITY_ACTIVE_LOW (1UL<<1)
#define     HSDP_BIDIR_PORT_CNTL_IGNORE_FAIL_MASK                  0x00000004
#define     HSDP_BIDIR_PORT_CNTL_IGNORE_FAIL_SHIFT                        2
#define         HSDP_BIDIR_PORT_CNTL_USE_FAIL                        (0UL<<2)
#define         HSDP_BIDIR_PORT_CNTL_IGNORE_FAIL                     (1UL<<2)
#define     HSDP_BIDIR_PORT_CNTL_MODE_MASK                         0x00000300
#define     HSDP_BIDIR_PORT_CNTL_MODE_SHIFT                               8
#define         HSDP_BIDIR_PORT_CNTL_MODE_CI_READ                    (0UL<<8)
#define         HSDP_BIDIR_PORT_CNTL_MODE_CI_WRITE                   (1UL<<8)
#define         HSDP_BIDIR_PORT_CNTL_MODE_1394_READ                  (2UL<<8)
#define         HSDP_BIDIR_PORT_CNTL_MODE_1394_WRITE                 (3UL<<8)
#define     HSDP_BIDIR_PORT_CNTL_SER_TO_PAR_CONV_MASK              0x00001000
#define     HSDP_BIDIR_PORT_CNTL_SER_TO_PAR_CONV_SHIFT                   12
#define         HSDP_BIDIR_PORT_CNTL_SER_TO_PAR_CONV_ENABLE         (1UL<<12)
#define         HSDP_BIDIR_PORT_CNTL_SER_TO_PAR_CONV_DISABLE        (0UL<<12)
#define     HSDP_BIDIR_PORT_CNTL_OUTPUT_MODES_ENABLE_MASK          0x00002000
#define     HSDP_BIDIR_PORT_CNTL_OUTPUT_MODES_ENABLE_SHIFT               13
#define         HSDP_BIDIR_PORT_CNTL_OUTPUT_MODES_ENABLE            (1UL<<13)
#define     HSDP_BIDIR_PORT_CNTL_OUTPUT_DATA_SEL_MASK              0x00030000
#define     HSDP_BIDIR_PORT_CNTL_OUTPUT_DATA_SEL_SHIFT                   16
#define         HSDP_BIDIR_PORT_CNTL_OUTPUT_DATA_SEL_DEMUX          (0UL<<16)
#define         HSDP_BIDIR_PORT_CNTL_OUTPUT_DATA_SEL_NIM            (1UL<<16)
#define         HSDP_BIDIR_PORT_CNTL_OUTPUT_DATA_SEL_DISABLE        (2UL<<16)
#define     HSDP_BIDIR_PORT_CNTL_OUTPUT_DATA_SUBSEL_MASK           0x000C0000
#define     HSDP_BIDIR_PORT_CNTL_OUTPUT_DATA_SUBSEL_SHIFT                18
#define         HSDP_BIDIR_PORT_CNTL_OUTPUT_DATA_SUBSEL_PARSER0     (0UL<<18)
#define         HSDP_BIDIR_PORT_CNTL_OUTPUT_DATA_SUBSEL_PARSER2     (2UL<<18)
#define         HSDP_BIDIR_PORT_CNTL_OUTPUT_DATA_SUBSEL_PARSER3     (3UL<<18)
#define     HSDP_BIDIR_PORT_CNTL_CLK_MODE_MASK                     0x00300000
#define     HSDP_BIDIR_PORT_CNTL_CLK_MODE_SHIFT                          20
#define         HSDP_BIDIR_PORT_CNTL_CLK_MODE_INPUT                 (0UL<<20)
#define         HSDP_BIDIR_PORT_CNTL_CLK_MODE_HSDP_CLK              (1UL<<20)
#define         HSDP_BIDIR_PORT_CNTL_CLK_MODE_GEN_CLK               (2UL<<20)
#define     HSDP_BIDIR_PORT_CNTL_TS_WRAP_SUBSEL_MASK               0x07000000
#define     HSDP_BIDIR_PORT_CNTL_TS_WRAP_SUBSEL_SHIFT                    24
#define         HSDP_BIDIR_PORT_CNTL_TS_WRAP_SUBSEL_TSA             (0UL<<24)
#define         HSDP_BIDIR_PORT_CNTL_TS_WRAP_SUBSEL_TSE             (4UL<<24)
#define         HSDP_BIDIR_PORT_CNTL_TS_WRAP_SUBSEL_TSF             (5UL<<24)
#define         HSDP_BIDIR_PORT_CNTL_TS_WRAP_SUBSEL_TSG             (6UL<<24)

#define HSDP_TSE_PORT_CNTL_REG                                 (HSDP_BASE + 0x10)
#define HSDP_TSF_PORT_CNTL_REG                                 (HSDP_BASE + 0x14)
#define HSDP_TSG_PORT_CNTL_REG                                 (HSDP_BASE + 0x18)
#define     HSDP_PAR_NIM_CNTL_INPUT_CLK_POLARITY_MASK              0x00000001
#define     HSDP_PAR_NIM_CNTL_INPUT_CLK_POLARITY_SHIFT                    0
#define         HSDP_PAR_NIM_CNTL_INPUT_CLK_POLARITY_RISING_EDGE     (0UL<<0)
#define         HSDP_PAR_NIM_CNTL_INPUT_CLK_POLARITY_FALLING_EDGE    (1UL<<0)
#define     HSDP_PAR_NIM_CNTL_ERROR_SIGNAL_POLARITY_MASK           0x00000002
#define     HSDP_PAR_NIM_CNTL_ERROR_SIGNAL_POLARITY_SHIFT                 1
#define         HSDP_PAR_NIM_CNTL_ERROR_SIGNAL_POLARITY_ACTIVE_HIGH  (0UL<<1)
#define         HSDP_PAR_NIM_CNTL_ERROR_SIGNAL_POLARITY_ACTIVE_LOW   (1UL<<1)
#define     HSDP_PAR_NIM_CNTL_IGNORE_FAIL_MASK                     0x00000004
#define     HSDP_PAR_NIM_CNTL_IGNORE_FAIL_SHIFT                           2
#define         HSDP_PAR_NIM_CNTL_USE_FAIL                           (0UL<<2)
#define         HSDP_PAR_NIM_CNTL_IGNORE_FAIL                        (1UL<<2)

#define HSDP_SP_INPUT_CNTL_REG                                 (HSDP_BASE + 0x20)
#define     HSDP_SP_INPUT_CNTL_PARSER0_INPUT_SEL_MASK              0x00000007
#define     HSDP_SP_INPUT_CNTL_PARSER0_INPUT_SEL_SHIFT                    0
#define         HSDP_SP_INPUT_CNTL_PARSER0_SER_NIM_PORT_0            (0UL<<0)
#define         HSDP_SP_INPUT_CNTL_PARSER0_BIDIR_PORT_0              (2UL<<0)
#define         HSDP_SP_INPUT_CNTL_PARSER0_BIDIR_PORT_1              (3UL<<0)
#define         HSDP_SP_INPUT_CNTL_PARSER0_PAR_NIM_PORT_0            (4UL<<0)
#define         HSDP_SP_INPUT_CNTL_PARSER0_PAR_NIM_PORT_1            (5UL<<0)
#define         HSDP_SP_INPUT_CNTL_PARSER0_PAR_NIM_PORT_2            (6UL<<0)
#define     HSDP_SP_INPUT_CNTL_PARSER2_INPUT_SEL_MASK              0x00070000
#define     HSDP_SP_INPUT_CNTL_PARSER2_INPUT_SEL_SHIFT                   16
#define         HSDP_SP_INPUT_CNTL_PARSER2_SER_NIM_PORT_0           (0UL<<16)
#define         HSDP_SP_INPUT_CNTL_PARSER2_BIDIR_PORT_0             (2UL<<16)
#define         HSDP_SP_INPUT_CNTL_PARSER2_BIDIR_PORT_1             (3UL<<16)
#define         HSDP_SP_INPUT_CNTL_PARSER2_PAR_NIM_PORT_0           (4UL<<16)
#define         HSDP_SP_INPUT_CNTL_PARSER2_PAR_NIM_PORT_1           (5UL<<16)
#define         HSDP_SP_INPUT_CNTL_PARSER2_PAR_NIM_PORT_2           (6UL<<16)
#define     HSDP_SP_INPUT_CNTL_PARSER3_INPUT_SEL_MASK              0x07000000
#define     HSDP_SP_INPUT_CNTL_PARSER3_INPUT_SEL_SHIFT                   24
#define         HSDP_SP_INPUT_CNTL_PARSER3_SER_NIM_PORT_0           (0UL<<24)
#define         HSDP_SP_INPUT_CNTL_PARSER3_BIDIR_PORT_0             (2UL<<24)
#define         HSDP_SP_INPUT_CNTL_PARSER3_BIDIR_PORT_1             (3UL<<24)
#define         HSDP_SP_INPUT_CNTL_PARSER3_PAR_NIM_PORT_0           (4UL<<24)
#define         HSDP_SP_INPUT_CNTL_PARSER3_PAR_NIM_PORT_1           (5UL<<24)
#define         HSDP_SP_INPUT_CNTL_PARSER3_PAR_NIM_PORT_2           (6UL<<24)

#define HSDP_SP_SPLICE_CNTL_REG                                (HSDP_BASE + 0x24)
#define     HSDP_SPLICE_CTRL_SER_NIM_PORT_0_MASK                   0x00000001
#define     HSDP_SPLICE_CTRL_SER_NIM_PORT_0_SHIFT                         0
#define         HSDP_SPLICE_CTRL_SER_NIM_PORT_0_ENABLE               (1UL<<0)
#define     HSDP_SPLICE_CTRL_BIDIR_PORT_0_MASK                     0x00000004
#define     HSDP_SPLICE_CTRL_BIDIR_PORT_0_SHIFT                           2
#define         HSDP_SPLICE_CTRL_BIDIR_PORT_0_ENABLE                 (1UL<<2)
#define     HSDP_SPLICE_CTRL_BIDIR_PORT_1_MASK                     0x00000008
#define     HSDP_SPLICE_CTRL_BIDIR_PORT_1_SHIFT                           3
#define         HSDP_SPLICE_CTRL_BIDIR_PORT_1_ENABLE                 (1UL<<3)
#define     HSDP_SPLICE_CTRL_PAR_NIM_PORT_0_MASK                   0x00000010
#define     HSDP_SPLICE_CTRL_PAR_NIM_PORT_0_SHIFT                         4
#define         HSDP_SPLICE_CTRL_PAR_NIM_PORT_0_ENABLE               (1UL<<4)
#define     HSDP_SPLICE_CTRL_PAR_NIM_PORT_1_MASK                   0x00000020
#define     HSDP_SPLICE_CTRL_PAR_NIM_PORT_1_SHIFT                         5
#define         HSDP_SPLICE_CTRL_PAR_NIM_PORT_1_ENABLE               (1UL<<5)
#define     HSDP_SPLICE_CTRL_PAR_NIM_PORT_2_MASK                   0x00000040
#define     HSDP_SPLICE_CTRL_PAR_NIM_PORT_2_SHIFT                         6
#define         HSDP_SPLICE_CTRL_PAR_NIM_PORT_2_ENABLE               (1UL<<6)
#define     HSDP_SPLICE_STATUS_SER_NIM_PORT_0_MASK                 0x00000100
#define     HSDP_SPLICE_STATUS_SER_NIM_PORT_0_SHIFT                       8
#define         HSDP_SPLICE_STATUS_SER_NIM_PORT_0_ENABLE             (1UL<<8)
#define     HSDP_SPLICE_STATUS_BIDIR_PORT_0_MASK                   0x00000400
#define     HSDP_SPLICE_STATUS_BIDIR_PORT_0_SHIFT                        10
#define         HSDP_SPLICE_STATUS_BIDIR_PORT_0_ENABLE              (1UL<<10)
#define     HSDP_SPLICE_STATUS_BIDIR_PORT_1_MASK                   0x00000800
#define     HSDP_SPLICE_STATUS_BIDIR_PORT_1_SHIFT                        11
#define         HSDP_SPLICE_STATUS_BIDIR_PORT_1_ENABLE              (1UL<<11)
#define     HSDP_SPLICE_STATUS_PAR_NIM_PORT_0_MASK                 0x00001000
#define     HSDP_SPLICE_STATUS_PAR_NIM_PORT_0_SHIFT                      12
#define         HSDP_SPLICE_STATUS_PAR_NIM_PORT_0_ENABLE            (1UL<<12)
#define     HSDP_SPLICE_STATUS_PAR_NIM_PORT_1_MASK                 0x00002000
#define     HSDP_SPLICE_STATUS_PAR_NIM_PORT_1_SHIFT                      13
#define         HSDP_SPLICE_STATUS_PAR_NIM_PORT_1_ENABLE            (1UL<<13)
#define     HSDP_SPLICE_STATUS_PAR_NIM_PORT_2_MASK                 0x00004000
#define     HSDP_SPLICE_STATUS_PAR_NIM_PORT_2_SHIFT                      14
#define         HSDP_SPLICE_STATUS_PAR_NIM_PORT_2_ENABLE            (1UL<<14)

#define HSDP_TS_PKT_CNTL_REG                                   (HSDP_BASE + 0x40)
#define     HSDP_TS_PKT_CNTL_PKT_SIZE_MASK                         0x000000FF
#define     HSDP_TS_PKT_CNTL_PKT_SIZE_SHIFT                               0
#define     HSDP_TS_PKT_CNTL_SYNC_BYTE_CODE_MASK                   0x0000FF00
#define     HSDP_TS_PKT_CNTL_SYNC_BYTE_CODE_SHIFT                         8
#define     HSDP_TS_PKT_CNTL_OUTPUT_FIFO_THRESHOLD_MASK            0x007C0000
#define     HSDP_TS_PKT_CNTL_OUTPUT_FIFO_THRESHOLD_SHIFT                 18
#define     HSDP_TS_PKT_CNTL_DSS_PKT_MODE_MASK                     0x01000000
#define     HSDP_TS_PKT_CNTL_DSS_PKT_MODE_SHIFT                          24
#define         HSDP_TS_PKT_CNTL_DSS_PKT_MODE_ENABLE                (1UL<<24)
#define     HSDP_TS_PKT_CNTL_ERROR_SIGNAL_FUNC_MASK                0x02000000
#define     HSDP_TS_PKT_CNTL_ERROR_SIGNAL_FUNC_SHIFT                     25
#define         HSDP_TS_PKT_CNTL_HOLD_PKT_ERROR                     (1UL<<25)

#define HSDP_TS_ERR_STATUS_REG                                 (HSDP_BASE + 0x48)
#define     HSDP_ERR_STATUS_LOST_SYNC_ON_BIDIR_PORT_0_MASK         0x00000001
#define     HSDP_ERR_STATUS_LOST_SYNC_ON_BIDIR_PORT_0_SHIFT               0
#define         HSDP_ERR_STATUS_LOST_SYNC_ON_BIDIR_PORT_0            (1UL<<0)
#define     HSDP_ERR_STATUS_PORT_0_OUTPUT_OVERRUN_MASK             0x00000002
#define     HSDP_ERR_STATUS_PORT_0_OUTPUT_OVERRUN_SHIFT                   1
#define         HSDP_ERR_STATUS_PORT_0_OUTPUT_OVERRUN                (1UL<<1)
#define     HSDP_ERR_STATUS_LOST_SYNC_ON_BIDIR_PORT_1_MASK         0x00000100
#define     HSDP_ERR_STATUS_LOST_SYNC_ON_BIDIR_PORT_1_SHIFT               8
#define         HSDP_ERR_STATUS_LOST_SYNC_ON_BIDIR_PORT_1            (1UL<<8)
#define     HSDP_ERR_STATUS_PORT_1_OUTPUT_OVERRUN_MASK             0x00000200
#define     HSDP_ERR_STATUS_PORT_1_OUTPUT_OVERRUN_SHIFT                   9
#define         HSDP_ERR_STATUS_PORT_1_OUTPUT_OVERRUN                (1UL<<9)

#define HSDP_TSC_GEN_CLK_CNTL                                  (HSDP_BASE + 0x58)
#define HSDP_TSD_GEN_CLK_CNTL                                  (HSDP_BASE + 0x5C)
#define     HSDP_CLK_CNTL_CLK_HI_CNT_MASK                          0x000000FF
#define     HSDP_CLK_CNTL_CLK_HI_CNT_SHIFT                                0
#define     HSDP_CLK_CNTL_TOTAL_CLK_CNT_MASK                       0x0000FF00
#define     HSDP_CLK_CNTL_TOTAL_CLK_CNT_SHIFT                             8

#endif


/**************/
/* DVB Parser */
/**************/

#ifdef INCL_DPS

/*
 *******************************************************************************
 *
 *  There are three mapping sections each with a header.  For Wabash, there
 *  are three demuxes/pawsers in the chip (0, 2, & 3)
 *
 *                         Programmable Parser Memory Map
 *
 *      DPS_RESET_VECTOR_ADDR_EX                         (DPS_BASE + 0x00000000)
 *      DPS_DBG_DUMP_PTR_EX                              (DPS_BASE + 0x00000004)
 *      DPS_DBG_BRK_VECTOR_EX                            (DPS_BASE + 0x00000008)
 *      DPS_VERSION_NUM_EX                               (DPS_BASE + 0x0000000C)
 *      DPS_CAPABILITIES_EX                              (DPS_BASE + 0x00000010)
 *      DPS_DVB_PRIV_DATA_EX                             (DPS_BASE + 0x00000014)
 *      DPS_NEGATIVE_MODE_REG_EX                         (DPS_BASE + 0x00000050)
 *      DPS_PTS_OFFSET_VIDEO_EX                          (DPS_BASE + 0x00000054)
 *      DPS_PTS_OFFSET_AUDIO_EX                          (DPS_BASE + 0x00000058)
 *      DPS_DVR_TRANSPORT_BLK_SIZE_EX                    (DPS_BASE + 0x0000005C)
 *      DPS_DVB_PRIV_DATA2_EX                            (DPS_BASE + 0x0000005C)
 *      DPS_VID_SPLICE_PID_REG_EX                        (DPS_BASE + 0x00000060)
 *      DPS_AUD_SPLICE_PID_REG_EX                        (DPS_BASE + 0x00000064)
 *      DPS_PARSER_STATUS_REG_EX                         (DPS_BASE + 0x00000068)
 *      DPS_TRANSPORT_PID_REG_EX                         (DPS_BASE + 0x0000006C)
 *      DPS_PES_PID_REG_EX                               (DPS_BASE + 0x00000070)
 *      DPS_REC_MON_PID_TABLE_EX                         (DPS_BASE + 0x00000070)
 *      DPS_PCR_PID_EX                                   (DPS_BASE + 0x00000074)
 *      DPS_KEY_ENABLE_REG_EX                            (DPS_BASE + 0x00000078)
 *      DPS_VERSION_MODES_REG_EX                         (DPS_BASE + 0x0000007C)
 *      DPS_PID_BASE_EX                                  (DPS_BASE + 0x00000080)
 *      DPS_FILTER_CONTROL_BASE_EX                       (DPS_BASE + 0x00000100)
 *      DPS_KEY_TABLE_BASE_EX                            (DPS_BASE + 0x00000180)
 *      DPS_TRANSPORT_WRITE_PTR_EX                       (DPS_BASE + 0x00000200)
 *      DPS_TRANSPORT_READ_PTR_EX                        (DPS_BASE + 0x00000204)
 *      DPS_TRANSPORT_START_ADDR_EX                      (DPS_BASE + 0x00000208)
 *      DPS_TRANSPORT_END_ADDR_EX                        (DPS_BASE + 0x0000020C)
 *      DPS_REC_EVENT_WRITE_PTR_EX                       (DPS_BASE + 0x00000210)
 *      DPS_REC_EVENT_READ_PTR_EX                        (DPS_BASE + 0x00000214)
 *      DPS_REC_EVENT_START_ADDR_EX                      (DPS_BASE + 0x00000218)
 *      DPS_REC_EVENT_END_ADDR_EX                        (DPS_BASE + 0x0000021C)
 *      DPS_TS_BLOCK_COUNT_EX                            (DPS_BASE + 0x00000278)
 *      DPS_HOST_CMD_WORD_EX                             (DPS_BASE + 0x0000027C)
 *      DPS_PLAY_VCORE_CMD_BUFFER_EX                     (DPS_BASE + 0x00000280)
 *      DPS_PID_SLOT_WRITE_PTR_EX(slot)      (DPS_BASE + 0x00000280 + (slot<<2))
 *      DPS_PID_SLOT_READ_PTR_EX(slot)       (DPS_BASE + 0x00000284 + (slot<<2))
 *      DPS_PID_SLOT_START_ADDR_EX(slot)     (DPS_BASE + 0x00000288 + (slot<<2))
 *      DPS_PID_SLOT_END_ADDR_EX(slot)       (DPS_BASE + 0x0000028C + (slot<<2))
 *      DPS_DVR_VIDEO_WRITE_PTR_EX                       (DPS_BASE + 0x00000280)
 *      DPS_DVR_VIDEO_READ_PTR_EX                        (DPS_BASE + 0x00000284)
 *      DPS_DVR_VIDEO_START_ADDR_EX                      (DPS_BASE + 0x00000288)
 *      DPS_DVR_VIDEO_END_ADDR_EX                        (DPS_BASE + 0x0000028C)
 *      DPS_DVR_AUDIO_WRITE_PTR_EX                       (DPS_BASE + 0x00000290)
 *      DPS_DVR_AUDIO_READ_PTR_EX                        (DPS_BASE + 0x00000294)
 *      DPS_DVR_AUDIO_START_ADDR_EX                      (DPS_BASE + 0x00000298)
 *      DPS_DVR_AUDIO_END_ADDR_EX                        (DPS_BASE + 0x0000029C)
 *      DPS_DVR_AUDIO1_WRITE_PTR_EX                      (DPS_BASE + 0x000002A0)
 *      DPS_DVR_AUDIO1_READ_PTR_EX                       (DPS_BASE + 0x000002A4)
 *      DPS_DVR_AUDIO1_START_ADDR_EX                     (DPS_BASE + 0x000002A8)
 *      DPS_DVR_AUDIO1_END_ADDR_EX                       (DPS_BASE + 0x000002AC)
 *      DPS_DVR_AUDIO2_WRITE_PTR_EX                      (DPS_BASE + 0x000002B0)
 *      DPS_DVR_AUDIO2_READ_PTR_EX                       (DPS_BASE + 0x000002B4)
 *      DPS_DVR_AUDIO2_START_ADDR_EX                     (DPS_BASE + 0x000002B8)
 *      DPS_DVR_AUDIO2_END_ADDR_EX                       (DPS_BASE + 0x000002BC)
 *      DPS_DVR_EVENT_WRITE_PTR_EX                       (DPS_BASE + 0x000002C0)
 *      DPS_DVR_EVENT_READ_PTR_EX                        (DPS_BASE + 0x000002C4)
 *      DPS_DVR_EVENT_START_ADDR_EX                      (DPS_BASE + 0x000002C8)
 *      DPS_DVR_EVENT_END_ADDR_EX                        (DPS_BASE + 0x000002CC)
 *      DPS_PSI_OVERFLOW_INDEX_EX                        (DPS_BASE + 0x000008D0)
 *      DPS_NOTIFY_CRC_EX                                (DPS_BASE + 0x000008D4)
 *      DPS_PTS_OFFSET_HI                                (DPS_BASE + 0x000008D8)
 *      DPS_PES_FRAME_RATE                               (DPS_BASE + 0x000008DC)
 *      DPS_SCRAMBLING_STATUS_TS                         (DPS_BASE + 0x000008E4)
 *      DPS_SCRAMBLING_STATUS_PES                        (DPS_BASE + 0x000008E8)
 *      DPS_DMA_MUX_SELECT_EX                            (DPS_BASE + 0x000008EC)
 *      DPS_AUDIO_PID_STREAM_ID_EX                       (DPS_BASE + 0x000008EC)
 *      DPS_DBG_DUMP_CPU_REG0_EX                         (DPS_BASE + 0x00002300)
 *      DPS_DBG_DUMP_CPU_REG1_EX                         (DPS_BASE + 0x00002304)
 *      DPS_DBG_DUMP_CPU_REG2_EX                         (DPS_BASE + 0x00002308)
 *      DPS_DBG_DUMP_CPU_REG3_EX                         (DPS_BASE + 0x0000230C)
 *      DPS_DBG_DUMP_CPU_REG4_EX                         (DPS_BASE + 0x00002310)
 *      DPS_DBG_DUMP_CPU_REG5_EX                         (DPS_BASE + 0x00002314)
 *      DPS_DBG_DUMP_CPU_REG6_EX                         (DPS_BASE + 0x00002318)
 *      DPS_DBG_DUMP_CPU_REG7_EX                         (DPS_BASE + 0x0000231C)
 *      DPS_DBG_DUMP_CPU_REG8_EX                         (DPS_BASE + 0x00002320)
 *      DPS_DBG_DUMP_CPU_REG9_EX                         (DPS_BASE + 0x00002324)
 *      DPS_DBG_DUMP_CPU_REG10_EX                        (DPS_BASE + 0x00002328)
 *      DPS_DBG_DUMP_CPU_REG11_EX                        (DPS_BASE + 0x0000232C)
 *      DPS_DBG_DUMP_CPU_REG12_EX                        (DPS_BASE + 0x00002330)
 *      DPS_DBG_DUMP_CPU_REG13_EX                        (DPS_BASE + 0x00002334)
 *      DPS_DBG_DUMP_CPU_REG14_EX                        (DPS_BASE + 0x00002338)
 *      DPS_DBG_DUMP_CPU_REG15_EX                        (DPS_BASE + 0x0000233C)
 *      DPS_DBG_DUMP_CURRENT_PID_EX                      (DPS_BASE + 0x00002340)
 *      DPS_DBG_DUMP_PKT_NUM_EX                          (DPS_BASE + 0x00002348)
 *      DPS_DBG_DUMP_PSI_BUFF_FULL_DROP_CNT_EX           (DPS_BASE + 0x0000234C)
 *      DPS_DBG_DUMP_PSI_CRC_DROP_CNT_EX                 (DPS_BASE + 0x0000236C)
 *      DPS_DBG_DUMP_PSI_PKT_LOSS_DROP_CNT_EX            (DPS_BASE + 0x00002370)
 *      DPS_DBG_DUMP_PSI_FILTER_DROP_CNT_EX              (DPS_BASE + 0x00002374)
 *      DPS_DBG_DUMP_PSI_SYNTAX_DROP_CNT_EX              (DPS_BASE + 0x00002378)
 *
 *                         Programmable Parser I/O Map
 *
 *      DPS_PCR_HIGH_EX                                  (DPS_BASE + 0x01000000)
 *      DPS_PCR_LOW_EX                                   (DPS_BASE + 0x01000004)
 *      DPS_PCR_EXTENSION_EX                             (DPS_BASE + 0x01000008)
 *      DPS_VIDEO_PTS_HIGH_EX                            (DPS_BASE + 0x01000020)
 *      DPS_VIDEO_PTS_LOW_EX                             (DPS_BASE + 0x01000024)
 *      DPS_AUDIO_PTS_HIGH_EX                            (DPS_BASE + 0x01000030)
 *      DPS_AUDIO_PTS_LOW_EX                             (DPS_BASE + 0x01000034)
 *      DPS_AC3_PTS_HIGH_EX                              (DPS_BASE + 0x01000040)
 *      DPS_AC3_PTS_LOW_EX                               (DPS_BASE + 0x01000044)
 *      DPS_AC3_BUFFER_LEVEL_EX                          (DPS_BASE + 0x01000064)
 *      DPS_VIDEO_BUFFER_LEVEL_EX                        (DPS_BASE + 0x01000068)
 *      DPS_AUDIO_BUFFER_LEVEL_EX                        (DPS_BASE + 0x0100006C)
 *      DPS_AC3_LOWWATERMARK_EX                          (DPS_BASE + 0x01000074)
 *      DPS_VIDEO_LOWWATERMARK_EX                        (DPS_BASE + 0x01000078)
 *      DPS_AUDIO_LOWWATERMARK_EX                        (DPS_BASE + 0x0100007C)
 *      DPS_VIDEO_WRITE_PTR_EX                           (DPS_BASE + 0x01000080)
 *      DPS_VIDEO_READ_PTR_EX                            (DPS_BASE + 0x01000084)
 *      DPS_VIDEO_START_ADDR_EX                          (DPS_BASE + 0x01000088)
 *      DPS_VIDEO_END_ADDR_EX                            (DPS_BASE + 0x0100008C)
 *      DPS_AUDIO_WRITE_PTR_EX                           (DPS_BASE + 0x01000090)
 *      DPS_AUDIO_READ_PTR_EX                            (DPS_BASE + 0x01000094)
 *      DPS_AUDIO_START_ADDR_EX                          (DPS_BASE + 0x01000098)
 *      DPS_AUDIO_END_ADDR_EX                            (DPS_BASE + 0x0100009C)
 *      DPS_USER_WRITE_PTR_EX                            (DPS_BASE + 0x010000A0)
 *      DPS_USER_READ_PTR_EX                             (DPS_BASE + 0x010000A4)
 *      DPS_USER_START_ADDR_EX                           (DPS_BASE + 0x010000A8)
 *      DPS_AUD_ANC_WRITE_PTR_EX                         (DPS_BASE + 0x010000B0)
 *      DPS_AUD_ANC_READ_PTR_EX                          (DPS_BASE + 0x010000B4)
 *      DPS_AUD_ANC_START_ADDR_EX                        (DPS_BASE + 0x010000B8)
 *      DPS_AUD_ANC_END_ADDR_EX                          (DPS_BASE + 0x010000BC)
 *      DPS_MEM_CLK_CNT_REG_EX                           (DPS_BASE + 0x01000100)
 *      DPS_MPG_CLK_CNT_REG_EX                           (DPS_BASE + 0x01000104)
 *      DPS_MPG_27MHZ_CLK_CNT_REG_EX                     (DPS_BASE + 0x01000108)
 *      DPS_AC3_WRITE_PTR_EX                             (DPS_BASE + 0x01000180)
 *      DPS_AC3_READ_PTR_EX                              (DPS_BASE + 0x01000184)
 *      DPS_AC3_START_ADDR_EX                            (DPS_BASE + 0x01000188)
 *      DPS_AC3_END_ADDR_EX                              (DPS_BASE + 0x0100018C)
 *
 *                         Host I/O Map
 *
 *      DPS_HOST_CTL_REG_EX                              (DPS_BASE + 0x01800000)
 *      DPS_INFO_CHANGE_REG_EX                           (DPS_BASE + 0x01800004)
 *      DPS_ISR_REG_EX                                   (DPS_BASE + 0x01800008)
 *      DPS_INT_ENABLE_REG_EX                            (DPS_BASE + 0x0180000C)
 *      DPS_EVENT_STATUS_REG_EX                          (DPS_BASE + 0x01800010)
 *      DPS_EVENT_ENABLE_REG_EX                          (DPS_BASE + 0x01800014)
 *      DPS_PARSER_CTL_REG_EX                            (DPS_BASE + 0x01800018)
 *      DPS_MEM_PAGE_EX                                  (DPS_BASE + 0x0180001C)
 *      DPS_TS_IN_BUFFER_FREE_EX                         (DPS_BASE + 0x01800020)
 *      DPS_TS_OUT_BUFFER_FREE_EX                        (DPS_BASE + 0x01800024)
 *      DPS_EXT_EVENT_STATUS_REG_EX                      (DPS_BASE + 0x01800028)
 *      DPS_FLUSH_STATUS_REG_EX                          (DPS_BASE + 0x0180002C)
 *      DPS_CMD_CTRL_EX                                  (DPS_BASE + 0x01800030)
 *      DPS_PATTERN_BASE_EX                              (DPS_BASE + 0x01800800)
 *      DPS_FILTER_MASK_BASE_EX                          (DPS_BASE + 0x01800A00)
 *      DPS_FILTER_MODE_BASE_EX                          (DPS_BASE + 0x01800C00)
 *      DPS_DMA_WINDOW_IN_EX                             (DPS_BASE + 0x01C00000)
 *      DPS_DMA_WINDOW_OUT_EX                            (DPS_BASE + 0x01D00000)
 *
 *******************************************************************************
 */

#define NUM_PID_SLOTS                                                    32
#define DPS_NUM_DEMUXES                                         (NUM_PARSERS+1)

#define     DPS_PID_MASK                                           0x00001fff
#define     DPS_BUFFER_WRAP_INDICATOR                              0x80000000

/*
 * Parser Memory Offset
 */

#define DPS_PAW_SYS_ADDRESS                                        0x02000000

/*
 * Useful typecasts
 */

typedef HW_DWORD DPS_PID;
typedef LPREG LPDPS_PID;
typedef HW_DWORD DPS_FILTER_ENABLE;
typedef LPREG LPDPS_FILTER_ENABLE;
typedef LPREG LPDPS_NEGATIVE_MODE_REG;
typedef HW_BYTE * DPS_ADDRESS;
typedef DPS_ADDRESS volatile *LPDPS_ADDRESS;

/*
 * Programmable Parser Memory Map
 */
#define DPS_RESET_VECTOR_ADDR_EX(instance)                                     \
                              ((volatile HW_DWORD *)(DPS_BASE(instance)+0x0000))
#define DPS_DBG_DUMP_PTR_EX(instance)                                          \
                              ((volatile HW_DWORD *)(DPS_BASE(instance)+0x0004))
#define DPS_DBG_BRK_VECTOR_EX(instance)                                        \
                              ((volatile HW_DWORD *)(DPS_BASE(instance)+0x0008))

#define DPS_VERSION_NUM_EX(instance)                                           \
                              ((volatile HW_DWORD *)(DPS_BASE(instance)+0x000C))
#define     DPS_REVISION_MASK                                      0x0000000f
#define     DPS_REVISION_SHIFT                                            0
#define     DPS_VERSION_MINOR_VERSION_MASK                         0x000000f0
#define     DPS_VERSION_MINOR_VERSION_SHIFT                               4
#define     DPS_VERSION_MAJOR_VERSION_MASK                         0x0000ff00
#define     DPS_VERSION_MAJOR_VERSION_SHIFT                               8
#define     DPS_VERSION_CHIP_VERSION_MASK                          0x00ff0000
#define     DPS_VERSION_CHIP_VERSION_SHIFT                               16
#define         DPS_VERSION_CHIP_VERSION_COL_REV_A                  (1UL<<16)
#define         DPS_VERSION_CHIP_VERSION_COL_REV_C                  (2UL<<16)
#define         DPS_VERSION_CHIP_VERSION_HONDO                      (3UL<<16)
#define     DPS_VERSION_UCODE_TYPE_MASK                            0xff000000
#define     DPS_VERSION_UCODE_TYPE_SHIFT                                 24
#define         DPS_VERSION_UCODE_TYPE_DVB                          (1UL<<24)
#define         DPS_VERSION_UCODE_TYPE_DSS                          (2UL<<24)
#define         DPS_VERSION_UCODE_TYPE_DVB_PVR_REC                  (3UL<<24)
#define         DPS_VERSION_UCODE_TYPE_DSS_PVR_REC                  (4UL<<24)
#define         DPS_VERSION_UCODE_TYPE_DVB_XTV_REC                  (5UL<<24)

#define DPS_CAPABILITIES_EX(instance)                                          \
                              ((volatile HW_DWORD *)(DPS_BASE(instance)+0x0010))
#define         DPS_CAPABILITIES_DEBUG                               (1UL<<0)
#define         DPS_CAPABILITIES_ACTV                                (1UL<<1)
#define         DPS_CAPABILITIES_FILTERED_TS_STREAM                  (1UL<<2)
#define         DPS_CAPABILITIES_FULL_TS_STREAM_STOR                 (1UL<<3)
#define         DPS_CAPABILITIES_BYTESWAP                            (1UL<<5)
#define         DPS_CAPABILITIES_DVR_CAPABLE                         (1UL<<6)
#define     DPS_CAPABILITIES_NUM_AUD_CHAN_MASK                     0x00000180
#define     DPS_CAPABILITIES_NUM_AUD_CHAN_SHIFT                           7
#define         DPS_CAPABILITIES_NUM_AUD_CHAN_1                      (0UL<<7)
#define         DPS_CAPABILITIES_NUM_AUD_CHAN_2                      (1UL<<7)
#define         DPS_CAPABILITIES_NUM_AUD_CHAN_3                      (2UL<<7)
#define     DPS_CAPABILITIES_SECT_FILTER_MODE_MASK                 0x00000200
#define     DPS_CAPABILITIES_SECT_FILTER_MODE_SHIFT                       9
#define         DPS_CAPABILITIES_SECT_FILTER_12_12                   (0UL<<9)
#define         DPS_CAPABILITIES_SECT_FILTER_8_8_8                   (1UL<<9)
#define         DPS_CAPABILITIES_PES_PKT_TO_PSI                     (1UL<<10)
#define         DPS_CAPABILITIES_PTS_OFFSET                         (1UL<<11)
#define     DPS_CAPABILITIES_TS_IN_SRC_SEL_MASK                    0x00001000
#define     DPS_CAPABILITIES_TS_IN_SRC_SEL_SHIFT                         12
#define         DPS_CAPABILITIES_TS_IN_SRC_SEL_NIM                  (0UL<<12)
#define         DPS_CAPABILITIES_TS_IN_SRC_SEL_DMA                  (1UL<<12)
#define     DPS_CAPABILITIES_TS_OUT_SRC_SEL_MASK                   0x00002000
#define     DPS_CAPABILITIES_TS_OUT_SRC_SEL_SHIFT                        13
#define         DPS_CAPABILITIES_TS_OUT_SRC_SEL_NIM                 (0UL<<13)
#define         DPS_CAPABILITIES_TS_OUT_SEC_SEL_DMA                 (1UL<<13)
#define         DPS_CAPABILITIES_AC3                                (1UL<<14)
#define         DPS_CAPABILITIES_DES                                (1UL<<15)
#define         DPS_CAPABILITIES_PVR_PLAY                           (1UL<<16)
#define         DPS_CAPABILITIES_AC3_SPDIF_FIX                      (1UL<<17)
#define         DPS_CAPABILITIES_DESCRAM_FIX                        (1UL<<18)
#define         DPS_CAPABILITIES_SYNC_IN                            (1UL<<19)
#define         DPS_CAPABILITIES_KEY64                              (1UL<<20)
#define         DPS_CAPABILITIES_PCR_PID_INT                        (1UL<<21)
#define         DPS_CAPABILITIES_DMA_MUX                            (1UL<<22)
#define         DPS_CAPABILITIES_NO_KEY_STATUS                      (1UL<<23)
#define         DPS_CAPABILITIES_AUD_KEY_INT                        (1UL<<24)
#define         DPS_CAPABILITIES_AUD_PES_FIX                        (1UL<<25)
#define         DPS_CAPABILITIES_BAD_PES_FIX                        (1UL<<26)
#define         DPS_CAPABILITIES_PES_PID_INT                        (1UL<<27)

#define DPS_DVB_PRIV_DATA_EX(instance)                                         \
                              ((volatile HW_DWORD *)(DPS_BASE(instance)+0x0014))
#define DPS_NEGATIVE_MODE_REG_EX(instance)                                     \
                              ((volatile HW_DWORD *)(DPS_BASE(instance)+0x0050))
#define DPS_PTS_OFFSET_VIDEO_EX(instance)                                      \
                              ((volatile HW_DWORD *)(DPS_BASE(instance)+0x0054))
#define DPS_PTS_OFFSET_AUDIO_EX(instance)                                      \
                              ((volatile HW_DWORD *)(DPS_BASE(instance)+0x0058))
#define DPS_DVB_PRIV_DATA2_EX(instance)                                        \
                              ((volatile HW_DWORD *)(DPS_BASE(instance)+0x005C))
#define DPS_DVR_TRANSPORT_BLK_SIZE_EX(instance)                                \
                              ((volatile HW_DWORD *)(DPS_BASE(instance)+0x005C))
#define DPS_VID_SPLICE_PID_REG_EX(instance)                                    \
                              ((volatile HW_DWORD *)(DPS_BASE(instance)+0x0060))
#define DPS_AUD_SPLICE_PID_REG_EX(instance)                                    \
                              ((volatile HW_DWORD *)(DPS_BASE(instance)+0x0064))

#define DPS_PARSER_STATUS_REG_EX(instance)                                     \
                              ((volatile HW_DWORD *)(DPS_BASE(instance)+0x0068))
#define     DPS_IN_SYNC_MASK                                       0x00000001
#define     DPS_IN_SYNC_SHIFT                                             0
#define         DPS_IN_SYNC                                          (1UL<<0)
#define     DPS_PARSER_READY_MASK                                  0x00000002
#define     DPS_PARSER_READY_SHIFT                                        1
#define         DPS_PARSER_READY                                     (1UL<<1)

#define DPS_TRANSPORT_PID_REG_EX(instance)                                     \
                              ((volatile HW_DWORD *)(DPS_BASE(instance)+0x006C))
#define DPS_PES_PID_REG_EX(instance)                                           \
                              ((volatile HW_DWORD *)(DPS_BASE(instance)+0x0070))
#define DPS_REC_MON_PID_TABLE_EX(instance)                                     \
                              ((volatile HW_DWORD *)(DPS_BASE(instance)+0x0070))
#define DPS_PCR_PID_EX(instance)                                               \
                              ((volatile HW_DWORD *)(DPS_BASE(instance)+0x0074))

#define DPS_KEY_ENABLE_REG_EX(instance)                                        \
                              ((volatile HW_DWORD *)(DPS_BASE(instance)+0x0078))
#define     DPS_EVEN_KEY0_ENABLE_MASK                              0x00000001
#define     DPS_EVEN_KEY0_ENABLE_SHIFT                                    0
#define         DPS_EVEN_KEY0_ENABLE                                 (1UL<<0)
#define     DPS_EVEN_KEY1_ENABLE_MASK                              0x00000002
#define     DPS_EVEN_KEY1_ENABLE_SHIFT                                    1
#define         DPS_EVEN_KEY1_ENABLE                                 (1UL<<1)
#define     DPS_EVEN_KEY2_ENABLE_MASK                              0x00000004
#define     DPS_EVEN_KEY2_ENABLE_SHIFT                                    2
#define         DPS_EVEN_KEY2_ENABLE                                 (1UL<<2)
#define     DPS_EVEN_KEY3_ENABLE_MASK                              0x00000008
#define     DPS_EVEN_KEY3_ENABLE_SHIFT                                    3
#define         DPS_EVEN_KEY3_ENABLE                                 (1UL<<3)
#define     DPS_EVEN_KEY4_ENABLE_MASK                              0x00000010
#define     DPS_EVEN_KEY4_ENABLE_SHIFT                                    4
#define         DPS_EVEN_KEY4_ENABLE                                 (1UL<<4)
#define     DPS_EVEN_KEY5_ENABLE_MASK                              0x00000020
#define     DPS_EVEN_KEY5_ENABLE_SHIFT                                    5
#define         DPS_EVEN_KEY5_ENABLE                                 (1UL<<5)
#define     DPS_ODD_KEY0_ENABLE_MASK                               0x00000100
#define     DPS_ODD_KEY0_ENABLE_SHIFT                                     8
#define         DPS_ODD_KEY0_ENABLE                                  (1UL<<8)
#define     DPS_ODD_KEY1_ENABLE_MASK                               0x00000200
#define     DPS_ODD_KEY1_ENABLE_SHIFT                                     9
#define         DPS_ODD_KEY1_ENABLE                                  (1UL<<9)
#define     DPS_ODD_KEY2_ENABLE_MASK                               0x00000400
#define     DPS_ODD_KEY2_ENABLE_SHIFT                                    10
#define         DPS_ODD_KEY2_ENABLE                                 (1UL<<10)
#define     DPS_ODD_KEY3_ENABLE_MASK                               0x00000800
#define     DPS_ODD_KEY3_ENABLE_SHIFT                                    11
#define         DPS_ODD_KEY3_ENABLE                                 (1UL<<11)
#define     DPS_ODD_KEY4_ENABLE_MASK                               0x00001000
#define     DPS_ODD_KEY4_ENABLE_SHIFT                                    12
#define         DPS_ODD_KEY4_ENABLE                                 (1UL<<12)
#define     DPS_ODD_KEY5_ENABLE_MASK                               0x00002000
#define     DPS_ODD_KEY5_ENABLE_SHIFT                                    13
#define         DPS_ODD_KEY5_ENABLE                                 (1UL<<13)

#define DPS_VERSION_MODES_REG_EX(instance)                                     \
                              ((volatile HW_DWORD *)(DPS_BASE(instance)+0x007C))
#define DPS_PID_BASE_EX(instance,slot)                                         \
                   ((volatile u_int32 *)(DPS_BASE(instance)+0x0080+((slot)<<2)))
#define DPS_FILTER_CONTROL_BASE_EX(instance,slot)                              \
                   ((volatile u_int32 *)(DPS_BASE(instance)+0x0100+((slot)<<2)))
#define DPS_KEY_TABLE_BASE_EX(instance,slot,index)                             \
      ((volatile u_int32 *)(DPS_BASE(instance)+0x0180+((slot)<<4)+((index)<<2)))
#define DPS_TRANSPORT_WRITE_PTR_EX(instance)                                   \
                              ((volatile HW_DWORD *)(DPS_BASE(instance)+0x0200))
#define DPS_TRANSPORT_READ_PTR_EX(instance)                                    \
                              ((volatile HW_DWORD *)(DPS_BASE(instance)+0x0204))
#define DPS_TRANSPORT_START_ADDR_EX(instance)                                  \
                              ((volatile HW_DWORD *)(DPS_BASE(instance)+0x0208))
#define DPS_TRANSPORT_END_ADDR_EX(instance)                                    \
                              ((volatile HW_DWORD *)(DPS_BASE(instance)+0x020C))
#if XTV_SUPPORT==NO
#define DPS_REC_EVENT_WRITE_PTR_EX(instance)                                   \
                              ((volatile HW_DWORD *)(DPS_BASE(instance)+0x0210))
#define DPS_REC_EVENT_READ_PTR_EX(instance)                                    \
                              ((volatile HW_DWORD *)(DPS_BASE(instance)+0x0214))
#define DPS_REC_EVENT_START_ADDR_EX(instance)                                  \
                              ((volatile HW_DWORD *)(DPS_BASE(instance)+0x0218))
#define DPS_REC_EVENT_END_ADDR_EX(instance)                                    \
                              ((volatile HW_DWORD *)(DPS_BASE(instance)+0x021C))
#else
#define DPS_REC_EVENT_WRITE_PTR_EX(instance)                                   \
                              ((volatile HW_DWORD *)(DPS_BASE(instance)+0x01D0))
#define DPS_REC_EVENT_READ_PTR_EX(instance)                                    \
                              ((volatile HW_DWORD *)(DPS_BASE(instance)+0x01D4))
#define DPS_REC_EVENT_START_ADDR_EX(instance)                                  \
                              ((volatile HW_DWORD *)(DPS_BASE(instance)+0x01D8))
#define DPS_REC_EVENT_END_ADDR_EX(instance)                                    \
                              ((volatile HW_DWORD *)(DPS_BASE(instance)+0x01DC))
#endif
#define DPS_TS_BLOCK_COUNT_EX(instance)                                        \
                              ((volatile HW_DWORD *)(DPS_BASE(instance)+0x0278))
#define DPS_HOST_CMD_WORD_EX(instance)                                         \
                              ((volatile HW_DWORD *)(DPS_BASE(instance)+0x027C))

#define DPS_PID_SLOT_WRITE_PTR_EX(instance, slot)                              \
                  ((volatile HW_DWORD *)(DPS_BASE(instance)+0x0280+((slot)<<4)))
#define DPS_PID_SLOT_READ_PTR_EX(instance, slot)                               \
                  ((volatile HW_DWORD *)(DPS_BASE(instance)+0x0284+((slot)<<4)))
#define DPS_PID_SLOT_START_ADDR_EX(instance, slot)                             \
                  ((volatile HW_DWORD *)(DPS_BASE(instance)+0x0288+((slot)<<4)))
#define DPS_PID_SLOT_END_ADDR_EX(instance, slot)                               \
                  ((volatile HW_DWORD *)(DPS_BASE(instance)+0x028C+((slot)<<4)))
#define DPS_PLAY_VCORE_CMD_BUFFER_EX(instance)                                 \
                              ((volatile HW_DWORD *)(DPS_BASE(instance)+0x0280))
#define DPS_DVR_VIDEO_WRITE_PTR_EX(instance)                                   \
                              ((volatile HW_DWORD *)(DPS_BASE(instance)+0x0280))
#define DPS_DVR_VIDEO_READ_PTR_EX(instance)                                    \
                              ((volatile HW_DWORD *)(DPS_BASE(instance)+0x0284))
#define DPS_DVR_VIDEO_START_ADDR_EX(instance)                                  \
                              ((volatile HW_DWORD *)(DPS_BASE(instance)+0x0288))
#define DPS_DVR_VIDEO_END_ADDR_EX(instance)                                    \
                              ((volatile HW_DWORD *)(DPS_BASE(instance)+0x028C))
#define DPS_DVR_AUDIO_WRITE_PTR_EX(instance)                                   \
                              ((volatile HW_DWORD *)(DPS_BASE(instance)+0x0290))
#define DPS_DVR_AUDIO_READ_PTR_EX(instance)                                    \
                              ((volatile HW_DWORD *)(DPS_BASE(instance)+0x0294))
#define DPS_DVR_AUDIO_START_ADDR_EX(instance)                                  \
                              ((volatile HW_DWORD *)(DPS_BASE(instance)+0x0298))
#define DPS_DVR_AUDIO_END_ADDR_EX(instance)                                    \
                              ((volatile HW_DWORD *)(DPS_BASE(instance)+0x029C))
#define DPS_DVR_AUDIO1_WRITE_PTR_EX(instance)                                  \
                              ((volatile HW_DWORD *)(DPS_BASE(instance)+0x02A0))
#define DPS_DVR_AUDIO1_READ_PTR_EX(instance)                                   \
                              ((volatile HW_DWORD *)(DPS_BASE(instance)+0x02A4))
#define DPS_DVR_AUDIO1_START_ADDR_EX(instance)                                 \
                              ((volatile HW_DWORD *)(DPS_BASE(instance)+0x02A8))
#define DPS_DVR_AUDIO1_END_ADDR_EX(instance)                                   \
                              ((volatile HW_DWORD *)(DPS_BASE(instance)+0x02AC))
#define DPS_DVR_AUDIO2_WRITE_PTR_EX(instance)                                  \
                              ((volatile HW_DWORD *)(DPS_BASE(instance)+0x02B0))
#define DPS_DVR_AUDIO2_READ_PTR_EX(instance)                                   \
                              ((volatile HW_DWORD *)(DPS_BASE(instance)+0x02B4))
#define DPS_DVR_AUDIO2_START_ADDR_EX(instance)                                 \
                              ((volatile HW_DWORD *)(DPS_BASE(instance)+0x02B8))
#define DPS_DVR_AUDIO2_END_ADDR_EX(instance)                                   \
                              ((volatile HW_DWORD *)(DPS_BASE(instance)+0x02BC))
#define DPS_DVR_EVENT_WRITE_PTR_EX(instance)                                   \
                              ((volatile HW_DWORD *)(DPS_BASE(instance)+0x02C0))
#define DPS_DVR_EVENT_READ_PTR_EX(instance)                                    \
                              ((volatile HW_DWORD *)(DPS_BASE(instance)+0x02C4))
#define DPS_DVR_EVENT_START_ADDR_EX(instance)                                  \
                              ((volatile HW_DWORD *)(DPS_BASE(instance)+0x02C8))
#define DPS_DVR_EVENT_END_ADDR_EX(instance)                                    \
                              ((volatile HW_DWORD *)(DPS_BASE(instance)+0x02CC))

#define DPS_PASSAGE_MODE(instance)                                             \
                              ((volatile HW_DWORD *)(DPS_BASE(instance)+0x0480))
#define DPS_PASSAGE_SHADOW_VIDEO_PID(instance)                                 \
                              ((volatile HW_DWORD *)(DPS_BASE(instance)+0x0484))
#define DPS_PASSAGE_SHADOW_AUDIO_PID(instance)                                 \
                              ((volatile HW_DWORD *)(DPS_BASE(instance)+0x0488))

#define DPS_PSI_OVERFLOW_INDEX_EX(instance)                                    \
                              ((volatile HW_DWORD *)(DPS_BASE(instance)+0x08D0))
#define DPS_NOTIFY_CRC_EX(instance)                                            \
                              ((volatile HW_DWORD *)(DPS_BASE(instance)+0x08D4))
#define DPS_PTS_OFFSET_HI_EX(instance)                                         \
                              ((volatile HW_DWORD *)(DPS_BASE(instance)+0x08D8))
#define DPS_PES_FRAME_RATE_EX(instance)                                        \
                              ((volatile HW_DWORD *)(DPS_BASE(instance)+0x08DC))
#define DPS_SCRAMBLING_STATUS_TS_EX(instance)                                  \
                              ((volatile HW_DWORD *)(DPS_BASE(instance)+0x08E4))
#define DPS_SCRAMBLING_STATUS_PES_EX(instance)                                 \
                              ((volatile HW_DWORD *)(DPS_BASE(instance)+0x08E8))
#define DPS_DMA_MUX_SELECT_EX(instance)                                        \
                              ((volatile HW_DWORD *)(DPS_BASE(instance)+0x08EC))
#define DPS_AUDIO_PID_STREAM_ID_EX(instance)                                   \
                              ((volatile HW_DWORD *)(DPS_BASE(instance)+0x08EC))

#define DPS_DBG_DUMP_CPU_REG0_EX(instance)                                     \
                              ((volatile HW_DWORD *)(DPS_BASE(instance)+0x2300))
#define DPS_DBG_DUMP_CPU_REG1_EX(instance)                                     \
                              ((volatile HW_DWORD *)(DPS_BASE(instance)+0x2304))
#define DPS_DBG_DUMP_CPU_REG2_EX(instance)                                     \
                              ((volatile HW_DWORD *)(DPS_BASE(instance)+0x2308))
#define DPS_DBG_DUMP_CPU_REG3_EX(instance)                                     \
                              ((volatile HW_DWORD *)(DPS_BASE(instance)+0x230C))
#define DPS_DBG_DUMP_CPU_REG4_EX(instance)                                     \
                              ((volatile HW_DWORD *)(DPS_BASE(instance)+0x2310))
#define DPS_DBG_DUMP_CPU_REG5_EX(instance)                                     \
                              ((volatile HW_DWORD *)(DPS_BASE(instance)+0x2314))
#define DPS_DBG_DUMP_CPU_REG6_EX(instance)                                     \
                              ((volatile HW_DWORD *)(DPS_BASE(instance)+0x2318))
#define DPS_DBG_DUMP_CPU_REG7_EX(instance)                                     \
                              ((volatile HW_DWORD *)(DPS_BASE(instance)+0x231C))
#define DPS_DBG_DUMP_CPU_REG8_EX(instance)                                     \
                              ((volatile HW_DWORD *)(DPS_BASE(instance)+0x2320))
#define DPS_DBG_DUMP_CPU_REG9_EX(instance)                                     \
                              ((volatile HW_DWORD *)(DPS_BASE(instance)+0x2324))
#define DPS_DBG_DUMP_CPU_REG10_EX(instance)                                    \
                              ((volatile HW_DWORD *)(DPS_BASE(instance)+0x2328))
#define DPS_DBG_DUMP_CPU_REG11_EX(instance)                                    \
                              ((volatile HW_DWORD *)(DPS_BASE(instance)+0x232C))
#define DPS_DBG_DUMP_CPU_REG12_EX(instance)                                    \
                              ((volatile HW_DWORD *)(DPS_BASE(instance)+0x2330))
#define DPS_DBG_DUMP_CPU_REG13_EX(instance)                                    \
                              ((volatile HW_DWORD *)(DPS_BASE(instance)+0x2334))
#define DPS_DBG_DUMP_CPU_REG14_EX(instance)                                    \
                              ((volatile HW_DWORD *)(DPS_BASE(instance)+0x2338))
#define DPS_DBG_DUMP_CPU_REG15_EX(instance)                                    \
                              ((volatile HW_DWORD *)(DPS_BASE(instance)+0x233C))
#define DPS_DBG_DUMP_CURRENT_PID_EX(instance)                                  \
                              ((volatile HW_DWORD *)(DPS_BASE(instance)+0x2340))
#define DPS_DBG_DUMP_PKT_NUM_EX(instance)                                      \
                              ((volatile HW_DWORD *)(DPS_BASE(instance)+0x2348))
#define DPS_DBG_DUMP_PSI_BUFF_FULL_DROP_CNT_EX(instance)                       \
                              ((volatile HW_DWORD *)(DPS_BASE(instance)+0x234C))
#define DPS_DBG_DUMP_PSI_CRC_DROP_CNT_EX(instance)                             \
                              ((volatile HW_DWORD *)(DPS_BASE(instance)+0x236C))
#define DPS_DBG_DUMP_PSI_PKT_LOSS_DROP_CNT_EX(instance)                        \
                              ((volatile HW_DWORD *)(DPS_BASE(instance)+0x2370))
#define DPS_DBG_DUMP_PSI_FILTER_DROP_CNT_EX(instance)                          \
                              ((volatile HW_DWORD *)(DPS_BASE(instance)+0x2374))
#define DPS_DBG_DUMP_PSI_SYNTAX_DROP_CNT_EX(instance)                          \
                              ((volatile HW_DWORD *)(DPS_BASE(instance)+0x2378))

/*
 * Programmable Parser I/O Map
 */

#define DPS_PCR_HIGH_EX(instance)                                              \
                          ((volatile HW_DWORD *)(DPS_BASE(instance)+0x01000000))
#define DPS_PCR_LOW_EX(instance)                                               \
                          ((volatile HW_DWORD *)(DPS_BASE(instance)+0x01000004))
#define DPS_PCR_EXTENSION_EX(instance)                                         \
                          ((volatile HW_DWORD *)(DPS_BASE(instance)+0x01000008))
#define DPS_VIDEO_PTS_HIGH_EX(instance)                                        \
                          ((volatile HW_DWORD *)(DPS_BASE(instance)+0x01000020))
#define DPS_VIDEO_PTS_LOW_EX(instance)                                         \
                          ((volatile HW_DWORD *)(DPS_BASE(instance)+0x01000024))
#define DPS_AUDIO_PTS_HIGH_EX(instance)                                        \
                          ((volatile HW_DWORD *)(DPS_BASE(instance)+0x01000030))
#define DPS_AUDIO_PTS_LOW_EX(instance)                                         \
                          ((volatile HW_DWORD *)(DPS_BASE(instance)+0x01000034))
#define DPS_AC3_PTS_HIGH_EX(instance)                                          \
                          ((volatile HW_DWORD *)(DPS_BASE(instance)+0x01000040))
#define DPS_AC3_PTS_LOW_EX(instance)                                           \
                          ((volatile HW_DWORD *)(DPS_BASE(instance)+0x01000044))
#define DPS_AC3_BUFFER_LEVEL_EX(instance)                                      \
                          ((volatile HW_DWORD *)(DPS_BASE(instance)+0x01000064))
#define DPS_VIDEO_BUFFER_LEVEL_EX(instance)                                    \
                          ((volatile HW_DWORD *)(DPS_BASE(instance)+0x01000068))
#define DPS_AUDIO_BUFFER_LEVEL_EX(instance)                                    \
                          ((volatile HW_DWORD *)(DPS_BASE(instance)+0x0100006C))
#define DPS_AC3_LOWWATERMARK_EX(instance)                                      \
                          ((volatile HW_DWORD *)(DPS_BASE(instance)+0x01000074))
#define DPS_VIDEO_LOWWATERMARK_EX(instance)                                    \
                          ((volatile HW_DWORD *)(DPS_BASE(instance)+0x01000078))
#define DPS_AUDIO_LOWWATERMARK_EX(instance)                                    \
                          ((volatile HW_DWORD *)(DPS_BASE(instance)+0x0100007C))
#define DPS_VIDEO_WRITE_PTR_EX(instance)                                       \
                          ((volatile HW_DWORD *)(DPS_BASE(instance)+0x01000080))
#define DPS_VIDEO_READ_PTR_EX(instance)                                        \
                          ((volatile HW_DWORD *)(DPS_BASE(instance)+0x01000084))
#define DPS_VIDEO_START_ADDR_EX(instance)                                      \
                          ((volatile HW_DWORD *)(DPS_BASE(instance)+0x01000088))
#define DPS_VIDEO_END_ADDR_EX(instance)                                        \
                          ((volatile HW_DWORD *)(DPS_BASE(instance)+0x0100008C))
#define DPS_AUDIO_WRITE_PTR_EX(instance)                                       \
                          ((volatile HW_DWORD *)(DPS_BASE(instance)+0x01000090))
#define DPS_AUDIO_READ_PTR_EX(instance)                                        \
                          ((volatile HW_DWORD *)(DPS_BASE(instance)+0x01000094))
#define DPS_AUDIO_START_ADDR_EX(instance)                                      \
                          ((volatile HW_DWORD *)(DPS_BASE(instance)+0x01000098))
#define DPS_AUDIO_END_ADDR_EX(instance)                                        \
                          ((volatile HW_DWORD *)(DPS_BASE(instance)+0x0100009C))
#define DPS_USER_WRITE_PTR_EX(instance)                                        \
                          ((volatile HW_DWORD *)(DPS_BASE(instance)+0x010000A0))
#define DPS_USER_READ_PTR_EX(instance)                                         \
                          ((volatile HW_DWORD *)(DPS_BASE(instance)+0x010000A4))
#define DPS_USER_START_ADDR_EX(instance)                                       \
                          ((volatile HW_DWORD *)(DPS_BASE(instance)+0x010000A8))
#define DPS_AUD_ANC_WRITE_PTR_EX(instance)                                     \
                          ((volatile HW_DWORD *)(DPS_BASE(instance)+0x010000B0))
#define DPS_AUD_ANC_READ_PTR_EX(instance)                                      \
                          ((volatile HW_DWORD *)(DPS_BASE(instance)+0x010000B4))
#define DPS_AUD_ANC_START_ADDR_EX(instance)                                    \
                          ((volatile HW_DWORD *)(DPS_BASE(instance)+0x010000B8))
#define DPS_AUD_ANC_END_ADDR_EX(instance)                                      \
                          ((volatile HW_DWORD *)(DPS_BASE(instance)+0x010000BC))
#define DPS_MEM_CLK_CNT_REG_EX(instance)                                       \
                          ((volatile HW_DWORD *)(DPS_BASE(instance)+0x01000100))
#define DPS_MPG_CLK_CNT_REG_EX(instance)                                       \
                          ((volatile HW_DWORD *)(DPS_BASE(instance)+0x01000104))
#define DPS_MPG_27MHZ_CLK_CNT_REG_EX(instance)                                 \
                          ((volatile HW_DWORD *)(DPS_BASE(instance)+0x01000108))
#define DPS_AC3_WRITE_PTR_EX(instance)                                         \
                          ((volatile HW_DWORD *)(DPS_BASE(instance)+0x01000180))
#define DPS_AC3_READ_PTR_EX(instance)                                          \
                          ((volatile HW_DWORD *)(DPS_BASE(instance)+0x01000184))
#define DPS_AC3_START_ADDR_EX(instance)                                        \
                          ((volatile HW_DWORD *)(DPS_BASE(instance)+0x01000188))
#define DPS_AC3_END_ADDR_EX(instance)                                          \
                          ((volatile HW_DWORD *)(DPS_BASE(instance)+0x0100018C))

/*
 * Host I/O Map
 */

#define DPS_HOST_CTL_REG_EX(instance)                                          \
                          ((volatile HW_DWORD *)(DPS_BASE(instance)+0x01800000))
#define     DPS_DVR_MODE_MASK                                      0x00000003
#define     DPS_DVR_MODE_SHIFT                                            0
#define         DPS_DVR_MODE_NORMAL_OPERATION                        (0UL<<0)
#define         DPS_DVR_MODE_0                                       (1UL<<0)
#define         DPS_DVR_MODE_LIVE_PLAY_WITH_EVENTS                   (1UL<<0)
#define         DPS_DVR_MODE_1                                       (2UL<<0)
#define         DPS_DVR_MODE_RECORD_WITH_EVENTS                      (2UL<<0)
#define     DPS_SPLICE_VIDEO_MASK                                  0x00000001
#define     DPS_SPLICE_VIDEO_SHIFT                                        0
#define         DPS_SPLICE_VIDEO                                     (1UL<<0)
#define     DPS_SPLICE_AUDIO_MASK                                  0x00000002
#define     DPS_SPLICE_AUDIO_SHIFT                                        1
#define         DPS_SPLICE_AUDIO                                     (1UL<<1)
#define     DPS_PARSER_ENABLE_MASK                                 0x00000004
#define     DPS_PARSER_ENABLE_SHIFT                                       2
#define         DPS_PARSER_ENABLE                                    (1UL<<2)
#define     DPS_PID_ENABLE_MASK                                    0x00000008
#define     DPS_PID_ENABLE_SHIFT                                          3
#define         DPS_PID_ENABLE                                       (1UL<<3)
#define     DPS_TRANSPORT_BLOCKS_MASK                              0x000003F0
#define     DPS_TRANSPORT_BLOCKS_SHIFT                                    4
#define     DPS_DVR_HIGHWATERMARK_MASK                             0x000003F0
#define     DPS_DVR_HIGHWATERMARK_SHIFT                                   4
#define     DPS_TPH_DISCARD_ON_ERROR_MASK                          0x00000400
#define     DPS_TPH_DISCARD_ON_ERROR_SHIFT                               10
#define         DPS_TPH_DISCARD_ON_ERROR                            (1UL<<10)
#define     DPS_INSERT_VIDEO_SEQ_ERRORS_MASK                       0x00000800
#define     DPS_INSERT_VIDEO_SEQ_ERRORS_SHIFT                            11
#define         DPS_INSERT_VIDEO_SEQ_ERRORS                         (1UL<<11)
#define     DPS_TRANSPORT_STORAGE_MODE_MASK                        0x00001000
#define     DPS_TRANSPORT_STORAGE_MODE_SHIFT                             12
#define         DPS_TRANSPORT_STORAGE_MODE                          (1UL<<12)
#define     DPS_BSWAP_MASK                                         0x00002000
#define     DPS_BSWAP_SHIFT                                              13
#define         DPS_BSWAP                                           (1UL<<13)
#define     DPS_WARM_RESET_MASK                                    0x00004000
#define     DPS_WARM_RESET_SHIFT                                         14
#define         DPS_WARM_RESET                                      (1UL<<14)
#define     DPS_NIM_FAIL_SIGNAL_VALID_MASK                         0x00008000
#define     DPS_NIM_FAIL_SIGNAL_VALID_SHIFT                              15
#define         DPS_NIM_FAIL_SIGNAL_VALID                           (1UL<<15)

#define DPS_INFO_CHANGE_REG_EX(instance)                                       \
                          ((volatile HW_DWORD *)(DPS_BASE(instance)+0x01800004))

#define DPS_ISR_REG_EX(instance)                                               \
                          ((volatile HW_DWORD *)(DPS_BASE(instance)+0x01800008))
#define DPS_INT_ENABLE_REG_EX(instance)                                        \
                          ((volatile HW_DWORD *)(DPS_BASE(instance)+0x0180000C))
#define     DPS_BITSTREAM_IN_FULL_MASK                             0x00000001
#define     DPS_BITSTREAM_IN_FULL_SHIFT                                   0
#define         DPS_BITSTREAM_IN_FULL                                (1UL<<0)
#define     DPS_BITSTREAM_OUT_FULL_MASK                            0x00000002
#define     DPS_BITSTREAM_OUT_FULL_SHIFT                                  1
#define         DPS_BITSTREAM_OUT_FULL                               (1UL<<1)
#define     DPS_EVENT_INTERRUPT_MASK                               0x00000004
#define     DPS_EVENT_INTERRUPT_SHIFT                                     2
#define         DPS_EVENT_INTERRUPT                                  (1UL<<2)
#define     DPS_EXT_EVENT_INTERRUPT_MASK                           0x00000008
#define     DPS_EXT_EVENT_INTERRUPT_SHIFT                                 3
#define         DPS_EXT_EVENT_INTERRUPT                              (1UL<<3)
#define     DPS_PSI_FULL_OR_CRC_ERROR                              0x80000000

#define DPS_EVENT_STATUS_REG_EX(instance)                                      \
                          ((volatile HW_DWORD *)(DPS_BASE(instance)+0x01800010))
#define DPS_EVENT_ENABLE_REG_EX(instance)                                      \
                          ((volatile HW_DWORD *)(DPS_BASE(instance)+0x01800014))
#define     DPS_PCR_BLOCK_STORED_MASK                              0x00000001
#define     DPS_PCR_BLOCK_STORED_SHIFT                                    0
#define         DPS_PCR_BLOCK_STORED                                 (1UL<<0)
#define     DPS_SYNC_ERROR_MASK                                    0x00000002
#define     DPS_SYNC_ERROR_SHIFT                                          1
#define         DPS_SYNC_ERROR                                       (1UL<<1)
#define     DPS_PSI_BLOCK_STORED_MASK                              0x00000004
#define     DPS_PSI_BLOCK_STORED_SHIFT                                    2
#define         DPS_PSI_BLOCK_STORED                                 (1UL<<2)
#define     DPS_PES_BLOCK_STORED_MASK                              0x00000008
#define     DPS_PES_BLOCK_STORED_SHIFT                                    3
#define         DPS_PES_BLOCK_STORED                                 (1UL<<3)
#define     DPS_PSI_BUFFER_FULL_MASK                               0x00000010
#define     DPS_PSI_BUFFER_FULL_SHIFT                                     4
#define         DPS_PSI_BUFFER_FULL                                  (1UL<<4)
#define     DPS_PES_BUFFER_FULL_MASK                               0x00000020
#define     DPS_PES_BUFFER_FULL_SHIFT                                     5
#define         DPS_PES_BUFFER_FULL                                  (1UL<<5)
#define     DPS_VIDEO_SPLICED_MASK                                 0x00000040
#define     DPS_VIDEO_SPLICED_SHIFT                                       6
#define         DPS_VIDEO_SPLICED                                    (1UL<<6)
#define     DPS_DVR_VIDEO_INTR_MASK                                0x00000040
#define     DPS_DVR_VIDEO_INTR_SHIFT                                      6
#define         DPS_DVR_VIDEO_INTR                                   (1UL<<6)
#define     DPS_REC_EVENT_INTR_MASK                                0x00000040
#define     DPS_REC_EVENT_INTR_SHIFT                                      6
#define         DPS_REC_EVENT_INTR                                   (1UL<<6)
#define     DPS_AUDIO_SPLICED_MASK                                 0x00000080
#define     DPS_AUDIO_SPLICED_SHIFT                                       7
#define         DPS_AUDIO_SPLICED                                    (1UL<<7)
#define     DPS_DVR_AUDIO_INTR_MASK                                0x00000080
#define     DPS_DVR_AUDIO_INTR_SHIFT                                      7
#define         DPS_DVR_AUDIO_INTR                                   (1UL<<7)
#define     DPS_REC_EVENT_FULL_MASK                                0x00000080
#define     DPS_REC_EVENT_FULL_SHIFT                                      7
#define         DPS_REC_EVENT_FULL                                   (1UL<<7)
#define     DPS_TRANSPORT_BLOCK_STORED_MASK                        0x00000100
#define     DPS_TRANSPORT_BLOCK_STORED_SHIFT                              8
#define         DPS_TRANSPORT_BLOCK_STORED                           (1UL<<8)
#define     DPS_DVR_AUDIO1_INTR_MASK                               0x00000100
#define     DPS_DVR_AUDIO1_INTR_SHIFT                                     8
#define         DPS_DVR_AUDIO1_INTR                                  (1UL<<8)
#define     DPS_TRANSPORT_BUFFER_FULL_MASK                         0x00000200
#define     DPS_TRANSPORT_BUFFER_FULL_SHIFT                               9
#define         DPS_TRANSPORT_BUFFER_FULL                            (1UL<<9)
#define     DPS_DVR_AUDIO2_INTR_MASK                               0x00000200
#define     DPS_DVR_AUDIO2_INTR_SHIFT                                     9
#define         DPS_DVR_AUDIO2_INTR                                  (1UL<<9)
#define     DPS_DVR_VIDEO_OVERFLOW_MASK                            0x00000400
#define     DPS_DVR_VIDEO_OVERFLOW_SHIFT                                 10
#define         DPS_DVR_VIDEO_OVERFLOW                              (1UL<<10)
#define     DPS_DVR_AUDIO_OVERFLOW_MASK                            0x00000800
#define     DPS_DVR_AUDIO_OVERFLOW_SHIFT                                 11
#define         DPS_DVR_AUDIO_OVERFLOW                              (1UL<<11)
#define     DPS_DVR_AUDIO1_OVERFLOW_MASK                           0x00001000
#define     DPS_DVR_AUDIO1_OVERFLOW_SHIFT                                12
#define         DPS_DVR_AUDIO1_OVERFLOW                             (1UL<<12)
#define     DPS_DVR_AUDIO2_OVERFLOW_MASK                           0x00002000
#define     DPS_DVR_AUDIO2_OVERFLOW_SHIFT                                13
#define         DPS_DVR_AUDIO2_OVERFLOW                             (1UL<<13)
#define     DPS_BAD_PES_HEADER_MASK                                0x00002000
#define     DPS_BAD_PES_HEADER_SHIFT                                     13
#define         DPS_BAD_PES_HEADER                                  (1UL<<13)
#define     DPS_VIDEO_PKT_RECEIVED_MASK                            0x00004000
#define     DPS_VIDEO_PKT_RECEIVED_SHIFT                                 14
#define         DPS_VIDEO_PKT_RECEIVED                              (1UL<<14)
#define     DPS_DVR_EVENT_INTR_MASK                                0x00004000
#define     DPS_DVR_EVENT_INTR_SHIFT                                     14
#define         DPS_DVR_EVENT_INTR                                  (1UL<<14)
#define     DPS_AUDIO_PKT_RECEIVED_MASK                            0x00008000
#define     DPS_AUDIO_PKT_RECEIVED_SHIFT                                 15
#define         DPS_AUDIO_PKT_RECEIVED                              (1UL<<15)
#define     DPS_DVR_EVENT_FULL_MASK                                0x00008000
#define     DPS_DVR_EVENT_FULL_SHIFT                                     15
#define         DPS_DVR_EVENT_FULL                                  (1UL<<15)

#define DPS_PARSER_CTL_REG_EX(instance)                                        \
                          ((volatile HW_DWORD *)(DPS_BASE(instance)+0x01800018))
#define     DPS_RUN_MASK                                           0x00000001
#define     DPS_RUN_SHIFT                                                 0
#define         DPS_RESET                                            (0UL<<0)
#define         DPS_RUN                                              (1UL<<0)
#define     DPS_FLUSH_DELAY_EVENTS_MASK                            0x00000002
#define     DPS_FLUSH_DELAY_EVENTS_SHIFT                                  1
#define         DPS_FLUSH_DELAY_EVENTS                               (1UL<<1)
#define     DPS_TSI_ERROR_POLARITY_MASK                            0x00000002
#define     DPS_TSI_ERROR_POLARITY_SHIFT                                  1
#define         DPS_TSI_ERROR_POLARITY                               (1UL<<1)
#define     DPS_TP_MODE_MASK_MASK                                  0x00000004
#define     DPS_TP_MODE_MASK_SHIFT                                        2
#define         DPS_TP_MODE_PES                                      (0UL<<2)
#define         DPS_TP_MODE_DVB                                      (1UL<<2)
#define     DPS_MPEG_PTR_MASTER_MASK                               0x00000008
#define     DPS_MPEG_PTR_MASTER_SHIFT                                     3
#define         DPS_MPEG_PTR_MASTER_DISABLE                          (0UL<<3)
#define         DPS_MPEG_PTR_MASTER_ENABLE                           (1UL<<3)
#define     DPS_INPUT_BYTESWAP_MASK                                0x00000010
#define     DPS_INPUT_BYTESWAP_SHIFT                                      4
#define         DPS_INPUT_BYTESWAP_BIG_ENDIAN                        (0UL<<4)
#define         DPS_INPUT_BYTESWAP_LITTLE_ENDIAN                     (1UL<<4)
#define     DPS_OUTPUT_BYTESWAP_MASK                               0x00000020
#define     DPS_OUTPUT_BYTESWAP_SHIFT                                     5
#define         DPS_OUTPUT_BYTESWAP_BIG_ENDIAN                       (0UL<<5)
#define         DPS_OUTPUT_BYTESWAP_LITTLE_ENDIAN                    (1UL<<5)
#define     DPS_HOST_IF_TIMEOUT_MASK                               0x000000C0
#define     DPS_HOST_IF_TIMEOUT_SHIFT                                     6
#define         DPS_HOST_IF_TIMEOUT_256_CLKS                         (0UL<<6)
#define         DPS_HOST_IF_TIMEOUT_312_CLKS                         (1UL<<6)
#define         DPS_HOST_IF_TIMEOUT_1024_CLKS                        (2UL<<6)
#define         DPS_HOST_IF_TIMEOUT_DISABLE                          (3UL<<6)
#define     DPS_PCR_ONLY_MASTER_MASK                               0x00000100
#define     DPS_PCR_ONLY_MASTER_SHIFT                                     8
#define         DPS_PCR_ONLY_MASTER_OFF                              (0UL<<8)
#define         DPS_PCR_ONLY_MASTER_ON                               (1UL<<8)
#define     DPS_TS_IN_DMA_THRESHOLD_MASK                           0x00000C00
#define     DPS_TS_IN_DMA_THRESHOLD_SHIFT                                10
#define         DPS_TS_IN_DMA_THRESHOLD_16                          (0UL<<10)
#define         DPS_TS_IN_DMA_THRESHOLD_32                          (1UL<<10)
#define         DPS_TS_IN_DMA_THRESHOLD_48                          (2UL<<10)
#define         DPS_TS_IN_DMA_THRESHOLD_64                          (3UL<<10)
#define     DPS_TS_OUT_DMA_THRESHOLD_MASK                          0x00003000
#define     DPS_TS_OUT_DMA_THRESHOLD_SHIFT                               12
#define         DPS_TS_OUT_DMA_THRESHOLD_16                         (0UL<<12)
#define         DPS_TS_OUT_DMA_THRESHOLD_32                         (1UL<<12)
#define         DPS_TS_OUT_DMA_THRESHOLD_48                         (2UL<<12)
#define         DPS_TS_OUT_DMA_THRESHOLD_64                         (3UL<<12)
#define     DPS_INPUT_SRC_SEL_MASK                                 0x00004000
#define     DPS_INPUT_SRC_SEL_SHIFT                                      14
#define         DPS_INPUT_SRC_SEL_NIM                               (0UL<<14)
#define         DPS_INPUT_SRC_SEL_DMA                               (1UL<<14)
#define     DPS_OUTPUT_SRC_SEL_MASK                                0x00008000
#define     DPS_OUTPUT_SRC_SEL_SHIFT                                     15
#define         DPS_OUTPUT_SRC_SEL_NIM                              (0UL<<15)
#define         DPS_OUTPUT_SRC_SEL_DMA                              (1UL<<15)
#define     DPS_TS_IN_DMA_ENABLE_MASK                              0x00010000
#define     DPS_TS_IN_DMA_ENABLE_SHIFT                                   16
#define         DPS_TS_IN_DMA_DISABLE                               (0UL<<16)
#define         DPS_TS_IN_DMA_ENABLE                                (1UL<<16)
#define     DPS_TS_OUT_DMA_ENABLE_MASK                             0x00020000
#define     DPS_TS_OUT_DMA_ENABLE_SHIFT                                  17
#define         DPS_TS_OUT_DMA_DISABLE                              (0UL<<17)
#define         DPS_TS_OUT_DMA_ENABLE                               (1UL<<17)
#define     DPS_TS_STREAM_OUTPUT_PACING_ENABLE_MASK                0x00040000
#define     DPS_TS_STREAM_OUTPUT_PACING_ENABLE_SHIFT                     18
#define         DPS_TS_STREAM_OUTPUT_PACING_DISABLE                 (0UL<<18)
#define         DPS_TS_STREAM_OUTPUT_PACING_ENABLE                  (1UL<<18)

#define DPS_MEM_PAGE_EX(instance)                                              \
                          ((volatile HW_DWORD *)(DPS_BASE(instance)+0x0180001C))
#define DPS_TS_IN_BUFFER_FREE_EX(instance)                                     \
                          ((volatile HW_DWORD *)(DPS_BASE(instance)+0x01800020))
#define DPS_TS_OUT_BUFFER_FREE_EX(instance)                                    \
                          ((volatile HW_DWORD *)(DPS_BASE(instance)+0x01800024))

#define DPS_EXT_EVENT_STATUS_REG_EX(instance)                                  \
                          ((volatile HW_DWORD *)(DPS_BASE(instance)+0x01800028))
#define     DPS_EXT_EVENT_TS_BLOCK_STORED_MASK                     0x00000001
#define     DPS_EXT_EVENT_TS_BLOCK_STORED_SHIFT                           0
#define         DPS_EXT_EVENT_TS_BLOCK_STORED                        (1UL<<1)
#define     DPS_EXT_EVENT_TS_OVERFLOW_MASK                         0x00000002
#define     DPS_EXT_EVENT_TS_OVERFLOW_SHIFT                               1
#define         DPS_EXT_EVENT_TS_OVERFLOW                            (1UL<<1)
#define     DPS_EXT_EVENT_MODE_TRANSITION_COMPLETE_MASK            0x00000004
#define     DPS_EXT_EVENT_MODE_TRANSITION_COMPLETE_SHIFT                  2
#define         DPS_EXT_EVENT_MODE_TRANSITION_COMPLETE               (1UL<<2)

#define DPS_FLUSH_STATUS_REG_EX(instance)                                      \
                          ((volatile HW_DWORD *)(DPS_BASE(instance)+0x0180002C))
#define     DPS_FLUSH_IN_PROGRESS_MASK                             0x00000001
#define     DPS_FLUSH_IN_PROGRESS_SHIFT                                   0
#define         DPS_FLUSH_IN_PROGRESS                                (1UL<<1)

#define DPS_CMD_CTRL_EX(instance)                                              \
                          ((volatile HW_DWORD *)(DPS_BASE(instance)+0x01800030))
#define     DPS_HOST_CMD_VCORE_CMD_CNT_MASK                        0x000000ff
#define     DPS_HOST_CMD_VCORE_CMD_CNT_SHIFT                              0
#define         DPS_HOST_CMD_DISABLE_PARSER                          (0UL<<0)
#define         DPS_HOST_CMD_ENABLE_PARSER                           (1UL<<0)
#define         DPS_HOST_CMD_TS_MODE_OFF                             (2UL<<0)
#define         DPS_HOST_CMD_TS_MODE_ON                              (3UL<<0)
#define         DPS_HOST_CMD_TRICK_MODE_OFF                          (4UL<<0)
#define         DPS_HOST_CMD_TRICK_MODE_ON                           (5UL<<0)
#define         DPS_HOST_CMD_RESET_PKT_CNT                           (6UL<<0)
#define     DPS_HOST_CMD_VCORE_CMD_MASK                            0x00000100
#define     DPS_HOST_CMD_VCORE_CMD_SHIFT                                  8
#define         DPS_HOST_CMD_VCORE_CMD                               (1UL<<8)

#define DPS_PATTERN_BASE_EX(instance,slot,index)                               \
  ((volatile u_int32 *)(DPS_BASE(instance)+0x01800800+((slot)<<4)+((index)<<2)))
#define DPS_FILTER_MASK_BASE_EX(instance,slot,index)                           \
  ((volatile u_int32 *)(DPS_BASE(instance)+0x01800A00+((slot)<<4)+((index)<<2)))
#define DPS_FILTER_MODE_BASE_EX(instance,slot,index)                           \
  ((volatile u_int32 *)(DPS_BASE(instance)+0x01800C00+((slot)<<4)+((index)<<2)))
#define DPS_DMA_WINDOW_IN_EX(instance)                                         \
                          ((volatile HW_DWORD *)(DPS_BASE(instance)+0x01C00000))
#define DPS_DMA_WINDOW_OUT_EX(instance)                                        \
                          ((volatile HW_DWORD *)(DPS_BASE(instance)+0x01D00000))

/*
 * DVR Event Header definitions
 */

#define     DPS_FLUSH_IN_PROGRESS_MASK                             0x00000001
#define     DPS_DVR_EVENT_HDR_PID_MASK	                           0x00001FFF
#define     DPS_DVR_EVENT_HDR_START_INDICATOR                      0x00004000
#define     DPS_DVR_EVENT_HDR_PTS_RECEIVED                         0x00010000
#define     DPS_DVR_EVENT_HDR_PCR_RECEIVED                         0x00020000
#define     DPS_DVR_EVENT_HDR_DISCONTINUITY_INDICATOR              0x00040000
#define     DPS_DVR_EVENT_HDR_SEQUENCE_HEADER                      0x00080000
#define     DPS_DVR_EVENT_HDR_PICTURE_HEADER                       0x00100000
#define     DPS_DVR_EVENT_HDR_GOP_HEADER                           0x00200000
#define     DPS_DVR_EVENT_HDR_I_FRAME_START                        0x04000000
#define     DPS_DVR_EVENT_HDR_PCR_SOURCE                           0x08000000
#define     DPS_DVR_EVENT_HDR_PTS_MSB                              0x10000000
#define     DPS_DVR_EVENT_HDR_PCR_MSB                              0x20000000
#define     DPS_DVR_EVENT_HDR_PTS_TYPE_MASK                        0xC0000000
#define     DPS_DVR_EVENT_HDR_PTS_VIDEO                            0x00000000
#define     DPS_DVR_EVENT_HDR_PTS_AUDIO                            0x40000000
#define     DPS_DVR_EVENT_HDR_PTS_AUDIO1                           0x80000000
#define     DPS_DVR_EVENT_HDR_PTS_AUDIO2                           0xC0000000

#define     DPS_DVR_EVENT_PTS_TYPE_VIDEO                           0x00000000
#define     DPS_DVR_EVENT_PTS_TYPE_AUDIO                           0x00000001
#define     DPS_DVR_EVENT_PTS_TYPE_AUDIO1                          0x00000002
#define     DPS_DVR_EVENT_PTS_TYPE_AUDIO2                          0x00000003

/*
 * DVR GOP Time Code definitions
 */

#define     DPS_DVR_GOP_TIME_CODE_PICTURES_MASK                    0x00001F80
#define     DPS_DVR_GOP_TIME_CODE_PICTURES_SHIFT                          7
#define     DPS_DVR_GOP_TIME_CODE_SECONDS_MASK                     0x0007E000
#define     DPS_DVR_GOP_TIME_CODE_SECONDS_SHIFT                          13
#define     DPS_DVR_GOP_TIME_CODE_MARKER_BIT                       0x00080000
#define     DPS_DVR_GOP_TIME_CODE_MINUTES_MASK                     0x03F00000
#define     DPS_DVR_GOP_TIME_CODE_MINUTES_SHIFT                          20
#define     DPS_DVR_GOP_TIME_CODE_HOURS_MASK                       0x7C000000
#define     DPS_DVR_GOP_TIME_CODE_HOURS_SHIFT                            26
#define     DPS_DVR_GOP_TIME_CODE_DROP_FRAME_FLAG                  0x80000000

/*
 * Filter Match Defitions
 */

typedef struct _DPS_FILTER_MATCH_TABLE
{
        HW_DWORD Match[3];
        HW_DWORD reserved;
} DPS_FILTER_MATCH_TABLE;
typedef DPS_FILTER_MATCH_TABLE volatile *LPDPS_FILTER_MATCH_TABLE;

/*
 * Filter Mask Defitions
 */

typedef struct _DPS_FILTER_MASK_TABLE
{
        HW_DWORD Mask[3];
        HW_DWORD reserved;
} DPS_FILTER_MASK_TABLE;
typedef DPS_FILTER_MASK_TABLE volatile *LPDPS_FILTER_MASK_TABLE;

/*
 * Filter Mode Defitions
 */

typedef struct _DPS_FILTER_MODE_TABLE
{
        HW_DWORD Mode[3];
        HW_DWORD reserved;
} DPS_FILTER_MODE_TABLE;
typedef DPS_FILTER_MODE_TABLE volatile *LPDPS_FILTER_MODE_TABLE;

/*
 * Key Definitions
 */

typedef struct _DPS_KEY_ENTRY
{
        HW_DWORD KeyHighEven;
        HW_DWORD KeyLowEven;
        HW_DWORD KeyHighOdd;
        HW_DWORD KeyLowOdd;
} DPS_KEY_ENTRY;
typedef DPS_KEY_ENTRY volatile *LPDPS_KEY_ENTRY;

#endif /* INCL_DPS */


/*******************/
/* Real Time Clock */
/*******************/
#ifdef INCL_RTC

/*
******************************************************************************
 *
 *     RTC_DATA_REG                                          (RTC_BASE + 0x00)
 *     RTC_MATCH_REG                                         (RTC_BASE + 0x04)
 *     RTC_EOI_REG                                           (RTC_BASE + 0x08)
 *     RTC_TEST_REG                                          (RTC_BASE + 0x0C)
 *
 *****************************************************************************
 */

#define RTC_DATA_REG                                         (RTC_BASE + 0x00)
#define RTC_MATCH_REG                                        (RTC_BASE + 0x04)
#define RTC_EOI_REG                                          (RTC_BASE + 0x08)
#define RTC_TEST_REG                                         (RTC_BASE + 0x0C)

#ifdef BITFIELDS
typedef struct _RTC_DATA
{
  HW_DWORD Value;
} RTC_DATA;
typedef RTC_DATA volatile *LPRTC_DATA;

typedef struct _RTC_MATCH
{
  HW_DWORD Value;
} RTC_MATCH;
typedef RTC_MATCH volatile *LPRTC_MATCH;

typedef struct _RTC_EOI
{
  HW_DWORD Reset;
} RTC_EOI;
typedef RTC_EOI volatile *LPRTC_EOI;

typedef struct _RTC_TEST
{
  BIT_FLD Test:1,
        TstClkEn:1,
        IntStatus:1,
        Reserved1:29;
} RTC_TEST;
typedef RTC_TEST volatile *LPRTC_TEST;
#endif /* BITFIELDS */

#endif

/**************************/
/* Modem Analog Front End */
/**************************/
#ifdef INCL_AFE

/* Last updated 4/22/99 */

/*
******************************************************************************
 *
 *     AFE_CONTROL_REG                                       (AFE_BASE + 0x00)
 *     AFE_STATUS_REG                                        (AFE_BASE + 0x04)
 *     AFE_TX_DATA_PORT_REG                                  (AFE_BASE + 0x08)
 *     AFE_RX_DATA_PORT_REG                                  (AFE_BASE + 0x0C)
 *     AFE_IA_CTRLWR_REG                                     (AFE_BASE + 0x10)
 *     AFE_IA_CTRLRD_REG                                     (AFE_BASE + 0x14)
 *     AFE_CLK_DIV_REG                                       (AFE_BASE + 0x18)
 *
 *****************************************************************************
 */

#define AFE_CONTROL_REG                                      (AFE_BASE + 0x00)
#define     AFE_CONTROL_TXUNDERRUNINT_MASK                         0x00000001
#define     AFE_CONTROL_TXUNDERRUNINT_SHIFT                               0
#define     AFE_CONTROL_TXDMA_MASK                                 0x00000002
#define     AFE_CONTROL_TXDMA_SHIFT                                       1
#define     AFE_CONTROL_TXPIO_MASK                                 0x00000004
#define     AFE_CONTROL_TXPIO_SHIFT                                       2
#define     AFE_CONTROL_RESERVED1_MASK                             0x00000008
#define     AFE_CONTROL_RESERVED1_SHIFT                                   3
#define     AFE_CONTROL_TXPACKETSIZE_MASK                          0x000000F0
#define     AFE_CONTROL_TXPACKETSIZE_SHIFT                                4
#define     AFE_CONTROL_TXFIFOSIZE_MASK                            0x00000F00
#define     AFE_CONTROL_TXFIFOSIZE_SHIFT                                  8
#define     AFE_CONTROL_RESERVED2_MASK                             0x00001000
#define     AFE_CONTROL_RESERVED2_SHIFT                                  12
#define     AFE_CONTROL_FIFOINTERACTBLK_MASK                       0x00002000
#define     AFE_CONTROL_FIFOINTERACTBLK_SHIFT                            13
#define     AFE_CONTROL_INTENABLE_MASK                             0x00004000
#define     AFE_CONTROL_INTENABLE_SHIFT                                  14
#define     AFE_CONTROL_CTRLEN_MASK                                0x00008000
#define     AFE_CONTROL_CTRLEN_SHIFT                                     15
#define     AFE_CONTROL_RXOVERRUNTEST_MASK                         0x00010000
#define     AFE_CONTROL_RXOVERRUNTEST_SHIFT                              16
#define     AFE_CONTROL_RXDMA_MASK                                 0x00020000
#define     AFE_CONTROL_RXDMA_SHIFT                                      17
#define     AFE_CONTROL_RXPIO_MASK                                 0x00040000
#define     AFE_CONTROL_RXPIO_SHIFT                                      18
#define     AFE_CONTROL_RESERVED4_MASK                             0x00080000
#define     AFE_CONTROL_RESERVED4_SHIFT                                  19
#define     AFE_CONTROL_RXPACKETSIZE_MASK                          0x00F00000
#define     AFE_CONTROL_RXPACKETSIZE_SHIFT                               20
#define     AFE_CONTROL_RXFIFOSIZE_MASK                            0x0F000000
#define     AFE_CONTROL_RXFIFOSIZE_SHIFT                                 24
#define     AFE_CONTROL_RESERVED5_MASK                             0x30000000
#define     AFE_CONTROL_RESERVED5_SHIFT                                  28
#define     AFE_CONTROL_LOOPBACKTEST_MASK                          0x40000000
#define     AFE_CONTROL_LOOPBACKTEST_SHIFT                               30
#define     AFE_CONTROL_RXDOUBLERATE_MASK                          0x80000000
#define     AFE_CONTROL_RXDOUBLERATE_SHIFT                               31
#define AFE_STATUS_REG                                       (AFE_BASE + 0x04)
#define     AFE_STATUS_TXUNDERRUN_MASK                             0x00000001
#define     AFE_STATUS_TXUNDERRUN_SHIFT                                   0
#define     AFE_STATUS_TXDMA_MASK                                  0x00000002
#define     AFE_STATUS_TXDMA_SHIFT                                        1
#define     AFE_STATUS_TXPIO_MASK                                  0x00000004
#define     AFE_STATUS_TXPIO_SHIFT                                        2
#define     AFE_STATUS_RESERVED1_MASK                              0x000000F8
#define     AFE_STATUS_RESERVED1_SHIFT                                    3
#define     AFE_STATUS_TXFIFOLEVEL_MASK                            0x00000F00
#define     AFE_STATUS_TXFIFOLEVEL_SHIFT                                  8
#define     AFE_STATUS_RESERVED2_MASK                              0x00003000
#define     AFE_STATUS_RESERVED2_SHIFT                                   12
#define     AFE_STATUS_INT_MASK                                    0x00004000
#define     AFE_STATUS_INT_SHIFT                                         14
#define     AFE_STATUS_RESERVED3_MASK                              0x00008000
#define     AFE_STATUS_RESERVED3_SHIFT                                   15
#define     AFE_STATUS_RXOVERRUN_MASK                              0x00010000
#define     AFE_STATUS_RXOVERRUN_SHIFT                                   16
#define     AFE_STATUS_RXDMA_MASK                                  0x00020000
#define     AFE_STATUS_RXDMA_SHIFT                                       17
#define     AFE_STATUS_RXPIO_MASK                                  0x00040000
#define     AFE_STATUS_RXPIO_SHIFT                                       18
#define     AFE_STATUS_RESERVED4_MASK                              0x00F80000
#define     AFE_STATUS_RESERVED4_SHIFT                                   19
#define     AFE_STATUS_RXFIFOLEVEL_MASK                            0x0F000000
#define     AFE_STATUS_RXFIFOLEVEL_SHIFT                                 24
#define     AFE_STATUS_RESERVED5_MASK                              0xF0000000
#define     AFE_STATUS_RESERVED5_SHIFT                                   28
#define AFE_TX_DATA_PORT_REG                                 (AFE_BASE + 0x08)
#define AFE_RX_DATA_PORT_REG                                 (AFE_BASE + 0x0C)
#define AFE_IA_CTRLWR_REG                                    (AFE_BASE + 0x10)
#define     AFE_IA_CTRLWR_VALUE_MASK                               0x0000FFFF
#define     AFE_IA_CTRLWR_VALUE_SHIFT                                     0
#define     AFE_IA_CTRLWR_RESERVED1_MASK                           0xFFFF0000
#define     AFE_IA_CTRLWR_RESERVED1_SHIFT                                16
#define AFE_IA_CTRLRD_REG                                    (AFE_BASE + 0x14)
#define     AFE_IA_CTRLRD_VALUE_MASK                               0x0000FFFF
#define     AFE_IA_CTRLRD_VALUE_SHIFT                                     0
#define     AFE_IA_CTRLRD_RESERVED1_MASK                           0xFFFF0000
#define     AFE_IA_CTRLRD_RESERVED1_SHIFT                                16
#define AFE_CLK_DIV_REG                                      (AFE_BASE + 0x18)
#define     AFE_CLK_DIV_LOW_MASK                                   0x0000FFFF
#define     AFE_CLK_DIV_LOW_SHIFT                                         0
#define     AFE_CLK_DIV_HIGH_MASK                                  0xFFFF0000
#define     AFE_CLK_DIV_HIGH_SHIFT                                       16

typedef HW_DWORD   AFE_TX_DATA_PORT;
typedef LPREG    LPAFE_TX_DATA_PORT;
typedef HW_DWORD   AFE_RX_DATA_PORT;
typedef LPREG    LPAFE_RX_DATA_PORT;

#define FIFO_INTERACT_BLK_AFE  0
#define FIFO_INTERACT_BLK_DAA  1

/* Bit positions for accessing register as a single HW_DWORD */

#define AFE_RX2X_EN         0x80000000  /* Enable dbl rate RX data from codec */
#define AFE_RX_FIFO_SZ_MSK  0x0F000000  /* RX fifo size 4/8 (32bit) words */
#define AFE_RX_PKT_SZ_MSK   0x00F00000  /* RX DMA pkt size 1/2/4/8 words */
#define AFE_RX_PIO_EN       0x00040000  /* Enable RX data via PIO (debug only) */
#define AFE_RX_DMA_EN       0x00020000  /* Enable RX data via DMA */
#define AFE_RX_ORUN_INT_EN  0x00010000  /* Enable RX fifo overrun interrupt */
#define AFE_TX_FIFO_SZ_MSK  0x00000F00  /* TX fifo size 4/8 (32bit) words */
#define AFE_TX_PKT_SZ_MSK   0x000000F0  /* TX DMA pkt size 1/2/4/8 words */
#define AFE_TX_PIO_EN       0x00000004  /* Enable TX data via PIO (debug only) */
#define AFE_TX_DMA_EN       0x00000002  /* Enable TX data via DMA */
#define AFE_TX_URUN_INT_EN  0x00000001  /* Enable TX fifo underrun interrupt */

#define AFE_RX_FIFO_SZ_4    (1 << 26)   /* RX fifo size 4 (32 bit) words */
#define AFE_RX_FIFO_SZ_8    (1 << 27)   /* RX fifo size 8 (32 bit) words default */
#define AFE_RX_PKT_SZ_1     (1 << 20)   /* RX DMA pkt size 1 word */
#define AFE_RX_PKT_SZ_2     (1 << 21)   /* RX DMA pkt size 2 words */
#define AFE_RX_PKT_SZ_4     (1 << 22)   /* RX DMA pkt size 4 words */
#define AFE_RX_PKT_SZ_8     (1 << 23)   /* RX DMA pkt size 8 words */

#define AFE_TX_FIFO_SZ_4    (1 << 10)   /* TX fifo size 4 (32 bit) words */
#define AFE_TX_FIFO_SZ_8    (1 << 11)   /* TX fifo size 8 (32 bit) words default */
#define AFE_TX_PKT_SZ_1     (1 << 4)    /* TX DMA pkt size 1 word */
#define AFE_TX_PKT_SZ_2     (1 << 5)    /* TX DMA pkt size 2 words */
#define AFE_TX_PKT_SZ_4     (1 << 6)    /* TX DMA pkt size 4 words */
#define AFE_TX_PKT_SZ_8     (1 << 7)    /* TX DMA pkt size 8 words */

/* Bit positions for accessing register as a single HW_DWORD */

#define AFE_STAT_RX_FIFO_LEVEL 0x0F000000
#define AFE_STAT_RX_PIO        0x00040000
#define AFE_STAT_RX_DMA        0x00020000
#define AFE_STAT_RX_ORUN       0x00010000
#define AFE_STAT_TX_FIFO_LEVEL 0x00000F00
#define AFE_STAT_TX_PIO        0x00000004
#define AFE_STAT_TX_DMA        0x00000002
#define AFE_STAT_TX_URUN       0x00000001

#ifdef BITFIELDS
typedef struct _AFE_CONTROL
{
  BIT_FLD TxUnderrunInt:1,
          TxDMA:1,
          TxPIO:1,
          Reserved1:1,
          TxPacketSize:4,
          TxFifoSize:4,
          Reserved2:1,
          FifoInteractBlk:1,
          IntEnable:1,
          CtrlEn:1,
          RxOverrunTest:1,
          RxDMA:1,
          RxPIO:1,
          Reserved4:1,
          RxPacketSize:4,
          RxFifoSize:4,
          Reserved5:2,
          LoopbackTest:1,
          RxDoubleRate:1;
} AFE_CONTROL;
typedef AFE_CONTROL volatile *LPAFE_CONTROL;

typedef struct _AFE_STATUS
{
  BIT_FLD TxUnderrun:1,
          TxDMA:1,
          TxPIO:1,
          Reserved1:5,
          TxFifoLevel:4,
          Reserved2:2,
          Int:1,
          Reserved3:1,
          RxOverrun:1,
          RxDMA:1,
          RxPIO:1,
          Reserved4:5,
          RxFifoLevel:4,
          Reserved5:4;
} AFE_STATUS;
typedef AFE_STATUS volatile *LPAFE_STATUS;

typedef struct _AFE_IA_CTRLWR
{
  BIT_FLD Value:16,
          Reserved1:16;
} AFE_IA_CTRLWR;
typedef AFE_IA_CTRLWR volatile *LPAFE_IA_CTRLWR;

typedef struct _AFE_IA_CTRLRD
{
  BIT_FLD Value:16,
          Reserved1:16;
} AFE_IA_CTRLRD;
typedef AFE_IA_CTRLRD volatile *LPAFE_IA_CTRLRD;

typedef struct _AFE_CLK_DIV
{
  BIT_FLD Low:16,
          High:16;
} AFE_CLK_DIV;
typedef AFE_CLK_DIV volatile *LPAFE_CLK_DIV;
#endif /* BITFIELDS */

#endif

/***************************/
/* SDRAM Control Registers */
/***************************/
#ifdef INCL_SDR

/* Last updated 1/11/00 */

/*
******************************************************************************
 *     SDR_MEM_CFG_REG                                       (SDR_BASE + 0x04)
 *     SDR_REF_CNT_REG                                       (SDR_BASE + 0x08)
 *     SDR_PROC_CRIT_REG                                     (SDR_BASE + 0x10)
 *     SDR_LOW_POWER_REG                                     (SDR_BASE + 0x20)
 *
 *****************************************************************************
 */

#define SDR_MEM_CFG_REG                                      (SDR_BASE + 0x04)
#define     SDR_MEM_CFG_RANK1_EMPTY_MASK                         0x00001000
#define     SDR_MEM_CFG_RANK1_EMPTY_SHIFT                              12
#define     SDR_MEM_CFG_RANK0_SIZE_MASK                          0x00006000
#define     SDR_MEM_CFG_RANK0_SIZE_SHIFT                               13
#define     SDR_MEM_CFG_RANK1_SIZE_MASK                          0x00018000
#define     SDR_MEM_CFG_RANK1_SIZE_SHIFT                               15
#define     SDR_MEM_CFG_RANK0_SIZE_MSB_MASK                      0x04000000
#define     SDR_MEM_CFG_RANK0_SIZE_MSB_SHIFT                           26
#define     SDR_MEM_CFG_RANK1_SIZE_MSB_MASK                      0x08000000
#define     SDR_MEM_CFG_RANK1_SIZE_MSB_SHIFT                           27
#define SDR_REF_CNT_REG                                      (SDR_BASE + 0x08)
#define SDR_PROC_CRIT_REG                                    (SDR_BASE + 0x10)
#define SDR_LOW_POWER_REG                                    (SDR_BASE + 0x20)

#define SDR_CFG_SYNCMEM_CFG             0x000007ff
#define SDR_CFG_RANK0_SIZE              0x04006000
#define SDR_CFG_RANK1_SIZE              0x08018000
#define SDR_CFG_ADR_MODE_LRG            0x00020000
#define SDR_CFG_WBSTP_LATENCY           0x00040000
#define SDR_CFG_RBSTP_LATENCY           0x00080000
#define SDR_CFG_RCD_CNT                 0x00100000
#define SDR_CFG_RFSH_CYCLE_CNT          0x00200000
#define SDR_CFG_RAS_PRE_CNT             0x00400000
#define SDR_CFG_BYTE_SWAP               0x00800000
#define SDR_CFG_RESET_CFG               0x01000000
#define SDR_CFG_TRAS_EXTEND             0x02000000
#define SDR_CFG_CLOSE_WAMBA             0x10000000
#define SDR_CFG_CLOSE_LT_4              0x20000000
#define SDR_CFG_CLOSE_GT_4              0x40000000
#define SDR_CFG_TRAS_LEN                0x80000000

#define SDR_CFG_RANK0_EMPTY             0x00000800
#define SDR_CFG_RANK1_EMPTY             0x00001000

#define SDR_RANK0_8MB_64                0x00000000
#define SDR_RANK0_4MB_16                0x00002000
#define SDR_RANK0_8MB_16                0x00004000
#define SDR_RANK0_16MB_64               0x00006000
#define SDR_RANK0_32MB_128              0x04000000
#define SDR_RANK0_64MB_256              0x04002000
#define SDR_RANK0_EMPTY                 0x00000800
#define SDR_RANK1_8MB_64                0x00000000
#define SDR_RANK1_4MB_16                0x00008000
#define SDR_RANK1_8MB_16                0x00010000
#define SDR_RANK1_16MB_64               0x00018000
#define SDR_RANK1_32MB_128              0x08000000
#define SDR_RANK1_64MB_256              0x08008000
#define SDR_RANK1_EMPTY                 0x00001000

#define SDR_8MB_64   0
#define SDR_4MB_16   1
#define SDR_8MB_16   2
#define SDR_16MB_64  3
#define SDR_32MB_128 4
#define SDR_64MB_256 5

#ifdef BITFIELDS
typedef struct _SDR_MEM_CFG
{
  BIT_FLD SyncmemCfg:11,
          Rank0Empty:1,
          Rank1Empty:1,
          Rank0Size:2,
          Rank1Size:2,
          AdrModeLrg:1,
          WbstpLatency:1,
          RbstpLatency:1,
          RCDCnt:1,
          RfshCycleCnt:1,
          RASPreCnt:1,
          ByteSwap:1,
          ResetCfg:1,
          tRASExtend:1,
          Rank0SizeMSB:1,
          Rank1SizeMSB:1,
          CloseWAMBA:1,
          CloseLT4:1,
          CloseGT4:1,
          tRASLen:1;
} SDR_MEM_CFG;
typedef SDR_MEM_CFG volatile *LPSDR_MEM_CFG;

typedef struct _SDR_PROC_CRIT
{
  BIT_FLD Count:16,
          Immediate:1,
          Enable:1,
          Reserved1:14;
} SDR_PROC_CRIT;
typedef SDR_PROC_CRIT volatile *LPSDR_PROC_CRIT;

typedef struct _SDR_LOW_POWER
{
  BIT_FLD Enable:1,
          Reserved1:30,
          MCAvailable:1;
} SDR_LOW_POWER;
typedef SDR_LOW_POWER volatile *LPSDR_LOW_POWER;
#endif /* BITFIELDS */

#define SDR_REF_CNT_MASK        0x000000ff

/* Bit masks to be used when accessing SDR_LOW_POWER_REG as a HW_DWORD */
#define SDR_LOW_POWER_ENABLE   0x00000001
#define SDR_LOW_POWER_MC_AVAIL 0x80000000

/* To be completed - Chris, how do you get into and out of auto-refresh mode ? */

#endif

/**********************/
/* Synchronous Serial */
/**********************/

/* Last updated 3/1/99 */

#ifdef INCL_SYN

/*
******************************************************************************
 *
 *     SYN_CONTROL_REG                                 (SYN_BASE + 0x00000000)
 *     SYN_FIFO_CTL_REG                                (SYN_BASE + 0x00000004)
 *     SYN_BR_DIVISOR_REG                              (SYN_BASE + 0x00000008)
 *     SYN_FIFO_REG                                    (SYN_BASE + 0x00000010)
 *     SYN_STATUS_REG                                  (SYN_BASE + 0x00000014)
 *
 *****************************************************************************
 */

#define SYN_CONTROL_REG                                (SYN_BASE + 0x00000000)
#define     SYN_CONTROL_ENABLE_MASK                                0x00000001
#define     SYN_CONTROL_ENABLE_SHIFT                                      0
#define     SYN_CONTROL_CLOCKHIINACTIVE_MASK                       0x00000002
#define     SYN_CONTROL_CLOCKHIINACTIVE_SHIFT                             1
#define     SYN_CONTROL_CLOCKSAMPLEFALLING_MASK                    0x00000004
#define     SYN_CONTROL_CLOCKSAMPLEFALLING_SHIFT                          2
#define     SYN_CONTROL_ORDERLSBFIRST_MASK                         0x00000008
#define     SYN_CONTROL_ORDERLSBFIRST_SHIFT                               3
#define     SYN_CONTROL_DATASIZE_MASK                              0x00000080
#define     SYN_CONTROL_DATASIZE_SHIFT                                    7
#define     SYN_CONTROL_ENABLELOOPBACK_MASK                        0x00000400
#define     SYN_CONTROL_ENABLELOOPBACK_SHIFT                             10
#define     SYN_CONTROL_TXINTENABLE_MASK                           0x00001000
#define     SYN_CONTROL_TXINTENABLE_SHIFT                                12
#define     SYN_CONTROL_RXINTENABLE_MASK                           0x00002000
#define     SYN_CONTROL_RXINTENABLE_SHIFT                                13
#define     SYN_CONTROL_RXOVERRUNINTENABLE_MASK                    0x00004000
#define     SYN_CONTROL_RXOVERRUNINTENABLE_SHIFT                         14
#define     SYN_CONTROL_IDLEINTENABLE_MASK                         0x00008000
#define     SYN_CONTROL_IDLEINTENABLE_SHIFT                              15
#define SYN_FIFO_CTL_REG                               (SYN_BASE + 0x00000004)
#define     SYN_FIFO_CTL_CLEARRXFIFO_MASK                          0x00000002
#define     SYN_FIFO_CTL_CLEARRXFIFO_SHIFT                                1
#define     SYN_FIFO_CTL_CLEARTXFIFO_MASK                          0x00000004
#define     SYN_FIFO_CTL_CLEARTXFIFO_SHIFT                                2
#define     SYN_FIFO_CTL_TXTHRESHOLD_MASK                          0x00000030
#define     SYN_FIFO_CTL_TXTHRESHOLD_SHIFT                                4
#define     SYN_FIFO_CTL_RXTHRESHOLD_MASK                          0x000000C0
#define     SYN_FIFO_CTL_RXTHRESHOLD_SHIFT                                6
#define SYN_BR_DIVISOR_REG                             (SYN_BASE + 0x00000008)
#define     SYN_BR_DIVISOR_DIVISOR_MASK                            0x000000FF
#define     SYN_BR_DIVISOR_DIVISOR_SHIFT                                  0
#define SYN_FIFO_REG                                   (SYN_BASE + 0x00000010)
#define     SYN_FIFO_DATA_MASK                                     0x0000FFFF
#define     SYN_FIFO_DATA_SHIFT                                           0
#define SYN_STATUS_REG                                 (SYN_BASE + 0x00000014)
#define     SYN_STATUS_TXREQ_MASK                                  0x00000001
#define     SYN_STATUS_TXREQ_SHIFT                                        0
#define     SYN_STATUS_RXREQ_MASK                                  0x00000002
#define     SYN_STATUS_RXREQ_SHIFT                                        1
#define     SYN_STATUS_RXOVERRUN_MASK                              0x00000004
#define     SYN_STATUS_RXOVERRUN_SHIFT                                    2
#define     SYN_STATUS_TXFIFODEPTH_MASK                            0x000000F0
#define     SYN_STATUS_TXFIFODEPTH_SHIFT                                  4
#define     SYN_STATUS_RXFIFODEPTH_MASK                            0x00000F00
#define     SYN_STATUS_RXFIFODEPTH_SHIFT                                  8

/* Tx FIFO Threshold values */
#define SYN_TX_FIFO_14_EMPTY   0x00
#define SYN_TX_FIFO_12_EMPTY   0x01
#define SYN_TX_FIFO_34_EMPTY   0x02
#define SYN_TX_FIFO_ALL_EMPTY  0x03

/* Rx FIFO Threshold values */
#define SYN_RX_FIFO_18_FULL    0x00
#define SYN_RX_FIFO_14_FULL    0x01
#define SYN_RX_FIFO_12_FULL    0x02
#define SYN_RX_FIFO_34_FULL    0x03

#ifdef BITFIELDS
typedef struct _SYN_CONTROL
{
  BIT_FLD Enable:1,
          ClockHiInactive:1,
          ClockSampleFalling:1,
          OrderLSBFirst:1,
          Reserved1:3,
          DataSize:1,
          Reserved2:2,
          EnableLoopback:1,
          Reserved3:1,
          TxIntEnable:1,
          RxIntEnable:1,
          RxOverrunIntEnable:1,
          IdleIntEnable:1,
          Reserved4:16;
} SYN_CONTROL;
typedef SYN_CONTROL volatile *LPSYN_CONTROL;

typedef struct _SYN_FIFO_CTL
{
  BIT_FLD Reserved1:1,
          ClearRxFifo:1,
          ClearTxFifo:1,
          Reserved2:1,
          TxThreshold:2,
          RxThreshold:2,
          Reserved3:16;
} SYN_FIFO_CTL;
typedef SYN_FIFO_CTL volatile *LPSYN_FIFO_CTL;

typedef struct _SYN_BR_DIVISOR
{
  BIT_FLD Divisor:8,
          Reserved1:24;
} SYN_BR_DIVISOR;
typedef SYN_BR_DIVISOR volatile *LPSYN_BR_DIVISOR;

typedef struct _SYN_FIFO
{
  BIT_FLD Data:16,
          Reserved1:16;
} SYN_FIFO;
typedef SYN_FIFO volatile *LPSYN_FIFO;


typedef struct _SYN_STATUS
{
  BIT_FLD TxReq:1,
          RxReq:1,
          RxOverrun:1,
          Reserved1:1,
          TxFifoDepth:4,
          RxFifoDepth:4,
          Reserved2:20;
} SYN_STATUS;
typedef SYN_STATUS volatile *LPSYN_STATUS;
#endif /* BITFIELDS */

#endif

/*************************/
/* Pulse Width Modulator */
/*************************/

/* Last updated 3/29/00 */

#ifdef INCL_PWM

/*
******************************************************************************
 *
 *     PWM_IRQR_REG                                    (PWM_BASE + 0x00000000)
 *     PWM_CONTROL_REG                                 (PWM_BASE + 0x00000004)
 *     PWM_PULSE_REG                                   (PWM_BASE + 0x00000008)
 *     PWM_STATUS_REG                                  (PWM_BASE + 0x0000000C)
 *
 *****************************************************************************
 */

#define PWM_IRQR_REG                                   (PWM_BASE + 0x00000000)
#define PWM_CONTROL_REG                                (PWM_BASE + 0x00000004)
#define     PWM_CONTROL_ENABLE_MASK                        0x00000001
#define     PWM_CONTROL_ENABLE_SHIFT                              0
#define     PWM_CONTROL_IDLE_HIGH_MASK                     0x00000002
#define     PWM_CONTROL_IDLE_HIGH_SHIFT                           1
#define     PWM_CONTROL_CONT_MASK                          0x00000004
#define     PWM_CONTROL_CONT_SHIFT                                2
#define     PWM_CONTROL_ENBL_RDY_INT_MASK                  0x00000008
#define     PWM_CONTROL_ENBL_RDY_INT_SHIFT                        3
#define     PWM_CONTROL_ENBL_DONE_INT_MASK                 0x00000010
#define     PWM_CONTROL_ENBL_DONE_INT_SHIFT                       4
#define     PWM_CONTROL_RELOAD_MASK                        0x00000020
#define     PWM_CONTROL_RELOAD_SHIFT                              5
#define     PWM_CONTROL_PRESCALE_MASK                      0x00000700
#define     PWM_CONTROL_PRESCALE_SHIFT                            8
#define PWM_PULSE_REG                                  (PWM_BASE + 0x00000008)
#define     PWM_PULSE_FIRST_MASK                           0x0000FFFF
#define     PWM_PULSE_FIRST_SHIFT                                 0
#define     PWM_PULSE_SECOND_MASK                          0xFFFF0000
#define     PWM_PULSE_SECOND_SHIFT                               16
#define PWM_STATUS_REG                                 (PWM_BASE + 0x0000000C)

#define PWM_READY_MASK 0x01
#define PWM_DONE_MASK  0x02

#define PWM_NUM_READY(x) (PWM_READY_MASK << ((x) * 2))
#define PWM_NUM_DONE(x)  (PWM_DONE_MASK << ((x) * 2))

#ifdef BITFIELDS
typedef struct _PWM_IRQR
{
  BIT_FLD Ready0:1,
          Done0:1,
          Reserved1:30;
} PWM_IRQR;
typedef PWM_IRQR volatile *LPPWM_IRQR;

typedef struct _PWM_CONTROL
{
  BIT_FLD Enable:1,
          IdleHigh:1,
          Continuous:1,
          EnableReadyInt:1,
          EnableDoneInt:1,
          Reload:1,
          Reserved1:2,
          Prescale:3,
          Reserved2:21;
} PWM_CONTROL;
typedef PWM_CONTROL volatile *LPPWM_CONTROL;

typedef struct _PWM_PULSE
{
  BIT_FLD First:16,
          Second:16;
} PWM_PULSE;
typedef PWM_PULSE volatile *LPPWM_PULSE;

typedef struct _PWM_STATUS
{
  BIT_FLD Busy:1,
          Reserved1:31;
} PWM_STATUS;
typedef PWM_STATUS volatile *LPPWM_STATUS;
#endif /* BITFIELDS */

#endif

/***************/
/* Pulse Timer */
/***************/
#ifdef INCL_PLS

/* Last updated 3/3/99 */

/*
******************************************************************************
 *
 *     PLS_CONTROL_REG                                       (PLS_BASE + 0x00)
 *     PLS_FILTER_CLK_DIV_REG                                (PLS_BASE + 0x04)
 *     PLS_PULSE_TIMER_DIV_REG                               (PLS_BASE + 0x08)
 *     PLS_STATUS_REG                                        (PLS_BASE + 0x0C)
 *     PLS_FIFO_DATA_REG                                     (PLS_BASE + 0x10)
 *
 *****************************************************************************
 */

#define PLS_CONTROL_REG                                      (PLS_BASE + 0x00)
#define     PLS_CONTROL_ENABLE_MASK                               0x00000001
#define     PLS_CONTROL_ENABLE_SHIFT                                     0
#define     PLS_CONTROL_FIFO_INT_CNTL_MASK                        0x00000002
#define     PLS_CONTROL_FIFO_INT_CNTL_SHIFT                              1
#define     PLS_CONTROL_EDGE_CNTL_MASK                            0x0000000C
#define     PLS_CONTROL_EDGE_CNTL_SHIFT                                  2
#define     PLS_CONTROL_FIFO_INT_ENABLE_MASK                      0x00000010
#define     PLS_CONTROL_FIFO_INT_ENABLE_SHIFT                            4
#define     PLS_CONTROL_OVERRUN_INT_ENABLE_MASK                   0x00000020
#define     PLS_CONTROL_OVERRUN_INT_ENABLE_SHIFT                         5
#define     PLS_CONTROL_TIMEOUT_INT_ENABLE_MASK                   0x00000040
#define     PLS_CONTROL_TIMEOUT_INT_ENABLE_SHIFT                         6
#define PLS_FILTER_CLK_DIV_REG                               (PLS_BASE + 0x04)
#define PLS_PULSE_TIMER_DIV_REG                              (PLS_BASE + 0x08)
#define PLS_STATUS_REG                                       (PLS_BASE + 0x0C)
#define     PLS_STATUS_FIFO_SERVICE_MASK                          0x00000001
#define     PLS_STATUS_FIFO_SERVICE_SHIFT                                0
#define     PLS_STATUS_FIFO_OVERRUN_MASK                          0x00000002
#define     PLS_STATUS_FIFO_OVERRUN_SHIFT                                1
#define     PLS_STATUS_TIMEOUT_MASK                               0x00000004
#define     PLS_STATUS_TIMEOUT_SHIFT                                     2
#define     PLS_STATUS_BUSY_MASK                                  0x00000008
#define     PLS_STATUS_BUSY_SHIFT                                        3
#define PLS_FIFO_DATA_REG                                    (PLS_BASE + 0x10)

#define PLS_FIFO_INT_HALF       0
#define PLS_FIFO_INT_NOT_EMPTY  1

#define PLS_EDGE_DISABLE        0x00
#define PLS_EDGE_FALLING        0x01
#define PLS_EDGE_RISING         0x02
#define PLS_EDGE_EITHER         0x03

/* Absolute values for writing the whole register at once */
#define PLS_DATA_MASK  0x0000FFFF
#define PLS_BIT_LEVEL  0x00010000
#define PLS_DATA_AVAIL 0x00020000

#ifdef BITFIELDS
typedef struct _PLS_CONTROL
{
  BIT_FLD Enable:1,
          FifoIntControl:1,
          EdgeControl:2,
          FifoIntEnable:1,
          OverrunIntEnable:1,
          TimeoutIntEnable:1,
          Reserved1:25;
} PLS_CONTROL;
typedef PLS_CONTROL volatile *LPPLS_CONTROL;

typedef struct _PLS_FILTER_CLK_DIV
{
  BIT_FLD Timeout:16,
          Reserved1:16;
} PLS_FILTER_CLK_DIV;
typedef PLS_FILTER_CLK_DIV volatile *LPPLS_FILTER_CLK_DIV;

typedef struct _PLS_PULSE_TIMER_DIV
{
  BIT_FLD Timeout:16,
          Reserved1:16;
} PLS_PULSE_TIMER_DIV;
typedef PLS_PULSE_TIMER_DIV volatile *LPPLS_PULSE_TIMER_DIV;

typedef struct _PLS_STATUS
{
  BIT_FLD FifoService:1,
          FifoOverrun:1,
          Timeout:1,
          Busy:1,
          Reserved1:28;
} PLS_STATUS;
typedef PLS_STATUS volatile *LPPLS_STATUS;

typedef struct _PLS_FIFO_DATA
{
  BIT_FLD Data:16,
          State:1,
          DataAvailable:1,
          Reserved1:14;
} PLS_FIFO_DATA;
typedef PLS_FIFO_DATA volatile *LPPLS_FIFO_DATA;
#endif /* BITFIELDS */

#endif

/*************************/
/* ASB Bridge/DMA Engine */
/*************************/
#ifdef INCL_DMA

/* Last updated 10/10/01 ... rwb */
/* to incorporate DMA changes in Wabash chip */

/* 
     Implementation Note :

     In CX2249X Silicon it should be noted that if the SRC or DEST DMA
	 is a PORT, one still needs to program the DMA_SRC/DESTADDR_REG_CH(x)
	 reg with the port's address (i.e. just programming the DMA_SRC/DESTBASE_REG_CH(x)
	 with the port address is not sufficient). It is also required the program
     DMA_SRC/DSTBUF_REG_CH(x) with "0" when using a port.

*/

#define DMA_CAPABILITY_REG   (DMA_BASE + 0x000) /* indicates # DMA channels in chip */
#define DMA_MODE_REG         (DMA_BASE + 0x004) /* enables additional functionality */
  #define DMA_BUFFERED 0x4   /* Enable Buffered DMA  */
  #define DMA_MULTIPLE 0x2   /* Enable multiple concurrent DMA operations */
  #define DMA_BUS_LOCK 0x1   /* Enable bus locking  */
#define DMA_INT_REG          (DMA_BASE + 0x008) /* current channels requesting service (RO) */
#define DMA_REQSTAT_REG      (DMA_BASE + 0x00C) /* reflects which devices are asserting DMA request */
#define DVTIMER_COUNT        (DMA_BASE + 0x010)
#define DVTIMER_LIMIT        (DMA_BASE + 0x014)
#define DVTIMER_MODE         (DMA_BASE + 0x018)

#define DMA_NUM_CHANNELS 4   /* Cx2249x silicon */
  #define  DMA_CH_0 0x1
  #define  DMA_CH_1 0x2
  #define  DMA_CH_2 0x4
  #define  DMA_CH_3 0x8
  #define  DMA_ATA  0x100	/* Used for DMA_INT_REG register only */

/* ** per channel registers	** */

#define DMA_CHANNEL_REG_BASE (DMA_BASE + 0x100)	/* offset to Channel 0 registers */
#define DMA_CHANNEL_SIZE     0x20               /* 8-32 bit registers per channel */

/* offsets from channel(n) base */
#define DMA_CONTROL_OFFSET   (DMA_CHANNEL_REG_BASE + 0x00)
#define DMA_STATUS_OFFSET    (DMA_CHANNEL_REG_BASE + 0x04)
#define DMA_SRCADDR_OFFSET   (DMA_CHANNEL_REG_BASE + 0x08)
#define DMA_DSTADDR_OFFSET   (DMA_CHANNEL_REG_BASE + 0x0C)
#define DMA_SRCBASE_OFFSET   (DMA_CHANNEL_REG_BASE + 0x10)
#define DMA_SRCBUF_OFFSET    (DMA_CHANNEL_REG_BASE + 0x14)
#define DMA_DSTBASE_OFFSET   (DMA_CHANNEL_REG_BASE + 0x18)
#define DMA_DSTBUF_OFFSET    (DMA_CHANNEL_REG_BASE + 0x1C)

#define DMA_CONTROL_REG_CH(x)  (((x)*DMA_CHANNEL_SIZE)+DMA_CONTROL_OFFSET) /* per channel configuration */
   /* Masks  */
   #define DMA_CHANNEL_RECOVERY_TIME(x)   (x<<16)   /* Recovery Time after DMA Op */
   #define DMA_CHANNEL_RECOVERY_TIME_MULT (1<<15)   /* Recovery Time Multiplier */
   #define DMA_CHANNEL_NON_PACED          (1<<14)   /* Non-Paced DMA */
   #define DMA_XACT_SZ(x)                 ((x-1)<<9)/* Move this many 32-bit words */
   #define DMA_SELECT_REQ(x)              (x<<4)    /* DMA request line for channel */
   #define DMA_XFER_MODE_NORMAL           (0<<3)    /* Normal operation (DMA req triggers Xfer) */
   #define DMA_XFER_MODE_SINGLE           (1<<3)    /* Single-Shot operation (Xfer buffer then terminate) */
   #define DMA_PREEMPT                    (1<<2)    /* Channel may preempt other DMA */
   #define DMA_INT_ENABLE                 (1<<1)    /* Enable DMA Interrupt */
   #define DMA_ENABLE                     1         /* Enable state machine */

#define DMA_STATUS_REG_CH(x)   (((x)*DMA_CHANNEL_SIZE)+DMA_STATUS_OFFSET) /* current status per channel */
   /* Masks  */
   #define DMA_READ_ERROR               (1<<6)     
   #define DMA_WRITE_ERROR              (1<<5)
   #define DMA_DST_INTERVAL             (1<<4)    /* Destination buffer has reached trigger level */
   #define DMA_SRC_INTERVAL             (1<<3)    /* Source buffer has reached trigger level */
   #define DMA_DST_FULL                 (1<<2)    /* DMA engine has wrapped around to start of destination buffer */
   #define DMA_SRC_EMPTY                (1<<1)    /* DMA engine has wrapped around to start of source buffer */
   #define DMA_ACTIVE                   1         /* Indicates whether or not a DMA xfer is currently active */

#define DMA_SRCADDR_REG_CH(x)   (((x)*DMA_CHANNEL_SIZE)+DMA_SRCADDR_OFFSET) /* current state machine address in source buffer */
#define DMA_DSTADDR_REG_CH(x)   (((x)*DMA_CHANNEL_SIZE)+DMA_DSTADDR_OFFSET) /* current state machine address in destination buffer */
#define DMA_SRCBASE_REG_CH(x)   (((x)*DMA_CHANNEL_SIZE)+DMA_SRCBASE_OFFSET) /* base address of source buffer */
   #define DMA_NONINCR                  (0)       /* Non-incrementing address (port)  */
   #define DMA_SRCINCR                  (1)       /* Incrementing address (buffer) */
#define DMA_SRCBUF_REG_CH(x)    (((x)*DMA_CHANNEL_SIZE)+DMA_SRCBUF_OFFSET)  /* source buffer size and interrupt interval */
#define DMA_DSTBASE_REG_CH(x)   (((x)*DMA_CHANNEL_SIZE)+DMA_DSTBASE_OFFSET) /* base address of destination buffer */
   /* see defines under DMA_SRCBASE_REG_CH(x) above .. */
#define DMA_DSTBUF_REG_CH(x)    (((x)*DMA_CHANNEL_SIZE)+DMA_DSTBUF_OFFSET)  /* destination buffer size and interrupt interval */


/* Definitions for new "STRIDE" registers
** note:  Overlap DMA Channel 7
*/
#define DMA_STRIDE_CTL          (DMA_CHANNEL_REG_BASE + 0x01e0)
#define DMA_STRIDE_STAT         (DMA_CHANNEL_REG_BASE + 0x01e4)
#define DMA_STRIDE_SRCADDR      (DMA_CHANNEL_REG_BASE + 0x01e8)
#define DMA_STRIDE_DSTADDR      (DMA_CHANNEL_REG_BASE + 0x01ec)
#define DMA_STRIDE_SRCBASE      (DMA_CHANNEL_REG_BASE + 0x01f0)
#define DMA_STRIDE_RUN          (DMA_CHANNEL_REG_BASE + 0x01f4)
#define DMA_STRIDE_DSTBASE      (DMA_CHANNEL_REG_BASE + 0x01f8)
#define DMA_STRIDE_MSTRIDE      (DMA_CHANNEL_REG_BASE + 0x01fc)


#endif

#ifdef INCL_DMD

/* What we call DMD_ here is actually the Cable PHY interface
   (CPHY in the spec, see section 11.7)  */

#define DMD_DS_CTRL1_BASE                         0x00360000
#define DMD_DS_CTRL3_BASE                         0x0036003C
#define DMD_DS_CTRL4_BASE                         0x00360044

#define DMD_DS_CTRL_CPHY_RR                       0x80000000
#define DMD_DS_CTRL_CPHY_IDLE                     0x40000000
#define DMD_DS_CTRL_CPHY_RW                       0x01000000
#define DMD_DS_CTRL_CPHY_SRR                      0x00FF0000
#define DMD_DS_CTRL_CPHY_ADR                      0x0000FF00
#define DMD_DS_CTRL_CPHY_DAT                      0x000000FF

/* Default symbol rates for 64QAM is 0x483df5 (4,734,453) 
   for 256QAM is 0x4c9440 (5,018,688) */
#define DMD_CD_SYMBOL_SRATE_FREQ_MSB_OFFSET       0x01
#define DMD_CD_SYMBOL_SRATE_FREQ_CSB_OFFSET       0x02
#define DMD_CD_SYMBOL_SRATE_FREQ_LSB_OFFSET       0x03

#define DMD_CD_QAM_MODE_OFFSET                    0x04
 #define DMD_CD_QAM_MODE_QAM4                      0x5
 #define DMD_CD_QAM_MODE_QAM16                     0x1
 #define DMD_CD_QAM_MODE_QAM32                     0x2
 #define DMD_CD_QAM_MODE_QAM64                     0x4
 #define DMD_CD_QAM_MODE_QAM128                    0x6
 #define DMD_CD_QAM_MODE_QAM256MCNS                0x3
 #define DMD_CD_QAM_MODE_QAM256DAVIC               0x7

#define DMD_CD_SPECTRAL_INV_OFFSET                0x5
 #define DMD_CD_SPECTRAL_INV_NORMAL                0x0
 #define DMD_CD_SPECTRAL_INV_INVERTED              0x10

#define DMD_CD_SOFT_RESET_OFFSET                  0x9
 #define DMD_CD_FEC_SOFT_RESET_ALL                 0x10
 #define DMD_CD_MOD_SOFT_RESET_ALL                 0x02
 #define DMD_CD_SOFT_RESET_MACHINE                 0x01

#define DMD_CDB_MPEG_CONTROL_OFFSET               0xA
 #define DMD_CDB_MP_BYPASS                         0x80
 #define DMD_CDB_MP_PACKET_MODE                    0x40
 #define DMD_CDB_MP_ERROR_OUT_MODE                 0x20
 #define DMD_CDB_MP_OUT_OF_SYNC_DIS                0x10
 #define DMD_CDB_RS_ER_CORRECT_DIS                 0x08
 #define DMD_CDB_MP_SYNC_WD_OVRWRT_DIS             0x04
 #define DMD_CDB_MP_TRANS_ERR_IND_DIS              0x02
 #define DMD_CDB_MP_ERROR_MODE                     0x01

#define DMD_CDB_MPEG_PACKET_START_OFFSET          0xB
 #define DMD_CDB_MP_PUNCT_SYNC_WORD                0x40
 #define DMD_CDB_MP_PACKET_START_POL               0x08


#define DMD_CDB_REED_SOLOMON_CONTROL_OFFSET       0xC
 #define DMD_CDB_RS_OUT_EDGE_POLARITY              0x02
 #define DMD_CDB_RS_OUTPUT_MODE                    0x01

#define DMD_CDB_RS_FAILURE_COUNT_OFFSET           0xD
 #define DMD_CDB_RS_FAILURE_COUNT                  0xFF

#define DMD_CDB_RS_MPEG_MISC_CONTROL_OFFSET       0xE
 #define DMD_CDB_MPSYNC                            0x80
 #define DMD_CDB_RSOOSYNC                          0x20
 #define DMD_CDB_SYNC_LOST                         0x10
 #define DMD_CDB_RSECD                             0x02
 #define DMD_CDB_RSCNTEN                           0x01

#define DMD_CDB_RS_SYMB_ERR_CNT_MSB_OFFSET        0x10
#define DMD_CDB_RS_SYMB_ERR_CNT_LSB_OFFSET        0x11
 #define DMD_CDB_RS_SYMB_ERR_CNT                   0xFF

#define DMD_CDB_TS_BLOCK_ERR_CNT_MSB_OFFSET       0x12
#define DMD_CDB_TS_BLOCK_ERR_CNT_LSB_OFFSET       0x13
 #define DMD_CDB_RS_BLOCK_ERR_CNT                   0xFF

#define DMD_CDB_FEC_SYNC_OFFSET                   0x17
 #define DMD_FEC_OUT_OF_SYNC                       0x80
 #define DMD_FEC_IN_SYNC                           0x40
 #define DMD_FEC_MAJ_LOGIC_DONE                    0x20
 #define DMD_FEC_OUT_OF_SYNC_DISABLE               0x02
 #define DMD_FEC_OUT_OF_SYNC_MODE                  0x01

#define DMD_CDB_DEINT_ACTUAL_MODE_OFFSET          0x1B
 #define DMD_CDB_DEINT_ACTUAL_MODE                 0x0F

#define DMD_CDB_TR_SYNC_CONTROL_OFFSET            0x20
 #define DMD_TR_SYNC                               0x08
 #define DMD_TR_SYNC_STATE                         0x04
 #define DMD_TR_MANUAL_SYNC                        0x02
 #define DMD_TR_SYNC_ADVANCE                       0x01

#define DMD_CD_AAGC_ACCUMULATOR_STATUS_OFFSET     0x27
 #define DMD_AAGC_ACCUMULATOR_MSB                  0xFF

#define DMD_CD_AAGC_CONTROL_OFFSET                0x29
 #define DMD_AAGC_SD_POL                           0x80
 #define DMD_AAGC_PRESCALAR                        0x60
 #define DMD_AAGC_GAIN                             0x07

#define DMD_CD_PHACC_FREQ_MSB_OFFSET              0x2A
#define DMD_CD_PHACC_FREQ_LSB_OFFSET              0x2B
 #define DMD_PHACC_FREQ                            0xFF

#define DMD_CD_DAGC_CONTROL_OFFSET                0x34
 #define DMD_DAGC2_MODE                            0x80
 #define DMD_DAGC2_GAIN                            0x70
 #define DMD_DAGC2_MANUAL                          0x04
 #define DMD_DAGC1_GAIN                            0x03

#define DMD_CD_EQ_CONTROL_OFFSET                  0x37
 #define DMD_EQCTL_DIV_8                           0x08
 #define DMD_EQCTL_LINEAR_EQ                       0x04
 #define DMD_EQCTL_RST_COEFF                       0x02
 #define DMD_EQCTL_CT_1024                         0x01

#define DMD_CD_EQ_RING_SELECT_OFFSET              0x39
 #define DMD_RING_SELECT_ALL_RINGS                 0x08
 #define DMD_RING_RING_SELECT                      0x03

#define DMD_CD_EQ_ADAPTER_GAIN_OFFSET             0x3A
 #define DMD_ADAPTER_GAIN                          0x0F

#define DMD_CD_EQ24_TAP_SELECT_OFFSET             0x39
 #define DMD_EQ24_TAP_FORCE_SAT                    0x80
 #define DMD_EQ24_TAP_UPD_TAP_READOUT              0x10
 #define DMD_EQ24_TAP_SELECT_REAL_TAP              0x08
 #define DMD_EQ24_TAP_TAP_SELECT                   0x07

#define DMD_CD_EQ_ACCUMULATOR_MSB_OFFSET          0x3D
#define DMD_CD_EQ_COEFFICIENT_MSB_OFFSET          0x3E
#define DMD_CD_EQ_ACC_AND_COEF_LSB                0x3F
 #define DMD_EQ24_ACCUMULATOR                      0xFF
 #define DMD_EQ24_COEFFICIENT                      0xFF

#define DMD_CD_EQ_ADAPTER_CONTROL_OFFSET          0x40
 #define DMD_ADAPTER_MODE                          0x04
 #define DMD_ADAPTER_MICRO_SELECTS_LMS_MODE        0x02
 #define DMD_ADAPTER_MICRO_CONTROL_ADAPT_MODE      0x01

#define DMD_CD_EQ24_ROT2_CONTROL_OFFSET           0x43
 #define DMD_ROT2_SMCOR_DISABLE                    0x80
 #define DMD_ROT2_PHASE_GAIN                       0x40
 #define DMD_ROT2_FIXED_GAIN                       0x07

#define DMD_CD_FEC_STATUS_TO_PADS_OFFSET          0x46
 #define DMD_FEC_STATUS_MPEG_SYNC                  0x40
 #define DMD_FEC_STATUS_RS_SYNC                    0x20
 #define DMD_FEC_STATUS_FRAME_SYNC                 0x10
 #define DMD_FEC_STATUS_TRELLIS_SYNC               0x08
 #define DMD_FEC_STATUS_SELECT                     0x07

#define DMD_CD_CTL_ACCUMULATOR_OFFSET             0x48
 #define DMD_CTL_ACCUMULATOR                       0xFF

#define DMD_CD_CTL_TRACKING_GAINS_OFFSET          0x4A
 #define DMD_TRK_IND_GAIN                          0xE0
 #define DMD_TRK_FRACT_GAIN                        0x18
 #define DMD_TRK_DIR_GAIN                          0x07

#define DMD_CD_SCTL_LOCK_CONTROL_OFFSET           0x55
 #define DMD_SCTL_LOCK_MODE                        0xC0
 #define DMD_SCTL_LOCK_INDICATOR                   0x20
 #define DMD_SCTL_LOCK_GAIN                        0x10
 #define DMD_SCTL_ACQUIS_TO_TRACK_DIS              0x02
 #define DMD_SCTL_NO_INDIRECT_PATH                 0x01

#define DMD_CD_RFAGC_SD_ACCUMULATOR_OFFSET        0x56
 #define DMD_RFAGC_SD_ACCUMULATOR                  0xFF

#define DMD_CD_RFAGC_SD_MIN_THRESHOLD_OFFSET      0x57
#define DMD_CD_RFAGC_SD_MAX_THRESHOLD_OFFSET      0x57
 #define DMD_RFAGC_THRESHOLD                       0xFF

#define DMD_CD_RFAGC_SD_CONTROL_OFFSET            0x56
 #define DMD_RFAGC_PRESCALAR                       0xC0
 #define DMD_RFAGC_SD_POL                          0x20
 #define DMD_RFAGC_LF_POL                          0x10
 #define DMD_RFAGC_COUNTER_GAIN                    0x0C
 #define DMD_RFAGC_LF_GAIN                         0x03

#define DMD_CD_RFAGC_SD_DATAIN_MSB_OFFSET         0x5A
 #define DMD_RFAGC_SD_DATAIN_MSB                   0xFF

#define DMD_CD_MEAN_SQUARE_ERR_MSB_OFFSET         0x69
#define DMD_CD_MEAN_SQUARE_ERR_LSB_OFFSET         0x6A
 #define DMD_MEAN_SQUARE_ERR                       0xFF

#define DMD_CD_TUNER_AUTO_MODE_OFFSET             0x6B
 #define DMD_TUNER_AUTO_MODE                       0x80
 #define DMD_TUNER_AUTO_MODE_CLK_POL               0x40
 #define DMD_TUNER_AUTO_MODE_FRAME_SIZE            0x3F

#endif


#ifdef INCL_CM_PERIPHERALS

#define CPHY_DS_CTRL1_REG                         0x0036003c
#define CPHY_DS_CTRL3_REG                         0x00360000
#define CPHY_DS_CTRL4_REG                         0x00360044
  #define CPHY_RR                                   (0x01 << 31)
  #define CPHY_IDLE                                 (0x01 << 30)
  #define CPHY_RW_MASK                              (0x01 << 24)
    #define CPHY_READ                                 (0x01 << 24)
    #define CPHY_WRITE                                (0x00 << 24)
  #define CPHY_SYMRATERATIO                         (0xff << 16)
    #define CPHY_SYMRATERATIO_SHIFT                          16
  #define CPHY_ADDR                                 (0xff <<  8)
    #define CPHY_ADDR_SHIFT                                   8
  #define CPHY_DAT                                  (0xff <<  0)
    #define CPHY_DAT_SHIFT                                    0


#define GP0_IO_REG                                0x350010 
#define GP_IO_ALLBANKS_IO_REG                     0x350010 
#define GP1_IO_REG                                0x350014 
#define GP_IO_ALLBANKS_OE_REG                     0x350014 
#define GP2_IO_REG                                0x350018 
#define GP3_IO_REG                                0x35001c 
  #define  GP_IO_BWE                                 ( 0xff << 16 )
    #define  GP_IO_BWE_SHIFT                                    16
  #define  GP_IO_OE                                  ( 0xff <<  8 )
    #define  GP_IO_OE_SHIFT                                     8
  #define  GP_IO_IO                                  ( 0xff <<  0 )
  #define  GP3_I2C1_EN                               ( 0x01 << 31 )
  #define  GP3_I2C0_EN                               ( 0x01 << 24 )
  /* if set, GP0_IO_REG twiddles all GPIO bits ([0-31]), not just GP0[0-7] */  
  #define  GP3_MODE_SIMULTANEOUS                     ( 0x01 << 25 )
  #define  GP3_RFP_EN                                ( 0x01 << 26 )
  #define  GP3_ECM_EN                                ( 0x03 << 28 )
  #define  GP3_TC_EN                                 ( 0x01 << 30 )

  #define  GP_IO_ALLBANKS_IO                         0xFFFFFFFF
  #define  GP_IO_ALLBANKS_OE                         0xFFFFFFFF

#define I2C_MSTR_A_REG                            0x360010
#define I2C_MSTR_C_REG                            0x360018
#define I2C_MSTR_D_REG                            0x36001C
  #define I2C_DB0                                    ( 0xff << 24 )
  #define I2C_DB1                                    ( 0xff << 16 )
  #define I2C_DB2                                    ( 0xff <<  8 )
  #define I2C_ENABLE_HARDWARE_MODE                   ( 0x01 <<  7 )
  #define I2C_RATE_MASK                              ( 0x01 <<  6 )
    #define I2C_RATE_100KHZ                          ( 0x00 <<  6 )
    #define I2C_RATE_400KHZ                          ( 0x01 <<  6 )
  #define I2C_NOSTOP                                 ( 0x01 <<  5 )
  #define I2C_NOS1B                                  ( 0x01 <<  4 )
  #define I2C_1BYTE_NOSTART                          ( 0x01 <<  4 )
  #define I2C_SYNC                                   ( 0x01 <<  3 )
  #define I2C_W3BRA                                  ( 0x01 <<  2 )
  #define I2C_WRITE3_OR_READ1                        ( 0x01 <<  2 ) 
  #define I2C_SCL                                    ( 0x01 <<  1 )
  #define I2C_SDA                                    ( 0x01 <<  0 )
#define I2C_CTRL_REG                              0x360020
  #define I2C_OUT                                    ( 0x0f <<  0 )
    #define I2C_OUT_SHIFT                                       0 


#endif




/***************************************/
/* Hardware buffer addresses and sizes */
/***************************************/

/* Buffer sizes */

/* Note: Buffer sizes in K bytes must be defined in CONFIG files or a */
/*       previously included customer header file.                    */

#define HWBUF_PCRPTS_SIZE      128
#define HWBUF_ENCAUD_SIZE      (ENCAUD_SIZE * 1024)
#define HWBUF_AUDTMP_SIZE      ( 13 * 1024)
#define HWBUF_DECAUD1_SIZE     ( 24 * 1024)
#define HWBUF_DECAUD2_SIZE     ( 24 * 1024)
#define HWBUF_ENCVID_SIZE      (ENCVID_SIZE * 1024)
#define HWBUF_USRDAT_SIZE      (USRDAT_SIZE * 1024)
#define HWBUF_AUDANC_SIZE      (AUDANC_SIZE * 1024)
#define HWBUF_DEC_I_SIZE       (DEC_I_SIZE * 1024)
#define HWBUF_DEC_P_SIZE       (DEC_P_SIZE * 1024)
#define HWBUF_DEC_B_SIZE       (DEC_B_SIZE * 1024)
#define HWBUF_TRNSPRT_SIZE     (TRNSPRT_SIZE * 1024)
#define HWBUF_DVR_EVENT_SIZE   (DVR_EVENT_SIZE * 1024)
#define HWBUF_DVR_VIDEO_SIZE   (DVR_VIDEO_SIZE * 1024)
#define HWBUF_DVR_AUDIO_SIZE   (DVR_AUDIO_SIZE * 1024)

#define DPS_NUM_ECM_SLOTS        6
#define DPS_NUM_EMM_SLOTS        10
#define DPS_ECM_SLOT_SIZE        256
#define DPS_EMM_SLOT_SIZE        256

/* Fixed size, fixed position buffers */
#define HWBUF_PCRPTS_ADDR        0x000380
#define HWBUF_AUDTMP_ADDR        (HWBUF_PCRPTS_ADDR        + HWBUF_PCRPTS_SIZE)
#define HWBUF_DECAUD1_ADDR       (HWBUF_AUDTMP_ADDR        + HWBUF_AUDTMP_SIZE)
#define HWBUF_DECAUD2_ADDR       (HWBUF_DECAUD1_ADDR       + HWBUF_DECAUD1_SIZE)
#define HWBUF_ENCAUD_ADDR        (HWBUF_DECAUD2_ADDR       + HWBUF_DECAUD2_SIZE)
#define HWBUF_ENCVID_ADDR        (HWBUF_ENCAUD_ADDR        + HWBUF_ENCAUD_SIZE)

#define HWBUF_USRDAT_ADDR        (HWBUF_ENCVID_ADDR        + HWBUF_ENCVID_SIZE)
#define HWBUF_AUDANC_ADDR        (HWBUF_USRDAT_ADDR        + HWBUF_USRDAT_SIZE)
#define HWBUF_DEC_I_ADDR         (HWBUF_AUDANC_ADDR        + HWBUF_AUDANC_SIZE)
#define HWBUF_DEC_P_ADDR         (HWBUF_DEC_I_ADDR         + HWBUF_DEC_I_SIZE)
#define HWBUF_DEC_B_ADDR         (HWBUF_DEC_P_ADDR         + HWBUF_DEC_P_SIZE)

#define HWBUF_TRNSPRT_ADDR       (HWBUF_DEC_B_ADDR         + HWBUF_DEC_B_SIZE)
#define HWBUF_DVR_EVENT_ADDR     (HWBUF_TRNSPRT_ADDR       + HWBUF_TRNSPRT_SIZE)
#define HWBUF_DVR_VIDEO_ADDR     (HWBUF_DVR_EVENT_ADDR     + HWBUF_DVR_EVENT_SIZE)
#define HWBUF_DVR_AUDIO_ADDR     (HWBUF_DVR_VIDEO_ADDR     + HWBUF_DVR_VIDEO_SIZE)
#define HWBUF_DVR_AUDIO1_ADDR    (HWBUF_DVR_AUDIO_ADDR     + HWBUF_DVR_AUDIO_SIZE)
#define HWBUF_DVR_AUDIO2_ADDR    (HWBUF_DVR_AUDIO1_ADDR    + HWBUF_DVR_AUDIO_SIZE)

#define HWBUF_CAM_ECM_START_ADDR (HWBUF_DVR_AUDIO2_ADDR    + HWBUF_DVR_AUDIO_SIZE)
#define HWBUF_CAM_ECM_SIZE       ((DPS_NUM_DEMUXES-1)*DPS_NUM_ECM_SLOTS*DPS_ECM_SLOT_SIZE)
#define HWBUF_CAM_EMM_START_ADDR (HWBUF_CAM_ECM_START_ADDR + HWBUF_CAM_ECM_SIZE)
#define HWBUF_CAM_EMM_SIZE       ((DPS_NUM_DEMUXES-1)*DPS_NUM_EMM_SLOTS*DPS_EMM_SLOT_SIZE)

#define HWBUF_ENCAC3_ADDR        (HWBUF_CAM_EMM_START_ADDR + HWBUF_CAM_EMM_SIZE)

#define HWBUF_TOP                (HWBUF_ENCAC3_ADDR        + HWBUF_ENCAUD_SIZE)

#define NUM_OF_GPIO              96

#endif   /* #ifdef _WABASH_H_ */

/****************************************************************************
 * Modifications:
 * $Log: 
 *  150  STB1.8.0  1.148.1.0   6/17/04 4:23:26 PM     Tim White       CR(s) 
 *        9484 : Remove changes made by version 1.147 for patch release.
 *        
 *  149  mpeg      1.148       6/16/04 11:12:31 AM    Tim White       CR(s) 
 *        9483 9484 : Changed pvr to rec for consistency.
 *        
 *  148  mpeg      1.147       6/14/04 9:26:37 AM     Larry Wang      CR(s) 
 *        9433 9434 : Add definitions for DirecTV specific registers.
 *  147  mpeg      1.146       5/24/04 4:44:43 PM     Tim White       CR(s) 
 *        9296 9297 : Add MPG_OFFSET registers for PVR.
 *        
 *  146  mpeg      1.145       5/4/04 2:21:41 PM      Tim White       CR(s) 
 *        9088 9089 : Update trick mode video microcode command set to match 
 *        spec.
 *        
 *  145  mpeg      1.144       4/22/04 5:05:24 PM     Sunil Cheruvu   CR(s) 
 *        8870 8871 : Added the Nand Flash support for Wabash(Milano rev 5 and 
 *        above) and Brazos(Bronco).
 *  144  mpeg      1.143       4/12/04 10:39:09 AM    Larry Wang      CR(s) 
 *        8826 8827 : Define DRM_FILTER_PHASE_0_PROBLEM as YES.
 *  143  mpeg      1.142       4/1/04 5:18:04 PM      Dave Aerne      CR(s) 
 *        8729 8730 : added support for ac3 passthrough w/o mpeg decode on 
 *        brazos and wabash chips.
 *  142  mpeg      1.141       3/23/04 3:12:00 PM     Larry Wang      CR(s) 
 *        8638 8639 : Redefine event buffer pointers for XTV.
 *  141  mpeg      1.140       3/10/04 2:54:17 PM     Larry Wang      CR(s) 
 *        8551 : Add definition of registers for Sony Passage.
 *  140  mpeg      1.139       3/10/04 10:41:29 AM    Bob Van Gulick  CR(s) 
 *        8546 : Add support to return PES frame rate
 *  139  mpeg      1.138       3/2/04 11:04:12 AM     Tim Ross        CR(s) 
 *        8451 : Added DPS_TS_BLOCK_COUNT_EX macro.
 *  138  mpeg      1.137       2/24/04 2:42:59 PM     Bob Van Gulick  CR(s) 
 *        8427 : Add timebase_offset functionality to demux
 *        
 *  137  mpeg      1.136       2/11/04 8:41:20 AM     Steven Jones    CR(s) 
 *        8382 : Add 14:9 format macros MPG_VID_ASPECT_149 and 
 *        MPG_VID_ASPECT_RATIO_149 to
 *        allow support for this aspect ratio which is included in DTG spec.
 *  136  mpeg      1.135       2/4/04 3:01:38 PM      Dave Wilson     CR(s) 
 *        8318 : Added definition of DRM_NEEDS_SPLIT_FIFO_ON_WIPE to allow 
 *        conditional compilation of workaround code to fix a chroma problem on
 *         image plane wipes with Brazos rev B.
 *        
 *  135  mpeg      1.134       1/27/04 11:45:59 AM    Mark Thissen    CR(s) 
 *        8269 : Fixing Formatting Error.
 *        
 *  134  mpeg      1.133       1/26/04 3:31:39 PM     Mark Thissen    CR(s) 
 *        8269 : Definitions for early sync reg for fast channel change.
 *        
 *  133  mpeg      1.132       1/23/04 3:07:46 PM     Angela Swartz   CR(s) 
 *        8264 : corrected a typo in ENC_CTL0_BLUEFIELD_ENABLE 1UL<<7, it was 
 *        1UL<<8
 *  132  mpeg      1.131       11/1/03 2:46:49 PM     Tim Ross        CR(s): 
 *        7719 7762 Added definitions for RST_SCRATCH_REG bits.
 *  131  mpeg      1.130       10/31/03 5:35:43 PM    Tim Ross        CR(s): 
 *        7719 7762 Added definitions of expansion scratch registers.
 *  130  mpeg      1.129       10/30/03 4:25:01 PM    Tim Ross        CR(s): 
 *        7719 7762 Added definitions for scratch register 0.
 *  129  mpeg      1.128       10/27/03 5:11:39 PM    Senthil Veluswamy CR(s): 
 *        7723 1) Updated register defines for ALT_FUNC and PIN_MUX registers 
 *        for Milano SC0 support
 *        2) removed / / comments
 *        3) Removed old pvcs log
 *  128  mpeg      1.127       10/11/03 4:52:18 PM    Craig Dry       Enable 
 *        Power Driver to work with Wabash.
 *  127  mpeg      1.126       9/19/03 6:48:16 PM     Dave Aerne      SCR(s) 
 *        7514 :
 *        cleanup to map CORE_RESET defines to standard CONTROL0 defines
 *        
 *  126  mpeg      1.125       9/2/03 2:08:56 PM      Angela Swartz   SCR(s) 
 *        7202 :
 *        added definitions for OSD1_CRIT in DWI control1 register
 *        
 *  125  mpeg      1.124       8/28/03 3:52:08 PM     Lucy C Allevato SCR(s) 
 *        7397 :
 *        removed the bit masks for DAC E and DAC F for wabash because wabash 
 *        has only 4 DACs instead of 6, also defined the disable DAC bit in the
 *         encoder control register
 *        
 *  124  mpeg      1.123       8/18/03 2:32:14 PM     Miles Bintz     SCR(s) 
 *        7291 :
 *        added mmy type definitions
 *        
 *  123  mpeg      1.122       8/1/03 2:56:00 PM      Sahil Bansal    SCR(s): 
 *        7107 
 *        Fix for Test Harness build break.  Added back in defines for 
 *        PLL_DIV1_HSDP_CLK_MASK and _SHIFT.
 *        
 *  122  mpeg      1.121       7/30/03 1:45:26 PM     Sahil Bansal    SCR(s): 
 *        7077 
 *        Change define PLL_DIV1_HSDP_CLK_MASK and PLL_DIV1_HSDP_CLK_SHIFT to 
 *        PLL_DIV2_HSDP_CLK_MASK and PLL_DIV2_HSDP_CLK_SHIFT and changed values
 *         to match wabash ic spec.
 *        
 *  121  mpeg      1.120       7/2/03 2:08:16 PM      Miles Bintz     SCR(s) 
 *        6807 :
 *        added GPIO pin mux definitions
 *        
 *  120  mpeg      1.119       6/9/03 5:56:56 PM      Bob Van Gulick  SCR(s) 
 *        6755 :
 *        Add support for 8 slot descram in demux.  Needed to move registers 
 *        between 1E0 and 200 to a different location to support this feature.
 *        
 *        
 *  119  mpeg      1.118       6/6/03 11:22:44 AM     Dave Wilson     SCR(s) 
 *        6737 :
 *        Added definitions for CHIP_SUPPORTS_xxx_RESET.
 *        
 *  118  mpeg      1.117       6/3/03 10:15:14 AM     Larry Wang      SCR(s) 
 *        6667 :
 *        Define HWBUF_ENCAC3_ADDR as the start pointer for the buffer of 
 *        secondary audio channel (AC3 pass through channel).
 *        
 *  117  mpeg      1.116       5/22/03 5:30:56 PM     Dave Wilson     SCR(s) 
 *        6564 6565 :
 *        Added definitions of CACHE_LINE_SIZE and NONCACHEABLE_RAM_MASK. 
 *        Previously
 *        these were in various KAL files.
 *        
 *  116  mpeg      1.115       5/20/03 6:11:50 PM     Dave Wilson     SCR(s) 
 *        6499 6500 :
 *        Added ENCODER_HAS_DAC_BUG for Brazos rev B.
 *        
 *  115  mpeg      1.114       5/15/03 3:34:50 PM     Dave Wilson     SCR(s) 
 *        6367 6366 :
 *        Added new macro DRM_HAS_16_BIT_CURSOR_BUG.
 *        
 *  114  mpeg      1.113       5/9/03 7:00:42 PM      Steve Glennon   SCR(s): 
 *        6224 6225 6190 6179 
 *        Addition of FA_BLEND_PLANE1
 *        
 *        
 *  113  mpeg      1.112       5/6/03 7:29:18 PM      Senthil Veluswamy SCR(s) 
 *        6215 :
 *        Fixed a Cut and paste bug in defining the GPI banks.
 *        
 *  112  mpeg      1.111       5/6/03 6:56:02 PM      Senthil Veluswamy SCR(s) 
 *        6215 6216 :
 *        New define for PCI Int bug and increased NUM GPI Banks to 4 if bug is
 *         present
 *        
 *  111  mpeg      1.110       5/6/03 5:12:50 PM      Steve Glennon   SCR(s) 
 *        6224 6225 :
 *        Added definition of DRM_OSD0_POINTER_REG (same as 
 *        DRM_OSD_POINTER_REG) and 
 *        DRM_OSD1_POINTER_REG to allow use of second graphics plane.
 *        
 *        
 *  110  mpeg      1.109       5/5/03 4:03:42 PM      Tim White       SCR(s) 
 *        6172 :
 *        Add ROM_BASE definition.
 *        
 *        
 *  109  mpeg      1.108       5/2/03 11:14:28 AM     Bob Van Gulick  SCR(s) 
 *        6151 :
 *        Add pts_offset and dma_mux memory locations
 *        
 *        
 *  108  mpeg      1.107       4/28/03 4:34:28 PM     Miles Bintz     SCR(s) 
 *        6115 :
 *        Added cnxt_drm_init_filter_coefficients register definition
 *        
 *  107  mpeg      1.106       4/23/03 11:08:12 AM    Dave Wilson     SCR(s) 
 *        5862 :
 *        Added DRM_TILE_TYPE chip feature due to changes in Brazos.
 *        
 *  106  mpeg      1.105       4/4/03 3:43:24 PM      Tim Ross        SCR(s) 
 *        5969 :
 *        Added new EVID register definitions.
 *        
 *  105  mpeg      1.104       4/1/03 6:24:54 PM      Miles Bintz     SCR(s) 
 *        5654 :
 *        corrected names of GPIO's
 *        
 *  104  mpeg      1.103       4/1/03 4:02:36 PM      Senthil Veluswamy SCR(s) 
 *        5930 :
 *        Added defines for Analog Audio (Audio Frame reg, MPG Control 1 reg, 
 *        GPIO Mux1 reg)
 *        
 *  103  mpeg      1.102       3/13/03 12:22:10 PM    Miles Bintz     SCR(s) 
 *        5753 :
 *        Added HSDP_BIDIR_PORT_CNTL_SER_TO_PAR_CONV_DISABLE definition to chip
 *         header file
 *        
 *        
 *  102  mpeg      1.101       3/5/03 4:06:10 PM      Dave Wilson     SCR(s) 
 *        5588 :
 *        Previous version of the file had the DRM cursor registers defined as 
 *        they
 *        were for Colorado. Wabash, however, contains the new register mapping
 *         (as also
 *        found in Brazos) so cursors were corrupted when used with the old 
 *        header. 
 *        Register definitions and DRM_CURSOR_FETCH_TYPE have been corrected 
 *        and cursors
 *        now look correct.
 *        
 *  101  mpeg      1.100       3/5/03 3:51:16 PM      Miles Bintz     SCR(s) 
 *        5634 :
 *        added CHIP_SUPPORTS_PIO_TIMED_RESET feature definition to chip header
 *         files
 *        
 *        
 *  100  mpeg      1.99        2/28/03 11:15:38 AM    Dave Aerne      SCR(s) 
 *        4672 :
 *        for audio AUTO_DETECT feature, added DPS_AUDIO_PID_STREAM_ID_EX at 
 *        address
 *        DPS_BASE + 0x1FC
 *        
 *  99   mpeg      1.98        2/19/03 10:51:02 AM    Dave Wilson     SCR(s) 
 *        5528 5543 :
 *        Added 2 new chip features to enable workarounds for a couple of DRM 
 *        bugs:
 *        DRM_HAS_WIPE_CHROMA_BUG = YES 
 *        DRM_HAS_UPSCALE_PAL_STILL_BUG = YES
 *        
 *  98   mpeg      1.97        2/5/03 11:48:52 AM     Dave Wilson     SCR(s) 
 *        5405 :
 *        Added ENC_CTL0_ALL_DAC_MASK and ENC_CTL0_ALL_DAC_SHIFT to allow all 
 *        DACs to
 *        be controlled in a single operation.
 *        
 *  97   mpeg      1.96        2/4/03 5:07:58 PM      Bob Van Gulick  SCR(s) 
 *        5407 :
 *        Change scrambling status to use separate TS and PES registers
 *        
 *        
 *  96   mpeg      1.95        1/24/03 7:00:42 PM     Brendan Donahe  SCR(s) 
 *        5315 :
 *        Updated 16-bit MC audio workaround fix with PCM_AUDIO_TYPE 
 *        definition.
 *        
 *        
 *  95   mpeg      1.94        1/24/03 2:35:52 PM     Brendan Donahe  SCR(s) 
 *        5315 :
 *        Added 3 audio (AUD_CHANNEL_CONTROL_REG) definitions - copyright, 
 *        6-channel 
 *        test, and MPEG PCM capture attenuation.
 *        
 *        
 *  94   mpeg      1.93        1/15/03 3:51:32 PM     Billy Jackman   SCR(s) 
 *        5095 :
 *        Added specification of INTERNAL_DEMOD as NOT_PRESENT.
 *        
 *  93   mpeg      1.92        12/17/02 4:05:02 PM    Senthil Veluswamy SCR(s) 
 *        5067 :
 *        removed IIC macros to private header
 *        
 *  92   mpeg      1.91        12/17/02 3:54:32 PM    Tim White       SCR(s) 
 *        5182 :
 *        Remove ARM_PIT_TYPE, no longer needed.
 *        
 *        
 *  91   mpeg      1.90        12/16/02 3:48:30 PM    Tim White       SCR(s) 
 *        5169 :
 *        Allow future chips to use Wabash code by default instead of the 
 *        Colorado code.
 *        
 *        
 *  90   mpeg      1.89        12/12/02 5:17:52 PM    Tim White       SCR(s) 
 *        5157 :
 *        Removed the runtime Wabash Rev_B (A) detection macro since Rev_A no 
 *        longer supported.
 *        Added the def'n for supporting PCI_ROM_MODE_SYNC_bits and removed the
 *         PCI_ROM_MODE_REG_TYPE def'n.
 *        
 *        
 *  89   mpeg      1.88        12/10/02 3:03:34 PM    Dave Wilson     SCR(s) 
 *        5071 :
 *        Added VIDEO_UCODE_RAM_SIZE_WORDS chip feature to define the size of 
 *        the video
 *        microcode RAM.
 *        
 *  88   mpeg      1.87        12/10/02 1:34:26 PM    Dave Wilson     SCR(s) 
 *        5091 :
 *        Reworked interrupt controller bit positions
 *        
 *  87   mpeg      1.86        12/10/02 10:11:32 AM   Bob Van Gulick  SCR(s) 
 *        5121 :
 *        Add scrambler status register for demux
 *        
 *        
 *  86   mpeg      1.85        12/6/02 5:12:24 PM     Dave Wilson     SCR(s) 
 *        4903 :
 *        Added new chip features to differentiate some DRM function changes in
 *         Brazos
 *        
 *  85   mpeg      1.84        10/30/02 2:46:06 PM    Bobby Bradford  SCR(s) 
 *        4866 :
 *        Fixed the PLL_PIN_ALT_FUNCT_AFE_DAA_DEL values ...
 *        the values in the header files matched the specs, but the
 *        spec is backwards.
 *        Also, for wabash, moved the definition of the mux0 spkr bit from
 *        bit 17 to bit 24 (for Kobuk ... Wabash is actually a don't care)
 *        
 *  84   mpeg      1.83        10/21/02 4:47:18 PM    Senthil Veluswamy SCR(s) 
 *        4790 :
 *        added define for the Audio AC3 Align Error
 *        
 *  83   mpeg      1.82        10/11/02 12:19:58 PM   Miles Bintz     SCR(s) 
 *        4431 :
 *        Added hsx arbiter definitions
 *        
 *        
 *  82   mpeg      1.81        10/10/02 3:57:00 PM    Brendan Donahe  SCR(s) 
 *        4774 :
 *        Changed SMC_TIME_CYCLES_MASK to 32 bits for Wabash B only.
 *        
 *        
 *  81   mpeg      1.80        9/18/02 5:25:18 PM     Joe Kroesche    SCR(s) 
 *        4610 :
 *        added DPS_ macro needed for crc notification feature
 *        
 *  80   mpeg      1.79        9/17/02 5:51:24 PM     Carroll Vance   SCR(s) 
 *        4613 :
 *        Added HAS_INTERNAL_PCI32 and HAS_EXTERNAL_PCI32
 *        
 *  79   mpeg      1.78        9/3/02 12:21:28 PM     Senthil Veluswamy SCR(s) 
 *        4502 :
 *        Put back Bob's changes for demux, that I overwrote in the previous 
 *        put
 *        
 *  78   mpeg      1.77        8/30/02 6:33:32 PM     Senthil Veluswamy SCR(s) 
 *        4502 :
 *        Defines for using the New IIC interface
 *        
 *  77   mpeg      1.76        8/30/02 3:02:12 PM     Larry Wang      SCR(s) 
 *        4499 :
 *        (1) define interrupt expansion registers; (2) define INTEXP_TYPE as 
 *        INTEXP_WABASH.
 *        
 *  76   mpeg      1.75        8/30/02 2:40:32 PM     Bob Van Gulick  SCR(s) 
 *        4485 :
 *        Add defines to support CRC checking in demux driver
 *        
 *        
 *  75   mpeg      1.74        8/22/02 6:02:22 PM     Brendan Donahe  SCR(s) 
 *        4464 :
 *        Repaired DRMYCCPAL_CB_* and DRMYCCPAL_CR_* to correctly match YCbCr 
 *        ordering
 *        applicable to default 8 bit palette
 *        
 *        
 *  74   mpeg      1.73        8/22/02 4:02:50 PM     Larry Wang      SCR(s) 
 *        4459 :
 *        Define PCI_ROM_MODE_REG_TYPE.
 *        
 *  73   mpeg      1.72        8/15/02 7:23:12 PM     Tim Ross        SCR(s) 
 *        4409 :
 *        Moved all default feature definitions here from hwconfig. Also moved 
 *        ISWABASH macro here from startup.h.
 *        
 *  72   mpeg      1.71        8/15/02 3:55:56 PM     Tim White       SCR(s) 
 *        4398 :
 *        Added new ATA bits for Rev_B.
 *        
 *        
 *  71   mpeg      1.70        8/14/02 4:54:58 PM     Dave Wilson     SCR(s) 
 *        4371 :
 *        Corrected definition of DRM_COLOR_KEY_REG contents. Should be 24 bits
 *         +
 *        8 reserved bits rather than 16 + 16 reserved.
 *        
 *  70   mpeg      1.69        6/14/02 11:08:40 AM    Carroll Vance   SCR(s) 
 *        3786 :
 *        Allowed use of DRM bitfields only in individual c files by putting 
 *        #define USE_BITFIELD_DRM
 *        at the top of a c file before #include'ing any header files that
 *        themselves #include colorado.h, hondo.h, or wabash.h.  DRM bitfields
 *        are pervasive, especially the RBG and YCC bitfields.
 *        
 *  69   mpeg      1.68        6/13/02 1:32:38 PM     Miles Bintz     SCR(s) 
 *        4001 :
 *        Added defines for peripherals on the CM side of the chip.
 *        
 *  68   mpeg      1.67        6/13/02 10:56:16 AM    Carroll Vance   SCR(s) 
 *        3786 :
 *        Turned off use of DRM bitfields
 *        
 *  67   mpeg      1.66        6/6/02 5:54:36 PM      Craig Dry       SCR(s) 
 *        3923 :
 *        Remove MPG bitfields conditionally - Step 1
 *        
 *  66   mpeg      1.65        6/3/02 2:49:20 PM      Craig Dry       SCR(s) 
 *        3922 :
 *        Conditionally remove SYN bitfield structures from header files
 *        
 *  65   mpeg      1.64        6/3/02 12:52:16 PM     Carroll Vance   SCR(s) 
 *        3918 :
 *        Forgot to change one use of PLL_LOCK_CMD to PLL_LOCK_CMD_VALUE. Fixed
 *         it.
 *        
 *  64   mpeg      1.63        6/3/02 12:43:48 PM     Carroll Vance   SCR(s) 
 *        3918 :
 *        Changed PLL_LOCK_CMD to PLL_LOCK_CMD_VALUE to avoid conflict with 
 *        bitfield type PLL_LOCK_CMD when customer uses this file with code 
 *        containing use of PLL_LOCK_CMD bitfield type.
 *        
 *  63   mpeg      1.62        5/31/02 7:29:06 PM     Craig Dry       SCR(s) 
 *        3916 :
 *        Eradicate bitfield use from AFE code
 *        
 *  62   mpeg      1.61        5/31/02 12:09:26 PM    Craig Dry       SCR(s) 
 *        3908 :
 *        Eradicate bitfield use from Smart Card(SMC) code, step 1
 *        
 *  61   mpeg      1.60        5/30/02 12:33:02 PM    Tim White       SCR(s) 
 *        3899 :
 *        Add WARM_RESET support.
 *        
 *        
 *  60   mpeg      1.59        5/22/02 5:42:24 PM     Craig Dry       SCR(s) 
 *        3855 :
 *        Eradicate bitfield use from SER(Serial) code, step 1
 *        
 *  59   mpeg      1.58        5/21/02 2:05:58 PM     Tim White       SCR(s) 
 *        3642 :
 *        Add DPS_PCR_BLOCK_STORED bit definition for new pawser microcode 
 *        build option -dPCR_PID_INT.
 *        
 *        
 *  58   mpeg      1.57        5/21/02 1:32:42 PM     Carroll Vance   SCR(s) 
 *        3786 :
 *        Added LPREG in front of DRM register addresses.
 *        
 *  57   mpeg      1.56        5/20/02 12:55:26 PM    Steve Glennon   SCR(s): 
 *        3830 
 *        Changed the CONTROL1 bitfield Reserved2 field to HaltDecode to match 
 *        colorado.h
 *        
 *        
 *  56   mpeg      1.55        5/17/02 12:17:44 PM    Craig Dry       SCR(s) 
 *        3810 :
 *        Eradicate bitfield use from IRD code
 *        
 *  55   mpeg      1.54        5/15/02 3:09:14 PM     Craig Dry       SCR(s) 
 *        3794 :
 *        Eradicate bitfields from PAR module
 *        
 *  54   mpeg      1.53        5/15/02 2:56:28 PM     Carroll Vance   SCR(s) 
 *        3786 :
 *        Added constants for DRM bitfields.
 *        
 *  53   mpeg      1.52        5/15/02 12:19:38 PM    Carroll Vance   SCR(s) 
 *        3789 1238924 :
 *        Clean up audio bitfield masks.
 *        
 *  52   mpeg      1.51        5/14/02 5:24:08 PM     Craig Dry       SCR(s) 
 *        3784 :
 *        Eradicate bitfields from PWM(Pulse Width Modulator)
 *        
 *  51   mpeg      1.50        5/14/02 11:45:40 AM    Craig Dry       SCR(s) 
 *        3776 :
 *        Eradicate bitfield use from PLS(Pulse Timer), step 1
 *        
 *  50   mpeg      1.49        5/13/02 1:55:48 PM     Tim White       SCR(s) 
 *        3760 :
 *        Updated DPS_ definition section.
 *        
 *        
 *  49   mpeg      1.48        5/10/02 3:19:56 PM     Craig Dry       SCR(s) 
 *        3754 :
 *        Eradicate bitfield use from SDR(SDRAM) code
 *        
 *  48   mpeg      1.47        5/7/02 5:58:18 PM      Craig Dry       SCR(s) 
 *        3726 :
 *        Eradicate bitfield use from I2C, step 1
 *        
 *  47   mpeg      1.46        5/7/02 5:10:14 PM      Tim White       SCR(s) 
 *        3721 :
 *        Remove ENC_ bitfield usage from codebase.
 *        
 *        
 *  46   mpeg      1.45        5/6/02 5:59:40 PM      Tim White       SCR(s) 
 *        3714 :
 *        Update HSDP_ macro definitions to use new bitfield-less format.
 *        
 *        
 *  45   mpeg      1.44        5/3/02 3:49:26 PM      Craig Dry       SCR(s) 
 *        3671 :
 *        Eradicate bitfield use from TIM module
 *        
 *  44   mpeg      1.43        5/3/02 2:47:08 PM      Tim White       SCR(s) 
 *        3696 :
 *        Remove RST_ bitfields from codebase.
 *        
 *        
 *  43   mpeg      1.42        5/3/02 1:56:58 PM      Dave Wilson     SCR(s) 
 *        3688 :
 *        Added definitions of new parser events used to detect data on audio 
 *        and 
 *        video PIDs.
 *        
 *  42   mpeg      1.41        5/2/02 5:25:46 PM      Tim White       SCR(s) 
 *        3693 :
 *        Use "UL" in the macro bit definitions to force unsigned and fix 
 *        compiler warnings.
 *        
 *        
 *  41   mpeg      1.40        5/1/02 7:00:22 PM      Carroll Vance   SCR(s) 
 *        3659 :
 *        Removed audio bitfield types.
 *        
 *  40   mpeg      1.39        5/1/02 5:45:10 PM      Tim White       SCR(s) 
 *        3678 :
 *        Remove GCP_ & ARM_ bitfields usage from the codebase.
 *        
 *        
 *  39   mpeg      1.38        5/1/02 3:31:06 PM      Tim White       SCR(s) 
 *        3673 :
 *        Remove PLL_ bitfields usage from codebase.
 *        
 *        
 *  38   mpeg      1.37        4/30/02 5:38:14 PM     Joe Kroesche    SCR(s) 
 *        3243 :
 *        added definition for new parser event bit (bad PES header)
 *        
 *  37   mpeg      1.36        4/30/02 4:20:30 PM     Craig Dry       SCR(s) 
 *        3662 :
 *        Use same build configurations as were used to test defect 3509. 
 *        
 *  36   mpeg      1.35        4/30/02 2:55:58 PM     Carroll Vance   SCR(s) 
 *        3659 :
 *        Removing bitfields from audio, step 1.
 *        
 *        
 *  35   mpeg      1.34        4/30/02 12:59:04 PM    Billy Jackman   SCR(s) 
 *        3656 :
 *        Changed expression (1<<31) to ((unsigned)1<<31) to avoid compiler 
 *        warning
 *        in SDT toolset.
 *        
 *  34   mpeg      1.33        4/29/02 6:17:16 PM     Craig Dry       SCR(s) 
 *        3652 :
 *        Eliminate bitfield use from Movie (MOV) module by conditional 
 *        compilation.
 *        
 *  33   mpeg      1.32        4/24/02 4:27:30 PM     Dave Wilson     SCR(s) 
 *        3611 :
 *        Updated video size register bit definitions to match latest 
 *        microcode. For
 *        Colorado and Hondo this affects only MPG_VID_SIZE_REG but for Wabash 
 *        the
 *        changes were made to MPG_ADDR_EXT_REG since the affected bits have 
 *        moved to a
 *        different register.
 *        
 *  32   mpeg      1.31        4/22/02 3:22:24 PM     Larry Wang      SCR(s) 
 *        3585 :
 *        Add definition of NUM_OF_GPIO for COLORADO, HONDO and WABASH chips.  
 *        Any GPIO bit number that is bigger than NUM_OF_GPIO means GPIO 
 *        extender.
 *        
 *  31   mpeg      1.30        4/19/02 7:32:18 PM     Senthil Veluswamy SCR(s) 
 *        3521 :
 *        Added defines for Uart sharing between CM & MPEG (PIN_ALT_FUNC_REG)
 *        
 *  30   mpeg      1.29        4/19/02 5:53:00 PM     Craig Dry       SCR(s) 
 *        3588 :
 *        Conditionally compile out RTC bitfield definitions.
 *        
 *  29   mpeg      1.28        4/18/02 11:11:46 AM    Craig Dry       SCR(s) 
 *        3576 :
 *        GXA bitfield definitions are now conditionally compiled out.
 *        
 *  28   mpeg      1.27        4/12/02 7:50:36 PM     Tim White       SCR(s) 
 *        3558 :
 *        Remove hardware register access macros.  Now in hwconfig.h.
 *        
 *        
 *  27   mpeg      1.26        4/11/02 3:15:28 PM     Tim White       SCR(s) 
 *        3543 :
 *        Changed BITFIELD #define to be in the <chipname>.h header files since
 *        nothing in these files can depend on config options from config files
 *         or
 *        hw/swconfig.h
 *        
 *        
 *  26   mpeg      1.25        4/10/02 5:15:50 PM     Tim White       SCR(s) 
 *        3509 :
 *        Eradicate external bus (PCI) bitfield usage.
 *        
 *        
 *  25   mpeg      1.24        4/5/02 2:23:24 PM      Tim White       SCR(s) 
 *        3510 :
 *        Backout DCS #3468
 *        
 *        
 *  24   mpeg      1.23        4/3/02 10:16:32 AM     Dave Wilson     SCR(s) 
 *        3494 :
 *        Fixed CNXT_REG_SET, CNXT_REG_GET, CNXT_FLAG_SET and CNXT_FLAG_GET 
 *        macros.
 *        Previous versions did not include correct parentheses in definition 
 *        causing
 *        macros to expand incorrectly in cases where the passed parameters 
 *        were
 *        complex expressions rather than single values.
 *        
 *  23   mpeg      1.22        3/28/02 3:35:30 PM     Quillian Rutherford 
 *        SCR(s) 3468 :
 *        Added bit flags to replace bit fields.
 *        
 *        
 *  22   mpeg      1.21        3/5/02 4:56:12 PM      Carroll Vance   SCR(s) 
 *        3307 :
 *        Modified 6 demux macros to use multiinstance interface.
 *        
 *        
 *  21   mpeg      1.20        3/4/02 5:49:42 PM      Tim White       SCR(s) 
 *        3299 :
 *        Add ATA_TYPE definition and misc req'd for ATA driver.
 *        
 *        
 *  20   mpeg      1.19        3/1/02 6:42:08 PM      Tim Ross        SCR(s) 
 *        3286 :
 *        Enabled clock recovery and AV sync.
 *        
 *  19   mpeg      1.18        2/21/02 4:22:10 PM     Tim White       SCR(s) 
 *        3229 :
 *        Use NUM_PARSERS definition for DPS_NUM_DEMUXES.
 *        
 *        
 *  18   mpeg      1.17        2/13/02 5:38:52 PM     Senthil Veluswamy SCR(s) 
 *        3149 :
 *        Corrected definitions for Enabling Wabash Uarts
 *        
 *  17   mpeg      1.16        2/7/02 11:53:30 AM     Bob Van Gulick  SCR(s) 
 *        3143 :
 *        Change a couple DPS macros to include multi-instance demux parameters
 *        
 *        
 *  16   mpeg      1.15        2/6/02 2:59:44 PM      Miles Bintz     SCR(s) 
 *        3147 :
 *        Added CPU_TYPE definition
 *        
 *        
 *  15   mpeg      1.14        2/5/02 2:05:20 PM      Ray Mack        SCR(s) 
 *        3132 :
 *        Duplicated Colorado NIM error label so DEMOD_HW will build correctly.
 *        
 *  14   mpeg      1.13        1/31/02 4:38:34 PM     Tim White       SCR(s) 
 *        3106 :
 *        Added generic register #defines for setting/getting register values 
 *        w/o bitfields used
 *        for the new HSDP driver.
 *        
 *        
 *  13   mpeg      1.12        1/15/02 3:29:14 PM     Senthil Veluswamy SCR(s) 
 *        2998 :
 *        added define for a RTC Clock set value (to get a 1micro sec tick), 
 *        updated bit definitions for the ALT_FUNC Reg (Uart2 bits)
 *        
 *  12   mpeg      1.11        1/11/02 10:38:00 AM    Dave Moore      SCR(s) 
 *        3006 :
 *        Added SDRAM 64MB_256, changed 32MB_64 to 32MB_128
 *        
 *        
 *  11   mpeg      1.10        1/9/02 4:20:52 PM      Dave Moore      SCR(s) 
 *        3006 :
 *        Changed GPIO_CONFIG to GPIOM_WABASH
 *        
 *        
 *  10   mpeg      1.9         12/18/01 2:42:14 PM    Miles Bintz     SCR(s) 
 *        2933 :
 *        Changed debug_port to trace_port and INTERNAL_UART1 to SERIAL1
 *        
 *        
 *  9    mpeg      1.8         12/18/01 11:50:48 AM   Bobby Bradford  SCR(s) 
 *        2933 :
 *        Incorporating WabashBranch changes
 *        
 *  8    mpeg      1.7         12/18/01 11:04:38 AM   Miles Bintz     SCR(s) 
 *        2933 :
 *        Merged in HSDP_TYPE, UART_TYPE, HSDP register definitions and some 
 *        QAM demod register definitions
 *        
 *  7    mpeg      1.6         12/7/01 1:56:26 PM     Miles Bintz     SCR(s) 
 *        2933 :
 *        Added UART_TYPE and HSDP_TYPE in chip feature section.  I had already
 *         started defining QAM demod register stuff too, so thats included as 
 *        well.
 *        
 *  6    mpeg      1.5         11/29/01 12:59:34 PM   Miles Bintz     SCR(s) 
 *        2936 :
 *        added extra pci config register offsets
 *        
 *  5    mpeg      1.4         11/20/01 3:55:26 PM    Quillian Rutherford 
 *        SCR(s) 2754 :
 *        Changed GPIOM_WABASH to GPIOM_HONDO since wabash has hondo gpio's
 *        
 *        
 *  4    mpeg      1.3         11/20/01 10:54:14 AM   Quillian Rutherford 
 *        SCR(s) 2911 :
 *        Update for vxworks on Athens/Abilene
 *        
 *  3    mpeg      1.2         11/8/01 3:43:00 PM     Bobby Bradford  SCR(s) 
 *        2753 :
 *        Added definition of PLL_TYPE as WABASH
 *        
 *        
 *  2    mpeg      1.1         11/6/01 2:56:52 PM     Bobby Bradford  SCR(s) 
 *        2753 :
 *        Fixed unterminated comment at EOF
 *        
 *  1    mpeg      1.0         11/6/01 9:20:12 AM     Bobby Bradford  
 * $
 ****************************************************************************/


