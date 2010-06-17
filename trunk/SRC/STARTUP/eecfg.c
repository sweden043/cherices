/****************************************************************************/
/*                   CONEXANT PROPRIETARY AND CONFIDENTIAL                  */
/*                       SOFTWARE FILE/MODULE HEADER                        */
/*                     Conexant Systems Inc. (c) 2003                       */
/*                              Austin, TX                                  */
/*                         All Rights Reserved                              */
/****************************************************************************/
/*
 * Filename:        eecfg.c
 *
 *
 * Description:     Config EEPROM handling
 *
 *
 * Author:          Tim White / Craig Dry
 *
 ****************************************************************************/
/* $Header: eecfg.c, 4, 3/19/04 2:21:34 PM, Craig Dry$
 ****************************************************************************/

/******************/
/* Include Files  */
/******************/
#include "stbcfg.h"
#include "basetype.h"
#include "startup.h"

/***************/
/* Global Data */
/***************/
extern CONFIG_TABLE config_table;

/********************************************************************/
/*  PopulateConfigTable                                             */
/*                                                                  */
/*  PARAMETERS:                                                     */
/*      None.                                                       */
/*                                                                  */
/*  DESCRIPTION:                                                    */
/*      This procedure copies configuration values from the start   */
/*      of pawser memory to a C structure.                          */
/*                                                                  */
/*  RETURNS: nothing                                                */
/*                                                                  */
/********************************************************************/
void PopulateConfigTable()
{
   config_table.xtal_speed      = 0;
   config_table.length          = 0;
   config_table.checksum        = 0;
   config_table.inv_checksum    = 0;
   config_table.version_code    = 0;
   config_table.customer_id     = 0;
   config_table.board_type      = 0;
   config_table.board_rev       = 0;
   config_table.dac_mux_out     = 0;
   config_table.modem_config    = 0;
   config_table.spur_sc1_option = 0;

   config_table.xtal_speed =        ReadCfgEeprom(BOARD_CONFIG_XTAL_SPEED_SIZE,
                                                  BOARD_CONFIG_XTAL_SPEED_OFFSET);
   config_table.length =            ReadCfgEeprom(BOARD_CONFIG_LENGTH_SIZE,
                                                  BOARD_CONFIG_LENGTH_OFFSET);
   config_table.checksum =          ReadCfgEeprom(BOARD_CONFIG_CHECKSUM_SIZE,
                                                  BOARD_CONFIG_CHECKSUM_OFFSET);
   config_table.inv_checksum =      ReadCfgEeprom(BOARD_CONFIG_INV_CHECKSUM_SIZE,
                                                  BOARD_CONFIG_INV_CHECKSUM_OFFSET);
   config_table.version_code =      ReadCfgEeprom(BOARD_CONFIG_VERSION_CODE_SIZE,
                                                  BOARD_CONFIG_VERSION_CODE_OFFSET);
   config_table.customer_id =       ReadCfgEeprom(BOARD_CONFIG_CUSTOMER_ID_SIZE,
                                                  BOARD_CONFIG_CUSTOMER_ID_OFFSET);
   config_table.board_type =        ReadCfgEeprom(BOARD_CONFIG_BOARD_TYPE_SIZE,
                                                  BOARD_CONFIG_BOARD_TYPE_OFFSET);
   config_table.board_rev =         ReadCfgEeprom(BOARD_CONFIG_BOARD_REV_SIZE,
                                                  BOARD_CONFIG_BOARD_REV_OFFSET);
   config_table.dac_mux_out =       ReadCfgEeprom(BOARD_CONFIG_DAC_MUX_OUT_SIZE,
                                                  BOARD_CONFIG_DAC_MUX_OUT_OFFSET);
   config_table.modem_config =      ReadCfgEeprom(BOARD_CONFIG_MODEM_CONFIG_SIZE,
                                                  BOARD_CONFIG_MODEM_CONFIG_OFFSET);
   config_table.spur_sc1_option =   ReadCfgEeprom(BOARD_CONFIG_SPUR_SC1_OPTION_SIZE,
                                                  BOARD_CONFIG_SPUR_SC1_OPTION_OFFSET);
}

/****************************************************************************
 * Modifications:
 * $Log: 
 *  4    mpeg      1.3         3/19/04 2:21:34 PM     Craig Dry       CR(s) 
 *        8599 : Fix Compiler/Assembler warnings for Codeldr.
 *  3    mpeg      1.2         11/1/03 3:01:02 PM     Tim Ross        CR(s): 
 *        7719 7762 Replaced FCopy with ReadCfgEeprom since config eeprom 
 *        contents is no longer
 *        stored in pawser RAM.
 *  2    mpeg      1.1         9/26/03 6:22:32 PM     Angela Swartz   SCR(s) 
 *        7563 :
 *        replace the use of memcpy with FCopy to allow codeldr to build under 
 *        VxWorks with the new build system
 *        
 *  1    mpeg      1.0         7/9/03 3:30:08 PM      Tim White       
 * $
 * 
 *    Rev 1.1   26 Sep 2003 17:22:32   swartzwg
 * SCR(s) 7563 :
 * replace the use of memcpy with FCopy to allow codeldr to build under VxWorks with the new build system
 * 
 *    Rev 1.0   09 Jul 2003 14:30:08   whiteth
 * SCR(s) 6901 :
 * Conexant reference IRD EEPROM configuration support.
 * 
 *
 ****************************************************************************/

