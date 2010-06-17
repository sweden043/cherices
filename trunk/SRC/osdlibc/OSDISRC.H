/****************************************************************************/
/*                   CONEXANT PROPRIETARY AND CONFIDENTIAL                  */
/*                       SOFTWARE FILE/MODULE HEADER                        */
/*          Conexant Systems Inc. (c) 1998, 1999, 2000, 2001, 2002          */
/*                              Austin, TX                                  */
/*                         All Rights Reserved                              */
/****************************************************************************/
/*
 * Filename:        osdisr.h
 *
 *
 * Description:     osdlib - OSD Library. ISR include file.
 *
 *
 * Author:          Dave Wilson
 *
 ****************************************************************************/
/* $Header: OSDISRC.H, 6, 9/6/02 4:21:32 PM, Matt Korte$
 ****************************************************************************/
#ifndef _OSDISR_H_
#define _OSDISR_H_

// Internal structure used when registering callback clients for the display
// line interrupt.
typedef struct _LINECALLBACK *PLINECALLBACK;
typedef struct _LINECALLBACK {
   PLINECALLBACK  pNext;
   PLINECALLBACK  pPrev;
   u_int32        dwLine;
   POSDLINEISR    pfnCallback;
   OSDFIELD       oField;
} LINECALLBACK;

void OSDIsrInit(void);
int OSDIsr(u_int32 dwIntID, bool bFIQ, PFNISR *pfnChain);
void MPGSizeARChange(void *pParam);
bool osdLineIntRegister(queue_id_t quID, u_int32 dwLine);
bool osdLineIntFree(queue_id_t quID);
bool osdSetMpgScaleDelay(u_int32 dwDelay);
u_int32 osdGetMpgScaleDelay(void);

void osdLineISR(u_int32 dwLine, OSDFIELD oField);
bool osdRegisterLineISR(POSDLINEISR pfnCallback, 
                        u_int32 dwLine, 
                        OSDFIELD oSignal);
void osdUnRegisterLineISR(POSDLINEISR pfnCallback, u_int32 dwLine);
void osdTTXLineISR(u_int32 dwLine, OSDFIELD oField);

/* Used in deferred MPEG size/aspect ratio updates */
extern void osdMpegInt5Received(void);

#endif /* _OSDISR_H_ */
/****************************************************************************
 * Modifications:
 * $Log: 
 *  6    mpeg      1.5         9/6/02 4:21:32 PM      Matt Korte      SCR(s) 
 *        4498 :
 *        Fixed Warnings
 *        
 *  5    mpeg      1.4         5/21/02 12:57:32 PM    Dave Wilson     SCR(s) 
 *        3609 :
 *        Added prototype for osdMpegInt5Received callback (messed up the last 
 *        PUT!)
 *        
 *  4    mpeg      1.3         5/21/02 12:51:18 PM    Dave Wilson     SCR(s) 
 *        3609 :
 *        AbortD
 *        
 *        
 *  3    mpeg      1.2         8/16/00 3:08:58 PM     Miles Bintz     modified 
 *        line int calls to uses queue instead of semaphores
 *        
 *  2    mpeg      1.1         2/28/00 3:31:20 PM     Rob Tilton      Added new
 *         ISR Line Callback API.
 *        
 *  1    mpeg      1.0         1/5/00 10:24:48 AM     Rob Tilton      
 * $
 * 
 *    Rev 1.5   06 Sep 2002 15:21:32   kortemw
 * SCR(s) 4498 :
 * Fixed Warnings
 ****************************************************************************/
