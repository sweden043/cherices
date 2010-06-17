/****************************************************************************/
/*                   CONEXANT PROPRIETARY AND CONFIDENTIAL                  */
/*                       SOFTWARE FILE/MODULE HEADER                        */
/*           Conexant Systems Inc. (c) 1998, 1999, 2000, 2001, 2002         */
/*                               Austin, TX                                 */
/*                            All Rights Reserved                           */
/****************************************************************************/
/*
 * Filename:       dvb_lib.c
 *
 * Description:    DVB specific code copied from watchtv/root.c
 *
 * Author:         Many different people!        
 *
 ****************************************************************************/
/* $Id: dvb_lib.c,v 1.17, 2004-03-31 20:44:09Z, Yong Lu$
 ****************************************************************************/

#include <string.h>
#include "stbcfg.h"
#include "basetype.h"
#include "retcodes.h"
#include "trace.h"
#include "kal.h"
#include "cfg_dvb.h"
#include "confmgr.h"
#include "demuxapi.h"
#include "dvb.h"
#include "dvb_priv.h"

#define INVALID_PROGRAM             (0xFFFF)
#if(IPSTB_INCLUDE == YES)
  #define PROCESS_NEXT_PAT_VERSION    (0)
#else
  #define PROCESS_NEXT_PAT_VERSION    (1)
#endif
static genfilter_mode PMT_Header_Notify(pgen_notify_t notify_data);
static void PMT_Section_Notify(pgen_notify_t notify_data);

static genfilter_mode PAT_Header_Notify(pgen_notify_t notify_data);
static void PAT_Section_Notify(pgen_notify_t notify_data);
static void ProcessPATSection(u_int8 *pBuffer);

static void ProcessPMTSection(u_int8 *pBuffer);
static int ProcessProgramDesc(u_int8 *pBuffer, u_int32 PATIndex);

/* Service Description Routines */
static genfilter_mode SDT_Header_Notify(pgen_notify_t notify_data);
static void SDT_Section_Notify(pgen_notify_t notify_data);
static void ProcessSDTSection(u_int8 *pBuffer);
static u_int32 GetServiceInfo(u_int8 *SDTptr);

DVB_PFNNOTIFY pfnDVBCallback = NULL;

static PATEntry PATEntryTable[MAX_PMT_ENTRIES];
static u_int32 NumPATEntries = 0;

static u_int32 PATChannel = GENDMX_BAD_CHANNEL;
/*                                 TID, LEN1, LEN2, STRID,STRID , VER , SEC , LastSEC */
static u_int8 PATFilterMatch[8] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}; /* table id=0x00 and section number = 0x00 */
static u_int8 PATFilterMask[8]  = {0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0x00}; /* look for specific table id and section number */
static u_int32 PATfid = GENDMX_BAD_FILTER;

static u_int8 PMTFilterMatch[8] = {0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}; /* table id=0x02 and section number = 0x00 */
static u_int8 PMTFilterMask[8]  = {0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0x00}; /* look for specific table id and section number */

#define SDTPID_VALUE    0x0011
#define SDT_FILTER_TSID_HI 3
#define SDT_FILTER_TSID_LO 4
static u_int32 SDTChannel = GENDMX_BAD_CHANNEL;
static u_int32 SDTfid = GENDMX_BAD_FILTER;
/*                                 TID,               LEN1, LEN2, STRID,STRID , VER , SEC , LastSEC */
static u_int8 SDTFilterMatch[8] = {SDT_ACTUAL_TABLEID, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}; /* table id=0x00 and section number = 0x00 */
static u_int8 SDTFilterMask[8]  = {SDT_TABLEID_FILTER, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0xFF, 0x00}; /* look for specific table id SDT actual, section num ==0  and required TSID */

static u_int32 CurrentProgram = 0;
static u_int32 CurrentProgramNumber = GENDMX_ERROR_RETURN;
static u_int32 CurrentAudio = 0;
static u_int32 CurrentVideo = 0;
static u_int32 CurrentData  = 0;
static u_int32 CurrentTSID  = 0;

/***********************************************************************/
/* cnxt_dvb_init                                                         */
/*      Initializes the PATEntry Table                                 */
/*      Registers the callback function to send a control message to   */
/*      the application task                                           */
/*                                                                     */
/*                                                                     */
/***********************************************************************/
void cnxt_dvb_init(DVB_PFNNOTIFY pDVBCallBack )
{
    int i,j;

    for (i = 0; i < MAX_PMT_ENTRIES; ++i)
    {
        PATEntryTable[i].PMTChannel       = GENDMX_BAD_CHANNEL;
        PATEntryTable[i].PMTChannelBuffer = NULL;
        PATEntryTable[i].PMTfid           = GENDMX_BAD_FILTER;
        PATEntryTable[i].ProgramNumber    = (u_int16) INVALID_PROGRAM;
        memset((void*) PATEntryTable[i].ServiceProvName,'\0',SIZEOF_SERVICEINFO);
        memset((void*) PATEntryTable[i].ServiceName,'\0',SIZEOF_SERVICEINFO);
        PATEntryTable[i].ProgramMapPID    = (u_int16) GEN_INVALID_PID;
        PATEntryTable[i].PCRPID           = (u_int16) GEN_INVALID_PID;
        PATEntryTable[i].NumAudioPIDs     = 0;
        PATEntryTable[i].NumVideoPIDs     = 0;
        PATEntryTable[i].NumDataPIDs      = 0;
        PATEntryTable[i].Version          = -1;

        for (j = 0; j < MAX_PID_ENTRIES; ++j)
        {
            PATEntryTable[i].VideoPIDs[j] = (u_int16) GEN_INVALID_PID;
            PATEntryTable[i].AudioPIDs[j] = (u_int16) GEN_INVALID_PID;
            PATEntryTable[i].DataPIDs[j]  = (u_int16) GEN_INVALID_PID;
        }
    }
    /* register a callback function to send a control message to notify the applicaton(Watchtv) */
    if(pDVBCallBack){
        pfnDVBCallback = pDVBCallBack;
    }
}

/*-------------------------------------------------------------------------------------**
** InitializeForNewPAT                                                                 **
** Desc.                                                                               **
**     This function will reinitialize the data structures when a new PAT is detected. **
** Params                                                                              **
**     None                                                                            **
** Returns                                                                             **
**     None                                                                            **
**-------------------------------------------------------------------------------------*/
 void InitializeForNewPAT()
{
    int i,j;
    u_int32 FreedChannels = 0;

    if (SDTChannel != GENDMX_BAD_CHANNEL)
    {
        if (cnxt_dmx_filter_control(gDemuxInstance, SDTChannel, SDTfid, GEN_DEMUX_DISABLE) != DMX_STATUS_OK)
        {
            trace_new(TL_ERROR,
                      TRACE_FG_LIGHT_RED
                        "cnxt_dmx_filter_control Failed while disabling SDT filter\n" 
                      TRACE_FG_NORMAL);

        }

        if (cnxt_dmx_channel_control(gDemuxInstance, SDTChannel, GEN_DEMUX_DISABLE) != DMX_STATUS_OK)
        {
            trace_new(TL_ERROR,
                      TRACE_FG_LIGHT_RED
                        "cnxt_dmx_channel_control Failed while disabling SDT channel\n"
                      TRACE_FG_NORMAL);
        } 
    }

    /* It's tricky here, have to free all the filter first */
    for (i = 0; i < MAX_PMT_ENTRIES; ++i)
    {
        if (PATEntryTable[i].PMTfid != GENDMX_BAD_FILTER)
        {
            /* The above scenario applies to filters as well */
            cnxt_dmx_filter_control(gDemuxInstance, PATEntryTable[i].PMTChannel,
                                    PATEntryTable[i].PMTfid, GEN_DEMUX_DISABLE);
            cnxt_dmx_filter_close(gDemuxInstance, PATEntryTable[i].PMTChannel,
                                  PATEntryTable[i].PMTfid);
            PATEntryTable[i].PMTfid = GENDMX_BAD_FILTER;
        }
    }

    for (i = 0; i < MAX_PMT_ENTRIES; ++i)
    {
        if (PATEntryTable[i].PMTChannel != GENDMX_BAD_CHANNEL &&
            !(FreedChannels & (1<<PATEntryTable[i].PMTChannel)))
        {
            /* Several slots in the PATEntryTable may contain the same PMTChannel ID.  We
            only want to free the PMT channel one time, otherwise the demux will complain
            that we are freeing a non-allocated channel.  Therefore, we keep a bitmask
            to see which channels have already been freed. */

            FreedChannels |= (1<<PATEntryTable[i].PMTChannel);

            cnxt_dmx_channel_control(gDemuxInstance, PATEntryTable[i].PMTChannel,
                                     GEN_DEMUX_DISABLE);
            cnxt_dmx_channel_close(gDemuxInstance, PATEntryTable[i].PMTChannel);
            mem_nc_free(PATEntryTable[i].PMTChannelBuffer);
        }

        PATEntryTable[i].PMTChannel       = GENDMX_BAD_CHANNEL;
        PATEntryTable[i].PMTChannelBuffer = NULL;
        PATEntryTable[i].PMTfid           = GENDMX_BAD_FILTER;
        memset((void*) PATEntryTable[i].ServiceProvName,'\0',SIZEOF_SERVICEINFO);
        memset((void*) PATEntryTable[i].ServiceName,'\0',SIZEOF_SERVICEINFO);
        PATEntryTable[i].NumAudioPIDs     = 0;
        PATEntryTable[i].NumVideoPIDs     = 0;
        PATEntryTable[i].NumDataPIDs      = 0;
        PATEntryTable[i].Version          = -1;

        for (j = 0; j < MAX_PID_ENTRIES; ++j)
        {
            PATEntryTable[i].VideoPIDs[j] = (u_int16) GEN_INVALID_PID;
            PATEntryTable[i].AudioPIDs[j] = (u_int16) GEN_INVALID_PID;
            PATEntryTable[i].DataPIDs[j]  = (u_int16) GEN_INVALID_PID;
        }
    }
}

/***********************************************************************/
/* GetPAT                                                              */
/*                                                                     */
/*   Setup the demux driver to aquire a PAT and then wait for one.     */
/*                                                                     */
/*                                                                     */
/***********************************************************************/
void GetPAT()
{
    void *buffer;
    u_int16 pid;

#if PARSER_PASSAGE_ENABLE==YES
    /* set passage mode */
    cnxt_dmx_set_passage_mode ( gDemuxInstance, 1 );

    pid = 0x1000;
#else
    pid = 0;
#endif

    /*Allocate a section channel for the PAT with 1 filter*/
    if (PATChannel == GENDMX_BAD_CHANNEL)
    {
        cnxt_dmx_channel_open(gDemuxInstance, 0, PSI_CHANNEL_TYPE,
                              &PATChannel);

        /* Register PAT channel notification */
        cnxt_dmx_set_section_channel_attributes(gDemuxInstance, PATChannel,
                                                (gen_callback_fct_t) PAT_Header_Notify,
                                                (gen_callback_fct_t) PAT_Section_Notify, 0, 4);

        /* Set channel buffer */
        buffer = (void *)( (u_int32)mem_nc_malloc(PAT_BUFFER_SIZE) & ~NCR_BASE );
        cnxt_dmx_set_channel_buffer(gDemuxInstance, PATChannel, buffer, PAT_BUFFER_SIZE);

        cnxt_dmx_filter_open(gDemuxInstance, PATChannel, 8, &PATfid);
    }
    else
    {
      trace_new(TL_INFO, "GetPAT: Setting up for a new PAT\n");
      PATFilterMatch[6] = 0x00;
      PATFilterMatch[5] = 0x00;
      PATFilterMask[5]  = 0x00;
      NumPATEntries = 0;
    }

    /* setup a filter for table id =0x01, section number=0x00 */
    if (cnxt_dmx_set_filter(gDemuxInstance, PATChannel, PATfid, PATFilterMatch,
                            PATFilterMask, (u_int8 *)NULL) != DMX_STATUS_OK)
    {
        trace_new(TL_ERROR, TRACE_FG_LIGHT_RED  "cnxt_dmx_set_filter Failed\n" TRACE_FG_NORMAL);
        return;
    }
    if (cnxt_dmx_filter_control(gDemuxInstance, PATChannel, PATfid,
                                (gencontrol_channel_t) GEN_DEMUX_ENABLE) != DMX_STATUS_OK)
    {
        trace_new(TL_ERROR, TRACE_FG_LIGHT_RED  "cnxt_dmx_filter_control Failed\n" TRACE_FG_NORMAL);
        return;
    }

    /* Set the PAT channel PID which is always 0 then enable the channel */
    cnxt_dmx_channel_set_pid(gDemuxInstance, PATChannel, pid);
    cnxt_dmx_channel_control(gDemuxInstance, PATChannel, (gencontrol_channel_t) GEN_DEMUX_ENABLE);
}

/***********************************************************************/
/* GetPMT                                                              */
/*   Set demux driver to aquire the PMT for PMTPid                     */
/*                                                                     */
/*                                                                     */
/*                                                                     */
/***********************************************************************/

void GetPMTs()
{
    u_int32 PMTChannel = GENDMX_BAD_CHANNEL, PMTfid = GENDMX_BAD_FILTER, i, j;
    void *buffer;
    bool found;

    /* reset data structures and release previous allocate PMT channels */
    InitializeForNewPAT ();

    /*
     * Handle all PMT's we'll need
     */
    for(i=0;i<NumPATEntries;i++)
    {
        /*
         * See if we have already set this PID
         */
        found = FALSE;
        for(j=0;j<i;j++)
        {
            if(PATEntryTable[i].ProgramMapPID == PATEntryTable[j].ProgramMapPID)
            {
                found = TRUE;
                PATEntryTable[i].PMTChannel = PATEntryTable[j].PMTChannel;
                PATEntryTable[i].PMTfid = PATEntryTable[j].PMTfid;
                PATEntryTable[i].PMTChannelBuffer = PATEntryTable[j].PMTChannelBuffer;
                break;
            }
        }

        if(!found)
        {
            /*
             * If we haven't already set this PID, we'll need another channel
             */

             /*
             * Allocate a section channel for the PMT
             */
            if (cnxt_dmx_channel_open(gDemuxInstance, 0,
                                      PSI_CHANNEL_TYPE, &PMTChannel) != DMX_STATUS_OK)
            {
                trace_new(TL_ERROR, TRACE_FG_LIGHT_RED  "GetPMT: cnxt_dmx_channel_open Failed\n" TRACE_FG_NORMAL);
                return;
            }

            /*
             * Register PMT notification functions
             */
            cnxt_dmx_set_section_channel_attributes(gDemuxInstance, PMTChannel,
                                                    (gen_callback_fct_t) PMT_Header_Notify,
                                                    (gen_callback_fct_t) PMT_Section_Notify, 0, 8);
            /*
             * Set PMT channel buffer
             */
            buffer = (void *)( (u_int32)mem_nc_malloc(PMT_BUFFER_SIZE) & ~NCR_BASE );
            cnxt_dmx_set_channel_buffer(gDemuxInstance, PMTChannel, buffer, PMT_BUFFER_SIZE);

            /*
             * Allocate a section filter
             */
            cnxt_dmx_filter_open(gDemuxInstance, PMTChannel, 8, &PMTfid);

            /*
             * Setup a filter for table id =0x02, section number=0x00, and
             * any program number
             */
            if (cnxt_dmx_set_filter(gDemuxInstance, PMTChannel, PMTfid,
                                    PMTFilterMatch, PMTFilterMask, NULL) != DMX_STATUS_OK)
            {
                trace_new(TL_ERROR, TRACE_FG_LIGHT_RED  "GetPMT: cnxt_dmx_set_filter Failed\n" TRACE_FG_NORMAL);
                return;
            }

            /*
             * Enable the filter
             */
            if (cnxt_dmx_filter_control(gDemuxInstance, PMTChannel, PMTfid,
                                        GEN_DEMUX_ENABLE) != DMX_STATUS_OK)
            {
                trace_new(TL_ERROR, TRACE_FG_LIGHT_RED  "GetPMT: cnxt_dmx_filter_control Failed\n" TRACE_FG_NORMAL);
                return;
            }

            PATEntryTable[i].PMTChannel       = PMTChannel;
            PATEntryTable[i].PMTfid           = PMTfid;
            PATEntryTable[i].PMTChannelBuffer = buffer;

            /*
             * Set the PMT PID
             */
            cnxt_dmx_channel_set_pid(gDemuxInstance, PMTChannel,
                                     (u_int16) PATEntryTable[i].ProgramMapPID);

            /*
             * Enable the channel
             */
            cnxt_dmx_channel_control(gDemuxInstance, PMTChannel, GEN_DEMUX_ENABLE);
        }
    }

    return;
}

/***********************************************************************/
/* GetSDT                                                              */
/*                                                                     */
/*   Setup the demux driver to aquire a SDT and then wait for one.     */
/*                                                                     */
/*                                                                     */
/***********************************************************************/
void GetSDT()
{
    void *buffer;

    /*Allocate a section channel for the SDT with 1 filter */
    if (SDTChannel == GENDMX_BAD_CHANNEL)
    {
        cnxt_dmx_channel_open(gDemuxInstance, 0, PSI_CHANNEL_TYPE,
                              &SDTChannel);

        /* Register SDT channel notification */
        cnxt_dmx_set_section_channel_attributes(gDemuxInstance, SDTChannel,
                                                (gen_callback_fct_t) SDT_Header_Notify,
                                                (gen_callback_fct_t) SDT_Section_Notify, SDT_TIMEOUT_VALUE, 8);

        /* Set channel buffer */
        buffer = (void *)( (u_int32)mem_nc_malloc(SDT_BUFFER_SIZE) & ~NCR_BASE );
        cnxt_dmx_set_channel_buffer(gDemuxInstance, SDTChannel, buffer, SDT_BUFFER_SIZE);

        cnxt_dmx_filter_open(gDemuxInstance, SDTChannel, 8, &SDTfid);
    }

    /* Inject current TSID into the SDT filter */
    SDTFilterMatch[SDT_FILTER_TSID_LO] = (u_int8)(CurrentTSID & 0xFF);
    SDTFilterMatch[SDT_FILTER_TSID_HI] = (u_int8)((CurrentTSID & 0xFF00) >> 8);
    
    /* setup a filter for table id =0x42 (SDT actual section), section number=0x00 and current TSID */
    if (cnxt_dmx_set_filter(gDemuxInstance, SDTChannel, SDTfid, SDTFilterMatch,
                            SDTFilterMask, (u_int8 *)NULL) != DMX_STATUS_OK)
    {
        trace_new(TL_ERROR, TRACE_FG_LIGHT_RED  "cnxt_dmx_set_filter Failed\n" TRACE_FG_NORMAL);
        return;
    }
    if (cnxt_dmx_filter_control(gDemuxInstance, SDTChannel, SDTfid,
                                (gencontrol_channel_t) GEN_DEMUX_ENABLE) != DMX_STATUS_OK)
    {
        trace_new(TL_ERROR, TRACE_FG_LIGHT_RED  "cnxt_dmx_filter_control Failed\n" TRACE_FG_NORMAL);
        return;
    }

    /* Set the SDT channel PID  */
    cnxt_dmx_channel_set_pid(gDemuxInstance, SDTChannel, (u_int16) SDTPID_VALUE);
    cnxt_dmx_channel_control(gDemuxInstance, SDTChannel, (gencontrol_channel_t) GEN_DEMUX_ENABLE);
}

/***********************************************************************/
/*        DumpPATEntries                                               */
/*                                                                     */
/*        Dump all programs to the serial terminal                     */
/*                                                                     */
/*                                                                     */
/***********************************************************************/
void DumpPATEntries()
{
    u_int32 i, j;

    for (i = 0; i < NumPATEntries; ++i){
        trace_new(TL_ALWAYS, "\n-------Entry #%d-------------------\n", i);
        trace_new(TL_ALWAYS, "Program Number = %d\n", PATEntryTable[i].ProgramNumber);
        if (PATEntryTable[i].ServiceName[0] != '\0')
         {
          trace_new(TL_ALWAYS, "Service Name : %s\n", PATEntryTable[i].ServiceName);
         }
        else
         {
          trace_new(TL_ALWAYS, "Service Name : NO SERVICE NAME\n");
         }
        trace_new(TL_ALWAYS, "Program Map PID= %d\n", PATEntryTable[i].ProgramMapPID);
        trace_new(TL_ALWAYS, "PCR PID        = 0x%x\n", PATEntryTable[i].PCRPID);
        trace_new(TL_ALWAYS, "Video PIDs     = ");
        for (j = 0; j < PATEntryTable[i].NumVideoPIDs; ++j)
            trace_new(TL_ALWAYS, "0x%x ", PATEntryTable[i].VideoPIDs[j]);
        trace_new(TL_ALWAYS, "\n");
        trace_new(TL_ALWAYS, "  Audio PIDs     = ");
        for (j = 0; j < PATEntryTable[i].NumAudioPIDs; ++j)
            trace_new(TL_ALWAYS, "0x%x ", PATEntryTable[i].AudioPIDs[j]);
        trace_new(TL_ALWAYS, "\n");
        trace_new(TL_ALWAYS, "   Data PIDs     = ");
        for (j = 0; j < PATEntryTable[i].NumDataPIDs; ++j)
            trace_new(TL_ALWAYS, "0x%x ", PATEntryTable[i].DataPIDs[j]);
        trace_new(TL_ALWAYS, "\n\n");
        task_time_sleep(10);
    }
}
static unsigned int PMTNotifyTag = 0;
static genfilter_mode PMT_Header_Notify(pgen_notify_t notify_data)
{
    u_int8 *header = notify_data->pData;
    u_int16 prg;
    u_int8 ver;
    u_int32 i;
    bool found;

    /*
     * Ensure we're not dealing with a timeout condition
     */

    if(notify_data->condition & GENDMX_CHANNEL_TIMEOUT)
    {
        notify_data->length = 0;
        return GENDEMUX_ONE_SHOT;   /* turn it off since we did not complete setup yet.*/
    }

    /*
     * Look for entry
     */
    found = FALSE;
    prg = (*(header+3)<<8)|(*(header+4));
    for(i=0;i<NumPATEntries;i++)
    {
        if(prg == PATEntryTable[i].ProgramNumber)
        {
            found = TRUE;
            break;
        }
    }

    /*
     * Did we find the entry?
     */
    if(found)
    {
        /*
         * Entry found, check version
         */
        ver = (*(header+5)&0x3E)>>1;
        if(ver != PATEntryTable[i].Version)
        {
            /*
             * New version, we'll take it
             */
            notify_data->skip = 0;
            notify_data->length = (*++header & 0x0F) << 8; notify_data->length |= *++header;
            notify_data->length += 4; /* for the CRC */
            notify_data->write_ptr = (u_int8 *) mem_malloc(notify_data->length);
            notify_data->tag = PMTNotifyTag++;
        }
        else
        {
            /*
             * Not newer version, skip section
             */
            notify_data->length    = 0;
            notify_data->write_ptr = 0;
        }
    }
    else
    {
        /*
         * No entry, skip section
         */
        notify_data->length    = 0;
        notify_data->write_ptr = 0;
    }

    return GENDEMUX_CONTINUOUS;
}


static void PMT_Section_Notify(pgen_notify_t notify_data)
{
    if (notify_data->write_ptr == NULL)
    {
        return;
    }

    if (notify_data->length == 0)
    {
        mem_free(notify_data->write_ptr);
        return;
    }

    if ((notify_data->tag == (PMTNotifyTag-1)) &&
        (notify_data->condition == (GENDMX_SECTION_AVAILABLE|GENDMX_CRC_CHECKED)))
    {
        ProcessPMTSection(notify_data->pData);
    }
    else
    {
        if (notify_data->condition == GENDMX_ERROR)
        {
            trace_new(TL_FUNC, "PMT_Section_Notify: SECTION_ERROR\n");
        }
    }

    mem_free(notify_data->write_ptr);
}
/**************************PAT Notify Functions ***********************/
static unsigned int NotifyTag = 0;
static genfilter_mode PAT_Header_Notify(pgen_notify_t notify_data)
{
    u_int8 *header = notify_data->pData;

    /*
     * Ensure we're not dealing with a timeout condition
     */

    if(notify_data->condition & GENDMX_CHANNEL_TIMEOUT)
    {
        notify_data->length = 0;
        return GENDEMUX_ONE_SHOT;   /* turn it off since we did not complete setup yet. */
    }

    if (notify_data->chid == PATChannel && header != NULL)
    {
        notify_data->skip = 0;
        ++header;
        notify_data->length = (*header++ & 0x0F) << 8; notify_data->length |= *header++;
        notify_data->length += 4; /* pad for the CRC*/
        notify_data->write_ptr = (u_int8 *) mem_malloc(notify_data->length);
        notify_data->tag = NotifyTag++;
    }
    else
    {
        notify_data->length = 0;
        notify_data->write_ptr = NULL;
        notify_data->tag = NotifyTag++;
    }
    return GENDEMUX_CONTINUOUS;
}

static void PAT_Section_Notify(pgen_notify_t notify_data)
{
    if (notify_data->write_ptr == NULL)
    {
        return;
    }

    if (notify_data->length == 0)
    {
        mem_free(notify_data->write_ptr);
        return;
    }

    if (notify_data->tag == NotifyTag-1 &&
        notify_data->condition == GENDMX_SECTION_AVAILABLE+GENDMX_CRC_CHECKED)
    {
        ProcessPATSection(notify_data->pData);
    }
    else
    {
        if (notify_data->condition == GENDMX_ERROR)
            trace_new(TL_ERROR, TRACE_FG_LIGHT_RED  "PAT_Section_Notfiy: SECTION_ERROR\n" TRACE_FG_NORMAL);
        if (notify_data->condition == GENDMX_CHANNEL_TIMEOUT)
            trace_new(TL_ERROR, TRACE_FG_LIGHT_RED  "PAT_Section_Notfiy: SECTION_ERROR\n" TRACE_FG_NORMAL);
    }
    mem_free(notify_data->write_ptr);
}

/*************************************************************************/

/**************************SDT Notify Functions ***********************/
static unsigned int SDTNotifyTag = 0;
static genfilter_mode SDT_Header_Notify(pgen_notify_t notify_data)
{
    u_int8 *header = notify_data->pData;

    /*
     * Ensure we're not dealing with a timeout condition
     */

    if(notify_data->condition & GENDMX_CHANNEL_TIMEOUT)
    {
        notify_data->length = 0;

        /*
         * Since the SDT has timed out we have to notify
         * the main state machine else it will stay in the
         * Getting SDT state
         */
        if(pfnDVBCallback){
            pfnDVBCallback(DVB_CTRL_GOT_SDT, 0, 0, 0);
        }    
        return GENDEMUX_ONE_SHOT;   /* turn it off since we did not complete setup yet.*/
    }

    if (notify_data->chid == SDTChannel && header != NULL)
    {
        notify_data->skip = 0;
        ++header;
        notify_data->length = (*header++ & 0x0F) << 8; notify_data->length |= *header++;
        notify_data->length += 4; /* pad for the CRC */
        notify_data->write_ptr = (u_int8 *) mem_malloc(notify_data->length);
        notify_data->tag = SDTNotifyTag++;
    }
    else
    {
        notify_data->length = 0;
        notify_data->write_ptr = NULL;
        notify_data->tag = SDTNotifyTag++;
    }
    return GENDEMUX_CONTINUOUS;
}

static void SDT_Section_Notify(pgen_notify_t notify_data)
{
    if (notify_data->write_ptr == NULL)
    {
        return;
    }

    if (notify_data->length == 0)
    {
        mem_free(notify_data->write_ptr);
        return;
    }

    if ((notify_data->tag == (SDTNotifyTag-1)) &&
        (notify_data->condition == (GENDMX_SECTION_AVAILABLE|GENDMX_CRC_CHECKED)))
    {
        cnxt_dmx_channel_control(gDemuxInstance, SDTChannel, (gencontrol_channel_t) GEN_DEMUX_DISABLE);
        ProcessSDTSection(notify_data->pData);
    }
    else
    {
        if (notify_data->condition == GENDMX_ERROR)
            trace_new(TL_ERROR, TRACE_FG_LIGHT_RED  "SDTT_Section_Notfiy: SECTION_ERROR\n" TRACE_FG_NORMAL);
        if (notify_data->condition == GENDMX_CHANNEL_TIMEOUT)
            trace_new(TL_ERROR, TRACE_FG_LIGHT_RED  "SDTT_Section_Notfiy: SECTION_ERROR\n" TRACE_FG_NORMAL);
    }
    mem_free(notify_data->write_ptr);
}

static void ProcessPATSection(u_int8 *pBuffer)
{
    int N;
    int table_id, sec_syntax_indicator, section_length, transport_stream_id, version_number;
    int current_next_indicator, section_number, last_section_number, program_number;
    u_int16 newprogram_map_PID;
    u_int16 network_PID = (u_int16) GEN_INVALID_PID;

    table_id = *pBuffer++;
    sec_syntax_indicator = (*pBuffer & 0x80) >> 7;
    section_length = (*pBuffer++ & 0x0F) << 8; section_length |= *pBuffer++;
    transport_stream_id = (*pBuffer++ << 8);    transport_stream_id |= (*pBuffer++);
    version_number = (*pBuffer & 0x3E) >> 1;
    current_next_indicator = *pBuffer++ & 0x01;
    section_number = *pBuffer++;
    last_section_number = *pBuffer++;
    N = (section_length - 8) / 4;

    trace_new(TL_INFO, "\n");
    trace_new(TL_INFO, "Program Association Table\n");
    trace_new(TL_INFO, "-------------------------\n");

    trace_new(TL_INFO, "table_id            = %d\n", table_id);
    trace_new(TL_INFO, "sec_syntax_indicator= %d\n", sec_syntax_indicator);
    trace_new(TL_INFO, "section_length      = %d\n", section_length);
    trace_new(TL_INFO, "transport_stream_id = %d\n", transport_stream_id);
    trace_new(TL_INFO, "version_number      = %d\n", version_number);
    trace_new(TL_INFO, "current_next_indicator = %d\n", current_next_indicator);
    trace_new(TL_INFO, "section_number      = %d\n", section_number);
    trace_new(TL_INFO, "last_section_number = %d\n", last_section_number);

    if (table_id != 0x00 || sec_syntax_indicator != 0x01 || section_length > 1021 || current_next_indicator != 1){
        trace_new(TL_ERROR, TRACE_FG_LIGHT_RED  "Invalid PAT section. Ignoring.\n" TRACE_FG_NORMAL);
        return;
    }

    if (section_number == 0){
        NumPATEntries = 0;
    }
    while (N--){
        program_number = (*pBuffer++ << 8); program_number |= *pBuffer++;
        trace_new(TL_INFO, "program_number  = %d\n", program_number);
        if (program_number == 0){
            network_PID = ((*pBuffer++ & 0x1F) << 8); network_PID |= *pBuffer++;
            trace_new(TL_INFO, "network_PID = %d\n", network_PID);
        }
        else{
            newprogram_map_PID = ((*pBuffer++ & 0x1F) << 8); newprogram_map_PID |= *pBuffer++;
            if (NumPATEntries < MAX_PMT_ENTRIES){
                PATEntryTable[NumPATEntries].ProgramNumber = program_number;
                PATEntryTable[NumPATEntries++].ProgramMapPID = newprogram_map_PID;
            }
            trace_new(TL_INFO, "program_map_PID = %d\n", newprogram_map_PID);
        }
    }

    if (section_number == last_section_number){
       /*
         Then change the filter to look for the next version of the PAT and section=0
         tableID=0x00, version_number=next version number, section number=0x00
        */
#if PROCESS_NEXT_PAT_VERSION        
        PATFilterMatch[6] = 0x00;    /* section number */
        PATFilterMatch[5] = ++version_number << 1;
        PATFilterMask[5]  = 0x3E;
        trace(
                TRACE_FG_LIGHT_BLUE
                    "processing PAT version %d\n"
                TRACE_FG_NORMAL,
                version_number
             );
        
        if (cnxt_dmx_set_filter(gDemuxInstance, PATChannel, PATfid, PATFilterMatch,
                                PATFilterMask, (u_int8 *) NULL) != DMX_STATUS_OK)
        {
            trace_new(TL_ERROR, TRACE_FG_LIGHT_RED  "cnxt_dmx_set_filter Failed\n" TRACE_FG_NORMAL);
        }
#else
        u_int8 *chan_buffer;
        u_int32 chan_buffer_size;

        if (cnxt_dmx_filter_control(gDemuxInstance, PATChannel, PATfid, GEN_DEMUX_DISABLE) != 
                                    DMX_STATUS_OK)
        {
            trace_new(TL_ERROR,
                      TRACE_FG_LIGHT_RED  
                         "cnxt_dmx_filter_control Failed while disabling PAT filter\n"
                      TRACE_FG_NORMAL);
        }
        
        if (cnxt_dmx_filter_close(gDemuxInstance, PATChannel, PATfid) != 
                                    DMX_STATUS_OK)
        {
            trace_new(TL_ERROR,
                      TRACE_FG_LIGHT_RED  
                         "cnxt_dmx_filter_close Failed while closing PAT filter\n"
                      TRACE_FG_NORMAL);
        }

        if (cnxt_dmx_channel_control(gDemuxInstance, PATChannel, GEN_DEMUX_DISABLE) !=
                DMX_STATUS_OK)
        {
            trace_new(TL_ERROR,
                      TRACE_FG_LIGHT_RED  
                         "cnxt_dmx_channel_control Failed while disabling PAT channel\n"
                      TRACE_FG_NORMAL);
        }

        cnxt_dmx_get_channel_buffer_pointer ( gDemuxInstance, PATChannel, &chan_buffer, &chan_buffer_size );
        mem_nc_free ( chan_buffer );
        if (cnxt_dmx_channel_close(gDemuxInstance, PATChannel) != 
                DMX_STATUS_OK)
        {
            trace_new(TL_ERROR,
                      TRACE_FG_LIGHT_RED  
                         "cnxt_dmx_channel_close Failed while closing PAT channel\n"
                      TRACE_FG_NORMAL);
        }

	PATChannel = GENDMX_BAD_CHANNEL;	// ÀµÔÆÁ¼added 20051122

#endif
        
        if(pfnDVBCallback){
            pfnDVBCallback(DVB_CTRL_GOT_PAT, (u_int32)transport_stream_id, 0, 0);
        }    
    }
    else
    {
        PATFilterMatch[6]++;         /* look for next section number */
        if (cnxt_dmx_set_filter(gDemuxInstance, PATChannel, PATfid, PATFilterMatch,
                                PATFilterMask, (u_int8 *) NULL) != DMX_STATUS_OK)
        {
            trace_new(TL_ERROR, TRACE_FG_LIGHT_RED  "cnxt_dmx_set_filter Failed\n" TRACE_FG_NORMAL);
        }
    }
}

/***********************************************************************/
/* ProcessPMTSection                                                   */
/*   Parses a PMT to extract program information(Audio/Video/PCR PIDs) */
/*                                                                     */
/*                                                                     */
/*                                                                     */
/***********************************************************************/
static void ProcessPMTSection(u_int8 *pBuffer)
{
    int Len, RemainingLength;
    u_int32 i;
    int table_id, sec_syntax_indicator, section_length, program_number, version_number;
    int current_next_indicator, section_number, last_section_number, program_info_length;
    int PATTableIndex;
    u_int16 PCR_PID;
    bool bNewPMT;

    table_id = *pBuffer++;
    sec_syntax_indicator = (*pBuffer & 0x80) >> 7;
    section_length = (*pBuffer++ & 0x0F) << 8; section_length |= *pBuffer++;
    program_number = (*pBuffer++ << 8);    program_number |= (*pBuffer++);

    PATTableIndex = -1;
    for (i=0; i<NumPATEntries; i++)
    {
        if (program_number == PATEntryTable[i].ProgramNumber)
        {
            PATTableIndex = i;
            break;
        }
    }

    if (PATTableIndex == -1)
    {
        trace_new(TL_ALWAYS, "Error: Program Number not in PAT, skipping PMT...\n");
        return;
    }

    version_number = (*pBuffer & 0x3E) >> 1;

    if(version_number == PATEntryTable[PATTableIndex].Version)
    {
        return;
    }

    current_next_indicator = *pBuffer++ & 0x01;
    section_number = *pBuffer++;
    last_section_number = *pBuffer++;
    PCR_PID = (*pBuffer++ & 0x1F) << 8; PCR_PID |= *pBuffer++;
    PATEntryTable[i].PCRPID = PCR_PID;

    program_info_length = (*pBuffer++ & 0x0F) << 8; program_info_length |= *pBuffer++;
    pBuffer += program_info_length;

    trace_new(TL_INFO, "\n");
    trace_new(TL_INFO, "Got Program Map Table\n");
    trace_new(TL_INFO, "-------------------------\n");

    trace_new(TL_INFO, "table_id            = %d\n", table_id);
    trace_new(TL_INFO, "sec_syntax_indicator= %d\n", sec_syntax_indicator);
    trace_new(TL_INFO, "section_length      = %d\n", section_length);
    trace_new(TL_INFO, "program_number      = %d\n", program_number);
    trace_new(TL_INFO, "version_number      = %d\n", version_number);
    trace_new(TL_INFO, "current_next_indicator = %d\n", current_next_indicator);
    trace_new(TL_INFO, "section_number      = %d\n", section_number);
    trace_new(TL_INFO, "last_section_number = %d\n", last_section_number);
    trace_new(TL_INFO, "PCR_PID             = %d\n", PCR_PID);
    trace_new(TL_INFO, "program_info_length = %d\n", program_info_length);

    if (table_id != 0x02 || sec_syntax_indicator != 0x01 || section_length > 1021 || current_next_indicator != 1)
    {
        trace_new(TL_ERROR, TRACE_FG_LIGHT_RED  "Invalid PMT section. Ignoring.\n" TRACE_FG_NORMAL);
        return;
    }

    RemainingLength = section_length - (9 + program_info_length); /*was 12 */
    do {
        Len = ProcessProgramDesc(pBuffer, PATTableIndex);
        pBuffer +=  Len;
        RemainingLength -= Len;
    } while (RemainingLength > 0);
    trace_new(TL_INFO, "\n----------------------------\n");

    /*
     * Look for next PMT version
     */
    bNewPMT = (PATEntryTable[i].Version == -1);
    PATEntryTable[i].Version = version_number;

    /* Send a message to the main task that we got a PMT */
    if(pfnDVBCallback)
    {
        if(bNewPMT)
        {
            pfnDVBCallback(DVB_CTRL_GOT_PMT, PATTableIndex, program_number, 0);
        }
        else
        {
            /* This is a new version of a PMT already reported */
            trace_new(TL_ERROR, TRACE_FG_LIGHT_RED "Updated PMT for program number %d\n" TRACE_FG_NORMAL, program_number);
        }
    }    
}

static int ProcessProgramDesc(u_int8 *pBuffer, u_int32 PATIndex)
{
    int stream_type, elementary_PID, ES_info_length;
    int ret = 0;
    int desc_tag, desc_length;
    u_int32 CA_Index;

    stream_type = *pBuffer++;
    elementary_PID = (*pBuffer++ & 0x1F) << 8; elementary_PID |= *pBuffer++;
    ES_info_length = (*pBuffer++ & 0x0F) << 8; ES_info_length |= *pBuffer++;
    trace_new(TL_INFO, "stream_type = %d, elementary_PID = %d\n", stream_type, elementary_PID);
    switch(stream_type){
        case 0:
            break;
        case 1:         /* Video */
        case 2:
        case 0x80:
            if (PATEntryTable[PATIndex].NumVideoPIDs < MAX_PID_ENTRIES)
                PATEntryTable[PATIndex].VideoPIDs[PATEntryTable[PATIndex].NumVideoPIDs++] = elementary_PID;
            break;
        case 3:        /* audio */
        case 4:
        case 0x81:
            if (PATEntryTable[PATIndex].NumAudioPIDs < MAX_PID_ENTRIES)
            {
                PATEntryTable[PATIndex].AudioPIDs[PATEntryTable[PATIndex].NumAudioPIDs] = elementary_PID;
                PATEntryTable[PATIndex].AudioTypes[PATEntryTable[PATIndex].NumAudioPIDs++] = stream_type;
            }
            break;
        case 6:         /* Data (teletext)*/
            if (PATEntryTable[PATIndex].NumDataPIDs < MAX_PID_ENTRIES)
                PATEntryTable[PATIndex].DataPIDs[PATEntryTable[PATIndex].NumDataPIDs++] = elementary_PID;
            break;
        default:
            break;
    }
    trace_new(TL_INFO, "ES_info_length = %d\n", ES_info_length);

    ret += (5+ES_info_length);

    CA_Index = 0;
    while (ES_info_length > 0)
    {
        desc_tag = *pBuffer++;
        ES_info_length--;
        switch(desc_tag)
        {
            case 2:                 /* video stream desc tag */
            case 3:                 /* audio stream desc tag */
                desc_length = (int) *pBuffer++;
                ES_info_length--;
                pBuffer += desc_length;
                ES_info_length -= desc_length;
                break;

#if PARSER_PASSAGE_ENABLE==YES
            case 9:                  /* CA desc tag */
                desc_length = (int) *pBuffer;
                pBuffer += 3;        /* skip ca_system_id */
                elementary_PID = (*pBuffer++ & 0x1F) << 8; elementary_PID |= *pBuffer++;
                ES_info_length -=5;
                desc_length -= 4;
                if ( ( stream_type == 1 ) || ( stream_type == 2 ) || ( stream_type == 0x80 ) )
                {
                    if ( ( desc_length == 0 ) || ( *pBuffer == 0 ) )
                    {
                        PATEntryTable[PATIndex].ShadowVideoPid = 0;
                    }
                    else
                    {
                        PATEntryTable[PATIndex].ShadowVideoPid = elementary_PID;
                    }
                }
                else if ( ( stream_type == 3 ) || ( stream_type == 4 ) || ( stream_type == 0x81 ) )
                {
                    if ( ( desc_length == 0 ) || ( *pBuffer == 0 ) )
                    {
                        PATEntryTable[PATIndex].ShadowAudioPid = 0;
                    }
                    else
                    {
                        PATEntryTable[PATIndex].ShadowAudioPid = elementary_PID;
                    }
                }
                pBuffer += desc_length;
                ES_info_length -= desc_length;
                break;
#endif                                

            default:                /* unhandled Desc tag */
                trace_new(TL_INFO, "---->Unhandled Descriptor Tag=0x%x\n", desc_tag);
                desc_length = (int) *pBuffer++;
                ES_info_length--;
                pBuffer += desc_length;
                ES_info_length -= desc_length;
                break;
        }
        CA_Index++;
    }
    return ret;
}

/********************************************************************/
/*  FUNCTION:    ProcessSDTSectiom                                  */
/*                                                                  */
/*  PARAMETERS:  Pointer to a Complete SDT section.                 */
/*                                                                  */
/*  DESCRIPTION: This routine is a wrapper which calls the SDT      */
/*               parser code to parse and extract the service       */
/*               information and then signals the main Task to set  */
/*               the application state machine to the next state.   */
/*                                                                  */
/*  RETURNS:     no return definition.                              */
/*  CONTEXT:     Must be called in task context                     */
/*                                                                  */
/********************************************************************/
static void ProcessSDTSection(u_int8 *pBuffer)
{
   u_int8* SDTptr=pBuffer;

   GetServiceInfo(SDTptr);

   if(pfnDVBCallback){
      pfnDVBCallback(DVB_CTRL_GOT_SDT, 0, 0, 0);
   }    

   return;
}


/* Useful Macros defined */
/* Convert2Word -> converts base byte ptr into a 16bit value with
               masking applied to upper and lower bytes */
#define Convert2Word(bytePtr,MaskUbyte,MaskLbyte,Offset) \
                (((*bytePtr&MaskUbyte)<<8)|((*(bytePtr+1)&MaskLbyte))); \
                bytePtr+=Offset;

/* Code to remove 'special' characters.
   See Appendix A of ETS 300 468       */
#define RemoveUndefinedCharacters(string,length,count) \
           for (count=0;count<length;count++)          \
              {                                        \
               if ( (string[count] <= 0x1F) || (string[count] >= 0x7F) )            \
                         {string[count] = 0x20; } /* Convert to space character */  \
              }

#define CRC_FIELD                         4
#define SDT_HEADER_SIZE                   8
#define SDT_SERVICE_LOOP_HEADER_SIZE      5
#define SERVICE_DESC_TAG                  0x48 /* Marker tag to look for. Service Descriptor */
#define DESC_HEADER                       2

/********************************************************************/
/*  FUNCTION   : GetServiceInfo                                     */
/*                                                                  */
/*  PARAMETERS : Pointer to a Complete SDT section.                 */
/*                                                                  */
/*  DESCRIPTION: Parses an SDT Actual section to retrieve Service   */
/*               information.Service Provider Name and Service Name.*/
/*  REFERENCE  : Specification ETS 300 468.                         */
/*               Title : Specification for Service Information (SI) */
/*               in DVB systems.                                    */
/*                                                                  */
/*  RETURNS    : 0==success.1==Fail.                                */
/*  CONTEXT    : Must be called in task context                     */
/*                                                                  */
/*  NOTES      : This rotines uses ProgramNumber in PAT to          */
/*               associate the service information.                 */
/*               If you need to understand what this algorithm is   */
/*               doing please refer to document ETS 300 468.        */
/********************************************************************/
static u_int32 GetServiceInfo(u_int8 *SDTptr)
{
 u_int8  *sdtptr=SDTptr,desc_length;
 u_int16 section_length;
 u_int16 service_loop_cnt=0;
 u_int16 desc_loop_length;
 u_int8  act_length;
 u_int16 CurrentService_id;
 int     ii,cc;


 if ((*sdtptr != SDT_ACTUAL_TABLEID) && (*sdtptr != SDT_OTHER_TABLEID)) /* check that it's the right table ! */
 {
   trace_new(TL_INFO, "Error ! expecting SDT, got table ID: 0x%x",*sdtptr);
   return (1);
 }

 sdtptr++;
 section_length   = Convert2Word(sdtptr,0x0F,0xFF,10);   /* extract section_length and move offset 10 bytes */
 service_loop_cnt = (section_length - (CRC_FIELD+SDT_HEADER_SIZE));


 while (service_loop_cnt != 0)
  {
    CurrentService_id = Convert2Word(sdtptr,0xFF,0xFF,3);
    desc_loop_length  = Convert2Word(sdtptr,0x0F,0xFF,2);

    service_loop_cnt -= (desc_loop_length+SDT_SERVICE_LOOP_HEADER_SIZE);

   if (desc_loop_length)
   {
    while (desc_loop_length != 0)
      {
        if (*sdtptr == SERVICE_DESC_TAG)
         {
          desc_length=*(++sdtptr);sdtptr+=2;

          if (desc_length)
          {
           if (*sdtptr) /* does service provider name exist? (check length>0) */
            {
             /* ensure that the string in this section is less than what we can hold */
             act_length = (*sdtptr< (SIZEOF_SERVICEINFO))? *sdtptr : (SIZEOF_SERVICEINFO-1);

             /* Search out Main Table until Program number (PAT) is equal to The required Service ID.
                if so then this is the associated service provider name for this program */
             for (ii=0;ii<NumPATEntries;ii++)
              {
               if (PATEntryTable[ii].ProgramNumber == CurrentService_id)
                 { break;  }
              }
              if (ii < NumPATEntries) /* if a valide service id found extract the name string */
                {
                 strncpy( (char*) PATEntryTable[ii].ServiceProvName,(char*) (sdtptr+1), act_length);
                 RemoveUndefinedCharacters(PATEntryTable[ii].ServiceProvName,act_length,cc);
                }
             sdtptr+=((*sdtptr)+1); /* OK! Got the info now get the service name */
            }
           else
            { sdtptr++; }

             if (*sdtptr) /* does service name exist? check length>0 */
              {
               /* ensure that the string in this section is less than what we can hold */
               act_length =    (*sdtptr< (SIZEOF_SERVICEINFO))? *sdtptr : (SIZEOF_SERVICEINFO-1);

                /* Search out Main Table until Program number (PAT) is equal to The required Service ID.
                   if so then this is the associated service name for this program */
               for (ii=0;ii<NumPATEntries;ii++)
                {
                 if (PATEntryTable[ii].ProgramNumber == CurrentService_id)
                   { break; }
                }

               if (ii < NumPATEntries)
                {
                 strncpy( (char*) PATEntryTable[ii].ServiceName,(char*) (sdtptr+1), act_length);
                 RemoveUndefinedCharacters(PATEntryTable[ii].ServiceName,act_length,cc);
                }
                sdtptr+=(*(sdtptr)+1); /* interate to next point */
              }
             else
              { sdtptr++; }
              desc_loop_length -= (desc_length+DESC_HEADER);
          } /* if desc_length */
         }
        else
         {
          desc_length=*(++sdtptr);
          sdtptr+=(desc_length+1);
          desc_loop_length -= (desc_length+DESC_HEADER);
         }
      }
   } /* if (desc_loop_length) */
  }
 return (0);
}

/***********************************************************************/
/* cnxt_dvb_find_next_program                                          */
/*  Sets CurrentProgram to the next Program with a video PID.          */
/*                                                                     */
/***********************************************************************/

bool cnxt_dvb_find_next_program(int *pNextProgramNumber)
{
    u_int16 OldProg = CurrentProgram;
    u_int16 CP;
    bool ret = FALSE;
    
    if ( !pNextProgramNumber ){
        trace_new(TL_ERROR, "cnxt_dvb_find_next_program: NULL pointer passed\n");
        return ret;
    }
        
    CP = OldProg;
    
    while (++CP != OldProg){
        if (CP == NumPATEntries)
            CP = 0;
        if (PATEntryTable[CP].VideoPIDs[0] != GEN_INVALID_PID ||
            PATEntryTable[CP].AudioPIDs[0] != GEN_INVALID_PID ||
            PATEntryTable[CP].DataPIDs[0]  != GEN_INVALID_PID){
            break;
         }
    }
    
    if (CP != OldProg || NumPATEntries == 1)
    {
        *pNextProgramNumber = (int)PATEntryTable[CP].ProgramNumber;
        ret = TRUE;
    }

    return ret;
}
bool cnxt_dvb_find_prev_program(int *pPrevProgramNumber)
{
    u_int16 OldProg = CurrentProgram;
    int CP;
    bool ret = FALSE;
    
    if ( !pPrevProgramNumber ){
        trace_new(TL_ERROR, "cnxt_dvb_find_prev_program: NULL pointer passed\n");
        return ret;
    }
    
    CP = (int) OldProg;
    
    while (--CP != OldProg){
        if (CP < 0)
            CP = NumPATEntries-1;
        if (PATEntryTable[CP].VideoPIDs[0] != GEN_INVALID_PID ||
            PATEntryTable[CP].AudioPIDs[0] != GEN_INVALID_PID ||
            PATEntryTable[CP].DataPIDs[0]  != GEN_INVALID_PID){
            break;
         }
    }
    
    if (CP != OldProg || NumPATEntries == 1)
    {
        *pPrevProgramNumber = (int)PATEntryTable[CP].ProgramNumber;
        ret = TRUE;
    }
    return ret;
}
/***********************************************************************/
/* cnxt_dvb_set_program_number                                                    */
/*                                                                     */
/*   Highest level function for switching programs.                    */
/*                                                                     */
/*                                                                     */
/***********************************************************************/
bool cnxt_dvb_set_program_number(int ProgramNumber )
{
    u_int32 i = 0;

    CurrentAudio = 0;
    CurrentVideo = 0;
    CurrentData  = 0;

    for ( i = 0; i < NumPATEntries; ++i)
        if (PATEntryTable[i].ProgramNumber == ProgramNumber)
            break;

    if (i < NumPATEntries)
    {
        CurrentProgram = i;
        CurrentProgramNumber = PATEntryTable[CurrentProgram].ProgramNumber;
        if (PATEntryTable[CurrentProgram].VideoPIDs[0] != GEN_INVALID_PID ||
            PATEntryTable[CurrentProgram].AudioPIDs[0] != GEN_INVALID_PID)
        {
            return TRUE;
        } else {
          trace_new(TL_INFO, "cnxt_dvb_set_program_number: Invalid PID found found!\n");
        }
    } else {
      trace_new(TL_INFO, "cnxt_dvb_set_program_number: Program Number not found!\n");
    }

    return FALSE;
}
/***********************************************************************/
/* cnxt_dvb_get_current_program_pat_entry                              */
/*                                                                     */
/*   get PATEntry[CurrentProgram] structure                            */
/*                                                                     */
/*                                                                     */
/***********************************************************************/

void cnxt_dvb_get_current_program_pat_entry(PATEntry *pCurrentEntry, u_int32 *puCurrentVideo, u_int32 *puCurrentAudio, u_int32 *puCurrentData )
{
   if( !pCurrentEntry || !puCurrentVideo || !puCurrentAudio || !puCurrentData ){
       trace_new(TL_ERROR, "cnxt_dvb_get_current_program_pat_entry: NULL pointer passed\n");
       return;
   }    
   
   memcpy(pCurrentEntry, PATEntryTable+CurrentProgram, sizeof(PATEntry));
   *puCurrentVideo = CurrentVideo;
   *puCurrentAudio = CurrentAudio;
   *puCurrentData = CurrentData;
}

/***********************************************************************/
/* cnxt_dvb_get_next_or_prev_audio                                     */
/*  Sets CurrentAudio to the next audio or prev audio                  */
/*  when multiple audio PIDs exist                                     */
/*                                                                     */
/*                                                                     */
/***********************************************************************/
bool cnxt_dvb_get_next_or_prev_audio(bool bNext, u_int16* puCurrentAudioPID)
{
    u_int32 OldAudio = CurrentAudio;

    if ( !puCurrentAudioPID ) {
        trace_new(TL_ERROR, "cnxt_dvb_get_next_or_prev_audio: NULL pointer passed\n");
        return FALSE;
    }
       
    /* If there is more than one valid audio PID, look for the next one. */
    if ( PATEntryTable[CurrentProgram].NumAudioPIDs > 1 )
    {
        *puCurrentAudioPID = GEN_INVALID_PID;

        if (bNext){ /* get next audio */
            while (++CurrentAudio != OldAudio){
                if (CurrentAudio >= PATEntryTable[CurrentProgram].NumAudioPIDs)
                    CurrentAudio = 0;
                if (PATEntryTable[CurrentProgram].AudioPIDs[CurrentAudio] != GEN_INVALID_PID){
                    *puCurrentAudioPID = PATEntryTable[CurrentProgram].AudioPIDs[CurrentAudio];
                    break;
                }
            }
        }
        else {  /* get prev audio */
            while (--CurrentAudio != OldAudio){
                if (CurrentAudio & 0x80000000)
                    CurrentAudio = PATEntryTable[CurrentProgram].NumAudioPIDs -1 ;
                if (PATEntryTable[CurrentProgram].AudioPIDs[CurrentAudio] != GEN_INVALID_PID){
                    *puCurrentAudioPID = PATEntryTable[CurrentProgram].AudioPIDs[CurrentAudio];
                    break;
                }
            }
        } 
        if (CurrentAudio != OldAudio){
            return TRUE;
        }       
    }
    return FALSE;
}

/***********************************************************************/
/* cnxt_dvb_get_next_or_prev_video                                     */
/*  Sets CurrentVideo to the next video or the prev video              */
/*  when multiple video PIDs exist                                     */
/*                                                                     */
/*                                                                     */
/*                                                                     */
/***********************************************************************/
bool cnxt_dvb_get_next_or_prev_video(bool bNext, u_int16* puCurrentVideoPID, u_int16* puCurrentAudioPID, 
                                     u_int16* puCurrentPCRPID, u_int16* puCurrentDataPID)
{
    u_int32 OldVideo = CurrentVideo;
    
    if( !puCurrentVideoPID || !puCurrentAudioPID || !puCurrentPCRPID || !puCurrentDataPID ) {
        trace_new(TL_ERROR, "cnxt_dvb_get_next_or_prev_video: NULL pointer passed\n");
        return FALSE;
    }
    
    /* If there is more than one valid video PID, look for the next one. */
    if ( PATEntryTable[CurrentProgram].NumVideoPIDs > 1 )
    {
        *puCurrentVideoPID = GEN_INVALID_PID;
        *puCurrentAudioPID = GEN_INVALID_PID;
        *puCurrentPCRPID   = GEN_INVALID_PID;
        *puCurrentDataPID  = GEN_INVALID_PID;

        if (bNext){ /* get next video */
            while (++CurrentVideo != OldVideo){
                if (CurrentVideo >= PATEntryTable[CurrentProgram].NumAudioPIDs)
                    CurrentVideo = 0;
                if (PATEntryTable[CurrentProgram].VideoPIDs[CurrentVideo] != GEN_INVALID_PID){
                    *puCurrentAudioPID = PATEntryTable[CurrentProgram].AudioPIDs[CurrentAudio];
                    *puCurrentVideoPID = PATEntryTable[CurrentProgram].VideoPIDs[CurrentVideo];
                    *puCurrentPCRPID   = PATEntryTable[CurrentProgram].PCRPID;
                    *puCurrentDataPID  = PATEntryTable[CurrentProgram].DataPIDs[CurrentData];
                    break;
                }
            }
        }
        else{ /* get prev video */
            while (--CurrentVideo != OldVideo){
                if (CurrentVideo & 0x80000000)
                    CurrentVideo = PATEntryTable[CurrentProgram].NumVideoPIDs -1 ;
                if (PATEntryTable[CurrentProgram].VideoPIDs[CurrentVideo] != GEN_INVALID_PID){
                    *puCurrentAudioPID = PATEntryTable[CurrentProgram].AudioPIDs[CurrentAudio];
                    *puCurrentVideoPID = PATEntryTable[CurrentProgram].VideoPIDs[CurrentVideo];
                    *puCurrentPCRPID   = PATEntryTable[CurrentProgram].PCRPID;
                    *puCurrentDataPID  = PATEntryTable[CurrentProgram].DataPIDs[CurrentData];
                    break;
                }
             }
        }
        if (CurrentVideo != OldVideo){
            return TRUE;
        }
    }
    return FALSE;    
}

/****************************************************************************************/
/* cnxt_dvb_get_next_or_prev_data                                                       */
/*  Sets CurrentData to the next data or the prev data when multiple data PIDs exist    */
/*                                                                                      */
/*                                                                                      */
/*                                                                                      */
/****************************************************************************************/
bool cnxt_dvb_get_next_or_prev_data(bool bNext, u_int16* puCurrentVideoPID, u_int16* puCurrentAudioPID,
                                    u_int16* puCurrentPCRPID, u_int16* puCurrentDataPID)
{
    u_int32 OldData = CurrentData;
    
    if( !puCurrentVideoPID || !puCurrentAudioPID || !puCurrentPCRPID || !puCurrentDataPID ) {
        trace_new(TL_ERROR, "cnxt_dvb_get_next_or_prev_data: NULL pointer passed\n");
        return FALSE;
    }
    /* If there is more than one valid video PID, look for the next one. */
    if ( PATEntryTable[CurrentProgram].NumDataPIDs > 1 )
    {
        *puCurrentDataPID = GEN_INVALID_PID;
        *puCurrentAudioPID = GEN_INVALID_PID;
        *puCurrentPCRPID   = GEN_INVALID_PID;
        *puCurrentDataPID  = GEN_INVALID_PID;

        if (bNext){ /* get next data */
            while (++CurrentData != OldData){
                if (CurrentData >= PATEntryTable[CurrentProgram].NumDataPIDs)
                    CurrentData = 0;
                if (PATEntryTable[CurrentProgram].VideoPIDs[CurrentData] != GEN_INVALID_PID){
                    *puCurrentAudioPID = PATEntryTable[CurrentProgram].AudioPIDs[CurrentAudio];
                    *puCurrentVideoPID = PATEntryTable[CurrentProgram].VideoPIDs[CurrentVideo];
                    *puCurrentPCRPID   = PATEntryTable[CurrentProgram].PCRPID;
                    *puCurrentDataPID  = PATEntryTable[CurrentProgram].DataPIDs[CurrentData];
                    break;
                }
            }
        }
        else{ /* get prev data */
            while (--CurrentData != OldData){
                if (CurrentData & 0x80000000)
                    CurrentData = PATEntryTable[CurrentProgram].NumDataPIDs -1 ;
                if (PATEntryTable[CurrentProgram].DataPIDs[CurrentData] != GEN_INVALID_PID){
                    *puCurrentAudioPID = PATEntryTable[CurrentProgram].AudioPIDs[CurrentAudio];
                    *puCurrentVideoPID = PATEntryTable[CurrentProgram].VideoPIDs[CurrentVideo];
                    *puCurrentPCRPID   = PATEntryTable[CurrentProgram].PCRPID;
                    *puCurrentDataPID  = PATEntryTable[CurrentProgram].DataPIDs[CurrentData];
                    break;
                }
             }
        }
        if (CurrentData != OldData){
            return TRUE;
        }
    }
    return FALSE;    
}

/***********************************************************************/
/* cnxt_dvb_get_num_pat_entries                                        */
/*  return the number of PAT entries                                   */
/*                                                                     */
/*                                                                     */
/***********************************************************************/
u_int32 cnxt_dvb_get_num_pat_entries()
{
    return NumPATEntries;
}  

/***********************************************************************/
/* cnxt_dvb_select_first_program                                       */
/*  Gets the first program stored in EEPROM                            */
/*                                                                     */
/*                                                                     */
/*                                                                     */
/***********************************************************************/
void cnxt_dvb_select_first_program()
{
   sabine_config_data *pConfig;
   int i;

   pConfig = config_lock_data();
   for (i = 0; i < NumPATEntries; i++) {
      if (PATEntryTable[i].ProgramNumber == pConfig->svl_service) {
         CurrentProgram = i - 1;
         break;
      }
   }
   config_unlock_data(pConfig);
   if (i == NumPATEntries) {
      CurrentProgram =  NumPATEntries - 1;
   }
   CurrentProgramNumber = PATEntryTable[CurrentProgram].ProgramNumber;
}

/***********************************************************************/
/* cnxt_dvb_set_currentTSIData                                         */
/*  Sets CurrentTSID to the specified value                            */
/*                                                                     */
/*                                                                     */
/*                                                                     */
/***********************************************************************/
void cnxt_dvb_set_currentTSID(u_int32 uCurrentTSID)
{
   CurrentTSID = uCurrentTSID;
}   
/***********************************************************************/
/* cnxt_dvb_get_current_program                                        */
/*  Gets CurrentProgram                                                */
/*                                                                     */
/*                                                                     */
/*                                                                     */
/***********************************************************************/
u_int32 cnxt_dvb_get_current_program()
{
   return CurrentProgram;
}  
/***********************************************************************/
/* cnxt_dvb_get_program_number                                         */
/*  Gets the program number of the specified program                   */
/*                                                                     */
/*                                                                     */
/*                                                                     */
/***********************************************************************/
u_int32 cnxt_dvb_get_program_number(u_int32 uProgram)
{
  return PATEntryTable[uProgram].ProgramNumber;
}

int PrintVideoDesc(u_int8 *pBuffer)
{
    int desc_tag, desc_length, multiple_frame_rate_flag, frame_rate_code, MPEG_1_Only_flag, constrained_parameter_flag;
    int still_picture_flag;

    desc_tag = *pBuffer++;
    desc_length = *pBuffer++;
    multiple_frame_rate_flag = (*pBuffer & 0x80) >> 7;
    frame_rate_code = (*pBuffer & 0x78) >> 3;
    MPEG_1_Only_flag = (*pBuffer & 0x40) >> 2;
    constrained_parameter_flag = (*pBuffer & 0x20) >> 1;
    still_picture_flag = (*pBuffer & 0x01);

    trace_new(TL_INFO, "   ---Video Desc---\n");
    trace_new(TL_INFO, "   desc_tag                = %d\n", desc_tag);
    trace_new(TL_INFO, "   desc_length             = %d\n", desc_length);
    trace_new(TL_INFO, "   multiple_frame_rate_flag= %d\n", multiple_frame_rate_flag);
    trace_new(TL_INFO, "   frame_rate_code         = %d\n", frame_rate_code);
    trace_new(TL_INFO, "   MPEG_1_Only_flag        = %d\n", MPEG_1_Only_flag);
    trace_new(TL_INFO, "   constrained_parameter_flag = %d\n", constrained_parameter_flag);
    trace_new(TL_INFO, "   still_picture_flag      = %d\n", still_picture_flag);

    if (MPEG_1_Only_flag == 1){
        trace_new(TL_INFO, "MPEG 1 Only Flag\n");
    }
    trace_new(TL_INFO, "   ----------------\n");

    return desc_length+2;                   // 2 bytes added for the tag & length fields

}

int PrintCADesc(u_int8 *pBuffer)
{
    int desc_tag, desc_length;
    u_int16 CA_system_ID;
    int i;
    u_int16 CA_PID;

    desc_tag = *pBuffer++;
    desc_length = *pBuffer++;

    CA_system_ID = (*pBuffer++ << 8); CA_system_ID |= *pBuffer++;
    CA_PID = (*pBuffer++ & 0x1F) << 8; CA_PID |= *pBuffer++;

    trace_new(TL_INFO, "   ---CA Desc---\n");
    trace_new(TL_INFO, "   desc_tag                = %d\n", desc_tag);
    trace_new(TL_INFO, "   desc_length             = %d\n", desc_length);
    trace_new(TL_INFO, "   CA_system_ID            = %d\n", CA_system_ID);
    trace_new(TL_INFO, "   CA_PID                  = %d\n", CA_PID);

    if (desc_length > 4){
        trace_new(TL_INFO, "          private data = ");
        for (i = 0; i < desc_length - 4; ++ i)
                trace_new(TL_INFO, "%X ", *pBuffer++);
        trace_new(TL_INFO, "\n          -----------\n");
    }

    trace_new(TL_INFO, "   ----------------\n");
    return desc_length+2;                   // 2 bytes added for the tag & length fields

}

int PrintDesc(u_int8 *pBuffer)
{
    int desc_tag;

    desc_tag = *pBuffer;
    switch(desc_tag)
    {
        case 2:                 // video stream desc tag
            return(PrintVideoDesc(pBuffer));
            break;

        case 3:                 // audio stream desc tag
            break;

        case 9:                 // CA desc tag
            return(PrintCADesc(pBuffer));
            break;

        default:                // unhandled Desc tag
            {   int len = *++pBuffer;
                trace_new(TL_INFO, "Unhandled Descriptor Tag\n");
                return(len+2);  // to skip the rest
                break;
            }
    }
    return 0;
}

int PrintProgramDesc(u_int8 *pBuffer, u_int32 PATIndex)
{
    u_int8 stream_type;
    u_int16 elementary_PID, ES_info_length;
    int Len, ret = 0;

    stream_type = *pBuffer++;
    elementary_PID = (*pBuffer++ & 0x1F) << 8; elementary_PID |= *pBuffer++;
    ES_info_length = (*pBuffer++ & 0x0F) << 8; ES_info_length |= *pBuffer++;
    trace_new(TL_INFO, "stream_type    = %d\n", stream_type);
    trace_new(TL_INFO, "elementary_PID = %d\n", elementary_PID);
    switch(stream_type){
        case 0:
            break;
        case 1:
        case 2:
        case 0x80:
            if (PATEntryTable[PATIndex].NumVideoPIDs < MAX_PID_ENTRIES)
                PATEntryTable[PATIndex].VideoPIDs[PATEntryTable[PATIndex].NumVideoPIDs++] = elementary_PID;
            break;
        case 3:
        case 4:
        case 0x81:
            if (PATEntryTable[PATIndex].NumAudioPIDs < MAX_PID_ENTRIES)
                PATEntryTable[PATIndex].AudioPIDs[PATEntryTable[PATIndex].NumAudioPIDs++] = elementary_PID;
            break;
        case 6:
            if (PATEntryTable[PATIndex].NumDataPIDs < MAX_PID_ENTRIES)
                PATEntryTable[PATIndex].DataPIDs[PATEntryTable[PATIndex].NumDataPIDs++] = elementary_PID;
            break;
        default:
            break;
    }

    trace_new(TL_INFO, "ES_info_length = %d\n", ES_info_length);
    ret += (5+ES_info_length);
    Len = PrintDesc(pBuffer);

    return ret;
}

/****************************************************************************
 * Modifications:
 * $Log: 
 *  18   mpeg      1.17        3/31/04 2:44:09 PM     Yong Lu         CR(s) 
 *        8716 8717 : disable PROCESS_NEXT_PAT_VERSION
 *  17   mpeg      1.16        3/10/04 2:51:20 PM     Larry Wang      CR(s) 
 *        8551 : Add code to acquire Sony Passage SPI tables.
 *  16   mpeg      1.15        1/27/04 1:11:43 PM     Song Qiao       CR(s) 
 *        8203 : 
 *        
 *        Re-enable PROCESS_NEXT_PAT_VERSION.
 *        
 *        
 *  15   mpeg      1.14        1/13/04 12:22:54 PM    Yong Lu         CR(s) 
 *        8078 : 
 *        current ipstb only process one PAT, will be changed late
 *  14   mpeg      1.13        12/9/03 10:13:32 AM    Larry Wang      CR(s) 
 *        8115 8118 : Mask off bit 28 of PSI buffer pointer.
 *        
 *  13   mpeg      1.12        12/3/03 2:27:25 PM     Larry Wang      CR(s): 
 *        8086 8087 Allocate PSI buffers by mem_nc_malloc().
 *  12   mpeg      1.11        11/3/03 2:07:53 PM     Billy Jackman   CR(s): 
 *        7787 7788 Fix spelling of Id keyword from ID because of case 
 *        sensitivity.
 *  11   mpeg      1.10        11/3/03 1:23:45 PM     Billy Jackman   CR(s): 
 *        7787 7788 Add TRACE_FG_NORMAL specification after trace text to 
 *        return trace output to normal foreground color.
 *  10   mpeg      1.9         9/16/03 5:42:38 PM     Angela Swartz   SCR(s) 
 *        7477 :
 *        #include stbcfg.h to pickup some macro defines
 *        
 *  9    mpeg      1.8         8/15/03 7:51:18 PM     Larry Wang      SCR(s) 
 *        7240 7284 5602 :
 *        (1) In InitializeForNewPAT(), free filter first; (2) Call 
 *        InitializeForNewPAT() right before setup PMT channel to ensure no 
 *        duplicate channel will be openned during "quick" PAT version update.
 *        
 *  8    mpeg      1.7         7/29/03 3:28:22 PM     Billy Jackman   SCR(s) 
 *        7056 7058 :
 *        Modified PMT code to only allocate one filter per PMT PID, instead of
 *         one
 *        filter per program.  This fixes the problem of running out of filters
 *         on
 *        transponders with high number of programs.
 *        
 *  7    mpeg      1.6         6/3/03 10:24:06 AM     Larry Wang      SCR(s) 
 *        6667 :
 *        Save audio stream types from PMT.
 *        
 *  6    mpeg      1.5         5/9/03 2:30:14 PM      Miles Bintz     SCR(s) 
 *        6170 6182 :
 *        added define to prevent new pat versions from being process (defaults
 *         to old way).  Changed function InitializeForNewPAT to shut down old 
 *        PMT and SDT channels
 *        
 *  5    mpeg      1.4         4/30/03 7:14:48 PM     Miles Bintz     SCR(s) 
 *        6100 :
 *        added new stream type (0x80) for video
 *        
 *  4    mpeg      1.3         4/11/03 6:00:20 PM     Dave Wilson     SCR(s) 
 *        5979 :
 *        Moved some definitions and externs to dvb_priv.h
 *        
 *  3    mpeg      1.2         4/7/03 10:53:32 AM     Angela Swartz   SCR(s) 
 *        5826 :
 *        moved PrintProgramDesc & other PrintXXDesc functions from watchtv to 
 *        DVB
 *        
 *  2    mpeg      1.1         4/1/03 5:23:40 PM      Miles Bintz     SCR(s) 
 *        5602 :
 *        dvb_lib wasn't closing the demux filters when switching to a new 
 *        transport stream (transponder).  Made modifications to close filters 
 *        and re-open filters when necessary.
 *        
 *        
 *  1    mpeg      1.0         3/18/03 4:58:00 PM     Angela Swartz   
 * $
 * 
 ****************************************************************************/

