/****************************************************************************/
/*                   CONEXANT PROPRIETARY AND CONFIDENTIAL                  */
/*                       SOFTWARE FILE/MODULE HEADER                        */
/*          Conexant Systems Inc. (c) 1998, 1999, 2000, 2001, 2002          */
/*                              Austin, TX                                  */
/*                         All Rights Reserved                              */
/****************************************************************************/
/*                                                                         
 * Filename:           HWFUNCS.C                                           
 *                                                                         
 * Description:        Low level functions used by the Sabine KAL. This    
 *                     isolates the main KAL from compilation changes      
 *                     needed to build versions for Sabine and PID7T.      
 *                                                                         
 * Author:             Dave Wilson                                         
 *                                                                         
 * Copyright Rockwell Semiconductor Systems, 1997                          
 * All Rights Reserved.                                                    
 *                                                                         
 ****************************************************************************/
/* $Header: hwfuncs.c, 52, 9/2/03 7:05:18 PM, Joe Kroesche$
 ****************************************************************************/

#include "stbcfg.h"
#include "hwlib.h"
#include "basetype.h"
#include "kal.h"
#include "hwlibint.h"
#include "critsec.h"
#include "hwfuncs.h"
#include "globals.h"
#include "retcodes.h"

#if RTOS == NUP
#include "nup.h"
#endif

extern interrupt_vector vectors[NUM_PICS][NUM_INTS_PER_PIC];
extern u_int32 gInterrupt_State;

LPREG lpIntEnable    = (LPREG)ITC_INTENABLE_REG;
LPREG lpIntStatus    = (LPREG)ITC_INTRIRQ_REG;
LPREG lpIntClear     = (LPREG)ITC_INTSTATCLR_REG;
#if (INTEXP_TYPE != NOT_PRESENT)
LPREG lpIntExpEnable = (LPREG)ITC_EXPENABLE_REG;
LPREG lpIntExpStatus = (LPREG)ITC_EXPSTATSET_REG;
LPREG lpIntExpClear  = (LPREG)ITC_EXPSTATCLR_REG;
#if RTOS == NUP
static unsigned long intExpEnable = 0;
#endif
#endif
LPREG lpIntGPIEnable = (LPREG)GPI_INT_ENABLE_REG;
LPREG lpIntGPIStatus = (LPREG)GPI_INT_REG;

LPREG lpTimerIntStatus = (LPREG)TIM_INT_STATUS_REG;

extern u_int32 split_gpio_int(u_int32 iInt, u_int32 *uBank, u_int32 *uBitNum);

/**********************************/
/**********************************/
/** Internal Function Prototypes **/
/**********************************/
/**********************************/
void start_hw_timer_reset(u_int32 uTimer, bool bReset);

/**********************************************************/
/* Reset the timer interrupt output during ISR processing */
/**********************************************************/
void clear_timer_int(u_int32 uTimer)
{
   /* For Sabine, writing to any of the registers associated with a */
   /* given timer will reset the interrupt. Here we read the limit  */
   /* and write it back again to accomplish this.                   */

   LPREG       lpLimit;
   HW_DWORD    LimitVal;

   lpLimit = (LPREG)((char *)TIM_LIMIT_REG + (TIM_BANK_SIZE * uTimer));
   LimitVal = *lpLimit;
   *lpLimit = LimitVal;
}
/************************************************/
/* mask out a given interrupt source in the PIC */
/************************************************/
void enable_pic_interrupt(u_int32 uInt) {
    LPREG pic_int_mask;
    u_int32 oldmask;

    pic_int_mask = (LPREG)ITC_INTENABLE_REG;
    
    /* we only deal with the central PIC, we'll expand later if necessary */
    if (uInt < 33) {
        oldmask = *pic_int_mask;
        *pic_int_mask = oldmask | ((u_int32)0x00000001 << uInt);
    }
}

void disable_pic_interrupt(u_int32 uInt) {
    LPREG pic_int_mask;
    u_int32 oldmask;

    pic_int_mask = (LPREG)ITC_INTENABLE_REG;
    
    /* we only deal with the central PIC, we'll expand later if necessary */
    if (uInt < 33) {
        oldmask = *pic_int_mask;
        *pic_int_mask = oldmask & ~((u_int32)0x00000001 << uInt);
    }
}

/*********************************************/
/* Clear a given interrupt source in the PIC */
/*********************************************/
void clear_pic_interrupt(u_int32 uPic, u_int32 uInt)
{
   u_int32 uBitPos;
   u_int32 uBank;
   u_int32 uTemp;

   /* Here the PIC number is not actually a PIC number as such (it used to   */
   /* be but then someone changed the architecture under me). It is actually */
   /* an indication of which tier in the interrupt stack handles this        */
   /* particular interrupt. PIC 0 is the main interrupt status register, PIC */
   /* 1 is the expansion register (if present), PIC 2 is the timer register, */
   /* and PIC 3 is the GPIO register.                                        */

   switch(uPic)
   {
     case PIC_CENTRAL:
       /* The easy case - we just muck with the main PIC */
       uBitPos = 1 << uInt;
       break;

#if (INTEXP_TYPE != NOT_PRESENT)
     case PIC_EXPANSION:
       /* Clear appropriate bit in expansion PIC */
       uBitPos = 1 << uInt;
       *lpIntExpClear = uBitPos;
       /* Read back the value we just wrote to make sure the write has happened */
       uTemp = *lpIntExpClear;

       /* Clear bit in main PIC only if all expansion PIC bits are clear */
       if (*lpIntExpStatus == 0)
    *lpIntClear = ITC_EXPANSION;

       return;
#endif

     case PIC_TIMERS:
       /* This is a no-op since the timer bit in the PIC is not latched */
       /* and we have another function to clear the appropriate timer's */
       /* interrupt.                                                    */
       return;

     case PIC_GPIO:
       split_gpio_int(uInt, &uBank, &uBitPos);
       CLEAR_GPIO_INT_BANK(uBank, uBitPos);
       return;

     default:
       return;
   }

   /* Clear the appropriate bit in the PIC */
   *lpIntClear = uBitPos;
   /* Read back the value we just wrote to make sure the write has happened */
   uTemp = *lpIntClear;
}


/*****************************************************************/
/* Set the period of a given hardware timer (but don't start it) */
/*****************************************************************/
u_int32 set_hw_timer(u_int32 uTimer, u_int32 uPerioduS)
{
   LPREG       lpLimit;
   LPREG       lpMode;
   LPREG       lpValue;
   HW_DWORD    OldMode;
   u_int32     uNewVal;

   /************************************************************************/
   /* WARNING: Function called from within a critical section! Do not add  */
   /*          trace or any other RTOS or C runtime function calls here!!! */
   /************************************************************************/
   
   /* Sabine has 8, 32 bit timers */
   if(uTimer > 7 )
     return(0);

   /* Which timer are we dealing with ? */
   lpLimit = (LPREG)((char *)TIM_LIMIT_REG + (TIM_BANK_SIZE * uTimer));
   lpMode  = (LPREG)((char *)TIM_MODE_REG  + (TIM_BANK_SIZE * uTimer));
   lpValue = (LPREG)((char *)TIM_VALUE_REG + (TIM_BANK_SIZE * uTimer));

   OldMode = *lpMode;

   /* Ensure the timer is stopped before we muck with it */
   CNXT_SET_VAL(lpMode, TIM_ENABLE_MASK, 0); 

   /* Set the timeout value (using 2 different calculation orders to try */
   /* to prevent over/underflow. This assumes a maximum SYSCLKFREQ of    */
   /* 21474KHz.                                                          */

   if(uPerioduS > TIMER_ALGORITHM_MAX)
     uNewVal = (uPerioduS/1000)*SYSCLKFREQ;
   else
     uNewVal = (uPerioduS*SYSCLKFREQ)/1000;

   if (uNewVal)
   {
     *lpLimit = uNewVal;
     *lpValue = 0;
   }

   /* Leave the timer as we found it */
   CNXT_SET(lpMode, TIM_ENABLE_MASK, OldMode);

   /* Return the value for the period register */

   return(uNewVal);
}

/*****************************************************************/
/* Set the period of a given hardware timer (but don't start it) */
/*****************************************************************/
u_int32 set_hw_timer_native(u_int32 uTimer, u_int32 uLimit)
{
   LPREG       lpLimit;
   LPREG       lpMode;
   LPREG       lpValue;
   HW_DWORD    OldMode;

   /* Sabine has 8, 32 bit timers */
   if(uTimer > 7 )
     return(0);

   /* Which timer are we dealing with ? */
   lpLimit = (LPREG)((char *)TIM_LIMIT_REG + (TIM_BANK_SIZE * uTimer));
   lpMode  = (LPREG)((char *)TIM_MODE_REG  + (TIM_BANK_SIZE * uTimer));
   lpValue = (LPREG)((char *)TIM_VALUE_REG + (TIM_BANK_SIZE * uTimer));

   OldMode = *lpMode;

   /* Ensure the timer is stopped before we muck with it */
   CNXT_SET_VAL(lpMode, TIM_ENABLE_MASK, 0); 


   *lpLimit = uLimit;
   *lpValue = 0;

   /* Leave the timer as we found it */
   CNXT_SET(lpMode, TIM_ENABLE_MASK, OldMode);

   /* Return the value for the period register */

   return(uLimit);
}

/********************************/
/* Start a given hardware timer */
/********************************/
void start_hw_timer(u_int32 uTimer)
{
  /************************************************************************/
  /* WARNING: Function called from within a critical section! Do not add  */
  /*          trace or any other RTOS or C runtime function calls here!!! */
  /************************************************************************/
  start_hw_timer_reset(uTimer, TRUE);
}

/*************************************************************************************/
/* Start a given hardware timer in a mode which does not reset to zero after timeout */
/*************************************************************************************/
void start_hw_timer_nowrap(u_int32 uTimer)
{
  start_hw_timer_reset(uTimer, FALSE);
}

/**************************************************************************************/
/* Start a given hardware timer with control over whether or not it resets on timeout */
/**************************************************************************************/
void start_hw_timer_reset(u_int32 uTimer, bool bReset)
{
   LPREG       lpMode;
   LPREG       lpValue;

   /************************************************************************/
   /* WARNING: Function called from within a critical section! Do not add  */
   /*          trace or any other RTOS or C runtime function calls here!!! */
   /************************************************************************/
   
   if(uTimer > 7)
     return;

   /* Which timer are we dealing with ? */

   lpMode  = (LPREG)((char *)TIM_MODE_REG  + (TIM_BANK_SIZE * uTimer));
   lpValue = (LPREG)((char *)TIM_VALUE_REG + (TIM_BANK_SIZE * uTimer));

   /* Reset the current count to 0 (timers count up from 0 to limit) */
   *lpValue = 0;
   CNXT_SET_VAL(lpMode, TIM_DONT_WRAP_MASK, (bReset ? 0 : 1)); 
   CNXT_SET_VAL(lpMode, TIM_ENABLE_MASK   , 1);
}

/*******************************/
/* Stop a given hardware timer */
/*******************************/
void stop_hw_timer(u_int32 uTimer)
{
   LPREG  lpMode;

   /************************************************************************/
   /* WARNING: Function called from within a critical section! Do not add  */
   /*          trace or any other RTOS or C runtime function calls here!!! */
   /************************************************************************/
   
   if(uTimer > 7)
     return;

   /* Which timer are we dealing with ? */

   lpMode  = (LPREG)((char *)TIM_MODE_REG + (TIM_BANK_SIZE * uTimer));

   CNXT_SET_VAL(lpMode, TIM_ENABLE_MASK, 0); 
}

/**************************************************/
/* Restart a stopped counter without resetting it */
/**************************************************/
void    restart_hw_timer(u_int32 uTimer)
{
   LPREG      lpMode;

   if(uTimer > 7)
     return;

   /* Which timer are we dealing with ? */

   lpMode  = (LPREG)((char *)TIM_MODE_REG + (TIM_BANK_SIZE * uTimer));

   CNXT_SET_VAL(lpMode, TIM_ENABLE_MASK, 1); 
}

/*******************************/
/* Read a given hardware timer */
/*******************************/
u_int32 read_hw_timer(u_int32 uTimer)
{
   LPREG        lpValue;

   if(uTimer > 7)
     return(0);

   /* Which timer are we dealing with ? */

   lpValue  = (LPREG)((char *)TIM_VALUE_REG + (TIM_BANK_SIZE * uTimer));

   return((u_int32)(*lpValue));
}

/*********************************************************/
/* Turn on or off the watchdog function on a given timer */
/*********************************************************/
void make_timer_watchdog(u_int32 uTimer, bool bEnable)
{
   bool ks;
   LPREG        lpMode;

   if(uTimer > 7)
     return;

   /* Which timer are we dealing with ? */

   lpMode  = (LPREG)((char *)TIM_MODE_REG + (TIM_BANK_SIZE * uTimer));

   ks = critical_section_begin();

   /* Turn watchdog on or off for this timer */
   CNXT_SET_VAL(lpMode, TIM_WATCHDOG_ENABLE_MASK, bEnable);

   critical_section_end(ks);

   return;
}

/*****************************************************/
/* Enable a particular interrupt line in a given PIC */
/* Needs to be critical_section safe!                */
/*****************************************************/
void enable_hw_int(u_int32 uPic, u_int32 uInt)
{
   u_int32 uBitPos = 0;
   u_int32 uEnable;
   LPREG      lpGpiIntEnable;
   LPREG      lpTimerIntEnable;
   u_int32 uTemp;
   u_int32 uBank;
   bool ks;

   /* Here the PIC number is not actually a PIC number as such (it used to   */
   /* be but then someone changed the architecture under me). It is actually */
   /* an indication of which tier in the interrupt stack handles this        */
   /* particular interrupt. PIC 0 is the main interrupt status register, PIC */
   /* 1 is the expansion register (if present), PIC 2 is the timer register, */
   /* and PIC 3 is the GPIO register.                                        */

   switch(uPic)
   {
     case PIC_CENTRAL:
       /* The easy case - we just muck with the main PIC */
       uBitPos = 1 << uInt;
       if(uBitPos)
       {
          /* Enable the interrupt in the main PIC */
          ks = critical_section_begin();
          uEnable = *lpIntEnable;
          uEnable |= uBitPos;
          *lpIntEnable = uEnable;
          gInterrupt_State |= (0x1 << uInt);
          critical_section_end(ks);
       }
       break;

#if (INTEXP_TYPE != NOT_PRESENT)
     case PIC_EXPANSION:
       uBitPos = 1 << uInt;
       if(uBitPos)
       {
          ks = critical_section_begin();
          /* Enable the interrupt in the expanded PIC */
          uEnable = *lpIntExpEnable;
          uEnable |= uBitPos;
          /* This should ONLY be done if NOT in interrupt context! */
          *lpIntExpEnable = uEnable;
#if RTOS == NUP
          /* save lpIntExpEnable for NUP implementation */
          intExpEnable |= uBitPos;
#endif

          /* Enable expansion interrupts in the main PIC */
          uEnable = *lpIntEnable;
          uEnable |= ITC_EXPANSION;
          *lpIntEnable = uEnable;
          gInterrupt_State |= (0x1 << ITC_EXPANSION_POS); /* For NUP */
          critical_section_end(ks);
       }
       break;
#endif

     case PIC_TIMERS:
       /* Enable the interrupt in the appropriate timer mode register */
       lpTimerIntEnable = (LPREG)(TIM_MODE_REG + (uInt * TIM_BANK_SIZE));
       ks = critical_section_begin();
       CNXT_SET_VAL(lpTimerIntEnable, TIM_INT_ENABLE_MASK, 1); 
       critical_section_end(ks);
       break;

     case PIC_GPIO:
       /* Enable the interrupt in the GPIO interrupt enable reg */
       split_gpio_int(uInt, &uBank, &uBitPos);

       /* Calculate correct int enable register address */
       lpGpiIntEnable = (LPREG)(GPI_INT_ENABLE_REG + (uBank * GPI_BANK_SIZE));

       /* Read-modify-write in a crit section just in case */
       ks = critical_section_begin();
       uTemp = *lpGpiIntEnable;
       uTemp |= (1 << uBitPos);
       *lpGpiIntEnable = uTemp;
       critical_section_end(ks);

       break;

     default:
       break;
   }

}

/******************************************************/
/* Disable a particular interrupt line in a given PIC */
/******************************************************/
void disable_hw_int(u_int32 uPic, u_int32 uInt)
{
   u_int32 uBitPos;
   u_int32 uEnable;
   LPREG      lpGpiIntEnable;
   LPREG      lpTimerIntEnable;
   u_int32 uTemp;
   u_int32 uBank;
   bool ks;

   /* Here the PIC number is not actually a PIC number as such (it used to   */
   /* be but then someone changed the architecture under me). It is actually */
   /* an indication of which tier in the interrupt stack handles this        */
   /* particular interrupt. PIC 0 is the main interrupt status register, PIC */
   /* 1 is the expansion register (if present), PIC 2 is the timer register, */
   /* and PIC 3 is the GPIO register.                                        */

   switch(uPic)
   {
     case PIC_CENTRAL:
       /* The easy case - we just muck with the main PIC */
       ks = critical_section_begin();
       uEnable = *lpIntEnable;
       uEnable &= ~(1 << uInt);
       *lpIntEnable = uEnable;
       gInterrupt_State &= ~(0x1 << uInt);
       critical_section_end(ks);

       break;

#if (INTEXP_TYPE != NOT_PRESENT)
     case PIC_EXPANSION:
       /* Disable expansion interrupts first individually... */
       ks = critical_section_begin();
       uEnable = *lpIntExpEnable;
       uEnable &= ~(1 << uInt);
       *lpIntExpEnable = uEnable;
#if RTOS == NUP
       /* save lpIntExpEnable for NUP implementation */
       intExpEnable &= ~(1 << uInt);
#endif
       critical_section_end(ks);
 
       /* ...then via the main PIC if all expansion interrupts are disabled. */
       if (*lpIntExpEnable == 0) 
       {
         ks = critical_section_begin();
         uEnable = *lpIntEnable;
         uEnable &= ~(ITC_EXPANSION);
         *lpIntEnable = uEnable;
         gInterrupt_State &= ~(0x1 << ITC_EXPANSION_POS); /* For NUP */
         critical_section_end(ks);
       }

       break;
#endif

     case PIC_TIMERS:
       /* Disable a timer's interrupt by clearing the IntEnable bit in the */
       /* timer mode register.                                             */
       lpTimerIntEnable = (LPREG)(TIM_MODE_REG + (uInt * TIM_BANK_SIZE));
       ks = critical_section_begin();
       CNXT_SET_VAL(lpTimerIntEnable, TIM_INT_ENABLE_MASK, 0); 
       critical_section_end(ks);
       break;

     case PIC_GPIO:
       /* Enable the interrupt in the GPIO interrupt enable reg */
       split_gpio_int(uInt, &uBank, &uBitPos);

       /* Calculate correct int enable register address */
       lpGpiIntEnable = (LPREG)(GPI_INT_ENABLE_REG + (uBank * GPI_BANK_SIZE));

       /* Read-modify-write in a crit section just in case */
       ks = critical_section_begin();
       uTemp = *lpGpiIntEnable;
       uTemp &= ~(1 << uBitPos); /* Clear the appropriate bit */
       *lpGpiIntEnable = uTemp;
       critical_section_end(ks);

       break;

     default:
       break;
   }
}

/**********************************************************/
/* Read the timeout value currently set for a given timer */
/**********************************************************/
u_int32 read_hw_timer_timeout(u_int32 uTimer)
{
   LPREG        lpLimit;

   if(uTimer > 7)
     return(0);

   /* Which timer are we dealing with ? */
   lpLimit  = (LPREG)((char *)TIM_LIMIT_REG + (TIM_BANK_SIZE * uTimer));

   return((u_int32)(*lpLimit));
}

/****************************************/
/* Is a given timer currently enabled ? */
/****************************************/
bool is_timer_running(u_int32 uTimer)
{
   LPREG       lpMode;

   if(uTimer > 7)
     return FALSE;

   /* Which timer are we dealing with ? */

   lpMode = (LPREG)((char *)TIM_MODE_REG + (TIM_BANK_SIZE * uTimer));

   if(CNXT_GET_VAL(lpMode, TIM_ENABLE_MASK))
     return(TRUE);
   else
     return(FALSE);
}

/****************************************/
/* Is a given timer currently pending ? */
/****************************************/
bool is_timer_pending(u_int32 uTimer)
{
   u_int32 *lpIrqStatus;
   u_int32 IrqStatus;

   if(uTimer > 7)
     return FALSE;

   /* Which timer are we dealing with ? */

   lpIrqStatus = (u_int32 *)(TIM_INT_STATUS_REG);
   IrqStatus   = *lpIrqStatus;

   if(IrqStatus & (1<<uTimer))
     return(TRUE);
   else
     return(FALSE);
}

/***************************************************/
/* Find out if a given interrupt is enabled or not */
/***************************************************/
bool is_int_enabled(u_int32 uPic, u_int32 uInt)
{
   u_int32 uEnable;
   u_int32 uBank;
   u_int32 uBitPos;
   u_int32 uTemp;
   LPREG   lpGpiIntEnable;
   LPREG      lpTimerIntEnable;

   /* Here the PIC number is not actually a PIC number as such (it used to   */
   /* be but then someone changed the architecture under me). It is actually */
   /* an indication of which tier in the interrupt stack handles this        */
   /* particular interrupt. PIC 0 is the main interrupt status register, PIC */
   /* 1 is the expansion register (if present), PIC 2 is the timer register, */
   /* and PIC 3 is the GPIO register.                                        */

   switch(uPic)
   {
     case PIC_CENTRAL:
       /* The easy case - we just muck with the main PIC */
       uEnable = *lpIntEnable;
       uEnable &= 1 << uInt;
       break;

#if (INTEXP_TYPE != NOT_PRESENT)
     case PIC_EXPANSION:
       /* First, see if expansion interrupts are enabled, then check to see 
          that particular expansion interrupt of interest is enabled. */
       if(*lpIntEnable & ITC_EXPANSION) {
         uEnable = *lpIntExpEnable;
    uEnable &= 1 << uInt;
       } else {
         uEnable = 0;
       }
       break;
#endif

     case PIC_TIMERS:
       /* Read the enable bit from the appropriate timer control register */
       /* We need not look at the main PIC since the timer bit is always  */
       /* enabled (that's the way the KAL  works)                         */
       lpTimerIntEnable = (LPREG)(TIM_MODE_REG + (uInt * TIM_BANK_SIZE));
       uEnable = (u_int32)CNXT_GET_VAL(lpTimerIntEnable, TIM_INT_ENABLE_MASK);
       break;

     case PIC_GPIO:
       /* Read the appropriate enable bit for this GPIO line */
       /* To be completed - we may have to look at the main PIC too here */
       split_gpio_int(uInt, &uBank, &uBitPos);

       /* Calculate correct int enable register address */
       lpGpiIntEnable = (LPREG)(GPI_INT_ENABLE_REG + (uBank * GPI_BANK_SIZE));

       uTemp = *lpGpiIntEnable;
       uEnable = uTemp & (1 << uBitPos);

       break;

     default:
       return(FALSE);
   }

   if(uEnable)
     return(TRUE);
   else
     return(FALSE);
}

/********************************************/
/* Read the raw (unmasked) interrupt status */
/********************************************/
bool is_raw_int_active(u_int32 uPic, u_int32 uInt)
{
   u_int32 uState;
   u_int32 uBank;
   u_int32 uBitPos;
   u_int32 uTemp;
   LPREG   lpGpiIntState;
   LPREG   lpTimerIntState;

   /* Here the PIC number is not actually a PIC number as such (it used to   */
   /* be but then someone changed the architecture under me). It is actually */
   /* an indication of which tier in the interrupt stack handles this        */
   /* particular interrupt. PIC 0 is the main interrupt status register, PIC */
   /* 1 is the expansion register (if present), PIC 2 is the timer register, */
   /* and PIC 3 is the GPIO register.                                        */

   switch(uPic)
   {
     case PIC_CENTRAL:
       uState = *lpIntClear; /* Reading this gives raw status */
       break;

#if (INTEXP_TYPE != NOT_PRESENT)
     case PIC_EXPANSION:
       /* First check to see that expansion interrupt is active in main PIC */
       if (*lpIntClear & ITC_EXPANSION)  
    uState = *lpIntExpStatus; /* Reading this gives raw status */
       else
         uState = 0;
       break;
#endif

     case PIC_TIMERS:
       lpTimerIntState = (LPREG)TIM_INT_STATUS_REG;
       uState = *lpTimerIntState;
       break;

     case PIC_GPIO:
       /* Enable the interrupt in the GPIO interrupt enable reg */
       split_gpio_int(uInt, &uBank, &uBitPos);

       /* Calculate correct int enable register address */
       lpGpiIntState = (LPREG)(GPI_INT_REG + (uBank * GPI_BANK_SIZE));

       uTemp  = *lpGpiIntState;
       uState = uTemp & (1 << uBitPos);
       if(uState)
         return(TRUE);
       else
         return(FALSE);

     default:
       return(FALSE);
   }

   uState &= (1 << uInt);

   if(uState)
     return(TRUE);
   else
     return(FALSE);
}

#if (INTEXP_TYPE != NOT_PRESENT)
/***********************************************************************/
/* Find the lowest expansion pin which is asking for interrupt service */
/***********************************************************************/
u_int32 get_active_exp_int(void)
{
   u_int32 uStatus;
   u_int32 uEnabled;
   u_int32 uBitPos;

   /* Get current enabled mask and status of expansion interrupt */
   uStatus  = *lpIntExpStatus;
#if RTOS == NUP
   uEnabled = intExpEnable;      /* NUP disable the int exp in BSP_IRQ_Handler */
#else
   uEnabled = *lpIntExpEnable;
#endif

   uBitPos = find_lowest_bit(uStatus & uEnabled);

   return(uBitPos);
}

#if RTOS == NUP
void restore_exp_int_enable_bit ( u_int32 dwInt )
{
   *lpIntExpEnable |= intExpEnable;  /* was ... |= ( 0x1 << dwInt ); */
}
#endif
 
#endif

/******************************************************************/
/* Find the lowest GPIO pin which is asking for interrupt service */
/******************************************************************/
u_int32 get_active_gpio_int(void)
{
   u_int32 uStatus;
   u_int32 uEnabled;
   u_int32 uBitPos = 0;
   u_int32 uLoop;
   LPREG   lpIntStatus;
   LPREG   lpIntEnable;

   /* Check each GPIO register in turn starting with the main (lowest) one */
   for(uLoop = 0; uLoop < NUM_GPI_BANKS; uLoop++)
   {
     lpIntStatus = (LPREG)(GPI_INT_REG + (uLoop * GPI_BANK_SIZE));
     lpIntEnable = (LPREG)(GPI_INT_ENABLE_REG + (uLoop * GPI_BANK_SIZE));

     uStatus  = *lpIntStatus;
     uEnabled = *lpIntEnable;

     uBitPos = find_lowest_bit(uStatus & uEnabled);
     if(uBitPos != 0xffffffff)
       break;
   }

   if(uBitPos != 0xffffffff)
      return(uBitPos + (uLoop * 32));
   else
      return 0xffffffff;

}

/*****************************************************************************/
/* Find the lowest numbered timer requesting interrupt service - Neches only */
/*****************************************************************************/
u_int32 get_active_timer_int(void)
{
   u_int32 uStatus;
   u_int32 uBitPos;

   /* Take a look at the timer interrupt status register */
   uStatus = *lpTimerIntStatus;

   /* Mask off unused bits */
   uStatus &= ((1 << NUM_TIM_BANKS) - 1);

   uBitPos = find_lowest_bit(uStatus);

   return(uBitPos);
}

/******************************************/
/* Find the leftmost set bit in a u_int32 */
/******************************************/
u_int32 find_highest_bit(u_int32 uValue)
{
   u_int32 uWork;
   u_int32 uCheck;
   u_int32 uLoop;

   /******************************/
   /* Quick check for zero input */
   /******************************/
   if(!uValue)
     return(0xFFFFFFFF);

   /***************************************************/
   /* Get the highest bit set in the uValue parameter */
   /***************************************************/
   uLoop  = 32;
   uCheck = 0xffffffff;
   do
   {
      uWork = (1 << (uLoop-1));
      if(uValue >= uWork)
      {
        uCheck = (uLoop-1);
        break;
      }
      uLoop--;
   } while (uLoop);

   return(uCheck);
}

/*******************************************/
/* Find the rightmost set bit in a u_int32 */
/*******************************************/
u_int32 find_lowest_bit(u_int32 uValue)
{
   u_int32 uWork;
   u_int32 uCheck;
   u_int32 uLoop;

   /******************************/
   /* Quick check for zero input */
   /******************************/
   if(!uValue)
     return(0xFFFFFFFF);

   /**************************************************/
   /* Get the lowest bit set in the uValue parameter */
   /**************************************************/
   uLoop  = 0;
   uCheck = 0xffffffff;
   do
   {
      uWork = (1 << uLoop);
      if(uValue & uWork)
      {
        uCheck = uLoop;
        break;
      }
      uLoop++;
   } while (uLoop < 32);

   return(uCheck);
}

/***********************************************************************/
/* Disable all hardware interrupts, clearing any pending GPIO or timer */
/* interrupts in the process. This is a brute force function designed  */
/* to be used only during hardware initialisation by the BSP.          */
/***********************************************************************/
void disable_all_hw_ints(void)
{
  u_int32 iLoop;
  LPREG  lpGpiIntEnable;
  LPREG  lpGpiIntStatus;

  /* Disable and clear all timer interrupts (Neches only) */
  for(iLoop = 0; iLoop < NUM_TIM_BANKS; iLoop++)
  {
    disable_hw_int(PIC_TIMERS, iLoop);
    clear_timer_int(iLoop);
  }

  /* To be completed - clear all PWM interrupts here */

  /* Disable and clear all GPIO interrupts */
  for(iLoop = 0; iLoop < NUM_GPI_BANKS; iLoop++)
  {
    lpGpiIntEnable = (LPREG)(GPI_INT_ENABLE_REG + (iLoop * GPI_BANK_SIZE));
    lpGpiIntStatus = (LPREG)(GPI_INT_REG + (iLoop * GPI_BANK_SIZE));
    *lpGpiIntEnable = 0;
    *lpGpiIntStatus = 0xFFFFFFFF;
  }

  /* Disable and clear all interrupts at the central interrupt controller */
  *lpIntEnable = 0;
  *lpIntClear  = 0xFFFFFFFF;

#if (INTEXP_TYPE != NOT_PRESENT)
  /* Disable and clear all interrupts at the expanded interrupt controller */
  *lpIntExpEnable = 0;
  *lpIntExpClear  = 0xFFFFFFFF;
#endif

  /* Force all future interrupts to be routed to IRQ rather than FIQ */
  *(LPREG)ITC_INTDEST_REG = 0xFFFFFFFF;
}

/****************************************************************************
 * Modifications:
 * $Log: 
 *  52   mpeg      1.51        9/2/03 7:05:18 PM      Joe Kroesche    SCR(s) 
 *        7415 :
 *        removed unneeded header files and reordered to eliminate warnings 
 *        when
 *        building for PSOS
 *        
 *  51   mpeg      1.50        7/14/03 10:09:50 AM    Bobby Bradford  SCR(s) 
 *        6946 :
 *        Removed unused references to CONFMGR (confmgr.h, transport_source =
 *        NIM_SATELLITE_DEMOD).
 *        
 *  50   mpeg      1.49        7/9/03 3:27:30 PM      Tim White       SCR(s) 
 *        6901 :
 *        Phase 3 codeldrext drop.
 *        
 *        
 *  49   mpeg      1.48        6/24/03 2:06:52 PM     Miles Bintz     SCR(s) 
 *        6822 :
 *        added initialization value to remove warning in release build
 *        
 *  48   mpeg      1.47        5/21/03 4:27:06 PM     Billy Jackman   SCR(s) 
 *        6532 6533 6534 :
 *        Modified code to adjust only the requested bit in the saved expansion
 *         interrupt
 *        mask (intExpEnable).  This avoids a problem where the intExpEnable 
 *        variable was
 *        inadvertantly cleared when disable_hw_int was called during an 
 *        interrupt service
 *        routine.
 *        
 *  47   mpeg      1.46        3/4/03 4:00:38 PM      Bobby Bradford  SCR(s) 
 *        5664 5665 :
 *        Modified code to support enable/disable of expansion interrupts
 *        properly (corresponding changes in NUP_IRQ.C)
 *        
 *  46   mpeg      1.45        2/13/03 12:01:50 PM    Matt Korte      SCR(s) 
 *        5479 :
 *        Removed old header reference
 *        
 *  45   mpeg      1.44        1/31/03 9:14:02 AM     Lucy C Allevato SCR(s) 
 *        5291 :
 *        Modified fix note for revision 1.43 that contained an unprintable 
 *        character, both in the workfile and the archive.
 *        
 *  44   mpeg      1.43        1/23/03 5:11:28 PM     Miles Bintz     SCR(s) 
 *        5275:  Fixed a copy/paste bug - removed a bitwise negation in the 
 *        set_gpio_output_high function
 *        
 *  43   mpeg      1.42        1/17/03 2:05:52 PM     Bobby Bradford  SCR(s) 
 *        5264 :
 *        Added SHADOW register(s) for extended GPIO to support BRONCO
 *        IRD (which does not support read-back of extended GPIO register)
 *        
 *  42   mpeg      1.41        12/16/02 2:01:24 PM    Dave Wilson     SCR(s) 
 *        5177 :
 *        
 *        
 *        
 *        
 *        
 *        Added function set_gpio_output_level. Used by LEDS.C to control 
 *        GPIO-connected
 *        LEDs.
 *        
 *  41   mpeg      1.40        12/10/02 4:03:02 PM    Dave Wilson     SCR(s) 
 *        5091 :
 *        Changed INTEXP_TYPE logic to ensure that expansion register support 
 *        is 
 *        included in Brazos builds as well as Wabash ones.
 *        
 *  40   mpeg      1.39        9/3/02 10:42:46 AM     Larry Wang      SCR(s) 
 *        4506 :
 *        For NUP, save interrupt expansion enable register (0x30450020) since 
 *        the interrupt expansion always disabled in IST.
 *        
 *  39   mpeg      1.38        8/30/02 3:24:16 PM     Larry Wang      SCR(s) 
 *        4499 :
 *        Implement the interrupt expansion.
 *        
 *  38   mpeg      1.37        6/26/02 5:45:58 PM     Bobby Bradford  SCR(s) 
 *        4085 :
 *        Changed standard header file include from "" to <> to remove compiler
 *        warnings in ADS
 *        
 *  37   mpeg      1.36        6/26/02 6:46:18 AM     Mike O'brien    SCR(s): 
 *        4016 
 *        Fixed race condition in the interrupt hw enable/disable functions. 
 *        Added critical sections around the PIC Central case.
 *        
 *  36   mpeg      1.35        5/9/02 6:39:58 PM      Craig Dry       SCR(s) 
 *        3749 :
 *        Remove remnant bitfield code along with enclosing #ifdefs.
 *        
 *  35   mpeg      1.34        5/3/02 3:53:42 PM      Craig Dry       SCR(s) 
 *        3671 :
 *        Eradicate bitfield use from TIM module
 *        
 *  34   mpeg      1.33        4/22/02 3:39:54 PM     Larry Wang      SCR(s) 
 *        3585 :
 *        Extend set_gpio_output_low() and set_gpio_output_high() to GPIO 
 *        extender.  If GPIO_bit is greater than NUM_OF_GPIO, it means GPIO 
 *        extender bit (GPIO_bit-NUM_OF_GPIO).
 *        
 *  33   mpeg      1.32        4/5/02 2:16:14 PM      Tim White       SCR(s) 
 *        3510 :
 *        Backout DCS #3468
 *        
 *        
 *  32   mpeg      1.31        4/1/02 10:01:38 PM     Miles Bintz     SCR(s) 
 *        3484 :
 *        added a bitwise or so set_gpio_int_edge is a read-modify-write
 *        
 *  31   mpeg      1.30        4/1/02 6:59:52 PM      Miles Bintz     SCR(s) 
 *        3470 :
 *        Added get_gpio_input function definition.
 *        
 *  30   mpeg      1.29        3/28/02 2:21:24 PM     Quillian Rutherford 
 *        SCR(s) 3468 :
 *        Removed bit fields
 *        
 *        
 *  29   mpeg      1.28        2/8/02 11:15:44 AM     Bobby Bradford  SCR(s) 
 *        3131 :
 *        Added new function, is_timer_pending(), which checks to see if a 
 *        given timer interrupt is pending (but hasn't been serviced yet).  
 *        This is used by the get_system_time_us() to check for hardware timer 
 *        roll-over.
 *        
 *  28   mpeg      1.27        7/12/01 8:18:22 AM     Quillian Rutherford 
 *        SCR(s) 2262 2263 2264 2265 2266 :
 *        Added a read to the clear_pic_interrupt() function that reads back 
 *        what
 *        was written to assure the write occurred.
 *        
 *        
 *  27   mpeg      1.26        5/4/01 2:54:10 PM      Tim White       DCS#1822,
 *         DCS#1824, DCS31825 -- Critical Section Overhaul
 *        
 *  26   mpeg      1.25        5/1/01 8:31:24 AM      Dave Wilson     DCS1817: 
 *        Remodev unsafe code from iwthin critical sections
 *        
 *  25   mpeg      1.24        1/18/01 9:48:12 AM     Dave Wilson     DCS976: 
 *        Added GPIO access function to make pin input
 *        
 *  24   mpeg      1.23        1/16/01 6:30:54 PM     Tim Ross        DCS966. 
 *        Modified get_active_gpio_int() to AND uStatus with uEnabled
 *        before searching for active bit. This is required for Hondo, but 
 *        should
 *        not adversely effect Colorado. :-)
 *        
 *  23   mpeg      1.22        12/15/00 2:03:42 PM    Ray Mack        Tracker 
 *        868 -- removed set_transport_source
 *        
 *  22   mpeg      1.21        11/27/00 2:29:38 PM    Ray Mack        moved 
 *        transport_source here
 *        
 *  21   mpeg      1.20        11/27/00 10:36:36 AM   Ray Mack        Added 
 *        function to set input transport source (NIM, DVB baseband, etc.)
 *        Removed warnings
 *        
 *  20   mpeg      1.19        11/17/00 11:57:14 AM   Dave Wilson     Fixed 
 *        GPIO access functions which had been setting and clearing bits other
 *        than the ones asked for.
 *        
 *  19   mpeg      1.18        10/19/00 6:55:06 PM    Miles Bintz     Added 
 *        gInterrupt_State so NUP doesn't turn on interrupts that are disabled
 *        
 *  18   mpeg      1.17        8/29/00 8:43:32 PM     Miles Bintz     removed 
 *        POST call (Nucleus specific)
 *        
 *  17   mpeg      1.16        8/29/00 5:33:56 PM     Ismail Mustafa  Fix for 
 *        enabling wrong interrupt bit on central PIC when enabling GPIO
 *        interrupts.
 *        
 *  16   mpeg      1.15        6/22/00 12:43:22 PM    Ray Mack        fixed bug
 *         in new function
 *        
 *  15   mpeg      1.14        6/22/00 12:33:54 PM    Ray Mack        added 
 *        functions to set GPIO bits high or low.  Allows selection of MACRO or
 *        function to do the job
 *        
 *  14   mpeg      1.13        6/7/00 10:47:34 AM     Ray Mack        fixed bug
 *         in get_active_gpio_int.  It only worked for the lowest bank of GPIO
 *        's.  I also changed all occurences of (uint32)-1 to 0xffffffff.  This
 *         was 
 *        *very* bad programming practice!!!
 *        
 *  13   mpeg      1.12        6/2/00 3:31:58 PM      Ray Mack        fix type 
 *        that ARM compiler complains about in set_gpio_int_polarity
 *        
 *  12   mpeg      1.11        6/2/00 10:50:48 AM     Ray Mack        Added 
 *        BOTH to set_gpio_int_edge and changed to use definitions from
 *        colorado.h
 *        
 *  11   mpeg      1.10        6/1/00 6:44:38 PM      Ray Mack        fixed 
 *        error in new GPIO set function
 *        
 *  10   mpeg      1.9         6/1/00 6:37:26 PM      Ray Mack        added 
 *        GPIO interrupt polarity function
 *        
 *  9    mpeg      1.8         3/9/00 2:00:36 PM      Miles Bintz     added 
 *        hwlib.h to list of includes
 *        
 *  8    mpeg      1.7         3/3/00 1:35:06 PM      Tim Ross        Changed 
 *        OS switch to RTOS switch. 
 *        
 *  7    mpeg      1.6         1/26/00 9:38:40 AM     Dave Wilson     Added 
 *        code to gather interrupt latency statistics
 *        Fixed a problem in timers - timer was not cleared when set.
 *        
 *  6    mpeg      1.5         11/18/99 8:09:00 PM    Dave Wilson     Changed 
 *        system timer handling to keep it inside HWLIB.
 *        
 *  5    mpeg      1.4         11/2/99 10:15:10 AM    Dave Wilson     Removed 
 *        all cases for Sabine, PID board and early emulation phases.
 *        
 *  4    mpeg      1.3         11/1/99 5:00:52 PM     Dave Wilson     Removed 
 *        all pSOS-specific headers and obsolete config file definitions.
 *        
 *  3    mpeg      1.2         10/14/99 7:15:28 PM    Dave Wilson     Removed 
 *        dependence on OpenTV headers.
 *        
 *  2    mpeg      1.1         9/28/99 1:57:28 PM     Dave Wilson     Added 
 *        tick timer functions.
 *        
 *  1    mpeg      1.0         9/14/99 3:11:54 PM     Dave Wilson     
 * $
 * 
 *    Rev 1.51   02 Sep 2003 18:05:18   kroescjl
 * SCR(s) 7415 :
 * removed unneeded header files and reordered to eliminate warnings when
 * building for PSOS
 * 
 *    Rev 1.50   14 Jul 2003 09:09:50   bradforw
 * SCR(s) 6946 :
 * Removed unused references to CONFMGR (confmgr.h, transport_source =
 * NIM_SATELLITE_DEMOD).
 * 
 *    Rev 1.49   09 Jul 2003 14:27:30   whiteth
 * SCR(s) 6901 :
 * Phase 3 codeldrext drop.
 * 
 * 
 *    Rev 1.48   24 Jun 2003 13:06:52   bintzmf
 * SCR(s) 6822 :
 * added initialization value to remove warning in release build
 *
 *    Rev 1.47   21 May 2003 15:27:06   jackmaw
 * SCR(s) 6532 6533 6534 :
 * Modified code to adjust only the requested bit in the saved expansion interrupt
 * mask (intExpEnable).  This avoids a problem where the intExpEnable variable was
 * inadvertantly cleared when disable_hw_int was called during an interrupt service
 * routine.
 * 
 *    Rev 1.46   04 Mar 2003 16:00:38   bradforw
 * SCR(s) 5664 5665 :
 * Modified code to support enable/disable of expansion interrupts
 * properly (corresponding changes in NUP_IRQ.C)
 * 
 *    Rev 1.45   13 Feb 2003 12:01:50   kortemw
 * SCR(s) 5479 :
 * Removed old header reference
 * 
 *    Rev 1.44   31 Jan 2003 09:14:02   allevalc
 * SCR(s) 5291 :
 * Modified fix note for revision 1.43 that contained an unprintable character, both in the workfile and the archive.
 * 
 *    Rev 1.43   23 Jan 2003 17:11:28   bintzmf
 * SCR(s) 5275 :
 * fixed a copy/paste bug - removed a bitwise negation in the set_gpio_output_high function
 * 
 * 
 *    Rev 1.42   17 Jan 2003 14:05:52   bradforw
 * SCR(s) 5264 :
 * Added SHADOW register(s) for extended GPIO to support BRONCO
 * IRD (which does not support read-back of extended GPIO register)
 * 
 *    Rev 1.41   16 Dec 2002 14:01:24   dawilson
 * SCR(s) 5177 :
 * Added function set_gpio_output_level. Used by LEDS.C to control GPIO-connected
 * LEDs.
 * 
 *    Rev 1.40   10 Dec 2002 16:03:02   dawilson
 * SCR(s) 5091 :
 * Changed INTEXP_TYPE logic to ensure that expansion register support is 
 * included in Brazos builds as well as Wabash ones.
 * 
 *    Rev 1.39   03 Sep 2002 09:42:46   wangl2
 * SCR(s) 4506 :
 * For NUP, save interrupt expansion enable register (0x30450020) since the interrupt expansion always disabled in IST.
 * 
 *    Rev 1.38   30 Aug 2002 14:24:16   wangl2
 * SCR(s) 4499 :
 * Implement the interrupt expansion.
 * 
 *    Rev 1.35   09 May 2002 17:39:58   dryd
 * SCR(s) 3749 :
 * Remove remnant bitfield code along with enclosing #ifdefs.
 * 
 *    Rev 1.34   03 May 2002 14:53:42   dryd
 * SCR(s) 3671 :
 * Eradicate bitfield use from TIM module
 * 
 *    Rev 1.33   22 Apr 2002 14:39:54   wangl2
 * SCR(s) 3585 :
 * Extend set_gpio_output_low() and set_gpio_output_high() to GPIO extender.  If GPIO_bit is greater than NUM_OF_GPIO, it means GPIO extender bit (GPIO_bit-NUM_OF_GPIO).
 * 
 *    Rev 1.32   05 Apr 2002 14:16:14   whiteth
 * SCR(s) 3510 :
 * Backout DCS #3468
 * 
 * 
 *    Rev 1.31   01 Apr 2002 22:01:38   bintzmf
 * SCR(s) 3484 :
 * added a bitwise or so set_gpio_int_edge is a read-modify-write
 *
 *    Rev 1.30   01 Apr 2002 18:59:52   bintzmf
 * SCR(s) 3470 :
 * Added get_gpio_input function definition.
 *
 *    Rev 1.29   28 Mar 2002 14:21:24   rutherq
 * SCR(s) 3468 :
 * Removed bit fields
 *
 *    Rev 1.28   Feb 08 2002 11:15:44   bradforw
 * SCR(s) 3131 :
 * Added new function, is_timer_pending(), which checks to see if a given timer interrupt is pending (but hasn't been serviced yet).  This is used by the get_system_time_us() to check for hardware timer roll-over.
 * 
 *    Rev 1.27   12 Jul 2001 07:18:22   rutherq
 * SCR(s) 2262 2263 2264 2265 2266 :
 * Added a read to the clear_pic_interrupt() function that reads back what
 * was written to assure the write occurred.
 * 
 * 
 *    Rev 1.26   04 May 2001 13:54:10   whiteth
 * DCS#1822, DCS#1824, DCS31825 -- Critical Section Overhaul
 * 
 *    Rev 1.25   01 May 2001 07:31:24   dawilson
 * DCS1817: Remodev unsafe code from iwthin critical sections
 * 
 *    Rev 1.24   18 Jan 2001 09:48:12   dawilson
 * DCS976: Added GPIO access function to make pin input
 *
 ****************************************************************************/

