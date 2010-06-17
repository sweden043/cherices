/****************************************************************************/
/*                   Conexant Proprietary and Confidential                  */
/*                       Software File/Module Header                        */
/*                      Conexant Systems Inc. (c) 2003                      */
/*                               Austin, TX                                 */
/*                            All Rights Reserved                           */
/****************************************************************************/
/*
 * Filename:       demuxdma.c
 *
 *
 * Description:    DMA Input Demux Extension
 *
 *
 * Author:         Tim White
 *
 ****************************************************************************/
/* $Id: demuxdma.c,v 1.10, 2004-06-15 22:15:40Z, Tim White$
 ****************************************************************************/

#include "stbcfg.h"
#include "retcodes.h"
#include "kal.h"
#include "demuxapi.h"
#include "demuxint.h"

/*
 * Local Definitions
 */
#define DMX_DMA_MAX_DMA_SIZE                 65504       /* 64KB-32bytes per xfer   */
#define DMX_DMA_INPUT_RECOVERY_TIME            15        /*  15 bus cycles          */
#define DMX_DMA_INPUT_TRANSFER_SIZE             4        /*   4 words               */
#define DMX_DMA_INPUT_TSIN_BUFFER_SIZE       0x00100000  /*  1MB aperture           */
#define DMX_DMA_INPUT_SP0_TSIN_REQUEST_LINE    12        /* Demux 0 TS_In Req Line  */
#define DMX_DMA_INPUT_SP2_TSIN_REQUEST_LINE    16        /* Demux 2 TS_In Req Line  */
#define DMX_DMA_INPUT_SP3_TSIN_REQUEST_LINE    18        /* Demux 3 TS_In Req Line  */
#define DMX_DMA_INPUT_SP0_TSIN_BUFFER_ADDR   0x39C00000  /* Demux 0 TS_In Buff Addr */
#define DMX_DMA_INPUT_SP2_TSIN_BUFFER_ADDR   0x3DC00000  /* Demux 2 TS_In Buff Addr */
#define DMX_DMA_INPUT_SP3_TSIN_BUFFER_ADDR   0x3FC00000  /* Demux 3 TS_In Buff Addr */

/*
 * Externals
 */
extern DemuxInfo gDemuxInfo[];
extern DemuxDriverInfo gDemuxDriverInfo;

/*
 * Internals
 */
/* INTERNAL_BEGIN */
static void dmx_dma_input_physio(DemuxInfo *dmx);
static bool dmx_dma_input_halt(DemuxInfo *dmx, bool immediate);
static DMX_STATUS dmx_dma_input_init(DemuxInfo *dmx);
static int dmx_dma_input_isr(u_int32 dwIntID, bool bFIQ, PFNISR *pfnChain);
/* INTERNAL_END */

/********************************************************************/
/*  FUNCTION:     cnxt_dmx_set_input_device                         */
/*                                                                  */
/*  DESCRIPTION:                                                    */
/*      Set the demux to get input from either the NIM or DMA.      */
/*  INTERNAL:                                                       */
/*      On Colorado chips, we have a special memory location the    */
/*      parser to handle input selection.  Rio chips have the       */
/*      setting in the parser control register.                     */
/*                                                                  */
/*  INPUT PARAMETERS:                                               */
/*      dmxid             - Demux instance                          */
/*      input_device      - Input device, one of                    */
/*                       <p>DMX_INPUT_DEV_NIM for NIM input         */
/*                       <p>DMX_INPUT_DEV_DMA for DMA input         */
/*                                                                  */
/*  OUTPUT PARAMETERS:                                              */
/*    None                                                          */
/*                                                                  */
/*  RETURN VALUES:                                                  */
/*      DMX_STATUS_OK     - Success                                 */
/*      DMX_BAD_DMXID     - Bad demux instance handle               */
/*      DMX_ERROR         - Error                                   */
/*                                                                  */
/*  INTERNAL:                                                       */
/*  !!! WARNING !!! WARNING !!! WARNING !!! WARNING !!! WARNING !!! */
/*  !!! WARNING !!! WARNING !!! WARNING !!! WARNING !!! WARNING !!! */
/*  NOTES:                                                          */
/*   IMPORTANT!                                                     */
/*   This function causes all state in the pawser microcode to be   */
/*   lost!  This includes such things as the PID table.  Only use   */
/*   this function when state information is unimportant.           */
/*                                                                  */
/*   This function must be called prior to starting the DMA input.  */
/*                                                                  */
/*  CONTEXT:                                                        */
/*   Must be called from non-interrupt context.                     */
/********************************************************************/
DMX_STATUS cnxt_dmx_set_input_device(u_int32 dmxid, u_int16 input_device)
{
    DemuxInfo *dmx;

    trace_new(DPS_FUNC_TRACE,"DEMUX:cnxt_dmx_set_input_device\n");

    /*
     * Sanity check dmxid value, avoid demux 1 & going
     * outside array bounds in next check
     */
    if(dmxid >= MAX_DEMUX_UNITS || dmxid == 1)
    {
      trace_new(DPS_ERROR_MSG,
          "DEMUX:cnxt_dmx_set_input_device: demux %d does not exist!\n", dmxid);
      return DMX_BAD_DMXID;
    }

    /*
     * Check to see that this demux id is already allocated
     */
    if (!gDemuxInfo[dmxid].DemuxInitialized)
    {
        trace_new(DPS_ERROR_MSG,
            "DEMUX:cnxt_dmx_set_input_device: demux %d does not exist!\n", dmxid);
        return DMX_BAD_DMXID;
    }

    dmx = &gDemuxInfo[dmxid];

    /*
     * Is DMA active?
     */
    if(dmx->dma_input_active)
    {
        trace_new(DPS_ERROR_MSG,
            "DEMUX:cnxt_dmx_set_input_device: Demux is active! Can't switch...\n");
        return (DMX_ERROR);
    }

    if (input_device == DMX_INPUT_DEV_NIM)
    {
        /*
         * Ensure the DMA system has been initialized.
         */
        if (!dmx->dma_input_init)
        {
            trace_new(DPS_ERROR_MSG,
                "DEMUX:cnxt_dmx_set_input_device: Demux DMA not initialized!\n");
            return (DMX_ERROR);
        }

        /*
         * If DMA input selected, switch it back to NIM input now
         */
        if (dmx->dma_input_selected)
        {
            /*
             * Set to NIM input
             */
#if (PARSER_MICROCODE == UCODE_COLORADO)
            *DPS_DMA_MUX_SELECT_EX(dmxid) &= 0x7fffffff;
#else
            *DPS_PARSER_CTL_REG_EX(dmxid) &= ~(DPS_INPUT_SRC_SEL_DMA              |
                                               DPS_TS_STREAM_OUTPUT_PACING_ENABLE |
                                               DPS_TS_IN_DMA_ENABLE               |
                                               DPS_RUN);
            *DPS_PARSER_CTL_REG_EX(dmxid) |=   DPS_RUN;
#endif
            dmx->dma_input_selected = FALSE;
        }
    }
    else if (input_device == DMX_INPUT_DEV_DMA)
    {
        /*
         * If NIM input selected, switch it to DMA input now
         */
        if (!dmx->dma_input_selected)
        {
            /*
             * Set to DMA input
             */
#if (PARSER_MICROCODE == UCODE_COLORADO)
           *DPS_DMA_MUX_SELECT_EX(dmxid) |= 0x80000000;
#else
           *DPS_PARSER_CTL_REG_EX(dmxid) &= ~(DPS_TS_STREAM_OUTPUT_PACING_ENABLE |
                                              DPS_RUN);
           *DPS_PARSER_CTL_REG_EX(dmxid) |=  (DPS_INPUT_SRC_SEL_DMA              |
                                              DPS_TS_IN_DMA_ENABLE               |
                                              DPS_RUN);
#endif
            dmx->dma_input_selected = TRUE;
        }
    }
    else
    {
        trace_new(DPS_ERROR_MSG, "DEMUX:cnxt_dmx_set_input_device: invalid input device\n");
        return DMX_ERROR;
    }

    return DMX_STATUS_OK;
}

/********************************************************************/
/*  FUNCTION:     cnxt_dmx_set_dma_input_endianess                  */
/*                                                                  */
/*  INPUT PARAMETERS:                                               */
/*      dmxid             - Demux instance                          */
/*      endianess         - DMX_BIG_ENDIAN                          */
/*                       <p>DMX_LITTLE_ENDIAN                       */
/*                                                                  */
/*  OUTPUT PARAMETERS:                                              */
/*    None                                                          */
/*                                                                  */
/*  RETURN VALUES:                                                  */
/*      DMX_STATUS_OK     - Success                                 */
/*      DMX_BAD_DMXID     - Bad demux instance handle               */
/*      DMX_ERROR         - Error                                   */
/*                                                                  */
/*  DESCRIPTION:                                                    */
/*    Set the endianess of the data that will be DMA'd into the     */
/*    specified demux.                                              */
/*                                                                  */
/*  CONTEXT:                                                        */
/*    Must be called from non-interrupt context.                    */
/*                                                                  */
/*  NOTES:                                                          */
/*    This function must be called after a demux has been opened    */
/*    and prior to starting the DMA input.                          */
/********************************************************************/
DMX_STATUS cnxt_dmx_set_dma_input_endianess(u_int32 dmxid, DMX_ENDIANESS endianess)
{
    DemuxInfo *dmx;

    trace_new(DPS_FUNC_TRACE,"DEMUX:cnxt_dmx_set_dma_input_endianess\n");

    /*
     * Sanity check dmxid value, avoid demux 1 & going
     * outside array bounds in next check
     */
    if(dmxid >= MAX_DEMUX_UNITS || dmxid == 1)
    {
      trace_new(DPS_ERROR_MSG,
          "DEMUX:cnxt_dmx_set_dma_input_endianess: demux %d does not exist!\n", dmxid);
      return DMX_BAD_DMXID;
    }

    /*
     * Check to see that this demux id is already allocated
     */
    if (!gDemuxInfo[dmxid].DemuxInitialized)
    {
        trace_new(DPS_ERROR_MSG,
            "DEMUX:cnxt_dmx_set_dma_input_endianess: demux %d does not exist!\n", dmxid);
        return DMX_BAD_DMXID;
    }

    dmx = &gDemuxInfo[dmxid];

    /*
     * Is DMA active?
     */
    if(dmx->dma_input_active)
    {
        trace_new(DPS_ERROR_MSG,
            "DEMUX:cnxt_dmx_set_dma_input_endianess: Demux is active! Can't switch...\n");
        return (DMX_ERROR);
    }

    switch (endianess)
    {
        case DMX_BIG_ENDIAN:
            /*
             * Set for big endian input.
             */
            *DPS_PARSER_CTL_REG_EX(dmxid) &= ~DPS_INPUT_BYTESWAP_LITTLE_ENDIAN;
            break;
        
        case DMX_LITTLE_ENDIAN:
            /*
             * Set for little endian input..
             */
            *DPS_PARSER_CTL_REG_EX(dmxid) |= DPS_INPUT_BYTESWAP_LITTLE_ENDIAN;
            break;
            
        default:
            trace_new(DPS_ERROR_MSG, "DEMUX:cnxt_dmx_set_input_device: invalid input endianess\n");
            return DMX_ERROR;
    }

    return DMX_STATUS_OK;
}

/********************************************************************/
/*  FUNCTION:     cnxt_dmx_set_dma_input_channel                    */
/*                                                                  */
/*  PARAMETERS:                                                     */
/*      dmxid             - Demux instance                          */
/*      channel           - System DMA channel.  One of:            */
/*                          <p>      0 - Clear DMA channel          */
/*                          <p>      DMA_CH_0                       */
/*                          <p>      DMA_CH_1                       */
/*                          <p>      DMA_CH_2                       */
/*                          <p>      DMA_CH_3                       */
/*                                                                  */
/*  DESCRIPTION:                                                    */
/*      Set the system DMA channel to use for demux DMA input.      */
/*                                                                  */
/*  CONTEXT:                                                        */
/*   Must be called from non-interrupt context.                     */
/*                                                                  */
/*  RETURN VALUES:                                                  */
/*      DMX_STATUS_OK     - Success                                 */
/*      DMX_ERROR         - Error                                   */
/*                                                                  */
/*  NOTES:                                                          */
/*    This function must be called prior to starting the DMA Input. */
/********************************************************************/
DMX_STATUS cnxt_dmx_set_dma_input_channel(u_int32 dmxid, u_int32 channel)
{
    DemuxInfo *dmx;
    u_int32   chan;
    bool      ks;

    trace_new(DPS_FUNC_TRACE, "DEMUX:cnxt_dmx_set_dma_input_channel: entered\n");

    /*
     * Sanity check dmxid value, avoid demux 1 & going
     * outside array bounds in next check
     */
    if(dmxid >= MAX_DEMUX_UNITS || dmxid == 1)
    {
        trace_new(DPS_ERROR_MSG,
            "DEMUX:cnxt_dmx_set_dma_input_channel: demux %d does not exist!\n", dmxid);
        return (DMX_BAD_DMXID);
    }

    /*
     * First check to see that this demux id is already allocated
     */
    if (!gDemuxInfo[dmxid].DemuxInitialized)
    {
        trace_new(DPS_ERROR_MSG,
            "DEMUX:cnxt_dmx_set_dma_input_channel: demux %d is not initialized!\n", dmxid);
        return (DMX_BAD_DMXID);
    }

    dmx = &gDemuxInfo[dmxid];

    /*
     * Check for clearing the DMA channel first
     */
    if(!channel)
    {
        ks = critical_section_begin();
        switch(dmx->dma_input_channel)
        {
            case 0:
                channel = DMA_CH_0;
                break;
            case 1:
                channel = DMA_CH_1;
                break;
            case 2:
                channel = DMA_CH_2;
                break;
            case 3:
                channel = DMA_CH_3;
                break;
            default:
                trace_new(DPS_ERROR_MSG,
                    "DEMUX:cnxt_dmx_set_dma_input_channel: Invalid DMA channel %d!\n", channel);
                return (DMX_ERROR);
        }

        /*
         * Clear the transport stream input DMA channel
         */
        dmx->dma_input_channel = 0;

        /*
         * Remove the channel to the global demux intr mask
         */
        gDemuxDriverInfo.dma_input_intr_mask &= ~channel;
        critical_section_end(ks);

        return (DMX_STATUS_OK);
    }

    /*
     * Check for valid channel
     */
    switch(channel)
    {
        case DMA_CH_0:
            chan = 0;
            break;
        case DMA_CH_1:
            chan = 1;
            break;
        case DMA_CH_2:
            chan = 2;
            break;
        case DMA_CH_3:
            chan = 3;
            break;

        default:
            trace_new(DPS_ERROR_MSG,
                "DEMUX:cnxt_dmx_set_dma_input_channel: Invalid DMA channel %d!\n", channel);
            return (DMX_ERROR);
    }

    /*
     * Check for channel already in use
     */
    if(channel & gDemuxDriverInfo.dma_input_intr_mask)
    {
        trace_new(DPS_ERROR_MSG,
            "DEMUX:cnxt_dmx_set_dma_input_channel: DMA channel %d already in use!\n", channel);
        return (DMX_ERROR);
    }

    ks = critical_section_begin();

    /*
     * Is device active?
     */
    if(dmx->dma_input_active)
    {
        critical_section_end(ks);
        trace_new(DPS_ERROR_MSG,
            "DEMUX:cnxt_dmx_set_dma_input_channel: Demux is active! Can't modify...\n");
        return (DMX_ERROR);
    }

    /*
     * Set the transport stream input DMA channel
     */
    dmx->dma_input_channel = chan;

    /*
     * Add the channel to the global demux intr mask
     */
    gDemuxDriverInfo.dma_input_intr_mask |= channel;

    critical_section_end(ks);
    return (DMX_STATUS_OK);
}

/********************************************************************/
/*  FUNCTION:     cnxt_dmx_set_dma_input_notify_fct                 */
/*                                                                  */
/*  PARAMETERS:                                                     */
/*      dmxid             - Demux instance                          */
/*      notify_fct        - Asynchronous notification function      */
/*                                                                  */
/*  DESCRIPTION:                                                    */
/*    Set the Demux DMA Input asynchronous notification function.   */
/*    The notitfication function is used for indicating a memory    */
/*    buffer is no longer being used by DMA and can be re-used by   */
/*    the client.                                                   */
/*                                                                  */
/*  CONTEXT:                                                        */
/*    Must be called from non-interrupt context.                    */
/*                                                                  */
/*  RETURN VALUES:                                                  */
/*      DMX_STATUS_OK     - Success                                 */
/*      DMX_ERROR         - Error                                   */
/*                                                                  */
/*  NOTES:                                                          */
/*    This function must be called prior to starting the DMA Input. */
/********************************************************************/
DMX_STATUS cnxt_dmx_set_dma_input_notify_fct(u_int32 dmxid,
                                dmx_dma_input_notify_fct_t notify_fct)
{
    DemuxInfo *dmx;
    bool      ks;

    trace_new(DPS_FUNC_TRACE, "DEMUX:cnxt_dmx_set_dma_input_buffer: entered\n");

    /*
     * Check for valid function address
     */
    if(!notify_fct)
    {
        trace_new(DPS_ERROR_MSG,
            "DEMUX:cnxt_dmx_set_dma_input_notify_fct: Bogus notify_fct!\n");
        return (DMX_ERROR);
    }

    /*
     * Sanity check dmxid value, avoid demux 1 & going
     * outside array bounds in next check
     */
    if(dmxid >= MAX_DEMUX_UNITS || dmxid == 1)
    {
        trace_new(DPS_ERROR_MSG,
            "DEMUX:cnxt_dmx_set_dma_input_notify_fct: demux %d does not exist!\n", dmxid);
        return (DMX_BAD_DMXID);
    }

    /*
     * First check to see that this demux id is already allocated
     */
    if (!gDemuxInfo[dmxid].DemuxInitialized)
    {
        trace_new(DPS_ERROR_MSG,
            "DEMUX:cnxt_dmx_set_dma_input_notify_fct: demux %d is not initialized!\n", dmxid);
        return (DMX_BAD_DMXID);
    }

    dmx = &gDemuxInfo[dmxid];

    ks = critical_section_begin();

    /*
     * Is device active?
     */
    if(dmx->dma_input_active)
    {
        critical_section_end(ks);
        trace_new(DPS_ERROR_MSG,
            "DEMUX:cnxt_dmx_set_dma_input_notify_fct: Demux is active! Can't modify...\n");
        return (DMX_ERROR);
    }

    /*
     * Set the asynchronous notification function
     */
    dmx->dma_input_notify = notify_fct;

    critical_section_end(ks);
    return (DMX_STATUS_OK);
}

/********************************************************************/
/*  FUNCTION:     cnxt_dmx_dma_input_add_buffer                     */
/*                                                                  */
/*  PARAMETERS:                                                     */
/*      dmxid             - Demux instance                          */
/*      tag               - 32 bit user-defined tag sent back on    */
/*                          the asynchronous notification callback  */
/*      buffer            - Memory buffer to DMA                    */
/*      size              - Size of memory buffer (in bytes)        */
/*                                                                  */
/*  DESCRIPTION:                                                    */
/*      Adds a memory buffer to the DMA queue.                      */
/*                                                                  */
/*  CONTEXT:                                                        */
/*      No Restriction                                              */
/*                                                                  */
/*  RETURN VALUES:                                                  */
/*      DMX_STATUS_OK     - Success                                 */
/*      DMX_ERROR         - Error                                   */
/*                                                                  */
/*  NOTES:                                                          */
/*    None                                                          */
/********************************************************************/
DMX_STATUS cnxt_dmx_dma_input_add_buffer(u_int32 dmxid,
                                         u_int32 tag,
                                         u_int8  *buffer,
                                         u_int32 size)
{
    DemuxInfo *dmx;
    bool      ks;
    dmx_dma_input_q_t *slot;

    isr_trace_new(DPS_FUNC_TRACE, "DEMUX:cnxt_dmx_dma_input_add_buffer: entered\n", 0, 0);

    /*
     * Ensure valid memory buffer
     */
    if(!buffer)
    {
        isr_trace_new(DPS_ERROR_MSG,
            "DEMUX:cnxt_dmx_dma_input_add_buffer: NULL memory buffer!\n", 0, 0);
        return (DMX_ERROR);
    }

    /*
     * Ensure buffer is word aligned
     */
    if((u_int32)buffer & 0x3)
    {
        isr_trace_new(DPS_ERROR_MSG,
            "DEMUX:cnxt_dmx_dma_input_add_buffer: Memory buffer not word aligned!\n", 0, 0);
        return (DMX_ERROR);
    }

    /*
     * Ensure valid memory buffer size
     */
    if(!size)
    {
        isr_trace_new(DPS_ERROR_MSG,
            "DEMUX:cnxt_dmx_dma_input_add_buffer: NULL memory buffer size!\n", 0, 0);
        return (DMX_ERROR);
    }

    /*
     * Ensure buffer is at least word aligned in size
     */
    if(size & 0x3)
    {
        isr_trace_new(DPS_ERROR_MSG,
            "DEMUX:cnxt_dmx_dma_input_add_buffer: Memory buffer size not word aligned!\n", 0, 0);
        return (DMX_ERROR);
    }

    /*
     * Sanity check dmxid value, avoid demux 1 & going
     * outside array bounds in next check
     */
    if(dmxid >= MAX_DEMUX_UNITS || dmxid == 1)
    {
        isr_trace_new(DPS_ERROR_MSG,
            "DEMUX:cnxt_dmx_dma_input_add_buffer: demux %d does not exist!\n", dmxid, 0);
        return (DMX_BAD_DMXID);
    }

    /*
     * First check to see that this demux id is already allocated
     */
    if (!gDemuxInfo[dmxid].DemuxInitialized)
    {
        isr_trace_new(DPS_ERROR_MSG,
            "DEMUX:cnxt_dmx_dma_input_add_buffer: demux %d is not initialized!\n", dmxid, 0);
        return (DMX_BAD_DMXID);
    }

    dmx = &gDemuxInfo[dmxid];

    ks = critical_section_begin();

    /*
     * Is DMA input active?
     */
    if(!dmx->dma_input_active)
    {
        critical_section_end(ks);
        isr_trace_new(DPS_ERROR_MSG,
            "DEMUX:cnxt_dmx_set_dma_input_add_buffer: Demux is not active! Can't modify...\n", 0, 0);
        return (DMX_ERROR);
    }

    /*
     * Queue the memory buffer
     */
    slot                  = dmx->dma_input_q_free;
    if(!slot)
    {
        critical_section_end(ks);
        isr_trace_new(DPS_ERROR_MSG,
            "DEMUX:cnxt_dmx_set_dma_input_add_buffer: No free queue slots!\n", 0, 0);
        return (DMX_ERROR);
    }

    dmx->dma_input_q_free = slot->next;

    slot->buffer = buffer;
    slot->size   = size;
    slot->tag    = tag;
    slot->next   = NULL;

    if(dmx->dma_input_q_tail)
    {
        dmx->dma_input_q_tail->next = slot;
    }
    else
    {
        dmx->dma_input_q_head = slot;
    }
    dmx->dma_input_q_tail = slot;

    /*
     * If not busy, give it a kick...
     */
    if(!dmx->dma_input_busy)
    {
        dmx_dma_input_physio(dmx);
    }

    critical_section_end(ks);
    return (DMX_STATUS_OK);
}

/********************************************************************/
/*  FUNCTION:     cnxt_dmx_dma_input_start                          */
/*                                                                  */
/*  PARAMETERS:                                                     */
/*      dmxid             - Demux instance                          */
/*                                                                  */
/*  DESCRIPTION:                                                    */
/*      Start Demux DMA Input processing.                           */
/*                                                                  */
/*  CONTEXT:                                                        */
/*    Must be called from non-interrupt context.                    */
/*                                                                  */
/*  RETURN VALUES:                                                  */
/*      DMX_STATUS_OK     - Success                                 */
/*      DMX_ERROR         - Error                                   */
/*                                                                  */
/*  NOTES:                                                          */
/*    None                                                          */
/********************************************************************/
DMX_STATUS cnxt_dmx_dma_input_start(u_int32 dmxid)
{
    DemuxInfo  *dmx;
    bool       ks;
    DMX_STATUS rc;

    trace_new(DPS_FUNC_TRACE, "DEMUX:cnxt_dmx_dma_input_start: entered\n");

    /*
     * Sanity check dmxid value, avoid demux 1 & going
     * outside array bounds in next check
     */
    if(dmxid >= MAX_DEMUX_UNITS || dmxid == 1)
    {
        trace_new(DPS_ERROR_MSG,
            "DEMUX:cnxt_dmx_dma_input_start: demux %d does not exist!\n", dmxid);
        return (DMX_BAD_DMXID);
    }

    /*
     * First check to see that this demux id is already allocated
     */
    if (!gDemuxInfo[dmxid].DemuxInitialized)
    {
        trace_new(DPS_ERROR_MSG,
            "DEMUX:cnxt_dmx_dma_input_start: demux %d is not initialized!\n", dmxid);
        return (DMX_BAD_DMXID);
    }

    dmx = &gDemuxInfo[dmxid];

    /*
     * If never been initialized, initialize...
     */
    if(!dmx->dma_input_init)
    {
        rc = dmx_dma_input_init(dmx);
        if(rc != DMX_STATUS_OK)
        {
            trace_new(DPS_ERROR_MSG,
                "DEMUX:cnxt_dmx_dma_input_start: Failed to init! rc = %d\n", rc);
            return(DMX_ERROR);
        }
        dmx->dma_input_init = TRUE;
    }

    ks = critical_section_begin();

    /*
     * If not active, activate...
     */
    if(!dmx->dma_input_active)
    {
        dmx->dma_input_active = TRUE;
    }

    critical_section_end(ks);
    return (DMX_STATUS_OK);
}

/********************************************************************/
/*  cnxt_dmx_dma_input_stop                                         */
/*                                                                  */
/*  PARAMETERS:                                                     */
/*      dmxid             - Demux instance                          */
/*      immediate         - TRUE, Stop DMA immediately              */
/*                       <p>FALSE, Stop DMA on next buffer boundary */
/*                                                                  */
/*  DESCRIPTION:                                                    */
/*      Stop Demux DMA Input processing.                            */
/*                                                                  */
/*  CONTEXT:                                                        */
/*    Must be called from non-interrupt context.                    */
/*                                                                  */
/*  RETURN VALUES:                                                  */
/*      DMX_STATUS_OK     - Success                                 */
/*      DMX_ERROR         - Error                                   */
/*                                                                  */
/*  NOTES:                                                          */
/*    None                                                          */
/********************************************************************/
DMX_STATUS cnxt_dmx_dma_input_stop(u_int32 dmxid, bool immediate)
{
    DemuxInfo *dmx;
    bool      ks, wait = FALSE;

    trace_new(DPS_FUNC_TRACE, "DEMUX:cnxt_dmx_dma_input_stop: entered\n");

    /*
     * Sanity check dmxid value, avoid demux 1 & going
     * outside array bounds in next check
     */
    if(dmxid >= MAX_DEMUX_UNITS || dmxid == 1)
    {
        trace_new(DPS_ERROR_MSG,
            "DEMUX:cnxt_dmx_dma_input_stop: demux %d does not exist!\n", dmxid);
        return (DMX_BAD_DMXID);
    }

    /*
     * First check to see that this demux id is already allocated
     */
    if (!gDemuxInfo[dmxid].DemuxInitialized)
    {
        trace_new(DPS_ERROR_MSG,
            "DEMUX:cnxt_dmx_dma_input_stop: demux %d is not initialized!\n", dmxid);
        return (DMX_BAD_DMXID);
    }

    dmx = &gDemuxInfo[dmxid];

    ks = critical_section_begin();

    /*
     * If active, inactivate...
     */
    if(dmx->dma_input_active)
    {
        dmx->dma_input_active = FALSE;
        wait = dmx_dma_input_halt(dmx, immediate);
    }

    critical_section_end(ks);

    /*
     * If busy, wait until current buffer is done
     */
    if(wait)
    {
        sem_get(dmx->dma_input_sem, KAL_WAIT_FOREVER);
    }

    return (DMX_STATUS_OK);
}

/********************************************************************/
/*  FUNCTION:     cnxt_dmx_dma_input_reset                          */
/*                                                                  */
/*  PARAMETERS:                                                     */
/*      dmxid             - Demux instance                          */
/*      immediate         - TRUE, Stop DMA immediately              */
/*                       <p>FALSE, Stop DMA on next buffer boundary */
/*                                                                  */
/*  DESCRIPTION:                                                    */
/*      Reset Demux DMA Input processing command queue.             */
/*                                                                  */
/*  CONTEXT:                                                        */
/*    Must be called from non-interrupt context.                    */
/*                                                                  */
/*  RETURN VALUES:                                                  */
/*      DMX_STATUS_OK     - Success                                 */
/*      DMX_ERROR         - Error                                   */
/*                                                                  */
/*  NOTES:                                                          */
/*    None                                                          */
/********************************************************************/
DMX_STATUS cnxt_dmx_dma_input_reset(u_int32 dmxid, bool immediate)
{
    DemuxInfo *dmx;
    u_int32   i;
    bool      ks, wait = FALSE;

    trace_new(DPS_FUNC_TRACE, "DEMUX:cnxt_dmx_dma_input_reset: entered\n");

    /*
     * Sanity check dmxid value, avoid demux 1 & going
     * outside array bounds in next check
     */
    if(dmxid >= MAX_DEMUX_UNITS || dmxid == 1)
    {
        trace_new(DPS_ERROR_MSG,
            "DEMUX:cnxt_dmx_dma_input_reset: demux %d does not exist!\n", dmxid);
        return (DMX_BAD_DMXID);
    }

    /*
     * First check to see that this demux id is already allocated
     */
    if (!gDemuxInfo[dmxid].DemuxInitialized)
    {
        trace_new(DPS_ERROR_MSG,
            "DEMUX:cnxt_dmx_dma_input_reset: demux %d is not initialized!\n", dmxid);
        return (DMX_BAD_DMXID);
    }

    dmx = &gDemuxInfo[dmxid];

    ks = critical_section_begin();

    /*
     * If active, inactivate...
     */
    if(dmx->dma_input_active)
    {
        dmx->dma_input_active = FALSE;
        wait = dmx_dma_input_halt(dmx, immediate);
    }

    critical_section_end(ks);

    /*
     * If busy, wait until current buffer is done
     */
    if(wait)
    {
        sem_get(dmx->dma_input_sem, KAL_WAIT_FOREVER);
    }

    ks = critical_section_begin();

    /*
     * Reset the queue
     */
    dmx->dma_current_size       = 0;
    dmx->dma_current_buffer_ptr = 0;
    dmx->dma_bytes_left         = 0;
    dmx->dma_input_q_head       = NULL;
    dmx->dma_input_q_tail       = NULL;
    dmx->dma_input_q_free       = &dmx->dma_input_q[0];
    for(i=0; i<(DMX_DMA_INPUT_Q_SIZE-1); i++)
    {
        dmx->dma_input_q[i].next = &dmx->dma_input_q[i+1];
    }
    dmx->dma_input_q[DMX_DMA_INPUT_Q_SIZE-1].next = NULL;

    critical_section_end(ks);
    return (DMX_STATUS_OK);
}

/********************************************************************/
/********************************************************************/
/***            I N T E R N A L    F U N C T I O N S              ***/
/********************************************************************/
/********************************************************************/
/* INTERNAL_BEGIN */
/********************************************************************/
/*  dmx_dma_input_physio                                            */
/*                                                                  */
/*  PARAMETERS:                                                     */
/*      dmx               - DemuxInfo ptr                           */
/*                                                                  */
/*  DESCRIPTION:                                                    */
/*      (Re)Activate DMA input.                                     */
/*                                                                  */
/*  CONTEXT:                                                        */
/*      Must be called with device interrupts disabled.             */
/*                                                                  */
/*  RETURNS:                                                        */
/*      Nothing.                                                    */
/********************************************************************/
static void dmx_dma_input_physio(DemuxInfo *dmx)
{
    u_int32 dma_xfer_size;
    dmx_dma_input_q_t *slot;

    /*
     * Function must be called with DMA interrupt temporarily disabled
     */

    /*
     * If busy, need to handle buffer completion
     */
    if(dmx->dma_input_busy)
    {
        /*
         * Stop DMA
         */
        *((volatile u_int32 *)DMA_CONTROL_REG_CH(dmx->dma_input_channel)) &= ~DMA_ENABLE;

        /*
         * Iff we're finished with the current request, call callback handler
         * and remove it from the active Q.
         */
        if(!dmx->dma_bytes_left)
        {
            /*
             * Call async notify function to indicate we're done with the buffer.
             */
            slot = dmx->dma_input_q_head;
            dmx->dma_input_notify(dmx->DemuxID, slot->tag, slot->buffer, slot->size);

            /*
             * Remove entry from the queue
             */
            dmx->dma_input_q_head = slot->next;
            if(!dmx->dma_input_q_head)
            {
                dmx->dma_input_q_tail = NULL;
            }

            /*
             * Add entry to free list
             */
            slot->next = dmx->dma_input_q_free;
            dmx->dma_input_q_free = slot;
        }
    }

    /*
     * If active == FALSE, a halt is requested, so stop here
     */
    if((!dmx->dma_input_active) && (!dmx->dma_bytes_left))
    {
        /*
         * Indicate not busy, and kick the semaphore to wake up the client
         */
        dmx->dma_current_size       = 0;
        dmx->dma_current_buffer_ptr = NULL;
        dmx->dma_bytes_left         = 0;
        dmx->dma_input_busy         = FALSE;
        sem_put(dmx->dma_input_sem);
        return;
    }

    /*
     * More data? If so, start the next DMA operation
     */
    if(dmx->dma_input_q_head)
    {
        /*
         * Get next buffer pointer and size (left) to do
         */
        if(dmx->dma_bytes_left)
        {
            /*
             * If the last request was multi-transaction (i.e. > 64KB), get
             * next part.
             */
            dmx->dma_current_buffer_ptr += dmx->dma_current_size;
            dmx->dma_current_size        = dmx->dma_bytes_left;
        }
        else
        {
            /*
             * Get next Q entry
             */
            dmx->dma_current_buffer_ptr = dmx->dma_input_q_head->buffer;
            dmx->dma_current_size       = dmx->dma_input_q_head->size;
        }

        /*
         * Do only 64KB at a time and adjust bytes_left accordingly for next time
         */
        if(dmx->dma_current_size > DMX_DMA_MAX_DMA_SIZE)
        {
            dmx->dma_bytes_left   = dmx->dma_current_size - DMX_DMA_MAX_DMA_SIZE;
            dmx->dma_current_size = DMX_DMA_MAX_DMA_SIZE;
        }
        else
        {
            dmx->dma_bytes_left   = 0;
        }

        /*
         * Determine maximum DMA per-transfer size.  Maximum is 16bytes.
         */
        if(!(dmx->dma_current_size & 0xF))
        {
            dma_xfer_size = 4;             /* 4 words, 16 bytes */
        }
        else if(!(dmx->dma_current_size & 0x7))
        {
            dma_xfer_size = 2;             /* 2 words, 8 bytes */
        }
        else
        {
            dma_xfer_size = 1;             /* 1 word, 4 bytes */
        }

        /*
         * Set channel attributes
         */
        *((volatile u_int32 *)DMA_CONTROL_REG_CH(dmx->dma_input_channel)) =
            DMA_CHANNEL_RECOVERY_TIME(DMX_DMA_INPUT_RECOVERY_TIME) | /* Recvry Time after DMA Op */
            DMA_XACT_SZ(dma_xfer_size)                             | /* Number of 32bit words    */
            DMA_SELECT_REQ(dmx->dma_req_sel)                       | /* DMA req line for channel */
            DMA_XFER_MODE_SINGLE                                   | /* Req line triggers xfer   */
            DMA_PREEMPT                                            | /* May preempt other DMA    */
            DMA_INT_ENABLE;                                          /* Enable interrups         */

        /*
         * Set DMA Source Base Address to address of DMA input buffer and indicate
         * to DMA state machine that it should increment the address
         */
        *((volatile u_int32 *)DMA_SRCBASE_REG_CH(dmx->dma_input_channel)) =
            ((u_int32)dmx->dma_current_buffer_ptr | DMA_SRCINCR);

        /*
         * Set DMA Current Source Address to address of DMA input buffer
         */
        *((volatile u_int32 *)DMA_SRCADDR_REG_CH(dmx->dma_input_channel)) =
            (u_int32)dmx->dma_current_buffer_ptr;

        /*
         * Set Buffer size to be the amount of data to DMA.  Once this is
         * complete, an interrupt (SRC_EMPTY) will be generated.
         */
        *((volatile u_int32 *)DMA_SRCBUF_REG_CH(dmx->dma_input_channel)) =
            dmx->dma_current_size;

        /*
         * Set DMA Destination Base Address to Base of Demux TS_In buffer and indicate
         * that the DMA state machine should increment the address (acts like memory
         * even though it really is a port to a FIFO)
         */
        *((volatile u_int32 *)DMA_DSTBASE_REG_CH(dmx->dma_input_channel)) =
            ((dmx->dma_ts_in_buffer & 0xfffffffc) | DMA_SRCINCR);

        /*
         * Set DMA Destination Current Address to Base of Demux TS_In buffer
         */
        *((volatile u_int32 *)DMA_DSTADDR_REG_CH(dmx->dma_input_channel)) =
            (dmx->dma_ts_in_buffer & 0xfffffffc);

        /*
         * Set characteristics of DMA Destination Buffer (interrupt interval and
         * destination buffer size).  The "size" of the TS_In buffer is 128bytes
         * but it acts like a port with a 1MB address space.
         *
         * NOTE:  We set the interval and size to 0 since we don't want interrupts
         *        from the destination side.  As long as no source buffer is
         *        larger than the destination aperture, this works.  This means
         *        in this case, no source buffer can be larger than 1MB.
         */
        *((volatile u_int32 *)DMA_DSTBUF_REG_CH(dmx->dma_input_channel)) = 0;

        /*
         * Set busy indicator
         */
        dmx->dma_input_busy = TRUE;

        /*
         * Start DMA
         */
        *((volatile u_int32 *)DMA_CONTROL_REG_CH(dmx->dma_input_channel)) |= DMA_ENABLE;
    }
    else
    {
        /*
         * Set idle indicator
         */
        dmx->dma_current_size       = 0;
        dmx->dma_current_buffer_ptr = NULL;
        dmx->dma_bytes_left         = 0;
        dmx->dma_input_busy         = FALSE;
    }
}

/********************************************************************/
/*  dmx_dma_input_halt                                              */
/*                                                                  */
/*  PARAMETERS:                                                     */
/*      dmx               - DemuxInfo ptr                           */
/*      immediate         - Stop immediately or wait until current  */
/*                          buffer is done.                         */
/*                                                                  */
/*  DESCRIPTION:                                                    */
/*      Deactivate DMA input.                                       */
/*                                                                  */
/*  CONTEXT:                                                        */
/*      Must be called with device interrupts disabled.             */
/*                                                                  */
/*  RETURNS:                                                        */
/*      TRUE   -- Caller must wait on semaphore                     */
/*      FALSE  -- Caller must not wait on semaphore                 */
/********************************************************************/
static bool dmx_dma_input_halt(DemuxInfo *dmx, bool immediate)
{
    /*
     * Function must be called with DMA interrupt temporarily disabled
     */

    /*
     * If DMA is busy...
     */
    if(dmx->dma_input_busy)
    {
        /*
         * If an immediate halt is requested, stop the DMA in its tracks
         */
        if(immediate)
        {
            /*
             * Stop DMA
             */
            *((volatile u_int32 *)DMA_CONTROL_REG_CH(dmx->dma_input_channel)) =
                *((volatile u_int32 *)DMA_CONTROL_REG_CH(dmx->dma_input_channel)) & ~DMA_ENABLE;

            /*
             * Wait for handshake (this is extremely quick, yes this ok to do here)
             */
            while(*((volatile u_int32 *)DMA_STATUS_REG_CH(dmx->dma_input_channel)) & DMA_ACTIVE)
                ;

            /*
             * Set idle indicator
             */
            dmx->dma_input_busy = FALSE;

            /*
             * DMA is idle
             */
            return (FALSE);
        }
        else
        {
            /*
             * No immediate halt (finish current buffer), and busy.  Indicate to the
             * caller that it must wait on the semaphore.  The semaphore is triggered
             * from the physio routine (from the ISR) when it sees active == FALSE.
             */
            return (TRUE);
        }
    }
    else
    {
        /*
         * DMA is idle
         */
        return (FALSE);
    }
}

/********************************************************************/
/*  dmx_dma_input_init                                              */
/*                                                                  */
/*  PARAMETERS:                                                     */
/*      dmx               - DemuxInfo ptr                           */
/*                                                                  */
/*  DESCRIPTION:                                                    */
/*      Initialize DMA input.                                       */
/*                                                                  */
/*  CONTEXT:                                                        */
/*      Task Context Only                                           */
/*                                                                  */
/*  RETURNS:                                                        */
/*      DMX_STATUS_OK     - Success                                 */
/*      DMX_ERROR         - Error                                   */
/********************************************************************/
static DMX_STATUS dmx_dma_input_init(DemuxInfo *dmx)
{
    /*
     * Ensure we have a valid DMA channel before continuing
     */
    if(dmx->dma_input_channel == 0xffffffff)
    {
        trace_new(DPS_ERROR_MSG, "DEMUX:dmx_dma_input_init: No DMA channel defined!\n");
        return (DMX_ERROR);
    }

    /*
     * Ensure we have a valid asynchronous notification function before continuing
     */
    if(!dmx->dma_input_notify)
    {
        trace_new(DPS_ERROR_MSG, "DEMUX:dmx_dma_input_init: No async notify_fct defined!\n");
        return (DMX_ERROR);
    }

    /*
     * Allocate a semaphore used for synchronizing a halt
     */
    dmx->dma_input_sem = sem_create(0, "DXDA");
    if(!dmx->dma_input_sem)
    {
        trace_new(DPS_ERROR_MSG, "Failed to allocate DMX DMA sem!\n");
        return (DMX_ERROR);
    }

    /*
     * Hook interrupt handler (if not already hooked)
     */
    if(!gDemuxDriverInfo.dma_input_init)
    {
        /*
         * One-Time driver initialization
         */
        if(int_register_isr(INT_DMA,
                            (PFNISR)dmx_dma_input_isr,
                            FALSE,
                            FALSE,
                            &gDemuxDriverInfo.dma_input_intr_chain) != RC_OK)
        {
            trace_new(DPS_ERROR_MSG, "Failed to hook DMX DMA ISR!\n");
            sem_delete(dmx->dma_input_sem);
            return (DMX_ERROR);
        }

        if (int_enable(INT_DMA) != RC_OK)
        {
            trace_new(DPS_ERROR_MSG, "Failed to internable DMX DMA ISR!\n");
            sem_delete(dmx->dma_input_sem);
            return (DMX_ERROR);
        }

        /*
         * Program the DMA Mode Register -- Disable Multiple Concurrent DMAs
         * and Bus Locking, enable Buffered Mode
         */
        *((volatile u_int32 *)DMA_MODE_REG) = DMA_BUFFERED;

        /*
         * Indicate ISR is active
         */
        gDemuxDriverInfo.dma_input_init = TRUE;
    }

    /*
     * Get the correct request line
     */
    switch(dmx->DemuxID)
    {
        case 0:
            dmx->dma_req_sel       = DMX_DMA_INPUT_SP0_TSIN_REQUEST_LINE;
            dmx->dma_ts_in_buffer  = DMX_DMA_INPUT_SP0_TSIN_BUFFER_ADDR;
            break;
        case 2:
            dmx->dma_req_sel       = DMX_DMA_INPUT_SP2_TSIN_REQUEST_LINE;
            dmx->dma_ts_in_buffer  = DMX_DMA_INPUT_SP2_TSIN_BUFFER_ADDR;
            break;
        case 3:
            dmx->dma_req_sel       = DMX_DMA_INPUT_SP3_TSIN_REQUEST_LINE;
            dmx->dma_ts_in_buffer  = DMX_DMA_INPUT_SP3_TSIN_BUFFER_ADDR;
            break;
    }

    return (DMX_STATUS_OK);
}

/********************************************************************/
/*  dmx_dma_input_isr                                               */
/*                                                                  */
/*  PARAMETERS:                                                     */
/*      dwIntID           - Interrupt ID                            */
/*      bFIQ              - TRUE for FIQ, FALSE otherwise           */
/*      pfnChain          - Next handler if not handled here        */
/*                                                                  */
/*  DESCRIPTION:                                                    */
/*      Demux input DMA interrupt handler.                          */
/*                                                                  */
/*  CONTEXT:                                                        */
/*      Called from FLIH                                            */
/*                                                                  */
/*  RETURNS:                                                        */
/*      RC_ISR_HANDLED    - Interrupt handled                       */
/*      RC_ISR_NOTHANDLED - Interrupt not handled, call next func   */
/********************************************************************/
static int dmx_dma_input_isr(u_int32 dwIntID, bool bFIQ, PFNISR *pfnChain)
{
    DemuxInfo *dmx;
    u_int32 i, intr_status, channel_status;

    /*
     * Read DMA interrupt status
     */
    intr_status = *((volatile u_int32 *)DMA_INT_REG) & gDemuxDriverInfo.dma_input_intr_mask;

    /*
     * Check to see if it's ours or not
     */
    if ( intr_status == 0 )
    {
        /*
         * Not for us pass it on down the chain...
         */
        *pfnChain = gDemuxDriverInfo.dma_input_intr_chain;
        return (RC_ISR_NOTHANDLED);
    }

    /*
     * Handle interrupt
     */
    while(intr_status)
    {
        /*
         * Get the correct DemuxInfo ptr
         */
        for(i=0; i<MAX_DEMUX_UNITS; i++)
        {
            if((1 << gDemuxInfo[i].dma_input_channel) & intr_status)
            {
                dmx = &gDemuxInfo[i];
                break;
            }
        }

        /*
         * Read the channel status register
         */
        channel_status = *((volatile u_int32 *)DMA_STATUS_REG_CH(dmx->dma_input_channel));

        /*
         * Handle interrupt condition, including error conditions
         */
        if(channel_status & DMA_SRC_EMPTY)
        {
            /*
             * Last DMA completed, schedule the next one
             */
            dmx_dma_input_physio(dmx);
        }

        if((channel_status & DMA_READ_ERROR) ||
           (channel_status & DMA_WRITE_ERROR))
        {
            /*
             * Increment error count for debugging
             */
            ++dmx->num_dma_input_errors;
            isr_trace_new(DPS_ERROR_MSG, "TS_In DMA I/O error!\n", 0, 0);
        }
    
        /*
         * Clear the interrupt source (bits 1-6)
         */
        *((volatile u_int32 *)DMA_STATUS_REG_CH(dmx->dma_input_channel)) =
            (DMA_READ_ERROR   | DMA_WRITE_ERROR | DMA_DST_INTERVAL |
             DMA_SRC_INTERVAL | DMA_DST_FULL    | DMA_SRC_EMPTY);

        /*
         * Indicate this intr has been handled
         */
        intr_status &= ~(1<<dmx->dma_input_channel);
    }

    /*
     * Indicate that we handled the interrupt
     */
    return (RC_ISR_HANDLED);
}
/* INTERNAL_END */

/****************************************************************************
 * Modifications:
 * $Log: 
 *  11   mpeg      1.10        6/15/04 5:15:40 PM     Tim White       CR(s) 
 *        9475 9474 : Updated function descriptions for documentation.
 *        
 *  10   mpeg      1.9         6/15/04 11:43:34 AM    Joe Kroesche    CR(s) 
 *        9474 9475 : updated function comments to improve automated 
 *        documentation generation
 *  9    mpeg      1.8         6/11/04 1:33:32 PM     Tim White       CR(s) 
 *        9445 9446 : Do not OR in the DemuxID into the channel_intr_mask.  
 *        This will cause problems trying to
 *        use pawser2 or pawser3.
 *        
 *  8    mpeg      1.7         5/18/04 2:45:19 PM     Tim White       CR(s) 
 *        9240 9239 : When a halt is issued followed by more DMA to the demux, 
 *        it is possible left-over
 *        data from a previous DMA request can be sent to the demux.  The 
 *        bytes_left and
 *        current_bytes fields were not being reset correctly.
 *        
 *  7    mpeg      1.6         4/29/04 9:36:32 AM     Larry Wang      CR(s) 
 *        9000 9233 : Mask interrupt status before handle DMA finish interrupt.
 *  6    mpeg      1.5         3/19/04 2:36:03 PM     Tim Ross        CR(s) 
 *        8545 : Added cnxt_dmx_set_dma_inpit_endianess() call to allow user to
 *         input either
 *        big endian or little endian data.
 *  5    mpeg      1.4         3/18/04 10:38:46 AM    Tim White       CR(s) 
 *        8591 : Allow use of >64KB buffers to the 
 *        cnxt_dmx_dma_input_add_buffer() function.  The
 *        DMA function internally handles breaking up the DMA transaction into 
 *        sub 64KB sections
 *        and only delivers the interrupt at the end.  Halt is supported only 
 *        on request
 *        boundaries.
 *        
 *  4    mpeg      1.3         3/16/04 3:35:05 PM     Tim White       CR(s) 
 *        8545 : Moved cnxt_dmx_set_input_device() to demuxdma.  Added function
 *        cnxt_dmx_get_timebase_offset() to demuxapi.  Fixed some trace in
 *        functions which can be called under ISR to use isr_trace_new().
 *        
 *  3    mpeg      1.2         12/7/03 6:22:35 PM     Tim White       CR(s) 
 *        8113 : Allow any word aligned buffer to work with the Demux DMA Input
 *         Extension.
 *        
 *  2    mpeg      1.1         11/25/03 3:53:15 PM    Tim White       CR(s): 
 *        8027 Drop Demux DMA Input Extension function.
 *        
 *  1    mpeg      1.0         11/19/03 10:10:17 AM   Tim White       CR(s): 
 *        7987 Demux DMA Input Extension.
 *        
 * $
 ****************************************************************************/

