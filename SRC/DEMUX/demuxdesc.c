/************************************************************************
 *                                                                      *
 * Sabine Project                                                       *
 *                                                                      *
 * Copyright (C) Rockwell Corporation 1998-1999.   All rights reserved. *
 *                                                                      *
 * Graphics BitBlt Functions for OpenTV OSD Driver                      *
 *                                                                      *
 ************************************************************************/
/************************************************************************
 * $Header: demuxdesc.c, 19, 11/14/03 5:32:09 PM, $
 ************************************************************************/

#include "stbcfg.h"
#include "basetype.h"
#include "kal.h"
#include "retcodes.h"
#include "demuxapi.h"
#include "demuxint.h"

/******************************************************************************
 * Define LOCAL macros for accessing ODD/EVEN KEY Enable Register(s)
 * Pre-Brazos has a single, bit-mapped register for both ODD and EVEN
 * Brazos has two separate registers ... One for ODD, One for EVEN
 */

#ifdef DPS_EVEN_KEY_ENABLE_REG_EX
#define LOCAL_DPS_EVEN_KEY_ENABLE_REG_EX     DPS_EVEN_KEY_ENABLE_REG_EX
#else
#define LOCAL_DPS_EVEN_KEY_ENABLE_REG_EX     DPS_KEY_ENABLE_REG_EX
#endif

#ifdef DPS_ODD_KEY_ENABLE_REG_EX
#define LOCAL_DPS_ODD_KEY_ENABLE_REG_EX      DPS_ODD_KEY_ENABLE_REG_EX
#else
#define LOCAL_DPS_ODD_KEY_ENABLE_REG_EX      DPS_KEY_ENABLE_REG_EX
#endif

/*
 * End of LOCAL Defines for accessing ODD/EVEN KEY Enable Register(s)
 *****************************************************************************/


extern DemuxInfo gDemuxInfo[]; 
#ifdef DV_TIMER
void dv_timer_handler (timer_id_t timer, void *userData);
timer_id_t dv_timer;
u_int32 video_write_ptr = 0;
#endif
#ifdef DEBUG

void DumpDescramblerDebugInfo() {
    bool ks;
    u_int32 dc, vp, hc, c0, c1, ps;

    ks = critical_section_begin();

    dc = *((volatile u_int32 *)0x38000078);
    vp = *((volatile u_int32 *)0x38000084);
    hc = *((volatile u_int32 *)0x39800000);
    c0 = *((volatile u_int32 *)0x30400000);
    c1 = *((volatile u_int32 *)0x30400004);
    ps = *((volatile u_int32 *)0x38000068);

    critical_section_end(ks);

    trace_new(TRACE_CA|TRACE_LEVEL_4, "38000068=%08x 38000078=%08x 38000084=%08x\n", ps, dc, vp);
    trace_new(TRACE_CA|TRACE_LEVEL_4, "39800000=%08x 30400000=%08x 30400004=%08x\n", hc, c0, c1);

#ifdef DV_TIMER
    ks = critical_section_begin();
    if (video_write_ptr) {
        hwtimer_stop(dv_timer);
    }
    critical_section_end(ks);
    video_write_ptr = *((volatile u_int32 *)0x39000080);
    hwtimer_start(dv_timer);
#endif
}
#endif

/*****************************************************************************
 * cnxt_dmx_descrambler_init
 *
 * parameters:
 *    dmxid - demux hardware instance handle that was returned by _open()
 * returns:
 *    DMX_STATUS_OK - no errors
 *    DMX_BAD_DMXID - bad demux instance handle
 *
 * description: Initializes the Descramblers structure of a demux instance.
 *    
 ****************************************************************************/
DMX_STATUS cnxt_dmx_descrambler_init(u_int32 dmxid) {
    int i;
    DemuxInfo *dmx;
    u_int32 EvenKeyEnable, OddKeyEnable;

    trace_new(DPS_FUNC_TRACE,"DEMUX:cnxt_dmx_descrambler_init entered\n");

    if(dmxid >= MAX_DEMUX_UNITS || dmxid == 1) { /* Sanity check dmxid value, avoid demux 1 & 
                            going outside array bounds in next check */
      trace_new(DPS_ERROR_MSG, "DEMUX:cnxt_dmx_descrambler_init: demux %d does not exist!\n",dmxid);
      return DMX_BAD_DMXID;
    }

    dmx = &gDemuxInfo[dmxid];

    EvenKeyEnable = DPS_EVEN_KEY0_ENABLE;
    OddKeyEnable  = DPS_ODD_KEY0_ENABLE;
    for (i = 0; i < DPS_NUM_DESCRAMBLERS; ++i) {
        dmx->Descramblers[i].Open = FALSE;
        dmx->Descramblers[i].DescDisabled = FALSE;
        dmx->Descramblers[i].chid = GENDMX_BAD_CHANNEL;
        dmx->Descramblers[i].pid = GEN_INVALID_PID;
        dmx->DescramblerKeyInfo[i].EvenKeyEnable = EvenKeyEnable;
        dmx->DescramblerKeyInfo[i].OddKeyEnable  = OddKeyEnable;
        EvenKeyEnable <<= 1;
        OddKeyEnable <<= 1;
    }
    /* Pre allocate Audio & Video */
    dmx->Descramblers[VIDEO_CHANNEL].chid = VIDEO_CHANNEL;
    dmx->Descramblers[AUDIO_CHANNEL].chid = AUDIO_CHANNEL;

#ifdef DV_TIMER

    /*
     * Create timer for checking for first data available
     */
    dv_timer = hwtimer_create(dv_timer_handler, 0, "XXDV");
    hwtimer_set(dv_timer, 1000, FALSE);
#endif

    return DMX_STATUS_OK;
}

/*****************************************************************************
 * cnxt_dmx_descrambler_open
 *
 * parameters:
 *    dmxid - demux hardware instance handle that was returned by _open()
 *    type  - Type of descrambler we are opening
 *    *descrambler_channel - The returned descramble channel index
 * returns:
 *    DMX_STATUS_OK - no errors
 *    DMX_BAD_DMXID - bad demux instance handle
 *
 * description: Opens a descrambling channel (channel slots 0-5 are available) 
 *    
 ****************************************************************************/
DMX_STATUS cnxt_dmx_descrambler_open(u_int32 dmxid, genchannel_t type, u_int32 *descrambler_channel) {
    int i;
    DemuxInfo *dmx;
    u_int32 num_aud_chans;

    *descrambler_channel = GENDMX_BAD_CHANNEL;
    trace_new(DPS_FUNC_TRACE,"DEMUX:cnxt_dmx_descrambler_open entered\n");

    if(dmxid >= MAX_DEMUX_UNITS || dmxid == 1) { /* Sanity check dmxid value, avoid demux 1 & 
                            going outside array bounds in next check */
      trace_new(DPS_ERROR_MSG, "DEMUX:cnxt_dmx_descrambler_open: demux %d does not exist!\n",dmxid);
      return DMX_BAD_DMXID;
    }

    /* First check to see that this demux id is already allocated */
    if (!gDemuxInfo[dmxid].DemuxInitialized) {
        trace_new(DPS_ERROR_MSG,"DEMUX:cnxt_dmx_descrambler_open: demux %d does not exist!\n",dmxid);
        return DMX_BAD_DMXID;
    }

    dmx = &gDemuxInfo[dmxid];

    /*
     * SPECIAL HARDWARE NOTE:  Descramblers are hard-wired 1:1 with the first 6
     * PID slots (i.e. Descrambler #2 matches with PID slot #2).  There is no
     * hardware mapping table between the two.  Since the hardware has special
     * PID slots for live video and live audio (PID slots 1 & 2), descramblers
     * 1 & 2 are also reserved for live video and live audio and must be explicitly
     * requested by the caller or else a generic descrambler (0, 3-5) is used.  
     */

    num_aud_chans = ( ( *( DPS_CAPABILITIES_EX ( dmxid ) ) & 
                        DPS_CAPABILITIES_NUM_AUD_CHAN_MASK ) >>
                        DPS_CAPABILITIES_NUM_AUD_CHAN_SHIFT ) + 1;

    /* If live video requested, use the special video slot */
    if (type == VIDEO_PES_TYPE) {
        dmx->Descramblers[VIDEO_CHANNEL].Open = TRUE;
        *descrambler_channel = VIDEO_CHANNEL;
    }

    /* If live audio requested, use the special audio slot */
    else if (type == AUDIO_PES_TYPE) {
      #if (defined DRIVER_INCL_NDSICAM)
        dmx->Descramblers[AUDIO_CHANNEL].Open = TRUE;
        *descrambler_channel = AUDIO_CHANNEL;
      #else /* !(defined DRIVER_INCL_NDSICAM) */
        for ( i = 0 ; i < num_aud_chans ; i ++ )
        {
            if ( dmx->Descramblers[AUDIO_CHANNEL+i].Open == FALSE )
            {
                dmx->Descramblers[AUDIO_CHANNEL+i].Open = TRUE;
                *descrambler_channel = AUDIO_CHANNEL+i;
                break;
            }
        }
      #endif /* (defined DRIVER_INCL_NDSICAM) */
    }

    /* If generic, use any descrambler not already open */
    else if (type == PES_CHANNEL_TYPE || 
             type ==  ES_CHANNEL_TYPE ||
             type == PSI_CHANNEL_TYPE) {

        for (i = 0; i < DPS_NUM_DESCRAMBLERS; i++) {
            if ( ( i < VIDEO_CHANNEL ) || ( i > VIDEO_CHANNEL + num_aud_chans ) )
            {
               if (dmx->Descramblers[i].Open == FALSE) {
                   dmx->Descramblers[i].Open = TRUE;
                   *descrambler_channel = i;
                   break;
               }
            }
        }
    }

    /* Invalid channel type */
    else {
      return DMX_ERROR;
    }

    /* Did we get one? */
    if(*descrambler_channel != GENDMX_BAD_CHANNEL)
    {
      dmx->Descramblers[*descrambler_channel].chid = *descrambler_channel;
      dmx->ChannelInfoTable[*descrambler_channel].stype = type;
      if(type == PSI_CHANNEL_TYPE) {
        dmx->ChannelInfoTable[*descrambler_channel].PESChannel = FALSE;
      } else {
        dmx->ChannelInfoTable[*descrambler_channel].PESChannel = TRUE;
      } 
      return DMX_STATUS_OK;
    }
    else
    {
        return DMX_ERROR;
    }
}

/*****************************************************************************
 * cnxt_dmx_descrambler_close
 *
 * parameters:
 *    dmxid - demux hardware instance handle that was returned by _open()
 *    descrambler - descrambler index to close
 * returns:
 *    DMX_STATUS_OK - no errors
 *    DMX_BAD_DMXID - bad demux instance handle
 *    DMX_ERROR - could not close correctly
 * 
 * description: Closes a descrambling channel
 *    
 ****************************************************************************/
DMX_STATUS cnxt_dmx_descrambler_close(u_int32 dmxid, u_int32 descrambler) {
    DemuxInfo *dmx;

    trace_new(DPS_FUNC_TRACE,"DEMUX:cnxt_dmx_descrambler_close entered\n");

    if(dmxid >= MAX_DEMUX_UNITS || dmxid == 1) { /* Sanity check dmxid value, avoid demux 1 & 
                            going outside array bounds in next check */
      trace_new(DPS_ERROR_MSG, "DEMUX:cnxt_dmx_descrambler_close: demux %d does not exist!\n",
        dmxid);
      return DMX_BAD_DMXID;
    }

    if(descrambler >= DPS_NUM_DESCRAMBLERS) { /* Sanity check descrambler channel value */
      trace_new(DPS_ERROR_MSG, "DEMUX:cnxt_dmx_descrambler_close: descram %d does not exist!\n",
        descrambler);
      return DMX_BAD_CHID;
    }      

    /* First check to see that this demux id is already allocated */
    if (!gDemuxInfo[dmxid].DemuxInitialized) {
        trace_new(DPS_ERROR_MSG,"DEMUX:cnxt_dmx_descrambler_close: demux %d does not exist!\n",dmxid);
        return DMX_BAD_DMXID;
    }

    dmx = &gDemuxInfo[dmxid];
    if ((dmx->Descramblers[descrambler].chid == AUDIO_CHANNEL || 
     dmx->Descramblers[descrambler].chid == VIDEO_CHANNEL)
        && (dmx->Descramblers[descrambler].Open)) {
        dmx->Descramblers[descrambler].Open = FALSE;
        dmx->Descramblers[descrambler].pid = GEN_INVALID_PID;
        return DMX_STATUS_OK;    
    }

    if (descrambler < DPS_NUM_DESCRAMBLERS) {
        if (dmx->Descramblers[descrambler].Open) {
            dmx->Descramblers[descrambler].Open = FALSE;
            dmx->Descramblers[descrambler].chid = GENDMX_BAD_CHANNEL;
            dmx->Descramblers[descrambler].pid = GEN_INVALID_PID;
            return DMX_STATUS_OK;
        }
    }
    return DMX_ERROR;
}

/*****************************************************************************
 * cnxt_dmx_query_channel_scrambling
 *
 * parameters:
 *    dmxid - demux hardware instance handle that was returned by _open()
 *    chid - channel instance handle 
 *    status - boolean return value:
 *              TRUE if channel is scrambled
 *              FALSE is channel is not scrambled
 * returns:
 *    DMX_STATUS_OK - no errors
 *    DMX_BAD_DMXID - bad demux instance handle
 *    DMX_BAD_CHID - bad channel instance handle
 *
 * description: Queries if a given channel is currently receiving a scrambled or
 *              unscrambled stream.  Following the read of the stream ststus, the
 *              info change bit will be set indicating to the microcode that it 
 *              can now clear the bit for the next test.
 *    
 ****************************************************************************/
DMX_STATUS cnxt_dmx_query_channel_scrambling(u_int32 dmxid, u_int32 chid, bool *status) {
    DemuxInfo *dmx;

    trace_new(DPS_FUNC_TRACE,"DEMUX:cnxt_dmx_query_channel_scrambling entered\n");

    if(dmxid >= MAX_DEMUX_UNITS || dmxid == 1) { /* Sanity check dmxid value, avoid demux 1 & 
                            going outside array bounds in next check */
      trace_new(DPS_ERROR_MSG, "DEMUX:cnxt_dmx_query_channel_scrambling: demux %d does not exist!\n",dmxid);
      return DMX_BAD_DMXID;
    }

    /* First check to see that this demux id is already allocated */
    if (!gDemuxInfo[dmxid].DemuxInitialized) {
        trace_new(DPS_ERROR_MSG,"DEMUX:cnxt_dmx_query_channel_scrambling: demux %d does not exist!\n",dmxid);
        return DMX_BAD_DMXID;
    }

    dmx = &gDemuxInfo[dmxid];

    if (chid >= DPS_NUM_DESCRAMBLERS)
        return DMX_BAD_CHID;
    
    /* Check to see if this feature is supported by the current ucode */
    if(DPS_UCODE_SCRAM_STATUS & 
       *(((u_int32 *)DPS_BASE(dmxid)) + DPS_UCODE_CAP_WORD_IDX)) {
      /* Check to see if the given channel is receiving a scrambled channel */
      if ((*DPS_SCRAMBLING_STATUS_TS_EX(dmxid) | *DPS_SCRAMBLING_STATUS_PES_EX(dmxid)) 
      & (1<<dmx->ChannelInfoTable[chid].Slot))
        *status = TRUE; 
      else 
        *status = FALSE;
    } else {
      trace_new(DPS_ERROR_MSG,"DEMUX:cnxt_dmx_query_channel_scrambling: Feature not supported by microcode!\n");
      return DMX_ERROR;
    }
      
    return DMX_STATUS_OK;
}

/*****************************************************************************
 * cnxt_dmx_descrambler_set_pid
 *
 * parameters:
 *    dmxid - demux hardware instance handle that was returned by _open()
 *    descrambler - descrambler index
 *    pid - pid value to set for descrambler channel
 * returns:
 *    DMX_STATUS_OK - no errors
 *    DMX_BAD_DMXID - bad demux instance handle
 *    DMX_BAD_CHID - bad channel instance handle
 *
 * description: Sets the PID for the channel in hardware. 
 *    
 ****************************************************************************/
DMX_STATUS cnxt_dmx_descrambler_set_pid(u_int32 dmxid, u_int32 descrambler, u_int16 pid)
{
    int i;
    DemuxInfo *dmx;

    trace_new(DPS_FUNC_TRACE,"DEMUX:cnxt_dmx_descrambler_set_pid entered\n");

    if(dmxid >= MAX_DEMUX_UNITS || dmxid == 1) { /* Sanity check dmxid value, avoid demux 1 & 
                            going outside array bounds in next check */
      trace_new(DPS_ERROR_MSG, "DEMUX:cnxt_dmx_descrambler_set_pid: demux %d does not exist!\n",
        dmxid);
      return DMX_BAD_DMXID;
    }

    /* First check to see that this demux id is already allocated */
    if (!gDemuxInfo[dmxid].DemuxInitialized) {
        trace_new(DPS_ERROR_MSG,"DEMUX:cnxt_dmx_descrambler_set_pid: demux %d does not exist!\n",dmxid);
        return DMX_BAD_DMXID;
    }

    dmx = &gDemuxInfo[dmxid];

    if (descrambler >= DPS_NUM_DESCRAMBLERS || !dmx->Descramblers[descrambler].Open)
        return DMX_BAD_CHID;

    dmx->Descramblers[descrambler].pid = pid;
    if (dmx->Descramblers[descrambler].chid != GENDMX_BAD_CHANNEL) {
        return cnxt_dmx_channel_set_pid(dmxid, dmx->Descramblers[descrambler].chid, pid);
    } else {
        for (i = 0; i < DPS_NUM_DESCRAMBLERS; ++i) {
            if (*DPS_PID_BASE_EX(dmxid,i) == pid) {
                dmx->Descramblers[descrambler].chid = i;
                break;
            }
        }    
    }   
    return DMX_STATUS_OK;
}

/*****************************************************************************
 * cnxt_dmx_descrambler_set_odd_keys
 *
 * parameters:
 *    dmxid - demux hardware instance handle that was returned by _open()
 *    descrambler - descrambler index
 *    odd_key_length - key length - should be < 8
 *    odd_key - string pointing to odd key
 * returns:
 *    DMX_STATUS_OK - no errors
 *    DMX_BAD_DMXID - bad demux instance handle
 *    DMX_BAD_CHID - bad channel instance handle
 *    DMX_ERROR - error with key size
 *
 * description: Sets the odd keys for the specified descrambler channel 
 *    
 ****************************************************************************/
DMX_STATUS cnxt_dmx_descrambler_set_odd_keys(u_int32 dmxid, u_int32 descrambler, 
                                             u_int32 odd_key_length, const char * odd_key) {
    u_int32 i;
    int KeyEnable = 0;
    DPS_KEY_ENTRY Key;
    char *pKey;
    char KeyNonZero;
    u_int32 chid;
    DemuxInfo *dmx;

    trace_new(DPS_FUNC_TRACE,"DEMUX:cnxt_dmx_descrambler_set_odd_keys entered\n");

    if(dmxid >= MAX_DEMUX_UNITS || dmxid == 1) { /* Sanity check dmxid value, avoid demux 1 & 
                            going outside array bounds in next check */
      trace_new(DPS_ERROR_MSG, "DEMUX:cnxt_dmx_descrambler_set_odd_keys: demux %d does not exist!\n",
        dmxid);
      return DMX_BAD_DMXID;
    }

    /* First check to see that this demux id is already allocated */
    if (!gDemuxInfo[dmxid].DemuxInitialized) {
        trace_new(DPS_ERROR_MSG,"DEMUX:cnxt_dmx_descrambler_set_odd_keys: demux %d does not exist!\n",dmxid);
        return DMX_BAD_DMXID;
    }

    dmx = &gDemuxInfo[dmxid];

    not_interrupt_safe();

#if defined DRIVER_INCL_NDSDEMUX || defined DRIVER_INCL_NDSICAM
    chid = descrambler;
#else 
    if (descrambler >= DPS_NUM_DESCRAMBLERS)
        return DMX_BAD_CHID;
    chid = dmx->Descramblers[descrambler].chid;
    if (chid == GENDMX_BAD_CHANNEL) {
        return DMX_BAD_CHID;
    }
#endif

    if (odd_key_length > 8)
        return DMX_ERROR;
    Key.KeyLowOdd = Key.KeyHighOdd;
    if (odd_key && odd_key_length > 0) {
        KeyNonZero = 0;
        pKey = (char *) &Key.KeyHighOdd;                /* The high 32 bits are in the low address */
        for (i = 0; i < odd_key_length; ++i) {           /* copy the key locally */
            KeyNonZero |= *odd_key;
            *pKey++ = *odd_key++;
        }
        if (dmx->Descramblers[descrambler].DescDisabled == FALSE)
            KeyEnable = dmx->DescramblerKeyInfo[chid].OddKeyEnable;
        if (KeyNonZero == 0) {
            *LOCAL_DPS_ODD_KEY_ENABLE_REG_EX(dmxid) &= ~KeyEnable; /* disable the key */
        }

        *DPS_KEY_TABLE_BASE_EX(dmxid,chid,KEY_ODD_HIGH) = Key.KeyLowOdd; /* flip the high & low */
        *DPS_KEY_TABLE_BASE_EX(dmxid,chid,KEY_ODD_LOW) = Key.KeyHighOdd; /* flip the high & low */

        trace_new(TRACE_CA|TRACE_LEVEL_4, "cnxt_dmx_descrambler_set_keys(chid=%d set ODD key)\n", chid);
        if (KeyNonZero) {
            *LOCAL_DPS_ODD_KEY_ENABLE_REG_EX(dmxid) |= KeyEnable; /* enable the key */
            trace_new(TRACE_CA|TRACE_LEVEL_4, "cnxt_dmx_descrambler_set_keys(chid=%d key_en%08x)\n", chid, KeyEnable);
        }
    }

#ifdef DEBUG
    /*
     * Dump important control registers
     */
    DumpDescramblerDebugInfo();
#endif

    return DMX_STATUS_OK;
}

/*****************************************************************************
 * cnxt_dmx_descrambler_set_even_keys
 *
 * parameters:
 *    dmxid - demux hardware instance handle that was returned by _open()
 *    descrambler - descrambler index
 *    even_key_length - key length - should be < 8
 *    even_key - string pointing to even key
 * returns:
 *    DMX_STATUS_OK - no errors
 *    DMX_BAD_DMXID - bad demux instance handle
 *    DMX_BAD_CHID - bad channel instance handle
 *    DMX_ERROR - error with key size
 *
 * description: Sets the even keys for the specified descrambler channel 
 *    
 ****************************************************************************/
DMX_STATUS cnxt_dmx_descrambler_set_even_keys(u_int32 dmxid, u_int32 descrambler, 
                                              u_int32 even_key_length, const char * even_key) {
    u_int32 i;
    int KeyEnable = 0;
    DPS_KEY_ENTRY Key;
    char *pKey;
    char KeyNonZero;
    u_int32 chid;
    DemuxInfo *dmx;

    trace_new(DPS_FUNC_TRACE,"DEMUX:cnxt_dmx_descrambler_set_even_keys entered\n");

    if(dmxid >= MAX_DEMUX_UNITS || dmxid == 1) { /* Sanity check dmxid value, avoid demux 1 & 
                            going outside array bounds in next check */
      trace_new(DPS_ERROR_MSG, "DEMUX:cnxt_dmx_descrambler_set_even_keys: demux %d does not exist!\n",
        dmxid);
      return DMX_BAD_DMXID;
    }

    /* First check to see that this demux id is already allocated */
    if (!gDemuxInfo[dmxid].DemuxInitialized) {
        trace_new(DPS_ERROR_MSG,"DEMUX:cnxt_dmx_descrambler_set_even_keys: demux %d does not exist!\n",dmxid);
        return DMX_BAD_DMXID;
    }

    dmx = &gDemuxInfo[dmxid];

    not_interrupt_safe();

#if defined DRIVER_INCL_NDSDEMUX || defined DRIVER_INCL_NDSICAM
    chid = descrambler;
#else
    chid = dmx->Descramblers[descrambler].chid;
    if (descrambler >= DPS_NUM_DESCRAMBLERS || chid == GENDMX_BAD_CHANNEL) {
        return DMX_BAD_CHID;
    }
#endif

    if (even_key_length > 8)
        return DMX_ERROR;
    if (even_key && even_key_length > 0) {
        KeyNonZero = 0;
        pKey = (char *) &Key.KeyHighEven;
        for (i = 0; i < even_key_length; ++i) {
            KeyNonZero |= *even_key;
            *pKey++ = *even_key++;
        }
        if (dmx->Descramblers[descrambler].DescDisabled == FALSE)
            KeyEnable = dmx->DescramblerKeyInfo[chid].EvenKeyEnable;
        if (KeyNonZero == 0) {
            *LOCAL_DPS_EVEN_KEY_ENABLE_REG_EX(dmxid) &= ~KeyEnable; /* disable the key */
        }
        *DPS_KEY_TABLE_BASE_EX(dmxid,chid,KEY_EVEN_HIGH) = Key.KeyLowEven; /* flip the high & low */
        *DPS_KEY_TABLE_BASE_EX(dmxid,chid,KEY_EVEN_LOW) = Key.KeyHighEven; /* flip the high & low */

        trace_new(TRACE_CA|TRACE_LEVEL_4, "cnxt_dmx_descrambler_set_keys(chid=%d set EVEN key)\n", chid);
        if (KeyNonZero) {
            *LOCAL_DPS_EVEN_KEY_ENABLE_REG_EX(dmxid) |= KeyEnable; /* enable the key */
            trace_new(TRACE_CA|TRACE_LEVEL_4, "cnxt_dmx_descrambler_set_keys(chid=%d key_en=%08x)\n", chid, KeyEnable);
        }
    }

#ifdef DEBUG
    /*
     * Dump important control registers
     */
    DumpDescramblerDebugInfo();
#endif

    return DMX_STATUS_OK;
}

/*****************************************************************************
 * cnxt_dmx_descrambler_control
 *
 * parameters:
 *    dmxid - demux hardware instance handle that was returned by _open()
 *    descrambler - descrambler index
 *    enable_disable - controls enabling or disabling of the descrambler channel
 * returns:
 *    DMX_STATUS_OK - no errors
 *    DMX_BAD_DMXID - bad demux instance handle
 *    DMX_BAD_CHID - bad channel instance handle
 *    DMX_ERROR - enable_disable set incorrectly
 *
 * description: Turns descrambling on and off.
 *    
 ****************************************************************************/
DMX_STATUS cnxt_dmx_descrambler_control(u_int32 dmxid, u_int32 descrambler, gencontrol_channel_t enable_disable) {

    int OddKeyEnable, EvenKeyEnable;
    u_int32 chid;
    DemuxInfo *dmx;

    trace_new(DPS_FUNC_TRACE,"DEMUX:cnxt_dmx_descrambler_control entered\n");

    if(dmxid >= MAX_DEMUX_UNITS || dmxid == 1) { /* Sanity check dmxid value, avoid demux 1 & 
                            going outside array bounds in next check */
      trace_new(DPS_ERROR_MSG, "DEMUX:cnxt_dmx_descrambler_control: demux %d does not exist!\n",
        dmxid);
      return DMX_BAD_DMXID;
    }

    /* First check to see that this demux id is already allocated */
    if (!gDemuxInfo[dmxid].DemuxInitialized) {
        trace_new(DPS_ERROR_MSG,"DEMUX:cnxt_dmx_descrambler_control: demux %d does not exist!\n",dmxid);
        return DMX_BAD_DMXID;
    }

    dmx = &gDemuxInfo[dmxid];

    not_interrupt_safe();

#if defined DRIVER_INCL_NDSDEMUX || defined DRIVER_INCL_NDSICAM
    chid = descrambler;
#else
    chid = dmx->Descramblers[descrambler].chid;
    if (descrambler >= DPS_NUM_DESCRAMBLERS || chid == GENDMX_BAD_CHANNEL) {
        return DMX_BAD_CHID;
    }
#endif

    OddKeyEnable = dmx->DescramblerKeyInfo[chid].OddKeyEnable;
    EvenKeyEnable = dmx->DescramblerKeyInfo[chid].EvenKeyEnable;

    if (enable_disable == GEN_DEMUX_ENABLE) {
        trace_new(TRACE_CA|TRACE_LEVEL_4,
                  "cnxt_dmx_descrambler_control(GEN_DEMUX_ENABLE chid=%d odd_key_en=%08x)\n", chid, OddKeyEnable);
        trace_new(TRACE_CA|TRACE_LEVEL_4,
                  "cnxt_dmx_descrambler_control(GEN_DEMUX_ENABLE chid=%d even_key_en=%08x)\n", chid, EvenKeyEnable);
        *LOCAL_DPS_ODD_KEY_ENABLE_REG_EX(dmxid) |= OddKeyEnable; /* enable the key */
        *LOCAL_DPS_EVEN_KEY_ENABLE_REG_EX(dmxid) |= EvenKeyEnable; /* enable the key */
    } else {
        trace_new(TRACE_CA|TRACE_LEVEL_4,
                  "cnxt_dmx_descrambler_control(GEN_DEMUX_DISABLE chid=%d odd_key_en=%08x)\n", chid, OddKeyEnable);
        trace_new(TRACE_CA|TRACE_LEVEL_4,
                  "cnxt_dmx_descrambler_control(GEN_DEMUX_DISABLE chid=%d even_key_en=%08x)\n", chid, EvenKeyEnable);
        if (enable_disable == GEN_DEMUX_DISABLE) {
           *LOCAL_DPS_ODD_KEY_ENABLE_REG_EX(dmxid) &= ~OddKeyEnable; /* disable the key */
           *LOCAL_DPS_EVEN_KEY_ENABLE_REG_EX(dmxid) &= ~EvenKeyEnable; /* disable the key */
        } else {
            return DMX_ERROR;
        }
    }
    return DMX_STATUS_OK;
}


/*****************************************************************************
 * descrambler_notify_pid_change
 *
 * parameters:
 *    dmxid - demux hardware instance handle that was returned by _open()
 *    chid - channel to associate the descrambler with
 *    pid - new pid for the channel
 * returns: void
 *
 * description: Turns descrambling on and off.
 *    
 ****************************************************************************/
void descrambler_notify_pid_change(u_int32 dmxid, u_int32 chid, u_int16 pid){
    int i;
    DemuxInfo *dmx = &gDemuxInfo[dmxid];

    for (i=0; i<DPS_NUM_DESCRAMBLERS; ++i) {
        if (
	        (dmx->Descramblers[i].pid == pid && dmx->Descramblers[i].Open)&&
               (dmx->Descramblers[i].chid!=1)&&(dmx->Descramblers[i].chid!=2)
	    )
	{
            dmx->Descramblers[i].chid = chid;   
            /* Enable the channel if DESCRAMBLING is to be enabled */
        }
    }   
}

void descrambler_connect_notify(u_int32 dmxid, bool connected){
}

#ifdef DV_TIMER
/**************************************************************************
** dv_timer_handler                                                      **
** Desc.                                                                 **
**     For debug use only.  Gives indication when data is avalailable.   **
**************************************************************************/
void dv_timer_handler(timer_id_t timer, void *userData)
{
    u_int32 w;

    if(cnxt_dmx_query_status(0) == DMX_STATUS_OK) {
      w = *((volatile u_int32 *)0x39000080);

      if (w != video_write_ptr) {
        hwtimer_stop(dv_timer);
        isr_trace_new(TRACE_CA|TRACE_LEVEL_4, "data in video decode buffer!\n", 0, 0);
      }
    }
}
#endif


/************************************************************************
 $Log: 
  19   mpeg      1.18        11/14/03 5:32:09 PM    Senthil Veluswamy CR(s): 
        7963 Backed out previous change in setting Descramble Audio Channel 
        keys to make NDS tests work again
  18   mpeg      1.17        8/28/03 1:58:32 PM     Larry Wang      SCR(s) 7364
         7365 :
        Allocate correct descrambler when we have multiple audio channels.
        
  17   mpeg      1.16        8/11/03 4:45:32 PM     Bob Van Gulick  SCR(s) 6860
         7226 :
        Change DESC_CHANNELS to DPS_NUM_DESCRAMBLERS in non-NDS code
        
        
  16   mpeg      1.15        8/11/03 2:39:06 PM     Bob Van Gulick  SCR(s) 6860
         7226 :
        Add NDS specific code back in that does not require a descrambler to be
         open before setting up keys and setting descrambler control.
        
        
  15   mpeg      1.14        6/9/03 5:58:56 PM      Bob Van Gulick  SCR(s) 6755
         :
        Add support for 8 slot descram in demux.  Also change use of 
        DESC_CHANNELS to DPS_NUM_DESCRAMBLERS.
        
        
  14   mpeg      1.13        5/12/03 1:26:14 PM     Brendan Donahe  SCR(s) 6187
         6176 :
        Cleanup for debug-mode dv_timer which can result in runtime KAL 
        warnings.
        
        
  13   mpeg      1.12        4/11/03 4:00:44 PM     Brendan Donahe  SCR(s) 6010
         :
        Added check to beginning of dv_timer_handler to use query_status() to 
        check
        whether pawser memory is available.  Without this, was causing data 
        aborts when
        demux 0 was closed.
        
        
  12   mpeg      1.11        4/11/03 12:59:42 PM    Brendan Donahe  SCR(s) 5997
         :
        Added check for demux microcode version so that 
        query_channel_descrambling
        could report error status when microcode doesn't support it.
        
        
  11   mpeg      1.10        4/2/03 11:58:30 AM     Brendan Donahe  SCR(s) 5886
         :
        Modifications to support 6 simultaneous unscrambled/scrambled SI 
        including
        channels usually used for video and audio (1 & 2) as well as enforcing 
        correct
        parser microcode version which has been changed in conjunction with 
        this 
        feature enhancement.
        
        
  10   mpeg      1.9         3/24/03 4:40:04 PM     Brendan Donahe  SCR(s) 5860
         5861 :
        Added channel/demux ID as well as control command arg checking in all
        functions.
        
        
  9    mpeg      1.8         2/12/03 3:17:20 PM     Bobby Bradford  SCR(s) 5467
         :
        Removed double-slash comment on header file inclusion, and
        removed the header file ... not needed.
        
  8    mpeg      1.7         2/12/03 3:01:12 PM     Bobby Bradford  SCR(s) 5467
         :
        1) Moved LOG keyword entry to end of file
        2) Removed a TAB character
        3) Changed the access to DPS_KEY_ENABLE register to support
        the pre-brazos, single key enable register operation and the
        new brazos odd/even key enable register operation.
        
  7    mpeg      1.6         2/4/03 5:09:04 PM      Bob Van Gulick  SCR(s) 5407
         :
        Change scrambling status to use separate TS and PES registers
        
        
  6    mpeg      1.5         12/19/02 9:46:40 AM    Bob Van Gulick  SCR(s) 5193
         :
        Modify scramble query function to not use the info change bit.
        
        
  5    mpeg      1.4         12/10/02 4:39:56 PM    Bob Van Gulick  SCR(s) 5121
         :
        Update scrambler query function to return if a given channel is 
        receinving a scrambled channel or not
        
        
  4    mpeg      1.3         11/12/02 5:30:32 PM    Bob Van Gulick  SCR(s) 4945
         :
        Add new descrambler query function.  Also fix logic for descrambler 
        state of audio/video channels
        
        
  3    mpeg      1.2         3/18/02 1:23:34 PM     Bob Van Gulick  SCR(s) 3399
         :
        Change ifdef DEBUG to be around debug function as well as declaration.
        
        
  2    mpeg      1.1         2/13/02 4:31:56 PM     Tim White       SCR(s) 3188
         :
        cnxt_dmx_descrambler_open() returns incorrect descrambler for the 
        VIDEO_PES_TYPE and AUDIO_PES_RTYTYPE channel types.
        
        
  1    mpeg      1.0         12/18/01 11:01:14 AM   Bob Van Gulick  
 $
 * 
 *    Rev 1.17   28 Aug 2003 12:58:32   wangl2
 * SCR(s) 7364 7365 :
 * Allocate correct descrambler when we have multiple audio channels.
 * 
 *    Rev 1.16   11 Aug 2003 15:45:32   vangulr
 * SCR(s) 6860 7226 :
 * Change DESC_CHANNELS to DPS_NUM_DESCRAMBLERS in non-NDS code
 * 
 * 
 *    Rev 1.15   11 Aug 2003 13:39:06   vangulr
 * SCR(s) 6860 7226 :
 * Add NDS specific code back in that does not require a descrambler to be open before setting up keys and setting descrambler control.
 * 
 * 
 *    Rev 1.14   09 Jun 2003 16:58:56   vangulr
 * SCR(s) 6755 :
 * Add support for 8 slot descram in demux.  Also change use of DESC_CHANNELS to DPS_NUM_DESCRAMBLERS.
 * 
 * 
 *    Rev 1.13   12 May 2003 12:26:14   donaheb
 * SCR(s) 6187 6176 :
 * Cleanup for debug-mode dv_timer which can result in runtime KAL warnings.
 * 
 * 
 *    Rev 1.12   11 Apr 2003 15:00:44   donaheb
 * SCR(s) 6010 :
 * Added check to beginning of dv_timer_handler to use query_status() to check
 * whether pawser memory is available.  Without this, was causing data aborts when
 * demux 0 was closed.
 * 
 * 
 *    Rev 1.11   11 Apr 2003 11:59:42   donaheb
 * SCR(s) 5997 :
 * Added check for demux microcode version so that query_channel_descrambling
 * could report error status when microcode doesn't support it.
 * 
 * 
 *    Rev 1.10   02 Apr 2003 11:58:30   donaheb
 * SCR(s) 5886 :
 * Modifications to support 6 simultaneous unscrambled/scrambled SI including
 * channels usually used for video and audio (1 & 2) as well as enforcing correct
 * parser microcode version which has been changed in conjunction with this 
 * feature enhancement.
 * 
 * 
 *    Rev 1.9   24 Mar 2003 16:40:04   donaheb
 * SCR(s) 5860 5861 :
 * Added channel/demux ID as well as control command arg checking in all
 * functions.
 * 
 * 
 *    Rev 1.8   12 Feb 2003 15:17:20   bradforw
 * SCR(s) 5467 :
 * Removed double-slash comment on header file inclusion, and
 * removed the header file ... not needed.
 * 
 *    Rev 1.7   12 Feb 2003 15:01:12   bradforw
 * SCR(s) 5467 :
 * 1) Moved LOG keyword entry to end of file
 * 2) Removed a TAB character
 * 3) Changed the access to DPS_KEY_ENABLE register to support
 * the pre-brazos, single key enable register operation and the
 * new brazos odd/even key enable register operation.
 * 
 *    Rev 1.6   04 Feb 2003 17:09:04   vangulr
 * SCR(s) 5407 :
 * Change scrambling status to use separate TS and PES registers
 * 
 * 
 *    Rev 1.5   19 Dec 2002 09:46:40   vangulr
 * SCR(s) 5193 :
 * Modify scramble query function to not use the info change bit.
 * 
 * 
 *    Rev 1.4   10 Dec 2002 16:39:56   vangulr
 * SCR(s) 5121 :
 * Update scrambler query function to return if a given channel is receinving a scrambled channel or not
 * 
 * 
 *    Rev 1.3   12 Nov 2002 17:30:32   vangulr
 * SCR(s) 4945 :
 * Add new descrambler query function.  Also fix logic for descrambler state of audio/video channels
 * 
 * 
 *    Rev 1.2   18 Mar 2002 13:23:34   vangulr
 * SCR(s) 3399 :
 * Change ifdef DEBUG to be around debug function as well as declaration.
 * 
 * 
 *    Rev 1.1   13 Feb 2002 16:31:56   whiteth
 * SCR(s) 3188 :
 * cnxt_dmx_descrambler_open() returns incorrect descrambler for the VIDEO_PES_TYPE and AUDIO_PES_RTYTYPE channel types.
 * 
 * 
 *    Rev 1.0   18 Dec 2001 11:01:14   vangulr
 * SCR(s) 2977 :
 * Initial drop
 * 
 * 
 ***********************************************************************************
 * ============================== old gendemux reference info below ================
 *********************************************************************************** 
 * 
 *    Rev 1.17   17 Oct 2001 09:13:30   whiteth
 * SCR(s) 2799 :
 * Added NDS N3 ICAM functionality.
 * 
 * 
 *    Rev 1.16   02 Aug 2001 16:24:34   whiteth
 * SCR(s) 2345 2346 2347 :
 * Prep work for adding new NDS verifier support.
 * 
 *
 *    Rev 1.15   17 Jul 2001 19:24:24   kroescjl
 * SCR(s) 2296 2297 :
 * roll back last changes - same as 1.13
 * 
 *    Rev 1.14   22 Jun 2001 16:19:30   kroescjl
 * SCR(s) 1758 2140 :
 * took out all the hardcoded dependencies and relation between channel
 * numbers and pid slots.  Most of the calls in this file now map directly
 * to a call in gendemux.c.  The only function that does real work here is
 * the set keys, which could probably be moved to gendemux.c.
 * 
 *    Rev 1.13   08 Jun 2001 13:31:02   whiteth
 * SCR(s) 2053 2054 :
 * Only use the video buffer debugging when NDS is present.
 * 
 * 
 *    Rev 1.12   08 Jun 2001 09:07:08   whiteth
 * SCR(s) 2045 2046 :
 * Refined the trace for when there's data in the video decode buffer
 * to improve trace/debug readibility.
 * 
 * 
 *    Rev 1.11   07 Jun 2001 14:30:42   whiteth
 * SCR(s) 1958 1990 :
 * Modified trace for better system-wide tracing.  Also added
 * a timer for debugging channel change problem.
 * 
 * 
 *    Rev 1.10   16 May 2001 16:25:22   whiteth
 * DCS#1179,1183,1909 -- Fix long channel change delay.
 * 
 *    Rev 1.9   23 Feb 2001 09:58:28   mustafa
 * DCS #1299. Removed parameter checking when NDSDEMUX is used. NDSDEMUX will
 * perform its own checking.
 * 
 *    Rev 1.8   06 Feb 2001 17:22:48   mustafa
 * DCS 1135. This does not completely fix the problem but allows at least
 * some descrambled small OpenTV apps to download correctly.
 * The change to the driver to manage the descrambling channels as virtual 
 * channels and link them to the actual channel once the PID is set by the
 * demux driver.
 * 
 *    Rev 1.7   25 Aug 2000 09:05:20   mustafa
 * Fixed warning.
 *
 *    Rev 1.6   19 Jun 2000 13:30:32   mustafa
 * Cleanup VxWorks warnings.
 *
 *    Rev 1.5   15 May 2000 16:18:24   mustafa
 * Added DescDisabled.
 *
 *    Rev 1.4   04 Apr 2000 19:59:44   mustafa
 * Disable channel before closing.
 *
 *    Rev 1.3   07 Mar 2000 16:03:12   mustafa
 * Backup.
 *
 *    Rev 1.2   31 Jan 2000 16:02:28   mustafa
 * Backup.
 *
 *    Rev 1.1   06 Jan 2000 15:18:30   mustafa
 * Backup.
 *
 *    Rev 1.0   03 Dec 1999 15:53:14   mustafa
 * Initial revision.
 *
 *    Rev 1.1   28 Sep 1999 15:03:48   mustafa
 * Cleanup.
 *
 *    Rev 1.0   02 Sep 1999 16:50:56   mustafa
 * Initial revision.
 *
 ************************************************************************/





