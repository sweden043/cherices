/****************************************************************************/
/*                   CONEXANT PROPRIETARY AND CONFIDENTIAL                  */
/*                       SOFTWARE FILE/MODULE HEADER                        */
/*           Conexant Systems Inc. (c) 1998, 1999, 2000, 2001, 2002         */
/*                               Austin, TX                                 */
/*                            All Rights Reserved                           */
/****************************************************************************/
/*
 * Filename:       TASKPRIO.H
 *
 *
 * Description:    Task names and priority definitions
 *
 *
 * Author:         Dave Wilson
 *
 ****************************************************************************/
/* $Id: taskprio.h,v 1.19, 2004-06-22 20:44:36Z, Bobby Bradford$
 ****************************************************************************/

#ifndef _TASKPRIO_H_
#define _TASKPRIO_H_

#define DMOD_TASK_NAME                  "DMOD"
#define DMOD_TASK_PRIORITY              41
#define DMOD_TASK_STACK_SIZE            0x0600

#define ISRH_TASK_NAME                  "ISRH"
#define ISRH_TASK_PRIORITY              DEFAULT_PRIORITY
#define ISRH_TASK_STACK_SIZE            0x1400

#define STCK_TASK_NAME                  "STCK"
#define STCK_TASK_PRIORITY              DEFAULT_PRIORITY
#define STCK_TASK_STACK_SIZE            0x1400

#define TLNT_TASK_NAME                  "TLNT"
#define TLNT_TASK_PRIORITY              DEFAULT_PRIORITY
#define TLNT_TASK_STACK_SIZE            0x1400

#define  CMD_TASK_NAME                  "CMD"
#define  CMD_TASK_PRIORITY              DEFAULT_PRIORITY
#define  CMD_TASK_STACK_SIZE            0x1400

#define CLEN_TASK_NAME                  "CLEN"
#define CLEN_TASK_PRIORITY              DEFAULT_PRIORITY
#define CLEN_TASK_STACK_SIZE            0x0400

#define MIP_IDLE_TASK_PRIORITY          1

#define LOWP_TASK_NAME                  "LOWP"
#define LOWP_TASK_PRIORITY              (MIP_IDLE_TASK_PRIORITY + 1) 
#define LOWP_TASK_STACK_SIZE            0x0400

#define  PSI_TASK_NAME                  "PSI"
#define  PSI_TASK_PRIORITY              (DEFAULT_PRIORITY+1)
#define  PSI_TASK_STACK_SIZE            0x2800

#define  ECM_TASK_NAME                  "ECM"
#define  ECM_TASK_PRIORITY              DEFAULT_PRIORITY
#define  ECM_TASK_STACK_SIZE            0x0800

#define  CWP_TASK_NAME                  "CWP"
#define  CWP_TASK_PRIORITY              DEFAULT_PRIORITY
#define  CWP_TASK_STACK_SIZE            0x0800

#define  CAP_TASK_NAME                  "CAP"
#define  CAP_TASK_PRIORITY              DEFAULT_PRIORITY
#define  CAP_TASK_STACK_SIZE            0x0800

#define  PIP_TASK_NAME                  "PIP"
#define  PIP_TASK_PRIORITY              DEFAULT_PRIORITY
#define  PIP_TASK_STACK_SIZE            0x0800

#define  TPT_TASK_NAME                  "TPT"
#define  TPT_TASK_PRIORITY              DEFAULT_PRIORITY+5
#define  TPT_TASK_STACK_SIZE            0x0400

#define  IFCT_TASK_NAME                 "IFCT"
#define  IFCT_TASK_PRIORITY             DEFAULT_PRIORITY
#define  IFCT_TASK_STACK_SIZE           DEFAULT_STACK_SIZE

#define  IFCB_TASK_NAME                 "IFCB"
#define  IFCB_TASK_PRIORITY             DEFAULT_PRIORITY
#define  IFCB_TASK_STACK_SIZE           DEFAULT_STACK_SIZE

/* It is highly critical for AC3 Passthrough for this task to get a chance to 
 * run. It is a short, well behaved task, that gets run every 32 milliseconds 
 */
#define  AFTK_TASK_NAME                 "AFTK"
#define  AFTK_TASK_PRIORITY             DEFAULT_PRIORITY + 20
#define  AFTK_TASK_STACK_SIZE           DEFAULT_STACK_SIZE

/*
 * The priority of the NDS tasks are in the 200+ range going to
 * 218.  The NDS task is the task that feeds these NDS tasks inside
 * the verifier with new ECM and EMM packets.  It *has* to be at
 * at least the priority of the NDS tasks to ensure the latency
 * is kept to a minimum to avoid buffer overflow for both the
 * ECM's and EMM's.  Do not change the priority of this task.  It
 * is a well-formed non-premptive queue driven task.
 */

#define  NDS_TASK_NAME                  "NDS"
#define  NDS_TASK_PRIORITY              220
#define  NDS_TASK_STACK_SIZE            0x0800

/* 
 * The NDS Smart Card Task supplies Smart Card events to the other 
 * NDS tasks. This task cannot wait for too long. So this task has to be higher
 * than other system tasks 
 */
#define NSCT_TASK_NAME                  "NSCT"
#define NSCT_TASK_PRIORITY              DEFAULT_PRIORITY+11
#define NSCT_TASK_STACK_SIZE            0x0800

#define RMPT_TASK_NAME                  "RMPT"
#define RMPT_TASK_PRIORITY              DEFAULT_PRIORITY
#define RMPT_TASK_STACK_SIZE            0x0400

/* PCM Driver PCM Mixing task information */
#define PCMM_TASK_NAME                  "PCMM"
#define PCMM_TASK_PRIORITY              DEFAULT_PRIORITY+5
#define PCMM_TASK_STACK_SIZE            DEFAULT_STACK_SIZE

/* PCM Driver Sample Rate Convert task information */
#define SSRC_TASK_NAME                  "SSRC"
#define SSRC_TASK_PRIORITY              DEFAULT_PRIORITY+5
#define SSRC_TASK_STACK_SIZE            DEFAULT_STACK_SIZE

#define  VID_TASK_NAME                  "VID"
#define  VID_TASK_PRIORITY              DEFAULT_PRIORITY+8
#define  VID_TASK_STACK_SIZE            0x0600

#define  AID_TASK_NAME                  "AID"
#define  AID_TASK_PRIORITY              DEFAULT_PRIORITY+8
#define  AID_TASK_STACK_SIZE            0x0600

#define G7DT_TASK_NAME                  "G7DT"
#define G7DT_TASK_PRIORITY              DEFAULT_PRIORITY
#define G7DT_TASK_STACK_SIZE            0x1400

#define OSD2_TASK_NAME                  "OSD2"
#define OSD2_TASK_PRIORITY              DEFAULT_PRIORITY
#define OSD2_TASK_STACK_SIZE            0x0400

#define TTEX_TASK_NAME                  "TTEX"
#define TTEX_TASK_PRIORITY              DEFAULT_PRIORITY+5
#define TTEX_TASK_STACK_SIZE            0x0800

#define TTXV_TASK_NAME                  "TTXV"
#define TTXV_TASK_PRIORITY              DEFAULT_PRIORITY+5
#define TTXV_TASK_STACK_SIZE            0x0400

#define TTXS_TASK_NAME                  "TTXS"
#define TTXS_TASK_PRIORITY              DEFAULT_PRIORITY+5
#define TTXS_TASK_STACK_SIZE            0x0400

#define CLKT_TASK_NAME                  "CLKT"
#define CLKT_TASK_PRIORITY              DEFAULT_PRIORITY
#define CLKT_TASK_STACK_SIZE            0x0400

#define BUFT_TASK_NAME                  "BUFT"
#define BUFT_TASK_PRIORITY              DEFAULT_PRIORITY
#define BUFT_TASK_STACK_SIZE            0x0400

#define ACQT_TASK_NAME                  "ACQT"
#define ACQT_TASK_PRIORITY              DEFAULT_PRIORITY+1
#define ACQT_TASK_STACK_SIZE            0x0600

/* Thomson cable front-end task information */
#define NIM_TASK_NAME                   "CF_T"
#define NIM_TASK_PRIORITY               DEFAULT_PRIORITY+1
#define NIM_TASK_STK_SIZE               0x0600

#define BASEBAND_TASK_NAME              "BAST"
#define BASEBAND_TASK_PRIORITY          DEFAULT_PRIORITY
#define BASEBAND_TASK_STACK_SIZE        0x0600

#define SCRT_TASK_NAME                  "SCRT"
#define SCRT_TASK_PRIORITY              DEFAULT_PRIORITY
#define SCRT_TASK_STACK_SIZE            0x0400

#define BEEP_TASK_NAME                  "BEEP"
#define BEEP_TASK_PRIORITY              DEFAULT_PRIORITY
#define BEEP_TASK_STACK_SIZE            0x0400

#define MDXT_TASK_NAME                  "MDXT"
#define MDXT_TASK_PRIORITY              DEFAULT_PRIORITY
#define MDXT_TASK_STACK_SIZE            0x0400

#define SMCD_TASK_NAME                  "SMCD"
#define SMCD_TASK_PRIORITY              DEFAULT_PRIORITY
#define SMCD_TASK_STACK_SIZE            0x0600

#define IRTA_TASK_NAME                  "IRTA"
#define IRTA_TASK_PRIORITY              DEFAULT_PRIORITY+10
#define IRTA_TASK_STACK_SIZE            0x0400

#define IRPT_TASK_NAME                  "IRPT"
#define IRPT_TASK_PRIORITY              DEFAULT_PRIORITY+10
#define IRPT_TASK_STACK_SIZE            0x0400

#define IRDT_TASK_NAME                  "IRDT"
#define IRDT_TASK_PRIORITY              DEFAULT_PRIORITY+10
#define IRDT_TASK_STACK_SIZE            0x0400

#define OCTP_TASK_NAME                  "OCTP"  /* octopus modem */
#define OCTP_TASK_PRIORITY              DEFAULT_PRIORITY
#define OCTP_TASK_STACK_SIZE            0x1000

#define SOAR_TASK_NAME                  "SOAR"
#define SOAR_TASK_PRIORITY              DEFAULT_PRIORITY+2
#define SOAR_TASK_STACK_SIZE            0x0600

#define MAUX_TASK_NAME                  "MAUX"
#define MAUX_TASK_PRIORITY              DEFAULT_PRIORITY-4
#define MAUX_TASK_STACK_SIZE            DEFAULT_STACK_SIZE*4

/* The PSI Buffer Full task is trivial - it waits on a semaphore   */
/* then sleeps then sets a hardware register then back to waiting  */
/* on the semaphore. It needs to be very high priority to re-awake */
/* the demux ISR when the PSI buffer is full.                      */
#define PBF_TASK_NAME                   "PBFT" /* PSI Buffer Full Task */
#define PBF_TASK_PRIORITY               DEFAULT_PRIORITY+6
#define PBF_TASK_STACK_SIZE             0x0400

#define E2PDRV_TASK_NAME                "ETSK"
#define E2PDRV_TASK_PRIORITY            DEFAULT_PRIORITY
#define E2PDRV_TASK_STACK_SIZE          DEFAULT_STACK_SIZE

#define TRQH_TASK_NAME                  "TRQH"
#define TRQH_TASK_PRIORITY              DEFAULT_PRIORITY
#define TRQH_TASK_STACK_SIZE            DEFAULT_STACK_SIZE

#define FTIR_TASK_NAME                  "FTIR"
#define FTIR_TASK_PRIORITY              DEFAULT_PRIORITY
#define FTIR_TASK_STACK_SIZE            DEFAULT_STACK_SIZE

#define BKIR_TASK_NAME                  "BKIR"
#define BKIR_TASK_PRIORITY              DEFAULT_PRIORITY
#define BKIR_TASK_STACK_SIZE            DEFAULT_STACK_SIZE

#define CTBT_TASK_NAME                  "CTBT"
#define CTBT_TASK_PRIORITY              DEFAULT_PRIORITY
#define CTBT_TASK_STACK_SIZE            2048

#define VDBG_TASK_NAME                  "VDBG"
#define VDBG_TASK_PRIORITY              DEFAULT_PRIORITY
#define VDBG_TASK_STACK_SIZE            DEFAULT_STACK_SIZE

#define WTMT_TASK_NAME                  "WTMT"
#define WTMT_TASK_PRIORITY              DEFAULT_PRIORITY
#define WTMT_TASK_STACK_SIZE            DEFAULT_STACK_SIZE

#define REDIR_TASK_NAME                 "RDIR"
#define REDIR_TASK_PRIORITY             DEFAULT_PRIORITY+4
#define REDIR_TASK_STACK_SIZE           DEFAULT_STACK_SIZE

#define CC_ATSC_TASK_NAME               "CC_ATSC"
#define CC_ATSC_TASK_PRIORITY           DEFAULT_PRIORITY
#define CC_ATSC_TASK_STACK_SIZE         DEFAULT_STACK_SIZE

#define DCI0_TASK_NAME                  "DCI0"
#define DCI0_TASK_PRIORITY              DEFAULT_PRIORITY
#define DCI0_TASK_STACK_SIZE            DEFAULT_STACK_SIZE

#define DCI1_TASK_NAME                  "DCI1"
#define DCI1_TASK_PRIORITY              (DEFAULT_PRIORITY-1)
#define DCI1_TASK_STACK_SIZE            DEFAULT_STACK_SIZE

#define CHMD_TASK_NAME                  "CHMD"
#define CHMD_TASK_PRIORITY              DEFAULT_PRIORITY
#define CHMD_TASK_STACK_SIZE            DEFAULT_STACK_SIZE

/* CABLE MODEM */

/* CM_MUX - Cable Modem Packet MUX/DEMUX */
/* (Note: so far, i have not investigated using smaller stack sizes - D.M.)  */

/* Also note the relationship in the CM_MUX priorities (except for the */
/* debug print task) should be maintained.                             */
#define CM_MUX_BASE_PRIORITY DEFAULT_PRIORITY

/* This task brings up the Network and initiates a DHCP Boot then exits */
#define CM_MUX_DHCP_TASK_NAME           "DHCP"
#define CM_MUX_DHCP_PRIORITY            (CM_MUX_BASE_PRIORITY+4)
#define CM_MUX_DHCP_STACK_SIZE          DEFAULT_STACK_SIZE*2 /* yes, needs a big stack */

/* This task prints diagnostics when compiled DEBUG */
#define CM_MUX_PRINT_TASK_NAME          "CMPT"
#define CM_MUX_PRINT_PRIORITY           (CM_MUX_BASE_PRIORITY-1)
#define CM_MUX_PRINT_STACK_SIZE         DEFAULT_STACK_SIZE

/* This task drives ETHERNET frames upstream (to Headend Server) */
#define CM_MUX_NET_TX_TASK_NAME         "CMTT"
#define CM_MUX_NET_TX_PRIORITY          (CM_MUX_BASE_PRIORITY+3)
#define CM_MUX_NET_TX_STACK_SIZE        DEFAULT_STACK_SIZE

/* This task drives UDP frames downstream (to IRD IP Stack) */
#define IPSTB_UDP_CLENT_RX_TASK_NAME    "UDPC"
#define IPSTB_UDP_CLENT_RX_PRIORITY     (CM_MUX_BASE_PRIORITY+5)
#define IPSTB_UDP_CLENT_RX_STACK_SIZE   DEFAULT_STACK_SIZE

/* This task drives ETHERNET frames downstream (to IRD IP Stack) */
#define CM_MUX_NET_RX_TASK_NAME         "CMRT"
#define CM_MUX_NET_RX_PRIORITY          (CM_MUX_BASE_PRIORITY+4)
#define CM_MUX_NET_RX_STACK_SIZE        DEFAULT_STACK_SIZE

/* This task drives CONTROL frames downstream (to IRD Control Clients) */
#define CM_MUX_CNTRL_MSG_TASK_NAME      "CMCT"
#define CM_MUX_CNTRL_MSG_PRIORITY       (CM_MUX_BASE_PRIORITY+4)
#define CM_MUX_CNTRL_MSG_STACK_SIZE     DEFAULT_STACK_SIZE

/* This task drives OOB frames downstream (to IRD OOB handler) */
#define CM_MUX_OOB_RX_TASK_NAME         "OOBR"
#define CM_MUX_OOB_RX_PRIORITY          (CM_MUX_BASE_PRIORITY+4)
#define CM_MUX_OOB_RX_STACK_SIZE        DEFAULT_STACK_SIZE

/* This task drives DSG IP frames downstream (to Host DSG App) */
#define CM_MUX_DSG_OOB_RX_TASK_NAME     "DSGR"
#define CM_MUX_DSG_OOB_RX_PRIORITY      (CM_MUX_BASE_PRIORITY+4)
#define CM_MUX_DSG_OOB_RX_STACK_SIZE    DEFAULT_STACK_SIZE

/* CM_END - VxW Ethernet over Cable Modem Enhanced Network Driver  */
/* (Note: so far, i have not investigated using smaller stack sizes - D.M.)  */

/* This task prints diagnostics when compiled DEBUG */
#define CM_END_PRINT_TASK_NAME          "CMEP"
#define CM_END_PRINT_PRIORITY           (DEFAULT_PRIORITY-1)
#define CM_END_PRINT_STACK_SIZE         DEFAULT_STACK_SIZE

/* END of CABLE MODEM */

#define ATVTUN_TASK_NAME                "AT"
#define ATVTUN_PRIORITY                 DEFAULT_PRIORITY
#define ATVTUN_STACK_SIZE               DEFAULT_STACK_SIZE

#define AVID_TASK_NAME                  "AVD"
#define AVID_PRIORITY                   DEFAULT_PRIORITY
#define AVID_STACK_SIZE                 DEFAULT_STACK_SIZE


/*
 * FLIPPER Softmdem Task Definitions
 */

/*
 * Off-Level ISR Task: Runs DMA and Event callbacks into
 * flipper datapump code. This task has hard real-time
 * requirements.
 * FILE: flipper\daa\daadrv.c
 */
#define FLIPPER_RXTX_TASK_NAME            "RXTX"
#define FLIPPER_RXTX_TASK_PRIORITY        (DEFAULT_PRIORITY+32)
#define FLIPPER_RXTX_STACK_SIZE           (4096) 

/*
 * Soar Host Task: This task does time delayed callbacks
 * into the SOAR driver to (primarily) run the state machine
 * FILE: flipper\daa\soarhost.c
 */
#define FLIPPER_SH_TASK_NAME              "SHTK"
#define FLIPPER_SH_TASK_PRIORITY          (DEFAULT_PRIORITY+15)
#define FLIPPER_SH_STACK_SIZE             (2048)

/*
 * Softmodem Controller Task:  This task processes all of the internal modem
 * threads.
 * FILE: flipper\ikmos\timerkal.c
 */
#define FLIPPER_MDMT_TASK_NAME            "MDMT"
#define FLIPPER_MDMT_TASK_PRIORITY        (DEFAULT_PRIORITY+32)
#define FLIPPER_MDMT_STACK_SIZE           (4096)


/*
 * End of FLIPPER Softmdem Task Definitions
 */

#endif /* _TASKPRIO_H_ */

/****************************************************************************
 * Modifications:
 * $Log: 
 *  20   mpeg      1.19        6/22/04 3:44:36 PM     Bobby Bradford  CR(s) 
 *        9538 9539 : Add task definitions for FLIPPER modem library tasks
 *  19   mpeg      1.18        5/21/04 3:03:00 AM     Steven Shen     CR(s) 
 *        9273 9274 : Add the DEMOD_DCF8722 task information for non-uCOS RTOS.
 *  18   mpeg      1.17        5/5/04 2:30:50 PM      Steve Glennon   CR(s) 
 *        9103 9102 : 
 *        Added DCI0 DCI1 and CHMD tasks
 *        
 *  17   mpeg      1.16        2/23/04 10:43:47 AM    Xin Golden      CR(s) 
 *        8437 : defined the task used in the closed caption driver.
 *  16   mpeg      1.15        2/20/04 4:43:29 AM     Ian Mitchell    CR(s) 
 *        8442 : Add task details for WTMT and REDIR tasks.
 *        
 *  15   mpeg      1.14        12/11/03 2:03:49 PM    Song Qiao       CR(s) 
 *        8125 : Added task info for IPSTB udp client task.
 *        
 *        
 *  14   mpeg      1.13        10/18/03 6:13:01 AM    Ian Mitchell    CR(s): 
 *        7681 Increased the priority of the LPOW task to one above the mipidle
 *         task.
 *  13   mpeg      1.12        10/2/03 6:28:54 PM     Larry Wang      SCR(s) 
 *        7615 :
 *        Bump up the priority of PSI task by 1.
 *        
 *  12   mpeg      1.11        9/23/03 4:28:20 PM     Craig Dry       SCR(s) 
 *        7532 :
 *        Add MIP_IDLE_TASK_PRIORITY.
 *        
 *  11   mpeg      1.10        9/22/03 4:51:40 PM     Bob Van Gulick  SCR(s) 
 *        7519 :
 *        Add support for DirecTV CAPs
 *        
 *        
 *  10   mpeg      1.9         8/27/03 11:03:04 AM    Bob Van Gulick  SCR(s) 
 *        7387 :
 *        Add support for CWP processing in DirecTV
 *        
 *        
 *  9    mpeg      1.8         7/29/03 10:54:40 AM    Tim Ross        SCR(s) 
 *        7064 :
 *        Shortened name of AVID task to 3 characters so that I could add
 *        a number to the end to distinguish between instances.
 *        
 *  8    mpeg      1.7         5/23/03 2:23:44 PM     Brendan Donahe  SCR(s) 
 *        6525 6524 :
 *        Smart card driver under VxW needed a little more stack space (lots of
 *         trace
 *        output).
 *        
 *        
 *  7    mpeg      1.6         5/16/03 9:19:24 PM     Tim Ross        SCR(s) 
 *        6295 6294 :
 *        Added details for AVID CC task.
 *        
 *  6    mpeg      1.5         5/8/03 6:23:38 PM      Tim Ross        SCR(s) 
 *        6276 6277 :
 *        Added analog TV tuner task(s) base-name, priority, and stack size.
 *        
 *  5    mpeg      1.4         3/24/03 2:51:20 PM     Miles Bintz     SCR(s) 
 *        5858 :
 *        merge in OOB CM tasks
 *        
 *        
 *  4    mpeg      1.3         3/4/03 11:57:20 AM     Ray Mack        SCR(s) 
 *        4944 :
 *        Increased stack size from 2K to 5K so there is enough margin.
 *        
 *  3    mpeg      1.2         11/25/02 3:42:46 PM    Senthil Veluswamy SCR(s) 
 *        5018 :
 *        Added defines/details for AC3Fix Task
 *        
 *  2    mpeg      1.1         11/20/02 2:21:16 PM    Craig Dry       SCR(s) 
 *        4991 :
 *        Canal+ DLI4.2 Audio Extensions and Audio Driver Enhancements
 *        
 *  1    mpeg      1.0         11/20/02 9:37:10 AM    Dave Wilson     
 * $
 *
 ****************************************************************************/

