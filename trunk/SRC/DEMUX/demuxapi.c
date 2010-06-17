/****************************************************************************/
/*                   Conexant Proprietary and Confidential                  */
/*                       Software File/Module Header                        */
/*          Conexant Systems Inc. (c) 1998, 1999, 2000, 2001, 2002          */
/*                               Austin, TX                                 */
/*                            All Rights Eeserved                           */
/****************************************************************************/
/*
 * Filename:       demuxapi.c
 *
 *
 * Description:    This file contains the generic interface to the gendemux
 *                 driver
 *
 * Author:         Bob Van Gulick 
 *
 ****************************************************************************/
/* $Id: demuxapi.c,v 1.70, 2004-05-24 16:58:00Z, Larry Wang$
 ****************************************************************************/

#include "stbcfg.h"
#include "basetype.h"
#include "globals.h"
#include "kal.h"
#include "startup.h"
#include "retcodes.h"
#include "demuxapi.h"
#include "clkrec.h"
#include "demuxint.h"

/*
 * This is required for setting up the NIM Extender when building with
 * CUSTOMER == VENDOR_B on a Conexant box (e.g. Klondike)
 */
#if CUSTOMER == VENDOR_B
    #include <gpioext.h>
extern void IICInit(void);
#endif                     

extern int DemuxIRQHandler0(u_int32 IntID, bool Fiq, PFNISR *pfnChain);
extern int DemuxIRQHandler2(u_int32 IntID, bool Fiq, PFNISR *pfnChain);
extern int DemuxIRQHandler3(u_int32 IntID, bool Fiq, PFNISR *pfnChain);
extern task_id_t PSIProcessID;
extern task_id_t XprtProcessID;
#define PES_TASK_ID XprtProcessID
DemuxInfo gDemuxInfo[MAX_DEMUX_UNITS];
bool      gDemuxInitialized = FALSE;    /* So we init demux structures on first open call */
u_int32   PSI_int_mask;
u_int32   ECM_int_mask;
u_int32   Xprt_int_mask;
#if PARSER_TYPE==DTV_PARSER
u_int32   CWP_int_mask;
u_int32   CAP_int_mask;
#endif

bool NewPCRFlag;

#ifdef DEBUG
extern u_int32 uTraceFlags;
#endif

/* Semaphores */
sem_id_t PSI_Sem_ID;
sem_id_t Xprt_Sem_ID;
#if PARSER_TYPE==DTV_PARSER
sem_id_t CWP_Sem_ID;
sem_id_t CAP_Sem_ID;
#endif
sem_id_t GenDmxPSITaskProcSem;
sem_id_t GenDmxPESTaskProcSem;

/* PCR Callback */
gen_pcr_callback_fct_t gPCRCallBack;

/* Event 6 callback */
gen_event6_callback_fct_t gEvent6CallBack;

/* External references */
#if defined DRIVER_INCL_NDSDEMUX || defined DRIVER_INCL_NDSICAM
extern void dmx_nds_init(bool ReInit);
extern void NotifyCAConnectNotify(u_int32 dmxid, bool connected);
#endif

#ifdef IPANEL_SEM_LOCKED_TEST1 
int flag_pop_sec_to_callback = 0 ; 
#endif

/*****************************************************************************
 * cnxt_dmx_open
 *
 * parameters:
 *    ReInit - a boolean indicating if we are re-initializing the hardware
 *    capabilities - a bitmask specifying the required capabilities for the
 *          demux hardware.  Multiple capabilities can be OR'd together.
 *          See below for capability list.
 *    dmxid - caller supplied storage for the demux hardware instance
 *          handle.  This value should be used for all other API functions
 *          to refer to the demux hardware instance.  This value is only
 *          valid if DMX_STATUS_OK is returned by the function.
 *
 * returns:
 *    demux hardware instance handle is returned through dmxid
 *    DMX_STATUS_OK - no errors
 *    DMX_ERROR - no demux hardware with the specified capabilities is
 *          available.  Either the hardware has already been allocated or
 *          the caller specified and impossible combination of capabilities
 *
 * description:
 *    Opens a demultiplexer hardware instance.  The caller specifies
 *    capabilities and demux hardware with the specified capabilities
 *    is allocated, initialized and an instance handle is returned
 * capabilities:
 *    DMX_CAP_PLAYBACK - able to play audio/video to the decoders
 *    DMX_CAP_RECORD - able to record to disk
 *    DMX_CAP_DESCRAMBLE - able to descramble
 ****************************************************************************/
DMX_STATUS cnxt_dmx_open(bool ReInit, u_int32 capabilities, u_int32* dmxid) {
    DemuxInfo *dmx;
    int   dmx_alloc, dmx_temp, i;

    /* uTraceFlags |= DPS_FUNC_TRACE; */

    trace_new(DPS_FUNC_TRACE,"DEMUX:cnxt_dmx_open entered\n");

    /* On the first ever open of a demux, we need to init all the DemuxInfo structures to  */
    /* be unassigned.  Also need to set up some static/global variables */
    if (!gDemuxInitialized) {
        for (i=0; i<=MAX_DEMUX_UNITS-1; i++) {
            gDemuxInfo[i].DemuxID = DEMUX_FREE;
        }

#if MAX_DEMUX_UNITS > 1

        /* For index 1 in the gDemuxInfo array, I will set the DemuxID to 0xdeadbeef so it never gets used */
        gDemuxInfo[1].DemuxID = 0xdeadbeef;
#endif

        /* Clear the bitmasks for interrupt processing */
        PSI_int_mask = ECM_int_mask = Xprt_int_mask = 0x0;
#if PARSER_TYPE==DTV_PARSER
        CWP_int_mask = 0x0;
        CAP_int_mask = 0x0;
#endif
        NewPCRFlag = FALSE;
    }

    /* Sanity check capabilities request */
    if((capabilities & DMX_CAP_PLAYBACK) && (capabilities & DMX_CAP_RECORD))
    {
        trace_new(DPS_ERROR_MSG,"cnxt_dmx_open: Capabilities mismatch = 0x%0x\n", capabilities);
        return DMX_ERROR;
    }

    dmx_alloc = -1;
    /* Lets check the capabilities of this demux and allocate the proper one */    
    if (!(capabilities & DMX_CAP_RECORD)) { /* Must be demux 0 */
        if (gDemuxInfo[0].DemuxID == DEMUX_FREE ||
            (gDemuxInfo[0].DemuxID != DEMUX_FREE &&
             ReInit == TRUE)) {
            dmx_alloc = 0;
        } else {
            trace_new(DPS_ERROR_MSG,"cnxt_dmx_open: Attempt to open multiple playback demuxes\n");
            return DMX_ERROR;
        } 
    } else {
        /* Ok, not a playback demux.  Lets look for a non-playback demux if we're tri-demux */
        /* Start at index 1 to skip the playback demux.                                         */
        for (dmx_temp=1; dmx_temp<MAX_DEMUX_UNITS; dmx_temp++) {
            if (gDemuxInfo[dmx_temp].DemuxID == DEMUX_FREE ||
                (gDemuxInfo[dmx_temp].DemuxID != DEMUX_FREE &&
                 ReInit == TRUE)) {
                dmx_alloc = dmx_temp;  /* got a free demux */
                break;
            }
        }

        if (dmx_alloc == -1) { /* Special case: If we still did not allocate a demux, that means no  */
                               /* non-descrambling slots were available. But demux 0 might still be  */
                               /* available.  So lets use it as a non-descrambled demux if it's free */
            if (gDemuxInfo[0].DemuxID == DEMUX_FREE) {
                dmx_alloc = 0;
            } else {
                trace_new(DPS_ERROR_MSG,"cnxt_dmx_open: no demux available to open\n");
                return DMX_ERROR;
            }
        }

    }
    /* For debugging purposes */
    /* dmx_alloc = 2; */

    dmx = &gDemuxInfo[dmx_alloc];

    /* Assign the all important DemuxID  */
    *dmxid = dmx->DemuxID = dmx_alloc;

    /* Keep track of the capabilities requested */
    dmx->CapabilitiesRequested = capabilities;

    /* Initialize the SW */
    if (genDemuxInitSW(dmx_alloc) != TRUE) {
        trace_new(DPS_ERROR_MSG,"cnxt_dmx_open: genDemuxInitSW failed\n");
        return DMX_ERROR;
    }

    /* Initialize the HW */
    if (genDemuxInitHardware(dmx_alloc, ReInit) != TRUE) {
        trace_new(DPS_ERROR_MSG,"cnxt_dmx_open: genDemuxInitHardware failed\n");
        return DMX_ERROR;
    }

    if (cnxt_dmx_descrambler_init(dmx_alloc) != DMX_STATUS_OK) {
        trace_new(DPS_ERROR_MSG,"cnxt_dmx_open: cnxt_dmx_descrambler_init failed\n");
        return DMX_ERROR;
    }

    if (!gDemuxInitialized) {
        gDemuxInitialized = TRUE;
#if defined DRIVER_INCL_NDSDEMUX || defined DRIVER_INCL_NDSICAM
    /* Only want to init NDS on first demux allocated */
        dmx_nds_init(FALSE);             /* initialize NDS HW */
#endif
    }

    dmx->DemuxInitialized = TRUE;

    trace_new(DPS_FUNC_TRACE,"DEMUX:cnxt_dmx_open Done\n");
    return DMX_STATUS_OK;
}

/*****************************************************************************
 * cnxt_dmx_close
 *
 * parameters:
 *    dmxid - demux hardware instance handle that was returned by _open()
 *
 * returns:
 *    DMX_STATUS_OK - no errors
 *    DMX_ERROR - the specified instance handle is not valid
 *          (perhaps not opened or already closed)
 *
 * description:
 *    closes a previously opened demux hardware instance.  The hardware
 *    will be shutdown and all allocated software structures are deleted.
 *    The hardware instance will then be available to be opened again.
 ****************************************************************************/
DMX_STATUS cnxt_dmx_close(u_int32 dmxid) {

    trace_new(DPS_FUNC_TRACE,"DEMUX:cnxt_dmx_close entered\n");

    if(dmxid >= MAX_DEMUX_UNITS || dmxid == 1) { /* Sanity check dmxid value, avoid demux 1 & going
                                                    outside array bounds in next check */
      trace_new(DPS_ERROR_MSG, "DEMUX:cnxt_dmx_close: Attempt to close non-existant demux!\n");
      return DMX_ERROR;
    }

    /* First check to see that this demux id is already allocated */
    if (!gDemuxInfo[dmxid].DemuxInitialized) {
        trace_new(DPS_ERROR_MSG,"DEMUX:cnxt_dmx_close: Attempt to close un-initialized demux!\n");
        return DMX_ERROR;
    }

    gDemuxInfo[dmxid].DemuxID = DEMUX_FREE;  /* Deallocate this DemuxInfo structure 
                                                Set this BEFORE holding parser in reset! */

    gen_dmx_shutdown(dmxid);

    return DMX_STATUS_OK;
}

/*****************************************************************************
 * cnxt_dmx_query_status
 *
 * parameters:
 *    dmxid - demux hardware instance handle that was returned by _open()
 *
 * returns:
 *    DMX_STATUS_OK - dmxid open and ready
 *    DMX_ERROR - dmxid is not open or ready
 *
 * description:
 *    closes a previously opened demux hardware instance.  The hardware
 *    will be shutdown and all allocated software structures are deleted.
 *    The hardware instance will then be available to be opened again.
 ****************************************************************************/
DMX_STATUS cnxt_dmx_query_status(u_int32 dmxid) {

 /* trace_new(DPS_FUNC_TRACE,"DEMUX:cnxt_dmx_query_status entered\n"); */

    if(dmxid >= MAX_DEMUX_UNITS || dmxid == 1) { /* Sanity check dmxid value, avoid demux 1 & going
                                                    outside array bounds in next check */
   /* trace_new(DPS_ERROR_MSG, "DEMUX:cnxt_dmx_query_status: Attempt to query non-existant demux!\n"); */
      return DMX_ERROR;
    }

    /* First check to see that this demux id is already allocated */
    if (!gDemuxInfo[dmxid].DemuxInitialized ||
        gDemuxInfo[dmxid].DemuxID == DEMUX_FREE) {
        return DMX_ERROR;
    }

    return DMX_STATUS_OK;
}

/*****************************************************************************
 * cnxt_dmx_reset
 *
 * parameters:
 *    dmxid - demux hardware instance handle that was returned by _open()
 *
 * returns:
 *    DMX_STATUS_OK - no errors
 *    DMX_ERROR - the specified instance handle is not valid
 *          (perhaps not opened or already closed)
 *
 * description:
 *    Reset and re-initializes the specified demux hardware instance.
 *    This function should only be used as a last resort to attempt to
 *    recover from a serious error condition.
 *    The software state (allocated channels, PIDs, filters, etc.) will
 *    not be re-initialized.  After the hardware is reset, it will be
 *    re-initialized according to the software state, not the defaults.
 ****************************************************************************/
DMX_STATUS cnxt_dmx_reset(u_int32 dmxid) {

    trace_new(DPS_FUNC_TRACE,"DEMUX:cnxt_dmx_reset entered\n");

    if(dmxid >= MAX_DEMUX_UNITS || dmxid == 1) { /* Sanity check dmxid value, avoid demux 1 & going
                                                    outside array bounds in next check */
      trace_new(DPS_ERROR_MSG, "DEMUX:cnxt_dmx_reset: demux %d does not exist!\n",dmxid);
      return DMX_BAD_DMXID;
    }

    /* First check to see that this demux id is already allocated */
    if (!gDemuxInfo[dmxid].DemuxInitialized) {
        trace_new(DPS_ERROR_MSG,"cnxt_dmx_reset: Attempt to close non-existent demux!\n");
        return DMX_ERROR;
    }

    genDemuxInitHardware(dmxid, TRUE);

    return DMX_STATUS_OK;
}

/*****************************************************************************
 * cnxt_dmx_channel_open
 *
 * parameters:
 *    dmxid - demux hardware instance handle that was returned by _open()
 *    capabilities - a bitmask specifying the required capabilities for the
 *          channel.  Multiple capabilities can be OR'd together.
 *          See below for capability list.
 *    chtype - The type of channel to allocate as defined in the enum genchannel_t
 *    chid - caller supplied storage for the channel instance.  This value
 *          is not valid if the function return value is not DMX_STATUS_OK
 *
 * returns:
 *    channel instance handle is returned through chid
 *    DMX_STATUS_OK - no errors
 *    DMX_BAD_DMXID - the specified demux instance handle is not valid
 *    DMX_CH_UNAVAILABLE - no channel with the specified capabilities
 *          could be allocated
 *
 * description:
 *    Opens and initializes a channel instance.  The caller specifies
 *    the channel capabilities and a channel instance is allocated
 *    and initialized.
 * capabilities:
 *    DMX_CH_CAP_DESCRAMBLING - able to descramble
 *    DMX_CH_CAP_HWFILTER - able to perform hardware filtering - not used?
 *    DMX_CH_CAP_SWFILTER - able to perform software filtering - not used?
 ****************************************************************************/
DMX_STATUS cnxt_dmx_channel_open(u_int32 dmxid, u_int32 capabilities, 
                                 genchannel_t channel_type, u_int32* chid)
{
    int i;
    DemuxInfo *dmx;
    u_int32 num_aud_chans;

    *chid = GENDMX_BAD_CHANNEL;
    trace_new(DPS_FUNC_TRACE,"DEMUX:cnxt_dmx_channel_open entered\n");

    /*
     * Sanity check dmxid value, avoid demux 1 & going
     * outside array bounds in next check
     */
    if(dmxid >= MAX_DEMUX_UNITS || dmxid == 1)
    {
      trace_new(DPS_ERROR_MSG, "DEMUX:cnxt_dmx_channel_open: demux %d does not exist!\n", dmxid);
      return DMX_BAD_DMXID;
    }

    /*
     * First check to see that this demux id is already allocated
     */
    if (!gDemuxInfo[dmxid].DemuxInitialized)
    {
        trace_new(DPS_ERROR_MSG,
            "DEMUX:cnxt_dmx_channel_open: demux %d is not initialized!\n", dmxid);
        return DMX_BAD_DMXID;
    }

    dmx = &gDemuxInfo[dmxid];

    switch (channel_type)
    {
        /*
         * Special case for live video channel (chid = 1)
         */
        case VIDEO_PES_TYPE:

            if (dmx->ChannelInfoTable[VIDEO_CHANNEL].InUseFlag == CHANNEL_FREE)
            {
                *chid = VIDEO_CHANNEL;
                dmx->ChannelInfoTable[VIDEO_CHANNEL].InUseFlag = CHANNEL_IN_USE;
                dmx->ChannelInfoTable[VIDEO_CHANNEL].PESChannel = TRUE;
                dmx->ChannelInfoTable[VIDEO_CHANNEL].stype = channel_type;
                dmx->DescChannelsAvail--;
                dmx->ChannelInfoTable[VIDEO_CHANNEL].TagSet = FALSE;
                trace_new(DPS_FUNC_TRACE, "DEMUX:Allocated VIDEO channel %d\n", i);
            }
            else
            {
                trace_new(DPS_ERROR_MSG,
                  "DEMUX:cnxt_dmx_channel_open failed - VIDEO_PES_TYPE channel already in use\n");
                return DMX_CH_UNAVAILABLE;
            }
            break;

        /*
         * Special case for live audio channel (chid = 2)
         */
        case AUDIO_PES_TYPE:

            if (dmx->ChannelInfoTable[AUDIO_CHANNEL].InUseFlag == CHANNEL_FREE)
            {
                *chid = AUDIO_CHANNEL;
                dmx->ChannelInfoTable[AUDIO_CHANNEL].InUseFlag = CHANNEL_IN_USE;
                dmx->ChannelInfoTable[AUDIO_CHANNEL].PESChannel = TRUE;
                dmx->ChannelInfoTable[AUDIO_CHANNEL].stype = channel_type;
                dmx->ChannelInfoTable[AUDIO_CHANNEL].TagSet = FALSE;
                dmx->DescChannelsAvail--;
                trace_new(DPS_FUNC_TRACE, "DEMUX:Allocated AUDIO channel %d\n", i);
            }
#ifdef HWBUF_ENCAC3_ADDR
            else if (dmx->ChannelInfoTable[AUDIO_CHANNEL+1].InUseFlag == CHANNEL_FREE)
            {
                *chid = AUDIO_CHANNEL+1;
                dmx->ChannelInfoTable[AUDIO_CHANNEL+1].InUseFlag = CHANNEL_IN_USE;
                dmx->ChannelInfoTable[AUDIO_CHANNEL+1].PESChannel = TRUE;
                dmx->ChannelInfoTable[AUDIO_CHANNEL+1].stype = channel_type;
                dmx->ChannelInfoTable[AUDIO_CHANNEL+1].TagSet = FALSE;
                dmx->DescChannelsAvail--;
                trace_new(DPS_FUNC_TRACE, "DEMUX:Allocated AUDIO channel %d\n", i);
            }
#endif
            else
            {
                trace_new(DPS_ERROR_MSG,
                  "DEMUX:cnxt_dmx_channel_open failed - AUDIO_PES_TYPE channel already in use\n");
                return DMX_CH_UNAVAILABLE;
            }
            break;

        /*
         * General case for PSI & PES
         */
        case PSI_CHANNEL_TYPE:
        case PES_CHANNEL_TYPE:
        case  ES_CHANNEL_TYPE:

            /*
             * Look for a non-descrambler capable slot if no descrambling is required
             */
            if (!(capabilities & DMX_CH_CAP_DESCRAMBLING))
            {
                /*
                 * Not requesting a descrambling channel
                 */
                for (i=DPS_NUM_DESCRAMBLERS; i<TOTAL_CHANNELS; ++i)
                {
                    /*
                     * Chid's DPS_NUM_DESCRAMBLERS - 31 do not support descrambling
                     */
                    if (dmx->ChannelInfoTable[i].InUseFlag == CHANNEL_FREE)
                    {
                        dmx->ChannelInfoTable[i].InUseFlag = CHANNEL_IN_USE;
                        dmx->ChannelInfoTable[i].stype = channel_type;
                        dmx->ChannelInfoTable[i].TagSet = FALSE;
                        if(channel_type == PSI_CHANNEL_TYPE)
                        {
                            dmx->ChannelInfoTable[i].PESChannel = FALSE;
                            trace_new(DPS_FUNC_TRACE,
                                "DEMUX:Allocated non-desc PSI channel %d\n", i);
                        }
                        else
                        {
                            /*
                             * Set an invalid value in the continuity counter so
                             * that the first packet is received regardless of the
                             * count value.
                             */
                            dmx->ChannelInfoTable[i].CCounter = (u_int32)-1;

                            dmx->ChannelInfoTable[i].PESChannel = TRUE;
                            trace_new(DPS_FUNC_TRACE,
                                "DEMUX:Allocated non-desc PES channel %d\n", i);
                        }
                        dmx->NonDescChannelsAvail--;
                        *chid = i;
                        return DMX_STATUS_OK;
                    }
                }
            }
    
            /*
             * Either we are looking for descrambling channel or there are no non-descrambling
             * channels left so we might as well use one with descrambling.
             */
        
            /*
             * Look for a descrambler capable slot
             */
            num_aud_chans = ( ( *( DPS_CAPABILITIES_EX ( dmxid ) ) & 
                                DPS_CAPABILITIES_NUM_AUD_CHAN_MASK ) >>
                                DPS_CAPABILITIES_NUM_AUD_CHAN_SHIFT ) + 1;
            for (i=0; i<DPS_NUM_DESCRAMBLERS; ++i)
            {
                /*
                 * Chid's 0 - DPS_NUM_DESCRAMBLERS support descrambling
                 * Chid's 1, 2, and possibly 3 & 4 are reserved special-purpose A/V PES channels
                 */
                if ( ( i < VIDEO_CHANNEL ) || ( i > VIDEO_CHANNEL + num_aud_chans ) )
                {
                    if (dmx->ChannelInfoTable[i].InUseFlag == CHANNEL_FREE)
                    {
                        dmx->ChannelInfoTable[i].InUseFlag = CHANNEL_IN_USE;
                        dmx->ChannelInfoTable[i].stype = channel_type;
                        dmx->ChannelInfoTable[i].TagSet = FALSE;
                        if(channel_type == PSI_CHANNEL_TYPE)
                        {
                            dmx->ChannelInfoTable[i].PESChannel = FALSE;
                            trace_new(DPS_FUNC_TRACE,
                                "DEMUX:Allocated desc PSI channel %d\n", i);
                        }
                        else
                        {
                            /*
                             * Set an invalid value in the continuity counter so
                             * that the first packet is received regardless of the
                             * count value.
                             */
                            dmx->ChannelInfoTable[i].CCounter = (u_int32)-1;

                            dmx->ChannelInfoTable[i].PESChannel = TRUE;
                            trace_new(DPS_FUNC_TRACE,
                                "DEMUX:Allocated desc PES channel %d\n", i);
                        }
                        dmx->DescChannelsAvail--;
                        *chid = i;
                        return DMX_STATUS_OK;
                    }
                }
            }

            /*
             * No channels available
             */
            trace_new(DPS_ERROR_MSG,"DEMUX:cnxt_dmx_channel_open failed - no empty slots\n");
            return DMX_CH_UNAVAILABLE;

        default:
            trace_new(DPS_ERROR_MSG,"DEMUX:cnxt_dmx_channel_open failed - invalid channel type\n");
            return DMX_CH_UNAVAILABLE;
    }

    return DMX_STATUS_OK;
}

/*****************************************************************************
 * cnxt_dmx_channel_close
 *
 * parameters:
 *    dmxid - demux hardware instance handle that was returned by _open()
 *    chid - the channel instance that was returned by _channel_open()
 *
 * returns:
 *    DMX_STATUS_OK - no errors
 *    DMX_BAD_DMXID - bad demux instance handle
 *    DMX_BAD_CHID - bad channel instance handle
 *
 * description:
 *    closes a previously opened channel.  The channel will be turned off
 *    and any allocated resources will be freed.
 *****************************************************************************/
DMX_STATUS cnxt_dmx_channel_close(u_int32 dmxid, u_int32 chid ) {
    DemuxInfo *dmx;
    u_int32 slot;

    trace_new(DPS_FUNC_TRACE,"DEMUX:cnxt_dmx_channel_close entered\n");

    if(dmxid >= MAX_DEMUX_UNITS || dmxid == 1) { /* Sanity check dmxid value, avoid demux 1 & going
                                                    outside array bounds in next check */
      trace_new(DPS_ERROR_MSG, "DEMUX:cnxt_dmx_channel_close: demux %d does not exist!\n",dmxid);
      return DMX_BAD_DMXID;
    }

    /* First check to see that this demux id is already allocated */
    if (!gDemuxInfo[dmxid].DemuxInitialized) {
        trace_new(DPS_ERROR_MSG,"DEMUX:cnxt_dmx_channel_close: demux %d does not exist!\n",dmxid);
        return DMX_BAD_DMXID;
    }

    dmx = &gDemuxInfo[dmxid];

    if (chid >= TOTAL_CHANNELS ||
        dmx->ChannelInfoTable[chid].DemuxEnable || 
        dmx->ChannelInfoTable[chid].FiltersAllocated) {
        return DMX_BAD_CHID;
    }

    if(dmx->ChannelInfoTable[chid].InUseFlag == CHANNEL_FREE) {
      trace_new(DPS_ERROR_MSG,"DEMUX:cnxt_dmx_channel_close: channel %d is already closed!\n", chid);
      return DMX_BAD_CHID;
    }
    
    slot = dmx->ChannelInfoTable[chid].Slot;    /* Get slot for this PID */

    /*
     * PID must be freed since DemuxEnable is checked above.  Must wait a bit
     * here before messing with the buffer pointers...
     */

    /*
     * Remove the buffer ptrs
     */
    if ( task_id () == PSIProcessID )
    {
       dmx->ChannelInfoTable[chid].CallbackFlushedChannel = TRUE;
    }

    *DPS_PID_SLOT_WRITE_PTR_EX(dmxid,slot) = 0x0;
    *DPS_PID_SLOT_READ_PTR_EX(dmxid,slot) = 0x0;
    *DPS_PID_SLOT_START_ADDR_EX(dmxid,slot) = 0x0;
    *DPS_PID_SLOT_END_ADDR_EX(dmxid,slot) = 0x0;

    if (dmx->ChannelInfoTable[chid].HdrArea) {
        mem_free(dmx->ChannelInfoTable[chid].HdrAlloc);
        dmx->ChannelInfoTable[chid].HdrSize  = 0;
        dmx->ChannelInfoTable[chid].HdrArea  = NULL;
        dmx->ChannelInfoTable[chid].HdrAlloc = NULL;
    }

    /*
     * If there was a tick timer active, stop it
     */

    if(dmx->ChannelInfoTable[chid].ChannelTimerActive)
    {
        if(RC_OK != tick_stop(dmx->ChannelInfoTable[chid].ChannelTimer))
        {
            trace_new(DPS_ERROR_MSG,"DEMUX:tick_stop failed.\n");
        }
    }

    /*
     * If there was a tick timer allocated, deallocate it
     */
    
    if(dmx->ChannelInfoTable[chid].ChannelTimer)
    {
        if(RC_OK != tick_destroy(dmx->ChannelInfoTable[chid].ChannelTimer))
        {
            trace_new(DPS_ERROR_MSG,"DEMUX:tick_destroy failed.\n");
        }
    }

    /*
     * Reset all channel structure information
     */

    dmx->ChannelInfoTable[chid].PID = (u_int16) GENDMX_BAD_CHANNEL;
    dmx->ChannelInfoTable[chid].Slot = chid;
    if (chid == VIDEO_CHANNEL)
        dmx->ChannelInfoTable[chid].stype = VIDEO_PES_TYPE;
    else if (chid == AUDIO_CHANNEL)
        dmx->ChannelInfoTable[chid].stype = AUDIO_PES_TYPE;
    else
        dmx->ChannelInfoTable[chid].stype = PSI_CHANNEL_TYPE;
    dmx->ChannelInfoTable[chid].NotifyData.pData = NULL;
    dmx->ChannelInfoTable[chid].NotifyData.condition= 0;
    dmx->ChannelInfoTable[chid].NotifyData.chid = chid;
    dmx->ChannelInfoTable[chid].NotifyData.write_ptr = NULL;
    dmx->ChannelInfoTable[chid].NotifyData.skip = 0;
    dmx->ChannelInfoTable[chid].NotifyData.length = 0;
    dmx->ChannelInfoTable[chid].NotifyData.tag = 0;
    dmx->ChannelInfoTable[chid].FilterEnable = 0x00000000;
    dmx->ChannelInfoTable[chid].FiltersAllocated = 0x00000000;
    dmx->ChannelInfoTable[chid].DisabledByFilter = FALSE;
    dmx->ChannelInfoTable[chid].InUseFlag = CHANNEL_FREE;
    dmx->ChannelInfoTable[chid].DemuxEnable = GEN_DEMUX_DISABLE;
    dmx->ChannelInfoTable[chid].Overflows = 0;
    dmx->ChannelInfoTable[chid].OverflowsHandled = 0;
    dmx->ChannelInfoTable[chid].pBuffer = 0;
    dmx->ChannelInfoTable[chid].pBufferEnd = 0;
    dmx->ChannelInfoTable[chid].pWritePtr = 0;
    dmx->ChannelInfoTable[chid].pReadPtr = 0;
    dmx->ChannelInfoTable[chid].pAckPtr = 0;
    dmx->ChannelInfoTable[chid].HdrErrNotify = NULL;
    dmx->ChannelInfoTable[chid].DataNotify = NULL;
    dmx->ChannelInfoTable[chid].ChannelTimer = 0;
    dmx->ChannelInfoTable[chid].ChannelTimerActive = FALSE;
    dmx->ChannelInfoTable[chid].NotifyCount = 0;
    dmx->ChannelInfoTable[chid].PESChannel = FALSE;
    dmx->ChannelInfoTable[chid].PESDataCount = 0;
    dmx->ChannelInfoTable[chid].CurrentFilterId = GENDMX_BAD_FILTER;
    dmx->ChannelInfoTable[chid].LoopMode = GENDEMUX_CONTINUOUS;
    dmx->ChannelInfoTable[chid].TimeOutMS = 0;      /* no time out */
    dmx->ChannelInfoTable[chid].TimerNotifyCount = 0;
    dmx->ChannelInfoTable[chid].Notify_Unlock = 0;
    dmx->ChannelInfoTable[chid].NotifyCalled  = 0;
    dmx->ChannelInfoTable[chid].bRecording = FALSE;
    
    if(chid < DPS_NUM_DESCRAMBLERS) {
      dmx->DescChannelsAvail++;
    } else {
      dmx->NonDescChannelsAvail++;
    }

    return DMX_STATUS_OK;
}

/*****************************************************************************
 * cnxt_dmx_channels_available
 *
 * parameters:
 *    dmxid - demux hardware instance handle that was returned by _open()
 *    capabilities - a bitmask specifying the required capabilities for the
 *          channels.  Multiple capabilities can be OR'd together.
 *          See below for capability list.
 *    cnt - caller supplied storage for count of channels with specified
 *          capabilities
 *    size_of_filter - Size of filter required for this channel
 *
 * returns:
 *    number of channels through the cnt parameter
 *    DMX_STATUS_OK - no errors
 *    DMX_BAD_ID - the specified demux instance handle is not valid
 *
 * description:
 *    Determines and returns to the caller the number of channels that
 *    are available that meet the required capabilities.
 ****************************************************************************/
DMX_STATUS cnxt_dmx_channels_available(u_int32 dmxid, u_int32 capabilities, u_int32* cnt,
                                       u_int32 size_of_filter ) {
    u_int32 NumChannels = 0;
    DemuxInfo *dmx;

    trace_new(DPS_FUNC_TRACE,"DEMUX:cnxt_dmx_channels_available entered\n");

    if(dmxid >= MAX_DEMUX_UNITS || dmxid == 1) { /* Sanity check dmxid value, avoid demux 1 & going
                                                    outside array bounds in next check */
      trace_new(DPS_ERROR_MSG, "DEMUX:cnxt_dmx_channels_available: demux %d does not exist!\n",dmxid);
      return DMX_BAD_DMXID;
    }

    /* First check to see that this demux id is already allocated */
    if (!gDemuxInfo[dmxid].DemuxInitialized) {
        trace_new(DPS_ERROR_MSG,"DEMUX:cnxt_dmx_channels_available: demux %d does not exist!\n",dmxid);
        return DMX_BAD_DMXID;
    }

    dmx = &gDemuxInfo[dmxid];

    if (dmx->HWFiltering) {
        if (size_of_filter <= GENDMX_MAX_HW_FILTER_SIZE) {
            if (capabilities & DMX_CH_CAP_DESCRAMBLING) {
                NumChannels = dmx->DescChannelsAvail;
            } else
                NumChannels = dmx->NonDescChannelsAvail+dmx->DescChannelsAvail;
        }
    } else {
        if (size_of_filter <= GENDMX_MAX_HW_FILTER_SIZE) {
            if (capabilities & DMX_CH_CAP_DESCRAMBLING) {
                NumChannels = dmx->DescChannelsAvail;
            } else
                NumChannels = dmx->NonDescChannelsAvail+dmx->DescChannelsAvail;
        }
    }
    *cnt = NumChannels;
    return DMX_STATUS_OK;
}

/*****************************************************************************
 * cnxt_dmx_channel_set_pid
 *
 * parameters:
 *    dmxid - demux hardware instance handle that was returned by _open()
 *    chid - the channel instance that was returned by _channel_open()
 *    pid - the PID to be assigned to this channel
 *
 * returns:
 *    DMX_STATUS_OK - no errors
 *    DMX_BAD_DMXID - bad demux instance handle
 *    DMX_BAD_CHID - bad channel instance handle
 *    DMX_BAD_PID - specified PID is not valid
 *    DMX_CH_CONTROL_ERROR - call to channel_control failed
 *
 * description:
 *    Assigns a PID value to this channel.  Note that this does not have
 *    any effect on hardware until _control_channel() is used to enable
 *    the channel.
 ****************************************************************************/
DMX_STATUS cnxt_dmx_channel_set_pid(u_int32 dmxid, u_int32 chid, u_int16 pid)  {
    DemuxInfo *dmx;
    int i;

    trace_new(DPS_FUNC_TRACE,"DEMUX:cnxt_dmx_channel_set_pid  entered\n");

    if(dmxid >= MAX_DEMUX_UNITS || dmxid == 1) { /* Sanity check dmxid value, avoid demux 1 & going
                                                    outside array bounds in next check */
      trace_new(DPS_ERROR_MSG, "DEMUX:cnxt_dmx_channel_set_pid: demux %d does not exist!\n",dmxid);
      return DMX_BAD_DMXID;
    }

    /* First check to see that this demux id is already allocated */
    if (!gDemuxInfo[dmxid].DemuxInitialized) {
        trace_new(DPS_ERROR_MSG,"DEMUX:cnxt_dmx_channel_set_pid: demux %d does not exist!\n",dmxid);
        return DMX_BAD_DMXID;
    }

    dmx = &gDemuxInfo[dmxid];

    if (chid >= TOTAL_CHANNELS)
        return DMX_BAD_CHID;

    /* Perform error checking on non video/audio.
       Any client can set the video/audio */
    if (dmx->ChannelInfoTable[chid].stype != AUDIO_PES_TYPE && 
        dmx->ChannelInfoTable[chid].stype != VIDEO_PES_TYPE) {
        if (dmx->ChannelInfoTable[chid].InUseFlag == CHANNEL_FREE) {
            return DMX_BAD_CHID;
        }
    }

    /*
     * Ensure PID is not already in table and enabled
     */

    for (i=0;i<TOTAL_CHANNELS;i++) {
        if (i==chid)
            continue;
        /*
         * If we found a PID, see if that channel is enabled.
         */
        if (dmx->ChannelInfoTable[i].PID == (pid & 0x1fff)) {
            /*
             * If we found a PID, see if that channel is opened.
             */
            if (dmx->ChannelInfoTable[i].InUseFlag == CHANNEL_IN_USE) {
                /*
                 * If we ever enable virtual channels with multiple channels with the
                 * same PID, we'll need the ability for multiple channels to have the
                 * same PID.  For now, it't not needed...
                 */

                 trace_new(DPS_ERROR_MSG,
                       "cnxt_dmx_channel_set_pid(PID already in table! pid=0x%04x, chid=%d, slot=%d)\n",
                       (pid & 0x1fff), chid, i);
                 return DMX_BAD_PID;
            } else {
                /*
                 * This is the case where a leftover PID is in the table in a different
                 * slot but is disabled.  In this case, we're ok.  But, in order to avoid
                 * having a demod connect/disconnect turn on the PID in the other slot,
                 * we'll force it off here.
                 */
                dmx->ChannelInfoTable[i].PID = GEN_NULL_PID;
            }
        }
    }

    /*
     * Save PID
     */

    dmx->ChannelInfoTable[chid].PID = pid & 0x1FFF;

    /*
     * Set destination
     */
    if(!(dmx->CapabilitiesRequested & DMX_CAP_RECORD))
    {
        if (dmx->ChannelInfoTable[chid].PESChannel) 
        {
          if(dmx->ChannelInfoTable[chid].stype != VIDEO_PES_TYPE &&
             dmx->ChannelInfoTable[chid].stype != AUDIO_PES_TYPE) 
          {
            *DPS_TRANSPORT_PID_REG_EX(dmxid) |=  (1<<chid);
            *DPS_PES_PID_REG_EX(dmxid) |= (1<<chid);
          }
        } else {
          /*
           * If PSI channel, force data into the per-slot PSI buffer.
           */
      
          *DPS_TRANSPORT_PID_REG_EX(dmxid) &= ~(1<<chid);
          *DPS_PES_PID_REG_EX(dmxid) &= ~(1<<chid);
        }
    }

    if (dmx->ChannelInfoTable[chid].DemuxEnable == GEN_DEMUX_ENABLE) {
        /*
         * If channel is enabled, call control channel to set the PID which
         * calls gen_dmx_hw_set_pid()
         */

        if (cnxt_dmx_channel_control(dmxid, chid, (gencontrol_channel_t) GEN_DEMUX_ENABLE) 
            != DMX_STATUS_OK) {
            trace_new(DPS_ERROR_MSG,"DEMUX:cnxt_dmx_channel_set_pid ->control_channel failed\n");
            return DMX_CH_CONTROL_ERROR;
        }
    }

    if (dmx->ChannelInfoTable[chid].bRecording == TRUE) {
        /*
         * If channel is recording, call control channel to set the PID which
         * calls gen_dmx_hw_set_pid()
         */

        if (cnxt_dmx_channel_control(dmxid, chid, (gencontrol_channel_t) GEN_DEMUX_ENABLE_RECORD) 
            != DMX_STATUS_OK) {
            trace_new(DPS_ERROR_MSG,"DEMUX:cnxt_dmx_channel_set_pid ->control_channel failed\n");
            return DMX_CH_CONTROL_ERROR;
        }
    }

    return DMX_STATUS_OK;
}

/*****************************************************************************
 * cnxt_dmx_channel_get_pid
 *
 * parameters:
 *    dmxid - demux hardware instance handle that was returned by _open()
 *    chid - the channel instance that was returned by _channel_open()
 *    pid - caller supplied storage for the pid
 *
 * returns:
 *    DMX_STATUS_OK - no errors
 *    DMX_BAD_DMXID - bad demux instance handle
 *    DMX_BAD_CHID - bad channel instance handle
 *    DMX_BAD_PTR - problem with pid pointer
 *
 * description:
 *    Returns the pid value associated with the channel.  If the channel
 *    does not have a pid assigned, then the value GEN_INVALID_PID is
 *    returned through the pid pointer.
 ****************************************************************************/
DMX_STATUS cnxt_dmx_channel_get_pid(u_int32 dmxid, u_int32 chid, 
                                    u_int16* pid ){
    DemuxInfo *dmx;

    trace_new(DPS_FUNC_TRACE,"DEMUX:cnxt_dmx_channel_get_pid  entered\n");

    if(dmxid >= MAX_DEMUX_UNITS || dmxid == 1) { /* Sanity check dmxid value, avoid demux 1 & going
                                                    outside array bounds in next check */
      trace_new(DPS_ERROR_MSG, "DEMUX:cnxt_dmx_channel_get_pid: demux %d does not exist!\n",dmxid);
      return DMX_BAD_DMXID;
    }

    /* First check to see that this demux id is already allocated */
    if (!gDemuxInfo[dmxid].DemuxInitialized) {
        trace_new(DPS_ERROR_MSG,"DEMUX:cnxt_dmx_channel_get_pid: demux %d does not exist!\n",dmxid);
        return DMX_BAD_DMXID;
    }

    dmx = &gDemuxInfo[dmxid];

    *pid = dmx->ChannelInfoTable[chid].PID & GEN_NULL_PID;
    return DMX_STATUS_OK;
}

/*****************************************************************************
 * cnxt_dmx_channel_control
 *
 * parameters:
 *    dmxid - demux hardware instance handle that was returned by _open()
 *    chid - the channel instance that was returned by _channel_open()
 *    channel_command - channel control code
 *          GEN_DEMUX_ENABLE
 *          GEN_DEMUX_DISABLE
 *          GEN_DEMUX_RESET
 *          GEN_DEMUX_DISABLE_RESET
 *
 * returns:
 *    DMX_STATUS_OK - no errors
 *    DMX_BAD_DMXID - bad demux instance handle
 *    DMX_BAD_CHID - bad channel instance handle
 *    DMX_BAD_CTL - bad control code
 *    DMX_CH_CONTROL_ERROR - channel could not be enabled (hardware resource?)
 *
 * description:
 *    will enable or disable the specified channel.  When enabled, the
 *    pid for the channel will be loaded into the demux hardware pid
 *    table and any filters associated with the channel will also be
 *    loaded.  When disabled, the pid is removed from the hardware pid
 *    table and the filters are cleared.  Enabling/disabling a channel
 *    that is already enabled/disabled will have no effect.  In order
 *    for a change to take effect on a channel, such as pid or filters,
 *    the channel must be disabled, then enabled.
 *
 *    WARNING: This function may be called during an ISR from either the audio
 *             or video drivers.  Therefore, all future changes to this function
 *             need to be interrupt safe.
 *
 ****************************************************************************/
DMX_STATUS cnxt_dmx_channel_control(u_int32 dmxid, u_int32 chid, 
                                    gencontrol_channel_t channel_command ) {
    DemuxInfo *dmx;
    sem_id_t sem;
    task_id_t tidCaller;
    bool bRunningPSICallback = FALSE;
    bool bRunningPESCallback = FALSE;
    bool ks;

    if(dmxid >= MAX_DEMUX_UNITS || dmxid == 1) { /* Sanity check dmxid value, avoid demux 1 & going
                                                    outside array bounds in next check */
      isr_trace_new(DPS_ERROR_MSG, "DEMUX:cnxt_dmx_channel_control: demux %d does not exist!\n",dmxid, 0);
      return DMX_BAD_DMXID;
    }

    /* First check to see that this demux id is already allocated */
    if (!gDemuxInfo[dmxid].DemuxInitialized) {
        isr_trace_new(DPS_ERROR_MSG,"DEMUX:cnxt_dmx_channel_control: demux %d does not exist!\n",dmxid,0);
        return DMX_BAD_DMXID;
    }

    dmx = &gDemuxInfo[dmxid];

    if (chid >= TOTAL_CHANNELS)
        return DMX_BAD_CHID;



    isr_trace_new( DPS_FUNC_TRACE,"DEMUX:cnxt_dmx_channel_control entered: Channel %d PID=0x%x\n", 
                  chid, dmx->ChannelInfoTable[chid].PID);
    isr_trace_new( DPS_FUNC_TRACE,"DEMUX:   Enable/Disable=%d\n", channel_command,0);

    if (dmx->ChannelInfoTable[chid].stype != VIDEO_PES_TYPE && 
        dmx->ChannelInfoTable[chid].stype != AUDIO_PES_TYPE) {
        if (dmx->ChannelInfoTable[chid].InUseFlag != CHANNEL_IN_USE) {
            isr_trace_new(DPS_ERROR_MSG,"DEMUX:cnxt_dmx_channel_control failed\n",0,0);
            return DMX_CH_CONTROL_ERROR;
        }
    }

    /* Check to see if this call is being made in the context of the SectionNotify or */
    /* HeaderNotify callbacks.                                                        */
    tidCaller = task_id();
    if(tidCaller == PSIProcessID)
      bRunningPSICallback = TRUE;
    if(tidCaller == PES_TASK_ID)
      bRunningPESCallback = TRUE;

    dmx->ChannelInfoTable[chid].DisabledByFilter = FALSE;

    switch (channel_command) {

    case GEN_DEMUX_DISABLE:

        /*
         * Disable channel
         */

        isr_trace_new( DPS_FUNC_TRACE,"cnxt_dmx_channel_control(DISABLE) chid=%d\n", chid,0);

        dmx->ChannelInfoTable[chid].DemuxEnable = GEN_DEMUX_DISABLE;

#if PVR==YES
        if(dmx->CapabilitiesRequested & DMX_CAP_RECORD)
        {
           /*
            * If we're on REC ucode, make sure they're not trying to play or monitor
            * a PES channel.
            */
            if (dmx->ChannelInfoTable[chid].stype != PSI_CHANNEL_TYPE)
            {
                isr_trace_new( DPS_ERROR_MSG,"Can't play/monitor PES or ES channel w/ \
                    record ucode!\n", 0,0);
                return DMX_CH_CONTROL_ERROR;
            }
            
            /*
             * If we're on REC ucode, we want to clear the monitor bit for this 
             * channel.
             */
            *DPS_REC_MON_PID_TABLE_EX(dmxid) &= ~(1<<chid);
        }
#endif /* PVR == YES */
        /* 
         * If we're not also recording, we have to do a full channel teardown.
         */
        if (!dmx->ChannelInfoTable[chid].bRecording)
        {
            /*
             * If there's a timer, ensure it's stopped
             */
            if (dmx->ChannelInfoTable[chid].ChannelTimerActive)
            {
                if(RC_OK != tick_stop(dmx->ChannelInfoTable[chid].ChannelTimer))
                {
                    isr_trace_new(DPS_ERROR_MSG,"DEMUX:tick_stop failed.\n",0,0);
                }
                dmx->ChannelInfoTable[chid].ChannelTimerActive = FALSE;
                dmx->ChannelInfoTable[chid].TimerNotifyCount = 0;
            }
            gen_dmx_hw_free_pid(dmxid, chid);
        }
        break;

    case GEN_DEMUX_ENABLE:

        isr_trace_new( DPS_FUNC_TRACE,"cnxt_dmx_channel_control(ENABLE) chid=%d\n", chid,0);

#if PVR==YES
        if(dmx->CapabilitiesRequested & DMX_CAP_RECORD)
        {
           /*
            * If we're on REC ucode, make sure they're not trying to play or monitor
            * a PES channel.
            */
            if (dmx->ChannelInfoTable[chid].stype != PSI_CHANNEL_TYPE)
            {
                isr_trace_new( DPS_ERROR_MSG,"Can't play/monitor PES or ES channel w/ \
                    record ucode!\n", 0,0);
                return DMX_CH_CONTROL_ERROR;
            }
            
            /*
             * If we're on REC ucode, we want to set the monitor bit for this 
             * channel.
             */
            *DPS_REC_MON_PID_TABLE_EX(dmxid) |= (1<<chid);
        }
#endif /* PVR == YES */
        /*
         * Ensure there is a buffer set
         */
        if (dmx->ChannelInfoTable[chid].stype != VIDEO_PES_TYPE && 
            dmx->ChannelInfoTable[chid].stype != AUDIO_PES_TYPE) {
            if ((u_int32)(*DPS_PID_SLOT_START_ADDR_EX(dmxid, chid) & ~DPS_PAW_SYS_ADDRESS) != 
                ((u_int32)dmx->ChannelInfoTable[chid].pBuffer & ~NCR_BASE)) {
                trace_new(DPS_ERROR_MSG, "cnxt_dmx_channel_control(chid=%d pid=%04x)  !!NO BUFFER!!\n", chid, dmx->ChannelInfoTable[chid].PID);

                /*
                 * Assign the pid slot read, write, start and end ptrs/addresses with default values
                 */
                *DPS_PID_SLOT_WRITE_PTR_EX(dmxid,chid) =
                  (((u_int32)dmx->ChannelInfoTable[chid].pBuffer&~NCR_BASE)|DPS_PAW_SYS_ADDRESS);
                *DPS_PID_SLOT_READ_PTR_EX(dmxid,chid) =
                  (((u_int32)dmx->ChannelInfoTable[chid].pBuffer&~NCR_BASE)|DPS_PAW_SYS_ADDRESS);
                *DPS_PID_SLOT_START_ADDR_EX(dmxid,chid) = 
                  (((u_int32)dmx->ChannelInfoTable[chid].pBuffer&~NCR_BASE)|DPS_PAW_SYS_ADDRESS);
                *DPS_PID_SLOT_END_ADDR_EX(dmxid,chid) =
                  (((u_int32)dmx->ChannelInfoTable[chid].pBufferEnd&~NCR_BASE)|DPS_PAW_SYS_ADDRESS);
            }
        }

        /* 
         * If we're not already recording, we have to do a full channel setup.
         */
        if (!dmx->ChannelInfoTable[chid].bRecording)
        {
           gen_dmx_hw_set_pid(dmxid, chid, dmx->ChannelInfoTable[chid].PID);

           /*
            * If there's a timeout, start the timer
            */
           if ((dmx->ChannelInfoTable[chid].TimeOutMS) &&
               (dmx->ChannelInfoTable[chid].ChannelTimer))
           {
               if (RC_OK != tick_start(dmx->ChannelInfoTable[chid].ChannelTimer))
               {
                   isr_trace_new(DPS_ERROR_MSG,"DEMUX:tick_start failed.\n",0,0);
               }
               dmx->ChannelInfoTable[chid].ChannelTimerActive = TRUE;
           }
           else
           {
               /*
                * If there's no timeout, but there's a timer, ensure it's stopped
                */
               if (dmx->ChannelInfoTable[chid].ChannelTimerActive)
               {
                   if (RC_OK != tick_stop(dmx->ChannelInfoTable[chid].ChannelTimer))
                   {
                       isr_trace_new(DPS_ERROR_MSG,"DEMUX:tick_stop failed.\n",0,0);
                   }
                   dmx->ChannelInfoTable[chid].ChannelTimerActive = FALSE;
                   dmx->ChannelInfoTable[chid].TimerNotifyCount = 0;
               }
           }
        }

        /*
         * Enable channel
         */
        dmx->ChannelInfoTable[chid].DemuxEnable = GEN_DEMUX_ENABLE;
        break;

#if PVR==YES
    case GEN_DEMUX_DISABLE_RECORD:

        /*
         * Disable channel recording
         */

        isr_trace_new( DPS_FUNC_TRACE,"cnxt_dmx_channel_control(DISABLE_RECORD) chid=%d\n", chid,0);

        /*
         * If we're not on REC ucode, return an error.
         */
        if(!(dmx->CapabilitiesRequested & DMX_CAP_RECORD))
        {
            isr_trace_new(DPS_ERROR_MSG, "Must have REC ucode to record!\n", 0,0);
            return DMX_CH_CONTROL_ERROR;
        }

        /*
         * Mark channel as not recording.
         */
        dmx->ChannelInfoTable[chid].bRecording = FALSE;

        /*
         * Clear transport PID table bit.
         * NOTE: These bits are ignored for A/V slots 1-4. These slots can only
         * be enabled/disable by setting/clearing the PID.
         */
        *DPS_TRANSPORT_PID_REG_EX(dmxid) &= ~(1<<chid);
        
        /* 
         * If it's a non-PSI channel or we're not also monitoring, we have to 
         * do a full channel teardown.
         */
        if ((dmx->ChannelInfoTable[chid].stype != PSI_CHANNEL_TYPE) ||
            (dmx->ChannelInfoTable[chid].DemuxEnable == GEN_DEMUX_DISABLE))
        {
            gen_dmx_hw_free_pid(dmxid, chid);
        }
        break;

    case GEN_DEMUX_ENABLE_RECORD:

        /*
         * Enable channel recording
         */
         
        isr_trace_new( DPS_FUNC_TRACE,"cnxt_dmx_channel_control(ENABLE_RECORD) chid=%d\n", chid,0);

        /*
         * If we're not on REC ucode, return an error.
         */
        if(!(dmx->CapabilitiesRequested & DMX_CAP_RECORD))
        {
            isr_trace_new(DPS_ERROR_MSG, "Must have REC ucode to record!\n", 0,0);
            return DMX_CH_CONTROL_ERROR;
        }

        /*
         * Ensure there is a buffer set
         */
        if (dmx->ChannelInfoTable[chid].stype != VIDEO_PES_TYPE && 
            dmx->ChannelInfoTable[chid].stype != AUDIO_PES_TYPE) 
        {
            if ((u_int32)(*DPS_TRANSPORT_START_ADDR_EX(dmxid) & ~DPS_PAW_SYS_ADDRESS) != 
                ((u_int32)dmx->rec_buffer_start_addr & ~NCR_BASE))
            {
                trace_new(DPS_ERROR_MSG, "cnxt_dmx_channel_control(chid=%d pid=%04x)  !!NO BUFFER!!\n", chid, dmx->ChannelInfoTable[chid].PID);
                return DMX_CH_CONTROL_ERROR;
            }
        }

        /* 
         * If it's a non-PSI channel or we're not already monitoring, we have to 
         * do a full channel setup.
         */
        if ((dmx->ChannelInfoTable[chid].stype != PSI_CHANNEL_TYPE) ||
            (dmx->ChannelInfoTable[chid].DemuxEnable == GEN_DEMUX_DISABLE))
        {
           gen_dmx_hw_set_pid(dmxid, chid, dmx->ChannelInfoTable[chid].PID);
        }

        /*
         * Set transport PID table bit.
         * NOTE: These bits are ignored for A/V slots 1-4. These slots can only
         * be enabled/disable by setting/clearing the PID.
         */
        *DPS_TRANSPORT_PID_REG_EX(dmxid) |= (1<<chid);
        
        /*
         * Mark channel as recording.
         */
        dmx->ChannelInfoTable[chid].bRecording = TRUE;
        break;
#endif /* PVR == YES */

    case GEN_DEMUX_RESET:

        /*
         * Disable channel
         */

        dmx->ChannelInfoTable[chid].DemuxEnable = GEN_DEMUX_DISABLE;

        isr_trace_new( DPS_FUNC_TRACE,"cnxt_dmx_channel_control(RESET) chid=%d\n", chid,0);
        /*********************************************
        Disable Channel & RESET it then ReEnable it
        **********************************************/

        /*
         * Turn off the PID
         */

        gen_dmx_hw_free_pid(dmxid, chid);

        /*
         * Delay for one transport packet here (less than 1mS but this is the shortest we can sleep so ...)
         */
         
        task_time_sleep(1);

        /*
         * Stop the processing tasks momentarily assuming we are not on the context of the PSI task
         */

          if (dmx->ChannelInfoTable[chid].PESChannel) {
              if(!bRunningPESCallback)
                sem = GenDmxPESTaskProcSem;
              else
                sem = (sem_id_t)0;  
          } else {
              /* If we are already running in the context of the PSI task, DON'T GRAB THE SEMAPHORE!!!!! */
              if(!bRunningPSICallback)
                sem = GenDmxPSITaskProcSem;
              else
                sem = (sem_id_t)0;  
          }
          
          if(sem)
          {
#ifdef IPANEL_SEM_LOCKED_TEST1 
	    if( flag_pop_sec_to_callback )  // GenDmxPSITaskProcSem is already locked!
	    {
	    	sem_put(sem) ; 
	    }
#endif

            sem_get(sem, KAL_WAIT_FOREVER);
          }

        /*
         * Reset buffer pointers
         */
        switch(dmx->ChannelInfoTable[chid].stype)
        {
            /* Video channel */
            case VIDEO_PES_TYPE:
                ks = critical_section_begin();
                *DPS_VIDEO_WRITE_PTR_EX(dmxid) = *DPS_VIDEO_READ_PTR_EX(dmxid);
                critical_section_end(ks);
                break;
            
            /* Audio channel */
            case AUDIO_PES_TYPE:
                ks = critical_section_begin();
                *DPS_AUDIO_WRITE_PTR_EX(dmxid) = *DPS_AUDIO_READ_PTR_EX(dmxid);
                critical_section_end(ks);
                break;
            
            /* SI/other channel */
            default:

                dmx->ChannelInfoTable[chid].pWritePtr     =
                dmx->ChannelInfoTable[chid].pReadPtr      =
                dmx->ChannelInfoTable[chid].pAckPtr       = dmx->ChannelInfoTable[chid].pBuffer;
        
                if(bRunningPSICallback)
                  dmx->ChannelInfoTable[chid].CallbackFlushedChannel = TRUE;

                /*
                 * Assign the pid slot read, write, start and end ptrs/addresses with default values
                 */
                *DPS_PID_SLOT_WRITE_PTR_EX(dmxid,chid) =
                (((u_int32)dmx->ChannelInfoTable[chid].pBuffer&~NCR_BASE)|DPS_PAW_SYS_ADDRESS);
                *DPS_PID_SLOT_READ_PTR_EX(dmxid,chid) =
                (((u_int32)dmx->ChannelInfoTable[chid].pBuffer&~NCR_BASE)|DPS_PAW_SYS_ADDRESS);
                *DPS_PID_SLOT_START_ADDR_EX(dmxid,chid) = 
                (((u_int32)dmx->ChannelInfoTable[chid].pBuffer&~NCR_BASE)|DPS_PAW_SYS_ADDRESS);
                *DPS_PID_SLOT_END_ADDR_EX(dmxid,chid) = 
                (((u_int32)dmx->ChannelInfoTable[chid].pBufferEnd&~NCR_BASE)|DPS_PAW_SYS_ADDRESS);
                
                break;
        }
        
        /*
         * Ok for processing task to continue
         */

        if(sem)
          sem_put(sem);

        dmx->ChannelInfoTable[chid].CurrentFilterId = GENDMX_BAD_FILTER;
        gen_dmx_hw_set_pid(dmxid, chid, dmx->ChannelInfoTable[chid].PID);

        #ifdef DEBUG
        if (DEBUGPID == dmx->ChannelInfoTable[chid].PID) {
            trace("PID=0x%x->IDLE, Demux reset called\n", dmx->ChannelInfoTable[chid].PID);
        }
        #endif

        /*
         * Enable channel
         */

        dmx->ChannelInfoTable[chid].DemuxEnable = GEN_DEMUX_ENABLE;
        break;

    case GEN_DEMUX_DISABLE_RESET:

        /*
         * Disable channel
         */

        dmx->ChannelInfoTable[chid].DemuxEnable = GEN_DEMUX_DISABLE;

        isr_trace_new( DPS_FUNC_TRACE,"cnxt_dmx_channel_control(DISABLE_RESET) chid=%d\n", chid,0);
        /*********************************************
        Disable Channel & RESET it then leave disabled
        **********************************************/

        /*
         * If there's a timer, ensure it's stopped
         */
        if (dmx->ChannelInfoTable[chid].ChannelTimerActive)
        {
            if(RC_OK != tick_stop(dmx->ChannelInfoTable[chid].ChannelTimer))
            {
                isr_trace_new(DPS_ERROR_MSG,"DEMUX:tick_stop failed.\n",0,0);
            }
            dmx->ChannelInfoTable[chid].ChannelTimerActive = FALSE;
            dmx->ChannelInfoTable[chid].TimerNotifyCount = 0;
        }

        /*
         * Turn off the PID
         */

        gen_dmx_hw_free_pid(dmxid, chid);

        /*
         * Delay for one transport packet here (less than 1mS but this is the shortest we can sleep so ...)
         */

        task_time_sleep(1);

        /*
         * Stop the processing tasks momentarily assuming we are not on the context of the PSI task
         */

          if (dmx->ChannelInfoTable[chid].PESChannel){
              if(!bRunningPESCallback)
                sem = GenDmxPESTaskProcSem;
              else
                sem = (sem_id_t)0;  
          } else {
              /* If we are already running in the context of the PSI task, DON'T GRAB THE SEMAPHORE!!!!! */
              if(!bRunningPSICallback)
                sem = GenDmxPSITaskProcSem;
              else
                sem = (sem_id_t)0;  
          }
          
          if(sem)
          {
#ifdef IPANEL_SEM_LOCKED_TEST1 
	    if( flag_pop_sec_to_callback )  // GenDmxPSITaskProcSem is already locked!
	    {
	    	sem_put(sem) ; 
	    }
#endif

            sem_get(sem, KAL_WAIT_FOREVER);
          }

        /*
         * Reset buffer pointers
         */

        switch(dmx->ChannelInfoTable[chid].stype)
        {
            /* Video channel */
            case VIDEO_PES_TYPE:
                ks = critical_section_begin();
                *DPS_VIDEO_WRITE_PTR_EX(dmxid) = *DPS_VIDEO_READ_PTR_EX(dmxid);
                critical_section_end(ks);
                break;
            
            /* Audio channel */
            case AUDIO_PES_TYPE:
                ks = critical_section_begin();
                *DPS_AUDIO_WRITE_PTR_EX(dmxid) = *DPS_AUDIO_READ_PTR_EX(dmxid);
                critical_section_end(ks);
                break;
            
            /* SI/other channel */
            default:
                dmx->ChannelInfoTable[chid].pWritePtr     =
                dmx->ChannelInfoTable[chid].pReadPtr      =
                dmx->ChannelInfoTable[chid].pAckPtr       = dmx->ChannelInfoTable[chid].pBuffer;
        
                if(bRunningPSICallback)
                  dmx->ChannelInfoTable[chid].CallbackFlushedChannel = TRUE;

                /*
                 * Assign the pid slot read, write, start and end ptrs/addresses with default values
                 */
                *DPS_PID_SLOT_WRITE_PTR_EX(dmxid,chid) = 
                (((u_int32)dmx->ChannelInfoTable[chid].pBuffer&~NCR_BASE)|DPS_PAW_SYS_ADDRESS);
                *DPS_PID_SLOT_READ_PTR_EX(dmxid,chid) = 
                (((u_int32)dmx->ChannelInfoTable[chid].pBuffer&~NCR_BASE)|DPS_PAW_SYS_ADDRESS);
                *DPS_PID_SLOT_START_ADDR_EX(dmxid,chid) = 
                (((u_int32)dmx->ChannelInfoTable[chid].pBuffer&~NCR_BASE)|DPS_PAW_SYS_ADDRESS);
                *DPS_PID_SLOT_END_ADDR_EX(dmxid,chid) = 
                (((u_int32)dmx->ChannelInfoTable[chid].pBufferEnd&~NCR_BASE)|DPS_PAW_SYS_ADDRESS);
                break;
        }
        
        /*
         * Ok for processing task to continue
         */
  
        if(sem)
          sem_put(sem);

        dmx->ChannelInfoTable[chid].CurrentFilterId = GENDMX_BAD_FILTER;

#ifdef DEBUG
        if (DEBUGPID == dmx->ChannelInfoTable[chid].PID) {
            trace("PID=0x%x->IDLE, Demux disable_reset called\n", dmx->ChannelInfoTable[chid].PID);
        }
#endif

        break;

    default:
        return DMX_BAD_CTL;
        break;
    }

    return DMX_STATUS_OK;
}

/*****************************************************************************
 * cnxt_dmx_filter_open
 *
 * parameters:
 *    dmxid - demux hardware instance handle that was returned by _open()
 *    chid - the channel instance that was returned by _channel_open()
 *    filter_size - total filter size including 2 byte don't care of length
 *    fid - the filter instance that will be returned
 *
 * returns:
 *    DMX_STATUS_OK - no errors
 *    DMX_BAD_DMXID - bad demux instance handle
 *    DMX_BAD_CHID - bad channel instance handle
 *    DMX_BAD_FILTER - bad filter size, etc..
 *
 * description:
 *    Allocates a new section filter instance.  The new filter instance will be returned in
 *    the fid field
 *    
 ****************************************************************************/
DMX_STATUS cnxt_dmx_filter_open(u_int32 dmxid, u_int32 chid, u_int32 filter_size,
                                u_int32* fid) {
    u_int32 i,j;
    DemuxInfo *dmx;

    trace_new(DPS_FUNC_TRACE,"DEMUX:cnxt_dmx_filter_open  entered\n");

    if(dmxid >= MAX_DEMUX_UNITS || dmxid == 1) { /* Sanity check dmxid value, avoid demux 1 & going
                                                    outside array bounds in next check */
      trace_new(DPS_ERROR_MSG, "DEMUX:cnxt_dmx_filter_open: demux %d does not exist!\n",dmxid);
      return DMX_BAD_DMXID;
    }

    /* First check to see that this demux id is already allocated */
    if (!gDemuxInfo[dmxid].DemuxInitialized) {
        trace_new(DPS_ERROR_MSG,"DEMUX:cnxt_dmx_filter_open: demux %d does not exist!\n",dmxid);
        return DMX_BAD_DMXID;
    }

    dmx = &gDemuxInfo[dmxid];

    if (chid >= TOTAL_CHANNELS || dmx->ChannelInfoTable[chid].InUseFlag == CHANNEL_FREE)
        return DMX_BAD_CHID;
    if (dmx->AvailableFilters == 0 || filter_size > (GENDMX_MAX_HW_FILTER_SIZE+2)) {
        trace_new(DPS_ERROR_MSG,"DEMUX:cnxt_dmx_filter_open failed\n");
        return DMX_BAD_FILTER;
    }

    for (i = 0; i < TOTAL_FILTERS; ++i) {                     /* Find an available filter */
        if (dmx->FilterTable[i].OwnerChid == GENDMX_BAD_CHANNEL) { /* if filter available */
            dmx->AvailableFilters--;
            dmx->ChannelInfoTable[chid].FiltersAllocated |= 1<<i;
            dmx->FilterTable[i].OwnerChid = chid;
#if PARSER_TYPE == DVB_PARSER
#if FILTER_ON_LENGTH == NO
            if (filter_size > 3)
                dmx->FilterTable[i].FilterSize = filter_size - 2;
            else
                dmx->FilterTable[i].FilterSize = filter_size;
#else
            /* if filter on length, filter size will include length field */
            dmx->FilterTable[i].FilterSize = filter_size;
#endif
#else
            /* Directv APG filter from client does NOT include length field */
            dmx->FilterTable[i].FilterSize = filter_size;
#endif
            for (j = 0; j < (GENDMX_MAX_HW_FILTER_SIZE/4); ++j) {
                dmx->FilterTable[i].Mask[j]   = 0;
                dmx->FilterTable[i].Match[j]  = 0;
                dmx->FilterTable[i].NotMask[j]= 0;
                *DPS_PATTERN_BASE_EX(dmxid,i,j) = 0;
                *DPS_FILTER_MASK_BASE_EX(dmxid,i,j) = 0;
#if PARSER_FILTERING == FILTER_888
                *DPS_FILTER_MODE_BASE_EX(dmxid,i,j) = 0;
#endif
            }
            dmx->FilterTable[i].NotMaskZero = TRUE;
            dmx->FilterTable[i].NewFilter = TRUE;
            *DPS_NEGATIVE_MODE_REG_EX(dmxid) &= ~(1<<i);
            *fid = i;
            return DMX_STATUS_OK;
        }
    }
    trace_new(DPS_ERROR_MSG,"DEMUX:cnxt_dmx_filter_open failed\n");
    return DMX_BAD_FILTER;
}

/*****************************************************************************
 * cnxt_dmx_filters_available
 *
 * parameters:
 *    dmxid - demux hardware instance handle that was returned by _open()
 *    num_filter - number of harware filters available - returned
 *
 * returns:
 *    DMX_STATUS_OK - no errors 
 *    DMX_BAD_DMXID - bad demux instance handle
 *
 * description:
 *    Returns the number of hardware filters available
 ****************************************************************************/
DMX_STATUS cnxt_dmx_filters_available(u_int32 dmxid, u_int32 *num_filters) {
    DemuxInfo *dmx;


    trace_new(DPS_FUNC_TRACE,"DEMUX:cnxt_dmx_filters_available entered\n");

    if(dmxid >= MAX_DEMUX_UNITS || dmxid == 1) { /* Sanity check dmxid value, avoid demux 1 & going
                                                    outside array bounds in next check */
      trace_new(DPS_ERROR_MSG, "cnxt_dmx_filters_available: demux %d does not exist!!\n", dmxid);
      return DMX_BAD_DMXID;
    }

    /* First check to see that this demux id is already allocated */
    if (!gDemuxInfo[dmxid].DemuxInitialized) {
        trace_new(DPS_ERROR_MSG,"DEMUX:cnxt_dmx_filters_available: demux %d does not exist!\n",dmxid);
        return DMX_BAD_DMXID;
    }

    dmx = &gDemuxInfo[dmxid];

    *num_filters = dmx->AvailableFilters;

    return DMX_STATUS_OK;
}

/*****************************************************************************
 * cnxt_dmx_filter_close
 *
 * parameters:
 *    dmxid - demux hardware instance handle that was returned by _open()
 *    chid - the channel instance that was returned by _channel_open()
 *    fid - the filter instance that was obtained from _filter_open()
 *
 * returns:
 *    DMX_STATUS_OK - no errors
 *    DMX_BAD_DMXID - bad demux instance handle
 *    DMX_BAD_CHID - bad channel instance handle
 *    DMX_BAD_FID - bad filter instance handle
 *
 * description:
 *    Close the given filter instance
 ****************************************************************************/

DMX_STATUS cnxt_dmx_filter_close(u_int32 dmxid, u_int32 chid, u_int32 fid ) {
    DemuxInfo *dmx;
    int i;

    trace_new(DPS_FUNC_TRACE,"DEMUX:cnxt_dmx_filter_close entered\n");

    if(dmxid >= MAX_DEMUX_UNITS || dmxid == 1) { /* Sanity check dmxid value, avoid demux 1 & going
                                                    outside array bounds in next check */
      trace_new(DPS_ERROR_MSG, "DEMUX:cnxt_dmx_filter_close: demux %d does not exist!\n",dmxid);
      return DMX_BAD_DMXID;
    }

    /* First check to see that this demux id is already allocated */
    if (!gDemuxInfo[dmxid].DemuxInitialized) {
        trace_new(DPS_ERROR_MSG,"DEMUX:cnxt_dmx_filter_close: demux %d does not exist!\n",dmxid);
        return DMX_BAD_DMXID;
    }

    dmx = &gDemuxInfo[dmxid];

    if (chid >= TOTAL_CHANNELS || dmx->ChannelInfoTable[chid].InUseFlag == CHANNEL_FREE)
        return DMX_BAD_CHID;
        
    if (fid >= TOTAL_FILTERS)
        return DMX_BAD_FID;
    
    /* TEMPORARY WORKAROUND!  It appears that filter is enabled if bit
       is set in DPS_NEGATIVE_MODE_REG!  Parser ucode fix required?    */
    *DPS_NEGATIVE_MODE_REG_EX(dmxid) &= ~(1<<fid);

    if (dmx->FilterTable[fid].OwnerChid == chid) {
        if (dmx->FilterTable[fid].FilterEnabled == GEN_DEMUX_DISABLE) {
            dmx->AvailableFilters++;
            dmx->ChannelInfoTable[chid].FiltersAllocated &= ~(1<<fid);
            dmx->FilterTable[fid].OwnerChid = GENDMX_BAD_CHANNEL;
            dmx->FilterTable[fid].FilterSize = 0;
            for (i = 0; i < (GENDMX_MAX_HW_FILTER_SIZE/4); ++i) {
                dmx->FilterTable[fid].Mask[i] = 0;
                dmx->FilterTable[fid].Match[i] = 0;
                dmx->FilterTable[fid].NotMask[i] = 0;
            }
            dmx->FilterTable[fid].NotMaskZero = TRUE;
            dmx->FilterTable[fid].NewFilter = TRUE;
            dmx->FilterTable[fid].ExtFilterSize = 0;
            dmx->FilterTable[fid].VersionEqual = TRUE;
            dmx->FilterTable[fid].ExtFilterEnabled = GEN_DEMUX_DISABLE;
            for (i = 0; i < (GENDMX_EXT_FILTER_SIZE/4); ++i) {
                dmx->FilterTable[i].ExtMatch[i] = 0;
                dmx->FilterTable[i].ExtMask[i] = 0;
            }
            dmx->FilterTable[i].ExtFilterOffset = 0;
            return DMX_STATUS_OK;
        }
    }
    return DMX_BAD_FID;
}

/*****************************************************************************
 * cnxt_dmx_channel_set_filter
 *
 * parameters:
 *    dmxid - demux hardware instance handle that was returned by _open()
 *    chid - the channel instance that was returned by _channel_open()
 *    fid - the filter instance that was obtained from _filter_open()
 *    filter - pointer to filter to match                               
 *    mask  - pointer to the don't care mask                           
 *    notmask - pointer to the not equal filter                        
 *
 * returns:
 *    DMX_STATUS_OK - no errors
 *    DMX_BAD_DMXID - bad demux instance handle
 *    DMX_BAD_CHID - bad channel instance handle
 *    DMX_BAD_FID - bad filter instance handle
 *    DMX_BAD_FILTER - bad filter parameters
 *
 * description:
 *    Assigns a filter ID to this channel.  Note that this does not have
 *    any effect on hardware until _control_channel() is used to enable
 *    the channel.
 ****************************************************************************/
DMX_STATUS cnxt_dmx_set_filter(u_int32 dmxid, u_int32 chid, u_int32 fid, 
                               u_int8 *filter, u_int8 *mask, u_int8 *notmask) {
    DemuxInfo *dmx;
    u_int32 i, vm_bit;
    u_int8 *pFilter, *pMask;
#if PARSER_FILTERING == FILTER_888
    u_int8 *pMode;
    u_int8 *LocalNotMask, *LocalMask;
    bool   NotMaskNonZero = FALSE;
#endif
    bool ks;

    trace_new(DPS_FUNC_TRACE,"DEMUX:cnxt_dmx_set_filter entered\n");

    if(dmxid >= MAX_DEMUX_UNITS || dmxid == 1) { /* Sanity check dmxid value, avoid demux 1 & going
                                                    outside array bounds in next check */
      trace_new(DPS_ERROR_MSG, "DEMUX:cnxt_dmx_set_filter: demux %d does not exist!\n",dmxid);
      return DMX_BAD_DMXID;
    }

    /* First check to see that this demux id is already allocated */
    if (!gDemuxInfo[dmxid].DemuxInitialized) {
        trace_new(DPS_ERROR_MSG,"DEMUX:cnxt_dmx_set_filter: demux %d does not exist!\n",dmxid);
        return DMX_BAD_DMXID;
    }

    dmx = &gDemuxInfo[dmxid];

    if (chid >= TOTAL_CHANNELS || dmx->ChannelInfoTable[chid].InUseFlag == CHANNEL_FREE)
        return DMX_BAD_CHID;

    if (fid >= TOTAL_FILTERS || dmx->FilterTable[fid].OwnerChid != chid)
        return DMX_BAD_FID;

    trace_new( DPS_FUNC_TRACE,"filter_size = %d\n", dmx->FilterTable[fid].FilterSize);

#if PARSER_FILTERING == FILTER_1212
    if (notmask != NULL) {
        return DMX_BAD_FILTER;
    }
#else

    /* Determine if we have a negative mode bit on that is masked in the filter */

    if (notmask != NULL) {
        LocalMask = mask;
        LocalNotMask = notmask;

        for (i = 0; i < GENDMX_MAX_HW_FILTER_SIZE+2; ++i) {

           if (i !=1 && i != 2) {   /* skip section length part */
                if (*LocalMask & *LocalNotMask) {
                   NotMaskNonZero = TRUE; 
                   break; /* Found a negative mode for any bit, get out of the check */
                }
           } 
           LocalMask++;
           LocalNotMask++;
        }
    }

    if (NotMaskNonZero) {
        /* NegativeModeReg must be told to use not equal mask */
        *DPS_NEGATIVE_MODE_REG_EX(dmxid) |= (1<<fid); 
        dmx->FilterTable[fid].NotMaskZero = FALSE;
    } else {
        *DPS_NEGATIVE_MODE_REG_EX(dmxid) &= ~(1<<fid);
        dmx->FilterTable[fid].NotMaskZero = TRUE;
    }

#endif

#ifdef DEBUG
    /* save the old filter for debug */
    for (i = 0; i < (GENDMX_MAX_HW_FILTER_SIZE/4); ++i) {
        dmx->FilterTable[fid].OldMatch[i] = dmx->FilterTable[fid].Match[i];
        dmx->FilterTable[fid].OldMask[i] = dmx->FilterTable[fid].Mask[i];
    }
#endif
    /*
    First copy the match and mask to our local table
    */

    ks = critical_section_begin();                    /* so we never use an invalid filter */
    pFilter = (u_int8 *) &dmx->FilterTable[fid].Match[0];
    pMask   = (u_int8 *) &dmx->FilterTable[fid].Mask[0];
#if PARSER_FILTERING == FILTER_888
    pMode   = (u_int8 *) &dmx->FilterTable[fid].NotMask[0];
#endif
    *pFilter++ = *filter++;
    *pMask++   = *mask++;
#if PARSER_FILTERING == FILTER_888
    if ( NotMaskNonZero )
    {
       *pMode++   = *notmask++;
       notmask += 2;
    }
    else
    {
       *pMode = 0;
    }
#endif
#if PARSER_TYPE == DVB_PARSER
#if FILTER_ON_LENGTH == NO
    /* skip length field in DVB sections */
    filter += 2;
    mask += 2;
#endif
#endif
    for (i = 1; i < dmx->FilterTable[fid].FilterSize; ++i) {  /* the filter length includes 2 bytes of packet length */
        *pFilter++ = *filter++;            /* save the match byte */
        *pMask++   = *mask++;              /* save the mask byte */
#if PARSER_FILTERING == FILTER_888
        if ( NotMaskNonZero )
        {
           *pMode++   = *notmask++;
        }
        else
        {
           *pMode++   = 0;
        }
#endif
    }

    /*
    Now zero out the unused parts of the match and mask
    */
    for (; i < GENDMX_MAX_HW_FILTER_SIZE; ++i) {
        *pFilter++ = 0;
        *pMask++   = 0;
#if PARSER_FILTERING == FILTER_888
        *pMode++   = 0;
#endif
    }
    critical_section_end(ks);

    /*
    Next copy the match and mask into the hardware 32 bits
    at a time;
    */
    dmx->FilterTable[fid].VersionEqual = TRUE;
    if (dmx->HWFiltering) {
        for (i = 0; i < (GENDMX_MAX_HW_FILTER_SIZE/4); ++i) {
            *DPS_PATTERN_BASE_EX(dmxid,fid,i) = (BSWAP(dmx->FilterTable[fid].Match[i]));
            *DPS_FILTER_MASK_BASE_EX(dmxid,fid,i) = (BSWAP(dmx->FilterTable[fid].Mask[i]));
#if PARSER_FILTERING == FILTER_888
            *DPS_FILTER_MODE_BASE_EX(dmxid,fid,i) = (BSWAP(dmx->FilterTable[fid].NotMask[i]));
#endif
        }
    }

    /* clear the version mode bit */
    vm_bit = (1<<fid);
#if (PARSER_MICROCODE == UCODE_COLORADO || PARSER_MICROCODE == UCODE_HONDO || PARSER_MICROCODE == UCODE_WABASH)
    *DPS_VERSION_MODES_REG_EX(dmxid) &= ~vm_bit;
#else
      /* Assume only 32-filter configuration for now!  Necessary to program each of 
         the banks (depends upon which bank mapping mode we happen to be in?).      */
    *DPS_VERSION_MODES_REG_EX_0_BANK_01(dmxid) &= ~vm_bit;
    *DPS_VERSION_MODES_REG_EX_1_BANK_01(dmxid) &= ~vm_bit;
    if(fid >= 16) {
      vm_bit = (1<<(fid-16));
      *DPS_VERSION_MODES_REG_EX_0_BANK_12(dmxid) &= ~vm_bit;
    } else {
      vm_bit &= 0x00ff;
      *DPS_VERSION_MODES_REG_EX_0_BANK_02(dmxid) &= ~vm_bit;
    }
#endif

    /* If we are setting a filter for the first time, then do not set the info change bit.  */
    /* This is a fix for OpenTV.  We were receiving a stream of PSI packets, when all of a  */
    /* sudden, a new filter was set on the pid we're the receiving packets on.  This caused */
    /* us to miss a section which ended up slowing down performance.                        */

    if (!dmx->FilterTable[fid].NewFilter) {
       *DPS_INFO_CHANGE_REG_EX(dmxid) |= (1<<dmx->ChannelInfoTable[chid].Slot);
    }
    else { /* We've set a filter now - so now we have an old filter */
       dmx->FilterTable[fid].NewFilter = FALSE;
    }

#ifdef DEBUG
#if PARSER_FILTERING == FILTER_1212
    trace_new( DPS_FUNC_TRACE, (char *)"Filter= %08x %08x %08x\n", dmx->FilterTable[fid].Match[0],
              dmx->FilterTable[fid].Match[1],
              dmx->FilterTable[fid].Match[2]);

    trace_new( DPS_FUNC_TRACE,(char *)"\nMask= %08x %08x %08x\n", dmx->FilterTable[fid].Mask[0],
              dmx->FilterTable[fid].Mask[1],
              dmx->FilterTable[fid].Mask[2]);
#endif
#endif

    return DMX_STATUS_OK;
}

/*****************************************************************************
 * cnxt_dmx_set_version_filter
 *
 * parameters:
 *    dmxid - demux hardware instance handle that was returned by _open()
 *    chid - the channel instance that was returned by _channel_open()
 *    fid - the filter instance that was obtained from _filter_open()
 *    version - 5 bit version number
 *    version_equal - true if normal compare for equality                          
 *    version - true if filter is to be enabled using params                      
 *
 * returns:
 *    DMX_STATUS_OK - no errors
 *    DMX_BAD_DMXID - bad demux instance handle
 *    DMX_BAD_CHID - bad channel instance handle
 *    DMX_BAD_FID - bad filter instance handle
 *    DMX_BAD_FILTER - bad filter parameters
 *
 * description:
 *    Assigns a version filter ID to this channel.
 ****************************************************************************/
DMX_STATUS cnxt_dmx_set_version_filter(u_int32 dmxid, u_int32 chid, u_int32 fid, 
                                       u_int8 version, bool version_equal, bool enable) {

    DemuxInfo *dmx;
    u_int32 vm_bit;

    trace_new( DPS_FUNC_TRACE,"cnxt_dmx_set_version_filter called chid=%d, fid=%d PID=0x%x\n", (int) chid, (int) fid, dmx->ChannelInfoTable[chid].PID);

    if(dmxid >= MAX_DEMUX_UNITS || dmxid == 1) { /* Sanity check dmxid value, avoid demux 1 & going
                                                    outside array bounds in next check */
      trace_new(DPS_ERROR_MSG, "DEMUX:cnxt_dmx_set_version_filter: demux %d does not exist!\n",dmxid);
      return DMX_BAD_DMXID;
    }

   /* First check to see that this demux id is already allocated */
    if (!gDemuxInfo[dmxid].DemuxInitialized) {
        trace_new(DPS_ERROR_MSG,"DEMUX:cnxt_dmx_set_version_filter: demux %d does not exist!\n",dmxid);
        return DMX_BAD_DMXID;
    }

    dmx = &gDemuxInfo[dmxid];
    if (chid >= TOTAL_CHANNELS)
        return DMX_BAD_CHID;

    if (fid >= TOTAL_FILTERS)
        return DMX_BAD_FID;

    vm_bit = (1<<fid);
    if (version_equal == TRUE) {
#if (PARSER_MICROCODE == UCODE_COLORADO || PARSER_MICROCODE == UCODE_HONDO || PARSER_MICROCODE == UCODE_WABASH)
      *DPS_VERSION_MODES_REG_EX(dmxid) &= ~vm_bit;
#else
      /* Assume only 32-filter configuration for now!  Necessary to program each of 
         the banks (depends upon which bank mapping mode we happen to be in?).      */
      *DPS_VERSION_MODES_REG_EX_0_BANK_01(dmxid) &= ~vm_bit;
      *DPS_VERSION_MODES_REG_EX_1_BANK_01(dmxid) &= ~vm_bit;
      if(fid >= 16) {
        vm_bit = (1<<(fid-16));
        *DPS_VERSION_MODES_REG_EX_0_BANK_12(dmxid) &= ~vm_bit;
      } else {
        vm_bit &= 0x00ff;
        *DPS_VERSION_MODES_REG_EX_0_BANK_02(dmxid) &= ~vm_bit;
      }
#endif
      dmx->FilterTable[fid].VersionEqual = TRUE;
    } else {
#if (PARSER_MICROCODE == UCODE_COLORADO || PARSER_MICROCODE == UCODE_HONDO || PARSER_MICROCODE == UCODE_WABASH)
      *DPS_VERSION_MODES_REG_EX(dmxid) |= vm_bit;
#else
      /* Assume only 32-filter configuration for now!  Necessary to program each of 
         the banks (depends upon which bank mapping mode we happen to be in?).      */
      *DPS_VERSION_MODES_REG_EX_0_BANK_01(dmxid) |= vm_bit;
      *DPS_VERSION_MODES_REG_EX_1_BANK_01(dmxid) |= vm_bit;
      if(fid >= 16) {
        vm_bit = (1<<(fid-16));
        *DPS_VERSION_MODES_REG_EX_0_BANK_12(dmxid) |= vm_bit;
      } else {
        vm_bit &= 0x00ff;
        *DPS_VERSION_MODES_REG_EX_0_BANK_02(dmxid) |= vm_bit;
      }
#endif
      dmx->FilterTable[fid].VersionEqual = FALSE;
    }

    if (enable) {
        version &= 0x1F;
        dmx->FilterTable[fid].Mask[0]  |=  0x3E000000;
        dmx->FilterTable[fid].Match[0] &= ~0x3E000000;
        dmx->FilterTable[fid].Match[0] |= ( (u_int32) version << 25);

        *DPS_PATTERN_BASE_EX(dmxid,fid,0) = BSWAP(dmx->FilterTable[fid].Match[0]);
        *DPS_FILTER_MASK_BASE_EX(dmxid,fid,0) = BSWAP(dmx->FilterTable[fid].Mask[0]);
    } else {
        dmx->FilterTable[fid].VersionEqual = TRUE;
        *DPS_FILTER_MASK_BASE_EX(dmxid,fid,0) &= ~0x001F0000;    
    }

    *DPS_INFO_CHANGE_REG_EX(dmxid) |= (1<<dmx->ChannelInfoTable[chid].Slot);
    return DMX_STATUS_OK;
}

/*****************************************************************************
 * cnxt_dmx_set_ext_filter
 *
 * parameters:
 *    dmxid - demux hardware instance handle that was returned by _open()
 *    chid - the channel instance that was returned by _channel_open()
 *    fid - the filter instance that was obtained from _filter_open()
 *    filter - pointer to filter to match
 *    mask - pointer to the don't care mask
 *    size - pointer to the not equal filter 
 *
 * returns:
 *    DMX_STATUS_OK - no errors
 *    DMX_BAD_DMXID - bad demux instance handle
 *    DMX_BAD_CHID - bad channel instance handle
 *    DMX_BAD_FID - bad filter instance handle
 *    DMX_BAD_FILTER - bad filter parameters
 *
 * description:
 *    Set the extended filter for the specified filter ID 
 ****************************************************************************/
DMX_STATUS cnxt_dmx_set_ext_filter(u_int32 dmxid, u_int32 chid, u_int32 fid, u_int8 *filter,
                                   u_int8 *mask, u_int32 size, u_int32 start_offset) {
    DemuxInfo *dmx;
    u_int32 i;
    u_int8 *pFilter, *pMask;

    if(dmxid >= MAX_DEMUX_UNITS || dmxid == 1) { /* Sanity check dmxid value, avoid demux 1 & going
                                                    outside array bounds in next check */
      trace_new(DPS_ERROR_MSG, "DEMUX:cnxt_dmx_set_ext_filter: demux %d does not exist!\n",dmxid);
      return DMX_BAD_DMXID;
    }

    /* First check to see that this demux id is already allocated */
    if (!gDemuxInfo[dmxid].DemuxInitialized) {
        trace_new(DPS_ERROR_MSG,"DEMUX:cnxt_dmx_set_ext_filter: demux %d does not exist!\n",dmxid);
        return DMX_BAD_DMXID;
    }

    dmx = &gDemuxInfo[dmxid];

    trace_new( DPS_FUNC_TRACE,"cnxt_dmx_set_ext_filter chid=%d, fid=%d PID=0x%x\n", (int) chid, (int) fid, dmx->ChannelInfoTable[chid].PID);

    if (chid >= TOTAL_CHANNELS)
        return DMX_BAD_CHID;

    if (fid >= TOTAL_FILTERS || dmx->FilterTable[fid].OwnerChid != chid)
        return DMX_BAD_FID;

    if (size > GENDMX_EXT_FILTER_SIZE) 
        return DMX_BAD_FILTER;

    dmx->FilterTable[fid].ExtFilterSize = size;
    dmx->FilterTable[fid].ExtFilterEnabled = GEN_DEMUX_ENABLE;
    dmx->FilterTable[fid].ExtFilterOffset  = start_offset;

    /*
    First copy the match and mask to our local table
    */
    pFilter = (u_int8 *) &dmx->FilterTable[fid].ExtMatch[0];
    pMask   = (u_int8 *) &dmx->FilterTable[fid].ExtMask[0];
    for (i = 0; i < size; ++i) {
        *pFilter++ = *filter++;            /* save the match byte */
        *pMask++   = *mask++;              /* save the mask byte */
    }
    dmx->FilterTable[fid].ExtFilterOffset = start_offset;
    /*
    Now zero out the unused parts of the match and mask
    */
    for (; i < GENDMX_EXT_FILTER_SIZE; ++i) {
        *pFilter++ = 0;
        *pMask++   = 0;
    }
    return DMX_STATUS_OK;
}

/*****************************************************************************
 * cnxt_dmx_filter_control
 *
 * parameters:
 *    dmxid - demux hardware instance handle that was returned by _open()
 *    chid - the channel instance that was returned by _channel_open()
 *    fid - the filter instance that was obtained from _filter_open()
 *    enable_disable - function to perform on the filter                        
 *
 * returns:
 *    DMX_STATUS_OK - no errors
 *    DMX_BAD_DMXID - bad demux instance handle
 *    DMX_BAD_CHID - bad channel instance handle
 *    DMX_BAD_FID - bad filter instance handle
 *    DMX_BAD_FILTER - bad filter parameters
 *
 * description:
 *    Enable or disable individual filters. The sections currently being received may be  
 *    discarded, but sections already received are not.                                       
 ****************************************************************************/
DMX_STATUS cnxt_dmx_filter_control(u_int32 dmxid, u_int32 chid, 
                                   u_int32 fid, gencontrol_channel_t enable_disable) {
    DemuxInfo *dmx;

	#ifdef DVNCA
	bool bOff = FALSE;
	#endif

    trace_new(DPS_FUNC_TRACE,"DEMUX:cnxt_dmx_filter_control entered\n");

    if(dmxid >= MAX_DEMUX_UNITS || dmxid == 1) { /* Sanity check dmxid value, avoid demux 1 & going
                                                    outside array bounds in next check */
      trace_new(DPS_ERROR_MSG, "DEMUX:cnxt_dmx_filter_control: demux %d does not exist!\n",dmxid);
      return DMX_BAD_DMXID;
    }

    /* First check to see that this demux id is already allocated */
    if (!gDemuxInfo[dmxid].DemuxInitialized) {
        trace_new(DPS_ERROR_MSG,"DEMUX:cnxt_dmx_filter_control: demux %d does not exist!\n",dmxid);
        return DMX_BAD_DMXID;
    }

    dmx = &gDemuxInfo[dmxid];

    if (chid >= TOTAL_CHANNELS || dmx->ChannelInfoTable[chid].InUseFlag == CHANNEL_FREE)
        return DMX_BAD_CHID;

    if (fid >= TOTAL_FILTERS || dmx->FilterTable[fid].OwnerChid != chid)
        return DMX_BAD_FID;

	#ifdef DVNCA
	if( (enable_disable == GEN_DEMUX_ENABLE) &&
	     (dmx->ChannelInfoTable[chid].TagSet) && 
	     (dmx->ChannelInfoTable[chid].Tag == 0x80000001) &&
	     (dmx->ChannelInfoTable[chid].DemuxEnable == FALSE) )
		bOff = TRUE;
	#endif


    switch(enable_disable) {
      case GEN_DEMUX_DISABLE:
      case GEN_DEMUX_ENABLE:
      case GEN_DEMUX_RESET:
      case GEN_DEMUX_DISABLE_RESET:
        dmx->FilterTable[fid].FilterEnabled = enable_disable;
        break;
      default:
        return DMX_BAD_CTL;
    }

    if (dmx->FilterTable[fid].FilterEnabled == GEN_DEMUX_ENABLE) {
        dmx->ChannelInfoTable[chid].FilterEnable |= (1 << fid); /* enable filter */

        *DPS_FILTER_CONTROL_BASE_EX(dmxid,chid) |= dmx->ChannelInfoTable[chid].FilterEnable;
        if (dmx->ChannelInfoTable[chid].DisabledByFilter == TRUE) {
            trace_new(DPS_ERROR_MSG,"DEMUX: Channel %d being reenabled automatically\n", chid);
            if (cnxt_dmx_channel_control(dmxid, chid, (gencontrol_channel_t) GEN_DEMUX_ENABLE) != DMX_STATUS_OK) {
                trace_new(DPS_ERROR_MSG,"DEMUX:cnxt_dmx_filter_control failed\n");
                return  DMX_BAD_FILTER;
            }
        }
    } else {
        dmx->ChannelInfoTable[chid].FilterEnable &= ~(1 << fid); /* disable filter */
        *DPS_FILTER_CONTROL_BASE_EX(dmxid,chid) &= ~(1 << fid);
    }
    if (dmx->HWFiltering) {
        if (dmx->ChannelInfoTable[chid].FilterEnable != 0) {
            *DPS_FILTER_CONTROL_BASE_EX(dmxid,chid) = dmx->ChannelInfoTable[chid].FilterEnable;
        } else {
            /*
             * This code was added by VENDOR_C for a fix (no defect number is known).
             *
             * If no filters are enabled then disable the PID and remember that
             *
             * trace_new(DPS_ERROR_MSG,"DEMUX: Last filter on channel %d disabled,
             *        disabling channel\n", chid);
             * if (cnxt_dmx_channel_control(dmxid, chid, (gencontrol_channel_t) GEN_DEMUX_DISABLE)
             *   != DMX_STATUS_OK) {
             *   trace_new(DPS_ERROR_MSG,"DEMUX:cnxt_dmx_filter_control failed\n");
             * }
             * dmx->ChannelInfoTable[chid].DisabledByFilter = TRUE;
             */

            /* Finally actually disable the filter in the hardware (after PID is disabled) */
            *DPS_FILTER_CONTROL_BASE_EX(dmxid,chid) = 0;
        }
    }
	#ifdef DVNCA
	if(bOff == TRUE) 
	{
		cnxt_dmx_channel_control(dmxid, chid, GEN_DEMUX_ENABLE);
	}
	#endif

    return DMX_STATUS_OK;
}

/*****************************************************************************
 * cnxt_dmx_set_section_channel_attributes
 *
 * parameters:
 *    dmxid          - demux hardware instance handle that was returned by _open()
 *    chid           - channel ID                                                  
 *    header_notify  - Header notification function                                
 *    section_notify - Section notification function                               
 *    TimeOut        - Time out in milliseconds (ms) for callbacks (optional)      
 *    MinHdrSize     - Minimum header size needed                                  
 *
 * returns:
 *    DMX_STATUS_OK - no errors
 *    DMX_BAD_DMXID - bad demux instance handle
 *    DMX_BAD_CHID - bad channel instance handle
 *
 * description:
 *    Sets attributes for a PSI channel.  This includes setting the header and     
 *    section notification functions, time_out (ms), and minimum header size       
 *    used in the header notification function.                                    
 ****************************************************************************/
#ifdef MHP
DMX_STATUS cnxt_dmx_set_section_channel_attributes(u_int32 dmxid, u_int32 chid,
                                                   gen_callback_fct_t header_notify,
                                                   gen_section_callback_fct_t section_notify,
                                                   u_int32 TimeOut, u_int32 MinHdrSize)
#else
DMX_STATUS cnxt_dmx_set_section_channel_attributes(u_int32 dmxid, u_int32 chid, 
                                                   gen_callback_fct_t header_notify,
                                                   gen_callback_fct_t section_notify,
                                                   u_int32 TimeOut, u_int32 MinHdrSize)
#endif
{
    DemuxInfo *dmx;
    task_id_t  pidCaller;

    trace_new(DPS_FUNC_TRACE,"DEMUX:cnxt_dmx_set_section_channel_attributes called\n");

    if(dmxid >= MAX_DEMUX_UNITS || dmxid == 1) { /* Sanity check dmxid value, avoid demux 1 & going
                                                    outside array bounds in next check */
      trace_new(DPS_ERROR_MSG, "DEMUX:cnxt_dmx_set_section_channel_attributes: demux %d does not exist!\n",dmxid);
      return DMX_BAD_DMXID;
    }

    /* First check to see that this demux id is already allocated */
    if (!gDemuxInfo[dmxid].DemuxInitialized) {
        trace_new(DPS_ERROR_MSG,"DEMUX:cnxt_dmx_set_section_channel_attributes: demux %d does not exist!\n",dmxid);
        return DMX_BAD_DMXID;
    }

    dmx = &gDemuxInfo[dmxid];

    if (chid >= TOTAL_CHANNELS || dmx->ChannelInfoTable[chid].InUseFlag == CHANNEL_FREE)
        return DMX_BAD_CHID;

    /* Only get the PSI semaphore if this function has not been called from the */
    /* section- or header-notify callback!                                      */
    pidCaller = task_id();
    
    if(pidCaller != PSIProcessID)
    {
#ifdef IPANEL_SEM_LOCKED_TEST1 
	    if( flag_pop_sec_to_callback )  // GenDmxPSITaskProcSem is already locked!
	    {
	    	sem_put(GenDmxPSITaskProcSem) ; 
	    }
#endif

      sem_get(GenDmxPSITaskProcSem, KAL_WAIT_FOREVER);
    }

    dmx->ChannelInfoTable[chid].HdrErrNotify   = (gen_callback_fct_t) header_notify;
    #ifdef MHP
    dmx->ChannelInfoTable[chid].DataNotify     = (gen_section_callback_fct_t) section_notify;
    #else
    dmx->ChannelInfoTable[chid].DataNotify     = (gen_callback_fct_t) section_notify;
    #endif
    dmx->ChannelInfoTable[chid].TimeOutMS      = TimeOut;

    /*
     * Ensure the timer is stopped
     */
    if(dmx->ChannelInfoTable[chid].ChannelTimerActive)
    {
        if(RC_OK != tick_stop (dmx->ChannelInfoTable[chid].ChannelTimer))
        {
            trace_new(DPS_ERROR_MSG,"DEMUX:tick_stop failed.\n");
        }
        dmx->ChannelInfoTable[chid].ChannelTimerActive = FALSE;
    }

    /*
     * Set the timeout frequency, allocate timer if necessary
     */
    if(TimeOut)
    {
        if(!dmx->ChannelInfoTable[chid].ChannelTimer)
        {
            /*
             * If there's a timeout and no timer allocated, allocate one
             */
            dmx->ChannelInfoTable[chid].ChannelTimer = tick_create (
                  (PFNTIMERC) gen_demux_timer_call_back,
                  (void *)(((u_int32)dmxid<<16)|(chid&0xffff)),
                  NULL);

            if(!dmx->ChannelInfoTable[chid].ChannelTimer)
            {
                trace_new(DPS_ERROR_MSG,"DEMUX:tick_create failed.\n");
            }
        }

        /*
         * Set timeout frequency
         */
        if(RC_OK != tick_set (dmx->ChannelInfoTable[chid].ChannelTimer,
                  dmx->ChannelInfoTable[chid].TimeOutMS, FALSE))
        {
            trace_new(DPS_ERROR_MSG,"DEMUX:tick_set failed.\n");
        }

        dmx->ChannelInfoTable[chid].ChannelTimerActive = FALSE;
    }
    
    /* Free up any previous header allocation */
    if(dmx->ChannelInfoTable[chid].HdrAlloc)
      mem_free(dmx->ChannelInfoTable[chid].HdrAlloc);

    if (MinHdrSize) {
        dmx->ChannelInfoTable[chid].HdrSize    = (MinHdrSize+3)&~0x3;
        dmx->ChannelInfoTable[chid].HdrAlloc   = mem_malloc(MinHdrSize+4);
        dmx->ChannelInfoTable[chid].HdrArea    = (u_int8*)(((u_int32)dmx->ChannelInfoTable[chid].HdrAlloc+3)&~0x3);
    } else {
        dmx->ChannelInfoTable[chid].HdrSize    = 0;
        dmx->ChannelInfoTable[chid].HdrAlloc   = NULL;
        dmx->ChannelInfoTable[chid].HdrArea    = NULL;
    }

    if(pidCaller != PSIProcessID)
      sem_put(GenDmxPSITaskProcSem);

    return DMX_STATUS_OK;
}

/*****************************************************************************
 * cnxt_dmx_set_section_channel_tag
 *
 * parameters:
 *    dmxid          - demux hardware instance handle that was returned by _open()
 *    chid           - channel ID                                                  
 *    tag            - tag value to pass in all callbacks from this channel        
 *
 * returns:
 *    DMX_STATUS_OK - no errors
 *    DMX_BAD_DMXID - bad demux instance handle
 *    DMX_BAD_CHID - bad channel instance handle
 *
 * description:
 *    Sets the tag value for a PSI channel.  This value will be included in the
 *    structure passed to all future header and section notifications for this
 *    channel.
 ****************************************************************************/
DMX_STATUS cnxt_dmx_set_section_channel_tag(u_int32 dmxid, u_int32 chid, u_int32 tag)
{
    DemuxInfo *dmx;
    task_id_t  pidCaller;

    trace_new(DPS_FUNC_TRACE,"DEMUX:cnxt_dmx_set_section_channel_tag called\n");

    if(dmxid >= MAX_DEMUX_UNITS || dmxid == 1) { /* Sanity check dmxid value, avoid demux 1 & going
                                                    outside array bounds in next check */
      trace_new(DPS_ERROR_MSG, "DEMUX:cnxt_dmx_set_section_channel_tag: demux %d does not exist!\n",dmxid);
      return DMX_BAD_DMXID;
    }

    /* First check to see that this demux id is already allocated */
    if (!gDemuxInfo[dmxid].DemuxInitialized) {
        trace_new(DPS_ERROR_MSG,"DEMUX:cnxt_dmx_set_section_channel_tag: demux %d does not exist!\n",dmxid);
        return DMX_BAD_DMXID;
    }

    dmx = &gDemuxInfo[dmxid];

    if (chid >= TOTAL_CHANNELS || dmx->ChannelInfoTable[chid].InUseFlag == CHANNEL_FREE)
        return DMX_BAD_CHID;

    /* Only get the PSI semaphore if this function has not been called from the */
    /* section- or header-notify callback!                                      */
    pidCaller = task_id();
    
    if(pidCaller != PSIProcessID)
    {
#ifdef IPANEL_SEM_LOCKED_TEST1 
	    if( flag_pop_sec_to_callback )  // GenDmxPSITaskProcSem is already locked!
	    {
	    	sem_put(GenDmxPSITaskProcSem) ; 
	    }
#endif

      sem_get(GenDmxPSITaskProcSem, KAL_WAIT_FOREVER);
    }

    dmx->ChannelInfoTable[chid].TagSet = TRUE;
    dmx->ChannelInfoTable[chid].Tag    = tag;

    if(pidCaller != PSIProcessID)
      sem_put(GenDmxPSITaskProcSem);

    return DMX_STATUS_OK;
}

/*****************************************************************************
 * cnxt_dmx_set_pes_channel_attributes
 *
 * parameters:
 *    dmxid          - demux hardware instance handle that was returned by _open()
 *    chid           - channel ID                                                  
 *    data_notify    - Section notification function                                
 *    error_notify   - Error notification function                               
 *
 * returns:
 *    DMX_STATUS_OK - no errors
 *    DMX_BAD_DMXID - bad demux instance handle
 *    DMX_BAD_CHID - bad channel instance handle
 *
 * description:
 *    Sets attributes for a PES channel.  This includes setting the data and     
 *    error notification functions of callbacks for data notifies.  
 ****************************************************************************/
DMX_STATUS cnxt_dmx_set_pes_channel_attributes(u_int32 dmxid, u_int32 chid, 
                                               gen_callback_fct_t data_notify,
                                               gen_callback_fct_t error_notify)
{
    DemuxInfo *dmx;
    task_id_t tidCaller;

    trace_new(DPS_FUNC_TRACE,"DEMUX:cnxt_dmx_set_pes_channel_attributes called\n");

    if(dmxid >= MAX_DEMUX_UNITS || dmxid == 1) { /* Sanity check dmxid value, avoid demux 1 & going
                                                    outside array bounds in next check */
      trace_new(DPS_ERROR_MSG, "DEMUX:cnxt_dmx_set_pes_channel_attributes: demux %d does not exist!\n",dmxid);
      return DMX_BAD_DMXID;
    }

    /* First check to see that this demux id is already allocated */
    if (!gDemuxInfo[dmxid].DemuxInitialized) {
        trace_new(DPS_ERROR_MSG,"DEMUX:cnxt_dmx_set_pes_channel_attributes: demux %d does not exist!\n",dmxid);
        return DMX_BAD_DMXID;
    }

    dmx = &gDemuxInfo[dmxid];

    if (chid >= TOTAL_CHANNELS || dmx->ChannelInfoTable[chid].InUseFlag == CHANNEL_FREE)
        return DMX_BAD_CHID;


    if ((dmx->ChannelInfoTable[chid].InUseFlag == CHANNEL_FREE) ||
        (!dmx->ChannelInfoTable[chid].PESChannel)) {
        trace_new(DPS_ERROR_MSG,"DEMUX:cnxt_dmx_set_pes_channel_attributes failed\n");
        return DMX_BAD_CHID;
    }

    tidCaller = task_id();
    
    if(tidCaller != PES_TASK_ID)
      sem_get(GenDmxPESTaskProcSem, KAL_WAIT_FOREVER);

    dmx->ChannelInfoTable[chid].HdrErrNotify = (gen_callback_fct_t) error_notify;
#ifdef MHP
    /* MHP change */
    dmx->ChannelInfoTable[chid].DataNotify   = (gen_section_callback_fct_t) data_notify;
#else
    dmx->ChannelInfoTable[chid].DataNotify   = (gen_callback_fct_t) data_notify;
#endif

    if(tidCaller != PES_TASK_ID)
      sem_put(GenDmxPESTaskProcSem);

    return DMX_STATUS_OK;
}

/*****************************************************************************
 * cnxt_dmx_connect_notify
 *
 * parameters:
 *    dmxid - demux hardware instance handle that was returned by _open()
 *    event_type - Connect or Disconnect
 *
 * returns:
 *    DMX_STATUS_OK - no errors
 *    DMX_BAD_DMXID - bad demux instance handle
 *    DMX_ERROR     - bad event_type
 *
 * description:
 *    This function is called from the demod driver to indicate a lock/unlock of a signal. 
 *    When  a lock is indicated the Parser is enabled and disabled otherwise.              
 *    
 ****************************************************************************/
DMX_STATUS cnxt_dmx_connect_notify(u_int32 dmxid, int event_type) {
    u_int32 i;
    bool ks;
    static int LastConnectStatus = GENDMX_UNKNOWN;
    DemuxInfo *dmx;

    trace_new(DPS_FUNC_TRACE,"DEMUX:cnxt_dmx_connect_notify\n");

    if(dmxid >= MAX_DEMUX_UNITS || dmxid == 1) { /* Sanity check dmxid value, avoid demux 1 & going
                                                    outside array bounds in next check */
      trace_new(DPS_ERROR_MSG, "DEMUX:cnxt_dmx_connect_notify: demux %d does not exist!\n",dmxid);
      return DMX_BAD_DMXID;
    }

    /* First check to see that this demux id is already allocated */
    if (!gDemuxInfo[dmxid].DemuxInitialized) {
        trace_new(DPS_ERROR_MSG,"DEMUX:cnxt_dmx_connect_notify: demux %d does not exist!\n",dmxid);
        return DMX_BAD_DMXID;
    }

    dmx = &gDemuxInfo[dmxid];

    if (event_type == LastConnectStatus) {
        return DMX_STATUS_OK;
    }
    LastConnectStatus = event_type;
    switch (event_type) {
    case  GENDMX_CONNECT:
        trace_new(DPS_FUNC_TRACE,"DEMUX:CONNECT NOTIFY\n");
        dmx->DemodConnected = TRUE;
#if defined DRIVER_INCL_NDSDEMUX || defined DRIVER_INCL_NDSICAM
        NotifyCAConnectNotify(dmx->DemuxID, TRUE);
#endif

        for (i = 0; i < TOTAL_CHANNELS; ++i) {
            if (dmx->ChannelInfoTable[i].DemuxEnable) {
                gen_dmx_hw_set_pid(dmxid, i, dmx->ChannelInfoTable[i].PID);
            }
        }

        *DPS_INFO_CHANGE_REG_EX(dmxid) |= 0xFFFFFFFF;
        *DPS_HOST_CTL_REG_EX(dmxid) |= DPS_PID_ENABLE;

#ifdef DEBUG
        trace_new(TRACE_DPS|TRACE_LEVEL_4, "GENDMX:DEMOD IS CONNECTED\n");
#endif

        break;
    case GENDMX_DISCONNECT:
#ifdef DEBUG
        trace_new(TRACE_DPS|TRACE_LEVEL_4, "GENDMX:DEMOD DISCONNECTED. Waiting on Sem...\n");
#endif

        *DPS_HOST_CTL_REG_EX(dmxid) &= ~(DPS_PID_ENABLE);

        ks = critical_section_begin();
        for (i=0;i<32;i++) {
            *DPS_PID_SLOT_READ_PTR_EX(dmxid,i) = *DPS_PID_SLOT_WRITE_PTR_EX(dmxid,i);
        }
        critical_section_end(ks);
        for (i = 0; i < TOTAL_CHANNELS; ++i) {
            gen_dmx_hw_free_pid(dmxid, i);
        }

        *DPS_INFO_CHANGE_REG_EX(dmxid) |= 0xFFFFFFFF;

        trace_new(DPS_FUNC_TRACE,"DEMUX:DISCONNECT NOTIFY\n");

#ifdef DEBUG
        trace_new(TRACE_DPS|TRACE_LEVEL_4, "GENDMX:DEMOD DISCONNECTED. Done Waiting on Sem...\n");
#endif

        dmx->DemodConnected = FALSE;
#if defined DRIVER_INCL_NDSDEMUX || defined DRIVER_INCL_NDSICAM
        NotifyCAConnectNotify(dmx->DemuxID, FALSE);
#endif
        break;
    default:
      return DMX_ERROR;
    }
    return DMX_STATUS_OK;
}

/*****************************************************************************
 * cnxt_dmx_get_channel_write_pointer
 *
 * parameters:
 *    dmxid - demux hardware instance handle that was returned by _open()
 *    chid - the channel instance that was returned by _channel_open()
 *    write_ptr - return value containing the write ptr
 *
 * returns:
 *    DMX_STATUS_OK - no errors
 *    DMX_BAD_DMXID - bad demux instance handle
 *    DMX_BAD_CHID - bad channel instance handle
 *
 * description:
 *    Gets the current queue write pointer 
 ****************************************************************************/
DMX_STATUS cnxt_dmx_get_channel_write_pointer(u_int32 dmxid, u_int32 chid, 
                                              u_int8 **write_ptr) {
    DemuxInfo *dmx;

    *write_ptr = NULL;

    trace_new(DPS_FUNC_TRACE,"DEMUX:cnxt_dmx_get_channel_write_pointer\n");

    if(dmxid >= MAX_DEMUX_UNITS || dmxid == 1) { /* Sanity check dmxid value, avoid demux 1 & going
                                                    outside array bounds in next check */
      trace_new(DPS_ERROR_MSG, "DEMUX:cnxt_dmx_get_channel_write_pointer: demux %d does not exist!\n",dmxid);
      return DMX_BAD_DMXID;
    }

    /* First check to see that this demux id is already allocated */
    if (!gDemuxInfo[dmxid].DemuxInitialized) {
        trace_new(DPS_ERROR_MSG,"DEMUX:cnxt_dmx_get_channel_write_pointer: demux %d does not exist!\n",dmxid);
        return DMX_BAD_DMXID;
    }

    dmx = &gDemuxInfo[dmxid];

    if (chid >= TOTAL_CHANNELS ||  
        dmx->ChannelInfoTable[chid].InUseFlag == CHANNEL_FREE ||
        !dmx->ChannelInfoTable[chid].PESChannel) {
        return DMX_BAD_CHID;
    } else {
        *write_ptr = dmx->ChannelInfoTable[chid].pWritePtr;
        return DMX_STATUS_OK;
    }

    return DMX_STATUS_OK;
}

/*****************************************************************************
 * cnxt_dmx_set_channel_buffer_pointer
 *
 * parameters:
 *    dmxid - demux hardware instance handle that was returned by _open()
 *    chid - the channel instance that was returned by _channel_open()
 *    write_ptr - return value containing the write ptr
 *    buff_size - return valuse containing buffer size 
 *
 * returns:
 *    DMX_STATUS_OK - no errors
 *    DMX_BAD_DMXID - bad demux instance handle
 *    DMX_BAD_CHID - bad channel instance handle
 *
 * description:
 *    Gets the current queue buffer pointer 
 ****************************************************************************/
DMX_STATUS cnxt_dmx_get_channel_buffer_pointer(u_int32 dmxid, u_int32 chid, 
                                               u_int8 **write_ptr, u_int32 *buff_size) {
    DemuxInfo *dmx;

    *write_ptr = NULL;

    trace_new(DPS_FUNC_TRACE,"DEMUX:cnxt_dmx_get_channel_buffer_pointer\n");

    if(dmxid >= MAX_DEMUX_UNITS || dmxid == 1) { /* Sanity check dmxid value, avoid demux 1 & going
                                                    outside array bounds in next check */
      trace_new(DPS_ERROR_MSG, "DEMUX:cnxt_dmx_get_channel_buffer_pointer: demux %d does not exist!\n",dmxid);
      return DMX_BAD_DMXID;
    }

    /* First check to see that this demux id is already allocated */
    if (!gDemuxInfo[dmxid].DemuxInitialized) {
        trace_new(DPS_ERROR_MSG,"DEMUX:cnxt_dmx_get_channel_buffer_pointer: demux %d does not exist!\n",dmxid);
        return DMX_BAD_DMXID;
    }

    dmx = &gDemuxInfo[dmxid];
    if (chid >= TOTAL_CHANNELS || dmx->ChannelInfoTable[chid].InUseFlag == CHANNEL_FREE)
        return DMX_BAD_CHID;

    /* For audio and video channels, we only care about the buffer size. */
    if (dmx->ChannelInfoTable[chid].stype == VIDEO_PES_TYPE) {
      *buff_size    = gen_dmx_get_video_buff_size() ;
      return DMX_STATUS_OK;
    } else if (dmx->ChannelInfoTable[chid].stype == AUDIO_PES_TYPE) {
        *buff_size    = gen_dmx_get_audio_buff_size();
        return DMX_STATUS_OK;
    }

    *write_ptr = dmx->ChannelInfoTable[chid].pBuffer;
    *buff_size = dmx->ChannelInfoTable[chid].pBufferEnd - dmx->ChannelInfoTable[chid].pBuffer;
    return DMX_STATUS_OK;
}

/*****************************************************************************
 * cnxt_dmx_set_pcr_pid
 *
 * parameters:
 *    dmxid - demux hardware instance handle that was returned by _open()
 *    pid - the pid value to set in the pcr
 *
 * returns:
 *    DMX_STATUS_OK - no errors
 *    DMX_BAD_DMXID - bad demux instance handle
 *
 * description:
 *    DMX_BAD_CHID - bad channel instance handle
 *    Sets the current pcr pid
 ****************************************************************************/
DMX_STATUS cnxt_dmx_set_pcr_pid(u_int32 dmxid, u_int16 pid) {
    DemuxInfo *dmx;
    bool ks;

    trace_new(DPS_FUNC_TRACE,"DEMUX:cnxt_dmx_set_pcr_pid\n");

    if(dmxid >= MAX_DEMUX_UNITS || dmxid == 1) { /* Sanity check dmxid value, avoid demux 1 & going
                                                    outside array bounds in next check */
      trace_new(DPS_ERROR_MSG, "DEMUX:cnxt_dmx_set_pcr_pid: demux %d does not exist!\n",dmxid);
      return DMX_BAD_DMXID;
    }

    /* First check to see that this demux id is already allocated */
    if (!gDemuxInfo[dmxid].DemuxInitialized) {
        trace_new(DPS_ERROR_MSG,"DEMUX:cnxt_dmx_set_pcr_pid: demux %d does not exist!\n",dmxid);
        return DMX_BAD_DMXID;
    }

    dmx = &gDemuxInfo[dmxid];


    if (dmx->PCR_PID_Value != pid) {
      ks = critical_section_begin();
      *DPS_PCR_PID_EX(dmxid) = pid;
      dmx->PCR_PID_Value = pid;
      NewPCRFlag = FALSE;
      critical_section_end(ks);
    }
    else { 
      trace_new(DPS_FUNC_TRACE, "DEMUX:cnxt_dmx_set_pcr_pid: PCR ALREADY SET!!\n"); 
    } 


    return DMX_STATUS_OK;
}

/*****************************************************************************
 * cnxt_dmx_get_current_pcr
 *
 * parameters:
 *    dmxid - demux hardware instance handle that was returned by _open()
 *    current_pcr - the pcr value that is returned
 *
 * returns:
 *    DMX_STATUS_OK - no errors
 *    DMX_BAD_DMXID - bad demux instance handle
 *    DMX_ERROR - if NewPCRFlag is FALSE
 *
 * description:
 *    Returns the current PCR 
 ****************************************************************************/
DMX_STATUS cnxt_dmx_get_current_pcr(u_int32 dmxid, genpcr_t *current_pcr) {
    u_int32 val;
    DemuxInfo *dmx;

    trace_new(DPS_FUNC_TRACE,"DEMUX:cnxt_dmx_get_current_pcr\n");

    if(dmxid >= MAX_DEMUX_UNITS || dmxid == 1) { /* Sanity check dmxid value, avoid demux 1 & going
                                                    outside array bounds in next check */
      trace_new(DPS_ERROR_MSG, "DEMUX:cnxt_dmx_get_current_pcr: demux %d does not exist!\n",dmxid);
      return DMX_BAD_DMXID;
    }

    /* First check to see that this demux id is already allocated */
    if (!gDemuxInfo[dmxid].DemuxInitialized) {
        trace_new(DPS_ERROR_MSG,"DEMUX:cnxt_dmx_get_current_pcr: demux %d does not exist!\n",dmxid);
        return DMX_BAD_DMXID;
    }

    dmx = &gDemuxInfo[dmxid];

    if (NewPCRFlag == FALSE)
        return DMX_ERROR;
    else {
        current_pcr->pcr_base_32lsb = (u_int32) *glpPCRLow;
        val = (u_int32) *glpPCRHigh;
        current_pcr->pcr_base_1msb  = (u_int32) val & 0x01;
        current_pcr->pcr_extension_9b = (u_int32) (val >> 1) & 0x1FF;
        trace_new(DPS_FUNC_TRACE,(char *)"PCR base=0x%x, ext=0x%x\n",
                  current_pcr->pcr_base_1msb, current_pcr->pcr_extension_9b);
    }
    return DMX_STATUS_OK;
}

/*****************************************************************************
 * cnxt_dmx_register_pcr_notify
 *
 * parameters:
 *    dmxid - demux hardware instance handle that was returned by _open()
 *    pcr_notify - Callback function for pcr arrival
 *
 * returns:
 *    DMX_STATUS_OK - no errors
 *
 * description:
 *    Register the pcr_notify function to be called when a pcr arrives in a 
 *    TS packet.
 ****************************************************************************/
DMX_STATUS cnxt_dmx_register_pcr_notify(gen_pcr_callback_fct_t pcr_notify) {

    trace_new(DPS_FUNC_TRACE,"DEMUX:cnxt_dmx_register_pcr_notify\n");

    gPCRCallBack = pcr_notify;

    return DMX_STATUS_OK;
}

/*****************************************************************************
 * cnxt_dmx_query_pcr_notify
 *
 * parameters:
 *    dmxid - demux hardware instance handle that was returned by _open()
 *    pcr_notify - Pointer to storage for returned function pointer
 *
 * returns:
 *    DMX_STATUS_OK - no errors
 *
 * description:
 *    Query the current PCR notification callback pointer. 
 ****************************************************************************/
DMX_STATUS cnxt_dmx_query_pcr_notify(gen_pcr_callback_fct_t *pcr_notify) {

    trace_new(DPS_FUNC_TRACE,"DEMUX:cnxt_dmx_query_pcr_notify\n");

    *pcr_notify = gPCRCallBack;

    return DMX_STATUS_OK;
}

/*****************************************************************************
 * cnxt_dmx_register_event6_notify
 *
 * parameters:
 *    dmxid - demux hardware instance handle that was returned by _open()
 *    pcr_notify - Callback function for event 6 notification
 *
 * returns:
 *    DMX_STATUS_OK - no errors
 *
 * description:
 *    Register the event_notify function to be called when the parser
 *    generates an event 6 interrupt. This is used as a software signal
 *    by some versions of the microcode.
 ****************************************************************************/
DMX_STATUS cnxt_dmx_register_event6_notify(gen_event6_callback_fct_t event_notify){

    trace_new(DPS_FUNC_TRACE,"DEMUX:cnxt_dmx_register_event6_notify\n");

    gEvent6CallBack = event_notify;

    return DMX_STATUS_OK;
}

/*****************************************************************************
 * cnxt_dmx_query_event6_notify
 *
 * parameters:
 *    dmxid - demux hardware instance handle that was returned by _open()
 *    event_notify - Pointer to storage for returned function pointer
 *
 * returns:
 *    DMX_STATUS_OK - no errors
 *
 * description:
 *    Query the current Event 6 notification callback pointer. 
 ****************************************************************************/
DMX_STATUS cnxt_dmx_query_event6_notify(gen_event6_callback_fct_t *event_notify){

    trace_new(DPS_FUNC_TRACE,"DEMUX:cnxt_dmx_query_event6_notify\n");

    *event_notify = gEvent6CallBack;

    return DMX_STATUS_OK;
}

/*****************************************************************************
 * cnxt_dmx_crc_enable
 *
 * parameters:
 *    dmxid - demux hardware instance handle that was returned by _open()
 *    crc_command - command to enable or disable CRC error notification.
 *                  Should be one of the following:
 *          GEN_DEMUX_ENABLE
 *          GEN_DEMUX_DISABLE
 *
 * returns:
 *    DMX_STATUS_OK - no errors
 *    DMX_BAD_DMXID - bad demux instance handle
 *    DMX_ERROR - error in parameter
 *
 * description:
 *    Enables CRC error notification.  If enabled, then sections with CRC
 *    errors detected are notified through the callback mechanism, and for
 *    the section notify, the condition field of the notify structure will
 *    indicate an error.  If CRC error notification is disabled (default),
 *    then sections with CRC errors detected are discarded by the driver
 *    and the client is not notified in any way.
 ****************************************************************************/
DMX_STATUS cnxt_dmx_crc_enable(u_int32 dmxid, gencontrol_channel_t crc_command)
{
    DemuxInfo *dmx;

    trace_new(DPS_FUNC_TRACE,"DEMUX:cnxt_dmx_crc_enable\n");

    if(dmxid >= MAX_DEMUX_UNITS || dmxid == 1) { /* Sanity check dmxid value, avoid demux 1 & going
                                                    outside array bounds in next check */
      trace_new(DPS_ERROR_MSG, "DEMUX:cnxt_dmx_crc_enable: demux %d does not exist!\n",dmxid);
      return DMX_BAD_DMXID;
    }

    /* First check to see that this demux id is already allocated */
    if (!gDemuxInfo[dmxid].DemuxInitialized) {
        trace_new(DPS_ERROR_MSG,"DEMUX:cnxt_dmx_crc_enable: demux %d does not exist!\n",dmxid);
        return DMX_BAD_DMXID;
    }

    dmx = &gDemuxInfo[dmxid];

    if( crc_command == GEN_DEMUX_ENABLE )
    {
        dmx->NotifyCRC = TRUE;
        return DMX_STATUS_OK;
    }
    else if( crc_command == GEN_DEMUX_DISABLE )
    {
        dmx->NotifyCRC = FALSE;
        return DMX_STATUS_OK;
    }

    return DMX_ERROR;
}

/*****************************************************************************
 * cnxt_dmx_set_channel_buffer
 *
 * parameters:
 *    dmxid - demux hardware instance handle that was returned by _open()
 *    chid - channel instance handle that was returned by _channel_open()
 *    buffer - channel buffer pointer
 *    size - channel buffer size
 *
 * returns:
 *    DMX_STATUS_OK - no errors
 *    DMX_BAD_DMXID - bad demux instance handle
 *    DMX_BAD_CHID - bad channel instance handle
 *
 * description:
 *    Passes the channel buffer pointer and size to the driver
 ****************************************************************************/
DMX_STATUS cnxt_dmx_set_channel_buffer(u_int32 dmxid, u_int32 chid, 
                                       void *buffer, u_int32 size) {
    DemuxInfo *dmx;
    u_int32 sz,bf;
    u_int32 slot;
    bool enabled = FALSE;

    trace_new(DPS_FUNC_TRACE,"DEMUX:cnxt_dmx_set_channel_buffer\n");

    if(dmxid >= MAX_DEMUX_UNITS || dmxid == 1) { /* Sanity check dmxid value, avoid demux 1 & going
                                                    outside array bounds in next check */
      trace_new(DPS_ERROR_MSG, "DEMUX:cnxt_dmx_set_channel_buffer: demux %d does not exist!\n",dmxid);
      return DMX_BAD_DMXID;
    }

    /* First check to see that this demux id is already allocated */
    if (!gDemuxInfo[dmxid].DemuxInitialized) {
        trace_new(DPS_ERROR_MSG,"DEMUX:cnxt_dmx_set_channel_buffer: demux %d does not exist!\n",dmxid);
        return DMX_BAD_DMXID;
    }

    dmx = &gDemuxInfo[dmxid];

    if (chid >= TOTAL_CHANNELS || dmx->ChannelInfoTable[chid].InUseFlag == CHANNEL_FREE)
        return DMX_BAD_CHID;


    /*
     * Ensure a buffer is given
     */
    if (!buffer || !size) {
        trace_new(DPS_ERROR_MSG,"DEMUX:cnxt_dmx_set_channel_buffer() invalid buffer/size!\n");
        return DMX_ERROR;
    }

    /*
     * Ensure the buffer is word aligned for the HW
     */

    bf = (u_int32) buffer;
    sz = (u_int32) size;
    if (bf & 3) {
        bf &= (~3);
        bf += 4;
        sz -= (bf-(u_int32)buffer);
        sz &= (~3);
    }

    /* There is an undocumented limitation on the size of any buffer on all Conexant STB's.  Bit
       22 is ignored making the greatest allocatable buffer 32MB.  If we don't check this, we could
       end up overwriting memory.  Should never happen, but do it as a sanity check. 
    */
    if (bf + size > BUFFER_BOUNDRY_32M) {
        trace_new(DPS_ERROR_MSG,"DEMUX:cnxt_dmx_set_channel_buffer() invalid buffer/size!\n");
        return DMX_ERROR;
    } 

    /*
     * If this slot is enabled, we need to disable it before changing
     * the buffer pointers.  We re-enable it after we're done.
     */

    if (dmx->ChannelInfoTable[chid].DemuxEnable) {
        enabled = TRUE;
        if (cnxt_dmx_channel_control(dmxid, chid, GEN_DEMUX_DISABLE) 
            != DMX_STATUS_OK) {
            trace_new(DPS_ERROR_MSG,"DEMUX: cnxt_dmx_control_channel failed\n");
            return  DMX_ERROR;
        }
    }

    /*
     * Save away buffer information
     */

    dmx->ChannelInfoTable[chid].pBuffer       =
    dmx->ChannelInfoTable[chid].pBufferEnd    =
    dmx->ChannelInfoTable[chid].pWritePtr     =
    dmx->ChannelInfoTable[chid].pReadPtr      =
    dmx->ChannelInfoTable[chid].pAckPtr       = (u_int8 *)((u_int32)bf|NCR_BASE);
    dmx->ChannelInfoTable[chid].pBufferEnd   += sz;

    /*
     * Assign the pid slot read, write, start and end ptrs/addresses with default values
     */
    slot = dmx->ChannelInfoTable[chid].Slot;    /* Get slot for this PID */

    *DPS_PID_SLOT_WRITE_PTR_EX(dmxid,slot) = 
    (((u_int32)dmx->ChannelInfoTable[slot].pBuffer&~NCR_BASE)|DPS_PAW_SYS_ADDRESS);
    *DPS_PID_SLOT_READ_PTR_EX(dmxid,slot) = 
    (((u_int32)dmx->ChannelInfoTable[slot].pBuffer&~NCR_BASE)|DPS_PAW_SYS_ADDRESS);
    *DPS_PID_SLOT_START_ADDR_EX(dmxid,slot) = 
    (((u_int32)dmx->ChannelInfoTable[slot].pBuffer&~NCR_BASE)|DPS_PAW_SYS_ADDRESS);
    *DPS_PID_SLOT_END_ADDR_EX(dmxid,slot) = 
    (((u_int32)dmx->ChannelInfoTable[slot].pBufferEnd&~NCR_BASE)|DPS_PAW_SYS_ADDRESS);

    if (dmx->ChannelInfoTable[chid].PESChannel) {
        dmx->ChannelInfoTable[chid].pBuffer    =
        dmx->ChannelInfoTable[chid].pWritePtr  =
        dmx->ChannelInfoTable[chid].pReadPtr   =
        dmx->ChannelInfoTable[chid].pAckPtr    = 
        (u_int8 *)((u_int32)dmx->ChannelInfoTable[chid].pBuffer&~NCR_BASE);
        dmx->ChannelInfoTable[chid].pBufferEnd =
        (u_int8 *)((u_int32)dmx->ChannelInfoTable[chid].pBufferEnd&~NCR_BASE);
    }

    /*
     * If this slot was enabled before changing the buffer pointers,
     * re-enable it now after setting the new buffer pointers.
     */

    if (enabled) {
        if (cnxt_dmx_channel_control(dmxid, chid, GEN_DEMUX_ENABLE)
            != DMX_STATUS_OK) {
            trace_new(DPS_ERROR_MSG,"DEMUX: cnxt_dmx_control_channel failed\n");
            return  DMX_ERROR;
        }
    }

    return DMX_STATUS_OK;
}

/*****************************************************************************
 * cnxt_dmx_read_pes_buffer
 *
 * parameters:
 *    dmxid - demux hardware instance handle that was returned by _open()
 *    chid - channel instance handle that was returned by _channel_open()
 *    size - size of data to read
 *    buffer - pointer to pes buffer
 *
 * returns:
 *    DMX_STATUS_OK - no errors
 *    DMX_BAD_DMXID - bad demux instance handle
 *    DMX_BAD_CHID - bad channel instance handle
 *
 * description:
 *  Copy size bytes from the read pointer to buffer.                     
 *  Only required when demux buffer queue cannot be accessed outside     
 *  the driver.                                                          
 ****************************************************************************/
DMX_STATUS cnxt_dmx_read_pes_buffer(u_int32 dmxid, u_int32 chid, u_int32 size, 
                                    u_int8 *buffer, u_int32 *actual_size) {
    ChannelInformationStruc *pChInfo;
    u_int32 size_to_copy;
    DemuxInfo *dmx;
    task_id_t  tidCaller;

    not_interrupt_safe();
    
    /*  trace_new(DPS_FUNC_TRACE,"DEMUX:cnxt_dmx_read_pes_buffer\n");*/

    if(dmxid >= MAX_DEMUX_UNITS || dmxid == 1) { /* Sanity check dmxid value, avoid demux 1 & going
                                                    outside array bounds in next check */
      trace_new(DPS_ERROR_MSG, "DEMUX:cnxt_dmx_read_pes_buffer: demux %d does not exist!\n",dmxid);
      return DMX_BAD_DMXID;
    }

    /* First check to see that this demux id is already allocated */
    if (!gDemuxInfo[dmxid].DemuxInitialized) {
        trace_new(DPS_ERROR_MSG,"DEMUX:cnxt_dmx_read_pes_buffer: demux %d does not exist!\n",dmxid);
        return DMX_BAD_DMXID;
    }

    dmx = &gDemuxInfo[dmxid];

    if (chid >= TOTAL_CHANNELS || dmx->ChannelInfoTable[chid].InUseFlag == CHANNEL_FREE)
        return DMX_BAD_CHID;

    tidCaller = task_id();
    
    pChInfo = &dmx->ChannelInfoTable[chid];
    if(tidCaller != PES_TASK_ID)
      sem_get(GenDmxPESTaskProcSem, KAL_WAIT_FOREVER);

    /*
     * Determine maximum size that can be copied
     */

    if (size <= pChInfo->PESDataCount) {
        size_to_copy = size;
    } else {
        size_to_copy = pChInfo->PESDataCount;
    }

    /*
     * Copy the data
     */

    WCopyBytes(buffer, pChInfo->pReadPtr, size_to_copy, pChInfo->pBuffer, pChInfo->pBufferEnd);

    /*
     * Update pointers
     */

    AdvancePtr(&pChInfo->pReadPtr, size_to_copy, pChInfo->pBuffer, pChInfo->pBufferEnd);

    /*
     * Indicate data read
     */

    pChInfo->PESDataCount -= size_to_copy;

    if(tidCaller != PES_TASK_ID)
      sem_put(GenDmxPESTaskProcSem);

    *actual_size = size_to_copy;
    return DMX_STATUS_OK;
}

/*****************************************************************************
 * cnxt_dmx_empty_pes_buffer
 *
 * parameters:
 *    dmxid - demux hardware instance handle that was returned by _open()
 *    chid - channel instance handle that was returned by _channel_open()
 *    write - write pointer position up to which data will be discarded
 *
 * returns:
 *    DMX_STATUS_OK - no errors
 *    DMX_BAD_DMXID - bad demux instance handle
 *    DMX_BAD_CHID - bad channel instance handle
 *
 * description:
 *  Discard all data currently in the PES buffer by advancing the read   
 *  pointer to just behind the supplied write pointer. This is used by OpenTV
 *  which assumes that demux PES channels possess only a hardware write
 *  pointer.
 ****************************************************************************/
DMX_STATUS cnxt_dmx_empty_pes_buffer(u_int32 dmxid, u_int32 chid, u_int8 *write) {
    ChannelInformationStruc *pChInfo;
    u_int32 size_to_discard;
    DemuxInfo *dmx;
    task_id_t tidCaller;

    if(dmxid >= MAX_DEMUX_UNITS || dmxid == 1) { /* Sanity check dmxid value, avoid demux 1 & going
                                                    outside array bounds in next check */
      trace_new(DPS_ERROR_MSG, "DEMUX:cnxt_dmx_empty_pes_buffer: demux %d does not exist!\n",dmxid);
      return DMX_BAD_DMXID;
    }

    /* First check to see that this demux id is already allocated */
    if (!gDemuxInfo[dmxid].DemuxInitialized) {
        trace_new(DPS_ERROR_MSG,"DEMUX:cnxt_dmx_read_pes_buffer: demux %d does not exist!\n",dmxid);
        return DMX_BAD_DMXID;
    }

    dmx = &gDemuxInfo[dmxid];

    if (chid >= TOTAL_CHANNELS || dmx->ChannelInfoTable[chid].InUseFlag == CHANNEL_FREE)
        return DMX_BAD_CHID;

    pChInfo = &dmx->ChannelInfoTable[chid];
    
    tidCaller = task_id();
    
    if(tidCaller != PES_TASK_ID)
      sem_get(GenDmxPESTaskProcSem, KAL_WAIT_FOREVER);

    /*
     * Determine the number of bytes we are discarding 
     */

    if(pChInfo->pReadPtr > write)
      size_to_discard = (pChInfo->pBufferEnd - pChInfo->pReadPtr) +
                        (write - pChInfo->pBuffer);
    else
      size_to_discard = write - pChInfo->pReadPtr;
    
    /*
     * Update pointers
     */

    AdvancePtr(&pChInfo->pReadPtr, size_to_discard, pChInfo->pBuffer, pChInfo->pBufferEnd);

    /*
     * Adjust the number of bytes left in the buffer
     */

    pChInfo->PESDataCount -= size_to_discard;

    if(tidCaller != PES_TASK_ID)
      sem_put(GenDmxPESTaskProcSem);

    return DMX_STATUS_OK;
}


/*****************************************************************************
 * cnxt_dmx_get_video_pid
 *
 * parameters:
 *    dmxid - demux hardware instance handle that was returned by _open()
 *    bHardware - if TRUE, read the PID set in the hardware
 *                if FALSE, read the PID assigned to the channel
 *    pid - the pid value to set in the pcr
 *
 * returns:
 *    DMX_STATUS_OK - no errors
 *    DMX_BAD_DMXID - bad demux instance handle
 *
 * description:
 *    The current setting of the video pid, as set in the
 *    channel. NOTE - This is not the current hardware
 *    setting but the PID that will be set when the channel
 *    is enabled. The video driver needs to know this so that
 *    it can determine if the PID changed even when video is
 *    stopped.
 ****************************************************************************/
DMX_STATUS cnxt_dmx_get_video_pid(u_int32 dmxid, bool bHardware, u_int16 *vidpid)
{
    DemuxInfo *dmx;

    trace_new(DPS_FUNC_TRACE,"DEMUX:cnxt_dmx_get_video_pid\n");

    if(dmxid >= MAX_DEMUX_UNITS || dmxid == 1) { /* Sanity check dmxid value, avoid demux 1 & going
                                                    outside array bounds in next check */
      trace_new(DPS_ERROR_MSG, "DEMUX:cnxt_dmx_get_video_pid: demux %d does not exist!\n",dmxid);
      return DMX_BAD_DMXID;
    }

    /* First check to see that this demux id is already allocated */
    if (!gDemuxInfo[dmxid].DemuxInitialized) {
        trace_new(DPS_ERROR_MSG,"DEMUX:cnxt_dmx_get_video_pid: demux %d does not exist!\n",dmxid);
        return DMX_BAD_DMXID;
    }

    dmx = &gDemuxInfo[dmxid];

    if (bHardware)
        *vidpid = *DPS_PID_BASE_EX(dmxid, VIDEO_CHANNEL) & 0x1fff;
    else
        *vidpid = dmx->ChannelInfoTable[VIDEO_CHANNEL].PID;

    return DMX_STATUS_OK;

}

/*****************************************************************************
 * cnxt_dmx_get_pes_frame_rate
 *
 * parameters:
 *    dmxid - demux hardware instance handle that was returned by _open()
 *    frame_rate - the PES frame rate as returned in real time from the pawser 
 *
 * returns:
 *    DMX_STATUS_OK - no errors
 *    DMX_BAD_DMXID - bad demux instance handle
 *
 * description:
 *    Returns the current PES video frame rate as read from the stream by 
 *    the microcode.                 
 *
 *    NOTE: The frame rate is only returned on Colorado >= Rev F 
 *          and other chips due to microcode space limitations.
 ****************************************************************************/
DMX_STATUS cnxt_dmx_get_pes_frame_rate(u_int32 dmxid, u_int16 *frame_rate)
{
    DemuxInfo *dmx;

    trace_new(DPS_FUNC_TRACE,"DEMUX:cnxt_dmx_get_pes_frame_rate\n");

    if(dmxid >= MAX_DEMUX_UNITS || dmxid == 1) { /* Sanity check dmxid value, avoid demux 1 & going
                                                    outside array bounds in next check */
      trace_new(DPS_ERROR_MSG, "DEMUX:cnxt_dmx_get_video_pid: demux %d does not exist!\n",dmxid);
      return DMX_BAD_DMXID;
    }

    /* First check to see that this demux id is already allocated */
    if (!gDemuxInfo[dmxid].DemuxInitialized) {
        trace_new(DPS_ERROR_MSG,"DEMUX:cnxt_dmx_get_pes_frame_rate: demux %d does not exist!\n",dmxid);
        return DMX_BAD_DMXID;
    }

    dmx = &gDemuxInfo[dmxid];

    /* Low order nibble is the PES frame rate */
    *frame_rate = *DPS_PES_FRAME_RATE_EX(dmxid) & 0xF; 
    return DMX_STATUS_OK;
}

/*****************************************************************************
 * cnxt_dmx_set_pts_offset
 *
 * parameters:
 *    dmxid   - demux hardware instance handle that was returned by _open()
 *    pts_video - Video PTS offset
 *    pts_audio - Audio PTS offset
 *
 * returns:
 *    DMX_STATUS_OK - no errors
 *    DMX_BAD_DMXID - bad demux instance handle
 *
 * description:
 *    Sets the PTS offsets for the audio and video streams. The pts offset is a 
 *    32 bit number that is added to the PTS to shift in +/- 6 hour intervals.
 *    We add the current timebase offset to the pts offset to get the final PTS 
 *    offset.       
 ****************************************************************************/
DMX_STATUS cnxt_dmx_set_pts_offset(u_int32 dmxid, u_int32 pts_video, u_int32 pts_audio)
{
    DemuxInfo *dmx;
    u_int32 video_low, audio_low, av_high = 0, temp;
    u_int64 work_offset;

    trace_new(DPS_FUNC_TRACE,"DEMUX:cnxt_dmx_set_pts_offset\n");

    if(dmxid >= MAX_DEMUX_UNITS || dmxid == 1) { /* Sanity check dmxid value, avoid demux 1 & going
                                                    outside array bounds in next check */
      trace_new(DPS_ERROR_MSG, "DEMUX:cnxt_dmx_set_pts_offset: demux %d does not exist!\n",dmxid);
      return DMX_BAD_DMXID;
    }

    /* First check to see that this demux id is already allocated */
    if (!gDemuxInfo[dmxid].DemuxInitialized) {
        trace_new(DPS_ERROR_MSG,"DEMUX:cnxt_dmx_set_pts_offset: demux %d does not exist!\n",dmxid);
        return DMX_BAD_DMXID;
    }

    dmx = &gDemuxInfo[dmxid];
   
    /* Get video pts data.  Sign extend bit 32 to get in 33 bit format */
    work_offset = (u_int64)pts_video + 
                ((u_int64)(pts_video & 0x80000000) << 1);

    /* Save in 33 bit format for later calculations */
    dmx->video_pts_offset = work_offset;
    
    work_offset += dmx->video_timebase_offset;
    av_high   = (u_int32)((work_offset>>32) & 0x1);  /* Video is in in bit 0 */
    video_low = (u_int32)(work_offset & (u_int64)0xFFFFFFFF);

    /* Get audio pts data.  Sign extend bit 32 to get in 33 bit format */
    work_offset = (u_int64)pts_audio + 
                ((u_int64)(pts_audio & 0x80000000) << 1);

    /* Save in 33 bit format for later calculations */
    dmx->audio_pts_offset = work_offset;

    work_offset += dmx->audio_timebase_offset;
    temp = (u_int32)((work_offset>>32) & 0x1);
    av_high  |= (temp << 1);  /* Audio is in in bit 1 */
    audio_low = (u_int32) (work_offset & (u_int64)0xFFFFFFFF);
                          
    *DPS_PTS_OFFSET_HI_EX(dmxid) = av_high;
    *DPS_PTS_OFFSET_VIDEO_EX(dmxid) = video_low;
    *DPS_PTS_OFFSET_AUDIO_EX(dmxid) = audio_low;

    return DMX_STATUS_OK;
}

/*****************************************************************************
 * cnxt_dmx_set_timebase_offset
 *
 * parameters:
 *    dmxid   - demux hardware instance handle that was returned by _open()
 *    timebase_video - Video timebase offset
 *    timebase_audio - Audio timebase offset
 *
 * returns:
 *    DMX_STATUS_OK - no errors
 *    DMX_BAD_DMXID - bad demux instance handle
 *
 * description:
 *    Sets the TIMEBASE offsets for the audo and video streams.  The timebase 
 *    is a 33 bit number that is added to the PTS to shift in +/- 12 hour intervals.
 *    We add the current pts offset to the timebase to get the final PTS offset.
 ****************************************************************************/
DMX_STATUS cnxt_dmx_set_timebase_offset(u_int32 dmxid, u_int64 timebase_video, u_int64 timebase_audio)
{
    DemuxInfo *dmx;
    u_int32 video_low, audio_low, av_high = 0, temp;
    u_int64 work_offset;

    isr_trace_new(DPS_FUNC_TRACE,"DEMUX:cnxt_dmx_set_timebase_offset\n",0,0);

    if(dmxid >= MAX_DEMUX_UNITS || dmxid == 1) { /* Sanity check dmxid value, avoid demux 1 & going
                                                    outside array bounds in next check */
      isr_trace_new(DPS_ERROR_MSG, "DEMUX:cnxt_dmx_set_timebase_offset: demux %d does not exist!\n",dmxid,0);
      return DMX_BAD_DMXID;
    }

    /* First check to see that this demux id is already allocated */
    if (!gDemuxInfo[dmxid].DemuxInitialized) {
        isr_trace_new(DPS_ERROR_MSG,"DEMUX:cnxt_dmx_set_timebase_offset: demux %d does not exist!\n",dmxid,0);
        return DMX_BAD_DMXID;
    }

    dmx = &gDemuxInfo[dmxid];
    dmx->video_timebase_offset = timebase_video;
    dmx->audio_timebase_offset = timebase_audio;
   
    /* Get video timebase data */
    work_offset = dmx->video_timebase_offset + dmx->video_pts_offset;
    av_high   = (u_int32)((work_offset>>32) & 0x1);  /* Video is in in bit 0 */
    video_low = (u_int32)(work_offset & (u_int64)0xFFFFFFFF);

    /* Get audio timebase data */
    work_offset = dmx->audio_timebase_offset + dmx->audio_pts_offset;
    temp = (u_int32)((work_offset>>32) & 0x1);
    av_high  |= (temp << 1);  /* Audio is in in bit 1 */
    audio_low = (u_int32)(work_offset & (u_int64)0xFFFFFFFF);

    *DPS_PTS_OFFSET_HI_EX(dmxid) = av_high;
    *DPS_PTS_OFFSET_VIDEO_EX(dmxid) = video_low;
    *DPS_PTS_OFFSET_AUDIO_EX(dmxid) = audio_low;

    return DMX_STATUS_OK;
}

/*****************************************************************************
 * cnxt_dmx_get_timebase_offset
 *
 * parameters:
 *    dmxid   - demux hardware instance handle that was returned by _open()
 *    timebase_video - Video timebase offset upon return
 *    timebase_audio - Audio timebase offset upon return
 *
 * returns:
 *    DMX_STATUS_OK - no errors
 *    DMX_BAD_DMXID - bad demux instance handle
 *
 * description:
 *    Gets the TIMEBASE offsets for the audo and video streams.  The timebase 
 *    is a 33 bit number that is added to the PTS to shift in +/- 12 hour intervals.
 *    We add the current pts offset to the timebase to get the final PTS offset.
 ****************************************************************************/
DMX_STATUS cnxt_dmx_get_timebase_offset(u_int32 dmxid, u_int64 *timebase_video, u_int64 *timebase_audio)
{
    DemuxInfo *dmx;

    isr_trace_new(DPS_FUNC_TRACE,"DEMUX:cnxt_dmx_get_timebase_offset\n",0,0);

    if(dmxid >= MAX_DEMUX_UNITS || dmxid == 1) { /* Sanity check dmxid value, avoid demux 1 & going
                                                    outside array bounds in next check */
      isr_trace_new(DPS_ERROR_MSG, "DEMUX:cnxt_dmx_get_timebase_offset: demux %d does not exist!\n",dmxid,0);
      return DMX_BAD_DMXID;
    }

    /* First check to see that this demux id is already allocated */
    if (!gDemuxInfo[dmxid].DemuxInitialized) {
        isr_trace_new(DPS_ERROR_MSG,"DEMUX:cnxt_dmx_get_timebase_offset: demux %d does not exist!\n",dmxid,0);
        return DMX_BAD_DMXID;
    }

    dmx = &gDemuxInfo[dmxid];
    *timebase_video = dmx->video_timebase_offset;
    *timebase_audio = dmx->audio_timebase_offset;
   
    return DMX_STATUS_OK;
}

#if PARSER_TYPE==DTV_PARSER
/*****************************************************************************
 * cnxt_dmx_set_CWP_attributes
 *
 * parameters:
 *    dmxid          - demux hardware instance handle that was returned by _open()
 *    cwp_notify     - CWP notification function                               
 *    TimeOut        - Time out in milliseconds (ms) for callbacks (optional)      
 *
 * returns:
 *    DMX_STATUS_OK - no errors
 *    DMX_BAD_DMXID - bad demux instance handle
 *
 * description:
 *    Sets attributes for CWP processing.  This includes setting the CWP notification 
 *    function and time_out (ms)
 ****************************************************************************/
DMX_STATUS cnxt_dmx_set_CWP_attributes(u_int32 dmxid, gen_callback_fct_t CWP_notify,
                                       u_int32 TimeOut)
{
    DemuxInfo *dmx;

    trace_new(DPS_FUNC_TRACE,"DEMUX:cnxt_dmx_set_CWP_attributes called\n");

    if(dmxid >= MAX_DEMUX_UNITS || dmxid == 1) { /* Sanity check dmxid value, avoid demux 1 & going
                                                    outside array bounds in next check */
      trace_new(DPS_ERROR_MSG, "DEMUX:cnxt_dmx_set_CWP_attributes: demux %d does not exist!\n",dmxid);
      return DMX_BAD_DMXID;
    }

    /* First check to see that this demux id is already allocated */
    if (!gDemuxInfo[dmxid].DemuxInitialized) {
        trace_new(DPS_ERROR_MSG,"DEMUX:cnxt_dmx_set_CWP_attributes: demux %d does not exist!\n",dmxid);
        return DMX_BAD_DMXID;
    }

    dmx = &gDemuxInfo[dmxid];

    dmx->CWPInfo.CWPNotify = (gen_callback_fct_t) CWP_notify;
    dmx->CWPInfo.CWPTimeOutMS = TimeOut;

    /*
     * Ensure the timer is stopped
     */
    if(dmx->CWPInfo.CWPTimerActive)
    {
        if(RC_OK != tick_stop (dmx->CWPInfo.CWPTimer))
        {
            trace_new(DPS_ERROR_MSG,"DEMUX:CWP tick_stop failed.\n");
        }
        dmx->CWPInfo.CWPTimerActive = FALSE;
    }

    /*
     * Set the timeout frequency, allocate timer if necessary
     */
    if(TimeOut)
    {
        if(!dmx->CWPInfo.CWPTimer)
        {
            /*
             * If there's a timeout and no timer allocated, allocate one
             */
            dmx->CWPInfo.CWPTimer = tick_create (
                  (PFNTIMERC) gen_demux_CWP_timer_call_back,
                  (void *)((u_int32)dmxid),
                  NULL);

            if(!dmx->CWPInfo.CWPTimer)
            {
                trace_new(DPS_ERROR_MSG,"DEMUX:CWP tick_create failed.\n");
            }
        }

        /*
         * Set timeout frequency
         */
        if(RC_OK != tick_set (dmx->CWPInfo.CWPTimer, dmx->CWPInfo.CWPTimeOutMS, FALSE))
        {
            trace_new(DPS_ERROR_MSG,"DEMUX:CWP tick_set failed.\n");
        }

        dmx->CWPInfo.CWPTimerActive = FALSE;
    }
    return DMX_STATUS_OK;
}

/*****************************************************************************
 * cnxt_dmx_set_CAP_attributes
 *
 * parameters:
 *    dmxid          - demux hardware instance handle that was returned by _open()
 *    cwp_notify     - CAP notification function                               
 *    TimeOut        - Time out in milliseconds (ms) for callbacks (optional)      
 *
 * returns:
 *    DMX_STATUS_OK - no errors
 *    DMX_BAD_DMXID - bad demux instance handle
 *
 * description:
 *    Sets attributes for CAP processing.  This includes setting the CAP notification 
 *    function and time_out (ms)
 ****************************************************************************/
DMX_STATUS cnxt_dmx_set_CAP_attributes(u_int32 dmxid, gen_callback_fct_t CAP_notify,
                                       u_int32 TimeOut)
{
    DemuxInfo *dmx;

    trace_new(DPS_FUNC_TRACE,"DEMUX:cnxt_dmx_set_CAP_attributes called\n");

    if(dmxid >= MAX_DEMUX_UNITS || dmxid == 1) { /* Sanity check dmxid value, avoid demux 1 & going
                                                    outside array bounds in next check */
      trace_new(DPS_ERROR_MSG, "DEMUX:cnxt_dmx_set_CAP_attributes: demux %d does not exist!\n",dmxid);
      return DMX_BAD_DMXID;
    }

    /* First check to see that this demux id is already allocated */
    if (!gDemuxInfo[dmxid].DemuxInitialized) {
        trace_new(DPS_ERROR_MSG,"DEMUX:cnxt_dmx_set_CAP_attributes: demux %d does not exist!\n",dmxid);
        return DMX_BAD_DMXID;
    }

    dmx = &gDemuxInfo[dmxid];

    dmx->CAPInfo.CAPNotify = (gen_callback_fct_t) CAP_notify;
    dmx->CAPInfo.CAPTimeOutMS = TimeOut;

    /*
     * Ensure the timer is stopped
     */
    if(dmx->CAPInfo.CAPTimerActive)
    {
        if(RC_OK != tick_stop (dmx->CAPInfo.CAPTimer))
        {
            trace_new(DPS_ERROR_MSG,"DEMUX:CAP tick_stop failed.\n");
        }
        dmx->CAPInfo.CAPTimerActive = FALSE;
    }

    /*
     * Set the timeout frequency, allocate timer if necessary
     */
    if(TimeOut)
    {
        if(!dmx->CAPInfo.CAPTimer)
        {
            /*
             * If there's a timeout and no timer allocated, allocate one
             */
            dmx->CAPInfo.CAPTimer = tick_create (
                  (PFNTIMERC) gen_demux_CAP_timer_call_back,
                  (void *)((u_int32)dmxid),
                  NULL);

            if(!dmx->CAPInfo.CAPTimer)
            {
                trace_new(DPS_ERROR_MSG,"DEMUX:CAP tick_create failed.\n");
            }
        }

        /*
         * Set timeout frequency
         */
        if(RC_OK != tick_set (dmx->CAPInfo.CAPTimer, dmx->CAPInfo.CAPTimeOutMS, FALSE))
        {
            trace_new(DPS_ERROR_MSG,"DEMUX:CAP tick_set failed.\n");
        }

        dmx->CAPInfo.CAPTimerActive = FALSE;
    }
    return DMX_STATUS_OK;
}

/*****************************************************************************
 * cnxt_dmx_set_CAP_scid
 *
 * parameters:
 *    dmxid          - demux hardware instance handle that was returned by _open()
 *    scid           - scid to receive CAP packets                                 
 *
 * returns:
 *    DMX_STATUS_OK - no errors
 *    DMX_BAD_DMXID - bad demux instance handle
 *    DMX_BAD_CTL - bad control code
 *
 * description:
 *    Defines the scid to receive CAP packets on.  CAP packets will not arrive until  
 *    CAPs are enabled with the cnxt_dmx_CAP_control function.
 ****************************************************************************/
DMX_STATUS cnxt_dmx_CAP_set_scid(u_int32 dmxid, u_int16 scid) {

    DemuxInfo *dmx;

    trace_new(DPS_FUNC_TRACE,"DEMUX:cnxt_dmx_CAP_set_scid called\n");

    if(dmxid >= MAX_DEMUX_UNITS || dmxid == 1) { /* Sanity check dmxid value, avoid demux 1 & going
                                                    outside array bounds in next check */
      trace_new(DPS_ERROR_MSG, "DEMUX:cnxt_dmx_CAP_set_scid: demux %d does not exist!\n",dmxid);
      return DMX_BAD_DMXID;
    }

    /* First check to see that this demux id is already allocated */
    if (!gDemuxInfo[dmxid].DemuxInitialized) {
        trace_new(DPS_ERROR_MSG,"DEMUX:cnxt_dmx_CAP_set_scid: demux %d does not exist!\n",dmxid);
        return DMX_BAD_DMXID;
    }

    dmx = &gDemuxInfo[dmxid];
    dmx->CAPInfo.CAP_scid = scid;
    return DMX_STATUS_OK;
}
/*****************************************************************************
 * cnxt_dmx_set_CAP_filter
 *
 * parameters:
 *    dmxid          - demux hardware instance handle that was returned by _open()
 *    filter         - filter to be used when receiving CAP packets                                 
 *
 * returns:
 *    DMX_STATUS_OK - no errors
 *    DMX_BAD_DMXID - bad demux instance handle
 *    DMX_BAD_CTL - bad control code
 *
 * description:
 *    Defines the filter for CAP packets.
 ****************************************************************************/
DMX_STATUS cnxt_dmx_CAP_set_filter(u_int32 dmxid, u_int32 filter) {

    DemuxInfo *dmx;

    trace_new(DPS_FUNC_TRACE,"DEMUX:cnxt_dmx_CAP_set_filter called\n");

    if(dmxid >= MAX_DEMUX_UNITS || dmxid == 1) { /* Sanity check dmxid value, avoid demux 1 & going
                                                    outside array bounds in next check */
      trace_new(DPS_ERROR_MSG, "DEMUX:cnxt_dmx_CAP_set_filter: demux %d does not exist!\n",dmxid);
      return DMX_BAD_DMXID;
    }

    /* First check to see that this demux id is already allocated */
    if (!gDemuxInfo[dmxid].DemuxInitialized) {
        trace_new(DPS_ERROR_MSG,"DEMUX:cnxt_dmx_CAP_set_filter: demux %d does not exist!\n",dmxid);
        return DMX_BAD_DMXID;
    }

    dmx = &gDemuxInfo[dmxid];
    dmx->CAPInfo.CAP_filter = filter;

    /* Set the filter for the microcode to use */
    *DPS_CAP_FILTER_EX(dmxid) = filter;
    return DMX_STATUS_OK;
}

/*****************************************************************************
 * cnxt_dmx_set_CAP_control
 *
 * parameters:
 *    dmxid           - demux hardware instance handle that was returned by _open()
 *    channel_command - channel control code
 *          GEN_DEMUX_ENABLE
 *          GEN_DEMUX_DISABLE
 *
 * returns:
 *    DMX_STATUS_OK - no errors
 *    DMX_BAD_DMXID - bad demux instance handle
 *
 * description:
 *    Enables/Disables the demux to receive CAP packets. 
 ****************************************************************************/
DMX_STATUS cnxt_dmx_CAP_control(u_int32 dmxid, gencontrol_channel_t channel_command) {

    DemuxInfo *dmx;

    trace_new(DPS_FUNC_TRACE,"DEMUX:cnxt_dmx_CAP_control called\n");

    if(dmxid >= MAX_DEMUX_UNITS || dmxid == 1) { /* Sanity check dmxid value, avoid demux 1 & going
                                                    outside array bounds in next check */
      trace_new(DPS_ERROR_MSG, "DEMUX:cnxt_dmx_CAP_control: demux %d does not exist!\n",dmxid);
      return DMX_BAD_DMXID;
    }

    /* First check to see that this demux id is already allocated */
    if (!gDemuxInfo[dmxid].DemuxInitialized) {
        trace_new(DPS_ERROR_MSG,"DEMUX:cnxt_dmx_CAP_control: demux %d does not exist!\n",dmxid);
        return DMX_BAD_DMXID;
    }

    dmx = &gDemuxInfo[dmxid];

    switch (channel_command) {
    case GEN_DEMUX_DISABLE:

        /*
         * Disable CAP's
         */

        dmx->CAPInfo.CAPEnable = GEN_DEMUX_DISABLE;

        isr_trace_new( DPS_FUNC_TRACE,"cnxt_dmx_CAP_control(DISABLE)\n",0,0);
        /*
         * If there's a timer, ensure it's stopped
         */
        if (dmx->CAPInfo.CAPTimerActive)
        {
            if(RC_OK != tick_stop(dmx->CAPInfo.CAPTimer))
            {
                isr_trace_new(DPS_ERROR_MSG,"DEMUX:tick_stop failed.\n",0,0);
            }
            dmx->CAPInfo.CAPTimerActive = FALSE;
            dmx->CAPInfo.CAPTimerNotifyCount = 0;
        }

        /* Deactivate CAPs in the hardware */
        *DPS_CAP_SCID_EX(dmxid) = 0xFFF; /* 0xFFF is an invalid SCID */
        break;

    case GEN_DEMUX_ENABLE:

        isr_trace_new( DPS_FUNC_TRACE,"cnxt_dmx_CAP_control(ENABLE)\n",0,0);

        /* Activate CAPs in the hardware */
        *DPS_CAP_SCID_EX(dmxid) = dmx->CAPInfo.CAP_scid;

        /*
         * If there's a timeout, start the timer
         */
        if ((dmx->CAPInfo.CAPTimeOutMS) &&
            (dmx->CAPInfo.CAPTimer))
        {
            if (RC_OK != tick_start(dmx->CAPInfo.CAPTimer))
            {
                isr_trace_new(DPS_ERROR_MSG,"DEMUX:tick_start failed.\n",0,0);
            }
            dmx->CAPInfo.CAPTimerActive = TRUE;
        }
        else
        {
            /*
             * If there's no timeout, but there's a timer, ensure it's stopped
             */
            if (dmx->CAPInfo.CAPTimerActive)
            {
                if (RC_OK != tick_stop(dmx->CAPInfo.CAPTimer))
                {
                    isr_trace_new(DPS_ERROR_MSG,"DEMUX:tick_stop failed.\n",0,0);
                }
                dmx->CAPInfo.CAPTimerActive = FALSE;
                dmx->CAPInfo.CAPTimerNotifyCount = 0;
            }
        }

        /*
         * Enable channel
         */

        dmx->CAPInfo.CAPEnable = GEN_DEMUX_ENABLE;
        break;
    default:
        return DMX_BAD_CTL;
        break;
    }

    return DMX_STATUS_OK;
}
#endif

#if PARSER_PASSAGE_ENABLE==YES
/*****************************************************************************
 * cnxt_dmx_set_passage_mode
 *
 * parameters:
 *    dmxid - demux hardware instance handle that was returned by _open()
 *    mode  - passage mode to be set
 *
 * returns:
 *    DMX_STATUS_OK - no errors
 *    DMX_ERROR - the specified instance handle is not valid
 *          (perhaps not opened or already closed)
 *
 * description:
 *    Set the passage mode.
 ****************************************************************************/
DMX_STATUS cnxt_dmx_set_passage_mode ( u_int32 dmxid, u_int32 mode )
{
   u_int32 real_mode;

   trace_new ( DPS_FUNC_TRACE,"DEMUX:cnxt_dmx_set_passage_mode entered\n" );

   /* Sanity check dmxid value, avoid demux 1 & going
      outside array bounds in next check */
   if ( dmxid >= MAX_DEMUX_UNITS || dmxid == 1 )
   {
      trace_new ( DPS_ERROR_MSG, "DEMUX:cnxt_dmx_set_passage_mode: demux %d does not exist!\n", dmxid );
      return DMX_BAD_DMXID;
   }

   /* First check to see that this demux id is already allocated */
   if ( !gDemuxInfo[dmxid].DemuxInitialized )
   {
      trace_new ( DPS_ERROR_MSG,"cnxt_dmx_set_passage_mode: Attempt to access non-existent demux!\n" );
      return DMX_ERROR;
   }

   real_mode = mode & 0x7;
   if ( ( real_mode != 0 ) && ( real_mode != 1 ) && ( real_mode != 2 ) && ( real_mode != 4 ) )
   {
      trace_new ( DPS_ERROR_MSG,"cnxt_dmx_set_passage_mode: Invalid passage mode!\n" );
      return DMX_ERROR;
   }

   *DPS_PASSAGE_MODE ( dmxid ) = ( mode & 0xf );

   return DMX_STATUS_OK;
}

/*****************************************************************************
 * cnxt_dmx_set_shadow_pid
 *
 * parameters:
 *    dmxid - demux hardware instance handle that was returned by _open()
 *    vid_pid - video shadow pid
 *    aud_pid - audio shadow pid
 *
 * returns:
 *    DMX_STATUS_OK - no errors
 *    DMX_ERROR - the specified instance handle is not valid
 *          (perhaps not opened or already closed)
 *
 * description:
 *    Set the shadow PIDs for sony passage stream.
 ****************************************************************************/
DMX_STATUS cnxt_dmx_set_shadow_pid ( u_int32 dmxid, u_int16 vid_pid, u_int16 aud_pid )
{
   trace_new ( DPS_FUNC_TRACE,"DEMUX:cnxt_dmx_set_shadow_pid entered\n" );

   /* Sanity check dmxid value, avoid demux 1 & going
      outside array bounds in next check */
   if ( dmxid >= MAX_DEMUX_UNITS || dmxid == 1 )
   {
      trace_new ( DPS_ERROR_MSG, "DEMUX:cnxt_dmx_set_shadow_pid: demux %d does not exist!\n", dmxid );
      return DMX_BAD_DMXID;
   }

   /* First check to see that this demux id is already allocated */
   if ( !gDemuxInfo[dmxid].DemuxInitialized )
   {
      trace_new ( DPS_ERROR_MSG,"cnxt_dmx_set_shadow_pid: Attempt to access non-existent demux!\n" );
      return DMX_ERROR;
   }

   *DPS_PASSAGE_SHADOW_VIDEO_PID ( dmxid ) = (u_int32)vid_pid;
   *DPS_PASSAGE_SHADOW_AUDIO_PID ( dmxid ) = (u_int32)aud_pid;

   return DMX_STATUS_OK;
}
#endif

int dare_dmx_Get_AVChannel_Status(u_int32 dmxid,genchannel_t channel_type)
{
    DemuxInfo *dmx;

    /*
     * Sanity check dmxid value, avoid demux 1 & going
     * outside array bounds in next check
     */
    if(dmxid >= MAX_DEMUX_UNITS || dmxid == 1)
    {
      trace_new(DPS_ERROR_MSG, "DEMUX:cnxt_dmx_channel_open: demux %d does not exist!\n", dmxid);
      return -1;
    }

    /*
     * First check to see that this demux id is already allocated
     */
    if (!gDemuxInfo[dmxid].DemuxInitialized)
    {
        trace_new(DPS_ERROR_MSG,
            "DEMUX:cnxt_dmx_channel_open: demux %d is not initialized!\n", dmxid);
        return -1;
    }

    dmx = &gDemuxInfo[dmxid];

    switch (channel_type)
    {
        /*
         * Special case for live video channel (chid = 1)
         */
        case VIDEO_PES_TYPE:

            return dmx->ChannelInfoTable[VIDEO_CHANNEL].InUseFlag;
        break;

        /*
         * Special case for live audio channel (chid = 2)
         */
        case AUDIO_PES_TYPE:
           return dmx->ChannelInfoTable[AUDIO_CHANNEL].InUseFlag;

       default:
            trace_new(DPS_ERROR_MSG,"DEMUX:cnxt_dmx_channel_open failed - invalid channel type\n");
            return DMX_CH_UNAVAILABLE;
    }

    return DMX_STATUS_OK;
}
/****************************************************************************
 * Modifications:
 * $Log: 
 *  71   mpeg      1.70        5/24/04 11:58:00 AM    Larry Wang      CR(s) 
 *        9289 9290 : Don't allow setting same PID for different opened demux 
 *        channels even if they are disabled.
 *  70   mpeg      1.69        3/17/04 9:09:50 AM     Dave Wilson     CR(s) 
 *        8579 : Reworked cnxt_dmx_channel_control so that any RESET option 
 *        (RESET or DISABLE_RESET) called for an audio or video channel will 
 *        cause the channel to be flushed. Also reduced a delay in this code 
 *        from 150mS to the minimum possible. The code was supposed to wait 1 
 *        transport packet time so 150mS was eons too long.
 *  69   mpeg      1.68        3/16/04 3:35:02 PM     Tim White       CR(s) 
 *        8545 : Moved cnxt_dmx_set_input_device() to demuxdma.  Added function
 *        cnxt_dmx_get_timebase_offset() to demuxapi.  Fixed some trace in
 *        functions which can be called under ISR to use isr_trace_new().
 *        
 *  68   mpeg      1.67        3/12/04 11:55:09 AM    Tim Ross        CR(s) 
 *        8545 : Moved check of buffer ptr from channel_set_pid() to 
 *        channel_control().
 *        Added a similar check for the transport buffer ptrs to 
 *        channel_control()
 *        for record ucode.
 *        Changed channel_set_pid() PES & PSI buffer setup to be conditionally 
 *        based on
 *        the absence of record ucode.
 *        Added a call to channel_control() to set the pids into the hardware 
 *        if channel
 *        is already enabled for recording.
 *  67   mpeg      1.66        3/10/04 2:48:33 PM     Larry Wang      CR(s) 
 *        8551 : Add demux APIs to set Sony Passage mode and shadow PIDs.
 *  66   mpeg      1.65        3/10/04 10:44:26 AM    Bob Van Gulick  CR(s) 
 *        8546 : Add support to return PES frame rate
 *        
 *  65   mpeg      1.64        3/3/04 11:47:48 AM     Tim Ross        CR(s) 
 *        8451 : Made PVR record extension changes conditional on PVR==YES to 
 *        prevent
 *        build errors with COlorado chips.
 *  64   mpeg      1.63        3/2/04 10:58:37 AM     Tim Ross        CR(s) 
 *        8451 : Added PVR record extension changes to 
 *        cnxt_dmx_channel_control().
 *  63   mpeg      1.62        2/24/04 2:45:05 PM     Bob Van Gulick  CR(s) 
 *        8427 : Add cnxt_dmx_set_timebase_offset function to offset PTS by +/-
 *         12 hours
 *        
 *  62   mpeg      1.61        1/26/04 2:39:23 PM     Larry Wang      CR(s) 
 *        8271 : Enable CRC status register after parser microcode restarted in
 *         cnxt_dmx_set_input_device().
 *  61   mpeg      1.60        1/15/04 3:28:17 PM     Bob Van Gulick  CR(s) 
 *        8224 : 
 *        Do not reload PCR if it has not changed from the current value
 *        
 *  60   mpeg      1.59        11/25/03 3:52:21 PM    Tim White       CR(s): 
 *        8027 Function cnxt_dmx_set_input_device() not setting correct bits in
 *         PARSER_CTL_REG in
 *        order to activate DMA input.
 *        
 *  59   mpeg      1.58        11/19/03 10:09:57 AM   Tim White       CR(s): 
 *        7987 Added Demux DMA and Demux PVR extension support phase 1.
 *        
 *  58   mpeg      1.57        9/22/03 4:52:46 PM     Bob Van Gulick  SCR(s) 
 *        7519 :
 *        Add support for DirecTV CAPs
 *        
 *        
 *  57   mpeg      1.56        9/16/03 4:04:50 PM     Tim White       SCR(s) 
 *        7474 :
 *        Add sanity check for capabilities.  Select correct demux based on 
 *        capabilities.
 *        
 *        
 *  56   mpeg      1.55        9/2/03 7:03:04 PM      Joe Kroesche    SCR(s) 
 *        7415 :
 *        removed unneeded header files to eliminate extra warnings when 
 *        building
 *        for PSOS
 *        
 *  55   mpeg      1.54        8/27/03 11:01:20 AM    Bob Van Gulick  SCR(s) 
 *        7387 :
 *        Add support for CWP processing in DirecTV 
 *        
 *        
 *  54   mpeg      1.53        8/13/03 1:41:04 PM     Larry Wang      SCR(s) 
 *        7179 7180 :
 *        Fix the data abort problem caused by an un-initialized pointer in 
 *        cnxt_dmx_set_ext_filter().
 *        
 *  53   mpeg      1.52        8/6/03 6:58:52 PM      Larry Wang      SCR(s) 
 *        7161 7162 :
 *        In the case where there are more than one audio channel, PSI channel 
 *        open has to skip all audio channel.
 *        
 *  52   mpeg      1.51        7/16/03 10:56:26 AM    Mark Thissen    SCR(s) 
 *        6983 :
 *        Changed logic statement in cnxt_dmx_set_input_device() which allows 
 *        demux TS input to be switched from DMA back to NIM
 *        
 *  51   mpeg      1.50        6/20/03 11:29:12 AM    Bob Van Gulick  SCR(s) 
 *        6812 :
 *        Remove ability of A/V slots to be used as PSI desram channels. 
 *        
 *        
 *  50   mpeg      1.49        6/9/03 5:58:50 PM      Bob Van Gulick  SCR(s) 
 *        6755 :
 *        Add support for 8 slot descram in demux.  Also change use of 
 *        DESC_CHANNELS to DPS_NUM_DESCRAMBLERS.
 *        
 *        
 *  49   mpeg      1.48        6/3/03 10:28:48 AM     Larry Wang      SCR(s) 
 *        6667 :
 *        When secondary audio channel buffer is defined, allow applications to
 *         open the second audio channel with channel_type=AUDIO_PES_TYPE.
 *        
 *  48   mpeg      1.47        5/2/03 11:12:12 AM     Bob Van Gulick  SCR(s) 
 *        6151 :
 *        Add functions to support pts_offset and input device settings
 *        
 *        
 *  47   mpeg      1.46        4/24/03 5:52:26 PM     Tim White       SCR(s) 
 *        6097 :
 *        Allow 6 descrambled simultaneous PES channels.  Remove #ifndef 
 *        USE_OLD_PES code.
 *        
 *        
 *  46   mpeg      1.45        4/16/03 4:49:14 PM     Larry Wang      SCR(s) 
 *        6045 :
 *        If FILTER_ON_LENGTH==YES, do not skip length field when setting up 
 *        section filters.
 *        
 *  45   mpeg      1.44        4/15/03 3:50:32 PM     Dave Wilson     SCR(s) 
 *        6025 :
 *        
 *        
 *        
 *        
 *        
 *        
 *        Added functions cnxt_dmx_register_event6_notify and 
 *        cnxt_dmx_query_event6_notify.
 *        These allow a client to hook the event 6 interrupt. This is used to 
 *        signal
 *        various events depending upon the version of demux microcode in use.
 *        
 *  44   mpeg      1.43        4/15/03 11:47:56 AM    Dave Wilson     SCR(s) 
 *        6021 :
 *        Added cnxt_dmx_query_pcr_notify function. This allows a client to 
 *        retrieve
 *        the current function pointer hooked to receive PCR notifications, 
 *        allowing
 *        the new client to replace the function and insert itself into the 
 *        notification
 *        chain.
 *        
 *  43   mpeg      1.42        4/11/03 6:36:24 PM     Brendan Donahe  SCR(s) 
 *        5985 :
 *        Fixed type on query_status - was checking for != DEMUX_FREE to error,
 *         not ==.
 *        
 *        
 *  42   mpeg      1.41        4/11/03 4:04:40 PM     Brendan Donahe  SCR(s) 
 *        6010 :
 *        Added cnxt_dmx_query_status() to allow tasks to query whether pawser 
 *        memory
 *        should be available (in other words, it's initialized and not in 
 *        reset).  
 *        Without checking this, tasks may cause data aborts by accessing 
 *        pawser memory.
 *        
 *        
 *  41   mpeg      1.40        4/10/03 5:02:30 PM     Dave Wilson     SCR(s) 
 *        5990 :
 *        
 *        
 *        
 *        Added cnxt_dmx_set_section_channel_tag API.d
 *        
 *        
 *  40   mpeg      1.39        4/4/03 2:45:20 PM      Brendan Donahe  SCR(s) 
 *        5966 :
 *        Changed cnxt_dmx_connect_notify to return DMX_ERROR when an unknown 
 *        event_type
 *        is passed in (argument 2).
 *        
 *        
 *  39   mpeg      1.38        4/4/03 9:42:38 AM      Bob Van Gulick  SCR(s) 
 *        5959 :
 *        Change DPS_MSG messages to DPS_FUNC_TRACE for several trace_new's
 *        
 *        
 *  38   mpeg      1.37        4/3/03 3:13:56 PM      Brendan Donahe  SCR(s) 
 *        5955 :
 *        Fixed temporary workaround, should &= in ~(1<<fid), not = ~(1<<fid).
 *        
 *        
 *  37   mpeg      1.36        4/2/03 11:58:24 AM     Brendan Donahe  SCR(s) 
 *        5886 :
 *        Modifications to support 6 simultaneous unscrambled/scrambled SI 
 *        including
 *        channels usually used for video and audio (1 & 2) as well as 
 *        enforcing correct
 *        parser microcode version which has been changed in conjunction with 
 *        this 
 *        feature enhancement.
 *        
 *        
 *  36   mpeg      1.35        3/25/03 11:44:58 AM    Brendan Donahe  SCR(s) 
 *        5845 5846 :
 *        Added version filtering support to Brazos, requiring configuration of
 *         4
 *        relocated version mode registers (positive/negative filtering) 
 *        supporting 3
 *        banks - only filters 0-31 are supported now.  Fixed paren syntax 
 *        issues.
 *        
 *        
 *  35   mpeg      1.34        3/24/03 11:02:42 AM    Larry Wang      SCR(s) 
 *        5853 :
 *        Don't skip length field in directv APG filter because clients will 
 *        not have it.
 *        
 *  34   mpeg      1.33        3/24/03 10:51:44 AM    Dave Wilson     SCR(s) 
 *        5443 :
 *        Added task context checking to PES-related APIs to allow them to be 
 *        called
 *        in the context of the PES task (OpenTV's DVB subtitling extension 
 *        does this).
 *        
 *        
 *  33   mpeg      1.32        3/19/03 6:10:42 PM     Brendan Donahe  SCR(s) 
 *        5776 5794 :
 *        Added check to ensure FID as passed into filter functions is checked 
 *        and
 *        function now returns DMX_BAD_FID if outside bounds before used as 
 *        array index.
 *        Added check for filter_control enumerated command - function now 
 *        returns 
 *        DMX_BAD_CTL upon bad command being passed in.
 *        
 *        
 *  32   mpeg      1.31        3/19/03 1:11:50 PM     Brendan Donahe  SCR(s) 
 *        5776 5794 :
 *        Added (per spec) the ability of cnxt_dmx_channel_control to return 
 *        DMX_BAD_CTL
 *        if incorrect control command is passed in.
 *        
 *        
 *  31   mpeg      1.30        3/18/03 5:31:42 PM     Brendan Donahe  SCR(s) 
 *        5807 5808 :
 *        Repaired PES/ES/Video/Audio channel availability counters in 
 *        channel_open
 *        and channel_close.
 *        
 *        
 *  30   mpeg      1.29        3/18/03 1:41:02 PM     Brendan Donahe  SCR(s) 
 *        5776 5794 :
 *        Repaired remainder of functions needing dmxid value check before use 
 *        as 
 *        array index.
 *        
 *        
 *  29   mpeg      1.28        3/17/03 6:18:20 PM     Brendan Donahe  SCR(s) 
 *        5776 5794 :
 *        In channel_close, moved use of dmxid in array until after bounds 
 *        checked.
 *        Added check to return error (as per API spec) upon closure of 
 *        unopened channel.
 *        
 *        
 *  28   mpeg      1.27        3/17/03 5:13:50 PM     Brendan Donahe  SCR(s) 
 *        5776 5782 5794 5797 :
 *        Added checks to cnxt_dmx_channel_open for input parameters and to 
 *        ensure
 *        that multiple calls to allocate a video channel, for instance, would 
 *        return
 *        CH_UNAVAILABLE.
 *        
 *        
 *  27   mpeg      1.26        3/17/03 3:14:14 PM     Dave Wilson     SCR(s) 
 *        5786 5785 :
 *        Reworked cnxt_dmx_set_section_channel_attributes to ensure that it 
 *        can 
 *        safely be called from the section notify callback. The function now 
 *        only tries
 *        to acquire the PSI semaphore if the calling context is not that of 
 *        the PSI
 *        task (which holds the semaphore during callbacks).
 *        
 *  26   mpeg      1.25        3/17/03 2:48:44 PM     Dave Wilson     SCR(s) 
 *        5784 5783 :
 *        Oops - realised after the last drop that I had not completely closed 
 *        the 
 *        loophole. The last version would still leak memory if calls to 
 *        cnxt_dmx_set_section_channel_attributes were made some with 
 *        MinHdrSize set to
 *        0 and others with non-zero values. This has now been corrected.
 *        
 *  25   mpeg      1.24        3/17/03 2:44:08 PM     Dave Wilson     SCR(s) 
 *        5784 5783 :
 *        Added code to cnxt_dmx_set_section_channel_attributes to free up any 
 *        existing
 *        HdrAlloc memory before allocating it again. This cures what would 
 *        have been a
 *        small memory leak had this function been called multiple times for 
 *        the same
 *        channel.
 *        
 *  24   mpeg      1.23        3/17/03 2:08:20 PM     Brendan Donahe  SCR(s) 
 *        5776 :
 *        Added dmxid bounds checks to channel_open, close, & filters_available
 *        functions to avoid crashes.
 *        
 *        
 *  23   mpeg      1.22        3/14/03 12:57:28 PM    Larry Wang      SCR(s) 
 *        5772 :
 *        Set CallbackFlushedChannel to be TRUE if cnxt_dmx_channel_close is 
 *        called in PSI task.
 *        
 *  22   mpeg      1.21        3/5/03 5:19:42 PM      Dave Wilson     SCR(s) 
 *        5667 5668 :
 *        Added code to cnxt_dmx_channel_control to determine if the call is 
 *        being
 *        made in the context of the PSI task. This will happen if a client 
 *        calls this
 *        function from its header or section notify callbacks. If the function
 *         is called
 *        in this context, we no longer try to get the PSI semaphore (it is 
 *        already
 *        held by the PSI task so getting it again causes deadlock) and we also
 *         set a 
 *        flag in the channel information structure (CallbackFlushedChannel) if
 *         the
 *        call to cnxt_dmx_channel_control affects the channel read/write/ack 
 *        pointers
 *        In these cases, the PSI task will not try to fix up the channel 
 *        buffer
 *        pointers itself after the notification callback returns.
 *        
 *  21   mpeg      1.20        2/26/03 1:19:24 PM     Larry Wang      SCR(s) 
 *        5593 5592 :
 *        Set Filter Mode Bytes to zero if NotMaskNonZero is FALSE.
 *        
 *  20   mpeg      1.19        1/31/03 12:11:38 PM    Dave Wilson     SCR(s) 
 *        5350 :
 *        Added new API cnxt_dmx_empty_pes_buffer to allow a caller to discard 
 *        part
 *        or all of the unread data in a PES channel ring buffer. This is used 
 *        by the 
 *        OpenTV demux driver since their model of PES buffer management 
 *        assumes that the
 *        hardware will just keep writing data and not stop if the buffer 
 *        overflows. This
 *        function is called by demux_get_pes_channel_write_pointer to ensure 
 *        that the
 *        buffer is always consumed as far as our hardware is concerned.
 *        
 *  19   mpeg      1.18        12/17/02 4:59:10 PM    Dave Wilson     SCR(s) 
 *        5185 :
 *        Removed some redundant code which was specific to our old IKOS 
 *        emulator.
 *        
 *  18   mpeg      1.17        11/20/02 3:57:18 PM    Bob Van Gulick  SCR(s) 
 *        4998 :
 *        Add register_pcr_notify_callback function to demux api.
 *        
 *        
 *  17   mpeg      1.16        10/24/02 2:52:44 PM    Bob Van Gulick  SCR(s) 
 *        4829 :
 *        Changes to fix Canal+ filtering. Do not or mode bits into mask and 
 *        only set negative mode table bit if filter has masked on mode bits.  
 *        Also general cleanup of filtering code.
 *        
 *        
 *  16   mpeg      1.15        10/16/02 3:20:38 PM    Bob Van Gulick  SCR(s) 
 *        4799 :
 *        Remove CANAL_PLUS_FILTERING #ifdefs and use #if PARSER_FILTERING == 
 *        FILTER_xxx
 *        instead.  PARSER_FILTERING is defined in the sw config
 *        
 *        
 *  15   mpeg      1.14        10/9/02 12:01:44 PM    Bob Van Gulick  SCR(s) 
 *        4762 :
 *        Fix ifndef to ifdef for Canal Plus filtering.
 *        
 *        
 *  14   mpeg      1.13        10/2/02 2:51:18 PM     Bob Van Gulick  SCR(s) 
 *        4636 :
 *        Only set info change bit while setting a filter if the filter had 
 *        already been allocated and set.  
 *        
 *        
 *  13   mpeg      1.12        9/19/02 3:42:38 PM     Joe Kroesche    SCR(s) 
 *        4610 :
 *        added crc notification feature, removed changes for previous crc 
 *        notification
 *        method. NOTE!!! requires matching pawser ucode update of #4626
 *        
 *  12   mpeg      1.11        9/9/02 11:02:06 AM     Bob Van Gulick  SCR(s) 
 *        4557 :
 *        Change ChannelTimer init to a 0 from NULL pointer to remove warning
 *        
 *        
 *  11   mpeg      1.10        9/5/02 6:30:02 PM      Bob Van Gulick  SCR(s) 
 *        4530 :
 *        Change CRC check to use Header Notify instead of Section Notify
 *        
 *        
 *  10   mpeg      1.9         8/30/02 3:05:52 PM     Bob Van Gulick  SCR(s) 
 *        4485 :
 *        Fix minor problem with previous drop
 *        
 *        
 *  9    mpeg      1.8         8/30/02 2:43:28 PM     Bob Van Gulick  SCR(s) 
 *        4485 :
 *        Add support for CRC checking of SI packets
 *        
 *        
 *  8    mpeg      1.7         8/29/02 4:36:40 PM     Brendan Donahe  SCR(s) 
 *        4491 :
 *        Moved code to set gDemuxInitialized outside of NDS define
 *        
 *        
 *  7    mpeg      1.6         8/5/02 11:55:08 AM     Tim White       SCR(s) 
 *        4330 :
 *        Fixed timeout and single shot (ONE_SHOT) capabilities.
 *        
 *        
 *  6    mpeg      1.5         6/27/02 5:57:16 PM     Tim White       SCR(s) 
 *        4108 :
 *        Convert MHP glue layer to use new DEMUX driver.
 *        
 *        
 *  5    mpeg      1.4         6/27/02 5:32:18 PM     Tim White       SCR(s) 
 *        4106 :
 *        Set or clear only the 1 bit which pertains to the filter being setup 
 *        for
 *        whether to use negative mode filtering or not.
 *        
 *        
 *  4    mpeg      1.3         5/13/02 12:10:58 PM    Tim White       SCR(s) 
 *        3760 :
 *        Renamed DPS_ HSDP definitions to be HSDP_ and only use when the HSDP 
 *        driver is not
 *        linked with the application.
 *        
 *        
 *  3    mpeg      1.2         2/12/02 1:27:46 PM     Bob Van Gulick  SCR(s) 
 *        3178 :
 *        Remove check for PES_CHANNEL in cnxt_dmx_set_channel_attributes - was
 *         causing teletext to fail.
 *        
 *        
 *  2    mpeg      1.1         2/7/02 11:52:36 AM     Bob Van Gulick  SCR(s) 
 *        3143 :
 *        remove dead code
 *        
 *        
 *  1    mpeg      1.0         12/18/01 2:55:58 PM    Bob Van Gulick  
 * $
 ****************************************************************************/


