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
#include <stdio.h>
#include "dds_dcps.h"
#include "os_report.h"
#include "qp_qosProvider.h"

static DDS_ReturnCode_t
_QosProvider_deinit (
    void *_this);

#if 1
/* Code inside if block can safely be removed when merged into the "Core
   Redesign" branch. */
#include "gapi.h"
#include "gapi_common.h"
#include "gapi_object.h"
#include "gapi_qos.h"
#include "gapi_structured.h"
#include "gapi_objManag.h"

/* Create _QosProvider type and DDS_QOSPROVIDER type definition in sac_common.h
   when used in "Core Redesign" branch. */
C_CLASS(_QosProvider);

/* uri and profile are stored at lower level only */
C_STRUCT(_QosProvider) {
    C_EXTENDS(_Object);
    qp_qosProvider qpQosProvider;
};

static DDS_ReturnCode_t
gapiReturnCodeToReturnCode(
    gapi_returnCode_t gapiResult)
{
    DDS_ReturnCode_t result = DDS_RETCODE_ERROR;

    switch (gapiResult) {
        case GAPI_RETCODE_OK:
            result = DDS_RETCODE_OK;
            break;
        case GAPI_RETCODE_UNSUPPORTED:
            result = DDS_RETCODE_UNSUPPORTED;
            break;
        case GAPI_RETCODE_BAD_PARAMETER:
            result = DDS_RETCODE_BAD_PARAMETER;
            break;
        case GAPI_RETCODE_PRECONDITION_NOT_MET:
            result = DDS_RETCODE_PRECONDITION_NOT_MET;
            break;
        case GAPI_RETCODE_OUT_OF_RESOURCES:
            result = DDS_RETCODE_OUT_OF_RESOURCES;
            break;
        case GAPI_RETCODE_NOT_ENABLED:
            result = DDS_RETCODE_NOT_ENABLED;
            break;
        case GAPI_RETCODE_IMMUTABLE_POLICY:
            result = DDS_RETCODE_IMMUTABLE_POLICY;
            break;
        case GAPI_RETCODE_INCONSISTENT_POLICY:
            result = DDS_RETCODE_INCONSISTENT_POLICY;
            break;
        case GAPI_RETCODE_ALREADY_DELETED:
            result = DDS_RETCODE_ALREADY_DELETED;
            break;
        case GAPI_RETCODE_TIMEOUT:
            result = DDS_RETCODE_TIMEOUT;
            break;
        case GAPI_RETCODE_NO_DATA:
            result = DDS_RETCODE_NO_DATA;
            break;
        case GAPI_RETCODE_ILLEGAL_OPERATION:
            result = DDS_RETCODE_ILLEGAL_OPERATION;
            break;
        case GAPI_RETCODE_ERROR:
        default:
            result = DDS_RETCODE_ERROR;
            break;
    }

    return result;
}

#define DDS_QosProviderClaimRead(handle, object) \
    (DDS_QosProviderReadLock ((gapi_handle)(handle), (object)))

static DDS_ReturnCode_t
DDS_QosProviderReadLock (
    gapi_handle handle,
    _QosProvider *object)
{
    _QosProvider qosProvider;
    gapi_returnCode_t gapiResult;
    DDS_ReturnCode_t result = DDS_RETCODE_OK;

    assert (handle != NULL);
    assert (object != NULL);

    qosProvider = (_QosProvider)
        gapi_objectReadClaim (handle, OBJECT_KIND_QOSPROVIDER, &gapiResult);
    result = gapiReturnCodeToReturnCode (gapiResult);

    if (result != DDS_RETCODE_OK) {
        OS_REPORT (OS_ERROR, "DDS_QosProviderReadLock", 0,
            "Could not read lock QosProvider");
    } else {
        *object = qosProvider;
    }

    return result;
}

#define DDS_QosProviderRelease(handle) \
    (DDS_QosProviderUnlock ((gapi_handle)(handle)))

static DDS_ReturnCode_t
DDS_QosProviderUnlock(
    gapi_handle handle)
{
    assert (handle != NULL);

    gapi_objectRelease (handle);

    return DDS_RETCODE_OK;
}

static DDS_ReturnCode_t
DDS_DomainParticipantQos_init (
    DDS_DomainParticipantQos *qos,
    const DDS_DomainParticipantQos *template)
{
    DDS_ReturnCode_t result;

    assert (template != NULL);

    if (qos != NULL) {
        (void)gapi_domainParticipantQosCopy (
            (gapi_domainParticipantQos *)template,
            (gapi_domainParticipantQos *)qos);
        result = DDS_RETCODE_OK;
    } else {
        result = DDS_RETCODE_BAD_PARAMETER;
    }

    return result;
}

static DDS_ReturnCode_t
DDS_DomainParticipantQos_deinit (
    DDS_DomainParticipantQos *qos)
{
    (void)gapi_domainParticipantQos_free (qos);
    return DDS_RETCODE_OK;
}

static DDS_ReturnCode_t
DDS_TopicQos_init (
    DDS_TopicQos *qos,
    const DDS_TopicQos *template)
{
    DDS_ReturnCode_t result;

    assert (template != NULL);

    if (qos != NULL) {
        (void)gapi_topicQosCopy (
            (gapi_topicQos *) template,
            (gapi_topicQos *) qos);
        result = DDS_RETCODE_OK;
    } else {
        result = DDS_RETCODE_BAD_PARAMETER;
    }

    return result;
}

static DDS_ReturnCode_t
DDS_TopicQos_deinit (
    DDS_TopicQos *qos)
{
    (void)gapi_topicQos_free (qos);
    return DDS_RETCODE_OK;
}

static DDS_ReturnCode_t
DDS_PublisherQos_init (
    DDS_PublisherQos *qos,
    const DDS_PublisherQos *template)
{
    DDS_ReturnCode_t result;

    assert (template != NULL);

    if (qos != NULL) {
        (void)gapi_publisherQosCopy (
            (gapi_publisherQos *) template,
            (gapi_publisherQos *) qos);
        result = DDS_RETCODE_OK;
    } else {
        result = DDS_RETCODE_BAD_PARAMETER;
    }

    return result;
}

static DDS_ReturnCode_t
DDS_PublisherQos_deinit (
    DDS_PublisherQos *qos)
{
    (void)gapi_publisherQos_free (qos);
    return DDS_RETCODE_OK;
}

static DDS_ReturnCode_t
DDS_DataWriterQos_init (
    DDS_DataWriterQos *qos,
    const DDS_DataWriterQos *template)
{
    DDS_ReturnCode_t result;

    assert (template != NULL);

    if (qos != NULL) {
        (void)gapi_dataWriterQosCopy (
            (gapi_dataWriterQos *) template,
            (gapi_dataWriterQos *) qos);
        result = DDS_RETCODE_OK;
    } else {
        result = DDS_RETCODE_BAD_PARAMETER;
    }

    return result;
}

static DDS_ReturnCode_t
DDS_DataWriterQos_deinit (
    DDS_DataWriterQos *qos)
{
    (void)gapi_dataWriterQos_free (qos);
    return DDS_RETCODE_OK;
}

static DDS_ReturnCode_t
DDS_SubscriberQos_init (
    DDS_SubscriberQos *qos,
    const DDS_SubscriberQos *template)
{
    DDS_ReturnCode_t result;

    assert (template != NULL);

    if (qos != NULL) {
        (void)gapi_subscriberQosCopy (
            (gapi_subscriberQos *) template,
            (gapi_subscriberQos *) qos);
        result = DDS_RETCODE_OK;
    } else {
        result = DDS_RETCODE_BAD_PARAMETER;
    }

    return result;
}

static DDS_ReturnCode_t
DDS_SubscriberQos_deinit (
    DDS_SubscriberQos *qos)
{
    (void)gapi_subscriberQos_free (qos);
    return DDS_RETCODE_OK;
}

static DDS_ReturnCode_t
DDS_DataReaderQos_init (
    DDS_DataReaderQos *qos,
    const DDS_DataReaderQos *template)
{
    DDS_ReturnCode_t result;

    assert (template != NULL);

    if (qos != NULL) {
        (void)gapi_dataReaderQosCopy (
            (gapi_dataReaderQos *) template,
            (gapi_dataReaderQos *) qos);
        result = DDS_RETCODE_OK;
    } else {
        result = DDS_RETCODE_BAD_PARAMETER;
    }

    return result;
}

static DDS_ReturnCode_t
DDS_DataReaderQos_deinit (
    DDS_DataReaderQos *qos)
{
    (void)gapi_dataReaderQos_free (qos);
    return DDS_RETCODE_OK;
}

#define DDS_QOSPROVIDER OBJECT_KIND_QOSPROVIDER

static gapi_boolean
_QosProviderFree(
    void *_this)
{
    gapi_boolean result = TRUE;

    if (_QosProvider_deinit (_Object (_this)) != DDS_RETCODE_OK) {
        result = FALSE;
    }

    return result;
}

static DDS_ReturnCode_t
DDS_Object_new(
    _ObjectKind kind,
    DDS_ReturnCode_t (*deallocator)(void *),
    _Object *_this)
{
    _QosProvider qosProvider;
    DDS_ReturnCode_t result = DDS_RETCODE_OK;

    assert (deallocator != NULL);
    assert (_this != NULL);

    qosProvider = (_QosProvider)_ObjectAlloc (kind, C_SIZEOF(_QosProvider), _QosProviderFree);
    if (qosProvider != NULL) {
        /* *_this will actually be a handle after this call, because
         * _ObjectRelease(...) returns a handle... */
        *_this = _Object (_ObjectRelease ((_Object)qosProvider));
    } else {
        result = DDS_RETCODE_OUT_OF_RESOURCES;
    }

    return result;
}

/* _QosProvder behaves slightly differently due to passing of gapi_handle
   objects instead of pointers to the actual object. This macro is used in
   DDS_QosProvider__alloc only, please don't use it anywhere else. */
#define _QosProvider(handle) ((_QosProvider)gapi_objectPeekUnchecked (gapi_handle (handle)))
#else
/* QoS policies are available in "Core Redesign" branch */
#include "sac_common.h"

#define _QosProvider(object) ((_QosProvider)(object))
#endif

extern void
__DDS_NamedDomainParticipantQos__copyOut (void *_from, void *_to);
extern void
__DDS_NamedPublisherQos__copyOut (void *_from, void *_to);
extern void
__DDS_NamedSubscriberQos__copyOut (void *_from, void *_to);
extern void
__DDS_NamedTopicQos__copyOut (void *_from, void *_to);
extern void
__DDS_NamedDataWriterQos__copyOut (void *_from, void *_to);
extern void
__DDS_NamedDataReaderQos__copyOut (void *_from, void *_to);

static const C_STRUCT(qp_qosProviderInputAttr) qpQosProviderAttr = {
    { /* Participant QoS */
        (qp_copyOut)&__DDS_NamedDomainParticipantQos__copyOut
    },
    { /* Topic QoS */
        (qp_copyOut)&__DDS_NamedTopicQos__copyOut
    },
    { /* Subscriber QoS */
        (qp_copyOut)&__DDS_NamedSubscriberQos__copyOut
    },
    { /* DataReader QoS */
        (qp_copyOut)&__DDS_NamedDataReaderQos__copyOut
    },
    { /* Publisher QoS */
        (qp_copyOut)&__DDS_NamedPublisherQos__copyOut
    },
    { /* DataWriter QoS */
        (qp_copyOut)&__DDS_NamedDataWriterQos__copyOut
    }
};

static DDS_ReturnCode_t
qpResultToReturnCode (qp_result qpResult)
{
    DDS_ReturnCode_t result = DDS_RETCODE_ERROR;

    switch (qpResult) {
        case QP_RESULT_OK:
            result = DDS_RETCODE_OK;
            break;
        case QP_RESULT_OUT_OF_MEMORY:
            result = DDS_RETCODE_OUT_OF_RESOURCES;
            break;
        case QP_RESULT_ILL_PARAM:
            result = DDS_RETCODE_BAD_PARAMETER;
            break;
        case QP_RESULT_NO_DATA:
            result = DDS_RETCODE_NO_DATA;
            break;
        default:
            result = DDS_RETCODE_ERROR;
            break;
    }

    return result;
}

static DDS_ReturnCode_t
_QosProvider_deinit (
    void *_this)
{
    _QosProvider qosProvider;

    assert(_this);

    qosProvider = (_QosProvider)_this;
    qp_qosProviderFree (qosProvider->qpQosProvider);
    qosProvider->qpQosProvider = NULL;

    return DDS_RETCODE_OK;
}

DDS_ReturnCode_t
DDS_QosProvider_get_participant_qos(
    DDS_QosProvider _this,
    DDS_DomainParticipantQos *qos,
    const char *id)
{
    _QosProvider qosProvider;
    qp_qosProvider qpQosProvider;
    qp_result qpResult;
    DDS_ReturnCode_t result = DDS_RETCODE_BAD_PARAMETER;
    DDS_NamedDomainParticipantQos namedParticipantQos;
    memset (&namedParticipantQos, 0, sizeof (DDS_NamedDomainParticipantQos));

    if (_this != NULL && qos != NULL) {
        result = DDS_QosProviderClaimRead (_this, &qosProvider);

        if (result == DDS_RETCODE_OK) {
            qpQosProvider = qosProvider->qpQosProvider;
            if (qpQosProvider == NULL) {
                result = DDS_RETCODE_PRECONDITION_NOT_MET;
            }
            if (result == DDS_RETCODE_OK) {
                qpResult = qp_qosProviderGetParticipantQos (
                    qpQosProvider, id, &namedParticipantQos);
                result = qpResultToReturnCode (qpResult);
            }
            if (result == DDS_RETCODE_OK) {
                result = DDS_DomainParticipantQos_init (
                    qos, &namedParticipantQos.participant_qos);
            }
            DDS_QosProviderRelease (_this);
        }
    }

    DDS_free(namedParticipantQos.name);
    (void)DDS_DomainParticipantQos_deinit (&namedParticipantQos.participant_qos);

    return result;
}

DDS_ReturnCode_t
DDS_QosProvider_get_topic_qos(
    DDS_QosProvider _this,
    DDS_TopicQos *qos,
    const char *id)
{
    _QosProvider qosProvider;
    qp_qosProvider qpQosProvider;
    qp_result qpResult;
    DDS_ReturnCode_t result = DDS_RETCODE_BAD_PARAMETER;
    DDS_NamedTopicQos namedTopicQos;
    memset (&namedTopicQos, 0, sizeof (DDS_NamedTopicQos));

    if (_this != NULL && qos != NULL) {
        result = DDS_QosProviderClaimRead (_this, &qosProvider);

        if (result == DDS_RETCODE_OK) {
            qpQosProvider = qosProvider->qpQosProvider;
            if (qpQosProvider == NULL) {
                result = DDS_RETCODE_PRECONDITION_NOT_MET;
            }
            if (result == DDS_RETCODE_OK) {
                qpResult = qp_qosProviderGetTopicQos (
                    qpQosProvider, id, &namedTopicQos);
                result = qpResultToReturnCode (qpResult);
            }
            if (result == DDS_RETCODE_OK) {
                result = DDS_TopicQos_init (
                    qos, &namedTopicQos.topic_qos);
            }
            DDS_QosProviderRelease (_this);
        }
    }

    DDS_free(&namedTopicQos.name);
    (void)DDS_TopicQos_deinit (&namedTopicQos.topic_qos);

    return result;
}

DDS_ReturnCode_t
DDS_QosProvider_get_subscriber_qos(
    DDS_QosProvider _this,
    DDS_SubscriberQos *qos,
    const char *id)
{
    _QosProvider qosProvider;
    qp_qosProvider qpQosProvider;
    qp_result qpResult;
    DDS_ReturnCode_t result = DDS_RETCODE_BAD_PARAMETER;
    DDS_NamedSubscriberQos namedSubscriberQos;
    memset (&namedSubscriberQos, 0, sizeof (DDS_NamedSubscriberQos));

    if (_this != NULL && qos != NULL) {
        result = DDS_QosProviderClaimRead (_this, &qosProvider);

        if (result == DDS_RETCODE_OK) {
            qpQosProvider = qosProvider->qpQosProvider;
            if (qpQosProvider == NULL) {
                result = DDS_RETCODE_PRECONDITION_NOT_MET;
            }
            if (result == DDS_RETCODE_OK) {
                qpResult = qp_qosProviderGetSubscriberQos (
                    qpQosProvider, id, &namedSubscriberQos);
                result = qpResultToReturnCode (qpResult);
            }
            if (result == DDS_RETCODE_OK) {
                result = DDS_SubscriberQos_init (
                    qos, &namedSubscriberQos.subscriber_qos);
            }
            DDS_QosProviderRelease (_this);
        }
    }

    DDS_free(&namedSubscriberQos.name);
    (void)DDS_SubscriberQos_deinit (&namedSubscriberQos.subscriber_qos);

    return result;
}

DDS_ReturnCode_t
DDS_QosProvider_get_datareader_qos(
    DDS_QosProvider _this,
    DDS_DataReaderQos *qos,
    const char *id)
{
    _QosProvider qosProvider;
    qp_qosProvider qpQosProvider;
    qp_result qpResult;
    DDS_ReturnCode_t result = DDS_RETCODE_BAD_PARAMETER;
    DDS_NamedDataReaderQos namedDataReaderQos;
    memset (&namedDataReaderQos, 0, sizeof (DDS_NamedDataReaderQos));

    if (_this != NULL && qos != NULL) {
        result = DDS_QosProviderClaimRead (_this, &qosProvider);

        if (result == DDS_RETCODE_OK) {
            qpQosProvider = qosProvider->qpQosProvider;
            if (qpQosProvider == NULL) {
                result = DDS_RETCODE_PRECONDITION_NOT_MET;
            }
            if (result == DDS_RETCODE_OK) {
                qpResult = qp_qosProviderGetDataReaderQos (
                    qpQosProvider, id, &namedDataReaderQos);
                result = qpResultToReturnCode (qpResult);
            }
            if (result == DDS_RETCODE_OK) {
                result = DDS_DataReaderQos_init (
                    qos, &namedDataReaderQos.datareader_qos);
            }
            DDS_QosProviderRelease (_this);
        }
    }

    DDS_free(&namedDataReaderQos.name);
    (void)DDS_DataReaderQos_deinit (&namedDataReaderQos.datareader_qos);

    return result;
}

DDS_ReturnCode_t
DDS_QosProvider_get_publisher_qos(
    DDS_QosProvider _this,
    DDS_PublisherQos *qos,
    const char *id)
{
    _QosProvider qosProvider;
    qp_qosProvider qpQosProvider;
    qp_result qpResult;
    DDS_ReturnCode_t result = DDS_RETCODE_BAD_PARAMETER;
    DDS_NamedPublisherQos namedPublisherQos;
    memset (&namedPublisherQos, 0, sizeof (DDS_NamedPublisherQos));

    if (_this != NULL && qos != NULL) {
        result = DDS_QosProviderClaimRead (_this, &qosProvider);

        if (result == DDS_RETCODE_OK) {
            qpQosProvider = qosProvider->qpQosProvider;
            if (qpQosProvider == NULL) {
                result = DDS_RETCODE_PRECONDITION_NOT_MET;
            }
            if (result == DDS_RETCODE_OK) {
                qpResult = qp_qosProviderGetPublisherQos (
                    qpQosProvider, id, &namedPublisherQos);
                result = qpResultToReturnCode (qpResult);
            }
            if (result == DDS_RETCODE_OK) {
                result = DDS_PublisherQos_init (
                    qos, &namedPublisherQos.publisher_qos);
            }
            DDS_QosProviderRelease (_this);
        }
    }

    DDS_free(&namedPublisherQos.name);
    (void)DDS_PublisherQos_deinit (&namedPublisherQos.publisher_qos);

    return result;
}

DDS_ReturnCode_t
DDS_QosProvider_get_datawriter_qos(
    DDS_QosProvider _this,
    DDS_DataWriterQos *qos,
    const char *id)
{
    _QosProvider qosProvider;
    qp_qosProvider qpQosProvider;
    qp_result qpResult;
    DDS_ReturnCode_t result = DDS_RETCODE_BAD_PARAMETER;
    DDS_NamedDataWriterQos namedDataWriterQos;
    memset (&namedDataWriterQos, 0, sizeof (DDS_NamedDataWriterQos));

    if (_this != NULL && qos != NULL) {
        result = DDS_QosProviderClaimRead (_this, &qosProvider);

        if (result == DDS_RETCODE_OK) {
            qpQosProvider = qosProvider->qpQosProvider;
            if (qpQosProvider == NULL) {
                result = DDS_RETCODE_PRECONDITION_NOT_MET;
            }
            if (result == DDS_RETCODE_OK) {
                qpResult = qp_qosProviderGetDataWriterQos (
                    qpQosProvider, id, &namedDataWriterQos);
                result = qpResultToReturnCode (qpResult);
            }
            if (result == DDS_RETCODE_OK) {
                result = DDS_DataWriterQos_init (
                    qos, &namedDataWriterQos.datawriter_qos);
            }
            DDS_QosProviderRelease (_this);
        }
    }

    DDS_free(&namedDataWriterQos.name);
    (void)DDS_DataWriterQos_deinit (&namedDataWriterQos.datawriter_qos);

    return result;
}

#define DDS_QosProvider_GetQpQosProvider()

DDS_QosProvider
DDS_QosProvider__alloc (
    const char *uri,
    const char *profile)
{
    _Object _this = NULL;
    qp_qosProvider qpQosProvider;
    DDS_ReturnCode_t result;

    if((qpQosProvider = qp_qosProviderNew (uri, profile, &qpQosProviderAttr)) == NULL){
        /* Error already reported by qp_qosProviderNew(...) */
        goto err_qp_qosProviderNew;
    }

    if((result = DDS_Object_new (DDS_QOSPROVIDER, _QosProvider_deinit, &_this)) != DDS_RETCODE_OK) {
        /* TODO: Report out-of-memory. */
        goto err_DDS_Object_new;
    }

    _QosProvider(_this)->qpQosProvider = qpQosProvider;

    return _this;

/* Error handling */
err_DDS_Object_new:
    qp_qosProviderFree(qpQosProvider);
err_qp_qosProviderNew:
    return NULL;
}
