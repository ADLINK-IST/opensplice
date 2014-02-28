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
#include "gapi_qos.h"
#include "gapi_common.h"
#include "gapi_error.h"

gapi_domainParticipantQos
gapi_domainParticipantQosDefault = DDS_DomainParticipantQos_default_value;

gapi_topicQos
gapi_topicQosDefault = DDS_TopicQos_default_value;

gapi_publisherQos
gapi_publisherQosDefault = DDS_PublisherQos_default_value;

gapi_subscriberQos
gapi_subscriberQosDefault = DDS_SubscriberQos_default_value;

gapi_dataReaderQos
gapi_dataReaderQosDefault = DDS_DataReaderQos_default_value;

gapi_dataReaderViewQos
gapi_dataReaderViewQosDefault = DDS_DataReaderViewQos_default_value;

gapi_dataWriterQos
gapi_dataWriterQosDefault = DDS_DataWriterQos_default_value;

static gapi_boolean
gapi_validBoolean(
    gapi_boolean value
    )
{
    gapi_boolean valid = FALSE;

    if ( (value == FALSE) || (value == TRUE) ) {
         valid = TRUE;
    }

    return valid;
}

static gapi_boolean
validUserDataQosPolicy (
    const gapi_userDataQosPolicy *policy,
    const gapi_context           *context,
    gapi_unsigned_long            qosId)
{
    gapi_boolean valid = TRUE;

    if ( !gapi_sequence_is_valid((void *)&policy->value) ) {
        gapi_errorInvalidQosPolicy(context, qosId,
                                   GAPI_USERDATA_QOS_POLICY_ID,
                                   GAPI_QOS_POLICY_ATTRIBUTE_VALUE_ID,
                                   GAPI_ERRORCODE_INVALID_SEQUENCE);
        valid = FALSE;
    }

    return valid;
}

static gapi_boolean
validTopicDataQosPolicy (
    const gapi_topicDataQosPolicy *policy,
    const gapi_context            *context,
    gapi_unsigned_long             qosId)
{
    gapi_boolean valid = TRUE;

    if ( !gapi_sequence_is_valid((void *)&policy->value) ) {
        gapi_errorInvalidQosPolicy(context, qosId,
                                   GAPI_TOPICDATA_QOS_POLICY_ID,
                                   GAPI_QOS_POLICY_ATTRIBUTE_VALUE_ID,
                                   GAPI_ERRORCODE_INVALID_SEQUENCE);
        valid = FALSE;
    }

    return valid;
}

static gapi_boolean
validGroupDataQosPolicy (
    const gapi_groupDataQosPolicy *policy,
    const gapi_context            *context,
    gapi_unsigned_long             qosId)
{
    gapi_boolean valid = TRUE;

    if ( !gapi_sequence_is_valid((void *)&policy->value) ) {
        gapi_errorInvalidQosPolicy(context, qosId,
                                   GAPI_GROUPDATA_QOS_POLICY_ID,
                                   GAPI_QOS_POLICY_ATTRIBUTE_VALUE_ID,
                                   GAPI_ERRORCODE_INVALID_SEQUENCE);
        valid = FALSE;
    }

    return valid;
}

static gapi_boolean
validPartitionQosPolicy (
    const gapi_partitionQosPolicy *policy,
    const gapi_context            *context,
    gapi_unsigned_long             qosId)
{
    gapi_boolean valid = TRUE;

    if ( !gapi_sequence_is_valid((void *)&policy->name) ) {
        gapi_errorInvalidQosPolicy(context, qosId,
                                   GAPI_PARTITION_QOS_POLICY_ID,
                                   GAPI_QOS_POLICY_ATTRIBUTE_NAME_ID,
                                   GAPI_ERRORCODE_INVALID_VALUE);
        valid = FALSE;
    }

    return valid;
}

static gapi_boolean
validDurabilityQosPolicy (
    const gapi_durabilityQosPolicy *policy,
    const gapi_context             *context,
    gapi_unsigned_long              qosId)
{
    gapi_boolean valid = TRUE;

    if ( (policy->kind != GAPI_VOLATILE_DURABILITY_QOS)        &&
         (policy->kind != GAPI_TRANSIENT_DURABILITY_QOS)       &&
         (policy->kind != GAPI_TRANSIENT_LOCAL_DURABILITY_QOS) &&
         (policy->kind != GAPI_PERSISTENT_DURABILITY_QOS) ) {
        gapi_errorInvalidQosPolicy(context, qosId,
                                   GAPI_DURABILITY_QOS_POLICY_ID,
                                   GAPI_QOS_POLICY_ATTRIBUTE_KIND_ID,
                                   GAPI_ERRORCODE_INVALID_VALUE);
        valid = FALSE;
    }

    return valid;
}

static gapi_boolean
validDurabilityServiceQosPolicy (
    const gapi_durabilityServiceQosPolicy *policy,
    const gapi_context                    *context,
    gapi_unsigned_long                     qosId)
{
    gapi_boolean valid = TRUE;

    if ( (policy->history_kind != GAPI_KEEP_LAST_HISTORY_QOS)  &&
         (policy->history_kind != GAPI_KEEP_ALL_HISTORY_QOS) ) {
        gapi_errorInvalidQosPolicy(context, qosId,
                                   GAPI_DURABILITYSERVICE_QOS_POLICY_ID,
                                   GAPI_QOS_POLICY_ATTRIBUTE_HISTORY_KIND_ID,
                                   GAPI_ERRORCODE_INVALID_VALUE);
        valid = FALSE;
    } else {
        if ( policy->history_kind == GAPI_KEEP_LAST_HISTORY_QOS ) {
            if ( policy->history_depth <= 0 ) {
                gapi_errorInvalidQosPolicy(context, qosId,
                                           GAPI_DURABILITYSERVICE_QOS_POLICY_ID,
                                           GAPI_QOS_POLICY_ATTRIBUTE_HISTORY_DEPTH_ID,
                                           GAPI_ERRORCODE_INVALID_VALUE);
                valid = FALSE;
            }
        }
    }

    if ( policy->max_samples < -1  ) {
        gapi_errorInvalidQosPolicy(context, qosId,
                                   GAPI_DURABILITYSERVICE_QOS_POLICY_ID,
                                   GAPI_QOS_POLICY_ATTRIBUTE_MAX_SAMPLES_ID,
                                   GAPI_ERRORCODE_INVALID_VALUE);
        valid = FALSE;
    }

    if ( policy->max_instances < -1 ) {
        gapi_errorInvalidQosPolicy(context, qosId,
                                   GAPI_DURABILITYSERVICE_QOS_POLICY_ID,
                                   GAPI_QOS_POLICY_ATTRIBUTE_MAX_INSTANCES_ID,
                                   GAPI_ERRORCODE_INVALID_VALUE);
        valid = FALSE;
    }

    if ( policy->max_samples_per_instance < -1 ) {
        gapi_errorInvalidQosPolicy(context, qosId,
                                   GAPI_DURABILITYSERVICE_QOS_POLICY_ID,
                                   GAPI_QOS_POLICY_ATTRIBUTE_MAX_SAMPLES_PER_INSTANCE_ID,
                                   GAPI_ERRORCODE_INVALID_VALUE);
        valid = FALSE;
    }

    if ( !gapi_validDuration(&policy->service_cleanup_delay) ) {
        gapi_errorInvalidQosPolicy(context, qosId,
                                   GAPI_DURABILITYSERVICE_QOS_POLICY_ID,
                                   GAPI_QOS_POLICY_ATTRIBUTE_SERVICE_CLEANUP_DELAY_ID,
                                   GAPI_ERRORCODE_INVALID_DURATION);
        valid = FALSE;
    }

    return valid;
}

static gapi_boolean
validLifespanQosPolicy (
    const gapi_lifespanQosPolicy *policy,
    const gapi_context           *context,
    gapi_unsigned_long            qosId)
{
    gapi_boolean valid = TRUE;

    if ( !gapi_validDuration(&policy->duration) ) {
        gapi_errorInvalidQosPolicy(context, qosId,
                                   GAPI_LIFESPAN_QOS_POLICY_ID,
                                   GAPI_QOS_POLICY_ATTRIBUTE_DURATION_ID,
                                   GAPI_ERRORCODE_INVALID_DURATION);
        valid = FALSE;
    }

    return valid;
}

static gapi_boolean
validDeadlineQosPolicy (
    const gapi_deadlineQosPolicy *policy,
    const gapi_context           *context,
    gapi_unsigned_long            qosId)
{
    gapi_boolean valid = TRUE;

    if ( !gapi_validDuration(&policy->period) ) {
        gapi_errorInvalidQosPolicy(context, qosId,
                                   GAPI_DEADLINE_QOS_POLICY_ID,
                                   GAPI_QOS_POLICY_ATTRIBUTE_PERIOD_ID,
                                   GAPI_ERRORCODE_INVALID_DURATION);
        valid = FALSE;
    }

    return valid;
}

static gapi_boolean
validPresentationQosPolicy (
    const gapi_presentationQosPolicy *policy,
    const gapi_context               *context,
    gapi_unsigned_long                qosId)
{
    gapi_boolean valid = TRUE;

    if ( !gapi_validBoolean(policy->coherent_access) ) {
        gapi_errorInvalidQosPolicy(context, qosId,
                                   GAPI_PRESENTATION_QOS_POLICY_ID,
                                   GAPI_QOS_POLICY_ATTRIBUTE_COHERENT_ACCESS_ID,
                                   GAPI_ERRORCODE_INVALID_VALUE);
        valid = FALSE;
    }

    if ( !gapi_validBoolean(policy->ordered_access) ) {
        gapi_errorInvalidQosPolicy(context, qosId,
                                   GAPI_PRESENTATION_QOS_POLICY_ID,
                                   GAPI_QOS_POLICY_ATTRIBUTE_ORDERED_ACCESS_ID,
                                   GAPI_ERRORCODE_INVALID_VALUE);
        valid = FALSE;
    }

    if ( (policy->access_scope != GAPI_INSTANCE_PRESENTATION_QOS) &&
         (policy->access_scope != GAPI_TOPIC_PRESENTATION_QOS)    &&
         (policy->access_scope != GAPI_GROUP_PRESENTATION_QOS) ) {
        gapi_errorInvalidQosPolicy(context, qosId,
                                   GAPI_PRESENTATION_QOS_POLICY_ID,
                                   GAPI_QOS_POLICY_ATTRIBUTE_ACCESS_SCOPE_ID,
                                   GAPI_ERRORCODE_INVALID_VALUE);
        valid = FALSE;
    }

    return valid;
}

static gapi_boolean
validOwnershipQosPolicy (
    const gapi_ownershipQosPolicy *policy,
    const gapi_context            *context,
    gapi_unsigned_long             qosId)
{
    gapi_boolean valid = TRUE;

    if ( (policy->kind != GAPI_SHARED_OWNERSHIP_QOS) &&
         (policy->kind != GAPI_EXCLUSIVE_OWNERSHIP_QOS) ) {
        gapi_errorInvalidQosPolicy(context, qosId,
                                   GAPI_OWNERSHIP_QOS_POLICY_ID,
                                   GAPI_QOS_POLICY_ATTRIBUTE_KIND_ID,
                                   GAPI_ERRORCODE_INVALID_VALUE);
        valid = FALSE;
    }

    return valid;
}

static gapi_boolean
validLivelinessQosPolicy (
    const gapi_livelinessQosPolicy *policy,
    const gapi_context             *context,
    gapi_unsigned_long              qosId)
{
    gapi_boolean valid = TRUE;

    if ( (policy->kind != GAPI_AUTOMATIC_LIVELINESS_QOS)             &&
         (policy->kind != GAPI_MANUAL_BY_PARTICIPANT_LIVELINESS_QOS) &&
         (policy->kind != GAPI_MANUAL_BY_TOPIC_LIVELINESS_QOS) ) {
        gapi_errorInvalidQosPolicy(context, qosId,
                                   GAPI_LIVELINESS_QOS_POLICY_ID,
                                   GAPI_QOS_POLICY_ATTRIBUTE_KIND_ID,
                                   GAPI_ERRORCODE_INVALID_VALUE);
        valid = FALSE;
    }

    if ( !gapi_validDuration(&policy->lease_duration) ) {
        gapi_errorInvalidQosPolicy(context, qosId,
                                   GAPI_LIVELINESS_QOS_POLICY_ID,
                                   GAPI_QOS_POLICY_ATTRIBUTE_LEASE_DURATION_ID,
                                   GAPI_ERRORCODE_INVALID_DURATION);
        valid = FALSE;
    }

    return valid;
}

static gapi_boolean
validTimeBasedFilterQosPolicy (
    const gapi_timeBasedFilterQosPolicy *policy,
    const gapi_context                  *context,
    gapi_unsigned_long                   qosId)
{
    gapi_boolean valid = TRUE;

    if ( !gapi_validDuration(&policy->minimum_separation) ) {
        gapi_errorInvalidQosPolicy(context, qosId,
                                   GAPI_TIMEBASEDFILTER_QOS_POLICY_ID,
                                   GAPI_QOS_POLICY_ATTRIBUTE_MINIMUM_SEPARATION_ID,
                                   GAPI_ERRORCODE_INVALID_DURATION);
        valid = FALSE;
    }

    return valid;
}


static gapi_boolean
validReliabilityQosPolicy (
    const gapi_reliabilityQosPolicy *policy,
    const gapi_context              *context,
    gapi_unsigned_long               qosId)
{
    gapi_boolean valid = TRUE;

    if ( (policy->kind != GAPI_BEST_EFFORT_RELIABILITY_QOS) &&
         (policy->kind != GAPI_RELIABLE_RELIABILITY_QOS) ) {
        gapi_errorInvalidQosPolicy(context, qosId,
                                   GAPI_RELIABILITY_QOS_POLICY_ID,
                                   GAPI_QOS_POLICY_ATTRIBUTE_KIND_ID,
                                   GAPI_ERRORCODE_INVALID_VALUE);
        valid = FALSE;
    }

    return valid;
}

static gapi_boolean
validDestinationOrderQosPolicy (
    const gapi_destinationOrderQosPolicy *policy,
    const gapi_context                   *context,
    gapi_unsigned_long                    qosId)
{
    gapi_boolean valid = TRUE;

    if ( (policy->kind != GAPI_BY_RECEPTION_TIMESTAMP_DESTINATIONORDER_QOS) &&
         (policy->kind != GAPI_BY_SOURCE_TIMESTAMP_DESTINATIONORDER_QOS) ) {
        gapi_errorInvalidQosPolicy(context, qosId,
                                   GAPI_DESTINATIONORDER_QOS_POLICY_ID,
                                   GAPI_QOS_POLICY_ATTRIBUTE_KIND_ID,
                                   GAPI_ERRORCODE_INVALID_VALUE);
        valid = FALSE;
    }

    return valid;
}

static gapi_boolean
validHistoryQosPolicy (
    const gapi_historyQosPolicy *policy,
    const gapi_context          *context,
    gapi_unsigned_long           qosId)
{
    gapi_boolean valid = TRUE;

    if ( (policy->kind != GAPI_KEEP_LAST_HISTORY_QOS) &&
         (policy->kind != GAPI_KEEP_ALL_HISTORY_QOS) ) {
        gapi_errorInvalidQosPolicy(context, qosId,
                                   GAPI_HISTORY_QOS_POLICY_ID,
                                   GAPI_QOS_POLICY_ATTRIBUTE_KIND_ID,
                                   GAPI_ERRORCODE_INVALID_VALUE);
        valid = FALSE;
    } else {
        if ( policy->kind == GAPI_KEEP_LAST_HISTORY_QOS ) {
            if ( policy->depth <= 0 ) {
                gapi_errorInvalidQosPolicy(context, qosId,
                                           GAPI_HISTORY_QOS_POLICY_ID,
                                           GAPI_QOS_POLICY_ATTRIBUTE_DEPTH_ID,
                                           GAPI_ERRORCODE_INVALID_VALUE);
                valid = FALSE;
            }
        }
    }

    return valid;
}

static gapi_boolean
validInvalidSampleVisibilityQosPolicy(
    const gapi_invalidSampleVisibilityQosPolicy *policy)
{
    gapi_boolean valid = TRUE;
    if ((policy->kind != GAPI_ALL_INVALID_SAMPLES) &&
        (policy->kind != GAPI_MINIMUM_INVALID_SAMPLES) &&
        (policy->kind != GAPI_NO_INVALID_SAMPLES)) {
        valid = FALSE;
    }
    return valid;
}

static gapi_boolean
validWriterDataLifecycleQosPolicy (
    const gapi_writerDataLifecycleQosPolicy *policy,
    const gapi_context                      *context,
    gapi_unsigned_long                       qosId)
{
    gapi_boolean valid = TRUE;

    if ( !gapi_validBoolean(policy->autodispose_unregistered_instances) ) {
        gapi_errorInvalidQosPolicy(context, qosId,
                                   GAPI_WRITERDATALIFECYCLE_QOS_POLICY_ID,
                                   GAPI_QOS_POLICY_ATTRIBUTE_AUTODISPOSE_UNREGISTERED_INSTANCES_ID,
                                   GAPI_ERRORCODE_INVALID_VALUE);
        valid = FALSE;
    }

    return valid;
}

static gapi_boolean
validReaderDataLifecycleQosPolicy (
    const gapi_readerDataLifecycleQosPolicy *policy,
    const gapi_context                      *context,
    gapi_unsigned_long                       qosId)
{
    gapi_boolean valid = TRUE;

    if ( !gapi_validDuration(&policy->autopurge_nowriter_samples_delay) ) {
        gapi_errorInvalidQosPolicy(context, qosId,
                                   GAPI_READERDATALIFECYCLE_QOS_POLICY_ID,
                                   GAPI_QOS_POLICY_ATTRIBUTE_AUTOPURGE_NOWRITER_SAMPLES_DELAY_ID,
                                   GAPI_ERRORCODE_INVALID_DURATION);
        valid = FALSE;
    }

    if ( !gapi_validDuration(&policy->autopurge_disposed_samples_delay) ) {
        gapi_errorInvalidQosPolicy(context, qosId,
                                   GAPI_READERDATALIFECYCLE_QOS_POLICY_ID,
                                   GAPI_QOS_POLICY_ATTRIBUTE_AUTOPURGE_DISPOSED_SAMPLES_DELAY_ID,
                                   GAPI_ERRORCODE_INVALID_DURATION);
        valid = FALSE;
    }

    if (!gapi_validBoolean(policy->enable_invalid_samples)) {
        gapi_errorInvalidQosPolicy(context, qosId,
                                   GAPI_READERDATALIFECYCLE_QOS_POLICY_ID,
                                   GAPI_QOS_POLICY_ATTRIBUTE_ENABLE_INVALID_SAMPLES_ID,
                                   GAPI_ERRORCODE_INVALID_VALUE);
        valid = FALSE;
    }

    if (!validInvalidSampleVisibilityQosPolicy(&policy->invalid_sample_visibility)) {
        gapi_errorInvalidQosPolicy(context, qosId,
                                   GAPI_READERDATALIFECYCLE_QOS_POLICY_ID,
                                   GAPI_QOS_POLICY_ATTRIBUTE_INVALID_SAMPLE_VISIBILITY_ID,
                                   GAPI_ERRORCODE_INVALID_VALUE);
        valid = FALSE;
    }

    return valid;
}

static gapi_boolean
validEntityFactoryQosPolicy (
    const gapi_entityFactoryQosPolicy *policy,
    const gapi_context                *context,
    gapi_unsigned_long                 qosId)
{
    gapi_boolean valid = TRUE;

    if ( !gapi_validBoolean(policy->autoenable_created_entities) ) {
        gapi_errorInvalidQosPolicy(context, qosId,
                                   GAPI_ENTITYFACTORY_QOS_POLICY_ID,
                                   GAPI_QOS_POLICY_ATTRIBUTE_AUTOENABLE_CREATED_ENTITIES_ID,
                                   GAPI_ERRORCODE_INVALID_VALUE);
        valid = FALSE;
    }

    return valid;
}

static gapi_boolean
validLatencyBudgetQosPolicy (
    const gapi_latencyBudgetQosPolicy *policy,
    const gapi_context                *context,
    gapi_unsigned_long                 qosId)
{
    gapi_boolean valid = TRUE;

    if ( !gapi_validDuration(&policy->duration) ) {
        gapi_errorInvalidQosPolicy(context, qosId,
                                   GAPI_LATENCYBUDGET_QOS_POLICY_ID,
                                   GAPI_QOS_POLICY_ATTRIBUTE_DURATION_ID,
                                   GAPI_ERRORCODE_INVALID_DURATION);
        valid = FALSE;
    }

    return valid;
}

static gapi_boolean
validResourceLimitsQosPolicy (
    const gapi_resourceLimitsQosPolicy *policy,
    const gapi_context                 *context,
    gapi_unsigned_long                  qosId)
{
    gapi_boolean valid = TRUE;


    if ( (policy->max_samples <= 0) && (policy->max_samples  != GAPI_LENGTH_UNLIMITED) ) {
        gapi_errorInvalidQosPolicy(context, qosId,
                                   GAPI_RESOURCELIMITS_QOS_POLICY_ID,
                                   GAPI_QOS_POLICY_ATTRIBUTE_MAX_SAMPLES_ID,
                                   GAPI_ERRORCODE_INVALID_VALUE);
        valid = FALSE;
    }

    if ( (policy->max_instances <= 0) && (policy->max_instances  != GAPI_LENGTH_UNLIMITED) ) {
        gapi_errorInvalidQosPolicy(context, qosId,
                                   GAPI_RESOURCELIMITS_QOS_POLICY_ID,
                                   GAPI_QOS_POLICY_ATTRIBUTE_MAX_INSTANCES_ID,
                                   GAPI_ERRORCODE_INVALID_VALUE);
        valid = FALSE;
    }

    if ( (policy->max_samples_per_instance <= 0) && (policy->max_samples_per_instance  != GAPI_LENGTH_UNLIMITED) ) {
        gapi_errorInvalidQosPolicy(context, qosId,
                                   GAPI_RESOURCELIMITS_QOS_POLICY_ID,
                                   GAPI_QOS_POLICY_ATTRIBUTE_MAX_SAMPLES_PER_INSTANCE_ID,
                                   GAPI_ERRORCODE_INVALID_VALUE);
        valid = FALSE;
    }

    return valid;
}

static gapi_boolean
validTransportPriorityQosPolicy (
    const gapi_transportPriorityQosPolicy *policy,
    const gapi_context                    *context,
    gapi_unsigned_long                     qosId)
{
    gapi_boolean valid = TRUE;

    return valid;
}

static gapi_boolean
validOwnershipStrengthQosPolicy (
    const gapi_ownershipStrengthQosPolicy *policy,
    const gapi_context                    *context,
    gapi_unsigned_long                     qosId)
{
    gapi_boolean valid = TRUE;

    return valid;
}

static gapi_boolean
validSubscriptionKeyQosPolicy (
    const gapi_subscriptionKeyQosPolicy *policy,
    const gapi_context                  *context,
    gapi_unsigned_long                   qosId)
{
    gapi_boolean valid = TRUE;

    if ( !gapi_validBoolean(policy->use_key_list) ) {
        gapi_errorInvalidQosPolicy(context, qosId,
                                   GAPI_SUBSCRIPTIONKEY_QOS_POLICY_ID,
                                   GAPI_QOS_POLICY_ATTRIBUTE_USE_KEY_LIST_ID,
                                   GAPI_ERRORCODE_INVALID_VALUE);
        valid = FALSE;
    }

    if ( valid && policy->use_key_list ) {
        if ( !gapi_sequence_is_valid((void *)&policy->key_list) ) {
        gapi_errorInvalidQosPolicy(context, qosId,
                                   GAPI_SUBSCRIPTIONKEY_QOS_POLICY_ID,
                                   GAPI_QOS_POLICY_ATTRIBUTE_KEY_LIST_ID,
                                   GAPI_ERRORCODE_INVALID_SEQUENCE);
            valid = FALSE;
        }
    }

    return valid;
}

static gapi_boolean
validViewKeyQosPolicy (
    const gapi_viewKeyQosPolicy *policy,
    const gapi_context          *context,
    gapi_unsigned_long           qosId)
{
    gapi_boolean valid = TRUE;

    if ( !gapi_validBoolean(policy->use_key_list) ) {
        gapi_errorInvalidQosPolicy(context, qosId,
                                   GAPI_VIEWKEY_QOS_POLICY_ID,
                                   GAPI_QOS_POLICY_ATTRIBUTE_USE_KEY_LIST_ID,
                                   GAPI_ERRORCODE_INVALID_VALUE);
        valid = FALSE;
    }

    if ( valid && policy->use_key_list ) {
        if ( !gapi_sequence_is_valid((void *)&policy->key_list) ) {
        gapi_errorInvalidQosPolicy(context, qosId,
                                   GAPI_VIEWKEY_QOS_POLICY_ID,
                                   GAPI_QOS_POLICY_ATTRIBUTE_KEY_LIST_ID,
                                   GAPI_ERRORCODE_INVALID_SEQUENCE);
            valid = FALSE;
        }
    }

    return valid;
}

static gapi_boolean
validReaderLifespanQosPolicy (
    const gapi_readerLifespanQosPolicy *policy,
    const gapi_context                 *context,
    gapi_unsigned_long                  qosId)
{
    gapi_boolean valid = TRUE;

    if ( !gapi_validBoolean(policy->use_lifespan) ) {
        gapi_errorInvalidQosPolicy(context, qosId,
                                   GAPI_READERLIFESPAN_QOS_POLICY_ID,
                                   GAPI_QOS_POLICY_ATTRIBUTE_USE_LIFESPAN_ID,
                                   GAPI_ERRORCODE_INVALID_VALUE);
        valid = FALSE;
    }

    if ( valid && policy->use_lifespan ) {
        if ( !gapi_validDuration(&policy->duration) ) {
            gapi_errorInvalidQosPolicy(context, qosId,
                                       GAPI_READERLIFESPAN_QOS_POLICY_ID,
                                       GAPI_QOS_POLICY_ATTRIBUTE_DURATION_ID,
                                       GAPI_ERRORCODE_INVALID_DURATION);
            valid = FALSE;
        }
    }

    return valid;
}


static gapi_boolean
validShareQosPolicy (
    const gapi_shareQosPolicy *policy,
    const gapi_context        *context,
    gapi_unsigned_long         qosId)
{
    gapi_boolean valid = TRUE;

    if ( !gapi_validBoolean(policy->enable) ) {
        gapi_errorInvalidQosPolicy(context, qosId,
                                   GAPI_SHARE_QOS_POLICY_ID,
                                   GAPI_QOS_POLICY_ATTRIBUTE_ENABLE_ID,
                                   GAPI_ERRORCODE_INVALID_VALUE);
        valid = FALSE;
    }

    if ( valid && policy->enable ) {
        if ( !policy->name ) {
            gapi_errorInvalidQosPolicy(context, qosId,
                                       GAPI_SHARE_QOS_POLICY_ID,
                                       GAPI_QOS_POLICY_ATTRIBUTE_NAME_ID,
                                       GAPI_ERRORCODE_INVALID_VALUE);
            valid = FALSE;
        }
    }

    return valid;
}

static gapi_boolean
validSchedulingClassQosPolicy (
    const gapi_schedulingClassQosPolicy *policy
    )
{
    gapi_boolean valid = TRUE;

    if (policy->kind != GAPI_SCHEDULE_REALTIME &&
        policy->kind != GAPI_SCHEDULE_TIMESHARING &&
        policy->kind != GAPI_SCHEDULE_DEFAULT) {
        valid = FALSE;
    }

    return valid;
}

static gapi_boolean
validSchedulingPriorityQosPolicy (
    const gapi_schedulingPriorityQosPolicy *policy
    )
{
    gapi_boolean valid = TRUE;

    if (policy->kind != GAPI_PRIORITY_ABSOLUTE &&
        policy->kind != GAPI_PRIORITY_RELATIVE) {
        valid = FALSE;
    }

    return valid;
}

static gapi_boolean
validSchedulingQosPolicy (
    const gapi_schedulingQosPolicy *policy,
    const gapi_context             *context,
    gapi_unsigned_long              qosId)
{
    gapi_boolean valid = TRUE;

    if ( !validSchedulingClassQosPolicy(&policy->scheduling_class) ) {
	gapi_errorInvalidQosPolicy(context, qosId,
                                   GAPI_SCHEDULING_QOS_POLICY_ID,
                                   GAPI_QOS_POLICY_ATTRIBUTE_SCHEDULING_CLASS_ID,
                                   GAPI_ERRORCODE_INVALID_VALUE);
        valid = FALSE;
    }

    if ( !validSchedulingPriorityQosPolicy(&policy->scheduling_priority_kind) ) {
	gapi_errorInvalidQosPolicy(context, qosId,
                                   GAPI_SCHEDULING_QOS_POLICY_ID,
                                   GAPI_QOS_POLICY_ATTRIBUTE_SCHEDULING_PRIORITY_KIND_ID,
                                   GAPI_ERRORCODE_INVALID_VALUE);
        valid = FALSE;
    }

    return valid;
}

gapi_returnCode_t
gapi_domainParticipantFactoryQosIsConsistent (
    const gapi_domainParticipantFactoryQos *qos,
    const gapi_context              *context
    )
{
    gapi_returnCode_t result = GAPI_RETCODE_OK;

    if (qos) {
        if ( !validEntityFactoryQosPolicy(&qos->entity_factory,
                                          context,
                                          GAPI_QOS_DOMAINPARTICIPANT_ID) )
        {
            result = GAPI_RETCODE_BAD_PARAMETER;
        }
    } else {
        result = GAPI_RETCODE_BAD_PARAMETER;
    }
    return result;
}


gapi_returnCode_t
gapi_domainParticipantQosIsConsistent (
    const gapi_domainParticipantQos *qos,
    const gapi_context              *context
    )
{
    if (qos == NULL) {
        return GAPI_RETCODE_BAD_PARAMETER;
    }
    if ( !validUserDataQosPolicy(&qos->user_data, context, GAPI_QOS_DOMAINPARTICIPANT_ID) ) {
        return GAPI_RETCODE_BAD_PARAMETER;
    }

    if ( !validEntityFactoryQosPolicy(&qos->entity_factory, context, GAPI_QOS_DOMAINPARTICIPANT_ID) ) {
        return GAPI_RETCODE_BAD_PARAMETER;
    }

    if ( !validSchedulingQosPolicy(&qos->watchdog_scheduling, context, GAPI_QOS_DOMAINPARTICIPANT_ID) ) {
	return GAPI_RETCODE_BAD_PARAMETER;
    }

    if ( !validSchedulingQosPolicy(&qos->listener_scheduling, context, GAPI_QOS_DOMAINPARTICIPANT_ID) ) {
	return GAPI_RETCODE_BAD_PARAMETER;
    }

    return GAPI_RETCODE_OK;
}

gapi_returnCode_t
gapi_topicQosIsConsistent (
    const gapi_topicQos *qos,
    const gapi_context  *context
    )
{
    if (qos == NULL) {
        return GAPI_RETCODE_BAD_PARAMETER;
    }
    if ( !validTopicDataQosPolicy(&qos->topic_data, context, GAPI_QOS_TOPIC_ID)                 ||
         !validDurabilityQosPolicy(&qos->durability, context, GAPI_QOS_TOPIC_ID)                ||
         !validDurabilityServiceQosPolicy(&qos->durability_service, context, GAPI_QOS_TOPIC_ID) ||
         !validDeadlineQosPolicy(&qos->deadline, context, GAPI_QOS_TOPIC_ID)                    ||
         !validLatencyBudgetQosPolicy(&qos->latency_budget, context, GAPI_QOS_TOPIC_ID)         ||
         !validLivelinessQosPolicy(&qos->liveliness, context, GAPI_QOS_TOPIC_ID)                ||
         !validReliabilityQosPolicy(&qos->reliability, context, GAPI_QOS_TOPIC_ID)              ||
         !validDestinationOrderQosPolicy(&qos->destination_order, context, GAPI_QOS_TOPIC_ID)   ||
         !validHistoryQosPolicy(&qos->history, context, GAPI_QOS_TOPIC_ID)                      ||
         !validResourceLimitsQosPolicy(&qos->resource_limits, context, GAPI_QOS_TOPIC_ID)       ||
         !validTransportPriorityQosPolicy(&qos->transport_priority, context, GAPI_QOS_TOPIC_ID) ||
         !validLifespanQosPolicy(&qos->lifespan, context, GAPI_QOS_TOPIC_ID)                    ||
         !validOwnershipQosPolicy(&qos->ownership, context, GAPI_QOS_TOPIC_ID) ) {
        return GAPI_RETCODE_BAD_PARAMETER;
    }

    if ( qos->history.kind == GAPI_KEEP_LAST_HISTORY_QOS ) {
        if ( qos->resource_limits.max_samples_per_instance != GAPI_LENGTH_UNLIMITED ) {
            if ( qos->history.depth > qos->resource_limits.max_samples_per_instance ) {
                gapi_errorInconsistentQosPolicy(context,
                                                GAPI_QOS_TOPIC_ID,
                                                GAPI_HISTORY_QOS_POLICY_ID,
                                                GAPI_QOS_POLICY_ATTRIBUTE_DEPTH_ID,
                                                GAPI_RESOURCELIMITS_QOS_POLICY_ID,
                                                GAPI_QOS_POLICY_ATTRIBUTE_MAX_SAMPLES_PER_INSTANCE_ID,
                                                GAPI_ERRORCODE_INCONSISTENT_VALUE);
                return GAPI_RETCODE_INCONSISTENT_POLICY;
            }
        }
    }

    return GAPI_RETCODE_OK;
}

gapi_boolean
gapi_topicQosEqual (
    const gapi_topicQos *qos1,
    const gapi_topicQos *qos2
    )
{
    gapi_boolean equal = TRUE;

    /* gapi_topicDataQosPolicy topic_data */

    if ( qos1->topic_data.value._length != qos2->topic_data.value._length ) {
        equal = FALSE;
    } else {
        if ( qos1->topic_data.value._length > 0 ) {
            if ( memcmp(qos1->topic_data.value._buffer, qos2->topic_data.value._buffer, qos1->topic_data.value._length) != 0 ) {
                equal = FALSE;
            }
        }
    }

    /* gapi_durabilityQosPolicy durability */
    if ( (qos1->durability.kind != qos2->durability.kind) ) {
        equal = FALSE;
    }

    /* gapi_durabilityServiceQosPolicy durability */
    if ( (qos1->durability_service.history_kind             != qos2->durability_service.history_kind)  ||
         (qos1->durability_service.history_depth            != qos2->durability_service.history_depth) ||
         (qos1->durability_service.max_samples              != qos2->durability_service.max_samples)   ||
         (qos1->durability_service.max_instances            != qos2->durability_service.max_instances) ||
         (qos1->durability_service.max_samples_per_instance != qos2->durability_service.max_samples_per_instance) ) {
        equal = FALSE;
    }

    /* gapi_deadlineQosPolicy deadline */
    if ( (qos1->deadline.period.sec     != qos2->deadline.period.sec) ||
         (qos1->deadline.period.nanosec != qos2->deadline.period.nanosec) ) {
        equal = FALSE;
    }

    /* gapi_latencyBudgetQosPolicy latency_budget */
    if ( (qos1->latency_budget.duration.sec     != qos2->latency_budget.duration.sec) ||
         (qos1->latency_budget.duration.nanosec != qos2->latency_budget.duration.nanosec) ) {
        equal = FALSE;
    }

    /* gapi_livelinessQosPolicy liveliness */
    if ( (qos1->liveliness.kind != qos2->liveliness.kind) ||
         (qos1->liveliness.lease_duration.sec     != qos2->liveliness.lease_duration.sec) ||
         (qos1->liveliness.lease_duration.nanosec != qos2->liveliness.lease_duration.nanosec) ) {
        equal = FALSE;
    }

    /* gapi_reliabilityQosPolicy reliability */
    if ( (qos1->reliability.kind != qos2->reliability.kind) ||
         (qos1->reliability.max_blocking_time.sec     != qos2->reliability.max_blocking_time.sec) ||
         (qos1->reliability.max_blocking_time.nanosec != qos2->reliability.max_blocking_time.nanosec) ) {
        equal = FALSE;
    }

    /* gapi_destinationOrderQosPolicy destination_order */
    if ( qos1->destination_order.kind != qos2->destination_order.kind  ) {
        equal = FALSE;
    }

    /* gapi_historyQosPolicy history */
    if ( (qos1->history.kind  != qos2->history.kind) ||
         (qos1->history.depth != qos2->history.depth) ) {
        equal = FALSE;
    }

    /* gapi_resourceLimitsQosPolicy resource_limits */
    if ( (qos1->resource_limits.max_samples              != qos2->resource_limits.max_samples)   ||
         (qos1->resource_limits.max_instances            != qos2->resource_limits.max_instances) ||
         (qos1->resource_limits.max_samples_per_instance != qos2->resource_limits.max_samples_per_instance) ) {
        equal = FALSE;
    }

    /* gapi_transportPriorityQosPolicy transport_priority */
    if ( qos1->transport_priority.value != qos2->transport_priority.value )  {
        equal = FALSE;
    }

    /* gapi_lifespanQosPolicy lifespan */
    if ( (qos1->lifespan.duration.sec     != qos2->lifespan.duration.sec) ||
         (qos1->lifespan.duration.nanosec != qos2->lifespan.duration.nanosec) ) {
        equal = FALSE;
    }

    /* gapi_ownershipQosPolicy ownership */
    if ( qos1->ownership.kind != qos2->ownership.kind ) {
        equal = FALSE;
    }

    return equal;
}

gapi_returnCode_t
gapi_publisherQosIsConsistent (
    const gapi_publisherQos *qos,
    const gapi_context      *context
    )
{
    gapi_returnCode_t retcode;

    if (qos == NULL) {
        return GAPI_RETCODE_BAD_PARAMETER;
    }
    if ( !validPresentationQosPolicy(&qos->presentation, context, GAPI_QOS_PUBLISHER_ID)    ||
         !validPartitionQosPolicy(&qos->partition, context, GAPI_QOS_PUBLISHER_ID)          ||
         !validGroupDataQosPolicy(&qos->group_data, context, GAPI_QOS_PUBLISHER_ID)         ||
         !validEntityFactoryQosPolicy(&qos->entity_factory, context, GAPI_QOS_PUBLISHER_ID) )
    {
        retcode = GAPI_RETCODE_BAD_PARAMETER;
    } else if ( qos->presentation.access_scope == GAPI_GROUP_PRESENTATION_QOS ) {
        gapi_errorUnsupportedQosPolicy(context,
                                       GAPI_QOS_PUBLISHER_ID,
                                       GAPI_PRESENTATION_QOS_POLICY_ID,
                                       GAPI_QOS_POLICY_ATTRIBUTE_ACCESS_SCOPE_ID,
                                       GAPI_ERRORCODE_UNSUPPORTED_VALUE);
        retcode = GAPI_RETCODE_UNSUPPORTED;
    } else if ( qos->presentation.ordered_access == TRUE ) {
        gapi_errorUnsupportedQosPolicy(context,
                                       GAPI_QOS_PUBLISHER_ID,
                                       GAPI_PRESENTATION_QOS_POLICY_ID,
                                       GAPI_QOS_POLICY_ATTRIBUTE_ORDERED_ACCESS_ID,
                                       GAPI_ERRORCODE_UNSUPPORTED_VALUE);
        retcode = GAPI_RETCODE_UNSUPPORTED;
    } else {
        retcode = GAPI_RETCODE_OK;
    }
    return retcode;
}

gapi_returnCode_t
gapi_subscriberQosIsConsistent (
    const gapi_subscriberQos *qos,
    const gapi_context       *context
    )
{
    gapi_returnCode_t retcode;

    if (qos == NULL) {
        return GAPI_RETCODE_BAD_PARAMETER;
    }
    if ( !validPresentationQosPolicy(&qos->presentation, context, GAPI_QOS_SUBSCRIBER_ID)    ||
         !validPartitionQosPolicy(&qos->partition, context, GAPI_QOS_SUBSCRIBER_ID)          ||
         !validGroupDataQosPolicy(&qos->group_data, context, GAPI_QOS_SUBSCRIBER_ID)         ||
         !validEntityFactoryQosPolicy(&qos->entity_factory, context, GAPI_QOS_SUBSCRIBER_ID) ||
         !validShareQosPolicy(&qos->share, context, GAPI_QOS_SUBSCRIBER_ID) ) {

        retcode = GAPI_RETCODE_BAD_PARAMETER;
    } else if ( qos->presentation.access_scope == GAPI_GROUP_PRESENTATION_QOS ) {
        gapi_errorUnsupportedQosPolicy(context,
                                       GAPI_QOS_SUBSCRIBER_ID,
                                       GAPI_PRESENTATION_QOS_POLICY_ID,
                                       GAPI_QOS_POLICY_ATTRIBUTE_ACCESS_SCOPE_ID,
                                       GAPI_ERRORCODE_UNSUPPORTED_VALUE);
        retcode = GAPI_RETCODE_UNSUPPORTED;
    } else if ( qos->presentation.ordered_access == TRUE ) {
        gapi_errorUnsupportedQosPolicy(context,
                                       GAPI_QOS_SUBSCRIBER_ID,
                                       GAPI_PRESENTATION_QOS_POLICY_ID,
                                       GAPI_QOS_POLICY_ATTRIBUTE_ORDERED_ACCESS_ID,
                                       GAPI_ERRORCODE_UNSUPPORTED_VALUE);
        retcode = GAPI_RETCODE_UNSUPPORTED;
    } else {
        retcode = GAPI_RETCODE_OK;
    }
    return retcode;
}

gapi_returnCode_t
gapi_dataReaderQosIsConsistent (
    const gapi_dataReaderQos *qos,
    const gapi_context       *context
    )
{
    if (qos == NULL) {
        return GAPI_RETCODE_BAD_PARAMETER;
    }
    if ( !validDurabilityQosPolicy(&qos->durability, context, GAPI_QOS_DATAREADER_ID)                     ||
         !validDeadlineQosPolicy(&qos->deadline, context, GAPI_QOS_DATAREADER_ID)                         ||
         !validLatencyBudgetQosPolicy(&qos->latency_budget, context, GAPI_QOS_DATAREADER_ID)              ||
         !validLivelinessQosPolicy(&qos->liveliness, context, GAPI_QOS_DATAREADER_ID)                     ||
         !validReliabilityQosPolicy(&qos->reliability, context, GAPI_QOS_DATAREADER_ID)                   ||
         !validDestinationOrderQosPolicy(&qos->destination_order, context, GAPI_QOS_DATAREADER_ID)        ||
         !validHistoryQosPolicy(&qos->history, context, GAPI_QOS_DATAREADER_ID)                           ||
         !validResourceLimitsQosPolicy(&qos->resource_limits, context, GAPI_QOS_DATAREADER_ID)            ||
         !validUserDataQosPolicy(&qos->user_data, context, GAPI_QOS_DATAREADER_ID)                        ||
         !validTimeBasedFilterQosPolicy(&qos->time_based_filter, context, GAPI_QOS_DATAREADER_ID)         ||
         !validOwnershipQosPolicy(&qos->ownership, context, GAPI_QOS_DATAREADER_ID)                       ||
         !validReaderDataLifecycleQosPolicy(&qos->reader_data_lifecycle, context, GAPI_QOS_DATAREADER_ID) ||
         !validSubscriptionKeyQosPolicy(&qos->subscription_keys, context, GAPI_QOS_DATAREADER_ID)         ||
         !validReaderLifespanQosPolicy(&qos->reader_lifespan, context, GAPI_QOS_DATAREADER_ID)            ||
         !validShareQosPolicy(&qos->share, context, GAPI_QOS_DATAREADER_ID) ) {
        return GAPI_RETCODE_BAD_PARAMETER;
    }

    if ( qos->history.kind == GAPI_KEEP_LAST_HISTORY_QOS ) {
        if ( qos->resource_limits.max_samples_per_instance != GAPI_LENGTH_UNLIMITED ) {
            if ( qos->history.depth > qos->resource_limits.max_samples_per_instance ) {
                gapi_errorInconsistentQosPolicy(context,
                                                GAPI_QOS_DATAREADER_ID,
                                                GAPI_HISTORY_QOS_POLICY_ID,
                                                GAPI_QOS_POLICY_ATTRIBUTE_DEPTH_ID,
                                                GAPI_RESOURCELIMITS_QOS_POLICY_ID,
                                                GAPI_QOS_POLICY_ATTRIBUTE_MAX_SAMPLES_PER_INSTANCE_ID,
                                                GAPI_ERRORCODE_INCONSISTENT_VALUE);
                return GAPI_RETCODE_INCONSISTENT_POLICY;
            }
        }
    }

    if (qos->reader_data_lifecycle.enable_invalid_samples == FALSE) {
        gapi_errorDeprecatedQosPolicy(context,
                                      GAPI_QOS_DATAREADER_ID,
                                      GAPI_READERDATALIFECYCLE_QOS_POLICY_ID,
                                      GAPI_QOS_POLICY_ATTRIBUTE_ENABLE_INVALID_SAMPLES_ID,
                                      GAPI_QOS_DATAREADER_ID,
                                      GAPI_READERDATALIFECYCLE_QOS_POLICY_ID,
                                      GAPI_QOS_POLICY_ATTRIBUTE_INVALID_SAMPLE_VISIBILITY_ID);
        if (qos->reader_data_lifecycle.invalid_sample_visibility.kind != GAPI_MINIMUM_INVALID_SAMPLES) {
            gapi_errorInconsistentQosPolicy(context,
                                            GAPI_QOS_DATAREADER_ID,
                                            GAPI_READERDATALIFECYCLE_QOS_POLICY_ID,
                                            GAPI_QOS_POLICY_ATTRIBUTE_ENABLE_INVALID_SAMPLES_ID,
                                            GAPI_READERDATALIFECYCLE_QOS_POLICY_ID,
                                            GAPI_QOS_POLICY_ATTRIBUTE_INVALID_SAMPLE_VISIBILITY_ID,
                                            GAPI_ERRORCODE_INCONSISTENT_VALUE);
            return GAPI_RETCODE_INCONSISTENT_POLICY;
        }
    }

    if ((qos->deadline.period.sec < qos->time_based_filter.minimum_separation.sec) ||
        ((qos->deadline.period.sec == qos->time_based_filter.minimum_separation.sec) &&
         (qos->deadline.period.nanosec < qos->time_based_filter.minimum_separation.nanosec))) {
        gapi_errorInconsistentQosPolicy(context,
                                        GAPI_QOS_DATAREADER_ID,
                                        GAPI_DEADLINE_QOS_POLICY_ID,
                                        GAPI_QOS_POLICY_ATTRIBUTE_PERIOD_ID,
                                        GAPI_TIMEBASEDFILTER_QOS_POLICY_ID,
                                        GAPI_QOS_POLICY_ATTRIBUTE_MINIMUM_SEPARATION_ID,
                                        GAPI_ERRORCODE_INCONSISTENT_VALUE);
        return GAPI_RETCODE_INCONSISTENT_POLICY;
    }

    if (qos->reader_data_lifecycle.invalid_sample_visibility.kind == GAPI_ALL_INVALID_SAMPLES) {
        gapi_errorUnsupportedQosPolicy(context,
                                       GAPI_QOS_DATAREADER_ID,
                                       GAPI_READERDATALIFECYCLE_QOS_POLICY_ID,
                                       GAPI_QOS_POLICY_ATTRIBUTE_INVALID_SAMPLE_VISIBILITY_ID,
                                       GAPI_ERRORCODE_UNSUPPORTED_VALUE);
        return GAPI_RETCODE_UNSUPPORTED;
    }

    return GAPI_RETCODE_OK;
}

gapi_returnCode_t
gapi_dataReaderViewQosIsConsistent (
    const gapi_dataReaderViewQos *qos,
    const gapi_context           *context
    )
{
    if (qos == NULL) {
        return GAPI_RETCODE_BAD_PARAMETER;
    }
    if ( !validViewKeyQosPolicy(&qos->view_keys, context, GAPI_QOS_DATAREADERVIEW_ID) ) {
        return GAPI_RETCODE_BAD_PARAMETER;
    }

    return GAPI_RETCODE_OK;
}


gapi_returnCode_t
gapi_dataWriterQosIsConsistent (
    const gapi_dataWriterQos *qos,
    const gapi_context       *context
    )
{
    if (qos == NULL) {
        return GAPI_RETCODE_BAD_PARAMETER;
    }
    if ( !validDurabilityQosPolicy(&qos->durability, context, GAPI_QOS_DATAWRITER_ID)                ||
         !validDeadlineQosPolicy(&qos->deadline, context, GAPI_QOS_DATAWRITER_ID)                    ||
         !validLatencyBudgetQosPolicy(&qos->latency_budget, context, GAPI_QOS_DATAWRITER_ID)         ||
         !validLivelinessQosPolicy(&qos->liveliness, context, GAPI_QOS_DATAWRITER_ID)                ||
         !validReliabilityQosPolicy(&qos->reliability, context, GAPI_QOS_DATAWRITER_ID)              ||
         !validDestinationOrderQosPolicy(&qos->destination_order, context, GAPI_QOS_DATAWRITER_ID)   ||
         !validHistoryQosPolicy(&qos->history, context, GAPI_QOS_DATAWRITER_ID)                      ||
         !validResourceLimitsQosPolicy(&qos->resource_limits, context, GAPI_QOS_DATAWRITER_ID)       ||
         !validTransportPriorityQosPolicy(&qos->transport_priority, context, GAPI_QOS_DATAWRITER_ID) ||
         !validLifespanQosPolicy(&qos->lifespan, context, GAPI_QOS_DATAWRITER_ID)                    ||
         !validUserDataQosPolicy(&qos->user_data, context, GAPI_QOS_DATAWRITER_ID)                   ||
         !validOwnershipQosPolicy(&qos->ownership, context, GAPI_QOS_DATAWRITER_ID)                  ||
         !validOwnershipStrengthQosPolicy(&qos->ownership_strength, context, GAPI_QOS_DATAWRITER_ID) ||
         !validWriterDataLifecycleQosPolicy(&qos->writer_data_lifecycle, context, GAPI_QOS_DATAWRITER_ID) ) {
        return GAPI_RETCODE_BAD_PARAMETER;
    }

    if ( qos->history.kind == GAPI_KEEP_LAST_HISTORY_QOS ) {
        if ( qos->resource_limits.max_samples_per_instance != GAPI_LENGTH_UNLIMITED ) {
            if ( qos->history.depth > qos->resource_limits.max_samples_per_instance ) {
                gapi_errorInconsistentQosPolicy(context,
                                                GAPI_QOS_DATAWRITER_ID,
                                                GAPI_HISTORY_QOS_POLICY_ID,
                                                GAPI_QOS_POLICY_ATTRIBUTE_DEPTH_ID,
                                                GAPI_RESOURCELIMITS_QOS_POLICY_ID,
                                                GAPI_QOS_POLICY_ATTRIBUTE_MAX_SAMPLES_PER_INSTANCE_ID,
                                                GAPI_ERRORCODE_INCONSISTENT_VALUE);
                return GAPI_RETCODE_INCONSISTENT_POLICY;
            }
        }
    }

    return GAPI_RETCODE_OK;
}
/*
 * Check mutability of Qos
 */



static gapi_boolean
immutableDurabilityQosPolicy (
    const gapi_durabilityQosPolicy *new_policy,
    const gapi_durabilityQosPolicy *old_policy,
    const gapi_context             *context,
    gapi_unsigned_long              qosId)
{
    gapi_boolean unchanged = TRUE;

    if ( new_policy->kind != old_policy->kind ) {
        gapi_errorImmutableQosPolicy(context, qosId,
                                   GAPI_DURABILITY_QOS_POLICY_ID);
        unchanged = FALSE;
    }

    return unchanged;
}

static gapi_boolean
immutableDurabilityServiceQosPolicy (
    const gapi_durabilityServiceQosPolicy *new_policy,
    const gapi_durabilityServiceQosPolicy *old_policy,
    const gapi_context                    *context,
    gapi_unsigned_long                     qosId)
{
    gapi_boolean unchanged = TRUE;

    if ( (new_policy->service_cleanup_delay.sec     != old_policy->service_cleanup_delay.sec)    ||
         (new_policy->service_cleanup_delay.nanosec != old_policy->service_cleanup_delay.nanosec) ||
         (new_policy->history_kind                  != old_policy->history_kind)                 ||
         (new_policy->history_depth                 != old_policy->history_depth)                ||
         (new_policy->max_samples                   != old_policy->max_samples)                  ||
         (new_policy->max_instances                 != old_policy->max_instances)                ||
         (new_policy->max_samples_per_instance      != old_policy->max_samples_per_instance)) {
        gapi_errorImmutableQosPolicy(context, qosId,
                                   GAPI_DURABILITYSERVICE_QOS_POLICY_ID);
        unchanged = FALSE;
    }

    return unchanged;
}


static gapi_boolean
immutablePresentationQosPolicy (
    const gapi_presentationQosPolicy *new_policy,
    const gapi_presentationQosPolicy *old_policy,
    const gapi_context               *context,
    gapi_unsigned_long                qosId)
{
    gapi_boolean unchanged = TRUE;

    if ( (new_policy->access_scope    != old_policy->access_scope) ||
         (new_policy->coherent_access != old_policy->coherent_access)          ||
         (new_policy->ordered_access  != old_policy->ordered_access)) {
        gapi_errorImmutableQosPolicy(context, qosId,
                                   GAPI_PRESENTATION_QOS_POLICY_ID);
        unchanged = FALSE;
    }

    return unchanged;
}


static gapi_boolean
immutableOwnershipQosPolicy (
    const gapi_ownershipQosPolicy *new_policy,
    const gapi_ownershipQosPolicy *old_policy,
    const gapi_context            *context,
    gapi_unsigned_long             qosId)
{
    gapi_boolean unchanged = TRUE;

    if ( (new_policy->kind    != old_policy->kind)) {
        gapi_errorImmutableQosPolicy(context, qosId,
                                   GAPI_OWNERSHIP_QOS_POLICY_ID);
        unchanged = FALSE;
    }

    return unchanged;
}


static gapi_boolean
immutableLivelinessQosPolicy (
    const gapi_livelinessQosPolicy *new_policy,
    const gapi_livelinessQosPolicy *old_policy,
    const gapi_context             *context,
    gapi_unsigned_long              qosId)
{
    gapi_boolean unchanged = TRUE;

    if ( (new_policy->kind                   != old_policy->kind)                  ||
         (new_policy->lease_duration.nanosec != old_policy->lease_duration.nanosec)||
         (new_policy->lease_duration.sec     != old_policy->lease_duration.sec)) {
        gapi_errorImmutableQosPolicy(context, qosId,
                                   GAPI_LIVELINESS_QOS_POLICY_ID);
        unchanged = FALSE;
    }

    return unchanged;
}



static gapi_boolean
immutableReliabilityQosPolicy (
    const gapi_reliabilityQosPolicy *new_policy,
    const gapi_reliabilityQosPolicy *old_policy,
    const gapi_context              *context,
    gapi_unsigned_long               qosId)
{
    gapi_boolean unchanged = TRUE;

    if ( (new_policy->kind                      != old_policy->kind)                     ||
         (new_policy->max_blocking_time.nanosec != old_policy->max_blocking_time.nanosec)||
         (new_policy->max_blocking_time.sec     != old_policy->max_blocking_time.sec) ||
         (new_policy->synchronous               != old_policy->synchronous)) {
        gapi_errorImmutableQosPolicy(context, qosId,
                                   GAPI_RELIABILITY_QOS_POLICY_ID);
        unchanged = FALSE;
    }

    return unchanged;
}


static gapi_boolean
immutableDestinationOrderQosPolicy (
    const gapi_destinationOrderQosPolicy *new_policy,
    const gapi_destinationOrderQosPolicy *old_policy,
    const gapi_context                   *context,
    gapi_unsigned_long                    qosId)
{
    gapi_boolean unchanged = TRUE;

    if ( new_policy->kind    != old_policy->kind) {
        gapi_errorImmutableQosPolicy(context, qosId,
                                   GAPI_DESTINATIONORDER_QOS_POLICY_ID);
        unchanged = FALSE;
    }

    return unchanged;
}


static gapi_boolean
immutableHistoryQosPolicy (
    const gapi_historyQosPolicy *new_policy,
    const gapi_historyQosPolicy *old_policy,
    const gapi_context          *context,
    gapi_unsigned_long           qosId)
{
    gapi_boolean unchanged = TRUE;

    if ( (new_policy->kind    != old_policy->kind) ||
         (new_policy->depth!= old_policy->depth)) {
        gapi_errorImmutableQosPolicy(context, qosId,
                                   GAPI_HISTORY_QOS_POLICY_ID);
        unchanged = FALSE;
    }

    return unchanged;
}




static gapi_boolean
immutableResourceLimitsQosPolicy (
    const gapi_resourceLimitsQosPolicy *new_policy,
    const gapi_resourceLimitsQosPolicy *old_policy,
    const gapi_context                 *context,
    gapi_unsigned_long                  qosId)
{
    gapi_boolean unchanged = TRUE;

    if ( (new_policy->max_samples                   != old_policy->max_samples)                  ||
         (new_policy->max_instances                 != old_policy->max_instances)                ||
         (new_policy->max_samples_per_instance      != old_policy->max_samples_per_instance)) {
        gapi_errorImmutableQosPolicy(context, qosId,
                                   GAPI_RESOURCELIMITS_QOS_POLICY_ID);
        unchanged = FALSE;
    }

    return unchanged;
}

static gapi_boolean
immutableSchedulingQosPolicy (
    const gapi_schedulingQosPolicy *new_policy,
    const gapi_schedulingQosPolicy *old_policy,
    const gapi_context                    *context,
    gapi_unsigned_long                     qosId)
{
    gapi_boolean unchanged = TRUE;

    if ( (new_policy->scheduling_class.kind         != old_policy->scheduling_class.kind)    ||
         (new_policy->scheduling_priority           != old_policy->scheduling_priority) ||
         (new_policy->scheduling_priority_kind.kind != old_policy->scheduling_priority_kind.kind)) {
        gapi_errorImmutableQosPolicy(context, qosId,
                                   GAPI_SCHEDULING_QOS_POLICY_ID);
        unchanged = FALSE;
    }

    return unchanged;
}

gapi_returnCode_t
gapi_domainParticipantFactoryQosCheckMutability (
    const gapi_domainParticipantFactoryQos *new_qos,
    const gapi_domainParticipantFactoryQos *old_qos,
    const gapi_context              *context
    )
{

    return GAPI_RETCODE_OK;
}


gapi_returnCode_t
gapi_domainParticipantQosCheckMutability (
    const gapi_domainParticipantQos *new_qos,
    const gapi_domainParticipantQos *old_qos,
    const gapi_context              *context
    )
{
    if ((new_qos == NULL) || (old_qos == NULL)) {
        return GAPI_RETCODE_BAD_PARAMETER;
    }
    if ( !immutableSchedulingQosPolicy(&new_qos->listener_scheduling, &old_qos->listener_scheduling, context, GAPI_QOS_TOPIC_ID) ||
         !immutableSchedulingQosPolicy(&new_qos->watchdog_scheduling, &old_qos->watchdog_scheduling, context, GAPI_QOS_TOPIC_ID)) {
        return GAPI_RETCODE_IMMUTABLE_POLICY;
    }


    return GAPI_RETCODE_OK;
}

gapi_returnCode_t
gapi_topicQosCheckMutability (
    const gapi_topicQos *new_qos,
    const gapi_topicQos *old_qos,
    const gapi_context  *context
    )
{
    if ((new_qos == NULL) || (old_qos == NULL)) {
        return GAPI_RETCODE_BAD_PARAMETER;
    }
    if ( !immutableDurabilityQosPolicy(&new_qos->durability, &old_qos->durability, context, GAPI_QOS_TOPIC_ID) ||
         !immutableDurabilityServiceQosPolicy(&new_qos->durability_service, &old_qos->durability_service, context, GAPI_QOS_TOPIC_ID) ||
         !immutableOwnershipQosPolicy(&new_qos->ownership, &old_qos->ownership, context, GAPI_QOS_TOPIC_ID) ||
         !immutableLivelinessQosPolicy(&new_qos->liveliness, &old_qos->liveliness, context, GAPI_QOS_TOPIC_ID) ||
         !immutableReliabilityQosPolicy(&new_qos->reliability, &old_qos->reliability, context, GAPI_QOS_TOPIC_ID) ||
         !immutableDestinationOrderQosPolicy(&new_qos->destination_order, &old_qos->destination_order, context, GAPI_QOS_TOPIC_ID) ||
         !immutableHistoryQosPolicy(&new_qos->history, &old_qos->history, context, GAPI_QOS_TOPIC_ID) ||
         !immutableResourceLimitsQosPolicy(&new_qos->resource_limits, &old_qos->resource_limits, context, GAPI_QOS_TOPIC_ID) ) {
        return GAPI_RETCODE_IMMUTABLE_POLICY;
    }


    return GAPI_RETCODE_OK;
}

gapi_returnCode_t
gapi_publisherQosCheckMutability (
    const gapi_publisherQos *new_qos,
    const gapi_publisherQos *old_qos,
    const gapi_context      *context
    )
{
    if ((new_qos == NULL) || (old_qos == NULL)) {
        return GAPI_RETCODE_BAD_PARAMETER;
    }
    if ( !immutablePresentationQosPolicy(&new_qos->presentation, &old_qos->presentation, context, GAPI_QOS_PUBLISHER_ID) ) {
        return GAPI_RETCODE_IMMUTABLE_POLICY;
    }

    return GAPI_RETCODE_OK;
}

gapi_returnCode_t
gapi_subscriberQosCheckMutability (
    const gapi_subscriberQos *new_qos,
    const gapi_subscriberQos *old_qos,
    const gapi_context       *context
    )
{
    if ((new_qos == NULL) || (old_qos == NULL)) {
        return GAPI_RETCODE_BAD_PARAMETER;
    }
    if ( !immutablePresentationQosPolicy(&new_qos->presentation, &old_qos->presentation, context, GAPI_QOS_SUBSCRIBER_ID) ) {
        return GAPI_RETCODE_IMMUTABLE_POLICY;
    }

    return GAPI_RETCODE_OK;
}

gapi_returnCode_t
gapi_dataReaderQosCheckMutability (
    const gapi_dataReaderQos *new_qos,
    const gapi_dataReaderQos *old_qos,
    const gapi_context       *context
    )
{
    if ((new_qos == NULL) || (old_qos == NULL)) {
        return GAPI_RETCODE_BAD_PARAMETER;
    }
    if ( !immutableDurabilityQosPolicy(&new_qos->durability, &old_qos->durability, context, GAPI_QOS_DATAREADER_ID) ||
         !immutableLivelinessQosPolicy(&new_qos->liveliness, &old_qos->liveliness, context, GAPI_QOS_DATAREADER_ID) ||
         !immutableReliabilityQosPolicy(&new_qos->reliability, &old_qos->reliability, context, GAPI_QOS_DATAREADER_ID) ||
         !immutableDestinationOrderQosPolicy(&new_qos->destination_order, &old_qos->destination_order, context, GAPI_QOS_DATAREADER_ID) ||
         !immutableHistoryQosPolicy(&new_qos->history, &old_qos->history, context, GAPI_QOS_DATAREADER_ID) ||
         !immutableOwnershipQosPolicy(&new_qos->ownership, &old_qos->ownership, context, GAPI_QOS_DATAREADER_ID) ||
         !immutableResourceLimitsQosPolicy(&new_qos->resource_limits, &old_qos->resource_limits, context, GAPI_QOS_DATAREADER_ID) ) {
        return GAPI_RETCODE_IMMUTABLE_POLICY;
    }

    return GAPI_RETCODE_OK;
}

gapi_returnCode_t
gapi_dataReaderViewQosCheckMutability (
    const gapi_dataReaderViewQos *new_qos,
    const gapi_dataReaderViewQos *old_qos,
    const gapi_context           *context
    )
{
    return GAPI_RETCODE_OK;
}


gapi_returnCode_t
gapi_dataWriterQosCheckMutability (
    const gapi_dataWriterQos *new_qos,
    const gapi_dataWriterQos *old_qos,
    const gapi_context       *context
    )
{
    if ((new_qos == NULL) || (old_qos == NULL)) {
        return GAPI_RETCODE_BAD_PARAMETER;
    }
    if ( !immutableDurabilityQosPolicy(&new_qos->durability, &old_qos->durability, context, GAPI_QOS_DATAWRITER_ID) ||
         !immutableLivelinessQosPolicy(&new_qos->liveliness, &old_qos->liveliness, context, GAPI_QOS_DATAWRITER_ID) ||
         !immutableReliabilityQosPolicy(&new_qos->reliability, &old_qos->reliability, context, GAPI_QOS_DATAWRITER_ID) ||
         !immutableDestinationOrderQosPolicy(&new_qos->destination_order, &old_qos->destination_order, context, GAPI_QOS_DATAWRITER_ID) ||
         !immutableHistoryQosPolicy(&new_qos->history, &old_qos->history, context, GAPI_QOS_DATAWRITER_ID) ||
         !immutableOwnershipQosPolicy(&new_qos->ownership, &old_qos->ownership, context, GAPI_QOS_DATAWRITER_ID) ||
         !immutableResourceLimitsQosPolicy(&new_qos->resource_limits, &old_qos->resource_limits, context, GAPI_QOS_DATAWRITER_ID) ) {
        return GAPI_RETCODE_IMMUTABLE_POLICY;
    }

    return GAPI_RETCODE_OK;
}

