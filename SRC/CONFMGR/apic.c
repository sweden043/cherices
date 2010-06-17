/****************************************************************************/
/*                            Conexant Systems                              */
/****************************************************************************/
/*                                                                          */
/* Filename:       apic.c                                                   */
/*                                                                          */
/* Description:    Startup API.                                             */
/*                                                                          */
/* Author:         Tim Ross                                                 */
/*                                                                          */
/* Copyright Conexant Systems, 1999                                         */
/* All Rights Reserved.                                                     */
/*                                                                          */
/****************************************************************************/

/******************/
/* Include Files  */
/******************/
#include <basetype.h>
#include <hwconfig.h>
#include <confmgr.h>

/********************************/
/* External Function Prototypes */
/********************************/

/**********************/
/* Local Definitions  */
/**********************/

/***************/
/* Global Data */
/***************/
bsp_config_data DefaultBSPCfg = 
    {
        sizeof(bsp_config_data),
        0,
        MAGIC,
        0,
        0,
        0,
        115200,
        0
    };


/********************************************************************/
/*  GetDefaultBSPCfg                                                */
/*                                                                  */
/*  PARAMETERS:                                                     */
/*      ppBSPCfgInfo - address of ptr to point at default data      */
/*                                                                  */
/*  DESCRIPTION:                                                    */
/*      Modifies caller's ptr to reference default BSP config data. */
/*                                                                  */
/*  RETURNS:                                                        */
/*      Nothing.                                                    */
/********************************************************************/
void GetDefaultBSPCfg(void **pBSPCfgInfo)
{
    *pBSPCfgInfo = (void *)&DefaultBSPCfg;
}    
