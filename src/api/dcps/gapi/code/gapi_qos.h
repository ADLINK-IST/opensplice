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
#ifndef GAPI_QOS_H
#define GAPI_QOS_H

#include "gapi.h"
#include "gapi_common.h"

#define GAPI_DURATION_INFINITE          { GAPI_DURATION_INFINITE_SEC, GAPI_DURATION_INFINITE_NSEC }
#define GAPI_DURATION_ZERO              { GAPI_DURATION_ZERO_SEC, GAPI_DURATION_ZERO_NSEC }

#define DDS_HISTORY_DEPTH_DEFAULT  (1)
#define DDS_RESOURCE_LIMIT_INFINITE (-1)

#define DEFAULT_MAX_BLOCKING_TIME  {0, 100000000} /* 100ms */
#define DDS_DEFAULT_LISTENER_STACKSIZE (0)

#define DDS_OwnershipStrengthQosPolicy_default_value { \
            0 \
        }

#define DDS_DurabilityQosPolicy_default_value { \
           GAPI_VOLATILE_DURABILITY_QOS \
        }

#define DDS_DeadlineQosPolicy_default_value { \
           GAPI_DURATION_INFINITE \
        }

#define DDS_LatencyBudgetQosPolicy_default_value { \
           GAPI_DURATION_ZERO \
        }

#define DDS_LivelinessQosPolicy_default_value { \
           GAPI_AUTOMATIC_LIVELINESS_QOS, \
           GAPI_DURATION_INFINITE \
        }

#define DDS_ReliabilityQosPolicy_default_value { \
           GAPI_BEST_EFFORT_RELIABILITY_QOS, \
           DEFAULT_MAX_BLOCKING_TIME, \
           FALSE \
        }

#define DDS_ReliabilityQosPolicy_writer_default_value { \
           GAPI_RELIABLE_RELIABILITY_QOS, \
           DEFAULT_MAX_BLOCKING_TIME, \
           FALSE \
        }

#define DDS_DestinationOrderQosPolicy_default_value { \
           GAPI_BY_RECEPTION_TIMESTAMP_DESTINATIONORDER_QOS \
        }

#define DDS_HistoryQosPolicy_default_value { \
           GAPI_KEEP_LAST_HISTORY_QOS, \
           1 \
        }

#define DDS_ResourceLimitsQosPolicy_default_value { \
           GAPI_LENGTH_UNLIMITED, \
           GAPI_LENGTH_UNLIMITED, \
           GAPI_LENGTH_UNLIMITED \
        }

#define DDS_TransportPriorityQosPolicy_default_value { \
           0 \
        }

#define DDS_LifespanQosPolicy_default_value { \
           GAPI_DURATION_INFINITE \
        }

#define DDS_OwnershipQosPolicy_default_value { \
           GAPI_SHARED_OWNERSHIP_QOS \
        }

#define DDS_PresentationQosPolicy_default_value { \
           GAPI_INSTANCE_PRESENTATION_QOS, \
           FALSE, \
           FALSE \
        }

#define DDS_EntityFactoryQosPolicy_default_value { \
           TRUE \
        }

#define DDS_WriterDataLifecycleQosPolicy_default_value { \
            TRUE, \
            GAPI_DURATION_INFINITE, \
            GAPI_DURATION_INFINITE \
        }

#define DDS_SchedulingQosPolicy_default_value { \
            { GAPI_SCHEDULE_DEFAULT }, \
            { GAPI_PRIORITY_RELATIVE }, \
            0 \
        }

#define DDS_UserDataQosPolicy_default_value { \
            { 0, 0, NULL, FALSE } \
        }

#define DDS_TopicDataQosPolicy_default_value { \
             { 0, 0, NULL, FALSE } \
        }

#define DDS_GroupDataQosPolicy_default_value { \
             { 0, 0, NULL, FALSE } \
        }

#define DDS_PartitionQosPolicy_default_value { \
             { 0, 0, NULL, FALSE } \
        }

#define DDS_ReaderDataLifecycleQosPolicy_default_value { \
             GAPI_DURATION_INFINITE, \
             GAPI_DURATION_INFINITE, \
             TRUE, \
             { GAPI_MINIMUM_INVALID_SAMPLES } \
        }

#define DDS_TimeBasedFilterQosPolicy_default_value { \
             GAPI_DURATION_ZERO \
        }

#define DDS_SubscriptionKeyQosPolicy_default_value { \
             FALSE, \
             { 0, 0, NULL, FALSE } \
        }

#define DDS_ReaderLifespanQosPolicy_default_value { \
             FALSE, \
             GAPI_DURATION_INFINITE \
        }

#define DDS_ShareQosPolicy_default_value { \
             NULL, \
             FALSE \
        }

#define DDS_ViewKeyQosPolicy_default_value { \
             FALSE, \
             { 0, 0, NULL, FALSE } \
        }

#define DDS_DurabilityServiceQosPolicy_default_value { \
            GAPI_DURATION_ZERO, \
            GAPI_KEEP_LAST_HISTORY_QOS, \
            1, \
            GAPI_LENGTH_UNLIMITED, \
            GAPI_LENGTH_UNLIMITED, \
            GAPI_LENGTH_UNLIMITED \
        }


#define DDS_DomainParticipantQos_default_value {    \
    DDS_UserDataQosPolicy_default_value,            \
    DDS_EntityFactoryQosPolicy_default_value,       \
    DDS_SchedulingQosPolicy_default_value,          \
    DDS_SchedulingQosPolicy_default_value           \
}

#define DDS_TopicQos_default_value {                \
    DDS_TopicDataQosPolicy_default_value,           \
    DDS_DurabilityQosPolicy_default_value,          \
    DDS_DurabilityServiceQosPolicy_default_value,   \
    DDS_DeadlineQosPolicy_default_value,            \
    DDS_LatencyBudgetQosPolicy_default_value,       \
    DDS_LivelinessQosPolicy_default_value,          \
    DDS_ReliabilityQosPolicy_default_value,         \
    DDS_DestinationOrderQosPolicy_default_value,    \
    DDS_HistoryQosPolicy_default_value,             \
    DDS_ResourceLimitsQosPolicy_default_value,      \
    DDS_TransportPriorityQosPolicy_default_value,   \
    DDS_LifespanQosPolicy_default_value,            \
    DDS_OwnershipQosPolicy_default_value            \
}

#define DDS_PublisherQos_default_value {            \
    DDS_PresentationQosPolicy_default_value,        \
    DDS_PartitionQosPolicy_default_value,           \
    DDS_GroupDataQosPolicy_default_value,           \
    DDS_EntityFactoryQosPolicy_default_value        \
}

#define DDS_SubscriberQos_default_value {           \
    DDS_PresentationQosPolicy_default_value,        \
    DDS_PartitionQosPolicy_default_value,           \
    DDS_GroupDataQosPolicy_default_value,           \
    DDS_EntityFactoryQosPolicy_default_value,       \
    DDS_ShareQosPolicy_default_value                \
}

#define DDS_DataReaderQos_default_value {           \
    DDS_DurabilityQosPolicy_default_value,          \
    DDS_DeadlineQosPolicy_default_value,            \
    DDS_LatencyBudgetQosPolicy_default_value,       \
    DDS_LivelinessQosPolicy_default_value,          \
    DDS_ReliabilityQosPolicy_default_value,         \
    DDS_DestinationOrderQosPolicy_default_value,    \
    DDS_HistoryQosPolicy_default_value,             \
    DDS_ResourceLimitsQosPolicy_default_value,      \
    DDS_UserDataQosPolicy_default_value,            \
    DDS_OwnershipQosPolicy_default_value,           \
    DDS_TimeBasedFilterQosPolicy_default_value,     \
    DDS_ReaderDataLifecycleQosPolicy_default_value, \
    DDS_SubscriptionKeyQosPolicy_default_value,     \
    DDS_ReaderLifespanQosPolicy_default_value,      \
    DDS_ShareQosPolicy_default_value                \
}

#define DDS_DataReaderViewQos_default_value {       \
    DDS_SubscriptionKeyQosPolicy_default_value      \
}

#define DDS_DataWriterQos_default_value {           \
    DDS_DurabilityQosPolicy_default_value,          \
    DDS_DeadlineQosPolicy_default_value,            \
    DDS_LatencyBudgetQosPolicy_default_value,       \
    DDS_LivelinessQosPolicy_default_value,          \
    DDS_ReliabilityQosPolicy_writer_default_value,  \
    DDS_DestinationOrderQosPolicy_default_value,    \
    DDS_HistoryQosPolicy_default_value,             \
    DDS_ResourceLimitsQosPolicy_default_value,      \
    DDS_TransportPriorityQosPolicy_default_value,   \
    DDS_LifespanQosPolicy_default_value,            \
    DDS_UserDataQosPolicy_default_value,            \
    DDS_OwnershipQosPolicy_default_value,           \
    DDS_OwnershipStrengthQosPolicy_default_value,   \
    DDS_WriterDataLifecycleQosPolicy_default_value  \
}

/*///////////////////////////////////////////////////*/

extern gapi_domainParticipantQos        gapi_domainParticipantQosDefault;
extern gapi_topicQos                    gapi_topicQosDefault;
extern gapi_publisherQos                gapi_publisherQosDefault;
extern gapi_subscriberQos               gapi_subscriberQosDefault;
extern gapi_dataReaderQos               gapi_dataReaderQosDefault;
extern gapi_dataReaderViewQos           gapi_dataReaderViewQosDefault;
extern gapi_dataWriterQos               gapi_dataWriterQosDefault;

gapi_returnCode_t
gapi_domainParticipantFactoryQosIsConsistent (
    const gapi_domainParticipantFactoryQos *qos,
    const gapi_context              *context);

gapi_returnCode_t
gapi_domainParticipantQosIsConsistent (
    const gapi_domainParticipantQos *qos,
    const gapi_context              *context);

gapi_returnCode_t
gapi_topicQosIsConsistent (
    const gapi_topicQos *qos,
    const gapi_context  *context);

gapi_boolean
gapi_topicQosEqual (
    const gapi_topicQos *qos1,
    const gapi_topicQos *qos2);

gapi_returnCode_t
gapi_publisherQosIsConsistent (
    const gapi_publisherQos *qos,
    const gapi_context      *context);

gapi_returnCode_t
gapi_subscriberQosIsConsistent (
    const gapi_subscriberQos *qos,
    const gapi_context       *context);

gapi_returnCode_t
gapi_dataReaderQosIsConsistent (
    const gapi_dataReaderQos *qos,
    const gapi_context       *context);


gapi_returnCode_t
gapi_dataReaderViewQosIsConsistent (
    const gapi_dataReaderViewQos *qos,
    const gapi_context           *context);

gapi_returnCode_t
gapi_dataWriterQosIsConsistent (
    const gapi_dataWriterQos *qos,
    const gapi_context       *context);

gapi_returnCode_t
gapi_domainParticipantFactoryQosCheckMutability (
    const gapi_domainParticipantFactoryQos *new_qos,
    const gapi_domainParticipantFactoryQos *old_qos,
    const gapi_context              *context);

gapi_returnCode_t
gapi_domainParticipantQosCheckMutability (
    const gapi_domainParticipantQos *new_qos,
    const gapi_domainParticipantQos *old_qos,
    const gapi_context              *context);

gapi_returnCode_t
gapi_topicQosCheckMutability (
    const gapi_topicQos *new_qos,
    const gapi_topicQos *old_qos,
    const gapi_context  *context);

gapi_returnCode_t
gapi_publisherQosCheckMutability (
    const gapi_publisherQos *new_qos,
    const gapi_publisherQos *old_qos,
    const gapi_context      *context);

gapi_returnCode_t
gapi_subscriberQosCheckMutability (
    const gapi_subscriberQos *new_qos,
    const gapi_subscriberQos *old_qos,
    const gapi_context       *context);

gapi_returnCode_t
gapi_dataReaderQosCheckMutability (
    const gapi_dataReaderQos *new_qos,
    const gapi_dataReaderQos *old_qos,
    const gapi_context       *context);

gapi_returnCode_t
gapi_dataReaderViewQosCheckMutability (
    const gapi_dataReaderViewQos *new_qos,
    const gapi_dataReaderViewQos *old_qos,
    const gapi_context           *context);


gapi_returnCode_t
gapi_dataWriterQosCheckMutability (
    const gapi_dataWriterQos *new_qos,
    const gapi_dataWriterQos *old_qos,
    const gapi_context       *context);

#endif
