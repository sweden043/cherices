/****************************************************************************/
/*                               Conexant Systems                           */
/****************************************************************************/
/*                                                                          */
/* Filename:           TRACE_A.H                                            */
/*                                                                          */
/* Description:        Public header file for end user trace/debug support  */
/*                     routines                                             */
/*                                                                          */
/* Author:             Miles Bintz                                          */
/*                                                                          */
/* Copyright Conexant Systems Inc, 2000                                     */
/* All Rights Reserved.                                                     */
/*                                                                          */
/****************************************************************************/
/*
$Header:                                       
$log:
*/
#ifndef _GENTRACE_H_
#define _GENTRACE_H_

typedef struct _genEvent {
    u_int32 timestamp;
    u_int32 EventType;
    u_int32 value;
} genEventtype;

typedef enum {DBG, TRC, ALERT, ASSRT, TRCDBG_SHUTDOWN} trctype;

typedef struct {
    u_int32 (*Alert0)(void);
    u_int32 (*Alert1)(void);
    u_int32 (*Alert2)(void);
    u_int32 (*Alert3)(void);
    u_int32 (*Alert4)(void);
} AlertCBTable;


#define MAXEVENTS             500
#define GENTRCMAXMSGS         100
#define GENDBGMAXSTRLEN       80

#define LVL_D0                0x00000001
#define LVL_D1                0x00000002
#define LVL_D2                0x00000004
#define LVL_D3                0x00000008
#define LVL_D4                0x00000010
#define LVL_D5                0x00000020
#define LVL_D6                0x00000040
#define LVL_D7                0x00000080
#define LVL_D8                0x00000100
#define LVL_D9                0x00000200
#define LVL_D10               0x00000400
#define LVL_D11               0x00000800
#define LVL_D12               0x00001000
#define LVL_D13               0x00002000
#define LVL_D14               0x00004000
#define LVL_D15               0x00008000
#define LVL_D16               0x00010000
#define LVL_D17               0x00020000
#define LVL_D18               0x00040000
#define LVL_D19               0x00080000
#define LVL_D20               0x00100000
#define LVL_D21               0x00200000
#define LVL_D22               0x00400000
#define LVL_D23               0x00800000
#define LVL_D24               0x01000000
#define LVL_D25               0x02000000
#define LVL_D26               0x04000000
#define LVL_D27               0x08000000
#define LVL_D28               0x10000000
#define LVL_D29               0x20000000
#define LVL_D30               0x40000000
#define LVL_D31               0x80000000

#define LVL_T0                0x00000001
#define LVL_T1                0x00000002
#define LVL_T2                0x00000004
#define LVL_T3                0x00000008
#define LVL_T4                0x00000010
#define LVL_T5                0x00000020
#define LVL_T6                0x00000040
#define LVL_T7                0x00000080
#define LVL_T8                0x00000100
#define LVL_T9                0x00000200
#define LVL_T10               0x00000400
#define LVL_T11               0x00000800
#define LVL_T12               0x00001000
#define LVL_T13               0x00002000
#define LVL_T14               0x00004000
#define LVL_T15               0x00008000
#define LVL_T16               0x00010000
#define LVL_T17               0x00020000
#define LVL_T18               0x00040000
#define LVL_T19               0x00080000
#define LVL_T20               0x00100000
#define LVL_T21               0x00200000
#define LVL_T22               0x00400000
#define LVL_T23               0x00800000
#define LVL_T24               0x01000000
#define LVL_T25               0x02000000
#define LVL_T26               0x04000000
#define LVL_T27               0x08000000
#define LVL_T28               0x10000000
#define LVL_T29               0x20000000
#define LVL_T30               0x40000000
#define LVL_T31               0x80000000

#define LVL_A0                0x01
#define LVL_A1                0x02
#define LVL_A2                0x03
#define LVL_A3                0x04
#define LVL_A4                0x05

/* Used to initialize all (debug, trace, assert, event, alert) functions */
bool genDebugInit();

/* There are 32 debug levels.  setting dbglvl can be used to mask out
 * which dbg levels are output.  genDebugSetLevel returns the last mask
 */
u_int32    genDebugSetLevel(u_int32 dbglvl);
void       genDebugMsg(char* module, u_int32 line, u_int32 dbglvl, char* msg, ...);

/* Trace levels use the same idea as debug levels */
u_int32    genTraceSetLevel(u_int32 trclvl);
void       genTraceMsg(char* module, u_int32 line, u_int32 trclvl, char *msg, ...);

/* Alerts always show up in both debug and release builds.
 * They cannot be masked out. Callbacks are used to take
 * different action depending on the level of the alert.  
 *
 * The callbacks should return a u_int32 which genAlertmsg
 * should then pass back to it's caller
 */
u_int32       genAlertMsg(char* module, u_int32 line, unsigned char alrt_lvl, char *msg, ...);
void          genAlertSetCallback(unsigned char alrtlvl, u_int32 (*alrtcb)(void));

/* Asserts test if a condition is true.  If it isn't, it halts the system
 */
void genAssert(char *module, u_int32 line, char* msg, u_int32 expr, ...);

/* Events are also used to debug/trace info but store values to memory.
 * Event does NOT output anything over a (usually slow) debug channel -- 
 * as such the effects of the Heisenburg Uncertainty Principle are minimized.
 *
 * genEvent will mark a time stamp in addition to the type and value of the posted event.
 * A user defined enum should be defined to set up event types.
 * 
 * A buffer will be globally allocated in the genDebug module.  The address
 * of this function will be printed out during init.  If an IRD crashes and
 * is reset w/o a power cycle, assuming
 *    1: All Memory isn't zeroed at startup
 *    2: The same software is being run
 * Then we can assume that the event buffer will be untouched and in the same
 * location as before.
 *
 * Of course, as with any peice of memory, our event buffer is succeptible to
 * misbehaved pointers.
 *
 * TODO:  Consider allocating memory chunk in code section of startup...
 */

void genEvent(u_int32 typ, u_int32 val);

#endif
