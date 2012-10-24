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
#ifndef C_COLLECTION_H
#define C_COLLECTION_H

/** \file c_collection.h
 *  \brief This file implements the database collection type class.
 *
 * The database supports built-in collection types that implement containers
 * that can aggregate an arbitrary number of references to literals as well
 * as objects.
 * Collections instances are defined for specific elements type so all elements
 * must be of the same type (or are a subtype of the collections element type).
 * Collection types can be used within application-defined types to implement
 * navigational relations.
 * Collections of objects implement a unidirectional 1-to-1 and 1-to-Many
 * relations.
 * Bi-directional relationships as specified in the ODMG specification are
 * currently not supported, they are hard to implement against the performance
 * requirements and no use cases are defined to justify the effort.
 *
 * The following sub-types are provided:
 *
 *    c_list  : a list collection type that implements a sequence of objects.
 *    c_set   : a set collection type that can hold objects in a random order
 *              where each object can at most occure only once.
 *    c_bag   : a bag collection type that can hold objects in a random order
 *              where each object can occure more than once.
 *    c_table : an indexed collection where objects are identified by a
 *              specific key value.
 *              Each key value has only one occurence in the table.
 *    c_query : an abstract collection that is specified by a predicate and
 *              source collection.
 *
 * The following constructor methods are provided:
 *
 *     c_list   c_listNew      (c_type subType);
 *     c_set    c_setNew       (c_type subType);
 *     c_bag    c_bagNew       (c_type subType);
 *     c_table  c_tableNew     (c_type subType, const c_char *keyNames);
 *     c_query  c_queryNew     (c_collection c, q_expr predicate,
 *                              c_value params[]);
 *
 * The following generic collection methods are provided:
 *
 *     c_type   c_subType      (c_collection c);
 *
 *     c_object c_insert       (c_collection c, c_object o);
 *     c_object c_remove       (c_collection c, c_object o,
 *                              c_removeCondition condition, c_voidp arg);
 *     c_object c_replace      (c_collection c, c_object o,
 *                              c_replaceCondition condition, c_voidp arg);
 *     c_bool   c_exists       (c_collection c, c_object o);
 *     c_object c_find         (c_collection c, c_object template);
 *     c_long   c_count        (c_collection c);
 *     c_object c_read         (c_collection c);
 *     c_object c_take         (c_collection c);
 *     c_iter   c_select       (c_collection c, c_long max);
 *     c_bool   c_readAction   (c_collection c, c_action action, c_voidp arg);
 *     c_bool   c_takeAction   (c_collection c, c_action action, c_voidp arg);
 *     c_bool   c_walk         (c_collection c, c_action action, c_voidp arg);
 *     c_bool   c_unite        (c_collection c1, c_collection c2);
 *     c_bool   c_differ       (c_collection c1, c_collection c2);
 *     c_bool   c_intersect    (c_collection c1, c_collection c2);
 *
 * The following list specific (positional) methods are provided:
 *
 *     c_object c_append       (c_list list, c_object o);
 *     c_list   c_concat       (c_list head, c_list tail);
 *     c_object c_replaceAt    (c_list list, c_object o, c_long index);
 *     c_bool   c_insertAfter  (c_list list, c_object o, c_long index);
 *     c_bool   c_insertBefore (c_list list, c_object o, c_long index);
 *     c_object c_readAt       (c_list list, c_long index);
 *     c_object c_removeAt     (c_list list, c_long index);
 *     c_object c_readLast     (c_list list);
 *
 * The following query specific methods are provided:
 *
 *     c_bool   c_querySetParams (c_query query, c_value params[]);
 *     c_bool   c_queryEval    (c_query query, c_object o);
 *
 * The following table specific methods are provided:
 *
 *     c_iter   c_keyValues    (c_table table, c_object o);
 *
 */

#include "c_base.h"
#include "c_metabase.h"
#include "c_iterator.h"
#include "q_expr.h"
#include "c_querybase.h"

#if defined (__cplusplus)
extern "C" {
#endif
#include "os_if.h"

#ifdef OSPL_BUILD_DB
#define OS_API OS_API_EXPORT
#else
#define OS_API OS_API_IMPORT
#endif
/* !!!!!!!!NOTE From here no more includes are allowed!!!!!!! */

C_CLASS(c_collection);

#define C_UNLIMITED (0)

typedef c_bool (*c_action)
               (c_object o, c_voidp arg);

typedef c_bool (*c_removeCondition)
               (c_object found, c_object requested, c_voidp arg);

typedef c_bool (*c_replaceCondition)
               (c_object original, c_object replacement, c_voidp arg);

typedef c_collection c_list;
typedef c_collection c_set;
typedef c_collection c_bag;
typedef c_collection c_table;
typedef c_collection c_query;

/**
 * \brief This operation constructs a list collection object.
 *
 * A list is an ordered sequence of elements, in which elements may have more
 * than one occurrence. The list is a subtype of c_collection and implements
 * the operations specified by the collection class. In addition the list
 * implements some extra positional operations.
 *
 * \param subType The type of the elements the set can contain.
 *
 * \return On a successful operation the created list object.
 *         Otherwise this method will return NULL, detailed error information
 *         is reported to the os report facility.
 */
OS_API c_list
c_listNew (
    c_type subType);

/**
 * \brief This operation constructs a set collection object.
 *
 * Set objects are unordered collections of elements, in which elements have
 * at most one occurrence. The c_set class is a subtype of c_collection and
 * implements the operations specified by the collection class.
 *
 * \param subType The type of the elements the set can contain.
 *
 * \return On a successful operation the created set object.
 *         Otherwise this method will return NULL, detailed error information
 *         is reported to the os report facility.
 */
OS_API c_set
c_setNew (
    c_type subType);

/**
 * \brief This operation constructs a bag collection object.
 *
 * Bag objects are unordered collections of elements, in which elements may
 * have more than one occurrence. The c_bag class is a subtype of c_collection
 * and implements the operations specified by the collection class.
 *
 * \param subType The type of the elements the bag can contain.
 *
 * \return On a successful operation the created bag object.
 *         Otherwise this method will return NULL, detailed error information
 *         is reported to the os report facility.
 */
OS_API c_bag
c_bagNew (
    c_type subType);

/**
 * \brief This operation constructs a table collection object.
 *
 * Table objects are collections of elements that are identified by their key
 * value. The key value of an element is specified as a string of comma
 * separated field names.
 * The specified key fields are part of the element type. Tables can at most
 * have one occurrence for each key value. The c_table class extends the
 * c_collection class with the constructor operation. The constructor
 * operation takes a type as parameter that specifies the element type the set
 * accepts.
 *
 * \param subType The element type of the table that must be created.
 * \param keyNames A comma seperated list of field names that specify the
 * table key value.
 *
 * \return On a successful operation the created table object.
 *         Otherwise this method will return NULL, detailed error information
 *         is reported to the os report facility.
 */
OS_API c_table
c_tableNew (
    c_type subType,
    const c_char *keyNames);

/**
 * \brief This operation constructs a query collection object.
 *
 * A Query object is a collections of elements specified by a predicate and
 * source collection. The elements associated to the query are the elements
 * from the source collection that fulfill the query specific predicate.
 *
 * \param source The source collection on which the query collection is defined.
 * \param predicate The predicate for which all query elements evaluate to true.
 * \param params[] The list of paramater values associated to the given
 *                 predicate.
 *
 * \return On a successful operation the created query is returned.
 *         Otherwise this method will return NULL, detailed error information
 *         is reported to the os report facility.
 */
OS_API c_query
c_queryNew (
    c_collection source,
    q_expr predicate,
    c_value params[]);

/**
 * \brief This operation returns the element type of the given collection type.
 *
 *  This operation returns the element type of the given collection type.
 *
 * \param c The collection object this method operates on.
 *
 * \return If the specified object is not a collection type this operation
 *         will return NULL.
 */
OS_API c_type
c_subType (
    c_collection c);


/**
 * \brief This operation will insert the given object into the specified
 *        collection.
 *
 * If the object is successfully inserted the operation will return the object
 * as being an element of the collection. In case the object is not inserted
 * the operation may either return NULL indicating that an error has occurred
 * or return another object indicating that the insert failed due to the
 * existence of the returned object.
 * Note that if successful inserted the reference count of the object is
 * increased because the list now references the object but the returning the
 * object will not extra increase the reference count.
 *
 * \param c The collection where the given object will be inserted.
 * \param o The object that must be inserted into the given collection.
 *
 * \return The object that is found to be an element of the collection.
 */
OS_API c_object
c_insert (
    c_collection c,
    c_object o);

/**
 * \brief This operation will remove the object from the specified collection
 *        that is identified by the given object.
 *
 * The actual collections subtype defines identification semantics.
 *
 * \param c The collection where the given object will be removed.
 * \param o The object that must be removed from the given collection.
 *
 * \return The operation will return the object that is removed from the
 *         collection.
 *         The operation will return NULL if no object is found or an illegal
 *         collection is specified.
 */
OS_API c_object
c_remove(
    c_collection c,
    c_object o,
    c_removeCondition condition,
    c_voidp arg);

/**
 * \brief This operation will conditionally insert the given object into the
 *        specified collection.
 *
 * In case another object is identified identical according to the comparison
 * semantics of the actual collection type this operation will execute the
 * specified condition operation that an application can provide that will
 * decide whether it will replace the existing object with the given object.
 * The existing and the to be inserted objects are passed to the condition
 * operation and the condition operation will return TRUE if the existing
 * object must be replaced.
 * Note that depending on the semantics of the actual collection type the
 * object that is replaced may be the same as the object inserted, the actual
 * result in that case will be that the reference count of the object is
 * increased.
 *
 * \param c The collection where the given object will be inserted.
 * \param o The object that must be inserted from the given collection.
 * \param condition The condition that will be applied to the existing and
 *                  new object to determine if the existing should be replaced.
 * \param arg The argument that is passed to the condition function as
 *            application hook.
 *
 * \return The replaced object is returned by this operation.
 *         NULL is returned if no object is replaced or an illegal collection
 *         is specified.
 */
OS_API c_object
c_replace(
    c_collection c,
    c_object o,
    c_replaceCondition condition,
    c_voidp arg);

/**
 * \brief This operation will check if the specified object is a member of the
 *        specified collection.
 *
 * \param c The collection where to look for the specified object.
 * \param o The object to look for.
 *
 * \return TRUE if the object is found otherwise FALSE is returned.
 */
OS_API c_bool
c_exists (
    c_collection c,
    c_object o);

/**
 * \brief This operation will lookup the member of the specified collection
 *        by the given template.
 *
 * Note that when the uniqueness of a member is determined by the pointer of
 * the object and the object can be found, the operation returns the template
 * object with the reference count increased.
 *
 * \param c The collection to look for the object specified by the template
 *          parameter.
 * \param templ The template specifies the object to look for.
 *
 * \return the object specified by the template paramter.
 *         NULL is returned when the object could not be found.
 */
OS_API c_object
c_find (
    c_collection c,
    c_object templ);

/**
 * \brief This operation will return the actual number of element that reside
 *        in the specified collection.
 *
 * This operation will return the actual number of element that reside in the
 * specified collection.
 *
 * \param c The collection this operation operates on.
 *
 * \return This operation will return 0 when an illegal collection is specified.
 */
OS_API c_long
c_count (
    c_collection c);

/**
 * \brief This operation will read the first element of the specified
 *        collection.
 *
 * This operation will read the first element of the specified collection.
 * The definition of first depends on the semantics of the actual collection
 * type.
 *
 * \param c The collection this operation operates on.
 *
 * \return The read will return a new reference to the first element.
 *         If the specified collection has no elements or is illegal the
 *         operation will return NULL.
 */
OS_API c_object
c_read (
    c_collection c);

/**
 * \brief This operation will take (read and remove) the first element of the
 *        specified collection.
 *
 * This operation will take (read and remove) the first element of the
 * specified collection. The definition of first depends on the semantics of
 * the actual collection type.
 *
 * \param c The collection this operation operates on.
 *
 * \return The take will return the element without altering the reference
 *         count. If the specified collection has no elements or is illegal
 *         the operation will return NULL.
 */
OS_API c_object
c_take (
    c_collection c);

/**
 * \brief This operation will read a list of element from a given collection.
 *
 * This operation will read a list of element from the specified collection
 * and return an iterator containing references to the read elements.
 * The elements are not removed from the collection so the reference count of
 * the read elements will be increased. The max parameter specifies the maximum
 * number of element that may be read; if the collection contains fewer
 * elements the amount that can be read is returned.
 *
 * \param
 *
 * \return
 */
OS_API c_iter
c_select (
    c_collection c,
    c_long max);

/**
 * \brief
 *
 *
 *
 * \param
 *
 * \return
 */
OS_API c_bool
c_readAction (
    c_collection c,
    c_action action,
    c_voidp arg);

/**
 * \brief
 *
 *
 *
 * \param
 *
 * \return
 */
OS_API c_bool
c_takeAction (
    c_collection c,
    c_action action,
    c_voidp arg);

/**
 * \brief This operation will execute the given action on all elements of the
 *        specified collection.
 *
 * This operation will visit all elements that reside within the specified
 * collection and execute the given action operation upon each visited element.
 * Each time the action operation is invoked the visited element and the
 * specified action argument actionArg are passed to the action operation.
 * The action operation must return TRUE to continue the walk action, if FALSE
 * is returned the walk is aborted and the c_walk operation will also return
 * FALSE to indicate an aborted walk.
 *
 * \param c The collection this method operates on.
 * \param action The action method which is inviked on all visited elements of
 *               this collection.
 * \param arg The generic application hook attribute that is passed to the
 *            action method.
 *
 * \return
 */
OS_API c_bool
c_walk (
    c_collection c,
    c_action action,
    c_voidp arg);

/**
 * \brief This operation will determine the union of two collections.
 *
 * The element type of both collections must be the same. The first collection
 * specified will be modified and contain the result union of both collection.
 *
 * \param c1 The first source and result collection of this operation.
 * \param c2 The second source collection of this operation.
 *
 * \return This operation will return TRUE on a successful operation and
 *         c1 = c1+c2. otherwise e.g. if the element types of both collection
 *         are not equal FALSE is returned.
 */
OS_API c_bool
c_unite (
    c_collection c1,
    c_collection c2);

/**
 * \brief This operation will determine the difference of two collections.
 *
 * The element type of both collections must be the same. The first collection
 * specified will be modified and contain the result difference of both
 * collection.
 *
 * \param c1 The first source and result collection of this operation.
 * \param c2 The second source collection of this operation.
 *
 * \return This operation will return TRUE on a successful operation and
 *         c1 = c1-c2. otherwise e.g. if the element types of both collection
 *         are not equal FALSE is returned.
 */
OS_API c_bool
c_differ (
    c_collection c1,
    c_collection c2);

/**
 * \brief This operation will determine the intersection of two collections.
 *
 * The element type of both collections must be the same. The first collection
 * specified will be modified and contain the result intersection of both
 * collection.
 *
 * \param c1 The first source and result collection of this operation.
 * \param c2 The second source collection of this operation.
 *
 * \return This operation will return TRUE on a successful operation and
 *         c1 = c1 ^ c2. otherwise e.g. if the element types of both
 *         collection are not equal FALSE is returned.
 */
OS_API c_bool
c_intersect (
    c_collection c1,
    c_collection c2);


/**
 * \brief This operation will append the given object to the specified list.
 *
 * This operation will append the given object to the specified list.
 * This operation will return the append object as a convenience meaning that
 * the reference count is only increased as result of the insertion into the
 * list but not as result of returning the object.
 *
 * \param list The list where this method operates on.
 * \param o The object that will be append to the specified list.
 *
 * \return The object that is appand to the list.
 */
OS_API c_object
c_append (
    c_list list,
    c_object o);

/**
 * \brief This operation wil concat to lists.
 *
 * This operation will destroy the tail list and append all tail elements to the
 * head list preserving the order of the elements.
 *
 * \param head The head list that will also be the result list.
 * \param tail the tail list that will be destroyed by this operation.
 *
 * \return the reasult list (being the same as the head list).
 */
OS_API c_list
c_concat (
    c_list head,
    c_list tail);

/**
 * \brief This operation will replace the nth element of the specified list
 *        with the given object.
 *
 * This operation will replace the nth element of the specified list with the
 * given object. The operation will return the replaced element. The object
 * will not be inserted if the specified index is out of range, in that case
 * the operation will return the give object.
 *
 * \param list The list that this method operates on.
 * \param o The object that must be inserted.
 * \param index The position in the list where the object must be inserted.
 *
 * \return The object that is either removed or not inserted.
 */
OS_API c_object
c_replaceAt (
    c_list list,
    c_object o,
    c_long index);

/**
 * \brief This operation will insert an object after a specific object in a
 *        given list.
 *
 * This operation will insert the given object in the specified list after
 * the nth element that is identified by the index parameter.
 *
 * \param list The list that this method operates on.
 * \param o The object that must be inserted.
 * \param index The position in the list of the list element where after the
 *              object must be inserted.
 *
 * \return TRUE if the given object is successfully inserted.
 *         Otherwise FALSE if the specified index is out of range.
 */
OS_API c_bool
c_insertAfter (
    c_list list,
    c_object o,
    c_long index);

/**
 * \brief This operation will insert an object beforea specific object in a
 *        given list.
 *
 * This operation will insert the given object in the specified list before
 * the nth element that is identified by the index parameter.
 *
 * \param list The list that this method operates on.
 * \param o The object that must be inserted.
 * \param index The position in the list of the list element where before the
 *              object must be inserted.
 *
 * \return TRUE if the given object is successfully inserted.
 *         Otherwise FALSE if the specified index is out of range.
 */
OS_API c_bool
c_insertBefore (
    c_list list,
    c_object o,
    c_long index);

/**
 * \brief This operation will return a reference to the list element
 *        identified by the given list index.
 *
 * This operation will return a reference to the list element identified
 * by the given list index.
 *
 * \param list The list that this method operates on.
 * \param index The index that specifies the list element.
 *
 * \return the list element identified by the given list index.
 *         or otherwise return NULL if the specified list index is out of range.
 */
OS_API c_object
c_readAt (
    c_list list,
    c_long index);

/**
 * \brief This operation will remove and return the list element identified by
 *        the given list index.
 *
 * This operation will remove and return the list element identified by the
 * given list index.
 *
 * \param list The list that this method operates on.
 * \param index The index that specifies the list element.
 *
 * \return the list element identified by the given list index.
 *         or otherwise return NULL if the specified list index is out of range.
 */
OS_API c_object
c_removeAt (
    c_list list,
    c_long index);

/**
 * \brief This operation will return a reference to the last list element.
 *
 *  This operation will return a reference to the last list element.
 *
 * \param
 *
 * \return
 */
OS_API c_object
c_readLast (
    c_list list);

/**
 * \brief This operation will return a reference to the next table element.
 *
 *  This operation will return a reference to the next table element.
 *  The next table element is the next greater key value compared to the
 *  specified object.
 *
 * \param
 *
 * \return
 */
OS_API c_object
c_tableNext (
    c_table table,
    c_object o);

/**
 * \brief This query operation sets the value of the query parameters.
 *
 * This query operation sets the value of the query parameters addressed by the
 * query predicate. These values are taken into account the next read or take
 * operation.
 *
 * \param query The query that this method operates on.
 * \param params The array of query parameters values that must be set.
 *
 * \return True if the operation was successful otherwise False.
 */
OS_API c_bool
c_querySetParams (
    c_query _this,
    c_value params[]);

/**
 * \brief This query operation evaluates the value of the specified object
 *        against the query predicate and returns the result value.
 *
 * This query operation evaluates the value of the specified object against
 * the query predicate and returns the result value.
 *
 * \param query The query that this method operates on.
 * \param o The object that will be evaluated.
 *
 * \return The result of the evaluation.
 */
OS_API c_bool
c_queryEval (
    c_query _this,
    c_object o);

/**
 * \brief This operation returns the predicate that belongs to the given query.
 *
 * \param q the query of which the predicate must be returned
 * \return the predicate of the given query
 */
OS_API c_qPred
c_queryGetPred(
		c_query _this);

/**
 * \brief This operation sets the predicate of the given query to the given predicate.
 *
 * \param q the query who's predicate must be set
 * \param p the predicate to which the query's predicate must be set
 */
OS_API void
c_querySetPred(
		c_query _this,
		c_qPred p);

/**
 * \brief This table operation inspects the specified object and fills a
 *        given array with its key values. The keys used
 *        are those defined at the table construction.
 *
 * This table operation inspects the specified object fills a given array
 * with its key values. The keys used
 * are those defined at the table construction.
 *
 * \param table  The table containing the key-field information
 * \param object The object that will be evaluated.
 * \param values The array to be filled with values
 *
 * \return The number of keys returned in the array
 */
OS_API c_long
c_tableGetKeyValues (
    c_table _this,
    c_object object,
    c_value *values);

OS_API c_long
c_tableSetKeyValues (
    c_table _this,
    c_object object,
    c_value *values);

OS_API c_long
c_tableNofKeys (
    c_table _this);

OS_API c_array
c_tableKeyList (
    c_table _this);

OS_API c_char *
c_tableKeyExpr (
    c_table _this);

OS_API c_long
c_tableCount (
    c_table _this);

OS_API c_object
c_tableInsert (
    c_table _this,
    c_object o);

OS_API c_object
c_tableRemove (
    c_table _this,
    c_object o,
    c_removeCondition condition,
    c_voidp arg);

#define _NIL_
#ifdef _NIL_
OS_API c_object
c_tableFind (
    c_table _this,
    c_value *keyValues);
#endif

OS_API c_bool
c_tableWalk(
    c_table _this,
    c_action action,
    c_voidp actionArg);

OS_API c_long
c_setCount(
    c_set _this);

OS_API c_object
c_setInsert(
    c_set _this,
    c_object o);

OS_API c_object
c_setRemove (
    c_set _this,
    c_object o,
    c_removeCondition condition,
    c_voidp arg);

OS_API c_bool
c_setWalk(
    c_set _this,
    c_action action,
    c_voidp actionArg);

OS_API c_long
c_bagCount(
    c_bag _this);

OS_API c_object
c_bagInsert (
    c_bag _this,
    c_object o);

OS_API c_bool
c_bagWalk(
    c_bag _this,
    c_action action,
    c_voidp actionArg);

OS_API c_long
c_listCount(
    c_list _this);

OS_API c_object
c_listInsert(
    c_list _this,
    c_object o);

OS_API c_bool
c_listWalk(
    c_list _this,
    c_action action,
    c_voidp actionArg);

OS_API c_object
c_listTemplateRemove (
    c_list _this,
    c_action condition,
    c_voidp arg);

OS_API c_array
c_arrayNew(
    c_type subType,
    c_long length);

OS_API c_sequence
c_sequenceNew(
    c_type subType,
    c_long maxsize,
    c_long length);

#define c_array(c) ((c_array)(c))
#define c_sequence(c) ((c_sequence)(c))

#undef OS_API

#if defined (__cplusplus)
}
#endif

#endif
