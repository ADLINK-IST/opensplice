/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2009 PrismTech 
 *   Limited and its licensees. All rights reserved. See file:
 *
 *                     $OSPL_HOME/LICENSE 
 *
 *   for full copyright notice and license terms. 
 *
 */
#ifndef GAPI_SUBSCRIBER_H
#define GAPI_SUBSCRIBER_H

#include "gapi_common.h"
#include "gapi_domainEntity.h"

#include "u_user.h"
#include "os_if.h"

#if defined (__cplusplus)
extern "C" {
#endif

#ifdef OSPL_BUILD_DCPSGAPI
#define OS_API OS_API_EXPORT
#else
#define OS_API OS_API_IMPORT
#endif
/* !!!!!!!!NOTE From here no more includes are allowed!!!!!!! */

#define U_SUBSCRIBER_GET(s)       u_subscriber(U_ENTITY_GET(s))
#define U_SUBSCRIBER_SET(s,e)     _EntitySetUserEntity(_Entity(s), u_entity(e))

#define _Subscriber(o) ((_Subscriber)(o))

#define gapi_subscriberClaim(h,r) \
        (_Subscriber(gapi_objectClaim(h,OBJECT_KIND_SUBSCRIBER,r)))

#define gapi_subscriberClaimNB(h,r) \
        (_Subscriber(gapi_objectClaimNB(h,OBJECT_KIND_SUBSCRIBER,r)))

#define _SubscriberAlloc() \
        (_Subscriber(_ObjectAlloc(OBJECT_KIND_SUBSCRIBER, \
                                  C_SIZEOF(_Subscriber), \
                                  NULL)))

C_STRUCT(_Subscriber) {
    C_EXTENDS(_DomainEntity);
    gapi_dataReaderQos             _defDataReaderQos;
    struct gapi_subscriberListener _Listener;
    gapi_set                       dataReaderSet;
    gapi_boolean                   builtin;
};

_Subscriber
_SubscriberNew (
    u_participant uParticipant,
    const gapi_subscriberQos *qos,
    const struct gapi_subscriberListener *listener,
    const gapi_statusMask mask,
    const _DomainParticipant participant);

gapi_returnCode_t
_SubscriberFree (
    _Subscriber _this);

gapi_boolean
_SubscriberPrepareDelete (
    _Subscriber _this);

OS_API u_subscriber
_SubscriberUsubscriber (
    _Subscriber _this);

_DomainParticipant
_SubscriberParticipant (
    _Subscriber _this);

_DataReader
_SubscriberLookupDatareader (
    _Subscriber _this,
    const gapi_char *topicName);

gapi_boolean
_SubscriberSetListenerInterestOnChildren (
    _Subscriber _this,
    _ListenerInterestInfo info);

void
_SubscriberSetDeleteAction (
    _Subscriber _this,
    gapi_deleteEntityAction action,
    void *argument);

gapi_boolean
_SubscriberContainsEntity (
    _Subscriber _this,
    gapi_instanceHandle_t handle);

void
_SubscriberNotifyListener(
    _Subscriber _this,
    gapi_statusMask triggerMask);

void
_SubscriberOnDataOnReaders (
    _Subscriber _this);

#undef OS_API

#if defined (__cplusplus)
}
#endif

#endif /* GAPI_SUBSCRIBER_H */
