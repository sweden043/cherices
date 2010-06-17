/****************************************************************************/
/*                   CONEXANT PROPRIETARY AND CONFIDENTIAL                  */
/*                       SOFTWARE FILE/MODULE HEADER                        */
/*          Conexant Systems Inc. (c) 1998, 1999, 2000, 2001, 2002          */
/*                              Austin, TX                                  */
/*                         All Rights Reserved                              */
/****************************************************************************/
/*
 * Filename:        BRAZOS.H
 *
 *
 * Description:     Public header file defining hardware-specific values
 *                  (such as register addresses, bit definitions, etc)
 *                  for IRD central IC codenamed Brazos.
 *
 *
 * Author:          Dave Wilson
 *
 ****************************************************************************/
/* $Id: brazos.h,v 1.84, 2004-06-16 16:12:44Z, Tim White$
 ****************************************************************************/

#ifndef _BRAZOS_H_
#define _BRAZOS_H_

#include "hwconfig.h"
#include "swconfig.h"

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
/* Display Refresh Module       DRM_              INCL_DRM                  */
/* Audio                        AUD_              INCL_AUD                  */
/* PLLs and Clocking            PLL_              INCL_PLL                  */
/* Reset and Power Management   RST_              INCL_RST                  */
/* Interrupt Controller         ITC_              INCL_ITC                  */
/* Timers                       TIM_              INCL_TIM                  */
/* I2C                          I2C_              INCL_I2C                  */
/* General Purpose IO           GPI_              INCL_GPI                  */
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
/* Internal QPSK Demodulator    DMD_              INCL_DMD                  */
/* Encryption Engine            ECY_              INCL_ECY                  */
/* Internal NTSC modulator      TVM_              INCL_TVM                  */
/****************************************************************************/

/****************************/
/* Chip Feature Definitions */
/****************************/
#define INTERNAL_ENCODER                INTERNAL_BT861_LIKE
#define INTERNAL_DEMOD                  INTERNAL_COBRA_LIKE
#define INTERNAL_DEMOD_HAS_ASX_BUG      YES
#define GPIO_CONFIG                     GPIOM_BRAZOS
#define NUM_PARSERS                     1
#define NUM_OSD_PLANES                  1
#define HAS_INTERNAL_PCI32              YES
#define HAS_EXTERNAL_PCI32              YES
#define HAS_PCI32                       (HAS_INTERNAL_PCI32 || HAS_EXTERNAL_PCI32)
#define HSDP_TYPE                       HSDP_BRAZOS
#define UART_TYPE                       UART_WABASH
#ifndef TRACE_PORT
  #define TRACE_PORT                    SERIAL1
#endif
#define ATA_TYPE                        ATA_BRAZOS

#ifndef CPU_TYPE
  #if (CHIP_REV==AUTOSENSE)
    #define CPU_TYPE                    AUTOSENSE
  #elif (CHIP_REV==REV_A_BRAZOS)
    #define CPU_TYPE                    CPU_ARM940T
    #define MMU_TYPE                    MMU_TYPE_940T
  #else
    #define CPU_TYPE                    CPU_ARM920T
    #define MMU_TYPE                    MMU_TYPE_920T
  #endif
#endif

/*************************************/
/* Chip rev detection identification */
/*************************************/
#if CHIP_REV==AUTOSENSE
  extern unsigned long ChipID;
  extern unsigned long ChipRevID;
  #define ISBRAZOSREVA (((ISBRAZOS)&&(ChipRevID==PCI_REV_ID_A_BRAZOS))?1:0)
#elif CHIP_REV==REV_A_BRAZOS
  #define ISBRAZOSREVA (1)
#else
  #define ISBRAZOSREVA (0)
#endif

/**************************/
/* Chip Features and Bugs */
/**************************/
#define PLL_TYPE                        PLL_TYPE_BRAZOS
#define MPG_SYNC_BIT_POSITION           MPG_SYNC_BIT_POSITION_WABASH
#define HSX_ARBITER                     HSX_ARBITER_WABASH

#define INTEXP_TYPE                     INTEXP_BRAZOS
#define IIC_TYPE                        IIC_TYPE_BRAZOS
#define VIDEO_UCODE_DOWNLOAD_TYPE       VIDEO_UCODE_DOWNLOAD_BRAZOS
#define DRM_SCALER_COEFF_TYPE           DRM_SCALER_COEFF_TYPE_BRAZOS
#define DRM_CURSOR_FETCH_TYPE           DRM_CURSOR_FETCH_TYPE_BRAZOS
#define DRM_HAS_WIPE_CHROMA_BUG         ISBRAZOSREVA
#define DRM_NEEDS_SPLIT_FIFO_ON_WIPE    YES
#define DRM_HAS_UPSCALED_PAL_STILL_BUG  YES
#define DRM_HAS_16_BIT_CURSOR_BUG       ISBRAZOSREVA
#define DRM_FILTER_PHASE_0_PROBLEM      NO
#define DRM_TILE_TYPE                   DRM_TILE_TYPE_BRAZOS
#define ENCODER_HAS_DAC_BUG             YES

#define PCM_AUDIO_TYPE                  PCM_AUDIO_BRAZOS

#define VIDEO_UCODE_RAM_SIZE_WORDS      1728

#define CHIP_SUPPORTS_PIO_TIMED_RESET   YES
#define CHIP_SUPPORTS_SMARTCARD_RESET   YES

#ifndef PLL_PRESCALE_VALUE
  #define PLL_PRESCALE_VALUE            0x00000155
#endif

/******************************************************************************/
/* For Brazos (and future) chips, there are bits in the PCI_ROM_MODE register */
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
      #ifndef INCL_DRM
        #ifndef INCL_AUD
          #ifndef INCL_PLL
            #ifndef INCL_RST
              #ifndef INCL_ITC
                #ifndef INCL_TIM
                  #ifndef INCL_I2C
                    #ifndef INCL_GPI
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
                                                          #ifndef INCL_ECY
                                                            #ifndef INCL_TVM
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

#ifdef INCL_ALL
  #define INCL_MSC
  #define INCL_ARM
  #define INCL_DRM
  #define INCL_AUD
  #define INCL_PLL
  #define INCL_RST
  #define INCL_ITC
  #define INCL_TIM
  #define INCL_I2C
  #define INCL_GPI
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
  #define INCL_ECY
  #define INCL_TVM
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

/***************************/
/* Register Base Addresses */
/***************************/

/* Updated 12/3/02 */
#define BUFF_BASE 0x00000000
#define NCR_BASE  0x10000000
#define ROM_BASE  0x20000000

#define NONCACHEABLE_RAM_MASK (~NCR_BASE)
#define CACHE_LINE_SIZE ((ISBRAZOSREVA)?16:32)

#define MSC_BASE  0x00000000
#define ARM_BASE  0x30060000
#define ECY_BASE  0x300A0000      
#define DRM_BASE  0x30460000
#define AUD_BASE  0x30590000
#define PLL_BASE  0x30440000
#define RST_BASE  0x30000000
#define ITC_BASE  0x30450000
#define TIM_BASE  0x30430000
#define I2C_BASE  0x305E0000
#define GPI_BASE  0x30470000
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
#define DPS_BASE(instance) (HW_DWORD)(DPS0_BASE+((instance)<<25))
#define SDR_BASE  0x30070000
#define PIO_BASE  0x31000000
#define PLS_BASE  0x305B0000
#define SYN_BASE  0x305A0000
#define DMA_BASE  0x30500000
#define PWM_BASE  0x30580000
#define DMD_BASE  0x304E0000
#define TVM_BASE  0x304E1000
#define GXA_BASE  0x304a0000
#define HSDP_BASE 0x304cc000
#define ENC_BASE  0x304d0000

#define TIM_BANK_SIZE 0x0010
#define NUM_TIM_BANKS 8

#define IRD_BANK_SIZE 0x0100
#define NUM_IRD_BANKS 2

/* There are 3 UARTs but the instances are not evenly   */
/* spaced in the address map so we only consider 2 here */
#define SER_BANK_SIZE 0x10000
#define NUM_SER_BANKS 2

#define GPI_BANK_SIZE 0x30
#define NUM_GPI_BANKS 4

#define SMC_BANK_SIZE 0x10000
#define NUM_SMC_BANKS 2

#define IIC_BANK_SIZE 0x0100
#define NUM_IIC_BANKS 2

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

#endif /* INCL_ARM */

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

/* Pixel types from a previous graphics accelerator. Used by some existing code */
#define GCP_1BPP            0x000
#define GCP_2BPP            0x001
#define GCP_4BPP            0x002
#define GCP_8BPP            0x003
#define GCP_16BPP           0x004
#define GCP_32BPP           0x005
#define GCP_24BPP           0x007

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

#define GXA_GQ_ENTRIES     32          /* Dword entries in graphics queue */
#define GXA_NON_QUEUED     0x00000000l /* nonqueued register offset */

/***************************/
/*  32 BIT ACCESS VALUES   */
/***************************/

/* GXA Fifo Depth Register */
#define GUI_BUSY           0x00080000l
#define RESULT_FIFO_BUSY   0x00040000l
#define GUI_QUEUE_BUSY     0x00020000l
#define GUI_FIFO_NON_EMPTY 0x00010000l

#define GUI_GQUE_DEPTH     0x7e000000l

/* Bitmap Context registers fields */
#define BM_TYPE_MASK   0xFF000000l
#define BM_PITCH_MASK  0x00003fffl

#define BM_FORMAT_MASK 0x001f0000l
#define BM_ADDR_MASK   0x1fffffffl

/* GXA Configuration Register */
#define ROP_FIELD             0x0000000Fl
#define MONO_FLIP             0x00000100l
#define TRANSP_ENABLE         0x00001000l
#define ENABLE_SCAN_COUNT     0x02000000l
#define PRESERVE_ALPHA        0x00080000l
#define FORCE_PAT_RELOAD      0x08000000l
#define INSIDE_OUT_ALPHA      0x01000000l
#define ALPHA_STORE_DST       0x00000000l
#define ALPHA_STORE_SRC       0x00400000l
#define ALPHA_STORE_FIXED     0x00800000l
#define USE_DST_ALPHA         0x00000000l
#define USE_SRC_ALPHA         0x00100000l
#define USE_FIXED_ALPHA       0x00200000l
#define USE_ALPHA_IN_TRANSP   0x00004000l


/* GXA Config2 32 bit access */
#define GXA_STAT_QMARK        0x00200000l
#define GXA_STAT_HSXERR       0x00100000l
#define GXA_EN_QMARK_IRQ      0x00020000l
#define GXA_EN_HSXERR_IRQ     0x00010000l
#define GXA_EN_RFF_BUSY       0x00000100l

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

#endif /* INCL_GXA */

/**************************/
/* Display Refresh Module */
/**************************/
#ifdef INCL_DRM

/* Last updated 12/6/2002 */

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

/*******************************************************************************
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
 *      DRM_SCALING_COEFF_REG                                 (DRM_BASE + 0x09C) 
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
 *      DRM_VID_CURRENT_LUMA_ADDR                             (DRM_BASE + 0x140) 
 *      DRM_VID_CURRENT_CHROMA_ADDR                           (DRM_BASE + 0x144) 
 *      DRM_VID_CURRENT_LUMA_2_ADDR                           (DRM_BASE + 0x148) 
 *      DRM_VID_CURRENT_CHROMA_2_ADDR                         (DRM_BASE + 0x14C) 
 *      DRM_SAR_REG                                           (DRM_BASE + 0x160)
 *
 *******************************************************************************/

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
#define DRM_VID_CURRENT_LUMA_ADDR                    ((LPREG)(DRM_BASE + 0x140))
#define DRM_VID_CURRENT_CHROMA_ADDR                  ((LPREG)(DRM_BASE + 0x144))
#define DRM_VID_CURRENT_LUMA_2_ADDR                  ((LPREG)(DRM_BASE + 0x148))
#define DRM_VID_CURRENT_CHROMA_2_ADDR                ((LPREG)(DRM_BASE + 0x14C))

#define DRM_SAR_REG                                  ((LPREG)(DRM_BASE + 0x160))

/* Red/Green/Blue masks and shifts for DRM palette entries */
#define     DRMRGBPAL_R_MASK                                       0x000000FF
#define     DRMRGBPAL_R_SHIFT                                             0
#define     DRMRGBPAL_G_MASK                                       0x0000FF00
#define     DRMRGBPAL_G_SHIFT                                             8
#define     DRMRGBPAL_B_MASK                                       0x00FF0000
#define     DRMRGBPAL_B_SHIFT                                            16
#define     DRMRGBPAL_ALPHA_MASK                                   0xFF000000
#define     DRMRGBPAL_ALPHA_SHIFT                                        24

/* Y/Cb/Cr masks and shifts for DRM palette entries */
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

#define DRM_SCALING_COEFF_REG                        ((LPREG)(DRM_BASE + 0x09C))
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

#define DRM_DWI_CONTROL_1_REG                        ((LPREG)(DRM_BASE + 0x018))
#define     DRM_DWI_CONTROL_1_MEM_WORD_GROUPS_MASK                 0x0000000F
#define     DRM_DWI_CONTROL_1_MEM_WORD_GROUPS_SHIFT                       0
#define     DRM_DWI_CONTROL_1_PAL_WORD_GROUPS_MASK                 0x000000F0
#define     DRM_DWI_CONTROL_1_PAL_WORD_GROUPS_SHIFT                       4
#define     DRM_DWI_CONTROL_1_PAL_REQ_ENABLE_MASK                  0x00000100
#define     DRM_DWI_CONTROL_1_PAL_REQ_ENABLE_SHIFT                        8
#define     DRM_DWI_CONTROL_1_RESERVED1_MASK                       0x0000FE00
#define     DRM_DWI_CONTROL_1_RESERVED1_SHIFT                             9
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
#define     DRM_DWI_CONTROL_2_SPLIT_FIFO_LUMA_MASK                 0x40000000
#define     DRM_DWI_CONTROL_2_SPLIT_FIFO_LUMA_SHIFT                      30
#define     DRM_DWI_CONTROL_2_SPLIT_FIFO_CHROMA_MASK               0x80000000
#define     DRM_DWI_CONTROL_2_SPLIT_FIFO_CHROMA_SHIFT                    31

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
#define     DRM_MPG_TILE_WIPE_CTRL_FETCH_REM_MASK                  0x00E00000
#define     DRM_MPG_TILE_WIPE_CTRL_FETCH_REM_SHIFT                       21
#define     DRM_MPG_TILE_WIPE_CTRL_TILE_FACTOR_MASK                0x0F000000
#define     DRM_MPG_TILE_WIPE_CTRL_TILE_FACTOR_SHIFT                     24
#define     DRM_MPG_TILE_WIPE_CTRL_TILE_Y_ENABLE_MASK              0x10000000
#define     DRM_MPG_TILE_WIPE_CTRL_TILE_Y_ENABLE_SHIFT                   28
#define     DRM_MPG_TILE_WIPE_CTRL_WIPE_ENABLE_MASK                0x20000000
#define     DRM_MPG_TILE_WIPE_CTRL_WIPE_ENABLE_SHIFT                     29
#define     DRM_MPG_TILE_WIPE_CTRL_TILE_X_ENABLE_MASK              0x40000000
#define     DRM_MPG_TILE_WIPE_CTRL_TILE_X_ENABLE_SHIFT                   30
#define     DRM_MPG_TILE_WIPE_CTRL_RESERVED3_MASK                  0x80000000
#define     DRM_MPG_TILE_WIPE_CTRL_RESERVED3_SHIFT                       31

#define DRM_VIDEO_ALPHA_REG                          ((LPREG)(DRM_BASE + 0x12C))
#define     DRM_VIDEO_ALPHA_RESERVED_MASK                          0x0000FFFF
#define     DRM_VIDEO_ALPHA_RESERVED_SHIFT                                0
#define     DRM_VIDEO_ALPHA_VIDEO1_ALPHA_MASK                      0x00FF0000
#define     DRM_VIDEO_ALPHA_VIDEO1_ALPHA_SHIFT                           16
#define     DRM_VIDEO_ALPHA_VIDEO0_ALPHA_MASK                      0xFF000000
#define     DRM_VIDEO_ALPHA_VIDEO0_ALPHA_SHIFT                           24

#define DRM_SATURATION_VALUES_REG                    ((LPREG)(DRM_BASE + 0x130))
#define     DRM_SATURATION_LOW_Y_MASK                              0x000000FF
#define     DRM_SATURATION_LOW_Y_SHIFT                                    0
#define     DRM_SATURATION_HIGH_Y_MASK                             0x0000FF00
#define     DRM_SATURATION_HIGH_Y_SHIFT                                   8
#define     DRM_SATURATION_LOW_C_MASK                              0x00FF0000
#define     DRM_SATURATION_LOW_C_SHIFT                                   16
#define     DRM_SATURATION_HIGH_C_MASK                             0xFF000000
#define     DRM_SATURATION_HIGH_C_SHIFT                                  24

/* General DRM definitions */
#define DRM_FIELD_1 0
#define DRM_FIELD_2 1

typedef HW_DWORD DRMRGBPAL;
typedef HW_DWORD DRMYCCPAL;

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
#define     ENC_CTL0_ALL_DAC_MASK                                  0x003F0000
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
#define     ENC_CTL0_DAC_E_ENABLE_MASK                             0x00100000
#define     ENC_CTL0_DAC_E_ENABLE_SHIFT                                  20
#define         ENC_CTL0_DAC_E_ENABLE                               (1UL<<20)
#define     ENC_CTL0_DAC_F_ENABLE_MASK                             0x00200000
#define     ENC_CTL0_DAC_F_ENABLE_SHIFT                                  21
#define         ENC_CTL0_DAC_F_ENABLE                               (1UL<<21)
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
#define ENC_YUV_MULTIPLICATION_MASK                          0xffffffff
#define ENC_YUV_MULTIPLICATION_SHIFT                              0
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

#endif /* iNCL_ENC */

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
 *      AUD_PLAY_TEST_REG                                      (AUD_BASE + 0x80)
 *      AUD_CAPT_TEST_REG                                      (AUD_BASE + 0x84)
 *      AUD_ATTEN_OV_TEST_REG                                  (AUD_BASE + 0x88)
 *      AUD_ATTEN_DATA_TEST_REG                                (AUD_BASE + 0x8c)
 *******************************************************************************
 */

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
#define     AUD_FRAME_PROTOCOL_RESERVED1_MASK                      0xFFFFFC00
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
#define     AUD_MPEG_CAPT_ATTEN_COEFFICIENT_MASK                   0x00000F00
#define     AUD_MPEG_CAPT_ATTEN_COEFFICIENT_SHIFT                         8
#define     AUD_MPEG_CAPT_ATTEN_SHIFT_MASK                         0x0000F000
#define     AUD_MPEG_CAPT_ATTEN_SHIFT_SHIFT                              12 
#define     AUD_RFMOD_ATTEN_COEFFICIENT_MASK                       0x000F0000
#define     AUD_RFMOD_ATTEN_COEFFICIENT_SHIFT                            16
#define     AUD_RFMOD_ATTEN_SHIFT_MASK                             0x00F00000
#define     AUD_RFMOD_ATTEN_SHIFT_SHIFT                                  20
#define     AUD_ATTEN_RESERVED1_MASK                               0xFF000000
#define     AUD_ATTEN_RESERVED1_SHIFT                                    24 

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

#if (CPU_TYPE!=CPU_ARM940T)

#define AUD_CHAN_STAT_HEADER_REG                               (AUD_BASE + 0x5C)
#define     AUD_CHAN_STAT_HEADER_CONSUMER_MASK                     0x00000001
#define     AUD_CHAN_STAT_HEADER_CONSUMER_SHIFT                           0
#define     AUD_CHAN_STAT_HEADER_AUDIO_FLAG_MASK                   0x00000002
#define     AUD_CHAN_STAT_HEADER_AUDIO_FLAG_SHIFT                         1
#define     AUD_CHAN_STAT_HEADER_COPYRIGHT_MASK                    0x00000004
#define     AUD_CHAN_STAT_HEADER_COPYRIGHT_SHIFT                          2
#define     AUD_CHAN_STAT_HEADER_EMPHASIS_MASK                     0x00000038
#define     AUD_CHAN_STAT_HEADER_EMPHASIS_SHIFT                           3
#define     AUD_CHAN_STAT_HEADER_MODE_MASK                         0x000000C0
#define     AUD_CHAN_STAT_HEADER_MODE_SHIFT                               5
#define     AUD_CHAN_STAT_HEADER_MODE00_CATEGORY_CODE_MASK         0x00007F00
#define     AUD_CHAN_STAT_HEADER_MODE00_CATEGORY_CODE_SHIFT               7
#define     AUD_CHAN_STAT_HEADER_MODE00_GENERATION_STAT_MASK       0x00008000
#define     AUD_CHAN_STAT_HEADER_MODE00_GENERATION_STAT_SHIFT             15
#define     AUD_CHAN_STAT_HEADER_MODE00_SOURCE_NUM_MASK            0x000F0000
#define     AUD_CHAN_STAT_HEADER_MODE00_SOURCE_NUM_SHIFT                  16
#define     AUD_CHAN_STAT_HEADER_MODE00_CHANNEL_NUM_MASK           0x00F00000
#define     AUD_CHAN_STAT_HEADER_MODE00_CHANNEL_NUM_SHIFT                 20
#define     AUD_CHAN_STAT_HEADER_MODE00_SAMPLE_FREQUENCY_MASK      0x00007F00
#define     AUD_CHAN_STAT_HEADER_MODE00_SAMPLE_FREQUENCY_SHIFT            24
#define     AUD_CHAN_STAT_HEADER_MODE00_CLK_ACCURACY_MASK          0x30000000
#define     AUD_CHAN_STAT_HEADER_MODE00_CLK_ACCURACY_SHIFT                28
#define     AUD_CHAN_STAT_HEADER_RESERVED1_MASK                    0xC0000000
#define     AUD_CHAN_STAT_HEADER_RESERVED1_SHIFT                          30

#endif /* (CPU_TYPE!=CPU_ARM940T) */

#define AUD_PLAY_TEST_REG                                      (AUD_BASE + 0x80)
#define AUD_CAPT_TEST_REG                                      (AUD_BASE + 0x84)
#define AUD_ATTEN_OV_TEST_REG                                  (AUD_BASE + 0x88)
#define AUD_ATTEN_DATA_TEST_REG                                (AUD_BASE + 0x8c)

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
 *      PLL_PIN_GPIO_MUX3_REG                                  (PLL_BASE + 0x5C)  
 *      PLL_CLK_TREE_SEL_REG                                   (PLL_BASE + 0x64)  
 *      PLL_PRESCALE_REG                                       (PLL_BASE + 0x68)
 *      PLL_TEST_REG                                           (PLL_BASE + 0x70)  
 *      PLL_PAD_FAST_CTRL_REG                                  (PLL_BASE + 0x74)  
 *      PLL_DIG_ASPL_CTRL_REG                                  (PLL_BASE + 0x80)  
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
#define     PLL_CPU_CONFIG_FRAC_MASK                               0x01FFFFFF
#define     PLL_CPU_CONFIG_FRAC_SHIFT                                     0
#define     PLL_CPU_CONFIG_INT_MASK                                0x7E000000
#define     PLL_CPU_CONFIG_INT_SHIFT                                     25

#define PLL_MEM_CONFIG_REG                                     (PLL_BASE + 0x0C)
#define     PLL_MEM_CONFIG_FRAC_MASK                               0x01FFFFFF
#define     PLL_MEM_CONFIG_FRAC_SHIFT                                     0
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

/* Referred to as PLL_DIV0_REG in the spec */
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

/* Referred to as PLL_DIV1_REG in the spec */
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
#define     PLL_DIV1_HSDP_CLK_MASK                                 0x03F00000
#define     PLL_DIV1_HSDP_CLK_SHIFT                                      20

#define PLL_CONFIG0_REG                                        (PLL_BASE + 0x20)
#define     PLL_CONFIG0_SW_CONFIG_MASK                             0x00000001
#define     PLL_CONFIG0_SW_CONFIG_SHIFT                                   0
#define     PLL_CONFIG0_SMARTCARD_RESET_MASK                       0x00000008 
#define     PLL_CONFIG0_SMARTCARD_RESET_SHIFT                             3
#define         PLL_CONFIG0_SMARTCARD_RESET_ENABLE                   (0UL<<3)
#define         PLL_CONFIG0_SMARTCARD_RESET_DISABLE                  (1UL<<3)
#define     PLL_CONFIG0_DEMOD_STANDALONE_TEST_MASK                 0x00000100 
#define     PLL_CONFIG0_DEMOD_STANDALONE_TEST_SHIFT                       8
#define         PLL_CONFIG0_DEMOD_STANDALONE_TEST_ENABLE             (0UL<<8)
#define         PLL_CONFIG0_DEMOD_STANDALONE_TEST_DISABLE            (1UL<<8)
#define     PLL_CONFIG0_IO_ADDR_WIDTH_MASK                         0x00001800 
#define     PLL_CONFIG0_IO_ADDR_WIDTH_SHIFT                              11
#define         PLL_CONFIG0_IO_ADDR_WIDTH_20_BITS                   (1UL<<11)
#define         PLL_CONFIG0_IO_ADDR_WIDTH_21_BITS                   (2UL<<11)
#define         PLL_CONFIG0_IO_ADDR_WIDTH_22_BITS                   (3UL<<11)
/* NB: This is intended to overlap the previous definition           */
/*     The meaning of buts 11:12 is determined by the bit 16 setting */
#define     PLL_CONFIG0_PCI_ARM_RESET_MODE_MASK                    0x00000800 
#define     PLL_CONFIG0_PCI_ARM_RESET_MODE_SHIFT                         11
#define         PLL_CONFIG0_PCI_ARM_RESET_HOLD                      (0UL<<11)
#define         PLL_CONFIG0_PCI_ARM_RESET_NORMAL                    (1UL<<11)
#define     PLL_CONFIG0_IO_BUS_MODE_MASK                           0x00010000 
#define     PLL_CONFIG0_IO_BUS_MODE_SHIFT                                16
#define         PLL_CONFIG0_IO_BUS_MODE_PCI                         (0UL<<16)
#define         PLL_CONFIG0_IO_BUS_MODE_STANDARD                    (1UL<<16)
#define     PLL_CONFIG0_PCI_HOSTBRIDGE_ENABLE_MASK                 0x00800000
#define     PLL_CONFIG0_PCI_HOSTBRIDGE_ENABLE_SHIFT                      23
#define         PLL_CONFIG0_PCI_HOSTBRIDGE_ENABLE_PCI_DEVICE        (0UL<<23)
#define         PLL_CONFIG0_PCI_HOSTBRIDGE_ENABLE_PCI_HOST          (1UL<<23)
#define     PLL_CONFIG0_ROM_WIDTH_MASK                             0x08000000
#define     PLL_CONFIG0_ROM_WIDTH_SHIFT                                  27
#define         PLL_CONFIG0_ROM_WIDTH_8_BIT                         (0UL<<27)
#define         PLL_CONFIG0_ROM_WIDTH_16_BIT                        (1UL<<27)
#define     PLL_CONFIG0_SMART_0_PCA_SELECT_MASK                    0x10000000
#define     PLL_CONFIG0_SMART_0_PCA_SELECT_SHIFT                         28
#define         PLL_CONFIG0_SMART_0_PCA                             (0UL<<28)
#define         PLL_CONFIG0_SMART_0_INTERNAL                        (1UL<<28)
#define     PLL_CONFIG0_SMART_1_8004_SELECT_MASK                   0x40000000
#define     PLL_CONFIG0_SMART_1_8004_SELECT_SHIFT                        30
#define         PLL_CONFIG0_SMART_1_USE_8004                        (0UL<<30)
#define         PLL_CONFIG0_SMART_1_NO_8004                         (1UL<<30)
#define     PLL_CONFIG0_PLL_BYPASS_MASK                            0x80000000
#define     PLL_CONFIG0_PLL_BYPASS_SHIFT                                 31 
#define         PLL_CONFIG0_PLL_BYPASS_USE_EXTERNAL_CLK             (0UL<<31)
#define         PLL_CONFIG0_PLL_BYPASS_USE_PLLS                     (1UL<<31)

#define PLL_DAA_DIV_REG                                        (PLL_BASE + 0x28)
#define     PLL_DAA_DIV_D4CLK_LOW_DIV_MASK                         0x0000000F
#define     PLL_DAA_DIV_D4CLK_LOW_DIV_SHIFT                               0
#define     PLL_DAA_DIV_D4CLK_HIGH_DIV_MASK                        0x000000F0
#define     PLL_DAA_DIV_D4CLK_HIGH_DIV_SHIFT                              4
#define     PLL_DAA_DIV_DHCLK_LOW_DIV_MASK                         0x00000F00
#define     PLL_DAA_DIV_DHCLK_LOW_DIV_SHIFT                               8
#define     PLL_DAA_DIV_DHCLK_HIGH_DIV_MASK                        0x0000F000
#define     PLL_DAA_DIV_DHCLK_HIGH_DIV_SHIFT                             12
/* To be completed. Appears to have been removed on Brazos                       */
/* #define     PLL_DAA_DIV_USE_EXT_CLK_MASK                           0x00010000 */
/* #define     PLL_DAA_DIV_USE_EXT_CLK_SHIFT                                16   */
/* #define         PLL_DAA_DIV_USE_EXT_CLK                             (1UL<<16) */

#define PLL_USB_CONFIG_REG                                     (PLL_BASE + 0x2C)
#define     PLL_USB_CONFIG_FRAC_MASK                               0x01FFFFFF
#define     PLL_USB_CONFIG_FRAC_SHIFT                                     0
#define     PLL_USB_CONFIG_INT_MASK                                0x7E000000
#define     PLL_USB_CONFIG_INT_SHIFT                                     25

#define PLL_MEM_DELAY_REG                                      (PLL_BASE + 0x30)
#define     PLL_MEM_DELAY_WRITE_DELAY_MASK                         0x00000007
#define     PLL_MEM_DELAY_WRITE_DELAY_SHIFT                               0
#define     PLL_MEM_DELAY_READ_DELAY_MASK                          0x00000038
#define     PLL_MEM_DELAY_READ_DELAY_SHIFT                                3
#define     PLL_MEM_DELAY_PAW_CLK_DELAY_MASK                       0x000003C0
#define     PLL_MEM_DELAY_PAW_CLK_DELAY_SHIFT                             6
#define     PLL_MEM_DELAY_PCI_CLK_DELAY_MASK                       0xFF000000
#define     PLL_MEM_DELAY_PCI_CLK_DELAY_SHIFT                            24

#define PLL_PIN_ALT_FUNC_REG                                   (PLL_BASE + 0x34)
#define     PLL_PIN_ALT_FUNC_AFE_DAA_SEL_MASK                      0x00000001
#define     PLL_PIN_ALT_FUNC_AFE_DAA_SEL_SHIFT                            0
#define         PLL_PIN_ALT_FUNC_DAA_SEL                             (1UL<<0)
#define         PLL_PIN_ALT_FUNC_AFE_SEL                             (0UL<<0)
#define     PLL_PIN_ALT_FUNC_DAA_SPKR_SEL_MASK                     0x00000002
#define     PLL_PIN_ALT_FUNC_DAA_SPKR_SEL_SHIFT                           1
#define         PLL_PIN_ALT_FUNC_DAA_SPKR_ENABLE                     (1UL<<1)
#define         PLL_PIN_ALT_FUNC_DAA_SPKR_DISABLE                    (0UL<<1)
#define     PLL_PIN_ALT_FUNC_SMART_MDP_SCM_MASK                    0x00000004
#define     PLL_PIN_ALT_FUNC_SMART_MDP_SCM_SHIFT                          2
#define         PLL_PIN_ALT_FUNC_SMART_MDP_SCM_OUTPUT_DISABLE        (0UL<<2)
#define         PLL_PIN_ALT_FUNC_SMART_MDP_SCM_OUTPUT_ENABLE         (1UL<<2)
#define     PLL_PIN_ALT_FUNC_USB_CLK_UCLK0_MASK                    0x00000008
#define     PLL_PIN_ALT_FUNC_USB_CLK_UCKL0_SHIFT                          3
#define         PLL_PIN_ALT_FUNC_USB_CLK_UCKL0_DISABLE               (1UL<<3)
#define         PLL_PIN_ALT_FUNC_USB_CLK_UCLK0_ENABLE                (0UL<<3)
#define     PLL_PIN_ALT_FUNC_UART2_SEL_MASK                        0x00000030
#define     PLL_PIN_ALT_FUNC_UART2_SEL_SHIFT                              4
#define         PLL_PIN_ALT_FUNC_UART2_PIO_04_03_PINS                (0UL<<4)
#define         PLL_PIN_ALT_FUNC_UART2_PIO_71_72_PINS                (1UL<<4)
#define         PLL_PIN_ALT_FUNC_UART2_PIO_11_73_PINS                (2UL<<4)
#define         PLL_PIN_ALT_FUNC_UART2_DISABLED                      (3UL<<4)
#define     PLL_PIN_ALT_FUNC_ENC_DBG_ENABLE_MASK                   0x00002000
#define     PLL_PIN_ALT_FUNC_ENC_DBG_ENABLE_SHIFT                        13
#define         PLL_PIN_ALT_FUNC_ENC_DBG_DISABLE                    (0UL<<13)
#define         PLL_PIN_ALT_FUNC_ENC_DBG_ENABLE                     (1UL<<13)
#define     PLL_PIN_ALT_FUNC_ENC_EXT_MASTER_SEL_MASK               0x00004000
#define     PLL_PIN_ALT_FUNC_ENC_EXT_MASTER_SEL_SHIFT                    14
#define         PLL_PIN_ALT_FUNC_ENC_EXT_MASTER_DISABLE             (0UL<<14)
#define         PLL_PIN_ALT_FUNC_ENC_EXT_MASTER_ENABLE              (1UL<<14)
#define     PLL_PIN_ALT_FUNC_HDA03_R04_SEL_MASK                    0x00008000
#define     PLL_PIN_ALT_FUNC_HDA03_R04_SEL_SHIFT                         15
#define         PLL_PIN_ALT_FUNC_HDA03_R04_DISABLE                  (0UL<<15)
#define         PLL_PIN_ALT_FUNC_HDA03_R04_ENABLE                   (1UL<<15)
#define     PLL_PIN_ALT_FUNC_HDA43_R62_SEL_MASK                    0x00010000
#define     PLL_PIN_ALT_FUNC_HDA43_R62_SEL_SHIFT                         16
#define         PLL_PIN_ALT_FUNC_HDA43_R62_DISABLE                  (0UL<<16)
#define         PLL_PIN_ALT_FUNC_HDA43_R62_ENABLE                   (1UL<<16)
#define     PLL_PIN_ALT_FUNC_EXT_DEMOD_PIN_MASK                    0x00080000
#define     PLL_PIN_ALT_FUNC_EXT_DEMOD_PIN_SHIFT                         19
#define         PLL_PIN_ALT_FUNC_EXT_DEMOD_PIO_73_11                (0UL<<19)
#define         PLL_PIN_ALT_FUNC_EXT_DEMOD_PIO_72_71                (1UL<<19)
#define     PLL_PIN_ALT_FUNC_EXT_DEMOD_MASK                        0x00100000
#define     PLL_PIN_ALT_FUNC_EXT_DEMOD_SHIFT                             20
#define         PLL_PIN_ALT_FUNC_EXT_DEMOD_DISABLE                  (0UL<<20)
#define         PLL_PIN_ALT_FUNC_EXT_DEMOD_ENABLE                   (1UL<<20)
#define     PLL_PIN_ALT_FUNC_INT_DEMOD_OUTPUT_MASK                 0x00200000
#define     PLL_PIN_ALT_FUNC_INT_DEMOD_OUTPUT_SHIFT                      21
#define         PLL_PIN_ALT_FUNC_INT_DEMOD_NORMAL                   (0UL<<21)
#define         PLL_PIN_ALT_FUNC_INT_DEMOD_TO_HSDP1                 (1UL<<21)
#define     PLL_PIN_ALT_FUNC_IO_RW_MASK                            0x10000000
#define     PLL_PIN_ALT_FUNC_IO_RW_SHIFT                                 28
#define         PLL_PIN_ALT_FUNC_IO_RW_NORMAL                       (0UL<<28)
#define         PLL_PIN_ALT_FUNC_IO_RW_PIO_54_53                    (1UL<<28)
#define     PLL_PIN_ALT_FUNC_PCI_REQGNT1_MASK                      0x20000000
#define     PLL_PIN_ALT_FUNC_PCI_REQGNT1_SHIFT                           29
#define         PLL_PIN_ALT_FUNC_PCI_REQGNT1_NORMAL                 (0UL<<29)
#define         PLL_PIN_ALT_FUNC_PCI_REQGNT1_PIO_53_54              (1UL<<29)
#define     PLL_PIN_ALT_FUNC_EXT_DEMOD_HSDP_MASK                   0x40000000
#define     PLL_PIN_ALT_FUNC_EXT_DEMOD_HSDP_SHIFT                        30
#define         PLL_PIN_ALT_FUNC_EXT_DEMOD_INPUT_TO_TSA             (0UL<<30)
#define         PLL_PIN_ALT_FUNC_EXT_DEMOD_OUTPUT_TO_TSC            (1UL<<30)


#define PLL_LOCK_CMD_REG                                       (PLL_BASE + 0x38)
#define     PLL_LOCK_CMD_MASK                                      0x000000FF
#define     PLL_LOCK_CMD_SHIFT                                            0
#define         PLL_UNLOCK_CMD1                                        0xF8
#define         PLL_UNLOCK_CMD2                                        0x2B
#define         PLL_LOCK_CMD_VALUE                                     0x00

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

/* NB: In some cases, pins on Brazos with the same function as a previous   */
/*     chip have been renamed. To prevent common code from breaking, labels */
/*     defining both the old and new names are included here.               */
#define PLL_PIN_GPIO_MUX0_REG                                  (PLL_BASE + 0x50)
#define         PLL_PIN_GPIO_MUX0_PIO_0                              (1UL<<0)
#define         PLL_PIN_GPIO_MUX0_PIO_1                              (1UL<<1)
#define         PLL_PIN_GPIO_MUX0_UART1TX                            (1UL<<1)
#define         PLL_PIN_GPIO_MUX0_PIO_2                              (1UL<<2)
#define         PLL_PIN_GPIO_MUX0_UART1RX                            (1UL<<2)
#define         PLL_PIN_GPIO_MUX0_PIO_3                              (1UL<<3)
#define         PLL_PIN_GPIO_MUX0_UART2TX                            (1UL<<3)
#define         PLL_PIN_GPIO_MUX0_UART2TX_3                          (1UL<<3)
#define         PLL_PIN_GPIO_MUX0_PIO_4                              (1UL<<4)
#define         PLL_PIN_GPIO_MUX0_UART2RX                            (1UL<<4)
#define         PLL_PIN_GPIO_MUX0_UART2RX_4                          (1UL<<4)
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
#define         PLL_PIN_GPIO_MUX0_PIO_15                            (1UL<<15)
#define         PLL_PIN_GPIO_MUX0_PIO_16                            (1UL<<16)
#define         PLL_PIN_GPIO_MUX0_PIO_17                            (1UL<<17)
#define         PLL_PIN_GPIO_MUX0_PIO_18                            (1UL<<18)
#define         PLL_PIN_GPIO_MUX0_MCLK_DCLK                         (1UL<<18)
#define         PLL_PIN_GPIO_MUX0_DAA_PWRCLK                        (1UL<<18)
#define         PLL_PIN_GPIO_MUX0_PIO_19                            (1UL<<19)
#define         PLL_PIN_GPIO_MUX0_MSTB_DCLKN                        (1UL<<19)
#define         PLL_PIN_GPIO_MUX0_DAA_PWRCLKN                       (1UL<<19)
#define         PLL_PIN_GPIO_MUX0_PIO_20                            (1UL<<20)
#define         PLL_PIN_GPIO_MUX0_PIO_21                            (1UL<<21)
#define         PLL_PIN_GPIO_MUX0_MRX_DIBP                          (1UL<<21)
#define         PLL_PIN_GPIO_MUX0_DAA_DATAP                         (1UL<<21)
#define         PLL_PIN_GPIO_MUX0_PIO_22                            (1UL<<22)
#define         PLL_PIN_GPIO_MUX0_MSCLK_DIBN                        (1UL<<22)
#define         PLL_PIN_GPIO_MUX0_DAA_DATAN                         (1UL<<22)
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
#define         PLL_PIN_GPIO_MUX2_ED0CLK                             (1UL<<9)
#define         PLL_PIN_GPIO_MUX2_PIO_74                            (1UL<<10)
#define         PLL_PIN_GPIO_MUX2_ED0ERR                            (1UL<<10)
#define         PLL_PIN_GPIO_MUX2_PIO_75                            (1UL<<11)
#define         PLL_PIN_GPIO_MUX2_ED0DVAL                           (1UL<<11)
#define         PLL_PIN_GPIO_MUX2_PIO_76                            (1UL<<12)
#define         PLL_PIN_GPIO_MUX2_PIO_77                            (1UL<<13)
#define         PLL_PIN_GPIO_MUX2_PIO_78                            (1UL<<14)
#define         PLL_PIN_GPIO_MUX2_PIO_79                            (1UL<<15)
#define         PLL_PIN_GPIO_MUX2_PIO_80                            (1UL<<16)
#define         PLL_PIN_GPIO_MUX2_PIO_81                            (1UL<<17)
#define         PLL_PIN_GPIO_MUX2_PIO_82                            (1UL<<18)
#define         PLL_PIN_GPIO_MUX2_PIO_83                            (1UL<<19)
#define         PLL_PIN_GPIO_MUX2_PIO_84                            (1UL<<20)
#define         PLL_PIN_GPIO_MUX2_PIO_85                            (1UL<<21)
#define         PLL_PIN_GPIO_MUX2_PIO_86                            (1UL<<22)
#define         PLL_PIN_GPIO_MUX2_PIO_87                            (1UL<<23)
#define         PLL_PIN_GPIO_MUX2_PIO_88                            (1UL<<24)
#define         PLL_PIN_GPIO_MUX2_PIO_89                            (1UL<<25)
#define         PLL_PIN_GPIO_MUX2_PIO_90                            (1UL<<26)
#define         PLL_PIN_GPIO_MUX2_PIO_91                            (1UL<<27)
#define         PLL_PIN_GPIO_MUX2_PIO_92                            (1UL<<28)
#define         PLL_PIN_GPIO_MUX2_PIO_93                            (1UL<<29)
#define         PLL_PIN_GPIO_MUX2_PIO_94                            (1UL<<30)
#define         PLL_PIN_GPIO_MUX2_PIO_95                            (1UL<<31)

#define PLL_PIN_GPIO_MUX3_REG                                  (PLL_BASE + 0x5C)
#define         PLL_PIN_GPIO_MUX3_PIO_96                            (1UL<<0)
#define         PLL_PIN_GPIO_MUX0_IR2_OUT                           (1UL<<0)
#define         PLL_PIN_GPIO_MUX3_PIO_97                            (1UL<<1)
#define         PLL_PIN_GPIO_MUX3_PIO_98                            (1UL<<2)
#define         PLL_PIN_GPIO_MUX3_PIO_99                            (1UL<<3)
#define         PLL_PIN_GPIO_MUX3_PIO_100                           (1UL<<4)
#define         PLL_PIN_GPIO_MUX3_PIO_101                           (1UL<<5)
#define         PLL_PIN_GPIO_MUX3_PIO_102                           (1UL<<6)
#define         PLL_PIN_GPIO_MUX3_PIO_103                           (1UL<<7)
#define         PLL_PIN_GPIO_MUX3_4S_RESETOUT                       (1UL<<7)
#define         PLL_PIN_GPIO_MUX3_PIO_104                           (1UL<<8)
#define         PLL_PIN_GPIO_MUX3_SCDA1                             (1UL<<8)
#define         PLL_PIN_GPIO_MUX3_PIO_105                           (1UL<<9)
#define         PLL_PIN_GPIO_MUX3_SCL1                              (1UL<<9)
#define         PLL_PIN_GPIO_MUX3_PIO_106                           (1UL<<10)
#define         PLL_PIN_GPIO_MUX3_XTO                               (1UL<<10)
#define         PLL_PIN_GPIO_MUX3_DSPKR                             (1UL<<10)
#define         PLL_PIN_GPIO_MUX3_PIO_107                           (1UL<<11)
#define         PLL_PIN_GPIO_MUX3_USBCLK                            (1UL<<11)
#define         PLL_PIN_GPIO_MUX3_PIO_108                           (1UL<<12)
#define         PLL_PIN_GPIO_MUX3_PIO_109                           (1UL<<13)
#define         PLL_PIN_GPIO_MUX3_PIO_110                           (1UL<<14)
#define         PLL_PIN_GPIO_MUX3_PIO_111                           (1UL<<15)
#define         PLL_PIN_GPIO_MUX3_PIO_112                           (1UL<<16)
#define         PLL_PIN_GPIO_MUX3_PIO_113                           (1UL<<17)
#define         PLL_PIN_GPIO_MUX3_PIO_114                           (1UL<<18)
#define         PLL_PIN_GPIO_MUX3_PIO_115                           (1UL<<19)
#define         PLL_PIN_GPIO_MUX3_PIO_116                           (1UL<<20)
#define         PLL_PIN_GPIO_MUX3_PIO_117                           (1UL<<21)
#define         PLL_PIN_GPIO_MUX3_PIO_118                           (1UL<<22)
#define         PLL_PIN_GPIO_MUX3_PIO_119                           (1UL<<23)
#define         PLL_PIN_GPIO_MUX3_PIO_120                           (1UL<<24)
#define         PLL_PIN_GPIO_MUX3_PIO_121                           (1UL<<25)
#define         PLL_PIN_GPIO_MUX3_PIO_122                           (1UL<<26)
#define         PLL_PIN_GPIO_MUX3_PIO_123                           (1UL<<27)
#define         PLL_PIN_GPIO_MUX3_PIO_124                           (1UL<<28)
#define         PLL_PIN_GPIO_MUX3_PIO_125                           (1UL<<29)
#define         PLL_PIN_GPIO_MUX3_PIO_126                           (1UL<<30)
#define         PLL_PIN_GPIO_MUX3_PIO_127                           (1UL<<31)

#define PLL_CLK_TREE_SEL_REG                                   (PLL_BASE + 0x64)
#define     PLL_CLK_TREE_SEL_PAWCLK_MASK                           0x00000003
#define     PLL_CLK_TREE_SEL_PAWCLK_SHIFT                                 0
#define         PLL_CLK_TREE_SEL_PAWCLK_MPG_PLL                      (0UL<<0)
#define         PLL_CLK_TREE_SEL_PAWCLK_MEM_PLL                      (1UL<<0)
#define         PLL_CLK_TREE_SEL_PAWCLK_ARM_PLL                      (2UL<<0)
#define         PLL_CLK_TREE_SEL_PAWCLK_USB_PLL                      (3UL<<0)
#define     PLL_CLK_TREE_SEL_MPGCLK_MASK                           0x00000030
#define     PLL_CLK_TREE_SEL_MPGCLK_SHIFT                                 4
#define         PLL_CLK_TREE_SEL_MPGCLK_MPG_PLL                      (0UL<<4)
#define         PLL_CLK_TREE_SEL_MPGCLK_MEM_PLL                      (1UL<<4)
#define         PLL_CLK_TREE_SEL_MPGCLK_ARM_PLL                      (2UL<<4)
#define         PLL_CLK_TREE_SEL_MPGCLK_USB_PLL                      (3UL<<4)
#define     PLL_CLK_TREE_SEL_MEMCLK_MASK                           0x00000300
#define     PLL_CLK_TREE_SEL_MEMCLK_SHIFT                                 8
#define         PLL_CLK_TREE_SEL_MEMCLK_MPG_PLL                      (0UL<<8)
#define         PLL_CLK_TREE_SEL_MEMCLK_MEM_PLL                      (1UL<<8)
#define         PLL_CLK_TREE_SEL_MEMCLK_ARM_PLL                      (2UL<<8)
#define         PLL_CLK_TREE_SEL_MEMCLK_USB_PLL                      (3UL<<8)
#define     PLL_CLK_TREE_SEL_PCICLK_MASK                           0x00003000
#define     PLL_CLK_TREE_SEL_PCICLK_SHIFT                                12
#define         PLL_CLK_TREE_SEL_PCICLK_MPG_PLL                      (0UL<<12)
#define         PLL_CLK_TREE_SEL_PCICLK_MEM_PLL                      (1UL<<12)
#define         PLL_CLK_TREE_SEL_PCICLK_ARM_PLL                      (2UL<<12)
#define         PLL_CLK_TREE_SEL_PCICLK_USB_PLL                      (3UL<<12)
#define     PLL_CLK_TREE_SEL_ARMCLK_MASK                           0x00030000
#define     PLL_CLK_TREE_SEL_ARMCLK_SHIFT                                16
#define         PLL_CLK_TREE_SEL_ARMCLK_MPG_PLL                      (0UL<<16)
#define         PLL_CLK_TREE_SEL_ARMCLK_MEM_PLL                      (1UL<<16)
#define         PLL_CLK_TREE_SEL_ARMCLK_ARM_PLL                      (2UL<<16)
#define         PLL_CLK_TREE_SEL_ARMCLK_USB_PLL                      (3UL<<16)
#define     PLL_CLK_TREE_SEL_AUDCLK_MASK                           0x00100000
#define     PLL_CLK_TREE_SEL_AUDCLK_SHIFT                                20
#define         PLL_CLK_TREE_SEL_AUDCLK_AUD_PLL                      (0UL<<20)
#define         PLL_CLK_TREE_SEL_AUDCLK_MPG_PLL                      (1UL<<20)
#define     PLL_CLK_TREE_SEL_USBCLK_MASK                           0x00200000
#define     PLL_CLK_TREE_SEL_USBCLK_SHIFT                                21
#define         PLL_CLK_TREE_SEL_USBCLK_USB_PLL                      (0UL<<21)
#define         PLL_CLK_TREE_SEL_USBCLK_MPG_PLL                      (1UL<<21)
#define     PLL_CLK_TREE_SEL_DAACLK_MASK                           0x00C00000
#define     PLL_CLK_TREE_SEL_DAACLK_SHIFT                                22
#define         PLL_CLK_TREE_SEL_DAACLK_MPG_PLL                      (0UL<<22)
#define         PLL_CLK_TREE_SEL_DAACLK_MEM_PLL                      (1UL<<22)
#define         PLL_CLK_TREE_SEL_DAACLK_ARM_PLL                      (2UL<<22)
#define         PLL_CLK_TREE_SEL_DAACLK_USB_PLL                      (3UL<<22)

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
#define         PLL_PRESCALE(val) (((val)==0)?2:(((val)==1)?1:(((val)==2)?4:3)))

/* CAUTION - The PLL_TEST_REG contains bits that can be read to determine if */
/* certain chip functions are enabled or disabled.  It also contains many    */
/* reserved bits and control bits for test functions.  You should never      */
/* write to this register.                                                   */
#define PLL_TEST_REG                                           (PLL_BASE + 0x70)  
#define     PLL_TEST_ALT_SYSTEM_DISABLE_MASK                       0x00010000
#define     PLL_TEST_ALT_SYSTEM_DISABLE_SHIFT                            16
#define         PLL_TEST_ALT_SYSTEM_ENABLED                         (0UL<<16)
#define         PLL_TEST_ALT_SYSTEM_DISABLED                        (1UL<<16)
#define     PLL_TEST_32BIT_RAM_DISABLE_MASK                        0x00020000
#define     PLL_TEST_32BIT_RAM_DISABLE_SHIFT                             17
#define         PLL_TEST_32BIT_RAM_ENABLED                          (0UL<<17)
#define         PLL_TEST_32BIT_RAM_DISABLED                         (1UL<<17)
#define     PLL_TEST_REMOD_DISABLE_MASK                            0x00040000
#define     PLL_TEST_REMOD_DISABLE_SHIFT                                 18
#define         PLL_TEST_REMOD_ENABLED                              (0UL<<18)
#define         PLL_TEST_REMOD_DISABLED                             (1UL<<18)
#define     PLL_TEST_DEMOD_DISABLE_MASK                            0x00080000
#define     PLL_TEST_DEMOD_DISABLE_SHIFT                                 19
#define         PLL_TEST_DEMOD_ENABLED                              (0UL<<19)
#define         PLL_TEST_DEMOD_DISABLED                             (1UL<<19)
#define     PLL_TEST_DOLBY_DISABLE_MASK                            0x00100000
#define     PLL_TEST_DOLBY_DISABLE_SHIFT                                 20
#define         PLL_TEST_DOLBY_ENABLED                              (0UL<<20)
#define         PLL_TEST_DOLBY_DISABLED                             (1UL<<20)
#define     PLL_TEST_NDS_DISABLE_MASK                              0x00200000
#define     PLL_TEST_NDS_DISABLE_SHIFT                                   21
#define         PLL_TEST_NDS_ENABLED                                (1UL<<21)
#define         PLL_TEST_NDS_DISABLED                               (0UL<<21)
#define     PLL_TEST_MACROVISION_DISABLE_MASK                      0x00400000
#define     PLL_TEST_MACROVISION_DISABLE_SHIFT                           22
#define         PLL_TEST_MACROVISION_ENABLED                        (1UL<<22)
#define         PLL_TEST_MACROVISION_DISABLED                       (0UL<<22)

#define PLL_PAD_FAST_CTRL_REG                                  (PLL_BASE + 0x74)
#define     PLL_FAST_CTRL_DEMOD_I2C_SEL_MASK                       0x00030000
#define     PLL_FAST_CTRL_DEMOD_I2C_SEL_SHIFT                            16
#define         PLL_FAST_CTRL_DEMOD_I2C_BUS_NONE                    (0UL<<16)
#define         PLL_FAST_CTRL_DEMOD_I2C_BUS_0                       (1UL<<16)
#define         PLL_FAST_CTRL_DEMOD_I2C_BUS_1                       (2UL<<16)
#define     PLL_FAST_CTRL_DEMOD_CONNECT_SEL_MASK                   0x00040000
#define     PLL_FAST_CTRL_DEMOD_CONNECT_SEL_SHIFT                        18
#define         PLL_FAST_CTRL_DEMOD_CONNECT_ASX                     (0UL<<18)
#define         PLL_FAST_CTRL_DEMOD_CONNECT_I2C                     (1UL<<18)
        
#define PLL_DIG_ASPL_CTRL_REG                                  (PLL_BASE + 0x80)
#define     PLL_DIG_ASPL_CTL_NUMERATOR_MASK                        0xFFFF0000
#define     PLL_DIG_ASPL_CTL_NUMERATOR_SHIFT                             16
#define     PLL_DIG_ASPL_CTL_DENOMINATOR_MASK                      0x0000FFFF
#define     PLL_DIG_ASPL_CTL_DENOMINATOR_SHIFT                            0

#define PLL_CLK_OBSERVATION_REG                                (PLL_BASE + 0x7C)
#define     PLL_CLK_OBSERVE_1_SEL_MASK                             0x0000000F
#define     PLL_CLK_OBSERVE_1_SEL_SHIFT                                   0
#define     PLL_CLK_OBSERVE_1_SEL_NONE                              (0UL<<0)
#define     PLL_CLK_OBSERVE_1_SEL_MPG                               (1UL<<0)
#define     PLL_CLK_OBSERVE_1_SEL_AUD                               (2UL<<0)
#define     PLL_CLK_OBSERVE_1_SEL_ARM                               (3UL<<0)
#define     PLL_CLK_OBSERVE_1_SEL_MEM                               (4UL<<0)
#define     PLL_CLK_OBSERVE_1_SEL_USB                               (5UL<<0)
#define     PLL_CLK_OBSERVE_2_SEL_MASK                             0x000000F0
#define     PLL_CLK_OBSERVE_2_SEL_SHIFT                                   4
#define     PLL_CLK_OBSERVE_2_SEL_NONE                              (0UL<<4)
#define     PLL_CLK_OBSERVE_2_SEL_MPG                               (1UL<<4)
#define     PLL_CLK_OBSERVE_2_SEL_AUD                               (2UL<<4)
#define     PLL_CLK_OBSERVE_2_SEL_ARM                               (3UL<<4)
#define     PLL_CLK_OBSERVE_2_SEL_MEM                               (4UL<<4)
#define     PLL_CLK_OBSERVE_2_SEL_USB                               (5UL<<4)
#define     PLL_CLK_OBSERVE_3_SEL_MASK                             0x00000F00
#define     PLL_CLK_OBSERVE_3_SEL_SHIFT                                   8
#define     PLL_CLK_OBSERVE_3_SEL_NONE                              (0UL<<8)
#define     PLL_CLK_OBSERVE_3_SEL_MPG                               (1UL<<8)
#define     PLL_CLK_OBSERVE_3_SEL_AUD                               (2UL<<8)
#define     PLL_CLK_OBSERVE_3_SEL_ARM                               (3UL<<8)
#define     PLL_CLK_OBSERVE_3_SEL_MEM                               (4UL<<8)
#define     PLL_CLK_OBSERVE_3_SEL_USB                               (5UL<<8)
#define     PLL_CLK_OBSERVE_3_DIV_MASK                             0x0000F000
#define     PLL_CLK_OBSERVE_3_DIV_SHIFT                                   12

#endif /* INCL_PLL */

/******************************/
/* Reset and Power Management */
/******************************/

#ifdef INCL_RST

/*******************************************************************************
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
 *      RST_RESET_RELEASE_REG                                  (RST_BASE + 0x28)
 *      RST_GRESET_REG                                         (RST_BASE + 0x2C) 
 *      RST_LPCONFIG_REG                                       (RST_BASE + 0x30)
 *      RST_LPCLOCK_REG                                        (RST_BASE + 0x34)
 *      RST_LPRESET_REG                                        (RST_BASE + 0x38)
 *      RST_LPTIMER_REG                                        (RST_BASE + 0x3C)
 *      RST_LOWPOWER_REG                                       (RST_BASE + 0x40)
 *      RST_SCRATCH2_REG                                       (RST_BASE + 0x50) 
 *      RST_SCRATCH3_REG                                       (RST_BASE + 0x54) 
 *      RST_SCRATCH4_REG                                       (RST_BASE + 0x58) 
 *      RST_SCRATCH5_REG                                       (RST_BASE + 0x5C) 
 *      RST_HSX_ARBITRATION_REG                                (RST_BASE + 0x60) 
 *      RST_HSX_ARBITER_PRI_REG                                (RST_BASE + 0x64) 
 *      RST_MEMORY_RESET_WAIT_REG                              (RST_BASE + 0x70) 
 *      RST_PUSHBTN_RESET_CTL_REG                              (RST_BASE + 0x74) 
 *      RST_PUSHBTN_RESET_TIMER_REG                            (RST_BASE + 0x78) 
 *      RST_SMARTCRD_RESET_CTL_REG                             (RST_BASE + 0x7C) 
 *      RST_PREFETCH_MONITOR_REG                               (RST_BASE + 0x80) 
 *      RST_HARDWARE_PAGETABLE_CTRL_REGION0_REG                (RST_BASE + 0xA0) 
 *      RST_HARDWARE_PAGETABLE_CTRL_REGION1_REG                (RST_BASE + 0xA4) 
 *      RST_HARDWARE_PAGETABLE_CTRL_REGION2_REG                (RST_BASE + 0xA8) 
 *      RST_HARDWARE_PAGETABLE_CTRL_REGION3_REG                (RST_BASE + 0xAC) 
 *      RST_HARDWARE_PAGETABLE_CTRL_REGION4_REG                (RST_BASE + 0xB0) 
 *      RST_HARDWARE_PAGETABLE_CTRL_REGION5_REG                (RST_BASE + 0xB4) 
 *      RST_HARDWARE_PAGETABLE_CTRL_REGION6_REG                (RST_BASE + 0xB8) 
 *      RST_HARDWARE_PAGETABLE_CTRL_REGION7_REG                (RST_BASE + 0xBC) 
 *      RST_HARDWARE_PAGETABLE_BASE                            (RST_BASE + 0x8000)
 *
 *******************************************************************************/

/* Last updated 12/6/02 */

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

#define RST_RESET_RELEASE_REG                                  (RST_BASE + 0x28)

#define RST_GRESET_REG                                         (RST_BASE + 0x2C) 
#define     RST_GRESET_PARSER_MASK                                 0x00000800    
#define     RST_GRESET_PARSER_SHIFT                                      11      
#define         RST_GRESET_PARSER                                    (1UL<<11)    
#define     RST_GRESET_GXA_MASK                                    0x00000400    
#define     RST_GRESET_GXA_SHIFT                                         10      
#define         RST_GRESET_GXA                                       (1UL<<10)    
#define     RST_GRESET_ASPL_MASK                                   0x00000100    
#define     RST_GRESET_ASPL_SHIFT                                         8      
#define         RST_GRESET_ASPL                                      (1UL<<8)    
#define     RST_GRESET_PCI_MASK                                    0x00000080    
#define     RST_GRESET_PCI_SHIFT                                          7      
#define         RST_GRESET_PCI                                       (1UL<<7)    
#define     RST_GRESET_MEM_MASK                                    0x00000040    
#define     RST_GRESET_MEM_SHIFT                                          6      
#define         RST_GRESET_MEM                                       (1UL<<6)    
#define     RST_GRESET_CLK_VIDEO_MASK                              0x00000020    
#define     RST_GRESET_CLK_VIDEO_SHIFT                                    5      
#define         RST_GRESET_CLK_VIDEO                                 (1UL<<5)    
#define     RST_GRESET_CLK27_MASK                                  0x00000010    
#define     RST_GRESET_CLK27_SHIFT                                        4      
#define         RST_GRESET_CLK27                                     (1UL<<4)    
#define     RST_GRESET_ASX_MASK                                    0x00000008    
#define     RST_GRESET_ASX_SHIFT                                          3      
#define         RST_GRESET_ASX                                       (1UL<<3)    
#define     RST_GRESET_EXT_MASK                                    0x00000004    
#define     RST_GRESET_EXT_SHIFT                                          2      
#define         RST_GRESET_EXT                                       (1UL<<2)    
#define     RST_GRESET_ARM_MASK                                    0x00000002    
#define     RST_GRESET_ARM_SHIFT                                          1      
#define         RST_GRESET_ARM                                       (1UL<<1)    

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
#define     RST_LOWPOWER_SCD_MASK                                  0x00600000
#define     RST_LOWPOWER_SCD_SHIFT                                       21
#define         RST_LOWPOWER_SCD                                    (3UL<<21)
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

#define RST_HSX_ARBITRATION_REG                                (RST_BASE + 0x60)
#define     RST_HSX_ARBITRATION_MODE_MASK                               0x00030000
#define     RST_HSX_ARBITRATION_MODE_SHIFT                                    16
#define         RST_HSX_ARBITRATION_NORMAL                               (0UL<<16)
#define         RST_HSX_ARBITRATION_SINGLE_HIGH                          (1UL<<16)
#define         RST_HSX_ARBITRATION_DOUBLE_HIGH                          (3UL<<16)
#define     RST_HSX_ARBITRATION_GXA_MASK                                0x00000200
#define     RST_HSX_ARBITRATION_GXA_SHIFT                                     9
#define         RST_HSX_ARBITRATION_NO_GXA                               (0UL<<9)
#define         RST_HSX_ARBITRATION_INCLUDE_GXA                          (1UL<<9)
#define     RST_HSX_ARBITRATION_SP0_MASK                                0x00000010
#define     RST_HSX_ARBITRATION_SP0_SHIFT                                     4
#define         RST_HSX_ARBITRATION_NO_SP0                               (0UL<<4)
#define         RST_HSX_ARBITRATION_INCLUDE_SP0                          (1UL<<4)
#define     RST_HSX_ARBITRATION_ATA_MASK                                0x00000008
#define     RST_HSX_ARBITRATION_ATA_SHIFT                                     3
#define         RST_HSX_ARBITRATION_NO_ATA                               (0UL<<3)
#define         RST_HSX_ARBITRATION_INCLUDE_ATA                          (1UL<<3)
#define     RST_HSX_ARBITRATION_DMA_MASK                                0x00000004
#define     RST_HSX_ARBITRATION_DMA_SHIFT                                     2
#define         RST_HSX_ARBITRATION_NO_DMA                               (0UL<<2)
#define         RST_HSX_ARBITRATION_INCLUDE_DMA                          (1UL<<2)
#define     RST_HSX_ARBITRATION_PIT_MASK                                0x00000002
#define     RST_HSX_ARBITRATION_PIT_SHIFT                                      1
#define         RST_HSX_ARBITRATION_NO_PIT                               (0UL<<1)
#define         RST_HSX_ARBITRATION_INCLUDE_PIT                          (1UL<<1)
#define     RST_HSX_ARBITRATION_PCI_MASK                                0x00000010
#define     RST_HSX_ARBITRATION_PCI_SHIFT                                      0
#define         RST_HSX_ARBITRATION_NO_PCI                               (0UL<<0)
#define         RST_HSX_ARBITRATION_INCLUDE_PCI                          (1UL<<0)

#define RST_HSX_ARBITER_PRI_REG                                (RST_BASE + 0x64)
#define     RST_HSX_ARBITER_HIGH_GXA_MASK                               0x02000000
#define     RST_HSX_ARBITER_HIGH_GXA_SHIFT                                    25
#define         RST_HSX_ARBITER_HIGH_GXA                                 (1UL<<25)
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

#define RST_MEMORY_RESET_WAIT_REG                              (RST_BASE + 0x70)
#define     RST_MEMORY_RESET_WAIT_MASK                                 0x0000FFFF
#define     RST_MEMORY_RESET_WAIT_SHIFT                                       0  

#define RST_PUSHBTN_RESET_CTL_REG                              (RST_BASE + 0x74)
#define     RST_PUSHBTN_RESET_CTL_DEBOUNCE_MASK                        0xFFFFFF00
#define     RST_PUSHBTN_RESET_CTL_DEBOUNCE_SHIFT                              8
#define     RST_PUSHBTN_RESET_CTL_POLARITY_MASK                        0x00000002
#define     RST_PUSHBTN_RESET_CTL_POLARITY_SHIFT                              1
#define         RST_PUSHBTN_RESET_CTL_ACTIVE_LOW                        (0UL<<1)
#define         RST_PUSHBTN_RESET_CTL_ACTIVE_HIGH                       (1UL<<1)
#define     RST_PUSHBTN_RESET_CTL_ENABLE_MASK                          0x00000001
#define     RST_PUSHBTN_RESET_CTL_ENABLE_SHIFT                                0
#define         RST_PUSHBTN_RESET_CTL_DISABLE                           (0UL<<0)
#define         RST_PUSHBTN_RESET_CTL_ENABLE                            (1UL<<0)

#define RST_PUSHBTN_RESET_TIMER_REG                            (RST_BASE + 0x78)
#define     RST_PUSHBTN_RESET_TIMER_MASK                               0x00FFFFFF
#define     RST_PUSHBTN_RESET_TIMER_SHIFT                                     0

#define RST_SMARTCRD_RESET_CTL_REG                             (RST_BASE + 0x7C)
#define     RST_SMARTCRD_RESET_1_REMOVE_MASK                           0x00020000
#define     RST_SMARTCRD_RESET_1_REMOVE_SHIFT                                17
#define     RST_SMARTCRD_RESET_1_INSERT_MASK                           0x00010000
#define     RST_SMARTCRD_RESET_1_INSERT_SHIFT                                16
#define     RST_SMARTCRD_RESET_0_REMOVE_MASK                           0x00000200
#define     RST_SMARTCRD_RESET_0_REMOVE_SHIFT                                 9
#define     RST_SMARTCRD_RESET_0_INSERT_MASK                           0x00000100
#define     RST_SMARTCRD_RESET_0_INSERT_SHIFT                                 8
#define     RST_SMARTCRD_RESET_ENABLE_MASK                             0x00000001
#define     RST_SMARTCRD_RESET_ENABLE_SHIFT                                   0

#define RST_PREFETCH_MONITOR_REG                               (RST_BASE + 0x80)
#define     RST_PREFETCH_MONITOR_BUFF0_MASK                            0x000000FF
#define     RST_PREFETCH_MONITOR_BUFF0_SHIFT                                  0
#define     RST_PREFETCH_MONITOR_BUFF1_MASK                            0x0000FF00
#define     RST_PREFETCH_MONITOR_BUFF1_SHIFT                                  8
#define     RST_PREFETCH_MONITOR_BUFF2_MASK                            0x00FF0000
#define     RST_PREFETCH_MONITOR_BUFF2_SHIFT                                 16
#define     RST_PREFETCH_MONITOR_BUFF3_MASK                            0xFF000000
#define     RST_PREFETCH_MONITOR_BUFF3_SHIFT                                 24

#define RST_HARDWARE_PAGETABLE_CTRL_REGION0_REG                (RST_BASE + 0xA0)
#define RST_HARDWARE_PAGETABLE_CTRL_REGION1_REG                (RST_BASE + 0xA4) 
#define RST_HARDWARE_PAGETABLE_CTRL_REGION2_REG                (RST_BASE + 0xA8) 
#define RST_HARDWARE_PAGETABLE_CTRL_REGION3_REG                (RST_BASE + 0xAC) 
#define RST_HARDWARE_PAGETABLE_CTRL_REGION4_REG                (RST_BASE + 0xB0) 
#define RST_HARDWARE_PAGETABLE_CTRL_REGION5_REG                (RST_BASE + 0xB4) 
#define RST_HARDWARE_PAGETABLE_CTRL_REGION6_REG                (RST_BASE + 0xB8) 
#define RST_HARDWARE_PAGETABLE_CTRL_REGION7_REG                (RST_BASE + 0xBC) 
#define     RST_HARDWARE_PAGETABLE_CTRL_BUFFERABLE_MASK                0x00000001
#define     RST_HARDWARE_PAGETABLE_CTRL_BUFFERABLE_SHIFT                      0
#define         RST_HARDWARE_PAGETABLE_CTRL_BUFFERABLE                   (1UL<<0)
#define     RST_HARDWARE_PAGETABLE_CTRL_CACHEABLE_MASK                 0x00000002
#define     RST_HARDWARE_PAGETABLE_CTRL_CACHEABLE_SHIFT                       1
#define         RST_HARDWARE_PAGETABLE_CTRL_CACHEABLE                    (1UL<<1)
#define     RST_HARDWARE_PAGETABLE_CTRL_PERMISSIONS_MASK               0x0000000C
#define     RST_HARDWARE_PAGETABLE_CTRL_PERMISSIONS_SHIFT                     2
#define         RST_HARDWARE_PAGETABLE_CTRL_PERMISSIONS_NONE             (0UL<<2)
#define         RST_HARDWARE_PAGETABLE_CTRL_PERMISSIONS_XXX1             (1UL<<2)
#define         RST_HARDWARE_PAGETABLE_CTRL_PERMISSIONS_XXX2             (2UL<<2)
#define         RST_HARDWARE_PAGETABLE_CTRL_PERMISSIONS_ALL              (3UL<<2)
#define     RST_HARDWARE_PAGETABLE_CTRL_SIZE_MASK                      0x0000FFF0
#define     RST_HARDWARE_PAGETABLE_CTRL_SIZE_SHIFT                            4
#define     RST_HARDWARE_PAGETABLE_CTRL_DOMAIN_MASK                    0x000F0000
#define     RST_HARDWARE_PAGETABLE_CTRL_DOMAIN_SHIFT                          16
#define     RST_HARDWARE_PAGETABLE_CTRL_ADDRESS_MASK                   0xFFF00000
#define     RST_HARDWARE_PAGETABLE_CTRL_ADDRESS_SHIFT                         20
#define RST_HARDWARE_PAGETABLE_BASE                            (RST_BASE + 0x8000)

#endif /* INCL_RST */

/************************/
/* Interrupt Controller */
/************************/

#ifdef INCL_ITC

/* Last updated 12/6/02 */

/******************************************************************************
 *
 *      ITC_INTDEST_REG                                  (ITC_BASE + 0x00)
 *      ITC_INTENABLE_REG                                (ITC_BASE + 0x04)
 *      ITC_INTRIRQ_REG                                  (ITC_BASE + 0x08)
 *      ITC_INTRFIQ_REG                                  (ITC_BASE + 0x0C)
 *      ITC_INTSTATCLR_REG                               (ITC_BASE + 0x10)
 *      ITC_INTSTATSET_REG                               (ITC_BASE + 0x14)
 *
 *****************************************************************************/

#define ITC_INTDEST_REG     (ITC_BASE + 0x00)
#define ITC_INTENABLE_REG   (ITC_BASE + 0x04)
#define ITC_INTRIRQ_REG     (ITC_BASE + 0x08)
#define ITC_INTRFIQ_REG     (ITC_BASE + 0x0C)
#define ITC_INTSTATCLR_REG  (ITC_BASE + 0x10)
#define ITC_INTSTATSET_REG  (ITC_BASE + 0x14)
#define ITC_EXPENABLE_REG   (ITC_BASE + 0x20)
#define ITC_EXPSTATCLR_REG  (ITC_BASE + 0x24)
#define ITC_EXPSTATSET_REG  (ITC_BASE + 0x28)

/* Interrupt status register */

#define ITC_EXPANSION      0x00000001
#define ITC_SYNCSERIAL     0x00000002
#define ITC_DRM            0x00000004
#define ITC_MPEG           0x00000008
#define ITC_RTC            0x00000010
#define ITC_RESERVED1      0x00000020 
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
#define ITC_ECY            0x00100000
#define ITC_PCI            0x00200000
#define ITC_PAR0           0x00400000
#define ITC_RESERVED3      0x00800000 
#define ITC_GXA            0x01000000
#define ITC_UART2          0x02000000
#define ITC_NDSC           0x04000000 
#define ITC_RESERVED4      0x08000000 
#define ITC_RESERVED5      0x10000000 
#define ITC_PWM            0x20000000
#define ITC_GPIO           0x40000000
#define ITC_TIMER          0x80000000

/* Bit positions for each interrupt source */
#define ITC_EXPANSION_POS        0
#define ITC_SYNCSERIAL_POS       1
#define ITC_DRM_POS              2
#define ITC_MPEG_POS             3
#define ITC_RTC_POS              4
#define ITC_RESERVED1_POS        5
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
#define ITC_RESERVED2_POS       19
#define ITC_ECY_POS             20
#define ITC_PCI_POS             21
#define ITC_PAR0_POS            22
#define ITC_RESERVED3_POS       23
#define ITC_GXA_POS             24
#define ITC_UART2_POS           25
#define ITC_NDSC_POS            26
#define ITC_RESERVED4_POS       27
#define ITC_RESERVED5_POS       28
#define ITC_PWM_POS             29
#define ITC_GPIO_POS            30
#define ITC_TIMER_POS           31

#define SYS_TIMER           0
#define WATCHDOG_TIMER      1

/* Expansion register sources and bit positions */
#define ITC_EXPANSION_CMTX     0x00000001
#define ITC_EXPANSION_CMRX     0x00000002
#define ITC_EXPANSION_I2C_0    0x00000004
#define ITC_EXPANSION_I2C_1    0x00000008
#define ITC_EXPANSION_DMD      0x00000010
#define ITC_EXPANSION_GIR2     0x00000020

#define ITC_EXPANSION_CMTX_POS     0
#define ITC_EXPANSION_CMRX_POS     1
#define ITC_EXPANSION_I2C_0_POS    2
#define ITC_EXPANSION_I2C_1_POS    3
#define ITC_EXPANSION_DMD_POS      4
#define ITC_EXPANSION_GIR2_POS     5

#endif /* INCL_ITC */

/**********/
/* Timers */
/**********/

#ifdef INCL_TIM

/* Last updated 3/17/99 */
/******************************************************************************
 *
 *     TIM_VALUE_REG                                  (TIM_BASE + 0x00)
 *     TIM_LIMIT_REG                                  (TIM_BASE + 0x04)
 *     TIM_MODE_REG                                   (TIM_BASE + 0x08)
 *     TIM_INT_STATUS_REG                             (TIM_BASE + 0x80)
 *
 *****************************************************************************/

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

/* Definitions allowing access to the mode register as a HW_DWORD */

#define TIMER_ENABLE   0x00000001
#define TIMER_DONTWRAP 0x00000002
#define TIMER_WATCHDOG 0x00000004

#endif /* INCL_TIM */

/*******/
/* I2C */
/*******/

#ifdef INCL_I2C

/******************************************************************************
 *
 *       CNXT_IIC_MODE_REG                            (I2C_BASE + 0x00)
 *       CNXT_IIC_CTRL_REG                            (I2C_BASE + 0x04)
 *       CNXT_IIC_STAT_REG                            (I2C_BASE + 0x08)
 *       CNXT_IIC_RDATA_REG                           (I2C_BASE + 0x0C)
 *
 *****************************************************************************/
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
#define     CNXT_IIC_MODE_BYTEORDER_LI_MASK              0x00000020
#define     CNXT_IIC_MODE_BYTEORDER_LI_SHIFT             5
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
#define DRIVE_GPIO_HIGH_BANK(b, x)   *(LPREG)(GPI_DRIVE_HIGH_REG + ((b)*GPI_BANK_SIZE)) = ((unsigned int)1<<(x))
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
#define     IRD_CONTROL_OVERFLOW_COUNT_DISABLE_MASK              0x00004000
#define     IRD_CONTROL_OVERFLOW_OUNT_DISABLE_SHIFT                    14
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

/* Macros allowing access to registers in different IRD instances */
#define IRD_CONTROL_REG_ADDR(iBank)          (IRD_CONTROL_REG        + (iBank)*IRD_BANK_SIZE)
#define IRD_TX_CLK_CONTROL_REG_ADDR(iBank)   (IRD_TX_CLK_CONTROL_REG + (iBank)*IRD_BANK_SIZE)
#define IRD_RX_CLK_CONTROL_REG_ADDR(iBank)   (IRD_RX_CLK_CONTROL_REG + (iBank)*IRD_BANK_SIZE)
#define IRD_TX_CARRIER_REG_ADDR(iBank)       (IRD_TX_CARRIER_REG     + (iBank)*IRD_BANK_SIZE)
#define IRD_STATUS_REG_ADDR(iBank)           (IRD_STATUS_REG         + (iBank)*IRD_BANK_SIZE)
#define IRD_INT_ENABLE_REG_ADDR(iBank)       (IRD_INT_ENABLE_REG     + (iBank)*IRD_BANK_SIZE)
#define IRD_LOWPASS_FILTER_REG_ADDR(iBank)   (IRD_LOWPASS_FILTER_REG + (iBank)*IRD_BANK_SIZE)
#define IRD_TEST_CONTROL_REG_ADDR(iBank)     (IRD_TEST_CONTROL_REG   + (iBank)*IRD_BANK_SIZE)
#define IRD_TEST_TXCLK_REG_ADDR(iBank)       (IRD_TEST_TXCLK_REG     + (iBank)*IRD_BANK_SIZE)
#define IRD_TEST_RXCLK_REG_ADDR(iBank)       (IRD_TEST_RXCLK_REG     + (iBank)*IRD_BANK_SIZE)
#define IRD_DATA_BASE_ADDR(iBank)            (IRD_DATA_BASE          + (iBank)*IRD_BANK_SIZE)

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
#define     SMC_PARITY_ENABLERX_DETECT_MASK                      0x00000010
#define     SMC_PARITY_ENABLERX_DETECT_SHIFT                            4
#define     SMC_PARITY_ENABLERX_REP_MASK                         0x00000020
#define     SMC_PARITY_ENABLERX_REP_SHIFT                               5
#define     SMC_PARITY_DISABLETX_GEN_MASK                        0x00000040
#define     SMC_PARITY_DISABLETX_GEN_SHIFT                              6
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
#define     MPG_CONTROL0_DOWNLOADAOC_MASK                          0x00000001
#define     MPG_CONTROL0_DOWNLOADAOC_SHIFT                                0
#define     MPG_CONTROL0_AOCCODEVALID_MASK                         0x00000002
#define     MPG_CONTROL0_AOCCODEVALID_SHIFT                               1
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
#define     MPG_CONTROL1_SPDIFISPCMIO_MASK                         0x08000000
#define     MPG_CONTROL1_SPDIFISPCMIO_SHIFT                              27
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
#define     MPG_ADDR_EXT_DECODE_TYPE_MASK                          0x00030000
#define     MPG_ADDR_EXT_DECODE_TYPE_SHIFT                               16
#define        MPG_ADDR_EXT_DECODE_TYPE_I                           (0 << 16)
#define        MPG_ADDR_EXT_DECODE_TYPE_P                           (1 << 16)
#define        MPG_ADDR_EXT_DECODE_TYPE_B                           (2 << 16)
#define MPG_AC3_DROP_CNT_REG                                  (MPG_BASE + 0x080)
#define MPG_AC3_PTS_HI_REG                                    (MPG_BASE + 0x084)
#define MPG_AC3_PTS_LO_REG                                    (MPG_BASE + 0x088)
#define MPG_OFFSET_HI_REG                                     (MPG_BASE + 0x08C)
#define MPG_OFFSET_LO_REG                                     (MPG_BASE + 0x090)
#define MPG_EARLY_SYNC_REG                                    (MPG_BASE + 0x094)  
#define MPG_EARLY_SYNC_MASK                                         0x01000000
#define MPG_EARLY_SYNC_SHIFT                                           24	


/* Definitions for LastDecoded field when accessed directly  */

/* NB: Last decoded field indication in MPG_ADDR_EXT_REG  indicates the */
/*     buffer that the last decoded image was written to NOT the actual */
/*     type of the image (since anchor pictures bounce between the 2    */
/*     anchor buffers regardless of whether they are I or P). To find   */
/*     the actual I, P, B type of the decoded image, look at the        */
/*     MPG_ADDR_EXT_DECODE_TYPE_x field instead.                        */

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
#define MPG_AOC_MC_VALID       MPG_CONTROL0_AOCCODEVALID_MASK
#define MPG_DOWN_LOAD_AOC_MC   MPG_CONTROL0_DOWNLOADAOC_MASK
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
  /* audio error bits are {[12:11], [5], [2:1]} */
  #define MPG_AUDIO_ERROR                     0x1826
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

#endif /* INCL_MPG */

/******************/
/* HSDP Registers */
/******************/

#ifdef INCL_HSDP

/*
 *******************************************************************************
 *
 *      HSDP_TSA_PORT_CNTL_REG                                 (HSDP_BASE + 0x00)
 *      HSDP_TSB_PORT_CNTL_REG                                 (HSDP_BASE + 0x04)
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

#define HSDP_TSB_PORT_CNTL_REG                                 (HSDP_BASE + 0x04)
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

#define NUM_PID_SLOTS                                                    32
#define DPS_NUM_DEMUXES                                            NUM_PARSERS

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

#define DPS_VERSION_MODES_REG_EX_0_BANK_01(instance)                           \
                              ((volatile HW_DWORD *)(DPS_BASE(instance)+0x0190))
#define DPS_VERSION_MODES_REG_EX_1_BANK_01(instance)                           \
                              ((volatile HW_DWORD *)(DPS_BASE(instance)+0x0194))
#define DPS_VERSION_MODES_REG_EX_0_BANK_02(instance)                           \
                              ((volatile HW_DWORD *)(DPS_BASE(instance)+0x0198))
#define DPS_VERSION_MODES_REG_EX_0_BANK_12(instance)                           \
                              ((volatile HW_DWORD *)(DPS_BASE(instance)+0x019C))
#define DPS_PID_BASE_EX(instance,slot)                                         \
                   ((volatile u_int32 *)(DPS_BASE(instance)+0x0080+((slot)<<2)))
#define DPS_FILTER_CONTROL_BASE_EX(instance,slot)                              \
                   ((volatile u_int32 *)(DPS_BASE(instance)+0x0100+((slot)<<2)))
#define DPS_EVEN_KEY_ENABLE_REG_EX(instance)                                   \
                              ((volatile HW_DWORD *)(DPS_BASE(instance)+0x0188))
#define DPS_EVEN_KEY0_ENABLE                                      (1UL<<0)
#define DPS_ODD_KEY_ENABLE_REG_EX(instance)                                    \
                              ((volatile HW_DWORD *)(DPS_BASE(instance)+0x018C))
#define DPS_ODD_KEY0_ENABLE                                       (1UL<<0)
#define DPS_KEY_TABLE_BASE_EX(instance,slot,index)                             \
      ((volatile u_int32 *)(DPS_BASE(instance)+0x01804000+((slot)<<4)+((index)<<2)))
#define DPS_TRANSPORT_WRITE_PTR_EX(instance)                                   \
                              ((volatile HW_DWORD *)(DPS_BASE(instance)+0x0200))
#define DPS_TRANSPORT_READ_PTR_EX(instance)                                    \
                              ((volatile HW_DWORD *)(DPS_BASE(instance)+0x0204))
#define DPS_TRANSPORT_START_ADDR_EX(instance)                                  \
                              ((volatile HW_DWORD *)(DPS_BASE(instance)+0x0208))
#define DPS_TRANSPORT_END_ADDR_EX(instance)                                    \
                              ((volatile HW_DWORD *)(DPS_BASE(instance)+0x020C))
#define DPS_REC_EVENT_WRITE_PTR_EX(instance)                                   \
                              ((volatile HW_DWORD *)(DPS_BASE(instance)+0x0210))
#define DPS_REC_EVENT_READ_PTR_EX(instance)                                    \
                              ((volatile HW_DWORD *)(DPS_BASE(instance)+0x0214))
#define DPS_REC_EVENT_START_ADDR_EX(instance)                                  \
                              ((volatile HW_DWORD *)(DPS_BASE(instance)+0x0218))
#define DPS_REC_EVENT_END_ADDR_EX(instance)                                    \
                              ((volatile HW_DWORD *)(DPS_BASE(instance)+0x021C))
#if PARSER_TYPE==DTV_PARSER
#define DPS_PIP_WRITE_PTR_EX(instance)                                         \
                              ((volatile HW_DWORD *)(DPS_BASE(instance)+0x0240))
#define DPS_PIP_READ_PTR_EX(instance)                                          \
                              ((volatile HW_DWORD *)(DPS_BASE(instance)+0x0244))
#define DPS_PIP_START_ADDR_EX(instance)                                        \
                              ((volatile HW_DWORD *)(DPS_BASE(instance)+0x0248))
#define DPS_PIP_END_ADDR_EX(instance)                                          \
                              ((volatile HW_DWORD *)(DPS_BASE(instance)+0x024C))
#define DPS_CAP_WRITE_PTR_EX(instance)                                         \
                              ((volatile HW_DWORD *)(DPS_BASE(instance)+0x0250))
#define DPS_CAP_READ_PTR_EX(instance)                                          \
                              ((volatile HW_DWORD *)(DPS_BASE(instance)+0x0254))
#define DPS_CAP_START_ADDR_EX(instance)                                        \
                              ((volatile HW_DWORD *)(DPS_BASE(instance)+0x0258))
#define DPS_CAP_END_ADDR_EX(instance)                                          \
                              ((volatile HW_DWORD *)(DPS_BASE(instance)+0x025C))
#define DPS_CWP_WRITE_PTR_EX(instance)                                         \
                              ((volatile HW_DWORD *)(DPS_BASE(instance)+0x0260))
#define DPS_CWP_READ_PTR_EX(instance)                                          \
                              ((volatile HW_DWORD *)(DPS_BASE(instance)+0x0264))
#define DPS_CWP_START_ADDR_EX(instance)                                        \
                              ((volatile HW_DWORD *)(DPS_BASE(instance)+0x0268))
#define DPS_CWP_END_ADDR_EX(instance)                                          \
                              ((volatile HW_DWORD *)(DPS_BASE(instance)+0x026C))
#define DPS_CAP_FILTER_EX(instance)                                            \
                              ((volatile HW_DWORD *)(DPS_BASE(instance)+0x0270))
#define DPS_CAP_SCID_EX(instance)                                              \
                              ((volatile HW_DWORD *)(DPS_BASE(instance)+0x0274))
#define DPS_PIP_SCID_EX(instance)                                              \
                              ((volatile HW_DWORD *)(DPS_BASE(instance)+0x0278))
#else
#define DPS_TS_BLOCK_COUNT_EX(instance)                                        \
                              ((volatile HW_DWORD *)(DPS_BASE(instance)+0x0278))
#endif
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
#define     DPS_CWP_BLOCK_STORED_MASK                              0x00000008
#define     DPS_CWP_BLOCK_STORED_SHIFT                                    3
#define         DPS_CWP_BLOCK_STORED                                 (1UL<<3)
#define     DPS_PSI_BUFFER_FULL_MASK                               0x00000010
#define     DPS_PSI_BUFFER_FULL_SHIFT                                     4
#define         DPS_PSI_BUFFER_FULL                                  (1UL<<4)
#define     DPS_CWP_BUFFER_FULL_MASK                               0x00000020
#define     DPS_CWP_BUFFER_FULL_SHIFT                                     5
#define         DPS_CWP_BUFFER_FULL                                  (1UL<<5)
#define     DPS_VIDEO_SPLICED_MASK                                 0x00000040
#define     DPS_VIDEO_SPLICED_SHIFT                                       6
#define         DPS_VIDEO_SPLICED                                    (1UL<<6)
#define     DPS_DVR_VIDEO_INTR_MASK                                0x00000040
#define     DPS_DVR_VIDEO_INTR_SHIFT                                      6
#define         DPS_DVR_VIDEO_INTR                                   (1UL<<6)
#define     DPS_REC_EVENT_INTR_MASK                                0x00000040
#define     DPS_REC_EVENT_INTR_SHIFT                                      6
#define         DPS_REC_EVENT_INTR                                   (1UL<<6)
#define     DPS_CAP_BLOCK_STORED_MASK                              0x00000040
#define     DPS_CAP_BLOCK_STORED_SHIFT                                    6
#define         DPS_CAP_BLOCK_STORED                                 (1UL<<6)
#define     DPS_CAP_BUFFER_FULL_MASK                               0x00000080
#define     DPS_CAP_BUFFER_FULL_SHIFT                                     7
#define         DPS_CAP_BUFFER_FULL                                  (1UL<<7)
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

#define AFE_RX2X_EN         0x80000000  /* Enable dbl rate RX data from codec    */
#define AFE_RX_FIFO_SZ_MSK  0x0F000000  /* RX fifo size 4/8 (32bit) words        */
#define AFE_RX_PKT_SZ_MSK   0x00F00000  /* RX DMA pkt size 1/2/4/8 words         */
#define AFE_RX_PIO_EN       0x00040000  /* Enable RX data via PIO (debug only)   */
#define AFE_RX_DMA_EN       0x00020000  /* Enable RX data via DMA                */
#define AFE_RX_ORUN_INT_EN  0x00010000  /* Enable RX fifo overrun interrupt      */
#define AFE_TX_FIFO_SZ_MSK  0x00000F00  /* TX fifo size 4/8 (32bit) words        */
#define AFE_TX_PKT_SZ_MSK   0x000000F0  /* TX DMA pkt size 1/2/4/8 words         */
#define AFE_TX_PIO_EN       0x00000004  /* Enable TX data via PIO (debug only)   */
#define AFE_TX_DMA_EN       0x00000002  /* Enable TX data via DMA                */
#define AFE_TX_URUN_INT_EN  0x00000001  /* Enable TX fifo underrun interrupt     */
                                        /*                                       */
#define AFE_RX_FIFO_SZ_4    (1 << 26)   /* RX fifo size 4 (32 bit) words         */
#define AFE_RX_FIFO_SZ_8    (1 << 27)   /* RX fifo size 8 (32 bit) words default */
#define AFE_RX_PKT_SZ_1     (1 << 20)   /* RX DMA pkt size 1 word                */
#define AFE_RX_PKT_SZ_2     (1 << 21)   /* RX DMA pkt size 2 words               */
#define AFE_RX_PKT_SZ_4     (1 << 22)   /* RX DMA pkt size 4 words               */
#define AFE_RX_PKT_SZ_8     (1 << 23)   /* RX DMA pkt size 8 words               */
                                        /*                                       */
#define AFE_TX_FIFO_SZ_4    (1 << 10)   /* TX fifo size 4 (32 bit) words         */
#define AFE_TX_FIFO_SZ_8    (1 << 11)   /* TX fifo size 8 (32 bit) words default */
#define AFE_TX_PKT_SZ_1     (1 << 4)    /* TX DMA pkt size 1 word                */
#define AFE_TX_PKT_SZ_2     (1 << 5)    /* TX DMA pkt size 2 words               */
#define AFE_TX_PKT_SZ_4     (1 << 6)    /* TX DMA pkt size 4 words               */
#define AFE_TX_PKT_SZ_8     (1 << 7)    /* TX DMA pkt size 8 words               */

/* Bit positions for accessing register as a single HW_DWORD */

#define AFE_STAT_RX_FIFO_LEVEL 0x0F000000
#define AFE_STAT_RX_PIO        0x00040000
#define AFE_STAT_RX_DMA        0x00020000
#define AFE_STAT_RX_ORUN       0x00010000
#define AFE_STAT_TX_FIFO_LEVEL 0x00000F00
#define AFE_STAT_TX_PIO        0x00000004
#define AFE_STAT_TX_DMA        0x00000002
#define AFE_STAT_TX_URUN       0x00000001

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

#define SDR_REF_CNT_MASK        0x000000ff

/* Bit masks to be used when accessing SDR_LOW_POWER_REG as a HW_DWORD */
#define SDR_LOW_POWER_ENABLE   0x00000001
#define SDR_LOW_POWER_MC_AVAIL 0x80000000

/* To be completed - how do you get into and out of auto-refresh mode ? */

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
  #define DMA_BUFFERED 0x4   /* Enable Buffered DMA                       */
  #define DMA_MULTIPLE 0x2   /* Enable multiple concurrent DMA operations */
  #define DMA_BUS_LOCK 0x1   /* Enable bus locking                        */
#define DMA_INT_REG          (DMA_BASE + 0x008) /* current channels requesting service (RO)         */
#define DMA_REQSTAT_REG      (DMA_BASE + 0x00C) /* reflects which devices are asserting DMA request */
#define DVTIMER_COUNT        (DMA_BASE + 0x010)
#define DVTIMER_LIMIT        (DMA_BASE + 0x014)
#define DVTIMER_MODE         (DMA_BASE + 0x018)

#define DMA_NUM_CHANNELS 4   /* Cx2249x silicon */
  #define  DMA_CH_0 0x1
  #define  DMA_CH_1 0x2
  #define  DMA_CH_2 0x4
  #define  DMA_CH_3 0x8
  #define  DMA_ATA  0x100	 /* Used for DMA_INT_REG register only */

/*  per channel registers	*/

#define DMA_CHANNEL_REG_BASE (DMA_BASE + 0x100)	/* offset to Channel 0 registers  */
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
   #define DMA_CHANNEL_RECOVERY_TIME(x)   (x<<16)    /* Recovery Time after DMA Op                         */
   #define DMA_CHANNEL_RECOVERY_TIME_MULT (1<<15)    /* Recovery Time Multiplier                           */
   #define DMA_CHANNEL_NON_PACED          (1<<14)    /* Non-Paced DMA                                      */
   #define DMA_XACT_SZ(x)                 ((x-1)<<9) /* Move this many 32-bit words                        */
   #define DMA_SELECT_REQ(x)              (x<<4)     /* DMA request line for channel                       */
   #define DMA_XFER_MODE_NORMAL           (0<<3)     /* Normal operation (DMA req triggers Xfer)           */
   #define DMA_XFER_MODE_SINGLE           (1<<3)     /* Single-Shot operation (Xfer buffer then terminate) */
   #define DMA_PREEMPT                    (1<<2)     /* Channel may preempt other DMA                      */
   #define DMA_INT_ENABLE                 (1<<1)     /* Enable DMA Interrupt                               */
   #define DMA_ENABLE                     1          /* Enable state machine                               */

#define DMA_STATUS_REG_CH(x)   (((x)*DMA_CHANNEL_SIZE)+DMA_STATUS_OFFSET) /* current status per channel */
   /* Masks  */
   #define DMA_READ_ERROR               (1<<6)     
   #define DMA_WRITE_ERROR              (1<<5)
   #define DMA_DST_INTERVAL             (1<<4)    /* Destination buffer has reached trigger level                 */
   #define DMA_SRC_INTERVAL             (1<<3)    /* Source buffer has reached trigger level                      */
   #define DMA_DST_FULL                 (1<<2)    /* DMA engine has wrapped around to start of destination buffer */
   #define DMA_SRC_EMPTY                (1<<1)    /* DMA engine has wrapped around to start of source buffer      */
   #define DMA_ACTIVE                   1         /* Indicates whether or not a DMA xfer is currently active      */

#define DMA_SRCADDR_REG_CH(x)   (((x)*DMA_CHANNEL_SIZE)+DMA_SRCADDR_OFFSET) /* current state machine address in source buffer      */
#define DMA_DSTADDR_REG_CH(x)   (((x)*DMA_CHANNEL_SIZE)+DMA_DSTADDR_OFFSET) /* current state machine address in destination buffer */
#define DMA_SRCBASE_REG_CH(x)   (((x)*DMA_CHANNEL_SIZE)+DMA_SRCBASE_OFFSET) /* base address of source buffer                       */
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

/***************************************/
/* Internal NTSC Channel 3/4 Modulator */
/***************************************/
#ifdef INCL_TVM

/* To be completed - check the sense of the bits in TVM_DAC_GENERAL_REG */

/* All registers are 8 bits. If no specific masks are defined, */
/* the value is a single 8 bit number                          */
#define TVM_REGISTER_MASK                  0x000000FF

#define TVM_MODULE_ID_REG                   (TVM_BASE + (0x00 * 4))
#define TVM_SW_HW_VERSION_REG               (TVM_BASE + (0x01 * 4))
#define TVM_SOFT_RESET_REG                  (TVM_BASE + (0x09 * 4))
#define     TVM_SOFT_RESET_DEFAULT_MASK       0x00000010
#define     TVM_SOFT_RESET_DEFAULT_SHIFT               4
#define         TVM_SOFT_RESET_DEFAULTS         (1UL<<4)
#define     TVM_SOFT_RESET_HARDWARE_MASK      0x00000002
#define     TVM_SOFT_RESET_HARDWARE_SHIFT              1
#define         TVM_SOFT_RESET_HARDWARE         (1UL<<1)
#define TVM_FL3_FL11_REG                    (TVM_BASE + (0x10 * 4))
#define     TVM_FL3_FL11_FL3_GAIN_MASK        0x000000E0
#define     TVM_FL3_FL11_FL3_GAIN_SHIFT                5
#define     TVM_FL3_FL11_FL3_K_MASK           0x00000018
#define     TVM_FL3_FL11_FL3_K_SHIFT                   3
#define     TVM_FL3_FL11_FL11_BYPASS_MASK     0x00000002
#define     TVM_FL3_FL11_FL11_BYPASS_SHIFT             1
#define     TVM_FL3_FL11_FL11_AUD_RATE_MASK   0x00000001
#define     TVM_FL3_FL11_FL11_AUD_RATE_SHIFT           0
#define TVM_FL1_K_REG                       (TVM_BASE + (0x11 * 4))
#define     TVM_FL1_K_KDA_MASK                0x000000F0
#define     TVM_FL1_K_KDA_SHIFT                        4      
#define     TVM_FL1_K_KIA_MASK                0x0000000F
#define     TVM_FL1_K_KIA_SHIFT                        0      
#define TVM_FL1_DDSSTD_MSB_REG              (TVM_BASE + (0x12 * 4))
#define     TVM_FL1_DDSSTD_AUD_MSB_MASK       0x0000000F
#define     TVM_FL1_DDSSTD_AUD_MSB_SHIFT               0      
#define TVM_FL1_DDSSTD_MID_REG              (TVM_BASE + (0x13 * 4))
#define TVM_FL1_DDSSTD_LSB_REG              (TVM_BASE + (0x14 * 4))
#define TVM_FL2_ZD_MSB_REG                  (TVM_BASE + (0x15 * 4))
#define TVM_FL2_ZD_LSB_PD_MSB_REG           (TVM_BASE + (0x16 * 4))
#define     TVM_FL2_ZD_LSB_MASK               0x000000F0
#define     TVM_FL2_ZD_LSB_SHIFT                       4
#define     TVM_FL2_PD_MSB_MASK               0x0000000F
#define     TVM_FL2_PD_MSB_SHIFT                       0
#define TVM_FL2_PD_LSB_REG                  (TVM_BASE + (0x17 * 4))
#define TVM_FL2_KD_MSB_REG                  (TVM_BASE + (0x18 * 4))
#define TVM_FL2_KD_LSB_REG                  (TVM_BASE + (0x19 * 4))
#define TVM_FL5_CARRIER_RATIO_MSB_REG       (TVM_BASE + (0x1A * 4))
#define     TVM_FL5_CARRIER_RATIO_MSB_MASK    0x0000007F           
#define     TVM_FL5_CARRIER_RATIO_MSB_SHIFT            0
#define TVM_FL5_CARRIER_RATIO_LSB_REG       (TVM_BASE + (0x1B * 4))
#define TVM_FL5_AUD_DEV_MSB_REG             (TVM_BASE + (0x1C * 4))
#define TVM_FL5_AUD_DEV_LSB_F_OFF_MSB_REG   (TVM_BASE + (0x1D * 4))
#define     TVM_FL5_AUD_DEV_LSB_MASK          0x000000F0
#define     TVM_FL5_AUD_DEV_LSB_SHIFT                  4
#define     TVM_FL5_F_OFF_MSB_MASK            0x0000000F
#define     TVM_FL5_F_OFF_MSB_SHIFT                    0
#define TVM_FL5_F_OFF_MID_REG               (TVM_BASE + (0x1E * 4))
#define TVM_FL5_F_OFF_LSB                   (TVM_BASE + (0x1F * 4))
#define TVM_FL7_FREQ1_REG                   (TVM_BASE + (0x20 * 4))
#define     TVM_FL7_FREQ1_MSB_MASK            0x0000000F
#define     TVM_FL7_FREQ1_MSB_SHIFT                    0
#define TVM_FL7_FREQ2_REG                   (TVM_BASE + (0x21 * 4))
#define TVM_FL7_FREQ3_REG                   (TVM_BASE + (0x22 * 4))
#define TVM_AMP7_MSB_REG                    (TVM_BASE + (0x23 * 4))
#define     TVM_AMP7_MSB_MASK                 0x0000007F
#define     TVM_AMP7 MSB_SHIFT                         0
#define TVM_AMP7_LSB_REG                    (TVM_BASE + (0x24 * 4))
#define TVM_SI_REG                          (TVM_BASE + (0x25 * 4))
#define TVM_FL10_K_REG                      (TVM_BASE + (0x28 * 4))
#define     TVM_FL10_KDV_MASK                 0x000000F0
#define     TVM_FL10_KDV_SHIFT                         4
#define     TVM_FL10_KIV_MASK                 0x0000000F
#define     TVM_FL10_KIV_SHIFT                         0
#define TVM_FL10_DDSSTD_VID_MSB_REG         (TVM_BASE + (0x29 * 4))
#define     TVM_FL10_DDSSTD_VID_MSB_MASK      0x0000000F
#define     TVM_FL10_DDSSTD_VID_MSB_SHIFT              4
#define TVM_FL10_DDSSTD_VID_MID_REG         (TVM_BASE + (0x2A * 4))
#define TVM_FL10_DDSSTD_VID_LSB_REG         (TVM_BASE + (0x2B * 4))
#define TVM_FL10_INPUT_GAIN_REG             (TVM_BASE + (0x2C * 4))
#define TVM_FL8_VDAC_GAIN_REG               (TVM_BASE + (0x2E * 4))
#define TVM_CHANNEL_SELECT_REG              (TVM_BASE + (0x2F * 4))
#define     TVM_CHANNEL_SELECT_MASK           0x00000001
#define     TVM_CHANNEL_SELECT_SHIFT                   0
#define         TVM_CHANNEL_SELECT_3            (0UL<<0)
#define         TVM_CHANNEL_SELECT_4            (1UL<<0)
#define TVM_DAC_GENERAL_REG                 (TVM_BASE + (0x30 * 4))
#define     TVM_DAC_GENERAL_PLL_LOCK_MASK     0x00000080
#define     TVM_DAC_GENERAL_PLL_LOCK_SHIFT             7
#define         TVM_DAC_GENERAL_PLL_LOCKED      (0UL<<7)
#define         TVM_DAC_GENERAL_PLL_UNLOCKED    (1UL<<7)
#define     TVM_DAC_GENERAL_PLL_ENABLE_MASK   0x00000040
#define     TVM_DAC_GENERAL_PLL_ENABLE_SHIFT           6
#define         TVM_DAC_GENERAL_PLL_DISABLED    (0UL<<6)
#define         TVM_DAC_GENERAL_PLL_ENABLED     (1UL<<6)
#define     TVM_DAC_GENERAL_POWER_DOWN_MASK   0x00000020
#define     TVM_DAC_GENERAL_POWER_DOWN_SHIFT           5
#define         TVM_DAC_GENERAL_POWER_UP        (0UL<<6)
#define         TVM_DAC_GENERAL_POWER_DOWN      (1UL<<6)
#define     TVM_DAC_GENERAL_RESET_RAND_MASK   0x00000010
#define     TVM_DAC_GENERAL_RESET_RAND_SHIFT           4
#define         TVM_DAC_GENERAL_RESET_RAND      (1UL<<4)
#define     TVM_DAC_GENERAL_RANDOM_MSB_MASK   0x00000008
#define     TVM_DAC_GENERAL_RANDOM_MSB_SHIFT           3
#define         TVM_DAC_GENERAL_NO_RANDOM_MSB   (0UL<<3)
#define         TVM_DAC_GENERAL_RANDOM_MSB      (1UL<<3)
#define     TVM_DAC_GENERAL_PLL_SELECT_MASK   0x00000004
#define     TVM_DAC_GENERAL_PLL_SELECT_SHIFT           2
#define     TVM_DAC_GENERAL_POSTDIV_MASK      0x00000003
#define     TVM_DAC_GENERAL_POSTDIV_SHIFT              0
#define TVM_DAC_PLL_XTS_REG                   (TVM_BASE + (0x31 * 4))
#define     TVM_DAC_PLL_XTS_MASK              0x0000003F
#define     TVM_DAC_PLL_XTS_SHIFT                      0
#define TVM_DAC_PLL_SPMP_REG                  (TVM_BASE + (0x32 * 4))
#define     TVM_DAC_PLL_SPMP_MASK             0x0000007F
#define     TVM_DAC_PLL_SPMP_THIFT                     0
#endif /* INCL_TVM */
                                   
/*****************************/
/* Internal QPSK Demodulator */
/*****************************/
#ifdef INCL_DMD

/**********************************************************************/
/* The demodulator register definitions are to be found in header file*/
/* cobra_enum.h. Driver software for the demod uses these definitions */
/* to interface with the hardware so mirror definitions here would    */
/* only complicate matters.                                           */
/*                                                                    */
/* To be completed - will this header be renamed?                     */
/**********************************************************************/

/* Map an I2C demod register byte address to an ASX memory-mapped */
/* register address                                               */
#define INTERNAL_DEMOD_REG_TO_ASX_ADDR(x) (((x)*4) + DMD_BASE)

#endif /* INCL_DMD */                         

/*********************/
/* Encryption Engine */
/*********************/
#ifdef INCL_ECY

/* Last updated 12/3/02 */

/* ECY_CAP_REG                       (ECY_BASE + 0x00)  */
/* ECY_GEN_INTR_REG                  (ECY_BASE + 0x04)  */
/* ECY_INTR_ENABLE_REG               (ECY_BASE + 0x08)  */
/* ECY_INTR_STATUS_REG               (ECY_BASE + 0x0C)  */
/* ECY_CHAN_1_MAPPING_REG            (ECY_BASE + 0x10)  */
/* ECY_CHAN_2_MAPPING_REG            (ECY_BASE + 0x14)  */
/* ECY_AUTH_ENABLE_REG               (ECY_BASE + 0x30)  */
/* ECY_CHAN_1_BASE                   (ECY_BASE + 0x100) */
/* ECY_CHAN_2_BASE                   (ECY_BASE + 0x200) */
/* ECY_CHAN_CTRL_REG(channel)        (ECY_BASE + 0x00)  */
/* ECY_CHAN_STATUS_REG(channel)      (ECY_BASE + 0x04)  */
/* ECY_CHAN_BYTE_CNT_REG(channel)    (ECY_BASE + 0x08)  */
/* ECY_CHAN_BUFFER_STATUS(channel)   (ECY_BASE + 0x0C)  */
/* ECY_CHAN_SRC_ADDR(channel)        (ECY_BASE + 0x10)  */
/* ECY_CHAN_DST_ADDR(channel)        (ECY_BASE + 0x14)  */
/* ECY_CHAN_MODE_REG(channel)        (ECY_BASE + 0x18)  */
/* ECY_CHAN_DATA_BASE(channel)       (ECY_BASE + 0x20)  */
/* ECY_CHAN_PID_TABLE_BASE(channel)  (ECY_BASE + 0x40)  */
/* ECY_CHAN_KEY_BASE(channel)        (ECY_BASE + 0x80)  */

#define ECY_CAP_REG                                     (ECY_BASE + 0x00)
#define     ECY_CAP_CHAN_TYPE_MASK                         0x000000FF
#define     ECY_CAP_CHAN_TYPE_SHIFT                           0
#define         ECY_CAP_CHAN_TYPE_BASIC                    (0UL<<0)
#define         ECY_CAP_CHAN_TYPE_EXT                      (1UL<<0)
#define     ECY_CAP_NUM_CHANS_MASK                         0x00000F00
#define     ECY_CAP_NUM_CHANS_SHIFT                           8
#define     ECY_CAP_ENCRYPTION_PROTOCOLS_SUPPORTED_MASK    0x00FF0000
#define     ECY_CAP_ENCRYPTION_PROTOCOLS_SUPPORTED_SHIFT      16
#define         ECY_CAP_PASSTHROUGH                        (1UL<<16)
#define         ECY_CAP_DES                                (2UL<<16)
#define         ECY_CAP_TDES                               (4UL<<16)
#define         ECY_CAP_AES                                (8UL<<16)

#define ECY_GEN_INTR_REG                                (ECY_BASE +0x04)
#define     ECY_GEN_INTR_KEY_0_MASK                        0x00000001
#define     ECY_GEN_INTR_KEY_0_SHIFT                          0
#define         ECY_GEN_INTR_KEY_0_ERR                     (1UL<<0)
#define     ECY_GEN_INTR_KEY_1_MASK                        0x00000002
#define     ECY_GEN_INTR_KEY_1_SHIFT                          1
#define         ECY_GEN_INTR_KEY_1_ERR                     (1UL<<1)

#define ECY_INTR_ENABLE_REG                             (ECY_BASE + 0x08)
#define ECY_INTR_STATUS_REG                             (ECY_BASE + 0x0C)
#define     ECY_GEN_INTR_MASK                              0x00000001
#define     ECY_GEN_INTR_SHIFT                                0
#define         ECY_GEN_INTR                               (1UL<<0)
#define     ECY_CHAN_1_INTR_MASK                           0x00000002
#define     ECY_CHAN_1_INTR_SHIFT                             1
#define         ECY_CHAN_1_INTR                            (1UL<<1)
#define     ECY_CHAN_2_INTR_MASK                           0x00000004
#define     ECY_CHAN_2_INTR_SHIFT                             2
#define         ECY_CHAN_2_INTR                            (1UL<<2)

#define ECY_CHAN_1_MAPPING_REG                          (ECY_BASE + 0x10)
#define ECY_CHAN_2_MAPPING_REG                          (ECY_BASE + 0x14)
#define     ECY_MAPPING_RANGE_SIZE_MASK                    0x0000FFFF
#define     ECY_MAPPING_RANGE_SIZE_SHIFT                      0
#define         ECY_MAPPING_RANGE_SIZE_64K                 (0xFFFF)
#define         ECY_MAPPING_RANGE_SIZE_128K                (0xFFFE)
#define         ECY_MAPPING_RANGE_SIZE_256K                (0xFFFC)
#define         ECY_MAPPING_RANGE_SIZE_512K                (0xFFF8)
#define         ECY_MAPPING_RANGE_SIZE_1MB                 (0xFFF0)
#define         ECY_MAPPING_RANGE_SIZE_2MB                 (0xFFE0)
#define         ECY_MAPPING_RANGE_SIZE_4MB                 (0xFFC0)
#define         ECY_MAPPING_RANGE_SIZE_8MB                 (0xFF80)
#define         ECY_MAPPING_RANGE_SIZE_16MB                (0xFF00)
#define         ECY_MAPPING_RANGE_SIZE_32MB                (0xFE00)
#define         ECY_MAPPING_RANGE_SIZE_64MB                (0xFC00)
#define         ECY_MAPPING_RANGE_SIZE_128MB               (0xF800)
#define         ECY_MAPPING_RANGE_SIZE_256MB               (0xF000)
#define         ECY_MAPPING_RANGE_SIZE_512MB               (0xE000)
#define         ECY_MAPPING_RANGE_SIZE_1GB                 (0xC000)
#define         ECY_MAPPING_RANGE_SIZE_2GB                 (0x8000)
#define         ECY_MAPPING_RANGE_SIZE_4GB                 (0x0000)
#define     ECY_MAPPING_BASE_ADDR_MASK                     0xFFFF0000
#define     ECY_MAPPING_BASE_ADDR_SHIFT                       0

#define ECY_AUTH_ENABLE_REG                             (ECY_BASE + 0x30)

#define ECY_KEY_CHANNEL                                    1
#define ECY_BULK_CHANNEL                                   2

#define ECY_CHAN_1_BASE                                 (ECY_BASE + 0x100)
#define ECY_CHAN_2_BASE                                 (ECY_BASE + 0x200)

#define ECY_CHAN_BASE(channel)             (HW_DWORD)(ECY_BASE+((channel)<<8))

#define ECY_CHAN_CTRL_REG(channel) \
                          ((volatile HW_DWORD *)(ECY_CHAN_BASE(channel)+0x00))
                          
#define     ECY_CHAN_CTRL_ENABLE_MASK                      0x00000001
#define     ECY_CHAN_CTRL_ENABLE_SHIFT                        0
#define         ECY_CHAN_CTRL_DISABLE                      (0UL<<0)
#define         ECY_CHAN_CTRL_ENABLE                       (1UL<<0)
#define     ECY_CHAN_CTRL_DONE_MASK                        0x00000001
#define     ECY_CHAN_CTRL_DONE_SHIFT                          0
#define         ECY_CHAN_CTRL_DONE                         (0UL<<0)
#define         ECY_CHAN_CTRL_BUSY                         (1UL<<0)
#define     ECY_CHAN_CTRL_DIRECTION_MASK                   0x00000002
#define     ECY_CHAN_CTRL_DIRECTION_SHIFT                     1
#define         ECY_CHAN_CTRL_DECRYPT                      (0UL<<1)
#define         ECY_CHAN_CTRL_ENCRYPT                      (1UL<<1)
#define     ECY_CHAN_CTRL_DES_TYPE_MASK                    0x00000004
#define     ECY_CHAN_CTRL_DES_TYPE_SHIFT                      2
#define         ECY_CHAN_CTRL_DES_TYPE_PASSIVE             (0UL<<2)
#define         ECY_CHAN_CTRL_DES_TYPE_ACTIVE              (1UL<<2)
#define     ECY_CHAN_CTRL_ENCRYPTION_MODE_MASK             0x000000F0
#define     ECY_CHAN_CTRL_ENCRYPTION_MODE_SHIFT               4
#define         ECY_CHAN_CTRL_PASSTHROUGH_MODE             (0UL<<4)
#define         ECY_CHAN_CTRL_DES_MODE                     (1UL<<4)
#define         ECY_CHAN_CTRL_TDES_MODE                    (2UL<<4)
#define         ECY_CHAN_CTRL_AES_MODE                     (3UL<<4)
#define     ECY_CHAN_CTRL_SRC_MODE_MASK                    0x00000100
#define     ECY_CHAN_CTRL_SRC_MODE_SHIFT                      8
#define         ECY_CHAN_CTRL_SRC_MODE_SLAVE               (0UL<<8)
#define         ECY_CHAN_CTRL_SRC_MODE_MASTER              (1UL<<8)
#define     ECY_CHAN_CTRL_DST_MODE_MASK                    0x00000200
#define     ECY_CHAN_CTRL_DST_MODE_SHIFT                      9
#define         ECY_CHAN_CTRL_DST_MODE_SLAVE               (0UL<<9)
#define         ECY_CHAN_CTRL_DST_MODE_MASTER              (1UL<<9)
#define     ECY_CHAN_CTRL_KEY_SELECT_MASK                  0x000F0000
#define     ECY_CHAN_CTRL_KEY_SELECT_SHIFT                    16
#define     ECY_CHAN_CTRL_KEY_SRC_MASK                     0x00100000
#define     ECY_CHAN_CTRL_KEY_SRC_SHIFT                       20
#define         ECY_CHAN_CTRL_KEY_SRC_SOFTWARE             (0UL<<20)
#define         ECY_CHAN_CTRL_KEY_SRC_HARDWARE             (1UL<<20)

#define ECY_CHAN_STATUS_REG(channel) \
                          ((volatile HW_DWORD *)(ECY_CHAN_BASE(channel)+0x04))
#define     ECY_CHAN_STATUS_OP_COMPLETE_MASK               0x00000001
#define     ECY_CHAN_STATUS_OP_COMPLETE_SHIFT                 0
#define         ECY_CHAN_STATUS_OP_COMPLETE                (1UL<<0)
#define     ECY_CHAN_STATUS_INTR_STATUS_MASK               0x00000001
#define     ECY_CHAN_STATUS_INTR_STATUS_SHIFT                 0
#define         ECY_CHAN_STATUS_INTR_STATUS                (1UL<<0)
#define     ECY_CHAN_STATUS_SRC_READ_ERR_MASK              0x00000002
#define     ECY_CHAN_STATUS_SRC_READ_ERR_SHIFT                1
#define         ECY_CHAN_STATUS_SRC_READ_ERR               (1UL<<1)
#define     ECY_CHAN_STATUS_DST_WRITE_ERR_MASK             0x00000004
#define     ECY_CHAN_STATUS_DST_WRITE_ERR_SHIFT               2
#define         ECY_CHAN_STATUS_DST_WRITE_ERR              (1UL<<2)
#define     ECY_CHAN_STATUS_DST_ERR_MASK                   0x00000004
#define     ECY_CHAN_STATUS_DST_ERR_SHIFT                     2
#define         ECY_CHAN_STATUS_DST_ERR                    (1UL<<2)
#define     ECY_CHAN_STATUS_CHAN_TYPE_MASK                 0xF0000000
#define     ECY_CHAN_STATUS_CHAN_TYPE_SHIFT                   28
#define         ECY_CHAN_STATUS_KEY                        (0UL<<28)

#define ECY_CHAN_BYTE_CNT_REG(channel) \
                          ((volatile HW_DWORD *)(ECY_CHAN_BASE(channel)+0x08))

#define ECY_CHAN_BUFFER_STATUS_REG(channel) \
                          ((volatile HW_DWORD *)(ECY_CHAN_BASE(channel)+0x0C))
                          
#define     ECY_CHAN_BUFFER_STATUS_SRC_BUFFER_MASK         0x000000FF
#define     ECY_CHAN_BUFFER_STATUS_SRC_BUFFER_SHIFT           0
#define     ECY_CHAN_BUFFER_STATUS_DST_BUFFER_MASK         0x0000FF00
#define     ECY_CHAN_BUFFER_STATUS_DST_BUFFER_SHIFT           8

#define ECY_CHAN_SRC_ADDR_REG(channel) \
                          ((volatile HW_DWORD *)(ECY_CHAN_BASE(channel)+0x10))

#define ECY_CHAN_DST_ADDR_REG(channel) \
                          ((volatile HW_DWORD *)(ECY_CHAN_BASE(channel)+0x14))

#define ECY_CHAN_MODE_REG(channel) \
                          ((volatile HW_DWORD *)(ECY_CHAN_BASE(channel)+0x18))
                          
#define     ECY_CHAN_MODE_TDES_CONFIG_MASK                 0x00000007
#define     ECY_CHAN_MODE_TDES_CONFIG_SHIFT                   0
#define         ECY_CHAN_MODE_TDES_EEE                     (0UL<<0)
#define         ECY_CHAN_MODE_TDES_EDE                     (2UL<<0)
#define     ECY_CHAN_MODE_INPUT_ENDIAN_MASK                0x00000100
#define     ECY_CHAN_MODE_INPUT_ENDIAN_SHIFT                  8
#define         ECY_CHAN_MODE_INPUT_LE                     (0UL<<8)
#define         ECY_CHAN_MODE_INPUT_BE                     (1UL<<8)
#define     ECY_CHAN_MODE_OUTPUT_ENDIAN_MASK               0x00000200
#define     ECY_CHAN_MODE_OUTPUT_ENDIAN_SHIFT                 9
#define         ECY_CHAN_MODE_OUTPUT_LE                    (0UL<<9)
#define         ECY_CHAN_MODE_OUTPUT_BE                    (1UL<<9)
#define     ECY_CHAN_MODE_KEY_ENDIAN_MASK                  0x00000400
#define     ECY_CHAN_MODE_KEY_ENDIAN_SHIFT                    10
#define         ECY_CHAN_MODE_KEY_LE                       (0UL<<10)
#define         ECY_CHAN_MODE_KEY_BE                       (1UL<<10)
#define     ECY_CHAN_MODE_KEY_ENABLE_DST_FIFO_REWIND_MASK  0x00001000
#define     ECY_CHAN_MODE_KEY_ENABLE_DST_FIFO_REWIND_SHIFT    12
#define         ECY_CHAN_MODE_KEY_ENABLE_DST_FIFO_REWIND   (0UL<<12)

#define ECY_CHAN_DATA_BASE(channel) \
                          ((volatile HW_DWORD *)(ECY_CHAN_BASE(channel) +0x20))

#define ECY_CHAN_PID_TABLE_BASE(channel) \
                          ((volatile HW_DWORD *)(ECY_CHAN_BASE(channel)+0x40))

#define ECY_CHAN_KEY_BASE_REG(channel) \
                          ((volatile HW_DWORD *)(ECY_CHAN_BASE(channel)+0x80))

#endif /* INCL_ECY */                 
           
/***************************************/
/* Hardware buffer addresses and sizes */
/***************************************/

/* Buffer sizes */

/* Note: Buffer sizes in K bytes must be defined in the hardware CONFIG file */

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
#define HWBUF_CWP_SIZE         (CWP_SIZE * 1024)
#define HWBUF_CAP_SIZE         (CAP_SIZE * 1024)

#define DPS_NUM_DESCRAMBLERS     25
#define DPS_NUM_ECM_SLOTS        6
#define DPS_NUM_EMM_SLOTS        10
#define DPS_ECM_SLOT_SIZE        256
#define DPS_EMM_SLOT_SIZE        256

/* Fixed size, fixed position buffers */

#ifdef MEM_ZISE_4M

#if MEM_ZISE_4M==YES

#ifndef MEM_START_ADDR
#define MEM_START_ADDR 0x000380
#endif

#define HWBUF_PCRPTS_ADDR        MEM_START_ADDR
#else
#define HWBUF_PCRPTS_ADDR        0x000380
#endif //#if MEM_ZISE_4M==YES

#else
#define HWBUF_PCRPTS_ADDR        0x000380
#endif //#ifdef MEM_ZISE_4M


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
#define HWBUF_CAM_ECM_SIZE       ((DPS_NUM_DEMUXES)*DPS_NUM_ECM_SLOTS*DPS_ECM_SLOT_SIZE)
#define HWBUF_CAM_EMM_START_ADDR (HWBUF_CAM_ECM_START_ADDR + HWBUF_CAM_ECM_SIZE)
#define HWBUF_CAM_EMM_SIZE       ((DPS_NUM_DEMUXES)*DPS_NUM_EMM_SLOTS*DPS_EMM_SLOT_SIZE)

#define HWBUF_ENCAC3_ADDR        (HWBUF_CAM_EMM_START_ADDR + HWBUF_CAM_EMM_SIZE)
#define HWBUF_CWP_ADDR           (HWBUF_ENCAC3_ADDR + HWBUF_ENCAUD_SIZE)
#define HWBUF_CAP_ADDR           (HWBUF_CWP_ADDR + HWBUF_CWP_SIZE)

#define HWBUF_TOP                (HWBUF_CAP_ADDR           + HWBUF_CAP_SIZE)

#define NUM_OF_GPIO              108

#endif   /* #ifdef _BRAZOS_H_ */

/****************************************************************************
 * Modifications:
 * $Log: 
 *  85   mpeg      1.84        6/16/04 11:12:44 AM    Tim White       CR(s) 
 *        9483 9484 : Changed pvr to rec for consistency.
 *        
 *  84   mpeg      1.83        5/24/04 4:44:29 PM     Tim White       CR(s) 
 *        9296 9297 : Add MPG_OFFSET registers for PVR.
 *        
 *  83   mpeg      1.82        5/4/04 2:21:31 PM      Tim White       CR(s) 
 *        9088 9089 : Update trick mode video microcode command set to match 
 *        spec.
 *        
 *  82   mpeg      1.81        4/22/04 5:07:39 PM     Sunil Cheruvu   CR(s) 
 *        8870 8871 : Added the Nand Flash support for Wabash(Milano rev 5 and 
 *        above) and Brazos(Bronco).
 *  81   mpeg      1.80        4/20/04 12:17:20 AM    Steve Glennon   CR(s) 
 *        8890 8889 : Changed NUM_OF_CPIOS from 96 to 108.
 *        
 *  80   mpeg      1.79        4/12/04 10:39:51 AM    Larry Wang      CR(s) 
 *        8826 8827 : Define DPS_TS_BLOCK_COUNT_EX register.
 *  79   mpeg      1.78        4/1/04 5:01:18 PM      Dave Aerne      CR(s) 
 *        8729 8730 : added support for ac3 passthrough w/o mpeg decode on 
 *        brazos and wabash chips.
 *  78   mpeg      1.77        3/19/04 2:36:53 PM     Craig Dry       CR(s) 
 *        8599 : Fix compiler warnings for Codeldr.
 *  77   mpeg      1.76        3/17/04 1:03:41 PM     Billy Jackman   CR(s) 
 *        8581 : Added definition of INTERNAL_DEMOD_HAS_ASX_BUG as YES.
 *  76   mpeg      1.75        3/10/04 2:54:01 PM     Larry Wang      CR(s) 
 *        8551 : Add definition of registers for Sony Passage.
 *  75   mpeg      1.74        3/10/04 10:41:23 AM    Bob Van Gulick  CR(s) 
 *        8546 : Add support to return PES frame rate
 *  74   mpeg      1.73        2/24/04 2:42:31 PM     Bob Van Gulick  CR(s) 
 *        8427 : Add timebase_offset functionality to demux
 *        
 *  73   mpeg      1.72        2/11/04 8:37:48 AM     Steven Jones    CR(s) 
 *        8382 : Add 14:9 format macros MPG_VID_ASPECT_149 and 
 *        MPG_VID_ASPECT_RATIO_149 to
 *        allow support for this aspect ratio which is included in DTG spec.
 *  72   mpeg      1.71        2/4/04 3:01:32 PM      Dave Wilson     CR(s) 
 *        8318 : Added definition of DRM_NEEDS_SPLIT_FIFO_ON_WIPE to allow 
 *        conditional compilation of workaround code to fix a chroma problem on
 *         image plane wipes with Brazos rev B.
 *        
 *  71   mpeg      1.70        1/27/04 11:44:59 AM    Mark Thissen    CR(s) 
 *        8269 : Fixing Formatting Error.
 *        
 *  70   mpeg      1.69        1/26/04 3:30:04 PM     Mark Thissen    CR(s) 
 *        8269 : Definitions for early sync reg for fast channel change.
 *        
 *  69   mpeg      1.68        1/26/04 12:16:30 PM    Billy Jackman   CR(s) 
 *        5111 : Correct the definition of PLL_FAST_CTRL_DEMOD_CONNECT_I2C so 
 *        that changes to demod_cobra.c to allow IIC bus access to Camaro work 
 *        correctly.
 *  68   mpeg      1.67        1/23/04 5:10:37 PM     Angela Swartz   CR(s) 
 *        8264 : Corrected a typo in ENC_CTL0_BLUEFIELD_ENABLE 1UL<<7, it was 
 *        1UL<<8
 *  67   mpeg      1.66        12/4/03 4:56:56 PM     Tim Ross        CR(s) 
 *        8099 : Added IR overflow disable bit definition.
 *  66   mpeg      1.65        11/1/03 2:52:58 PM     Tim Ross        CR(s): 
 *        7719 7762 Added definitions for RST_SCRATCH_REG bits.
 *  65   mpeg      1.64        10/30/03 4:26:49 PM    Tim Ross        CR(s): 
 *        7719 7762 Added definitions for scratch register 0.
 *  64   mpeg      1.63        9/23/03 4:42:32 PM     Craig Dry       SCR(s) 
 *        7532 :
 *        New defines for Power Driver
 *        
 *  63   mpeg      1.62        9/22/03 4:51:30 PM     Bob Van Gulick  SCR(s) 
 *        7519 :
 *        Add support for DirecTV CAPs
 *        
 *        
 *  62   mpeg      1.61        9/19/03 6:48:02 PM     Dave Aerne      SCR(s) 
 *        7514 :
 *        cleanup to map CORE_RESET defines to standard CONTROL0 defines
 *        
 *  61   mpeg      1.60        8/27/03 11:02:54 AM    Bob Van Gulick  SCR(s) 
 *        7387 :
 *        Add support for CWP processing in DirecTV
 *        
 *        
 *  60   mpeg      1.59        8/18/03 2:32:18 PM     Miles Bintz     SCR(s) 
 *        7291 :
 *        added mmy type definitions
 *        
 *  59   mpeg      1.58        7/18/03 11:56:04 AM    Dave Wilson     SCR(s) 
 *        6967 :
 *        Added definition of label DRM_FILTER_PHASE_0_PROBLEM NO. This 
 *        prevents 
 *        Brazos builds including the special-case scaling code that was 
 *        introduced to
 *        fix the Colorado/Wabash DRM scaler problem.
 *        
 *  58   mpeg      1.57        7/9/03 6:21:38 PM      Senthil Veluswamy SCR(s) 
 *        6922 :
 *        Added register/bit defines for SPDIF header data register
 *        
 *  57   mpeg      1.56        6/23/03 7:36:16 PM     Senthil Veluswamy SCR(s) 
 *        6641 :
 *        Added masks in the AUD Attn registers for RF Modulator and Audio 
 *        Decoder volume.
 *        
 *  56   mpeg      1.55        6/9/03 5:57:16 PM      Bob Van Gulick  SCR(s) 
 *        6755 :
 *        Add support for 8 slot descram in demux.  Needed to move registers 
 *        between 1E0 and 200 to a different location to support this feature.
 *        
 *        
 *  55   mpeg      1.54        6/6/03 11:18:26 AM     Dave Wilson     SCR(s) 
 *        6737 :
 *        Added definitions for RST_SMARTCRD_RESET_CTL_REG
 *        
 *  54   mpeg      1.53        6/4/03 4:30:08 PM      Billy Jackman   SCR(s) 
 *        6712 :
 *        Added definitions for PLL_TEST_REG and the function disable bits in 
 *        it.  Use
 *        these bits to determine if a function has been disabled by 
 *        programming a fuse.
 *        
 *  53   mpeg      1.52        6/3/03 10:15:24 AM     Larry Wang      SCR(s) 
 *        6667 :
 *        Define HWBUF_ENCAC3_ADDR as the start pointer for the buffer of 
 *        secondary audio channel (AC3 pass through channel).
 *        
 *  52   mpeg      1.51        5/29/03 7:07:36 PM     Angela Swartz   SCR(s) 
 *        6523 6248 :
 *        added definitions for added bits in TVM_DAC_GENERAL_REG for RF 
 *        Modulator
 *        
 *  51   mpeg      1.50        5/28/03 2:16:34 PM     Bobby Bradford  SCR(s) 
 *        6607 6612 :
 *        Added definitions for the PLL_CLK_OBSERVATION_REG, to enable
 *        selection of the PIO18 (Clock Observe #3) for smartmdp modem
 *        operation
 *        
 *  50   mpeg      1.49        5/22/03 5:30:52 PM     Dave Wilson     SCR(s) 
 *        6564 6565 :
 *        Added definitions of CACHE_LINE_SIZE and NONCACHEABLE_RAM_MASK. 
 *        Previously
 *        these were in various KAL files.
 *        
 *  49   mpeg      1.48        5/20/03 6:11:42 PM     Dave Wilson     SCR(s) 
 *        6499 6500 :
 *        Added ENCODER_HAS_DAC_BUG for Brazos rev B.
 *        
 *  48   mpeg      1.47        5/20/03 3:16:26 PM     Dave Wilson     SCR(s) 
 *        6485 6486 :
 *        Changed definition of DRM_HAS_WIPE_CHROMA_BUG such that it is defined
 *         as YES
 *        for Brazos Rev A and NO for Rev B.
 *        
 *  47   mpeg      1.46        5/19/03 9:47:24 PM     Angela Swartz   SCR(s) 
 *        6443 6444 :
 *        Added definitions relatingto the RF Modulator
 *        
 *  46   mpeg      1.45        5/19/03 4:42:12 PM     Dave Wilson     SCR(s) 
 *        6437 6438 :
 *        Added definitions relating to Brazos rev B's 
 *        DRM_SATURATION_VALUES_REG
 *        
 *  45   mpeg      1.44        5/15/03 6:46:42 PM     Dave Aerne      SCR(s) 
 *        6261 6291 :
 *        added defines for audio aoc microcode download
 *        
 *  44   mpeg      1.43        5/15/03 3:34:34 PM     Dave Wilson     SCR(s) 
 *        6367 6366 :
 *        Added new macro DRM_HAS_16_BIT_CURSOR_BUG.
 *        
 *  43   mpeg      1.42        5/14/03 4:11:14 PM     Tim White       SCR(s) 
 *        6346 6347 :
 *        Added hardware pagetable address.
 *        
 *        
 *  42   mpeg      1.41        5/9/03 10:39:14 AM     Tim White       SCR(s) 
 *        6282 6283 :
 *        NDS CAM ECM & EMM buffers were incorrectly allocated in the same 
 *        memory.
 *        
 *        
 *  41   mpeg      1.40        5/2/03 11:13:56 AM     Bob Van Gulick  SCR(s) 
 *        6151 :
 *        Add pts_offset and dma_mux memory locations
 *        
 *        
 *  40   mpeg      1.39        4/23/03 11:08:44 AM    Dave Wilson     SCR(s) 
 *        5862 :
 *        Added DRM_TILE_TYPE chip feature and updated wipe/tile DRM registers 
 *        to
 *        reflect Brazos rev B changes.
 *        
 *  39   mpeg      1.38        4/15/03 11:58:42 AM    Dave Wilson     SCR(s) 
 *        6023 :
 *        Added new definitions for bits 16 and 17 of MPG_ADDR_EXT_REG. These 
 *        now 
 *        indicate the type of picture last decoded when a decode complete 
 *        interrupt
 *        fires. This addition requires microcode version 0.3 or later.
 *        
 *  38   mpeg      1.37        3/26/03 9:48:14 AM     Billy Jackman   SCR(s) 
 *        5854 :
 *        Correct setting of CPU_TYPE so that the proper type is set for Brazos
 *         rev A
 *        and for later revisions.
 *        Added page table control register definitions needed for rev B and 
 *        beyond.
 *        
 *        
 *  37   mpeg      1.36        3/25/03 11:09:42 AM    Brendan Donahe  SCR(s) 
 *        5845 :
 *        Replaced single DPS_VERSION_MODES_REG with 4 to support 3 banks of 
 *        version
 *        filtering modes.
 *        
 *        
 *  36   mpeg      1.35        3/17/03 4:26:32 PM     Dave Wilson     SCR(s) 
 *        5756 :
 *        Added macros to allow access to different infra-red block instances 
 *        based upon
 *        a bank number.
 *        
 *  35   mpeg      1.34        3/17/03 12:02:20 PM    Dave Wilson     SCR(s) 
 *        5788 :
 *        Corrected IRD_BANK_SIZE to match the spec. Was previously defined as 
 *        0x1000
 *        but should be 0x100
 *        
 *  34   mpeg      1.33        3/13/03 2:46:02 PM     Miles Bintz     SCR(s) 
 *        5753 :
 *        fixed type
 *        typo
 *        
 *        
 *  33   mpeg      1.32        3/13/03 12:21:54 PM    Miles Bintz     SCR(s) 
 *        5753 :
 *        Added HSDP_BIDIR_PORT_CNTL_SER_TO_PAR_CONV_DISABLE definition to chip
 *         header file
 *        
 *        
 *  32   mpeg      1.31        3/7/03 10:58:34 AM     Bobby Bradford  SCR(s) 
 *        5708 :
 *        Fixed the definition of the DSPKR GPIO setting (Mux0 --> Mux3)
 *        
 *  31   mpeg      1.30        3/5/03 3:51:06 PM      Miles Bintz     SCR(s) 
 *        5634 :
 *        added CHIP_SUPPORTS_PIO_TIMED_RESET feature definition to chip header
 *         files
 *        
 *        
 *  30   mpeg      1.29        2/28/03 11:15:18 AM    Dave Aerne      SCR(s) 
 *        4672 :
 *        for audio AUTO_DETECT feature, added DPS_AUDIO_PID_STREAM_ID_EX at 
 *        address
 *        DPS_BASE + 0x1FC
 *        
 *  29   mpeg      1.28        2/20/03 2:11:22 PM     Brendan Donahe  SCR(s) 
 *        5567 :
 *        Set ATA_TYPE for Brazos, added PIN_ALT_FUNC_HDA*
 *        
 *        
 *  28   mpeg      1.27        2/19/03 2:16:52 PM     Bobby Bradford  SCR(s) 
 *        5467 5549 :
 *        Changed definition of the DPS_KEY_TABLE_BASE_EX macro ... there
 *        was an incorrect offset specified in the BRAZOS data sheet
 *        
 *  27   mpeg      1.26        2/19/03 10:50:44 AM    Dave Wilson     SCR(s) 
 *        5528 5543 :
 *        Added 2 new chip features to enable workarounds for a couple of DRM 
 *        bugs:
 *        DRM_HAS_WIPE_CHROMA_BUG = YES 
 *        DRM_HAS_UPSCALE_PAL_STILL_BUG = YES
 *        
 *  26   mpeg      1.25        2/17/03 4:14:48 PM     Larry Wang      SCR(s) 
 *        5525 :
 *        define bit 4, 5, 6 of SMC_PARITY_REG for Brazos.
 *        
 *  25   mpeg      1.24        2/14/03 3:00:28 PM     Dave Wilson     SCR(s) 
 *        5513 :
 *        Corrected definition of DRM_CURSOR_STORE_ADDR_REG. The previous 
 *        version
 *        did not take into account changes from Wabash to Brazos.
 *        
 *  24   mpeg      1.23        2/12/03 3:03:00 PM     Bobby Bradford  SCR(s) 
 *        5467 :
 *        1) Removed unused/unreferenced DPS_ODD|EVEN_KEY_ENABLE_SHIFT|MASK 
 *        defines
 *        2) Added DPS_ODD|EVEN_KEY_ENABLE_REG_EX macros, removed the single
 *        DPS_KEY_ENABLE_EX macro
 *        
 *  23   mpeg      1.22        2/11/03 3:46:40 PM     Bobby Bradford  SCR(s) 
 *        5467 :
 *        Changed DPS_KEY_TABLE_BASE offset from 0x0180 to 0x4000 (changed
 *        in spec, but not in the header file yet)
 *        
 *  22   mpeg      1.21        2/6/03 3:11:06 PM      Dave Aerne      SCR(s) 
 *        5262 :
 *        added definitions for AUD_MPEG_CAPT_ATTEN regs plus assorted AUD test
 *         regs
 *        
 *  21   mpeg      1.20        2/5/03 11:48:48 AM     Dave Wilson     SCR(s) 
 *        5405 :
 *        Added ENC_CTL0_ALL_DAC_MASK and ENC_CTL0_ALL_DAC_SHIFT to allow all 
 *        DACs to
 *        be controlled in a single operation.
 *        
 *  20   mpeg      1.19        2/4/03 5:07:52 PM      Bob Van Gulick  SCR(s) 
 *        5407 :
 *        Change scrambling status to use separate TS and PES registers
 *        
 *        
 *  19   mpeg      1.18        1/24/03 7:00:38 PM     Brendan Donahe  SCR(s) 
 *        5315 :
 *        Updated 16-bit MC audio workaround fix with PCM_AUDIO_TYPE 
 *        definition.
 *        
 *        
 *  18   mpeg      1.17        1/24/03 2:35:48 PM     Brendan Donahe  SCR(s) 
 *        5315 :
 *        Added 3 audio (AUD_CHANNEL_CONTROL_REG) definitions - copyright, 
 *        6-channel 
 *        test, and MPEG PCM capture attenuation.
 *        
 *        
 *  17   mpeg      1.16        1/23/03 2:28:20 PM     Bobby Bradford  SCR(s) 
 *        5103 :
 *        Add definitions for DAA clock select (new for Brazos)
 *        
 *  16   mpeg      1.15        1/22/03 10:41:04 AM    Tim White       SCR(s) 
 *        5099 :
 *        Added new PLL type for Brazos.
 *        
 *        
 *  15   mpeg      1.14        1/17/03 4:46:04 PM     Miles Bintz     SCR(s) 
 *        5089 :
 *        Added definition for PIO timed reset functionality
 *        
 *        
 *  14   mpeg      1.13        1/15/03 2:33:46 PM     Billy Jackman   SCR(s) 
 *        5095 :
 *        Added specification of INTERNAL_DEMOD as INTERNAL_COBRA_LIKE.
 *        
 *  13   mpeg      1.12        12/19/02 3:12:08 PM    Tim White       SCR(s) 
 *        5068 :
 *        Set Prescale to all 1's for Brazos.
 *        
 *        
 *  12   mpeg      1.11        12/17/02 4:02:44 PM    Senthil Veluswamy SCR(s) 
 *        5067 :
 *        New IIC register bits for Brazos, support for 2 IIC banks, removed 
 *        IIC macros to private header.
 *        
 *  11   mpeg      1.10        12/17/02 3:48:22 PM    Tim White       SCR(s) 
 *        5182 :
 *        Remove ARM_PIT_TYPE, no longer needed.
 *        
 *        
 *  10   mpeg      1.9         12/16/02 3:48:14 PM    Tim White       SCR(s) 
 *        5169 :
 *        Allow future chips to use Wabash code by default instead of the 
 *        Colorado code.
 *        
 *        
 *  9    mpeg      1.8         12/12/02 5:13:56 PM    Tim White       SCR(s) 
 *        5157 :
 *        Added Brazos specific ATA & PLL types.  Added indicator for PCI sync 
 *        bits.
 *        Added a ISBRAZOSREVA macro for runtime chipREV detection.  Removed 
 *        PCI_ROM_MODE_REG_TYPE def'n.
 *        
 *        
 *  8    mpeg      1.7         12/10/02 3:03:40 PM    Dave Wilson     SCR(s) 
 *        5071 :
 *        Added VIDEO_UCODE_RAM_SIZE_WORDS chip feature to define the size of 
 *        the video
 *        microcode RAM.
 *        
 *  7    mpeg      1.6         12/10/02 1:59:16 PM    Bob Van Gulick  SCR(s) 
 *        5121 :
 *        Add scrambler status memory location for demux
 *        
 *        
 *  6    mpeg      1.5         12/10/02 1:34:16 PM    Dave Wilson     SCR(s) 
 *        5091 :
 *        Reworked interrupt controller bit positions
 *        
 *  5    mpeg      1.4         12/9/02 3:50:18 PM     Dave Wilson     SCR(s) 
 *        4903 :
 *        Minor change to the TVM register set - added one register that had 
 *        been
 *        missed out on the last edit.
 *        
 *  4    mpeg      1.3         12/6/02 5:11:40 PM     Dave Wilson     SCR(s) 
 *        4903 :
 *        Latest version - added TV modulator, updated I2C and ITC blocks
 *        
 *  3    mpeg      1.2         12/5/02 3:47:28 PM     Dave Wilson     SCR(s) 
 *        4903 :
 *        Interim version - system controller and system registers updated.
 *        
 *  2    mpeg      1.1         12/4/02 1:59:34 PM     Dave Wilson     SCR(s) 
 *        4903 :
 *        Added ECY block and made a few other changes. This is an interim put 
 *        since
 *        the file, in it's current state, allows WATCHTV to build (though, it 
 *        is 
 *        unlikely to run correctly!).
 *        
 *  1    mpeg      1.0         11/6/02 10:18:42 AM    Dave Wilson     
 * $
 ****************************************************************************/


