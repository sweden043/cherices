/****************************************************************************/
/*                   CONEXANT PROPRIETARY AND CONFIDENTIAL                  */
/*                       SOFTWARE FILE/MODULE HEADER                        */
/*                 Conexant Systems Inc. (c) 1998, 1999, 2000               */
/*                               Austin, TX                                 */
/*                            All Rights Reserved                           */
/****************************************************************************/
/*
 * Filename:       sky_cmod.h
 *
 *
 * Description:    BiB CMODULE compliant driver for HW (ACF) Modem 
 *                 
 *
 * Author:         Dave Moore
 *
 ****************************************************************************/
/* $Header: sky_cmod.h, 17, 2/13/03 12:31:34 PM, Matt Korte$
 ****************************************************************************/

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include "retcodes.h"
#include "opentvx.h"
#include "kal.h"
#include "init.h"
#include "iic.h"
#include "hwlib.h"

/* Modem System Query and Control Externals */
extern o_modem_attributes cmod_attributes ;
extern o_modem_output cmod_output ;
extern o_modem_connect cmod_connect ;
extern o_modem_status cmod_status ;
extern o_modem_sys_desc cmod_sys_desc ;

/* Serial System Query and Control Externals */
extern o_serial_attributes serial_attributes[];

/*----------------------------------------------------------------------*/
/* Register addresses (relative to the base address)                    */
/*----------------------------------------------------------------------*/
#define TX      0  /* TX holding register                 */
#define RX      0  /* RX holding register                 */
#define DLLO    0  /* Divisor latch low                   */
#define DLHI    1  /* Divisor latch high                  */
#define IENB    1  /* Interrupt enable register           */
/* Interrupt Identification register   */
#define IIR ( cmod_attributes.use_external_modem ? 6 : 2 )
#define FCR     2  /* FIFO control register (16550A only) */
#define LINECTL 3  /* Line control register               */
#define MODCTL  4  /* Modem control register              */
#define LSTAT   5  /* Line status register                */
#define MODSTAT 6  /* Modem status register               */
#define SPR     7  /* Scratch Pad Register                */

/*----------------------------------------------------------------------*/
/* Define bits in the Interrupt Enable register                         */
/*----------------------------------------------------------------------*/
#define    RXINT    0x01         /* Enable receive interrupt            */
#define    TXINT ( cmod_attributes.use_external_modem ? 0x20 : 0x02 )
#define    LSINT ( cmod_attributes.use_external_modem ? 0x00 : 0x04 )
#define    MSINT    0x08         /* Enable modem status interrupt       */

/*----------------------------------------------------------------------*/
/* Define bits in the Modem Control register (MODCTL)                   */
/*----------------------------------------------------------------------*/
#define    DTR    0x01           /* Data Terminal Ready                 */
#define    RTS    0x02           /* Request To Send                     */
#define    OUT1   0x04           /* Auxiliary out                       */
#define    OUT2   0x08           /* Enables interrupts from SmartSCM    */

/*----------------------------------------------------------------------*/
/* Define bits in the Modem Status Register (MODSTAT)                   */
/*----------------------------------------------------------------------*/
#define MOD_DCTS  0x01           /* delta CTS indicator                 */
#define MOD_DDSR  0x02           /* delta DSR indicator                 */
#define MOD_TERI  0x04           /* Trailing Edge of RI detector        */
#define MOD_DDCD  0x08           /* delta DCD indicator                 */
#define MOD_CTS   0x10           /* CTS signal                          */
#define MOD_DSR   0x20           /* DSR  signal                         */
#define MOD_RI    0x40           /* RI   signal                         */
#define MOD_DCD   0x80           /* DCD  signal                         */


/*----------------------------------------------------------------------*/
/* Define the bits in Line Status Register (LSR)                        */
/*----------------------------------------------------------------------*/
#define    LSR_DR     0x01        /* Data ready                         */
#define    LSR_OE     0x02        /* Overrun error                      */
#define    LSR_PE     0x04        /* Parity error                       */
#define    LSR_FE     0x08        /* Framing error                      */
#define    LSR_BI     0x10        /* Break interrupt                    */
#define    LSR_THRE   0x20        /* Transmitter holding register empty */
#define    LSR_TEMT   0x40        /* Transmitter empty                  */
#define    LSR_FFE    0x80        /* Receiver FIFO error                */


/*----------------------------------------------------------------------*/
/* Define the bits in Line Control Register (LCR)                       */
/*----------------------------------------------------------------------*/
#define  CHAR_SIZE_MASK ( cmod_attributes.use_external_modem ? 0x01 : 0x03 )
#define  STOP_BITS_MASK   0x04
#define  PARITY_BIT_MASK  0x38

#define  STOP_BITS_2      0x04    /*two stop bits if char size is 6,7,8 */
#define  STOP_BITS_1_5    0x04    /*1.5 stop bits if char size is 5 bits*/
#define  STOP_BITS_1      0x00    /* no bits needs to be set            */
#define  PARITY_ODD       0x08    /* enable odd parity bit              */
#define  PARITY_EVEN      0x18    /* enable even parity bit             */
#define  PARITY_STICK_0   0x38    /* parity bit is always 0             */
#define  PARITY_STICK_1   0x28    /* parity bit is always 1             */
#define  TR_BREAK_SIGNAL  0x40    /* send the break condtion            */
#define  DLA_BIT          0x80    /* Allow baud rate divisor access     */


/*----------------------------------------------------------------------*/
/* Define the bits in FIFO Control Register (FCR) (Write-Only)          */
/*----------------------------------------------------------------------*/
#define  ENABLE_FIFO      0x01    /* 1: enable, 0 disable               */
#define  CLEAR_RCV_FIFO   0x02    /* Clear RCV FIFO (self-clearing bit) */
#define  CLEAR_XMIT_FIFO  0x04    /* Clear XMIT FIFO (self-clearing bit)*/

/* 1655X (SmartSCM) */
#define  FIFO_THRES_MASK  0xC0
#define  FIFO_THRES_1     0x00    /* Trigger Level 1                    */
#define  FIFO_THRES_4     0x40    /* Trigger Level 4                    */
#define  FIFO_THRES_8     0x80    /* Trigger Level 8                    */
#define  FIFO_THRES_14    0xC0    /* Trigger Level 16                   */

/* Cnxt Uart */
#define  RX_FIFO_1_PLUS    0x00   /* Trigger on 1 or more */
#define  RX_FIFO_4_PLUS    0x40   /* Trigger on 4 or more */
#define  RX_FIFO_8_PLUS    0x80   /* Trigger on 8 or more */
#define  RX_FIFO_12_PLUS   0xC0   /* Trigger on 12 or more */
#define  TX_FIFO_4_PLUS    0x00   /* Trigger on 4 or more empty */
#define  TX_FIFO_8_PLUS    0x10   /* Trigger on 8 or more empty */
#define  TX_FIFO_12_PLUS   0x20   /* Trigger on 12 or more empty */
#define  TX_FIFO_EMPTY     0x30   /* Trigger on completely empty */

/*----------------------------------------------------------------------*/
/* Define the bits in Interrupt Identification Register (read-only)     */
/*----------------------------------------------------------------------*/
#define  FIFO_EXIST       0xC0    /* set when in FIFO mode              */
#define  INTR_ID_MASK     0x07
#define  INTR_NOTHING     0x01    /* no interrupt                       */
#define  INTR_RCV_ERR     0x06    /* receive error interrupt            */
#define  INTR_RCV_AVAIL   0x04    /* receive data available             */
#define  INTR_XMT_EMPT    0x02    /* Xmit Holding Register empty        */
#define  INTR_MOD_STAT    0x00    /* Modem status changed               */

#ifndef min
#define min((a),(b)) ( (a) < (b) ? (a) : (b) )
#endif

#define CR '\015'
#define LF '\012'
#define CLOCK_INPUT ( cmod_attributes.use_external_modem ? 54000000 : 28224000 )
#define SCRATCH_LEN 40  
#define CARRIER_WAIT_250MS_UNITS 250  /* should be >> S7 */

/* SmartSCM Supported Country Codes */
#define AUSTRALIA "09" 
#define Hungary "51" 
#define POLAND "8A"
#define AUSTRIA "0A" 
#define INDIA "53" 
#define PORTUGAL "8B"
#define BELGIUM "0F" 
#define IRELAND "57" 
#define RUSSIA "B8"
#define BULGARIA "1B" 
#define ISRAEL "58" 
#define SINGAPORE "9C"
#define CANADA "20" 
#define ITALY "59" 
#define SPAIN "A0"
#define CHINA "26" 
#define JAPAN "00" 
#define SWEDEN "A5"
#define CZECH_REPUBLIC  "2E"
#define SLAVAK_REPUBLIC "2E"
#define KOREA "61" 
#define SWITZERLAND "A6"
#define DENMARK "31" 
#define LUXEMBOURG "69" 
#define TAIWAN "FE"
#define FINLAND "3C" 
#define MEXICO "73" 
#define UNITED_KINGDOM "B4"
#define FRANCE "3D" 
#define NETHERLANDS "7B" 
#define UNITED_STATES "B5"
#define GERMANY "42" 
#define NEW_ZEALAND "7E"
#define GREECE "46" 
#define NORWAY "82"
#define HONG_KONG "50" 
#define PHILIPPINES "89"
#define TBR21 "FD"

/* IMPORTANT!!! - The 5 defines below should NOT BE MODIFIED else you will           */
/*  break the modem driver. That being said, if you know what you are doing and      */
/*  want to eliminate one of these set it to the null string "" but don't delete it! */
/*     i.e. #define XXX_PREFIX ""                                                    */

/* NOTE: the maximum length of the body of an "AT" command string is 40 characters   */
/*       for the SmartSCM Modem.                                                     */

#define MODEM_RESET "ATZ"

/*line detection hyteresis levels (volts) */
#define LINE_CONNECTED_THRESHOLD 20.0 /* 20 + */
#define LINE_BUSY_THRESHOLD 3.0 /* 3 - LINE_CONNECTED_THRESHOLD */
/* lowest reported voltage should be 1.4 even when no line connection */
#define LINE_NOT_CONNECTED_THRESHOLD 0.0 /* 0 - LINE_BUSY_THRESHOLD */

/* WS: moved BASIC_PREFIX_1 & 2 defines into the vendor header files */ 

#define VOICE_PREFIX "AT+FCLASS=8"                        /* set voice service class */
#define DATA_PREFIX  "AT+FCLASS=0"                        /* set data service class */


#define SET 1
#define CLEAR 0
#define GET_GPIO_PIN_STATE( a ) ( *(unsigned int volatile *)0x30470004 & ( 1<<a ) )


#define TX_EMPTY 0
#define TX_DATA_AVAIL 1
#define RCV_DATA_AVAIL 2
#define RCV_DATA_OVERFLOW 4
#define TRANSMIT_BUFFER_EMPTY 8
#define LOST_DSR 16
#define PARALLEL_OFFHOOK_DISCONNECT 32

/* RX / TX Speed buffer sizes */
#define RX_BUFF_SZ 2048
#define TX_BUFF_SZ 2048

/* Flow Control */
#define RX_LOW_WATER_MARK (RX_BUFF_SZ / 4)
#define RX_HIGH_WATER_MARK (RX_BUFF_SZ - RX_LOW_WATER_MARK)
#define TX_LOW_WATER_MARK (TX_BUFF_SZ / 4)
#define TX_HIGH_WATER_MARK (TX_BUFF_SZ - TX_LOW_WATER_MARK)
#define BLOCK_REMOTE_WRITES   1 /* mark flow control action needed */
#define REMOTE_WRITES_BLOCKED 2 /* sent XOFF or dropped RTS */
#define UNBLOCK_REMOTE_WRITES 4 /* send XON or assert RTS */
#define LOCAL_WRITES_BLOCKED 8  /* received XOFF or detected CTS deassertion */
#define XON 0x11  /* (DC1) ctrl-q */
#define XOFF 0x13 /* (DC3) ctrl-s */  


typedef struct {
     CMODULE_T
     void *port_base;    /* I/O address of register file            */
     PFNISR  pfnChain;   /* address of next func to chain to on unhandled ISR */
     int serial_ndx;     /* which serial port are we using          */
     unsigned char int_mode; /* current interrupt mask              */
     unsigned char flow_flags; /* mask of flow control state        */
     unsigned char XOnCharacter;   /* XON/XOFF flow control         */
     unsigned char XOffCharacter;  /* XON/XOFF flow control         */

     /* Circular Buffer Management */

     int rx_space_avail; /* how much space is currently unused      */
     int rx_read_ndx;    /* next member of rcv_buf[] to read        */
     int rx_write_ndx;   /* next member of rcv_buf[] to write (ISR) */
     int last_rx_ndx;    /* previous read index                     */
     int last_rx_byte_cnt; /* how many bytes we delivered last read */
     unsigned char rx_buf[RX_BUFF_SZ] ;  /* circular RECV buffer   */

     int tx_space_avail; /* how much space is currently unused      */
     int tx_read_ndx;    /* next member of tx_buf[] to read (ISR)   */
     int tx_write_ndx;   /* next member of tx_buf[] to write        */
     unsigned char tx_buf[TX_BUFF_SZ] ;   /* circular XMIT buffer   */
	 int disable_write_notifications; /* set to TRUE if TX buffer filling up */

     bool break_rcvd;    /* received at least 1 break on port       */
     int  FramingErrs;   /* Framing Errors this connection          */
     int  OverrunErrs;   /* Overrun Errors this connection          */
     int  ParityErrs;    /* Parity Errors this connection           */
     int  rx_bytes_dropped; /* bytes dropped due to overflow        */
#ifdef DEBUG
     unsigned int rx_isrs;
     unsigned int tx_isrs;
     unsigned int bytes_received;
#endif
#ifdef CMOD_TEST_BUCKET
	 opentv_pid_t pid ;
#endif

} CMDM_T ;



/* The <calltype> is one of "online", "voice", or "verifier".  */
typedef enum { ONLINE, VOICE, VERIFIER } call_t;
/* <calltype>.<phone number>.[<min rate>[.<max rate>]]\0  */
#define MAX_PHONE_NUM_LEN 64 
typedef struct {
  call_t    call_type ; // ONLINE, VOICE, VERIFIER
  char      number[MAX_PHONE_NUM_LEN];
  int       min_rate;   
  int       max_rate;   
  char      modulation_string[SCRATCH_LEN]; /* +MS */
} call_data ;


/* Used by cstack_call() to run callbacks from ISR */
typedef struct {
    /* "To use cstack_call(), a C structure must be allocated    */
    /* which contains as its first field a pointer to a function */
    /* of type cstack_fct_t."                                    */
    cstack_fct_t dummy;
    int type;
    int value;
    CMDM_T *me;
} cmod_msg_t; 


#define NUM_OF_MODULATIONS 5
#define MAX_SPEEDS_PER_MODULATION 21
#define EMPTY -99

/****************************************************************************
 * Modifications:
 * $Log: 
 *  17   mpeg      1.16        2/13/03 12:31:34 PM    Matt Korte      SCR(s) 
 *        5479 :
 *        Removed old header reference
 *        
 *  16   mpeg      1.15        11/26/02 5:02:22 PM    Dave Wilson     SCR(s) 
 *        4902 :
 *        Removed definition of adapter EEPROM I2C address. This is in the 
 *        board config
 *        file now.
 *        
 *  15   mpeg      1.14        1/11/01 4:18:18 PM     Angela          DCS#935 -
 *         moved BASIC_PREFIX_1&2 to the vendor header files
 *        
 *  14   mpeg      1.13        1/8/01 2:19:12 AM      Dave Wilson     DCS917: 
 *        Changed aprallel offhook detection threshold to 1.2V in modem
 *        initialisation string for Vendor A.
 *        
 *  13   mpeg      1.12        1/5/01 11:54:58 AM     Angela          Moved 
 *        LINE_XXX_XXX macros, so that they work for all the modems
 *        
 *  12   mpeg      1.11        12/21/00 3:57:54 PM    Dave Moore      Add 2 
 *        more seconds of delay on dial prefix.
 *        
 *  11   mpeg      1.10        12/19/00 1:22:52 PM    Dave Moore      More 
 *        Genesys changes.
 *        
 *  10   mpeg      1.9         12/18/00 2:09:46 PM    Dave Moore      New line 
 *        sensing code.
 *        
 *  9    mpeg      1.8         12/14/00 9:39:26 PM    QA - Roger Taylor Joe K. 
 *        Moved the if statement for Vendor_A from before
 *        #if CUSTOMER == VENDOR_A
 *        
 *  8    mpeg      1.7         12/14/00 8:54:16 PM    Dave Moore      Genesys 
 *        testing.
 *        
 *  7    mpeg      1.6         12/1/00 4:12:58 PM     Dave Moore      Changes 
 *        for BSkyB testing.
 *        
 *  6    mpeg      1.5         11/29/00 3:00:54 PM    Dave Moore      Added 
 *        extra country codes, changed PREFIX for
 *        vendor_a.
 *        
 *  5    mpeg      1.4         10/9/00 5:21:06 PM     Joe Kroesche    fixed 
 *        problem with smartmdp vs smartscm init strings
 *        
 *  4    mpeg      1.3         10/8/00 9:07:56 PM     Joe Kroesche    
 *        modification to init strings to support smartmdp
 *        
 *  3    mpeg      1.2         9/21/00 3:49:34 PM     Dave Moore      added 
 *        bytes_received field.
 *        
 *  2    mpeg      1.1         8/30/00 10:41:06 AM    Dave Moore      
 *        Improvements to write handling.
 *        
 *  1    mpeg      1.0         8/24/00 7:18:06 PM     Dave Moore      
 * $
 * 
 *    Rev 1.16   13 Feb 2003 12:31:34   kortemw
 * SCR(s) 5479 :
 * Removed old header reference
 * 
 *    Rev 1.15   26 Nov 2002 17:02:22   dawilson
 * SCR(s) 4902 :
 * Removed definition of adapter EEPROM I2C address. This is in the board config
 * file now.
 * 
 *    Rev 1.14   11 Jan 2001 16:18:18   angela
 * moved BASIC_PREFIX_1&2 into vendor header files
 * 
 *    Rev 1.13   08 Jan 2001 02:19:12   dawilson
 * DCS917: Changed aprallel offhook detection threshold to 1.2V in modem
 * initialisation string for Vendor A.
 * 
 *    Rev 1.12   05 Jan 2001 11:54:58   angela
 * Moved LINE_XXX_XXX macros, so that they work for all the modems
 * 
 *    Rev 1.11   Dec 21 2000 15:57:54   mooreda
 * Add 2 more seconds of delay on dial prefix.
 * 
 *    Rev 1.10   Dec 19 2000 13:22:52   mooreda
 * More Genesys changes.
 * 
 *    Rev 1.9   Dec 18 2000 14:09:46   mooreda
 * New line sensing code.
 * 
 *    Rev 1.7   Dec 14 2000 20:54:16   mooreda
 * Genesys testing.
 * 
 *    Rev 1.6   Dec 01 2000 16:12:58   mooreda
 * Changes for BSkyB testing.
 * 
 *    Rev 1.5   Nov 29 2000 15:00:54   mooreda
 * Added extra country codes, changed PREFIX for
 * vendor_a.
 * 
 *    Rev 1.4   Oct 09 2000 16:21:06   kroescjl
 * fixed problem with smartmdp vs smartscm init strings
 * 
 *    Rev 1.3   Oct 08 2000 20:07:56   kroescjl
 * modification to init strings to support smartmdp
 * 
 *    Rev 1.2   Sep 21 2000 14:49:34   mooreda
 * added bytes_received field.
 * 
 *    Rev 1.1   Aug 30 2000 09:41:06   mooreda
 * Improvements to write handling.
 * 
 *    Rev 1.0   Aug 24 2000 18:18:06   mooreda
 * Initial revision.
 * 
 *    Rev 1.1   Aug 24 2000 16:29:02   mooreda
 * Updates after initial testing runs.
 *
 ****************************************************************************/

