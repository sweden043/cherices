/****************************************************************************/
/*                   CONEXANT PROPRIETARY AND CONFIDENTIAL                  */
/*                    Conexant Systems Inc. (c) 2002-2004                   */
/*                              Austin, TX                                  */
/*                         All Rights Reserved                              */
/****************************************************************************/
/*
 * Filename: lnb.c
 *
 * Description: This file contains the implementation of the driver support
 *              code to deal with setting the LNB for the Cobra family of
 *              demod chips (CX24121 and CX24130).
 *
 * Author: Billy Jackman
 *
 ****************************************************************************/
/*
 $Id: lnb.c,v 1.14.1.0, 2004-06-08 11:08:38Z, Ian Mitchell$
 *
 ****************************************************************************/

#include "stbcfg.h"
#include "basetype.h"
#include "demod_types.h"
#include "trace.h"
#include "retcodes.h"
#include "kal.h"
#include "gpio.h"
#include "cobra.h"
#include "lnb.h"

static LNB_SETTINGS lnb_parameters;

static struct {
    bool enabled;
    NIM_SATELLITE_POLARISATION polarization;
    bool tone_enabled;
} lnb_state = { FALSE };

/* Some handy tags for trace level specifications. */
#define TL_ALWAYS  (TRACE_LEVEL_ALWAYS | TRACE_DMD)
#define TL_ERROR   (TRACE_LEVEL_ALWAYS | TRACE_DMD)
#define TL_INFO    (TRACE_LEVEL_3 | TRACE_DMD)
#define TL_FUNC    (TRACE_LEVEL_2 | TRACE_DMD)
#define TL_VERBOSE (TRACE_LEVEL_1 | TRACE_DMD)

/*****************************************************************************/
/*  FUNCTION:    cnxt_lnb_set_parameters                                     */
/*                                                                           */
/*  PARAMETERS:  lnb - pointer to the LNB_SETTINGS structure defining the    */
/*                   LNB settings to be used.                                */
/*                                                                           */
/*  DESCRIPTION: This function makes a local copy of the LNB settings to be  */
/*               used in future tuning operations.                           */
/*                                                                           */
/*  RETURNS:     DEMOD_SUCCESS - the function completed successfully.        */
/*                                                                           */
/*  CONTEXT:     Must be called from a non-interrupt context.                */
/*                                                                           */
/*****************************************************************************/
DEMOD_STATUS cnxt_lnb_set_parameters( LNB_SETTINGS *lnb )
{
    FCopy ( &lnb_parameters, lnb, sizeof ( LNB_SETTINGS ) );
    return DEMOD_SUCCESS;
}

/*****************************************************************************/
/*  FUNCTION:    cnxt_lnb_get_parameters                                     */
/*                                                                           */
/*  PARAMETERS:  lnb - pointer to the LNB_SETTINGS structure to be filled in */
/*                   with a copy of the LNB settings in use.                 */
/*                                                                           */
/*  DESCRIPTION: This function fills in the LNB_SETTINGS structure from the  */
/*               local copy.                                                 */
/*                                                                           */
/*  RETURNS:     DEMOD_SUCCESS - the function completed successfully.        */
/*                                                                           */
/*  CONTEXT:     Must be called from a non-interrupt context.                */
/*                                                                           */
/*****************************************************************************/
DEMOD_STATUS cnxt_lnb_get_parameters( LNB_SETTINGS *lnb )
{
    FCopy ( lnb, &lnb_parameters, sizeof ( LNB_SETTINGS ) );
    return DEMOD_SUCCESS;
}

/*****************************************************************************/
/*  FUNCTION:    cnxt_set_lnb                                                */
/*                                                                           */
/*  PARAMETERS:  pNIM - pointer to the Cobra driver NIM structure for the    */
/*                   NIM the LNB is to be set for.                           */
/*               tuning - pointer to the TUNING_SPEC structure containing    */
/*                   specification of the current tuning operation.          */
/*               freq - pointer to the tuner frequency resulting from the    */
/*                   tuning request and current LNB settings.                */
/*                                                                           */
/*  DESCRIPTION: This function fills in the freq parameter based on the      */
/*               contents of the tuning parameter and current LNB settings.  */
/*               This calculates the tuner frequency based on the LNB LO     */
/*               frequency and the transponder frequency.  If the LNB is     */
/*               configured as LNB_MANUAL, the input frequency is the tuner  */
/*               frequency, so it is just copied.  This function sets the    */
/*               LNB signalling voltage and tone as appropriate.             */
/*                                                                           */
/*  RETURNS:     DEMOD_SUCCESS - the function completed successfully.        */
/*                                                                           */
/*  CONTEXT:     Must be called from a non-interrupt context.                */
/*                                                                           */
/*****************************************************************************/
DEMOD_STATUS cnxt_set_lnb( NIM *pNIM, TUNING_SPEC *tuning, u_int32 *freq )
{
    int result_frequency;
    bool tone_enable = FALSE;

    switch ( lnb_parameters.type )
    {
        /* For a single band LNB, the resulting frequency is just the difference
           between the transponder frequency and the LNB frequency.  In all
           cases, 22 KHz signalling is off for a single band LNB. */
        case LNB_SINGLE_FREQUENCY:
            result_frequency = (int)tuning->tune.nim_satellite_tune.frequency -
                    (int)lnb_parameters.lnb_a;
            tone_enable = FALSE;
            break;

        /* For a dual band LNB, the resulting frequency is the difference
           between the transponder frequency and either the low band or the high
           band LNB frequency, determined by the switch frequency.  The 22 KHz
           signalling is set on to select high band, off to select low band. */
        case LNB_DUAL_FREQUENCY:
            if ( tuning->tune.nim_satellite_tune.frequency >=
                    lnb_parameters.lnb_switch )
            {
                result_frequency =
                        (int)tuning->tune.nim_satellite_tune.frequency -
                        (int)lnb_parameters.lnb_b;
                tone_enable = TRUE;
            }
            else
            {
                result_frequency =
                        (int)tuning->tune.nim_satellite_tune.frequency -
                        (int)lnb_parameters.lnb_a;
                tone_enable = FALSE;
            }
            break;

        /* For an orbital position LNB, the resulting frequency is just the
           difference between the transponder frequency and the LNB frequency
           for the matching orbital position.  The 22 KHz signalling is set on
           if the requested orbital position specifies it, set off otherwise. */
        case LNB_ORBITAL_POSITION:
            if ( tuning->tune.nim_satellite_tune.orbital_position ==
                    lnb_parameters.orbital_position_a )
            {
                result_frequency =
                        (int)tuning->tune.nim_satellite_tune.frequency -
                        (int)lnb_parameters.lnb_a;
                if ( lnb_parameters.orbital_22khz_a )
                {
                    tone_enable = TRUE;
                }
                else
                {
                    tone_enable = FALSE;
                }
            }
            else if ( tuning->tune.nim_satellite_tune.orbital_position ==
                    lnb_parameters.orbital_position_b )
            {
                result_frequency =
                        (int)tuning->tune.nim_satellite_tune.frequency -
                        (int)lnb_parameters.lnb_b;
                if ( lnb_parameters.orbital_22khz_b )
                {
                    tone_enable = TRUE;
                }
                else
                {
                    tone_enable = FALSE;
                }
            }
            else
            {
                result_frequency =
                        (int)tuning->tune.nim_satellite_tune.frequency -
                        (int)lnb_parameters.lnb_c;
                if ( lnb_parameters.orbital_22khz_c )
                {
                    tone_enable = TRUE;
                }
                else
                {
                    tone_enable = FALSE;
                }
            }
            break;

        /* For a manual LNB, the resulting frequency is just the input
           frequency. */
        case LNB_MANUAL:
            result_frequency = (int)tuning->tune.nim_satellite_tune.frequency;
            break;

        default:
            #if RTOS != NOOS
                error_log( ERROR_WARNING | RC_SDM_BADVAL );
            #endif /* RTOS != NOOS */
            break;
    }

    /* Set the appropriate LNB settings if not a manually controlled LNB. */
    if ( lnb_parameters.type != LNB_MANUAL )
    {
        cnxt_lnb_set_tone_enable( pNIM, tone_enable );
    }
    cnxt_lnb_set_polarization( pNIM,
                tuning->tune.nim_satellite_tune.polarisation );

    /* In case the result frequency comes
       out negative, put it back positive. */
    if ( result_frequency < 0 )
    {
        result_frequency = -result_frequency;
    }

	 /* Multiply to convert KHz to Hz */
    result_frequency *= 1000;

    *freq = (u_int32)result_frequency;

    return DEMOD_SUCCESS;
}

/*****************************************************************************/
/*  FUNCTION:    cnxt_lnb_init                                               */
/*                                                                           */
/*  PARAMETERS:  pNIM - pointer to the Cobra driver NIM structure for the    */
/*                   NIM the LNB code is being initialized for.              */
/*                                                                           */
/*  DESCRIPTION: This function initializes LNB signalling for the NIM.       */
/*                                                                           */
/*  RETURNS:     DEMOD_SUCCESS - the function completed successfully.        */
/*               DEMOD_ERROR - there was an error in a low level driver      */
/*                   function.                                               */
/*                                                                           */
/*  CONTEXT:     Must be called from a non-interrupt context.                */
/*                                                                           */
/*****************************************************************************/
DEMOD_STATUS cnxt_lnb_init( NIM *pNIM )
{
    #if (INTERNAL_DEMOD == INTERNAL_COBRA_LIKE)
    #else
      #if RTOS != NOOS
        int api_error;
      #endif /* RTOS != NOOS */
    #endif

    /* Set the PIO that controls the direction of the 22KHz signal for output
       as the initial setting.  Any DiSEqC input will need to change it. */
    #if PIO_LNB_22KHZ_DIRECTION != GPIO_INVALID
        /*
         * !!! HACK ALERT !!!
         * WARNING!!!!! Hack required to keep from setting this PIO on a
         * Bronco1. When Bronco1 boards disappear, so should this hack. (PIO
         * setting should become conditional only upon value of
         * PIO_LNB_22KHZ_DIRECTION != GPIO_INVALID.)
         * !!! HACK ALERT !!!
         */
        #if I2C_CONFIG_EEPROM_ADDR != NOT_PRESENT
        {
            extern int ConfigurationValid;
            extern CONFIG_TABLE config_table;

            if ( ConfigurationValid &&
                    ( config_table.board_type != 0x00 ||
                        ((config_table.board_type == 0x00) &&
                         (config_table.board_rev != 0x01)) ) )
            {
                cnxt_gpio_set_output_level( PIO_LNB_22KHZ_DIRECTION, TRUE );
            }      
        }
        #endif
        /*
         * !!! HACK ALERT !!!
         * !!! HACK ALERT !!!
         */
    #endif

    /* Set up directional control for the PIO controlling LNB enable. */
    #if (INTERNAL_DEMOD == INTERNAL_COBRA_LIKE)
        /* No direction control necessary. */
    #else
        /* Cobra GPIO4 is the enable for the external LNB signal generator;
           set as an output. */
        if ( RegisterWrite( pNIM, CX24130_GPIO4DIR, 1 ) != True )
        {
        #if RTOS != NOOS
            trace_new( TL_ERROR,
                    "Cobra demod failed to set direction of LNB signal enable"
                    " (GPIO4).\n" );
            trace_new( TL_ERROR, "File: %s, line: %d\n",
                    API_GetErrorFilename(pNIM),
                    API_GetErrorLineNumber(pNIM) );
            api_error = API_GetLastError( pNIM );
            trace_new( TL_ERROR, "Error %d, %s\n", api_error,
                    API_GetErrorMessage(pNIM, (APIERRNO)api_error) );
        #endif /* RTOS != NOOS */
            return DEMOD_ERROR;
        }
    #endif
    
    /* Actually enable LNB output unless initialization with output disabled is
       requested. */
    #ifndef LNB_INITIALLY_DISABLED
        cnxt_lnb_set_output_enable( pNIM, TRUE );
    #endif

    return DEMOD_SUCCESS;
}

DEMOD_STATUS cnxt_lnb_set_output_enable( NIM *pNIM, bool enable )
{
    #if (INTERNAL_DEMOD == INTERNAL_COBRA_LIKE)
    #else
      #if RTOS != NOOS
        int api_error;
      #endif /* RTOS != NOOS */
    #endif

    /* Set up the LNB enable signal.  It is different for internal/external. */
    #if (INTERNAL_DEMOD == INTERNAL_COBRA_LIKE)
        /* LNB enable signal is driven from the PIO expander; set to 1 to
           enable, 0 to disable. */
        cnxt_gpio_set_output_level( PIO_LNB_ENABLE, enable );
        lnb_state.enabled = enable;
    #else
        /* Cobra GPIO4 is the enable for the external LNB signal generator;
           set to 1 to enable, 0 to disable. */
        if ( RegisterWrite( pNIM, CX24130_GPIO4VAL, enable ) == True )
        {
            lnb_state.enabled = enable;
        }
        else
        {
        #if RTOS != NOOS
            trace_new( TL_ERROR,
                    "Cobra demod failed to write to LNB signal enable"
                    " (GPIO4).\n" );
            trace_new( TL_ERROR, "File: %s, line: %d\n",
                    API_GetErrorFilename(pNIM),
                    API_GetErrorLineNumber(pNIM) );
            api_error = API_GetLastError( pNIM );
            trace_new( TL_ERROR, "Error %d, %s\n", api_error,
                    API_GetErrorMessage(pNIM, (APIERRNO)api_error) );
        #endif /* RTOS != NOOS */
            return DEMOD_ERROR;
        }
    #endif
    return DEMOD_SUCCESS;
}

DEMOD_STATUS cnxt_lnb_get_output_enable( NIM *pNIM, bool *enable )
{
    *enable = lnb_state.enabled;
    return DEMOD_SUCCESS;
}

DEMOD_STATUS cnxt_lnb_set_polarization( NIM *pNIM,
        NIM_SATELLITE_POLARISATION polarization )
{
    #if RTOS != NOOS
      int api_error;
    #endif /* RTOS != NOOS */
    LNBPOL polarization_voltage;

    lnb_state.polarization = polarization;
    
    /* Polarization voltage is calculated the same way for all LNB types. */
    switch ( polarization )
    {
        case M_HORIZONTAL:
            polarization_voltage = (LNBPOL)(
                    lnb_parameters.horizontal_voltage == V_12VOLTS ?
                        LNB_LOW : LNB_HIGH );
            break;
        case M_LEFT:
            polarization_voltage = (LNBPOL)( lnb_parameters.left_voltage ==
                    V_12VOLTS ? LNB_LOW : LNB_HIGH );
            break;
        case M_VERTICAL:
            polarization_voltage = (LNBPOL)( lnb_parameters.vertical_voltage ==
                    V_12VOLTS ? LNB_LOW : LNB_HIGH );
            break;
        case M_RIGHT:
            polarization_voltage = (LNBPOL)( lnb_parameters.right_voltage ==
                    V_12VOLTS ? LNB_LOW : LNB_HIGH );
            break;
        default:
#if RTOS != NOOS
            error_log( ERROR_WARNING | RC_SDM_BADVAL );
#endif /* RTOS != NOOS */
            polarization_voltage = (LNBPOL)( lnb_parameters.right_voltage ==
                    V_12VOLTS ? LNB_LOW : LNB_HIGH );
            lnb_state.polarization = M_RIGHT;
    }

    if ( !API_SetLNBDC( pNIM, polarization_voltage ) )
    {
    #if RTOS != NOOS
        trace_new( TL_ERROR, "API_SetLNBDC failed. File: %s, line: %d\n",
                API_GetErrorFilename( pNIM ),
                API_GetErrorLineNumber( pNIM ) );
        api_error = API_GetLastError( pNIM );
        trace_new( TL_ERROR, "Error %d, %s\n", api_error,
                API_GetErrorMessage( pNIM, (APIERRNO)api_error) );
    #endif /* RTOS != NOOS */
        return DEMOD_ERROR;
    }
    
    return DEMOD_SUCCESS;
}

DEMOD_STATUS cnxt_lnb_get_polarization( NIM *pNIM,
        NIM_SATELLITE_POLARISATION *polarization )
{
    *polarization = lnb_state.polarization;
    return DEMOD_SUCCESS;
}

DEMOD_STATUS cnxt_lnb_set_tone_enable( NIM *pNIM, bool enable )
{
    #if RTOS != NOOS
        int api_error;
    #endif /* RTOS != NOOS */
    #if (LNB_22KHZ_CONTROL == LNB_22KHZ_ENABLE)
        LNBMODE lnbmode;
    #else
        LNBTONE tone;
    #endif

    #if (LNB_22KHZ_CONTROL == LNB_22KHZ_ENABLE)
        if ( enable == FALSE )
        {
            lnbmode.lnb_mode = LNBMODE_MANUAL_ZERO;
        }
        else
        {
            lnbmode.lnb_mode = LNBMODE_MANUAL_ONE;
        }
        if ( !API_SetLNBMode( pNIM, &lnbmode ) )
        {
        #if RTOS != NOOS
            trace_new( TL_ERROR, "API_SetLNBMode failed. "
                    "File: %s, line: %d\n", API_GetErrorFilename( pNIM ),
                    API_GetErrorLineNumber( pNIM ) );
            api_error = API_GetLastError( pNIM );
            trace_new( TL_ERROR, "Error %d, %s\n", api_error,
                    API_GetErrorMessage( pNIM, (APIERRNO)api_error) );
        #endif /* RTOS != NOOS */
            return DEMOD_ERROR;
        }
    #else
        if ( enable )
        {
            tone = LNBTONE_ON;
        }
        else
        {
            tone = LNBTONE_OFF;
        }
        if ( !API_SetLNBTone( pNIM, tone ) )
        {
        #if RTOS != NOOS
            trace_new( TL_ERROR, "API_SetLNBTone failed. File: %s, line: %d\n",
                    API_GetErrorFilename( pNIM ),
                    API_GetErrorLineNumber( pNIM ) );
            api_error = API_GetLastError( pNIM );
            trace_new( TL_ERROR, "Error %d, %s\n", api_error,
                    API_GetErrorMessage( pNIM, (APIERRNO)api_error) );
        #endif /* RTOS != NOOS */
            return DEMOD_ERROR;
        }
    #endif

    lnb_state.tone_enabled = enable;
    return DEMOD_SUCCESS;
}

DEMOD_STATUS cnxt_lnb_get_tone_enable( NIM *pNIM, bool *enable )
{
    *enable = lnb_state.tone_enabled;
    return DEMOD_SUCCESS;
}

 /****************************************************************************
 * Modifications:
 *
 * $Log: 
 *  16   STB1.8.0  1.14.1.0    6/8/04 6:08:38 AM      Ian Mitchell    CR(s) 
 *        9383 : Math overflow in the function cnxt_set_lnb caused an incorrect
 *         frequency to be returned in cases where the frequency is near the 
 *        upper limit.
 *        
 *  15   mpeg      1.14        4/1/04 8:57:00 AM      Billy Jackman   CR(s) 
 *        8722 8723 : Modified to scale RF frequency from KHz to Hz when using 
 *        manual LNB control.
 *  14   mpeg      1.13        3/24/04 11:25:55 AM    Matt Korte      CR(s) 
 *        8648 8649 : Fix warnings
 *  13   mpeg      1.12        3/24/04 10:45:31 AM    Matt Korte      CR(s) 
 *        8648 8649 : If built with NOOS, then don't call trace_new()
 *  12   mpeg      1.11        3/22/04 2:40:55 PM     Billy Jackman   CR(s) 
 *        8585 : Correct errors when building for external Cobra demod.
 *  11   mpeg      1.10        3/22/04 2:05:48 PM     Billy Jackman   CR(s) 
 *        8585 : Fix initialization error.
 *  10   mpeg      1.9         3/22/04 1:10:29 PM     Billy Jackman   CR(s) 
 *        8585 : Modified keywords to new StarTeam format.
 *        Changed cnxt_set_lnb function to not pass back indications for how to
 *         set polarization voltage and tone but to set them directly.
 *        Added cnxt_lnb_init function for Cobra driver to call.
 *        Added APIs cnxt_lnb_set_output_enable, cnxt_lnb_get_output_enable, 
 *        cnxt_lnb_set_polarization, cnxt_lnb_get_polarization, 
 *        cnxt_lnb_set_tone_enable and cnxt_lnb_get_tone_enable to support 
 *        NDSCORE requirements.
 *        Added LNB of type LNB_MANUAL to allow NDSCORE manual control of LNB 
 *        signalling.
 *        
 *  9    mpeg      1.8         10/17/03 9:59:30 AM    Larry Wang      CR(s): 
 *        7673 Replace memcpy and memset with FCopy and FFillBytes.
 *  8    mpeg      1.7         7/8/03 7:12:12 PM      Billy Jackman   SCR(s) 
 *        6909 :
 *        Put things back the way they belong.
 *        
 *  7    mpeg      1.6         7/8/03 4:24:54 PM      Tim White       SCR(s) 
 *        6908 :
 *        Back out last change (1.5) due to error when attempting to tune using
 *         NDSTESTS
 *        (or NDSTESTS_STUB) application from file cobra_cx24108.c.  Defect 
 *        #6909 opened
 *        to fix this problem.
 *        
 *        
 *  6    mpeg      1.5         6/30/03 6:14:24 PM     Billy Jackman   SCR(s) 
 *        5816 :
 *        Modified cnxt_set_lnb to take into account the type of LNB used and 
 *        do the
 *        right thing for each of single, dual, and orbital position.
 *        
 *  5    mpeg      1.4         6/24/03 6:31:56 PM     Tim White       SCR(s) 
 *        6831 :
 *        Add flash, hsdp, demux, OSD, and demod support to codeldrext
 *        
 *        
 *  4    mpeg      1.3         3/21/03 9:46:02 AM     Billy Jackman   SCR(s) 
 *        5842 :
 *        If the resulting frequency from an LNB calculation is negative, take 
 *        its
 *        absolute value before returning the result.
 *        
 *  3    mpeg      1.2         3/19/03 8:54:50 AM     Billy Jackman   SCR(s) 
 *        5792 :
 *        Modified 22KHz tone setting so that if the lnb_a and lnb_b parameters
 *         are the
 *        same and equal to 11250KHz, the code will activate 22KHz.  If they 
 *        are the same
 *        and not equal to 11250KHz, the code will deactivate 22KHz.
 *        
 *  2    mpeg      1.1         11/27/02 1:25:32 PM    Billy Jackman   SCR(s) 
 *        4989 :
 *        Modified handling of LNB polarization and tone to allow correct 
 *        setting
 *        by the channel change API.
 *        
 *  1    mpeg      1.0         9/30/02 12:16:22 PM    Billy Jackman   
 * $
 * 
****************************************************************************/

