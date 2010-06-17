;/****************************************************************************/
;/*                   CONEXANT PROPRIETARY AND CONFIDENTIAL                  */
;/*                       SOFTWARE FILE/MODULE HEADER                        */
;/*                 Conexant Systems Inc. (c) 1998, 1999, 2000               */
;/*                               Austin, TX                                 */
;/*                            All Rights Reserved                           */
;/****************************************************************************/
;/*                                                                          */
;/* Filename:       nup_inis.s                                               */
;/*                                                                          */
;/*                                                                          */
;/* Description:    initialization functions to pick up where startup left   */
;/*                 off.  modified from ATI's INT_PID.S                      */
;/*                                                                          */
;/* Author:         Miles Bintz                                              */
;/*                                                                          */
;/****************************************************************************/
;/*************************************************************************/
;/*                                                                       */
;/*            Copyright (c) 1994 -1999 Accelerated Technology, Inc.      */
;/*                                                                       */
;/* PROPRIETARY RIGHTS of Accelerated Technology are involved in the      */
;/* subject matter of this material.  All manufacturing, reproduction,    */
;/* use, and sales rights pertaining to this subject matter are governed  */
;/* by the license agreement.  The recipient of this software implicitly  */
;/* accepts the terms of the license.                                     */
;/*                                                                       */
;/*************************************************************************/
;
;/*************************************************************************/
;/*                                                                       */
;/* FILE NAME                                            VERSION          */
;/*                                                                       */
;/*      int_pid.s                                   ARM 6/7/9 1.11.18    */
;/*                                                                       */
;/* COMPONENT                                                             */
;/*                                                                       */
;/*      IN - Initialization                                              */
;/*                                                                       */
;/* DESCRIPTION                                                           */
;/*                                                                       */
;/*      This file contains the target processor dependent initialization */
;/*      routines and data.                                               */
;/*                                                                       */
;/* AUTHOR                                                                */
;/*      William E. Lamie, Accelerated Technology, Inc.                   */
;/*                                                                       */
;/*      Major Revision:                                                  */
;/*                                                                       */
;/*          M. Kyle Craig, Accelerated Technology, Inc.                  */
;/*                                                                       */
;/*                                                                       */
;/*                                                                       */
;/*                                                                       */
;/* DATA STRUCTURES                                                       */
;/*                                                                       */
;/*      INT_Vectors                         Interrupt vector table       */
;/*                                                                       */
;/* FUNCTIONS                                                             */
;/*                                                                       */
;/*      INT_Initialize                      Target initialization        */
;/*      INT_Vectors_Loaded                  Returns a NU_TRUE if all the */
;/*                                            default vectors are loaded */
;/*      INT_Setup_Vector                    Sets up an actual vector     */
;/*                                                                       */
;/* DEPENDENCIES                                                          */
;/*                                                                       */
;/*      nucleus.h                           System constants             */
;/*                                                                       */
;/* HISTORY                                                               */
;/*                                                                       */
;/*         NAME            DATE                    REMARKS               */
;/*                                                                       */
;/*      W. Lamie        08-27-1994      Created initial version 1.0      */
;/*      D. Lamie        08-27-1994      Verified version 1.0             */
;/*      M. Trippi       07-11-1996      New NUCLEUS.H and TMT.S created  */
;/*                                        version 1.2                    */
;/*      M. Trippi       02-18-1997      Released version 1.3             */
;/*      M. Manning      03-04-1997      Released version 1.4             */
;/*      J. Bolton       08-26-1997      Released version 1.5             */
;/*      J. Bolton       10-08-1997      Released version 1.6             */
;/*      M. Kyle Craig   02-05-1999      Revised  version 1.11.17         */
;/*      George Clark    02-05-1999      Verified and Released 1.11.17    */
;/*      B. Whatley      08-12-1999      Verified and Released 1.11.18    */  
;/*                                                                       */
;/*************************************************************************/
;#define         NU_SOURCE_FILE
;
;#include        "nucleus.h"                 /* System constants          */
;
;
;/* Define constants used in low-level initialization.  */
;
;
LOCKOUT         EQU     &C0                 ; Interrupt lockout value
LOCK_MSK        EQU     &C0                 ; Interrupt lockout mask value
MODE_MASK       EQU     &1F                 ; Processor Mode Mask
SUP_MODE        EQU     &13                 ; Supervisor Mode (SVC)

IRQ_MODE        EQU     &12                 ; Interrupt Mode (IRQ)
FIQ_MODE        EQU     &11                 ; Fast Interrupt Mode (FIQ)


; For ARM ports there are three methods of debugging.  
; 
;   1) Using the Embedded ICE or Multi-ICE.
;   2) Using Angel monitor on board in ROM
;   3) Using UMON monitor in conjunction with Nucleus UDB.
;
; For the Embedded ICE tool the NU_ANGELROM_SUPPORT and the NU_UDB_SUPPORT
; should both be set to False.
; For debugging using the Angel monitor, only the NU_ANGELROM_SUPPORT should
; be set to TRUE
; For UDB support only the NU_UDB_SUPPORT should be set to TRUE
; All of this should be set as an assembler argument using the -pd option.
; For example to set NU_ANGELROM_SUPPORT it would be -pd "NU_ANGELROM_SUPPORT SETL {TRUE}"

;
; If using the PID board with the ARM940T daughtercard you will need to assemble
; the file with the option -DNU_ARM9_SUPPORT
;
    AREA |C$$data|,DATA
|x$dataseg|

OLD_UNDEF_VECT
    DCD    &00000000
;
OLD_UNDEF_ADDR
    DCD    &00000000
;
OLD_SWI_VECT
    DCD    &00000000
;
OLD_SWI_ADDR
    DCD    &00000000
;
OLD_IRQ_VECT
    DCD    &00000000
;
OLD_IRQ_ADDR
    DCD    &00000000
;
OLD_FIQ_VECT
    DCD    &00000000
;
OLD_FIQ_ADDR
    DCD    &00000000



;/* Define the initialization flag that indicates whether or not all of the
;   default vectors have been loaded during initialization.  */
;
;INT    INT_Loaded_Flag;

     EXPORT  INT_Loaded_Flag
INT_Loaded_Flag
    DCD     &00000000

;

    IF :DEF: |ads$version|
    AREA |C$$code|, CODE, READONLY
    ELSE
    AREA |C$$code|, CODE, READONLY,INTERWORK
    ENDIF

;/* Define the global system stack variable.  This is setup by the 
;   initialization routine.  */
;
;extern VOID            *TCD_System_Stack;
;
    IMPORT  TCD_System_Stack
    IMPORT  TCT_System_Limit
   IF :LNOT:(:DEF: |ads$version|)
    IMPORT  Initialise940
   ENDIF

    IMPORT  IRQ_Wrapper
	IMPORT  Other_ISR
    IMPORT  CleanDCache
    IMPORT  FlushDCache
    IMPORT  FlushICache
    IMPORT  DrainWriteBuffer

    EXPORT  INT_Vectors
INT_Vectors
    LDR    pc,INT_Table
    LDR    pc,(INT_Table + 4)
    LDR    pc,(INT_Table + 8)
    LDR    pc,(INT_Table + 12)
    LDR    pc,(INT_Table + 16)
    LDR    pc,(INT_Table + 20)
    LDR    pc,(INT_Table + 24)
    LDR    pc,(INT_Table + 28)


    EXPORT    INT_Table
INT_Table

INT_Initialize_Addr        DCD    INT_Initialize
Undef_Instr_Addr           DCD    Other_ISR
SWI_Addr                   DCD    Other_ISR 
Prefetch_Abort_Addr        DCD    Other_ISR
Data_Abort_Addr            DCD    Other_ISR
FIQ_Handler_Addr           DCD    Other_ISR
IRQ_Handler_Addr           DCD    IRQ_Wrapper
Undefined_Addr             DCD    0                ; NO LONGER USED

Loaded_Flag
    DCD     INT_Loaded_Flag

;
;
;/*************************************************************************/
;/*                                                                       */
;/* FUNCTION                                                              */
;/*                                                                       */
;/*      INT_Initialize                                                   */
;/*                                                                       */
;/* DESCRIPTION                                                           */
;/*                                                                       */
;/*      This function sets up the global system stack variable and       */
;/*      transfers control to the target independent initialization       */
;/*      function INC_Initialize.  Responsibilities of this function      */
;/*      include the following:                                           */
;/*                                                                       */
;/*             - Setup necessary processor/system control registers      */
;/*             - Initialize the vector table                             */
;/*             - Setup the system stack pointers                         */
;/*             - Setup the timer interrupt                               */
;/*             - Calculate the timer HISR stack and priority             */
;/*             - Calculate the first available memory address            */
;/*             - Transfer control to INC_Initialize to initialize all of */
;/*               the system components.                                  */
;/*                                                                       */
;/* AUTHOR                                                                */
;/*                                                                       */
;/*      William E. Lamie, Accelerated Technology, Inc.                   */
;/*                                                                       */
;/*      Major Revision:                                                  */
;/*                                                                       */
;/*          M. Kyle Craig, Accelerated Technology, Inc.                  */
;/*                                                                       */
;/*                                                                       */
;/*                                                                       */
;/*                                                                       */
;/* CALLED BY                                                             */
;/*                                                                       */
;/*      Nothing. This function is the ENTRY point for Nucleus PLUS.      */
;/*                                                                       */
;/* CALLS                                                                 */
;/*                                                                       */
;/*      INC_Initialize                      Common initialization        */
;/*                                                                       */
;/* INPUTS                                                                */
;/*                                                                       */
;/*      None                                                             */
;/*                                                                       */
;/* OUTPUTS                                                               */
;/*                                                                       */
;/*      None                                                             */
;/*                                                                       */
;/* HISTORY                                                               */
;/*                                                                       */
;/*         NAME            DATE                    REMARKS               */
;/*                                                                       */
;/*      W. Lamie        08-27-1994      Created initial version 1.0      */
;/*      D. Lamie        08-27-1994      Verified version 1.0             */
;/*                                                                       */
;/*************************************************************************/
;VOID    INT_Initialize(void)
;{
    EXPORT  INT_Initialize
INT_Initialize


;    /* Disable interrupts.  They should stay disabled until
;       INC_Initialize calls TCT_Schedule which will turn them
;       back on
;     */

    MRS    r0, CPSR
    BIC    r0,r0,#MODE_MASK
    ORR    r0,r0,#SUP_MODE
    ORR    r0,r0,#LOCKOUT

    MSR    CPSR_cxsf,r0


;/* This code actually copies the 'LDR pc, address' instructions
;   and the associated addresses to location 0. Nucleus PLUS
;   uses the IRQ vector for the system timer and serial, and
;   PLUS also supports FIQ interrupts.

;   All other vectors are copied for completeness.
;   Reference interrupt handlers for Undefined Instruction,
;   SWI, Prefetch Abort, Data Abort, are provided at the
;   bottom of this file.
;
;   Please see ARM User Guide Chapter 13, Writing Code for ROM
;   for more details.         
;*/
;
; following code is from INT_Install_Vector_Table in Nucleus distribution
;
    STMDB    sp!,{r0-r9,lr}                  ; Save registers on stack

    MOV     r8, #0x00                       ; Pickup address of vector table (0x00000000)
    LDR     r9, =INT_Vectors                ; Pickup address of our vector table
    LDMIA   r9!,{r0-r7}                     ; Load vector table values into registers
    STMIA   r8!,{r0-r7}                     ; Store vector table values at correct address
    LDMIA   r9!,{r0-r7}                     ; Load vector table values into registers
    STMIA   r8!,{r0-r7}                     ; Store vector table values at correct address

    LDMIA   sp!,{r0-r9,lr}                  ; Restore registers

; conexant chip specific cache cleaning
    stmdb    sp!, {lr}                      ; Save lr
    bl       CleanDCache                    ; Clean D cache
    bl       FlushDCache                    ; Flush D cache
    bl       FlushICache                    ; Flush I cache
    bl       DrainWriteBuffer               ; Empty write posting buffer
    ldmia    sp!, {lr}                      ; Restore lr
;
     BX     lr                              ; Return to caller

;}
;
;
;
;/*************************************************************************/
;/*                                                                       */
;/* FUNCTION                                                              */
;/*                                                                       */
;/*      INT_Vectors_Loaded                                               */
;/*                                                                       */
;/* DESCRIPTION                                                           */
;/*                                                                       */
;/*      This function returns the flag that indicates whether or not     */
;/*      all the default vectors have been loaded.  If it is false,       */
;/*      each LISR register also loads the ISR shell into the actual      */
;/*      vector table.                                                    */
;/*                                                                       */
;/* AUTHOR                                                                */
;/*                                                                       */
;/*      William E. Lamie, Accelerated Technology, Inc.                   */
;/*                                                                       */
;/*                                                                       */
;/*      Major Revision:                                                  */
;/*                                                                       */
;/*          M. Kyle Craig, Accelerated Technology, Inc.                  */
;/*                                                                       */
;/*                                                                       */
;/*                                                                       */
;/* CALLED BY                                                             */
;/*                                                                       */
;/*      TCC_Register_LISR                   Register LISR for vector     */
;/*                                                                       */
;/* CALLS                                                                 */
;/*                                                                       */
;/*      None                                                             */
;/*                                                                       */
;/* INPUTS                                                                */
;/*                                                                       */
;/*      None                                                             */
;/*                                                                       */
;/* OUTPUTS                                                               */
;/*                                                                       */
;/*      None                                                             */
;/*                                                                       */
;/* HISTORY                                                               */
;/*                                                                       */
;/*         NAME            DATE                    REMARKS               */
;/*                                                                       */
;/*      W. Lamie        08-27-1994      Created initial version 1.0      */
;/*      D. Lamie        08-27-1994      Verified version 1.0             */
;/*                                                                       */
;/*************************************************************************/
;INT    INT_Vectors_Loaded(void)
;{
    EXPORT  INT_Vectors_Loaded
INT_Vectors_Loaded
;
;    /* Just return the loaded vectors flag.  */
;    return(INT_Loaded_Flag);
;
     LDR    r1,Loaded_Flag                  ; Get the address
     LDR    r0,[r1,#0]                      ; Load current value

     BX     lr                              ; Return to caller
;}
;
;
;/*************************************************************************/
;/*                                                                       */
;/* FUNCTION                                                              */
;/*                                                                       */
;/*      INT_Setup_Vector                                                 */
;/*                                                                       */
;/* DESCRIPTION                                                           */
;/*                                                                       */
;/*      This function sets up the specified vector with the new vector   */
;/*      value.  The previous vector value is returned to the caller.     */
;/*                                                                       */
;/* AUTHOR                                                                */
;/*                                                                       */
;/*      William E. Lamie, Accelerated Technology, Inc.                   */
;/*                                                                       */
;/*                                                                       */
;/*      Major Revision:                                                  */
;/*                                                                       */
;/*          M. Kyle Craig, Accelerated Technology, Inc.                  */
;/*                                                                       */
;/*                                                                       */
;/*                                                                       */
;/* CALLED BY                                                             */
;/*                                                                       */
;/*      Application                                                      */
;/*      TCC_Register_LISR                   Register LISR for vector     */
;/*                                                                       */
;/* CALLS                                                                 */
;/*                                                                       */
;/*      None                                                             */
;/*                                                                       */
;/* INPUTS                                                                */
;/*                                                                       */
;/*      vector                              Vector number to setup       */
;/*      new                                 Pointer to new assembly      */
;/*                                            language ISR               */
;/*                                                                       */
;/* OUTPUTS                                                               */
;/*                                                                       */
;/*      old vector contents                                              */
;/*                                                                       */
;/* HISTORY                                                               */
;/*                                                                       */
;/*         NAME            DATE                    REMARKS               */
;/*                                                                       */
;/*      W. Lamie        08-27-1994      Created initial version 1.0      */
;/*      D. Lamie        08-27-1994      Verified version 1.0             */
;/*                                                                       */
;/*************************************************************************/
;VOID  *INT_Setup_Vector(INT vector, VOID *new)
;{
    EXPORT  INT_Setup_Vector
INT_Setup_Vector
;
;VOID    *old_vector;                        /* Old interrupt vector      */
;VOID   **vector_table;                      /* Pointer to vector table   */
;
;    /* Calculate the starting address of the actual vector table.  */
;    vector_table =  (VOID **) 0;
;
;    /* Pickup the old interrupt vector.  */
;    old_vector =  vector_table[vector];
;    
;    /* Setup the new interrupt vector.  */
;    vector_table[vector] =  new;
;    
;    /* Return the old interrupt vector.  */
;    return(old_vector);
;

; This function is actually not used in our BSP but needs to resolve to
; keep the linker happy.  So make it an infinite loop and we can trap
; here if somebody actually calls it.

     b INT_Setup_Vector

;}
;
;
;
;/*************************************************************************/
;/*                                                                       */
;/* FUNCTION                                                              */
;/*                                                                       */
;/*      INT_Retrieve_Shell                                               */
;/*                                                                       */
;/* DESCRIPTION                                                           */
;/*                                                                       */
;/*      This function retrieves the pointer to the shell interrupt       */
;/*      service routine.  The shell interrupt service routine calls      */
;/*      the LISR dispatch routine.                                       */
;/*                                                                       */
;/* AUTHOR                                                                */
;/*                                                                       */
;/*      William E. Lamie, Accelerated Technology, Inc.                   */
;/*                                                                       */
;/*                                                                       */
;/*      Major Revision:                                                  */
;/*                                                                       */
;/*          M. Kyle Craig, Accelerated Technology, Inc.                  */
;/*                                                                       */
;/*                                                                       */
;/*                                                                       */
;/* CALLED BY                                                             */
;/*                                                                       */
;/*      TCC_Register_LISR                   Register LISR for vector     */
;/*                                                                       */
;/* CALLS                                                                 */
;/*                                                                       */
;/*      None                                                             */
;/*                                                                       */
;/* INPUTS                                                                */
;/*                                                                       */
;/*      vector                              Vector number to setup       */
;/*                                                                       */
;/* OUTPUTS                                                               */
;/*                                                                       */
;/*      shell pointer                                                    */
;/*                                                                       */
;/* HISTORY                                                               */
;/*                                                                       */
;/*         NAME            DATE                    REMARKS               */
;/*                                                                       */
;/*      W. Lamie        08-27-1994      Created initial version 1.0      */
;/*      D. Lamie        08-27-1994      Verified version 1.0             */
;/*                                                                       */
;/*************************************************************************/
;VOID  *INT_Retrieve_Shell(INT vector)
;{
    EXPORT  INT_Retrieve_Shell
INT_Retrieve_Shell
;
;    /* Return the LISR Shell interrupt routine.  */
;    return(INT_Vectors[vector]);
;
; This function is actually not used in our BSP but needs to resolve to
; keep the linker happy.  So make it an infinite loop and we can trap
; here if somebody actually calls it.

     b INT_Retrieve_Shell


;}

    END

