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
#ifndef DLRL_KERNEL_CACHE_BRIDGE_H
#define DLRL_KERNEL_CACHE_BRIDGE_H

/* collection includes */
#include "Coll_Set.h"
#include "Coll_List.h"

/* DLRL Kernel includes */
#include "DLRL_Kernel.h"

#if defined (__cplusplus)
extern "C" {
#endif

/* \brief Creates a DCPS publisher.
 *
 * Mutex claims during this operation:<ul>
 * <li>The mutex of the <code>DK_CacheFactoryAdmin</code> object.</li></ul>
 *
 * \param exception If an exception occurs the values in this struct will be modified.
 * \param userData The userData as provided by the language specific binding when entering the kernel
 * \param participant The DCPS domain participant belonging to the <code>DK_CacheAdmin</code> object for which the
 * publisher needs to be created.
 * \param ls_participant The language specific DCPS domain participant belonging to the <code>DK_CacheAdmin</code>
 * object for which the publisher needs to be created.
 * \param ls_publisher An out parameter which should be set to the language specific publisher so the kernel can store
 * it.
 *
 * \return <code>NULL</code> if and only if an exception occured or returns the created publisher
 */
/* NOT IN DESIGN - param added */
typedef C_STRUCT(u_publisher)* (*DK_CacheBridge_us_createPublisher)(
    DLRL_Exception* exception,
    void* userData,
    u_participant participant,
    DLRL_LS_object ls_participant,
    DLRL_LS_object *ls_publisher);

/* \brief Creates a DCPS subscriber.
 *
 * Mutex claims during this operation:<ul>
 * <li>The mutex of the <code>DK_CacheFactoryAdmin</code> object.</li></ul>
 *
 * \param exception If an exception occurs the values in this struct will be modified.
 * \param userData The userData as provided by the language specific binding when entering the kernel
 * \param participant The DCPS domain participant belonging to the <code>DK_CacheAdmin</code> object for which the
 * subscriber needs to be created.
 * \param ls_subscriber An out parameter which should be set to the language specific subscriber so the kernel can store
 * it.
 *
 * \return <code>NULL</code> if and only if an exception occured or returns the created subscriber
 */
/* NOT IN DESIGN - param added */
typedef C_STRUCT(u_subscriber)* (*DK_CacheBridge_us_createSubscriber)(
    DLRL_Exception* exception,
    void* userData,
    u_participant participant,
    DLRL_LS_object ls_participant,
    DLRL_LS_object *ls_subscriber);

/* \brief Deletes a DCPS publisher.
 *
 * Mutex claims during this operation:<ul>
 * <li>The mutex of the <code>DK_CacheFactoryAdmin</code> object.</li>
 * <li>The update and admin mutex of the <code>DK_CacheAdmin</code> object</li></ul>
 *
 * \param exception If an exception occurs the values in this struct will be modified.
 * \param userData The userData as provided by the language specific binding when entering the kernel
 * \param participant The DCPS domain participant belonging to the <code>DK_CacheAdmin</code> object for which the
 * publisher needs to be deleted.
 * \param ls_subscriber The language specific publisher
 */
/* NOT IN DESIGN - param added */
typedef void (*DK_CacheBridge_us_deletePublisher)(
    DLRL_Exception* exception,
    void* userData,
    u_participant participant,
    DLRL_LS_object ls_participant,
    DLRL_LS_object ls_publisher);

/* \brief Deletes a DCPS subscriber.
 *
 * Mutex claims during this operation:<ul>
 * <li>The mutex of the <code>DK_CacheFactoryAdmin</code> object.</li>
 * <li>The update and admin mutex of the <code>DK_CacheAdmin</code> object</li></ul>
 *
 * \param exception If an exception occurs the values in this struct will be modified.
 * \param userData The userData as provided by the language specific binding when entering the kernel
 * \param participant The DCPS domain participant belonging to the <code>DK_CacheAdmin</code> object for which the
 * subscriber needs to be deleted.
 * \param ls_subscriber The language specific subscriber
 */
/* NOT IN DESIGN - param added */
typedef void (*DK_CacheBridge_us_deleteSubscriber)(
    DLRL_Exception* exception,
    void* userData,
    u_participant participant,
    DLRL_LS_object ls_participant,
    DLRL_LS_object ls_subscriber);

/* \brief Triggers all registered Cache Listeners for start of updates
 *
 * Mutex claims during this operation:<ul>
 * <li>The update mutex of the <code>DK_CacheAdmin</code> object</li></ul>
 *
 * \param exception If an exception occurs the values in this struct will be modified.
 * \param relatedCache The cache for which the listeners need to be triggered
 * \param userData The userData as provided by the language specific binding when entering the kernel
 */
typedef void (*DK_CacheBridge_us_triggerListenersWithStartOfUpdates)(
    DLRL_Exception* exception,
    DK_CacheAdmin* relatedCache,
    void* userData);

/* \brief Triggers all registered Cache Listeners for end of updates
 *
 * Mutex claims during this operation:<ul>
 * <li>The update mutex of the <code>DK_CacheAdmin</code> object</li></ul>
 *
 * \param exception If an exception occurs the values in this struct will be modified.
 * \param relatedCache The cache for which the listeners need to be triggered
 * \param userData The userData as provided by the language specific binding when entering the kernel
 */
typedef void (*DK_CacheBridge_us_triggerListenersWithEndOfUpdates)(
    DLRL_Exception* exception,
    DK_CacheAdmin* relatedCache,
    void* userData);

/* \brief Triggers all registered Cache Listeners for updates enabled
 *
 * Mutex claims during this operation:<ul>
 * <li>The update mutex of the <code>DK_CacheAdmin</code> object</li></ul>
 *
 * \param exception If an exception occurs the values in this struct will be modified.
 * \param relatedCache The cache for which the listeners need to be triggered
 * \param userData The userData as provided by the language specific binding when entering the kernel
 */
typedef void (*DK_CacheBridge_us_triggerListenersWithUpdatesEnabled)(
    DLRL_Exception* exception,
    DK_CacheAdmin* relatedCache,
    void* userData);

/* \brief Triggers all registered Cache Listeners for updates disabled
 *
 * Mutex claims during this operation:<ul>
 * <li>The update mutex of the <code>DK_CacheAdmin</code> object</li></ul>
 *
 * \param exception If an exception occurs the values in this struct will be modified.
 * \param relatedCache The cache for which the listeners need to be triggered
 * \param userData The userData as provided by the language specific binding when entering the kernel
 */
typedef void (*DK_CacheBridge_us_triggerListenersWithUpdatesDisabled)(
    DLRL_Exception* exception,
    DK_CacheAdmin* relatedCache,
    void* userData);

typedef void (*DK_CacheBridge_us_homesAction)(
    DLRL_Exception* exception,
    void* userData,
    const Coll_List* homes,
    void** arg);

typedef void (*DK_CacheBridge_us_listenersAction)(
    DLRL_Exception* exception,
    void* userData,
    const Coll_Set* listeners,
    void** arg);

typedef void (*DK_CacheBridge_us_accessesAction)(
    DLRL_Exception* exception,
    void* userData,
    const Coll_Set* accesses,
    void** arg);

typedef void (*DK_CacheBridge_us_objectsAction)(
    DLRL_Exception* exception,
    void* userData,
    void** arg,
    LOC_unsigned_long totalSize,
    LOC_unsigned_long* elementIndex,
    DK_ObjectArrayHolder* holder);

typedef LOC_boolean (*DK_CacheBridge_us_isDataAvailable)(
    DLRL_Exception* exception,
    void* userData,
    DLRL_LS_object ls_subscriber);

typedef struct DK_CacheBridge_s{
    DK_CacheBridge_us_createPublisher createPublisher;
    DK_CacheBridge_us_createSubscriber createSubscriber;
    DK_CacheBridge_us_deletePublisher deletePublisher;
    DK_CacheBridge_us_deleteSubscriber deleteSubscriber;
    DK_CacheBridge_us_triggerListenersWithStartOfUpdates triggerListenersWithStartOfUpdates;
    DK_CacheBridge_us_triggerListenersWithEndOfUpdates triggerListenersWithEndOfUpdates;
    DK_CacheBridge_us_triggerListenersWithUpdatesEnabled triggerListenersWithUpdatesEnabled;
    DK_CacheBridge_us_triggerListenersWithUpdatesDisabled triggerListenersWithUpdatesDisabled;
    DK_CacheBridge_us_homesAction homesAction;
    DK_CacheBridge_us_listenersAction listenersAction;
    DK_CacheBridge_us_accessesAction accessesAction;
    DK_CacheBridge_us_objectsAction objectsAction;
    DK_CacheBridge_us_isDataAvailable isDataAvailable;
} DK_CacheBridge;

extern DK_CacheBridge cacheBridge;

#if defined (__cplusplus)
}
#endif

#endif /* DLRL_KERNEL_CACHE_BRIDGE_H */
