/****************************************************************************/
/*                   CONEXANT PROPRIETARY AND CONFIDENTIAL                  */
/*                        SOFTWARE FILE/MODULE HEADER                       */
/*                 Copyright Conexant Systems Inc. 1998-2003                */
/*                                Austin, TX                                */
/*                            All Rights Reserved                           */
/****************************************************************************/
/*
 * Filename:        handle.c
 *
 *
 * Description:     Utilities for handle processing
 *
 *
 * Author:          Larry
 *
 ****************************************************************************/
/* $Header: handle.c, 5, 9/23/03 3:15:18 PM, Billy Jackman$
 ****************************************************************************/

/*****************/
/* Include Files */
/*****************/
#include "basetype.h"
#include "handle.h"
#include "kal.h"

/*********************/
/* Utility functions */
/*********************/

/****************************************************************************/
/*  FUNCTION:    cnxt_handle_add                                            */
/*                                                                          */
/*  DESCRIPTION: This function adds the specified instance to the end of    */
/*               the specified instance list, checking along the way to     */
/*               make sure the instance list does not already contain the   */
/*               specified instance.                                        */
/*                                                                          */
/*  INPUTS:      ppFirstInst - the address of the variable that holds the   */
/*                  pointer to the instance list.                           */
/*               pInst - the address of the instance to be added to the     */
/*                  instance list.                                          */
/*                                                                          */
/*  OUTPUTS:     None.                                                      */
/*                                                                          */
/*  RETURNS:     TRUE  - Success.                                           */
/*               FALSE - The list is corrupted.                             */
/*                                                                          */
/*  NOTES:       None.                                                      */
/*                                                                          */
/*  CONTEXT:     This handle function may only be called from task context. */
/*                                                                          */
/****************************************************************************/
bool cnxt_handle_add ( void **ppFirstInst, void *pInst )
{
   CNXT_HANDLE_PREFACE *pInstList;

   pInstList = (CNXT_HANDLE_PREFACE*)*ppFirstInst;

   /* If the list is currently empty, insert the new instance at the head. */
   if ( *ppFirstInst == NULL )
   {
      ((CNXT_HANDLE_PREFACE*)pInst)->pNext = NULL;
      *ppFirstInst = pInst;
      return TRUE;
   }

   /* insert pInst at the tail, make sure pInst is not in the list */
   while ( 1 )
   {
      /* check for list corruption */
      if ( pInstList != pInstList->pSelf )
      {
         return FALSE;
      }

      if ( pInstList == (CNXT_HANDLE_PREFACE*)pInst )
      {
         /* pInst is in the list */
         return FALSE;
      }

      if ( pInstList->pNext )
      {
         /* walk to the next */
         pInstList = pInstList->pNext;
      }
      else
      {
         /* we are at the end */
         ((CNXT_HANDLE_PREFACE*)pInst)->pNext = NULL;
         pInstList->pNext = (CNXT_HANDLE_PREFACE*)pInst;
         return TRUE;
      }
   }
}

/****************************************************************************/
/*  FUNCTION:    cnxt_handle_enqueue                                        */
/*                                                                          */
/*  DESCRIPTION: This function adds the specified instance to the end of    */
/*               the specified instance list, checking along the way to     */
/*               make sure the instance list does not already contain the   */
/*               specified instance.                                        */
/*                                                                          */
/*  INPUTS:      ppFirstInst - the address of the variable that holds the   */
/*                  pointer to the instance list.                           */
/*               pInst - the address of the instance to be added to the     */
/*                  instance list.                                          */
/*                                                                          */
/*  OUTPUTS:     None.                                                      */
/*                                                                          */
/*  RETURNS:     TRUE  - Success.                                           */
/*               FALSE - The instance was already in the list.              */
/*                                                                          */
/*  NOTES:       None.                                                      */
/*                                                                          */
/*  CONTEXT:     This handle function may only be called from task context. */
/*                                                                          */
/****************************************************************************/
bool cnxt_handle_enqueue ( void **ppFirstInst, void *pInst )
{
   CNXT_HANDLE_PREFACE *pInstList;

   pInstList = (CNXT_HANDLE_PREFACE*)*ppFirstInst;

   /* If the list is currently empty, insert the new instance at the head. */
   if ( *ppFirstInst == NULL )
   {
      ((CNXT_HANDLE_PREFACE*)pInst)->pNext = NULL;
      *ppFirstInst = pInst;
      return TRUE;
   }

   /* insert pInst at the tail, make sure pInst is not in the list */
   while ( 1 )
   {
      if ( pInstList == (CNXT_HANDLE_PREFACE*)pInst )
      {
         /* pInst is in the list */
         return FALSE;
      }

      if ( pInstList->pNext )
      {
         /* walk to the next */
         pInstList = pInstList->pNext;
      }
      else
      {
         /* we are at the end */
         ((CNXT_HANDLE_PREFACE*)pInst)->pNext = NULL;
         pInstList->pNext = (CNXT_HANDLE_PREFACE*)pInst;
         return TRUE;
      }
   }
}

/****************************************************************************/
/*  FUNCTION:    cnxt_handle_remove                                         */
/*                                                                          */
/*  DESCRIPTION: This function removes the specified instance from the      */
/*               specified instance list, checking along the way to make    */
/*               make sure the instance list is not corrupted.              */
/*                                                                          */
/*  INPUTS:      ppFirstInst - the address of the variable that holds the   */
/*                  pointer to the instance list.                           */
/*               pInst - the address of the instance to be removed from the */
/*                  instance list.                                          */
/*                                                                          */
/*  OUTPUTS:     None.                                                      */
/*                                                                          */
/*  RETURNS:     TRUE  - Success.                                           */
/*               FALSE - The list is corrupted.                             */
/*                                                                          */
/*  NOTES:       None.                                                      */
/*                                                                          */
/*  CONTEXT:     This handle function may only be called from task context. */
/*                                                                          */
/****************************************************************************/
bool cnxt_handle_remove ( void **ppFirstInst, void *pInst )
{
   CNXT_HANDLE_PREFACE *pInstList;
   bool bKs;

   pInstList = (CNXT_HANDLE_PREFACE*)*ppFirstInst;

   /* If the list is empty, indicate failure. */
   if ( pInstList == NULL )
   {
      return FALSE;
   }

   /* If the requested instance is at the head, remove it. */
   if ( pInstList == (CNXT_HANDLE_PREFACE*)pInst )
   {
      bKs = critical_section_begin ();
      *ppFirstInst = pInstList->pNext;
      pInstList->pNext = NULL;
      critical_section_end ( bKs );
      return TRUE;
   }

   /* Search the instance, remove it if found. */
   while ( 1 )
   {
      /* check for list corruption */
      if ( pInstList != pInstList->pSelf )
      {
         return FALSE;
      }

      if ( pInstList->pNext )
      {
         if ( pInstList->pNext == (CNXT_HANDLE_PREFACE*)pInst )
         {
            /* found it, removing */
            bKs = critical_section_begin ();
            pInstList->pNext = ((CNXT_HANDLE_PREFACE*)pInst)->pNext;
            ((CNXT_HANDLE_PREFACE*)pInst)->pNext = NULL;
            critical_section_end (bKs);
            return TRUE;
         }
         /* walk to the next */
         pInstList = pInstList->pNext;
      }
      else
      {
         /* we are at the end, pInst not found */
         return FALSE;
      }
   }
}

/****************************************************************************/
/*  FUNCTION:    cnxt_handle_dequeue                                        */
/*                                                                          */
/*  DESCRIPTION: This function removes the first instance from the          */
/*               specified instance list and sets *ppInst to its address.   */
/*                                                                          */
/*  INPUTS:      ppFirstInst - the address of the variable that holds the   */
/*                  pointer to the instance list.                           */
/*                                                                          */
/*  OUTPUTS:     ppInst - storage for the address of the instance removed   */
/*                  from the instance list.                                 */
/*                                                                          */
/*  RETURNS:     TRUE  - Success.                                           */
/*               FALSE - The list is empty.                                 */
/*                                                                          */
/*  NOTES:       None.                                                      */
/*                                                                          */
/*  CONTEXT:     This handle function may only be called from task context. */
/*                                                                          */
/****************************************************************************/
bool cnxt_handle_dequeue ( void **ppFirstInst, void **ppInst )
{
   CNXT_HANDLE_PREFACE *pInstList;
   bool bKs;

   pInstList = (CNXT_HANDLE_PREFACE*)*ppFirstInst;

   /* If the list is empty, indicate failure. */
   if ( pInstList == NULL )
   {
      return FALSE;
   }

   /* Remove the head of the list. */
   bKs = critical_section_begin ();
   *ppFirstInst = pInstList->pNext;
   pInstList->pNext = NULL;
   *ppInst = pInstList;
   critical_section_end ( bKs );
   return TRUE;
}

#ifdef DEBUG
/****************************************************************************/
/*  FUNCTION:    cnxt_handle_exist                                          */
/*                                                                          */
/*  DESCRIPTION: This function checks an instance list to see if the        */
/*               specified instance is in the list.                         */
/*                                                                          */
/*  INPUTS:      ppFirstInst - the address of the variable that holds the   */
/*                  pointer to the instance list.                           */
/*               pInst - the address of the instance to check for.          */
/*                                                                          */
/*  RETURNS:     TRUE  - The instance was found.                            */
/*               FALSE - The instance was not found.                        */
/*                                                                          */
/*  NOTES:       None.                                                      */
/*                                                                          */
/*  CONTEXT:     This handle function may only be called from task context. */
/*                                                                          */
/****************************************************************************/
bool cnxt_handle_exist ( void **ppFirstInst, void *pInst )
{
   CNXT_HANDLE_PREFACE *pInstList;
   bool bKs;
   bool bExist = FALSE;

   if ( ( pInst == NULL ) || ( (CNXT_HANDLE_PREFACE*)pInst != ( (CNXT_HANDLE_PREFACE*)pInst )->pSelf ) )
   {
      /* pInst is not valid, can't be in the list */
      return FALSE;
   }

   bKs = critical_section_begin ();

   /* we got exclusive access */
   pInstList = (CNXT_HANDLE_PREFACE*)*ppFirstInst;

   /* search for pInst */
   while ( 1 )
   {
      if ( pInstList == NULL )
      {
         /* we are at the end, pInst not found */
         break;
      }

      if ( pInstList == (CNXT_HANDLE_PREFACE*)pInst )
      {
         /* found it */
         bExist = TRUE;
         break;
      }

      /* check for list corruption */
      if ( pInstList != pInstList->pSelf )
      {
         break;
      }

      /* work to the next */
      pInstList = pInstList->pNext;
   }

   critical_section_end ( bKs );
   return bExist;
}

#endif /* #ifdef DEBUG */

/****************************************************************************/
/*  FUNCTION:    cnxt_handle_get_type                                       */
/*                                                                          */
/*  DESCRIPTION: This function sets its pType parameter to the type of the  */
/*               hHandle parameter.                                         */
/*                                                                          */
/*  INPUTS:      hHandle - the handle to determine the type of.             */
/*               pType - the parameter to store the type in.                */
/*                                                                          */
/*  RETURNS:     TRUE  - Success.                                           */
/*               FALSE - Failure.                                           */
/*                                                                          */
/*  NOTES:       None.                                                      */
/*                                                                          */
/*  CONTEXT:     This handle function may only be called from task context. */
/*                                                                          */
/****************************************************************************/
bool cnxt_handle_get_type ( CNXT_HANDLE hHandle, CNXT_HANDLE_TYPE *pType )
{
   if ( hHandle == NULL )
   {
      return FALSE;
   }

   if ( pType == NULL )
   {
      return FALSE;
   }

   *pType = ((CNXT_HANDLE_PREFACE *)hHandle)->Type;
   return TRUE;
}

/****************************************************************************/
/*  FUNCTION:    cnxt_handle_get_physunit                                   */
/*                                                                          */
/*  DESCRIPTION: This function sets its puPhysUnit parameter to the         */
/*               physical unit designation of the hHandle parameter.        */
/*                                                                          */
/*  INPUTS:      hHandle - the handle to determine the physical unit of.    */
/*               puPhysUnit - the parameter to store the physical unit in.  */
/*                                                                          */
/*  RETURNS:     TRUE  - Success.                                           */
/*               FALSE - Failure.                                           */
/*                                                                          */
/*  NOTES:       None.                                                      */
/*                                                                          */
/*  CONTEXT:     This handle function may only be called from task context. */
/*                                                                          */
/****************************************************************************/
bool cnxt_handle_get_physunit ( CNXT_HANDLE hHandle, u_int32 *puPhysUnit )
{
   if ( hHandle == NULL )
   {
      return FALSE;
   }

   if ( puPhysUnit == NULL )
   {
      return FALSE;
   }

   *puPhysUnit = ((CNXT_HANDLE_PREFACE *)hHandle)->uPhysUnit;
   return TRUE;
}


/****************************************************************************
 * Modifications:
 * $Log: 
 *  5    mpeg      1.4         9/23/03 3:15:18 PM     Billy Jackman   SCR(s) 
 *        7527 :
 *        Add function cnxt_handle_enqueue to be used for adding handles back 
 *        onto a
 *        free list. cnxt_handle_enqueue skips the check of pSelf for each 
 *        instance it
 *        visits, avoiding an incorrect failure.
 *        
 *  4    mpeg      1.3         9/17/03 2:01:16 PM     Billy Jackman   SCR(s) 
 *        7482 :
 *        Updated for new Multi Instance Driver Guidelines
 *        
 *        
 *  3    mpeg      1.2         2/11/03 9:20:02 AM     Larry Wang      SCR(s) 
 *        5462 :
 *        Move cnxt_handle_add() and cnxt_handle_remove() out of #ifdef DEBUG 
 *        block.
 *        
 *  2    mpeg      1.1         1/30/03 8:15:32 AM     Larry Wang      SCR(s) 
 *        5323 :
 *        Add $Log section.
 *        
 *  1    mpeg      1.0         1/27/03 12:32:20 PM    Larry Wang      
 * $
 * 
 *    Rev 1.4   23 Sep 2003 14:15:18   jackmaw
 * SCR(s) 7527 :
 * Add function cnxt_handle_enqueue to be used for adding handles back onto a
 * free list. cnxt_handle_enqueue skips the check of pSelf for each instance it
 * visits, avoiding an incorrect failure.
 * 
 *    Rev 1.3   17 Sep 2003 13:01:16   jackmaw
 * SCR(s) 7482 :
 * Updated for new Multi Instance Driver Guidelines
 * 
 * 
 *    Rev 1.2   11 Feb 2003 09:20:02   wangl2
 * SCR(s) 5462 :
 * Move cnxt_handle_add() and cnxt_handle_remove() out of #ifdef DEBUG block.
 * 
 *    Rev 1.1   30 Jan 2003 08:15:32   wangl2
 * SCR(s) 5323 :
 * Add $Log section.
 *
 ****************************************************************************/

