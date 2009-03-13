#include "os_heap.h"
#include "gapi_object.h"
#include "gapi_publisher.h"
#include "gapi_publisherStatus.h"

_PublisherStatus
_PublisherStatusNew(
    _Publisher entity,
    const struct gapi_publisherListener *_listener,
    const gapi_statusMask mask)
{
    _PublisherStatus publisherStatus;

    publisherStatus = _PublisherStatusAlloc();

    if ( publisherStatus != NULL ) {
        _Entity      factory;
        _Status      parent;
        gapi_boolean active = FALSE;
        _Status      status = _Status(publisherStatus);

        factory = _EntityGetFactory(_Entity(entity));
        parent  = _EntityGetStatus(_Entity(factory));
         
        if ( _listener ) {
            status->callbackInfo.listenerData =
                    _listener->listener_data; 
            status->callbackInfo.on_offered_deadline_missed =
                    _listener->on_offered_deadline_missed;
            status->callbackInfo.on_offered_incompatible_qos =
                    _listener->on_offered_incompatible_qos;
            status->callbackInfo.on_liveliness_lost =
                    _listener->on_liveliness_lost;
            status->callbackInfo.on_publication_match =
                    _listener->on_publication_match;
            active = TRUE;
        }           
                
        _StatusInit(status, _Entity(entity), parent, 1,
                    STATUS_KIND_PUBLISHER, PUBLISHER_STATUS_MASK, 
                    PUBLISHER_STATUS_INTEREST,mask, active,
                    _StatusDefaultListener);
    }

    _EntityRelease(publisherStatus);

    return publisherStatus;
}   

void
_PublisherStatusFree (
    _PublisherStatus info)
{
    assert(info);

    _EntityClaim(info);

    _StatusDeinit(_Status(info));
}

gapi_boolean
_PublisherStatusSetListener(
    _PublisherStatus _this,
    const struct gapi_publisherListener *_listener,
    gapi_statusMask mask)
{
    gapi_boolean result = FALSE;
    _Status status = _Status(_this);
     
    if ( _listener ) {
        status->callbackInfo.listenerData =
                _listener->listener_data; 
        status->callbackInfo.on_offered_deadline_missed =
                _listener->on_offered_deadline_missed;
        status->callbackInfo.on_offered_incompatible_qos =
                _listener->on_offered_incompatible_qos;
        status->callbackInfo.on_liveliness_lost =
                _listener->on_liveliness_lost;
        status->callbackInfo.on_publication_match =
                _listener->on_publication_match;
    } else {
        mask = GAPI_STATUS_KIND_NULL;
    }

    result = _StatusSetListener(_Status(_this), mask);

    return result;
}


gapi_boolean
_PublisherStatusSetInterest (
    _Status _this,
    _ListenerInterestInfo _info)
{
    _Publisher publisher = _Publisher(_this->entity);

    return _PublisherSetListenerInterestOnChildren(publisher, _info);
}

