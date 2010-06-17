#ifndef __APIFIX
#define __APIFIX

/****************************************************************************/
/*                   CONEXANT PROPRIETARY AND CONFIDENTIAL                  */
/*                    Conexant Systems Inc. (c) 2002-2004                   */
/*                              Austin, TX                                  */
/*                         All Rights Reserved                              */
/****************************************************************************/
/*
 * Filename: apifix.h
 *
 * Description: This file allows the modification of API entry point names 
 *              by the addition of a prefix and/or suffix without the need 
 *              to modify the original source files.
 *
 * Author: Billy Jackman
 *
 ****************************************************************************/
/*
 $Id: apifix.h,v 1.3, 2004-03-16 17:40:46Z, Billy Jackman$
 *
 ****************************************************************************/

#define API_InitEnvironment(a,b,c,d,e,f,g,h,i,j,k) Cobra_API_InitEnvironment(a,b,c,d,e,f,g,h,i,j,k)
#define API_ChangeChannel(a,b) Cobra_API_ChangeChannel(a,b)
#define API_Monitor(a,b,c) Cobra_API_Monitor(a,b,c)
#define API_NIMGetChipInfo(a,b,c,d,e,f) Cobra_API_NIMGetChipInfo(a,b,c,d,e,f)
#define API_GetDriverVersion(a,b) Cobra_API_GetDriverVersion(a,b)
#define API_NIMGetTunerCount(a) Cobra_API_NIMGetTunerCount(a)
#define API_NIMGetTunerIDs(a,b,c) Cobra_API_NIMGetTunerIDs(a,b,c)
#define API_ReleaseEnvironment(a) Cobra_API_ReleaseEnvironment(a)
#define API_SetTunerFrequency(a,b) Cobra_API_SetTunerFrequency(a,b)
#define API_GetTunerFrequency(a,b) Cobra_API_GetTunerFrequency(a,b)
#define API_GetPLLFrequency(a,b) Cobra_API_GetPLLFrequency(a,b)
#define API_GetTunerVCO(a,b) Cobra_API_GetTunerVCO(a,b)
#define API_GetVCODivider(a,b) Cobra_API_GetVCODivider(a,b)
#define API_GetTunerReferenceDivider(a,b) Cobra_API_GetTunerReferenceDivider(a,b)
#define API_SetTunerReferenceDivider(a,b) Cobra_API_SetTunerReferenceDivider(a,b)
#define API_SetTunerRegisters(a,b,c,d) Cobra_API_SetTunerRegisters(a,b,c,d)
#define API_GetTunerRegisters(a,b,c,d) Cobra_API_GetTunerRegisters(a,b,c,d)
#define API_SetTunerParameters(a,b) Cobra_API_SetTunerParameters(a,b)
#define API_GetTunerParameter(a,b) Cobra_API_GetTunerParameter(a,b)
#define API_GetTunerType(a,b) Cobra_API_GetTunerType(a,b)
#define API_FindVCOEdges(a,b) Cobra_API_FindVCOEdges(a,b)
#define API_SetVCOEdges(a,b,c) Cobra_API_SetVCOEdges(a,b,c)
#define API_GetVCOEdges(a,b,c) Cobra_API_GetVCOEdges(a,b,c)
#define API_GetVCOBreakpoints(a,b,c,d) Cobra_API_GetVCOBreakpoints(a,b,c,d)
#define API_GetVCOBreakpointPercentage(a,b) Cobra_API_GetVCOBreakpointPercentage(a,b)
#define API_SetVCOBreakpointPercentage(a,b) Cobra_API_SetVCOBreakpointPercentage(a,b)
#define API_GetCrystalFreq(a,b) Cobra_API_GetCrystalFreq(a,b)
#define API_SetOutputOptions(a,b) Cobra_API_SetOutputOptions(a,b)
#define API_GetOutputOptions(a,b) Cobra_API_GetOutputOptions(a,b)
#define API_SetInterruptOptions(a,b) Cobra_API_SetInterruptOptions(a,b)
#define API_SetSearchRangeLimit(a,b,c) Cobra_API_SetSearchRangeLimit(a,b,c)
#define API_GetSearchRangeLimit(a,b) Cobra_API_GetSearchRangeLimit(a,b)
#define API_SetModulation(a,b) Cobra_API_SetModulation(a,b)
#define API_GetModulation(a,b) Cobra_API_GetModulation(a,b)
#define API_GetAssociatedSampleFrequency(a,b,c) Cobra_API_GetAssociatedSampleFrequency(a,b,c)
#define API_SetSampleFrequency(a,b) Cobra_API_SetSampleFrequency(a,b)
#define __API_SetSampleFrequency(a,b) Cobra___API_SetSampleFrequency(a,b)
#define API_GetSampleFrequency(a,b) Cobra_API_GetSampleFrequency(a,b)
#define API_SetTransportSpec(a,b) Cobra_API_SetTransportSpec(a,b)
#define API_GetTransportSpec(a,b) Cobra_API_GetTransportSpec(a,b)
#define API_SetDescramble(a,b) Cobra_API_SetDescramble(a,b)
#define API_GetDescramble(a,b) Cobra_API_GetDescramble(a,b)
#define API_SetSymbolRate(a,b) Cobra_API_SetSymbolRate(a,b)
#define API_GetSymbolRate(a,b) Cobra_API_GetSymbolRate(a,b)
#define API_GetMinSymbolRate(a,b) Cobra_API_GetMinSymbolRate(a,b)
#define API_GetMaxSymbolRate(a,b) Cobra_API_GetMaxSymbolRate(a,b)
#define API_SetViterbiRate(a,b) Cobra_API_SetViterbiRate(a,b)
#define API_GetViterbiRate(a,b) Cobra_API_GetViterbiRate(a,b)
#define API_SetSpectralInversion(a,b) Cobra_API_SetSpectralInversion(a,b)
#define API_GetSpectralInversion(a,b) Cobra_API_GetSpectralInversion(a,b)
#define API_AcqBegin(a) Cobra_API_AcqBegin(a)
#define API_AcqContinue(a,b) Cobra_API_AcqContinue(a,b)
#define API_AcqStop(a) Cobra_API_AcqStop(a)
#define API_AcqSoftReset(a) Cobra_API_AcqSoftReset(a)
#define API_AcqSetViterbiCodeRates(a,b) Cobra_API_AcqSetViterbiCodeRates(a,b)
#define API_AcqGetViterbiCodeRates(a,b) Cobra_API_AcqGetViterbiCodeRates(a,b)
#define API_AcqTrackingSetup(a) Cobra_API_AcqTrackingSetup(a)
#define API_AcqBinSizeList(a,b,c) Cobra_API_AcqBinSizeList(a,b,c)
#define API_GetPendingInterrupts(a,b) Cobra_API_GetPendingInterrupts(a,b)
#define _API_ClearPendingInterrupts(a,b) Cobra__API_ClearPendingInterrupts(a,b)
#define API_GetLockIndicators(a,b) Cobra_API_GetLockIndicators(a,b)
#define API_SetDemodErrorMode(a,b) Cobra_API_SetDemodErrorMode(a,b)
#define API_GetDemodErrorMode(a,b) Cobra_API_GetDemodErrorMode(a,b)
#define API_GetLastError(a) Cobra_API_GetLastError(a)
#define API_GetErrorMessage(a,b) Cobra_API_GetErrorMessage(a,b)
#define API_GetChannelEsNo(a,b,c,d) Cobra_API_GetChannelEsNo(a,b,c,d)
#define API_GetPNBER(a,b,c,d) Cobra_API_GetPNBER(a,b,c,d)
#define API_GetBER(a,b,c,d) Cobra_API_GetBER(a,b,c,d)
#define API_GetByteErrors(a,b,c,d) Cobra_API_GetByteErrors(a,b,c,d)
#define API_GetBlockErrors(a,b,c,d) Cobra_API_GetBlockErrors(a,b,c,d)
#define API_GetNormCount(a,b) Cobra_API_GetNormCount(a,b)
#define API_GetFrequencyOffset(a,b) Cobra_API_GetFrequencyOffset(a,b)
#define API_GetAcquisitionOffset(a,b) Cobra_API_GetAcquisitionOffset(a,b)
#define API_SetCentralFreq(a,b) Cobra_API_SetCentralFreq(a,b)
#define API_GetCentralFreq(a,b) Cobra_API_GetCentralFreq(a,b)
#define API_GetCTL(a,b) Cobra_API_GetCTL(a,b)
#define API_EnableRSCorrection(a,b) Cobra_API_EnableRSCorrection(a,b)
#define API_GetAGCAcc(a,b) Cobra_API_GetAGCAcc(a,b)
#define API_GetBTL(a,b) Cobra_API_GetBTL(a,b)
#define API_SetLNBDC(a,b) Cobra_API_SetLNBDC(a,b)
#define API_GetLNBDC(a,b) Cobra_API_GetLNBDC(a,b)
#define API_SetLNBMode(a,b) Cobra_API_SetLNBMode(a,b)
#define API_SetLNBTone(a,b) Cobra_API_SetLNBTone(a,b)
#define API_SendDiseqcMessage(a,b,c,d,e) Cobra_API_SendDiseqcMessage(a,b,c,d,e)
#define API_ReadReg(a,b,c) Cobra_API_ReadReg(a,b,c)
#define API_WriteReg(a,b,c) Cobra_API_WriteReg(a,b,c)
#define API_TstateMpegOutputs(a,b) Cobra_API_TstateMpegOutputs(a,b)
#define API_GetEffectiveFrequency(a,b) Cobra_API_GetEffectiveFrequency(a,b)
#define API_CalculatePLLFrequency(a,b,c,d) Cobra_API_CalculatePLLFrequency(a,b,c,d)
#define API_CalculatePdFrequency(a,b,c) Cobra_API_CalculatePdFrequency(a,b,c)
#define API_CalculateNAR(a,b,c,d,e) Cobra_API_CalculateNAR(a,b,c,d,e)
#define API_SetSleepMode(a,b) Cobra_API_SetSleepMode(a,b)
#define API_ConstOn(a,b) Cobra_API_ConstOn(a,b)
#define API_ConstOff(a) Cobra_API_ConstOff(a)
#define API_ConstCount(a) Cobra_API_ConstCount(a)
#define API_ConstSetBusy(a,b) Cobra_API_ConstSetBusy(a,b)
#define API_ConstGetBusy(a) Cobra_API_ConstGetBusy(a)
#define API_ConstGetPoints(a,b,c,d) Cobra_API_ConstGetPoints(a,b,c,d)
#define API_ConstGetIQSample(a,b,c) Cobra_API_ConstGetIQSample(a,b,c)
#define API_ConstGetUnbufferedIQSample(a,b,c) Cobra_API_ConstGetUnbufferedIQSample(a,b,c)
#define API_SetDriverWait(a,b) Cobra_API_SetDriverWait(a,b)
#define API_GetErrorFilename(a) Cobra_API_GetErrorFilename(a)
#define API_GetErrorLineNumber(a) Cobra_API_GetErrorLineNumber(a)
#ifdef OPTIMAL_FS_CODE                 /* Fs optimization */
#define API_Opt_Fs_Enable(a,b) Cobra_API_Opt_Fs_Enable(a,b)
#define API_Opt_Fs_Disable(a) Cobra_API_Opt_Fs_Disable(a)
#endif  /* #ifdef OPTIMAL_FS_CODE */
#define API_GetTunerStructure(a,b) Cobra_API_GetTunerStructure(a,b)

 /****************************************************************************
 * Modifications:
 *
 * $Log: 
 *  4    mpeg      1.3         3/16/04 11:40:46 AM    Billy Jackman   CR(s) 
 *        8571 8572 : Update Cobra driver to San Diego code revision 2.5.3.  
 *        Add API_GetTunerStructure API definition.
 *        Removed conditional compilation directives for defining 
 *        STRIP_REGNAMES and made striping register names the default action in
 *         cobra.h.
 *        Updated to StarTeam type keywords for expansion.
 *        
 *  3    mpeg      1.2         7/9/03 3:26:50 PM      Tim White       SCR(s) 
 *        6901 :
 *        Phase 3 codeldrext drop.
 *        
 *        
 *  2    mpeg      1.1         5/14/03 2:08:40 PM     Billy Jackman   SCR(s) 
 *        6336 6337 :
 *        Removed API_SetOutputIO and added API_GetLNBDC to correspond to the 
 *        API
 *        present in San Diego code release 2.4.2.0.
 *        
 *  1    mpeg      1.0         11/27/02 2:08:42 PM    Billy Jackman   
 * $
 * 
 ****************************************************************************/

#endif /* __APIFIX */

