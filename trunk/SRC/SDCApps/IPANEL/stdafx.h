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
#ifndef _IPANEL_MIDDLEWARE_PORTING_STDAFX_API_FUNCTOTYPE_H_
#define _IPANEL_MIDDLEWARE_PORTING_STDAFX_API_FUNCTOTYPE_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include <nucleus.h>
#include <kal.h>
#include "stbcfg.h"
#include "basetype.h"
#include "kal.h"
#include "iic.h"
#include "nvstore.h"
#include "trace.h"
#include "genir.h"
#include "buttons.h"
#include "osdlib.h"
#include "osdlibc.h"
#include "gfxtypes.h"
#include "gfxlib.h"
#include "gxagfx.h"
#include "hwconfig.h"
#include "tvenc.h"
#include "globals.h"
#include "aud_comn.h"
#include "pcm.h"
#include "aud_api.h"
#include "retcodes.h"
#include "hsdp.h"
#include "ts_route.h"
#include "demod_user_api.h"
#include "demuxapi.h"
#include "video.h"
#include "fpleds.h"
#include "vidlibc.h"
#include "fpleds.h"
#include "iofuncs.h"
#include "smartcard.h"

#include "..\..\NUPINCL\nucleus.h"
#include "..\..\nupnet\externs.h"
#include "..\..\eth\smsc911x.h"
#include "..\..\nupnet/icmp.h"
#include "..\..\eth/eth.h"
#include "..\..\nupnet/dev.h"
#include "..\..\nupnet/target.h"
#include "..\..\nupnet/dhcp.h"
#include "..\..\nupnet/Socketd.h"
#include "..\..\nupnet/sockdefs.h"
#include "..\..\nupnet/mem_defs.h"

#ifdef __cplusplus
}
#endif

#endif //_IPANEL_MIDDLEWARE_PORTING_STDAFX_API_FUNCTOTYPE_H_

