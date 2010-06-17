/*****************************************************************************/
/* File: irlib.h                                                             */
/*                                                                           */
/* Module: Infrared Decode Library.                                          */
/*                                                                           */
/* Description:  Hardware API for the Infrared Decode Module.                */
/*                                                                           */
/* Copyright, 1999 Conexant Systems, Inc. All Rights Reserved                */
/*****************************************************************************/
/*****************************************************************************
$Header: irlib.h, 2, 11/12/99 4:43:14 PM, $
$Log: 
 2    mpeg      1.1         11/12/99 4:43:14 PM    Rob Tilton      Changed to a
        generic keyboard and cursor callback.
       
 1    mpeg      1.0         11/9/99 10:44:10 AM    Rob Tilton      
$
 * 
 *    Rev 1.1   12 Nov 1999 16:43:14   rtilton
 * Changed to a generic keyboard and cursor callback.
 * 
 *    Rev 1.0   09 Nov 1999 10:44:10   rtilton
 * Initial revision.
*****************************************************************************/
#ifndef _IRLIB_H_
#define _IRLIB_H_

#define IR_TAB             0x100
#define IR_BACKSP          0x101
#define IR_RTARROW         0x102
#define IR_LTARROW         0x103
#define IR_UPARROW         0x104
#define IR_DNARROW         0x105
#define IR_LSHIFT          0x106
#define IR_RSHIFT          0x107
#define IR_INSERT          0x108
#define IR_DELETE          0x109
#define IR_PGUP            0x10A
#define IR_PGDN            0x10B
#define IR_HOME            0x10C
#define IR_END             0x10D
#define IR_WINDOWS         0x10E
#define IR_BREAK           0x10F
#define IR_ESCAPE          0x110
#define IR_F1              0x111
#define IR_F2              0x112
#define IR_F3              0x113
#define IR_F4              0x114
#define IR_F5              0x115
#define IR_F6              0x116
#define IR_F7              0x117
#define IR_F8              0x118
#define IR_F9              0x119
#define IR_F10             0x11A
#define IR_F11             0x11B
#define IR_F12             0x11C
#define IR_CTRL            0x11D
#define IR_FN              0x11E
#define IR_MENU            0x11F
#define IR_ALT             0x120
#define IR_CAPLOCK         0x121
#define IR_NUMLOCK         0x122
#define IR_SCRLOCK         0x123
#define IR_PRTSC           0x124
#define IR_BACKTAB         0x125
#define IR_ENTER           0x126

#define IR_FORWARD         0x140
#define IR_CLEAR           0x141
#define IR_SELECT          0x142
#define IR_REVERSE         0x143
#define IR_RECORD          0x144
#define IR_PLAY            0x145
#define IR_PAUSE           0x146
#define IR_STOP            0x147
#define IR_RECALL          0x148
#define IR_DISPLAY         0x149
#define IR_TV_VCR          0x14A
#define IR_INPUT           0x14B
#define IR_PIP             0x14C
#define IR_CH_CNTRL        0x14D
#define IR_SIZE            0x14E
#define IR_SWAP            0x14F
#define IR_MUTE            0x150
#define IR_SLEEP           0x151
#define IR_GUIDE           0x152
#define IR_POWER           0x153

// Remote button defines.
#define IR_BUTTON_1        1
#define IR_BUTTON_2        2

// Key action commands for keyboard callbacks.
#define IR_KEY_UP          1
#define IR_KEY_DOWN        2

// Callback function pointers.
typedef void (*PFIRKBHANDLER)(u_int32 dwAction, u_int16 wChar);
typedef void (*PFIRCURSORHANDLER)(u_int32 dwButtons, int16 nDeltaX, int16 nDeltaY);

// IRLIB exported API's.
bool irRegisterKbHandler(PFIRKBHANDLER pfnKeyboard);
bool irUnregisterKbHandler(void);
bool irRegisterCursorHandler(PFIRCURSORHANDLER pfnCursor);
bool irUnregisterCursorHandler(void);

#endif /* _IRLIB_H_ */
