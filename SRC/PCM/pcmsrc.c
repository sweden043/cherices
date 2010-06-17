/******************************************************************************/
/*                           Conexant Systems, Inc.                           */
/******************************************************************************/
/*                                                                            */
/* Filename:       PCMSRC.C                                                   */
/*                                                                            */
/* Description:    Generic PCM Driver, Sample Rate Conversion                 */
/*                                                                            */
/* Author:         Christopher J. Chapman / Steve G Glennon                   */
/*                                                                            */
/* Copyright (c) 2000 Conexant Systems, Inc.                                  */
/* All Rights Reserved.                                                       */
/*                                                                            */
/******************************************************************************/
/* $Header: pcmsrc.c, 17, 4/28/03 3:56:20 PM, $
 ******************************************************************************/


#include "stbcfg.h"
#include "basetype.h"
#include "kal.h"
#include "gen_aud.h"
#include "pcm.h"
#include "pcmcq.h"
#include "pcmpriv.h"

#include "globals.h"

#if (PCM_DISABLE_SRC != YES)

extern u_int32 PCM_DataTransferred;
static bool sbWaitStop = FALSE; /* To identify the PCM Stop Event */

extern PCM20_CQ PCMInputQ;      /* Holds input samples */
extern PCM20_CQ PCMSRC_Q;       /* Holds sample rate converted data */

/* #define INPUT_BUFF_SIZE 384 */
/* #define INPUT_BUFF_SIZE 1024 */
/* #define INPUT_BUFF_SIZE 2048 */
/* #define INPUT_BUFF_SIZE 4096 */
/* #define INPUT_BUFF_SIZE 512 */
/* #define INPUT_BUFF_SIZE 256 */
/* #define INPUT_BUFF_SIZE 128 */
/* #define INPUT_BUFF_SIZE 64 */
#define INPUT_BUFF_SIZE 32 

/* ************************************************************************** */
/* 

   Filter kernels based on Matlab's "FIR1(N,Wn)" command with N = 7 (for
   8 taps -- N.B.: half are given below since the other half are
   symmetrical) and Wn = x/y for filter name FIR_Wn_x_y[]. The
   coefficients have been multiplied by 2^11 and rounded to an integer
   value.
   
*/

/* ************************************************************************** */

static int MDIndex = 1;
static int SDIndex = 0;

#if USE_UNOPTIMIZED_PCM_SRC == YES
static PCM20 prev_data[TAPS];
static PCM20 prev_left_data[TAPS];
static PCM20 prev_right_data[TAPS];
static int MFADIndex = 0;
static int SFADIndex = 0;
#else
extern int MFADIndex;
extern int SFADIndex;
#endif /* USE_UNOPTIMIZED_PCM_SRC == YES */

/* ************************************************************************** */

#if USE_UNOPTIMIZED_PCM_SRC != YES
/* External Functions */
extern void MonoLinearInterpolate(PCM20 * InputBuffer, PCM20 * OutputBuffer,
                                          int InputSamples, int Interpolator);
extern void StereoLinearInterpolate(PCM20 * InputBuffer, PCM20 * OutputBuffer,
                                          int InputSamples, int Interpolator);
extern void MonoFilterAndDecimate(PCM20 * InputPtr, PCM20 * OutputPtr,
                     int InputSamples, int Decimator, int32 *PCMFilterKernel);
extern void StereoFilterAndDecimate(PCM20 * InputPtr, PCM20 * OutputPtr,
                     int InputSamples, int Decimator, int32 *PCMFilterKernel);
#endif /* USE_UNOPTIMIZED_PCM_SRC != YES */

/*******************************************************************************

   ChooseSRCParameters()

   Description:
      Selects the appropriate interpolator, decimator, and filter kernel to
      use for the given sample rate conversion.

   Parameters
      PCMSampleRate          Sample rate of input data
      Interpolator           Returned interpolation factor
      Decimator              Returned decimation factor
      PCMFilterKernel        Pointer to FIR coefficients

   Return Value
      None

   Notes
      None

*******************************************************************************/

void ChooseSRCParameters(cnxt_pcm_sample_rate_t PCMSampleRate,
                         u_int32 *pInterpolator, 
                         u_int32 *pDecimator,
                         int32  **pPCMFilterKernel)
{
   u_int8 MPEGSampleRate;
   u_int32 control1;
   /* in case everything falls through, assume no sample rate
      conversion */
   *pInterpolator = 1;
   *pDecimator = 1;
   *pPCMFilterKernel = NULL;

   /* determine target sample rate */
   control1 = *glpCtrl1Reg;
   MPEGSampleRate = (control1 & 0x00E00000) >> 21;
   
   /* check sample rates within bounds */
   if ((MPEGSampleRate <= MPG_SAMPFREQ_16000) &&
       (PCMSampleRate < SAMPLE_RATE_INVALID))
   {
      *pInterpolator    = a_cnxt_pcm_src_array[MPEGSampleRate][PCMSampleRate].Interpolator;
      *pDecimator       = a_cnxt_pcm_src_array[MPEGSampleRate][PCMSampleRate].Decimator;
      *pPCMFilterKernel = a_cnxt_pcm_src_array[MPEGSampleRate][PCMSampleRate].Filter;
   
   }
   return;

} /* pcmChooseSRCParameters */

#if USE_UNOPTIMIZED_PCM_SRC == YES

/*******************************************************************************

   MonoLinearInterpolate()

   Description:
      Performs a linear interpolation on a set of mono PCM samples.

   Parameters
      InputBuffer -- PCM samples to be interpolated
      OutputBuffer -- the interpolated PCM samples
      InputSamples -- number of samples to be interpolated
      Interpolator -- the factor by which to interpolate

   Return Value
      None

   Notes
      None

*******************************************************************************/

void MonoLinearInterpolate(PCM20 * InputBuffer, PCM20 * OutputBuffer,
   int InputSamples, int Interpolator)
{
   PCM20 Step;
   PCM20 Temp;
   int j;
   PCM20 *InputPtr, *OutputPtr;
   PCM20 *FinalOutputPtr;
   static PCM20 PrevValue = 0;
/*   time_o Time; */
   
/*   trace_new(TRACE_LEVEL_2 | TRACE_AUD, "Interpolating samples:\n"); */

   InputPtr = InputBuffer;
   OutputPtr = OutputBuffer;
   FinalOutputPtr = &OutputBuffer[((InputSamples * Interpolator) - 1)];

   while(OutputPtr < FinalOutputPtr)
   {
      Step = *InputPtr - PrevValue;
      Step /= Interpolator;
      Temp = PrevValue + Step;
      for(j=0; j < Interpolator; j++)
      {
         *OutputPtr++ = Temp;
         Temp += Step;
      }
      PrevValue = *InputPtr++;
   }
}


/*******************************************************************************

   StereoLinearInterpolate()

   Description:
      Performs a linear interpolation on a set of stereo PCM samples.

   Parameters
      InputBuffer -- PCM samples to be interpolated
      OutputBuffer -- the interpolated PCM samples
      InputSamples -- number of samples to be interpolated
      Interpolator -- the factor by which to interpolate

   Return Value
      None

   Notes
      None

*******************************************************************************/
void StereoLinearInterpolate(PCM20 * InputBuffer, PCM20 * OutputBuffer,
   int InputSamples, int Interpolator)
{
   PCM20 LeftStep;
   PCM20 RightStep;
   PCM20 LeftTemp;
   PCM20 RightTemp;
   int j;
   PCM20 *InputPtr, *OutputPtr;
   PCM20 *FinalOutputPtr;
   static PCM20 PrevLeftValue = 0;
   static PCM20 PrevRightValue = 0;

   InputPtr = InputBuffer;
   OutputPtr = OutputBuffer;
   FinalOutputPtr = &OutputBuffer[((InputSamples * Interpolator) - 1)];

   while(OutputPtr <= FinalOutputPtr)
   {
      LeftStep = *InputPtr - PrevLeftValue;
      LeftStep /= Interpolator;

      RightStep = *(InputPtr + 1) - PrevRightValue;
      RightStep /= Interpolator;

      LeftTemp = PrevLeftValue + LeftStep;
      RightTemp = PrevRightValue + RightStep;

      for(j=0; j < Interpolator; j++)
      {
         *OutputPtr++ = LeftTemp;
         LeftTemp += LeftStep;
         *OutputPtr++ = RightTemp;
         RightTemp += RightStep;
      }
      PrevLeftValue = *InputPtr++;
      PrevRightValue = *InputPtr++;
   }
}

/*******************************************************************************

   MonoFilterAndDecimate()

   Description:
      Performs an FIR filter and decimation on a set of mono PCM samples.

   Parameters
      InputPtr -- pointer to PCM samples to be filtered & decimated
      OutputPtr -- pointer to a buffer for the filtered & decimated PCM samples
      InputSamples -- number of samples to be filtered & decimated
      Decimator -- the factor by which to decimate

   Return Value
      None

   Notes
      None

*******************************************************************************/

void MonoFilterAndDecimate( PCM20 * InputPtr, 
                            PCM20 * OutputPtr,
                            int InputSamples, 
                            int Decimator,
                            int32 *PCMFilterKernel)
{
   int i;
   int j, k;
   PCM20 Result;

   while(InputSamples)
   {
      i = Decimator;
      while((i > 0) && (InputSamples > 0))
      {
         prev_data[MFADIndex++] = *InputPtr++;
         if(MFADIndex >= TAPS)
         {
            MFADIndex = 0;
         }
         i--;
         InputSamples--;
      };

      /* if we ran out of input samples before collecting enough points for
         decimation, break out of the loop and finish this when we have more
         data */
      if((InputSamples == 0) && (i != 0))
         break; 

      Result = 0;
      
      i = 0;
      j = MFADIndex - 1; /* index is one ahead of last data entered */
      k = MFADIndex;
      do
      {
         if (j < 0)
         {
            j += TAPS;
         }
         if (k >= TAPS)
         {
            k -= TAPS;
         }
         Result += ((prev_data[j] + prev_data[k]) * PCMFilterKernel[i]) >> 11;
         j--;
         k++;
         i++;
      } while(i < HALF_TAPS);

      if(Result > 524287)
      {
         Result = 524287;
      }
      else if (Result < -524288)
      {
         Result = -524288;
      }
      *OutputPtr++ = Result;
   }
}

#endif /* USE_UNOPTIMIZED_PCM_SRC == YES */

/*******************************************************************************

   MonoDecimate()

   Description:
      Performs a decimation on a set of mono PCM samples.

   Parameters
      InputPtr -- pointer to PCM samples to be decimated
      OutputPtr -- pointer to a buffer for the decimated PCM samples
      InputSamples -- number of samples to be decimated
      Decimator -- the factor by which to decimate

   Return Value
      None

   Notes
      This routine is intended to be used only on samples with source and target
      Nyquist frequencies above normal hearing range (i.e. >= 16kHz).

      This routine may also be used for lower frequency source and target
      sample rates when CPU utilization is critical and the high frequency
      noise resulting from interpolation and decimation is deemed acceptable.
      
*******************************************************************************/

void MonoDecimate(PCM20 * InputPtr, PCM20 * OutputPtr,
   int InputSamples, int Decimator)
{


   while(InputSamples)
   {
      if (MDIndex == Decimator)
      {
         MDIndex = 1;
         *OutputPtr++ = *InputPtr++;
      } else
      {
         MDIndex++;
         InputPtr++;
      }
      InputSamples--;
   }
}


#if USE_UNOPTIMIZED_PCM_SRC == YES

/*******************************************************************************

   StereoFilterAndDecimate()

   Description:
      Performs an FIR filter and decimation on a set of stereo PCM samples.

   Parameters
      InputPtr -- pointer to PCM samples to be filtered & decimated
      OutputPtr -- pointer to a buffer for the filtered & decimated PCM samples
      InputSamples -- number of samples to be filtered & decimated
      Decimator -- the factor by which to decimate

   Return Value
      None

   Notes
      None

*******************************************************************************/

void StereoFilterAndDecimate( PCM20 * InputPtr, 
                              PCM20 * OutputPtr,
                              int InputSamples, 
                              int Decimator,
                              int32 *PCMFilterKernel)
{
   int i;
   int j, k;
   PCM20 LeftResult;
   PCM20 RightResult;

   while(InputSamples)
   {
      i = Decimator;
      while((i > 0) && (InputSamples > 0))
      {
         prev_left_data[SFADIndex] = *InputPtr++;
         prev_right_data[SFADIndex] = *InputPtr++;
         InputSamples-= 2;
         i--;

         SFADIndex++;
         if(SFADIndex >= TAPS)
         {
            SFADIndex = 0;
         }
      };

      /* if we ran out of input samples before collecting enough points for
         decimation, break out of the loop and finish this when we have more
         data */
      if((InputSamples == 0) && (i != 0))
         break; 

      LeftResult = 0;
      RightResult = 0;
      
      i = 0;
      j = SFADIndex - 1; /* index is one ahead of last data entered */
      k = SFADIndex;
      do
      {
         if (j < 0)
         {
            j += TAPS;
         }
         if (k >= TAPS)
         {
            k -= TAPS;
         }
         LeftResult += ((prev_left_data[j] + prev_left_data[k])
            * PCMFilterKernel[i]) >> 11;
         RightResult += ((prev_right_data[j] + prev_right_data[k])
            * PCMFilterKernel[i]) >> 11;
         j--;
         k++;
         i++;
      } while(i < HALF_TAPS);

      if(RightResult > 524287)
      {
         RightResult = 524287;
      }
      else if (RightResult < -524288)
      {
         RightResult = -524288;
      }

      if(LeftResult > 524287)
      {
         LeftResult = 524287;
      }
      else if (LeftResult < -524288)
      {
         LeftResult = -524288;
      }
      *OutputPtr++ = LeftResult;
      *OutputPtr++ = RightResult;
   }
}

#endif /* USE_UNOPTIMIZED_PCM_SRC == YES */



/*******************************************************************************

   StereoDecimate()

   Description:
      Performs a decimation on a set of stereo PCM samples.

   Parameters
      InputPtr -- pointer to PCM samples to be decimated
      OutputPtr -- pointer to a buffer for the decimated PCM samples
      InputSamples -- number of samples to be decimated
      Decimator -- the factor by which to decimate

   Return Value
      None

   Notes
      This routine is intended to be used only on samples with source and target
      Nyquist frequencies above normal hearing range (i.e. >= 16kHz).

      This routine may also be used for lower frequency source and target
      sample rates when CPU utilization is critical and the high frequency
      noise resulting from interpolation and decimation is deemed acceptable.

*******************************************************************************/

void StereoDecimate(PCM20 * InputPtr, PCM20 * OutputPtr,
   int InputSamples, int Decimator)
{

   while(InputSamples)
   {
      if (SDIndex == Decimator)
      {
         SDIndex = 1;
         *OutputPtr++ = *InputPtr++;
         *OutputPtr++ = *InputPtr++;
      } else
      {
         SDIndex++;
         InputPtr += 2;
      }
      InputSamples -= 2;
   }
}





/*******************************************************************************

   SampleRateConvert()

   Description:
      Performs the sample rate conversion

   Parameters
      Interpolator    Interpolation factor
      Decimator       Decimation factor
      InputSamples    Number of input samples
      InputBuffer     Pointer to input samples
      OutputBuffer    Pointer to output samples
      bStereo         TRUE if stereo samples
      PCMFilterKernel Pointer to FIR coefficients

   Return Value
      The number of output samples (samples resulting from SRC)

   Notes
      None

*******************************************************************************/

u_int32 SampleRateConvert(u_int32 Interpolator, 
                         u_int32 Decimator,
                         u_int32 InputSamples, 
                         PCM20 * InputBuffer, 
                         PCM20 * OutputBuffer,
                         bool bStereo,
                         int32 *PCMFilterKernel)
{
   u_int32 IntermediateSamples;
   u_int32 OutputSamples;
   static PCM20 IntermediateBuffer[INPUT_BUFF_SIZE * 26]; /* worst case size */

   IntermediateSamples = InputSamples * Interpolator;
   OutputSamples = IntermediateSamples / Decimator;

   /* actual sample rate conversion */
   if (bStereo)
   {
      /* round OutputSamples down to an even number (due to sample pairs) */
      OutputSamples = (OutputSamples & 0xFFFFFFFE);
      StereoLinearInterpolate(InputBuffer, 
                              IntermediateBuffer, 
                              InputSamples,
                              Interpolator);
      if(PCMFilterKernel)
      {
         /* handle case of filter needed */
         StereoFilterAndDecimate(IntermediateBuffer, 
                                 OutputBuffer, 
                                 IntermediateSamples, 
                                 Decimator,
                                 PCMFilterKernel);
      } else
      {
         /* handle case of filter not needed */
         StereoDecimate(IntermediateBuffer, OutputBuffer, 
            IntermediateSamples, Decimator);
      }
   } 
   else /* Mono */
   {
      MonoLinearInterpolate(InputBuffer, IntermediateBuffer, InputSamples,
         Interpolator);
      if(PCMFilterKernel)
      {
         /* handle case of filter needed */
         MonoFilterAndDecimate(IntermediateBuffer, OutputBuffer,
            IntermediateSamples, Decimator, PCMFilterKernel);
      } else
      {
         /* handle case of filter not needed */
         MonoDecimate(IntermediateBuffer, OutputBuffer,
            IntermediateSamples, Decimator);
      }
   } /* endif stereo */

   return OutputSamples;
}

#endif /* (PCM_DISABLE_SRC != YES) */

/*******************************************************************************

   RequestRawData()

   Description:
      Request more PCM data for sample rate conversion

   Parameters
      None

   Return Value
      None

   Notes
      None

*******************************************************************************/

void RequestRawDataIfNeeded(void)
{
   u_int32 uiRequestThreshold;
   
   /* Work out the request threshold */
   uiRequestThreshold = s_cnxt_pcm_status.ui32_request_threshold;
   /* If stereo, need twice the number of PCM20's of space */
   if (s_cnxt_pcm_status.s_format.b_stereo)
   {
      uiRequestThreshold <<= 1;
   } 

   /*
    * if there's enough queue space free
    * and there is a routine with which to request data
    */      
   
   if((!s_cnxt_pcm_status.pfn_data_request) && 
      (s_cnxt_pcm_status.pfn_event_callback == NULL))
   {
      /* if there is no routine with which to request data */
      task_time_sleep(1000); /* sleep until there is one */
   } 
   else if (PCM20_CQ_FreeElements(&PCMInputQ) >= uiRequestThreshold)
   {
      /* if there is room for more data, request it */
      if(s_cnxt_pcm_status.pfn_event_callback == NULL)
      {
         s_cnxt_pcm_status.pfn_data_request();
      }
      else
      {
         s_cnxt_pcm_status.pfn_event_callback(EV_PCM_FEED, 
                                             NULL, 
                                             s_cnxt_pcm_status.p_event_info);
      }
   }
}

#if (PCM_DISABLE_SRC != YES)

/*******************************************************************************

   StoreConvertedData()

   Description:
      Enqueue PCM data that has been sample rate converted in the SRC to Mix
      queue.

   Parameters
      None

   Return Value
      None

   Notes
      None

*******************************************************************************/

void StoreConvertedData(u_int32 SamplesToEnqueue, PCM20 * BufferToEnqueue)
{
   u_int32 SamplesEnqueued;
   u_int32 Timeout = 0;

   /* wait for free queue space */
   while((PCM20_CQ_FreeElements(&PCMSRCQ) < SamplesToEnqueue) &&
         (Timeout < MAX_SRC_TIMEOUT))
   {
   
/*
      trace_new(TRACE_LEVEL_2 | TRACE_AUD,
         "SRC -- waiting for free space in PCMSRCQ.\n");
*/
      task_time_sleep(SRC_SEM_WAIT_TIME); 
      Timeout += SRC_SEM_WAIT_TIME;
   }

   /* copy SRC'd data to mixer input queue */
   SamplesEnqueued = 
#if USE_UNOPTIMIZED_PCM_SRC == YES
                        PCM20_CQ_EnqueueBuffer(
#else /* !USE_UNOPTIMIZED_PCM_SRC */
                     PCM20_CQ_FastEnqueueBuffer(
#endif /* USE_UNOPTIMIZED_PCM_SRC */
                                                &PCMSRCQ, BufferToEnqueue,
                                                SamplesToEnqueue);

   if(SamplesEnqueued < SamplesToEnqueue)
   {
      trace_new(TRACE_LEVEL_2 | TRACE_AUD,
         "SRC -- enqueued %d samples, intended to enqueue %d samples!\n",
         SamplesEnqueued, SamplesToEnqueue);

   }
}



/*******************************************************************************

   cnxt_src_task()

   Description:
      The main sample rate conversion task -- pulls data from the "clip to SRC"
      PCM data queue, sample rate converts it, and then pushes the data into the
      "SRC to mix" queue.

   Parameters
      None

   Return Value
      None

   Notes
      None

*******************************************************************************/
void cnxt_src_task(void)
{
   u_int32 InputSamples;

   static PCM20 InputBuffer[INPUT_BUFF_SIZE];
   static PCM20 OutputBuffer[INPUT_BUFF_SIZE * 6]; /* worst case size */

   PCM20 *BufferToEnqueue;
   u_int32 SamplesToEnqueue;

   u_int32 Interpolator, OldInterpolator;
   u_int32 Decimator, OldDecimator;
   int32 *PCMFilterKernel;

   OldInterpolator = 0;
   OldDecimator    = 0;
   
   trace_new(TRACE_LEVEL_2 | TRACE_AUD, "Inside PCM SRC task.\n");

   while(1)
   {
      /* If the state is stopping, clear out the input queue */
      if ((s_cnxt_pcm_status.e_state == STOPPING) ||
          (s_cnxt_pcm_status.e_state == IDLE))
      {
         sbWaitStop = FALSE;
         PCM20_CQ_ClearQ(&PCMInputQ);
         eSRCTaskState = IDLE;
         /* wait on the SRC semaphore with a timeout (timeout will occur) */
         sem_get(cnxt_src_sem_id, (4 * (SRC_SEM_WAIT_TIME)));
      } 
      
      /* If there is enough data to process
         and if the current clip is being started (PLAY)
            OR
         if the clip is supposed to be playing (PLAYING), but there is
         some data left in the queue to process
         */
      else if (((PCM20_CQ_UsedElements(&PCMInputQ) >= INPUT_BUFF_SIZE)
         && (s_cnxt_pcm_status.e_state == PLAY))
         ||
         ((s_cnxt_pcm_status.e_state == PLAYING) &&
         (PCM20_CQ_UsedElements(&PCMInputQ) > 0)))
      {
         eSRCTaskState = PLAYING;
         sbWaitStop = TRUE;
         /* copy data from G729 output queue */
         InputSamples = 
#if USE_UNOPTIMIZED_PCM_SRC == YES
                        PCM20_CQ_DequeueBuffer(
#else /* !USE_UNOPTIMIZED_PCM_SRC */
                     PCM20_CQ_FastDequeueBuffer(
#endif /* USE_UNOPTIMIZED_PCM_SRC */
                                                &PCMInputQ, InputBuffer,
                                                INPUT_BUFF_SIZE);
/*
         if(InputSamples != INPUT_BUFF_SIZE)
         {
            trace_new(TRACE_LEVEL_2 | TRACE_AUD,
               "SRC -- requested %d samples, received %d samples.\n",
               INPUT_BUFF_SIZE, InputSamples);
         }
*/
         /* attenuate data */
         AttenuateSamples(InputBuffer, 
                          INPUT_BUFF_SIZE,
                          s_cnxt_pcm_status.s_format.b_stereo,
                          s_cnxt_pcm_status.ui8_pcm_vol_left,
                          s_cnxt_pcm_status.ui8_pcm_vol_right );

         /* SRC data */
   
         /* Choose Sample Rate Convert parameters based on PCM sample rate */
         ChooseSRCParameters(s_cnxt_pcm_status.s_format.e_rate, 
                             &Interpolator,
                             &Decimator,
                             &PCMFilterKernel);
         
         if ((Interpolator != OldInterpolator) ||
             (Decimator != OldDecimator)) 
         {
            OldInterpolator = Interpolator;
            OldDecimator = Decimator;
            MFADIndex = 0;
            MDIndex = 1;
            SFADIndex = 0;
            SDIndex = 1;
         } /* endif */
         
         if ((Interpolator == 1) && (Decimator == 1))
         {
            /* handle case of no sample rate conversion needed */
            SamplesToEnqueue = InputSamples;
            BufferToEnqueue = InputBuffer;
         } 
         else
         {
            /* handle case of sample rate conversion needed */
            SamplesToEnqueue = SampleRateConvert(Interpolator, 
                                                 Decimator, 
                                                 InputSamples,
                                                 InputBuffer, 
                                                 OutputBuffer,
                                                 s_cnxt_pcm_status.s_format.b_stereo,
                                                 PCMFilterKernel);
            BufferToEnqueue = OutputBuffer;
         }

         /* if in PLAY state, transition to "PLAYING" */
         if (s_cnxt_pcm_status.e_state == PLAY)
         {
            bool kstate;
            kstate = critical_section_begin();  // Turn off interrupts
            s_cnxt_pcm_status.e_state = PLAYING;
            critical_section_end(kstate);       // Turn on interrupts if they were on
 
         } /* endif in PLAY state */

         StoreConvertedData(SamplesToEnqueue, BufferToEnqueue);
      } 
      else 
      {
         /* wait on the SRC semaphore with a timeout */
         sem_get(cnxt_src_sem_id, SRC_SEM_WAIT_TIME);

        /* if a Callback is requested... */
        if((s_cnxt_pcm_status.pfn_event_callback != NULL) && (sbWaitStop))
        {
            s_cnxt_pcm_status.pfn_event_callback(EV_PCM_EMPTY, 
                                                 NULL, 
                                                 s_cnxt_pcm_status.p_event_info);
        }
      } /* endif able to SRC samples */

      /* Decide whether we are in a state where we should request data */
      if ((s_cnxt_pcm_status.e_state == PLAY)    ||
          (s_cnxt_pcm_status.e_state == PLAYING))
      {
         RequestRawDataIfNeeded();
         
      } /* endif PLAY or PLAYING */
      
   } /* endwhile(1) */
   
   /* should never get here */
   return;
   
}  /* cnxt_src_task */

#endif /* (PCM_DISABLE_SRC != YES) */

/****************************************************************************
 * $Log: 
 *  17   mpeg      1.16        4/28/03 3:56:20 PM     Senthil Veluswamy SCR(s) 
 *        6114 :
 *        Modified to use the Fast En/DeQ routines when SRC Optimization is 
 *        turned on.
 *        
 *  16   mpeg      1.15        4/21/03 8:34:08 PM     Senthil Veluswamy SCR(s) 
 *        6066 6065 :
 *        Changed PCM_DISABLE_SRC to take values YES and NO instead of just 
 *        being defined.
 *        
 *  15   mpeg      1.14        4/21/03 8:05:46 PM     Senthil Veluswamy SCR(s) 
 *        6066 6065 :
 *        Wrapped code to compile time switch out the SRC code if 
 *        PCM_DISABLE_SRC is set to YES
 *        
 *  14   mpeg      1.13        2/13/03 12:25:02 PM    Matt Korte      SCR(s) 
 *        5479 :
 *        Removed old header reference
 *        
 *  13   mpeg      1.12        1/14/03 10:34:22 AM    Senthil Veluswamy SCR(s) 
 *        5230 :
 *        Fixed bug. Was erroneously using NULL fn ptr.
 *        
 *  12   mpeg      1.11        1/10/03 5:39:28 PM     Senthil Veluswamy SCR(s) 
 *        5230 :
 *        Added support for General Event Notification & Data Feed/Stop Events.
 *        
 *  11   mpeg      1.10        12/3/02 2:51:08 PM     Senthil Veluswamy SCR(s) 
 *        5058 :
 *        Fixed warnings from extern vars
 *        
 *  10   mpeg      1.9         11/27/02 12:50:12 PM   Senthil Veluswamy SCR(s) 
 *        5001 :
 *        Replaced check for RTOS with check for USE_UNOPTIMIZED_PCM_SRC to 
 *        select between using the Optimized ARM routines and the Unoptimized 
 *        routines
 *        
 *  9    mpeg      1.8         11/25/02 12:48:20 PM   Senthil Veluswamy SCR(s) 
 *        5001 :
 *        Modified to use optimized PCM SRC routines for MonoLinearInterpolate,
 *         StereoLinearInterpolate, MonoFilterAndDecimate, 
 *        StereoFilterAndDecimate. 
 *        
 *  8    mpeg      1.7         11/23/02 4:40:32 PM    Senthil Veluswamy SCR(s) 
 *        5001 :
 *        Replaced StereoFilterAndDecimate with optimized Assembly routine.
 *        
 *  7    mpeg      1.6         11/22/02 10:51:54 AM   Senthil Veluswamy SCR(s) 
 *        5012 :
 *        Wrapped the optimized routines for VxW for now. Will need to create a
 *         separate VxW assembly file later
 *        
 *  6    mpeg      1.5         11/20/02 5:58:24 PM    Senthil Veluswamy SCR(s) 
 *        5001 :
 *        Replaced StereoLinearInterpolate with a call to the Assembly coded 
 *        StereoLinearInterpolate routine
 *        
 *  5    mpeg      1.4         6/20/02 6:11:54 PM     Steve Glennon   SCR(s): 
 *        4072 
 *        Eliminated a case where the task could always be runnable when in 
 *        idle state
 *        Put a wait in for this case to free up CPU.
 *        
 *  4    mpeg      1.3         6/20/02 5:14:56 PM     Steve Glennon   SCR(s): 
 *        4071 
 *        Added code to flush PCMSRCQ and PCMInputQ when issued a stop command
 *        
 *  3    mpeg      1.2         6/19/02 5:54:36 PM     Steve Glennon   SCR(s): 
 *        4066 
 *        Many fixes to original code which did not work correctly at all. 
 *        Enqueued
 *        and dequeued data from different queues etc.
 *        
 *  2    mpeg      1.1         5/17/02 5:03:56 PM     Steve Glennon   SCR(s): 
 *        3822 
 *        Removed use of bitfields to access hardware registers.
 *        
 *        
 *  1    mpeg      1.0         5/15/02 3:28:12 AM     Steve Glennon   
 * $
 * 
 *    Rev 1.16   28 Apr 2003 14:56:20   velusws
 * SCR(s) 6114 :
 * Modified to use the Fast En/DeQ routines when SRC Optimization is turned on.
 * 
 *    Rev 1.15   21 Apr 2003 19:34:08   velusws
 * SCR(s) 6066 6065 :
 * Changed PCM_DISABLE_SRC to take values YES and NO instead of just being defined.
 * 
 *    Rev 1.14   21 Apr 2003 19:05:46   velusws
 * SCR(s) 6066 6065 :
 * Wrapped code to compile time switch out the SRC code if PCM_DISABLE_SRC is set to YES
 * 
 *    Rev 1.13   13 Feb 2003 12:25:02   kortemw
 * SCR(s) 5479 :
 * Removed old header reference
 * 
 *    Rev 1.12   14 Jan 2003 10:34:22   velusws
 * SCR(s) 5230 :
 * Fixed bug. Was erroneously using NULL fn ptr.
 * 
 *    Rev 1.11   10 Jan 2003 17:39:28   velusws
 * SCR(s) 5230 :
 * Added support for General Event Notification & Data Feed/Stop Events.
 * 
 *    Rev 1.10   03 Dec 2002 14:51:08   velusws
 * SCR(s) 5058 :
 * Fixed warnings from extern vars
 * 
 *    Rev 1.9   27 Nov 2002 12:50:12   velusws
 * SCR(s) 5001 :
 * Replaced check for RTOS with check for USE_UNOPTIMIZED_PCM_SRC to select between using the Optimized ARM routines and the Unoptimized routines
 * 
 *    Rev 1.8   25 Nov 2002 12:48:20   velusws
 * SCR(s) 5001 :
 * Modified to use optimized PCM SRC routines for MonoLinearInterpolate, StereoLinearInterpolate, MonoFilterAndDecimate, StereoFilterAndDecimate. 
 * 
 *    Rev 1.7   23 Nov 2002 16:40:32   velusws
 * SCR(s) 5001 :
 * Replaced StereoFilterAndDecimate with optimized Assembly routine.
 * 
 *    Rev 1.6   22 Nov 2002 10:51:54   velusws
 * SCR(s) 5012 :
 * Wrapped the optimized routines for VxW for now. Will need to create a separate VxW assembly file later
 * 
 *    Rev 1.5   20 Nov 2002 17:58:24   velusws
 * SCR(s) 5001 :
 * Replaced StereoLinearInterpolate with a call to the Assembly coded StereoLinearInterpolate routine
 * 
 *    Rev 1.4   Jun 20 2002 18:11:54   glennon
 * SCR(s): 4072 
 * Eliminated a case where the task could always be runnable when in idle state
 * Put a wait in for this case to free up CPU.
 * 
 *    Rev 1.3   Jun 20 2002 17:14:56   glennon
 * SCR(s): 4071 
 * Added code to flush PCMSRCQ and PCMInputQ when issued a stop command
 * 
 *    Rev 1.2   Jun 19 2002 17:54:36   glennon
 * SCR(s): 4066 
 * Many fixes to original code which did not work correctly at all. Enqueued
 * and dequeued data from different queues etc.
 * 
 *    Rev 1.1   17 May 2002 16:03:56   glennon
 * SCR(s): 3822 
 * Removed use of bitfields to access hardware registers.
 * 
 * 
 *    Rev 1.0   15 May 2002 02:28:12   glennon
 * SCR(s): 2438 
 * Sample Rate Conversion routines for the generic PCM driver
 * 
 *
 ****************************************************************************/

