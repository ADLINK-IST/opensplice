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
#include "gapi_topic.h"
#include "gapi_topicStatus.h"
#include "gapi_kernel.h"

_TopicStatus
_TopicStatusNew (
    _Topic entity,
    const struct gapi_topicListener *_listener,
    const gapi_statusMask mask)
{
    _TopicStatus topicStatus;

    topicStatus = _TopicStatusAlloc();

    if ( topicStatus != NULL ) {
        _Entity      factory;
        _Status      parent;
        gapi_boolean active = FALSE;
        _Status      status = _Status(topicStatus);

        if ( _listener ) {
            status->callbackInfo.listenerData =
                    _listener->listener_data;
            status->callbackInfo.on_inconsistent_topic =
                    _listener->on_inconsistent_topic;
            status->callbackInfo.on_all_data_disposed =
                    _listener->on_all_data_disposed;
            active = TRUE;
        }           
 
        factory = _EntityGetFactory(_Entity(entity));
        parent  = _EntityGetStatus(_Entity(factory));
                
        _StatusInit(status, _Entity(entity), parent, 1,
                    STATUS_KIND_TOPIC, TOPIC_STATUS_MASK,
                    TOPIC_STATUS_INTEREST, mask, active,
                    (ListenerAction)_TopicNotifyListener);
    }

    _EntityRelease(topicStatus);

    return topicStatus;
}

void
_TopicStatusFree (
    _TopicStatus info
    )
{
    assert(info);

    _EntityClaim(info);

    _StatusDeinit(_Status(info));
}

gapi_boolean
_TopicStatusSetListener(
    _TopicStatus _this,
    const struct gapi_topicListener *_listener,
    gapi_statusMask mask)
{
    gapi_boolean result = FALSE;
    _Status status = _Status(_this);
    
    if ( _listener ) {
        status->callbackInfo.listenerData =
                _listener->listener_data;
        status->callbackInfo.on_inconsistent_topic =
                _listener->on_inconsistent_topic;
        status->callbackInfo.on_all_data_disposed =
                _listener->on_all_data_disposed;
    } else {
        mask = GAPI_STATUS_KIND_NULL;
    }

    result = _StatusSetListener(_Status(_this), mask);

    return result;
}

