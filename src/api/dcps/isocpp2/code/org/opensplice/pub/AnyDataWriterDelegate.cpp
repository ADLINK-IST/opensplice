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


/**
 * @file
 */

#include <dds/pub/AnyDataWriter.hpp>
#include <org/opensplice/pub/AnyDataWriterDelegate.hpp>
#include <org/opensplice/core/ReportUtils.hpp>
#include <org/opensplice/core/ScopedLock.hpp>
#include <org/opensplice/topic/BuiltinTopicCopy.hpp>
#include <org/opensplice/core/TimeUtils.hpp>

namespace org
{
namespace opensplice
{
namespace pub
{

/* For dynamic casting to AnyDataWriterDelegate to work for a few older compilers,
 * it is needed that (at least) the constructor is moved to the cpp file. */
AnyDataWriterDelegate::AnyDataWriterDelegate(
        const dds::pub::qos::DataWriterQos& qos,
        const dds::topic::TopicDescription& td)
    : copyIn(NULL), copyOut(NULL), qos_(qos), td_(td)
{
}

AnyDataWriterDelegate::~AnyDataWriterDelegate()
{
}

void
AnyDataWriterDelegate::close()
{
    this->td_ = dds::topic::TopicDescription(dds::core::null);
    org::opensplice::core::EntityDelegate::close();
}

const dds::topic::TopicDescription&
AnyDataWriterDelegate::topic_description() const
{
    return this->td_;
}

dds::pub::qos::DataWriterQos
AnyDataWriterDelegate::qos() const
{
    org::opensplice::core::ScopedObjectLock scopedLock(*this);
    return qos_;
}


void
AnyDataWriterDelegate::qos(const dds::pub::qos::DataWriterQos& qos)
{
    org::opensplice::core::ScopedObjectLock scopedLock(*this);
    qos->check();
    u_writerQos dwQos = qos.delegate().u_qos();
    u_result uResult = u_writerSetQos((u_writer)(this->userHandle), dwQos);
    u_writerQosFree(dwQos);
    ISOCPP_U_RESULT_CHECK_AND_THROW(uResult, "Could not set writer qos.");
    this->qos_ = qos;
}


struct WriterCopyInfo {
    org::opensplice::pub::AnyDataWriterDelegate *helper;
    const void *data;
};

void
AnyDataWriterDelegate::write(
    u_writer writer,
    const void *data,
    const dds::core::InstanceHandle& handle,
    const dds::core::Time& timestamp)
{
    struct WriterCopyInfo info = {this, data};
    os_timeW t = org::opensplice::core::timeUtils::convertTime(timestamp, maxSupportedSeconds_);

    u_result uResult = u_writerWrite(
                           writer, (u_writerCopy)copy_data, (void *)&info, t, handle.delegate().handle());
    ISOCPP_U_RESULT_CHECK_AND_THROW(uResult, "u_writerWrite failed.");
}

void
AnyDataWriterDelegate::writedispose(
    u_writer writer,
    const void *data,
    const dds::core::InstanceHandle& handle,
    const dds::core::Time& timestamp)
{
    struct WriterCopyInfo info = {this, data};
    os_timeW t = org::opensplice::core::timeUtils::convertTime(timestamp, maxSupportedSeconds_);

    u_result uResult = u_writerWriteDispose(
                           writer, (u_writerCopy)copy_data, (void *)&info, t, handle.delegate().handle());
    ISOCPP_U_RESULT_CHECK_AND_THROW(uResult, "u_writerWrite failed.");
}



u_instanceHandle
AnyDataWriterDelegate::register_instance(
    u_writer writer,
    const void *data,
    const dds::core::Time& timestamp)
{
    struct WriterCopyInfo info = {this, data};
    os_timeW t = org::opensplice::core::timeUtils::convertTime(timestamp, maxSupportedSeconds_);
    u_instanceHandle handle;

    u_result uResult = u_writerRegisterInstance(
                           writer, (u_writerCopy)copy_data, (void *)&info, t, &handle);
    ISOCPP_U_RESULT_CHECK_AND_THROW(uResult, "u_writerRegisterInstance failed.");

    return handle;
}

void
AnyDataWriterDelegate::unregister_instance(
    u_writer writer,
    const dds::core::InstanceHandle& handle,
    const dds::core::Time& timestamp)
{
    os_timeW t = org::opensplice::core::timeUtils::convertTime(timestamp, maxSupportedSeconds_);
    /* copy_data() will not be called because data arg is NULL. */
    u_result uResult = u_writerUnregisterInstance(
                           writer, (u_writerCopy)copy_data, NULL, t, handle.delegate().handle());
    ISOCPP_U_RESULT_CHECK_AND_THROW(uResult, "u_writerUnregisterInstance failed.");

}

void
AnyDataWriterDelegate::unregister_instance(
    u_writer writer,
    const void *data,
    const dds::core::Time& timestamp)
{
    struct WriterCopyInfo info = {this, data};
    os_timeW t = org::opensplice::core::timeUtils::convertTime(timestamp, maxSupportedSeconds_);
    /* because handle is 0, the data is used to identify the instance. */
    u_result uResult = u_writerUnregisterInstance(
                           writer, (u_writerCopy)copy_data, (void *)&info, t, 0);
    ISOCPP_U_RESULT_CHECK_AND_THROW(uResult, "u_writerUnregisterInstance failed.");

}

void
AnyDataWriterDelegate::dispose_instance(
    u_writer writer,
    const dds::core::InstanceHandle& handle,
    const dds::core::Time& timestamp)
{
    os_timeW t = org::opensplice::core::timeUtils::convertTime(timestamp, maxSupportedSeconds_);
    /* copy_data() will not be called because data arg is NULL. */
    u_result uResult = u_writerDispose(
                           writer, (u_writerCopy)copy_data, NULL, t, handle.delegate().handle());
    ISOCPP_U_RESULT_CHECK_AND_THROW(uResult, "u_writerDispose failed.");
}

void
AnyDataWriterDelegate::dispose_instance(
    u_writer writer,
    const void *data,
    const dds::core::Time& timestamp)
{
    os_timeW t = org::opensplice::core::timeUtils::convertTime(timestamp, maxSupportedSeconds_);
    struct WriterCopyInfo info = {this, data};
    /* because handle is 0, the data is used to identify the instance. */
    u_result uResult = u_writerDispose(
                       writer, (u_writerCopy)copy_data, (void *)&info, t, 0);
    ISOCPP_U_RESULT_CHECK_AND_THROW(uResult, "u_writerDispose failed.");
}

void
AnyDataWriterDelegate::get_key_value(
    u_writer writer,
    void *data,
    const dds::core::InstanceHandle& handle)
{
    u_result uResult = u_writerCopyKeysFromInstanceHandle(
                           writer, handle.delegate().handle(), (u_writerCopyKeyAction)this->copyOut, data);
    ISOCPP_U_RESULT_CHECK_AND_THROW(uResult, "u_writerCopyKeysFromInstanceHandle failed.");

}

u_instanceHandle
AnyDataWriterDelegate::lookup_instance(
    u_writer writer,
    const void *data)
{
    struct WriterCopyInfo info = {this, data};
    u_instanceHandle handle;

    u_result uResult = u_writerLookupInstance(
                           writer, (u_writerCopy)copy_data, (void *)&info, &handle);
    ISOCPP_U_RESULT_CHECK_AND_THROW(uResult, "u_writerLookupInstance failed.");

    return handle;
}

static v_result
copy_liveliness_lost_status(
    c_voidp info,
    c_voidp arg)
{
    struct v_livelinessLostInfo *from = (struct v_livelinessLostInfo *)info;
    ::dds::core::status::LivelinessLostStatus *to = static_cast< ::dds::core::status::LivelinessLostStatus * >(arg);
    to->delegate().v_status(*from);
    return V_RESULT_OK;
}

const ::dds::core::status::LivelinessLostStatus
AnyDataWriterDelegate::liveliness_lost_status()
{
    this->check();
    dds::core::status::LivelinessLostStatus status;
    u_result uResult = u_writerGetLivelinessLostStatus(
                                (u_writer)(this->userHandle),
                                true,
                                copy_liveliness_lost_status,
                                &status);
    ISOCPP_U_RESULT_CHECK_AND_THROW(uResult, "u_writerGetLivelinessLostStatus failed.");
    return status;
}

static v_result
copy_offered_deadline_missed_status(
    c_voidp info,
    c_voidp arg)
{
    struct v_deadlineMissedInfo *from = (struct v_deadlineMissedInfo *)info;
    ::dds::core::status::OfferedDeadlineMissedStatus *to = static_cast< ::dds::core::status::OfferedDeadlineMissedStatus * >(arg);
    to->delegate().v_status(*from);
    return V_RESULT_OK;
}

const ::dds::core::status::OfferedDeadlineMissedStatus
AnyDataWriterDelegate::offered_deadline_missed_status()
{
    dds::core::status::OfferedDeadlineMissedStatus status;
    u_result uResult = u_writerGetDeadlineMissedStatus(
                                (u_writer)(this->userHandle),
                                true,
                                copy_offered_deadline_missed_status,
                                &status);
    ISOCPP_U_RESULT_CHECK_AND_THROW(uResult, "u_writerGetDeadlineMissedStatus failed.");
    return status;
}


static v_result
copy_incompatible_qos_status(
    c_voidp info,
    c_voidp arg)
{
    struct v_incompatibleQosInfo *from = (struct v_incompatibleQosInfo *)info;
    ::dds::core::status::OfferedIncompatibleQosStatus *to = static_cast< ::dds::core::status::OfferedIncompatibleQosStatus * >(arg);
    to->delegate().v_status(*from);
    return V_RESULT_OK;
}

const ::dds::core::status::OfferedIncompatibleQosStatus
AnyDataWriterDelegate::offered_incompatible_qos_status()
{
    dds::core::status::OfferedIncompatibleQosStatus status;
    u_result uResult = u_writerGetIncompatibleQosStatus(
                                (u_writer)(this->userHandle),
                                true,
                                copy_incompatible_qos_status,
                                &status);
    ISOCPP_U_RESULT_CHECK_AND_THROW(uResult, "u_writerGetIncompatibleQosStatus failed.");
    return status;
}

static v_result
copy_publication_matched_status(
    c_voidp info,
    c_voidp arg)
{
    struct v_topicMatchInfo *from= (struct v_topicMatchInfo *)info;
    ::dds::core::status::PublicationMatchedStatus *to = static_cast< ::dds::core::status::PublicationMatchedStatus * >(arg);;
    to->delegate().v_status(*from);
    return V_RESULT_OK;
}

const ::dds::core::status::PublicationMatchedStatus
AnyDataWriterDelegate::publication_matched_status()
{
    dds::core::status::PublicationMatchedStatus status;
    u_result uResult = u_writerGetPublicationMatchStatus(
                                (u_writer)(this->userHandle),
                                true,
                                copy_publication_matched_status,
                                &status);
    ISOCPP_U_RESULT_CHECK_AND_THROW(uResult, "u_writerGetPublicationMatchStatus failed.");
    return status;
}

static v_result
copy_matched_subscription(
    u_subscriptionInfo *info,
    void *arg)
{
    dds::core::InstanceHandleSeq *seq  = reinterpret_cast<dds::core::InstanceHandleSeq *>(arg);
    ::dds::core::InstanceHandle handle = u_instanceHandleFromGID(info->key);
    seq->push_back(handle);
    return V_RESULT_OK;
}

::dds::core::InstanceHandleSeq
AnyDataWriterDelegate::matched_subscriptions()
{
    ::dds::core::InstanceHandleSeq handleSeq;
    u_result uResult = u_writerGetMatchedSubscriptions(
                                (u_writer)(this->userHandle),
                                copy_matched_subscription,
                                &handleSeq);
    ISOCPP_U_RESULT_CHECK_AND_THROW(uResult, "u_writerGetMatchedSubscriptions failed.");
    return handleSeq;
}

static v_result
copy_matched_subscription_data(
    u_subscriptionInfo *info,
    void *arg)
{
    dds::topic::SubscriptionBuiltinTopicData *to = reinterpret_cast<dds::topic::SubscriptionBuiltinTopicData *>(arg);
    __SubscriptionBuiltinTopicData__copyOut(info, to);
    return V_RESULT_OK;
}

const dds::topic::SubscriptionBuiltinTopicData
AnyDataWriterDelegate::matched_subscription_data(const ::dds::core::InstanceHandle& h)
{
    dds::topic::SubscriptionBuiltinTopicData dataSample;
    u_result uResult = u_writerGetMatchedSubscriptionData(
                                (u_writer)(this->userHandle),
                                h->handle(),
                                copy_matched_subscription_data,
                                &dataSample);
    ISOCPP_U_RESULT_CHECK_AND_THROW(uResult, "u_writerGetMatchedSubscriptionData failed.");
    return dataSample;
}

void
AnyDataWriterDelegate::assert_liveliness()
{
    u_result uResult = u_writerAssertLiveliness((u_writer)(this->userHandle));
    ISOCPP_U_RESULT_CHECK_AND_THROW(uResult, "u_writerAssertLiveliness failed.");
}

void
AnyDataWriterDelegate::wait_for_acknowledgments(
    const dds::core::Duration& timeout)
{
    os_duration d = org::opensplice::core::timeUtils::convertDuration(timeout);
    u_result uResult = u_writerWaitForAcknowledgments((u_writer)(this->userHandle), d);
    ISOCPP_U_RESULT_CHECK_AND_THROW(uResult, "u_writerWaitForAcknowledgments failed.");
}

v_copyin_result
AnyDataWriterDelegate::copy_data(c_type t, void *data, void *to)
{
    v_copyin_result result;
    c_base base = c_getBase(c_object(t));
    struct WriterCopyInfo *info = (struct WriterCopyInfo *)data;

    result = info->helper->getCopyIn()(base, info->data, to);

    return result;
}

dds::pub::TAnyDataWriter<AnyDataWriterDelegate>
AnyDataWriterDelegate::wrapper_to_any()
{
    ISOCPP_REPORT_STACK_DELEGATE_BEGIN(this);
    AnyDataWriterDelegate::ref_type ref =
            OSPL_CXX11_STD_MODULE::dynamic_pointer_cast<AnyDataWriterDelegate>(this->get_strong_ref());
    dds::pub::AnyDataWriter any_writer(ref);
    ISOCPP_REPORT_STACK_END();
    return any_writer;
}



}
}
}
