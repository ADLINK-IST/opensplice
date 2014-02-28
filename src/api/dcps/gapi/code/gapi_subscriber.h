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
#ifndef GAPI_SUBSCRIBER_H
#define GAPI_SUBSCRIBER_H

#include "gapi_common.h"
#include "gapi_entity.h"

#include "u_user.h"
#include "os_if.h"

#if defined (__cplusplus)
extern "C" {
#endif

#ifdef OSPL_BUILD_CORE
#define OS_API OS_API_EXPORT
#else
#define OS_API OS_API_IMPORT
#endif
/* !!!!!!!!NOTE From here no more includes are allowed!!!!!!! */

#define U_SUBSCRIBER_GET(s) \
        u_subscriber(U_ENTITY_GET(s))

#define U_SUBSCRIBER_SET(s,e) \
        _EntitySetUserEntity(_Entity(s), u_entity(e))

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
    C_EXTENDS(_Entity);
    gapi_dataReaderQos             _defDataReaderQos;
    struct gapi_subscriberListener _Listener;
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

/* The following method is intended to be local for the gapi component but
 * is clandestine used by the DLRL, for this we need OS_API to support windows.
 * This needs to be fixed in the future.
 */
OS_API u_subscriber
_SubscriberUsubscriber (
    _Subscriber _this);

c_long
_SubscriberReaderCount (
    _Subscriber _this);

gapi_boolean
_SubscriberContainsEntity (
    _Subscriber _this,
    gapi_instanceHandle_t handle);

gapi_returnCode_t
_SubscriberDeleteContainedEntities (
    _Subscriber _this);

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
