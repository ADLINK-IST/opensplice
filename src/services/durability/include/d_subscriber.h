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

#ifndef D_SUBSCRIBER_H
#define D_SUBSCRIBER_H

#include "d__types.h"
#include "u_user.h"

#if defined (__cplusplus)
extern "C" {
#endif

#define d_subscriber(w)         ((d_subscriber)(w))

d_subscriber    d_subscriberNew                                 (d_admin admin);

void            d_subscriberFree                                (d_subscriber subscriber);

d_admin         d_subscriberGetAdmin                            (d_subscriber subscriber);

u_subscriber    d_subscriberGetSubscriber                       (d_subscriber subscriber);

d_waitset       d_subscriberGetWaitset                          (d_subscriber subscriber);

u_subscriber    d_subscriberGetPersistentSubscriber             (d_subscriber subscriber);

void            d_subscriberInitStatusListener                  (d_subscriber subscriber);

c_bool          d_subscriberSetStatusListenerEnabled            (d_subscriber subscriber,
                                                                 c_bool enable);

void            d_subscriberInitGroupRemoteListener             (d_subscriber subscriber);

c_bool          d_subscriberSetGroupRemoteListenerEnabled       (d_subscriber subscriber,
                                                                 c_bool enable);

void            d_subscriberInitGroupLocalListener              (d_subscriber subscriber);

c_bool          d_subscriberSetGroupLocalListenerEnabled        (d_subscriber subscriber,
                                                                 c_bool enable);

void            d_subscriberInitGroupsRequestListener           (d_subscriber subscriber);

c_bool          d_subscriberSetGroupsRequestListenerEnabled     (d_subscriber subscriber,
                                                                 c_bool enable);

void            d_subscriberInitSampleRequestListener           (d_subscriber subscriber);

c_bool          d_subscriberSetSampleRequestListenerEnabled     (d_subscriber subscriber,
                                                                 c_bool enable);

void            d_subscriberInitSampleChainListener             (d_subscriber subscriber);

c_bool          d_subscriberSetSampleChainListenerEnabled       (d_subscriber subscriber,
                                                                 c_bool enable);

void            d_subscriberInitNameSpacesRequestListener       (d_subscriber subscriber);

c_bool          d_subscriberSetNameSpacesRequestListenerEnabled (d_subscriber subscriber,
                                                                 c_bool enable);

void            d_subscriberInitNameSpacesListener              (d_subscriber subscriber);

c_bool          d_subscriberSetNameSpacesListenerEnabled        (d_subscriber subscriber,
                                                                 c_bool enable);

void            d_subscriberInitPersistentDataListener          (d_subscriber subscriber);

c_bool          d_subscriberSetPersistentDataListenerEnabled    (d_subscriber subscriber,
                                                                 c_bool enable);

void            d_subscriberInitDeleteDataListener              (d_subscriber subscriber);

c_bool          d_subscriberSetDeleteDataListenerEnabled        (d_subscriber subscriber,
                                                                 c_bool enable);

d_store         d_subscriberGetPersistentStore                  (d_subscriber subscriber);

c_bool          d_subscriberAreRemoteGroupsHandled              (d_subscriber subscriber);

d_groupLocalListener        d_subscriberGetGroupLocalListener           (d_subscriber subscriber);

d_sampleChainListener       d_subscriberGetSampleChainListener          (d_subscriber subscriber);

d_nameSpacesRequestListener d_subscriberGetNameSpacesRequestListener    (d_subscriber subscriber);

#if defined (__cplusplus)
}
#endif

#endif /* D_SUBSCRIBER_H */
