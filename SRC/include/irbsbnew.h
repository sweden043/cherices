#ifndef _IRBSB_H_
#define _IRBSB_H_

#include "genir.h"

enum irstate
{
   STATE_HEADER_SEARCH,
   STATE_MODE_SEARCH,
   STATE_TRAILER_SEARCH,
   STATE_CUSTOMER_SEARCH,
   STATE_BSKYBMODE_SEARCH,
   STATE_CMD_SEARCH, 
   STATE_SFT_SEARCH
};

/**********************************************************/
/* The following struct contains IR decode instance infos */
/* that are to be passed to cnxt_irrx_register() call.    */
/**********************************************************/
typedef struct IRDECODE_INSTANCE {
   enum irstate current_state;
   u_int32 gap_count;
   bool bKeyRepeat;
   u_int8 num_decoded_cmdbits;   //number of decoded command bits
   u_int32 cmd_bits;             //partially decoded command if not enough data
   u_int32 * device_table;       //pointer to OpenTV remote control table (primary or secondary)
   u_int16 device_and_cmd;       //device address and command found in "STATE_CMD_SEARCH"
   u_int8 last_device;           //last device index into the OpenTV remote control table
   u_int32 last_keycode;
   u_int8 last_state;
   bool bKeySent;
} IRDECODE_INSTANCE, * PIRDECODE_INSTANCE;

void        IR_Init(IRRX_PORTID portid);
IRRX_STATUS IR_SW_Init(IRRX_PORTID portid, void **ppinstance);
void        IR_Decode(void * pinst_decode, PIRRX_DATAINFO pdatainfo, PIRRX_KEYINFO pkeyinfo);

#endif /*_IRBSB_H_*/
