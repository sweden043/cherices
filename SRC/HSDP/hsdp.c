/****************************************************************************/
/*                   CONEXANT PROPRIETARY AND CONFIDENTIAL                  */
/*                       SOFTWARE FILE/MODULE HEADER                        */
/*          Conexant Systems Inc. (c) 1998, 1999, 2000, 2001, 2002          */
/*                              Austin, TX                                  */
/*                         All Rights Reserved                              */
/****************************************************************************/
/*
 * Filename:        hsdp.c
 *
 *
 * Description:     HSDP route driver
 *
 *
 * Author:          Miles Bintz
 *
 ****************************************************************************/
/* $Id: hsdp.c,v 1.42, 2004-03-16 16:20:23Z, Miles Bintz$
 ****************************************************************************/

// notes to myself:  1. an api needs to be created to control the following features
// a. ci/1394 r/w mode
// b. clock source
// 2. defaults need to be set up so watchtv can call nim0->demux0, demux0->dvb
//   (after that, the app/developer needs to know what they are doing to the hsdp
//    cntl reg)
// 5. an api should be created to tell the HSDP0/1 whether it is ser or par
// 6. which bits say whether i/o is ser or par?  ser_to_par conversion?  what about par_to_ser?
// 7. support for ed0 needs to be added
// 

#include "stbcfg.h"
#include "kal.h"
#include "trace.h"
#include "hsdp.h"

/*************************/
/*        Externs        */
/*************************/
extern hsdp_port hsdp_port_data[];
extern const int hsdp_num_ports;
extern int is_route_legal(int src, int dest);


/*************************/
/*        Globals        */
/*************************/
static sem_id_t hsdp_sem;

#ifndef EXTERNAL_TS_SRC0
/*  #error There are no external TS sources defined.*/
    
#endif

TSROUTE_STATUS cnxt_hsdp_init() {

    hsdp_sem = sem_create(1, "HSDP");
    if (hsdp_sem == 0) {
        trace_new(TRACE_LEVEL_3 | TRACE_DPS, "HSDP: ERROR:  Could not create HSDP semaphore\n");
        return TSROUTE_STATUS_ERROR;
    }

    cnxt_hsdp_pktctrl(HSDP_PACKET_DVB, 0xbc, 0x47, 0);
    
#if (HSDP_TYPE != HSDP_COLORADO)
    /* The pawser clock is run at 108 mhz. These values divide 108 to generate
     * the HSDP clock.  We set it for ~8.3 MHz with a ~46% duty cycle.
     */
    CNXT_SET_VAL(HSDP_TSC_GEN_CLK_CNTL, HSDP_CLK_CNTL_CLK_HI_CNT_MASK, 6);
    CNXT_SET_VAL(HSDP_TSC_GEN_CLK_CNTL, HSDP_CLK_CNTL_TOTAL_CLK_CNT_MASK, 13);
    
    CNXT_SET_VAL(HSDP_TSD_GEN_CLK_CNTL, HSDP_CLK_CNTL_CLK_HI_CNT_MASK, 6);
    CNXT_SET_VAL(HSDP_TSD_GEN_CLK_CNTL, HSDP_CLK_CNTL_TOTAL_CLK_CNT_MASK, 13);
#else 
    CNXT_SET_VAL(HSDP_TNF_CLK_CNTL_REG, HSDP_CLK_CNTL_CLK_HI_CNT_MASK, 6);
    CNXT_SET_VAL(HSDP_TNF_CLK_CNTL_REG, HSDP_CLK_CNTL_TOTAL_CLK_CNT_MASK, 13);

#endif


#if (HSDP_TYPE == HSDP_HONDO) || (HSDP_TYPE == HSDP_WABASH)
    CNXT_SET_VAL(HSDP_TS_PKT_CNTL_REG, HSDP_TS_PKT_CNTL_OUTPUT_FIFO_THRESHOLD_MASK, 0);
    CNXT_SET(HSDP_TS_PKT_CNTL_REG, HSDP_TS_PKT_CNTL_ERROR_SIGNAL_FUNC_MASK, 0);
#endif    

#if (HSDP_TYPE == HSDP_BRAZOS)
#if IRD_HAS_EXTERNAL_SAT_DEMOD
/******************************************************************************
 * !!! HACK ALERT !!!
 * NOTE:  The following conditional check is temporary code to allow the modem
 * redirector to work on UART2 when building for BRONCO IRD.
 * The goal is to move all (or most) PIN MUXING and ALT FUNC assignments to 
 * a common place (e.g. STARTUP and/or CODELDR), and to remove these types of
 * settings in the various drivers where possible.
 * The value(s) will be defined in the appropriate hardware config files
 *****************************************************************************/
#if (REDIRECTOR_PORT != INTERNAL_UART2)
/******************************************************************************
 * End of HACK
 *****************************************************************************/
    CNXT_SET(PLL_PIN_ALT_FUNC_REG, PLL_PIN_ALT_FUNC_EXT_DEMOD_MASK, PLL_PIN_ALT_FUNC_EXT_DEMOD_ENABLE);
#endif   /* One final part of the HACK */
#endif
#endif

    return TSROUTE_STATUS_OK;
}


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
TSROUTE_STATUS cnxt_hsdp_pktctrl(HSDP_PACKET_MODE mode, unsigned char pkt_size, unsigned char pkt_sync, int entire_pkt)
{
#if (HSDP_TYPE != HSDP_COLORADO)
    if (mode == HSDP_PACKET_DVB)
    {
        CNXT_SET(HSDP_TS_PKT_CNTL_REG, HSDP_TS_PKT_CNTL_DSS_PKT_MODE_MASK, 0);
    }
    else
    {
        CNXT_SET(HSDP_TS_PKT_CNTL_REG, HSDP_TS_PKT_CNTL_DSS_PKT_MODE_MASK, 1);
    }
#endif

    /* Regardless of the type of input port, we default to 188 byte DVB packets */
    CNXT_SET_VAL(HSDP_TS_PKT_CNTL_REG, HSDP_TS_PKT_CNTL_PKT_SIZE_MASK, pkt_size);
    CNXT_SET_VAL(HSDP_TS_PKT_CNTL_REG, HSDP_TS_PKT_CNTL_SYNC_BYTE_CODE_MASK, pkt_sync);
    
#if (HSDP_TYPE != HSDP_COLORADO)
    if (entire_pkt) {
        CNXT_SET(HSDP_TS_PKT_CNTL_REG,
                     HSDP_TS_PKT_CNTL_ERROR_SIGNAL_FUNC_MASK, 
                     HSDP_TS_PKT_CNTL_HOLD_PKT_ERROR);
    } else {
        CNXT_SET_VAL(HSDP_TS_PKT_CNTL_REG, HSDP_TS_PKT_CNTL_ERROR_SIGNAL_FUNC_MASK, 0);
    }
#endif

    return TSROUTE_STATUS_OK;
}


/************************************************************************
 * creates or deletes a path between a TS source and a TS destination   *
 *                                                                      *
 * params:  HSDP_PORT specifying source and destination for transport   *
 *          stream                                                      *
 *                                                                      *
 * returns: TSROUTE_STATUS_OK, TSROUTE_STATUS_ERROR,                    *
 *          TSROUTE_STATUS_INVALID                                      *
 ************************************************************************/
TSROUTE_STATUS cnxt_hsdp_route(HSDP_PORT src, HSDP_PORT dest, HSDP_PORT_TYPE port_type)  {
#if HSDP_TYPE != HSDP_COLORADO
    int destflag = 0;
#endif                        

    if (sem_get(hsdp_sem, KAL_WAIT_FOREVER) != TSROUTE_STATUS_OK) {
       trace_new(TRACE_LEVEL_3 | TRACE_DPS, "HSDP: ERROR:  Line %d: Could not obtain HSDP semaphore!\n", __LINE__);
       return TSROUTE_STATUS_ERROR;
    }

    switch (is_route_legal(src, dest))  {
        case -1:
            trace_new(TRACE_LEVEL_3 | TRACE_DPS, "HSDP: ERROR:  The requested route (%d to %d) is illegal in this IRD.\n", src, dest);
            sem_put(hsdp_sem);
            return(TSROUTE_STATUS_ERROR);
            break;
            
        case 0: 
            trace_new(TRACE_LEVEL_3 | TRACE_DPS, "HSDP: ERROR:  The requested route is busy.\n");
            sem_put(hsdp_sem);
            return(TSROUTE_STATUS_BUSY);
            break;
        case 1:
           
            switch (dest) {
                case HSDP_NIM0:
#if HSDP_TYPE != HSDP_COLORADO
                case HSDP_NIM1:
#endif
                case HSDP_NIM2: 
                case HSDP_NIM3:
                    /* this is invalid.  We can not route out to a NIM.  should never get here */
                    trace_new(TRACE_LEVEL_3 | TRACE_DPS,
                              "HSDP: ERROR:  The requested dest (%d) is illegal\n", dest);
                    break;

                case HSDP_HSDP0:
                case HSDP_HSDP1:
                    /* need to add this block of code for hsdp0/1 destination routing in order to */
                    /* be able to set ser/par conversion bit and ci or 1394 read/write modes since  */
                    /* 1394 will use hsdp0 as an output port */
                    switch (port_type) {
                        case HSDP_TYPE_SER:
                            CNXT_SET(hsdp_port_data[dest].ctl_address,
                                     HSDP_SER_NIM_CNTL_SER_TO_PAR_CONV_MASK,
                                     HSDP_SER_NIM_CNTL_SER_TO_PAR_CONV_ENABLE);
                            break;

                        case HSDP_TYPE_BIDIR_SER:
                            CNXT_SET(hsdp_port_data[dest].ctl_address,
                                     HSDP_BIDIR_PORT_CNTL_SER_TO_PAR_CONV_MASK,
                                     HSDP_BIDIR_PORT_CNTL_SER_TO_PAR_CONV_ENABLE);
                            CNXT_SET(hsdp_port_data[dest].ctl_address,
                                     HSDP_BIDIR_PORT_CNTL_MODE_MASK,
                                     HSDP_BIDIR_PORT_CNTL_MODE_CI_READ);
                            break;
                        
                        case HSDP_TYPE_BIDIR:
                            CNXT_SET(hsdp_port_data[dest].ctl_address,
                                     HSDP_BIDIR_PORT_CNTL_SER_TO_PAR_CONV_MASK,
                                     HSDP_BIDIR_PORT_CNTL_SER_TO_PAR_CONV_DISABLE);
                            CNXT_SET(hsdp_port_data[dest].ctl_address,
                                     HSDP_BIDIR_PORT_CNTL_MODE_MASK,
                                     HSDP_BIDIR_PORT_CNTL_MODE_CI_READ);
                            break;


/****/
                        case HSDP_TYPE_PAR:
#if (HSDP_TYPE == HSDP_WABASH) || (HSDP_TYPE == HSDP_BRAZOS)
                            CNXT_SET(hsdp_port_data[dest].ctl_address,
                                     HSDP_PAR_NIM_CNTL_INPUT_CLK_POLARITY_MASK,
                                     HSDP_PAR_NIM_CNTL_INPUT_CLK_POLARITY_FALLING_EDGE);
/* Added for Wabash, HSDP1 used to output to POD  */
#if (HSDP_TYPE == HSDP_WABASH)
                            CNXT_SET(hsdp_port_data[dest].ctl_address,
                                     HSDP_BIDIR_PORT_CNTL_MODE_MASK,
                                     HSDP_BIDIR_PORT_CNTL_MODE_CI_WRITE);
#endif                            

#else
                            trace_new(TRACE_DMD | TRACE_LEVEL_3,
                                      "%s: Line %d: Requesting invalid HSDP port...", __FILE__, __LINE__);
#endif                            
                            break;
                            
                        /* add in case for 1394 since will need 1394 mode read/write bits set properly */
                        case HSDP_TYPE_1394:
#if (HSDP_TYPE == HSDP_WABASH) 
                            CNXT_SET(hsdp_port_data[dest].ctl_address,
                                     HSDP_SER_NIM_CNTL_SER_TO_PAR_CONV_MASK,
                                     HSDP_SER_NIM_CNTL_SER_TO_PAR_CONV_ENABLE);

                            CNXT_SET(hsdp_port_data[dest].ctl_address,
                                     HSDP_BIDIR_PORT_CNTL_MODE_MASK,
                                     HSDP_BIDIR_PORT_CNTL_MODE_1394_WRITE);

                            CNXT_SET(hsdp_port_data[dest].ctl_address,
                                     HSDP_BIDIR_PORT_CNTL_CLK_MODE_MASK,
                                     HSDP_BIDIR_PORT_CNTL_CLK_MODE_HSDP_CLK);

                            /* 1394 output should be have rising edge clock polarity */
                            CNXT_SET(hsdp_port_data[dest].ctl_address,
                                     HSDP_PAR_NIM_CNTL_INPUT_CLK_POLARITY_MASK,
                                     HSDP_PAR_NIM_CNTL_INPUT_CLK_POLARITY_RISING_EDGE);
#endif
                            break;
                                     
                        default:
                            /* nothing to do */
                            break;
                    }

                    
                    /* its a port, and we knows its legal so it must
                       be a bidir port*/
                    switch (src) {
                        case HSDP_HSDP0: case HSDP_HSDP1:
                            /* this is invalid.  We can not route hsdp to hsdp. shoud never get here */
                            trace_new(TRACE_LEVEL_3 | TRACE_DPS,
                                      "HSDP: ERROR:  The requested source (%d) is illegal\n", src);
                            break;
                        case HSDP_NIM0:
#if HSDP_TYPE != HSDP_COLORADO
                        case HSDP_NIM1:
#endif
                        case HSDP_NIM2: 
                        case HSDP_NIM3:
                            /* we're selecting the output of another port */
                            CNXT_SET(hsdp_port_data[dest].ctl_address,
                                     HSDP_BIDIR_PORT_CNTL_OUTPUT_DATA_SEL_MASK,
                                     HSDP_BIDIR_PORT_CNTL_OUTPUT_DATA_SEL_NIM);
                            /* the order of the HSDP_PORT enumeration is important
                             * because of the following line.
                             * With the order that they are in, the enumeration
                             * lines up with the SUBSEL register
                             */
                            CNXT_SET(hsdp_port_data[dest].ctl_address,
                                     HSDP_BIDIR_PORT_CNTL_INPUT_CLK_POLARITY_MASK,
                                     HSDP_BIDIR_PORT_CNTL_INPUT_CLK_POLARITY_FALLING_EDGE);
#if HSDP_TYPE != HSDP_COLORADO     /* colorado only has one nim input */
                            CNXT_SET_VAL(hsdp_port_data[dest].ctl_address,
                                         HSDP_BIDIR_PORT_CNTL_TS_WRAP_SUBSEL_MASK, src);

                            if (port_type != HSDP_TYPE_1394 )
                            {
                              CNXT_SET(hsdp_port_data[dest].ctl_address,
                                       HSDP_BIDIR_PORT_CNTL_CLK_MODE_MASK,
                                       HSDP_BIDIR_PORT_CNTL_CLK_MODE_INPUT);
                            }
                            else
                            {
                                /* 1394 output should be have rising edge clock polarity */
                                CNXT_SET(hsdp_port_data[dest].ctl_address,
                                     HSDP_PAR_NIM_CNTL_INPUT_CLK_POLARITY_MASK,
                                     HSDP_PAR_NIM_CNTL_INPUT_CLK_POLARITY_RISING_EDGE);

                            }

                            CNXT_SET(hsdp_port_data[dest].ctl_address,
                                     HSDP_BIDIR_PORT_CNTL_OUTPUT_MODES_ENABLE_MASK,
                                     HSDP_BIDIR_PORT_CNTL_OUTPUT_MODES_ENABLE);
#endif                            
                            break;
                        case HSDP_DEMUX0:
#if HSDP_TYPE != HSDP_COLORADO
                        case HSDP_DEMUX1:
                        case HSDP_DEMUX2:
#endif
                            /* When routing out of a demux on brazos, it would seem that
                             * we need to clock it.  On Colorado and WaBASH the pawser
                             * can clock the HSDP port, but we should be able to "generate"
                             * a clock seperate from the pawser as long as it is as fast
                             * as the fastest data rate the port will have to handle.  Since
                             * we can get away with a fixed clock rate and since the use of
                             * the generated clock is optional, it will be set up at init and
                             * left alone unless the user explicitly calls set_clock_params
                             * 
                             *
                             */
#if HSDP_TYPE != HSDP_COLORADO
                            if (src == HSDP_DEMUX0) {
                                destflag = HSDP_BIDIR_PORT_CNTL_OUTPUT_DATA_SUBSEL_PARSER0;
                            }
                            if (src == HSDP_DEMUX1) {
                                destflag = HSDP_BIDIR_PORT_CNTL_OUTPUT_DATA_SUBSEL_PARSER2;
                            }
                            if (src == HSDP_DEMUX2) {
                                destflag = HSDP_BIDIR_PORT_CNTL_OUTPUT_DATA_SUBSEL_PARSER3;
                            }
#endif
                            CNXT_SET(hsdp_port_data[dest].ctl_address,
                                     HSDP_BIDIR_PORT_CNTL_OUTPUT_DATA_SEL_MASK,
                                     HSDP_BIDIR_PORT_CNTL_OUTPUT_DATA_SEL_DEMUX);
#if HSDP_TYPE != HSDP_COLORADO
                            CNXT_SET(hsdp_port_data[dest].ctl_address,
                                     HSDP_BIDIR_PORT_CNTL_OUTPUT_DATA_SUBSEL_MASK,
                                     destflag);
#endif                            
            
                            if (port_type != HSDP_TYPE_1394)
                            {
                                CNXT_SET(hsdp_port_data[dest].ctl_address,
                                         HSDP_BIDIR_PORT_CNTL_MODE_MASK,
                                         HSDP_BIDIR_PORT_CNTL_MODE_CI_WRITE);
                                CNXT_SET(hsdp_port_data[dest].ctl_address,
                                         HSDP_BIDIR_PORT_CNTL_INPUT_CLK_POLARITY_MASK,
                                         HSDP_BIDIR_PORT_CNTL_INPUT_CLK_POLARITY_FALLING_EDGE);

#if (HSDP_TYPE != HSDP_COLORADO)
                            CNXT_SET(hsdp_port_data[dest].ctl_address,
                                     HSDP_BIDIR_PORT_CNTL_OUTPUT_MODES_ENABLE_MASK,
                                     HSDP_BIDIR_PORT_CNTL_OUTPUT_MODES_ENABLE);
                            CNXT_SET(hsdp_port_data[dest].ctl_address,
                                     HSDP_BIDIR_PORT_CNTL_CLK_MODE_MASK,
                                     HSDP_BIDIR_PORT_CNTL_CLK_MODE_GEN_CLK);
                            
#endif
                            }
                            else
                            {
                                CNXT_SET(hsdp_port_data[dest].ctl_address,
                                         HSDP_BIDIR_PORT_CNTL_MODE_MASK,
                                         HSDP_BIDIR_PORT_CNTL_MODE_1394_WRITE);
                                CNXT_SET(hsdp_port_data[dest].ctl_address,
                                         HSDP_BIDIR_PORT_CNTL_INPUT_CLK_POLARITY_MASK,
                                         HSDP_BIDIR_PORT_CNTL_INPUT_CLK_POLARITY_RISING_EDGE);

#if (HSDP_TYPE != HSDP_COLORADO)
                                CNXT_SET(hsdp_port_data[dest].ctl_address,
                                         HSDP_BIDIR_PORT_CNTL_OUTPUT_MODES_ENABLE_MASK,
                                         HSDP_BIDIR_PORT_CNTL_OUTPUT_MODES_ENABLE);
#endif
                            }
                            break;

                        default:
                            trace_new(TRACE_DMD | TRACE_LEVEL_3,
                                      "%s: Line %d: Requesting invalid HSDP port...", __FILE__, __LINE__);
                            break;

/*#endif*/
                    }
                    break;
/* Still case dest !!*/
                case HSDP_DEMUX0:
#if (HSDP_TYPE != HSDP_COLORADO)
                    if ( (src == HSDP_HSDP0) || (src == HSDP_HSDP1) ) {
                        /* reset the output modes enable bit */
                        CNXT_SET(hsdp_port_data[src].ctl_address,
                                 HSDP_BIDIR_PORT_CNTL_OUTPUT_MODES_ENABLE_MASK,
                                 0);
#else
                    if ( src == HSDP_HSDP0 ) {
#endif
                        /* make sure the HSDP port is an input */
                        CNXT_SET(hsdp_port_data[src].ctl_address,
                                 HSDP_BIDIR_PORT_CNTL_MODE_MASK,
                                 HSDP_BIDIR_PORT_CNTL_MODE_CI_READ);
                        /* make sure the clock polarity is correct for input */
                        CNXT_SET(hsdp_port_data[src].ctl_address,
                                 HSDP_BIDIR_PORT_CNTL_INPUT_CLK_POLARITY_MASK,
                                 HSDP_BIDIR_PORT_CNTL_INPUT_CLK_POLARITY_RISING_EDGE);
                    }

                    switch (port_type) {
                        case HSDP_TYPE_SER:
#if (TS_MUX_TYPE == TS_MUX_TYPE_BRONCO) && (DIGITAL_DISPLAY == PW113_DISPLAY)
									 if(src == HSDP_NIM1)
									 {	  
									     CNXT_SET(HSDP_SP_INPUT_CNTL_REG,
			                                     HSDP_SP_INPUT_CNTL_PARSER0_INPUT_SEL_MASK,
			                                     HSDP_SP_INPUT_CNTL_PARSER0_SER_NIM_PORT_0);
										  CNXT_SET(PLL_PIN_ALT_FUNC_REG,
			                                     PLL_PIN_ALT_FUNC_EXT_DEMOD_HSDP_MASK,
			                                     PLL_PIN_ALT_FUNC_EXT_DEMOD_HSDP_MASK);
										  CNXT_SET(HSDP_TSA_PORT_CNTL_REG,
			                                     HSDP_SER_NIM_CNTL_SER_TO_PAR_CONV_MASK,
			                                     HSDP_SER_NIM_CNTL_SER_TO_PAR_CONV_ENABLE);										  
									 }
									 else
#endif
									 {
                                CNXT_SET(hsdp_port_data[src].ctl_address,
                                     HSDP_SER_NIM_CNTL_SER_TO_PAR_CONV_MASK,
                                     HSDP_SER_NIM_CNTL_SER_TO_PAR_CONV_ENABLE);
									 }
                            break;

#if HSDP_TYPE != HSDP_COLORADO
                        case HSDP_TYPE_BIDIR_SER:
                            CNXT_SET(hsdp_port_data[src].ctl_address,
                                     HSDP_BIDIR_PORT_CNTL_SER_TO_PAR_CONV_MASK,
                                     HSDP_BIDIR_PORT_CNTL_SER_TO_PAR_CONV_ENABLE);
                            CNXT_SET(hsdp_port_data[src].ctl_address,
                                     HSDP_BIDIR_PORT_CNTL_MODE_MASK,
                                     HSDP_BIDIR_PORT_CNTL_MODE_CI_READ);
                            break;
#endif
                        
                        case HSDP_TYPE_BIDIR:
                            CNXT_SET(hsdp_port_data[src].ctl_address,
                                     HSDP_BIDIR_PORT_CNTL_SER_TO_PAR_CONV_MASK,
                                     HSDP_BIDIR_PORT_CNTL_SER_TO_PAR_CONV_DISABLE);
                            CNXT_SET(hsdp_port_data[src].ctl_address,
                                     HSDP_BIDIR_PORT_CNTL_MODE_MASK,
                                     HSDP_BIDIR_PORT_CNTL_MODE_CI_READ);
                            break;

                        case HSDP_TYPE_PAR:
#if HSDP_TYPE == HSDP_WABASH		   /* video demod is on falling edge while POD input is on rising edge */
                            if (src != HSDP_HSDP0)
                            {
                              CNXT_SET(hsdp_port_data[src].ctl_address,
                                     HSDP_PAR_NIM_CNTL_INPUT_CLK_POLARITY_MASK,
                                     HSDP_PAR_NIM_CNTL_INPUT_CLK_POLARITY_FALLING_EDGE);
							}
#endif                            
                            break;         

                        default:
                            trace_new(TRACE_DMD | TRACE_LEVEL_3,
                                      "%s: Line %d: Requesting invalid HSDP port...", __FILE__, __LINE__);
                            /* nothing to do */
                            break;
                    }
                    /* the order of the HSDP_PORT enumeration is important because of the following line.
                       With the order that they are in, the enumeration lines up with the SUBSEL register */
#if ((TS_MUX_TYPE == TS_MUX_TYPE_BRONCO) && (DIGITAL_DISPLAY == PW113_DISPLAY))
						  /* Only check this if we are running on the crosby platform. */
						  /* if the source is nim1 and serial we have already set this register. */
                    if(!((src == HSDP_NIM1) && (port_type == HSDP_TYPE_SER)))
#endif
						  {
                        CNXT_SET_VAL(HSDP_SP_INPUT_CNTL_REG,
                                 HSDP_SP_INPUT_CNTL_PARSER0_INPUT_SEL_MASK, src);
						  }

                    break;
                    
#if HSDP_TYPE != HSDP_COLORADO
                case HSDP_DEMUX1:
                    if ( (src == HSDP_HSDP0) || (src == HSDP_HSDP1) ) {
                        /* reset the output modes enable bit */
                        CNXT_SET(hsdp_port_data[src].ctl_address,
                                 HSDP_BIDIR_PORT_CNTL_OUTPUT_MODES_ENABLE_MASK,
                                 0);
                        /* make sure the HSDP port is an input */
                        CNXT_SET(hsdp_port_data[src].ctl_address,
                                 HSDP_BIDIR_PORT_CNTL_MODE_MASK,
                                 HSDP_BIDIR_PORT_CNTL_MODE_CI_READ);
                        /* make sure the clock polarity is correct for input */
                        CNXT_SET(hsdp_port_data[src].ctl_address,
                                 HSDP_BIDIR_PORT_CNTL_INPUT_CLK_POLARITY_MASK,
                                 HSDP_BIDIR_PORT_CNTL_INPUT_CLK_POLARITY_RISING_EDGE);
                    }
                    
                    switch (port_type) {
                        case HSDP_TYPE_SER:
                            CNXT_SET(hsdp_port_data[src].ctl_address,
                                     HSDP_SER_NIM_CNTL_SER_TO_PAR_CONV_MASK,
                                     HSDP_SER_NIM_CNTL_SER_TO_PAR_CONV_ENABLE);
                            break;

                        case HSDP_TYPE_BIDIR_SER:
                            CNXT_SET(hsdp_port_data[src].ctl_address,
                                     HSDP_BIDIR_PORT_CNTL_SER_TO_PAR_CONV_MASK,
                                     HSDP_BIDIR_PORT_CNTL_SER_TO_PAR_CONV_ENABLE);
                            CNXT_SET(hsdp_port_data[src].ctl_address,
                                     HSDP_BIDIR_PORT_CNTL_MODE_MASK,
                                     HSDP_BIDIR_PORT_CNTL_MODE_CI_READ);
                            break;
                        
                        case HSDP_TYPE_BIDIR:
                            CNXT_SET(hsdp_port_data[src].ctl_address,
                                     HSDP_BIDIR_PORT_CNTL_SER_TO_PAR_CONV_MASK,
                                     HSDP_BIDIR_PORT_CNTL_SER_TO_PAR_CONV_DISABLE);
                            CNXT_SET(hsdp_port_data[src].ctl_address,
                                     HSDP_BIDIR_PORT_CNTL_MODE_MASK,
                                     HSDP_BIDIR_PORT_CNTL_MODE_CI_READ);
                            break;


/****/
                        case HSDP_TYPE_PAR:
#if HSDP_TYPE == HSDP_WABASH
                            CNXT_SET(hsdp_port_data[src].ctl_address,
                                     HSDP_PAR_NIM_CNTL_INPUT_CLK_POLARITY_MASK,
                                     HSDP_PAR_NIM_CNTL_INPUT_CLK_POLARITY_FALLING_EDGE);
#endif                            
                            break;         

                        default:
                            trace_new(TRACE_DMD | TRACE_LEVEL_3,
                                      "%s: Line %d: Requesting invalid HSDP port...", __FILE__, __LINE__);
                            /* nothing to do */
                            break;
                    }
                    /* the order of the HSDP_PORT enumeration is important because of the following line.
                       With the order that they are in, the enumeration lines up with the SUBSEL register */
                    CNXT_SET_VAL(HSDP_SP_INPUT_CNTL_REG,
                                 HSDP_SP_INPUT_CNTL_PARSER2_INPUT_SEL_MASK, src);
                    break;
                    
                case HSDP_DEMUX2:
                    if ( (src == HSDP_HSDP0) || (src == HSDP_HSDP1) ) {
                        /* reset the output modes enable bit */
                        CNXT_SET(hsdp_port_data[src].ctl_address,
                                 HSDP_BIDIR_PORT_CNTL_OUTPUT_MODES_ENABLE_MASK,
                                 0);
                        /* make sure the HSDP port is an input */
                        CNXT_SET(hsdp_port_data[src].ctl_address,
                                 HSDP_BIDIR_PORT_CNTL_MODE_MASK,
                                 HSDP_BIDIR_PORT_CNTL_MODE_CI_READ);
                        /* make sure the clock polarity is correct for input */
                        CNXT_SET(hsdp_port_data[src].ctl_address,
                                 HSDP_BIDIR_PORT_CNTL_INPUT_CLK_POLARITY_MASK,
                                 HSDP_BIDIR_PORT_CNTL_INPUT_CLK_POLARITY_RISING_EDGE);
                    }
                    
                    switch (port_type) {
                        case HSDP_TYPE_SER:
                            CNXT_SET(hsdp_port_data[src].ctl_address,
                                     HSDP_SER_NIM_CNTL_SER_TO_PAR_CONV_MASK,
                                     HSDP_SER_NIM_CNTL_SER_TO_PAR_CONV_ENABLE);
                            break;

                        case HSDP_TYPE_BIDIR_SER:
                            CNXT_SET(hsdp_port_data[src].ctl_address,
                                     HSDP_BIDIR_PORT_CNTL_SER_TO_PAR_CONV_MASK,
                                     HSDP_BIDIR_PORT_CNTL_SER_TO_PAR_CONV_ENABLE);
                            CNXT_SET(hsdp_port_data[src].ctl_address,
                                     HSDP_BIDIR_PORT_CNTL_MODE_MASK,
                                     HSDP_BIDIR_PORT_CNTL_MODE_CI_READ);
                            break;
                        
                        case HSDP_TYPE_BIDIR:
                            CNXT_SET(hsdp_port_data[src].ctl_address,
                                     HSDP_BIDIR_PORT_CNTL_SER_TO_PAR_CONV_MASK,
                                     HSDP_BIDIR_PORT_CNTL_SER_TO_PAR_CONV_DISABLE);
                            CNXT_SET(hsdp_port_data[src].ctl_address,
                                     HSDP_BIDIR_PORT_CNTL_MODE_MASK,
                                     HSDP_BIDIR_PORT_CNTL_MODE_CI_READ);
                            break;

                       case HSDP_TYPE_PAR:
#if HSDP_TYPE == HSDP_WABASH
                            CNXT_SET(hsdp_port_data[src].ctl_address,
                                     HSDP_PAR_NIM_CNTL_INPUT_CLK_POLARITY_MASK,
                                     HSDP_PAR_NIM_CNTL_INPUT_CLK_POLARITY_FALLING_EDGE);
#endif                            
                            break;         

                        default:
                            trace_new(TRACE_DMD | TRACE_LEVEL_3,
                                      "%s: Line %d: Requesting invalid HSDP port...", __FILE__, __LINE__);
                            /* nothing to do */
                            break;
                    }
                    /* the order of the HSDP_PORT enumeration is important because of the following line.
                       With the order that they are in, the enumeration lines up with the SUBSEL register */
                    CNXT_SET_VAL(HSDP_SP_INPUT_CNTL_REG,
                                 HSDP_SP_INPUT_CNTL_PARSER3_INPUT_SEL_MASK, src);
                    break;
#endif
                default:
                    trace_new(TRACE_LEVEL_ALWAYS | TRACE_DPS, "HSDP: ERROR:  File %s, Line %d: Shouldn't ever get here!\n", __FILE__, __LINE__);
                    sem_put(hsdp_sem);
                    return(TSROUTE_STATUS_ERROR);
            }					      			       
            sem_put(hsdp_sem);
            return TSROUTE_STATUS_OK;
            break;
        default:
            trace_new(TRACE_LEVEL_ALWAYS | TRACE_DPS, "HSDP: ERROR:  File %s, Line %d: Shouldn't ever get here!\n", __FILE__, __LINE__);
            sem_put(hsdp_sem);
            return(TSROUTE_STATUS_ERROR);
            break;
    }
}

/************************************************************************
 * creates or deletes a path between a TS source and a TS destination   *
 *                                                                      *
 * params:  HSDP_PORT specifying source and destination for transport   *
 *          stream                                                      *
 *                                                                      *
 * returns: TSROUTE_STATUS_OK, TSROUTE_STATUS_ERROR,                    *
 *          TSROUTE_STATUS_INVALID                                      *
 ************************************************************************/
#if 0
TSROUTE_STATUS cnxt_hsdp_unroute(HSDP_PORT src, HSDP_PORT dest)  {
    /* This function is place holder for manipulating data structures that
     * would contain state information
     */
    return(TSROUTE_STATUS_OK);
}
#endif

/************************************************************************
 * enables and configures error signaling for specified HSDP            *
 *                                                                      *
 * params:  port - which port to configure                              *
 *          ignore - use or ignore fail signal                          *
 *          polarity - set signal active high or active low for error   *
 *                                                                      *
 * returns: TSROUTE_STATUS_OK, TSROUTE_STATUS_ERROR                     *
 ************************************************************************/
TSROUTE_STATUS cnxt_hsdp_set_error_params(HSDP_PORT     port, 
                                          int           ignore, 
                                          HSDP_POLARITY polarity)  
{

   u_int32        uRegMsk;
   u_int32        uRegVal;

    /* for serial, hsdp, and parallel ports the error fail and error polarity
     * bits are in the same location (for colorado, wabash, and brazos)
     */
    uRegMsk = HSDP_SER_NIM_CNTL_ERROR_SIGNAL_POLARITY_MASK;
    switch (polarity) {
        case HSDP_POLARITY_NEGATIVE:
            uRegVal = HSDP_SER_NIM_CNTL_ERROR_SIGNAL_POLARITY_ACTIVE_LOW;
            break;
        case HSDP_POLARITY_POSITIVE:
            uRegVal = HSDP_SER_NIM_CNTL_ERROR_SIGNAL_POLARITY_ACTIVE_HIGH;
            break;
        default:
            return TSROUTE_STATUS_INVALID;
    }

#if (HSDP_TYPE != HSDP_COLORADO)
    if (ignore) {
        uRegMsk |= (HSDP_SER_NIM_CNTL_IGNORE_FAIL_MASK);
        uRegVal |= (HSDP_SER_NIM_CNTL_IGNORE_FAIL);        
    } else {
        uRegMsk |= (HSDP_SER_NIM_CNTL_IGNORE_FAIL_MASK);
        uRegVal |= (HSDP_SER_NIM_CNTL_USE_FAIL);
    }
#endif

    CNXT_SET(hsdp_port_data[port].ctl_address, uRegMsk, uRegVal);

    return TSROUTE_STATUS_OK;
}


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
TSROUTE_STATUS cnxt_hsdp_set_clock_params (HSDP_PORT        port,
                                           HSDP_CLOCK_TYPE  clk_type,
                                           HSDP_POLARITY    clk_pol,
                                           int              freq,
                                           int              duty)
{
   /* for serial, hsdp, and parallel ports the clock
    * bits are in the same location (for colorado, wabash, and brazos)
    */
    switch (clk_pol) {
        case HSDP_POLARITY_POSITIVE:
            CNXT_SET(hsdp_port_data[port].ctl_address, 
                 HSDP_SER_NIM_CNTL_INPUT_CLK_POLARITY_MASK,
                 HSDP_SER_NIM_CNTL_INPUT_CLK_POLARITY_RISING_EDGE);
            break;
        case HSDP_POLARITY_NEGATIVE:
            CNXT_SET(hsdp_port_data[port].ctl_address, 
                 HSDP_SER_NIM_CNTL_INPUT_CLK_POLARITY_MASK,
                 HSDP_SER_NIM_CNTL_INPUT_CLK_POLARITY_FALLING_EDGE);
            break;
        default:
            return TSROUTE_STATUS_INVALID;
    }
   

#if (HSDP_TYPE != HSDP_COLORADO)
   
   if ((port == HSDP_HSDP0) || (port == HSDP_HSDP1)) 
   {
       LPREG          clkreg;
       u_int32        uClkMsk;
       u_int32        uClkVal;

      switch (clk_type)
      {
         case HSDP_CLOCK_GENERATE:                      
            if (duty==0) 
            {
                duty = 1;
            }

            clkreg = (LPREG)( 
                       (unsigned int)hsdp_port_data[port].ctl_address +
                       (unsigned int)0x50
                            );

            uClkMsk = HSDP_CLK_CNTL_TOTAL_CLK_CNT_MASK |
                      HSDP_CLK_CNTL_CLK_HI_CNT_MASK;
            uClkVal = (   ( (108000000L/freq) & 0xff )
                               << HSDP_CLK_CNTL_TOTAL_CLK_CNT_SHIFT  )   |
                      (   ( (duty*1080000L/freq) & 0xff )
                               << HSDP_CLK_CNTL_CLK_HI_CNT_SHIFT   );

            CNXT_SET(clkreg, uClkMsk, uClkVal);
            
            CNXT_SET(hsdp_port_data[port].ctl_address, 
                     HSDP_BIDIR_PORT_CNTL_CLK_MODE_MASK,
                     HSDP_BIDIR_PORT_CNTL_CLK_MODE_GEN_CLK);
            break;
              
         case HSDP_CLOCK_SERIAL:
            CNXT_SET(hsdp_port_data[port].ctl_address, 
                     HSDP_BIDIR_PORT_CNTL_CLK_MODE_MASK,
                     HSDP_BIDIR_PORT_CNTL_CLK_MODE_HSDP_CLK);
                
                break;

         case HSDP_CLOCK_INPUT:
             CNXT_SET(hsdp_port_data[port].ctl_address, 
                      HSDP_BIDIR_PORT_CNTL_CLK_MODE_MASK,
                      HSDP_BIDIR_PORT_CNTL_CLK_MODE_INPUT);

         default:
             return TSROUTE_STATUS_INVALID;
      }



      /* reset HSDPO[01] contorl logic */
      CNXT_SET((hsdp_port_data[port].ctl_address), 
               HSDP_BIDIR_PORT_CNTL_OUTPUT_MODES_ENABLE_MASK,
               0);

      CNXT_SET((hsdp_port_data[port].ctl_address), 
               HSDP_BIDIR_PORT_CNTL_OUTPUT_MODES_ENABLE_MASK,
               HSDP_BIDIR_PORT_CNTL_OUTPUT_MODES_ENABLE);
    
   }

#endif

   return TSROUTE_STATUS_OK;
}

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
TSROUTE_STATUS cnxt_hsdp_get_error(int *error)  {
    *error = CNXT_GET_VAL(HSDP_TS_ERR_STATUS_REG, 0xFFFFFFFF);
    return(TSROUTE_STATUS_OK);
}



/****************************************************************************
 * Modifications:
 * $Log: 
 *  43   mpeg      1.42        3/16/04 10:20:23 AM    Miles Bintz     CR(s) 
 *        8567 : fixed warnings
 *  42   mpeg      1.41        1/29/04 12:02:25 PM    Miles Bintz     CR(s) 
 *        8305 : added new line to end of character
 *  41   mpeg      1.40        11/25/03 8:38:05 AM    Ian Mitchell    CR(s): 
 *        7739 7982 Configure differently if the software asks for nim 1 in 
 *        serial to be connected to demux 0 in the crosby playform. It now 
 *        routes ED0 to demux 0.
 *  40   mpeg      1.39        9/30/03 1:28:32 AM     Sahil Bansal    SCR(s): 
 *        7584 
 *        Added support to demux0 as source in hsdp_route_output() for 1394 tx.
 *          Needed to make sure various bits in hsdp0 register were being set 
 *        correctly for 1394 hsdp0 as dest and demux0 as source.
 *        
 *  39   mpeg      1.38        9/29/03 5:04:40 PM     Sahil Bansal    SCR(s): 
 *        7578 
 *        Changed hsdp route function so that in cases for 1394, the clock 
 *        polarity bit is set to 0 (rising edge) on the hsdp0 control register.
 *        
 *  38   mpeg      1.37        9/24/03 9:15:48 PM     Moshe Yehushua  SCR(s): 
 *        7544 
 *        The source for Demux0 is now clocked on the rising edge if it is 
 *        HSDP0 (i.e. POD) for Wabash
 *        
 *  37   mpeg      1.36        9/24/03 6:50:24 PM     Moshe Yehushua  SCR(s): 
 *        7542 
 *        set the Port Mode to 1 (output) in hsdp_route()
 *        
 *  36   mpeg      1.35        9/23/03 3:07:10 PM     Billy Jackman   SCR(s) 
 *        6765 7499 7505 :
 *        Modified code to not set the clock polarity specifically to falling 
 *        edge for
 *        any Brazos port of type HSDP_TYPE_PAR. Setting it that way was 
 *        keeping OTV
 *        baseband input from working; it needs to be rising edge.
 *        Moved around diagnostic messages so they do not incorrectly print for
 *         cases
 *        where there is no error.
 *        
 *  35   mpeg      1.34        8/4/03 9:32:04 AM      Billy Jackman   SCR(s) 
 *        7080 7121 :
 *        Modified code to modify HSDP port mode and clock polarity to input 
 *        settings
 *        when the HSDP port is used as an input.  This allows setting the port
 *         as an
 *        input even after it has been set to output by previous routings.
 *        
 *  34   mpeg      1.33        7/31/03 2:39:52 PM     Sahil Bansal    SCR(s): 
 *        7094 
 *        Needed to guard code added for 1394 with #if (HSDP_TYPE == 
 *        HSDP_WABASH) since this code used defines like 
 *        HSDP_BIDIR_PORT_CNTL_MODE_MASK which are not valid for other 
 *        configurations like Klondike,etc.
 *        
 *  33   mpeg      1.32        7/30/03 2:02:52 PM     Sahil Bansal    SCR(s): 
 *        6675 
 *        Changed code to support 1394 ts routing outputted to HSDP0
 *        
 *  32   mpeg      1.31        7/23/03 2:09:02 PM     Miles Bintz     SCR(s) 
 *        7027 :
 *        Added the output mode enable bit
 *        
 *  31   mpeg      1.30        7/16/03 2:45:28 PM     Miles Bintz     SCR(s) 
 *        6933 6932 :
 *        moved code around to work around ADS compiler bug
 *        
 *  30   mpeg      1.29        7/14/03 7:26:58 PM     Miles Bintz     SCR(s) 
 *        6933 6932 :
 *        fixed build break
 *        
 *  29   mpeg      1.28        7/14/03 5:36:52 PM     Miles Bintz     SCR(s) 
 *        6933 6932 :
 *        Forced clock to be generated for DVB output when source is pawser.  
 *        When source is NIM, the clock is input from the nim
 *        
 *        
 *  28   mpeg      1.27        7/2/03 2:11:08 PM      Craig Dry       SCR(s) 
 *        6870 :
 *        For Bronco 1, the Aux Baseband port was being identified as Parallel.
 *          It is now properly identified as Serial.
 *        
 *  27   mpeg      1.26        6/4/03 5:45:30 PM      Billy Jackman   SCR(s) 
 *        6720 6721 :
 *        Change CNXT_SET_VAL to CNXT_SET to make using the chip header value 
 *        definition
 *        correct.
 *        
 *  26   mpeg      1.25        5/20/03 2:10:40 PM     Miles Bintz     SCR(s) 
 *        6470 6471 :
 *        modified error_param function, modified clk_param function, modified 
 *        hsdp_route function
 *        
 *  25   mpeg      1.24        5/20/03 1:36:00 PM     Miles Bintz     SCR(s) 
 *        6470 6471 :
 *        modified error_param function, modified clk_param function, modified 
 *        hsdp_route function
 *        
 *  24   mpeg      1.23        5/19/03 9:29:20 PM     Hang Chan       SCR(s): 
 *        6466 6467 
 *        Modifiy cnxt_hsdp_set_error_param and cnxt_hsdp_set_clock_param so 
 *        that it correctly program the register.
 *        
 *        
 *  23   mpeg      1.22        5/15/03 2:55:00 PM     Miles Bintz     SCR(s) 
 *        6364 6365 :
 *        wrapped access to registers/bits which colorado doesn't have with a 
 *        HSDP_TYPE != COLORADO
 *        
 *  22   mpeg      1.21        5/14/03 9:43:36 PM     Miles Bintz     SCR(s) 
 *        6353 6354 :
 *        rework of HSDP driver
 *        
 *  21   mpeg      1.20        5/2/03 4:59:24 PM      Craig Dry       SCR(s) 
 *        5521 :
 *        Conditionally remove accesses to Alt Func registers.
 *        
 *  20   mpeg      1.19        4/1/03 2:04:20 PM      Tim White       SCR(s) 
 *        5925 :
 *        Added runtime support for both Bronco1 & Bronco3 boards with one 
 *        hwconfig file.  ALL CHANGES MADE WITH THIS DEFECT ARE HACKS.  Please 
 *        remove when support for Bronco1 is no longer required.
 *        
 *        
 *  19   mpeg      1.18        3/13/03 12:10:50 PM    Miles Bintz     SCR(s) 
 *        5753 5754 :
 *        Modified to support Bronco Rev 3 which uses parallel zephyr header 
 *        instead of serial zephyr header
 *        
 *        
 *  18   mpeg      1.17        2/19/03 11:32:06 AM    Bobby Bradford  SCR(s) 
 *        5544 :
 *        Added a "hack" to allow the redirector driver (for softmodem)
 *        to work with BRONCOMDM build.  This "hack" will be removed when
 *        the pin muxing and alt func settings are consolidated into a
 *        single driver/location.
 *        
 *  17   mpeg      1.16        1/28/03 11:27:18 AM    Billy Jackman   SCR(s) 
 *        5336 :
 *        Use NIM2 as the internal demod in parallel mode.
 *        
 *  16   mpeg      1.15        1/15/03 2:21:32 PM     Billy Jackman   SCR(s) 
 *        5249 :
 *        Added a semicolon to correct a compiler error in code conditionally
 *        compiled for Brazos.
 *        
 *  15   mpeg      1.14        12/5/02 2:18:26 PM     Miles Bintz     SCR(s) 
 *        5074 :
 *        Added structures to support the brazos chip
 *        
 *        
 *  14   mpeg      1.13        9/25/02 11:31:22 AM    Lucy C Allevato SCR(s) 
 *        4650 :
 *        Fixed some VxWorks compiler warnings
 *        
 *  13   mpeg      1.12        7/17/02 12:47:02 PM    Miles Bintz     SCR(s) 
 *        4213 :
 *        Added a default case to get rid of a warning
 *        
 *        
 *  12   mpeg      1.11        7/12/02 4:13:24 PM     Miles Bintz     SCR(s) 
 *        4186 :
 *        When output too HSDP1 from demux 0, the port mode need to be set to 
 *        CI write.
 *        
 *  11   mpeg      1.10        6/14/02 11:16:24 AM    Miles Bintz     SCR(s) 
 *        4001 :
 *        Colorado doesn't have PARALLEL NIM inputs.  Those cases needed to be 
 *        #if'd
 *        out
 *        
 *  10   mpeg      1.9         6/13/02 4:32:06 PM     Miles Bintz     SCR(s) 
 *        4001 :
 *        Added parallel NIM case
 *        
 *        
 *  9    mpeg      1.8         5/14/02 1:12:36 PM     Tim White       SCR(s) 
 *        3775 :
 *        Add support for Serial NIM based Klondike IRD's.
 *        
 *        
 *  8    mpeg      1.7         5/13/02 6:08:58 PM     Tim White       SCR(s) 
 *        3768 :
 *        Add support for the SERIAL_NIM input ports found on the Abilene IRD.
 *        
 *        
 *  7    mpeg      1.6         5/6/02 5:59:02 PM      Tim White       SCR(s) 
 *        3714 :
 *        Update HSDP_ macro definitions to not use CNXT_REG_SET & CNXT_REG_GET
 *         etc...
 *        
 *        
 *  6    mpeg      1.5         4/30/02 3:17:24 PM     Billy Jackman   SCR(s) 
 *        3660 :
 *        Fixed a logic error that also caused a compiler warning.
 *        
 *  5    mpeg      1.4         3/12/02 4:56:02 PM     Bob Van Gulick  SCR(s) 
 *        3359 :
 *        Add support for demux's 2 & 3
 *        
 *        
 *  4    mpeg      1.3         1/30/02 2:06:38 PM     Miles Bintz     SCR(s) 
 *        3058 :
 *        Development/interim checkin
 *        
 *        
 *  3    mpeg      1.2         1/21/02 2:44:26 PM     Miles Bintz     SCR(s) 
 *        3058 :
 *        api tweaks and document updates
 *        
 *        
 *  2    mpeg      1.1         1/18/02 1:34:08 PM     Miles Bintz     SCR(s) 
 *        3058 :
 *        Beta checkin
 *        
 *        
 *  1    mpeg      1.0         1/8/02 5:41:56 PM      Miles Bintz     
 * $
 *
 ****************************************************************************/


