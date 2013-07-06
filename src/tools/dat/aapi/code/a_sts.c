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

#define A_STS_INFINITE_SIZE           -1
#define A_STS_BLOCKSIZES {0, 4, 8, 16, 32, 64, 128, 256, 512, 1024, A_STS_INFINITE_SIZE}


// type that holds a counter for a specific free-value.
// an instance of this type will be held in a tree.
//
typedef struct a_stsCounter {
	c_long objectSize;
	a_counter count;
} *a_stsCounter;



typedef struct a_stsBlock {
	a_stsBlockKind kind;
	c_address      startAddress;
	c_long         size;
} *a_stsBlock;


struct a_stsContext_s {
	c_long         objectsCount;
	a_hshHashtable memoryBlocks;
	a_stsBlock     largestBlock;
	a_treTree      freeSpaceCounters;      // tree of counters (a_stsCounter)
};



/***********************************************************
 BLOCK OPERATIONS (Static!) - Create, Compare, Destroy
 ***********************************************************/

/**
 * Creates a new block on heap. Must be destroyed with
 * a_stsDestroyBlock after use.
 */
static a_stsBlock
a_stsCreateNewBlock(
	a_stsBlockKind kind,
	c_address startAddress,
	c_long size)
{
	a_stsBlock block = a_memAlloc(sizeof(struct a_stsBlock));
	if (block) {
		block->kind = kind;
		block->startAddress = startAddress;
		block->size = size;
	}
	return block;
}


/**
 * Destroys a block and frees up memory.
 * Typically used as a call back function.
 */
static void
a_stsDestroyBlock(
	a_stsBlock block)
{
	if (block) {
		a_memFree(block);
	}
}


#if 0
/**
 * Compares two block's values.
 * Returns 1 if block1 < block2
 *         0           =
 *        -1           >
 *       -99 if one of the blocks is NULL.
 * Typically used as a call back function.
 */
static int
a_stsCompareBlocks(
	a_stsBlock block1,
	a_stsBlock block2)
{
	int result;
	if (block1 && block2) {
		if (block1->startAddress < block2->startAddress) {
			result = 1;
		} else if (block2->startAddress < block1->startAddress) {
			result = -1;
		} else {
			result = 0;
		}
	} else {
		result = -99;
	}
	return result;
}
#endif


/***********************************************************
 COUNTER OPERATIONS (Static!) - Create, Compare, Destroy
 ***********************************************************/

static a_stsCounter
a_stsCreateNewCounter(
	c_long freeObjectSize,
	a_counter initialCountervalue)
{
	a_stsCounter counter = a_memAlloc(sizeof(struct a_stsCounter));
	if (counter) {
		counter->objectSize = freeObjectSize;
		counter->count = initialCountervalue;
	}
	return counter;
}


/**
 * Destroys a counter and frees up memory.
 * Typically used as a call back function.
 */
static void
a_stsDestroyCounter(
	a_stsCounter *counter)
{
	if (*counter) {
		a_memFree(*counter);
	}
}


/**
 * Compares two counter's values.
 * Returns 1 if counter1 < counter2
 *         0             =
 *        -1             >
 *       -99 if one of the counters is NULL.
 * Typically used as a call back function.
 */
static int
a_stsCompareCounters(
	a_stsCounter counter1,
	a_stsCounter counter2)
{
	int result;
	if (counter1 && counter2) {
		if (counter1->objectSize < counter2->objectSize) {
			result = 1;
		} else if (counter2->objectSize < counter1->objectSize) {
			result = -1;
		} else {
			result = 0;
		}
	} else {
		result = -99;
	}
	return result;
}



/***********************************************************
 CONTEXT OPERATIONS - Init & De-Init
 ***********************************************************/

/**
 * \brief
 * Creates a new context
 *
 * This operation creates a new context for this file. The
 * context is used for most operations in this file.
 * Remember to de-initilialise the context after use.
 *
 * \return
 * Pointer to a newly created context, or NULL if the operation
 * failed (memory full?).
 *
 * \see
 * a_shsContext a_shsDeInit
 */
a_stsContext
a_stsInit()
{
	a_stsContext context = a_memAlloc(sizeof(struct a_stsContext_s));
	if (context) {
		context->memoryBlocks = NULL;
		context->freeSpaceCounters = NULL;
		context->objectsCount = A_STS_MIN_OBJECTSCOUNT;
	}
	assert(context);
	return context;
}


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
 * Use this operation before first a_stsInsertNewObject, otherwise
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
int
a_stsSetObjectsCount(
	a_stsContext context,
	c_long newObjectsCount)
{
	int result;
	if (context) {
		context->objectsCount = newObjectsCount;
		result = 1;
	} else {
		result = 0;
	}
	return result;
}


/**
 * \brief
 * De-initialises the context
 *
 * This operation de-initialises this file's context and frees up
 * memory.
 *
 * \param context
 * This file's context to de-initialise
 *
 * \see
 * a_shsContext a_shsInit
 */
void
a_stsDeInit(
	a_stsContext context)
{
	if (context) {
		if (context->memoryBlocks) {
			a_hshDestroyHashtable(context->memoryBlocks);
		}
		if (context->freeSpaceCounters) {
			a_treDestroyTree(context->freeSpaceCounters, (a_treFreeAction)a_stsDestroyCounter);
		}
		a_memFree(context);
	}
}



/***********************************************************
 MEMORY BLOCK OPERATIONS - Insert a new block
 ***********************************************************/

/**
 * \brief
 * Inserts a new block into the data structure
 *
 * This operation creates a new data structure, holding a
 * \a memory \a block's information.
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
 * Memory Block's size
 *
 * \return
 * Boolean value specifying whether the operation was successful.
 */
int
a_stsInsertNewBlock(
	a_stsContext context,
	a_stsBlockKind kind,
	c_address startAddress,
	c_long size)
{
	int result;
	if (context) {
		assert(context);
		if (!context->memoryBlocks) {
			a_hshDestroyAction destroyAction = (a_hshDestroyAction)a_stsDestroyBlock;
			context->memoryBlocks = a_hshCreateHashtable(context->objectsCount * A_STS_BLOCKSCOUNTMULTIPLIER, destroyAction);
		}
		assert(context->memoryBlocks);
		if (context->memoryBlocks) {
			a_stsBlock block = a_stsCreateNewBlock(kind, startAddress, size);
			assert(block);
			if (block) {
				result = a_hshInsertEntry(context->memoryBlocks, (a_hshKey)startAddress, (a_hshValue)block);
				assert(result);
				if (!result) {
					a_stsDestroyBlock(block);
				}
			} else {
				result = 0;  // a_stsCreateNewBlock failed
			}
		} else {
			result = 0;  // hashtable could not be created
		}
	} else {
		result = 0;  // no context
	}
	return result;
}



int
a_stsBlocksCount(
	a_stsContext context)
{
	int result;
	if (context) {
		result = a_hshCount(context->memoryBlocks);
		if (result < 0) {
			result = 0;
		}
	} else {
		result = -1;
	}
	return result;
}




/***********************************************************
 FREE SPACE COUNTERS OPERATIONS - Increase a counter
 ***********************************************************/

/**
 * \brief
 * Increases the counter for the value of freeObjectSize by one.
 *
 * This operation increases the counter for the value of freeObjectSize
 * by one.
 *
 * \param context
 * This file's context. If context is NULL, this operation will fail.
 *
 * \param freeObjectSize
 * The size of the ObjectSpace that is free, to increase the counter of.
 * This size must not include the header's size!
 *
 * \return
 * Boolean value specifying whether this operation was successful.
 */
int
a_stsIncFreeSpaceCounter(
	a_stsContext context,
	c_long freeObjectSize)
{
	int result;
	if (context) {
		if (!context->freeSpaceCounters) {
			context->freeSpaceCounters = a_treCreateTree();
		}
		if (context->freeSpaceCounters) {
			a_treSortAction sortAction = (a_treSortAction)a_stsCompareCounters;
			a_stsCounter searchCounter = a_stsCreateNewCounter(freeObjectSize, 1);
			if (searchCounter) {
				a_stsCounter counter = a_treFindValue(context->freeSpaceCounters, (void *)searchCounter, sortAction);
				if (counter) {
					counter->count += 1;
					a_memFree(searchCounter);
					result = 1;
				} else {
					result = a_treInsertNode(context->freeSpaceCounters, (void *)searchCounter,
						sortAction, A_STS_NODUPESALLOWED);
				}
			} else {
				result = 0;
			}
		} else {
			result = 0;
		}
	} else {
		result = 0;
	}
	return result;
}


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
 * size will be retrieved to determine the length of the
 * memory portion that is occupied.\n
 * All non occupied segments are considered to be free for use.
 * Those free segments are counted and stored in a tree data
 * structure, sorted by size. The results of those counters
 * can later be retrieved with a walk function.
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
int
a_stsCalculateFreeSpaces(
	a_stsContext context,
	c_address shmAddress,
	c_long shmSize)
{
	int result;
//	printf("[a_stsCalculateFreeSpaces] start\n");
	assert(context);
	if (context->memoryBlocks) {
//		printf("[a_stsCalculateFreeSpaces] memoryBlocks != NULL\n");
		c_address shmEndAddress = (c_address)(shmAddress + (c_address)shmSize);
		c_address address = shmAddress;
		a_stsBlock block = (a_stsBlock)a_hshRead(context->memoryBlocks, (a_hshKey)address);
//		printf("[a_stsCalculateFreeSpaces] start:%8.8X end:%8.8X\n", (unsigned int)shmAddress, (unsigned int)shmEndAddress);

		/* search for the first known memory block */
		while ((address < shmEndAddress) && !block) {
			address += A_PTRSIZE;
			block = (a_stsBlock)a_hshRead(context->memoryBlocks, (a_hshKey)address);
		}
//		printf("[a_stsCalculateFreeSpaces] address:%8.8X block:%8.8X\n", (unsigned int)address, (unsigned int)block);
		
		c_address startGapAddress;
		c_long gapSize;
		c_long objectFreeDataSize;
		result = 1;
		while (result && (address < shmEndAddress)) {
			if (block) {
				address += (0 < block->size) ? block->size : A_PTRSIZE;
				block = (a_stsBlock)a_hshRead(context->memoryBlocks, (a_hshKey)address);
			} else {
				startGapAddress = address;
				while ((address < shmEndAddress) && !block) {
					address += A_PTRSIZE;
					block = (a_stsBlock)a_hshRead(context->memoryBlocks, (a_hshKey)address);
				}
				if (block) {
					gapSize = (c_long)(address - startGapAddress);
					objectFreeDataSize = gapSize - A_HEADERSIZE;                // could be negative!
//					printf("[a_stsCalculateFreeSpaces] addr:%8.8X gap:%ld\n",
//						(unsigned int)startGapAddress, (long)objectFreeDataSize);
					a_stsIncFreeSpaceCounter(context, objectFreeDataSize);
				}
			}
		}
		if (result) {
			gapSize = (c_long)(address - startGapAddress);
			objectFreeDataSize = gapSize - A_HEADERSIZE;               // could be negative!
			a_stsIncFreeSpaceCounter(context, objectFreeDataSize);
		}
	} else {
//		printf("[a_stsCalculateFreeSpaces] memoryBlocks == NULL\n");
		result = 0;
	}
//	printf("[a_stsCalculateFreeSpaces] end\n");
	return result;
}



/***********************************************************
 FREE SPACES WALK
 ***********************************************************/

/**
 * \brief
 * Local context for a_stsFreeSpaceCounterWalk
 */
typedef struct a_stsFreeSpaceCounterWalkContext {
	a_stsContext stsContext;
	a_stsFreeSpaceCountersWalkAction action;
	void *actionArg;
} *a_stsFreeSpaceCounterWalkContext;


/**
 * \brief
 * Local call back function for a_stsFreeSpaceCountersWalk
 */
static int
a_stsFreeSpaceCountersWalkCallback(
	a_stsCounter counter,
	a_stsFreeSpaceCounterWalkContext walkContext)
{
	int result;
	if (counter && walkContext) {
		result = (walkContext->action)(counter->objectSize, counter->count, walkContext->actionArg);
	} else {
		result = 0;
	}
	return result;
}


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
 * blah...
 *
 * \param action Arg
 * Pointer to a user defined function that will be passed along.
 *
 * \return
 * Boolean value specifying whether the operation was successful.
 *
 * \see
 * a_stsFreeSpaceCountersWalkAction
 */
int
a_stsFreeSpaceCountersWalk(
	a_stsContext context,
	a_stsFreeSpaceCountersWalkAction walkAction,
	void *actionArg)
{
	int result;
	if (context && walkAction) {
		a_treWalkAction freeSpaceCountersWalkAction = (a_treWalkAction)a_stsFreeSpaceCountersWalkCallback;
		struct a_stsFreeSpaceCounterWalkContext walkContext;
		walkContext.stsContext = context;
		walkContext.action = walkAction;
		walkContext.actionArg = actionArg;
		result = a_treWalk(context->freeSpaceCounters, freeSpaceCountersWalkAction, &walkContext);
	} else {
		result = 0;
	}
	return result;
}


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
int
a_stsFreeSpaceCountersCount(
	a_stsContext context)
{
	int result;
	if (context) {
		result = a_treCount(context->freeSpaceCounters);
		if (result < 0) {
			result = 0;
		}
	} else {
		result = -1;
	}
	return result;
}


//END a_sts.c
