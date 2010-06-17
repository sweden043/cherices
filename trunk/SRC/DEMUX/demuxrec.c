/***************************************************************************
*                   CONEXANT PROPRIETARY AND CONFIDENTIAL                  *
*                       SOFTWARE FILE/MODULE HEADER                        *
*                      Conexant Systems Inc. (c) 2004                      *
*                                Austin, TX                                *
*                           All Rights Reserved                            *
****************************************************************************
*
* Filename: demuxrec.c
*
* Description: Demux PVR Record Extensions 
*
* Author: Tim Ross
*
****************************************************************************
* $Id: demuxrec.c,v 1.7, 2004-06-15 22:27:13Z, Tim White$
****************************************************************************/


/****************
* Include Files *
*****************/
#include "stbcfg.h"
#include "retcodes.h"
#include "kal.h"
#include "demuxapi.h"
#include "demuxint.h"

/**************
* Definitions *
***************/
#if PARSER_TYPE == DVB_PARSER
   #define PACKET_SIZE     188
#elif PARSER_TYPE == DTV_PARSER
   #define PACKET_SIZE     130
#else
#error Do not understand PARSER_TYPE value specified!!!
#endif

#define EVENT_SIZE         16

/***********
* Typedefs *
************/

/*******************
* Global Variables *       
********************/

/******************
* Local Variables *
*******************/

/**********************
* Function Prototypes *
***********************/

/***********************
* External Global Data *
************************/
extern DemuxInfo gDemuxInfo[];

/**********************
* External Prototypes *
***********************/

/****************
* API Functions *
*****************/

/******************************************************************************
* Function Name:  cnxt_dmx_set_transport_buffer 
*
* Description:
*     Set the demux transport buffer pointers and provide the demux driver
*     with a callback function and tag value.  The callback function will
*     be called after each notify_block_size bytes have been written to the
*     transport buffer or if the transport buffer overruns.
*
* Inputs:         dmxid - demux unit number
*                 buffer - pointer to first byte of transport buffer
*                 size - Size of transport buffer in bytes. The transport
*                        buffer must be:
*                  <p>a) aligned on a 32-bit boundary,
*                  <p>b) an integral multiple of the transport packet size
*                   (188 bytes for DVB, 130 bytes for DSS), and 
*                  <p>c) an integral multiple less than 63 of notify_block_size. 
*                 tag - 32-bit value to be returned to the client when
*                       notify_function() is called.
*                 notify_block_size - number of bytes demux should write before calling 
*                    notify_function callback
*                 notify_function - callback function to be called in
*                    interrupt context by the demux each time notify_block_size
*                    bytes have been written to the transport buffer or if
*                    the transport buffer overruns.
*
* Outputs:        None.
*
* Return Values:  DMX_STATUS_OK - success
*                 DMX_BAD_DMXID - the demux ID provided is invalid
*                 DMX_ERROR - an internal error occurred
*
* Notes: 
*
* Context:        Callable from interrupt or non-interrupt context.
*
*****************************************************************************/
DMX_STATUS cnxt_dmx_set_transport_buffer(
   u_int32              dmxid, 
   u_int8               *buffer,
   u_int32              size, 
   u_int32              tag,
   u_int32              notify_block_size,
   dmx_rec_notify_fct_t notify_function)
{
   DemuxInfo   *dmx;
   u_int32     num_packets, bf;

   isr_trace_new(DPS_FUNC_TRACE, "DEMUX:cnxt_dmx_set_transport_buffer: entered\n", 0, 0);

   /*
    * Sanity check dmxid value, avoid demux 1 & going
    * outside array bounds in next check
    */
   if(dmxid >= MAX_DEMUX_UNITS || dmxid == 1)
   {
      isr_trace_new(DPS_ERROR_MSG,
         "DEMUX:cnxt_dmx_set_transport_buffer: demux %d does not exist!\n", 
         dmxid, 0);
      return DMX_BAD_DMXID;
   }

   /*
    * First check to see that this demux id is already allocated
    */
   if (!gDemuxInfo[dmxid].DemuxInitialized)
   {
      isr_trace_new(DPS_ERROR_MSG,
         "DEMUX:cnxt_dmx_set_transport_buffer: demux %d is not initialized!\n", 
         dmxid, 0);
      return DMX_BAD_DMXID;
   }

   dmx = &gDemuxInfo[dmxid];

   /*
    * Check for valid buffer values
    */
   if(!((u_int32)buffer))
   {
      isr_trace_new(DPS_ERROR_MSG, "DEMUX:cnxt_dmx_set_transport_buffer: invalid buffer \
         pointer!\n", 0, 0);
      return DMX_ERROR;
   }
   if((u_int32)buffer & 0x3)
   {
      isr_trace_new(DPS_ERROR_MSG, "DEMUX:cnxt_dmx_set_transport_buffer: buffer \
         alignment!\n", 0, 0);
      return DMX_ERROR;
   }
   if(!size)
   {
      isr_trace_new(DPS_ERROR_MSG, "DEMUX:cnxt_dmx_set_transport_buffer: size = 0!\n", 0, 0);
      return DMX_ERROR;
   }
   num_packets = size / PACKET_SIZE;
   if((num_packets * PACKET_SIZE) != size)
   {
      isr_trace_new(DPS_ERROR_MSG, 
         "DEMUX:cnxt_dmx_set_transport_buffer: buffer size !/ 188!\n", 0, 0);
      return DMX_ERROR;
   }
   if(!notify_block_size)
   {
      isr_trace_new(DPS_ERROR_MSG, 
         "DEMUX:cnxt_dmx_set_transport_buffer: bad blocking factor!\n", 0, 0);
      return DMX_ERROR;
   }
   bf = notify_block_size / PACKET_SIZE;
   if((bf>=num_packets) || (bf>63))
   {
      isr_trace_new(DPS_ERROR_MSG, 
         "DEMUX:cnxt_dmx_set_transport_buffer: bad blocking factor!\n", 0, 0);
      return DMX_ERROR;
   }
   if(!notify_function)
   {
      isr_trace_new(DPS_ERROR_MSG, 
         "DEMUX:cnxt_dmx_set_transport_buffer: must supply notify function!\n", 0, 0);
      return DMX_ERROR;
   }

   /*
    * Save data
    */
   dmx->rec_buffer_start_addr          = buffer;
   dmx->rec_buffer_end_addr            = buffer + size;
   dmx->rec_buffer_read_ptr            = buffer;
   dmx->rec_buffer_write_ptr           = buffer;
   dmx->rec_buffer_notify_block_size   = notify_block_size;
   dmx->rec_buffer_tag                 = tag;
   dmx->rec_buffer_notify              = notify_function;

   /*
    * Set buffer pointers
    */
   *DPS_TRANSPORT_START_ADDR_EX(dmxid) = (((u_int32)dmx->rec_buffer_start_addr & ~NCR_BASE) |
                                          DPS_PAW_SYS_ADDRESS);
   *DPS_TRANSPORT_END_ADDR_EX(dmxid)   = (((u_int32)dmx->rec_buffer_end_addr   & ~NCR_BASE) |
                                          DPS_PAW_SYS_ADDRESS);
   *DPS_TRANSPORT_READ_PTR_EX(dmxid)   = (((u_int32)dmx->rec_buffer_read_ptr   & ~NCR_BASE) |
                                          DPS_PAW_SYS_ADDRESS);
   *DPS_TRANSPORT_WRITE_PTR_EX(dmxid)  = (((u_int32)dmx->rec_buffer_write_ptr  & ~NCR_BASE) |
                                          DPS_PAW_SYS_ADDRESS);

   /*
    * Set blocking factor.
    */
   *DPS_HOST_CTL_REG_EX(dmxid) &= ~DPS_TRANSPORT_BLOCKS_MASK;
   *DPS_HOST_CTL_REG_EX(dmxid) |= (bf << DPS_TRANSPORT_BLOCKS_SHIFT);

   /*
    * Reset blocking factor
    */
   *DPS_TS_BLOCK_COUNT_EX(dmxid) = 0;

   /*
    * Force correct endian-ness of output data...
    */
   *DPS_PARSER_CTL_REG_EX(dmxid) |= DPS_OUTPUT_BYTESWAP_LITTLE_ENDIAN;

   return DMX_STATUS_OK;
}

/******************************************************************************
* Function Name:  cnxt_dmx_get_transport_buffer_ptrs 
*
* Description:    Get transport buffer start, end, read, and write pointers.  
*
* Inputs:         dmxid - demux unit number
*
* Outputs:
*           start -     Pointer to first byte of transport buffer.
*           end   -     Pointer to first byte beyond the end of the transport
*                       buffer.
*           read  -     Pointer to the next byte of the transport buffer that
*                       will be read by the record driver.
*           write -     Pointer to the next byte of the transport buffer that
*                       will be written by the demux.
*
* Return Values:  DMX_STATUS_OK - success
*                 DMX_BAD_DMXID - the demux ID provided is invalid
*                 DMX_ERROR - an internal error occurred
*
* Notes: 
*
* Context:        Callable from interrupt or non-interrupt context.
*
*****************************************************************************/
DMX_STATUS cnxt_dmx_get_transport_buffer_ptrs(
   u_int32  dmxid,
   u_int8   **start, 
   u_int8   **end,
   u_int8   **read, 
   u_int8   **write)
{
   DemuxInfo *dmx;

   isr_trace_new(DPS_FUNC_TRACE, 
      "DEMUX:cnxt_dmx_get_transport_buffer_ptrs: entered\n", 0, 0);

   /*
    * Sanity check dmxid value, avoid demux 1 & going
    * outside array bounds in next check
    */
   if(dmxid >= MAX_DEMUX_UNITS || dmxid == 1)
   {
      isr_trace_new(DPS_ERROR_MSG,
         "DEMUX:cnxt_dmx_get_transport_buffer_ptrs: demux %d does not exist!\n", 
         dmxid, 0);
      return DMX_BAD_DMXID;
   }

   /*
    * First check to see that this demux id is already allocated
    */
   if (!gDemuxInfo[dmxid].DemuxInitialized)
   {
      isr_trace_new(DPS_ERROR_MSG,
         "DEMUX:cnxt_dmx_get_transport_buffer_ptrs: demux %d is not initialized!\n", 
         dmxid, 0);
      return DMX_BAD_DMXID;
   }
   
   /*
    * Check for NULL ptrs
    */
   if (!((u_int32)start) || !((u_int32)end) || 
       !((u_int32)read)  || !((u_int32)write))
   {
      isr_trace_new(DPS_ERROR_MSG, 
         "DEMUX:cnxt_dmx_get_transport_buffer_ptrs: invalid pointer for storing output!\n",
         0, 0);
      return DMX_ERROR;
   }

   dmx = &gDemuxInfo[dmxid];

   /*
    * Get buffer pointers
    */
   dmx->rec_buffer_start_addr = (u_int8 *)((*DPS_TRANSPORT_START_ADDR_EX(dmxid) & 
                                           ~DPS_PAW_SYS_ADDRESS) | NCR_BASE);
   dmx->rec_buffer_end_addr   = (u_int8 *)((*DPS_TRANSPORT_END_ADDR_EX(dmxid) & 
                                           ~DPS_PAW_SYS_ADDRESS) | NCR_BASE);
   dmx->rec_buffer_read_ptr   = (u_int8 *)((*DPS_TRANSPORT_READ_PTR_EX(dmxid) & 
                                           ~DPS_PAW_SYS_ADDRESS) | NCR_BASE);
   dmx->rec_buffer_write_ptr  = (u_int8 *)((*DPS_TRANSPORT_WRITE_PTR_EX(dmxid) & 
                                           ~DPS_PAW_SYS_ADDRESS) | NCR_BASE);
                               
   *start   = dmx->rec_buffer_start_addr;
   *end     = dmx->rec_buffer_end_addr;
   *read    = dmx->rec_buffer_read_ptr;
   *write   = dmx->rec_buffer_write_ptr;

   return DMX_STATUS_OK;
}

/******************************************************************************
* Function Name:  cnxt_dmx_set_event_buffer 
*
* Description:
*                 Set the demux event buffer pointers and provide the demux
*                 driver with a callback function and tag value.  The callback
*                 function will be called after one or more events have
*                 been written to the event buffer or if the event buffer
*                 overruns.
*
* Inputs:
*                 dmxid -     Demux unit number.
*                 buffer -    Pointer to first byte of event buffer.
*                 size -      Size of event buffer in bytes. The event buffer
*                             must be aligned on a 32-bit boundary and an
*                             integral multiple of 16 bytes (the size of
*                             each event). 
*                 tag -       A 32-bit value to be returned to the client
*                             when notify_function() is called.
*                 notify_function - A callback function that will be called
*                             in interrupt context by the demux each time
*                             one or more events have been written to the
*                             event buffer, or if the event buffer overruns.
*
* Outputs:        None.
*
* Return Values:  DMX_STATUS_OK - success
*                 DMX_BAD_DMXID - the demux ID provided is invalid
*                 DMX_ERROR - an internal error occurred
*
* Notes: 
*
* Context:        Callable from interrupt or non-interrupt context.
*
*****************************************************************************/
DMX_STATUS cnxt_dmx_set_event_buffer(
   u_int32              dmxid, 
   u_int8               *buffer,
   u_int32              size, 
   u_int32              tag,
   dmx_rec_notify_fct_t notify_function)
{
   DemuxInfo   *dmx;
   u_int32     num_events;

   isr_trace_new(DPS_FUNC_TRACE, 
      "DEMUX:cnxt_dmx_set_event_buffer: entered\n", 0, 0);

   /*
    * Sanity check dmxid value, avoid demux 1 & going
    * outside array bounds in next check
    */
   if(dmxid >= MAX_DEMUX_UNITS || dmxid == 1)
   {
      isr_trace_new(DPS_ERROR_MSG,
         "DEMUX:cnxt_dmx_set_event_buffer: demux %d does not exist!\n", dmxid, 0);
      return DMX_BAD_DMXID;
   }

   /*
    * First check to see that this demux id is already allocated
    */
   if (!gDemuxInfo[dmxid].DemuxInitialized)
   {
      isr_trace_new(DPS_ERROR_MSG,
         "DEMUX:cnxt_dmx_set_event_buffer: demux %d is not initialized!\n", dmxid, 0);
      return DMX_BAD_DMXID;
   }

   dmx = &gDemuxInfo[dmxid];

   /*
    * Check for valid buffer values
    */
   if(!((u_int32)buffer))
   {
      isr_trace_new(DPS_ERROR_MSG, "DEMUX:cnxt_dmx_set_event_buffer: invalid buffer \
         pointer!\n", 0, 0);
      return DMX_ERROR;
   }
   if((u_int32)buffer & 0x3)
   {
      isr_trace_new(DPS_ERROR_MSG, "DEMUX:cnxt_dmx_set_event_buffer: buffer \
         alignment!\n", 0, 0);
      return DMX_ERROR;
   }
   if(!size)
   {
      isr_trace_new(DPS_ERROR_MSG, "DEMUX:cnxt_dmx_set_event_buffer: size = 0!\n", 0, 0);
      return DMX_ERROR;
   }
   num_events = size / EVENT_SIZE;
   if((num_events * EVENT_SIZE) != size)
   {
      isr_trace_new(DPS_ERROR_MSG, 
         "DEMUX:cnxt_dmx_set_event_buffer: buffer size !/ EVENT_SIZE!\n", 0, 0);
      return DMX_ERROR;
   }
   if(!notify_function)
   {
      isr_trace_new(DPS_ERROR_MSG, 
         "DEMUX:cnxt_dmx_set_event_buffer: must supply notify function!\n", 0, 0);
      return DMX_ERROR;
   }

   /*
    * Save data
    */
   dmx->rec_event_buffer_start_addr          = buffer;
   dmx->rec_event_buffer_end_addr            = buffer + size;
   dmx->rec_event_buffer_read_ptr            = buffer;
   dmx->rec_event_buffer_write_ptr           = buffer;
   dmx->rec_event_buffer_tag                 = tag;
   dmx->rec_event_buffer_notify              = notify_function;

   /*
    * Set buffer pointers
    */
   *DPS_REC_EVENT_START_ADDR_EX(dmxid) = (((u_int32)dmx->rec_event_buffer_start_addr 
                                          & ~NCR_BASE) | DPS_PAW_SYS_ADDRESS);
   *DPS_REC_EVENT_END_ADDR_EX(dmxid)   = (((u_int32)dmx->rec_event_buffer_end_addr   
                                          & ~NCR_BASE) | DPS_PAW_SYS_ADDRESS);
   *DPS_REC_EVENT_READ_PTR_EX(dmxid)   = (((u_int32)dmx->rec_event_buffer_read_ptr   
                                          & ~NCR_BASE) | DPS_PAW_SYS_ADDRESS);
   *DPS_REC_EVENT_WRITE_PTR_EX(dmxid)  = (((u_int32)dmx->rec_event_buffer_write_ptr  
                                          & ~NCR_BASE) | DPS_PAW_SYS_ADDRESS);

   return DMX_STATUS_OK;
}

/******************************************************************************
* Function Name:  cnxt_dmx_get_event_buffer_ptrs 
*
* Description:    Get the record event buffer start, end, read, and write  
*                 pointers.                                            
*
* Inputs:         dmxid - demux unit number
*
* Outputs:        start - Pointer to first byte of event buffer.
*                 end - Pointer to first byte beyond the end of the event buffer.
*                 read - Pointer to the next byte of the event buffer that
*                       will be read by the record driver.
*                 write - Pointer to the next byte of the event buffer that
*                       will be written by the demux
*
* Return Values:  DMX_STATUS_OK - success
*                 DMX_BAD_DMXID - the demux ID provided is invalid
*                 DMX_ERROR - an internal error occurred
*
* Notes: 
*
* Context:        Callable from interrupt or non-interrupt context.
*
*****************************************************************************/
DMX_STATUS cnxt_dmx_get_event_buffer_ptrs(
   u_int32  dmxid,
   u_int8   **start, 
   u_int8   **end,
   u_int8   **read, 
   u_int8   **write)
{
   DemuxInfo *dmx;

   isr_trace_new(DPS_FUNC_TRACE, 
      "DEMUX:cnxt_dmx_get_event_buffer_ptrs: entered\n", 0, 0);

   /*
    * Sanity check dmxid value, avoid demux 1 & going
    * outside array bounds in next check
    */
   if(dmxid >= MAX_DEMUX_UNITS || dmxid == 1)
   {
      isr_trace_new(DPS_ERROR_MSG,
         "DEMUX:cnxt_dmx_get_event_buffer_ptrs: demux %d does not exist!\n", 
         dmxid, 0);
      return DMX_BAD_DMXID;
   }

   /*
    * First check to see that this demux id is already allocated
    */
   if (!gDemuxInfo[dmxid].DemuxInitialized)
   {
      isr_trace_new(DPS_ERROR_MSG,
         "DEMUX:cnxt_dmx_get_event_buffer_ptrs: demux %d is not initialized!\n", 
         dmxid, 0);
      return DMX_BAD_DMXID;
   }

   /*
    * Check for NULL ptrs
    */
   if (!((u_int32)start) || !((u_int32)end) || 
       !((u_int32)read)  || !((u_int32)write))
   {
      isr_trace_new(DPS_ERROR_MSG, 
         "DEMUX:cnxt_dmx_get_event_buffer_ptrs: invalid pointer for storing output!\n",
         0, 0);
      return DMX_ERROR;
   }

   dmx = &gDemuxInfo[dmxid];

   /*
    * Get buffer pointers
    */
   dmx->rec_event_buffer_start_addr = (u_int8 *)((*DPS_REC_EVENT_START_ADDR_EX(dmxid) & 
                                      ~DPS_PAW_SYS_ADDRESS) | NCR_BASE);
   dmx->rec_event_buffer_end_addr   = (u_int8 *)((*DPS_REC_EVENT_END_ADDR_EX(dmxid) & 
                                      ~DPS_PAW_SYS_ADDRESS) | NCR_BASE);
   dmx->rec_event_buffer_read_ptr   = (u_int8 *)((*DPS_REC_EVENT_READ_PTR_EX(dmxid) & 
                                      ~DPS_PAW_SYS_ADDRESS) | NCR_BASE);
   dmx->rec_event_buffer_write_ptr  = (u_int8 *)((*DPS_REC_EVENT_WRITE_PTR_EX(dmxid) & 
                                      ~DPS_PAW_SYS_ADDRESS) | NCR_BASE);
                               
   *start   = dmx->rec_event_buffer_start_addr;
   *end     = dmx->rec_event_buffer_end_addr;
   *read    = dmx->rec_event_buffer_read_ptr;
   *write   = dmx->rec_event_buffer_write_ptr;

   return DMX_STATUS_OK;
}

/******************************************************************************
* Function Name:  cnxt_dmx_transport_bytes_recorded 
*
* Description:    Indicate to demux the number of transport bytes that       
*                 have been read by the client so that the demux can advance 
*                 its event buffer read pointer.                             
*
* Inputs:         dmxid - demux unit number
*                 bytes - Number of transport bytes that have been read
*                       (recorded).  This value cannot be greater than the
*                       transport buffer size.
*
* Outputs:        None.
*
* Return Values:  DMX_STATUS_OK - success
*                 DMX_BAD_DMXID - the demux ID provided is invalid
*                 DMX_ERROR - an internal error occurred
*
* Notes:          This should be called once the data has been removed
*                 from the transport buffer and stored to disk. 
*
* Context:        Callable from interrupt or non-interrupt context
*
*****************************************************************************/
DMX_STATUS cnxt_dmx_transport_bytes_recorded(u_int32 dmxid, u_int32 bytes)
{
   DemuxInfo   *dmx;

   isr_trace_new(DPS_FUNC_TRACE, "DEMUX:cnxt_dmx_data_recorded: entered\n", 0, 0);

   /*
    * Sanity check dmxid value, avoid demux 1 & going
    * outside array bounds in next check
    */
   if(dmxid >= MAX_DEMUX_UNITS || dmxid == 1)
   {
      isr_trace_new(DPS_ERROR_MSG,
         "DEMUX:cnxt_dmx_data_recorded: demux %d does not exist!\n", dmxid, 0);
      return DMX_BAD_DMXID;
   }

   /*
    * First check to see that this demux id is already allocated
    */
   if (!gDemuxInfo[dmxid].DemuxInitialized)
   {
      isr_trace_new(DPS_ERROR_MSG,
         "DEMUX:cnxt_dmx_data_recorded: demux %d is not initialized!\n", dmxid, 0);
      return DMX_BAD_DMXID;
   }
   
   dmx = &gDemuxInfo[dmxid];

   /*
    * Check for valid block count
    */
   if (bytes > (dmx->rec_buffer_start_addr - dmx->rec_buffer_end_addr))
   {
      isr_trace_new(DPS_ERROR_MSG,
         "DEMUX:cnxt_dmx_transport_blocks_recorded: invalid byte count!\n", 0, 0);
      return DMX_ERROR;
   }
      
   /* 
    * Advance the read ptr by the number of bytes read.
    * 
    * NOTE: It's too difficult to check for valid positioning of read ptr 
    * relative to write ptr because pawser could move the
    * write ptr causing it to wrap while we are advancing the 
    * read ptr.
    */
   dmx->rec_buffer_read_ptr += bytes;
   if (dmx->rec_buffer_read_ptr >= dmx->rec_buffer_end_addr)
   {
      dmx->rec_buffer_read_ptr = dmx->rec_buffer_start_addr + (dmx->rec_buffer_read_ptr 
                                  - dmx->rec_buffer_end_addr);
   }
   *DPS_TRANSPORT_READ_PTR_EX(dmxid)  = (((u_int32)dmx->rec_buffer_read_ptr  & ~NCR_BASE) |
                                         DPS_PAW_SYS_ADDRESS);

   return DMX_STATUS_OK;
}

/******************************************************************************
* Function Name:  cnxt_dmx_events_recorded 
*
* Description:    Indicate to demux the number of transport stream event bytes
*                 that have been read by the client so that the demux can     
*                 advance its event buffer read pointer.                      
*
* Inputs:         dmxid - demux unit number
*                 bytes - Number of event bytes that have been read (recorded).
*                       This value cannot be greater than the total size of
*                       the event buffer.
*
* Outputs:        None.
*
* Return Values:  DMX_STATUS_OK - success
*                 DMX_BAD_DMXID - the demux ID provided is invalid
*                 DMX_ERROR - an internal error occurred
*
* Notes:          This should be called once the user has read event data
*                 from the event buffer and stored it to disk.
*
* Context:        Callable from interrupt or non-interrupt context
*
*****************************************************************************/
DMX_STATUS cnxt_dmx_events_recorded(u_int32 dmxid, u_int32 bytes)
{
   DemuxInfo   *dmx;

   isr_trace_new(DPS_FUNC_TRACE, "DEMUX:cnxt_dmx_events_recorded: entered\n", 0, 0);

   /*
    * Sanity check dmxid value, avoid demux 1 & going
    * outside array bounds in next check
    */
   if(dmxid >= MAX_DEMUX_UNITS || dmxid == 1)
   {
      isr_trace_new(DPS_ERROR_MSG,
         "DEMUX:cnxt_dmx_events_recorded: demux %d does not exist!\n", dmxid, 0);
      return DMX_BAD_DMXID;
   }

   /*
    * First check to see that this demux id is already allocated
    */
   if (!gDemuxInfo[dmxid].DemuxInitialized)
   {
      isr_trace_new(DPS_ERROR_MSG,
         "DEMUX:cnxt_dmx_events_recorded: demux %d is not initialized!\n", dmxid, 0);
      return DMX_BAD_DMXID;
   }
   
   dmx = &gDemuxInfo[dmxid];

   /*
    * Check for valid block count
    */
   if (bytes > (dmx->rec_event_buffer_start_addr - dmx->rec_event_buffer_end_addr))
   {
      isr_trace_new(DPS_ERROR_MSG,
         "DEMUX:cnxt_dmx_events_recorded: invalid byte count!\n", 0, 0);
      return DMX_ERROR;
   }
      
   /* 
    * Advance the read ptr by the number of bytes read.
    * 
    * NOTE: It's too difficult to check for valid positioning of read ptr 
    * relative to write ptr because pawser could move the
    * write ptr causing it to wrap while we are advancing the 
    * read ptr.
    */
   dmx->rec_event_buffer_read_ptr += bytes;
   if (dmx->rec_event_buffer_read_ptr >= dmx->rec_event_buffer_end_addr)
   {
      dmx->rec_event_buffer_read_ptr = dmx->rec_event_buffer_start_addr + 
                                        (dmx->rec_event_buffer_read_ptr 
                                        - dmx->rec_event_buffer_end_addr);
   }
   *DPS_REC_EVENT_READ_PTR_EX(dmxid)  = (((u_int32)dmx->rec_event_buffer_read_ptr  
                                        & ~NCR_BASE) | DPS_PAW_SYS_ADDRESS);

   return DMX_STATUS_OK;
}

/******************
* Local Functions *
******************/

/****************************************************************************
 * Modifications:
 * $Log: 
 *  8    mpeg      1.7         6/15/04 5:27:13 PM     Tim White       CR(s) 
 *        9474 9475 : Changed pvr to rec for consistency.
 *        
 *  7    mpeg      1.6         6/15/04 11:43:58 AM    Joe Kroesche    CR(s) 
 *        9474 9475 : updated function comments to improve automated 
 *        documentation generation
 *  6    mpeg      1.5         5/14/04 4:14:00 PM     Tim Ross        CR(s) 
 *        9205 9206 : Changed cnxt_dmx_events_recorded(), 
 *        cnxt_dmx_transport_bytes_recorded(),  to be callable from interrupt 
 *        context.
 *  5    mpeg      1.4         3/19/04 2:35:23 PM     Tim Ross        CR(s) 
 *        8545 : Changed endianess set during cnxt_dmx_set_transport_buffer() 
 *        from big to
 *        little.
 *  4    mpeg      1.3         3/17/04 2:52:15 PM     Tim Ross        CR(s) 
 *        8545 : Corrected incorrect NULL ptr check in get_buffer_ptrs 
 *        functions.
 *  3    mpeg      1.2         3/10/04 1:26:49 PM     Tim Ross        CR(s) 
 *        8545 : Corrected compilation errors for PVR record demux code. 
 *  2    mpeg      1.1         2/24/04 4:55:08 PM     Tim Ross        CR(s) 
 *        8451 : Initial Drop
 *  1    mpeg      1.0         2/24/04 9:04:55 AM     Tim Ross        CR(s) 
 *        8451 : Demux PVR Record Extensions
 * $
 ****************************************************************************/

