/****************************************************************************/
/*                   Conexant Proprietary and Confidential                  */
/*                       Software File/Module Header                        */
/*              Conexant Systems Inc. (c) 1998, 1999, 2000, 2001, 2002      */
/*                               Austin, TX                                 */
/*                            All Rights Reserved                           */
/****************************************************************************/
/*
 * Filename:       aud_comn.h
 *
 *
 * Description:    Definitions and prototypes for the AUD_COMN driver, which
 *                 is the general audio driver.
 *
 *
 * Author:         Matthew W. Korte
 *
 ****************************************************************************/
/* $Header: aud_comn.h, 15, 4/1/04 5:20:33 PM, Dave Aerne$
 ****************************************************************************/

#ifndef _AUD_COMN_H_
#define _AUD_COMN_H_

/************** define Section Start ***********************************/
/* AUDIO_INFO Start */
/*
* This section is for the 'soft' register for Audio Info
* and associated definitions. The 'soft' register is a
* location in the Audio 2 decode buffer that the audio
* microcode updates with a 32 bit value every audio frame
* and when they go to reset. The upper bit currently contains
* a valid bit, and the low 20 bits contain the mpeg header.
*/
#define AUDIO_INFO_REG            0xf200        /* Location of 'soft' reg */
#define AUDIO_INFO_MIN_TIME       120           /* Time to wait for new value */
#define AUDIO_INFO_VALID_MASK     0x80000000    /* Mask for audio valid */
#define AUDIO_INFO_HEADER_MASK    0x000fffff    /* Mask for mpeg header */
#define AUDIO_HEADER_EMPH_MASK    0x00000003    /* Header: Emphasis */
#define AUDIO_HEADER_ORG_MASK     0x00000004    /* Header: original/copy (Not Used) */
#define AUDIO_HEADER_CPYRHT_MASK  0x00000008    /* Header: Copyright, 1=YES */
#define AUDIO_HEADER_MODEX_MASK   0x00000030    /* Header: Mode Extension */
#define AUDIO_HEADER_MODE_MASK    0x000000c0    /* Header: Mode */
#define AUDIO_HEADER_PRIVATE_MASK 0x00000100    /* Header: Private (Not Used) */
#define AUDIO_HEADER_PAD_MASK     0x00000200    /* Header: Padding, 1=ON */
#define AUDIO_HEADER_FREQ_MASK    0x00000c00    /* Header: Sampling Frequency */
#define AUDIO_HEADER_BITRATE_MASK 0x0000f000    /* Header: Bit rate */
#define AUDIO_HEADER_PROT_MASK    0x00010000    /* Header: Protection: 1=No CRC present */
#define AUDIO_HEADER_LAYER_MASK   0x00060000    /* Header: Layer */
#define AUDIO_HEADER_ID_MASK      0x00080000    /* Header: ID */

/* Emphasis defined: */
#define AUDIO_EMPH_NONE           0x00    /* No emphasis */
#define AUDIO_EMPH_50_15_USEC     0x01    /* 50/15 microsec emphasis */

/* Mode defined: See macro definitions in extern section for use */
#define AUDIO_MODE_STEREO         0x00    /* Stereo */
#define AUDIO_MODE_DUAL_MONO      0x80    /* Dual Mono */
#define AUDIO_MODE_MONO           0xc0    /* Single Mono */
#define AUDIO_MODE_JOINT_STEREO   0x40    /* Joint Stereo, see mode ext field */

/* Mode Extention field (if mode is joint stereo) */
#define AUDIO_MODEX_SUB_4_31      0x00    /* Subband 4-31 in intensity stereo */
#define AUDIO_MODEX_SUB_8_31      0x10    /* Subband 8-31 in intensity stereo */
#define AUDIO_MODEX_SUB_12_31     0x20    /* Subband 12-31 in intensity stereo */
#define AUDIO_MODEX_SUB_16_31     0x30    /* Subband 16-31 in intensity stereo */

/* Sample Frequency defined: */
#define AUDIO_SAMP_FREQ_44        0x000   /* Sample Freq of 44.1KHz or 22.05KHz (See ID) */
#define AUDIO_SAMP_FREQ_48        0x400   /* Sample Freq of 48  KHz or 24   KHz (See ID) */
#define AUDIO_SAMP_FREQ_32        0x800   /* Sample Freq of 32  KHz or 16   KHz (See ID) */

                                          /*                 Layer   Layer    */
                                          /*                  II      I       */
#define AUDIO_BITRATE_INDEX_0     0x0000  /* Bitrate Index 0 (Free format)    */
#define AUDIO_BITRATE_INDEX_1     0x1000  /* Bitrate Index 1 (032 or 032 kbs) */
#define AUDIO_BITRATE_INDEX_2     0x2000  /* Bitrate Index 2 (048 or 064 kbs) */
#define AUDIO_BITRATE_INDEX_3     0x3000  /* Bitrate Index 3 (056 or 096 kbs) */
#define AUDIO_BITRATE_INDEX_4     0x4000  /* Bitrate Index 4 (064 or 128 kbs) */
#define AUDIO_BITRATE_INDEX_5     0x5000  /* Bitrate Index 5 (080 or 160 kbs) */
#define AUDIO_BITRATE_INDEX_6     0x6000  /* Bitrate Index 6 (096 or 192 kbs) */
#define AUDIO_BITRATE_INDEX_7     0x7000  /* Bitrate Index 7 (112 or 224 kbs) */
#define AUDIO_BITRATE_INDEX_8     0x8000  /* Bitrate Index 8 (128 or 256 kbs) */
#define AUDIO_BITRATE_INDEX_9     0x9000  /* Bitrate Index 9 (160 or 288 kbs) */
#define AUDIO_BITRATE_INDEX_a     0xa000  /* Bitrate Index a (192 or 320 kbs) */
#define AUDIO_BITRATE_INDEX_b     0xb000  /* Bitrate Index b (224 or 352 kbs) */
#define AUDIO_BITRATE_INDEX_c     0xc000  /* Bitrate Index c (256 or 384 kbs) */
#define AUDIO_BITRATE_INDEX_d     0xd000  /* Bitrate Index d (320 or 416 kbs) */
#define AUDIO_BITRATE_INDEX_e     0xe000  /* Bitrate Index e (384 or 448 kbs) */

#define AUDIO_LAYER_II            0x40000 /* Layer II */
#define AUDIO_LAYER_I             0x60000 /* Layer I */

#define AUDIO_ID_NORMAL           0x80000 /* Normal, use sample freq as is */
#define AUDIO_ID_HALF             0x00000 /* Half SF, use 1/2 sample freq  */
/* AUDIO_INFO End */

/* AUDIO_UCODE Start */
#if AUDIO_MICROCODE == UCODE_COLORADO
#define MPEG_AUDIO_UCODE      col_mpeg_audio_ucode
#define MPEG_AUDIO_UCODE_SIZE col_mpeg_audio_ucode_size
#define AC3_AUDIO_UCODE       col_ac3_audio_ucode
#define AC3_AUDIO_UCODE_SIZE  col_ac3_audio_ucode_size
#elif AUDIO_MICROCODE == UCODE_HONDO
#define MPEG_AUDIO_UCODE      hond_mpeg_audio_ucode
#define MPEG_AUDIO_UCODE_SIZE hond_mpeg_audio_ucode_size
#define AC3_AUDIO_UCODE       hond_ac3_audio_ucode
#define AC3_AUDIO_UCODE_SIZE  hond_ac3_audio_ucode_size
#elif AUDIO_MICROCODE == UCODE_WABASH
#define MPEG_AUDIO_UCODE      wabash_mpeg_audio_ucode
#define MPEG_AUDIO_UCODE_SIZE wabash_mpeg_audio_ucode_size
#define AC3_AUDIO_UCODE       wabash_ac3_audio_ucode
#define AC3_AUDIO_UCODE_SIZE  wabash_ac3_audio_ucode_size
#else /* AUDIO_MICROCODE == UCODE_BRAZOS */
#define MPEG_AUDIO_UCODE      brazos_mpeg_audio_ucode
#define MPEG_AUDIO_UCODE_SIZE brazos_mpeg_audio_ucode_size
#define AC3_AUDIO_UCODE       brazos_ac3_audio_ucode
#define AC3_AUDIO_UCODE_SIZE  brazos_ac3_audio_ucode_size
#define AOC_AUDIO_UCODE       brazos_aoc_audio_ucode
#define AOC_AUDIO_UCODE_SIZE  brazos_aoc_audio_ucode_size
#endif
/* AUDIO_UCODE End */

/* Audio stop Start */
/* Maximum time to wait for audio decode to stop */
#define AUDIO_STOP_MAX_WAIT   100
/* Audio stop End   */

#if ((PCM_AUDIO_TYPE != PCM_AUDIO_COLORADO) && (PCM_AUDIO_TYPE != PCM_AUDIO_WABASH))

/* SPDIF Header (bytes 0-4) definition */
typedef enum
{
   AUDIO_SPDIF_HEADER_PARAM_CONSUMER,
   AUDIO_SPDIF_HEADER_PARAM_DATA_AUDIO,
   AUDIO_SPDIF_HEADER_PARAM_COPYRIGHT,
   AUDIO_SPDIF_HEADER_PARAM_EMPHASIS,
   AUDIO_SPDIF_HEADER_PARAM_MODE,
   AUDIO_SPDIF_HEADER_PARAM_MODE00_CATEGORY_CODE,
   AUDIO_SPDIF_HEADER_PARAM_MODE00_GENERATION_STATUS,
   AUDIO_SPDIF_HEADER_PARAM_MODE00_SOURCE_NUM,
   AUDIO_SPDIF_HEADER_PARAM_MODE00_CHANNEL_NUM,
   AUDIO_SPDIF_HEADER_PARAM_MODE00_SAMPLE_FREQUENCY,
   AUDIO_SPDIF_HEADER_PARAM_MODE00_CLK_ACCURACY
} AUDIO_SPDIF_HEADER_PARAM;

#endif /* PCM_AUDIO_TYPE != PCM_AUDIO_COLORADO && PCM_AUDIO_WABASH */

/************** define Section End   ***********************************/

/************** typedef Section Start *********************************/
typedef void (*gen_aud_ISRNotify_t)(void);
typedef void (*gen_aud_ASRCNotify_t)(unsigned int SampleRate);
typedef bool (*mixer_volume_func_t)(unsigned char left, unsigned char right);    
/************** typedef Section End   *********************************/

/************** prototype Section Start ********************************/
extern bool gen_audio_init(bool useAC3ucode, bool reinit);
extern void gen_audio_play(bool SyncWithPCR);
extern void gen_audio_stop( void );
extern void hw_set_aud_mute(bool);
extern void hw_set_aud_sync(bool);
extern int audio_set_volume(int percentage);
extern int audio_get_volume(void);
#if ((PCM_AUDIO_TYPE != PCM_AUDIO_COLORADO) && (PCM_AUDIO_TYPE != PCM_AUDIO_WABASH))
 extern int auddecoder_set_volume(int percentage);
 extern int auddecoder_get_volume(void);
 #if (CPU_TYPE!=CPU_ARM940T)
  #if CPU_TYPE == AUTOSENSE
   extern int GetCPUType();
  #endif
 extern int rfmod_set_volume(int percentage);
 extern int rfmod_get_volume(void);
 #endif /* (CPU_TYPE!=CPU_ARM940T) */
#endif /* PCM_AUDIO_TYPE != PCM_AUDIO_COLORADO && PCM_AUDIO_WABASH */
extern int configure_PLL(int rate);
extern gen_aud_ISRNotify_t gen_aud_ISRNotifyRegister(u_int32 interrupt_bit,
                               gen_aud_ISRNotify_t handler);
extern void gen_aud_ASRCNotifyRegister(gen_aud_ASRCNotify_t ASRChangeNotify);
extern void AudMPGVolumeFuncRegister(mixer_volume_func_t volume_function);
extern void AudPCMVolumeFuncRegister(mixer_volume_func_t volume_function);
extern int audio_mpeg_isr( u_int32, bool, PFNISR * );
extern void aud_before_reset(void);
extern void aud_after_reset(void);
extern bool cnxt_download_ucode( bool useAC3ucode );
extern bool cnxt_ac3_passthru( bool TurnAc3On );
extern bool cnxt_MpgDecAc3Pt( bool EnableDecAndPt );
extern bool cnxt_audio_is_valid( void );
extern u_int32 cnxt_audio_get_header( void );
extern u_int32 cnxt_audio_get_audio_info( void );
extern void cnxt_audio_drop_out( void );
extern void cnxt_audio_get_buffer_errors( u_int32 *pOverflow, u_int32 *pUnderflow);

#if ((PCM_AUDIO_TYPE != PCM_AUDIO_COLORADO) && (PCM_AUDIO_TYPE != PCM_AUDIO_WABASH))
 #if (CPU_TYPE!=CPU_ARM940T)
int cnxt_audio_spdif_headerbits_set(AUDIO_SPDIF_HEADER_PARAM eParam, 
                                    u_int32 ui32Value);
int cnxt_audio_spdif_headerbits_get(AUDIO_SPDIF_HEADER_PARAM eParam, 
                                    u_int32 *pui32Value);
 #endif /* (CPU_TYPE!=CPU_ARM940T) */
#endif /* PCM_AUDIO_TYPE != PCM_AUDIO_COLORADO && PCM_AUDIO_WABASH */

#define cnxt_audio_get_mode()       (cnxt_audio_get_header() & AUDIO_HEADER_MODE_MASK)
#define cnxt_audio_is_stereo()      (cnxt_audio_get_mode() == AUDIO_MODE_STEREO)
#define cnxt_audio_is_dual_mono()   (cnxt_audio_get_mode() == AUDIO_MODE_DUAL_MONO)
#define cnxt_audio_is_mono()        (cnxt_audio_get_mode() == AUDIO_MODE_MONO)
#define cnxt_audio_is_jnt_stereo()  (cnxt_audio_get_mode() == AUDIO_MODE_JOINT_STEREO)

bool cnxt_check_av_channel_status(int channel_id);

void ipanel_get_apts(u_int32 *pts_value);

/************** prototype Section End   ********************************/
#endif /* _AUD_COMN_H_ */
/****************************************************************************
 * Modifications:
 * $Log: 
 *  15   mpeg      1.14        4/1/04 5:20:33 PM      Dave Aerne      CR(s) 
 *        8729 8730 : added support for ac3 passthrough w/o mpeg decode on 
 *        brazos and wabash chips.
 *  14   mpeg      1.13        7/9/03 6:18:28 PM      Senthil Veluswamy SCR(s) 
 *        6922 :
 *        Added interfaces to get and set the SPDIF header bits on Bronco B+. 
 *        Added enum with defines for the various SPDIF header bits
 *        
 *  13   mpeg      1.12        6/23/03 7:07:22 PM     Senthil Veluswamy SCR(s) 
 *        6641 6642 6643 :
 *        Added interfaces to get/set rf modulator and audio decoder volume.
 *        
 *  12   mpeg      1.11        5/15/03 6:22:30 PM     Dave Aerne      SCR(s) 
 *        6261 6262 6291 :
 *        added defines for brazos audio microcodes
 *        
 *  11   mpeg      1.10        3/19/03 3:59:32 PM     Dave Wilson     SCR(s) 
 *        5136 5829 :
 *        Added prototype for new function cnxt_audio_get_buffer_errors
 *        
 *  10   mpeg      1.9         12/16/02 3:47:52 PM    Tim White       SCR(s) 
 *        5169 :
 *        Allow future chips to use Wabash code by default instead of the 
 *        Colorado code.
 *        
 *        
 *  9    mpeg      1.8         11/25/02 7:43:10 PM    Craig Dry       SCR(s) 
 *        4991 :
 *        use AUDIO_LAYER_I rather than AUDIO_LAYER_1
 *        
 *  8    mpeg      1.7         11/20/02 2:28:00 PM    Craig Dry       SCR(s) 
 *        4991 :
 *        Canal+ DLI4.2 Audio Extensions and Audio Driver Enhancements
 *        
 *  7    mpeg      1.6         9/3/02 7:31:12 PM      Matt Korte      SCR(s) 
 *        4498 :
 *        Remove warnings
 *        
 *  6    mpeg      1.5         7/1/02 5:11:00 PM      Matt Korte      SCR(s) 
 *        4120 :
 *        Change gen_aud_ISRNotifyRegister() to be more generic
 *        
 *        
 *  5    mpeg      1.4         5/20/02 11:30:10 AM    Steve Glennon   SCR(s): 
 *        3827 
 *        Checked back in aud_comn.h
 *        
 *        
 *  4    mpeg      1.3         5/16/02 10:55:04 AM    Dave Wilson     SCR(s) 
 *        3785 :
 *        Added prototypes for cnxt_audio_get_audio_info and 
 *        cnxt_audio_dropout.
 *        
 *  3    mpeg      1.2         5/13/02 1:55:04 PM     Tim White       SCR(s) 
 *        3760 :
 *        Removed duplicate definition found in <chip>.h files.
 *        
 *        
 *  2    mpeg      1.1         5/2/02 4:58:08 PM      Dave Wilson     SCR(s) 
 *        3687 :
 *        Increased AUDIO_INFO_MIN_TIME from 100mS to 120mS.
 *        
 *  1    mpeg      1.0         3/14/02 3:52:56 PM     Matt Korte      
 * $
 * 
 *    Rev 1.13   09 Jul 2003 17:18:28   velusws
 * SCR(s) 6922 :
 * Added interfaces to get and set the SPDIF header bits on Bronco B+. Added enum with defines for the various SPDIF header bits
 * 
 *    Rev 1.12   23 Jun 2003 18:07:22   velusws
 * SCR(s) 6641 6642 6643 :
 * Added interfaces to get/set rf modulator and audio decoder volume.
 * 
 *    Rev 1.11   15 May 2003 17:22:30   aernedj
 * SCR(s) 6261 6262 6291 :
 * added defines for brazos audio microcodes
 * 
 *    Rev 1.10   19 Mar 2003 15:59:32   dawilson
 * SCR(s) 5136 5829 :
 * Added prototype for new function cnxt_audio_get_buffer_errors
 * 
 *    Rev 1.9   16 Dec 2002 15:47:52   whiteth
 * SCR(s) 5169 :
 * Allow future chips to use Wabash code by default instead of the Colorado code.
 * 
 * 
 *    Rev 1.8   25 Nov 2002 19:43:10   dryd
 * SCR(s) 4991 :
 * use AUDIO_LAYER_I rather than AUDIO_LAYER_1
 * 
 *    Rev 1.7   20 Nov 2002 14:28:00   dryd
 * SCR(s) 4991 :
 * Canal+ DLI4.2 Audio Extensions and Audio Driver Enhancements
 * 
 *    Rev 1.6   03 Sep 2002 18:31:12   kortemw
 * SCR(s) 4498 :
 * Remove warnings
 * 
 *    Rev 1.5   01 Jul 2002 16:11:00   kortemw
 * SCR(s) 4120 :
 * Change gen_aud_ISRNotifyRegister() to be more generic
 * 
 * 
 *    Rev 1.4   20 May 2002 10:30:10   glennon
 * SCR(s): 3827 
 * Checked back in aud_comn.h
 * 
 * 
 *    Rev 1.3   16 May 2002 09:55:04   dawilson
 * SCR(s) 3785 :
 * Added prototypes for cnxt_audio_get_audio_info and cnxt_audio_dropout.
 * 
 *    Rev 1.2   13 May 2002 12:55:04   whiteth
 * SCR(s) 3760 :
 * Removed duplicate definition found in <chip>.h files.
 * 
 * 
 *    Rev 1.1   02 May 2002 15:58:08   dawilson
 * SCR(s) 3687 :
 * Increased AUDIO_INFO_MIN_TIME from 100mS to 120mS.
 * 
 *    Rev 1.0   14 Mar 2002 15:52:56   kortemw
 * SCR(s) 3375 :
 * Contains information for the generic audio driver called aud_comn.
 * 
 ****************************************************************************/

