/****************************************************************************/
/*                   CONEXANT PROPRIETARY AND CONFIDENTIAL                  */
/*                       SOFTWARE FILE/MODULE HEADER                        */
/*          Conexant Systems Inc. (c) 1998, 1999, 2000, 2001, 2002          */
/*                              Austin, TX                                  */
/*                         All Rights Reserved                              */
/****************************************************************************/
/*                                                                          */
/* Filename:       HEAP.C                                                   */
/*                                                                          */
/* Description:    Simple memory heap manager                               */
/*                                                                          */
/* Author:         Dave Wilson                                              */
/*                                                                          */
/*                                                                          */
/****************************************************************************/
/* $Header: cnxtheap.c, 7, 8/13/03 12:56:40 PM, Dave Wilson$
 ****************************************************************************/

/*****************/
/* Include Files */
/*****************/
#include "hwconfig.h"
#include "retcodes.h"
#include "kal.h"
#include <string.h>
#include "cnxtheap.h"
#include "trace.h"

/******************************************/
/* Local Definitions and Global Variables */
/******************************************/
typedef struct _heap_header
{
  u_int32 uMarker;
  size_t  sizeHeap;
  sem_id_t semHeap;
} heap_header;

typedef struct _heap_block
{
  struct _heap_block *pNext;
  size_t       sizeBlock;
  bool         bUsed;
} heap_block;

#define HEAP_MARKER  0xFAB0ABBA
#ifdef DEBUG
#define FREE_MARKER  0xFE
#define ALLOC_MARKER 0xAA
#endif

extern int GetCPUType(void);

/********************************/       
/* Internal Function Prototypes */
/********************************/

bool   heap_tidy(heap_block *lpHeap);

/**********************/
/* Exported Functions */
/**********************/

/********************************************************************/
/*  FUNCTION:    heap_create                                        */
/*                                                                  */
/*  PARAMETERS:  pHeap    - pointer to the block of memory to be    */
/*                          managed as a heap.                      */
/*               sizeHeap - size of the memory block                */
/*                                                                  */
/*  DESCRIPTION: Initialise a block of memory with control blocks   */
/*               allowing it to be managed as a heap.               */
/*                                                                  */
/*  RETURNS:     A handle to the heap if successfull, NULL if error */
/********************************************************************/
heap_handle heap_create(void *pHeap, size_t sizeHeap)
{
  heap_header *pHeader;
  heap_block  *pBottom;
  heap_block  *pTop;
  char         cName[5];
  static int   uHeapNum = 0;  
  
  /* Look for silly sized heaps and reject the request */
  if(sizeHeap < (sizeof(heap_header) + 2*sizeof(heap_block)))
    return((heap_handle)NULL);

  
  /* Create block markers at the top and bottom of the heap */
  pHeader = (heap_header *)pHeap;
  pBottom = (heap_block *)(pHeader+1);
  pTop    = (heap_block *)((unsigned char *)pHeap + (sizeHeap - sizeof(heap_block)));
  
  pHeader->uMarker  = HEAP_MARKER;
  pHeader->sizeHeap = sizeHeap - sizeof(heap_header);
  
  pBottom->pNext = pTop;
  pTop->pNext    = NULL;
  
  pBottom->bUsed = FALSE;
  pTop->bUsed    = TRUE; /* This is important! */
  
  pBottom->sizeBlock = pHeader->sizeHeap - 2*sizeof(heap_block);
  pTop->sizeBlock    = 0;

  #ifdef DEBUG
  /* Fill the free block of memory with a marker */
  memset((pBottom+1), FREE_MARKER, pBottom->sizeBlock);
  #endif

  /* Create the semaphore used to control access to the heap */
  cName[0] = 'H';
  cName[1] = 'S';
  cName[2] = '0'+uHeapNum/10;
  cName[3] = '0'+uHeapNum%10;
  cName[4] = 0;
  uHeapNum++;
  pHeader->semHeap = sem_create(1, cName);
  if(pHeader->semHeap == (sem_id_t)0)
    return((heap_handle)NULL);
  else
    return((heap_handle)pHeap);
}

/********************************************************************/
/*  FUNCTION:    heap_destroy                                       */
/*                                                                  */
/*  PARAMETERS:  hHeap    - handle to the heap which is to be       */
/*                          destroyed.                              */
/*                                                                  */
/*  DESCRIPTION: Destroy a heap, freeing all allocations in the     */
/*               heap and resources allocated to manage it. Note    */
/*               that the caller is responsible for freeing the     */
/*               memory block comprising the heap itself.           */
/*                                                                  */
/*  RETURNS:     TRUE on success, FALSE otherwise.                  */
/********************************************************************/
bool heap_destroy(heap_handle hHeap)
{
  int iRetcode;
  heap_header *pHeap;
  
  pHeap = (heap_header *)hHeap;
  
  /* Is this a valid heap? */
  if (pHeap->uMarker != HEAP_MARKER)
    return(FALSE);

  /* Grab the access semaphore */
  iRetcode = sem_get(pHeap->semHeap, KAL_WAIT_FOREVER);
  if (iRetcode == RC_OK)
  {
    /* Mark the heap as invalid */
    pHeap->uMarker = 0;
    
    /* Destroy the semaphore */
    sem_delete(pHeap->semHeap);
    
    return(TRUE);
  }
  else
    return(FALSE);
}

/********************************************************************/
/*  FUNCTION:    heap_alloc                                         */
/*                                                                  */
/*  PARAMETERS:  hHeap    - handle to the heap from which the       */
/*                          allocation is being requested.          */
/*               sizeAlloc - the size of the block to allocate      */
/*                                                                  */
/*  DESCRIPTION: Allocate a block of memory from a heap which has   */
/*               previously been initialised.                       */
/*                                                                  */
/*  RETURNS:     A valid memory pointer if successful, NULL on error*/
/********************************************************************/
void *heap_alloc(heap_handle hHeap, size_t sizeAlloc)
{
  heap_block  *pBlock;
  heap_block  *pNew;
  heap_header *pHeap;
  void *pReturn = NULL;
  int  iRetcode;
  
  pHeap = (heap_header *)hHeap;
  
  /* Is this a valid heap? */
  if (pHeap->uMarker != HEAP_MARKER)
    return(NULL);

  /* Round the size up to the next multiple of 4 bytes */
  sizeAlloc = (sizeAlloc+3) & 0xFFFFFFFC;

  /* Grab the access semaphore */
  iRetcode = sem_get(pHeap->semHeap, KAL_WAIT_FOREVER);
  if (iRetcode == RC_OK)
  {
    /* Find a free block large enough to satisfy this request */
    pBlock = (heap_block *)(pHeap+1);
  
    while (pBlock && !pReturn)
    {
      if (!pBlock->bUsed && (pBlock->sizeBlock >= sizeAlloc))
      {
        /* Case 1 - the block is large enough that we split it in 2 */
        if(pBlock->sizeBlock > (sizeAlloc+sizeof(heap_block)))
        {
          /* We found a free block big enough */
          pNew   = (heap_block *)((char *)pBlock + sizeof(heap_block) + sizeAlloc);
          pNew->bUsed     = FALSE;
          pNew->pNext     = pBlock->pNext;
          pNew->sizeBlock = pBlock->sizeBlock - (sizeof(heap_block) + sizeAlloc);
      
          pBlock->pNext   = pNew;
          pBlock->sizeBlock = sizeAlloc;
          pBlock->bUsed   = TRUE;
        }
        else
        {
          /* Use the existing block and leave some space at the end */
          pBlock->bUsed = TRUE;
        }

        #ifdef DEBUG
        /* Fill the newly allocated block of memory with a marker */
        memset((pBlock+1), ALLOC_MARKER, pBlock->sizeBlock);
        heap_clean_and_flush_cache((void *)(pBlock+1), pBlock->sizeBlock);
        #endif
      
        pReturn = (void *)(pBlock+1);
      }
      else
      {
        pBlock = pBlock->pNext;
      }  
    }
    
    sem_put(pHeap->semHeap);
  }  
  
  return(pReturn);
}

/********************************************************************/
/*  FUNCTION:    heap_free                                          */
/*                                                                  */
/*  PARAMETERS:  hHeap    - handle to the heap to which a block     */
/*                          allocation is being returned.           */
/*               pBlock   - pointer to the block being returned to  */
/*                          the heap.                               */
/*                                                                  */
/*  DESCRIPTION: Return a block that was previously allocated using */
/*               a call to heap_alloc to the heap.                  */
/*                                                                  */
/*  RETURNS:     TRUE on success, FALSE on error.                   */
/********************************************************************/
bool heap_free(heap_handle hHeap, void *pBlock)
{
  heap_block *pHeapBlock;
  heap_header *pHeap;
  bool bRetcode = FALSE;
  int  iRetcode;
 
  pHeap = (heap_header *)hHeap;
  
  /* Is this a valid heap? */
  if (pHeap->uMarker != HEAP_MARKER)
    return(FALSE);
 
  iRetcode = sem_get(pHeap->semHeap, KAL_WAIT_FOREVER);
  if (iRetcode == RC_OK)
  {
    /* Walk the heap looking for the block that is being freed */
    pHeapBlock = (heap_block *)(pHeap+1);
    
    while (pHeapBlock)
    {
      if (pBlock == (void *)(pHeapBlock+1))
      {
        /* We found the block so free it up */
        pHeapBlock->bUsed = FALSE;
        bRetcode = TRUE;

        #ifdef DEBUG
        /* Fill the free block of memory with a marker */
        memset(pBlock, FREE_MARKER, pHeapBlock->sizeBlock);
        heap_clean_and_flush_cache((void *)(pHeapBlock+1), pHeapBlock->sizeBlock);
        #endif
        
        heap_tidy((heap_block *)(pHeap+1));
        
        break;
      }
      else
        pHeapBlock = pHeapBlock->pNext;
    }  
    
    sem_put(pHeap->semHeap);
  }  
  
  return(bRetcode);

}

/********************************************************************/
/*  FUNCTION:    heap_free_all                                      */
/*                                                                  */
/*  PARAMETERS:  hHeap    - handle to the heap which is to have     */
/*                          all allocated blocks freed.             */
/*                                                                  */
/*  DESCRIPTION: Free all allocated blocks in the heap whose pointer*/
/*               is passed to the function.                         */
/*                                                                  */
/*  RETURNS:     TRUE on success, FALSE on error.                   */
/********************************************************************/
bool heap_free_all(heap_handle hHeap)
{
  heap_block *pFirst;
  heap_block *pLast;
  heap_header *pHeader;
  int iRetcode;
  
  pHeader = (heap_header *)hHeap;
  
  /* Is this a valid heap? */
  if (pHeader->uMarker != HEAP_MARKER)
    return(FALSE);
    
  iRetcode = sem_get(pHeader->semHeap, KAL_WAIT_FOREVER);
  if (iRetcode == RC_OK)
  {
    pFirst = (heap_block *)(pHeader+1);
    pLast  = (heap_block *)((unsigned char *)pHeader + (pHeader->sizeHeap + sizeof(heap_header) - sizeof(heap_block)));
    
    pFirst->bUsed     = FALSE;
    pFirst->pNext     = pLast;
    pFirst->sizeBlock = pHeader->sizeHeap - 2*sizeof(heap_block);
    
    pLast->bUsed      = FALSE;
    pLast->pNext      = NULL;
    pLast->sizeBlock  = 0;
    
    #ifdef DEBUG
    /* Fill the free block of memory with a marker */
    memset((pFirst+1), FREE_MARKER, pFirst->sizeBlock);
    #endif
    
    sem_put(pHeader->semHeap);
    
    return(TRUE);
  }
  else
    return(FALSE);
}

/********************************************************************/
/*  FUNCTION:    heap_check                                         */
/*                                                                  */
/*  PARAMETERS:  hHeap    - handle to the heap which is to be       */
/*                          checked for a valid header.             */
/*                                                                  */
/*  DESCRIPTION: Check to see that the heap header has not been     */
/*               corrupted.                                         */
/*                                                                  */
/*  RETURNS:     TRUE if header is intact, FALSE if corrupted       */
/********************************************************************/
bool heap_check(heap_handle hHeap)
{
  heap_header *pHeader;
  
  pHeader = (heap_header *)hHeap;
  
  /* Is the heap header marker intact? */
  if (pHeader->uMarker != HEAP_MARKER)
    return(FALSE);
  else
    return(TRUE);  
}

/***************************************************************************/
/* FUNCTION:    heap_clean_and_flush_cache                                 */
/*                                                                         */
/* PARAMETERS:  pBlock    - Pointer to block of memory to clean and flush  */
/*              sizeBlock - Size of memory block                           */
/*                                                                         */
/* DESCRIPTION: Ensures that any data written to the passed buffer via the */
/*              cached address space is flushed out to real SDRAM thus     */
/*              ensuring that hardware accessing the buffer will use the   */
/*              expected data.                                             */
/*                                                                         */
/* RETURNS:     Nothing                                                    */
/***************************************************************************/
void heap_clean_and_flush_cache(void *pBlock, size_t sizeBlock)
{
   /* This function is only useful if using the cache */
  #if MMU_CACHE_DISABLE == 0

     /* Don't do anything on the ARM940T core */
    #if CPU_TYPE == AUTOSENSE || CPU_TYPE == CPU_ARM920T

      /* If Brazos, ensure not Rev_A (940 core) */
      #if CPU_TYPE == AUTOSENSE
        if (GetCPUType() == CPU_ARM920T)
      #endif /* #if CPU_TYPE == AUTOSENSE */
        {
            /* Finally, flush and invalidate the buffer area */
            FlushAndInvalDCacheRegion ((u_int8 *)pBlock, sizeBlock);
        }
    #endif /* CPU_TYPE == AUTOSENSE || CPU_TYPE == CPU_ARM920T */

  #endif /* MMU_CACHE_DISABLE == 0 */
}

/**********************/
/* Internal Functions */
/**********************/

/********************************************************************/
/*  FUNCTION:    heap_tidy                                          */
/*                                                                  */
/*  PARAMETERS:  pBlock   - pointer to the first heap which is to be*/
/*                          tidied                                  */
/*                                                                  */
/*  DESCRIPTION: Walk the blocks in the heap and amalgamate any     */
/*               adjacent free blocks.                              */
/*                                                                  */
/*  RETURNS:     TRUE on success, FALSE on error.                   */
/********************************************************************/
bool heap_tidy(heap_block *pBlock)
{
  heap_block *pAnchor = pBlock;
  heap_block *pTemp;
  
  /* While we are not at the end of the list */
  while (pAnchor)
  {
    /* and this block is marked as free */
    if (!pAnchor->bUsed)
    {
      /* and the next block is not the end of the list */
      if (pAnchor->pNext)
      {
        /* and it is also marked as free */
        if (!pAnchor->pNext->bUsed)
        {
          /* then merge the 2 blocks into one */
          pTemp = pAnchor->pNext;
          pAnchor->sizeBlock = pAnchor->sizeBlock + pTemp->sizeBlock + sizeof(heap_block);
          pAnchor->pNext     = pTemp->pNext;
          #ifdef DEBUG
          memset(pTemp, FREE_MARKER, sizeof(heap_block));
          #endif
        }
        else
        {
          /* The next block is not free so move on in the list */
          pAnchor = pAnchor->pNext;
        }
      }
    }
    else
    {
      /* Block is used, move on to the next one */
      pAnchor = pAnchor->pNext;
    }  
  }  
  
  return(TRUE);
}

#ifdef DEBUG

/********************************************************************/
/*  FUNCTION:    heap_dump                                          */
/*                                                                  */
/*  PARAMETERS:  hHeap - Handle to the heap whose contents are to   */
/*                       be dumped.                                 */
/*                                                                  */
/*  DESCRIPTION:                                                    */
/*                                                                  */
/*  RETURNS:     Nothing                                            */
/*                                                                  */
/*  CONTEXT:     Must be called from a non-interrupt context        */
/*                                                                  */
/********************************************************************/
#define HEAP_TRACE (TRACE_CTL|TRACE_LEVEL_ALWAYS)

void heap_dump(heap_handle hHeap)
{
  heap_header *pHeader;
  heap_block  *pBlock;
  bool        bCorrupt;
  u_int32     uLargestFree = 0;
  u_int32     uTotalFree   = 0;
  int         iLoop        = 0;
  int         iRetcode;
  
  trace_new(HEAP_TRACE, "Request to dump heap at 0x%08x\n", hHeap);
  trace_new(HEAP_TRACE, "----------------------------------\n", hHeap);
  
  pHeader = (heap_header *)hHeap;
  
  /* Is the heap header marker intact? */
  if (pHeader->uMarker != HEAP_MARKER)
  {
    trace_new(HEAP_TRACE, "Heap is corrupt - marker value is wrong!\n");
  }
  else
  {
    trace_new(HEAP_TRACE, "Heap size is %d (0x%x) bytes\n\n", pHeader->sizeHeap);
    
    /* Get the heap semaphore */
    iRetcode = sem_get(pHeader->semHeap, KAL_WAIT_FOREVER);
    if (iRetcode == RC_OK)
    {
      trace_new(HEAP_TRACE, "Block dump:\n");
      
      pBlock = (heap_block *)(pHeader+1);
      iLoop  = 0;
      
      /* Walk the heap dumping block info */
      while (pBlock)
      {
        /* Make sure the block pointer is valid */
        if ((pBlock > (heap_block *)pHeader) && 
           ((char *)pBlock < (char *)(pHeader+1)+pHeader->sizeHeap))
        {
          trace_new(HEAP_TRACE, 
                    "%2d: %s block at 0x%08x, size %d (0x%0x)\n",
                    iLoop++,
                    pBlock->bUsed ? "USED" : "FREE",
                    pBlock,
                    pBlock->sizeBlock,
                    pBlock->sizeBlock);
                
          /* Update some statistics */          
          if (!pBlock->bUsed)
          {
            uTotalFree += pBlock->sizeBlock;
            if(pBlock->sizeBlock > uLargestFree)
              uLargestFree = pBlock->sizeBlock;
          }  
          
          pBlock = pBlock->pNext;
        }
        else
        {
          trace_new(HEAP_TRACE, "Heap is corrupt - invalid block pointer 0x%08x!\n", pBlock);
          bCorrupt = TRUE;
          break;
        }
      }  
      
      if (!bCorrupt)
      {
        trace_new(HEAP_TRACE, "Total free space in heap %d (0x%x) bytes\n",
            uTotalFree, uTotalFree);
            
        trace_new(HEAP_TRACE, "Largest free block %d (0x%x) bytes\n",
            uLargestFree, uLargestFree);
      }
      
      /* Release the semaphore */
      sem_put(pHeader->semHeap);
    }  
  }
  
  trace_new(HEAP_TRACE, "End of heap dump\n\n");
}
#endif

/****************************************************************************
 * Modifications:
 * $Log: 
 *  7    mpeg      1.6         8/13/03 12:56:40 PM    Dave Wilson     SCR(s): 
 *        7253 7252 
 *        Fixed an error in heap_dump which would cause a heap corruption to be
 *         
 *        detected if the heap only contained 1 allocated block which was the 
 *        whole
 *        size of the heap.
 *        
 *  6    mpeg      1.5         7/18/03 3:31:12 PM     Dave Wilson     SCR(s) 
 *        6997 :
 *        Added prototype for GetCPUType to get rid of a compiler warning.
 *        
 *  5    mpeg      1.4         7/15/03 1:47:50 PM     Dave Wilson     SCR(s) 
 *        6898 :
 *        Added heap_clean_and_flush_cache function and used it to ensure that 
 *        markers
 *        written to free and allocated blocks in debug builds are flushed 
 *        before
 *        the heap_alloc and heap_free calls return. Previously, this code had 
 *        been
 *        seen to cause some minor image corruption when the buffers were 
 *        allocated
 *        for use by the video decoder or GXA.
 *        
 *  4    mpeg      1.3         9/6/02 4:14:08 PM      Matt Korte      SCR(s) 
 *        4498 :
 *        Fixed Warnings
 *        
 *  3    mpeg      1.2         4/20/01 12:16:22 PM    Dave Wilson     DCS1124: 
 *        Additions for debug of new video memory management code
 *        
 *  2    mpeg      1.1         12/11/00 10:20:12 AM   Miles Bintz     changed 
 *        include "heap.h" to include "cnxtheap.h"
 *        
 *  1    mpeg      1.0         12/11/00 10:18:12 AM   Miles Bintz     
 * $
 * 
 *    Rev 1.6   13 Aug 2003 11:56:40   dawilson
 * SCR(s): 7253 7252 
 * Fixed an error in heap_dump which would cause a heap corruption to be 
 * detected if the heap only contained 1 allocated block which was the whole
 * size of the heap.
 * 
 *    Rev 1.5   18 Jul 2003 14:31:12   dawilson
 * SCR(s) 6997 :
 * Added prototype for GetCPUType to get rid of a compiler warning.
 * 
 *    Rev 1.4   15 Jul 2003 12:47:50   dawilson
 * SCR(s) 6898 :
 * Added heap_clean_and_flush_cache function and used it to ensure that markers
 * written to free and allocated blocks in debug builds are flushed before
 * the heap_alloc and heap_free calls return. Previously, this code had been
 * seen to cause some minor image corruption when the buffers were allocated
 * for use by the video decoder or GXA.
 * 
 *    Rev 1.3   06 Sep 2002 15:14:08   kortemw
 * SCR(s) 4498 :
 * Fixed Warnings
 * 
 *    Rev 1.2   20 Apr 2001 11:16:22   dawilson
 * DCS1124: Additions for debug of new video memory management code
 * 
 *    Rev 1.1   11 Dec 2000 10:20:12   bintzmf
 * changed include "heap.h" to include "cnxtheap.h"
 * 
 *    Rev 1.0   11 Dec 2000 10:18:12   bintzmf
 * Initial revision.
 * 
 *    Rev 1.3   20 Nov 2000 12:11:02   dawilson
 * Ensured that allocations are always multiples of 4 bytes.
 * 
 *    Rev 1.2   15 Nov 2000 15:22:08   dawilson
 * Fixed most the most blatant bugs
 * 
 *    Rev 1.1   15 Nov 2000 11:28:08   dawilson
 * Changed API to use handles rather than plain pointers to the heap
 * 
 *    Rev 1.0   15 Nov 2000 09:06:26   dawilson
 * Initial revision.
 *
 ****************************************************************************/

