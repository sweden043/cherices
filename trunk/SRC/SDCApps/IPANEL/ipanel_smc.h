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
#ifndef _IPANEL_MIDDLEWARE_PORTING_SMC_API_FUNCTOTYPE_H_
#define _IPANEL_MIDDLEWARE_PORTING_SMC_API_FUNCTOTYPE_H_

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
    IPANEL_CARD_IN,
    IPANEL_CARD_OUT,
    IPANEL_CARD_ERROR
} IPANEL_SMARTCARD_STATUS_e;

typedef enum
{
    IPANEL_SMARTCARD_STANDARD_ISO7816,
    IPANEL_SMARTCARD_STANDARD_NDS,
    IPANEL_SMARTCARD_STANDARD_EMV96,
    IPANEL_SMARTCARD_STANDARD_EMV2000,
    IPANEL_SMARTCARD_STANDARD_ECHOCHAR_T,
    IPANEL_SMARTCARD_STANDARD_UNDEFINE
} IPANEL_SMARTCARD_STANDARD_e;

typedef INT32_T (*IPANEL_SC_STATUS_NOTIFY)(
                        INT32_T                     cardno,
                        IPANEL_SMARTCARD_STATUS_e   status,
                        BYTE_T                     *art,
                        IPANEL_SMARTCARD_STANDARD_e standard
                    );

//--------------------------------------------------------------------------------------------------
//  EXTERN DATA DEFINITION
//--------------------------------------------------------------------------------------------------
//


//--------------------------------------------------------------------------------------------------
//  EXTERN FUNCTION PROTOTYPES
//--------------------------------------------------------------------------------------------------
//
INT32_T ipanel_porting_smartcard_reset(INT32_T cardno, BYTE_T *msgATR);

INT32_T ipanel_porting_smartcard_open(INT32_T cardno, IPANEL_SC_STATUS_NOTIFY func);

INT32_T ipanel_porting_smartcard_close(INT32_T cardno);

INT32_T ipanel_porting_smartcard_transfer_data(
        INT32_T         cardno,
        CONST BYTE_T   *reqdata,
        INT32_T         reqlen,
        BYTE_T         *repdata,
        INT32_T        *replen,
        UINT16_T       *statusword
    );

INT32_T  ipanel_smartcard_init(void);

void ipanel_smartcard_exit(void);

#ifdef __cplusplus
}
#endif

#endif  // _IPANEL_MIDDLEWARE_PORTING_SMC_API_FUNCTOTYPE_H_

