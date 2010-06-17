/* cobra_api.h */

/*+++ *******************************************************************\
*
*   Copyright and Disclaimer:
*
*       ---------------------------------------------------------------
*       ALL SOFTWARE, APPLICATIONS, DOCUMENTATION, OR MATERIALS        
*       FURNISHED HEREIN IS PROVIDED *AS IS*.  CONEXANT DOES NOT MAKE  
*       ANY WARRANTIES, EITHER EXPRESS OR IMPLIED, AND HEREBY EXPRESSLY
*       DISCLAIMS ANY AND ALL SUCH WARRANTIES TO THE EXTENT PERMITTED  
*       BY LAW, INCLUDING, SPECIFICALLY, ANY IMPLIED WARRANTY ARISING  
*       BY STATUTE OR OTHERWISE IN LAW OR FROM A COURSE OF DEALING OR  
*       USAGE OF TRADE.  CONEXANT DOES NOT MAKE ANY WARRANTIES, EITHER 
*       EXPRESS OR IMPLIED, AND HEREBY EXPRESSLY DISCLAIMS ANY AND ALL 
*       SUCH WARRANTIES WITH RESPECT TO ALL SOFTWARE, APPLICATIONS,    
*       DOCUMENTATION, AND MATERIALS INCLUDING ALL IMPLIED WARRANTIES  
*       OF MERCHANTABILITY, OR OF MERCHANTABLE QUALITY, OR OF FITNESS  
*       FOR ANY PURPOSE, PARTICULAR, SPECIFIC OR OTHERWISE, OR OF      
*       NONINFRINGEMENT OF INTELLECTUAL PROPERTY RIGHTS OF OTHERS,     
*       RELATING TO THE SOFTWARE, APPLICATIONS, OPERATION,             
*       DOCUMENTATION, DATA OR RESULTS GENERATED BY THE OPERATION OR   
*       USE THEREOF, AND MATERIALS PROVIDED HEREIN.  THE ENTIRE RISK AS
*       TO THE SUBSTANCE, QUALITY AND PERFORMANCE OF SOFTWARE,         
*       APPLICATIONS, AND DOCUMENTATION DESCRIBING SUCH SOFTWARE       
*       REMAINS WITH THE BUYER.                                        
*                                                                      
*       REGARDLESS OF WHETHER ANY REMEDY SET FORTH HEREIN FAILS OF ITS 
*       ESSENTIAL PURPOSE OR OTHERWISE, CONEXANT SHALL NOT BE LIABLE   
*       FOR ANY EXEMPLARY, SPECIAL, PUNITIVE, SPECULATIVE, INDIRECT,   
*       CONSEQUENTIAL OR INCIDENTAL DAMAGES OF ANY KIND (INCLUDING     
*       WITHOUT LIMITATION LOST PROFITS, LOSS OF INCOME, LOSS OF       
*       GOODWILL, OR OTHER TANGIBLE OR INTANGIBLE BUSINESS LOSS)       
*       ARISING OUT OF OR IN CONNECTION WITH, DIRECTLY OR INDIRECTLY,  
*       SOFTWARE, APPLICATIONS, DOCUMENTATION, OR ANY SERVICES OR      
*       MATERIALS PROVIDED HEREUNDER, OR USE OR INABILITY TO USE THE   
*       SOFTWARE, EVEN IF CONEXANT HAS BEEN ADVISED OF THE POSSIBILITY 
*       OF SUCH DAMAGES.                                               
*
*       Copyright (c) 2001 Conexant Systems, Inc.
*       All Rights Reserved.
*       ---------------------------------------------------------------
*
*   Module Revision Id:
*
*       $Header: cobra_api.h, 5, 3/16/04 11:45:04 AM, Billy Jackman$
*
*   Abstract:
*
*       Contains prototypes for the Cobra API functions.
*
\******************************************************************* ---*/

#ifndef COBRA_API_H_DEFINED
#define COBRA_API_H_DEFINED

#ifdef __cplusplus
extern "C" {
#endif  /* __cplusplus */ 

/*******************************************************************************************************/
/* API prototypes */
/*******************************************************************************************************/
BOOL  API_InitEnvironment(NIM *nim,unsigned long demodhandle,void ( * SBwrite)(unsigned long ,unsigned char ,unsigned char ,unsigned long *),unsigned char ( * SBread)(unsigned long ,unsigned char ,unsigned long *),TRANSPEC transpec,BOOL (*TUNER_install)(NIM *nim),unsigned long crystalfreq,VCOINIT vcoinit,MPEG_OUT *mpeg_out,LNBMODE *lnbmode,BOOL (*waitfunct)(NIM *nim,int mscount));
BOOL  API_ChangeChannel(NIM *nim,CHANOBJ *chanobj);
BOOL  API_Monitor(NIM *nim,ACQSTATE *acqstate,LOCKIND *lockind);
BOOL  API_NIMGetChipInfo(NIM *nim,char **demod_string ,char **Tuner_string ,int *demod_type,int *tuner_type,int *board_type);
BOOL  API_GetDriverVersion(NIM *nim,VERDRV *verdrv);
BOOL  API_NIMGetTunerCount(int *tuner_count);
BOOL  API_NIMGetTunerIDs(int count,TUNER *tuner,char **tuner_strs );
BOOL  API_ReleaseEnvironment(NIM *nim);
BOOL  API_SetTunerFrequency(NIM *nim,unsigned long freq);
BOOL  API_GetTunerFrequency(NIM *nim,unsigned long *freq);
BOOL  API_GetPLLFrequency(NIM *nim,unsigned long *pllfreq);
BOOL  API_GetTunerVCO(NIM *nim,unsigned char *vcono);
BOOL  API_GetVCODivider(NIM *nim,VCODIV*);
BOOL  API_GetTunerReferenceDivider(NIM *nim,RDIVVAL *rvalue);
BOOL  API_SetTunerReferenceDivider(NIM *nim,RDIVVAL rvalue);
BOOL  API_SetTunerRegisters(NIM *nim,int nvalue,int avalue,RDIVVAL rvalue);
BOOL  API_GetTunerRegisters(NIM *nim,int *nvalue,int *avalue,RDIVVAL *rvalue);
BOOL  API_SetTunerParameters(NIM *nim,TUNERPARMS*);
BOOL  API_GetTunerParameter(NIM *nim,TUNERPARMS*);
BOOL  API_GetTunerType(NIM *nim,TUNER *tunertype);
BOOL  API_FindVCOEdges(NIM *nim,RDIVVAL rdiv);
BOOL  API_SetVCOEdges(NIM *nim,VCO_EDGE *vcoedge,int *vcoedgecount);
BOOL  API_GetVCOEdges(NIM *nim,VCO_EDGE *vcoedge,int *vcoedgecount);
BOOL  API_GetVCOBreakpoints(NIM *nim,int bp_percentage,VCO_EDGE *vcoedge,VCO_BP*);
BOOL  API_GetVCOBreakpointPercentage(NIM *nim,int *bp_percentage);
BOOL  API_SetVCOBreakpointPercentage(NIM *nim,int bp_percentage);
BOOL  API_GetCrystalFreq(NIM *nim,unsigned long *frequency);
BOOL  API_SetOutputOptions(NIM *nim,MPEG_OUT *mpeg_out);
BOOL  API_GetOutputOptions(NIM *nim,MPEG_OUT *mpeg_out);
BOOL  API_SetInterruptOptions(NIM *nim,INTEROPTS interopts);
BOOL  API_SetSearchRangeLimit(NIM *nim,unsigned long lnboffset,unsigned long *actual);
BOOL  API_GetSearchRangeLimit(NIM *nim,unsigned long *lnboffset);
BOOL  API_SetModulation(NIM *nim,MODTYPE modtype);
BOOL  API_GetModulation(NIM *nim,MODTYPE *modtype);
BOOL  API_GetAssociatedSampleFrequency(NIM *nim, SAMPFRQ sampfrq, unsigned long *AssdFs);
BOOL  API_SetSampleFrequency(NIM *nim,SAMPFRQ sampfrq);
BOOL  __API_SetSampleFrequency(NIM *nim,unsigned long sampratehz);
BOOL  API_GetSampleFrequency(NIM *nim,unsigned long *samplerate);
BOOL  API_SetTransportSpec(NIM *nim,TRANSPEC transpec);
BOOL  API_GetTransportSpec(NIM *nim,TRANSPEC *transpec);
BOOL  API_SetDescramble(NIM *nim,DESCRMB descramble);
BOOL  API_GetDescramble(NIM *nim,DESCRMB *descramble);
BOOL  API_SetSymbolRate(NIM *nim,SYMBRATE symbolrate);
BOOL  API_GetSymbolRate(NIM *nim,SYMBRATE *symbolrate);
BOOL  API_GetMinSymbolRate(NIM *nim,unsigned long *minsymbolrate);
BOOL  API_GetMaxSymbolRate(NIM *nim,unsigned long *maxsymbolrate);
BOOL  API_SetViterbiRate(NIM *nim,CODERATE coderate);
BOOL  API_GetViterbiRate(NIM *nim,CODERATE *coderate);
BOOL  API_SetSpectralInversion(NIM *nim,SPECINV specinv);
BOOL  API_GetSpectralInversion(NIM *nim,SPECINV *specinv);
BOOL  API_AcqBegin(NIM *nim);
BOOL  API_AcqContinue(NIM *nim,ACQSTATE *acqstate);
BOOL  API_AcqStop(NIM *nim);
BOOL  API_AcqSoftReset(NIM *nim);
BOOL  API_AcqSetViterbiCodeRates(NIM *nim,unsigned int vcr);
BOOL  API_AcqGetViterbiCodeRates(NIM *nim,unsigned int *vcr);
BOOL  API_AcqTrackingSetup(NIM *nim);
BOOL  API_AcqBinSizeList(NIM *nim,SYMBRATE symbrate,BINLIST *binlist);
BOOL  API_GetPendingInterrupts(NIM *nim,INTEROPTS *interopts);
BOOL  _API_ClearPendingInterrupts(NIM *nim,INTEROPTS intropts);
BOOL  API_GetLockIndicators(NIM *nim,LOCKIND *lockind);
BOOL  API_SetDemodErrorMode(NIM *nim,ERRORMODE errmode);
BOOL  API_GetDemodErrorMode(NIM *nim,ERRORMODE *errmode);
int   API_GetLastError(NIM *nim);
char * API_GetErrorMessage(NIM *nim,APIERRNO __errno);
BOOL  API_GetChannelEsNo(NIM *nim,ESNOMODE emode,CMPLXNO *esno,MSTATUS *mstat);
BOOL  API_GetPNBER(NIM *nim,PNBER errwindow,CMPLXNO *pnber,MSTATUS *mstat);
BOOL  API_GetBER(NIM *nim,unsigned long errwindow,CMPLXNO *ber,MSTATUS *mstat);
BOOL  API_GetByteErrors(NIM *nim,unsigned long errwindow,CMPLXNO *byteerr,MSTATUS *mstat);
BOOL  API_GetBlockErrors(NIM *nim,unsigned long errwindow,CMPLXNO *blockerr,MSTATUS *mstat);
BOOL  API_GetNormCount(NIM *nim,unsigned char *normcounter);
BOOL  API_GetFrequencyOffset(NIM *nim,long *freqoffset);
BOOL  API_GetAcquisitionOffset(NIM *nim,long *lnboffset);
BOOL  API_SetCentralFreq(NIM *nim,long centralfreq);
BOOL  API_GetCentralFreq(NIM *nim,long *centralfreq);
BOOL  API_GetCTL(NIM *nim,long *ctl);
BOOL  API_EnableRSCorrection(NIM *nim,BOOL opt);
BOOL  API_GetAGCAcc(NIM *nim,AGCACC *agcacc);
BOOL  API_GetBTL(NIM *nim,long *btl);
BOOL  API_SetLNBDC(NIM *nim,LNBPOL lnbpol);
BOOL  API_GetLNBDC(NIM *nim,LNBPOL *lnbpol);
BOOL  API_SetLNBMode(NIM *nim,LNBMODE *lnbmode);
BOOL  API_SetLNBTone(NIM *nim,LNBTONE lnbtone);
BOOL  API_GetLNBTone(NIM *nim,LNBTONE *lnbtone);
BOOL  API_SendDiseqcMessage(NIM *nim,unsigned char *message,unsigned char message_length,BOOL last_message,LNBBURST bursttype);
BOOL  API_ReadReg(NIM *nim,int reg,unsigned char *data);
BOOL  API_WriteReg(NIM *nim,int reg,unsigned char *data);
BOOL  API_TstateMpegOutputs(NIM *nim,TSTATE tvalue);
BOOL  API_GetEffectiveFrequency(NIM *nim,unsigned long *effectfrq);
BOOL  API_CalculatePLLFrequency(NIM *nim,unsigned int nreg,unsigned int areg,unsigned long *pllfreq);
BOOL  API_CalculatePdFrequency(NIM *nim,int rreg,unsigned long *pdfreq);
BOOL  API_CalculateNAR(NIM *nim,unsigned long Fdesired,RDIVVAL R,unsigned int *N,unsigned int *A);
BOOL  API_SetSleepMode(NIM *nim,BOOL sleep);
BOOL  API_GetTunerStructure (NIM *nim, ACTIVE_TUNER* tuner);

#ifdef INCLUDE_CONSTELLATION
BOOL  API_ConstOn(NIM *nim,IQPAK *iqpak);
BOOL  API_ConstOff(NIM *nim);
int   API_ConstCount(NIM *nim);
BOOL  API_ConstSetBusy(NIM *nim,BOOL busy_state);
BOOL  API_ConstGetBusy(NIM *nim);
BOOL  API_ConstGetPoints(NIM *nim,unsigned char *ivals,unsigned char *qvals,int iqcount);
BOOL  API_ConstGetIQSample(NIM *nim,unsigned char *I,unsigned char *Q);
BOOL  API_ConstGetUnbufferedIQSample(NIM *nim,signed char *I,signed char *Q);
#endif  /* #ifdef INCLUDE_CONSTELLATION */

BOOL  API_SetDriverWait(NIM *nim,BOOL (*waitfunct)(NIM *nim,int mscount));
char *API_GetErrorFilename(NIM *nim);
unsigned long API_GetErrorLineNumber(NIM *nim);
#ifdef OPTIMAL_FS_CODE                 /* Fs optimization */
BOOL  API_Opt_Fs_Enable(NIM *nim,BOOL flag);
BOOL  API_Opt_Fs_Disable(NIM *nim);
#endif  /* #ifdef OPTIMAL_FS_CODE */

/*******************************************************************************************************/
/* Cx24123 specific APIs */
/*******************************************************************************************************/
#ifdef CAMARIC_FEATURES
#ifdef INCLUDE_DISEQC2
BOOL  API_DiseqcReceiveMessage(NIM *nim, unsigned char *buffer, int buffer_len, int *received_len, RXMODE rxmode, int *parityerrors);
BOOL  API_DiseqcSetRxMode(NIM *nim, RXMODE rxmode);
BOOL  API_DiseqcGetRxMode(NIM *nim, RXMODE *rxmode);
BOOL  API_Diseqc22KHzSetAmplitude(NIM *nim, int amplitude);
BOOL  API_Diseqc22KHzGetAmplitude(NIM *nim, int *amplitude);
BOOL  API_DiseqcSetVersion(NIM *nim, DISEQC_VER dv);
BOOL  API_DiseqcGetVersion(NIM *nim, DISEQC_VER *dv);
BOOL  API_SetDiseqcInterrupt(NIM *nim,BOOL on);
BOOL  API_GetDiseqcInterrupt(NIM *nim,INTEROPTS *interopts);
BOOL  API_ClearDiseqcInterrupt(NIM *nim);
#endif /* #ifdef INCLUDE_DISEQC2 */

BOOL  API_LNBSetDrain(NIM *nim, LNBDRAIN  lnbdrain);
BOOL  API_LNBGetDrain(NIM *nim, LNBDRAIN  *lnbdrain);
BOOL  API_SCESetWindowSize(NIM *nim, int winsize);
BOOL  API_SCEGetWindowSize(NIM *nim, int *winsize);
BOOL  API_SCEGetSymbolCount(NIM *nim, long *intportion, long *fracportion);
BOOL  API_SetDemodClockOut(NIM *nim, DEMODCLK  setting);
BOOL  API_GetDemodClockOut(NIM *nim, DEMODCLK  *setting);
BOOL  API_EchostarLegacyPolarity(NIM *nim, ECHOPOLARITY  polarity);
BOOL  API_EchostarLegacySendMessage(NIM *nim, char *buffer, int len, ECHOPOLARITY polarity, int *result, ECHOWINDOW window);

/*******************************************************************************************************/
/* (end-of-Cx24123 specific APIs) */
/*******************************************************************************************************/
#endif  /* #ifdef CAMARIC_FEATURES */

/*******************************************************************************************************/
/*******************************************************************************************************/
/*******************************************************************************************************/

#ifdef __cplusplus
}
#endif

/*******************************************************************************************************/
/*******************************************************************************************************/
/*******************************************************************************************************/

#endif  /* #ifndef COBRA_API_H_DEFINED */ 
/* CR 9509 : Add an extra newline */
