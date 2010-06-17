/****************************************************************************/
/*                   CONEXANT PROPRIETARY AND CONFIDENTIAL                  */
/*                       SOFTWARE FILE/MODULE HEADER                        */
/*                   Conexant Systems Inc. (c) 1998-2003                    */
/*                               Austin, TX                                 */
/*                            All Rights Reserved                           */
/****************************************************************************/
/*
 * Filename:       confmgr.c
 *
 *
 * Description:    Functions managing the storage and retrieval of 
 *                 configuration data in non-volatile memory.
 *
 *
 * Author:         Dave Wilson
 *
 ****************************************************************************/
/* $Header: confmgr.c, 63, 11/5/03 2:28:58 PM, Song Qiao$
 ****************************************************************************/
#include <stdio.h>
#include <string.h>
#include "stbcfg.h"
#ifdef OPENTV
#include "opentvx.h"
#else
#include "basetype.h"
#endif
#include "kal.h"
#include "globals.h"
#include "retcodes.h"
#include "confmgr.h"
#include "nvstore.h"
#include "fpleds.h"
#include "osddefs.h"
#include "tuner.h"
#include "demod_types.h"
#include "demod.h"
#include "startup.h"
#include "buttons.h"


/* Set the default trace level to use */
#ifdef DRIVER_INCL_NDSTESTS
 #include "conf_ndstst.h"
#else
  #ifdef DRIVER_INCL_OTV12CTL 
   #include "conf_otv12.h"
  #else
   #include "conf_defaults.h"
  #endif
#endif

/********************/
/* Global variables */
/********************/

bool         bConfigInit;
int          iOverrideVideoType = FROM_CONFIG;
int          iOverrideSource = FROM_CONFIG;

sem_id_t     semConfigAccess;
sem_id_t     semBspConfigAccess;

sabine_config_data sabine_config;
bsp_config_data    bsp_config;

/* NOTE: the macros such as CONF_XXXX are defined in conf_xxxx.h headers */
sabine_config_data sabine_defaults =
{
  sizeof(sabine_config_data),
  0,
  MAGIC,
  CONF_VIDEO_OUTPUT_STANDARD,
  PANSCAN,
  ASPECT43,
  VIDEO_COMPOSITE,
  TRUE,
  AUDIO_MPEG2,
  AUDIO_SOURCE_INTERNAL,
  TRUE,
  TRUE,

#if IRD_HAS_CABLE_DEMOD
  NIM_CABLE,
#elif IRD_HAS_TER_DEMOD
  NIM_TERRESTRIAL,
#elif IRD_HAS_SAT_DEMOD
  NIM_SATELLITE_DEMOD,
#else
  NIM_DVB_BASEBAND,
#endif

  CONF_FREQUENCY, 
  CONF_SYMBOL_RATE,
  CONF_FEC,
  CONF_POLARISATION,
  CONF_LNB_A,
  CONF_LNB_B,
  0x01154000,
  LNB_TYPE_SINGLE | LNB_LONG_WEST_A | LNB_LONG_WEST_B | LNB_LONG_WEST_C |
                    LNB_22KHZ_OFF_A | LNB_22KHZ_ON_B  | LNB_22KHZ_ON_C,
  1010,
  1190,
  1098,
  CONF_RIGHT_IS_LOW,
  0x77816600,
  BANDWIDTH_8_MHZ,
  CONF_CM_FREQUENCY,
  CONF_CM_SYMBOL_RATE,
  64,
  SPECTRUM_NORMAL,
  CONF_CM_ANNEX,
  CONF_CM_UART_FLAGS,
  0,                    /* CNXT_CM_DOCSIS_MODE */
  0,                    /* CNXT_OC_OOBFE_MODE_DOCSIS_ONLY */
  0,
  CONF_ATV_CHANNEL,
  TUNER_BROADCAST,
  CONF_ATV_COUNTRY,
  TUNER,
  {0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF},
  0x80,
  0x80,
  0x80,
  SVL_USE_NIT,

#if IRD_HAS_CABLE_DEMOD
  166,
#else
  101,
#endif

  CONFMGR_DEFAULT_TRACE,
  FALSE,

#ifndef DRIVER_INCL_RFMOD
  NEW_REMOTE_PRESENT | NEW_KEYB_PRESENT,
#else
  NEW_REMOTE_PRESENT | NEW_KEYB_PRESENT | RFMOD_OFF,
#endif

  0,
  0,
  0,
  0,

  {0x00, 0x00, 0x00, 0x00, 0x00, 0x02,
   0x00, 0x11, 0x22, 0x33, 0x44, 0x55,
   0x33, 0x33, 0x33, 0x33, 0x33, 0x33,
   0x44, 0x44, 0x44, 0x44, 0x44, 0x44,
   0x55, 0x55, 0x55, 0x55, 0x55, 0x55,
   0x66, 0x66, 0x66, 0x66, 0x66, 0x66,
   0x77, 0x77, 0x77, 0x77, 0x77, 0x77,
   0x88, 0x88, 0x88, 0x88, 0x88, 0x88},

  0,        /* Power Cal Pad */
  64,       /* DEFAULT_US_POWER_GAIN */
  4096,     /* DEFAULT_US_POWER_OFFSET */

  /* DownstreamPowerCal_Defaults */
  70000000, 0, 0, 0, 0,
  70000000, 0, 0, 0, 0,
  70000000, 0, 0, 0, 0,
  /* Inband Video DownstreamPowerCal_Defaults */
  0, 0xe0, 0xe0, 0, 0,
  158000000, 0xe0, 0xe0, 0, 0,
  464000000, 0xe0, 0xe0, 0, 0
};


/******************/
/* External Stuff */
/******************/

extern bool FPGetState(u_int16 key_code);

u_int32 NewCheckSum(void *StructPtr, int StructSize);

typedef union _BspSabineParms
{
   sabine_config_data sabine;
   bsp_config_data    bsp;
} BspSabineParms;

/*****************************/
/* Local Function Prototypes */
/*****************************/
bool config_manager_init(void);

/***********************************/
/***********************************/
/**                               **/
/** Exported functions start here **/
/**                               **/
/***********************************/
/***********************************/


/*******************************************************************/
/*    NewCheckSum: Calculate the checksum for a STORAGE structure     */
/*                                                                 */
/*      INPUTS: StructPtr - ptr to structure to checksum           */
/*              StructSize - size of structure in bytes            */
/*     RETURNS: Checksum for the structure                         */
/*        NOTE: We consider the "checksum" to be simply the sum of */
/*              the individual bytes in the structure              */
/*                                                                 */
/*******************************************************************/
u_int32 NewCheckSum(void *StructPtr, int StructSize)
{
u_int8 *p = (u_int8 *)StructPtr;
u_int32 tot = 0;

while(StructSize--)
    tot += *p++ & 0xFF;
return tot;
}

/***************************************************************************/
/* Retrieve a pointer to the current configuration data. This pointer will */
/* be to a copy of the data in RAM that the caller can then modify as      */
/* required. While the config data is locked, no other process may access  */
/* it.                                                                     */
/***************************************************************************/
sabine_config_data *config_lock_data(void)
{
   trace_new(TRACE_KAL | TRACE_LEVEL_2, "config_lock_data\n");

   /*************************************/
   /* Initialise the configuration data */
   /*************************************/
   if(!bConfigInit)
     bConfigInit = config_manager_init();

   if(!bConfigInit)
     return(NULL);

   /* Get the access semaphore */
   sem_get(semConfigAccess, KAL_WAIT_FOREVER);

   /* Return a pointer to the data to the caller */
   return(&sabine_config);
}

/*************************************************************************/
/* Unlock the config data. This frees up the data for another process to */
/* lock.                                                                 */
/*************************************************************************/
void config_unlock_data(sabine_config_data *pConfig)
{
   int iRetcode;

   trace_new(TRACE_KAL | TRACE_LEVEL_2, "config_unlock_data\n");

   if(!bConfigInit)
     return;

   /* Ensure that we have previously been called to lock the data */
   iRetcode = sem_get(semConfigAccess, 0);
   if(!iRetcode){
     trace_new(TRACE_KAL | TRACE_LEVEL_3, "Unmatched config_unlock_data call!\n");
   }
   else{
     sabine_config.checksum = NewCheckSum(&sabine_config.magic, sizeof(sabine_config) - 2*sizeof(u_int32));
   }

   /* Release the access semaphore */
   sem_put(semConfigAccess);
}

/***************************************************************************/
/* Retrieve a pointer to the current BSP config data. This pointer will    */
/* be to a copy of the data in RAM that the caller can then modify as      */
/* required. While the config data is locked, no other process may access  */
/* it.                                                                     */
/***************************************************************************/
bsp_config_data *config_lock_bsp_data(void)
{
   trace_new(TRACE_KAL | TRACE_LEVEL_2, "config_lock_bsp_data\n");

   /*************************************/
   /* Initialise the configuration data */
   /*************************************/
   if(!bConfigInit)
     bConfigInit = config_manager_init();

   if(!bConfigInit)
     return(NULL);

   /* Get the access semaphore */
   sem_get(semBspConfigAccess, KAL_WAIT_FOREVER);

   /* Return a pointer to the data to the caller */
   return(&bsp_config);
}

/*************************************************************************/
/* Unlock the config data. This frees up the data for another process to */
/* lock.                                                                 */
/*************************************************************************/
void config_unlock_bsp_data(bsp_config_data *pConfig)
{
   int iRetcode;

   trace_new(TRACE_KAL | TRACE_LEVEL_2, "config_unlock_bsp_data\n");

   if(!bConfigInit)
     return;

   /* Ensure that we have previously been called to lock the data */
   iRetcode = sem_get(semBspConfigAccess, 0);
   if(!iRetcode)
     trace_new(TRACE_KAL | TRACE_LEVEL_3, "Unmatched config_unlock_bsp_data call!\n");

   /* Fix up the structure checksum */
   bsp_config.checksum = NewCheckSum(&bsp_config.magic, sizeof(bsp_config) - 2*sizeof(u_int32));

   /* Release the access semaphore */
   sem_put(semBspConfigAccess);
}

/*****************************************************************************/
/* Write the latest config data to flash if it is different from the version */
/* currently stored there.                                                   */
/*****************************************************************************/
#ifndef LOADER
#ifdef DRIVER_INCL_NVSTORE
bool config_write_data(void)
{
   BspSabineParms info;
   u_int32 iRetcode;
   bool    bRetcode = TRUE;
   u_int32 kal_size_data,
           bsp_size_data;

   trace_new(TRACE_KAL | TRACE_LEVEL_2, "config_write_data\n");

   /* First we read the existing data into our own buffer then compare  */
   /* it with the latest RAM copy. If it is different, we write it back */
   /* to flash, otherwise we just return telling the caller we are done.*/

   /* This is done separately for the sabine config info and the pSOS   */
   /* BSP information.                                                  */

   /* Handle sabine config information */

   iRetcode = nv_read(NV_KAL_CLIENT_ID, &info.sabine, sizeof(sabine_config_data), 0, &kal_size_data);

   if((kal_size_data != sizeof(sabine_config_data)) ||
       memcmp(&info.sabine, &sabine_config, sizeof(sabine_config_data)))
   {
     iRetcode = nv_write(NV_KAL_CLIENT_ID, &sabine_config, sizeof(sabine_config_data), 0, &kal_size_data);
     if(kal_size_data != sizeof(sabine_config_data))
     {
       trace_new(TRACE_KAL | TRACE_LEVEL_ALWAYS, "Can't write config info to flash!\n");
       bRetcode = FALSE;
     }
   }

   /* Handle BSP config information */

   iRetcode = nv_read(NV_BSP_CLIENT_ID, &info.bsp, sizeof(bsp_config_data), 0, &bsp_size_data);

   if((bsp_size_data != sizeof(bsp_config_data)) ||
       memcmp(&info.bsp, &bsp_config, sizeof(bsp_config_data)))
   {
     iRetcode = nv_write(NV_BSP_CLIENT_ID, &bsp_config, sizeof(bsp_config_data), 0, &bsp_size_data);
     if(bsp_size_data != sizeof(bsp_config_data))
     {
       trace_new(TRACE_KAL | TRACE_LEVEL_ALWAYS, "Can't write BSP config info to flash!\n");
       bRetcode = FALSE;
     }
   }

   return(bRetcode);
}
#else
/**********************************************************************/
/* If we build the code as part of an image which does not have our   */
/* non-volatile storage driver in it, we just pretend to have written */
/* the values.                                                        */
/**********************************************************************/
bool config_write_data(void)
{
  return(TRUE);
}
#endif /* DRIVER_INCL_NVSTORE */
#endif /* LOADER */

/***********************************/
/***********************************/
/**                               **/
/** Internal functions start here **/
/**                               **/
/***********************************/
/***********************************/

#if (EMULATION_LEVEL == FINAL_HARDWARE)
/********************************************************************/
/*  FUNCTION:    config_manager_init                                */
/*                                                                  */
/*  PARAMETERS:  None                                               */
/*                                                                  */
/*  DESCRIPTION: Initialise the config manager component. Set the   */
/*               global config data structure contents and create   */
/*               all resources required by the driver.              */
/*                                                                  */
/*  RETURNS:     TRUE on success, FALSE on error                    */
/*                                                                  */
/*  CONTEXT:     Must be called in task context                     */
/*                                                                  */
/********************************************************************/
#define CONFIG_DELAY 1500         /* milsec delay after display change */
bool config_manager_init(void)
{
   #ifndef DRIVER_INCL_OTV12CTL
   u_int32 iReadSabine;
   u_int32 iReadBsp;
   u_int32 iCheckSabine;
   u_int32 iCheckBsp;
   u_int32 kal_size_data;
   u_int32 bsp_size_data;
   #endif
   bool    bResetDefaults = FALSE;
   sabine_config_data *sabine_config_defaults;
   void    *bsp_config_defaults;
   #if defined(DRIVER_INCL_GPIOBTNS) || defined(DRIVER_INCL_SCANBTNS) || defined(DRIVER_INCL_ONEBTN)
   button_ftable btnfuncs;
   bool          bRetcode;
   #endif

   trace_new(TRACE_KAL | TRACE_LEVEL_2, "Initialising config manager.\n");

   /* Set the initial default values */
   sabine_config_defaults = &sabine_defaults;

   /* Get the state of several front panel buttons and, if necesary, override */
   /* the selected video output format based upon these.                      */

   LEDString("    ");

   /* NB: We ignore all front panel overrides in the OpenTV 1.2 image */
   /* when we build the release version of the code with EPG          */
   /* @@@SG Changed so that Conexant (KLONDIKE) retail EPG builds can */
   /* have front panel overrides to allow DVB Baseband selection      */
   /* Without this we cannot test retail EPG builds in Austin         */
   #if defined (DEBUG) || !defined(NDS_EPG) || (CUSTOMER == CNXT)

   /* if both button drivers are included, use the scan matrix driver */
   #if (defined(DRIVER_INCL_GPIOBTNS) && defined(DRIVER_INCL_SCANBTNS))
       bRetcode = scan_button_get_pointers(&btnfuncs);
   #else
      /* Only 1 keyboard driver is included in the build so we can safely call */
      /* the generic entry point (which is the same entry point as scan_button_get_pointers.)  */
      #if (defined(DRIVER_INCL_GPIOBTNS) || defined(DRIVER_INCL_SCANBTNS))
         bRetcode = button_get_pointers(&btnfuncs);
      #endif
   #endif /* Both button drivers included */

   #if defined(DRIVER_INCL_GPIOBTNS) || defined(DRIVER_INCL_SCANBTNS) || defined(DRIVER_INCL_ONEBTN)
   if (bRetcode)
   {
      if((btnfuncs.button_is_pressed)(BTN_CODE_SELECT))
      {
           if((btnfuncs.button_is_pressed)(BTN_CODE_LEFT))
           {
              #ifdef DRIVER_INCL_NDSTESTS
              trace_new(TRACE_KAL | TRACE_LEVEL_3, "Resetting config data to NDS RFin defaults\n");
              LEDStringAt("NDS RFin Config Data",0,0);
              #endif
              
              #ifdef DRIVER_INCL_OTV12CTL 
              trace_new(TRACE_KAL | TRACE_LEVEL_3, "Resetting config data to European defaults\n");
              LEDStringAt("OpenTV1.2 PAL Config Data",0,0);
              #else
              trace_new(TRACE_KAL | TRACE_LEVEL_3, "Resetting config data to USA defaults\n");
              LEDStringAt("USA Config Data",0,0);
              #endif
              
              LEDStringAt("being used now!",0,1);
              bResetDefaults = TRUE;
              sabine_config_defaults = &sabine_defaults;
              task_time_sleep(CONFIG_DELAY);  /* delay long enough to see the Confmgr LED display */
              LEDClearDisplay();
           }
           if((btnfuncs.button_is_pressed)(BTN_CODE_RIGHT))
           {
              trace_new(TRACE_KAL | TRACE_LEVEL_3, "SELECT+RIGHT is not being used\n");
              LEDStringAt("NOT USED YET",0,0);
              task_time_sleep(CONFIG_DELAY);  /* delay long enough to see the Confmgr LED display */
              LEDClearDisplay();
           }
         
           if((btnfuncs.button_is_pressed)(BTN_CODE_UP))
           {
              trace_new(TRACE_KAL | TRACE_LEVEL_3, "Source override: RF\n");
              LEDStringAt("RF Input NIM",0,0);
              LEDStringAt("being used now!",0,1);
              iOverrideSource = NIM_SATELLITE_DEMOD;
              task_time_sleep(CONFIG_DELAY);  /* delay long enough to see the Confmgr LED display */
              LEDClearDisplay();
           }
           if((btnfuncs.button_is_pressed)(BTN_CODE_DOWN))
           {
              trace_new(TRACE_KAL | TRACE_LEVEL_3, "Source override: DVB\n");
              LEDStringAt("DB Input NIM",0,0);
              LEDStringAt("being used now!",0,1);
              iOverrideSource = NIM_DVB_BASEBAND;
              task_time_sleep(CONFIG_DELAY);  /* delay long enough to see the Confmgr LED display */
              LEDClearDisplay();
           }
      }
      else 
      {
        if((btnfuncs.button_is_pressed)(BTN_CODE_RIGHT))
        {
           trace_new(TRACE_KAL | TRACE_LEVEL_3, "Video Output Format override: SECAM\n");
           LEDStringAt("SECA Video Output",0,0);
           LEDStringAt("being used now!",0,1);
           iOverrideVideoType = SECAM;
           task_time_sleep(CONFIG_DELAY);  /* delay long enough to see the Confmgr LED display */
           LEDClearDisplay();
        }

        if((btnfuncs.button_is_pressed)(BTN_CODE_DOWN))
        {
           trace_new(TRACE_KAL | TRACE_LEVEL_3, "Video Output Format override: PAL\n");
           LEDStringAt("PAL Video Output",0,0);
           LEDStringAt("being used now!",0,1);
           iOverrideVideoType = PAL;
           task_time_sleep(CONFIG_DELAY);  /* delay long enough to see the Confmgr LED display */
           LEDClearDisplay();
        }

        if((btnfuncs.button_is_pressed)(BTN_CODE_UP))
        {
           trace_new(TRACE_KAL | TRACE_LEVEL_3, "Video Output Format override: NTSC\n");
           LEDStringAt("NTSC Video Output",0,0);
           LEDStringAt("being used now!",0,1);
           iOverrideVideoType = NTSC;
           task_time_sleep(CONFIG_DELAY);  /* delay long enough to see the Confmgr LED display */
           LEDClearDisplay();
        }
      }

    /* Emergency case - erase all NV storage and default to European settings */
    #ifdef DRIVER_INCL_NVSTORE
    if((btnfuncs.button_is_pressed)(BTN_CODE_MENU))
    {
       trace_new(TRACE_KAL | TRACE_LEVEL_3, "Clearing non-volatile storage\n");
       LEDStringAt("Clearing NonVolatile",0,0);
       LEDStringAt("storage now!",0,1);
       bResetDefaults = TRUE;
       sabine_config_defaults = &sabine_defaults;
       nv_erase();
       task_time_sleep(CONFIG_DELAY);  /* delay long enough to see the Confmgr LED display */
       LEDClearDisplay();
    }
    #endif /* DRIVER_INCL_NVSTORE */
   }
   #endif /* #if defined(DRIVER_INCL_GPIOBTNS) || defined(DRIVER_INCL_SCANBTNS) || defined(DRIVER_INCL_ONEBTN) */
   
   #endif /* Special cases where we remove the button check */
   
   #ifdef LOADER
   iOverrideSource = NIM_OPENTV_BASEBAND;
   #endif

   if(iOverrideVideoType != FROM_CONFIG)
   {
     sabine_config_defaults->video_standard = iOverrideVideoType;
   }

   if(iOverrideSource != FROM_CONFIG)
   {
     sabine_config_defaults->nim_type = iOverrideSource;
   }

   /* Create the access semaphore */
   semConfigAccess = sem_create(1, "CDSM");
   if(semConfigAccess == 0)
   {
      trace_new(TRACE_KAL | TRACE_LEVEL_ALWAYS, "Can't create config access semaphore!\n");
      error_log(ERROR_FATAL | RC_KAL_NORESOURCES);
   }

   semBspConfigAccess = sem_create(1, "BDSM");
   if(semBspConfigAccess == 0)
   {
      trace_new(TRACE_KAL | TRACE_LEVEL_ALWAYS, "Can't create bsp config access semaphore!\n");
      error_log(ERROR_FATAL | RC_KAL_NORESOURCES);
   }

   /* Copy the default data into the "live" buffers */
   memcpy(&sabine_config, sabine_config_defaults, sizeof(sabine_config_data));
   GetDefaultBSPCfg(&bsp_config_defaults);
   memcpy(&bsp_config, bsp_config_defaults, sizeof(bsp_config_data));

   /* Fix up checksum and magic number fields */

   sabine_config.checksum = NewCheckSum(&sabine_config.magic, sizeof(sabine_config) - 2*sizeof(u_int32));
   bsp_config.checksum = NewCheckSum(&bsp_config.magic, sizeof(bsp_config) - 2*sizeof(u_int32));
   bsp_config.magic = MAGIC;

   /* Copy the latest version from flash on top of our config copy (this  */
   /* allows us to handle downlevel versions of the structure in flash    */
   /* assuming that no-one has added or removed fields in the middle of   */
   /* the structure).                                                     */

   #ifdef DRIVER_INCL_NVSTORE
   if(!bResetDefaults)
   {
     #ifndef LOADER
     iReadSabine  = nv_read(NV_KAL_CLIENT_ID, &sabine_config, sizeof(sabine_config_data), 0, &kal_size_data);
     iCheckSabine = NewCheckSum(&sabine_config.magic, sizeof(sabine_config) - 2*sizeof(u_int32));
     iReadBsp     = nv_read(NV_BSP_CLIENT_ID, &bsp_config, sizeof(bsp_config_data), 0, &bsp_size_data);
     iCheckBsp    = NewCheckSum(&bsp_config.magic, sizeof(bsp_config) - 2*sizeof(u_int32));
     #endif
     /* Make sure the checksums in the new data are valid. If not, use defaults */

     if((kal_size_data != sabine_config.length) ||
        (sabine_config.magic != MAGIC)        ||
        (iCheckSabine != sabine_config.checksum))
     {
       trace_new(TRACE_KAL | TRACE_LEVEL_2, "Config data corrupt. Resetting defaults\n");
       memcpy(&sabine_config, &sabine_defaults, sizeof(sabine_config_data));
     }
     else
     {
       if(iOverrideVideoType != FROM_CONFIG)
         sabine_config.video_standard = iOverrideVideoType;

       if(iOverrideSource != FROM_CONFIG)
       {
         sabine_config.nim_type = iOverrideSource;
       }

     }

     if((bsp_config.magic != MAGIC)        ||
        (iCheckBsp != bsp_config.checksum))
     {
       trace_new(TRACE_KAL | TRACE_LEVEL_2, "BSP config data corrupt. Resetting defaults\n");
       GetDefaultBSPCfg(&bsp_config_defaults);
       memcpy(&bsp_config, bsp_config_defaults, sizeof(bsp_config_data));
       bsp_config.checksum = NewCheckSum(&bsp_config.magic, sizeof(bsp_config) - 2*sizeof(u_int32));
       bsp_config.magic = MAGIC;
     }

     /* Update the length and checksum in case a downlevel version */
     /* was read from the flash                                    */
     sabine_config.length = sizeof(sabine_config_data);
     sabine_config.checksum = NewCheckSum(&sabine_config.magic, sizeof(sabine_config) - 2*sizeof(u_int32));
     #ifndef LOADER
     config_write_data();
     #endif
   }
   else
   {
      trace_new(TRACE_KAL | TRACE_LEVEL_3, "Writing default config options to flash\n");
      #ifndef LOADER
      config_write_data();
      #endif
   }
   #endif /* DRIVER_INCL_NVSTORE */

   return(TRUE);
}
#else /* EMULATION_LEVEL != FINAL_HARDWARE */

/********************************************************************/
/*  FUNCTION:    config_manager_init                                */
/*                                                                  */
/*  PARAMETERS:  None                                               */
/*                                                                  */
/*  DESCRIPTION: Initialise the config manager component. Set the   */
/*               global config data structure contents and create   */
/*               all resources required by the driver.              */
/*                                                                  */
/*               This is a special version of the function for use  */
/*               in emulation systems where there is no keyboard    */
/*               and no (real) NV storage.                          */
/*                                                                  */
/*  RETURNS:     TRUE on success, FALSE on error                    */
/*                                                                  */
/*  CONTEXT:     Must be called in task context                     */
/*                                                                  */
/********************************************************************/
bool config_manager_init(void)
{
   sabine_config_data *sabine_config_defaults;
   void    *bsp_config_defaults;

   trace_new(TRACE_KAL | TRACE_LEVEL_2, "Initialising config manager.\n");

   /* Set the initial default values */
   sabine_config_defaults = &sabine_defaults;

   /* Create the access semaphore */
   semConfigAccess = sem_create(1, "CDSM");
   if(semConfigAccess == 0)
   {
      trace_new(TRACE_KAL | TRACE_LEVEL_ALWAYS, "Can't create config access semaphore!\n");
      error_log(ERROR_FATAL | RC_KAL_NORESOURCES);
   }

   semBspConfigAccess = sem_create(1, "BDSM");
   if(semBspConfigAccess == 0)
   {
      trace_new(TRACE_KAL | TRACE_LEVEL_ALWAYS, "Can't create bsp config access semaphore!\n");
      error_log(ERROR_FATAL | RC_KAL_NORESOURCES);
   }

   /* Copy the default data into the "live" buffers */
   memcpy(&sabine_config, sabine_config_defaults, sizeof(sabine_config_data));
   GetDefaultBSPCfg(&bsp_config_defaults);
   memcpy(&bsp_config, bsp_config_defaults, sizeof(bsp_config_data));

   /* Fix up checksum and magic number fields */

   sabine_config.checksum = NewCheckSum(&sabine_config.magic, sizeof(sabine_config) - 2*sizeof(u_int32));
   bsp_config.checksum = NewCheckSum(&bsp_config.magic, sizeof(bsp_config) - 2*sizeof(u_int32));
   bsp_config.magic = MAGIC;

   return(TRUE);
}
#endif

/****************************************************************************
 * Modifications:
 * $Log: 
 *  63   mpeg      1.62        11/5/03 2:28:58 PM     Song Qiao       CR(s): 
 *        7752 7824 Added inband video downstream power calibration setting 
 *        default values to sabine_defaults.
 *        
 *  62   mpeg      1.61        10/30/03 10:49:21 AM   Tim Ross        CR(s): 
 *        7743 7744 Updated cable default channel info to match ADC headend. 
 *        Removed channel override when 'Guide' is pressed that was supposed to
 *         be
 *        specific for Wabash but was actually being applied to all IRDs.
 *  61   mpeg      1.60        9/26/03 1:57:16 PM     Billy Jackman   SCR(s) 
 *        7555 :
 *        Handle config manager entries as native types.
 *        
 *  60   mpeg      1.59        9/25/03 6:25:04 PM     Billy Jackman   SCR(s) 
 *        7553 :
 *        Remove explicit includes of the problematic files.
 *        
 *  59   mpeg      1.58        8/8/03 1:56:54 PM      Angela Swartz   SCR(s) 
 *        7210 7209 :
 *        restore the feature of guild button for wabash; set the default 
 *        params to tune MTV2 stream
 *        
 *  58   mpeg      1.57        8/4/03 5:54:06 PM      Angela Swartz   SCR(s) 
 *        7137 7138 :
 *        removed the European defaults;
 *        moved the default config data that is specific to the build to the 
 *        headers for easy lookup; 
 *        remove the use of SELECT+RIGHT keypress option, it can be used in the
 *         future
 *        
 *  57   mpeg      1.56        7/23/03 7:34:02 PM     Sunil Cheruvu   SCR(s): 
 *        6681 
 *        Note: Tracker 6680 is linked to this Tracker.  Added the US and DS 
 *        Power calibration infrastructure defaults for OC_OOBFE.
 *        
 *  56   mpeg      1.55        7/10/03 9:02:56 PM     Song Qiao       SCR(s): 
 *        6912 
 *        Added default values for MacAddr.
 *        
 *        
 *  55   mpeg      1.54        7/8/03 4:27:12 PM      Miles Bintz     SCR(s) 
 *        6807 :
 *        milano has two serial ports so share the second with either mpeg or 
 *        CM
 *        
 *  54   mpeg      1.53        6/30/03 2:26:46 PM     Billy Jackman   SCR(s) 
 *        5816 :
 *        Modified configuration initial values to reflect all LNB settings and
 *         to set
 *        terrestrial transponder frequency.
 *        
 *  53   mpeg      1.52        6/25/03 6:22:48 PM     Billy Jackman   SCR(s) 
 *        6844 :
 *        Got rid of some now un-needed workaround code for symbol conflicts 
 *        between
 *        opentvx.h and demod_types.h.
 *        Changed initializers to conform to the new sabine_config_data 
 *        structure.
 *        Made some #includes unconditional so that data types and definitions 
 *        would be
 *        available for all configurations.
 *        
 *  52   mpeg      1.51        5/30/03 5:45:00 PM     Tim White       SCR(s) 
 *        6632 6633 :
 *        Added specific NDS RFin string.  Added LCD clears following the 
 *        timeout of the
 *        confmgr display information.
 *        
 *        
 *  51   mpeg      1.50        5/14/03 2:26:50 PM     Matt Korte      SCR(s) 
 *        6325 6326 :
 *        Fixed FP display and confmgr init issues
 *        
 *  50   mpeg      1.49        5/9/03 5:57:08 PM      Sahil Bansal    SCR(s): 
 *        6286 6287 
 *        Added support for cm mode selection to watchtv
 *        
 *  49   mpeg      1.48        4/26/03 5:26:38 PM     Sahil Bansal    SCR(s): 
 *        6110 
 *        Added 2 NVRAM parms for OC_OOBFE driver
 *        
 *  48   mpeg      1.47        4/25/03 2:53:04 PM     Tim White       SCR(s) 
 *        5949 :
 *        Added support for RF cable input for Wabash IRD NDTESTS.
 *        
 *        
 *  47   mpeg      1.46        4/24/03 4:58:12 PM     Tim White       SCR(s) 
 *        5949 :
 *        Allow NDSTESTS to use RF input.
 *        
 *        
 *  46   mpeg      1.45        2/13/03 11:27:38 AM    Matt Korte      SCR(s) 
 *        5479 :
 *        Removed old header reference
 *        
 *  45   mpeg      1.44        2/6/03 2:22:24 PM      Angela Swartz   SCR(s) 
 *        5410 :
 *        added RF modulator default setting(PASSTHRU) to the device_flags 
 *        field for none-1.2 and NTSC build
 *        
 *  44   mpeg      1.43        1/24/03 9:54:30 AM     Dave Moore      SCR(s) 
 *        5305 :
 *        removed FREQ_SCREEN
 *        
 *        
 *  43   mpeg      1.42        12/18/02 11:02:36 AM   Dave Wilson     SCR(s) 
 *        5185 :
 *        Removed redundant emulation code and generally simplified things, 
 *        removing
 *        a lot of unneeded #ifdefs in the process.
 *        
 *  42   mpeg      1.41        8/22/02 8:00:50 PM     Miles Bintz     SCR(s) 
 *        4465 :
 *        Changed the default value for spectral inversion (cable settings) 
 *        from 0 to SPECTRUM_NORMAL
 *        
 *        
 *  41   mpeg      1.40        8/22/02 12:17:36 PM    Miles Bintz     SCR(s) 
 *        4452 :
 *        When building opentv, different header files were included.  There 
 *        was a name conflict between a #define and an enumeration.
 *        
 *  40   mpeg      1.39        8/21/02 12:09:24 PM    Miles Bintz     SCR(s) 
 *        4436 :
 *        Added annex a/b defaults.  Change USA defaults to CMT and "Clr" 
 *        defaults to MTV2 stream.  
 *        
 *  39   mpeg      1.38        8/13/02 1:23:06 PM     Miles Bintz     SCR(s) 
 *        4376 :
 *        Changed QAM mode default from 1 to 64 to match previous change in an 
 *        enumeration.  Also added USA defaults so you can force NTSC on the 
 *        front panel.  Lastly, added default program number in SVL_PROGRAM.
 *        
 *  38   mpeg      1.37        7/25/02 1:58:40 PM     Dave Wilson     SCR(s) 
 *        2775 :
 *        Changed references to the Astra service list manager to the new 
 *        transport
 *        stream manager.
 *        
 *  37   mpeg      1.36        6/19/02 5:06:20 PM     Miles Bintz     SCR(s) 
 *        4001 :
 *        Set defaults to cable #if IRD_HAS_CABLE_DEMOD
 *        
 *  36   mpeg      1.35        6/3/02 2:38:40 PM      Ray Mack        SCR(s) 
 *        3920 :
 *        changed all frequency defaults to only be significant to MHz.  
 *        Cleaned up very old comments and took out C++ style comments.
 *        
 *  35   mpeg      1.34        3/1/02 9:10:06 AM      Ray Mack        SCR(s) 
 *        3110 :
 *        Changed default channel number from 100 to 101.  Also changed the 
 *        comments to reflect the actual values in the data structures.  THe 
 *        default transponder for dmfront use is now 12.36980 GHz now (changed 
 *        a few days ago by Bob VanGulik).
 *        
 *  34   mpeg      1.33        2/19/02 3:33:30 PM     Bob Van Gulick  SCR(s) 
 *        3173 :
 *        Change default transponder for USA to 12.36980
 *        
 *        
 *  33   mpeg      1.32        1/3/02 1:42:10 PM      Miles Bintz     SCR(s) 
 *        2933 :
 *        Potential fix for build break from apps that didn't include one of 
 *        the button drivers.
 *        
 *  32   mpeg      1.31        12/18/01 3:16:30 PM    Miles Bintz     SCR(s) 
 *        2933 :
 *        merged in wabash branch changes
 *        
 *        
 *  31   mpeg      1.30        7/3/01 10:32:52 AM     Tim White       SCR(s) 
 *        2178 2179 2180 :
 *        Merge branched Hondo specific code back into the mainstream source 
 *        database.
 *        
 *        
 *  30   mpeg      1.29        3/19/01 10:59:26 AM    Tim Ross        DCS966. 
 *        Added changes necessary for emulating hondo.
 *        
 *  29   mpeg      1.28        3/4/01 12:42:34 AM     Steve Glennon   Changed 
 *        so that override buttons are allowed for release EPG Conexant build.
 *        This allows us to debug and test the "retail" EPG build in Austin as
 *        we need to be able to use the override buttons to select DVB 
 *        Basevband in.
 *        
 *  28   mpeg      1.27        2/22/01 11:39:34 AM    Ismail Mustafa  Added NDS
 *         parameters for NDSTESTS USA defaults. DCS #1282.
 *        
 *  27   mpeg      1.26        2/16/01 5:17:06 PM     Dave Wilson     DCS1217: 
 *        Changes for Vendor D rev 7 IRD
 *        
 *  26   mpeg      1.25        2/15/01 4:15:42 PM     Ismail Mustafa  DCS 
 *        #1235, #1236, #1237. Added NDSTESTS default config info.
 *        
 *  25   mpeg      1.24        1/4/01 11:43:32 AM     Anzhi Chen      Renamed 
 *        CheckSum() as NewCheckSum() to avoid conflict with CheckSum() in
 *        PSOSCFG dir.
 *        
 *  24   mpeg      1.23        1/3/01 4:17:06 PM      Ismail Mustafa  Added 
 *        Checksum().
 *        
 *  23   mpeg      1.22        12/5/00 9:40:34 AM     Dave Wilson     Corrected
 *         #ifdef logic to compile out overrides in non-debug EPG builds of 1.2
 *        
 *  22   mpeg      1.21        11/30/00 5:14:30 AM    Dave Wilson     Removed 
 *        front panel button overrides for secure, non debug builds
 *        
 *  21   mpeg      1.20        9/18/00 5:22:54 PM     Tim White       Echostar 
 *        changed the transponder frequency again to 12.253160
 *        
 *  20   mpeg      1.19        9/14/00 3:41:08 PM     Tim White       Echostar 
 *        changed the free-to-air transponder from 12.5156 to 12.34064 last 
 *        night.
 *        
 *  19   mpeg      1.18        8/14/00 5:20:26 PM     Dave Wilson     Changed 
 *        OpenTV 1.2 default source to satellite NIM
 *        
 *  18   mpeg      1.17        7/13/00 1:45:54 PM     Dave Wilson     For 
 *        OpenTV 1.2, don't even think about saving anything to NV storage
 *        
 *  17   mpeg      1.16        7/10/00 5:59:36 PM     Dave Wilson     For 
 *        OpenTV 1.2, no attempt is made to access parameters in NV storage.
 *        
 *  16   mpeg      1.15        6/9/00 11:54:52 AM     Tim White       Added 
 *        code for Echostar bringup.
 *        
 *  15   mpeg      1.14        5/23/00 3:13:26 PM     Dave Wilson     Changed 
 *        trace flags for OpenTV 1.2 build
 *        
 *  14   mpeg      1.13        5/17/00 3:24:20 PM     Dave Wilson     Added 
 *        OpenTV EN2 defaults to select OpenTV baseband input
 *        
 *  13   mpeg      1.12        5/1/00 11:54:12 AM     Dave Wilson     Revreted 
 *        to setting European values by default
 *        Fixed OpenTV 1.2 default to use DVB baseband input
 *        
 *  12   mpeg      1.11        4/25/00 11:28:16 PM    Tim White       Changed 
 *        to NTSC (USA) defaults.
 *        
 *  11   mpeg      1.10        4/25/00 4:11:02 PM     Dave Wilson     Changed 
 *        default video format to composite to cater for least common 
 *        denominator systems
 *        
 *  10   mpeg      1.9         4/13/00 10:11:36 AM    Dave Wilson     Now uses 
 *        low level button driver rather than OpenTV driver.
 *        
 *  9    mpeg      1.8         3/20/00 10:08:40 AM    Dave Wilson     Added 
 *        changes to allow compilation without NVSTORE.
 *        
 *  8    mpeg      1.7         3/1/00 6:41:14 PM      Dave Wilson     Changed 
 *        default FEC and polarisation values to use new labels from DEMOD.H
 *        
 *  7    mpeg      1.6         2/16/00 10:46:42 AM    Senthil Veluswamy Changed
 *         file to use { } to fix the DEBUG=NO trace problem
 *        - took the code back to earlier state and just added braces.
 *        
 *  6    mpeg      1.5         2/15/00 11:41:30 PM    Senthil Veluswamy Changed
 *         config_unlock_data to work in DEBUG=NO builds.
 *        
 *  5    mpeg      1.4         12/14/99 2:33:00 PM    Dave Wilson     Added new
 *         fields for service list handling
 *        
 *  4    mpeg      1.3         11/2/99 12:04:36 PM    Dave Wilson     Fixed 
 *        typo - changed FREQ_SCAN to FREQ_SCREEN
 *        
 *  3    mpeg      1.2         11/1/99 6:26:02 PM     Tim Ross        Added 
 *        call to GetDefaultBSPCfg() instead of referencing the 
 *        STARTUP data directly.
 *        Corrected bug w/ blanking the LEDs on non-frequency scanning
 *        builds.
 *        
 *  2    mpeg      1.1         11/1/99 4:09:04 PM     Tim Ross        Made LED 
 *        clearing conditional based on frequency scanning.
 *        
 *  1    mpeg      1.0         10/29/99 12:31:54 PM   Dave Wilson     
 * $
 * 
 *    Rev 1.60   26 Sep 2003 12:57:16   jackmaw
 * SCR(s) 7555 :
 * Handle config manager entries as native types.
 * 
 *    Rev 1.59   25 Sep 2003 17:25:04   jackmaw
 * SCR(s) 7553 :
 * Remove explicit includes of the problematic files.
 * 
 *    Rev 1.58   08 Aug 2003 12:56:54   swartzwg
 * SCR(s) 7210 7209 :
 * restore the feature of guild button for wabash; set the default params to tune MTV2 stream
 * 
 *    Rev 1.57   04 Aug 2003 16:54:06   swartzwg
 * SCR(s) 7137 7138 :
 * removed the European defaults;
 * moved the default config data that is specific to the build to the headers for easy lookup; 
 * remove the use of SELECT+RIGHT keypress option, it can be used in the future
 * 
 *    Rev 1.56   23 Jul 2003 18:34:02   cheruvs
 * SCR(s): 6681 
 * Note: Tracker 6680 is linked to this Tracker.  Added the US and DS Power calibration infrastructure defaults for OC_OOBFE.
 * 
 *    Rev 1.55   10 Jul 2003 20:02:56   qiaos
 * SCR(s): 6912 
 * Added default values for MacAddr.
 * 
 * 
 *    Rev 1.54   08 Jul 2003 15:27:12   bintzmf
 * SCR(s) 6807 :
 * milano has two serial ports so share the second with either mpeg or CM
 * 
 *    Rev 1.53   30 Jun 2003 13:26:46   jackmaw
 * SCR(s) 5816 :
 * Modified configuration initial values to reflect all LNB settings and to set
 * terrestrial transponder frequency.
 * 
 *    Rev 1.52   25 Jun 2003 17:22:48   jackmaw
 * SCR(s) 6844 :
 * Got rid of some now un-needed workaround code for symbol conflicts between
 * opentvx.h and demod_types.h.
 * Changed initializers to conform to the new sabine_config_data structure.
 * Made some #includes unconditional so that data types and definitions would be
 * available for all configurations.
 * 
 *    Rev 1.51   30 May 2003 16:45:00   whiteth
 * SCR(s) 6632 6633 :
 * Added specific NDS RFin string.  Added LCD clears following the timeout of the
 * confmgr display information.
 * 
 * 
 *    Rev 1.50   14 May 2003 13:26:50   kortemw
 * SCR(s) 6325 6326 :
 * Fixed FP display and confmgr init issues
 * 
 *    Rev 1.49   09 May 2003 16:57:08   bansals
 * SCR(s): 6286 6287 
 * Added support for cm mode selection to watchtv
 * 
 *    Rev 1.48   26 Apr 2003 16:26:38   bansals
 * SCR(s): 6110 
 * Added 2 NVRAM parms for OC_OOBFE driver
 * 
 *    Rev 1.47   25 Apr 2003 13:53:04   whiteth
 * SCR(s) 5949 :
 * Added support for RF cable input for Wabash IRD NDTESTS.
 * 
 * 
 *    Rev 1.46   24 Apr 2003 15:58:12   whiteth
 * SCR(s) 5949 :
 * Allow NDSTESTS to use RF input.
 * 
 * 
 *    Rev 1.45   13 Feb 2003 11:27:38   kortemw
 * SCR(s) 5479 :
 * Removed old header reference
 * 
 *    Rev 1.44   06 Feb 2003 14:22:24   swartzwg
 * SCR(s) 5410 :
 * added RF modulator default setting(PASSTHRU) to the device_flags field for none-1.2 and NTSC build
 * 
 *    Rev 1.43   24 Jan 2003 09:54:30   mooreda
 * SCR(s) 5305 :
 * removed FREQ_SCREEN
 * 
 * 
 *    Rev 1.42   18 Dec 2002 11:02:36   dawilson
 * SCR(s) 5185 :
 * Removed redundant emulation code and generally simplified things, removing
 * a lot of unneeded #ifdefs in the process.
 * 
 *    Rev 1.41   22 Aug 2002 19:00:50   bintzmf
 * SCR(s) 4465 :
 * Changed the default value for spectral inversion (cable settings) from 0 to SPECTRUM_NORMAL
 * 
 * 
 *    Rev 1.40   22 Aug 2002 11:17:36   bintzmf
 * SCR(s) 4452 :
 * When building opentv, different header files were included.  There was a name conflict between a #define and an enumeration.
 * 
 *    Rev 1.39   21 Aug 2002 11:09:24   bintzmf
 * SCR(s) 4436 :
 * Added annex a/b defaults.  Change USA defaults to CMT and "Clr" defaults to MTV2 stream.  
 * 
 *    Rev 1.38   13 Aug 2002 12:23:06   bintzmf
 * SCR(s) 4376 :
 * Changed QAM mode default from 1 to 64 to match previous change in an enumeration.  Also added USA defaults so you can force NTSC on the front panel.  Lastly, added default program number in SVL_PROGRAM.
 * 
 *    Rev 1.37   25 Jul 2002 12:58:40   dawilson
 * SCR(s) 2775 :
 * Changed references to the Astra service list manager to the new transport
 * stream manager.
 * 
 *    Rev 1.36   19 Jun 2002 16:06:20   bintzmf
 * SCR(s) 4001 :
 * Set defaults to cable #if IRD_HAS_CABLE_DEMOD
 * 
 *    Rev 1.35   03 Jun 2002 13:38:40   raymack
 * SCR(s) 3920 :
 * changed all frequency defaults to only be significant to MHz.  Cleaned up very old comments and took out C++ style comments.
 * 
 *    Rev 1.34   01 Mar 2002 09:10:06   raymack
 * SCR(s) 3110 :
 * Changed default channel number from 100 to 101.  Also changed the comments to reflect the actual values in the data structures.  THe default transponder for dmfront use is now 12.36980 GHz now (changed a few days ago by Bob VanGulik).
 * 
 *    Rev 1.33   19 Feb 2002 15:33:30   vangulr
 * SCR(s) 3173 :
 * Change default transponder for USA to 12.36980
 * 
 * 
 *    Rev 1.32   03 Jan 2002 13:42:10   bintzmf
 * SCR(s) 2933 :
 * Potential fix for build break from apps that didn't include one of the button drivers.
 * 
 *    Rev 1.31   18 Dec 2001 15:16:30   bintzmf
 * SCR(s) 2933 :
 * merged in wabash branch changes
 * 
 * 
 *    Rev 1.30.1.0   07 Dec 2001 15:00:02   bintzmf
 * SCR(s) 2933 :
 * Removed board_and_vendor_id() call/check
 * 
 * 
 *    Rev 1.30   03 Jul 2001 09:32:52   whiteth
 * SCR(s) 2178 2179 2180 :
 * Merge branched Hondo specific code back into the mainstream source database.
 * 
 *
 *    Rev 1.29.1.1   07 May 2001 13:20:50   prattac
 * Removed support for THOR boards.
 * Changed USA defaults to expect new (without trackball) remote.
 * 
 *    Rev 1.29.1.0   04 May 2001 13:25:26   prattac
 * Added support for ABILENE
 * 
 *    Rev 1.29   19 Mar 2001 10:59:26   rossst
 * DCS966. Added changes necessary for emulating hondo.
 * 
 *    Rev 1.28.1.0   03 May 2001 14:53:20   kroescjl
 * SCR 1844: added modem trace by default
 *
 *  Rev 1.28   04 Mar 2001 00:42:34   glennon
 * Changed so that override buttons are allowed for release EPG Conexant build.
 * This allows us to debug and test the "retail" EPG build in Austin as
 * we need to be able to use the override buttons to select DVB Basevband in.
 * 
 *    Rev 1.27   22 Feb 2001 11:39:34   mustafa
 * Added NDS parameters for NDSTESTS USA defaults. DCS #1282.
 * 
 *    Rev 1.26   16 Feb 2001 17:17:06   dawilson
 * DCS1217: Changes for Vendor D rev 7 IRD
 * 
 *    Rev 1.25   15 Feb 2001 16:15:42   mustafa
 * DCS #1235, #1236, #1237. Added NDSTESTS default config info.
 * 
 *    Rev 1.24   04 Jan 2001 11:43:32   achen
 * Renamed CheckSum() as NewCheckSum() to avoid conflict with CheckSum() in
 * PSOSCFG dir.
 * 
 *    Rev 1.23   03 Jan 2001 16:17:06   mustafa
 * Added Checksum().
 * 
 *    Rev 1.22   05 Dec 2000 09:40:34   dawilson
 * Corrected #ifdef logic to compile out overrides in non-debug EPG builds of 1.2
 * 
 *    Rev 1.21   30 Nov 2000 05:14:30   dawilson
 * Removed front panel button overrides for secure, non debug builds
 * 
 *    Rev 1.20   18 Sep 2000 16:22:54   whiteth
 * Echostar changed the transponder frequency again to 12.253160
 * 
 *    Rev 1.19   14 Sep 2000 14:41:08   whiteth
 * Echostar changed the free-to-air transponder from 12.5156 to 12.34064 last night.
 * 
 *    Rev 1.18   14 Aug 2000 16:20:26   dawilson
 * Changed OpenTV 1.2 default source to satellite NIM
 * 
 *    Rev 1.17   13 Jul 2000 12:45:54   dawilson
 * For OpenTV 1.2, don't even think about saving anything to NV storage
 * 
 *    Rev 1.16   10 Jul 2000 16:59:36   dawilson
 * For OpenTV 1.2, no attempt is made to access parameters in NV storage.
 * 
 *    Rev 1.15   09 Jun 2000 10:54:52   whiteth
 * Added code for Echostar bringup.
 * 
 *    Rev 1.14   23 May 2000 14:13:26   dawilson
 * Changed trace flags for OpenTV 1.2 build
 * 
 *    Rev 1.13   17 May 2000 14:24:20   dawilson
 * Added OpenTV EN2 defaults to select OpenTV baseband input
 * 
 *    Rev 1.12   01 May 2000 10:54:12   dawilson
 * Revreted to setting European values by default
 * Fixed OpenTV 1.2 default to use DVB baseband input
 * 
 *    Rev 1.11   25 Apr 2000 22:28:16   whiteth
 * Changed to NTSC (USA) defaults.
 * 
 *    Rev 1.10   25 Apr 2000 15:11:02   dawilson
 * Changed default video format to composite to cater for least common 
 * denominator systems
 * 
 *    Rev 1.9   13 Apr 2000 09:11:36   dawilson
 * Now uses low level button driver rather than OpenTV driver.
 * 
 *    Rev 1.8   20 Mar 2000 10:08:40   dawilson
 * Added changes to allow compilation without NVSTORE.
 * 
 *    Rev 1.7   01 Mar 2000 18:41:14   dawilson
 * Changed default FEC and polarisation values to use new labels from DEMOD.H
 * 
 *    Rev 1.6   16 Feb 2000 10:46:42   velusws
 * Changed file to use { } to fix the DEBUG=NO trace problem
 * - took the code back to earlier state and just added braces.
 * 
 *    Rev 1.5   15 Feb 2000 23:41:30   velusws
 * Changed config_unlock_data to work in DEBUG=NO builds.
 * 
 *    Rev 1.4   14 Dec 1999 14:33:00   dawilson
 * Added new fields for service list handling
 * 
 *    Rev 1.3   02 Nov 1999 12:04:36   dawilson
 * Fixed typo - changed FREQ_SCAN to FREQ_SCREEN
 * 
 *    Rev 1.2   01 Nov 1999 18:26:02   rossst
 * Added call to GetDefaultBSPCfg() instead of referencing the 
 * STARTUP data directly.
 * Corrected bug w/ blanking the LEDs on non-frequency scanning
 * builds.
 * 
 *    Rev 1.1   01 Nov 1999 16:09:04   rossst
 * Made LED clearing conditional based on frequency scanning.
 * 
 *    Rev 1.0   29 Oct 1999 11:31:54   dawilson
 * Initial revision.
 * 
 ****************************************************************************/

