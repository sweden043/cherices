/****************************************************************************/
/*                   CONEXANT PROPRIETARY AND CONFIDENTIAL                  */
/*                       SOFTWARE FILE/MODULE HEADER                        */
/*          Conexant Systems Inc. (c) 1998, 1999, 2000, 2001, 2002          */
/*                              Austin, TX                                  */
/*                         All Rights Reserved                              */
/****************************************************************************/
/*
 * Filename:        micro.c
 *
 *
 * Description:     Video Microcode Load Function
 *
 *
 * Author:          Dave Wilson
 *
 ****************************************************************************/
/* $Header: micro.c, 49, 9/2/03 7:28:08 PM, Joe Kroesche$
 ****************************************************************************/

#include "stbcfg.h"
#include "basetype.h"
#include "kal.h"
#include "globals.h"
#include "startup.h"

extern u_int32 gDemuxInstance;

extern void ResetVCore(void);

#if VIDEO_UCODE_DOWNLOAD_TYPE == VIDEO_UCODE_DOWNLOAD_BRAZOS
/* On Brazos, video microcode is accessed using a 32-bit wide register. On previous */
/* chips, however, the register is only 16 bits wide.                               */
typedef u_int32 *PVIDEO_MICROCODE;
#else
typedef u_int16 *PVIDEO_MICROCODE;
#endif

bool LoadMicrocode(bool Video, void *pStart, void *pEnd)
{
    u_int32 save, use, ret = TRUE, count = 0;
    PVIDEO_MICROCODE pStrt = pStart;
    u_int16 *pStrtAudio = (u_int16 *) pStart;
    LPREG pReg = (u_int32 *) glpMCWrite;
#if AUDIO_MICROCODE == UCODE_BRAZOS
    u_int32 *pStrtAOC = pStart;
#endif

    /* We can't use a critical section here since the function calls trace */
    /* and since this may take a long time!                                */
    
    save = * ((LPREG) glpCtrl0);
    use = save;

    if (Video)
    {
        #ifdef DEBUG
		static u_int8 i = 0;  /* Vid ucode load counter */
		u_int32 ucode_size = (u_int32) pEnd - (u_int32) pStart;
        ucode_size >>= 2;

	/* You will never see the trace below unless loading a subsequent n+1
     * version of video ucode and if DEBUG is defined.
     */
 		++i;   /* Always inc Vid ucode load counter */
	 	if (save & MPG_VIDEO_MC_VALID)  
     	trace_new(TRACE_MPG|TRACE_LEVEL_ALWAYS,"\nToggling Video Ucode Valid Bit on ucode load #%d.\n",i);
       	 						   

        /* Note: As of the moment (12/6/01) all current and future microcode is
         * expected to be smaller than 1472 << 2, hence the 1472 number
         */
        trace_new( TRACE_MPG|TRACE_LEVEL_2, "Loading the Video Microcode...\n");
        if ( ucode_size > VIDEO_UCODE_RAM_SIZE_WORDS)
        {
            trace_new( TRACE_MPG|TRACE_LEVEL_2, "VIDEO: ERROR. Microcode size too big for microcode memory buffer!\n");
        }
        #endif

/********************************************************************************************/
/* The Video ucode valid bit (bit 5 of MPG_CONTROL0_REG) is always low after reset 		    */
/* and before loading initial video ucode.  In order to load possible subsequent versions   */
/* of video ucode, it was necessary to toggle this bit low first.  Here we have provided    */
/* such a line of code. 																			*/
/********************************************************************************************/
    /* Ensure Valid bit is low for every video ucode load */
    save = * ((LPREG) glpCtrl0) & ~MPG_VIDEO_MC_VALID;
    
        * ((LPREG)glpCtrl0) = save | MPG_DOWN_LOAD_MC | MPG_DOWN_LOAD_VIDEO_MC;
        
        while (pStrt != pEnd)
        {
            /* NB: Note that pStrt type varies depending upon whether we are */
            /* downloading to Brazos or an earlier chip. Pointer increment   */
            /* arithmetic will be different in these 2 cases!                */
            *pReg = *pStrt++;
        }
    }
    else
    {       /* Audio */
#if AUDIO_MICROCODE == UCODE_BRAZOS
        if(!ISBRAZOSREVA && ((u_int32) pEnd - (u_int32) pStart == 4096))
        /* if Brazos but not Brazos rev A and audio and download num words is 4096 */
        /* indicating down load of AOC microcode */
        { 
           use &= ~(MPG_AOC_MC_VALID | MPG_AUDIO_MC_VALID | MPG_DOWN_LOAD_MC | MPG_DOWN_LOAD_AOC_MC | MPG_DOWN_LOAD_VIDEO_MC); 

	         /* Take away MC Valid */
	         *((LPREG)MPG_CONTROL0_REG) = use;  

	         /* Assert ucode download */
           *((LPREG)MPG_CONTROL0_REG) = use | MPG_DOWN_LOAD_MC | MPG_DOWN_LOAD_AOC_MC;  
           trace_new( TRACE_MPG|TRACE_LEVEL_2, "Loading Audio AOC Microcode %d %x %x %x\n", Video, pStart, pEnd, save);
 	         while (pStrtAOC != pEnd)
           {
             *pReg = *pStrtAOC++;
           }
           /* deassert MPG_DOWN_LOAD_MC but keep MPG_DOWN_LOAD_AOC_MC asserted in preparation for verify */
           use |= MPG_DOWN_LOAD_AOC_MC;
	         * ((LPREG)glpCtrl0) = use;  
  
           /* Assert download to reset aoc microcode RAM read/write address */
	         * ((LPREG)glpCtrl0) = use | MPG_DOWN_LOAD_MC;
  
           /* deassert MPG_DOWN_LOAD_MC in preparation for upload and verify */
	         * ((LPREG)glpCtrl0) = use;  
        }
        else {
#endif 

#if AUDIO_MICROCODE == UCODE_BRAZOS
      use &= ~(MPG_AUDIO_MC_VALID | MPG_DOWN_LOAD_MC | MPG_DOWN_LOAD_AOC_MC | MPG_DOWN_LOAD_VIDEO_MC); 
#else
      /* for chips which don't have RAM-based aoc ucode */
      use &= ~(MPG_AUDIO_MC_VALID | MPG_DOWN_LOAD_MC | MPG_DOWN_LOAD_VIDEO_MC); 
#endif 

      if(*((LPREG)MPG_CONTROL0_REG) & MPG_AUDIO_MC_VALID)
      {
			  *((LPREG)MPG_CONTROL1_REG) |= 0x400;
        while((count < 10) && (!(*((LPREG)MPG_DECODE_STATUS_REG) & 0x2000)) )
        {
          task_time_sleep(20);
          count++;
			  }
        if(!(*((LPREG)MPG_DECODE_STATUS_REG) & 0x2000))
        {
          trace_new(TRACE_LEVEL_ALWAYS,"AUDIO: Download failure.  Reseting the VCORE! 0x%x\n",*((LPREG)MPG_DECODE_STATUS_REG));
          ResetVCore();
        }
      }
  
	    /* Take away MC Valid */
	    *((LPREG)MPG_CONTROL0_REG) = use;  
  
	    /* Assert ucode download */
      *((LPREG)MPG_CONTROL0_REG) = use | MPG_DOWN_LOAD_MC;  
      trace_new( TRACE_MPG|TRACE_LEVEL_2, "Loading Audio Microcode %d %x %x %x\n", Video, pStart, pEnd, save);
 	    while (pStrtAudio != pEnd)
      {
        *pReg = *pStrtAudio++;
      }
  
      /* Invalid for download */
	    * ((LPREG)glpCtrl0) = use;  
  
      /* Assert download */
	    * ((LPREG)glpCtrl0) = use | MPG_DOWN_LOAD_MC;
  
      /* Invalid for download  */
	    * ((LPREG)glpCtrl0) = use;  

#if AUDIO_MICROCODE == UCODE_BRAZOS
      }
#endif 
    }
    
    /* Read back the micro code and verify it. */
    
    /* The following 2 lines are needed for a hardware race condition per Mark Baur */
    if (Video)
    {
      save &=  ~(MPG_VIDEO_MC_VALID | MPG_DOWN_LOAD_VIDEO_MC | MPG_DOWN_LOAD_MC);
      * ((LPREG)glpCtrl0) = save | MPG_VIDEO_MC_VALID;
      * ((LPREG)glpCtrl0) = save | MPG_DOWN_LOAD_MC | MPG_DOWN_LOAD_VIDEO_MC;
      * ((LPREG)glpCtrl0) = save | MPG_DOWN_LOAD_VIDEO_MC;
      pStrt = (PVIDEO_MICROCODE) pStart;
      while (pStrt != pEnd)
      {
          /* NB: Note that pStrt type varies depending upon whether we are */
          /* downloading to Brazos or an earlier chip. Pointer increment   */
          /* arithmetic will be different in these 2 cases!                */
          if ( *pReg != *pStrt++)
          {
              ret = FALSE;
              trace_new( TRACE_MPG|TRACE_LEVEL_2, "Error:Video Microcode did not compare!\n");
          }
      }
    }
    else
    {       /* Audio */
#if AUDIO_MICROCODE == UCODE_BRAZOS
      if(!ISBRAZOSREVA && ((u_int32) pEnd - (u_int32) pStart == 4096))
      /* if Brazos but not Rev A then verify AOC ucode */
      {
         trace_new( TRACE_MPG|TRACE_LEVEL_2, "Verifying the Audio AOC Microcode...\n");
         pStrtAOC = pStart;
         while (pStrtAOC != pEnd)
         {
            if (*pReg != *pStrtAOC++)
            {
               ret = FALSE;
               trace_new( TRACE_MPG|TRACE_LEVEL_2, "Error:Audio AOC Microcode did not compare!\n");
            }
         }
      }
      else {
#endif

      trace_new( TRACE_MPG|TRACE_LEVEL_2, "Verifying the Audio Microcode...\n");
      pStrtAudio = (u_int16 *) pStart;
      while (pStrtAudio != pEnd)
      {
         if (*pReg != *pStrtAudio++)
         {
             ret = FALSE;
             trace_new( TRACE_MPG|TRACE_LEVEL_2, "Error:Audio Microcode did not compare!\n");
         }
      }
#if AUDIO_MICROCODE == UCODE_BRAZOS
      }
#endif 
    }

    /* Now set the microcode valid bit */
    if (Video)
    {
        * ((LPREG)glpCtrl0) = save | MPG_DOWN_LOAD_MC | MPG_DOWN_LOAD_VIDEO_MC;
        * ((LPREG)glpCtrl0) = save | MPG_DOWN_LOAD_VIDEO_MC;
        * ((LPREG)glpCtrl0) = save | MPG_VIDEO_MC_VALID;
    
    }
    else
    {
#if AUDIO_MICROCODE == UCODE_BRAZOS
      if(!ISBRAZOSREVA && ((u_int32) pEnd - (u_int32) pStart == 4096))
      /* if Brazos but not Rev A then assert ucode valid for AOC */
      {
         /* Assert download */
    	    *((LPREG)glpCtrl0) = use | MPG_DOWN_LOAD_MC;
      
         /* Assert aoc mc is valid */
    	    * ((LPREG)glpCtrl0) = use | MPG_AOC_MC_VALID;  
	    }
      else {
#endif

      /* Assert download */
    	*((LPREG)glpCtrl0) = use | MPG_DOWN_LOAD_MC;
      
      /* Deassert CTRL1 disc bit */ 
    	*((LPREG)glpCtrl1) &= ~0x400;
       
      /* Assert audio mc is valid */
    	* ((LPREG)glpCtrl0) = use | MPG_AUDIO_MC_VALID;  
#if AUDIO_MICROCODE == UCODE_BRAZOS
      }
#endif 
	  }

#ifdef DEBUG
   if (ret)
   {
        trace_new( TRACE_MPG|TRACE_LEVEL_2, "Load of Microcode successful!\n");
   }     
   else
   {
        trace_new( TRACE_MPG|TRACE_LEVEL_2, "Load of Microcode failed!\n");
   }     
#endif
    return ret;

}

void WaitOnCtlr()
{
    while (CNXT_GET_VAL(glpDecoderStatusReg, MPG_DECODE_STATUS_CMDVALID_MASK))
        ;
}

/*******************************************************************/
/* Function: InitMPEGVideo                                         */
/* Parameters: void                                                */
/* Return: void                                                    */
/* Remarks:  Initializes the I,P & B buffer addresses for the MPEG */
/*           Video Decoder.                                        */
/*******************************************************************/
void InitMPEGVideo()
{
    bool ks;

        trace_new(TRACE_MPG|TRACE_LEVEL_ALWAYS,"InitMPEGVideo: WaitOnCtrl\n");
    WaitOnCtlr();
        trace_new(TRACE_MPG|TRACE_LEVEL_ALWAYS,"InitMPEGVideo: Sending InitIPB\n");

    ks = critical_section_begin();
    *glpMpgCmdReg = (u_int32) MPG_COMMAND_INITI;
    WaitOnCtlr();
    *glpMpgCmdReg = (u_int32) MPG_PIC_ADDR_BIT | (HWBUF_DEC_I_ADDR & 0xFFF);
    WaitOnCtlr();
    *glpMpgCmdReg = (u_int32) MPG_PIC_ADDR_BIT | ( (HWBUF_DEC_I_ADDR >> 12) & 0xFFF );
    WaitOnCtlr();

    *glpMpgCmdReg = (u_int32) MPG_COMMAND_INITP;
    WaitOnCtlr();
    *glpMpgCmdReg = (u_int32) MPG_PIC_ADDR_BIT | (HWBUF_DEC_P_ADDR & 0xFFF);
    WaitOnCtlr();
    *glpMpgCmdReg = (u_int32) MPG_PIC_ADDR_BIT | ( (HWBUF_DEC_P_ADDR >> 12) & 0xFFF );
    WaitOnCtlr();

    *glpMpgCmdReg = (u_int32) MPG_COMMAND_INITB;
    WaitOnCtlr();
    *glpMpgCmdReg = (u_int32) MPG_PIC_ADDR_BIT | (HWBUF_DEC_B_ADDR & 0xFFF);
    WaitOnCtlr();
    *glpMpgCmdReg = (u_int32) MPG_PIC_ADDR_BIT | ( (HWBUF_DEC_B_ADDR >> 12) & 0xFFF );
    WaitOnCtlr();

    CNXT_SET_VAL(glpCtrl0, MPG_CONTROL0_DISABLEPICUSERDATA_MASK, 0);
    CNXT_SET_VAL(glpCtrl0, MPG_CONTROL0_DISABLEGOPUSERDATA_MASK, 0);
    CNXT_SET_VAL(glpCtrl0, MPG_CONTROL0_DISABLESEQUSERDATA_MASK, 0);

    *DPS_USER_START_ADDR_EX(gDemuxInstance) = HWBUF_USRDAT_ADDR;
    *DPS_AUD_ANC_START_ADDR_EX(gDemuxInstance) = HWBUF_AUDANC_ADDR;
    *DPS_AUD_ANC_END_ADDR_EX(gDemuxInstance) = (HWBUF_DEC_I_ADDR - 1);
    *DPS_AUD_ANC_WRITE_PTR_EX(gDemuxInstance) = *DPS_AUD_ANC_START_ADDR_EX(gDemuxInstance);
    *DPS_AUD_ANC_READ_PTR_EX(gDemuxInstance) = *DPS_AUD_ANC_START_ADDR_EX(gDemuxInstance);
    *DPS_USER_READ_PTR_EX(gDemuxInstance) = *DPS_USER_START_ADDR_EX(gDemuxInstance);        
    *DPS_USER_WRITE_PTR_EX(gDemuxInstance) = *DPS_USER_START_ADDR_EX(gDemuxInstance);        

    CNXT_SET_VAL(glpCtrl0, MPG_CONTROL0_ENABLEINT_MASK, 1);
    CNXT_SET_VAL(glpCtrl0, MPG_CONTROL0_FULLB_MASK, 1);
 #ifndef WABASH_AVSYNC_CLKREC_DISABLED
    CNXT_SET_VAL(glpCtrl0, MPG_CONTROL0_ENABLESYNC_MASK, 1);
 #endif
    CNXT_SET_VAL(glpCtrl1, MPG_CONTROL1_ENCVIDEOENABLE_MASK, 1);

    critical_section_end(ks);
}


void VideoReInitHW()
{
    bool ks;
    u_int32 save;
    /*
       The following 2 lines are needed for a hardware race condition
       per Mark Baur
    */
    ks = critical_section_begin();
    save = * ((LPREG)glpCtrl0);
    * ((LPREG)glpCtrl0) = 0;
    * ((LPREG)glpCtrl0) =  save;

    * ((LPREG)glpCtrl0) &=  ~MPG_VIDEO_MC_VALID;
    * ((LPREG)glpCtrl0) |=   MPG_DOWN_LOAD_VIDEO_MC;
    save = * ((LPREG)glpCtrl0); // delay
    * ((LPREG)glpCtrl0) &=  ~MPG_DOWN_LOAD_VIDEO_MC;
    * ((LPREG)glpCtrl0) |=  MPG_VIDEO_MC_VALID;
    critical_section_end(ks);

    InitMPEGVideo();
}

/****************************************************************************
 * Modifications:
 * $Log: 
 *  49   mpeg      1.48        9/2/03 7:28:08 PM      Joe Kroesche    SCR(s) 
 *        7415 :
 *        deleted unneeded header file to eliminate warnings when building for 
 *        psos
 *        
 *  48   mpeg      1.47        6/4/03 6:35:44 PM      Dave Aerne      SCR(s) 
 *        6727 :
 *        needed to deassert audio decoder ucode valid in addition to aoc ucode
 *         valid
 *        to enable re-download of aoc ucode w/o power on reset sequence.
 *        
 *  47   mpeg      1.46        5/19/03 4:56:48 PM     Dave Aerne      SCR(s) 
 *        6456 6457 :
 *        added ifdef around brazos-specific variable for non-brazos builds.
 *        
 *  46   mpeg      1.45        5/19/03 2:42:30 PM     Dave Aerne      SCR(s) 
 *        6434 6435 :
 *        per Joe's note added delay to download and upload.
 *        Also added necessary control bit settings for proper AOC microcode 
 *        operation.
 *        
 *  45   mpeg      1.44        5/15/03 5:22:24 PM     Dave Aerne      SCR(s) 
 *        6261 6291 :
 *        added support to download and upload(verify) aoc ucode for Brazos Rev
 *         B 
 *        and later revs
 *        
 *  44   mpeg      1.43        4/29/03 8:31:14 PM     Dave Aerne      SCR(s) 
 *        6127 :
 *        added code to init audio ancillary read and write pointers when audio
 *         
 *        ancillary buffer is defined.
 *        
 *  43   mpeg      1.42        2/26/03 1:26:58 PM     Billy Jackman   SCR(s) 
 *        5594 :
 *        Modified comment format to avoid compiler warning about embedded 
 *        comment
 *        delimiter.
 *        
 *  42   mpeg      1.41        2/24/03 1:54:12 PM     Mark Thissen    SCR(s) 
 *        5591 :
 *        Added code to toggle Video Ucode Valid bit to low for subsequent 
 *        ucode loads.
 *        
 *  41   mpeg      1.40        12/10/02 3:02:00 PM    Dave Wilson     SCR(s) 
 *        5071 :
 *        For Brazos, downloads must be done 32 bits at a time rather than 16. 
 *        Code
 *        updated to handle this (though, obviously, I have not yet been able 
 *        to test
 *        it!). Also moved the size of the video microcode RAM into the 
 *        relevant chip
 *        header.
 *        
 *  40   mpeg      1.39        6/10/02 3:07:06 PM     Craig Dry       SCR(s) 
 *        3923 :
 *        Remove MPG bitfields unconditionally - Step 2
 *        
 *  39   mpeg      1.38        6/6/02 5:50:50 PM      Craig Dry       SCR(s) 
 *        3923 :
 *        Remove MPG bitfields conditionally - Step 1
 *        
 *  38   mpeg      1.37        5/22/02 4:49:30 PM     Miles Bintz     SCR(s) 
 *        3854 :
 *        Added another vmc_valid !vmc_valud vmc_valid sequence to work around 
 *        microcodehang
 *        
 *  37   mpeg      1.36        5/13/02 1:48:20 PM     Tim White       SCR(s) 
 *        3760 :
 *        DPS_ HSDP definitions are now HSDP_, remove reliance on legacy DPS 
 *        globals.
 *        
 *        
 *  36   mpeg      1.35        4/5/02 11:39:30 AM     Tim White       SCR(s) 
 *        3510 :
 *        Backout DCS #3468
 *        
 *        
 *  35   mpeg      1.34        3/28/02 2:11:18 PM     Quillian Rutherford 
 *        SCR(s) 3468 :
 *        Removed bit fields
 *        
 *        
 *  34   mpeg      1.33        3/12/02 5:02:20 PM     Bob Van Gulick  SCR(s) 
 *        3359 :
 *        Add multi-instance demux support
 *        
 *        
 *  33   mpeg      1.32        1/31/02 9:04:18 AM     Tim White       SCR(s) 
 *        3106 :
 *        Disable AVSync for Wabash for the time being.
 *        
 *        
 *  32   mpeg      1.31        12/19/01 3:36:52 PM    Bob Van Gulick  SCR(s) 
 *        2977 :
 *        Backout multi-instance demux usage
 *        
 *        
 *  31   mpeg      1.30        12/18/01 5:10:04 PM    Quillian Rutherford 
 *        SCR(s) 2933 :
 *        Merge in wabash code
 *        
 *        
 *  30   mpeg      1.29        12/18/01 9:58:46 AM    Bob Van Gulick  SCR(s) 
 *        2977 :
 *        Changes to support new multi-instance demux driver
 *        
 *        
 *  29   mpeg      1.28        7/3/01 11:08:06 AM     Tim White       SCR(s) 
 *        2178 2179 2180 :
 *        Merge branched Hondo specific code back into the mainstream source 
 *        database.
 *        
 *        
 *  28   mpeg      1.27        6/26/01 6:27:40 PM     Quillian Rutherford 
 *        SCR(s) 2163 :
 *        This version of video/micro.c only uses ResetVCore in the case of not
 *         getting
 *        a handshake back from the core when downloading new audio microcode.
 *        
 *        
 *  27   mpeg      1.26        4/12/01 8:21:50 PM     Amy Pratt       DCS914 
 *        Removed Neches support.
 *        
 *  26   mpeg      1.25        4/11/01 2:06:36 PM     Dave Wilson     DCS1675: 
 *        Removed some critical sections that contained blocking calls. It's
 *        not a good idea to turn interrupts off then block on an OS object!
 *        
 *  25   mpeg      1.24        2/1/01 1:13:32 PM      Tim Ross        DCS966.
 *        Added support for Hondo.
 *        
 *  24   mpeg      1.23        1/22/01 1:55:30 PM     Matt Korte      DCS #985.
 *        Audio microcode is only 16 bits wide. Pack 2 of
 *        these into each 32 bits in order to halve the
 *        audio microcode size.
 *        
 *  23   mpeg      1.22        12/5/00 4:17:30 PM     Matt Korte      Fixed 
 *        LoadMicrocode so it is reuseable
 *        
 *  22   mpeg      1.21        10/10/00 9:41:50 AM    Ismail Mustafa  Add check
 *         for microcode size before loading.
 *        
 *  21   mpeg      1.20        4/20/00 11:34:20 AM    Ismail Mustafa  Removed 
 *        OpenTVX.h and added basetype.h.
 *        
 *  20   mpeg      1.19        4/18/00 4:14:42 PM     Ismail Mustafa  Fix for 
 *        VxWorks. Removed workaround code for the bitfield compiler bug.
 *        Compiler is now fixed.
 *        
 *  19   mpeg      1.18        4/10/00 6:08:22 PM     Ismail Mustafa  Added 
 *        ifdef around setting of Audio Ancillary pointers.
 *        
 *  18   mpeg      1.17        4/4/00 8:46:44 PM      Ismail Mustafa  Changed 
 *        to support User Data.
 *        
 *  17   mpeg      1.16        2/4/00 1:09:18 PM      Ismail Mustafa  Fixed bit
 *         fields problem.
 *        
 *  16   mpeg      1.15        1/3/00 9:31:38 AM      Ismail Mustafa  Removed 
 *        SABINE ifdefs.
 *        
 *  15   mpeg      1.14        11/9/99 11:06:26 AM    Ismail Mustafa  Moved 
 *        critical section calls here from video.c.
 *        
 *  14   mpeg      1.13        4/30/99 5:24:30 PM     Ismail Mustafa  Removed 
 *        the USER_DATA_TEST.
 *        
 *  13   mpeg      1.12        4/27/99 11:36:06 AM    Ismail Mustafa  Added 
 *        code for User Data Test.
 *        
 *  12   mpeg      1.11        4/19/99 11:40:30 AM    Ismail Mustafa  Changes 
 *        for Neches.
 *        
 *  11   mpeg      1.10        3/26/99 10:08:30 AM    Ismail Mustafa  Added 
 *        Neches changes.
 *        
 *  10   mpeg      1.9         1/29/99 9:24:14 AM     Ismail Mustafa  Fix 
 *        warnings.
 *        
 *  9    mpeg      1.8         12/22/98 4:19:04 PM    Ismail Mustafa  Changed 
 *        reset hw code.
 *        
 *  8    mpeg      1.7         9/29/98 3:26:56 PM     Ismail Mustafa  Set the 
 *        FullB in Control Reg 0.
 *        
 *  7    mpeg      1.6         8/5/98 3:55:24 PM      Ismail Mustafa  
 *        VideoHWReInit added.
 *        
 *  6    mpeg      1.5         6/22/98 5:45:20 PM     Ismail Mustafa  Added 
 *        some hard coded values for stills testing.
 *        
 *  5    mpeg      1.4         5/28/98 9:13:32 AM     Ismail Mustafa  Use 
 *        defines instead of numbers.
 *        
 *  4    mpeg      1.3         4/20/98 4:21:10 PM     Ismail Mustafa  Fix for 
 *        audio microcode loading.
 *        
 *  3    mpeg      1.2         4/20/98 12:13:52 PM    Ismail Mustafa  Added 
 *        read micro read back.
 *        
 *  2    mpeg      1.1         4/17/98 10:20:52 AM    Ismail Mustafa  Fixed the
 *         code for loading the microcode.
 *        
 *  1    mpeg      1.0         4/13/98 11:50:18 AM    Ismail Mustafa  
 * $
 * 
 *    Rev 1.48   02 Sep 2003 18:28:08   kroescjl
 * SCR(s) 7415 :
 * deleted unneeded header file to eliminate warnings when building for psos
 * 
 *    Rev 1.47   04 Jun 2003 17:35:44   aernedj
 * SCR(s) 6727 :
 * needed to deassert audio decoder ucode valid in addition to aoc ucode valid
 * to enable re-download of aoc ucode w/o power on reset sequence.
 * 
 *    Rev 1.46   19 May 2003 15:56:48   aernedj
 * SCR(s) 6456 6457 :
 * added ifdef around brazos-specific variable for non-brazos builds.
 * 
 *    Rev 1.45   19 May 2003 13:42:30   aernedj
 * SCR(s) 6434 6435 :
 * per Joe's note added delay to download and upload.
 * Also added necessary control bit settings for proper AOC microcode operation.
 * 
 *    Rev 1.44   15 May 2003 16:22:24   aernedj
 * SCR(s) 6261 6291 :
 * added support to download and upload(verify) aoc ucode for Brazos Rev B 
 * and later revs
 * 
 *    Rev 1.43   29 Apr 2003 19:31:14   aernedj
 * SCR(s) 6127 :
 * added code to init audio ancillary read and write pointers when audio 
 * ancillary buffer is defined.
 * 
 *    Rev 1.42   26 Feb 2003 13:26:58   jackmaw
 * SCR(s) 5594 :
 * Modified comment format to avoid compiler warning about embedded comment
 * delimiter.
 * 
 *    Rev 1.41   24 Feb 2003 13:54:12   thissem
 * SCR(s) 5591 :
 * Added code to toggle Video Ucode Valid bit to low for subsequent ucode loads.
 * 
 *    Rev 1.40   10 Dec 2002 15:02:00   dawilson
 * SCR(s) 5071 :
 * For Brazos, downloads must be done 32 bits at a time rather than 16. Code
 * updated to handle this (though, obviously, I have not yet been able to test
 * it!). Also moved the size of the video microcode RAM into the relevant chip
 * header.
 * 
 *    Rev 1.39   10 Jun 2002 14:07:06   dryd
 * SCR(s) 3923 :
 * Remove MPG bitfields unconditionally - Step 2
 * 
 *    Rev 1.38   06 Jun 2002 16:50:50   dryd
 * SCR(s) 3923 :
 * Remove MPG bitfields conditionally - Step 1
 * 
 *    Rev 1.37   22 May 2002 15:49:30   bintzmf
 * SCR(s) 3854 :
 * Added another vmc_valid !vmc_valud vmc_valid sequence to work around microcodehang
 * 
 *    Rev 1.36   13 May 2002 12:48:20   whiteth
 * SCR(s) 3760 :
 * DPS_ HSDP definitions are now HSDP_, remove reliance on legacy DPS globals.
 * 
 * 
 *    Rev 1.35   05 Apr 2002 11:39:30   whiteth
 * SCR(s) 3510 :
 * Backout DCS #3468
 * 
 * 
 *    Rev 1.33   12 Mar 2002 17:02:20   vangulr
 * SCR(s) 3359 :
 *  Add multi-instance demux support
 * 
 * 
 *    Rev 1.32   31 Jan 2002 09:04:18   whiteth
 * SCR(s) 3106 :
 * Disable AVSync for Wabash for the time being.
 * 
 * 
 *    Rev 1.31   19 Dec 2001 15:36:52   vangulr
 * SCR(s) 2977 :
 * Backout multi-instance demux usage
 * 
 * 
 *    Rev 1.30   18 Dec 2001 17:10:04   rutherq
 * SCR(s) 2933 :
 * Merge in wabash code
 * 
 * 
 *    Rev 1.29   18 Dec 2001 09:58:46   vangulr
 * SCR(s) 2977 :
 * Changes to support new multi-instance demux driver
 * 
 * 
 *    Rev 1.28.1.0   07 Dec 2001 14:24:42   rutherq
 * SCR(s) 2933 :
 * Removed reference to ISATLEASTCOLORADOREVC macro
 * 
 * 
 *    Rev 1.28   03 Jul 2001 10:08:06   whiteth
 * SCR(s) 2178 2179 2180 :
 * Merge branched Hondo specific code back into the mainstream source database.
 * 
 * 
 *    Rev 1.27   26 Jun 2001 17:27:40   rutherq
 * SCR(s) 2163 :
 * This version of video/micro.c only uses ResetVCore in the case of not getting
 * a handshake back from the core when downloading new audio microcode.
 * 
 *    Rev 1.26.1.0   22 May 2001 14:41:24   prattac
 * DCS1642 Removed references to DCS966 define.
 *
 *    Rev 1.26   12 Apr 2001 19:21:50   prattac
 * DCS914 Removed Neches support.
 * 
 *    Rev 1.25   11 Apr 2001 13:06:36   dawilson
 * DCS1675: Removed some critical sections that contained blocking calls. It's
 * not a good idea to turn interrupts off then block on an OS object!
 * 
 *    Rev 1.24   01 Feb 2001 13:13:32   rossst
 * DCS966.
 * Added support for Hondo.
 * 
 *    Rev 1.23   22 Jan 2001 13:55:30   kortemw
 * DCS #985.
 * Audio microcode is only 16 bits wide. Pack 2 of
 * these into each 32 bits in order to halve the
 * audio microcode size.
 * 
 *    Rev 1.22   05 Dec 2000 16:17:30   kortemw
 * Fixed LoadMicrocode so it is reuseable
 * 
 *    Rev 1.21   10 Oct 2000 08:41:50   mustafa
 * Add check for microcode size before loading.
 * 
 *    Rev 1.20   20 Apr 2000 10:34:20   mustafa
 * Removed OpenTVX.h and added basetype.h.
 *
 *    Rev 1.19   18 Apr 2000 15:14:42   mustafa
 * Fix for VxWorks. Removed workaround code for the bitfield compiler bug.
 * Compiler is now fixed.
 *
 *    Rev 1.18   10 Apr 2000 17:08:22   mustafa
 * Added ifdef around setting of Audio Ancillary pointers.
 *
 *    Rev 1.17   04 Apr 2000 19:46:44   mustafa
 * Changed to support User Data.
 *
 *    Rev 1.16   04 Feb 2000 13:09:18   mustafa
 * Fixed bit fields problem.
 *
 *    Rev 1.15   03 Jan 2000 09:31:38   mustafa
 * Removed SABINE ifdefs.
 *
 *    Rev 1.14   09 Nov 1999 11:06:26   mustafa
 * Moved critical section calls here from video.c.
 *
 *    Rev 1.13   30 Apr 1999 16:24:30   mustafa
 * Removed the USER_DATA_TEST.
 *
 *    Rev 1.12   27 Apr 1999 10:36:06   mustafa
 * Added code for User Data Test.
 *
 *    Rev 1.11   19 Apr 1999 10:40:30   mustafa
 * Changes for Neches.
 *
 *    Rev 1.10   26 Mar 1999 10:08:30   mustafa
 * Added Neches changes.
 *
 *    Rev 1.9   29 Jan 1999 09:24:14   mustafa
 * Fix warnings.
 *
 *    Rev 1.8   22 Dec 1998 16:19:04   mustafa
 * Changed reset hw code.
 *
 *    Rev 1.7   29 Sep 1998 14:26:56   mustafa
 * Set the FullB in Control Reg 0.
 *
 *    Rev 1.6   05 Aug 1998 14:55:24   mustafa
 * VideoHWReInit added.
 *
 *    Rev 1.5   22 Jun 1998 16:45:20   mustafa
 * Added some hard coded values for stills testing.
 *
 *    Rev 1.4   28 May 1998 08:13:32   mustafa
 * Use defines instead of numbers.
 *
 *    Rev 1.3   20 Apr 1998 15:21:10   mustafa
 * Fix for audio microcode loading.
 *
 *    Rev 1.2   20 Apr 1998 11:13:52   mustafa
 * Added read micro read back.
 *
 *    Rev 1.1   17 Apr 1998 09:20:52   mustafa
 * Fixed the code for loading the microcode.
 *
 *    Rev 1.0   13 Apr 1998 10:50:18   mustafa
 * Initial revision.
 *
 ****************************************************************************/

