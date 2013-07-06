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
#ifndef DJA_CACHE_BRIDGE_H
#define DJA_CACHE_BRIDGE_H

/* DLRL includes */
#include "DLRL_Types.h"

u_publisher DJA_CacheBridge_us_createPublisher(DLRL_Exception* exception, void* userData, u_participant participant,
                                                DLRL_LS_object ls_participant, DLRL_LS_object* ls_publisher);

u_subscriber DJA_CacheBridge_us_createSubscriber(DLRL_Exception* exception, void* userData, u_participant participant,
                                                    DLRL_LS_object ls_participant, DLRL_LS_object* ls_subscriber);

void DJA_CacheBridge_us_deletePublisher(DLRL_Exception* exception, void* userData, u_participant participant,
                                                      DLRL_LS_object ls_participant, DLRL_LS_object ls_publisher);

void DJA_CacheBridge_us_deleteSubscriber(DLRL_Exception* exception,  void* userData, u_participant participant,
                                                    DLRL_LS_object ls_participant, DLRL_LS_object ls_subscriber);

void DJA_CacheBridge_us_triggerListenersWithStartOfUpdates(DLRL_Exception* exception, DK_CacheAdmin* relatedCache,
                                                        void* userData);

void DJA_CacheBridge_us_triggerListenersWithEndOfUpdates(DLRL_Exception* exception, DK_CacheAdmin* relatedCache,
                                                        void* userData);

void DJA_CacheBridge_us_triggerListenersWithUpdatesEnabled(DLRL_Exception* exception, DK_CacheAdmin* relatedCache,
                                                            void* userData);

void DJA_CacheBridge_us_triggerListenersWithUpdatesDisabled(DLRL_Exception* exception, DK_CacheAdmin* relatedCache,
                                                        void* userData);

void DJA_CacheBridge_us_homesAction(DLRL_Exception* exception, void* userData, const Coll_List* homes, void** arg);

void DJA_CacheBridge_us_listenersAction(DLRL_Exception* exception, void* userData, const Coll_Set* homes, void** arg);

void DJA_CacheBridge_us_accessesAction(DLRL_Exception* exception, void* userData, const Coll_Set* homes, void** arg);

LOC_boolean DJA_CacheBridge_us_isDataAvailable(
    DLRL_Exception* exception,
    void* userData,
    DLRL_LS_object ls_subscriber);

void DJA_CacheBridge_us_objectsAction(DLRL_Exception* exception, void* userData, void** arg,
                            LOC_unsigned_long totalSize, LOC_unsigned_long* elementIndex, DK_ObjectArrayHolder* holder);

#endif /* DJA_CACHE_BRIDGE_H */
