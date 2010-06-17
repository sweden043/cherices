/****************************************************************************/
/*                   CONEXANT PROPRIETARY AND CONFIDENTIAL                  */
/*                       SOFTWARE FILE/MODULE HEADER                        */
/*                     Conexant Systems Inc. (c)  2003                      */
/*                              Austin, TX                                  */
/*                         All Rights Reserved                              */
/****************************************************************************/
/*
 * Filename:        DIRECTV.c
 *
 *
 * Description:     DIRECTV Pawser microcode
 *
 *
 * Author:          Bob Van Gulick
 *
 ****************************************************************************/
/* $Header:DIRECTV.C, 4, 6/14/2004 10:39:12 PM, Larry Wang$
 ****************************************************************************/

#include <stdio.h>
#include "stbcfg.h"
#include "basetype.h"

/*
 ****************************************************************************
 *
 * How to build the microcode:
 * ===========================
 *
 * pawasm -l -a -c <flags> directv.s
 * rename directv.c <filename>.h
 *
 * Build Option      Naming Indicator      Cap Bit      Notes
 * ------------      ----------------      -------      -----
 * -l                                                   Generate listing file
 * -a                                                   Generate ascii memory file
 * -c                                                   Generate .c (.h) file
 * -dBRAZOS                                             Target: Brazos (default)
 * -dDEBUG                                     0        internal
 * -dMULTI_PSI                                 1        default
 * -dXPRT                                      2        default
 * -dAPG                  a                    3        APG parsing
 * -dBYTESWAP_PSI                              5        default   
 * -dNAV                                      10        obsolete
 *
 * Versions Available:
 * ===================
 * directv.h                - Generic MPG support
 *   pawasm -c -l -a -dBRAZOS -dXPRT directv.s
 * 
 * directv_a.h              - Generic APG support
 *   pawasm -c -l -a -dBRAZOS -dXPRT -dAPG -dMULTI_PSI directv.s
 * 
 ****************************************************************************
 */

/*
 * Inclusions
 */
#if (XTV_SUPPORT==YES)

   /*
    * XTV Microcode
    */
 
   #if (PARSER_MICROCODE==UCODE_HONDO)
      const u_int32 RecPawserMicrocode[] =
      #include "directv_xtv_rec.h"
      const u_int32 RecPawserMicrocodeSize = sizeof(RecPawserMicrocode);
      const u_int32 PlayPawserMicrocode[] =
      #include "directv_xtv_ply.h"
      const u_int32 PlayPawserMicrocodeSize = sizeof(PlayPawserMicrocode);
   #else
      #error "Microcode not supported!"
   #endif

#else

   const u_int32 PawserMicrocode[] =
   #if (DIRECTV_PROGRAM_GUIDE==MPG)
      #include "directv.h"
   #elif (DIRECTV_PROGRAM_GUIDE==APG)
      #include "directv_a.h"
   #endif
   const u_int32 MicrocodeSize = sizeof(PawserMicrocode);

#endif

