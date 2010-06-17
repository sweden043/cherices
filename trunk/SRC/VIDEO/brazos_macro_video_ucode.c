/****************************************************************************/
/*                   CONEXANT PROPRIETARY AND CONFIDENTIAL                  */
/*                       SOFTWARE FILE/MODULE HEADER                        */
/*                   Conexant Systems Inc. (c) 1998-2003                    */
/*                               Austin, TX                                 */
/*                            All Rights Reserved                           */
/****************************************************************************/
/*
 * Filename:       brazos_macro_video_ucode.c
 *
 *
 * Description:    Brazos video microcode with macroblock-based error recovery
 *
 *
 * Author:         Truman Ng
 *
 ****************************************************************************/
/* $Id: brazos_macro_video_ucode.c,v 1.11, 2004-02-17 21:23:25Z, Dave Wilson$
 ****************************************************************************/

/* Current version - Brazos dual mode DVB/DTV microcode version 0.15 */

/* Microcode source revision log:                                                                             */
/*                                                                                                            */
/*  0.00 - Initial version. Copied from Wabash 0.07 version on 11/01/02                                       */
/*  0.01 - Copied from Wabash 0.09: added ifdefs for trickmode, allows room for macroblock error concealment  */
/*  0.02 - dss mode incorporated, new SYNC code                                                               */
/*  0.03 - write to GEN_REG1 register in host, indicate I or P picture decoded                                */
/*  0.04 - clear SYNC bit in ERROR, make sure in sync 3 frames before setting A/V sync bit                    */
/*  0.05 - change still image decoding to continuously decode pictures until new command posted               */
/*  0.06 - search picture header in playmain                                                                  */
/*  0.07 - fix for decoder freeze with attenuated noisy data.                                                 */
/*  0.09 - fix DSS video skip problem due to STC carrying over to bit 33.                                     */
/*  0.10 - fix I, P buffer pointer initialisation                                                             */
/*  0.11 - HD decode tolerance, channel change time improvements                                              */
/*  0.12 - Changes to fix attenuated data cases that cause Vcore to hang                                      */
/*  0.13 - fix the decoder hang when encountering attenuated/noisy input signals.                             */
/*  0.14 - fix for declaring early sync 3 frames early for use with fast channel change.                      */
/*  0x15 - fix for problem getting decoder into idle state after decoding a still                             */

const unsigned char brazos_macro_video_ucode[] = {
0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x10,
0x02,0x00,0x00,0x14,
0x5b,0x10,0x00,0x00,
0x7e,0x10,0x00,0x00,
0xdf,0x10,0x00,0x00,
0x00,0x10,0x00,0x00,
0x00,0x10,0x00,0x00,
0x67,0x10,0x00,0x00,
0x00,0x10,0x00,0x00,
0xb3,0x11,0x00,0x00,
0xb6,0x11,0x00,0x00,
0x4f,0x12,0x00,0x00,
0x59,0x12,0x00,0x00,
0x63,0x12,0x00,0x00,
0x00,0x10,0x00,0x00,
0x00,0x10,0x00,0x00,
0x01,0x10,0x00,0x00,
0x00,0x10,0x00,0x00,
0x00,0x10,0x00,0x00,
0x69,0x12,0x00,0x00,
0x71,0x12,0x00,0x00,
0xa4,0x12,0x00,0x00,
0xa5,0x12,0x00,0x00,
0xb0,0x12,0x00,0x00,
0xbe,0x12,0x00,0x00,
0xc5,0x12,0x00,0x00,
0xca,0x12,0x00,0x00,
0xcb,0x12,0x00,0x00,
0xce,0x12,0x00,0x00,
0xd1,0x12,0x00,0x00,
0x0f,0x13,0x00,0x00,
0x10,0x13,0x00,0x00,
0x22,0x13,0x00,0x00,
0x2d,0x13,0x00,0x00,
0x2f,0x13,0x00,0x00,
0x4a,0x13,0x00,0x00,
0x4f,0x13,0x00,0x00,
0x50,0x13,0x00,0x00,
0x51,0x13,0x00,0x00,
0x56,0x13,0x00,0x00,
0x5d,0x13,0x00,0x00,
0x60,0x13,0x00,0x00,
0x65,0x13,0x00,0x00,
0x6a,0x13,0x00,0x00,
0x71,0x13,0x00,0x00,
0x74,0x13,0x00,0x00,
0x79,0x13,0x00,0x00,
0x7d,0x13,0x00,0x00,
0x83,0x13,0x00,0x00,
0x95,0x13,0x00,0x00,
0xb1,0x13,0x00,0x00,
0xbd,0x13,0x00,0x00,
0xc7,0x13,0x00,0x00,
0xd2,0x13,0x00,0x00,
0xd9,0x13,0x00,0x00,
0xeb,0x13,0x00,0x00,
0xdc,0x13,0x00,0x00,
0xdd,0x13,0x00,0x00,
0xde,0x13,0x00,0x00,
0xe0,0x13,0x00,0x00,
0xe3,0x13,0x00,0x00,
0x00,0x10,0x00,0x00,
0x01,0x10,0x00,0x00,
0x01,0x10,0x00,0x00,
0x42,0x10,0x00,0x00,
0x00,0x80,0x17,0x20,
0x4a,0xd8,0x02,0x00,
0x00,0x80,0x17,0x20,
0x4d,0xe0,0x02,0x00,
0x00,0x80,0x17,0x20,
0x51,0xe8,0x02,0x00,
0x00,0x80,0x17,0x20,
0x50,0x10,0x00,0x00,
0x01,0x80,0x17,0x20,
0x00,0x00,0x00,0x20,
0x55,0x10,0x00,0x00,
0x01,0x80,0x17,0x20,
0x00,0x00,0x00,0x20,
0x54,0x10,0x00,0x00,
0x00,0xf0,0xc1,0x18,
0x01,0x80,0x17,0x20,
0x00,0x00,0x00,0x20,
0x00,0xf0,0xc1,0x18,
0x00,0xf0,0xc1,0x18,
0x00,0xf0,0xc1,0x18,
0x00,0xf0,0xc1,0x18,
0x02,0xf8,0x06,0x00,
0x51,0x0c,0x00,0x00,
0x02,0x00,0x00,0x14,
0x51,0x14,0x00,0x00,
0x00,0x14,0x00,0x39,
0x66,0x58,0x00,0x00,
0x01,0xfc,0x16,0x20,
0x1e,0x00,0xb4,0x21,
0xc0,0x03,0xa4,0x21,
0x01,0x00,0x34,0x24,
0x00,0x00,0x00,0x20,
0x60,0xa8,0x01,0x00,
0x01,0xf8,0x16,0x20,
0x00,0xf8,0x16,0x20,
0x00,0xfc,0x16,0x20,
0x01,0x10,0x00,0x00,
0x79,0x78,0x02,0x00,
0x00,0xcc,0x16,0x20,
0x01,0xd0,0x16,0x20,
0xe2,0x02,0x14,0x24,
0x00,0x00,0x00,0x20,
0x71,0xb8,0x05,0x00,
0x25,0x02,0x00,0x08,
0x00,0x7c,0x02,0x20,
0x6f,0x60,0x00,0x00,
0x73,0x10,0x00,0x00,
0x71,0xf8,0x01,0x00,
0xd7,0x45,0xa2,0x20,
0x77,0xf0,0x01,0x00,
0x80,0xfc,0xa1,0x3a,
0x00,0x7c,0x02,0x20,
0x76,0x60,0x00,0x00,
0x77,0xf8,0x01,0x00,
0x80,0x46,0xb6,0x20,
0x7c,0x30,0x02,0x00,
0x7a,0xf8,0x01,0x00,
0x86,0x47,0xb6,0x20,
0x01,0x10,0x00,0x00,
0x80,0xfc,0xa1,0x3a,
0x00,0x50,0x02,0x20,
0x00,0xc0,0x16,0x20,
0x83,0xa0,0x06,0x00,
0x00,0x00,0x04,0x00,
0x80,0x10,0x00,0x00,
0x00,0x1c,0x00,0x39,
0x00,0xdc,0x16,0x20,
0x00,0xd8,0x16,0x20,
0x00,0xd4,0x16,0x20,
0xaf,0x00,0x00,0x08,
0xd0,0x00,0x00,0x08,
0x04,0x70,0x16,0x20,
0x02,0x00,0x00,0x08,
0x38,0x00,0xa0,0x24,
0x00,0x00,0x00,0x20,
0x91,0xa0,0x01,0x00,
0x00,0xb8,0x16,0x20,
0x90,0x01,0x00,0x08,
0x88,0x10,0x00,0x00,
0x37,0x04,0x00,0x08,
0xc0,0xfc,0xa1,0x3a,
0x00,0xe0,0xa1,0x3a,
0x97,0xa8,0x00,0x00,
0x03,0x02,0x00,0x08,
0x94,0x10,0x00,0x00,
0x00,0xb8,0x16,0x20,
0x90,0x01,0x00,0x08,
0x11,0x02,0x00,0x26,
0x00,0x00,0x00,0x20,
0xa0,0x9c,0x09,0x21,
0x80,0x98,0x0d,0x22,
0x00,0x7c,0x02,0x20,
0xa1,0x10,0x00,0x00,
0x00,0xb8,0x16,0x20,
0x90,0x01,0x00,0x08,
0xd0,0x00,0x00,0x08,
0x01,0x70,0x16,0x20,
0x34,0x04,0x00,0x08,
0xa4,0x60,0x00,0x00,
0x9f,0x30,0x01,0x00,
0x29,0x02,0x00,0x08,
0xff,0x01,0x00,0x08,
0x00,0x7c,0x02,0x20,
0xaa,0x10,0x00,0x00,
0x01,0x18,0x01,0x00,
0x00,0xb8,0x16,0x20,
0x90,0x01,0x00,0x08,
0x10,0x02,0x00,0x08,
0xaa,0x10,0x00,0x00,
0x00,0x8c,0x16,0x20,
0x01,0xb8,0x16,0x20,
0x00,0x94,0x16,0x20,
0x01,0x84,0x16,0x20,
0x00,0xc0,0x16,0x20,
0x00,0xd0,0x16,0x20,
0x00,0xa4,0x16,0x20,
0x00,0x98,0x16,0x20,
0x00,0xf4,0x16,0x20,
0x00,0xcc,0x16,0x20,
0x00,0xa0,0x16,0x20,
0x00,0xd0,0xa1,0x3a,
0x00,0xc0,0xa1,0x3a,
0x00,0xc4,0xa1,0x3a,
0x00,0xe8,0xa0,0x3a,
0x00,0xec,0xa0,0x3a,
0x00,0xf0,0xa0,0x3a,
0x00,0xf4,0xa0,0x3a,
0x00,0x68,0xa1,0x3a,
0x00,0x5c,0xa1,0x3a,
0x00,0x60,0xa1,0x3a,
0x00,0x04,0xa1,0x3a,
0x00,0x08,0xa1,0x3a,
0x00,0xe8,0x16,0x20,
0x00,0xe0,0xa1,0x3a,
0x00,0xe4,0xa1,0x3a,
0x00,0xe8,0xa1,0x3a,
0x60,0xec,0xa1,0x3a,
0x00,0xf0,0xa1,0x3a,
0x00,0x45,0xb6,0x20,
0xcd,0xf8,0x01,0x00,
0x37,0x45,0xbe,0x20,
0x00,0x00,0x00,0x0c,
0x17,0x00,0xb4,0x21,
0x18,0x00,0x34,0x21,
0xde,0x20,0x04,0x00,
0x00,0x00,0x00,0x20,
0x00,0x00,0x00,0x20,
0x00,0x00,0x00,0x20,
0x00,0x00,0x00,0x20,
0x00,0x00,0x00,0x20,
0x01,0x00,0x34,0x24,
0x00,0x00,0x00,0x20,
0xd2,0xa8,0x01,0x00,
0x87,0x47,0xb6,0x20,
0x00,0x00,0x00,0x20,
0xdc,0x10,0x00,0x00,
0x00,0x00,0x00,0x0c,
0x00,0xc0,0x16,0x20,
0x00,0x20,0x02,0x20,
0xe3,0x48,0x00,0x00,
0xe1,0x10,0x00,0x00,
0xe3,0x48,0x00,0x00,
0x79,0x51,0x00,0x00,
0x1a,0x01,0x00,0x08,
0xd3,0x01,0x00,0x08,
0xf8,0xd0,0x06,0x00,
0x17,0xdc,0xb4,0x21,
0x40,0xeb,0x84,0x21,
0x38,0xe7,0x94,0x22,
0x18,0xe0,0xb0,0x22,
0xba,0x03,0x90,0x24,
0x99,0x03,0x90,0x25,
0x78,0x03,0x90,0x25,
0x45,0xb9,0x01,0x00,
0x18,0xe4,0x15,0x2d,
0x20,0xe4,0xa5,0x21,
0x17,0x00,0x10,0x2d,
0x40,0xeb,0x84,0x24,
0x39,0xe7,0x80,0x25,
0x00,0xe3,0x94,0x25,
0x0f,0xb9,0x01,0x00,
0x13,0x11,0x00,0x00,
0x0a,0xdc,0xb4,0x21,
0xfb,0x30,0x00,0x00,
0x0c,0xdc,0xb4,0x21,
0x40,0xeb,0x84,0x21,
0x19,0xe4,0xb0,0x22,
0x18,0xe0,0xb0,0x22,
0xba,0xf3,0x91,0x24,
0x00,0x00,0x00,0x20,
0x99,0xef,0x91,0x25,
0x00,0x00,0x00,0x20,
0x78,0xeb,0x91,0x25,
0x00,0x00,0x00,0x20,
0x45,0xb9,0x01,0x00,
0x17,0x00,0x10,0x2d,
0x40,0xeb,0x84,0x24,
0x20,0xe7,0x94,0x25,
0x00,0xe3,0x94,0x25,
0x0f,0xb9,0x01,0x00,
0x79,0xd1,0x01,0x00,
0x01,0xcc,0x16,0x20,
0x0c,0xf9,0x01,0x00,
0x81,0x46,0xb6,0x20,
0x79,0x11,0x00,0x00,
0x5d,0x03,0x90,0x24,
0x3c,0x03,0x90,0x25,
0x1b,0x03,0x90,0x25,
0x67,0xb9,0x01,0x00,
0x79,0xd1,0x01,0x00,
0x01,0xcc,0x16,0x20,
0x00,0x00,0x00,0x20,
0x79,0x59,0x04,0x00,
0x17,0xf9,0x01,0x00,
0x81,0x46,0xb6,0x20,
0x79,0x11,0x00,0x00,
0x1a,0x49,0x02,0x00,
0x00,0xe8,0xc0,0x18,
0x00,0x00,0x00,0x20,
0x00,0x00,0x00,0x20,
0x00,0x00,0x00,0x20,
0x00,0x00,0x00,0x20,
0x01,0xe8,0x16,0x20,
0x00,0xe8,0x16,0x20,
0x22,0x49,0x02,0x00,
0x00,0xe4,0xc0,0x18,
0x01,0xe8,0x16,0x20,
0x00,0xe8,0x16,0x20,
0x26,0x49,0x02,0x00,
0x00,0xe0,0xc0,0x18,
0x01,0xe8,0x16,0x20,
0x00,0xe8,0x16,0x20,
0x2a,0x49,0x02,0x00,
0x00,0xf4,0xc0,0x18,
0x01,0xe8,0x16,0x20,
0x00,0xe8,0x16,0x20,
0x2e,0x49,0x02,0x00,
0x00,0xf0,0xc0,0x18,
0x01,0xe8,0x16,0x20,
0x00,0xe8,0x16,0x20,
0x32,0x49,0x02,0x00,
0x00,0xec,0xc0,0x18,
0x01,0xe8,0x16,0x20,
0x00,0xe8,0x16,0x20,
0x00,0x00,0x00,0x20,
0x00,0x00,0x00,0x20,
0x00,0x00,0x00,0x20,
0x00,0x00,0x00,0x20,
0x00,0x03,0x80,0x2c,
0x00,0x00,0x20,0x2c,
0x00,0x00,0x04,0x30,
0x00,0x00,0x04,0x30,
0x00,0xe3,0x84,0x24,
0x60,0x03,0x80,0x2c,
0x00,0x00,0x20,0x2c,
0x00,0x00,0x04,0x30,
0x00,0x00,0x04,0x30,
0x60,0xef,0x84,0x24,
0x00,0x00,0x00,0x0c,
0x38,0x60,0xbd,0x21,
0x01,0xe0,0x16,0x20,
0x4a,0x31,0x00,0x00,
0xcc,0x00,0xb4,0x26,
0x4c,0x11,0x00,0x00,
0xca,0x00,0xb4,0x26,
0x00,0x00,0x00,0x20,
0x80,0x03,0x08,0x24,
0x60,0x03,0x0c,0x25,
0x40,0x03,0x14,0x25,
0x57,0xb9,0x01,0x00,
0x5a,0x59,0x04,0x00,
0x40,0x08,0xa1,0x3a,
0x00,0x84,0x16,0x20,
0x16,0x02,0x00,0x08,
0x54,0xf9,0x01,0x00,
0x81,0x46,0xb6,0x20,
0x5a,0x11,0x00,0x00,
0x00,0xcc,0x16,0x20,
0x58,0xf9,0x01,0x00,
0xd7,0x45,0xa2,0x20,
0x5a,0xf9,0x01,0x00,
0x18,0x45,0xbe,0x20,
0x42,0x00,0xf4,0x24,
0x00,0x00,0x00,0x20,
0x62,0xa1,0x01,0x00,
0x63,0x59,0x00,0x00,
0x80,0xfc,0xa1,0x3a,
0x63,0x11,0x00,0x00,
0x20,0x08,0xa1,0x3a,
0xaa,0x01,0x00,0x08,
0x00,0x7c,0x02,0x20,
0x65,0x61,0x00,0x00,
0x01,0x10,0x00,0x00,
0x6c,0xf1,0x00,0x00,
0x00,0xcc,0x16,0x20,
0x69,0xf9,0x01,0x00,
0xd7,0x45,0xa2,0x20,
0x71,0x21,0x01,0x00,
0x79,0x29,0x01,0x00,
0x23,0x00,0xf4,0x24,
0x71,0xb9,0x05,0x00,
0x21,0x04,0xbd,0x21,
0x79,0x11,0x00,0x00,
0x00,0x04,0x15,0x20,
0x37,0x5c,0xbd,0x21,
0x00,0xe0,0x16,0x20,
0x37,0x45,0xbe,0x20,
0x9b,0x01,0x00,0x08,
0xaa,0x01,0x00,0x08,
0x10,0x02,0x00,0x08,
0x01,0x10,0x00,0x00,
0x00,0xe0,0x16,0x20,
0x8b,0x51,0x00,0x00,
0x9b,0x01,0x00,0x08,
0x8a,0xc1,0x06,0x00,
0x84,0xd1,0x06,0x00,
0x18,0xe4,0xb5,0x21,
0x17,0x00,0xb4,0x21,
0xa0,0xf7,0x84,0x21,
0x99,0xf3,0x80,0x22,
0x60,0xef,0x94,0x22,
0x8a,0x11,0x00,0x00,
0x0a,0x00,0xb4,0x21,
0x87,0x31,0x00,0x00,
0x0c,0x00,0xb4,0x21,
0xa0,0xf7,0x84,0x21,
0x80,0xf3,0x94,0x22,
0x60,0xef,0x94,0x22,
0xaa,0x01,0x00,0x08,
0xeb,0x01,0x00,0x08,
0x01,0x58,0x01,0x00,
0x00,0x7c,0x02,0x20,
0x8e,0x61,0x00,0x00,
0x01,0x10,0x00,0x00,
0x00,0x20,0x02,0x20,
0x93,0x49,0x00,0x00,
0x91,0x11,0x00,0x00,
0x93,0x49,0x00,0x00,
0x9a,0x51,0x00,0x00,
0x1a,0x01,0x00,0x08,
0x98,0xc9,0x01,0x00,
0x9b,0x01,0x00,0x08,
0xaa,0x01,0x00,0x08,
0x01,0xb8,0x16,0x20,
0x00,0x00,0x00,0x0c,
0xa3,0xd1,0x06,0x00,
0x1b,0xec,0xb5,0x21,
0x1a,0x00,0xb4,0x21,
0xa0,0xf7,0x84,0x21,
0x9b,0xf3,0x80,0x22,
0x60,0xef,0x94,0x22,
0x00,0x00,0x00,0x20,
0xa9,0x11,0x00,0x00,
0x09,0x00,0xb4,0x21,
0xa6,0x31,0x00,0x00,
0x0b,0x00,0xb4,0x21,
0xa0,0xf7,0x84,0x21,
0x80,0xf3,0x94,0x22,
0x60,0xef,0x94,0x22,
0x00,0x00,0x00,0x0c,
0xaa,0xf9,0x01,0x00,
0x5d,0x45,0xb2,0x20,
0x00,0x00,0x00,0x20,
0xad,0xf9,0x01,0x00,
0x7c,0x45,0xb2,0x20,
0x00,0x00,0x00,0x20,
0xb0,0xf9,0x01,0x00,
0x9b,0x45,0xb2,0x20,
0x00,0x00,0x00,0x0c,
0x01,0xd4,0x16,0x20,
0x00,0x1c,0x00,0x39,
0x00,0x10,0x00,0x00,
0x30,0x04,0x00,0x08,
0xaf,0x00,0x00,0x08,
0x01,0xdc,0x16,0x20,
0x01,0x84,0x16,0x20,
0xd0,0x00,0x00,0x08,
0x02,0x70,0x16,0x20,
0x34,0x04,0x00,0x08,
0xc0,0xfc,0xa1,0x3a,
0x11,0x02,0x00,0x26,
0x00,0x00,0x00,0x20,
0xa0,0x9c,0x09,0x21,
0x80,0x98,0x0d,0x22,
0x00,0x7c,0x02,0x20,
0x00,0x84,0x16,0x20,
0x01,0x70,0x16,0x20,
0x34,0x04,0x00,0x08,
0xc4,0x31,0x01,0x00,
0x00,0x7c,0x02,0x20,
0xc8,0x61,0x00,0x00,
0x29,0x02,0x00,0x08,
0xca,0x69,0x00,0x00,
0x00,0xfc,0xa1,0x3a,
0x42,0x02,0x00,0x08,
0x00,0x7c,0x02,0x20,
0xce,0x61,0x00,0x00,
0xcf,0x69,0x00,0x00,
0x01,0x00,0x04,0x00,
0xb6,0xd9,0x04,0x00,
0xc4,0x11,0x00,0x00,
0x00,0x03,0x80,0x2c,
0xe0,0x01,0xa4,0x24,
0x00,0x00,0x00,0x20,
0xdf,0xa9,0x01,0x00,
0x60,0x03,0x80,0x2c,
0x00,0x00,0xa4,0x24,
0x00,0x00,0x00,0x20,
0xea,0xa9,0x01,0x00,
0x6d,0xef,0x94,0x21,
0x00,0x00,0x20,0x29,
0x60,0xef,0x84,0x21,
0xea,0x11,0x00,0x00,
0x60,0x03,0x80,0x2c,
0xe0,0x01,0xa4,0x24,
0x00,0x00,0x00,0x20,
0xea,0xa9,0x01,0x00,
0x00,0x03,0x80,0x2c,
0x00,0x00,0xa4,0x24,
0x00,0x00,0x00,0x20,
0xea,0xa9,0x01,0x00,
0x0d,0x00,0xb4,0x21,
0x00,0x00,0x20,0x29,
0x00,0xe3,0x84,0x21,
0x00,0x00,0x00,0x0c,
0x40,0x00,0xf4,0x24,
0x00,0x00,0x00,0x20,
0xf0,0xa1,0x01,0x00,
0x00,0x08,0xa1,0x3a,
0xf6,0x11,0x00,0x00,
0x00,0x84,0x16,0x20,
0x16,0x02,0x00,0x08,
0x00,0x7c,0x02,0x20,
0xf6,0x11,0x00,0x00,
0x00,0xb8,0x16,0x20,
0x90,0x01,0x00,0x08,
0xd0,0x00,0x00,0x08,
0x01,0x70,0x16,0x20,
0x34,0x04,0x00,0x08,
0xf9,0x61,0x00,0x00,
0xf4,0x31,0x01,0x00,
0xfb,0x69,0x00,0x00,
0xfe,0x31,0x00,0x00,
0xc0,0xfc,0xa1,0x3a,
0x29,0x02,0x00,0x08,
0x02,0x92,0x02,0x00,
0x00,0x7c,0x02,0x20,
0x01,0x62,0x00,0x00,
0x00,0x00,0x00,0x0c,
0x00,0x7c,0x02,0x20,
0x07,0x12,0x00,0x00,
0x00,0xb8,0x16,0x20,
0x90,0x01,0x00,0x08,
0xd0,0x00,0x00,0x08,
0x00,0x70,0x16,0x20,
0x34,0x04,0x00,0x08,
0x0a,0x62,0x00,0x00,
0x05,0x32,0x01,0x00,
0x0c,0x6a,0x00,0x00,
0x00,0x00,0x00,0x0c,
0x00,0xb8,0x16,0x20,
0x90,0x01,0x00,0x08,
0xd0,0x00,0x00,0x08,
0x00,0x70,0x16,0x20,
0x80,0xfc,0xa1,0x3a,
0x34,0x04,0x00,0x08,
0x0e,0x32,0x01,0x00,
0x00,0x00,0x00,0x0c,
0x1f,0x1a,0x01,0x00,
0x1c,0x5a,0x02,0x00,
0x1c,0x12,0x01,0x00,
0x19,0x1a,0x00,0x00,
0x40,0xfc,0xa1,0x3a,
0x27,0x12,0x00,0x00,
0x1c,0x1a,0x00,0x00,
0x20,0xfc,0xa1,0x3a,
0x27,0x12,0x00,0x00,
0x25,0x42,0x01,0x00,
0x20,0x1a,0x00,0x00,
0xe0,0xfc,0xa1,0x3a,
0x00,0xa4,0x16,0x20,
0x00,0x00,0x00,0x20,
0x00,0x00,0x00,0x20,
0x25,0x1a,0x00,0x00,
0x00,0xfc,0xa1,0x3a,
0x42,0x02,0x00,0x08,
0x00,0x00,0x00,0x0c,
0x00,0x00,0xb4,0x21,
0x2d,0x4a,0x01,0x00,
0x04,0x00,0xb4,0x21,
0x00,0x00,0x00,0x20,
0x30,0x1a,0x01,0x00,
0x40,0x00,0xa4,0x21,
0x32,0x12,0x00,0x00,
0xc0,0x02,0x04,0x21,
0x00,0x00,0x00,0x20,
0x32,0xfa,0x01,0x00,
0xa0,0x45,0xa6,0x20,
0x00,0x00,0x00,0x20,
0x00,0x00,0x00,0x20,
0x02,0x00,0xb4,0x21,
0x3b,0x22,0x01,0x00,
0x00,0x00,0xb4,0x21,
0x3b,0x2a,0x01,0x00,
0x01,0x00,0xb4,0x21,
0x3b,0xfa,0x01,0x00,
0xe0,0x45,0xa6,0x20,
0x3e,0xd2,0x02,0x00,
0x41,0x22,0x02,0x00,
0x3f,0xfa,0x01,0x00,
0x84,0x47,0xb6,0x20,
0x00,0x00,0x00,0x0c,
0x45,0xd2,0x05,0x00,
0x97,0x00,0xa0,0x21,
0x46,0x12,0x00,0x00,
0x17,0x00,0xa0,0x21,
0x42,0xfa,0x01,0x00,
0xc0,0x45,0xa6,0x20,
0x00,0x00,0x00,0x20,
0x00,0x00,0x00,0x20,
0x00,0x00,0x00,0x20,
0x4e,0x2a,0x02,0x00,
0x4c,0xfa,0x01,0x00,
0x85,0x47,0xb6,0x20,
0x00,0x00,0x00,0x0c,
0x00,0x14,0x00,0x39,
0xc1,0x02,0x14,0x24,
0x00,0x00,0x00,0x20,
0x5d,0xa2,0x01,0x00,
0x53,0x02,0x00,0x00,
0x00,0x84,0xc1,0x18,
0x55,0x02,0x00,0x00,
0x00,0x80,0xc1,0x18,
0x00,0x00,0x00,0x20,
0x00,0x10,0x00,0x00,
0x00,0x14,0x00,0x39,
0xc1,0x02,0x14,0x24,
0x00,0x00,0x00,0x20,
0x53,0xa2,0x01,0x00,
0x5d,0x02,0x00,0x00,
0x00,0x94,0xc1,0x18,
0x5f,0x02,0x00,0x00,
0x00,0x90,0xc1,0x18,
0x00,0x00,0x00,0x20,
0x00,0x10,0x00,0x00,
0x00,0x14,0x00,0x39,
0x64,0x02,0x00,0x00,
0x00,0xa4,0xc1,0x18,
0x66,0x02,0x00,0x00,
0x00,0xa0,0xc1,0x18,
0x00,0x10,0x00,0x00,
0xd0,0x00,0x00,0x08,
0x01,0x9c,0x16,0x20,
0x01,0xa8,0x16,0x20,
0x00,0xa0,0x16,0x20,
0x19,0x00,0xb4,0x21,
0x00,0x48,0x21,0x3a,
0x20,0xe0,0xa1,0x3a,
0x02,0x00,0x00,0x14,
0x00,0xca,0x01,0x3a,
0x00,0xc0,0xc1,0x1c,
0x20,0xce,0x01,0x3a,
0x19,0x02,0x14,0x24,
0x78,0xba,0x05,0x00,
0x10,0xc0,0xb5,0x24,
0x00,0x00,0x00,0x20,
0x00,0x02,0x00,0x29,
0x00,0x00,0x20,0x29,
0x00,0x00,0x20,0x29,
0x00,0x00,0x04,0x2d,
0x00,0x00,0x04,0x2d,
0x00,0x00,0x04,0x2d,
0x00,0xe4,0x21,0x3a,
0x00,0x00,0x00,0x20,
0xf0,0x01,0xa0,0x21,
0x00,0x00,0x20,0x2c,
0x00,0xc0,0x05,0x30,
0x00,0x00,0x00,0x20,
0x59,0x44,0xa2,0x20,
0x00,0xc4,0xc1,0x1c,
0x00,0x00,0x00,0x20,
0xf1,0x01,0xa0,0x21,
0x00,0x00,0x20,0x2c,
0x00,0xc4,0x05,0x30,
0x00,0x00,0x00,0x20,
0x8b,0xfa,0x01,0x00,
0x71,0x44,0xa2,0x20,
0x40,0x6f,0xe1,0x3a,
0x00,0x68,0xc1,0x1c,
0x8f,0xfa,0x01,0x00,
0x9a,0x44,0xbe,0x20,
0x9a,0x02,0x02,0x00,
0x40,0x02,0x00,0x3a,
0x10,0x00,0x20,0x24,
0x13,0x00,0x20,0x21,
0x11,0x00,0x20,0x24,
0x00,0x00,0x00,0x20,
0x9a,0xa2,0x01,0x00,
0x98,0xfa,0x01,0x00,
0x80,0x47,0xb6,0x20,
0xa1,0x0a,0x02,0x00,
0x60,0x03,0xe0,0x3a,
0x1a,0x00,0x3c,0x24,
0x00,0x00,0x00,0x20,
0xa1,0xa2,0x01,0x00,
0x9f,0xfa,0x01,0x00,
0x81,0x47,0xb6,0x20,
0x00,0x02,0x00,0x2c,
0x00,0xc4,0x06,0x20,
0x00,0x00,0x00,0x0c,
0x02,0x00,0x00,0x14,
0x00,0x9c,0x16,0x20,
0xa9,0x1a,0x02,0x00,
0xa7,0xfa,0x01,0x00,
0x83,0x47,0xb6,0x20,
0xad,0xe2,0x01,0x00,
0x01,0x98,0x16,0x20,
0x01,0x00,0x04,0x00,
0x00,0x00,0x00,0x0c,
0xd0,0x00,0x00,0x08,
0x02,0x70,0x16,0x20,
0x02,0x00,0x00,0x14,
0x00,0xe0,0xc0,0x1c,
0x00,0xe4,0xc0,0x1c,
0xb8,0x44,0xb2,0x20,
0x00,0xe8,0xc0,0x1c,
0xb4,0xfa,0x01,0x00,
0xd9,0x44,0xb2,0x20,
0x00,0x00,0x00,0x20,
0xb7,0xfa,0x01,0x00,
0xfa,0x44,0xb2,0x20,
0xbd,0x12,0x02,0x00,
0xba,0xfa,0x01,0x00,
0x82,0x47,0xb6,0x20,
0x00,0xa0,0x16,0x20,
0x00,0x00,0x00,0x0c,
0xd0,0x00,0x00,0x08,
0x10,0x78,0x02,0x20,
0x11,0xb0,0x02,0x20,
0x00,0xa0,0x16,0x20,
0x00,0xd8,0xa0,0x3a,
0x00,0xdc,0xa0,0x3a,
0x02,0x00,0x00,0x14,
0x40,0xe2,0xe0,0x3a,
0x00,0x48,0xc1,0x1c,
0x18,0x00,0x30,0x24,
0x00,0x90,0x02,0x20,
0x02,0x00,0x00,0x14,
0x02,0x00,0x00,0x14,
0x00,0xa0,0xc0,0x1c,
0x00,0xa4,0xc0,0x1c,
0x02,0x00,0x00,0x14,
0x00,0xa8,0xc0,0x1c,
0x00,0xac,0xc0,0x1c,
0x02,0x00,0x00,0x14,
0xfc,0x32,0x01,0x00,
0xfc,0x1a,0x01,0x00,
0xed,0x5a,0x02,0x00,
0x20,0xfa,0x00,0x2c,
0x00,0xf8,0x20,0x29,
0x01,0xf8,0x34,0x24,
0x20,0x02,0x00,0x29,
0x00,0x00,0x20,0x29,
0x00,0x02,0x04,0x26,
0x00,0x00,0x00,0x20,
0x00,0x54,0x41,0x3a,
0x00,0x58,0x61,0x3a,
0xe5,0x3a,0x00,0x00,
0x20,0x02,0x00,0x2c,
0x00,0x02,0x04,0x26,
0x00,0x00,0x00,0x20,
0x00,0xe4,0x40,0x3a,
0x00,0xe0,0x60,0x3a,
0xb9,0x56,0xf1,0x24,
0xd8,0x5a,0xf1,0x25,
0x20,0xe2,0x00,0x29,
0x00,0xe0,0x20,0x29,
0x20,0xe6,0x00,0x29,
0xeb,0x42,0x00,0x00,
0x00,0xeb,0x80,0x29,
0xf0,0x12,0x00,0x00,
0x00,0xeb,0x80,0x3a,
0xf0,0x12,0x00,0x00,
0x20,0xe2,0x00,0x29,
0x20,0xe6,0x00,0x29,
0x00,0xe8,0x20,0x29,
0x18,0x02,0x10,0x26,
0x00,0x00,0x00,0x20,
0x20,0xad,0x09,0x21,
0x00,0xa9,0x0d,0x22,
0x19,0x02,0x10,0x26,
0x00,0x00,0x00,0x20,
0x60,0xb5,0x09,0x21,
0x40,0xb1,0x0d,0x22,
0x1a,0x02,0x10,0x26,
0x00,0x00,0x00,0x20,
0xa0,0xbd,0x09,0x21,
0x80,0xb9,0x0d,0x22,
0x06,0x13,0x01,0x00,
0x10,0x02,0x14,0x26,
0x00,0x00,0x00,0x20,
0x00,0x74,0x41,0x3a,
0x00,0x70,0x61,0x3a,
0x08,0x02,0x14,0x26,
0x00,0x00,0x00,0x20,
0x00,0x7c,0x41,0x3a,
0x00,0x78,0x61,0x3a,
0x0e,0x13,0x00,0x00,
0x08,0x02,0x14,0x26,
0x00,0x00,0x00,0x20,
0x00,0x74,0x41,0x3a,
0x00,0x70,0x61,0x3a,
0x04,0x02,0x14,0x26,
0x00,0x00,0x00,0x20,
0x00,0x7c,0x41,0x3a,
0x00,0x78,0x61,0x3a,
0x02,0x00,0x00,0x14,
0x00,0x00,0x00,0x0c,
0x00,0xa8,0x16,0x20,
0x00,0xd8,0xa0,0x3a,
0x00,0xdc,0xa0,0x3a,
0x00,0xc8,0x16,0x20,
0xd0,0x00,0x00,0x08,
0x00,0x04,0x03,0x20,
0xd0,0x00,0x00,0x08,
0x00,0x18,0x03,0x20,
0x00,0x00,0x14,0x35,
0x01,0x00,0x14,0x35,
0x18,0xb3,0x01,0x00,
0x02,0x30,0x01,0x00,
0x21,0x23,0x01,0x00,
0x11,0x02,0x00,0x26,
0x00,0x00,0x00,0x20,
0x20,0x8c,0x09,0x21,
0x00,0x88,0x0d,0x22,
0x02,0x00,0x00,0x14,
0x00,0xe0,0xc0,0x1c,
0x00,0xe4,0xc0,0x1c,
0x02,0x30,0x01,0x00,
0x29,0x23,0x01,0x00,
0x00,0xd3,0x81,0x3a,
0x01,0xa4,0x16,0x20,
0x02,0x00,0x00,0x14,
0x00,0xd3,0x81,0x3a,
0x2a,0x1b,0x00,0x00,
0xe0,0xfc,0xa1,0x3a,
0x02,0x00,0x00,0x14,
0x00,0xa0,0x16,0x20,
0x02,0x00,0x00,0x14,
0xc0,0xe2,0x80,0x3a,
0xe0,0xe6,0x80,0x3a,
0x00,0xd8,0xc0,0x1c,
0x00,0xdc,0xc0,0x1c,
0xc1,0xda,0x94,0x24,
0xe1,0xde,0x94,0x24,
0x20,0x02,0x00,0x2c,
0xc0,0x02,0x84,0x24,
0x00,0x00,0x00,0x20,
0x3b,0xbb,0x05,0x00,
0x01,0x94,0x16,0x20,
0x00,0x00,0x00,0x0c,
0xd8,0x02,0x90,0x24,
0x3f,0xbb,0x01,0x00,
0x01,0x94,0x16,0x20,
0x01,0x8c,0x16,0x20,
0x16,0xac,0x12,0x20,
0x00,0x00,0x00,0x20,
0x49,0xf3,0x01,0x00,
0x49,0x73,0x00,0x00,
0x41,0x2b,0x00,0x00,
0x05,0x00,0xb4,0x21,
0x01,0x00,0x34,0x24,
0x00,0x00,0x00,0x20,
0x45,0xab,0x01,0x00,
0x48,0x2b,0x00,0x00,
0x00,0x00,0x00,0x0c,
0x17,0xc8,0x12,0x20,
0xc0,0xe6,0x80,0x3a,
0xf3,0x05,0x00,0x08,
0xa5,0x04,0x00,0x08,
0x02,0x00,0x00,0x14,
0x02,0x00,0x00,0x14,
0x02,0x00,0x00,0x14,
0x53,0xc3,0x01,0x00,
0x00,0x90,0x80,0x3a,
0x00,0x00,0xd0,0x1d,
0x08,0x80,0x30,0x3d,
0x02,0x00,0x00,0x14,
0x58,0xc3,0x01,0x00,
0x20,0x94,0x80,0x3a,
0x00,0x00,0xc0,0x1c,
0x20,0x00,0x84,0x3c,
0x09,0x00,0x30,0x3d,
0x00,0x84,0x04,0x3e,
0x02,0x00,0x00,0x14,
0x04,0x00,0xd0,0x1d,
0x08,0x90,0x30,0x3d,
0x02,0x00,0x00,0x14,
0x00,0x00,0xc0,0x1c,
0xa0,0x00,0x84,0x3c,
0x09,0x00,0x30,0x3d,
0x00,0x94,0x04,0x3e,
0x02,0x00,0x00,0x14,
0x67,0xc3,0x01,0x00,
0x40,0x98,0x80,0x3a,
0x02,0x00,0xd0,0x1d,
0x0a,0x88,0x30,0x3d,
0x02,0x00,0x00,0x14,
0x6c,0xc3,0x01,0x00,
0x60,0x9c,0x80,0x3a,
0x00,0x00,0xc0,0x1c,
0x60,0x00,0x84,0x3c,
0x0b,0x00,0x30,0x3d,
0x00,0x8c,0x04,0x3e,
0x02,0x00,0x00,0x14,
0x06,0x00,0xd0,0x1d,
0x0a,0x98,0x30,0x3d,
0x02,0x00,0x00,0x14,
0x00,0x00,0xc0,0x1c,
0xe0,0x00,0x84,0x3c,
0x0b,0x00,0x30,0x3d,
0x00,0x9c,0x04,0x3e,
0x02,0x00,0x00,0x14,
0x00,0x00,0xd0,0x1d,
0x08,0x80,0x30,0x3d,
0x00,0x90,0x20,0x3a,
0x02,0x00,0x00,0x14,
0x00,0x00,0xc0,0x1c,
0x20,0x00,0x84,0x3c,
0x09,0x00,0x30,0x3d,
0x00,0x84,0x04,0x3e,
0x00,0x94,0x20,0x3a,
0x02,0x00,0x00,0x14,
0x00,0x00,0xd0,0x1d,
0x08,0x80,0x30,0x3d,
0x00,0x90,0x20,0x3a,
0x00,0xe4,0x80,0x3a,
0x00,0xe0,0xc0,0x1c,
0x8c,0x13,0x01,0x00,
0x38,0x8b,0x90,0x3b,
0x00,0x98,0x20,0x3a,
0x02,0x00,0x00,0x14,
0x91,0x4b,0x01,0x00,
0x38,0x03,0x90,0x3b,
0x19,0x98,0x30,0x21,
0x38,0x8b,0x90,0x3b,
0x02,0x00,0x00,0x14,
0x38,0x03,0x90,0x3b,
0x19,0x88,0x30,0x21,
0x38,0x9b,0x90,0x3b,
0x02,0x00,0x00,0x14,
0x00,0x00,0xc0,0x1c,
0x20,0x00,0x84,0x3c,
0x09,0x00,0x30,0x3d,
0x00,0x84,0x04,0x3e,
0x00,0x94,0x20,0x3a,
0x20,0xe4,0x80,0x29,
0x00,0xe0,0xc0,0x1c,
0xa4,0x13,0x01,0x00,
0x38,0x8f,0x90,0x3b,
0xa1,0x03,0x01,0x00,
0x61,0x8c,0x94,0x24,
0xa2,0x13,0x00,0x00,
0x61,0x8c,0x94,0x21,
0x00,0x9c,0x20,0x3a,
0xae,0x13,0x00,0x00,
0xa9,0x4b,0x01,0x00,
0x38,0x03,0x90,0x3b,
0x19,0x9c,0x30,0x21,
0x38,0x8f,0x90,0x3b,
0xac,0x13,0x00,0x00,
0x38,0x03,0x90,0x3b,
0x19,0x8c,0x30,0x21,
0x38,0x9f,0x90,0x3b,
0x61,0x8c,0x94,0x24,
0xe1,0x9c,0x94,0x21,
0x03,0x8c,0x10,0x2d,
0x07,0x9c,0x10,0x2d,
0x02,0x00,0x00,0x14,
0x02,0x28,0x01,0x00,
0x00,0x88,0x02,0x20,
0xb9,0x63,0x05,0x00,
0xb9,0x6b,0x05,0x00,
0x01,0x8c,0x16,0x20,
0x01,0x88,0x16,0x20,
0x00,0x8c,0x16,0x20,
0xa5,0x04,0x00,0x08,
0xbb,0x23,0x01,0x00,
0xa5,0x04,0x00,0x08,
0xae,0x04,0x00,0x08,
0x02,0x00,0x00,0x14,
0x00,0x88,0x02,0x20,
0xc5,0x23,0x05,0x00,
0xc5,0x6b,0x05,0x00,
0xc5,0x63,0x05,0x00,
0xc5,0xcb,0x02,0x00,
0x01,0x8c,0x16,0x20,
0x01,0x88,0x16,0x20,
0x00,0x8c,0x16,0x20,
0xae,0x04,0x00,0x08,
0x02,0x00,0x00,0x14,
0xe1,0xde,0x94,0x21,
0xd1,0xfb,0x04,0x00,
0x00,0x02,0x00,0x2c,
0x01,0x00,0x34,0x24,
0x17,0x00,0x30,0x24,
0x00,0x00,0x00,0x20,
0xd1,0xbb,0x01,0x00,
0x00,0xdc,0xa0,0x3a,
0x00,0xe4,0x20,0x3a,
0xf3,0x05,0x00,0x08,
0x02,0x00,0x00,0x14,
0xd0,0x00,0x00,0x08,
0xd5,0xeb,0x05,0x00,
0xd8,0xf3,0x01,0x00,
0xd5,0x1b,0x00,0x00,
0xd8,0xeb,0x01,0x00,
0x80,0xfc,0xa1,0x3a,
0x02,0x00,0x00,0x14,
0xd0,0x00,0x00,0x08,
0x01,0x70,0x16,0x20,
0x02,0x00,0x00,0x14,
0x02,0x00,0x00,0x14,
0x02,0x00,0x00,0x14,
0xd0,0x00,0x00,0x08,
0x00,0x4c,0x03,0x20,
0x00,0xe0,0xc0,0x1c,
0x00,0xe4,0xc0,0x1c,
0x02,0x00,0x00,0x14,
0xd0,0x00,0x00,0x08,
0x00,0xe8,0xc0,0x1c,
0x00,0xec,0xc0,0x1c,
0x18,0x1c,0x12,0x20,
0x19,0x1c,0x12,0x20,
0x1a,0x1c,0x12,0x20,
0x1b,0x1c,0x13,0x20,
0x02,0x00,0x00,0x14,
0xf7,0xe3,0x01,0x00,
0x01,0x00,0x04,0x00,
0x13,0xac,0x02,0x00,
0xf0,0x8b,0x04,0x00,
0xc4,0x11,0x00,0x00,
0x29,0x02,0x00,0x08,
0x00,0xfc,0xa1,0x3a,
0x42,0x02,0x00,0x08,
0x00,0x7c,0x02,0x20,
0xf4,0x63,0x00,0x00,
0xf5,0x6b,0x00,0x00,
0xb6,0x11,0x00,0x00,
0xfa,0x03,0x00,0x00,
0x29,0x04,0x00,0x08,
0x00,0x10,0x00,0x00,
0x00,0x74,0x00,0x00,
0x00,0xec,0x04,0x00,
0x00,0xcc,0x16,0x20,
0xfd,0xfb,0x01,0x00,
0xd7,0x45,0xa2,0x20,
0x05,0xb4,0x06,0x00,
0x01,0xa0,0x16,0x20,
0x01,0xf0,0x16,0x20,
0x29,0x04,0x00,0x08,
0x00,0xf0,0x16,0x20,
0x06,0x14,0x00,0x00,
0x0d,0x94,0x00,0x00,
0x00,0x8c,0x16,0x20,
0x01,0xe4,0x16,0x20,
0x00,0x9c,0x16,0x20,
0xd0,0x00,0x00,0x08,
0x02,0x70,0x16,0x20,
0x00,0x00,0x00,0x20,
0x02,0x00,0x00,0x14,
0x0f,0x9c,0x00,0x00,
0x10,0x14,0x00,0x00,
0x13,0xa4,0x00,0x00,
0x00,0x8c,0x16,0x20,
0x01,0xe4,0x16,0x20,
0x00,0x14,0x00,0x00,
0x01,0x8c,0x16,0x20,
0x01,0xa0,0x16,0x20,
0x01,0xf0,0x16,0x20,
0xc0,0x42,0x81,0x3a,
0xe0,0x46,0x81,0x3a,
0xe0,0x02,0x94,0x24,
0x00,0x00,0x00,0x20,
0x1c,0xa4,0x01,0x00,
0xe1,0x46,0x95,0x24,
0x29,0x04,0x00,0x08,
0x22,0xe4,0x05,0x00,
0xd0,0x00,0x00,0x08,
0x00,0x04,0x03,0x20,
0xd0,0x00,0x00,0x08,
0x00,0x18,0x03,0x20,
0x00,0xf0,0x16,0x20,
0xd0,0x00,0x00,0x08,
0x27,0xe4,0x05,0x00,
0x04,0x70,0x16,0x20,
0x02,0x00,0x00,0x14,
0x05,0x70,0x16,0x20,
0x02,0x00,0x00,0x14,
0x0e,0x00,0xb4,0x21,
0x00,0x00,0x04,0x2d,
0x00,0x00,0x00,0x20,
0x01,0x00,0x34,0x24,
0x00,0x00,0x00,0x20,
0x2b,0xac,0x01,0x00,
0x00,0x00,0x00,0x20,
0x00,0x3c,0x02,0x20,
0x00,0x3c,0x02,0x20,
0x00,0x3c,0x02,0x20,
0x00,0x00,0x00,0x0c,
0x02,0x00,0x00,0x08,
0x37,0xdc,0x00,0x00,
0x00,0x00,0x00,0x0c,
0x39,0xbc,0x00,0x00,
0x51,0x14,0x00,0x00,
0x4a,0x74,0x00,0x00,
0x42,0xcc,0x00,0x00,
0x00,0x94,0x16,0x20,
0x00,0x8c,0x16,0x20,
0x49,0x8c,0x00,0x00,
0xd0,0x00,0x00,0x08,
0x01,0x70,0x16,0x20,
0x00,0xe4,0x16,0x20,
0x34,0x14,0x00,0x00,
0x3b,0x7c,0x00,0x00,
0x00,0x94,0x16,0x20,
0x00,0x8c,0x16,0x20,
0x49,0x8c,0x00,0x00,
0xd0,0x00,0x00,0x08,
0x00,0x70,0x16,0x20,
0x34,0x14,0x00,0x00,
0x00,0x00,0x00,0x0c,
0x51,0xbc,0x04,0x00,
0x4d,0x94,0x00,0x00,
0x42,0x14,0x00,0x00,
0x4f,0x9c,0x00,0x00,
0x42,0x14,0x00,0x00,
0x58,0xa4,0x00,0x00,
0x42,0x14,0x00,0x00,
0xd0,0x00,0x00,0x08,
0x04,0x70,0x16,0x20,
0x01,0xe4,0x16,0x20,
0x01,0xa0,0x16,0x20,
0x00,0x94,0x16,0x20,
0x00,0x8c,0x16,0x20,
0x34,0x14,0x00,0x00,
0xd0,0x00,0x00,0x08,
0x00,0x04,0x03,0x20,
0xd0,0x00,0x00,0x08,
0x00,0x18,0x03,0x20,
0x5f,0xe4,0x05,0x00,
0x64,0x04,0x00,0x08,
0x42,0x14,0x00,0x00,
0x62,0xac,0x06,0x00,
0x67,0x04,0x00,0x08,
0x3b,0x14,0x00,0x00,
0x64,0x04,0x00,0x08,
0x3b,0x14,0x00,0x00,
0x00,0xde,0x00,0x2c,
0x20,0x02,0x00,0x2c,
0x01,0xd8,0x34,0x24,
0x00,0x02,0x00,0x2c,
0xc0,0x02,0x84,0x26,
0x00,0x00,0x00,0x20,
0xe0,0x3e,0x89,0x21,
0x00,0x02,0x00,0x2c,
0x01,0x00,0x34,0x24,
0x11,0x00,0x3c,0x24,
0x00,0x00,0x00,0x20,
0xa4,0xbc,0x05,0x00,
0x00,0x02,0x00,0x2c,
0x00,0x02,0xe4,0x26,
0x00,0x00,0x00,0x20,
0x20,0x02,0xe8,0x21,
0x20,0x00,0xa4,0x21,
0x0f,0x00,0x3c,0x24,
0x00,0x00,0x00,0x20,
0xa4,0xbc,0x01,0x00,
0x11,0xc8,0x1e,0x20,
0x00,0x00,0x00,0x20,
0x00,0x00,0x00,0x36,
0x00,0x00,0x00,0x20,
0x00,0x00,0x00,0x20,
0x00,0x00,0xb8,0x21,
0x00,0xe0,0x00,0x20,
0x00,0x00,0x00,0x36,
0x00,0x00,0x00,0x20,
0x00,0x00,0x00,0x20,
0x00,0x00,0xb8,0x21,
0x00,0xe8,0x00,0x20,
0x00,0xe0,0xa0,0x3a,
0x00,0xe8,0xa0,0x3a,
0x00,0xe6,0xe0,0x3a,
0xf3,0x05,0x00,0x08,
0x18,0x88,0x12,0x20,
0x00,0xe3,0x80,0x29,
0x40,0xeb,0x80,0x29,
0x95,0x64,0x01,0x00,
0x00,0x83,0x80,0x3a,
0x40,0x87,0x80,0x3a,
0x00,0x88,0xa0,0x3a,
0x00,0x8c,0xa0,0x3a,
0x00,0x93,0x80,0x3a,
0x40,0x97,0x80,0x3a,
0x00,0x98,0xa0,0x3a,
0x00,0x9c,0xa0,0x3a,
0x9d,0x14,0x00,0x00,
0x00,0x80,0xa0,0x3a,
0x00,0x84,0xa0,0x3a,
0x00,0x8b,0x80,0x3a,
0x40,0x8f,0x80,0x3a,
0x00,0x90,0xa0,0x3a,
0x00,0x94,0xa0,0x3a,
0x00,0x9b,0x80,0x3a,
0x40,0x9f,0x80,0x3a,
0xba,0x04,0x00,0x08,
0xa2,0xb4,0x01,0x00,
0x00,0x44,0xa1,0x3a,
0x01,0x42,0xf5,0x21,
0xa3,0x14,0x00,0x00,
0x21,0x46,0xf5,0x21,
0x6b,0x14,0x00,0x00,
0x00,0x00,0x00,0x0c,
0x00,0x80,0xa0,0x3a,
0x00,0x84,0xa0,0x3a,
0x00,0x88,0xa0,0x3a,
0x00,0x8c,0xa0,0x3a,
0x00,0x90,0xa0,0x3a,
0x00,0x94,0xa0,0x3a,
0x00,0x98,0xa0,0x3a,
0x00,0x9c,0xa0,0x3a,
0x00,0x00,0x00,0x0c,
0xb0,0x9c,0x01,0x00,
0xa5,0x04,0x00,0x08,
0xb6,0x6c,0x01,0x00,
0x22,0x00,0xb0,0x31,
0x00,0x00,0x04,0x35,
0x23,0x00,0xb0,0x31,
0x00,0x00,0x04,0x35,
0xba,0x14,0x00,0x00,
0x00,0x00,0x10,0x2d,
0x00,0x00,0x04,0x35,
0x01,0x00,0x10,0x2d,
0x00,0x00,0x04,0x35,
0xd0,0x00,0x00,0x08,
0x00,0x08,0x03,0x20,
0xd0,0x00,0x00,0x08,
0x00,0x14,0x03,0x20,
0xc2,0x74,0x00,0x00,
0xe0,0xe2,0x80,0x3a,
0xc0,0xe6,0x80,0x3a,
0xc4,0x14,0x00,0x00,
0x20,0xe2,0xe0,0x3a,
0x00,0xe6,0xe0,0x3a,
0xd4,0xfc,0x00,0x00,
0x00,0x02,0x00,0x2c,
0x18,0x00,0x30,0x24,
0x00,0x00,0x00,0x20,
0xd4,0xac,0x01,0x00,
0x00,0xe0,0xa0,0x3a,
0x21,0xe7,0x94,0x21,
0x00,0xac,0x06,0x20,
0xd0,0x74,0x00,0x00,
0x00,0xdc,0xa0,0x3a,
0xc1,0xda,0x94,0x21,
0xd2,0x14,0x00,0x00,
0x00,0x44,0xa1,0x3a,
0x01,0x42,0xf5,0x21,
0xd2,0x2c,0x00,0x00,
0xf3,0x05,0x00,0x08,
0x00,0xc3,0x80,0x3a,
0x20,0xc7,0x80,0x3a,
0x70,0x02,0xe0,0x26,
0x00,0x00,0x00,0x20,
0x10,0xf4,0x50,0x34,
0x00,0xf0,0xac,0x22,
0xe0,0x6c,0x01,0x00,
0x00,0xb4,0x16,0x20,
0x40,0xe0,0x80,0x3a,
0x60,0xe4,0x80,0x3a,
0x22,0x05,0x00,0x08,
0xe5,0x74,0x00,0x00,
0xe5,0x64,0x01,0x00,
0x01,0xb4,0x16,0x20,
0x00,0xe0,0x80,0x3a,
0x20,0xe4,0x80,0x3a,
0x22,0x05,0x00,0x08,
0xee,0x6c,0x01,0x00,
0x02,0xb4,0x16,0x20,
0xeb,0x8c,0x01,0x00,
0xc0,0xe0,0x80,0x3a,
0xe0,0xe4,0x80,0x3a,
0xed,0x14,0x00,0x00,
0x40,0xe0,0x80,0x3a,
0x60,0xe4,0x80,0x3a,
0x22,0x05,0x00,0x08,
0xf7,0x64,0x01,0x00,
0x03,0xb4,0x16,0x20,
0xf4,0x8c,0x01,0x00,
0x80,0xe0,0x80,0x3a,
0xa0,0xe4,0x80,0x3a,
0xf6,0x14,0x00,0x00,
0x00,0xe0,0x80,0x3a,
0x20,0xe4,0x80,0x3a,
0x22,0x05,0x00,0x08,
0xfa,0x1c,0x01,0x00,
0x76,0x05,0x00,0x08,
0xfb,0x14,0x00,0x00,
0x66,0x05,0x00,0x08,
0x60,0x02,0xe0,0x29,
0x00,0x02,0x04,0x26,
0x00,0x00,0x00,0x20,
0x10,0xf4,0x50,0x34,
0x00,0xf0,0xac,0x22,
0x05,0x6d,0x01,0x00,
0x04,0xb4,0x16,0x20,
0x40,0xe0,0x80,0x3a,
0x60,0xe4,0x80,0x3a,
0x41,0x05,0x00,0x08,
0x0f,0x6d,0x01,0x00,
0x06,0xb4,0x16,0x20,
0x0b,0x8d,0x01,0x00,
0xc0,0xe0,0x80,0x3a,
0xe0,0xe4,0x80,0x3a,
0x0d,0x15,0x00,0x00,
0x40,0xe0,0x80,0x3a,
0x60,0xe4,0x80,0x3a,
0x41,0x05,0x00,0x08,
0x1d,0x75,0x00,0x00,
0x14,0x65,0x01,0x00,
0x05,0xb4,0x16,0x20,
0x00,0xe0,0x80,0x3a,
0x20,0xe4,0x80,0x3a,
0x41,0x05,0x00,0x08,
0x1d,0x65,0x01,0x00,
0x07,0xb4,0x16,0x20,
0x1a,0x8d,0x01,0x00,
0x80,0xe0,0x80,0x3a,
0xa0,0xe4,0x80,0x3a,
0x1c,0x15,0x00,0x00,
0x00,0xe0,0x80,0x3a,
0x20,0xe4,0x80,0x3a,
0x41,0x05,0x00,0x08,
0x20,0x1d,0x01,0x00,
0xac,0x05,0x00,0x08,
0x00,0x00,0x00,0x0c,
0x6e,0x05,0x00,0x08,
0x00,0x00,0x00,0x0c,
0xd0,0x00,0x00,0x08,
0x20,0xe7,0x94,0x3c,
0x18,0xe3,0x90,0x29,
0x39,0xe7,0x90,0x29,
0x00,0x0c,0x03,0x20,
0x00,0xe4,0x04,0x3e,
0x30,0x75,0x01,0x00,
0x2d,0x7d,0x01,0x00,
0x30,0xec,0x00,0x21,
0x00,0xea,0x00,0x23,
0x36,0x15,0x00,0x00,
0x20,0xec,0x00,0x3a,
0x00,0xe8,0x00,0x3a,
0x36,0x15,0x00,0x00,
0x34,0x7d,0x01,0x00,
0xb0,0xec,0x00,0x21,
0x04,0xea,0x00,0x23,
0x36,0x15,0x00,0x00,
0xa0,0xec,0x00,0x3a,
0x80,0xe8,0x00,0x3a,
0x7d,0xef,0x90,0x21,
0x5c,0xeb,0x90,0x22,
0x78,0xef,0x90,0x21,
0x30,0x03,0x80,0x26,
0x1a,0xeb,0x90,0x23,
0x60,0xef,0x88,0x21,
0x40,0xeb,0x8c,0x22,
0x62,0x85,0x01,0x00,
0x7d,0xef,0x9c,0x21,
0x5c,0xeb,0x9c,0x22,
0x62,0x15,0x00,0x00,
0xd0,0x00,0x00,0x08,
0x00,0xe3,0x80,0x27,
0x20,0x03,0x94,0x3c,
0x00,0xe4,0x20,0x27,
0x18,0xe3,0x90,0x29,
0x39,0xe7,0x90,0x29,
0x00,0x0c,0x03,0x20,
0x18,0xe0,0x10,0x2d,
0x19,0xe4,0x10,0x3e,
0x52,0x75,0x01,0x00,
0x4f,0x7d,0x01,0x00,
0x70,0xec,0x00,0x21,
0x02,0xea,0x00,0x23,
0x58,0x15,0x00,0x00,
0x60,0xec,0x00,0x3a,
0x40,0xe8,0x00,0x3a,
0x58,0x15,0x00,0x00,
0x56,0x7d,0x01,0x00,
0xf0,0xec,0x00,0x21,
0x06,0xea,0x00,0x23,
0x58,0x15,0x00,0x00,
0xe0,0xec,0x00,0x3a,
0xc0,0xe8,0x00,0x3a,
0x7d,0xef,0x90,0x21,
0x5c,0xeb,0x90,0x22,
0x78,0xef,0x90,0x21,
0x30,0x03,0x80,0x26,
0x1a,0xeb,0x90,0x23,
0x60,0xef,0x88,0x21,
0x40,0xeb,0x8c,0x22,
0x62,0x85,0x01,0x00,
0x7f,0xef,0x9c,0x21,
0x5e,0xeb,0x9c,0x22,
0xd0,0x00,0x00,0x08,
0x1b,0x10,0x12,0x20,
0x1a,0x10,0x13,0x20,
0x00,0x00,0x00,0x0c,
0x3d,0xec,0x10,0x21,
0x1c,0xe8,0x10,0x22,
0x6b,0x0d,0x01,0x00,
0x70,0xef,0x80,0x21,
0x1a,0xea,0x10,0x23,
0x7d,0xe7,0x9c,0x21,
0x5c,0xe3,0x9c,0x22,
0xe5,0x15,0x00,0x00,
0x7d,0xec,0x10,0x21,
0x5c,0xe8,0x10,0x22,
0x73,0x0d,0x01,0x00,
0x70,0xef,0x80,0x21,
0x1a,0xea,0x10,0x23,
0x7f,0xe7,0x9c,0x21,
0x5e,0xe3,0x9c,0x22,
0xe5,0x15,0x00,0x00,
0x90,0x02,0xe0,0x26,
0x00,0x00,0x00,0x20,
0x10,0xf4,0x50,0x34,
0x00,0xf0,0xac,0x22,
0x90,0x5d,0x02,0x00,
0x7d,0x15,0x01,0x00,
0x95,0x15,0x00,0x00,
0xd1,0x03,0x90,0x24,
0x8e,0xbd,0x01,0x00,
0x87,0x4d,0x01,0x00,
0xab,0xe7,0x80,0x21,
0x8a,0xe3,0x80,0x22,
0xb5,0xf7,0x9c,0x24,
0x96,0xf3,0x9c,0x25,
0xa9,0xef,0x80,0x21,
0x88,0xeb,0x80,0x22,
0xe5,0x15,0x00,0x00,
0xab,0xef,0x80,0x21,
0x8a,0xeb,0x80,0x22,
0xb5,0xf7,0x9c,0x24,
0x96,0xf3,0x9c,0x25,
0xa9,0xe7,0x80,0x21,
0x88,0xe3,0x80,0x22,
0xe5,0x15,0x00,0x00,
0xa7,0x4d,0x01,0x00,
0xa2,0x15,0x00,0x00,
0x92,0x05,0x01,0x00,
0x98,0x15,0x00,0x00,
0x94,0x0d,0x01,0x00,
0x95,0x15,0x00,0x00,
0xa2,0x15,0x00,0x00,
0xab,0xef,0x80,0x21,
0x8a,0xeb,0x80,0x22,
0x9a,0x15,0x00,0x00,
0xa9,0xef,0x80,0x21,
0x88,0xeb,0x80,0x22,
0x9d,0x3d,0x00,0x00,
0x06,0x02,0x14,0x26,
0x9f,0x15,0x00,0x00,
0x08,0x02,0x14,0x26,
0x00,0x00,0x00,0x20,
0x60,0xe7,0x88,0x21,
0x40,0xe3,0x8c,0x22,
0xe5,0x15,0x00,0x00,
0xa9,0xef,0x80,0x21,
0x88,0xeb,0x80,0x22,
0xab,0xe7,0x80,0x21,
0x8a,0xe3,0x80,0x22,
0xe5,0x15,0x00,0x00,
0xa9,0xe7,0x80,0x21,
0x88,0xe3,0x80,0x22,
0xab,0xef,0x80,0x21,
0x8a,0xeb,0x80,0x22,
0xe5,0x15,0x00,0x00,
0x60,0x02,0xe0,0x29,
0x00,0x00,0x20,0x29,
0x00,0x02,0x04,0x26,
0x00,0x00,0x00,0x20,
0x10,0xf4,0x50,0x34,
0x00,0xf0,0xac,0x22,
0xcd,0x5d,0x02,0x00,
0xb5,0x15,0x01,0x00,
0xd2,0x15,0x00,0x00,
0xcb,0x45,0x00,0x00,
0xd1,0x03,0x90,0x24,
0xcb,0xbd,0x01,0x00,
0xc2,0x4d,0x01,0x00,
0xaf,0xe7,0x80,0x21,
0x8e,0xe3,0x80,0x22,
0xad,0xf7,0x80,0x21,
0x8c,0xf3,0x80,0x22,
0xaf,0xf7,0x80,0x24,
0x8e,0xf3,0x80,0x25,
0xad,0xef,0x80,0x21,
0x8c,0xeb,0x80,0x22,
0xe5,0x15,0x00,0x00,
0xaf,0xef,0x80,0x21,
0x8e,0xeb,0x80,0x22,
0xad,0xf7,0x80,0x21,
0x8c,0xf3,0x80,0x22,
0xaf,0xf7,0x80,0x24,
0x8e,0xf3,0x80,0x25,
0xad,0xe7,0x80,0x21,
0x8c,0xe3,0x80,0x22,
0xe5,0x15,0x00,0x00,
0xe1,0x4d,0x01,0x00,
0xdc,0x15,0x00,0x00,
0xcf,0x05,0x01,0x00,
0xd5,0x15,0x00,0x00,
0xd1,0x0d,0x01,0x00,
0xd2,0x15,0x00,0x00,
0xdc,0x15,0x00,0x00,
0xaf,0xef,0x80,0x21,
0x8e,0xeb,0x80,0x22,
0xd7,0x15,0x00,0x00,
0xad,0xef,0x80,0x21,
0x8c,0xeb,0x80,0x22,
0x04,0x02,0x14,0x26,
0x00,0x00,0x00,0x20,
0x60,0xe7,0x88,0x21,
0x40,0xe3,0x8c,0x22,
0xe5,0x15,0x00,0x00,
0xad,0xef,0x80,0x21,
0x8c,0xeb,0x80,0x22,
0xaf,0xe7,0x80,0x21,
0x8e,0xe3,0x80,0x22,
0xe5,0x15,0x00,0x00,
0xad,0xe7,0x80,0x21,
0x8c,0xe3,0x80,0x22,
0xaf,0xef,0x80,0x21,
0x8e,0xeb,0x80,0x22,
0xd0,0x00,0x00,0x08,
0x19,0x02,0x14,0x24,
0xed,0xbd,0x01,0x00,
0x1b,0x1c,0x12,0x20,
0x1a,0x1c,0x12,0x20,
0x19,0x1c,0x12,0x20,
0x18,0x1c,0x13,0x20,
0x00,0x00,0x00,0x0c,
0xd0,0x00,0x00,0x08,
0x09,0x1c,0x02,0x20,
0x08,0x1c,0x02,0x20,
0x09,0x1c,0x02,0x20,
0x08,0x1c,0x03,0x20,
0x00,0x00,0x00,0x0c,
0x19,0x4c,0x11,0x30,
0xf6,0x15,0x01,0x00,
0x00,0x4c,0x05,0x2d,
0x00,0x50,0x21,0x29,
0xfd,0x3d,0x00,0x00,
0x26,0x03,0x94,0x26,
0x00,0x00,0x00,0x20,
0x00,0x50,0x41,0x3a,
0xfd,0x15,0x01,0x00,
0x00,0x50,0x05,0x2d,
0x00,0x00,0x00,0x0c,
0x10,0x00,0x06,0x00,
};

const int brazos_macro_video_ucode_size = 6140;

/****************************************************************************
 * Modifications:
 * $Log: 
 *  12   mpeg      1.11        2/17/04 3:23:25 PM     Dave Wilson     CR(s) 
 *        8322 8323 : Microcode version 0.15 - includes a fix for hesitation 
 *        after declaring sync and checks for new commands every time a 
 *        sequence end is found in the stream. This allows the end of still 
 *        decode operations to be reliably determined by the video driver.
 *  11   mpeg      1.10        1/26/04 3:24:32 PM     Mark Thissen    CR(s) 
 *        8269 8270 : Numerous changes to support declaration of early sync 3 
 *        frames early.
 *        
 *  10   mpeg      1.9         12/12/03 10:03:34 AM   Xin Golden      CR(s) 
 *        8135 8136 : fixed the decoder hang when encountering attenuated/noisy
 *         input signals
 *  9    mpeg      1.8         11/24/03 11:11:01 AM   Mark Thissen    CR(s): 
 *        8011 8012 Video ucode to provide fix for attenuated/high BER data 
 *        causing Vcore to hang and requiring either Vcore reset or issuance of
 *         continue command.
 *        
 *  8    mpeg      1.7         10/14/03 4:51:45 PM    Billy Jackman   CR(s): 
 *        7638 Brazos video microcode version 0.11.  Channel change 
 *        improvements and HD
 *        
 *        stream tolerance.  If a stream provides video that is too large, set 
 *        the width
 *        
 *        to 0x3f0 so the driver can detect this.
 *  7    mpeg      1.6         10/2/03 1:06:22 PM     Dave Wilson     SCR(s) 
 *        7085 7611 :
 *        Microcode version 0.10 - fixes a problem in previous versions where 
 *        the I
 *        and P buffer pointers set would not necessarily be applied to the 
 *        correct
 *        buffers. This resulted in unreliable determination of the next buffer
 *         to be
 *        decoded into and, as a consequence, some crud seen in decoding stills
 *         for
 *        display or into background buffers.
 *        
 *  6    mpeg      1.5         9/11/03 12:22:36 PM    Larry Wang      SCR(s) 
 *        7417 :
 *        Correct A/V sync problems based on the modifications in the previous 
 *        revision.
 *        
 *  5    mpeg      1.4         9/4/03 6:33:20 PM      Larry Wang      SCR(s) 
 *        7417 :
 *        fix DSS video skip problem due to STC carrying over to bit 33.
 *        
 *  4    mpeg      1.3         8/21/03 5:23:08 PM     Mark Thissen    SCR(s) 
 *        7338 7339 :
 *        Fixed decoder lock up problem seen when noisy or attenuated signals 
 *        were being demodulated.
 *        
 *  3    mpeg      1.2         7/30/03 5:54:50 PM     Dave Wilson     SCR(s) 
 *        6769 :
 *        Microcode version 0.6 - changes still decode sequencing such that 
 *        decode 
 *        continues until a stop command is received regardless of sequence end
 *         tags in
 *        the data.
 *        
 *  2    mpeg      1.1         4/15/03 11:56:12 AM    Dave Wilson     SCR(s) 
 *        6023 :
 *        Microcode version 0.3. Includes newly defined bits in the 
 *        MPG_ADDR_EXT_REG
 *        to indicate which type of picture was decoded when a decode complete 
 *        interrupt
 *        is received.
 *        
 *  1    mpeg      1.0         3/20/03 3:06:48 PM     Dave Wilson     
 * $
 ****************************************************************************/
