/****************************************************************************/
/*                   CONEXANT PROPRIETARY AND CONFIDENTIAL                  */
/*                       SOFTWARE FILE/MODULE HEADER                        */
/*                   Conexant Systems Inc. (c) 1998 - 2003                  */
/*                               Austin, TX                                 */
/*                            All Rights Reserved                           */
/****************************************************************************/
/*
 * Filename:       dvb_gen.c
 *
 *
 * Description:    Generic DVB section access and parsing functions
 *
 *
 * Author:         Dave Wilson
 *
 ****************************************************************************/
/* $Header: dvb_gen.c, 7, 12/9/03 10:13:43 AM, Larry Wang$
 ****************************************************************************/

/*****************/
/* Include Files */
/*****************/
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

/******************************************/
/* Local Definitions and Global Variables */
/******************************************/
typedef enum _DVBREQUESTTYPE
{
  DVBREQUEST_PAT,
  DVBREQUEST_PMT,
  DVBREQUEST_CAT,
  DVBREQUEST_Other
} DVBREQUESTTYPE;

#define NUM_FILTER_BYTES 8
#define DVB_REQUEST_MAGIC_NUMBER 0xFAB0ABBA

/* State structure used to track each table request */
typedef struct _DVBREQUEST
{
  DVBREQUESTTYPE eType;
  bool           bContinuous;
  u_int32        uMagic;
  u_int32        uChannel;
  u_int32        uFilter;
  u_int8         cFilterMatch[NUM_FILTER_BYTES];
  u_int8         cFilterMask[NUM_FILTER_BYTES];
  void *         pBuffer;
  u_int32        uBufferSize;
  u_int32        uTag;
  u_int16        uVersion;
  u_int16        uSection;
  u_int16        uCurrentNext;
  int            iDemuxInstance;
} DVBREQUEST, *PDVBREQUEST;

/********************************/       
/* Internal Function Prototypes */
/********************************/

static genfilter_mode dvb_request_header_notify(pgen_notify_t notify_data);
static void dvb_request_section_notify(pgen_notify_t notify_data);
static DVBREQUESTHANDLE cnxt_dvb_retrieve_section_internal(int gDemux, DVBREQUESTTYPE eType, u_int16 uPID, u_int16 uTableID, u_int16 uTableIDMask, u_int16 uVersionNum, u_int16 uSectionNum, u_int16 uID, u_int16 uCurrentNext, u_int32 uBufferSize, u_int32 uTag, bool bContinuous);

/********************************************************************/
/*  FUNCTION:    cnxt_dvb_retrieve_section_internal                 */
/*                                                                  */
/*  PARAMETERS:  eType          - Type of request being made        */
/*               uPID           - PID to search for section         */
/*               uTableID       - Table ID to search for or         */
/*                                DVB_DONT_CARE to get any.         */
/*               uTableIDMask   - Mask to associate with TableID    */
/*                                if TableID is not DVB_DONT_CARE.  */
/*               uVersionNum    - Version number to search for      */
/*                                DVB_DONT_CARE to get any.         */
/*               uSectionNum    - Section number to search for or   */
/*                                DVB_DONT_CARE to get any.         */
/*               uID            - ID to search for or DVB_DONT_CARE */
/*                                to get any.                       */
/*               uCurrentNext   - 1 to get current table, 0 to get  */
/*                                next table, DVB_DONT_CARE to get  */
/*                                either                            */
/*               uBufferSize    - Size of buffer to use for request */
/*               uTag           - Tag which will be passed back to  */
/*                                the DVB callback with the data    */
/*               bContinuous    - TRUE to capture sections          */
/*                                continuously, FALSE to capture    */
/*                                once then stop.                   */
/*                                                                  */
/*  DESCRIPTION: Retrieve an arbitrary table from the stream. Data  */
/*               will be returned asynchronously on the DVB         */
/*               callback.                                          */
/*                                                                  */
/*  RETURNS:     A valid request handle if successful, else NULL.   */
/*                                                                  */
/*  CONTEXT:     Must be called in task context                     */
/*                                                                  */
/********************************************************************/
static DVBREQUESTHANDLE cnxt_dvb_retrieve_section_internal(int gDemux, DVBREQUESTTYPE eType, u_int16 uPID, u_int16 uTableID, u_int16 uTableIDMask, u_int16 uVersionNum, u_int16 uSectionNum, u_int16 uID, u_int16 uCurrentNext, u_int32 uBufferSize, u_int32 uTag, bool bContinuous)
{
  PDVBREQUEST pRequest;
  int         iLoop;

  /* Allocate a request structure */
  pRequest = (PDVBREQUEST)mem_malloc(sizeof(DVBREQUEST));
  if(pRequest)
  {  
    /* Allocate a channel buffer */
    pRequest->pBuffer = (void *)( (u_int32)mem_nc_malloc(uBufferSize) & ~NCR_BASE );
    if(!pRequest->pBuffer)
    {
      trace_new(TL_ERROR, TRACE_FG_LIGHT_RED  "DVB: Unable to allocate channel buffer\n" TRACE_FG_NORMAL);
      mem_free(pRequest);
      return((DVBREQUESTHANDLE)NULL);
    }
    
    /* We got all the buffers we need so fill in some values now */
    pRequest->uMagic         = DVB_REQUEST_MAGIC_NUMBER;
    pRequest->eType          = eType;
    pRequest->uBufferSize    = uBufferSize;
    pRequest->uTag           = uTag;
    pRequest->uVersion       = uVersionNum;
    pRequest->uSection       = uSectionNum;
    pRequest->uCurrentNext   = uCurrentNext;
    pRequest->bContinuous    = bContinuous;
    pRequest->iDemuxInstance = gDemux;
    
    /* Figure out what our filter and mask should look like */
    for(iLoop = 0; iLoop < NUM_FILTER_BYTES; iLoop++)
    {
      pRequest->cFilterMatch[iLoop] = (u_int8)0;    
      pRequest->cFilterMask[iLoop] = (u_int8)0;    
    }  
  
    /* Table ID */
    if(uTableID != DVB_DONT_CARE)
    {
      pRequest->cFilterMask[0]  = (u_int8)uTableIDMask;
      pRequest->cFilterMatch[0] = (u_int8)uTableID;
    }  
  
    /* ID (meaning varies with table) */
    if(uID != DVB_DONT_CARE)
    {
      pRequest->cFilterMask[3]  = 0xFF;
      pRequest->cFilterMask[4]  = 0xFF;
      pRequest->cFilterMatch[3] = (u_int8)((uID & 0xFF00) >> 8);
      pRequest->cFilterMatch[4] = (u_int8)(uID & 0xFF);
    }
    
    /* Version number */
    if(uVersionNum != DVB_DONT_CARE)
    {
      pRequest->cFilterMask[5] |= 0x3E;
      pRequest->cFilterMatch[5] |= ((uVersionNum & 0x1F) << 1);
    }

    /* Current/next indicator */
    if(uCurrentNext != DVB_DONT_CARE)
    {
      pRequest->cFilterMask[5] |= 0x01;
      pRequest->cFilterMatch[5] |= (uCurrentNext & 0x01);
    }  
    
    /* Section Number */
    if(uSectionNum != DVB_DONT_CARE)
    {
      pRequest->cFilterMask[6]  = 0xFF;
      pRequest->cFilterMatch[6] = (u_int8)(uSectionNum & 0xFF);
    }

    /* Open a demux channel for the request */  
    cnxt_dmx_channel_open(gDemux, 0, PSI_CHANNEL_TYPE,
                          &(pRequest->uChannel));

    /* Register PAT channel notification */
    cnxt_dmx_set_section_channel_attributes(gDemux, pRequest->uChannel,
                                            (gen_callback_fct_t) dvb_request_header_notify,
                                            (gen_callback_fct_t) dvb_request_section_notify, 
                                            0, 
                                            sizeof(DVBSECTIONHEADER));
                                            
    cnxt_dmx_set_section_channel_tag(gDemux, pRequest->uChannel, (u_int32)pRequest);                                       

    /* Set channel buffer */
    cnxt_dmx_set_channel_buffer(gDemux, pRequest->uChannel, pRequest->pBuffer, pRequest->uBufferSize);

    cnxt_dmx_filter_open(gDemux, pRequest->uChannel, NUM_FILTER_BYTES, &(pRequest->uFilter));

    /* setup a filter for table id =0x01, section number=0x00 */
    if (cnxt_dmx_set_filter(gDemux, 
                            pRequest->uChannel, 
                            pRequest->uFilter, 
                            pRequest->cFilterMatch,
                            pRequest->cFilterMask, 
                            (u_int8 *)NULL) != DMX_STATUS_OK)
    {
        trace_new(TL_ERROR, TRACE_FG_LIGHT_RED  "DVB: cnxt_dmx_set_filter Failed\n" TRACE_FG_NORMAL);
        mem_nc_free(pRequest->pBuffer);
        mem_free(pRequest);
        return((DVBREQUESTHANDLE)NULL);
    }
    
    if (cnxt_dmx_filter_control(gDemux, 
                                pRequest->uChannel, 
                                pRequest->uFilter,
                                (gencontrol_channel_t) GEN_DEMUX_ENABLE) != DMX_STATUS_OK)
    {
        trace_new(TL_ERROR, TRACE_FG_LIGHT_RED  "DVB: cnxt_dmx_filter_control Failed\n" TRACE_FG_NORMAL);
        cnxt_dvb_cancel_request((DVBREQUESTHANDLE)pRequest);
        return((DVBREQUESTHANDLE)NULL);
    }

    /* Set the PID and enable the channel */
    if(cnxt_dmx_channel_set_pid(gDemux, pRequest->uChannel, (u_int16)uPID) != DMX_STATUS_OK)
    {
        trace_new(TL_ERROR, TRACE_FG_LIGHT_RED  "DVB: cnxt_dmx_channel_set_pid Failed\n" TRACE_FG_NORMAL);
        cnxt_dvb_cancel_request((DVBREQUESTHANDLE)pRequest);
        return((DVBREQUESTHANDLE)NULL);
    }
    
    if(cnxt_dmx_channel_control(gDemux, pRequest->uChannel, (gencontrol_channel_t) GEN_DEMUX_ENABLE) != DMX_STATUS_OK)
    {
        trace_new(TL_ERROR, TRACE_FG_LIGHT_RED  "DVB: cnxt_dmx_channel_control Failed\n" TRACE_FG_NORMAL);
        cnxt_dvb_cancel_request((DVBREQUESTHANDLE)pRequest);
        return((DVBREQUESTHANDLE)NULL);
    }
  }
  else
  {
    trace_new(TL_ERROR, TRACE_FG_LIGHT_RED  "DVB: Unable to allocate DVB request structure\n" TRACE_FG_NORMAL); 
  }  
  
  return((DVBREQUESTHANDLE)pRequest);
}

/********************************************************************/
/*  FUNCTION:    cnxt_dvb_retrieve_pat                              */
/*                                                                  */
/*  PARAMETERS:  uVersionNum    - Required version number or        */
/*                                DVB_DONT_CARE for any version     */
/*               uSectionNum    - Required section number or        */
/*                                DVB_DONT_CARE for any section     */
/*               uCurrentNext   - 1 to get current table, 0 to get  */
/*                                next table, DVB_DONT_CARE to get  */
/*                                either                            */
/*               uTag           - Tag which will be passed back to  */
/*                                the DVB callback with the data    */
/*                                                                  */
/*  DESCRIPTION: Retrieve the PAT from the current stream. Data     */
/*               will be returned asynchronously on the DVB         */
/*               callback.                                          */
/*                                                                  */
/*  RETURNS:     A valid request handle if successful, else NULL.   */
/*                                                                  */
/*  CONTEXT:     Must be called in task context                     */
/*                                                                  */
/********************************************************************/
DVBREQUESTHANDLE cnxt_dvb_retrieve_pat(int gDemux, u_int16 uVersionNum, u_int16 uSectionNum, u_int16 uCurrentNext, u_int32 uTag, bool bContinuous) 
{
  PDVBREQUEST pRequest;

  trace_new(TL_INFO, "DVB: cnxt_dvb_retrieve_pat version %d, section %d\n", uVersionNum, uSectionNum);
  
  pRequest = cnxt_dvb_retrieve_section_internal(gDemux,
                                                DVBREQUEST_PAT, 
                                                0,
                                                0,
                                                0xFF,
                                                uVersionNum, 
                                                uSectionNum, 
                                                DVB_DONT_CARE,
                                                uCurrentNext, 
                                                PAT_BUFFER_SIZE,
                                                uTag,
                                                bContinuous);
                                                
  trace_new(TL_INFO, "DVB: cnxt_dvb_retrieve_pat returning 0x%08x\n", pRequest);
                                       
  return((DVBREQUESTHANDLE)pRequest);
}


/********************************************************************/
/*  FUNCTION:    cnxt_dvb_retrieve_pmt                              */
/*                                                                  */
/*  PARAMETERS:  uVersionNumber - Required version number or        */
/*                                DVB_DONT_CARE for any version     */
/*               uServiceID     - A service ID to filter on or      */
/*                                DVB_DONT_CARE to get any.         */
/*               uCurrentNext   - 1 to get current table, 0 to get  */
/*                                next tabke, DVB_DONT_CARE to get  */
/*                                either                            */
/*               uTag           - Tag which will be passed back to  */
/*                                the DVB callback with the data    */
/*                                                                  */
/*  DESCRIPTION: Retrieve the PMT with PID uPMTPID from the current */
/*               stream. Data will be returned asynchronously on    */
/*               the DVB callback.                                  */
/*                                                                  */
/*  RETURNS:     A valid request handle if successful, else NULL.   */
/*                                                                  */
/*  CONTEXT:     Must be called in task context                     */
/*                                                                  */
/********************************************************************/
DVBREQUESTHANDLE cnxt_dvb_retrieve_pmt(int gDemux, u_int16 uPMTPID, u_int16 uVersionNumber, u_int16 uServiceID, u_int16 uCurrentNext, u_int32 uTag, bool bContinuous) 
{
  PDVBREQUEST pRequest;
  
  trace_new(TL_INFO, "DVB: cnxt_dvb_retrieve_pmt PID %04x, Service %d\n", uPMTPID, uServiceID);
  
  pRequest = cnxt_dvb_retrieve_section_internal(gDemux, 
                                                DVBREQUEST_PMT, 
                                                uPMTPID,
                                                0x02,
                                                0xFF,
                                                uVersionNumber, 
                                                DVB_DONT_CARE,
                                                uServiceID,   
                                                uCurrentNext, 
                                                PMT_BUFFER_SIZE,
                                                uTag,
                                                bContinuous);

  trace_new(TL_INFO, "DVB: cnxt_dvb_retrieve_pmt returning 0x%08x\n", pRequest);
                                       
  return((DVBREQUESTHANDLE)pRequest);
}

/********************************************************************/
/*  FUNCTION:    cnxt_dvb_retrieve_cat                              */
/*                                                                  */
/*  PARAMETERS:  uVersionNumber - Required version number or        */
/*                                DVB_DONT_CARE for any version     */
/*               uSectionNumber - Required section number or        */
/*                                DVB_DONT_CARE for any section     */
/*               uCurrentNext   - 1 to get current table, 0 to get  */
/*                                next tabke, DVB_DONT_CARE to get  */
/*                                either                            */
/*               uTag           - Tag which will be passed back to  */
/*                                the DVB callback with the data    */
/*                                                                  */
/*  DESCRIPTION: Retrieve the CAT from the current stream. Data     */
/*               will be returned asynchronously on the DVB         */
/*               callback.                                          */
/*                                                                  */
/*  RETURNS:     A valid request handle if successful, else NULL.   */
/*                                                                  */
/*  CONTEXT:     Must be called in task context                     */
/*                                                                  */
/********************************************************************/
DVBREQUESTHANDLE cnxt_dvb_retrieve_cat(int gDemux, u_int16 uVersionNumber, u_int16 uSectionNumber, u_int16 uCurrentNext, u_int32 uTag, bool bContinuous) 
{
  PDVBREQUEST pRequest;

  trace_new(TL_INFO, "DVB: cnxt_dvb_retrieve_cat version %d, section %d\n", uVersionNumber, uSectionNumber);
  
  pRequest = cnxt_dvb_retrieve_section_internal(gDemux, 
                                                DVBREQUEST_CAT, 
                                                0x01,
                                                0x01,
                                                0xFF,
                                                uVersionNumber,
                                                uSectionNumber,
                                                DVB_DONT_CARE,
                                                uCurrentNext, 
                                                CAT_BUFFER_SIZE,
                                                uTag,
                                                bContinuous);
                                       
  trace_new(TL_INFO, "DVB: cnxt_dvb_retrieve_cat returning 0x%08x\n", pRequest);
  
  return((DVBREQUESTHANDLE)pRequest);
}

/********************************************************************/
/*  FUNCTION:    cnxt_dvb_retrieve_section                          */
/*                                                                  */
/*  PARAMETERS:  uVersionNumber - Required version number or        */
/*                                DVB_DONT_CARE for any version     */
/*               uSectionNumber - Required section number or        */
/*                                DVB_DONT_CARE for any section     */
/*               uCurrentNext   - 1 to get current table, 0 to get  */
/*                                next tabke, DVB_DONT_CARE to get  */
/*                                either                            */
/*               uTag           - Tag which will be passed back to  */
/*                                the DVB callback with the data    */
/*                                                                  */
/*  DESCRIPTION: Retrieve the CAT from the current stream. Data     */
/*               will be returned asynchronously on the DVB         */
/*               callback.                                          */
/*                                                                  */
/*  RETURNS:     A valid request handle if successful, else NULL.   */
/*                                                                  */
/*  CONTEXT:     Must be called in task context                     */
/*                                                                  */
/********************************************************************/
DVBREQUESTHANDLE cnxt_dvb_retrieve_section(int gDemux, u_int16 uPID, u_int16 uTableID, u_int16 uTableIDMask, u_int16 uVersionNum, u_int16 uSectionNum, u_int16 uID, u_int16 uCurrentNext, u_int32 uBufferSize, u_int32 uTag, bool bContinuous)
{
  PDVBREQUEST pRequest;

  trace_new(TL_INFO, "DVB: cnxt_dvb_retrieve_section PID %04x, table 0x%x, mask 0x%x\n", uPID, uTableID, uTableIDMask);
  
  pRequest = cnxt_dvb_retrieve_section_internal(gDemux,
                                                DVBREQUEST_Other, 
                                                uPID,
                                                uTableID,
                                                uTableIDMask,
                                                uVersionNum,
                                                uSectionNum,
                                                uID,
                                                uCurrentNext, 
                                                uBufferSize,
                                                uTag,
                                                bContinuous);

  trace_new(TL_INFO, "DVB: cnxt_dvb_retrieve_section returning 0x%08x\n", pRequest);
                                       
  return((DVBREQUESTHANDLE)pRequest);
}


/********************************************************************/
/*  FUNCTION:    cnxt_dvb_cancel_request                            */
/*                                                                  */
/*  PARAMETERS:  hRequest - handle returned by a previous call to   */
/*                          cnxt_dvb_retrieve_pat/pmt/cat.          */
/*                                                                  */
/*  DESCRIPTION: Free a section request handle and all resources    */
/*               associated with it. Note that this DOES NOT free   */
/*               memory allocated to store any sections already     */
/*               returned to the client. This is the client's       */
/*               responsibility.                                    */
/*                                                                  */
/*  RETURNS:     TRUE on success, FALSE on failure.                 */
/*                                                                  */
/*  CONTEXT:     Should be called in task context                   */
/*                                                                  */
/********************************************************************/
bool cnxt_dvb_cancel_request(DVBREQUESTHANDLE hRequest) 
{
  PDVBREQUEST pRequest = (PDVBREQUEST)hRequest;
  DMX_STATUS  eRetcode;
  int         iDemux;
  bool        bRetcode = TRUE;
  
  trace_new(TL_INFO, "DVB: cnxt_dvb_cancel_request 0x%08x\n", hRequest);
  
  /* Check that the handle is not NULL and has not been freed already */
  if(pRequest && (pRequest->uMagic == DVB_REQUEST_MAGIC_NUMBER))
  {
    iDemux = pRequest->iDemuxInstance;
    
    eRetcode = cnxt_dmx_channel_control(iDemux, pRequest->uChannel, GEN_DEMUX_DISABLE);
    if(eRetcode != DMX_STATUS_OK)
    {
      trace_new(TL_ERROR, "DVB: Error from cnxt_dmx_channel_control %d\n", eRetcode);
      bRetcode = FALSE;
    }
    
    eRetcode = cnxt_dmx_filter_control(iDemux, pRequest->uChannel, pRequest->uFilter, GEN_DEMUX_DISABLE);
    if(eRetcode != DMX_STATUS_OK)
    {
      trace_new(TL_ERROR, "DVB: Error from cnxt_dmx_channel_control %d\n", eRetcode);
      bRetcode = FALSE;
    }
    
    eRetcode = cnxt_dmx_filter_close(iDemux, pRequest->uChannel, pRequest->uFilter);
    if(eRetcode != DMX_STATUS_OK)
    {
      trace_new(TL_ERROR, "DVB: Error from cnxt_dmx_filter_close %d\n", eRetcode);
      bRetcode = FALSE;
    }
    eRetcode = cnxt_dmx_channel_close(iDemux, pRequest->uChannel);
    if(eRetcode != DMX_STATUS_OK)
    {
      trace_new(TL_ERROR, "DVB: Error from cnxt_dmx_channel_close %d\n", eRetcode);
      bRetcode = FALSE;
    }
    
    /* Mark handle as invalid in case someone tries to free it twice */
    pRequest->uMagic = 0;
    
    /* Free memory tied to the handle */
    mem_nc_free(pRequest->pBuffer);
    mem_free(pRequest);
    return(bRetcode);
  }
  else
  {
    trace_new(TL_ERROR, TRACE_FG_LIGHT_RED  "DVB: Bad handle 0x%08x passed to cnxt_dvb_cancel_request\n" TRACE_FG_NORMAL, hRequest);
    return(FALSE);
  }  
}

/********************************************************************/
/*  FUNCTION:    cnxt_dvb_parse_pat                                 */
/*                                                                  */
/*  PARAMETERS:  pPAT        - Pointer to the PAT whose entries are */
/*                             to be parsed.                        */
/*               pfnCallback - Pointer to a function which will be  */
/*                             called for each program found in the */
/*                             PAT.                                 */
/*               uTag        - Tag which will be passed to the      */
/*                             callback function along with info on */
/*                             each parsed program.                 */
/*                                                                  */
/*  DESCRIPTION: Parse the supplied PAT and make a call to the      */
/*               supplied function for each program found. If the   */
/*               callback returns TRUE, keep parsing. If it returns */
/*               FALSE or the end of the PAT is reached, return     */
/*               from this function.                                */
/*                                                                  */
/*  RETURNS:     TRUE on success, FALSE on error                    */
/*                                                                  */
/*  CONTEXT:     May be called in any context. If called under      */
/*               interrupt, the function pointed to by pfnCallback  */
/*               must also be interrupt-safe.                       */
/********************************************************************/
bool cnxt_dvb_parse_pat(PDVBPAT pPAT, DVB_PFNPARSEPATENTRY pfnCallback, u_int32 uTag)
{
  PDVBSECTIONHEADER pHeader = (PDVBSECTIONHEADER)pPAT;
  PDVBPATPROGRAM pProgram;
  u_int32 uLength;
  u_int16 uSID;
  u_int16 uPMTPID;
  int     iNumPrograms;
  int     iLoop;
  bool    bEnd;

  if(!pfnCallback || !pPAT)
    return(FALSE);
    
  /* How many programs are described in this PAT? */  
  uLength = DVBSECTIONHEADER_LENGTH(pHeader->uLenSyntax);
  iNumPrograms = (int)((uLength - (DVB_HEADER_SIZE_AFTER_LEN+DVB_SECTION_CRC_SIZE)) / sizeof(DVBPATPROGRAM));

  /* Get a pointer to the first program in the PAT section */
  pProgram = (PDVBPATPROGRAM)(pHeader+1);
  
  /* Loop through each program returning information to the caller */
  for(iLoop = 0; iLoop < iNumPrograms; iLoop++)
  {
    uSID    = DVBPATPROGRAM_SERVICE(pProgram->uProgram);
    uPMTPID = DVBPATPROGRAM_PID(pProgram->uPID);
    
    /* Pass information on this program back to the caller */
    bEnd    = pfnCallback(iLoop, uSID, uPMTPID, uTag);
    if(!bEnd)
      break;
      
    /* Move on to the next program */  
    pProgram++;  
  }  
  return(TRUE);  
}


/********************************************************************/
/*  FUNCTION:    cnxt_dvb_get_pat_info                              */
/*                                                                  */
/*  PARAMETERS:  pPAT          - Pointer to a PAT section as passed */
/*                               to the DVB callback.               */
/*               pPatInfo      - Storage for returned information   */
/*                               about this PAT.                    */
/*                                                                  */
/*  DESCRIPTION: This function parses the header of the PAT that is */
/*               passed and returns the information it contains in  */
/*               a DVBPATINFO structure.                            */
/*                                                                  */
/*  RETURNS:     TRUE on success, FALSE if the PAT passed is not    */
/*               valid.                                             */
/*                                                                  */
/*  CONTEXT:     May be called in any context                       */
/*                                                                  */
/********************************************************************/
bool cnxt_dvb_get_pat_info(PDVBPAT pPAT, PDVBPATINFO pPatInfo)
{
  PDVBSECTIONHEADER pHeader = (PDVBSECTIONHEADER)pPAT;
  u_int32 uLength;
  
  /* Make sure we were not passed a NULL pointer */
  if(!pPatInfo)
    return(FALSE);
 
  /* Read the basic header fields */  
  pPatInfo->cLastSection = DVBSECTIONHEADER_LASTSECTION(pHeader->cLastSection);
  pPatInfo->cSection     = DVBSECTIONHEADER_SECTION(pHeader->cSection);
  pPatInfo->cVersion     = DVBSECTIONHEADER_VERSION(pHeader->cVerCurNext);
  pPatInfo->uTSID        = DVBSECTIONHEADER_ID(pHeader->uID);
  
  /* Calculate the number of programs in this PAT */
  uLength = DVBSECTIONHEADER_LENGTH(pHeader->uLenSyntax);
  pPatInfo->uNumServices = (uLength - (DVB_HEADER_SIZE_AFTER_LEN+DVB_SECTION_CRC_SIZE)) / sizeof(DVBPATPROGRAM);
  
  return(TRUE);
}

/********************************************************************/
/*  FUNCTION:    cnxt_dvb_get_pmt_info                              */
/*                                                                  */
/*  PARAMETERS:  pPAT          - Pointer to a PMT section as passed */
/*                               to the DVB callback.               */
/*               pPatInfo      - Storage for returned information   */
/*                               about this PMT.                    */
/*                                                                  */
/*  DESCRIPTION: This function parses the header of the PMT that is */
/*               passed and returns the information it contains in  */
/*               a DVBPMTINFO structure.                            */
/*                                                                  */
/*  RETURNS:     TRUE on success, FALSE if the PMT passed is not    */
/*               valid.                                             */
/*                                                                  */
/*  CONTEXT:     May be called in any context                       */
/*                                                                  */
/********************************************************************/
bool cnxt_dvb_get_pmt_info(PDVBPAT pPMT, PDVBPMTINFO pPmtInfo) 
{
  PDVBSECTIONHEADER pHeader = (PDVBSECTIONHEADER)pPMT;
  u_int32 uLength;
  u_int16 uProgInfoLen;
  u_int16 uESDescLen;
  u_int16 uPCRPID;
  u_int16 uNumProgs;
  u_int8  *pByte;
  u_int8  *pLastByte;
  
  /* Make sure we were not passed a NULL pointer */
  if(!pPmtInfo)
    return(FALSE);
 
  /* Read the basic header fields */  
  pPmtInfo->cLastSection = DVBSECTIONHEADER_LASTSECTION(pHeader->cLastSection);
  pPmtInfo->cSection     = DVBSECTIONHEADER_SECTION(pHeader->cSection);
  pPmtInfo->cVersion     = DVBSECTIONHEADER_VERSION(pHeader->cVerCurNext);
  pPmtInfo->uServiceID   = DVBSECTIONHEADER_ID(pHeader->uID);
  
  /* Extract the PCR PID which appears immediately after the common header fields */
  uPCRPID = *(u_int16 *)(pHeader+1);
  pPmtInfo->uPCRPID = DVBSECTIONHEADER_SWAPWORD16(uPCRPID) & 0x1FFF;
  
  /* Calculate the number of streams in this PMT */
  uLength = DVBSECTIONHEADER_LENGTH(pHeader->uLenSyntax);

  /* Get a pointer to the end of the header */
  pByte = (u_int8 *)(pHeader+1);
  
  /* Get a pointer to the last byte in the section */
  pLastByte = pByte + (uLength-5);

  /* Get past the PCR PID field */
  pByte += 2; 
  
  /* Read the length of the program descriptors block and skip past it */
  uProgInfoLen = DVB_READSWAPPEDWORD16(pByte) & 0xFFF;
  pByte += (2+uProgInfoLen);
  
  /* Loop through the descriptors counting streams */
  uNumProgs = 0;
  
  while(pByte < pLastByte)
  {
    uNumProgs++;
    
    /* Skip the stream ID and PID */
    pByte += 3;
    
    /* Read the ES descriptor length (if one is there) */
    uESDescLen = DVB_READSWAPPEDWORD16(pByte) & 0xFFF;
    
    /* Skip to the start of the next stream's descriptor */
    pByte += (2+uESDescLen);
  }

  pPmtInfo->uNumStreams = uNumProgs;  
  
  return(TRUE);
}

/********************************************************************/
/*  FUNCTION:    cnxt_dvb_get_cat_info                              */
/*                                                                  */
/*  PARAMETERS:  pPAT          - Pointer to a CAT section as passed */
/*                               to the DVB callback.               */
/*               pPatInfo      - Storage for returned information   */
/*                               about this CAT.                    */
/*                                                                  */
/*  DESCRIPTION: This function parses the header of the CAT that is */
/*               passed and returns the information it contains in  */
/*               a DVBCATINFO structure.                            */
/*                                                                  */
/*  RETURNS:     TRUE on success, FALSE if the CAT passes is not    */
/*               valid.                                             */
/*                                                                  */
/*  CONTEXT:     May be called in any context                       */
/*                                                                  */
/********************************************************************/
bool cnxt_dvb_get_cat_info(PDVBCAT pCAT, PDVBCATINFO pCatInfo) 
{                                   
  PDVBSECTIONHEADER pHeader = (PDVBSECTIONHEADER)pCAT;
  u_int32 uLength;
  
  /* Make sure we were not passed a NULL pointer */
  if(!pCatInfo)
    return(FALSE);
 
  /* Read the basic header fields */  
  pCatInfo->cLastSection = DVBSECTIONHEADER_LASTSECTION(pHeader->cLastSection);
  pCatInfo->cSection     = DVBSECTIONHEADER_SECTION(pHeader->cSection);
  pCatInfo->cVersion     = DVBSECTIONHEADER_VERSION(pHeader->cVerCurNext);
  
  /* Calculate the size of the descriptor block */
  uLength = DVBSECTIONHEADER_LENGTH(pHeader->uLenSyntax);
  pCatInfo->uLenDescriptors = uLength - 5;
  pCatInfo->pDescriptors = (u_int8 *)(pHeader+1);
  
  return(TRUE);
}

/********************************************************************/
/*  FUNCTION:    cnxt_dvb_parse_pmt                                 */
/*                                                                  */
/*  PARAMETERS:  pPMT        - Pointer to the PMT whose streams are */
/*                             to be parsed.                        */
/*               pfnCallback - Pointer to a function which will be  */
/*                             called for each stream found in the  */
/*                             PMT.                                 */
/*               uTag        - Tag which will be passed to the      */
/*                             callback function along with info on */
/*                             each parsed program.                 */
/*                                                                  */
/*  DESCRIPTION: Parse the supplied PMT and make a call to the      */
/*               supplied function for each stream found. If the    */
/*               callback returns TRUE, keep parsing. If it returns */
/*               FALSE or the end of the PMT is reached, return     */
/*               from this function.                                */
/*                                                                  */
/*  RETURNS:     TRUE on success, FALSE on error                    */
/*                                                                  */
/*  CONTEXT:     May be called in any context. If called under      */
/*               interrupt, the function pointed to by pfnCallback  */
/*               must also be interrupt-safe.                       */
/********************************************************************/
bool cnxt_dvb_parse_pmt(PDVBPMT pPMT, DVB_PFNPARSEPMTENTRY pfnCallback, u_int32 uTag)
{
  PDVBSECTIONHEADER pHeader = (PDVBSECTIONHEADER)pPMT;
  u_int32 uLength;
  u_int16 uProgInfoLen;
  u_int16 uESDescLen;
  u_int16 uStreamNum;
  u_int8  *pByte;
  u_int8  *pLastByte;
  u_int16 uStreamPID;
  u_int8  cStreamType;
  bool    bRetcode;
  
  /* Make sure we were not passed a NULL pointer */
  if(!pPMT || !pfnCallback)
    return(FALSE);
 
  /* Calculate the number of streams in this PMT */
  uLength = DVBSECTIONHEADER_LENGTH(pHeader->uLenSyntax);

  /* Get a pointer to the end of the header */
  pByte = (u_int8 *)(pHeader+1);
  
  /* Get a pointer to the last byte in the section */
  pLastByte = pByte + (uLength-5);
  
  /* Skip the PCR PID field */
  pByte += 2; /* Get past the PCR PID field */
  
  /* How long are the program descriptors? */
  uProgInfoLen = DVB_READSWAPPEDWORD16(pByte) & 0xFFF;
  
  /* Skip past the program descriptors (if any) */
  pByte += (2+uProgInfoLen);
  
  uStreamNum = 0;
  
  while(pByte < pLastByte)
  {
    /* Read the stream ID and PID */
    cStreamType = *pByte++;
    uStreamPID = DVB_READSWAPPEDWORD16(pByte) & 0x1FFF;
    pByte += 2;
    
    /* Read the ES descriptor length (if one is there) */
    uESDescLen = DVB_READSWAPPEDWORD16(pByte) & 0xFFF;
    pByte += 2;

    /* Pass this stream's info to the caller */
    bRetcode = pfnCallback(uStreamNum, cStreamType, uStreamPID, pByte, uESDescLen, uTag);
    if(!bRetcode)
      break;
    
    /* Skip to the start of the next stream's descriptor */
    pByte += uESDescLen;
    uStreamNum++;
  }
  
  return(TRUE);
}

/********************************************************************/
/*  FUNCTION:    dvb_request_header_notify                          */
/*                                                                  */
/*  PARAMETERS:  pNotifyData - information on the section whose     */
/*                             header we are being notified about   */
/*                                                                  */
/*  DESCRIPTION: This callback is called by the demux driver when a */
/*               section that we may be interested in is recieved.  */
/*               We are passed the first few bytes and given the    */
/*               opportunity to retrieve the rest of the section or */
/*               ignore it.                                         */
/*                                                                  */
/*  RETURNS:     GENDEMUX_ONE_SHOT if we want to turn the channel   */
/*               off or GENDEMUX_CONTINUOUS if we want to continue  */
/*               looking for sections.                              */
/*                                                                  */
/*  CONTEXT:     Will be called only by the demux driver and in     */
/*               task context.                                      */
/*                                                                  */
/********************************************************************/
static genfilter_mode dvb_request_header_notify(pgen_notify_t pNotifyData)
{
  PDVBREQUEST pRequest = (PDVBREQUEST)pNotifyData->tag;
  u_int8 *header       = pNotifyData->pData;

  /*
   * Ensure we're not dealing with a timeout condition
   */

  if(pNotifyData->condition & GENDMX_CHANNEL_TIMEOUT)
  {
      trace_new(TL_ERROR, TRACE_FG_LIGHT_RED  "DVB: Timeout for request 0x%08x\n" TRACE_FG_NORMAL, pRequest);
      
      /* Inform the client of the timeout error */
      if(pfnDVBCallback)
        pfnDVBCallback(DVB_CTRL_TIMEOUT, (u_int32)pRequest, 0, 0);
        
      /* Turn off acquisition for this channel */  
      pNotifyData->length = 0;
      return GENDEMUX_ONE_SHOT;   
  }

  if ((pNotifyData->chid == pRequest->uChannel) && (header != NULL))
  {
      if(!(pNotifyData->condition & GENDMX_ERROR))
      {
        pNotifyData->skip = 0;
        ++header;
        pNotifyData->length = (*header++ & 0x0F) << 8; pNotifyData->length |= *header++;
        pNotifyData->length += 4; /* pad for the CRC*/
        pNotifyData->write_ptr = (u_int8 *) mem_malloc(pNotifyData->length);
      
        trace_new(TL_INFO, "DVB: Header notify. Requesting  %d byte section. Handle 0x%08x\n", pNotifyData->length, pRequest);
      }
      else
      {
        /* Error in section - reject it and wait for the next one */
        trace_new(TL_ERROR, TRACE_FG_LIGHT_RED "DVB: Header notify. Error in section for handle 0x%08x. Waiting for next one...\n" TRACE_FG_NORMAL, pRequest);
        pNotifyData->length = 0;
        pNotifyData->write_ptr = NULL;
        return(GENDEMUX_CONTINUOUS);
      }  
  }
  else
  {
      trace_new(TL_ERROR, TRACE_FG_LIGHT_RED "DVB: Header notify. NULL ptr or unmatched channel. Request 0x%08x\n" TRACE_FG_NORMAL, pRequest);
      pNotifyData->length = 0;
      pNotifyData->write_ptr = NULL;
  }
  
  if(pRequest->bContinuous)
    return(GENDEMUX_CONTINUOUS);
  else
    return(GENDEMUX_ONE_SHOT);
}

/********************************************************************/
/*  FUNCTION:    dvb_request_section_notify                         */
/*                                                                  */
/*  PARAMETERS:  pNotifyData - information on the section whose     */
/*                             data we are being notified about     */
/*                                                                  */
/*  DESCRIPTION: This callback is called by the demux driver when a */
/*               section that we indicated interest in has been     */
/*               completely received.                               */
/*                                                                  */
/*  RETURNS:     Nothing                                            */
/*                                                                  */
/*  CONTEXT:     Will be called only by the demux driver and in     */
/*               task context.                                      */
/*                                                                  */
/********************************************************************/
static void dvb_request_section_notify(pgen_notify_t pNotifyData)
{
  DVB_CTRL_MSG_ID uMessage;
  PDVBREQUEST pRequest = (PDVBREQUEST)pNotifyData->tag;
    
  /* Were we passed a good buffer pointer? */  
  if (pNotifyData->write_ptr == NULL)
  {
      trace_new(TL_ERROR, TRACE_FG_LIGHT_RED  "DVB: Section notify passed NULL pointer\n" TRACE_FG_NORMAL);
      return;
  } 

  /* Does this notification result from a valid request? */
  if(!pRequest || (pRequest->uMagic != DVB_REQUEST_MAGIC_NUMBER))
  {
      trace_new(TL_ERROR, TRACE_FG_LIGHT_RED  "DVB: Unexpected section notify - handle 0x%08x problem \n" TRACE_FG_NORMAL, pRequest);
      mem_free(pNotifyData->write_ptr);
      return;
  }

  /* Were we passed any data in the buffer? */
  if (pNotifyData->length == 0)
  {
      trace_new(TL_ERROR, TRACE_FG_LIGHT_RED  "DVB: Section notify passed no data\n" TRACE_FG_NORMAL);
      mem_free(pNotifyData->write_ptr);
      return;
  }

  /* Was the section received correctly? */
  if (pNotifyData->condition == GENDMX_SECTION_AVAILABLE+GENDMX_CRC_CHECKED)
  {
      trace_new(TL_INFO, "DVB: Section notify for %d byte section\n", pNotifyData->length);
      
      if(pfnDVBCallback)
      {
        switch(pRequest->eType)
        {
          case DVBREQUEST_PAT:   uMessage = DVB_CTRL_PAT_SECTION; break;
          case DVBREQUEST_PMT:   uMessage = DVB_CTRL_PMT_SECTION; break;
          case DVBREQUEST_CAT:   uMessage = DVB_CTRL_CAT_SECTION; break;
          default:               uMessage = DVB_CTRL_OTHER_SECTION; break;
        }  
        pfnDVBCallback(uMessage, (u_int32)pRequest, (u_int32)pNotifyData->write_ptr, pRequest->uTag);
      }  
  }
  else
  {
      /* An error occurred - what was it? */
      if(pNotifyData->condition == GENDMX_CHANNEL_TIMEOUT)
      {
          uMessage = DVB_CTRL_TIMEOUT;
          trace_new(TL_ERROR, TRACE_FG_LIGHT_RED  "DVB: Section notify: TIMEOUT_ERROR\n" TRACE_FG_NORMAL);
      }
      else
      {
          uMessage = DVB_CTRL_ERROR;
          trace_new(TL_ERROR, TRACE_FG_LIGHT_RED  "DVB: Section notify: SECTION_ERROR\n" TRACE_FG_NORMAL);
      }
      
      /* Inform the client of the error */
      if(pfnDVBCallback)
        pfnDVBCallback(uMessage, (u_int32)pRequest, pNotifyData->condition, pRequest->uTag);
  }
}
  

/****************************************************************************
 * Modifications:
 * $Log: 
 *  7    mpeg      1.6         12/9/03 10:13:43 AM    Larry Wang      CR(s) 
 *        8115 8118 : Mask off bit 28 of PSI buffer pointer.
 *        
 *  6    mpeg      1.5         12/3/03 2:27:40 PM     Larry Wang      CR(s): 
 *        8086 8087 Allocate PSI buffers by mem_nc_malloc().
 *  5    mpeg      1.4         9/16/03 5:42:44 PM     Angela Swartz   SCR(s) 
 *        7477 :
 *        #include stbcfg.h to pickup some macro defines
 *        
 *  4    mpeg      1.3         7/3/03 11:00:28 AM     Dave Wilson     SCR(s) 
 *        6879 :
 *        Header notify callback now ignores any section where an error is 
 *        reported.
 *        Previously, the section was requested anyway.
 *        Added more function entry/exit trace.
 *        
 *  3    mpeg      1.2         4/17/03 11:55:28 AM    Dave Wilson     SCR(s) 
 *        6051 :
 *        Added a demux instance parameter to all APIs that end up calling into
 *         the
 *        demux driver. This allows the interface to be used on multi-demux 
 *        systems.
 *        
 *  2    mpeg      1.1         4/17/03 11:40:06 AM    Dave Wilson     SCR(s) 
 *        6024 :
 *        Added table ID mask parameter to cnxt_dvb_retrieve_section
 *        
 *  1    mpeg      1.0         4/11/03 6:00:34 PM     Dave Wilson     
 * $
 * 
 ****************************************************************************/

