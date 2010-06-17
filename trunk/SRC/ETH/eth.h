#ifndef	_ETH_H_
#define _ETH_H_

#include "nucleus.h"


typedef enum
{
   ETH_SUCCESS=NU_SUCCESS,
   ETH_ERROR,
   ETH_NO_HARDWARE,
   ETH_NOFIFO,
   ETH_NOMEM,
   ETH_TIMEOUT,
   ETH_BAD_PARAMETER,
   ETH_INITIALIZED,
   ETH_LINKDOWN,
   ETH_LINKUP
} ETH_STATUS;


typedef struct _ETH_STATE
{
	u_int32	rx_packets;				/* packet received totally*/
	u_int32	tx_packets;				/* packet sent totally */
	u_int32	rx_bytes;				/*  */	
	u_int32	tx_bytes;					/*  */
	u_int32	rx_errors;				/* the total count of rx error */
	u_int32	tx_errors;				/* the total count of tx error */
	
	u_int32	rx_dropped;				/* rx dropped by hardware  */	
	u_int32	rx_discarded;			/* rx discared by software  */	
	u_int32 	tx_discarded;			/* tx discarded by stack */	
	u_int32	rx_multicast;			/* rx multicast packet received*/
	u_int32	collisions;				/* tx and rx collisions*/	
	u_int32	rx_length_errors;		/* */	
	u_int32	rx_crc_errors;			/* */
	u_int32	tx_aborted_errors;	/* */	
	u_int32	tx_carrier_errors;		/* */	

	u_int32 	tx_serious_error;		/*some fatal error such as TX does't work any more*/
	u_int32 	rx_serious_error;		/*some Rx fatal error such as no enough buffer */
	
}ETH_STATS;

STATUS eth_init (STATUS (*dev_init)(DV_DEVICE_ENTRY *dev),u_int32 ipaddr,u_int32 subnet_mask,u_int32 gateway);
STATUS eth_close (STATUS (*dev_init)(DV_DEVICE_ENTRY *dev));

#endif