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
#ifndef GAPI_STATUS_H
#define GAPI_STATUS_H

#include "gapi_common.h"
#include "gapi_entity.h"
#include "u_entity.h"

#define MAX_LISTENER_DEPTH 3

#define GAPI_STATUS_KIND_FULL 0x1fffU
#define GAPI_STATUS_KIND_NULL 0x0U

typedef struct _ListenerInfo_s {
    gapi_object handle;
    gapi_statusMask mask;
} _ListenerInfo;

#define _Status(o) ((_Status)(o))

typedef struct _ListenerCallbackInfo_s {
    void *listenerData;
    gapi_listener_InconsistentTopicListener on_inconsistent_topic;
    gapi_listener_AllDataDisposedListener on_all_data_disposed;
    gapi_listener_OfferedDeadlineMissedListener on_offered_deadline_missed;
    gapi_listener_OfferedIncompatibleQosListener on_offered_incompatible_qos;
    gapi_listener_LivelinessLostListener on_liveliness_lost;
    gapi_listener_PublicationMatchedListener on_publication_match;
    gapi_listener_RequestedDeadlineMissedListener on_requested_deadline_missed;
    gapi_listener_RequestedIncompatibleQosListener on_requested_incompatible_qos;
    gapi_listener_SampleRejectedListener on_sample_rejected;
    gapi_listener_LivelinessChangedListener on_liveliness_changed;
    gapi_listener_DataAvailableListener on_data_available;
    gapi_listener_SubscriptionMatchedListener on_subscription_match;
    gapi_listener_SampleLostListener on_sample_lost;
    gapi_listener_DataOnReadersListener on_data_on_readers;
} _ListenerCallbackInfo;
    
typedef void (*ListenerAction)(_Entity, gapi_statusMask);

typedef enum {
    STATUS_KIND_UNKNOWN,
    STATUS_KIND_PARTICIPANT,
    STATUS_KIND_TOPIC,
    STATUS_KIND_PUBLISHER,
    STATUS_KIND_SUBSCRIBER,
    STATUS_KIND_DATAWRITER,
    STATUS_KIND_DATAREADER
} _StatusKind;    

C_STRUCT(_Status) {
    C_EXTENDS(_Object);
    _StatusKind           kind;
    _Status               parent;
    _Entity               entity;
    _ListenerInfo         listenerInfo[MAX_LISTENER_DEPTH];
    long                  depth;
    gapi_statusMask       enabled;
    gapi_statusMask       validMask;
    gapi_statusMask       interestMask;
    u_entity              userEntity;
    _ListenerCallbackInfo callbackInfo;
    ListenerAction        notify;
};

_Status
_StatusNew(
    _Entity entity,
    _StatusKind kind,
    const struct gapi_listener *info,
    gapi_statusMask mask);

void
_StatusInit(
    _Status _this,
    _Entity entity,
    _StatusKind kind,
    const struct gapi_listener *info,
    gapi_statusMask mask);

void
_StatusDeinit(
    _Status info);

gapi_boolean
_StatusSetListener(
    _Status status,
    const struct gapi_listener *info,
    gapi_statusMask mask);

gapi_object
_StatusFindTarget(
    _Status status,
    gapi_statusMask mask);

gapi_statusMask
_StatusGetCurrentStatus(
    _Status status);

gapi_statusMask
_StatusGetMaskStatus (
    _Status status,
    c_long  eventKindMask);

void
_StatusNotifyEvent (
    _Status status,
    c_ulong events);

void
_StatusNotifyDataAvailable (
    _Status status,
    gapi_object source);

c_bool
_StatusNotifyDataOnReaders (
    _Status status,
    gapi_object source);

void
_StatusNotifySubscriptionMatch (
    _Status status,
    gapi_object source,
    gapi_subscriptionMatchedStatus *info);

void
_StatusNotifyRequestedIncompatibleQos (
    _Status status,
    gapi_object source,
    gapi_requestedIncompatibleQosStatus *info);

void
_StatusNotifyRequestedDeadlineMissed (
    _Status status,
    gapi_object source,
    gapi_requestedDeadlineMissedStatus *info);

void
_StatusNotifySampleRejected (
    _Status status,
    gapi_object source,
    gapi_sampleRejectedStatus *info);

void
_StatusNotifyLivelinessChanged (
    _Status status,
    gapi_object source,
    gapi_livelinessChangedStatus *info);

void
_StatusNotifySampleLost (
    _Status status,
    gapi_object source,
    gapi_sampleLostStatus *info);

void
_StatusNotifyPublicationMatch (
    _Status status,
    gapi_object source,
    gapi_publicationMatchedStatus *info);

void
_StatusNotifyLivelinessLost (
    _Status status,
    gapi_object source,
    gapi_livelinessLostStatus *info);

void
_StatusNotifyOfferedIncompatibleQos (
    _Status status,
    gapi_object source,
    gapi_offeredIncompatibleQosStatus *info);

void
_StatusNotifyOfferedDeadlineMissed (
    _Status status,
    gapi_object source,
    gapi_offeredDeadlineMissedStatus *info);

void
_StatusNotifyAllDataDisposed (
    _Status status,
    gapi_object source);

void
_StatusNotifyInconsistentTopic (
    _Status status,
    gapi_object source,
    gapi_inconsistentTopicStatus *info);

#endif
