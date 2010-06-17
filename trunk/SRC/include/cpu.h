/****************************************************************************/
/*                            Conexant Systems                              */
/****************************************************************************/
/*                                                                          */
/* Filename:       cpu.h                                                    */
/*                                                                          */
/* Description:    CPU register definitions.                                */
/*                                                                          */
/* Author:         Tim Ross                                                 */
/*                                                                          */
/* Copyright Conexant Systems, 1999                                         */
/* All Rights Reserved.                                                     */
/*                                                                          */
/****************************************************************************/

#ifndef _CPU_H
#define _CPU_H

/*****************/
/* Include Files */
/*****************/
#include <kal.h>

/******************************/
/* ARM CPU Common Definitions */
/******************************/
#ifdef BITFIELDS
typedef struct _PSR {
    BIT_FLD Mode:5;
    BIT_FLD Thumb:1;
    BIT_FLD FIQDisabled:1;
    BIT_FLD IRQDisabled:1;
    BIT_FLD Reserved:20;
    BIT_FLD Overflow:1;
    BIT_FLD Carry:1;
    BIT_FLD Zero:1;
    BIT_FLD Negative:1;
} PSR;
typedef volatile PSR *LPPSR;
#endif

#define CPU_MODE        0x1f
#define CPU_MODE_USER   0x10
#define CPU_MODE_FIQ    0x11
#define CPU_MODE_IRQ    0x12
#define CPU_MODE_SUPER  0x13
#define CPU_MODE_ABORT  0x17
#define CPU_MODE_UNDEF  0x1b
#define CPU_MODE_SYSTEM 0x1f

#define CPU_THUMB       0x20
#define CPU_FIQ_DIS     0x40
#define CPU_IRQ_DIS     0x80

#define CPU_OVERFLOW    0x10000000
#define CPU_CARRY       0x20000000
#define CPU_ZERO        0x40000000
#define CPU_NEGATIVE    0x80000000

#define CPU_RESET_VECT  0x0
#define CPU_UNDEF_VECT  0x4
#define CPU_SWI_VECT    0x8
#define CPU_IABORT_VECT 0xc
#define CPU_DABORT_VECT 0x10
#define CPU_RESV_VECT   0x14
#define CPU_IRQ_VECT    0x18
#define CPU_FIQ_VECT    0x1c

#endif
