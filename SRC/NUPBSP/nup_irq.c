/****************************************************************************/
/*                   CONEXANT PROPRIETARY AND CONFIDENTIAL                  */
/*                       SOFTWARE FILE/MODULE HEADER                        */
/*       Conexant Systems Inc. (c) 1998, 1999, 2000, 2001, 2002, 2003       */
/*                               Austin, TX                                 */
/*                            All Rights Reserved                           */
/****************************************************************************/
/*                                                                          */
/* Filename:       nup_irq.c                                                */
/*                                                                          */
/*                                                                          */
/* Description:    ISR related functions                                    */
/*                                                                          */
/* Author:         Miles Bintz                                              */
/*                                                                          */
/****************************************************************************/
/* $Header:   
 ****************************************************************************/
#include "stbcfg.h"
#include "basetype.h"
#include "hwfuncs.h"
#include "hwlib.h"
#include "nup.h"

/*************/
/* Externs   */
/*************/
extern bool DisableInterrupts();
extern void EnableInterrupts();
extern void TMT_Timer_Interrupt();
extern void kal_int_handler(u_int32 uInts, u_int32 uPic, bool bFIQ);
extern u_int8 NUP_Initialized;
#ifdef DEBUG
#ifdef CRIT_SEC_LATENCY_CHECK
extern int from_int_handler;
#endif
#endif

/****************************************************************************/
/****************************************************************************/

volatile u_int32 IntMask = 0;

u_int32 IntCount = 0;

NU_HISR kal_int_hisr;
u_int32 hisr_stack[KAL_INT_HISR_STACK_SIZE];

u_int32 TMD_HISR_Stack[TM_HISR_STACK_SIZE];

/****************************************************************************/
/* int_handler                                                              */
/*                                                                          */
/* This function forms the main HISR which processes interrupts passed to   */
/* the BSP_IRQ_Handler low level interrupt service routine. It merely       */
/* disabled interrupts and calls the main KAL interrupt handler.            */
/*                                                                          */
/*                                                                          */
/*                                                                          */
/****************************************************************************/

void int_handler(void) {
    u_int32 cs;
    
#ifdef DEBUG
#ifdef CRIT_SEC_LATENCY_CHECK
	from_int_handler = 1;
#endif
#endif    
    /*
     * Disable interrupts to ensure KAL interrupt handlers are automatically
     * protected against interrupts and scheduling
     */

    cs = (u_int32)DisableInterrupts();

    #if (USE_BRAZOS16_PCM_SW_WORKAROUND == YES)
    #define NU_DISABLE_INTERRUPTS_ONLY 0x80
    NU_Control_Interrupts(NU_DISABLE_INTERRUPTS_ONLY);
    #else /* (USE_BRAZOS16_PCM_SW_WORKAROUND == YES) */
    NU_Control_Interrupts(NU_DISABLE_INTERRUPTS);
    #endif /* (USE_BRAZOS16_PCM_SW_WORKAROUND == YES) */
    
    /* kal_int_handler is normally passed the interrupt status directly
     * from the PIC chip.  Here however, since the IntMask is roughly the
     * same piece of information, we pass that instead.  As a result,
     * kal_int_handler requires a small modification at the end (to clear
     * the bits in the IntMask global variable)
     */

    kal_int_handler(IntMask, PIC_CENTRAL, FALSE);
    
    /*
     * Reenable interrupts after the handlers have have run
     */

    NU_Control_Interrupts(NU_ENABLE_INTERRUPTS);
    if(cs)
      EnableInterrupts();
      
#ifdef DEBUG
#ifdef CRIT_SEC_LATENCY_CHECK
	from_int_handler = 0;
#endif
#endif    

}

/* The BSP IRQ Handler is ARM's IRQ exception handler.  It is the first thing
 * called when an interrupt occurs.  It processes timer ticks for the OS
 * then passes control along to the kal_int_hisr
 */
void BSP_IRQ_Handler(void) {
    u_int32 IntToSvc;
    int i,count;
    static int tickcount = 0;
     
    LPREG IntStatus, IntEnable, TimerInterrupt;
      IntStatus = (LPREG)ITC_INTRIRQ_REG;
      IntEnable = (LPREG)ITC_INTENABLE_REG;
      TimerInterrupt = (LPREG)TIM_INT_STATUS_REG;
    
    
    /* Determine which interrupts are in need of service */
#if (INTEXP_TYPE != NOT_PRESENT)
    IntToSvc = (u_int32)(*IntStatus) & ((u_int32)(*IntEnable) | ITC_EXPANSION);
#else
    IntToSvc = (u_int32)(*IntStatus) & (u_int32)(*IntEnable);
#endif
         /* Note: ITC_EXPANSION is **ALWAYS** enabled in hardware, so always
         ** treat it as enabled in software
         */

    /* How many distinct interrupts are currently pending? */
    count=0;
    for (i = 0; i < 32; i++) {
        if ((IntToSvc >> i) & 0x1) count++;
    }

    /* For debug only - catch cases where more than 1 interrupt is pending */
    /* or where no interrupts are pending.                                 */
    if (count > 1 || count == 0) {   /* conditions to watch for */
        i++;                         /* a place to set our breakpoint */
    }
     
            
    if (IntToSvc & ITC_TIMER) {                    /* Is this interrupt from a timer? */
        if (*TimerInterrupt & (1 << SYS_TIMER)) {  /* Is it the system timer?         */
            tickcount++;
            if (tickcount == SYS_TIMER_DIVIDE) {
                TMT_Timer_Interrupt();       /* Let OS know the timer ticked    */
                tickcount = 0;
            }
            POST(POST_IRQ_TIMER);
        }
    }
    
    if (IntToSvc) {
        POST(POST_IRQ_OTHER);
        
        /* interrupts are already disabled so we are guarunteed mutex */
        /* Keep track of which interrupt we need to service */
        POST(POST_INTMASK_MOD);
        IntMask |= IntToSvc; 
        /* Disable pending interrupts on the PIC so that we can reenable
         * interrupts on the processor before servicing the offending
         * device.  Nucleus insists on reenabling interrupts when this
         * function returns and this prevents us from being interrupted
         * during the time the HISR is processing a previous interrupt.
         * The HWLIB/KAL model for interrupts assumes that they are never
         * interrupted. */
        *IntEnable = (*IntEnable & ~IntToSvc); 

        #if (INTEXP_TYPE != NOT_PRESENT)
        /* disable the expanded interrupt (since this bit in the main PIC is not masked) */
        if ( IntToSvc & ITC_EXPANSION )
        {
           *(LPREG)ITC_EXPENABLE_REG = 0x00000000;
        }
        #endif

        POST(POST_PICINT_DIS);

        /* For each pending interrupt, queue the HISR to process it (this has very low latency!) */
        for (i = 0; i < count; i++){
            if (NUP_Initialized){
                NU_Activate_HISR(&kal_int_hisr);
            }
        }
    }
}

#ifdef DEBUG
#define MODE_MASK    0x1F
#define USER_MODE    0x10
#define FIQ_MODE     0x11
#define IRQ_MODE     0x12
#define SVC_MODE     0x13
#define ABORT_MODE   0x17
#define UNDEF_MODE   0x1B
#define SYSTEM_MODE  0x1F
char modestrings[7][13] =  {"   USER     ",
                            "    FIQ     ",
                            "    IRQ     ",
                            "    SVC     ",
                            "   ABORT    ",
                            "UNDEF INSTR ",
                            "   SYSTEM   "};
char spstr[11];
char lrstr[11];

void inttohexstr(int x, char *str)  {
    int i;
    
    str[0] = '0'; str[1] = 'x'; str[10] = '\0';
    
    for (i = 0; i < 8; i++) {
        switch (x & 0xf) {
            case 0: case 1:
            case 2: case 3:
            case 4: case 5:
            case 6: case 7:
            case 8: case 9:
                str[9-i] = (x & 0xf) + 0x30;
                break;
            case 0xA: case 0xB:
            case 0xC: case 0xD:
            case 0xE: case 0xF:
                str[9-i] = (x & 0xf) + 0x37;
                break;
        }
        x >>= 4;
    }
}
#endif

/*****************************************************************/
/* Handler for all processor exceptions other than IRQ and reset */
/*****************************************************************/
void BSP_Other_ISR_Handler(int cpsr, int spsr, int sp, int lr)  {
#ifdef DEBUG
    int curmode, prevmode;
    extern void putstringNoQ( char* str );

    /*****************************************************************/
    /* Write a marker to the RST_SCRATCH_REG indicating which type   */
    /* of exception occurred. This can be used, for example, to      */
    /* trigger data capture on the emulator.                         */
    /*****************************************************************/
    *(LPREG)RST_SCRATCH_REG = 0xDEAD0000         | 
                              (cpsr & MODE_MASK) | 
                              ((spsr & MODE_MASK) << 8);
        
    /*****************************************************************/
    /* In a debug build, we dump some diagnostic information via the */
    /* serial port then sit in a tight loop.                         */
    /*****************************************************************/
    putstringNoQ("\r\n\r\nERROR!  Unhandled exception:\r\n");
    putstringNoQ("Current  processor mode  is: ");
    inttohexstr(sp, spstr);
    inttohexstr(lr, lrstr);
    
    curmode = cpsr & MODE_MASK;
    prevmode = spsr & MODE_MASK;
    
    switch (curmode) {
        case USER_MODE:
            putstringNoQ(modestrings[0]);
            break;
        case FIQ_MODE:
            putstringNoQ(modestrings[1]);
            break;
        case IRQ_MODE:
            putstringNoQ(modestrings[2]);
            break;
        case SVC_MODE:
            putstringNoQ(modestrings[3]);
            break;
        case ABORT_MODE:
            putstringNoQ(modestrings[4]);
            break;
        case UNDEF_MODE:
            putstringNoQ(modestrings[5]);
            break;
        case SYSTEM_MODE:
            putstringNoQ(modestrings[6]);
            break;
    }
    
    putstringNoQ("\r\nPrevious processor mode was: ");
    switch (prevmode) {
        case USER_MODE:
            putstringNoQ(modestrings[0]);
            break;
        case FIQ_MODE:
            putstringNoQ(modestrings[1]);
            break;
        case IRQ_MODE:
            putstringNoQ(modestrings[2]);
            break;
        case SVC_MODE:
            putstringNoQ(modestrings[3]);
            break;
        case ABORT_MODE:
            putstringNoQ(modestrings[4]);
            break;
        case UNDEF_MODE:
            putstringNoQ(modestrings[5]);
            break;
        case SYSTEM_MODE:
            putstringNoQ(modestrings[6]);
            break;
    }
    

    putstringNoQ("\r\nStack pointer = ");
    putstringNoQ(spstr);
    putstringNoQ("\r\nLink register = ");
    putstringNoQ(lrstr);
    
    while(1);

#else    
    /**************************************************************************/
    /* In a release build, we delay for a very short time then reboot the IRD */
    /**************************************************************************/
    int i;
    /* wait for about a second */
    for (i = 0; i < 33333333; i++);
    
      reboot_IRD();   /* Release build - something bad happened - reboot box! */
#endif


}

/****************************************************************************
 * Modifications:
 * $Log: 
 *  17   mpeg      1.16        6/3/03 3:41:46 PM      Dave Wilson     SCR(s) 
 *        6650 6649 :
 *        Removed calls to critical_section_begin/end from int_handler and 
 *        replaced
 *        them with explicit Disable/EnableInterrupts calls. Use of a critical 
 *        section
 *        clashed with new code that is used to ensure that no KAL functions 
 *        are called
 *        from within critical sections.
 *        
 *  16   mpeg      1.15        3/5/03 8:59:42 AM      Bobby Bradford  SCR(s) 
 *        5664 5665 :
 *        Made reference to ITC_EXPANSION conditional (failed to build for
 *        Colorado builds)
 *        
 *  15   mpeg      1.14        3/4/03 4:02:54 PM      Bobby Bradford  SCR(s) 
 *        5664 5665 :
 *        Modfied handling of expansion interrupts in the BSP irq handler.
 *        see correpsonding changes in HWFUNCS.C ...
 *        
 *  14   mpeg      1.13        2/13/03 12:48:18 PM    Matt Korte      SCR(s) 
 *        5479 :
 *        Removed old header reference
 *        
 *  13   mpeg      1.12        2/5/03 2:46:46 PM      Senthil Veluswamy SCR(s) 
 *        5401 :
 *        Added code to disable turning off the FIQ when running the Brazos 
 *        16bit memory PCM playback workaround
 *        
 *  12   mpeg      1.11        1/8/03 2:56:30 PM      Dave Wilson     SCR(s) 
 *        5220 :
 *        Added code to write a marker value to the RST_SCRATCH_REG when an 
 *        exception
 *        occurs. This can be used to trigger the emulator to stop tracing. 
 *        Value 
 *        written is 0xDEAD + spsr mode + cpsr mode.
 *        
 *  11   mpeg      1.10        12/16/02 3:49:12 PM    Tim White       SCR(s) 
 *        5169 :
 *        Allow future chips to use Wabash code by default instead of the 
 *        Colorado code.
 *        
 *        
 *  10   mpeg      1.9         10/10/02 2:24:36 PM    Dave Wilson     SCR(s) 
 *        4765 :
 *        Clarified and corrected some commenting. Also replaced a few 
 *        hardcoded
 *        numbers with the correct labels from the chip header file.
 *        
 *  9    mpeg      1.8         9/6/02 4:31:38 PM      Bobby Bradford  SCR(s) 
 *        4552 :
 *        Made definition of IntExpEnable and IntExtStatus conditional in
 *        the same way their reference is conditional, to remove a compiler
 *        warning.
 *        
 *  8    mpeg      1.7         9/3/02 10:49:32 AM     Larry Wang      SCR(s) 
 *        4506 :
 *        For wabash, disable interrupt expansion by clear the corresponding 
 *        bit in interrupt expansion enable register (0x30450020).
 *        
 *  7    mpeg      1.6         5/13/02 6:52:22 PM     Senthil Veluswamy SCR(s) 
 *        3769 :
 *        removed build warnings for Release build
 *        
 *  6    mpeg      1.5         4/29/02 12:32:20 PM    Billy Jackman   SCR(s) 
 *        3647 :
 *        Added extern declaration for putstringNoQ.  Added modification log at
 *         end of
 *        file.
 *        
 *  5    mpeg      1.4         9/6/01 4:00:06 PM      Miles Bintz     SCR(s) 
 *        1801 2486 :
 *        Added BSP_Other_ISR handler to handle undef_instr, SWI, FIQ, and 
 *        abort exceptions
 *        
 *  4    mpeg      1.3         7/17/01 4:45:08 PM     Angela          SCR(s) 
 *        2327 :
 *        added critical section latency check
 *        
 *  3    mpeg      1.2         5/4/01 3:36:08 PM      Tim White       DCS#1821,
 *         DCS#1823 -- Add interrupt disabling and task switch prevention 
 *        around
 *        the ISR handler task such that all driver slih's will have the same 
 *        operating
 *        environment as pSOS provides (no interrupts and no task switching).
 *        
 *  2    mpeg      1.1         8/29/00 5:43:50 PM     Ismail Mustafa  Disabled 
 *        timer interrupt and removed critical section around call to
 *        kal_int_handler.
 *        
 *  1    mpeg      1.0         8/25/00 4:25:34 PM     Ismail Mustafa  
 * $
 * 
 *    Rev 1.16   03 Jun 2003 14:41:46   dawilson
 * SCR(s) 6650 6649 :
 * Removed calls to critical_section_begin/end from int_handler and replaced
 * them with explicit Disable/EnableInterrupts calls. Use of a critical section
 * clashed with new code that is used to ensure that no KAL functions are called
 * from within critical sections.
 * 
 *    Rev 1.15   05 Mar 2003 08:59:42   bradforw
 * SCR(s) 5664 5665 :
 * Made reference to ITC_EXPANSION conditional (failed to build for
 * Colorado builds)
 * 
 *    Rev 1.14   04 Mar 2003 16:02:54   bradforw
 * SCR(s) 5664 5665 :
 * Modfied handling of expansion interrupts in the BSP irq handler.
 * see correpsonding changes in HWFUNCS.C ...
 * 
 *    Rev 1.13   13 Feb 2003 12:48:18   kortemw
 * SCR(s) 5479 :
 * Removed old header reference
 * 
 *    Rev 1.12   05 Feb 2003 14:46:46   velusws
 * SCR(s) 5401 :
 * Added code to disable turning off the FIQ when running the Brazos 16bit memory PCM playback workaround
 * 
 *    Rev 1.11   08 Jan 2003 14:56:30   dawilson
 * SCR(s) 5220 :
 * Added code to write a marker value to the RST_SCRATCH_REG when an exception
 * occurs. This can be used to trigger the emulator to stop tracing. Value 
 * written is 0xDEAD + spsr mode + cpsr mode.
 * 
 *    Rev 1.10   16 Dec 2002 15:49:12   whiteth
 * SCR(s) 5169 :
 * Allow future chips to use Wabash code by default instead of the Colorado code.
 * 
 * 
 *    Rev 1.9   10 Oct 2002 13:24:36   dawilson
 * SCR(s) 4765 :
 * Clarified and corrected some commenting. Also replaced a few hardcoded
 * numbers with the correct labels from the chip header file.
 * 
 *    Rev 1.8   06 Sep 2002 15:31:38   bradforw
 * SCR(s) 4552 :
 * Made definition of IntExpEnable and IntExtStatus conditional in
 * the same way their reference is conditional, to remove a compiler
 * warning.
 * 
 *    Rev 1.7   03 Sep 2002 09:49:32   wangl2
 * SCR(s) 4506 :
 * For wabash, disable interrupt expansion by clear the corresponding bit in interrupt expansion enable register (0x30450020).
 * 
 *    Rev 1.6   13 May 2002 17:52:22   velusws
 * SCR(s) 3769 :
 *  removed build warnings for Release build
 * 
 *    Rev 1.5   29 Apr 2002 11:32:20   jackmaw
 * SCR(s) 3647 :
 * Added extern declaration for putstringNoQ.  Added modification log at end of
 * file.
 * 
 ****************************************************************************/

