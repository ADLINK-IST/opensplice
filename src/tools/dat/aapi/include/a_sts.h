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
 * This file contains functions for holding the data structure(s) for
 * memory statistics.
 */


#ifndef A_STS_H
#define A_STS_H

#include "a_def.h"



/**
 * \brief
 * Kinds of memory blocks used within this file
 */
typedef enum a_stsBlockKind {
	A_STS_BLOCK_UNDEFINED,     ///< Undefined value, used internally
	A_STS_BLOCK_ADMIN,         ///< Administration
	A_STS_BLOCK_OBJECTHEADER,  ///< Database Object's Header
	A_STS_BLOCK_OBJECTDATA,    ///< Database Object's Data
	A_STS_BLOCK_MARKER,        ///< Marker
	A_STS_BLOCK_AVLNODE,       ///< SPLICE-DDS AVL-Tree Type
	A_STS_BLOCK_FREESPACE,     ///< Free Database space
	A_STS_BLOCK_COUNT          ///< Enum count, used internally
} a_stsBlockKind;


/**
 * \brief
 * (Hidden) Data Structure for this file
 */
typedef struct a_stsContext_s *a_stsContext;



/***********************************************************
 CONTEXT OPERATIONS - Init & De-Init
 ***********************************************************/

/**
 * \brief
 * Creates a new context
 *
 * This operation creates a new context for this file. The context is
 * used for most operations in this file. Remember to de-initilialise
 * the context after use.
 *
 * \return
 * Pointer to a newly created context, or NULL if the operation
 * failed (memory full?).
 *
 * \see
 * a_shsContext a_shsDeInit
 */
a_stsContext a_stsInit();


/**
 * \brief
 * Sets a (new) value for the number of database objects
 *
 * This operation sets a new value for the number of database objects
 * into this file's context. With this value, an optimum array size
 * for the hash table to be used can be determined. Internally,
 * this value will be multiplied, due to the storing of not only
 * the data portions of database objects, but the headers and
 * "in between markers" as well.
 *
 * \note
 * Use this operation before first a_stsInsertNewBlock, otherwise
 * a default value will be used, which might slow down the overall
 * process on large database sizes.
 *
 * \param context
 * This file's context. If context is NULL, this operation will fail.
 *
 * \param newObjectsCount
 * Value of the (expected) total number of database objects.
 *
 * \return
 * Boolean value specifying whether this operation was successful.
 */
int a_stsSetObjectsCount(a_stsContext context, c_long newObjectsCount);


/**
 * \brief
 * De-initialises the context
 *
 * This operation de-initialises this file's context and frees up
 * memory.
 *
 * \param context
 * This file's context to de-initialise.
 *
 * \see
 * a_shsContext a_shsInit
 */
void a_stsDeInit(a_stsContext context);


/***********************************************************
 MEMORY BLOCK OPERATIONS - Insert a new block
 ***********************************************************/

/**
 * \brief
 * Inserts a new block into the data structure
 *
 * This operation creates a new data structure, holding a \a memory
 * \a block's information.
 *
 * \param context
 * This file's context. If context is NULL, the operation will fail.
 *
 * \param kind
 * Memory Block's kind
 *
 * \param startAddress
 * Memory Block's Start Address
 *
 * \param size
 * Memory Block's size.
 *
 * \return
 * Boolean value specifying whether the operation was successful.
 */
int a_stsInsertNewBlock(
	a_stsContext context, a_stsBlockKind kind, c_address startAddress,
	c_long size);


/**
 * \brief
 * Counts the number of collected Memory Blocks
 *
 * This operation counts and returns the number of collected Memory
 * Blocks
 *
 * \param context
 * This file's context. If context is NULL, this operation will fail.
 *
 * \return
 * The number of Memory Blocks, or -1 if anything failed.
 *
 * \see
 * a_stsContext
 */
int a_stsBlocksCount(a_stsContext context);


/***********************************************************
 FREE SPACE COUNTERS OPERATIONS - Increase a counter
 ***********************************************************/

/**
 * \brief
 * Increases the counter for the value of freeObjectSize by one.
 *
 * This operation increases the counter for the value of
 * freeObjectSize by one.
 *
 * \param context
 * This file's context. If context is NULL, this operation will fail.
 *
 * \param freeObjectSize
 * The size of the ObjectSpace that is free, to increase the counter
 * of. This size must not include the header's size!
 *
 * \return
 * Boolean value specufying whether this operation was successful.
 */
int a_stsIncFreeSpaceCounter(a_stsContext context, c_long freeObjectSize);


/***********************************************************
 CALCULATE FREE SPACES
 ***********************************************************/

/**
 * \brief
 * Scans the Shared Memory for unused memory space
 *
 * This operation scans for unused memory space in the Shared
 * Memory segment by scanning the Shared Memory at every
 * pointer-size interval to check if it's the start of a
 * memory block, that was collected earlier. If it is, its
 * size is retrieved to determine the length of the
 * memory portion that is occupied.\n
 * All non occupied segments are considered to be free for use.
 * Those free segments are counted and stored in a tree data
 * structure, sorted by size. The results of those counters
 * can later be retrieved by a walk function.
 *
 * \param context
 * This file's context, holding the collected memory blocks and
 * FreeSpaceCounters.
 *
 * \param shmAddress
 * Memory Start Address of the Shared Memory Segment to scan in
 *
 * \param shmSize
 * Size of the Shared Memory Segment
 *
 * \return
 * Boolean value specifying whether the operation was successful.
 *
 * \see
 * a_stsContext
 */
int a_stsCalculateFreeSpaces(
	a_stsContext context, c_address shmAddress, c_long shmSize);


/***********************************************************
 FREE SPACES WALK
 ***********************************************************/

/**
 * \brief
 * Type definition of the user defined function that will be called
 * by a_stsFreeSpaceCountersWalk.
 *
 * \param freeSpaceSize
 * Size of the memory block(s) that was found, and counted
 *
 * \param count
 * Number of times a free space memory block (of size freeSpaceSize)
 * was found
 *
 * \param actionArg
 * Pointer to a user defined context that will be passed along
 *
 * \return
 * The user defined function must return true (1) to let the walk
 * continue. If false (0) is returned, the walk will abort.
 *
 * \see
 * a_stsFreeSpaceCountersWalk
 */
typedef int (*a_stsFreeSpaceCountersWalkAction)
	(c_long freeSpaceSize, c_long count, void *actionArg);


/**
 * \brief
 * Walks over all FreeSpaceCounters
 *
 * This operation walks over all FreeSpaceCounters and calls a
 * user defined function, passing along a counter's free space size
 * and its count.
 * 
 * \param context
 * This file's context. If context is NULL, the operation will fail.
 *
 * \param walkAction
 * Pointer to a user defined function that must be called for every
 * \a FreeSpaceCounter
 *
 * \param actionArg
 * Pointer to a user defined function that will be passed along.
 *
 * \return
 * Boolean value specifying whether the operation was successful.
 *
 * \see
 * a_stsFreeSpaceCountersWalkAction
 */
int a_stsFreeSpaceCountersWalk(
	a_stsContext context, a_stsFreeSpaceCountersWalkAction walkAction,
	void *actionArg);


/**
 * \brief
 * Counts the number of FreeSpaceCounters
 *
 * This operation counts the number of collected FreeSpaceCounters
 *
 * \param context
 * This file's context
 *
 * \return
 * The number of collected FreeSpaceCounters, or -1 if context is
 * NULL
 *
 * \see
 * a_stsContext
 */
int a_stsFreeSpaceCountersCount(a_stsContext context);


#endif  /* A_STS_H */


//END a_stst.h
