/****************************************************************************/
/*                   CONEXANT PROPRIETARY AND CONFIDENTIAL                  */
/*                       SOFTWARE FILE/MODULE HEADER                        */
/*                Copyright Conexant Systems Inc. 1998-2003                 */
/*                         All Rights Reserved                              */
/****************************************************************************/
/*
 * Filename:        tvenc.c
 *
 *
 * Description:     Functions relating to setup and operation of various supported video encoders
 *
 *
 * Author:          Xin Golden (based on the encoder.c in pvcs\osdlibc)
 *
 ****************************************************************************/
/* $Header: tvenc.c, 12, 3/18/04 11:54:38 AM, Xin Golden$
 ****************************************************************************/

/*****************/
/* Include Files */
/*****************/
#include <string.h>
#include "stbcfg.h"
#include "basetype.h"
#include "kal.h"
#include "retcodes.h"
#include "globals.h"
#include "handle.h"
#include "osdlibc.h"
#include "vidlibc.h"
#include "vidprvc.h"
#include "tvenc.h"
#include "tvenc_priv.h"
#include "tvenc_module_api.h"


/***********************/
/* External References */
/***********************/
/* global variables used in osdlibc.*/
/* Should be removed after the multi-instance osdlibc driver is integrated */ 
extern int gnOsdMaxHeight;
extern int gnOsdMaxWidth;
extern u_int32    gdwEncoderType; /* Beware - referenced still by some external drivers! */

/* Define the init pointers for all the tvenc drivers.  NULL is
   not present.  Adding a new kind of tvenc to the system will
   require adding a block here, a line in the definition of
   the tvenc_init array, and a value in CNXT_TVENC_MODULE in tvenc.h.  */
/* the order of the encoder type in CNXT_TVENC_MODULE needs to 
   be the same as it is in tvenc_init array                      */

#if VIDEO_ENCODER_0 == INTERNAL
   extern INIT_FUNC     bt861_internal_tvenc_init;
   #define BT861_INTERNAL_TVENC & bt861_internal_tvenc_init
   #define BT861_TVENC   0
#else
     #define BT861_INTERNAL_TVENC   0
     #if VIDEO_ENCODER_0 == TVENC_BT861
        extern INIT_FUNC   bt861_tvenc_init;
        #define BT861_TVENC & bt861_tvenc_init
    #else
       #error "Specified TV encoder is not supported!"
    #endif
#endif


static INIT_FUNC              *tvenc_init[] =
{
   BT861_TVENC,
   BT861_INTERNAL_TVENC
};


/*******************/
/* Local Variables */
/*******************/
static CNXT_TVENC_DRIVER_INST DriverInst = { 0 };
static CNXT_TVENC_DRIVER_INST *pDriverInst = &DriverInst;

static CNXT_TVENC_INST InstArray[CNXT_TVENC_MAX_HANDLES];  /* Memory pool to get inst */
static bool            bInstUsed[CNXT_TVENC_MAX_HANDLES];  


static CNXT_TVENC_UNIT_INST UnitInst[CNXT_TVENC_NUM_UNITS];  /* for Driver Inst structure */
 /* Static array of actual caps */   
static CNXT_TVENC_CAPS CapsArray[CNXT_TVENC_NUM_UNITS] = 
{ 
  {
     sizeof(CNXT_TVENC_CAPS) , 
     0,  
     TVENC_CONTROL_BRIGHTNESS | TVENC_CONTROL_CONTRAST | TVENC_CONTROL_HUE \
     | TVENC_CONTROL_SATURATION | TVENC_CONTROL_SHARPNESS,
     VIDEO_SIGNAL_OUTPUT_TYPES
  }
}; 

static TVENC_FTABLE           function_table[CNXT_TVENC_MAX_MODULE];

/********************************/
/* Internal Function Prototypes */
/********************************/
static void cnxt_tvenc_notify_all_clients(CNXT_TVENC_EVENT Event);
static void    private_tvenc_internal_data_init(void);
static u_int32 private_tvenc_get_unit_num(CNXT_TVENC_CAPS *pCaps);
static CNXT_TVENC_STATUS private_close ( CNXT_TVENC_INST *pInst );
                                              

/***************************/
/* Local utility functions */
/***************************/

/********************************************************************/
/*  FUNCTION:    get_unitinst                                       */
/*                                                                  */
/*  PARAMETERS:  Handle - a handle returned on a previous call to   */
/*                        cnxt_tvenc_open.                          */
/*               pUnitInst - returned pointer to the unit instance  */
/*                           pointed by the handle                  */
/*                                                                  */
/*  DESCRIPTION: Return an unit instance pointed by the input handle*/            
/*                                                                  */
/*  RETURNS:     CNXT_TVENC_OK                                      */
/*               CNXT_TVENC_NOT_INIT                                */
/*               CNXT_TVENC_BAD_HANDLE                              */
/*               CNXT_TVENC_INTERNAL_ERROR                          */
/*                                                                  */
/*  CONTEXT:    Must be called in task context.                     */
/*                                                                  */
/********************************************************************/
CNXT_TVENC_STATUS get_unitinst( CNXT_TVENC_HANDLE Handle, CNXT_TVENC_UNIT_INST *pUnitInst )
{
   CNXT_TVENC_INST *pInst = (CNXT_TVENC_INST*)Handle;
   u_int32 uUnitNumber;
   
   IS_DRIVER_INITED(TVENC,pDriverInst->bInit);
   IS_VALID_HANDLE(TVENC, pInst, &(pDriverInst->pUnitInst[pInst->uUnitNumber].pFirstInst));

   if ( (uUnitNumber = pInst->uUnitNumber) > CNXT_TVENC_NUM_UNITS)
   {
      return CNXT_TVENC_INTERNAL_ERROR;
   }

   /* get the unit instance */
  *pUnitInst = pDriverInst->pUnitInst[uUnitNumber];

   return CNXT_TVENC_OK;
}

static void cnxt_tvenc_notify_all_clients(CNXT_TVENC_EVENT Event)
{
   u_int32 i;
   CNXT_TVENC_INST *pInst;

   /* 
    * Loop through the open handles, calling the pNotifyFn with 
    * CNXT_AVID_EVENT_TERMINATED notice
    */
   for (i = 0; i < CNXT_TVENC_NUM_UNITS; ++i)
   {
      pInst = pDriverInst->pUnitInst[i].pFirstInst;
      while ( pInst )
      {
         if ( pInst->pNotifyFn )
         {
            pInst->pNotifyFn((CNXT_TVENC_HANDLE)pInst,
                             pInst->pUserData,
                             Event,
                             NULL,
                             NULL);
         }
         pInst = (CNXT_TVENC_INST*)pInst->Preface.pNext;
      }
   }
}

/********************************************************************/
/*  FUNCTION:    private_tvenc_internal_data_init                   */
/*                                                                  */
/*  PARAMETERS:  None                                               */
/*                                                                  */
/*  DESCRIPTION: Initialize the internal data structures            */
/*                                                                  */
/*  RETURNS:     None                                               */
/*                                                                  */
/*  CONTEXT:    Must be called in task context.                     */
/*                                                                  */
/********************************************************************/
static void private_tvenc_internal_data_init(void)
{
   u_int32 i;

   /* Loop through the array of available Inst to set them to */
   /* unused, and make the list pointer invalid.              */
   
   for (i = 0; i < CNXT_TVENC_MAX_HANDLES; ++i)
   {
      bInstUsed[i] = FALSE;
      InstArray[i].Preface.pSelf = NULL;
      InstArray[i].Preface.pNext = &InstArray[i+1].Preface;
   }

   /* Terminate the linked list of instance structures. */
   InstArray[CNXT_TVENC_MAX_HANDLES-1].Preface.pNext = NULL;

   /* Initialize UnitInst */
   for (i = 0; i < CNXT_TVENC_NUM_UNITS; ++i)
   {
      UnitInst[i].pFirstInst = NULL;  /* list of handles is empty */
      UnitInst[i].bExclusive = FALSE;
      UnitInst[i].pCaps      = &CapsArray[i];
      UnitInst[i].UnitSem    = (sem_id_t)0;

      /* VIDEO_OUTPUT_CVBS, VIDEO_OUTPUT_RGB, and VIDEO_OUTPUT_YC are defined in hwconfig.cfg */
      UnitInst[i].uOutputConnection = VIDEO_SIGNAL_OUTPUT_TYPES;
      UnitInst[i].VideoStandard = CNXT_TVENC_STANDARD_INVALID;   /* Not PAL, nor NTSC, nor SECAM. */

      UnitInst[i].uExtTVENCIICAddr = I2C_ADDR_BT861;
      UnitInst[i].uExtTVENCBusAddr = I2C_BUS_BT861;

      UnitInst[i].uHBlankInitial = UnitInst[i].uVBlankInitial = 0;
      UnitInst[i].uHBlank = UnitInst[i].uVBlank = 0;
      UnitInst[i].iHSync = UnitInst[i].iHSyncInitial = 0;

      /* default values for picture controls */
      UnitInst[i].picctrl_brightness = 0;
      UnitInst[i].picctrl_contrast = 0;
      UnitInst[i].picctrl_hue = 0;
      UnitInst[i].picctrl_saturation = 0;
      UnitInst[i].picctrl_sharpness = 0;

      /* default values for X and Y screen start position offsets */
      UnitInst[i].XOffset = 0;
      UnitInst[i].YOffset = 0;

      /* default settings for teletext */
      UnitInst[i].uTTXBF1 = UnitInst[i].uTTXEF1 = 0;
      UnitInst[i].uTTXBF2 = UnitInst[i].uTTXEF2 = 0;

      /* disable closed captioning */
      UnitInst[i].bCCEnabled = FALSE; 

      UnitInst[i].WSS_Settings.AspectRatio = ASPECT_RATIO_INVALID;
      UnitInst[i].WSS_Settings.bColorCoding = FALSE;
      UnitInst[i].WSS_Settings.bCopyRestrict = FALSE;
      UnitInst[i].WSS_Settings.bCopyright = FALSE;
      UnitInst[i].WSS_Settings.bFilmMode = FALSE;
      UnitInst[i].WSS_Settings.bHelper = FALSE;
      UnitInst[i].WSS_Settings.bSubtitlesTTX = FALSE;
      UnitInst[i].WSS_Settings.bSurroundSound = FALSE;
      UnitInst[i].WSS_Settings.SubtitlingMode = SBTITLE_INVALID;
   }
}

/********************************************************************/
/*  FUNCTION:    private_tvenc_get_unit_num                         */
/*                                                                  */
/*   PARAMETERS: pCaps   - capabilities required of tvenc to be     */
/*                         opened.                                  */
/*                                                                  */
/*  DESCRIPTION: return the unit number based on pCaps              */
/*                                                                  */
/*  RETURNS:     uUnit - unit number                                */
/*                                                                  */
/*  CONTEXT:    Must be called in task context.                     */
/*                                                                  */
/********************************************************************/
static u_int32 private_tvenc_get_unit_num(CNXT_TVENC_CAPS *pCaps)
{
   u_int32 uUnit = 0;

   return uUnit;
}

/*********************************************************************/
/*  private_close ()                                                 */
/*                                                                   */
/*  PARAMETERS: CNXT_TVENC_INST *pInst                               */
/*                                                                   */
/*  DESCRIPTION: This function closes a instance of a device         */
/*                                                                   */
/*  RETURNS: CNXT_TVENC_OK if success                                */
/*           CNXT_TVENC_INTERNAL_ERROR if internal error occurs      */
/*********************************************************************/
static CNXT_TVENC_STATUS private_close ( CNXT_TVENC_INST *pInst )
{
   CNXT_TVENC_UNIT_INST *pUnitInst;

   pUnitInst = &(pDriverInst->pUnitInst[pInst->uUnitNumber]);

   /* release the resource of the instance */
   if (REMOVE_HANDLE(&pUnitInst->pFirstInst, pInst) == FALSE)
   {
      return CNXT_TVENC_INTERNAL_ERROR;
   }

   DESTROY_HANDLE(&(pDriverInst->pInstList), pInst);

   /* set the unit to be shared if no instance opened for the unit, also kill the Unit semaphore */
   if ( pUnitInst->pFirstInst == NULL )
   {
      pUnitInst->bExclusive = FALSE;
      sem_delete( pUnitInst->UnitSem );
      pUnitInst->UnitSem = (sem_id_t)0;
   }

   return CNXT_TVENC_OK;
} 

/*********************************************************************/
/*  reflect ()                                                       */
/*                                                                   */
/*  PARAMETERS: u_int32 uBase: value to be reflected                 */
/*              u_int8  uBits: number of the bottom bits of uBase to */
/*              to reflected.  For example, reflect(0x3e23,3)==0x3e26*/
/*                                                                   */
/*  DESCRIPTION: This function reflects value uBase with the bottom  */
/*               uBits reflected.                                    */
/*                                                                   */
/*  RETURNS: u_int32 reflected value.                                */
/*********************************************************************/
u_int32 reflect (u_int32 uBase, u_int8 uBits)
{
   int   i;
   u_int32 t = uBase;
   
   for (i=0; i<uBits; i++)
   {
      if (t & 1)
      {
         uBase|=  BITMASK((uBits-1)-i);
      }
      else
      {
         uBase&= ~BITMASK((uBits-1)-i);
      }
      t>>=1;
   }
   return uBase;
}

/* calculate CRC for CGMS. Poly is x**6+x+1 */
/*********************************************************************/
/*  cal_crc_8 ()                                                     */
/*                                                                   */
/*  PARAMETERS: u_int8 uCRCValue: initial CRC value                  */
/*              u_int16 uData: input data                            */
/*                                                                   */
/*  DESCRIPTION: This function calculates the CRC value of an input  */
/*               data using poly x**6+x+1.  The CRC register has     */ 
/*               6 bits.  The LSB of the CRC register is bit 0.      */
/*                                                                   */
/*  RETURNS: u_int8 CRC value.                                       */
/*********************************************************************/
u_int8 cal_crc_8(u_int8 uCRCValue, u_int16 uData)
{
   int n;
   u_int16 xmsb;
   u_int16 mask = 0x8000;

   for(n=CGMS_DATALEN; n; n--, uData <<=1)
   {
      xmsb = (uData & mask) ? 1 : 0;
      if (xmsb != (uCRCValue & 0x1))
      {
         uCRCValue = (uCRCValue >> 1) ^ CRC_POLY;
      }
      else
      {
         uCRCValue >>= 1;
      }
   }   
   return (uCRCValue<<2); 
}

/*****************/
/* API functions */
/*****************/
/********************************************************************/
/*  FUNCTION:    cnxt_tvenc_init                                    */
/*                                                                  */
/*  PARAMETERS:  pCfg - Pointer to driver configuration structure.  */
/*                                                                  */
/*  DESCRIPTION: This function initialises the driver and makes it  */
/*               ready for use.                                     */
/*                                                                  */
/*  RETURNS:     CNXT_TVENC_OK                                      */
/*               CNXT_TVENC_ALREADY_INIT                            */
/*               CNXT_TVENC_RESOURCE_ERROR                          */
/*               CNXT_TVENC_INTERNAL_ERROR                          */
/*                                                                  */
/*  CONTEXT:     May be called in any context.                      */
/*                                                                  */
/********************************************************************/
CNXT_TVENC_STATUS cnxt_tvenc_init ( CNXT_TVENC_CONFIG *pCfg )
{
   static bool ReEntry = FALSE;
   CNXT_TVENC_STATUS eRetcode;
   CNXT_TVENC_MODULE module = CNXT_TVENC_INVALID;
   bool bKs;
   u_int32 i;

     
   /* Need to ensure that this routine only called once, so     */
   /* start crit section, test the semaphore for existence.     */
   /* If not exist, then create.                                */
   /* get out of the critical section, and see if the semaphore */
   /* exists. If it doesn't, then return an error.              */
   /* Now get the semaphore. If this fails then                 */
   /* someone else grapped the semaphore, or there is a bigger  */
   /* system resource problem                                   */
   
   while (1)
   {
      bKs = critical_section_begin();
      if ( ReEntry == FALSE )
      {
         ReEntry = TRUE;
         critical_section_end(bKs);
         break;
      }
      critical_section_end(bKs);
      task_time_sleep(5);
   }

   if (pDriverInst->DriverSem == 0)
   {
      pDriverInst->DriverSem = sem_create(1, NULL);
   }

   if (pDriverInst->DriverSem == 0)
   {
      ReEntry = FALSE;
      return CNXT_TVENC_RESOURCE_ERROR;
   }

   if (sem_get(pDriverInst->DriverSem, KAL_WAIT_FOREVER) != RC_OK)
   {
      ReEntry = FALSE;
      return CNXT_TVENC_INTERNAL_ERROR;
   }

   if (pDriverInst->bInit == TRUE)
   {
      sem_put(pDriverInst->DriverSem);
      ReEntry = FALSE;
      return CNXT_TVENC_ALREADY_INIT;
   }

   /* Initialise our internal data structures */
   private_tvenc_internal_data_init();
  
   /* connect internal data to the driver instance structure */
   pDriverInst->pInstList = InstArray;
   pDriverInst->pUnitInst  = UnitInst;
   
   for (i = 0; i < CNXT_TVENC_NUM_UNITS; i++)
   { 

   #if VIDEO_ENCODER_0 == INTERNAL
      module = CNXT_TVENC_BT861_INTERNAL;
   #endif

   #if VIDEO_ENCODER_0 == TVENC_BT861
      module  = CNXT_TVENC_BT861;
      gdwEncoderType = BT861;
   #endif

      if( module != CNXT_TVENC_INVALID )
      {
         eRetcode = tvenc_init[module](&(pDriverInst->pUnitInst[i]), &function_table[module]);
      }
      else
      {
         eRetcode = CNXT_TVENC_INTERNAL_ERROR;
         trace_new(TRACE_TVENC, "no device is configured \n");
         return eRetcode;
      }

   }
  
   /* Flag that we are initialised, release the semaphore and go home */
   pDriverInst->bInit = TRUE; 
   sem_put(pDriverInst->DriverSem); 
   ReEntry = FALSE;
  
   return(eRetcode);
}

/********************************************************************/
/*  FUNCTION:    cnxt_tvenc_term                                    */
/*                                                                  */
/*  PARAMETERS:  None                                               */
/*                                                                  */
/*  DESCRIPTION: This function shuts down the driver, closing any   */
/*               open handles in the process.                       */
/*                                                                  */
/*  RETURNS:     CNXT_TVENC_OK                                      */
/*               CNXT_TVENC_NOT_INIT                                */
/*               CNXT_TVENC_CLOSED_HANDLE                           */
/*               CNXT_TVENC_INTERNAL_ERROR                          */
/*                                                                  */
/*  CONTEXT:     Must be called in task context.                    */
/*                                                                  */
/********************************************************************/
CNXT_TVENC_STATUS cnxt_tvenc_term ( void )
{
   u_int32 i;
   u_int32 uNumOfOpenHandles = 0;
   CNXT_TVENC_INST *pInst;
   CNXT_TVENC_STATUS eRetTemp;
   CNXT_TVENC_STATUS eRet = CNXT_TVENC_OK;

   IS_DRIVER_INITED(TVENC, pDriverInst->bInit);
   
   if (sem_get(pDriverInst->DriverSem, KAL_WAIT_FOREVER) != RC_OK)
      return CNXT_TVENC_INTERNAL_ERROR;

   /* Mark the driver as not initialized */
   pDriverInst->bInit = FALSE;

   /* notify all clients that the driver is terminated */
   cnxt_tvenc_notify_all_clients( CNXT_TVENC_EVENT_TERM );

   /* Close all open handles if there are any */
   for (i = 0; i < CNXT_TVENC_NUM_UNITS; ++i)
   {
      pInst = pDriverInst->pUnitInst[i].pFirstInst;
      while( pInst )
      {
         eRetTemp = private_close( pInst );
         if( eRetTemp != CNXT_TVENC_OK )
         {
            eRet = eRetTemp;
         }   
         pInst = (CNXT_TVENC_INST*)pInst->Preface.pNext;
         uNumOfOpenHandles++;
      }   
   }

   /* Destroy semaphore (critical section); */
   if (pDriverInst->DriverSem)
   {
      sem_delete(pDriverInst->DriverSem);
      pDriverInst->DriverSem = 0;
   }

   if( eRet != CNXT_TVENC_OK )
   {
      trace_new( TRACE_TVENC, "cnxt_tvenc_term: open instances exist, close an instance failed\n" );
      return eRet;
   }

   if( uNumOfOpenHandles )
   {
      trace_new( TRACE_TVENC, "cnxt_tvenc_term: return CLOSED_HANDLES %d\n", uNumOfOpenHandles);
      return CNXT_TVENC_CLOSED_HANDLE;
   }

   trace_new( TRACE_TVENC, "cnxt_tvenc_term: return OK\n");

   return CNXT_TVENC_OK;
}

/********************************************************************/
/*  FUNCTION:    cnxt_tvenc_get_units                               */
/*                                                                  */
/*  PARAMETERS:  puCount - storage for returned unit count          */
/*                                                                  */
/*  DESCRIPTION: This function returns the number of encoders in    */
/*               the system.                                        */
/*                                                                  */
/*  RETURNS:     CNXT_TVENC_OK                                      */
/*               CNXT_TVENC_NOT_INIT                                */
/*               CNXT_TVENC_BAD_PARAMETER                           */
/*                                                                  */
/*  CONTEXT:     Must be called in task context.                    */
/*                                                                  */
/********************************************************************/
CNXT_TVENC_STATUS cnxt_tvenc_get_units ( u_int32 *puCount )
{
   IS_DRIVER_INITED(TVENC,pDriverInst->bInit);
   IS_NOT_NULL_POINTER(TVENC, puCount);
  
   *puCount = CNXT_TVENC_NUM_UNITS;
   return(CNXT_TVENC_OK);
}

/********************************************************************/
/*  FUNCTION:    cnxt_tvenc_get_unit_caps                           */
/*                                                                  */
/*  PARAMETERS:  uUnitNumber - the unit whose capabilities are to   */
/*                             be returned. Unit number is 0-based. */
/*               pCaps       - pointer to storage for the returned  */
/*                             capabilities. Caller must complete   */
/*                             the uLength field prior to making    */
/*                             the call.                            */
/*                                                                  */
/*  DESCRIPTION: This function returns information on the           */
/*               capabilities of one of the video encoders in the   */
/*               system.                                            */
/*                                                                  */
/*  RETURNS:     CNXT_TVENC_OK                                      */
/*               CNXT_TVENC_NOT_INIT                                */
/*               CNXT_TVENC_BAD_UNIT                                */
/*               CNXT_TVENC_BAD_PARAMETER                           */
/*                                                                  */
/*  CONTEXT:     Must be called in task context.                    */
/*                                                                  */
/********************************************************************/
CNXT_TVENC_STATUS cnxt_tvenc_get_unit_caps ( u_int32         uUnitNumber, 
                                             CNXT_TVENC_CAPS *pCaps )
{
   u_int32 uLength;

   IS_DRIVER_INITED(TVENC, pDriverInst->bInit);
   IS_NOT_NULL_POINTER(TVENC, pCaps);

   /* Check that the module requested exists */
   if (uUnitNumber >= CNXT_TVENC_NUM_UNITS)
   {
      return CNXT_TVENC_BAD_UNIT;
   }

   /* Check that the passed length was not zero */
   uLength = pCaps->uLength;
   if (uLength == 0)
   {
      return CNXT_TVENC_BAD_PARAMETER;   
   }

   /* Copy the caps structure to the clients buffer with length protection */
   uLength = min(uLength, sizeof(CNXT_TVENC_CAPS));
   memcpy(pCaps, pDriverInst->pUnitInst[uUnitNumber].pCaps, uLength);
   pCaps->uLength = uLength;

   return CNXT_TVENC_OK;
}                                                 

/********************************************************************/
/*  FUNCTION:    cnxt_tvenc_open                                    */
/*                                                                  */
/*  PARAMETERS:  pHandle - pointer to storage for returned handle   */
/*               pCaps   - capabilities required of encoder to be   */
/*                         opened.                                  */
/*               pNotifyFn - callback                               */
/*               pUserData - user data value                        */
/*                                                                  */
/*  DESCRIPTION: This function creates a new driver instance        */
/*               meeting the requirements as provided in the pCaps  */
/*               structure. If successful, a handle is returned in  */
/*               *pHandle.                                          */
/*                                                                  */
/*  RETURNS:     CNXT_TVENC_OK                                      */
/*               CNXT_TVENC_NOT_INIT                                */
/*               CNXT_TVENC_BAD_PARAMETER                           */
/*               CNXT_TVENC_NOT_AVAILABLE                           */
/*               CNXT_TVENC_RESOURCE_ERROR                          */
/*               CNXT_TVENC_INTERNAL_ERROR                          */
/*                                                                  */
/*  CONTEXT:     Must be called in task context.                    */
/*                                                                  */
/********************************************************************/
CNXT_TVENC_STATUS cnxt_tvenc_open ( CNXT_TVENC_HANDLE    *pHandle,
                                    CNXT_TVENC_CAPS      *pCaps,
                                    CNXT_TVENC_PFNNOTIFY pNotifyFn,
                                    void                 *pUserData )
{
   CNXT_TVENC_INST *pInst;
   CNXT_TVENC_UNIT_INST *pUnitInst;
   u_int32 uUnit;

   trace_new(TRACE_TVENC, "TVENC: Opening...\n");
   
   IS_DRIVER_INITED(TVENC,pDriverInst->bInit);
   IS_NOT_NULL_POINTER(TVENC, pHandle);
   IS_NOT_NULL_POINTER(TVENC, pCaps);

   if (sem_get(pDriverInst->DriverSem, KAL_WAIT_FOREVER) != RC_OK)
   {
      return CNXT_TVENC_INTERNAL_ERROR;
   }

   /* Figure out unit number based on info in pCaps */
   uUnit = private_tvenc_get_unit_num(pCaps);

   /* Check bExclusive */
   pUnitInst = &(pDriverInst->pUnitInst[uUnit]);
   if ((pUnitInst->pFirstInst != NULL) && 
       (pUnitInst->bExclusive || pCaps->bExclusive))
   {
      sem_put(pDriverInst->DriverSem); 
      return CNXT_TVENC_NOT_AVAILABLE;
   }

   /* check if the requested output connections are all supported */
   if ( (pCaps->uConnections & ~(pUnitInst->pCaps->uConnections)) != 0)
   {
      sem_put(pDriverInst->DriverSem); 
      trace_new(TRACE_TVENC, "TVENC Opening: not all requested output connections are supported.  Return \n");
      return CNXT_TVENC_NOT_AVAILABLE;
   }

   /* check if the requested picture controls are all supported */
   if ( (pCaps->uControls & ~(pUnitInst->pCaps->uControls)) != 0)
   {
      sem_put(pDriverInst->DriverSem); 
      trace_new(TRACE_TVENC, "TVENC Opening: not all requested picture controls are supported.  Return \n");
      return CNXT_TVENC_NOT_AVAILABLE;
   }

   /* Create an instance */
   /* create an instance */
   if ( !CREATE_HANDLE(&(pDriverInst->pInstList), &pInst) )
   {
      *pHandle = NULL;
      sem_put(pDriverInst->DriverSem);
      return CNXT_TVENC_RESOURCE_ERROR;
   }

   /* add the instance into the list */
   pInst->uUnitNumber = uUnit;
   if (ADD_HANDLE (&pUnitInst->pFirstInst, pInst) == FALSE)
   {
      DESTROY_HANDLE(&(pDriverInst->pInstList), pInst);
      sem_put(pDriverInst->DriverSem); 
      return CNXT_TVENC_INTERNAL_ERROR;
   }

   pInst->Preface.pSelf = (CNXT_HANDLE_PREFACE*)pInst;
   pInst->pNotifyFn = pNotifyFn;    /* Use this fcn to notify appl of events */
   pInst->pUserData = pUserData;    /* Store data the inst needs */

   /* if first time unit opened, create an unit semaphore to prevent the unit instance */
   /* data from being modified by more then one users at the same time                 */   
   if( pUnitInst->pFirstInst->Preface.pNext == NULL )
   {
      pUnitInst->UnitSem = sem_create(1, NULL);
      if( pUnitInst->UnitSem == 0 )
      {
         REMOVE_HANDLE(&pUnitInst->pFirstInst, pInst); 
         DESTROY_HANDLE(&(pDriverInst->pInstList), pInst);
         sem_put(pDriverInst->DriverSem); 
         return CNXT_TVENC_RESOURCE_ERROR;
      }   
   }   

   /* set driver bExclusive field */
   pUnitInst->bExclusive = pCaps->bExclusive;

   *pHandle = (CNXT_TVENC_HANDLE)pInst;

   cnxt_tvenc_set_video_standard(*pHandle, CNXT_TVENC_PAL_B_WEUR); 
   
   trace_new(TRACE_TVENC, "TVENC: New handle returned 0x%08x\n", *pHandle);

   sem_put(pDriverInst->DriverSem); 
   return CNXT_TVENC_OK;
}

/********************************************************************/
/*  FUNCTION:    cnxt_tvenc_close                                   */
/*                                                                  */
/*  PARAMETERS:  Handle - a handle returned on a previous call to   */
/*                        cnxt_tvenc_open.                          */
/*                                                                  */
/*  DESCRIPTION: This function closes a handle freeing up any       */
/*               resources associated with it.                      */
/*                                                                  */
/*  RETURNS:     CNXT_TVENC_OK                                      */
/*               CNXT_TVENC_NOT_INIT                                */
/*               CNXT_TVENC_BAD_HANDLE                              */
/*               CNXT_TVENC_INTERNAL_ERROR                          */
/*                                                                  */
/*  CONTEXT:     Must be called in task context.                    */
/*                                                                  */
/********************************************************************/
CNXT_TVENC_STATUS cnxt_tvenc_close ( CNXT_TVENC_HANDLE Handle )
{
   CNXT_TVENC_INST *pInst = (CNXT_TVENC_INST*)Handle;
   CNXT_TVENC_STATUS eRet;
   
   IS_DRIVER_INITED(TVENC,pDriverInst->bInit);
   IS_VALID_HANDLE(TVENC, pInst, &(pDriverInst->pUnitInst[pInst->uUnitNumber].pFirstInst));

   if (sem_get(pDriverInst->DriverSem, KAL_WAIT_FOREVER) != RC_OK)
   {
      return CNXT_TVENC_INTERNAL_ERROR;
   }

   eRet = private_close( pInst );
   
   sem_put(pDriverInst->DriverSem); /* must be last line of routine! */
   
   return eRet;
}

/********************************************************************/
/*  FUNCTION:    cnxt_tvenc_set_picture_control                     */
/*                                                                  */
/*  PARAMETERS:  Handle - a handle returned on a previous call to   */
/*                        cnxt_tvenc_open.                          */
/*               Control - an enum specifying which picture control */
/*                        namely brightness, contrast, saturation,  */
/*                        hue and sharpness to set.                 */
/*               Value - an int which specifies the new value to set*/
/*                  The range of it is [-127, 128]                  */
/*                                                                  */
/*  DESCRIPTION: This function allows the caller to change the      */
/*               current settings of brightness, contrast, hue and  */
/*               saturation assuming the connected video encoder    */
/*               supports these.                                    */
/*                                                                  */
/*  RETURNS:     CNXT_TVENC_OK                                      */
/*               CNXT_TVENC_NOT_INIT                                */
/*               CNXT_TVENC_BAD_HANDLE                              */
/*               CNXT_TVENC_NOT_AVAILABLE                           */
/*               CNXT_TVENC_BAD_PARAMETER                           */
/*               CNXT_TVENC_INTERNAL_ERROR                          */
/*                                                                  */
/*  CONTEXT:     Must be called in task context.                    */
/*                                                                  */
/********************************************************************/
CNXT_TVENC_STATUS cnxt_tvenc_set_picture_control( CNXT_TVENC_HANDLE Handle, CNXT_TVENC_CONTROL Control, int32 Value )
{
   CNXT_TVENC_INST *pInst = (CNXT_TVENC_INST*)Handle;
   CNXT_TVENC_STATUS eRetcode;
   CNXT_TVENC_MODULE module;
   u_int32 uUnitNumber;
   CNXT_TVENC_UNIT_INST *pUnitInst;

   trace_new(TRACE_TVENC, "TVENC: Setting picture controls for handle 0x%08x\n", Handle);
   
   IS_DRIVER_INITED(TVENC,pDriverInst->bInit);
   IS_VALID_HANDLE(TVENC, pInst, &(pDriverInst->pUnitInst[pInst->uUnitNumber].pFirstInst));

   if ((Value < -128) || (Value > 127))
   {
      return CNXT_TVENC_BAD_PARAMETER;
   }

   if ( (uUnitNumber = pInst->uUnitNumber) > CNXT_TVENC_NUM_UNITS)
   {
      return CNXT_TVENC_INTERNAL_ERROR;
   }

   /* get the encoder type */
   module = pDriverInst->pUnitInst[uUnitNumber].module;

   /* get the unit instance */
   pUnitInst = &(pDriverInst->pUnitInst[uUnitNumber]);

   if (sem_get(pUnitInst->UnitSem, KAL_WAIT_FOREVER) != RC_OK)
   {
      return CNXT_TVENC_INTERNAL_ERROR;
   }

   /* Set the picture controls */
   if (function_table[module].set_picctl != NULL)
   {
      eRetcode = function_table[module].set_picctl(pUnitInst, Control, Value);
   }
   else
   {
      eRetcode = CNXT_TVENC_INTERNAL_ERROR;
   }
   
   sem_put(pUnitInst->UnitSem); 
   return(eRetcode);
}

/********************************************************************/
/*  FUNCTION:    cnxt_tvenc_get_picture_control                     */
/*                                                                  */
/*  PARAMETERS:  Handle - a handle returned on a previous call to   */
/*                        cnxt_tvenc_open.                          */
/*               pControls - pointer to a structure to receive the  */
/*                        current brightness, contrast, saturation  */
/*                        and hue settings.                         */
/*                                                                  */
/*  DESCRIPTION: This function allows the caller to query the       */
/*               current settings of brightness, contrast, hue and  */
/*               saturation assuming the connected video encoder    */
/*               supports these. The uFlags field in the pControls  */
/*               structure is set to show which parameters may be   */
/*               set by a call to cnxt_tvenc_set_picture_controls   */
/*                                                                  */
/*  RETURNS:     CNXT_TVENC_OK                                      */
/*               CNXT_TVENC_NOT_INIT                                */
/*               CNXT_TVENC_BAD_HANDLE                              */
/*               CNXT_TVENC_BAD_PARAMETER                           */
/*               CNXT_TVENC_NOT_AVAILABLE                           */
/*               CNXT_TVENC_INTERNAL_ERROR                          */
/*                                                                  */
/*  CONTEXT:     Must be called in task context.                    */
/*                                                                  */
/********************************************************************/
CNXT_TVENC_STATUS cnxt_tvenc_get_picture_control( CNXT_TVENC_HANDLE Handle, \
                                CNXT_TVENC_CONTROL Control, int32 *pValue )
{
   CNXT_TVENC_INST *pInst = (CNXT_TVENC_INST*)Handle;
   CNXT_TVENC_STATUS eRetcode;
   CNXT_TVENC_MODULE module;
   u_int32 uUnitNumber;
   CNXT_TVENC_UNIT_INST *pUnitInst;

   trace_new(TRACE_TVENC, "TVENC: Getting picture controls for handle 0x%08x\n", Handle);
   
   IS_DRIVER_INITED(TVENC,pDriverInst->bInit);
   IS_VALID_HANDLE(TVENC, pInst, &(pDriverInst->pUnitInst[pInst->uUnitNumber].pFirstInst));
   IS_NOT_NULL_POINTER(TVENC, pValue);

   if ( (uUnitNumber = pInst->uUnitNumber) > CNXT_TVENC_NUM_UNITS)
   {
      return CNXT_TVENC_INTERNAL_ERROR;
   }

   /* Set the controls if possible */
   /* get the encoder type */
   module = pDriverInst->pUnitInst[uUnitNumber].module;

   /* get the unit instance */
   pUnitInst = &(pDriverInst->pUnitInst[uUnitNumber]);

   /* Get the picture controls */
   if (function_table[module].get_picctl != NULL)
   {
      eRetcode = function_table[module].get_picctl(pUnitInst, Control, pValue);
   }
   else
   {
      eRetcode = CNXT_TVENC_INTERNAL_ERROR;
   }

   return(eRetcode);
}

/********************************************************************/
/*  FUNCTION:    cnxt_tvenc_set_video_standard                      */
/*                                                                  */
/*  PARAMETERS:  Handle - a handle returned on a previous call to   */
/*                        cnxt_tvenc_open.                          */
/*               Standard - output video format.                    */
/*                           One of the following:                  */
/*                              CNXT_TVENC_NTSC_M,                  */
/*                              CNXT_TVENC_NTSC_JAPAN,              */   
/*                              CNXT_TVENC_PAL_B_ITALY,             */
/*                              CNXT_TVENC_PAL_B_WEUR,              */ 
/*                              CNXT_TVENC_PAL_B_AUS,               */ 
/*                              CNXT_TVENC_PAL_B_NZ,                */ 
/*                              CNXT_TVENC_PAL_I,                   */ 
/*                              CNXT_TVENC_SECAM_L,                 */
/*                              CNXT_TVENC_SECAM_D                  */ 
/*                                                                  */
/*  DESCRIPTION: This function sets the connected video encoder to  */
/*               the input output video standard                    */
/*                                                                  */
/*  RETURNS:     CNXT_TVENC_OK                                      */
/*               CNXT_TVENC_NOT_INIT                                */
/*               CNXT_TVENC_BAD_HANDLE                              */
/*               CNXT_TVENC_BAD_PARAMETER                           */
/*               CNXT_TVENC_INTERNAL_ERROR                          */
/*                                                                  */
/*  CONTEXT:     Must be called in task context.                    */
/*                                                                  */
/********************************************************************/
CNXT_TVENC_STATUS cnxt_tvenc_set_video_standard( CNXT_TVENC_HANDLE Handle, CNXT_TVENC_VIDEO_STANDARD Standard )
{
   CNXT_TVENC_INST *pInst = (CNXT_TVENC_INST*)Handle;
   CNXT_TVENC_STATUS eRetcode;
   CNXT_TVENC_MODULE module;
   u_int32 uUnitNumber;
   CNXT_TVENC_UNIT_INST *pUnitInst;

   trace_new(TRACE_TVENC, "TVENC: Setting output video standard for handle 0x%08x\n", Handle);
   
   IS_DRIVER_INITED(TVENC,pDriverInst->bInit);
   IS_VALID_HANDLE(TVENC, pInst, &(pDriverInst->pUnitInst[pInst->uUnitNumber].pFirstInst));

   /* check if the input video standard is valid */
   if( (Standard <= CNXT_TVENC_STANDARD_INVALID) || (Standard >= CNXT_TVENC_STANDARD_MAX) ) 
   {
      trace_new( TRACE_TVENC, "cnxt_tvenc_set_video_standard: invalid video standard %d was specified\n", Standard );
      return CNXT_TVENC_BAD_PARAMETER;
   }

   if ( (uUnitNumber = pInst->uUnitNumber) > CNXT_TVENC_NUM_UNITS)
   {
      return CNXT_TVENC_INTERNAL_ERROR;
   }

   /* get the encoder type */
   module = pDriverInst->pUnitInst[uUnitNumber].module;

   /* get the unit instance */
   pUnitInst = &(pDriverInst->pUnitInst[uUnitNumber]);

   if (sem_get(pUnitInst->UnitSem, KAL_WAIT_FOREVER) != RC_OK)
   {
      return CNXT_TVENC_INTERNAL_ERROR;
   }

   /* Set the video standard for the connected TV encoder */
   if (function_table[module].set_standard != NULL)
   {
      eRetcode = function_table[module].set_standard(pUnitInst, Standard);
   }
   else
   {
      eRetcode = CNXT_TVENC_INTERNAL_ERROR;
   }

   pUnitInst->VideoStandard = Standard;

   /******************************************************/
   /* Enable the outputs that are configured for the IRD */
   /******************************************************/
   /* Set the video standard for the connected TV encoder */
   if (function_table[module].set_connection != NULL)
   {
      eRetcode = function_table[module].set_connection(pUnitInst, pUnitInst->pCaps->uConnections);
   }
   else
   {
      eRetcode = CNXT_TVENC_INTERNAL_ERROR;
   }

   if( (Standard == CNXT_TVENC_NTSC_M) || (Standard == CNXT_TVENC_NTSC_JAPAN) )
   {
      gnOsdMaxHeight = 480;
   }
   else
   {
      gnOsdMaxHeight = 576;
   }

   gnOsdMaxWidth = 720;

   #ifndef OPENTV
   if (gnOsdMaxHeight != 480)
      CNXT_SET_VAL(glpDrmControl, DRM_CONTROL_OUTPUT_FORMAT_MASK, 1);
   else
      CNXT_SET_VAL(glpDrmControl, DRM_CONTROL_OUTPUT_FORMAT_MASK, 0);
   #else
   /* CX22490/1/6 has a bug that causes upscaled MPEG stills to be badly filtered */
   /* (they flicker) if this bit is set to 1. We need to set it to 1 when playing */
   /* motion video but the video driver turns it on and off as necessary. We also */
   /* need to set it correctly when doing aspect ratio conversion so this is why  */
   /* this code is OpenTV specific - it doesn't support aspect ratio conversion   */
   CNXT_SET_VAL(glpDrmControl, DRM_CONTROL_OUTPUT_FORMAT_MASK, 0);
   #endif

   CNXT_SET_VAL(glpDrmScreenStart, DRM_SCREEN_START_FIRST_ACTIVE_PIXEL_MASK, pUnitInst->uHBlank);
   CNXT_SET_VAL(glpDrmScreenStart, DRM_SCREEN_START_FIELD2_ACTIVE_LINE_MASK, pUnitInst->uVBlank);
   CNXT_SET_VAL(glpDrmScreenStart, DRM_SCREEN_START_FIELD1_ACTIVE_LINE_MASK, pUnitInst->uVBlank);
   CNXT_SET_VAL(glpDrmScreenEnd, DRM_SCREEN_END_LAST_ACTIVE_PIXEL_MASK,
      pUnitInst->uHBlank + (2*OSD_MAX_WIDTH) - 1);
   CNXT_SET_VAL(glpDrmScreenEnd, DRM_SCREEN_END_FIELD2_LAST_LINE_MASK,
      pUnitInst->uVBlank + (gnOsdMaxHeight/2) - 1);
   CNXT_SET_VAL(glpDrmScreenEnd, DRM_SCREEN_END_FIELD1_LAST_LINE_MASK,
      pUnitInst->uVBlank + (gnOsdMaxHeight/2) - 1);

   pDriverInst->pUnitInst[uUnitNumber].uHBlankInitial = pUnitInst->uHBlank;
   pDriverInst->pUnitInst[uUnitNumber].uVBlankInitial = pUnitInst->uVBlank;
   
   /* Set the X and Y start positions for the connected TV encoder */
   if (function_table[module].set_posoffset != NULL)
   {
      eRetcode = function_table[module].set_posoffset(pUnitInst, INITIAL_X_OFFSET, INITIAL_Y_OFFSET);
   }
   else
   {
      eRetcode = CNXT_TVENC_INTERNAL_ERROR;
   }

   vidSetHWBuffSize();
   
   /* notify all clients of reset of the driver */
   cnxt_tvenc_notify_all_clients(CNXT_TVENC_EVENT_VIDEO_STANDARD_SET);

   sem_put(pUnitInst->UnitSem); 
   return(eRetcode);
}

/********************************************************************/
/*  FUNCTION:    cnxt_tvenc_get_output_video_standard               */
/*                                                                  */
/*  PARAMETERS:  Handle - a handle returned on a previous call to   */
/*                        cnxt_tvenc_open.                          */
/*               pStandard - pointer to a structure to receive      */
/*                           output video format.                   */
/*               Return values should be one of the following:      */
/*                              CNXT_TVENC_NTSC_M,                  */
/*                              CNXT_TVENC_NTSC_JAPAN,              */   
/*                              CNXT_TVENC_PAL_B_ITALY,             */
/*                              CNXT_TVENC_PAL_B_WEUR,              */ 
/*                              CNXT_TVENC_PAL_B_AUS,               */ 
/*                              CNXT_TVENC_PAL_B_NZ,                */ 
/*                              CNXT_TVENC_PAL_I,                   */ 
/*                              CNXT_TVENC_SECAM_L,                 */
/*                              CNXT_TVENC_SECAM_D                  */ 
/*                                                                  */
/*  DESCRIPTION: This function returns the output video standard    */
/*               from the connected video encoder.                  */
/*                                                                  */
/*  RETURNS:     CNXT_TVENC_OK                                      */
/*               CNXT_TVENC_NOT_INIT                                */
/*               CNXT_TVENC_BAD_HANDLE                              */
/*               CNXT_TVENC_BAD_PARAMETER                           */
/*               CNXT_TVENC_INTERNAL_ERROR                          */
/*                                                                  */
/*  CONTEXT:     Must be called in task context.                    */
/*                                                                  */
/********************************************************************/
CNXT_TVENC_STATUS cnxt_tvenc_get_video_standard( CNXT_TVENC_HANDLE Handle, CNXT_TVENC_VIDEO_STANDARD *pStandard )
{
   CNXT_TVENC_INST *pInst = (CNXT_TVENC_INST*)Handle;
   CNXT_TVENC_STATUS eRetcode;
   CNXT_TVENC_MODULE module;
   u_int32 uUnitNumber;
   CNXT_TVENC_UNIT_INST *pUnitInst;

   trace_new(TRACE_TVENC, "TVENC: Getting output video standard for handle 0x%08x\n", Handle);
   
   IS_DRIVER_INITED(TVENC,pDriverInst->bInit);
   IS_VALID_HANDLE(TVENC, pInst, &(pDriverInst->pUnitInst[pInst->uUnitNumber].pFirstInst));
   IS_NOT_NULL_POINTER(TVENC, pStandard);

   if ( (uUnitNumber = pInst->uUnitNumber) > CNXT_TVENC_NUM_UNITS)
   {
      return CNXT_TVENC_INTERNAL_ERROR;
   }

   /* get the encoder type */
   module = pDriverInst->pUnitInst[uUnitNumber].module;

   /* get the unit instance */
   pUnitInst = &(pDriverInst->pUnitInst[uUnitNumber]);

   /* Get the video standard */
   if (function_table[module].set_standard != NULL)
   {
      eRetcode = function_table[module].get_standard(pUnitInst, pStandard);
   }
   else
   {
      eRetcode = CNXT_TVENC_INTERNAL_ERROR;
   }

   return(eRetcode);
}

/********************************************************************/
/*  FUNCTION:    cnxt_tvenc_set_display_position                    */
/*                                                                  */
/*  PARAMETERS:  Handle - a handle returned on a previous call to   */
/*                        cnxt_tvenc_open.                          */
/*               Xoffset - Horizontal offset from default position  */
/*               Yoffset - Vertical offset from default position    */  
/*                                                                  */                                                                
/*  DESCRIPTION: This function adjusts the display origin by a small*/
/*               amount in the vertical and horizontal directions.  */
/*                                                                  */
/*  RETURNS:     CNXT_TVENC_OK                                      */
/*               CNXT_TVENC_NOT_INIT                                */
/*               CNXT_TVENC_BAD_HANDLE                              */
/*               CNXT_TVENC_BAD_PARAMETER                           */
/*               CNXT_TVENC_INTERNAL_ERROR                          */
/*                                                                  */
/*  CONTEXT:     Must be called in task context.                    */
/*                                                                  */
/********************************************************************/
CNXT_TVENC_STATUS cnxt_tvenc_set_display_position_offset( CNXT_TVENC_HANDLE Handle, int8 XOffset, int8 YOffset )
{
   CNXT_TVENC_INST *pInst = (CNXT_TVENC_INST*)Handle;
   CNXT_TVENC_STATUS eRetcode;
   CNXT_TVENC_MODULE module;
   u_int32 uUnitNumber;
   CNXT_TVENC_UNIT_INST *pUnitInst;

   trace_new(TRACE_TVENC, "TVENC: Setting the screen start positions for handle 0x%08x\n", Handle);
   
   IS_DRIVER_INITED(TVENC,pDriverInst->bInit);
   IS_VALID_HANDLE(TVENC, pInst, &(pDriverInst->pUnitInst[pInst->uUnitNumber].pFirstInst));

   /* Make sure we are passed valid values */
   if((abs(XOffset) > MAX_SCREEN_POSITION_X_OFFSET) ||
      (abs(YOffset) > MAX_SCREEN_POSITION_Y_OFFSET))
   {
      trace_new(TRACE_TVENC, "TVENC: Attempt to set invalid screen offset (%d, %d). Max is (%d, %d)\n",
                XOffset, YOffset, MAX_SCREEN_POSITION_X_OFFSET, MAX_SCREEN_POSITION_Y_OFFSET);
      return CNXT_TVENC_BAD_PARAMETER;
   }

   if ( (uUnitNumber = pInst->uUnitNumber) > CNXT_TVENC_NUM_UNITS)
   {
      return CNXT_TVENC_INTERNAL_ERROR;
   }

   /* get the encoder type */
   module = pDriverInst->pUnitInst[uUnitNumber].module;

   /* get the unit instance */
   pUnitInst = &(pDriverInst->pUnitInst[uUnitNumber]);

   if (sem_get(pUnitInst->UnitSem, KAL_WAIT_FOREVER) != RC_OK)
   {
      return CNXT_TVENC_INTERNAL_ERROR;
   }

   /* Set the X and Y screen start positions for the connected TV encoder */
   if (function_table[module].set_posoffset != NULL)
   {
      eRetcode = function_table[module].set_posoffset(pUnitInst, XOffset, YOffset);
   }
   else
   {
      eRetcode = CNXT_TVENC_INTERNAL_ERROR;
   }

   sem_put(pUnitInst->UnitSem); 
   return(eRetcode);

}

/********************************************************************/
/*  FUNCTION:    cnxt_tvenc_get_display_position                    */
/*                                                                  */
/*  PARAMETERS:  Handle - a handle returned on a previous call to   */
/*                        cnxt_tvenc_open.                          */
/*               pXStart - pointer to the horizontal screen start   */
/*                         position                                 */
/*               pYStart - pointer to the vertical screen start     */
/*                         position                                 */  
/*                                                                  */                                                                
/*  DESCRIPTION: Query the current X and Y start positions values   */
/*               for the display.                                   */
/*                                                                  */
/*  RETURNS:     CNXT_TVENC_OK                                      */
/*               CNXT_TVENC_NOT_INIT                                */
/*               CNXT_TVENC_BAD_HANDLE                              */
/*               CNXT_TVENC_BAD_PARAMETER                           */
/*               CNXT_TVENC_INTERNAL_ERROR                          */
/*                                                                  */
/*  CONTEXT:     Must be called in task context.                    */
/*                                                                  */
/********************************************************************/
CNXT_TVENC_STATUS cnxt_tvenc_get_display_position_offset( CNXT_TVENC_HANDLE Handle, int8 *pXOffset, int8 *pYOffset )
{
   CNXT_TVENC_INST *pInst = (CNXT_TVENC_INST*)Handle;
   CNXT_TVENC_STATUS eRetcode;
   CNXT_TVENC_MODULE module;
   u_int32 uUnitNumber;
   CNXT_TVENC_UNIT_INST *pUnitInst;

   trace_new(TRACE_TVENC, "TVENC: Getting screen start positions for handle 0x%08x\n", Handle);
   
   IS_DRIVER_INITED(TVENC,pDriverInst->bInit);
   IS_VALID_HANDLE(TVENC, pInst, &(pDriverInst->pUnitInst[pInst->uUnitNumber].pFirstInst));
   IS_NOT_NULL_POINTER(TVENC, pXOffset);
   IS_NOT_NULL_POINTER(TVENC, pYOffset);
                                                                    
   if ( (uUnitNumber = pInst->uUnitNumber) > CNXT_TVENC_NUM_UNITS)
   {
      return CNXT_TVENC_INTERNAL_ERROR;
   }

   /* get the encoder type */
   module = pDriverInst->pUnitInst[uUnitNumber].module;

   /* get the unit instance */
   pUnitInst = &(pDriverInst->pUnitInst[uUnitNumber]);

   /* Get the screen start X and Y positions */
   if (function_table[module].get_posoffset != NULL)
   {
      eRetcode = function_table[module].get_posoffset(pUnitInst, pXOffset, pYOffset);
   }
   else
   {
      eRetcode = CNXT_TVENC_INTERNAL_ERROR;
   }
   
   return(eRetcode);

}

/********************************************************************/
/*  FUNCTION:    cnxt_tvenc_set_output_connection                   */
/*                                                                  */
/*  PARAMETERS:  Handle - a handle returned on a previous call to   */
/*                        cnxt_tvenc_open.                          */
/*               uConnection - Video output connection type.        */
/*                  It should be a logic OR of the following:       */
/*                      VIDEO_OUTPUT_CVBS                           */
/*                      VIDEO_OUTPUT_RGB                            */
/*                      VIDEO_OUTPUT_YC                             */
/*                                                                  */                                                                
/*  DESCRIPTION: This function sets the video output connection.    */
/*                                                                  */
/*  RETURNS:     CNXT_TVENC_OK                                      */
/*               CNXT_TVENC_NOT_INIT                                */
/*               CNXT_TVENC_NOT_AVAILABLE                           */
/*               CNXT_TVENC_BAD_HANDLE                              */
/*               CNXT_TVENC_BAD_PARAMETER                           */
/*               CNXT_TVENC_INTERNAL_ERROR                          */
/*                                                                  */
/*  CONTEXT:     Must be called in task context.                    */
/*                                                                  */
/********************************************************************/
CNXT_TVENC_STATUS cnxt_tvenc_set_output_connection( CNXT_TVENC_HANDLE Handle, u_int8 uConnection )
{
   CNXT_TVENC_INST *pInst = (CNXT_TVENC_INST*)Handle;
   CNXT_TVENC_STATUS eRetcode;
   CNXT_TVENC_MODULE module;
   u_int32 uUnitNumber;
   CNXT_TVENC_UNIT_INST *pUnitInst;

   trace_new(TRACE_TVENC, "TVENC: Setting the video output connection for handle 0x%08x\n", Handle);
   
   IS_DRIVER_INITED(TVENC,pDriverInst->bInit);
   IS_VALID_HANDLE(TVENC, pInst, &(pDriverInst->pUnitInst[pInst->uUnitNumber].pFirstInst));

   /* uConnection has to be one of the three output connection types */
   if ( (uConnection != 0) && (uConnection & (VIDEO_OUTPUT_CVBS | VIDEO_OUTPUT_RGB | VIDEO_OUTPUT_YC)) == 0 )
   {
      return CNXT_TVENC_BAD_PARAMETER;
   }
    
   if ( (uUnitNumber = pInst->uUnitNumber) > CNXT_TVENC_NUM_UNITS)
   {
      return CNXT_TVENC_INTERNAL_ERROR;
   }

   /* get the encoder type */
   module = pDriverInst->pUnitInst[uUnitNumber].module;

   /* get the unit instance */
   pUnitInst = &(pDriverInst->pUnitInst[uUnitNumber]);

   /* check if the requested output connections are all supported */
   if ( (uConnection & ~(pUnitInst->pCaps->uConnections)) != 0)
   {
      trace_new(TRACE_TVENC, "TVENC: Invalid video connection type %d \n", uConnection);
      return(CNXT_TVENC_NOT_AVAILABLE);
   }

   if (sem_get(pUnitInst->UnitSem, KAL_WAIT_FOREVER) != RC_OK)
   {
      return CNXT_TVENC_INTERNAL_ERROR;
   }

   /* Set the video standard for the connected TV encoder */
   if (function_table[module].set_connection != NULL)
   {
      eRetcode = function_table[module].set_connection(pUnitInst, uConnection);
   }
   else
   {
      eRetcode = CNXT_TVENC_INTERNAL_ERROR;
   }
       
   sem_put(pUnitInst->UnitSem); 
   return(eRetcode);
}

/********************************************************************/
/*  FUNCTION:    cnxt_tvenc_get_output_connection                   */
/*                                                                  */
/*  PARAMETERS:  Handle - a handle returne on a previous call to    */
/*                        cnxt_tvenc_open.                          */
/*               puConnection - Pointer to receive the video output */
/*                    connection type. The return values should be  */
/*                    a logic OR of the following:                  */
/*                      VIDEO_OUTPUT_CVBS                           */
/*                      VIDEO_OUTPUT_RGB                            */
/*                      VIDEO_OUTPUT_YC                             */
/*                                                                  */                                                                
/*  DESCRIPTION: This function sets the video output connection.    */
/*                                                                  */
/*  RETURNS:     CNXT_TVENC_OK                                      */
/*               CNXT_TVENC_NOT_INIT                                */
/*               CNXT_TVENC_NOT_AVAILABLE                           */
/*               CNXT_TVENC_BAD_HANDLE                              */
/*               CNXT_TVENC_BAD_PARAMETER                           */
/*               CNXT_TVENC_INTERNAL_ERROR                          */
/*                                                                  */
/*  CONTEXT:     Must be called in task context.                    */
/*                                                                  */
/********************************************************************/
CNXT_TVENC_STATUS cnxt_tvenc_get_output_connection( CNXT_TVENC_HANDLE Handle, u_int8 *puConnection )
{
   CNXT_TVENC_INST *pInst = (CNXT_TVENC_INST*)Handle;
   CNXT_TVENC_STATUS eRetcode;
   CNXT_TVENC_MODULE module;
   u_int32 uUnitNumber;
   CNXT_TVENC_UNIT_INST *pUnitInst;

   trace_new(TRACE_TVENC, "TVENC: Getting video connection types for handle 0x%08x\n", Handle);
   
   IS_DRIVER_INITED(TVENC,pDriverInst->bInit);
   IS_VALID_HANDLE(TVENC, pInst, &(pDriverInst->pUnitInst[pInst->uUnitNumber].pFirstInst));
   IS_NOT_NULL_POINTER(TVENC, puConnection);

   if ( (uUnitNumber = pInst->uUnitNumber) > CNXT_TVENC_NUM_UNITS)
   {
      return CNXT_TVENC_INTERNAL_ERROR;
   }

   /* get the encoder type */
   module = pDriverInst->pUnitInst[uUnitNumber].module;

   /* get the unit instance */
   pUnitInst = &(pDriverInst->pUnitInst[uUnitNumber]);

   /* Get the video output connection types */
   if (function_table[module].get_connection != NULL)
   {
      eRetcode = function_table[module].get_connection(pUnitInst, puConnection);
   }
   else
   {
      eRetcode = CNXT_TVENC_INTERNAL_ERROR;
   }
   
   return(eRetcode);

}

/********************************************************************/
/*  FUNCTION:    cnxt_tvenc_video_timing_reset                      */
/*                                                                  */
/*  PARAMETERS:  Handle - a handle returne on a previous call to    */
/*                        cnxt_tvenc_open.                          */
/*                                                                  */                                                                
/*  DESCRIPTION: This function resets the video timing register.    */
/*                                                                  */
/*  RETURNS:     CNXT_TVENC_OK                                      */
/*               CNXT_TVENC_NOT_INIT                                */
/*               CNXT_TVENC_NOT_AVAILABLE                           */
/*               CNXT_TVENC_BAD_HANDLE                              */
/*               CNXT_TVENC_INTERNAL_ERROR                          */
/*                                                                  */
/*  CONTEXT:     Must be called in task context.                    */
/*                                                                  */
/********************************************************************/
CNXT_TVENC_STATUS cnxt_tvenc_video_timing_reset( CNXT_TVENC_HANDLE Handle )
{
   CNXT_TVENC_INST *pInst = (CNXT_TVENC_INST*)Handle;
   CNXT_TVENC_STATUS eRetcode;
   CNXT_TVENC_MODULE module;
   u_int32 uUnitNumber;
   CNXT_TVENC_UNIT_INST *pUnitInst;

   IS_DRIVER_INITED(TVENC,pDriverInst->bInit);
   IS_VALID_HANDLE(TVENC, pInst, &(pDriverInst->pUnitInst[pInst->uUnitNumber].pFirstInst));
    
   if ( (uUnitNumber = pInst->uUnitNumber) > CNXT_TVENC_NUM_UNITS)
   {
      return CNXT_TVENC_INTERNAL_ERROR;
   }

   /* get the encoder type */
   module = pDriverInst->pUnitInst[uUnitNumber].module;

   /* get the unit instance */
   pUnitInst = &(pDriverInst->pUnitInst[uUnitNumber]);

   if (sem_get(pUnitInst->UnitSem, KAL_WAIT_FOREVER) != RC_OK)
   {
      return CNXT_TVENC_INTERNAL_ERROR;
   }

   /* Get the video output connection types */
   if (function_table[module].vtiming_reset != NULL)
   {
      eRetcode = function_table[module].vtiming_reset(pUnitInst);
   }
   else
   {
      eRetcode = CNXT_TVENC_INTERNAL_ERROR;
   }

   sem_put(pUnitInst->UnitSem); /* must be last line of routine! */

   return(eRetcode);
}

/********************************************************************/
/*  FUNCTION:    cnxt_tvenc_ttx_set_lines                           */
/*                                                                  */
/*  PARAMETERS:  Handle - a handle returne on a previous call to    */
/*                        cnxt_tvenc_open.                          */
/*                                                                  */                                                                
/*  DESCRIPTION: This function specifies which output lines in the  */
/*               VBI will be occupied by teletext data.             */
/*                                                                  */
/*  RETURNS:     CNXT_TVENC_OK                                      */
/*               CNXT_TVENC_NOT_INIT                                */
/*               CNXT_TVENC_NOT_AVAILABLE                           */
/*               CNXT_TVENC_BAD_HANDLE                              */
/*               CNXT_TVENC_BAD_PARAMETER                           */
/*               CNXT_TVENC_INTERNAL_ERROR                          */
/*                                                                  */
/*  CONTEXT:     Must be called in task context.                    */
/*                                                                  */
/********************************************************************/
CNXT_TVENC_STATUS cnxt_tvenc_ttx_set_lines( CNXT_TVENC_HANDLE Handle, u_int32 uField1ActiveLines, u_int32 uField2ActiveLines )
{
   CNXT_TVENC_INST *pInst = (CNXT_TVENC_INST*)Handle;
   CNXT_TVENC_STATUS eRetcode;
   CNXT_TVENC_MODULE module;
   u_int32 uUnitNumber;
   CNXT_TVENC_UNIT_INST *pUnitInst;

   IS_DRIVER_INITED(TVENC,pDriverInst->bInit);
   IS_VALID_HANDLE(TVENC, pInst, &(pDriverInst->pUnitInst[pInst->uUnitNumber].pFirstInst));

   if ( (uUnitNumber = pInst->uUnitNumber) > CNXT_TVENC_NUM_UNITS)
   {
      return CNXT_TVENC_INTERNAL_ERROR;
   }

   /* get the encoder type */
   module = pDriverInst->pUnitInst[uUnitNumber].module;

   /* get the unit instance */
   pUnitInst = &(pDriverInst->pUnitInst[uUnitNumber]);

   /* teletext is not supported for all NTSC and SECAM standards */
   if( (pUnitInst->VideoStandard == CNXT_TVENC_NTSC_M) \
    || (pUnitInst->VideoStandard == CNXT_TVENC_NTSC_JAPAN) \
    || (pUnitInst->VideoStandard == CNXT_TVENC_SECAM_L) \
    || (pUnitInst->VideoStandard == CNXT_TVENC_SECAM_D) )
   {
      return CNXT_TVENC_NOT_AVAILABLE;
   }   

   if (sem_get(pUnitInst->UnitSem, KAL_WAIT_FOREVER) != RC_OK)
   {
      return CNXT_TVENC_INTERNAL_ERROR;
   }

   /* Set the specified active and inactive teletext lines */
   if (function_table[module].ttx_set_lines != NULL)
   {
      eRetcode = function_table[module].ttx_set_lines(pUnitInst, uField1ActiveLines, uField2ActiveLines);
   }
   else
   {
      eRetcode = CNXT_TVENC_INTERNAL_ERROR;
   }

   sem_put(pUnitInst->UnitSem); /* must be last line of routine! */

   return(eRetcode);
}


/********************************************************************/
/*  FUNCTION:    cnxt_tvenc_ttx_enable                              */
/*                                                                  */
/*  PARAMETERS:  Handle - a handle returne on a previous call to    */
/*                        cnxt_tvenc_open.                          */
/*                                                                  */                                                                
/*  DESCRIPTION: This function enables teletext encoding.           */
/*                                                                  */
/*  RETURNS:     CNXT_TVENC_OK                                      */
/*               CNXT_TVENC_NOT_INIT                                */
/*               CNXT_TVENC_NOT_AVAILABLE                           */
/*               CNXT_TVENC_BAD_HANDLE                              */
/*               CNXT_TVENC_INTERNAL_ERROR                          */
/*                                                                  */
/*  CONTEXT:     Must be called in task context.                    */
/*                                                                  */
/********************************************************************/
CNXT_TVENC_STATUS cnxt_tvenc_ttx_enable( CNXT_TVENC_HANDLE Handle )
{
   CNXT_TVENC_INST *pInst = (CNXT_TVENC_INST*)Handle;
   CNXT_TVENC_STATUS eRetcode;
   CNXT_TVENC_MODULE module;
   u_int32 uUnitNumber;
   CNXT_TVENC_UNIT_INST *pUnitInst;

   IS_DRIVER_INITED(TVENC,pDriverInst->bInit);
   IS_VALID_HANDLE(TVENC, pInst, &(pDriverInst->pUnitInst[pInst->uUnitNumber].pFirstInst));

   if ( (uUnitNumber = pInst->uUnitNumber) > CNXT_TVENC_NUM_UNITS)
   {
      return CNXT_TVENC_INTERNAL_ERROR;
   }

   /* get the encoder type */
   module = pDriverInst->pUnitInst[uUnitNumber].module;

   /* get the unit instance */
   pUnitInst = &(pDriverInst->pUnitInst[uUnitNumber]);

   /* teletext is not supported for all NTSC and SECAM standards */
   if( (pUnitInst->VideoStandard == CNXT_TVENC_NTSC_M) \
    || (pUnitInst->VideoStandard == CNXT_TVENC_NTSC_JAPAN) \
    || (pUnitInst->VideoStandard == CNXT_TVENC_SECAM_L) \
    || (pUnitInst->VideoStandard == CNXT_TVENC_SECAM_D) )
   {
      return CNXT_TVENC_NOT_AVAILABLE;
   }   

   if (sem_get(pUnitInst->UnitSem, KAL_WAIT_FOREVER) != RC_OK)
   {
      return CNXT_TVENC_INTERNAL_ERROR;
   }

      /* set the teletext encoding */
   if (function_table[module].ttx_enable != NULL)
   {
      eRetcode = function_table[module].ttx_enable(pUnitInst);
   }
   else
   {
      eRetcode = CNXT_TVENC_INTERNAL_ERROR;
   }

   sem_put(pUnitInst->UnitSem); /* must be last line of routine! */

   return(eRetcode);
}

/********************************************************************/
/*  FUNCTION:    cnxt_tvenc_ttx_disable                             */
/*                                                                  */
/*  PARAMETERS:  Handle - a handle returne on a previous call to    */
/*                        cnxt_tvenc_open.                          */
/*                                                                  */                                                                
/*  DESCRIPTION: This function disables teletext encoding.          */
/*                                                                  */
/*  RETURNS:     CNXT_TVENC_OK                                      */
/*               CNXT_TVENC_NOT_INIT                                */
/*               CNXT_TVENC_NOT_AVAILABLE                           */
/*               CNXT_TVENC_BAD_HANDLE                              */
/*               CNXT_TVENC_INTERNAL_ERROR                          */
/*                                                                  */
/*  CONTEXT:     Must be called in task context.                    */
/*                                                                  */
/********************************************************************/
CNXT_TVENC_STATUS cnxt_tvenc_ttx_disable( CNXT_TVENC_HANDLE Handle )
{
   CNXT_TVENC_INST *pInst = (CNXT_TVENC_INST*)Handle;
   CNXT_TVENC_STATUS eRetcode;
   CNXT_TVENC_MODULE module;
   u_int32 uUnitNumber;
   CNXT_TVENC_UNIT_INST *pUnitInst;

   IS_DRIVER_INITED(TVENC,pDriverInst->bInit);
   IS_VALID_HANDLE(TVENC, pInst, &(pDriverInst->pUnitInst[pInst->uUnitNumber].pFirstInst));

   if ( (uUnitNumber = pInst->uUnitNumber) > CNXT_TVENC_NUM_UNITS)
   {
      return CNXT_TVENC_INTERNAL_ERROR;
   }

   /* get the encoder type */
   module = pDriverInst->pUnitInst[uUnitNumber].module;

   /* get the unit instance */
   pUnitInst = &(pDriverInst->pUnitInst[uUnitNumber]);

   /* teletext is not supported for all NTSC and SECAM standards */
   if( (pUnitInst->VideoStandard == CNXT_TVENC_NTSC_M) \
    || (pUnitInst->VideoStandard == CNXT_TVENC_NTSC_JAPAN) \
    || (pUnitInst->VideoStandard == CNXT_TVENC_SECAM_L) \
    || (pUnitInst->VideoStandard == CNXT_TVENC_SECAM_D) )
   {
      return CNXT_TVENC_NOT_AVAILABLE;
   }
     
   if (sem_get(pUnitInst->UnitSem, KAL_WAIT_FOREVER) != RC_OK)
   {
      return CNXT_TVENC_INTERNAL_ERROR;
   }
    
      /* Disable teletext encoding */
   if (function_table[module].ttx_disable != NULL)
   {
      eRetcode = function_table[module].ttx_disable(pUnitInst);
   }
   else
   {
      eRetcode = CNXT_TVENC_INTERNAL_ERROR;
   }

   sem_put(pUnitInst->UnitSem); /* must be last line of routine! */

   return(eRetcode);
}

/********************************************************************/
/*  FUNCTION:    cnxt_tvenc_cc_enable                               */
/*                                                                  */
/*  PARAMETERS:  Handle - a handle returne on a previous call to    */
/*                        cnxt_tvenc_open.                          */
/*                                                                  */                                                                
/*  DESCRIPTION: This function sets the appropriate encoder         */
/*               registers in the TV encoder to enable closed       */
/*               captioning.                                        */
/*                                                                  */
/*  RETURNS:     CNXT_TVENC_OK                                      */
/*               CNXT_TVENC_NOT_INIT                                */
/*               CNXT_TVENC_NOT_AVAILABLE                           */
/*               CNXT_TVENC_BAD_HANDLE                              */
/*               CNXT_TVENC_INTERNAL_ERROR                          */
/*                                                                  */
/*  CONTEXT:     Must be called in task context.                    */
/*                                                                  */
/********************************************************************/
CNXT_TVENC_STATUS cnxt_tvenc_cc_enable( CNXT_TVENC_HANDLE Handle )
{
   CNXT_TVENC_INST *pInst = (CNXT_TVENC_INST*)Handle;
   CNXT_TVENC_STATUS eRetcode;
   CNXT_TVENC_MODULE module;
   u_int32 uUnitNumber;
   CNXT_TVENC_UNIT_INST *pUnitInst;

   IS_DRIVER_INITED(TVENC,pDriverInst->bInit);
   IS_VALID_HANDLE(TVENC, pInst, &(pDriverInst->pUnitInst[pInst->uUnitNumber].pFirstInst));

   if ( (uUnitNumber = pInst->uUnitNumber) > CNXT_TVENC_NUM_UNITS)
   {
      return CNXT_TVENC_INTERNAL_ERROR;
   }

   /* get the encoder type */
   module = pDriverInst->pUnitInst[uUnitNumber].module;

   /* get the unit instance */
   pUnitInst = &(pDriverInst->pUnitInst[uUnitNumber]);

   /* teletext is not supported for all PAL and SECAM standards */
   if( (pUnitInst->VideoStandard == CNXT_TVENC_PAL_B_ITALY) \
    || (pUnitInst->VideoStandard == CNXT_TVENC_PAL_B_WEUR) \
    || (pUnitInst->VideoStandard == CNXT_TVENC_PAL_B_AUS) \
    || (pUnitInst->VideoStandard == CNXT_TVENC_PAL_B_NZ) \
    || (pUnitInst->VideoStandard == CNXT_TVENC_PAL_I) \
    || (pUnitInst->VideoStandard == CNXT_TVENC_SECAM_L) \
    || (pUnitInst->VideoStandard == CNXT_TVENC_SECAM_D) )
   {
      return CNXT_TVENC_NOT_AVAILABLE;
   }   

   if (sem_get(pUnitInst->UnitSem, KAL_WAIT_FOREVER) != RC_OK)
   {
      return CNXT_TVENC_INTERNAL_ERROR;
   }

      /* Enable closed captioning */
   if (function_table[module].cc_enable != NULL)
   {
      eRetcode = function_table[module].cc_enable(pUnitInst);
   }
   else
   {
      eRetcode = CNXT_TVENC_INTERNAL_ERROR;
   }

   /* enable the bCCEnabled flag */
   pUnitInst->bCCEnabled = TRUE;

   sem_put(pUnitInst->UnitSem); /* must be last line of routine! */

   return(eRetcode);
}

/********************************************************************/
/*  FUNCTION:    cnxt_tvenc_cc_send_data                            */
/*                                                                  */
/*  PARAMETERS:  Handle - a handle returne on a previous call to    */
/*                        cnxt_tvenc_open.                          */
/*               Type - the type of captioning data being sent,     */
/*                   either 0 or 1.                                 */
/*               uByteOne - byte one of the captioning data.        */
/*               uByteTwo - byte two of the captioning data.        */
/*                                                                  */                                                                
/*  DESCRIPTION: This function sends the captioning data to the     */
/*               encoder registers specified by type.               */
/*                                                                  */
/*  RETURNS:     CNXT_TVENC_OK                                      */
/*               CNXT_TVENC_NOT_INIT                                */
/*               CNXT_TVENC_NOT_AVAILABLE                           */
/*               CNXT_TVENC_BAD_HANDLE                              */
/*               CNXT_TVENC_INTERNAL_ERROR                          */
/*                                                                  */
/*  CONTEXT:     Must be called in task context.                    */
/*                                                                  */
/********************************************************************/
CNXT_TVENC_STATUS cnxt_tvenc_cc_send_data( CNXT_TVENC_HANDLE Handle, CNXT_TVENC_CC_TYPE Type, \
                                            u_int8 uByteOne, u_int8 uByteTwo )
{
   CNXT_TVENC_INST *pInst = (CNXT_TVENC_INST*)Handle;
   CNXT_TVENC_STATUS eRetcode;
   CNXT_TVENC_MODULE module;
   u_int32 uUnitNumber;
   CNXT_TVENC_UNIT_INST *pUnitInst;

   IS_DRIVER_INITED(TVENC,pDriverInst->bInit);
   IS_VALID_HANDLE(TVENC, pInst, &(pDriverInst->pUnitInst[pInst->uUnitNumber].pFirstInst));

   /* Data can only be either CC or XDS type */
   if ( (Type != CNXT_TVENC_CC_SEL) && (Type != CNXT_TVENC_XDS_SEL) )
   {
      return CNXT_TVENC_BAD_PARAMETER;
   }

   if ( (uUnitNumber = pInst->uUnitNumber) > CNXT_TVENC_NUM_UNITS)
   {
      return CNXT_TVENC_INTERNAL_ERROR;
   }

   /* get the encoder type */
   module = pDriverInst->pUnitInst[uUnitNumber].module;

   /* get the unit instance */
   pUnitInst = &(pDriverInst->pUnitInst[uUnitNumber]);

   /* teletext is not supported for all PAL and SECAM standards */
   if( (pUnitInst->VideoStandard == CNXT_TVENC_PAL_B_ITALY) \
    || (pUnitInst->VideoStandard == CNXT_TVENC_PAL_B_WEUR) \
    || (pUnitInst->VideoStandard == CNXT_TVENC_PAL_B_AUS) \
    || (pUnitInst->VideoStandard == CNXT_TVENC_PAL_B_NZ) \
    || (pUnitInst->VideoStandard == CNXT_TVENC_PAL_I) \
    || (pUnitInst->VideoStandard == CNXT_TVENC_SECAM_L) \
    || (pUnitInst->VideoStandard == CNXT_TVENC_SECAM_D) )
   {
      return CNXT_TVENC_NOT_AVAILABLE;
   }   
    
   /* if closed captioning is not enabled, return CNXT_TVENC_CC_NOT_INIT */
   if( !pUnitInst->bCCEnabled )
   {
      trace_new( TRACE_TVENC, "cnxt_tvenc_cc_send_data: closed captioning has not been enabled. \n");
      return CNXT_TVENC_CC_NOT_INIT;
   }

   if (sem_get(pUnitInst->UnitSem, KAL_WAIT_FOREVER) != RC_OK)
   {
      return CNXT_TVENC_INTERNAL_ERROR;
   }

      /* Send closed captioning data */
   if (function_table[module].cc_send_data != NULL)
   {
      eRetcode = function_table[module].cc_send_data(pUnitInst, Type, uByteOne, uByteTwo);
   }
   else
   {
      eRetcode = CNXT_TVENC_INTERNAL_ERROR;
   }

   sem_put(pUnitInst->UnitSem); /* must be last line of routine! */

   return(eRetcode);
}

/********************************************************************/
/*  FUNCTION:    cnxt_tvenc_cc_disable                              */
/*                                                                  */
/*  PARAMETERS:  Handle - a handle returne on a previous call to    */
/*                        cnxt_tvenc_open.                          */
/*                                                                  */                                                                
/*  DESCRIPTION: This function disables the closed captioning.      */
/*                                                                  */
/*  RETURNS:     CNXT_TVENC_OK                                      */
/*               CNXT_TVENC_NOT_INIT                                */
/*               CNXT_TVENC_NOT_AVAILABLE                           */
/*               CNXT_TVENC_BAD_HANDLE                              */
/*               CNXT_TVENC_INTERNAL_ERROR                          */
/*                                                                  */
/*  CONTEXT:     Must be called in task context.                    */
/*                                                                  */
/********************************************************************/
CNXT_TVENC_STATUS cnxt_tvenc_cc_disable( CNXT_TVENC_HANDLE Handle )
{
   CNXT_TVENC_INST *pInst = (CNXT_TVENC_INST*)Handle;
   CNXT_TVENC_STATUS eRetcode;
   CNXT_TVENC_MODULE module;
   u_int32 uUnitNumber;
   CNXT_TVENC_UNIT_INST *pUnitInst;

   IS_DRIVER_INITED(TVENC,pDriverInst->bInit);
   IS_VALID_HANDLE(TVENC, pInst, &(pDriverInst->pUnitInst[pInst->uUnitNumber].pFirstInst));

   if ( (uUnitNumber = pInst->uUnitNumber) > CNXT_TVENC_NUM_UNITS)
   {
      return CNXT_TVENC_INTERNAL_ERROR;
   }

   /* get the encoder type */
   module = pDriverInst->pUnitInst[uUnitNumber].module;

   /* get the unit instance */
   pUnitInst = &(pDriverInst->pUnitInst[uUnitNumber]);

   /* closed captioning is not supported for all PAL and SECAM standards */
   if( (pUnitInst->VideoStandard == CNXT_TVENC_PAL_B_ITALY) \
    || (pUnitInst->VideoStandard == CNXT_TVENC_PAL_B_WEUR) \
    || (pUnitInst->VideoStandard == CNXT_TVENC_PAL_B_AUS) \
    || (pUnitInst->VideoStandard == CNXT_TVENC_PAL_B_NZ) \
    || (pUnitInst->VideoStandard == CNXT_TVENC_PAL_I) \
    || (pUnitInst->VideoStandard == CNXT_TVENC_SECAM_L) \
    || (pUnitInst->VideoStandard == CNXT_TVENC_SECAM_D) )
   {
      return CNXT_TVENC_NOT_AVAILABLE;
   }   

   if (sem_get(pUnitInst->UnitSem, KAL_WAIT_FOREVER) != RC_OK)
   {
      return CNXT_TVENC_INTERNAL_ERROR;
   }

      /* Send closed captioning data */
   if (function_table[module].cc_disable != NULL)
   {
      eRetcode = function_table[module].cc_disable(pUnitInst);
   }
   else
   {
      eRetcode = CNXT_TVENC_INTERNAL_ERROR;
   }

   /* disable the bCCEnabled flag */
   pUnitInst->bCCEnabled = FALSE;

   sem_put(pUnitInst->UnitSem); /* must be last line of routine! */

   return(eRetcode);
}

/********************************************************************/
/*  FUNCTION:    cnxt_tvenc_wss_enable                              */
/*                                                                  */
/*  PARAMETERS:  Handle - a handle returne on a previous call to    */
/*                        cnxt_tvenc_open.                          */
/*                                                                  */                                                                
/*  DESCRIPTION: This function sets the appropriate encoder         */
/*               registers in the TV encoder to enable wide         */
/*               screen signaling.                                  */
/*                                                                  */
/*  RETURNS:     CNXT_TVENC_OK                                      */
/*               CNXT_TVENC_NOT_INIT                                */
/*               CNXT_TVENC_NOT_AVAILABLE                           */
/*               CNXT_TVENC_BAD_HANDLE                              */
/*               CNXT_TVENC_INTERNAL_ERROR                          */
/*                                                                  */
/*  CONTEXT:     Must be called in task context.                    */
/*                                                                  */
/********************************************************************/
CNXT_TVENC_STATUS cnxt_tvenc_wss_enable( CNXT_TVENC_HANDLE Handle )
{
   CNXT_TVENC_INST *pInst = (CNXT_TVENC_INST*)Handle;
   CNXT_TVENC_STATUS eRetcode;
   CNXT_TVENC_MODULE module;
   u_int32 uUnitNumber;
   CNXT_TVENC_UNIT_INST *pUnitInst;

   IS_DRIVER_INITED(TVENC,pDriverInst->bInit);
   IS_VALID_HANDLE(TVENC, pInst, &(pDriverInst->pUnitInst[pInst->uUnitNumber].pFirstInst));

   if ( (uUnitNumber = pInst->uUnitNumber) > CNXT_TVENC_NUM_UNITS)
   {
      return CNXT_TVENC_INTERNAL_ERROR;
   }

   /* get the encoder type */
   module = pDriverInst->pUnitInst[uUnitNumber].module;

   /* get the unit instance */
   pUnitInst = &(pDriverInst->pUnitInst[uUnitNumber]);

   /* wide screen signaling is not supported for NTSC standard */
   if( (pUnitInst->VideoStandard == CNXT_TVENC_NTSC_M) \
    || (pUnitInst->VideoStandard == CNXT_TVENC_NTSC_JAPAN) )
   {
      return CNXT_TVENC_NOT_AVAILABLE;
   }   

   if (sem_get(pUnitInst->UnitSem, KAL_WAIT_FOREVER) != RC_OK)
   {
      return CNXT_TVENC_INTERNAL_ERROR;
   }

      /* Enable wide screen signaling */
   if (function_table[module].wss_enable != NULL)
   {
      eRetcode = function_table[module].wss_enable(pUnitInst);
   }
   else
   {
      eRetcode = CNXT_TVENC_INTERNAL_ERROR;
   }

   sem_put(pUnitInst->UnitSem); /* must be last line of routine! */

   return(eRetcode);
}

/********************************************************************/
/*  FUNCTION:    cnxt_tvenc_wss_set_config                          */
/*                                                                  */
/*  PARAMETERS:  Handle - a handle returne on a previous call to    */
/*                        cnxt_tvenc_open.                          */
/*               pWSS_Settings - pointer to structure containing    */
/*               wide screen signaling configuration settings       */
/*                                                                  */                                                                
/*  DESCRIPTION: This function passes the configuration information */
/*               of the wide screen signaling to TV encoder.        */
/*                                                                  */
/*  RETURNS:     CNXT_TVENC_OK                                      */
/*               CNXT_TVENC_NOT_INIT                                */
/*               CNXT_TVENC_NOT_AVAILABLE                           */
/*               CNXT_TVENC_BAD_HANDLE                              */
/*               CNXT_TVENC_BAD_PARAMETER                           */
/*               CNXT_TVENC_INTERNAL_ERROR                          */
/*                                                                  */
/*  CONTEXT:     Must be called in task context.                    */
/*                                                                  */
/********************************************************************/
CNXT_TVENC_STATUS cnxt_tvenc_wss_set_config( CNXT_TVENC_HANDLE Handle, CNXT_TVENC_WSS_SETTINGS *pWSS_Settings )
{
   CNXT_TVENC_INST *pInst = (CNXT_TVENC_INST*)Handle;
   CNXT_TVENC_STATUS eRetcode;
   CNXT_TVENC_MODULE module;
   u_int32 uUnitNumber;
   CNXT_TVENC_UNIT_INST *pUnitInst;

   IS_DRIVER_INITED(TVENC,pDriverInst->bInit);
   IS_VALID_HANDLE(TVENC, pInst, &(pDriverInst->pUnitInst[pInst->uUnitNumber].pFirstInst));
   IS_NOT_NULL_POINTER(TVENC, pWSS_Settings);

   if ( (uUnitNumber = pInst->uUnitNumber) > CNXT_TVENC_NUM_UNITS)
   {
      return CNXT_TVENC_INTERNAL_ERROR;
   }

   /* get the encoder type */
   module = pDriverInst->pUnitInst[uUnitNumber].module;

   /* get the unit instance */
   pUnitInst = &(pDriverInst->pUnitInst[uUnitNumber]);

   /* wide screen signaling is not supported for NTSC standard */
   if( (pUnitInst->VideoStandard == CNXT_TVENC_NTSC_M) \
    || (pUnitInst->VideoStandard == CNXT_TVENC_NTSC_JAPAN) )
   {
      return CNXT_TVENC_NOT_AVAILABLE;
   }   

   if (sem_get(pUnitInst->UnitSem, KAL_WAIT_FOREVER) != RC_OK)
   {
      return CNXT_TVENC_INTERNAL_ERROR;
   }

      /* Set configuration info for wide screen signaling */
   if (function_table[module].wss_setconfig != NULL)
   {
      eRetcode = function_table[module].wss_setconfig(pUnitInst, pWSS_Settings);
   }
   else
   {
      eRetcode = CNXT_TVENC_INTERNAL_ERROR;
   }

   sem_put(pUnitInst->UnitSem); /* must be last line of routine! */

   return(eRetcode);
}

/********************************************************************/
/*  FUNCTION:    cnxt_tvenc_wss_get_config                          */
/*                                                                  */
/*  PARAMETERS:  Handle - a handle returne on a previous call to    */
/*                        cnxt_tvenc_open.                          */
/*               pWSS_Settings - pointer to structure receiving     */
/*               wide screen signaling configuration settings       */
/*                                                                  */                                                                
/*  DESCRIPTION: This function queries the configuration information*/
/*               of the wide screen signaling to TV encoder.        */
/*                                                                  */
/*  RETURNS:     CNXT_TVENC_OK                                      */
/*               CNXT_TVENC_NOT_INIT                                */
/*               CNXT_TVENC_NOT_AVAILABLE                           */
/*               CNXT_TVENC_BAD_HANDLE                              */
/*               CNXT_TVENC_BAD_PARAMETER                           */
/*               CNXT_TVENC_INTERNAL_ERROR                          */
/*                                                                  */
/*  CONTEXT:     Must be called in task context.                    */
/*                                                                  */
/********************************************************************/
CNXT_TVENC_STATUS cnxt_tvenc_wss_get_config( CNXT_TVENC_HANDLE Handle, CNXT_TVENC_WSS_SETTINGS *pWSS_Settings )
{
   CNXT_TVENC_INST *pInst = (CNXT_TVENC_INST*)Handle;
   CNXT_TVENC_STATUS eRetcode;
   CNXT_TVENC_MODULE module;
   u_int32 uUnitNumber;
   CNXT_TVENC_UNIT_INST *pUnitInst;

   IS_DRIVER_INITED(TVENC,pDriverInst->bInit);
   IS_VALID_HANDLE(TVENC, pInst, &(pDriverInst->pUnitInst[pInst->uUnitNumber].pFirstInst));
   IS_NOT_NULL_POINTER(TVENC, pWSS_Settings);

   if ( (uUnitNumber = pInst->uUnitNumber) > CNXT_TVENC_NUM_UNITS)
   {
      return CNXT_TVENC_INTERNAL_ERROR;
   }

   /* get the encoder type */
   module = pDriverInst->pUnitInst[uUnitNumber].module;

   /* get the unit instance */
   pUnitInst = &(pDriverInst->pUnitInst[uUnitNumber]);

   /* wide screen signaling is not supported for NTSC standard */
   if( (pUnitInst->VideoStandard == CNXT_TVENC_NTSC_M) \
    || (pUnitInst->VideoStandard == CNXT_TVENC_NTSC_JAPAN) )
   {
      return CNXT_TVENC_NOT_AVAILABLE;
   }   

      /* Read the configuration info of wide screen signaling */
   if (function_table[module].wss_getconfig != NULL)
   {
      eRetcode = function_table[module].wss_getconfig(pUnitInst, pWSS_Settings);
   }
   else
   {
      eRetcode = CNXT_TVENC_INTERNAL_ERROR;
   }

   return(eRetcode);
}

/********************************************************************/
/*  FUNCTION:    cnxt_tvenc_wss_disable                             */
/*                                                                  */
/*  PARAMETERS:  Handle - a handle returne on a previous call to    */
/*                        cnxt_tvenc_open.                          */
/*                                                                  */                                                                
/*  DESCRIPTION: This function disables wide screen signaling       */
/*               in the encoder.                                    */
/*                                                                  */
/*  RETURNS:     CNXT_TVENC_OK                                      */
/*               CNXT_TVENC_NOT_INIT                                */
/*               CNXT_TVENC_NOT_AVAILABLE                           */
/*               CNXT_TVENC_BAD_HANDLE                              */
/*               CNXT_TVENC_INTERNAL_ERROR                          */
/*                                                                  */
/*  CONTEXT:     Must be called in task context.                    */
/*                                                                  */
/********************************************************************/
CNXT_TVENC_STATUS cnxt_tvenc_wss_disable( CNXT_TVENC_HANDLE Handle )
{
   CNXT_TVENC_INST *pInst = (CNXT_TVENC_INST*)Handle;
   CNXT_TVENC_STATUS eRetcode;
   CNXT_TVENC_MODULE module;
   u_int32 uUnitNumber;
   CNXT_TVENC_UNIT_INST *pUnitInst;

   IS_DRIVER_INITED(TVENC,pDriverInst->bInit);
   IS_VALID_HANDLE(TVENC, pInst, &(pDriverInst->pUnitInst[pInst->uUnitNumber].pFirstInst));

   if ( (uUnitNumber = pInst->uUnitNumber) > CNXT_TVENC_NUM_UNITS)
   {
      return CNXT_TVENC_INTERNAL_ERROR;
   }

   /* get the encoder type */
   module = pDriverInst->pUnitInst[uUnitNumber].module;

   /* get the unit instance */
   pUnitInst = &(pDriverInst->pUnitInst[uUnitNumber]);

   /* wide screen signaling is not supported for NTSC standard */
   if( (pUnitInst->VideoStandard == CNXT_TVENC_NTSC_M) \
    || (pUnitInst->VideoStandard == CNXT_TVENC_NTSC_JAPAN) )
   {
      return CNXT_TVENC_NOT_AVAILABLE;
   }   

   if (sem_get(pUnitInst->UnitSem, KAL_WAIT_FOREVER) != RC_OK)
   {
      return CNXT_TVENC_INTERNAL_ERROR;
   }

      /* Disable wide screen signaling */
   if (function_table[module].wss_disable != NULL)
   {
      eRetcode = function_table[module].wss_disable(pUnitInst);
   }
   else
   {
      eRetcode = CNXT_TVENC_INTERNAL_ERROR;
   }

   sem_put(pUnitInst->UnitSem); /* must be last line of routine! */

   return(eRetcode);
}

/********************************************************************/
/*  FUNCTION:    cnxt_tvenc_cgms_enable                             */
/*                                                                  */
/*  PARAMETERS:  Handle - a handle returne on a previous call to    */
/*                        cnxt_tvenc_open.                          */
/*                                                                  */                                                                
/*  DESCRIPTION: This function sets the appropriate encoder         */
/*               registers in the TV encoder to enable copy         */
/*               generation managment system.                       */
/*                                                                  */
/*  RETURNS:     CNXT_TVENC_OK                                      */
/*               CNXT_TVENC_NOT_INIT                                */
/*               CNXT_TVENC_NOT_AVAILABLE                           */
/*               CNXT_TVENC_BAD_HANDLE                              */
/*               CNXT_TVENC_INTERNAL_ERROR                          */
/*                                                                  */
/*  CONTEXT:     Must be called in task context.                    */
/*                                                                  */
/********************************************************************/
CNXT_TVENC_STATUS cnxt_tvenc_cgms_enable( CNXT_TVENC_HANDLE Handle )
{
   CNXT_TVENC_INST *pInst = (CNXT_TVENC_INST*)Handle;
   CNXT_TVENC_STATUS eRetcode;
   CNXT_TVENC_MODULE module;
   u_int32 uUnitNumber;
   CNXT_TVENC_UNIT_INST *pUnitInst;

   IS_DRIVER_INITED(TVENC,pDriverInst->bInit);
   IS_VALID_HANDLE(TVENC, pInst, &(pDriverInst->pUnitInst[pInst->uUnitNumber].pFirstInst));

   if ( (uUnitNumber = pInst->uUnitNumber) > CNXT_TVENC_NUM_UNITS)
   {
      return CNXT_TVENC_INTERNAL_ERROR;
   }

   /* get the encoder type */
   module = pDriverInst->pUnitInst[uUnitNumber].module;

   /* get the unit instance */
   pUnitInst = &(pDriverInst->pUnitInst[uUnitNumber]);

   /* CGMS is not supported for PAL and SECAM standards */
   if( (pUnitInst->VideoStandard == CNXT_TVENC_PAL_B_ITALY) \
    || (pUnitInst->VideoStandard == CNXT_TVENC_PAL_B_WEUR) \
    || (pUnitInst->VideoStandard == CNXT_TVENC_PAL_B_AUS) \
    || (pUnitInst->VideoStandard == CNXT_TVENC_PAL_B_NZ) \
    || (pUnitInst->VideoStandard == CNXT_TVENC_PAL_I) \
    || (pUnitInst->VideoStandard == CNXT_TVENC_SECAM_L) \
    || (pUnitInst->VideoStandard == CNXT_TVENC_SECAM_D) )
   {
      return CNXT_TVENC_NOT_AVAILABLE;
   }   

   if (sem_get(pUnitInst->UnitSem, KAL_WAIT_FOREVER) != RC_OK)
   {
      return CNXT_TVENC_INTERNAL_ERROR;
   }

      /* Enable cgms */
   if (function_table[module].cgms_enable != NULL)
   {
      eRetcode = function_table[module].cgms_enable(pUnitInst);
   }
   else
   {
      eRetcode = CNXT_TVENC_INTERNAL_ERROR;
   }

   sem_put(pUnitInst->UnitSem); /* must be last line of routine! */

   return(eRetcode);
}

/********************************************************************/
/*  FUNCTION:    cnxt_tvenc_cgms_set_config                         */
/*                                                                  */
/*  PARAMETERS:  Handle - a handle returne on a previous call to    */
/*                        cnxt_tvenc_open.                          */
/*               pCGMS_Settings - pointer to structure containing   */
/*               CGMS configuration settings                        */
/*                                                                  */                                                                
/*  DESCRIPTION: This function passes the configuration information */
/*               of the CGMS configuration to TV encoder.           */
/*                                                                  */
/*  RETURNS:     CNXT_TVENC_OK                                      */
/*               CNXT_TVENC_NOT_INIT                                */
/*               CNXT_TVENC_NOT_AVAILABLE                           */
/*               CNXT_TVENC_BAD_HANDLE                              */
/*               CNXT_TVENC_BAD_PARAMETER                           */
/*               CNXT_TVENC_INTERNAL_ERROR                          */
/*                                                                  */
/*  CONTEXT:     Must be called in task context.                    */
/*                                                                  */
/********************************************************************/
CNXT_TVENC_STATUS cnxt_tvenc_cgms_set_config( CNXT_TVENC_HANDLE Handle, 
                            CNXT_TVENC_CGMS_SETTINGS *pCGMS_Settings )
{
   CNXT_TVENC_INST *pInst = (CNXT_TVENC_INST*)Handle;
   CNXT_TVENC_STATUS eRetcode;
   CNXT_TVENC_MODULE module;
   u_int32 uUnitNumber;
   CNXT_TVENC_UNIT_INST *pUnitInst;

   IS_DRIVER_INITED(TVENC,pDriverInst->bInit);
   IS_VALID_HANDLE(TVENC, pInst, &(pDriverInst->pUnitInst[pInst->uUnitNumber].pFirstInst));
   IS_NOT_NULL_POINTER(TVENC, pCGMS_Settings);

   if ( (uUnitNumber = pInst->uUnitNumber) > CNXT_TVENC_NUM_UNITS)
   {
      return CNXT_TVENC_INTERNAL_ERROR;
   }

   /* get the encoder type */
   module = pDriverInst->pUnitInst[uUnitNumber].module;

   /* get the unit instance */
   pUnitInst = &(pDriverInst->pUnitInst[uUnitNumber]);

   /* CGMS is not supported for PAL and SECAM standards */
   if( (pUnitInst->VideoStandard == CNXT_TVENC_PAL_B_ITALY) \
    || (pUnitInst->VideoStandard == CNXT_TVENC_PAL_B_WEUR) \
    || (pUnitInst->VideoStandard == CNXT_TVENC_PAL_B_AUS) \
    || (pUnitInst->VideoStandard == CNXT_TVENC_PAL_B_NZ) \
    || (pUnitInst->VideoStandard == CNXT_TVENC_PAL_I) \
    || (pUnitInst->VideoStandard == CNXT_TVENC_SECAM_L) \
    || (pUnitInst->VideoStandard == CNXT_TVENC_SECAM_D) )
   {
      return CNXT_TVENC_NOT_AVAILABLE;
   }   

   if (sem_get(pUnitInst->UnitSem, KAL_WAIT_FOREVER) != RC_OK)
   {
      return CNXT_TVENC_INTERNAL_ERROR;
   }

      /* Set configuration info for CGMS */
   if (function_table[module].cgms_setconfig != NULL)
   {
      eRetcode = function_table[module].cgms_setconfig(pUnitInst, pCGMS_Settings);
   }
   else
   {
      eRetcode = CNXT_TVENC_INTERNAL_ERROR;
   }

   sem_put(pUnitInst->UnitSem); /* must be last line of routine! */

   return(eRetcode);
}

/********************************************************************/
/*  FUNCTION:    cnxt_tvenc_cgms_get_config                         */
/*                                                                  */
/*  PARAMETERS:  Handle - a handle returne on a previous call to    */
/*                        cnxt_tvenc_open.                          */
/*               pCGMS_Settings - pointer to structure receiving    */
/*               CGMS configuration settings                        */
/*                                                                  */                                                                
/*  DESCRIPTION: This function queries the configuration information*/
/*               of the CGMS to TV encoder.                         */
/*                                                                  */
/*  RETURNS:     CNXT_TVENC_OK                                      */
/*               CNXT_TVENC_NOT_INIT                                */
/*               CNXT_TVENC_NOT_AVAILABLE                           */
/*               CNXT_TVENC_BAD_HANDLE                              */
/*               CNXT_TVENC_BAD_PARAMETER                           */
/*               CNXT_TVENC_INTERNAL_ERROR                          */
/*                                                                  */
/*  CONTEXT:     Must be called in task context.                    */
/*                                                                  */
/********************************************************************/
CNXT_TVENC_STATUS cnxt_tvenc_cgms_get_config( CNXT_TVENC_HANDLE Handle, 
                             CNXT_TVENC_CGMS_SETTINGS *pCGMS_Settings )
{
   CNXT_TVENC_INST *pInst = (CNXT_TVENC_INST*)Handle;
   CNXT_TVENC_STATUS eRetcode;
   CNXT_TVENC_MODULE module;
   u_int32 uUnitNumber;
   CNXT_TVENC_UNIT_INST *pUnitInst;

   IS_DRIVER_INITED(TVENC,pDriverInst->bInit);
   IS_VALID_HANDLE(TVENC, pInst, &(pDriverInst->pUnitInst[pInst->uUnitNumber].pFirstInst));
   IS_NOT_NULL_POINTER(TVENC, pCGMS_Settings);

   if ( (uUnitNumber = pInst->uUnitNumber) > CNXT_TVENC_NUM_UNITS)
   {
      return CNXT_TVENC_INTERNAL_ERROR;
   }

   /* get the encoder type */
   module = pDriverInst->pUnitInst[uUnitNumber].module;

   /* get the unit instance */
   pUnitInst = &(pDriverInst->pUnitInst[uUnitNumber]);

   /* CGMS is not supported for PAL and SECAM standards */
   if( (pUnitInst->VideoStandard == CNXT_TVENC_PAL_B_ITALY) \
    || (pUnitInst->VideoStandard == CNXT_TVENC_PAL_B_WEUR) \
    || (pUnitInst->VideoStandard == CNXT_TVENC_PAL_B_AUS) \
    || (pUnitInst->VideoStandard == CNXT_TVENC_PAL_B_NZ) \
    || (pUnitInst->VideoStandard == CNXT_TVENC_PAL_I) \
    || (pUnitInst->VideoStandard == CNXT_TVENC_SECAM_L) \
    || (pUnitInst->VideoStandard == CNXT_TVENC_SECAM_D) )
   {
      return CNXT_TVENC_NOT_AVAILABLE;
   }   

      /* Read the configuration info of CGMS */
   if (function_table[module].cgms_getconfig != NULL)
   {
      eRetcode = function_table[module].cgms_getconfig(pUnitInst, pCGMS_Settings);
   }
   else
   {
      eRetcode = CNXT_TVENC_INTERNAL_ERROR;
   }

   return(eRetcode);
}

/********************************************************************/
/*  FUNCTION:    cnxt_tvenc_cgms_disable                            */
/*                                                                  */
/*  PARAMETERS:  Handle - a handle returne on a previous call to    */
/*                        cnxt_tvenc_open.                          */
/*                                                                  */                                                                
/*  DESCRIPTION: This function disables CGMS in the encoder.        */
/*                                                                  */
/*  RETURNS:     CNXT_TVENC_OK                                      */
/*               CNXT_TVENC_NOT_INIT                                */
/*               CNXT_TVENC_NOT_AVAILABLE                           */
/*               CNXT_TVENC_BAD_HANDLE                              */
/*               CNXT_TVENC_INTERNAL_ERROR                          */
/*                                                                  */
/*  CONTEXT:     Must be called in task context.                    */
/*                                                                  */
/********************************************************************/
CNXT_TVENC_STATUS cnxt_tvenc_cgms_disable( CNXT_TVENC_HANDLE Handle )
{
   CNXT_TVENC_INST *pInst = (CNXT_TVENC_INST*)Handle;
   CNXT_TVENC_STATUS eRetcode;
   CNXT_TVENC_MODULE module;
   u_int32 uUnitNumber;
   CNXT_TVENC_UNIT_INST *pUnitInst;

   IS_DRIVER_INITED(TVENC,pDriverInst->bInit);
   IS_VALID_HANDLE(TVENC, pInst, &(pDriverInst->pUnitInst[pInst->uUnitNumber].pFirstInst));

   if ( (uUnitNumber = pInst->uUnitNumber) > CNXT_TVENC_NUM_UNITS)
   {
      return CNXT_TVENC_INTERNAL_ERROR;
   }

   /* get the encoder type */
   module = pDriverInst->pUnitInst[uUnitNumber].module;

   /* get the unit instance */
   pUnitInst = &(pDriverInst->pUnitInst[uUnitNumber]);

   /* CGMS is not supported for PAL and SECAM standards */
   if( (pUnitInst->VideoStandard == CNXT_TVENC_PAL_B_ITALY) \
    || (pUnitInst->VideoStandard == CNXT_TVENC_PAL_B_WEUR) \
    || (pUnitInst->VideoStandard == CNXT_TVENC_PAL_B_AUS) \
    || (pUnitInst->VideoStandard == CNXT_TVENC_PAL_B_NZ) \
    || (pUnitInst->VideoStandard == CNXT_TVENC_PAL_I) \
    || (pUnitInst->VideoStandard == CNXT_TVENC_SECAM_L) \
    || (pUnitInst->VideoStandard == CNXT_TVENC_SECAM_D) )
   {
      return CNXT_TVENC_NOT_AVAILABLE;
   }   

   if (sem_get(pUnitInst->UnitSem, KAL_WAIT_FOREVER) != RC_OK)
   {
      return CNXT_TVENC_INTERNAL_ERROR;
   }

      /* Disable CGMS */
   if (function_table[module].cgms_disable != NULL)
   {
      eRetcode = function_table[module].cgms_disable(pUnitInst);
   }
   else
   {
      eRetcode = CNXT_TVENC_INTERNAL_ERROR;
   }

   sem_put(pUnitInst->UnitSem); /* must be last line of routine! */

   return(eRetcode);
}

/****************************************************************************
 * Modifications:
 * $Log: 
 *  12   mpeg      1.11        3/18/04 11:54:38 AM    Xin Golden      CR(s) 
 *        8592 : modified picture control setting range from [-127,128] to 
 *        [-128,127].
 *  11   mpeg      1.10        3/16/04 10:56:34 AM    Xin Golden      CR(s) 
 *        8567 : initialized eRetcode when function_table is null to fix 
 *        warnings when build in release.
 *  10   mpeg      1.9         1/6/04 6:57:02 PM      Xin Golden      CR(s) 
 *        8158 : added feature to disable all DACs in function 
 *        cnxt_tvenc_set_output_connection.
 *  9    mpeg      1.8         10/24/03 11:37:25 AM   Xin Golden      CR(s): 
 *        7463 Please ignore the last log.  Add CGMS support in tvenc driver.
 *  8    mpeg      1.7         10/24/03 11:25:39 AM   Xin Golden      CR(s): 
 *        7463 Run test# 10 in the TVENC test case.  To execute every test 
 *        cases in test# 10, we need to use a video analyser or a DVD player 
 *        supporting CGMS.  For now I used the video analyser to test it, and 
 *        it seems fine.
 *  7    mpeg      1.6         10/16/03 2:31:15 PM    Xin Golden      CR(s): 
 *        5519 added wss support for SECAM standard.
 *  6    mpeg      1.5         9/25/03 12:45:22 PM    Lucy C Allevato SCR(s) 
 *        7548 :
 *        define gdwEncoderType as extern because it's defined in encoder.c.
 *        
 *  5    mpeg      1.4         9/24/03 6:14:40 PM     Lucy C Allevato SCR(s) 
 *        5519 :
 *        add definition of gdwEncoderType which is still referenced by other 
 *        drivers for now.  Need to remove it later.
 *        
 *  4    mpeg      1.3         9/17/03 4:00:02 PM     Lucy C Allevato SCR(s) 
 *        7482 :
 *        update the TV encoder driver to use the new handle lib.
 *        
 *  3    mpeg      1.2         8/26/03 7:12:10 PM     Lucy C Allevato SCR(s) 
 *        5519 :
 *        #include handle.h unconditionally
 *        
 *  2    mpeg      1.1         8/18/03 7:55:32 PM     Lucy C Allevato SCR(s) 
 *        5519 :
 *        modified the cc APIs
 *        
 *  1    mpeg      1.0         7/30/03 3:59:44 PM     Lucy C Allevato 
 * $
 ****************************************************************************/

