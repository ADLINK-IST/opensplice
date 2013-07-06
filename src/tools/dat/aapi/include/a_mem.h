/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2013 PrismTech
 *   Limited and its licensees. All rights reserved. See file:
 *
 *                     $OSPL_HOME/LICENSE 
 *
 *   for full copyright notice and license terms. 
 *
 */
 * operations specific to Shared Memory.
 */

#ifndef A_MEM_H
#define A_MEM_H

#include "a_def.h"


/**
 * \brief
 * Allocates memory of specified size on heap.
 *
 * This operation allocates memory on heap and returns a pointer
 * to the newly reserved memory segment, or NULL if failed.
 *
 * \param size
 * Size of the memory segment to reserve.
 *
 * \return
 * Pointer to the newly reserved memory segment, or NULL if failed
 * (out of memory?).
 *
 * \remark
 * os_malloc() from Splice is used internally, trying to operate
 * platform independent. It was tested only on Solaris though.
 */
void *a_memAlloc(long size);


/**
 * \brief
 * Copies memory
 *
 * This operation copies memory from one address to another,
 * specified by its size.
 *
 * \param toAddr
 * Memory Address where to copy data to.
 *
 * \param fromAddr
 * Memory Address wehere to copy from.
 *
 * \param size
 * Number of bytes to copy.
 *
 * \return
 * Boolean value specifying whether the operation was successful
 *
 * \note
 * This is like the memcpy function, but \a a_memCopyMem tends to be
 * platform independent.
 */
int a_memCopyMem(void *toAddr, void *fromAddr, long size);


/**
 * \brief
 * Frees a memory segment on heap.
 *
 * This operation will free up memory that was previously allocated
 * with a_memAlloc.
 *
 * \param ptr
 * Pointer to the memory segment (on heap) that needs to be freed
 *
 * \note
 * Use this function instead of free(), to be platform independent
 */
void a_memFree(void *ptr);


/**
 * \brief
 * Duplicates a string.
 *
 * This operation allocates memory on heap and copies the source
 * string into.
 *
 * \param src
 * Pointer to the string to be copied.
 *
 * \return
 * Pointer to a new string on heap, with the same value as \a src.
 *
 * \note
 * Same functionality as \a strdup, but implemented here to intend to
 * be platform independent (uses Splice's os_malloc() internally).
 */
char *a_memStrdup(char *src);


#endif   /* A_MEM_H */

//END a_mem.h
