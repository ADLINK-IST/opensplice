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
#include "gapi_object.h"
#include "gapi_domainParticipantStatus.h"
#include "gapi_subscriberStatus.h"
#include "gapi_subscriber.h"

_SubscriberStatus
_SubscriberStatusNew(
    _Subscriber entity,
    const struct gapi_subscriberListener *_listener,
    const gapi_statusMask mask)
{
    _SubscriberStatus subscriberStatus;

    subscriberStatus = _SubscriberStatusAlloc();

    if ( subscriberStatus != NULL ) {
        _Entity      factory;
        _Status      parent;
        gapi_boolean active = FALSE;
        _Status      status = _Status(subscriberStatus);

        factory = _EntityGetFactory(_Entity(entity));
        parent  = _EntityGetStatus(_Entity(factory));
        
        if ( _listener ) {
            status->callbackInfo.listenerData =
                    _listener->listener_data;
            status->callbackInfo.on_requested_deadline_missed =
                    _listener->on_requested_deadline_missed;
            status->callbackInfo.on_requested_incompatible_qos =
                    _listener->on_requested_incompatible_qos;
            status->callbackInfo.on_sample_rejected =
                    _listener->on_sample_rejected;
            status->callbackInfo.on_liveliness_changed =
                    _listener->on_liveliness_changed;
            status->callbackInfo.on_data_available =
                    _listener->on_data_available;
            status->callbackInfo.on_subscription_match =
                    _listener->on_subscription_match;
            status->callbackInfo.on_sample_lost =
                    _listener->on_sample_lost;
            status->callbackInfo.on_data_on_readers =
                    _listener->on_data_on_readers;
            active = TRUE;
        }           
                 
        _StatusInit(status, _Entity(entity), parent, 1,
                    STATUS_KIND_SUBSCRIBER, GAPI_STATUS_KIND_NULL,
                    SUBSCRIBER_STATUS_INTEREST,mask, active,
                    (ListenerAction)_SubscriberNotifyListener);
    }

    _EntityRelease(subscriberStatus);

    return subscriberStatus;
}   

void
_SubscriberStatusFree (
    _SubscriberStatus info
    )
{
    assert(info);

    _EntityClaim(info);

    _StatusDeinit(_Status(info));
}

gapi_boolean
_SubscriberStatusSetListener(
    _SubscriberStatus _this,
    const struct gapi_subscriberListener *_listener,
    gapi_statusMask mask)
{
    gapi_boolean result = FALSE;
    _Status status = _Status(_this);
     
    if ( _listener ) {
        status->callbackInfo.listenerData =
                _listener->listener_data;
        status->callbackInfo.on_requested_deadline_missed =
                _listener->on_requested_deadline_missed;
        status->callbackInfo.on_requested_incompatible_qos =
                _listener->on_requested_incompatible_qos;
        status->callbackInfo.on_sample_rejected =
                _listener->on_sample_rejected;
        status->callbackInfo.on_liveliness_changed =
                _listener->on_liveliness_changed;
        status->callbackInfo.on_data_available =
                _listener->on_data_available;
        status->callbackInfo.on_subscription_match =
                _listener->on_subscription_match;
        status->callbackInfo.on_sample_lost =
                _listener->on_sample_lost;
        status->callbackInfo.on_data_on_readers =
                _listener->on_data_on_readers;
    } else {
        mask = GAPI_STATUS_KIND_NULL;
    }
  
    result = _StatusSetListener(_Status(_this), mask);

    return result;
}

gapi_boolean
_SubscriberStatusSetInterest (
    _Status _this,
    _ListenerInterestInfo _info)
{
    _Subscriber subscriber = _Subscriber(_this->entity);

    return _SubscriberSetListenerInterestOnChildren(subscriber, _info);
}

