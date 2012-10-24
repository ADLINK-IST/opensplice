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
#ifndef COLL_SET_H
#define COLL_SET_H

#include "Coll_Iter.h"

#if defined (__cplusplus)
extern "C" {
#endif

/*  forward declarations */
typedef struct Coll_Set_s Coll_Set;


struct Coll_Set_s {
    long      _nr_elements;
    int       (*_isLess)(void *left, void *right);
    int       _is_sorted;
    Coll_Iter *_first_element;
    Coll_Iter *_last_element;
};


/**
 * \brief Constructor for a Coll_Set
 *
 * This method allocates a Coll_Set_s structure initialises it and return a pointer to it.
 * No elements are added to the set.
 *
 * \param sorted specifies if the set keeps the element sorted or not. If set to yes the add, remove and contains functions
 * are more quickly but it is not allowed to use
 */
Coll_Set *
Coll_Set_new(
     int (* const isLess)(void *, void*),
     int sorted);

/**
 * \brief Initializer for a preallocated Coll_Set
 *
 * This method initializes a preallocated Coll_Set_s structure .
 * No elements are added to the set.
 * The method cannot be used on a existing Coll_Set which contains elements. This will result in memory leaks.
 *
 * \param sorted specifies if the set keeps the element sorted or not. If set to yes the add, remove and contains functions
 * are more quickly but it is not allowed to use */
void
Coll_Set_init(
     Coll_Set *_this,
     int (* const isLess)(void *, void*),
     int sorted);

/**
 * \brief Coll_Set destructor
 *
 * This destructor frees the memory used by the set header
 * It can only be used when no elements exits in the set. If the set is not empty a COLL_ERROR_NOT_EMPTY is returned.
 */
long
Coll_Set_delete(
     Coll_Set *_this);

/**
 * \brief Getter for the number of elements in the Set
 */
long
Coll_Set_getNrOfElements(
    const Coll_Set *_this);

/**
 * \brief Getter for an iterator on the first element
 */
Coll_Iter *
Coll_Set_getFirstElement(
    const Coll_Set *_this);

/**
 * \brief Getter for an iterator on the last element
 */
Coll_Iter *
Coll_Set_getLastElement(
    Coll_Set *_this);

/**
 * \brief Inserts an object to the set
 *
 * If a problem occurs during element allocation a COLL_ERROR_ALLOC is returned.
 * Objects can only exist once in a set. Adding an object which already exists in the Set has no effect.
 * It is allowed to add NULL to the set.
 */
long
Coll_Set_add(
     Coll_Set *_this,
     void *object);

/**
 * \brief Inserts an object to the set of which the user knows that it is not present in the set.
 *
 * The unique object is added at the end of the Set. The Set needs to be initialized as unsorted.
 * COLL_ERROR_PRECONDITION_NOT_MET is returned if the method is called on a sorted set.
 * If a problem occurs during element allocation a COLL_ERROR_ALLOC is returned.
 *
 * It is the users responsibility that added objects are unique when this method is used.
 * Adding a non-unique object to the Set will lead to undefined behaviour.
 *
 * It is allowed to add NULL to the set.
 */
long
Coll_Set_addUniqueObject(
     Coll_Set *_this,
     void *object);

/**
 * \brief Removes an object from the set
 *
 * \return the removed object
 */
/* NOT IN DESIGN - return value     */
void*
Coll_Set_remove(
    Coll_Set *_this,
    void *object);

/**
 * \brief Checks if the object is part of the set
 */
int
Coll_Set_contains(
     Coll_Set *_this,
     void *object);

/* NOT IN DESIGN from private to package */
Coll_Iter*
Coll_Set_find(
    Coll_Set* _this,
    void* object);

#if defined (__cplusplus)
}
#endif

#endif /* COLL_SET_H */
