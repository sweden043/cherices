/****************************************************************************/
/*                   CONEXANT PROPRIETARY AND CONFIDENTIAL                  */
/*                       SOFTWARE FILE/MODULE HEADER                        */
/*          Conexant Systems Inc. (c) 1998, 1999, 2000, 2001, 2002          */
/*                              Austin, TX                                  */
/*                         All Rights Reserved                              */
/****************************************************************************/
/*
 * Filename:        dvb.c
 *
 *
 * Description:     DVB Pawser microcode
 *
 *
 * Author:          Tim Ross
 *
 ****************************************************************************/
/* $Id:dvb.c,v 1.42, 2004-08-17 23:11:22Z, Tim White$
 ****************************************************************************/

#include "stbcfg.h"
#include "basetype.h"

/*
 ****************************************************************************
 *
 * How to build the microcode:
 * ===========================
 *
 * pawasm -l -a -c <flags> dvb.s
 * rename dvb.c <filename>.h
 *
 * Version information:
 * ====================
 *
 * Version 2.X.X is legacy microcode
 * Version 3.X.X is current microcode
 * 2.6.D is built from the same level of source as 3.0.7
 * 2.6.E is built from the same level of source as 3.0.8
 *   ...
 * Legacy means not-MULTI-PSI (i.e. single/global PSI buffer)
 *
 * Flag meaning:
 * =============
 *
 * Build Option      Naming Indicator      Cap Bit      Notes
 * ------------      ----------------      -------      -----
 * -l                                                   Generate listing file
 * -a                                                   Generate ascii memory file
 * -c                                                   Generate .c (.h) file
 *   (default)           dvb_a                          Target: Colorado
 * -dREVB                dvb_b                          Target: Colorado DVR
 * -dHONDO               dvb_c                          Target: Hondo/Wabash
 * -dBRAZOS              dvb_d                          Target: Brazos
 * -dDEBUG                d                    0        internal
 * -dMULTI_PSI            p                             default
 * -dXPRT                                      2        default
 * -dCAM                  n                    4
 * -dDVR                  r                    6
 * -dAUD_2CH              2                    7
 * -dAUD_3CH              3                    8
 * -dFILTER_888           c                    9
 * -dNAV                                      10        obsolete
 * -dPASSAGE                                  11        Sony Passage capable
 * -dDES                  s                   15        DES ECB descrambling
 * -dPLAY                 _pvr_ply            16
 * -dAC3_SPDIF_FIX        _ac3                17
 * -dDESCRAM_FIX                              18
 * -dKEY64                k                   20
 * -dPCR_PID_INT                              21        default
 * -dDMA_MUX              x                   22
 * -dPES_FRAME_RATE       f                   23        Pes frame_rate return on Colorado
 *                                                      RevF only.  On by default on 
 *                                                      other chips.
 * -dAUD_KEY_INT                              24        not used
 * -dAUD_PES_FIX                              25        not used
 * -dBAD_PES_INT                              26        default
 * -dPES_PID_INT                              27        default
 * -dDISCARD_BAD_PES                                    default
 * -dCX24110_FIX                              28
 * -dCRC_NOTIFY                               29        default
 * -dBOOT_LOADER          l                             boot_loader
 * -dSCRAM_STATUS                             31          
 * -dFILTER_ON_LENGTH     i          
 * -dAUDIO_STREAM_ID                                    default (except DVR)
 * -dPES_HDR_INT                                        default
 *                        f                             Colorado only
 *
 * Versions Available:
 * ===================
 * dvb_ap.h              - Colorado, Generic
 *   pawasm -l -a -c -dMULTI_PSI -dXPRT -dCRC_NOTIFY -dAUDIO_STREAM_ID -dPCR_PID_INT -dBAD_PES_INT -dPES_PID_INT -dPES_HDR_INT -dDESCRAM_FIX dvb.s
 * dvb_apf.h             - Colorado RevF, Generic
 *   pawasm -l -a -c -dMULTI_PSI -dXPRT -dCRC_NOTIFY -dAUDIO_STREAM_ID -dPCR_PID_INT -dBAD_PES_INT -dPES_PID_INT -dPES_HDR_INT -dPES_FRAME_RATE dvb.s
 * dvb_cp.h              - Wabash, Generic
 *   pawasm -l -a -c -dMULTI_PSI -dXPRT -dCRC_NOTIFY -dAUDIO_STREAM_ID -dPCR_PID_INT -dBAD_PES_INT -dPES_PID_INT -dPES_HDR_INT -dHONDO -dAC3 -dPES_FRAME_RATE dvb.s
 * dvb_dp.h              - Brazos, Generic
 *   pawasm -l -a -c -dMULTI_PSI -dXPRT -dCRC_NOTIFY -dAUDIO_STREAM_ID -dPCR_PID_INT -dBAD_PES_INT -dPES_PID_INT -dPES_HDR_INT -dBRAZOS -dAC3 -dPES_FRAME_RATE dvb.s
 * dvb_dkp.h             - Brazos, Key64
 *   pawasm -l -a -c -dMULTI_PSI -dXPRT -dCRC_NOTIFY -dAUDIO_STREAM_ID -dPCR_PID_INT -dBAD_PES_INT -dPES_PID_INT -dPES_HDR_INT -dBRAZOS -dAC3 -dPES_FRAME_RATE -dKEY64 -dSCRAM_STATUS dvb.s
 * 
 * dvb_apx.h             - Colorado, runtime input mux selection capable
 *   pawasm -l -a -c -dMULTI_PSI -dXPRT -dCRC_NOTIFY -dAUDIO_STREAM_ID -dPCR_PID_INT -dBAD_PES_INT -dPES_PID_INT -dPES_HDR_INT -dDMA_MUX -dDESCRAM_FIX dvb.s
 * dvb_apxf.h            - Colorado RevF, runtime input mux selection capable
 *   pawasm -l -a -c -dMULTI_PSI -dXPRT -dCRC_NOTIFY -dAUDIO_STREAM_ID -dPCR_PID_INT -dBAD_PES_INT -dPES_PID_INT -dPES_HDR_INT -dDMA_MUX -dPES_FRAME_RATE dvb.s
 * 
 * dvb_acp_pssg.h        - Colorado, Canal+, Sony Passage
 *   pawasm -l -a -c -dMULTI_PSI -dXPRT -dCRC_NOTIFY -dAUDIO_STREAM_ID -dFILTER_888 -dPCR_PID_INT -dBAD_PES_INT -dPES_PID_INT -dPES_HDR_INT -dDESCRAM_FIX -dPASSAGE dvb.s
 * dvb_acpf_pssg.h       - Colorado RevF, Canal+, Sony Passage
 *   pawasm -l -a -c -dMULTI_PSI -dXPRT -dCRC_NOTIFY -dAUDIO_STREAM_ID -dFILTER_888 -dPCR_PID_INT -dBAD_PES_INT -dPES_PID_INT -dPES_HDR_INT -dPES_FRAME_RATE -dPASSAGE dvb.s
 * dvb_ccp_pssg.h        - Wabash, Canal+, Sony Passage
 *   pawasm -l -a -c -dMULTI_PSI -dXPRT -dCRC_NOTIFY -dAUDIO_STREAM_ID -dFILTER_888 -dPCR_PID_INT -dBAD_PES_INT -dPES_PID_INT -dPES_HDR_INT -dHONDO -dAC3 -dPES_FRAME_RATE -dPASSAGE dvb.s
 * dvb_dcp_pssg.h        - Brazos, Canal+, Sony Passage
 *   pawasm -l -a -c -dMULTI_PSI -dXPRT -dCRC_NOTIFY -dAUDIO_STREAM_ID -dFILTER_888 -dPCR_PID_INT -dBAD_PES_INT -dPES_PID_INT -dPES_HDR_INT -dBRAZOS -dAC3 -dPES_FRAME_RATE -dPASSAGE dvb.s
 *
 * dvb_acp.h             - Colorado, Canal+
 *   pawasm -l -a -c -dMULTI_PSI -dXPRT -dCRC_NOTIFY -dAUDIO_STREAM_ID -dFILTER_888 -dPCR_PID_INT -dBAD_PES_INT -dPES_PID_INT -dPES_HDR_INT -dDESCRAM_FIX dvb.s
 * dvb_acpf.h            - Colorado RevF, Canal+
 *   pawasm -l -a -c -dMULTI_PSI -dXPRT -dCRC_NOTIFY -dAUDIO_STREAM_ID -dFILTER_888 -dPCR_PID_INT -dBAD_PES_INT -dPES_PID_INT -dPES_HDR_INT -dPES_FRAME_RATE dvb.s
 * dvb_ccp.h             - Wabash, Canal+
 *   pawasm -l -a -c -dMULTI_PSI -dXPRT -dCRC_NOTIFY -dAUDIO_STREAM_ID -dFILTER_888 -dPCR_PID_INT -dBAD_PES_INT -dPES_PID_INT -dPES_HDR_INT -dHONDO -dAC3 -dPES_FRAME_RATE dvb.s
 * dvb_ccps.h            - Wabash, Canal+, DES ECB
 *   pawasm -l -a -c -dMULTI_PSI -dXPRT -dCRC_NOTIFY -dAUDIO_STREAM_ID -dFILTER_888 -dPCR_PID_INT -dBAD_PES_INT -dPES_PID_INT -dPES_HDR_INT -dHONDO -dAC3 -dDES -dPES_FRAME_RATE dvb.s
 * dvb_cps.h             - Wabash, DES ECB
 *   pawasm -l -a -c -dMULTI_PSI -dXPRT -dCRC_NOTIFY -dAUDIO_STREAM_ID -dPCR_PID_INT -dBAD_PES_INT -dPES_PID_INT -dPES_HDR_INT -dHONDO -dAC3 -dDES -dPES_FRAME_RATE dvb.s
 * dvb_dcp.h             - Brazos, Canal+
 *   pawasm -l -a -c -dMULTI_PSI -dXPRT -dCRC_NOTIFY -dAUDIO_STREAM_ID -dFILTER_888 -dPCR_PID_INT -dBAD_PES_INT -dPES_PID_INT -dPES_HDR_INT -dBRAZOS -dAC3 -dPES_FRAME_RATE dvb.s
 * 
 * dvb_acpk.h           - Colorado, Canal+, Key64
 *   pawasm -l -a -c -dMULTI_PSI -dSCRAM_STATUS -dXPRT -dCRC_NOTIFY -dAUDIO_STREAM_ID -dFILTER_888 -dKEY64 -dPCR_PID_INT -dBAD_PES_INT -dPES_PID_INT -dPES_HDR_INT -dDESCRAM_FIX dvb.s
 * dvb_acpkf.h           - Colorado RevF, Canal+, Key64
 *   pawasm -l -a -c -dMULTI_PSI -dSCRAM_STATUS -dXPRT -dCRC_NOTIFY -dAUDIO_STREAM_ID -dFILTER_888 -dKEY64 -dPCR_PID_INT -dBAD_PES_INT -dPES_PID_INT -dPES_HDR_INT -dPES_FRAME_RATE dvb.s
 * 
 * dvb_anp.h             - Colorado, NDS
 *   pawasm -l -a -c -dMULTI_PSI -dXPRT -dCRC_NOTIFY -dAUDIO_STREAM_ID -dCAM -dPCR_PID_INT -dBAD_PES_INT -dPES_PID_INT -dPES_HDR_INT -dDESCRAM_FIX dvb.s
 * dvb_anpf.h             - Colorado RevF, NDS
 *   pawasm -l -a -c -dMULTI_PSI -dXPRT -dCRC_NOTIFY -dAUDIO_STREAM_ID -dCAM -dPCR_PID_INT -dBAD_PES_INT -dPES_PID_INT -dPES_HDR_INT -dPES_FRAME_RATE dvb.s
 * dvb_cnp.h             - Wabash, NDS
 *   pawasm -l -a -c -dMULTI_PSI -dXPRT -dCRC_NOTIFY -dAUDIO_STREAM_ID -dCAM -dPCR_PID_INT -dBAD_PES_INT -dPES_PID_INT -dPES_HDR_INT -dHONDO -dAC3 -dPES_FRAME_RATE dvb.s
 * dvb_dnp.h             - Brazos, NDS
 *   pawasm -l -a -c -dMULTI_PSI -dXPRT -dCRC_NOTIFY -dAUDIO_STREAM_ID -dCAM -dPCR_PID_INT -dBAD_PES_INT -dPES_PID_INT -dPES_HDR_INT -dBRAZOS -dAC3 -dPES_FRAME_RATE dvb.s
 * 
 * dvb_cnp_xtv.h         - Hondo, Wabash XTV Record
 *   pawasm -l -a -c -dXTV -dCAM -dPCR_PID_INT -dCRC_NOTIFY -dBAD_PES_INT -dPES_PID_INT -dHONDO -dMULTI_PSI dvb.s
 * dvb_cnp_xtv_ply.h     - Hondo, Wabash XTV Play
 *   pawasm -l -a -c -dXTV_PLAY -dMULTI_PSI -dXPRT -dCRC_NOTIFY -dAUDIO_STREAM_ID -dCAM -dPCR_PID_INT -dBAD_PES_INT -dPES_PID_INT -dPES_HDR_INT -dHONDO -dAC3 -dPES_FRAME_RATE dvb.s
 * 
 * dvb_ancp.h            - Colorado, NDS, Canal+
 *   pawasm -l -a -c -dMULTI_PSI -dXPRT -dCRC_NOTIFY -dAUDIO_STREAM_ID -dCAM -dFILTER_888 -dPCR_PID_INT -dBAD_PES_INT -dPES_PID_INT -dPES_HDR_INT -dDESCRAM_FIX dvb.s
 * dvb_ancpf.h            - Colorado RevF, NDS, Canal+
 *   pawasm -l -a -c -dMULTI_PSI -dXPRT -dCRC_NOTIFY -dAUDIO_STREAM_ID -dCAM -dFILTER_888 -dPCR_PID_INT -dBAD_PES_INT -dPES_PID_INT -dPES_HDR_INT -dPES_FRAME_RATE dvb.s
 * dvb_cncp.h            - Wabash, NDS, Canal+
 *   pawasm -l -a -c -dMULTI_PSI -dXPRT -dCRC_NOTIFY -dAUDIO_STREAM_ID -dCAM -dFILTER_888 -dPCR_PID_INT -dBAD_PES_INT -dPES_PID_INT -dPES_HDR_INT -dHONDO -dAC3 -dPES_FRAME_RATE dvb.s
 * dvb_cncpi.h           - Wabash, NDS, Canal+, Filter on Section Length
 *   pawasm -l -a -c -dMULTI_PSI -dXPRT -dCRC_NOTIFY -dAUDIO_STREAM_ID -dCAM -dFILTER_888 -dPCR_PID_INT -dBAD_PES_INT -dPES_PID_INT -dPES_HDR_INT -dFILTER_ON_LENGTH -dHONDO -dAC3 -dPES_FRAME_RATE dvb.s
 * dvb_dncp.h            - Brazos, NDS, Canal+
 *   pawasm -l -a -c -dMULTI_PSI -dXPRT -dCRC_NOTIFY -dAUDIO_STREAM_ID -dCAM -dFILTER_888 -dPCR_PID_INT -dBAD_PES_INT -dPES_PID_INT -dPES_HDR_INT -dBRAZOS -dAC3 -dPES_FRAME_RATE dvb.s
 * 
 * dvb_cp_pvr_ply.h      - Hondo, Wabash PVR Play
 *   pawasm -l -a -c -dMULTI_PSI -dXPRT -dCRC_NOTIFY -dAUDIO_STREAM_ID -dPCR_PID_INT -dBAD_PES_INT -dPES_PID_INT -dHONDO dvb.s
 * dvb_cp_pvr_rec.h      - Hondo, Wabash PVR Record
 *   pawasm -l -a -c -dREC -dPCR_PID_INT -dCRC_NOTIFY -dBAD_PES_INT -dPES_PID_INT -dHONDO -dMULTI_PSI dvb.s
 *
 * dvb_anc.h             - Colorado Legacy NDS, Canal+
 *   pawasm -l -a -c -dXPRT -dCAM -dFILTER_888 -dDESCRAM_FIX dvb.s
 * 
 * dvb_br.h              - Colorado Legacy DVR 1 aud ch
 *   pawasm -l -a -c -dNAV -dDVR -dREVB -dDESCRAM_FIX dvb.s
 * dvb_br2.h             - Colorado Legacy DVR 2 aud ch
 *   pawasm -l -a -c -dNAV -dDVR -dREVB -dDESCRAM_FIX -dAUD_2CH dvb.s
 * dvb_br3.h             - Colorado Legacy DVR 3 aud ch
 *   pawasm -l -a -c -dNAV -dDVR -dREVB -dDESCRAM_FIX -dAUD_3CH dvb.s
 * 
 * dvb_bpv.h             - Colorado Legacy DVR 1 aud ch, Multi-PSI, XPRT, 12:12
 *   pawasm -l -a -c -dXPRT -dDVR -dREVB -dMULTI_PSI -dCRC_NOTIFY -dDESCRAM_FIX dvb.s
 * dvb_bpv2.h            - Colorado Legacy DVR 2 aud ch, Multi-PSI, XPRT, 12:12
 *   pawasm -l -a -c -dXPRT -dDVR -dREVB -dMULTI_PSI -dCRC_NOTIFY -dDESCRAM_FIX -dAUD_2CH dvb.s
 * dvb_bpv3.h            - Colorado Legacy DVR 3 aud ch, Multi-PSI, XPRT, 12:12
 *   pawasm -l -a -c -dXPRT -dDVR -dREVB -dMULTI_PSI -dCRC_NOTIFY -dDESCRAM_FIX -dAUD_3CH dvb.s
 * 
 * dvb_bpxv.h            - Colorado Legacy DVR 1 aud ch, Multi-PSI, XPRT, 12:12, DMA NIM
 *   pawasm -l -a -c -dXPRT -dDVR -dREVB -dMULTI_PSI -dCRC_NOTIFY -dDMA_MUX -dDESCRAM_FIX dvb.s
 * dvb_bpxv2.h           - Colorado Legacy DVR 2 aud ch, Multi-PSI, XPRT, 12:12, DMA NIM
 *   pawasm -l -a -c -dXPRT -dDVR -dREVB -dMULTI_PSI -dCRC_NOTIFY -dAUD_2CH -dDMA_MUX -dDESCRAM_FIX dvb.s
 * dvb_bpxv3.h           - Colorado Legacy DVR 3 aud ch, Multi-PSI, XPRT, 12:12, DMA NIM
 *   pawasm -l -a -c -dXPRT -dDVR -dREVB -dMULTI_PSI -dCRC_NOTIFY -dAUD_3CH -dDMA_MUX -dDESCRAM_FIX dvb.s
 * 
 * dvb_bcpv.h            - Colorado Legacy DVR 1 aud ch, Multi-PSI, XPRT, 8:8:8 -> MHP
 *   pawasm -l -a -c -dXPRT -dDVR -dREVB -dMULTI_PSI -dCRC_NOTIFY -dFILTER_888 -dDESCRAM_FIX dvb.s
 * dvb_bcpv2.h           - Colorado Legacy DVR 2 aud ch, Multi-PSI, XPRT, 8:8:8 -> MHP
 *   pawasm -l -a -c -dXPRT -dDVR -dREVB -dMULTI_PSI -dCRC_NOTIFY -dFILTER_888 -dAUD_2CH -dDESCRAM_FIX dvb.s
 * dvb_bcpv3.h           - Colorado Legacy DVR 3 aud ch, Multi-PSI, XPRT, 8:8:8 -> MHP
 *   pawasm -l -a -c -dXPRT -dDVR -dREVB -dMULTI_PSI -dCRC_NOTIFY -dFILTER_888 -dAUD_3CH -dDESCRAM_FIX dvb.s
 *
 * dvb_apl.h             - Colorado Bootloader only code (no PES, no A/V)
 *   pawasm -l -a -c -dMULTI_PSI -dBOOT_LOADER dvb.s
 * dvb_cpl.h             - Wabash Bootloader only code (no PES, no A/V)
 *   pawasm -l -a -c -dMULTI_PSI -dBOOT_LOADER -dHONDO dvb.s
 * dvb_dpl.h             - Brazos Bootloader only code (no PES, no A/V)
 *   pawasm -l -a -c -dMULTI_PSI -dBOOT_LOADER -dBRAZOS dvb.s
 * 
 * dvb_a.h               - Colorado Legacy Generic
 *   pawasm -a -c -l -dXPRT -dNAV -dPES_FRAME_RATE dvb.s
 *
 * dvb_an.h              - Colorado Legacy NDS
 *   pawasm -a -c -l -dXPRT -dNAV -dNDS -dPES_FRAME_RATE dvb.s
 *
 * dvb_a_ac3.h           - Colorado Legacy Generic w/ AC3 fix
 *   pawasm -a -c -l -dXPRT -dNAV -dAC3_SPDIF_FIX dvb.s
 * 
 ****************************************************************************
 */

/*
 * Assertions
 */
#if (PARSER_PSI_BUFFER==SINGLE_PSI) && (DMXVER==DEMUX)
 #error "Attempted to build new demux driver with single buffer PSI support!"
#endif

#if (PVR==YES) && (DMXVER==GENDMXC)
 #error "Attempted to build PVR support with legacy demux driver!"
#endif

#if (PARSER_NAV_SUPPORT==YES) && (DMXVER==DEMUX)
 #error "Attempted to build NAV support with new demux driver!"
#endif

#if (PVR==YES) && (PARSER_NDS_ICAM==YES)
 #error "Attempted to build PVR support with NDS ICAM!"
#endif

#if (PARSER_PSI_BUFFER==MULTI_PSI) && (PARSER_NAV_SUPPORT==YES)
 #error "Attempted to build NAV support with multi buffer PSI support!"
#endif

#if (LEGACY_DVR==YES) && (PARSER_MICROCODE!=UCODE_COLORADO)
 #error "Attempted to build legacy DVR support for !COLORADO chip!"
#endif

#if (XTV_SUPPORT==YES) && (PARSER_NDS_ICAM==NO)
 #error "Attempted to build XTV support without NDS ICAM!"
#endif

/*
 * Inclusions
 */

#if (PARSER_BOOTLOADER==YES)
  #if (PARSER_MICROCODE==UCODE_COLORADO)
    #if (CHIP_REV==AUTOSENSE)
      const u_int32 PawserMicrocode[] =
      #include "dvb_apl.h"
      const u_int32 MicrocodeSize = sizeof(PawserMicrocode);
      const u_int32 RevCPawserMicrocode[] =
      #include "dvb_apl.h"
      const u_int32 RevCMicrocodeSize = sizeof(RevCPawserMicrocode);
    #elif (CHIP_REV==REV_C_COLO)
      const u_int32 PawserMicrocode[] =
      #include "dvb_apl.h"
      const u_int32 MicrocodeSize = sizeof(PawserMicrocode);
    #else
      const u_int32 PawserMicrocode[] =
      #include "dvb_apl.h"
      const u_int32 MicrocodeSize = sizeof(PawserMicrocode);
    #endif
  #elif (PARSER_MICROCODE==UCODE_HONDO)
    const u_int32 PawserMicrocode[] =
    #include "dvb_cpl.h"
    const u_int32 MicrocodeSize = sizeof(PawserMicrocode);
  #elif (PARSER_MICROCODE==UCODE_BRAZOS)
    const u_int32 PawserMicrocode[] =
    #include "dvb_dpl.h"
    const u_int32 MicrocodeSize = sizeof(PawserMicrocode);
  #else
    #error "Microcode not supported!"
  #endif
#else

#if (((PVR==YES)||((PARSER_MICROCODE==UCODE_HONDO)&&defined(DMXREC))) && (DMXVER==DEMUX) && (PARSER_PSI_BUFFER==MULTI_PSI) && (PARSER_FILTERING==FILTER_1212) && (PARSER_NDS_ICAM==NO))

  /*
   * PVR Microcode
   */

  #if (PARSER_MICROCODE==UCODE_HONDO)
    const u_int32 RecPawserMicrocode[] =
    #include "dvb_cp_pvr_rec.h"
    const u_int32 RecPawserMicrocodeSize = sizeof(RecPawserMicrocode);
    const u_int32 PlayPawserMicrocode[] =
    #include "dvb_cp_pvr_ply.h"
    const u_int32 PlayPawserMicrocodeSize = sizeof(PlayPawserMicrocode);
  #else
    #error "Microcode not supported!"
  #endif

#elif ((XTV_SUPPORT==YES) && (PARSER_PSI_BUFFER==MULTI_PSI) && (PARSER_FILTERING==FILTER_1212) && (PARSER_NDS_ICAM==YES) && (PARSER_XPRT_BUFFER==YES))

  /*
   * XTV Microcode
   */

  #if (PARSER_MICROCODE==UCODE_HONDO)
    const u_int32 RecPawserMicrocode[] =
    #include "dvb_cnp_xtv.h"
    const u_int32 RecPawserMicrocodeSize = sizeof(RecPawserMicrocode);
    const u_int32 PlayPawserMicrocode[] =
    #include "dvb_cnp_xtv_ply.h"
    const u_int32 PlayPawserMicrocodeSize = sizeof(PlayPawserMicrocode);
  #else
    #error "Microcode not supported!"
  #endif

#elif (LEGACY_DVR==YES)

  /*
   * Legacy DVR Microcode
   */


  /*
   * Legacy DVR Microcode
   */

  #if (PARSER_PSI_BUFFER==MULTI_PSI)
    #if (PARSER_FILTERING==FILTER_888)
      const u_int32 PawserMicrocode[] =
      #include "dvb_bcpv.h"
      /*#include "dvb_bcpv2.h"*/
      /*#include "dvb_bcpv3.h"*/
      const u_int32 MicrocodeSize = sizeof(PawserMicrocode);
      const u_int32 RevCPawserMicrocode[] =
      #include "dvb_bcpv.h"
      /*#include "dvb_bcpv2.h"*/
      /*#include "dvb_bcpv3.h"*/
      const u_int32 RevCMicrocodeSize = sizeof(RevCPawserMicrocode);
    #else
      #if (PARSER_DMA_MUX_SEL==YES)
        const u_int32 PawserMicrocode[] =
        #include "dvb_bpxv.h"
        /*#include "dvb_bpxv2.h"*/
        /*#include "dvb_bpxv3.h"*/
        const u_int32 MicrocodeSize = sizeof(PawserMicrocode);
        const u_int32 RevCPawserMicrocode[] =
        #include "dvb_bpxv.h"
        /*#include "dvb_bpxv2.h"*/
        /*#include "dvb_bpxv3.h"*/
        const u_int32 RevCMicrocodeSize = sizeof(RevCPawserMicrocode);
      #else
        const u_int32 PawserMicrocode[] =
        #include "dvb_bpv.h"
        /*#include "dvb_bpv2.h"*/
        /*#include "dvb_bpv3.h"*/
        const u_int32 MicrocodeSize = sizeof(PawserMicrocode);
        const u_int32 RevCPawserMicrocode[] =
        #include "dvb_bpv.h"
        /*#include "dvb_bpv2.h"*/
        /*#include "dvb_bpv3.h"*/
        const u_int32 RevCMicrocodeSize = sizeof(RevCPawserMicrocode);
      #endif
    #endif
  #else
      const u_int32 PawserMicrocode[] =
      #include "dvb_br.h"
      /*#include "dvb_br2.h"*/
      /*#include "dvb_br3.h"*/
      const u_int32 MicrocodeSize = sizeof(PawserMicrocode);
      const u_int32 RevCPawserMicrocode[] =
      #include "dvb_br.h"
      /*#include "dvb_br2.h"*/
      /*#include "dvb_br3.h"*/
      const u_int32 RevCMicrocodeSize = sizeof(RevCPawserMicrocode);
  #endif

#elif ((PARSER_PSI_BUFFER==MULTI_PSI) && (PARSER_FILTERING==FILTER_1212) && (PARSER_NDS_ICAM==YES) && (PARSER_XPRT_BUFFER==YES))

  /*
   * Generic NDS Microcode
   */

  #if (PARSER_MICROCODE==UCODE_COLORADO)
    #if (CHIP_REV==AUTOSENSE)
      const u_int32 PawserMicrocode[] =
      #include "dvb_anpf.h"
      const u_int32 MicrocodeSize = sizeof(PawserMicrocode);
      const u_int32 RevCPawserMicrocode[] =
      #include "dvb_anp.h"
      const u_int32 RevCMicrocodeSize = sizeof(RevCPawserMicrocode);
    #elif (CHIP_REV==REV_C_COLO)
      const u_int32 PawserMicrocode[] =
      #include "dvb_anp.h"
      const u_int32 MicrocodeSize = sizeof(PawserMicrocode);
    #else
      const u_int32 PawserMicrocode[] =
      #include "dvb_anpf.h"
      const u_int32 MicrocodeSize = sizeof(PawserMicrocode);
    #endif
  #elif (PARSER_MICROCODE==UCODE_HONDO)
    const u_int32 PawserMicrocode[] =
    #include "dvb_cnp.h"
    const u_int32 MicrocodeSize = sizeof(PawserMicrocode);
  #elif (PARSER_MICROCODE==UCODE_BRAZOS)
    const u_int32 PawserMicrocode[] =
    #include "dvb_dnp.h"
    const u_int32 MicrocodeSize = sizeof(PawserMicrocode);
  #else
    #error "Microcode not supported!"
  #endif

#elif ((PARSER_PSI_BUFFER==MULTI_PSI) && (PARSER_FILTERING==FILTER_888) && (PARSER_NDS_ICAM==NO) && (PARSER_XPRT_BUFFER==YES))

  #if ((PARSER_KEY64==YES) && (PARSER_MICROCODE==UCODE_COLORADO))

    /*
     * Non-generic Canal+ Non-NDS Microcode
     */

    #if (CHIP_REV==AUTOSENSE)
      const u_int32 PawserMicrocode[] =
      #include "dvb_acpkf.h"
      const u_int32 MicrocodeSize = sizeof(PawserMicrocode);
      const u_int32 RevCPawserMicrocode[] =
      #include "dvb_acpk.h"
      const u_int32 RevCMicrocodeSize = sizeof(RevCPawserMicrocode);
    #elif (CHIP_REV==REV_C_COLO)
      const u_int32 PawserMicrocode[] =
      #include "dvb_acpk.h"
      const u_int32 MicrocodeSize = sizeof(PawserMicrocode);
    #else
      const u_int32 PawserMicrocode[] =
      #include "dvb_acpkf.h"
      const u_int32 MicrocodeSize = sizeof(PawserMicrocode);
    #endif

  #else

    /*
     * Generic Canal+ Non-NDS Microcode
     */

    #if (PARSER_MICROCODE==UCODE_COLORADO)
      #if (CHIP_REV==AUTOSENSE)
        const u_int32 PawserMicrocode[] =
        #if (PARSER_PASSAGE_ENABLE==YES)
        #include "dvb_acpf_pssg.h"
        #else
        #include "dvb_acpf.h"
        #endif
        const u_int32 MicrocodeSize = sizeof(PawserMicrocode);
        const u_int32 RevCPawserMicrocode[] =
        #if (PARSER_PASSAGE_ENABLE==YES)
        #include "dvb_acp_pssg.h"
        #else
        #include "dvb_acp.h"
        #endif
        const u_int32 RevCMicrocodeSize = sizeof(RevCPawserMicrocode);
      #elif (CHIP_REV==REV_C_COLO)
        const u_int32 PawserMicrocode[] =
        #if (PARSER_PASSAGE_ENABLE==YES)
        #include "dvb_acp_pssg.h"
        #else
        #include "dvb_acp.h"
        #endif
        const u_int32 MicrocodeSize = sizeof(PawserMicrocode);
      #else
        const u_int32 PawserMicrocode[] =
        #if (PARSER_PASSAGE_ENABLE==YES)
        #include "dvb_acpf_pssg.h"
        #else
        #include "dvb_acpf.h"
        #endif
        const u_int32 MicrocodeSize = sizeof(PawserMicrocode);
      #endif
    #elif (PARSER_MICROCODE==UCODE_HONDO)
      const u_int32 PawserMicrocode[] =
      #if (PARSER_DES_DESCRAMBLER==NO)
        #if (PARSER_PASSAGE_ENABLE==YES)
        #include "dvb_ccp_pssg.h"
        #else
        #include "dvb_ccp.h"
        #endif
      #else
        #include "dvb_ccps.h"
      #endif
      const u_int32 MicrocodeSize = sizeof(PawserMicrocode);
    #elif (PARSER_MICROCODE==UCODE_BRAZOS)
      const u_int32 PawserMicrocode[] =
      #if (PARSER_PASSAGE_ENABLE==YES)
      #include "dvb_dcp_pssg.h"
      #else
      #include "dvb_dcp.h"
      #endif
      const u_int32 MicrocodeSize = sizeof(PawserMicrocode);
    #else
      #error "Microcode not supported!"
    #endif
  #endif

#elif ((PARSER_PSI_BUFFER==MULTI_PSI) && (PARSER_FILTERING==FILTER_888) && (PARSER_NDS_ICAM==YES) && (PARSER_XPRT_BUFFER==YES))

  /*
   * Generic Canal+ NDS Microcode
   */

  #if (PARSER_MICROCODE==UCODE_COLORADO)
    #if (CHIP_REV==AUTOSENSE)
      const u_int32 PawserMicrocode[] =
      #include "dvb_ancpf.h"
      const u_int32 MicrocodeSize = sizeof(PawserMicrocode);
      const u_int32 RevCPawserMicrocode[] =
      #include "dvb_ancp.h"
      const u_int32 RevCMicrocodeSize = sizeof(RevCPawserMicrocode);
    #elif (CHIP_REV==REV_C_COLO)
      const u_int32 PawserMicrocode[] =
      #include "dvb_ancp.h"
      const u_int32 MicrocodeSize = sizeof(PawserMicrocode);
    #else
      const u_int32 PawserMicrocode[] =
      #include "dvb_ancpf.h"
      const u_int32 MicrocodeSize = sizeof(PawserMicrocode);
    #endif
  #elif (PARSER_MICROCODE==UCODE_HONDO)
    const u_int32 PawserMicrocode[] =
    #if (FILTER_ON_LENGTH==NO)
      #include "dvb_cncp.h"
    #else
      #include "dvb_cncpi.h"
    #endif
    const u_int32 MicrocodeSize = sizeof(PawserMicrocode);
  #elif (PARSER_MICROCODE==UCODE_BRAZOS)
    const u_int32 PawserMicrocode[] =
    #include "dvb_dncp.h"
    const u_int32 MicrocodeSize = sizeof(PawserMicrocode);
  #else
    #error "Microcode not supported!"
  #endif

#elif ((PARSER_PSI_BUFFER==MULTI_PSI) && (PARSER_FILTERING==FILTER_1212) && (PARSER_NDS_ICAM==NO) && (PARSER_XPRT_BUFFER==YES))

  #if (PARSER_DMA_MUX_SEL==YES)

    /*
     * Runtime input mux selection capable Non-NDS Microcode
     */

    #if (PARSER_MICROCODE==UCODE_COLORADO)
      #if (CHIP_REV==AUTOSENSE)
        const u_int32 PawserMicrocode[] =
        #include "dvb_apxf.h"
        const u_int32 MicrocodeSize = sizeof(PawserMicrocode);
        const u_int32 RevCPawserMicrocode[] =
        #include "dvb_apx.h"
        const u_int32 RevCMicrocodeSize = sizeof(RevCPawserMicrocode);
      #elif (CHIP_REV==REV_C_COLO)
        const u_int32 PawserMicrocode[] =
        #include "dvb_apx.h"
        const u_int32 MicrocodeSize = sizeof(PawserMicrocode);
      #else
        const u_int32 PawserMicrocode[] =
        #include "dvb_apxf.h"
        const u_int32 MicrocodeSize = sizeof(PawserMicrocode);
      #endif
    #else
      #error "Microcode not supported!"
    #endif

  #else

    #if (PARSER_KEY64==YES)

      /*
       * Generic Non-NDS Microcode (Key64)
       */

      #if (PARSER_MICROCODE==UCODE_BRAZOS)
        const u_int32 PawserMicrocode[] =
        #include "dvb_dkp.h"
        const u_int32 MicrocodeSize = sizeof(PawserMicrocode);
      #else
        #error "Microcode not supported!"
      #endif

    #else

      /*
       * Generic Non-NDS Microcode
       */

      #if (PARSER_MICROCODE==UCODE_COLORADO)
        #if (CHIP_REV==AUTOSENSE)
          const u_int32 PawserMicrocode[] =
          #include "dvb_apf.h"
          const u_int32 MicrocodeSize = sizeof(PawserMicrocode);
          const u_int32 RevCPawserMicrocode[] =
          #include "dvb_ap.h"
          const u_int32 RevCMicrocodeSize = sizeof(RevCPawserMicrocode);
        #elif (CHIP_REV==REV_C_COLO)
          const u_int32 PawserMicrocode[] =
          #include "dvb_ap.h"
          const u_int32 MicrocodeSize = sizeof(PawserMicrocode);
        #else
          const u_int32 PawserMicrocode[] =
          #include "dvb_apf.h"
          const u_int32 MicrocodeSize = sizeof(PawserMicrocode);
        #endif
      #elif (PARSER_MICROCODE==UCODE_HONDO)
        const u_int32 PawserMicrocode[] =
        #if (PARSER_DES_DESCRAMBLER==NO)
          #include "dvb_cp.h"
        #else
          #include "dvb_cps.h"
        #endif
        const u_int32 MicrocodeSize = sizeof(PawserMicrocode);
      #elif (PARSER_MICROCODE==UCODE_BRAZOS)
        const u_int32 PawserMicrocode[] =
        #include "dvb_dp.h"
        const u_int32 MicrocodeSize = sizeof(PawserMicrocode);
      #else
        #error "Microcode not supported!"
      #endif

    #endif

  #endif

#elif ((PARSER_PSI_BUFFER==SINGLE_PSI) && (PARSER_FILTERING==FILTER_888) && (PARSER_NDS_ICAM==YES) && (PARSER_XPRT_BUFFER==YES))

  /*
   * Legacy Canal+ NDS Microcode
   */

  #if (PARSER_MICROCODE==UCODE_COLORADO)
    #if (CHIP_REV==AUTOSENSE)
      const u_int32 PawserMicrocode[] =
      #include "dvb_anc.h"
      const u_int32 MicrocodeSize = sizeof(PawserMicrocode);
      const u_int32 RevCPawserMicrocode[] =
      #include "dvb_anc.h"
      const u_int32 RevCMicrocodeSize = sizeof(RevCPawserMicrocode);
    #elif (CHIP_REV==REV_C_COLO)
      const u_int32 PawserMicrocode[] =
      #include "dvb_anc.h"
      const u_int32 MicrocodeSize = sizeof(PawserMicrocode);
    #else
      const u_int32 PawserMicrocode[] =
      #include "dvb_anc.h"
      const u_int32 MicrocodeSize = sizeof(PawserMicrocode);
    #endif
  #else
    #error "Microcode not supported!"
  #endif

#elif ((PARSER_PSI_BUFFER==SINGLE_PSI) && (PARSER_FILTERING==FILTER_1212) && (PARSER_NDS_ICAM==NO) && (PARSER_XPRT_BUFFER==YES) && (PARSER_AC3_SPDIF_FIX==NO))

  /* AC3_SPDIF_FIX option is ignored below since the fix is now in software, not
   * microcode */ 
  /*
   * Legacy Non-NDS Microcode
   */

  #if (PARSER_MICROCODE==UCODE_COLORADO)
    #if (CHIP_REV==AUTOSENSE)
      const u_int32 PawserMicrocode[] =
      #include "dvb_a.h"
      const u_int32 MicrocodeSize = sizeof(PawserMicrocode);
      const u_int32 RevCPawserMicrocode[] =
      #include "dvb_a.h"
      const u_int32 RevCMicrocodeSize = sizeof(RevCPawserMicrocode);
    #elif (CHIP_REV==REV_C_COLO)
      const u_int32 PawserMicrocode[] =
      #include "dvb_a.h"
      const u_int32 MicrocodeSize = sizeof(PawserMicrocode);
    #else
      const u_int32 PawserMicrocode[] =
      #include "dvb_a.h"
      const u_int32 MicrocodeSize = sizeof(PawserMicrocode);
    #endif
  #else
    #error "Microcode not supported!"
  #endif

#elif ((PARSER_PSI_BUFFER==SINGLE_PSI) && (PARSER_FILTERING==FILTER_1212) && (PARSER_NDS_ICAM==NO) && (PARSER_XPRT_BUFFER==YES) && (PARSER_AC3_SPDIF_FIX==YES))

  /*
   * Legacy Non-NDS Microcode
   */

  #if (PARSER_MICROCODE==UCODE_COLORADO)
    #if (CHIP_REV==AUTOSENSE)
      const u_int32 PawserMicrocode[] =
      #include "dvb_a.h"
      const u_int32 MicrocodeSize = sizeof(PawserMicrocode);
      const u_int32 RevCPawserMicrocode[] =
      #include "dvb_a.h"
      const u_int32 RevCMicrocodeSize = sizeof(RevCPawserMicrocode);
    #elif (CHIP_REV==REV_C_COLO)
      const u_int32 PawserMicrocode[] =
      #include "dvb_a.h"
      const u_int32 MicrocodeSize = sizeof(PawserMicrocode);
    #else
      const u_int32 PawserMicrocode[] =
      #include "dvb_a.h"
      const u_int32 MicrocodeSize = sizeof(PawserMicrocode);
    #endif
  #else
    #error "Microcode not supported!"
  #endif

#elif ((PARSER_PSI_BUFFER==SINGLE_PSI) && (PARSER_FILTERING==FILTER_1212) && (PARSER_NDS_ICAM==YES) && (PARSER_XPRT_BUFFER==YES) && (PARSER_AC3_SPDIF_FIX==NO))

  /*
   * Legacy NDS Microcode
   */

  #if (PARSER_MICROCODE==UCODE_COLORADO)
    #if (CHIP_REV==AUTOSENSE)
      const u_int32 PawserMicrocode[] =
      #include "dvb_an.h"
      const u_int32 MicrocodeSize = sizeof(PawserMicrocode);
      const u_int32 RevCPawserMicrocode[] =
      #include "dvb_an.h"
      const u_int32 RevCMicrocodeSize = sizeof(RevCPawserMicrocode);
    #elif (CHIP_REV==REV_C_COLO)
      const u_int32 PawserMicrocode[] =
      #include "dvb_an.h"
      const u_int32 MicrocodeSize = sizeof(PawserMicrocode);
    #else
      const u_int32 PawserMicrocode[] =
      #include "dvb_an.h"
      const u_int32 MicrocodeSize = sizeof(PawserMicrocode);
    #endif
  #else
    #error "Microcode not supported!"
  #endif

#else

  /*
   * No matches
   */

  #error "Microcode not supported!"
#endif
#endif

/****************************************************************************
 * Modifications:
 * $Log:
 *  43   mpeg      1.42        8/18/2004 7:11:22 AM   Tim White       CR(s)
 *       10132 : Add -dKEY64 -dSCRAM_STATUS demux microcode build option for
 *       Brazos.
 *  42   mpeg      1.41        7/20/2004 2:36:16 AM   Kunal Shah      CR(s)
 *       9795 : PVR code only activated on PVR enabled IRDs.
 *  41   mpeg      1.40        7/10/2004 4:22:06 AM   Kunal Shah      CR(s)
 *       9715 : Enabled Record extension to Demux to be available from
 *         gen swconfig
 *  40   mpeg      1.39        4/29/2004 10:40:47 PM  Larry Wang      CR(s)
 *       9000 9001 : Set playback demux ucode to dvb_cnp_xtv_ply.h.
 *  39   mpeg      1.38        3/24/2004 4:51:57 AM   Larry Wang      CR(s)
 *       8638 8639 : Add XTV ucode.
 *  38   mpeg      1.37        3/11/2004 4:57:29 AM   Larry Wang      CR(s)
 *       8551 : Add new ucodes to support Sony Passage.
 *  37   mpeg      1.36        3/11/2004 12:51:29 AM  Bob Van Gulick  CR(s)
 *       8546 : Add PES_FRAME_RATE option to microcodes
 *  36   mpeg      1.35        10/11/2003 5:22:16 AM  Bob Van Gulick  Add
 *       dvb_cps.h: Wabash, 1212 filtering and DES descrambling
 *  35   mpeg      1.34        9/3/2003 8:26:10 AM    Joe Kroesche    SCR(s)
 *       7415 :
 *       removed unneeded header file to eliminate warnings when building for
 *       PSOS
 *  34   mpeg      1.33        8/5/2003 12:01:50 AM   Larry Wang      SCR(s)
 *       7124 7125 :
 *       Add microcode support for DES ECB.
 *  33   mpeg      1.32        6/25/2003 7:37:12 AM   Tim White       SCR(s)
 *       6831 :
 *       Add flash, hsdp, demux, OSD, and demod support to codeldrext.
 *       
 *  32   mpeg      1.31        6/10/2003 6:55:18 AM   Bob Van Gulick  SCR(s)
 *       6755 :
 *       Updates to various microcode build options
 *       
 *  31   mpeg      1.30        6/3/2003 11:34:24 PM   Larry Wang      SCR(s)
 *       6667 :
 *       Update the building instruction for wabash and brazos microcode to
 *       include -dAC3.
 *  30   mpeg      1.29        5/14/2003 5:31:08 AM   Bob Van Gulick  SCR(s)
 *       6319 6320 :
 *       Remove AUDIO_STREAM_ID from DVR builds due to space constraints
 *       
 *  29   mpeg      1.28        5/2/2003 11:59:08 PM   Bob Van Gulick  SCR(s)
 *       6151 :
 *       remove ucode file dvb_acpkf.h and build dvb_acpk.h instead.  f was
 *       removed since PTS_OFFSET is now a default build option.
 *       
 *  28   mpeg      1.27        4/17/2003 6:10:30 AM   Larry Wang      SCR(s)
 *       6045 :
 *       Include dvb_cncpi.h if SWCONFIG=PACEWAB.
 *  27   mpeg      1.26        4/17/2003 4:00:36 AM   Bob Van Gulick  SCR(s)
 *       6024 :
 *       Add -dPES_HDR_INT as default option
 *       
 *  26   mpeg      1.25        3/7/2003 6:41:54 AM    Bob Van Gulick  SCR(s)
 *       5697 :
 *       Add -dCRC_NOTIFY so we are compatible with the DEMUX
 *       for PVR record microcode
 *       
 *  25   mpeg      1.24        3/7/2003 4:52:42 AM    Bob Van Gulick  SCR(s)
 *       5697 :
 *       Add -dMULTI_PSI to PVR record microcode
 *       
 *  24   mpeg      1.23        2/28/2003 7:43:04 AM   Bob Van Gulick  SCR(s)
 *       4672 :
 *       Add support for -dAUDIO_STREAM_ID.
 *       
 *  23   mpeg      1.22        2/21/2003 5:19:18 AM   Tim White       SCR(s)
 *       5556 5557 :
 *       Remove non-DVB code from file.
 *       
 *  22   mpeg      1.21        2/20/2003 7:28:40 AM   Tim White       SCR(s)
 *       5557 5556 :
 *       Added Brazos pawser microcodes.
 *       
 *  21   mpeg      1.20        2/15/2003 12:35:38 AM  Larry Wang      SCR(s)
 *       5504 :
 *       Load DSS microcode #if PARSER_TYPE==DSS_TYPE.
 *  20   mpeg      1.19        2/5/2003 7:04:14 AM    Bob Van Gulick  SCR(s)
 *       5407 :
 *       Remove SCRAM_STATUS as a default build option.  Only use with
 *       dvb_acpkf.h.
 *       
 *  19   mpeg      1.18        12/11/2002 6:37:46 AM  Bob Van Gulick  SCR(s)
 *       5121 :
 *       Update microcode files to support the default option of SCRAM_STATUS
 *       
 *  18   mpeg      1.17        11/16/2002 3:00:14 AM  Tim White       SCR(s)
 *       4935 :
 *       Removed the microcode AC3_SPDIF_FIX support for all microcodes except
 *       dvb_a_ac3.h.
 *       
 *  17   mpeg      1.16        11/11/2002 11:27:22 PM Tim White       SCR(s)
 *       4930 :
 *       Add -dDMA_MUX pawser microcode support to DVR microcode(s).
 *       
 *  16   mpeg      1.15        10/3/2002 11:35:56 PM  Tim White       SCR(s)
 *       4737 :
 *       Added dvb_a_ac3.h.
 *       
 *  15   mpeg      1.14        9/20/2002 3:35:32 AM   Tim White       SCR(s)
 *       4626 :
 *       Drop version 3.2.0 microcode.  Add -dBOOT_LOADER build option
 *       (dvb_apl.h file).
 *       
 *  14   mpeg      1.13        9/6/2002 1:41:20 AM    Tim White       SCR(s)
 *       4530 :
 *       Remove the AC3_SPDIF_FIX build option from the Wabash microcodes.
 *       
 *  13   mpeg      1.12        8/30/2002 7:03:12 AM   Tim White       SCR(s)
 *       4485 :
 *       Added CRC_CHECK option built as default for all microcodes using
 *       MULTI_PSI.
 *       
 *  12   mpeg      1.11        8/17/2002 7:05:50 AM   Tim White       SCR(s)
 *       4420 :
 *       Add support for new DVR microcode which supports DVR, XPRT, and
 *       MULTI_PSI together.
 *       
 *  11   mpeg      1.10        5/31/2002 2:37:34 AM   Tim White       SCR(s)
 *       3899 :
 *       Added new microcode flavor, dvb_apx_ac3.h & dvb_apx.h for using the
 *       -dDMA_MUX build option.
 *       
 *  10   mpeg      1.9         5/22/2002 4:51:20 AM   Tim White       SCR(s)
 *       3642 :
 *       Use PARSER_PTS_OFFSET & PARSER_KEY64 as the differentiator for the
 *       customer specific
 *       microcode files dvb_acpkf_ac3.h & dvb_acpkf.h.
 *       
 *  9    mpeg      1.8         5/22/2002 4:31:38 AM   Tim White       SCR(s)
 *       3642 :
 *       Added comment block about how-to-build, etc...
 *       
 *  8    mpeg      1.7         5/22/2002 3:29:12 AM   Tim White       SCR(s)
 *       3642 :
 *       Renamed dvb_acpikeb_ac3.h to dvb_acpkf_ac3.h and dvb_acpikb.h to
 *       dvb_acpkf.h.
 *       
 *  7    mpeg      1.6         5/1/2002 6:19:58 AM    Tim White       SCR(s)
 *       3664 :
 *       Added -dBAD_PES_INT parser microcode build option support.
 *       
 *  6    mpeg      1.5         4/27/2002 4:25:42 AM   Tim White       SCR(s)
 *       3562 :
 *       Add support for Colorado Rev_F.  Added 6 new microcode variations to
 *       allow for
 *       run-time chip rev detection support which are nothing more than Rev_C
 *       versions
 *       with the -dAC3_SPDIF_FIX option removed.
 *       
 *  5    mpeg      1.4         3/26/2002 4:56:56 AM   Tim White       SCR(s)
 *       3433 :
 *       Changed parser microcode files from .c files to .h files.
 *       
 *  4    mpeg      1.3         3/26/2002 3:17:38 AM   Tim White       SCR(s)
 *       3433 :
 *       Combine the complete parser microcode matrix into one version of the
 *       file.  Also, all microcode files are now .h files instead of .c files.
 *       
 *  3    mpeg      1.2         1/15/2002 1:20:38 AM   Tim White       SCR(s)
 *       3042 :
 *       This is the default pawser microcode branch (dvb_anp_ac3)
 *       
 *  2    mpeg      1.1         1/15/2002 1:16:04 AM   Tim White       SCR(s)
 *       3035 :
 *       Added history log.
 *       
 *  1    mpeg      1.0         1/15/2002 1:11:12 AM   Tim White       
 * $
 ****************************************************************************/


