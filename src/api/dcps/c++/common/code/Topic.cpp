/*
 *                         Vortex OpenSplice
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR ADLINK
 *   Technology Limited, its affiliated companies and licensors. All rights
 *   reserved.
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
#include "v_kernelParser.h"
#include "u_user.h"
#include "u_observable.h"
#include "DomainParticipant.h"
#include "TopicDescription.h"
#include "Topic.h"
#include "StatusUtils.h"
#include "Constants.h"
#include "QosUtils.h"
#include "ReportUtils.h"

using namespace DDS::OpenSplice::Utils;


DDS::Boolean
DDS::OpenSplice::set_topic_listener_mask (
  DDS::Object_ptr element,
  void *arg)
{
    DDS::OpenSplice::Topic *topic = dynamic_cast<DDS::OpenSplice::Topic *>(element);
    DDS::StatusMask *mask = (DDS::StatusMask *)arg;
    DDS::Boolean result = false;

    if (topic) {
        result = topic->set_participant_listener_mask(*mask) == DDS::RETCODE_OK;
    }
    return result;
}

DDS::OpenSplice::Topic::Topic() :
    DDS::OpenSplice::Entity(DDS::OpenSplice::TOPIC),
    topicListenerInterest(0),
    participantListenerInterest(0)
{
}

DDS::OpenSplice::Topic::~Topic()
{
}

DDS::ReturnCode_t
DDS::OpenSplice::Topic::init(
    const u_topic uTopic,
    DDS::OpenSplice::DomainParticipant *participant,
    const DDS::Char *topic_name,
    const DDS::Char *type_name,
    DDS::OpenSplice::TypeSupportMetaHolder *meta_holder)
{
    DDS::ReturnCode_t result;
    result = this->nlReq_init(uTopic, participant, topic_name, type_name, meta_holder);
    return result;
}

DDS::ReturnCode_t
DDS::OpenSplice::Topic::nlReq_init(
    const u_topic uTopic,
    DDS::OpenSplice::DomainParticipant *participant,
    const DDS::Char *topic_name,
    const DDS::Char *type_name,
    DDS::OpenSplice::TypeSupportMetaHolder *meta_holder)
{
    DDS::ReturnCode_t result;
    DDS::Char *expression;
    DDS::ULong length;
    const DDS::Char *prefix = "select * from ";

    assert (participant != NULL);
    assert (topic_name != NULL);
    assert (type_name != NULL);
    assert (uTopic != NULL);

    result = DDS::OpenSplice::Entity::nlReq_init(u_entity(uTopic));

    if (result == DDS::RETCODE_OK) {
        length = strlen(prefix) + strlen(topic_name) + 1;
        expression = DDS::string_alloc(length);
        if ( expression ) {
            snprintf(expression, length, "%s%s", prefix, topic_name);
            result = DDS::OpenSplice::TopicDescription::nlReq_init(participant,
                                                              topic_name,
                                                              type_name,
                                                              expression,
                                                              meta_holder);
            DDS::string_free(expression);
            setDomainId(participant->getDomainId());
        } else {
            result = DDS::RETCODE_OUT_OF_RESOURCES;
            CPP_REPORT(result, "Could not create Topic '%s'.", topic_name);
        }
    }

    return result;
}

DDS::ReturnCode_t
DDS::OpenSplice::Topic::wlReq_deinit()
{
    DDS::ReturnCode_t result;

    result = DDS::OpenSplice::TopicDescription::wlReq_deinit();
    if (result == DDS::RETCODE_OK) {
        this->disable_callbacks();

        result = DDS::OpenSplice::Entity::wlReq_deinit();
    }
    return result;
}

static v_result
copy_inconsistent_topic_status (
    c_voidp info,
    c_voidp arg)
{
    struct v_inconsistentTopicInfo *from;
    ::DDS::InconsistentTopicStatus *to;

    from = (struct v_inconsistentTopicInfo *)info;
    to = (::DDS::InconsistentTopicStatus *)arg;

    to->total_count = from->totalCount;
    to->total_count_change = from->totalChanged;

    return V_RESULT_OK;
}

DDS::ReturnCode_t
DDS::OpenSplice::Topic::get_inconsistent_topic_status (
    DDS::InconsistentTopicStatus &a_status
) THROW_ORB_EXCEPTIONS
{
    DDS::ReturnCode_t result;
    u_result uResult;

    CPP_REPORT_STACK();

    result = this->check ();
    if (result == DDS::RETCODE_OK) {
        uResult = u_topicGetInconsistentTopicStatus(
                      u_topic(DDS::OpenSplice::Entity::rlReq_get_user_entity()),
                      TRUE,
                      copy_inconsistent_topic_status,
                      &a_status);
        result = this->uResultToReturnCode(uResult);
        if (result != DDS::RETCODE_OK) {
            CPP_REPORT(result, "Could not get topic status.");
        }
    }

    CPP_REPORT_FLUSH(this, result != DDS::RETCODE_OK);

    return result;
}

DDS::ReturnCode_t
DDS::OpenSplice::Topic::validate_filter (
    const DDS::Char *filter_expression,
    const DDS::StringSeq &filter_parameters
) THROW_ORB_EXCEPTIONS
{
    DDS::ReturnCode_t result;
    q_expr expr = NULL;
    c_value *params = NULL;
    u_topic uTopic = NULL;
    int n, length;

    CPP_REPORT_STACK();

    result = DDS::RETCODE_BAD_PARAMETER;
    length = filter_parameters.length();
    if ((length >= 0) && (length < 100)) {
        expr = v_parser_parse(filter_expression);
        if (expr != NULL) {
            if(length > 0) {
                params = (c_value *)os_malloc(length * sizeof(struct c_value));
                for (n=0;n<length;n++) {
                    const c_string param = (const c_string) (const char *)(filter_parameters[n]);
                    params[n] = c_stringValue(param);
                }
            }
        } else {
            CPP_REPORT(result, "filter_expression '%s' is invalid", filter_expression);
        }
    } else {
        CPP_REPORT(result, "Invalid number of filter_parameters '%d', maximum is 99", length);
    }

    if (expr) {
        uTopic = u_topic(DDS::OpenSplice::Entity::rlReq_get_user_entity());
        if (u_topicContentFilterValidate2(uTopic, expr, params, length)) {
            result = DDS::RETCODE_OK;
        } else {
            CPP_REPORT(result, "filter_expression '%s' is invalid.", filter_expression);
        }
        q_dispose(expr);
        os_free(params);
    }

    CPP_REPORT_FLUSH(this, result != DDS::RETCODE_OK);

    return result;
}

static v_result
copy_all_data_disposed_status (
    c_voidp info,
    c_voidp arg)
{
    struct v_allDataDisposedInfo *from;
    ::DDS::AllDataDisposedTopicStatus *to;

    from = (struct v_allDataDisposedInfo *)info;
    to = (::DDS::AllDataDisposedTopicStatus *)arg;

    to->total_count = from->totalCount;
    to->total_count_change = from->totalChanged;

    return V_RESULT_OK;
}

DDS::ReturnCode_t
DDS::OpenSplice::Topic::get_all_data_disposed_topic_status (
    DDS::AllDataDisposedTopicStatus & a_status
) THROW_ORB_EXCEPTIONS
{
    DDS::ReturnCode_t result;
    u_result uResult;

    CPP_REPORT_STACK();

    result = this->check ();
    if (result == DDS::RETCODE_OK) {
        uResult = u_topicGetAllDataDisposedStatus(
                      u_topic(DDS::OpenSplice::Entity::rlReq_get_user_entity()),
                      TRUE,
                      copy_all_data_disposed_status,
                      &a_status);
        result = this->uResultToReturnCode(uResult);
        if (result != DDS::RETCODE_OK) {
            CPP_REPORT(result, "Could not get topic status.");
        }
    }

    CPP_REPORT_FLUSH(this, result != DDS::RETCODE_OK);

    return result;
}

DDS::ReturnCode_t
DDS::OpenSplice::Topic::get_qos (
    DDS::TopicQos & qos
) THROW_ORB_EXCEPTIONS
{
    DDS::ReturnCode_t result;
    u_topic uTopic;
    u_topicQos uTopicQos;
    u_result uResult;

    CPP_REPORT_STACK();

    if (&qos == &TOPIC_QOS_DEFAULT) {
        result = DDS::RETCODE_BAD_PARAMETER;
        CPP_REPORT(result, "QoS 'TOPIC_QOS_DEFAULT' is read-only.");
    } else {
        result = this->check ();
    }

    if (result == DDS::RETCODE_OK) {
        uTopic = u_topic (rlReq_get_user_entity ());
        uResult = u_topicGetQos (uTopic, &uTopicQos);
        result = uResultToReturnCode (uResult);
        if (result == DDS::RETCODE_OK) {
            result = DDS::OpenSplice::Utils::copyQosOut (uTopicQos, qos);
            u_topicQosFree(uTopicQos);
        } else {
            CPP_REPORT(result, "Could not copy TopicQos.");
        }
    }

    CPP_REPORT_FLUSH(this, result != DDS::RETCODE_OK);

    return result;
}

DDS::ReturnCode_t
DDS::OpenSplice::Topic::set_qos (
    const DDS::TopicQos & qos
) THROW_ORB_EXCEPTIONS
{
    DDS::ReturnCode_t result;
    DDS::TopicQos topicQos;
    DDS::TopicQos *topicQos_ptr;
    u_topic uTopic;
    u_topicQos uTopicQos;
    u_result uResult;

    CPP_REPORT_STACK();

    if (&qos == &TOPIC_QOS_DEFAULT) {
        topicQos_ptr = NULL;
        result = DDS::RETCODE_OK;
    } else {
        topicQos_ptr = const_cast<DDS::TopicQos *>(&qos);
        result = DDS::OpenSplice::Utils::qosIsConsistent (qos);
    }

    if (result == DDS::RETCODE_OK) {
        uTopicQos = u_topicQosNew (NULL);
        if (uTopicQos == NULL) {
            result = DDS::RETCODE_OUT_OF_RESOURCES;
            CPP_REPORT(result, "Could not copy TopicQos.");
        }
    }

    if (result == DDS::RETCODE_OK) {
        result = this->write_lock ();
        if (result == DDS::RETCODE_OK) {
            if (topicQos_ptr == NULL) {
                result = DDS::OpenSplice::TopicDescription::participant->get_default_topic_qos (topicQos);
                topicQos_ptr = &topicQos;
            }

            if (result == DDS::RETCODE_OK) {
                result = DDS::OpenSplice::Utils::copyQosIn (
                                                    *topicQos_ptr, uTopicQos);
                if (result == DDS::RETCODE_OK) {
                    uTopic = u_topic (rlReq_get_user_entity ());
                    uResult = u_topicSetQos (uTopic, uTopicQos);
                    result = uResultToReturnCode (uResult);
                    if (result != DDS::RETCODE_OK) {
                        CPP_REPORT(result, "Could not apply TopicQos.");
                    }
                }
            }
            this->unlock ();
        }

        u_topicQosFree (uTopicQos);
    }

    CPP_REPORT_FLUSH(this, result != DDS::RETCODE_OK);

    return result;
}

DDS::TopicListener_ptr
DDS::OpenSplice::Topic::get_listener (
) THROW_ORB_EXCEPTIONS
{
    DDS::TopicListener_ptr listener;

    listener = dynamic_cast<DDS::TopicListener_ptr>(
        DDS::OpenSplice::Entity::nlReq_get_listener());

    return listener;
}

DDS::ReturnCode_t
DDS::OpenSplice::Topic::set_listener (
    DDS::TopicListener_ptr a_listener,
    DDS::StatusMask mask
) THROW_ORB_EXCEPTIONS
{
    DDS::ReturnCode_t result;

    CPP_REPORT_STACK();

    result = this->write_lock ();
    if (result == DDS::RETCODE_OK) {
        result = DDS::OpenSplice::Entity::wlReq_set_listener(a_listener, mask | participantListenerInterest);
        if (result == DDS::RETCODE_OK) {
            if (a_listener != NULL) {
                topicListenerInterest = mask;
            } else {
                topicListenerInterest = 0;
            }
        }
        this->unlock ();
    }

    CPP_REPORT_FLUSH(this, result != DDS::RETCODE_OK);

    return result;
}

DDS::ReturnCode_t
DDS::OpenSplice::Topic::set_participant_listener_mask (
    ::DDS::StatusMask mask
) THROW_ORB_EXCEPTIONS
{
    DDS::ReturnCode_t result;

    CPP_REPORT_STACK();

    result = this->write_lock ();
    if (result == DDS::RETCODE_OK) {
        result = DDS::OpenSplice::Entity::wlReq_set_listener_mask(mask | topicListenerInterest);
        if (result == DDS::RETCODE_OK) {
            participantListenerInterest = mask;
        }
        this->unlock ();
    }

    CPP_REPORT_FLUSH(this, result != DDS::RETCODE_OK);

    return result;
}

DDS::ReturnCode_t
DDS::OpenSplice::Topic::dispose_all_data (
) THROW_ORB_EXCEPTIONS
{
    DDS::ReturnCode_t result;
    u_topic uTopic;
    u_result uResult;

    CPP_REPORT_STACK();

    result = this->write_lock ();
    if (result == DDS::RETCODE_OK) {
        uTopic = u_topic(DDS::OpenSplice::Entity::rlReq_get_user_entity());
        uResult = u_topicDisposeAllData(uTopic);
        result = uResultToReturnCode (uResult);
        if (result != DDS::RETCODE_OK) {
            CPP_REPORT(result, "Could not dispose all data for Topic.");
        }
        this->unlock ();
    }

    CPP_REPORT_FLUSH(this, result != DDS::RETCODE_OK);

    return result;
}

void
DDS::OpenSplice::Topic::nlReq_notify_listener(
    DDS::OpenSplice::Entity *sourceEntity,
    DDS::ULong               triggerMask,
    void                    *eventData)
{
    DDS::TopicListener_ptr listener;

    /* Using _narrow to cast Listener, this increases the refcount to ensure
     * the ListenerObject is not deleted while notifying. */
    listener = DDS::TopicListener::_narrow(this->listener);
    if (triggerMask & V_EVENT_INCONSISTENT_TOPIC) {
        if (listener && (topicListenerInterest & DDS::INCONSISTENT_TOPIC_STATUS)) {
            DDS::InconsistentTopicStatus status;
            DDS::OpenSplice::Utils::copyStatusOut(v_topicStatus(eventData)->inconsistentTopic, status);
            listener->on_inconsistent_topic(dynamic_cast<DDS::Topic_ptr>(sourceEntity), status);
        } else if (participantListenerInterest & DDS::INCONSISTENT_TOPIC_STATUS) {
            participant->nlReq_notify_listener(sourceEntity, triggerMask, eventData);
        }
    }

    if (triggerMask & V_EVENT_ALL_DATA_DISPOSED) {
        if (listener && (topicListenerInterest & DDS::ALL_DATA_DISPOSED_TOPIC_STATUS)) {
            DDS::ExtTopicListener_ptr extListener;
            extListener = dynamic_cast<DDS::ExtTopicListener_ptr>(listener);
            if (extListener) {
                extListener->on_all_data_disposed(dynamic_cast<DDS::Topic_ptr>(sourceEntity));
            }
        } else if (participantListenerInterest & DDS::ALL_DATA_DISPOSED_TOPIC_STATUS) {
            participant->nlReq_notify_listener(sourceEntity, triggerMask, eventData);
        }
    }
    DDS::release(listener);
}
