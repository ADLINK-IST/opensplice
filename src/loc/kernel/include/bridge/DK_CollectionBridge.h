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
#ifndef DLRL_KERNEL_COLLECTION_BRIDGE_H
#define DLRL_KERNEL_COLLECTION_BRIDGE_H

/* DLRL Kernel includes */
#include "DLRL_Kernel.h"

#if defined (__cplusplus)
extern "C" {
#endif

/* \brief Creates the language specific collection for a kernel collection.
 *
 * If the language binding requires it, any reference increasing should be done before returning this operation, as the
 * kernel will maintain the returned pointer.
 *
 * Mutex claims during this operation:<ul>
 * <li>The update mutex of the <code>DK_CacheAdmin</code> object to which the objects in question belong.</li>
 * <li>The update and admin mutexes of ALL <code>DK_ObjectHomeAdmin</code> objects which are registered to the
 * <code>DK_CacheAdmin</code> to which the objects in question belong.</li></ul>
 *
 * \param exception If an exception occurs the values in this struct will be modified.
 * \param userData The userData as provided by the language specific binding when entering the kernel
 * \param collection The kernel collection for which a language specific collection must be created
 * \param relationType The type of the collection (set/map/etc)
 *
 * \return The language specific collection or <code>NULL</code>.
 */
typedef DLRL_LS_object (*DK_CollectionBridge_us_createLSCollection)(
    DLRL_Exception* exception,
    void* userData,
    DK_Collection* collection,
    DK_RelationType relationType);

typedef struct DK_CollectionBridge_s{
    DK_CollectionBridge_us_createLSCollection createLSCollection;
} DK_CollectionBridge;

extern DK_CollectionBridge collectionBridge;

#if defined (__cplusplus)
}
#endif

#endif /* DLRL_KERNEL_COLLECTION_BRIDGE_H */
