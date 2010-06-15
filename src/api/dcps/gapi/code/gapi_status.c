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

#include "gapi_entity.h"
#include "gapi_domainEntity.h"
#include "gapi_domainParticipantStatus.h"
#include "gapi_publisherStatus.h"
#include "gapi_subscriberStatus.h"
#include "gapi_kernel.h"


#include "os_mutex.h"
#include "os_heap.h"

#include "u_entity.h"
#include "u_dispatcher.h"
#include "v_state.h"
#include "v_event.h"

C_STRUCT(_ListenerInterestInfo) {
    gapi_object       handle;
    long                depth;
    gapi_statusMask mask;
};

static void
setListenerInitial(
    _Status status,
    gapi_boolean active,
    gapi_statusMask initmask
    );

static long
findListener(
    _Status status,
    gapi_statusMask mask);
    
static void
findCommonInterest(
    _Status status,
    _Status requester);

static gapi_boolean
setInterest (
    _Status status,
    _ListenerInterestInfo _info);

static gapi_boolean
setEnabledMask(
    _Status status,
    _ListenerInterestInfo _info);

void
_StatusInit(
    _Status status,
    _Entity entity,
    _Status parent,
    long depth,
    _StatusKind kind,
    gapi_statusMask valid,
    gapi_statusMask interest,
    gapi_statusMask initmask,
    gapi_boolean active,
    ListenerAction notify)
{
    int i;

    assert(status);
    
    status->entity = entity;
    status->parent = parent;
    status->depth  = depth;
    status->kind   = kind;
    status->validMask    = valid;
    status->interestMask = interest;
    status->notify       = notify;
    status->dispatchOn   = FALSE;
    
    for ( i = 0; i < MAX_LISTENER_DEPTH; i++ ) {
        memset(&status->listenerInfo[i], 0, sizeof(_ListenerInfo));
    }

    status->enabled    = GAPI_STATUS_KIND_NULL;
    status->userEntity = _EntityUEntity(entity);

    setListenerInitial(status, active, initmask);
}

void
_StatusDeinit(
    _Status status)
{
    C_STRUCT(_ListenerInterestInfo) _info;
    
    assert(status);

    _info.handle = NULL;
    _info.depth  = status->depth;
    _info.mask   = GAPI_STATUS_KIND_NULL;

    setInterest(status, &_info);

    status->enabled = GAPI_STATUS_KIND_NULL;

    _EntityDelete(status);
}

static void
setListenerInitial(
    _Status status,
    gapi_boolean active,
    gapi_statusMask initmask)
{
    if ( active ) {
        _StatusSetListener(status, initmask);
    }

    if ( status->parent ) {
        findCommonInterest(status->parent, status);
    }
}

gapi_boolean
_StatusSetListener(
    _Status status,
    gapi_statusMask mask)
{
    C_STRUCT(_ListenerInterestInfo) _info;

    if ( mask & GAPI_DATA_ON_READERS_STATUS ) {
        if ( (status->kind == STATUS_KIND_PARTICIPANT) || (status->kind == STATUS_KIND_SUBSCRIBER) ) {
            mask |= GAPI_DATA_AVAILABLE_STATUS;
        }
    }

    mask &= status->interestMask;

    _info.handle = _EntityHandle(status->entity);
    _info.depth  = status->depth;
    _info.mask   = mask;

    return setInterest(status, &_info);
}

gapi_boolean
_StatusSetListenerInterest (
    _Status status,
    _ListenerInterestInfo _info)
{
    gapi_boolean result;

    result = setInterest(status, _info);
    return result;
}

static gapi_boolean
setInterest (
    _Status status,
    _ListenerInterestInfo _info)
{
    gapi_boolean result = TRUE;
    
    if(status){
        assert(_info->depth >=0 && _info->depth < MAX_LISTENER_DEPTH);

        status->listenerInfo[_info->depth].handle = _info->handle;
        status->listenerInfo[_info->depth].mask   = _info->mask;
     
        setEnabledMask(status, _info);
    
        switch ( status->kind ) {
            case STATUS_KIND_PARTICIPANT:
                result = _DomainParticipantStatusSetInterest(status, _info);
                break;
            case STATUS_KIND_PUBLISHER:
                result = _PublisherStatusSetInterest(status, _info);
                break;
            case STATUS_KIND_SUBSCRIBER:
                result = _SubscriberStatusSetInterest(status, _info);
                break;
            default:
                break;
        }
    }
    return result;
}
    
gapi_statusMask
_StatusSetListenerMask(
        _Status status)
{
    assert(status->depth >=0 && status->depth < MAX_LISTENER_DEPTH);
    return status->listenerInfo[status->depth].mask;
}

gapi_statusMask
_StatusGetCurrentStatus(
    _Status status)
{
    long events = 0L;
    gapi_statusMask currentStatus;
   
    events = kernelStatusGet(status->userEntity);
    
    if ( _ObjectGetKind(_Object(status)) == OBJECT_KIND_SUBSCRIBER_STATUS ) {
        currentStatus = _StatusGetMaskStatus(status, events) & GAPI_DATA_ON_READERS_STATUS;
    } else {
        currentStatus = _StatusGetMaskStatus(status, events) & status->validMask;
    }

    return currentStatus;
}

gapi_object
_StatusFindTarget(
    _Status status,
    gapi_statusMask mask)
{
    gapi_object result = NULL;
    long i;

    assert(status->depth >=0 && status->depth < MAX_LISTENER_DEPTH);
    for ( i = status->depth; !result && i >= 0; i-- ) {
        if ( (status->listenerInfo[i].mask & mask) != GAPI_STATUS_KIND_NULL ) {
            result = status->listenerInfo[i].handle;
        }
    }

    return result;
}

static long
findListener(
    _Status status,
    gapi_statusMask mask)
{
    long result = -1;
    long i;

    assert(status->depth >=0 && status->depth < MAX_LISTENER_DEPTH);

    for ( i = status->depth; result == -1 && i >= 0; i-- ) {
        if ( (status->listenerInfo[i].mask & mask) != GAPI_STATUS_KIND_NULL ) {
            result = i;
        }
    }

    return result;
}

static void
findCommonInterest(
    _Status status,
    _Status requester)
{
    C_STRUCT(_ListenerInterestInfo) _info;
   
    _EntityClaim(status);
        
    assert(status->depth >=0 && status->depth < MAX_LISTENER_DEPTH);

    _info.handle = _EntityHandle(status->entity);
    _info.depth  = status->depth;
    _info.mask   = status->listenerInfo[status->depth].mask;
 
    setInterest(requester, &_info);
    
    if ( status->parent ) {
        findCommonInterest(status->parent, requester);
    }
   
    _EntityRelease(status);
}

static _DomainParticipant
getParticipant (
    _Status status)
{
    _DomainParticipant participant;

    if ( _ObjectGetKind(_Object(status->entity)) == OBJECT_KIND_DOMAINPARTICIPANT ) {
        participant = (_DomainParticipant)status->entity;
    } else {
        participant = _DomainEntityParticipant((_DomainEntity)status->entity);
    }

    return participant;
}
    
static gapi_boolean
setEnabledMask(
    _Status status,
    _ListenerInterestInfo _info
    )
{
    long i;
    gapi_statusMask mask = GAPI_STATUS_KIND_NULL;

    assert(status->depth >=0 && status->depth < MAX_LISTENER_DEPTH);

    for ( i = 0; i <= status->depth; i++ ) {
        mask |= status->listenerInfo[i].mask;
    }

    mask &= status->validMask;

    if ( mask != GAPI_STATUS_KIND_NULL ) {
        if ( status->enabled == GAPI_STATUS_KIND_NULL ) {
            _DomainParticipantAddListenerInterest(getParticipant(status),
                                                  status);
        }
    } else {
        if ( status->enabled != GAPI_STATUS_KIND_NULL ) {
            _DomainParticipantRemoveListenerInterest(getParticipant(status),
                                                     status);
        }
    }

    status->enabled = mask;

    return TRUE;
}


gapi_statusMask
_StatusGetMaskStatus (
    _Status status,
    c_long  eventKindMask)
{
    gapi_statusMask StatusMask = GAPI_STATUS_KIND_NULL;

    switch ( status->kind ) {
    case STATUS_KIND_TOPIC:
        if ( eventKindMask & V_EVENT_INCONSISTENT_TOPIC ) {
            StatusMask |= GAPI_INCONSISTENT_TOPIC_STATUS;
        }
        if ( eventKindMask & V_EVENT_ALL_DATA_DISPOSED ) {
            StatusMask |= GAPI_ALL_DATA_DISPOSED_STATUS;
        }
        break;
    case STATUS_KIND_SUBSCRIBER:
        if ( eventKindMask & V_EVENT_DATA_AVAILABLE ) {
            StatusMask |= GAPI_DATA_ON_READERS_STATUS;
        }
        break;    
    case STATUS_KIND_DATAWRITER:
        if ( eventKindMask & V_EVENT_LIVELINESS_LOST ) {
            StatusMask |= GAPI_LIVELINESS_LOST_STATUS;
        }
        if ( eventKindMask & V_EVENT_DEADLINE_MISSED ) {
            StatusMask |= GAPI_OFFERED_DEADLINE_MISSED_STATUS;
        }
        if ( eventKindMask & V_EVENT_INCOMPATIBLE_QOS ) {
            StatusMask |= GAPI_OFFERED_INCOMPATIBLE_QOS_STATUS;
        }
        break;    
    case STATUS_KIND_DATAREADER:
        if ( eventKindMask & V_EVENT_SAMPLE_REJECTED ) {
            StatusMask |= GAPI_SAMPLE_REJECTED_STATUS;
        }
        if ( eventKindMask & V_EVENT_LIVELINESS_CHANGED ) {
            StatusMask |= GAPI_LIVELINESS_CHANGED_STATUS;
        }
        if ( eventKindMask & V_EVENT_DEADLINE_MISSED ) {
            StatusMask |= GAPI_REQUESTED_DEADLINE_MISSED_STATUS;
        }
        if ( eventKindMask & V_EVENT_INCOMPATIBLE_QOS) {
            StatusMask |= GAPI_REQUESTED_INCOMPATIBLE_QOS_STATUS;
        }
        if ( eventKindMask & V_EVENT_DATA_AVAILABLE ) {
            StatusMask |= GAPI_DATA_AVAILABLE_STATUS;
        }
        if ( eventKindMask & V_EVENT_SAMPLE_LOST ) {
            StatusMask |= GAPI_SAMPLE_LOST_STATUS;
        }
        break;    
    default:
        break;
    }

    return StatusMask;
}

void
_StatusDefaultListener(
    _Entity entity,
    gapi_statusMask mask)
{
}
