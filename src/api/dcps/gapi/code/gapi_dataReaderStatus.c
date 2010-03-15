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
#include "gapi_subscriberStatus.h"
#include "gapi_subscriber.h"
#include "gapi_dataReaderStatus.h"
#include "gapi_dataReader.h"
#include "gapi_kernel.h"

#include "v_event.h"
#include "v_status.h"
#include "v_subscriber.h"

_DataReaderStatus
_DataReaderStatusNew(
    _DataReader entity,
    const struct gapi_dataReaderListener *_listener,
    const gapi_statusMask mask)
{
    _DataReaderStatus readerStatus;

    readerStatus = _DataReaderStatusAlloc();

    if ( readerStatus != NULL ) {
        _Entity      factory;
        _Status      parent;
        gapi_boolean active = FALSE;
        _Status      status = _Status(readerStatus);

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
            active = TRUE;
        }           
                
        _StatusInit(status, _Entity(entity), parent, 2,
                    STATUS_KIND_DATAREADER, READER_STATUS_MASK,
                    READER_STATUS_INTEREST,mask,
                    active, (ListenerAction)_DataReaderNotifyListener);
    }

    _EntityRelease(readerStatus);

    return readerStatus;
}   


void
_DataReaderStatusFree (
    _DataReaderStatus info)
{
    assert(info);

    _EntityClaim(info);

    _StatusDeinit(_Status(info));
}

gapi_boolean
_DataReaderStatusSetListener(
    _DataReaderStatus       _this,
    const struct gapi_dataReaderListener *_listener,
    gapi_statusMask     mask)
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
    } else {
        mask = GAPI_STATUS_KIND_NULL;
    }
        
    result = _StatusSetListener(status, mask);

    return result;
}

static void
gapi_requestedIncompatibleQosStatus_free (
    void *object)
{
    gapi_requestedIncompatibleQosStatus *o;

    o = (gapi_requestedIncompatibleQosStatus *) object;
    
    gapi_free(o->policies._buffer);
}


gapi_requestedIncompatibleQosStatus *
gapi_requestedIncompatibleQosStatus_alloc (
    void)
{
    gapi_requestedIncompatibleQosStatus *newStatus;

    newStatus = (gapi_requestedIncompatibleQosStatus *)
                    gapi__malloc(gapi_requestedIncompatibleQosStatus_free, 0, sizeof(gapi_requestedIncompatibleQosStatus));

    if ( newStatus ) {
        newStatus->policies._buffer  = gapi_qosPolicyCountSeq_allocbuf(MAX_POLICY_COUNT_ID);
        newStatus->policies._length  = 0;
        newStatus->policies._maximum = MAX_POLICY_COUNT_ID;
        newStatus->policies._release = TRUE;
        
        if ( newStatus->policies._buffer == NULL ) {
            gapi_free(newStatus);
            newStatus = NULL;
        }
    }

    return newStatus;
}
