
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

#include "qp_defaultQos.h"

#define QP_LENGTH_UNLIMITED                        (-1)
#define QP_DURATION_INFINITE_SEC                   (0x7fffffff)
#define QP_DURATION_INFINITE_NSEC                  (0x7fffffffU)
#define QP_DURATION_INFINITE                       { QP_DURATION_INFINITE_SEC, QP_DURATION_INFINITE_NSEC }

#define QP_DURATION_ZERO_SEC                       (0)
#define QP_DURATION_ZERO_NSEC                      (0U)
#define QP_DURATION_ZERO                           { QP_DURATION_ZERO_SEC, QP_DURATION_ZERO_NSEC }

#define QP_DEFAULT_MAX_BLOCKING_TIME  {0, 100000000} /* 100ms */

#define QP_OwnershipStrengthQosPolicy_default_value { 0 }
#define QP_DurabilityQosPolicy_default_value { _DDS_VOLATILE_DURABILITY_QOS }
#define QP_DeadlineQosPolicy_default_value { QP_DURATION_INFINITE }
#define QP_LatencyBudgetQosPolicy_default_value { QP_DURATION_ZERO }
#define QP_LivelinessQosPolicy_default_value { _DDS_AUTOMATIC_LIVELINESS_QOS, QP_DURATION_INFINITE }
#define QP_ReliabilityQosPolicy_default_value { _DDS_BEST_EFFORT_RELIABILITY_QOS, QP_DEFAULT_MAX_BLOCKING_TIME, FALSE }
#define QP_ReliabilityQosPolicy_writer_default_value { _DDS_RELIABLE_RELIABILITY_QOS, QP_DEFAULT_MAX_BLOCKING_TIME, FALSE }
#define QP_DestinationOrderQosPolicy_default_value { _DDS_BY_RECEPTION_TIMESTAMP_DESTINATIONORDER_QOS }
#define QP_HistoryQosPolicy_default_value { _DDS_KEEP_LAST_HISTORY_QOS, 1 }
#define QP_ResourceLimitsQosPolicy_default_value { QP_LENGTH_UNLIMITED, QP_LENGTH_UNLIMITED, QP_LENGTH_UNLIMITED }
#define QP_TransportPriorityQosPolicy_default_value { 0 }
#define QP_LifespanQosPolicy_default_value { QP_DURATION_INFINITE }
#define QP_OwnershipQosPolicy_default_value { _DDS_SHARED_OWNERSHIP_QOS }
#define QP_PresentationQosPolicy_default_value { _DDS_INSTANCE_PRESENTATION_QOS, FALSE, FALSE }
#define QP_EntityFactoryQosPolicy_default_value { TRUE }
#define QP_WriterDataLifecycleQosPolicy_default_value { TRUE, QP_DURATION_INFINITE, QP_DURATION_INFINITE }
#define QP_SchedulingQosPolicy_default_value { { _DDS_SCHEDULE_DEFAULT }, { _DDS_PRIORITY_RELATIVE }, 0 }
#define QP_OctetSequence_default_value NULL
#define QP_StringSequence_default_value NULL
#define QP_UserDataQosPolicy_default_value { QP_OctetSequence_default_value }
#define QP_TopicDataQosPolicy_default_value { QP_OctetSequence_default_value }
#define QP_GroupDataQosPolicy_default_value { QP_OctetSequence_default_value }
#define QP_PartitionQosPolicy_default_value { QP_OctetSequence_default_value }
#define QP_ReaderDataLifecycleQosPolicy_default_value { QP_DURATION_INFINITE, QP_DURATION_INFINITE, TRUE, { _DDS_MINIMUM_INVALID_SAMPLES } }
#define QP_TimeBasedFilterQosPolicy_default_value { QP_DURATION_ZERO }
#define QP_SubscriptionKeyQosPolicy_default_value { FALSE, QP_StringSequence_default_value }
#define QP_ReaderLifespanQosPolicy_default_value { FALSE, QP_DURATION_INFINITE }
#define QP_ShareQosPolicy_default_value { NULL, FALSE }
#define QP_ViewKeyQosPolicy_default_value { FALSE, NULL }
#define QP_DurabilityServiceQosPolicy_default_value { QP_DURATION_ZERO, _DDS_KEEP_LAST_HISTORY_QOS, 1, QP_LENGTH_UNLIMITED, QP_LENGTH_UNLIMITED, QP_LENGTH_UNLIMITED }

#define QP_DomainParticipantQos_default_value {    \
    QP_UserDataQosPolicy_default_value,            \
    QP_EntityFactoryQosPolicy_default_value,       \
    QP_SchedulingQosPolicy_default_value,          \
    QP_SchedulingQosPolicy_default_value           \
}

#define QP_TopicQos_default_value {                \
    QP_TopicDataQosPolicy_default_value,           \
    QP_DurabilityQosPolicy_default_value,          \
    QP_DurabilityServiceQosPolicy_default_value,   \
    QP_DeadlineQosPolicy_default_value,            \
    QP_LatencyBudgetQosPolicy_default_value,       \
    QP_LivelinessQosPolicy_default_value,          \
    QP_ReliabilityQosPolicy_default_value,         \
    QP_DestinationOrderQosPolicy_default_value,    \
    QP_HistoryQosPolicy_default_value,             \
    QP_ResourceLimitsQosPolicy_default_value,      \
    QP_TransportPriorityQosPolicy_default_value,   \
    QP_LifespanQosPolicy_default_value,            \
    QP_OwnershipQosPolicy_default_value            \
}

#define QP_PublisherQos_default_value {            \
    QP_PresentationQosPolicy_default_value,        \
    QP_PartitionQosPolicy_default_value,           \
    QP_GroupDataQosPolicy_default_value,           \
    QP_EntityFactoryQosPolicy_default_value        \
}

#define QP_SubscriberQos_default_value {           \
    QP_PresentationQosPolicy_default_value,        \
    QP_PartitionQosPolicy_default_value,           \
    QP_GroupDataQosPolicy_default_value,           \
    QP_EntityFactoryQosPolicy_default_value,       \
    QP_ShareQosPolicy_default_value                \
}

#define QP_DataReaderQos_default_value {           \
    QP_DurabilityQosPolicy_default_value,          \
    QP_DeadlineQosPolicy_default_value,            \
    QP_LatencyBudgetQosPolicy_default_value,       \
    QP_LivelinessQosPolicy_default_value,          \
    QP_ReliabilityQosPolicy_default_value,         \
    QP_DestinationOrderQosPolicy_default_value,    \
    QP_HistoryQosPolicy_default_value,             \
    QP_ResourceLimitsQosPolicy_default_value,      \
    QP_UserDataQosPolicy_default_value,            \
    QP_OwnershipQosPolicy_default_value,           \
    QP_TimeBasedFilterQosPolicy_default_value,     \
    QP_ReaderDataLifecycleQosPolicy_default_value, \
    QP_SubscriptionKeyQosPolicy_default_value,     \
    QP_ReaderLifespanQosPolicy_default_value,      \
    QP_ShareQosPolicy_default_value                \
}

#define QP_DataReaderViewQos_default_value {       \
    QP_SubscriptionKeyQosPolicy_default_value      \
}

#define QP_DataWriterQos_default_value {           \
    QP_DurabilityQosPolicy_default_value,          \
    QP_DeadlineQosPolicy_default_value,            \
    QP_LatencyBudgetQosPolicy_default_value,       \
    QP_LivelinessQosPolicy_default_value,          \
    QP_ReliabilityQosPolicy_writer_default_value,  \
    QP_DestinationOrderQosPolicy_default_value,    \
    QP_HistoryQosPolicy_default_value,             \
    QP_ResourceLimitsQosPolicy_default_value,      \
    QP_TransportPriorityQosPolicy_default_value,   \
    QP_LifespanQosPolicy_default_value,            \
    QP_UserDataQosPolicy_default_value,            \
    QP_OwnershipQosPolicy_default_value,           \
    QP_OwnershipStrengthQosPolicy_default_value,   \
    QP_WriterDataLifecycleQosPolicy_default_value  \
}

const struct _DDS_NamedDomainParticipantQos qp_NamedDomainParticipantQos_default = {NULL, QP_DomainParticipantQos_default_value};
const struct _DDS_NamedTopicQos qp_NamedTopicQos_default = {NULL, QP_TopicQos_default_value};
const struct _DDS_NamedPublisherQos qp_NamedPublisherQos_default = {NULL, QP_PublisherQos_default_value};
const struct _DDS_NamedDataWriterQos qp_NamedDataWriterQos_default = {NULL, QP_DataWriterQos_default_value};
const struct _DDS_NamedSubscriberQos qp_NamedSubscriberQos_default = {NULL, QP_SubscriberQos_default_value};
const struct _DDS_NamedDataReaderQos qp_NamedDataReaderQos_default = {NULL, QP_DataReaderQos_default_value};
