/****************************************************************************/
/*                          Conexant Systems Inc.                           */
/****************************************************************************/
/*                                                                          */
/* Filename:       CRTHOOKS.C                                               */
/*                                                                          */
/* Description:    Hook functions required when using the ARM embedded C    */
/*                 runtime libraries.                                       */
/*                                                                          */
/* Author:         Dave Wilson                                              */
/*                                                                          */
/* Copyright Conexant Systems Inc, 2000                                     */
/* All Rights Reserved.                                                     */
/*                                                                          */
/****************************************************************************/

/*****************/
/* Include Files */
/*****************/
#include "hwconfig.h"
#include "kal.h"
#include "trace.h"

typedef struct
{
  unsigned ErrCode;
  char ErrString[252];
} ErrBlock;

typedef unsigned RegSet[16];

/********************/
/* Global variables */
/********************/
int errno;

/********************************************************************/
/*  FUNCTION:    __rt_trap                                          */
/*                                                                  */
/*  PARAMETERS:  err  - Information on the error which occurred     */
/*               regs - Registers at the time of the error          */
/*                                                                  */
/*  DESCRIPTION: Called by the C runtime library to tell is of      */
/*               various runtime errors.                            */
/*                                                                  */
/*  RETURNS:     Does not return.                                   */
/********************************************************************/
void __rt_trap(ErrBlock * err, RegSet regs)
{
  trace_new(TRACE_KAL|TRACE_LEVEL_ALWAYS, "RUNTIME ERROR 0x%08x %s\n",
            err->ErrCode,
            err->ErrString);
  fatal_exit(0xFFFF);          
}

/********************************************************************/
/*  FUNCTION:    __rt_errno_addr                                    */
/*                                                                  */
/*  PARAMETERS:  None                                               */
/*                                                                  */
/*  DESCRIPTION: Return the address of the errno variable to the    */
/*               runtime library.                                   */
/*                                                                  */
/*  RETURNS:     The address of errno.                              */
/********************************************************************/
volatile int *__rt_errno_addr(void)
{
  return(&errno);
}

/********************************************************************/
/*  FUNCTION:    malloc                                             */
/*                                                                  */
/*  PARAMETERS:  size - number of bytes to allocate                 */
/*                                                                  */
/*  DESCRIPTION: Allocate a block of RAM of a given size. This is   */
/*               a wrapper to allow malloc calls to be linked. It   */
/*               redirects the call to the KAL mem_malloc function. */
/*                                                                  */
/*  RETURNS:     Pointer to allocated memory if successful, NULL    */
/*               otherwise.                                         */
/********************************************************************/
void *malloc(size_t size)
{
  return(mem_malloc(size));
}

/********************************************************************/
/*  FUNCTION:    free                                               */
/*                                                                  */
/*  PARAMETERS:  ptr - pointer to block of memory to return to the  */
/*                     heap.                                        */
/*                                                                  */
/*  DESCRIPTION: Free a block of RAM allocated earlier using a call */
/*               to malloc. This is a wrapper that merely passes    */
/*               the call on the the mem_free KAL function.         */
/*                                                                  */
/*  RETURNS:     Nothing                                            */
/********************************************************************/
void free(void *ptr)
{
  mem_free(ptr);
}
