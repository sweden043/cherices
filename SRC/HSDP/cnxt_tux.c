/****************************************************************************/
/*                 Conexant Systems, Inc.                                   */
/****************************************************************************/
/*                                                                          */
/* Filename:           CNXT_TUX.C                                           */
/*                                                                          */
/* Description:        Conexant IRD specific transport routing functions    */
/*                                                                          */
/* Author:             Miles Bintz                                          */
/*                                                                          */
/* Copyright Conexant Systems, Inc. 2002, 2003                              */
/* All Rights Reserved.                                                     */
/*                                                                          */
/****************************************************************************/
/*
 * $Header: cnxt_tux.c, 25, 4/21/04 11:53:32 AM, Steve Glennon$
 *
 ****************************************************************************/

/****************************************************************************
 *
 * PREFACE
 *
 *-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
 *
 * This file contains three functions which need to be implemented
 * in order for the ts_route and hsdp_route components to work.
 * However, HOW these routines are implemented is very dependent on the
 * design of an IRD.
 *
 * The API is:
 *
 *  TSROUTE_STATUS cnxt_tux_init();
 *  TSROUTE_STATUS cnxt_tux_route_input(TS_PORT port, HSDP_PORT *connection);
 *  TSROUTE_STATUS cnxt_tux_route_output(TS_PORT port, HSDP_PORT *connection);
 *
 * The input and output functions operate the same way, only they set the
 * direction of the muxes (if present) to operate in the other direction.
 *
 * These functions must take as the TS_PORT parameter a physical connector
 * on the box (TS_PORT).  The functions returns, in the *connection parameter,
 * which HSDP or TS port on the chip the physical input/output is connected to.
 * NOTE: This connection can change depending on which way a connection is 
 * being established since each port on the chip can only operate in 
 * one direction at a time. 
 * 
 * Since this array is defined so that the enumerations index this array,
 * there must be an element in the array for every known 
 * connector type.  If a particular connector is unsupported on a box, it 
 * should return TSROUTE_STATUS_INVALID as the return code.
 * 
 ****************************************************************************/

#include "hwconfig.h"
#include "kal.h"
#include "trace.h"
#include "hsdp.h"
#include "iic.h"
#include "cnxt_tux.h"

/************************************************************************
 *                                                                      * 
 *  EXTERNS                                                             *
 *                                                                      *
 ************************************************************************/
extern hsdp_port      hsdp_port_data[];
#if (TS_MUX_TYPE == TS_MUX_TYPE_ATHENS) || (TS_MUX_TYPE == TS_MUX_TYPE_MILANO)
extern HW_DWORD CNXT_PIO_EXP_REG_SHADOW;
#endif

/*
 * !!! HACK ALERT !!!
 * WARNING!!!!! Hack requested for supporting Bronco1 & Bronco3 at runtime.  When
 * Bronco1 boards disappear, so should this hack.
 * !!! HACK ALERT !!!
 */
#if ((HSDP_TYPE == HSDP_BRAZOS) && (I2C_CONFIG_EEPROM_ADDR != NOT_PRESENT))
extern int ConfigurationValid;
extern CONFIG_TABLE  config_table;
#endif
/*
 * !!! HACK ALERT !!!
 * !!! HACK ALERT !!!
 */





/*****************************************************************************/
/*****************************************************************************/
/*   TS_MUX_TYPE == UNDEFINED                                                */
/*****************************************************************************/
/*****************************************************************************/
#ifndef TS_MUX_TYPE
  #error TS_MUX_TYPE should be defined in the .CFG or in HWCONFIG.H
#endif













/*****************************************************************************/
/*****************************************************************************/
/*   TS_MUX_TYPE == KLONDIKE                                                 */
/*****************************************************************************/
/*****************************************************************************/
#if TS_MUX_TYPE == TS_MUX_TYPE_KLONDIKE
  tux_initdata initdata = { 
	  /* alt_func mask */
	  0x0,
	  
	  /* alt_func value */
	  0x0,
	  
	  /* mux0 */
           PLL_PIN_GPIO_MUX0_HS1CLK        | 
               PLL_PIN_GPIO_MUX0_HS1D0     | 
               PLL_PIN_GPIO_MUX0_HS1D1,
	   
	  /* mux1 */
          PLL_PIN_GPIO_MUX1_HS0ERRAV      |
             PLL_PIN_GPIO_MUX1_HS0D0      | 
             PLL_PIN_GPIO_MUX1_HS0D1      | 
             PLL_PIN_GPIO_MUX1_HS0D2      | 
             PLL_PIN_GPIO_MUX1_HS0D3      |
             PLL_PIN_GPIO_MUX1_HS0D4      |
             PLL_PIN_GPIO_MUX1_HS0D5      |
             PLL_PIN_GPIO_MUX1_HS0D6      |
             PLL_PIN_GPIO_MUX1_HS0D7      |
             PLL_PIN_GPIO_MUX1_HS0CLK     |
             PLL_PIN_GPIO_MUX1_HS0VALEN   |
             PLL_PIN_GPIO_MUX1_HS0SYNC    |
             PLL_PIN_GPIO_MUX1_HS0RW      |
             PLL_PIN_GPIO_MUX1_HS1D2      |
             PLL_PIN_GPIO_MUX1_HS1D3      |
             PLL_PIN_GPIO_MUX1_HS1D4      |
             PLL_PIN_GPIO_MUX1_HS1D5      |
             PLL_PIN_GPIO_MUX1_HS1D6      |
             PLL_PIN_GPIO_MUX1_HS1D7      |
             PLL_PIN_GPIO_MUX1_HS1SYNC    |
             PLL_PIN_GPIO_MUX1_HS1ERRAV   |
             PLL_PIN_GPIO_MUX1_HS1RW      |
	     PLL_PIN_GPIO_MUX1_NIM_FAIL,
          
	  /* mux2 */
          0x0
  };


    /* On klondike, all of these bits are on IIC GPIO extenders
       Bit 5 = HSDP0_CLK_EN on IIC addr 0x72 PIO #1 - HS0 Clk not used for 1394
       Bit 4 = DVB_RE on IIC addr 0x70 PIO #4
       Bit 3 = HS1_MUX_SEL on IIC addr 0x70 GPIO #3
       Bit 2 = HS0_MUX_SEL on IIC addr 0x70 GPIO #2
       Bit 1 = TS1_MUX_SEL on IIC addr 0x70 GPIO #1
       Bit 0 = TS0_MUX_SET on IIC addr 0x70 GPIO #0 */
  tux_data ts_mux_data[] =  {
    /* NIM0 is a serial  NIM connected directly to the
       chip, thus, it doesn't matter what the GPIOs are */
       { TRUE,    HSDP_NIM0,    HSDP_TYPE_SER,  0x0,   0x0,    HSDP_NC,     HSDP_TYPE_NONE,    0x0,   -1},
    /* NIM1 is actually still "NIM0" on klondike but its
       connected to HSDP0 on the chip. */
       { TRUE,   HSDP_HSDP0,    HSDP_TYPE_PAR, 0x27,  0x01,    HSDP_NC,     HSDP_TYPE_NONE,    0x0,   -1},
    /* NIM 2 and NIM 3 don't exist on klondike */
       {FALSE,      HSDP_NC,   HSDP_TYPE_NONE,    0,    -1,    HSDP_NC,     HSDP_TYPE_NONE,      0,   -1},
       {FALSE,      HSDP_NC,   HSDP_TYPE_NONE,    0,    -1,    HSDP_NC,     HSDP_TYPE_NONE,      0,   -1},
    /* DVB baseband */
       { TRUE,   HSDP_HSDP0,    HSDP_TYPE_PAR, 0x37,  0x13, HSDP_HSDP1,      HSDP_TYPE_PAR,   0x10,  0x0},
    /* OTV port */
       { TRUE,   HSDP_HSDP0,    HSDP_TYPE_PAR, 0x27,  0x02,    HSDP_NC,      HSDP_TYPE_PAR,    0x0,   -1},
    /* Common Interfce */
       { TRUE,   HSDP_HSDP0,    HSDP_TYPE_PAR, 0x24,  0x04, HSDP_HSDP1,      HSDP_TYPE_PAR,   0x08, 0x08},
    /* 1394 uses the same bus as CI but disables HS0 CLK */
       { TRUE,   HSDP_HSDP0,   HSDP_TYPE_1394, 0x24,  0x24, HSDP_HSDP1,     HSDP_TYPE_1394,   0x08, 0x08},
    /* NIM4 doesn't exist on klondike */
       {FALSE,      HSDP_NC,   HSDP_TYPE_NONE,    0,    -1,    HSDP_NC,     HSDP_TYPE_NONE,      0,   -1},
    /* POD doesn't exist on klondike */
       {FALSE,      HSDP_NC,   HSDP_TYPE_NONE,    0,    -1,    HSDP_NC,     HSDP_TYPE_NONE,      0,   -1}
  };


TSROUTE_STATUS cnxt_tux_route_input(TS_PORT port, HSDP_PORT *connection, HSDP_PORT_TYPE *port_type)
{
static TS_PORT nimtype = (TS_PORT)-1;
#if TS_MUX_TYPE == TS_MUX_TYPE_KLONDIKE
u_int8       byBoardCode, byVendorCode;
int   demod_serial;
#endif

    if ( (ts_mux_data[port].legal) && (ts_mux_data[port].input_port != HSDP_NC) ) 
    {
#if TS_MUX_TYPE == TS_MUX_TYPE_KLONDIKE
        /* On colorado, the actual type of connection between the NIM 
	 * and the Colorado is defined by resistors on the config 
	 * lines on the motherboard
	 */
        if ((port == TS_PORT_NIM0) || (port == TS_PORT_NIM1))
        {
            if (nimtype == -1)
            {
               read_board_and_vendor_codes(&byBoardCode, &byVendorCode);
               demod_serial = (byBoardCode & KLONDIKE_PARALLEL_NIM) ? FALSE : TRUE;
               if (!demod_serial)
               {
                  trace_new( TRACE_DMD | TRACE_LEVEL_2, "Configure for parallel attached NIM\n");
                  nimtype = TS_PORT_NIM1;
               }
               else
               {
                  trace_new( TRACE_DMD | TRACE_LEVEL_2, "Configure for serial attached NIM\n");
                  nimtype = TS_PORT_NIM0;
               }

            }
            port = nimtype;
        }

        write_gpio_extender(ts_mux_data[port].ts_mux_input_mask & 0x1F,
                             ts_mux_data[port].ts_mux_input_values);
        
        if (ts_mux_data[port].ts_mux_input_mask & 0x20) {
            write_second_gpio_extender(0x02, ts_mux_data[port].ts_mux_input_values >> 4);
        }

        *connection = ts_mux_data[port].input_port;
        *port_type  = ts_mux_data[port].inport_type;

#endif
        return TSROUTE_STATUS_OK;
    }
    else
    {
        *connection = HSDP_NC;
        return TSROUTE_STATUS_INVALID;
    }
}

TSROUTE_STATUS cnxt_tux_route_output(TS_PORT port, HSDP_PORT *connection, HSDP_PORT_TYPE *port_type) {
    /* This should be the same exact routine as the route_input
       routine except all "inputs" change to "outputs" */

    if ( (ts_mux_data[port].legal) && (ts_mux_data[port].output_port != HSDP_NC) ) {

        write_gpio_extender(ts_mux_data[port].ts_mux_output_mask & 0x1F,
                             ts_mux_data[port].ts_mux_output_values);
        if (ts_mux_data[port].ts_mux_output_mask & 0x20) {
            write_second_gpio_extender(0x02, ts_mux_data[port].ts_mux_output_values >> 4);
        }

        *connection = ts_mux_data[port].output_port;
        *port_type  = ts_mux_data[port].outport_type;

        return TSROUTE_STATUS_OK;
    } else {
        *connection = HSDP_NC;
        return TSROUTE_STATUS_INVALID;
    }

}
#endif








/*****************************************************************************/
/*****************************************************************************/
/*   TS_MUX_TYPE == ABILENE                                                  */
/*****************************************************************************/
/*****************************************************************************/

#if TS_MUX_TYPE == TS_MUX_TYPE_ABILENE
  tux_initdata initdata = { 
	  /* alt_func mask */
	  0x0,
	  /* alt_func value */
	  0x0,
	  /* mux0 */
           PLL_PIN_GPIO_MUX0_HS1CLK       |
               PLL_PIN_GPIO_MUX0_HS1D0    |
               PLL_PIN_GPIO_MUX0_HS1D1,
 	  
	  /* mux1 */
          PLL_PIN_GPIO_MUX1_HS0ERRAV      |
             PLL_PIN_GPIO_MUX1_HS0D0      | 
             PLL_PIN_GPIO_MUX1_HS0D1      | 
             PLL_PIN_GPIO_MUX1_HS0D2      | 
             PLL_PIN_GPIO_MUX1_HS0D3      |
             PLL_PIN_GPIO_MUX1_HS0D4      |
             PLL_PIN_GPIO_MUX1_HS0D5      |
             PLL_PIN_GPIO_MUX1_HS0D6      |
             PLL_PIN_GPIO_MUX1_HS0D7      |
             PLL_PIN_GPIO_MUX1_HS0CLK     |
             PLL_PIN_GPIO_MUX1_HS0VALEN   |
             PLL_PIN_GPIO_MUX1_HS0SYNC    |
             PLL_PIN_GPIO_MUX1_HS0RW      |
             PLL_PIN_GPIO_MUX1_HS1D2      |
             PLL_PIN_GPIO_MUX1_HS1D3      |
             PLL_PIN_GPIO_MUX1_HS1D4      |
             PLL_PIN_GPIO_MUX1_HS1D5      |
             PLL_PIN_GPIO_MUX1_HS1D6      |
             PLL_PIN_GPIO_MUX1_HS1D7      |
             PLL_PIN_GPIO_MUX1_HS1SYNC    |
             PLL_PIN_GPIO_MUX1_HS1ERRAV   |
             PLL_PIN_GPIO_MUX1_HS1RW      |
	     PLL_PIN_GPIO_MUX1_NIM0FAIL   |
             PLL_PIN_GPIO_MUX1_NIM1SYNC   |
             PLL_PIN_GPIO_MUX1_NIM1DVAL,
	  
	  /* mux2 */
           PLL_PIN_GPIO_MUX2_NIM1D0       |
             PLL_PIN_GPIO_MUX2_NIM1CLK    |
             PLL_PIN_GPIO_MUX2_NIM1FAIL,
  };


   /* On abilene, the pio is done by a PIO expander memory mapped
      at 0x3138xxxx.  The bits in the structure have these meanings:

      Bit 0: HSDP0 1 = serial (zephyr)/0 = parallel (edge) selector (bit 14)
      Bit 1: 1 = 1394 Clk Enable (bit 15).

      On abilene, the edge connector can be routed to HSDP0 or HSDP1.  This
      driver wasn't architected to support a TS_PORT to go to multiple HSDP_PORTs.
      Since there could be some wacky routes, we're limiting, in software, the
      ability to connect 1394, CI, and any other device that connects to the
      edge connector to route to HSDP1 only.
   */
static LPREG lpGPIOExpander = (LPREG)CNXT_AB_PIO_EXP_REG;
  tux_data ts_mux_data[] =  {
    /* NIM0 is a serial  NIM connected directly to the
       chip, thus, it doesn't matter what the GPIOs are */
       { TRUE,   HSDP_NIM0,   HSDP_TYPE_SER,    0x0,    0x0,    HSDP_NC,    HSDP_TYPE_NONE,    0x0,     -1},
    /* NIM1 is actually still "NIM0" on klondike but its
       connected to HSDP0 on the chip. */
       { TRUE,   HSDP_NIM1,   HSDP_TYPE_SER,    0x0,    0x0,    HSDP_NC,    HSDP_TYPE_NONE,    0x0,     -1},
    /* NIM 2 and NIM 3 don't exist on abilene */
       {FALSE,     HSDP_NC,  HSDP_TYPE_NONE,      0,     -1,    HSDP_NC,    HSDP_TYPE_NONE,      0,     -1},
       {FALSE,     HSDP_NC,  HSDP_TYPE_NONE,      0,     -1,    HSDP_NC,    HSDP_TYPE_NONE,      0,     -1},
    /* DVB baseband */
       { TRUE,  HSDP_HSDP0,   HSDP_TYPE_SER, 0xC000, 0x4000, HSDP_HSDP1,     HSDP_TYPE_PAR, 0x4000, 0x0000},
    /* OTV port */
       { TRUE,  HSDP_HSDP0,   HSDP_TYPE_SER, 0xC000, 0x4000,    HSDP_NC,    HSDP_TYPE_NONE,    0x0,     -1},
    /* Common Interfce */
       { TRUE,  HSDP_HSDP0,   HSDP_TYPE_PAR, 0xC000, 0x0000, HSDP_HSDP1,     HSDP_TYPE_PAR, 0x0000, 0x0000},
    /* 1394 uses the same bus as CI but disables HS0 CLK */
       { TRUE,  HSDP_HSDP0,  HSDP_TYPE_1394, 0xC000, 0x8000, HSDP_HSDP1,    HSDP_TYPE_1394, 0x0000, 0x0000},
    /* NIM4 doesn't exist on abilene */
       {FALSE,     HSDP_NC,  HSDP_TYPE_NONE,      0,     -1,    HSDP_NC,    HSDP_TYPE_NONE,      0,     -1},
    /* POD doesn't exist on abilene */
       {FALSE,     HSDP_NC,  HSDP_TYPE_NONE,      0,     -1,    HSDP_NC,    HSDP_TYPE_NONE,      0,     -1}
  };

TSROUTE_STATUS cnxt_tux_route_input(TS_PORT port, HSDP_PORT *connection, HSDP_PORT_TYPE *port_type) {

    if ( (ts_mux_data[port].legal) && (ts_mux_data[port].input_port != HSDP_NC) ) {

        *lpGPIOExpander &= (~ts_mux_data[port].ts_mux_input_mask);
        *lpGPIOExpander |= (ts_mux_data[port].ts_mux_input_values);

        *connection = ts_mux_data[port].input_port;
        *port_type  = ts_mux_data[port].inport_type;

        return TSROUTE_STATUS_OK;
    } else {
        *connection = HSDP_NC;
        return TSROUTE_STATUS_INVALID;
    }
}

TSROUTE_STATUS cnxt_tux_route_output(TS_PORT port, HSDP_PORT *connection, HSDP_PORT_TYPE *port_type) {) {
    /* This should be the same exact routine as the route_input
       routine except all "inputs" change to "outputs" */

    if ( (ts_mux_data[port].legal) && (ts_mux_data[port].output_port != HSDP_NC) ) {
        *lpGPIOExpander &= (~ts_mux_data[port].ts_mux_output_mask);
        *lpGPIOExpander |= (ts_mux_data[port].ts_mux_output_values);

        *connection = ts_mux_data[port].output_port;
        *port_type  = ts_mux_data[port].outport_type;

        return TSROUTE_STATUS_OK;
    } else {
        *connection = HSDP_NC;
        return TSROUTE_STATUS_INVALID;
    }

}

#endif


















/*****************************************************************************/
/*****************************************************************************/
/*   TS_MUX_TYPE == ATHENS                                                   */
/*****************************************************************************/
/*****************************************************************************/

#if TS_MUX_TYPE == TS_MUX_TYPE_ATHENS
  tux_initdata initdata = { 
	  /* alt_func mask */
	  0x0,
	  /* alt_func value */
	  0x0,
	  /* mux0 */
           PLL_PIN_GPIO_MUX0_HS1CLK       | 
               PLL_PIN_GPIO_MUX0_HS1D0    |
               PLL_PIN_GPIO_MUX0_HS1D1,
        
	  /* mux1 */
           PLL_PIN_GPIO_MUX1_HS0D0        |
           PLL_PIN_GPIO_MUX1_HS0D1        |
           PLL_PIN_GPIO_MUX1_HS0D2        |
           PLL_PIN_GPIO_MUX1_HS0D3        |
           PLL_PIN_GPIO_MUX1_HS0D4        |
           PLL_PIN_GPIO_MUX1_HS0D5        |
           PLL_PIN_GPIO_MUX1_HS0D6        |
           PLL_PIN_GPIO_MUX1_HS0D7        |
           PLL_PIN_GPIO_MUX1_HS0CLK       |
           PLL_PIN_GPIO_MUX1_HS0VALEN     |
           PLL_PIN_GPIO_MUX1_HS0SYNC      |
           PLL_PIN_GPIO_MUX1_HS0ERRAV     |
           PLL_PIN_GPIO_MUX1_HS1D2        |
           PLL_PIN_GPIO_MUX1_HS1D3        |
           PLL_PIN_GPIO_MUX1_HS1D4        | 
           PLL_PIN_GPIO_MUX1_HS1D5        |
           PLL_PIN_GPIO_MUX1_HS1D6        |
           PLL_PIN_GPIO_MUX1_HS1D7        |
           PLL_PIN_GPIO_MUX1_HS1SYNC      |
           PLL_PIN_GPIO_MUX1_HS1ERRAV     |
           PLL_PIN_GPIO_MUX1_HS1RW        |
           PLL_PIN_GPIO_MUX1_HS1VALEN,
          
	  /* mux2 */
          0x0
  };


   /* On athens, the pio is done by a PIO expander memory mapped
      at 0x3138xxxx.  The bits in the structure have these meanings:

      Bit 0: HSDP0 1 = serial (zephyr)/0 = parallel (edge) selector (bit 6)
      Bit 1: 1 = 1394 Clk Enable (bit 7).

      On athens, the edge connector can be routed to HSDP0 or HSDP1.  This
      driver wasn't architected to support a TS_PORT to go to multiple HSDP_PORTs.
      Since there could be some wacky routes, we're limiting, in software, the
      ability to connect 1394, CI, and any other device that connects to the
      edge connector to route to HSDP1 only.
   */
static LPREG lpGPIOExpander = (LPREG)CNXT_PIO_EXP_REG;
  tux_data ts_mux_data[] =  {
    /* NIM0 is ED0 on WaBASH */
       {TRUE,   HSDP_NIM0,   HSDP_TYPE_SER,    0x0,    0x0,    HSDP_NC,  HSDP_TYPE_NONE,    0x0,     -1},
    /* NIM1 has no connection */
       {TRUE,   HSDP_NIM1,  HSDP_TYPE_NONE,    0x0,    0x0,    HSDP_NC,  HSDP_TYPE_NONE,    0x0,     -1},
    /* NIM 2 and NIM 3 are internal QAM demods on wabash */
       {TRUE,   HSDP_NIM2,   HSDP_TYPE_PAR,      0,    0x0,    HSDP_NC,  HSDP_TYPE_NONE,      0,     -1},
       {TRUE,   HSDP_NIM3,   HSDP_TYPE_PAR,      0,    0x0,    HSDP_NC,  HSDP_TYPE_NONE,      0,     -1},
    /* DVB baseband */
       {TRUE,  HSDP_HSDP0,   HSDP_TYPE_SER,   0xC0,   0x40, HSDP_HSDP1,   HSDP_TYPE_PAR,   0x40,    0x0},
    /* OTV port */
       {TRUE,  HSDP_HSDP0,   HSDP_TYPE_SER,   0xC0,   0x40,    HSDP_NC,  HSDP_TYPE_NONE,   0x00,     -1},
    /* Common Interfce */
       {TRUE,  HSDP_HSDP0,   HSDP_TYPE_PAR,   0xC0,   0x00, HSDP_HSDP1,   HSDP_TYPE_PAR,   0x00,   0x00},
    /* 1394 uses the same bus as CI but disables HS0 CLK */
       {TRUE,  HSDP_HSDP0,  HSDP_TYPE_1394,   0xC0,   0x80, HSDP_HSDP1,  HSDP_TYPE_1394,   0x00,   0x00},
    /* NIM4 is the OOB demod, internal, parellel */
       {TRUE,   HSDP_NIM4,   HSDP_TYPE_PAR,      0,    0x0,    HSDP_NC,  HSDP_TYPE_NONE,      0      -1},
    /* POD looks the same as common interface */
       {TRUE,  HSDP_HSDP0,   HSDP_TYPE_PAR,   0xC0,   0x00, HSDP_HSDP1,   HSDP_TYPE_PAR,   0x00,   0x00},
  };

TSROUTE_STATUS cnxt_tux_route_input(TS_PORT port, HSDP_PORT *connection, HSDP_PORT_TYPE *port_type) {

    if ( (ts_mux_data[port].legal) && (ts_mux_data[port].input_port != HSDP_NC) ) {

        CNXT_PIO_EXP_REG_SHADOW &= (~ts_mux_data[port].ts_mux_input_mask);
        CNXT_PIO_EXP_REG_SHADOW |= (ts_mux_data[port].ts_mux_input_values);
        *lpGPIOExpander = CNXT_PIO_EXP_REG_SHADOW;
        
        *connection = ts_mux_data[port].input_port;
        *port_type  = ts_mux_data[port].inport_type;
        return TSROUTE_STATUS_OK;
    } else {
        *connection = HSDP_NC;
        return TSROUTE_STATUS_INVALID;
    }
}

TSROUTE_STATUS cnxt_tux_route_output(TS_PORT port, HSDP_PORT *connection, HSDP_PORT_TYPE *port_type) {
    /* This should be the same exact routine as the route_input
       routine except all "inputs" change to "outputs" */

    if ( (ts_mux_data[port].legal) && (ts_mux_data[port].output_port != HSDP_NC) ) {
        CNXT_PIO_EXP_REG_SHADOW &= (~ts_mux_data[port].ts_mux_output_mask);
        CNXT_PIO_EXP_REG_SHADOW |= (ts_mux_data[port].ts_mux_output_values);
        *lpGPIOExpander = CNXT_PIO_EXP_REG_SHADOW;
        
        *connection = ts_mux_data[port].output_port;
        *port_type  = ts_mux_data[port].outport_type;

        return TSROUTE_STATUS_OK;
    } else {
        *connection = HSDP_NC;
        return TSROUTE_STATUS_INVALID;
    }

}

#endif









/*****************************************************************************/
/*****************************************************************************/
/*   TS_MUX_TYPE == MILANO                                                   */
/*****************************************************************************/
/*****************************************************************************/

#if TS_MUX_TYPE == TS_MUX_TYPE_MILANO
  tux_initdata initdata = { 
	  /* alt_func mask */
	  PLL_PIN_ALT_FUNC_ED_ENABLE_MASK | PLL_PIN_ALT_FUNC_ED_ALTERNATE_MASK,
	  /* alt_func value */
	  PLL_PIN_ALT_FUNC_ED_ENABLE_ENABLED | PLL_PIN_ALT_FUNC_ED_ALTERNATE_ALTERNATE,
	  /* mux0 */
           PLL_PIN_GPIO_MUX0_HS1CLK       | 
               PLL_PIN_GPIO_MUX0_HS1D0    |
               PLL_PIN_GPIO_MUX0_HS1D1,
        
	  /* mux1 */
           PLL_PIN_GPIO_MUX1_HS0D0        |
           PLL_PIN_GPIO_MUX1_HS0CLK       |
           PLL_PIN_GPIO_MUX1_HS0VALEN     |
           PLL_PIN_GPIO_MUX1_HS0SYNC      |
           PLL_PIN_GPIO_MUX1_HS0ERRAV     |
           PLL_PIN_GPIO_MUX1_HS1D2        |
           PLL_PIN_GPIO_MUX1_HS1D3        |
           PLL_PIN_GPIO_MUX1_HS1D4        | 
           PLL_PIN_GPIO_MUX1_HS1D5        |
           PLL_PIN_GPIO_MUX1_HS1D6        |
           PLL_PIN_GPIO_MUX1_HS1D7        |
           PLL_PIN_GPIO_MUX1_HS1SYNC      |
           PLL_PIN_GPIO_MUX1_HS1ERRAV     |
           PLL_PIN_GPIO_MUX1_HS1RW        |
           PLL_PIN_GPIO_MUX1_HS1VALEN,
          
	  /* mux2 */
          0x0
  };


   /* On milano, the pio is done by a write-only PIO expander memory mapped
      at 0x3138xxxx.  The bits in the structure have these meanings:

      DVB/POD into ED0       PIO Expander Bit 5 (SD)
      
      On milano, there is one mux which which selects between POD output and DVB.
      The output of that mux is connected to ED0.  
      HSDP1 routes to POD input
      HSDP0 is connected to a 1394 adapter in serial mode (acts as input and output?)
      
   */
static LPREG lpGPIOExpander = (LPREG)CNXT_PIO_EXP_REG;
  tux_data ts_mux_data[] =  {
    /* NIM0 which is ED0 on a WaBASH is used for POD/DVB baseband input */
       {FALSE,  HSDP_NIM0,  HSDP_TYPE_NONE,    0x0,    0x0,   HSDP_NC,     HSDP_TYPE_NONE,      0,     -1},
    /* NIM1 has no connection */
       {FALSE,  HSDP_NIM1,  HSDP_TYPE_NONE,    0x0,    0x0,   HSDP_NC,     HSDP_TYPE_NONE,      0,     -1},
    /* NIM 2 and NIM 3 are internal QAM demods on wabash */
       {TRUE,   HSDP_NIM2,  HSDP_TYPE_PAR,      0,    0x0,    HSDP_NC,     HSDP_TYPE_NONE,      0,     -1},
       {TRUE,   HSDP_NIM3,  HSDP_TYPE_PAR,      0,    0x0,    HSDP_NC,     HSDP_TYPE_NONE,      0,     -1},
    /* DVB baseband */
       {TRUE,   HSDP_NIM0,  HSDP_TYPE_SER,   0x20,   0x20,    HSDP_NC,     HSDP_TYPE_NONE,      0,     -1},
    /* OTV port */
       {TRUE,   HSDP_NIM0,  HSDP_TYPE_SER,   0x20,   0x20,    HSDP_NC,     HSDP_TYPE_NONE,      0,     -1},
    /* Common Interfce */
       {FALSE,  HSDP_NC,    HSDP_TYPE_PAR,      0,      0,    HSDP_NC,     HSDP_TYPE_NONE,      0,     -1},
    /* 1394 outputs on HSDP0.  Looks like it also inputs on HSDP0.... can not simultaneously */
       {TRUE,   HSDP_NC,    HSDP_TYPE_NONE,   0x00,   0x00,   HSDP_HSDP0,  HSDP_TYPE_1394,      0,     0},
    /* NIM4 is the OOB demod, internal, parellel */
       {TRUE,   HSDP_NIM4,  HSDP_TYPE_PAR,      0,    0x0,    HSDP_NC,     HSDP_TYPE_NONE,      0      -1},
    /* POD looks the same as common interface */
       {TRUE,   HSDP_NIM0,  HSDP_TYPE_SER,   0x20,   0x00,    HSDP_HSDP1,  HSDP_TYPE_PAR,   0x00,   0x00},
  };

TSROUTE_STATUS cnxt_tux_route_input(TS_PORT port, HSDP_PORT *connection, HSDP_PORT_TYPE *port_type) {
    if ( (ts_mux_data[port].legal) && (ts_mux_data[port].input_port != HSDP_NC) ) {

        CNXT_PIO_EXP_REG_SHADOW &= (~ts_mux_data[port].ts_mux_input_mask);
        CNXT_PIO_EXP_REG_SHADOW |= (ts_mux_data[port].ts_mux_input_values);
        *lpGPIOExpander = CNXT_PIO_EXP_REG_SHADOW;
        
        *connection = ts_mux_data[port].input_port;
        *port_type  = ts_mux_data[port].inport_type;
        return TSROUTE_STATUS_OK;
    } else {
        *connection = HSDP_NC;
        return TSROUTE_STATUS_INVALID;
    }
}

TSROUTE_STATUS cnxt_tux_route_output(TS_PORT port, HSDP_PORT *connection, HSDP_PORT_TYPE *port_type) {
    /* This should be the same exact routine as the route_input
       routine except all "inputs" change to "outputs" */

    if ( (ts_mux_data[port].legal) && (ts_mux_data[port].output_port != HSDP_NC) ) {
        CNXT_PIO_EXP_REG_SHADOW &= (~ts_mux_data[port].ts_mux_output_mask);
        CNXT_PIO_EXP_REG_SHADOW |= (ts_mux_data[port].ts_mux_output_values);
        *lpGPIOExpander = CNXT_PIO_EXP_REG_SHADOW;
        
        *connection = ts_mux_data[port].output_port;
        *port_type  = ts_mux_data[port].outport_type;

        return TSROUTE_STATUS_OK;
    } else {
        *connection = HSDP_NC;
        return TSROUTE_STATUS_INVALID;
    }

}

#endif















/*****************************************************************************/
/*****************************************************************************/
/*   TS_MUX_TYPE == BRADY                                                    */
/*****************************************************************************/
/*****************************************************************************/
#if TS_MUX_TYPE == TS_MUX_TYPE_BRADY

  tux_initdata initdata = { 
	  /* alt_func mask */
	  0x0,
	  /* alt_func value */
	  0x0,
	  /* mux0 */
	  0x0,
	  /* mux1 */
          PLL_PIN_GPIO_MUX1_HS0ERRAV      |
             PLL_PIN_GPIO_MUX1_HS0D0      | 
             PLL_PIN_GPIO_MUX1_HS0D1      | 
             PLL_PIN_GPIO_MUX1_HS0D2      | 
             PLL_PIN_GPIO_MUX1_HS0D3      |
             PLL_PIN_GPIO_MUX1_HS0D4      |
             PLL_PIN_GPIO_MUX1_HS0D5      |
             PLL_PIN_GPIO_MUX1_HS0D6      |
             PLL_PIN_GPIO_MUX1_HS0D7      |
             PLL_PIN_GPIO_MUX1_HS0CLK     |
             PLL_PIN_GPIO_MUX1_HS0VALEN   |
             PLL_PIN_GPIO_MUX1_HS0SYNC    |
             PLL_PIN_GPIO_MUX1_HS0RW,
          /* mux2 */
          0x0
  };

  /* setup input for Brady, Brady Basically has only
     one single Parrallel Transport stream interface from
     the Terrestrial NIM */

  tux_data ts_mux_data[] =  {
    /* NIM0 is a Parrallel NIM connected directly to the chip */
       { TRUE,   HSDP_HSDP0,   HSDP_TYPE_PAR,     0,    0,    HSDP_NC,  HSDP_TYPE_NONE,     0,   -1},    
    /* NIM1 is actually still "NIM0" on klondike but its
       connected to HSDP0 on the chip. */
       {FALSE,   HSDP_HSDP0,   HSDP_TYPE_PAR,     0,    0,    HSDP_NC,  HSDP_TYPE_NONE,     0,   -1},
    /* NIM 2 and NIM 3 don't exist on klondike */
       {FALSE,      HSDP_NC,  HSDP_TYPE_NONE,     0,   -1,    HSDP_NC,  HSDP_TYPE_NONE,     0,   -1},
       {FALSE,      HSDP_NC,  HSDP_TYPE_NONE,     0,   -1,    HSDP_NC,  HSDP_TYPE_NONE,     0,   -1},
    /* DVB baseband */
       { TRUE,    HSDP_NIM0,   HSDP_TYPE_SER,     0,    0,    HSDP_NC,  HSDP_TYPE_NONE,     0,   -1},
    /* OTV port */
       {FALSE,      HSDP_NC,  HSDP_TYPE_NONE,     0,   -1,    HSDP_NC,  HSDP_TYPE_NONE,     0,   -1},
    /* Common Interfce */
       {FALSE,      HSDP_NC,  HSDP_TYPE_NONE,     0,   -1,    HSDP_NC,  HSDP_TYPE_NONE,     0,   -1},
    /* 1394 uses the same bus as CI but disables HS0 CLK */
       {FALSE,      HSDP_NC,  HSDP_TYPE_NONE,     0,   -1,    HSDP_NC,  HSDP_TYPE_NONE,     0,   -1},
    /* NIM4 doesn't exist on brady */
       {FALSE,      HSDP_NC,  HSDP_TYPE_NONE,     0,   -1,    HSDP_NC,  HSDP_TYPE_NONE,     0,   -1},
    /* POD doesn't exist on brady */
       {FALSE,      HSDP_NC,  HSDP_TYPE_NONE,     0,   -1,    HSDP_NC,  HSDP_TYPE_NONE,     0,   -1}
  };

TSROUTE_STATUS cnxt_tux_route_input(TS_PORT port, HSDP_PORT *connection, HSDP_PORT_TYPE *port_type) 
{
   bool ks;

    if ( (ts_mux_data[port].legal) && (ts_mux_data[port].input_port != HSDP_NC) ) {
      /* On Colorado, we also have to worry about the internal transport */
      /* stream muxing.                                                  */
      ks = critical_section_begin();

      CNXT_SET_VAL(HSDP_TSB_PORT_CNTL_REG,
              HSDP_BIDIR_PORT_CNTL_OUTPUT_DATA_SEL_MASK,
              HSDP_BIDIR_PORT_CNTL_OUTPUT_DATA_SEL_DEMUX);

      CNXT_SET_VAL(HSDP_SP_INPUT_CNTL_REG,
              HSDP_SP_INPUT_CNTL_PARSER0_INPUT_SEL_MASK,
              HSDP_SP_INPUT_CNTL_PARSER0_BIDIR_PORT_0);

      critical_section_end(ks);

      *connection = ts_mux_data[port].input_port;
      *port_type  = ts_mux_data[port].inport_type;
      return TSROUTE_STATUS_OK;
   }
   else
   {
      *connection = HSDP_NC;
      return TSROUTE_STATUS_INVALID;
   }
}

/*
 * Brady has a Single Serial Bi-direction Port.
 * Note : use the Zephyr box to convert into Parallel TS
 */

TSROUTE_STATUS cnxt_tux_route_output(TS_PORT port, HSDP_PORT *connection, HSDP_PORT_TYPE *port_type) {
    /* This should be the same exact routine as the route_input
       routine except all "inputs" change to "outputs" */

    if ( (ts_mux_data[port].legal) && (ts_mux_data[port].output_port != HSDP_NC) ) {

         /* GRB TODO */


        *port_type  = ts_mux_data[port].outport_type;


        return TSROUTE_STATUS_OK;
    } else {
        *connection = HSDP_NC;
        return TSROUTE_STATUS_INVALID;
    }

}
#endif


















/*****************************************************************************/
/*****************************************************************************/
/*   TS_MUX_TYPE == BRONCO                                                   */
/*****************************************************************************/
/*****************************************************************************/
#if TS_MUX_TYPE == TS_MUX_TYPE_BRONCO
  tux_initdata initdata = { 
	  /* alt_func mask */
	  0x0,
	  /* alt_func value */
	  0x0,
	  /* mux0 */
           PLL_PIN_GPIO_MUX0_HS1CLK       | 
               PLL_PIN_GPIO_MUX0_HS1D0    |
               PLL_PIN_GPIO_MUX0_HS1D1,
 	  /* mux1 */
          PLL_PIN_GPIO_MUX1_HS0ERRAV      |
             PLL_PIN_GPIO_MUX1_HS0D0      | 
             PLL_PIN_GPIO_MUX1_HS0D1      | 
             PLL_PIN_GPIO_MUX1_HS0D2      | 
             PLL_PIN_GPIO_MUX1_HS0D3      |
             PLL_PIN_GPIO_MUX1_HS0D4      |
             PLL_PIN_GPIO_MUX1_HS0D5      |
             PLL_PIN_GPIO_MUX1_HS0D6      |
             PLL_PIN_GPIO_MUX1_HS0D7      |
             PLL_PIN_GPIO_MUX1_HS0CLK     |
             PLL_PIN_GPIO_MUX1_HS0VALEN   |
             PLL_PIN_GPIO_MUX1_HS0SYNC    |
             PLL_PIN_GPIO_MUX1_HS0RW      |
             PLL_PIN_GPIO_MUX1_HS1D2      |
             PLL_PIN_GPIO_MUX1_HS1D3      |
             PLL_PIN_GPIO_MUX1_HS1D4      | 
             PLL_PIN_GPIO_MUX1_HS1D5      |
             PLL_PIN_GPIO_MUX1_HS1D6      |
             PLL_PIN_GPIO_MUX1_HS1D7      |
             PLL_PIN_GPIO_MUX1_HS1SYNC    |
             PLL_PIN_GPIO_MUX1_HS1ERRAV   |
             PLL_PIN_GPIO_MUX1_HS1RW      |
             PLL_PIN_GPIO_MUX1_HS1VALEN,
 
          /* mux2 */
          0x0
  };


   /* On Bronco/Brazos, there is no TS mux.  There is one internal
    * QPSK demod which is connected to Parallel NIM0.  NIM1 is a
    * serial HSDP input.  HSDP0 and HSDP1 are serial or parallel 
    * bidirectional ports.
   */

   /* tux_data follows the order of the TS_PORT enumeration in hsdp.h */
   
tux_data ts_mux_data[] =  {
    /* NIM0 is an internal demod and doesn't need muxing */
       { TRUE,  HSDP_NIM0,   HSDP_TYPE_SER,     0,      0,      HSDP_NC,  HSDP_TYPE_NONE,  0,   -1},
    /* NIM1 is for an external demod but may also be used as the input from
     * a zephyr box.  there is no mux so gpios don't matter. */
       { TRUE,  HSDP_NIM1,   HSDP_TYPE_SER,     0,      0,      HSDP_NC,  HSDP_TYPE_NONE,  0,   -1},
    /* NIM 2 is the internal demod in parallel mode */
       { TRUE,  HSDP_NIM2,   HSDP_TYPE_PAR,     0,      0,      HSDP_NC,  HSDP_TYPE_NONE,  0,   -1},
    /* NIM 3 is N/A on brazos */
       {FALSE,    HSDP_NC,  HSDP_TYPE_NONE,     0,     -1,      HSDP_NC,  HSDP_TYPE_NONE,  0,   -1},
    /* DVB baseband & OTV both come off a zephyr and are USUALLY connected  
     * to HSDP1 */
       { TRUE, HSDP_HSDP1,   HSDP_TYPE_PAR,     0,      0,   HSDP_HSDP1,   HSDP_TYPE_PAR,  0,    0},
       { TRUE, HSDP_HSDP1,   HSDP_TYPE_PAR,     0,      0,      HSDP_NC,  HSDP_TYPE_NONE,  0,   -1},
    /* Common Interfce */
       {TRUE, HSDP_HSDP0,   HSDP_TYPE_PAR,  0xC0,      0,   HSDP_HSDP1,   HSDP_TYPE_PAR,  0,    0},
    /* 1394 uses the same bus as CI but disables HS0 CLK */
       {FALSE, HSDP_HSDP0,  HSDP_TYPE_1394,  0xC0,   0x80,   HSDP_HSDP1,  HSDP_TYPE_1394,  0,    0}
  };

TSROUTE_STATUS cnxt_tux_route_input(TS_PORT port, HSDP_PORT *connection, HSDP_PORT_TYPE *port_type) {

    if ( (ts_mux_data[port].legal) && (ts_mux_data[port].input_port != HSDP_NC) ) {

        *connection = ts_mux_data[port].input_port;
        *port_type  = ts_mux_data[port].inport_type;

/*
 * !!! HACK ALERT !!!
 * WARNING!!!!! Hack requested for supporting Bronco1 & Bronco3 at runtime.  When
 * Bronco1 boards disappear, so should this hack.
 * !!! HACK ALERT !!!
 */
#if ((HSDP_TYPE == HSDP_BRAZOS) && (I2C_CONFIG_EEPROM_ADDR != NOT_PRESENT))
	if(!ConfigurationValid ||
	   ((config_table.board_type == 0x00) && (config_table.board_rev == 0x01))) {
	  if((port == TS_PORT_DVB) || (port == TS_PORT_OTV)) {
	    *port_type = HSDP_TYPE_SER;
	  }
	}      
#endif
/*
 * !!! HACK ALERT !!!
 * !!! HACK ALERT !!!
 */

        return TSROUTE_STATUS_OK;
    } else {
        *connection = HSDP_NC;
        return TSROUTE_STATUS_INVALID;
    }
}

TSROUTE_STATUS cnxt_tux_route_output(TS_PORT port, HSDP_PORT *connection, HSDP_PORT_TYPE *port_type) {
    /* This should be the same exact routine as the route_input
       routine except all "inputs" change to "outputs" */

    if ( (ts_mux_data[port].legal) && (ts_mux_data[port].input_port != HSDP_NC) ) {

        *connection = ts_mux_data[port].output_port;
        *port_type  = ts_mux_data[port].outport_type;

        return TSROUTE_STATUS_OK;
    } else {
        *connection = HSDP_NC;
        return TSROUTE_STATUS_INVALID;
    }

}

#endif



TSROUTE_STATUS cnxt_tux_init()  {
    /* If semaphores are required to make the muxing thread-safe, then the semaphore
     * should be created here... At this time I don't think it is required because
     * the muxing is probably going to get set at init time and not change.
     */
	
    /* Here we're also going to hit the chip's GPIOs/pin muxing as defined in 
     * tux_initdata because which HSDP ports on the chip are actually in use is a board
     * specific detail.
     */
#if PLL_PIN_ALT_FUNC_REG_DEFAULT == NOT_DETERMINED 
    *(LPREG)PLL_PIN_ALT_FUNC_REG = 
	    (*(LPREG)PLL_PIN_ALT_FUNC_REG & ~(initdata.pio_altfunc_mask)) |
	    initdata.pio_altfunc_val;
#endif

#if PLL_PIN_GPIO_MUX0_REG_DEFAULT == NOT_DETERMINED
    *(LPREG)PLL_PIN_GPIO_MUX0_REG |= initdata.pio_pinmux0;
#endif

#if PLL_PIN_GPIO_MUX1_REG_DEFAULT == NOT_DETERMINED
    *(LPREG)PLL_PIN_GPIO_MUX1_REG |= initdata.pio_pinmux1;
#endif
    
#if PLL_PIN_GPIO_MUX2_REG_DEFAULT == NOT_DETERMINED
    *(LPREG)PLL_PIN_GPIO_MUX2_REG |= initdata.pio_pinmux2;
#endif


    return TSROUTE_STATUS_OK;
}

/*****************************************************************************
 * $Log: 
 *  25   mpeg      1.24        4/21/04 11:53:32 AM    Steve Glennon   CR(s) 
 *        8901 8902 : Modified the Brazos support to change TS_PORT_CI from an 
 *        "illegal" routing
 *        to a legal one so we can support DVBCI through the Haskell board 
 *        attached to Brazos.
 *        
 *  24   mpeg      1.23        8/8/03 7:53:38 PM      Sahil Bansal    SCR(s): 
 *        7221 7222 
 *        Changed ts_mux_data entry for 1394 output so that output value is 0 
 *        and not -1 since it was causing wrong gpio lines on gpio expander to 
 *        be toggled when cnxt_tux_route_output() was called for 1394 ts 
 *        routing.  This then caused ethernet on cm to go down.
 *        
 *  23   mpeg      1.22        7/30/03 2:02:54 PM     Sahil Bansal    SCR(s): 
 *        6675 
 *        Changed code to support 1394 ts routing outputted to HSDP0
 *        
 *  22   mpeg      1.21        7/28/03 3:42:04 PM     Miles Bintz     SCR(s) 
 *        7060 :
 *        changed bronco DVB baseband out to HSDP1
 *        
 *  21   mpeg      1.20        7/25/03 10:49:12 AM    Miles Bintz     SCR(s) 
 *        7045 :
 *        removed unmactched brace
 *        
 *  20   mpeg      1.19        7/22/03 1:12:16 PM     Miles Bintz     SCR(s) 
 *        6988 :
 *        removed unmatched bracket
 *        
 *  19   mpeg      1.18        7/18/03 3:49:26 PM     Miles Bintz     SCR(s) 
 *        6988 :
 *        gpio expander shouldn't be touched if input_port or output_port is 
 *        set to HSDP_NC
 *        
 *  18   mpeg      1.17        7/2/03 2:10:44 PM      Craig Dry       SCR(s) 
 *        6870 :
 *        For Bronco 1, the Aux Baseband port was being identified as Parallel.
 *          It is now properly identified as Serial.
 *        
 *  17   mpeg      1.16        6/30/03 5:47:32 PM     Miles Bintz     SCR(s) 
 *        6807 :
 *        changes to support milano HSDP routing
 *        
 *  16   mpeg      1.15        5/21/03 1:47:46 PM     Brendan Donahe  SCR(s) 
 *        6503 6504 :
 *        Added code to conditionally force use of serial input only for Bronco
 *         1 
 *        boards.
 *        
 *        
 *  15   mpeg      1.14        5/15/03 5:05:12 PM     Miles Bintz     SCR(s) 
 *        6364 6365 :
 *        changed port type from serial to parallel for baseband input
 *        
 *  14   mpeg      1.13        5/14/03 9:45:48 PM     Miles Bintz     SCR(s) 
 *        6353 6354 :
 *        rework of HSDP driver
 *        
 *  13   mpeg      1.12        5/8/03 4:07:38 PM      Craig Dry       SCR(s) 
 *        5521 :
 *        Conditionally remove modifications to GPIO Pin Mux registers.
 *        
 *  12   mpeg      1.11        4/1/03 2:03:46 PM      Tim White       SCR(s) 
 *        5925 :
 *        Remove the Bronco3 specifc route check.
 *        
 *        
 *  11   mpeg      1.10        3/13/03 12:10:58 PM    Miles Bintz     SCR(s) 
 *        5753 5754 :
 *        Modified to support Bronco Rev 3 which uses parallel zephyr header 
 *        instead of serial zephyr header
 *        
 *        
 *  10   mpeg      1.9         1/28/03 11:27:14 AM    Billy Jackman   SCR(s) 
 *        5336 :
 *        Use NIM2 as the internal demod in parallel mode.
 *        
 *  9    mpeg      1.8         1/23/03 3:02:44 PM     Dave Wilson     SCR(s) 
 *        5292 :
 *        Updates to allow correct operation of the baseband inputs on the 
 *        Bronco IRD.
 *        
 *  8    mpeg      1.7         1/9/03 10:52:50 AM     Ian Mitchell    SCR(s): 
 *        5198 
 *        Changed configuration data for the basband input on the Brady 
 *        development platform.
 *        
 *  7    mpeg      1.6         12/5/02 2:18:32 PM     Miles Bintz     SCR(s) 
 *        5074 :
 *        Added structures to support the brazos chip
 *        
 *        
 *  6    mpeg      1.5         7/12/02 8:13:34 AM     Steven Jones    SCR(s): 
 *        4176 
 *        Fix for Brady.
 *        
 *  5    mpeg      1.4         6/13/02 4:28:30 PM     Miles Bintz     SCR(s) 
 *        4001 :
 *        changed routing to allow internal qam demod
 *        
 *  4    mpeg      1.3         4/12/02 2:34:42 PM     Ray Mack        SCR(s) 
 *        3545 :
 *        changed the way that the serial vs parallel decision is made to match
 *         reality.
 *        
 *  3    mpeg      1.2         1/30/02 2:06:44 PM     Miles Bintz     SCR(s) 
 *        3058 :
 *        Development/interim checkin
 *        
 *        
 *  2    mpeg      1.1         1/21/02 2:44:24 PM     Miles Bintz     SCR(s) 
 *        3058 :
 *        api tweaks and document updates
 *        
 *        
 *  1    mpeg      1.0         1/18/02 1:35:02 PM     Miles Bintz     
 * $
 * 
 *    Rev 1.23   08 Aug 2003 18:53:38   bansals
 * SCR(s): 7221 7222 
 * Changed ts_mux_data entry for 1394 output so that output value is 0 and not -1 since it was causing wrong gpio lines on gpio expander to be toggled when cnxt_tux_route_output() was called for 1394 ts routing.  This then caused ethernet on cm to go down.
 * 
 *    Rev 1.22   30 Jul 2003 13:02:54   bansals
 * SCR(s): 6675 
 * Changed code to support 1394 ts routing outputted to HSDP0
 * 
 *    Rev 1.21   28 Jul 2003 14:42:04   bintzmf
 * SCR(s) 7060 :
 * changed bronco DVB baseband out to HSDP1
 * 
 *    Rev 1.20   25 Jul 2003 09:49:12   bintzmf
 * SCR(s) 7045 :
 * removed unmactched brace
 * 
 *    Rev 1.19   22 Jul 2003 12:12:16   bintzmf
 * SCR(s) 6988 :
 * removed unmatched bracket
 * 
 *    Rev 1.18   18 Jul 2003 14:49:26   bintzmf
 * SCR(s) 6988 :
 * gpio expander shouldn't be touched if input_port or output_port is set to HSDP_NC
 * 
 *    Rev 1.17   02 Jul 2003 13:10:44   dryd
 * SCR(s) 6870 :
 * For Bronco 1, the Aux Baseband port was being identified as Parallel.  It is now properly identified as Serial.
 * 
 *    Rev 1.16   30 Jun 2003 16:47:32   bintzmf
 * SCR(s) 6807 :
 * changes to support milano HSDP routing
 * 
 *    Rev 1.15   21 May 2003 12:47:46   donaheb
 * SCR(s) 6503 6504 :
 * Added code to conditionally force use of serial input only for Bronco 1 
 * boards.
 * 
 * 
 *    Rev 1.14   15 May 2003 16:05:12   bintzmf
 * SCR(s) 6364 6365 :
 * changed port type from serial to parallel for baseband input
 * 
 *    Rev 1.13   14 May 2003 20:45:48   bintzmf
 * SCR(s) 6353 6354 :
 * rework of HSDP driver
 * 
 *    Rev 1.11   01 Apr 2003 14:03:46   whiteth
 * SCR(s) 5925 :
 * Remove the Bronco3 specifc route check.
 * 
 * 
 *    Rev 1.10   13 Mar 2003 12:10:58   bintzmf
 * SCR(s) 5753 5754 :
 * Modified to support Bronco Rev 3 which uses parallel zephyr header instead of serial zephyr header
 * 
 * 
 *    Rev 1.9   28 Jan 2003 11:27:14   jackmaw
 * SCR(s) 5336 :
 * Use NIM2 as the internal demod in parallel mode.
 * 
 *    Rev 1.8   23 Jan 2003 15:02:44   dawilson
 * SCR(s) 5292 :
 * Updates to allow correct operation of the baseband inputs on the Bronco IRD.
 * 
 *    Rev 1.7   09 Jan 2003 10:52:50   mitchei
 * SCR(s): 5198 
 * Changed configuration data for the basband input on the Brady development platform.
 * 
 *    Rev 1.6   05 Dec 2002 14:18:32   bintzmf
 * SCR(s) 5074 :
 * Added structures to support the brazos chip
 * 
 *****************************************************************************/

