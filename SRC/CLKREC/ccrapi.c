/****************************************************************************/
/*                   CONEXANT PROPRIETARY AND CONFIDENTIAL                  */
/*                       SOFTWARE FILE/MODULE HEADER                        */
/*                 Conexant Systems Inc. (c) 1998, 1999, 2000               */
/*                               Austin, TX                                 */
/*                            All Rights Reserved                           */
/****************************************************************************/
/*
 * Filename:       crapi.c
 *
 *
 * Description:    Source file for low-level Clock Recovery API functions
 *                 
 *
 * Author:         Amy C. Pratt
 *
 ****************************************************************************/
/* $Header: ccrapi.c, 7, 4/22/04 3:42:43 PM, Larry Wang$
 ****************************************************************************/

/** include files **/
#include "stbcfg.h"
#define INCL_MPG
#include "basetype.h"
#include "globals.h"
#include "ccrapi.h"

/** local definitions */
#ifndef NULL
#define NULL 0
#endif 

/** external data **/
extern u_int32 ccrDiscontThreshold;
extern u_int32 ccrPCRTimerValue;
extern bool ccrPCRTimerEnable;
extern ccrFilterFunc ccrCurrentFilter;
extern ccrFilterInitFunc ccrCurrentFilterInit;
extern ccrFilterFunc ccrSimpleFilter;
extern ccrFilterInitFunc ccrSimpleFilterInit;

/** internal functions **/

/** private data **/
extern LPREG lpPCRTimer;

/** public functions **/


/* NAME                                                                 */
/* ccrSetDiscontinuityThreshold                                         */
/*      Set upper limit for difference between STC and PCR. If          */
/*      STC-PCR is greater than this limit, it is considered to         */
/*      be a discontinuity.                                             */
/*                                                                      */
/* PARAMETERS                                                           */
/* limit 	Specifies the maximum allowable error between STC and PCR.  */
/*          Should be expressed in terms of 27MHz clock ticks. This     */
/*          limit should be at least as large as the largest expected   */
/*          jitter.                                                     */
/* RETURNS                                                              */
/*      CCR_LIMIT_TOO_SMALL    if the requested limit is less than 300  */
/*      CCR_LIMIT_TOO_LARGE    if the requested limit is larger than    */
/*                             0x40000000.  (this is approximately 40   */
/*                             seconds.)                                */
/*      CCR_RETURN_SUCCESS     otherwise                                */
/*                                                                      */
/* DESCRIPTION                                                          */
/* ccrSetDiscontinutiyThreshold specifies the maximum allowable         */
/* difference between the STC and PCR. At PCR arrival, if the           */
/* error is greater than limit, the clock recovery driver will          */
/* assume that a discontinuity has occurred.  Clock recovery            */
/* will be suspended. A/V sync, if enabled, will be temporarily         */
/* disabled.  The next arriving PCR will be loaded into the STC.        */
/* Clock recovery and A/V sync will resume.                             */
/*                                                                      */
/* The current discontinutiy threshold can be obtained with             */
/* ccrGetDiscontinuityThreshold.                                        */
/*                                                                      */
/* CCR_DISCONT_THRESHOLD designates the default discontinuity threshold.*/
CCR_RETCODE ccrSetDiscontinuityThreshold(u_int32 limit)
{
    if (limit < 300) return CCR_LIMIT_TOO_SMALL;
    if (limit > 0x40000000) return CCR_LIMIT_TOO_LARGE;

    ccrDiscontThreshold = limit;
    return CCR_RETURN_SUCCESS;
}
	 
/* NAME                                                                        */
/* ccrGetDiscontinuityThreshold                                                */
/*       Get the current upper limit for difference between STC and PCR.       */
/*                                                                             */
/* RETURNS                                                                     */
/*      ccrGetDiscontinuityThreshold returns the maximum allowable difference  */
/*      between the STC and the PCR.                                           */
/*                                                                             */
/* DESCRIPTION                                                                 */
/*      Differences larger than this limit will cause the clock recovery       */
/*      software to assume a PCR discontinuity has occurred.                   */
/*                                                                             */
/*      For more information, see ccrSetDiscontinuityThreshold.                */
/*                                                                             */
/*      CCR_DISCONT_THRESHOLD designatest the default discontinuity threshold. */
/*                                                                             */

u_int32 ccrGetDiscontinuityThreshold(void)
{
    return ccrDiscontThreshold;
}

/* NAME                                                                  */
/* ccrSetPCRTimer                                                        */
/*        Set the value for the PCR timer.                               */
/*                                                                       */
/* PARAMETERS                                                            */
/* timer	Specifies the amount of time, in ticks of a 27MHz clock,     */
/*         to wait before accepting a new PCR.                           */
/* enable	If TRUE, the timer will be enabled.   If FALSE, the          */
/*         timer will be disabled.                                       */
/*                                                                       */
/* RETURNS                                                               */
/*   CCR_TIMEVAL_TOO_LARGE if the requested timer value is too large to  */
/*                         fit into the hardware register                */
/*   CCR_RETURN_SUCCESS    otherwise                                     */
/*                                                                       */
/* DESCRIPTION                                                           */
/*        ccrSetPCRTimer can be used to restrict the number of PCRs      */
/*        that are retrieved from the MPEG stream.  When the timer       */
/*        is enabled, the first arriving PCR starts the timer.           */
/*        Any PCR that arrives while the timer is counting down from     */
/*        timer_val will be ignored.  The next PCR to arrive after       */
/*        the timer expires will be loaded into the hardware, and the    */
/*        timer will start again.                                        */
/*                                                                       */
/*        By default the timer is enabled and set to limit the PCRs to   */
/*        a maximum of 50 per second.  It is not recommended             */
/*        that a timer_val greater than 27000000 (equal to 0.1 seconds)  */
/*        be used.                                                       */

CCR_RETCODE ccrSetPCRTimer(u_int32 timer_val, bool enable)
{
    /* timer_val must fit into 29 bits */
    if (timer_val >= (1<<29)) return CCR_TIMEVAL_TOO_LARGE;

    ccrPCRTimerValue = timer_val;
    ccrPCRTimerEnable = enable;
    CNXT_SET_VAL(lpPCRTimer, MPG_PCR_TIMER_INFO_TIMERVALUE_MASK, ccrPCRTimerValue);
    CNXT_SET_VAL(lpPCRTimer, MPG_PCR_TIMER_INFO_ENABLETIMER_MASK, ccrPCRTimerEnable);

    return CCR_RETURN_SUCCESS;
}

/* NAME                                                                  */
/* ccrSetSTCFromBitstream                                                */
/*       Sets the STC value to be equal to the most recent PCR value     */
/*       from the bitstream.                                             */
/*                                                                       */
/* DESCRIPTION                                                           */
/*       Each PCR that is extracted from the bitstream is placed into    */
/*       a backup counter that continues to increment with the 27MHz     */
/*       clock.  ccrSetSTCFromBitstream will cause the value that is     */
/*       currently in the backup counter to become the STC.              */
/*                                                                       */
/*       It is the caller's responsibility to make sure a valid PCR PID  */
/*       is currently enabled.  If this is not the case, the backup      */
/*       counter may contain garbage data, and this data will become     */
/*       the new STC.                                                    */

void ccrSetSTCFromBitstream(void)
{
    CNXT_SET_VAL(glpCtrl0, MPG_CONTROL0_LOADBACKUPSTC_MASK, 1);
}


/* NAME                                                                         */
/*     ccrInstallFilter - Installs a new filter routine.                        */
/*                                                                              */
/* C SPECIFICATION                                                              */
/* CCR_RETCODE ccrInstallFilter(ccrFilterFunc filter, ccrFilterInit filterInit) */
/*                                                                              */
/* PARAMETERS                                                                   */
/*     filter    A pointer to a user-supplied filter function                   */
/*     filterInit 	A pointer to a user-supplied function that performs any     */
/*                 necessary initialization for the filter function.            */
/*                                                                              */
/* RETURNS                                                                      */
/*     CCR_NULL_POINTER    if either function pointer is NULL                   */
/*     CCR_RETURN_SUCCESS  otherwise.                                           */
/*                                                                              */
/* DESCRIPTION                                                                  */
/*     This function will install a user-supplied clock recovery filter         */
/*     function and an initialization routine.  If either of the input          */
/*     parameters is a NULL pointer, the default filter function will be        */
/*     installed.  A new filter may be installed at any time.  However,         */
/*     if this installation function is invoked after the clock recovery        */
/*     driver has been initialized, the caller should call the filter           */
/*     initialization routine immediately prior to installing the new filter.   */
/*                                                                              */
/*     A simple filter designed to handle up to 50 microseconds of PCR jitter   */
/*     will be installed by default.                                            */

CCR_RETCODE ccrInstallFilter(ccrFilterFunc filter, ccrFilterInitFunc filterInit)
{
    if ((filter == NULL) || (filterInit == NULL))  {
         ccrCurrentFilter = ccrSimpleFilter;
         ccrCurrentFilterInit = ccrSimpleFilterInit;
         return CCR_NULL_POINTER;
    }

    ccrCurrentFilter = filter;
    ccrCurrentFilterInit = filterInit;

    return CCR_RETURN_SUCCESS;
}

/****************************************************************************
 * Modifications:
 * $Log: 
 *  7    mpeg      1.6         4/22/04 3:42:43 PM     Larry Wang      CR(s) 
 *        8929 8930 : Define lpPCRTimer as an extern variable.
 *  6    mpeg      1.5         2/13/03 11:22:00 AM    Matt Korte      SCR(s) 
 *        5479 :
 *        Removed old header reference
 *        
 *  5    mpeg      1.4         6/10/02 3:12:22 PM     Craig Dry       SCR(s) 
 *        3923 :
 *        Remove MPG bitfields unconditionally - Step 2
 *        
 *  4    mpeg      1.3         6/6/02 5:49:52 PM      Craig Dry       SCR(s) 
 *        3923 :
 *        Remove MPG bitfields conditionally - Step 1
 *        
 *  3    mpeg      1.2         4/5/02 11:54:58 AM     Tim White       SCR(s) 
 *        3510 :
 *        Backout DCS #3468
 *        
 *        
 *  2    mpeg      1.1         3/28/02 2:52:38 PM     Quillian Rutherford 
 *        SCR(s) 3468 :
 *        Removed bit fields
 *        
 *        
 *  1    mpeg      1.0         8/20/00 7:42:20 PM     Amy Pratt       
 * $
 * 
 *    Rev 1.5   13 Feb 2003 11:22:00   kortemw
 * SCR(s) 5479 :
 * Removed old header reference
 * 
 *    Rev 1.4   10 Jun 2002 14:12:22   dryd
 * SCR(s) 3923 :
 * Remove MPG bitfields unconditionally - Step 2
 * 
 *    Rev 1.3   06 Jun 2002 16:49:52   dryd
 * SCR(s) 3923 :
 * Remove MPG bitfields conditionally - Step 1
 * 
 *    Rev 1.2   05 Apr 2002 11:54:58   whiteth
 * SCR(s) 3510 :
 * Backout DCS #3468
 * 
 * 
 *    Rev 1.0   20 Aug 2000 18:42:20   prattac
 * Initial revision.
 *
 ****************************************************************************/

