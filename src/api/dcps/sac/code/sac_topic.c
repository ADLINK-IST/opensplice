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

#include "dds_dcps.h"
#include "sac_common.h"
#include "sac_object.h"
#include "sac_entity.h"
#include "sac_topicDescription.h"
#include "sac_domainParticipant.h"
#include "u_entity.h"
#include "v_status.h"
#include "v_kernelParser.h"
#include "sac_report.h"

#define DDS_TopicClaim(_this, topic) \
        DDS_Object_claim(DDS_Object(_this), DDS_TOPIC, (_Object *)topic)

#define DDS_TopicClaimRead(_this, topic) \
        DDS_Object_claim(DDS_Object(_this), DDS_TOPIC, (_Object *)topic)

#define DDS_TopicRelease(_this) \
        DDS_Object_release(DDS_Object(_this))

#define DDS_TopicCheck(_this, topic) \
        DDS_Object_check_and_assign(DDS_Object(_this), DDS_TOPIC, (_Object *)topic)

#define _Topic_get_user_entity(_this) \
        u_topic(_Entity_get_user_entity(_Entity(_Topic(_this))))

#define TOPIC_STATUS_MASK (DDS_INCONSISTENT_TOPIC_STATUS|\
                           DDS_ALL_DATA_DISPOSED_TOPIC_STATUS)

DDS_ReturnCode_t
_Topic_deinit (
    _Object _this)
{
    (void)DDS_Entity_set_listener_interest(DDS_Entity(_this), 0);
    return DDS_TopicDescription_deinit(DDS_TopicDescription(_this));
}

DDS_Topic
DDS_TopicNew (
    const DDS_DomainParticipant participant,
    const DDS_char *topic_name,
    const DDS_char *type_name,
    const DDS_TypeSupport typeSupport,
    const u_topic uTopic)
{
    DDS_ReturnCode_t result;
    DDS_char *expression;
    size_t length;
    _Topic topic = NULL;
    const DDS_char *prefix = "select * from ";

    length = strlen(prefix) + strlen(topic_name) + 1;
    expression = (char *) os_malloc(length);

    if ( expression ) {
        result = DDS_Object_new(DDS_TOPIC, _Topic_deinit, (_Object *)&topic);
        if (result == DDS_RETCODE_OK) {
            DDS_Object_set_domain_id(_Object(topic), DDS_Object_get_domain_id(participant));
            snprintf(expression, length, "%s%s", prefix, topic_name);
            result = DDS_TopicDescription_init(topic, participant,
                                               topic_name, type_name,
                                               expression, typeSupport, uTopic);
            if (result == DDS_RETCODE_OK) {
                cmn_listenerDispatcher listenerDispatcher;

                listenerDispatcher = DDS_Entity_get_listenerDispatcher(participant);
                result = DDS_Entity_set_listenerDispatcher(topic, listenerDispatcher);
            }
            topic->topicListenerInterest = 0;
            topic->participantListenerInterest = 0;
        }
        os_free(expression);
    }
    return (DDS_Topic)topic;
}

static v_result
copy_inconsistent_topic_status (
    c_voidp info,
    c_voidp arg)
{
    struct v_inconsistentTopicInfo *from;
    DDS_InconsistentTopicStatus *to;

    from = (struct v_inconsistentTopicInfo *)info;
    to = (DDS_InconsistentTopicStatus *)arg;

    to->total_count = from->totalCount;
    to->total_count_change = from->totalChanged;

    return V_RESULT_OK;
}

/*     // Access the status
 *     InconsistentTopicStatus
 *     get_inconsistent_topic_status();
 */
DDS_ReturnCode_t
DDS_Topic_get_inconsistent_topic_status (
    DDS_Topic _this,
    DDS_InconsistentTopicStatus *status)
{
    DDS_ReturnCode_t result;
    _Topic topic;
    u_result uResult;

    SAC_REPORT_STACK();

    if (status != NULL) {
        result = DDS_TopicCheck(_this, &topic);
        if (result == DDS_RETCODE_OK) {
            uResult = u_topicGetInconsistentTopicStatus(
                          u_topic(_Topic_get_user_entity(topic)),
                          TRUE,
                          copy_inconsistent_topic_status,
                          status);
            result = DDS_ReturnCode_get(uResult);
        }
    } else {
        result = DDS_RETCODE_BAD_PARAMETER;
        SAC_REPORT(result, "InconsistentTopicStatus holder = NULL");
    }
    SAC_REPORT_FLUSH(_this, result != DDS_RETCODE_OK);
    return result;
}

/*     ReturnCode_t
 *     set_listener(
 *         in TopicListener a_listener,
 *         in StatusKindMask mask);
 */
DDS_ReturnCode_t
DDS_Topic_set_listener (
    DDS_Topic _this,
    const struct DDS_TopicListener *a_listener,
    const DDS_StatusMask mask)
{
    DDS_ReturnCode_t result;
    _Topic topic;

    SAC_REPORT_STACK();

    result = DDS_TopicClaim(_this, &topic);
    if (result == DDS_RETCODE_OK) {
        memset(&topic->listener, 0, sizeof(struct DDS_ExtTopicListener));
        topic->topicListenerInterest = mask;
        if (a_listener != NULL) {
            if (mask & DDS_ALL_DATA_DISPOSED_TOPIC_STATUS) {
                memcpy(&topic->listener, a_listener, sizeof(struct DDS_ExtTopicListener));
            } else {
                memcpy(&topic->listener, a_listener, sizeof(struct DDS_TopicListener));
            }
        }
        result = DDS_Entity_set_listener_interest(DDS_Entity(topic),
                                         topic->topicListenerInterest |
                                         topic->participantListenerInterest);
        DDS_TopicRelease(_this);
    }
    SAC_REPORT_FLUSH(_this, result != DDS_RETCODE_OK);
    return result;
}

/*     TopicListener
 *     get_listener();
 */
struct DDS_TopicListener
DDS_Topic_get_listener (
    DDS_Topic _this)
{
    DDS_ReturnCode_t result;
    _Topic topic;
    struct DDS_TopicListener listener;

    SAC_REPORT_STACK();

    result = DDS_TopicClaimRead(_this, &topic);
    if (result == DDS_RETCODE_OK) {
        memcpy(&listener, &topic->listener, sizeof(struct DDS_TopicListener));
        DDS_TopicRelease(_this);
    } else {
        memset(&listener, 0, sizeof(struct DDS_TopicListener));
    }
    SAC_REPORT_FLUSH(_this, result != DDS_RETCODE_OK);
    return listener;
}

/*     ReturnCode_t
 *     set_listener_mask(
 *         in StatusKindMask mask);
 */
DDS_ReturnCode_t
DDS_Topic_set_listener_mask (
    _Topic _this,
    const DDS_StatusMask mask)
{
    DDS_ReturnCode_t result;

    assert(_this);

    _this->topicListenerInterest = mask;
    result = DDS_Entity_set_listener_interest(DDS_Entity(_this),
                                              _this->topicListenerInterest |
                                              _this->participantListenerInterest);

    return result;
}


/*     ReturnCode_t
 *     set_qos(
 *         in TopicQos qos);
 */
DDS_ReturnCode_t
DDS_Topic_set_qos (
    DDS_Topic _this,
    const DDS_TopicQos *qos)
{
    DDS_ReturnCode_t result = DDS_RETCODE_OK;
    _Topic topic;
    u_topicQos tQos = NULL;
    u_result uResult;
    u_topic uTopic;
    DDS_TopicQos topicQos;

    SAC_REPORT_STACK();

    memset(&topicQos, 0, sizeof(DDS_TopicQos));
    (void)DDS_TopicQos_init(&topicQos, DDS_TOPIC_QOS_DEFAULT);

    result = DDS_TopicQos_is_consistent(qos);
    if (result == DDS_RETCODE_OK) {
        result = DDS_TopicClaim(_this, &topic);
    }
    if (result == DDS_RETCODE_OK) {
        if (qos == DDS_TOPIC_QOS_DEFAULT) {
            result = DDS_DomainParticipant_get_default_topic_qos(
                _TopicDescription(topic), &topicQos);
            qos = &topicQos;
        }
        if (result == DDS_RETCODE_OK) {
            tQos = DDS_TopicQos_copyIn(qos);
            if (tQos == NULL) {
                result = DDS_RETCODE_OUT_OF_RESOURCES;
                SAC_REPORT(result, "Failed to copy DDS_TopicQos");
            }
        }
        if (tQos != NULL) {
            uTopic = _Topic_get_user_entity(topic);
            uResult = u_topicSetQos(uTopic, tQos);
            result = DDS_ReturnCode_get(uResult);
            u_topicQosFree(tQos);
        }
        DDS_TopicRelease(_this);
    }

    (void)DDS_TopicQos_deinit(&topicQos);

    SAC_REPORT_FLUSH(_this, result != DDS_RETCODE_OK);
    return result;
}

/*     ReturnCode_t
 *     get_qos(
 *         inout TopicQos qos);
 */
DDS_ReturnCode_t
DDS_Topic_get_qos (
    DDS_Topic _this,
    DDS_TopicQos *qos)
{
    DDS_ReturnCode_t result;
    _Topic t;
    u_topicQos uQos;
    u_result uResult;

    SAC_REPORT_STACK();

    result = DDS_TopicCheck(_this, &t);
    if (result == DDS_RETCODE_OK) {
        if (qos == NULL) {
            result = DDS_RETCODE_BAD_PARAMETER;
            SAC_REPORT(result, "TopicQos = NULL");
        } else if (qos == DDS_TOPIC_QOS_DEFAULT) {
            result = DDS_RETCODE_BAD_PARAMETER;
            SAC_REPORT(result, "QoS 'TOPIC_QOS_DEFAULT' is read-only.");
        }
    }
    if (result == DDS_RETCODE_OK) {
        uResult = u_topicGetQos(_Topic_get_user_entity(t), &uQos);
        if (uResult == U_RESULT_OK) {
            result = DDS_TopicQos_copyOut(uQos, qos);
            u_topicQosFree(uQos);
        } else {
            result = DDS_ReturnCode_get(uResult);
        }
    }
    SAC_REPORT_FLUSH(_this, result != DDS_RETCODE_OK);
    return result;
}

/*     DDS_ReturnCode_t
 *     dispose_all_data();
 */
DDS_ReturnCode_t
DDS_Topic_dispose_all_data (
    DDS_Topic _this)
{
    DDS_ReturnCode_t result;
    _Topic topic;
    u_topic uTopic;
    u_result uResult;

    SAC_REPORT_STACK();

    result = DDS_TopicClaim(_this, &topic);
    if (result == DDS_RETCODE_OK) {
        uTopic = u_topic(_Topic_get_user_entity(topic));
        uResult = u_topicDisposeAllData(uTopic);
        result = DDS_ReturnCode_get(uResult);
        DDS_TopicRelease(_this);
    }
    SAC_REPORT_FLUSH(_this, result != DDS_RETCODE_OK);
    return result;
}

DDS_ReturnCode_t
DDS_Topic_set_participantListenerInterest(
    DDS_Topic _this,
    const DDS_StatusMask interest)
{
    DDS_ReturnCode_t result;
    _Topic topic;

    SAC_REPORT_STACK();

    result = DDS_TopicClaim(_this, &topic);
    if (result == DDS_RETCODE_OK) {
        topic->participantListenerInterest = interest;
        result = DDS_Entity_set_listener_interest(DDS_Entity(topic),
                                                  topic->topicListenerInterest |
                                                  topic->participantListenerInterest);
        DDS_TopicRelease(_this);
        result = DDS_RETCODE_OK;
    }
    SAC_REPORT_FLUSH(_this, result != DDS_RETCODE_OK);
    return result;
}

DDS_ReturnCode_t
DDS_Topic_notify_listener(
    DDS_Topic _this,
    v_listenerEvent event)
{
    DDS_ReturnCode_t result;
    DDS_StatusMask participantInterest;
    u_eventMask triggerMask;
    struct DDS_ExtTopicListener cb;

    cb = _Topic(_this)->listener;
    triggerMask = event->kind;
    result = DDS_RETCODE_OK;

    participantInterest = _Topic(_this)->participantListenerInterest;
    if (triggerMask & V_EVENT_INCONSISTENT_TOPIC) {
        DDS_InconsistentTopicStatus status;
        DDS_InconsistentTopicStatus_init(&status, &((v_topicStatus)event->eventData)->inconsistentTopic);
        if (cb.on_inconsistent_topic != NULL) {
            cb.on_inconsistent_topic(cb.listener_data, _this, &status);
        } else if (participantInterest & DDS_INCONSISTENT_TOPIC_STATUS) {
            DDS_DomainParticipant_notify_listener(_TopicDescription(_this)->participant, event);
        }
    }
    if ( triggerMask & V_EVENT_ALL_DATA_DISPOSED ) {
        if (cb.on_all_data_disposed != NULL) {
            cb.on_all_data_disposed(cb.listener_data, _this);
        } else if (participantInterest & DDS_ALL_DATA_DISPOSED_TOPIC_STATUS) {
            DDS_DomainParticipant_notify_listener(_TopicDescription(_this)->participant, event);
        }
    }

    return result;
}

/*     DDS_string
 *     DDS_Topic_get_metadescription();
 */
DDS_string
DDS_Topic_get_metadescription (
    DDS_Topic _this)
{
    DDS_ReturnCode_t result;
    _Topic topic;
    u_topic uTopic;
    DDS_string descriptor = NULL;
    os_char *tmp;

    SAC_REPORT_STACK();

    result = DDS_TopicCheck(_this, &topic);
    if (result == DDS_RETCODE_OK) {
        uTopic = u_topic(_Topic_get_user_entity(topic));
        tmp = u_topicMetaDescriptor(uTopic);
        descriptor = DDS_string_dup(tmp);
        os_free(tmp);
    }
    SAC_REPORT_FLUSH(_this, result != DDS_RETCODE_OK);
    return descriptor;
}

/*     DDS_string
 *     DDS_Topic_get_keylist();
 */
DDS_string
DDS_Topic_get_keylist (
    DDS_Topic _this)
{
    DDS_ReturnCode_t result;
    _Topic topic;
    u_topic uTopic;
    DDS_string keylist = NULL;
    os_char *tmp;

    SAC_REPORT_STACK();

    result = DDS_TopicCheck(_this, &topic);
    if (result == DDS_RETCODE_OK) {
        uTopic = u_topic(_Topic_get_user_entity(topic));
        tmp = u_topicKeyExpr(uTopic);
        keylist = DDS_string_dup(tmp);
        os_free(tmp);
    }
    SAC_REPORT_FLUSH(_this, result != DDS_RETCODE_OK);
    return keylist;
}

DDS_ReturnCode_t
DDS_Topic_validate_filter(
    DDS_Topic _this,
    const DDS_char *filter_expression,
    const DDS_StringSeq *filter_parameters)
{
    DDS_ReturnCode_t result = DDS_RETCODE_OK;
    _Topic topic;
    q_expr expr = NULL;
    c_value *params = NULL;
    u_topic uTopic = NULL;
    DDS_unsigned_long n;


    if (filter_parameters->_length < 100) {
        expr = v_parser_parse(filter_expression);
        if (expr) {
            if(filter_parameters->_length > 0) {
                params = (c_value *)os_malloc(filter_parameters->_length * sizeof(struct c_value));
                for (n = 0; n < filter_parameters->_length; n++) {
                    params[n] = c_stringValue(filter_parameters->_buffer[n]);
                }
            }
        } else {
            result = DDS_RETCODE_BAD_PARAMETER;
            SAC_REPORT(result, "Invalid filter expression: %s",
                         filter_expression);
        }
    } else {
        result = DDS_RETCODE_BAD_PARAMETER;
        SAC_REPORT(result, "Invalid number of parameters %d (0 <= nrOfParams < max(99))",
                     filter_parameters->_length);
    }
    if (expr) {
        result = DDS_TopicCheck(_this, &topic);
        if (result == DDS_RETCODE_OK) {
            uTopic = u_topic(_Topic_get_user_entity(topic));
            if (u_topicContentFilterValidate2(uTopic, expr, params)) {
                result = DDS_RETCODE_OK;
            } else {
                result = DDS_RETCODE_BAD_PARAMETER;
                SAC_REPORT(result, "Invalid filter expression: %s", filter_expression);
            }
        }
        q_dispose(expr);
        os_free(params);
    }
    return result;
}

DDS_ReturnCode_t
DDS_Topic_get_typeSupport(
    DDS_Topic _this,
    DDS_TypeSupport *ts)
{
	return DDS_TopicDescription_get_typeSupport(_this, ts);
}

