/****************************************************************************/
/*                   CONEXANT PROPRIETARY AND CONFIDENTIAL                  */
/*                       SOFTWARE FILE/MODULE HEADER                        */
/*           Conexant Systems Inc. (c) 1998, 1999, 2000, 2001, 2002         */
/*                               Austin, TX                                 */
/*                            All Rights Reserved                           */
/****************************************************************************/
/*
 * Filename:       hsdp.h
 *
 *
 * Description:    High Speed Data Port definitions and routing driver
 *
 *
 * Author:         Miles Bintz
 *
 ****************************************************************************/
/* $Header: hsdp.h, 4, 5/14/03 9:48:14 PM, Miles Bintz$
 ****************************************************************************/

#ifndef _HSDP_H_
#define _HSDP_H_

/****************************************************************************
 * The type of port is for use in the internals of the HSDP
 * driver.  For example, on the cx22490 NIM0 is serial, but
 * on the CX24430, NIM0 is serial, NIM1 is none, and NIM2-4 are
 * internal and considered parallel.
 ****************************************************************************/
typedef enum {HSDP_TYPE_NONE,
              HSDP_TYPE_SER,
              HSDP_TYPE_PAR,
              HSDP_TYPE_BIDIR,
              HSDP_TYPE_BIDIR_SER,
              HSDP_TYPE_1394
             } HSDP_PORT_TYPE;

/*****************************************************************************
 * HSDP ports are ports that exist on the chip.
 *
 * ie. there are a group of pins on the chip that are used for
 * inputting transport streams to the demuxes or outputting data from a demod
 * or demux.
 * 
 * !! IMPORTANT: !!
 * The reason the HSDP_PORTs are enumerated in this order
 * is because this is the order they are in in the demux input select register
 * and the HSDP0/1 control register.
 * !! IMPORTANT: !!
 * 
 * With colorado, it was just NIM0 a reserved space for
 * NIM1 and HSDP0 and 1.
 *
 * Hondo had NIM0 and NIM1, and HSDP0 and 1.
 *
 * Wabash has support for one external demod (NIM1).  A NIM is 
 * reserved, HSDP0 and 1 are in their normal places, and the
 * three internal demods come after HSDP0 and 1.  WaBASH doesn't 
 * really have a NIM 4 but it does have two NIM1s ("NIM1 parallel"
 * is the IB data demod, "NIM1 serial" is ED0).
 * 
 *****************************************************************************/
typedef enum {HSDP_NIM0=0,
#if HSDP_TYPE != HSDP_COLORADO
              HSDP_NIM1,
#endif
              HSDP_HSDP0,
              HSDP_HSDP1,
              HSDP_NIM2,
              HSDP_NIM3,
#if HSDP_TYPE != HSDP_COLORADO
              HSDP_NIM4,
#endif
              HSDP_DEMUX0,
              HSDP_DEMUX1,
              HSDP_DEMUX2,
              HSDP_NC = -1} HSDP_PORT;

#if HSDP_TYPE != HSDP_COLORADO
  #define MAX_HSDP_PORTS 10
#else
  #define MAX_HSDP_PORTS 8
#endif
// #define MAX_PARSERS    3
// #define MAX_TS_PORTS   8


/* The TSROUTE return codes can be used for all three pieces
   of this driver: ts_route, hsdp_route and tux_route since the 
   semantics of each return code is relevant to all three drivers */
typedef enum {TSROUTE_STATUS_OK, TSROUTE_STATUS_ERROR, TSROUTE_STATUS_INVALID, TSROUTE_STATUS_BUSY} TSROUTE_STATUS;
typedef enum {HSDP_CLOCK_GENERATE, HSDP_CLOCK_SERIAL, HSDP_CLOCK_INPUT} HSDP_CLOCK_TYPE;
typedef enum {HSDP_POLARITY_NEGATIVE, HSDP_POLARITY_POSITIVE} HSDP_POLARITY;
typedef enum {HSDP_PACKET_DVB, HSDP_PACKET_DSS} HSDP_PACKET_MODE;
/************************************************************************
 * The following structure contains private data which is specific      *
 * to each chip.                                                        *
 * It defines the base address and type of                              *
 * port so that, regardless of which chip this driver operates on,      *
 * the code stays relatively generic.  Lastly, it contains values to    *
 * be OR'ed into the pin muxing registers to enable a particular port's *
 * pins.                                                                *
 ************************************************************************/

typedef struct _hsdp_port {
    const LPREG             ctl_address;
} hsdp_port;

/************************************************************************
 * initializes the hsdp  driver and                                     *
 *                                                                      *
 * returns: TSROUTE_STATUS_OK, TSROUTE_STATUS_ERROR                     *
 ************************************************************************/
TSROUTE_STATUS cnxt_hsdp_init();

/************************************************************************
 * control the handling of HSDP packets                                 *
 *                                                                      *
 * params:  mode = DSS, DVB                                             *
 *          pkt_size = number of bytes between syncs.  Usually 188 for  *
 *          DVB, 130 for DSS.                                           *
 *          pkt_sync = for DVB = 0x47                                   *
 *          entire_packet - signal error for sync byte or entire packet *
 *                                                                      *
 * returns: TSROUTE_STATUS_OK, TSROUTE_STATUS_ERROR                     *
 ************************************************************************/
TSROUTE_STATUS cnxt_hsdp_pktctrl(HSDP_PACKET_MODE mode, unsigned char pkt_size, unsigned char pkt_sync, int entire_packet);

/************************************************************************
 * creates or deletes a path between a TS source and a TS destination   *
 *                                                                      *
 * params:  HSDP_PORT specifying source and destination for transport   *
 *          stream                                                      *
 *                                                                      *
 * returns: TSROUTE_STATUS_OK, TSROUTE_STATUS_ERROR,                    *
 *          TSROUTE_STATUS_INVALID                                      *
 ************************************************************************/
TSROUTE_STATUS cnxt_hsdp_route(HSDP_PORT src, HSDP_PORT dest, HSDP_PORT_TYPE port_type);
/* TSROUTE_STATUS cnxt_hsdp_unroute(int from, int to); */

/************************************************************************
 * enables and configures error signaling for specified HSDP            *
 *                                                                      *
 * params:  port - which port to configure                              *
 *          ignore - use or ignore fail signal                          *
 *          entire_packet - signal error for sync byte or entire packet *
 *          polarity - set signal active high or active low for error   *
 *                                                                      *
 * returns: TSROUTE_STATUS_OK, TSROUTE_STATUS_ERROR                     *
 ************************************************************************/
TSROUTE_STATUS cnxt_hsdp_set_error_params(HSDP_PORT port, int ignore, HSDP_POLARITY polarity);

/************************************************************************
 * Selects clock source, frequency and polarity                         *
 *                                                                      *
 * params:  port - which port to configure                              *
 *          mode - generated, serial, clock is input                    *
 *          polarity - raising/falling edge, pos = rising, neg = fall   *
 *          freq - 500,000 Hz to 54,000,000 Hz clock                    *
 *          duty - duty cycle (0 to 100 percent)                        *
 *                                                                      *
 * returns: TSROUTE_STATUS_OK, TSROUTE_STATUS_ERROR                     *
 ************************************************************************/
TSROUTE_STATUS cnxt_hsdp_set_clock_params
                   (HSDP_PORT port, HSDP_CLOCK_TYPE clk_type, HSDP_POLARITY clk_pol, int freq, int duty);

/************************************************************************
 * returns, by reference, the status of the HSDP                        *
 *                                                                      *
 * params:  *error - pointer to error variable                          *
 *                 bit 0 - lost sync on BIDIR port 0                    *
 *                 bit 1 - port 0 overrun                               *
 *                                                                      *
 *                 bit 8 - lost sync on BIDIR port 1                    *
 *                 bit 9 - port 1 overrun                               *
 *                                                                      *
 * returns: TSROUTE_STATUS_OK, TSROUTE_STATUS_ERROR                     *
 ************************************************************************/
TSROUTE_STATUS cnxt_hsdp_get_error(int *error);

#include "ts_route.h"

#endif /* _HSDP_H_ */

/****************************************************************************
 * Modifications:
 * $Log: 
 *  4    mpeg      1.3         5/14/03 9:48:14 PM     Miles Bintz     SCR(s) 
 *        6353 6354 :
 *        rework of HSDP driver
 *        
 *  3    mpeg      1.2         1/23/03 3:01:36 PM     Dave Wilson     SCR(s) 
 *        5292 :
 *        Added file header, footer and modification log.
 *        
 *  2    mpeg      1.1         1/30/02 2:07:42 PM     Miles Bintz     SCR(s) 
 *        3058 :
 *        development/interim checkin
 *        
 *        
 *  1    mpeg      1.0         1/8/02 5:42:28 PM      Miles Bintz     
 * $
 * 
 *    Rev 1.3   14 May 2003 20:48:14   bintzmf
 * SCR(s) 6353 6354 :
 * rework of HSDP driver
 * 
 *    Rev 1.2   23 Jan 2003 15:01:36   dawilson
 * SCR(s) 5292 :
 * Added file header, footer and modification log.
 *
 ****************************************************************************/

