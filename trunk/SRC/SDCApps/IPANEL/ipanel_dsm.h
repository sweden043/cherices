/*********************************************************************
    Copyright (c) 2008 - 2010 Embedded Internet Solutions, Inc
    All rights reserved. You are not allowed to copy or distribute
    the code without permission.
    This is the demo implenment of the base Porting APIs needed by iPanel MiddleWare. 
    Maybe you should modify it accorrding to Platform.
    
    Note: the "int" in the file is 32bits
    
    History:
		version		date		name		desc
         0.01     2009/8/1     Vicegod     create
*********************************************************************/
#ifndef _IPANEL_MIDDLEWARE_PORTING_DSM_API_FUNCTOTYPE_H_
#define _IPANEL_MIDDLEWARE_PORTING_DSM_API_FUNCTOTYPE_H_

#include "ipanel_typedef.h"

#ifdef __cplusplus
extern "C" {
#endif

//--------------------------------------------------------------------------------------------------
//  CONSTANTS DEFINITION
//--------------------------------------------------------------------------------------------------
//


//--------------------------------------------------------------------------------------------------
//  TYPES DEFINITION
//--------------------------------------------------------------------------------------------------
//
typedef enum
{
    IPANEL_DSM_TYPE_PES,
    IPANEL_DSM_TYPE_TS,
    IPANEL_DSM_TYPE_UNKNOWN
} IPANEL_ENCRY_MODE_e;

//--------------------------------------------------------------------------------------------------
//  EXTERN DATA DEFINITION
//--------------------------------------------------------------------------------------------------
//


//--------------------------------------------------------------------------------------------------
//  EXTERN FUNCTION PROTOTYPES
//--------------------------------------------------------------------------------------------------
//
UINT32_T ipanel_porting_descrambler_allocate(IPANEL_ENCRY_MODE_e mode);

INT32_T ipanel_porting_descrambler_free(UINT32_T handle);

INT32_T ipanel_porting_descrambler_set_pid(UINT32_T handle, UINT16_T pid);

INT32_T ipanel_porting_descrambler_set_oddkey(UINT32_T handle, BYTE_T *key, INT32_T len);

INT32_T ipanel_porting_descrambler_set_evenkey(UINT32_T handle, BYTE_T *key, INT32_T len);

#ifdef __cplusplus
}
#endif

#endif  // _IPANEL_MIDDLEWARE_PORTING_DSM_API_FUNCTOTYPE_H_

