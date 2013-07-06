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
 *
 * This file contains the functions for loading and saving a memory
 * block from & to a file.\n
 * The file format is divided into two portions, that are
 * sequentially stored. First, there is an internal data structure,
 * maintained by a_fil, that consists of data to (re)create the
 * memory block. After that, the complete memory block is stored.\n
 * Due to programming limitations, both the \a Shared \a Memory
 * \a Name and \a Database \a Name are limited to 255 chars!
 */



#ifndef A_FIL_H
#define A_FIL_H

#include "a_def.h"


/**
 * \brief Context for a_fil:
 */
typedef struct a_filContext_s *a_filContext;


/**
 * \brief
 * Initialises the context for this file and returns a pointer to it
 * upon success
 *
 * \param fname
 * File Name that must be written to or read from
 *
 * \param shm_name
 * Shared Memory Name
 *
 * \param db_name
 * Database Name
 *
 * \param address
 * Shared Memory Start Address
 *
 * \param shm_size
 * Shared Memory Size
 *
 * \return
 * Pointer to a newly created context, or NULL if the context could
 * not be created.
 *
 * \see
 * a_filDeInit a_filContext
 */
a_filContext a_filInit(
	char *fname, char *shm_name, char *db_name, c_address address, c_long shm_size);


/**
 * \brief
 * De-Initialises the context
 *
 * This operation de-initialises the context and frees up memory.
 *
 * \param context
 * The context to be de-initialised
 *
 * \see
 * a_filInit a_filContext
 */
void a_filDeInit(a_filContext context);


/**
 * \brief
 * Writes a memory dump to a file, preceded by a header.
 *
 * This operation writes a memory chunk to file, after having
 * written a header with specific data about this chunk, for later
 * to (re)create the memory segment.\n
 * Although the memory start address to write from was on heap
 * in the original design, hence this function's name, the
 * address can also be of that of the attached shared memory
 * block.
 *
 * \param context
 * This file's context, which must have been initialised by
 * a_filInit. If context is NULL, this operation will fail.\n
 * If the file (specified at context creation) already exists,
 * this operation will fail.
 *
 * \param heapAddress
 * Memory Start Address to copy from. The size of the memory segment
 * is taken from the context.
 *
 * \return
 * Boolean value specifying whether the operation was successful.
 *
 * \see
 * a_filContext a_filInit
 */
int a_filHeap2File(a_filContext context, c_address heapAddress);


/**
 * \brief
 * Reads the header of a memory file, that was created by
 * a_filHeap2File.
 *
 * This operation reads the header portion of a memory file, that
 * was previously created by a_filHeap2File. Various members of the
 * context will be filled, overwriting old ones. The context must
 * have been initialised with a_filInit.\n
 * After having read the header, various \a Get \a Commands can be
 * used to retrieve the information. With this information you'll be
 * able to set up and reserve a (shared) memory segment and go on
 * with \a a_filFile2Heap, to load the file's memory data into
 * memory.
 *
 * \param context
 * Context holding the information for this operation. In this case,
 * the context's file name is used. Other members will be overwritten
 * by the data found in the file's header.\n
 * If context is NULL or if the file can not be found, this operation
 * will fail.
 *
 * \return
 * Boolean value specifying wether this operation was successful.
 *
 * \see
 * a_filContext a_filInit a_filFile2Heap
 */
int a_filReadHeader(a_filContext context);


/**
 * \brief
 * Reads the memory portions of a file, that was created by
 * a_filHeap2File, into memory.
 *
 * This operation reads a memory file, with the file name that
 * is set in the context, into memory. The header that is preceeded
 * by the memory portion, will be completely ignored.\n
 * Before this operation is executed, the header's info can be
 * extracted by executing \a a_filReadHeader first, after which its
 * values can be queried by the various \a Get functions in
 * \a a_fil (this file).\n
 * With this information, a memory segment must be allocated
 * (typically a malloc() function) before this function is called.
 *
 * \param context
 * Context holding the information for this operation. In this case,
 * the context's file name and shared memory size are used. If
 * context is NULL or if the file can not be found, this operation
 * will fail.
 *
 * \param heapAddress
 * Memory Start Address to copy the file's contents into. Despite the
 * name, this parameter's value can also be that of the start of the
 * attached shared memory segment, or any memory address for that
 * matter.
 *
 * \return
 * Boolean value specifying wether this operation was successful.
 *
 * \note
 * This operation assumes the reserved memory segment at
 * \a heapAddress is of the same size as that of the size to load
 * into memory, i.e. the value \a size in the file's header. This
 * operation does not check for this. If those sizes differ, this
 * operation will probably result in a segmentation fault.
 *
 * \see
 * a_filContext a_filInit a_filReadHeader
 */
int a_filFile2Heap(a_filContext context, c_address heapAddress);


/**
 * \brief
 * Returns a pointer to the currently known shm_name in the context.
 *
 * \param
 * context The context to retrieve the \a shm_name from. If context
 * is NULL, this operation will fail.
 *
 * \return
 * Pointer to shm_name in the context
 */
char *a_filGetShmName(a_filContext context);


/**
 * \brief
 * Returns a pointer to the currently known db_name in the context
 *
 * \param context
 * The context to retrieve the \a db_name from. If context is NULL,
 * this operation will fail.
 *
 * \return
 * Pointer to db_name in the context
 */
char *a_filGetDbName(a_filContext context);


/**
 * \brief
 * Returns the currently known shm_size in the context
 *
 * \param context
 * The context to retrieve the \a shm_size from. If context is NULL,
 * this operation will fail.
 *
 * \return
 * Value of shm_size in the context
 */
c_long a_filGetShmSize(a_filContext context);


/**
 * \brief
 * Returns the currently known address value in the context
 *
 * \param context
 * The context to retrieve the \a shm_address from. If context is
 * NULL, this operation will fail.
 *
 * \return
 * Value of shm_address in the context
 */
c_address a_filGetShmAddress(a_filContext context);


#endif  /* A_FIL_H */

//END a_fil.h
