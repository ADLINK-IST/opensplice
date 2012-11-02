/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2011 PrismTech
 *   Limited and its licensees. All rights reserved. See file:
 *
 *                     $OSPL_HOME/LICENSE
 *
 *   for full copyright notice and license terms.
 *
 */
#include "ccpp_QosUtils.h"
#include "orb_abstraction.h"

namespace DDS
{
    static const DDS::UserDataQosPolicy DEFAULT_USERDATA_QOSPOLICY = {
        DDS::octSeq()
    };

    static const DDS::TopicDataQosPolicy DEFAULT_TOPICDATA_QOSPOLICY = {
        DDS::octSeq()
    };

    static const DDS::GroupDataQosPolicy DEFAULT_GROUPDATA_QOSPOLICY = {
        DDS::octSeq()
    };

    static const DDS::TransportPriorityQosPolicy DEFAULT_TRANSPORTPRIORITY_QOSPOLICY = { 0L };

    static const DDS::LifespanQosPolicy DEFAULT_LIFESPAN_QOSPOLICY = {
        {DURATION_INFINITE_SEC, DURATION_INFINITE_NSEC}
    };

    static const DDS::DurabilityQosPolicy DEFAULT_DURABILITY_QOSPOLICY = {
        VOLATILE_DURABILITY_QOS
    };

    static const DDS::DurabilityServiceQosPolicy DEFAULT_DURABILITYSERVICE_QOSPOLICY = {
        {DURATION_ZERO_SEC, DURATION_ZERO_NSEC},
        KEEP_LAST_HISTORY_QOS,
        1L,
        LENGTH_UNLIMITED,
        LENGTH_UNLIMITED,
        LENGTH_UNLIMITED
    };

    static const DDS::PresentationQosPolicy DEFAULT_PRESENTATION_QOSPOLICY = {
        INSTANCE_PRESENTATION_QOS,
        false,
        false
    };

    static const DDS::DeadlineQosPolicy DEFAULT_DEADLINE_QOSPOLICY = {
        {DURATION_INFINITE_SEC, DURATION_INFINITE_NSEC}
    };

    static const DDS::LatencyBudgetQosPolicy DEFAULT_LATENCYBUDGET_QOSPOLICY = {
        {DURATION_ZERO_SEC, DURATION_ZERO_NSEC}
    };

    static const DDS::OwnershipQosPolicy DEFAULT_OWNERSHIP_QOSPOLICY = {
        SHARED_OWNERSHIP_QOS
    };

    static const DDS::OwnershipStrengthQosPolicy DEFAULT_OWNERSHIPSTRENGTH_QOSPOLICY  = { 0L };

    static const DDS::LivelinessQosPolicy DEFAULT_LIVELINESS_QOSPOLICY = {
        AUTOMATIC_LIVELINESS_QOS,
        {DURATION_INFINITE_SEC, DURATION_INFINITE_NSEC}
    };

    static const DDS::TimeBasedFilterQosPolicy DEFAULT_TIMEBASEDFILTER_QOSPOLICY = {
        {DURATION_ZERO_SEC, DURATION_ZERO_NSEC}
    };

    static const DDS::PartitionQosPolicy DEFAULT_PARTITION_QOSPOLICY = {
        DDS::StringSeq()
    };

    static const DDS::ReliabilityQosPolicy DEFAULT_RELIABILITY_QOSPOLICY = {
        BEST_EFFORT_RELIABILITY_QOS,
        {DURATION_INFINITE_SEC, DURATION_INFINITE_NSEC}
    };

    static const DDS::DestinationOrderQosPolicy DEFAULT_DESTINATIONORDER_QOSPOLICY = {
        BY_RECEPTION_TIMESTAMP_DESTINATIONORDER_QOS
    };

    static const DDS::HistoryQosPolicy DEFAULT_HISTORY_QOSPOLICY = {
        KEEP_LAST_HISTORY_QOS,
        1L
    };

    static const DDS::ResourceLimitsQosPolicy DEFAULT_RESOURCELIMITS_QOSPOLICY = {
        LENGTH_UNLIMITED,
        LENGTH_UNLIMITED,
        LENGTH_UNLIMITED
    };

    static const DDS::EntityFactoryQosPolicy DEFAULT_ENTITYFACTORY_QOSPOLICY = {
        true
    };

    static const DDS::WriterDataLifecycleQosPolicy DEFAULT_WRITERDATALIFECYCLE_QOSPOLICY= {
        true,
        {DURATION_INFINITE_SEC, DURATION_INFINITE_NSEC},
        {DURATION_INFINITE_SEC, DURATION_INFINITE_NSEC}
    };

    static const DDS::ReaderDataLifecycleQosPolicy DEFAULT_READERDATALIFECYCLE_QOSPOLICY= {
        {DURATION_INFINITE_SEC, DURATION_INFINITE_NSEC},
        {DURATION_INFINITE_SEC, DURATION_INFINITE_NSEC},
        true
    };

    static const DDS::SchedulingQosPolicy DEFAULT_SCHEDULING_QOSPOLICY= {
        { SCHEDULE_DEFAULT },
        { PRIORITY_RELATIVE },
        0
    };

	static const DDS::SubscriptionKeyQosPolicy DEFAULT_SUBSCRIPTIONKEY_QOSPOLICY= {
		false,
		DDS::StringSeq()
	};

	static const DDS::ReaderLifespanQosPolicy DEFAULT_READERLIFESPAN_QOSPOLICY= {
		false,
        {DURATION_INFINITE_SEC, DURATION_INFINITE_NSEC}
	};

	static const DDS::ShareQosPolicy DEFAULT_SHARE_QOSPOLICY= {
		"",
		false
	};

	static const DDS::ViewKeyQosPolicy DEFAULT_VIEWKEY_QOSPOLICY= {
	    false,
	    DDS::StringSeq()
	};


    static const DDS::DomainParticipantFactoryQos * const
    initializeParticipantFactoryQos()
    {
        DDS::DomainParticipantFactoryQos *qos = new DDS::DomainParticipantFactoryQos();
        qos->entity_factory         = DEFAULT_ENTITYFACTORY_QOSPOLICY;
        return qos;
    }

    static const DDS::DomainParticipantQos * const
    initializeParticipantQos()
    {
        DDS::DomainParticipantQos *qos = new DDS::DomainParticipantQos();
        qos->user_data              = DEFAULT_USERDATA_QOSPOLICY;
        qos->entity_factory         = DEFAULT_ENTITYFACTORY_QOSPOLICY;
	qos->watchdog_scheduling    = DEFAULT_SCHEDULING_QOSPOLICY;
	qos->listener_scheduling    = DEFAULT_SCHEDULING_QOSPOLICY;
        return qos;
    }

    static const DDS::TopicQos * const
    initializeTopicQos()
    {
        DDS::TopicQos *qos          = new DDS::TopicQos();
        qos->topic_data             = DEFAULT_TOPICDATA_QOSPOLICY;
        qos->durability             = DEFAULT_DURABILITY_QOSPOLICY;
        qos->durability_service     = DEFAULT_DURABILITYSERVICE_QOSPOLICY;
        qos->deadline               = DEFAULT_DEADLINE_QOSPOLICY;
        qos->latency_budget         = DEFAULT_LATENCYBUDGET_QOSPOLICY;
        qos->liveliness             = DEFAULT_LIVELINESS_QOSPOLICY;
        qos->reliability            = DEFAULT_RELIABILITY_QOSPOLICY;
        qos->destination_order      = DEFAULT_DESTINATIONORDER_QOSPOLICY;
        qos->history                = DEFAULT_HISTORY_QOSPOLICY;
        qos->resource_limits        = DEFAULT_RESOURCELIMITS_QOSPOLICY;
        qos->transport_priority     = DEFAULT_TRANSPORTPRIORITY_QOSPOLICY;
        qos->lifespan               = DEFAULT_LIFESPAN_QOSPOLICY;
        qos->ownership              = DEFAULT_OWNERSHIP_QOSPOLICY;
        return qos;
    }

    static const DDS::PublisherQos * const
    initializePublisherQos()
    {
        DDS::PublisherQos *qos      = new DDS::PublisherQos();
        qos->presentation           = DEFAULT_PRESENTATION_QOSPOLICY;
        qos->partition              = DEFAULT_PARTITION_QOSPOLICY;
        qos->group_data             = DEFAULT_GROUPDATA_QOSPOLICY;
        qos->entity_factory         = DEFAULT_ENTITYFACTORY_QOSPOLICY;
        return qos;
    }

    static const DDS::SubscriberQos * const
    initializeSubscriberQos()
    {
        DDS::SubscriberQos *qos     = new DDS::SubscriberQos();
        qos->presentation           = DEFAULT_PRESENTATION_QOSPOLICY;
        qos->partition              = DEFAULT_PARTITION_QOSPOLICY;
        qos->group_data             = DEFAULT_GROUPDATA_QOSPOLICY;
        qos->entity_factory         = DEFAULT_ENTITYFACTORY_QOSPOLICY;
        qos->share                  = DEFAULT_SHARE_QOSPOLICY;
        return qos;
    }

    static const DDS::DataReaderQos * const
    initializeDataReaderQos()
    {
        DDS::DataReaderQos *qos     = new DDS::DataReaderQos();
        qos->durability             = DEFAULT_DURABILITY_QOSPOLICY;
        qos->deadline               = DEFAULT_DEADLINE_QOSPOLICY;
        qos->latency_budget         = DEFAULT_LATENCYBUDGET_QOSPOLICY;
        qos->liveliness             = DEFAULT_LIVELINESS_QOSPOLICY;
        qos->reliability            = DEFAULT_RELIABILITY_QOSPOLICY;
        qos->destination_order      = DEFAULT_DESTINATIONORDER_QOSPOLICY;
        qos->history                = DEFAULT_HISTORY_QOSPOLICY;
        qos->resource_limits        = DEFAULT_RESOURCELIMITS_QOSPOLICY;
        qos->user_data              = DEFAULT_USERDATA_QOSPOLICY;
		qos->ownership              = DEFAULT_OWNERSHIP_QOSPOLICY;
        qos->time_based_filter      = DEFAULT_TIMEBASEDFILTER_QOSPOLICY;
        qos->reader_data_lifecycle  = DEFAULT_READERDATALIFECYCLE_QOSPOLICY;
		qos->subscription_keys      = DEFAULT_SUBSCRIPTIONKEY_QOSPOLICY;
		qos->reader_lifespan        = DEFAULT_READERLIFESPAN_QOSPOLICY;
		qos->share                  = DEFAULT_SHARE_QOSPOLICY;
        return qos;
    }

    static const DDS::DataReaderViewQos * const
    initializeDataReaderViewQos()
    {
        DDS::DataReaderViewQos *qos = new DDS::DataReaderViewQos();
        qos->view_keys = DEFAULT_VIEWKEY_QOSPOLICY;
        return qos;
    }

    static const DDS::DataWriterQos * const
    initializeDataWriterQos()
    {
        DDS::DataWriterQos *qos     = new DDS::DataWriterQos();
        qos->durability             = DEFAULT_DURABILITY_QOSPOLICY;
        qos->deadline               = DEFAULT_DEADLINE_QOSPOLICY;
        qos->latency_budget         = DEFAULT_LATENCYBUDGET_QOSPOLICY;
        qos->liveliness             = DEFAULT_LIVELINESS_QOSPOLICY;
        qos->reliability            = DEFAULT_RELIABILITY_QOSPOLICY;
        qos->destination_order      = DEFAULT_DESTINATIONORDER_QOSPOLICY;
        qos->history                = DEFAULT_HISTORY_QOSPOLICY;
        qos->resource_limits        = DEFAULT_RESOURCELIMITS_QOSPOLICY;
        qos->transport_priority     = DEFAULT_TRANSPORTPRIORITY_QOSPOLICY;
        qos->lifespan               = DEFAULT_LIFESPAN_QOSPOLICY;
        qos->user_data              = DEFAULT_USERDATA_QOSPOLICY;
        qos->ownership              = DEFAULT_OWNERSHIP_QOSPOLICY;
        qos->ownership_strength     = DEFAULT_OWNERSHIPSTRENGTH_QOSPOLICY;
        qos->writer_data_lifecycle  = DEFAULT_WRITERDATALIFECYCLE_QOSPOLICY;
        return qos;
    }

            //static const DDS::DefaultQos UniqueInstance;

    const DDS::DomainParticipantFactoryQos * const DDS::DefaultQos::ParticipantFactoryQosDefault      = DDS::initializeParticipantFactoryQos();
    const DDS::DomainParticipantQos * const DDS::DefaultQos::ParticipantQosDefault      = DDS::initializeParticipantQos();
    const DDS::TopicQos             * const DDS::DefaultQos::TopicQosDefault            = DDS::initializeTopicQos();
    const DDS::PublisherQos         * const DDS::DefaultQos::PublisherQosDefault        = DDS::initializePublisherQos();
    const DDS::SubscriberQos        * const DDS::DefaultQos::SubscriberQosDefault       = DDS::initializeSubscriberQos();
    const DDS::DataReaderQos        * const DDS::DefaultQos::DataReaderQosDefault       = DDS::initializeDataReaderQos();
    const DDS::DataReaderQos        * const DDS::DefaultQos::DataReaderQosUseTopicQos   = DDS::initializeDataReaderQos();
    const DDS::DataReaderViewQos    * const DDS::DefaultQos::DataReaderViewQosDefault   = DDS::initializeDataReaderViewQos();
    const DDS::DataWriterQos        * const DDS::DefaultQos::DataWriterQosDefault       = DDS::initializeDataWriterQos();
    const DDS::DataWriterQos        * const DDS::DefaultQos::DataWriterQosUseTopicQos   = DDS::initializeDataWriterQos();

}



//policies conversions

void DDS::ccpp_UserDataQosPolicy_copyIn( const DDS::UserDataQosPolicy &from,
        gapi_userDataQosPolicy &to )
{
    DDS::ccpp_sequenceCopyIn<DDS::octSeq, CORBA::Octet, gapi_octetSeq, gapi_octet>(from.value,
            to.value);
}

void DDS::ccpp_UserDataQosPolicy_copyOut( const gapi_userDataQosPolicy &from,
        DDS::UserDataQosPolicy &to )
{
    DDS::ccpp_sequenceCopyOut< gapi_octetSeq, gapi_octet, DDS::octSeq, CORBA::Octet >(
            from.value, to.value );
}

void DDS::ccpp_EntityFactoryQosPolicy_copyIn( const DDS::EntityFactoryQosPolicy &from,
        gapi_entityFactoryQosPolicy &to)
{
    to.autoenable_created_entities = from.autoenable_created_entities;
}

void DDS::ccpp_EntityFactoryQosPolicy_copyOut( const gapi_entityFactoryQosPolicy &from,
         DDS::EntityFactoryQosPolicy &to)
{
    to.autoenable_created_entities = from.autoenable_created_entities;
}

void DDS::ccpp_TopicDataQosPolicy_copyIn( const DDS::TopicDataQosPolicy &from,
        gapi_topicDataQosPolicy &to)
{
    DDS::ccpp_sequenceCopyIn< DDS::octSeq, CORBA::Octet, gapi_octetSeq, gapi_octet >(
            from.value, to.value );
}

void DDS::ccpp_DurabilityQosPolicy_copyIn( const DDS::DurabilityQosPolicy &from,
        gapi_durabilityQosPolicy &to)
{
    switch(from.kind)
    {
      case DDS::VOLATILE_DURABILITY_QOS:
        to.kind = GAPI_VOLATILE_DURABILITY_QOS;
        break;
      case DDS::TRANSIENT_LOCAL_DURABILITY_QOS:
        to.kind = GAPI_TRANSIENT_LOCAL_DURABILITY_QOS;
        break;
      case DDS::TRANSIENT_DURABILITY_QOS:
        to.kind = GAPI_TRANSIENT_DURABILITY_QOS;
        break;
      case DDS::PERSISTENT_DURABILITY_QOS:
        to.kind = GAPI_PERSISTENT_DURABILITY_QOS;
        break;
      default:
        // impossible to reach
        break;
    }
}

void DDS::ccpp_DurabilityServiceQosPolicy_copyIn( const DDS::DurabilityServiceQosPolicy &from,
        gapi_durabilityServiceQosPolicy &to)
{
    DDS::ccpp_Duration_copyIn(from.service_cleanup_delay, to.service_cleanup_delay);
    switch(from.history_kind)
    {
      case DDS::KEEP_LAST_HISTORY_QOS:
        to.history_kind = GAPI_KEEP_LAST_HISTORY_QOS;
        break;
      case DDS::KEEP_ALL_HISTORY_QOS:
        to.history_kind = GAPI_KEEP_ALL_HISTORY_QOS;
        break;
      default:
        // impossible to reach
        break;
    }
    to.history_depth = from.history_depth;
    to.max_samples = from.max_samples;
    to.max_instances = from.max_instances;
    to.max_samples_per_instance = from.max_samples_per_instance;
}

void DDS::ccpp_DeadlineQosPolicy_copyIn( const DDS::DeadlineQosPolicy &from,
        gapi_deadlineQosPolicy &to)
{
    DDS::ccpp_Duration_copyIn( from.period, to.period);
}

void DDS::ccpp_LatencyBudgetQosPolicy_copyIn( const DDS::LatencyBudgetQosPolicy &from,
        gapi_latencyBudgetQosPolicy &to)
{
    DDS::ccpp_Duration_copyIn( from.duration, to.duration);
}

void DDS::ccpp_LivelinessQosPolicy_copyIn( const DDS::LivelinessQosPolicy &from,
        gapi_livelinessQosPolicy &to)
{
    switch (from.kind)
    {
      case DDS::AUTOMATIC_LIVELINESS_QOS:
        to.kind = GAPI_AUTOMATIC_LIVELINESS_QOS;
        break;
      case DDS::MANUAL_BY_PARTICIPANT_LIVELINESS_QOS:
        to.kind = GAPI_MANUAL_BY_PARTICIPANT_LIVELINESS_QOS;
        break;
      case DDS::MANUAL_BY_TOPIC_LIVELINESS_QOS:
        to.kind = GAPI_MANUAL_BY_TOPIC_LIVELINESS_QOS;
        break;
      default:
        //impossible to reach
        break;
    }
    DDS::ccpp_Duration_copyIn(from.lease_duration, to.lease_duration);
}

void DDS::ccpp_ReliabilityQosPolicy_copyIn( const DDS::ReliabilityQosPolicy &from,
        gapi_reliabilityQosPolicy &to)
{
    switch (from.kind)
    {
      case DDS::BEST_EFFORT_RELIABILITY_QOS:
        to.kind = GAPI_BEST_EFFORT_RELIABILITY_QOS;
        break;
      case DDS::RELIABLE_RELIABILITY_QOS:
        to.kind = GAPI_RELIABLE_RELIABILITY_QOS;
        break;
      default:
        //impossible to reach
        break;
    }
    DDS::ccpp_Duration_copyIn(from.max_blocking_time, to.max_blocking_time);
    to.synchronous = from.synchronous ? TRUE : FALSE;
}

void DDS::ccpp_DestinationOrderQosPolicy_copyIn( const DDS::DestinationOrderQosPolicy &from,
        gapi_destinationOrderQosPolicy &to)
{
    switch (from.kind)
    {
      case DDS::BY_RECEPTION_TIMESTAMP_DESTINATIONORDER_QOS:
        to.kind = GAPI_BY_RECEPTION_TIMESTAMP_DESTINATIONORDER_QOS;
        break;
      case DDS::BY_SOURCE_TIMESTAMP_DESTINATIONORDER_QOS:
        to.kind = GAPI_BY_SOURCE_TIMESTAMP_DESTINATIONORDER_QOS;
        break;
      default:
        //impossible to reach
        break;
    }
}

void DDS::ccpp_HistoryQosPolicy_copyIn( const DDS::HistoryQosPolicy &from,
        gapi_historyQosPolicy &to)
{
    switch (from.kind)
    {
      case DDS::KEEP_LAST_HISTORY_QOS:
        to.kind = GAPI_KEEP_LAST_HISTORY_QOS;
        break;
      case DDS::KEEP_ALL_HISTORY_QOS:
        to.kind = GAPI_KEEP_ALL_HISTORY_QOS;
        break;
      default:
        //impossible to reach
        break;
    }
    to.depth = from.depth;
}

void DDS::ccpp_ResourceLimitsQosPolicy_copyIn( const DDS::ResourceLimitsQosPolicy &from,
        gapi_resourceLimitsQosPolicy &to)
{
   to.max_samples = from.max_samples;
   to.max_instances = from.max_instances;
   to.max_samples_per_instance = from.max_samples_per_instance;
}

void DDS::ccpp_TransportPriorityQosPolicy_copyIn( const DDS::TransportPriorityQosPolicy &from,
        gapi_transportPriorityQosPolicy &to)
{
    to.value = from.value;
}

void DDS::ccpp_LifespanQosPolicy_copyIn( const DDS::LifespanQosPolicy &from,
        gapi_lifespanQosPolicy &to)
{
    DDS::ccpp_Duration_copyIn( from.duration, to.duration);
}

void DDS::ccpp_OwnershipQosPolicy_copyIn( const DDS::OwnershipQosPolicy &from,
        gapi_ownershipQosPolicy &to)
{
    switch (from.kind)
    {
      case DDS::SHARED_OWNERSHIP_QOS:
        to.kind = GAPI_SHARED_OWNERSHIP_QOS;
        break;
      case DDS::EXCLUSIVE_OWNERSHIP_QOS:
        to.kind = GAPI_EXCLUSIVE_OWNERSHIP_QOS;
        break;
      default:
        //impossible to reach
        break;
     }
}

void DDS::ccpp_OwnershipStrengthQosPolicy_copyIn(const DDS::OwnershipStrengthQosPolicy &from,
        gapi_ownershipStrengthQosPolicy &to)
{
  to.value = from.value;
}

void DDS::ccpp_WriterDataLifecycleQosPolicy_copyIn( const DDS::WriterDataLifecycleQosPolicy &from,
        gapi_writerDataLifecycleQosPolicy &to)
{
  to.autodispose_unregistered_instances = from.autodispose_unregistered_instances;
  DDS::ccpp_Duration_copyIn( from.autopurge_suspended_samples_delay, to.autopurge_suspended_samples_delay);
  DDS::ccpp_Duration_copyIn( from.autounregister_instance_delay, to.autounregister_instance_delay);
}

void DDS::ccpp_PresentationQosPolicy_copyIn( const DDS::PresentationQosPolicy & from,
        gapi_presentationQosPolicy & to)
{
  to.access_scope = (gapi_presentationQosPolicyAccessScopeKind)from.access_scope;
  to.coherent_access = from.coherent_access;
  to.ordered_access = from.ordered_access;
}

void DDS::ccpp_PartitionQosPolicy_copyIn( const DDS::PartitionQosPolicy &from,
        gapi_partitionQosPolicy &to)
{
  DDS::ccpp_sequenceCopyIn(from.name, to.name);
}

void DDS::ccpp_GroupDataQosPolicy_copyIn(const DDS::GroupDataQosPolicy &from,
        gapi_groupDataQosPolicy &to)
{
    DDS::ccpp_sequenceCopyIn< DDS::octSeq, CORBA::Octet, gapi_octetSeq, gapi_octet >
            ( from.value, to.value );
}


void DDS::ccpp_TopicDataQosPolicy_copyOut( const gapi_topicDataQosPolicy &from,
        DDS::TopicDataQosPolicy &to)
{
    DDS::ccpp_sequenceCopyOut< gapi_octetSeq, gapi_octet, DDS::octSeq, CORBA::Octet>(
            from.value, to.value );
}

void DDS::ccpp_DurabilityQosPolicy_copyOut( const gapi_durabilityQosPolicy &from,
        DDS::DurabilityQosPolicy &to)
{
    switch(from.kind)
    {
      case GAPI_VOLATILE_DURABILITY_QOS:
        to.kind = DDS::VOLATILE_DURABILITY_QOS;
        break;
      case GAPI_TRANSIENT_LOCAL_DURABILITY_QOS:
        to.kind = DDS::TRANSIENT_LOCAL_DURABILITY_QOS;
        break;
      case GAPI_TRANSIENT_DURABILITY_QOS:
        to.kind = DDS::TRANSIENT_DURABILITY_QOS;
        break;
      case GAPI_PERSISTENT_DURABILITY_QOS:
        to.kind = DDS::PERSISTENT_DURABILITY_QOS;
        break;
      default:
        // impossible to reach
        break;
    }
}

void DDS::ccpp_DurabilityServiceQosPolicy_copyOut( const gapi_durabilityServiceQosPolicy &from,
        DDS::DurabilityServiceQosPolicy &to)
{
    DDS::ccpp_Duration_copyOut(from.service_cleanup_delay, to.service_cleanup_delay);
    switch(from.history_kind)
    {
      case GAPI_KEEP_LAST_HISTORY_QOS:
        to.history_kind = DDS::KEEP_LAST_HISTORY_QOS;
        break;
      case GAPI_KEEP_ALL_HISTORY_QOS:
        to.history_kind = DDS::KEEP_ALL_HISTORY_QOS;
        break;
      default:
        // impossible to reach
        break;
    }
    to.history_depth = from.history_depth;
    to.max_samples = from.max_samples;
    to.max_instances = from.max_instances;
    to.max_samples_per_instance = from.max_samples_per_instance;
}

void DDS::ccpp_SampleRejectedStatusKind_copyOut( const gapi_sampleRejectedStatusKind & from,
        DDS::SampleRejectedStatusKind & to)
{
    switch (from)
    {
    case GAPI_NOT_REJECTED:
        to = DDS::NOT_REJECTED;
        break;
    case GAPI_REJECTED_BY_INSTANCES_LIMIT:
        to = DDS::REJECTED_BY_INSTANCES_LIMIT;
        break;
    case GAPI_REJECTED_BY_SAMPLES_LIMIT:
        to = DDS::REJECTED_BY_SAMPLES_LIMIT;
        break;
    case GAPI_REJECTED_BY_SAMPLES_PER_INSTANCE_LIMIT:
        to = DDS::REJECTED_BY_SAMPLES_PER_INSTANCE_LIMIT;
        break;
    default:
      // impossible to reach
      break;
    }
}

void DDS::ccpp_DeadlineQosPolicy_copyOut( const gapi_deadlineQosPolicy &from,
        DDS::DeadlineQosPolicy &to)
{
    DDS::ccpp_Duration_copyOut( from.period, to.period);
}


void DDS::ccpp_LatencyBudgetQosPolicy_copyOut( const gapi_latencyBudgetQosPolicy &from,
        DDS::LatencyBudgetQosPolicy &to)
{
    DDS::ccpp_Duration_copyOut( from.duration, to.duration);
}

void DDS::ccpp_LivelinessQosPolicy_copyOut( const gapi_livelinessQosPolicy &from,
        DDS::LivelinessQosPolicy &to)
{
    switch (from.kind)
    {
      case GAPI_AUTOMATIC_LIVELINESS_QOS:
        to.kind = DDS::AUTOMATIC_LIVELINESS_QOS;
        break;
      case GAPI_MANUAL_BY_PARTICIPANT_LIVELINESS_QOS:
        to.kind = DDS::MANUAL_BY_PARTICIPANT_LIVELINESS_QOS;
        break;
      case GAPI_MANUAL_BY_TOPIC_LIVELINESS_QOS:
        to.kind = DDS::MANUAL_BY_TOPIC_LIVELINESS_QOS;
        break;
      default:
        //impossible to reach
        break;
    }
    DDS::ccpp_Duration_copyOut(from.lease_duration, to.lease_duration);
}

void DDS::ccpp_ReliabilityQosPolicy_copyOut( const gapi_reliabilityQosPolicy &from,
        DDS::ReliabilityQosPolicy &to)
{
    switch (from.kind)
    {
      case GAPI_BEST_EFFORT_RELIABILITY_QOS:
        to.kind = DDS::BEST_EFFORT_RELIABILITY_QOS;
        break;
      case GAPI_RELIABLE_RELIABILITY_QOS:
        to.kind = DDS::RELIABLE_RELIABILITY_QOS;
        break;
      default:
        //impossible to reach
        break;
    }
    DDS::ccpp_Duration_copyOut(from.max_blocking_time, to.max_blocking_time);
    to.synchronous = from.synchronous ? TRUE : FALSE;
}

void DDS::ccpp_DestinationOrderQosPolicy_copyOut( const gapi_destinationOrderQosPolicy &from,
        DDS::DestinationOrderQosPolicy &to)
{
    switch (from.kind)
    {
      case GAPI_BY_RECEPTION_TIMESTAMP_DESTINATIONORDER_QOS:
        to.kind = DDS::BY_RECEPTION_TIMESTAMP_DESTINATIONORDER_QOS;
        break;
      case GAPI_BY_SOURCE_TIMESTAMP_DESTINATIONORDER_QOS:
        to.kind = DDS::BY_SOURCE_TIMESTAMP_DESTINATIONORDER_QOS;
        break;
      default:
        //impossible to reach
        break;
    }
}

void DDS::ccpp_HistoryQosPolicy_copyOut( const gapi_historyQosPolicy &from,
        DDS::HistoryQosPolicy &to)
{
    switch (from.kind)
    {
      case GAPI_KEEP_LAST_HISTORY_QOS:
        to.kind = DDS::KEEP_LAST_HISTORY_QOS;
        break;
      case GAPI_KEEP_ALL_HISTORY_QOS:
        to.kind = DDS::KEEP_ALL_HISTORY_QOS;
        break;
      default:
        //impossible to reach
        break;
    }
    to.depth = from.depth;
}

void DDS::ccpp_ResourceLimitsQosPolicy_copyOut( const gapi_resourceLimitsQosPolicy &from,
        DDS::ResourceLimitsQosPolicy &to)
{
   to.max_samples = from.max_samples;
   to.max_instances = from.max_instances;
   to.max_samples_per_instance = from.max_samples_per_instance;
}

void DDS::ccpp_TransportPriorityQosPolicy_copyOut( const gapi_transportPriorityQosPolicy &from,
        DDS::TransportPriorityQosPolicy &to)
{
    to.value = from.value;
}

void DDS::ccpp_LifespanQosPolicy_copyOut( const gapi_lifespanQosPolicy &from,
        DDS::LifespanQosPolicy &to)
{
    DDS::ccpp_Duration_copyOut( from.duration, to.duration);
}

void DDS::ccpp_OwnershipQosPolicy_copyOut( const gapi_ownershipQosPolicy &from,
        DDS::OwnershipQosPolicy &to)
{
    switch (from.kind)
    {
      case GAPI_SHARED_OWNERSHIP_QOS:
        to.kind = DDS::SHARED_OWNERSHIP_QOS;
        break;
      case GAPI_EXCLUSIVE_OWNERSHIP_QOS:
        to.kind = DDS::EXCLUSIVE_OWNERSHIP_QOS;
        break;
      default:
        //impossible to reach
        break;
     }
}

void DDS::ccpp_OwnershipStrengthQosPolicy_copyOut(const gapi_ownershipStrengthQosPolicy &from,
        DDS::OwnershipStrengthQosPolicy &to)
{
  to.value = from.value;
}

void DDS::ccpp_WriterDataLifecycleQosPolicy_copyOut( const gapi_writerDataLifecycleQosPolicy &from,
        DDS::WriterDataLifecycleQosPolicy &to)
{
  to.autodispose_unregistered_instances = from.autodispose_unregistered_instances;
  ccpp_Duration_copyOut(from.autopurge_suspended_samples_delay, to.autopurge_suspended_samples_delay);
  ccpp_Duration_copyOut(from.autounregister_instance_delay, to.autounregister_instance_delay);
}

void DDS::ccpp_ReaderDataLifecycleQosPolicy_copyOut( const gapi_readerDataLifecycleQosPolicy &from,
        DDS::ReaderDataLifecycleQosPolicy &to)
{
  ccpp_Duration_copyOut(from.autopurge_nowriter_samples_delay, to.autopurge_nowriter_samples_delay);
  ccpp_Duration_copyOut(from.autopurge_disposed_samples_delay, to.autopurge_disposed_samples_delay);
  to.enable_invalid_samples = from.enable_invalid_samples;
}

void DDS::ccpp_ReaderDataLifecycleQosPolicy_copyIn( const DDS::ReaderDataLifecycleQosPolicy &from,
        gapi_readerDataLifecycleQosPolicy &to)
{
  ccpp_Duration_copyIn(from.autopurge_nowriter_samples_delay, to.autopurge_nowriter_samples_delay);
  ccpp_Duration_copyIn(from.autopurge_disposed_samples_delay, to.autopurge_disposed_samples_delay);
  to.enable_invalid_samples = from.enable_invalid_samples;
}

void DDS::ccpp_PresentationQosPolicy_copyOut( const gapi_presentationQosPolicy & from,
        DDS::PresentationQosPolicy & to)
{
  to.access_scope = (DDS::PresentationQosPolicyAccessScopeKind)from.access_scope;
  to.coherent_access = from.coherent_access;
  to.ordered_access = from.ordered_access;
}

void DDS::ccpp_PartitionQosPolicy_copyOut( const gapi_partitionQosPolicy &from,
        DDS::PartitionQosPolicy &to)
{
  DDS::ccpp_sequenceCopyOut(from.name, to.name);
}

void DDS::ccpp_GroupDataQosPolicy_copyOut(
        const gapi_groupDataQosPolicy &from,
        DDS::GroupDataQosPolicy &to)
{
    DDS::ccpp_sequenceCopyOut< gapi_octetSeq, gapi_octet, DDS::octSeq, CORBA::Octet >(
            from.value, to.value );
}

void DDS::ccpp_SubscriptionKeyQosPolicy_copyIn (
     const DDS::SubscriptionKeyQosPolicy &from,
     gapi_subscriptionKeyQosPolicy &to )
{
    to.use_key_list = (from.use_key_list != FALSE);
    DDS::ccpp_sequenceCopyIn(from.key_list, to.key_list);
}

void DDS::ccpp_SubscriptionKeyQosPolicy_copyOut(
        const gapi_subscriptionKeyQosPolicy &from,
        DDS::SubscriptionKeyQosPolicy &to )
{
    DDS::ccpp_sequenceCopyOut(from.key_list, to.key_list);
    to.use_key_list = from.use_key_list;
}

void DDS::ccpp_ReaderLifespanQosPolicy_copyIn (
     const DDS::ReaderLifespanQosPolicy &from,
     gapi_readerLifespanQosPolicy &to )
{
    to.use_lifespan = (from.use_lifespan != FALSE);
    DDS::ccpp_Duration_copyIn( from.duration, to.duration);
}

void DDS::ccpp_ReaderLifespanQosPolicy_copyOut (
    const gapi_readerLifespanQosPolicy &from,
    DDS::ReaderLifespanQosPolicy &to )
{
    to.use_lifespan = from.use_lifespan;
    DDS::ccpp_Duration_copyOut(from.duration, to.duration);
}

void DDS::ccpp_ShareQosPolicy_copyIn (
     const DDS::ShareQosPolicy &from,
     gapi_shareQosPolicy &to )
{
    const char *value = from.name;

    to.enable = from.enable;
    to.name = gapi_string_dup(value);
}

void DDS::ccpp_ShareQosPolicy_copyOut (
    const gapi_shareQosPolicy &from,
    DDS::ShareQosPolicy &to )
{
    to.enable = from.enable;
    to.name = CORBA::string_dup(reinterpret_cast<const char *>(from.name));
}

void DDS::ccpp_SchedulingClassQosPolicy_copyIn ( const DDS::SchedulingClassQosPolicy &from,
        gapi_schedulingClassQosPolicy &to )
{
    switch(from.kind)
    {
      case DDS::SCHEDULE_TIMESHARING:
        to.kind = GAPI_SCHEDULE_TIMESHARING;
        break;
      case DDS::SCHEDULE_REALTIME:
        to.kind = GAPI_SCHEDULE_REALTIME;
        break;
      case DDS::SCHEDULE_DEFAULT:
        to.kind = GAPI_SCHEDULE_DEFAULT;
        break;
      default:
        // impossible to reach
        break;
    }
}

void DDS::ccpp_SchedulingClassQosPolicy_copyOut ( const gapi_schedulingClassQosPolicy &from,
	DDS::SchedulingClassQosPolicy &to )
{
    switch(from.kind)
    {
      case GAPI_SCHEDULE_TIMESHARING:
        to.kind = DDS::SCHEDULE_TIMESHARING;
        break;
      case GAPI_SCHEDULE_REALTIME:
        to.kind = DDS::SCHEDULE_REALTIME;
        break;
      case GAPI_SCHEDULE_DEFAULT:
        to.kind = DDS::SCHEDULE_DEFAULT;
        break;
      default:
        // impossible to reach
        break;
    }
}

void DDS::ccpp_SchedulingPriorityQosPolicy_copyIn ( const DDS::SchedulingPriorityQosPolicy &from,
        gapi_schedulingPriorityQosPolicy &to )
{
    switch(from.kind)
    {
      case DDS::PRIORITY_ABSOLUTE:
        to.kind = GAPI_PRIORITY_ABSOLUTE;
        break;
      case DDS::PRIORITY_RELATIVE:
        to.kind = GAPI_PRIORITY_RELATIVE;
        break;
      default:
        // impossible to reach
        break;
    }
}

void DDS::ccpp_SchedulingPriorityQosPolicy_copyOut ( const gapi_schedulingPriorityQosPolicy &from,
	DDS::SchedulingPriorityQosPolicy &to )
{
    switch(from.kind)
    {
      case GAPI_PRIORITY_ABSOLUTE:
        to.kind = DDS::PRIORITY_ABSOLUTE;
        break;
      case GAPI_PRIORITY_RELATIVE:
        to.kind = DDS::PRIORITY_RELATIVE;
        break;
      default:
        // impossible to reach
        break;
    }
}

void DDS::ccpp_SchedulingQosPolicy_copyIn ( const DDS::SchedulingQosPolicy &from,
        gapi_schedulingQosPolicy &to )
{
    DDS::ccpp_SchedulingClassQosPolicy_copyIn ( from.scheduling_class, to.scheduling_class );
    DDS::ccpp_SchedulingPriorityQosPolicy_copyIn ( from.scheduling_priority_kind, to.scheduling_priority_kind );
    to.scheduling_priority = from.scheduling_priority;
}

void DDS::ccpp_SchedulingQosPolicy_copyOut ( const gapi_schedulingQosPolicy &from,
	DDS::SchedulingQosPolicy &to )
{
    DDS::ccpp_SchedulingClassQosPolicy_copyOut ( from.scheduling_class, to.scheduling_class );
    DDS::ccpp_SchedulingPriorityQosPolicy_copyOut ( from.scheduling_priority_kind, to.scheduling_priority_kind );
    to.scheduling_priority = from.scheduling_priority;
}

//Qos conversions

void DDS::ccpp_DomainParticipantFactoryQos_copyIn( const DDS::DomainParticipantFactoryQos &from,
        gapi_domainParticipantFactoryQos &to )
{
    DDS::ccpp_EntityFactoryQosPolicy_copyIn ( from.entity_factory, to.entity_factory );
}

void DDS::ccpp_DomainParticipantFactoryQos_copyOut( const gapi_domainParticipantFactoryQos &from,
        DDS::DomainParticipantFactoryQos &to )
{
    DDS::ccpp_EntityFactoryQosPolicy_copyOut ( from.entity_factory, to.entity_factory );
}

void DDS::ccpp_DomainParticipantQos_copyIn( const DDS::DomainParticipantQos &from,
        gapi_domainParticipantQos &to )
{
    DDS::ccpp_UserDataQosPolicy_copyIn( from.user_data, to.user_data );
    DDS::ccpp_EntityFactoryQosPolicy_copyIn ( from.entity_factory, to.entity_factory );
    DDS::ccpp_SchedulingQosPolicy_copyIn ( from.watchdog_scheduling, to.watchdog_scheduling );
    DDS::ccpp_SchedulingQosPolicy_copyIn ( from.listener_scheduling, to.listener_scheduling );
}

void DDS::ccpp_DomainParticipantQos_copyOut( const gapi_domainParticipantQos &from,
        DDS::DomainParticipantQos &to )
{
    DDS::ccpp_UserDataQosPolicy_copyOut( from.user_data, to.user_data );
    DDS::ccpp_EntityFactoryQosPolicy_copyOut ( from.entity_factory, to.entity_factory );
    DDS::ccpp_SchedulingQosPolicy_copyOut ( from.watchdog_scheduling, to.watchdog_scheduling );
    DDS::ccpp_SchedulingQosPolicy_copyOut ( from.listener_scheduling, to.listener_scheduling );
}

void DDS::ccpp_TopicQos_copyIn( const DDS::TopicQos &from,
        gapi_topicQos &to)
{
  DDS::ccpp_TopicDataQosPolicy_copyIn( from.topic_data, to.topic_data);
  DDS::ccpp_DurabilityQosPolicy_copyIn( from.durability, to.durability);
  DDS::ccpp_DurabilityServiceQosPolicy_copyIn( from.durability_service, to.durability_service);
  DDS::ccpp_DeadlineQosPolicy_copyIn( from.deadline, to.deadline);
  DDS::ccpp_LatencyBudgetQosPolicy_copyIn( from.latency_budget, to.latency_budget);
  DDS::ccpp_LivelinessQosPolicy_copyIn( from.liveliness, to.liveliness);
  DDS::ccpp_ReliabilityQosPolicy_copyIn( from.reliability, to.reliability);
  DDS::ccpp_DestinationOrderQosPolicy_copyIn( from.destination_order, to.destination_order);
  DDS::ccpp_HistoryQosPolicy_copyIn( from.history, to.history);
  DDS::ccpp_ResourceLimitsQosPolicy_copyIn( from.resource_limits, to.resource_limits);
  DDS::ccpp_TransportPriorityQosPolicy_copyIn( from.transport_priority, to.transport_priority);
  DDS::ccpp_LifespanQosPolicy_copyIn( from.lifespan, to.lifespan);
  DDS::ccpp_OwnershipQosPolicy_copyIn( from.ownership, to.ownership);
}

void DDS::ccpp_TopicQos_copyOut( const gapi_topicQos &from,
        DDS::TopicQos &to)
{
  DDS::ccpp_TopicDataQosPolicy_copyOut( from.topic_data, to.topic_data);
  DDS::ccpp_DurabilityQosPolicy_copyOut( from.durability, to.durability);
  DDS::ccpp_DurabilityServiceQosPolicy_copyOut( from.durability_service, to.durability_service);
  DDS::ccpp_DeadlineQosPolicy_copyOut( from.deadline, to.deadline);
  DDS::ccpp_LatencyBudgetQosPolicy_copyOut( from.latency_budget, to.latency_budget);
  DDS::ccpp_LivelinessQosPolicy_copyOut( from.liveliness, to.liveliness);
  DDS::ccpp_ReliabilityQosPolicy_copyOut( from.reliability, to.reliability);
  DDS::ccpp_DestinationOrderQosPolicy_copyOut( from.destination_order, to.destination_order);
  DDS::ccpp_HistoryQosPolicy_copyOut( from.history, to.history);
  DDS::ccpp_ResourceLimitsQosPolicy_copyOut( from.resource_limits, to.resource_limits);
  DDS::ccpp_TransportPriorityQosPolicy_copyOut( from.transport_priority, to.transport_priority);
  DDS::ccpp_LifespanQosPolicy_copyOut( from.lifespan, to.lifespan);
  DDS::ccpp_OwnershipQosPolicy_copyOut( from.ownership, to.ownership);
}

void DDS::ccpp_DataWriterQos_copyIn( const DDS::DataWriterQos &from,
        gapi_dataWriterQos &to)
{
  DDS::ccpp_DurabilityQosPolicy_copyIn( from.durability, to.durability);
  DDS::ccpp_DeadlineQosPolicy_copyIn( from.deadline, to.deadline);
  DDS::ccpp_LatencyBudgetQosPolicy_copyIn( from.latency_budget, to.latency_budget);
  DDS::ccpp_LivelinessQosPolicy_copyIn( from.liveliness, to.liveliness);
  DDS::ccpp_ReliabilityQosPolicy_copyIn( from.reliability, to.reliability);
  DDS::ccpp_DestinationOrderQosPolicy_copyIn( from.destination_order, to.destination_order);
  DDS::ccpp_HistoryQosPolicy_copyIn( from.history, to.history);
  DDS::ccpp_ResourceLimitsQosPolicy_copyIn( from.resource_limits, to.resource_limits);
  DDS::ccpp_TransportPriorityQosPolicy_copyIn( from.transport_priority, to.transport_priority);
  DDS::ccpp_LifespanQosPolicy_copyIn( from.lifespan, to.lifespan);
  DDS::ccpp_UserDataQosPolicy_copyIn( from.user_data, to.user_data );
  DDS::ccpp_OwnershipQosPolicy_copyIn(from.ownership, to.ownership);
  DDS::ccpp_OwnershipStrengthQosPolicy_copyIn(from.ownership_strength, to.ownership_strength);
  DDS::ccpp_WriterDataLifecycleQosPolicy_copyIn(from.writer_data_lifecycle, to.writer_data_lifecycle);
}

void DDS::ccpp_DataWriterQos_copyOut( const gapi_dataWriterQos &from,
        DDS::DataWriterQos &to)
{
  DDS::ccpp_DurabilityQosPolicy_copyOut( from.durability, to.durability);
  DDS::ccpp_DeadlineQosPolicy_copyOut( from.deadline, to.deadline);
  DDS::ccpp_LatencyBudgetQosPolicy_copyOut( from.latency_budget, to.latency_budget);
  DDS::ccpp_LivelinessQosPolicy_copyOut( from.liveliness, to.liveliness);
  DDS::ccpp_ReliabilityQosPolicy_copyOut( from.reliability, to.reliability);
  DDS::ccpp_DestinationOrderQosPolicy_copyOut( from.destination_order, to.destination_order);
  DDS::ccpp_HistoryQosPolicy_copyOut( from.history, to.history);
  DDS::ccpp_ResourceLimitsQosPolicy_copyOut( from.resource_limits, to.resource_limits);
  DDS::ccpp_TransportPriorityQosPolicy_copyOut( from.transport_priority, to.transport_priority);
  DDS::ccpp_LifespanQosPolicy_copyOut( from.lifespan, to.lifespan);
  DDS::ccpp_UserDataQosPolicy_copyOut( from.user_data, to.user_data );
  DDS::ccpp_OwnershipQosPolicy_copyOut(from.ownership, to.ownership);
  DDS::ccpp_OwnershipStrengthQosPolicy_copyOut(from.ownership_strength, to.ownership_strength);
  DDS::ccpp_WriterDataLifecycleQosPolicy_copyOut(from.writer_data_lifecycle, to.writer_data_lifecycle);
}

void DDS::ccpp_DataReaderQos_copyOut( const gapi_dataReaderQos &from,
        DDS::DataReaderQos &to)
{
  DDS::ccpp_DurabilityQosPolicy_copyOut( from.durability, to.durability);
  DDS::ccpp_DeadlineQosPolicy_copyOut( from.deadline, to.deadline);
  DDS::ccpp_LatencyBudgetQosPolicy_copyOut( from.latency_budget, to.latency_budget);
  DDS::ccpp_LivelinessQosPolicy_copyOut( from.liveliness, to.liveliness);
  DDS::ccpp_ReliabilityQosPolicy_copyOut( from.reliability, to.reliability);
  DDS::ccpp_DestinationOrderQosPolicy_copyOut( from.destination_order, to.destination_order);
  DDS::ccpp_HistoryQosPolicy_copyOut( from.history, to.history);
  DDS::ccpp_ResourceLimitsQosPolicy_copyOut( from.resource_limits, to.resource_limits);
  DDS::ccpp_UserDataQosPolicy_copyOut( from.user_data, to.user_data );
  DDS::ccpp_OwnershipQosPolicy_copyOut(from.ownership, to.ownership);
  DDS::ccpp_TimeBasedFilterQosPolicy_copyOut(from.time_based_filter, to.time_based_filter);
  DDS::ccpp_ReaderDataLifecycleQosPolicy_copyOut(from.reader_data_lifecycle, to.reader_data_lifecycle);
  DDS::ccpp_SubscriptionKeyQosPolicy_copyOut(from.subscription_keys, to.subscription_keys);
  DDS::ccpp_ReaderLifespanQosPolicy_copyOut(from.reader_lifespan, to.reader_lifespan);
  DDS::ccpp_ShareQosPolicy_copyOut(from.share, to.share);
}

void DDS::ccpp_DataReaderQos_copyIn( const DDS::DataReaderQos &from,
        gapi_dataReaderQos &to)
{
  DDS::ccpp_DurabilityQosPolicy_copyIn( from.durability, to.durability);
  DDS::ccpp_DeadlineQosPolicy_copyIn( from.deadline, to.deadline);
  DDS::ccpp_LatencyBudgetQosPolicy_copyIn( from.latency_budget, to.latency_budget);
  DDS::ccpp_LivelinessQosPolicy_copyIn( from.liveliness, to.liveliness);
  DDS::ccpp_ReliabilityQosPolicy_copyIn( from.reliability, to.reliability);
  DDS::ccpp_DestinationOrderQosPolicy_copyIn( from.destination_order, to.destination_order);
  DDS::ccpp_HistoryQosPolicy_copyIn( from.history, to.history);
  DDS::ccpp_ResourceLimitsQosPolicy_copyIn( from.resource_limits, to.resource_limits);
  DDS::ccpp_UserDataQosPolicy_copyIn( from.user_data, to.user_data );
  DDS::ccpp_OwnershipQosPolicy_copyIn(from.ownership, to.ownership);
  DDS::ccpp_TimeBasedFilterQosPolicy_copyIn(from.time_based_filter, to.time_based_filter);
  DDS::ccpp_ReaderDataLifecycleQosPolicy_copyIn(from.reader_data_lifecycle, to.reader_data_lifecycle);
  DDS::ccpp_SubscriptionKeyQosPolicy_copyIn(from.subscription_keys, to.subscription_keys);
  DDS::ccpp_ReaderLifespanQosPolicy_copyIn(from.reader_lifespan, to.reader_lifespan);
  DDS::ccpp_ShareQosPolicy_copyIn(from.share, to.share);
}

void DDS::ccpp_PublisherQos_copyOut( const gapi_publisherQos &from,
        DDS::PublisherQos &to)
{
  DDS::ccpp_PresentationQosPolicy_copyOut( from.presentation, to.presentation);
  DDS::ccpp_PartitionQosPolicy_copyOut( from.partition, to.partition);
  DDS::ccpp_GroupDataQosPolicy_copyOut( from.group_data, to.group_data);
  DDS::ccpp_EntityFactoryQosPolicy_copyOut( from.entity_factory, to.entity_factory);
}

void DDS::ccpp_PublisherQos_copyIn( const DDS::PublisherQos &from,
        gapi_publisherQos &to)
{
  DDS::ccpp_PresentationQosPolicy_copyIn( from.presentation, to.presentation);
  DDS::ccpp_PartitionQosPolicy_copyIn( from.partition, to.partition);
  DDS::ccpp_GroupDataQosPolicy_copyIn( from.group_data, to.group_data);
  DDS::ccpp_EntityFactoryQosPolicy_copyIn( from.entity_factory, to.entity_factory);
}

void DDS::ccpp_SubscriberQos_copyIn( const DDS::SubscriberQos &from,
    gapi_subscriberQos &to)
{
  DDS::ccpp_PresentationQosPolicy_copyIn( from.presentation, to.presentation);
  DDS::ccpp_PartitionQosPolicy_copyIn( from.partition, to.partition);
  DDS::ccpp_GroupDataQosPolicy_copyIn( from.group_data, to.group_data);
  DDS::ccpp_EntityFactoryQosPolicy_copyIn( from.entity_factory, to.entity_factory);
  DDS::ccpp_ShareQosPolicy_copyIn(from.share, to.share);
}

void DDS::ccpp_SubscriberQos_copyOut( const gapi_subscriberQos &from,
    DDS::SubscriberQos &to)
{
  DDS::ccpp_PresentationQosPolicy_copyOut( from.presentation, to.presentation);
  DDS::ccpp_PartitionQosPolicy_copyOut( from.partition, to.partition);
  DDS::ccpp_GroupDataQosPolicy_copyOut( from.group_data, to.group_data);
  DDS::ccpp_EntityFactoryQosPolicy_copyOut( from.entity_factory, to.entity_factory);
  DDS::ccpp_ShareQosPolicy_copyOut(from.share, to.share);
}

void DDS::ccpp_OfferedIncompatibleQosStatus_copyOut(
        const gapi_offeredIncompatibleQosStatus & from,
        DDS::OfferedIncompatibleQosStatus &to)
{
  to.total_count = from.total_count;
  to.total_count_change = from.total_count_change;
  to.last_policy_id = from.last_policy_id;
  DDS::ccpp_sequenceCopyOut<gapi_qosPolicyCountSeq, gapi_qosPolicyCount, DDS::QosPolicyCountSeq, DDS::QosPolicyCount>(from.policies, to.policies);
}

void DDS::ccpp_RequestedIncompatibleQosStatus_copyOut(
        const gapi_requestedIncompatibleQosStatus & from,
        DDS::RequestedIncompatibleQosStatus &to)
{
  to.total_count = from.total_count;
  to.total_count_change = from.total_count_change;
  to.last_policy_id = from.last_policy_id;
  DDS::ccpp_sequenceCopyOut<gapi_qosPolicyCountSeq, gapi_qosPolicyCount, DDS::QosPolicyCountSeq, DDS::QosPolicyCount>(from.policies, to.policies);
}

void DDS::ccpp_TimeBasedFilterQosPolicy_copyOut( const gapi_timeBasedFilterQosPolicy & from,
    DDS::TimeBasedFilterQosPolicy & to)
{
  DDS::ccpp_Duration_copyOut(from.minimum_separation, to.minimum_separation);
}

void DDS::ccpp_TimeBasedFilterQosPolicy_copyIn( const DDS::TimeBasedFilterQosPolicy & from,
    gapi_timeBasedFilterQosPolicy & to)
{
  DDS::ccpp_Duration_copyIn(from.minimum_separation, to.minimum_separation);
}

void DDS::ccpp_RequestedDeadlineMissedStatus_copyOut(
        const gapi_requestedDeadlineMissedStatus & from,
        DDS::RequestedDeadlineMissedStatus &to)
{
  to.total_count = from.total_count;
  to.total_count_change = from.total_count_change;
  to.last_instance_handle = from.last_instance_handle;
}

void DDS::ccpp_SampleRejectedStatus_copyOut(
        const gapi_sampleRejectedStatus & from,
        DDS::SampleRejectedStatus &to)
{
  to.total_count = from.total_count;
  to.total_count_change = from.total_count_change;
  ccpp_SampleRejectedStatusKind_copyOut(from.last_reason, to.last_reason);
  to.last_instance_handle = from.last_instance_handle;
}

void DDS::ccpp_LivelinessChangedStatus_copyOut(
        const gapi_livelinessChangedStatus & from,
        DDS::LivelinessChangedStatus &to)
{
  to.alive_count = from.alive_count;
  to.alive_count_change = from.alive_count_change;
  to.not_alive_count = from.not_alive_count;
  to.not_alive_count_change = from.not_alive_count_change;
  to.last_publication_handle = from.last_publication_handle;
}

void DDS::ccpp_SubscriptionMatchedStatus_copyOut(
        const gapi_subscriptionMatchedStatus & from,
        DDS::SubscriptionMatchedStatus &to)
{
  to.total_count = from.total_count;
  to.total_count_change = from.total_count_change;
  to.current_count = from.current_count;
  to.current_count_change = from.current_count_change;
  to.last_publication_handle = (DDS::InstanceHandle_t)from.last_publication_handle;
}
void DDS::ccpp_SampleLostStatus_copyOut(
        const gapi_sampleLostStatus & from,
        DDS::SampleLostStatus &to)
{
  to.total_count = from.total_count;
  to.total_count_change = from.total_count_change;
}

void DDS::ccpp_InconsistentTopicStatus_copyOut(
        const gapi_inconsistentTopicStatus & from,
        DDS::InconsistentTopicStatus &to)
{
  to.total_count = from.total_count;
  to.total_count_change = from.total_count_change;
}

void DDS::ccpp_AllDataDisposedTopicStatus_copyOut(
        const gapi_allDataDisposedTopicStatus & from,
        DDS::AllDataDisposedTopicStatus &to)
{
  to.total_count = from.total_count;
  to.total_count_change = from.total_count_change;
}

void DDS::ccpp_OfferedDeadlineMissedStatus_copyOut(
        const gapi_offeredDeadlineMissedStatus & from,
        DDS::OfferedDeadlineMissedStatus &to)
{
  to.total_count = from.total_count;
  to.total_count_change = from.total_count_change;
  to.last_instance_handle = from.last_instance_handle;
}

void DDS::ccpp_LivelinessLostStatus_copyOut(
        const gapi_livelinessLostStatus & from,
        DDS::LivelinessLostStatus &to)
{
  to.total_count = from.total_count;
  to.total_count_change = from.total_count_change;
}

void DDS::ccpp_PublicationMatchedStatus_copyOut(
        const gapi_publicationMatchedStatus & from,
        DDS::PublicationMatchedStatus &to)
{
  to.total_count = from.total_count;
  to.total_count_change = from.total_count_change;
  to.current_count = from.current_count;
  to.current_count_change = from.current_count_change;
  to.last_subscription_handle = (DDS::InstanceHandle_t)from.last_subscription_handle;
}

void DDS::ccpp_DataReaderViewQos_copyIn(
        const DDS::DataReaderViewQos &from,
        gapi_dataReaderViewQos &to)
{
    DDS::ccpp_ViewKeyQosPolicy_copyIn( from.view_keys, to.view_keys);
}

void DDS::ccpp_DataReaderViewQos_copyOut(
        const gapi_dataReaderViewQos &from,
        DDS::DataReaderViewQos &to)
{
    DDS::ccpp_ViewKeyQosPolicy_copyOut( from.view_keys, to.view_keys);
}

void DDS::ccpp_ViewKeyQosPolicy_copyIn(
        const DDS::ViewKeyQosPolicy &from,
        gapi_viewKeyQosPolicy &to)
{
    to.use_key_list = from.use_key_list;
    DDS::ccpp_sequenceCopyIn(from.key_list, to.key_list);
}

void DDS::ccpp_ViewKeyQosPolicy_copyOut(
        const gapi_viewKeyQosPolicy &from,
        DDS::ViewKeyQosPolicy &to)
{
    to.use_key_list = from.use_key_list;
    DDS::ccpp_sequenceCopyOut(from.key_list, to.key_list);
}
