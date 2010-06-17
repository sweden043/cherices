 /****************************************************************************/ 
 /*                   CONEXANT PROPRIETARY AND CONFIDENTIAL                  */
 /*                       SOFTWARE FILE/MODULE HEADER                        */
 /*                     Conexant Systems Inc. (c) 2003                       */
 /*                              Austin, TX                                  */
 /*                         All Rights Reserved                              */
 /****************************************************************************/
 /*
  * Filename:    irdtv.h
  *
  *
  * Description: Direct TV infrared remote control decoder header file
  *
  *
  * Author: Tim Ross
  *
  ****************************************************************************/
 /* $Id: irdtv.h,v 1.0, 2003-12-04 21:50:44Z, Tim Ross$
  ****************************************************************************/ 

#ifndef _IRDTV_H
#define _IRDTV_H

#include "genir.h"

enum irstate
{
   STATE_HEADER_SEARCH,
   STATE_REPEAT_SEARCH,
   STATE_PACKET_DATA_SEARCH
};

/**********************************************************/
/* The following struct contains IR decode instance infos */
/* that are to be passed to cnxt_irrx_register() call.    */
/**********************************************************/
typedef struct IRDECODE_INSTANCE {
   enum irstate current_state;
   u_int32 present_packet;    
   u_int32 bits_found;
   u_int32 last_packet;    
   u_int32 last_cnxtcode;
} IRDECODE_INSTANCE, * PIRDECODE_INSTANCE;

void        IR_Init(IRRX_PORTID portid);
IRRX_STATUS IR_SW_Init(IRRX_PORTID portid, void **ppinstance);
void        IR_Decode(void * pinst_decode, PIRRX_DATAINFO pdatainfo, PIRRX_KEYINFO pkeyinfo);

#endif /*_IRDTV_H*/

 /****************************************************************************
 * Modifications:
 * $Log:
 ****************************************************************************/ 
