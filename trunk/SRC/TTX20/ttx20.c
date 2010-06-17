/******************************************************************************/
/*                   CONEXANT PROPRIETARY AND CONFIDENTIAL                    */
/*                        SOFTWARE FILE/MODULE HEADER                         */
/*                 Copyright Conexant Systems Inc. 1998-2004                  */
/*                                 Austin, TX                                 */
/*                            All Rights Reserved                             */
/******************************************************************************/
/*
 * Filename:        ttx20.c
 *
 *
 * Description:     TTX main module version 2.0
 *
 *
 * Author:          Miles Bintz, Billy Jackman
 *
 ******************************************************************************/
/* $Id: ttx20.c,v 1.41, 2004-03-24 17:45:15Z, Billy Jackman$
 ******************************************************************************/

#include <stdarg.h>
#include <string.h>
#include "stbcfg.h"
#include "kal.h"

#ifdef DRIVER_INCL_GENDMXC
#include "gendemux.h"
#else
#include "demuxapi.h"
#endif
#include "retcodes.h"
#include "globals.h"
#include "osdlibc.h"
#include "retcodes.h"
#include "iic.h"
#include "ttx20.h"
#include "hamming.h"
#include "confmgr.h"
#if defined(DRIVER_INCL_TVENC)
#include "tvenc.h"
#endif


#if (TTX_DEBUG_STATIC_BUFFER == 1)
u_int8 gbyTesttopData[12][48];
u_int8 gbyTestbotData[12][48];
#endif

/*************************/
/* Function Declarations */
/*************************/
void                  ttx_task(void);
void                  ProcessPESHeader(int nb, int client_id);
void                  ttx_encoder_init();
cnxt_ttx_status       ttx_do_insert(unsigned char *buf, long long pts, int field, int line);
cnxt_ttx_status       ttx_do_extract(int client_id, unsigned char *buf);
genfilter_mode        ttx_DataCB(pgen_notify_t pNotifyData);
genfilter_mode        ttx_ErrorCB(pgen_notify_t pNotifyData);
void                  ttx_line_ISR(u_int32 dwLine, OSDFIELD oField);

/**********************/
/*      Externs       */
/**********************/
extern void  FCopy(unsigned char *, unsigned char *, int);
extern void  FFillBytes(unsigned char *, char, int);

#if defined(DRIVER_INCL_TVENC)
/* TV encoder driver handle which is declared in OSDInit */ 
extern CNXT_TVENC_HANDLE gTvencHandle;
#else
#if (INTERNAL_ENCODER==EXTERNAL)
extern u_int32 gdwEncoderType;
#endif
#endif

#if ( defined(OPENTV) ) && (INCLUDE_OTV_EXTENSION_TELETEXT == YES)
    extern u_int32 en2_vbi_chid;
#else /* OPENTV */
    #ifdef DRIVER_INCL_GENDMXC
        extern u_int32 gdwDemuxID;
    #endif
    #ifdef DRIVER_INCL_DEMUX
        extern u_int32     gDemuxInstance;
    #endif
#endif /* OPENTV && OTV_EXT */


/**********************/
/*      Globals       */
/**********************/
char                ttx_Wrapped;
int                 ttx_inline, ttx_outline;

ttx_PES_Parse_State ttx_pes_ps[TTX_MAX_CLIENTS];

#ifdef DRIVER_INCL_GENDMXC
int               ttx_dmxHnd;
#endif

extern u_int32 gDemuxInstance;

int               ttx_numClients;

unsigned char     ttx_pes_data[TTX_DMX_BUFFER_SIZE];
task_id_t         ttx_task_id;
task_id_t         check_task_id;
queue_id_t        ttx_queue_id;
sttx_ClientData   sttx_cd[TTX_MAX_CLIENTS];
long long         this_pts[TTX_MAX_CLIENTS];

u_int16  ttx_video_standard = 0;

#define TTX_SYNC_NONE      (0)
#define TTX_SYNC_PTS       (1)
#define TTX_SYNC_LINE      (2)
#define TTX_SYNC_OVERFLOW  (3)

u_int32 ttx_sync_mode = TTX_SYNC_NONE;

/* Offsets and bit definitions for PES header. */
#define PTS_FLAG_OFFSET    (3)
#define PTS_FLAG_BIT       (0x80)
#define OPT_HDR_LEN_OFFSET (4)
#define PTS_OFFSET         (5)

/* Offsets for teletext data segment. */
#define TTX_TYPE_OFFSET      (0)
#define TTX_LENGTH_OFFSET    (1)
#define TTX_POSITION_OFFSET  (2)
#define TTX_FRAMING_OFFSET   (3)


#if !TTX_DEBUG
const unsigned char   gbyBitSwap[256];
unsigned char     DRMbotbuf[ TTX_LINES_PER_FIELD * (TTX_DRM_LINE_SIZE) + TTX_CACHE_PAD];
unsigned char     DRMtopbuf[ TTX_LINES_PER_FIELD * (TTX_DRM_LINE_SIZE) + TTX_CACHE_PAD];
unsigned char     *pDRMtopbuf, *pDRMbotbuf;
sttxbuf           ttx_buf[ TTX_NUM_BUFFERED_LINES ];

#else

const unsigned char   gbyBitSwap[256];
int               pad0[1024];
unsigned char     DRMbotbuf[ TTX_LINES_PER_FIELD * (TTX_DRM_LINE_SIZE) + TTX_CACHE_PAD];
int               pad1[1024];
unsigned char     DRMtopbuf[ TTX_LINES_PER_FIELD * (TTX_DRM_LINE_SIZE) + TTX_CACHE_PAD];
int               pad2[1024];
unsigned char     *pDRMtopbuf, *pDRMbotbuf;
int               pad3[1024];
sttxbuf           ttx_buf[ TTX_NUM_BUFFERED_LINES ];
int               pad4[1024];

void check_task(void)  {
    int i,j;
    while (1) {
        for (i = 0; i < 1024; i++) {
            if (pad0[i] != 0xffffffff) trace_new(TRACE_OSD | TRACE_LEVEL_ALWAYS, "pad0: bad stuff at 0x%08x !\n", &pad0[i]);
            if (pad1[i] != 0xeeeeeeee) trace_new(TRACE_OSD | TRACE_LEVEL_ALWAYS, "pad1: bad stuff at 0x%08x !\n", &pad1[i]);
            if (pad2[i] != 0xdddddddd) trace_new(TRACE_OSD | TRACE_LEVEL_ALWAYS, "pad2: bad stuff at 0x%08x !\n", &pad2[i]);
            if (pad3[i] != 0xcccccccc) trace_new(TRACE_OSD | TRACE_LEVEL_ALWAYS, "pad3: bad stuff at 0x%08x !\n", &pad3[i]);
            if (pad4[i] != 0xbbbbbbbb) trace_new(TRACE_OSD | TRACE_LEVEL_ALWAYS, "pad4: bad stuff at 0x%08x !\n", &pad4[i]);
        }
        task_time_sleep(20);
        if (j++ % 1000 == 0) {
#if TTX_DEBUG
            trace("ttx check task is alive!\n");
#endif
        }
    }   
}
#endif

cnxt_ttx_status cnxt_ttx_Init()  {
   int i;
#ifdef DRIVER_INCL_CONFMGR
   sabine_config_data *pConfigData;
#endif /* DRIVER_INCL_CONFMGR */

   /* Make sure OSDInit has been called first. */
   OSDInit();
   
#ifdef DRIVER_INCL_CONFMGR
   /* Get our non-volatile config data */
   pConfigData = config_lock_data();
   if (pConfigData)
   {
      ttx_video_standard = pConfigData->video_standard;
      config_unlock_data(pConfigData);
   }
   if ( ttx_video_standard != PAL )
      return CNXT_TTX_FAILURE;
#else
   ttx_video_standard = PAL;
#endif /* DRIVER_INCL_CONFMGR */

#if TTX_DEBUG
   trace_set_level(TRACE_LEVEL_3 | TRACE_DPS | TRACE_OSD | TRACE_DMD | TRACE_KAL | TRACE_CTL);
   FFillBytes((unsigned char*)pad0, (char)0xFF, 0x1000);
   FFillBytes((unsigned char*)pad1, (char)0xEE, 0x1000);
   FFillBytes((unsigned char*)pad2, (char)0xDD, 0x1000);
   FFillBytes((unsigned char*)pad3, (char)0xCC, 0x1000);
   FFillBytes((unsigned char*)pad4, (char)0xBB, 0x1000);
   check_task_id = task_create((PFNTASK)check_task, 0, NULL, CTBT_TASK_STACK_SIZE, CTBT_TASK_PRIORITY, CTBT_TASK_NAME);
#endif
   
#if ( defined(OPENTV) ) && (INCLUDE_OTV_EXTENSION_TELETEXT == YES)
       /* Do nothing...  */
#else /* OPENTV && OTV_EXT */
    #ifdef DRIVER_INCL_GENDMXC
       ttx_dmxHnd = gen_demux_get_callback_handle();
       if (ttx_dmxHnd == -1)  {
          return(CNXT_TTX_DMX_ERROR);
       }
    #endif
#endif /* OPENTV && OTV_EXT */
   ttx_queue_id = qu_create(10, "TTXQ");
   if (ttx_queue_id == 0)  {
       trace_new(TRACE_OSD | TRACE_LEVEL_ALWAYS, "ttx20.c  cnxt_ttx_Init:  qu_create failed.\n");
       return(CNXT_TTX_FAILURE);          
   }

   ttx_task_id = task_create((PFNTASK)ttx_task, 0, NULL, TTEX_TASK_STACK_SIZE, TTEX_TASK_PRIORITY, TTEX_TASK_NAME);
   if (ttx_task_id == 0)  {
       trace_new(TRACE_OSD | TRACE_LEVEL_ALWAYS, "ttx20.c  cnxt_ttx_Init:  task_create failed.\n");
       return(CNXT_TTX_FAILURE);
   }
   
   ttx_numClients = 0;
   for (i = 0; i < TTX_MAX_CLIENTS; i++)  {
       sttx_cd[i].state = CNXT_TTX_UNALLOCATED;
       sttx_cd[i].pid_link = -1;
       sttx_cd[i].pid = -1;
       this_pts[i] = -1;
   }


   return(CNXT_TTX_SUCCESS);
}

/*
The alternate form of the function is:
cnxt_ttx_status cnxt_ttx_Create_Instance(cnxt_ttx_Type type, int *hTTX, void *pBuf, int buf_size, cnxt_ttx_pfncallback pCallback);
*/
cnxt_ttx_status cnxt_ttx_Create_Instance(cnxt_ttx_Type typ, int *hTTX, ...)  {
   va_list ap;
   int i;
#if ( defined(OPENTV) ) && (INCLUDE_OTV_EXTENSION_TELETEXT == YES)
       /* do nothing */
#else /* OPENTV && OTV_EXT */
   #ifdef DRIVER_INCL_DEMUX
      DMX_STATUS ret = DMX_ERROR;
   #endif
   #ifndef DRIVER_INCL_GENDMXC
      size_t NumChannels = 0;
   #endif
#endif /* OPENTV && OTV_EXT */
   
   /* Are there any available clients? */
   if (ttx_numClients == TTX_MAX_CLIENTS)  {
      return(CNXT_TTX_FAILURE);
   }   

   /* if they are asking for an insertion client, check if one is already allocated... */
   if (typ == CNXT_TTX_INSERT)  {
       for (i = 0; i < TTX_MAX_CLIENTS; i++) {
           if ((sttx_cd[i].typ == CNXT_TTX_INSERT) && (sttx_cd[i].state > CNXT_TTX_UNALLOCATED))  {
               return(CNXT_TTX_FAILURE);
           }
       }
       /* Insertion is only allowed for PAL video... */
       if ( ttx_video_standard != PAL )
           return CNXT_TTX_FAILURE;
       
   }
#if ( defined(OPENTV) ) && (INCLUDE_OTV_EXTENSION_TELETEXT == YES)
       else
       {
               /* Only allow reinsertion if OTV EN2 */
               return CNXT_TTX_FAILURE;
       }
#endif

   /* there is an available channel, which one? */
   for (i = 0; i < TTX_MAX_CLIENTS; i++) {
       if (sttx_cd[i].state == CNXT_TTX_UNALLOCATED) break;
   }
   /* Client ID is index */
   *hTTX = i;
   
#if ( defined(OPENTV) ) && (INCLUDE_OTV_EXTENSION_TELETEXT == YES)
    #ifdef DRIVER_INCL_GENDMXC
       if (gen_dmx_register_section_channel_notifications(
                   gdwDemuxID, en2_vbi_chid,
                   (gen_callback_fct_t)ttx_ErrorCB,
                   (gen_callback_fct_t)ttx_DataCB, 
                   0, 0) != 0)
       {
          trace_new(TRACE_OSD | TRACE_LEVEL_ALWAYS, "ttx20.c  Create_Instance: error setting callbacks for OTV VBI reinsertion.\n");
          return(CNXT_TTX_DMX_ERROR);
       }
    #else  /* DRIVER_INCL_GENDMXC */
       if (cnxt_dmx_channel_control(gDemuxInstance, en2_vbi_chid, GEN_DEMUX_RESET) 
               != DMX_STATUS_OK) 
       {
          trace_new(TRACE_OSD | TRACE_LEVEL_ALWAYS, "ttx20.c  Create_Instance: error resetting demux channel.\n");
          return CNXT_TTX_DMX_ERROR;
       }
           
       if (cnxt_dmx_set_section_channel_attributes(
                   gDemuxInstance, en2_vbi_chid,
                   (gen_callback_fct_t)ttx_ErrorCB, 
                   (gen_callback_fct_t)ttx_DataCB,
                   0, 4) != DMX_STATUS_OK)
       {
          trace_new(TRACE_OSD | TRACE_LEVEL_ALWAYS, "ttx20.c  Create_Instance: error setting callbacks for OTV VBI reinsertion.\n");
          return CNXT_TTX_DMX_ERROR;
       }
    #endif   /* DRIVER_INCL_GENDMXC */
    sttx_cd[i].dmxchid = en2_vbi_chid;

#else /* OPENTV && OTV_EXT */
    #ifdef DRIVER_INCL_GENDMXC
       /* Check how many channels are available and allocate one if available */
       if (gen_dmx_get_number_of_channels_available(TTX_SCRAMBLING, 0, 0) < 1)  {
          trace_new(TRACE_OSD | TRACE_LEVEL_ALWAYS, "ttx20.c  Create_Instance:  get_#_channels_available failed.\n");
          return(CNXT_TTX_DMX_ERROR);
       };
    #else
        ret = cnxt_dmx_channels_available(gDemuxInstance, DMX_CH_CAP_DESCRAMBLING, (u_int32*)&NumChannels, (u_int32)0);
        if (ret != DMX_STATUS_OK || NumChannels < 1) {
            trace_new(DPS_ERROR_MSG,"ttx20.c: Failed cnxt_dmx_channels_available!!\n");
            return(CNXT_TTX_DMX_ERROR);
        }
    #endif
       
       /* allocate_channel should work so lets see if we can get some memory first... */
       sttx_cd[i].dmxbuffer = (void*)( (u_int32)mem_nc_malloc(TTX_DMX_BUFFER_SIZE) & ~NCR_BASE );
       if (sttx_cd[i].dmxbuffer == 0)  {
           trace_new(TRACE_OSD | TRACE_LEVEL_ALWAYS, "ttx20.c  Create_Instance:  mem_nc_malloc failed.\n");
           return(CNXT_TTX_FAILURE);
       }
       
       /* Now... lets see if we can get the channel */
    #ifdef DRIVER_INCL_GENDMXC
       sttx_cd[i].dmxchid = gen_dmx_allocate_channel(ttx_dmxHnd, PES_CHANNEL_TYPE, 0, TRUE);
       if (sttx_cd[i].dmxchid == GENDMX_BAD_CHANNEL)  {
          trace_new(TRACE_OSD | TRACE_LEVEL_ALWAYS, "ttx20.c  Create_Instance:  dmx_allocate_channel failed.\n");
          mem_nc_free(sttx_cd[i].dmxbuffer);
          return(CNXT_TTX_DMX_ERROR);
       }
    #else
       if (cnxt_dmx_channel_open(gDemuxInstance, DMX_CH_CAP_DESCRAMBLING, 
                     PES_CHANNEL_TYPE, (u_int32*)&sttx_cd[i].dmxchid) != DMX_STATUS_OK) {
          trace_new(TRACE_OSD | TRACE_LEVEL_ALWAYS, "ttx20.c  Create_Instance: cnxt_dmx_channel_open failed.\n");
          mem_nc_free(sttx_cd[i].dmxbuffer);
          return(CNXT_TTX_DMX_ERROR);
       }
    
    #endif
       
       /* Give the channel its buffer... */
    #ifdef DRIVER_INCL_GENDMXC
       if (gen_dmx_set_channel_buffer(ttx_dmxHnd, sttx_cd[i].dmxchid, sttx_cd[i].dmxbuffer, TTX_DMX_BUFFER_SIZE) != 0) {
          /* Something went wrong... */
          trace_new(TRACE_OSD | TRACE_LEVEL_ALWAYS, "ttx20.c  Create_Instance:  set_channel_buffer failed.\n");
          gen_dmx_free_channel(ttx_dmxHnd, sttx_cd[i].dmxchid);
          mem_nc_free(sttx_cd[i].dmxbuffer);
          return(CNXT_TTX_DMX_ERROR);
       }
       
    #else
       if (cnxt_dmx_set_channel_buffer(gDemuxInstance,  sttx_cd[i].dmxchid, sttx_cd[i].dmxbuffer, TTX_DMX_BUFFER_SIZE) != DMX_STATUS_OK) {
          /* Something went wrong... */
          trace_new(TRACE_OSD | TRACE_LEVEL_ALWAYS, "ttx20.c  Create_Instance:  set_channel_buffer failed.\n");
          cnxt_dmx_channel_close(gDemuxInstance, sttx_cd[i].dmxchid);
          mem_nc_free(sttx_cd[i].dmxbuffer);
          return(CNXT_TTX_DMX_ERROR);
       }
    #endif
       
       /* Register the callbacks */
    #ifdef DRIVER_INCL_GENDMXC
       if (gen_dmx_register_section_channel_notifications(ttx_dmxHnd, sttx_cd[i].dmxchid,
             (gen_callback_fct_t)ttx_ErrorCB,
             (gen_callback_fct_t)ttx_DataCB, 
              0, 0) != 0)  {
          trace_new(TRACE_OSD | TRACE_LEVEL_ALWAYS, "ttx20.c  Create_Instance:  register_section_channel_notification failed.\n");
          gen_dmx_free_channel(ttx_dmxHnd, sttx_cd[i].dmxchid);
          mem_nc_free(sttx_cd[i].dmxbuffer);
          return(CNXT_TTX_DMX_ERROR);
       }
    #else
       if (cnxt_dmx_set_section_channel_attributes((u_int32)gDemuxInstance, sttx_cd[i].dmxchid,
            (gen_callback_fct_t)ttx_ErrorCB, (gen_callback_fct_t)ttx_DataCB, 0, 4) != DMX_STATUS_OK) {
          trace_new(TRACE_OSD | TRACE_LEVEL_ALWAYS, "ttx20.c  Create_Instance: cnxt_dmx_set_section_channel_attributes failed.\n");
          cnxt_dmx_channel_close(gDemuxInstance, sttx_cd[i].dmxchid);
          mem_nc_free(sttx_cd[i].dmxbuffer);
          return(CNXT_TTX_DMX_ERROR);
       }
    #endif

#endif /* OPENTV && OTV_EXT */

   if (typ == CNXT_TTX_EXTRACT)  {
       /* Demux is all set.  If typ is extract, lets get the extra parameters... */
       va_start(ap, hTTX);
       sttx_cd[i].ttxbuf     = va_arg(ap, void*);
       sttx_cd[i].ttxbufsize = va_arg(ap, int);
       sttx_cd[i].fncb       = (cnxt_ttx_pfncallback)va_arg(ap, void*);
       va_end(ap);
   } else {
       /* This task will wait for a DRM interrupt indicating that we've gone
          past VBI line 23 or 336 (the teletext lines) so we can safely write
          to those buffers.  The catch is that we need to fill the buffer 
          between (lines 24 and lines 321) or (line 337 and line 7) */
       ttx_Wrapped = 0;
       ttx_inline = 0;
       ttx_outline = 0;

       pDRMtopbuf = (unsigned char*)(((unsigned int)(DRMtopbuf + TTX_CACHE_PAD) & ~TTX_CACHE_MASK) | NCR_BASE);
       pDRMbotbuf = (unsigned char*)(((unsigned int)(DRMbotbuf + TTX_CACHE_PAD) & ~TTX_CACHE_MASK) | NCR_BASE);
       memset(pDRMtopbuf, 0, TTX_LINES_PER_FIELD * (TTX_DRM_LINE_SIZE)); 
       memset(pDRMbotbuf, 0, TTX_LINES_PER_FIELD * (TTX_DRM_LINE_SIZE)); 
       ttx_encoder_init();
       osdRegisterLineISR(ttx_line_ISR, 30, BOTH);
       
   }
   
   sttx_cd[i].typ   = typ;
   sttx_cd[i].state = CNXT_TTX_CLOSED;
   ttx_numClients++;
   return(CNXT_TTX_SUCCESS);
}

cnxt_ttx_status cnxt_ttx_Release_Instance(int hTTX)  {

   /* First, check for a valid handle. */
   if ( (hTTX < 0) || (hTTX >= TTX_MAX_CLIENTS) )
       return(CNXT_TTX_FAILURE);
   
   if (sttx_cd[hTTX].state == CNXT_TTX_CLOSED)  {
       if (sttx_cd[hTTX].typ == CNXT_TTX_INSERT)  {
          osdUnRegisterLineISR(ttx_line_ISR, 30);
       }
#if ( defined(OPENTV) ) && (INCLUDE_OTV_EXTENSION_TELETEXT == YES)
     #ifdef DRIVER_INCL_GENDMXC
        if (gen_dmx_register_section_channel_notifications(
                    gdwDemuxID, en2_vbi_chid,
                    (gen_callback_fct_t)0,
                    (gen_callback_fct_t)0, 
                    0, 0) != 0)
        {
           trace_new(TRACE_OSD | TRACE_LEVEL_ALWAYS, "ttx20.c  Create_Instance: error setting callbacks for OTV VBI reinsertion.\n");
           return(CNXT_TTX_DMX_ERROR);
        }
     #else
        if (cnxt_dmx_set_section_channel_attributes(
                    gDemuxInstance, en2_vbi_chid,
                    (gen_callback_fct_t)0, 
                    (gen_callback_fct_t)0,
                    0, 4) != DMX_STATUS_OK)
        {
           trace_new(TRACE_OSD | TRACE_LEVEL_ALWAYS, "ttx20.c  Create_Instance: error setting callbacks for OTV VBI reinsertion.\n");
           return CNXT_TTX_DMX_ERROR;
        }
     #endif
#else /* OPENTV && OTV_EXT */
        
    #ifdef DRIVER_INCL_GENDMXC       
           if (gen_dmx_control_channel(ttx_dmxHnd, sttx_cd[hTTX].dmxchid, GEN_DEMUX_DISABLE) != 0)  {
              trace_new(TRACE_OSD | TRACE_LEVEL_ALWAYS, "ttx20.c  Release_Instance:  dmx_control_channel failed.\n");
              return(CNXT_TTX_DMX_ERROR);
           }
    #else
           if (cnxt_dmx_channel_control(gDemuxInstance, sttx_cd[hTTX].dmxchid, GEN_DEMUX_DISABLE) != DMX_STATUS_OK) {
             trace_new(TRACE_OSD | TRACE_LEVEL_ALWAYS, "ttx20.c  Release_Instance: cnxt_dmx_channel_control failed.\n");
    
             return(CNXT_TTX_DMX_ERROR);
       }
    #endif
    
    #ifdef DRIVER_INCL_GENDMXC       
           if (gen_dmx_free_channel(ttx_dmxHnd, sttx_cd[hTTX].dmxchid) != 0)  {
               trace_new(TRACE_OSD | TRACE_LEVEL_ALWAYS, "ttx20.c  Release_Instance:  dmx_free_channel failed.\n");
               return(CNXT_TTX_DMX_ERROR);
           }
    #else
           if (cnxt_dmx_channel_close(gDemuxInstance, sttx_cd[hTTX].dmxchid) != DMX_STATUS_OK)  {
               trace_new(TRACE_OSD | TRACE_LEVEL_ALWAYS, "ttx20.c  Release_Instance:  cnxt_dmx_channel_close failed.\n");
               return(CNXT_TTX_DMX_ERROR);
           }
    #endif
           mem_nc_free(sttx_cd[hTTX].dmxbuffer);
    
#endif /* OPENTV && OTV_EXT */

       sttx_cd[hTTX].state = CNXT_TTX_UNALLOCATED;
       ttx_numClients--;
       if (ttx_numClients < 0)  {
           fatal_exit(0x54545844); /* 'TTXD' */
           return(CNXT_TTX_FAILURE);
       }
   } else {
       return(CNXT_TTX_FAILURE);
   }
   return(CNXT_TTX_SUCCESS);
}

cnxt_ttx_status cnxt_ttx_Open(int hTTX, int pid)  {
#if ( defined(OPENTV) ) && (INCLUDE_OTV_EXTENSION_TELETEXT == YES)
   sttx_cd[hTTX].state = CNXT_TTX_OPEN;
   return(CNXT_TTX_SUCCESS);
#else /* OPENTV && OTV_EXT */
   int i;
   int last_link = 0, next_link = 0;

   /* First, check for a valid handle. */
   if ( (hTTX < 0) || (hTTX >= TTX_MAX_CLIENTS) )
       return(CNXT_TTX_FAILURE);
   
   /* Check if this handle is already open (has a pid assigned) */
   if (sttx_cd[hTTX].state == CNXT_TTX_UNALLOCATED)  {
       return(CNXT_TTX_FAILURE);
   }
   
   if (sttx_cd[hTTX].state == CNXT_TTX_CLOSED)  {
       /* Check to see if this PID is already opened in another instance */
       for (i = 0; i < TTX_MAX_CLIENTS; i++) {
           if ((pid == sttx_cd[i].pid) && (sttx_cd[i].state > CNXT_TTX_CLOSED))  {
               next_link = i;
               /* If this PID is used by multiple instances, follow the
                  list until we find the end... */
               while (next_link != -1) {
                   last_link = next_link;
                   next_link = sttx_cd[next_link].pid_link;
               }
               /* the new end of the list is this (hTTX) index */
               sttx_cd[last_link].pid_link = hTTX;
               sttx_cd[hTTX].pid = pid;
               sttx_cd[hTTX].state = CNXT_TTX_OPEN;
               return(CNXT_TTX_SUCCESS);
           }
       }

       /* If we got to the end of the list, this is a unique PID */
       if (i == TTX_MAX_CLIENTS)  {
        #ifdef DRIVER_INCL_GENDMXC       
           if (gen_dmx_set_channel_pid(ttx_dmxHnd, sttx_cd[hTTX].dmxchid, pid) != 0)  {
               trace_new(TRACE_OSD | TRACE_LEVEL_ALWAYS, "ttx20.c  ttx_Open:  set_channel_pid failed.\n");
               return(CNXT_TTX_DMX_ERROR);
           }
        #else
           if (cnxt_dmx_channel_set_pid(gDemuxInstance, sttx_cd[hTTX].dmxchid, pid) != 0)  {
               trace_new(TRACE_OSD | TRACE_LEVEL_ALWAYS, "ttx20.c  ttx_Open:  cnxt_dmx_channel_set_pid failed.\n");
               return(CNXT_TTX_DMX_ERROR);
           }
        #endif
           sttx_cd[hTTX].pid = pid;
           sttx_cd[hTTX].state = CNXT_TTX_OPEN;
           return(CNXT_TTX_SUCCESS);
       }
   }
   return(CNXT_TTX_SUCCESS);
#endif /* OPENTV && OTV_EXT */
}

cnxt_ttx_status cnxt_ttx_Close(int hTTX)  {
#if ( defined(OPENTV) ) && (INCLUDE_OTV_EXTENSION_TELETEXT == YES)
   sttx_cd[hTTX].state = CNXT_TTX_CLOSED;
   return(CNXT_TTX_SUCCESS);
#else /* OPENTV && OTV_EXT */
   int i;
   
   /* First, check for a valid handle. */
   if ( (hTTX < 0) || (hTTX >= TTX_MAX_CLIENTS) )
      return(CNXT_TTX_FAILURE);
   
   if (sttx_cd[hTTX].state == CNXT_TTX_PLAYING)  {
      cnxt_ttx_Stop(hTTX); 
   }
   
   if (sttx_cd[hTTX].state == CNXT_TTX_OPEN)  {
       /* Is this PID we are about to close in use by someone else? */
       /* Check if someone is linked to us... */
       for (i = 0; i < TTX_MAX_CLIENTS; i++) {
          if (sttx_cd[i].pid_link == hTTX)  {
              break;
          }
       }
       /* Then check to see if we're linked to anyone else... */
       if (sttx_cd[hTTX].pid_link != -1)  {
           /* If we're in the middle, we must connect the previos to the next... */
           if (i != TTX_MAX_CLIENTS)  {
               sttx_cd[i].pid_link = sttx_cd[hTTX].pid_link;
           } else {
               /* Else we are the first.  Nothing needs to be done */
           }
           sttx_cd[hTTX].state = CNXT_TTX_CLOSED;
           return(CNXT_TTX_SUCCESS);
       } else {
           /* We're not linked to anyone, but if i != MAX_CLIENTS, 
              then someone is linked to us.  Terminate this link */
           if (i != TTX_MAX_CLIENTS)  {
               sttx_cd[i].pid_link = -1;
               sttx_cd[hTTX].state = CNXT_TTX_CLOSED;
               return(CNXT_TTX_SUCCESS);
           }
       }

       /* if we get here, then we're solo and this PID must be disabled */
    #ifdef DRIVER_INCL_GENDMXC       
       if (gen_dmx_control_channel(ttx_dmxHnd, sttx_cd[hTTX].dmxchid, GEN_DEMUX_DISABLE) != 0)  {
           trace_new(TRACE_OSD | TRACE_LEVEL_ALWAYS, "ttx20.c  ttx_Close:  dmx_register_callbacks failed.\n");
           return(CNXT_TTX_DMX_ERROR);
       }
    #else
       if (cnxt_dmx_channel_control(gDemuxInstance, sttx_cd[hTTX].dmxchid, GEN_DEMUX_DISABLE) != DMX_STATUS_OK) {
         trace_new(TRACE_OSD | TRACE_LEVEL_ALWAYS, "ttx20.c  Release_Instance: cnxt_dmx_channel_control failed.\n");
         return(CNXT_TTX_DMX_ERROR);
       }
    #endif
       sttx_cd[hTTX].pid   = -1;
       sttx_cd[hTTX].state = CNXT_TTX_CLOSED;
       return(CNXT_TTX_SUCCESS);
   } else {
       return(CNXT_TTX_FAILURE);
   }
#endif /* OPENTV && OTV_EXT */
}

cnxt_ttx_status cnxt_ttx_Start(int hTTX)  {
   int cs;
   #if defined(DRIVER_INCL_TVENC)
   CNXT_TVENC_STATUS eRetcode;
   #endif

   /* First, check for a valid handle. */
   if ( (hTTX < 0) || (hTTX >= TTX_MAX_CLIENTS) )
       return(CNXT_TTX_FAILURE);
   
   if (sttx_cd[hTTX].state == CNXT_TTX_OPEN)  {
       this_pts[hTTX] = -1;
       /* when starting (or restarting) VBI insertion, reset PES parse state */
       ttx_pes_ps[hTTX] = tpsTTXStreamID; 
       ttx_sync_mode = TTX_SYNC_NONE;
       ttx_inline = ttx_outline = 0;

#if ( defined(OPENTV) ) && (INCLUDE_OTV_EXTENSION_TELETEXT == YES)
   /* Do nothing */
#else /* OPENTV && OTV_EXT */
       
    #ifdef DRIVER_INCL_GENDMXC       
       if (gen_dmx_control_channel(ttx_dmxHnd, sttx_cd[hTTX].dmxchid, GEN_DEMUX_ENABLE) != 0)  {
           return(CNXT_TTX_DMX_ERROR);
       }
    #else
       if (cnxt_dmx_channel_control(gDemuxInstance, sttx_cd[hTTX].dmxchid, GEN_DEMUX_ENABLE) != DMX_STATUS_OK) {
         trace_new(TRACE_OSD | TRACE_LEVEL_ALWAYS, "ttx20.c  Release_Instance: cnxt_dmx_channel_control failed.\n");
         return(CNXT_TTX_DMX_ERROR);
       }
    #endif

#endif /* OPENTV && OTV_EXT */

       if (sttx_cd[hTTX].typ == CNXT_TTX_INSERT)  {
           cs = critical_section_begin();
           CNXT_SET_VAL(glpDrmControl, DRM_CONTROL_TELETEXT_ENABLE_MASK, 1);
           /* Set the Transport Blocks to 1 so we get data ASAP */
#if ( defined(OPENTV) ) && (INCLUDE_OTV_EXTENSION_TELETEXT == YES)
           /* How will this effect OTV? */
           *((LPREG)DPS_HOST_CTL_REG_EX(gDemuxInstance)) &= ~DPS_TRANSPORT_BLOCKS_MASK;
           *((LPREG)DPS_HOST_CTL_REG_EX(gDemuxInstance)) |= (1 << DPS_TRANSPORT_BLOCKS_SHIFT);
#else /* OPENTV && OTV_EXT */
           *((LPREG)DPS_HOST_CTL_REG_EX(gDemuxInstance)) &= ~DPS_TRANSPORT_BLOCKS_MASK;
           *((LPREG)DPS_HOST_CTL_REG_EX(gDemuxInstance)) |= (1 << DPS_TRANSPORT_BLOCKS_SHIFT);
#endif /* OPENTV && OTV_EXT */
           
           critical_section_end(cs);

           #if defined(DRIVER_INCL_TVENC)
           /* enable TV encoder for teletext using the new tvenc driver */
           eRetcode = cnxt_tvenc_ttx_enable(gTvencHandle);
           if( eRetcode != CNXT_TVENC_OK)
           {
              return(CNXT_TTX_FAILURE);
           }
           #else
           #if (INTERNAL_ENCODER==EXTERNAL)
           /* Enable TTX in the encoder. */
           if (gdwEncoderType == BT865) {
               #if TTX_REQ_IS_CLOCK
               iicWriteIndexedReg(I2C_ADDR_BT865, 0xB2, 0x30, I2C_BUS_BT865); /* TXE = 1, TXRM = 1    */
                                                                    /* TXE 0x10 TXRM 0x20   */
                                                                    /* TXE 0x10 TXRM 0x20   */
               #else
               iicWriteIndexedReg(I2C_ADDR_BT865, 0xB2, 0x10, I2C_BUS_BT865); /* TXE = 1, TXRM = 0    */
                                                                    /* TXE 0x10 TXRM 0x20   */
                                                                    /* TXE 0x10 TXRM 0x20   */
               #endif
           } else {
               #if TTX_REQ_IS_CLOCK
               iicWriteIndexedReg(I2C_ADDR_BT861, 0x59, 0x03, I2C_BUS_BT861);  /* TXE = 1, TXRM = 1   */
                                                                     /* TXE 0x01 TXRM 0x02  */
               #else
               iicWriteIndexedReg(I2C_ADDR_BT861, 0x59, 0x01, I2C_BUS_BT861);  /* TXE = 1, TXRM = 0   */
                                                                     /* TXE 0x01 TXRM 0x02  */
               #endif
           }  /* (gdwEncoderType == BT865) */
           #else /* INTERNAL_ENCODER=BT861_TYPE */
           
           CNXT_SET_VAL(ENC_TTX_LINE_CTL_REG, ENC_TTXLINE_CTL_TXE_MASK, 1);
           #endif
           #endif /*  defined(DRIVER_INCL_TVENC) */
       }
       sttx_cd[hTTX].state = CNXT_TTX_PLAYING;
       return(CNXT_TTX_SUCCESS);
   } else {
       return(CNXT_TTX_FAILURE);
   }
}

cnxt_ttx_status cnxt_ttx_Stop(int hTTX)  {
   int cs;
   #if defined(DRIVER_INCL_TVENC)
   CNXT_TVENC_STATUS eRetcode;
   #endif

   /* First, check for a valid handle. */
   if ( (hTTX < 0) || (hTTX >= TTX_MAX_CLIENTS) )
       return(CNXT_TTX_FAILURE);
   
   if (sttx_cd[hTTX].state == CNXT_TTX_PLAYING)  {
#if ( defined(OPENTV) ) && (INCLUDE_OTV_EXTENSION_TELETEXT == YES)
    /* Do nothing... */
#else /* OPENTV && OTV_EXT */

    #ifdef DRIVER_INCL_GENDMXC       
       if (gen_dmx_control_channel(ttx_dmxHnd, sttx_cd[hTTX].dmxchid, GEN_DEMUX_DISABLE) != 0)  {
           return(CNXT_TTX_DMX_ERROR);
       }
    #else
       if (cnxt_dmx_channel_control(gDemuxInstance, sttx_cd[hTTX].dmxchid, GEN_DEMUX_DISABLE) != DMX_STATUS_OK) {
         trace_new(TRACE_OSD | TRACE_LEVEL_ALWAYS, "ttx20.c  Release_Instance: cnxt_dmx_channel_control failed.\n");
         return(CNXT_TTX_DMX_ERROR);
   }
    #endif
#endif /* OPENTV && OTV_EXT */

       if (sttx_cd[hTTX].typ == CNXT_TTX_INSERT)  {
           cs = critical_section_begin();
           CNXT_SET_VAL(glpDrmControl, DRM_CONTROL_TELETEXT_ENABLE_MASK, 0);
           critical_section_end(cs);

           #if defined(DRIVER_INCL_TVENC)
           eRetcode = cnxt_tvenc_ttx_disable(gTvencHandle);
           if( eRetcode != CNXT_TVENC_OK)
           {
              return(CNXT_TTX_FAILURE);
           }
           #else
           #if (INTERNAL_ENCODER==EXTERNAL)
           /* Disable TTX in the encoder. */
           if (gdwEncoderType == BT865) {
               #if TTX_REQ_IS_CLOCK
               iicWriteIndexedReg(I2C_ADDR_BT865, 0xB2, 0x20, I2C_BUS_BT865); /* TXE = 1, TXRM = 1    */
                                                                    /* TXE 0x10 TXRM 0x20   */
                                                                    /* TXE 0x10 TXRM 0x20   */
               #else
               iicWriteIndexedReg(I2C_ADDR_BT865, 0xB2, 0x00, I2C_BUS_BT865); /* TXE = 1, TXRM = 0    */
                                                                    /* TXE 0x10 TXRM 0x20   */
                                                                    /* TXE 0x10 TXRM 0x20   */
               #endif
           } else {
               #if TTX_REQ_IS_CLOCK
               iicWriteIndexedReg(I2C_ADDR_BT861, 0x59, 0x02, I2C_BUS_BT861);  /* TXE = 1, TXRM = 1   */
                                                                     /* TXE 0x01 TXRM 0x02  */
               #else
               iicWriteIndexedReg(I2C_ADDR_BT861, 0x59, 0x00, I2C_BUS_BT861);  /* TXE = 1, TXRM = 0   */
                                                                     /* TXE 0x01 TXRM 0x02  */
               #endif
           }  /* (gdwEncoderType == BT865) */
           #else /* INTERNAL_ENCODER=BT861_TYPE */
           
           CNXT_SET_VAL(ENC_TTX_LINE_CTL_REG, ENC_TTXLINE_CTL_TXE_MASK, 0);
           #endif
           #endif  /*  defined(DRIVER_INCL_TVENC) */
       }
       sttx_cd[hTTX].state = CNXT_TTX_OPEN;
       return(CNXT_TTX_SUCCESS);
   } else {
       return(CNXT_TTX_FAILURE);
   }
}

/* cnxt_ttx_status cnxt_ttx_Stop(int hTTX)   defined in header file as cnxt_ttx_Close */
cnxt_ttx_status cnxt_ttx_Set_Focus(int hTTX, int mag, int page, int subcode)  {

   /* First, check for a valid handle. */
   if ( (hTTX < 0) || (hTTX >= TTX_MAX_CLIENTS) )
       return(CNXT_TTX_FAILURE);
   
   if (mag > -1 && mag < 8)  {
      sttx_cd[hTTX].mag = mag;
      sttx_cd[hTTX].page = page;
      sttx_cd[hTTX].subcode = subcode;
   } else {
       return(CNXT_TTX_PARAM_ERROR);
   }
   return(CNXT_TTX_SUCCESS);
}

void ttx_encoder_init()  {
#if defined(DRIVER_INCL_TVENC)
   u_int32 uFieldActiveLines;
#endif

   /* teletext needs to appear on VBI lines 7 to 22 and 320 to 335 */

   /* teletext only has meaning in PAL */
   if ( ttx_video_standard != PAL )
      return;

#if TTX_REQ_IS_CLOCK
   CNXT_SET_VAL(glpDrmControl, DRM_CONTROL_TELETEXT_REQ_ENABLE_MASK, 1);  
   /* ttx_req is a ( 0 = request signal | 1 = clock ) */
#else
   CNXT_SET_VAL(glpDrmControl, DRM_CONTROL_TELETEXT_REQ_ENABLE_MASK, 0);  
   /* ttx_req is a ( 0 = request signal | 1 = clock ) */
#endif
   CNXT_SET_VAL(glpDrmControl, DRM_CONTROL_TELETEXT_ENABLE_MASK, 1);  
   
   /* Set the buffer pointer.  */
   CNXT_SET_VAL(glpDrmTTField1Addr, DRM_ADDRESS_ADDRESS_MASK, (unsigned int)pDRMtopbuf);
   CNXT_SET_VAL(glpDrmTTField2Addr, DRM_ADDRESS_ADDRESS_MASK, (unsigned int)pDRMbotbuf);

   /* Enable all lines.  */
   CNXT_SET_VAL(glpDrmTTStride, DRM_TELETEXT_STRIDE_TT_DISABLE_LOW_MASK, 0);
   CNXT_SET_VAL(glpDrmTTStride, DRM_TELETEXT_STRIDE_TT_DISABLE_HIGH_MASK, 0);

   /* Set the TTX line stride.  */
   CNXT_SET_VAL(glpDrmTTStride, DRM_TELETEXT_STRIDE_LINE_STRIDE_MASK, TTX_DRM_LINE_SIZE);

#if defined(DRIVER_INCL_TVENC)
   uFieldActiveLines = CNXT_TVENC_VBI_LINE_7 |CNXT_TVENC_VBI_LINE_8 | CNXT_TVENC_VBI_LINE_9 \
       | CNXT_TVENC_VBI_LINE_10 | CNXT_TVENC_VBI_LINE_11 | CNXT_TVENC_VBI_LINE_12 \
       | CNXT_TVENC_VBI_LINE_13 | CNXT_TVENC_VBI_LINE_14 | CNXT_TVENC_VBI_LINE_15 \
       | CNXT_TVENC_VBI_LINE_16 | CNXT_TVENC_VBI_LINE_17 | CNXT_TVENC_VBI_LINE_18 \
       | CNXT_TVENC_VBI_LINE_19 | CNXT_TVENC_VBI_LINE_20 | CNXT_TVENC_VBI_LINE_21 \
       | CNXT_TVENC_VBI_LINE_22;

   cnxt_tvenc_ttx_set_lines(gTvencHandle, uFieldActiveLines, uFieldActiveLines);
#else
#if (INTERNAL_ENCODER==EXTERNAL)

#if PLL_PIN_GPIO_MUX0_REG_DEFAULT == NOT_DETERMINED
   /* Since the encoder is external, we need to enable the TTX functionality */
   /* of a couple of pins.                                                   */
   *((LPREG)PLL_PIN_GPIO_MUX0_REG) |= (PLL_PIN_GPIO_MUX0_TTXREQ |
                                       PLL_PIN_GPIO_MUX0_TTXDAT);
#endif /* PLL_PIN_GPIO_MUX0_REG_DEFAULT == NOT_DETERMINED  */

   if (gdwEncoderType == BT861)
   {
      CNXT_SET_VAL(glpDrmTTField1, DRM_FIELD1_TELETEXT_FIRST_LINE_MASK, TTX_FIRST_LINE+1);
      CNXT_SET_VAL(glpDrmTTField1, DRM_FIELD1_TELETEXT_LAST_LINE_MASK, TTX_FIRST_LINE + TTX_LINES_PER_FIELD);
      CNXT_SET_VAL(glpDrmTTField2, DRM_FIELD1_TELETEXT_FIRST_LINE_MASK, TTX_FIRST_LINE);
      CNXT_SET_VAL(glpDrmTTField2, DRM_FIELD1_TELETEXT_LAST_LINE_MASK, TTX_FIRST_LINE + TTX_LINES_PER_FIELD - 1);
/*      CNXT_SET_VAL(glpDrmTTField2, FirstLine, TTX_FIRST_LINE + 1);
      CNXT_SET_VAL(glpDrmTTField2, LastLine, TTX_FIRST_LINE + TTX_LINES_PER_FIELD);     */

#if !TTX_REQ_IS_CLOCK
      iicWriteIndexedReg(I2C_ADDR_BT861, 0x4D, 0x4C, I2C_BUS_BT861);   /* TXHS = 0x14F      */
      iicWriteIndexedReg(I2C_ADDR_BT861, 0x4E, 0x01, I2C_BUS_BT861);
      iicWriteIndexedReg(I2C_ADDR_BT861, 0x4F, 0x09, I2C_BUS_BT861);   /* TXHE = 0x009      */
      iicWriteIndexedReg(I2C_ADDR_BT861, 0x50, 0x00, I2C_BUS_BT861);
#endif
      /* End Field is (First Line + Lines Per Field - 1) because (FL to FL + LPF inclusive) is LPF + 1 */
      /* set up the 865 for lines 6 to 22 and 7 to 23 */
      iicWriteIndexedReg(I2C_ADDR_BT861, 0x51, TTX_FIRST_LINE, I2C_BUS_BT861);                               /* TXBF1 = 7         */
      iicWriteIndexedReg(I2C_ADDR_BT861, 0x52, 00, I2C_BUS_BT861);
      iicWriteIndexedReg(I2C_ADDR_BT861, 0x53, TTX_FIRST_LINE + TTX_LINES_PER_FIELD - 1, I2C_BUS_BT861);     /* TXEF1 = 22        */
      iicWriteIndexedReg(I2C_ADDR_BT861, 0x54, 00, I2C_BUS_BT861);
      iicWriteIndexedReg(I2C_ADDR_BT861, 0x55, TTX_FIRST_LINE, I2C_BUS_BT861);                               /* TXBF2 = 7         */
      iicWriteIndexedReg(I2C_ADDR_BT861, 0x56, 00, I2C_BUS_BT861);
      iicWriteIndexedReg(I2C_ADDR_BT861, 0x57, TTX_FIRST_LINE + TTX_LINES_PER_FIELD - 1, I2C_BUS_BT861);     /* TXEF2 = 22        */
      iicWriteIndexedReg(I2C_ADDR_BT861, 0x58, 00, I2C_BUS_BT861);
      iicWriteIndexedReg(I2C_ADDR_BT861, 0x5A, 0x00, I2C_BUS_BT861);   /* TTX_DIS = 0x0000  */
      iicWriteIndexedReg(I2C_ADDR_BT861, 0x5B, 0x00, I2C_BUS_BT861);
   }
   else
   {
      CNXT_SET_VAL(glpDrmTTField1, DRM_FIELD1_TELETEXT_FIRST_LINE_MASK, TTX_FIRST_LINE + 1);
      CNXT_SET_VAL(glpDrmTTField1, DRM_FIELD1_TELETEXT_LAST_LINE_MASK, TTX_FIRST_LINE + TTX_LINES_PER_FIELD - 1);
      CNXT_SET_VAL(glpDrmTTField2, DRM_FIELD1_TELETEXT_FIRST_LINE_MASK, TTX_FIRST_LINE);
      CNXT_SET_VAL(glpDrmTTField2, DRM_FIELD1_TELETEXT_LAST_LINE_MASK, TTX_FIRST_LINE + TTX_LINES_PER_FIELD - 1);
      /* set up the 865 for lines 6 to 22 and 7 to 23 */
      iicWriteIndexedReg(I2C_ADDR_BT865, 0xB4, TTX_FIRST_LINE, I2C_BUS_BT865);                             /* TXBF1 = std pal line       txbf1 = 7   */
      iicWriteIndexedReg(I2C_ADDR_BT865, 0xB6, TTX_FIRST_LINE + TTX_LINES_PER_FIELD - 1, I2C_BUS_BT865);   /* TXEF1 = std pal line       txef1 = 22  */
      iicWriteIndexedReg(I2C_ADDR_BT865, 0xB8, TTX_FIRST_LINE, I2C_BUS_BT865);                             /* TXBF2 + 313 = std pal line txbf2 = 7   */
      iicWriteIndexedReg(I2C_ADDR_BT865, 0xBA, TTX_FIRST_LINE + TTX_LINES_PER_FIELD - 1, I2C_BUS_BT865);   /* TXEF2 + 313 = std pal line txef2 = 22  */
   }
#else
   /* Since I2C writes every field crater the system, hard code the enables.     */
   CNXT_SET_VAL(glpDrmTTField1, DRM_FIELD1_TELETEXT_FIRST_LINE_MASK, TTX_FIRST_LINE+1);
   CNXT_SET_VAL(glpDrmTTField1, DRM_FIELD1_TELETEXT_LAST_LINE_MASK,  TTX_FIRST_LINE + TTX_LINES_PER_FIELD);
   
   CNXT_SET_VAL(glpDrmTTField2, DRM_FIELD1_TELETEXT_FIRST_LINE_MASK, TTX_FIRST_LINE);
   CNXT_SET_VAL(glpDrmTTField2, DRM_FIELD1_TELETEXT_LAST_LINE_MASK,  TTX_FIRST_LINE + TTX_LINES_PER_FIELD - 1);

   CNXT_SET_VAL(ENC_TTX_REQ_REG, ENC_TTX_REQ_CTL_TTXHS_MASK, 0x14C);
   CNXT_SET_VAL(ENC_TTX_REQ_REG, ENC_TTX_REQ_CTL_TTXHE_MASK, 0x009);
   
   CNXT_SET_VAL(ENC_TTX_F1_CTL_REG, ENC_TTXF1_CTL_TTXBF1_MASK, TTX_FIRST_LINE);
   CNXT_SET_VAL(ENC_TTX_F1_CTL_REG, ENC_TTXF1_CTL_TTXEF1_MASK,
                                    (TTX_FIRST_LINE + TTX_LINES_PER_FIELD - 1));

   CNXT_SET_VAL(ENC_TTX_F2_CTL_REG, ENC_TTXF2_CTL_TTXBF2_MASK, TTX_FIRST_LINE);
   CNXT_SET_VAL(ENC_TTX_F2_CTL_REG, ENC_TTXF2_CTL_TTXEF2_MASK,
                                    (TTX_FIRST_LINE + TTX_LINES_PER_FIELD - 1));
   
   CNXT_SET_VAL(ENC_TTX_LINE_CTL_REG, ENC_TTXLINE_CTL_TTX_DIS_MASK, 0);

#if TTX_REQ_IS_CLOCK
   CNXT_SET_VAL(ENC_TTX_LINE_CTL_REG, ENC_TTXLINE_CTL_TXRM_MASK, 1);
#else
   CNXT_SET_VAL(ENC_TTX_LINE_CTL_REG, ENC_TTXLINE_CTL_TXRM_MASK, 0);
#endif   
   
#endif

#endif   /*  defined(DRIVER_INCL_TVENC) */
}

genfilter_mode ttx_DataCB(pgen_notify_t pNotifyData) {
    int qu_msg[4] =  {0, 0, 0, 0};
    int status;
    
    qu_msg[0] = pNotifyData->chid;
    qu_msg[1] = pNotifyData->length;
    if ( RC_OK != (status = qu_send(ttx_queue_id, qu_msg)) )
        trace_new(TRACE_OSD | TRACE_LEVEL_ALWAYS, "qu_send status 0x%08x\n", status);
    return GENDEMUX_CONTINUOUS;
    
}

genfilter_mode ttx_ErrorCB(pgen_notify_t pNotifyData) {
    trace_new(TRACE_OSD | TRACE_LEVEL_ALWAYS, "ttx20.c  Received error callback!\n");
    return GENDEMUX_CONTINUOUS;
}

void ttx_task(void)  {
    int qu_msg[4];
    int i, nb = 0;
    
    while (1) {
        qu_receive(ttx_queue_id, KAL_WAIT_FOREVER, qu_msg);
        /* find which ttx instance this dmx chid belongs to */
        for (i = 0; i < TTX_MAX_CLIENTS; i++) {
            if (sttx_cd[i].dmxchid == qu_msg[0])  {
               break;
            }
        }
        if (i == TTX_MAX_CLIENTS)  {
            trace_new(TRACE_OSD | TRACE_LEVEL_ALWAYS, "ttx20.c  ttx_task:  chID from DMX doesn't pair with a TTX instance!\n");
        }
        
#if TTX_DEBUG
       trace_new(TRACE_OSD | TRACE_LEVEL_ALWAYS, "ttx20.c qu_msg[0] = 0x%08x qu_msg[1] = 0x%08x\n", qu_msg[0], qu_msg[1]);
#endif

#ifdef DRIVER_INCL_GENDMXC       
        nb = gen_dmx_read_pes_buffer(ttx_dmxHnd, sttx_cd[i].dmxchid, qu_msg[1], ttx_pes_data);
#else
       if (cnxt_dmx_read_pes_buffer(gDemuxInstance, sttx_cd[i].dmxchid, qu_msg[1], ttx_pes_data, (u_int32*)&nb) != DMX_STATUS_OK) {
         trace_new(TRACE_OSD | TRACE_LEVEL_ALWAYS, "ttx20.c  cnxt_dmx_read_pes_buffer failed.\n");
       }
#endif
        
#if TTX_DEBUG
       if ( nb != qu_msg[1] )
           trace_new(TRACE_OSD | TRACE_LEVEL_ALWAYS, "ttx20.c nb != qu_msg[1]\n");
#endif

#if TTX_DUMP_PES_DATA
{
    int ii, loop_limit;
    int pes_cnt = nb;
    unsigned char *pes_ptr = ttx_pes_data;
    char *trace_ptr;
    char tbuf[64];
    unsigned char hex[16] = {'0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F'};
    
    while( pes_cnt > 0 )
    {
        trace_ptr = tbuf;
        loop_limit = 20;
        if ( loop_limit > pes_cnt ) loop_limit = pes_cnt;
        pes_cnt -= loop_limit;
        for ( ii=0; ii<loop_limit; ++ii )
        {
            *trace_ptr++ = hex[((*pes_ptr)>>4)&0xf];
            *trace_ptr++ = hex[(*pes_ptr++)&0xf];
            *trace_ptr++ = ' ';
        }
        *trace_ptr++ = '\n';
        *trace_ptr++ = 0;
        trace("%s", tbuf);
    }
}
#endif

        while (i != -1) {
            ProcessPESHeader(nb, i);
            i = sttx_cd[i].pid_link;
        }
    }
}

void ProcessPESHeader(int nb, int client_id)  {
    int i;
    int OptHdrCount;
    int TTXDataLen;
    int teletext_type;
    u_int8 PTSFlag;
    u_int8 TTXDataID;
    u_int8 TTXPosition;
    int ttxField;
    int ttxLine;

    i = 0;
    while( i < nb ) {
        switch(ttx_pes_ps[client_id])  {
            case tpsTTXStreamID:
               /* If we don't find a teletext stream ID, look for one at the
                  next boundary. */
               if ( (ttx_pes_data[i] != 0) || (ttx_pes_data[i+1] != 0) ||
                    (ttx_pes_data[i+2] != 1) || (ttx_pes_data[i+3] != 0xBD) )
               {
                   trace_new(TRACE_OSD | TRACE_LEVEL_ALWAYS, "ttx20.c  ProcessPESHeader:  not a ttx stream ID 0x%02x%02x%02x%02x\n",
                        ttx_pes_data[i],ttx_pes_data[i+1],ttx_pes_data[i+2],ttx_pes_data[i+3]);
                   /* Stream ID can only occur at transport packet content boundary. */
                   i = ((i/184)+1)*184;
                   break;
               }

               /* Step over the stream ID. */
               i += 4;

               /* At this point, ttx_pes_data[i] is a teletext PES header. */

               /* Get the flag that indicates if there is a PTS specified in
                  the header. If there is, get it as well. */
               PTSFlag = ttx_pes_data[i+PTS_FLAG_OFFSET] & PTS_FLAG_BIT;
               if (PTSFlag != 0) {
                  this_pts[client_id] = (
                    ((0xfe & (long long)ttx_pes_data[i+PTS_OFFSET+4] ) >> 1  )  | 
                    ((0xff & (long long)ttx_pes_data[i+PTS_OFFSET+3] ) << 7  )  |
                    ((0xfe & (long long)ttx_pes_data[i+PTS_OFFSET+2] ) << 14 )  |
                    ((0xff & (long long)ttx_pes_data[i+PTS_OFFSET+1] ) << 22 )  |
                    ((0x0e & (long long)ttx_pes_data[i+PTS_OFFSET+0] ) << 29 )   );
               }

               /* Get the optional header length and step over the rest of the header. */
               OptHdrCount = ttx_pes_data[i+OPT_HDR_LEN_OFFSET];
               i += 5;            /* Step over the PES length, option bits and header length. */
               i += OptHdrCount;  /* Step over the optional header. */

               /* Check to make sure the data identifier is for teletext. */
               TTXDataID = ttx_pes_data[i];
               if ((TTXDataID < 0x10) || (TTXDataID > 0x1F)) {
                   /* PES Data ID was not for TTX, look for stream ID at the next boundary. */
                   trace_new(TRACE_OSD | TRACE_LEVEL_ALWAYS, "ttx20.c  ProcessPESHeader:  bad TTX data ID 0x%02x\n", TTXDataID);
                   ttx_pes_ps[client_id] = tpsTTXStreamID;
                   i = ((i/184)+1)*184;
                   break;
               }
               ++i;
               
               /* After parsing the header, we should be at a 46-byte boundary.
                  Look for another stream ID if we are not. */
               if (i % 46 != 0) {
                   trace_new(TRACE_OSD | TRACE_LEVEL_ALWAYS, "ttx20.c  ProcessPESHeader:  bad boundary\n");
                   ttx_pes_ps[client_id] = tpsTTXStreamID;
                   i = ((i/184)+1)*184;
                   break;
               }

               ttx_pes_ps[client_id] = tpsTTXDataID;
               /* fall through */

           /* At this point in parsing, we must be at a 46-byte boundary. We
              expect to find a teletext data segment, or perhaps a start code. */
           case tpsTTXDataID:
               /* If we are also at a 184-byte boundary, check for a start code. */
               if ( (i % 184 == 0) && (ttx_pes_data[i] == 0) && (ttx_pes_data[i+1] == 0) &&
                    (ttx_pes_data[i+2] == 1) )
               {
                   ttx_pes_ps[client_id] = tpsTTXStreamID;
                   break;
               }

               /* Collect the teletext data type and length. Error check the length. */
               teletext_type = ttx_pes_data[i+TTX_TYPE_OFFSET];
               TTXDataLen = ttx_pes_data[i+TTX_LENGTH_OFFSET];
               if ( TTXDataLen != 44 )
               {
                   trace_new(TRACE_OSD | TRACE_LEVEL_ALWAYS, "ttx20.c  ProcessPESHeader:  bad TTX data length\n");
                   ttx_pes_ps[client_id] = tpsTTXStreamID;
                   i = ((i/184)+1)*184;
                   break;
               }

               /* Collect the teletext field/line information. */
               TTXPosition = ttx_pes_data[i+TTX_POSITION_OFFSET];
               ttxField = (TTXPosition >> 5) & 1;
               ttxLine = TTXPosition & 0x1f;

               /* Process the data. */
               if ((teletext_type == 2) || (teletext_type == 3)) {
                   switch (sttx_cd[client_id].typ) {
                        case CNXT_TTX_INSERT:
                             /* there's no real way to find an error here without analyzing the
                                contents of the packet.  For insertion, assume if we get here that
                                the next 43 bytes are correct. */
                             ttx_do_insert(&ttx_pes_data[i+TTX_POSITION_OFFSET], this_pts[client_id], ttxField, ttxLine);
                             break;
                        case CNXT_TTX_EXTRACT:
                             ttx_do_extract(client_id, &ttx_pes_data[i+TTX_FRAMING_OFFSET]);
                             break;
                   }
               }

               /* Step over the data segment and look for more. */
               i += 46;
               ttx_pes_ps[client_id] = tpsTTXDataID;
               break;
        }
    }
}

long long calc_PTS_offset( long long pts, long long stc )
{
/* Handle wrap conditions.  If the high bits do not match, add the full range
   of an STC value to the lower of the PTS and STC. */
    if ( (pts & 0x100000000LL) != (stc & 0x100000000LL) )
    {
        if ( pts & 0x100000000LL )
        {
            stc += 0x200000000LL;
        }
        else
        {
            pts += 0x200000000LL;
        }
    }
    return stc - pts;
}

int ttx_PTS_in_window( long long pts, long long stc )
{
    long long diff;

/* If the PTS is unknown, it cannot be in the window. */
    if ( pts == -1 )
    {
        return 0;
    }

/* Calculate the difference between the STC and PTS and compare it to the limits. */
    diff = calc_PTS_offset( pts, stc );
    if ( diff > 5400 )
    {
        return 0;
    }
    if ( diff < -14400 )
    {
        return 0;
    }

    return 1;
}

cnxt_ttx_status ttx_do_insert(unsigned char *buf, long long pts, int field, int line)  {
    unsigned int cs;
    unsigned char *pDest;
    int current_in_window, first_in_window, queue_size;
#if TTX_DUMP_INSERT_DATA
#if 999
    unsigned char hex[16] = {'0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F'};
    int dumpswap;
    static unsigned char dumpbuf[256];
    unsigned char *outptr = dumpbuf;
    unsigned char C;
#else
    unsigned char dumpbuf[50];
#endif
    int i;
#endif
    u_int32 Hi, Lo;
    long long stc;

    cs = critical_section_begin();
    Lo = *glpSTCLo;
    Hi = (*glpSTCHi) & 1;
    stc = (((long long)Hi) << 32) + (long long)Lo;
    critical_section_end(cs);

/* Check our output mode and the queue occupancy as appropriate to try to avoid
   queue overflows and keep the output process as synchronized as it can be. */

    /* Change to TTX_SYNC_OVERFLOW if the queue is already full. */
    if ( NEXT_LINE( ttx_inline ) == ttx_outline )
    {
        if ( ttx_sync_mode != TTX_SYNC_OVERFLOW )
        {
#if TTX_DEBUG
            trace_new(TRACE_OSD | TRACE_LEVEL_ALWAYS, "ttx20.c  ttx_do_insert:  Queue overflow, change to TTX_SYNC_OVERFLOW\n");
#endif
            ttx_sync_mode = TTX_SYNC_OVERFLOW;
        }
        return (CNXT_TTX_FAILURE);
    }

    /* If the queue is currently empty, decide between syncing just by line or
       by PTS and line based on whether the PTS of the input line is within an
       acceptable window from the current STC. */
    current_in_window = ttx_PTS_in_window( pts, stc );
    if ( ttx_inline == ttx_outline )
    {
        if ( current_in_window )
        {
            if ( ttx_sync_mode != TTX_SYNC_PTS )
            {
#if TTX_DEBUG
                trace_new(TRACE_OSD | TRACE_LEVEL_ALWAYS, "ttx20.c  ttx_do_insert:  PTS in window, change to TTX_SYNC_PTS\n");
#endif
            }
            ttx_sync_mode = TTX_SYNC_PTS;
        }
        else
        {
            if ( ttx_sync_mode != TTX_SYNC_LINE )
            {
#if TTX_DEBUG
                trace_new(TRACE_OSD | TRACE_LEVEL_ALWAYS, "ttx20.c  ttx_do_insert:  PTS not in window, change to TTX_SYNC_LINE\n");
#endif
            }
            ttx_sync_mode = TTX_SYNC_LINE;
        }
        first_in_window = current_in_window;
    }
    else
    {
        first_in_window = ttx_PTS_in_window( ttx_buf[ttx_outline].pts, stc );
    }

    /* If we are syncing the output at all, change to TTX_SYNC_NONE if the queue
       is getting too full, OR if we are syncing by PTS but the current line PTS
       is outside of the window or the first line PTS is outside the window OR
       if we are syncing by line and the current line PTS is now inside the 
       window. */
    queue_size = ttx_inline - ttx_outline;
    if ( queue_size < 0 )
    {
        queue_size += TTX_NUM_BUFFERED_LINES;
    }
    if ( ttx_sync_mode == TTX_SYNC_PTS  ||  ttx_sync_mode == TTX_SYNC_LINE )
    {
        if ( queue_size > ( (3*TTX_NUM_BUFFERED_LINES) / 4 ) )
        {
#if TTX_DEBUG
            trace_new(TRACE_OSD | TRACE_LEVEL_ALWAYS, "ttx20.c  ttx_do_insert:  Queue filling, change to TTX_SYNC_NONE\n");
#endif
            ttx_sync_mode = TTX_SYNC_NONE;
        }
        else if ( ttx_sync_mode == TTX_SYNC_PTS  &&  ( !current_in_window || !first_in_window ) )
        {
#if TTX_DEBUG
            trace_new(TRACE_OSD | TRACE_LEVEL_ALWAYS, "ttx20.c  ttx_do_insert:  PTS not in window, change to TTX_SYNC_NONE\n");
#endif
            ttx_sync_mode = TTX_SYNC_NONE;
        }
        else if ( ttx_sync_mode == TTX_SYNC_LINE  &&  current_in_window )
        {
#if TTX_DEBUG
            trace_new(TRACE_OSD | TRACE_LEVEL_ALWAYS, "ttx20.c  ttx_do_insert:  PTS in window, change to TTX_SYNC_NONE\n");
#endif
            ttx_sync_mode = TTX_SYNC_NONE;
        }
    }

    ttx_buf[ttx_inline].pts = pts;
    ttx_buf[ttx_inline].field = field;
    ttx_buf[ttx_inline].vbi_line = line;
    pDest = ttx_buf[ttx_inline].data;
    pDest[0] = pDest[1] = 0xAA; /* clock run in (0xCC bitswapped) */
    pDest += 2;

    if (buf[1] == 0xE4)  {
        FCopy(pDest, &buf[1], 44);
    } else if (buf[1] == 0x27) {
        int i;
        for (i = 1; i < 45; i++ ) {
            pDest[i] = gbyBitSwap[buf[i]];
        }
    } else {
        trace("framing code was neither 0xE4 nor 0x27\n");
    }

/* data is now valid, bump the index */
    ttx_inline = NEXT_LINE( ttx_inline );

#if TTX_DUMP_INSERT_DATA
#if 999
    dumpswap = (buf[1] == 0xe4);
    *outptr++ = '<';
    for ( i=0; i<45; ++i )
    {
        C = dumpswap ? gbyBitSwap[buf[i]] : buf[i];
        *outptr++ = hex[(C>>4)&0xf];
        *outptr++ = hex[C&0xf];
        *outptr++ = ' ';
    }
    *outptr++ = '>';
    *outptr++ = '\n';
    *outptr++ = 0;
    trace("%s", dumpbuf);
#else
    FCopy(dumpbuf, buf, 45);
    if (dumpbuf[1] == 0xE4)  {
        for (i = 0; i < 44; i++) {
            dumpbuf[i] = 0x7f & gbyBitSwap[dumpbuf[i]];
            if ((dumpbuf[i] < 0x30) || (dumpbuf[i] > 0x7F)) {
                dumpbuf[i] = ' ';
            }
        }
    } else {
        for (i = 0; i < 44; i++) {
            if ((dumpbuf[i] < 0x30) || (dumpbuf[i] > 0x7F)) {
                dumpbuf[i] = ' ';
            }
        }
    }        
    dumpbuf[43] = 0;
    trace("ttx  :   %s\n", dumpbuf);
#endif    
#endif    

    return (CNXT_TTX_SUCCESS);
}
#if 0
cnxt_ttx_status ttx_do_extract(int client_id, unsigned char *buf)  {
   int i;
   int mag, packet;
   int control = 0;
   int page;
   int sts0, sts1, sts2, sts3;
   int  x ,y, h0, h1, h2, h3;
   uSubcode subcode;

   if (buf[0] == 0xE4)  {
       for (i = 0; (i < 44) && (i < sttx_cd[client_id].ttxbufsize); i++)  {
           sttx_cd[client_id].ttxbuf[i] = gbyBitSwap[buf[i]];
       }
   } else {
       FCopy((unsigned char*)sttx_cd[client_id].ttxbuf, (unsigned char*)buf, 44);
   }
  
   sts1 =  HammingDecode ( CODING_8_4, sttx_cd[client_id].ttxbuf[1], &x );
   sts2 =  HammingDecode ( CODING_8_4, sttx_cd[client_id].ttxbuf[2], &y );

   if ( (sts1 == 0) && (sts2 == 0) ) {
       mag = x & 0x7;
       packet = (y << 1) | ((x & 0x8) >> 3);

       if ( packet > 32 ) {
           return CNXT_TTX_FAILURE;
       }

       page = -99;
       if ( packet == 0 ) {
           /* packet 0 is the beginning of page
           if we have packet zero, we know that
           the next 28 packets with the correct magazine
           all belong to this page. */


           sts0 = HammingDecode ( CODING_8_4, sttx_cd[client_id].ttxbuf[4], &h0 );
           sts1 = HammingDecode ( CODING_8_4, sttx_cd[client_id].ttxbuf[3], &h1 );
           if ( (sts0 == 0) && (sts1 == 0) ) {
               page = ( 0x10 * h0 ) + h1;
           } else {
               return CNXT_TTX_FAILURE;
           }

           sts0 = HammingDecode ( CODING_8_4, sttx_cd[client_id].ttxbuf[5], &h0 );
           sts1 = HammingDecode ( CODING_8_4, sttx_cd[client_id].ttxbuf[6], &h1 );
           sts2 = HammingDecode ( CODING_8_4, sttx_cd[client_id].ttxbuf[7], &h2 );
           sts3 = HammingDecode ( CODING_8_4, sttx_cd[client_id].ttxbuf[8], &h3 );

           if ( (sts0 == 0) && (sts1 == 0) && (sts2 == 0) && (sts3 == 0) ) {
               subcode.sc.s1 = h0 & 0xf;
               subcode.sc.s2 = h1 & 0x7; 
               subcode.sc.s3 = h2 & 0xf;
               subcode.sc.s4 = h3 & 0x3;
           } else {
               return CNXT_TTX_FAILURE;
           }

           /* remaining control bits are in bytes 8 and 9 */
           sts0 = HammingDecode ( CODING_8_4, sttx_cd[client_id].ttxbuf[9], &h0 );
           sts2 = HammingDecode ( CODING_8_4, sttx_cd[client_id].ttxbuf[10], &h2 );


           if ( (sts0 == 0) && (sts2 == 0) ) {
               control = ((h1 & 0x8) >> 3) |  /* Control bit  4      */
               ((h3 & 0xc) >> 1) |  /* Control bits 5,6    */
               (h0        << 3) |  /* Control bits 7-10   */
               (h2        << 7);   /* Control bits 11-14  */
           } else {
               return(CNXT_TTX_FAILURE);
           }
       } /* packet == 0 */
   } else { /* ((sts1 == 0) && (sts2 == 0)) */
       trace_new ( TRACE_OSD | TRACE_LEVEL_ALWAYS, "bad hamming decode\n" );
       return(CNXT_TTX_FAILURE);
   }


#if 0   
   if ( packet == 0 ) {
       if ( sttx_cd[client_id].acquiring ) {
           if ( control & 0x80 ) {                               /* serial mode */
               trace_new(TRACE_OSD | TRACE_LEVEL_1, "serial\n");
               /* If this is a serial packet 0 and we were acquiring, this terminates
                  the acquisition regardless of page and magazine number */
               sttx_cd[client_id].acquiring = 0;

               /* callback with null */

               /* If this page and magazine match our request, handle this packet,
                  otherwise there is nothing to do so exit */
               if ((sttx_cd[client_id].page != page) || (sttx_cd[client_id].mag != mag))  {
                   return(CNXT_TTX_SUCCESS);
               }
           } else {                                  /* parallel mode */
               trace_new(TRACE_OSD | TRACE_LEVEL_1, "parallel\n");
               if ( sttx_cd[client_id].mag == mag ) {
                   /* If the stream is in parallel mode then IFF the magazines match 
                      AND the pages do NOT then acquisition is done. */
                   if (sttx_cd[client_id].page != page)  {
                       sttx_cd[client_id].acquiring = 0;
                       /* callback with null */
                   }
               } else {
                   /* If the magazines don't match in parallel mode, do nothing */
                   return(CNXT_TTX_SUCCESS);
               }
           }
       } /* if ( sttx_cd[client_id].acquring ) */

       if ( sttx_cd[client_id].mag == mag ) {
           if ( sttx_cd[client_id].page != -1 ) {
               if ( sttx_cd[client_id].page == page ) {
                   if ( sttx_cd[client_id].subcode != -1 ) {
                       if ( sttx_cd[client_id].subcode == subcode.word ) {
                           /* magazine, page, and subcode all matched, so start acquiring */
                           sttx_cd[client_id].acquiring = 1;
                       }
                   } else { /* subcode was -1 and pages matched so start acquiring */
                       sttx_cd[client_id].acquiring = 1;
                   } 
               }
           } else { /* page was -1 and magazines matched so start acquiring */
               sttx_cd[client_id].acquiring = 1;
           } 
       } /* pReq->magazine_number ==  mag */

   } /* packet == 0 */

   if ( (mag == sttx_cd[client_id].mag) && (sttx_cd[client_id].acquiring) && (sttx_cd[client_id].fncb != 0)) {
       for (i = 0; (i < 44) && (i < sttx_cd[client_id].ttxbufsize); i++)  {
           sttx_cd[client_id].ttxbuf[i] = sttx_cd[client_id].ttxbuf[i] & 0x7f;
       }
       (sttx_cd[client_id].fncb)(client_id, packet, (void*)sttx_cd[client_id].ttxbuf);
   }
   #endif

   if ( sttx_cd[client_id].fncb != 0) {
       (sttx_cd[client_id].fncb)(client_id, packet, (void*)sttx_cd[client_id].ttxbuf);
   }
   
   return(CNXT_TTX_SUCCESS);
   
}
#else
extern int decode_hamming_84 (unsigned char codeword, unsigned char * pOutput);
cnxt_ttx_status ttx_do_extract(int client_id, unsigned char *buf)  {
   int i;
   int mag, packet;
   int control = 0;
   int page;
   int sts0, sts1, sts2, sts3;
   unsigned char  x ,y, h0, h1, h2, h3;
   uSubcode subcode;

   if (buf[0] == 0xE4)  {
       for (i = 0; (i < 44) && (i < sttx_cd[client_id].ttxbufsize); i++)  {
           sttx_cd[client_id].ttxbuf[i] = gbyBitSwap[buf[i]];
       }
   } else {
       FCopy((unsigned char*)sttx_cd[client_id].ttxbuf, (unsigned char*)buf, 44);
   }
  
   sts1 =  decode_hamming_84 ( sttx_cd[client_id].ttxbuf[1], &x );
   sts2 =  decode_hamming_84 ( sttx_cd[client_id].ttxbuf[2], &y );

   if ( (sts1 == 0) && (sts2 == 0) ) {
       mag = x & 0x7;
       packet = (y << 1) | ((x & 0x8) >> 3);

       if ( packet > 32 ) {
           return CNXT_TTX_FAILURE;
       }

       page = -99;
       if ( packet == 0 ) {
           /* packet 0 is the beginning of page
           if we have packet zero, we know that
           the next 28 packets with the correct magazine
           all belong to this page. */

           sts0 = decode_hamming_84 ( sttx_cd[client_id].ttxbuf[4], &h0 );
           sts1 = decode_hamming_84 ( sttx_cd[client_id].ttxbuf[3], &h1 );
           if ( (sts0 == 0) && (sts1 == 0) ) {
               page = ( 0x10 * h0 ) + h1;
           } else {
               return CNXT_TTX_FAILURE;
           }

           sts0 = decode_hamming_84 ( sttx_cd[client_id].ttxbuf[5], &h0 );
           sts1 = decode_hamming_84 ( sttx_cd[client_id].ttxbuf[6], &h1 );
           sts2 = decode_hamming_84 ( sttx_cd[client_id].ttxbuf[7], &h2 );
           sts3 = decode_hamming_84 ( sttx_cd[client_id].ttxbuf[8], &h3 );

           if ( (sts0 == 0) && (sts1 == 0) && (sts2 == 0) && (sts3 == 0) ) {
               subcode.sc.s1 = h0 & 0xf;
               subcode.sc.s2 = h1 & 0x7; 
               subcode.sc.s3 = h2 & 0xf;
               subcode.sc.s4 = h3 & 0x3;
           } else {
               return CNXT_TTX_FAILURE;
           }

           /* remaining control bits are in bytes 8 and 9 */
           sts0 = decode_hamming_84 ( sttx_cd[client_id].ttxbuf[9], &h0 );
           sts2 = decode_hamming_84 ( sttx_cd[client_id].ttxbuf[10], &h2 );


           if ( (sts0 == 0) && (sts2 == 0) ) {
               control = ((h1 & 0x8) >> 3) |  /* Control bit  4      */
               ((h3 & 0xc) >> 1) |  /* Control bits 5,6    */
               (h0        << 3) |  /* Control bits 7-10   */
               (h2        << 7);   /* Control bits 11-14  */
           } else {
               return(CNXT_TTX_FAILURE);
           }
       } /* packet == 0 */
   } else { /* ((sts1 == 0) && (sts2 == 0)) */
       trace_new ( TRACE_OSD | TRACE_LEVEL_ALWAYS, "bad hamming decode\n" );
       return(CNXT_TTX_FAILURE);
   }

   if ( sttx_cd[client_id].fncb != 0) {
       (sttx_cd[client_id].fncb)(client_id, packet, (void*)sttx_cd[client_id].ttxbuf);
   }
   
   return(CNXT_TTX_SUCCESS);
   
}
#endif

void ttx_line_ISR(u_int32 dwLine, OSDFIELD oField) {
#if TTX_DEBUG_STATIC_BUFFER
    unsigned int  i;
#endif
    unsigned char* pDRM;
    unsigned char* pTmp;
    unsigned int lBuf;
    u_int32 physical_field, physical_line, send_current_line;
    long long pts_offset;

#if TTX_DUMP_ISR_DATA
    unsigned char dumpbuf[50];
#endif
    unsigned int drm_outline;
    unsigned int current_line = CNXT_GET_VAL(glpDrmStatus, DRM_STATUS_LINE_MASK);
    u_int32 Hi, Lo;
    long long stc;

    Lo = *glpSTCLo;
    Hi = (*glpSTCHi) & 1;
    stc = (((long long)Hi) << 32) + (long long)Lo;

    /* If the current output line is not well away from the vbi, skip this time. */
    if ( (current_line < 30) || (current_line > 300) )
    {
        isr_trace_new(TRACE_OSD | TRACE_LEVEL_ALWAYS, "ttx20.c  ttx_line_ISR:  skipping this callback!\n",0,0);
        return;
    }

#if TTX_DEBUG_STATIC_BUFFER

    lBuf = (oField == TOP ? (unsigned int)pDRMbotbuf : (unsigned int)pDRMtopbuf );
    for (i = 0; i < 12; i++) {

        pDRM = (unsigned char*)( lBuf + (i * TTX_DRM_LINE_SIZE) | BSWAP_OFFSET);
        pTmp = (oField == TOP ? (unsigned char*)gbyTesttopData[i] : (unsigned char*)gbyTestbotData[i]);
        
        FCopy(pDRM, pTmp, TTX_DRM_LINE_SIZE);
        
    }

#else

    drm_outline = 0;
    physical_line = 7;
    if ( oField == TOP )
    {
        physical_field = 1;
        lBuf = (unsigned int)pDRMbotbuf;
    }
    else
    {
        physical_field = 0;
        lBuf = (unsigned int)pDRMtopbuf;
    }

    /* For each output line, determine if the line data at the head of the queue
       will be sent, or the output line will be set to zeroes. */
    while ( drm_outline < TTX_LINES_PER_FIELD )
    {
        /* Default to sending zeroes.  Only try to send real data if the queue
           is not empty. */
        send_current_line = FALSE;
        if ( ttx_outline != ttx_inline )
        {
            switch ( ttx_sync_mode )
            {
                case TTX_SYNC_PTS:
                    pts_offset = calc_PTS_offset( ttx_buf[ttx_outline].pts, stc );
                    if ( pts_offset >= 0  &&
                            physical_field == ttx_buf[ttx_outline].field  &&
                            (physical_line == ttx_buf[ttx_outline].vbi_line  ||
                                    ttx_buf[ttx_outline].vbi_line == 0) )
                    {
                        send_current_line = TRUE;
                    }
                    break;

                case TTX_SYNC_LINE:
                    if ( physical_field == ttx_buf[ttx_outline].field  &&
                            (physical_line == ttx_buf[ttx_outline].vbi_line  ||
                                    ttx_buf[ttx_outline].vbi_line == 0) )
                    {
                        send_current_line = TRUE;
                    }
                    break;

                case TTX_SYNC_NONE:
                case TTX_SYNC_OVERFLOW:
                    send_current_line = TRUE;
                    break;

                default:
                    break;

            }
        }
        
        pDRM = (unsigned char*)( (lBuf + (drm_outline * TTX_DRM_LINE_SIZE)) | BSWAP_OFFSET);
        if ( send_current_line )
        {
            pTmp = (unsigned char*)ttx_buf[ttx_outline].data;
            FCopy(pDRM, pTmp, TTX_DRM_LINE_SIZE);
#if TTX_DUMP_ISR_DATA
            FCopy(dumpbuf, pTmp, 44);
            if (dumpbuf[2] == 0xE4)  {
                for (i = 0; i < 44; i++) {
                    dumpbuf[i] = 0x7f & gbyBitSwap[dumpbuf[i]];
                    if ((dumpbuf[i] < 0x30) || (dumpbuf[i] > 0x7F)) {
                        dumpbuf[i] = ' ';
                    }
                }
            } else {
                for (i = 0; i < 44; i++) {
                    if ((dumpbuf[i] < 0x30) || (dumpbuf[i] > 0x7F)) {
                        dumpbuf[i] = ' ';
                    }
                }
            }        
            dumpbuf[43] = 0;
            dumpbuf[42] = '\n';
            isr_trace((char*)dumpbuf,0,0);
#endif    
            ttx_outline = NEXT_LINE( ttx_outline );
        }
        else
        {
            FFillBytes(pDRM, 0, TTX_DRM_LINE_SIZE);
        }

        ++drm_outline;
        ++physical_line;
    }
#endif
}


const unsigned char gbyBitSwap[256] =
                      {0x00, 0x80, 0x40, 0xC0, 0x20, 0xA0, 0x60, 0xE0,
                       0x10, 0x90, 0x50, 0xD0, 0x30, 0xB0, 0x70, 0xF0,
                       0x08, 0x88, 0x48, 0xC8, 0x28, 0xA8, 0x68, 0xE8,
                       0x18, 0x98, 0x58, 0xD8, 0x38, 0xB8, 0x78, 0xF8,
                       0x04, 0x84, 0x44, 0xC4, 0x24, 0xA4, 0x64, 0xE4,
                       0x14, 0x94, 0x54, 0xD4, 0x34, 0xB4, 0x74, 0xF4,
                       0x0C, 0x8C, 0x4C, 0xCC, 0x2C, 0xAC, 0x6C, 0xEC,
                       0x1C, 0x9C, 0x5C, 0xDC, 0x3C, 0xBC, 0x7C, 0xFC,
                       0x02, 0x82, 0x42, 0xC2, 0x22, 0xA2, 0x62, 0xE2,
                       0x12, 0x92, 0x52, 0xD2, 0x32, 0xB2, 0x72, 0xF2,
                       0x0A, 0x8A, 0x4A, 0xCA, 0x2A, 0xAA, 0x6A, 0xEA,
                       0x1A, 0x9A, 0x5A, 0xDA, 0x3A, 0xBA, 0x7A, 0xFA,
                       0x06, 0x86, 0x46, 0xC6, 0x26, 0xA6, 0x66, 0xE6,
                       0x16, 0x96, 0x56, 0xD6, 0x36, 0xB6, 0x76, 0xF6,
                       0x0E, 0x8E, 0x4E, 0xCE, 0x2E, 0xAE, 0x6E, 0xEE,
                       0x1E, 0x9E, 0x5E, 0xDE, 0x3E, 0xBE, 0x7E, 0xFE,
                       0x01, 0x81, 0x41, 0xC1, 0x21, 0xA1, 0x61, 0xE1,
                       0x11, 0x91, 0x51, 0xD1, 0x31, 0xB1, 0x71, 0xF1,
                       0x09, 0x89, 0x49, 0xC9, 0x29, 0xA9, 0x69, 0xE9,
                       0x19, 0x99, 0x59, 0xD9, 0x39, 0xB9, 0x79, 0xF9,
                       0x05, 0x85, 0x45, 0xC5, 0x25, 0xA5, 0x65, 0xE5,
                       0x15, 0x95, 0x55, 0xD5, 0x35, 0xB5, 0x75, 0xF5,
                       0x0D, 0x8D, 0x4D, 0xCD, 0x2D, 0xAD, 0x6D, 0xED,
                       0x1D, 0x9D, 0x5D, 0xDD, 0x3D, 0xBD, 0x7D, 0xFD,
                       0x03, 0x83, 0x43, 0xC3, 0x23, 0xA3, 0x63, 0xE3,
                       0x13, 0x93, 0x53, 0xD3, 0x33, 0xB3, 0x73, 0xF3,
                       0x0B, 0x8B, 0x4B, 0xCB, 0x2B, 0xAB, 0x6B, 0xEB,
                       0x1B, 0x9B, 0x5B, 0xDB, 0x3B, 0xBB, 0x7B, 0xFB,
                       0x07, 0x87, 0x47, 0xC7, 0x27, 0xA7, 0x67, 0xE7,
                       0x17, 0x97, 0x57, 0xD7, 0x37, 0xB7, 0x77, 0xF7,
                       0x0F, 0x8F, 0x4F, 0xCF, 0x2F, 0xAF, 0x6F, 0xEF,
                       0x1F, 0x9F, 0x5F, 0xDF, 0x3F, 0xBF, 0x7F, 0xFF};

#if (TTX_DEBUG_STATIC_BUFFER == 1)
#define TTX_PAD 0
u_int8 gbyTesttopData[12][48] = {
{ 0xAA,0xAA,0xE4,0x40,0xA8,0xA8,0xA8,0xA8,0xA8,0xA8,0xA8,0xA8,0xA8,0x0B,0xB3,0xAD,0x6D,0x2C,0xAD,0x04,0x0B,0x8C,0x0D,0x0D,0x04,0x04,0xB3,0xF7,0x76,0x04,0x04,0x8C,0x04,0x52,0x86,0x76,0x04,0x8C,0x4C,0x5D,0x0D,0x0D,0x5D,0x0D,0x0D,TTX_PAD,TTX_PAD,TTX_PAD },
{ 0xAA,0xAA,0xE4,0xE3,0xA8,0xC8,0x3D,0x34,0x0D,0x68,0x3D,0x34,0x25,0x49,0x2C,0x04,0x04,0xA8,0x3D,0x34,0x0D,0x89,0x2C,0x04,0x2C,0x29,0x2C,0x04,0x04,0xFE,0xCE,0xC7,0xDF,0xFE,0xC7,0xCE,0xDF,0xFE,0xE6,0xCE,0xF7,0xFE,0xCE,0xC7,0xDF,TTX_PAD,TTX_PAD,TTX_PAD },
{ 0xAA,0xAA,0xE4,0x40,0x40,0xC8,0xAD,0x04,0xAD,0x68,0xAE,0x0D,0x04,0x49,0xAD,0x04,0x04,0xA8,0xAE,0x0E,0xA4,0x89,0xAE,0x0E,0xAD,0x29,0xAD,0x04,0x04,0xFE,0xFE,0x57,0xFE,0xFE,0x54,0xF7,0xFE,0xFE,0x5D,0xF4,0xFE,0xFE,0xFE,0x57,0xFE,TTX_PAD,TTX_PAD,TTX_PAD },
{ 0xAA,0xAA,0xE4,0xE3,0x40,0xC8,0xAD,0x04,0xAD,0x68,0xAD,0x04,0x04,0x49,0xAD,0x04,0x04,0xA8,0xAD,0x04,0x04,0x89,0xAD,0x04,0xAD,0x29,0xAD,0x04,0x04,0xFE,0xFE,0x57,0xFE,0xFE,0x57,0xFE,0xFE,0xFE,0xF7,0xFE,0x57,0xFE,0xFE,0x57,0xFE,TTX_PAD,TTX_PAD,TTX_PAD },
{ 0xAA,0xAA,0xE4,0x40,0x92,0xC8,0xC4,0xC4,0x04,0x68,0xC4,0xC4,0x85,0x49,0xC4,0xC4,0x85,0xA8,0x85,0x04,0x04,0x89,0x85,0x04,0x85,0x29,0x85,0x04,0x04,0xFE,0xFE,0x7F,0xFE,0xFE,0x3E,0x3E,0x7F,0xFE,0xBF,0x3E,0xFE,0xFE,0xFE,0x7F,0xFE,TTX_PAD,TTX_PAD,TTX_PAD },
{ 0xAA,0xAA,0xE4,0xE3,0x92,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,TTX_PAD,TTX_PAD,TTX_PAD },
{ 0xAA,0xAA,0xE4,0x40,0x7A,0xB0,0x83,0x43,0xC2,0x23,0xA2,0x62,0xE3,0x13,0x92,0x52,0x04,0xD3,0x32,0xB3,0x73,0xF2,0x0B,0x8A,0x4A,0x04,0xCB,0x2A,0xAB,0x6B,0xEA,0x1A,0x9B,0x5B,0x04,0x0D,0x8C,0x4C,0xCD,0x2C,0xAD,0x6D,0xEC,0x1C,0x9D,TTX_PAD,TTX_PAD,TTX_PAD },
{ 0xAA,0xAA,0xE4,0xE3,0x7A,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,TTX_PAD,TTX_PAD,TTX_PAD },
{ 0xAA,0xAA,0xE4,0x40,0x26,0x04,0x83,0x43,0xC2,0x23,0xA2,0x62,0xE3,0x13,0x92,0x52,0x04,0xD3,0x32,0xB3,0x73,0xF2,0x0B,0x8A,0x4A,0x04,0xCB,0x2A,0xAB,0x6B,0xEA,0x1A,0x9B,0x5B,0x04,0x0D,0x8C,0x4C,0xCD,0x2C,0xAD,0x6D,0xEC,0x1C,0x9D,TTX_PAD,TTX_PAD,TTX_PAD },
{ 0xAA,0xAA,0xE4,0xE3,0x26,0x04,0x86,0x46,0xC7,0x26,0xA7,0x67,0xE6,0x16,0x97,0x57,0x04,0xD6,0x37,0xB6,0x76,0xF7,0x0E,0x8F,0x4F,0x04,0xCE,0x2F,0xAE,0x6E,0xEF,0x1F,0x9E,0x5E,0x04,0x0D,0x8C,0x4C,0xCD,0x2C,0xAD,0x6D,0xEC,0x1C,0x9D,TTX_PAD,TTX_PAD,TTX_PAD },
{ 0xAA,0xAA,0xE4,0x40,0xCE,0xC1,0x8A,0xAB,0x92,0xC2,0xD3,0x04,0x43,0x4A,0xF2,0xEA,0x73,0x04,0x62,0xF2,0x1A,0x04,0x52,0xAB,0xB3,0x0B,0xCB,0x04,0xF2,0x6B,0xA2,0x4A,0x04,0x2A,0x13,0xA2,0x04,0x32,0x83,0x5B,0x9B,0x04,0x23,0xF2,0xE3,TTX_PAD,TTX_PAD,TTX_PAD },
{ 0xAA,0xAA,0xE4,0xE3,0xCE,0xC1,0x8F,0xAE,0x97,0xC7,0xD6,0x04,0x46,0x4F,0xF7,0xEF,0x76,0x04,0x67,0xF7,0x1F,0x04,0x57,0xAE,0xB6,0x0E,0xCE,0x04,0xF7,0x6E,0xA7,0x4F,0x04,0x2F,0x16,0xA7,0x04,0x37,0x86,0x5E,0x9E,0x04,0x26,0xF7,0xE6,TTX_PAD,TTX_PAD,TTX_PAD }};

u_int8 gbyTestbotData[12][48] = {
{ 0xAA,0xAA,0xE4,0x40,0x1C,0x04,0xFE,0x7F,0xFE,0x7F,0xFE,0x7F,0xFE,0x7F,0xFE,0x7F,0xFE,0x7F,0xFE,0x7F,0xFE,0x7F,0xFE,0x7F,0xFE,0x7F,0xFE,0x7F,0xFE,0x7F,0xFE,0x7F,0xFE,0x7F,0xFE,0x7F,0xFE,0x7F,0xFE,0x7F,0xFE,0x7F,0xFE,0x7F,0xFE,TTX_PAD,TTX_PAD,TTX_PAD },
{ 0xAA,0xAA,0xE4,0xE3,0x1C,0x40,0xFE,0x7F,0xFE,0x7F,0xFE,0x7F,0xFE,0x7F,0xFE,0x7F,0xFE,0x7F,0xFE,0x7F,0xFE,0x7F,0xFE,0x7F,0xFE,0x7F,0xFE,0x7F,0xFE,0x7F,0xFE,0x7F,0xFE,0x7F,0xFE,0x7F,0xFE,0x7F,0xFE,0x7F,0xFE,0x7F,0xFE,0x7F,0xFE,TTX_PAD,TTX_PAD,TTX_PAD },
{ 0xAA,0xAA,0xE4,0x40,0xF4,0x80,0xFE,0x7F,0xFE,0x7F,0xFE,0x7F,0xFE,0x7F,0xFE,0x7F,0xFE,0x7F,0xFE,0x7F,0xFE,0x7F,0xFE,0x7F,0xFE,0x7F,0xFE,0x7F,0xFE,0x7F,0xFE,0x7F,0xFE,0x7F,0xFE,0x7F,0xFE,0x7F,0xFE,0x7F,0xFE,0x7F,0xFE,0x7F,0xFE,TTX_PAD,TTX_PAD,TTX_PAD },
{ 0xAA,0xAA,0xE4,0xE3,0xF4,0x20,0xFE,0x7F,0xFE,0x7F,0xFE,0x7F,0xFE,0x7F,0xFE,0x7F,0xFE,0x7F,0xFE,0x7F,0xFE,0x7F,0xFE,0x7F,0xFE,0x7F,0xFE,0x7F,0xFE,0x7F,0xFE,0x7F,0xFE,0x7F,0xFE,0x7F,0xFE,0x7F,0xFE,0x7F,0xFE,0x7F,0xFE,0x7F,0xFE,TTX_PAD,TTX_PAD,TTX_PAD },
{ 0xAA,0xAA,0xE4,0x40,0x0B,0x79,0x58,0xE9,0xFE,0xC8,0xFE,0x68,0xFE,0x49,0xFE,0xA8,0xFE,0x89,0xFE,0x29,0xFE,0x04,0x98,0x04,0x04,0x04,0x04,0xE0,0xB9,0xE0,0x54,0xC1,0x54,0x61,0x54,0x40,0x54,0xA1,0x54,0x80,0x54,0x20,0x54,0x04,0x04,TTX_PAD,TTX_PAD,TTX_PAD },
{ 0xAA,0xAA,0xE4,0xE3,0x0B,0x04,0x58,0xE9,0xFE,0xC8,0xFE,0x68,0xFE,0x49,0xFE,0xA8,0xFE,0x89,0xFE,0x29,0xFE,0x04,0x98,0xC1,0x04,0x04,0x04,0x04,0xB9,0xE0,0x54,0xC1,0x54,0x61,0x54,0x40,0x54,0xA1,0x54,0x80,0x54,0x20,0x54,0x04,0x04,TTX_PAD,TTX_PAD,TTX_PAD },
{ 0xAA,0xAA,0xE4,0x40,0xE3,0x04,0x04,0xE9,0xFE,0xC8,0xFE,0x68,0xFE,0x49,0xFE,0xA8,0xFE,0x89,0xFE,0x29,0xFE,0x04,0x04,0x61,0x04,0x04,0x04,0x04,0xB9,0xE0,0x54,0xC1,0x54,0x61,0x54,0x40,0x54,0xA1,0x54,0x80,0x54,0x20,0x54,0x04,0x04,TTX_PAD,TTX_PAD,TTX_PAD },
{ 0xAA,0xAA,0xE4,0xE3,0xE3,0x79,0x04,0xE9,0xFE,0xC8,0xFE,0x68,0xFE,0x49,0xFE,0xA8,0xFE,0x89,0xFE,0x29,0xFE,0x04,0x04,0x40,0x04,0x04,0x04,0x04,0xB9,0xE0,0x54,0xC1,0x54,0x61,0x54,0x40,0x54,0xA1,0x54,0x80,0x54,0x20,0x54,0x04,0x04,TTX_PAD,TTX_PAD,TTX_PAD },
{ 0xAA,0xAA,0xE4,0x40,0x31,0x04,0x04,0xC2,0xF2,0x73,0xC2,0xA2,0x83,0x32,0x04,0x5D,0x10,0x19,0x2A,0x13,0x92,0xCB,0x91,0xA1,0x04,0x04,0x04,0x04,0xB9,0xE0,0x54,0xC1,0x54,0x61,0x54,0x40,0x54,0xA1,0x54,0x80,0x54,0x20,0x54,0x04,0x04,TTX_PAD,TTX_PAD,TTX_PAD },
{ 0xAA,0xAA,0xE4,0xE3,0x31,0x10,0x19,0x2A,0xA2,0xCB,0x2A,0x04,0x0B,0x83,0xE3,0xA2,0x04,0x92,0xCB,0x04,0x04,0x04,0x91,0x80,0x04,0x04,0x04,0x04,0xB9,0xE0,0x54,0xC1,0x54,0x61,0x54,0x40,0x54,0xA1,0x54,0x80,0x54,0x20,0x54,0x04,0x04,TTX_PAD,TTX_PAD,TTX_PAD },
{ 0xAA,0xAA,0xE4,0x40,0xD9,0x10,0x19,0xE3,0xA2,0x73,0xA2,0x4A,0x83,0x2A,0xA2,0x23,0x04,0x43,0x9B,0x04,0x04,0x04,0x91,0x20,0x04,0x04,0x04,0x04,0xB9,0xE0,0x54,0xC1,0x54,0x61,0x54,0x40,0x54,0xA1,0x54,0x80,0x54,0x20,0x54,0x04,0x04,TTX_PAD,TTX_PAD,TTX_PAD },
{ 0xAA,0xAA,0xE4,0xE3,0xD9,0x10,0x19,0x0B,0x13,0x92,0x32,0x92,0x0B,0xCB,0x04,0x0B,0xB3,0xAD,0x6D,0x2C,0xAD,0x04,0x91,0x04,0x04,0x04,0x04,0x04,0x04,0xE0,0x54,0xC1,0x54,0x61,0x54,0x40,0x54,0xA1,0x54,0x80,0x54,0x20,0x54,0x04,0x04,TTX_PAD,TTX_PAD,TTX_PAD }};
#endif

/**************************************************************************** 
 * $Log: 
 *  42   mpeg      1.41        3/24/04 11:45:15 AM    Billy Jackman   CR(s) 
 *        8535 8536 8537 8650 : Simplified PES packet parsing for teletext and 
 *        removed possibility of infinite loop during parsing.
 *        Modified setting of teletext start and end lines in DRM to align with
 *         hardware.
 *  41   mpeg      1.40        3/4/04 3:44:32 PM      Billy Jackman   CR(s) 
 *        8515 8516 8517 : Changed a call to trace_new to isr_trace_new in 
 *        function ttx_line_ISR.
 *  40   mpeg      1.39        3/3/04 2:31:23 PM      Billy Jackman   CR(s) 
 *        8499 8500 8501 : Modified PES data parsing to simplify operation.  
 *        Modified teletext data queueing and output processing mechanism to 
 *        ensure that data is not lost and is delivered on time when possible. 
 *         Corrected settings for DRM and encoder VBI lines used to carry 
 *        teletext so they all match.  Modified expansion keywords for 
 *        StarTeam.
 *  39   mpeg      1.38        12/9/03 10:16:33 AM    Larry Wang      CR(s) 
 *        8115 8118 : Mask off bit 28 of PSI buffer pointer.
 *        
 *  38   mpeg      1.37        12/3/03 2:47:50 PM     Larry Wang      CR(s): 
 *        8086 8087 Allocate demux channel buffer by mem_nc_malloc().
 *        
 *  37   mpeg      1.36        10/16/03 10:12:33 AM   Xin Golden      CR(s): 
 *        5519 call the new TV encoder driver conditionally.
 *  36   mpeg      1.35        8/12/03 12:14:06 PM    Lucy C Allevato SCR(s) 
 *        7244 7245 :
 *        remove isr_trace_new from a critical section, because isr_trace_new 
 *        calls qu_send which is not critical section safe.
 *        
 *  35   mpeg      1.34        6/24/03 2:31:34 PM     Miles Bintz     SCR(s) 
 *        6822 :
 *        added initialization value to remove warning in release build
 *        
 *  34   mpeg      1.33        5/6/03 2:26:56 PM      Craig Dry       SCR(s) 
 *        5521 :
 *        Conditionally remove access to GPIO Pin Mux register 0.
 *        
 *  33   mpeg      1.32        3/10/03 6:40:58 PM     Miles Bintz     SCR(s) 
 *        5726 5727 :
 *        If OPENTV_TELETEXT_EXTENSION is defiend, then OPENTV must also be 
 *        defined... modified preprocessor directives to reflect this.
 *        
 *  32   mpeg      1.31        2/21/03 2:50:08 PM     Miles Bintz     SCR(s) 
 *        5583 :
 *        moved initialization of parse state variable to ttx_start
 *        
 *        
 *  31   mpeg      1.30        2/11/03 1:08:28 PM     Miles Bintz     SCR(s) 
 *        5372 :
 *        fixed typo
 *        
 *        
 *  30   mpeg      1.29        2/11/03 12:40:28 PM    Miles Bintz     SCR(s) 
 *        5372 :
 *        Added a workaround for OTV VTS tests to make ttx driver ignore PTS 
 *        for ttx packes
 *        
 *        
 *  29   mpeg      1.28        2/7/03 6:08:22 PM      Miles Bintz     SCR(s) 
 *        5372 :
 *        If OTV EN2 teletext extension is included, TTX20 driver should NOT do
 *         any demux operations.  This is a workaround for OTV limitations...
 *        
 *        
 *  28   mpeg      1.27        11/26/02 4:02:36 PM    Dave Wilson     SCR(s) 
 *        4902 :
 *        Changed to use label from config file to define which I2C bus device 
 *        is on.
 *        
 *  27   mpeg      1.26        9/25/02 9:47:58 PM     Carroll Vance   SCR(s) 
 *        3786 :
 *        Removing old DRM and AUD conditional bitfield code.
 *        
 *        
 *  26   mpeg      1.25        8/6/02 4:13:38 PM      Billy Jackman   SCR(s) 
 *        4149 :
 *        Make the #include of confmgr.h unconditional, since the dependency 
 *        checker
 *        does not get the same #defines as the compiler.
 *        
 *  25   mpeg      1.24        6/14/02 4:58:36 PM     Billy Jackman   SCR(s) 
 *        4026 :
 *        Put back initialization of both field addresses for both the bitfield
 *         and
 *        non-bitfield case.
 *        
 *  24   mpeg      1.23        6/12/02 11:57:34 AM    Carroll Vance   SCR(s) 
 *        3786 :
 *        Removing DRM bitfields.
 *        
 *  23   mpeg      1.22        5/21/02 1:40:20 PM     Carroll Vance   SCR(s) 
 *        3786 :
 *        Removed DRM bitfields.
 *        
 *  22   mpeg      1.21        5/13/02 1:53:04 PM     Tim White       SCR(s) 
 *        3760 :
 *        Allow code to work with both demux drivers without #ifdef's for using
 *        gDemuxInstance. 
 *        
 *        
 *  21   mpeg      1.20        5/7/02 5:08:36 PM      Tim White       SCR(s) 
 *        3721 :
 *        Remove CNXT_REG_SET & CNXT_REG_GET macro usage.
 *        
 *        
 *  20   mpeg      1.19        5/1/02 3:28:58 PM      Tim White       SCR(s) 
 *        3673 :
 *        Remove PLL_ bitfields usage from codebase.
 *        
 *        
 *  19   mpeg      1.18        4/5/02 11:42:08 AM     Tim White       SCR(s) 
 *        3510 :
 *        Backout DCS #3468
 *        
 *        
 *  18   mpeg      1.17        3/28/02 2:22:04 PM     Quillian Rutherford 
 *        SCR(s) 3468 :
 *        Removed bit fields
 *        
 *        
 *  17   mpeg      1.16        3/26/02 9:52:10 AM     Billy Jackman   SCR(s) 
 *        3446 3429 :
 *        Added parentheses to avoid compiler warnings.
 *        
 *  16   mpeg      1.15        3/22/02 10:58:32 AM    Billy Jackman   SCR(s) 
 *        3429 3430 :
 *        Removed the code that was using GetChipRev() to determine whether to 
 *        program
 *        GPIO pins for use with teletext.  This is now done based on whether 
 *        the video
 *        encoder is external or internal.  This also gets rid of a compiler 
 *        warning
 *        about GetChipRev.
 *        
 *  15   mpeg      1.14        3/22/02 10:52:34 AM    Billy Jackman   SCR(s) 
 *        3429 :
 *        #include <string.h> to get memset defined.
 *        Remove or provide the proper #ifdef context for unused variables.
 *        
 *  14   mpeg      1.13        3/18/02 3:03:58 PM     Billy Jackman   SCR(s) 
 *        3387 :
 *        Make sure that teletext does not try to function with NTSC video.
 *        
 *  13   mpeg      1.12        3/11/02 11:05:56 AM    Ian Mitchell    SCR(s): 
 *        3342 
 *        used new definitions for CTBT_TASK task settings
 *        
 *  12   mpeg      1.11        3/5/02 9:51:44 AM      Billy Jackman   SCR(s) 
 *        3237 :
 *        Fixed compile error for vxworks.
 *        
 *  11   mpeg      1.10        3/4/02 1:14:44 PM      Billy Jackman   SCR(s) 
 *        3237 :
 *        Added pts, field, and line parameters to ttx_do_insert.
 *        Set the number of transport packets needed to raise a data ready 
 *        interrupt to
 *        one so that teletext driver gets all information in a timely manner.
 *        Changed the sense of TTX_REQ_IS_CLOCK test at end of 
 *        ttx_encoder_init.
 *        Check the return status from qu_send in ttx_DataCB in case of error.
 *        Added some trace code, particularly to dump raw PES data.
 *        Changed call to ...read_pes_buffer to only request as much data as is
 *         known
 *        to be ready.
 *        Corrected an error in extraction of PTS from packet.
 *        Modify the ttx_line_ISR routine to send data at the requested time, 
 *        on the
 *        requested field.
 *        
 *  10   mpeg      1.9         2/25/02 8:07:06 PM     Billy Jackman   SCR(s) 
 *        3237 :
 *        Put back in the GENDMXC/DEMUX capability that was lost in the 
 *        previous put.
 *        
 *  9    mpeg      1.8         2/22/02 2:46:40 PM     Billy Jackman   SCR(s) 
 *        3237 :
 *        Modified to handle timing by queueing all packets and playing them 
 *        according
 *        to STC.
 *        
 *  8    mpeg      1.7         2/6/02 6:00:16 PM      Bob Van Gulick  SCR(s) 
 *        3143 :
 *        Conditionally compile in DEMUX or GENDMXC
 *        
 *        
 *  7    mpeg      1.6         2/4/02 1:42:10 PM      Miles Bintz     SCR(s) 
 *        3123 :
 *        Added code to pay attention to tick_count and PTSs to throttle 
 *        outgoing ttx data.
 *        
 *        
 *  6    mpeg      1.5         1/28/02 4:58:12 PM     Miles Bintz     SCR(s) 
 *        3093 :
 *        Removed a type cast for a function pointer on line 981 to fix an 
 *        error regarding the void return type.
 *        
 *  5    mpeg      1.4         1/28/02 4:10:32 PM     Miles Bintz     SCR(s) 
 *        2592 :
 *        Code rot: updated ttx20 to use updated demux code
 *        
 *        
 *  4    mpeg      1.3         1/21/02 5:50:16 PM     Dave Wilson     SCR(s) 
 *        3071 :
 *        Removed references to FCopyBytes. This function has disappeared and 
 *        been
 *        replaced by FCopy.
 *        
 *  3    mpeg      1.2         1/21/02 4:11:32 PM     Bob Van Gulick  SCR(s) 
 *        3071 :
 *        ttx20.c did not build with the per slot psi demux change.  Now fixed.
 *        
 *        
 *  2    mpeg      1.1         8/21/01 12:26:44 PM    Miles Bintz     SCR(s) 
 *        2519 :
 *        added code to check the framing code to see if bitswapping is 
 *        necessary
 *        
 *  1    mpeg      1.0         8/13/01 12:11:42 PM    Miles Bintz     
 * $
 ****************************************************************************/

