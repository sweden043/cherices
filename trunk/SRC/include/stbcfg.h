/****************************************************************************
 *                            Conexant Systems
 ****************************************************************************
 *
 * Filename:       stbcfg.h
 *
 * Description:    Top Level Set Top Box ConFiGuration File for C Files
 *
 * Author:         Bobby Bradford
 *
 * Copyright Conexant Systems, 2001
 * All Rights Reserved.
 *
 ****************************************************************************
 * $Header: stbcfg.h, 3, 12/8/03 3:44:19 PM, Dave Wilson$
 ****************************************************************************/


#ifndef __STBCFG_H__
#define __STBCFG_H__


/****************************************************************************
 * FIRST ... ALWAYS ... Get the HWCONFIG header for hardware level defintions
 ****************************************************************************/
   #include "hwconfig.h"


/****************************************************************************
 * SECOND ... Get the SWCONFIG header for software level definitions
 ****************************************************************************/
   #include "swconfig.h"


/****************************************************************************
 * THIRD ... Add new high-level configurations here, as they are defined
 ****************************************************************************/



/****************************************************************************
 * FOURTH ... Include the generated drv_incl file                          
 ****************************************************************************/
   #include "drv_incl.h"
   
   
/****************************************************************************
 * FIFTH ... Include toolchain-specific macro definitions                 
 ****************************************************************************/
   #include "toolchain.h"
   
#endif   /* #ifndef __STBCFG_H__ */

/****************************************************************************
 * $Log: 
 *  3    mpeg      1.2         12/8/03 3:44:19 PM     Dave Wilson     CR(s) 
 *        8117 : Added include line for toolchain.h. This is intended to 
 *        include any toolchain-specific macro definitions.
 *        
 *  2    mpeg      1.1         9/9/03 4:42:12 PM      Miles Bintz     SCR(s) 
 *        7291 :
 *        updates for reworked build system
 *        
 *  1    mpeg      1.0         2/25/02 8:48:20 AM     Bobby Bradford  
 * $
 * 
 *    Rev 1.1   09 Sep 2003 15:42:12   bintzmf
 * SCR(s) 7291 :
 * updates for reworked build system
 * 
 *    Rev 1.0   Feb 25 2002 08:48:20   bradforw
 * SCR(s) 3201 :
 * 
 *
 ****************************************************************************/

