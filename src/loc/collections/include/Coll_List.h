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
#ifndef COLL_LIST_H
#define COLL_LIST_H

#include "Coll_Defs.h"
#include "Coll_Iter.h"

#if defined (__cplusplus)
extern "C" {
#endif
#include "os_if.h"

#ifdef OSPL_BUILD_LOC_COLLECTIONS
#define OS_API OS_API_EXPORT
#else
#define OS_API OS_API_IMPORT
#endif
/* !!!!!!!!NOTE From here no more includes are allowed!!!!!!! */

/*  forward declarations */
typedef struct Coll_List_s Coll_List;


struct Coll_List_s {
    unsigned long _nr_elements;
    Coll_Iter *_first_element;
    Coll_Iter *_last_element;
};

/**
 * \brief Constructor for a Coll_List
 *
 * This method allocates a Coll_List_s structure initialises it and return a pointer to it.
 * No elements are added to the list.
 */
OS_API Coll_List *
Coll_List_new(
     void);

/**
 * \brief Initializer for a preallocated Coll_List
 *
 * This method initializes a preallocated Coll_List_s structure .
 * No elements are added to the list.
 * The method cannot be used on a existing Coll_List which contains elements. This will result in memory leaks.
 */
OS_API void
Coll_List_init(
     Coll_List *_this);

/**
 * \brief Coll_List destructor
 *
 * This destructor frees the memory used by the list header
 * It can only be used when no elements exits in the list. If the list is not empty a COLL_ERROR_NOT_EMPTY is returned.
 */
OS_API long
Coll_List_delete(
     Coll_List *_this);

/**
 * \brief Getter for the number of elements
 */
OS_API unsigned long
Coll_List_getNrOfElements(
    const Coll_List *_this);

/**
 * \brief Getter for an iterator on the first element
 */
OS_API Coll_Iter *
Coll_List_getFirstElement(
    const Coll_List *_this);

/**
 * \brief Getter for an iterator on the last element
 */
OS_API Coll_Iter *
Coll_List_getLastElement(
    Coll_List *_this);

/**
 * \brief Adds an object to the end of the list. The return value a status code indicating if the pushback was success
 * full or not
 *
 * If a problem occurs during element allocation a COLL_ERROR_ALLOC is returned
 * It is allowed to add a NULL object to the list.
 * The same object may be added multiple times to the list.
 */
OS_API long
Coll_List_pushBack(
     Coll_List *_this,
     void *object);

/**
 * \brief Removes an object from the end of the list
 *
 * A pointer to the removed object is returned. If the list is empty NULL is returned.
 */
OS_API void *
Coll_List_popBack(
     Coll_List *_this);


/**
 * \brief Removes an object from the head of the list
 *
 * A pointer to the removed object is returned. If the list is empty NULL is returned.
 */
void *
Coll_List_popFront(
    Coll_List *_this);

/**
 * \brief Returns the pointer to the object that is stored in element number 'index' counting from the start of the list
 *
 * In case index is bigger than the number of elements, NULL is returned
 */
OS_API void *
Coll_List_getObject(
    Coll_List *_this,
    unsigned long index);

#undef OS_API

#if defined (__cplusplus)
}
#endif

#endif /* COLL_LIST_H */
