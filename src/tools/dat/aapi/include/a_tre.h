/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2011 PrismTech
 *   Limited and its licensees. All rights reserved. See file:
 *
 *                     $OSPL_HOME/LICENSE 
 *
 *   for full copyright notice and license terms. 
 *
 */
 *
 * This file implements an AVL-Tree and uses pointers to user defined
 * data structures, to keep its use generic. The tree is balanced at
 * all times, as defined by the "AVL Property", upon returning every
 * modification function (Insert, Delete).
 */


#ifndef A_TRE_H
#define A_TRE_H


/**
 * \brief
 * Tree Type
 *
 * Abstract Data Type for an AVL-Tree Implementation. The tree uses
 * void pointers to user defined data structures.
 */
typedef struct a_treTree_s *a_treTree;


/**
 * \brief
 * Type Definition for a user defined function for destroying a user
 * defined entry.
 *
 * This definition describes the type of a function that
 * \a a_treDestroyTree calls back to, in order to destroy the user
 * defined data and free up memory.
 *
 * \param valuePtr
 * A void pointer to the user defined data structure that must be
 * destroyed.
 *
 * \see
 * a_treDestroyTree
 */
typedef void (*a_treFreeAction)(void *valuePtr);


/**
 * \brief
 * Type Definition for comapring two user defined data structures.
 *
 * This definition describes the type of a function that several
 * functions in this section use to determine the order of two data
 * structures, as this tree implementation has no knowledge of its
 * contents.
 * 
 * \param valuePtr1
 * Pointer to the first user defined data structure. This value will
 * not be NULL.
 *
 * \param valuePtr2
 * Pointer to the second user defined data structure. This value will
 * not be NULL.
 *
 * \return
 * The user defined function must return\n
 *    1 if valuePtr1 < valuePtr2\n
 *    0 if valuePtr1 = valuePtr2\n
 *   -1 if valuePtr2 < valuePtr1.\n
 * Any other return value is considered to be invalid.
 *
 * \see
 * a_treInsertNode a_treFindValue
 * a_treFindLowestHigherValue
 * a_treFindHighestLowerValue
 */
typedef int (*a_treSortAction)(void *valuePtr1, void *valuePtr2);


/***********************************************************
 CREATION AND DESTROY
 ***********************************************************/

/**
 * \brief
 * Creates a new tree and returns a pointer to that tree.
 *
 * This operation creates a new tree and returns a pointer to that
 * tree. If no memory could be allocated, i.e. if malloc failed,
 * NULL is returned.
 *
 * \return
 * Pointer to the newly created tree, or NULL if tree creation
 * failed.
 */
a_treTree a_treCreateTree();


/**
 * \brief
 * Destroys a tree, and frees its memory.
 *
 * This operation destroys a tree and all its nodes. A call is made
 * to a user defined function to destroy the user defined data,
 * related to that node.\n
 * This user defined function must be of type \a a_treFreeAction.
 *
 * \param tree
 * The tree to destroy. If tree = NULL, this operation will fail.
 *
 * \param freeAction
 * A user defined function that will destroy the user defined data.
 * freeAction may be NULL. In that case, only the tree itself will be
 * destroyed.
 *
 * \return
 * Boolean value specifying whether the operation was successful.
 *
 * \see
 * a_treFreeAction
 */
int a_treDestroyTree(a_treTree tree, a_treFreeAction freeAction);


/***********************************************************
 INTERT, FIND, DELETE
 ***********************************************************/

/**
 * \brief
 * Creates a new node and inserts that node into the list.
 *
 * This operation creates a new node (internally) and inserts it,
 * with a pointer to the user defined data structure, ordered into
 * the list.
 *
 * \param tree
 * The tree into which the new node must be created. If tree is NULL,
 * the operation will fail.
 *
 * \param valuePtr
 * Pointer to the user defined data structre. valuePtr is expected
 * not to be NULL.
 *
 * \param sortAction
 * Pointer to a user defined function that compares two user defined
 * data structures, for the tree to determine its sort order.
 *
 * \param dupesAllowed
 * Boolean setting specifying whether duplicate entries are allowed.
 * If a duplicate is detected while this setting is false, the
 * operation will fail. Remember that in that case the user defined
 * data structure will not have a reference from this list. You'll
 * need to store the reference yourself, or destroy the data and free
 * up memory in order to avoid memory leaks.
 *
 * \return
 * Boolean value specifying whether the operation was successful.
 *
 * \see
 * a_treSortAction
 */
int a_treInsertNode(
	a_treTree tree, void *valuePtr, a_treSortAction sortAction,
	int dupesAllowed);


/**
 * \brief
 * Searches for a value in a tree.
 *
 * This operation searches within the tree for a node with a value,
 * that is the same of valuePtr, and returns a pointer to the value
 * that the (internal) node holds.
 *
 * \note
 * The user defined data structures are compared, not the pointers to
 * those structures.
 *
 * \param tree
 * The tree in which the value must be searched for.
 *
 * \param valuePtr
 * A pointer to the user defined data structre. valuePtr is expected
 * not to be NULL.
 *
 * \param sortAction
 * Pointer to a user defined function that compares two user defined
 * data structures, for the tree to know its sort order.
 *
 * \return
 * Returns a pointer to the found value or NULL if not found.
 *
 * \see
 * a_treSortAction
 */
void *a_treFindValue(
	a_treTree tree, void *valuePtr, a_treSortAction sortAction);


/**
 * \brief
 * Finds and returns the first entry in the tree, that is defined as
 * the lowest ordered.
 *
 * This operation returns a pointer to the user defined data
 * structure that is the lowest in order, or NULL if the tree is
 * empty. The (sort)order of the tree was determined at every
 * insertion of a value. (Internally, the value of the left most
 * (semi) leaf node is returned.)
 *
 * \param tree
 * The tree in which the first value must be found and returned.
 *
 * \return
 * Pointer to the user defined data structure of the first value, or
 * NULL if the list is empty.
 *
 * \see
 * a_treFindLastValue
 */
void *a_treFindFirstValue(a_treTree tree);


/**
 * \brief
 * Finds and returns the last entry in the tree, that is defined as
 * the highest ordered.
 *
 * This operation returns a pointer to the user defined data
 * structure that is the highest in order, or NULL if the tree is
 * empty. The (sort)order of the tree was determined at every
 * insertion of a value. (Internally, the value of the right most
 * (semi) leaf node is returned.)
 *
 * \param tree
 * The tree in which the last value must be found and returned.
 *
 * \return
 * Pointer to the user defined data structure of the last value, or
 * NULL if the list is empty.
 *
 * \see
 * a_treFindFirstValue
 */
void *a_treFindLastValue(a_treTree tree);


/**
 * \brief
 * Searches for the nearest, higher user defined data structure in a
 * tree, specified by a pointer to an search instance.
 *
 * This operation searches within a tree for the nearest, higher (in
 * order) data structure, not being equal to the given search pointer.
 *
 * \note
 * The user defined data structures are compared, not the pointers to
 * those structures.
 *
 * \param tree
 * The tree in which the value must be searched for.
 *
 * \param valuePtr
 * A pointer to the user defined data structre. valuePtr is expected
 * not to be NULL.
 *
 * \param sortAction
 * Pointer to a user defined function that compares two user defined
 * data structures, for the tree to know its sort order.
 *
 * \return
 * Pointer to the found value or NULL if not found, tree is empty or
 * tree = NULL.
 *
 * \see
 * a_treSortAction a_treFindLowestHigherValue
 */
void *a_treFindLowestHigherValue(a_treTree tree, void *valuePtr, a_treSortAction sortAction);


/**
 * \brief
 * Searches for the nearest, lower user defined data structure in a
 * tree, specified by a pointer to an search instance.
 *
 * This operation searches within a tree for the nearest, lower (in
 * order) data structure, not being equal to the given search pointer.
 *
 * \note
 * The user defined data structures are compared, not the pointers to
 * those structures.
 *
 * \param tree
 * The tree in which the value must be searched for.
 *
 * \param valuePtr
 * A pointer to the user defined data structre. valuePtr is expected
 * not to be NULL.
 *
 * \param sortAction
 * Pointer to a user defined function that compares two user defined
 * data structures, for the tree to know its sort order.
 *
 * \return
 * Pointer to the found value or NULL if not found, tree is empty or
 * tree = NULL.
 *
 * \see
 * a_treSortAction a_treFindLowestHigherValue
 */
void *a_treFindHighestLowerValue(
	a_treTree tree, void *valuePtr, a_treSortAction sortAction);


/**
 * \brief
 * Searches for a value in a tree and, if found, removes the
 * corresponding node that held a pointer to that value.
 *
 * This operation searches within the tree for a node with a value,
 * that is the same of valuePtr, and returns a pointer to the found
 * user defined data structure. The (internal) node that held this
 * pointer will be removed from the tree and its memory will be
 * freed. If needed, the tree will be rebalanced.
 *
 * \note
 * The user defined data structures are compared, not the pointers to
 * those structures.
 *
 * \note
 * Remember to destroy the user defined data structure yourself, as
 * after this point the tree holds no reference to it anymore.
 *
 * \param tree
 * The tree in which the value must be searched for.
 *
 * \param valuePtr
 * A pointer to the user defined data structre. valuePtr is expected
 * not to be NULL.
 *
 * \param sortAction
 * Pointer to a user defined function that compares two user defined
 * data structures, for the tree to know its sort order.
 *
 * \return
 * Pointer to the found value or NULL if not found.
 *
 * \see
 * a_treSortAction
 */
void *a_treFindAndRemoveValue(
	a_treTree tree, void *valuePtr, a_treSortAction sortAction);


/***********************************************************
 COUNT
 ***********************************************************/

/**
 * \brief
 * Returns the number of elements in a tree.
 *
 * This operation counts the nodes in a tree (recursively) and
 * returns the result.
 *
 * \param tree
 * The tree to count its nodes of.
 *
 * \return
 * The number of elements in a tree, 0 if tree is empty (duh!) or -1
 * if tree = NULL.
 */
int a_treCount(a_treTree tree);


/***********************************************************
 WALK
 ***********************************************************/

/**
 * \brief
 * Type Definition for a user defined function that a_treWalk calls
 * back to.
 *
 * \param valuePtr
 * Pointer to a user defined data structure.
 *
 * \param arg
 * Pointer to a user defined context that will be passed along.
 *
 * \return
 * The user defined function must return true (1) in order to let the
 * walk continue. If false (0) is returned, the walk will abort.
 *
 * \warning
 * Do not use any modification functions (insert, delete) while
 * walking a tree.
 *
 * \see
 * a_treWalk
 */
typedef int (*a_treWalkAction)(void *valuePtr, void *arg);


/**
 * \brief
 * Performs a walk over all nodes and calls a user defined function
 * with a pointer to the user defined data structure that the node
 * holds.
 *
 * This operation walks over all nodes, in a sorted order (from low
 * to high), and calls a user defined function for every node,
 * passing on a pointer to the user defined data structure that the
 * node holds, along with a pointer to a user defined context.
 *
 * \param tree
 * The tree to walk over.
 *
 * \param walkAction
 * Pointer to a user defined function to call every node's value
 * pointer with, along with the pointer to the user defined context
 * \a arg.
 *
 * \param arg
 * Pointer to a user defined context that will be passed along to the
 * user defined call back function, without any modification. May be
 * NULL.
 *
 * \return
 * Returns true (1) if the walk was successful, false (0) if the walk
 * was aborted or -1 if tree or walkAction NULL.
 *
 * \see
 * a_treWalkAction
 */
int a_treWalk(a_treTree tree, a_treWalkAction walkAction, void *arg);


/**
 * \brief
 * Type Definition for a user defined function that a_treWalkReverse
 * calls back to.
 *
 * \param valuePtr
 * Pointer to a user defined data structure.
 *
 * \param arg
 * Pointer to a user defined context that will be passed along.
 *
 * \param depth
 * Tree depth for the node that holds the pointer that is passed
 * along to the call back function.
 *
 * \return
 * The user defined function must return true (1) in order to let the
 * walk continue. If false (0) is returned, the walk will abort.
 *
 * \note
 * For test purposes.
 *
 * \warning
 * Do not use any modification functions (insert, delete) while
 * walking a tree.
 *
 * \see
 * a_treWalkReverse
 */
typedef int (*a_treWalkReverseAction)(void *valuePtr, void *arg, int depth);


/**
 * \brief
 * Performs a walk over all nodes in reverse order and calls a user
 * defined function with a pointer to the user defined data structure
 * that the node holds.
 *
 * This operation walks over all nodes, in a reverse order (from high
 * to low), and calls a user defined function for every node, passing
 * on a pointer to the user defined data structure that the node
 * holds, along with a pointer to a user defined context.
 *
 * \param tree
 * The tree to walk over.
 *
 * \param walkAction
 * Pointer to a user defined function to call every node's value
 * pointer with, along with the pointer to the user defined context
 * \a arg.
 *
 * \param arg
 * Pointer to a user defined context that will be passed along to the
 * user defined call back function, without any modification. May be
 * NULL.
 *
 * \return
 * Returns true (1) if the walk was successful, false (0) if the walk
 * was aborted or -1 if tree or walkAction is NULL.
 *
 * \see
 * a_treWalkreverseAction
 */
int a_treWalkReverse(
	a_treTree tree, a_treWalkReverseAction walkAction, void *arg);


#endif   /* A_TRE_H */

//END a_tre.h
