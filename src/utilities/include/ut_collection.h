/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR PrismTech
 *   Limited, its affiliated companies and licensors. All rights reserved.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 */
#ifndef UT_COLLECTION_H
#define UT_COLLECTION_H

/** \file ut_collection.h
 *  \brief This file implements collection type class.
 *
 * A collection represents a group of objects, known as its elements. Some
 * collections allow duplicate elements and others do not. Some are ordered
 * and others unordered. This interface provides an abstract constructor
 * function in which a specific kind of collection is created. The constructor
 * requires a pointer to a comparision function so the collection implementation
 * is able to compare elements.
 * Collections instances are defined for specific elements type so all elements
 * must be of the same type.
 * Collection types can be used within application-defined types to implement
 * navigational relations. Collections of objects implement a unidirectional
 * 1-to-1 and 1-to-many relations.
 *
 * At this moment the following sub-types are provided:
 *
 *    ut_table : an indexed collection where objects are identified by a specific
 *               key value. Each key value has only one occurence in the table.
 *
 * The following collection constructor method is provided:
 *
 *     ut_collection  ut_listNew      (ut_compareElementsFunc cmpFunc, c_voidp arg);
 *     ut_collection  ut_setNew       (ut_compareElementsFunc cmpFunc, c_voidp arg);
 *     ut_collection  ut_bagNew       (ut_compareElementsFunc cmpFunc, c_voidp arg);
 *     ut_collection  ut_tableNew     (ut_compareElementsFunc cmpFunc, c_voidp arg);
 *
 * The following destructor method is provided:
 *     void      ut_collectionFree (ut_collection c,
 *                                  ut_freeElementsFunc action,
 *                                  c_voidp arg);
 *
 * The following generic collection methods are provided:
 *
 *     c_bool    ut_add      (ut_collection c, c_object o);
 *     c_object  ut_get      (ut_collection c, c_object o);
 *     c_object  ut_remove   (ut_collection c, c_object o);
 *     c_bool    ut_contains (ut_collection c, c_object o);
 *     c_long    ut_count    (ut_collection c);
 *     c_bool    ut_walk     (ut_collection c, c_action action, c_voidp arg);
 *
 * The following list specific (positional) methods are provided:
 *
 *     c_object     ut_getAt    (ut_list list, c_long index);
 *     c_object     ut_removeAt (ut_list list, c_long index);
 *
 * The following table specific methods are provided:
 *
 *     c_bool       ut_put      (ut_table t, c_object key, c_object value);
 *
 */
#include "os_defs.h"
#include "os_classbase.h"
#include "os_iterator.h"

#if defined (__cplusplus)
extern "C" {
#endif
#include "os_if.h"

#ifdef OSPL_BUILD_CORE
#define OS_API OS_API_EXPORT
#else
#define OS_API OS_API_IMPORT
#endif
/* !!!!!!!!NOTE From here no more includes are allowed!!!!!!! */

#define ut_collection(c) ((ut_collection)(c))
#define ut_table(t) ((ut_table) (t))
#define ut_set(s) ((ut_set) (s))

OS_CLASS(ut_collection);

/* The next classes are all specializations of ut_collection */
OS_CLASS(ut_list);
OS_CLASS(ut_set);
OS_CLASS(ut_bag);
OS_CLASS(ut_table);

typedef enum ut_collectionType {
    UT_LIST,
    UT_SET,
    UT_BAG,
    UT_TABLE
}ut_collectionType;

/**
 * \brief Callback function that compares its two arguments for order.
 *
 * Returns C_LT, C_EQ, or a C_GT as the first argument is less than,
 * equal to, or greater than the second.
 * This operation helps the collection implementation to determine how an
 * element should be ordered or stored. A table uses a key-value combination to
 * store elements. The compare function should be able to order the key values.
 *
 * \param o1 The object to which the second object is compared.
 * \param o2 The second object that is compared.
 * \param args User data.
 *
 * \return  C_LT, C_EQ, or a C_GT as the first argument is less than,
 *          equal to, or greater than the second.
 */
typedef os_equality (*ut_compareElementsFunc) (void *o1, void *o2, void *args);

/**
 * \brief Callback function which is called for all collection elements.
 *
 * The ut_walk function visits all elements of a collection and calls this
 * callback function for the visited elements.
 * The user can implement this function and perform operations on the visited
 * elements. The action operation must return TRUE to continue the walk action,
 * if FALSE is returned the walk is aborted and the ut_walk operation will
 * also return FALSE to indicate an aborted walk.
 *
 * \param o     The currently visited object.
 * \param arg   User provided data.
 *
 * \return 0 when the walk action should stop, otherwise return any other value.
 */
typedef os_int32 (*ut_actionFunc)(void *o, void *arg);

/**
 * \brief Callback function which is called for all key/value pairs in the collection.
 *
 * The ut_walk function visits all key/value pairs of a collection and calls this
 * callback function for the visited pairs.
 * The user can implement this function and perform operations on the visited
 * pairs. The action operation must return TRUE to continue the walk action,
 * if FALSE is returned the walk is aborted and the ut_walk operation will
 * also return FALSE to indicate an aborted walk.
 *
 * \param k     The key of the currently visited object.
 * \param o     The currently visited object.
 * \param arg   User provided data.
 *
 * \return 0 when the walk action should stop, otherwise return any other value.
 */
typedef os_int32 (*ut_actionKeyValueFunc)(void *k, void *o, void *arg);

/**
 * \brief Free an elements stored within the collection.
 *
 * This callback function is called when ut_collectionFree is called. It
 * enables the user to free the elements contained within the collection.
 *
 */
typedef void (*ut_freeElementFunc)(void *o, void *arg);

/**
 * \brief This operation constructs a set collection object.
 *
 * Set objects are unordered collections of elements, in which elements have at most
 * one occurrence. The c_set class is a subtype of c_collection and implements the
 * operations specified by the collection class.
 *
 * \param cmpFunc Compare function for elements stored in this set.
 * \param arg Arguments for the compare function.
 *
 * \return On a successful operation the created set object.
 *         Otherwise this method will return NULL, detailed error information is reported
 *         to the os report facility.
 */
OS_API ut_set
ut_setNew(
    const ut_compareElementsFunc cmpFunc,
    void *cmpArg,
    const ut_freeElementFunc freeValue,
    void *freeValueArg);

#ifndef NDEBUG
/**
 * \brief This operation constructs a list collection object.
 *
 * A list is an ordered sequence of elements, in which elements may have more
 * than one occurrence. The list is a subtype of c_collection and implements the
 * operations specified by the collection class. In addition the list implements some
 * extra positional operations.
 *
 * \param cmpFunc Compare function for elements stored in this list.
 * \param arg Arguments for the compare function.
 *
 * \return On a successful operation the created list object.
 *         Otherwise this method will return NULL, detailed error information is reported
 *         to the os report facility.
 */
OS_API ut_collection
ut_listNew(
    const ut_compareElementsFunc cmpFunc,
    void *arg);

/**
 * \brief This operation constructs a bag collection object.
 *
 * Bag objects are unordered collections of elements, in which elements may have more
 * than one occurrence. The c_bag class is a subtype of c_collection and implements the
 * operations specified by the collection class.
 *
 * \param cmpFunc Compare function for elements stored in this bag.
 * \param arg Arguments for the compare function.
 *
 * \return On a successful operation the created bag object.
 *         Otherwise this method will return NULL, detailed error information is reported
 *         to the os report facility.
 */
OS_API ut_collection
ut_bagNew(
    const ut_compareElementsFunc cmpFunc,
    void *arg);
#endif /* NDEBUG */

/**
 * \brief This operation constructs a table collection object.
 *
 * Table objects are collections of elements that are identified by their key
 * value. The key value of an element can be of any format as long as the
 * ut_compareElementsFunc can order them. Tables can at most have one occurrence
 * for each key value.
 *
 * The freeKey and freeValue callback functions enables the developer to free
 * the keys and values that are stored in the table. It is assumed that the
 * application will take responsibility for the keys and values in the table.
 *
 * If no callback functions are provided, non empty references to keys and
 * values can be lost.
 *
 * \param cmpFunc      Compare function for elements stored in this table.
 * \param cmpArg       Arguments for the compare function.
 * \param freeKey      User provided callback function that takes responsibility
 *                     of the keys within the table.
 * \param freeKeyArg   The generic application hook attribute that is passed to
 *                     the freeKey method.
 * \param freeValue    User provided callback function that takes responsibility
 *                     of the values within the table.
 * \param freeValueArg The generic application hook attribute that is passed to
 *                     the freeValue method.
 *
 * \return On a successful operation the created table object.
 *         Otherwise this method will return NULL, detailed error information is reported
 *         to the os report facility.
 */
OS_API ut_table
ut_tableNew(
    const ut_compareElementsFunc cmpFunc,
    void *cmpArg,
    const ut_freeElementFunc freeKey,
    void *freeKeyArg,
    const ut_freeElementFunc freeValue,
    void *freeValueArg);

/**
 * \brief Clears all allocated elements for a specified collection, but not the
 * collection itself.
 *
 * This method clears all elements contained in the collection, deleting them
 * using the deletion functions provided to the collection's constructor. If no
 * deletion functions were provided, elements can be lost when clearing the collection.
 *
 * \param c         The collection that will be cleared.
 */
OS_API void
ut_clear(
    ut_collection c);

/**
 * \brief Frees the allocated resources for a specified collection.
 *
 * This method clears all elements contained in the collection, deleting them
 * using the deletion functions provided to the collection's constructor. Then
 * it will proceed to delete the resources used by the collection administration
 * itself. If no deletion functions were provided, elements can be lost.
 *
 * \param c      The collection that will be freed.
 */
OS_API void
ut_collectionFree(
    ut_collection c);

/**
 * \brief Returns the object stored in the collection.
 *
 * This operation finds the specified object and returns it. The object itself
 * will remain an element in the collection. The user should not free the
 * returned object because it should first be removed from the collection.
 *
 * \param c The collection where the given object will be inserted.
 * \param o The object that must be inserted into the given collection.
 *
 * \return The object that is found to be an element of the collection.
 */
OS_API void *
ut_get(
    ut_collection c,
    void *o);

/**
 * \brief This operation will remove the given object from the specified collection.
 *
 * When the underlaying collection type allows duplicate elements this operation
 * will remove the first occurence. If the collection does not contain the
 * given element, NULL will be returned. See function ut_listRemoveAt for a
 * ut_list specific remove operation.
 * NOTE that this operation does not free the object! It is merely removed from
 * the collection.
 * \param c The collection where the given object will be removed from.
 * \param o The object that must be removed from the given collection.
 *          When using a ut_table this param is used for the key.
 *
 * \return The operation will return the object that is removed from the collection.
 *         The operation will return NULL if no object is found or an illegal
 *         collection is specified.
 */
OS_API void *
ut_remove(
    ut_collection c,
    void *o);

/**
 * \brief This operation will check if the specified object is a member of the
 * specified collection.
 *
 * \param c The collection where to look for the specified object.
 * \param o The object to look for.
 *
 * \return 0 if the object is NOT found, otherwise any other value might be returned.
 */
OS_API os_int32
ut_contains(
    ut_collection c,
    void *o);

/**
 * \brief This operation will return the actual number of element that reside
 *        in the specified collection.
 *
 * This operation will return the actual number of element that reside in the
 * specified collection.
 *
 * \param c The collection this operation operates on.
 *
 * \return This operation will return -1 when an illegal collection is specified.
 */
OS_API os_int32
ut_count(
    ut_collection c);

/**
 * \brief This operation will execute the given action on all elements of the
 *        specified collection.
 *
 * This operation will visit all elements of the specified collection and
 * execute the given action operation upon each visited element.
 * Each time the action operation is invoked the visited element and the
 * specified action argument actionArg are passed to the action operation.
 * The action operation must return TRUE to continue the walk action, if FALSE
 * is returned the walk is aborted and the c_walk operation will also return
 * FALSE to indicate an aborted walk. The elements will be visited in an
 * ascending order.
 *
 * \param c      The collection this method operates on.
 * \param action The action method which is involked on all visited elements
 *               of this collection.
 * \param arg    The generic application hook attribute that is passed to
 *               the action method.
 *
 * \return
 */
OS_API os_int32
ut_walk(
    ut_collection c,
    const ut_actionFunc action,
    void *arg);

/**
 * \brief This operation will add the given element into the specified set.
 *
 * If the element is successfully inserted the operation will return TRUE.
 * FALSE indicates an error has occurred or that the element was already
 * contained. The value should be stored on heap since the table will
 * store the key pointer.
 *
 * \param s The set where the given element will be inserted.
 * \param value The element that needs to be stored.
 *
 * \return TRUE when successfully inserted. FALSE when already in use
 * or when an error occured.
 */
OS_API os_int32
ut_setInsert(
    ut_set s,
    void *value);

/**
 * \brief Clears all allocated elements for the specified set, but not the
 * set itself.
 *
 * This method clears all elements contained in the set, deleting them
 * using the deletion functions provided to the set's constructor. If no
 * deletion functions were provided, elements can be lost when clearing the
 * set.
 *
 * \param set     The set that will be cleared.
 */OS_API void
ut_setClear(
    ut_set set);

/**
 * \brief Frees the allocated resources for a specified set.
 *
 * \param set        The set that will be freed.
 */
OS_API void
ut_setFree(
    ut_set set);

/**
 * \brief This operation will add the given object into the specified table.
 *
 * If the object is successfully inserted the operation will return TRUE.
 * FALSE indicates an error has occurred or that the key was already used.
 * The key value should be stored on heap since the table will
 * store the key pointer.
 *
 * \param t The table where the given object will be inserted.
 * \param key Key with which the specified value is to be associated.
 * \param value value to be associated with the specified key.
 *
 * \return 0 if the element has not been stored in the table. Otherwise a
 *           value not equal to 0 is returned.
 */
OS_API os_int32
ut_tableInsert(
    ut_table t,
    void *key,
    void *value);

/**
 * \brief This operation will execute the given action on all key/value pairs of the
 *        specified table.
 *
 * This operation will visit all key/value pairs of the specified table and
 * execute the given action operation upon each visited pair.
 * Each time the action operation is invoked the visited pair and the
 * specified action argument actionArg are passed to the action operation.
 * The action operation must return TRUE to continue the walk action, if FALSE
 * is returned the walk is aborted and the c_walk operation will also return
 * FALSE to indicate an aborted walk. The elements will be visited in an
 * ascending order.
 *
 * \param t      The table this method operates on.
 * \param action The action method which is involked on all visited pairs
 *               of this table.
 * \param arg    The generic application hook attribute that is passed to
 *               the action method.
 *
 * \return
 */

OS_API os_int32
ut_tableKeyValueWalk(
    ut_table t,
    const ut_actionKeyValueFunc action,
    void *arg);

/**
 * \brief This operation will return a reference to the next table element.
 *
 *  This operation will return a reference to the next table element.
 *  The next table element is the next greater key value compared to the
 *  specified object.
 *
 * \param table The table this method operates on.
 * \param o     The element relative to which the next greater element is searched.
 *
 * \return The element that has a greater key than the specified object.
 */
OS_API void *
ut_tableNext(
    ut_table table,
    void *o);

/**
 * \brief Clears all allocated elements for the specified table, but not the
 * table itself.
 *
 * This method clears all elements contained in the table, deleting them
 * using the deletion functions provided to the table's constructor. If no
 * deletion functions were provided, elements can be lost when clearing the
 * table.
 *
 * \param table     The table that will be cleared.
 */
OS_API void
ut_tableClear(
    ut_table table);

/**
 * \brief Frees the allocated resources for a specified table.
 *
 * \param table        The table that will be freed.
 */
OS_API void
ut_tableFree(
    ut_table table);

#undef OS_API

#if defined (__cplusplus)
}
#endif

#endif
