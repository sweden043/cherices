/****************************************************************************/
/*                   CONEXANT PROPRIETARY AND CONFIDENTIAL                  */
/*                       SOFTWARE FILE/MODULE HEADER                        */
/*           Conexant Systems Inc. (c) 1998, 1999, 2000, 2001, 2002         */
/*                               Austin, TX                                 */
/*                            All Rights Reserved                           */
/****************************************************************************/
/*
 * Filename:       drmfilter.c
 *
 *
 * Description:    Code to initialise the DRM scaler filter coefficients
 *
 *
 * Author:         Dave Wilson
 *
 ****************************************************************************/
/* $Header: drmfilter.c, 7, 10/24/03 4:05:37 PM, $
 ****************************************************************************/


/*****************/
/* Include Files */
/*****************/
#include "basetype.h"
#include "stbcfg.h"
#include "drmfilter.h"
#include "trace.h"

/******************************************/
/* Local Definitions and Global Variables */
/******************************************/

#if (DRM_SCALER_COEFF_TYPE != DRM_SCALER_COEFF_TYPE_COLORADO)
/* On wabash, only rev C has the scaler... */
extern unsigned long ChipRevID;

u_int32 uDRMFilterCoefficients[] =
{
  0x98000000,
  0x00000000,
  0x00100000,
  0x000087b1,
  0x000fdf81,
  0x00011771,
  0x000fa702,
  0x00019f32,
  0x000f6e83,
  0x00022ee3,
  0x000f2e14,
  0x0002c694,
  0x000eddb5,
  0x00036654,
  0x000e7576,
  0x00040605,
  0x000e1527,
  0x0004adb6,
  0x000d9d07,
  0x00052d96,
  0x000d44d7,
  0x0005c567,
  0x000cd4a7,
  0x00067d27,
  0x000c34b7,
  0x00070517,
  0x000bc497,
  0x0007b4e7,
  0x000b24a7,
  0x000844d7,
  0x000aa497,
  0x0008ecb7,
  0x000a04a7,
  0x00097ca7,
  0x00097ca7,
  0x000a04a7,
  0x0008ecb7,
  0x000aa497,
  0x000844d7,
  0x000b24a7,
  0x0007b4e7,
  0x000bc497,
  0x00070517,
  0x000c34b7,
  0x00067d27,
  0x000cd4a7,
  0x0005c567,
  0x000d44d7,
  0x00052d96,
  0x000d9d07,
  0x0004adb6,
  0x000e1527,
  0x00040605,
  0x000e7576,
  0x00036654,
  0x000eddb5,
  0x0002c694,
  0x000f2e14,
  0x00022ee3,
  0x000f6e83,
  0x00019f32,
  0x000fa702,
  0x00011771,
  0x000fdf81,
  0x000087b1
};

u_int32 uNumDRMFilterCoefficients = sizeof(uDRMFilterCoefficients)/sizeof(u_int32);
#endif  /* !DRM_SCALER_COEFF_TYPE_COLORADO */


/********************************************************************/
/*  FUNCTION:    cnxt_drm_init_filter_coefficients                  */
/*                                                                  */
/*  PARAMETERS:  None                                               */
/*                                                                  */
/*  DESCRIPTION: Dummy function for ICs earlier than Brazos (where  */
/*               the filter coefficients were in internal ROM)      */
/*                                                                  */
/*  RETURNS:     Nothing                                            */
/*                                                                  */
/*  CONTEXT:     May be called in any context                       */
/*                                                                  */
/********************************************************************/
void cnxt_drm_init_filter_coefficients(void)
{
#if (DRM_SCALER_COEFF_TYPE == DRM_SCALER_COEFF_TYPE_COLORADO)
  /* Dummy function - no initialisation required for chips */
  /* earlier than Brazos.                                  */
    return;
#elif (DRM_SCALER_COEFF_TYPE == DRM_SCALER_COEFF_TYPE_WABASH)
    u_int32 uLoop;

    if (ChipRevID >= PCI_REV_ID_C_WABASH) {
        /* Write the values one after the other to the coefficient register */
        for (uLoop = 0; uLoop < uNumDRMFilterCoefficients; uLoop++)
        {
            *(LPREG)DRM_SCALING_COEFF_REG = uDRMFilterCoefficients[uLoop];
        }
    }

#elif (DRM_SCALER_COEFF_TYPE == DRM_SCALER_COEFF_TYPE_BRAZOS)
    u_int32 uLoop;

    /* Write the values one after the other to the coefficient register */
    for (uLoop = 0; uLoop < uNumDRMFilterCoefficients; uLoop++)
    {
        *(LPREG)DRM_SCALING_COEFF_REG = uDRMFilterCoefficients[uLoop];
    }
    
    /* For Brazos (rev B) we also need to set the DRM_SATURATION_VALUES register */
    if(!ISBRAZOSREVA)
    {
      CNXT_SET_VAL(DRM_SATURATION_VALUES_REG, DRM_SATURATION_LOW_Y_MASK,  0x10);
      CNXT_SET_VAL(DRM_SATURATION_VALUES_REG, DRM_SATURATION_LOW_C_MASK,  0x10);
      CNXT_SET_VAL(DRM_SATURATION_VALUES_REG, DRM_SATURATION_HIGH_Y_MASK, 0xEB);
      CNXT_SET_VAL(DRM_SATURATION_VALUES_REG, DRM_SATURATION_HIGH_C_MASK, 0xF0);
    }
#else
#error Unknown DRM_SCALER_COEFF_TYPE!
#endif

}

/****************************************************************************
 * Modifications:
 * $Log: 
 *  7    mpeg      1.6         10/24/03 4:05:37 PM    Sahil Bansal    CR(s): 
 *        7665 Added support for higher pll/clock speeds on for wabash revc1 
 *        parts.  now recv1
 *        will run mpeg arm/fclk at 250, cm fclk=240, mpeg sdram clk=142, and 
 *        cm blck=142.
 *  6    mpeg      1.5         5/19/03 4:40:44 PM     Dave Wilson     SCR(s) 
 *        6437 6438 6439 :
 *        Set the DRM_SATURATION_VALUES_REG for Brazos Rev B.
 *        
 *  5    mpeg      1.4         4/28/03 4:37:44 PM     Miles Bintz     SCR(s) 
 *        6115 :
 *        
 *        
 *  4    mpeg      1.3         1/23/03 3:16:10 PM     Dave Wilson     SCR(s) 
 *        5296 :
 *        Replaced filter coefficients on advice from hardware engineer. This 
 *        cures
 *        the vertical striping of scaled video windows that had previously 
 *        been seen.
 *        
 *  3    mpeg      1.2         1/22/03 5:24:56 PM     Dave Wilson     SCR(s) 
 *        5099 :
 *        Updated filter values as per instructions from hardware engineers.
 *        
 *  2    mpeg      1.1         1/22/03 9:36:30 AM     Matt Korte      SCR(s) 
 *        5258 :
 *        Fix EOF.
 *        
 *        
 *  1    mpeg      1.0         12/17/02 3:37:34 PM    Dave Wilson     
 * $
 * 
 *    Rev 1.5   19 May 2003 15:40:44   dawilson
 * SCR(s) 6437 6438 6439 :
 * Set the DRM_SATURATION_VALUES_REG for Brazos Rev B.
 * 
 *    Rev 1.4   28 Apr 2003 15:37:44   bintzmf
 * SCR(s) 6115 :
 * 
 * 
 *    Rev 1.3   23 Jan 2003 15:16:10   dawilson
 * SCR(s) 5296 :
 * Replaced filter coefficients on advice from hardware engineer. This cures
 * the vertical striping of scaled video windows that had previously been seen.
 * 
 *    Rev 1.2   22 Jan 2003 17:24:56   dawilson
 * SCR(s) 5099 :
 * Updated filter values as per instructions from hardware engineers.
 * 
 *    Rev 1.1   22 Jan 2003 09:36:30   kortemw
 * SCR(s) 5258 :
 * Fix EOF.
 * 
 * 
 *    Rev 1.0   17 Dec 2002 15:37:34   dawilson
 * SCR(s) 5073 :
 * Function to initialise the DRM scaler filter coefficients on Brazos.
 *
 ****************************************************************************/

