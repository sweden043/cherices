/****************************************************************************/
/*                   CONEXANT PROPRIETARY AND CONFIDENTIAL                  */
/*                       SOFTWARE FILE/MODULE HEADER                        */
/*                 Conexant Systems Inc. (c) 1998, 1999, 2000               */
/*                               Austin, TX                                 */
/*                            All Rights Reserved                           */
/****************************************************************************/
/*
 * Filename:       uartsmpl.h
 *
 *
 * Description:    Function prototypes for the simple uart driver
 *
 *
 * Author:         Miles Bintz
 *
 ****************************************************************************/
/* $Header: uartsmpl.h, 1, 8/29/00 7:13:50 PM, Miles Bintz$
 ****************************************************************************/

#include "stdarg.h"

void     UART_SIMPLE_Init();
int      UART_SIMPLE_putchar_polled(unsigned char thechar);
int      UART_SIMPLE_isr(unsigned long dwIntID, unsigned char bFIQ, void *pfnchain);
int      UART_SIMPLE_vTxStr(char *fmt, va_list ap);
int      UART_SIMPLE_TxStr(char *fmt, ...);
int      UART_SIMPLE_TxStr_polled(char *fmt, ...);
int      UART_SIMPLE_Tx(unsigned char c);
int      UART_SIMPLE_Rx(unsigned char *c);

#if RTOS==NUP

#define  printf       UART_SIMPLE_TxStr
#define  vprintf      UART_SIMPLE_vTxStr
#define  NO_RUNTIME_STDIO

#endif
