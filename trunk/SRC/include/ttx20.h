/******************************************************************************/
/*                   CONEXANT PROPRIETARY AND CONFIDENTIAL                    */
/*                        SOFTWARE FILE/MODULE HEADER                         */
/*                 Copyright Conexant Systems Inc. 1998-2004                  */
/*                                 Austin, TX                                 */
/*                            All Rights Reserved                             */
/******************************************************************************/
/*
 * Filename:        ttx20.h
 *
 *
 * Description:     Header file for TTX Driver version 2.0
 *
 *
 * Author:          Miles Bintz, Billy Jackman
 *
 ******************************************************************************/
/* $Id: ttx20.h,v 1.7, 2004-03-24 17:47:15Z, Billy Jackman$
 ******************************************************************************/

#define TTX_MAX_CLIENTS                  2
#define TTX_DMX_BUFFER_SIZE              4096
#define   TTX_DEBUG                      0
#define   TTX_SCRAMBLING                 1
/* The TV encoder can operate in two modes,
   one where the ttx_req signal goes high
   for the entire VBI line, and another where
   the req "clocks" out the data one bit at a time */ 
#define   TTX_REQ_IS_CLOCK               1
#define   TTX_CACHE_MASK                 0x1f  
#define   TTX_CACHE_PAD                  0x20
#define   TTX_LINES_PER_FIELD            16
#define   TTX_FIRST_LINE                 7
#define   TTX_DRM_LINE_SIZE              0x30
#define   TTX_NUM_BUFFERED_LINES         (TTX_LINES_PER_FIELD * 8)
#define   NEXT_LINE(L) (L+1>=TTX_NUM_BUFFERED_LINES ? 0 : L+1)

/* Setting this to a one uses a 26 lines of static TTX 
   data to send to the TV encoder */
#define   TTX_DEBUG_STATIC_BUFFER        0

/* This routine dumps data that is processed in the ttx_do_insert
   function */
#define   TTX_DUMP_INSERT_DATA           0

/* This routine dumps the data that is processed by the VBI line ISR
   Whatever you see as output from the DUMP_ISR_DATA *should* be what
   is also sent to the TV encoder */
#define   TTX_DUMP_ISR_DATA              0

/* This define controls tracing of the raw PES data */
#define   TTX_DUMP_PES_DATA              0


typedef enum {CNXT_TTX_INSERT = 0, CNXT_TTX_EXTRACT} cnxt_ttx_Type;
typedef enum {CNXT_TTX_SUCCESS = 0, CNXT_TTX_FAILURE, CNXT_TTX_DMX_ERROR, CNXT_TTX_PARAM_ERROR} cnxt_ttx_status;
typedef enum {CNXT_TTX_UNALLOCATED = 0, CNXT_TTX_CLOSED, CNXT_TTX_OPEN, CNXT_TTX_PLAYING} cnxt_ttx_state;

typedef void (*cnxt_ttx_pfncallback)(int, int, char *);

typedef struct {
   cnxt_ttx_state         state;                 /* indicates whether this teletext instance is currently in use */
   void                   *dmxbuffer;            /* pointer to buffer given to demux */
   int                    dmxchid;               /* channel ID of demux resource used for this teletext instance */
   int                    pid;                   /* PID for this teletext instance */
   char                   *ttxbuf;               /* buffer which will contain raw PES teletext data */
   int                    ttxbufsize;            /* size of above buffer */
   int                    mag, page, subcode;    /* filter for teletext extraction */
   int                    acquiring;
   int                    pid_link;              /* when two clients request the same pid, only one is registered with
                                                    demux.  Thus, when it makes one callback, we must make multiple
                                                    callbacks to distribute the data */
   cnxt_ttx_pfncallback   fncb;                  /* pointer to callback function */
   cnxt_ttx_Type          typ;                   /* insertion/extraction ? */
} sttx_ClientData;

typedef struct {
   int     field;
   int     vbi_line;
   long long pts;
   unsigned char data[TTX_DRM_LINE_SIZE];
} sttxbuf;


typedef union {
   struct {
      unsigned int  s1:4,
                    s2:3,
                    s3:4,
                    s4:2;
   } sc;
   int word;
} uSubcode;

cnxt_ttx_status cnxt_ttx_Init();

/*
The alternate form of the function is:
cnxt_ttx_status cnxt_ttx_Create_Instance(cnxt_ttx_Type type, int *hTTX, void *pBuf, int buf_size, cnxt_ttx_pfncallback pCallback);
*/
cnxt_ttx_status cnxt_ttx_Create_Instance(cnxt_ttx_Type typ, int *hTTX, ...);

cnxt_ttx_status cnxt_ttx_Release_Instance(int hTTX);

cnxt_ttx_status cnxt_ttx_Open(int hTTX, int pid);

cnxt_ttx_status cnxt_ttx_Close(int hTTX);

cnxt_ttx_status cnxt_ttx_Start(int hTTX);

cnxt_ttx_status cnxt_ttx_Stop(int hTTX);

cnxt_ttx_status cnxt_ttx_Set_Focus(int hTTX, int mag, int page, int subcode);

typedef enum {
   tpsTTXStreamID,
   tpsTTXDataID
} ttx_PES_Parse_State;


/*******************************************************************************
 * Modifications:
 * $Log: 
 *  8    mpeg      1.7         3/24/04 11:47:15 AM    Billy Jackman   CR(s) 
 *        8535 8536 8537 8650 : Simplified PES packet parsing.
 *        Modified Log: keyword to enable expansion.
 *  7    mpeg      1.6         3/3/04 2:25:45 PM      Billy Jackman   CR(s) 
 *        8499 8500 8501 : Add a state used for PES packet parsing.  Remove 
 *        some union definitions containing bitfields.  Change keyword 
 *        expansion fields for StarTeam.
 *  6    mpeg      1.5         3/26/02 9:49:30 AM     Billy Jackman   SCR(s) 
 *        3446 3429 :
 *        Changed field "word" in uSubcode union from unsigned to signed to 
 *        avoid a
 *        compiler warning.
 *        
 *  5    mpeg      1.4         3/18/02 3:17:50 PM     Billy Jackman   SCR(s) 
 *        3387 :
 *        Changed mode of teletext transfer from enable to clocked.  This mode 
 *        seems
 *        to work better.
 *        
 *  4    mpeg      1.3         3/4/02 1:06:54 PM      Billy Jackman   SCR(s) 
 *        3237 :
 *        Added new trace control define for tracing raw PES data.  Removed 
 *        unused 
 *        define.
 *        
 *  3    mpeg      1.2         2/22/02 2:41:26 PM     Billy Jackman   SCR(s) 
 *        3237 :
 *        Fixes for teletext output timing.
 *        
 *  2    mpeg      1.1         2/4/02 1:44:58 PM      Miles Bintz     SCR(s) 
 *        3123 :
 *        Added some comments and pre-processor defines to aid in debugging.
 *        
 *        
 *  1    mpeg      1.0         1/21/02 3:13:08 PM     Dave Wilson     
 * $
 *
 ******************************************************************************/

