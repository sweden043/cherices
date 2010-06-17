/****************************************************************************/
/*                    Conexant Systems Inc - CN8600                         */
/****************************************************************************/
/*                                                                          */
/* Filename:           HWLIB.H                                              */
/*                                                                          */
/* Description:        Public header file for low level function library    */
/*                     used by the KAL or called directly by non-KAL apps.  */
/*                                                                          */
/* Author:             Dave Wilson                                          */
/*                                                                          */
/* Copyright Conexant Systems Inc, 1999-2003                                */
/* All Rights Reserved.                                                     */
/*                                                                          */
/****************************************************************************/
/* $Header: hwlib.h, 46, 5/6/04 5:24:49 PM, Miles Bintz$
 ****************************************************************************/
#ifndef _HWLIB_H_
#define _HWLIB_H_

/*******************************************************************************/
/* Note: Functions in this module do not make use of any RTOS services. Some   */
/* should be semaphore-protected to prevent potential problems caused by       */
/* global data areas being accessed from different tasks simultaneously but    */
/* this synchronisation is left to the caller. Functions which may be affected */
/* in this way are marked below.                                               */
/*******************************************************************************/

/******************************************/
/* Type definitions required within HWLIB */
/******************************************/
#include <basetype.h>
#include "hwfuncs.h"

typedef u_int32 timer_id_t;
typedef u_int32 tick_id_t;
typedef bool crit_state;

typedef int  (*PFNISR)(u_int32, bool, void *);
typedef void (*PFNTIMERC)(timer_id_t, void *);
typedef void (*PFNTICKC)(tick_id_t, void *);
typedef void (*PFNTIMER)(timer_id_t);

/* Maximum length of an OS object name string */
#define MAX_OBJ_NAME_LENGTH 15
typedef struct _tick_info_t 
{
   bool bRunning; 
   u_int32 flags;
   PFNTIMERC pfnCallback;
   void *pUserData;
   const char name[MAX_OBJ_NAME_LENGTH];
} tick_info_t;


/****************************/
/* Initialisation Functions */
/****************************/
bool hwlib_int_initialise(void);
bool hwlib_timer_initialise(u_int32 period_us, bool bStart);
void hwlib_watchdog_enable(void);

/******************************/
/* Critical Section Functions */
/******************************/
bool hw_critical_section_begin( u_int32 ra );
void critical_section_end( bool previous_state);
void only_at_interrupt_safe( void );
u_int32 critical_section_begin( void );
bool not_interrupt_safe( void );

/* We do not include the critical section check functions if building for OpenTV  */
/* since, despite it's spec stating otherwise, it frequently makes KAL calls from */
/* within critical sections.                                                      */
#if (defined DEBUG) && (!defined OPENTV)
  #ifdef BAD_CRITICAL_SECTION_RETURN_CHECK
    #define not_from_critical_section asm_not_from_critical_section
  #else
    void c_not_from_critical_section(u_int32 uReturnAddress);
    #define not_from_critical_section() c_not_from_critical_section(0xDEADBEEF);
  #endif
#else
  #define not_from_critical_section  if (0) ((int (*)(void)) 0)
#endif

/**********************************/
/**********************************/
/* Interrupt Service Routine APIs */
/**********************************/
/**********************************/

int int_register_isr(u_int32 dwIntID,
                           PFNISR  pfnHandler,
                           bool    bFIQ,
                           bool    bInvert,
                           PFNISR *pfnChain);
int int_enable(u_int32 dwIntID);
int int_disable(u_int32 dwIntID);

/**********************/
/**********************/
/** Helper Functions **/
/**********************/
/**********************/
u_int32 *GetStackPtr(void);


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

/**********************/
/* Timer Service APIs */
/**********************/

timer_id_t hwtimer_create(PFNTIMERC pfnCallback, void * pUserData, const char *name);
int        hwtimer_destroy(timer_id_t timer);
int        hwtimer_set(timer_id_t timer, u_int32 period_us, bool bOneShot);
int        hwtimer_start(timer_id_t);
int        hwtimer_stop(timer_id_t);

int        hwtimer_sys_set(u_int32 period_us);
int        hwtimer_sys_start(void);
int        hwtimer_sys_stop(void);
u_int32    hwtimer_sys_get_rate(void);
int        hwtimer_sys_register_client(PFNTIMERC pfnSysFunc, int iIndex, void *pArg);
int        hwtimer_sys_remove_client(int iIndex);
u_int32    get_system_time( void );
u_int32    get_system_time_us( void );

#define MAX_SYSTIMER_CLIENTS 4
#define SYSTIMER_CLIENT_RTOS 3
#define SYSTIMER_CLIENT_KAL  2
#define SYSTIMER_CLIENT_APP1 1
#define SYSTIMER_CLIENT_APP2 0

/*********************/
/* Tick Counter APIs */
/*********************/

tick_id_t  tick_create(PFNTICKC pfnCallback, void * pUserData, const char *name);
int        tick_destroy(tick_id_t timer);
int        tick_set(tick_id_t timer, u_int32 period_ms, bool bOneShot);
int        tick_start(tick_id_t);
int        tick_stop(tick_id_t);
int        tick_get_info(tick_id_t timer, tick_info_t *info);
int        tick_id_from_name(const char *name, tick_id_t *timer);

/**************************************************/
/* Miscellaneous Shared resource access functions */
/**************************************************/
void   read_board_and_vendor_codes(u_int8 *pBoard, u_int8 *pVendor);
void   read_chip_id_and_revision(u_int32 *pChip, u_int8 *pRev);

void   set_gpio_int_edge(unsigned int  GPIO_bit, int  polarity);   
#define RISING  0xaa
#define FALLING 0x55

/*****************************/
/* Caching Control Functions */
/*****************************/
#define     cache_flush_i       FlushICache
void FlushICache(void);

#define     cache_disable_i     DisableICache
void DisableICache(void);

#define     cache_enable_i      EnableICache
void EnableICache(void);

#define     cache_disable_d     DisableDCache
void DisableDCache(void);

#define     cache_flush_d       FlushDCache
void FlushDCache(void);

#define     cache_clean_d       CleanDCache
void CleanDCache(void);

#define     cache_enable_d      EnableDCache
void EnableDCache(void);

#define     cache_drain_d       DrainWriteBuffer
void DrainWriteBuffer(void);

#if (CPU_TYPE==CPU_ARM920T) || (CPU_TYPE==AUTOSENSE)
void CleanDCacheMVA(u_int32 MVA);
void CleanAndInvalDCacheMVA(u_int32 MVA);
#endif

#if (CPU_TYPE==CPU_ARM940T) || (CPU_TYPE==AUTOSENSE)
void EnableDCacheForRegion(u_int32 Region);
bool DisableDCacheForRegion(u_int32 Region);
void EnableICacheForRegion(u_int32 Region);
bool DisableICacheForRegion(u_int32 Region);
#endif

/*
 * This function is useful when sharing memory between the ARM920T core
 * and a DMA device (e.g. GXA).  While this function is implemented on
 * an ARM940T core, its use is *not* recommended for performance reasons.
 * On an ARM920T core, this function utilizes the ARM920 flush & clean
 * using MVA (modified virtual address) MMU operations.  There are no
 * such operations available on the ARM940T.  Therefore, if this is
 * called using an ARM940T core, the entire DCcache is cleaned & flushed
 * which will likely lead to performance issues.
 */
void FlushAndInvalDCacheRegion(u_int8 *addr, u_int32 size);

/***********************************************/
/* Additional functions required by OpenTV 1.2 */
/***********************************************/
/*#ifdef OPENTV_12*/
#if RTOS == PSOS
/* This is a vxWorks system call so only define it for pSOS */
void reboot(void);
#endif
void  reboot_IRD(void);
/*#endif*/

bool in_pci_mode( void );
int pci_get_bar( unsigned int dev_ven, unsigned int bar_no, u_int32 *bar );

#endif /* _HWLIB_H_ */
/****************************************************************************
 * Modifications:
 * $Log: 
 *  46   mpeg      1.45        5/6/04 5:24:49 PM      Miles Bintz     CR(s) 
 *        9016 9017 : added prototype for in_pci_mode
 *  45   mpeg      1.44        6/5/03 2:25:30 PM      Dave Wilson     SCR(s) 
 *        6716 :
 *        Remove critical section checking for all OpenTV builds. OpenTV 
 *        insists on
 *        calling task_id from within a critical section and this messes things
 *         up
 *        badly if the critical section check is in place.
 *        
 *  44   mpeg      1.43        6/3/03 5:14:24 PM      Dave Wilson     SCR(s) 
 *        6652 6651 :
 *        Added optionally compiled code to allow measurement of the longest 
 *        ISRs in
 *        a given application. By default, this is compiled out. Define 
 *        ISR_TIMING_CHECK
 *        to include it.
 *        
 *  43   mpeg      1.42        5/27/03 12:19:48 PM    Tim White       SCR(s) 
 *        6587 :
 *        Add region flush & invalidate function which is useful for sharing 
 *        memory
 *        between the ARM920T core and a DMA device (e.g. GXA).
 *        
 *        
 *  42   mpeg      1.41        4/30/03 4:50:10 PM     Billy Jackman   SCR(s) 
 *        6113 :
 *        Modified conditional inclusion of some external references to include
 *         the
 *        case of CPU_TYPE=AUTOSENSE, not just CPU_ARM920T and CPU_ARM940T.
 *        Eliminated double slash comments.
 *        
 *  41   mpeg      1.40        2/5/03 6:31:56 PM      Brendan Donahe  SCR(s) 
 *        5421 :
 *        Changed HW bit definitions for INT_DBGCOMMRX to be derived from CMRX 
 *        instead
 *        of CMTX (typo).
 *        
 *        
 *  40   mpeg      1.39        1/31/03 5:43:28 PM     Dave Moore      SCR(s) 
 *        5375 :
 *        Removed FlushCachesC(). Added cache_drain_d and removed 
 *        cache_flush_all.
 *        
 *        
 *  39   mpeg      1.38        12/20/02 9:51:52 PM    Tim Ross        SCR(s) 
 *        5206 :
 *        Added definitions for all functions in cache.s.
 *        
 *  38   mpeg      1.37        12/18/02 12:49:54 PM   Senthil Veluswamy SCR(s) 
 *        5190 :
 *        Modified setting of the IIC Interrupt defines and updated comments.
 *        
 *  37   mpeg      1.36        12/10/02 1:35:20 PM    Dave Wilson     SCR(s) 
 *        5091 :
 *        Changed the way interrupt identifiers are defined to make it easier 
 *        to
 *        change in future. Also removed the hardcoded bit position information
 *         and
 *        replaced this with bit positions from the relevant chip header file.
 *        
 *  36   mpeg      1.35        10/30/02 10:01:24 AM   Dave Moore      SCR(s) 
 *        4833 :
 *        backed out previous change
 *        
 *        
 *  35   mpeg      1.34        10/29/02 5:28:32 PM    Dave Moore      SCR(s) 
 *        4833 :
 *        removed prototypes for critical_section_begin and not_interrupt_safe
 *        (they are now in hwlib\critsec.h).
 *        
 *  34   mpeg      1.33        10/1/02 12:18:28 PM    Miles Bintz     SCR(s) 
 *        4644 :
 *        added pci_get_bar function prototype to hwlib.h
 *        
 *        
 *  33   mpeg      1.32        10/1/02 10:52:30 AM    Matt Korte      SCR(s) 
 *        4724 :
 *        Fixed warnings
 *        
 *  32   mpeg      1.31        8/30/02 6:40:46 PM     Senthil Veluswamy SCR(s) 
 *        4502 :
 *        Defines for using the New IIC interface. Renamed INT_NEW_I2C back to 
 *        INT_I2C
 *        
 *  31   mpeg      1.30        8/30/02 3:05:46 PM     Larry Wang      SCR(s) 
 *        4499 :
 *        Re-define INT_DBGCOMMTX and INT_DBGCOMMRX as interrupt expansion for 
 *        Wabash.  Define INT_NEW_I2C for Wabash.
 *        
 *  30   mpeg      1.29        2/8/02 11:11:50 AM     Bobby Bradford  SCR(s) 
 *        3131 :
 *        Added new prototype(s) ... is_timer_pending(), and 
 *        get_system_time_us() to the hardware library functions.  These two 
 *        functions allow us to get the system time, with microsecond 
 *        resolution.  These functions were added to support some FLIPPER 
 *        testing, but could be useful for other applications as well.
 *        
 *  29   mpeg      1.28        9/8/01 8:24:38 AM      Dave Wilson     SCR(s) 
 *        1999 1998 :
 *        Added prorotype for hwlib_enable_watchdog
 *        
 *  28   mpeg      1.27        4/20/01 10:11:48 AM    Tim White       DCS#1687,
 *         DCS#1747, DCS#1748 - Add pSOS task switch prevention, move OS 
 *        specific
 *        functionality out of hwlib.c and into psoskal.c/nupkal.c, and update 
 *        critical
 *        section debugging function for both release and debug without using 
 *        assembly code.
 *        
 *  27   mpeg      1.26        4/11/01 6:54:18 PM     Amy Pratt       DCS914 
 *        Removed Neches support.
 *        
 *  26   mpeg      1.25        3/30/01 9:33:30 AM     Steve Glennon   DCS# 
 *        1504/1505 Changed prototype of not_interrupt_safe to return bool
 *        
 *  25   mpeg      1.24        3/23/01 12:06:50 PM    Steve Glennon   
 *        DCS#1475/1476
 *        Added functions "not_interrupt_safe" and "only_at_interrupt_safe"
 *        
 *  24   mpeg      1.23        7/5/00 3:41:28 PM      Joe Kroesche    added 
 *        prototype for get_system_time function to timer API
 *        
 *  23   mpeg      1.22        6/23/00 5:56:56 PM     Senthil Veluswamy moved 
 *        MAX_OBJ_NAME_LENGTH here from kal.h
 *        
 *  22   mpeg      1.21        6/23/00 5:46:56 PM     Senthil Veluswamy changed
 *         member name in struct tick_info_t to an array from pointer
 *        
 *  21   mpeg      1.20        6/1/00 6:39:22 PM      Ray Mack        forgot 
 *        the polarity defines for GPIO function
 *        
 *  20   mpeg      1.19        6/1/00 6:35:58 PM      Ray Mack        added 
 *        GPIO interrupt polarity function
 *        
 *  19   mpeg      1.18        5/1/00 7:15:38 PM      Tim Ross        Changed 
 *        the names of the cache functions referenced by the KAL.
 *        
 *  18   mpeg      1.17        4/12/00 2:50:34 PM     Senthil Veluswamy moved a
 *         few cache functions from STARTUP and mapped the new
 *        - Pace routines to them
 *        
 *  17   mpeg      1.16        4/6/00 5:05:52 PM      Tim Ross        Added 
 *        INT_USB and INT_PAW2 for Colorado.
 *        
 *  16   mpeg      1.15        4/3/00 11:27:46 AM     Tim Ross        Added KAL
 *         extensions.
 *        
 *  15   mpeg      1.14        3/10/00 5:27:06 PM     Dave Wilson     Added 
 *        pSOS-only reboot function
 *        
 *  14   mpeg      1.13        1/19/00 11:29:00 AM    Senthil Veluswamy renamed
 *         INT_ICAM to INT_NDSC. This is the NDS CA SCard Interrupt
 *        
 *  13   mpeg      1.12        1/18/00 10:08:30 AM    Dave Wilson     Added 
 *        reboot function for OpenTV 1.2
 *        
 *  12   mpeg      1.11        1/11/00 3:46:14 PM     Tim Ross        Added 
 *        DBGCOMMTX and DBGCOMMRX interrupt bit definitions.
 *        
 *  11   mpeg      1.10        12/7/99 3:54:14 PM     Ismail Mustafa  Added new
 *         PAWSER and ICAM interrupts.
 *        
 *  10   mpeg      1.9         11/18/99 8:06:42 PM    Dave Wilson     Reworked 
 *        system timer handling architecture to keep in inside HWLIB.
 *        
 *  9    mpeg      1.8         11/4/99 3:07:24 PM     Dave Wilson     Removed 
 *        all Sabine and PID board special cases.
 *        Added INT_DAA definition.
 *        
 *  8    mpeg      1.7         10/29/99 10:58:50 AM   Dave Wilson     Moved 
 *        GPIO extender APIs from HWLIB to IIC.
 *        
 *  7    mpeg      1.6         10/28/99 12:54:52 PM   Dave Wilson     Changed 
 *        all timer_ APIs to hwtimer_
 *        
 *  6    mpeg      1.5         10/6/99 4:44:26 PM     Dave Wilson     Added 
 *        read_chip_id_and_revision prototype
 *        
 *  5    mpeg      1.4         10/6/99 2:43:28 PM     Anzhi Chen      Added 
 *        INT_TICKTIMER for SABINE.
 *        
 *  4    mpeg      1.3         9/28/99 1:57:04 PM     Dave Wilson     Added 
 *        tick timer functions
 *        
 *  3    mpeg      1.2         9/23/99 3:27:36 PM     Dave Wilson     Moved top
 *         level timer functions from kal header.
 *        Added tick function prototypes.
 *        
 *  2    mpeg      1.1         9/14/99 3:08:26 PM     Dave Wilson     Changes 
 *        due to splitting KAL into PSOSKAL and HWLIB components.
 *        
 *  1    mpeg      1.0         9/10/99 6:05:50 PM     Dave Wilson     
 * $
 * 
 *    Rev 1.44   05 Jun 2003 13:25:30   dawilson
 * SCR(s) 6716 :
 * Remove critical section checking for all OpenTV builds. OpenTV insists on
 * calling task_id from within a critical section and this messes things up
 * badly if the critical section check is in place.
 * 
 *    Rev 1.43   03 Jun 2003 16:14:24   dawilson
 * SCR(s) 6652 6651 :
 * Added optionally compiled code to allow measurement of the longest ISRs in
 * a given application. By default, this is compiled out. Define ISR_TIMING_CHECK
 * to include it.
 * 
 *    Rev 1.42   27 May 2003 11:19:48   whiteth
 * SCR(s) 6587 :
 * Add region flush & invalidate function which is useful for sharing memory
 * between the ARM920T core and a DMA device (e.g. GXA).
 * 
 * 
 *    Rev 1.41   30 Apr 2003 15:50:10   jackmaw
 * SCR(s) 6113 :
 * Modified conditional inclusion of some external references to include the
 * case of CPU_TYPE=AUTOSENSE, not just CPU_ARM920T and CPU_ARM940T.
 * Eliminated double slash comments.
 * 
 *    Rev 1.40   05 Feb 2003 18:31:56   donaheb
 * SCR(s) 5421 :
 * Changed HW bit definitions for INT_DBGCOMMRX to be derived from CMRX instead
 * of CMTX (typo).
 * 
 * 
 *    Rev 1.39   31 Jan 2003 17:43:28   mooreda
 * SCR(s) 5375 :
 * Removed FlushCachesC(). Added cache_drain_d and removed cache_flush_all.
 * 
 * 
 *    Rev 1.38   20 Dec 2002 21:51:52   rossst
 * SCR(s) 5206 :
 * Added definitions for all functions in cache.s.
 * 
 *    Rev 1.37   18 Dec 2002 12:49:54   velusws
 * SCR(s) 5190 :
 * Modified setting of the IIC Interrupt defines and updated comments.
 * 
 *    Rev 1.36   10 Dec 2002 13:35:20   dawilson
 * SCR(s) 5091 :
 * Changed the way interrupt identifiers are defined to make it easier to
 * change in future. Also removed the hardcoded bit position information and
 * replaced this with bit positions from the relevant chip header file.
 * 
 *    Rev 1.35   30 Oct 2002 10:01:24   mooreda
 * SCR(s) 4833 :
 * backed out previous change
 * 
 * 
 *    Rev 1.34   29 Oct 2002 17:28:32   mooreda
 * SCR(s) 4833 :
 * removed prototypes for critical_section_begin and not_interrupt_safe
 * (they are now in hwlib\critsec.h).
 * 
 *    Rev 1.33   01 Oct 2002 11:18:28   bintzmf
 * SCR(s) 4644 :
 * added pci_get_bar function prototype to hwlib.h
 * 
 * 
 *    Rev 1.32   01 Oct 2002 09:52:30   kortemw
 * SCR(s) 4724 :
 * Fixed warnings
 ****************************************************************************/

