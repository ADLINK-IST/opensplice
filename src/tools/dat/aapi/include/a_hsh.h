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
 * This file implements a (generic) hash table. This Hash Table
 * is made up of an array of \a buckets (internal data structure)
 * in which elements can reside. An internal hash function, with
 * an \a a_hshKey as input, determines what bucket to use for
 * storage. A bucket can hold an unlimited number of elements.
 * The bucket's internal data structure is that of a linked list.\n
 * An entry, also refered to as an element, consist of a key and
 * a (pointer) value.
 */


#ifndef A_HSH_H
#define A_HSH_H


#include "a_def.h"




/**
 * \brief
 * Type definition for the key used in this hash table.
 */
typedef c_address a_hshKey;
// typedef a_address a_hshKey;
// typedef unsigned long a_hshKey;


/**
 * \brief
 * Pointer to a user defined data structure holding the user data.
 */
typedef c_voidp a_hshValue;
// typedef void *a_hshValue;


/**
 * \brief
 * Type Definition of a user defined function for destroying the user
 * defined data.
 *
 * This type definition describes the function header of a user
 * defined function for destroying a user defined data structure.
 *
 * \param value
 * Pointer to a user defined data structure.
 *
 * \return
 * The user defined function must return true (1) if destroying the
 * user data was successful, false (0) if it was not.
 *
 * \see
 * a_hshCreateHashtable
 */
typedef int (*a_hshDestroyAction)(a_hshValue value);


/**
 * \brief
 * Type Definition of a user defined function for calling back to
 * when walking over all elements.
 *
 * This type definition describes the function header of a user
 * defined function for calling back to when all elements are walked
 * over.
 *
 * \param key
 * Key value for the current element
 *
 * \param value
 * Pointer value for the current element, corresponding to \a key
 *
 * \param actionArg
 * Pointer to a user defined context that will be passed along
 *
 * \see
 * a_hshWalk
 */
typedef int (*a_hshWalkAction)
	(a_hshKey key, a_hshValue value, void *actionArg);


/**
 * \brief
 * (Hidden) Data Structure holding the Hash Table
 */
typedef struct a_hshHashtable_s *a_hshHashtable;



/***************************************************************
 CREATE & DESTROY
 ***************************************************************/


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
a_hshHashtable a_hshCreateHashtable(
	c_long tableSize, a_hshDestroyAction destroyAction);


/**
 * \brief
 * Destroys the hashtable
 *
 * This operation destroys the hashtable, freeing memory. For all
 * elements, a user defined function destroyAction, that was
 * specified at creation time, will be called to destroy the
 * user defined data.
 *
 * \param hashtable
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
int a_hshDestroyHashtable(a_hshHashtable hashtable);


/***************************************************************
 INSERT A NEW ENTRY
 ***************************************************************/

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
int a_hshInsertEntry(
	a_hshHashtable hashtable, a_hshKey key, a_hshValue value);


/***************************************************************
 FIND & REMOVE ENTRY
 ***************************************************************/

/**
 * \brief
 * Searches for a key in a hash table and returns its corresponding
 * value.
 *
 * This operation searches for the element with specified \a key
 * in the hash table and returns the corresponding pointer value
 * that points to a user defined data structure.
 *
 * \param hashtable
 * The hash table that needs to be searched in.
 *
 * \param key
 * The key that needs te be searched for.
 *
 * \return
 * Pointer to a user defined data structure for the corresponding
 * \a key, if found, or NULL if \a key is not found or if hashtable
 * is NULL.
 *
 * \see
 * a_hshTake
 */
a_hshValue a_hshRead(a_hshHashtable hashtable, a_hshKey key);


/**
 * \brief
 * Searches for a key in a hash table and returns its corresponding
 * value, removing the element that held the key and value from the
 * hash table.
 *
 * This operation searches for the element with specified \a key
 * in the hash table and returns the corresponding pointer value
 * that points to a user defined data structure. The element that
 * held this \a key and \a pointer \a value will be removed.
 *
 * \param hashtable
 * The hash table that needs to be searched in.
 *
 * \param key
 * The key that needs te be searched for.
 *
 * \return
 * Pointer to a user defined data structure for the corresponding
 * \a key, if found, or NULL if \a key is not found or if hashtable
 * is NULL.
 *
 * \note
 * Remember to destroy the user defined data structure, where the
 * result of this function points to, yourself, as there will be no
 * more references to that structure within the hash table.
 *
 * \see
 * a_hshRead
 */
a_hshValue a_hshTake(a_hshHashtable hashtable, a_hshKey key);


/***************************************************************
 COUNT
 ***************************************************************/

/**
 * \brief
 * Returns the total number of elements in the hashtable.
 *
 * This operation returns the total number of elements currently
 * in the hash table.
 *
 * \param hashtable
 * The hash table to request the \a count from.
 *
 * \return
 * The number of elements in the hash table. If \a hashtable = NULL,
 * -1 will be returned.
 */
a_counter a_hshCount(a_hshHashtable hashtable);


/**
 * \brief
 * Returns the number of elements of the largest bucket.
 *
 * This operation will return the number of elements of the bucket
 * holding the most elements.
 *
 * \note
 * This function is merely available for testing purposes and should
 * be used to determine the collision size. A perfect hash table is
 * considered a hash table with a Largest Bucket Count of 1 and all
 * array elements used.
 *
 * \param hashtable
 * The has table to retrieve the largest bucket count from.
 *
 * \return
 * Number of elements of the largest bucket found in the hash table.
 * If \a hashtable is NULL, -1 will be returned.
 */
a_counter a_hshLargestBucketCount(a_hshHashtable hashtable);


/***************************************************************
 WALK
 ***************************************************************/

/**
 * \brief
 * Walks over all elements in the hash table.
 *
 * This operation 'walks' over all elements in the hash table and
 * calls a user defined call back function for every element.
 *
 * \param hashtable
 * The hash table of which all elements should be walked over. If
 * \a hashtable is NULL, this operation will fail.
 *
 * \param walkAction
 * Pointer to a user defined function to call every element with. If
 * \a walkAction is NULL, this operation will fail.
 *
 * \param actionArg
 * Pointer to a user defined context that will be passed along with
 * every call to the call back function. May be NULL.
 *
 * \return
 * True (1) if operation was successful and all elements were walked
 * over or false (0) if either hashtable or walkAction is NULL, or if
 * the walk was aborted because the user defined function returned
 * false (0).
 *
 * \note
 * The order in which the elements are walked over is undetermined.
 *
 * \warning
 * Do not use any insertion or remove during a walk.
 *
 * \see
 * a_hshWalkAction
 */
int a_hshWalk(
	a_hshHashtable hashtable, a_hshWalkAction walkAction, void *actionArg);


#endif  /* A_HSH_H */


//END  a_hsh.h
