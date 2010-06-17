/****************************************************************************/
/*                   CONEXANT PROPRIETARY AND CONFIDENTIAL                  */
/*                       SOFTWARE FILE/MODULE HEADER                        */
/*         Conexant Systems Inc. (c) 1998, 1999, 2000, 2001, 2002, 2003     */
/*                               Austin, TX                                 */
/*                            All Rights Reserved                           */
/****************************************************************************/
/*
 * Filename:       ts_route.h
 *
 * Description:    Function declarations for the TS Route API
 *
 *
 *
 * Author:         Miles Bintz
 *
 *****************************************************************************/

/*****************************************************************************
 $Header: ts_route.h, 1, 5/14/03 9:31:28 PM, Miles Bintz$
 *****************************************************************************/
#ifndef _ts_route_h_
#define _ts_route_h_

#include "hsdp.h"

/*****************************************************************************
 *  TS Ports are ports that exist on the back of the box.
 *
 *  TS_PORT_NIM[0-3], as used in this context, refers to an
 *  actual "Network Interface Module".  Since a NIM could be
 *  satellite, cable, or terrestrial, simply specifying NIM
 *  is the most general way to handle this case.
 *
 *  Demuxes, while not actually a port on the back of the box
 *  is a source for transport streams and could connect to a
 *  port on the back of the box
 ****************************************************************************/
typedef enum {TS_PORT_NIM0,
              TS_PORT_NIM1,
              TS_PORT_NIM2,
              TS_PORT_NIM3,
              TS_PORT_DVB,
              TS_PORT_OTV,
              TS_PORT_CI,
              TS_PORT_1394,
              TS_PORT_NIM4,
              TS_PORT_POD,
              TS_DEMUX0,
              TS_DEMUX1,
              TS_DEMUX2
             } TS_PORT;


TSROUTE_STATUS cnxt_ts_route_init();

/******************************************************************************
 * FUNCTION:     cnxt_ts_route_input
 *
 * PARAMETERS:   ts_src, dmx
 *    ts_src - which transport stream source to take data from
 *    dmx    - which demux to send data to
 *
 * RETURNS:      OK, ERROR
 *
 * DESCRIPTION:  take a src, call the board muxing function (tux route) to
 * set the mux appropriately and discover which HSDP on the chip this port is
 * connected to (either directly or through aforementioned mux).  Then route 
 * this src to the destination assuming muxes were set correctly in the 
 * tux_route call.  Also determine whether a port is operating in serial
 * or parallel mode and set accordingly.
 *
 * NOTE:  The only sensible destination for an input call is a demux
 * 
 * NOTE:         tux_route needs to know board specific data like how muxes are
 * wired up.  cnxt_hsdp_route needs to know about all the chips and which 
 * connections are possible
 *****************************************************************************/
TSROUTE_STATUS cnxt_ts_route_input(TS_PORT ts_src, HSDP_PORT dmx);


/******************************************************************************
 * FUNCTION:     cnxt_ts_route_output
 *
 * PARAMETERS:   ts_src, dmx
 *    ts_src   - which transport stream source to take data from
 *    ts_dest  - which transport stream destination to send data to
 *    out_port - pointer to value which holds which HSDP port is driving 
 *               the ts_dest port or null if "don't care"
 *
 * RETURNS:      OK, ERROR
 *
 * DESCRIPTION:  given a destination, call the board muxing function 
 * (tux route) to set the mux appropriately and discover which HSDP on
 * the chip the output
 * port is connected to (either directly or through aforementioned mux).  
 * Then route this src to the destination (through the HSDP ports)
 * assuming muxes were set correctly
 * in the tux_route call.  Finally return back which HSDP port is driving 
 * the destination.
 *
 * NOTE:  The only ports that can drive the ts_dest is HSDP0/1.  Return which
 * one it is so the clock and error control functions can be called.
 * 
 *****************************************************************************/
TSROUTE_STATUS cnxt_ts_route_output(TS_PORT ts_src, TS_PORT ts_des, HSDP_PORT *out_port);

#endif
/*****************************************************************************
 * Change Log:
 * $Log: 
 *  1    mpeg      1.0         5/14/03 9:31:28 PM     Miles Bintz     
 * $
 * 
 *    Rev 1.0   14 May 2003 20:31:28   bintzmf
 * SCR(s) 6353 6354 :
 * rework of HSDP driver
 *****************************************************************************/

