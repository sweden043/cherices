#ifndef _IRSEJ_H
#define _IRSEJ_H

#include "genir.h"

enum irstate
{
   STATE_HEADER_SEARCH,
   STATE_CMD_SEARCH
};

/**********************************************************/
/* The following struct contains IR decode instance infos */
/* that are to be passed to cnxt_irrx_register() call.    */
/**********************************************************/
typedef struct IRDECODE_INSTANCE {
   enum irstate current_state;
   u_int8 dbps;
   u_int8 count_bitpairs;
   u_int32 present_packet;    
   u_int32 last_packet;    
   u_int32 last_cnxtcode;
   u_int32 last_buttonmask;
   int16 last_deltax;
   int16 last_deltay;
} IRDECODE_INSTANCE, * PIRDECODE_INSTANCE;

void        IR_Init(IRRX_PORTID portid);
IRRX_STATUS IR_SW_Init(IRRX_PORTID portid, void **ppinstance);
void        IR_Decode(void * pinst_decode, PIRRX_DATAINFO pdatainfo, PIRRX_KEYINFO pkeyinfo);

#endif /*_IRSEJ_H*/
