/****************************************************************************/
/*                   CONEXANT PROPRIETARY AND CONFIDENTIAL                  */
/*                       SOFTWARE FILE/MODULE HEADER                        */
/*          Conexant Systems Inc. (c) 1998, 1999, 2000, 2001, 2002          */
/*                              Austin, TX                                  */
/*                         All Rights Reserved                              */
/****************************************************************************/
/*
 * Filename:        avutcomn.c
 *
 *
 * Description:     Source code for utility functions used by the
 *                  generic Audio and Video drivers
 *
 *
 * Author:          Amy Pratt
 *
 ****************************************************************************/
/* $Header: avutcomn.c, 17, 9/2/03 7:09:12 PM, Joe Kroesche$
 ****************************************************************************/

/** include files **/

#include "stbcfg.h"
#include <stdio.h>
#include <string.h>


#ifdef OPENTV
#include "opentvx.h"
#else
#include "basetype.h"
#include "cnxtv.h"
#endif

#include "audio.h"
#include "kal.h"
#include "retcodes.h"
#include "globals.h"

/** local definitions **/

#define SWAP_COPY
#define AC3_ALIGNED

/* if the audio/video encoded buffer write pointer gets close
   to overlapping the encoded buffer read pointer, it will cause
   an interrupt and a reset.  We want to avoid this when placing
   host data into the encoded buffers. For audio, the hardware
   should generate this interrupt/reset if the write pointer is
   within 2K bytes of the read pointer.  However, while running
   the OTV rooster data through, the interrupt happens before
   reaching this point.  I have set this value to 3K bytes, and
   things work for now.
*/
#define AUD_OVERLAP_PROTECTION 0xc80

/* default settings */

/** external functions **/

/** external data **/
extern u_int32 num_vid_leftovers;
char leftover[2][4];
bool FirstPTS;
bool LastAudPTSMatured;
bool LastVidPTSMatured;

static LPREG lpSTCHi = (LPREG)MPG_STC_HI_REG;
static LPREG lpSTCLo = (LPREG)MPG_STC_LO_REG;
extern u_int32 gDemuxInstance;
extern LPMPG_ADDRESS lpAudWritePtr;

/** internal functions **/

/** public data **/
u_int32 num_aud_leftovers = 0;

/** private data **/

/** public functions **/

/** private functions **/

void send_PTS(unsigned char [], bool);
int BytesUntilStartCode( farcir_q *q, int Len, char *end_of_queue, bool *PrefixFound);
/*************** handle_PES **************************/

#define INCREMENT_OR_RETURN  \
             {  ptr++; \
               if (ptr == end_of_queue) ptr = (char *)q->begin; \
               bytes_consumed++; \
               if (bytes_consumed == limit) return bytes_consumed;}

/*
 *   description:
          * This function takes a Packetized Elementary Stream, strips
            the Packet Header information off, and passes the remaining
            elementary stream to the hardware encoded buffer.  It must
            handle both MPEG-I and MPEG-II packet headers.  It must be
            able to start and stop parsing data at any point in the
            header (depending on what data is in the queue).
     input:
          * pointer to far circular queue q, which contains audio
            or video packetized elementary stream data.
          * pointer to perm_data, a data structure containing
            information that needs to be retained between calls to
            handle_PES()
          * a flag that indicates whether or not PTS data should
            be sent to the hardware
     returns:
          * the number of bytes processed by this function and
            by copy_data() (called by this function)
     side effects:
          * The PTS value, if present, will be extracted from the
            PES header and stored in perm_data->pts_array[].
          * If a PTS/PCR value is present, and hardware synchronization
            is enabled, the PTS value will be written to memory along
            with the address where the corresponding data will be placed
            in memory.
          * Other than the PTS, any header data will be skipped.
          * If the end of a header is reached, q->out will be
            changed to reflect start of the MPEG data.  Then copy_data()
            will be called to send the MPEG data to the audio or video
            encoded buffer.
          * The fields where_in_PES, progress, bytes_left_in_packet,
            bytes_left_in_header, and pts_present are used to determine
            where to begin parsing, and to send the correct amount
            of data to the hardware buffer.  At audio_play() or video_play()
            they should be initialized as follows:
                where_in_PES = FIND_PREFIX
                progress = LAST_BYTE_NONZERO
                bytes_left_in_packet = 0
                bytes_left_in_header = 0
                pts_present = 0

     notes:
         * This function has been tested with a driver-level test.
         * It really doesn't make much sense to send a Packetized
           Elementary Stream without PTSes (why not just use an ES?)
         * The use of PTSes with host audio will not occur until
           audio_pause() and audio_resume() are supported/implemented/required.

 */

int handle_PES( farcir_q *q, PES_global_data *perm_data, bool isAudio )
{
    HW_DWORD     tmpCtrl1;
   char * ptr;           /* pointer to current byte in q */
   u_int32 packet_len;
   char * end_of_queue;  /* pointer to last byte in the array that forms q */
   char * start_of_ES;   /* pointer to the first byte in the Elementary Stream */
   int bytes_consumed = 0;  /* number of bytes handled by driver */
   int limit;            /* number of bytes in queue */
   u_int8 stream_id;
   ptr = (char *)q->begin + q->out;
   limit = (q->out > q->in) ? (q->size + q->in - q->out) : (q->in - q->out);
   end_of_queue = (char *)q->begin + q->size;

   /* MKI assert(isAudio == TRUE);  now supports video too */

   switch ( perm_data->where_in_PES ) {
     case FIND_PREFIX:
       while ( perm_data->progress != GET_PACKET_LENGTH1 ) {
         switch ( perm_data->progress ) {
           case LAST_BYTE_NONZERO :
             while( *ptr != 0 )
               INCREMENT_OR_RETURN;
             perm_data->progress = LAST_BYTE_ZERO;
             /* fall through */
           case LAST_BYTE_ZERO :
             if (*ptr != 0) {
               /* go back to beginning of search */
               perm_data->progress = LAST_BYTE_NONZERO;
               INCREMENT_OR_RETURN;
               continue;
             } else {
               perm_data->progress = LAST_TWO_BYTES_ZERO;
               INCREMENT_OR_RETURN;
             }
             /* fall through */
           case LAST_TWO_BYTES_ZERO:
             while (*ptr == 0) { /* skip extra zeros */
               INCREMENT_OR_RETURN;
             }
             if (*ptr != 1) {
               perm_data->progress = LAST_BYTE_NONZERO;
               INCREMENT_OR_RETURN;
               continue;
             } else {
               perm_data->progress = SKIP_STREAM_ID;
               INCREMENT_OR_RETURN;
               break;
             }
           /* MKI , for video we must check stream ID as part of PREFIX.*/
           case SKIP_STREAM_ID:
            stream_id = (u_int8) *((u_int8 *)ptr);
            if (!isAudio){
                if ( stream_id < 0xE0 || stream_id > 0xEF ){   /* E0 to EF are valid video stream ID */
                   perm_data->progress = LAST_BYTE_NONZERO;
                   INCREMENT_OR_RETURN;
                   continue;
                }
                else{
                   perm_data->progress = GET_PACKET_LENGTH1;
                   perm_data->where_in_PES = PARSING_HEADER;
                   INCREMENT_OR_RETURN;
                }
            }
            else{
  	        if ( (stream_id < 0xC0 || stream_id > 0xDF) &&
		     (stream_id != 0xBD) ) {   /* BD and (C0 to DF) are valid audio stream ID */
		   perm_data->progress = LAST_BYTE_NONZERO;
                   INCREMENT_OR_RETURN;
                   continue;
                }
                else{
                   perm_data->progress = GET_PACKET_LENGTH1;
                   perm_data->where_in_PES = PARSING_HEADER;
                   INCREMENT_OR_RETURN;
                }
            }
           default:
             break;
         }
       }
       /* fall through */
/* MKI this is now handled above.   perm_data->progress = SKIP_STREAM_ID; */
     case PARSING_HEADER:
       do {
         switch (perm_data->progress) {
         case GET_PACKET_LENGTH1:
            packet_len = (u_int32) *( (u_int8 *)ptr);
            packet_len <<= 8;
             perm_data->bytes_left_in_packet = (int) packet_len;
             ptr++;  /* skip stuffing bytes */
             if (ptr == end_of_queue) ptr = (char *)q->begin;
             bytes_consumed++;
             if (bytes_consumed == limit) {
                 perm_data->progress = GET_PACKET_LENGTH2;
                 return bytes_consumed;
             }
         case GET_PACKET_LENGTH2:
            packet_len = (u_int32) *( (u_int8 *)ptr);
            packet_len |= (u_int32) perm_data->bytes_left_in_packet;
             perm_data->bytes_left_in_packet = (int) packet_len;
             perm_data->InitialPacketLen = perm_data->bytes_left_in_packet;
             if (perm_data->bytes_left_in_packet == 0){     /* MKI video stream unspecified length */
                perm_data->PesLenTBD = TRUE;                /* MKI figure length out later */
             }
             else
                perm_data->PesLenTBD = FALSE;
             #ifdef DEBUG
             if (perm_data->bytes_left_in_packet < 0 && isAudio)
                trace_new(TRACE_LEVEL_ALWAYS | TRACE_AUD,"handle_pes ERROR:bytes_left_in_packet < 0, Audio\n");
             if (perm_data->bytes_left_in_packet < 0 && !isAudio)
                trace_new(TRACE_LEVEL_ALWAYS | TRACE_AUD,"handle_pes ERROR:bytes_left_in_packet < 0, Video\n");
             #endif
             ptr++;  /* skip stuffing bytes */
             if (ptr == end_of_queue) ptr = (char *)q->begin;
             bytes_consumed++;
             if (bytes_consumed == limit) {
                 perm_data->progress = GET_MPEG1_MPEG2;
                 return bytes_consumed;
             }
         case GET_MPEG1_MPEG2:
             /* is this MPEG-1 or MPEG-2 systems ? */
             if ((*ptr & 0xC0) == 0x80) { /* first two bits '10' */
                 /* It's MPEG-2 */
                 perm_data->progress = SKIPPING_DATA_BITS;
                 break;
             } /* else
                * It's MPEG-1
                * progress = SKIPPING_STUFFING_BYTES; */
             /* fall through */
         case SKIPPING_STUFFING_BYTES:
             while (*ptr == (char)0xFF) {
                 perm_data->bytes_left_in_packet--;
                 ptr++;  /* skip stuffing bytes */
                 if (ptr == end_of_queue) ptr = (char *)q->begin;
                 bytes_consumed++;
                 if (bytes_consumed == limit) {
                     perm_data->progress = SKIPPING_STUFFING_BYTES;
                     return bytes_consumed;
                 }
             }
             if ((*ptr & 0xC0) == 0x40) { /* skip STD info */
                 perm_data->bytes_left_in_packet--;
                 ptr++;
                 if (ptr == end_of_queue) ptr = (char *)q->begin;
                 bytes_consumed++;
                 if (bytes_consumed == limit) {
                     perm_data->progress = SKIP_SECOND_BYTE_OF_STD;
                     return bytes_consumed;
                 }
             } else {
                 perm_data->progress = HAS_PTS;
                 continue;
             }
             /* fall through */
         case SKIP_SECOND_BYTE_OF_STD:
             perm_data->bytes_left_in_packet--;
             ptr++;
             if(ptr == end_of_queue) ptr = (char *) q->begin;
             bytes_consumed++;
             perm_data->progress = HAS_PTS;
             if (bytes_consumed == limit) {
                 return bytes_consumed;
             }
             /* fall through */
         case HAS_PTS:
             if ((*ptr & 0xF0) == 0x20 || (*ptr & 0xF0) == 0x30) {
                 perm_data->progress = GET_PTS_MPEG1;
                 continue;
             } else {
                 perm_data->bytes_left_in_packet--;
                 ptr++;
                 if (ptr == end_of_queue) ptr = (char *) q->begin;
                 bytes_consumed++;
                 perm_data->where_in_PES = SENDING_DATA;
                 if (bytes_consumed == limit) return bytes_consumed;
                 continue;
             }
         case GET_PTS_MPEG1:
         case GET_PTS_MPEG1+1:
         case GET_PTS_MPEG1+2:
         case GET_PTS_MPEG1+3:
         case FIFTH_BYTE_OF_PTS:
             /* get PTS */
             for ( ; perm_data->progress<= FIFTH_BYTE_OF_PTS; perm_data->progress++) {
                 perm_data->pts_array[perm_data->progress-GET_PTS_MPEG1] = *ptr;
                 perm_data->bytes_left_in_packet--;
                 ptr++;
                 if (ptr == end_of_queue) ptr = (char *)q->begin;
                 bytes_consumed++;
                 if (bytes_consumed == limit) return bytes_consumed;
             }
             tmpCtrl1 = *glpCtrl1;
             if (CNXT_GET_VAL(&tmpCtrl1, MPG_CONTROL1_ENABLEAUDSYNC_MASK) == 1) 
	         send_PTS(perm_data->pts_array, !isAudio);
             if ((perm_data->pts_array[0] & 0xF0) != 0x30) { /* no DTS */
                  /* Update q->out before passing q to copy_data.
                   * This simplifies copy_data().  audio_data_ready() will
                   * make sure q->out is set to the correct value before
                   * returning.*/
                 q->out = ptr - (char *)q->begin;
                 perm_data->where_in_PES = SENDING_DATA;
                 continue;
             } else perm_data->progress = SKIP_DTS;
             /* fall through */
         case SKIP_DTS:
         case SKIP_DTS+1:
         case SKIP_DTS+2:
         case SKIP_DTS+3:
         case END_OF_DTS:
             /* skip DTS */
             for ( ; perm_data->progress <= END_OF_DTS; perm_data->progress++) {
                 perm_data->bytes_left_in_packet--;
                 ptr++;
                 if (ptr == end_of_queue) ptr = (char *) q->begin;
                 bytes_consumed++;
                 if (bytes_consumed == limit) return bytes_consumed;
             }

                /* Update q->out before passing q to copy_data.
                 * This simplifies copy_data().  audio_data_ready() will
                 * make sure q->out is set to the correct value before
                 * returning.*/
             q->out = ptr - (char *)q->begin;
             perm_data->where_in_PES = SENDING_DATA;
             continue;
         case SKIPPING_DATA_BITS:
             ptr++;
             perm_data->bytes_left_in_packet--;
             if (ptr == end_of_queue) ptr = (char *) q->begin;
             bytes_consumed++;
             if (bytes_consumed == limit) {
                 perm_data->progress = READING_PTS_BITS;
                 return bytes_consumed;
             }
         case READING_PTS_BITS:
             perm_data->pts_present = ((1<<7) & *((u_int8 *) ptr)) && 1;
             ptr++;
             perm_data->bytes_left_in_packet--;

             if (ptr == end_of_queue) ptr = (char *) q->begin;
             bytes_consumed++;
             if (bytes_consumed == limit) {
                 perm_data->progress = GET_HEADER_LENGTH;
                 return bytes_consumed;
             }
             /* fall through */
         case GET_HEADER_LENGTH:
             perm_data->bytes_left_in_header = *((u_int8 *)ptr);
             ptr++;
             perm_data->bytes_left_in_packet--;

             if (ptr == end_of_queue) ptr = (char *) q->begin;
             bytes_consumed++;
             if (bytes_consumed == limit) {
                 if (perm_data->pts_present) perm_data->progress = GET_PTS_MPEG2;
                 else perm_data->progress = SKIPPING_HEADER;
                 return bytes_consumed;
             }

             if (!perm_data->pts_present) {
                 perm_data->progress = SKIPPING_HEADER;
                 continue;
             }
             perm_data->progress = GET_PTS_MPEG2;
             /* fall through */
         case GET_PTS_MPEG2:
         case GET_PTS_MPEG2+1:
         case GET_PTS_MPEG2+2:
         case GET_PTS_MPEG2+3:
         case FIFTH_BYTE_OF_MPEG2_PTS:
             /* get PTS */
             for ( ; perm_data->progress<= FIFTH_BYTE_OF_MPEG2_PTS;
                                            perm_data->progress++) {
                 perm_data->pts_array[perm_data->progress-GET_PTS_MPEG2] = *((u_int8 *)ptr);
                 perm_data->bytes_left_in_packet--;
                 perm_data->bytes_left_in_header--;
                 ptr++;
                 if (ptr == end_of_queue) ptr = (char *)q->begin;
                 bytes_consumed++;
                 if (bytes_consumed == limit) return bytes_consumed;
             }
             tmpCtrl1 = *glpCtrl1;
             if (CNXT_GET_VAL(&tmpCtrl1, MPG_CONTROL1_ENABLEAUDSYNC_MASK) == 1) 
	         send_PTS(perm_data->pts_array, !isAudio);
             perm_data->where_in_PES = SENDING_DATA;
         case SKIPPING_HEADER:
             if ((bytes_consumed + perm_data->bytes_left_in_header) >= limit) {
                 perm_data->bytes_left_in_packet -= (limit - bytes_consumed);
                 perm_data->bytes_left_in_header -= (limit - bytes_consumed);
                 if (perm_data->bytes_left_in_header == 0)
                     perm_data->where_in_PES = SENDING_DATA;
                 bytes_consumed = limit;
                 return bytes_consumed;
             }
             bytes_consumed += perm_data->bytes_left_in_header;
             perm_data->where_in_PES = SENDING_DATA;
             start_of_ES = ptr + perm_data->bytes_left_in_header;
             if ( start_of_ES > end_of_queue)
                 start_of_ES -= q->size;

               /* Update q->out before passing q to copy_data.
                * This simplifies copy_data().  audio_data_ready() will
                * make sure q->out is set to the correct value before
                * returning.*/
             q->out = start_of_ES - (char *)q->begin;
             perm_data->bytes_left_in_packet -= perm_data->bytes_left_in_header;
         default:
             /* error */
             break;
         }
       } while (perm_data->where_in_PES != SENDING_DATA);
       /* fall through */

     case SENDING_DATA:
         {
         int bytes_copied, bytes_left_in_queue;
         bool PrefixFound = FALSE;
         bytes_left_in_queue = (q->size + q->in - q->out) % q->size;

         /* MKI. If length is to be determined, then find the number of bytes
            until the next start code prefix.
         */
         if (perm_data->PesLenTBD){
            /* save_bytes = perm_data->bytes_left_in_packet; */
            perm_data->bytes_left_in_packet = BytesUntilStartCode(q, bytes_left_in_queue, end_of_queue, &PrefixFound);
            if (PrefixFound){
                perm_data->PesLenTBD = FALSE; /* the length is now known so we should start at the top now */
            }
         }
         bytes_copied = copy_data( q, min(perm_data->bytes_left_in_packet,
                                   bytes_left_in_queue), isAudio);
         bytes_consumed += bytes_copied;
         perm_data->bytes_left_in_packet -= bytes_copied;
         if (perm_data->bytes_left_in_packet == 0 && perm_data->PesLenTBD == FALSE) {
             perm_data->where_in_PES = FIND_PREFIX;
             perm_data->progress = LAST_BYTE_NONZERO;
             /* FUTURE update ptr and continue */
         }
         }
         return bytes_consumed;
         break;
     default:
         break;
    }

return bytes_consumed;
}


/******************************MKI*******************************************/
/* returns number of bytes to copy. Which is either all data or until start */
/* code prefix is found.                                                    */
/*                                                                          */
/* input : far circular queue q                                             */
/*         # of bytes in q                                                  */
/*         pointer to end of queue                                          */
/*         address of flag to be set, indicating whether prefix was found or*/
/*         not.                                                             */
/****************************************************************************/
int BytesUntilStartCode( farcir_q *q, int Len, char *end_of_queue, bool *PrefixFound)
{
    u_int32 NumberOfZeros = 0;
    u_int32 SearchState = 0;
    char *pData =  (char *) q->begin + q->out;
    int i;
    int RetCount = 0;

    for (i = 0; i < Len; ++i){
        switch (SearchState){
            case 0x00:
                   switch (*pData){
                        case 0:
                           ++NumberOfZeros;
                           break;
                        case 0x01:
                           if (NumberOfZeros >= 2){
                              SearchState = 1;
                           }
                           NumberOfZeros = 0;
                           break;
                        default:
                           NumberOfZeros = 0;
                           break;
                   }
                   break;
            case 0x01:
                   SearchState = 0;
                   if ( (u_int8) *pData >= 0xE0 && (u_int8) *pData <= 0xEF){
                        *PrefixFound = TRUE;
                        return i - 3;
                   }
                   break;
            default:
                   break;
        }
        ++pData;
        if (pData >= end_of_queue)
            pData = (char *) q->begin;
    }

    switch (SearchState){
        case 0x00:
            if (NumberOfZeros > 2)
                NumberOfZeros = 2;
            RetCount = i - NumberOfZeros;
            break;
        case 0x01:
            RetCount = i - 3;
            break;
        default:
            break;
    }

    *PrefixFound = FALSE;
    return RetCount;
}


#ifdef PES_TEST
int copy_data( farcir_q *q, int bytes_to_copy, bool isAudio )
{
    int fd, bytes_written, num_to_copy;

    fd = open("output.mpg", O_WRONLY|O_APPEND|O_BINARY);
    if (fd <= 0) {
        printf("Cannot open output.mpg\n");
        return 0;
    }

    num_to_copy = min(bytes_to_copy, (q->size - q->out));
    bytes_written = write(fd, ((char *)q->begin + q->out), num_to_copy);

    close(fd);


    return bytes_written;
}
#else
/***************** copy data ************/
/* returns number of bytes copied       */
/* input : far circular queue q         */
/*         # of bytes in q to be copied */
/* calling function will take care of   */
/* q->out, but this function needs to   */
/* move write_ptr                       */
/****************************************/
int copy_data( farcir_q *q, int bytes_to_copy, bool isAudio )
{
    /* bytes_in_queue = min(bytes_left_in_packet, bytes_left_in_queue) */

    char * read_ptr;  /* hardware encoded audio/video buffer read pointer */
    char * write_ptr; /* hardware encoded audio/video buffer write pointer */

    char **read_ptr_reg, **write_ptr_reg;
    char * hwbuf_encoded_byte_addr;
    int hwbuf_encoded_size;
    u_int32 *num_leftovers_ptr;
    int aud_vid_index;
    int bytes_to_queue_wrap, bytes_to_buffer_wrap, bytes_to_buffer_full;
    int bytes_copied = 0;
#ifdef SWAP_COPY
    int i;
#endif
    int   tmp;
    
    int write_ptr_wrap;

    if (isAudio) {
        read_ptr_reg = (char **)DPS_AUDIO_READ_PTR_EX(gDemuxInstance);
        write_ptr_reg = (char **)DPS_AUDIO_WRITE_PTR_EX(gDemuxInstance);
        hwbuf_encoded_byte_addr = (char *)HWBUF_ENCAUD_ADDR;
        hwbuf_encoded_size = HWBUF_ENCAUD_SIZE;
        aud_vid_index = 0;
        num_leftovers_ptr = &num_aud_leftovers;
    } else {
        read_ptr_reg = (char **)DPS_VIDEO_READ_PTR_EX(gDemuxInstance);
        write_ptr_reg = (char **)DPS_VIDEO_WRITE_PTR_EX(gDemuxInstance);
        hwbuf_encoded_byte_addr = (char *)HWBUF_ENCAUD_ADDR;
        hwbuf_encoded_byte_addr = (char *)HWBUF_ENCVID_ADDR;
        hwbuf_encoded_size = HWBUF_ENCVID_SIZE;
        aud_vid_index = 1;
        num_leftovers_ptr = &num_vid_leftovers;
    }

    read_ptr = (char *)((u_int32)*read_ptr_reg & ~0x80000000);
    write_ptr = (char *)((u_int32)*write_ptr_reg & ~0x80000000);
    write_ptr_wrap = ((u_int32)*write_ptr_reg & 0x80000000) >> 31;


    bytes_to_queue_wrap = q->size - q->out;
    bytes_to_buffer_wrap = (char *)hwbuf_encoded_byte_addr + hwbuf_encoded_size
                            - write_ptr;
    bytes_to_buffer_full = (read_ptr > write_ptr)
                 ? (read_ptr - write_ptr - AUD_OVERLAP_PROTECTION)
                 : (read_ptr + hwbuf_encoded_size - write_ptr - AUD_OVERLAP_PROTECTION);
        /* don't let write pointer get too close to read pointer */

    if (bytes_to_buffer_full <= 0) return 0;

    trace_new(TRACE_LEVEL_1 | TRACE_AUD, "Audio read pointer is %x.\n",read_ptr);
    trace_new(TRACE_LEVEL_1 | TRACE_AUD, "Audio write pointer is %x.\n", write_ptr);

    trace_new(TRACE_LEVEL_1 | TRACE_AUD, "bytes to queue wrap:  %x\n", bytes_to_queue_wrap);
    trace_new(TRACE_LEVEL_1 | TRACE_AUD, "bytes to buffer wrap: %x\n", bytes_to_buffer_wrap);
    trace_new(TRACE_LEVEL_1 | TRACE_AUD, "bytes to buffer full: %x\n", bytes_to_buffer_full);


#ifdef SWAP_COPY
    /* make sure write_ptr is on a 32bit boundary */
    /* First copy any leftovers */
    if (*num_leftovers_ptr != 0) {
        if (*num_leftovers_ptr + bytes_to_copy >= 4) {
            for (i = 3; i>= (int)(4-*num_leftovers_ptr); i--)
            {
               *(char *)(((u_int32)write_ptr+i) | (BUFF_BASE | NCR_BASE)) = leftover[aud_vid_index][3-i];
            }
            for (; i>=0; i--) {
               *(char *)(((u_int32)write_ptr+i) | (BUFF_BASE | NCR_BASE)) = *((char *)q->begin+q->out);
               q->out++;
               if (q->out == q->size) q->out = 0;
               bytes_copied++;
               bytes_to_copy--;
            }
            if (bytes_to_buffer_full < 8) bytes_to_copy = 0;
            write_ptr += 4;
            if (write_ptr >= hwbuf_encoded_byte_addr + hwbuf_encoded_size) {
                write_ptr = hwbuf_encoded_byte_addr; /* wrap to start of encoded buffer */
                /* toggle wrap bit */
                write_ptr_wrap = !write_ptr_wrap;
            }
            *num_leftovers_ptr = 0;
        } else { /* not enough new data to put in hardware buffer.*/
                 /* instead copy few bytes to leftover array */
            for (i = *num_leftovers_ptr; i< (int)(*num_leftovers_ptr + bytes_to_copy);
                                         i++, bytes_copied++) {
                leftover[aud_vid_index][i] = *((char *)q->begin + q->out);
                q->out++;
                if (q->out == q->size) q->out = 0;
            }
            *num_leftovers_ptr = i;
            bytes_to_copy = 0;
        }
    }

    bytes_to_copy = min(bytes_to_copy, bytes_to_buffer_full);

    /* copy data 4 bytes at a time, swapping order */
    while(bytes_to_copy > 3) {
        for (i = 3; i>= 0; i--) {
           *(char *)(((u_int32)write_ptr+i) | (BUFF_BASE | NCR_BASE)) = *((char *)q->begin+q->out);
           q->out++;
           if (q->out == q->size)
               q->out = 0;  /* wrap to beginning of circular queue */
        }
        write_ptr += 4;
        if (write_ptr >= hwbuf_encoded_byte_addr + hwbuf_encoded_size) {
            write_ptr = hwbuf_encoded_byte_addr; /* wrap to start of encoded buffer */
            /* toggle wrap bit */
            write_ptr_wrap = !write_ptr_wrap;
        }
        bytes_copied  += 4;
        bytes_to_copy -= 4;
        if (write_ptr == (char *)(read_ptr - 4)) {
            break;  /* hardware buffer full */
        }
    }

    /* if there are less than 4 bytes left, copy them to the leftover array */
    if ((bytes_to_copy > 0) && (bytes_to_copy < 4)) {
        for (tmp=0 ; tmp < bytes_to_copy; bytes_copied++, tmp++) {
            leftover[aud_vid_index][tmp] = *((char *)q->begin+q->out);
            q->out++;
            if (q->out == q->size) q->out = 0;
        }
        *num_leftovers_ptr = tmp;
    }

#else
    /* *write_ptr_reg is quad-word aligned, but we may
       have copied more bytes to the buffer           */
    write_ptr += *num_leftovers_ptr;

    bytes_to_copy = min(bytes_to_copy, bytes_to_buffer_full);

    for (; bytes_copied < bytes_to_copy; bytes_copied++) {
        *(char *)((u_int32)write_ptr|BUFF_BASE|BSWAP_OFFSET|NCR_BASE) =
                       *((char *)q->begin+q->out);
        write_ptr++;
        q->out++;
        if (write_ptr == hwbuf_encoded_byte_addr + hwbuf_encoded_size) {
            write_ptr = hwbuf_encoded_byte_addr;
            /* toggle wrap bit */
            write_ptr_wrap = !write_ptr_wrap;
        }
        if (q->out == q->size) q->out = 0;
        if (write_ptr == (char *)(read_ptr - 8))
            break;  /* hardware buffer full */
    }

    /* did we copy any bytes past the last quad-word boundary? */
    *num_leftovers_ptr = (bytes_copied + *num_leftovers_ptr) % 4;
    /* make sure *write_ptr_reg is quad-word aligned */
    write_ptr -= *num_leftovers_ptr;


#endif /* SWAP_COPY */

    *write_ptr_reg = write_ptr + (write_ptr_wrap << 31);

    trace_new(TRACE_LEVEL_1 | TRACE_AUD, "*num_leftovers_ptr:  %x \n", *num_leftovers_ptr);
    trace_new(TRACE_LEVEL_1 | TRACE_AUD, "bytes copied = %x.\n", bytes_copied);
    trace_new(TRACE_LEVEL_1 | TRACE_AUD, "Audio write pointer is %x. (after copy)\n", *write_ptr_reg);
    return bytes_copied;
}

#ifdef AC3_ALIGNED
bool ac3_present(char *c, u_int32 nchar, u_int32 *offset)
{
   u_int32 nchar_left = nchar;

   while (nchar_left >= 6)
   {
      if ((c[0] == 0x0B) && (c[1] == 0x77) && 
          ((c[4] & 0xC0) == 0x00) && ((c[4] & 0x3F) <= 37) && 
          ((c[5] & 0xF8) <= 0x40) )
      {
         *offset = nchar - nchar_left;
         return TRUE;
      }
      c++;
      nchar_left--;
   }
   return FALSE;
}

int copy_data_ac3_aligned( farcir_q *q, int bytes_to_copy, bool isAudio )
{
    /* bytes_in_queue = min(bytes_left_in_packet, bytes_left_in_queue) */

    char * read_ptr;  /* hardware encoded audio/video buffer read pointer */
    char * write_ptr; /* hardware encoded audio/video buffer write pointer */

    char **read_ptr_reg, **write_ptr_reg;
    char * hwbuf_encoded_byte_addr;
    int hwbuf_encoded_size;
    u_int32 *num_leftovers_ptr;
    int aud_vid_index;
    int bytes_to_buffer_full;
    int bytes_copied = 0;
    int i;
    int   tmp;
    
    int write_ptr_wrap;
    u_int32 ac3_offset;

    read_ptr_reg = (char **)DPS_AUDIO_READ_PTR_EX(gDemuxInstance);
    write_ptr_reg = (char **)DPS_AUDIO_WRITE_PTR_EX(gDemuxInstance);
    hwbuf_encoded_byte_addr = (char *)HWBUF_ENCAUD_ADDR;
    hwbuf_encoded_size = HWBUF_ENCAUD_SIZE;
    aud_vid_index = 0;
    num_leftovers_ptr = &num_aud_leftovers;

    read_ptr = (char *)((u_int32)*read_ptr_reg & ~0x80000000);
    write_ptr = (char *)((u_int32)*write_ptr_reg & ~0x80000000);
    write_ptr_wrap = ((u_int32)*write_ptr_reg & 0x80000000) >> 31;

    bytes_to_buffer_full = (read_ptr > write_ptr)
                 ? (read_ptr - write_ptr - AUD_OVERLAP_PROTECTION)
                 : (read_ptr + hwbuf_encoded_size - write_ptr - AUD_OVERLAP_PROTECTION);
        /* don't let write pointer get too close to read pointer */

    if (bytes_to_buffer_full <= 0) return 0;

    trace_new(TRACE_LEVEL_1 | TRACE_AUD, "Audio read pointer is %x.\n",read_ptr);
    trace_new(TRACE_LEVEL_1 | TRACE_AUD, "Audio write pointer is %x.\n", write_ptr);
    trace_new(TRACE_LEVEL_1 | TRACE_AUD, "bytes to buffer full: %x\n", bytes_to_buffer_full);

    /* Align any AC3 header to an 8-byte boundary */
    if (ac3_present((char *)q->begin+q->out, 
                    min(bytes_to_copy, bytes_to_buffer_full),
                    &ac3_offset))
    {
       if ((((u_int32)write_ptr + *num_leftovers_ptr + ac3_offset) & 7))
          /* We're definitely out of alignment. */
       {
          /* Discard everything before start of new header */
          q->out        += ac3_offset;
          bytes_copied  += ac3_offset;
          bytes_to_copy -= ac3_offset;
          *num_leftovers_ptr = 0;
          
          /* Write pointer is on a boundary of 4.  */
          /* Make sure it is on a boundary of 8 */
          if (((u_int32)write_ptr & 7) != 0)
          {
             write_ptr += 4;
             if (write_ptr >= hwbuf_encoded_byte_addr + hwbuf_encoded_size) 
             {
                write_ptr = hwbuf_encoded_byte_addr; /* wrap to start of encoded buffer */
                /* toggle wrap bit */
                write_ptr_wrap = !write_ptr_wrap;
             }
          }
       }
    }
       
    /* Copy any leftovers */
    if (*num_leftovers_ptr != 0) {
        if (*num_leftovers_ptr + bytes_to_copy >= 4) {
            for (i = 3; i>= (int)(4-*num_leftovers_ptr); i--)
            {
               *(char *)(((u_int32)write_ptr+i) | (BUFF_BASE | NCR_BASE)) = leftover[aud_vid_index][3-i];
            }
            for (; i>=0; i--) {
               *(char *)(((u_int32)write_ptr+i) | (BUFF_BASE | NCR_BASE)) = *((char *)q->begin+q->out);
               q->out++;
               if (q->out == q->size) q->out = 0;
               bytes_copied++;
               bytes_to_copy--;
            }
            if (bytes_to_buffer_full < 8) bytes_to_copy = 0;
            write_ptr += 4;
            if (write_ptr >= hwbuf_encoded_byte_addr + hwbuf_encoded_size) {
                write_ptr = hwbuf_encoded_byte_addr; /* wrap to start of encoded buffer */
                /* toggle wrap bit */
                write_ptr_wrap = !write_ptr_wrap;
            }
            *num_leftovers_ptr = 0;
        } else { /* not enough new data to put in hardware buffer.*/
                 /* instead copy few bytes to leftover array */
            for (i = *num_leftovers_ptr; i< (int)(*num_leftovers_ptr + bytes_to_copy);
                                         i++, bytes_copied++) {
                leftover[aud_vid_index][i] = *((char *)q->begin + q->out);
                q->out++;
                if (q->out == q->size) q->out = 0;
            }
            *num_leftovers_ptr = i;
            bytes_to_copy = 0;
        }
    }

    bytes_to_copy = min(bytes_to_copy, bytes_to_buffer_full);

    /* copy data 4 bytes at a time, swapping order */
    while(bytes_to_copy > 3) {
        for (i = 3; i>= 0; i--) {
           *(char *)(((u_int32)write_ptr+i) | (BUFF_BASE | NCR_BASE)) = *((char *)q->begin+q->out);
           q->out++;
           if (q->out == q->size)
               q->out = 0;  /* wrap to beginning of circular queue */
        }
        write_ptr += 4;
        if (write_ptr >= hwbuf_encoded_byte_addr + hwbuf_encoded_size) {
            write_ptr = hwbuf_encoded_byte_addr; /* wrap to start of encoded buffer */
            /* toggle wrap bit */
            write_ptr_wrap = !write_ptr_wrap;
        }
        bytes_copied  += 4;
        bytes_to_copy -= 4;
        if (write_ptr == (char *)(read_ptr - 4)) {
            break;  /* hardware buffer full */
        }
    }

    /* if there are less than 4 bytes left, copy them to the leftover array */
    if ((bytes_to_copy > 0) && (bytes_to_copy < 4)) {
        for (tmp=0 ; tmp < bytes_to_copy; bytes_copied++, tmp++) {
            leftover[aud_vid_index][tmp] = *((char *)q->begin+q->out);
            q->out++;
            if (q->out == q->size) q->out = 0;
        }
        *num_leftovers_ptr = tmp;
    }

    *write_ptr_reg = write_ptr + (write_ptr_wrap << 31);

    trace_new(TRACE_LEVEL_1 | TRACE_AUD, "*num_leftovers_ptr:  %x \n", *num_leftovers_ptr);
    trace_new(TRACE_LEVEL_1 | TRACE_AUD, "bytes copied = %x.\n", bytes_copied);
    trace_new(TRACE_LEVEL_1 | TRACE_AUD, "Audio write pointer is %x. (after copy)\n", *write_ptr_reg);
    return bytes_copied;
}
#endif /* AC3_ALIGNED */
#endif

#ifdef AC3_ALIGNED
int handle_PES_ac3_aligned( farcir_q *q, PES_global_data *perm_data, bool isAudio )
{
    HW_DWORD     tmpCtrl1;
   char * ptr;           /* pointer to current byte in q */
   u_int32 packet_len;
   char * end_of_queue;  /* pointer to last byte in the array that forms q */
   char * start_of_ES;   /* pointer to the first byte in the Elementary Stream */
   int bytes_consumed = 0;  /* number of bytes handled by driver */
   int limit;            /* number of bytes in queue */
   u_int8 stream_id;
   ptr = (char *)q->begin + q->out;
   limit = (q->out > q->in) ? (q->size + q->in - q->out) : (q->in - q->out);
   end_of_queue = (char *)q->begin + q->size;

   /* MKI assert(isAudio == TRUE);  now supports video too */

   switch ( perm_data->where_in_PES ) {
     case FIND_PREFIX:
       while ( perm_data->progress != GET_PACKET_LENGTH1 ) {
         switch ( perm_data->progress ) {
           case LAST_BYTE_NONZERO :
             while( *ptr != 0 )
               INCREMENT_OR_RETURN;
             perm_data->progress = LAST_BYTE_ZERO;
             /* fall through */
           case LAST_BYTE_ZERO :
             if (*ptr != 0) {
               /* go back to beginning of search */
               perm_data->progress = LAST_BYTE_NONZERO;
               INCREMENT_OR_RETURN;
               continue;
             } else {
               perm_data->progress = LAST_TWO_BYTES_ZERO;
               INCREMENT_OR_RETURN;
             }
             /* fall through */
           case LAST_TWO_BYTES_ZERO:
             while (*ptr == 0) { /* skip extra zeros */
               INCREMENT_OR_RETURN;
             }
             if (*ptr != 1) {
               perm_data->progress = LAST_BYTE_NONZERO;
               INCREMENT_OR_RETURN;
               continue;
             } else {
               perm_data->progress = SKIP_STREAM_ID;
               INCREMENT_OR_RETURN;
               break;
             }
           /* MKI , for video we must check stream ID as part of PREFIX.*/
           case SKIP_STREAM_ID:
            stream_id = (u_int8) *((u_int8 *)ptr);
            if (!isAudio){
                if ( stream_id < 0xE0 || stream_id > 0xEF ){   /* E0 to EF are valid video stream ID */
                   perm_data->progress = LAST_BYTE_NONZERO;
                   INCREMENT_OR_RETURN;
                   continue;
                }
                else{
                   perm_data->progress = GET_PACKET_LENGTH1;
                   perm_data->where_in_PES = PARSING_HEADER;
                   INCREMENT_OR_RETURN;
                }
            }
            else{
  	        if ( (stream_id < 0xC0 || stream_id > 0xDF) &&
		     (stream_id != 0xBD) ) {   /* BD and (C0 to DF) are valid audio stream ID */
		   perm_data->progress = LAST_BYTE_NONZERO;
                   INCREMENT_OR_RETURN;
                   continue;
                }
                else{
                   perm_data->progress = GET_PACKET_LENGTH1;
                   perm_data->where_in_PES = PARSING_HEADER;
                   INCREMENT_OR_RETURN;
                }
            }
           default:
             break;
         }
       }
       /* fall through */
/* MKI this is now handled above.   perm_data->progress = SKIP_STREAM_ID; */
     case PARSING_HEADER:
       do {
         switch (perm_data->progress) {
         case GET_PACKET_LENGTH1:
            packet_len = (u_int32) *( (u_int8 *)ptr);
            packet_len <<= 8;
             perm_data->bytes_left_in_packet = (int) packet_len;
             ptr++;  /* skip stuffing bytes */
             if (ptr == end_of_queue) ptr = (char *)q->begin;
             bytes_consumed++;
             if (bytes_consumed == limit) {
                 perm_data->progress = GET_PACKET_LENGTH2;
                 return bytes_consumed;
             }
         case GET_PACKET_LENGTH2:
            packet_len = (u_int32) *( (u_int8 *)ptr);
            packet_len |= (u_int32) perm_data->bytes_left_in_packet;
             perm_data->bytes_left_in_packet = (int) packet_len;
             perm_data->InitialPacketLen = perm_data->bytes_left_in_packet;
             if (perm_data->bytes_left_in_packet == 0){     /* MKI video stream unspecified length */
                perm_data->PesLenTBD = TRUE;                /* MKI figure length out later */
             }
             else
                perm_data->PesLenTBD = FALSE;
             #ifdef DEBUG
             if (perm_data->bytes_left_in_packet < 0 && isAudio)
                trace_new(TRACE_LEVEL_ALWAYS | TRACE_AUD,"handle_pes ERROR:bytes_left_in_packet < 0, Audio\n");
             if (perm_data->bytes_left_in_packet < 0 && !isAudio)
                trace_new(TRACE_LEVEL_ALWAYS | TRACE_AUD,"handle_pes ERROR:bytes_left_in_packet < 0, Video\n");
             #endif
             ptr++;  /* skip stuffing bytes */
             if (ptr == end_of_queue) ptr = (char *)q->begin;
             bytes_consumed++;
             if (bytes_consumed == limit) {
                 perm_data->progress = GET_MPEG1_MPEG2;
                 return bytes_consumed;
             }
         case GET_MPEG1_MPEG2:
             /* is this MPEG-1 or MPEG-2 systems ? */
             if ((*ptr & 0xC0) == 0x80) { /* first two bits '10' */
                 /* It's MPEG-2 */
                 perm_data->progress = SKIPPING_DATA_BITS;
                 break;
             } /* else
                * It's MPEG-1
                * progress = SKIPPING_STUFFING_BYTES; */
             /* fall through */
         case SKIPPING_STUFFING_BYTES:
             while (*ptr == (char)0xFF) {
                 perm_data->bytes_left_in_packet--;
                 ptr++;  /* skip stuffing bytes */
                 if (ptr == end_of_queue) ptr = (char *)q->begin;
                 bytes_consumed++;
                 if (bytes_consumed == limit) {
                     perm_data->progress = SKIPPING_STUFFING_BYTES;
                     return bytes_consumed;
                 }
             }
             if ((*ptr & 0xC0) == 0x40) { /* skip STD info */
                 perm_data->bytes_left_in_packet--;
                 ptr++;
                 if (ptr == end_of_queue) ptr = (char *)q->begin;
                 bytes_consumed++;
                 if (bytes_consumed == limit) {
                     perm_data->progress = SKIP_SECOND_BYTE_OF_STD;
                     return bytes_consumed;
                 }
             } else {
                 perm_data->progress = HAS_PTS;
                 continue;
             }
             /* fall through */
         case SKIP_SECOND_BYTE_OF_STD:
             perm_data->bytes_left_in_packet--;
             ptr++;
             if(ptr == end_of_queue) ptr = (char *) q->begin;
             bytes_consumed++;
             perm_data->progress = HAS_PTS;
             if (bytes_consumed == limit) {
                 return bytes_consumed;
             }
             /* fall through */
         case HAS_PTS:
             if ((*ptr & 0xF0) == 0x20 || (*ptr & 0xF0) == 0x30) {
                 perm_data->progress = GET_PTS_MPEG1;
                 continue;
             } else {
                 perm_data->bytes_left_in_packet--;
                 ptr++;
                 if (ptr == end_of_queue) ptr = (char *) q->begin;
                 bytes_consumed++;
                 perm_data->where_in_PES = SENDING_DATA;
                 if (bytes_consumed == limit) return bytes_consumed;
                 continue;
             }
         case GET_PTS_MPEG1:
         case GET_PTS_MPEG1+1:
         case GET_PTS_MPEG1+2:
         case GET_PTS_MPEG1+3:
         case FIFTH_BYTE_OF_PTS:
             /* get PTS */
             for ( ; perm_data->progress<= FIFTH_BYTE_OF_PTS; perm_data->progress++) {
                 perm_data->pts_array[perm_data->progress-GET_PTS_MPEG1] = *ptr;
                 perm_data->bytes_left_in_packet--;
                 ptr++;
                 if (ptr == end_of_queue) ptr = (char *)q->begin;
                 bytes_consumed++;
                 if (bytes_consumed == limit) return bytes_consumed;
             }
             tmpCtrl1 = *glpCtrl1;
             if (CNXT_GET_VAL(&tmpCtrl1, MPG_CONTROL1_ENABLEAUDSYNC_MASK) == 1) 
	         send_PTS(perm_data->pts_array, !isAudio);
             if ((perm_data->pts_array[0] & 0xF0) != 0x30) { /* no DTS */
                  /* Update q->out before passing q to copy_data.
                   * This simplifies copy_data().  audio_data_ready() will
                   * make sure q->out is set to the correct value before
                   * returning.*/
                 q->out = ptr - (char *)q->begin;
                 perm_data->where_in_PES = SENDING_DATA;
                 continue;
             } else perm_data->progress = SKIP_DTS;
             /* fall through */
         case SKIP_DTS:
         case SKIP_DTS+1:
         case SKIP_DTS+2:
         case SKIP_DTS+3:
         case END_OF_DTS:
             /* skip DTS */
             for ( ; perm_data->progress <= END_OF_DTS; perm_data->progress++) {
                 perm_data->bytes_left_in_packet--;
                 ptr++;
                 if (ptr == end_of_queue) ptr = (char *) q->begin;
                 bytes_consumed++;
                 if (bytes_consumed == limit) return bytes_consumed;
             }

                /* Update q->out before passing q to copy_data.
                 * This simplifies copy_data().  audio_data_ready() will
                 * make sure q->out is set to the correct value before
                 * returning.*/
             q->out = ptr - (char *)q->begin;
             perm_data->where_in_PES = SENDING_DATA;
             continue;
         case SKIPPING_DATA_BITS:
             ptr++;
             perm_data->bytes_left_in_packet--;
             if (ptr == end_of_queue) ptr = (char *) q->begin;
             bytes_consumed++;
             if (bytes_consumed == limit) {
                 perm_data->progress = READING_PTS_BITS;
                 return bytes_consumed;
             }
         case READING_PTS_BITS:
             perm_data->pts_present = ((1<<7) & *((u_int8 *) ptr)) && 1;
             ptr++;
             perm_data->bytes_left_in_packet--;

             if (ptr == end_of_queue) ptr = (char *) q->begin;
             bytes_consumed++;
             if (bytes_consumed == limit) {
                 perm_data->progress = GET_HEADER_LENGTH;
                 return bytes_consumed;
             }
             /* fall through */
         case GET_HEADER_LENGTH:
             perm_data->bytes_left_in_header = *((u_int8 *)ptr);
             ptr++;
             perm_data->bytes_left_in_packet--;

             if (ptr == end_of_queue) ptr = (char *) q->begin;
             bytes_consumed++;
             if (bytes_consumed == limit) {
                 if (perm_data->pts_present) perm_data->progress = GET_PTS_MPEG2;
                 else perm_data->progress = SKIPPING_HEADER;
                 return bytes_consumed;
             }

             if (!perm_data->pts_present) {
                 perm_data->progress = SKIPPING_HEADER;
                 continue;
             }
             perm_data->progress = GET_PTS_MPEG2;
             /* fall through */
         case GET_PTS_MPEG2:
         case GET_PTS_MPEG2+1:
         case GET_PTS_MPEG2+2:
         case GET_PTS_MPEG2+3:
         case FIFTH_BYTE_OF_MPEG2_PTS:
             /* get PTS */
             for ( ; perm_data->progress<= FIFTH_BYTE_OF_MPEG2_PTS;
                                            perm_data->progress++) {
                 perm_data->pts_array[perm_data->progress-GET_PTS_MPEG2] = *((u_int8 *)ptr);
                 perm_data->bytes_left_in_packet--;
                 perm_data->bytes_left_in_header--;
                 ptr++;
                 if (ptr == end_of_queue) ptr = (char *)q->begin;
                 bytes_consumed++;
                 if (bytes_consumed == limit) return bytes_consumed;
             }
             tmpCtrl1 = *glpCtrl1;
             if (CNXT_GET_VAL(&tmpCtrl1, MPG_CONTROL1_ENABLEAUDSYNC_MASK) == 1) 
	         send_PTS(perm_data->pts_array, !isAudio);
             perm_data->where_in_PES = SENDING_DATA;
         case SKIPPING_HEADER:
             if ((bytes_consumed + perm_data->bytes_left_in_header) >= limit) {
                 perm_data->bytes_left_in_packet -= (limit - bytes_consumed);
                 perm_data->bytes_left_in_header -= (limit - bytes_consumed);
                 if (perm_data->bytes_left_in_header == 0)
                     perm_data->where_in_PES = SENDING_DATA;
                 bytes_consumed = limit;
                 return bytes_consumed;
             }
             bytes_consumed += perm_data->bytes_left_in_header;
             perm_data->where_in_PES = SENDING_DATA;
             start_of_ES = ptr + perm_data->bytes_left_in_header;
             if ( start_of_ES > end_of_queue)
                 start_of_ES -= q->size;

               /* Update q->out before passing q to copy_data.
                * This simplifies copy_data().  audio_data_ready() will
                * make sure q->out is set to the correct value before
                * returning.*/
             q->out = start_of_ES - (char *)q->begin;
             perm_data->bytes_left_in_packet -= perm_data->bytes_left_in_header;
         default:
             /* error */
             break;
         }
       } while (perm_data->where_in_PES != SENDING_DATA);
       /* fall through */

     case SENDING_DATA:
         {
         int bytes_copied, bytes_left_in_queue;
         bool PrefixFound = FALSE;
         bytes_left_in_queue = (q->size + q->in - q->out) % q->size;

         /* MKI. If length is to be determined, then find the number of bytes
            until the next start code prefix.
         */
         if (perm_data->PesLenTBD){
            /* save_bytes = perm_data->bytes_left_in_packet; */
            perm_data->bytes_left_in_packet = BytesUntilStartCode(q, bytes_left_in_queue, end_of_queue, &PrefixFound);
            if (PrefixFound){
                perm_data->PesLenTBD = FALSE; /* the length is now known so we should start at the top now */
            }
         }
         bytes_copied = copy_data_ac3_aligned( q, min(perm_data->bytes_left_in_packet,
                                                      bytes_left_in_queue), isAudio);
         bytes_consumed += bytes_copied;
         perm_data->bytes_left_in_packet -= bytes_copied;
         if (perm_data->bytes_left_in_packet == 0 && perm_data->PesLenTBD == FALSE) {
             perm_data->where_in_PES = FIND_PREFIX;
             perm_data->progress = LAST_BYTE_NONZERO;
             /* FUTURE update ptr and continue */
         }
         }
         return bytes_consumed;
         break;
     default:
         break;
    }

return bytes_consumed;
}
#endif /* AC3_ALIGNED */


/*
 ** send_PTS
 *
 *  FILENAME: AUDIO\avutil.c
 *
 *  PARAMETERS:
 *
 *  DESCRIPTION:  This function should reformat the PTS contained
 *                in pts_array[] and place it in memory where
 *                the hardware expects it to be.  It then notifies
 *                the hardware that a new PTS has been written,
 *                and tells the hardware whether the PTS is for
 *                audio or video.
 *
 *  NOTE:  This function is untested and unused.  It really has
 *         no purpose until audio_pause() and audio_resume()
 *         are implemented/required.
 */

void send_PTS( unsigned char pts_array[], bool isVideo)
{
    u_int32 PTSLow, PTSHigh, DataStartAddr;
    unsigned char * PTSStoreAddr;
    HW_DWORD     tmpCtrl0;
    bool kstate;

    if (FirstPTS==TRUE) LastAudPTSMatured=TRUE;
    /* process_time_sleep(10); */

    trace_new(TRACE_LEVEL_2 | TRACE_AUD, "STC Lo/Hi  %x/%x\n", *lpSTCLo, *lpSTCHi );
    if (!isVideo) {
        if (!LastAudPTSMatured) {
            trace_new(TRACE_LEVEL_3 | TRACE_AUD, "Last audio PTS has not matured.\n");
            return;
        }
        PTSStoreAddr = (unsigned char *)(0x1CC << 1);
        DataStartAddr = (u_int32)*lpAudWritePtr;
    } else {
        if (!LastVidPTSMatured) {
            trace_new(TRACE_LEVEL_3 | TRACE_AUD, "Last video PTS has not matured.\n");
            return;
        }
        PTSStoreAddr = (unsigned char *)(0x1C0 << 1);
        DataStartAddr = (u_int32)*DPS_VIDEO_WRITE_PTR_EX(gDemuxInstance);
    }

    /* format new PTS and copy to memory */
    PTSLow = ((pts_array[4] & 0xFE) >> 1);
    PTSLow |= ((pts_array[3] & 0xFF) << 7);
    PTSLow |= ((pts_array[2] & 0xFE) << 14);
    PTSLow |= ((pts_array[1] & 0xFF) << 22);
    PTSLow |= (pts_array[0] & 0x06) << 29;

    PTSHigh = (pts_array[0] & 0x8) >> 3;         /* PTS[32] to [31] */
    PTSHigh += (DataStartAddr & 0x3fffff) << 1;   /* 22 low bits to [22:1] */
    PTSHigh += (DataStartAddr & 0x80000000) >> 16; /* wrap bit to [23] */

    if(FirstPTS == TRUE) {
        /* shove PTS into STC */
        trace_new(TRACE_LEVEL_3 | TRACE_AUD, "Shoving 1st PTS into STC.  Hi/Lo = %x/%x\n", PTSHigh & 0x1, PTSLow);
        *lpSTCLo = PTSLow;
        *lpSTCHi = PTSHigh & 0x1;
        FirstPTS = FALSE;
        *glpIntMask |= MPG_PTS_MATURED;
    } else {


    *(u_int32 *)((u_int32)PTSStoreAddr | (BUFF_BASE | NCR_BASE)) = PTSLow;
    *(u_int32 *)(((u_int32)PTSStoreAddr + 4) | (BUFF_BASE  | NCR_BASE)) = PTSHigh;
    trace_new(TRACE_LEVEL_2 | TRACE_AUD, " placed PTSLow/High into memory  %x/%x \n", PTSLow, PTSHigh);

    kstate = critical_section_begin();
    tmpCtrl0 = *glpCtrl0;

    /* set new PTS bit */
    CNXT_SET_VAL(&tmpCtrl0, MPG_CONTROL0_NEWPTSSTORED_MASK, 1);
    /* set audio/video bit */
    CNXT_SET_VAL(&tmpCtrl0, MPG_CONTROL0_NEWPTSISVIDEO_MASK, isVideo);

    *glpCtrl0 = tmpCtrl0;
    if (!isVideo) LastAudPTSMatured = FALSE;
    else LastVidPTSMatured = FALSE;

    CNXT_SET_VAL(glpCtrl0, MPG_CONTROL0_NEWPTSSTORED_MASK, 0);

    critical_section_end(kstate);
   }

}

/****************************************************************************
 * Modifications:
 * $Log: 
 *  17   mpeg      1.16        9/2/03 7:09:12 PM      Joe Kroesche    SCR(s) 
 *        7415 :
 *        reordered header files to eliminate warnings when building for PSOS
 *        
 *  16   mpeg      1.15        6/2/03 3:48:18 PM      Craig Dry       SCR(s) 
 *        6579 :
 *        An AC3 header must be aligned to an 8 byte boundary.  Previously it
 *        was thought to be a 4 byte boundary.
 *        
 *  15   mpeg      1.14        5/23/03 1:44:40 PM     Craig Dry       SCR(s) 
 *        6579 :
 *        Force AC3 header to 32 bit boundary in audio decode buffer.
 *        
 *  14   mpeg      1.13        2/13/03 11:15:42 AM    Matt Korte      SCR(s) 
 *        5479 :
 *        Removed old header reference
 *        
 *  13   mpeg      1.12        12/2/02 2:10:40 PM     Craig Dry       SCR(s) 
 *        4991 :
 *        Handle AC3 Start-Code ID (0xBD) to be handled along with MPEG audio
 *        Start-Code ID's by handle_PES routine.
 *        
 *  12   mpeg      1.11        6/10/02 3:21:38 PM     Craig Dry       SCR(s) 
 *        3923 :
 *        Remove MPG bitfields unconditionally - Step 2
 *        
 *  11   mpeg      1.10        6/6/02 5:47:48 PM      Craig Dry       SCR(s) 
 *        3923 :
 *        Remove MPG bitfields conditionally - Step 1
 *        
 *  10   mpeg      1.9         5/13/02 12:22:14 PM    Tim White       SCR(s) 
 *        3760 :
 *        DPS_ bitfields changes, remove DPS_ HSDP definitions, legacy gendmxc 
 *        globals...
 *        
 *        
 *  9    mpeg      1.8         4/5/02 11:59:18 AM     Tim White       SCR(s) 
 *        3510 :
 *        Backout DCS #3468
 *        
 *        
 *  8    mpeg      1.7         3/28/02 2:59:42 PM     Quillian Rutherford 
 *        SCR(s) 3468 :
 *        Removed bit fields
 *        
 *        
 *  7    mpeg      1.6         3/12/02 4:57:44 PM     Bob Van Gulick  SCR(s) 
 *        3359 :
 *        Add multi-instance demux support
 *        
 *        
 *  6    mpeg      1.5         5/21/01 10:40:04 AM    Dave Wilson     SCR(s) 
 *        1828 1829 :
 *        The copy_data routine had a bug relating to the amount of data copied
 *        when data was passed in chunks and the leftover array was being used.
 *         This
 *        showed up in the Fathom game which calls video_data_ready multiple 
 *        times
 *        for its stills. This is not a complete fix for the problem seen, 
 *        though,
 *        since the h/w still can't decode the offending still!
 *        
 *  5    mpeg      1.4         4/6/01 5:03:22 PM      Amy Pratt       DCS914 
 *        Removed old neches code and references to CHIP_NAME
 *        
 *  4    mpeg      1.3         6/6/00 9:10:04 PM      Amy Pratt       Changes 
 *        to avoid warnings in VxWorks builds
 *        
 *  3    mpeg      1.2         6/6/00 1:31:20 PM      Amy Pratt       Removed 
 *        all C++ style comments
 *        Corrected header information
 *        
 *  2    mpeg      1.1         5/26/00 6:57:36 PM     Amy Pratt       Made 
 *        changes to link without old audio driver,
 *        removed *ifdef DEBUG from around trace calls.
 *        
 *  1    mpeg      1.0         5/24/00 9:54:40 PM     Amy Pratt       
 * $
 * 
 *    Rev 1.16   02 Sep 2003 18:09:12   kroescjl
 * SCR(s) 7415 :
 * reordered header files to eliminate warnings when building for PSOS
 * 
 *    Rev 1.15   02 Jun 2003 14:48:18   dryd
 * SCR(s) 6579 :
 * An AC3 header must be aligned to an 8 byte boundary.  Previously it
 * was thought to be a 4 byte boundary.
 * 
 *    Rev 1.14   23 May 2003 12:44:40   dryd
 * SCR(s) 6579 :
 * Force AC3 header to 32 bit boundary in audio decode buffer.
 * 
 *    Rev 1.13   13 Feb 2003 11:15:42   kortemw
 * SCR(s) 5479 :
 * Removed old header reference
 * 
 *    Rev 1.12   02 Dec 2002 14:10:40   dryd
 * SCR(s) 4991 :
 * Handle AC3 Start-Code ID (0xBD) to be handled along with MPEG audio
 * Start-Code ID's by handle_PES routine.
 * 
 *    Rev 1.11   10 Jun 2002 14:21:38   dryd
 * SCR(s) 3923 :
 * Remove MPG bitfields unconditionally - Step 2
 * 
 *    Rev 1.10   06 Jun 2002 16:47:48   dryd
 * SCR(s) 3923 :
 * Remove MPG bitfields conditionally - Step 1
 * 
 *    Rev 1.9   13 May 2002 11:22:14   whiteth
 * SCR(s) 3760 :
 * DPS_ bitfields changes, remove DPS_ HSDP definitions, legacy gendmxc globals...
 * 
 * 
 *    Rev 1.8   05 Apr 2002 11:59:18   whiteth
 * SCR(s) 3510 :
 * Backout DCS #3468
 * 
 * 
 *    Rev 1.6   12 Mar 2002 16:57:44   vangulr
 * SCR(s) 3359 :
 *  Add multi-instance demux support
 * 
 * 
 *    Rev 1.5   21 May 2001 09:40:04   dawilson
 * SCR(s) 1828 1829 :
 * The copy_data routine had a bug relating to the amount of data copied
 * when data was passed in chunks and the leftover array was being used. This
 * showed up in the Fathom game which calls video_data_ready multiple times
 * for its stills. This is not a complete fix for the problem seen, though,
 * since the h/w still can't decode the offending still!
 * 
 *    Rev 1.4   06 Apr 2001 16:03:22   prattac
 * DCS914 Removed old neches code and references to CHIP_NAME
 * 
 *    Rev 1.3   06 Jun 2000 20:10:04   prattac
 * Changes to avoid warnings in VxWorks builds
 * 
 *    Rev 1.2   06 Jun 2000 12:31:20   prattac
 * Removed all C++ style comments
 * Corrected header information
 * 
 *    Rev 1.1   26 May 2000 17:57:36   prattac
 * Made changes to link without old audio driver,
 * removed *ifdef DEBUG from around trace calls.
 * 
 *    Rev 1.0   24 May 2000 20:54:40   prattac
 * Initial revision.
 * 
 * **********************************************************
 * CHANGE LOG FROM AVUTIL.C
 * **********************************************************
 *    Rev 1.35   11 Apr 2000 13:40:20   mustafa
 * Fixed bug causing jittery video and audio. Previous fix for warning 
 * caused bug.
 * 
 *    Rev 1.34   10 Apr 2000 13:24:24   prattac
 * Don't need to include PLL register definitions.
 * 
 *    Rev 1.33   10 Apr 2000 09:07:46   dawilson
 * More changes with opentv_en1.h.  
 * 
 *    Rev 1.32   06 Apr 2000 17:58:54   prattac
 * Removed inclusion of "opentvx.h for non-opentv builds.  Also cleaned
 * up warning messages.
 * 
 *    Rev 1.31   06 Apr 2000 11:49:50   raymack
 * fixes to remove warnings
 * 
 *    Rev 1.30   29 Mar 2000 10:43:28   raymack
 * changes to eliminate bit fields
 * 
 *    Rev 1.29   20 Mar 2000 13:34:10   prattac
 * Changed send_PTS() function to use unsigned char instead of char.
 * Changes require new audio.h, rev 1.6
 * 
 *    Rev 1.28   14 Mar 2000 11:18:18   mustafa
 * Fixed problem with occasional incorrect length due to signed char vs.
 * unsigned char.
 *
 *    Rev 1.27   02 Feb 2000 18:46:04   prattac
 * Moved AC-3 microcode to its own library.
 *
 *    Rev 1.26   11 Jan 2000 19:25:54   prattac
 * For colorado, get audio read/write pointer register definitions from
 * the DPS block instead of the MPG block.
 *
 *    Rev 1.25   10 Jan 2000 17:35:02   prattac
 * Initial changes to support COLORADO.  Divider now has its own register,
 * not a field in AUD_PLL_CONFIG.  MPG_VID_ENC_**_PTR_REG no longer defined.
 *
 *    Rev 1.24   06 Jan 2000 12:03:14   prattac
 * Corrected download_ucode() function so it dowloads AC3 microcode when
 * in AC3 mode and MPEG microcode in MPEG mode.  Previously it was downloading
 * the wrong microcode.
 *
 *    Rev 1.23   05 Jan 2000 17:52:32   prattac
 * Look for OPENTV #define instead of DRIVER_INCL_OPENTV in preparation
 * for thumb builds.
 *
 *    Rev 1.22   05 Jan 2000 16:55:14   prattac
 * Initial support for AC-3.  Download AC-3 microcode when requested.
 *
 *    Rev 1.21   01 Nov 1999 14:16:02   mustafa
 * Made several changes to the handling of PES data to avoid hangs when live
 * data is fed in.
 *
 *    Rev 1.20   22 Oct 1999 16:55:48   prattac
 * Removed SABINE specific code and prepared for builds with new
 * config files (CN8600).
 * Lots of fixes in send_PTS() in preparation for sync'ed
 * audio and video from disk.
 *
 *    Rev 1.19   23 Aug 1999 12:41:36   prattac
 * look for DRIVER_INCL_OPENTV define instead of OPENTV
 *
 *    Rev 1.18   29 Jul 1999 09:10:32   mustafa
 * Modified to support Video PES. Changes are indicated by MKI throughout.
 *
 *    Rev 1.17   24 Jun 1999 20:21:10   prattac
 * Swap bytes manually, do not use byte swapped memory region.
 *
 *    Rev 1.16   06 Apr 1999 18:40:12   prattac
 * Initial changes for NECHES:  Use Audio only sync enable bit.
 *
 *    Rev 1.15   23 Mar 1999 17:44:00   prattac
 * Cleanup before code review.  Removed dead code, added comments.
 *
 *    Rev 1.14   08 Jan 1999 17:30:24   prattac
 * Or in NCR_BASE (non cached region) whenever writing from/reading to
 * hardware addresses in low memory.
 *
 *    Rev 1.13   06 Jan 1999 19:16:22   prattac
 * Increased OVERLAP_PROTECTION constant to avoid audio buffer overflows
 * at previous value (was 0xc00, now 0xc80).
 *
 *    Rev 1.12   10 Nov 1998 03:00:58   prattac
 * removed configure_PLL() and associated variables. (now in audio.c)
 *
 *    Rev 1.11   21 Sep 1998 17:31:20   prattac
 * Changes to eliminate compiler warnings about unused variables.
 *
 *    Rev 1.10   18 Sep 1998 17:12:04   prattac
 * *  Added/fixed code to use byte swapped memory instead of manually
 *        swapped copy (in copy_data()).
 * *  Added initialization of audio PLL reg. to the new nominal audio
 *        frequency at audio sample frequency change.
 *
 *    Rev 1.9   28 Jul 1998 11:29:50   prattac
 * First pass implementation of audio clock recovery
 *
 *    Rev 1.8   30 Jun 1998 16:11:54   prattac
 * Changed trace calls to trace_new.
 *
 *    Rev 1.7   26 Jun 1998 16:36:52   prattac
 * * Added code to pass PTS to hardware buffer, and initialize STC if needed.
 * * Added code to toggle wrap bit when audio/video encoded buffer read/write
 *   pointers wrap around the end of their respective buffers.
 * * Added code to handle FSCODE changed interrupts and convert the incoming
 *   audio PTS to the audio clock rate
 * * Added code to test the audio clock recovery registers (but not actually
 *   perform audio clock recovery.)
 * * Added variable 'reenable_sync' to help handle discontinuities
 *
 *    Rev 1.6   15 Jun 1998 09:51:04   mustafa
 * Fixed problem in copy_data where leftovers were getting lost.
 *
 ****************************************************************************/

