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

#endif
/****************************************************************************
 * Modifications:
 * $Log: 
 *  1    mpeg      1.0         11/15/00 11:28:44 AM   Dave Wilson     
 * $
 * 
 *    Rev 1.0   15 Nov 2000 11:28:44   dawilson
 * Initial revision.
 *
 ****************************************************************************/

