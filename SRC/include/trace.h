/****************************************************************************/
/*                 Rockwell Semiconductor Systems - SABINE                  */
/****************************************************************************/
/*                                                                          */
/* Filename:           TRACE.H                                              */
/*                                                                          */
/* Description:        Public header file for the KAL Trace and Error       */
/*                     logging functions                                    */
/*                                                                          */
/* Author:             Dave Wilson                                          */
/*                                                                          */
/* Copyright Conexant Systems Inc, 1999                                     */
/* All Rights Reserved.                                                     */
/*                                                                          */
/****************************************************************************/
/* $Id: trace.h,v 1.22, 2004-05-18 15:24:09Z, Xin Golden$
 ****************************************************************************/

#ifndef _TRACE_H_
#define _TRACE_H_

#include "stbcfg.h" /* for ENABLE_TRACE_IN_RELEASE */

#define TRACE_USE_COLOR  1

/***************************/
/***************************/
/* Trace and error logging */
/***************************/
/***************************/

/* Max number of chars in a trace msg */
#define MAX_TRACEMSG_CHARS 161
/* Max number of trace msgs in trace queue */
#if (RTOS == UCOS)
#define MAX_TRACEMSGS       150
#else
#define MAX_TRACEMSGS       200
#endif
#define MAX_ISR_DEBUG_QUEUE 50

/************************************************************************/
/* This number will vary depending upon the RTOS in use. For pSOS, it   */
/* is defined as LC_SSIZE in sys_conf.h but using this label here would */
/* make the trace component RTOS-dependent. Though it's not a very nice */
/* thing to do, I redefine the label here with the same value - sorry ! */
/************************************************************************/
#define PRT_BUFF_SIZE       512


#if !(defined TRACE_OUTPUT) && (ENABLE_TRACE_IN_RELEASE == NO)
void error_log(int error_num);
#define isr_error_log error_log
#define cs_error_log error_log
#else
void debug_error_log(int error_num, char *file, int linenum);
#define error_log(x) debug_error_log((x), __FILE__, __LINE__);
void debug_isr_error_log(int error_num, char *file, int linenum);
#define isr_error_log(x) debug_isr_error_log((x), __FILE__, __LINE__);
void debug_cs_error_log(int error_num, char *file, int linenum);
#define cs_error_log(x) debug_cs_error_log((x), __FILE__, __LINE__);
#endif

bool trace_init(void);

#if (defined TRACE_OUTPUT) || (ENABLE_TRACE_IN_RELEASE == YES)
extern u_int32 uTraceFlags;
void trace(char *string, ...);
void trace_new(u_int32 flags, char *string, ...);
void isr_trace(char *string, u_int32 value1, u_int32 value2);
void isr_trace_new(u_int32 flags, char *string, u_int32 value1, u_int32 value2);
void cs_trace(u_int32 flags, char *string, u_int32 value1, u_int32 value2);
void flush_trace_on_abort(void);
void trace_set_level(u_int32 flags);
#define trace_get_level(var)  (*(var) = uTraceFlags)
void checkpoint(int checkpoint_num);
#else
/* In non-Debug builds, trace calls are compiled to nothing */
#define trace                if (0) ((int (*)(char *, ...)) 0)
#define trace_new            if (0) ((int (*)(u_int32,char *, ...)) 0)
#define isr_trace            if (0) ((int (*)(char*,u_int32,u_int32)) 0)
#define isr_trace_new        if (0) ((int (*)(u_int32,char*,u_int32,u_int32)) 0)
#define cs_trace             if (0) ((int (*)(u_int32,char*,u_int32,u_int32)) 0)
#define flush_trace_on_abort if (0) ((int (*)(void)) 0)
#define trace_set_level(x)   ((void)(x))
#define trace_get_level(var) ((void)*(var))
#define checkpoint           if (0) ((int (*)(int)) 0)
#endif


/* Flag indicating a fatal error - top bit or error_num */
#define ERROR_FATAL   0x80000000
#define ERROR_WARNING 0x00000000

/* Masks for the component generating an error and it's error number */
#define ERROR_SOURCE 0x00FF0000
#define ERROR_VALUE  0x0000FFFF

/* Flags for trace levels */
#define TRACE_MASK_MODULE  0x0FFFFFFF
#define TRACE_LEVEL_MASK   0x70000000
#define TRACE_TIMESTAMP_MASK    0x80000000

#define TRACE_LEVEL_NEVER  0x00000000
#define TRACE_LEVEL_1      0x10000000
#define TRACE_LEVEL_2      0x20000000
#define TRACE_LEVEL_3      0x30000000
#define TRACE_LEVEL_4      0x40000000
#define TRACE_LEVEL_5      0x50000000
#define TRACE_LEVEL_6      0x60000000
#define TRACE_LEVEL_ALWAYS 0x70000000

#define TRACE_NO_TIMESTAMP 0x80000000

#define TRACE_ALL          0x0FFFFFFF
#define TRACE_KAL          0x00000001
#define TRACE_XTV          0x00000002
#define TRACE_DVR TRACE_XTV
#define TRACE_ATA TRACE_XTV
#define TRACE_SMC          0x00000004
#define TRACE_SER          0x00000008
#define TRACE_AR           0x00000010
#define TRACE_RTC          0x00000020
#define TRACE_RST          0x00000040
#define TRACE_AUD          0x00000080
#define TRACE_MPG          0x00000100
#define TRACE_MOV          0x00000200
#define TRACE_IRD          0x00000400
#define TRACE_I2C          0x00000800
#define TRACE_GCP          0x00001000
#define TRACE_ETH          0x00002000
#define TRACE_NET TRACE_ETH
#define TRACE_OSD          0x00004000
#define TRACE_DPS          0x00008000
#define TRACE_KEY          0x00010000
#define TRACE_BTN TRACE_KEY
#define TRACE_CM           0x00020000
#define TRACE_CM_MUX       TRACE_CM
#define TRACE_TSTHOSTIF    TRACE_CM
#define TRACE_CTL          0x00040000
#define TRACE_GEN          0x00080000
#define TRACE_NV           0x00100000
#define TRACE_CI           0x00200000
#define TRACE_CA           0x00400000
#define TRACE_DMD          0x00800000
#define TRACE_OCD          0x01000000
#define TRACE_TST          0x02000000
#define TRACE_MODEM        0x04000000
#define TRACE_CR           0x08000000

#if TRACE_USE_COLOR

#define TRACE_FG_LIGHT_BLUE            "\033[1;34m"
#define TRACE_FG_LIGHT_GREEN           "\033[1;32m"
#define TRACE_FG_LIGHT_CYAN            "\033[1;36m"
#define TRACE_FG_LIGHT_RED             "\033[1;31m"
#define TRACE_FG_WHITE                 "\033[1;37m"
#define TRACE_FG_NORMAL                TRACE_FG_WHITE
#define TRACE_BG_GRAY                  "\033[0;47m"
#define TRACE_BG_BLACK                 "\033[0;40m"

#else

#define TRACE_FG_LIGHT_BLUE            ""
#define TRACE_FG_LIGHT_GREEN           ""
#define TRACE_FG_LIGHT_CYAN            ""
#define TRACE_FG_WHITE                 ""
#define TRACE_FG_NORMAL                TRACE_FG_WHITE
#define TRACE_BG_GRAY                  ""
#define TRACE_BG_BLACK                 ""

#endif

/**********************/
/* Timestamp Function */
/**********************/
#if (defined TRACE_OUTPUT) || (ENABLE_TRACE_IN_RELEASE == YES)
/* When trace is enabled, this aliases to trace_new */
#define timestamp_message(str) trace_new(TRACE_LEVEL_ALWAYS|TRACE_MASK_MODULE, (str))
#else
/* When trace is disabled, we have a special function for timstamp messages */
void timestamp_message(char *str);
#endif

/******************/
/* Internal Flags */
/******************/
#define PROCESS_EXIT ((u_int32)-1)
#define TRACE_MSG    ((u_int32)1)
#define ERROR_MSG    ((u_int32)2)
#define TRACE_Q_MSG  ((u_int32)3)
#define TIME_MSG     ((u_int32)4)
#define TRACE_MSG_NO_TS ((u_int32)5)

#endif /* _TRACE_H_ */

/****************************************************************************
 * Modifications:
 * $Log: 
 *  23   mpeg      1.22        5/18/04 10:24:09 AM    Xin Golden      CR(s) 
 *        9222 9223 : modified MAX_TRACEMSGS for ucos because the number of 
 *        large msg buffers can be only 180 for ucos which can cause qu_send 
 *        failed in some cases.
 *  22   mpeg      1.21        4/27/04 3:36:21 PM     Miles Bintz     CR(s) 
 *        8979 8980 : switch on TRACE_OUTPUT define instead of debug
 *  21   mpeg      1.20        10/22/03 7:25:00 AM    Dave Wilson     CR(s): 
 *        7693 Added prototype for timestamp_message
 *  20   mpeg      1.19        9/16/03 5:49:50 PM     Angela Swartz   SCR(s) 
 *        7477 :
 *        #include stbcfg.h to pickup #define of ENABLE_TRACE_IN_RELEASE
 *        
 *  19   mpeg      1.18        9/16/03 4:36:10 PM     Angela Swartz   SCR(s) 
 *        7477 :
 *        Enable trace output if ENABLE_TRACE_IN_RELEASE is set to YES in the 
 *        sw config file for release build
 *        
 *  18   mpeg      1.17        9/11/02 4:52:18 PM     Miles Bintz     SCR(s) 
 *        4583 :
 *        Added defines for ANSI color codes
 *        
 *  17   mpeg      1.16        7/8/02 5:09:16 PM      Matt Korte      SCR(s) 
 *        4152 :
 *        Change trace_get_level to take pointer as parameter so it is function
 *         like.
 *        
 *        
 *  16   mpeg      1.15        6/7/02 11:08:54 AM     Matt Korte      SCR(s) 
 *        3961 :
 *        Added trace_get_level, made trace_set_level return void.
 *        
 *        
 *  15   mpeg      1.14        5/24/02 4:41:38 PM     Matt Korte      SCR(s) 
 *        3856 :
 *        Change #define for trace_set_level so that it doesn't generate 
 *        warnings.
 *        Fixed problems with the header and log of the file.
 *        
 *        
 *  14   mpeg      1.13        5/23/02 9:24:42 AM     Bobby Bradford  SCR(s) 
 *        3840 :
 *        Modified TRACE_LEVEL definitions to use 3 bits instead of 4,
 *        and added support for TRACE_NO_TIMESTAMP bit flag (bit 31)
 *        
 *  13   mpeg      1.12        4/2/02 4:02:28 PM      Tim White       SCR(s) 
 *        3491 :
 *        Changed cs_xxxx error log definitions.
 *        
 *        
 *  12   mpeg      1.11        1/9/02 4:18:48 PM      Dave Moore      SCR(s) 
 *        3006 :
 *        Added TRACE_CM for WaBASH Cable
 *        
 *        
 *  11   mpeg      1.10        9/19/01 11:00:56 AM    Dave Moore      SCR(s) 
 *        2647 2648 :
 *        
 *        
 *  10   mpeg      1.9         6/7/01 3:33:46 PM      Tim White       SCR(s) 
 *        1958 1990 :
 *        Modified tracing facility to provide system time stamps.
 *        
 *        
 *  9    mpeg      1.8         5/4/01 2:29:20 PM      Tim White       DCS#1822,
 *         DCS#1824, DCS31825 -- Critical Section Overhaul
 *        
 *  8    mpeg      1.7         2/28/01 11:28:56 AM    Dave Wilson     DCS1329: 
 *        Reduced buffering from 100 messages to 50
 *        
 *  7    mpeg      1.6         1/23/01 10:26:14 AM    Dave Wilson     DCS987: 
 *        Made sure that no unused code is included in the release build.
 *        
 *  6    mpeg      1.5         12/11/00 12:53:06 PM   Tim White       Removed 
 *        TRACE_TUN, added TRACE_XTV hook number 0x00000002.
 *        
 *  5    mpeg      1.4         11/2/00 1:48:52 PM     Anzhi Chen      Added 
 *        TRACE_AR for audience research driver.
 *        
 *  4    mpeg      1.3         6/23/00 2:35:24 PM     Tim White       Added 
 *        TRACE_DVR hook.  Removed TRACE_BSP since it's not used.
 *        
 *  3    mpeg      1.2         12/6/99 3:14:36 PM     Dave Wilson     Added 
 *        flush_trace_on_abort
 *        
 *  2    mpeg      1.1         9/13/99 2:28:52 PM     Dave Wilson     Trace & 
 *        error functions split out from KAL.
 *        
 *  1    mpeg      1.0         9/10/99 6:05:34 PM     Dave Wilson     
 * $
 * 
 ****************************************************************************/

