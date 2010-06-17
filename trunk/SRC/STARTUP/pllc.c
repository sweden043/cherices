/****************************************************************************/
/*                   CONEXANT PROPRIETARY AND CONFIDENTIAL                  */
/*                       SOFTWARE FILE/MODULE HEADER                        */
/*                      Conexant Systems Inc. (c) 2003                      */
/*                              Austin, TX                                  */
/*                         All Rights Reserved                              */
/****************************************************************************/
/*
 * Filename:        pllc.c
 *
 *
 * Description:     PLL helper functions
 *
 *
 * Author:          Tim White
 *
 ****************************************************************************/
/* $Id: pllc.c,v 1.4, 2003-11-10 22:35:33Z, Tim White$
 ****************************************************************************/

/******************/
/* Include Files  */
/******************/
#include "stbcfg.h"
#include "basetype.h"
#include "startup.h"
#include "stbcfg.h"

/********************************************************************/
/*  CalcFreqFromFracPart                                            */
/*                                                                  */
/*  DESCRIPTION:                                                    */
/*      Calculate ( freq * pll_frac ) / ( 2 ^ frac_len )            */
/*      Note that ( freq * pll_frac ) is much bigger                */
/*      than unsigned 0xffffffff.  The algorithm used may result    */
/*      up to 3 less than what should be                            */
/*                                                                  */
/*  PARAMETERS:                                                     */
/*      See description                                             */
/*                                                                  */
/*  RETURNS:                                                        */
/*      result described above                                      */
/********************************************************************/
static u_int32 CalcFreqFromFracPart ( u_int32 freq,
                                      u_int32 pll_frac,
                                      u_int32 frac_len )
{
   u_int32 k, l, freq0, frac0, retval;

   if ( frac_len == 25 )
   {
      k = 13;
      l = 12;
   }
   else if ( frac_len == 16 )
   {
      k = 8;
      l = 8;
   }
   else
   {
      return 0xffffffff;
   }

   freq0    = freq >> k;
   freq     = freq - ( freq0 << k );
   frac0    = pll_frac >> l;
   pll_frac = pll_frac - ( frac0 << l );

   retval = freq0 * frac0 + ( ( freq0 * pll_frac ) >> l )
                          + ( ( frac0 * freq ) >> k )
                          + ( ( freq * pll_frac ) >> frac_len );

   return retval;
}

/********************************************************************/
/*  CalkClkFreqAndPeriod                                            */
/*                                                                  */
/*  DESCRIPTION:                                                    */
/*      Calculate clock frequency and period for a specific PLL.    */
/*                                                                  */
/*  PARAMETERS:                                                     */
/*      pll_source - One of:  ARM_PLL_SOURCE, MEM_PLL_SOURCE        */
/*      xtal_freq  - Xtal frequency in Hz                           */
/*      raw        - Do not include the post divider (i.e. VCO)     */
/*                                                                  */
/*  RETURNS:                                                        */
/*      frequency  - Returned clock frequency in Hz                 */
/*      period     - Returned clock period in 100ns increments      */
/********************************************************************/
void CalcClkFreqAndPeriod (u_int32 *frequency,
                           u_int32 *period,
                           PLL_SOURCE pll_source,
                           u_int32 xtal_freq,
                           bool raw )
{
    u_int32 pll_int, pll_frac, pll_div, pll_prescale, clk_freq, remainder, frac_shift, clk_period;

    *frequency = 0;
    *period    = 0;

    /*
     * Calculate the clk_freq and clk_period
     */

    /* Calculation below depends on the PLL details of the chip.  */
    /* Colorado/Hondo use a pre-divide of 1 and an XTAL frequency */
    /* of 14.31818MHz. Wabash uses a pre-divide of 3, another     */
    /* un-documented pre-divide of 2, and either a XTAL frequency */
    /* of 36.4089MHz (USA) or 46.8114MHz (Europe).                */

    /* In the following calculations, the frac value read from the*/
    /* PLL multiplier must be first converted to a floating       */
    /* point value, then divided by the number of bits used to    */
    /* represent the frac value (2^16) represented as a floating  */
    /* point value. This will cause the fixed-point integer       */
    /* representation of a fractional value to be converted to a  */
    /* floating-point fractional value.                           */

    /*
     * Read the PLL values from the chip
     */
    switch(pll_source)
    {
        case ARM_PLL_SOURCE:
            pll_int  = CNXT_GET(PLL_CPU_CONFIG_REG, PLL_CPU_CONFIG_INT_MASK) >>
                                                    PLL_CPU_CONFIG_INT_SHIFT;
            pll_frac = CNXT_GET(PLL_CPU_CONFIG_REG, PLL_CPU_CONFIG_FRAC_MASK) >>
                                                    PLL_CPU_CONFIG_FRAC_SHIFT;
            pll_div  = CNXT_GET(PLL_DIV1_REG,       PLL_DIV1_ARM_CLK_MASK) >>
                                                    PLL_DIV1_ARM_CLK_SHIFT;
          #if PLL_TYPE == PLL_TYPE_COLORADO
            pll_prescale = 1;
          #else
            pll_prescale = PLL_PRESCALE (((*(LPREG)PLL_PRESCALE_REG)&PLL_PRESCALE_ARM_MASK)>>PLL_PRESCALE_ARM_SHIFT);
          #endif
            frac_shift = PLL_CPU_CONFIG_FRAC_SHIFT;
            break;

        case MEM_PLL_SOURCE:
            pll_int  = CNXT_GET(PLL_MEM_CONFIG_REG, PLL_MEM_CONFIG_INT_MASK) >>
                                                    PLL_MEM_CONFIG_INT_SHIFT;
            pll_frac = CNXT_GET(PLL_MEM_CONFIG_REG, PLL_MEM_CONFIG_FRAC_MASK) >>
                                                    PLL_MEM_CONFIG_FRAC_SHIFT;
            pll_div  = CNXT_GET(PLL_DIV1_REG,       PLL_DIV1_MEM_CLK_MASK) >>
                                                    PLL_DIV1_MEM_CLK_SHIFT;
          #if PLL_TYPE == PLL_TYPE_COLORADO
            pll_prescale = 1;
          #else
            pll_prescale = PLL_PRESCALE (((*(LPREG)PLL_PRESCALE_REG)&PLL_PRESCALE_MEM_MASK)>>PLL_PRESCALE_MEM_SHIFT);
          #endif
            frac_shift = PLL_MEM_CONFIG_FRAC_SHIFT;
            break;

       default:
            return;
    }

  #ifdef DLOAD

    /*
     * If called from the Download (DLOAD) Utility, run-time select the
     * appropriate calculation function.
     */

    if ((ISCOLORADO)||(ISHONDO))
    {
         xtal_freq    = 14318180;
         pll_prescale = 1;
    }
    else if(ISWABASH)
    {
          xtal_freq    = 46811428;
          pll_prescale = 3;
    }
    else /* ISBRAZOS and beyond */
    {
          xtal_freq    = 16317333;
          pll_prescale = 1;
    }

  #endif /* DLOAD */

    /*
     * If raw, do not factor in the post divide value, just use pre-scale.
     * Otherwise, multiply the pre-scale with the post divide value.
     */
    if(raw)
    {
        pll_div = pll_prescale;
    }
    else
    {
        pll_div = pll_div * pll_prescale;
    }

    /* 
     * calculate clk_freq = xtal_freq*(pll_int+pll_frac/2^n)/pll_div
     * note that algorithm used here may result up to 4Hz less than what it should be
     */
    clk_freq  = xtal_freq / pll_div;
    remainder = xtal_freq % pll_div;

    clk_freq = CalcFreqFromFracPart ( clk_freq, pll_frac, (25-frac_shift) )
                      + (clk_freq * pll_int) + (( remainder * pll_int ) / pll_div);

  #if PLL_TYPE == PLL_TYPE_WABASH
    clk_freq = clk_freq / 2;
  #endif

    /*
     * Calculate clk_period = (10^11)/clk_freq
     * note that the algorithm will be broken if clk_freq is higher
     * than 429.5MHz.  it should not happen in the predictable future.
     */
    clk_period  = ( 1000000000 / clk_freq ) * 100;
    remainder   = ( 1000000000 % clk_freq ) * 10;

    clk_period += ( remainder / clk_freq ) * 10;
    remainder   = ( remainder % clk_freq ) * 10;

    clk_period += remainder / clk_freq;

    /*
     * Return the values
     */
    *frequency = clk_freq;
    *period    = clk_period;
}

/****************************************************************************
 * Modifications:
 * $Log: 
 *  5    mpeg      1.4         11/10/03 4:35:33 PM    Tim White       CR(s): 
 *        7900 7901 Added capability to receive raw VCO frequency/period 
 *        information.
 *        
 *  4    mpeg      1.3         10/30/03 4:14:10 PM    Tim White       CR(s): 
 *        7756 7757 Remove the CalcMemClkPeriod() function, make it generic, 
 *        and call it CalcFreqAndPeriod().
 *        
 *  3    mpeg      1.2         10/17/03 9:48:10 AM    Larry Wang      CR(s): 
 *        7673 Calculate MemClkPeriod32 with fixed point.
 *  2    mpeg      1.1         9/2/03 6:58:00 PM      Joe Kroesche    SCR(s) 
 *        7415 :
 *        removed unneeded header files that were causing PSOS warnings
 *        
 *  1    mpeg      1.0         5/14/03 4:38:28 PM     Tim White       
 * $
 ****************************************************************************/
