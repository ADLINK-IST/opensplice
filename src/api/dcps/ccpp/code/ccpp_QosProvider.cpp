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
#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "ccpp_dds_dcps.h"
#include "ccpp_DomainParticipantFactory.h"
#include "ccpp_QosProvider.h"
#include "ccpp_QosUtils.h"
#include "qp_qosProvider.h"
#include "gapi.h"
#include "os_report.h"

static DDS::ReturnCode_t
qpResultToReturnCode (qp_result qpResult)
{
    DDS::ReturnCode_t result = DDS::RETCODE_ERROR;

    switch (qpResult) {
        case QP_RESULT_OK:
            result = DDS::RETCODE_OK;
            break;
        case QP_RESULT_NO_DATA:
            result = DDS::RETCODE_NO_DATA;
            break;
        case QP_RESULT_OUT_OF_MEMORY:
            result = DDS::RETCODE_OUT_OF_RESOURCES;
            break;
        case QP_RESULT_ILL_PARAM:
            result = DDS::RETCODE_BAD_PARAMETER;
            break;
        default:
            result = DDS::RETCODE_ERROR;
            break;
    }

    return result;
}

DDS::QosProvider::QosProvider (
    const char *uri,
    const char *profile = NULL)
 :  qosProvider(NULL)
{
    assert (uri != NULL);

    this->qosProvider = qp_qosProviderNew (
        uri, profile, DDS::ccpp_getQosProviderInputAttr());
}

DDS::QosProvider::~QosProvider()
{
    if (this->qosProvider != NULL) {
        qp_qosProviderFree (this->qosProvider);
        this->qosProvider = NULL;
    }
}

DDS::ReturnCode_t
DDS::QosProvider::is_ready ()
{
    DDS::ReturnCode_t result = DDS::RETCODE_OK;

    if (this->qosProvider == NULL) {
        OS_REPORT (OS_ERROR, "DDS::QosProvider::is_ready", 0,
            "QosProvider not properly instantiated");
        result = DDS::RETCODE_ERROR;
    }

    return result;
}

DDS::ReturnCode_t
DDS::QosProvider::get_participant_qos (
    DDS::DomainParticipantQos &participantQos,
    const char *id = NULL)
{
    qp_result qpResult;
    DDS::ReturnCode_t result;
    DDS::NamedDomainParticipantQos namedParticipantQos;

    result = this->is_ready ();

    if (result == DDS::RETCODE_OK) {
        if (&participantQos == &PARTICIPANT_QOS_DEFAULT) {
            result = DDS::RETCODE_BAD_PARAMETER;
        } else {
            qpResult = qp_qosProviderGetParticipantQos (
                this->qosProvider, id, &namedParticipantQos);
            result = qpResultToReturnCode (qpResult);
            if (result == DDS::RETCODE_OK) {
                participantQos = namedParticipantQos.domainparticipant_qos;
            }
        }
    }

    return result;
}

DDS::ReturnCode_t
DDS::QosProvider::get_topic_qos (
    DDS::TopicQos &topicQos,
    const char *id = NULL)
{
    qp_result qpResult;
    DDS::ReturnCode_t result;
    DDS::NamedTopicQos namedTopicQos;

    result = this->is_ready ();

    if (result == DDS::RETCODE_OK) {
        if (&topicQos == &TOPIC_QOS_DEFAULT) {
            result = DDS::RETCODE_BAD_PARAMETER;
        } else {
            qpResult = qp_qosProviderGetTopicQos (
                this->qosProvider, id, &namedTopicQos);
            result = qpResultToReturnCode (qpResult);
            if (result == DDS::RETCODE_OK) {
                topicQos = namedTopicQos.topic_qos;
            }
        }
    }

    return result;
}

DDS::ReturnCode_t
DDS::QosProvider::get_subscriber_qos (
    DDS::SubscriberQos &subscriberQos,
    const char *id = NULL)
{
    qp_result qpResult;
    DDS::ReturnCode_t result;
    DDS::NamedSubscriberQos namedSubscriberQos;

    result = this->is_ready ();

    if (result == DDS::RETCODE_OK) {
        if (&subscriberQos == &SUBSCRIBER_QOS_DEFAULT) {
            result = DDS::RETCODE_BAD_PARAMETER;
        } else {
            qpResult = qp_qosProviderGetSubscriberQos (
                this->qosProvider, id, &namedSubscriberQos);
            result = qpResultToReturnCode (qpResult);
            if (result == DDS::RETCODE_OK) {
                subscriberQos = namedSubscriberQos.subscriber_qos;
            }
        }
    }

    return result;
}

DDS::ReturnCode_t
DDS::QosProvider::get_datareader_qos (
    DDS::DataReaderQos &dataReaderQos,
    const char *id = NULL)
{
    qp_result qpResult;
    DDS::ReturnCode_t result;
    DDS::NamedDataReaderQos namedDataReaderQos;

    result = this->is_ready ();

    if (result == DDS::RETCODE_OK) {
        if (&dataReaderQos == &DATAREADER_QOS_DEFAULT ||
            &dataReaderQos == &DATAREADER_QOS_USE_TOPIC_QOS)
        {
            result = DDS::RETCODE_BAD_PARAMETER;
        } else {
            qpResult = qp_qosProviderGetDataReaderQos (
                this->qosProvider, id, &namedDataReaderQos);
            result = qpResultToReturnCode (qpResult);
            if (result == DDS::RETCODE_OK) {
                dataReaderQos = namedDataReaderQos.datareader_qos;
            }
        }
    }

    return result;
}

DDS::ReturnCode_t
DDS::QosProvider::get_publisher_qos (
    DDS::PublisherQos &publisherQos,
    const char *id = NULL)
{
    qp_result qpResult;
    DDS::ReturnCode_t result;
    DDS::NamedPublisherQos namedPublisherQos;

    result = this->is_ready ();

    if (result == DDS::RETCODE_OK) {
        if (&publisherQos == &PUBLISHER_QOS_DEFAULT) {
            result = DDS::RETCODE_BAD_PARAMETER;
        } else {
            qpResult = qp_qosProviderGetPublisherQos (
                this->qosProvider, id, &namedPublisherQos);
            result = qpResultToReturnCode (qpResult);
            if (result == DDS::RETCODE_OK) {
                publisherQos = namedPublisherQos.publisher_qos;
            }
        }
    }

    return result;
}

DDS::ReturnCode_t
DDS::QosProvider::get_datawriter_qos (
    DDS::DataWriterQos &dataWriterQos,
    const char *id = NULL)
{
    qp_result qpResult;
    DDS::ReturnCode_t result;
    DDS::NamedDataWriterQos namedDataWriterQos;

    result = this->is_ready ();

    if (result == DDS::RETCODE_OK) {
        if (&dataWriterQos == &DATAWRITER_QOS_DEFAULT ||
            &dataWriterQos == &DATAWRITER_QOS_USE_TOPIC_QOS)
        {
            result = DDS::RETCODE_BAD_PARAMETER;
        } else {
            qpResult = qp_qosProviderGetDataWriterQos (
                this->qosProvider, id, &namedDataWriterQos);
            result = qpResultToReturnCode (qpResult);
            if (result == DDS::RETCODE_OK) {
                dataWriterQos = namedDataWriterQos.datawriter_qos;
            }
        }
    }

    return result;
}
