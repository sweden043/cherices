/****************************************************************************/
/*                   CONEXANT PROPRIETARY AND CONFIDENTIAL                  */
/*                       SOFTWARE FILE/MODULE HEADER                        */
/*                    Conexant Systems Inc. (c) 1998-2002                   */
/*                               Austin, TX                                 */
/*                            All Rights Reserved                           */
/****************************************************************************/
/*
 * Filename:       ac3fix.c
 *
 *
 * Description:    Source File for the MPEG Audio Data Dry Error Fix
 *
 *
 * Author:         Senthil Veluswamy (based on code in video.c)
 *
 ****************************************************************************/
/* $Header: datadryfix.c, 2, 2/13/03 11:31:38 AM, Matt Korte$
 ****************************************************************************/

/*****************/
/* Include Files */
/*****************/
#include "stbcfg.h"
#include "kal.h"
#include "retcodes.h"
#include "trace.h"
#include "hwlib.h"
#include "aud_comn.h"
#include "aud_error.h"
#include "string.h"

/***********/
/* Aliases */
/***********/

/***************/
/* Global Data */
/***************/
bool bCheckForAudioDry = TRUE;

/***********************/
/* Function prototypes */
/***********************/
extern void register_mpg_audio_error_callback
            (pfnMPGAudioErrorCallback pfnAudioErrorCb, u_int32 uDSRRegValue);

bool datadry_fix_init(void);
static void datadry_fix_align_callback(u_int32 uDSRRegValue);

/************************/
/* Function definitions */
/************************/
/*********************************************************************/
/*  datadry_fix_init()                                               */
/*                                                                   */
/*  PARAMETERS: None.                                                */
/*                                                                   */
/*  DESCRIPTION:Register a callback to handle the Data dry error.    */
/*                                                                   */
/*  RETURNS:    Nothing.                                             */
/*********************************************************************/
bool datadry_fix_init(void)
{
   /* Register the Data Dry Error Callback */
   register_mpg_audio_error_callback(datadry_fix_align_callback,
                                     MPG_AUDIO_DATA_DRY);

   return(TRUE);
}

/*********************************************************************/
/*  datadry_fix_align_callback()                                     */
/*                                                                   */
/*  PARAMETERS: uDSRRegValue - Value of the DSR register.            */
/*                                                                   */
/*  DESCRIPTION:Handle the Data dry error when called.               */
/*                                                                   */
/*  RETURNS:    Nothing.                                             */
/*********************************************************************/
static void datadry_fix_align_callback(u_int32 uDSRRegValue)
{
   /* For Canal+ we must mute the audio the instant that the encoded data dries  */
   /* up. This the audio decoder from repeating the last frame in their somewhat */
   /* unusual tests involving partially scrambled streams.                       */
   if((uDSRRegValue & MPG_AUDIO_DATA_DRY) && bCheckForAudioDry)
   {
      isr_trace_new(TRACE_MPG|TRACE_LEVEL_3, 
         "DataDryFixIsr: Signalling audio to mute on data drying up\n", 0, 0);
      cnxt_audio_drop_out();
      bCheckForAudioDry = FALSE;
   }

   return;
}

/****************************************************************************
 * Modifications:
 * $Log: 
 *  2    mpeg      1.1         2/13/03 11:31:38 AM    Matt Korte      SCR(s) 
 *        5479 :
 *        Removed old header reference
 *        
 *  1    mpeg      1.0         11/15/02 4:00:40 PM    Senthil Veluswamy 
 * $
 * 
 *    Rev 1.1   13 Feb 2003 11:31:38   kortemw
 * SCR(s) 5479 :
 * Removed old header reference
 * 
 *    Rev 1.0   15 Nov 2002 16:00:40   velusws
 * SCR(s) 4965 :
 * Source File for the MPEG Audio Data Dry Error Fix
 ****************************************************************************/

