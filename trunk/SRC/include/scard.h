/****************************************************************************/
/*                            Conexant Systems                              */
/****************************************************************************/
/*                                                                          */
/* Filename:       scard.h                                                  */
/*                                                                          */
/* Description:    Smart Card Header file.                                  */
/*                                                                          */
/* Author:         Senthil Veluswamy                                        */
/*                                                                          */
/* Date:           09/15/98                                                 */
/*                                                                          */
/* Copyright Conexant Systems, 1999                                         */
/* All Rights Reserved.                                                     */
/*                                                                          */
/****************************************************************************/

#ifndef _SCARD_H_
#define _SCARD_H_

/*****************/
/* Include Files */
/*****************/

/***********/
/* Aliases */
/***********/

// Smart Card reference clock.
#if (EMULATION_LEVEL == PID7T)
#define SC_REF_CLK 4000000
#else
#define SC_REF_CLK 54000000
#endif

// Neches Smart Card Retcodes.
#define SC0                     (MOD_SMC + 0x000)   
#define SC1                     (MOD_SMC + 0x800)
#define SC_TXIRQ_ERR            (MOD_SMC + 0x10)
#define SC_RXIRQ_ERR            (MOD_SMC + 0x20)
#define SC_GPIRQ_ERR            (MOD_SMC + 0x30)
#define SC_ITASK_ERR            (MOD_SMC + 0x40)
#define SC_ATR_ERR              (MOD_SMC + 0x50)
#define SC_READ_ERR             (MOD_SMC + 0x60)
#define SC_RESET_ERR            (MOD_SMC + 0x70)
#define SC_WRITE_ERR            (MOD_SMC + 0x80)
#define SC_ACTIV_ERR            (MOD_SMC + 0xA0)
#define SC_WARM_ERR             (MOD_SMC + 0xB0)
#define SC_FTABLE_REG_ERR       (MOD_SMC + 0x01)
#define SC_SEMA_CREATE_ERR      (MOD_SMC + 0x02)
#define SC_TASK_CREATE_ERR      (MOD_SMC + 0x03)
#define SC_CARD_REMOVED         (MOD_SMC + 0x04)
#define SC_CARD_MUTE            (MOD_SMC + 0x05)
#define SC_WRONG_STATE          (MOD_SMC + 0x06)
#define SC_ACK_ERROR            (MOD_SMC + 0x07)
#define SC_ISR_REG_ERR          (MOD_SMC + 0x08)
#define SC_ISR_ENABLE_ERR       (MOD_SMC + 0x09)
#define SC_ISR_ERR              (MOD_SMC + 0x0A)

// other aliases.
#define RFU -1

/*************************/
/* Function declarations */
/*************************/
bool scard_init(void);
void scd_interface_task(void*);
int scd_read(int, const char*, char*);
int scd_write(int, const char*, const char*);
int scd_atr(int, char*);
void scd_reset(int);

#endif // _SCARD_H_ 