/****************************************************************************/
/*                   CONEXANT PROPRIETARY AND CONFIDENTIAL                  */
/*                       SOFTWARE FILE/MODULE HEADER                        */
/*                   Conexant Systems Inc. (c) 1998 - 2003                  */
/*                               Austin, TX                                 */
/*                            All Rights Reserved                           */
/****************************************************************************/
/*
 * Filename:       int_id.h
 *
 *
 * Description:    Interrupt Identifier definitions and associated macros
 *
 *
 * Author:         Dave Wilson
 *
 ****************************************************************************/
/* $Id: int_id.h,v 1.0, 2004-03-01 21:02:13Z, Dave Wilson$
 ****************************************************************************/

#ifndef _INT_ID_H_
#define _INT_ID_H_

/**********************************************************************/
/* Use this macro to generate the dwIntID parameter to any of the     */
/* KAL interrupt service functions. Parameter x is either 0 or 1 and  */
/* indicates the PIC in use, parameter y is the bit number associated */
/* with the requested interrupt.                                      */
/**********************************************************************/
#define CALC_INT_ID(x, y) ((x << 16) | (y))
#define PIC_FROM_INT_ID(x) (x >> 16)
#define INT_FROM_INT_ID(x) (x & 0xFF)

#define PIC_CENTRAL      0x00
#define PIC_EXPANSION    0x01
#define PIC_TIMERS       0x02
#define PIC_GPIO         0x03

/************************/
/* Useful Interrupt IDs */
/************************/

/* Main interrupt status register - sources common to all chips */
#define INT_SYNCSER     CALC_INT_ID(PIC_CENTRAL, ITC_SYNCSERIAL_POS)
#define INT_DRM         CALC_INT_ID(PIC_CENTRAL, ITC_DRM_POS)
#define INT_MPEGSUB     CALC_INT_ID(PIC_CENTRAL, ITC_MPEG_POS)
#define INT_RTC         CALC_INT_ID(PIC_CENTRAL, ITC_RTC_POS)
#define INT_AUDIORX     CALC_INT_ID(PIC_CENTRAL, ITC_AUDIORX_POS)
#define INT_AUDIOTX     CALC_INT_ID(PIC_CENTRAL, ITC_AUDIOTX_POS)
#define INT_DVT         CALC_INT_ID(PIC_CENTRAL, ITC_DVT_POS)
#define INT_GIR         CALC_INT_ID(PIC_CENTRAL, ITC_GIR_POS)
#define INT_UART0       CALC_INT_ID(PIC_CENTRAL, ITC_UART0_POS)
#define INT_UART1       CALC_INT_ID(PIC_CENTRAL, ITC_UART1_POS)
#define INT_SCD0        CALC_INT_ID(PIC_CENTRAL, ITC_SCR0_POS)
#define INT_DMA         CALC_INT_ID(PIC_CENTRAL, ITC_DMA_POS)
#define INT_AFE         CALC_INT_ID(PIC_CENTRAL, ITC_AFE_POS)
#define INT_SCD1        CALC_INT_ID(PIC_CENTRAL, ITC_SCR1_POS)
#define INT_PLSTIMER    CALC_INT_ID(PIC_CENTRAL, ITC_PULSETIMER_POS)
#define INT_DAA         CALC_INT_ID(PIC_CENTRAL, ITC_DAA_POS)
#define INT_PCI         CALC_INT_ID(PIC_CENTRAL, ITC_PCI_POS)
#define INT_PAW1        CALC_INT_ID(PIC_CENTRAL, ITC_PAR0_POS)
#define INT_GXA         CALC_INT_ID(PIC_CENTRAL, ITC_GXA_POS)
#define INT_UART2       CALC_INT_ID(PIC_CENTRAL, ITC_UART2_POS)
#define INT_NDSC        CALC_INT_ID(PIC_CENTRAL, ITC_NDSC_POS)
#define INT_PWM         CALC_INT_ID(PIC_CENTRAL, ITC_PWM_POS)
#define INT_GPIO        CALC_INT_ID(PIC_CENTRAL, ITC_GPIO_POS)
#define INT_TIMERS      CALC_INT_ID(PIC_CENTRAL, ITC_TIMER_POS)
#define INT_RESERVED1   CALC_INT_ID(PIC_CENTRAL, ITC_RESERVED1_POS)
#define INT_RESERVED2   CALC_INT_ID(PIC_CENTRAL, ITC_RESERVED2_POS)

/*************************************************************/
/* Main interrupt controller sources which are chip-specific */
/*************************************************************/

/* Interrupt expansion register */
#ifdef ITC_EXPANSION_POS
  #define INT_EXP         CALC_INT_ID(PIC_CENTRAL, ITC_EXPANSION_POS)
#endif

/* External video (ELVIS) input */
#ifdef ITC_EXTVID_POS
  #define INT_EXTVID      CALC_INT_ID(PIC_CENTRAL, ITC_EXTVID_POS)
#endif

/* ATAPI interface */
#ifdef ITC_ATAPI_POS
  #define INT_ATAPI       CALC_INT_ID(PIC_CENTRAL, ITC_ATAPI_POS)
#endif

/* USB controller */
#ifdef ITC_USB_POS
  #define INT_USB         CALC_INT_ID(PIC_CENTRAL, ITC_USB_POS)
#endif

/* Additional parser instances */
#ifdef ITC_PAR1_POS
  #define INT_PAW2        CALC_INT_ID(PIC_CENTRAL, ITC_PAR1_POS)
#endif
#ifdef ITC_PAR2_POS
  #define INT_PAW3        CALC_INT_ID(PIC_CENTRAL, ITC_PAR2_POS)
#endif

/* Debug communication channel (Multi-ICE) */
#ifdef ITC_CMTX_POS
  #define INT_DBGCOMMTX CALC_INT_ID(PIC_CENTRAL,ITC_CMTX_POS)
#endif
#ifdef ITC_CMRX_POS
  #define INT_DBGCOMMRX CALC_INT_ID(PIC_CENTRAL,ITC_CMRX_POS)
#endif

/* I2C in the main interrupt controller */
#ifdef ITC_I2C_POS
  /* For Wabash, the code supports one only IIC module. Both the old and 
      the new (Eric) IIC interfaces are present on the Chip. So we need
      to check for IIC_TYPE here to see which hw interface is used.*/
  #if (IIC_TYPE == IIC_TYPE_COLORADO)
    #define INT_I2C       CALC_INT_ID(PIC_CENTRAL,ITC_I2C_POS)
  #endif  
#endif  
  
/* Reserved bits (may be used by software to generate interrupts) */
#ifdef ITC_RESERVED3_POS
  #define INT_RESERVED3 CALC_INT_ID(PIC_CENTRAL, ITC_RESERVED3_POS)
#endif
#ifdef ITC_RESERVED4_POS
  #define INT_RESERVED4 CALC_INT_ID(PIC_CENTRAL, ITC_RESERVED4_POS)
#endif
#ifdef ITC_RESERVED5_POS
  #define INT_RESERVED5 CALC_INT_ID(PIC_CENTRAL, ITC_RESERVED5_POS)
#endif

/************************************************************/
/* Expansion register interrupts (if this register exists!) */
/************************************************************/
#ifdef ITC_EXPANSION_POS

  /* Debug communication channel (Multi-ICE) */
  #ifdef ITC_EXPANSION_CMTX_POS 
    #define INT_DBGCOMMTX CALC_INT_ID(PIC_EXPANSION, ITC_EXPANSION_CMTX_POS)
  #endif
  #ifdef ITC_EXPANSION_CMRX_POS 
    #define INT_DBGCOMMRX CALC_INT_ID(PIC_EXPANSION, ITC_EXPANSION_CMRX_POS)
  #endif
  
  #ifdef ITC_EXPANSION_I2C_0_POS
    #define INT_I2C0       CALC_INT_ID(PIC_EXPANSION, ITC_EXPANSION_I2C_0_POS)
    /* Alias this to INT_I2C for Chips using the new (Eric) I2C controller */
    #define INT_I2C INT_I2C0
  #endif
  
  #ifdef ITC_EXPANSION_I2C_1_POS
    #define INT_I2C1       CALC_INT_ID(PIC_EXPANSION, ITC_EXPANSION_I2C_1_POS)
  #endif
  
  #ifdef ITC_EXPANSION_DMD_POS  
    #define INT_QPSK_DEMOD CALC_INT_ID(PIC_EXPANSION, ITC_EXPANSION_DMD_POS)
  #endif
  
  #ifdef ITC_EXPANSION_GIR2_POS
    #define INT_GIR2       CALC_INT_ID(PIC_EXPANSION, ITC_EXPANSION_GIR2_POS)
  #endif
#endif /* defined ITC_EXPANSION_POS */

/***************************/
/* Timer interrupt sources */
/***************************/
#define INT_SYSTIMER  CALC_INT_ID(PIC_TIMERS, SYS_TIMER)
#define INT_WATCHDOG  CALC_INT_ID(PIC_TIMERS, WATCHDOG_TIMER)
#define INT_TICKTIMER CALC_INT_ID(PIC_TIMERS, 2)
#define INT_TIMER0    CALC_INT_ID(PIC_TIMERS, 0)
#define INT_TIMER1    CALC_INT_ID(PIC_TIMERS, 1)
#define INT_TIMER2    CALC_INT_ID(PIC_TIMERS, 2)
#define INT_TIMER3    CALC_INT_ID(PIC_TIMERS, 3)
#define INT_TIMER4    CALC_INT_ID(PIC_TIMERS, 4)
#define INT_TIMER5    CALC_INT_ID(PIC_TIMERS, 5)
#define INT_TIMER6    CALC_INT_ID(PIC_TIMERS, 6)
#define INT_TIMER7    CALC_INT_ID(PIC_TIMERS, 7)

/*********************************************************************/
/* On Colorado and Wabash, there are 3 banks of GPIO registers while */
/* on Brazos there are 4. While not all bits are populated in each   */
/* register, the software allows for this. GPIO pins are numbered    */
/* 0-31 in the main register bank at 0x30470000, 32-63 in the bank   */
/* at 0x30470100 and 64-95 in the bank at 0x30470200 and so on. Note */
/* that pin muxing means that many of the GPIOs defined here may be  */
/* being used for other functions.                                   */
/*********************************************************************/
#define INT_GPIO0    CALC_INT_ID(PIC_GPIO, 0x00)
#define INT_GPIO1    CALC_INT_ID(PIC_GPIO, 0x01)
#define INT_GPIO2    CALC_INT_ID(PIC_GPIO, 0x02)
#define INT_GPIO3    CALC_INT_ID(PIC_GPIO, 0x03)
#define INT_GPIO4    CALC_INT_ID(PIC_GPIO, 0x04)
#define INT_GPIO5    CALC_INT_ID(PIC_GPIO, 0x05)
#define INT_GPIO6    CALC_INT_ID(PIC_GPIO, 0x06)
#define INT_GPIO7    CALC_INT_ID(PIC_GPIO, 0x07)
#define INT_GPIO8    CALC_INT_ID(PIC_GPIO, 0x08)
#define INT_GPIO9    CALC_INT_ID(PIC_GPIO, 0x09)
#define INT_GPIO10   CALC_INT_ID(PIC_GPIO, 0x0A)
#define INT_GPIO11   CALC_INT_ID(PIC_GPIO, 0x0B)
#define INT_GPIO12   CALC_INT_ID(PIC_GPIO, 0x0C)
#define INT_GPIO13   CALC_INT_ID(PIC_GPIO, 0x0D)
#define INT_GPIO14   CALC_INT_ID(PIC_GPIO, 0x0E)
#define INT_GPIO15   CALC_INT_ID(PIC_GPIO, 0x0F)
#define INT_GPIO16   CALC_INT_ID(PIC_GPIO, 0x10)
#define INT_GPIO17   CALC_INT_ID(PIC_GPIO, 0x11)
#define INT_GPIO18   CALC_INT_ID(PIC_GPIO, 0x12)
#define INT_GPIO19   CALC_INT_ID(PIC_GPIO, 0x13)
#define INT_GPIO20   CALC_INT_ID(PIC_GPIO, 0x14)
#define INT_GPIO21   CALC_INT_ID(PIC_GPIO, 0x15)
#define INT_GPIO22   CALC_INT_ID(PIC_GPIO, 0x16)
#define INT_GPIO23   CALC_INT_ID(PIC_GPIO, 0x17)
#define INT_GPIO24   CALC_INT_ID(PIC_GPIO, 0x18)
#define INT_GPIO25   CALC_INT_ID(PIC_GPIO, 0x19)
#define INT_GPIO26   CALC_INT_ID(PIC_GPIO, 0x1A)
#define INT_GPIO27   CALC_INT_ID(PIC_GPIO, 0x1B)
#define INT_GPIO28   CALC_INT_ID(PIC_GPIO, 0x1C)
#define INT_GPIO29   CALC_INT_ID(PIC_GPIO, 0x1D)
#define INT_GPIO30   CALC_INT_ID(PIC_GPIO, 0x1E)
#define INT_GPIO31   CALC_INT_ID(PIC_GPIO, 0x1F)

#define INT_GPIO32   CALC_INT_ID(PIC_GPIO, 0x20)
#define INT_GPIO33   CALC_INT_ID(PIC_GPIO, 0x21)
#define INT_GPIO34   CALC_INT_ID(PIC_GPIO, 0x22)
#define INT_GPIO35   CALC_INT_ID(PIC_GPIO, 0x23)
#define INT_GPIO36   CALC_INT_ID(PIC_GPIO, 0x24)
#define INT_GPIO37   CALC_INT_ID(PIC_GPIO, 0x25)
#define INT_GPIO38   CALC_INT_ID(PIC_GPIO, 0x26)
#define INT_GPIO39   CALC_INT_ID(PIC_GPIO, 0x27)
#define INT_GPIO40   CALC_INT_ID(PIC_GPIO, 0x28)
#define INT_GPIO41   CALC_INT_ID(PIC_GPIO, 0x29)
#define INT_GPIO42   CALC_INT_ID(PIC_GPIO, 0x2A)
#define INT_GPIO43   CALC_INT_ID(PIC_GPIO, 0x2B)
#define INT_GPIO44   CALC_INT_ID(PIC_GPIO, 0x2C)
#define INT_GPIO45   CALC_INT_ID(PIC_GPIO, 0x2D)
#define INT_GPIO46   CALC_INT_ID(PIC_GPIO, 0x2E)
#define INT_GPIO47   CALC_INT_ID(PIC_GPIO, 0x2F)
#define INT_GPIO48   CALC_INT_ID(PIC_GPIO, 0x30)
#define INT_GPIO49   CALC_INT_ID(PIC_GPIO, 0x31)
#define INT_GPIO50   CALC_INT_ID(PIC_GPIO, 0x32)
#define INT_GPIO51   CALC_INT_ID(PIC_GPIO, 0x33)
#define INT_GPIO52   CALC_INT_ID(PIC_GPIO, 0x34)
#define INT_GPIO53   CALC_INT_ID(PIC_GPIO, 0x35)
#define INT_GPIO54   CALC_INT_ID(PIC_GPIO, 0x36)
#define INT_GPIO55   CALC_INT_ID(PIC_GPIO, 0x37)
#define INT_GPIO56   CALC_INT_ID(PIC_GPIO, 0x38)
#define INT_GPIO57   CALC_INT_ID(PIC_GPIO, 0x39)
#define INT_GPIO58   CALC_INT_ID(PIC_GPIO, 0x3A)
#define INT_GPIO59   CALC_INT_ID(PIC_GPIO, 0x3B)
#define INT_GPIO60   CALC_INT_ID(PIC_GPIO, 0x3C)
#define INT_GPIO61   CALC_INT_ID(PIC_GPIO, 0x3D)
#define INT_GPIO62   CALC_INT_ID(PIC_GPIO, 0x3E)
#define INT_GPIO63   CALC_INT_ID(PIC_GPIO, 0x3F)

#define INT_GPIO64   CALC_INT_ID(PIC_GPIO, 0x40)
#define INT_GPIO65   CALC_INT_ID(PIC_GPIO, 0x41)
#define INT_GPIO66   CALC_INT_ID(PIC_GPIO, 0x42)
#define INT_GPIO67   CALC_INT_ID(PIC_GPIO, 0x43)
#define INT_GPIO68   CALC_INT_ID(PIC_GPIO, 0x44)
#define INT_GPIO69   CALC_INT_ID(PIC_GPIO, 0x45)
#define INT_GPIO70   CALC_INT_ID(PIC_GPIO, 0x46)
#define INT_GPIO71   CALC_INT_ID(PIC_GPIO, 0x47)
#define INT_GPIO72   CALC_INT_ID(PIC_GPIO, 0x48)
#define INT_GPIO73   CALC_INT_ID(PIC_GPIO, 0x49)
#define INT_GPIO74   CALC_INT_ID(PIC_GPIO, 0x4A)
#define INT_GPIO75   CALC_INT_ID(PIC_GPIO, 0x4B)
#define INT_GPIO76   CALC_INT_ID(PIC_GPIO, 0x4C)
#define INT_GPIO77   CALC_INT_ID(PIC_GPIO, 0x4D)
#define INT_GPIO78   CALC_INT_ID(PIC_GPIO, 0x4E)
#define INT_GPIO79   CALC_INT_ID(PIC_GPIO, 0x4F)
#define INT_GPIO80   CALC_INT_ID(PIC_GPIO, 0x50)
#define INT_GPIO81   CALC_INT_ID(PIC_GPIO, 0x51)
#define INT_GPIO82   CALC_INT_ID(PIC_GPIO, 0x52)
#define INT_GPIO83   CALC_INT_ID(PIC_GPIO, 0x53)
#define INT_GPIO84   CALC_INT_ID(PIC_GPIO, 0x54)
#define INT_GPIO85   CALC_INT_ID(PIC_GPIO, 0x55)
#define INT_GPIO86   CALC_INT_ID(PIC_GPIO, 0x56)
#define INT_GPIO87   CALC_INT_ID(PIC_GPIO, 0x57)
#define INT_GPIO88   CALC_INT_ID(PIC_GPIO, 0x58)
#define INT_GPIO89   CALC_INT_ID(PIC_GPIO, 0x59)
#define INT_GPIO90   CALC_INT_ID(PIC_GPIO, 0x5A)
#define INT_GPIO91   CALC_INT_ID(PIC_GPIO, 0x5B)
#define INT_GPIO92   CALC_INT_ID(PIC_GPIO, 0x5C)
#define INT_GPIO93   CALC_INT_ID(PIC_GPIO, 0x5D)
#define INT_GPIO94   CALC_INT_ID(PIC_GPIO, 0x5E)
#define INT_GPIO95   CALC_INT_ID(PIC_GPIO, 0x5F)

#if (NUM_GPI_BANKS > 3)
#define INT_GPIO96   CALC_INT_ID(PIC_GPIO, 0x60)
#define INT_GPIO97   CALC_INT_ID(PIC_GPIO, 0x61)
#define INT_GPIO98   CALC_INT_ID(PIC_GPIO, 0x62)
#define INT_GPIO99   CALC_INT_ID(PIC_GPIO, 0x63)
#define INT_GPIO100  CALC_INT_ID(PIC_GPIO, 0x64)
#define INT_GPIO101  CALC_INT_ID(PIC_GPIO, 0x65)
#define INT_GPIO102  CALC_INT_ID(PIC_GPIO, 0x66)
#define INT_GPIO103  CALC_INT_ID(PIC_GPIO, 0x67)
#define INT_GPIO104  CALC_INT_ID(PIC_GPIO, 0x68)
#define INT_GPIO105  CALC_INT_ID(PIC_GPIO, 0x69)
#define INT_GPIO106  CALC_INT_ID(PIC_GPIO, 0x6A)
#define INT_GPIO107  CALC_INT_ID(PIC_GPIO, 0x6B)
#define INT_GPIO108  CALC_INT_ID(PIC_GPIO, 0x6C)
#define INT_GPIO109  CALC_INT_ID(PIC_GPIO, 0x6D)
#define INT_GPIO110  CALC_INT_ID(PIC_GPIO, 0x6E)
#define INT_GPIO111  CALC_INT_ID(PIC_GPIO, 0x6F)
#define INT_GPIO112  CALC_INT_ID(PIC_GPIO, 0x70)
#define INT_GPIO113  CALC_INT_ID(PIC_GPIO, 0x71)
#define INT_GPIO114  CALC_INT_ID(PIC_GPIO, 0x72)
#define INT_GPIO115  CALC_INT_ID(PIC_GPIO, 0x73)
#define INT_GPIO116  CALC_INT_ID(PIC_GPIO, 0x74)
#define INT_GPIO117  CALC_INT_ID(PIC_GPIO, 0x75)
#define INT_GPIO118  CALC_INT_ID(PIC_GPIO, 0x76)
#define INT_GPIO119  CALC_INT_ID(PIC_GPIO, 0x77)
#define INT_GPIO120  CALC_INT_ID(PIC_GPIO, 0x78)
#define INT_GPIO121  CALC_INT_ID(PIC_GPIO, 0x79)
#define INT_GPIO122  CALC_INT_ID(PIC_GPIO, 0x7A)
#define INT_GPIO123  CALC_INT_ID(PIC_GPIO, 0x7B)
#define INT_GPIO124  CALC_INT_ID(PIC_GPIO, 0x7C)
#define INT_GPIO125  CALC_INT_ID(PIC_GPIO, 0x7D)
#define INT_GPIO126  CALC_INT_ID(PIC_GPIO, 0x7E)
#define INT_GPIO127  CALC_INT_ID(PIC_GPIO, 0x7F)
#endif
#define INT_INVALID  CALC_INT_ID(0, 33)

#endif /* INT_ID_H_ */

/****************************************************************************
 * Modifications:
 * $Log: 
 *  1    mpeg      1.0         3/1/04 3:02:13 PM      Dave Wilson     CR(s) 
 *        8484 : Interrupt source identifiers (Edwards header file)
 * $
 *
 ****************************************************************************/

