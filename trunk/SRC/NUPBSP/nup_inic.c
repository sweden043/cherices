/****************************************************************************/
/*                   CONEXANT PROPRIETARY AND CONFIDENTIAL                  */
/*                       SOFTWARE FILE/MODULE HEADER                        */
/*                 Conexant Systems Inc. (c) 1998, 1999, 2000               */
/*                               Austin, TX                                 */
/*                            All Rights Reserved                           */
/****************************************************************************/
/*                                                                          */
/* Filename:       nup_inic.c                                               */
/*                                                                          */
/*                                                                          */
/* Description:    initialization functions to pick up where nup_inis.s t   */
/*                 left off.                                                */
/*                                                                          */
/* Author:         Miles Bintz                                              */
/*                                                                          */
/****************************************************************************/
/****************************************************************************
 * $Header: nup_inic.c, 12, 2/13/03 12:48:14 PM, Matt Korte$
 *
 ****************************************************************************/

#include "stbcfg.h"
#include "basetype.h"
#include "hwlib.h"
#include "hwfuncs.h"
#include "nup.h"

#define REGION_SIZE           2000
#define PARTITION_SIZE        1000 
#define STACK_SIZE            1000 

/************************************************************************/
/************************************************************************/

/***************************/
/*   Externs               */
/***************************/
/* Root Task prototype */
extern VOID root(UNSIGNED argc, VOID *argv);

extern void INC_Initialize(void *pfam);
extern void INT_Initialize(void);
extern void hwlib_timer_initialize(u_int32 uRate, bool bStart);
extern void int_handler(void);
extern void *GetSuperStackBase();
extern void App_Init(void *pfam);
extern void TracePort_Init( void );
extern int  GetRAMSize(void);

extern   u_int32    TMD_HISR_Stack[TM_HISR_STACK_SIZE];
extern   VOID      *TMD_HISR_Stack_Ptr;
extern   UNSIGNED   TMD_HISR_Stack_Size;
extern   INT        TMD_HISR_Priority;

extern NU_HISR kal_int_hisr;
extern u_int32 hisr_stack[KAL_INT_HISR_STACK_SIZE];

extern VOID *FirstAvailMem;
extern VOID *TCD_System_Stack;
extern VOID *TCT_System_Limit;

/* System Memory (we do the memory alloc/dealloc from here for all the objects
   components, etc as N+ does not handle system memory */
extern NU_MEMORY_POOL gSystemMemory;

/* Root Task vars */
NU_TASK gRootTask;
UNSIGNED gpRootStack[STACK_SIZE];

/***************************/
/*   Globals               */
/***************************/
u_int8          NUP_Initialized;
u_int32           tRoot;                     /* Application Root Task - highest priority */

/***************************/
/*   Locals                */
/***************************/
void SysInit();
u_int32 CheckSum(void *StructPtr, int StructSize);


void SysInit() {
    
    NUP_Initialized = 0;
    /* Register Nucleus exceptions handlers, perform misc low level inits */
    INT_Initialize();

    TCD_System_Stack = GetSuperStackBase();
    /* TCD_System_Stack is the higher address since stacks work down */
    TCT_System_Limit = (VOID *)((u_int32)TCD_System_Stack - (u_int32)1024);

    /* Set up some global variables used by NUP's timer
     * component.  These need to be set before the calls to TCC_Create_Hisr
     * are made in TMI_Initialize
     */

    /* Stacks start at higher memory and work down! */
    TMD_HISR_Stack_Ptr = (VOID*)(TMD_HISR_Stack);
    TMD_HISR_Stack_Size = TM_HISR_STACK_SIZE;
    TMD_HISR_Priority = TM_HISR_PRIORITY;

    /* Set the system timer to fire every 2ms */
    hwlib_timer_initialise(2000,TRUE); 

    /* initialize the trace output port; this will hook the a serial port into printf */
    TracePort_Init();

    /* Call INC_Initialize with a pointer to the first available memory
       address after the compiler's global data.  This memory may be used
       by the application.  */
    INC_Initialize(FirstAvailMem);
}


/* Application Initialize gets called from INC_Initialize.
 * The timer hisr is created here because it muse be done
 * after the TM component is initialized.  The stack pointer
 * and size are set up in INT_Initialize
 */
void Application_Initialize(void *pfam) {

    STATUS       func_status;

    int          memory_left;

    NU_Create_HISR(&kal_int_hisr,              /* Pointer to the HISR structure,    */
                                               /* this gets filled by this function */
                   "KALINTHI",                 /* Name for the HISR                 */
                   int_handler,                /* Function entry point              */
                   0,                          /* Priority, 0-2 0 = highest         */
                   hisr_stack,                 /* pointer to stack for HISR task    */
                   KAL_INT_HISR_STACK_SIZE);   /* stack size                        */

    /* Create a single heap containing all the available memory (minus a 4K buffer ?) */
    memory_left = (GetRAMSize() - (int)pfam) - 4096;

    /* Create System memory (one block to contain all the remaining memory */
    func_status = NU_Create_Memory_Pool(&gSystemMemory, 
                                        "SysMem", 
                                        pfam, 
                                        memory_left, 
                                        128, 
                                        NU_PRIORITY);
    if(func_status != NU_SUCCESS)
    {
       /* Wait here */
       while(1);
    }

    /* Create the root task */
    func_status = NU_Create_Task(&gRootTask, 
                                 "RootTask", 
                                 root, 
                                 0, 
                                 NU_NULL,    /* No Args */
                                 (VOID *)&gpRootStack,
                                 STACK_SIZE,
                                 220,        /* Default KAL Priority */
                                 10,         /* KAL Time Slice */
                                 NU_PREEMPT,
                                 NU_START);
    if(func_status!=NU_SUCCESS){
       /* Wait here */
       while(1);
    }

    NUP_Initialized = 1;
}

// void kal_early_initialise() {}
void InitVectors() {}
void OSVariableInit() {}
void StartOS() {
    SysInit();
}

u_int32 CheckSum(void *StructPtr, int StructSize) {
    u_int8 *p = (u_int8 *)StructPtr;
    u_int16 tot = 0;

    while(StructSize--)
        tot += *p++ & 0xFF;
    return tot;
}

/****************************************************************************
 * Modifications:
 * $Log: 
 *  12   mpeg      1.11        2/13/03 12:48:14 PM    Matt Korte      SCR(s) 
 *        5479 :
 *        Removed old header reference
 *        
 *  11   mpeg      1.10        11/6/02 1:32:26 PM     Dave Wilson     SCR(s) 
 *        4907 :
 *        Removed types that were previously defined in types.h. This was a 
 *        pSOS header
 *        that somehow made its way into the Nucleus+ BSP.
 *        
 *  10   mpeg      1.9         10/10/02 2:24:32 PM    Dave Wilson     SCR(s) 
 *        4765 :
 *        Clarified and corrected some commenting. Also replaced a few 
 *        hardcoded
 *        numbers with the correct labels from the chip header file.
 *        
 *  9    mpeg      1.8         2/17/02 12:08:50 PM    Senthil Veluswamy SCR(s) 
 *        3013 :
 *        Removed calls to the pSOS Transkit, checks for NUPKALEX
 *        
 *  8    mpeg      1.7         9/21/01 6:58:10 PM     Senthil Veluswamy SCR(s) 
 *        2574 :
 *        replaced dynamically allocated Root Stack with an Array
 *        
 *  7    mpeg      1.6         9/19/01 8:50:16 PM     Senthil Veluswamy SCR(s) 
 *        2574 :
 *        TKIT is a commndline var. So replaced it with check for NUPKALEX 
 *        inclusion to access TKIT code
 *        
 *  6    mpeg      1.5         9/19/01 5:49:52 PM     Senthil Veluswamy SCR(s) 
 *        2574 :
 *        Made inclusion of the Trans Kit a command line switch (TKIT=YES/NO)
 *        
 *  5    mpeg      1.4         10/2/00 9:25:14 PM     Miles Bintz     combined 
 *        Transkit regions into one
 *        
 *  4    mpeg      1.3         10/2/00 7:16:28 PM     Miles Bintz     
 *        eliminated the Nucleus memory pool and created a gigantic "Region 0"
 *        
 *  3    mpeg      1.2         9/11/00 5:20:16 PM     Joe Kroesche    added 
 *        init call to traceport, took out simple uart init
 *        
 *  2    mpeg      1.1         8/29/00 5:42:10 PM     Ismail Mustafa  Removed 
 *        partition and region create and added the GlobalMemory Pool.
 *        
 *  1    mpeg      1.0         8/25/00 4:25:14 PM     Ismail Mustafa  
 * $
 * 
 *    Rev 1.11   13 Feb 2003 12:48:14   kortemw
 * SCR(s) 5479 :
 * Removed old header reference
 * 
 *    Rev 1.10   06 Nov 2002 13:32:26   dawilson
 * SCR(s) 4907 :
 * Removed types that were previously defined in types.h. This was a pSOS header
 * that somehow made its way into the Nucleus+ BSP.
 * 
 *    Rev 1.9   10 Oct 2002 13:24:32   dawilson
 * SCR(s) 4765 :
 * Clarified and corrected some commenting. Also replaced a few hardcoded
 * numbers with the correct labels from the chip header file.
 * 
 *    Rev 1.8   17 Feb 2002 12:08:50   velusws
 * SCR(s) 3013 :
 * Removed calls to the pSOS Transkit, checks for NUPKALEX
 *
 ****************************************************************************/

