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
#include "gapi_publisherStatus.h"
#include "gapi_dataWriter.h"
#include "gapi_dataWriterStatus.h"
#include "gapi_kernel.h"

_DataWriterStatus
_DataWriterStatusNew(
    _DataWriter entity,
    const struct gapi_dataWriterListener *_listener,
    const gapi_statusMask mask)
{
    _DataWriterStatus writerStatus;

    writerStatus = _DataWriterStatusAlloc();

    if ( writerStatus != NULL ) {
        _Entity      factory;
        _Status      parent;
        gapi_boolean active = FALSE;
        _Status      status = _Status(writerStatus);

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
                  
        _StatusInit(status, _Entity(entity), parent, 2,
                    STATUS_KIND_DATAWRITER, WRITER_STATUS_MASK,
                    WRITER_STATUS_INTEREST,mask, active,
                    (ListenerAction)_DataWriterNotifyListener);
    }

    _EntityRelease(writerStatus);

    return writerStatus;
}   

void
_DataWriterStatusFree (
    _DataWriterStatus info)
{
    assert(info);

    _EntityClaim(info);

    _StatusDeinit(_Status(info));
}

gapi_boolean
_DataWriterStatusSetListener(
    _DataWriterStatus _this,
    const struct gapi_dataWriterListener *_listener,
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

    result = _StatusSetListener(status, mask);

    return result;
}

static void
gapi_offeredIncompatibleQosStatus_free (
    void *object)
{
    gapi_offeredIncompatibleQosStatus *o;

    o = (gapi_offeredIncompatibleQosStatus *) object;
    
    gapi_free(o->policies._buffer);
}

gapi_offeredIncompatibleQosStatus *
gapi_offeredIncompatibleQosStatus_alloc (
    void)
{
    gapi_offeredIncompatibleQosStatus *newStatus;

    newStatus = (gapi_offeredIncompatibleQosStatus *)
                    gapi__malloc(gapi_offeredIncompatibleQosStatus_free, 0, sizeof(gapi_offeredIncompatibleQosStatus));

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

