/****************************************************************************/
/*                   Conexant Proprietary and Confidential                  */
/*                       Software File/Module Header                        */
/*              Conexant Systems Inc. (c) 1998, 1999, 2000, 2001            */
/*                               Austin, TX                                 */
/*                            All Rights Eeserved                           */
/****************************************************************************/
/*
 * Filename:       gdmxsi.c
 *
 *
 * Description:    Contains all the TS/SI handling functions in the demux driver
 *
 * Author:         Bob Van Gulick 
 *
 ****************************************************************************/
/* $Header: demuxsi.c, 23, 3/31/04 4:41:54 PM, Yong Lu$
 ****************************************************************************/

#include "stbcfg.h"
#include "basetype.h"

#if CUSTOMER == VENDOR_C
    #define VENDOR_C_DEMUX_CHANGES
#endif

#define INCL_DPS
#define INCL_MPG
#define INCL_GPI
#define INCL_PLL
#define INCL_PCI
#define INCL_ITC

#include "globals.h"
#include "kal.h"
#include "startup.h"
#include "retcodes.h"
#include "demuxapi.h"
#include "demuxint.h"
#include "clkrec.h"

#define CRC_CHECK 
#if (IPSTB_INCLUDE == YES)
#undef CRC_CHECK 
#endif

extern DemuxInfo gDemuxInfo[];
extern sem_id_t GenDmxPSITaskProcSem;

extern sem_id_t PSI_Sem_ID;
extern u_int32 PSI_int_mask;


/*-----------------------------------------------------**
** gen_dmx_section_notify                              **
**     Params: wptr      write pointer                 **
**             chid      channel id                    **
**             length    section length                **
**             tag       section tag                   **
**             condition indicates errors if any       **
**             fid       filter id that fired          **
**       This function calls the section Notify.       **
**-----------------------------------------------------*/
bool gen_dmx_section_notify(u_int32 dmxid, u_int8 *wptr, u_int32 chid, u_int32 filter_mask) {
    u_int32 i, fid;
    u_int32 Match, Mask, DataWord;
    u_int8 *local_wptr;
    DemuxInfo *dmx = &gDemuxInfo[dmxid];

    /*
     * Find the fid from the filter_mask
     */

    if (filter_mask) {
        fid = 0;
        i = 1;
        while (!(filter_mask&i)) {
            ++fid;
            i <<= 1;
        }
    } else {
        fid = GENDMX_BAD_FILTER;
    }

    /*
     * If TOGGLE mode, perform the toggle
     */

    if ((dmx->ChannelInfoTable[chid].LoopMode == GENDEMUX_TOGGLE) && (fid < TOTAL_FILTERS)) {
        dmx->FilterTable[fid].Match[0]  ^= 0x00000001;
        *DPS_PATTERN_BASE_EX(dmxid,fid,0) ^= 0x01000000; /*toggle low bit of table id */
    }

    /*
     * Determine if the section passes the extended filter if enabled
     */

    if (fid < TOTAL_FILTERS && fid != GENDMX_BAD_FILTER) {
        if (dmx->FilterTable[fid].ExtFilterEnabled == GEN_DEMUX_ENABLE &&
            dmx->FilterTable[fid].FilterEnabled == GEN_DEMUX_ENABLE) {
            /*
             * Using extended filtering, perform the check now
             */

            local_wptr = wptr + dmx->FilterTable[fid].ExtFilterOffset;
            for (i = 0; i <  GENDMX_EXT_FILTER_SIZE/4; ++i) {
                Match = dmx->FilterTable[fid].ExtMatch[i];
                Mask  = dmx->FilterTable[fid].ExtMask[i];
                DataWord = (u_int32) *local_wptr++;
                DataWord |= (u_int32) ((*local_wptr++) << 8);
                DataWord |= (u_int32) ((*local_wptr++) << 16);
                DataWord |= (u_int32) ((*local_wptr++) << 24);
                if ((Match & Mask) != (DataWord & Mask)) {
                    /*
                     * Does not match extended filter, so throw the section away
                     */

                    /*
                     * Indicate client was not notified
                     */
                     #ifdef DVNCA
                     //trace(">>>>not match ext filter\n");
                     mem_free(wptr);
                     #endif

                    return(FALSE);
                }
            }
        }
    }

    trace_new(DPS_ISR_MSG,"DEMUX: SectionNotify Called for Channel %d\n", chid);
    trace_new(DPS_ISR_MSG,"PID =0x%x  ", dmx->ChannelInfoTable[chid].PID);
    trace_new(DPS_ISR_MSG,"wptr=0x%x  ", (int) wptr);
    trace_new(DPS_ISR_MSG,"Len=0x%x  ", (int) dmx->ChannelInfoTable[chid].NotifyData.length);
    trace_new(DPS_ISR_MSG,"condition=%d\n", (int) dmx->ChannelInfoTable[chid].NotifyData.condition);

    /*
     * Perform callback now
     */

	#ifdef DVNCA
	if((dmx->ChannelInfoTable[chid].TagSet) && 
	    (dmx->ChannelInfoTable[chid].Tag == 0x80000001))
		dmx->ChannelInfoTable[chid].NotifyData.fid_mask = filter_mask&dmx->ChannelInfoTable[chid].FilterEnable;
	else
		dmx->ChannelInfoTable[chid].NotifyData.fid_mask = filter_mask; /* provide fid mask to client */
	#else
	dmx->ChannelInfoTable[chid].NotifyData.fid_mask = filter_mask; /* provide fid mask to client */
	#endif
    dmx->ChannelInfoTable[chid].NotifyData.pData = wptr;
    /* set the condition according to crc notification and crc status
     * if notification is enabled and there is an error, then set the condition
     * to error.  If crc notification is not enabled and there is an error,
     * then we should not get here
     */
    if( dmx->NotifyCRC && (dmx->ChannelInfoTable[chid].CrcStatus == 0x03000000) )
    {
        dmx->ChannelInfoTable[chid].NotifyData.condition = GENDMX_ERROR;
        trace_new( DPS_ISR_MSG, "DEMUX: section notify with crc error\n" );
        trace_new( DPS_ISR_MSG, "DEMUX: chid=%d, fidmask=%08X\n", chid, filter_mask );
    }
    else
    {
        dmx->ChannelInfoTable[chid].NotifyData.condition = GENDMX_SECTION_AVAILABLE | GENDMX_CRC_CHECKED;
    }

    if (dmx->ChannelInfoTable[chid].DataNotify != NULL) {
        dmx->ChannelInfoTable[chid].NotifyCount++;
        if ( dmx->ChannelInfoTable[chid].DemuxEnable )
        {
           dmx->ChannelInfoTable[chid].NotifyCalled++;
           if(dmx->ChannelInfoTable[chid].TagSet)
             dmx->ChannelInfoTable[chid].NotifyData.tag = dmx->ChannelInfoTable[chid].Tag;

#ifdef IPANEL_SEM_LOCKED_TEST1 
	   flag_pop_sec_to_callback  = 1 ; 
#endif

#ifdef IPANEL_SEM_LOCKED_TEST2
	  sem_put(GenDmxPSITaskProcSem);
#endif

#ifdef MHP
           /* MHP change */
           dmx->ChannelInfoTable[chid].DataNotify(&(dmx->ChannelInfoTable[chid].NotifyData), fid);
#else
           dmx->ChannelInfoTable[chid].DataNotify(&(dmx->ChannelInfoTable[chid].NotifyData));
#endif

#ifdef IPANEL_SEM_LOCKED_TEST2
	  sem_get(GenDmxPSITaskProcSem,KAL_WAIT_FOREVER );
#endif

#ifdef IPANEL_SEM_LOCKED_TEST1 
	   flag_pop_sec_to_callback  = 0 ; 
#endif

           dmx->ChannelInfoTable[chid].NotifyCalled--;
        }
        else
        {
           if ( dmx->ChannelInfoTable[chid].NotifyData.write_ptr )
           {
              /* client allocate a buffer, still need call the callback to return it */
              dmx->ChannelInfoTable[chid].NotifyCalled++;
              if(dmx->ChannelInfoTable[chid].TagSet)
                dmx->ChannelInfoTable[chid].NotifyData.tag = dmx->ChannelInfoTable[chid].Tag;

#ifdef IPANEL_SEM_LOCKED_TEST1 
	   flag_pop_sec_to_callback  = 1 ; 
#endif

#ifdef IPANEL_SEM_LOCKED_TEST2
	  sem_put(GenDmxPSITaskProcSem);
#endif

#ifdef MHP
              /* MHP change */
              dmx->ChannelInfoTable[chid].DataNotify(&(dmx->ChannelInfoTable[chid].NotifyData), fid);
#else
              dmx->ChannelInfoTable[chid].DataNotify(&(dmx->ChannelInfoTable[chid].NotifyData));
#endif

#ifdef IPANEL_SEM_LOCKED_TEST2
	  sem_get(GenDmxPSITaskProcSem,KAL_WAIT_FOREVER );
#endif

#ifdef IPANEL_SEM_LOCKED_TEST1 
	   flag_pop_sec_to_callback  = 0 ; 
#endif

              dmx->ChannelInfoTable[chid].NotifyCalled--;
           }
           return FALSE;
        }
    } else {
        #ifdef DVNCA
        //trace(">>>>>>no notify\n");
        mem_free(wptr);
        #endif

        return(FALSE);
    }

    /*
     * If it's ONE_SHOT, shut the channel off
     */

    if (dmx->ChannelInfoTable[chid].LoopMode == GENDEMUX_ONE_SHOT)
    {
        if (cnxt_dmx_channel_control(dmxid, chid, GEN_DEMUX_DISABLE) != DMX_STATUS_OK)
        {
            trace_new(DPS_ERROR_MSG,"DEMUX: cnxt_dmx_control_channel failed\n");
        }
    }

    #ifdef DVNCA
    else if( (dmx->ChannelInfoTable[chid].TagSet) &&
    	          (dmx->ChannelInfoTable[chid].Tag == 0x80000001) &&
    	          (dmx->ChannelInfoTable[chid].FilterEnable == 0) )
    {
        if (cnxt_dmx_channel_control(dmxid, chid, GEN_DEMUX_DISABLE) != DMX_STATUS_OK)
        {
            trace_new(DPS_ERROR_MSG,"DEMUX: cnxt_dmx_control_channel failed\n");
        }
    }
    #endif

    /*
     * Indicate client was notified
     */

    return(TRUE);
}

/*----------------------------------------------------------------------------**
** gen_dmx_section_unlock                                                     **
**                                                                            **
** Params:                                                                    **
**         chid :  Channel ID                                                 **
**         buffer: The address of a section to unlock.                        **
**         length: The size of the section to unlock.                         **
** Desc:                                                                      **
**         Frees a section of the circular buffer when no longer needed.      **
**         OpenTV does not handle buffer wrap, so reset to start of buffer    **
**         when possible.                                                     **
** Returns:                                                                   **
**         nothing                                                            **
**----------------------------------------------------------------------------*/
void gen_dmx_section_unlock ( u_int32 dmxid, 
                              u_int32 chid,
                              u_int8 *buffer,
                              u_int32 length )
{
   ChannelInformationStruc *pChInfo;
   volatile u_int32        *ptr, *start_ptr;
   u_int32                 len;
   int32                   bytes_locked;
   DemuxInfo               *dmx = &gDemuxInfo[dmxid];

   pChInfo = &dmx->ChannelInfoTable[chid];

   trace_new ( DPS_FUNC_TRACE, "DEMUX:gen_dmx_section_unlock called chid=%d\n", chid );

   if ( ( !dmx->DemuxInitialized ) || 
        ( buffer > pChInfo->pBufferEnd ) ||
        ( buffer < pChInfo->pBuffer ) )

   {
      return;
   }

   /*  Stop the processing tasks momentarily */
   sem_get ( GenDmxPSITaskProcSem, KAL_WAIT_FOREVER );

   /* Ensure slot is enabled */
   if ( pChInfo->InUseFlag == CHANNEL_IN_USE )
   {
      /* find the section that contains the pointer "buffer" */
      start_ptr = NULL;
      ptr = (volatile u_int32 *)pChInfo->pReadPtr;
      if ( pChInfo->pAckPtr >= pChInfo->pReadPtr )
      {
         bytes_locked = (int32)( pChInfo->pAckPtr - pChInfo->pReadPtr );
      }
      else
      {
         bytes_locked = (int32)( pChInfo->pAckPtr
                                 - pChInfo->pBuffer
                                 + pChInfo->pBufferEnd
                                 - pChInfo->pReadPtr );
      }

      while ( bytes_locked > 0 )
      {
         /* get length of the section */
         len = *ptr;
         if ( ( len & 0xffff0000 ) != 0xffff0000 )
         {
            /* locked sections corrupted, unlock all sections */
            trace_new ( DPS_ERROR_MSG,
                        "gen_dmx_section_unlock() Bogus len=0x%08x @ 0x08%x\n",
                        len,
                        ptr );
            pChInfo->pReadPtr = pChInfo->pAckPtr;

            *DPS_PID_SLOT_READ_PTR_EX(dmxid, chid) =
                   (((u_int32)pChInfo->pReadPtr&~NCR_BASE)|DPS_PAW_SYS_ADDRESS);
            sem_put ( GenDmxPSITaskProcSem );
            return;
         }

         len &= 0xffff;

         /* check if buffer is within this section */
         if ( ( (u_int8*)ptr + len ) < pChInfo->pBufferEnd )
         {
            if ( ( buffer >= (u_int8*)ptr ) && ( buffer <= ( (u_int8*)ptr + len ) ) )
            {
               /* we found the section we want to unlock */
               start_ptr = ptr;
               break;
            }
         }
         else
         {
            if ( ( buffer >= (u_int8*)ptr ) || 
                 ( buffer <= pChInfo->pBuffer + (u_int32)ptr + len - (u_int32)pChInfo->pBufferEnd ) )
            {
               /* we found the section we want to unlock */
               start_ptr = ptr;
               break;
            }
         }

         /* this is not the section that contains buffer */
         AdvancePtr ( (u_int8 **)&ptr, len, pChInfo->pBuffer, pChInfo->pBufferEnd );
         bytes_locked -= (int32)len;
      }
      
      /* unlock the section if found */
      if ( start_ptr )
      {
         *( start_ptr + 1 ) = 0xffffffff;
         pChInfo->Notify_Unlock --;
      }

      if ( (u_int8*)start_ptr == pChInfo->pReadPtr )
      {
         /* we are dealing with the 1st locked section, move ReadPtr if we can */
         if ( pChInfo->pAckPtr >= pChInfo->pReadPtr )
         {
            bytes_locked = (int32)( pChInfo->pAckPtr - pChInfo->pReadPtr );
         }
         else
         {
            bytes_locked = (int32)( pChInfo->pAckPtr
                                    - pChInfo->pBuffer
                                    + pChInfo->pBufferEnd
                                    - pChInfo->pReadPtr );
         }

         while ( bytes_locked > 0 )
         {
            ptr = (volatile u_int32 *)pChInfo->pReadPtr;
            len = *ptr ++;
            if ( ( len & 0xffff0000 ) != 0xffff0000 )
            {
               /* locked sections corrupted, unlock all sections */
               trace_new ( DPS_ERROR_MSG,
                           "gen_dmx_section_unlock() Bogus len=0x%08x @ 0x08%x\n",
                           len,
                           ptr );
               pChInfo->pReadPtr = pChInfo->pAckPtr;
   
               *DPS_PID_SLOT_READ_PTR_EX(dmxid, chid) =
                      (((u_int32)pChInfo->pReadPtr&~NCR_BASE)|DPS_PAW_SYS_ADDRESS);
               sem_put ( GenDmxPSITaskProcSem );
               return;
            }

            len &= 0xffff;

            if ( *ptr != 0xffffffff )
            {
               /* we reached a locked section, stop here */
               break;
            }

            /* advance ReadPtr */
            AdvancePtr ( &pChInfo->pReadPtr, len, pChInfo->pBuffer, pChInfo->pBufferEnd );

            bytes_locked -= (int32)len;
         }

         if ( (u_int8*)start_ptr != pChInfo->pReadPtr )
         {
            /* read pointer got updated, move HW read pointer */
            *DPS_PID_SLOT_READ_PTR_EX(dmxid, chid) =
                   (((u_int32)pChInfo->pReadPtr&~NCR_BASE)|DPS_PAW_SYS_ADDRESS);
         }
      }
   }

   sem_put ( GenDmxPSITaskProcSem );
}

/*-------------------------------------------------------------------**
** gen_dmx_PSI_Task                                                  **
**     params: none                                                  **
**         This is the main Demux PSI Task.  It is driven by the     **
**         PSI_Sem_ID semaphore from the demux IRQ handler.          **
**-------------------------------------------------------------------*/
void gen_dmx_PSI_Task(void *arg) 
{
   volatile u_int32 *section_ptr, *start_ptr;
   u_int32 chid, filter_mask, section_length=0, total_length;
   ChannelInformationStruc *pChInfo;
   u_int32 dmxid;
   DemuxInfo *dmx;
   bool ks, section_locked; 
   volatile u_int32 *pCrcStatus; /* pointer to crc status word at end of section */
   int32 bytes_available;
#if PARSER_TYPE==DTV_PARSER
   volatile u_int32 *length_ptr;
#endif


   while ( 1 )
   {
      /*
       * Wait for PSI event interrupt
       */
      sem_get ( PSI_Sem_ID, KAL_WAIT_FOREVER );

      /*********************************************************
       * When we get here, we need to check the PSI demux 
       * bitmask to see which demux needs PSI processing
       *********************************************************/
      for ( dmxid = 0 ; dmxid < MAX_DEMUX_UNITS ; dmxid ++ )
      {
         if ( dmxid == 1 )
         {
            /* Skip demux 1 for now - not used */
            continue;
         }

         /* Is there work for this demux? */
         if ( PSI_int_mask & ( 1 << dmxid ) )
         {
            dmx = &gDemuxInfo[dmxid];

            /*
             * Ensure mutual exclusion to the control
             */
            sem_get ( GenDmxPSITaskProcSem, KAL_WAIT_FOREVER );

            /*
             * Currently the way this works is that for every PSI
             * slot that is enabled, the slot is inspected for new
             * sections that have arrived.  For each slot that has new
             * data, all sections for that slot will be processed before
             * looking at the next slot.
             *
             * This could be modified rather easily to process 1 or more
             * sections per slot before moving onto the next slot even
             * if there is more data for a given slot.  This increases
             * the frequency of checking each slot, removes the possibility
             * of starved slots behind a busy slot (since currently they
             * are checked starting at slot 0 every time), and allows the
             * ECM slot to have unilateral priority.  Be careful though
             * because a strict "process one section per slot" approach
             * will increase the possibility of losing sections which are
             * part of a large data construct from the client (e.g. OTV).
             * It is usually wiser to grab several sections in a row, then
             * drop several sections in a row rather than take one drop one.
             *
             * Since there is no longer a separate ECM priority path, ECM
             * is treated just like any other PSI slot.  We'll need to know
             * which slot is the ECM slot (via the ChannelInfoTable at
             * allocate channel time) so that this slot is always checked
             * first and can be checked in between X number of other checks
             * and/or processing for one slot assuming multiple sections
             * are available per slot.
             *
             * Also it would be real easy and likely beneficial to not
             * always check starting from slot 0 to 31 everytime but rather
             * implement a somewhat random (or priority) based scheme to
             * drive the check order each time through.  This could be as
             * simple as start checking on the slot + 1 that you did the
             * previous time through (e.g.  Go from 0 to 31 the first time.
             * Go from 1 to 31 then 0 the second time, 2-31,0-1 the 3rd,
             * 3-31,0-2 the 4th, etc.  This eliminates some of the priority
             * automatically given for which slot is used.  This would be
             * a recommended change.
             */

            /*
             * Inspect all buffers for new sections
             */
            for ( chid = 0 ; chid < TOTAL_CHANNELS ; chid++ )
            {
               /*
                * Skip audio and video channels
                */

               if ( dmx->ChannelInfoTable[chid].stype == VIDEO_PES_TYPE ||
                    dmx->ChannelInfoTable[chid].stype == AUDIO_PES_TYPE )
               {
                  continue;
               }


               /*
                * Since generic ECM handling is done through this path
                * just like any other PSI type, it might be a good idea
                * to prioritize ECM handling above all other PSI.  To do
                * that, we would need to know which slot contains the PID
                * for ECM handling.  This could be done when the channel
                * is setup and saved in the ChannelInfoTable[].  That slot
                * would then always be checked first in this function.
                */
               pChInfo = &dmx->ChannelInfoTable[chid];
                   
               if ( !pChInfo->PESChannel && pChInfo->DemuxEnable )
               {
                  /*
                   * Save write ptr for reference
                   */
                  pChInfo->pWritePtr = (u_int8*)((*DPS_PID_SLOT_WRITE_PTR_EX(dmxid,chid)
                                                   & ~DPS_PAW_SYS_ADDRESS) | NCR_BASE);

                  /*
                   * figure out available data length
                   */
                  if ( pChInfo->pWritePtr >= pChInfo->pAckPtr )
                  {
                     bytes_available = (int32)( pChInfo->pWritePtr - pChInfo->pAckPtr );
                  }
                  else
                  {
                     bytes_available = (int32)( pChInfo->pWritePtr
                                                - pChInfo->pBuffer
                                                + pChInfo->pBufferEnd
                                                - pChInfo->pAckPtr );
                  }

                  section_length = 0xffffffff;

                  while ( bytes_available > 0 )
                  {
                     /*
                      * New data available on this channel
                      */

                     /*
                      * If there's a timer, stop it
                      */
                     if ( pChInfo->ChannelTimerActive )
                     {
                        if ( RC_OK != tick_stop ( pChInfo->ChannelTimer ) )
                        {
                           trace_new(DPS_ERROR_MSG,"DEMUX:tick_stop failed.\n");
                        }
                        pChInfo->ChannelTimerActive = FALSE;
                     }
                           
                     /*
                      * Set our flag indicating whether or not the client flushed the channnel 
                      * while processing a header notify or section notify callback.                    
                      */
                     pChInfo->CallbackFlushedChannel = FALSE;

                     /*
                      * Grab the AckPtr
                      */
                     section_ptr = start_ptr = (volatile u_int32 *) pChInfo->pAckPtr;
                     pCrcStatus = section_ptr; /* start to set up crc status pointer */

                     /*
                      * get the filter_mask
                      */
                     filter_mask = *section_ptr++;
                     filter_mask = BSWAP ( filter_mask );

                     if ( section_ptr >= (u_int32 *)pChInfo->pBufferEnd )
                     {
                        section_ptr = (volatile u_int32 *) pChInfo->pBuffer;
                     }

#if PARSER_TYPE==DTV_PARSER
                     /*
                      * APG frame length is defined by DIRECTV spec as 12 bits
                      * starting with bit 4 in byte 6 extending into byte 7
                      * formatted in BE
                      */
                     length_ptr = section_ptr + 1;
                     if ( length_ptr >= (u_int32*)pChInfo->pBufferEnd )
                     {
                        length_ptr = (volatile u_int32*)pChInfo->pBuffer;
                     }
                     section_length = *length_ptr;
                     section_length = BSWAP ( section_length );
                     section_length = ( section_length & 0xfff ) + 8;
#else
                     /*
                      * Section length is defined in the DVB spec as 12 bits
                      * starting with bit 4 in byte 2 extending into byte 3
                      * formatted in BE.
                      */
                     section_length = *section_ptr;
                     section_length = BSWAP ( section_length );
                     section_length = ( section_length >> 8 ) & 0xfff;
                     section_length += 3; /* plus table_id and section_length */
#endif

                     /*
                      * This is the length by which the Ack/Read ptr is
                      * updated or the amount of data the parser gave us.
                      * There is a 1 word filter_mask header followed by
                      * the section followed by 0-3 stuffing bytes.
                      */
                     total_length = ( section_length + 7 ) & (~3);

                     /* find pointer to crc status word at end of section and extract crc status */
                     AdvancePtr ( (u_int8**)&pCrcStatus, total_length,
                                   pChInfo->pBuffer, pChInfo->pBufferEnd );
                     pChInfo->CrcStatus = *pCrcStatus;
                     /* now adjust total length to include the crc status */
                     total_length += 4;

                     /*
                      * Setup structure for header notify call
                      */
                     pChInfo->NotifyData.pData     = pChInfo->HdrArea;
                     pChInfo->NotifyData.condition = 0;
                     pChInfo->NotifyData.length    = section_length;
                     pChInfo->NotifyData.skip      = 0;
                     #ifdef DVNCA
                     if((pChInfo->TagSet) && (pChInfo->Tag == 0x80000001))
                     	pChInfo->NotifyData.fid_mask = filter_mask&pChInfo->FilterEnable;
                     else
                        pChInfo->NotifyData.fid_mask  = filter_mask;
                     #else
                     pChInfo->NotifyData.fid_mask  = filter_mask;
                     #endif

                     /*
                      * Copy header area
                      */
                     WCopyBytes ( pChInfo->HdrArea, (u_int8 *)section_ptr,
                                  pChInfo->HdrSize, pChInfo->pBuffer,
                                  pChInfo->pBufferEnd );

#ifdef CRC_CHECK 
                     /*
                      *  If CRC/checksum is not correct, length
                      *  field could be wrong.  For this case
                      *  pChInfo->CrcStatus will not necessarily
                      *  be 0x3!!!  If CRC/checksum error happens,
                      *  we cannot locate next section/frame 
                      *  either.  We have to throw all the data 
                      *  up to WritePtr and re-sync AckPtr and 
                      *  ReadPtr to WritePtr.
                      */
                     if ( pChInfo->CrcStatus != 0x01000000 &&
                          pChInfo->CrcStatus != 0x00000000 )
                     {
                        /* CRC/checksum error */

                        if ( dmx->NotifyCRC )
                        {
                           /* notify CRC error */
                           if ( pChInfo->HdrErrNotify )
                           {
                              pChInfo->NotifyData.condition = GENDMX_ERROR;
                              if ( pChInfo->TagSet )
                              {
                                 pChInfo->NotifyData.tag = pChInfo->Tag;
                              }
                              if ( pChInfo->DemuxEnable )
                              {
                                 pChInfo->NotifyCalled++;
                                 pChInfo->LoopMode = pChInfo->HdrErrNotify(&(pChInfo->NotifyData));
                                 pChInfo->NotifyCalled--;
                              }
                           }
                        }

                        if ( ( pChInfo->CrcStatus == 0x03000000 ) && 
                             ( (int32)total_length < bytes_available ) )
                        {
                           /* length may be correct, try next frame */
                           AdvancePtr ( &pChInfo->pAckPtr, total_length,
                                        pChInfo->pBuffer, pChInfo->pBufferEnd );

                           if ( pChInfo->pReadPtr == (u_int8 *)start_ptr )
                           {
                              /* no locked section at all */
                              pChInfo->pReadPtr = pChInfo->pAckPtr;
   
                              /* Move hardware Read ptr */
                              *DPS_PID_SLOT_READ_PTR_EX(dmxid,chid) =
                                   (((u_int32)pChInfo->pReadPtr&~NCR_BASE)|DPS_PAW_SYS_ADDRESS);
                           }
                           else
                           {
                              /* locked section before */
                              *start_ptr++ = 0xffff0000 | ( total_length & 0xffff );
   
                              /* this section is not locked, mark it */
                              if ( start_ptr >= (u_int32 *)pChInfo->pBufferEnd )
                              {
                                 start_ptr = (u_int32 *)pChInfo->pBuffer;
                              }
                              *start_ptr = 0xffffffff;
                           }

                           bytes_available  -= (int32)total_length;
                        }
                        else
                        {
                           /* length is wrong, throw away data and re-sync pointers */
                           pChInfo->pAckPtr = pChInfo->pWritePtr;

                           if ( pChInfo->pReadPtr == (u_int8 *)start_ptr )
                           {
                              /* no locked section at all */
                              pChInfo->pReadPtr = pChInfo->pAckPtr;

                              /* Move hardware Read ptr */
                              *DPS_PID_SLOT_READ_PTR_EX(dmxid,chid) =
                                   (((u_int32)pChInfo->pReadPtr&~NCR_BASE)|DPS_PAW_SYS_ADDRESS);
                           }
                           else
                           {
                              /*
                               * locked section before, mark remaining data as one 
                               * unlocked section till ack ptr
                               */
                              *start_ptr++ = 0xffff0000 | ( bytes_available & 0xffff );
                              if ( start_ptr >= (u_int32 *)pChInfo->pBufferEnd )
                              {
                                 start_ptr = (u_int32 *)pChInfo->pBuffer;
                              }
                              *start_ptr = 0xffffffff;
                           }
                           bytes_available   = 0;
                        }

                        continue;
                     }
#endif
                     /*
                      * Call the client header nofify function
                      */
                     if ( pChInfo->DemuxEnable )
                     {
                        if ( pChInfo->HdrErrNotify )
                        {
                           if ( pChInfo->TagSet )
                           {
                              pChInfo->NotifyData.tag = pChInfo->Tag;
                           }
                           pChInfo->NotifyCalled++;
                           pChInfo->LoopMode = pChInfo->HdrErrNotify(&(pChInfo->NotifyData));
                           pChInfo->NotifyCalled--;
                        }
                     }
                     else
                     {
                        /* channel disabled, client don't need any data */
                        pChInfo->NotifyData.length = 0;
                     }
                        
                     section_locked = FALSE;
                            
                     if ( pChInfo->NotifyData.length )
                     {
                        /*
                         * The client wants the section
                         */

                        if ( pChInfo->NotifyData.skip )
                        {
                           /*
                            * Process the skip
                            */

                           AdvancePtr((u_int8 **)&section_ptr, pChInfo->NotifyData.skip,
                                      pChInfo->pBuffer, pChInfo->pBufferEnd);
                           pChInfo->NotifyData.skip = 0;
                        }

                        /*
                         * Update with new section length from the client.
                         */
                        if ( pChInfo->NotifyData.length <= section_length )
                        {
                           /*
                            * If client requested less than we offered, we must
                            * copy that amount.
                            */

                           section_length = pChInfo->NotifyData.length;
                        }
                        else
                        {
                           /*
                            * If client requested more than we offered, we can
                            * round up to a word boundary if we need to.
                            */

                           if (((section_length+3)&~0x3) <= pChInfo->NotifyData.length)
                           {
                              section_length = (section_length+3)&~0x3;
                           }
                        }

                        /*
                         * Did client provide copy buffer?
                         */
                        if (pChInfo->NotifyData.write_ptr)
                        {
                           /*
                            * Client provided buffer to copy section to
                            */

                           WCopyBytes(pChInfo->NotifyData.write_ptr, (u_int8 *)section_ptr,
                                      section_length, pChInfo->pBuffer,
                                      pChInfo->pBufferEnd);

                           /*
                            * Call the section notify function now
                            */

                           gen_dmx_section_notify ( dmxid,
                                                    (u_int8*)pChInfo->NotifyData.write_ptr,
                                                    chid,
                                                    filter_mask );
                        }
                        else
                        {
                           section_locked  = gen_dmx_section_notify ( dmxid,
                                                                      (u_int8 *)section_ptr,
                                                                      chid,
                                                                      filter_mask );
                        }
                     }

                     if ( pChInfo->CallbackFlushedChannel )
                     {
                        bytes_available = 0;
                     }
                     else
                     {
                        /*
                         * Indicate the section is ack'd 
                         */
                        AdvancePtr ( &pChInfo->pAckPtr, total_length,
                                     pChInfo->pBuffer, pChInfo->pBufferEnd );

                        if ( pChInfo->pReadPtr == (u_int8 *)start_ptr )
                        {
                           /* no locked section before */
                           if ( section_locked )
                           {
                              /* this section locked */
                              *start_ptr = 0xffff0000 | ( total_length & 0xffff );
                              pChInfo->Notify_Unlock++;
                           }
                           else
                           {
                              /* no locked section at all */
                              pChInfo->pReadPtr = pChInfo->pAckPtr;

                              /* Move hardware Read ptr */
                              *DPS_PID_SLOT_READ_PTR_EX(dmxid,chid) =
                                   (((u_int32)pChInfo->pReadPtr&~NCR_BASE)|DPS_PAW_SYS_ADDRESS);
                           }
                        }
                        else
                        {
                           /* locked section before */
                           *start_ptr = 0xffff0000 | ( total_length & 0xffff );

                           if ( section_locked == FALSE )
                           {
                              /* this section is not locked, mark it */
                              start_ptr ++;
                              if ( start_ptr >= (u_int32 *)pChInfo->pBufferEnd )
                              {
                                 start_ptr = (u_int32 *)pChInfo->pBuffer;
                              }
                              *start_ptr = 0xffffffff;
                           }
                           else
                           {
                              pChInfo->Notify_Unlock++;
                           }
                        }

                        /* decrease bytes available */
                        bytes_available -= total_length;
                     }

                  } /* while ( bytes_available > 0 ) */

               } /* if ((!pChInfo->PESChannel)... */

               /*
                * Handle timeouts
                */
               if ( pChInfo->TimerNotifyCount )
               {
                  if ( section_length == 0xffffffff )
                  {
                     /*
                      * If a timeout occurred for this channel and we didn't
                      * process any data for this channel just now, send a
                      * TIMEOUT header notify.
                      */

                     pChInfo->NotifyData.pData     = (u_int8 *) NULL;
                     pChInfo->NotifyData.condition = GENDMX_CHANNEL_TIMEOUT;
                     if ( ( pChInfo->HdrErrNotify ) && pChInfo->DemuxEnable )
                     {
                        if ( pChInfo->TagSet )
                        {
                           pChInfo->NotifyData.tag = pChInfo->Tag;
                        }
                        pChInfo->NotifyCalled++;
                        pChInfo->LoopMode = pChInfo->HdrErrNotify(&(pChInfo->NotifyData));
                        pChInfo->NotifyCalled--;
                     }

                     if ( pChInfo->LoopMode == GENDEMUX_ONE_SHOT )
                     {
                        /*
                         * If a timeout occurred and client returned ONE_SHOT,
                         * disable channel
                         */
                        if ( cnxt_dmx_channel_control ( dmxid, chid,
                                      GEN_DEMUX_DISABLE) != DMX_STATUS_OK )
                        {
                           trace_new ( DPS_ERROR_MSG,
                               "DEMUX: cnxt_dmx_control_channel failed\n" );
                        }
                     }
                     else
                     {
                        /*
                         * If there's a timer, it's not ONE_SHOT mode, and
                         * it's off, restart it...
                         */
                        if ( RC_OK != tick_start ( pChInfo->ChannelTimer ) )
                        {
                           trace_new ( DPS_ERROR_MSG,"DEMUX:tick_start failed.\n" );
                        }
                     }
                  }

                  /*
                   * Indicate timeout handled
                   */
                  pChInfo->TimerNotifyCount = 0;
               }

            } /* for chid  */

            /* Now that we've completed PSI processing for one demux, clear the bitmask, but */
            /* make sure the clear is in a critical section so no other interrupts could be   */
            /* missed while we are clearing the bit. */
            ks = critical_section_begin();            
            PSI_int_mask &= ~(1<<dmxid);
            critical_section_end(ks);

            /*
             * Ensure mutual exclusion to the control and unlock functions
             */
            sem_put ( GenDmxPSITaskProcSem );

         } /* if PSI_int_mask */

      } /* for dmxid */

   } /* while ( 1 ) */
}

#ifdef TRACK_PSI_PACKET_LOSS
void gen_demux_psi_packet_loss_timer_call_back (timer_id_t timer, void *pUserData)
{
    PSIBufferFullDropCount = *((volatile u_int32 *) DPS_DBG_DUMP_PSI_BUFF_FULL_DROP_CNT);
    PSICRCDropCount        = *((volatile u_int32 *) DPS_DBG_DUMP_PSI_CRC_DROP_CNT);
    PSIPacketLossDropCount = *((volatile u_int32 *) DPS_DBG_DUMP_PSI_PKT_LOSS_DROP_CNT);
    PSIFilterDropCount     = *((volatile u_int32 *) DPS_DBG_DUMP_PSI_FILTER_DROP_CNT);
    PSISyntaxDropCount     = *((volatile u_int32 *) DPS_DBG_DUMP_PSI_SYNTAX_DROP_CNT);
}
#endif
/*
 *
$Log: 
 23   mpeg      1.22        3/31/04 4:41:54 PM     Yong Lu         CR(s) 8716 
       8717 : re-instate the bypass of PSI CRC check for IPSTB
 22   mpeg      1.21        1/26/04 2:40:28 PM     Larry Wang      CR(s) 8271 :
        Remove temporary code that is not necessary with this fix.
       
 21   mpeg      1.20        1/12/04 6:30:44 PM     Yong Lu         CR(s) 8078 :
        temporary code for IPSTB CSE build, will be removed late
 20   mpeg      1.19        10/24/03 11:11:21 AM   Tim White       CR(s): 7713 
       Rearrange #ifdef PARSER_TYPE to be consistent with local variable 
       declaration and
       also make it the same as what's in codeldr mdemux.c
       
 19   mpeg      1.18        9/2/03 7:03:14 PM      Joe Kroesche    SCR(s) 7415 
       :
       removed unneeded header files to eliminate extra warnings when building
       for PSOS
       
 18   mpeg      1.17        7/3/03 1:14:34 PM      Bob Van Gulick  SCR(s) 6886 
       :
       Change CRC check to also accept 0 as a valid crc notify.  This indicates
        that there was no CRC in the section and therefore was not checked.
       
       
 17   mpeg      1.16        7/2/03 5:03:42 PM      Larry Wang      SCR(s) 6867 
       :
       Don't call the callbacks once the PSI task sees the channel is disabled.
       
 16   mpeg      1.15        6/24/03 2:40:40 PM     Miles Bintz     SCR(s) 6822 
       :
       added initialization value to remove warning in release build
       
 15   mpeg      1.14        6/10/03 3:44:10 PM     Larry Wang      SCR(s) 6758 
       :
       a minor change over the last revision.
       
 14   mpeg      1.13        6/10/03 3:24:56 PM     Larry Wang      SCR(s) 6758 
       :
       (1) change gen_dmx_PSI_Task to handle the loss of frame sync due to CRC 
       error;
       (2) change gen_dmx_section_unlock to support multiple locked sections.
       
 13   mpeg      1.12        4/10/03 5:02:36 PM     Dave Wilson     SCR(s) 5990 
       :
       
       
       
       Added cnxt_dmx_set_section_channel_tag API.d
       
       
 12   mpeg      1.11        4/2/03 11:58:14 AM     Brendan Donahe  SCR(s) 5886 
       :
       Modifications to support 6 simultaneous unscrambled/scrambled SI 
       including
       channels usually used for video and audio (1 & 2) as well as enforcing 
       correct
       parser microcode version which has been changed in conjunction with this
        
       feature enhancement.
       
       
 11   mpeg      1.10        3/28/03 11:53:26 AM    Larry Wang      SCR(s) 5904 
       :
       Add code to handle DIRECTV APG frame checksum error.
       
 10   mpeg      1.9         3/20/03 10:02:00 AM    Larry Wang      SCR(s) 5833 
       :
       For Directv APG frame, set the pointer correctly to the length field.
       
 9    mpeg      1.8         3/14/03 10:03:20 AM    Larry Wang      SCR(s) 5766 
       :
       Figure out the length of APG frame in PSI task.
       
 8    mpeg      1.7         3/5/03 5:17:30 PM      Dave Wilson     SCR(s) 5667 
       5668 :
       PSI task reworked to check to see if anyone called 
       cnxt_dmx_channel_control
       and disabled/flushed the channel from the section notify callback. If 
       this is
       detected, the PSI task no longer mucks with the channel buffer pointers 
       but
       merely goes on to process the next channel. This should avoid the 
       possibility
       of pointer corruption in such cases.
       
 7    mpeg      1.6         2/13/03 11:42:18 AM    Matt Korte      SCR(s) 5479 
       :
       Removed old header reference
       
 6    mpeg      1.5         9/19/02 3:42:44 PM     Joe Kroesche    SCR(s) 4610 
       :
       added crc notification feature, removed changes for previous crc 
       notification
       method. NOTE!!! requires matching pawser ucode update of #4626
       
 5    mpeg      1.4         9/5/02 6:29:54 PM      Bob Van Gulick  SCR(s) 4530 
       :
       Change CRC check to use Header Notify instead of Section Notify
       
       
 4    mpeg      1.3         8/30/02 2:43:32 PM     Bob Van Gulick  SCR(s) 4485 
       :
       Add support for CRC checking of SI packets
       
       
 3    mpeg      1.2         8/5/02 11:55:10 AM     Tim White       SCR(s) 4330 
       :
       Fixed timeout and single shot (ONE_SHOT) capabilities.
       
       
 2    mpeg      1.1         6/27/02 5:57:18 PM     Tim White       SCR(s) 4108 
       :
       Convert MHP glue layer to use new DEMUX driver.
       
       
 1    mpeg      1.0         12/18/01 11:01:02 AM   Bob Van Gulick  
$
 * 
 *    Rev 1.18   02 Sep 2003 18:03:14   kroescjl
 * SCR(s) 7415 :
 * removed unneeded header files to eliminate extra warnings when building
 * for PSOS
 * 
 *    Rev 1.17   03 Jul 2003 12:14:34   vangulr
 * SCR(s) 6886 :
 * Change CRC check to also accept 0 as a valid crc notify.  This indicates that there was no CRC in the section and therefore was not checked.
 * 
 * 
 *    Rev 1.16   02 Jul 2003 16:03:42   wangl2
 * SCR(s) 6867 :
 * Don't call the callbacks once the PSI task sees the channel is disabled.
 * 
 *    Rev 1.15   24 Jun 2003 13:40:40   bintzmf
 * SCR(s) 6822 :
 * added initialization value to remove warning in release build
 * 
 *    Rev 1.14   10 Jun 2003 14:44:10   wangl2
 * SCR(s) 6758 :
 * a minor change over the last revision.
 * 
 *    Rev 1.13   10 Jun 2003 14:24:56   wangl2
 * SCR(s) 6758 :
 * (1) change gen_dmx_PSI_Task to handle the loss of frame sync due to CRC error;
 * (2) change gen_dmx_section_unlock to support multiple locked sections.
 * 
 *    Rev 1.12   10 Apr 2003 16:02:36   dawilson
 * SCR(s) 5990 :
 * 
 * 
 * 
 * Added cnxt_dmx_set_section_channel_tag API.d
 * 
 * 
 *    Rev 1.11   02 Apr 2003 11:58:14   donaheb
 * SCR(s) 5886 :
 * Modifications to support 6 simultaneous unscrambled/scrambled SI including
 * channels usually used for video and audio (1 & 2) as well as enforcing correct
 * parser microcode version which has been changed in conjunction with this 
 * feature enhancement.
 * 
 * 
 *    Rev 1.10   28 Mar 2003 11:53:26   wangl2
 * SCR(s) 5904 :
 * Add code to handle DIRECTV APG frame checksum error.
 * 
 *    Rev 1.9   20 Mar 2003 10:02:00   wangl2
 * SCR(s) 5833 :
 * For Directv APG frame, set the pointer correctly to the length field.
 * 
 *    Rev 1.8   14 Mar 2003 10:03:20   wangl2
 * SCR(s) 5766 :
 * Figure out the length of APG frame in PSI task.
 * 
 *    Rev 1.7   05 Mar 2003 17:17:30   dawilson
 * SCR(s) 5667 5668 :
 * PSI task reworked to check to see if anyone called cnxt_dmx_channel_control
 * and disabled/flushed the channel from the section notify callback. If this is
 * detected, the PSI task no longer mucks with the channel buffer pointers but
 * merely goes on to process the next channel. This should avoid the possibility
 * of pointer corruption in such cases.
 * 
 *    Rev 1.6   13 Feb 2003 11:42:18   kortemw
 * SCR(s) 5479 :
 * Removed old header reference
 * 
 *    Rev 1.5   19 Sep 2002 14:42:44   kroescjl
 * SCR(s) 4610 :
 * added crc notification feature, removed changes for previous crc notification
 * method. NOTE!!! requires matching pawser ucode update of #4626
 * 
 *    Rev 1.4   05 Sep 2002 17:29:54   vangulr
 * SCR(s) 4530 :
 * Change CRC check to use Header Notify instead of Section Notify
 * 
 *    Rev 1.3   30 Aug 2002 13:43:32   vangulr
 * SCR(s) 4485 :
 * Add support for CRC checking of SI packets
 * 
 *    Rev 1.2   05 Aug 2002 10:55:10   whiteth
 * SCR(s) 4330 :
 * Fixed timeout and single shot (ONE_SHOT) capabilities.
 * 
 *    Rev 1.1   27 Jun 2002 16:57:18   whiteth
 * SCR(s) 4108 :
 * Convert MHP glue layer to use new DEMUX driver.
 * 
 *    Rev 1.0   18 Dec 2001 11:01:02   vangulr
 * SCR(s) 2977 :
 * Initial drop
 * 
*/

