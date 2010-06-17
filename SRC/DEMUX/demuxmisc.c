/****************************************************************************/
/*                   CONEXANT PROPRIETARY AND CONFIDENTIAL                  */
/*                       SOFTWARE FILE/MODULE HEADER                        */
/*          Conexant Systems Inc. (c) 1998, 1999, 2000, 2001, 2002          */
/*                              Austin, TX                                  */
/*                         All Rights Reserved                              */
/****************************************************************************/
/*
 * Filename:       demuxmisc.c
 *
 *
 * Description:    Miscellaneous demux routines
 *
 *
 * Author:         Bob Van Gulick 
 *
 ****************************************************************************/
/* $Id: demuxmisc.c,v 1.34, 2004-06-16 16:13:34Z, Tim White$
 ****************************************************************************/

#include "stbcfg.h"
#include "basetype.h"
#include "globals.h"
#include "kal.h"
#include "startup.h"
#include "retcodes.h"
#include "demuxapi.h"
#include "demuxint.h"
#include "clkrec.h"

int GenDmxTSMaxBlocksToCopy = 0;

#ifdef TRACK_PSI_PACKET_LOSS
    #define TRACK_PSI_PACKET_LOSS_FREQ 1000
u_int32 PSIBufferFullDropCount = 0;
u_int32 PSICRCDropCount        = 0;
u_int32 PSIPacketLossDropCount = 0;
u_int32 PSIFilterDropCount     = 0;
u_int32 PSISyntaxDropCount     = 0;
tick_id_t PSIPacketLossTimer;
#endif

#if PARSER_TYPE==DTV_PARSER
u_int8 CWP_Buffer[CWP_BUFFER_SIZE];
u_int8 CAP_Buffer[CAP_BUFFER_SIZE];
#endif

#if (CANAL_AUDIO_HANDLING == YES)
u_int32 gdwVideoPktsReceived = 0;
u_int32 gdwAudioPktsReceived = 0;
extern void cnxt_audio_drop_out(void);
#endif /* CANAL_AUDIO_HANDLING */

/* Semaphores */
extern sem_id_t PSI_Sem_ID;
#if PARSER_TYPE==DTV_PARSER
extern sem_id_t CWP_Sem_ID;
extern sem_id_t CAP_Sem_ID;
#endif
extern sem_id_t Xprt_Sem_ID;
extern sem_id_t GenDmxPSITaskProcSem;
extern sem_id_t GenDmxPESTaskProcSem;

/* Task ID's */
task_id_t PSIProcessID;
task_id_t CWP_ProcessID;
task_id_t CAP_ProcessID;
task_id_t PESProcessID;
task_id_t XprtProcessID;

bool Dump = 0;

extern u_int32 PSI_int_mask;
extern u_int32 CWP_int_mask;
extern u_int32 CAP_int_mask;
extern u_int32 PES_int_mask;
extern u_int32 Xprt_int_mask;
extern bool NewPCRFlag;

extern DemuxInfo gDemuxInfo[];

extern gen_pcr_callback_fct_t gPCRCallBack;
extern gen_event6_callback_fct_t gEvent6CallBack;

#ifdef DRIVER_INCL_CLKREC
extern void clk_rec_PCRNotifyRegister(clk_rec_PCRNotify_t handler);
#endif /* DRIVER_INCL_CLKREC */
void DemuxIRQPCRNotify(u_int32 PCR_High, u_int32 PCR_Low);
extern void FCopy(char *pDest, char *pSrc, u_int32 len);

#if defined DRIVER_INCL_NDSDEMUX || defined DRIVER_INCL_NDSICAM
void NDS_IRQ_Handler(u_int32 dmxid);
#endif

#ifdef DEBUG
u_int8 *pXprtBuff;
u_int32 XprtSize;
u_int32 XprtIndex = 0;

void demux_set_xprt_buffer(void *buffer, size_t size) {
    pXprtBuff = (u_int8 *) buffer;
    XprtSize = (u_int32) size;
}
#endif

#if DMXDMA==YES
DemuxDriverInfo gDemuxDriverInfo;
#endif

/* Routine to calculate the bit position of a given bitmask */
int Calc_Bit_Pos(u_int32 bitmask) {
    int bitnum,mask;

    bitnum = 0;
    mask = bitmask;
    while (((mask & 1) != 1) && bitnum<32) {
        mask >>= 1;
        bitnum++;
    }
    if (bitnum>32) {
        return -1;  /* Should never get here */
    }
    return bitnum;
}

/*-------------------------------------------------------------------------------------------**
** genDemuxInitSW                                                                            **
** Params:                                                                                   **
**         None                                                                              **
** Desc.:                                                                                    **
**         Software initialization including tasks, queus, interrupts and structures.        **
** Return Value:                                                                             **
**         TRUE OK                                                                           **
**         FLASE Init Failed                                                                 **
**-------------------------------------------------------------------------------------------*/
bool genDemuxInitSW(u_int32 dmxid) {
    u_int32 i, j;
    static bool FirstTime = TRUE;
    DemuxInfo *dmx = &gDemuxInfo[dmxid];

/*
 * This is required for setting up the NIM Extender when building with
 * CUSTOMER == VENDOR_B on a Conexant box (e.g. Klondike)
 */
#if CUSTOMER == VENDOR_B
    static BOOL demod_init = FALSE;

/* When running on a Conexant Box initialize the demod driver */
    if (dmx->demod_init == FALSE) {
      dmx->demod_init = TRUE;
      SetNIMExtenderKlondike();
    }
#endif

#if DMXDMA==YES
    gDemuxDriverInfo.dma_input_init       = FALSE;
    gDemuxDriverInfo.dma_input_intr_mask  = 0;
    gDemuxDriverInfo.dma_input_intr_chain = (PFNISR) 0;
#endif

    trace_new(DPS_FUNC_TRACE,"DEMUX: genDemuxInitSW Called.\n");
#ifdef DRIVER_INCL_CLKREC
    clk_rec_PCRNotifyRegister(DemuxIRQPCRNotify);
#endif /* DRIVER_INCL_CLKREC */

    dmx->NonDescChannelsAvail= TOTAL_CHANNELS - DPS_NUM_DESCRAMBLERS - 2; /* '2' for the video & audio channels*/
    dmx->DescChannelsAvail   = DPS_NUM_DESCRAMBLERS;
    dmx->AvailableFilters    = TOTAL_FILTERS;
    dmx->GenDmxTSMaxBlocksToCopy = 0;
    dmx->SyncErrorCount = 0;
    dmx->gInSyncCount = 0;
    dmx->pOldfnHandler = 0;
    dmx->HWFiltering = 1;
    dmx->DemodConnected = FALSE;
    dmx->DemuxInitialized = FALSE;
    dmx->PSILocalBufferFull = FALSE;
    dmx->TransportBufferFull = FALSE;
    dmx->gParserInSync = FALSE;
    dmx->NotifyCRC = FALSE;
    dmx->PCR_PID_Value = GEN_NULL_PID;
    dmx->video_pts_offset = 0;
    dmx->video_timebase_offset = 0;
    dmx->audio_pts_offset = 0;
    dmx->audio_timebase_offset = 0;

#if PARSER_TYPE==DTV_PARSER
    /* CWP parameters */
    dmx->CWPInfo.CWPTimerActive = FALSE;
    dmx->CWPInfo.CWPBufferFull = FALSE;
    dmx->CWPInfo.CWP_Overflows = 0;
    dmx->CWPInfo.CWPTimeOutMS = 0;
    dmx->CWPInfo.CWPTimerNotifyCount = 0;
    dmx->CWPInfo.CWPDataCount = 0;
    dmx->CWPInfo.CWPMaxBlocksToCopy = 0;
   dmx->CWPInfo.pCWPBuffer = 
       dmx->CWPInfo.pCWPWritePtr = 
       dmx->CWPInfo.pCWPReadPtr = &CWP_Buffer[0];
    dmx->CWPInfo.pCWPBufferEnd = &CWP_Buffer[CWP_BUFFER_SIZE];

    /* CAP parameters */
    dmx->CAPInfo.CAPTimerActive = FALSE;
    dmx->CAPInfo.CAPBufferFull = FALSE;
    dmx->CAPInfo.CAP_Overflows = 0;
    dmx->CAPInfo.CAPTimeOutMS = 0;
    dmx->CAPInfo.CAPTimerNotifyCount = 0;
    dmx->CAPInfo.CAPDataCount = 0;
    dmx->CAPInfo.CAPMaxBlocksToCopy = 0;
    dmx->CAPInfo.CAP_scid = 0;
    dmx->CAPInfo.CAP_filter = 0;
    dmx->CAPInfo.CAPEnable = FALSE;
     dmx->CAPInfo.pCAPBuffer = 
       dmx->CAPInfo.pCAPWritePtr = 
       dmx->CAPInfo.pCAPReadPtr = &CAP_Buffer[0];
    dmx->CAPInfo.pCAPBufferEnd = &CAP_Buffer[CAP_BUFFER_SIZE];
#endif

#if ( (PVR==YES) || (XTV_SUPPORT==YES) )
    dmx->rec_buffer_start_addr          = 0;
    dmx->rec_buffer_end_addr            = 0;
    dmx->rec_buffer_read_ptr            = 0;
    dmx->rec_buffer_write_ptr           = 0;
    dmx->rec_buffer_notify_block_size   = 0;
    dmx->rec_buffer_tag                 = 0;
    dmx->rec_buffer_notify              = (dmx_rec_notify_fct_t) 0;
#endif

#if DMXDMA==YES
    dmx->dma_input_selected          = FALSE;
    dmx->dma_input_init              = FALSE;
    dmx->dma_input_active            = FALSE;
    dmx->dma_input_busy              = FALSE;
    dmx->dma_input_channel           = 0xffffffff;
    dmx->num_dma_input_errors        = 0;
    dmx->dma_input_notify            = (dmx_dma_input_notify_fct_t) 0;
    dmx->dma_input_q_head            = (dmx_dma_input_q_t *) 0;
    dmx->dma_input_q_tail            = (dmx_dma_input_q_t *) 0;
    dmx->dma_input_q_free            = &dmx->dma_input_q[0];
    dmx->dma_input_sem               = (sem_id_t) 0;
    dmx->dma_ts_in_buffer            = 0;
    dmx->dma_req_sel                 = 0;
    dmx->dma_current_size            = 0;
    dmx->dma_current_buffer_ptr      = NULL;
    dmx->dma_bytes_left              = 0;
    for(i=0; i<(DMX_DMA_INPUT_Q_SIZE-1); i++)
    {
        dmx->dma_input_q[i].next = &dmx->dma_input_q[i+1];
    }
    dmx->dma_input_q[DMX_DMA_INPUT_Q_SIZE-1].next = NULL;
#endif

    for (i = 0; i < TOTAL_CHANNELS; ++i) {
        dmx->ChannelInfoTable[i].PID = (u_int16) GENDMX_BAD_CHANNEL;
        dmx->ChannelInfoTable[i].Slot = i;
        if (i == VIDEO_CHANNEL)
            dmx->ChannelInfoTable[i].stype = VIDEO_PES_TYPE;
        else if (i == AUDIO_CHANNEL)
            dmx->ChannelInfoTable[i].stype = AUDIO_PES_TYPE;
        else
            dmx->ChannelInfoTable[i].stype = PSI_CHANNEL_TYPE;
        dmx->ChannelInfoTable[i].NotifyData.pData = NULL;
        dmx->ChannelInfoTable[i].NotifyData.condition= 0;
        dmx->ChannelInfoTable[i].NotifyData.chid = i;
        dmx->ChannelInfoTable[i].NotifyData.write_ptr = NULL;
        dmx->ChannelInfoTable[i].NotifyData.skip = 0;
        dmx->ChannelInfoTable[i].NotifyData.length = 0;
        dmx->ChannelInfoTable[i].NotifyData.tag = 0;
        dmx->ChannelInfoTable[i].FilterEnable = 0x00000000;
        dmx->ChannelInfoTable[i].FiltersAllocated = 0x00000000;
        dmx->ChannelInfoTable[i].DisabledByFilter = FALSE;
        dmx->ChannelInfoTable[i].InUseFlag = CHANNEL_FREE;
        dmx->ChannelInfoTable[i].DemuxEnable = GEN_DEMUX_DISABLE;
        dmx->ChannelInfoTable[i].Overflows = 0;
        dmx->ChannelInfoTable[i].OverflowsHandled = 0;
        dmx->ChannelInfoTable[i].pBuffer = 0;
        dmx->ChannelInfoTable[i].pBufferEnd = 0;
        dmx->ChannelInfoTable[i].pWritePtr = 0;
        dmx->ChannelInfoTable[i].pReadPtr = 0;
        dmx->ChannelInfoTable[i].pAckPtr = 0;
        dmx->ChannelInfoTable[i].HdrErrNotify = NULL;
        dmx->ChannelInfoTable[i].DataNotify = NULL;
        dmx->ChannelInfoTable[i].HdrSize = 0;
        dmx->ChannelInfoTable[i].HdrArea = NULL;
        dmx->ChannelInfoTable[i].HdrAlloc = NULL;
        dmx->ChannelInfoTable[i].ChannelTimer = 0;
        dmx->ChannelInfoTable[i].NotifyCount = 0;
        dmx->ChannelInfoTable[i].PESChannel = FALSE;
        dmx->ChannelInfoTable[i].PESDataCount = 0;
        dmx->ChannelInfoTable[i].CurrentFilterId = GENDMX_BAD_FILTER;
        dmx->ChannelInfoTable[i].LoopMode = GENDEMUX_CONTINUOUS;
        dmx->ChannelInfoTable[i].TimeOutMS = 0;      /* no time out */
        dmx->ChannelInfoTable[i].TimerNotifyCount = 0;
        dmx->ChannelInfoTable[i].Notify_Unlock = 0;
        dmx->ChannelInfoTable[i].NotifyCalled  = 0;
        dmx->ChannelInfoTable[i].bRecording = FALSE;

        dmx->FilterTable[i].OwnerChid = GENDMX_BAD_CHANNEL;
        dmx->FilterTable[i].FilterSize = 0;
        dmx->FilterTable[i].FilterEnabled = GEN_DEMUX_DISABLE;
        dmx->FilterTable[i].ExtFilterEnabled = GEN_DEMUX_DISABLE;
        dmx->FilterTable[i].ExtFilterSize = 0;
        dmx->FilterTable[i].ExtFilterOffset = 0;
        dmx->FilterTable[i].VersionEqual = TRUE;

        for (j = 0; j < (GENDMX_MAX_HW_FILTER_SIZE/4); ++j) {
            dmx->FilterTable[i].Match[j] = 0;
            dmx->FilterTable[i].Mask[j] = 0;
            dmx->FilterTable[i].NotMask[j] = 0;
            dmx->FilterTable[i].NotMaskZero = TRUE;
        }
        for (j = 0; j < (GENDMX_EXT_FILTER_SIZE/4); ++j) {
            dmx->FilterTable[i].ExtMatch[j] = 0;
            dmx->FilterTable[i].ExtMask[j] = 0;
        }
        dmx->FilterTable[i].ExtFilterOffset = 0;

    }

    if (FirstTime) {
    
        /* Clear our callback function pointers */
        gPCRCallBack = NULL;
        gEvent6CallBack = NULL;
        
        /* Create the tasks and semaphores */
        if ((PSI_Sem_ID = sem_create(0,"PSISEM")) == 0) {
            trace_new(DPS_ERROR_MSG,"DEMUX:PSI Sem Create Failed\n");
            error_log(RC_DPS_SCREATE_FAILED + ERROR_FATAL);
            return FALSE;
        }
#if PARSER_TYPE==DTV_PARSER
        if ((CWP_Sem_ID = sem_create(0,"CWPSEM")) == 0) {
            trace_new(DPS_ERROR_MSG,"DEMUX:CWP Sem Create Failed\n");
            error_log(RC_DPS_SCREATE_FAILED + ERROR_FATAL);
            return FALSE;
        }
        if ((CAP_Sem_ID = sem_create(0,"CAPSEM")) == 0) {
            trace_new(DPS_ERROR_MSG,"DEMUX:CAP Sem Create Failed\n");
            error_log(RC_DPS_SCREATE_FAILED + ERROR_FATAL);
            return FALSE;
        }
#endif
        if ((Xprt_Sem_ID = sem_create(0,"XPTSEM")) == 0) {
            trace_new(DPS_ERROR_MSG,"DEMUX:XPRT Sem Create Failed\n");
            error_log(RC_DPS_SCREATE_FAILED + ERROR_FATAL);
            return FALSE;
        }
        if ((GenDmxPSITaskProcSem = sem_create(1,"PSIX")) == 0) {
            trace_new(DPS_ERROR_MSG,"DEMUX:GenDmxPSITaskProcSem Create Failed\n");
            error_log(RC_DPS_SCREATE_FAILED + ERROR_FATAL);
            return FALSE;
        }

        if ((GenDmxPESTaskProcSem = sem_create(1,"PESX")) == 0) {
            trace_new(DPS_ERROR_MSG,"DEMUX:GenDmxPESTaskProcSem Create Failed\n");
            error_log(RC_DPS_SCREATE_FAILED + ERROR_FATAL);
            return FALSE;
        }

	 if ((PSIProcessID = task_create(gen_dmx_PSI_Task, 0, 0, PSI_TASK_STACK_SIZE, PSI_TASK_PRIORITY, PSI_TASK_NAME)) == 0) {
	 	trace_new(DPS_ERROR_MSG,"DEMUX:PSI Process Create Failed\n");
             error_log(RC_DPS_PCREATE_FAILED + ERROR_FATAL);
             return FALSE;
        }

        if ((XprtProcessID = task_create(gen_dmx_Xprt_Task, 0, 0, TPT_TASK_STACK_SIZE, TPT_TASK_PRIORITY, TPT_TASK_NAME)) == 0) {
            trace_new(DPS_ERROR_MSG,"DEMUX:Process Create Failed\n");
            error_log(RC_DPS_PCREATE_FAILED + ERROR_FATAL);
            return FALSE;
        }

#if PARSER_TYPE==DTV_PARSER
        if ((CWP_ProcessID = task_create(gen_dmx_CWP_Task, 0, 0, CWP_TASK_STACK_SIZE, CWP_TASK_PRIORITY, CWP_TASK_NAME)) == 0) {
            trace_new(DPS_ERROR_MSG,"DEMUX:CWP Process Create Failed\n");
            error_log(RC_DPS_PCREATE_FAILED + ERROR_FATAL);
            return FALSE;
        }
        if ((CAP_ProcessID = task_create(gen_dmx_CAP_Task, 0, 0, CAP_TASK_STACK_SIZE, CAP_TASK_PRIORITY, CAP_TASK_NAME)) == 0) {
            trace_new(DPS_ERROR_MSG,"DEMUX:CAP Process Create Failed\n");
            error_log(RC_DPS_PCREATE_FAILED + ERROR_FATAL);
            return FALSE;
        }
#endif
#ifdef TRACK_PSI_PACKET_LOSS
        PSIPacketLossTimer = tick_create((PFNTIMERC) gen_demux_psi_packet_loss_timer_call_back,
                                         NULL, (const char *) "PSZX");
        tick_set(PSIPacketLossTimer, TRACK_PSI_PACKET_LOSS_FREQ, FALSE);
        tick_start(PSIPacketLossTimer);
#endif
        FirstTime = FALSE;
    }

    /*
     * If ISR has not yet been registered for this demux, do it now
     */
    if(!dmx->dmx_interrupt_bit)
    {
        switch(dmx->DemuxID)
        {
#if MAX_DEMUX_UNITS > 1
            case 3:
                dmx->DemuxIRQHandler   = (PFNISR)DemuxIRQHandler3;
                dmx->dmx_interrupt_bit = Calc_Bit_Pos(ITC_PAR3);
                break;
            case 2:
                dmx->DemuxIRQHandler   = (PFNISR)DemuxIRQHandler2;
                dmx->dmx_interrupt_bit = Calc_Bit_Pos(ITC_PAR2);
                break;
#endif
            case 0:
            default:
                dmx->DemuxIRQHandler   = (PFNISR)DemuxIRQHandler0;
                dmx->dmx_interrupt_bit = Calc_Bit_Pos(ITC_PAR0);
                break;
        }

        /********************************************/
        /* Register/Enable the ISR - done per demux */
        /********************************************/
        if (int_register_isr(dmx->dmx_interrupt_bit, dmx->DemuxIRQHandler, FALSE, FALSE, 
                             (PFNISR *)&dmx->pOldfnHandler) != RC_OK)
        {
            trace_new(DPS_ERROR_MSG,"DEMUX:Error: int_register_isr failed\n");
            error_log(RC_DPS_REGISR_FAILED + ERROR_FATAL);
            return FALSE;
        }

        if (int_enable(dmx->dmx_interrupt_bit) != RC_OK)
        {
            trace_new(DPS_ERROR_MSG,"DEMUX:Error: int_enable failed\n");
            error_log(RC_DPS_REGISR_FAILED + ERROR_FATAL);
            return FALSE;
        }
    }

    return TRUE;
}

/*---------------------------------------------------------------**
** DemuxIRQHandler0,2 & 3                                        **
** Params:                                                       **
**         IntID: Interrupt ID                                   **
**         Fiq  : FIQ or IRQ (always IRQ)                        **
**         pfnChain : Pointer to last ISR installed.             **
** Desc.:                                                        **
**         Shell interrupt handler for each demux unit.  It will **
**         call DemuxIRQHandler to perform mainline interrupt    **
**         handling.                                             **
**                                                               **
** Returns:                                                      **
**         RC_ISR_NOTHANDLED : IRQ not handled here.             **
**         RC_ISR_HANDLED    : IRQ handled here                  **
**---------------------------------------------------------------*/

int DemuxIRQHandler0(u_int32 IntID, bool Fiq, PFNISR *pfnChain) {
    return DemuxIRQHandler(IntID, Fiq, pfnChain, 0);
}

int DemuxIRQHandler2(u_int32 IntID, bool Fiq, PFNISR *pfnChain) {
    return DemuxIRQHandler(IntID, Fiq, pfnChain, 2);
}

int DemuxIRQHandler3(u_int32 IntID, bool Fiq, PFNISR *pfnChain) {
    return DemuxIRQHandler(IntID, Fiq, pfnChain, 3);
}

/*---------------------------------------------------------------**
** DemuxIRQHandler                                               **
** Params:                                                       **
**         IntID: Interrupt ID                                   **
**         Fiq  : FIQ or IRQ (always IRQ)                        **
**         pfnChain : Pointer to last ISR installed.             **
**         dmxid: demux id to process interrupts on              **
** Desc.:                                                        **
**         Handles all the Demux related interrupts.             **
**                                                               **
** Returns:                                                      **
**         RC_ISR_NOTHANDLED : IRQ not handled here.             **
**         RC_ISR_HANDLED    : IRQ handled here                  **
**---------------------------------------------------------------*/
int DemuxIRQHandler(u_int32 IntID, bool Fiq, PFNISR *pfnChain, u_int32 dmxid) {
    u_int32 IRQReason, EventReason = 0;
    u_int32 reg_value;
    DemuxInfo *dmx = &gDemuxInfo[dmxid];

#if defined DRIVER_INCL_NDSDEMUX || defined DRIVER_INCL_NDSICAM
    NDS_IRQ_Handler(dmxid);
#endif

    IRQReason = *DPS_ISR_REG_EX(dmxid) & DEMUX_ISR_BITS;

    if (IRQReason & DPS_BITSTREAM_IN_FULL) {
        isr_trace_new(DPS_ERROR_MSG,"DEMUXISR: BITSTREAM_IN_FULL TSInFree=%d\n", 
                      *DPS_TS_IN_BUFFER_FREE_EX(dmxid),0);

        *DPS_INT_ENABLE_REG_EX(dmxid) &= ~DPS_BITSTREAM_IN_FULL;
    }

    if (IRQReason & DPS_BITSTREAM_OUT_FULL) {
        *DPS_INT_ENABLE_REG_EX(dmxid) &= ~DPS_BITSTREAM_OUT_FULL;
        isr_trace_new(DPS_ERROR_MSG,"DEMUXISR: BITSTREAM_OUT_FULL interrupt?\n", 0, 0);
    }

    if (IRQReason & DPS_EVENT_INTERRUPT) {
        EventReason = *DPS_EVENT_STATUS_REG_EX(dmxid) & *DPS_EVENT_ENABLE_REG_EX(dmxid);

#if PARSER_TYPE==DTV_PARSER
        /* Process CWP packets if block stored or buffer full */
        if ((EventReason & DPS_CWP_BLOCK_STORED) || 
            (EventReason & DPS_CWP_BUFFER_FULL)) {

            /* Set the bit mask so the task knows we have work for this demux */
            CWP_int_mask |= 1<<dmxid;
            /* Now simply signal to the CWP task */
            sem_put(CWP_Sem_ID);
        }

        /* Process CAP packets if block stored or buffer full */
        if ((EventReason & DPS_CAP_BLOCK_STORED) || 
            (EventReason & DPS_CAP_BUFFER_FULL)) {

            /* Set the bit mask so the task knows we have work for this demux */
            CAP_int_mask |= 1<<dmxid;
            /* Now simply signal to the CAP task */
            sem_put(CAP_Sem_ID);
        }
#endif

        /* Process packets if block stored or buffer full */
        if ((EventReason & DPS_PSI_BLOCK_STORED) || 
            (EventReason & DPS_PSI_BUFFER_FULL)) {

            /* Set the bit mask so the task knows we have work for this demux */
            PSI_int_mask |= 1<<dmxid;
            /* Now simply signal to the PSI task */
            sem_put(PSI_Sem_ID);
        }

#if (LEGACY_DVR==NO)
        if (EventReason & DPS_VIDEO_SPLICED) {
            /* This is "Event 6" so call any registered handler */
            if(gEvent6CallBack)
              gEvent6CallBack();
              
            *DPS_HOST_CTL_REG_EX(dmxid) &= ~DPS_SPLICE_VIDEO;
        }
        if (EventReason & DPS_AUDIO_SPLICED) {
            *DPS_HOST_CTL_REG_EX(dmxid) &= ~DPS_SPLICE_AUDIO;
        }
#endif

#if PARSER_TYPE==DTV_PARSER
        if (EventReason & DPS_CWP_BUFFER_FULL) {
            ++dmx->CWPInfo.CWP_Overflows;
            isr_trace_new(DPS_ISR_MSG,"DEMUXISR: CWP Buffer FULL!!!\n",0, 0);
        }
        if (EventReason & DPS_CAP_BUFFER_FULL) {
            ++dmx->CAPInfo.CAP_Overflows;
            isr_trace_new(DPS_ISR_MSG,"DEMUXISR: CAP Buffer FULL!!!\n",0, 0);
        }
#endif

        if (EventReason & DPS_PSI_BUFFER_FULL) {
            reg_value = (u_int32)*DPS_PSI_OVERFLOW_INDEX_EX(dmxid);
            ++dmx->ChannelInfoTable[reg_value].Overflows;
            isr_trace_new(DPS_ISR_MSG,"DEMUXISR: PSI Buffer FULL!!! 0x%08x\n", reg_value, 0);
        }

#if (LEGACY_DVR==YES)
        if (EventReason & DPS_ALT_TRANSPORT_BUFFER_FULL) {
            isr_trace("DEMUXISR: TS FULL TSRead=0x%x, TSWrite=0x%x\n", 
                      (u_int32) *DPS_TRANSPORT_READ_PTR_EX(dmxid),
                      (u_int32) *DPS_TRANSPORT_WRITE_PTR_EX(dmxid));

            *DPS_EVENT_STATUS_REG_EX(dmxid) &= ~DPS_ALT_TRANSPORT_BUFFER_FULL;

            dmx->TransportBufferFull = TRUE;
            Xprt_int_mask |= 1<<dmxid;
            sem_put(Xprt_Sem_ID);
        }
        if (EventReason & DPS_ALT_TRANSPORT_BLOCK_STORED) {
            Xprt_int_mask |= 1<<dmxid;
            sem_put(Xprt_Sem_ID);
        }
#else
  #if ( (PVR==YES) || (XTV_SUPPORT==YES) )
        if(dmx->CapabilitiesRequested & DMX_CAP_RECORD)
        {
            if (EventReason & DPS_TRANSPORT_BUFFER_FULL)
            {
                isr_trace("DEMUXISR: TS FULL TSRead=0x%x, TSWrite=0x%x\n",
                          (u_int32) *DPS_TRANSPORT_READ_PTR_EX(dmxid),
                          (u_int32) *DPS_TRANSPORT_WRITE_PTR_EX(dmxid));

//                /*
//                 * Acknowledge interrupt from microcode
//                 */
//                *DPS_EVENT_STATUS_REG_EX(dmxid) &= ~DPS_TRANSPORT_BUFFER_FULL;
//
                /*
                 * Call PVR Record Handler
                 */
                if(dmx->rec_buffer_notify)
                {
                    dmx->rec_buffer_notify(dmxid, dmx->rec_buffer_tag, DMX_REC_TS_BUFFER_FULL);
                }
            }
            if (EventReason & DPS_TRANSPORT_BLOCK_STORED)
            {
                /*
                 * Call PVR Record Handler
                 */
                if(dmx->rec_buffer_notify)
                {
                    dmx->rec_buffer_notify(dmxid, dmx->rec_buffer_tag, DMX_REC_TS_BUFFER_INTR);
                }
            }
            if (EventReason & DPS_REC_EVENT_FULL)
            {
                isr_trace("DEMUXISR: PVR EVENT FULL Read=0x%x, Write=0x%x\n",
                          (u_int32) *DPS_REC_EVENT_READ_PTR_EX(dmxid),
                          (u_int32) *DPS_REC_EVENT_WRITE_PTR_EX(dmxid));

//                /*
//                 * Acknowledge interrupt from microcode
//                 */
//                *DPS_EVENT_STATUS_REG_EX(dmxid) &= ~DPS_REC_EVENT_FULL;
//
//#if 0
                /*
                 * Call PVR Record Event Handler
                 */
                if(dmx->rec_event_buffer_notify)
                {
                    dmx->rec_event_buffer_notify(dmxid,
                                                 dmx->rec_event_buffer_tag,
                                                 DMX_REC_EVT_BUFFER_FULL);
                }
//#endif
            }
            if (EventReason & DPS_REC_EVENT_INTR)
            {
//#if 0
                /*
                 * Call PVR Record Event Handler
                 */
                if(dmx->rec_event_buffer_notify)
                {
                    dmx->rec_event_buffer_notify(dmxid,
                                                 dmx->rec_event_buffer_tag,
                                                 DMX_REC_EVT_BUFFER_INTR);
                }
//#endif
            }
        }
        else
  #endif /* PVR */
        {
            if (EventReason & DPS_TRANSPORT_BUFFER_FULL) {
                isr_trace("DEMUXISR: TS FULL TSRead=0x%x, TSWrite=0x%x\n", 
                          (u_int32) *DPS_TRANSPORT_READ_PTR_EX(dmxid),
                          (u_int32) *DPS_TRANSPORT_WRITE_PTR_EX(dmxid));

                *DPS_EVENT_STATUS_REG_EX(dmxid) &= ~DPS_TRANSPORT_BUFFER_FULL;

                dmx->TransportBufferFull = TRUE;
                Xprt_int_mask |= 1<<dmxid;
                sem_put(Xprt_Sem_ID);
            }
            if (EventReason & DPS_TRANSPORT_BLOCK_STORED) {
                Xprt_int_mask |= 1<<dmxid;
                sem_put(Xprt_Sem_ID);
            }
        }
#endif /* ! LEGACY_DVR */

        if (EventReason & DPS_SYNC_ERROR) {
            if (dmx->gParserInSync == TRUE) {
                isr_trace_new(TRACE_DPS|TRACE_LEVEL_3,"DEMUXISR: PAWSER LOST SYNC!\n", 0, 0);
                dmx->gParserInSync = FALSE;
                dmx->gInSyncCount = 0;
            }
        }
        #if CANAL_AUDIO_HANDLING == YES
        if (EventReason & DPS_BAD_PES_HEADER){
	  /*isr_trace_new(TRACE_MPG|TRACE_LEVEL_3,"DEMUXISR: Bad PES header detected. Events 0x%08x, Enable 0x%08x\n", EventReason, *glpDpsEventEnable); */
            *DPS_EVENT_ENABLE_REG_EX(dmxid) &= ~DPS_BAD_PES_HEADER;
            cnxt_audio_drop_out();
        }

        if (EventReason & DPS_VIDEO_PKT_RECEIVED){
	  /*isr_trace_new(TRACE_MPG|TRACE_LEVEL_3,"DEMUXISR: Video packet received. Events 0x%08x, Enable 0x%08x\n", EventReason, *glpDpsEventEnable); */
            *DPS_EVENT_ENABLE_REG_EX(dmxid) &= ~DPS_VIDEO_PKT_RECEIVED;
            gdwVideoPktsReceived++;
        }

        if (EventReason & DPS_AUDIO_PKT_RECEIVED){
	  /*isr_trace_new(TRACE_MPG|TRACE_LEVEL_3,"DEMUXISR: Audio packet received. Events 0x%08x, Enable 0x%08x\n", EventReason, *glpDpsEventEnable); */
            *DPS_EVENT_ENABLE_REG_EX(dmxid) &= ~DPS_AUDIO_PKT_RECEIVED;
            gdwAudioPktsReceived++;
        }
        #endif /* CANAL_AUDIO_HANDLING == YES */

#if (LEGACY_DVR==YES)
        dvr_check_for_dvr_events (dmxid, EventReason);
#endif /* LEGACY_DVR */
    }

/* If the driver thinks we are out of sync, we check the bit to make sure. */
    if (dmx->gParserInSync == FALSE) {
        if (*DPS_PARSER_STATUS_REG_EX(dmxid) & DPS_IN_SYNC) {
            dmx->gInSyncCount++;
            if (dmx->gInSyncCount >= 2) {
                isr_trace_new(TRACE_DPS|TRACE_LEVEL_3,"DEMUXISR: PAWSER SYNC IS BACK\n", 0, 0);
                dmx->gParserInSync = TRUE;
            }
        }
    }

    *DPS_EVENT_STATUS_REG_EX(dmxid) = EventReason;
    *DPS_ISR_REG_EX(dmxid) = IRQReason;

    return RC_ISR_HANDLED;
}  

void ProcessTSPacket(u_int32 dmxid, u_int8 *pXprtBuffer) {
    u_int32 i, chid=0, cnt;
    u_int8 *pTmp;
#ifndef DRIVER_INCL_NDSTESTS
    u_int8 adf_control, adf_length;
    static u_int32 cc;
#endif
    u_int16 pid;
    ChannelInformationStruc *pChInfo;
    u_int32 first_word, len;
    DemuxInfo *dmx = &gDemuxInfo[dmxid];

    first_word = *((volatile u_int32 *)pXprtBuffer);

    pTmp = (u_int8 *)&first_word;

    if (*pTmp++ != 0x47) {
        trace_new(DPS_ERROR_MSG,"DEMUX:Error: Did not find 0x47 as expected\n");
        return;
    }

#ifndef DRIVER_INCL_NDSTESTS
    pXprtBuffer += 4;

    if (*pTmp & 0x80) {               /* transport_error_indicator */
        trace_new(DPS_ERROR_MSG,"DEMUX:Error: transport_error_indicator\n");
        return;
    }
#endif

    pid = (*pTmp++ & ~0xE0) << 8;
    pid |=  *pTmp++;
    pChInfo = NULL;
    for (i=0;i<TOTAL_CHANNELS;i++) {
        if (dmx->ChannelInfoTable[i].PID == pid) {
            pChInfo = &dmx->ChannelInfoTable[i];
            chid = i;
            break;
        }
    }
    if (pChInfo == NULL) {
        trace_new(DPS_ERROR_MSG,"DEMUX:Error: pid not in table.pid=%d\n", pid);
        return;
    }

#ifndef DRIVER_INCL_NDSTESTS
    adf_control = (*pTmp & 0x30) >> 4;
    cc = *pTmp++ & 0x0F;
    if (pChInfo->CCounter == cc) {
        /*trace("Skipping repeated CC=%d\n", ccounter); */
        return;
    }
    pChInfo->CCounter = cc;

    cnt = 184;
    if (adf_control == 0x02 || adf_control == 0x03) {    /* skip adaptation field */
        adf_length = *pXprtBuffer++;
        --cnt;
        if (adf_length >= cnt) {
            trace_new(DPS_ERROR_MSG,"DEMUX:Error: adf_length = %d, cnt = %d\n", adf_length, cnt);
            return;
        }
        pXprtBuffer += adf_length;
        cnt -= adf_length;
    }

    if (adf_control == 0x01 || adf_control == 0x03) 
#else
    cnt = 188;
#endif
    {
        if ((pChInfo->pWritePtr+cnt) <= pChInfo->pBufferEnd) {
           if (pChInfo->pWritePtr >= pChInfo->pReadPtr) {

        /* Check for special condition where read ptr is at buffer start and write
           ptr is one block away from buffer end.  This is a biffer full condition */

              if (((pChInfo->pWritePtr+cnt) == pChInfo->pBufferEnd) &&
                  (pChInfo->pReadPtr == pChInfo->pBuffer)) {

                  /*trace("PES buffer full, PES key data lost. W=0x%x, R=0x%x\n",
                         pChInfo->pWritePtr, pChInfo->pReadPtr);
                  */
                  return;
              } 
              FCopy((char *)pChInfo->pWritePtr, (char *)pXprtBuffer, cnt);
           } else {
              if ((pChInfo->pWritePtr+cnt) < pChInfo->pReadPtr)  {
                  FCopy((char *)pChInfo->pWritePtr, (char *)pXprtBuffer, cnt);
              } else {
                  /*trace("PES buffer full, PES key data lost. W=0x%x, R=0x%x\n",
                         pChInfo->pWritePtr, pChInfo->pReadPtr);
                  */
                  return;
              }
           }
        } else {
          if ((pChInfo->pWritePtr >= pChInfo->pReadPtr) &&
           ((pChInfo->pBuffer+(cnt-(pChInfo->pBufferEnd-pChInfo->pWritePtr))) <
             pChInfo->pReadPtr)) {
            len = pChInfo->pBufferEnd - pChInfo->pWritePtr;
            FCopy((char *)pChInfo->pWritePtr, (char *)pXprtBuffer, len);
            pXprtBuffer += len;
            len = cnt - len;
            FCopy((char *)pChInfo->pBuffer, (char *)pXprtBuffer, len);
          } else {
 
            /*trace("PES buffer full, PES data lost. W=0x%x, R=0x%x\n",
                   pChInfo->pWritePtr, pChInfo->pReadPtr);
            */
            return;
          }
        }
        AdvancePtr(&pChInfo->pWritePtr, cnt, pChInfo->pBuffer, pChInfo->pBufferEnd);
        pChInfo->PESDataCount += cnt;
    }

#ifndef DRIVER_INCL_NDSTESTS
    if (pChInfo->DataNotify) {
        pChInfo->NotifyData.pData = pChInfo->pReadPtr;
        pChInfo->NotifyData.condition = 0;
        pChInfo->NotifyData.chid = chid;
        pChInfo->NotifyData.length =  pChInfo->PESDataCount;
#ifdef MHP
        /* MHP change */
        pChInfo->LoopMode = pChInfo->DataNotify(&(pChInfo->NotifyData), 0xffffffff);
#else
        pChInfo->LoopMode = pChInfo->DataNotify(&(pChInfo->NotifyData));
#endif
        /*
         * If client indicated ONE_SHOT, disable channel
         */
        if (pChInfo->LoopMode == GENDEMUX_ONE_SHOT)
        {
            if (cnxt_dmx_channel_control(dmxid, chid, GEN_DEMUX_DISABLE) != DMX_STATUS_OK)
            {
                trace_new(DPS_ERROR_MSG,"DEMUX: cnxt_dmx_control_channel failed\n");
            }
        }
    }
#endif
}

void gen_dmx_Xprt_Task(void *arg) {
    static int BlocksToCopy, LastBlocksToCopy, bytes_to_copy, i;
    static u_int8 *SaveWrite, *SaveRead;
    bool ks;
    u_int32 dmxid;
    DemuxInfo *dmx;

    for (;;) {
        sem_get(Xprt_Sem_ID, KAL_WAIT_FOREVER);
        sem_get(GenDmxPESTaskProcSem, KAL_WAIT_FOREVER);
        /*********************************************************
         * When we get here, we need to check the Xprt demux 
         * bitmask to see which demux needs Xprt processing
         *********************************************************/
        for (dmxid=0; dmxid<MAX_DEMUX_UNITS; dmxid++) {
            if (dmxid == 1) continue;  /* Skip demux 1 for now - not used */

            /* Is there work for this demux? */
            if (Xprt_int_mask & (1<<dmxid)) {
                dmx = &gDemuxInfo[dmxid];
                SaveRead  = (u_int8*)*DPS_TRANSPORT_READ_PTR_EX(dmxid);
                SaveWrite = (u_int8*)*DPS_TRANSPORT_WRITE_PTR_EX(dmxid);
                if (SaveWrite >= SaveRead) {
                    bytes_to_copy = ( (u_int32) SaveWrite - (u_int32) SaveRead);
                    if (bytes_to_copy == 0 && dmx->TransportBufferFull) {
                        bytes_to_copy = (u_int32)*DPS_TRANSPORT_END_ADDR_EX(dmxid) - 
                                        (u_int32)*DPS_TRANSPORT_START_ADDR_EX(dmxid);
                    }
                } else {
                    bytes_to_copy = (u_int32) SaveWrite - 
                                    (u_int32)*DPS_TRANSPORT_START_ADDR_EX(dmxid);
                    bytes_to_copy += (u_int32)*DPS_TRANSPORT_END_ADDR_EX(dmxid) - 
                                     (u_int32) SaveRead + 1;
                }
                BlocksToCopy = bytes_to_copy / TS_COPY_BLOCK_SIZE;
                /*trace("BlocksToCopy = %d\n", BlocksToCopy); */
                bytes_to_copy %= TS_COPY_BLOCK_SIZE;
                LastBlocksToCopy = BlocksToCopy;
                if (BlocksToCopy > dmx->GenDmxTSMaxBlocksToCopy) {
                    dmx->GenDmxTSMaxBlocksToCopy = BlocksToCopy;
                    if (dmx->GenDmxTSMaxBlocksToCopy >= ((HWBUF_TRNSPRT_SIZE+TS_COPY_BLOCK_SIZE-1)/TS_COPY_BLOCK_SIZE)) {

                        trace_new(TRACE_LEVEL_ALWAYS|TRACE_DPS,"       -> At TS Buffer Capacity\n");
                    }
                }
                if (BlocksToCopy > 0) {
                    for (i = 0; i < BlocksToCopy; ++i) {
                        ProcessTSPacket(dmxid, (u_int8 *) (((u_int32)SaveRead+NCR_BASE) & ~DPS_PAW_SYS_ADDRESS));
                        SaveRead += TS_COPY_BLOCK_SIZE;
                        if (SaveRead >= (u_int8*)(*DPS_TRANSPORT_END_ADDR_EX(dmxid))) {
                            SaveRead = (u_int8*)(*DPS_TRANSPORT_START_ADDR_EX(dmxid));
                        }
                    }
                    *DPS_TRANSPORT_READ_PTR_EX(dmxid) = (u_int32)SaveRead;
                    SaveWrite = (u_int8*)(*DPS_TRANSPORT_WRITE_PTR_EX(dmxid));
                }
                if (dmx->TransportBufferFull == TRUE) {
                    dmx->TransportBufferFull = FALSE;
                    *DPS_EVENT_STATUS_REG_EX(dmxid) = DPS_TRANSPORT_BUFFER_FULL;
                    *DPS_EVENT_ENABLE_REG_EX(dmxid) |= DPS_TRANSPORT_BUFFER_FULL;
                }

                /* Now that we've completed Xprt processing for one demux, clear the bitmask, but */
                /* make sure the clear is in a critical section so no other interrupts could be   */
                /* missed while we are clearing the bit. */
                ks = critical_section_begin();            
                Xprt_int_mask &= ~(1<<dmxid);
                critical_section_end(ks);
            } /* if Xprt ... */
        } /* for dmxid ... */

        sem_put(GenDmxPESTaskProcSem);
    } /* for ;; ... */
}

#if PARSER_TYPE==DTV_PARSER
void ProcessCWPPacket(u_int32 dmxid, u_int8 *pCWPBuffer) {
    u_int32 cnt;
    u_int16 *pTmp;
    u_int16 scid_slot, control_sync;
    u_int32 first_word, len;
    DemuxInfo *dmx = &gDemuxInfo[dmxid];

    first_word = *((volatile u_int32 *)pCWPBuffer);

    first_word = BSWAP (first_word);

    pTmp = (u_int16 *)&first_word;

    scid_slot = *pTmp++;
    control_sync = *pTmp>>12;
        
    pCWPBuffer += 4;  /* To get past first word in packet */
    cnt = CWP_COPY_BLOCK_SIZE;

    if ((dmx->CWPInfo.pCWPWritePtr+cnt) <= dmx->CWPInfo.pCWPBufferEnd) {
        if (dmx->CWPInfo.pCWPWritePtr >= dmx->CWPInfo.pCWPReadPtr) {

        /* Check for special condition where read ptr is at buffer start and write
           ptr is one block away from buffer end.  This is a biffer full condition */

              if (((dmx->CWPInfo.pCWPWritePtr+cnt) == dmx->CWPInfo.pCWPBufferEnd) &&
                  (dmx->CWPInfo.pCWPReadPtr == dmx->CWPInfo.pCWPBuffer)) {

                  /*trace("CWP buffer full, CWP key data lost. W=0x%x, R=0x%x\n",
                         dmx->CWPInfo.pCWPWritePtr, dmx->CWPInfo.pCWPReadPtr);
                  */
                  return;
              } 
              FCopy((char *)dmx->CWPInfo.pCWPWritePtr, (char *)pCWPBuffer, cnt);
        } else {
              if ((dmx->CWPInfo.pCWPWritePtr+cnt) < dmx->CWPInfo.pCWPReadPtr)  {
                  FCopy((char *)dmx->CWPInfo.pCWPWritePtr, (char *)pCWPBuffer, cnt);
              } else {
                  /*trace("CWP buffer full, CWP key data lost. W=0x%x, R=0x%x\n",
                         dmx->CWPInfo.pCWPWritePtr, dmx->CWPInfo.pCWPReadPtr);
                  */
                  return;
              }
        }
    } else {
        if ((dmx->CWPInfo.pCWPWritePtr >= dmx->CWPInfo.pCWPReadPtr) &&
           ((dmx->CWPInfo.pCWPBuffer+(cnt-(dmx->CWPInfo.pCWPBufferEnd-dmx->CWPInfo.pCWPWritePtr))) <
             dmx->CWPInfo.pCWPReadPtr)) {
            len = dmx->CWPInfo.pCWPBufferEnd - dmx->CWPInfo.pCWPWritePtr;
            FCopy((char *)dmx->CWPInfo.pCWPWritePtr, (char *)pCWPBuffer, len);
            pCWPBuffer += len;
            len = cnt - len;
            FCopy((char *)dmx->CWPInfo.pCWPBuffer, (char *)pCWPBuffer, len);
        } else {
 
            /*trace("CWP buffer full, CWP data lost. W=0x%x, R=0x%x\n",
                   dmx->CWPInfo.pCWPWritePtr, dmx->CWPInfo.pCWPReadPtr);
            */
            return;
        }
    }

    AdvancePtr(&dmx->CWPInfo.pCWPWritePtr, cnt, dmx->CWPInfo.pCWPBuffer, dmx->CWPInfo.pCWPBufferEnd);
    dmx->CWPInfo.CWPDataCount += cnt;

    if (dmx->CWPInfo.CWPNotify) {
        dmx->CWPInfo.CWPNotifyData.pData = dmx->CWPInfo.pCWPReadPtr;
        dmx->CWPInfo.CWPNotifyData.condition = 0;
        dmx->CWPInfo.CWPNotifyData.tag = control_sync;
        dmx->CWPInfo.CWPNotifyData.chid = scid_slot;
        dmx->CWPInfo.CWPNotifyData.length = CWP_KEY_SIZE; 
        /* Perform the callback */
        dmx->CWPInfo.CWPNotify(&(dmx->CWPInfo.CWPNotifyData));

        /* Move read pointer up to next block since callback is complete */
        dmx->CWPInfo.pCWPReadPtr += CWP_COPY_BLOCK_SIZE;
        if (dmx->CWPInfo.pCWPReadPtr >= dmx->CWPInfo.pCWPBufferEnd) {
          dmx->CWPInfo.pCWPReadPtr = dmx->CWPInfo.pCWPBuffer;
        }
    }
}

/*-------------------------------------------------------------------**
** gen_dmx_CWP_Task                                                  **
**     params: none                                                  **
**         This is the main Demux CWP Task.  It is driven by the     **
**         CWP_Sem_ID semaphore from the demux IRQ handler.          **
**-------------------------------------------------------------------*/
void gen_dmx_CWP_Task(void *arg) 
{
    static int BlocksToCopy, LastBlocksToCopy, bytes_to_copy, i;
    static u_int8 *SaveWrite, *SaveRead;
    bool ks;
    u_int32 dmxid;
    DemuxInfo *dmx;

    for (;;) {
        sem_get(CWP_Sem_ID, KAL_WAIT_FOREVER);
        /*********************************************************
         * When we get here, we need to check the CWP demux 
         * bitmask to see which demux needs CWP processing
         *********************************************************/
        for (dmxid=0; dmxid<MAX_DEMUX_UNITS; dmxid++) {
            if (dmxid == 1) continue;  /* Skip demux 1 for now - not used */

            /* Is there work for this demux? */
            if (CWP_int_mask & (1<<dmxid)) {
                dmx = &gDemuxInfo[dmxid];
                SaveRead  = (u_int8*)*DPS_CWP_READ_PTR_EX(dmxid);
                SaveWrite = (u_int8*)*DPS_CWP_WRITE_PTR_EX(dmxid);
                if (SaveWrite >= SaveRead) {
                    bytes_to_copy = ( (u_int32) SaveWrite - (u_int32) SaveRead);
                    if (bytes_to_copy == 0 && dmx->CWPInfo.CWPBufferFull) {
                        bytes_to_copy = (u_int32)*DPS_CWP_END_ADDR_EX(dmxid) - 
                                        (u_int32)*DPS_CWP_START_ADDR_EX(dmxid);
                    }
                } else {
                    bytes_to_copy = (u_int32) SaveWrite - 
                                    (u_int32)*DPS_CWP_START_ADDR_EX(dmxid);
                    bytes_to_copy += (u_int32)*DPS_CWP_END_ADDR_EX(dmxid) - 
                                     (u_int32) SaveRead + 1;
                }
                BlocksToCopy = bytes_to_copy >> CWP_BLOCK_SIZE_SHIFT;
                LastBlocksToCopy = BlocksToCopy;
                if (BlocksToCopy > dmx->CWPInfo.CWPMaxBlocksToCopy) {
                    dmx->CWPInfo.CWPMaxBlocksToCopy = BlocksToCopy;
                    if (dmx->CWPInfo.CWPMaxBlocksToCopy >= ((HWBUF_CWP_SIZE+
                                    CWP_COPY_BLOCK_SIZE-1)/CWP_COPY_BLOCK_SIZE)) {
                        trace_new(TRACE_LEVEL_ALWAYS|TRACE_DPS,"At CWP Buffer Capacity!\n");
                    }
                }
                if (BlocksToCopy > 0) {
                    for (i = 0; i < BlocksToCopy; ++i) {
                        ProcessCWPPacket(dmxid, (u_int8 *) (((u_int32)SaveRead+NCR_BASE) & ~DPS_PAW_SYS_ADDRESS));
                        SaveRead += CWP_COPY_BLOCK_SIZE;
                        if (SaveRead >= (u_int8*)(*DPS_CWP_END_ADDR_EX(dmxid))) {
                            SaveRead = (u_int8*)(*DPS_CWP_START_ADDR_EX(dmxid));
                        }
                    }
                    *DPS_CWP_READ_PTR_EX(dmxid) = (u_int32)SaveRead;
                    SaveWrite = (u_int8*)(*DPS_CWP_WRITE_PTR_EX(dmxid));
                }
                if (dmx->CWPInfo.CWPBufferFull == TRUE) {
                    dmx->CWPInfo.CWPBufferFull = FALSE;
                    *DPS_EVENT_STATUS_REG_EX(dmxid) = DPS_CWP_BUFFER_FULL;
                    *DPS_EVENT_ENABLE_REG_EX(dmxid) |= DPS_CWP_BUFFER_FULL;
                }

                /* Now that we've completed CWP processing for one demux, clear the bitmask, but */
                /* make sure the clear is in a critical section so no other interrupts could be   */
                /* missed while we are clearing the bit. */
                ks = critical_section_begin();            
                CWP_int_mask &= ~(1<<dmxid);
                critical_section_end(ks);
            } /* if CWP ... */
        } /* for dmxid ... */
    } /* for ;; ... */
}

void ProcessCAPPacket(u_int32 dmxid, u_int8 *pCAPBuffer) {
    u_int32 cnt;
    u_int32 len;
    DemuxInfo *dmx = &gDemuxInfo[dmxid];

    cnt = CAP_COPY_BLOCK_SIZE;

    if ((dmx->CAPInfo.pCAPWritePtr+cnt) <= dmx->CAPInfo.pCAPBufferEnd) {
        if (dmx->CAPInfo.pCAPWritePtr >= dmx->CAPInfo.pCAPReadPtr) {

        /* Check for special condition where read ptr is at buffer start and write
           ptr is one block away from buffer end.  This is a biffer full condition */

              if (((dmx->CAPInfo.pCAPWritePtr+cnt) == dmx->CAPInfo.pCAPBufferEnd) &&
                  (dmx->CAPInfo.pCAPReadPtr == dmx->CAPInfo.pCAPBuffer)) {

                  /*trace("CAP buffer full, CAP key data lost. W=0x%x, R=0x%x\n",
                         dmx->CAPInfo.pCAPWritePtr, dmx->CAPInfo.pCAPReadPtr);
                  */
                  return;
              } 
              FCopy((char *)dmx->CAPInfo.pCAPWritePtr, (char *)pCAPBuffer, cnt);
        } else {
              if ((dmx->CAPInfo.pCAPWritePtr+cnt) < dmx->CAPInfo.pCAPReadPtr)  {
                  FCopy((char *)dmx->CAPInfo.pCAPWritePtr, (char *)pCAPBuffer, cnt);
              } else {
                  /*trace("CAP buffer full, CAP key data lost. W=0x%x, R=0x%x\n",
                         dmx->CAPInfo.pCAPWritePtr, dmx->CAPInfo.pCAPReadPtr);
                  */
                  return;
              }
        }
    } else {
        if ((dmx->CAPInfo.pCAPWritePtr >= dmx->CAPInfo.pCAPReadPtr) &&
           ((dmx->CAPInfo.pCAPBuffer+(cnt-(dmx->CAPInfo.pCAPBufferEnd-dmx->CAPInfo.pCAPWritePtr))) <
             dmx->CAPInfo.pCAPReadPtr)) {
            len = dmx->CAPInfo.pCAPBufferEnd - dmx->CAPInfo.pCAPWritePtr;
            FCopy((char *)dmx->CAPInfo.pCAPWritePtr, (char *)pCAPBuffer, len);
            pCAPBuffer += len;
            len = cnt - len;
            FCopy((char *)dmx->CAPInfo.pCAPBuffer, (char *)pCAPBuffer, len);
        } else {
 
            /*trace("CAP buffer full, CAP data lost. W=0x%x, R=0x%x\n",
                   dmx->CAPInfo.pCAPWritePtr, dmx->CAPInfo.pCAPReadPtr);
            */
            return;
        }
    }

    AdvancePtr(&dmx->CAPInfo.pCAPWritePtr, cnt, dmx->CAPInfo.pCAPBuffer, dmx->CAPInfo.pCAPBufferEnd);
    dmx->CAPInfo.CAPDataCount += cnt;

    if (dmx->CAPInfo.CAPNotify) {
        dmx->CAPInfo.CAPNotifyData.pData = dmx->CAPInfo.pCAPReadPtr;
        dmx->CAPInfo.CAPNotifyData.condition = 0;
        dmx->CAPInfo.CAPNotifyData.tag = 0;
        dmx->CAPInfo.CAPNotifyData.chid = 0;
        dmx->CAPInfo.CAPNotifyData.length = CAP_KEY_SIZE; 
        /* Perform the callback */
        dmx->CAPInfo.CAPNotify(&(dmx->CAPInfo.CAPNotifyData));

        /* Move read pointer up to next block since callback is complete */
        dmx->CAPInfo.pCAPReadPtr += CAP_COPY_BLOCK_SIZE;
        if (dmx->CAPInfo.pCAPReadPtr >= dmx->CAPInfo.pCAPBufferEnd) {
          dmx->CAPInfo.pCAPReadPtr = dmx->CAPInfo.pCAPBuffer;
        }
    }
}

/*-------------------------------------------------------------------**
** gen_dmx_CAP_Task                                                  **
**     params: none                                                  **
**         This is the main Demux CAP Task.  It is driven by the     **
**         CAP_Sem_ID semaphore from the demux IRQ handler.          **
**-------------------------------------------------------------------*/
void gen_dmx_CAP_Task(void *arg) 
{
    static int BlocksToCopy, LastBlocksToCopy, bytes_to_copy, i;
    static u_int8 *SaveWrite, *SaveRead;
    bool ks;
    u_int32 dmxid;
    DemuxInfo *dmx;

    for (;;) {
        sem_get(CAP_Sem_ID, KAL_WAIT_FOREVER);
        /*********************************************************
         * When we get here, we need to check the CAP demux 
         * bitmask to see which demux needs CAP processing
         *********************************************************/
        for (dmxid=0; dmxid<MAX_DEMUX_UNITS; dmxid++) {
            if (dmxid == 1) continue;  /* Skip demux 1 for now - not used */

            /* Is there work for this demux? */
            if (CAP_int_mask & (1<<dmxid)) {
                dmx = &gDemuxInfo[dmxid];
                SaveRead  = (u_int8*)*DPS_CAP_READ_PTR_EX(dmxid);
                SaveWrite = (u_int8*)*DPS_CAP_WRITE_PTR_EX(dmxid);
                if (SaveWrite >= SaveRead) {
                    bytes_to_copy = ( (u_int32) SaveWrite - (u_int32) SaveRead);
                    if (bytes_to_copy == 0 && dmx->CAPInfo.CAPBufferFull) {
                        bytes_to_copy = (u_int32)*DPS_CAP_END_ADDR_EX(dmxid) - 
                                        (u_int32)*DPS_CAP_START_ADDR_EX(dmxid);
                    }
                } else {
                    bytes_to_copy = (u_int32) SaveWrite - 
                                    (u_int32)*DPS_CAP_START_ADDR_EX(dmxid);
                    bytes_to_copy += (u_int32)*DPS_CAP_END_ADDR_EX(dmxid) - 
                                     (u_int32) SaveRead + 1;
                }
                BlocksToCopy = bytes_to_copy >> CAP_BLOCK_SIZE_SHIFT;
                LastBlocksToCopy = BlocksToCopy;
                if (BlocksToCopy > dmx->CAPInfo.CAPMaxBlocksToCopy) {
                    dmx->CAPInfo.CAPMaxBlocksToCopy = BlocksToCopy;
                    if (dmx->CAPInfo.CAPMaxBlocksToCopy >= ((HWBUF_TRNSPRT_SIZE+
                                    CAP_COPY_BLOCK_SIZE-1)/CAP_COPY_BLOCK_SIZE)) {
                        trace_new(TRACE_LEVEL_ALWAYS|TRACE_DPS,"At CAP Buffer Capacity!\n");
                    }
                }
                if (BlocksToCopy > 0) {
                    for (i = 0; i < BlocksToCopy; ++i) {
                        ProcessCAPPacket(dmxid, (u_int8 *) (((u_int32)SaveRead+NCR_BASE) & ~DPS_PAW_SYS_ADDRESS));
                        SaveRead += CAP_COPY_BLOCK_SIZE;
                        if (SaveRead >= (u_int8*)(*DPS_CAP_END_ADDR_EX(dmxid))) {
                            SaveRead = (u_int8*)(*DPS_CAP_START_ADDR_EX(dmxid));
                        }
                    }
                    *DPS_CAP_READ_PTR_EX(dmxid) = (u_int32)SaveRead;
                    SaveWrite = (u_int8*)(*DPS_CAP_WRITE_PTR_EX(dmxid));
                }
                if (dmx->CAPInfo.CAPBufferFull == TRUE) {
                    dmx->CAPInfo.CAPBufferFull = FALSE;
                    *DPS_EVENT_STATUS_REG_EX(dmxid) = DPS_CAP_BUFFER_FULL;
                    *DPS_EVENT_ENABLE_REG_EX(dmxid) |= DPS_CAP_BUFFER_FULL;
                }

                /* Now that we've completed CAP processing for one demux, clear the bitmask, but */
                /* make sure the clear is in a critical section so no other interrupts could be   */
                /* missed while we are clearing the bit. */
                ks = critical_section_begin();            
                CAP_int_mask &= ~(1<<dmxid);
                critical_section_end(ks);
            } /* if CAP ... */
        } /* for dmxid ... */
    } /* for ;; ... */
}
#endif

/*--------------------------------------------------------------------------------------**
** DemuxIRQPCRNotify                                                                    **
** Desc.                                                                                **
**     Called by the clock recovery task to notify demux driver when a new PCR has      **
**     been received.                                                                   **
** Params.                                                                              **
**     None                                                                             **
** Returns                                                                              **
**     Nothing                                                                          **
** TBD Add parameter to pass in PCR value                                               **
**--------------------------------------------------------------------------------------*/
void DemuxIRQPCRNotify(u_int32 PCR_High, u_int32 PCR_Low) {
    isr_trace_new(DPS_ISR_MSG,"DEMUXISR: MPG_PCR_RECEIVED\n", 0, 0);
    NewPCRFlag = TRUE;
    if (gPCRCallBack != NULL) {
        gPCRCallBack(PCR_High, PCR_Low);
    }
}

/*----------------------------------------------------------------------------**
** gen_demux_timer_call_back                                                  **
**                                                                            **
** Params:                                                                    **
**         timer    : Tick timer id                                           **
**         pUserData: Upper 16bits is dmxid, Lower 16bits is chid             **
** Desc:                                                                      **
**         Stops the tick timer for this channel and notifies the PSI         **
**         handler task so a client callback occurs.                          **
** Returns:                                                                   **
**         nothing                                                            **
** Environment:                                                               **
**         ISR only                                                           **
**----------------------------------------------------------------------------*/
void gen_demux_timer_call_back(timer_id_t timer, void *pUserData)
{
    u_int32 dmxid = ((u_int32)pUserData) >> 16;
    u_int32 chid  = ((u_int32)pUserData) & 0xffff;
    DemuxInfo *dmx = &gDemuxInfo[dmxid];

    /*
     * Ensure this channel has a timer
     */
    if(dmx->ChannelInfoTable[chid].ChannelTimerActive &&
       dmx->ChannelInfoTable[chid].ChannelTimer)
    {
        /*
         * Stop the timer
         */
        if(RC_OK != tick_stop (dmx->ChannelInfoTable[chid].ChannelTimer))
        {
            isr_trace_new(DPS_ERROR_MSG,"DEMUX:tick_stop failed.\n",0,0);
        }
        dmx->ChannelInfoTable[chid].ChannelTimerActive = FALSE;

        /*
         * Indicate there is a timeout condition present
         */
        ++dmx->ChannelInfoTable[chid].TimerNotifyCount;

        /*
         * Trigger the PSI task to run which makes the callback
         * to the client.
         */
        PSI_int_mask |= 1<<dmxid;
        sem_put(PSI_Sem_ID);
    }
    else
    {
        /* Badness */
        isr_trace_new(DPS_ERROR_MSG,
            "DEMUX:timer_call_back with no timer! dmxid=%d chid=%d\n",
            dmxid, chid);
    }
}

#if PARSER_TYPE==DTV_PARSER
/*----------------------------------------------------------------------------**
** gen_demux_CWP_timer_call_back                                              **
**                                                                            **
** Params:                                                                    **
**         timer    : Tick timer id                                           **
**         pUserData: dmxid                                                   **
** Desc:                                                                      **
**         Stops the tick timer for CWPs and notifies the CWP                 **
**         handler task so a client callback occurs.                          **
** Returns:                                                                   **
**         nothing                                                            **
** Environment:                                                               **
**         ISR only                                                           **
**----------------------------------------------------------------------------*/
void gen_demux_CWP_timer_call_back(timer_id_t timer, void *pUserData)
{
    u_int32 dmxid = (u_int32)pUserData;
    DemuxInfo *dmx = &gDemuxInfo[dmxid];

    /*
     * Ensure a CWP timer was allocated
     */
    if(dmx->CWPInfo.CWPTimerActive &&
       dmx->CWPInfo.CWPTimer)
    {
        /*
         * Stop the timer
         */
        if(RC_OK != tick_stop (dmx->CWPInfo.CWPTimer))
        {
            isr_trace_new(DPS_ERROR_MSG,"DEMUX:CWP tick_stop failed.\n",0,0);
        }
        dmx->CWPInfo.CWPTimerActive = FALSE;

        /*
         * Indicate there is a timeout condition present
         */
        ++dmx->CWPInfo.CWPTimerNotifyCount;

        /*
         * Trigger the PSI task to run which makes the callback
         * to the client.
         */
        CWP_int_mask |= 1<<dmxid;
        sem_put(CWP_Sem_ID);
    }
    else
    {
        /* Badness */
        isr_trace_new(DPS_ERROR_MSG,
            "DEMUX:CWP timer_call_back with no timer! dmxid=%d\n",
            dmxid, 0);
    }
}

/*----------------------------------------------------------------------------**
** gen_demux_CAP_timer_call_back                                              **
**                                                                            **
** Params:                                                                    **
**         timer    : Tick timer id                                           **
**         pUserData: dmxid                                                   **
** Desc:                                                                      **
**         Stops the tick timer for CAPs and notifies the CAP                 **
**         handler task so a client callback occurs.                          **
** Returns:                                                                   **
**         nothing                                                            **
** Environment:                                                               **
**         ISR only                                                           **
**----------------------------------------------------------------------------*/
void gen_demux_CAP_timer_call_back(timer_id_t timer, void *pUserData)
{
    u_int32 dmxid = (u_int32)pUserData;
    DemuxInfo *dmx = &gDemuxInfo[dmxid];

    /*
     * Ensure a CAP timer was allocated
     */
    if(dmx->CAPInfo.CAPTimerActive &&
       dmx->CAPInfo.CAPTimer)
    {
        /*
         * Stop the timer
         */
        if(RC_OK != tick_stop (dmx->CAPInfo.CAPTimer))
        {
            isr_trace_new(DPS_ERROR_MSG,"DEMUX:CAP tick_stop failed.\n",0,0);
        }
        dmx->CAPInfo.CAPTimerActive = FALSE;

        /*
         * Indicate there is a timeout condition present
         */
        ++dmx->CAPInfo.CAPTimerNotifyCount;

        /*
         * Trigger the PSI task to run which makes the callback
         * to the client.
         */
        CAP_int_mask |= 1<<dmxid;
        sem_put(CAP_Sem_ID);
    }
    else
    {
        /* Badness */
        isr_trace_new(DPS_ERROR_MSG,
            "DEMUX:CAP timer_call_back with no timer! dmxid=%d\n",
            dmxid, 0);
    }
}
#endif

/*-------------------------------------------------------------------**
** AdvancePtr                                                        **
**     description                                                   **
**        Advances a pointer in a wrapped buffer.                    **
**     params                                                        **
**        p_ptr      - Pointer to the ptr to be advanced             **
**        length     - Number of bytes to advance ptr (above) by     **
**        pBuffer    - Start ptr of the buffer ptr is in             **
**        pBufferEnd - End ptr of the source buffer (exclusive)      **
**     returns                                                       **
**        TRUE is buffer wrapped                                     **
**-------------------------------------------------------------------*/
bool AdvancePtr(u_int8 **p_ptr, u_int32 length, u_int8 *pBuffer, u_int8 *pBufferEnd) {
    u_int8 *ptr;
    bool wrapped = FALSE;

    ptr = *p_ptr + length;
    if (ptr >= pBufferEnd) {
        /*
         * Pointer wrapped buffer, offset pointer from start of buffer
         */

        ptr = pBuffer+(ptr-pBufferEnd);
        wrapped = TRUE;
    }

    /*
     * Send the updated ptr back
     */

    *p_ptr = ptr;
    return(wrapped);
}

/*-------------------------------------------------------**
** gen_dmx_get_filter_chid                               **
** Desc.                                                 **
**     returns the channel ID of the owner of the filter **
**                                                       **
** Params:                                               **
**     fid: filter ID                                    **
**                                                       **
** Returns:                                              **
**     chid: channel id                                  **
**                                                       **
**-------------------------------------------------------*/
u_int32 gen_dmx_get_filter_chid(u_int32 dmxid, u_int32 fid) {
    DemuxInfo *dmx = &gDemuxInfo[dmxid];
    if (fid < TOTAL_FILTERS) {
        return dmx->FilterTable[fid].
        OwnerChid;
    } else {
        return GENDMX_BAD_CHANNEL;
    }
}
/*-------------------------------------------------------**
** gen_dmx_get_channel_filter                            **
** Desc.                                                 **
**    This function will return the first filter owned   **
**    by the specified channel. This is used for drivers **
**    that are filter centric.                           **
**                                                       **
**     returns the filter  ID of the owner of the channel**
**                                                       **
** Params:                                               **
**    chid: filter ID                                    **
**                                                       **
** Returns:                                              **
**     fid : channel id                                  **
**                                                       **
**-------------------------------------------------------*/
u_int32 gen_dmx_get_channel_filter(u_int32 dmxid, u_int32 chid) {
    u_int32 i;
    u_int32 f_bit = 1;
    DemuxInfo *dmx = &gDemuxInfo[dmxid];

    if (chid >= TOTAL_CHANNELS || !dmx->DemuxInitialized)
        return GENDMX_BAD_FILTER;
    for (i = 0; i < TOTAL_FILTERS; ++i) {
        if (dmx->ChannelInfoTable[chid].FiltersAllocated & f_bit) {
            return i;
        }
        f_bit <<= 1;
    }
    return GENDMX_BAD_FILTER;
}

/*-------------------------------------------------------**
** gen_dmx_get_allocated_filters                         **
** Desc.                                                 **
**     returns a 32 bit bitmap for each filter the chid  **
**     is using                                          **
** Params:                                               **
**     chid: channel id                                  **
**                                                       **
** Returns:                                              **
**     fid bitmap                                        **
**                                                       **
**-------------------------------------------------------*/
u_int32 gen_dmx_get_allocated_filters(u_int32 dmxid, u_int32 chid) {
    DemuxInfo *dmx = &gDemuxInfo[dmxid];
    if (chid >= TOTAL_CHANNELS || !dmx->DemuxInitialized)
        return 0;
    return dmx->ChannelInfoTable[chid].FiltersAllocated;
}

/*---------------------------------------------------------------------------------------------------**
** gen_dmx_section_buffers_inquire                                                                   **
** Params:                                                                                           **
**         chid: channel ID                                                                          **
**         beg : address to store the start address of the buffer                                    **
**         end : address to store the end address of the buffer                                      **
** Desc:                                                                                             **
**         Informs the client of the start and end addresses of the circular buffer for the channel. **
** Returns:                                                                                          **
**         0  Success                                                                                **
**         -1 Error                                                                                  **
**                                                                                                   **
**---------------------------------------------------------------------------------------------------*/
u_int32 gen_dmx_buffers_inquire(u_int32 dmxid, u_int32 chid, u_int8 **beg, u_int8 **end) {
    DemuxInfo *dmx = &gDemuxInfo[dmxid];
    trace_new(DPS_FUNC_TRACE,"DEMUX:gen_dmx_buffers_inquire called\n");
    if (!dmx->DemuxInitialized)
        return GENDMX_ERROR_RETURN;

    if (dmx->ChannelInfoTable[chid].InUseFlag == CHANNEL_FREE) {
        trace_new(DPS_ERROR_MSG,"DEMUX:gen_dmx_buffers_inquire failed\n");
        return GENDMX_ERROR_RETURN;
    }
    *beg = dmx->ChannelInfoTable[chid].pBuffer;
    *end = (dmx->ChannelInfoTable[chid].pBufferEnd-1);
    return 0;
}

/*--------------------------------------------------------**
** gen_dmx_channel_enabled                                **
** Params:                                                **
**         chid: channel id                               **
** Desc.:                                                 **
**         Returns the state of the channel.              **
** Returns:                                               **
**         TRUE    - Filter is Enabled,                   **
**         FALSE   - Filter is Disabled.                  **
**                                                        **
**--------------------------------------------------------*/
bool gen_dmx_channel_enabled(u_int32 dmxid, u_int32 chid) {
    DemuxInfo *dmx = &gDemuxInfo[dmxid];
    if (chid < TOTAL_CHANNELS)
        return dmx->ChannelInfoTable[chid].DemuxEnable;
    else
        return FALSE;
}

/*-------------------------------------------------------------------**
** WCopyBytes                                                        **
**     description                                                   **
**        Copies data from a wrapped uncached buffer.                **
**     params                                                        **
**        dest       - Pointer to the cached destination buffer      **
**        source     - Pointer to the uncached wrapped source buffer **
**        length     - Number of bytes to copy                       **
**        pBuffer    - Start ptr of the source buffer                **
**        pBufferEnd - End ptr of the source buffer (exclusive)      **
**     returns                                                       **
**        TRUE is buffer wrapped                                     **
**-------------------------------------------------------------------*/
bool WCopyBytes(u_int8 *dest, u_int8 *source, u_int32 length, u_int8 *pBuffer, u_int8 *pBufferEnd)
{
    u_int8 *ptr;
    u_int32 len;
    bool wrapped = FALSE;

    if ((source+length) <= pBufferEnd) {
        /*
         * Area to copy does not wrap the buffer
         */

        FCopy((char *)dest, (char *)source, length);
    } else {
        wrapped = TRUE;

        /*
         * Area to copy wraps the buffer, copy in two parts
         * with the first part up to the end of the buffer
         */

        ptr = dest;
        len = pBufferEnd - source;
        FCopy((char *)ptr, (char *)source, len);

        /*
         * Now copy the last part of the area starting at
         * the beginning of the buffer
         */

        ptr += len;
        len = length - len;
        FCopy((char *)ptr, (char *)pBuffer, len);
    }
    return(wrapped);
}

void DumpAll(u_int32 dmxid) {
    int i;
    DemuxInfo *dmx = &gDemuxInfo[dmxid];

    /*
    trace_new(DPS_FUNC_TRACE,"---------Filters----------\n");
    for (i = 0; i < TOTAL_FILTERS; ++i) {
        if (dmx->FilterTable[i].FilterEnabled) {
            trace_new(DPS_ERROR_MSG,"--------------------------\n");
            trace_new(DPS_ERROR_MSG,"Filter ID = #%d\n", i);
            trace_new(DPS_ERROR_MSG,"  OwnerChid=%d\n", dmx->FilterTable[i].OwnerChid);
            trace_new(DPS_ERROR_MSG,"  Size     =%d\n", dmx->FilterTable[i].FilterSize);
            trace_new(DPS_ERROR_MSG,"  Enabled  =%d\n", dmx->FilterTable[i].FilterEnabled);
            #if PARSER_FILTERING == FILTER_1212
            trace_new(DPS_ERROR_MSG,"Match = %0x %0x %0x\n", dmx->FilterTable[i].Match[0], dmx->FilterTable[i].Match[1], dmx->FilterTable[i].Match[2]);
            trace_new(DPS_ERROR_MSG,"Mask  = %0x %0x %0x\n", dmx->FilterTable[i].Mask[0], dmx->FilterTable[i].Mask[1], dmx->FilterTable[i].Mask[2]);
            #else
            trace_new(DPS_ERROR_MSG,"Match = %0x %0x\n", dmx->FilterTable[i].Match[0], dmx->FilterTable[i].Match[1]);
            trace_new(DPS_ERROR_MSG,"Mask  = %0x %0x\n", dmx->FilterTable[i].Mask[0], dmx->FilterTable[i].Mask[1]);
            #endif
        }
    }
    */
    trace_new(DPS_FUNC_TRACE,"---------Channels---------\n");
    for (i = 0; i < 32; ++i) {
        if (dmx->ChannelInfoTable[i].InUseFlag) {
            trace_new(DPS_ERROR_MSG,"--------------------------\n");
            trace_new(DPS_ERROR_MSG,"Channel #=%d\n", i);
            trace_new(DPS_ERROR_MSG," PID                = 0x%x\n",dmx->ChannelInfoTable[i].PID);
            trace_new(DPS_ERROR_MSG," FilterEnable       = 0x%0x\n",dmx->ChannelInfoTable[i].FilterEnable);
            trace_new(DPS_ERROR_MSG," InUseFlag          =    %d\n",dmx->ChannelInfoTable[i].InUseFlag);
            trace_new(DPS_ERROR_MSG," DemuxEnable        =    %d\n",dmx->ChannelInfoTable[i].DemuxEnable);
            trace_new(DPS_ERROR_MSG," PESChannel         =    %d\n",dmx->ChannelInfoTable[i].PESChannel);
            trace_new(DPS_ERROR_MSG," Overflows          =    %d\n",dmx->ChannelInfoTable[i].Overflows);
            trace_new(DPS_ERROR_MSG," OverflowsHandled   =    %d\n",dmx->ChannelInfoTable[i].OverflowsHandled);
        }
    }
    Dump=0;
}


u_int32 gen_dmx_get_audio_buff_size() {
    return HWBUF_ENCAUD_SIZE;
}

u_int32 gen_dmx_get_video_buff_size() {
    return HWBUF_ENCVID_SIZE;
}

/*
 * gen_dmx_get_audio_pid
 *
 * parameters:
 *    dmx       - DemuxInfo
 *
 * returns:
 *    The current setting of the audio pid, as set in the
 *    hardware.
 */
u_int16 gen_dmx_get_audio_pid(u_int32 dmxid)
{
    return(*DPS_PID_BASE_EX(dmxid,AUDIO_CHANNEL) & 0x1fff );
}

/****************************************************************************
 * Modifications:
 * $Log: 
 *  35   mpeg      1.34        6/16/04 11:13:34 AM    Tim White       CR(s) 
 *        9483 9484 : Changed pvr to rec for consistency.
 *        
 *  34   mpeg      1.33        3/23/04 3:19:25 PM     Larry Wang      CR(s) 
 *        8638 8639 : Deal with XTV record and record event interrupts in the 
 *        same way as in PVR app.
 *  33   mpeg      1.32        3/18/04 10:38:44 AM    Tim White       CR(s) 
 *        8591 : Allow use of >64KB buffers to the 
 *        cnxt_dmx_dma_input_add_buffer() function.  The
 *        DMA function internally handles breaking up the DMA transaction into 
 *        sub 64KB sections
 *        and only delivers the interrupt at the end.  Halt is supported only 
 *        on request
 *        boundaries.
 *        
 *  32   mpeg      1.31        3/16/04 3:57:08 PM     Tim White       CR(s) 
 *        8545 : Add dma_input_selected bool for DMXDMA operation.
 *        
 *  31   mpeg      1.30        3/16/04 9:35:53 AM     Tim Ross        CR(s) 
 *        8545 : Removed conditional compilation from demux event interrupt 
 *        handling to
 *        enable PVR event processing.
 *  30   mpeg      1.29        3/10/04 1:26:46 PM     Tim Ross        CR(s) 
 *        8545 : Corrected compilation errors for PVR record demux code. 
 *  29   mpeg      1.28        3/10/04 10:57:19 AM    Tim White       CR(s) 
 *        8545 : Removed PVR playback LWM notification from demux.
 *        
 *  28   mpeg      1.27        3/2/04 10:59:32 AM     Tim Ross        CR(s) 
 *        8451 : Initialized bRecording member of channel struct.
 *  27   mpeg      1.26        2/24/04 2:45:09 PM     Bob Van Gulick  CR(s) 
 *        8427 : Add cnxt_dmx_set_timebase_offset function to offset PTS by +/-
 *         12 hours
 *        
 *  26   mpeg      1.25        12/7/03 6:22:33 PM     Tim White       CR(s) 
 *        8113 : Allow any word aligned buffer to work with the Demux DMA Input
 *         Extension.
 *        
 *  25   mpeg      1.24        11/25/03 3:52:46 PM    Tim White       CR(s): 
 *        8027 Drop Demux DMA Input Extension function.
 *        
 *  24   mpeg      1.23        11/19/03 10:10:03 AM   Tim White       CR(s): 
 *        7987 Added Demux DMA and Demux PVR extension support phase 1.
 *        
 *  23   mpeg      1.22        9/22/03 4:52:52 PM     Bob Van Gulick  SCR(s) 
 *        7519 :
 *        Add support for DirecTV CAPs
 *        
 *        
 *  22   mpeg      1.21        9/2/03 7:03:12 PM      Joe Kroesche    SCR(s) 
 *        7415 :
 *        removed unneeded header files to eliminate extra warnings when 
 *        building
 *        for PSOS
 *        
 *  21   mpeg      1.20        8/27/03 11:01:16 AM    Bob Van Gulick  SCR(s) 
 *        7387 :
 *        Add support for CWP processing in DirecTV 
 *        
 *        
 *  20   mpeg      1.19        6/24/03 2:40:40 PM     Miles Bintz     SCR(s) 
 *        6822 :
 *        added initialization value to remove warning in release build
 *        
 *  19   mpeg      1.18        6/9/03 5:59:00 PM      Bob Van Gulick  SCR(s) 
 *        6755 :
 *        Add support for 8 slot descram in demux.  Also change use of 
 *        DESC_CHANNELS to DPS_NUM_DESCRAMBLERS.
 *        
 *        
 *  18   mpeg      1.17        4/24/03 4:52:20 PM     Tim White       SCR(s) 
 *        6097 :
 *        Allow 6 descrambled simultaneous PES channels.  Remove #ifndef 
 *        USE_OLD_PES code.
 *        
 *        
 *  17   mpeg      1.16        4/15/03 3:50:42 PM     Dave Wilson     SCR(s) 
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
 *  16   mpeg      1.15        4/2/03 11:58:42 AM     Brendan Donahe  SCR(s) 
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
 *  15   mpeg      1.14        3/3/03 11:15:40 AM     Larry Wang      SCR(s) 
 *        5631 :
 *        Replace hard coded number 188 with TS_COPY_BLOCK_SIZE.
 *        
 *        
 *  14   mpeg      1.13        11/20/02 5:06:38 PM    Bob Van Gulick  SCR(s) 
 *        4998 :
 *        Remove gendemux version of register_pcr_notify function.  It is now 
 *        part of the API.
 *        
 *        
 *  13   mpeg      1.12        11/20/02 3:57:22 PM    Bob Van Gulick  SCR(s) 
 *        4998 :
 *        Add register_pcr_notify_callback function to demux api.
 *        
 *        
 *  12   mpeg      1.11        10/16/02 3:20:42 PM    Bob Van Gulick  SCR(s) 
 *        4799 :
 *        Remove CANAL_PLUS_FILTERING #ifdefs and use #if PARSER_FILTERING == 
 *        FILTER_xxx
 *        instead.  PARSER_FILTERING is defined in the sw config
 *        
 *        
 *  11   mpeg      1.10        9/19/02 3:42:46 PM     Joe Kroesche    SCR(s) 
 *        4610 :
 *        added crc notification feature, removed changes for previous crc 
 *        notification
 *        method. NOTE!!! requires matching pawser ucode update of #4626
 *        
 *  10   mpeg      1.9         9/18/02 4:25:34 PM     Joe Kroesche    SCR(s) 
 *        4619 :
 *        Added conditionally compiled code to support Canal+ special cases
 *        
 *  9    mpeg      1.8         9/9/02 11:02:10 AM     Bob Van Gulick  SCR(s) 
 *        4557 :
 *        Change ChannelTimer init to a 0 from NULL pointer to remove warning
 *        
 *        
 *  8    mpeg      1.7         9/5/02 6:30:08 PM      Bob Van Gulick  SCR(s) 
 *        4530 :
 *        Change CRC check to use Header Notify instead of Section Notify
 *        
 *        
 *  7    mpeg      1.6         8/30/02 2:43:34 PM     Bob Van Gulick  SCR(s) 
 *        4485 :
 *        Add support for CRC checking of SI packets
 *        
 *        
 *  6    mpeg      1.5         8/28/02 6:25:24 PM     Bob Van Gulick  SCR(s) 
 *        4484 :
 *        Remove special handling of odd non word aligned transport buffers and
 *         pass all PES copies through FCopy which can handle all alignments.
 *        
 *        
 *  5    mpeg      1.4         8/16/02 6:05:06 PM     Tim White       SCR(s) 
 *        4420 :
 *        Add support for new DVR microcode which supports DVR, XPRT, and 
 *        MULTI_PSI together.
 *        
 *        
 *  4    mpeg      1.3         8/5/02 11:55:14 AM     Tim White       SCR(s) 
 *        4330 :
 *        Fixed timeout and single shot (ONE_SHOT) capabilities.
 *        
 *        
 *  3    mpeg      1.2         6/27/02 5:57:18 PM     Tim White       SCR(s) 
 *        4108 :
 *        Convert MHP glue layer to use new DEMUX driver.
 *        
 *        
 *  2    mpeg      1.1         4/26/02 3:16:12 PM     Tim White       SCR(s) 
 *        3562 :
 *        Add support for Colorado Rev_F.
 *        
 *        
 *  1    mpeg      1.0         12/18/01 11:00:32 AM   Bob Van Gulick  
 * $
 ****************************************************************************/

