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
#ifndef GAPI_STATUS_H
#define GAPI_STATUS_H

#include "gapi_common.h"
#include "gapi_entity.h"
#include "u_entity.h"

#define MAX_LISTENER_DEPTH 3

#define GAPI_STATUS_KIND_FULL 0x1fffU
#define GAPI_STATUS_KIND_NULL 0x0U

#define TOPIC_STATUS_MASK           (GAPI_INCONSISTENT_TOPIC_STATUS|\
                                     GAPI_ALL_DATA_DISPOSED_STATUS)
#define READER_STATUS_MASK          (GAPI_SAMPLE_REJECTED_STATUS            |\
                                     GAPI_LIVELINESS_CHANGED_STATUS         |\
                                     GAPI_REQUESTED_DEADLINE_MISSED_STATUS  |\
                                     GAPI_REQUESTED_INCOMPATIBLE_QOS_STATUS |\
                                     GAPI_DATA_AVAILABLE_STATUS             |\
                                     GAPI_SAMPLE_LOST_STATUS                |\
                                     GAPI_SUBSCRIPTION_MATCH_STATUS)
#define WRITER_STATUS_MASK          (GAPI_LIVELINESS_LOST_STATUS            |\
                                     GAPI_OFFERED_DEADLINE_MISSED_STATUS    |\
                                     GAPI_OFFERED_INCOMPATIBLE_QOS_STATUS   |\
                                     GAPI_PUBLICATION_MATCH_STATUS)
#define SUBSCRIBER_STATUS_MASK      (GAPI_DATA_ON_READERS_STATUS)
#define PUBLISHER_STATUS_MASK       (GAPI_STATUS_KIND_NULL)
#define PARTICIPANT_STATUS_MASK     (GAPI_STATUS_KIND_NULL)
 
    
#define TOPIC_STATUS_INTEREST       (TOPIC_STATUS_MASK)
#define READER_STATUS_INTEREST      (READER_STATUS_MASK)
#define WRITER_STATUS_INTEREST      (WRITER_STATUS_MASK)
#define SUBSCRIBER_STATUS_INTEREST  (READER_STATUS_INTEREST    | SUBSCRIBER_STATUS_MASK)
#define PUBLISHER_STATUS_INTEREST   (WRITER_STATUS_INTEREST    | PUBLISHER_STATUS_MASK)
#define PARTICIPANT_STATUS_INTEREST (TOPIC_STATUS_INTEREST     | SUBSCRIBER_STATUS_INTEREST |\
                                     PUBLISHER_STATUS_INTEREST | PARTICIPANT_STATUS_MASK)

#define _STATUS_ENTITY(s)            (_Status(s)->entity)
#define _STATUS_PARENT(s)            (_Status(s)->parent)
#define _STATUS_USER_ENTITY(s)       (_Status(s)->userEntity)

typedef struct _ListenerInfo_s {
    gapi_object             handle;
    gapi_statusMask       mask;
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
    gapi_boolean          dispatchOn;
    gapi_statusMask       enabled;
    gapi_statusMask       validMask;
    gapi_statusMask       interestMask;
    u_entity              userEntity;
    _ListenerCallbackInfo callbackInfo;
    ListenerAction        notify;
};

typedef struct ListenerEvent {
    gapi_statusMask event;
    gapi_object     source;
} ListenerEvent;


void
_StatusInit(
    _Status info,
    _Entity entity,
    _Status parent,
    long depth,
    _StatusKind kind,
    gapi_statusMask valid,
    gapi_statusMask interest,
    gapi_statusMask initmask,
    gapi_boolean active,
    ListenerAction notify);

void
_StatusDeinit(
    _Status info);

void
_StatusDefaultListener(
    _Entity entity,
    gapi_statusMask mask);

gapi_boolean
_StatusSetListener(
    _Status status,
    gapi_statusMask mask);

gapi_boolean
_StatusSetListenerInterest (
    _Status status,
    _ListenerInterestInfo info);

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
    c_long eventKindMask);

#endif
