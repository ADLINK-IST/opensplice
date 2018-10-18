

#include <stdio.h>
#include <string.h>

#include "os_stdlib.h"
#include "os_process.h"
#include "os_thread.h"

#include <stdio.h>
#include <string.h>

#include "dds_dcps.h"
#include "dds_dcps_private.h"

typedef struct config {
    DDS_DomainParticipant participant;
    DDS_Subscriber builtin_subscriber;
    DDS_Subscriber subscriber;
    DDS_Publisher publisher;
    DDS_WaitSet waitset;
    FILE *logfile;
    char *partition;
} *topmon_config;

typedef struct logger {
    topmon_config config;
    char *topic_name;
    DDS_DataReader builtin_reader;
    DDS_StatusCondition builtin_status_condition;
    DDS_DataWriter topmon_writer;
    DDS_DataReader topmon_reader;
    DDS_StatusCondition topmon_status_condition;
} *topmon_logger;

static DDS_DomainParticipantFactory                    factory = NULL;
static DDS_DomainParticipant                           dp      = NULL;

static topmon_logger DCPSParticipant_logger = NULL;
static topmon_logger DCPSTopic_logger = NULL;
static topmon_logger DCPSPublication_logger = NULL;
static topmon_logger DCPSSubscription_logger = NULL;
static topmon_logger CMPublisher_logger = NULL;
static topmon_logger CMSubscriber_logger = NULL;
static topmon_logger CMDataWriter_logger = NULL;
static topmon_logger CMDataReader_logger = NULL;
static topmon_logger CMParticipant_logger = NULL;
static topmon_logger DCPSType_logger = NULL;

static DDS_ConditionSeq *cs = NULL;

#if 0
static topmon_participantDataWriter wrtTm = NULL;
static topmon_participantDataReader rdrTm = NULL;
#endif

typedef struct _publicationInfo *publicationInfo;
struct _publicationInfo {
    DDS_BuiltinTopicKey_t key;
    DDS_PublicationBuiltinTopicData data;
    DDS_CMDataWriterBuiltinTopicData cmdata;
    publicationInfo next;
};

const char *
instanceState_image(
    DDS_SampleInfo *info)
{
    if (info->view_state == DDS_NEW_VIEW_STATE) {
        switch (info->instance_state) {
        case DDS_ALIVE_INSTANCE_STATE: return "NEW";
        case DDS_NOT_ALIVE_DISPOSED_INSTANCE_STATE: return "NEW DISPOSED";
        case DDS_NOT_ALIVE_NO_WRITERS_INSTANCE_STATE: return "NEW NOT ALIVE";
        default : return "INVALID STATE";
        }
    } else {
        switch (info->instance_state) {
        case DDS_ALIVE_INSTANCE_STATE: return "UPDATE";
        case DDS_NOT_ALIVE_DISPOSED_INSTANCE_STATE: return "DISPOSED";
        case DDS_NOT_ALIVE_NO_WRITERS_INSTANCE_STATE: return "NOT ALIVE";
        default : return "INVALID STATE";
        }
    }
}

const char *
DDS_DurabilityQosPolicyKind_image(
    DDS_DurabilityQosPolicyKind kind)
{
    switch (kind) {
    case DDS_VOLATILE_DURABILITY_QOS: return "VOLATILE";
    case DDS_TRANSIENT_LOCAL_DURABILITY_QOS: return "TRANSIENT_LOCAL";
    case DDS_TRANSIENT_DURABILITY_QOS: return "TRANSIENT";
    case DDS_PERSISTENT_DURABILITY_QOS: return "PERSISTENT";
    }
    return NULL;
}

const char *
DDS_ReliabilityQosPolicyKind_image(
    DDS_ReliabilityQosPolicyKind kind)
{
    switch (kind) {
    case DDS_BEST_EFFORT_RELIABILITY_QOS: return "BESTEFFORT";
    case DDS_RELIABLE_RELIABILITY_QOS: return "RELIABLE";
    }
    return NULL;
}

const char *
DDS_HistoryQosPolicyKind_image(
    DDS_HistoryQosPolicyKind kind)
{
    switch (kind) {
    case DDS_KEEP_LAST_HISTORY_QOS: return "KEEPLAST";
    case DDS_KEEP_ALL_HISTORY_QOS: return "KEEPALL";
    }
    return NULL;
}

const char *
DDS_DestinationOrderQosPolicyKind_image(
    DDS_DestinationOrderQosPolicyKind kind)
{
    switch (kind) {
    case DDS_BY_RECEPTION_TIMESTAMP_DESTINATIONORDER_QOS: return "BY_RECEPTION";
    case DDS_BY_SOURCE_TIMESTAMP_DESTINATIONORDER_QOS: return "BY_SOURCE";
    }
    return NULL;
}

const char *
DDS_OwnershipQosPolicyKind_image(
    DDS_OwnershipQosPolicyKind kind)
{
    switch (kind) {
    case DDS_SHARED_OWNERSHIP_QOS: return "SHARED";
    case DDS_EXCLUSIVE_OWNERSHIP_QOS: return "EXCLUSIVE";
    }
    return NULL;
}

char *
DDS_PartitionQosPolicy_image(
    DDS_PartitionQosPolicy partitions)
{
    unsigned int i, len;
    char *image;

    len = 0;
    for (i=0; i<partitions.name._length; i++) {
        len += strlen(partitions.name._buffer[i]);
        len++;
    }
    if (len > 0) {
        image = os_malloc(len);
        len = sprintf(image,"%s",partitions.name._buffer[0]);
        for (i=1; i<partitions.name._length; i++) {
            len += sprintf(&image[len],",%s",partitions.name._buffer[i]);
        }
    } else {
        image = strdup("*");
    }
    return image;
}

static void
logParticipant(
    DDS_ParticipantBuiltinTopicData *data,
    DDS_SampleInfo *info)
{
    /*
     * struct DDS_ParticipantBuiltinTopicData {
     *     DDS_BuiltinTopicKey_t key;
     *     DDS_UserDataQosPolicy user_data;
     * };
     * struct DDS_UserDataQosPolicy {
     *     DDS_octSeq value;
     * };
                 */
    fprintf(DCPSParticipant_logger->config->logfile,
            "%llu %s Participant id[%d,%d,%d] time[%u,%u]\n",
            (info->publication_handle << 32),
            instanceState_image(info),
            data->key[0], data->key[1], data->key[2],
            info->source_timestamp.sec, info->source_timestamp.nanosec);
}

static void
logTopic(
    DDS_TopicBuiltinTopicData *data,
    DDS_SampleInfo *info)
{
    /*
     * struct DDS_TopicBuiltinTopicData {
     *     DDS_BuiltinTopicKey_t key;
     *     DDS_string name;
     *     DDS_string type_name;
     *     DDS_DurabilityQosPolicy durability;
     *     DDS_DurabilityServiceQosPolicy durability_service;
     *     DDS_DeadlineQosPolicy deadline;
     *     DDS_LatencyBudgetQosPolicy latency_budget;
     *     DDS_LivelinessQosPolicy liveliness;
     *     DDS_ReliabilityQosPolicy reliability;
     *     DDS_TransportPriorityQosPolicy transport_priority;
     *     DDS_LifespanQosPolicy lifespan;
     *     DDS_DestinationOrderQosPolicy destination_order;
     *     DDS_HistoryQosPolicy history;
     *     DDS_ResourceLimitsQosPolicy resource_limits;
     *     DDS_OwnershipQosPolicy ownership;
     *     DDS_TopicDataQosPolicy topic_data;
     * };
     */
    fprintf(DCPSTopic_logger->config->logfile,
            "%llu %s Topic id[%u,%u,%u] time[%u,%u] name[%s] type[%s] policies[%s,%s,%s,%s,%s] limits[%d,%d,%d]\n",
            (info->publication_handle << 32),
            instanceState_image(info),
            data->key[0], data->key[1], data->key[2],
            info->source_timestamp.sec, info->source_timestamp.nanosec,
            data->name, data->type_name,
            DDS_DurabilityQosPolicyKind_image(data->durability.kind),
            DDS_ReliabilityQosPolicyKind_image(data->reliability.kind),
            DDS_HistoryQosPolicyKind_image(data->history.kind),
            DDS_DestinationOrderQosPolicyKind_image(data->destination_order.kind),
            DDS_OwnershipQosPolicyKind_image(data->ownership.kind),
            data->resource_limits.max_samples, data->resource_limits.max_instances, data->resource_limits.max_samples_per_instance);
}

static os_boolean
processParticipant(
    DDS_StatusCondition condition)
{
    DDS_ParticipantBuiltinTopicDataDataReader reader = NULL;
    DDS_sequence_DDS_ParticipantBuiltinTopicData *dataList = NULL;
    DDS_sequence_DDS_SampleInfo *infoList = NULL;
    DDS_ParticipantBuiltinTopicData *data;
    DDS_SampleInfo *info;
    DDS_ReturnCode_t result;

    dataList = DDS_sequence_DDS_ParticipantBuiltinTopicData__alloc();
    infoList = DDS_sequence_DDS_SampleInfo__alloc();

    if (condition == DCPSParticipant_logger->builtin_status_condition) {
        reader = DCPSParticipant_logger->builtin_reader;
    } else if (condition == DCPSParticipant_logger->topmon_status_condition) {
        reader = DCPSParticipant_logger->topmon_reader;
    } else {
        return OS_FALSE;
    }
    result = DDS_ParticipantBuiltinTopicDataDataReader_take(
                 reader, dataList, infoList,
                 DDS_LENGTH_UNLIMITED,
                 DDS_ANY_SAMPLE_STATE,
                 DDS_ANY_VIEW_STATE,
                 DDS_ANY_INSTANCE_STATE);

    if ( result == DDS_RETCODE_OK ) {
        unsigned int i;
        for (i=0; i<dataList->_length; i++) {
            data = &(dataList->_buffer[i]);
            info = &(infoList->_buffer[i]);
            if (reader == DCPSParticipant_logger->builtin_reader && DCPSParticipant_logger->config->publisher) {
                switch (info->instance_state) {
                case DDS_ALIVE_INSTANCE_STATE:
                    DDS_ParticipantBuiltinTopicDataDataWriter_write(DCPSParticipant_logger->topmon_writer, data, NULL);
                break;
                case DDS_NOT_ALIVE_DISPOSED_INSTANCE_STATE:
                    DDS_ParticipantBuiltinTopicDataDataWriter_dispose(DCPSParticipant_logger->topmon_writer, data, NULL);
                break;
                case DDS_NOT_ALIVE_NO_WRITERS_INSTANCE_STATE:
                    DDS_ParticipantBuiltinTopicDataDataWriter_unregister_instance(DCPSParticipant_logger->topmon_writer, data, NULL);
                break;
                }
            } else {
                logParticipant(data, info);
#if 0
                /*
                 * struct DDS_ParticipantBuiltinTopicData {
                 *     DDS_BuiltinTopicKey_t key;
                 *     DDS_UserDataQosPolicy user_data;
                 * };
                 * struct DDS_UserDataQosPolicy {
                 *     DDS_octSeq value;
                 * };
                 */
                fprintf(DCPSParticipant_logger->config->logfile,
                   "%llu %s Participant id[%d,%d,%d] time[%u,%u]\n",
                   (info->publication_handle << 32),
                   instanceState_image(info),
                   data->key[0], data->key[1], data->key[2],
                   info->source_timestamp.sec, info->source_timestamp.nanosec);
#endif
            }
        }
        DDS_ParticipantBuiltinTopicDataDataReader_return_loan(DCPSParticipant_logger->builtin_reader,
                                                              dataList, infoList);
    }
    DDS_free(dataList);
    DDS_free(infoList);
    return OS_TRUE;
}

static os_boolean
processTopic(
    DDS_StatusCondition condition)
{
    DDS_TopicBuiltinTopicDataDataReader reader = NULL;
    DDS_sequence_DDS_TopicBuiltinTopicData *dataList = NULL;
    DDS_sequence_DDS_SampleInfo *infoList = NULL;
    DDS_TopicBuiltinTopicData *data;
    DDS_SampleInfo *info;
    DDS_ReturnCode_t result;

    dataList = DDS_sequence_DDS_TopicBuiltinTopicData__alloc();
    infoList = DDS_sequence_DDS_SampleInfo__alloc();

    if (condition == DCPSTopic_logger->builtin_status_condition) {
        reader = DCPSTopic_logger->builtin_reader;
    } else if (condition == DCPSTopic_logger->topmon_status_condition) {
        reader = DCPSTopic_logger->topmon_reader;
    } else {
        return OS_FALSE;
    }
    result = DDS_TopicBuiltinTopicDataDataReader_take(
                 reader, dataList, infoList,
                 DDS_LENGTH_UNLIMITED,
                 DDS_ANY_SAMPLE_STATE,
                 DDS_ANY_VIEW_STATE,
                 DDS_ANY_INSTANCE_STATE);

    if ( result == DDS_RETCODE_OK ) {
        unsigned int i;
        for (i=0; i<dataList->_length; i++) {
            data = &(dataList->_buffer[i]);
            info = &(infoList->_buffer[i]);
            if (reader == DCPSTopic_logger->builtin_reader && DCPSTopic_logger->config->publisher) {
                switch (info->instance_state) {
                case DDS_ALIVE_INSTANCE_STATE:
                    DDS_TopicBuiltinTopicDataDataWriter_write(DCPSTopic_logger->topmon_writer, data, NULL);
                break;
                case DDS_NOT_ALIVE_DISPOSED_INSTANCE_STATE:
                    DDS_TopicBuiltinTopicDataDataWriter_dispose(DCPSTopic_logger->topmon_writer, data, NULL);
                break;
                case DDS_NOT_ALIVE_NO_WRITERS_INSTANCE_STATE:
                    DDS_TopicBuiltinTopicDataDataWriter_unregister_instance(DCPSTopic_logger->topmon_writer, data, NULL);
                break;
                }
            } else {
                logTopic(data, info);
#if 0
                /*
                 * struct DDS_TopicBuiltinTopicData {
                 *     DDS_BuiltinTopicKey_t key;
                 *     DDS_string name;
                 *     DDS_string type_name;
                 *     DDS_DurabilityQosPolicy durability;
                 *     DDS_DurabilityServiceQosPolicy durability_service;
                 *     DDS_DeadlineQosPolicy deadline;
                 *     DDS_LatencyBudgetQosPolicy latency_budget;
                 *     DDS_LivelinessQosPolicy liveliness;
                 *     DDS_ReliabilityQosPolicy reliability;
                 *     DDS_TransportPriorityQosPolicy transport_priority;
                 *     DDS_LifespanQosPolicy lifespan;
                 *     DDS_DestinationOrderQosPolicy destination_order;
                 *     DDS_HistoryQosPolicy history;
                 *     DDS_ResourceLimitsQosPolicy resource_limits;
                 *     DDS_OwnershipQosPolicy ownership;
                 *     DDS_TopicDataQosPolicy topic_data;
                 * };
                 */
                fprintf(DCPSTopic_logger->config->logfile,
                   "%llu %s Topic id[%u,%u,%u] time[%u,%u] name[%s] type[%s] policies[%s,%s,%s,%s,%s] limits[%d,%d,%d]\n",
                   (info->publication_handle << 32),
                   instanceState_image(info),
                   data->key[0], data->key[1], data->key[2],
                   info->source_timestamp.sec, info->source_timestamp.nanosec,
                   data->name, data->type_name,
                   DDS_DurabilityQosPolicyKind_image(data->durability.kind),
                   DDS_ReliabilityQosPolicyKind_image(data->reliability.kind),
                   DDS_HistoryQosPolicyKind_image(data->history.kind),
                   DDS_DestinationOrderQosPolicyKind_image(data->destination_order.kind),
                   DDS_OwnershipQosPolicyKind_image(data->ownership.kind),
                   data->resource_limits.max_samples, data->resource_limits.max_instances, data->resource_limits.max_samples_per_instance);
#endif
            }
        }
        DDS_TopicBuiltinTopicDataDataReader_return_loan(DCPSTopic_logger->builtin_reader, dataList, infoList);
    }
    DDS_free(dataList);
    DDS_free(infoList);
    return OS_TRUE;
}

static os_boolean
processPublication(
    DDS_StatusCondition condition)
{
    DDS_PublicationBuiltinTopicDataDataReader reader = NULL;
    DDS_sequence_DDS_PublicationBuiltinTopicData *dataList = NULL;
    DDS_sequence_DDS_SampleInfo *infoList = NULL;
    DDS_PublicationBuiltinTopicData *data;
    DDS_SampleInfo *info;
    DDS_ReturnCode_t result;

    dataList = DDS_sequence_DDS_PublicationBuiltinTopicData__alloc();
    infoList = DDS_sequence_DDS_SampleInfo__alloc();

    if (condition == DCPSPublication_logger->builtin_status_condition) {
        reader = DCPSPublication_logger->builtin_reader;
    } else if (condition == DCPSPublication_logger->topmon_status_condition) {
        reader = DCPSPublication_logger->topmon_reader;
    } else {
        return OS_FALSE;
    }
    result = DDS_PublicationBuiltinTopicDataDataReader_take(
                 reader, dataList, infoList,
                 DDS_LENGTH_UNLIMITED,
                 DDS_ANY_SAMPLE_STATE,
                 DDS_ANY_VIEW_STATE,
                 DDS_ANY_INSTANCE_STATE);

    if ( result == DDS_RETCODE_OK ) {
        unsigned int i;
        for (i=0; i<dataList->_length; i++) {
            data = &(dataList->_buffer[i]);
            info = &(infoList->_buffer[i]);
            if (reader == DCPSPublication_logger->builtin_reader && DCPSPublication_logger->config->publisher) {
                switch (info->instance_state) {
                case DDS_ALIVE_INSTANCE_STATE:
                    DDS_PublicationBuiltinTopicDataDataWriter_write(DCPSPublication_logger->topmon_writer, data, NULL);
                break;
                case DDS_NOT_ALIVE_DISPOSED_INSTANCE_STATE:
                    DDS_PublicationBuiltinTopicDataDataWriter_dispose(DCPSPublication_logger->topmon_writer, data, NULL);
                break;
                case DDS_NOT_ALIVE_NO_WRITERS_INSTANCE_STATE:
                    DDS_PublicationBuiltinTopicDataDataWriter_unregister_instance(DCPSPublication_logger->topmon_writer, data, NULL);
                break;
                }
            } else {
                char *partition = NULL;
                /*
                 * struct DDS_PublicationBuiltinTopicData {
                 *     DDS_BuiltinTopicKey_t key;
                 *     DDS_BuiltinTopicKey_t participant_key;
                 *     DDS_string topic_name;
                 *     DDS_string type_name;
                 *     DDS_DurabilityQosPolicy durability;
                 *     DDS_DeadlineQosPolicy deadline;
                 *     DDS_LatencyBudgetQosPolicy latency_budget;
                 *     DDS_LivelinessQosPolicy liveliness;
                 *     DDS_ReliabilityQosPolicy reliability;
                 *     DDS_LifespanQosPolicy lifespan;
                 *     DDS_DestinationOrderQosPolicy destination_order;
                 *     DDS_UserDataQosPolicy user_data;
                 *     DDS_OwnershipQosPolicy ownership;
                 *     DDS_OwnershipStrengthQosPolicy ownership_strength;
                 *     DDS_PresentationQosPolicy presentation;
                 *     DDS_PartitionQosPolicy partition;
                 *     DDS_TopicDataQosPolicy topic_data;
                 *     DDS_GroupDataQosPolicy group_data;
                 * };
                 */
                partition = DDS_PartitionQosPolicy_image(data->partition);
                fprintf(DCPSPublication_logger->config->logfile,
                   "%llu %s DataWriter id[%d,%d,%d] time[%u,%u] topic[%s] partition[%s] policies[%s,%s,%s,%s]\n",
                   (info->publication_handle << 32),
                   instanceState_image(info),
                   data->key[0], data->key[1], data->key[2],
                   info->source_timestamp.sec, info->source_timestamp.nanosec,
                   data->topic_name,
                   partition,
                   DDS_DurabilityQosPolicyKind_image(data->durability.kind),
                   DDS_ReliabilityQosPolicyKind_image(data->reliability.kind),
                   DDS_DestinationOrderQosPolicyKind_image(data->destination_order.kind),
                   DDS_OwnershipQosPolicyKind_image(data->ownership.kind));
                os_free(partition);
            }
        }
        DDS_PublicationBuiltinTopicDataDataReader_return_loan(DCPSPublication_logger->builtin_reader, dataList, infoList);
    }
    DDS_free(dataList);
    DDS_free(infoList);
    return OS_TRUE;
}

static os_boolean
processSubscription(
    DDS_StatusCondition condition)
{
    DDS_SubscriptionBuiltinTopicDataDataReader reader = NULL;
    DDS_sequence_DDS_SubscriptionBuiltinTopicData *dataList = NULL;
    DDS_sequence_DDS_SampleInfo *infoList = NULL;
    DDS_SubscriptionBuiltinTopicData *data;
    DDS_SampleInfo *info;
    DDS_ReturnCode_t result;

    dataList = DDS_sequence_DDS_SubscriptionBuiltinTopicData__alloc();
    infoList = DDS_sequence_DDS_SampleInfo__alloc();

    if (condition == DCPSSubscription_logger->builtin_status_condition) {
        reader = DCPSSubscription_logger->builtin_reader;
    } else if (condition == DCPSSubscription_logger->topmon_status_condition) {
        reader = DCPSSubscription_logger->topmon_reader;
    } else {
        return OS_FALSE;
    }
    result = DDS_SubscriptionBuiltinTopicDataDataReader_take(
                 reader, dataList, infoList,
                 DDS_LENGTH_UNLIMITED,
                 DDS_ANY_SAMPLE_STATE,
                 DDS_ANY_VIEW_STATE,
                 DDS_ANY_INSTANCE_STATE);

    if ( result == DDS_RETCODE_OK ) {
        unsigned int i;
        for (i=0; i<dataList->_length; i++) {
            data = &(dataList->_buffer[i]);
            info = &(infoList->_buffer[i]);
            if (reader == DCPSSubscription_logger->builtin_reader && DCPSSubscription_logger->config->publisher) {
                switch (info->instance_state) {
                case DDS_ALIVE_INSTANCE_STATE:
                    DDS_SubscriptionBuiltinTopicDataDataWriter_write(DCPSSubscription_logger->topmon_writer, data, NULL);
                break;
                case DDS_NOT_ALIVE_DISPOSED_INSTANCE_STATE:
                    DDS_SubscriptionBuiltinTopicDataDataWriter_dispose(DCPSSubscription_logger->topmon_writer, data, NULL);
                break;
                case DDS_NOT_ALIVE_NO_WRITERS_INSTANCE_STATE:
                    DDS_SubscriptionBuiltinTopicDataDataWriter_unregister_instance(DCPSSubscription_logger->topmon_writer, data, NULL);
                break;
                }
            } else {
                char *partition = NULL;
                /*
                 * struct DDS_SubscriptionBuiltinTopicData {
                 *     DDS_BuiltinTopicKey_t key;
                 *     DDS_BuiltinTopicKey_t participant_key;
                 *     DDS_string topic_name;
                 *     DDS_string type_name;
                 *     DDS_DurabilityQosPolicy durability;
                 *     DDS_DeadlineQosPolicy deadline;
                 *     DDS_LatencyBudgetQosPolicy latency_budget;
                 *     DDS_LivelinessQosPolicy liveliness;
                 *     DDS_ReliabilityQosPolicy reliability;
                 *     DDS_OwnershipQosPolicy ownership;
                 *     DDS_DestinationOrderQosPolicy destination_order;
                 *     DDS_UserDataQosPolicy user_data;
                 *     DDS_TimeBasedFilterQosPolicy time_based_filter;
                 *     DDS_PresentationQosPolicy presentation;
                 *     DDS_PartitionQosPolicy partition;
                 *     DDS_TopicDataQosPolicy topic_data;
                 *     DDS_GroupDataQosPolicy group_data;
                 * };
                 */
                partition = DDS_PartitionQosPolicy_image(data->partition);
                fprintf(DCPSSubscription_logger->config->logfile,
                   "%llu %s DataReader id[%d,%d,%d] time[%u,%u] participant[%d,%d,%d] topic[%s] partition[%s] policies[%s,%s,%s,%s]\n",
                   (info->publication_handle << 32),
                   instanceState_image(info),
                   data->key[0], data->key[1], data->key[2],
                   info->source_timestamp.sec, info->source_timestamp.nanosec,
                   data->participant_key[0], data->participant_key[1], data->participant_key[2],
                   data->topic_name,
                   partition,
                   DDS_DurabilityQosPolicyKind_image(data->durability.kind),
                   DDS_ReliabilityQosPolicyKind_image(data->reliability.kind),
                   DDS_DestinationOrderQosPolicyKind_image(data->destination_order.kind),
                   DDS_OwnershipQosPolicyKind_image(data->ownership.kind));
                os_free(partition);
            }
        }
        DDS_SubscriptionBuiltinTopicDataDataReader_return_loan(DCPSSubscription_logger->builtin_reader, dataList, infoList);
    }
    DDS_free(dataList);
    DDS_free(infoList);
    return OS_TRUE;
}

static os_boolean
processCmPublisher(
    DDS_StatusCondition condition)
{
    DDS_CMPublisherBuiltinTopicDataDataReader reader = NULL;
    DDS_sequence_DDS_CMPublisherBuiltinTopicData *dataList = NULL;
    DDS_sequence_DDS_SampleInfo *infoList = NULL;
    DDS_CMPublisherBuiltinTopicData *data;
    DDS_SampleInfo *info;
    DDS_ReturnCode_t result;

    dataList = DDS_sequence_DDS_CMPublisherBuiltinTopicData__alloc();
    infoList = DDS_sequence_DDS_SampleInfo__alloc();

    if (condition == CMPublisher_logger->builtin_status_condition) {
        reader = CMPublisher_logger->builtin_reader;
    } else if (condition == CMPublisher_logger->topmon_status_condition) {
        reader = CMPublisher_logger->topmon_reader;
    } else {
        return OS_FALSE;
    }
    result = DDS_CMPublisherBuiltinTopicDataDataReader_take(
                 reader, dataList, infoList,
                 DDS_LENGTH_UNLIMITED,
                 DDS_ANY_SAMPLE_STATE,
                 DDS_ANY_VIEW_STATE,
                 DDS_ANY_INSTANCE_STATE);

    /*
     * struct DDS_CMPublisherBuiltinTopicData {
     *     DDS_BuiltinTopicKey_t key;
     *     DDS_ProductDataQosPolicy product;
     *     DDS_BuiltinTopicKey_t participant_key;
     *     DDS_string name;
     *     DDS_EntityFactoryQosPolicy entity_factory;
     *     DDS_PartitionQosPolicy partition;
     * };
     */
    if ( result == DDS_RETCODE_OK ) {
        unsigned int i;
        for (i=0; i<dataList->_length; i++) {
            data = &(dataList->_buffer[i]);
            info = &(infoList->_buffer[i]);
            if (reader == CMPublisher_logger->builtin_reader && CMPublisher_logger->config->publisher) {
                switch (info->instance_state) {
                case DDS_ALIVE_INSTANCE_STATE:
                    DDS_CMPublisherBuiltinTopicDataDataWriter_write(CMPublisher_logger->topmon_writer, data, NULL);
                break;
                case DDS_NOT_ALIVE_DISPOSED_INSTANCE_STATE:
                    DDS_CMPublisherBuiltinTopicDataDataWriter_dispose(CMPublisher_logger->topmon_writer, data, NULL);
                break;
                case DDS_NOT_ALIVE_NO_WRITERS_INSTANCE_STATE:
                    DDS_CMPublisherBuiltinTopicDataDataWriter_unregister_instance(CMPublisher_logger->topmon_writer, data, NULL);
                break;
                }
            } else {
                fprintf(CMPublisher_logger->config->logfile,
                   "%llu %s Publisher id[%u,%u,%u] time[%u,%u] name[%s] product[%s]\n",
                   (info->publication_handle << 32),
                   instanceState_image(info),
                   data->key[0], data->key[1], data->key[2],
                   info->source_timestamp.sec, info->source_timestamp.nanosec,
                   data->name,
                   data->product.value ? data->product.value : "");
            }
        }
        DDS_CMPublisherBuiltinTopicDataDataReader_return_loan(CMPublisher_logger->builtin_reader, dataList, infoList);
    }
    DDS_free(dataList);
    DDS_free(infoList);
    return OS_TRUE;
}

static os_boolean
processCmSubscriber(
    DDS_StatusCondition condition)
{
    DDS_CMSubscriberBuiltinTopicDataDataReader reader = NULL;
    DDS_sequence_DDS_CMSubscriberBuiltinTopicData *dataList = NULL;
    DDS_sequence_DDS_SampleInfo *infoList = NULL;
    DDS_CMSubscriberBuiltinTopicData *data;
    DDS_SampleInfo *info;
    DDS_ReturnCode_t result;

    dataList = DDS_sequence_DDS_CMSubscriberBuiltinTopicData__alloc();
    infoList = DDS_sequence_DDS_SampleInfo__alloc();

    if (condition == CMSubscriber_logger->builtin_status_condition) {
        reader = CMSubscriber_logger->builtin_reader;
    } else if (condition == CMSubscriber_logger->topmon_status_condition) {
        reader = CMSubscriber_logger->topmon_reader;
    } else {
        return OS_FALSE;
    }
    result = DDS_CMSubscriberBuiltinTopicDataDataReader_take(
                 reader, dataList, infoList,
                 DDS_LENGTH_UNLIMITED,
                 DDS_ANY_SAMPLE_STATE,
                 DDS_ANY_VIEW_STATE,
                 DDS_ANY_INSTANCE_STATE);

    if ( result == DDS_RETCODE_OK ) {
        unsigned int i;
        for (i=0; i<dataList->_length; i++) {
            data = &(dataList->_buffer[i]);
            info = &(infoList->_buffer[i]);
            if (reader == CMSubscriber_logger->builtin_reader && CMSubscriber_logger->config->publisher) {
                switch (info->instance_state) {
                case DDS_ALIVE_INSTANCE_STATE:
                    DDS_CMSubscriberBuiltinTopicDataDataWriter_write(CMSubscriber_logger->topmon_writer, data, NULL);
                break;
                case DDS_NOT_ALIVE_DISPOSED_INSTANCE_STATE:
                    DDS_CMSubscriberBuiltinTopicDataDataWriter_dispose(CMSubscriber_logger->topmon_writer, data, NULL);
                break;
                case DDS_NOT_ALIVE_NO_WRITERS_INSTANCE_STATE:
                    DDS_CMSubscriberBuiltinTopicDataDataWriter_unregister_instance(CMSubscriber_logger->topmon_writer, data, NULL);
                break;
                }
            } else {
                /*
                 * struct DDS_CMSubscriberBuiltinTopicData {
                 *     DDS_BuiltinTopicKey_t key;
                 *     DDS_ProductDataQosPolicy product;
                 *     DDS_BuiltinTopicKey_t participant_key;
                 *     DDS_string name;
                 *     DDS_EntityFactoryQosPolicy entity_factory;
                 *     DDS_ShareQosPolicy share;
                 *     DDS_PartitionQosPolicy partition;
                 * };
                 */
                fprintf(CMSubscriber_logger->config->logfile,
                   "%llu %s Subscriber id[%u,%u,%u] time[%u,%u] name[%s] product[%s]\n",
                   (info->publication_handle << 32),
                   instanceState_image(info),
                   data->key[0], data->key[1], data->key[2],
                   info->source_timestamp.sec, info->source_timestamp.nanosec,
                   data->name,
                   data->product.value ? data->product.value : "");
            }
        }
        DDS_CMSubscriberBuiltinTopicDataDataReader_return_loan(CMSubscriber_logger->builtin_reader, dataList, infoList);
    }
    DDS_free(dataList);
    DDS_free(infoList);
    return OS_TRUE;
}

static os_boolean
processCmWriter(
    DDS_StatusCondition condition)
{
    DDS_CMDataWriterBuiltinTopicDataDataReader reader = NULL;
    DDS_sequence_DDS_CMDataWriterBuiltinTopicData *dataList = NULL;
    DDS_sequence_DDS_SampleInfo *infoList = NULL;
    DDS_CMDataWriterBuiltinTopicData *data;
    DDS_SampleInfo *info;
    DDS_ReturnCode_t result;

    dataList = DDS_sequence_DDS_CMDataWriterBuiltinTopicData__alloc();
    infoList = DDS_sequence_DDS_SampleInfo__alloc();

    if (condition == CMDataWriter_logger->builtin_status_condition) {
        reader = CMDataWriter_logger->builtin_reader;
    } else if (condition == CMDataWriter_logger->topmon_status_condition) {
        reader = CMDataWriter_logger->topmon_reader;
    } else {
        return OS_FALSE;
    }
    result = DDS_CMDataWriterBuiltinTopicDataDataReader_take(
                 reader, dataList, infoList,
                 DDS_LENGTH_UNLIMITED,
                 DDS_ANY_SAMPLE_STATE,
                 DDS_ANY_VIEW_STATE,
                 DDS_ANY_INSTANCE_STATE);

    if ( result == DDS_RETCODE_OK ) {
        unsigned int i;
        for (i=0; i<dataList->_length; i++) {
            data = &(dataList->_buffer[i]);
            info = &(infoList->_buffer[i]);
            if (reader == CMDataWriter_logger->builtin_reader && CMDataWriter_logger->config->publisher) {
                switch (info->instance_state) {
                case DDS_ALIVE_INSTANCE_STATE:
                    DDS_CMDataWriterBuiltinTopicDataDataWriter_write(CMDataWriter_logger->topmon_writer, data, NULL);
                break;
                case DDS_NOT_ALIVE_DISPOSED_INSTANCE_STATE:
                    DDS_CMDataWriterBuiltinTopicDataDataWriter_dispose(CMDataWriter_logger->topmon_writer, data, NULL);
                break;
                case DDS_NOT_ALIVE_NO_WRITERS_INSTANCE_STATE:
                    DDS_CMDataWriterBuiltinTopicDataDataWriter_unregister_instance(CMDataWriter_logger->topmon_writer, data, NULL);
                break;
                }
            } else {
                /*
                 * struct DDS_CMDataWriterBuiltinTopicData {
                 *     DDS_BuiltinTopicKey_t key;
                 *     DDS_ProductDataQosPolicy product;
                 *     DDS_BuiltinTopicKey_t publisher_key;
                 *     DDS_string name;
                 *     DDS_HistoryQosPolicy history;
                 *     DDS_ResourceLimitsQosPolicy resource_limits;
                 *     DDS_WriterDataLifecycleQosPolicy writer_data_lifecycle;
                 * };
                 */
                fprintf(CMDataWriter_logger->config->logfile,
                   "%llu %s DataWriter id[%u,%u,%u] time[%u,%u] name[%s] policies[%s] product[%s] limits[%d,%d,%d]\n",
                   (info->publication_handle << 32),
                   instanceState_image(info),
                   data->key[0], data->key[1], data->key[2],
                   info->source_timestamp.sec, info->source_timestamp.nanosec,
                   data->name,
                   DDS_HistoryQosPolicyKind_image(data->history.kind),
                   data->product.value ? data->product.value : "",
                   data->resource_limits.max_samples, data->resource_limits.max_instances, data->resource_limits.max_samples_per_instance);
            }
        }
        DDS_CMDataWriterBuiltinTopicDataDataReader_return_loan(CMDataWriter_logger->builtin_reader, dataList, infoList);
    }
    DDS_free(dataList);
    DDS_free(infoList);
    return OS_TRUE;
}

static os_boolean
processCmReader(
    DDS_StatusCondition condition)
{
    DDS_CMDataReaderBuiltinTopicDataDataReader reader = NULL;
    DDS_sequence_DDS_CMDataReaderBuiltinTopicData *dataList = NULL;
    DDS_sequence_DDS_SampleInfo *infoList = NULL;
    DDS_CMDataReaderBuiltinTopicData *data;
    DDS_SampleInfo *info;
    DDS_ReturnCode_t result;

    dataList = DDS_sequence_DDS_CMDataReaderBuiltinTopicData__alloc();
    infoList = DDS_sequence_DDS_SampleInfo__alloc();

    if (condition == CMDataReader_logger->builtin_status_condition) {
        reader = CMDataReader_logger->builtin_reader;
    } else if (condition == CMDataReader_logger->topmon_status_condition) {
        reader = CMDataReader_logger->topmon_reader;
    } else {
        return OS_FALSE;
    }
    result = DDS_CMDataReaderBuiltinTopicDataDataReader_take(
                 reader, dataList, infoList,
                 DDS_LENGTH_UNLIMITED,
                 DDS_ANY_SAMPLE_STATE,
                 DDS_ANY_VIEW_STATE,
                 DDS_ANY_INSTANCE_STATE);

    if ( result == DDS_RETCODE_OK ) {
        unsigned int i;
        for (i=0; i<dataList->_length; i++) {
            data = &(dataList->_buffer[i]);
            info = &(infoList->_buffer[i]);
            if (reader == CMDataReader_logger->builtin_reader && CMDataReader_logger->config->publisher) {
                switch (info->instance_state) {
                case DDS_ALIVE_INSTANCE_STATE:
                    DDS_CMDataReaderBuiltinTopicDataDataWriter_write(CMDataReader_logger->topmon_writer, data, NULL);
                break;
                case DDS_NOT_ALIVE_DISPOSED_INSTANCE_STATE:
                    DDS_CMDataReaderBuiltinTopicDataDataWriter_dispose(CMDataReader_logger->topmon_writer, data, NULL);
                break;
                case DDS_NOT_ALIVE_NO_WRITERS_INSTANCE_STATE:
                    DDS_CMDataReaderBuiltinTopicDataDataWriter_unregister_instance(CMDataReader_logger->topmon_writer, data, NULL);
                break;
                }
            } else {
                /*
                 * struct DDS_CMDataReaderBuiltinTopicData {
                 *     DDS_BuiltinTopicKey_t key;
                 *     DDS_ProductDataQosPolicy product;
                 *     DDS_BuiltinTopicKey_t subscriber_key;
                 *     DDS_string name;
                 *     DDS_HistoryQosPolicy history;
                 *     DDS_ResourceLimitsQosPolicy resource_limits;
                 *     DDS_ReaderDataLifecycleQosPolicy reader_data_lifecycle;
                 *     DDS_UserKeyQosPolicy subscription_keys;
                 *     DDS_ReaderLifespanQosPolicy reader_lifespan;
                 *     DDS_ShareQosPolicy share;
                 * };
                 */
                fprintf(CMDataReader_logger->config->logfile,
                   "%llu %s DataReader id[%u,%u,%u] time[%u,%u] name[%s] policies[%s] product[%s] limits[%d,%d,%d]\n",
                   (info->publication_handle << 32),
                   instanceState_image(info),
                   data->key[0], data->key[1], data->key[2],
                   info->source_timestamp.sec, info->source_timestamp.nanosec,
                   data->name,
                   DDS_HistoryQosPolicyKind_image(data->history.kind),
                   data->product.value ? data->product.value : "",
                   data->resource_limits.max_samples, data->resource_limits.max_instances, data->resource_limits.max_samples_per_instance);
            }
        }
        DDS_CMDataReaderBuiltinTopicDataDataReader_return_loan(CMDataReader_logger->builtin_reader, dataList, infoList);
    }
    DDS_free(dataList);
    DDS_free(infoList);
    return OS_TRUE;
}

static os_boolean
processCmParticipant(
    DDS_StatusCondition condition)
{
    DDS_CMParticipantBuiltinTopicDataDataReader reader = NULL;
    DDS_sequence_DDS_CMParticipantBuiltinTopicData *dataList = NULL;
    DDS_sequence_DDS_SampleInfo *infoList = NULL;
    DDS_CMParticipantBuiltinTopicData *data;
    DDS_SampleInfo *info;
    DDS_ReturnCode_t result;

    dataList = DDS_sequence_DDS_CMParticipantBuiltinTopicData__alloc();
    infoList = DDS_sequence_DDS_SampleInfo__alloc();

    if (condition == CMParticipant_logger->builtin_status_condition) {
        reader = CMParticipant_logger->builtin_reader;
    } else if (condition == CMParticipant_logger->topmon_status_condition) {
        reader = CMParticipant_logger->topmon_reader;
    } else {
        return OS_FALSE;
    }
    result = DDS_CMParticipantBuiltinTopicDataDataReader_take(
                 reader, dataList, infoList,
                 DDS_LENGTH_UNLIMITED,
                 DDS_ANY_SAMPLE_STATE,
                 DDS_ANY_VIEW_STATE,
                 DDS_ANY_INSTANCE_STATE);

    /*
     * struct DDS_CMParticipantBuiltinTopicData {
     *     DDS_BuiltinTopicKey_t key;
     *     DDS_ProductDataQosPolicy product;
     * };
     */
    if ( result == DDS_RETCODE_OK ) {
        unsigned int i;
        for (i=0; i<dataList->_length; i++) {
            data = &(dataList->_buffer[i]);
            info = &(infoList->_buffer[i]);
            if (reader == CMParticipant_logger->builtin_reader && CMParticipant_logger->config->publisher) {
                switch (info->instance_state) {
                case DDS_ALIVE_INSTANCE_STATE:
                    DDS_CMParticipantBuiltinTopicDataDataWriter_write(CMParticipant_logger->topmon_writer, data, NULL);
                break;
                case DDS_NOT_ALIVE_DISPOSED_INSTANCE_STATE:
                    DDS_CMParticipantBuiltinTopicDataDataWriter_dispose(CMParticipant_logger->topmon_writer, data, NULL);
                break;
                case DDS_NOT_ALIVE_NO_WRITERS_INSTANCE_STATE:
                    DDS_CMParticipantBuiltinTopicDataDataWriter_unregister_instance(CMParticipant_logger->topmon_writer, data, NULL);
                break;
                }
            } else {
                fprintf(CMParticipant_logger->config->logfile,
                   "%llu %s Participant id[%u,%u,%u] time[%u,%u] product[%s]\n",
                   (info->publication_handle << 32),
                   instanceState_image(info),
                   data->key[0], data->key[1], data->key[2],
                   info->source_timestamp.sec, info->source_timestamp.nanosec,
                   data->product.value ? data->product.value : "");
            }
        }
        DDS_CMParticipantBuiltinTopicDataDataReader_return_loan(CMParticipant_logger->builtin_reader, dataList, infoList);
    }
    DDS_free(dataList);
    DDS_free(infoList);
    return OS_TRUE;
}

static os_boolean
processType(
    DDS_StatusCondition condition)
{
    DDS_TypeBuiltinTopicDataDataReader reader = NULL;
    DDS_sequence_DDS_TypeBuiltinTopicData *dataList = NULL;
    DDS_sequence_DDS_SampleInfo *infoList = NULL;
    DDS_TypeBuiltinTopicData *data;
    DDS_SampleInfo *info;
    DDS_ReturnCode_t result;

    dataList = DDS_sequence_DDS_TypeBuiltinTopicData__alloc();
    infoList = DDS_sequence_DDS_SampleInfo__alloc();

    if (condition == DCPSType_logger->builtin_status_condition) {
        reader = DCPSType_logger->builtin_reader;
    } else if (condition == DCPSType_logger->topmon_status_condition) {
        reader = DCPSType_logger->topmon_reader;
    } else {
        return OS_FALSE;
    }
    result = DDS_TypeBuiltinTopicDataDataReader_take(
                 reader, dataList, infoList,
                 DDS_LENGTH_UNLIMITED,
                 DDS_ANY_SAMPLE_STATE,
                 DDS_ANY_VIEW_STATE,
                 DDS_ANY_INSTANCE_STATE);

    if ( result == DDS_RETCODE_OK ) {
        unsigned int i;
        for (i=0; i<dataList->_length; i++) {
            data = &(dataList->_buffer[i]);
            info = &(infoList->_buffer[i]);
            if (reader == DCPSType_logger->builtin_reader && DCPSType_logger->config->publisher) {
                switch (info->instance_state) {
                case DDS_ALIVE_INSTANCE_STATE:
                    DDS_TypeBuiltinTopicDataDataWriter_write(DCPSType_logger->topmon_writer, data, NULL);
                break;
                case DDS_NOT_ALIVE_DISPOSED_INSTANCE_STATE:
                    DDS_TypeBuiltinTopicDataDataWriter_dispose(DCPSType_logger->topmon_writer, data, NULL);
                break;
                case DDS_NOT_ALIVE_NO_WRITERS_INSTANCE_STATE:
                    DDS_TypeBuiltinTopicDataDataWriter_unregister_instance(DCPSType_logger->topmon_writer, data, NULL);
                break;
                }
            } else {
                /*
                 * struct DDS_TypeBuiltinTopicData {
                 *     DDS_string name;
                 *     DDS_DataRepresentationId_t data_representation_id;
                 *     DDS_TypeHash type_hash;
                 *     DDS_octSeq meta_data;
                 *     DDS_octSeq extentions;
                 * };
                 */
                fprintf(DCPSType_logger->config->logfile,
                   "%llu %s DCPSType name[%s] time[%u,%u]\n",
                   (info->publication_handle << 32),
                   instanceState_image(info),
                   data->name,
                   info->source_timestamp.sec, info->source_timestamp.nanosec);
            }
        }
        DDS_TypeBuiltinTopicDataDataReader_return_loan(DCPSType_logger->builtin_reader, dataList, infoList);
    }
    DDS_free(dataList);
    DDS_free(infoList);
    return OS_TRUE;
}

topmon_config
topmon_configNew(
    DDS_DomainParticipant participant,
    char *partition,
    FILE *logfile)
{
    topmon_config config = NULL;

    config = os_malloc(sizeof(*config));
    config->participant = participant;
    config->partition = partition ? strdup(partition) : NULL;
    config->logfile = logfile ? logfile : stdout;
    config->builtin_subscriber = DDS_DomainParticipant_get_builtin_subscriber(participant);
    config->waitset = DDS_WaitSet__alloc();

    if (partition) {
        DDS_PublisherQos *pqos;
        DDS_SubscriberQos *sqos;

        pqos = DDS_PublisherQos__alloc();
        (void)DDS_DomainParticipant_get_default_publisher_qos(dp, pqos);
        pqos->partition.name._maximum   = 1;
        pqos->partition.name._length    = 1;
        pqos->partition.name._buffer    = DDS_StringSeq_allocbuf(1);
        pqos->partition.name._buffer[0] = DDS_string_dup(partition);

        sqos = DDS_SubscriberQos__alloc();
        (void)DDS_DomainParticipant_get_default_subscriber_qos(dp, sqos);
        sqos->partition.name._maximum   = 1;
        sqos->partition.name._length    = 1;
        sqos->partition.name._buffer    = DDS_StringSeq_allocbuf(1);
        sqos->partition.name._buffer[0] = DDS_string_dup(partition);

        config->publisher = DDS_DomainParticipant_create_publisher(participant, pqos, NULL, 0);
        config->subscriber = DDS_DomainParticipant_create_subscriber(participant, sqos, NULL, 0);
    } else {
        config->publisher = NULL;
        config->subscriber = NULL;
    }
    return config;
}

topmon_logger
topmon_loggerNew(
    topmon_config config,
    const char *topic_name)
{
    DDS_ReturnCode_t result;
    topmon_logger _this = NULL;
    DDS_Topic topic;

    _this = os_malloc(sizeof(*_this));
    if (_this) {
        _this->config = config;
        _this->topic_name = strdup(topic_name);
        _this->builtin_reader =
               DDS_Subscriber_lookup_datareader(config->builtin_subscriber, topic_name);
        _this->builtin_status_condition =
               DDS_ParticipantBuiltinTopicDataDataReader_get_statuscondition(_this->builtin_reader);
        result = DDS_StatusCondition_set_enabled_statuses(_this->builtin_status_condition, DDS_DATA_AVAILABLE_STATUS);
        if (result != DDS_RETCODE_OK) {
            printf("Error faild to set status conditions on builtin_status_condition\n");
        }
        result = DDS_WaitSet_attach_condition(config->waitset, _this->builtin_status_condition);
        if (result != DDS_RETCODE_OK) {
            printf("Error faild to attach builtin_status_condition to waitset\n");
        }
        topic = DDS_DomainParticipant_lookup_topicdescription(config->participant, topic_name);
        if (config->publisher) {
            _this->topmon_writer =
                   DDS_Publisher_create_datawriter(config->publisher, topic, DDS_DATAWRITER_QOS_USE_TOPIC_QOS, NULL, 0);
        } else {
            _this->topmon_writer = NULL;
        }
        if (config->subscriber) {
            DDS_DataReaderQos rqos;

            memset(&rqos,0, sizeof(rqos));
            result = DDS_Subscriber_get_default_datareader_qos(config->subscriber, &rqos);
            rqos.history.kind = DDS_KEEP_ALL_HISTORY_QOS;

            _this->topmon_reader =
                   DDS_Subscriber_create_datareader(config->subscriber, topic, &rqos, NULL, 0);
            _this->topmon_status_condition =
                   DDS_ParticipantBuiltinTopicDataDataReader_get_statuscondition(_this->topmon_reader);
            result = DDS_StatusCondition_set_enabled_statuses(_this->topmon_status_condition, DDS_DATA_AVAILABLE_STATUS);
            if (result != DDS_RETCODE_OK) {
                printf("Error faild to set status conditions on topmon_status_condition\n");
            }
            result = DDS_WaitSet_attach_condition(config->waitset, _this->topmon_status_condition);
            if (result != DDS_RETCODE_OK) {
                printf("Error faild to attach topmon_status_condition to waitset\n");
            }
        } else {
            _this->topmon_reader = NULL;
            _this->topmon_status_condition = NULL;
        }
        DDS_free(topic);
    }
    return _this;
}


int
main(int argc, char *argv[])
{
    DDS_Duration_t infinite = DDS_DURATION_INFINITE;
    DDS_ReturnCode_t result;
    topmon_config config;
    FILE *logfile = NULL;
    int opt;
    char *partition = NULL;

    while ((opt = getopt (argc, argv, "p:f:")) != -1) {
        switch (opt) {
        case 'p': partition = strdup(optarg); break;
        case 'f': logfile = fopen(optarg, "w"); break;
        case '?': printf("**** topmon usage: ****\n*\n");
                  printf("* topmon [-p <partition>] [-f <logfile>]\n*\n");
                  printf("* topmon\n");
                  printf("* -- print local topology state changes to stdout\n*\n");
                  printf("* topmon -f <filename>\n");
                  printf("* -- log local topology state changes to <filename>\n*\n");
                  printf("* topmon -p <partition>\n");
                  printf("* -- re-publish local topology state changes to <partition>\n*\n");
                  printf("* topmon -p <partition> -f <filename>\n");
                  printf("* -- re-publish local topology state changes to <partition> and log global topology state changes to <filename>\n*\n");
                  printf("***********************\n");
                  return 0;
        break;
        default: break;
        }
    }

    factory = DDS_DomainParticipantFactory_get_instance();
    dp = DDS_DomainParticipantFactory_create_participant(
             factory,
             DDS_DOMAIN_ID_DEFAULT,
             DDS_PARTICIPANT_QOS_DEFAULT,
             NULL,
             0);

    config = topmon_configNew(dp, partition, logfile);

    DCPSParticipant_logger = topmon_loggerNew(config, "DCPSParticipant");
    DCPSTopic_logger = topmon_loggerNew(config, "DCPSTopic");
    DCPSPublication_logger = topmon_loggerNew(config, "DCPSPublication");
    DCPSSubscription_logger = topmon_loggerNew(config, "DCPSSubscription");
    CMPublisher_logger = topmon_loggerNew(config, "CMPublisher");
    CMSubscriber_logger = topmon_loggerNew(config, "CMSubscriber");
    CMDataWriter_logger = topmon_loggerNew(config, "CMDataWriter");
    CMDataReader_logger = topmon_loggerNew(config, "CMDataReader");
    CMParticipant_logger = topmon_loggerNew(config, "CMParticipant");
    DCPSType_logger = topmon_loggerNew(config, "DCPSType");

   /***********************************************************************************************/
    cs = DDS_ConditionSeq__alloc();

    while (TRUE) {
        result = DDS_WaitSet_wait (config->waitset, cs, &infinite);
        if (result == DDS_RETCODE_OK) {
            unsigned int i;
            for (i = 0; i < cs->_length; i++) {
                if (!processParticipant(cs->_buffer[i]) &&
                    !processTopic(cs->_buffer[i]) &&
                    !processPublication(cs->_buffer[i]) &&
                    !processSubscription(cs->_buffer[i]) &&
                    !processCmPublisher(cs->_buffer[i]) &&
                    !processCmSubscriber(cs->_buffer[i]) &&
                    !processCmWriter(cs->_buffer[i]) &&
                    !processCmReader(cs->_buffer[i]) &&
                    !processCmParticipant(cs->_buffer[i]) &&
                    !processType(cs->_buffer[i]))
                {
                    abort();
                }
            }
        }
    }

    result = DDS_DomainParticipant_delete_contained_entities(dp);
    if ( result != DDS_RETCODE_OK ) {
    }

    result = DDS_DomainParticipantFactory_delete_participant(factory, dp);
    if ( result != DDS_RETCODE_OK ) {
    }
    DDS_DomainParticipantFactory_delete_contained_entities(factory);
    return 0;
}

//static void
//reportResultCode(DDS_ReturnCode_t code)
//{
    //const char *image;
    //image = DDS_ReturnCode_image(code);
    //test_message(FW_NOTE, image);
//}
