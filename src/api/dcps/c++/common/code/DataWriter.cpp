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
#include <string.h>
#include "os_heap.h"
#include "DataWriter.h"
#include "Constants.h"
#include "QosUtils.h"
#include "StatusUtils.h"
#include "MiscUtils.h"
#include "StatusUtils.h"
#include "ReportUtils.h"
#include "SequenceUtils.h"
#include "dds_builtinTopicsSplDcps.h"

#include "u_observable.h"

using namespace DDS::OpenSplice::Utils;

DDS::OpenSplice::DataWriter::DataWriter () :
    DDS::OpenSplice::Entity (DATAWRITER),
    publisher(NULL),
    topic(NULL)
{
    // Do nothing.
}

DDS::OpenSplice::DataWriter::~DataWriter ()
{
    // Delegate to parent.
}

DDS::ReturnCode_t
DDS::OpenSplice::DataWriter::nlReq_init (
    DDS::OpenSplice::Publisher *publisher,
    const DDS::DataWriterQos &qos,
    DDS::OpenSplice::Topic *a_topic,
    const char *name)
{
    DDS::ReturnCode_t result = DDS::RETCODE_OK;
    u_writerQos uWriterQos = NULL;
    u_writer uWriter = NULL;

    assert (publisher != NULL);
    /* Only check for QoS consistency here in debug builds. It's unnecessary to
       to verify it twice in release builds. */
    assert (DDS::OpenSplice::Utils::qosIsConsistent (qos) == DDS::RETCODE_OK);
    assert (a_topic != NULL);
    assert (name != NULL);

    uWriterQos = u_writerQosNew (NULL);
    if (uWriterQos == NULL) {
        result = DDS::RETCODE_OUT_OF_RESOURCES;
        CPP_REPORT(result, "Could not copy DataWriterQos.");
    } else {
        result = DDS::OpenSplice::Utils::copyQosIn (qos, uWriterQos);
    }

    if (result == DDS::RETCODE_OK) {
        result = a_topic->write_lock ();
    }

    if (result == DDS::RETCODE_OK) {
        uWriter = u_writerNew (
            u_publisher (publisher->rlReq_get_user_entity ()),
            name,
            u_topic (a_topic->rlReq_get_user_entity ()),
            uWriterQos);
        if (!uWriter) {
            result = DDS::RETCODE_OUT_OF_RESOURCES;
            CPP_REPORT(result, "Could not create DataWriter.");
        }

        if (result == DDS::RETCODE_OK) {
            result = DDS::OpenSplice::Entity::nlReq_init (u_entity (uWriter));
            if (result == DDS::RETCODE_OK) {
                (void) DDS::Publisher::_duplicate(publisher);
                this->publisher = publisher;
                (void) DDS::Topic::_duplicate(a_topic);
                this->topic = a_topic;
                a_topic->wlReq_incrNrUsers();
                setDomainId(publisher->getDomainId());
            }
        }

        a_topic->unlock ();
    }

    if (uWriterQos) {
        u_writerQosFree (uWriterQos);
    }

    return result;
}

DDS::ReturnCode_t
DDS::OpenSplice::DataWriter::wlReq_deinit ()
{
    DDS::ReturnCode_t result;

    this->disable_callbacks();

    if (this->topic != NULL) {
        result = this->topic->write_lock();
        if (result == DDS::RETCODE_OK) {
            this->topic->wlReq_decrNrUsers();
            this->topic->unlock();
        } else {
            CPP_PANIC("Could not lock Topic.");
        }
        DDS::release (this->topic);
        this->topic = NULL;
    }
    if (this->publisher != NULL) {
        DDS::release (this->publisher);
        this->publisher = NULL;
    }
    result = DDS::OpenSplice::Entity::wlReq_deinit();

    return result;
}

DDS::ReturnCode_t
DDS::OpenSplice::DataWriter::set_qos (
    const DDS::DataWriterQos &qos
) THROW_ORB_EXCEPTIONS
{
    DDS::ReturnCode_t result;
    DDS::DataWriterQos writerQos;
    DDS::TopicQos topicQos;
    u_writer uWriter;
    u_writerQos uWriterQos = NULL;
    u_result uResult;

    CPP_REPORT_STACK();

    result = DDS::OpenSplice::Utils::qosIsConsistent (qos);

    if (result == DDS::RETCODE_OK) {
        uWriterQos = u_writerQosNew (NULL);
        if (uWriterQos == NULL) {
            result = DDS::RETCODE_OUT_OF_RESOURCES;
            CPP_REPORT(result, "Could not copy DataWriterQos.");
        }
    }

    if (result == DDS::RETCODE_OK) {
        result = this->write_lock();
        if (result == DDS::RETCODE_OK) {
            if (&qos == &DATAWRITER_QOS_DEFAULT) {
                /* QoS consistency is checked by set_default_datawriter_qos */
                result = publisher->get_default_datawriter_qos (writerQos);
                if (result == DDS::RETCODE_OK) {
                    result = DDS::OpenSplice::Utils::copyQosIn (
                        writerQos, uWriterQos);
                }
            } else if (&qos == &DATAWRITER_QOS_USE_TOPIC_QOS) {
                result = publisher->get_default_datawriter_qos (writerQos);
                if (result == DDS::RETCODE_OK) {
                    result = topic->get_qos(topicQos);
                    if (result == DDS::RETCODE_OK) {
                        result = publisher->copy_from_topic_qos (
                            writerQos, topicQos);
                        if (result == DDS::RETCODE_OK) {
                            result = DDS::OpenSplice::Utils::qosIsConsistent (
                                writerQos);
                            if (result == DDS::RETCODE_OK) {
                                result = DDS::OpenSplice::Utils::copyQosIn (
                                    writerQos, uWriterQos);
                            }
                        }
                    }
                }
            } else {
                result = DDS::OpenSplice::Utils::copyQosIn (
                    qos, uWriterQos);
            }

            if (result == DDS::RETCODE_OK) {
                uWriter = u_writer (this->rlReq_get_user_entity ());
                assert (uWriter != NULL);
                uResult = u_writerSetQos (uWriter, uWriterQos);
                result = uResultToReturnCode (uResult);
                if (result != DDS::RETCODE_OK) {
                    CPP_REPORT(result, "Could not apply DataWriterQos.");
                }
            }

            this->unlock();
        }
    }

    if (uWriterQos != NULL) {
        u_writerQosFree (uWriterQos);
    }

    CPP_REPORT_FLUSH(this, result != DDS::RETCODE_OK);

    return result;
}

DDS::ReturnCode_t
DDS::OpenSplice::DataWriter::get_qos (
    DDS::DataWriterQos &qos
) THROW_ORB_EXCEPTIONS
{
   DDS::ReturnCode_t result;
   u_writer uWriter;
   u_writerQos uWriterQos;
   u_result uResult;

   CPP_REPORT_STACK();

   if (&qos == &DATAWRITER_QOS_DEFAULT) {
       result = DDS::RETCODE_BAD_PARAMETER;
       CPP_REPORT(result, "QoS 'DATAWRITER_QOS_DEFAULT' is read-only.");
   } else if (&qos == &DATAWRITER_QOS_USE_TOPIC_QOS) {
       result = DDS::RETCODE_BAD_PARAMETER;
       CPP_REPORT(result, "QoS 'DATAWRITER_QOS_USE_TOPIC_QOS' is read-only.");
   } else {
       result = this->check ();
   }

   if (result == DDS::RETCODE_OK) {
       uWriter = u_writer (this->rlReq_get_user_entity ());
       uResult = u_writerGetQos(uWriter, &uWriterQos);
       result = uResultToReturnCode (uResult);
       if (result == DDS::RETCODE_OK) {
           result = DDS::OpenSplice::Utils::copyQosOut (
               uWriterQos, qos);
           u_writerQosFree(uWriterQos);
       } else {
           CPP_REPORT(result, "Could not copy DataWriterQos.");
       }
   }

   CPP_REPORT_FLUSH(this, result != DDS::RETCODE_OK);

   return result;
}

DDS::ReturnCode_t
DDS::OpenSplice::DataWriter::set_listener (
    DDS::DataWriterListener_ptr a_listener,
    DDS::StatusMask mask
) THROW_ORB_EXCEPTIONS
{
    DDS::ReturnCode_t result;

    CPP_REPORT_STACK();
    result = DDS::OpenSplice::Entity::nlReq_set_listener(a_listener, mask);
    CPP_REPORT_FLUSH(this, result != DDS::RETCODE_OK);

    return result;
}

DDS::DataWriterListener_ptr
DDS::OpenSplice::DataWriter::get_listener (
) THROW_ORB_EXCEPTIONS
{
    DDS::DataWriterListener_ptr listener;

    CPP_REPORT_STACK();
    listener = dynamic_cast<DDS::DataWriterListener_ptr>(
        DDS::OpenSplice::Entity::nlReq_get_listener());
    CPP_REPORT_FLUSH(this, listener == NULL);

    return listener;
}

DDS::Topic_ptr
DDS::OpenSplice::DataWriter::get_topic (
) THROW_ORB_EXCEPTIONS
{
    DDS::ReturnCode_t result;
    DDS::Topic_ptr topic_ptr = NULL;

    CPP_REPORT_STACK();

    result = this->check();
    if (result == DDS::RETCODE_OK) {
        topic_ptr = DDS::Topic::_duplicate(this->topic);
    }

    CPP_REPORT_FLUSH(this, result != DDS::RETCODE_OK);

    return topic_ptr;
}

DDS::Publisher_ptr
DDS::OpenSplice::DataWriter::get_publisher (
) THROW_ORB_EXCEPTIONS
{
    DDS::ReturnCode_t result;
    DDS::Publisher_ptr publisher_ptr = NULL;

    CPP_REPORT_STACK();

    result = this->check();
    if (result == DDS::RETCODE_OK) {
        publisher_ptr = DDS::Publisher::_duplicate(this->publisher);
    }

    CPP_REPORT_FLUSH(this, result != DDS::RETCODE_OK);

    return publisher_ptr;
}

DDS::ReturnCode_t
DDS::OpenSplice::DataWriter::wait_for_acknowledgments (
    const DDS::Duration_t &max_wait
) THROW_ORB_EXCEPTIONS
{
    DDS::ReturnCode_t result;
    os_duration timeout;
    u_writer uWriter;
    u_result uResult;

    CPP_REPORT_STACK();

    result = this->check();
    if (result == DDS::RETCODE_OK) {
        result = DDS::OpenSplice::Utils::copyDurationIn(max_wait, timeout);
        if (result == DDS::RETCODE_OK) {
            uWriter = u_writer (this->rlReq_get_user_entity());
            assert (uWriter != NULL);
            uResult = u_writerWaitForAcknowledgments(
                    uWriter, timeout);
            result = uResultToReturnCode(uResult);
        }
    }

    CPP_REPORT_FLUSH(this, (result != DDS::RETCODE_OK) && (result != DDS::RETCODE_TIMEOUT));

    return result;
}

u_result
DDS::OpenSplice::DataWriter::copy_liveliness_lost_status(
    u_status info,
    void *arg)
{
    struct v_livelinessLostInfo *from = reinterpret_cast<struct v_livelinessLostInfo *>(info);
    DDS::LivelinessLostStatus *to = reinterpret_cast<DDS::LivelinessLostStatus *>(arg);

    copyStatusOut(*from, *to);

    return U_RESULT_OK;
}

DDS::ReturnCode_t
DDS::OpenSplice::DataWriter::get_liveliness_lost_status (
    DDS::LivelinessLostStatus &status
) THROW_ORB_EXCEPTIONS
{
    DDS::ReturnCode_t result;
    u_writer uWriter;
    u_result uResult;

    CPP_REPORT_STACK();

    result = this->check();
    if (result == DDS::RETCODE_OK) {
        uWriter = u_writer(this->rlReq_get_user_entity());
        assert (uWriter != NULL);
        uResult = u_writerGetLivelinessLostStatus(
                uWriter, TRUE, copy_liveliness_lost_status, &status);
        result = uResultToReturnCode (uResult);
    }

    CPP_REPORT_FLUSH(this, result != DDS::RETCODE_OK);

    return result;
}

u_result
DDS::OpenSplice::DataWriter::copy_deadline_missed_status(
    u_status info,
    void *arg)
{
    struct v_deadlineMissedInfo *from = reinterpret_cast<struct v_deadlineMissedInfo *>(info);
    DDS::OfferedDeadlineMissedStatus *to = reinterpret_cast<DDS::OfferedDeadlineMissedStatus *>(arg);
    u_result uResult = U_RESULT_INTERNAL_ERROR;
    v_object instance;

    to->total_count = from->totalCount;
    to->total_count_change = from->totalChanged;

    if (!v_handleIsNil(from->instanceHandle)) {
        if (v_handleClaim(from->instanceHandle, &instance) == V_HANDLE_OK) {
            to->last_instance_handle = u_instanceHandleNew(v_public(instance));
            if (v_handleRelease(from->instanceHandle) == V_HANDLE_OK) {
                uResult = U_RESULT_OK;
            }
        }
    } else {
        uResult = U_RESULT_OK;
    }

    return uResult;
}

DDS::ReturnCode_t
DDS::OpenSplice::DataWriter::get_offered_deadline_missed_status (
    DDS::OfferedDeadlineMissedStatus &status
) THROW_ORB_EXCEPTIONS
{
    DDS::ReturnCode_t result;
    u_writer uWriter;
    u_result uResult;

    CPP_REPORT_STACK();

    result = this->check ();
    if (result == DDS::RETCODE_OK) {
        uWriter = u_writer(this->rlReq_get_user_entity());
        assert (uWriter != NULL);
        uResult = u_writerGetDeadlineMissedStatus(
                uWriter, TRUE, copy_deadline_missed_status, &status);
        result = uResultToReturnCode (uResult);
    }

    CPP_REPORT_FLUSH(this, result != DDS::RETCODE_OK);

    return result;
}

u_result
DDS::OpenSplice::DataWriter::copy_incompatible_qos_status(
    u_status info,
    void *arg)
{
    DDS::ReturnCode_t result;
    struct v_incompatibleQosInfo *from = reinterpret_cast<struct v_incompatibleQosInfo *>(info);
    DDS::OfferedIncompatibleQosStatus *to = reinterpret_cast<DDS::OfferedIncompatibleQosStatus *>(arg);

    result = copyStatusOut(*from, *to);

    return ReturnCodeTouResult(result);
}

DDS::ReturnCode_t
DDS::OpenSplice::DataWriter::get_offered_incompatible_qos_status (
    DDS::OfferedIncompatibleQosStatus &status
) THROW_ORB_EXCEPTIONS
{
    DDS::ReturnCode_t result;
    u_writer uWriter;
    u_result uResult;

    CPP_REPORT_STACK();

    result = this->check ();
    if (result == DDS::RETCODE_OK) {
        uWriter = u_writer(this->rlReq_get_user_entity());
        assert (uWriter != NULL);
        uResult = u_writerGetIncompatibleQosStatus(
                uWriter, TRUE, copy_incompatible_qos_status, &status);
        result = uResultToReturnCode (uResult);
    }

    CPP_REPORT_FLUSH(this, result != DDS::RETCODE_OK);

    return result;
}

u_result
DDS::OpenSplice::DataWriter::copy_publication_matched_status(
    u_status info,
    void *arg)
{
    struct v_topicMatchInfo *from = reinterpret_cast<struct v_topicMatchInfo *>(info);
    DDS::PublicationMatchedStatus *to = reinterpret_cast<DDS::PublicationMatchedStatus *>(arg);

    copyStatusOut(*from, *to);

    return U_RESULT_OK;
}

DDS::ReturnCode_t
DDS::OpenSplice::DataWriter::get_publication_matched_status (
    DDS::PublicationMatchedStatus &status
) THROW_ORB_EXCEPTIONS
{
    DDS::ReturnCode_t result;
    u_writer uWriter;
    u_result uResult;

    CPP_REPORT_STACK();

    result = this->check();
    if (result == DDS::RETCODE_OK) {
        uWriter = u_writer(this->rlReq_get_user_entity());
        assert (uWriter != NULL);
        uResult = u_writerGetPublicationMatchStatus(
                uWriter, TRUE, copy_publication_matched_status, &status);
        result = uResultToReturnCode(uResult);
    }

    CPP_REPORT_FLUSH(this, result != DDS::RETCODE_OK);

    return result;
}

DDS::ReturnCode_t
DDS::OpenSplice::DataWriter::assert_liveliness (
) THROW_ORB_EXCEPTIONS
{
    DDS::ReturnCode_t result;
    u_writer uWriter;
    u_result uResult;

    CPP_REPORT_STACK();

    result = this->check();
    if (result == DDS::RETCODE_OK) {
        uWriter = u_writer(this->rlReq_get_user_entity());
        assert (uWriter != NULL);
        uResult = u_writerAssertLiveliness(
                uWriter);
        result = uResultToReturnCode(uResult);
    }

    CPP_REPORT_FLUSH(this, result != DDS::RETCODE_OK);

    return result;
}

v_result
DDS::OpenSplice::DataWriter::copy_matched_subscription(
    u_subscriptionInfo *info,
    void *arg)
{
    DDS::InstanceHandleSeq *seq  = reinterpret_cast<DDS::InstanceHandleSeq *>(arg);
    DDS::InstanceHandle_t handle = u_instanceHandleFromGID(info->key);

    DDS::OpenSplice::Utils::appendSequenceItem<
                                        DDS::InstanceHandleSeq,
                                        DDS::InstanceHandle_t>(
                                                        *seq,
                                                        handle);

    return V_RESULT_OK;
}

DDS::ReturnCode_t
DDS::OpenSplice::DataWriter::get_matched_subscriptions (
    DDS::InstanceHandleSeq &subscription_handles
) THROW_ORB_EXCEPTIONS
{
    DDS::ReturnCode_t result;
    u_writer uWriter;
    u_result uResult;

    CPP_REPORT_STACK();

    result = this->check();
    if (result == DDS::RETCODE_OK) {
        uWriter = u_writer(this->rlReq_get_user_entity());
        assert (uWriter != NULL);
        uResult = u_writerGetMatchedSubscriptions(
                uWriter, copy_matched_subscription, &subscription_handles);
        result = uResultToReturnCode(uResult);
    }

    CPP_REPORT_FLUSH(this, result != DDS::RETCODE_OK);

    return result;
}

v_result
DDS::OpenSplice::DataWriter::copy_matched_subscription_data(
    u_subscriptionInfo *info,
    void *arg)
{
    DDS::SubscriptionBuiltinTopicData *to = reinterpret_cast<DDS::SubscriptionBuiltinTopicData *>(arg);
    __DDS_SubscriptionBuiltinTopicData__copyOut(info, to);
    return V_RESULT_OK;
}

DDS::ReturnCode_t
DDS::OpenSplice::DataWriter::get_matched_subscription_data (
    DDS::SubscriptionBuiltinTopicData &subscription_data,
    DDS::InstanceHandle_t subscription_handle
) THROW_ORB_EXCEPTIONS
{
    DDS::ReturnCode_t result;
    u_writer uWriter;
    u_result uResult;

    CPP_REPORT_STACK();

    result = this->check();
    if (result == DDS::RETCODE_OK) {
        uWriter = u_writer(this->rlReq_get_user_entity());
        assert (uWriter != NULL);
        uResult = u_writerGetMatchedSubscriptionData(
                uWriter, (u_instanceHandle)subscription_handle, copy_matched_subscription_data, &subscription_data);
        result = uResultToReturnCode(uResult);
    }

    CPP_REPORT_FLUSH(this, result != DDS::RETCODE_OK);

    return result;
}

void
DDS::OpenSplice::DataWriter::nlReq_notify_listener(
    DDS::OpenSplice::Entity *sourceEntity,
    DDS::ULong               triggerMask,
    void                    *eventData)
{
    DDS::DataWriterListener_ptr listener;

    /* Using _narrow to cast Listener, this increases the refcount to ensure
     * the ListenerObject is not deleted while notifying. */
    listener = DDS::DataWriterListener::_narrow(this->listener);
    if (listener) {

        if (triggerMask & V_EVENT_LIVELINESS_LOST) {
            DDS::LivelinessLostStatus status;
            DDS::OpenSplice::Utils::copyStatusOut(v_writerStatus(eventData)->livelinessLost,
                                                  status);
            listener->on_liveliness_lost(dynamic_cast<DDS::DataWriter_ptr>(sourceEntity),
                                         status);
        }

        if (triggerMask & V_EVENT_OFFERED_DEADLINE_MISSED) {
            DDS::OfferedDeadlineMissedStatus status;
            DDS::OpenSplice::Utils::copyStatusOut(v_writerStatus(eventData)->deadlineMissed,
                                                  status);
            listener->on_offered_deadline_missed(dynamic_cast<DDS::DataWriter_ptr>(sourceEntity),
                                                 status);
        }

        if (triggerMask & V_EVENT_OFFERED_INCOMPATIBLE_QOS) {
            DDS::OfferedIncompatibleQosStatus status;
            DDS::OpenSplice::Utils::copyStatusOut(v_writerStatus(eventData)->incompatibleQos,
                                                  status);
            listener->on_offered_incompatible_qos(dynamic_cast<DDS::DataWriter_ptr>(sourceEntity),
                                                  status);
        }

        if (triggerMask & V_EVENT_PUBLICATION_MATCHED) {
            DDS::PublicationMatchedStatus status;
            DDS::OpenSplice::Utils::copyStatusOut(v_writerStatus(eventData)->publicationMatch,
                                                  status);
            listener->on_publication_matched(dynamic_cast<DDS::DataWriter_ptr>(sourceEntity),
                                             status);
        }
        DDS::release(listener);
    }
}
