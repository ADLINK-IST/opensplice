/*
 *                         Vortex OpenSplice
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR ADLINK
 *   Technology Limited, its affiliated companies and licensors. All rights
 *   reserved.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 */
#include <stdio.h>
#include "dds_dcps.h"
#include "sac_report.h"
#include "sac_object.h"
#include "cmn_qosProvider.h"
#include "dds_namedQosTypes.h"

static DDS_ReturnCode_t
_QosProvider_deinit (
     _Object _this);

/* QoS policies are available in "Core Redesign" branch */
#include "sac_common.h"

#define _QosProvider(object) ((_QosProvider)(object))

#define DDS_QosProviderClaim(_this, qosProvider) \
        DDS_Object_claim(DDS_Object(_this), DDS_QOSPROVIDER, (_Object *)qosProvider)

#define DDS_QosProviderClaimRead(_this, qosProvider) \
        DDS_Object_claim(DDS_Object(_this), DDS_QOSPROVIDER, (_Object *)qosProvider)

#define DDS_QosProviderRelease(_this) \
        DDS_Object_release(DDS_Object(_this))

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

static const C_STRUCT(cmn_qosProviderInputAttr) qpQosProviderAttr = {
    { /* Participant QoS */
        (cmn_qpCopyOut)&__DDS_NamedDomainParticipantQos__copyOut
    },
    { /* Topic QoS */
        (cmn_qpCopyOut)&__DDS_NamedTopicQos__copyOut
    },
    { /* Subscriber QoS */
        (cmn_qpCopyOut)&__DDS_NamedSubscriberQos__copyOut
    },
    { /* DataReader QoS */
        (cmn_qpCopyOut)&__DDS_NamedDataReaderQos__copyOut
    },
    { /* Publisher QoS */
        (cmn_qpCopyOut)&__DDS_NamedPublisherQos__copyOut
    },
    { /* DataWriter QoS */
        (cmn_qpCopyOut)&__DDS_NamedDataWriterQos__copyOut
    }
};

static DDS_ReturnCode_t
qpResultToReturnCode (cmn_qpResult qpResult)
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
    _Object _this)
{
    _QosProvider qosProvider;

    assert(_this);

    qosProvider = (_QosProvider)_this;
    cmn_qosProviderFree (qosProvider->qpQosProvider);
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
    cmn_qosProvider qpQosProvider;
    cmn_qpResult qpResult;
    DDS_ReturnCode_t result = DDS_RETCODE_OK;
    DDS_NamedDomainParticipantQos namedParticipantQos;
    memset (&namedParticipantQos, 0, sizeof (DDS_NamedDomainParticipantQos));

    SAC_REPORT_STACK();

    if (_this == NULL) {
        result = DDS_RETCODE_BAD_PARAMETER;
        SAC_REPORT(result, "QosProvider = NULL");
        goto err_bad_parameter;
    }
    if (qos == NULL) {
        result = DDS_RETCODE_BAD_PARAMETER;
        SAC_REPORT(result, "DomainParticipantQos holder = NULL");
        goto err_bad_parameter;
    } else if (qos == DDS_PARTICIPANT_QOS_DEFAULT) {
        result = DDS_RETCODE_BAD_PARAMETER;
        SAC_REPORT(result, "QoS 'PARTICIPANT_QOS_DEFAULT' is read-only.");
        goto err_bad_parameter;
    }
    result = DDS_QosProviderClaimRead (_this, &qosProvider);
    if (result != DDS_RETCODE_OK) {
        goto err_bad_parameter;
    }
    qpQosProvider = qosProvider->qpQosProvider;
    if (qpQosProvider == NULL) {
        result = DDS_RETCODE_PRECONDITION_NOT_MET;
        SAC_REPORT(result, "QosProvider = NULL");
        goto err_not_found;
    }
    qpResult = cmn_qosProviderGetParticipantQos (qpQosProvider, id, &namedParticipantQos);
    result = qpResultToReturnCode (qpResult);
    if (result != DDS_RETCODE_OK) {
        goto err_not_found;
    }
    result = DDS_DomainParticipantQos_init (qos, &namedParticipantQos.domainparticipant_qos);
    if (result != DDS_RETCODE_OK) {
        goto err_init_failed;
    }
    (void)DDS_DomainParticipantQos_deinit (&namedParticipantQos.domainparticipant_qos);
err_init_failed:
    DDS_free(namedParticipantQos.name);
err_not_found:
    DDS_QosProviderRelease (_this);
err_bad_parameter:
    SAC_REPORT_FLUSH(_this, result != DDS_RETCODE_OK);
    return result;
}

DDS_ReturnCode_t
DDS_QosProvider_get_topic_qos(
    DDS_QosProvider _this,
    DDS_TopicQos *qos,
    const char *id)
{
    _QosProvider qosProvider;
    cmn_qosProvider qpQosProvider;
    cmn_qpResult qpResult;
    DDS_ReturnCode_t result = DDS_RETCODE_OK;
    DDS_NamedTopicQos namedTopicQos;
    memset (&namedTopicQos, 0, sizeof (DDS_NamedTopicQos));

    SAC_REPORT_STACK();

    if (_this == NULL) {
        result = DDS_RETCODE_BAD_PARAMETER;
        SAC_REPORT(result, "QosProvider = NULL");
        goto err_bad_parameter;
    }
    if (qos == NULL) {
        result = DDS_RETCODE_BAD_PARAMETER;
        SAC_REPORT(result, "TopicQos holder = NULL");
        goto err_bad_parameter;
    } else if (qos == DDS_TOPIC_QOS_DEFAULT) {
        result = DDS_RETCODE_BAD_PARAMETER;
        SAC_REPORT(result, "QoS 'TOPIC_QOS_DEFAULT' is read-only.");
        goto err_bad_parameter;
    }
    result = DDS_QosProviderClaimRead (_this, &qosProvider);
    if (result != DDS_RETCODE_OK) {
        goto err_bad_parameter;
    }
    qpQosProvider = qosProvider->qpQosProvider;
    if (qpQosProvider == NULL) {
        result = DDS_RETCODE_PRECONDITION_NOT_MET;
        SAC_REPORT(result, "QosProvider = NULL");
        goto err_not_found;
    }
    qpResult = cmn_qosProviderGetTopicQos (qpQosProvider, id, &namedTopicQos);
    result = qpResultToReturnCode (qpResult);
    if (result != DDS_RETCODE_OK) {
        goto err_not_found;
    }
    result = DDS_TopicQos_init (qos, &namedTopicQos.topic_qos);
    if (result != DDS_RETCODE_OK) {
        goto err_init_failed;
    }
    (void)DDS_TopicQos_deinit (&namedTopicQos.topic_qos);
err_init_failed:
    DDS_free(namedTopicQos.name);
err_not_found:
    DDS_QosProviderRelease (_this);
err_bad_parameter:
    SAC_REPORT_FLUSH(_this, result != DDS_RETCODE_OK);
    return result;
}

DDS_ReturnCode_t
DDS_QosProvider_get_subscriber_qos(
    DDS_QosProvider _this,
    DDS_SubscriberQos *qos,
    const char *id)
{
    _QosProvider qosProvider;
    cmn_qosProvider qpQosProvider;
    cmn_qpResult qpResult;
    DDS_ReturnCode_t result = DDS_RETCODE_OK;
    DDS_NamedSubscriberQos namedSubscriberQos;
    memset (&namedSubscriberQos, 0, sizeof (DDS_NamedSubscriberQos));

    SAC_REPORT_STACK();

    if (_this == NULL) {
        result = DDS_RETCODE_BAD_PARAMETER;
        SAC_REPORT(result, "QosProvider = NULL");
        goto err_bad_parameter;
    }
    if (qos == NULL) {
        result = DDS_RETCODE_BAD_PARAMETER;
        SAC_REPORT(result, "SubscriberQos holder = NULL");
        goto err_bad_parameter;
    } else if (qos == DDS_SUBSCRIBER_QOS_DEFAULT) {
        result = DDS_RETCODE_BAD_PARAMETER;
        SAC_REPORT(result, "QoS 'SUBSCRIBER_QOS_DEFAULT' is read-only.");
        goto err_bad_parameter;
    }
    result = DDS_QosProviderClaimRead (_this, &qosProvider);
    if (result != DDS_RETCODE_OK) {
        goto err_bad_parameter;
    }
    qpQosProvider = qosProvider->qpQosProvider;
    if (qpQosProvider == NULL) {
        result = DDS_RETCODE_PRECONDITION_NOT_MET;
        SAC_REPORT(result, "QosProvider = NULL");
        goto err_not_found;
    }
    qpResult = cmn_qosProviderGetSubscriberQos (qpQosProvider, id, &namedSubscriberQos);
    result = qpResultToReturnCode (qpResult);
    if (result != DDS_RETCODE_OK) {
        goto err_not_found;
    }
    result = DDS_SubscriberQos_init (qos, &namedSubscriberQos.subscriber_qos);
    if (result != DDS_RETCODE_OK) {
        goto err_init_failed;
    }
    (void)DDS_SubscriberQos_deinit (&namedSubscriberQos.subscriber_qos);
err_init_failed:
    DDS_free(namedSubscriberQos.name);
err_not_found:
    DDS_QosProviderRelease (_this);
err_bad_parameter:
    SAC_REPORT_FLUSH(_this, result != DDS_RETCODE_OK);
    return result;
}

DDS_ReturnCode_t
DDS_QosProvider_get_datareader_qos(
    DDS_QosProvider _this,
    DDS_DataReaderQos *qos,
    const char *id)
{
    _QosProvider qosProvider;
    cmn_qosProvider qpQosProvider;
    cmn_qpResult qpResult;
    DDS_ReturnCode_t result = DDS_RETCODE_OK;
    DDS_NamedDataReaderQos namedDataReaderQos;
    memset (&namedDataReaderQos, 0, sizeof (DDS_NamedDataReaderQos));

    SAC_REPORT_STACK();

    if (_this == NULL) {
        result = DDS_RETCODE_BAD_PARAMETER;
        SAC_REPORT(result, "QosProvider = NULL");
        goto err_bad_parameter;
    }
    if (qos == NULL) {
        result = DDS_RETCODE_BAD_PARAMETER;
        SAC_REPORT(result, "DataReaderQos holder = NULL");
        goto err_bad_parameter;
    } else if (qos == DDS_DATAREADER_QOS_DEFAULT) {
        result = DDS_RETCODE_BAD_PARAMETER;
        SAC_REPORT(result, "QoS 'DATAREADER_QOS_DEFAULT' is read-only.");
        goto err_bad_parameter;
    } else if (qos == DDS_DATAREADER_QOS_USE_TOPIC_QOS) {
        result = DDS_RETCODE_BAD_PARAMETER;
        SAC_REPORT(result, "QoS 'DATAREADER_QOS_USE_TOPIC_QOS' is read-only.");
        goto err_bad_parameter;
    }
    result = DDS_QosProviderClaimRead (_this, &qosProvider);
    if (result != DDS_RETCODE_OK) {
        goto err_bad_parameter;
    }
    qpQosProvider = qosProvider->qpQosProvider;
    if (qpQosProvider == NULL) {
        result = DDS_RETCODE_PRECONDITION_NOT_MET;
        SAC_REPORT(result, "QosProvider = NULL");
        goto err_not_found;
    }
    qpResult = cmn_qosProviderGetDataReaderQos (qpQosProvider, id, &namedDataReaderQos);
    result = qpResultToReturnCode (qpResult);
    if (result != DDS_RETCODE_OK) {
        goto err_not_found;
    }
    result = DDS_DataReaderQos_init (qos, &namedDataReaderQos.datareader_qos);
    if (result != DDS_RETCODE_OK) {
        goto err_init_failed;
    }
    (void)DDS_DataReaderQos_deinit (&namedDataReaderQos.datareader_qos);
err_init_failed:
    DDS_free(namedDataReaderQos.name);
err_not_found:
    DDS_QosProviderRelease (_this);
err_bad_parameter:
    SAC_REPORT_FLUSH(_this, result != DDS_RETCODE_OK);
    return result;
}

DDS_ReturnCode_t
DDS_QosProvider_get_publisher_qos(
    DDS_QosProvider _this,
    DDS_PublisherQos *qos,
    const char *id)
{
    _QosProvider qosProvider;
    cmn_qosProvider qpQosProvider;
    cmn_qpResult qpResult;
    DDS_ReturnCode_t result = DDS_RETCODE_OK;
    DDS_NamedPublisherQos namedPublisherQos;
    memset (&namedPublisherQos, 0, sizeof (DDS_NamedPublisherQos));

    SAC_REPORT_STACK();

    if (_this == NULL) {
        result = DDS_RETCODE_BAD_PARAMETER;
        SAC_REPORT(result, "QosProvider = NULL");
        goto err_bad_parameter;
    }
    if (qos == NULL) {
        result = DDS_RETCODE_BAD_PARAMETER;
        SAC_REPORT(result, "PublisherQos holder = NULL");
        goto err_bad_parameter;
    } else if (qos == DDS_PUBLISHER_QOS_DEFAULT) {
        result = DDS_RETCODE_BAD_PARAMETER;
        SAC_REPORT(result, "QoS 'PUBLISHER_QOS_DEFAULT' is read-only.");
        goto err_bad_parameter;
    }
    result = DDS_QosProviderClaimRead (_this, &qosProvider);
    if (result != DDS_RETCODE_OK) {
        goto err_bad_parameter;
    }
    qpQosProvider = qosProvider->qpQosProvider;
    if (qpQosProvider == NULL) {
        result = DDS_RETCODE_PRECONDITION_NOT_MET;
        SAC_REPORT(result, "QosProvider = NULL");
        goto err_not_found;
    }
    qpResult = cmn_qosProviderGetPublisherQos (qpQosProvider, id, &namedPublisherQos);
    result = qpResultToReturnCode (qpResult);
    if (result != DDS_RETCODE_OK) {
        goto err_not_found;
    }
    result = DDS_PublisherQos_init (qos, &namedPublisherQos.publisher_qos);
    if (result != DDS_RETCODE_OK) {
        goto err_init_failed;
    }
    (void)DDS_PublisherQos_deinit (&namedPublisherQos.publisher_qos);
err_init_failed:
    DDS_free(namedPublisherQos.name);
err_not_found:
    DDS_QosProviderRelease (_this);
err_bad_parameter:
    SAC_REPORT_FLUSH(_this, result != DDS_RETCODE_OK);
    return result;
}

DDS_ReturnCode_t
DDS_QosProvider_get_datawriter_qos(
    DDS_QosProvider _this,
    DDS_DataWriterQos *qos,
    const char *id)
{
    _QosProvider qosProvider;
    cmn_qosProvider qpQosProvider;
    cmn_qpResult qpResult;
    DDS_ReturnCode_t result = DDS_RETCODE_OK;
    DDS_NamedDataWriterQos namedDataWriterQos;
    memset (&namedDataWriterQos, 0, sizeof (DDS_NamedDataWriterQos));

    SAC_REPORT_STACK();

    if (_this == NULL) {
        result = DDS_RETCODE_BAD_PARAMETER;
        SAC_REPORT(result, "QosProvider = NULL");
        goto err_bad_parameter;
    }
    if (qos == NULL) {
        result = DDS_RETCODE_BAD_PARAMETER;
        SAC_REPORT(result, "DataWriterQos holder = NULL");
        goto err_bad_parameter;
    } else if (qos == DDS_DATAWRITER_QOS_DEFAULT) {
        result = DDS_RETCODE_BAD_PARAMETER;
        SAC_REPORT(result, "QoS 'DATAWRITER_QOS_DEFAULT' is read-only.");
        goto err_bad_parameter;
    } else if (qos == DDS_DATAWRITER_QOS_USE_TOPIC_QOS) {
        result = DDS_RETCODE_BAD_PARAMETER;
        SAC_REPORT(result, "QoS 'DATAWRTIER_QOS_USE_TOPIC_QOS' is read-only.");
        goto err_bad_parameter;
    }
    result = DDS_QosProviderClaimRead (_this, &qosProvider);
    if (result != DDS_RETCODE_OK) {
        goto err_bad_parameter;
    }
    qpQosProvider = qosProvider->qpQosProvider;
    if (qpQosProvider == NULL) {
        result = DDS_RETCODE_PRECONDITION_NOT_MET;
        SAC_REPORT(result, "QosProvider = NULL");
        goto err_not_found;
    }
    qpResult = cmn_qosProviderGetDataWriterQos (qpQosProvider, id, &namedDataWriterQos);
    result = qpResultToReturnCode (qpResult);
    if (result != DDS_RETCODE_OK) {
        goto err_not_found;
    }
    result = DDS_DataWriterQos_init (qos, &namedDataWriterQos.datawriter_qos);
    if (result != DDS_RETCODE_OK) {
        goto err_init_failed;
    }
    (void)DDS_DataWriterQos_deinit (&namedDataWriterQos.datawriter_qos);
err_init_failed:
    DDS_free(namedDataWriterQos.name);
err_not_found:
    DDS_QosProviderRelease (_this);
err_bad_parameter:
    SAC_REPORT_FLUSH(_this, result != DDS_RETCODE_OK);
    return result;
}

#define DDS_QosProvider_GetQpQosProvider()

DDS_QosProvider
DDS_QosProvider__alloc (
    const char *uri,
    const char *profile)
{
    _Object _this = NULL;
    cmn_qosProvider qpQosProvider;
    DDS_ReturnCode_t result;

    if((qpQosProvider = cmn_qosProviderNew (uri, profile, &qpQosProviderAttr)) == NULL){
        /* Error already reported by cmn_qosProviderNew(...) */
        goto err_cmn_qosProviderNew;
    }

    if((result = DDS_Object_new (DDS_QOSPROVIDER, _QosProvider_deinit, &_this)) != DDS_RETCODE_OK) {
        /* TODO: Report out-of-memory. */
        goto err_DDS_Object_new;
    }

    _QosProvider(_this)->qpQosProvider = qpQosProvider;

    return _this;

/* Error handling */
err_DDS_Object_new:
    cmn_qosProviderFree(qpQosProvider);
err_cmn_qosProviderNew:
    return NULL;
}
