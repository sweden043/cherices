/****************************************************************************/
/*                   CONEXANT PROPRIETARY AND CONFIDENTIAL                  */
/*                       SOFTWARE FILE/MODULE HEADER                        */
/*          Conexant Systems Inc. (c) 1998, 1999, 2000, 2001, 2002          */
/*                              Austin, TX                                  */
/*                         All Rights Reserved                              */
/****************************************************************************/
/*
 * Filename:        hsdppriv.c
 *
 *
 * Description:     HSDP Functionality
 *
 *
 * Author:          Miles Bintz
 *
 ****************************************************************************/
/* $Header: hsdppriv.c, 14, 5/14/03 9:45:28 PM, Miles Bintz$
 ****************************************************************************/

#include "stbcfg.h"
#include "kal.h"
#include "trace.h"
#include "hsdp.h"

/* pin muxing for the MPEG decoder in various modes (ie. IO/PCI) are as follows:
   Also included in the table is the TSx name by which the spec
   refers to the port.

         |  22490 IO   | 22492 IO   | 22492 PCI   | 24430 IO   | 24430 PCI  | 24152 IO 
---------|-------------|------------|-------------|------------|------------|------------
NIM0     |  Serial     | Serial     | Serial      | Internal   | Internal   | Internal
         | Not Shared  | Not Shared | Not Shared  |   (TSE)    |   (TSE)    |   (TSB)
         |   (TSA)     |   (TSA)    |   (TSA)     |            |            |
---------|-------------|------------|-------------|------------|------------|------------
NIM1     |    N / A    | Serial     | Serial      | Internal   | Internal   | External
         |             | Shared PIO | Shared PIO  |   (TSF)    |   (TSF)    | Ser ED0
         |             |   (TSB)    |   (TSB)     |            |            |  (TSA) 
---------|-------------|------------|-------------|------------|------------|------------
HSDP0    |  Parallel   | Ser/Par    | Serial      | Ser/Par    | Serial     |  Ser/Par
         | Shared PIO  | Shared PIO | Shared PIO  | Shared PIO | Shared PIO | Shared PIO
         |   (TSB)     |   (TSC)    |   (TSC)     |   (TSC)    |   (TSC)    |  (TSC)
---------|-------------|------------|-------------|------------|------------|------------
HSDP1    |  Parellel   | Ser/Par    | Serial      | Ser/Par    | Serial     | Ser/Par
         | Shared PIO  | Shared PIO | Shared PIO  | Shared PIO | Shared PIO | Shared PIO
         |   (TSC)     |   (TSD)    |   (TSD)     |   (TSD)    |   (TSD)    |  (TSD)
---------+-------------+------------+-------------+------------+------------|------------
NIM2     |    N / A    |   N / A    |   N / A     |  N / A     |  N / A     |  N / A
         |             |            |             | (see blw)  | (see blw)  |
         |             |            |             |            |            |
---------|-------------|------------|-------------|------------|------------|------------
NIM3     |    N / A    |   N / A    |   N / A     | Serial     | Serial     |  N / A
         |             |            |             | Shared     | Shared     |
         |             |            |             |   (TSA)    |   (TSA)    |
---------|-------------|------------|-------------|------------|------------|------------
NIM4     |    N / A    |   N / A    |   N / A     | Parallel   | Parallel   |  N / A
         |             |            |             | Internal   | Internal   |
         |             |            |             |   (TSG)    |   (TSG)    |
---------|-------------|------------|-------------|------------|------------|------------
 *
 * Note regarding NIM2:  WaBASH has what the spec refers to as "NIM1 Parallel" and
 * "NIM1 Serial".  It also has "TSB" (input to demux or source for HSDP0/1) reserved.
 * Therefore, we can cheat with our "NIM" nomenclature and allow our HSDP enumeration
 * (HSDP_PORT) to still line up correctly with the demux input select or HSDP0/1 source
 * select. 
*/


/************************************************************************
 * The following structure contains private data which is specific
 * to each chip.
 * It defines the base address and type of
 * port so that, regardless of which chip this driver operates on,
 * the code stays relatively generic.
 *
 ************************************************************************/

#if HSDP_TYPE == HSDP_BRAZOS
const u_int8 legal_routes[MAX_HSDP_PORTS][MAX_HSDP_PORTS] = 

/* From:     To: NIM0  NIM1  HSDP0  HSDP1  NIM2  NIM3  NIM4  DMX0  DMX1  DMX2*/
/* NIM0   */ {  { 0  ,  0  ,  1   ,  1   ,  0  ,  0  ,   0,   1  ,  1  ,  1   },
/* NIM1   */    { 0  ,  0  ,  1   ,  1   ,  0  ,  0  ,   0,   1  ,  1  ,  1   },
/* HSD0   */    { 0  ,  0  ,  0   ,  1   ,  0  ,  0  ,   0,   1  ,  1  ,  1   },
/* HSD1   */    { 0  ,  0  ,  1   ,  0   ,  0  ,  0  ,   0,   1  ,  1  ,  1   },
/* NIM2   */    { 0  ,  0  ,  1   ,  1   ,  0  ,  0  ,   0,   1  ,  1  ,  1   },
/* NIM3   */    { 0  ,  0  ,  0   ,  0   ,  0  ,  0  ,   0,   0  ,  0  ,  0   },
/* NIM4   */    { 0  ,  0  ,  0   ,  0   ,  0  ,  0  ,   0,   0  ,  0  ,  0   },
/* DMX0   */    { 0  ,  0  ,  1   ,  1   ,  0  ,  0  ,   0,   0  ,  0  ,  0   },
/* DMX1   */    { 0  ,  0  ,  1   ,  1   ,  0  ,  0  ,   0,   0  ,  0  ,  0   },
/* DMX2   */    { 0  ,  0  ,  1   ,  1   ,  0  ,  0  ,   0,   0  ,  0  ,  0   } };

/* If connection state management is desired, uncomment the array
   below and implement code to set and clear the relevant bits when
   connections are made and broken. */
      
/* u_int8 busy_routes[MAX_HSDP_PORTS][MAX_HSDP_PORTS] = { {} ... }; */

hsdp_port      hsdp_port_data[] = {                                         
    /* TSB is the internal demod on Brazos, no muxing required */
    {  (LPREG)HSDP_TSB_PORT_CNTL_REG, },

    {  (LPREG)HSDP_TSA_PORT_CNTL_REG, },

    {  (LPREG)HSDP_TSC_PORT_CNTL_REG, },

    { (LPREG)HSDP_TSD_PORT_CNTL_REG, },

    /* NIM2 is actually the internal demod but in parallel
     * mode instead of serial.  No muxing is required since
     * this is the internal demod.
     */
    {  (LPREG)HSDP_TSB_PORT_CNTL_REG, },

    {  (LPREG)HSDP_TSF_PORT_CNTL_REG, },
    }; 
#endif


#if HSDP_TYPE == HSDP_WABASH
const u_int8 legal_routes[MAX_HSDP_PORTS][MAX_HSDP_PORTS] = 

/* From:     To: NIM0  NIM1  HSDP0  HSDP1  NIM2  NIM3  NIM4  DMX0  DMX1  DMX2*/
/* NIM0   */ {  { 0  ,  0  ,  1   ,  1   ,  0  ,  0  ,  0,    1  ,  1  ,  1   },
/* NIM1   */    { 0  ,  0  ,  0   ,  0   ,  0  ,  0  ,  0,    0  ,  0  ,  0   },
/* HSD0   */    { 0  ,  0  ,  0   ,  1   ,  0  ,  0  ,  0,    1  ,  1  ,  1   },
/* HSD1   */    { 0  ,  0  ,  1   ,  0   ,  0  ,  0  ,  0,    1  ,  1  ,  1   },
/* NIM2   */    { 0  ,  0  ,  1   ,  1   ,  0  ,  0  ,  0,    1  ,  1  ,  1   },
/* NIM3   */    { 0  ,  0  ,  1   ,  1   ,  0  ,  0  ,  0,    1  ,  1  ,  1   },
/* NIM4   */    { 0  ,  0  ,  1   ,  1   ,  0  ,  0  ,  0,    1  ,  1  ,  1   },
/* DMX0   */    { 0  ,  0  ,  1   ,  1   ,  0  ,  0  ,  0,    0  ,  0  ,  0   },
/* DMX1   */    { 0  ,  0  ,  1   ,  1   ,  0  ,  0  ,  0,    0  ,  0  ,  0   },
/* DMX2   */    { 0  ,  0  ,  1   ,  1   ,  0  ,  0  ,  0,    0  ,  0  ,  0   } };
/* If connection state managired, uncommenement is dest the array
   below.   However, it will take another MAX_HSDP_PORTS^2 (currently 81)
   bytes of memory */
/* u_int8 busy_routes[MAX_HSDP_PORTS][MAX_HSDP_PORTS] = { {} ... }; */
hsdp_port      hsdp_port_data[] = {                                         
    /* WaBASH has support for one external NIM, but is not used 
     * in any configuration
     */
    {  (LPREG)HSDP_TSA_PORT_CNTL_REG, },

    /* A source for TSB doesn't exist on WaBASH */
    {  (LPREG)0, }, 
    
    {  (LPREG)HSDP_TSC_PORT_CNTL_REG, }, 
    
    {  (LPREG)HSDP_TSD_PORT_CNTL_REG, }, 
    
    /* The following three NIMs are internal.  Thus, they have
       no pins to mux */
    {  (LPREG)HSDP_TSE_PORT_CNTL_REG, },
    
    {  (LPREG)HSDP_TSF_PORT_CNTL_REG, },
    }; 
#endif

#if HSDP_TYPE == HSDP_HONDO
const u_int8 legal_routes[MAX_HSDP_PORTS][MAX_HSDP_PORTS] = 
/* From:     To: NIM0  NIM1  HSDP0  HSDP1  NIM2  NIM3  NIM4  DMX0  DMX1  DMX2*/
/* NIM0   */ {  { 0  ,  0  ,  1   ,  1   ,  0  ,  0  ,  0,    1  ,  1  ,  1   },
/* NIM1   */    { 0  ,  0  ,  1   ,  1   ,  0  ,  0  ,  0,    1  ,  1  ,  1   },
/* HSD0   */    { 0  ,  0  ,  0   ,  1   ,  0  ,  0  ,  0,    1  ,  1  ,  1   },
/* HSD1   */    { 0  ,  0  ,  1   ,  0   ,  0  ,  0  ,  0,    1  ,  1  ,  1   },
/* NIM2   */    { 0  ,  0  ,  0   ,  0   ,  0  ,  0  ,  0,    0  ,  0  ,  0   },
/* NIM3   */    { 0  ,  0  ,  0   ,  0   ,  0  ,  0  ,  0,    0  ,  0  ,  0   },
/* NIM4   */    { 0  ,  0  ,  0   ,  0   ,  0  ,  0  ,  0,    0  ,  0  ,  0   },
/* DMX0   */    { 0  ,  0  ,  1   ,  1   ,  0  ,  0  ,  0,    0  ,  0  ,  0   },
/* DMX1   */    { 0  ,  0  ,  1   ,  1   ,  0  ,  0  ,  0,    0  ,  0  ,  0   },
/* DMX2   */    { 0  ,  0  ,  1   ,  1   ,  0  ,  0  ,  0,    0  ,  0  ,  0   } };
/* If connection state management is desired, uncomment the array
   below.   However, it will take another MAX_HSDP_PORTS^2 (currently 81)
   bytes of memory */
/* u_int8 busy_routes[MAX_HSDP_PORTS][MAX_HSDP_PORTS] = { {} ... }; */

hsdp_port      hsdp_port_data[] = {                                         
    /* The NIM0 serial interface has only one signal muxed: PIO58_NIMFAIL */
    {  (LPREG)HSDP_TSA_PORT_CNTL_REG },

    /* The NIM1 serial interface signals are all muxed */
    {  (LPREG)HSDP_TSB_PORT_CNTL_REG },

    {  (LPREG)HSDP_TSC_PORT_CNTL_REG },

    {  (LPREG)HSDP_TSD_PORT_CNTL_REG } };
#endif

#if HSDP_TYPE == HSDP_COLORADO
const u_int8 legal_routes[MAX_HSDP_PORTS][MAX_HSDP_PORTS] = 
/* From:     To: NIM0  NIM1  HSDP0  HSDP1  NIM2  NIM3  NIM4  DMX0  DMX1  DMX2*/
/* NIM0   */ {  { 0  ,        1   ,  1   ,  0  ,  0  ,        1  ,  0  ,  0   },
/* NIM1   */
/* HSD0   */    { 0  ,        0   ,  1   ,  0  ,  0  ,        1  ,  0  ,  0   },
/* HSD1   */    { 0  ,        1   ,  0   ,  0  ,  0  ,        1  ,  0  ,  0   },
/* NIM2   */    { 0  ,        0   ,  0   ,  0  ,  0  ,        0  ,  0  ,  0   },
/* NIM3   */    { 0  ,        0   ,  0   ,  0  ,  0  ,        0  ,  0  ,  0   },
/* NIM4   */
/* DMX0   */    { 0  ,        1   ,  1   ,  0  ,  0  ,        0  ,  0  ,  0   },
/* DMX1   */    { 0  ,        0   ,  0   ,  0  ,  0  ,        0  ,  0  ,  0   },
/* DMX2   */    { 0  ,        0   ,  0   ,  0  ,  0  ,        0  ,  0  ,  0   } };
/* If connection state management is desired, uncomment the array
   below.   However, it will take another MAX_HSDP_PORTS^2 (currently 81)
   bytes of memory */
/* u_int8 busy_routes[MAX_HSDP_PORTS][MAX_HSDP_PORTS] = { {} ... }; */


hsdp_port      hsdp_port_data[] = {                                         
    /* The NIM0 serial interface has only one signal muxed: PIO58_NIMFAIL */
    {  (LPREG)HSDP_TSA_PORT_CNTL_REG },
    
    /* The other bidir ports signals are muxed */
    {  (LPREG)HSDP_TSB_PORT_CNTL_REG },
       
    {  (LPREG)HSDP_TSC_PORT_CNTL_REG },
    
    {  (LPREG)0 }, 
    
    {  (LPREG)0 },
    
     };
#endif

const int hsdp_num_ports = sizeof(hsdp_port_data) / sizeof(hsdp_port);

/*  is_route_legal checks to see if the route is allowed
    using the data defined above.  It could also checks to see
    if the resource is available.  

 returns 1 for legal route
           0 for busy route (optional)
          -1 for an illegal route

*/
int is_route_legal(HSDP_PORT src, HSDP_PORT dest)  {
    /* HSDP_DEMUX2 is the highest enumerated HSDP_PORT. 
       If its greater than DEMUX2 its invalid  */
    if ((src > HSDP_DEMUX2) || (dest > HSDP_DEMUX2))  {
        trace_new(TRACE_LEVEL_3 | TRACE_DPS, "HSDP: ERROR:  The source or destination is invalid.\n", src, dest);
        return(-1);
    } 
    if (legal_routes[src][dest]) {
        /* if (busy_routes[src][dest]) return(0); */
        return(1);
    } else {
        return -1;
    }
    
}

/****************************************************************************
 * Modifications:
 * $Log: 
 *  14   mpeg      1.13        5/14/03 9:45:28 PM     Miles Bintz     SCR(s) 
 *        6353 6354 :
 *        rework of HSDP driver
 *        
 *  13   mpeg      1.12        4/1/03 6:23:46 PM      Miles Bintz     SCR(s) 
 *        5654 :
 *        removed NIM0FAIL from wabash section since NIM0FAIL doesn't exist
 *        
 *        
 *  12   mpeg      1.11        4/1/03 2:03:48 PM      Tim White       SCR(s) 
 *        5925 :
 *        Remove the Bronco3 specifc route check.
 *        
 *        
 *  11   mpeg      1.10        3/13/03 12:10:54 PM    Miles Bintz     SCR(s) 
 *        5753 5754 :
 *        Modified to support Bronco Rev 3 which uses parallel zephyr header 
 *        instead of serial zephyr header
 *        
 *        
 *  10   mpeg      1.9         1/28/03 11:27:20 AM    Billy Jackman   SCR(s) 
 *        5336 :
 *        Use NIM2 as the internal demod in parallel mode.
 *        
 *  9    mpeg      1.8         1/23/03 3:02:44 PM     Dave Wilson     SCR(s) 
 *        5292 :
 *        Updates to allow correct operation of the baseband inputs on the 
 *        Bronco IRD.
 *        
 *  8    mpeg      1.7         12/5/02 2:18:20 PM     Miles Bintz     SCR(s) 
 *        5074 :
 *        Added structures to support the brazos chip
 *        
 *        
 *  7    mpeg      1.6         5/13/02 12:16:58 PM    Tim White       SCR(s) 
 *        3760 :
 *        Renamed DPS_ HSDP definitions to be HSDP_ 
 *        
 *        
 *  6    mpeg      1.5         4/30/02 3:17:50 PM     Billy Jackman   SCR(s) 
 *        3660 :
 *        Removed an unused local variable to get rid of a compiler warning.
 *        
 *  5    mpeg      1.4         2/5/02 6:04:48 PM      Miles Bintz     SCR(s) 
 *        3058 :
 *        Changed the hsdp_port_type from hsdp_bidir to hsdp_bidir_ser for 
 *        wabash.
 *        
 *        
 *  4    mpeg      1.3         1/30/02 2:06:42 PM     Miles Bintz     SCR(s) 
 *        3058 :
 *        Development/interim checkin
 *        
 *        
 *  3    mpeg      1.2         1/21/02 2:44:22 PM     Miles Bintz     SCR(s) 
 *        3058 :
 *        api tweaks and document updates
 *        
 *        
 *  2    mpeg      1.1         1/18/02 1:34:24 PM     Miles Bintz     SCR(s) 
 *        3058 :
 *        Beta checkin
 *        
 *        
 *  1    mpeg      1.0         1/8/02 5:42:08 PM      Miles Bintz     
 * $
 * 
 *    Rev 1.13   14 May 2003 20:45:28   bintzmf
 * SCR(s) 6353 6354 :
 * rework of HSDP driver
 * 
 *    Rev 1.12   01 Apr 2003 18:23:46   bintzmf
 * SCR(s) 5654 :
 * removed NIM0FAIL from wabash section since NIM0FAIL doesn't exist
 * 
 * 
 *    Rev 1.11   01 Apr 2003 14:03:48   whiteth
 * SCR(s) 5925 :
 * Remove the Bronco3 specifc route check.
 * 
 * 
 *    Rev 1.10   13 Mar 2003 12:10:54   bintzmf
 * SCR(s) 5753 5754 :
 * Modified to support Bronco Rev 3 which uses parallel zephyr header instead of serial zephyr header
 * 
 * 
 *    Rev 1.9   28 Jan 2003 11:27:20   jackmaw
 * SCR(s) 5336 :
 * Use NIM2 as the internal demod in parallel mode.
 * 
 *    Rev 1.8   23 Jan 2003 15:02:44   dawilson
 * SCR(s) 5292 :
 * Updates to allow correct operation of the baseband inputs on the Bronco IRD.
 * 
 *    Rev 1.7   05 Dec 2002 14:18:20   bintzmf
 * SCR(s) 5074 :
 * Added structures to support the brazos chip
 * 
 * 
 *    Rev 1.6   13 May 2002 11:16:58   whiteth
 * SCR(s) 3760 :
 * Renamed DPS_ HSDP definitions to be HSDP_ 
 * 
 *
 ****************************************************************************/

