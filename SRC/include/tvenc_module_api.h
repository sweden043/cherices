#ifndef __TVENC_MODULE_API
#define __TVENC_MODULE_API

/****************************************************************************/
/*                   CONEXANT PROPRIETARY AND CONFIDENTIAL                  */
/*                       SOFTWARE FILE/MODULE HEADER                        */
/*                Copyright Conexant Systems Inc. 1998-2003                 */
/*                         All Rights Reserved                              */
/****************************************************************************/
/*
 * Filename:        tvenc_module_api.h
 *
 * Description: This file contains the specification of the module-level
 *              interface for the TV encoder driver.
 *
 * Author: Xin Golden
 *
 ****************************************************************************/
/* $Header: tvenc_module_api.h, 2, 10/24/03 11:50:45 AM, Xin Golden$
 ****************************************************************************/
#include "basetype.h"
#include "tvenc.h"

typedef CNXT_TVENC_STATUS (SET_PICCTL_FUNC)( CNXT_TVENC_UNIT_INST *pUnitInst, CNXT_TVENC_CONTROL Control, int32 Value );
typedef CNXT_TVENC_STATUS (GET_PICCTL_FUNC)( CNXT_TVENC_UNIT_INST *pUnitInst, CNXT_TVENC_CONTROL Control, int32 *pValue );
typedef CNXT_TVENC_STATUS (SET_STANDARD_FUNC)( CNXT_TVENC_UNIT_INST *pUnitInst, CNXT_TVENC_VIDEO_STANDARD Standard );
typedef CNXT_TVENC_STATUS (GET_STANDARD_FUNC)( CNXT_TVENC_UNIT_INST *pUnitInst, CNXT_TVENC_VIDEO_STANDARD *pStandard );
typedef CNXT_TVENC_STATUS (SET_POS_FUNC)( CNXT_TVENC_UNIT_INST *pUnitInst, int8 XOffset, int8 YOffset );
typedef CNXT_TVENC_STATUS (GET_POS_FUNC)( CNXT_TVENC_UNIT_INST *pUnitInst, int8 *pXStart, int8 *pYStart );
typedef CNXT_TVENC_STATUS (SET_CONNECTION_FUNC)( CNXT_TVENC_UNIT_INST *pUnitInst, u_int8 uConnection);
typedef CNXT_TVENC_STATUS (GET_CONNECTION_FUNC)( CNXT_TVENC_UNIT_INST *pUnitInst, u_int8 *puConnection );
typedef CNXT_TVENC_STATUS (VTIMING_RESET_FUNC)( CNXT_TVENC_UNIT_INST *pUnitInst );
typedef CNXT_TVENC_STATUS (TTX_ENABLE_FUNC)( CNXT_TVENC_UNIT_INST *pUnitInst );
typedef CNXT_TVENC_STATUS (TTX_SET_LINES_FUNC)( CNXT_TVENC_UNIT_INST *pUnitInst, u_int32 uField1ActiveLines, u_int32 uField2ActiveLines );
typedef CNXT_TVENC_STATUS (TTX_DISABLE_FUNC)( CNXT_TVENC_UNIT_INST *pUnitInst );
typedef CNXT_TVENC_STATUS (CC_ENABLE_FUNC)( CNXT_TVENC_UNIT_INST *pUnitInst );
typedef CNXT_TVENC_STATUS (CC_SEND_DATA_FUNC)( CNXT_TVENC_UNIT_INST *pUnitInst, CNXT_TVENC_CC_TYPE Type, \
                            u_int8 uByteOne, u_int8 uByteTwo );
typedef CNXT_TVENC_STATUS (CC_DISABLE_FUNC)( CNXT_TVENC_UNIT_INST *pUnitInst );
typedef CNXT_TVENC_STATUS (WSS_ENABLE_FUNC)( CNXT_TVENC_UNIT_INST *pUnitInst );
typedef CNXT_TVENC_STATUS (WSS_SETCONFIG_FUNC)( CNXT_TVENC_UNIT_INST *pUnitInst, CNXT_TVENC_WSS_SETTINGS *pWSS_Settings );
typedef CNXT_TVENC_STATUS (WSS_GETCONFIG_FUNC)( CNXT_TVENC_UNIT_INST *pUnitInst, CNXT_TVENC_WSS_SETTINGS *pWSS_Settings );
typedef CNXT_TVENC_STATUS (WSS_DISABLE_FUNC)( CNXT_TVENC_UNIT_INST *pUnitInst );
typedef CNXT_TVENC_STATUS (CGMS_ENABLE_FUNC)( CNXT_TVENC_UNIT_INST *pUnitInst );
typedef CNXT_TVENC_STATUS (CGMS_SETCONFIG_FUNC)( CNXT_TVENC_UNIT_INST *pUnitInst, CNXT_TVENC_CGMS_SETTINGS *pCGMS_Settings );
typedef CNXT_TVENC_STATUS (CGMS_GETCONFIG_FUNC)( CNXT_TVENC_UNIT_INST *pUnitInst, CNXT_TVENC_CGMS_SETTINGS *pCGMS_Settings );
typedef CNXT_TVENC_STATUS (CGMS_DISABLE_FUNC)( CNXT_TVENC_UNIT_INST *pUnitInst );


typedef struct {
    SET_PICCTL_FUNC        *set_picctl;
    GET_PICCTL_FUNC        *get_picctl;
    SET_STANDARD_FUNC      *set_standard;
    GET_STANDARD_FUNC      *get_standard;
    SET_POS_FUNC           *set_posoffset;
    GET_POS_FUNC           *get_posoffset;
    SET_CONNECTION_FUNC    *set_connection;
    GET_CONNECTION_FUNC    *get_connection;
    VTIMING_RESET_FUNC     *vtiming_reset;
    TTX_ENABLE_FUNC        *ttx_enable;
    TTX_SET_LINES_FUNC     *ttx_set_lines;
    TTX_DISABLE_FUNC       *ttx_disable;
    CC_ENABLE_FUNC         *cc_enable;
    CC_SEND_DATA_FUNC      *cc_send_data;
    CC_DISABLE_FUNC        *cc_disable;
    WSS_ENABLE_FUNC        *wss_enable;
    WSS_SETCONFIG_FUNC     *wss_setconfig;
    WSS_GETCONFIG_FUNC     *wss_getconfig;
    WSS_DISABLE_FUNC       *wss_disable;
    CGMS_ENABLE_FUNC       *cgms_enable;
    CGMS_SETCONFIG_FUNC    *cgms_setconfig;
    CGMS_GETCONFIG_FUNC    *cgms_getconfig;
    CGMS_DISABLE_FUNC      *cgms_disable;

} TVENC_FTABLE;

typedef CNXT_TVENC_STATUS (INIT_FUNC)( CNXT_TVENC_UNIT_INST *pUnitInst, TVENC_FTABLE *function_table );


/****************************************************************************
 * $Log: 
 *  2    mpeg      1.1         10/24/03 11:50:45 AM   Xin Golden      CR(s): 
 *        7463 add CGMS support to tvenc driver.
 *  1    mpeg      1.0         7/30/03 4:08:34 PM     Lucy C Allevato 
 * $
 * 
 *    Rev 1.0   30 Jul 2003 15:08:34   goldenx
 * SCR(s) 5519 :
 * initial revision
 ****************************************************************************/

#endif /* __TVENC_MODULE_API */

