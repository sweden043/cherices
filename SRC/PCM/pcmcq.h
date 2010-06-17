/****************************************************************************/
/*                          Conexant Systems, Inc.                          */
/****************************************************************************/
/*                                                                          */
/* Filename:       PCMCQ.H                                                  */
/*                                                                          */
/* Description:    Generic PCM Driver, Circular Queue                       */
/*                                                                          */
/* Author:         Christopher J. Chapman                                   */
/*                                                                          */
/* Copyright (c) 2000 Conexant Systems, Inc.                                */
/* All Rights Reserved.                                                     */
/*                                                                          */
/****************************************************************************/
/* $Header: pcmcq.h, 3, 4/28/03 5:55:52 PM, $
 ****************************************************************************/


typedef signed long  PCM20;   /* 20 bit signed PCM data (in a 32 bit word) */
typedef signed long  PCM32;   /* 32 bit signed PCM data (in a 32 bit word) */

typedef struct
{
   PCM20 *Begin;
   PCM20 *Enqueue;
   PCM20 *Dequeue;
   PCM20 *End;                /* actual last data sample, NOT last + 1 */
} PCM20_CQ; /* Circular queue for 16 bit PCM data */

bool     PCM20_CQ_CreateQ(PCM20_CQ *Q, u_int32 QElements);
void     PCM20_CQ_ClearQ(PCM20_CQ *Q);
void     PCM20_CQ_FlushQ(PCM20_CQ *Q);
PCM20   *PCM20_CQ_NextQElement(PCM20_CQ *Q, PCM20 *Pointer);
u_int32  PCM20_CQ_UsedElements(PCM20_CQ *Q);
u_int32  PCM20_CQ_FreeElements(PCM20_CQ *Q);
bool     PCM20_CQ_FullQ(PCM20_CQ *Q);
bool     PCM20_CQ_EmptyQ(PCM20_CQ *Q);
u_int32  PCM20_CQ_EnqueueBuffer(PCM20_CQ *Q,
                                PCM20    *SamplesToEnqueue,
                                u_int32   NumberOfSamplesToEnqueue);
u_int32  PCM20_CQ_FastEnqueueBuffer(PCM20_CQ *Q,
                                    PCM20    *SamplesToEnqueue,
                                    u_int32   NumberOfSamplesToEnqueue);
u_int32  PCM20_CQ_DequeueBuffer(PCM20_CQ *Q,
                                PCM20    *DequeuedSamples,
                                u_int32   NumberOfSamplesToDequeue);
u_int32  PCM20_CQ_FastDequeueBuffer(PCM20_CQ *Q,
                                    PCM20    *DequeuedSamples,
                                    u_int32   NumberOfSamplesToDequeue);
u_int32  PCM20_CQ_EnqueueElement(PCM20_CQ *Q, PCM20 SampleToEnqueue);
u_int32  PCM20_CQ_DequeueElement(PCM20_CQ *Q, PCM20 *DequeuedSample);
u_int32  PCM20_CQ_ConvertAndEnqueueBuffer(PCM20_CQ *Q,
                                          u_int8    *InputBuffer,
                                          u_int32   SizeOfBuffer,
                                          u_int32   BytesPerSample,
                                          bool      bSigned);

/* For the Fast En/DeQ routines */
extern void FCopy(char *pDest, char *pSrc, u_int32 len);

/****************************************************************************
 * $Log: 
 *  3    mpeg      1.2         4/28/03 5:55:52 PM     Senthil Veluswamy SCR(s) 
 *        6114 :
 *        Added prototype for FCopy
 *        
 *  2    mpeg      1.1         4/28/03 4:02:26 PM     Senthil Veluswamy SCR(s) 
 *        6114 :
 *        Added prototypes for Fast En/DeQ routines
 *        
 *  1    mpeg      1.0         5/15/02 3:28:10 AM     Steve Glennon   
 * $
 * 
 *    Rev 1.2   28 Apr 2003 16:55:52   velusws
 * SCR(s) 6114 :
 * Added prototype for FCopy
 * 
 *    Rev 1.1   28 Apr 2003 15:02:26   velusws
 * SCR(s) 6114 :
 * Added prototypes for Fast En/DeQ routines
 * 
 *    Rev 1.0   15 May 2002 02:28:10   glennon
 * SCR(s): 2438 
 * Circular Queue handling functions for generic PCM driver (header)
 * 
 *
 ****************************************************************************/
