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
#include <stdlib.h>
#include <string.h>

#include "ccpp_dds_dcps.h"
#include "DomainParticipantFactory.h"
#include "QosProvider.h"
#include "QosUtils.h"
#include "ReportUtils.h"
#include "cmn_qosProvider.h"

static DDS::ReturnCode_t
qpResultToReturnCode (cmn_qpResult qpResult)
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

/* The below qosProviderAttr uses the defaults supplied by the qosProvider
 * implementation. If a C++ QoS would have to be used, the idlpp-generated
 * copyIn-function with static type-caching disabled should be used. By
 * default these are named __DDS_NamedDomainParticipantQos__copyIn_noCache,
 * etc. */
static const C_STRUCT(cmn_qosProviderInputAttr) qosProviderAttr = {
    { /* ParticipantQos */
        (cmn_qpCopyOut)&__DDS_NamedDomainParticipantQos__copyOut
    },
    { /* TopicQos */
        (cmn_qpCopyOut)&__DDS_NamedTopicQos__copyOut
    },
    { /* SubscriberQos */
        (cmn_qpCopyOut)&__DDS_NamedSubscriberQos__copyOut
    },
    { /* DataReaderQos */
        (cmn_qpCopyOut)&__DDS_NamedDataReaderQos__copyOut
    },
    { /* PublisherQos */
        (cmn_qpCopyOut)&__DDS_NamedPublisherQos__copyOut
    },
    { /* DataWriterQos */
        (cmn_qpCopyOut)&__DDS_NamedDataWriterQos__copyOut
    }
};

DDS::QosProvider::QosProvider (
    const char *uri,
    const char *profile = NULL) :
/*    DDS::OpenSplice::CppSuperClass(DDS::OpenSplice::QOSPROVIDER), */
    qosProvider(NULL)
{
    assert (uri != NULL);

    this->qosProvider = cmn_qosProviderNew (
        uri, profile, &qosProviderAttr);
}

DDS::QosProvider::~QosProvider()
{
    this->deinit();
    if (this->qosProvider != NULL) {
        cmn_qosProviderFree (this->qosProvider);
        this->qosProvider = NULL;
    }
}

DDS::ReturnCode_t
DDS::QosProvider::is_ready ()
{
    DDS::ReturnCode_t result = DDS::RETCODE_OK;

    if (this->qosProvider == NULL) {
        result = DDS::RETCODE_ERROR;
        CPP_REPORT(result, "QosProvider is not initialized.");
    }

    return result;
}

DDS::ReturnCode_t
DDS::QosProvider::get_participant_qos (
    DDS::DomainParticipantQos &participantQos,
    const char *id = NULL)
{
    cmn_qpResult qpResult;
    DDS::ReturnCode_t result;
    DDS::NamedDomainParticipantQos namedParticipantQos;

    CPP_REPORT_STACK();

    result = this->is_ready ();

    if (result == DDS::RETCODE_OK) {
        if (&participantQos == &PARTICIPANT_QOS_DEFAULT) {
            result = DDS::RETCODE_BAD_PARAMETER;
            CPP_REPORT(result, "QoS 'PARTICIPANT_QOS_DEFAULT' is read-only.");
        } else {
            qpResult = cmn_qosProviderGetParticipantQos (
                this->qosProvider, id, &namedParticipantQos);
            result = qpResultToReturnCode (qpResult);
            if (result == DDS::RETCODE_OK) {
                participantQos = namedParticipantQos.domainparticipant_qos;
            } else {
                CPP_REPORT(result, "Could not copy DomainParticipantQos.");
            }
        }
    }

    CPP_REPORT_FLUSH_NO_ID(result != DDS::RETCODE_OK);

    return result;
}

DDS::ReturnCode_t
DDS::QosProvider::get_topic_qos (
    DDS::TopicQos &topicQos,
    const char *id = NULL)
{
    cmn_qpResult qpResult;
    DDS::ReturnCode_t result;
    DDS::NamedTopicQos namedTopicQos;

    CPP_REPORT_STACK();

    result = this->is_ready ();

    if (result == DDS::RETCODE_OK) {
        if (&topicQos == &TOPIC_QOS_DEFAULT) {
            result = DDS::RETCODE_BAD_PARAMETER;
            CPP_REPORT(result, "QoS 'TOPIC_QOS_DEFAULT' is read-only.");
        } else {
            qpResult = cmn_qosProviderGetTopicQos (
                this->qosProvider, id, &namedTopicQos);
            result = qpResultToReturnCode (qpResult);
            if (result == DDS::RETCODE_OK) {
                topicQos = namedTopicQos.topic_qos;
            } else {
                CPP_REPORT(result, "Could not copy TopicQos.");
            }
        }
    }

    CPP_REPORT_FLUSH_NO_ID(result != DDS::RETCODE_OK);

    return result;
}

DDS::ReturnCode_t
DDS::QosProvider::get_subscriber_qos (
    DDS::SubscriberQos &subscriberQos,
    const char *id = NULL)
{
    cmn_qpResult qpResult;
    DDS::ReturnCode_t result;
    DDS::NamedSubscriberQos namedSubscriberQos;

    CPP_REPORT_STACK();

    result = this->is_ready ();

    if (result == DDS::RETCODE_OK) {
        if (&subscriberQos == &SUBSCRIBER_QOS_DEFAULT) {
            result = DDS::RETCODE_BAD_PARAMETER;
            CPP_REPORT(result, "QoS 'SUBSCRIBER_QOS_DEFAULT' is read-only.");
        } else {
            qpResult = cmn_qosProviderGetSubscriberQos (
                this->qosProvider, id, &namedSubscriberQos);
            result = qpResultToReturnCode (qpResult);
            if (result == DDS::RETCODE_OK) {
                subscriberQos = namedSubscriberQos.subscriber_qos;
            } else {
                CPP_REPORT(result, "Could not copy SubscriberQos.");
            }
        }
    }

    CPP_REPORT_FLUSH_NO_ID(result != DDS::RETCODE_OK);

    return result;
}

DDS::ReturnCode_t
DDS::QosProvider::get_datareader_qos (
    DDS::DataReaderQos &dataReaderQos,
    const char *id = NULL)
{
    cmn_qpResult qpResult;
    DDS::ReturnCode_t result;
    DDS::NamedDataReaderQos namedDataReaderQos;

    CPP_REPORT_STACK();

    result = this->is_ready ();

    if (result == DDS::RETCODE_OK) {
        if (&dataReaderQos == &DATAREADER_QOS_DEFAULT) {
            result = DDS::RETCODE_BAD_PARAMETER;
            CPP_REPORT(result, "QoS 'DATAREADER_QOS_DEFAULT' is read-only.");
        } else if (&dataReaderQos == &DATAREADER_QOS_USE_TOPIC_QOS) {
            result = DDS::RETCODE_BAD_PARAMETER;
            CPP_REPORT(result, "QoS 'DATAREADER_QOS_USE_TOPIC_QOS' is read-only.");
        } else {
            qpResult = cmn_qosProviderGetDataReaderQos (
                this->qosProvider, id, &namedDataReaderQos);
            result = qpResultToReturnCode (qpResult);
            if (result == DDS::RETCODE_OK) {
                dataReaderQos = namedDataReaderQos.datareader_qos;
            } else {
                CPP_REPORT(result, "Could not copy DataReaderQos.");
            }
        }
    }

    CPP_REPORT_FLUSH_NO_ID(result != DDS::RETCODE_OK);

    return result;
}

DDS::ReturnCode_t
DDS::QosProvider::get_publisher_qos (
    DDS::PublisherQos &publisherQos,
    const char *id = NULL)
{
    cmn_qpResult qpResult;
    DDS::ReturnCode_t result;
    DDS::NamedPublisherQos namedPublisherQos;

    CPP_REPORT_STACK();

    result = this->is_ready ();

    if (result == DDS::RETCODE_OK) {
        if (&publisherQos == &PUBLISHER_QOS_DEFAULT) {
            result = DDS::RETCODE_BAD_PARAMETER;
            CPP_REPORT(result, "QoS 'PUBLISHER_QOS_DEFAULT' is read-only.");
        } else {
            qpResult = cmn_qosProviderGetPublisherQos (
                this->qosProvider, id, &namedPublisherQos);
            result = qpResultToReturnCode (qpResult);
            if (result == DDS::RETCODE_OK) {
                publisherQos = namedPublisherQos.publisher_qos;
            } else {
                CPP_REPORT(result, "Could not copy PublisherQos.");
            }
        }
    }

    CPP_REPORT_FLUSH_NO_ID(result != DDS::RETCODE_OK);

    return result;
}

DDS::ReturnCode_t
DDS::QosProvider::get_datawriter_qos (
    DDS::DataWriterQos &dataWriterQos,
    const char *id = NULL)
{
    cmn_qpResult qpResult;
    DDS::ReturnCode_t result;
    DDS::NamedDataWriterQos namedDataWriterQos;

    CPP_REPORT_STACK();

    result = this->is_ready ();

    if (result == DDS::RETCODE_OK) {
        if (&dataWriterQos == &DATAWRITER_QOS_DEFAULT) {
            result = DDS::RETCODE_BAD_PARAMETER;
            CPP_REPORT(result, "QoS 'DATAWRITER_QOS_DEFAULT' is read-only.");
        } else if (&dataWriterQos == &DATAWRITER_QOS_USE_TOPIC_QOS) {
            result = DDS::RETCODE_BAD_PARAMETER;
            CPP_REPORT(result, "QoS 'DATAWRITER_QOS_USE_TOPIC_QOS' is read-only.");
        } else {
            qpResult = cmn_qosProviderGetDataWriterQos (
                this->qosProvider, id, &namedDataWriterQos);
            result = qpResultToReturnCode (qpResult);
            if (result == DDS::RETCODE_OK) {
                dataWriterQos = namedDataWriterQos.datawriter_qos;
            } else {
                CPP_REPORT(result, "Could not copy DataWriterQos.");
            }
        }
    }

    CPP_REPORT_FLUSH_NO_ID(result != DDS::RETCODE_OK);

    return result;
}

DDS::ReturnCode_t
DDS::QosProvider::deinit(
    void
) THROW_ORB_EXCEPTIONS
{
    DDS::ReturnCode_t result = DDS::RETCODE_OK;
/*
    result = this->write_lock();
    if (result == DDS::RETCODE_OK) {
        result = DDS::OpenSplice::CppSuperClass::wlReq_deinit();
        this->unlock();
    }*/
    return result;
}
