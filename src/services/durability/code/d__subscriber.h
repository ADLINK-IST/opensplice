/*
 *                         Vortex OpenSplice
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR ADLINK
 *   Technology Limited, its affiliated companies and licensors. All rights
 *   reserved.
 *
 *   Licensed under the ADLINK Software License Agreement Rev 2.7 2nd October
 *   2014 (the "License"); you may not use this file except in compliance with
 *   the License.
 *   You may obtain a copy of the License at:
 *                      $OSPL_HOME/LICENSE
 *
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 */

#ifndef D__SUBSCRIBER_H
#define D__SUBSCRIBER_H

#include "d__types.h"
#include "u_user.h"

#if defined (__cplusplus)
extern "C" {
#endif

/**
 * Macro that checks the d_subscriber validity.
 * Because d_subscriber is a concrete class typechecking is required.
 */
#define             d_subscriberIsValid(_this)   \
    d_objectIsValid(d_object(_this), D_SUBSCRIBER)

/**
 * \brief The d_subscriber cast macro.
 *
 * This macro casts an object to a d_subscriber object.
 */
#define d_subscriber(_this) ((d_subscriber)(_this))

C_STRUCT(d_subscriber){
    C_EXTENDS(d_object);
    d_admin                     admin;
    u_subscriber                subscriber;          /* subscriber for durability protocol */
    u_subscriber                subscriber2;         /* subscriber for client-side durability protocol */
    u_subscriber                builtinSubscriber;   /* builtin subscriber */
    u_subscriber                persistentSubscriber;
    d_waitset                   waitset;

    d_statusListener                 statusListener;
    d_groupRemoteListener            groupRemoteListener;
    d_groupLocalListener             groupLocalListener;
    d_groupsRequestListener          groupsRequestListener;
    d_sampleRequestListener          sampleRequestListener;
    d_sampleChainListener            sampleChainListener;
    d_nameSpacesRequestListener      nameSpacesRequestListener;
    d_nameSpacesListener             nameSpacesListener;
    d_persistentDataListener         persistentDataListener;
    d_deleteDataListener             deleteDataListener;
    d_remoteReaderListener           remoteReaderListener;
    d_dcpsHeartbeatListener          dcpsHeartbeatListener;
    d_historicalDataRequestListener  historicalDataRequestListener;
    d_durabilityStateRequestListener durabilityStateRequestListener;
    d_dcpsPublicationListener        dcpsPublicationListener;
    d_capabilityListener             capabilityListener;

    d_store                     persistentStore;
};


d_subscriber            d_subscriberNew                                 (d_admin admin);

void                    d_subscriberDeinit                              (d_subscriber subscriber);

void                    d_subscriberFree                                (d_subscriber subscriber);

d_admin                 d_subscriberGetAdmin                            (d_subscriber subscriber);

u_subscriber            d_subscriberGetSubscriber                       (d_subscriber subscriber);

d_waitset               d_subscriberGetWaitset                          (d_subscriber subscriber);

u_subscriber            d_subscriberGetPersistentSubscriber             (d_subscriber subscriber);

c_bool                  d_subscriberStartListeners                      (d_subscriber subscriber);

void                    d_subscriberStopListeners                       (d_subscriber subscriber);

d_store                 d_subscriberGetPersistentStore                  (d_subscriber subscriber);

c_bool                  d_subscriberAreRemoteGroupsHandled              (d_subscriber subscriber);

d_groupLocalListener        d_subscriberGetGroupLocalListener           (d_subscriber subscriber);

d_sampleChainListener       d_subscriberGetSampleChainListener          (d_subscriber subscriber);

d_nameSpacesRequestListener d_subscriberGetNameSpacesRequestListener    (d_subscriber subscriber);

d_historicalDataRequestListener d_subscriberGetHistoricalDataRequestListener(d_subscriber subscriber);

#if defined (__cplusplus)
}
#endif

#endif /* D__SUBSCRIBER_H */
