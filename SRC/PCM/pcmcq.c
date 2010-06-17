/******************************************************************************/
/*                           Conexant Systems, Inc.                           */
/******************************************************************************/
/*                                                                            */
/* Filename:       PCMCQ.C                                                    */
/*                                                                            */
/* Description:    Generic PCM Driver, Circular Queue                         */
/*                                                                            */
/* Author:         Christopher J. Chapman/ Steve G Glennon                    */
/*                                                                            */
/* Copyright (c) 2000 Conexant Systems, Inc.                                  */
/* All Rights Reserved.                                                       */
/*                                                                            */
/******************************************************************************/
/* $Header: pcmcq.c, 5, 5/5/03 6:52:38 PM, $
 ******************************************************************************/
 
#include "basetype.h"
#include "pcmcq.h"
#include "kal.h"
#include <string.h>

/*******************************************************************************
   PCM20_CQ_CreateQ()

   Description:
      Creates a circular queue      

   Parameters
      Q -- pointer to a queue structure to be initialized
      QElements -- the number of elements to keep in the queue

   Return Value
      TRUE if queue creation succeeded
      FALSE if queue creation failed (e.g. due to memory allocation problems)

   Notes
      None

*******************************************************************************/

bool PCM20_CQ_CreateQ(PCM20_CQ *Q, u_int32 QElements)
{
   bool  Succeeded;
   Q->Begin = (PCM20 *)mem_malloc(sizeof(PCM20) * (QElements+1));
   if (Q->Begin != NULL)
   {
      Q->End = Q->Begin + QElements;
      Q->Enqueue = Q->Begin;
      Q->Dequeue = Q->Begin;
      Succeeded = TRUE;
   }
   else
   {
      Succeeded = FALSE;
   }

   return Succeeded;
}

/*******************************************************************************
   PCM20_CQ_ClearQ()

   Description:
      Clears a circular queue      

   Parameters
      Q -- pointer to a queue structure to be cleared

   Return Value
      None

   Notes
      Resets the indices, thus clearing out the queue.
      Only safe if you know the provider and the consumer could not
      possibly be touching the queue.

*******************************************************************************/

void PCM20_CQ_ClearQ(PCM20_CQ *Q)
{
   Q->Enqueue = Q->Begin;
   Q->Dequeue = Q->Begin;
}
/*******************************************************************************
   PCM20_CQ_FlushQ()

   Description:
      Flushes a circular queue. Difference from clearing is that it only 
      touches the Dequeue setting, hence being safe for the consumer to
      empty the queue without knowledge of the state of the provider.

   Parameters
      Q -- pointer to a queue structure to be flushed

   Return Value
      None

   Notes
      Safe to call when you are the consumer and don't know the state of the
      provider.

*******************************************************************************/

void PCM20_CQ_FlushQ(PCM20_CQ *Q)
{
   PCM20 DummySample;
   while (!PCM20_CQ_EmptyQ(Q))
   {
      PCM20_CQ_DequeueElement(Q, &DummySample);
   } /* endwhile */
   return;
   
} /* PCM20_CQ_FlushQ */

/*******************************************************************************
   PCM20_CQ_NextQElement()

   Description:
      Returns a pointer to the next element in a circular queue      

   Parameters
      Q -- pointer to a queue structure
      Pointer -- pointer to an element in Q

   Return Value
      A pointer to the next element in Q after Pointer

   Notes
      None

*******************************************************************************/

PCM20 *PCM20_CQ_NextQElement(PCM20_CQ *Q, PCM20 *Pointer)
{
   PCM20 *NextQElement;

   NextQElement = Pointer + 1;

   if (NextQElement > Q->End)
   {
      NextQElement = Q->Begin;
   }

   return NextQElement;
}

/*******************************************************************************
   PCM20_CQ_UsedElements()

   Description:
      Returns the number of elements present enqueued in the queue.

   Parameters
      Q -- pointer to a queue structure 

   Return Value
      The number of samples stored in the queue.

   Notes
      None

*******************************************************************************/

u_int32 PCM20_CQ_UsedElements(PCM20_CQ *Q)
{
   u_int32   EnqueuedElements;
   if (Q->Enqueue >= Q->Dequeue)
   {
      EnqueuedElements = (Q->Enqueue - Q->Dequeue);
   }
   else
   {
      EnqueuedElements = ((Q->End - Q->Dequeue) + (Q->Enqueue - Q->Begin));
   }

   return EnqueuedElements;
}

/*******************************************************************************
   PCM20_CQ_FreeElements()

   Description:
      Returns the number of unused elements in the queue.

   Parameters
      Q -- pointer to a queue structure 

   Return Value
      The number of free entries in the queue.

   Notes
      None

*******************************************************************************/

u_int32 PCM20_CQ_FreeElements(PCM20_CQ *Q)
{
   u_int32   FreeElements;
   if (Q->Enqueue >= Q->Dequeue)
   {
      FreeElements = ((Q->Dequeue - Q->Begin) + (Q->End - Q->Enqueue));
   }
   else
   {
      FreeElements = (Q->Dequeue - Q->Enqueue);
   }

   return FreeElements;
}

/*******************************************************************************

   PCM20_CQ_FullQ()

   Description:
      Determines if the queue is full.

   Parameters
      Q -- pointer to a queue structure 

   Return Value
      TRUE if the queue is full
      FALSE otherwise

   Notes
      None

*******************************************************************************/

bool PCM20_CQ_FullQ(PCM20_CQ *Q)
{
   PCM20 *NextQElement;
   bool  Q_Full;

   NextQElement = PCM20_CQ_NextQElement(Q, Q->Enqueue);
   if (NextQElement == Q->Dequeue)
   {
      Q_Full = TRUE;
   }
   else
   {
      Q_Full = FALSE;
   }

   return Q_Full;
}

/*******************************************************************************

   PCM20_CQ_EmptyQ()

   Description:
      Determines if the queue is empty.

   Parameters
      Q -- pointer to a queue structure 

   Return Value
      TRUE if the queue is empty
      FALSE otherwise

   Notes
      None

*******************************************************************************/

bool PCM20_CQ_EmptyQ(PCM20_CQ *Q)
{
   bool  Q_Empty;

   if (Q->Enqueue == Q->Dequeue)
   {
      Q_Empty = TRUE;
   }
   else
   {
      Q_Empty = FALSE;
   }

   return Q_Empty;
}

/*******************************************************************************

   PCM20_CQ_EnqueueBuffer()

   Description:
      Enqueues the elements of a buffer into the queue.

   Parameters
      Q -- pointer to a queue structure
      SamplesToEnqueue -- pointer to the buffer to enqueue
      NumberOfSamplesToEnqueue -- number of samples from the buffer to enqueue

   Return Value
      The number of samples successfully enqueued.

   Notes
      None

*******************************************************************************/

u_int32 PCM20_CQ_EnqueueBuffer(PCM20_CQ *Q,
                              PCM20    *SamplesToEnqueue,
                              u_int32   NumberOfSamplesToEnqueue)
{
   u_int32   SamplesLeft;
   PCM20    *pNextElement;

   SamplesLeft = NumberOfSamplesToEnqueue;

   while (SamplesLeft)
   {
      *Q->Enqueue = *SamplesToEnqueue++;
      SamplesLeft--;

      /* increment the enqueue pointer & wrap if necessary */
      pNextElement = Q->Enqueue + 1;
      if (pNextElement > Q->End)
      {
         pNextElement = Q->Begin;
      }

      /* make sure not to overflow the queue */
      if (pNextElement == Q->Dequeue)
      {
         /* went too far, queue is full, back up a step */
         pNextElement--;
         if (pNextElement < Q->Begin)
         {
            pNextElement = Q->End;
         }

         Q->Enqueue = pNextElement;
         break;                  /* get out of while loop, since the queue is full */
      }
      else
      {
         Q->Enqueue = pNextElement;
      }
   }

   return(NumberOfSamplesToEnqueue - SamplesLeft);
}

/*******************************************************************************

   PCM20_CQ_FastEnqueueBuffer()

   Description:
      Enqueues the elements of a buffer into the queue.

   Parameters
      Q -- pointer to a queue structure
      SamplesToEnqueue -- pointer to the buffer to enqueue
      NumberOfSamplesToEnqueue -- number of samples from the buffer to enqueue

   Return Value
      The number of samples successfully enqueued.

   Notes
      None

*******************************************************************************/

u_int32 PCM20_CQ_FastEnqueueBuffer(PCM20_CQ *Q,
                                   PCM20    *SamplesToEnqueue,
                                   u_int32   NumberOfSamplesToEnqueue)
{
   u_int32   SamplesLeft;
   u_int32   SpaceLeft;
   u_int32   SamplesEnqueued = 0;
   u_int32   SamplesWork;

   /* Get the free space in the queue. We cannot fill up to the Dequeue 
      location. So minus 1 (to prevent Q Empty) */
   SpaceLeft = PCM20_CQ_FreeElements(Q);
   if(SpaceLeft)
   {
      /* Start Filling up free space from the Enqueue Pointer, upto the end of 
         the Q */

      /* How many samples can we enqueue ? */
      SamplesLeft = min(NumberOfSamplesToEnqueue, SpaceLeft);

      /* Space left after the EnQ Ptr */
      SpaceLeft = (Q->End + 1 - Q->Enqueue);
      /* How many samples do we have ? */
      SamplesWork = min(SpaceLeft, SamplesLeft);

      /* Store Staring from the EnQ Ptr, upto the end of the Q */
      FCopy((char *)Q->Enqueue, (char *)SamplesToEnqueue, SamplesWork * sizeof(PCM20));
      SamplesEnqueued += SamplesWork;
      SamplesLeft -= SamplesWork;
      Q->Enqueue += SamplesWork;
      SamplesToEnqueue += SamplesWork;

      /* How much more free space do we have ? */
      SpaceLeft = (Q->Dequeue - 1 - Q->Begin);
      /* Do we have any samples left ? */
      SamplesWork = min(SpaceLeft, SamplesLeft);
      if(SamplesWork)
      {
         /* Store the remaining samples */
         FCopy((char *)Q->Begin, (char *)SamplesToEnqueue, SamplesWork * sizeof(PCM20));
         SamplesEnqueued += SamplesWork;
         //SamplesLeft -= SamplesWork;
         Q->Enqueue = Q->Begin + SamplesWork;
         //SamplesToEnqueue += SamplesWork;
      }
   } /* endif */
   
   /* Adjust Pointers if necessary */
   if(Q->Enqueue > Q->End)
   {
      Q->Enqueue = Q->Begin;
   }

   return SamplesEnqueued;
   
} /* PCM20_CQ_FastEnqueueBuffer */

/*******************************************************************************

   PCM20_CQ_ConvertAndEnqueueBuffer()

   Description:
      Converts a raw buffer of possibly non-20-bit, possibly unsigned, samples
      to 20 bit signed samples and enqueues the converted samples into the
      queue.

   Parameters
      Q -- pointer to a queue structure
      InputBuffer -- pointer to the buffer
      SizeOfBuffer -- size of the buffer (in bytes)

   Return Value
      The number of samples successfully enqueued.

   Notes
      None

*******************************************************************************/

u_int32 PCM20_CQ_ConvertAndEnqueueBuffer(PCM20_CQ *Q,
                                        u_int8    *InputBuffer,
                                        u_int32   SizeOfBuffer,
                                        u_int32   BytesPerSample,
                                        bool     bSigned)
{
   u_int32   SamplesEnqueued;
   u_int8    *pCurrentSample;
   u_int8    *pLastSample;
   PCM20    CurrentSample;
   PCM20    *pNextElement;

   pCurrentSample = InputBuffer;
   pLastSample = InputBuffer + SizeOfBuffer;
   SamplesEnqueued = 0;

   while (pCurrentSample < pLastSample)
   {
      CurrentSample = 0;
      /* read in and convert to 20 bits */
      switch (BytesPerSample)
      {
      case 1:     /* 8 bits to 20 bits */
         CurrentSample = ((*pCurrentSample) << 12);
         break;
      case 2:     /* 16 bits to 20 bits */
         CurrentSample =
            (
               ((*pCurrentSample) + (*(pCurrentSample + 1) << 8)) <<
               4
            );
         break;
      case 3:     /* 24 bits to 20 bits */
         CurrentSample =
            (
               (
                  (*pCurrentSample) + (*(pCurrentSample + 1) << 8) +
                     (*(pCurrentSample + 2) << 16)
               ) >>
               4
            );
         break;
      default:
         /* this should never happen */
         trace_new(TRACE_LEVEL_2 | TRACE_AUD,
                            "PCM -- Error -- Unhandled PCM format!\n");
         break;
      }

      pCurrentSample += BytesPerSample;

      if (!bSigned)
      {
         CurrentSample = CurrentSample - (PCM20) 0x80000;
      } /* endif not signed input samples */

      /* sign extend to 32 bits for math later on */
      if (CurrentSample & 0x80000)
      {
         CurrentSample |= 0xFFF00000;
      }

      *Q->Enqueue = CurrentSample;
      SamplesEnqueued++;

      /* increment the enqueue pointer & wrap if necessary */
      pNextElement = Q->Enqueue + 1;
      if (pNextElement > Q->End)
      {
         pNextElement = Q->Begin;
      }

      /* make sure not to overflow the queue */
      if (pNextElement == Q->Dequeue)
      {
         /* went too far, queue is full, back up a step */
         SamplesEnqueued--;
         pNextElement--;
         if (pNextElement < Q->Begin)
         {
            pNextElement = Q->End;
         }

         Q->Enqueue = pNextElement;
         break;   /* get out of while loop, since the queue is full */
      }
      else
      {
         Q->Enqueue = pNextElement;
      }
   }

   return(SamplesEnqueued);
}

/*******************************************************************************

   PCM20_CQ_DequeueBuffer()

   Description
      Enqueues the elements of a buffer into the queue.

   Parameters
      Q -- pointer to a queue structure
      DequeuedSamples -- point to buffer in which to store dequeued samples
      NumberOfSamplesToDequeue -- the number of samples to dequeue

   Return Value
      The number of samples successfully dequeued [see note below].

   Notes
      This routine zero-fills the remainder of the buffer if there are not
      enough enqueued samples to fill the number of samples requested.

*******************************************************************************/

u_int32 PCM20_CQ_DequeueBuffer(PCM20_CQ *Q,
                              PCM20    *DequeuedSamples,
                              u_int32   NumberOfSamplesToDequeue)
{
   u_int32   SamplesLeft;
   u_int32   SamplesDequeued;

   SamplesLeft = NumberOfSamplesToDequeue;

   /* while there are still requested samples left to get
      and the queue is not empty */
   while (SamplesLeft && (Q->Enqueue != Q->Dequeue))
   {
      *DequeuedSamples++ = *Q->Dequeue;

      /* increment the dequeue pointer & wrap if necessary */
      Q->Dequeue++;
      if (Q->Dequeue > Q->End)
      {
         Q->Dequeue = Q->Begin;
      }

      SamplesLeft--;
   }

   SamplesDequeued = (NumberOfSamplesToDequeue - SamplesLeft);

   while (SamplesLeft)
   {
      *DequeuedSamples++ = 0;
      SamplesLeft--;
   }

   return SamplesDequeued;
}

u_int32 PCM20_CQ_FastDequeueBuffer(PCM20_CQ *Q,
                                   PCM20    *DequeuedSamples,
                                   u_int32   NumberOfSamplesToDequeue)
{
   u_int32 SamplesToDequeue;
   u_int32 SamplesWork;
   u_int32 SamplesDequeued = 0;
   u_int32 SpaceLeft;

   /* Get the #Samples in the queue */
   SamplesToDequeue = PCM20_CQ_UsedElements(Q);
   if(SamplesToDequeue)
   {
      /* Get Samples from the DeQ Ptr, upto the End of the Q */

      /* How many samples can we dequeue ? */
      SamplesToDequeue = min(SamplesToDequeue, NumberOfSamplesToDequeue);

      /* How many storage locations till the end of the Q? */
      SamplesWork = (Q->End + 1 - Q->Dequeue);
      /* How many do we want? */
      SamplesWork = min(SamplesWork, SamplesToDequeue);

      /* Start DeQing starting from the DeQ ptr */
      FCopy((char *)DequeuedSamples, (char *)Q->Dequeue, SamplesWork * sizeof(PCM20));
      SamplesDequeued += SamplesWork;
      SamplesToDequeue -= SamplesWork;
      Q->Dequeue += SamplesWork;
      DequeuedSamples += SamplesWork;

      /* How many more storage locations present on the Q? */
      SamplesWork = (Q->Enqueue - Q->Begin);
      /* Do we need any more samples? */
      SamplesWork = min(SamplesToDequeue, SamplesWork);
      if(SamplesWork)
      {
         FCopy((char *)DequeuedSamples, (char *)Q->Begin, SamplesWork * sizeof(PCM20));
         SamplesDequeued += SamplesWork;
         //SamplesToDequeue -= SamplesWork;
         Q->Dequeue = Q->Begin + SamplesWork;
         //DequeuedSamples += SamplesWork;
      }
   }

   /* How many did we copy ? */
   SpaceLeft = NumberOfSamplesToDequeue - SamplesDequeued;

   /* Zero out the remaining buffer space */
   if(SpaceLeft)
   {
      
      memset(DequeuedSamples, 0, SpaceLeft);
   }

   /* Adjust Pointers if necessary */
   if(Q->Dequeue > Q->End)
   {
      Q->Dequeue = Q->Begin;
   }

   return(SamplesDequeued);
}

/*******************************************************************************

   PCM20_CQ_EnqueueElement()

   Description
      Enqueues an elements into the queue.

   Parameters
      Q -- pointer to a queue structure
      SampleToEnqueue -- a PCM sample to enqueue

   Return Value
      The number of samples successfully enqueued (1 or 0).

   Notes
      None

*******************************************************************************/

u_int32 PCM20_CQ_EnqueueElement(PCM20_CQ *Q, PCM20 SampleToEnqueue)
{
   u_int32   SamplesEnqueued;

   if (!PCM20_CQ_FullQ(Q))
   {
      *Q->Enqueue = SampleToEnqueue;
      Q->Enqueue = PCM20_CQ_NextQElement(Q, Q->Enqueue);
      SamplesEnqueued = 1;
   }
   else
   {
      SamplesEnqueued = 0;
   }

   return SamplesEnqueued;
}

/*******************************************************************************

   PCM20_CQ_DequeueElement()

   Description
      Dequeues an elements from the queue.

   Parameters
      Q -- pointer to a queue structure
      DequeuedSample -- the dequeued PCM sample

   Return Value
      The number of samples successfully dequeued (1 or 0) [see note below].

   Notes
      This routine puts a zero in DequeuedSample if there are no enqueued
      samples available.

*******************************************************************************/
u_int32 PCM20_CQ_DequeueElement(PCM20_CQ *Q, PCM20 *DequeuedSample)
{
   u_int32   SamplesDequeued;

   if (!PCM20_CQ_EmptyQ(Q))
   {
      *DequeuedSample = *Q->Dequeue;
      Q->Dequeue = PCM20_CQ_NextQElement(Q, Q->Dequeue);
      SamplesDequeued = 1;
   }
   else
   {
      *DequeuedSample = 0;
      SamplesDequeued = 0;
   }

   return SamplesDequeued;
}

/* ************************************************************************** */
/****************************************************************************
 * $Log: 
 *  5    mpeg      1.4         5/5/03 6:52:38 PM      Senthil Veluswamy SCR(s) 
 *        6114 :
 *        Increased Circular Q size by 1 to accomodate N elements (Circular Q 
 *        of N elements needs to be of size N+1 to accomodate for non 
 *        overlapping EnQ and DeQ at Q Full) and made related boundary changes.
 *        
 *  4    mpeg      1.3         4/28/03 5:53:44 PM     Senthil Veluswamy SCR(s) 
 *        6114 :
 *        Added New Fast En/DeQ interfaces, changed the definition of the Q End
 *         Ptr to point to the end of the queue. Was pointing to End+1 before.
 *        
 *  3    mpeg      1.2         8/12/02 3:20:12 PM     Lucy C Allevato SCR(s) 
 *        4368 :
 *        Changed "string.h" to <string.h>
 *        
 *  2    mpeg      1.1         6/19/02 5:54:36 PM     Steve Glennon   SCR(s): 
 *        4066 
 *        Many fixes to original code which did not work correctly at all. 
 *        Enqueued
 *        and dequeued data from different queues etc.
 *        
 *  1    mpeg      1.0         5/15/02 3:28:08 AM     Steve Glennon   
 * $
 * 
 *    Rev 1.4   05 May 2003 17:52:38   velusws
 * SCR(s) 6114 :
 * Increased Circular Q size by 1 to accomodate N elements (Circular Q of N elements needs to be of size N+1 to accomodate for non overlapping EnQ and DeQ at Q Full) and made related boundary changes.
 * 
 *    Rev 1.3   28 Apr 2003 16:53:44   velusws
 * SCR(s) 6114 :
 * Added New Fast En/DeQ interfaces, changed the definition of the Q End Ptr to point to the end of the queue. Was pointing to End+1 before.
 * 
 *    Rev 1.2   Aug 12 2002 15:20:12   allevalc
 * SCR(s) 4368 :
 * Changed "string.h" to <string.h>
 * 
 *    Rev 1.1   Jun 19 2002 17:54:36   glennon
 * SCR(s): 4066 
 * Many fixes to original code which did not work correctly at all. Enqueued
 * and dequeued data from different queues etc.
 * 
 *    Rev 1.0   15 May 2002 02:28:08   glennon
 * SCR(s): 2438 
 * Circular Queue handling functions for generic PCM driver
 * 
 *
 ****************************************************************************/

