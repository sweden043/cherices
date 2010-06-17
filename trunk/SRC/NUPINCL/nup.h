#ifndef _NUPBSP_H_
#define _NUPBSP_H_

#include <nucleus.h>
#include <nup_cfg.h>

#if ((CUSTOMER==CNXT) && defined(DEBUG))
/* #define  POST(x)     { unsigned char *post = (unsigned char*)0x31400080; *post = x; } */
#define  POST(x)     { unsigned long tmp; tmp = x; }
#else

#if CUSTOMER == VENDOR_C
	#define  POST(x)     { unsigned char *post = (unsigned char*)0x31700080; *post = x; }
#else
	#define  POST(x)     { unsigned long tmp; tmp = x + x; }
#endif
#endif

#define  POST_IRQ              0x00
   #define  POST_IRQ_HANDLER           POST_IRQ + 0x00
   #define  POST_IRQ_TIMER             POST_IRQ + 0x01
   #define  POST_IRQ_OTHER             POST_IRQ + 0x02

#define  POST_UART             0x10
	#define  POST_UART_INIT            POST_UART + 0x00
	#define  POST_UART_IRQ             POST_UART + 0x01
		#define  POST_UART_LSR         POST_UART + 0x02
		#define  POST_UART_RXC         POST_UART + 0x03
		#define  POST_UART_THR         POST_UART + 0x04
		#define  POST_UART_MSR         POST_UART + 0x05
	#define  POST_UART_TX_NOW          POST_UART + 0x06
	#define  POST_UART_TX_Q            POST_UART + 0x07

#define  POST_PICINT           0x20
	#define  POST_PICINT_DIS           POST_PICINT + 0x00
	#define  POST_PICINT_EN            POST_PICINT + 0x01

#define  POST_INTMASK          0x30
	#define  POST_INTMASK_MOD          POST_INTMASK + 0x00

#define  POST_BADBAD           0xFF

#endif
