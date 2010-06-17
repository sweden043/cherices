/****************************************************************************/
/*                   CONEXANT PROPRIETARY AND CONFIDENTIAL                  */
/*                       SOFTWARE FILE/MODULE HEADER                        */
/*                 Conexant Systems Inc. (c) 1998, 1999, 2000               */
/*                               Austin, TX                                 */
/*                            All Rights Reserved                           */
/****************************************************************************/
/*
 * Filename:      genir.h
 *
 * Description:   Generic Infra Red Remote Control Driver Header.   
 *
 * Author:        Anzhi Chen
 *
 ****************************************************************************/
/* $Id: genir.h,v 1.21, 2003-12-04 22:17:19Z, Tim Ross$
 ****************************************************************************/
#ifndef _GENIR_H_
#define _GENIR_H_

#define IR_BUFFER_SIZE 512   /* size of the ring buffer which reads FIFO values in ISR */
#define IR_INDEX_MASK IR_BUFFER_SIZE-1
#define IR_LEVEL_MASK  0x10000
#define IR_DATA_MASK 0x0000FFFF

#define IRRX_REMOTEBUTTON 0x0001
#define IRRX_TRACKBALL 0x0002	
#define IRRX_KEYBOARD 0x0004
#define IRRX_UNKNOWN 0x0008
#define IRRX_FRONT 0x0010
#define IRRX_BACK 0x0020
#define IRRX_DEVICE_0 0x0100
#define IRRX_DEVICE_1 0x0200
#define IRRX_DEVICE_2 0x0400
#define IRRX_DEVICE_3 0x0800

#define IRRX_KEYDOWN 0x0001
#define IRRX_KEYUP 0x0002	
#define IRRX_KEYHOLD 0x0003

/* Return code from subfuctions */
#define IR_SUCCESS 0
#define IR_MISSING_DATA 1
#define IR_WRONG_DATA 2
#define IR_REPEAT 3
#define IR_HEADER 4
#define IR_PACKET_DATA 5
#define IR_END 6

// Debug messaging functions
#define IRD_ERROR_MSG       (TRACE_IRD|TRACE_LEVEL_ALWAYS)
#define IRD_MSG             (TRACE_IRD|TRACE_LEVEL_2)
#define IRD_ISR_MSG         (TRACE_IRD|TRACE_LEVEL_1)

/* irrx function return status codes */
typedef enum
{
   IRRX_OK,
   IRRX_ERROR
} IRRX_STATUS;

/* irrx location code */
typedef enum
{
   IRRX_TYPE_FRONT,
   IRRX_TYPE_BACK
} IRRX_PORT;

typedef enum
{             
   PORT_UNKNOWN, 
   PORT_FRONT, 
   PORT_BACK
} IRRX_PORTID;

typedef struct _IRRX_HWINFO {
   u_int32 control_val;
   u_int32 clkdivide_val;
   u_int32 filter_val;
} IRRX_HWINFO, *PIRRX_HWINFO;

typedef struct _IRRX_KEYINFO {
   u_int32 key_code;    /* remote control or keyboard only */
   u_int32 buttonmask;  /* xy device only */
   int16 deltax;        /* xy device only */
   int16 deltay;        /* xy device only */
   u_int16 user_data;
   u_int8 device_type;
   u_int8 key_state;    /* remote control or keyboard only */
} IRRX_KEYINFO, *PIRRX_KEYINFO;

typedef struct _IRRX_DATAINFO {
   u_int32 data[IR_BUFFER_SIZE]; /* IR ring buffer */
   u_int16  iread;               /* read index     */
   u_int16  iwrite;              /* write index    */
   sem_id_t sem_id;
} IRRX_DATAINFO, *PIRRX_DATAINFO;

typedef void (*PFNDECODE)(void *, PIRRX_DATAINFO, PIRRX_KEYINFO);
typedef void (*PFNNOTIFY)(void *, PIRRX_KEYINFO);  
typedef void (*PFNIRINIT)(IRRX_PORTID);
typedef IRRX_STATUS (*PFNIRSWINIT)(IRRX_PORTID, void **);

typedef struct _IRRX_INFO {
   IRRX_PORTID portid;
   PFNDECODE pfndecode;
   void * pinst_decode;
   PFNNOTIFY pfnnotify;
   void * pinst_notify;
} IRRX_INFO, *PIRRX_INFO;

IRRX_STATUS cnxt_irrx_init(const IRRX_PORT port, PFNIRINIT pfninit);
IRRX_STATUS cnxt_irrx_swinit(const IRRX_PORTID portid, PFNIRSWINIT pfnswinit, void ** ppinstdecode);
IRRX_STATUS cnxt_irrx_register(IRRX_PORTID portid, PFNDECODE pfndecode, void * pinst_decode, 
                              PFNNOTIFY pfnnotify, void *pinst_notify);

/* followings are the Conexant defined key codes */
#define CNXT_NUL          0x0   
#define CNXT_SOH          0x1 
#define CNXT_STX          0x2 
#define CNXT_ETX          0x3 
#define CNXT_EOT          0x4 
#define CNXT_ENQ          0x5 
#define CNXT_ACK          0x6 
#define CNXT_BEL          0x7
#define CNXT_BS           0x8
#define CNXT_HT           0x9
#define CNXT_LF           0xa
#define CNXT_VT           0xb
#define CNXT_FF           0xc
#define CNXT_CR           0xd
#define CNXT_SO           0xe
#define CNXT_SI           0xf
#define CNXT_DLE          0x10
#define CNXT_DC1          0x11
#define CNXT_DC2          0x12
#define CNXT_DC3          0x13
#define CNXT_DC4          0x14
#define CNXT_NAK          0x15
#define CNXT_SYN          0x16
#define CNXT_ETB          0x17
#define CNXT_CAN          0x18
#define CNXT_EM           0x19
#define CNXT_SUB          0x1a
#define CNXT_ESC          0x1b
#define CNXT_FS           0x1c
#define CNXT_GS           0x1d
#define CNXT_RS           0x1e
#define CNXT_US           0x1f
#define CNXT_SP           0x20
#define CNXT_EXCLAMATION  0x21
#define CNXT_QUOTATION    0x22
#define CNXT_POUND        0x23
#define CNXT_DOLLAR       0x24
#define CNXT_PERCENT      0x25
#define CNXT_AMPERSAND    0x26
#define CNXT_APOSTROPHE   0x27
#define CNXT_LPARENTHESIS 0x28
#define CNXT_RPARENTHESIS 0x29
#define CNXT_ASTERISK     0x2a
#define CNXT_PLUS         0x2b
#define CNXT_COMMA        0x2c
#define CNXT_HYPHEN       0x2d
#define CNXT_PERIOD       0x2e
#define CNXT_BACKSLASH    0x2f
#define CNXT_0            0x30
#define CNXT_1            0x31
#define CNXT_2            0x32
#define CNXT_3            0x33
#define CNXT_4            0x34
#define CNXT_5            0x35
#define CNXT_6            0x36
#define CNXT_7            0x37
#define CNXT_8            0x38
#define CNXT_9            0x39
#define CNXT_COLON        0x3a
#define CNXT_SEMICOLON    0x3b
#define CNXT_LESSTHAN     0x3c
#define CNXT_EQUALS       0x3d
#define CNXT_GREATERTHAN  0x3e
#define CNXT_QUESTION     0x3f
#define CNXT_AT           0x40
#define CNXT_CAPA         0x41
#define CNXT_CAPB         0x42
#define CNXT_CAPC         0x43
#define CNXT_CAPD         0x44
#define CNXT_CAPE         0x45
#define CNXT_CAPF         0x46
#define CNXT_CAPG         0x47
#define CNXT_CAPH         0x48
#define CNXT_CAPI         0x49
#define CNXT_CAPJ         0x4a
#define CNXT_CAPK         0x4b
#define CNXT_CAPL         0x4c
#define CNXT_CAPM         0x4d
#define CNXT_CAPN         0x4e
#define CNXT_CAPO         0x4f
#define CNXT_CAPP         0x50
#define CNXT_CAPQ         0x51
#define CNXT_CAPR         0x52
#define CNXT_CAPS         0x53
#define CNXT_CAPT         0x54
#define CNXT_CAPU         0x55
#define CNXT_CAPV         0x56
#define CNXT_CAPW         0x57
#define CNXT_CAPX         0x58
#define CNXT_CAPY         0x59
#define CNXT_CAPZ         0x5a
#define CNXT_LBRACKET     0x5b
#define CNXT_REVSOLIDUS   0x5c
#define CNXT_RBRACKET     0x5d
#define CNXT_ACCENT       0x5e
#define CNXT_LOWLINE      0x5f
#define CNXT_GRAVEACCENT  0x60
#define CNXT_LOWA         0x61
#define CNXT_LOWB         0x62
#define CNXT_LOWC         0x63
#define CNXT_LOWD         0x64
#define CNXT_LOWE         0x65
#define CNXT_LOWF         0x66
#define CNXT_LOWG         0x67
#define CNXT_LOWH         0x68
#define CNXT_LOWI         0x69
#define CNXT_LOWJ         0x6a
#define CNXT_LOWK         0x6b
#define CNXT_LOWL         0x6c
#define CNXT_LOWM         0x6d
#define CNXT_LOWN         0x6e
#define CNXT_LOWO         0x6f
#define CNXT_LOWP         0x70
#define CNXT_LOWQ         0x71
#define CNXT_LOWR         0x72
#define CNXT_LOWS         0x73
#define CNXT_LOWT         0x74
#define CNXT_LOWU         0x75
#define CNXT_LOWV         0x76
#define CNXT_LOWW         0x77
#define CNXT_LOWX         0x78
#define CNXT_LOWY         0x79
#define CNXT_LOWZ         0x7a
#define CNXT_LBRACE       0x7b     
#define CNXT_VERTLINE     0x7c
#define CNXT_RBRACE       0x7d
#define CNXT_TILDE        0x7e
#define CNXT_DELETE       0x7f
#define CNXT_CAPLOCK      0x80
#define CNXT_LSHIFT       0x81
#define CNXT_RSHIFT       0x82
#define CNXT_LCTRL        0x83
#define CNXT_LALT         0x84
#define CNXT_RALT         0x85
#define CNXT_INSERT       0x86
#define CNXT_LARROW       0x87
#define CNXT_HOME         0x88
#define CNXT_END          0x89
#define CNXT_UPARROW      0x8a
#define CNXT_DOWNARROW    0x8b
#define CNXT_PAGEUP       0x8c
#define CNXT_PAGEDOWN     0x8d
#define CNXT_RARROW       0x8e
#define CNXT_NUMLOCK      0x8f
#define CNXT_F1           0x90 
#define CNXT_F2           0x91 
#define CNXT_F3           0x92 
#define CNXT_F4           0x93 
#define CNXT_F5           0x94 
#define CNXT_F6           0x95 
#define CNXT_F7           0x96 
#define CNXT_F8           0x97
#define CNXT_F9           0x98
#define CNXT_F10          0x99
#define CNXT_F11          0x9a
#define CNXT_F12          0x9b
#define CNXT_PRINTSCREEN  0x9c
#define CNXT_SCROLLLOCK   0x9d
#define CNXT_PAUSE        0x9e 
#define CNXT_FN           0x9f 
#define CNXT_LWINDOWS     0xa0
#define CNXT_APPLICATION  0xa1
#define CNXT_RECORD       0xa2
#define CNXT_EJECT        0xa3
#define CNXT_SEARCH       0xa4
#define CNXT_BATTERYLOW   0xa5
#define CNXT_CUT          0xa6
#define CNXT_PASTE        0xa7

#define CNXT_POWER        0xb0             
#define CNXT_FORWARD      0xb1          
#define CNXT_BUY          0xb2       /*股票*/   
#define CNXT_GUIDE        0xb3            
#define CNXT_ENTER        0xb4                
#define CNXT_EXIT         0xb5         
#define CNXT_REVERSE      0xb6
#define CNXT_WWW          0xb7
#define CNXT_PLAY         0xb8
#define CNXT_STOP         0xb9
#define CNXT_MENU         0xba
#define CNXT_PC           0xbb
#define CNXT_HELP         0xbc
#define CNXT_TVVCR        0xbd    
#define CNXT_LAST         0xbe      
#define CNXT_LOCK         0xbf
#define CNXT_CABLE        0xc0
#define CNXT_VOLUP        0xc1       
#define CNXT_VOLDN        0xc2
#define CNXT_MUTE         0xc3
#define CNXT_CHANUP       0xc4
#define CNXT_CHANDN       0xc5
#define CNXT_INFO         0xc6
#define CNXT_TV           0xc7
#define CNXT_RM_1         0xc8
#define CNXT_RM_2         0xc9
#define CNXT_RM_3         0xca
#define CNXT_RM_4         0xcb
#define CNXT_RM_5         0xcc
#define CNXT_RM_6         0xcd
#define CNXT_RM_7         0xce
#define CNXT_RM_8         0xcf
#define CNXT_RM_9         0xd0
#define CNXT_RM_0         0xd1
#define CNXT_TEXT         0xd2                   
#define CNXT_UP           0xd3
#define CNXT_DOWN         0xd4
#define CNXT_LEFT         0xd5
#define CNXT_RIGHT        0xd6
#define CNXT_RED          0xd7
#define CNXT_GREEN        0xd8   
#define CNXT_YELLOW       0xd9
#define CNXT_BLUE         0xda
#define CNXT_BOXOFFICE    0xdb
#define CNXT_SERVICES     0xdc
#define CNXT_SAT          0xdd /*data broadcast*/
#define CNXT_BACKUP       0xde
#define CNXT_TVGUIDE      0xdf
#define CNXT_INTERACTIVE  0xe0
#define CNXT_EMAIL        0xe1
#define CNXT_REFRESH      0xe2
#define CNXT_PRINT        0xe3
#define CNXT_INPUT        0xe4
#define CNXT_SLEEP        0xe5
#define CNXT_DISPLAY      0xe6
#define CNXT_PARENT_CTL   0xe7
#define CNXT_CANCEL       0xe8
#define CNXT_RSV0         0xe9/*old 声道*/
#define CNXT_MAIL         0xea/*邮件-  短信*/
#define CNXT_RSV2         0xeb/*old 节目分类*/
#define CNXT_RSV3         0xec/*old 中/英*/
#define CNXT_MODE         0xed 
#define CNXT_VIEW         0xed/*mosaic*/
#define CNXT_ADDRESS      0xef 
#define CNXT_RECALL       0xf0
#define CNXT_Factory_Test 0xf1 // add by mxd for factory test
#define CNXT_GAME          0xf2//add for the new remote control device
#define CNXT_TVLIST        0xf3//add for the new remote control device

#define CNXT_VOD          0xf7     /*点播*/
#define CNXT_CINAME       0xf8        /*影院*/
#define CNXT_TRACK        0xf9    /*声道*/                                                                                       
#define CNXT_CHANNEL      0xfa  /*频道*/
#define CNXT_EPG               0xfb     
#define CNXT_DATEBOAST           0xfc       /*资讯*/
#define CNXT_SORT           0xfd     /*分类*/                                                                         
#define CNXT_RATOTV      0xff/*电视/广播*/     

#endif /* _GENIR_H_ */
 /****************************************************************************
 * Modifications:
 * $Log: 
 *  22   mpeg      1.21        12/4/03 4:17:19 PM     Tim Ross        CR(s) 
 *        8098 8099 8100 : Added new subfunction retcodes to support Direct TV 
 *        IR remote.
 *  21   mpeg      1.20        10/15/02 4:58:46 PM    Steve Glennon   SCR(s): 
 *        4796 
 *        Added prototype for cnxt_irrx_swinit
 *        
 *        
 *  20   mpeg      1.19        10/25/01 4:06:26 PM    Dave Wilson     SCR(s) 
 *        2774 :
 *        Added code to include the Seijin protocol user_id bits in the 
 *        structure
 *        passed to IR_Notify in OTV2CTRL. This allows us to differentiate 
 *        between
 *        devices with different IDs and decide whether we need to remap 
 *        trackball
 *        packets to keystrokes or not. The convention assumed is that 
 *        keyboards
 *        (which have DIP switches allowing the user_id to be set) will send a
 *        non-zero value whereas remotes (which do not have DIP switches) will
 *        send 0.
 *        
 *        
 *  19   mpeg      1.18        7/3/01 10:43:30 AM     Tim White       SCR(s) 
 *        2178 2179 2180 :
 *        Merge branched Hondo specific code back into the mainstream source 
 *        database.
 *        
 *        
 *  18   mpeg      1.17        5/14/01 11:34:26 AM    Anzhi Chen      Moved 
 *        subfunction return function definitions to here.
 *        
 *  17   mpeg      1.16        4/12/01 5:12:26 PM     Anzhi Chen      Renamed 
 *        IRRX_BAD as IRRX_UNKNOWN.
 *        
 *  16   mpeg      1.15        3/15/01 5:28:16 PM     Anzhi Chen      Added 
 *        definition for IR_DATA_MASK.
 *        
 *  15   mpeg      1.14        3/12/01 5:03:46 PM     Anzhi Chen      Changed 
 *        the definition values for IRRX_TYPE's.
 *        
 *  14   mpeg      1.13        3/12/01 3:33:22 PM     Anzhi Chen      Added 
 *        several more key code definitions.
 *        
 *  13   mpeg      1.12        2/28/01 4:37:02 PM     Anzhi Chen      Moved 
 *        some definitions around.
 *        
 *  12   mpeg      1.11        2/28/01 3:28:36 PM     Anzhi Chen      Renamed 
 *        IRRX_DEVICE_0 to IRRX_DEVICE_INDEX_0 to indicate it's index.
 *        
 *  11   mpeg      1.10        2/28/01 11:52:18 AM    Anzhi Chen      Added 
 *        three CNXT_ key codes.
 *        
 *  10   mpeg      1.9         2/28/01 10:52:38 AM    Anzhi Chen      Added 
 *        definitions for device indexs.
 *        
 *  9    mpeg      1.8         2/27/01 4:32:18 PM     Anzhi Chen      Changed 
 *        CNXT_TABLE_SIZE to 0xe4.
 *        
 *  8    mpeg      1.7         2/27/01 4:07:26 PM     Anzhi Chen      Added 
 *        CNXT_TABLE_SIZE.
 *        
 *  7    mpeg      1.6         2/22/01 5:28:40 PM     Anzhi Chen      Added 
 *        three definitions for the three remote-style buttons on Sejin 
 *        keyboard.
 *        
 *  6    mpeg      1.5         2/22/01 12:05:54 PM    Anzhi Chen      Added 
 *        more members to the IRRX_KEYINFO structure for the xy device.
 *        
 *  5    mpeg      1.4         2/13/01 3:26:08 PM     Anzhi Chen      Changed 
 *        the protocol for cnxt_irrx_init().
 *        
 *  4    mpeg      1.3         2/7/01 6:37:12 PM      Anzhi Chen      Added 
 *        missed key codes.
 *        
 *  3    mpeg      1.2         2/7/01 2:36:32 PM      Anzhi Chen      Changed 
 *        PFNDECODE type definition.
 *        
 *  2    mpeg      1.1         2/6/01 6:48:22 PM      Anzhi Chen      Fixed 
 *        compiler errors.
 *        
 *  1    mpeg      1.0         2/6/01 3:41:46 PM      Anzhi Chen      
 * $
 ****************************************************************************/ 
                                                                               
