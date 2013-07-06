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
#include "os_report.h"
#include "gapi.h"
#include "gapi_object.h"
#include "gapi_error.h"


#define ERROR_CONTEXT "DCPS API"

static const char *methodName[] = {
    "unknown",
    "get_qos",
    "set_qos",
    "get_listener",
    "set_listener",
    "get_statuscondition",
    "get_status_change",
    "enable",
    "create_publisher",
    "delete_publisher",
    "create_subscriber",
    "delete_subscriber",
    "create_topic",
    "delete_topic",
    "create_contentfilteredtopic",
    "delete_contentfilteredtopic",
    "create_multitopic",
    "delete_multitopic",
    "find_topic",
    "lookup_topic_description",
    "get_builtin_subscriber",
    "ignore_participant",
    "ignore_topic",
    "ignore_publication",
    "ignore_subscription",
    "get_domain_id",
    "delete_contained_entities",
    "assert_liveliness",
    "set_default_publisher_qos",
    "get_default_publisher_qos",
    "set_default_subscriber_qos",
    "get_default_subscriber_qos",
    "set_default_topic_qos",
    "get_default_topic_qos",
    "create_participant",
    "delete_participant",
    "get_instance",
    "lookup_participant",
    "set_default_participant_qos",
    "get_default_participant_qos",
    "get_participant",
    "get_type_name",
    "get_name",
    "get_inconsistent_topic_status",
    "get_related_topic",
    "get_filter_expression",
    "get_expression_parameters",
    "set_expression_parameters",
    "get_subscription_expression",
    "register_type",
    "create_datawriter",
    "delete_datawriter",
    "lookup_datawriter",
    "suspend_publications",
    "resume_publications",
    "begin_coherent_changes",
    "end_coherent_changes",
    "set_default_datawriter_qos",
    "get_default_datawriter_qos",
    "copy_from_topic_qos",
    "create_datareader",
    "delete_datareader",
    "lookup_datareader",
    "begin_access",
    "end_access",
    "get_datareaders",
    "notify_datareaders",
    "get_sample_lost_status",
    "set_default_datareader_qos",
    "get_default_datareader_qos",
    "register_instance",
    "register_instance_w_timestamp",
    "unregister_instance",
    "unregister_instance_w_timestamp",
    "get_key_value",
    "write",
    "write_w_timestamp",
    "dispose",
    "dispose_w_timestamp",
    "get_liveliness_lost_status",
    "get_offered_deadline_missed_status",
    "get_offered_incompatible_qos_status",
    "get_publication_match_status",
    "get_topic",
    "get_publisher",
    "get_matched_subscription_data",
    "get_matched_subscriptions",
    "read",
    "take",
    "read_w_condition",
    "take_w_condition",
    "read_next_sample",
    "take_next_sample",
    "read_instance",
    "take_instance",
    "read_next_instance",
    "take_next_instance",
    "read_next_instance_w_condition",
    "take_next_instance_w_condition",
    "return_loan",
    "create_readcondition",
    "create_querycondition",
    "delete_readcondition",
    "get_liveliness_changed_status",
    "get_requested_deadline_missed_status",
    "get_requested_incompatible_qos_status",
    "get_sample_rejected_status",
    "get_subscription_match_status",
    "get_topicdescription",
    "get_subscriber",
    "wait_for_historical_data",
    "get_matched_publication_data",
    "get_matched_publications",
    "get_trigger_value",
    "set_trigger_value",
    "set_enabled_statuses",
    "get_enabled_statuses",
    "get_entity",
    "get_datareader",
    "get_sample_state_mask",
    "get_view_state_mask",
    "get_instance_state_mask",
    "get_query_expression",
    "set_query_arguments",
    "get_query_arguments",
    "attach_condition",
    "detach_condition",
    "wait",
    "get_conditions",
    "create_view",
    "delete_view"
};

static const char *qosName[] = {
    "UnknownQos",
    "DomainParticipantQos",
    "TopicQos",
    "PublisherQos",
    "SubscriberQos",
    "DataWriterQos",
    "DataReaderQos",
    "DataReaderViewQos"
};

static const char *qosPolicyName[] = {
    "Unknown Qos Policy",
    GAPI_USERDATA_QOS_POLICY_NAME,
    GAPI_DURABILITY_QOS_POLICY_NAME,
    GAPI_PRESENTATION_QOS_POLICY_NAME,
    GAPI_DEADLINE_QOS_POLICY_NAME,
    GAPI_LATENCYBUDGET_QOS_POLICY_NAME,
    GAPI_OWNERSHIP_QOS_POLICY_NAME,
    GAPI_OWNERSHIPSTRENGTH_QOS_POLICY_NAME,
    GAPI_LIVELINESS_QOS_POLICY_NAME,
    GAPI_TIMEBASEDFILTER_QOS_POLICY_NAME,
    GAPI_PARTITION_QOS_POLICY_NAME,
    GAPI_RELIABILITY_QOS_POLICY_NAME,
    GAPI_DESTINATIONORDER_QOS_POLICY_NAME,
    GAPI_HISTORY_QOS_POLICY_NAME,
    GAPI_RESOURCELIMITS_QOS_POLICY_NAME,
    GAPI_ENTITYFACTORY_QOS_POLICY_NAME,
    GAPI_WRITERDATALIFECYCLE_QOS_POLICY_NAME,
    GAPI_READERDATALIFECYCLE_QOS_POLICY_NAME,
    GAPI_TOPICDATA_QOS_POLICY_NAME,
    GAPI_GROUPDATA_QOS_POLICY_NAME,
    GAPI_TRANSPORTPRIORITY_QOS_POLICY_NAME,
    GAPI_LIFESPAN_QOS_POLICY_NAME,
    GAPI_DURABILITYSERVICE_QOS_POLICY_NAME,
    GAPI_SUBSCRIPTIONKEY_QOS_POLICY_NAME,
    GAPI_VIEWKEY_QOS_POLICY_NAME,
    GAPI_READERLIFESPAN_QOS_POLICY_NAME,
    GAPI_SHARE_QOS_POLICY_NAME,
    GAPI_SCHEDULING_QOS_POLICY_NAME
};


static const char *qosAttributeName[] = {
    "unknown",
    "value",
    "kind",
    "duration",
    "service_cleanup_delay",
    "access_scope",
    "coherent_access",
    "ordered_access",
    "period",
    "lease_duration",
    "minimum_separation",
    "name",
    "enable",
    "max_blocking_time",
    "depth",
    "max_samples",
    "max_instances",
    "max_samples_per_instance",
    "history",
    "history_depth",
    "autoenable_created_entities",
    "autodispose_unregistered_instances",
    "autopurge_nowriter_samples_delay ",
    "autopurge_disposed_samples_delay",
    "use_key_list",
    "key_list",
    "use_lifespan",
    "scheduling_class",
    "scheduling_priority_kind",
    "enable_invalid_samples",
    "invalid_sample_visibility"
};


static const char *errorMessage[] = {
    "undefined error",
    "unknown error",
    "out of resources",
    "creation of kernel entity failed",
    "invalid value",
    "invalid duration",
    "invalid time",
    "entity inuse",
    "entities contained",
    "entity unknown",
    "handle not registered",
    "handle not match",
    "handle invalid",
    "invalid_sequence",
    "unsupported_value",
    "inconsistent_value",
    "immutable_qos_policy",
    "inconsistent_qos",
    "unsupported_qos_policy",
    "contains_conditions",
    "contains_loans",
    "inconsistent topic"
};

#define GAPI_RETCODE_COUNT (13)

static const char *retcode_image[GAPI_RETCODE_COUNT] = {
    "RETCODE_OK",
    "RETCODE_ERROR",
    "RETCODE_UNSUPPORTED",
    "RETCODE_BAD_PARAMETER",
    "RETCODE_PRECONDITION_NOT_MET",
    "RETCODE_OUT_OF_RESOURCES",
    "RETCODE_NOT_ENABLED",
    "RETCODE_IMMUTABLE_POLICY",
    "RETCODE_INCONSISTENT_POLICY",
    "RETCODE_ALREADY_DELETED",
    "RETCODE_TIMEOUT",
    "RETCODE_NO_DATA",
    "RETCODE_ILLEGAL_OPERATION"
};

const char *
gapi_retcode_image (
    gapi_returnCode_t retcode)
{
    const char *image;

    if ((retcode >= 0) || (retcode < GAPI_RETCODE_COUNT)) {
        image = retcode_image[retcode];
    } else {
        image = "<UNDEFINED RETCODE>";
    }
    return image;
}

static const char *
getEntityName (
    gapi_entity entity)
{
    _ObjectKind kind;

#define _CASE_(k,n) case OBJECT_KIND_##k: return #n

    kind = gapi_objectGetKind(entity);
    switch ( kind ) {
        _CASE_(ENTITY,Entity);
        _CASE_(DOMAIN,Domain);
        _CASE_(DOMAINPARTICIPANTFACTORY,DomainParticipantFactory);
        _CASE_(DOMAINPARTICIPANT,DomainParticipant);
        _CASE_(TYPESUPPORT,Typesupport);
        _CASE_(TOPICDESCRIPTION,TopicDescription);
        _CASE_(TOPIC,topic);
        _CASE_(CONTENTFILTEREDTOPIC,ContentFilteredtopic);
        _CASE_(MULTITOPIC,MultiTopic);
        _CASE_(PUBLISHER,Publisher);
        _CASE_(SUBSCRIBER,Subscriber);
        _CASE_(DATAWRITER,Datawriter);
        _CASE_(DATAREADER,Datareader);
        _CASE_(CONDITION,Condition);
        _CASE_(STATUSCONDITION,Statuscondition);
        _CASE_(READCONDITION,Readcondition);
        _CASE_(QUERYCONDITION,Querycondition);
        _CASE_(WAITSET,WAITSET);
    default:
        return "UNKNOWN";
    }

#undef _CASE_
}

#define VALID_METHOD_ID(i)    ((i < GAPI_METHOD_MAX)?i:0)
#define VALID_ERROR_ID(i)     ((i < GAPI_ERRORCODE_MAX)?i:0)
#define VALID_QOS_ID(i)       ((i < GAPI_QOS_ID_MAX)?i:0)
#define VALID_POLICY_ID(i)    ((i < GAPI_QOS_POLICY_MAX_ID)?i:0)
#define VALID_ATTRIBUTE_ID(i) ((i < GAPI_QOS_POLICY_ATTRIBUTE_MAX_ID)?i:0)

const char *
gapi_context_getEntityName(
    const gapi_context *_this)
{
    return getEntityName(_this->entity);
}

const char *
gapi_context_getMethodName(
    const gapi_context *_this)
{
    return methodName[VALID_METHOD_ID(_this->methodId)];
}

const char *
gapi_context_getQosName(
    gapi_unsigned_long qos_id)
{
    return qosName[VALID_QOS_ID(qos_id)];
}

const char *
gapi_context_getQosPolicyName(
    gapi_unsigned_long policy_id)
{
    return qosPolicyName[VALID_POLICY_ID(policy_id)];
}

const char *
gapi_context_getQosAttributeName(
    gapi_unsigned_long attr_id)
{
    return qosAttributeName[VALID_ATTRIBUTE_ID(attr_id)];
}

const char *
gapi_context_getErrorMessage(
    gapi_unsigned_long error_code)
{
    return errorMessage[VALID_ERROR_ID(error_code)];
}

#define TYPEPARSE_ERROR   "Type parse error"
#define TYPEPARSE_CONTEXT "DCPS TYPE"

void
gapi_typeParseError (
    sd_errorReport report)
{
    if ( report ) {
        if ( report->message ) {
            if ( report->location ) {
                OS_REPORT_3(OS_API_INFO, TYPEPARSE_CONTEXT, 0,
                            "%s : %s at %s", TYPEPARSE_ERROR,
                            report->message, report->location);
            } else {
                OS_REPORT_2(OS_API_INFO, TYPEPARSE_CONTEXT, 0,
                            "%s : %s", TYPEPARSE_ERROR,
                            report->message);
            }
        } else {
            OS_REPORT(OS_API_INFO, TYPEPARSE_CONTEXT, 0, TYPEPARSE_ERROR);
        }
    } else {
        OS_REPORT(OS_API_INFO, TYPEPARSE_CONTEXT, 0, TYPEPARSE_ERROR);
    }
}

