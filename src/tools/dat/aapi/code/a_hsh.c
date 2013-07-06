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
/* A bucket is what we call the start of a linked list, at every
 * array index.
 */
struct a_hshBucket_s {
	struct a_hshNode_s *first;
	a_counter count;
};
typedef struct a_hshBucket_s *a_hshBucket;


/* A Hashtable has an array of buckets, which is defined here
 * as a flexible array member. Therefore it must be declared
 * as the last member in the struct.
 */
struct a_hshHashtable_s {
	c_long tableSize;
	a_counter count;
	a_hshDestroyAction destroyAction;
	a_hshBucket buckets[];              // flexible array size, must be last member of struct!
};



/***************************************************************
 CREATE & DESTROY
 ***************************************************************/


/* Computes the hash for the given key
 */
static c_long
a_hshHash(
	a_hshHashtable hashtable,
	a_hshKey key)
{
	return (hashtable ? ((c_long)(unsigned long)key % (unsigned long)hashtable->tableSize) : -1);
}



/* Creates and initialises a new Node and returns a pointer to that node.
 * Returns NULL if anything failed, like memory full.
 */
static a_hshNode
a_hshCreateNode(
	a_hshKey key,
	a_hshValue value)
{
	a_hshNode node = a_memAlloc(sizeof(struct a_hshNode_s));
	if (node) {
		node->key = key;
		node->value = value;
		node->next = NULL;
	}
	return node;
}



/* Creates and initialises a new bucket. Returns NULL if creation
 * failed (memory full?).
 */
static a_hshBucket
a_hshCreateBucket()
{
	a_hshBucket bucket = a_memAlloc(sizeof(struct a_hshBucket_s));
	if (bucket) {
		bucket->first = NULL;
		bucket->count = 0;
	}
	return bucket;
}



/* Destroys a bucket and all nodes in it.
 * Returns 0 if bucket was empty.
 */
static int
a_hshDestroyBucket(
	a_hshBucket bucket,
	a_hshDestroyAction destroyAction)
{
	int result = bucket ? 1 : 0;
	if (bucket) {
		while (bucket->first && result) {
			a_hshNode node = bucket->first;
			if (node->value) {
				if (destroyAction) {
					result = (destroyAction)(node->value);
				}
			}
			bucket->first = node->next;
			a_memFree(node);
		}
		a_memFree(bucket);
	}
	return result;
}



/**
 * \brief
 * Creates a new HashTable
 *
 * This operation creates a new hash table and returns a pointer
 * to it. The table size refers to the number of elements to be
 * used in the internal array. The real array size used at
 * creation may differ from the given here. Due to optimilisation
 * reasons, the \a array \a size modulo \a the \a size \a of
 * \a a_hshKey may not be zero, since \a a_hshKey is the key value
 * for the \a hash \a function. If it is, this operation will
 * increase the array size.
 *
 * \param tableSize
 * The requested array size. If \a tableSize is zero or negative,
 * this operation will fail.
 *
 * \param destroyAction
 * Pointer to a user defined function for destroying a user defined
 * data structure. This value will be used in other functions. For
 * convenience reasons it must be specified here this once. This
 * value may be NULL. In that case, no user defined function will be
 * called upon data destroying, only internal hash table memory will
 * be freed. This way, the key value to the hash table, \a a_hshkey,
 * is used as the user data itself, instead of a pointer to the data.
 *
 * \return
 * Pointer to the newly created hash table, or NULL if anything
 * failed, like when there's not enough memory available for the
 * specified \a tableSize, or when \a tableSize is lower than 1.
 *
 * \see
 * a_hshDestroyAction
 */
a_hshHashtable
a_hshCreateHashtable(
	c_long tableSize,
	a_hshDestroyAction destroyAction)
{
	a_hshHashtable hashtable;
	while (tableSize % sizeof(a_hshKey) == 0) {   // force size *not* to be dividable by 4
		tableSize++;
	}
	if (0 < tableSize) {
		size_t size = sizeof(struct a_hshHashtable_s) + tableSize * sizeof(struct a_hshBucket_s);
		hashtable = a_memAlloc(size);
		if (hashtable) {
			c_long index;
			hashtable->tableSize = tableSize;
			hashtable->destroyAction = destroyAction;
			hashtable->count = 0;
			index = tableSize;
			while (index--) {
				hashtable->buckets[index] = NULL;
			}
		}
	} else {
		hashtable = NULL;
	}
	return hashtable;
}



/**
 * \brief
 * Destroys the hashtable
 *
 * This operation destroys the hashtable, freeing memory. For all
 * elements, a user defined function destroyAction, that was
 * specified at creation time, will be called to destroy the
 * user defined data.
 *
 * \param hashTable
 * The Hash Table to be destroyed.
 *
 * \return
 * True (1) if the operation was successful and memory was freed,
 * false (0) if anything failed.
 *
 * \note
 * In order to destroy all elements, the user defined function
 * \a destroyAction must return true (1) to indicate the destruction
 * of the user defined data structure was successful. If false (0)
 * is returned, this operation will abort and will return false (0)
 * itself, to indicate the Hash Table could not be destroyed.
 *
 * \see
 * a_hshDestroyAction
 */
int
a_hshDestroyHashtable(
	a_hshHashtable hashtable)
{
	int result = hashtable ? 1 : 0;
	if (hashtable) {
		c_long index = hashtable->tableSize;
		while (index-- && result) {
			a_hshBucket bucket = hashtable->buckets[index];
			if (bucket) {
				result = a_hshDestroyBucket(bucket, hashtable->destroyAction);
			}
		}
		a_memFree(hashtable);
	}
	return result;
}



/***************************************************************
 INSERT A NEW ENTRY
 ***************************************************************/

/* Inserts a node (newNode) recursively into a linked list,
 * specifying the previous (parent) node pointer.
 */
static int
a_hshInsertEntryNode(
	a_hshNode *node,
	a_hshNode newNode)
{
	int result;
	if (*node == NULL) {
		*node = newNode;
		result = 1;
	} else if (newNode->key < (*node)->key) {
		newNode->next = *node;
		*node = newNode;
		result = 1;
	} else if (newNode->key == (*node)->key) {  // no dupes allowed!
		a_memFree(newNode);
		result = 0;
	} else {
		result = a_hshInsertEntryNode( &((*node)->next), newNode);
	}
	return result;
}



/* Creates a new node and inserts it into the specified bucket.
 * Assumption: bucket's value is valid
 */
static int
a_hshInsertEntryIntoBucket(
	a_hshBucket bucket,
	a_hshKey key,
	a_hshNode node)
{
	int result = a_hshInsertEntryNode(&(bucket->first), node);
	if (result) {
		bucket->count++;
	}
	return result;
}



/**
 * \brief
 * Inserts an entry into the hashtable.
 *
 * This operation will create e new element (internal data structure)
 * into the hash table, holding the pointer \a value to the user
 * defined data structure. \a key is used for positioning in the
 * array.
 *
 * \param hashtable
 * The hash table into which the data needs to be stored. If
 * hashtable is NULL, this operation will fail.
 *
 * \param key
 * Key that corresponds to the user data \a value
 *
 * \param value
 * Pointer to the user defined data structure that needs to be held
 * in the hash table.
 *
 * \return
 * True (1) if operation was successful, false (0) if it was not.
 */
int
a_hshInsertEntry(
	a_hshHashtable hashtable,
	a_hshKey key,
	a_hshValue value)
{
	int result;
	if (hashtable) {
		c_long hash = a_hshHash(hashtable, key);
		a_hshBucket bucket = hashtable->buckets[hash];
		a_hshNode node;
		if (!bucket) {
			bucket = a_hshCreateBucket();
			hashtable->buckets[hash] = bucket;
		}
		node = a_hshCreateNode(key, value);
		if (node) {
			result = a_hshInsertEntryIntoBucket(bucket, key, node);
		} else {
			result = 0;
		}
		if (result) {
			hashtable->count += 1;
		}
	} else {
		result = 0;
	}
	assert(result);
	return result;
}



/***************************************************************
 FIND & REMOVE ENTRY
 ***************************************************************/

/* 
 */
static a_hshValue
a_hshFindValue(
	a_hshNode node,
	a_hshKey key)
{
	a_hshValue result;
	if (node) {
		if (node->key == key) {
			result = node->value;
		} else {
			result = a_hshFindValue(node->next, key);
		}
	} else {
		result = NULL;
	}
	return result;
}



/* Returns a value specified by its key, or
 * NULL if not found.
 */
a_hshValue
a_hshRead(
	a_hshHashtable hashtable,
	a_hshKey key)
{
	a_hshValue result;
	if (hashtable) {
		a_hshBucket bucket = hashtable->buckets[a_hshHash(hashtable, key)];
		if (bucket) {
			result = a_hshFindValue(bucket->first, key);
		} else {
			result = NULL;
		}
	} else {
		result = NULL;
	}
	return result;
}



/* Searches from a specified (start)node for an entry value,
 * specified by its key, and returns the value, removing the
 * node from the linked list (=bucket).
 * Returns NULL if not found.
 */
static a_hshValue
a_hshFindValueRemoveNode(
	a_hshNode *node,
	a_hshKey key)
{
	a_hshValue result;
	if (*node) {
		if ((*node)->key == key) {
			result = (*node)->value;
			a_hshNode tmpNode = (*node);
			*node = (*node)->next;
			a_memFree(tmpNode);
		} else {
			result = a_hshFindValueRemoveNode( &((*node)->next), key);
		}
	} else {
		result = NULL;
	}
	return result;
}




static a_hshValue
a_hshTakeNode(
	a_hshNode *firstNode,
	a_hshKey key)
{
	a_hshValue result;
	if (*firstNode) {
		if ((*firstNode)->key == key) {
			result = (*firstNode)->value;
			a_hshNode removeNode = *firstNode;
			*firstNode = (*firstNode)->next;
			a_memFree(removeNode);
		} else {
			result = a_hshTakeNode(&((*firstNode)->next), key);
		}
	} else {
		result = NULL;
	}
	return result;
}
	


/* Searches for a node in a specific bucket and returns a pointer
 * to the corresponding value.
 * Assumes bucket != NULL.
 */
static a_hshValue
a_hshTakeFromBucket(
	a_hshBucket bucket,
	a_hshKey key)
{
	a_hshValue result;
	if (bucket->first) {
		result = a_hshTakeNode(&(bucket->first), key);
		if (result) {
			bucket->count -= 1;
		}
		if (!bucket->count) {
			bucket->first = NULL;
		}
	} else {
		result = NULL;
	}
	return result;
}




/* Returns a value, specified by its key, and removes the entry
 * from the hashtable. Returns NULL if not found.
 */
a_hshValue
a_hshTake(
	a_hshHashtable hashtable,
	a_hshKey key)
{
	a_hshValue result;
	if (hashtable) {
		c_long hash = a_hshHash(hashtable, key);
		a_hshBucket bucket = hashtable->buckets[hash];
		result = bucket ? a_hshTakeFromBucket(bucket, key) : NULL;
		if (result) {
			hashtable->count--;
		}
	} else {
		result = NULL;
	}
	return result;
}



/***************************************************************
 COUNT
 ***************************************************************/

/* Returns the total number of nodes in the hashtable
 */
a_counter
a_hshCount(
	a_hshHashtable hashtable)
{
	return hashtable ? hashtable->count : -1;
}


/* Returns the count of the largest bucket
 */
a_counter
a_hshLargestBucketCount(
	a_hshHashtable hashtable)
{
	a_counter result;
	if (hashtable) {
		c_long index = hashtable->tableSize;
		a_hshBucket bucket;
		result = 0;
		while (index--) {
			bucket = hashtable->buckets[index];
			if (bucket) {
				if (result < bucket->count) {
					result = bucket->count;
				}
			}
		}
	} else {
		result = -1;
	}
	return result;
}



/***************************************************************
 WALK
 ***************************************************************/


/* Returns all Entry Values, one by one, in calls to a user defined
 * function. Due to performance reasons, the order in which the
 * hashtable is walked is undetermined.
 * In order to continue the walk, the user defined function must
 * return >0. If 0 is returned, the walk will abort and will
 * return 0 itself, inditcating an aborted walk.
 * Returns 0 if hashtable == NULL.
 */
int
a_hshWalk(
	a_hshHashtable hashtable,
	a_hshWalkAction walkAction,
	void *actionArg)
{
	int result;
	if (hashtable && walkAction) {
		c_long index = hashtable->tableSize;
		result = 1;
		while (index-- &&  0 < result) {
			a_hshBucket bucket = hashtable->buckets[index];
			if (bucket) {
				a_hshNode node = bucket->first;
				while (node &&  0 < result) {
					result = (walkAction)(node->key, node->value, actionArg);
					node = node->next;
				}
			}
		}
	} else {
		result = 0;
	}
	return result;
}
	


//END  a_hsh.c
