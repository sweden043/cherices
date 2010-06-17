/****************************************************************************/
/*                   CONEXANT PROPRIETARY AND CONFIDENTIAL                  */
/*                       SOFTWARE FILE/MODULE HEADER                        */
/*           Conexant Systems Inc. (c) 1998, 1999, 2000, 2001, 2002         */
/*                               Austin, TX                                 */
/*                            All Rights Reserved                           */
/****************************************************************************/
/*
 * Filename:       cnxt_tux.h
 *
 *
 * Description:    API definition for changing board-specific
 *                 transport-stream routing devices
 *
 *
 * Author:         Miles Bintz
 *
 ****************************************************************************/
/* $Header: cnxt_tux.h, 2, 5/14/03 9:45:42 PM, Miles Bintz$
 ****************************************************************************/

#ifndef _CNXTTUX_H_
#define _CNXTTUX_H_

/************************************************************************
 * The following structure contains private data which is specific      *
 * to each chip.                                                        *
 * It defines the base address and type of                              *
 * port so that, regardless of which chip this driver operates on,      *
 * the code stays relatively generic.  Lastly, it contains values to    *
 * be OR'ed into the pin muxing registers to enable a particular port's *
 * pins.                                                                *
 ************************************************************************/

typedef struct _tux_initdata {
 /************************************************************************
  * These values are for configuring a chip's PIOs to operate
    as a NIM port or HSDP port.  These values should be set in a 
    module which has board specific information and knows which
    pins are actually in use.
 
    NOTE:  The need for these definitions will go away when this
    data is incorporated into a config file.
 
 
  ************************************************************************/
    const u_int32           pio_altfunc_mask;
    const u_int32           pio_altfunc_val;

    /* NOTE2: These values get OR'ed (RMW) in to their respective regs. */
    const u_int32           pio_pinmux0;
    const u_int32           pio_pinmux1;
    const u_int32           pio_pinmux2;
} tux_initdata;


/*************************************************************************
 * This structure is what actually maps a TS_PORT (one of the physical
 * connectors on the back of the box) to an HSDP_PORT (one of the 
 * actual groupings of pins on the chip
 *
 * An array of these should be defined for each type of board, indexed by 
 * the TS_PORT enumeration
 *
 *************************************************************************/
typedef struct _tux_data {
    /* legal defines whether a TS_PORT of this type would exist on the
     * box.  For example, a POD port would never exist on a Klondike.
     */
    const int               legal;

    /* When a TS_PORT is a source of TS streams, it connects to the 
     * "input_port".  If input_port can be serial or parallel, this 
     * is defined by inport_type and most likely will not change
     * for a given board.
     * ts_mux_input_mask and value are values which control PIO
     * expanders which connect to the control lines of the muxes (if any)
     */
    const HSDP_PORT         input_port;
    const HSDP_PORT_TYPE    inport_type;
 /* -1 if the port can only be used as an output */
    const int               ts_mux_input_mask;
    const int               ts_mux_input_values; 

    const HSDP_PORT         output_port;
    const HSDP_PORT_TYPE    outport_type;
 /* -1 if the port can only be used as an input */
    const int               ts_mux_output_mask;
    const int               ts_mux_output_values;
} tux_data;


/************************************************************************
 * The tux functions are board specific functions which map ports on    *
 * the IRD to physical pins on the chip.  These routines expect as      *
 * their parameters a TS_PORT and a pointer to an HSDP_PORT.  Upon      *
 * a successful routing, the tux routine should return                  *
 * TSROUTE_STATUS_OK and set *connection to the HSDP_PORT which the     *
 * TS_PORT is connected to.                                             *
 *                                                                      *
 * returns: TSROUTE_STATUS_OK, TSROUTE_STATUS_ERROR                     *
 *          TSROUTE_STATUS_INVALID                                      *
 ************************************************************************/
TSROUTE_STATUS cnxt_tux_route_input(TS_PORT port, HSDP_PORT *connection, HSDP_PORT_TYPE *port_type);
TSROUTE_STATUS cnxt_tux_route_output(TS_PORT port, HSDP_PORT *connection, HSDP_PORT_TYPE *port_type);
TSROUTE_STATUS cnxt_tux_init();

#endif /* _CNXTTUX_H_ */

/****************************************************************************
 * Modifications:
 * $Log: 
 *  2    mpeg      1.1         5/14/03 9:45:42 PM     Miles Bintz     SCR(s) 
 *        6353 6354 :
 *        rework of HSDP driver
 *        
 *  1    mpeg      1.0         5/14/03 9:32:48 PM     Miles Bintz     
 * $
 * 
 *    Rev 1.1   14 May 2003 20:45:42   bintzmf
 * SCR(s) 6353 6354 :
 * rework of HSDP driver
 * 
 *    Rev 1.0   14 May 2003 20:32:48   bintzmf
 * SCR(s) 6353 6354 :
 * rework of HSDP driver
 * 
 *    Rev 1.2   23 Jan 2003 15:01:36   dawilson
 * SCR(s) 5292 :
 * Added file header, footer and modification log.
 *
 ****************************************************************************/

