/****************************************************************************/
/*                   CONEXANT PROPRIETARY AND CONFIDENTIAL                  */
/*                       SOFTWARE FILE/MODULE HEADER                        */
/*         Conexant Systems Inc. (c) 1998, 1999, 2000, 2001, 2002, 2003     */
/*                               Austin, TX                                 */
/*                            All Rights Reserved                           */
/****************************************************************************/
/*
 * Filename:       ts_route.c
 *
 * Description:    ts_route functions map board specific mux info to 
 *                 to their respective HSDP ports
 *
 *
 * Author:         Miles Bintz
 *
 *****************************************************************************/

/*****************************************************************************
 $Header: ts_route.c, 3, 5/20/03 2:42:48 PM, Miles Bintz$
 *****************************************************************************/
/*****************************************************************************
 *
 * PREFACE
 *
 *-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
 *
 *  Why does there need to be a ts_route_input and a ts_route_output
 * as opposed to just a general ts_route_link?
 *
 * Lets say that you want to route a stream
 * out to a 1394 device and then back from that 1394 device into
 * a demux.
 *
 * Assume that the API has just the ts_route_link().  The calls would
 * look like this:
 * ts_route_link(TS_PORT_NIM0, TS_PORT_1394);
 * ts_route_link(TS_PORT_1394, TS_DEMUX0);
 *
 * At this layer, we can infer that since 1394 is the "destination"
 * on the first call, that we want to route OUT to this port.
 * On the second call, since 1394 is the source, we can infer we 
 * want to route IN from this port.
 * The next question is, which HSDP does the 1394 device connect 
 * to on the chip?  (ed0, hs0, hs1?) Furthermore, how does the board 
 * need to set its muxes (if any) to allow for this connection?  
 *
 * This information is provided by the cnxt_tux_route_* APIs which
 * contain _board specific_ information.  It takes, as a parameter,
 * an input or output port, sets the muxes accordingly, and returns
 * which HSDP that input/output is now connected to.
 *
 * That said, cnxt_tux_route_*() needs to know which way you are going
 * with this connection.  Why?  When outputting to 1394, the data will 
 * come from (say) HS1.  When inputting to 1394, that data will go to
 * (say) HS0.  So even though 1394 looks like the input and output are
 * at the same location, they route to different locations on the chip.
 *
 * So the "epiphany" question is this: can a general ts_route_link() 
 * call provide the information necessary to determine which way to set
 * the muxes only by specifying 1394?  Lets look at the hypothetical
 * call stack:
 *    ts_route_link(TS_PORT_NIM0, TS_PORT_1394);
 *     +-> cnxt_tux_route_link(TS_PORT_1394, &conn);
 *           +-> set_mux();
 *           +-> *conn = port;
 *     +-> cnxt_hsdp_route(TS_PORT_NIM0, conn);
 *    ts_route_link(TS_PORT_1394, TS_DEMUX0);
 *     +-> cnxt_tux_route_link(TS_PORT_1394, &conn);
 *           +-> set_mux();
 *           +-> *conn = port;
 *     +-> cnxt_hsdp_route(conn, TS_DEMUX0);
 *
 * Notice: How would cnxt_hsdp_route know to go from NIM0 to conn in the
 * first call, and from conn to demux0 in the second?   Notice also the
 * tux_route_link() calls are identical.  Clearly, a simple "link" API
 * is insufficient.
 *
 * Consider the route_in and route_out API.  When routing into the chip,
 * what are the possible destinations?
 * 
 * Demuxes are the only logical  destination for an input.
 *
 * When routing out, what are possible sources?  
 * Anything.
 *
 * When routing out, what are possible destinations?
 * HS0 or HS1 (generally speaking the 'n' bidirectional data ports).
 *
 *-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
 *
 * TO THE CUSTOMER
 *
 *-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
 *
 * Since your board most likely will NOT have any muxes, the tux_route calls
 * are not necessary.  With the freedom of not needing to 
 * support an arbitrary number of IRD designs and having your devices connect
 * directly to the chip's HSDPs, you can greatly simplify this process by 
 * calling the cnxt_hsdp_route functions directly.
 * 
 *****************************************************************************/

#include "stbcfg.h"
#include "kal.h"
#include "trace.h"
#include "hsdp.h"
#include "cnxt_tux.h"

/******************************************************************************
 * FUNCTION:     cnxt_ts_route_init
 *
 * PARAMETERS:   NONE
 *
 * RETURNS:      OK, ERROR
 *
 * DESCRIPTION:  initialize the hsdp routing code
 *
 *****************************************************************************/

TSROUTE_STATUS cnxt_ts_route_init() {
   TSROUTE_STATUS rc;
   rc = cnxt_hsdp_init();
   if (rc != TSROUTE_STATUS_OK) {
       return rc;
   }
   return cnxt_tux_init();
}

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
TSROUTE_STATUS cnxt_ts_route_input(TS_PORT ts_src, HSDP_PORT dmx)  {
    HSDP_PORT conn;
    HSDP_PORT_TYPE port_type;

    if (cnxt_tux_route_input(ts_src, &conn, &port_type) == TSROUTE_STATUS_OK) {
        cnxt_hsdp_route(conn, dmx, port_type);
    } else {
        return TSROUTE_STATUS_ERROR;
    }
    return TSROUTE_STATUS_OK;
}

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

TSROUTE_STATUS cnxt_ts_route_output(TS_PORT ts_src, TS_PORT ts_dest, HSDP_PORT *out_port)  {
    HSDP_PORT conn;
    HSDP_PORT_TYPE port_type;

    /* The destination can't be a demux in the route out function */
    if (ts_dest < TS_DEMUX0) {
        if (cnxt_tux_route_output(ts_dest, &conn, &port_type) == TSROUTE_STATUS_OK) {

            if (out_port != NULL)
            {
                *out_port = conn;
            }
            /* Notice in this call to cnxt_hsdp_route, the parameters are switched */
            switch (ts_src) {
                case TS_DEMUX0:
                    cnxt_hsdp_route(HSDP_DEMUX0, conn, port_type);
                    break;
                case TS_DEMUX1:
                    cnxt_hsdp_route(HSDP_DEMUX1, conn, port_type);
                    break;
                case TS_DEMUX2:
                    cnxt_hsdp_route(HSDP_DEMUX2, conn, port_type);
                    break;
                case TS_PORT_NIM0:
                    cnxt_hsdp_route(HSDP_NIM0, conn, port_type);
                    break;
#if HSDP_TYPE  != HSDP_COLORADO
                case TS_PORT_NIM1:
                    cnxt_hsdp_route(HSDP_NIM1, conn, port_type);
                    break;
#endif
                case TS_PORT_NIM2:
                    cnxt_hsdp_route(HSDP_NIM2, conn, port_type);
                    break;
                case TS_PORT_NIM3:
                    cnxt_hsdp_route(HSDP_NIM3, conn, port_type);
                    break;
                case TS_PORT_CI: case TS_PORT_1394:
                    cnxt_hsdp_route(HSDP_HSDP0, conn, port_type);
                    break;
                default:
                    return TSROUTE_STATUS_ERROR;
            } 
        } else { 
            return TSROUTE_STATUS_ERROR;
        }
    } else {
        return TSROUTE_STATUS_INVALID;
    }
    return TSROUTE_STATUS_OK;
}





/*****************************************************************************
 * This file was split from hsdp\hsdp.c  See that file for more history.
 *
 * Change Log:
 * $Log: 
 *  3    mpeg      1.2         5/20/03 2:42:48 PM     Miles Bintz     SCR(s) 
 *        6470 6471 :
 *        out_port should be set to conn not to port_Type
 *        
 *  2    mpeg      1.1         5/14/03 9:45:20 PM     Miles Bintz     SCR(s) 
 *        6353 6354 :
 *        rework of HSDP driver
 *        
 *  1    mpeg      1.0         5/14/03 9:32:48 PM     Miles Bintz     
 * $
 * 
 *    Rev 1.2   20 May 2003 13:42:48   bintzmf
 * SCR(s) 6470 6471 :
 * out_port should be set to conn not to port_Type
 * 
 *    Rev 1.1   14 May 2003 20:45:20   bintzmf
 * SCR(s) 6353 6354 :
 * rework of HSDP driver
 * 
 *    Rev 1.0   14 May 2003 20:32:48   bintzmf
 * SCR(s) 6353 6354 :
 * rework of HSDP driver
 *****************************************************************************/

