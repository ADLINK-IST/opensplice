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
#ifndef COLL_ITER_H
#define COLL_ITER_H

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
typedef struct Coll_Iter_s Coll_Iter;

/**
 * \brief Constructor for a Coll_Iter
 *
 * This method allocates a Coll_Iter_s structure initialises it and return a pointer to it.
 */
OS_API Coll_Iter *
Coll_Iter_new(
    void);

/**
 * \brief Destructor for a Coll_Iter
 */
OS_API long
Coll_Iter_delete(
    Coll_Iter *_this);

/**
 * \brief Getter for next element
 */
OS_API Coll_Iter *
Coll_Iter_getNext(
    Coll_Iter *_this);

/**
 * \brief Setter for next element
 */
OS_API void
Coll_Iter_setNext(
    Coll_Iter *_this,
    Coll_Iter *next);

/**
 * \brief Getter for prev element
 */
OS_API Coll_Iter *
Coll_Iter_getPrev(
    Coll_Iter *_this);

/**
 * \brief Setter for prev element
 */
OS_API void
Coll_Iter_setPrev(
    Coll_Iter *_this,
    Coll_Iter *prev);

/**
 * \brief Getter for object
 */
OS_API void *
Coll_Iter_getObject(
    Coll_Iter *_this);

/**
 * \brief Setter for object
 */
OS_API void
Coll_Iter_setObject(
    Coll_Iter *_this,
    void *object);

#undef OS_API

#if defined (__cplusplus)
}
#endif

#endif /* COLL_ITER_H */
