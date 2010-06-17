/****************************************************************************/
/*                 Rockwell Semiconductor Systems - SABINE                  */
/****************************************************************************/
/*                                                                          */
/* Filename:           AUDIO.H                                              */
/*                                                                          */
/* Description:        Public header file for sabine audio driver (OpenTV   */
/*                     function prototypes)                                 */
/*                                                                          */
/* Author:             Amy Pratt                                            */
/*                                                                          */
/* Copyright Rockwell Semiconductor Systems, 1998                           */
/* All Rights Reserved.                                                     */
/*                                                                          */
/****************************************************************************/
/****************************************************************************
$Header: audio.h, 12, 6/23/03 7:24:42 PM, $
****************************************************************************/

#ifndef _AUDIO_H_
#define _AUDIO_H_

/* audio function prototypes */

voidF audio_get_buffer( void );
int   audio_buffer_size( void );
void  audio_mute( void );
void  audio_unmute( void );
void  audio_turn_sync_off( void );
void  audio_turn_sync_on( void );
void  audio_play( audio_format a_format, int sync );
void  audio_stop( void );
void  audio_pause( void );
void  audio_resume( void );
int   audio_data_ready( farcir_q *q );
void  audio_report_empty_data( void );
bool  audio_init( bool );
int   audio_set_volume(int percentage);
int   audio_get_volume(void);
#if ((PCM_AUDIO_TYPE != PCM_AUDIO_COLORADO) && (PCM_AUDIO_TYPE != PCM_AUDIO_WABASH))
 int auddecoder_set_volume(int percentage);
 int auddecoder_get_volume(void);
 #if (CPU_TYPE!=CPU_ARM940T)
  #if CPU_TYPE == AUTOSENSE
   extern int GetCPUType();
  #endif
 int rfmod_set_volume(int percentage);
 int rfmod_get_volume(void);
 #endif /* (CPU_TYPE!=CPU_ARM940T) */
#endif /* PCM_AUDIO_TYPE != PCM_AUDIO_COLORADO && PCM_AUDIO_WABASH */

#ifndef min
#define min(x,y)        (((x) < (y)) ? (x) : (y))
#endif

#define AUDIO_BUFFER_SIZE     HWBUF_ENCAUD_SIZE
#define AUDIO_BUFFER_ADDRESS  audio_sw_buffer

typedef struct  _PES_global_data
{
  int  where_in_PES;
  int  progress;
  int  bytes_left_in_packet;
  int  bytes_left_in_header;
  int  pts_present;
  unsigned char pts_array[5];
  bool PesLenTBD;
  int  InitialPacketLen;
} PES_global_data;

#ifndef _AUD_API_C_
extern PES_global_data aud_PES_global_data;
#endif

enum { FIND_PREFIX, PARSING_HEADER, SENDING_DATA };
enum { PREFIX_FOUND, LAST_BYTE_NONZERO, LAST_BYTE_ZERO, LAST_TWO_BYTES_ZERO,
             SKIP_STREAM_ID, GET_PACKET_LENGTH1, GET_PACKET_LENGTH2,
                 GET_MPEG1_MPEG2, SKIPPING_DATA_BITS, SKIPPING_STUFFING_BYTES,
         SKIP_SECOND_BYTE_OF_STD, GET_PTS_MPEG1, PTS12, PTS13, PTS14,
                 FIFTH_BYTE_OF_PTS, SKIP_DTS, SKIP_DTS2, SKIP_DTS3, SKIP_DTS4,
                 END_OF_DTS, SKIPPING_HEADER, READING_PTS_BITS, GET_HEADER_LENGTH,
                 GET_PTS_MPEG2, PTS22, PTS23, PTS24, FIFTH_BYTE_OF_MPEG2_PTS, HAS_PTS };

#define IDLE_AUDIO 0
#define SAT_AUDIO  1
#define OTV_AUDIO  2


#define DEFAULT_RATE 10

extern LPMPG_ADDRESS lpAudReadPtr;
extern LPMPG_ADDRESS lpAudWritePtr;


extern bool LoadMicrocode(bool, void *, void *);
extern u_int32 Audio_Code_Start[];
extern u_int32 Audio_Code_End[];
#ifdef DRIVER_INCL_AC3
extern u_int32 AC3_Code_Start[];
extern u_int32 AC3_Code_End[];
#endif

extern int copy_data( farcir_q *, int, bool );
extern int handle_PES( farcir_q *, PES_global_data *, bool );
extern int pack_and_copy_data( farcir_q * );  /* TODO  write this function */

extern void aud_before_reset( void );
extern void aud_after_reset( void );
#endif /*_ AUDIO_H_ */
/****************************************************************************
$Log: 
 12   mpeg      1.11        6/23/03 7:24:42 PM     Senthil Veluswamy SCR(s) 
       6641 6642 6643 :
       Added interfaces to get/set rf modulator and audio decoder volume.
       
 11   mpeg      1.10        2/13/03 11:14:44 AM    Matt Korte      SCR(s) 5479 
       :
       Removed old header reference
       
 10   mpeg      1.9         2/5/03 12:38:46 PM     Miles Bintz     SCR(s) 5227 
       :
       Removed ^Z character from end of file
       
       
 9    mpeg      1.8         1/14/03 2:20:12 PM     Craig Dry       SCR(s) 5242 
       :
       Move PES info from aud_api.c to audio.h
       
 8    mpeg      1.7         4/6/00 6:57:10 PM      Amy Pratt       removed 
       inclusion of "opentvx.h" for non-opentv builds
       
 7    mpeg      1.6         3/20/00 1:36:04 PM     Amy Pratt       changed 
       pts_array[] from type char to type unsigned char.  This
       change requires a new avutil.c, rev 1.29
       
 6    mpeg      1.5         2/2/00 6:53:14 PM      Amy Pratt       Moved AC-3 
       microcode to its own library.
       
 5    mpeg      1.4         1/5/00 4:52:40 PM      Amy Pratt       Added 
       declaration of AC3_Code_Start and AC3_Code_End for initial AC3 support.
       
 4    mpeg      1.3         7/29/99 10:12:34 AM    Ismail Mustafa  Added 
       PesLenTBD and InitialPacketLen to PES_global_data. The latter
       is for debugging purposes only.
       
 3    mpeg      1.2         5/26/99 6:15:00 PM     Dave Wilson     Added 
       prototypes for set and get volume APIs.
       
 2    mpeg      1.1         4/15/99 4:34:20 PM     Amy Pratt       Added two 
       new functions, aud_before_reset and aud_after_reset
       that must be called before and after an MPEG reset.
       
 1    mpeg      1.0         1/27/99 4:04:04 PM     Steve Glennon   
$
 * 
 *    Rev 1.11   23 Jun 2003 18:24:42   velusws
 * SCR(s) 6641 6642 6643 :
 * Added interfaces to get/set rf modulator and audio decoder volume.
 * 
 *    Rev 1.10   13 Feb 2003 11:14:44   kortemw
 * SCR(s) 5479 :
 * Removed old header reference
 * 
 *    Rev 1.9   05 Feb 2003 12:38:46   bintzmf
 * SCR(s) 5227 :
 * Removed ^Z character from end of file
 * 
 * 
 *    Rev 1.8   14 Jan 2003 14:20:12   dryd
 * SCR(s) 5242 :
 * Move PES info from aud_api.c to audio.h
 * 
 *    Rev 1.7   06 Apr 2000 17:57:10   prattac
 * removed inclusion of "opentvx.h" for non-opentv builds
 * 
 *    Rev 1.6   20 Mar 2000 13:36:04   prattac
 * changed pts_array[] from type char to type unsigned char.  This
 * change requires a new avutil.c, rev 1.29
 * 
 *    Rev 1.5   02 Feb 2000 18:53:14   prattac
 * Moved AC-3 microcode to its own library.
 * 
 *    Rev 1.4   05 Jan 2000 16:52:40   prattac
 * Added declaration of AC3_Code_Start and AC3_Code_End for initial AC3 support.
 * 
 *    Rev 1.3   29 Jul 1999 09:12:34   mustafa
 * Added PesLenTBD and InitialPacketLen to PES_global_data. The latter
 * is for debugging purposes only.
 *
 *    Rev 1.2   26 May 1999 17:15:00   dawilson
 * Added prototypes for set and get volume APIs.
 *
 *    Rev 1.1   15 Apr 1999 15:34:20   prattac
 * Added two new functions, aud_before_reset and aud_after_reset
 * that must be called before and after an MPEG reset.
 *
 *    Rev 1.0   27 Jan 1999 16:04:04   glennon
 * Initial revision.
 *
 *    Rev 1.12   27 Jan 1999 15:38:32   glennon
 * Explicitly put log info into header
 *
 *    Rev 1.11   27 Jan 1999 15:36:12   glennon
 * Added PVCS Log to header so we have change history when it moves to global
 * include directory
 *
 * Rev 1.10
 * Checked in:     11 Dec 1998 10:04:16
 * Last modified:  10 Dec 1998 15:08:42
 * Author id: mustafa     lines deleted/added/moved: 11/12/0
 * Changed IDLE to IDLE_AUDIO to avoid conflicts.
 * -----------------------------------
 * Rev 1.9
 * Checked in:     11 Dec 1998 02:35:12
 * Last modified:  11 Dec 1998 02:18:38
 * Author id: prattac     lines deleted/added/moved: 0/4/0
 * Changes to help demux enable/disable audio PID at appropriate times.
 * -----------------------------------
 * Rev 1.8
 * Checked in:     29 Apr 1998 15:21:20
 * Last modified:  24 Apr 1998 13:51:36
 * Author id: prattac     lines deleted/added/moved: 15/12/0
 * Split audio.c into two files, audio.c for OpenTV functions and
 * avutil.c for audio/video utility functions.
 * -----------------------------------
 * Rev 1.7
 * Checked in:     21 Apr 1998 18:27:46
 * Last modified:  21 Apr 1998 15:07:22
 * Author id: prattac     lines deleted/added/moved: 0/2/0
 * Added code to flush audio encoded buffer at audio_report_data_empty()
 * -----------------------------------
 * Rev 1.6
 * Checked in:     17 Apr 1998 11:36:52
 * Last modified:  17 Apr 1998 11:32:52
 * Author id: prattac     lines deleted/added/moved: 0/3/0
 * Added include for hwconfig.h
 * -----------------------------------
 * Rev 1.5
 * Checked in:     27 Mar 1998 15:03:58
 * Last modified:  27 Mar 1998 15:03:18
 * Author id: prattac     lines deleted/added/moved: 1/5/0
 * Allocate and assign arm audio buffers
 * -----------------------------------
 * Rev 1.4
 * Checked in:     26 Mar 1998 17:55:10
 * Last modified:  26 Mar 1998 17:54:10
 * Author id: prattac     lines deleted/added/moved: 1/1/0
 * Use opentvx.h.
 * -----------------------------------
 * Rev 1.3
 * Checked in:     24 Mar 1998 11:43:00
 * Last modified:  19 Mar 1998 16:57:54
 * Author id: prattac     lines deleted/added/moved: 0/3/0
 * Now using globals.h.  Also fixed some bugs in handle_PES().
 * -----------------------------------
 * Rev 1.2
 * Checked in:     19 Mar 1998 12:31:20
 * Last modified:  19 Mar 1998 12:29:56
 * Author id: prattac     lines deleted/added/moved: 6/13/0
 * Changes to make handle_PES() generic enough to handle both audio
 * and video PES headers.
 * -----------------------------------
 * Rev 1.1
 * Checked in:     18 Mar 1998 19:15:02
 * Last modified:  18 Mar 1998 19:14:04
 * Author id: prattac     lines deleted/added/moved: 3/3/0
 * Changes after design review
 * -----------------------------------
 * Rev 1.0
 * Checked in:     16 Mar 1998 13:57:34
 * Last modified:  13 Mar 1998 11:55:04
 * Author id: prattac     lines deleted/added/moved: 0/0/0
 * Initial revision.
 ****************************************************************************/

