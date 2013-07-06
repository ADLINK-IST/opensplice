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
#ifndef GAPI_ERROR_H
#define GAPI_ERROR_H

#include "gapi.h"
#include "gapi_common.h"

#include "sd_errorReport.h"
#include "os_report.h"

#define GAPI_QOS_DOMAINPARTICIPANT_ID                                       1
#define GAPI_QOS_TOPIC_ID                                                   2
#define GAPI_QOS_PUBLISHER_ID                                               3
#define GAPI_QOS_SUBSCRIBER_ID                                              4
#define GAPI_QOS_DATAWRITER_ID                                              5
#define GAPI_QOS_DATAREADER_ID                                              6
#define GAPI_QOS_DATAREADERVIEW_ID                                          7
#define GAPI_QOS_ID_MAX                                                     GAPI_QOS_DATAREADERVIEW_ID + 1

#define GAPI_QOS_POLICY_MAX_ID                                              GAPI_SCHEDULING_QOS_POLICY_ID + 1

#define GAPI_QOS_POLICY_ATTRIBUTE_VALUE_ID                                  1
#define GAPI_QOS_POLICY_ATTRIBUTE_KIND_ID                                   2
#define GAPI_QOS_POLICY_ATTRIBUTE_DURATION_ID                               3
#define GAPI_QOS_POLICY_ATTRIBUTE_SERVICE_CLEANUP_DELAY_ID                  4
#define GAPI_QOS_POLICY_ATTRIBUTE_ACCESS_SCOPE_ID                           5
#define GAPI_QOS_POLICY_ATTRIBUTE_COHERENT_ACCESS_ID                        6
#define GAPI_QOS_POLICY_ATTRIBUTE_ORDERED_ACCESS_ID                         7
#define GAPI_QOS_POLICY_ATTRIBUTE_PERIOD_ID                                 8
#define GAPI_QOS_POLICY_ATTRIBUTE_LEASE_DURATION_ID                         9
#define GAPI_QOS_POLICY_ATTRIBUTE_MINIMUM_SEPARATION_ID                     10
#define GAPI_QOS_POLICY_ATTRIBUTE_NAME_ID                                   11
#define GAPI_QOS_POLICY_ATTRIBUTE_ENABLE_ID                                 12
#define GAPI_QOS_POLICY_ATTRIBUTE_MAX_BLOCKING_TIME_ID                      13
#define GAPI_QOS_POLICY_ATTRIBUTE_DEPTH_ID                                  14
#define GAPI_QOS_POLICY_ATTRIBUTE_MAX_SAMPLES_ID                            15
#define GAPI_QOS_POLICY_ATTRIBUTE_MAX_INSTANCES_ID                          16
#define GAPI_QOS_POLICY_ATTRIBUTE_MAX_SAMPLES_PER_INSTANCE_ID               17
#define GAPI_QOS_POLICY_ATTRIBUTE_HISTORY_KIND_ID                           18
#define GAPI_QOS_POLICY_ATTRIBUTE_HISTORY_DEPTH_ID                          19
#define GAPI_QOS_POLICY_ATTRIBUTE_AUTOENABLE_CREATED_ENTITIES_ID            20
#define GAPI_QOS_POLICY_ATTRIBUTE_AUTODISPOSE_UNREGISTERED_INSTANCES_ID     21
#define GAPI_QOS_POLICY_ATTRIBUTE_AUTOPURGE_NOWRITER_SAMPLES_DELAY_ID       22
#define GAPI_QOS_POLICY_ATTRIBUTE_AUTOPURGE_DISPOSED_SAMPLES_DELAY_ID       23
#define GAPI_QOS_POLICY_ATTRIBUTE_USE_KEY_LIST_ID                           24
#define GAPI_QOS_POLICY_ATTRIBUTE_KEY_LIST_ID                               25
#define GAPI_QOS_POLICY_ATTRIBUTE_USE_LIFESPAN_ID                           26
#define GAPI_QOS_POLICY_ATTRIBUTE_SCHEDULING_CLASS_ID                       27
#define GAPI_QOS_POLICY_ATTRIBUTE_SCHEDULING_PRIORITY_KIND_ID               28
#define GAPI_QOS_POLICY_ATTRIBUTE_ENABLE_INVALID_SAMPLES_ID                 29
#define GAPI_QOS_POLICY_ATTRIBUTE_INVALID_SAMPLE_VISIBILITY_ID              30
#define GAPI_QOS_POLICY_ATTRIBUTE_MAX_ID                                    31


/* Entity methods */
#define GAPI_METHOD_GET_QOS                                                 1
#define GAPI_METHOD_SET_QOS                                                 2
#define GAPI_METHOD_GET_LISTENER                                            3
#define GAPI_METHOD_SET_LISTENER                                            4
#define GAPI_METHOD_GET_STATUSCONDITION                                     5
#define GAPI_METHOD_GET_STATUS_CHANGE                                       6
#define GAPI_METHOD_ENABLE                                                  7

/* DomainParticipant methods */
#define GAPI_METHOD_CREATE_PUBLISHER                                        8
#define GAPI_METHOD_DELETE_PUBLISHER                                        9
#define GAPI_METHOD_CREATE_SUBSCRIBER                                       10
#define GAPI_METHOD_DELETE_SUBSCRIBER                                       11
#define GAPI_METHOD_CREATE_TOPIC                                            12
#define GAPI_METHOD_DELETE_TOPIC                                            13
#define GAPI_METHOD_CREATE_CONTENTFILTEREDTOPIC                             14
#define GAPI_METHOD_DELETE_CONTENTFILTEREDTOPIC                             15
#define GAPI_METHOD_CREATE_MULTITOPIC                                       16
#define GAPI_METHOD_DELETE_MULTITOPIC                                       17
#define GAPI_METHOD_FIND_TOPIC                                              18
#define GAPI_METHOD_LOOKUP_TOPIC_DESCRIPTION                                19
#define GAPI_METHOD_GET_BUILTIN_SUBSCRIBER                                  20
#define GAPI_METHOD_IGNORE_PARTICIPANT                                      21
#define GAPI_METHOD_IGNORE_TOPIC                                            22
#define GAPI_METHOD_IGNORE_PUBLICATION                                      23
#define GAPI_METHOD_IGNORE_SUBSCRIPTION                                     24
#define GAPI_METHOD_GET_DOMAIN_ID                                           25
#define GAPI_METHOD_DELETE_CONTAINED_ENTITIES                               26
#define GAPI_METHOD_ASSERT_LIVELINESS                                       27
#define GAPI_METHOD_SET_DEFAULT_PUBLISHER_QOS                               28
#define GAPI_METHOD_GET_DEFAULT_PUBLISHER_QOS                               29
#define GAPI_METHOD_SET_DEFAULT_SUBSCRIBER_QOS                              30
#define GAPI_METHOD_GET_DEFAULT_SUBSCRIBER_QOS                              31
#define GAPI_METHOD_SET_DEFAULT_TOPIC_QOS                                   32
#define GAPI_METHOD_GET_DEFAULT_TOPIC_QOS                                   33

/* DomainParticipantFactory */
#define GAPI_METHOD_CREATE_PARTICIPANT                                      34
#define GAPI_METHOD_DELETE_PARTICIPANT                                      35
#define GAPI_METHOD_GET_INSTANCE                                            36
#define GAPI_METHOD_LOOKUP_PARTICIPANT                                      37
#define GAPI_METHOD_SET_DEFAULT_PARTICIPANT_QOS                             38
#define GAPI_METHOD_GET_DEFAULT_PARTICIPANT_QOS                             39

/* TopicDescription */
#define GAPI_METHOD_GET_PARTICIPANT                                         40
#define GAPI_METHOD_GET_TYPE_NAME                                           41
#define GAPI_METHOD_GET_NAME                                                42

/* Topic */
#define GAPI_METHOD_GET_INCONSISTENT_TOPIC_STATUS                           43

/* ContentFilteredTopic */
#define GAPI_METHOD_GET_RELATED_TOPIC                                       44
#define GAPI_METHOD_GET_FILTER_EXPRESSION                                   45
#define GAPI_METHOD_GET_EXPRESSION_PARAMETERS                               46
#define GAPI_METHOD_SET_EXPRESSION_PARAMETERS                               47

/* MultiTopic */
#define GAPI_METHOD_GET_SUBSCRIPTION_EXPRESSION                             48

/* TypeSupport */
#define GAPI_METHOD_REGISTER_TYPE                                           49

/* Publisher */
#define GAPI_METHOD_CREATE_DATAWRITER                                       50
#define GAPI_METHOD_DELETE_DATAWRITER                                       51
#define GAPI_METHOD_LOOKUP_DATAWRITER                                       52
#define GAPI_METHOD_SUSPEND_PUBLICATIONS                                    53
#define GAPI_METHOD_RESUME_PUBLICATIONS                                     54
#define GAPI_METHOD_BEGIN_COHERENT_CHANGES                                  55
#define GAPI_METHOD_END_COHERENT_CHANGES                                    56
#define GAPI_METHOD_SET_DEFAULT_DATAWRITER_QOS                              57
#define GAPI_METHOD_GET_DEFAULT_DATAWRITER_QOS                              58
#define GAPI_METHOD_COPY_FROM_TOPIC_QOS                                     59


/* Subscriber */
#define GAPI_METHOD_CREATE_DATAREADER                                       60
#define GAPI_METHOD_DELETE_DATAREADER                                       61
#define GAPI_METHOD_LOOKUP_DATAREADER                                       62
#define GAPI_METHOD_BEGIN_ACCESS                                            63
#define GAPI_METHOD_END_ACCESS                                              64
#define GAPI_METHOD_GET_DATAREADERS                                         65
#define GAPI_METHOD_NOTIFY_DATAREADERS                                      66
#define GAPI_METHOD_GET_SAMPLE_LOST_STATUS                                  67
#define GAPI_METHOD_SET_DEFAULT_DATAREADER_QOS                              68
#define GAPI_METHOD_GET_DEFAULT_DATAREADER_QOS                              69

/* DataWriter */
#define GAPI_METHOD_REGISTER_INSTANCE                                       70
#define GAPI_METHOD_REGISTER_INSTANCE_W_TIMESTAMP                           71
#define GAPI_METHOD_UNREGISTER_INSTANCE                                     72
#define GAPI_METHOD_UNREGISTER_INSTANCE_W_TIMESTAMP                         73
#define GAPI_METHOD_GET_KEY_VALUE                                           74
#define GAPI_METHOD_WRITE                                                   75
#define GAPI_METHOD_WRITE_W_TIMESTAMP                                       76
#define GAPI_METHOD_DISPOSE                                                 77
#define GAPI_METHOD_DISPOSE_W_TIMESTAMP                                     78
#define GAPI_METHOD_GET_LIVELINESS_LOST_STATUS                              79
#define GAPI_METHOD_GET_OFFERED_DEADLINE_MISSED_STATUS                      80
#define GAPI_METHOD_GET_OFFERED_INCOMPATIBLE_QOS_STATUS                     81
#define GAPI_METHOD_GET_PUBLICATION_MATCH_STATUS                            82
#define GAPI_METHOD_GET_TOPIC                                               83
#define GAPI_METHOD_GET_PUBLISHER                                           84
#define GAPI_METHOD_GET_MATCHED_SUBSCRIPTION_DATA                           85
#define GAPI_METHOD_GET_MATCHED_SUBSCRIPTIONS                               86

/* DataReader */
#define GAPI_METHOD_READ                                                    87
#define GAPI_METHOD_TAKE                                                    88
#define GAPI_METHOD_READ_W_CONDITION                                        89
#define GAPI_METHOD_TAKE_W_CONDITION                                        90
#define GAPI_METHOD_READ_NEXT_SAMPLE                                        91
#define GAPI_METHOD_TAKE_NEXT_SAMPLE                                        92
#define GAPI_METHOD_READ_INSTANCE                                           93
#define GAPI_METHOD_TAKE_INSTANCE                                           94
#define GAPI_METHOD_READ_NEXT_INSTANCE                                      95
#define GAPI_METHOD_TAKE_NEXT_INSTANCE                                      96
#define GAPI_METHOD_READ_NEXT_INSTANCE_W_CONDITION                          97
#define GAPI_METHOD_TAKE_NEXT_INSTANCE_W_CONDITION                          98
#define GAPI_METHOD_RETURN_LOAN                                             99
#define GAPI_METHOD_CREATE_READCONDITION                                    100
#define GAPI_METHOD_CREATE_QUERYCONDITION                                   101
#define GAPI_METHOD_DELETE_READCONDITION                                    102
#define GAPI_METHOD_GET_LIVELINESS_CHANGED_STATUS                           103
#define GAPI_METHOD_GET_REQUESTED_DEADLINE_MISSED_STATUS                    104
#define GAPI_METHOD_GET_REQUESTED_INCOMPATIBLE_QOS_STATUS                   105
#define GAPI_METHOD_GET_SAMPLE_REJECTED_STATUS                              106
#define GAPI_METHOD_GET_SUBSCRIPTION_MATCH_STATUS                           107
#define GAPI_METHOD_GET_TOPICDESCRIPTION                                    108
#define GAPI_METHOD_GET_SUBSCRIBER                                          109
#define GAPI_METHOD_WAIT_FOR_HISTORICAL_DATA                                110
#define GAPI_METHOD_GET_MATCHED_PUBLICATION_DATA                            111
#define GAPI_METHOD_GET_MATCHED_PUBLICATIONS                                112

/* Condition */
#define GAPI_METHOD_GET_TRIGGER_VALUE                                       113

/* GuardCondition */
#define GAPI_METHOD_SET_TRIGGER_VALUE                                       114

/* StatusCondition */
#define GAPI_METHOD_SET_ENABLED_STATUSES                                    115
#define GAPI_METHOD_GET_ENABLED_STATUSES                                    116
#define GAPI_METHOD_GET_ENTITY                                              117

/* ReadCondition */
#define GAPI_METHOD_GET_DATAREADER                                          118
#define GAPI_METHOD_GET_SAMPLE_STATE_MASK                                   119
#define GAPI_METHOD_GET_VIEW_STATE_MASK                                     120
#define GAPI_METHOD_GET_INSTANCE_STATE_MASK                                 121

/* QueryCondition  */
#define GAPI_METHOD_GET_QUERY_EXPRESSION                                    122
#define GAPI_METHOD_SET_QUERY_ARGUMENTS                                     123
#define GAPI_METHOD_GET_QUERY_ARGUMENTS                                     124

/* WaitSet */
#define GAPI_METHOD_ATTACH_CONDITION                                        125
#define GAPI_METHOD_DETACH_CONDITION                                        126
#define GAPI_METHOD_WAIT                                                    127
#define GAPI_METHOD_GET_CONDITIONS                                          128

/* Additional methods */
#define GAPI_METHOD_CREATE_VIEW                                             129
#define GAPI_METHOD_DELETE_VIEW                                             130
#define GAPI_METHOD_SET_DEFAULT_DATAREADERVIEW_QOS                          131

/* Domain */
#define GAPI_METHOD_DELETE_DOMAIN                                           132

#define GAPI_METHOD_MAX                                                     133
#define GAPI_ERRORCODE_MAX                                                   22


#define ERROR_CONTEXT "DCPS API"

const char *
gapi_context_getEntityName(
    const gapi_context *_this);

const char *
gapi_context_getMethodName(
    const gapi_context *_this);

const char *
gapi_context_getQosName(
    gapi_unsigned_long qos_id);

const char *
gapi_context_getQosPolicyName(
    gapi_unsigned_long policy_id);

const char *
gapi_context_getQosAttributeName(
    gapi_unsigned_long attr_id);

const char *
gapi_context_getErrorMessage(
    gapi_unsigned_long error_code);

#define gapi_redefineError(context) \
        { \
            os_reportInfo *osInfo = os_reportGetApiInfo(); \
            if (osInfo != NULL) { \
                OS_REPORT_4( OS_API_INFO, ERROR_CONTEXT, \
                             osInfo->reportCode, "%s::%s %s,%s", \
                             gapi_context_getEntityName(context), \
                             gapi_context_getMethodName(context), \
                             gapi_context_getErrorMessage(osInfo->reportCode), \
                             osInfo->description); \
            } else { \
                OS_REPORT_2(OS_API_INFO, ERROR_CONTEXT, \
                            0, "%s::%s : Unspecified error", \
                            gapi_context_getEntityName(context), \
                            gapi_context_getMethodName(context)); \
            } \
        }

#define gapi_errorReport(context,errcode) \
        OS_REPORT_3(OS_API_INFO, ERROR_CONTEXT, \
                    errcode, "%s::%s %s", \
                    gapi_context_getEntityName(context), \
                    gapi_context_getMethodName(context), \
                    gapi_context_getErrorMessage(errcode))

#define gapi_errorInvalidQosPolicy(context,qosId,policyId,attributeId,errcode) \
        OS_REPORT_6(OS_API_INFO, ERROR_CONTEXT, \
                    errcode,"%s::%s %s %s.%s %s", \
                    gapi_context_getEntityName(context), \
                    gapi_context_getMethodName(context), \
                    gapi_context_getQosName(qosId), \
                    gapi_context_getQosPolicyName(policyId), \
                    gapi_context_getQosAttributeName(attributeId), \
                    gapi_context_getErrorMessage(errcode))

#define gapi_errorImmutableQosPolicy(context,qosId,policyId) \
        OS_REPORT_5(OS_API_INFO, ERROR_CONTEXT, \
                    GAPI_ERRORCODE_IMMUTABLE_QOS_POLICY, \
                    "%s::%s %s %s.%s", \
                    gapi_context_getEntityName(context), \
                    gapi_context_getMethodName(context), \
                    gapi_context_getQosName(qosId), \
                    gapi_context_getQosPolicyName(policyId), \
                    gapi_context_getErrorMessage(GAPI_ERRORCODE_IMMUTABLE_QOS_POLICY))

#define gapi_errorInconsistentQosPolicy(context,qosId,policyId1, \
                                        attributeId1,policyId2, \
                                        attributeId2,errcode) \
        OS_REPORT_7(OS_API_INFO, ERROR_CONTEXT, \
                    GAPI_ERRORCODE_INCONSISTENT_QOS, \
                    "%s::%s %s %s.%s inconsistent with %s.%s", \
                    gapi_context_getEntityName(context), \
                    gapi_context_getMethodName(context), \
                    gapi_context_getQosName(qosId), \
                    gapi_context_getQosPolicyName(policyId1), \
                    gapi_context_getQosAttributeName(attributeId1), \
                    gapi_context_getQosPolicyName(policyId2), \
                    gapi_context_getQosAttributeName(attributeId2))

#define gapi_errorUnsupportedQosPolicy(context,qosId,policyId,attributeId,errcode) \
        OS_REPORT_6(OS_API_INFO, ERROR_CONTEXT, \
                    GAPI_ERRORCODE_UNSUPPORTED_QOS_POLICY, \
                    "%s::%s %s.%s.%s %s", \
                    gapi_context_getEntityName(context), \
                    gapi_context_getMethodName(context), \
                    gapi_context_getQosName(qosId), \
                    gapi_context_getQosPolicyName(policyId), \
                    gapi_context_getQosAttributeName(attributeId), \
                    gapi_context_getErrorMessage(errcode))

#define gapi_errorDeprecatedQosPolicy(context,dQosId,dPolicyId,dAttributeId,qosId,policyId,attributeId) \
        OS_REPORT_8(OS_API_INFO, ERROR_CONTEXT, \
                    GAPI_ERRORCODE_UNDEFINED, \
                    "%s::%s %s.%s.%s is deprecated and will be replaced by %s.%s.%s\n" \
                    "Mixed usage of a deprecated policy and its replacement counterpart will trigger an inconsistent QoS error!", \
                    gapi_context_getEntityName(context), \
                    gapi_context_getMethodName(context), \
                    gapi_context_getQosName(dQosId), \
                    gapi_context_getQosPolicyName(dPolicyId), \
                    gapi_context_getQosAttributeName(dAttributeId), \
                    gapi_context_getQosName(qosId), \
                    gapi_context_getQosPolicyName(policyId), \
                    gapi_context_getQosAttributeName(attributeId))

void
gapi_typeParseError (
    sd_errorReport report);

const char *
gapi_retcode_image (
    gapi_returnCode_t retcode);

#endif /* GAPI_ERROR_H */
