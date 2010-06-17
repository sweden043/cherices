/****************************************************************************/
/*                   CONEXANT PROPRIETARY AND CONFIDENTIAL                  */
/*                       SOFTWARE FILE/MODULE HEADER                        */
/*                   Conexant Systems Inc. (c) 1998-2003                    */
/*                              Austin, TX                                  */
/*                         All Rights Reserved                              */
/****************************************************************************/
/*
 * Filename:        startupc.c
 *
 *
 * Description:     Hardware and software initialization.
 *
 *
 * Author:          Tim Ross
 *
 ****************************************************************************/
/* $Id: startupc.c,v 1.110, 2004-06-28 02:09:46Z, Xiao Guang Yan$
 ****************************************************************************/

/******************/
/* Include Files  */
/******************/
#include "stbcfg.h"
#include "basetype.h"
#include "board.h"
#include "ram.h"
#include "startuppci.h"
#include "pccard.h"
#include "startup.h"
#include "stbcfg.h"
#include "kal.h"
#include "leds.h"

/*
 * !!! HACK ALERT !!!
 * WARNING!!!!! Hack requested for supporting Bronco1 & Bronco3 at runtime.  When
 * Bronco1 boards disappear, so should this hack.
 * !!! HACK ALERT !!!
 */
#if defined (DRIVER_INCL_SCANBTNS)
#if ((FRONT_PANEL_KEYPAD_TYPE == FRONT_PANEL_KEYPAD_BRONCO) && \
     (I2C_CONFIG_EEPROM_ADDR != NOT_PRESENT))
#include "..\scanbtns\scanbtns.h"
extern button_data button_matrix[FRONT_PANEL_KEYPAD_NUM_ROWS][FRONT_PANEL_KEYPAD_NUM_COLS];
#endif
#endif /*#if defined (DRIVER_INCL_SCANBTNS)*/
/*
 * !!! HACK ALERT !!!
 * !!! HACK ALERT !!!
 */

#define WABASH_DEV_ID  0x5F6e14F1   /* Wabash Device/Vendor ID for PCI config space */
#define CM_DAC_PLL_REG 0x360030     /* Offset into CM of DAC_PLL register           */


/********************************/
/* External Function Prototypes */
/********************************/
extern void InitVectors(void);
extern void StartOS(void);
extern void OSVariableInit(void);
extern bool InitFlashInfo(void);
#if RTOS != VXWORKS
extern void InitMMUandCaches(void);
extern void SetupROMMemoryRegions(void);
#endif
extern unsigned long GetChipID(void);
extern unsigned long GetChipRev(void);
extern unsigned char GetBoardID(void);
extern unsigned char GetVendorID(void);
extern BOARD_TYPES BoardTypes[];
extern double ceil(double); /* from math.h */
extern int memcpy(char *, char *, int);
#ifdef STANDALONE
extern int entry(void);
#endif

/**********************/
/* Local Definitions  */
/**********************/
#if I2C_CONFIG_EEPROM_ADDR != NOT_PRESENT
extern void PopulateConfigTable(void);
#endif

void InitDACPLL(void);

/***************/
/* Global Data */
/***************/
u_int32 ArmClkFreq32, ArmClkPeriod32, MemClkFreq32, MemClkPeriod32;
int RAMSize, RAMWidth;
unsigned long ChipID, ChipRevID;
unsigned char BoardID, VendorID;
u_int32  gboard_type = 0;
u_int32  gboard_rev = 0;
extern bool  gbNeedmoreHasNandFlash;
CONFIG_TABLE  config_table;
extern u_int8 NumROMBanks;
extern BANK_INFO Bank[];

#ifdef DRIVER_INCL_ATA
extern void cnxt_ata_config(void);
#endif /* DRIVER_INCL_ATA */

int ConfigurationValid;
u_int32 xtal_frequency;

#if __ARMCC_VERSION > 99999 /* Only do this if the toolset is ADS */
extern void cnxt_main(void); /* This function is in startup\startup.s */
/********************************************************************/
/*  $$Sub$main                                                      */
/*                                                                  */
/*  PARAMETERS:                                                     */
/*      None.                                                       */
/*                                                                  */
/*  DESCRIPTION:                                                    */
/*      This is a wrapper around the conexant main function that    */
/*       makes sure the proper version is called from the ARM ADS   */
/*       library.                                                   */
/*                                                                  */
/*  RETURNS:                                                        */
/*      Nothing.                                                    */
/********************************************************************/
void $Sub$$main(void)
{
	cnxt_main();
}
#endif

/********************************************************************/
/*  CStartup                                                        */
/*                                                                  */
/*  PARAMETERS:                                                     */
/*      None.                                                       */
/*                                                                  */
/*  DESCRIPTION:                                                    */
/*      This is the entry point for the 'C' portion of the startup  */
/*      sequence.                                                   */
/*                                                                  */
/*  RETURNS:                                                        */
/*      Never returns.                                              */
/********************************************************************/
void CStartup(void)
{
    LPREG               pReg, pPrimary, pSecondary;
    int                 Card;
#ifdef STANDALONE
    int                 i;
    char                str[100];
#endif
    bool                UsingSyncIO = FALSE;


   /*
    * For Colorado chips (only), the Pawser and GXA need a bit set to allow
    * proper arbitration between them. This was introduced on the D1 chip rev.
    * This should not be carried through to newer chips.
    */
#ifdef PAWSER_GXA_ARBITRATOR
    LPREG glpASBModeReg = (LPREG) (RST_BASE + 0x20);
#endif /* defined(PAWSER_GXA_ARBITRATOR) */

    /*
     * Used to set the HSX Arbiter which controls priorities on HSX bus.
     * This should be carried through to newer chips starting with Wabash.
     */
#if HSX_ARBITER != HSX_ARBITER_NONE
    LPREG lpHSXarb = (LPREG)(RST_HSX_ARBITER_PRI_REG);
#endif

    ConfigurationValid = 0;
#if I2C_CONFIG_EEPROM_ADDR != NOT_PRESENT
    ConfigurationValid = *((int*)RST_SCRATCH_REG) & RST_SCRATCH_CFG_EEPROM_VALID_MASK;
    if (ConfigurationValid)
    {
       PopulateConfigTable();
       /********************************/
       /* Get Board type and Board Rev */
       /********************************/
       gboard_type = config_table.board_type;
       gboard_rev = config_table.board_rev;
    }
#else
    gboard_type = IRD_BOARD_TYPE;
#endif

    /*
     * Ensure LINKER_RAM_BASE >= HWBUF_TOP
     
     
    if(LINKER_RAM_BASE < HWBUF_TOP)
    {
        Checkpoint("!ERROR: LINKER_RAM_BASE < HWBUF_TOP\n\r");
        while(1);
    }
    */

    /**************************/
    /* Get Vendor & Board IDs */
    /**************************/
    VendorID  = GetVendorID();
    BoardID   = GetBoardID();

    /*****************************/
    /* Get Chip and Chip Rev IDs */
    /*****************************/
    ChipID    = GetChipID();
    ChipRevID = GetChipRev();
    
    /*********************************/
    /* Set the global xtal_frequency */
    /*********************************/
#if CRYSTAL_FREQUENCY == AUTOSENSE
  #if I2C_CONFIG_EEPROM_ADDR != NOT_PRESENT
    if(ConfigurationValid)
    {
        /*
         * Run-time select the Xtal frequency from the EEPROM data
         */
        xtal_frequency = config_table.xtal_speed;

        /*
         * If Brazos chips (and beyond), check ~io_cs3 to see if a
         * divide by 4 is required.
         */
        if(!(*((u_int32*)PLL_CONFIG0_REG) & 0x8))
        {
            xtal_frequency >>= 2;
        }
    }
    else
    {
        /*
         * Special case for Brazos Rev_A which doesn't have EEPROM but specifies
         * CRYSTAL_FREQUENCY = AUTOSENSE.  In this case, we know the xtal_frequency
         * is 10111000Hz.  This should be removed once Brazos RevA chips are out
         * of circulation.
         */
        if((ISBRAZOS) && (GetChipRev() == 0x0))
        {
            xtal_frequency = 10111000;
        }
        else
        {
            /*
             * It is an error to use run-time Xtal frequency selection without
             * the EEPROM data present (valid).
             */
            Checkpoint("Attempted run-time Xtal selection w/o EEPROM config data\n\r");
            while(1);
        }
    }
  #else
    /*
     * It is an error to use run-time Xtal frequency selection without
     * the EEPROM data present (valid).
     */
    #error "(CRYSTAL_FREQUENCY == AUTOSENSE && I2C_CONFIG_EEPROM_ADDR == NOT_PRESENT)"
  #endif
#else
    /*
     * If the CRYSTAL_FREQUENCY is specified, use it
     */
    xtal_frequency = CRYSTAL_FREQUENCY;
#endif

    /**********************************/
    /* Approximate the MemClk period. */
    /**********************************/
    CalcClkFreqAndPeriod (&ArmClkFreq32, &ArmClkPeriod32, ARM_PLL_SOURCE, xtal_frequency, FALSE);
    CalcClkFreqAndPeriod (&MemClkFreq32, &MemClkPeriod32, MEM_PLL_SOURCE, xtal_frequency, FALSE);
    /* MemClkPeriod32 is in 100ns units, use this fact below...	*/

    /***********************/
    /* Disable interrupts. */
    /***********************/
#ifndef STANDALONE
    Checkpoint("Disabling interrupts via the interrupt controller...\n\r");
    disable_all_hw_ints();
#endif

    /*******************************/
    /* Determine memory bus width. */
    /*******************************/
    RAMWidth = GetRAMWidth();

    /***********************************/
    /* Determine how much RAM we have. */
    /***********************************/
    RAMSize = GetRAMSize();

    /********************/
    /* Initialize ROMs. */
    /********************/
    Checkpoint("Initializing ROM controller registers...\n\r");

    /*
     * Call routine to setup ROM controller registers.
     */
    SetupROMs();
    InitFlashInfo();

    /*************************/
    /* Initialize ISA slots. */
    /*************************/
    Checkpoint("Initializing ISA controller registers...\n\r");

    /***********************************/
    /* Setup ISA cards for that board. */
    /***********************************/
    /* Cycle through each ISA card we know. */
    for (Card=0; Card < iNumISADevices; Card++)
    {
        if(gbNeedmoreHasNandFlash)
        {
           if(ISADevices[Card].DescNum == CNXT_NANDFLASH_DESCRIPTOR)
           {
               /* don't setup Nand flash desc here, It is done in SetupROMs */
               continue;
           }
        }

        pPrimary   = (LPREG) PCI_ISA_DESC_BASE + ISADevices[Card].DescNum;
        pSecondary = (LPREG) PCI_DESC2_BASE    + ISADevices[Card].DescNum;

        /* Setup the descriptor designated in the card's ISADevice */
        /* structure with the other parameters in the ISADevice    */
        /* structure.                                              */

        *pPrimary = (PCI_ISA_DESC_TYPE_ISA                                     |
                     (ISADevices[Card].ExtWait << PCI_ISA_DESC_EXT_WAIT_SHIFT) |
                     (ISADevices[Card].IOMemory << PCI_ISA_DESC_IO_TYPE_SHIFT) |
                     (ISADevices[Card].ChipSelect << PCI_ISA_DESC_CS_SHIFT));

            switch (ISADevices[Card].IORegMode)
            {
                case PCCARD:
                    *pPrimary |= PCI_NEEDS_ISA_REG;
                    *pPrimary &= ~PCI_ISA_DESC_REG_IS_PROG;
                    break;
                case PROGRAMMABLE:
                    *pPrimary |= PCI_NEEDS_ISA_REG;
                    *pPrimary |= PCI_ISA_DESC_REG_IS_PROG;
                    break;
                case OFF:
                default:
                    *pPrimary &= ~PCI_NEEDS_ISA_REG;
                    *pPrimary &= ~PCI_ISA_DESC_REG_IS_PROG;
            }

        /* Set syncIO mode if required */
        *pPrimary = ((*pPrimary & ~PCI_ISA_DESC_SYNC_IO_ENABLE_MASK) |
                     (ISADevices[Card].bClocked << PCI_ISA_DESC_SYNC_IO_ENABLE_SHIFT));
        if(ISADevices[Card].bClocked)
        {
            UsingSyncIO = TRUE;
        }

        /* The timings designated in each card's ISADevice structure   */
        /* specify absolute time in ns. However, the descriptor itself */
        /* interprets timings in number of memclk cycles. Therefore,   */
        /* we have to convert the absolute time required by the card   */
        /* into memclk cycles.                                         */

        *pPrimary = ( (*pPrimary & ~PCI_ISA_DESC_WRITE_WAIT_MASK) |
            ( ( (unsigned int)( ((unsigned int)ISADevices[Card].WriteAccessTime*100)/MemClkPeriod32 + 1) ) <<
                                   PCI_ISA_DESC_WRITE_WAIT_SHIFT));
        *pPrimary = ((*pPrimary & ~PCI_ISA_DESC_READ_WAIT_MASK) |
            ( ( (unsigned int)( ((unsigned int)ISADevices[Card].ReadAccessTime*100)/MemClkPeriod32 + 1) ) <<
                                   PCI_ISA_DESC_READ_WAIT_SHIFT));

        /* *((LPREG)PCI_DESC2_REG+ISADevices[Card].DescNum) = 0xff021021; */


        *pPrimary &= ~(PCI_ISA_DESC_SETUP_TIME_MASK | PCI_ISA_DESC_HOLD_TIME_MASK);


        *pSecondary = (
               (( (unsigned int)( ((unsigned int)ISADevices[Card].IORegSetupTime*100)/MemClkPeriod32 + 1) ) <<
                             PCI_DESC2_REG_SETUP_TIME_SHIFT)  |
               (( (unsigned int)( ((unsigned int)ISADevices[Card].IORegAccessTime*100)/MemClkPeriod32 + 1) ) <<
                             PCI_DESC2_REG_ACCESS_TIME_SHIFT) |
               (( (unsigned int)( ((unsigned int)ISADevices[Card].CSSetupTime*100)/MemClkPeriod32 + 1) ) <<
                             PCI_DESC2_CS_SETUP_TIME_SHIFT)   |
               (( (unsigned int)( ((unsigned int)ISADevices[Card].CSHoldTime*100)/MemClkPeriod32 + 1) ) <<
                             PCI_DESC2_CS_HOLD_TIME_SHIFT)    |
               (( (unsigned int)( ((unsigned int)ISADevices[Card].CtrlSetupTime*100)/MemClkPeriod32 + 1) ) <<
                             PCI_DESC2_CTRL_SETUP_TIME_SHIFT) |
               (( (unsigned int)( ((unsigned int)ISADevices[Card].CtrlHoldTime*100)/MemClkPeriod32 + 1) ) <<
                             PCI_DESC2_ADDR_HOLD_TIME_SHIFT)
                      );


        pReg = (LPREG)PCI_ROM_XOE_MASK_REG;

        *pReg = *pReg |
            (ISADevices[Card].XOEMask << ISADevices[Card].ChipSelect);
    }

    /* If anything is using SyncIO, enable it */
    if(UsingSyncIO)
    {
        /*****************************************
         * Enable Program synchronous I/O clock  *
         *****************************************/
         pReg = (LPREG)PCI_ROM_MODE_REG;
         *pReg = *pReg | 0x400;
    }

    /************************************/
    /* Program RTC to tick at 1ms rate. */
    /************************************/
    Checkpoint("Setting the RTC clock...\n\r");
    pReg = (LPREG)PLL_RTC_DIVIDER_REG;
    *pReg = PLL_RTC_DIV_54MHz_TO_1000Hz;


#if (HAS_INTERNAL_PCI32 == YES) || \
    ((HAS_EXTERNAL_PCI32 == YES) && (EXTERNAL_PCI_IO_MODE != IO_MODE))
    /**********************************************************/
    /* Configure as PCI Master and scan for devices           */
    /**********************************************************/

    /*
     * Has internal PCI or external PCI possibly in PCI mode.  PCI
     * may need to be initialized.
     */
    #if (HAS_EXTERNAL_PCI32 == YES) && (EXTERNAL_PCI_IO_MODE == AUTOSENSE) \
        && (HAS_INTERNAL_PCI32 == NO)
    /*
     * Only do the check to see if external PCI is in
     * PCI or I/O mode if auto-sensing and has only an
     * external PCI bus.  If chip had an internal PCI bus,
     * like Wabash, there would be no need to check because
     * InitPCI() should always be called for the internal PCI bus.
     */
    if ( !(*(LPREG)PLL_CONFIG0_REG & PLL_CONFIG0_CNXT_GENERIC_IO_PCI_MODE_MASK) )
        /*
         * If bit is set to 0, it indicates PCI strapped and so PCI should be initialized.
         */
    #endif
    {
        Checkpoint("Initializing PCI...\n\r");
        InitPCI();
        /* If Wabash class device, initialize DAC PLL (used for cable/OOB demods) */
        if (ISWABASH)
        {
         InitDACPLL();
        } /* endif */
    }
#endif

    /*****************************/
    /* Clear out GPIO registers. */
    /*****************************/
    Checkpoint("Initializing the GPIO pins...\n\r");

    /*
     * Init all GPIO interrupt sources to known inactive state
     */
    pReg = (LPREG)GPI_POS_EDGE_REG;
    *pReg = 0;
    pReg = (LPREG)GPI_NEG_EDGE_REG;
    *pReg = 0;


#if PLL_PIN_GPIO_MUX0_REG_DEFAULT == NOT_DETERMINED
    /*
     * All Conexant reference IRD's (to-date) use GPIO pin 10
     * connected to the IR out diodes.  When the chip comes out
     * of reset, the GPIO 10 line can float (older IRD's) with a
     * pullup resistor. The pullup resistor turns the IR out diodes
     * on which is bad.  Being on causes two problems:  1) It can
     * burn out the IR diode, and 2) We shouldn't be driving IR
     * all the time anyway since it's environmentally bad to be
     * blasting IR all the time.  Therefore, as soon as possible,
     * the IR out diodes should be turned off by setting GPIO 10
     * to its alternate function, an IR out driver, which defaults
     * to being off.  CARE MUST BE TAKEN FOR FUTURE REFERENCE
     * IRD's (AND CUSTOMER BOARD'S) TO ENSURE THIS IF OFF!!!
     */
    CNXT_SET(PLL_PIN_GPIO_MUX0_REG, PLL_PIN_GPIO_MUX0_PIO_10, PLL_PIN_GPIO_MUX0_PIO_10);
#endif /* PLL_PIN_GPIO_MUX0_REG_DEFAULT == NOT_DETERMINED  */

    /****************************************/
    /* Turn on the Power LED if appropriate */
    /****************************************/
    /*cnxt_led_initialize(FALSE);*/
   // cnxt_led_set(PIO_LED_POWER, CNXT_LED_LIT);

#ifdef DRIVER_INCL_ATA
    /***************************************************/
    /* Initialize ATA subsystem (if present)           */
    /***************************************************/
    Checkpoint("Configuring ATA subsystem...\n\r");
    cnxt_ata_config();
#endif /* DRIVER_INCL_ATA */

   /*
    * For Colorado chips (only), the Pawser and GXA need a bit set to allow
    * proper arbitration between them. This was introduced on the D1 chip rev.
    * This should not be carried through to newer chips.
    */
#ifdef PAWSER_GXA_ARBITRATOR
   (*glpASBModeReg) |= 0x8;	/* Turn on pawser-gxa arbitration in D1 */
#endif /* defined(PAWSER_GXA_ARBITRATOR) */

    /*
     * Set the HSX Arbiter, which controls priorities on HSX bus, to have
     * the pawser (SP0) have highest priority.  This should be carried
     * through to newer chips starting with Wabash.
     */
#if HSX_ARBITER != HSX_ARBITER_NONE
    *lpHSXarb |= RST_HSX_ARBITER_HIGH_SP0;
#endif

/*
 * !!! HACK ALERT !!!
 * WARNING!!!!! Hack requested for supporting Bronco1 & Bronco3 at runtime.  When
 * Bronco1 boards disappear, so should this hack.
 * !!! HACK ALERT !!!
 */
#if defined (DRIVER_INCL_SCANBTNS)
#if ((FRONT_PANEL_KEYPAD_TYPE == FRONT_PANEL_KEYPAD_BRONCO) && \
     (I2C_CONFIG_EEPROM_ADDR != NOT_PRESENT))
    if(!ConfigurationValid ||
         ((config_table.board_type == 0x00) && (config_table.board_rev == 0x01)))
    {
        button_matrix[0][0].column_code = (102+GPIO_DEVICE_ID_INTERNAL+GPIO_PIN_IS_INPUT +GPIO_POSITIVE_POLARITY);
        button_matrix[1][0].column_code = (102+GPIO_DEVICE_ID_INTERNAL+GPIO_PIN_IS_INPUT +GPIO_POSITIVE_POLARITY);
        button_matrix[2][0].column_code = (102+GPIO_DEVICE_ID_INTERNAL+GPIO_PIN_IS_INPUT +GPIO_POSITIVE_POLARITY);
    }
#endif
#endif /*#if defined (DRIVER_INCL_SCANBTNS)*/
/*
 * !!! HACK ALERT !!!
 * !!! HACK ALERT !!!
 */


    /*
     * We're done setting up pre-RTOS hardware initialization at this point.
     */

    /*
     * Disable and Invalidate the I Cache
     */
    DisableICache();
    FlushICache();

    /*
     * Setup the MMU/MPU for simple RTOS environments (i.e. not VxWorks).  The more
     * complex RTOS environments (i.e. VxWorks) perform their own MMU/MPU setup.
     */
#if RTOS != VXWORKS
    InitMMUandCaches();
    SetupROMMemoryRegions();
#endif

#ifdef STANDALONE
    Checkpoint("                          RUNNING                           \r\n");
    i = entry();
    sprintf(str, "                        FINISHED (%d)                        \r\n", i);
    Checkpoint(str);
    while(1);
#else
    /***************************************************/
    /* Initialize KAL interrupt handler functionality. */
    /***************************************************/
    Checkpoint("KAL early initialization...\n\r");
    kal_early_initialise();

    /*********************************/
    /* Initialize exception vectors. */
    /*********************************/
    Checkpoint("Initializing exception vectors...\n\r");
    InitVectors();

    /****************************/
    /* Initialize OS variables. */
    /****************************/
    Checkpoint("Initializing OS variables...\n\r");
    OSVariableInit();

    /*****************************************/
    /* Initialize OS components and boot OS. */
    /*****************************************/
    Checkpoint("Starting OS initialization...\n\r");
    StartOS();
#endif

}

/********************************************************************/
/*  InitDACPLL                                                      */
/*                                                                  */
/*  PARAMETERS:                                                     */
/*      None.                                                       */
/*                                                                  */
/*  DESCRIPTION:                                                    */
/*      Initializes the DAC PLL register in Wabash class devices.   */
/*      This can only be done after the PCI has been initialized    */
/*      and only should be called on PCI based designs. This reg    */
/*      controls the clock divider for the DAC and is used for      */
/*      the demodulators (video QAM, DOCSIS inband QAM, and OOB     */
/*      demods)                                                     */
/*                                                                  */
/*      This is centralized here so that the other different        */
/*      components (QAM demod driver, Cable Modem, DAVIC, OOB       */
/*      drivers do not need to worry about it. It is set once.      */
/*                                                                  */
/*  RETURNS: Nothing                                                */
/*                                                                  */
/********************************************************************/
void InitDACPLL(void)
{
   u_int32 pci_address;

   if (DAC_PLL_REG_VALUE != 0)
   {
      /* Get the address of the PCI aperture into the Cable Modem side */
      pci_get_bar(WABASH_DEV_ID, 0, &pci_address);

      if (pci_address != 0)
      {
         /* The following may look hokey, but we know the PCI base */
         /* address has the bottom bits zeroed, so we can just OR  */
          *(LPREG)(pci_address | CM_DAC_PLL_REG) = DAC_PLL_REG_VALUE;

      } /* endif */
   } /* endif */

}

/****************************************************************************
 * Modifications:
 * $Log:
 *  105  mpeg      1.104       11/10/2003 4:35:41 PM  Tim White       CR(s):
 *       7900 7901 Added capability to receive raw VCO frequency/period
 *       information.
 *  104  mpeg      1.103       11/1/2003 3:03:29 PM   Tim Ross        CR(s):
 *       7719 7762 Now reads ConfigurationValid from scratch register instead
 *       of from pawser RAM.
 *  103  mpeg      1.102       10/30/2003 4:07:56 PM  Tim White       CR(s):
 *       7756 7757 Stop using CalcMemClkPeriod() function and instead use the
 *       new function CalcFreqAndPeriod()
 *       which is done for both the ARM and MEM PLL's.  Add 3 new globals:
 *       ArmClkFreq32, ArmClkPeriod32,
 *       and MemClkFreq32 to go along with the already existing global
 *       MemClkPeriod32.
 *  102  mpeg      1.101       10/17/2003 9:45:59 AM  Larry Wang      CR(s):
 *       7673 Calculate MemClkPeriod locally based on MemClkPeriod32.
 *       MemClkPeriod is not global anymore.
 *  101  mpeg      1.100       9/2/2003 6:58:14 PM    Joe Kroesche    SCR(s)
 *       7415 :
 *       removed unneeded header files that were causing PSOS warnings
 *  100  mpeg      1.99        8/26/2003 5:12:14 PM   Sahil Bansal    SCR(s):
 *       7377
 *       Change setting of XOEMask register to use Chip Select as shifter and
 *       not Descriptor Number.
 *  99   mpeg      1.98        8/18/2003 7:15:40 AM   Ian Mitchell    SCR(s):
 *       7300
 *       New function "void $Sub$$main(void)" that is called from the ADS
 *       startup routine.
 *       It calls the function "cnxt_main" in startup.s
 *  98   mpeg      1.97        7/20/2003 10:12:34 AM  Tim White       SCR(s)
 *       7000 :
 *       The loaders and initial startup code use instruction caching (no
 *       MMU/MPU) only.
 *
 *  97   mpeg      1.96        7/9/2003 3:29:32 PM    Tim White       SCR(s)
 *       6901 :
 *       Phase 3 codeldrext drop.
 *
 *  96   mpeg      1.95        6/24/2003 6:38:50 PM   Tim White       SCR(s)
 *       6831 :
 *       Add flash, hsdp, demux, OSD, and demod support to codeldrext.
 *
 *  95   mpeg      1.94        6/9/2003 4:52:28 PM    Tim White       SCR(s)
 *       6752 :
 *       Do not call SetupROMMemoryRegions() under VxWorks.
 *
 *  94   mpeg      1.93        6/5/2003 5:32:58 PM    Tim White       SCR(s)
 *       6660 :
 *       Flash banks separately controlled by the 920 MMU using hardware
 *       virtual pagemapping.
 *
 *  93   mpeg      1.92        5/16/2003 12:37:50 AM  Steve Glennon   SCR(s):
 *       6388
 *       Added code to set the CM DAC PLL register using a value from the
 *       config file
 *       on Wabash class devices. This is to get this register set up in a
 *       single place rather than in several (QAM demod driver, CM, etc.)
 *
 *  92   mpeg      1.91        5/14/2003 4:51:46 PM   Tim White       SCR(s)
 *       6346 :
 *       Moved PLL calculation code into separate function in pllc.c.  Also
 *       added a runtime
 *       check to ensure LINKER_RAM_BASE >= HWBUF_TOP.
 *
 *  91   mpeg      1.90        5/6/2003 1:28:58 PM    Craig Dry       SCR(s)
 *       5521 :
 *       Conditionally remove access to GPIO Pin Mux register 0.
 *  90   mpeg      1.89        5/5/2003 5:06:52 PM    Tim White       SCR(s)
 *       6172 :
 *       Remove duplicate low-level boot support code and use startup directory
 *       for building
 *       codeldr.  Remove 7 segment LED support.
 *
 *  89   mpeg      1.88        4/30/2003 6:04:24 PM   Billy Jackman   SCR(s)
 *       6113 :
 *       Added variable CPUType and function GetCPUType to allow C code to get
 *       the
 *       type of CPU when built with CPU_TYPE=AUTOSENSE.
 *       Removed some vendor-specific code.
 *       Modified conditional compilation directives and code to add the
 *       capability to
 *       detect the CPU type (920 vs. 940) at runtime when CPU_TYPE=AUTOSENSE.
 *  88   mpeg      1.87        4/4/2003 6:02:20 PM    Craig Dry       SCR(s)
 *       5956 :
 *       Initialize ConfigurationValid to 0.
 *  87   mpeg      1.86        4/4/2003 5:33:48 PM    Craig Dry       SCR(s)
 *       5956 :
 *       Move the ConfigurationValid flag from program memory to pawser memory.
 *       Previously startup.s declared the flag in program memory, but the C
 *       initialization of data areas wiped it out for rom links.  The ram
 *       links
 *       worked just fine.  Pawser memory avoids the problem altogether.
 *  86   mpeg      1.85        4/2/2003 11:26:12 AM   Bobby Bradford  SCR(s)
 *       5943 :
 *       Added conditional for DRIVER_INCL_SCANBTNS around the "hack" code.
 *  85   mpeg      1.84        4/1/2003 2:06:30 PM    Tim White       SCR(s)
 *       5925 :
 *       Added runtime support for both Bronco1 & Bronco3 boards with one
 *       hwconfig file.  ALL CHANGES MADE WITH THIS DEFECT ARE HACKS.  Please
 *       remove when support for Bronco1 is no longer required.
 *
 *  84   mpeg      1.83        3/28/2003 4:24:12 PM   Tim White       SCR(s)
 *       5909 :
 *       Replace CRYSTAL_FREQUENCY #define with xtal_frequency global.
 *
 *  83   mpeg      1.82        3/28/2003 8:11:10 AM   Craig Dry       SCR(s)
 *       5873 :
 *       Replaced strncpy with memcpy.
 *  82   mpeg      1.81        3/25/2003 6:54:34 PM   Craig Dry       SCR(s)
 *       5873 :
 *       Added procedure to populate C structure of type CONFIG_TABLE with
 *       configuration values found in pawser memory.
 *  81   mpeg      1.80        2/13/2003 12:20:52 PM  Matt Korte      SCR(s)
 *       5479 :
 *       Removed old header reference
 *  80   mpeg      1.79        1/23/2003 4:29:18 PM   Tim White       SCR(s)
 *       5298 :
 *       Added RAMWidth global variable which indicates the width of the memory
 *       bus.  Call
 *       new function GetRAMWidth() to obtain this value.  This value is used
 *       in the
 *       GetRAMSize() calculation.
 *
 *  79   mpeg      1.78        1/23/2003 2:14:24 PM   Tim White       SCR(s)
 *       5294 :
 *       MEM PLL is 25bits, not 24, so divide by 0x2000000 not 0x1000000.
 *
 *  78   mpeg      1.77        1/22/2003 10:44:16 AM  Tim White       SCR(s)
 *       5099 :
 *       Modified the MemClkPeriod setting code to read the PLL's correctly and
 *       remove the
 *       use of the RMO()'ed CNXT_GET_VAL() macros.
 *
 *  77   mpeg      1.76        12/20/2002 1:59:56 PM  Dave Wilson     SCR(s)
 *       5096 :
 *       For emulation only, hardwire MemClkPeriod to 8.0. Existing code
 *       evaluates
 *       to 0 and causes a C runtime floating point divide by 0 error.
 *  76   mpeg      1.75        12/16/2002 4:17:34 PM  Tim White       SCR(s)
 *       5169 :
 *       Allow future chips to use Wabash code by default instead of the
 *       Colorado code.
 *       Also, read the prescale from the chip instead of using a hardcoded
 *       value.
 *
 *  75   mpeg      1.74        12/16/2002 2:20:02 PM  Dave Wilson     SCR(s)
 *       5080 :
 *       Changes to support new general GPIO definition model for front panel
 *       LEDs.
 *  74   mpeg      1.73        12/12/2002 5:30:08 PM  Tim White       SCR(s)
 *       5157 :
 *       Use != HSX_ARBITER_NONE instead of == HSX_ARBITER_WABASH.  Fixed
 *       reading of MEM_PLL
 *       for MemClkPeriod for use with Brazos (and beyond).  Force IR diode to
 *       be off unconditionally.
 *
 *  73   mpeg      1.72        10/30/2002 2:47:06 PM  Bobby Bradford  SCR(s)
 *       4866 :
 *       Added support for BURNET front panel buttons (single GPIO)
 *  72   mpeg      1.71        10/11/2002 12:18:44 PM Miles Bintz     SCR(s)
 *       4431 :
 *       added check for HSX arbiter and gave priority to Soft Parser 0.  This
 *       works around a problem where the parser is being starved from the HSX
 *       bus while the demo is running.
 *
 *  71   mpeg      1.70        9/17/2002 5:39:14 PM   Carroll Vance   SCR(s)
 *       4613 :
 *       InitPCI() will always be called for wabash boxes, even when in I/O
 *       mode, because it has an internal PCI bus.
 *  70   mpeg      1.69        9/4/2002 4:23:22 PM    Carroll Vance   SCR(s)
 *       4524 :
 *       Turning off IR out diodes for Colorado and Wabash boxes.
 *  69   mpeg      1.68        9/4/2002 10:36:20 AM   Tim White       SCR(s)
 *       4517 :
 *       Back out changes made by DCS #4503.
 *
 *  68   mpeg      1.67        8/30/2002 8:01:32 PM   Carroll Vance   SCR(s)
 *       4503 :
 *
 *  67   mpeg      1.66        8/15/2002 6:48:16 PM   Tim Ross        SCR(s)
 *       4409 :
 *       Moved ISCOLORADOREVC, ISHONDO, and ISWABASH macros t respective chip
 *       header files.
 *  66   mpeg      1.65        7/16/2002 8:03:08 PM   Matt Korte      SCR(s)
 *       4215 :
 *       Clean up warnings.
 *
 *  65   mpeg      1.64        7/12/2002 8:23:50 AM   Steven Jones    SCR(s):
 *       4176
 *       Startup for Brady.
 *  64   mpeg      1.63        6/26/2002 1:52:48 PM   Tim White       SCR(s)
 *       4095 :
 *       ConfigATA changed to cnxt_ata_config.
 *
 *  63   mpeg      1.62        5/21/2002 6:16:20 PM   Joe Kroesche    SCR(s)
 *       3846 :
 *       removed include of math.h and added prototype for ceil() which is the
 *       math lib function being used in this file
 *  62   mpeg      1.61        5/7/2002 4:18:02 PM    Dave Moore      SCR(s)
 *       3724 :
 *       Only call SetupRomMemoryRegions for ARM940
 *
 *  61   mpeg      1.60        5/1/2002 3:28:00 PM    Tim White       SCR(s)
 *       3673 :
 *       Remove PLL_ bitfields usage from codebase.
 *
 *  60   mpeg      1.59        4/30/2002 1:16:30 PM   Billy Jackman   SCR(s)
 *       3656 :
 *       Changed use of STANDALONE from #if (STANDALONE == 0/1) to #if(n)def
 *       STANDALONE.
 *       Removed unused variable pConfig0.  Made declaration of variables i and
 *       str
 *       conditional the same as the code using them.
 *  59   mpeg      1.58        4/25/2002 1:02:06 AM   Tim Ross        SCR(s)
 *       3620 :
 *       Removed PCI bus scan workaround for cable-modem device internal to
 *       Wabash.
 *  58   mpeg      1.57        4/24/2002 4:24:02 PM   Dave Moore      SCR(s)
 *       3587 :
 *       Removed Haifa code from startup
 *
 *  57   mpeg      1.56        4/23/2002 11:38:02 AM  Dave Moore      SCR(s)
 *       3587 :
 *       Temporary work-around for PCI bus scan on WaBASH
 *
 *  56   mpeg      1.55        4/10/2002 5:11:24 PM   Tim White       SCR(s)
 *       3509 :
 *       Eradicate external bus (PCI) bitfield usage.
 *
 *  55   mpeg      1.54        4/5/2002 2:09:02 PM    Tim White       SCR(s)
 *       3510 :
 *       Backout DCS #3468
 *
 *  54   mpeg      1.53        4/3/2002 1:27:48 AM    Carroll Vance   SCR(s)
 *       3492 :
 *       SCR 3492
 *  53   mpeg      1.52        3/28/2002 2:47:20 PM   Quillian Rutherford
 *       SCR(s) 3468 :
 *       Removed bit fields
 *
 *  52   mpeg      1.51        2/14/2002 1:52:24 PM   Tim White       SCR(s)
 *       3183 :
 *       CRYSTAL_FREQUENCY has to be cast to a double for the calculation.
 *
 *  51   mpeg      1.50        2/8/2002 11:24:08 AM   Miles Bintz     SCR(s)
 *       3153 :
 *       removed hardcode 14.31818 frequency and substituted CRYSTAL_FREQUENCY
 *       / 1000000 as defined
 *       in config file or hwconfig.*
 *
 *  50   mpeg      1.49        1/31/2002 12:48:34 PM  Bobby Bradford  SCR(s)
 *       3107 :
 *       Modified the calculation of the memory clock period ... refer to
 *       tracker #3107 note(s) for more details
 *  49   mpeg      1.48        1/18/2002 5:29:36 PM   Tim White       SCR(s)
 *       3059 :
 *       Add wabash specific path for calculating MemClock period.
 *
 *  48   mpeg      1.47        11/29/2001 4:48:46 PM  Miles Bintz     SCR(s)
 *       2936 :
 *       Changed cnxtpci.h dependency to startup\startuppci.h
 *  47   mpeg      1.46        11/29/2001 2:26:48 PM  Matt Korte      SCR(s)
 *       2937 :
 *       Set bit 3 of 0x30000020 for Colorado Rev D1
 *       so that arbitration between GXA and Pawser on
 *       hsx bus happens.
 *
 *  46   mpeg      1.45        11/20/2001 3:32:36 PM  Quillian Rutherford
 *       SCR(s) 1754 :
 *       Fixed a typo
 *  45   mpeg      1.44        11/20/2001 11:33:04 AM Quillian Rutherford
 *       SCR(s) 2754 :
 *       Updated the ISA card initialization
 *  44   mpeg      1.43        8/22/2001 5:20:44 PM   Miles Bintz     SCR(s)
 *       2526 :
 *       To make the startup code independent of the KAL, HWLIB, and OSes,
 *       certain initialization calls were #ifdef'ed out.  If #STANDALONE == 1
 *       entry() is called.
 *  43   mpeg      1.42        7/5/2001 11:26:50 AM   Tim White       SCR(s)
 *       2178 2179 2180 :
 *       Hondo Merge
 *
 *  42   mpeg      1.41        7/3/2001 11:07:12 AM   Tim White       SCR(s)
 *       2178 2179 2180 :
 *       Merge branched Hondo specific code back into the mainstream source
 *       database.
 *
 *  41   mpeg      1.40        6/12/2001 4:47:52 PM   Dave Moore      SCR(s)
 *       2082 :
 *       Check for PCI stapping, call InitPCI if reqd.
 *
 *  40   mpeg      1.39        4/12/2001 4:46:18 PM   Amy Pratt       DCS914
 *       Removed support for Neches.
 *  39   mpeg      1.38        3/21/2001 12:39:12 PM  Dave Wilson     DCS1398:
 *       Merge Vendor D source file changes
 *  38   mpeg      1.37        2/15/2001 4:46:40 PM   Tim White       DCS#1230:
 *       Globalize MemClkPeriod for use by the Soft Modem code.
 *  37   mpeg      1.36        1/31/2001 2:30:30 PM   Tim White       DCS#0988:
 *       Reclaim footprint space.
 *  36   mpeg      1.35        1/29/2001 2:46:00 PM   Tim White       Changed
 *       pci.h to cnxtpci.h.
 *  35   mpeg      1.34        10/30/2000 5:35:02 PM  Joe Kroesche    made
 *       primary descriptor setup and hold=0 if secondary descriptor is used
 *  34   mpeg      1.33        9/27/2000 4:17:16 PM   Tim White       Allow PLL
 *       to be modified for customer hack assuming the PLL's are locked.
 *  33   mpeg      1.32        9/27/2000 3:38:02 PM   Dave Wilson     Added
 *       code to turn on vendor D's power LED (and, hence, their encoder)
 *  32   mpeg      1.31        9/27/2000 1:50:04 PM   Dave Wilson     Removed
 *       yesterdays vendor D "fix". It turns out that there was a wiring
 *       problem on the board and the fix was merely a workaround that
 *       prevented
 *       the ON/Standby LED from being used. Code is now correct and board will
 *       be
 *       reworked.
 *  31   mpeg      1.30        9/26/2000 6:09:12 PM   Tim White       Added
 *       switchable secondary descriptor capability to low-level flash
 *       function.
 *  30   mpeg      1.29        9/26/2000 4:54:24 PM   Tim White       More
 *       Vendor_D changes...
 *  29   mpeg      1.28        9/26/2000 4:29:38 PM   Tim White       Added
 *       Vendor_D setting to allow PIO76 to function as an output GPIO.
 *  28   mpeg      1.27        9/11/2000 12:01:28 PM  Ismail Mustafa  Removed
 *       VENDOR_B specific code that was labelled temp code and was
 *       modifying the MEM_DELAY register for eithernet support.
 *  27   mpeg      1.26        8/10/2000 8:02:16 PM   Tim Ross        Rolled in
 *       VENDOR_B changes.
 *  26   mpeg      1.25        7/18/2000 6:05:46 PM   Tim White       Merge in
 *       PACE changes to ISA tables.
 *  25   mpeg      1.24        6/12/2000 9:28:10 AM   Ray Mack        fixed
 *       warning caused by order dependency of include files
 *  24   mpeg      1.23        6/11/2000 5:33:28 PM   Ray Mack        changes
 *       to eliminate compiler warnings
 *  23   mpeg      1.22        6/9/2000 12:48:06 PM   Tim White       Fixed
 *       board ID.
 *  22   mpeg      1.21        5/2/2000 8:50:28 PM    Ray Mack        Take out
 *       PCI stuff for Colorado
 *  21   mpeg      1.20        5/1/2000 11:10:04 PM   Ray Mack        Changes
 *       so VxWorks will link
 *  20   mpeg      1.19        5/1/2000 7:09:50 PM    Senthil Veluswamy Changed
 *       MemClkPeriod32 to extern. Original declaration moved to Setuprom.c
 *  19   mpeg      1.18        4/27/2000 4:20:18 PM   Tim White       Fixed bug
 *       in board definitions.
 *  18   mpeg      1.17        4/24/2000 11:21:14 AM  Tim White       Moved
 *       strapping CONFIG0 from chip header file(s) to board header file(s).
 *  17   mpeg      1.16        4/12/2000 3:52:50 PM   Dave Wilson     Changed
 *       PLL_CONFIG0 register accesses to use new union type
 *  16   mpeg      1.15        4/10/2000 6:21:36 PM   Tim White       Added the
 *       correct Write (program) values, added several new Flash ROM types.
 *  15   mpeg      1.14        4/7/2000 3:08:44 PM    Dave Wilson     Changes
 *       to allow STARTUP to work on Klondike boards
 *  14   mpeg      1.13        3/22/2000 4:27:50 PM   Tim White       Removed
 *       old IOCHRDY bits and replaced with ExtWait:2 which is valid for both
 *       Colorado and all Neches (A&B) chips according to the specs.
 *  13   mpeg      1.12        3/8/2000 5:20:14 PM    Tim White
 *       Restructured the BANK_INFO array and added support for new Intel Flash
 *       ROM.
 *  12   mpeg      1.11        3/2/2000 5:34:20 PM    Tim White       Added
 *       ChipRevID global variable which is used along with ChipID in the
 *       chip/rev
 *       new macros in startup.h.  Also added the InitFlashInfo() function so
 *       that the
 *       flash subsystem is setup from startupc.c right after SetupROMs() is
 *       complete.
 *       This sets up the per bank memory regions in the CP_15 Memory
 *       Protection Unit
 *       to allow separate cache control per ROM bank.
 *  11   mpeg      1.10        1/5/2000 6:10:06 PM    Tim Ross        Enabled
 *       ISA descriptor timing calculations.
 *  10   mpeg      1.9         12/8/1999 5:12:48 PM   Tim Ross        Added
 *       changes for Colorado.
 *  9    mpeg      1.8         11/22/1999 10:32:44 AM Tim White       Added ATA
 *       startup call if ATA subsystem is included in build which
 *       is true only for the Delayed Viewing feature at this point.
 *  8    mpeg      1.7         10/28/1999 12:21:26 PM Dave Wilson     Removed
 *       GetChipID function which is now found in API.S
 *  7    mpeg      1.6         10/19/1999 3:30:10 PM  Tim Ross        Removed
 *       SABINE and PID board dependencies.
 *       Made compatible with CN8600 CFG file.
 *  6    mpeg      1.5         9/30/1999 5:33:48 PM   Tim Ross        made file
 *       conditional compile for Non-ARM boards
 *  5    mpeg      1.4         9/30/1999 5:28:32 PM   Senthil Veluswamy No
 *       change.
 *  4    mpeg      1.3         9/9/1999 11:24:48 AM   Tim Ross        Removed
 *       return value from SetupROMs().
 *  3    mpeg      1.2         7/2/1999 7:32:56 PM    Tim Ross        Updated
 *       starting point of all checkpoint values for  'C' code to begin
 *       at 0x10.
 *  2    mpeg      1.1         7/1/1999 1:05:14 PM    Tim Ross        Added
 *       GetRamSize() and GetChipID calls.
 *  1    mpeg      1.0         6/8/1999 12:53:10 PM   Tim Ross
 * $
 ****************************************************************************/

