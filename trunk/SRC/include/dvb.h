/****************************************************************************/
/*                   CONEXANT PROPRIETARY AND CONFIDENTIAL                  */
/*                       SOFTWARE FILE/MODULE HEADER                        */
/*           Conexant Systems Inc. (c) 1998, 1999, 2000, 2001, 2002         */
/*                               Austin, TX                                 */
/*                            All Rights Reserved                           */
/****************************************************************************/
/*
 * Filename:       dvb.h
 *
 * Description:    DVB public header
 *
 * Author:         Angela Swartz copied from watchtv/root.c        
 *
 ****************************************************************************/
/* $Header: dvb.h, 10, 3/10/04 2:55:53 PM, Larry Wang$
 ****************************************************************************/
#include "basetype.h"

/* SDT section defintions */
#define SIZEOF_SERVICEINFO     20
#define SDT_ACTUAL_TABLEID     0x42
#define SDT_OTHER_TABLEID      0x46
#define SDT_TABLEID_FILTER     0xFB  /* To catch 0x42 or 0x46 */
#define SDT_TIMEOUT_VALUE      0     /* Timeout in Milliseconds */
/**********************************************/
/* Resource limit and buffer size definitions */
/**********************************************/
#define MAX_PMT_ENTRIES         50
#define MAX_PID_ENTRIES          36
#define MAX_CA_DESC             20

typedef struct _PATEntry
{
    u_int32 PMTChannel;
    void   *PMTChannelBuffer;
    u_int32 PMTfid;
    u_int16 ProgramNumber;
    u_int16 ProgramMapPID;
    char    ServiceProvName[SIZEOF_SERVICEINFO];
    char    ServiceName[SIZEOF_SERVICEINFO];
    u_int32 NumVideoPIDs;
    u_int16 VideoPIDs[MAX_PID_ENTRIES];
    u_int32 NumAudioPIDs;
    u_int16 AudioPIDs[MAX_PID_ENTRIES];
    u_int8  AudioTypes[MAX_PID_ENTRIES];
    u_int16 PCRPID;
    u_int32 NumDataPIDs;
    u_int16 DataPIDs[MAX_PID_ENTRIES];
    int8  Version;
#if PARSER_PASSAGE_ENABLE==YES
    u_int16 ShadowVideoPid;
    u_int16 ShadowAudioPid;
#endif
} PATEntry;

typedef enum _DVB_CTRL_MSG_ID
{
  DVB_CTRL_GOT_PAT,
  DVB_CTRL_GOT_PMT,
  DVB_CTRL_GOT_SDT,
  DVB_CTRL_PAT_SECTION,  /* Used in conjunction with cnxt_dvb_retrieve_pat     */
  DVB_CTRL_PMT_SECTION,  /* Used in conjunction with cnxt_dvb_retrieve_pmt     */
  DVB_CTRL_CAT_SECTION,  /* Used in conjunction with cnxt_dvb_retrieve_cat     */
  DVB_CTRL_OTHER_SECTION,/* Used in conjunction with cnxt_dvb_retrieve_section */ 
  DVB_CTRL_TIMEOUT,
  DVB_CTRL_ERROR
} DVB_CTRL_MSG_ID;

typedef bool (*DVB_PFNNOTIFY) (u_int32, u_int32, u_int32, u_int32);

void DumpPATEntries(void);
void GetPMTs(void);
void GetSDT(void);
void GetPAT(void);
void cnxt_dvb_init(DVB_PFNNOTIFY pDVBCallBack );
bool cnxt_dvb_set_program_number(int ProgramNumber);
bool cnxt_dvb_find_next_program(int *pNextProgramNumber);
bool cnxt_dvb_find_prev_program(int *pPrevProgramNumber);
bool cnxt_dvb_get_next_or_prev_audio(bool bNext, u_int16* puCurrentAudioPID);
bool cnxt_dvb_get_next_or_prev_video(bool bNext, u_int16* puCurrentVideoPID, u_int16* puCurrentAudioPID, 
                                     u_int16* puCurrentPCRPID, u_int16* puCurrentDataPID);
bool cnxt_dvb_get_next_or_prev_data(bool bNext, u_int16* puCurrentVideoPID, u_int16* puCurrentAudioPID,
                                    u_int16* puCurrentPCRPID, u_int16* puCurrentDataPID);
void cnxt_dvb_select_first_program();
void cnxt_dvb_set_currentTSID(u_int32 uCurrentTSID);
void cnxt_dvb_get_current_program_pat_entry(PATEntry *pCurrentEntry, u_int32 *puCurrentVideo, u_int32 *puCurrentAudio, u_int32 *puCurrentData );
u_int32 cnxt_dvb_get_num_pat_entries();
u_int32 cnxt_dvb_get_current_program();
u_int32 cnxt_dvb_get_program_number(u_int32 uProgram);

/***************************************************************************/
/* General Purpose DVB-SI functions and structures not specific to WatchTV */
/***************************************************************************/
#define DVB_DONT_CARE (u_int16)(0xFFFF)

/* Note: These structures are overlaid on the byte stream so MUST BE PACKED */

/* Basic header found on all standard DVB SI tables */
PACKED_STRUCT
{
  PACKED_MEMBER(u_int8,  cTableID)
  PACKED_MEMBER(u_int16, uLenSyntax)
  PACKED_MEMBER(u_int16, uID)
  PACKED_MEMBER(u_int8,  cVerCurNext)
  PACKED_MEMBER(u_int8,  cSection)
  PACKED_MEMBER(u_int8,  cLastSection)
} PACKED_STRUCT_NAME(DVBSECTIONHEADER)

typedef DVBSECTIONHEADER *PDVBSECTIONHEADER;

PACKED_STRUCT
{
  PACKED_MEMBER(u_int16, uProgram)
  PACKED_MEMBER(u_int16, uPID)
} PACKED_STRUCT_NAME(DVBPATPROGRAM) 

typedef DVBPATPROGRAM *PDVBPATPROGRAM;

PACKED_STRUCT
{
  PACKED_MEMBER(u_int8,  cTag)
  PACKED_MEMBER(u_int8,  cLength)
  PACKED_MEMBER(u_int16, uCASystemID)
  PACKED_MEMBER(u_int16, uCAPID)
} PACKED_STRUCT_NAME(DVBCADESCRIPTORHEADER)

typedef DVBCADESCRIPTORHEADER *PDVBCADESCRIPTORHEADER;

#define DVB_CA_DESCRIPTOR_TAG 0x09

#define DVB_SECTION_CRC_SIZE 4
#define DVB_HEADER_SIZE_AFTER_LEN 5

/* Macros used to access various fields in DVBSECTIONHEADER. In each case,    */
/* the parameter will be the value of the u_int8 or u_int16 field containing  */
/* the item of interest for example DVBSECTIONHEADER_SYNTAX(pHdr->uLenSyntax) */
#define DVBSECTIONHEADER_TABLEID(x)     (x)
#define DVBSECTIONHEADER_SYNTAX(x)      ((x) & 0x80) >> 7)
#define DVBSECTIONHEADER_LENGTH(x)      ((((x) & 0xFF00) >> 8) | (((x) & 0x000F) << 8))
#define DVBSECTIONHEADER_ID(x)          ((((x) & 0xFF) << 8) | (((x) & 0xFF00) >> 8))
#define DVBSECTIONHEADER_CURNEXT(x)     ((x) & 0x01)
#define DVBSECTIONHEADER_VERSION(x)     (((x) & 0x3E) >> 1)
#define DVBSECTIONHEADER_SECTION(x)     (x)
#define DVBSECTIONHEADER_LASTSECTION(x) (x)

#define DVBSECTIONHEADER_SWAPWORD16(x)  ((((x) & 0xFF00) >> 8) | (((x) & 0x00FF) << 8))

#define DVBPATPROGRAM_SERVICE(x)        ((((x) & 0xFF00) >> 8) | (((x) & 0x00FF) << 8))
#define DVBPATPROGRAM_PID(x)            ((((x) & 0xFF00) >> 8) | (((x) & 0x001F) << 8))

#define DVB_READSWAPPEDWORD16(ptr)       (((u_int16)*(ptr) << 8) | (u_int16)*((u_int8 *)(ptr) + 1))

/* User-friendly structures containing info parsed from PAT, PMT and CAT headers.      */ 
/* Element packing is as per the compiler option defaults since these are not overlaid */
/* on any byte stream.                                                                 */
typedef struct _DVBPATINFO
{
  u_int8 cVersion;
  u_int8 cSection;
  u_int8 cLastSection;
  u_int16 uTSID;
  u_int16 uNumServices;
} DVBPATINFO, *PDVBPATINFO;

typedef struct _DVBPMTINFO
{
  u_int8 cVersion;
  u_int8 cSection;
  u_int8 cLastSection;
  u_int16 uServiceID;
  u_int16 uPCRPID;
  u_int16 uNumStreams;
} DVBPMTINFO, *PDVBPMTINFO;

typedef struct _DVBCATINFO
{
  u_int8 cVersion;
  u_int8 cSection;
  u_int8 cLastSection;
  u_int8 *pDescriptors;
  u_int16 uLenDescriptors;
} DVBCATINFO, *PDVBCATINFO;

typedef u_int8 *PDVBPAT;
typedef u_int8 *PDVBPMT;
typedef u_int8 *PDVBCAT;

typedef void *DVBREQUESTHANDLE;

/* PAT service parsing callback */
typedef bool (*DVB_PFNPARSEPATENTRY)(int iIndex, u_int16 uServiceID, u_int16 uPMTPID, u_int32 uTag);

/* PMT stream parsing callback */
typedef bool (*DVB_PFNPARSEPMTENTRY)(int iIndex, u_int8 uStreamType, u_int16 uPID, u_int8 *pESDesc, u_int16 uLenESDesc, u_int32 uTag);

/* Retrieve table sections asynchronously */
DVBREQUESTHANDLE cnxt_dvb_retrieve_pat(int iDemux, u_int16 uVersionNumber, u_int16 uSectionNum, u_int16 uCurrentNext, u_int32 uTag, bool bContinuous);
DVBREQUESTHANDLE cnxt_dvb_retrieve_pmt(int iDemux, u_int16 uPMTPID, u_int16 uVersionNumber, u_int16 uServiceID, u_int16 uCurrentNext, u_int32 uTag, bool bContinuous);
DVBREQUESTHANDLE cnxt_dvb_retrieve_cat(int iDemux, u_int16 uVersionNumber, u_int16 uSectionNumber, u_int16 uCurrentNext, u_int32 uTag, bool bContinuous);
DVBREQUESTHANDLE cnxt_dvb_retrieve_section(int iDemux, u_int16 uPID, u_int16 uTableID, u_int16 uTableIDMask, u_int16 uVersionNum, u_int16 uSectionNum, u_int16 uID, u_int16 uCurrentNext, u_int32 uBufferSize, u_int32 uTag, bool bContinuous);

/* Free up a section handle and release all associated resources */
bool cnxt_dvb_cancel_request(DVBREQUESTHANDLE hRequest);

/* Get table header information */
bool cnxt_dvb_get_pat_info(PDVBPAT pPAT, PDVBPATINFO pPatInfo);
bool cnxt_dvb_get_pmt_info(PDVBPAT pPMT, PDVBPMTINFO pPmtInfo);
bool cnxt_dvb_get_cat_info(PDVBCAT pCAT, PDVBCATINFO pCatInfo);

/* Parse service and stream information */
bool cnxt_dvb_parse_pat(PDVBPAT pPAT, DVB_PFNPARSEPATENTRY pfnCallback, u_int32 uTag);
bool cnxt_dvb_parse_pmt(PDVBPMT pPMT, DVB_PFNPARSEPMTENTRY pfnCallback, u_int32 uTag);


/****************************************************************************
 * Modifications:
 * $Log: 
 *  10   mpeg      1.9         3/10/04 2:55:53 PM     Larry Wang      CR(s) 
 *        8551 : Add members to PATEntry to hold shadow PIDs.
 *  9    mpeg      1.8         12/8/03 3:43:50 PM     Dave Wilson     CR(s) 
 *        8117 : Replaced multiple definitions of structures with different 
 *        toolchain packing syntax with new versions using macros from 
 *        toolchain.h
 *        
 *  8    mpeg      1.7         8/29/03 12:08:04 PM    Miles Bintz     SCR(s) 
 *        7291 :
 *        back out previous change since ARM_TOOLKIT isn't defined at the 
 *        source code level
 *        
 *  7    mpeg      1.6         8/27/03 6:28:38 PM     Miles Bintz     SCR(s) 
 *        7291 :
 *        definition of a structure and its packing paramters is dependant on 
 *        toolkit not RTOS
 *        
 *  6    mpeg      1.5         6/19/03 5:31:40 PM     Larry Wang      SCR(s) 
 *        6811 :
 *        Change the definitions of structures DVBSECTIONHEADER, DVBPATPROGRAM 
 *        and DVBCADESCRIPTORHEADER for vxworks to make it build and run under 
 *        vxworks environment.
 *        
 *  5    mpeg      1.4         6/3/03 10:17:22 AM     Larry Wang      SCR(s) 
 *        6667 :
 *        Add AudioTypes to PATEntry structure so that we can identify what 
 *        kind of audio it is for an audio PID.
 *        
 *  4    mpeg      1.3         4/17/03 11:54:52 AM    Dave Wilson     SCR(s) 
 *        6051 :
 *        
 *        
 *        
 *        
 *        
 *        
 *        
 *        Added a demux instance parameter to all calls that will end up 
 *        calling into the
 *        demux driver. This allows the new interface to be used on multi-demux
 *         systems
 *        to capture PAT, PMT, CAT, etc from any of the installed demuxes.
 *        
 *  3    mpeg      1.2         4/17/03 10:37:22 AM    Dave Wilson     SCR(s) 
 *        6024 :
 *        Added CA descriptor types.
 *        
 *        
 *  2    mpeg      1.1         4/11/03 6:02:44 PM     Dave Wilson     SCR(s) 
 *        5979 :
 *        Added typedefs and prototypes for general-purpose DVB object request 
 *        and
 *        parsing interface.
 *        
 *  1    mpeg      1.0         3/18/03 4:54:02 PM     Angela Swartz   
 * $
 * 
 *    Rev 1.7   29 Aug 2003 11:08:04   bintzmf
 * SCR(s) 7291 :
 * back out previous change since ARM_TOOLKIT isn't defined at the source code level
 * 
 *    Rev 1.5   19 Jun 2003 16:31:40   wangl2
 * SCR(s) 6811 :
 * Change the definitions of structures DVBSECTIONHEADER, DVBPATPROGRAM and DVBCADESCRIPTORHEADER for vxworks to make it build and run under vxworks environment.
 * 
 *    Rev 1.4   03 Jun 2003 09:17:22   wangl2
 * SCR(s) 6667 :
 * Add AudioTypes to PATEntry structure so that we can identify what kind of audio it is for an audio PID.
 * 
 *    Rev 1.3   17 Apr 2003 10:54:52   dawilson
 * SCR(s) 6051 :
 * 
 * 
 * 
 * 
 * 
 * 
 * 
 * Added a demux instance parameter to all calls that will end up calling into the
 * demux driver. This allows the new interface to be used on multi-demux systems
 * to capture PAT, PMT, CAT, etc from any of the installed demuxes.
 * 
 *    Rev 1.2   17 Apr 2003 09:37:22   dawilson
 * SCR(s) 6024 :
 * Added CA descriptor types.
 * 
 * 
 *    Rev 1.1   11 Apr 2003 17:02:44   dawilson
 * SCR(s) 5979 :
 * Added typedefs and prototypes for general-purpose DVB object request and
 * parsing interface.
 * 
 *    Rev 1.0   18 Mar 2003 16:54:02   swartzwg
 * SCR(s) 5804 :
 * DVB public header file
 * 
****************************************************************************/


