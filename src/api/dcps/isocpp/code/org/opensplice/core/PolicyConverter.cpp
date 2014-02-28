/*
*                         OpenSplice DDS
*
*   This software and documentation are Copyright 2006 to 2012 PrismTech
*   Limited and its licensees. All rights reserved. See file:
*
*                     $OSPL_HOME/LICENSE
*
*   for full copyright notice and license terms.
*
*/


/**
 * @file
 */

#include <org/opensplice/core/policy/PolicyConverter.hpp>
#include <iostream>

/////////////////////////////////////////////////////////////////////////////

dds::core::policy::Deadline
org::opensplice::core::policy::convertPolicy(const DDS::DeadlineQosPolicy& from)
{
    return dds::core::policy::Deadline(dds::core::Duration(from.period.sec, from.period.nanosec));
}

DDS::DeadlineQosPolicy
org::opensplice::core::policy::convertPolicy(const dds::core::policy::Deadline& from)
{
    DDS::DeadlineQosPolicy to;
    to.period.sec = static_cast<int32_t>(from.period().sec());
    to.period.nanosec = from.period().nanosec();
    return to;
}

/////////////////////////////////////////////////////////////////////////////

dds::core::policy::DestinationOrder
org::opensplice::core::policy::convertPolicy(const DDS::DestinationOrderQosPolicy& from)
{
    if(from.kind == DDS::BY_RECEPTION_TIMESTAMP_DESTINATIONORDER_QOS)
    {
        return dds::core::policy::DestinationOrder::ReceptionTimestamp();
    }
    return dds::core::policy::DestinationOrder::SourceTimestamp();
}

DDS::DestinationOrderQosPolicy
org::opensplice::core::policy::convertPolicy(const dds::core::policy::DestinationOrder& from)
{
    DDS::DestinationOrderQosPolicy to;
    if(from.kind() == dds::core::policy::DestinationOrderKind::BY_RECEPTION_TIMESTAMP)
    {
        to.kind = DDS::BY_RECEPTION_TIMESTAMP_DESTINATIONORDER_QOS;
    }
    else
    {
        to.kind = DDS::BY_SOURCE_TIMESTAMP_DESTINATIONORDER_QOS;
    }
    return to;
}

/////////////////////////////////////////////////////////////////////////////

dds::core::policy::Durability
org::opensplice::core::policy::convertPolicy(const DDS::DurabilityQosPolicy& from)
{
    if(from.kind == DDS::TRANSIENT_LOCAL_DURABILITY_QOS)
    {
        return dds::core::policy::Durability::TransientLocal();
    }
    else if(from.kind == DDS::TRANSIENT_DURABILITY_QOS)
    {
        return dds::core::policy::Durability::Transient();
    }
    else if(from.kind == DDS::PERSISTENT_DURABILITY_QOS)
    {
        return dds::core::policy::Durability::Persistent();
    }
    else
    {
        return dds::core::policy::Durability::Volatile();
    }
}

DDS::DurabilityQosPolicy
org::opensplice::core::policy::convertPolicy(const dds::core::policy::Durability& from)
{
    DDS::DurabilityQosPolicy to;
    if(from.kind() == dds::core::policy::DurabilityKind::TRANSIENT_LOCAL)
    {
        to.kind = DDS::TRANSIENT_LOCAL_DURABILITY_QOS;
    }
    else if(from.kind() == dds::core::policy::DurabilityKind::TRANSIENT)
    {
        to.kind = DDS::TRANSIENT_DURABILITY_QOS;
    }
    else if(from.kind() == dds::core::policy::DurabilityKind::PERSISTENT)
    {
        to.kind = DDS::PERSISTENT_DURABILITY_QOS;
    }
    else
    {
        to.kind = DDS::VOLATILE_DURABILITY_QOS;
    }
    return to;
}

/////////////////////////////////////////////////////////////////////////////
#ifdef OMG_DDS_PERSISTENCE_SUPPORT

dds::core::policy::DurabilityService
org::opensplice::core::policy::convertPolicy(const DDS::DurabilityServiceQosPolicy& from)
{
    dds::core::policy::DurabilityService to(
        dds::core::Duration(from.service_cleanup_delay.sec, from.service_cleanup_delay.nanosec),
        dds::core::policy::HistoryKind::KEEP_LAST,
        from.history_depth,
        from.max_samples,
        from.max_instances,
        from.max_samples_per_instance);

    if(from.history_kind == DDS::KEEP_ALL_HISTORY_QOS)
    {
        to.history_kind(dds::core::policy::HistoryKind::KEEP_ALL);
    }

    return to;
}

DDS::DurabilityServiceQosPolicy
org::opensplice::core::policy::convertPolicy(const dds::core::policy::DurabilityService& from)
{
    DDS::DurabilityServiceQosPolicy to;
    to.service_cleanup_delay.sec = static_cast<int32_t>(from.service_cleanup_delay().sec());
    to.service_cleanup_delay.nanosec = from.service_cleanup_delay().nanosec();
    if(from.history_kind() == dds::core::policy::HistoryKind::KEEP_LAST)
    {
        to.history_kind = DDS::KEEP_LAST_HISTORY_QOS;
    }
    else
    {
        to.history_kind = DDS::KEEP_ALL_HISTORY_QOS;
    }
    to.history_depth = from.history_depth();
    to.max_samples = from.max_samples();
    to.max_instances = from.max_instances();
    to.max_samples_per_instance = from.max_samples_per_instance();
    return to;
}

#endif  // OMG_DDS_PERSISTENCE_SUPPORT
/////////////////////////////////////////////////////////////////////////////

dds::core::policy::EntityFactory
org::opensplice::core::policy::convertPolicy(const DDS::EntityFactoryQosPolicy& from)
{
    if(!from.autoenable_created_entities)
    {
        return dds::core::policy::EntityFactory::ManuallyEnable();
    }
    return dds::core::policy::EntityFactory::AutoEnable();
}

DDS::EntityFactoryQosPolicy
org::opensplice::core::policy::convertPolicy(const dds::core::policy::EntityFactory& from)
{
    DDS::EntityFactoryQosPolicy to;
    if(from.autoenable_created_entities())
    {
        to.autoenable_created_entities = true;
    }
    else
    {
        to.autoenable_created_entities = false;
    }
    return to;
}

/////////////////////////////////////////////////////////////////////////////

dds::core::policy::GroupData
org::opensplice::core::policy::convertPolicy(const DDS::GroupDataQosPolicy& from)
{
    dds::core::ByteSeq seq;
    for(u_int i = 0; i < from.value.length(); i++)
    {
        seq.push_back(from.value[i]);
    }
    return dds::core::policy::GroupData(seq);
}

DDS::GroupDataQosPolicy
org::opensplice::core::policy::convertPolicy(const dds::core::policy::GroupData& from)
{
    DDS::GroupDataQosPolicy to;
    to.value.length(static_cast<DDS::ULong>(from.value().size()));
    for(unsigned int i = 0; i < from.value().size(); i++)
    {
        to.value[i] = from.value()[i];
    }
    return to;
}

/////////////////////////////////////////////////////////////////////////////

dds::core::policy::History
org::opensplice::core::policy::convertPolicy(const DDS::HistoryQosPolicy& from)
{
    if(from.kind == DDS::KEEP_ALL_HISTORY_QOS)
    {
        return dds::core::policy::History::KeepAll();
    }

    return dds::core::policy::History::KeepLast(from.depth);
}

DDS::HistoryQosPolicy
org::opensplice::core::policy::convertPolicy(const dds::core::policy::History& from)
{
    DDS::HistoryQosPolicy to;
    if(from.kind() == dds::core::policy::HistoryKind::KEEP_ALL)
    {
        to.kind = DDS::KEEP_ALL_HISTORY_QOS;
        to.depth = 1;
    }
    else
    {
        to.kind = DDS::KEEP_LAST_HISTORY_QOS;
        to.depth = from.depth();
    }
    return to;
}

/////////////////////////////////////////////////////////////////////////////

dds::core::policy::LatencyBudget
org::opensplice::core::policy::convertPolicy(const DDS::LatencyBudgetQosPolicy& from)
{
    return dds::core::policy::LatencyBudget(dds::core::Duration(from.duration.sec, from.duration.nanosec));
}

DDS::LatencyBudgetQosPolicy
org::opensplice::core::policy::convertPolicy(const dds::core::policy::LatencyBudget& from)
{
    DDS::LatencyBudgetQosPolicy to;
    to.duration.sec = static_cast<int32_t>(from.duration().sec());
    to.duration.nanosec = from.duration().nanosec();
    return to;
}

/////////////////////////////////////////////////////////////////////////////

dds::core::policy::Lifespan
org::opensplice::core::policy::convertPolicy(const DDS::LifespanQosPolicy& from)
{
    return dds::core::policy::Lifespan(dds::core::Duration(from.duration.sec, from.duration.nanosec));
}

DDS::LifespanQosPolicy
org::opensplice::core::policy::convertPolicy(const dds::core::policy::Lifespan& from)
{
    DDS::LifespanQosPolicy to;
    to.duration.sec = static_cast<int32_t>(from.duration().sec());
    to.duration.nanosec = from.duration().nanosec();
    return to;
}

/////////////////////////////////////////////////////////////////////////////

dds::core::policy::Liveliness
org::opensplice::core::policy::convertPolicy(const DDS::LivelinessQosPolicy& from)
{
    if(from.kind == DDS::MANUAL_BY_PARTICIPANT_LIVELINESS_QOS)
    {
        return dds::core::policy::Liveliness(dds::core::policy::LivelinessKind::MANUAL_BY_PARTICIPANT,
                                             dds::core::Duration(from.lease_duration.sec, from.lease_duration.nanosec));
    }
    else if(from.kind == DDS::MANUAL_BY_TOPIC_LIVELINESS_QOS)
    {
        return dds::core::policy::Liveliness(dds::core::policy::LivelinessKind::MANUAL_BY_TOPIC,
                                             dds::core::Duration(from.lease_duration.sec, from.lease_duration.nanosec));
    }
    else
    {
        return dds::core::policy::Liveliness(dds::core::policy::LivelinessKind::AUTOMATIC,
                                             dds::core::Duration(from.lease_duration.sec, from.lease_duration.nanosec));
    }
}

DDS::LivelinessQosPolicy
org::opensplice::core::policy::convertPolicy(const dds::core::policy::Liveliness& from)
{
    DDS::LivelinessQosPolicy to;
    if(from.kind() == dds::core::policy::LivelinessKind::MANUAL_BY_PARTICIPANT)
    {
        to.kind = DDS::MANUAL_BY_PARTICIPANT_LIVELINESS_QOS;
    }
    else if(from.kind() == dds::core::policy::LivelinessKind::MANUAL_BY_TOPIC)
    {
        to.kind = DDS::MANUAL_BY_TOPIC_LIVELINESS_QOS;
    }
    else
    {
        to.kind = DDS::AUTOMATIC_LIVELINESS_QOS;
    }
    to.lease_duration.sec = static_cast<int32_t>(from.lease_duration().sec());
    to.lease_duration.nanosec = from.lease_duration().nanosec();
    return to;
}

/////////////////////////////////////////////////////////////////////////////
#ifdef OMG_DDS_OWNERSHIP_SUPPORT

dds::core::policy::Ownership
org::opensplice::core::policy::convertPolicy(const DDS::OwnershipQosPolicy& from)
{
    if(from.kind == DDS::EXCLUSIVE_OWNERSHIP_QOS)
    {
        return dds::core::policy::Ownership::Exclusive();
    }
    else
    {
        return dds::core::policy::Ownership::Shared();
    }
}

DDS::OwnershipQosPolicy
org::opensplice::core::policy::convertPolicy(const dds::core::policy::Ownership& from)
{
    DDS::OwnershipQosPolicy to;
    if(from.kind() == dds::core::policy::OwnershipKind::EXCLUSIVE)
    {
        to.kind = DDS::EXCLUSIVE_OWNERSHIP_QOS;
    }
    else
    {
        to.kind = DDS::SHARED_OWNERSHIP_QOS;
    }
    return to;
}

/////////////////////////////////////////////////////////////////////////////

dds::core::policy::OwnershipStrength
org::opensplice::core::policy::convertPolicy(const DDS::OwnershipStrengthQosPolicy& from)
{
    return dds::core::policy::OwnershipStrength(from.value);
}

DDS::OwnershipStrengthQosPolicy
org::opensplice::core::policy::convertPolicy(const dds::core::policy::OwnershipStrength& from)
{
    DDS::OwnershipStrengthQosPolicy to;
    to.value = from.value();
    return to;
}

#endif  // OMG_DDS_OWNERSHIP_SUPPORT
/////////////////////////////////////////////////////////////////////////////

dds::core::policy::Partition
org::opensplice::core::policy::convertPolicy(const DDS::PartitionQosPolicy& from)
{
    dds::core::StringSeq ss;
    for(u_int i = 0; i < from.name.length(); i++)
    {
        ss.push_back(from.name[i]);
    }
    return dds::core::policy::Partition(ss);
}

DDS::PartitionQosPolicy
org::opensplice::core::policy::convertPolicy(const dds::core::policy::Partition& from)
{
    DDS::PartitionQosPolicy to;
    to.name.length(static_cast<DDS::ULong>(from.name().size()));
    for(unsigned int i = 0; i < from.name().size(); i++)
    {
        to.name[i] = from.name()[i].c_str();
    }
    return to;
}

/////////////////////////////////////////////////////////////////////////////

dds::core::policy::Presentation
org::opensplice::core::policy::convertPolicy(const DDS::PresentationQosPolicy& from)
{
    if(from.access_scope == DDS::TOPIC_PRESENTATION_QOS)
    {
        /** @internal @bug OSPL-918 DDS::Boolean is not (yet!) a bool
        @todo Remove fudge when OSPL-918 fixed
        @see http://jira.prismtech.com:8080/browse/OSPL-918 */
        return dds::core::policy::Presentation::TopicAccessScope((from.coherent_access ? true : false),
                (from.ordered_access ? true : false));
    }
    else if(from.access_scope == DDS::GROUP_PRESENTATION_QOS)
    {
        return dds::core::policy::Presentation::GroupAccessScope((from.coherent_access ? true : false),
                (from.ordered_access ? true : false));
    }
    else
    {
        return dds::core::policy::Presentation::InstanceAccessScope((from.coherent_access ? true : false),
                (from.ordered_access ? true : false));
    }
}

DDS::PresentationQosPolicy
org::opensplice::core::policy::convertPolicy(const dds::core::policy::Presentation& from)
{
    DDS::PresentationQosPolicy to;
    if(from.access_scope() == dds::core::policy::PresentationAccessScopeKind::TOPIC)
    {
        to.access_scope = DDS::TOPIC_PRESENTATION_QOS;
        to.coherent_access = from.coherent_access();
        to.ordered_access = from.ordered_access();
    }
    else if(from.access_scope() == dds::core::policy::PresentationAccessScopeKind::GROUP)
    {
        to.access_scope = DDS::GROUP_PRESENTATION_QOS;
        to.coherent_access = from.coherent_access();
        to.ordered_access = from.ordered_access();
    }
    else
    {
        to.access_scope = DDS::INSTANCE_PRESENTATION_QOS;
        to.coherent_access = from.coherent_access();
        to.ordered_access = from.ordered_access();
    }
    return to;
}

/////////////////////////////////////////////////////////////////////////////

dds::core::policy::ReaderDataLifecycle
org::opensplice::core::policy::convertPolicy(const DDS::ReaderDataLifecycleQosPolicy& from)
{
    return dds::core::policy::ReaderDataLifecycle(
               dds::core::Duration(from.autopurge_nowriter_samples_delay.sec, from.autopurge_nowriter_samples_delay.nanosec),
               dds::core::Duration(from.autopurge_disposed_samples_delay.sec, from.autopurge_disposed_samples_delay.nanosec));
}

DDS::ReaderDataLifecycleQosPolicy
org::opensplice::core::policy::convertPolicy(const dds::core::policy::ReaderDataLifecycle& from)
{
    DDS::ReaderDataLifecycleQosPolicy to;
    to.autopurge_nowriter_samples_delay.sec = static_cast<int32_t>(from.autopurge_nowriter_samples_delay().sec());
    to.autopurge_nowriter_samples_delay.nanosec = from.autopurge_nowriter_samples_delay().nanosec();
    to.autopurge_disposed_samples_delay.sec = static_cast<int32_t>(from.autopurge_disposed_samples_delay().sec());
    to.autopurge_disposed_samples_delay.nanosec = from.autopurge_disposed_samples_delay().nanosec();
    to.enable_invalid_samples = true;
    to.invalid_sample_visibility.kind = DDS::MINIMUM_INVALID_SAMPLES;
    return to;
}

///////////////////////////////////////////////////////////////////////////////

dds::core::policy::Reliability
org::opensplice::core::policy::convertPolicy(const DDS::ReliabilityQosPolicy& from)
{
    if(from.kind == DDS::BEST_EFFORT_RELIABILITY_QOS)
    {
        return dds::core::policy::Reliability::BestEffort();
    }

    dds::core::Duration d(from.max_blocking_time.sec, from.max_blocking_time.nanosec);
    return dds::core::policy::Reliability::Reliable(d);
}

DDS::ReliabilityQosPolicy
org::opensplice::core::policy::convertPolicy(const dds::core::policy::Reliability& from)
{
    DDS::ReliabilityQosPolicy to;
    if(from.kind() == dds::core::policy::ReliabilityKind::BEST_EFFORT)
    {
        to.kind = DDS::BEST_EFFORT_RELIABILITY_QOS;
    }
    else
    {
        to.kind = DDS::RELIABLE_RELIABILITY_QOS;
    }
    to.synchronous = false;
    to.max_blocking_time.sec = (int32_t)from.max_blocking_time().sec();
    to.max_blocking_time.nanosec = (uint32_t)from.max_blocking_time().nanosec();
    return to;
}

/////////////////////////////////////////////////////////////////////////////

dds::core::policy::ResourceLimits
org::opensplice::core::policy::convertPolicy(const DDS::ResourceLimitsQosPolicy& from)
{
    return dds::core::policy::ResourceLimits(from.max_samples, from.max_instances, from.max_samples_per_instance);
}

DDS::ResourceLimitsQosPolicy
org::opensplice::core::policy::convertPolicy(const dds::core::policy::ResourceLimits& from)
{
    DDS::ResourceLimitsQosPolicy to;
    to.max_samples = from.max_samples();
    to.max_instances = from.max_instances();
    to.max_samples_per_instance = from.max_samples_per_instance();
    return to;
}

/////////////////////////////////////////////////////////////////////////////

dds::core::policy::TimeBasedFilter
org::opensplice::core::policy::convertPolicy(const DDS::TimeBasedFilterQosPolicy& from)
{
    return dds::core::policy::TimeBasedFilter(
               dds::core::Duration(from.minimum_separation.sec, from.minimum_separation.nanosec));
}

DDS::TimeBasedFilterQosPolicy
org::opensplice::core::policy::convertPolicy(const dds::core::policy::TimeBasedFilter& from)
{
    DDS::TimeBasedFilterQosPolicy to;
    to.minimum_separation.sec = static_cast<int32_t>(from.minimum_separation().sec());
    to.minimum_separation.nanosec = from.minimum_separation().nanosec();
    return to;
}

/////////////////////////////////////////////////////////////////////////////

dds::core::policy::TopicData
org::opensplice::core::policy::convertPolicy(const DDS::TopicDataQosPolicy& from)
{
    dds::core::ByteSeq seq;
    for(u_int i = 0; i < from.value.length(); i++)
    {
        seq.push_back(from.value[i]);
    }
    return dds::core::policy::TopicData(seq);
}

DDS::TopicDataQosPolicy
org::opensplice::core::policy::convertPolicy(const dds::core::policy::TopicData& from)
{
    DDS::TopicDataQosPolicy to;
    to.value.length(static_cast<DDS::ULong>(from.value().size()));
    for(unsigned int i = 0; i < from.value().size(); i++)
    {
        to.value[i] = from.value()[i];
    }
    return to;
}

/////////////////////////////////////////////////////////////////////////////

dds::core::policy::TransportPriority
org::opensplice::core::policy::convertPolicy(const DDS::TransportPriorityQosPolicy& from)
{
    return dds::core::policy::TransportPriority(from.value);
}

DDS::TransportPriorityQosPolicy
org::opensplice::core::policy::convertPolicy(const dds::core::policy::TransportPriority& from)
{
    DDS::TransportPriorityQosPolicy to;
    to.value = from.value();
    return to;
}

/////////////////////////////////////////////////////////////////////////////

dds::core::policy::UserData
org::opensplice::core::policy::convertPolicy(const DDS::UserDataQosPolicy& from)
{
    dds::core::ByteSeq seq;
    for(u_int i = 0; i < from.value.length(); i++)
    {
        seq.push_back(from.value[i]);
    }
    return dds::core::policy::UserData(seq);
}

DDS::UserDataQosPolicy
org::opensplice::core::policy::convertPolicy(const dds::core::policy::UserData& from)
{
    DDS::UserDataQosPolicy to;
    to.value.length(static_cast<DDS::ULong>(from.value().size()));
    for(unsigned int i = 0; i < from.value().size(); i++)
    {
        to.value[i] = from.value()[i];
    }
    return to;
}

/////////////////////////////////////////////////////////////////////////////

dds::core::policy::WriterDataLifecycle
org::opensplice::core::policy::convertPolicy(const DDS::WriterDataLifecycleQosPolicy& from)
{
    return dds::core::policy::WriterDataLifecycle(from.autodispose_unregistered_instances ? true : false);
}

DDS::WriterDataLifecycleQosPolicy
org::opensplice::core::policy::convertPolicy(const dds::core::policy::WriterDataLifecycle& from)
{
    DDS::WriterDataLifecycleQosPolicy to;
    to.autodispose_unregistered_instances = from.autodispose_unregistered_instances();
    to.autopurge_suspended_samples_delay.sec = DDS::DURATION_INFINITE_SEC;
    to.autopurge_suspended_samples_delay.nanosec = DDS::DURATION_INFINITE_NSEC;
    to.autounregister_instance_delay.sec = DDS::DURATION_INFINITE_SEC;
    to.autounregister_instance_delay.nanosec = DDS::DURATION_INFINITE_NSEC;
    return to;
}

/////////////////////////////////////////////////////////////////////////////
