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
 * This file contains the global definitions and macros for AAPI.
 */


#ifndef A_DEF_H
#define A_DEF_H


/* Private files included here for cloning c_base:
*/
#include "ut_avl.h"
#include "c_metabase.h"
#include "c_sync.h"


#ifndef NULL
#define NULL 0
#endif


#ifdef __linux
#define A_DEF_HEADEREXPAND 0
#else
#define A_DEF_HEADEREXPAND 1
#endif



/**
 * \brief Internally used type for holding a counter's value.
 */
typedef c_long a_counter;


/**
 * \brief Initial Hashtable's ArraySize for holding
 *        all pointer references (occurrences).
 */
#define A_OCCURRENCES_ARRAY_SIZE                ((1<<15) - 1)


/**
 * \brief Formula for defining a new ArraySize for
 *        the occurrences Hashtable, once the
 *        MaxUsed value has been determined.
 */
#define A_OCCARRSIZEFROMMAXUSED(mmStateMaxUsed) (mmStateMaxUsed / 10)


/**
 * \brief Default Pointer Size
 */
#define A_PTRSIZE                               (sizeof(c_voidp))


/**
 * \brief Default Object's Header Size, typically
 *        6 * A_PTRSIZE in the debug version.
 */
#define A_HEADERSIZE                            ((5 + A_DEF_HEADEREXPAND) * A_PTRSIZE)


/**
 * \brief Calculates the start address of an
 *        header's \a Type address, given
 *        the object's start address.
 *
 * \note This operation is only accurate in
 *       SPLICE's development version!
 */
#define A_TYPEADDR(objAddr)                     (objAddr - (1 + A_DEF_HEADEREXPAND) * A_PTRSIZE)


/**
 * \brief Calculates the start address of an
 *        header's \a nextObject address, given
 *        the object's start address.
 *
 * \note This operation is only valid in
 *       SPLICE's debug mode!
 */
#define A_NEXTADDR(objAddr)                     (objAddr - (4 + A_DEF_HEADEREXPAND) * A_PTRSIZE)


/**
 * \brief Calculates the start address of an
 *        header's \a prevObject address, given
 *        the object's start address.
 *
 * \note This operation is only valid in
 *       SPLICE's debug mode!
 */
#define A_PREVADDR(objAddr)                     (objAddr - (3 + A_DEF_HEADEREXPAND) * A_PTRSIZE)


/**
 * \brief Calculates the address holding the
 *        confidence check, given the object's
 *        start address.
 *
 * \note This operation is only valid in
 *       SPLICE's debug mode!
 */
#define A_CONFADDR(objAddr)                     (objAddr - (5 + A_DEF_HEADEREXPAND) * A_PTRSIZE)


/**
 * \brief Calculates the address holding the
 *        Reference Count, given the object's
 *        start address.
 */
#define A_REFCADDR(objAddr)                     (objAddr - (2 + A_DEF_HEADEREXPAND) * A_PTRSIZE)


/**
 * \brief Calculates the address holding the
 *        4 padding bytes, given the object's
 *        start address.
 */
#define A_PADDADDR(objAddr)                     (objAddr - A_DEF_HEADEREXPAND * A_PTRSIZE)


/**
 * \brief Value to wipe (overwrite) all known objects with
 */
#define A_WIPEVALUE                             (0x11111111)
//#define A_WIPEVALUE                             (0x0BADCAFE)


/**
 * \brief Confidence Value
 */
#define A_CONFIDENCE                            (0x504F5448)


/**
 * \brief Calculates a header's start address,
 *        specified by its object address.
 */
#define A_HEADERADDR(objAddr)                   (objAddr - A_HEADERSIZE)


/**
 * \brief Calculates an object's start address,
 *        specified by its header address.
 */
#define A_OBJECTADDR(hdrAddr)                   (hdrAddr + A_HEADERSIZE)


/**
 * \brief
 * Recreation of c_base
 *
 * Because c_base is a private type, we need to create
 * our own, which we call a_base. ;-) Now we can cast a
 * c_base to our a_base and use its members.
 *
 * \todo
 * Tap into the private definition of c_base (somehow)
 */
C_CLASS(a_base);

/**
 * \brief
 * Clone of c_base
 */
C_STRUCT(a_base) {
        C_EXTENDS(c_module);
        c_mm      mm;
        c_long    confidence;
        ut_avlTree_t bindings;
        c_ulong   skipUnknownMutexbindLock[9];    // cheap hack to skip
        c_ulong   skipUnknownMutexSchemaLock[8];  //      some c_ulongs
//      c_mutex   bindLock;
//      c_mutex   schemaLock;
        c_type    metaType[M_COUNT];
        c_type    string_type;
        c_object  firstObject;
        c_object  lastObject;
};


#endif  /* A_DEF_H */


//END  a_def.h
