/****************************************************************************/
/*                          Conexant Systems Inc.                           */
/****************************************************************************/
/*                                                                          */
/* Filename:       HEAP.H                                                   */
/*                                                                          */
/* Description:    Simple heap management functions                         */
/*                                                                          */
/* Author:         Dave Wilson                                              */
/*                                                                          */
/* Copyright Conexant Systems Inc, 2000                                     */
/* All Rights Reserved.                                                     */
/*                                                                          */
/****************************************************************************/

#ifndef _HEAP_H_
#define _HEAP_H_


typedef void *heap_handle;

heap_handle heap_create(void *pHeap, size_t sizeHeap);
bool   heap_destroy(heap_handle hHeap);
void * heap_alloc(heap_handle hHeap, size_t sizeAlloc);
bool   heap_free(heap_handle hHeap, void *pBlock);
bool   heap_free_all(heap_handle hHeap);
bool   heap_check(heap_handle hHeap);
void   heap_clean_and_flush_cache(void *pBlock, size_t sizeBlock);
#ifdef DEBUG
void heap_dump(heap_handle hHeap);
#endif

#endif
/****************************************************************************
 * Modifications:
 * $Log: 
 *  4    mpeg      1.3         7/15/03 1:48:42 PM     Dave Wilson     SCR(s) 
 *        6898 :
 *        Added prototype for heap_clean_and_flush_cache.
 *        
 *  3    mpeg      1.2         4/25/01 5:35:28 AM     Dave Wilson     DCS1772: 
 *        Added prototype for heap_dump used by video driver
 *        
 *  2    mpeg      1.1         4/20/01 12:13:54 PM    Dave Wilson     DCS1124: 
 *        Major changes to video memory management to get Sky Text app running
 *        
 *  1    mpeg      1.0         12/11/00 10:16:26 AM   Miles Bintz     
 * $
 * 
 *    Rev 1.3   15 Jul 2003 12:48:42   dawilson
 * SCR(s) 6898 :
 * Added prototype for heap_clean_and_flush_cache.
 * 
 *    Rev 1.2   25 Apr 2001 04:35:28   dawilson
 * DCS1772: Added prototype for heap_dump used by video driver
 * 
 *    Rev 1.1   20 Apr 2001 11:13:54   dawilson
 * DCS1124: Major changes to video memory management to get Sky Text app running
 * 
 *    Rev 1.0   11 Dec 2000 10:16:26   bintzmf
 * Initial revision.
 * 
 *    Rev 1.0   15 Nov 2000 11:28:44   dawilson
 * Initial revision.
 *
 ****************************************************************************/

