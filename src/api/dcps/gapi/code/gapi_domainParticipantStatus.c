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

#include "os_heap.h"
#include "gapi_domainParticipantStatus.h"
#include "gapi_domainParticipant.h"

_DomainParticipantStatus
_DomainParticipantStatusNew (
    _DomainParticipant             entity,
    const struct gapi_domainParticipantListener *_listener,
    const gapi_statusMask mask
    )
{
    _DomainParticipantStatus participantStatus;

    assert(entity);
    
    participantStatus = _DomainParticipantStatusAlloc();

    if ( participantStatus != NULL ) {
        gapi_boolean active = FALSE;
        _Status      status = _Status(participantStatus);
        
        if ( _listener ) {
            status->callbackInfo.listenerData = _listener->listener_data;
            status->callbackInfo.on_inconsistent_topic = _listener->on_inconsistent_topic;
            status->callbackInfo.on_all_data_disposed = _listener->on_all_data_disposed;
            status->callbackInfo.on_requested_deadline_missed = _listener->on_requested_deadline_missed;
            status->callbackInfo.on_requested_incompatible_qos = _listener->on_requested_incompatible_qos;
            status->callbackInfo.on_sample_rejected = _listener->on_sample_rejected;
            status->callbackInfo.on_liveliness_changed = _listener->on_liveliness_changed;
            status->callbackInfo.on_data_available = _listener->on_data_available;
            status->callbackInfo.on_subscription_match = _listener->on_subscription_match;
            status->callbackInfo.on_sample_lost = _listener->on_sample_lost;
            status->callbackInfo.on_data_on_readers = _listener->on_data_on_readers;
            status->callbackInfo.on_offered_deadline_missed = _listener->on_offered_deadline_missed;
            status->callbackInfo.on_offered_incompatible_qos = _listener->on_offered_incompatible_qos;
            status->callbackInfo.on_liveliness_lost = _listener->on_liveliness_lost;
            status->callbackInfo.on_publication_match = _listener->on_publication_match;
            active = TRUE;
        }

        _StatusInit(status, _Entity(entity), NULL, 0,
                    STATUS_KIND_PARTICIPANT, PARTICIPANT_STATUS_MASK,
                    PARTICIPANT_STATUS_INTEREST,mask, active, _StatusDefaultListener);
    }

    _EntityRelease(participantStatus);

    return participantStatus;
}

void
_DomainParticipantStatusFree (
    _DomainParticipantStatus info
    )
{
    assert(info);

    _EntityClaim(info);
    
    _StatusDeinit(_Status(info));
}


gapi_boolean
_DomainParticipantStatusSetListener(
    _DomainParticipantStatus _this,
    const struct gapi_domainParticipantListener *_listener,
    gapi_statusMask mask)
{
    gapi_boolean result = FALSE;
    _Status status = _Status(_this);
     
    if ( _listener ) {            
        status->callbackInfo.listenerData = _listener->listener_data;
        status->callbackInfo.on_inconsistent_topic = _listener->on_inconsistent_topic;
        status->callbackInfo.on_all_data_disposed = _listener->on_all_data_disposed;
        status->callbackInfo.on_requested_deadline_missed = _listener->on_requested_deadline_missed;
        status->callbackInfo.on_requested_incompatible_qos = _listener->on_requested_incompatible_qos;
        status->callbackInfo.on_sample_rejected = _listener->on_sample_rejected;
        status->callbackInfo.on_liveliness_changed = _listener->on_liveliness_changed;
        status->callbackInfo.on_data_available = _listener->on_data_available;
        status->callbackInfo.on_subscription_match = _listener->on_subscription_match;
        status->callbackInfo.on_sample_lost = _listener->on_sample_lost;
        status->callbackInfo.on_data_on_readers = _listener->on_data_on_readers;
        status->callbackInfo.on_offered_deadline_missed = _listener->on_offered_deadline_missed;
        status->callbackInfo.on_offered_incompatible_qos = _listener->on_offered_incompatible_qos;
        status->callbackInfo.on_liveliness_lost = _listener->on_liveliness_lost;
        status->callbackInfo.on_publication_match = _listener->on_publication_match;
    } else {
        mask = GAPI_STATUS_KIND_NULL;
    }
        
    result = _StatusSetListener(status, mask);

    return result;
}



gapi_boolean
_DomainParticipantStatusSetInterest (
    _Status               _this,
    _ListenerInterestInfo _info
    )
{
    _DomainParticipant participant = _DomainParticipant(_this->entity);

    return _DomainParticipantSetListenerInterestOnChildren(participant, _info);
}
    

