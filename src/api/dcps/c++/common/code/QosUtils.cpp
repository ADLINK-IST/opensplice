/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR PrismTech
 *   Limited, its affiliated companies and licensors. All rights reserved.
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
#include "Constants.h"
#include "PolicyUtils.h"
#include "QosUtils.h"
#include "MiscUtils.h"
#include "SequenceUtils.h"
#include "ReportUtils.h"

/*
 * Default QoSses
 */

namespace DDS {
namespace OpenSplice {
namespace Utils {

static const DataReaderQos *
    initializeDataReaderQos()
    {
        DataReaderQos *qos = new DataReaderQos();
        qos->durability = DDS::OpenSplice::Utils::DurabilityQosPolicy_default;
        qos->deadline = DDS::OpenSplice::Utils::DeadlineQosPolicy_default;
        qos->latency_budget = DDS::OpenSplice::Utils::LatencyBudgetQosPolicy_default;
        qos->liveliness = DDS::OpenSplice::Utils::LivelinessQosPolicy_default;
        qos->reliability = DDS::OpenSplice::Utils::ReliabilityQosPolicy_default;
        qos->destination_order = DDS::OpenSplice::Utils::DestinationOrderQosPolicy_default;
        qos->history = DDS::OpenSplice::Utils::HistoryQosPolicy_default;
        qos->resource_limits = DDS::OpenSplice::Utils::ResourceLimitsQosPolicy_default;
        qos->user_data = DDS::OpenSplice::Utils::UserDataQosPolicy_default;
        qos->ownership = DDS::OpenSplice::Utils::OwnershipQosPolicy_default;
        qos->time_based_filter = DDS::OpenSplice::Utils::TimeBasedFilterQosPolicy_default;
        qos->reader_data_lifecycle = DDS::OpenSplice::Utils::ReaderDataLifecycleQosPolicy_default;
        qos->subscription_keys = DDS::OpenSplice::Utils::SubscriptionKeyQosPolicy_default;
        qos->reader_lifespan = DDS::OpenSplice::Utils::ReaderLifespanQosPolicy_default;
        qos->share = DDS::OpenSplice::Utils::ShareQosPolicy_default;
        return qos;
    }

static const DataReaderViewQos *
initializeDataReaderViewQos()
{
    DataReaderViewQos *qos = new DataReaderViewQos();
    qos->view_keys = DDS::OpenSplice::Utils::ViewKeyQosPolicy_default;
    return qos;
}

static const DataWriterQos *
    initializeDataWriterQos()
    {
        DataWriterQos *qos          = new DataWriterQos();
        qos->durability = DDS::OpenSplice::Utils::DurabilityQosPolicy_default;
        qos->deadline = DDS::OpenSplice::Utils::DeadlineQosPolicy_default;
        qos->latency_budget = DDS::OpenSplice::Utils::LatencyBudgetQosPolicy_default;
        qos->liveliness = DDS::OpenSplice::Utils::LivelinessQosPolicy_default;
        qos->reliability = DDS::OpenSplice::Utils::ReliabilityQosPolicy_writerDefault;
        qos->destination_order = DDS::OpenSplice::Utils::DestinationOrderQosPolicy_default;
        qos->history = DDS::OpenSplice::Utils::HistoryQosPolicy_default;
        qos->resource_limits = DDS::OpenSplice::Utils::ResourceLimitsQosPolicy_default;
        qos->transport_priority = DDS::OpenSplice::Utils::TransportPriorityQosPolicy_default;
        qos->lifespan = DDS::OpenSplice::Utils::LifespanQosPolicy_default;
        qos->user_data = DDS::OpenSplice::Utils::UserDataQosPolicy_default;
        qos->ownership = DDS::OpenSplice::Utils::OwnershipQosPolicy_default;
        qos->ownership_strength = DDS::OpenSplice::Utils::OwnershipStrengthQosPolicy_default;
        qos->writer_data_lifecycle = DDS::OpenSplice::Utils::WriterDataLifecycleQosPolicy_default;
        return qos;
    }

static const DomainParticipantFactoryQos *
    initializeDomainParticipantFactoryQos()
    {
        DomainParticipantFactoryQos *qos          = new DomainParticipantFactoryQos();
        qos->entity_factory = DDS::OpenSplice::Utils::EntityFactoryQosPolicy_default;
        return qos;
    }

static const DomainParticipantQos *
    initializeDomainParticipantQos()
    {
        DomainParticipantQos *qos          = new DomainParticipantQos();
        qos->user_data = DDS::OpenSplice::Utils::UserDataQosPolicy_default;
        qos->entity_factory = DDS::OpenSplice::Utils::EntityFactoryQosPolicy_default;
        qos->watchdog_scheduling = DDS::OpenSplice::Utils::SchedulingQosPolicy_default;
        qos->listener_scheduling = DDS::OpenSplice::Utils::SchedulingQosPolicy_default;
        return qos;
    }

static const PublisherQos *
    initializePublisherQos()
    {
        PublisherQos *qos          = new PublisherQos();
        qos->presentation = DDS::OpenSplice::Utils::PresentationQosPolicy_default;
        qos->partition = DDS::OpenSplice::Utils::PartitionQosPolicy_default;
        qos->group_data = DDS::OpenSplice::Utils::GroupDataQosPolicy_default;
        qos->entity_factory = DDS::OpenSplice::Utils::EntityFactoryQosPolicy_default;
        return qos;
    }

static const SubscriberQos *
    initializeSubscriberQos()
    {
        SubscriberQos *qos          = new SubscriberQos();
        qos->presentation = DDS::OpenSplice::Utils::PresentationQosPolicy_default;
        qos->partition = DDS::OpenSplice::Utils::PartitionQosPolicy_default;
        qos->group_data = DDS::OpenSplice::Utils::GroupDataQosPolicy_default;
        qos->entity_factory = DDS::OpenSplice::Utils::EntityFactoryQosPolicy_default;
        qos->share = DDS::OpenSplice::Utils::ShareQosPolicy_default;
        return qos;
    }

static const DDS::TopicQos *
    initializeTopicQos()
    {
        DDS::TopicQos *qos          = new DDS::TopicQos();
        qos->topic_data             = DDS::OpenSplice::Utils::TopicDataQosPolicy_default;
        qos->durability             = DDS::OpenSplice::Utils::DurabilityQosPolicy_default;
        qos->durability_service     = DDS::OpenSplice::Utils::DurabilityServiceQosPolicy_default;
        qos->deadline               = DDS::OpenSplice::Utils::DeadlineQosPolicy_default;
        qos->latency_budget         = DDS::OpenSplice::Utils::LatencyBudgetQosPolicy_default;
        qos->liveliness             = DDS::OpenSplice::Utils::LivelinessQosPolicy_default;
        qos->reliability            = DDS::OpenSplice::Utils::ReliabilityQosPolicy_default;
        qos->destination_order      = DDS::OpenSplice::Utils::DestinationOrderQosPolicy_default;
        qos->history                = DDS::OpenSplice::Utils::HistoryQosPolicy_default;
        qos->resource_limits        = DDS::OpenSplice::Utils::ResourceLimitsQosPolicy_default;
        qos->transport_priority     = DDS::OpenSplice::Utils::TransportPriorityQosPolicy_default;
        qos->lifespan               = DDS::OpenSplice::Utils::LifespanQosPolicy_default;
        qos->ownership              = DDS::OpenSplice::Utils::OwnershipQosPolicy_default;
        return qos;
    }

const DDS::DataReaderQos *
FactoryDefaultQosHolder::get_dataReaderQos_default()
{
    static const DDS::DataReaderQos *DataReaderQos_default = DDS::OpenSplice::Utils::initializeDataReaderQos();
    return DataReaderQos_default;
}

const DDS::DataReaderQos *
FactoryDefaultQosHolder::get_dataReaderQos_use_topic()
{
    static const DDS::DataReaderQos *DataReaderQos_use_topic = DDS::OpenSplice::Utils::initializeDataReaderQos();
    return DataReaderQos_use_topic;
}

const DDS::DataReaderViewQos *
FactoryDefaultQosHolder::get_dataReaderViewQos_default()
{
    static const DDS::DataReaderViewQos *DataReaderViewQos_default = DDS::OpenSplice::Utils::initializeDataReaderViewQos();
    return DataReaderViewQos_default;
}

const DDS::DataWriterQos *
FactoryDefaultQosHolder::get_dataWriterQos_default()
{
    static const DDS::DataWriterQos *DataWriterQos_default = DDS::OpenSplice::Utils::initializeDataWriterQos();
    return DataWriterQos_default;
}

const DDS::DataWriterQos *
FactoryDefaultQosHolder::get_dataWriterQos_use_topic()
{
    static const DDS::DataWriterQos *DataWriterQos_use_topic = DDS::OpenSplice::Utils::initializeDataWriterQos();
    return DataWriterQos_use_topic;
}

const DDS::DomainParticipantFactoryQos *
FactoryDefaultQosHolder::get_domainParticipantFactoryQos_default()
{
    static const DDS::DomainParticipantFactoryQos *DomainParticipantFactoryQos_default =
            DDS::OpenSplice::Utils::initializeDomainParticipantFactoryQos();
    return DomainParticipantFactoryQos_default;
}

const DDS::DomainParticipantQos *
FactoryDefaultQosHolder::get_domainParticipantQos_default()
{
    static const DDS::DomainParticipantQos *DomainParticipantQos_default = DDS::OpenSplice::Utils::initializeDomainParticipantQos();
    return DomainParticipantQos_default;
}

const DDS::PublisherQos *
FactoryDefaultQosHolder::get_publisherQos_default()
{
    static const DDS::PublisherQos *PublisherQos_default = DDS::OpenSplice::Utils::initializePublisherQos();
    return PublisherQos_default;
}

const DDS::SubscriberQos *
FactoryDefaultQosHolder::get_subscriberQos_default()
{
    static const DDS::SubscriberQos *SubscriberQos_default = DDS::OpenSplice::Utils::initializeSubscriberQos();
    return SubscriberQos_default;
}

const DDS::TopicQos *
FactoryDefaultQosHolder::get_topicQos_default()
{
    static const DDS::TopicQos *TopicQos_default = DDS::OpenSplice::Utils::initializeTopicQos();
    return TopicQos_default;
}

} /* end namespace Utils */
} /* end namespace OpenSplice */
} /* end namespace DDS */


/*
 * Qos validations
 */

DDS::ReturnCode_t
DDS::OpenSplice::Utils::qosIsConsistent(
    const DDS::DataReaderQos &qos)
{
    DDS::ReturnCode_t result = DDS::RETCODE_OK;

    if (&qos != &DATAREADER_QOS_DEFAULT &&
        &qos != &DATAREADER_QOS_USE_TOPIC_QOS)
    {
        if ((result = DDS::OpenSplice::Utils::policyIsValid(
                 qos.durability))                   == DDS::RETCODE_OK
         && (result = DDS::OpenSplice::Utils::policyIsValid(
                 qos.deadline))                     == DDS::RETCODE_OK
         && (result = DDS::OpenSplice::Utils::policyIsValid(
                 qos.latency_budget))               == DDS::RETCODE_OK
         && (result = DDS::OpenSplice::Utils::policyIsValid(
                 qos.liveliness))                   == DDS::RETCODE_OK
         && (result = DDS::OpenSplice::Utils::policyIsValid(
                 qos.reliability))                  == DDS::RETCODE_OK
         && (result = DDS::OpenSplice::Utils::policyIsValid(
                 qos.destination_order))            == DDS::RETCODE_OK
         && (result = DDS::OpenSplice::Utils::policyIsValid(
                 qos.history))                      == DDS::RETCODE_OK
         && (result = DDS::OpenSplice::Utils::policyIsValid(
                 qos.resource_limits))              == DDS::RETCODE_OK
         && (result = DDS::OpenSplice::Utils::policyIsValid(
                 qos.user_data))                    == DDS::RETCODE_OK
         && (result = DDS::OpenSplice::Utils::policyIsValid(
                 qos.time_based_filter))            == DDS::RETCODE_OK
         && (result = DDS::OpenSplice::Utils::policyIsValid(
                 qos.ownership))                    == DDS::RETCODE_OK
         && (result = DDS::OpenSplice::Utils::policyIsValid(
                 qos.reader_data_lifecycle))        == DDS::RETCODE_OK
         && (result = DDS::OpenSplice::Utils::policyIsValid(
                 qos.subscription_keys))            == DDS::RETCODE_OK
         && (result = DDS::OpenSplice::Utils::policyIsValid(
                 qos.reader_lifespan))              == DDS::RETCODE_OK
         && (result = DDS::OpenSplice::Utils::policyIsValid(
                 qos.share))                        == DDS::RETCODE_OK
         && (result = DDS::OpenSplice::Utils::policiesAreConsistent(
                 qos.history, qos.resource_limits)) == DDS::RETCODE_OK)
        {
            result = DDS::OpenSplice::Utils::policiesAreConsistent(
                qos.deadline, qos.time_based_filter);
        }
    }

    return result;
}

DDS::ReturnCode_t
DDS::OpenSplice::Utils::qosIsConsistent(
    const DDS::DataReaderViewQos &qos)
{
    DDS::ReturnCode_t result = DDS::RETCODE_OK;

    if (&qos != &DATAREADERVIEW_QOS_DEFAULT) {
        result = DDS::OpenSplice::Utils::policyIsValid(qos.view_keys);
    }

    return result;
}

DDS::ReturnCode_t
DDS::OpenSplice::Utils::qosIsConsistent(
        const DDS::DataWriterQos &qos)
{
    DDS::ReturnCode_t result = DDS::RETCODE_OK;

    if (&qos != &DATAWRITER_QOS_DEFAULT &&
        &qos != &DATAWRITER_QOS_USE_TOPIC_QOS)
    {
        if ((result = DDS::OpenSplice::Utils::policyIsValid(
                 qos.durability))            == DDS::RETCODE_OK
         && (result = DDS::OpenSplice::Utils::policyIsValid(
                 qos.deadline))              == DDS::RETCODE_OK
         && (result = DDS::OpenSplice::Utils::policyIsValid(
                 qos.latency_budget))        == DDS::RETCODE_OK
         && (result = DDS::OpenSplice::Utils::policyIsValid(
                 qos.liveliness))            == DDS::RETCODE_OK
         && (result = DDS::OpenSplice::Utils::policyIsValid(
                 qos.reliability))           == DDS::RETCODE_OK
         && (result = DDS::OpenSplice::Utils::policyIsValid(
                 qos.destination_order))     == DDS::RETCODE_OK
         && (result = DDS::OpenSplice::Utils::policyIsValid(
                 qos.history))               == DDS::RETCODE_OK
         && (result = DDS::OpenSplice::Utils::policyIsValid(
                 qos.resource_limits))       == DDS::RETCODE_OK
         && (result = DDS::OpenSplice::Utils::policyIsValid(
                 qos.transport_priority))    == DDS::RETCODE_OK
         && (result = DDS::OpenSplice::Utils::policyIsValid(
                 qos.lifespan))              == DDS::RETCODE_OK
         && (result = DDS::OpenSplice::Utils::policyIsValid(
                 qos.user_data))             == DDS::RETCODE_OK
         && (result = DDS::OpenSplice::Utils::policyIsValid(
                 qos.ownership))             == DDS::RETCODE_OK
         && (result = DDS::OpenSplice::Utils::policyIsValid(
                 qos.ownership_strength))    == DDS::RETCODE_OK
         && (result = DDS::OpenSplice::Utils::policyIsValid(
                 qos.writer_data_lifecycle)) == DDS::RETCODE_OK)
        {
            result = DDS::OpenSplice::Utils::policiesAreConsistent(
                qos.history, qos.resource_limits);
        }
    }

    return result;
}

DDS::ReturnCode_t
DDS::OpenSplice::Utils::qosIsConsistent(
        const DDS::DomainParticipantFactoryQos &qos)
{
    DDS::ReturnCode_t result = DDS::RETCODE_OK;

    if (&qos != &PARTICIPANTFACTORY_QOS_DEFAULT) {
        result = DDS::OpenSplice::Utils::policyIsValid(qos.entity_factory);
    }

    return result;
}

DDS::ReturnCode_t
DDS::OpenSplice::Utils::qosIsConsistent(
    const DDS::DomainParticipantQos &qos)
{
    DDS::ReturnCode_t result = DDS::RETCODE_OK;

    if (&qos != &PARTICIPANT_QOS_DEFAULT) {
        if ((result = DDS::OpenSplice::Utils::policyIsValid(
                 qos.user_data))           == DDS::RETCODE_OK
         && (result = DDS::OpenSplice::Utils::policyIsValid(
                 qos.entity_factory))      == DDS::RETCODE_OK
         && (result = DDS::OpenSplice::Utils::policyIsValid(
                 qos.watchdog_scheduling)) == DDS::RETCODE_OK)
        {
            result = DDS::OpenSplice::Utils::policyIsValid(
                qos.listener_scheduling);
        }
    }

    return result;
}

DDS::ReturnCode_t
DDS::OpenSplice::Utils::qosIsConsistent(
    const DDS::PublisherQos &qos)
{
    DDS::ReturnCode_t result = DDS::RETCODE_OK;

    if (&qos != &(PUBLISHER_QOS_DEFAULT)) {
        if ((result = DDS::OpenSplice::Utils::policyIsValid(
                 qos.presentation))   == DDS::RETCODE_OK
         && (result = DDS::OpenSplice::Utils::policyIsValid(
                 qos.partition))      == DDS::RETCODE_OK
         && (result = DDS::OpenSplice::Utils::policyIsValid(
                 qos.group_data))     == DDS::RETCODE_OK)
        {
            result = DDS::OpenSplice::Utils::policyIsValid(qos.entity_factory);
        }
    }

    return result;
}

DDS::ReturnCode_t
DDS::OpenSplice::Utils::qosIsConsistent(
    const DDS::SubscriberQos &qos)
{
    DDS::ReturnCode_t result = DDS::RETCODE_OK;

    if (&qos != &SUBSCRIBER_QOS_DEFAULT) {
        if ((result = DDS::OpenSplice::Utils::policyIsValid(
                 qos.presentation))   == DDS::RETCODE_OK
         && (result = DDS::OpenSplice::Utils::policyIsValid(
                 qos.partition))      == DDS::RETCODE_OK
         && (result = DDS::OpenSplice::Utils::policyIsValid(
                 qos.group_data))     == DDS::RETCODE_OK
         && (result = DDS::OpenSplice::Utils::policyIsValid(
                 qos.entity_factory)) == DDS::RETCODE_OK)
        {
            result = DDS::OpenSplice::Utils::policyIsValid(qos.share);
        }
    }

    return result;
}

DDS::ReturnCode_t
DDS::OpenSplice::Utils::qosIsConsistent(
    const DDS::TopicQos &qos)
{
    DDS::ReturnCode_t result = DDS::RETCODE_OK;

    if (&qos != &TOPIC_QOS_DEFAULT) {
        if ((result = DDS::OpenSplice::Utils::policyIsValid(
                 qos.topic_data))         == DDS::RETCODE_OK
         && (result = DDS::OpenSplice::Utils::policyIsValid(
                 qos.durability))         == DDS::RETCODE_OK
         && (result = DDS::OpenSplice::Utils::policyIsValid(
                 qos.durability_service)) == DDS::RETCODE_OK
         && (result = DDS::OpenSplice::Utils::policyIsValid(
                 qos.deadline))           == DDS::RETCODE_OK
         && (result = DDS::OpenSplice::Utils::policyIsValid(
                 qos.latency_budget))     == DDS::RETCODE_OK
         && (result = DDS::OpenSplice::Utils::policyIsValid(
                 qos.liveliness))         == DDS::RETCODE_OK
         && (result = DDS::OpenSplice::Utils::policyIsValid(
                 qos.reliability))        == DDS::RETCODE_OK
         && (result = DDS::OpenSplice::Utils::policyIsValid(
                 qos.destination_order))  == DDS::RETCODE_OK
         && (result = DDS::OpenSplice::Utils::policyIsValid(
                 qos.history))            == DDS::RETCODE_OK
         && (result = DDS::OpenSplice::Utils::policyIsValid(
                 qos.resource_limits))    == DDS::RETCODE_OK
         && (result = DDS::OpenSplice::Utils::policyIsValid(
                 qos.transport_priority)) == DDS::RETCODE_OK
         && (result = DDS::OpenSplice::Utils::policyIsValid(
                 qos.lifespan))           == DDS::RETCODE_OK
         && (result = DDS::OpenSplice::Utils::policyIsValid(
                 qos.ownership))          == DDS::RETCODE_OK)
        {
            result = DDS::OpenSplice::Utils::policiesAreConsistent(
                qos.history, qos.resource_limits);
        }
    }

    return result;
}




/*
 * QoS Comparison
 */
DDS::Boolean
DDS::OpenSplice::Utils::qosIsEqual (
    const DDS::DataReaderQos &a,
    const DDS::DataReaderQos &b)
{
    DDS::Boolean equal = TRUE;

    if (&a != &b) {
        if (policyIsEqual (a.durability, b.durability) == FALSE ||
            policyIsEqual (a.deadline, b.deadline) == FALSE ||
            policyIsEqual (a.latency_budget, b.latency_budget) == FALSE ||
            policyIsEqual (a.liveliness, b.liveliness) == FALSE ||
            policyIsEqual (a.reliability, b.reliability) == FALSE ||
            policyIsEqual (a.destination_order, b.destination_order) == FALSE ||
            policyIsEqual (a.history, b.history) == FALSE ||
            policyIsEqual (a.resource_limits, b.resource_limits) == FALSE ||
            policyIsEqual (a.user_data, b.user_data) == FALSE ||
            policyIsEqual (a.ownership, b.ownership) == FALSE ||
            policyIsEqual (a.time_based_filter, b.time_based_filter) == FALSE ||
            policyIsEqual (a.reader_data_lifecycle, b.reader_data_lifecycle) == FALSE ||
            policyIsEqual (a.subscription_keys, b.subscription_keys) == FALSE ||
            policyIsEqual (a.reader_lifespan, b.reader_lifespan) == FALSE ||
            policyIsEqual (a.share, b.share) == FALSE)
        {
            equal = FALSE;
        }
    }

    return equal;
}

DDS::Boolean
DDS::OpenSplice::Utils::qosIsEqual (
    const DDS::DataReaderViewQos &a,
    const DDS::DataReaderViewQos &b)
{
    DDS::Boolean equal = TRUE;

    if (&a != &b) {
        equal = policyIsEqual (a.view_keys, b.view_keys);
    }

    return equal;
}

DDS::Boolean
DDS::OpenSplice::Utils::qosIsEqual (
    const DDS::DataWriterQos &a,
    const DDS::DataWriterQos &b)
{
    DDS::Boolean equal = TRUE;

    if (&a != &b) {
        if (policyIsEqual (a.durability, b.durability) == FALSE ||
            policyIsEqual (a.deadline, b.deadline) == FALSE ||
            policyIsEqual (a.latency_budget, b.latency_budget) == FALSE ||
            policyIsEqual (a.liveliness, b.liveliness) == FALSE ||
            policyIsEqual (a.reliability, b.reliability) == FALSE ||
            policyIsEqual (a.destination_order, b.destination_order) == FALSE ||
            policyIsEqual (a.history, b.history) == FALSE ||
            policyIsEqual (a.resource_limits, b.resource_limits) == FALSE ||
            policyIsEqual (a.transport_priority, b.transport_priority) == FALSE ||
            policyIsEqual (a.lifespan, b.lifespan) == FALSE ||
            policyIsEqual (a.user_data, b.user_data) == FALSE ||
            policyIsEqual (a.ownership, b.ownership) == FALSE ||
            policyIsEqual (a.ownership_strength, b.ownership_strength) == FALSE ||
            policyIsEqual (a.writer_data_lifecycle, b.writer_data_lifecycle) == FALSE)
        {
            equal = FALSE;
        }
    }

    return equal;
}

DDS::Boolean
DDS::OpenSplice::Utils::qosIsEqual (
    const DDS::DomainParticipantFactoryQos &a,
    const DDS::DomainParticipantFactoryQos &b)
{
    DDS::Boolean equal = TRUE;

    if (&a != &b) {
        equal = policyIsEqual (a.entity_factory, b.entity_factory);
    }

    return equal;
}

DDS::Boolean
DDS::OpenSplice::Utils::qosIsEqual (
    const DDS::DomainParticipantQos &a,
    const DDS::DomainParticipantQos &b)
{
    DDS::Boolean equal = TRUE;

    if (&a != &b) {
        if (policyIsEqual (a.user_data, b.user_data) == FALSE ||
            policyIsEqual (a.entity_factory, b.entity_factory) == FALSE ||
            policyIsEqual (a.watchdog_scheduling, b.watchdog_scheduling) == FALSE ||
            policyIsEqual (a.listener_scheduling, b.listener_scheduling) == FALSE)
        {
            equal = FALSE;
        }
    }

    return equal;
}

DDS::Boolean
DDS::OpenSplice::Utils::qosIsEqual (
    const DDS::PublisherQos &a,
    const DDS::PublisherQos &b)
{
    DDS::Boolean equal = TRUE;

    if (&a != &b) {
        if (policyIsEqual (a.presentation, b.presentation) == FALSE ||
            policyIsEqual (a.partition, b.partition) == FALSE ||
            policyIsEqual (a.group_data, b.group_data) == FALSE ||
            policyIsEqual (a.entity_factory, b.entity_factory) == FALSE)
        {
            equal = FALSE;
        }
    }

    return equal;
}

DDS::Boolean
DDS::OpenSplice::Utils::qosIsEqual (
    const DDS::SubscriberQos &a,
    const DDS::SubscriberQos &b)
{
    DDS::Boolean equal = TRUE;

    if (&a != &b) {
        if (policyIsEqual (a.presentation, b.presentation) == FALSE ||
            policyIsEqual (a.partition, b.partition) == FALSE ||
            policyIsEqual (a.group_data, b.group_data) == FALSE ||
            policyIsEqual (a.entity_factory, b.entity_factory) == FALSE ||
            policyIsEqual (a.share, b.share) == FALSE)
        {
            equal = FALSE;
        }
    }

    return equal;
}

DDS::Boolean
DDS::OpenSplice::Utils::qosIsEqual (
    const DDS::TopicQos &a,
    const DDS::TopicQos &b)
{
    DDS::Boolean equal = TRUE;

    if (&a != &b) {
        if (policyIsEqual (a.topic_data, b.topic_data) == FALSE ||
            policyIsEqual (a.durability, b.durability) == FALSE ||
            policyIsEqual (a.durability_service, b.durability_service) == FALSE ||
            policyIsEqual (a.deadline, b.deadline) == FALSE ||
            policyIsEqual (a.latency_budget, b.latency_budget) == FALSE ||
            policyIsEqual (a.liveliness, b.liveliness) == FALSE ||
            policyIsEqual (a.reliability, b.reliability) == FALSE ||
            policyIsEqual (a.destination_order, b.destination_order) == FALSE ||
            policyIsEqual (a.history, b.history) == FALSE ||
            policyIsEqual (a.resource_limits, b.resource_limits) == FALSE ||
            policyIsEqual (a.transport_priority, b.transport_priority) == FALSE ||
            policyIsEqual (a.lifespan, b.lifespan) == FALSE ||
            policyIsEqual (a.ownership, b.ownership) == FALSE)
        {
            equal = FALSE;
        }
    }

    return equal;
}




/*
 * QoS conversions
 */
DDS::ReturnCode_t
DDS::OpenSplice::Utils::copyQosIn(
        const DDS::DataReaderQos &from,
        u_readerQos &to)
{
    DDS::ReturnCode_t result;

    /*
     * For clearity and consistency reasons, let the caller be responsible for both the
     * creation and destruction of the user layer QoS into which will be copied.
     * So, don't accept a NULL for the recieving qos and don't create it.
     */
    assert(to != NULL);

    result =     DDS::OpenSplice::Utils::copyPolicyIn(from.durability,            to->durability);
    if (result == DDS::RETCODE_OK) {
        result = DDS::OpenSplice::Utils::copyPolicyIn(from.deadline,              to->deadline);
    }
    if (result == DDS::RETCODE_OK) {
        result = DDS::OpenSplice::Utils::copyPolicyIn(from.latency_budget,        to->latency);
    }
    if (result == DDS::RETCODE_OK) {
        result = DDS::OpenSplice::Utils::copyPolicyIn(from.liveliness,            to->liveliness);
    }
    if (result == DDS::RETCODE_OK) {
        result = DDS::OpenSplice::Utils::copyPolicyIn(from.reliability,           to->reliability);
    }
    if (result == DDS::RETCODE_OK) {
        result = DDS::OpenSplice::Utils::copyPolicyIn(from.destination_order,     to->orderby);
    }
    if (result == DDS::RETCODE_OK) {
        result = DDS::OpenSplice::Utils::copyPolicyIn(from.history,               to->history);
    }
    if (result == DDS::RETCODE_OK) {
        result = DDS::OpenSplice::Utils::copyPolicyIn(from.resource_limits,       to->resource);
    }
    if (result == DDS::RETCODE_OK) {
        result = DDS::OpenSplice::Utils::copyPolicyIn(from.user_data,             to->userData );
    }
    if (result == DDS::RETCODE_OK) {
        result = DDS::OpenSplice::Utils::copyPolicyIn(from.ownership,             to->ownership);
    }
    if (result == DDS::RETCODE_OK) {
        result = DDS::OpenSplice::Utils::copyPolicyIn(from.time_based_filter,     to->pacing);
    }
    if (result == DDS::RETCODE_OK) {
        result = DDS::OpenSplice::Utils::copyPolicyIn(from.reader_data_lifecycle, to->lifecycle);
    }
    if (result == DDS::RETCODE_OK) {
        result = DDS::OpenSplice::Utils::copyPolicyIn(from.subscription_keys,     to->userKey);
    }
    if (result == DDS::RETCODE_OK) {
        result = DDS::OpenSplice::Utils::copyPolicyIn(from.reader_lifespan,       to->lifespan);
    }
    if (result == DDS::RETCODE_OK) {
        result = DDS::OpenSplice::Utils::copyPolicyIn(from.share,                 to->share);
    }

    return result;
}

DDS::ReturnCode_t
DDS::OpenSplice::Utils::copyQosIn(
        const DDS::DataReaderViewQos &from,
        u_dataViewQos &to)
{
    DDS::ReturnCode_t result;

    assert(to != NULL);

    result = DDS::OpenSplice::Utils::copyPolicyIn(from.view_keys, to->userKey);

    return result;
}


DDS::ReturnCode_t
DDS::OpenSplice::Utils::copyQosIn(
        const DDS::DataWriterQos &from,
        u_writerQos &to)
{
    DDS::ReturnCode_t result;

    assert(to != NULL);

    result =     DDS::OpenSplice::Utils::copyPolicyIn(from.durability,            to->durability);
    if (result == DDS::RETCODE_OK) {
        result = DDS::OpenSplice::Utils::copyPolicyIn(from.deadline,              to->deadline);
    }
    if (result == DDS::RETCODE_OK) {
        result = DDS::OpenSplice::Utils::copyPolicyIn(from.latency_budget,        to->latency);
    }
    if (result == DDS::RETCODE_OK) {
        result = DDS::OpenSplice::Utils::copyPolicyIn(from.liveliness,            to->liveliness);
    }
    if (result == DDS::RETCODE_OK) {
        result = DDS::OpenSplice::Utils::copyPolicyIn(from.reliability,           to->reliability);
    }
    if (result == DDS::RETCODE_OK) {
        result = DDS::OpenSplice::Utils::copyPolicyIn(from.destination_order,     to->orderby);
    }
    if (result == DDS::RETCODE_OK) {
        result = DDS::OpenSplice::Utils::copyPolicyIn(from.history,               to->history);
    }
    if (result == DDS::RETCODE_OK) {
        result = DDS::OpenSplice::Utils::copyPolicyIn(from.resource_limits,       to->resource);
    }
    if (result == DDS::RETCODE_OK) {
        result = DDS::OpenSplice::Utils::copyPolicyIn(from.transport_priority,    to->transport);
    }
    if (result == DDS::RETCODE_OK) {
        result = DDS::OpenSplice::Utils::copyPolicyIn(from.lifespan,              to->lifespan);
    }
    if (result == DDS::RETCODE_OK) {
        result = DDS::OpenSplice::Utils::copyPolicyIn(from.user_data,             to->userData);
    }
    if (result == DDS::RETCODE_OK) {
        result = DDS::OpenSplice::Utils::copyPolicyIn(from.ownership,             to->ownership);
    }
    if (result == DDS::RETCODE_OK) {
        result = DDS::OpenSplice::Utils::copyPolicyIn(from.ownership_strength,    to->strength);
    }
    if (result == DDS::RETCODE_OK) {
        result = DDS::OpenSplice::Utils::copyPolicyIn(from.writer_data_lifecycle, to->lifecycle);
    }

    return result;
}


DDS::ReturnCode_t
DDS::OpenSplice::Utils::copyQosIn(
        const DDS::DomainParticipantQos &from,
        u_participantQos &to)
{
    DDS::ReturnCode_t result;

    assert(to != NULL);

    result =     DDS::OpenSplice::Utils::copyPolicyIn(from.user_data,           to->userData);
    if (result == DDS::RETCODE_OK) {
        result = DDS::OpenSplice::Utils::copyPolicyIn(from.entity_factory,      to->entityFactory);
    }
    if (result == DDS::RETCODE_OK) {
        result = DDS::OpenSplice::Utils::copyPolicyIn(from.watchdog_scheduling, to->watchdogScheduling);
    }

    /*
     * DomainParticipantQos.listener_scheduling is not represented in u_participantQos.......
     */

    return result;
}

DDS::ReturnCode_t
DDS::OpenSplice::Utils::copyQosIn(
        const DDS::PublisherQos &from,
        u_publisherQos &to)
{
    DDS::ReturnCode_t result;

    assert(to != NULL);

    result =     DDS::OpenSplice::Utils::copyPolicyIn(from.presentation,   to->presentation);
    if (result == DDS::RETCODE_OK) {
        result = DDS::OpenSplice::Utils::copyPolicyIn(from.partition,      to->partition);
    }
    if (result == DDS::RETCODE_OK) {
        result = DDS::OpenSplice::Utils::copyPolicyIn(from.group_data,     to->groupData);
    }
    if (result == DDS::RETCODE_OK) {
        result = DDS::OpenSplice::Utils::copyPolicyIn(from.entity_factory, to->entityFactory);
    }

    return result;
}

DDS::ReturnCode_t
DDS::OpenSplice::Utils::copyQosIn(
        const DDS::SubscriberQos &from,
        u_subscriberQos &to)
{
    DDS::ReturnCode_t result;

    assert(to != NULL);

    result =     DDS::OpenSplice::Utils::copyPolicyIn(from.presentation,   to->presentation);
    if (result == DDS::RETCODE_OK) {
        result = DDS::OpenSplice::Utils::copyPolicyIn(from.partition,      to->partition);
    }
    if (result == DDS::RETCODE_OK) {
        result = DDS::OpenSplice::Utils::copyPolicyIn(from.group_data,     to->groupData);
    }
    if (result == DDS::RETCODE_OK) {
        result = DDS::OpenSplice::Utils::copyPolicyIn(from.entity_factory, to->entityFactory);
    }
    if (result == DDS::RETCODE_OK) {
        result = DDS::OpenSplice::Utils::copyPolicyIn(from.share,          to->share);
    }

    return result;
}

DDS::ReturnCode_t
DDS::OpenSplice::Utils::copyQosIn(
        const DDS::TopicQos &from,
        u_topicQos &to)
{
    DDS::ReturnCode_t result;

    assert(to != NULL);

    result =     DDS::OpenSplice::Utils::copyPolicyIn(from.topic_data,          to->topicData);
    if (result == DDS::RETCODE_OK) {
        result = DDS::OpenSplice::Utils::copyPolicyIn(from.liveliness,          to->liveliness);
    }
    if (result == DDS::RETCODE_OK) {
        result = DDS::OpenSplice::Utils::copyPolicyIn(from.reliability,         to->reliability);
    }
    if (result == DDS::RETCODE_OK) {
        result = DDS::OpenSplice::Utils::copyPolicyIn(from.destination_order,   to->orderby);
    }
    if (result == DDS::RETCODE_OK) {
        result = DDS::OpenSplice::Utils::copyPolicyIn(from.history,             to->history);
    }
    if (result == DDS::RETCODE_OK) {
        result = DDS::OpenSplice::Utils::copyPolicyIn(from.ownership,           to->ownership);
    }
    if (result == DDS::RETCODE_OK) {
        result = DDS::OpenSplice::Utils::copyPolicyIn(from.durability,          to->durability);
    }
    if (result == DDS::RETCODE_OK) {
        result = DDS::OpenSplice::Utils::copyPolicyIn(from.durability_service,  to->durabilityService);
    }
    if (result == DDS::RETCODE_OK) {
        result = DDS::OpenSplice::Utils::copyPolicyIn(from.resource_limits,     to->resource);
    }
    if (result == DDS::RETCODE_OK) {
        result = DDS::OpenSplice::Utils::copyPolicyIn(from.transport_priority,  to->transport);
    }
    if (result == DDS::RETCODE_OK) {
        result = DDS::OpenSplice::Utils::copyPolicyIn(from.lifespan,            to->lifespan);
    }
    if (result == DDS::RETCODE_OK) {
        result = DDS::OpenSplice::Utils::copyPolicyIn(from.deadline,            to->deadline);
    }
    if (result == DDS::RETCODE_OK) {
        result = DDS::OpenSplice::Utils::copyPolicyIn(from.latency_budget,      to->latency);
    }

    return result;
}

DDS::ReturnCode_t
DDS::OpenSplice::Utils::copyQosOut(
        const u_readerQos &from,
        DDS::DataReaderQos &to)
{
    DDS::ReturnCode_t result;

    assert(from != NULL);

    result =     DDS::OpenSplice::Utils::copyPolicyOut(from->durability,  to.durability);
    if (result == DDS::RETCODE_OK) {
        result = DDS::OpenSplice::Utils::copyPolicyOut(from->deadline,    to.deadline);
    }
    if (result == DDS::RETCODE_OK) {
        result = DDS::OpenSplice::Utils::copyPolicyOut(from->latency,     to.latency_budget);
    }
    if (result == DDS::RETCODE_OK) {
        result = DDS::OpenSplice::Utils::copyPolicyOut(from->liveliness,  to.liveliness);
    }
    if (result == DDS::RETCODE_OK) {
        result = DDS::OpenSplice::Utils::copyPolicyOut(from->reliability, to.reliability);
    }
    if (result == DDS::RETCODE_OK) {
        result = DDS::OpenSplice::Utils::copyPolicyOut(from->orderby,     to.destination_order);
    }
    if (result == DDS::RETCODE_OK) {
        result = DDS::OpenSplice::Utils::copyPolicyOut(from->history,     to.history);
    }
    if (result == DDS::RETCODE_OK) {
        result = DDS::OpenSplice::Utils::copyPolicyOut(from->resource,    to.resource_limits);
    }
    if (result == DDS::RETCODE_OK) {
        result = DDS::OpenSplice::Utils::copyPolicyOut(from->userData,    to.user_data);
    }
    if (result == DDS::RETCODE_OK) {
        result = DDS::OpenSplice::Utils::copyPolicyOut(from->ownership,   to.ownership);
    }
    if (result == DDS::RETCODE_OK) {
        result = DDS::OpenSplice::Utils::copyPolicyOut(from->pacing,      to.time_based_filter);
    }
    if (result == DDS::RETCODE_OK) {
        result = DDS::OpenSplice::Utils::copyPolicyOut(from->lifecycle,   to.reader_data_lifecycle);
    }
    if (result == DDS::RETCODE_OK) {
        result = DDS::OpenSplice::Utils::copyPolicyOut(from->userKey,     to.subscription_keys);
    }
    if (result == DDS::RETCODE_OK) {
        result = DDS::OpenSplice::Utils::copyPolicyOut(from->lifespan,    to.reader_lifespan);
    }
    if (result == DDS::RETCODE_OK) {
        result = DDS::OpenSplice::Utils::copyPolicyOut(from->share,       to.share);
    }

    return result;
}

DDS::ReturnCode_t
DDS::OpenSplice::Utils::copyQosOut(
        const u_dataViewQos &from,
        DDS::DataReaderViewQos &to)
{
    DDS::ReturnCode_t result;

    assert(from != NULL);

    result = DDS::OpenSplice::Utils::copyPolicyOut(from->userKey, to.view_keys);

    return result;
}

DDS::ReturnCode_t
DDS::OpenSplice::Utils::copyQosOut(
        const u_writerQos &from,
        DDS::DataWriterQos &to)
{
    DDS::ReturnCode_t result;

    assert(from != NULL);

    result =     DDS::OpenSplice::Utils::copyPolicyOut(from->durability,  to.durability);
    if (result == DDS::RETCODE_OK) {
        result = DDS::OpenSplice::Utils::copyPolicyOut(from->deadline,    to.deadline);
    }
    if (result == DDS::RETCODE_OK) {
        result = DDS::OpenSplice::Utils::copyPolicyOut(from->latency,     to.latency_budget);
    }
    if (result == DDS::RETCODE_OK) {
        result = DDS::OpenSplice::Utils::copyPolicyOut(from->liveliness,  to.liveliness);
    }
    if (result == DDS::RETCODE_OK) {
        result = DDS::OpenSplice::Utils::copyPolicyOut(from->reliability, to.reliability);
    }
    if (result == DDS::RETCODE_OK) {
        result = DDS::OpenSplice::Utils::copyPolicyOut(from->orderby,     to.destination_order);
    }
    if (result == DDS::RETCODE_OK) {
        result = DDS::OpenSplice::Utils::copyPolicyOut(from->history,     to.history);
    }
    if (result == DDS::RETCODE_OK) {
        result = DDS::OpenSplice::Utils::copyPolicyOut(from->resource,    to.resource_limits);
    }
    if (result == DDS::RETCODE_OK) {
        result = DDS::OpenSplice::Utils::copyPolicyOut(from->transport,   to.transport_priority);
    }
    if (result == DDS::RETCODE_OK) {
        result = DDS::OpenSplice::Utils::copyPolicyOut(from->lifespan,    to.lifespan);
    }
    if (result == DDS::RETCODE_OK) {
        result = DDS::OpenSplice::Utils::copyPolicyOut(from->userData,    to.user_data);
    }
    if (result == DDS::RETCODE_OK) {
        result = DDS::OpenSplice::Utils::copyPolicyOut(from->ownership,   to.ownership);
    }
    if (result == DDS::RETCODE_OK) {
        result = DDS::OpenSplice::Utils::copyPolicyOut(from->strength,    to.ownership_strength);
    }
    if (result == DDS::RETCODE_OK) {
        result = DDS::OpenSplice::Utils::copyPolicyOut(from->lifecycle,   to.writer_data_lifecycle);
    }

    return result;
}

DDS::ReturnCode_t
DDS::OpenSplice::Utils::copyQosOut(
        const u_participantQos &from,
        DDS::DomainParticipantQos &to)
{
    DDS::ReturnCode_t result;

    assert(from != NULL);

    result =     DDS::OpenSplice::Utils::copyPolicyOut(from->userData,            to.user_data);
    if (result == DDS::RETCODE_OK) {
        result = DDS::OpenSplice::Utils::copyPolicyOut(from->entityFactory,       to.entity_factory);
    }
    if (result == DDS::RETCODE_OK) {
        result = DDS::OpenSplice::Utils::copyPolicyOut(from->watchdogScheduling,  to.watchdog_scheduling);
    }

    /* DomainParticipantQos.listener_scheduling is not represented in u_participantQos:
     * Set default values.
     * TODO: Question; is default values for listener_scheduling good enough? */
    to.listener_scheduling = SchedulingQosPolicy_default;

    return result;
}

DDS::ReturnCode_t
DDS::OpenSplice::Utils::copyQosOut(
        const u_publisherQos &from,
        DDS::PublisherQos &to)
{
    DDS::ReturnCode_t result;

    assert(from != NULL);

    result =     DDS::OpenSplice::Utils::copyPolicyOut(from->presentation,   to.presentation);
    if (result == DDS::RETCODE_OK) {
        result = DDS::OpenSplice::Utils::copyPolicyOut(from->partition,      to.partition);
    }
    if (result == DDS::RETCODE_OK) {
        result = DDS::OpenSplice::Utils::copyPolicyOut(from->groupData,      to.group_data);
    }
    if (result == DDS::RETCODE_OK) {
        result = DDS::OpenSplice::Utils::copyPolicyOut(from->entityFactory,  to.entity_factory);
    }

    return result;
}

DDS::ReturnCode_t
DDS::OpenSplice::Utils::copyQosOut(
        const u_subscriberQos &from,
        DDS::SubscriberQos &to)
{
    DDS::ReturnCode_t result;

    assert(from != NULL);

    result =     DDS::OpenSplice::Utils::copyPolicyOut(from->presentation,   to.presentation);
    if (result == DDS::RETCODE_OK) {
        result = DDS::OpenSplice::Utils::copyPolicyOut(from->partition,      to.partition);
    }
    if (result == DDS::RETCODE_OK) {
        result = DDS::OpenSplice::Utils::copyPolicyOut(from->groupData,      to.group_data);
    }
    if (result == DDS::RETCODE_OK) {
        result = DDS::OpenSplice::Utils::copyPolicyOut(from->entityFactory,  to.entity_factory);
    }
    if (result == DDS::RETCODE_OK) {
        result = DDS::OpenSplice::Utils::copyPolicyOut(from->share,          to.share);
    }

    return result;
}

DDS::ReturnCode_t
DDS::OpenSplice::Utils::copyQosOut(
        const u_topicQos &from,
        DDS::TopicQos &to)
{
    DDS::ReturnCode_t result;

    assert(from != NULL);

    result =     DDS::OpenSplice::Utils::copyPolicyOut(from->topicData,         to.topic_data);
    if (result == DDS::RETCODE_OK) {
        result = DDS::OpenSplice::Utils::copyPolicyOut(from->durability,        to.durability);
    }
    if (result == DDS::RETCODE_OK) {
        result = DDS::OpenSplice::Utils::copyPolicyOut(from->durabilityService, to.durability_service);
    }
    if (result == DDS::RETCODE_OK) {
        result = DDS::OpenSplice::Utils::copyPolicyOut(from->liveliness,        to.liveliness);
    }
    if (result == DDS::RETCODE_OK) {
        result = DDS::OpenSplice::Utils::copyPolicyOut(from->reliability,       to.reliability);
    }
    if (result == DDS::RETCODE_OK) {
        result = DDS::OpenSplice::Utils::copyPolicyOut(from->orderby,           to.destination_order);
    }
    if (result == DDS::RETCODE_OK) {
        result = DDS::OpenSplice::Utils::copyPolicyOut(from->history,           to.history);
    }
    if (result == DDS::RETCODE_OK) {
        result = DDS::OpenSplice::Utils::copyPolicyOut(from->ownership,         to.ownership);
    }
    if (result == DDS::RETCODE_OK) {
        result = DDS::OpenSplice::Utils::copyPolicyOut(from->resource,          to.resource_limits);
    }
    if (result == DDS::RETCODE_OK) {
        result = DDS::OpenSplice::Utils::copyPolicyOut(from->transport,         to.transport_priority);
    }
    if (result == DDS::RETCODE_OK) {
        result = DDS::OpenSplice::Utils::copyPolicyOut(from->lifespan,          to.lifespan);
    }
    if (result == DDS::RETCODE_OK) {
        result = DDS::OpenSplice::Utils::copyPolicyOut(from->deadline,          to.deadline);
    }
    if (result == DDS::RETCODE_OK) {
        result = DDS::OpenSplice::Utils::copyPolicyOut(from->latency,           to.latency_budget);
    }

    return result;
}
