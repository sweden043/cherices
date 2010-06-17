/******************************************************************************/
/*                           Conexant Systems, Inc.                           */
/******************************************************************************/
/*                                                                            */
/* Filename:       ATTENUAT.C                                                 */
/*                                                                            */
/* Description:    Generic PCM Driver, Attenuation                            */
/*                                                                            */
/* Author:         Christopher J. Chapman                                     */
/*                                                                            */
/* Copyright (c) 2000 Conexant Systems, Inc.                                  */
/* All Rights Reserved.                                                       */
/*                                                                            */
/******************************************************************************/
/* $Header: attenuat.c, 2, 6/19/02 5:54:36 PM, Steve Glennon$
 ****************************************************************************/



#include "basetype.h"
#include "pcmcq.h"
#include "kal.h"
#include "pcm.h"
#include "gen_aud.h"
#include "pcmpriv.h"

/*
   Note:

   These coefficients are to be used to attenuate a 20 bit signal by
   multiplying the signal data by the multiplier and then shifting
   the signal to the right by 11 bits.

   The index to this array is perceived percentage of full volume, e.g.:

   y = perceived 10% of x's volume
   y = ((attenuation_coefficients[10] * x) >> 11);

   y = perceived 95% of x's volume
   y = ((attenuation_coefficients[95] * x) >> 11);

   If "L" is perceived loudness and "A1" and "A2" are signal amplitudes,
   their relationship is approximately:

   L = (A2/A1)^(2/3)

   A2/A1 = L^(3/2)   

*/

/* 0 - 100 corresponds to (perceived) 0% - 100% of full signal */
int32 attenuation_coefficients[101] =
{

   0, 2, 6, 11, 16, 23, 30, 38, 46, 55, 65, 75, 85, 96, 107, 119, 131,
   144, 156, 170, 183, 197, 211, 226, 241, 256, 272, 287, 303, 320, 337,
   353, 371, 388, 406, 424, 442, 461, 480, 499, 518, 538, 557, 577, 598,
   618, 639, 660, 681, 702, 724, 746, 768, 790, 813, 835, 858, 881, 905,
   928, 952, 976, 1000, 1024, 1049, 1073, 1098, 1123, 1148, 1174, 1199,
   1225, 1251, 1277, 1304, 1330, 1357, 1384, 1411, 1438, 1465, 1493,
   1521, 1549, 1577, 1605, 1633, 1662, 1691, 1720, 1749, 1778, 1807,
   1837, 1866, 1896, 1926, 1957, 1987, 2017, 2048
};



/*******************************************************************************

   AttenuateMonoSamples()

   Description:
      Attenutates a buffer of mono (not stereo) PCM samples to
      the specified volume

   Parameters
      Samples -- buffer of mono 20 bit signed samples
      SampleCount -- how many samples in buffer
      Volume -- volume in percent (i.e. 100 means 100% of full signal)

   Return Value
      None

   Notes
      Volume policy is not mandated here. Volume is just a number and 
      uses a table to apply a scaling to samples.

*******************************************************************************/

void AttenuateMonoSamples(PCM20 * Samples, int SampleCount, int Volume)
{
   PCM20 *pCurrentSample;
   int i;
   int32  PcmMonoAttenShift;
   int32  PcmMonoAttenMulti;

   /* Calculate a single value of shift and multiply for mono samples */
   PcmMonoAttenShift   = cnxt_pcm_attenuation_shift[Volume];
   PcmMonoAttenMulti   = cnxt_pcm_attenuation_multiply[Volume];

   pCurrentSample = Samples;
   for(i=0; i<SampleCount; i++)
   {
      *pCurrentSample = (*pCurrentSample * PcmMonoAttenMulti) >> PcmMonoAttenShift;
      pCurrentSample++;
   }
}

/*******************************************************************************

   AttenuateStereoSamples()

   Description:
      Attenutates a buffer of stereo PCM samples to the specified volume

   Parameters
      Samples -- buffer of stereo 20 bit signed samples
      SampleCount -- how many samples in buffer
      LeftVolume -- volume in percent for left channel
      RightVolume -- volume in percent for right channel

   Return Value
      None

   Notes
      Volume policy is not mandated here. Volume is just a number and 
      uses a table to apply a scaling to samples.

*******************************************************************************/

void AttenuateStereoSamples(PCM20 * Samples, int SampleCount, int LeftVolume,
   int RightVolume)
{
   PCM20 *pCurrentSample;
   int i;
   int32 PcmLeftAttenShift;
   int32 PcmRightAttenShift;
   int32 PcmLeftAttenMulti;
   int32 PcmRightAttenMulti;
   
   // For each channel, get a multiplication factor and a shift value
   // that will result in the proper attenuation to step the volume
   PcmLeftAttenShift   = cnxt_pcm_attenuation_shift[LeftVolume];
   PcmLeftAttenMulti   = cnxt_pcm_attenuation_multiply[LeftVolume];
   PcmRightAttenShift  = cnxt_pcm_attenuation_shift[RightVolume];
   PcmRightAttenMulti  = cnxt_pcm_attenuation_multiply[RightVolume];

   /* assuming offset 0 is a left channel sample */
   pCurrentSample = Samples;
   for(i=0; i<SampleCount; i += 2)
   {
      *pCurrentSample = (*pCurrentSample * PcmLeftAttenMulti) >> PcmLeftAttenShift;
      pCurrentSample++;
      *pCurrentSample = (*pCurrentSample * PcmRightAttenMulti) >> PcmRightAttenShift;
      pCurrentSample++;
   }

}

/*******************************************************************************

   AttenuateSamples()

   Description:
      Attenutates a buffer of  PCM samples to the specified volume

   Parameters
      Samples -- buffer of 20 bit signed samples
      SampleCount -- how many samples in buffer
      bStereo -- TRUE if stereo samples
      ui8LeftVol -- left volume
      ui8RightVol -- Right volume

   Return Value
      None

   Notes
      Volume policy is not mandated here. Volume is just a number and 
      uses a table to apply a scaling to samples.
*******************************************************************************/

void AttenuateSamples(PCM20 *Samples, 
                      int    SampleCount,
                      bool   bStereo,
                      u_int8 ui8LeftVol,
                      u_int8 ui8RightVol)

{
   
   /* attenuate data */
   if (bStereo)
   {
      AttenuateStereoSamples(Samples, 
                             SampleCount, 
                             ui8LeftVol,
                             ui8RightVol);
   }
   else
   {
      AttenuateMonoSamples(Samples, 
                           SampleCount,
                           ((ui8LeftVol + ui8RightVol) >> 1));
      
   }
   return;
   
} /* AttenuateSamples */


/* ************************************************************************** */

/****************************************************************************
 * $Log: 
 *  2    mpeg      1.1         6/19/02 5:54:36 PM     Steve Glennon   SCR(s): 
 *        4066 
 *        Many fixes to original code which did not work correctly at all. 
 *        Enqueued
 *        and dequeued data from different queues etc.
 *        
 *  1    mpeg      1.0         5/15/02 3:28:06 AM     Steve Glennon   
 * $
 * 
 *    Rev 1.1   Jun 19 2002 17:54:36   glennon
 * SCR(s): 4066 
 * Many fixes to original code which did not work correctly at all. Enqueued
 * and dequeued data from different queues etc.
 * 
 *    Rev 1.0   15 May 2002 02:28:06   glennon
 * SCR(s): 2438 
 * Attenuation functions for generic PCM driver
 * 
 *
 ****************************************************************************/
