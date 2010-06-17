/****************************************************************************/
/*                   CONEXANT PROPRIETARY AND CONFIDENTIAL                  */
/*                       SOFTWARE FILE/MODULE HEADER                        */
/*                      Conexant Systems Inc. (c) 2004                      */
/*                              Austin, TX                                  */
/*                         All Rights Reserved                              */
/****************************************************************************/
/*
 * Filename:        FLASHSAMNAND.C
 *
 *
 * Description:     Low level Samsung Nand flash memory access functions
 *
 *
 * Author:          Sunil Cheruvu
 *
 ****************************************************************************/
/* $ Header:$
 ****************************************************************************/

//
// WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING
// WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING
//
// 1. ABSOLUTELY DONOT erase invalid blocks
//
// WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING
// WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING
//

/*****************/
/* Include Files */
/*****************/
#include <string.h>
#include "stbcfg.h"
#include "basetype.h"
#ifndef DLOAD
#include "kal.h"
#endif
#ifdef DLOAD
#include "defs.h"
#endif
#include "startup.h"
#include "flashrw.h"
#include "ecc.h"
#include "retcodes.h"

#ifdef DLOAD
   #if DOWNLOAD_SERIAL_SUPPORT==YES
      #include "libfuncs.h"
   #else
      #include <stdio.h>
   #endif
#endif

#if !defined(DLOAD) && (RTOS != NOOS)
   timer_id_t sam_nand_timer;
   extern bool bKalUp;
   #define INDEX_FROM_TIMERID(x) ((x) & 0xFFFF)
   void samnand_timer_handler(timer_id_t timer, void *userData) { }
#endif

/**************************/
/* Flash Commands  */
/**************************/
#define FLASH_CMD_ID            	      0x90
#define FLASH_CMD_READ1A               0x00 /* A area */
#define FLASH_CMD_READ1B               0x01 /* B area, valid for x8 only */
#define FLASH_CMD_READ2_SPARE          0x50 /* Spare area */
#define FLASH_CMD_PAGE_PROG_SETUP      0x80
#define FLASH_CMD_PAGE_PROG            0x10
#define FLASH_CMD_ERASE_SETUP    0x60
#define FLASH_CMD_ERASE          0xD0
#define FLASH_CMD_STATUS        	      0x70
#define FLASH_CMD_RESET                0xFF
#define SEND_CMD(CMD) (*NANDCMD = CMD)
#define READ_DATA() (*NANDDATA)
#define WRITE_DATA(val) (*NANDDATA = val)
#define CRC_INIT_VALUE      0xFFFFFFFFL 
#define NAND_FLASH_SEEK_CURRENT  1
#define NAND_FLASH_SEEK_START    0
// dummy read to flush writes
#define DUMMY_READ { u_int32 gdummy; (gdummy = *(volatile u_int32*)0x30010000); }

/* Wait for erase to complete via Ready/Busy signal */
#define WAIT_FOR_RDY   \
while (!GET_GPIO_PIN_BANK(CNXT_NANDFLASH_READY_BUSY_GPIO/32, \
         CNXT_NANDFLASH_READY_BUSY_GPIO%32)) { }

#define SAM_NAND_UNIT_TEST 0

#define READ_START_BLOCK_ADDR    1
#define WRITE_START_BLOCK_ADDR   1
#define MAX_BAD_BLOCKS 50


u_int32 read_start_block_addr  =  READ_START_BLOCK_ADDR;
u_int32 write_start_block_addr =  WRITE_START_BLOCK_ADDR;
extern u_int32 CNXT_NANDFLASH_READY_BUSY_GPIO;
extern u_int32 CNXT_NANDFLASH_CHIP_ENABLE_GPIO;
extern u_int32 CNXT_NANDFLASH_CHIP_SELECT;


u_int32 sam_nand_crc32
(
   u_int8   *pBlock,    /* Pointer to the buffer to compute the crc from  */
   u_int32  nBlockLen,  /* size of the buffer  */
   u_int32  crc         /* crc from old block */
);


typedef enum
{
   NAND_FLASH_STATUS_SUCCESS = 0,
   NAND_FLASH_STATUS_PAGE_WRITE_ERROR,
   NAND_FLASH_STATUS_PAGE_READ_ERROR,
   NAND_FLASH_STATUS_BLOCK_WRITE_ERROR,
   NAND_FLASH_STATUS_BLOCK_READ_ERROR,
   NAND_FLASH_STATUS_BLOCK_ERASE_ERROR,
   NAND_FLASH_STATUS_READ_ID_ERROR,
   NAND_FLASH_STATUS_ECC_ERROR,
   NAND_FLASH_STATUS_BANK_ERASE_ERROR,
   NAND_FLASH_STATUS_UNCORRECTABLE_ERROR,
   NAND_FLASH_STATUS_ECC_CORRECTED
}NAND_FLASH_STATUS_TYPE;



/******************************************/
/* Local Definitions and Global Variables */
/******************************************/
extern BANK_INFO Bank[];
extern int CurrentBank;

extern int FastCopyBytes( const void *dst, const void *src, int count );
extern int FCopy( const void *dst, const void *src, int count );
extern void FFillBytes(void *dest, int value, size_t count);



/**************************/
/* Function Prototypes    */   
/**************************/
/* Write to the spare area */
int write_spare_area(u_int32 pageaddress, 
                     u_int32 columnaddress, 
                     void* pdata, 
                     u_int32 size);

int IsBlockValid(u_int32 block, bool* pbValidBlock);
void PrintBadBlockCatalog();
typedef struct
{
   u_int32 blockaddr;
   u_int32 pageaddr0;
   u_int8  val0;
   u_int32 pageaddr1;
   u_int8  val1;
}BAD_BLOCK_INFO_TYPE;

/* this is the max bad blocks array anticipated */
BAD_BLOCK_INFO_TYPE BadBlock[MAX_BAD_BLOCKS];

/* To track the actual number of bad blocks for cataloguing */
u_int32 bbcount = 0;

void EnableChip(void)
{
   SET_GPIO_PIN_BANK(CNXT_NANDFLASH_CHIP_ENABLE_GPIO/32, 
                     CNXT_NANDFLASH_CHIP_ENABLE_GPIO%32, 
                     NAND_FLASH_CHIP_ENABLE_VAL);
}
 
void DisableChip(void)
{
   SET_GPIO_PIN_BANK(CNXT_NANDFLASH_CHIP_ENABLE_GPIO/32, 
                     CNXT_NANDFLASH_CHIP_ENABLE_GPIO%32, 
                     NAND_FLASH_CHIP_DISABLE_VAL);
} 


void samnand_delay_usec(u_int32 timer_interval_in_usec)
{
   /* Timers tick at 54 Mhz, giving maximum 79.5sec count */
   u_int8 uTimer = 1; /* The real hw timer register number */
   volatile u_int32 *p;

#if ( defined(DLOAD) || (RTOS == NOOS) )

#define uSEC_1 ((u_int32)54)
#define mSEC_10 ((u_int32)( uSEC_1 * 10 * 1000 ))

   u_int32 timer_interval = timer_interval_in_usec * uSEC_1;

   /* Make sure Interrupt generation is off */
   p = (volatile u_int32*)(TIM_MODE_REG + (TIM_BANK_SIZE * uTimer));
   *p = 0x0;

   /* Clear Counter value to zero */
   p = (volatile u_int32*)(TIM_VALUE_REG + (TIM_BANK_SIZE * uTimer));
   *p = 0x0;

   /** Program Limit **/
   p = (volatile u_int32*)(TIM_LIMIT_REG + (TIM_BANK_SIZE * uTimer));
   *p = timer_interval; /* 10ms */

   /* Enable the timer */
   p = (volatile u_int32*)( TIM_MODE_REG + (TIM_BANK_SIZE * uTimer));
   *p = 0x3;  /* count past limit, enable timer */

   /* Now spin in a while loop until the time has elasped */
   while ( 1 )
   {
      /* get current counter value */
      p = (volatile u_int32*)(TIM_VALUE_REG + (TIM_BANK_SIZE * uTimer));
      if (  *p  >=  timer_interval )
      {
         /* Time elapsed */
         p = (volatile u_int32*)( TIM_MODE_REG + (TIM_BANK_SIZE * uTimer));
         *p = 0x0;  /* disable timer */
         break;  /* and go do our thing... */
      }
   }
#elif !defined(DLOAD) && (RTOS != NOOS)

   if (bKalUp)
   {
      /* Create hwtimer */
      sam_nand_timer = hwtimer_create(samnand_timer_handler, 0, NULL);
      if(!sam_nand_timer)
      {
         trace_new(TRACE_TST|TRACE_LEVEL_ALWAYS, "Failed to allocate hwtimer! for Samsung Nand Flash Driver\n");
         return;
      }

      /* Set timer to fire after timer_interval_in_usec us */
      hwtimer_set(sam_nand_timer, timer_interval_in_usec, TRUE);

      /* Start the timer */
      hwtimer_start(sam_nand_timer);

      /*    if(IS_VALID_TIMERID(sam_nand_timer)) */
      /*  Note: It is not necessary to do the above check */
      /* Has this timer been allocated ? */
      uTimer = INDEX_FROM_TIMERID(sam_nand_timer);
      /* Now spin in a while loop until the time has elasped */
      while ( 1 )
      {
         /* get current counter value */
         p = (volatile u_int32*)(TIM_VALUE_REG + (TIM_BANK_SIZE * uTimer));
         if (  *p  >=  timer_interval_in_usec )
         {
            /* Time elapsed */
            p = (volatile u_int32*)( TIM_MODE_REG  + (TIM_BANK_SIZE * uTimer) );
            *p = 0x0;  /* disable timer */
            break;  /* and go do our thing... */
         }
      }
      /* Cleanup */
      hwtimer_stop(sam_nand_timer);
      hwtimer_destroy(sam_nand_timer);
   } /* end of if (bKalUp) */

#endif
}


/********************************************************************
*  erase_samnand_block                                                   
*                                                                  
*  PARAMETERS:                                                     
*      addr - Block address                                        
*      callback - pointer to function to be called after the erase 
*                                                                  
*  DESCRIPTION:                                                    
*      This function erases the specified block from Samsung Nand  
*      flash device.                                               
*                                                                  
*  RETURNS:                                                        
*      0 for success and non zero in case of error                 
*                                                                  
********************************************************************/
int erase_samnand_block(u_int32 addr, void (*callback)(void))
{
   int result = 0;
   u_int8 device_status = 0;
      
   /* Enable the chip via PIO here(Note: Active low) */
   EnableChip();

   SEND_CMD(FLASH_CMD_READ1A);

   /* Send erase setup command. */
   SEND_CMD(FLASH_CMD_ERASE_SETUP);
   /* no need to write the LSByte for block erase */   
   *NANDADDR = (addr >> 8) & 0xFF;
   *NANDADDR = (addr >> 16) & 0xFF;
   /* Send erase confirm command. */
   SEND_CMD(FLASH_CMD_ERASE);
  
   /* do a dummy read here to flush the writes Suggested by Ken K */
   DUMMY_READ;

   /* Deassert the chip enable signal */
   DisableChip();
   
   /* Wait for erase to complete via Ready/Busy signal */
   WAIT_FOR_RDY;

   samnand_delay_usec(20*10);

   EnableChip();

   SEND_CMD(FLASH_CMD_STATUS);

   device_status = (u_int8)*NANDDATA; 

   if ( device_status & 0x01)
   {
      /* Error occured */
#if defined(DLOAD) 
      printf("ERROR: erase_samnand_block(), Erase command FAILED........\n");
#endif
      result = 1;
   }      

   /* do a dummy read here to flush the writes Suggested by Ken K */
   DUMMY_READ;  // dummy read to flush writes

   /* Deassert the chip enable signal */
   DisableChip();

   if(callback)
   {
      callback();
   }

   return result;
}

/********************************************************************
*  erase_samnand                                                   
*                                                                  
*  PARAMETERS:                                                     
*      addr - Block address                                        
*      callback - pointer to function to be called after the erase 
*                                                                  
*  DESCRIPTION:                                                    
*      This fucntion erases the specified block from Samsung Nand  
*      flash device.                                               
*                                                                  
*  RETURNS:                                                        
*      0 for success and non zero in case of error                 
*                                                                  
********************************************************************/
int erase_samnand(u_int32 blockaddr, void (*callback)(void))
{
   u_int32 pageindex = 0;
   u_int32 address = 0;
   u_int32 pagesize = 0;
   u_int32 blocksize = 0;
   int result;
   pagesize = Bank[CurrentBank].pNandFlashDesc->NandFlashPageSize;
   blocksize = pagesize *
                       Bank[CurrentBank].pNandFlashDesc->NandFlashPagesPerBlock;

#if defined(DLOAD) 
////   printf("Erasing block[0x%lx]\n", blockaddr);
#endif
   address = ((blockaddr * blocksize) + (pageindex*pagesize)) >> 1;/* A8 is not used */
   result = erase_samnand_block(address, NULL);

   return result;
}

/********************************************************************
*  erase_samnand_bank                                                   
*                                                                  
*  PARAMETERS:                                                     
*      addr - Block address                                        
*      callback - pointer to function to be called after the erase 
*                                                                  
*  DESCRIPTION:                                                    
*      This fucntion erases the specified block from Samsung Nand  
*      flash device.                                               
*                                                                  
*  RETURNS:                                                        
*      0 for success and non zero in case of error                 
*                                                                  
********************************************************************/

int erase_samnand_bank(void *addr, void (*callback)(void))
{
   int result = 0;
   int block=0;
   bool bValidBlock = FALSE;

   /* since there is no chip/Bank erase command in Samsung Nand Flash 
      we iterate through the blocks one by one and erase */
   for(block=WRITE_START_BLOCK_ADDR; block < Bank[CurrentBank].pNandFlashDesc->NandFlashNumOfBlocks; block++)
   {
      /* Erase only the valid blocks */
      /* NOTE: NEVER, NEVER, NEVER EVER erase invalid blocks as the invalid block
      info will be permanently lost */
      if(IsBlockValid(block, &bValidBlock) == 0)
      {         
         if(bValidBlock)
         {
            result = erase_samnand(block, NULL);
         }
      }
   }
   /* Wait for erase to complete. */

   if(callback)
   {
      callback();
   }
   return result;
}


/********************************************************************
*  read_ID_sam_nandflash                                           
*                                                                  
*  PARAMETERS:                                                     
*      pdeviceid - pointer to the device id to be returned                                        
*                                                                  
*  DESCRIPTION:                                                    
*      This fucntion returns the device ID from samsung Nand Flash.                                               
*                                                                  
*      -----------------------------                                                            
*      \   Maker code\  Device code\
*      -----------------------------
*             MS byte     LS byte                                                  
*                                                                  
*  RETURNS:                                                        
*      0 for success and non zero in case of error                 
*                                                                  
********************************************************************/
int read_ID_sam_nandflash(u_int16* pdeviceid)
{
   u_int8 device_code, maker_code;
   int status = 0;

   EnableChip();
   SEND_CMD(FLASH_CMD_ID);  // read id
   *NANDADDR = 0x00000000;  // address
   maker_code = *NANDDATA;
   device_code = *NANDDATA;
   DisableChip();

   *pdeviceid = (maker_code<<8) | device_code;

   return status;
}


/********************************************************************
*  write_page_sam_nandflash                                           
*                                                                  
*  PARAMETERS:                                                     
*      address,  - page address                                        
       pdata -   pointer to page data
       pspare - pointer to spare data
*                                                                  
*  DESCRIPTION:                                                    
*      This function programs the data+spare into a page in Nand Flash.                                               
*                                                                  
*                                                                  
*  RETURNS:                                                        
*      0 for success and non zero in case of error                 
*                                                                  
********************************************************************/
NAND_FLASH_STATUS_TYPE write_page_sam_nandflash(u_int32 address, 
                                                const void* pdata,
                                                const void* pspare)
{
   u_int32 i = 0;
   NAND_FLASH_STATUS_TYPE status = NAND_FLASH_STATUS_SUCCESS;
   u_int8* ptr8 =   (u_int8*)pdata;
   u_int16* ptr16 = (u_int16*)pdata;
   u_int32* ptr32 = (u_int32*)pdata;
   u_int8 device_status = 0;

   EnableChip();
   SEND_CMD(FLASH_CMD_READ1A);  // seq. data in  command
   SEND_CMD(FLASH_CMD_PAGE_PROG_SETUP);  // seq. data in  command
   *NANDADDR = address & 0xff;
   *NANDADDR = address >> 8 & 0xff;
   *NANDADDR = address >> 16 & 0xff;

      /* Write data. */ 
   while (i < (Bank[CurrentBank].pNandFlashDesc->NandFlashPageSize +
               Bank[CurrentBank].pNandFlashDesc->NandFlashSpareAreaSize)) 
   {
      if(i == Bank[CurrentBank].pNandFlashDesc->NandFlashPageSize)
      {
         /* Set the ponters to spare area */
         ptr8 = (u_int8*)pspare;
         ptr16 = (u_int16*)pspare;
         ptr32 = (u_int32*)pspare;
      }
      switch (Bank[CurrentBank].BankWidth)
      {                
         case 8:
           WRITE_DATA( *ptr8++ & 0xFF);
           break;
         case 16:
           WRITE_DATA( *(u_int16*)ptr16++ & 0xFFFF);
           break;
         case 32:
           WRITE_DATA( *(u_int32*)ptr32++ & 0xFFFFFFF);
           break;
      }
      i++;
   }

   SEND_CMD(FLASH_CMD_PAGE_PROG);  // page program command
   DUMMY_READ;
   DisableChip();

   /* Wait for write to complete via Ready/Busy signal */
   WAIT_FOR_RDY;

   EnableChip();
   SEND_CMD(FLASH_CMD_STATUS);  // status command
   // check status
   device_status = READ_DATA();
   if( device_status & 0x01) 
   {
#if defined(DLOAD) 
      printf("***Error: Status error in Writepage\n");
#endif
      status = NAND_FLASH_STATUS_PAGE_WRITE_ERROR;
   }
   DisableChip();
   return status;
}

/********************************************************************
*  read_page_sam_nandflash                                           
*                                                                  
*  PARAMETERS:                                                     
*      address,  - page address                                        
*      pdata -   pointer to page data
*                                                                  
*  DESCRIPTION:                                                    
*      This function reads the data from a page in Nand Flash.                                               
*                                                                  
*                                                                  
*  RETURNS:                                                        
*      0 for success and non zero in case of error                 
*                                                                  
********************************************************************/
NAND_FLASH_STATUS_TYPE read_page_sam_nandflash(u_int32 address, 
                                               void* pdata)
{
   u_int32 i = 0;
   NAND_FLASH_STATUS_TYPE status = NAND_FLASH_STATUS_SUCCESS;
   u_int8* ptr8;
   u_int16* ptr16;
   u_int32* ptr32;

#if 0
   printf("   read page at address[0x%lx], block[0x%lx], page[0x%lx]\n", 
               address, (address >>13), (address>>9) & 0xF);
#endif

   ptr8 = (u_int8*)pdata;
   ptr16 = (u_int16*)pdata;
   ptr32 = (u_int32*)pdata;

   EnableChip();
   SEND_CMD(FLASH_CMD_READ1A);  // seq. data in  command
   *NANDADDR = address & 0xff;
   *NANDADDR = address >> 8 & 0xff;
   *NANDADDR = address >> 16 & 0xff;
   WAIT_FOR_RDY;

   /* This delay is not properly documented in the data sheet */
   /* However this accounts for the delay(tRR) needed beyond tR */
   samnand_delay_usec(20*10);

   while (i< (Bank[CurrentBank].pNandFlashDesc->NandFlashPageSize +
         Bank[CurrentBank].pNandFlashDesc->NandFlashSpareAreaSize)) 
   {
      switch (Bank[CurrentBank].BankWidth)
      {                
         case 8:
            *ptr8++ = (u_int8)(*NANDDATA) & 0xFF;
            break;
         case 16:
           *ptr16++ = READ_DATA() & 0xFFFF;
           break;
         case 32:
           *ptr32++ = READ_DATA() & 0xFFFFFFF;
           break;
      }
      i++;
   }
   
   DisableChip();

   return status;
}

/********************************************************************/
/*  read_sam_nandflash                                              */
/*                                                                  */
/*  PARAMETERS:                                                     */
/*      logicaladdress -                                            */
/*               0(Start at beginning block of flash)               */
/*               1(Start at next block of flash)                    */
/*      pdata - pointer to the destination buffer                   */
/*      size - Number of bytes to read                              */
/*                                                                  */
/*  DESCRIPTION:                                                    */
/*      Reads the specified bytes from nand flash.                  */
/*                                                                  */
/*  RETURNS:                                                        */
/*      Nothing.                                                    */
/********************************************************************/
int read_sam_nandflash(u_int32 logicaladdress,
                       u_int8* pdata,
                       u_int32 size)
{
   u_int32 pagesize = Bank[CurrentBank].pNandFlashDesc->NandFlashPageSize;
   u_int32 pagesperblock = Bank[CurrentBank].pNandFlashDesc->NandFlashPagesPerBlock;
   u_int32 blocksize = pagesize * pagesperblock;
   u_int32 maxnumofblocks = Bank[CurrentBank].pNandFlashDesc->NandFlashNumOfBlocks;
   u_int32 numofpages = size / pagesize;
   u_int32 partialpagesize = size % pagesize;
   u_int8  page[528];/* PageSize+SpareSize]; */
   u_int16 ecc_gen[3];
   u_int8  ecc_gen_8[3];

   u_int32 offset = 0;
   u_int8 correctedbyte;
   int32  retStatus = 0;
   u_int32 totalpagesreadcount = 0;
   bool bValidBlock = FALSE;
   u_int32  block = 0;
   u_int32 pageindex = 0;
   u_int32 address = 0;
   u_int32 pageshift;

   switch(pagesperblock)
   {
      case 16:
         pageshift = 4; /* bits */
      break;
      case 32:
         pageshift = 5; /* bits */
      break;
      case 64:
         pageshift = 6; /* bits */
      break;
      default:
         pageshift = 4; /* bits */
      break;
   }
     
   if(partialpagesize)
   {
      /* To Read the last page */
      numofpages++;
   } 
   if(logicaladdress == 0)
   {
      /* Begin at the first valid block */
      read_start_block_addr = READ_START_BLOCK_ADDR;
   }
   /* While reading skip the Bad/Invalid blocks */
   for(block = read_start_block_addr;  block < maxnumofblocks; block++)
   {
      if(totalpagesreadcount >= numofpages)
      {
         /* Done with all the pages */
         break;
      }
      IsBlockValid(block, &bValidBlock);
      if(bValidBlock)
      {
         /* Block is valid, now go read all the pages in the block */
         for(pageindex = 0; pageindex < pagesperblock; pageindex++)
         {
            address = ((block * blocksize) + (pageindex*pagesize)) >> 1;/* A8 is not used */
            read_page_sam_nandflash(address, page);
            make_ecc_512Byte(page, ecc_gen);
            
            ecc_gen_8[0] = (u_int8)ecc_gen[0];
            ecc_gen_8[1] = (u_int8)ecc_gen[1];
            ecc_gen_8[2] = (u_int8)ecc_gen[2];
            
            if( (ecc_gen_8[0] != page[518]) ||
                (ecc_gen_8[1] != page[519]) ||
                (ecc_gen_8[2] != page[520]) )
            {
               /* ECC mismatch occured */

#if defined(DLOAD) 
               printf("read page addr[0x%lx],blk[0x%lx],page[0x%lx] orig[%02x %02x %02x], comp[%02x %02x %02x]\n", 
                        address, (address >>(8+pageshift)), (address>>8) & (pageshift-1),
                        page[518], page[519], page[520], 
                        (u_int8)ecc_gen_8[0], (u_int8)ecc_gen_8[1], (u_int8)ecc_gen_8[2]);
#else
//               CheckpointC("SUNIL: read page ECC mismatch...\n\r");
               
#endif
               /* Detect and correct an error*/
               
               retStatus = perform_ecc_512Byte(&page[518], (u_int8*)ecc_gen_8, page, &offset, (u_int8*)&correctedbyte);

               if ( (ECC_CORRECTABLE_ERROR == retStatus) || (ECC_NO_ERROR == retStatus) )
               {
#if defined(DLOAD) 
                  printf("Corrected the error via ECC during read.....** Line %d in %s\n", __LINE__, __FILE__);
#endif /* defined(DLOAD) */
                  retStatus = 0;
               }
               else
               {
#if defined(DLOAD) 
                  printf("ERROR: Fatal error during read.....** Line %d in %s\n", __LINE__, __FILE__);
                  retStatus = 1;
#endif /* defined(DLOAD) */
               }
            }
            /* Now copy the page */ 
            totalpagesreadcount++;         
            if(partialpagesize && (totalpagesreadcount == numofpages))
            {
               /* Last page and partial and so copy */
               FCopy((u_int8*)pdata, (u_int8*)page, partialpagesize);
               break;      
            }
            else
            {
               FCopy((u_int8*)pdata, (u_int8*)page, pagesize);
               if (totalpagesreadcount == numofpages)
               {
                  break;
               }
            }
            pdata += pagesize;
         }
      }   
   }
   read_start_block_addr = block; 
   return retStatus;
}

/********************************************************************/
/*  write_sam_nandflash                                             */
/*                                                                  */
/*  PARAMETERS:                                                     */
/*      logicaladdress -                                            */
/*               0(Start at beginning block of flash)               */
/*               1(Start at next block of flash)                    */
/*      pdata - pointer to the source buffer                        */
/*      size - Number of bytes to write                             */
/*                                                                  */
/*  DESCRIPTION:                                                    */
/*      Writes the buffer to Nand Flash                              */
/*                                                                  */
/*  RETURNS:                                                        */
/*      0 - Success.                                                */
/*      1 - Failure.                                                */
/********************************************************************/
int write_sam_nandflash(u_int32 logicaladdress,
                       const void* pdata,
                       u_int32 size)
{
   
   int retstatus = 0;
   u_int32 pagesize = Bank[CurrentBank].pNandFlashDesc->NandFlashPageSize;
   u_int32 sparesize = Bank[CurrentBank].pNandFlashDesc->NandFlashSpareAreaSize;
   u_int32 blocksize = Bank[CurrentBank].pNandFlashDesc->NandFlashPageSize *
                       Bank[CurrentBank].pNandFlashDesc->NandFlashPagesPerBlock;

   u_int32 numofpages = size / pagesize;
   u_int32 partialpagesize = size % pagesize;
   u_int32 numofblocks = size/blocksize;
   u_int32 partialblocksize = size % blocksize;   
   u_int32 maxnumofblocks = Bank[CurrentBank].pNandFlashDesc->NandFlashNumOfBlocks;
   u_int32 pagesperblock = Bank[CurrentBank].pNandFlashDesc->NandFlashPagesPerBlock;

   u_int32 address = 0;
   u_int32 blockaddr = 0;
   u_int16 ecc_gen[3];
   u_int8* ptr8;
   u_int16* ptr16;
   u_int32* ptr32;
   u_int8 BBByte = 0xBB;
   u_int16 BBWord = (0xBB << 8);
   bool bValidBlock = FALSE;

   u_int8* ptrspare8 = NULL;
   u_int8* ptrsparesave8 = NULL;
   u_int16* ptrspare16 = NULL;
   u_int16* ptrsparesave16 = NULL;
   u_int8* ptrpartialpage8 = NULL;
   u_int16* ptrpartialpage16 = NULL;

   u_int32 totalpageswrittencount = 0;
   u_int32 totalsparewrittencount = 0;
   u_int32 pageindex = 0;

   u_int8 sparearray[512]; /* To accommadate max limit */
   u_int8 pageplusspare[528]; /* For partial page */
   
   bool bdone = FALSE; /* to track if the block/page has been written successfully */

   u_int8*  ptr8_1 = NULL;
   u_int16* ptr16_1 = NULL;

   ptr8 = (u_int8*)pdata;
   ptr16 = (u_int16*)pdata;
   ptr32 = (u_int32*)pdata;


   if(partialblocksize)
   {
      numofblocks++;   
   }
   if(partialpagesize)
   {
      numofpages++;      
   }

#if 0
   if(logicaladdress == 0)
   {
      blockaddr = WRITE_START_BLOCK_ADDR; /* sset the start block addr */      
   }
   else
   {
      blockaddr = write_start_block_addr; /* sset the start block addr */
   }
#else
      blockaddr = write_start_block_addr; /* set the start block addr */
#endif
   while(blockaddr < maxnumofblocks) 
   {      
      if(totalpageswrittencount >= numofpages)
      {
         /* done with all pages */
         break;
      }

      ptrsparesave8 = ptrspare8 = (u_int8*)sparearray;
      ptrsparesave16 = ptrspare16 = (u_int16*)sparearray;
      ptr8_1 = ptr8;
      ptr16_1 = ptr16;

      totalsparewrittencount = totalpageswrittencount;
      /* generate the ECC data for all pages in the block first */
      FFillBytes(ptrspare8, 0xFF, sparesize * pagesperblock);
      for(pageindex = 0; pageindex < pagesperblock; pageindex++)
      {
         if (totalsparewrittencount >= numofpages)
         {
            break;
         }
         switch (Bank[CurrentBank].BankWidth)
         {                
            case 8:
              /* check if this is the last block and last page */
              if(partialpagesize && (totalsparewrittencount == numofpages-1))
              {                 
                 /* Partial last page, handle it appropriately */
                 ptrpartialpage8 = (u_int8*)pageplusspare;
                 FFillBytes((u_int8*)ptrpartialpage8, 0xFF, pagesize+sparesize);
                 /* copy the data from the buffer to the page */
                 FCopy(ptrpartialpage8, ptr8_1, partialpagesize);
                 ptr8_1 = ptrpartialpage8;                                   
              }
              make_ecc_512Byte(ptr8_1, ecc_gen);
              ptrspare8[6] = (u_int8)ecc_gen[0];
              ptrspare8[7] = (u_int8)ecc_gen[1];
              ptrspare8[8] = (u_int8)ecc_gen[2];
              ptrspare8 += sparesize;
              ptr8_1 += pagesize;
              break;
            case 16:
              /* check if this is the last block and last page */
              if(partialpagesize && (totalsparewrittencount == numofpages-1))
              {                 
                 /* Partial last page, handle it appropriately */
                 ptrpartialpage16 = (u_int16*)pageplusspare;
                 FFillBytes((u_int8*)ptrpartialpage16, 0xFF, pagesize+sparesize);
                 /* copy the data from the buffer to the page */
                 FCopy(ptrpartialpage16, ptr16_1, partialpagesize);
                 ptr16_1 = ptrpartialpage16;
              }
              make_ecc_256Word(ptr16_1, ecc_gen);
              ptrspare16[3] = ((u_int8)ecc_gen[1] << 8) | (u_int8)ecc_gen[0];
              ptrspare16[4] = (u_int8)ecc_gen[2];
              ptrspare16 += sparesize;
              ptr16_1 += pagesize;
              break;
            case 32:
              break;
         } /* End of switch */
         totalsparewrittencount++;
      }          

      /* Restore the spare ptrs */
      ptrspare8 = ptrsparesave8;
      ptrspare16 = ptrsparesave16;
      bdone = FALSE;
      /* Now start the actual Flash writes */
      while(bdone == FALSE)
      {
         IsBlockValid(blockaddr, &bValidBlock);
         if(FALSE == bValidBlock)
         {
            blockaddr++;
            continue;
         }

         if(FALSE == bValidBlock)
         {
#if defined(DLOAD) 
            printf("FATAL ERROR: No valid blocks found to write the image........\n");
#endif
            return 1;
         }
         /* Erase the valid block */
         erase_samnand(blockaddr, NULL);
         for(pageindex =0; pageindex < pagesperblock; pageindex++)
         {
            if (totalpageswrittencount >= numofpages)
            {
               break;
            }

            address = ((blockaddr * blocksize) + (pageindex*pagesize)) >> 1;/* A8 is not used */
            switch (Bank[CurrentBank].BankWidth)
            {                
               case 8:
                 /* check if this is the last block and last page */
                 if(partialpagesize && (totalpageswrittencount == numofpages-1))
                 {
                    /* setup for programming the partial page */
                    ptr8 = ptrpartialpage8;                  
                 }
                 if (write_page_sam_nandflash(address, (void*)ptr8, (void*)ptrspare8))
                 {
#if defined(DLOAD) 
                    printf("WARNING: Write page failed at 0x%lx, marking the block as BAD\n", address);
#endif
                    /* A write page operation failed, Update the BI area in page 0 and retry*/
                    address = ((blockaddr * blocksize) + (0*pagesize)) >> 1;/* A8 is not used */
                    write_spare_area(address, 5, &BBByte, 1);
                    /* retract the data and spare pointers */
                    ptr8 -= (pageindex) * pagesize; 
                    ptrspare8 -= (pageindex) * sparesize;
                    totalpageswrittencount -= pageindex;
                    bdone = FALSE; /* Set up so we can try the block again */
                    break;
                 }
                 else
                 {
                    /* page write successful, advance the data and spare pointers */
                    bdone = TRUE;
                    ptr8 += pagesize;
                    ptrspare8 += sparesize;
                    totalpageswrittencount++;
                 }
                 break;
               case 16:
                 /* check if this is the last block and last page */
                 if(partialpagesize && (totalpageswrittencount == numofpages-1))
                 {
                    /* setup for programming the partial page */
                    ptr16 = ptrpartialpage16;                  
                 }
                 if (write_page_sam_nandflash(address, (void*)ptr16, (void*)ptrspare16))
                 {
                    /* A write page operation failed, Update the BI area in page 0 and retry*/
                    address = ((blockaddr * blocksize) + (0*pagesize)) >> 1;/* A8 is not used */
                    write_spare_area(address, 5, (u_int16*)&BBWord, 1);
                    /* retract the data and spare pointers */
                    ptr16 -= (pageindex) * pagesize; 
                    ptrspare16 -= (pageindex) * sparesize;
                    totalpageswrittencount -= pageindex;
                    bdone = FALSE; /* Set up so we can try the block again */
                    break;
                 }
                 else
                 {
                    /* page write successful, advance the data and spare pointers */
                    bdone = TRUE;
                    ptr16 += pagesize;
                    ptrspare16 += sparesize;
                    totalpageswrittencount++;
                 }
                 break;
               case 32:
                 break;
            } /* end of switch */
            if(!bdone)
            {
               break; /* out of for loop - error case(write to a block failed */
            }
         } /* end of for */
         /* increment the block addr */
         blockaddr++;
      }/* end of while(bdone == FALSE) */
   }/* end of while(blockaddr < maxnumofblocks) */

   write_start_block_addr = blockaddr;

   return retstatus;
}

/* Write to the spare area */
int write_spare_area(u_int32 pageaddress, 
                     u_int32 columnaddress, 
                     void* pdata, 
                     u_int32 size)
{
   int retstatus = 0;
   u_int8* ptr8;
   u_int16* ptr16;
   u_int32* ptr32;
   u_int32 i = 0;
   u_int32 address = 0;

   ptr8 = (u_int8*)pdata;
   ptr16 = (u_int16*)pdata;
   ptr32 = (u_int32*)pdata;

  EnableChip();  
  address = address|pageaddress;
  address = address | columnaddress;

  SEND_CMD(FLASH_CMD_READ2_SPARE);  // set pointer to spare command
  SEND_CMD(FLASH_CMD_PAGE_PROG_SETUP);
  *NANDADDR = address & 0xff;
  *NANDADDR = address >> 8 & 0xff;
  *NANDADDR =  address >> 16 & 0xff;

   for(i=0; i < size; i++)
   {
      switch (Bank[CurrentBank].BankWidth)
      {                
         case 8:
           WRITE_DATA( *ptr8++ & 0xFF);
           break;
         case 16:
           WRITE_DATA( *ptr16++ & 0xFFFF);
           break;
         case 32:
           WRITE_DATA( *ptr32++ & 0xFFFFFFF);
           break;
      }
   }
   SEND_CMD(FLASH_CMD_PAGE_PROG);  // page program command
   DUMMY_READ;
   DisableChip();

   /* Wait for write to complete via Ready/Busy signal */
   WAIT_FOR_RDY;

   EnableChip();
   SEND_CMD(FLASH_CMD_STATUS);  // status command
   // check status
   if(READ_DATA() & 0x01) 
   {
#if defined(DLOAD) 
      printf("***Error: Status error in Write spare area\n");
#endif
      ///status = NAND_FLASH_STATUS_PAGE_WRITE_ERROR;
   }
   DisableChip();
   return retstatus;
}


void read_spare_area(u_int32 pageaddress, u_int8 columnaddress, u_int8 size, void* buffer)
{
   u_int32 i=0, address=0;
   u_int8* ptr8 = NULL;
   u_int16* ptr16 = NULL;
   ptr8 = (u_int8*)buffer;
   ptr16 = (u_int16*)buffer;

   EnableChip();  

   address = address|pageaddress;
   address = address | columnaddress;

   // read bytes
   SEND_CMD(FLASH_CMD_READ2_SPARE);  // read spare command
   *NANDADDR = address & 0xff;
   *NANDADDR = (address >> 8) & 0xff;
   *NANDADDR = (address >> 16) & 0xff;

   /* Wait for write to complete via Ready/Busy signal */
   WAIT_FOR_RDY;
   
   /* This delay is not properly documented in the data sheet */
   /* However this accounts for the delay(tRR) needed beyond tR */
   samnand_delay_usec(20*10);

   // Read back the spare area.
   for(i=0; i<size; i++)
   {
      switch (Bank[CurrentBank].BankWidth)
      {                
         case 8:
            *ptr8++ = (u_int8)*NANDDATA; //Get the Byte
         break;
         case 16:
            *ptr16++ = *NANDDATA; //Get the Word
         break;
         case 32:
         default:
         break;
      }
   }
   DisableChip();
}

int IsBlockValid(u_int32 block, bool* pbValidBlock)
{
   u_int32 retstatus = 0;

   u_int32 pagesize = Bank[CurrentBank].pNandFlashDesc->NandFlashPageSize;
   u_int32 blocksize = pagesize *
                       Bank[CurrentBank].pNandFlashDesc->NandFlashPagesPerBlock;

   u_int32 address = 0;
   u_int32 pageindex = 0;
   u_int8  BBytefrompage0 = 0xFF;
   u_int8  BBytefrompage1 = 0xFF;
   
   u_int8  ColumnAddr = 5; /* For x8 */

   *pbValidBlock = TRUE;

   /* Read spare area of page 0 in this block */
   pageindex = 0;
   address = ((block * blocksize) + (pageindex*pagesize)) >> 1; /* A8 is not used */
   read_spare_area(address, ColumnAddr, 1, (void*)&BBytefrompage0);
   BadBlock[bbcount].pageaddr0 = address;

   /* Read spare area of page 1 in this block */
   pageindex = 1;
   address = ((block * blocksize) + (pageindex*pagesize)) >> 1; /* A8 is not used */
   read_spare_area(address, ColumnAddr, 1, (void*)&BBytefrompage1);
   BadBlock[bbcount].pageaddr1 = address;

   if( (BBytefrompage0 != 0xFF) || (BBytefrompage1 != 0xFF) )
   {
      BadBlock[bbcount].val0 = BBytefrompage0;
      BadBlock[bbcount].val1 = BBytefrompage1;
      BadBlock[bbcount].blockaddr = block;
      bbcount++;
      *pbValidBlock = FALSE;
   }
   return retstatus;
}

int CountInvalidBlocks(u_int32* pinvalidblockcount)
{
   u_int32 retstatus = 0;
   u_int32 maxnumofblocks = Bank[CurrentBank].pNandFlashDesc->NandFlashNumOfBlocks;
   u_int32 block = 0;
   bool bValidBlock = FALSE;
   /* reset the bad block count */
   bbcount = 0;

   FFillBytes(BadBlock, 0, sizeof(BadBlock));
   if(!pinvalidblockcount)
   {
#if defined(DLOAD) 
      printf("CountInvalidBlocks: pinvalidblockcount is NULL\n");
#endif
      return 1;
   }
   *pinvalidblockcount = 0;
   for(block = 0; block < maxnumofblocks; block++)
   {
      IsBlockValid(block, &bValidBlock);
      if(bValidBlock == FALSE)
      {
         (*pinvalidblockcount)++; 
      }
   }
   return retstatus;
}

static u_int32 crc32_table[256] = 
/*CRC32 Lookup Table Data values  */
{
   /*   0 -- */   0x00000000, 0x77073096, 0xee0e612c, 0x990951ba, 
   /*   4 -- */   0x076dc419, 0x706af48f, 0xe963a535, 0x9e6495a3, 
   /*   8 -- */   0x0edb8832, 0x79dcb8a4, 0xe0d5e91e, 0x97d2d988, 
   /*  12 -- */   0x09b64c2b, 0x7eb17cbd, 0xe7b82d07, 0x90bf1d91, 
   /*  16 -- */   0x1db71064, 0x6ab020f2, 0xf3b97148, 0x84be41de, 
   /*  20 -- */   0x1adad47d, 0x6ddde4eb, 0xf4d4b551, 0x83d385c7, 
   /*  24 -- */   0x136c9856, 0x646ba8c0, 0xfd62f97a, 0x8a65c9ec, 
   /*  28 -- */   0x14015c4f, 0x63066cd9, 0xfa0f3d63, 0x8d080df5, 
   /*  32 -- */   0x3b6e20c8, 0x4c69105e, 0xd56041e4, 0xa2677172, 
   /*  36 -- */   0x3c03e4d1, 0x4b04d447, 0xd20d85fd, 0xa50ab56b, 
   /*  40 -- */   0x35b5a8fa, 0x42b2986c, 0xdbbbc9d6, 0xacbcf940, 
   /*  44 -- */   0x32d86ce3, 0x45df5c75, 0xdcd60dcf, 0xabd13d59, 
   /*  48 -- */   0x26d930ac, 0x51de003a, 0xc8d75180, 0xbfd06116, 
   /*  52 -- */   0x21b4f4b5, 0x56b3c423, 0xcfba9599, 0xb8bda50f, 
   /*  56 -- */   0x2802b89e, 0x5f058808, 0xc60cd9b2, 0xb10be924, 
   /*  60 -- */   0x2f6f7c87, 0x58684c11, 0xc1611dab, 0xb6662d3d, 
   /*  64 -- */   0x76dc4190, 0x01db7106, 0x98d220bc, 0xefd5102a, 
   /*  68 -- */   0x71b18589, 0x06b6b51f, 0x9fbfe4a5, 0xe8b8d433, 
   /*  72 -- */   0x7807c9a2, 0x0f00f934, 0x9609a88e, 0xe10e9818, 
   /*  76 -- */   0x7f6a0dbb, 0x086d3d2d, 0x91646c97, 0xe6635c01, 
   /*  80 -- */   0x6b6b51f4, 0x1c6c6162, 0x856530d8, 0xf262004e, 
   /*  84 -- */   0x6c0695ed, 0x1b01a57b, 0x8208f4c1, 0xf50fc457, 
   /*  88 -- */   0x65b0d9c6, 0x12b7e950, 0x8bbeb8ea, 0xfcb9887c, 
   /*  92 -- */   0x62dd1ddf, 0x15da2d49, 0x8cd37cf3, 0xfbd44c65, 
   /*  96 -- */   0x4db26158, 0x3ab551ce, 0xa3bc0074, 0xd4bb30e2, 
   /* 100 -- */   0x4adfa541, 0x3dd895d7, 0xa4d1c46d, 0xd3d6f4fb, 
   /* 104 -- */   0x4369e96a, 0x346ed9fc, 0xad678846, 0xda60b8d0, 
   /* 108 -- */   0x44042d73, 0x33031de5, 0xaa0a4c5f, 0xdd0d7cc9, 
   /* 112 -- */   0x5005713c, 0x270241aa, 0xbe0b1010, 0xc90c2086, 
   /* 116 -- */   0x5768b525, 0x206f85b3, 0xb966d409, 0xce61e49f, 
   /* 120 -- */   0x5edef90e, 0x29d9c998, 0xb0d09822, 0xc7d7a8b4, 
   /* 124 -- */   0x59b33d17, 0x2eb40d81, 0xb7bd5c3b, 0xc0ba6cad, 
   /* 128 -- */   0xedb88320, 0x9abfb3b6, 0x03b6e20c, 0x74b1d29a, 
   /* 132 -- */   0xead54739, 0x9dd277af, 0x04db2615, 0x73dc1683, 
   /* 136 -- */   0xe3630b12, 0x94643b84, 0x0d6d6a3e, 0x7a6a5aa8, 
   /* 140 -- */   0xe40ecf0b, 0x9309ff9d, 0x0a00ae27, 0x7d079eb1, 
   /* 144 -- */   0xf00f9344, 0x8708a3d2, 0x1e01f268, 0x6906c2fe, 
   /* 148 -- */   0xf762575d, 0x806567cb, 0x196c3671, 0x6e6b06e7, 
   /* 152 -- */   0xfed41b76, 0x89d32be0, 0x10da7a5a, 0x67dd4acc, 
   /* 156 -- */   0xf9b9df6f, 0x8ebeeff9, 0x17b7be43, 0x60b08ed5, 
   /* 160 -- */   0xd6d6a3e8, 0xa1d1937e, 0x38d8c2c4, 0x4fdff252, 
   /* 164 -- */   0xd1bb67f1, 0xa6bc5767, 0x3fb506dd, 0x48b2364b, 
   /* 168 -- */   0xd80d2bda, 0xaf0a1b4c, 0x36034af6, 0x41047a60, 
   /* 172 -- */   0xdf60efc3, 0xa867df55, 0x316e8eef, 0x4669be79, 
   /* 176 -- */   0xcb61b38c, 0xbc66831a, 0x256fd2a0, 0x5268e236, 
   /* 180 -- */   0xcc0c7795, 0xbb0b4703, 0x220216b9, 0x5505262f, 
   /* 184 -- */   0xc5ba3bbe, 0xb2bd0b28, 0x2bb45a92, 0x5cb36a04, 
   /* 188 -- */   0xc2d7ffa7, 0xb5d0cf31, 0x2cd99e8b, 0x5bdeae1d, 
   /* 192 -- */   0x9b64c2b0, 0xec63f226, 0x756aa39c, 0x026d930a, 
   /* 196 -- */   0x9c0906a9, 0xeb0e363f, 0x72076785, 0x05005713, 
   /* 200 -- */   0x95bf4a82, 0xe2b87a14, 0x7bb12bae, 0x0cb61b38, 
   /* 204 -- */   0x92d28e9b, 0xe5d5be0d, 0x7cdcefb7, 0x0bdbdf21, 
   /* 208 -- */   0x86d3d2d4, 0xf1d4e242, 0x68ddb3f8, 0x1fda836e, 
   /* 212 -- */   0x81be16cd, 0xf6b9265b, 0x6fb077e1, 0x18b74777, 
   /* 216 -- */   0x88085ae6, 0xff0f6a70, 0x66063bca, 0x11010b5c, 
   /* 220 -- */   0x8f659eff, 0xf862ae69, 0x616bffd3, 0x166ccf45, 
   /* 224 -- */   0xa00ae278, 0xd70dd2ee, 0x4e048354, 0x3903b3c2, 
   /* 228 -- */   0xa7672661, 0xd06016f7, 0x4969474d, 0x3e6e77db, 
   /* 232 -- */   0xaed16a4a, 0xd9d65adc, 0x40df0b66, 0x37d83bf0, 
   /* 236 -- */   0xa9bcae53, 0xdebb9ec5, 0x47b2cf7f, 0x30b5ffe9, 
   /* 240 -- */   0xbdbdf21c, 0xcabac28a, 0x53b39330, 0x24b4a3a6, 
   /* 244 -- */   0xbad03605, 0xcdd70693, 0x54de5729, 0x23d967bf, 
   /* 248 -- */   0xb3667a2e, 0xc4614ab8, 0x5d681b02, 0x2a6f2b94, 
   /* 252 -- */   0xb40bbe37, 0xc30c8ea1, 0x5a05df1b, 0x2d02ef8d
};

/* ----------------------------------------------------------------------------  */
/* FUNCTION: sam_nand_crc32                                                               */
/* RETURNS: new crc value                                                        */
/* PARAMETERS: pBlock - pointer to the buffer to calculate a CRC from            */
/*             nBlockLen - size of the buffer                                    */
/*                                                                               */
/* COMMENTS: The CRC returned is the checksum of the buffer.  To calculate       */
/*           checksums of many blocks, for each subsequent block, use the        */
/*           CRC value from the old block as the start value.                    */
/*                                                                               */
/* ----------------------------------------------------------------------------  */
u_int32 sam_nand_crc32
(
   u_int8   *pBlock,    /* Pointer to the buffer to compute the crc from  */
   u_int32  nBlockLen,  /* size of the buffer  */
   u_int32  crc         /* crc from old block */
)
{
   u_int32 tmp;

   while ( nBlockLen-- > 0 )
   {
      tmp = (crc ^ *pBlock) & 0x000000FFL;
      crc = ( (crc>>8) & 0x00FFFFFFL ) ^ crc32_table[tmp];
      pBlock++;
   }
   
   return( crc );   
}


void PrintBadBlockCatalog()
{
#if defined(DLOAD)
   u_int32 i;
   printf("==========================================\n");
   printf("Bad Block catalog: Count = %lu\n", bbcount);
   printf("==========================================\n\n");
   printf("Block#\tPage0\tval0\tpage1\tval1\n");
   for(i=0; i < bbcount; i++)
   {
      printf("%lx\t%lx\t%x\t%lx\t%x\n", BadBlock[i].blockaddr,
               BadBlock[i].pageaddr0, BadBlock[i].val0,
               BadBlock[i].pageaddr1, BadBlock[i].val1);
   }
#endif
}


void dump_data(u_int8* pdata, u_int32 size)
{
#if defined(DLOAD) 
   u_int32 i,j = 0;

   for(i=0; i < size; i++)
   {
      if(i%20 == 0)
      {
         printf("\n %lu:\t", j++);
      }
      printf("%02x ", *pdata++);    
   }
   printf("\n");
#endif
}

#if SAM_NAND_UNIT_TEST

u_int8 magic_page[528];

u_int8 magic_page_1[528] = 
{
0x92,0x8b,0x95,0xfb,0xcf,0xd1,0xe0,0x13,0x56,0xda,
0x0e,0x6b,0xfb,0x66,0xca,0x81,0x08,0x36,0x64,0x80,
0x03,0xd3,0x7c,0x74,0x29,0x23,0xf3,0x65,0x37,0xb4,
0xf9,0xe5,0xc0,0x21,0x03,0x12,0xce,0xfd,0x88,0x92,
0xe3,0x9a,0x59,0x89,0x37,0x1d,0x55,0x1c,0xc2,0x33,
0xe7,0xe3,0xe7,0xe9,0x1a,0x01,0x4e,0xdf,0xfd,0x2b,
0x4d,0xe6,0x65,0x5c,0xe9,0xcd,0x92,0x0b,0xc9,0xd3,
0xe1,0x80,0x2d,0x1a,0x94,0xc9,0xb8,0x86,0x38,0x53,
0xb7,0xde,0xd9,0x17,0x0e,0x27,0x7d,0x0b,0x12,0x84,
0x8d,0x27,0xfc,0x26,0x2f,0xad,0xa2,0xf2,0x3c,0x1e,
0x35,0x13,0xc6,0xe7,0x2e,0x81,0xe8,0xd0,0x2e,0xb9,
0x51,0xae,0x6a,0xc4,0x33,0x4d,0xfc,0x96,0xa3,0x38,
0xa1,0x25,0x0b,0xb7,0xc3,0x11,0x17,0xba,0xe2,0xc8,
0x7c,0x79,0x2a,0x12,0x57,0x09,0x49,0xca,0x83,0xb9,
0x68,0x61,0x41,0x6b,0xc7,0xc4,0xf1,0xad,0xde,0xd7,
0xd0,0x61,0x11,0xa5,0xe5,0xb3,0xc6,0xdc,0xa0,0x9b,
0x08,0xd9,0xd2,0x3e,0x65,0xba,0x79,0xd5,0xc3,0xc8,
0x14,0x91,0xdd,0xbb,0x6d,0xf9,0x1c,0x96,0x97,0x53,
0x83,0x1e,0x09,0x94,0xd7,0xc0,0x11,0x60,0xcd,0xd1,
0x12,0x99,0x52,0x05,0x42,0x00,0x49,0x88,0xd3,0xf6,
0x45,0x9c,0xcf,0xda,0xa8,0x9a,0x7f,0xfe,0x1c,0x40,
0xaa,0xcb,0x5d,0x31,0x18,0x93,0x51,0x03,0x83,0xfa,
0x44,0xcb,0xf6,0xb3,0xe4,0xfc,0xd1,0xc8,0xd7,0x8e,
0xd2,0xc8,0xdd,0xbe,0xee,0x0b,0x86,0xe3,0x63,0x52,
0xdc,0x9f,0x33,0x1d,0x84,0x6b,0x0c,0x53,0xbd,0x50,
0x37,0xb3,0xb9,0xbb,0x1f,0x9b,0x49,0x48,0x10,0x16,
0xbb,0x71,0xa7,0x5a,0xfd,0xfb,0x2e,0x0d,0x59,0x13,
0x4a,0x81,0x58,0x65,0x40,0xba,0xb1,0x3c,0x36,0x7d,
0x3e,0xdc,0xa9,0x2f,0xe2,0x25,0x9b,0x39,0x6c,0x41,
0x0a,0x58,0xc1,0xe9,0x44,0x6d,0x7a,0xf2,0x4f,0x3e,
0x27,0x35,0xab,0x4b,0xed,0x16,0xe3,0x92,0x44,0xa8,
0x2c,0x70,0xdc,0xb2,0xe0,0x03,0x97,0xa0,0x46,0x26,
0x1d,0x18,0x53,0xf3,0x5b,0xc5,0x63,0x7b,0x0d,0x97,
0x15,0xf1,0x99,0x6d,0x3d,0x27,0x5f,0x8b,0xb1,0x02,
0x41,0x45,0xca,0xbc,0x03,0x30,0xdf,0xbe,0x53,0xda,
0x38,0x31,0x11,0xce,0x84,0xc3,0x1c,0x66,0x00,0x2f,
0x53,0xb1,0xc8,0x6c,0x7e,0xf0,0x7f,0xba,0xd3,0xf0,
0xc5,0x5c,0xcc,0x3f,0xac,0xa7,0x81,0x07,0x34,0xde,
0x36,0x13,0xd1,0x5d,0x5f,0x3c,0xb8,0x93,0x3e,0x41,
0x59,0xe7,0xcd,0x5e,0xb7,0x68,0x49,0x8b,0x31,0x64,
0x54,0x48,0x3b,0x69,0x34,0x54,0x37,0x2c,0x0a,0xbb,
0xea,0xd9,0x05,0x1f,0x35,0xe0,0x86,0x0a,0x90,0x24,
0x9e,0x5f,0x74,0x3a,0x63,0x8a,0xb2,0x93,0x0d,0x9a,
0x58,0xb3,0x50,0xe5,0xb5,0x63,0x65,0x59,0x8d,0xf6,
0x36,0xa9,0x96,0x96,0x26,0xe5,0xca,0x25,0x65,0xb9,
0x2f,0x2b,0x3c,0xdc,0x99,0xec,0xdc,0xfc,0x8d,0x11,
0x06,0x10,0xbd,0x6d,0x1b,0x72,0xd9,0x79,0x71,0xb4,
0x9c,0x68,0x00,0x7e,0x4e,0x71,0xa6,0x64,0x3c,0xc9,
0x40,0x1b,0xbb,0x51,0xbd,0x6d,0x55,0x7c,0xe6,0x4c,
0x21,0xf5,0x91,0x73,0xef,0x8a,0xe8,0x6a,0xd1,0x4e,
0x92,0xe1,0x11,0x98,0x47,0x3b,0xad,0xba,0x35,0xaa,
0xf9,0x22,0x40,0xd6,0xe2,0xb3,0x6c,0xff,0x76,0x16,
0x93,0x63,0x4b,0x4c,0xb0,0xdd,0x3e,0x37
};
/* computed magic ECC = 0x59 0x56 0x55 */
u_int8 magic_page_2[528] = 
{
0xaf,0x29,0x8a,0x4b,0x21,0x44,0xed,0x76,0x56,0xef,
0xab,0xfc,0xf4,0x29,0xd7,0x4d,0xfb,0x43,0xa8,0xe0,
0x64,0xf2,0x7a,0x92,0x21,0x04,0xa3,0x58,0x8f,0xf0,
0xce,0x9c,0x6e,0x38,0x14,0xc3,0xed,0x87,0x67,0x91,
0x72,0xba,0x4a,0x10,0x95,0x3f,0xad,0x18,0x7d,0x38,
0xf7,0x1d,0x20,0x2e,0xb5,0xea,0x85,0xf6,0x46,0xe9,
0x97,0x2d,0x22,0xa4,0x1e,0x76,0x85,0x93,0xf1,0xd7,
0x18,0x73,0xfe,0xdd,0x65,0xab,0xa4,0xe2,0xbb,0x8a,
0x97,0xb3,0xbe,0xd4,0xbe,0x45,0xc9,0xee,0x3c,0x6d,
0xf0,0xaf,0x19,0xa4,0x36,0x8a,0xd1,0x4d,0x48,0xa5,
0x96,0x1f,0x5e,0x9b,0xa0,0x0c,0xfb,0xee,0x6c,0xdd,
0xcb,0x68,0xa1,0x55,0x39,0x78,0x8a,0xfc,0x0a,0xc7,
0xe9,0x16,0xb6,0x93,0x02,0x3a,0x06,0xfc,0x1f,0x19,
0x68,0xf4,0x05,0x7c,0xfd,0xdd,0xfd,0xef,0x35,0x09,
0xf8,0xab,0x8b,0xc8,0xf1,0x35,0xc2,0x8f,0x90,0xcd,
0x2d,0x73,0xb9,0x94,0xcc,0x0e,0xfd,0x03,0x70,0x7d,
0xa0,0xa7,0x94,0x12,0x2e,0x43,0x21,0xdd,0xc2,0xa5,
0xd8,0x40,0x98,0xfa,0x0b,0x17,0x65,0x5d,0x5f,0x34,
0x21,0x64,0x4c,0xc0,0x58,0xfe,0x10,0x30,0xfe,0xc7,
0x3a,0x62,0xf6,0x3f,0x22,0xa1,0x43,0x7a,0xd3,0x1a,
0xa0,0xbf,0xd3,0xed,0x61,0xae,0x40,0x14,0xdc,0xc9,
0x33,0x93,0xd1,0xa1,0x66,0xd5,0x79,0x60,0x7e,0x1d,
0x3c,0x88,0x8a,0x55,0x41,0x97,0x88,0xda,0xbc,0xd9,
0x06,0x5f,0x49,0x41,0xb6,0x7f,0x2a,0xde,0x85,0x53,
0x3d,0xee,0x0a,0x72,0x7b,0x18,0x5d,0xab,0xcf,0x39,
0xba,0x8a,0xb0,0xf3,0xeb,0xd2,0x2a,0xda,0x62,0x0c,
0x56,0x52,0x0c,0x50,0x74,0xa4,0xb6,0xf4,0x10,0xd7,
0x17,0xbc,0xef,0xb1,0xd0,0x25,0xbb,0x12,0xf8,0x0e,
0x96,0x79,0x8c,0x46,0x18,0x71,0x4e,0x2e,0xd8,0x2b,
0x71,0x13,0x8d,0x85,0x9e,0xe3,0x5e,0x56,0xac,0xe4,
0x73,0x17,0x97,0x0c,0xdf,0x6c,0x19,0x16,0xf2,0x0c,
0xe1,0x1f,0x8b,0x8c,0x5e,0xad,0xf6,0x99,0x06,0x8c,
0x7e,0x1b,0x19,0xe5,0x27,0xc9,0xd9,0xe5,0x47,0xb6,
0x95,0x79,0xf6,0x53,0x8b,0xcf,0x7e,0xec,0x98,0x44,
0x47,0xb6,0xa7,0x51,0xf0,0x49,0xbe,0xb4,0x92,0xb9,
0xc8,0xe7,0xcc,0x34,0xe0,0x0b,0xa8,0x2b,0xbf,0xf3,
0x71,0x1c,0x12,0xed,0x04,0xf0,0x5c,0x6d,0x41,0xf4,
0xaa,0xc0,0x22,0x75,0xbc,0xee,0x0a,0x81,0xe1,0x62,
0x73,0xd4,0x32,0x0e,0x79,0x71,0x74,0x21,0x5e,0x66,
0xb1,0x02,0xa5,0x4f,0x67,0x13,0x46,0x8d,0x56,0x1c,
0xa0,0x3b,0xcb,0xb3,0xae,0xe1,0x77,0x95,0x23,0x76,
0x9c,0xd1,0x6a,0x1f,0x42,0x20,0xc1,0xc2,0x24,0xd6,
0xd6,0x6f,0xf9,0xd4,0xae,0x0d,0x7f,0x51,0xad,0xf7,
0xad,0x67,0x97,0x68,0x9f,0xd5,0x5f,0x48,0x90,0xb4,
0x88,0xdf,0xea,0xb3,0x08,0x03,0x5d,0x15,0xe0,0x8c,
0xe3,0xf1,0xe5,0x6a,0xc5,0x88,0xdc,0x06,0xe4,0x9e,
0xee,0x34,0x75,0x30,0xa8,0x1c,0x6a,0xf5,0xea,0x29,
0xa8,0x91,0x8c,0xe9,0x6c,0xc9,0xa3,0x9c,0x49,0xb5,
0x9b,0x27,0x4b,0xc8,0x22,0xb4,0x23,0x0c,0x03,0x5f,
0xc9,0x8b,0x02,0xf8,0x28,0x28,0xd7,0xe8,0x95,0x76,
0x47,0x38,0x41,0xc4,0x04,0xb1,0xe2,0x6b,0x55,0xbe,
0x32,0x20,0xb8,0xdd,0x96,0xc7,0xe6,0xff,0x88,0x22,
0xf9,0xb2,0x04,0xb6,0xa2,0xb1,0x23,0x59
};
/* computed magic ECC = 0xfc 0xc3 0x0 */
u_int8 magic_page_3[528] = 
{
0x4f,0xd5,0x74,0xb3,0xdd,0xba,0x11,0x88,0xa8,0x0f,
0x53,0x02,0x0a,0xd5,0x27,0xb1,0x03,0xbb,0x56,0x1e,
0x9a,0x0c,0x84,0x6c,0xdd,0xfa,0x5b,0xa6,0x6f,0x0e,
0x30,0x62,0x90,0xc6,0xea,0x3b,0x11,0x77,0x97,0x6d,
0x8c,0x44,0xb4,0xee,0x69,0xbf,0x51,0xe6,0x81,0xc6,
0x07,0xe1,0xde,0xd0,0x49,0x14,0x79,0x08,0xb8,0x15,
0x67,0xd1,0xdc,0x5a,0xe0,0x88,0x79,0x6b,0x0d,0x27,
0xe6,0x8b,0x00,0x21,0x99,0x53,0x5a,0x1c,0x43,0x74,
0x67,0x4b,0x40,0x2a,0x40,0xb9,0x35,0x10,0xc2,0x91,
0x0e,0x4f,0xe5,0x5a,0xc8,0x74,0x2d,0xb1,0xb6,0x59,
0x68,0xdf,0xa0,0x63,0x5e,0xf2,0x03,0x10,0x92,0x21,
0x33,0x96,0x5d,0xa9,0xc5,0x86,0x74,0x02,0xf4,0x37,
0x15,0xe8,0x48,0x6b,0xfc,0xc4,0xf8,0x02,0xdf,0xe5,
0x96,0x0a,0xf9,0x82,0x01,0x21,0x01,0x0f,0xc9,0xf5,
0x06,0x53,0x73,0x36,0x0d,0xc9,0x3c,0x6f,0x6e,0x31,
0xd1,0x8b,0x45,0x6a,0x32,0xf0,0x01,0xfb,0x8e,0x81,
0x5e,0x57,0x6a,0xec,0xd0,0xbb,0xdd,0x21,0x3c,0x59,
0x26,0xbe,0x66,0x04,0xf3,0xe7,0x99,0xa1,0x9f,0xca,
0xdd,0x9a,0xb2,0x3e,0xa6,0x00,0xee,0xce,0x00,0x37,
0xc4,0x9c,0x08,0xbf,0xdc,0x5d,0xbb,0x84,0x2b,0xe4,
0x5e,0x3f,0x2b,0x11,0x9d,0x50,0xbe,0xea,0x22,0x35,
0xcb,0x6b,0x2d,0x5d,0x98,0x29,0x85,0x9e,0x80,0xe1,
0xc2,0x76,0x74,0xa9,0xbd,0x67,0x76,0x24,0x42,0x25,
0xf8,0x9f,0xb5,0xbd,0x48,0x7f,0xd4,0x20,0x79,0xab,
0xc1,0x10,0xf4,0x8c,0x83,0xe6,0xa1,0x53,0x2f,0xc5,
0x44,0x74,0x4e,0x0b,0x13,0x2c,0xd4,0x24,0x9c,0xf2,
0xa8,0xac,0xf2,0xae,0x8a,0x5a,0x48,0x0a,0xee,0x27,
0xe7,0x42,0x0f,0x4d,0x2e,0xd9,0x43,0xec,0x06,0xf0,
0x68,0x85,0x72,0xb8,0xe6,0x8d,0xb0,0xd0,0x26,0xd3,
0x8d,0xeb,0x71,0x79,0x60,0x1b,0xa0,0xa8,0x52,0x1a,
0x8b,0xe7,0x67,0xf2,0x1f,0x92,0xe5,0xe8,0x0c,0xf2,
0x1d,0xdf,0x73,0x72,0xa0,0x51,0x08,0x65,0xf8,0x72,
0x80,0xe3,0xe5,0x19,0xd7,0x35,0x25,0x19,0xb7,0x48,
0x69,0x85,0x08,0xab,0x73,0x2f,0x80,0x12,0x66,0xba,
0xb7,0x48,0x57,0xad,0x0e,0xb5,0x40,0x4a,0x6c,0x45,
0x36,0x17,0x32,0xca,0x1e,0xf3,0x56,0xd3,0x3f,0x0b,
0x8d,0xe2,0xec,0x11,0xfa,0x0e,0xa2,0x91,0xbd,0x0a,
0x54,0x3e,0xdc,0x89,0x42,0x10,0xf4,0x7d,0x1d,0x9c,
0x8b,0x2a,0xcc,0xf0,0x85,0x8d,0x8a,0xdd,0xa0,0x98,
0x4d,0xfc,0x59,0xaf,0x97,0xeb,0xb8,0x71,0xa8,0xe2,
0x5e,0xc3,0x33,0x4b,0x50,0x1d,0x87,0x69,0xdb,0x88,
0x62,0x2d,0x94,0xdf,0xbc,0xde,0x3d,0x3c,0xda,0x28,
0x28,0x8f,0x05,0x2a,0x50,0xf1,0x7f,0xad,0x51,0x07,
0x51,0x97,0x67,0x96,0x5f,0x29,0x9f,0xb6,0x6e,0x4a,
0x76,0x1f,0x14,0x4b,0xf6,0xfb,0xa1,0xe9,0x1e,0x72,
0x1b,0x0d,0x19,0x94,0x39,0x76,0x22,0xf8,0x1a,0x60,
0x10,0xca,0x89,0xce,0x56,0xe2,0x94,0x09,0x14,0xd5,
0x56,0x6d,0x72,0x15,0x92,0x35,0x5b,0x62,0xb5,0x49,
0x63,0xd7,0xb3,0x36,0xdc,0x4a,0xdb,0xf2,0xfb,0x9f,
0x35,0x73,0xfc,0x06,0xd6,0xd6,0x27,0x16,0x69,0x88,
0xb7,0xc6,0xbd,0x3a,0xfa,0x4d,0x1c,0x93,0xa9,0x40,
0xcc,0xde,0x46,0x21,0x68,0x37,0x18,0xff,0x76,0xdc,
0x05,0x4c,0xfa,0x48,0x5c,0x4d,0xdb,0xa5
};
/*computed magic ECC = 0xf3 0xf3 0xf3 */
u_int8 magic_page_4[528] = 
{
0x4e,0xd4,0x73,0xb2,0xdc,0xb9,0x10,0x87,0xa7,0x0e,
0x52,0x01,0x09,0xd4,0x26,0xb0,0x02,0xba,0x55,0x1d,
0x99,0x0b,0x83,0x6b,0xdc,0xf9,0x5a,0xa5,0x6e,0x0d,
0x2f,0x61,0x8f,0xc5,0xe9,0x3a,0x10,0x76,0x96,0x6c,
0x8b,0x43,0xb3,0xed,0x68,0xbe,0x50,0xe5,0x80,0xc5,
0x06,0xe0,0xdd,0xcf,0x48,0x13,0x78,0x07,0xb7,0x14,
0x66,0xd0,0xdb,0x59,0xdf,0x87,0x78,0x6a,0x0c,0x26,
0xe5,0x8a,0xff,0x20,0x98,0x52,0x59,0x1b,0x42,0x73,
0x66,0x4a,0x3f,0x29,0x3f,0xb8,0x34,0x0f,0xc1,0x90,
0x0d,0x4e,0xe4,0x59,0xc7,0x73,0x2c,0xb0,0xb5,0x58,
0x67,0xde,0x9f,0x62,0x5d,0xf1,0x02,0x0f,0x91,0x20,
0x32,0x95,0x5c,0xa8,0xc4,0x85,0x73,0x01,0xf3,0x36,
0x14,0xe7,0x47,0x6a,0xfb,0xc3,0xf7,0x01,0xde,0xe4,
0x95,0x09,0xf8,0x81,0x00,0x20,0x00,0x0e,0xc8,0xf4,
0x05,0x52,0x72,0x35,0x0c,0xc8,0x3b,0x6e,0x6d,0x30,
0xd0,0x8a,0x44,0x69,0x31,0xef,0x00,0xfa,0x8d,0x80,
0x5d,0x56,0x69,0xeb,0xcf,0xba,0xdc,0x20,0x3b,0x58,
0x25,0xbd,0x65,0x03,0xf2,0xe6,0x98,0xa0,0x9e,0xc9,
0xdc,0x99,0xb1,0x3d,0xa5,0xff,0xed,0xcd,0xff,0x36,
0xc3,0x9b,0x07,0xbe,0xdb,0x5c,0xba,0x83,0x2a,0xe3,
0x5d,0x3e,0x2a,0x10,0x9c,0x4f,0xbd,0xe9,0x21,0x34,
0xca,0x6a,0x2c,0x5c,0x97,0x28,0x84,0x9d,0x7f,0xe0,
0xc1,0x75,0x73,0xa8,0xbc,0x66,0x75,0x23,0x41,0x24,
0xf7,0x9e,0xb4,0xbc,0x47,0x7e,0xd3,0x1f,0x78,0xaa,
0xc0,0x0f,0xf3,0x8b,0x82,0xe5,0xa0,0x52,0x2e,0xc4,
0x43,0x73,0x4d,0x0a,0x12,0x2b,0xd3,0x23,0x9b,0xf1,
0xa7,0xab,0xf1,0xad,0x89,0x59,0x47,0x09,0xed,0x26,
0xe6,0x41,0x0e,0x4c,0x2d,0xd8,0x42,0xeb,0x05,0xef,
0x67,0x84,0x71,0xb7,0xe5,0x8c,0xaf,0xcf,0x25,0xd2,
0x8c,0xea,0x70,0x78,0x5f,0x1a,0x9f,0xa7,0x51,0x19,
0x8a,0xe6,0x66,0xf1,0x1e,0x91,0xe4,0xe7,0x0b,0xf1,
0x1c,0xde,0x72,0x71,0x9f,0x50,0x07,0x64,0xf7,0x71,
0x7f,0xe2,0xe4,0x18,0xd6,0x34,0x24,0x18,0xb6,0x47,
0x68,0x84,0x07,0xaa,0x72,0x2e,0x7f,0x11,0x65,0xb9,
0xb6,0x47,0x56,0xac,0x0d,0xb4,0x3f,0x49,0x6b,0x44,
0x35,0x16,0x31,0xc9,0x1d,0xf2,0x55,0xd2,0x3e,0x0a,
0x8c,0xe1,0xeb,0x10,0xf9,0x0d,0xa1,0x90,0xbc,0x09,
0x53,0x3d,0xdb,0x88,0x41,0x0f,0xf3,0x7c,0x1c,0x9b,
0x8a,0x29,0xcb,0xef,0x84,0x8c,0x89,0xdc,0x9f,0x97,
0x4c,0xfb,0x58,0xae,0x96,0xea,0xb7,0x70,0xa7,0xe1,
0x5d,0xc2,0x32,0x4a,0x4f,0x1c,0x86,0x68,0xda,0x87,
0x61,0x2c,0x93,0xde,0xbb,0xdd,0x3c,0x3b,0xd9,0x27,
0x27,0x8e,0x04,0x29,0x4f,0xf0,0x7e,0xac,0x50,0x06,
0x50,0x96,0x66,0x95,0x5e,0x28,0x9e,0xb5,0x6d,0x49,
0x75,0x1e,0x13,0x4a,0xf5,0xfa,0xa0,0xe8,0x1d,0x71,
0x1a,0x0c,0x18,0x93,0x38,0x75,0x21,0xf7,0x19,0x5f,
0x0f,0xc9,0x88,0xcd,0x55,0xe1,0x93,0x08,0x13,0xd4,
0x55,0x6c,0x71,0x14,0x91,0x34,0x5a,0x61,0xb4,0x48,
0x62,0xd6,0xb2,0x35,0xdb,0x49,0xda,0xf1,0xfa,0x9e,
0x34,0x72,0xfb,0x05,0xd5,0xd5,0x26,0x15,0x68,0x87,
0xb6,0xc5,0xbc,0x39,0xf9,0x4c,0x1b,0x92,0xa8,0x3f,
0xcb,0xdd,0x45,0x20,0x67,0x36,0x17,0xff,0x75,0xdb,
0x04,0x4b,0xf9,0x47,0x5b,0x4c,0xda,0xa4
};


 /* computed magic ECC = 0x5a 0x66 0x65 */
#if defined(DLOAD) 


void nand_flash_unit_test(void)
{
//   u_int32 invalidblockcount = 0;
   extern int CurrentBank;
   u_int16 deviceid = 0;
//   u_int8  BBytefrompage0;
//   u_int32 pageindex = 0;
//   u_int32 block = 0;
//   u_int32 BByteIndex = 5;
//   u_int32 address = 0;
//   u_int8 page[528];
//   u_int32 i = 0;
   
   u_int32 pagesize = 0;
   u_int32 blocksize = 0;
//   u_int16 ecc_gen[3];

   CurrentBank = 1;

   pagesize = Bank[CurrentBank].pNandFlashDesc->NandFlashPageSize;
   blocksize = pagesize *
                       Bank[CurrentBank].pNandFlashDesc->NandFlashPagesPerBlock;

   printf("Entering nand_flash_unit_test=============\n\n");

   read_ID_sam_nandflash(&deviceid);
   printf("read_ID_sam_nandflash: ID = [0x%X]\n", deviceid);





#if 0
   /* Construct a magic page */
   FFillBytes(magic_page, 0xFF, 528);
   for(i=0; i<528; i++)
   {
      magic_page[i] = ((address+i)>>((i & 3) * 8)) & 0xff;
   }
   magic_page[517] = 0xFF; /* BB Byte */
#endif


#if 0
   /* Contiguos read across blocks */
//   srand( (unsigned)time( NULL ) );      // Random Fuction

   erase_samnand(1, NULL);


   magic_page_1[517] = 0xFF; /* BB Byte */
   make_ecc_512Byte(magic_page_1, ecc_gen);          
   magic_page_1[518] = (u_int8)ecc_gen[0];
   magic_page_1[519] = (u_int8)ecc_gen[1];
   magic_page_1[520] = (u_int8)ecc_gen[2];
   printf("magic = 0x%x 0x%x 0x%x\n", magic_page_1[518], magic_page_1[519], magic_page_1[520]);

   pageindex = 0;
   block = 1;
   address = (block * blocksize) + (pageindex*pagesize);

   write_page_sam_nandflash(address, (void*)magic_page_1, (void*)&magic_page_1[512]);
   read_page_sam_nandflash(address, page);
   dump_data(page, 528);
   
   erase_samnand(2, NULL);
   pageindex = 0;
   block = 2;
   address = (block * blocksize) + (pageindex*pagesize);

   magic_page_2[517] = 0xFF; /* BB Byte */
   make_ecc_512Byte(magic_page_2, ecc_gen);          
   magic_page_2[518] = (u_int8)ecc_gen[0];
   magic_page_2[519] = (u_int8)ecc_gen[1];
   magic_page_2[520] = (u_int8)ecc_gen[2];
   printf("magic = 0x%x 0x%x 0x%x\n", magic_page_2[518], magic_page_2[519], magic_page_2[520]);

   write_page_sam_nandflash(address, magic_page_2, &magic_page_2[512]);
   read_page_sam_nandflash(address, page);
   dump_data(page, 528);

   /* Partial block and Partial page test */
   pageindex = 1;
   block = 2;
   address = (block * blocksize) + (pageindex*pagesize);

   magic_page_3[517] = 0xFF; /* BB Byte */
   make_ecc_512Byte(magic_page_3, ecc_gen);          
   magic_page_3[518] = (u_int8)ecc_gen[0];
   magic_page_3[519] = (u_int8)ecc_gen[1];
   magic_page_3[520] = (u_int8)ecc_gen[2];
   printf("magic = 0x%x 0x%x 0x%x\n", magic_page_3[518], magic_page_3[519], magic_page_3[520]);

   write_page_sam_nandflash(address, magic_page_3, &magic_page_3[512]);
   read_page_sam_nandflash(address, page);
   dump_data(page, 528);
#endif
#if 0
   {
      /* Write the pages using buffer */
      #define MAX_SIZE  (512 *16 + 512+512)
      u_int8 temp_write[MAX_SIZE]; /* pagesize*pagesperblock*numofblocks */
      u_int8 temp_read[MAX_SIZE]; /* pagesize*pagesperblock*numofblocks */
      FFillBytes((u_int8*)temp_write, 0xFF, MAX_SIZE);
      FFillBytes((u_int8*)temp_read, 0xFF, MAX_SIZE);

      FCopy(temp_write, magic_page_1, 512);
      FCopy(temp_write+(512*16), magic_page_2, 512);
      FCopy(temp_write+(512*17), magic_page_3, 512);

      write_sam_nandflash(0, temp_write, MAX_SIZE-312);

   {
      printf("Reading block 1 and block 2 data......... \n");
      read_sam_nandflash(0, temp_read, MAX_SIZE-312);
      dump_data(temp_read, 512);
      dump_data(temp_read+(512*16), 512);       
      dump_data(temp_read+(512*17), 512);       
   }

   }
#endif

   printf("Exiting nand_flash_unit_test=============\n\n");
    
}
#endif

#if 0
   {
      volatile u_int32* gpioreadaddr = (volatile u_int32*)0x30470064;
      if ( ( (*gpioreadaddr >> (89%32)) & 0x01) == 1) 
      {
         temp++;
      }
   }
#endif

#endif /* SAM_NAND_UNIT_TEST */

/****************************************************************************
 * $Log: 
 *  4    mpeg      1.3         5/4/04 4:57:46 PM      Sunil Cheruvu   CR(s) 
 *        9092 9093 : The particular bad block in question is  marked as bad 
 *        and the flash write operation will continue with the next available 
 *        valid/good block.
 *  3    mpeg      1.2         5/4/04 3:41:54 PM      Sunil Cheruvu   CR(s) 
 *        9014 9015 : Added code to use the hw timer1 in the case of Download &
 *         Codeldr.  And in other cases the hw timer functions are used to get 
 *        and set the timer.
 *  2    mpeg      1.1         4/23/04 7:24:43 PM     Sunil Cheruvu   CR(s) 
 *        8870 8871 : Fix the DL_SER ADS and SDT build breaks
 *  1    mpeg      1.0         4/22/04 4:17:11 PM     Sunil Cheruvu   CR(s) 
 *        8870 8871 : Added the Nand Flash support for Wabash(Milano rev 5 and 
 *        above) and Brazos.
 * $
 ****************************************************************************/

