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
#ifndef CCPP_QOSUTILS_H
#define CCPP_QOSUTILS_H

#include "ccpp_Utils.h"
#include "gapi.h"

/* !!!!!!!!NOTE From here no more includes are allowed!!!!!!! */

C_CLASS(qp_qosProviderInputAttr);

//policies conversions

namespace DDS
{
    OS_DCPS_API void ccpp_UserDataQosPolicy_copyIn(
                  const ::DDS::UserDataQosPolicy &from,
                  gapi_userDataQosPolicy &to );

    OS_DCPS_API void ccpp_UserDataQosPolicy_copyOut(
                  const gapi_userDataQosPolicy &from,
                  ::DDS::UserDataQosPolicy &to );

    OS_DCPS_API void ccpp_EntityFactoryQosPolicy_copyIn(
                  const ::DDS::EntityFactoryQosPolicy &from,
                  gapi_entityFactoryQosPolicy &to);

    OS_DCPS_API void ccpp_EntityFactoryQosPolicy_copyOut(
                  const gapi_entityFactoryQosPolicy &from,
                  ::DDS::EntityFactoryQosPolicy &to);

    OS_DCPS_API void ccpp_TopicDataQosPolicy_copyIn(
                  const ::DDS::TopicDataQosPolicy &from,
                  gapi_topicDataQosPolicy &to);

    OS_DCPS_API void ccpp_DurabilityQosPolicy_copyIn(
                  const ::DDS::DurabilityQosPolicy &from,
                  gapi_durabilityQosPolicy &to);

    OS_DCPS_API void ccpp_DurabilityServiceQosPolicy_copyIn(
                  const ::DDS::DurabilityServiceQosPolicy &from,
                  gapi_durabilityServiceQosPolicy &to);

    OS_DCPS_API void ccpp_DeadlineQosPolicy_copyIn(
                  const ::DDS::DeadlineQosPolicy &from,
                  gapi_deadlineQosPolicy &to);

    OS_DCPS_API void ccpp_LatencyBudgetQosPolicy_copyIn(
                  const ::DDS::LatencyBudgetQosPolicy &from,
                  gapi_latencyBudgetQosPolicy &to);

    OS_DCPS_API void ccpp_LivelinessQosPolicy_copyIn(
                  const ::DDS::LivelinessQosPolicy &from,
                  gapi_livelinessQosPolicy &to);

    OS_DCPS_API void ccpp_ReliabilityQosPolicy_copyIn(
                  const ::DDS::ReliabilityQosPolicy &from,
                  gapi_reliabilityQosPolicy &to);

    OS_DCPS_API void ccpp_DestinationOrderQosPolicy_copyIn(
                  const ::DDS::DestinationOrderQosPolicy &from,
                  gapi_destinationOrderQosPolicy &to);

    OS_DCPS_API void ccpp_HistoryQosPolicy_copyIn(
                  const ::DDS::HistoryQosPolicy &from,
                  gapi_historyQosPolicy &to);

    OS_DCPS_API void ccpp_ResourceLimitsQosPolicy_copyIn(
                  const ::DDS::ResourceLimitsQosPolicy &from,
                  gapi_resourceLimitsQosPolicy &to);

    OS_DCPS_API void ccpp_TransportPriorityQosPolicy_copyIn(
                  const ::DDS::TransportPriorityQosPolicy &from,
                  gapi_transportPriorityQosPolicy &to);

    OS_DCPS_API void ccpp_LifespanQosPolicy_copyIn(
                  const ::DDS::LifespanQosPolicy &from,
                  gapi_lifespanQosPolicy &to);

    OS_DCPS_API void ccpp_OwnershipQosPolicy_copyIn(
                  const ::DDS::OwnershipQosPolicy &from,
                  gapi_ownershipQosPolicy &to);

    OS_DCPS_API void ccpp_SubscriptionKeyQosPolicy_copyIn (
                  const DDS::SubscriptionKeyQosPolicy &from,
                  gapi_subscriptionKeyQosPolicy &to );

    OS_DCPS_API void ccpp_ReaderLifespanQosPolicy_copyIn (
                  const DDS::ReaderLifespanQosPolicy &from,
                  gapi_readerLifespanQosPolicy &to );

    OS_DCPS_API void ccpp_ShareQosPolicy_copyIn (
                  const DDS::ShareQosPolicy &from,
                  gapi_shareQosPolicy &to );

    OS_DCPS_API void ccpp_TopicDataQosPolicy_copyOut(
                  const gapi_topicDataQosPolicy &from,
                  ::DDS::TopicDataQosPolicy &to);

    OS_DCPS_API void ccpp_DurabilityQosPolicy_copyOut(
                  const gapi_durabilityQosPolicy &from,
                  ::DDS::DurabilityQosPolicy &to);

    OS_DCPS_API void ccpp_DurabilityServiceQosPolicy_copyOut(
                  const gapi_durabilityServiceQosPolicy &from,
                  ::DDS::DurabilityServiceQosPolicy &to);

    OS_DCPS_API void ccpp_DeadlineQosPolicy_copyOut(
                  const gapi_deadlineQosPolicy &from,
                  ::DDS::DeadlineQosPolicy &to);

    OS_DCPS_API void ccpp_LatencyBudgetQosPolicy_copyOut(
                  const gapi_latencyBudgetQosPolicy &from,
                  ::DDS::LatencyBudgetQosPolicy &to);

    OS_DCPS_API void ccpp_LivelinessQosPolicy_copyOut(
                  const gapi_livelinessQosPolicy &from,
                  ::DDS::LivelinessQosPolicy &to);

    OS_DCPS_API void ccpp_ReliabilityQosPolicy_copyOut(
                  const gapi_reliabilityQosPolicy &from,
                  ::DDS::ReliabilityQosPolicy &to);

    OS_DCPS_API void ccpp_DestinationOrderQosPolicy_copyOut(
                  const gapi_destinationOrderQosPolicy &from,
                  ::DDS::DestinationOrderQosPolicy &to);

    OS_DCPS_API void ccpp_HistoryQosPolicy_copyOut(
                  const gapi_historyQosPolicy &from,
                  ::DDS::HistoryQosPolicy &to);

    OS_DCPS_API void ccpp_ResourceLimitsQosPolicy_copyOut(
                  const gapi_resourceLimitsQosPolicy &from,
                  ::DDS::ResourceLimitsQosPolicy &to);

    OS_DCPS_API void ccpp_TransportPriorityQosPolicy_copyOut(
                  const gapi_transportPriorityQosPolicy &from,
                  ::DDS::TransportPriorityQosPolicy &to);

    OS_DCPS_API void ccpp_LifespanQosPolicy_copyOut(
                  const gapi_lifespanQosPolicy &from,
                  ::DDS::LifespanQosPolicy &to);

    OS_DCPS_API void ccpp_OwnershipQosPolicy_copyOut(
                  const gapi_ownershipQosPolicy &from,
                  ::DDS::OwnershipQosPolicy &to);

    OS_DCPS_API void ccpp_SubscriptionKeyQosPolicy_copyOut(
                  const gapi_subscriptionKeyQosPolicy &from,
                  DDS::SubscriptionKeyQosPolicy &to );

    OS_DCPS_API void ccpp_ReaderLifespanQosPolicy_copyOut (
                  const gapi_readerLifespanQosPolicy &from,
                  DDS::ReaderLifespanQosPolicy &to );

    OS_DCPS_API void ccpp_ShareQosPolicy_copyOut (
                  const gapi_shareQosPolicy &from,
    DDS::ShareQosPolicy &to );

    //Qos conversions

    OS_DCPS_API void ccpp_DomainParticipantFactoryQos_copyIn(
                  const ::DDS::DomainParticipantFactoryQos &from,
                  gapi_domainParticipantFactoryQos &to);

    OS_DCPS_API void ccpp_DomainParticipantFactoryQos_copyOut(
                  const gapi_domainParticipantFactoryQos &from,
                  ::DDS::DomainParticipantFactoryQos &to);

    OS_DCPS_API void ccpp_DomainParticipantQos_copyIn(
                  const ::DDS::DomainParticipantQos &from,
                  gapi_domainParticipantQos &to);

    OS_DCPS_API void ccpp_DomainParticipantQos_copyOut(
                  const gapi_domainParticipantQos &from,
                  ::DDS::DomainParticipantQos &to);

    OS_DCPS_API void ccpp_TopicQos_copyIn(
                  const ::DDS::TopicQos &from,
                  gapi_topicQos &to);

    OS_DCPS_API void ccpp_TopicQos_copyOut(
                  const gapi_topicQos &from,
                  ::DDS::TopicQos &to);

    OS_DCPS_API void ccpp_DataWriterQos_copyOut(
                  const gapi_dataWriterQos &from,
                  ::DDS::DataWriterQos &to);

    OS_DCPS_API void ccpp_DataReaderQos_copyOut(
                  const gapi_dataReaderQos &from,
                  ::DDS::DataReaderQos &to);

    OS_DCPS_API void ccpp_DataWriterQos_copyIn(
                  const ::DDS::DataWriterQos &from,
                  gapi_dataWriterQos &to);

    OS_DCPS_API void ccpp_DataReaderQos_copyIn(
                  const ::DDS::DataReaderQos &from,
                  gapi_dataReaderQos &to);

    OS_DCPS_API void ccpp_PublisherQos_copyOut(
                  const gapi_publisherQos &from,
                  ::DDS::PublisherQos &to);

    OS_DCPS_API void ccpp_PublisherQos_copyIn(
                  const ::DDS::PublisherQos &from,
                  gapi_publisherQos &to);

    OS_DCPS_API void ccpp_OwnershipStrengthQosPolicy_copyIn(
                  const ::DDS::OwnershipStrengthQosPolicy &from,
                  gapi_ownershipStrengthQosPolicy &to);

    OS_DCPS_API void ccpp_WriterDataLifecycleQosPolicy_copyIn(
                  const ::DDS::WriterDataLifecycleQosPolicy &from,
                  gapi_writerDataLifecycleQosPolicy &to);

    OS_DCPS_API void ccpp_PresentationQosPolicy_copyIn(
                  const ::DDS::PresentationQosPolicy & from,
                  gapi_presentationQosPolicy & to);

    OS_DCPS_API void ccpp_PartitionQosPolicy_copyIn(
                  const ::DDS::PartitionQosPolicy &from,
                  gapi_partitionQosPolicy &to);

    OS_DCPS_API void ccpp_GroupDataQosPolicy_copyIn(
                  const ::DDS::GroupDataQosPolicy &from,
                  gapi_groupDataQosPolicy &to);

    OS_DCPS_API void ccpp_OwnershipStrengthQosPolicy_copyOut(
                  const gapi_ownershipStrengthQosPolicy &from,
                  ::DDS::OwnershipStrengthQosPolicy &to);

    OS_DCPS_API void ccpp_WriterDataLifecycleQosPolicy_copyOut(
                  const gapi_writerDataLifecycleQosPolicy &from,
                  ::DDS::WriterDataLifecycleQosPolicy &to);

    OS_DCPS_API void ccpp_ReaderDataLifecycleQosPolicy_copyIn(
                  const ::DDS::ReaderDataLifecycleQosPolicy &from,
                  gapi_readerDataLifecycleQosPolicy &to);

    OS_DCPS_API void ccpp_ReaderDataLifecycleQosPolicy_copyOut(
                  const gapi_readerDataLifecycleQosPolicy &from,
                  ::DDS::ReaderDataLifecycleQosPolicy &to);

    OS_DCPS_API void ccpp_PresentationQosPolicy_copyOut(
                  const gapi_presentationQosPolicy & from,
                  ::DDS::PresentationQosPolicy & to);

    OS_DCPS_API void ccpp_PartitionQosPolicy_copyOut(
                  const gapi_partitionQosPolicy &from,
                  ::DDS::PartitionQosPolicy &to);

    OS_DCPS_API void ccpp_GroupDataQosPolicy_copyOut(
                  const gapi_groupDataQosPolicy &from,
                  ::DDS::GroupDataQosPolicy &to);

    OS_DCPS_API void ccpp_SchedulingClassQosPolicy_copyOut(
                  const gapi_schedulingClassQosPolicy &from,
                  ::DDS::SchedulingClassQosPolicy &to);

    OS_DCPS_API void ccpp_SchedulingClassQosPolicy_copyIn(
                  const ::DDS::SchedulingClassQosPolicy &from,
                  gapi_schedulingClassQosPolicy &to);

    OS_DCPS_API void ccpp_SchedulingPriorityQosPolicy_copyOut(
                  const gapi_schedulingPriorityQosPolicy &from,
                  ::DDS::SchedulingPriorityQosPolicy &to);

    OS_DCPS_API void ccpp_SchedulingPriorityQosPolicy_copyIn(
                  const ::DDS::SchedulingPriorityQosPolicy &from,
                  gapi_schedulingPriorityQosPolicy &to);

    OS_DCPS_API void ccpp_SchedulingQosPolicy_copyOut(
                  const gapi_schedulingQosPolicy &from,
                  ::DDS::SchedulingQosPolicy &to);

    OS_DCPS_API void ccpp_SchedulingQosPolicy_copyIn(
                  const ::DDS::SchedulingQosPolicy &from,
                  gapi_schedulingQosPolicy &to);

    OS_DCPS_API void ccpp_SubscriberQos_copyIn(
                  const ::DDS::SubscriberQos &from,
          	gapi_subscriberQos &to);

    OS_DCPS_API void ccpp_SubscriberQos_copyOut(
                  const gapi_subscriberQos &from,
                  ::DDS::SubscriberQos &to);

    OS_DCPS_API void ccpp_DataReaderViewQos_copyIn(
                   const ::DDS::DataReaderViewQos &from,
                   gapi_dataReaderViewQos &to);

    OS_DCPS_API void ccpp_ViewKeyQosPolicy_copyIn(
            const ::DDS::ViewKeyQosPolicy &from,
            gapi_viewKeyQosPolicy &to);

    OS_DCPS_API void ccpp_DataReaderViewQos_copyOut(
            const gapi_dataReaderViewQos &from,
            ::DDS::DataReaderViewQos &to);

    OS_DCPS_API void ccpp_ViewKeyQosPolicy_copyOut(
            const gapi_viewKeyQosPolicy &from,
            ::DDS::ViewKeyQosPolicy &to);

    OS_DCPS_API void ccpp_OfferedIncompatibleQosStatus_copyOut(
                  const gapi_offeredIncompatibleQosStatus & from,
                  ::DDS::OfferedIncompatibleQosStatus &to);

    OS_DCPS_API void ccpp_RequestedIncompatibleQosStatus_copyOut(
                  const gapi_requestedIncompatibleQosStatus & from,
                  ::DDS::RequestedIncompatibleQosStatus &to);

   OS_DCPS_API void ccpp_TimeBasedFilterQosPolicy_copyOut(
                  const gapi_timeBasedFilterQosPolicy & from,
                  ::DDS::TimeBasedFilterQosPolicy & to);

   OS_DCPS_API void ccpp_TimeBasedFilterQosPolicy_copyIn(
                  const ::DDS::TimeBasedFilterQosPolicy & from,
                  gapi_timeBasedFilterQosPolicy & to);

    OS_DCPS_API void ccpp_RequestedDeadlineMissedStatus_copyOut(
                  const gapi_requestedDeadlineMissedStatus & from,
                  ::DDS::RequestedDeadlineMissedStatus &to);

    OS_DCPS_API void ccpp_SampleRejectedStatusKind_copyOut(
                  const gapi_sampleRejectedStatusKind & from,
                  ::DDS::SampleRejectedStatusKind & to);

    OS_DCPS_API void ccpp_SampleRejectedStatus_copyOut(
                  const gapi_sampleRejectedStatus & from,
                  ::DDS::SampleRejectedStatus & to);

    OS_DCPS_API void ccpp_LivelinessChangedStatus_copyOut(
                  const gapi_livelinessChangedStatus & from,
                  ::DDS::LivelinessChangedStatus &to);

    OS_DCPS_API void ccpp_SubscriptionMatchedStatus_copyOut(
                  const gapi_subscriptionMatchedStatus & from,
                  ::DDS::SubscriptionMatchedStatus &to);

    OS_DCPS_API void ccpp_SampleLostStatus_copyOut(
                  const gapi_sampleLostStatus & from,
                  ::DDS::SampleLostStatus &to);

    OS_DCPS_API void ccpp_InconsistentTopicStatus_copyOut(
                  const gapi_inconsistentTopicStatus & from,
                  ::DDS::InconsistentTopicStatus &to);

    OS_DCPS_API void ccpp_AllDataDisposedTopicStatus_copyOut(
                  const gapi_allDataDisposedTopicStatus & from,
                  ::DDS::AllDataDisposedTopicStatus &to);

    OS_DCPS_API void ccpp_OfferedDeadlineMissedStatus_copyOut(
                  const gapi_offeredDeadlineMissedStatus & from,
                  ::DDS::OfferedDeadlineMissedStatus &to);

    OS_DCPS_API void ccpp_LivelinessLostStatus_copyOut(
                  const gapi_livelinessLostStatus & from,
                  ::DDS::LivelinessLostStatus &to);

    OS_DCPS_API void ccpp_PublicationMatchedStatus_copyOut(
                  const gapi_publicationMatchedStatus & from,
                  ::DDS::PublicationMatchedStatus &to);

    OS_DCPS_API void ccpp_InvalidSampleVisibilityQosPolicy_copyOut(
                  const gapi_invalidSampleVisibilityQosPolicy &from,
                  ::DDS::InvalidSampleVisibilityQosPolicy &to);

    OS_DCPS_API void ccpp_InvalidSampleVisibilityQosPolicy_copyIn(
                  const ::DDS::InvalidSampleVisibilityQosPolicy &from,
                  gapi_invalidSampleVisibilityQosPolicy &to);

    struct OS_DCPS_API DefaultQos
    {
        static ::DDS::DomainParticipantFactoryQos_var ParticipantFactoryQosDefault;
        static ::DDS::DomainParticipantQos_var        ParticipantQosDefault;
        static ::DDS::TopicQos_var                    TopicQosDefault;
        static ::DDS::PublisherQos_var                PublisherQosDefault;
        static ::DDS::SubscriberQos_var               SubscriberQosDefault;
        static ::DDS::DataReaderQos_var               DataReaderQosDefault;
        static ::DDS::DataReaderQos_var               DataReaderQosUseTopicQos;
        static ::DDS::DataWriterQos_var               DataWriterQosDefault;
        static ::DDS::DataWriterQos_var               DataWriterQosUseTopicQos;
        static ::DDS::DataReaderViewQos_var           DataReaderViewQosDefault;
    };

    OS_DCPS_API const C_STRUCT(qp_qosProviderInputAttr) * ccpp_getQosProviderInputAttr();

    /* Overloaded QoS policy equality operators */
    OS_DCPS_API bool operator==( const DDS::Duration_t &a, const DDS::Duration_t &b);
    OS_DCPS_API bool operator!=( const DDS::Duration_t &a, const DDS::Duration_t &b);

    OS_DCPS_API bool operator==( const DDS::octSeq &a, const DDS::octSeq &b);
    OS_DCPS_API bool operator!=( const DDS::octSeq &a, const DDS::octSeq &b);

    OS_DCPS_API bool operator==( const DDS::StringSeq &a, const DDS::StringSeq &b);
    OS_DCPS_API bool operator!=( const DDS::StringSeq &a, const DDS::StringSeq &b);

    OS_DCPS_API bool operator==( const DDS::DeadlineQosPolicy            &a, const DDS::DeadlineQosPolicy            &b);
    OS_DCPS_API bool operator!=( const DDS::DeadlineQosPolicy            &a, const DDS::DeadlineQosPolicy            &b);

    OS_DCPS_API bool operator==( const DDS::DestinationOrderQosPolicy    &a, const DDS::DestinationOrderQosPolicy    &b);
    OS_DCPS_API bool operator!=( const DDS::DestinationOrderQosPolicy    &a, const DDS::DestinationOrderQosPolicy    &b);

    OS_DCPS_API bool operator==( const DDS::DurabilityQosPolicy          &a, const DDS::DurabilityQosPolicy          &b);
    OS_DCPS_API bool operator!=( const DDS::DurabilityQosPolicy          &a, const DDS::DurabilityQosPolicy          &b);

    OS_DCPS_API bool operator==( const DDS::DurabilityServiceQosPolicy   &a, const DDS::DurabilityServiceQosPolicy   &b);
    OS_DCPS_API bool operator!=( const DDS::DurabilityServiceQosPolicy   &a, const DDS::DurabilityServiceQosPolicy   &b);

    OS_DCPS_API bool operator==( const DDS::EntityFactoryQosPolicy       &a, const DDS::EntityFactoryQosPolicy       &b);
    OS_DCPS_API bool operator!=( const DDS::EntityFactoryQosPolicy       &a, const DDS::EntityFactoryQosPolicy       &b);

    OS_DCPS_API bool operator==( const DDS::GroupDataQosPolicy           &a, const DDS::GroupDataQosPolicy           &b);
    OS_DCPS_API bool operator!=( const DDS::GroupDataQosPolicy           &a, const DDS::GroupDataQosPolicy           &b);

    OS_DCPS_API bool operator==( const DDS::HistoryQosPolicy             &a, const DDS::HistoryQosPolicy             &b);
    OS_DCPS_API bool operator!=( const DDS::HistoryQosPolicy             &a, const DDS::HistoryQosPolicy             &b);

    OS_DCPS_API bool operator==( const DDS::LatencyBudgetQosPolicy       &a, const DDS::LatencyBudgetQosPolicy       &b);
    OS_DCPS_API bool operator!=( const DDS::LatencyBudgetQosPolicy       &a, const DDS::LatencyBudgetQosPolicy       &b);

    OS_DCPS_API bool operator==( const DDS::LifespanQosPolicy            &a, const DDS::LifespanQosPolicy            &b);
    OS_DCPS_API bool operator!=( const DDS::LifespanQosPolicy            &a, const DDS::LifespanQosPolicy            &b);

    OS_DCPS_API bool operator==( const DDS::LivelinessQosPolicy          &a, const DDS::LivelinessQosPolicy          &b);
    OS_DCPS_API bool operator!=( const DDS::LivelinessQosPolicy          &a, const DDS::LivelinessQosPolicy          &b);

    OS_DCPS_API bool operator==( const DDS::OwnershipQosPolicy           &a, const DDS::OwnershipQosPolicy           &b);
    OS_DCPS_API bool operator!=( const DDS::OwnershipQosPolicy           &a, const DDS::OwnershipQosPolicy           &b);

    OS_DCPS_API bool operator==( const DDS::OwnershipStrengthQosPolicy   &a, const DDS::OwnershipStrengthQosPolicy   &b);
    OS_DCPS_API bool operator!=( const DDS::OwnershipStrengthQosPolicy   &a, const DDS::OwnershipStrengthQosPolicy   &b);

    OS_DCPS_API bool operator==( const DDS::PartitionQosPolicy           &a, const DDS::PartitionQosPolicy           &b);
    OS_DCPS_API bool operator!=( const DDS::PartitionQosPolicy           &a, const DDS::PartitionQosPolicy           &b);

    OS_DCPS_API bool operator==( const DDS::PresentationQosPolicy        &a, const DDS::PresentationQosPolicy        &b);
    OS_DCPS_API bool operator!=( const DDS::PresentationQosPolicy        &a, const DDS::PresentationQosPolicy        &b);

    OS_DCPS_API bool operator==( const DDS::ReaderDataLifecycleQosPolicy &a, const DDS::ReaderDataLifecycleQosPolicy &b);
    OS_DCPS_API bool operator!=( const DDS::ReaderDataLifecycleQosPolicy &a, const DDS::ReaderDataLifecycleQosPolicy &b);

    OS_DCPS_API bool operator==( const DDS::ReaderLifespanQosPolicy      &a, const DDS::ReaderLifespanQosPolicy      &b);
    OS_DCPS_API bool operator!=( const DDS::ReaderLifespanQosPolicy      &a, const DDS::ReaderLifespanQosPolicy      &b);

    OS_DCPS_API bool operator==( const DDS::ReliabilityQosPolicy         &a, const DDS::ReliabilityQosPolicy         &b);
    OS_DCPS_API bool operator!=( const DDS::ReliabilityQosPolicy         &a, const DDS::ReliabilityQosPolicy         &b);

    OS_DCPS_API bool operator==( const DDS::ResourceLimitsQosPolicy      &a, const DDS::ResourceLimitsQosPolicy      &b);
    OS_DCPS_API bool operator!=( const DDS::ResourceLimitsQosPolicy      &a, const DDS::ResourceLimitsQosPolicy      &b);

    OS_DCPS_API bool operator==( const DDS::ShareQosPolicy               &a, const DDS::ShareQosPolicy               &b);
    OS_DCPS_API bool operator!=( const DDS::ShareQosPolicy               &a, const DDS::ShareQosPolicy               &b);

    OS_DCPS_API bool operator==( const DDS::SchedulingClassQosPolicy     &a, const DDS::SchedulingClassQosPolicy     &b);
    OS_DCPS_API bool operator!=( const DDS::SchedulingClassQosPolicy     &a, const DDS::SchedulingClassQosPolicy     &b);

    OS_DCPS_API bool operator==( const DDS::SchedulingPriorityQosPolicy  &a, const DDS::SchedulingPriorityQosPolicy  &b);
    OS_DCPS_API bool operator!=( const DDS::SchedulingPriorityQosPolicy  &a, const DDS::SchedulingPriorityQosPolicy  &b);

    OS_DCPS_API bool operator==( const DDS::SchedulingQosPolicy          &a, const DDS::SchedulingQosPolicy          &b);
    OS_DCPS_API bool operator!=( const DDS::SchedulingQosPolicy          &a, const DDS::SchedulingQosPolicy          &b);

    OS_DCPS_API bool operator==( const DDS::SubscriptionKeyQosPolicy     &a, const DDS::SubscriptionKeyQosPolicy     &b);
    OS_DCPS_API bool operator!=( const DDS::SubscriptionKeyQosPolicy     &a, const DDS::SubscriptionKeyQosPolicy     &b);

    OS_DCPS_API bool operator==( const DDS::TimeBasedFilterQosPolicy     &a, const DDS::TimeBasedFilterQosPolicy     &b);
    OS_DCPS_API bool operator!=( const DDS::TimeBasedFilterQosPolicy     &a, const DDS::TimeBasedFilterQosPolicy     &b);

    OS_DCPS_API bool operator==( const DDS::TopicDataQosPolicy           &a, const DDS::TopicDataQosPolicy           &b);
    OS_DCPS_API bool operator!=( const DDS::TopicDataQosPolicy           &a, const DDS::TopicDataQosPolicy           &b);

    OS_DCPS_API bool operator==( const DDS::TransportPriorityQosPolicy   &a, const DDS::TransportPriorityQosPolicy   &b);
    OS_DCPS_API bool operator!=( const DDS::TransportPriorityQosPolicy   &a, const DDS::TransportPriorityQosPolicy   &b);

    OS_DCPS_API bool operator==( const DDS::UserDataQosPolicy            &a, const DDS::UserDataQosPolicy            &b);
    OS_DCPS_API bool operator!=( const DDS::UserDataQosPolicy            &a, const DDS::UserDataQosPolicy            &b);

    OS_DCPS_API bool operator==( const DDS::ViewKeyQosPolicy             &a, const DDS::ViewKeyQosPolicy             &b);
    OS_DCPS_API bool operator!=( const DDS::ViewKeyQosPolicy             &a, const DDS::ViewKeyQosPolicy             &b);

    OS_DCPS_API bool operator==( const DDS::WriterDataLifecycleQosPolicy &a, const DDS::WriterDataLifecycleQosPolicy &b);
    OS_DCPS_API bool operator!=( const DDS::WriterDataLifecycleQosPolicy &a, const DDS::WriterDataLifecycleQosPolicy &b);

    /* Overloaded QoS equality operators */
    OS_DCPS_API bool operator==( const DDS::DataReaderQos        &a,  const DDS::DataReaderQos        &b);
    OS_DCPS_API bool operator!=( const DDS::DataReaderQos        &a,  const DDS::DataReaderQos        &b);

    OS_DCPS_API bool operator==( const DDS::DataReaderViewQos    &a,  const DDS::DataReaderViewQos    &b);
    OS_DCPS_API bool operator!=( const DDS::DataReaderViewQos    &a,  const DDS::DataReaderViewQos    &b);

    OS_DCPS_API bool operator==( const DDS::DataWriterQos        &a,  const DDS::DataWriterQos        &b);
    OS_DCPS_API bool operator!=( const DDS::DataWriterQos        &a,  const DDS::DataWriterQos        &b);

    OS_DCPS_API bool operator==( const DDS::DomainParticipantQos &a,  const DDS::DomainParticipantQos &b);
    OS_DCPS_API bool operator!=( const DDS::DomainParticipantQos &a,  const DDS::DomainParticipantQos &b);

    OS_DCPS_API bool operator==( const DDS::PublisherQos         &a,  const DDS::PublisherQos         &b);
    OS_DCPS_API bool operator!=( const DDS::PublisherQos         &a,  const DDS::PublisherQos         &b);

    OS_DCPS_API bool operator==( const DDS::SubscriberQos        &a,  const DDS::SubscriberQos        &b);
    OS_DCPS_API bool operator!=( const DDS::SubscriberQos        &a,  const DDS::SubscriberQos        &b);

    OS_DCPS_API bool operator==( const DDS::TopicQos             &a,  const DDS::TopicQos             &b);
    OS_DCPS_API bool operator!=( const DDS::TopicQos             &a,  const DDS::TopicQos             &b);
}

#endif /* QOSUTILS */
