/****************************************************************************/
/*                   CONEXANT PROPRIETARY AND CONFIDENTIAL                  */
/*                       SOFTWARE FILE/MODULE HEADER                        */
/*          Conexant Systems Inc. (c) 1998, 1999, 2000, 2001, 2002          */
/*                              Austin, TX                                  */
/*                         All Rights Reserved                              */
/****************************************************************************/
/*
 * Filename:       demuxhw.c
 *
 *
 * Description:    Contains all hardware interaction function in the demux driver
 *
 * Author:         Bob Van Gulick 
 *
 ****************************************************************************/
/* $Id: demuxhw.c,v 1.34, 2004-05-18 19:43:49Z, Tim White$
 ****************************************************************************/

#include "stbcfg.h"
#include "basetype.h"
#include "globals.h"
#include "kal.h"
#include "startup.h"
#include "retcodes.h"
#include "demuxapi.h"
#include "demuxint.h"
#include "clkrec.h"

#if ((PVR==YES)||(XTV_SUPPORT==YES))
  /*
   * For PVR, there are two microcodes in the image:  Play & Record versions
   */
  extern u_int32 RecPawserMicrocode[], PlayPawserMicrocode[];
  extern u_int32 RecPawserMicrocodeSize, PlayPawserMicrocodeSize;
#else
  #if (PARSER_MICROCODE==UCODE_COLORADO) && (CHIP_REV==AUTOSENSE)
    /*
     * For Colorado runtime chip detect, there are two microcodes in
     * the image:  Rev_C/D & Rev_F
     */
    extern u_int32 PawserMicrocode[], RevCPawserMicrocode[];
    extern u_int32 MicrocodeSize, RevCMicrocodeSize;
  #else
    /*
     * There is one microcode in the image
     */
    extern u_int32 PawserMicrocode[];
    extern u_int32 MicrocodeSize;
  #endif
#endif // PVR

#if (CANAL_AUDIO_HANDLING == YES)
#define AUDIO_PTS_OFFSET 9000
#define VIDEO_PTS_OFFSET 9000
#endif /* CANAL_AUDIO_HANDLING */

extern DemuxInfo gDemuxInfo[];

#if defined DRIVER_INCL_NDSDEMUX || defined DRIVER_INCL_NDSICAM
extern void NotifyCAPidChange(u_int32 dmxid, u_int32 chid, u_int16 pid);
extern void dmx_nds_init(bool ReInit);
#else 
extern void descrambler_notify_pid_change(u_int32 dmxid, u_int32 chid, u_int16 pid);
#endif

/*-------------------------------------------------------------------------------------------**
** genDemuxInitHardware                                                                      **
** Params:                                                                                   **
**         ReInit: 0 for first time, 0 for reinit.                                           **
** Desc.:                                                                                    **
**         Initializes all the register pointers, the HW PID Table, and the Mask/Match area. **
**         Also may create the queue and main task before enabling the the Demux hardware.   **
** Return Value:                                                                             **
**         TRUE OK                                                                           **
**         FLASE Init Failed                                                                 **
**-------------------------------------------------------------------------------------------*/

bool genDemuxInitHardware(u_int32 dmxid, bool ReInit) {
    u_int32 i, j, *pMem;
    bool ks;
    u_int32  chip_id;
    u_int8   chip_rev;
    DemuxInfo *dmx = &gDemuxInfo[dmxid];
    u_int32 TotalBlocksInTSBuffer;

    trace_new(DPS_FUNC_TRACE,"DEMUX: InitHardware Called\n");

    read_chip_id_and_revision(&chip_id, &chip_rev);

    /*Initialize the required register pointers. */
    ks = critical_section_begin();
    *DPS_INT_ENABLE_REG_EX(dmxid) = 0;
    *DPS_HOST_CTL_REG_EX(dmxid) &= ~(DPS_PARSER_ENABLE | DPS_PID_ENABLE);

    /* Disable parser while we load the Pawser microcode */
    *DPS_PARSER_CTL_REG_EX(dmxid) &= ~DPS_RUN;

    /* Make sure non active parsers have MPEG_MASTER turned off! */
    for (i=0; i<MAX_DEMUX_UNITS; i++)
    {
        if (i == 1) continue;  /* There is no demux 1 on Wabash */
        if ((i != dmxid) && (gDemuxInfo[i].DemuxID == DEMUX_FREE))
        {
            *DPS_PARSER_CTL_REG_EX(i) &= ~DPS_MPEG_PTR_MASTER_ENABLE;
        }
    }

    /*
     * Load Pawser Microcode(s) here
     */

#if ((PVR==YES)||(XTV_SUPPORT==YES))
    if(dmx->CapabilitiesRequested & DMX_CAP_RECORD)
    {
        /*
         * Load PVR Record microcode
         */
        pMem = (u_int32 *) DPS_BASE(dmxid);
        for (i = 0; i < RecPawserMicrocodeSize/4; ++i)
        {
            *pMem++ = RecPawserMicrocode[i];
        }
    }
    else /* if(dmx->CapabilitiesRequested & DMX_CAP_PLAYBACK) */
    {
        /*
         * Load PVR Play microcode
         */
        pMem = (u_int32 *) DPS_BASE(dmxid);
        for (i = 0; i < PlayPawserMicrocodeSize/4; ++i)
        {
            *pMem++ = PlayPawserMicrocode[i];
        }
    }
#else
  #if (PARSER_MICROCODE==UCODE_COLORADO) && (CHIP_REV==AUTOSENSE)
    if(ISCOLORADOREVC)
    {
        /*
         * For Colorado Rev_C/D chips, use AC3 SPDIF Fix ucode
         */
        pMem = (u_int32 *) DPS_BASE(dmxid);
        for (i = 0; i < RevCMicrocodeSize/4; ++i)
        {
            *pMem++ = RevCPawserMicrocode[i];
        }
    }
    else
  #endif
    {
        /*
         * For all cases other than CHIP_REV==AUTOSENSE on a Colorado chip...
         */
        pMem = (u_int32 *) DPS_BASE(dmxid);
        for (i = 0; i < MicrocodeSize/4; ++i)
        {
            *pMem++ = PawserMicrocode[i];
        }
    }
#endif

#if PARSER_TYPE==DVB_PARSER
    /* Ensure that correct version of microcode was loaded into play pawser */
    pMem = (u_int32 *) DPS_BASE(dmxid);
    if((pMem[DPS_UCODE_WORD_IDX] & DPS_UCODE_VERSION_MASK) != DPS_UCODE_VERSION) {
      isr_trace_new(TRACE_LEVEL_ALWAYS | TRACE_MASK_MODULE, 
		    "GenDemuxInitHardware: Incompatible parser microcode version 0x%x!\n",
		    pMem[DPS_UCODE_WORD_IDX] & DPS_UCODE_VERSION_MASK, 0);
      fatal_exit(ERROR_FATAL | GENDMX_ERROR);
    }
#endif

    /******************************************************************************
     * The following line of code will make the demux that is being initialized
     * the new 'Master' demux.  Once we have an application that uses demux 2 or 3, 
     * we will need to modify the API to support setting of the master demux.  The
     * master will most likely be demux 0 (descrambled) once all demux's being used
     * are initialized, but we don't know how this information will come to us at this
     * time.
     ******************************************************************************/
    if(dmx->CapabilitiesRequested & DMX_CAP_RECORD)
    {
        *DPS_PARSER_CTL_REG_EX(dmxid) |= (DPS_RUN | DPS_TP_MODE_DVB);
    }
    else
    {
        *DPS_PARSER_CTL_REG_EX(dmxid) |= (DPS_RUN | DPS_MPEG_PTR_MASTER_ENABLE | DPS_TP_MODE_DVB);
    }

    /* default input byteswap is big endian */
    if(dmx->CapabilitiesRequested & DMX_CAP_LITTLE_ENDIAN)
    {
       *DPS_PARSER_CTL_REG_EX(dmxid) |= (DPS_RUN | DPS_INPUT_BYTESWAP_LITTLE_ENDIAN);
    }

    *DPS_HOST_CTL_REG_EX(dmxid) |= (DPS_PARSER_ENABLE|DPS_BSWAP);    

    while (!(*DPS_PARSER_STATUS_REG_EX(dmxid) & DPS_PARSER_READY))
        ;

/* NEEDSWORK -- Only 1 transport buffer for all demuxes !!! */
/* NEEDSWORK -- Only 1 transport buffer for all demuxes !!! */
/* NEEDSWORK -- Only 1 transport buffer for all demuxes !!! */

    *DPS_TRANSPORT_START_ADDR_EX(dmxid) = HWBUF_TRNSPRT_ADDR + DPS_PAW_SYS_ADDRESS;
    *DPS_TRANSPORT_END_ADDR_EX(dmxid) = 
      HWBUF_TRNSPRT_ADDR  + DPS_PAW_SYS_ADDRESS + ((HWBUF_TRNSPRT_SIZE/TS_COPY_BLOCK_SIZE) * TS_COPY_BLOCK_SIZE);
    *DPS_TRANSPORT_READ_PTR_EX(dmxid) = HWBUF_TRNSPRT_ADDR + DPS_PAW_SYS_ADDRESS;
    *DPS_TRANSPORT_WRITE_PTR_EX(dmxid) = HWBUF_TRNSPRT_ADDR + DPS_PAW_SYS_ADDRESS;

#if PARSER_TYPE==DTV_PARSER
    *DPS_CWP_START_ADDR_EX(dmxid) = HWBUF_CWP_ADDR + DPS_PAW_SYS_ADDRESS;
    *DPS_CWP_END_ADDR_EX(dmxid) = 
      HWBUF_CWP_ADDR  + DPS_PAW_SYS_ADDRESS + ((HWBUF_CWP_SIZE/CWP_COPY_BLOCK_SIZE) * CWP_COPY_BLOCK_SIZE);
    *DPS_CWP_READ_PTR_EX(dmxid) = HWBUF_CWP_ADDR + DPS_PAW_SYS_ADDRESS;
    *DPS_CWP_WRITE_PTR_EX(dmxid) = HWBUF_CWP_ADDR + DPS_PAW_SYS_ADDRESS;

    *DPS_CAP_START_ADDR_EX(dmxid) = HWBUF_CAP_ADDR + DPS_PAW_SYS_ADDRESS;
    *DPS_CAP_END_ADDR_EX(dmxid) = 
      HWBUF_CAP_ADDR  + DPS_PAW_SYS_ADDRESS + ((HWBUF_CAP_SIZE/CAP_COPY_BLOCK_SIZE) * CAP_COPY_BLOCK_SIZE);
    *DPS_CAP_READ_PTR_EX(dmxid) = HWBUF_CAP_ADDR + DPS_PAW_SYS_ADDRESS;
    *DPS_CAP_WRITE_PTR_EX(dmxid) = HWBUF_CAP_ADDR + DPS_PAW_SYS_ADDRESS;
#endif

    /*
     * Legacy DVR
     */

#if (LEGACY_DVR==YES)
    dvr_init(dmxid);
#endif

/* NEEDSWORK -- Only 1 user buffer for all demuxes !!! */
/* NEEDSWORK -- Only 1 user buffer for all demuxes !!! */
/* NEEDSWORK -- Only 1 user buffer for all demuxes !!! */

    *DPS_USER_START_ADDR_EX(dmxid) = HWBUF_USRDAT_ADDR;
    critical_section_end(ks);

    /*
    Initialize the PID Storage area to NULL PID.
    Initialize the match, mask tables to
    */

    if (!ReInit) {
        for (i=0; i<TOTAL_CHANNELS; ++i) {
            *DPS_PID_BASE_EX(dmxid,i) = GEN_NULL_PID;
            *DPS_FILTER_CONTROL_BASE_EX(dmxid,i) = 0x00000000; /* disable all filter bits */

            for (j=0; j<(GENDMX_MAX_HW_FILTER_SIZE/4); ++j) {
                *DPS_FILTER_MASK_BASE_EX(dmxid,i,j) = 0;    /* mask set to don't care */
                *DPS_PATTERN_BASE_EX(dmxid,i,j) = 0; /* Clear the patterns */
#if PARSER_FILTERING == FILTER_888
                *DPS_FILTER_MODE_BASE_EX(dmxid,i,j) = 0;
#endif
            }

        }
    } else {  /* ReInit case */
        for (i = 0; i < TOTAL_CHANNELS; ++i) {
            for (j = 0; j < GENDMX_MAX_HW_FILTER_SIZE; ++j) {
                *DPS_FILTER_MASK_BASE_EX(dmxid,i,j) = BSWAP(dmx->FilterTable[i].Mask[j]);
                *DPS_PATTERN_BASE_EX(dmxid,i,j) = BSWAP(dmx->FilterTable[i].Match[j]);
#if PARSER_FILTERING == FILTER_888
                if (dmx->FilterTable[i].NotMaskZero == TRUE) {
                    *DPS_FILTER_MODE_BASE_EX(dmxid,i,j) = 0;
                    *DPS_NEGATIVE_MODE_REG_EX(dmxid) &= ~(1 << i); /* clear NegativeMode bit of this filter */
                } else {
                    *DPS_FILTER_MODE_BASE_EX(dmxid,i,j) = BSWAP(dmx->FilterTable[i].NotMask[j]);
                    *DPS_FILTER_MASK_BASE_EX(dmxid,i,j) |= *DPS_FILTER_MODE_BASE_EX(dmxid,i,j);
                    *DPS_NEGATIVE_MODE_REG_EX(dmxid) |= (1 << i); /* clear NegativeMode bit of this filter */
                }
#endif
            }
            *DPS_FILTER_CONTROL_BASE_EX(dmxid,i) = dmx->ChannelInfoTable[i].FilterEnable; 
            *DPS_PID_BASE_EX(dmxid,i) = dmx->ChannelInfoTable[i].PID; 
        }
    }
    *DPS_PCR_PID_EX(dmxid) = dmx->PCR_PID_Value; 

    ks = critical_section_begin();
    *DPS_INT_ENABLE_REG_EX(dmxid) = DEMUX_ISR_BITS;
    #if CANAL_AUDIO_HANDLING == YES
    *DPS_EVENT_ENABLE_REG_EX(dmxid) = DEMUX_EVENT_BITS | DPS_BAD_PES_HEADER |
                                      DPS_VIDEO_PKT_RECEIVED | DPS_AUDIO_PKT_RECEIVED;
    #else /* CANAL_AUDIO_HANDLING != YES */
    *DPS_EVENT_ENABLE_REG_EX(dmxid) = DEMUX_EVENT_BITS;
    #endif /* CANAL_ADIO_HANDLING */
    *DPS_ISR_REG_EX(dmxid) = *DPS_ISR_REG_EX(dmxid); /* Clear out any interrupts */
    *DPS_HOST_CTL_REG_EX(dmxid) =
    (DPS_PARSER_ENABLE | DPS_PID_ENABLE| DPS_BSWAP);
#if (PARSER_MICROCODE == UCODE_COLORADO || PARSER_MICROCODE == UCODE_HONDO || PARSER_MICROCODE == UCODE_WABASH)
    *DPS_VERSION_MODES_REG_EX(dmxid) = 0;
#else
    *DPS_VERSION_MODES_REG_EX_0_BANK_01(dmxid) = 0;
    *DPS_VERSION_MODES_REG_EX_1_BANK_01(dmxid) = 0;
    *DPS_VERSION_MODES_REG_EX_0_BANK_12(dmxid) = 0;
    *DPS_VERSION_MODES_REG_EX_0_BANK_02(dmxid) = 0;
#endif

    /* Set the Transport Blocks to the maximum of 63 */
    TotalBlocksInTSBuffer = ((HWBUF_TRNSPRT_SIZE+TS_COPY_BLOCK_SIZE-1)/TS_COPY_BLOCK_SIZE)>>2;
    if (TotalBlocksInTSBuffer>63) {
        TotalBlocksInTSBuffer = 63;
    }
#if (LEGACY_DVR==YES)
    *DPS_DVR_TRANSPORT_BLK_SIZE_EX(dmxid) = TotalBlocksInTSBuffer;
#else
    *DPS_HOST_CTL_REG_EX(dmxid) &= ~DPS_TRANSPORT_BLOCKS_MASK;
#ifndef DRIVER_INCL_NDSTESTS
    *DPS_HOST_CTL_REG_EX(dmxid) |= (TotalBlocksInTSBuffer << DPS_TRANSPORT_BLOCKS_SHIFT);
#endif
#endif
    *( (LPREG)glpCtrl0) |= MPEG_MPEG_ACTIVE;

#ifndef DRIVER_INCL_HSDP

    /*
     * For new code, the HSDP driver is used which eliminates the need to perform
     * the following operation.  It is possible to have code which uses the new
     * demux (this) driver but not the HSDP driver, hence the reason this code
     * need to stay under a conditional inclusion.
     */

#if CUSTOMER != VENDOR_B
    *((LPREG) HSDP_TSA_PORT_CNTL_REG) = HSDP_SER_NIM_CNTL_SER_TO_PAR_CONV_ENABLE; /* NIM Select */
#endif

#if PLL_PIN_GPIO_MUX0_REG_DEFAULT == NOT_DETERMINED
        *((LPREG) PLL_PIN_GPIO_MUX0_REG) |= (
                                            PLL_PIN_GPIO_MUX0_PIO_29 |
                                            PLL_PIN_GPIO_MUX0_PIO_30 |
                                            PLL_PIN_GPIO_MUX0_PIO_31);

        *((LPREG) PLL_PIN_GPIO_MUX1_REG) |= (
                                            PLL_PIN_GPIO_MUX1_PIO_32 |
                                            PLL_PIN_GPIO_MUX1_PIO_33 |
                                            PLL_PIN_GPIO_MUX1_PIO_34 |
                                            PLL_PIN_GPIO_MUX1_PIO_35 |
                                            PLL_PIN_GPIO_MUX1_PIO_36 |
                                            PLL_PIN_GPIO_MUX1_PIO_37 |
                                            PLL_PIN_GPIO_MUX1_PIO_38 |
                                            PLL_PIN_GPIO_MUX1_PIO_43 |
                                            PLL_PIN_GPIO_MUX1_PIO_46 |
                                            PLL_PIN_GPIO_MUX1_PIO_47 |
                                            PLL_PIN_GPIO_MUX1_PIO_48 |
                                            PLL_PIN_GPIO_MUX1_PIO_49 |
                                            PLL_PIN_GPIO_MUX1_PIO_50 |
                                            PLL_PIN_GPIO_MUX1_PIO_51 |
                                            PLL_PIN_GPIO_MUX1_PIO_52 |
                                            PLL_PIN_GPIO_MUX1_PIO_53 |
                                            PLL_PIN_GPIO_MUX1_PIO_54 |
                                            PLL_PIN_GPIO_MUX1_PIO_55 |
                                            PLL_PIN_GPIO_MUX1_PIO_56 |
                                            PLL_PIN_GPIO_MUX1_PIO_57 |
                                            PLL_PIN_GPIO_MUX1_PIO_62);
#if (EMULATION_LEVEL == FINAL_HARDWARE) && (CUSTOMER != VENDOR_D) && (CUSTOMER != VENDOR_B)
        *((LPREG) PLL_PIN_GPIO_MUX1_REG) |= PLL_PIN_GPIO_MUX1_PIO_42;
#endif
#endif /* PLL_PIN_GPIO_MUX0_REG_DEFAULT == NOT_DETERMINED */
#if GPIO_CONFIG == GPIOM_COLORADO
    *((LPREG) HSDP_TSB_PORT_CNTL_REG) = HSDP_BIDIR_PORT_CNTL_MODE_CI_READ;
#elif GPIO_CONFIG == GPIOM_HONDO
    *((LPREG) HSDP_TSC_PORT_CNTL_REG) = HSDP_BIDIR_PORT_CNTL_MODE_CI_READ;
#endif

#if CUSTOMER == VENDOR_B
  #if GPIO_CONFIG == GPIOM_COLORADO
    *((LPREG) HSDP_TSC_PORT_CNTL_REG) = HSDP_BIDIR_PORT_CNTL_MODE_CI_READ;
  #elif GPIO_CONFIG == GPIOM_HONDO
    *((LPREG) HSDP_TSD_PORT_CNTL_REG) = HSDP_BIDIR_PORT_CNTL_MODE_CI_READ;
  #endif
    *((UINT8*) HSDP_SP_INPUT_CNTL_REG) = 0x01;
#else
  #if GPIO_CONFIG == GPIOM_COLORADO
    *((LPREG) HSDP_TSC_PORT_CNTL_REG) = HSDP_BIDIR_PORT_CNTL_MODE_CI_WRITE;
  #elif GPIO_CONFIG == GPIOM_HONDO
    *((LPREG) HSDP_TSD_PORT_CNTL_REG) = HSDP_BIDIR_PORT_CNTL_MODE_CI_WRITE;
  #endif
#endif
    *((LPREG) HSDP_TS_PKT_CNTL_REG)   = 0x000047BC;

#endif /* !DRIVER_INCL_HSDP */

    #if (CANAL_AUDIO_HANDLING == YES)
    /* For Canal+ we need to set 1/10s PTS offset to compensate for PTS */
    /* errors in their version 2.0 test stream (some pictures have PTS  */
    /* values which mature before the data has arrived!)                */
    cnxt_dmx_set_pts_offset(dmxid, AUDIO_PTS_OFFSET, VIDEO_PTS_OFFSET);
    #endif
    *DPS_NOTIFY_CRC_EX(dmxid) |= 1;     /* enable "crc" mode */

    critical_section_end(ks);

#if defined DRIVER_INCL_NDSDEMUX || defined DRIVER_INCL_NDSICAM
    if (ReInit) {
        dmx_nds_init(ReInit);                       /* re-initialize NDS HW */
    }
#endif
    trace_new(DPS_FUNC_TRACE,"DEMUX: InitHardware Done\n");
    return TRUE;
}

/*--------------------------------------------------------------------------**
** gen_dmx_hw_free_pid                                                      **
** Desc.                                                                    **
**      If no other channels are using this PID (ie. slot) then free it by  **
**      setting it to NULL PID. Otherwise mark the channel's slot as no PID **
**      assigned                                                            **
** Params                                                                   **
**     chid    Channel ID of channel to remove it's PID                     **
**                                                                          **
** Returns                                                                  **
**         Nothing                                                          **
**--------------------------------------------------------------------------*/
void gen_dmx_hw_free_pid(u_int32 dmxid, u_int32 chid) {
    DemuxInfo *dmx = &gDemuxInfo[dmxid];
    u_int32 slot = dmx->ChannelInfoTable[chid].Slot;    /* Get slot for this PID */
    bool ks;


#ifdef DEBUG
    if (slot == VIDEO_CHANNEL || slot == AUDIO_CHANNEL) {
        isr_trace_new(TRACE_DPS|TRACE_LEVEL_4, "hw_free_pid(slot=%d)\n", slot, 0);
    }
#endif

    /*
     * Remove the PID from the table
     */

    *DPS_PID_BASE_EX(dmxid,slot) = GEN_NULL_PID;               /* Remove PID from HW table */

    /*
     * If Audio or Video, inform the appropriate decoder to stop
     */

    if (dmx->ChannelInfoTable[slot].stype == VIDEO_PES_TYPE)
    {
        ks = critical_section_begin();
        if (*( (LPREG)glpCtrl0) & MPG_ENABLE_SYNC) {
            *( (LPREG)glpCtrl0) &= ~MPG_ENABLE_SYNC;
            *( (LPREG)glpCtrl0) |= MPG_ENABLE_SYNC;
        }
        critical_section_end(ks);
    }
    if (dmx->ChannelInfoTable[slot].stype == AUDIO_PES_TYPE)
    {
        ks = critical_section_begin();
        if (*( (LPREG)glpCtrl1) & MPG_AUD_SYNC_ENABLED) {
            *( (LPREG)glpCtrl1) &= ~MPG_AUD_SYNC_ENABLED;
            *( (LPREG)glpCtrl1) |= MPG_AUD_SYNC_ENABLED;
        }
        critical_section_end(ks);
    }

    /*
     * Do not reset the slot buffer ptr
     */
}

/*
 * This is required for setting up the NIM Extender when building with
 * CUSTOMER == VENDOR_B on a Conexant box (e.g. Klondike)
 */
#if CUSTOMER == VENDOR_B
static void SetNIMExtenderKlondike(void)
{
    bool ks;
    LPREG lpSpInput = (LPREG)DPS_SP_INPUT_CNTL_REG;
    u_int32 uValue;
    u_int8 bMuxVal, bMux2Val;

    bMuxVal = GPIO1_TSI_DVB_IN | GPIO1_HSDP_IN_TSIN | GPIO1_DVB_IN;
    bMux2Val = GPIO2_HSDP_CLK_MUX_ENABLED;

    /* Set the MUXes correctly for the required transport stream */
    IICInit();
    write_gpio_extender(GPIO1_TSI_MUX_MASK |
                        GPIO1_HSDP_IN_MUX_MASK |
                        GPIO1_DVBP_DIR_MASK,
                        bMuxVal);
    write_second_gpio_extender(GPIO2_HSDP_CLK_MASK,
                               bMux2Val);

    /* On Colorado, we also have to worry about the internal transport */
    /* stream muxing.                                                  */
    ks = critical_section_begin();

    uValue = *lpSpInput;
    uValue &= ~DPS_SP0_INPUT_CNTL_MASK;

    uValue |= DPS_SP0_TSB_INPUT_CNTL;

    *lpSpInput = uValue;

    critical_section_end(ks);
}
#endif

/*----------------------------------------------------------------------------------------**
**    Name: gen_dmx_shutdown                                                              **
**    Purpose: Puts the HW & SW in an IDLE state.                                         **
**             to perform any hardware initializations.                                   **
**    Parameters: dmx DemuxInfo struct                                                    **
**    Return value: None                                                                  **
**----------------------------------------------------------------------------------------*/
void gen_dmx_shutdown(u_int32 dmxid){
    bool ks;
    int i;
    DemuxInfo *dmx = &gDemuxInfo[dmxid];

    /* shutdown the Pawser */
    ks = critical_section_begin();
    *DPS_INT_ENABLE_REG_EX(dmxid) = 0; /* Disable interrupts */
    *DPS_HOST_CTL_REG_EX(dmxid) &= ~(DPS_PARSER_ENABLE | DPS_PID_ENABLE);
    *DPS_PARSER_CTL_REG_EX(dmxid) &= ~DPS_RUN;
    critical_section_end(ks);

    /* Initialize the software data structures */
    genDemuxInitSW(dmxid);

    /* Reinitialize the header & section callback tables */
    for (i = 0; i < TOTAL_CHANNELS; ++i) {
        dmx->ChannelInfoTable[i].HdrErrNotify   = NULL;
        dmx->ChannelInfoTable[i].DataNotify     = NULL;
    }
    dmx->DemuxInitialized = FALSE;
}

/*--------------------------------------------------------------------------------------------**
** gen_dmx_hw_set_pid                                                                         **
** Description                                                                                **
**     This function will find a slot in the hardware PID table. If the pid is already in the **
**     table then that slot is used. Otherwise the slot associated with the chid is used.     **
**     The filter enable register is also updated.                                            **
**                                                                                            **
**                                                                                            **
** Params                                                                                     **
**     chid    Channel ID to set PID for                                                      **
**     pid     PID to set                                                                     **
** Returns:                                                                                   **
**     Nothing                                                                                **
**--------------------------------------------------------------------------------------------*/
void gen_dmx_hw_set_pid(u_int32 dmxid, u_int32 chid, u_int16 pid) {
    DemuxInfo *dmx = &gDemuxInfo[dmxid];
    u_int32 slot = dmx->ChannelInfoTable[chid].Slot;    /* Get slot for this PID */

#ifdef DEBUG
    if (slot == VIDEO_CHANNEL || slot == AUDIO_CHANNEL) {
        trace_new(TRACE_DPS|TRACE_LEVEL_4, "hw_set_pid(slot=%d pid=%04x)\n", slot, pid);
    }
#endif

    /*
     * If the PID is already set, then dont set it again, just return
     */
    if ((*DPS_PID_BASE_EX(dmxid,chid) & 0x1fff) == pid) {
        return;
    }

    /*
     * Tell CA code about new PID
     */

#if defined DRIVER_INCL_NDSDEMUX || defined DRIVER_INCL_NDSICAM
    NotifyCAPidChange(dmxid, slot, pid);
#else
    descrambler_notify_pid_change(dmxid, slot, pid);
#endif

    /*
     * Set the PID
     */
    *DPS_PID_BASE_EX(dmxid,slot) = pid;

#if CUSTOMER == VENDOR_C
    if ((slot == VIDEO_CHANNEL) && (pid == 0x1fff)) {
        gen_dmx_set_pcr_pid(dmxid, pid);
    }
#endif

    /*
     * Notify parser of PID table change
     */
    *DPS_FILTER_CONTROL_BASE_EX(dmxid,slot) = 
    dmx->ChannelInfoTable[chid].FilterEnable;
    *DPS_INFO_CHANGE_REG_EX(dmxid) |= (1<<slot);

    return;
}

/*---------------------------------------------------------------------------------------**
** gen_dmx_set_standby_video                                                             **
** Desc.                                                                                 **
**     Sets the ACTV standby video PID                                                   **
** Params                                                                                **
**     VideoPID    New video PID to switch to                                            **
** Returns                                                                               **
**     Nothing                                                                           **
**                                                                                       **
**---------------------------------------------------------------------------------------*/
void gen_dmx_set_standby_video(u_int32 dmxid, u_int16 VideoPID) {
#if (LEGACY_DVR==NO)
    *DPS_VID_SPLICE_PID_REG_EX(dmxid) = (u_int32) VideoPID; 
#endif
}

/*--------------------------------------------------------------------------------------**
** gen_dmx_set_standby_audio                                                            **
** Desc.                                                                                **
**     Sets the ACTV standby audio PID                                                  **
** Params                                                                               **
**     VideoPID    New audio PID to switch to                                           **
** Returns                                                                              **
**     Nothing                                                                          **
**                                                                                      **
**--------------------------------------------------------------------------------------*/
void gen_dmx_set_standby_audio(u_int32 dmxid, u_int16 AudioPID) {
#if (LEGACY_DVR==NO)
    *DPS_AUD_SPLICE_PID_REG_EX(dmxid) = (u_int32) AudioPID; 
#endif
}

/*---------------------------------------------------------------------------------------**
** gen_dmx_enable_video_switch                                                           **
** Desc.                                                                                 **
**     Enables the switch to occur at the next splice count of 0.                        **
** Params                                                                                **
**     None                                                                              **
** Returns                                                                               **
**     Nothing                                                                           **
**                                                                                       **
**---------------------------------------------------------------------------------------*/
void gen_dmx_enable_video_switch(u_int32 dmxid) {
#if (LEGACY_DVR==NO)
    *DPS_HOST_CTL_REG_EX(dmxid) |= DPS_SPLICE_VIDEO; 
#endif
}

/*--------------------------------------------------------------------------------------**
** gen_dmx_enable_audio_switch                                                          **
** Desc.                                                                                **
**     Enables the switch to occur at the next splice count of 0.                       **
** Params                                                                               **
**     None                                                                             **
** Returns                                                                              **
**     Nothing                                                                          **
**                                                                                      **
**--------------------------------------------------------------------------------------*/
void gen_dmx_enable_audio_switch(u_int32 dmxid) {
#if (LEGACY_DVR==NO)
    *DPS_HOST_CTL_REG_EX(dmxid) |= DPS_SPLICE_AUDIO; 
#endif
}

/*--------------------------------------------------------------------------------------**
** gen_dmx_get_video_switch_status                                                      **
** Desc.                                                                                **
**     Returns the switch status for the video.                                         **
** Params                                                                               **
**     None                                                                             **
** Returns                                                                              **
**     0 if switch has occured                                                          **
**     1 if switch has not occured                                                      **
**                                                                                      **
**--------------------------------------------------------------------------------------*/
u_int32 gen_dmx_get_video_switch_status(u_int32 dmxid) {
#if (LEGACY_DVR==NO)
    return(u_int32)(*DPS_HOST_CTL_REG_EX(dmxid) & DPS_SPLICE_VIDEO);
#else
    return(1);
#endif
}

/*--------------------------------------------------------------------------------------**
** gen_dmx_get_audio_switch_status                                                      **
** Desc.                                                                                **
**     Returns the switch status for the video.                                         **
** Params                                                                               **
**     None                                                                             **
** Returns                                                                              **
**     0 if switch has occured                                                          **
**     <> 0 if switch has not occured                                                   **
**                                                                                      **
**--------------------------------------------------------------------------------------*/
u_int32 gen_dmx_get_audio_switch_status(u_int32 dmxid) {
#if (LEGACY_DVR==NO)
    return(u_int32)(*DPS_HOST_CTL_REG_EX(dmxid) & DPS_SPLICE_AUDIO);
#else
    return(1);
#endif
}

/****************************************************************************
 * Modifications:
 * $Log: 
 *  35   mpeg      1.34        5/18/04 2:43:49 PM     Tim White       CR(s) 
 *        9236 9235 : Use DmxID for using correct demux instance when loading 
 *        microcode instead of
 *        the hardcoded values.
 *        
 *  34   mpeg      1.33        3/23/04 3:17:42 PM     Larry Wang      CR(s) 
 *        8638 8639 : Load XTV microcode in the same way as PVR application.
 *  33   mpeg      1.32        3/10/04 10:44:29 AM    Bob Van Gulick  CR(s) 
 *        8546 : Add support to return PES frame rate
 *        
 *  32   mpeg      1.31        1/12/04 6:24:56 PM     Yong Lu         CR(s) 
 *        8078 : added support for little endian
 *  31   mpeg      1.30        12/23/03 10:35:56 AM   Mark Thissen    CR(s) 
 *        8165 : Changed code so that it toggles video sync with PTS for PES 
 *        type video only and audio sync with PTS for PES type audio only.
 *        
 *  30   mpeg      1.29        11/19/03 10:09:59 AM   Tim White       CR(s): 
 *        7987 Added Demux DMA and Demux PVR extension support phase 1.
 *        
 *  29   mpeg      1.28        9/22/03 4:52:48 PM     Bob Van Gulick  SCR(s) 
 *        7519 :
 *        Add support for DirecTV CAPs
 *        
 *        
 *  28   mpeg      1.27        9/16/03 4:05:48 PM     Tim White       SCR(s) 
 *        7474 :
 *        Do not disable already running demux.  Load proper microcode onto 
 *        selected demux
 *        based on capability bits.  Set MASTER bit only if it's the 
 *        playback/live demux
 *        based on capability bits.
 *        
 *        
 *  27   mpeg      1.26        9/2/03 7:03:08 PM      Joe Kroesche    SCR(s) 
 *        7415 :
 *        removed unneeded header files to eliminate extra warnings when 
 *        building
 *        for PSOS
 *        
 *  26   mpeg      1.25        8/27/03 11:01:24 AM    Bob Van Gulick  SCR(s) 
 *        7387 :
 *        Add support for CWP processing in DirecTV 
 *        
 *        
 *  25   mpeg      1.24        5/6/03 5:33:24 PM      Craig Dry       SCR(s) 
 *        5521 :
 *        Conditionally remove access to GPIO Pin Mux registers.
 *        
 *  24   mpeg      1.23        5/2/03 11:12:36 AM     Bob Van Gulick  SCR(s) 
 *        6151 :
 *        Change pts_offset settings to use new demux function
 *        
 *        
 *  23   mpeg      1.22        4/24/03 5:47:14 PM     Tim White       SCR(s) 
 *        6097 :
 *        Allow 6 descrambled simultaneous PES channels.  Remove #ifndef 
 *        USE_OLD_PES code.
 *        
 *        
 *  22   mpeg      1.21        4/3/03 9:56:24 AM      Larry Wang      SCR(s) 
 *        5952 :
 *        Skip pawser microcode version check for DirecTV parser.
 *        
 *  21   mpeg      1.20        4/2/03 11:58:36 AM     Brendan Donahe  SCR(s) 
 *        5886 :
 *        Modifications to support 6 simultaneous unscrambled/scrambled SI 
 *        including
 *        channels usually used for video and audio (1 & 2) as well as 
 *        enforcing correct
 *        parser microcode version which has been changed in conjunction with 
 *        this 
 *        feature enhancement.
 *        
 *        
 *  20   mpeg      1.19        3/25/03 11:47:14 AM    Brendan Donahe  SCR(s) 
 *        5845 :
 *        Code to support initialization (zeroing out) of 4 version mode 
 *        filtering
 *        registers on Brazos.
 *        
 *        
 *  19   mpeg      1.18        3/3/03 2:07:30 PM      Ray Mack        SCR(s) 
 *        5439 :
 *        INSERT_VIDEO_SEQ_ERRORS should not be set in any of our drivers.
 *        
 *  18   mpeg      1.17        3/3/03 11:10:34 AM     Larry Wang      SCR(s) 
 *        5631 :
 *        Replace hard coded number 188 with TS_COPY_BLOCK_SIZE.
 *        
 *  17   mpeg      1.16        12/17/02 4:59:14 PM    Dave Wilson     SCR(s) 
 *        5185 :
 *        Removed some redundant code which was specific to our old IKOS 
 *        emulator.
 *        
 *  16   mpeg      1.15        12/16/02 3:45:30 PM    Tim White       SCR(s) 
 *        5169 :
 *        Removed unnecessary chip_id and chip_rev detection code.
 *        
 *        
 *  15   mpeg      1.14        11/15/02 12:59:14 PM   Tim White       SCR(s) 
 *        4935 :
 *        Removed support for two run-time detectable pawser microcodes.
 *        
 *        
 *  14   mpeg      1.13        10/16/02 3:20:46 PM    Bob Van Gulick  SCR(s) 
 *        4799 :
 *        Remove CANAL_PLUS_FILTERING #ifdefs and use #if PARSER_FILTERING == 
 *        FILTER_xxx
 *        instead.  PARSER_FILTERING is defined in the sw config
 *        
 *        
 *  13   mpeg      1.12        9/19/02 3:42:48 PM     Joe Kroesche    SCR(s) 
 *        4610 :
 *        added crc notification feature, removed changes for previous crc 
 *        notification
 *        method. NOTE!!! requires matching pawser ucode update of #4626
 *        
 *  12   mpeg      1.11        9/18/02 4:24:42 PM     Joe Kroesche    SCR(s) 
 *        4619 :
 *        Added conditionally compiled code to support Canal+ special cases
 *        
 *  11   mpeg      1.10        9/5/02 6:29:56 PM      Bob Van Gulick  SCR(s) 
 *        4530 :
 *        Change CRC check to use Header Notify instead of Section Notify
 *        
 *        
 *  10   mpeg      1.9         8/29/02 4:40:46 PM     Brendan Donahe  SCR(s) 
 *        4489 4490 :
 *        Now uses dmxid when loading microcode instead of hardcoded 0
 *        Changed genDemuxInitSW call to pass dmxid instead of dmx pointer
 *        
 *        
 *  9    mpeg      1.8         8/16/02 6:05:04 PM     Tim White       SCR(s) 
 *        4420 :
 *        Add support for new DVR microcode which supports DVR, XPRT, and 
 *        MULTI_PSI together.
 *        
 *        
 *  8    mpeg      1.7         6/27/02 5:57:34 PM     Tim White       SCR(s) 
 *        4108 :
 *        Add magic char to Header for PVCS to deal with.
 *        
 *        
 *  7    mpeg      1.6         5/13/02 12:11:00 PM    Tim White       SCR(s) 
 *        3760 :
 *        Renamed DPS_ HSDP definitions to be HSDP_ and only use when the HSDP 
 *        driver is not
 *        linked with the application.
 *        
 *        
 *  6    mpeg      1.5         4/26/02 3:16:08 PM     Tim White       SCR(s) 
 *        3562 :
 *        Add support for Colorado Rev_F.
 *        
 *        
 *  5    mpeg      1.4         3/25/02 4:24:36 PM     Tim White       SCR(s) 
 *        3433 :
 *        PVR is always defined, if PVR is to be used, it's defined to be a 1, 
 *        otherwise a 0.
 *        
 *        
 *  4    mpeg      1.3         3/15/02 12:52:52 PM    Tim White       SCR(s) 
 *        3352 :
 *        Add support for loading multiple microcodes on different pawsers for 
 *        Hondo PCR support.
 *        
 *        
 *  3    mpeg      1.2         2/20/02 10:20:26 AM    Bob Van Gulick  SCR(s) 
 *        3173 :
 *        remove <>'s from include of demuxint.h
 *        
 *        
 *  2    mpeg      1.1         2/7/02 11:51:48 AM     Bob Van Gulick  SCR(s) 
 *        3143 :
 *        fix compiler warnings by adding function prototypes
 *        
 *        
 *  1    mpeg      1.0         12/18/01 11:00:44 AM   Bob Van Gulick  
 * $
 ****************************************************************************/

