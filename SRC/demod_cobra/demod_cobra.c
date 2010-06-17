/****************************************************************************/ 
/*                   CONEXANT PROPRIETARY AND CONFIDENTIAL                  */
/*                     Conexant Systems Inc. (c) 2002                       */
/*                              Austin, TX                                  */
/*                         All Rights Reserved                              */
/****************************************************************************/
/*
 * Filename: demod_cobra.c
 *
 * Description: This file contains the implementation of the driver module
 *              for the Cobra family of demod chips (CX24121 and CX24130).
 *              This driver also works for the Camaro/Camaric family.
 *
 * Author: Billy Jackman
 *
 ****************************************************************************/
/* $Id: demod_cobra.c,v 1.36, 2004-07-01 08:52:14Z, Steven Shen$
 ****************************************************************************/

#include "stbcfg.h"
#include "basetype.h"
#include "iic.h"
#include "trace.h"
#include "kal.h"
#include "retcodes.h"
#undef BYTE
#include "demod_module_api.h"
#include "cobra.h"
#include "lnb.h"

/* This driver works for both the CX24121 (one unit) and CX24130 (two units).
   If you add support for another Cobra chip with more units, you must check
   for arrays declared with this extent and given initial values.  You must
   assign the appropriate initial values for added units. */
#define MAXIMUM_NUMBER_UNITS (2)

/* These defines are for local state values. */
#define UNINITIALIZED (0)
#define DISCONNECTED (1)
#define CONNECTING (2)
#define CONNECTED (3)
#define SCANNING (4)
#define FADE (5)

/* These defines are for internal commands. */
#define COBRA_CONNECT (1)
#define COBRA_DISCONNECT (2)
#define COBRA_SCAN (3)
#define COBRA_TIMEOUT (4)
#define COBRA_INTERRUPT (5)

/*
 * This global varibale is used to identify the current using NIM,
 * It will be only used by the satellite scan-sky module.
 */
NIM            currentNIM;

/*
 * Add skyscan_flag to supporting the satellite scan-sky function,
 * The default value of skyscan_flag is 0. If it is set to 1, the 
 * cobra_task will do nothing, and the front end is controlled by 
 * the satellite scan-sky module.
 */
static int     skyscan_flag = 0;


/* These definitions are related to the Cobra task message queue. */
#define MAX_COBRA_MSGS (16)
static queue_id_t cobra_message_q = (queue_id_t)0;

/* These definitions are related to interrupt operation. */
#define COBRA_NO_INTERRUPTS (0)
#define COBRA_CONNECTING_INTERRUPTS (INTR_ACQ_SYNC)
#define COBRA_SCANNING_INTERRUPTS (INTR_ACQ_SYNC|INTR_ACQ_FAILURE)
#define COBRA_CONNECTED_INTERRUPTS (INTR_ACQ_FAILURE|INTR_DEMOD_LOSS) /* 2004-11-8 14:32 Derek modified it for miss signal lost in low symbol rate TP*/ 
#define COBRA_NIM_INT INT_GPIO69
#define COBRA_GPIO_INTERRUPT 69
#define COBRA_INTERRUPT_BANK   (COBRA_GPIO_INTERRUPT >> 5)
#define COBRA_INTERRUPT_BIT    (COBRA_GPIO_INTERRUPT & 31)
static PFNISR chained_isr = 0;
#define DEMOD_CONNECT_COUNTDOWN (256)
#define DEMOD_SCAN_COUNTDOWN (256)
static u_int32 demod_interrupt = 0;

/* These definitions are related to the tick timer.  Timeout values are in
   milliseconds. */
#if (INTERNAL_DEMOD == INTERNAL_COBRA_LIKE)
#define COBRA_CONNECTING_TIMEOUT 5000
#define COBRA_SCANNING_TIMEOUT 5000
#else
#define COBRA_CONNECTING_TIMEOUT 500
#define COBRA_SCANNING_TIMEOUT 500
#endif
tick_id_t hCobraTick[MAXIMUM_NUMBER_UNITS] = { 0, 0 };

#define BER_WINDOW_SIZE (255)
#define SKIP_BER

/* These definitions are related to the demod access method, IIC or ASX. */
#define DEMOD_INITIAL  (0)
#define DEMOD_IIC      (1)
#define DEMOD_REGISTER (2)
#define DEMOD_HYBRID   (3)

static u_int32 demod_access_method = DEMOD_INITIAL;
static u_int32 demod_iic_bus = I2C_BUS_NONE;

/* Per-unit data structures. */
static u_int32 local_state[MAXIMUM_NUMBER_UNITS];
static TUNING_SPEC local_tuning[MAXIMUM_NUMBER_UNITS];
static MODULE_STATUS_FUNCTION *callbacks[MAXIMUM_NUMBER_UNITS] = { 0, 0 };
static NIM NIMs[MAXIMUM_NUMBER_UNITS];
static ACQSTATE OldAcqState[MAXIMUM_NUMBER_UNITS];
static LOCKIND OldLock[MAXIMUM_NUMBER_UNITS];
static ACQSTATE NewAcqState[MAXIMUM_NUMBER_UNITS];
static LOCKIND NewLock[MAXIMUM_NUMBER_UNITS];
static int demod_handle[MAXIMUM_NUMBER_UNITS] = { 0, 0 };
static SCAN_SPEC local_scan[MAXIMUM_NUMBER_UNITS];
static u_int32 local_symrates[MAXIMUM_NUMBER_UNITS][16];
static NIM_SATELLITE_POLARISATION local_polarizations[MAXIMUM_NUMBER_UNITS][16];
static u_int32 local_viterbis[MAXIMUM_NUMBER_UNITS];
static struct {
    u_int32 current_symbol_rate;
    u_int32 current_frequency;
    u_int32 current_polarity;
} scan_parms[MAXIMUM_NUMBER_UNITS];
static u_int32 acq_failure_countdown[MAXIMUM_NUMBER_UNITS] = { 0, 0 };

/* Global tuning structure. */
static MPEG_OUT MPEG;

/* Global flags and counters. */
static u_int32 my_module = -1;
static u_int32 initialized = 0;
static int local_unit_count = 0;
static int global_timeout_flag = 0;
static int global_interrupt_flag = 0;

/* This are the default code rates to try when connecting in auto fec mode */
static unsigned int viterbicoderates = CODERATE_2DIV3 | CODERATE_3DIV4 | CODERATE_4DIV5 | CODERATE_5DIV6 | CODERATE_6DIV7 | CODERATE_7DIV8;

/* Some handy tags for trace level specifications. */
#define TL_ALWAYS  (TRACE_LEVEL_ALWAYS | TRACE_DMD)
#define TL_ERROR   (TRACE_LEVEL_ALWAYS | TRACE_DMD)
#define TL_INFO    (TRACE_LEVEL_3 | TRACE_DMD)
#define TL_FUNC    (TRACE_LEVEL_2 | TRACE_DMD)
#define TL_VERBOSE (TRACE_LEVEL_1 | TRACE_DMD)


/*****************************************************************************/
/*  FUNCTION:    NIM_Wait                                                    */
/*                                                                           */
/*  PARAMETERS:  nim - pointer to the NIM structure of the NIM requesting    */
/*                   the delay.                                              */
/*               waitms - the number of milliseconds to delay.               */
/*                                                                           */
/*  DESCRIPTION: This function is used by the NIM-specific code to provide   */
/*               an OS dependent method to delay a specified number of       */
/*               milliseconds.                                               */
/*                                                                           */
/*  RETURNS:     TRUE to indicate success.                                   */
/*                                                                           */
/*  CONTEXT:     Must be called from non-interrupt context.                  */
/*                                                                           */
/*****************************************************************************/
static BOOL NIM_Wait( NIM *nim, int waitms )
{
    task_time_sleep( waitms );
    return TRUE;
}

/*****************************************************************************/
/*  FUNCTION:    SBWrite                                                     */
/*                                                                           */
/*  PARAMETERS:  demod_handle - handle of demod for write request (San Diego */
/*                   code handle, not multi-instance demod handle).          */
/*               reg_addr - the register offset to be written.               */
/*               data - the data value to be written.                        */
/*               status - pointer to status value filled in at completion    */
/*                   (0 for success, non-0 for failure).                     */
/*                                                                           */
/*  DESCRIPTION: This function writes the specified value to the specified   */
/*               register offset of the specified demod unit.                */
/*                                                                           */
/*  RETURNS:     Nothing.                                                    */
/*                                                                           */
/*  CONTEXT:     Must be called from non-interrupt context.                  */
/*                                                                           */
/*****************************************************************************/
static void SBWrite( unsigned long demod_handle, unsigned char reg_addr, unsigned char data, unsigned long *status )
{
    unsigned char serial_bus_addr;

    if ( 0 == status )
    {
        trace_new( TL_ERROR, "Bad status parameter to Cobra SBWrite!\n" );
        error_log( ERROR_WARNING | RC_SDM_BADVAL );
        return;
    }

    if ( demod_handle & 0xfffeff00 )
    {
        trace_new( TL_ERROR, "Bad demod_handle parameter to Cobra SBWrite!\n" );
        error_log( ERROR_WARNING | RC_SDM_BADVAL );
        *status = 1;
        return;
    }

#if 0
wjj
this error check fails because sd code uses undocumented registers
    if ( reg_addr > 0x67 )
    {
        trace_new( TL_ERROR, "Bad reg_addr parameter to Cobra SBWrite!\n" );
        error_log( ERROR_WARNING | RC_SDM_BADVAL );
        *status = 1;
        return;
    }
#endif

    /* perform the write */
    if ( (demod_access_method == DEMOD_IIC)
#if (INTERNAL_DEMOD == INTERNAL_COBRA_LIKE)
            || ((demod_access_method == DEMOD_HYBRID) &&
                (reg_addr == 0 || reg_addr == 0x33 || reg_addr == 0x34 ||
                reg_addr == 0x39 || reg_addr == 0x3a || reg_addr == 0x3b || reg_addr == 0x3c ||
                reg_addr == 0x43 || reg_addr == 0x44))
#endif
        )
    {
        /* get serial bus address from demod handle */
        serial_bus_addr = (unsigned char)(demod_handle & 0x000000FF);

        /* determine DEMOD_A or DEMOD_B register address offset. */
        if ( (demod_handle >> 16) != 0 )
            reg_addr |= 0x80;   /* add 128 to register address. */

#if (INTERNAL_DEMOD == INTERNAL_COBRA_LIKE)
        if ( demod_access_method == DEMOD_HYBRID )
        {
            CNXT_SET( PLL_PAD_FAST_CTRL_REG, PLL_FAST_CTRL_DEMOD_CONNECT_SEL_MASK,
                    PLL_FAST_CTRL_DEMOD_CONNECT_I2C );
        }
#endif

        *status = !iicWriteIndexedReg( serial_bus_addr, reg_addr, data, demod_iic_bus );

#if (INTERNAL_DEMOD == INTERNAL_COBRA_LIKE)
        if ( demod_access_method == DEMOD_HYBRID )
        {
            CNXT_SET( PLL_PAD_FAST_CTRL_REG, PLL_FAST_CTRL_DEMOD_CONNECT_SEL_MASK,
                    PLL_FAST_CTRL_DEMOD_CONNECT_ASX );
        }
#endif
    }
#if (INTERNAL_DEMOD == INTERNAL_COBRA_LIKE)
    else if ( demod_access_method == DEMOD_REGISTER || demod_access_method == DEMOD_HYBRID )
    {
        *(LPREG)INTERNAL_DEMOD_REG_TO_ASX_ADDR(reg_addr) = data;
        *status = 0; /* indicate success */
    }
#endif
    else
    {
        trace_new( TL_ERROR, "Bad demod access method in Cobra SBWrite!\n" );
        *status = 1; /* indicate failure */
    }
}

/*****************************************************************************/
/*  FUNCTION:    SBRead                                                      */
/*                                                                           */
/*  PARAMETERS:  demod_handle - handle of demod for read request (San Diego  */
/*                   code handle, not multi-instance demod handle).          */
/*               reg_addr - the register offset to be read.                  */
/*               status - pointer to status value filled in at completion    */
/*                   (0 for success, non-0 for failure).                     */
/*                                                                           */
/*  DESCRIPTION: This function reads from the specified register offset of   */
/*               the specified demod unit and returns the value read.        */
/*                                                                           */
/*  RETURNS:     The value read from the device.                             */
/*                                                                           */
/*  CONTEXT:     Must be called from non-interrupt context.                  */
/*                                                                           */
/*****************************************************************************/
static unsigned char SBRead( unsigned long demod_handle, unsigned char reg_addr, unsigned long *status )
{
    unsigned char serial_bus_addr;
    unsigned char reg_value = 0;

    if ( 0 == status )
    {
        trace_new( TL_ERROR, "Bad status parameter to Cobra SBRead!\n" );
        error_log( ERROR_WARNING | RC_SDM_BADVAL );
        return 0;
    }

    if ( demod_handle & 0xfffeff00 )
    {
        trace_new( TL_ERROR, "Bad demod_handle parameter to Cobra SBRead!\n" );
        error_log( ERROR_WARNING | RC_SDM_BADVAL );
        *status = 1;
        return 0;
    }

#if 0
wjj
this error check fails because sd code uses undocumented registers
    if ( reg_addr > 0x67 )
    {
        trace_new( TL_ERROR, "Bad reg_addr parameter to Cobra SBRead!\n" );
        error_log( ERROR_WARNING | RC_SDM_BADVAL );
        *status = 1;
        return 0;
    }
#endif

    /* perform the read */
    if ( (demod_access_method == DEMOD_IIC)
#if (INTERNAL_DEMOD == INTERNAL_COBRA_LIKE)
            || ((demod_access_method == DEMOD_HYBRID) &&
                (reg_addr == 0 || reg_addr == 0x33 || reg_addr == 0x34 ||
                reg_addr == 0x39 || reg_addr == 0x3a || reg_addr == 0x3b || reg_addr == 0x3c ||
                reg_addr == 0x43 || reg_addr == 0x44))
#endif
        )
    {
        /* get serial bus address from demod handle */
        serial_bus_addr = (unsigned char)(demod_handle & 0x000000FF);

        /* determine DEMOD_A or DEMOD_B register address offset. */
        if ( (demod_handle >> 16) != 0 )
            reg_addr |= 0x80;   /* add 128 to register address. */

#if (INTERNAL_DEMOD == INTERNAL_COBRA_LIKE)
        if ( demod_access_method == DEMOD_HYBRID )
        {
            CNXT_SET( PLL_PAD_FAST_CTRL_REG, PLL_FAST_CTRL_DEMOD_CONNECT_SEL_MASK,
                    PLL_FAST_CTRL_DEMOD_CONNECT_I2C );
        }
#endif

        *status = !iicReadIndexedReg( serial_bus_addr, reg_addr, &reg_value, demod_iic_bus );

#if (INTERNAL_DEMOD == INTERNAL_COBRA_LIKE)
        if ( demod_access_method == DEMOD_HYBRID )
        {
            CNXT_SET( PLL_PAD_FAST_CTRL_REG, PLL_FAST_CTRL_DEMOD_CONNECT_SEL_MASK,
                    PLL_FAST_CTRL_DEMOD_CONNECT_ASX );
        }
#endif
    }
#if (INTERNAL_DEMOD == INTERNAL_COBRA_LIKE)
    else if ( demod_access_method == DEMOD_REGISTER || demod_access_method == DEMOD_HYBRID )
    {
        reg_value = *(LPREG)INTERNAL_DEMOD_REG_TO_ASX_ADDR(reg_addr);
        *status = 0; /* indicate success */
    }
#endif
    else
    {
        trace_new( TL_ERROR, "Bad demod access method in Cobra SBRead!\n" );
        *status = 1; /* indicate failure */
        return 0;
    }

    return reg_value;
}

/*****************************************************************************/
/*  FUNCTION:    xlat_fec_in                                                 */
/*                                                                           */
/*  PARAMETERS:  fec - the FEC rate as defined for multi-instance demod.     */
/*                                                                           */
/*  DESCRIPTION: This function translates an FEC rate from the enumeration   */
/*               used by multi-instance demod to that used by San Diego      */
/*               driver code.                                                */
/*                                                                           */
/*  RETURNS:     The San Diego driver code equivalent FEC rate.              */
/*                                                                           */
/*  CONTEXT:     May be called from interrupt or non-interrupt context.      */
/*                                                                           */
/*****************************************************************************/
static CODERATE xlat_fec_in( NIM_FEC_RATE fec )
{
    switch ( fec )
    {
        case M_RATE_1_2:  return CODERATE_1DIV2;
        case M_RATE_2_3:  return CODERATE_2DIV3;
        case M_RATE_3_4:  return CODERATE_3DIV4;
        case M_RATE_3_5:  return CODERATE_3DIV5;
        case M_RATE_4_5:  return CODERATE_4DIV5;
        case M_RATE_5_6:  return CODERATE_5DIV6;
        case M_RATE_5_11: return CODERATE_5DIV11;
        case M_RATE_6_7:  return CODERATE_6DIV7;
        case M_RATE_7_8:  return CODERATE_7DIV8;
        default:          return CODERATE_NONE;
    }
}

/*****************************************************************************/
/*  FUNCTION:    xlat_fec_out                                                */
/*                                                                           */
/*  PARAMETERS:  fec - the FEC rate as defined by San Diego driver code.     */
/*                                                                           */
/*  DESCRIPTION: This function translates an FEC rate from the enumeration   */
/*               used by San Diego driver code to that used by the multi-    */
/*               instance demod.                                             */
/*                                                                           */
/*  RETURNS:     The multi-instance demod equivalent FEC rate.               */
/*                                                                           */
/*  CONTEXT:     May be called from interrupt or non-interrupt context.      */
/*                                                                           */
/*****************************************************************************/
NIM_FEC_RATE xlat_fec_out( CODERATE fec )
{
    switch ( fec )
    {
        case CODERATE_1DIV2:  return M_RATE_1_2;
        case CODERATE_2DIV3:  return M_RATE_2_3;
        case CODERATE_3DIV4:  return M_RATE_3_4;
        case CODERATE_3DIV5:  return M_RATE_3_5;
        case CODERATE_4DIV5:  return M_RATE_4_5;
        case CODERATE_5DIV6:  return M_RATE_5_6;
        case CODERATE_5DIV11: return M_RATE_5_11;
        case CODERATE_6DIV7:  return M_RATE_6_7;
        case CODERATE_7DIV8:  return M_RATE_7_8;
        default:              return M_RATE_NONE;
    }
}

/*****************************************************************************/
/*  FUNCTION:    xlat_spectrum_in                                            */
/*                                                                           */
/*  PARAMETERS:  spectrum - the spectrum specifier as defined for multi-     */
/*               instance demod.                                             */
/*                                                                           */
/*  DESCRIPTION: This function translates a spectrum specifier from the      */
/*               enumeration used by multi-instance demod to that used by    */
/*               San Diego driver code.                                      */
/*                                                                           */
/*  RETURNS:     The San Diego driver code equivalent spectrum specifier.    */
/*                                                                           */
/*  CONTEXT:     May be called from interrupt or non-interrupt context.      */
/*                                                                           */
/*****************************************************************************/
static SPECINV xlat_spectrum_in( NIM_SATELLITE_SPECTRUM spectrum )
{
    switch ( spectrum )
    {
        case SAT_SPECTRUM_NORMAL:    return SPEC_INV_OFF;
        case SAT_SPECTRUM_INVERTED:  return SPEC_INV_ON;
        case SAT_SPECTRUM_AUTO:      return SPEC_INV_BOTH;
        default:                     return SPEC_INV_BOTH;
    }
}

/*****************************************************************************/
/*  FUNCTION:    cobra_connecting_state                                      */
/*                                                                           */
/*  PARAMETERS:  unit - the unit to process in the CONNECTING state.         */
/*               pending - the mask of currently pending interrupts.         */
/*               old - the previous acquisition state.                       */
/*               new - the new (current) acquisition state.                  */
/*                                                                           */
/*  DESCRIPTION: This function implements the processing required on receipt */
/*               of an interrupt in the CONNECTING state.                    */
/*                                                                           */
/*  RETURNS:     Nothing.                                                    */
/*                                                                           */
/*  CONTEXT:     Will be run in task context.                                */
/*                                                                           */
/*****************************************************************************/
static void cobra_connecting_state( u_int32 unit, INTEROPTS pending, ACQSTATE old, ACQSTATE new )
{
    int rc;
    DEMOD_CALLBACK_DATA parm;
     CODERATE codeRate;

    trace_new( TL_FUNC, "CONNECTING state processing for unit %d\n", unit );

    /* If we got an acquisition interrupt, and the new acquisition state is
       locked and tracking, then we will transition to the connected state. */
    if ( (pending & INTR_ACQ_SYNC) && (new == ACQ_LOCKED_AND_TRACKING) )
    {
        local_state[unit] = CONNECTED;
          /* Get the code rate */
          API_GetViterbiRate(&NIMs[unit], &codeRate);
          /* Update the local tuning structure with the code rate */
          local_tuning[unit].tune.nim_satellite_tune.fec = xlat_fec_out(codeRate);

        rc = tick_stop( hCobraTick[unit] );
        if ( rc != RC_OK )
        {
            trace_new( TL_ERROR, "Cobra can't stop tick timer!\n" );
            error_log( ERROR_WARNING | RC_SDM_SYSERR );
        }

        trace_new( TL_FUNC, "Switching to CONNECTED state for unit %d\n", unit );
        trace_new( TL_VERBOSE, "There were %d acquisition failures\n", DEMOD_CONNECT_COUNTDOWN-acq_failure_countdown[unit] );

        if ( callbacks[unit] )
        {
            parm.parm.type = DEMOD_CONNECTED;
            callbacks[unit]( my_module, unit, DEMOD_CONNECT_STATUS, &parm );
        }

        /* Now clear the pending interrupts from the demod, and re-enable
           interrupts for a loss of sync. */
        API_ClearPendingInterrupts( &NIMs[unit] );
        API_SetInterruptOptions( &NIMs[unit], COBRA_CONNECTED_INTERRUPTS );
#if (INTERNAL_DEMOD == INTERNAL_COBRA_LIKE)
#else
        if ( RC_OK != int_enable( demod_interrupt ) )
        {
            trace_new( TL_ERROR, "Cobra can't enable NIM interrupt in cobra_connecting_state!\n" );
            error_log( ERROR_FATAL | RC_SDM_SYSERR );
        }
#endif
    }

    /* We got an acquisition interrupt, but the new acquisition state is not
       locked and tracking, so count down to determine status. */
    else
    {
        /* If not completely through, reset the interrupt and keep going. */
        if ( acq_failure_countdown[unit] )
        {
            --acq_failure_countdown[unit];
            API_ClearPendingInterrupts( &NIMs[unit] );
#if (INTERNAL_DEMOD == INTERNAL_COBRA_LIKE)
#else
            if ( RC_OK != int_enable( demod_interrupt ) )
            {
                trace_new( TL_ERROR, "Cobra can't enable NIM interrupt in cobra_connecting_state!\n" );
                error_log( ERROR_FATAL | RC_SDM_SYSERR );
            }
#endif
        }
        else
        {
            local_state[unit] = DISCONNECTED;

            rc = tick_stop( hCobraTick[unit] );
            if ( rc != RC_OK )
            {
                trace_new( TL_ERROR, "Cobra can't stop tick timer!\n" );
                error_log( ERROR_WARNING | RC_SDM_SYSERR );
            }

            if ( callbacks[unit] )
            {
                parm.parm.type = DEMOD_FAILED;
                callbacks[unit]( my_module, unit, DEMOD_CONNECT_STATUS, &parm );
            }
        }
    }
}

/*****************************************************************************/
/*  FUNCTION:    cobra_connected_state                                       */
/*                                                                           */
/*  PARAMETERS:  unit - the unit to process in the CONNECTED state.          */
/*               pending - the mask of currently pending interrupts.         */
/*               old - the previous acquisition state.                       */
/*               new - the new (current) acquisition state.                  */
/*                                                                           */
/*  DESCRIPTION: This function implements the processing required on receipt */
/*               of an interrupt in the CONNECTED state.                     */
/*                                                                           */
/*  RETURNS:     Nothing.                                                    */
/*                                                                           */
/*  CONTEXT:     Will be run in task context.                                */
/*                                                                           */
/*****************************************************************************/
static void cobra_connected_state( u_int32 unit, INTEROPTS pending, ACQSTATE old, ACQSTATE new )
{
    DEMOD_CALLBACK_DATA parm;

    trace_new( TL_FUNC, "CONNECTED state processing for unit %d\n", unit );

    /* If we got a loss of sync interrupt, and the new acquisition state is not
       locked and tracking, then we will transition to the signal fade state. */
    /* 2004-11-8 14:32 Derek modified it for miss signal lost in low symbol rate TP*/ 
    if ( (pending & COBRA_CONNECTED_INTERRUPTS) && (new != ACQ_LOCKED_AND_TRACKING) )  
//    if ( (pending & INTR_ACQ_FAILURE) && (new != ACQ_LOCKED_AND_TRACKING) )/* Orginal */
    {
        trace_new( TL_FUNC, "Switching to FADE state for unit %d\n", unit );
        local_state[unit] = FADE;

        if ( callbacks[unit] )
        {
            parm.parm.type = DEMOD_DRIVER_LOST_SIGNAL;
            callbacks[unit]( my_module, unit, DEMOD_CONNECT_STATUS, &parm );
        }

        /* Now clear the pending interrupts from the demod, and re-enable
           interrupts for regaining sync. */
        API_ClearPendingInterrupts( &NIMs[unit] );
        API_SetInterruptOptions( &NIMs[unit], COBRA_CONNECTING_INTERRUPTS );
#if (INTERNAL_DEMOD == INTERNAL_COBRA_LIKE)
#else
        if ( RC_OK != int_enable( demod_interrupt ) )
        {
            trace_new( TL_ERROR, "Cobra can't enable NIM interrupt in cobra_connected_state!\n" );
            error_log( ERROR_FATAL | RC_SDM_SYSERR );
        }
#endif
    }

    /* We got a loss of sync interrupt, but the new acquisition state is
       locked and tracking, so wait for the next interrupt. */
    else
    {
        API_ClearPendingInterrupts( &NIMs[unit] );
#if (INTERNAL_DEMOD == INTERNAL_COBRA_LIKE)
#else
        if ( RC_OK != int_enable( demod_interrupt ) )
        {
            trace_new( TL_ERROR, "Cobra can't enable NIM interrupt in cobra_connected_state!\n" );
            error_log( ERROR_FATAL | RC_SDM_SYSERR );
        }
#endif
    }
}

/*****************************************************************************/
/*  FUNCTION:    cobra_scanning_state                                        */
/*                                                                           */
/*  PARAMETERS:  unit - the unit to process in the SCANNING state.           */
/*               pending - the mask of currently pending interrupts.         */
/*               old - the previous acquisition state.                       */
/*               new - the new (current) acquisition state.                  */
/*                                                                           */
/*  DESCRIPTION: This function implements the processing required on receipt */
/*               of an interrupt in the SCANNING state.                      */
/*                                                                           */
/*  RETURNS:     Nothing.                                                    */
/*                                                                           */
/*  CONTEXT:     Will be run in task context.                                */
/*                                                                           */
/*****************************************************************************/
static void cobra_scanning_state( u_int32 unit, INTEROPTS pending, ACQSTATE old, ACQSTATE new )
{
    int rc;
    DEMOD_CALLBACK_DATA parm;
    u_int32 msg[4];
    int return_code;
    CODERATE coderate;

    trace_new( TL_FUNC, "SCANNING state processing for unit %d\n", unit );

    /* If we got an acquisition interrupt, and the new acquisition state is
       either locked and tracking or fade, then we will transition to the
       connected state. */
    if ( (pending & INTR_ACQ_SYNC) && ((new == ACQ_LOCKED_AND_TRACKING) || (new == ACQ_FADE)) )
    {
        local_state[unit] = CONNECTED;

        rc = tick_stop( hCobraTick[unit] );
        if ( rc != RC_OK )
        {
            trace_new( TL_ERROR, "Cobra can't stop tick timer!\n" );
            error_log( ERROR_WARNING | RC_SDM_SYSERR );
        }

        API_GetViterbiRate( &NIMs[unit], &coderate );
        local_tuning[unit].tune.nim_satellite_tune.fec = xlat_fec_out( coderate );
        trace_new( TL_FUNC, "Switching to CONNECTED state for unit %d\n", unit );
        trace_new( TL_VERBOSE, "There were %d acquisition failures\n", DEMOD_SCAN_COUNTDOWN-acq_failure_countdown[unit] );

        if ( callbacks[unit] )
        {
            parm.parm.type = DEMOD_SCAN_COMPLETE;
            callbacks[unit]( my_module, unit, DEMOD_SCAN_STATUS, &parm );
        }

        /* Now clear the pending interrupts from the demod, and re-enable
           interrupts for a loss of sync. */
        API_ClearPendingInterrupts( &NIMs[unit] );
        API_SetInterruptOptions( &NIMs[unit], COBRA_CONNECTED_INTERRUPTS );
#if (INTERNAL_DEMOD == INTERNAL_COBRA_LIKE)
#else
        if ( RC_OK != int_enable( demod_interrupt ) )
        {
            trace_new( TL_ERROR, "Cobra can't enable NIM interrupt in cobra_scanning_state!\n" );
            error_log( ERROR_FATAL | RC_SDM_SYSERR );
        }
#endif
    }

    /* Either we got an acquisition interrupt, but the new acquisition state is
       neither locked and tracking nor fade, or we got an acquisition failure;
       count down to determine status. */
    else
    {
        /* If not completely through, reset the interrupt and keep going. */
        if ( acq_failure_countdown[unit] )
        {
            --acq_failure_countdown[unit];
            API_ClearPendingInterrupts( &NIMs[unit] );
#if (INTERNAL_DEMOD == INTERNAL_COBRA_LIKE)
#else
            int_enable( demod_interrupt );
#endif
        }
        /* Enough is enough; try something else or give up completely. */
        else
        {
            /* Send a control message to the Cobra demod task to signal the timeout. */
            msg[0] = unit;
            msg[1] = msg[2] = 0;
            msg[3] = COBRA_TIMEOUT;
            return_code = qu_send( cobra_message_q, msg );
            if (return_code != RC_OK)
            {
                trace_new( TL_ERROR, "Cobra can't send message in cobra_scanning_state!\n" );
                error_log( ERROR_WARNING | RC_SDM_QFULL );
                global_timeout_flag |= (unit == 0) ? 1 : 2;
            }
        }
    }
}

/*****************************************************************************/
/*  FUNCTION:    cobra_fade_state                                            */
/*                                                                           */
/*  PARAMETERS:  unit - the unit to process in the FADE state.               */
/*               pending - the mask of currently pending interrupts.         */
/*               old - the previous acquisition state.                       */
/*               new - the new (current) acquisition state.                  */
/*                                                                           */
/*  DESCRIPTION: This function implements the processing required on receipt */
/*               of an interrupt in the FADE state.                          */
/*                                                                           */
/*  RETURNS:     Nothing.                                                    */
/*                                                                           */
/*  CONTEXT:     Will be run in task context.                                */
/*                                                                           */
/*****************************************************************************/
static void cobra_fade_state( u_int32 unit, INTEROPTS pending, ACQSTATE old, ACQSTATE new )
{
    DEMOD_CALLBACK_DATA parm;

    trace_new( TL_FUNC, "FADE state processing for unit %d\n", unit );

    /* If we got an acquisition interrupt, and the new acquisition state is
       locked and tracking, then we will transition to the connected state. */
    if ( (pending & INTR_ACQ_SYNC) && (new == ACQ_LOCKED_AND_TRACKING) )
    {
        local_state[unit] = CONNECTED;
        trace_new( TL_FUNC, "Switching to CONNECTED state for unit %d\n", unit );

        if ( callbacks[unit] )
        {
            parm.parm.type = DEMOD_DRIVER_REACQUIRED_SIGNAL;
            callbacks[unit]( my_module, unit, DEMOD_CONNECT_STATUS, &parm );
        }

        /* Now clear the pending interrupts from the demod, and re-enable
           interrupts for a loss of sync. */
        API_ClearPendingInterrupts( &NIMs[unit] );
        API_SetInterruptOptions( &NIMs[unit], COBRA_CONNECTED_INTERRUPTS );
#if (INTERNAL_DEMOD == INTERNAL_COBRA_LIKE)
#else
        if ( RC_OK != int_enable( demod_interrupt ) )
        {
            trace_new( TL_ERROR, "Cobra can't enable NIM interrupt in cobra_connecting_state!\n" );
            error_log( ERROR_FATAL | RC_SDM_SYSERR );
        }
#endif
    }

    /* We got an acquisition interrupt, but the new acquisition state is not
       locked and tracking.  Clear the pending interrupts and wait for another. */
    else
    {
        API_ClearPendingInterrupts( &NIMs[unit] );
#if (INTERNAL_DEMOD == INTERNAL_COBRA_LIKE)
#else
        if ( RC_OK != int_enable( demod_interrupt ) )
        {
            trace_new( TL_ERROR, "Cobra can't enable NIM interrupt in cobra_connecting_state!\n" );
            error_log( ERROR_FATAL | RC_SDM_SYSERR );
        }
#endif
    }
}

/*****************************************************************************/
/*  FUNCTION:    cobra_timeout_msg                                           */
/*                                                                           */
/*  PARAMETERS:  unit - the unit that experienced the timeout.               */
/*                                                                           */
/*  DESCRIPTION: This function implements the processing required on receipt */
/*               of a TIMEOUT message.                                       */
/*                                                                           */
/*  RETURNS:     Nothing.                                                    */
/*                                                                           */
/*  CONTEXT:     Will be run in task context.                                */
/*                                                                           */
/*****************************************************************************/
static void cobra_timeout_msg( u_int32 unit )
{
    DEMOD_CALLBACK_DATA parm;
    int more_scanning = 1;
    CHANOBJ chan;
    BOOL APIresult;
    int rc;
    int api_error;
    CMPLXNO esno;
    MSTATUS status;

    trace_new( TL_FUNC, "New cobra timeout message for unit %d\n", unit );

    switch( local_state[unit] )
    {
        case CONNECTING:
            /* Since we timed out in CONNECTING, we don't want any more
               interrupts and we announce the failure. */
            API_SetInterruptOptions( &NIMs[unit], COBRA_NO_INTERRUPTS );
            local_state[unit] = DISCONNECTED;
            if ( callbacks[unit] )
            {
                parm.parm.type = DEMOD_FAILED;
                callbacks[unit]( my_module, unit, DEMOD_CONNECT_STATUS, &parm );
            }
            break;

        case SCANNING:
            /* Since we timed out in SCANNING, check to see if there are more
               settings to try before we declare failure. */
            if ( ++scan_parms[unit].current_symbol_rate >= local_scan[unit].scan.satellite.num_symrates )
            {
                scan_parms[unit].current_symbol_rate = 0;
                scan_parms[unit].current_frequency += local_scan[unit].scan.satellite.hop_value;
                if ( scan_parms[unit].current_frequency > local_scan[unit].scan.satellite.end_frequency )
                {
                    scan_parms[unit].current_frequency = local_scan[unit].scan.satellite.start_frequency;
                    if ( ++scan_parms[unit].current_polarity >= local_scan[unit].scan.satellite.num_pols )
                    {
                        API_SetInterruptOptions( &NIMs[unit], COBRA_NO_INTERRUPTS );
                        local_state[unit] = DISCONNECTED;
                        if ( callbacks[unit] )
                        {
                            parm.parm.type = DEMOD_SCAN_NO_SIGNAL;
                            callbacks[unit]( my_module, unit, DEMOD_SCAN_STATUS, &parm );
                        }
                        more_scanning = 0;
                    }
                }
            }

            /* More to try, so set up the next tuning operation and let fly. */
            if ( more_scanning )
            {
                local_scan[unit].current.tune.nim_satellite_tune.symbol_rate = local_symrates[unit][scan_parms[unit].current_symbol_rate];
                local_scan[unit].current.tune.nim_satellite_tune.frequency = scan_parms[unit].current_frequency;
                local_scan[unit].current.tune.nim_satellite_tune.polarisation = local_polarizations[unit][scan_parms[unit].current_polarity];
                local_tuning[unit] = local_scan[unit].current;
                cnxt_set_lnb( &NIMs[unit], &(local_scan[unit].current), &chan.frequency );
                chan.modtype = MOD_QPSK;
                chan.coderate = xlat_fec_in( local_scan[unit].current.tune.nim_satellite_tune.fec );
                chan.symbrate = local_scan[unit].current.tune.nim_satellite_tune.symbol_rate;
                chan.specinv = SPEC_INV_BOTH; /* Fix me! wjj */
                chan.samplerate = SAMPLE_FREQ_NOM;
                chan.viterbicoderates = local_viterbis[unit];

                /* Make sure all interrupts (including acq_fail) are reset, then
                   call API_ChangeChannel to accomplish the tuning. */
                API_AcqBegin( &NIMs[unit] );
                API_ClearPendingInterrupts( &NIMs[unit] );
                acq_failure_countdown[unit] = DEMOD_SCAN_COUNTDOWN;
                APIresult = API_ChangeChannel( &NIMs[unit], &chan );

                if ( APIresult )
                {
                    trace_new( TL_VERBOSE, "API_ChangeChannel succeeded for unit %d\n", unit );

                    /* Start the demod monitoring signal quality. */
                    if ( !API_GetChannelEsNo( &NIMs[0], ESNOMODE_SNAPSHOT, &esno, &status ) )
                    {
                        trace_new( TL_ERROR, "API_GetChannelEsNo failed\n" );
                        trace_new( TL_ERROR, "File: %s, line: %d\n", API_GetErrorFilename(&NIMs[0]), API_GetErrorLineNumber(&NIMs[0]) );
                        api_error = API_GetLastError( &NIMs[0] );
                        trace_new( TL_ERROR, "Error %d, %s\n", api_error, API_GetErrorMessage(&NIMs[0], (APIERRNO)api_error) );
                    }
    
                    rc = tick_set( hCobraTick[unit], COBRA_SCANNING_TIMEOUT, TRUE );
                    if ( rc != RC_OK )
                    {
                        trace_new( TL_ERROR, "Cobra can't set tick timer for scan continue!\n" );
                        error_log( ERROR_FATAL | RC_SDM_SYSERR );
                    }
                    else
                    {
                        rc = tick_start( hCobraTick[unit] );
                        if ( rc != RC_OK )
                        {
                            trace_new( TL_ERROR, "Cobra can't start tick timer for scan continue!\n" );
                            error_log( ERROR_FATAL | RC_SDM_SYSERR );
                        }
                    }
                    API_SetInterruptOptions( &NIMs[unit], COBRA_SCANNING_INTERRUPTS );
#if (INTERNAL_DEMOD == INTERNAL_COBRA_LIKE)
#else
                    if ( RC_OK != int_enable( demod_interrupt ) )
                    {
                        trace_new( TL_ERROR, "Cobra can't enable NIM interrupt in cobra_timeout_msg!\n" );
                        error_log( ERROR_FATAL | RC_SDM_SYSERR );
                    }
#endif
                }
                else
                {
                    API_SetInterruptOptions( &NIMs[unit], COBRA_NO_INTERRUPTS );
                    trace_new( TL_ERROR, "API_ChangeChannel failed for unit %d\n", unit );
                    trace_new( TL_ERROR, "File: %s, line: %d\n", API_GetErrorFilename(&NIMs[unit]), API_GetErrorLineNumber(&NIMs[unit]) );
                    api_error = API_GetLastError( &NIMs[unit] );
                    trace_new( TL_ERROR, "Error %d, %s\n", api_error, API_GetErrorMessage(&NIMs[unit], (APIERRNO)api_error) );
                    if ( callbacks[unit] )
                    {
                        parm.parm.type = DEMOD_FAILED;
                        callbacks[unit]( my_module, unit, DEMOD_SCAN_STATUS, &parm );
                    }
                }
            }
            break;

        case CONNECTED:
            break;

        case UNINITIALIZED:
        case DISCONNECTED:
        default:
            break;
    }
}

/*****************************************************************************/
/*  FUNCTION:    cobra_interrupt_msg                                         */
/*                                                                           */
/*  PARAMETERS:  None.                                                       */
/*                                                                           */
/*  DESCRIPTION: This function implements the processing required on receipt */
/*               of an INTERRUPT message.                                    */
/*                                                                           */
/*  RETURNS:     Nothing.                                                    */
/*                                                                           */
/*  CONTEXT:     Will be run in task context.                                */
/*                                                                           */
/*****************************************************************************/
static void cobra_interrupt_msg( void )
{
    int ii;
    INTEROPTS pending;

    trace_new( TL_FUNC, "New cobra interrupt message\n" );

    for ( ii=0; ii<local_unit_count; ++ii )
    {
        API_GetPendingInterrupts( &NIMs[ii], &pending );
        trace_new( TL_VERBOSE, "Pending interrupts for unit %d: 0x%02x\n", ii, pending );

        API_Monitor( &NIMs[ii], &NewAcqState[ii], &NewLock[ii] );
        if ( NewAcqState[ii] != OldAcqState[ii] )
        {
            trace_new( TL_FUNC, "OldAcqState = %d, NewAcqState = %d\n", OldAcqState[ii], NewAcqState[ii] );
        }
        else
        {
            trace_new( TL_VERBOSE, "OldAcqState = %d, NewAcqState = %d\n", OldAcqState[ii], NewAcqState[ii] );
        }

        switch( local_state[ii] )
        {
            case CONNECTING:
                cobra_connecting_state( ii, pending, OldAcqState[ii], NewAcqState[ii] );
                break;

            case CONNECTED:
                cobra_connected_state( ii, pending, OldAcqState[ii], NewAcqState[ii] );
                break;

            case SCANNING:
                cobra_scanning_state( ii, pending, OldAcqState[ii], NewAcqState[ii] );
                break;

            case FADE:
                cobra_fade_state( ii, pending, OldAcqState[ii], NewAcqState[ii] );
                break;

            /* Interrupts in unexpected states can be ignored.  The ISR already
               disabled any further interrupts. */
            case UNINITIALIZED:
            case DISCONNECTED:
            default:
                break;
        }

        /* New (current) state becomes the old state. */
        OldAcqState[ii] = NewAcqState[ii];
        OldLock[ii] = NewLock[ii];
    }
}

/*****************************************************************************/
/*  FUNCTION:    cobra_connect_msg                                           */
/*                                                                           */
/*  PARAMETERS:  unit - the unit specified in the CONNECT message.           */
/*               tune - the tuning specification in the CONNECT messsage.    */
/*                                                                           */
/*  DESCRIPTION: This function implements the processing required on receipt */
/*               of a CONNECT message.                                       */
/*                                                                           */
/*  RETURNS:     Nothing.                                                    */
/*                                                                           */
/*  CONTEXT:     Will be run in task context.                                */
/*                                                                           */
/*****************************************************************************/
static void cobra_connect_msg( u_int32 unit, TUNING_SPEC *tune )
{
    CHANOBJ chan;
    int api_error;
    DEMOD_CALLBACK_DATA parm;
    BOOL APIresult;
    int rc;
    CMPLXNO esno;
    MSTATUS status;

    trace_new( TL_FUNC, "New connect message for unit %d\n", unit );

    local_state[unit] = CONNECTING;

    cnxt_set_lnb( &NIMs[unit], tune, &chan.frequency );

    chan.modtype = MOD_QPSK;
     if(tune->tune.nim_satellite_tune.fec != M_RATE_AUTO)
     {
          /* Manual mode. */
          chan.coderate = xlat_fec_in( tune->tune.nim_satellite_tune.fec );       
          /* Not sure why we do this but I'll leave it there as it dosent cause much harm! */
          chan.viterbicoderates = CODERATE_2DIV3 | CODERATE_3DIV4; /* only for testing - should be 0 */
     }
     else
     {
         /* Automatic mode, try all code rates. */
         chan.coderate = CODERATE_1DIV2;
          chan.viterbicoderates = viterbicoderates;
     }
    chan.symbrate = tune->tune.nim_satellite_tune.symbol_rate;
    chan.specinv = xlat_spectrum_in( tune->tune.nim_satellite_tune.spectrum );
    chan.samplerate = SAMPLE_FREQ_NOM;
    
    /* Make sure all interrupts (including acq_fail) are reset, then call
       API_ChangeChannel to accomplish the tuning. */
    API_AcqBegin( &NIMs[unit] );
    API_ClearPendingInterrupts( &NIMs[unit] );
    acq_failure_countdown[unit] = DEMOD_CONNECT_COUNTDOWN;
    APIresult = API_ChangeChannel( &NIMs[unit], &chan );

    if ( APIresult )
    {
        trace_new( TL_VERBOSE, "API_ChangeChannel succeeded for unit %d\n", unit );

#if 0
{ u_int32 ii;
task_time_sleep( 1000 );
for ( ii=0x304e0000; ii<=0x304e03fc; ii+=32 )
{
    trace_new( TL_VERBOSE, "0x%08x: 0x%02x\n0x%08x: 0x%02x\n0x%08x: 0x%02x\n0x%08x: 0x%02x\n0x%08x: 0x%02x\n0x%08x: 0x%02x\n0x%08x: 0x%02x\n0x%08x: 0x%02x\n",
    ii, *(LPREG)(ii), ii+4, *(LPREG)(ii+4), ii+8, *(LPREG)(ii+8), ii+12, *(LPREG)(ii+12), ii+16, *(LPREG)(ii+16), ii+20, *(LPREG)(ii+20), ii+24, *(LPREG)(ii+24), ii+28, *(LPREG)(ii+28) );
}
}
#endif

        /* Start the demod monitoring signal quality. */
        if ( !API_GetChannelEsNo( &NIMs[0], ESNOMODE_SNAPSHOT, &esno, &status ) )
        {
            trace_new( TL_ERROR, "API_GetChannelEsNo failed\n" );
            trace_new( TL_ERROR, "File: %s, line: %d\n", API_GetErrorFilename(&NIMs[0]), API_GetErrorLineNumber(&NIMs[0]) );
            api_error = API_GetLastError( &NIMs[0] );
            trace_new( TL_ERROR, "Error %d, %s\n", api_error, API_GetErrorMessage(&NIMs[0], (APIERRNO)api_error) );
        }
    
        rc = tick_set( hCobraTick[unit], COBRA_CONNECTING_TIMEOUT, TRUE );
        if ( rc != RC_OK )
        {
            trace_new( TL_ERROR, "Cobra can't set tick timer for connect message!\n" );
            error_log( ERROR_FATAL | RC_SDM_SYSERR );
        }
        else
        {
            rc = tick_start( hCobraTick[unit] );
            if ( rc != RC_OK )
            {
                trace_new( TL_ERROR, "Cobra can't start tick timer for connect message!\n" );
                error_log( ERROR_FATAL | RC_SDM_SYSERR );
            }
        }
        API_SetInterruptOptions( &NIMs[unit], COBRA_CONNECTING_INTERRUPTS );
#if (INTERNAL_DEMOD == INTERNAL_COBRA_LIKE)
#else
        if ( RC_OK != int_enable( demod_interrupt ) )
        {
            trace_new( TL_ERROR, "Cobra can't enable NIM interrupt in cobra_connect_msg!\n" );
            error_log( ERROR_FATAL | RC_SDM_SYSERR );
        }
#endif
    }
    else
    {
        API_SetInterruptOptions( &NIMs[unit], COBRA_NO_INTERRUPTS );
        trace_new( TL_ERROR, "API_ChangeChannel failed for unit %d\n", unit );
        trace_new( TL_ERROR, "File: %s, line: %d\n", API_GetErrorFilename(&NIMs[unit]), API_GetErrorLineNumber(&NIMs[unit]) );
        api_error = API_GetLastError( &NIMs[unit] );
        trace_new( TL_ERROR, "Error %d, %s\n", api_error, API_GetErrorMessage(&NIMs[unit], (APIERRNO)api_error) );
        if ( callbacks[unit] )
        {
            parm.parm.type = DEMOD_FAILED;
            callbacks[unit]( my_module, unit, DEMOD_CONNECT_STATUS, &parm );
        }
    }
}

/*****************************************************************************/
/*  FUNCTION:    cobra_disconnect_msg                                        */
/*                                                                           */
/*  PARAMETERS:  unit - the unit specified in the DISCONNECT message.        */
/*                                                                           */
/*  DESCRIPTION: This function implements the processing required on receipt */
/*               of a DISCONNECT message.                                    */
/*                                                                           */
/*  RETURNS:     Nothing.                                                    */
/*                                                                           */
/*  CONTEXT:     Will be run in task context.                                */
/*                                                                           */
/*****************************************************************************/
static void cobra_disconnect_msg( u_int32 unit )
{
    DEMOD_CALLBACK_DATA parm;

    trace_new( TL_FUNC, "New disconnect message for unit %d\n", unit );

    API_SetInterruptOptions( &NIMs[unit], COBRA_NO_INTERRUPTS );
    local_state[unit] = DISCONNECTED;

    if ( callbacks[unit] )
    {
        parm.parm.type = DEMOD_DISCONNECTED;
        callbacks[unit]( my_module, unit, DEMOD_DISCONNECT_STATUS, &parm );
    }
}

/*****************************************************************************/
/*  FUNCTION:    cobra_scan_msg                                              */
/*                                                                           */
/*  PARAMETERS:  unit - the unit specified in the SCAN message.              */
/*               scan - the scanning specification in the SCAN messsage.     */
/*                                                                           */
/*  DESCRIPTION: This function implements the processing required on receipt */
/*               of a SCAN message.                                          */
/*                                                                           */
/*  RETURNS:     Nothing.                                                    */
/*                                                                           */
/*  CONTEXT:     Will be run in task context.                                */
/*                                                                           */
/*****************************************************************************/
static void cobra_scan_msg( u_int32 unit, SCAN_SPEC *scan )
{
    CHANOBJ chan;
    int api_error;
    DEMOD_CALLBACK_DATA parm;
    BOOL APIresult;
    int rc;
    int ii;
    CMPLXNO esno;
    MSTATUS status;

    trace_new( TL_FUNC, "New scan message for unit %d\n", unit );

    /* Change state to scanning, and remember the parameters for the operation. */
    local_state[unit] = SCANNING;
    local_scan[unit] = *scan;

    /* Set the current frequency to the initial frequency, and set the current
       indices for symbol rate and polarity searches. */
    local_scan[unit].current.type = DEMOD_NIM_SATELLITE;
    scan_parms[unit].current_frequency = scan->scan.satellite.start_frequency;
    local_scan[unit].current.tune.nim_satellite_tune.frequency = scan_parms[unit].current_frequency;
    scan_parms[unit].current_symbol_rate = 0;
    local_scan[unit].current.tune.nim_satellite_tune.fec = scan->scan.satellite.fecs[0];
    scan_parms[unit].current_polarity = 0;
    local_scan[unit].current.tune.nim_satellite_tune.polarisation = scan->scan.satellite.polarizations[0];

    /* Make a local copy of the symbol rate search list. */
    local_scan[unit].scan.satellite.symbol_rates = &local_symrates[unit][0];
    local_scan[unit].current.tune.nim_satellite_tune.symbol_rate = scan->scan.satellite.symbol_rates[0];
    for ( ii=0; ii<scan->scan.satellite.num_symrates; ++ii )
    {
        local_symrates[unit][ii] = scan->scan.satellite.symbol_rates[ii];
    }

    /* Make a local copy of the polarizations search list. */
    local_scan[unit].scan.satellite.polarizations = &local_polarizations[unit][0];
    local_scan[unit].current.tune.nim_satellite_tune.polarisation = scan->scan.satellite.polarizations[0];
    for ( ii=0; ii<scan->scan.satellite.num_pols; ++ii )
    {
        local_polarizations[unit][ii] = scan->scan.satellite.polarizations[ii];
    }

    /* Form the viterbi search mask from the viterbi search list. */
    local_viterbis[unit] = 0;
    for ( ii=0; ii<scan->scan.satellite.num_fecs; ++ii )
    {
        local_viterbis[unit] |= xlat_fec_in( scan->scan.satellite.fecs[ii] );
    }

    local_tuning[unit] = local_scan[unit].current;

    cnxt_set_lnb( &NIMs[unit], &local_scan[unit].current, &chan.frequency );
    chan.modtype = MOD_QPSK;
    chan.coderate = xlat_fec_in( local_scan[unit].current.tune.nim_satellite_tune.fec );
    chan.symbrate = local_scan[unit].current.tune.nim_satellite_tune.symbol_rate;
    chan.specinv = SPEC_INV_BOTH; /* Fix me! wjj */
    chan.samplerate = SAMPLE_FREQ_NOM;
    chan.viterbicoderates = local_viterbis[unit];

    /* Make sure all interrupts (including acq_fail) are reset, then call
       API_ChangeChannel to accomplish the tuning. */
    API_AcqBegin( &NIMs[unit] );
    API_ClearPendingInterrupts( &NIMs[unit] );
    acq_failure_countdown[unit] = DEMOD_SCAN_COUNTDOWN;
    APIresult = API_ChangeChannel( &NIMs[unit], &chan );

    if ( APIresult )
    {
        trace_new( TL_VERBOSE, "API_ChangeChannel succeeded for unit %d\n", unit );

        /* Start the demod monitoring signal quality. */
        if ( !API_GetChannelEsNo( &NIMs[0], ESNOMODE_SNAPSHOT, &esno, &status ) )
        {
            trace_new( TL_ERROR, "API_GetChannelEsNo failed\n" );
            trace_new( TL_ERROR, "File: %s, line: %d\n", API_GetErrorFilename(&NIMs[0]), API_GetErrorLineNumber(&NIMs[0]) );
            api_error = API_GetLastError( &NIMs[0] );
            trace_new( TL_ERROR, "Error %d, %s\n", api_error, API_GetErrorMessage(&NIMs[0], (APIERRNO)api_error) );
        }
    
        rc = tick_set( hCobraTick[unit], COBRA_SCANNING_TIMEOUT, TRUE );
        if ( rc != RC_OK )
        {
            trace_new( TL_ERROR, "Cobra can't set tick timer for connect message!\n" );
            error_log( ERROR_FATAL | RC_SDM_SYSERR );
        }
        else
        {
            rc = tick_start( hCobraTick[unit] );
            if ( rc != RC_OK )
            {
                trace_new( TL_ERROR, "Cobra can't start tick timer for connect message!\n" );
                error_log( ERROR_FATAL | RC_SDM_SYSERR );
            }
        }
        API_SetInterruptOptions( &NIMs[unit], COBRA_CONNECTING_INTERRUPTS );
#if (INTERNAL_DEMOD == INTERNAL_COBRA_LIKE)
#else
        if ( RC_OK != int_enable( demod_interrupt ) )
        {
            trace_new( TL_ERROR, "Cobra can't enable NIM interrupt in cobra_scan_msg!\n" );
            error_log( ERROR_FATAL | RC_SDM_SYSERR );
        }
#endif
    }
    else
    {
        API_SetInterruptOptions( &NIMs[unit], COBRA_NO_INTERRUPTS );
        trace_new( TL_ERROR, "API_ChangeChannel failed for unit %d\n", unit );
        trace_new( TL_ERROR, "File: %s, line: %d\n", API_GetErrorFilename(&NIMs[unit]), API_GetErrorLineNumber(&NIMs[unit]) );
        api_error = API_GetLastError( &NIMs[unit] );
        trace_new( TL_ERROR, "Error %d, %s\n", api_error, API_GetErrorMessage(&NIMs[unit], (APIERRNO)api_error) );
        if ( callbacks[unit] )
        {
            parm.parm.type = DEMOD_SCAN_NO_SIGNAL;
            callbacks[unit]( my_module, unit, DEMOD_SCAN_STATUS, &parm );
        }
    }
}

/*****************************************************************************/
/*  FUNCTION:    cobra_task                                                  */
/*                                                                           */
/*  PARAMETERS:  parm - (unused)                                             */
/*                                                                           */
/*  DESCRIPTION: This function implements the acquisition task for the Cobra */
/*               demod.  The task runs forever, checking an input message    */
/*               queue for work to do.                                       */
/*                                                                           */
/*  RETURNS:     Never.                                                      */
/*                                                                           */
/*  CONTEXT:     Will be run in task context.                                */
/*                                                                           */
/*****************************************************************************/
static void cobra_task( void *parm )
{
    int rc, ii;
    u_int32 msg[4];
    u_int32 message, unit;
    u_int32 send_msg[4];

    while (1)
    {
        /* Check for new message, process the message received. */
#if (INTERNAL_DEMOD == INTERNAL_COBRA_LIKE)
        rc = qu_receive( cobra_message_q, 50, msg );
#else
        rc = qu_receive( cobra_message_q, 2000, msg );
#endif

      if (skyscan_flag == 0)
      {
        /* If the message receive timed out, monitor current state. */
        if ( rc == RC_KAL_TIMEOUT )
        {
#if (INTERNAL_DEMOD == INTERNAL_COBRA_LIKE)
            /* manufacture an interrupt message if there is an int pending */
            if ( SBRead( demod_handle[0], 0x33, &message ) & SBRead( demod_handle[0], 0x34, &message ) & 0x3f )
            {
                send_msg[0] = send_msg[1] = send_msg[2] = 0;
                send_msg[3] = COBRA_INTERRUPT;
                rc = qu_send( cobra_message_q, send_msg );
            }
#endif
            for ( ii=0; ii<local_unit_count; ++ii )
            {
                API_Monitor( &NIMs[ii], &NewAcqState[ii], &NewLock[ii] );
                if ( NewAcqState[ii] != OldAcqState[ii] )
                {
                    trace_new( TL_FUNC, "Unit %d, OldAcqState = %d, NewAcqState = %d\n", ii, OldAcqState[ii], NewAcqState[ii] );
                    OldAcqState[ii] = NewAcqState[ii];
                }
            }
        }
        else if (rc == RC_OK)
        {
            /* We have just successfully removed a message from the message
               queue.  Check to see if a previous attempt to place a timeout
               or interrupt message into the queue failed, and try to recover
               by queueing the message now.  Check interrupt first, then
               timeout, to bias us towards success. */
            if ( global_interrupt_flag )
            {
                send_msg[0] = send_msg[1] = send_msg[2] = 0;
                send_msg[3] = COBRA_INTERRUPT;
                rc = qu_send( cobra_message_q, send_msg );
                if (rc == RC_OK)
                {
                    global_interrupt_flag = 0;
                }
            }

            if ( global_timeout_flag )
            {
                unit = (global_timeout_flag & 1) ? 0 : 1;
                send_msg[0] = unit;
                send_msg[1] = send_msg[2] = 0;
                send_msg[3] = COBRA_TIMEOUT;
                rc = qu_send( cobra_message_q, send_msg );
                if (rc == RC_OK)
                {
                    global_timeout_flag ^= (global_timeout_flag & 1) ? 1 : 2;
                }
            }

            /* Process the incoming message. */
            unit = msg[0];
            message = msg[3];
            switch( message )
            {
                case COBRA_TIMEOUT:
                    cobra_timeout_msg( unit );
                    break;

                case COBRA_INTERRUPT:
                    cobra_interrupt_msg();
                    break;

                case COBRA_CONNECT:
                    cobra_connect_msg( unit, (TUNING_SPEC *)msg[1] );
                    break;

                case COBRA_DISCONNECT:
                    cobra_disconnect_msg( unit );
                    break;

                case COBRA_SCAN:
                    cobra_scan_msg( unit, (SCAN_SPEC *)msg[1] );
                    break;

                default:
                    trace_new( TL_ERROR, "New message %d for unit %d\n", message, unit );
                    break;
            }
        }
        else
        {
            trace_new( TL_ERROR, "Strange error 0x%x dequeueing a message in cobra_task\n", rc );
        }
      } /* endif (skyscan_flag == 0) */
    }
}

/*****************************************************************************/
/*  FUNCTION:    cobra_tick_callback                                         */
/*                                                                           */
/*  PARAMETERS:  hTick - the handle of the tick timer that expired.          */
/*               pUser - the user-specified parameter for the timeout.       */
/*                                                                           */
/*  DESCRIPTION: This function handles tick timer callbacks.                 */
/*                                                                           */
/*  RETURNS:     Nothing.                                                    */
/*                                                                           */
/*  CONTEXT:     Will be called from an interrupt context.                   */
/*                                                                           */
/*****************************************************************************/
void cobra_tick_callback( tick_id_t hTick, void *pUser )
{
    u_int32 unit = (u_int32)pUser;
    u_int32 msg[4];
    int return_code;

    isr_trace_new( TL_FUNC, "Cobra demod got timeout callback\n", 0, 0 );

    /* Send a control message to the Cobra demod task to signal the timeout. */
    msg[0] = unit;
    msg[1] = msg[2] = 0;
    msg[3] = COBRA_TIMEOUT;
    return_code = qu_send( cobra_message_q, msg );
    if (return_code != RC_OK)
    {
        trace_new( TL_ERROR, "Cobra can't send message in tick callback!\n" );
        error_log( ERROR_WARNING | RC_SDM_QFULL );
        global_timeout_flag |= (unit == 0) ? 1 : 2;
    }
}

/*****************************************************************************/
/*  FUNCTION:    cobra_isr                                                   */
/*                                                                           */
/*  PARAMETERS:  int_ID - the ID of the interrupt being handled.             */
/*               isFIQ - flag indicating FIQ or not.                         */
/*               previous - function pointer to be filled in if interrupt    */
/*                   chaining is required.                                   */
/*                                                                           */
/*  DESCRIPTION: This function handles interrupts from the Cobra demod.      */
/*                                                                           */
/*  RETURNS:     RC_ISR_HANDLED - Interrupt fully handled by this routine.   */
/*               RC_ISR_NOTHANDLED - Interrupt not handled by this function. */
/*                   (KAL should chain to the function whose pointer is      */
/*                   stored in pfnChain).                                    */
/*                                                                           */
/*  CONTEXT:     Will be called from an interrupt context.                   */
/*                                                                           */
/*****************************************************************************/
int cobra_isr( u_int32 int_ID, bool isFIQ, void *previous )
{
    u_int32 msg[4];
    int return_code;

    isr_trace_new( TL_FUNC, "Cobra demod got interrupt 0x%08x\n", int_ID, 0 );

    /* Send a control message to the Cobra demod task to check status. */
    msg[0] = msg[1] = msg[2] = 0;
    msg[3] = COBRA_INTERRUPT;
    return_code = qu_send( cobra_message_q, msg );
    if (return_code != RC_OK)
    {
        trace_new( TL_ERROR, "Cobra can't send message in ISR!\n" );
        error_log( ERROR_WARNING | RC_SDM_QFULL );
        global_interrupt_flag = 1;
    }

    /* Make sure that there will be no more interrupts from the Cobra demod
       until this one is fully handled (cleared).  Since the demod is hung
       off of the IIC bus, we cannot talk to it from here, must globally
       disable the GPIO interrupt. */
    int_disable( demod_interrupt );
    return RC_ISR_HANDLED;
}

/*****************************************************************************/
/*  FUNCTION:    cnxt_demod_get_unit_type                                    */
/*                                                                           */
/*  PARAMETERS:  unit - the unit number for which type information is        */
/*                   requested.                                              */
/*               unit_type - pointer to the DEMOD_NIM_TYPE variable to be    */
/*                   filled out.                                             */
/*                                                                           */
/*  DESCRIPTION: This function returns information about the type of the     */
/*               unit that is the subject of the request.                    */
/*                                                                           */
/*  RETURNS:     DEMOD_UNINITIALIZED - this module has not been initialized. */
/*               DEMOD_BAD_UNIT - the unit number is not valid.              */
/*               DEMOD_BAD_PARAMETER - the unit_type parameter is NULL.      */
/*               DEMOD_SUCCESS - the function completed successfully.        */
/*                                                                           */
/*  CONTEXT:     Must be called from a non-interrupt context.                */
/*                                                                           */
/*****************************************************************************/
static DEMOD_STATUS cnxt_demod_get_unit_type( u_int32 unit, DEMOD_NIM_TYPE *unit_type )
{
    trace_new( TL_FUNC, "Entered get_unit_type for Cobra unit %d\n", unit );

    if ( !initialized )
    {
        trace_new( TL_ERROR, "Cobra demod has not been initialized\n" );
        return DEMOD_UNINITIALIZED;
    }
    
    if ( unit >= local_unit_count )
    {
        trace_new( TL_ERROR, "Cobra demod received bad unit number %d\n", unit );
        return DEMOD_BAD_UNIT;
    }
    
    if ( 0 == unit_type )
    {
        trace_new( TL_ERROR, "Cobra demod received bad unit_type parameter\n" );
        return DEMOD_BAD_PARAMETER;
    }

    *unit_type = DEMOD_NIM_SATELLITE;
    return DEMOD_SUCCESS;
}

/*****************************************************************************/
/*  FUNCTION:    cnxt_demod_ioctl                                            */
/*                                                                           */
/*  PARAMETERS:  unit - the unit number for which the ioctl operation is     */
/*                   requested.                                              */
/*               type - the type of ioctl operation requested.               */
/*               data - pointer to the DEMOD_IOCTL_TYPE structure that       */
/*                   contains data for the operation or will be filled out   */
/*                   with data by the operation.                             */
/*                                                                           */
/*  DESCRIPTION: This function implements various module-specific control or */
/*               status functions.                                           */
/*                                                                           */
/*  RETURNS:     DEMOD_UNINITIALIZED - this module has not been initialized. */
/*               DEMOD_BAD_UNIT - the unit number is not valid.              */
/*               DEMOD_BAD_COMMAND - the command specified is not legal.     */
/*               DEMOD_BAD_PARAMETER - the data parameter is NULL.           */
/*               DEMOD_SUCCESS - the function completed successfully.        */
/*                                                                           */
/*  CONTEXT:     Must be called from a non-interrupt context.                */
/*                                                                           */
/*****************************************************************************/
static DEMOD_STATUS cnxt_demod_ioctl( u_int32 unit, DEMOD_IOCTL_TYPE type, void *data )
{
    DEMOD_STATUS   dsRet;
    LNBBURST       lbBurstType;
    DISEQC_MESSAGE *dmDiseqcMessage;
    AUTO_FECS      *afFECs;

    trace_new( TL_FUNC, "Entered ioctl for Cobra unit %d\n", unit );

    if ( !initialized )
    {
        trace_new( TL_ERROR, "Cobra demod has not been initialized\n" );
        return DEMOD_UNINITIALIZED;
    }
    
    if ( unit >= local_unit_count )
    {
        trace_new( TL_ERROR, "Cobra demod received bad unit number %d\n", unit );
        return DEMOD_BAD_UNIT;
    }

    switch(type)
    {
        case SET_LNB:
            if ( 0 == data )
            {
                trace_new( TL_ERROR, "Cobra demod received bad data parameter for SET_LNB\n" );
                dsRet = DEMOD_BAD_PARAMETER;
            }
            else
            {
                dsRet = cnxt_lnb_set_parameters( (LNB_SETTINGS *)data );
            }
        break;

        case SEND_DISEQC_MESSAGE:
            if ( 0 == data )
            {
                trace_new( TL_ERROR, "Cobra demod received bad data parameter for the DiSEcQ message to be sent!\n" );
                dsRet = DEMOD_BAD_PARAMETER;
            }
            else
            {   /* We have a pointer to valid data */
                dmDiseqcMessage = (DISEQC_MESSAGE *)data;

                /* Convert the burst type to the format that the demod requires */
                switch(dmDiseqcMessage->btBurstType)
                {
                    case BURST_TYPE_MODULATED:
                        lbBurstType = LNBBURST_MODULATED;
                    break;
                    case BURST_TYPE_UNMODULATED:
                        lbBurstType = LNBBURST_UNMODULATED;
                    break;
                    default:
                        lbBurstType = LNBBURST_UNDEF;
                    break;
                }
                    
                /* Send the message */                      
                if (!API_SendDiseqcMessage(&NIMs[unit],
                                             (unsigned char *)dmDiseqcMessage->pMessage,
                                             (unsigned char)dmDiseqcMessage->ui8MessageLength,
                                             (BOOL)dmDiseqcMessage->bLastMessage,
                                             lbBurstType)
                   )
                {
                    trace_new( TL_ERROR, "Cobra demod received bad data parameter for the DiSEcQ message to be sent!\n" );
                    dsRet = DEMOD_BAD_PARAMETER;
                }
                else
                {
                    dsRet = DEMOD_SUCCESS;  
                }

            }
        break;
        
        #ifdef INCLUDE_DISEQC2
        case RECEIVE_DISEQC_MESSAGE:
            dmDiseqcMessage = (DISEQC_MESSAGE *)data;
            if ( 0 == data )
            {
               trace_new( TL_ERROR, "Cobra demod received bad data parameter for the DiSEcQ message to be received!\n" );
               dsRet = DEMOD_BAD_PARAMETER;
            }
            else
            {
               if( !API_DiseqcReceiveMessage(&NIMs[unit],
                                             dmDiseqcMessage->pMessage,
                                             (int)(dmDiseqcMessage->bufferlen),
                                             (int *)(&dmDiseqcMessage->receivedlen),
                                             (RXMODE)(dmDiseqcMessage->rxmode),
                                             (int *)(&dmDiseqcMessage->parityerr)) )
               {
                  trace_new( TL_ERROR, "Cobra demod received bad data parameter for the DiSEcQ message to be received!\n" );
                  dsRet = DEMOD_BAD_PARAMETER;
               }
               else
               {
                  dsRet = DEMOD_SUCCESS;
               }
            } /* endif (0 == data) */
        break;
        #endif
        
        case SET_AUTO_FECS:
            if ( 0 == data )
            {
                trace_new( TL_ERROR, "Cobra demod received bad data parameter for the FEC rates\n" );
                dsRet = DEMOD_BAD_PARAMETER;
            }
            else
            {
                /* Save the pointer to the data */
                afFECs = (AUTO_FECS *)data;

                /* Set the fecs variable */
                if(afFECs->coderate_2div3 == TRUE)
                {
                    viterbicoderates = CODERATE_2DIV3;
                }
                else
                {
                    viterbicoderates = 0;
                }

                if(afFECs->coderate_3div4 == TRUE)
                {
                    viterbicoderates |= CODERATE_3DIV4;
                }

                if(afFECs->coderate_4div5 == TRUE)
                {
                    viterbicoderates |= CODERATE_4DIV5;
                }

                if(afFECs->coderate_5div6 == TRUE)
                {
                    viterbicoderates |= CODERATE_5DIV6;
                }

                if(afFECs->coderate_6div7 == TRUE)
                {
                    viterbicoderates |= CODERATE_6DIV7;
                }

                if(afFECs->coderate_7div8 == TRUE)
                {
                    viterbicoderates |= CODERATE_7DIV8;
                }
                
                /* check that its not zero */
                if( viterbicoderates != 0 )
                {                                
                    dsRet = DEMOD_SUCCESS;
                }
                else
                {
                    trace_new( TL_ERROR, "Cobra demod received bad data parameter for the FEC rates\n" );               
                    dsRet = DEMOD_BAD_PARAMETER;
                }
            }
        break;

        case SET_LNB_OUTPUT_ENABLE:
            if ( 0 == data )
            {
                trace_new( TL_ERROR, "Cobra demod received bad data parameter for SET_LNB_OUTPUT_ENABLE\n" );
                dsRet = DEMOD_BAD_PARAMETER;
            }
            else
            {
                dsRet = cnxt_lnb_set_output_enable( &NIMs[0], *(bool *)data );
            }
            break;

        case GET_LNB_OUTPUT_ENABLE:
            if ( 0 == data )
            {
                trace_new( TL_ERROR, "Cobra demod received bad data parameter for GET_LNB_OUTPUT_ENABLE\n" );
                dsRet = DEMOD_BAD_PARAMETER;
            }
            else
            {
                dsRet = cnxt_lnb_get_output_enable( &NIMs[0], (bool *)data );
            }
            break;

        case SET_LNB_POLARIZATION:
            if ( 0 == data )
            {
                trace_new( TL_ERROR, "Cobra demod received bad data parameter for SET_LNB_POLARIZATION\n" );
                dsRet = DEMOD_BAD_PARAMETER;
            }
            else
            {
                dsRet = cnxt_lnb_set_polarization( &NIMs[0], *(NIM_SATELLITE_POLARISATION *)data );
            }
            break;

        case GET_LNB_POLARIZATION:
            if ( 0 == data )
            {
                trace_new( TL_ERROR, "Cobra demod received bad data parameter for GET_LNB_POLARIZATION\n" );
                dsRet = DEMOD_BAD_PARAMETER;
            }
            else
            {
                dsRet = cnxt_lnb_get_polarization( &NIMs[0], (NIM_SATELLITE_POLARISATION *)data );
            }
            break;

        case SET_LNB_TONE:
            if ( 0 == data )
            {
                trace_new( TL_ERROR, "Cobra demod received bad data parameter for SET_LNB_TONE\n" );
                dsRet = DEMOD_BAD_PARAMETER;
            }
            else
            {
                dsRet = cnxt_lnb_set_tone_enable( &NIMs[0], *(bool *)data );
            }
            break;

        case GET_LNB_TONE:
            if ( 0 == data )
            {
                trace_new( TL_ERROR, "Cobra demod received bad data parameter for GET_LNB_TONE\n" );
                dsRet = DEMOD_BAD_PARAMETER;
            }
            else
            {
                dsRet = cnxt_lnb_get_tone_enable( &NIMs[0], (bool *)data );
            }
            break;

        default:
            trace_new( TL_ERROR, "Cobra demod received bad ioctl type parameter %d\n", (int)(type) );
            dsRet = DEMOD_BAD_COMMAND;
        break;

    }

    return dsRet; 
}

/*****************************************************************************/
/*  FUNCTION:    cnxt_demod_connect                                          */
/*                                                                           */
/*  PARAMETERS:  unit - the unit number for which the connect operation is   */
/*                   requested.                                              */
/*               tuning - pointer to the TUNING_SPEC structure containing    */
/*                   parameters for the requested connection.                */
/*                                                                           */
/*  DESCRIPTION: This function connects to a stream by tuning the interface  */
/*               to the requested specification.                             */
/*                                                                           */
/*  RETURNS:     DEMOD_UNINITIALIZED - this module has not been initialized. */
/*               DEMOD_BAD_UNIT - the unit number is not valid.              */
/*               DEMOD_BAD_PARAMETER - the tuning or connection_limit        */
/*                   parameter is invalid.                                   */
/*               DEMOD_IS_BUSY - the demod unit is busy with another         */
/*                   operation.                                              */
/*               DEMOD_ERROR - there was an error during an OS call.         */
/*               DEMOD_SUCCESS - the function completed successfully.        */
/*                                                                           */
/*  CONTEXT:     Must be called from a non-interrupt context.                */
/*                                                                           */
/*****************************************************************************/
static DEMOD_STATUS cnxt_demod_connect( u_int32 unit, TUNING_SPEC *tuning, u_int32 *connection_limit )
{
    u_int32 msg[4];
    int return_code;

    trace_new( TL_FUNC, "Entered connect for Cobra unit %d\n", unit );

    if ( !initialized )
    {
        trace_new( TL_ERROR, "Cobra demod has not been initialized\n" );
        return DEMOD_UNINITIALIZED;
    }
    
    if ( unit >= local_unit_count )
    {
        trace_new( TL_ERROR, "Cobra demod received bad unit number %d\n", unit );
        return DEMOD_BAD_UNIT;
    }

    if ( (0 == tuning) || (tuning->type != DEMOD_NIM_SATELLITE) )
    {
        trace_new( TL_ERROR, "Cobra demod received bad tuning parameter\n" );
        return DEMOD_BAD_PARAMETER;
    }

    if ( 0 == connection_limit )
    {
        trace_new( TL_ERROR, "Cobra demod received bad connection_limit parameter\n" );
        return DEMOD_BAD_PARAMETER;
    }

/*    if ( local_state[unit] != DISCONNECTED )
    {
        trace_new( TL_ERROR, "Cobra demod cannot connect when not disconnected\n" );
        return DEMOD_IS_BUSY;
    }*/

    local_tuning[unit] = *tuning;

    /* Send a control message to the Cobra demod task to connect. */
    msg[0] = unit;
    msg[1] = (u_int32)&local_tuning[unit];
    msg[2] = 0;
    msg[3] = COBRA_CONNECT;
    return_code = qu_send( cobra_message_q, msg );
    if ( return_code != RC_OK )
    {
        trace_new( TL_ERROR, "Cobra can't send message in demod connect!\n" );
        error_log( ERROR_WARNING | RC_SDM_QFULL );
        return DEMOD_ERROR;
    }

    return DEMOD_SUCCESS;
}

/*****************************************************************************/
/*  FUNCTION:    cnxt_demod_disconnect                                       */
/*                                                                           */
/*  PARAMETERS:  unit - the unit number for which the disconnect operation   */
/*                   is requested.                                           */
/*                                                                           */
/*  DESCRIPTION: This function disconnects from a stream.                    */
/*                                                                           */
/*  RETURNS:     DEMOD_UNINITIALIZED - this module has not been initialized. */
/*               DEMOD_BAD_UNIT - the unit number is not valid.              */
/*               DEMOD_ERROR - there was an error during an OS call.         */
/*               DEMOD_SUCCESS - the function completed successfully.        */
/*                                                                           */
/*  CONTEXT:     Must be called from a non-interrupt context.                */
/*                                                                           */
/*****************************************************************************/
static DEMOD_STATUS cnxt_demod_disconnect( u_int32 unit )
{
    u_int32 msg[4];
    int return_code;

    trace_new( TL_FUNC, "Entered disconnect for Cobra unit %d\n", unit );

    if ( !initialized )
    {
        trace_new( TL_ERROR, "Cobra demod has not been initialized\n" );
        return DEMOD_UNINITIALIZED;
    }
    
    if ( unit >= local_unit_count )
    {
        trace_new( TL_ERROR, "Cobra demod received bad unit number %d\n", unit );
        return DEMOD_BAD_UNIT;
    }

    /* Send a control message to the Cobra demod task to disconnect. */
    msg[0] = unit;
    msg[1] = 0;
    msg[2] = 0;
    msg[3] = COBRA_DISCONNECT;
    return_code = qu_send( cobra_message_q, msg );
    if ( return_code != RC_OK )
    {
        trace_new( TL_ERROR, "Cobra can't send message in demod disconnect!\n" );
        error_log( ERROR_WARNING | RC_SDM_QFULL );
        return DEMOD_ERROR;
    }

    return DEMOD_SUCCESS;
}

/*****************************************************************************/
/*  FUNCTION:    cnxt_demod_scan                                             */
/*                                                                           */
/*  PARAMETERS:  unit - the unit number for which the scan operation is      */
/*                   requested.                                              */
/*               scanning - pointer to the SCAN_SPEC structure that contains */
/*                   parameters for the scan operation.                      */
/*                                                                           */
/*  DESCRIPTION: This function causes the specified unit to scan for a       */
/*               signal with the specified scanning parameters.              */
/*                                                                           */
/*  RETURNS:     DEMOD_UNINITIALIZED - this module has not been initialized. */
/*               DEMOD_BAD_UNIT - the unit number is not valid.              */
/*               DEMOD_BAD_PARAMETER - the scan parameter is invalid.        */
/*               DEMOD_IS_BUSY - the demod unit is busy with another         */
/*                   operation.                                              */
/*               DEMOD_ERROR - there was an error during an OS call.         */
/*               DEMOD_SUCCESS - the function completed successfully.        */
/*                                                                           */
/*  CONTEXT:     Must be called from a non-interrupt context.                */
/*                                                                           */
/*****************************************************************************/
static DEMOD_STATUS cnxt_demod_scan( u_int32 unit, SCAN_SPEC *scan )
{
    u_int32 msg[4];
    int return_code;

    trace_new( TL_FUNC, "Entered scan for Cobra unit %d\n", unit );

    if ( !initialized )
    {
        trace_new( TL_ERROR, "Cobra demod has not been initialized\n" );
        return DEMOD_UNINITIALIZED;
    }
    
    if ( unit >= local_unit_count )
    {
        trace_new( TL_ERROR, "Cobra demod received bad unit number %d\n", unit );
        return DEMOD_BAD_UNIT;
    }

    if ( (0 == scan) || (scan->type != DEMOD_NIM_SATELLITE) )
    {
        trace_new( TL_ERROR, "Cobra demod received bad scanning parameter\n" );
        return DEMOD_BAD_PARAMETER;
    }

    /* Send a control message to the Cobra demod task to scan. */
    msg[0] = unit;
    msg[1] = (u_int32)scan;
    msg[2] = 0;
    msg[3] = COBRA_SCAN;
    return_code = qu_send( cobra_message_q, msg );
    if ( return_code != RC_OK )
    {
        trace_new( TL_ERROR, "Cobra can't send message in demod scan!\n" );
        error_log( ERROR_WARNING | RC_SDM_QFULL );
        return DEMOD_ERROR;
    }

    return DEMOD_SUCCESS;
}

/*****************************************************************************/
/*  FUNCTION:    cnxt_demod_get_tuning                                       */
/*                                                                           */
/*  PARAMETERS:  unit - the unit number for which the get tuning operation   */
/*                   is requested.                                           */
/*               tuning - pointer to the TUNING_SPEC structure to be filled  */
/*                   out with parameters for the current connection.         */
/*                                                                           */
/*  DESCRIPTION: This function returns the tuning parameters for the         */
/*               specified unit.                                             */
/*                                                                           */
/*  RETURNS:     DEMOD_UNINITIALIZED - this module has not been initialized. */
/*               DEMOD_BAD_UNIT - the unit number is not valid.              */
/*               DEMOD_BAD_PARAMETER - the tuning parameter is invalid.      */
/*               DEMOD_SUCCESS - the function completed successfully.        */
/*                                                                           */
/*  CONTEXT:     Must be called from a non-interrupt context.                */
/*                                                                           */
/*****************************************************************************/
static DEMOD_STATUS cnxt_demod_get_tuning( u_int32 unit, TUNING_SPEC *tuning )
{
    trace_new( TL_FUNC, "Entered get tuning for Cobra unit %d\n", unit );

    if ( !initialized )
    {
        trace_new( TL_ERROR, "Cobra demod has not been initialized\n" );
        return DEMOD_UNINITIALIZED;
    }
    
    if ( unit >= local_unit_count )
    {
        trace_new( TL_ERROR, "Cobra demod received bad unit number %d\n", unit );
        return DEMOD_BAD_UNIT;
    }

    if ( 0 == tuning )
    {
        trace_new( TL_ERROR, "Cobra demod received bad tuning parameter\n" );
        return DEMOD_BAD_PARAMETER;
    }

    *tuning = local_tuning[unit]; /*maybe some actual info?*/
    return DEMOD_SUCCESS;
}

/*****************************************************************************/
/*  FUNCTION:    cnxt_demod_set_callback                                     */
/*                                                                           */
/*  PARAMETERS:  unit - the unit number for which the callback is to be set. */
/*               callback - the function address to be used for the callback */
/*                   for this unit.                                          */
/*                                                                           */
/*  DESCRIPTION: This function sets the callback pointer for the specified   */
/*               unit.                                                       */
/*                                                                           */
/*  RETURNS:     DEMOD_UNINITIALIZED - this module has not been initialized. */
/*               DEMOD_BAD_UNIT - the unit number is not valid.              */
/*               DEMOD_BAD_PARAMETER - the callback parameter is invalid.    */
/*               DEMOD_SUCCESS - the function completed successfully.        */
/*                                                                           */
/*  CONTEXT:     Must be called from a non-interrupt context.                */
/*                                                                           */
/*****************************************************************************/
static DEMOD_STATUS cnxt_demod_set_callback( u_int32 unit, MODULE_STATUS_FUNCTION *callback )
{
    trace_new( TL_FUNC, "Entered set callback for Cobra unit %d\n", unit );

    if ( !initialized )
    {
        trace_new( TL_ERROR, "Cobra demod has not been initialized\n" );
        return DEMOD_UNINITIALIZED;
    }
    
    if ( unit >= local_unit_count )
    {
        trace_new( TL_ERROR, "Cobra demod received bad unit number %d\n", unit );
        return DEMOD_BAD_UNIT;
    }

    if ( 0 == callback )
    {
        trace_new( TL_ERROR, "Cobra demod received bad callback parameter\n" );
        return DEMOD_BAD_PARAMETER;
    }

    callbacks[unit] = callback;
    return DEMOD_SUCCESS;
}

/*****************************************************************************/
/*  FUNCTION:    cnxt_demod_clear_callback                                   */
/*                                                                           */
/*  PARAMETERS:  unit - the unit number for which the callback is to be      */
/*                   cleared.                                                */
/*                                                                           */
/*  DESCRIPTION: This function clears the callback pointer for the specified */
/*               unit.                                                       */
/*                                                                           */
/*  RETURNS:     DEMOD_UNINITIALIZED - this module has not been initialized. */
/*               DEMOD_BAD_UNIT - the unit number is not valid.              */
/*               DEMOD_SUCCESS - the function completed successfully.        */
/*                                                                           */
/*  CONTEXT:     Must be called from a non-interrupt context.                */
/*                                                                           */
/*****************************************************************************/
static DEMOD_STATUS cnxt_demod_clear_callback( u_int32 unit )
{
    trace_new( TL_FUNC, "Entered clear callback for Cobra unit %d\n", unit );

    if ( !initialized )
    {
        trace_new( TL_ERROR, "Cobra demod has not been initialized\n" );
        return DEMOD_UNINITIALIZED;
    }
    
    if ( unit >= local_unit_count )
    {
        trace_new( TL_ERROR, "Cobra demod received bad unit number %d\n", unit );
        return DEMOD_BAD_UNIT;
    }

    callbacks[unit] = 0;
    return DEMOD_SUCCESS;
}

/*****************************************************************************/
/*  FUNCTION:    cnxt_demod_get_signal_stats                                 */
/*                                                                           */
/*  PARAMETERS:  unit - the unit number for which the get signal stats       */
/*                   operation is requested.                                 */
/*               signal_stats - pointer to the SIGNAL_STATS structure to be  */
/*                   filled out with parameters for the specified unit.      */
/*                                                                           */
/*  DESCRIPTION: This function returns the signal statistics for the         */
/*               specified unit.                                             */
/*                                                                           */
/*  RETURNS:     DEMOD_UNINITIALIZED - this module has not been initialized. */
/*               DEMOD_BAD_UNIT - the unit number is not valid.              */
/*               DEMOD_BAD_PARAMETER - the signal_stats parameter is         */
/*                   invalid.                                                */
/*               DEMOD_UNIMPLEMENTED - this function is not implemented.     */
/*                                                                           */
/*  CONTEXT:     Must be called from a non-interrupt context.                */
/*                                                                           */
/*****************************************************************************/
static DEMOD_STATUS cnxt_demod_get_signal_stats( u_int32 unit, SIGNAL_STATS *signal_stats )
{
    AGCACC agcacc;
    CMPLXNO esno;
    MSTATUS status;
    int api_error;
    #define NUMBER_OF_RETRYS_TO_READ_BER (20)
#ifdef SKIP_BER 
#else   
    u_int8 i = NUMBER_OF_RETRYS_TO_READ_BER;
    CMPLXNO ber;
#endif    
    trace_new( TL_FUNC, "Entered get signal stats for Cobra unit %d\n", unit );

    if ( !initialized )
    {
        trace_new( TL_ERROR, "Cobra demod has not been initialized\n" );
        return DEMOD_UNINITIALIZED;
    }
    
    if ( unit >= local_unit_count )
    {
        trace_new( TL_ERROR, "Cobra demod received bad unit number %d\n", unit );
        return DEMOD_BAD_UNIT;
    }

    if ( 0 == signal_stats )
    {
        trace_new( TL_ERROR, "Cobra demod received bad signal_stats parameter\n" );
        return DEMOD_BAD_PARAMETER;
    }

    signal_stats->type = DEMOD_NIM_SATELLITE;

    if ( local_state[unit] == CONNECTED )
    {
        /* Get the power level from the AGC accumulator */
        if ( !API_GetAGCAcc( &NIMs[unit], &agcacc ) )
        {
            trace_new( TL_ERROR, "API_GetAGCAcc failed\n" );
            trace_new( TL_ERROR, "File: %s, line: %d\n", API_GetErrorFilename(&NIMs[unit]), API_GetErrorLineNumber(&NIMs[unit]) );
            api_error = API_GetLastError( &NIMs[unit] );
            trace_new( TL_ERROR, "Error %d, %s\n", api_error, API_GetErrorMessage(&NIMs[unit], (APIERRNO)api_error) );
            return DEMOD_ERROR;
        }
        else
        {
            /* AGCACC is signed, from -128 to 127.  Convert to 0 to 255. */
            signal_stats->stats.s_signal.signal_strength  = 128 + agcacc;
        }
        
        /* Get the quality from the error rate. */
        if ( !API_GetChannelEsNo( &NIMs[unit], ESNOMODE_SNAPSHOT, &esno, &status ) )
        {
            trace_new( TL_ERROR, "API_GetChannelEsNo failed\n" );
            trace_new( TL_ERROR, "File: %s, line: %d\n", API_GetErrorFilename(&NIMs[unit]), API_GetErrorLineNumber(&NIMs[unit]) );
            api_error = API_GetLastError( &NIMs[unit] );
            trace_new( TL_ERROR, "Error %d, %s\n", api_error, API_GetErrorMessage(&NIMs[unit], (APIERRNO)api_error) );
            return DEMOD_ERROR;
        }
        else
        {
            /* If measurement not done, report an error. */
            if ( status == MSTATUS_NOTDONE )
            {
#ifdef SKIP_BER
#else                
                /* esno ranges from 2.0 to 20.0.  Convert to 0 to 255. */
                signal_stats->stats.s_signal.signal_quality = (((esno.integer-20)*256)/181);
#endif                
                return DEMOD_ERROR;
            }
            
            /* esno ranges from 2.0 to 20.0.  Convert to 0 to 255. */
            signal_stats->stats.s_signal.signal_quality = (((esno.integer-20)*256)/181);
        }
#ifdef SKIP_BER
#else
        /* Get the BER. */
        if ( !API_GetBER(&NIMs[unit],BER_WINDOW_SIZE,&ber,&status) )
        {
            trace_new( TL_ERROR, "API_GetChannelEsNo failed\n" );
            trace_new( TL_ERROR, "File: %s, line: %d\n", API_GetErrorFilename(&NIMs[unit]), API_GetErrorLineNumber(&NIMs[unit]) );
            api_error = API_GetLastError( &NIMs[unit] );
            trace_new( TL_ERROR, "Error %d, %s\n", api_error, API_GetErrorMessage(&NIMs[unit], (APIERRNO)api_error) );
            return DEMOD_ERROR;
        }
        else
        {       
            /* If the measurement is not doe try a few more times. */
            for(i = NUMBER_OF_RETRYS_TO_READ_BER;(i != 0) && (status == MSTATUS_NOTDONE);i--)
            {
                task_time_sleep(100);
                API_GetBER(&NIMs[unit],BER_WINDOW_SIZE,&ber,&status);
            }

            /* If measurement not done by now, report zero BER. */
            if ( status == MSTATUS_NOTDONE )
            {
                signal_stats->stats.s_signal.signal_ber_int = 0;
                signal_stats->stats.s_signal.signal_ber_div = 1;
            }
            else
            {
                /* The measurement is done. */
                signal_stats->stats.s_signal.signal_ber_int = ber.integer;
                signal_stats->stats.s_signal.signal_ber_div = ber.divider;
            }
        }
#endif        
    }
    else
    {
        /* Get the power level from the AGC accumulator */
        if ( !API_GetAGCAcc( &NIMs[unit], &agcacc ) )
        {
            trace_new( TL_ERROR, "API_GetAGCAcc failed\n" );
            trace_new( TL_ERROR, "File: %s, line: %d\n", API_GetErrorFilename(&NIMs[unit]), API_GetErrorLineNumber(&NIMs[unit]) );
            api_error = API_GetLastError( &NIMs[unit] );
            trace_new( TL_ERROR, "Error %d, %s\n", api_error, API_GetErrorMessage(&NIMs[unit], (APIERRNO)api_error) );
            return DEMOD_ERROR;
        }
        else
        {
            /* AGCACC is signed, from -128 to 127.  Convert to 0 to 255. */
            signal_stats->stats.s_signal.signal_strength  = 128 + agcacc;
        }
        signal_stats->stats.s_signal.signal_quality  = 0;
#ifdef SKIP_BER  
#else      
        signal_stats->stats.s_signal.signal_strength = 0;
#endif        
        return DEMOD_NOT_LOCKED;
    }

    return DEMOD_SUCCESS;
}

/*****************************************************************************/
/*  FUNCTION:    cnxt_demod_get_lock_status                                  */
/*                                                                           */
/*  PARAMETERS:  unit - the unit number for which the get lock status        */
/*                   operation is requested.                                 */
/*               locked - pointer to the boolean to be filled out indicating */
/*                   locked/not locked for the specified unit.               */
/*                                                                           */
/*  DESCRIPTION: This function returns the lock status for the specified     */
/*               unit.                                                       */
/*                                                                           */
/*  RETURNS:     DEMOD_UNINITIALIZED - this module has not been initialized. */
/*               DEMOD_BAD_UNIT - the unit number is not valid.              */
/*               DEMOD_BAD_PARAMETER - the locked parameter is invalid.      */
/*               DEMOD_SUCCESS - the function completed successfully.        */
/*                                                                           */
/*  CONTEXT:     Must be called from a non-interrupt context.                */
/*                                                                           */
/*****************************************************************************/
static DEMOD_STATUS cnxt_demod_get_lock_status( u_int32 unit, bool *locked )
{
    trace_new( TL_FUNC, "Entered get lock status for Cobra unit %d\n", unit );

    if ( !initialized )
    {
        trace_new( TL_ERROR, "Cobra demod has not been initialized\n" );
        return DEMOD_UNINITIALIZED;
    }
    
    if ( unit >= local_unit_count )
    {
        trace_new( TL_ERROR, "Cobra demod received bad unit number %d\n", unit );
        return DEMOD_BAD_UNIT;
    }

    if ( 0 == locked )
    {
        trace_new( TL_ERROR, "Cobra demod received bad locked parameter\n" );
        return DEMOD_BAD_PARAMETER;
    }

    *locked = (local_state[unit] == CONNECTED);
    return DEMOD_SUCCESS;
}

/*****************************************************************************/
/*  FUNCTION:    cnxt_cobra_demod_init                                       */
/*                                                                           */
/*  PARAMETERS:  module - the module designation to be used by this driver   */
/*                   in subsequent callbacks.                                */
/*               number_units - pointer to an integer to be filled out with  */
/*                   the total number of units in this module.               */
/*               function_table - pointer to the DEMOD_FTABLE structure to   */
/*                   be filled out with function pointers for this module.   */
/*                                                                           */
/*  DESCRIPTION: This function initializes the module and returns a count of */
/*               units in the module and the function table for module       */
/*               functions.                                                  */
/*                                                                           */
/*  RETURNS:     DEMOD_INITIALIZED - this module has already been            */
/*                   initialized.                                            */
/*               DEMOD_BAD_PARAMETER - there is a bad parameter.             */
/*               DEMOD_ERROR - there was an error during an OS call.         */
/*               DEMOD_SUCCESS - the function completed successfully.        */
/*                                                                           */
/*  CONTEXT:     Must be called from a non-interrupt context.                */
/*                                                                           */
/*****************************************************************************/
DEMOD_STATUS cnxt_cobra_demod_init( u_int32 module, u_int32 *number_units, DEMOD_FTABLE *function_table )
{
    int ii;
    BOOL demod_retval;
    task_id_t pid;
    int api_error;
    char tickname[5] = "CBTn";
    LNBMODE lnbmode;
    u_int8 byBoardCode, byVendorCode;
    TRANSPEC transpec;
#if (INTERNAL_DEMOD == INTERNAL_COBRA_LIKE)
    u_int32 stat;
#endif

    trace_new( TL_FUNC, "Entered init for Cobra module %d\n", module );

    if ( initialized )
    {
        trace_new( TL_ERROR, "Cobra demod has already been initialized\n" );
        return DEMOD_INITIALIZED;
    }

    if ( 0 == number_units )
    {
        trace_new( TL_ERROR, "Cobra demod received bad number_units parameter\n" );
        return DEMOD_BAD_PARAMETER;
    }

    if ( 0 == function_table )
    {
        trace_new( TL_ERROR, "Cobra demod received bad function_table parameter\n" );
        return DEMOD_BAD_PARAMETER;
    }

    /* Find the NIM for which to initialize the interface.  Look first for a
       NIM connected via IIC, then for an onboard demod if no IIC demod found. */
#if I2C_BUS_CX24121 != I2C_BUS_NONE
    if ( iicAddressTest( I2C_ADDR_CX24121, I2C_BUS_CX24121, 0 ) )
    {
        demod_handle[0] = I2C_ADDR_CX24121;
        demod_handle[1] = 0x10000 + I2C_ADDR_CX24121;
        demod_access_method = DEMOD_IIC;
        demod_iic_bus = I2C_BUS_CX24121;
        trace_new( TL_FUNC, "Cobra demod: found device present at address 0x%02x\n", I2C_ADDR_CX24121 );
    }
    else if ( iicAddressTest( I2C_ADDR_CX24121_DSS, I2C_BUS_CX24121, 0 ) )
    {
        demod_handle[0] = I2C_ADDR_CX24121_DSS;
        demod_handle[1] = 0x10000 + I2C_ADDR_CX24121_DSS;
        demod_access_method = DEMOD_IIC;
        demod_iic_bus = I2C_BUS_CX24121;
        trace_new( TL_FUNC, "Cobra demod: found device present at address 0x%02x\n", I2C_ADDR_CX24121_DSS );
    }
    else
#endif
#if (INTERNAL_DEMOD == INTERNAL_COBRA_LIKE)
    {
        /* If there is an onboard demod, make sure it has not been disabled
           before trying to use it. */
        if ( CNXT_GET( PLL_TEST_REG, PLL_TEST_DEMOD_DISABLE_MASK ) ==
                PLL_TEST_DEMOD_DISABLED )
        {
            return DEMOD_NO_HARDWARE;
        }

        /* Set up the access method for the internal demod based on the
           configuration settings. */
        #if ( (CAMARO_ACCESS_METHOD == REGISTER_ONLY) ||           \
                ((CAMARO_ACCESS_METHOD == REGISTER_WORKAROUND) &&  \
                        (INTERNAL_DEMOD_HAS_ASX_BUG == NO)) )

            CNXT_SET( PLL_PAD_FAST_CTRL_REG, PLL_FAST_CTRL_DEMOD_CONNECT_SEL_MASK,
                    PLL_FAST_CTRL_DEMOD_CONNECT_ASX );
            CNXT_SET( PLL_PAD_FAST_CTRL_REG, PLL_FAST_CTRL_DEMOD_I2C_SEL_MASK,
                    PLL_FAST_CTRL_DEMOD_I2C_BUS_NONE );
            demod_handle[0] = 0;
            demod_handle[1] = 0x10000;
            demod_access_method = DEMOD_REGISTER;
            trace_new( TL_FUNC, "Cobra demod: using internal device via ASX bus\n" );

        #elif (CAMARO_ACCESS_METHOD == I2C_BUS_ONLY)

            CNXT_SET( PLL_PAD_FAST_CTRL_REG, PLL_FAST_CTRL_DEMOD_CONNECT_SEL_MASK,
                    PLL_FAST_CTRL_DEMOD_CONNECT_I2C );
            #if I2C_BUS_CAMARO == I2C_BUS_1
                CNXT_SET( PLL_PAD_FAST_CTRL_REG, PLL_FAST_CTRL_DEMOD_I2C_SEL_MASK,
                        PLL_FAST_CTRL_DEMOD_I2C_BUS_1 );
            #else
                CNXT_SET( PLL_PAD_FAST_CTRL_REG, PLL_FAST_CTRL_DEMOD_I2C_SEL_MASK,
                        PLL_FAST_CTRL_DEMOD_I2C_BUS_0 );
            #endif
            demod_handle[0] = I2C_ADDR_CAMARO;
            demod_handle[1] = 0x10000 | I2C_ADDR_CAMARO;
            demod_iic_bus = I2C_BUS_CAMARO;
            demod_access_method = DEMOD_IIC;
            trace_new( TL_FUNC, "Cobra demod: using internal device via IIC bus %d at address 0x%02x\n", I2C_BUS_CAMARO, I2C_ADDR_CAMARO );

        #else

            CNXT_SET( PLL_PAD_FAST_CTRL_REG, PLL_FAST_CTRL_DEMOD_CONNECT_SEL_MASK,
                    PLL_FAST_CTRL_DEMOD_CONNECT_ASX );
            #if I2C_BUS_CAMARO == I2C_BUS_1
                CNXT_SET( PLL_PAD_FAST_CTRL_REG, PLL_FAST_CTRL_DEMOD_I2C_SEL_MASK,
                        PLL_FAST_CTRL_DEMOD_I2C_BUS_1 );
            #else
                CNXT_SET( PLL_PAD_FAST_CTRL_REG, PLL_FAST_CTRL_DEMOD_I2C_SEL_MASK,
                        PLL_FAST_CTRL_DEMOD_I2C_BUS_0 );
            #endif
            demod_handle[0] = I2C_ADDR_CAMARO;
            demod_handle[1] = 0x10000 | I2C_ADDR_CAMARO;
            demod_iic_bus = I2C_BUS_CAMARO;
            demod_access_method = DEMOD_HYBRID;
            trace_new( TL_FUNC, "Cobra demod: using internal device via ASX and IIC bus %d at address 0x%02x\n", I2C_BUS_CAMARO, I2C_ADDR_CAMARO );

        #endif

        /* workaround to try to keep Camaro powered up and clocked on Brazos A */
        SBWrite( demod_handle[0], 0xff, 0x39, &stat );
        SBWrite( demod_handle[0], 0xef, 0x00, &stat );
        SBWrite( demod_handle[0], 0x67, 0x80, &stat );
        task_time_sleep(1000);
    }
#else
    {
        return DEMOD_NO_HARDWARE;
    }
#endif

    /* Configure output mode. */
    /* For Klondike (Colorado-based) IRDs, use the board ID to determine the
       NIM connection type.  Otherwise, use parallel. */
    read_board_and_vendor_codes( &byBoardCode, &byVendorCode );
    if ( (byBoardCode & KLONDIKE_MASK) == BOARD_KLONDIKE )
    {
        if ( byBoardCode & KLONDIKE_PARALLEL_NIM )
        {
            trace_new( TL_FUNC, "Configure for parallel attached NIM\n" );
            MPEG.OutputMode = PARALLEL_OUT;
            MPEG.ClkOutEdge = CLKOUT_SETUP3_HOLD5;
        }
        else
        {
            trace_new( TL_FUNC, "Configure for serial attached NIM\n" );
            MPEG.OutputMode = SERIAL_OUT;
            MPEG.ClkOutEdge = CLKOUT_DATALR_DATACF;
        }
    }
    else
    {
        trace_new( TL_FUNC, "Configure for parallel attached NIM\n" );
        MPEG.OutputMode = PARALLEL_OUT;
        MPEG.ClkOutEdge = CLKOUT_SETUP3_HOLD5;
    }

    MPEG.ClkParityMode = CLK_CONTINUOUS;
    MPEG.HoldTime = SMALL_HOLD_TIME;
    MPEG.StartSignalPolarity = ACTIVE_HIGH;
    MPEG.StartSignalWidth = BYTE_WIDE;
    MPEG.ValidSignalPolarity = ACTIVE_HIGH;
    MPEG.ValidSignalActiveMode = ENTIRE_PACKET;
    MPEG.FailSignalPolarity = ACTIVE_HIGH;
    MPEG.FailSignalActiveMode = FIRST_BYTE;
    MPEG.SyncPunctMode = SYNC_WORD_NOT_PUNCTURED;
    MPEG.FailValueWhenNoSync = FAIL_HIGH_WHEN_NO_SYNC;
    MPEG.ClkSmoothSel = CLK_SMOOTHING_OFF;
    MPEG.RSCntlPin1Sel = RS_CNTLPIN_START; 
    MPEG.RSCntlPin2Sel = RS_CNTLPIN_VALID;
    MPEG.RSCntlPin3Sel = RS_CNTLPIN_FAIL; 
    MPEG.NullDataMode = FIXED_NULL_DATA_DISABLED; 
    MPEG.NullDataValue = FIXED_NULL_DATA_LOW; 
    MPEG.ValidSignalWhenFail = VALID_SIGNAL_INACTIVE_WHEN_FAIL; 
    MPEG.StartSignalWhenFail = START_SIGNAL_INACTIVE_WHEN_FAIL; 
    MPEG.ParityDataSel = RS_PARITY_DATA_UNCHANGED; 

#if (LNB_22KHZ_CONTROL == LNB_22KHZ_TONE)
    lnbmode.tone_clock = 115;
    lnbmode.cycle_count = 0;
    lnbmode.lnb_mode = LNBMODE_TONE;
#else
    lnbmode.tone_clock = 0;
    lnbmode.cycle_count = 0;
    lnbmode.lnb_mode = LNBMODE_MANUAL_ZERO;
#endif

    /* Check for a special case here.  An external NIM at address 0xEA is
       configured for DSS and must be connected via a parallel interface. */
    if ( (demod_handle[0] == I2C_ADDR_CX24121_DSS) && (MPEG.OutputMode != PARALLEL_OUT) )
    {
        trace_new( TL_ERROR, "Cobra demod: Error, DSS configured NIM via serial interface.\n" );
        return DEMOD_ERROR;
    }

    /* Set the transport specification.  If the demod is on-board, it can do
       either DVB or DSS automagically.  If the demod is not on-board, set it
       to do only DVB. */
#if (INTERNAL_DEMOD == INTERNAL_COBRA_LIKE)
    transpec = SPEC_DVB_DSS;
#else
    transpec = SPEC_DVB;
#endif

#if (INTERNAL_DEMOD == INTERNAL_COBRA_LIKE)
    /* perform an internal soft reset */
    SBWrite( demod_handle[0], 0, 0xff, &stat );
    SBWrite( demod_handle[0], 0, 0, &stat );
#endif

    /* do the low level initialization */
    demod_retval = API_InitEnvironment( &NIMs[0], demod_handle[0], SBWrite, SBRead, transpec, 0, 10111000, True, &MPEG, &lnbmode, &NIM_Wait );
    if ( !demod_retval )
    {
        trace_new( TL_ERROR, "Cobra demod failed low level initialization of unit 0.\n" );
        trace_new( TL_ERROR, "File: %s, line: %d\n", API_GetErrorFilename(&NIMs[0]), API_GetErrorLineNumber(&NIMs[0]) );
        api_error = API_GetLastError( &NIMs[0] );
        trace_new( TL_ERROR, "Error %d, %s\n", api_error, API_GetErrorMessage(&NIMs[0], (APIERRNO)api_error) );
        return DEMOD_ERROR;
    }

    /* initialization for DiSEqC 2.x */
#ifdef INCLUDE_DISEQC2
    demod_retval = API_DiseqcSetVersion( &NIMs[0], DISEQC_VER_2X);
    if ( !demod_retval )
    {
        return DEMOD_ERROR;
    }
#endif

#if (INTERNAL_DEMOD == INTERNAL_COBRA_LIKE)
    /* workaround to try to keep Camaro powered up and clocked on Brazos A */
    SBWrite( demod_handle[0], 0xff, 0x39, &stat );
    SBWrite( demod_handle[0], 0xef, 0x00, &stat );
    SBWrite( demod_handle[0], 0x67, 0x80, &stat );
    task_time_sleep(1000);
#endif

#if (INTERNAL_DEMOD == INTERNAL_COBRA_LIKE)
    /* API_InitEnvironment calls API_SetOutputIO to set up MPEG output control
       register 3.  Unfortunately, API_SetOutputIO has not yet been updated to
       understand that Camaro has 3 RS_CNTL pins, versus only 2 for external
       demods.  Write directly to the register to initialize all three signals.
       When API_SetOutputIO is updated, change to use it. */
    SBWrite( demod_handle[0], 0x06, 0x21, &stat );
#endif

    cnxt_lnb_init( &NIMs[0] );

    /* Get the local unit count by checking hardware type; initialize second
       unit if present. */
    if ( NIMs[0].demod_type == CX24130 )
    {
        local_unit_count = 2;
        demod_retval = API_InitEnvironment( &NIMs[1], demod_handle[1], SBWrite, SBRead, SPEC_DVB, 0, 10111000, True, &MPEG, 0, 0 );
        if ( !demod_retval )
        {
            trace_new( TL_ERROR, "Cobra demod failed low level initialization of unit 1.\n" );
            trace_new( TL_ERROR, "File: %s, line: %d\n", API_GetErrorFilename(&NIMs[1]), API_GetErrorLineNumber(&NIMs[1]) );
            api_error = API_GetLastError( &NIMs[1] );
            trace_new( TL_ERROR, "Error %d, %s\n", api_error, API_GetErrorMessage(&NIMs[1], (APIERRNO)api_error) );
            return DEMOD_ERROR;
        }
        #ifdef INCLUDE_DISEQC2
        demod_retval = API_DiseqcSetVersion( &NIMs[1], DISEQC_VER_2X);
        if ( !demod_retval )
        {
            return DEMOD_ERROR;
        }
        #endif
    }
    else
    {
        local_unit_count = 1;
    }
    *number_units = local_unit_count;

    initialized = 1;
    my_module = module;

    /* Initialize some per-unit structures. */
    for ( ii=0; ii<MAXIMUM_NUMBER_UNITS; ++ii )
    {
        local_state[ii] = DISCONNECTED;
        local_tuning[ii].type = DEMOD_NIM_SATELLITE;
        NewAcqState[ii] = OldAcqState[ii] = ACQ_OFF;

        /* Create the tick timer we will use to timeout state transitions. */
        tickname[3] = '0' + ii;
        hCobraTick[ii] = tick_create( cobra_tick_callback, (void *)ii, tickname );
        if ( 0 == hCobraTick[ii] )
        {
            trace_new( TL_ERROR, "Cobra demod can't create tick timer for unit %d!\n", ii );
            error_log( ERROR_FATAL | RC_SDM_SYSERR );
            return DEMOD_ERROR;
        }
    }

    /* Fill out the function table. */
    function_table->unit_type = cnxt_demod_get_unit_type;
    function_table->ioctl = cnxt_demod_ioctl;
    function_table->connect = cnxt_demod_connect;
    function_table->disconnect = cnxt_demod_disconnect;
    function_table->scan = cnxt_demod_scan;
    function_table->get_tuning = cnxt_demod_get_tuning;
    function_table->set_callback = cnxt_demod_set_callback;
    function_table->clear_callback = cnxt_demod_clear_callback;
    function_table->get_signal = cnxt_demod_get_signal_stats;
    function_table->get_lock = cnxt_demod_get_lock_status;

    /* Create the Cobra demod message queue. */
    cobra_message_q = qu_create( MAX_COBRA_MSGS, "COBRA" );
    if ( cobra_message_q == (queue_id_t)0 )
    {
        trace_new( TL_ERROR, "Cobra demod can't create message queue!\n" );
        error_log( ERROR_FATAL | RC_SDM_QCREATE );
        return DEMOD_ERROR;
    }

    /* Create the Cobra NIM acquisition task. */
    pid = task_create( cobra_task, NULL, NULL, ACQT_TASK_STACK_SIZE, ACQT_TASK_PRIORITY, ACQT_TASK_NAME );
    if ( pid == 0 )
    {
        trace_new( TL_ERROR, "Cobra demod can't create task!\n" );
        error_log( ERROR_FATAL | RC_SDM_PCREATE );
        return DEMOD_ERROR;
    }

    /* Set the Cobra demod interrupt options to no interrupts. */
    API_SetInterruptOptions( &NIMs[0], COBRA_NO_INTERRUPTS );
    
    /* Try to clear any currently pending Cobra demod interrupts. */
    API_ClearPendingInterrupts( &NIMs[0] );

    /* For external NIMs, the interrupt line is on a GPIO, for internal demod,
       the interrupt line is in the interrupt register. */
#if (INTERNAL_DEMOD == INTERNAL_COBRA_LIKE)
    demod_interrupt = INT_QPSK_DEMOD;
#else
    /* Claim the pin and set it up for input, interrupt on rising edge. */
    demod_interrupt = COBRA_NIM_INT;
    MAKE_GPIO_INPUT_BANK( COBRA_INTERRUPT_BANK, COBRA_INTERRUPT_BIT );
    SET_GPIO_INT_EDGE_BANK( COBRA_INTERRUPT_BANK, COBRA_INTERRUPT_BIT, NEG_EDGE );
#endif

    /* Register the interrupt for the Cobra demod. */
    if ( RC_OK != int_register_isr( demod_interrupt, cobra_isr, 0, 0, &chained_isr ) )
    {
        trace_new( TL_ERROR, "Cobra can't register interrupt handler!\n" );
        error_log( ERROR_FATAL | RC_SDM_SYSERR );
        return DEMOD_ERROR;
    }

#if (INTERNAL_DEMOD == INTERNAL_COBRA_LIKE)
#else
    /* enable the interrupt at pic level */
    clear_pic_interrupt( PIC_FROM_INT_ID(demod_interrupt), INT_FROM_INT_ID(demod_interrupt) );
    if ( RC_OK != int_enable( demod_interrupt ) )
    {
        trace_new( TL_ERROR, "Cobra can't enable NIM interrupt!\n" );
        error_log( ERROR_FATAL | RC_SDM_SYSERR );
        return DEMOD_ERROR;
    }
#endif

    return DEMOD_SUCCESS;
}


/*
 * These two functions are used to support the satellite scan-sky function.
 */

DEMOD_STATUS cnxt_cobra_borrow_nim (u_int32 unit, NIM **pNim)
{
   if ( !initialized )
   {
      trace_new( TL_ERROR, "Cobra demod has not been initialized\n" );
      return DEMOD_ERROR;
   }
   
   skyscan_flag = 1;
   *pNim = &NIMs[unit];
   return DEMOD_SUCCESS;
}

DEMOD_STATUS cnxt_cobra_return_nim (void)
{
   skyscan_flag = 0;
   return DEMOD_SUCCESS;
}


 /****************************************************************************
 * Modifications:
 *
 * $Log: 
 *  37   mpeg      1.36        7/1/04 3:52:14 AM      Steven Shen     CR(s) 
 *        9631 9632 : Move the API_DiseqcSetVersion function into this file 
 *        from cobra_api.c.
 *  36   mpeg      1.35        6/30/04 3:50:10 AM     Steven Shen     CR(s) 
 *        9624 9625 : Add the support of receiving DiSEqC message and changing 
 *        the scan-sky status.
 *  35   mpeg      1.34        5/6/04 12:24:28 PM     Dave Wilson     CR(s) 
 *        8640 8641 : Fixed a race condition where the variable local_state was
 *         being set incorrectly if disconnect and connect were called back to 
 *        back. This variable must only ever be set in the context of the 
 *        cobra_task. Previous version set it outside this task in 
 *        cnxt_demod_connect.
 *  34   mpeg      1.33        3/22/04 1:15:48 PM     Billy Jackman   CR(s) 
 *        8585 : Removed all code that directly performs LNB signalling 
 *        functionality and moved it to lnb.c.
 *        Added spectral inversion specification to tuning structure to support
 *         NDSCORE.
 *        Modified satellite statistics structure to report BER as an integer 
 *        and divisor instead of a real to support NDSCORE.
 *        Added ioctl functions to support set/get of LNB signal enable, 
 *        set/get of LNB polarization, and set/get of LNB tone enable to 
 *        support NDSCORE.
 *        
 *  33   mpeg      1.32        3/17/04 1:37:40 PM     Billy Jackman   CR(s) 
 *        8581 8582 : Modified code to check for the presence of the ASX bus 
 *        Camaro access bug and implement a hybrid ASX bus/IIC bus access 
 *        method when needed.
 *  32   mpeg      1.31        1/26/04 12:27:07 PM    Billy Jackman   CR(s) 
 *        5111 8267 : Added code to support specifying that the Camaro demod on
 *         Brazos should be accessed via IIC bus instead of the ASX register 
 *        bus.  The code checks the values of I2C_BUS_CAMARO and 
 *        I2C_ADDR_CAMARO to determine the IIC bus and address to use to 
 *        communicate with Camaro.
 *  31   mpeg      1.30        11/7/03 5:22:10 AM     Ian Mitchell    CR(s): 
 *        7865 7866 In the connect function if the FEC is set to AUTO then the 
 *        cobra hardware searches all FEC's.
 *        Extra IOCTL functionality to change the FEC's automatically searched 
 *        for this, the default is all supported FEC's.
 *        
 *  30   mpeg      1.29        10/3/03 12:34:02 PM    Billy Jackman   SCR(s) 
 *        7579 :
 *        Make sure to fill out the tuning type member of tuning structure to 
 *        be
 *        reported back.
 *        
 *  29   mpeg      1.28        10/3/03 9:21:10 AM     Billy Jackman   SCR(s) 
 *        7579 :
 *        Slight modification to scanning to keep track of current tuning 
 *        parameters.
 *        
 *  28   mpeg      1.27        8/15/03 1:49:52 PM     Billy Jackman   SCR(s) 
 *        7280 7281 :
 *        Added code to set a direction control for LNB 22KHz signalling if the
 *         key
 *        PIO_LNB_22KHZ_DIRECTION is specified as a PIO pin.
 *        
 *  27   mpeg      1.26        8/5/03 6:36:46 AM      Ian Mitchell    SCR(s): 
 *        7153 7155 
 *        Fix spelling for diseqc.
 *        
 *  26   mpeg      1.25        7/31/03 1:38:46 PM     Billy Jackman   SCR(s) 
 *        5894 5895 :
 *        Modified code to allow transitions involving signal fade and recovery
 *         to work
 *        correctly and to send the right callback notifications.
 *        
 *  25   mpeg      1.24        7/11/03 12:48:42 PM    Ian Mitchell    SCR(s): 
 *        6896 
 *        Populate the new signal_ber element of the satellite statistics 
 *        structure
 *        in the statistics function.
 *        Add DiSEqC control to the IOCTL function.
 *        
 *  24   mpeg      1.23        7/7/03 4:52:26 PM      Billy Jackman   SCR(s) 
 *        6882 :
 *        Got rid of another useless debug message.
 *        
 *  23   mpeg      1.22        7/3/03 1:03:38 PM      Billy Jackman   SCR(s) 
 *        6882 6883 6884 6885 :
 *        Changed debug trace information to not repeat unchanged states.
 *        Changed polling timeout in interrupt workaround code from 500 to 50 
 *        ms.
 *        Changed default condition to be more scanning to do in 
 *        cobra_timeout_message.
 *        Removed a check for status disconnected in cnxt_demod_scan.
 *        
 *  22   mpeg      1.21        6/4/03 4:33:06 PM      Billy Jackman   SCR(s) 
 *        6712 :
 *        Check the PLL_TEST_REG to determine if the onboard demod has been 
 *        disabled
 *        before trying to use it.
 *        
 *  21   mpeg      1.20        5/14/03 2:13:18 PM     Billy Jackman   SCR(s) 
 *        6336 6337 :
 *        Added function NIM_Wait to be called from the San Diego driver code 
 *        to
 *        accomplish a delay of the specified number of milliseconds.
 *        Removed specification of transpec in the channel object.  Latest 
 *        driver code
 *        does not require this in order to autodetect DVB/DSS.
 *        Modified specification of the MPEG output interface to match changes 
 *        in San
 *        Diego code.
 *        Added NIM_Wait as a parameter to API_Init_Environment to specify the 
 *        delay
 *        function to be used by San Diego code.
 *        Removed obsolete function waitabit previously used for a fixed delay 
 *        from
 *        San Diego code.
 *        
 *  20   mpeg      1.19        4/23/03 11:00:56 AM    Billy Jackman   SCR(s) 
 *        6075 6076 :
 *        Added code to issue a soft reset command to the onboard demod before 
 *        calling
 *        the initialization API for the demod.
 *        
 *  19   mpeg      1.18        3/21/03 4:53:06 PM     Billy Jackman   SCR(s) 
 *        5490 5823 :
 *        Added an internal state for signal fade instead of going disconnected
 *         to
 *        handle loss of signal.
 *        
 *  18   mpeg      1.17        3/17/03 4:47:22 PM     Billy Jackman   SCR(s) 
 *        5793 :
 *        Use test for LNB_22KHZ_CONTROL == LNB_22KHZ_ENABLE to determine how 
 *        to handle
 *        LNB 22KHz signalling enable so that the proper control is generated 
 *        on Bronco
 *        as well as Klondike IRDs.
 *        
 *  17   mpeg      1.16        2/20/03 1:56:46 PM     Billy Jackman   SCR(s) 
 *        5143 :
 *        Changes to scan_message function to correctly set up current scan 
 *        state to
 *        allow programming of the demod to be correct for the requested 
 *        parameters.
 *        
 *  16   mpeg      1.15        2/13/03 1:32:22 PM     Billy Jackman   SCR(s) 
 *        5077 :
 *        Changes to allow use of the SPEC_DVB_DSS transport specification on 
 *        Camaro
 *        to allow tuning to either DVB or DSS dynamically.
 *        
 *  15   mpeg      1.14        2/11/03 3:09:48 PM     Billy Jackman   SCR(s) 
 *        5458 :
 *        Make sure to set up RS_CNTL2 for Brazos.
 *        Assign handles even for register access of internal demod.
 *        Use handle when calling SBWrite.
 *        
 *  14   mpeg      1.13        2/11/03 2:44:20 PM     Billy Jackman   SCR(s) 
 *        5143 :
 *        Changed use of DEMOD_BUSY to DEMOD_IS_BUSY to avoid conflict with 
 *        OpenTV
 *        header file demod.h.
 *        
 *  13   mpeg      1.12        2/4/03 8:23:26 AM      Billy Jackman   SCR(s) 
 *        5391 :
 *        Make address testing on an IIC bus conditional on that IIC bus being 
 *        defined
 *        to avoid bad bus number to the address test.
 *        
 *  12   mpeg      1.11        2/4/03 8:01:58 AM      Billy Jackman   SCR(s) 
 *        5143 :
 *        Changed parameters to some callbacks to be consistent across 
 *        satellite demods.
 *        
 *  11   mpeg      1.10        1/29/03 10:38:34 AM    Billy Jackman   SCR(s) 
 *        5343 :
 *        Include gpio.h to avoid compiler warning.
 *        
 *  10   mpeg      1.9         1/28/03 3:04:24 PM     Billy Jackman   SCR(s) 
 *        5095 :
 *        Use the right call to set the LNB enable so PIO expander doesn't get 
 *        trashed.
 *        
 *  9    mpeg      1.8         1/28/03 11:46:04 AM    Billy Jackman   SCR(s) 
 *        5095 :
 *        Include some workarounds for Rev A Brazos internal demod.
 *        Make the driver use polling vs. interrupts for Rev A Brazos.
 *        
 *  8    mpeg      1.7         1/17/03 5:32:24 PM     Billy Jackman   SCR(s) 
 *        5095 :
 *        Modified interrupt handling to use the internal interrupt for builtin
 *         demod.
 *        
 *  7    mpeg      1.6         1/15/03 5:19:22 PM     Billy Jackman   SCR(s) 
 *        5095 :
 *        Removed un-needed function clock().
 *        Added support for register based access in addition to IIC access.
 *        Changed demod detection to include checking for onboard demod.
 *        
 *  6    mpeg      1.5         12/13/02 4:14:42 PM    Billy Jackman   SCR(s) 
 *        5155 :
 *        Implemented signal level and quality.
 *        
 *  5    mpeg      1.4         12/3/02 3:11:00 PM     Billy Jackman   SCR(s) 
 *        5055 :
 *        Fix compiler warnings under VxWorks.
 *        
 *  4    mpeg      1.3         11/27/02 3:52:02 PM    Billy Jackman   SCR(s) 
 *        5019 :
 *        Use new standard I2C address/bus designations.
 *        
 *  3    mpeg      1.2         11/27/02 1:21:50 PM    Billy Jackman   SCR(s) 
 *        4989 4977 :
 *        Modified LNB handling to correctly set outputs for Klondike LNB.
 *        Removed critical sections enclosing I2C operations.
 *        Commented out code that prohibited a connect operation if the current
 *         state
 *        was not disconnected; this needs some more thought.
 *        Added I2C bus test for device presence in order to be able to 
 *        autoconfigure
 *        for satellite demods that are present.
 *        Added code to handle serial vs. parallel NIM connection on Klondike.
 *        
 *  2    mpeg      1.1         11/8/02 2:03:08 PM     Billy Jackman   SCR(s) 
 *        4928 :
 *        Changed I2C address (demod_handle) to 0x6A to match Nimrod board.
 *        
 *  1    mpeg      1.0         9/30/02 12:15:10 PM    Billy Jackman   
 * $
 * 
 *    Rev 1.29   03 Oct 2003 11:34:02   jackmaw
 * SCR(s) 7579 :
 * Make sure to fill out the tuning type member of tuning structure to be
 * reported back.
 * 
 *    Rev 1.28   03 Oct 2003 08:21:10   jackmaw
 * SCR(s) 7579 :
 * Slight modification to scanning to keep track of current tuning parameters.
 * 
 *    Rev 1.27   15 Aug 2003 12:49:52   jackmaw
 * SCR(s) 7280 7281 :
 * Added code to set a direction control for LNB 22KHz signalling if the key
 * PIO_LNB_22KHZ_DIRECTION is specified as a PIO pin.
 * 
 *    Rev 1.26   05 Aug 2003 05:36:46   mitchei
 * SCR(s): 7153 7155 
 * Fix spelling for diseqc.
 * 
 *    Rev 1.25   31 Jul 2003 12:38:46   jackmaw
 * SCR(s) 5894 5895 :
 * Modified code to allow transitions involving signal fade and recovery to work
 * correctly and to send the right callback notifications.
 * 
 *    Rev 1.24   11 Jul 2003 11:48:42   mitchei
 * SCR(s): 6896 
 * Populate the new signal_ber element of the satellite statistics structure
 * in the statistics function.
 * Add DiSEqC control to the IOCTL function.
 * 
 *    Rev 1.23   07 Jul 2003 15:52:26   jackmaw
 * SCR(s) 6882 :
 * Got rid of another useless debug message.
 * 
 *    Rev 1.22   03 Jul 2003 12:03:38   jackmaw
 * SCR(s) 6882 6883 6884 6885 :
 * Changed debug trace information to not repeat unchanged states.
 * Changed polling timeout in interrupt workaround code from 500 to 50 ms.
 * Changed default condition to be more scanning to do in cobra_timeout_message.
 * Removed a check for status disconnected in cnxt_demod_scan.
 * 
 *    Rev 1.21   04 Jun 2003 15:33:06   jackmaw
 * SCR(s) 6712 :
 * Check the PLL_TEST_REG to determine if the onboard demod has been disabled
 * before trying to use it.
 * 
 *    Rev 1.20   14 May 2003 13:13:18   jackmaw
 * SCR(s) 6336 6337 :
 * Added function NIM_Wait to be called from the San Diego driver code to
 * accomplish a delay of the specified number of milliseconds.
 * Removed specification of transpec in the channel object.  Latest driver code
 * does not require this in order to autodetect DVB/DSS.
 * Modified specification of the MPEG output interface to match changes in San
 * Diego code.
 * Added NIM_Wait as a parameter to API_Init_Environment to specify the delay
 * function to be used by San Diego code.
 * Removed obsolete function waitabit previously used for a fixed delay from
 * San Diego code.
 * 
 *    Rev 1.19   23 Apr 2003 10:00:56   jackmaw
 * SCR(s) 6075 6076 :
 * Added code to issue a soft reset command to the onboard demod before calling
 * the initialization API for the demod.
 * 
 *    Rev 1.18   21 Mar 2003 16:53:06   jackmaw
 * SCR(s) 5490 5823 :
 * Added an internal state for signal fade instead of going disconnected to
 * handle loss of signal.
 * 
 *    Rev 1.17   17 Mar 2003 16:47:22   jackmaw
 * SCR(s) 5793 :
 * Use test for LNB_22KHZ_CONTROL == LNB_22KHZ_ENABLE to determine how to handle
 * LNB 22KHz signalling enable so that the proper control is generated on Bronco
 * as well as Klondike IRDs.
 * 
 *    Rev 1.16   20 Feb 2003 13:56:46   jackmaw
 * SCR(s) 5143 :
 * Changes to scan_message function to correctly set up current scan state to
 * allow programming of the demod to be correct for the requested parameters.
 * 
 *    Rev 1.15   13 Feb 2003 13:32:22   jackmaw
 * SCR(s) 5077 :
 * Changes to allow use of the SPEC_DVB_DSS transport specification on Camaro
 * to allow tuning to either DVB or DSS dynamically.
 * 
 *    Rev 1.14   11 Feb 2003 15:09:48   jackmaw
 * SCR(s) 5458 :
 * Make sure to set up RS_CNTL2 for Brazos.
 * Assign handles even for register access of internal demod.
 * Use handle when calling SBWrite.
 * 
 *    Rev 1.13   11 Feb 2003 14:44:20   jackmaw
 * SCR(s) 5143 :
 * Changed use of DEMOD_BUSY to DEMOD_IS_BUSY to avoid conflict with OpenTV
 * header file demod.h.
 * 
 *    Rev 1.12   04 Feb 2003 08:23:26   jackmaw
 * SCR(s) 5391 :
 * Make address testing on an IIC bus conditional on that IIC bus being defined
 * to avoid bad bus number to the address test.
 * 
 *    Rev 1.11   04 Feb 2003 08:01:58   jackmaw
 * SCR(s) 5143 :
 * Changed parameters to some callbacks to be consistent across satellite demods.
 * 
 *    Rev 1.10   29 Jan 2003 10:38:34   jackmaw
 * SCR(s) 5343 :
 * Include gpio.h to avoid compiler warning.
 * 
 *    Rev 1.9   28 Jan 2003 15:04:24   jackmaw
 * SCR(s) 5095 :
 * Use the right call to set the LNB enable so PIO expander doesn't get trashed.
 * 
 *    Rev 1.8   28 Jan 2003 11:46:04   jackmaw
 * SCR(s) 5095 :
 * Include some workarounds for Rev A Brazos internal demod.
 * Make the driver use polling vs. interrupts for Rev A Brazos.
 * 
 *    Rev 1.7   17 Jan 2003 17:32:24   jackmaw
 * SCR(s) 5095 :
 * Modified interrupt handling to use the internal interrupt for builtin demod.
 * 
 *    Rev 1.6   15 Jan 2003 17:19:22   jackmaw
 * SCR(s) 5095 :
 * Removed un-needed function clock().
 * Added support for register based access in addition to IIC access.
 * Changed demod detection to include checking for onboard demod.
 * 
 *    Rev 1.5   13 Dec 2002 16:14:42   jackmaw
 * SCR(s) 5155 :
 * Implemented signal level and quality.
 * 
 *    Rev 1.4   03 Dec 2002 15:11:00   jackmaw
 * SCR(s) 5055 :
 * Fix compiler warnings under VxWorks.
 * 
 *    Rev 1.3   27 Nov 2002 15:52:02   jackmaw
 * SCR(s) 5019 :
 * Use new standard I2C address/bus designations.
 * 
 *    Rev 1.2   27 Nov 2002 13:21:50   jackmaw
 * SCR(s) 4989 4977 :
 * Modified LNB handling to correctly set outputs for Klondike LNB.
 * Removed critical sections enclosing I2C operations.
 * Commented out code that prohibited a connect operation if the current state
 * was not disconnected; this needs some more thought.
 * Added I2C bus test for device presence in order to be able to autoconfigure
 * for satellite demods that are present.
 * Added code to handle serial vs. parallel NIM connection on Klondike.
 * 
 *    Rev 1.1   08 Nov 2002 14:03:08   jackmaw
 * SCR(s) 4928 :
 * Changed I2C address (demod_handle) to 0x6A to match Nimrod board.
 * 
 *    Rev 1.0   30 Sep 2002 11:15:10   jackmaw
 * SCR(s) 4714 :
 * Implementation of Cobra demod module
 *
 ****************************************************************************/

