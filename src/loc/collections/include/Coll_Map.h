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
#ifndef COLL_MAP_H
#define COLL_MAP_H

#include "Coll_Defs.h"

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
typedef struct Coll_Map_s Coll_Map;
typedef struct Coll_MapNode_s Coll_MapNode;

struct Coll_Map_s {
    long         _nr_elements;
    int          (*_isLess)(void *left, void *right);
    Coll_MapNode *_tree;
};

/**
 * \brief Constructor for a Coll_Map
 *
 * This method allocates a Coll_Map_s structure initialises it and return a pointer to it.
 * No elements are added to the map.
 */
OS_API Coll_Map *
Coll_Map_new(
     int (* const isLess)(void *, void*));

/**
 * \brief Initializer for a preallocated Coll_Map
 *
 * This method initializes a preallocated Coll_Map_s structure .
 * No elements are added to the map.
 * The method cannot be used on a existing Coll_Map which contains elements. This will result in memory leaks.
 */
OS_API void
Coll_Map_init(
     Coll_Map *_this,
     int (* const isLess)(void *, void*));

/**
 * \brief Coll_Map destructor
 *
 * This destructor frees the memory used by the map header
 * It can only be used when no elements exits in the map. If the map is not empty a COLL_ERROR_NOT_EMPTY is returned.
 */
OS_API long
Coll_Map_delete(
     Coll_Map *_this);

/**
 * \brief Getter for the number of elements
 */
OS_API long
Coll_Map_getNrOfElements(
    Coll_Map *_this);

/**
 * \brief Inserts an object to the map.
 *
 * Insert an object identified with key in the map. If a holder for a previously stored object is provided
 * the previously stored object will be replaced with the new object. The key itself will not be replaced.
 * The previously stored object will be returned via the holder.
 *
 * \param _this       pointer to the map to which the object should be added
 * \param key         pointer to the key by which the object is uniquely identified
 * \param new_object  pointer to the object to be stored in the map; is allowed to be NULL
 * \param old_object  pointer to a holder for returning the previously stored object for the key; is allowed to be be NULL
 *
 * \return COLL_OK                    object succesfully added to map
 *         COLL_ERROR_ALLOC           memory allocation error
 *         COLL_ERROR_ALREADY_EXISTS  An object with this key already exists (old_object = NULL)
 */
OS_API long
Coll_Map_add(
    Coll_Map *_this,
    void *key,
    void *new_object,
    void **old_object);

/**
 * \brief Removes an object from the map by key
 *
 * A pointer to the removed object is returned. The stored key value returns the original pointer which is used as key.
 * The user may use this out parameter to free the memory used by this key after the element has been removed from the
 * map. If the user already has a copy of the key this out parameter is unnecessary and a NULL value may be passed
 * instead.
 */
OS_API void *
Coll_Map_remove(
    Coll_Map *_this,
    void *key,
    void **stored_key);

/**
 * \brief Gets an object for a certain key. If the key is not present NULL is returned.
 */
OS_API void *
Coll_Map_get(
    Coll_Map *_this,
    void *key);

#undef OS_API

#if defined (__cplusplus)
}
#endif

#endif /* COLL_LIST_H */
