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

#include <dds/topic/AnyTopic.hpp>
#include <org/opensplice/core/ScopedLock.hpp>
#include <org/opensplice/core/EntityDelegate.hpp>
#include <org/opensplice/topic/AnyTopicDelegate.hpp>

namespace org
{
namespace opensplice
{
namespace topic
{

/* For dynamic casting to AnyTopicDelegate to work for a few older compilers,
 * it is needed that (at least) the constructor is moved to the cpp file. */
AnyTopicDelegate::AnyTopicDelegate(
        const dds::topic::qos::TopicQos& qos,
        const dds::domain::DomainParticipant& dp,
        const std::string& name,
        const std::string& type_name)
    : org::opensplice::core::EntityDelegate(),
      org::opensplice::topic::TopicDescriptionDelegate(dp, name, type_name),
      qos_(qos)
{
}

AnyTopicDelegate::AnyTopicDelegate(
        const dds::topic::qos::TopicQos& qos,
        const dds::domain::DomainParticipant& dp,
        const std::string& name,
        const std::string& type_name,
        u_topic uTopic)
    : org::opensplice::core::EntityDelegate(),
      org::opensplice::topic::TopicDescriptionDelegate(dp, name, type_name),
      qos_(qos)
{
    this->userHandle = u_object(uTopic);
}


AnyTopicDelegate::~AnyTopicDelegate()
{
}

void
AnyTopicDelegate::init(ObjectDelegate::weak_ref_type weak_ref)
{
    /* Set weak_ref before passing ourselves to other isocpp objects. */
    this->set_weak_ref(weak_ref);
    /* Register topic at participant. */
    this->myParticipant.delegate()->add_topic(*this);

}

dds::topic::qos::TopicQos
AnyTopicDelegate::qos() const
{
    org::opensplice::core::ScopedObjectLock scopedLock(*this);
    return qos_;
}

void
AnyTopicDelegate::qos(const dds::topic::qos::TopicQos& qos)
{
    org::opensplice::core::ScopedObjectLock scopedLock(*this);
    u_result uResult;

    // get and validate the kernel qos
    org::opensplice::topic::qos::TopicQosDelegate tQos = qos.delegate();
    tQos.check();
    u_topicQos uTopicQos = tQos.u_qos();
    if (!uTopicQos) {
        ISOCPP_THROW_EXCEPTION(ISOCPP_ERROR, "Could not convert topic qos.");
    }
    uResult = u_topicSetQos((u_topic)(this->userHandle), uTopicQos);
    u_topicQosFree(uTopicQos);
    ISOCPP_U_RESULT_CHECK_AND_THROW(uResult, "Could not set topic qos.");

    qos_ = qos;
}

std::string
AnyTopicDelegate::reader_expression() const
{
    std::string rExpr;
    rExpr += "select * from ";
    rExpr += org::opensplice::topic::TopicDescriptionDelegate::myTopicName;
    return rExpr;
}

c_value *
AnyTopicDelegate::reader_parameters() const
{
    return NULL;
}

void
AnyTopicDelegate::dispose_all_data(void)
{
    u_result uResult = u_topicDisposeAllData(u_topic(this->userHandle));
    ISOCPP_U_RESULT_CHECK_AND_THROW(uResult, "Dispose all topic data failed");
}


v_result
AnyTopicDelegate::copy_inconsistent_topic_status(
    c_voidp info,
    c_voidp arg)
{
    struct v_inconsistentTopicInfo *from = (struct v_inconsistentTopicInfo *)info;
    ::dds::core::status::InconsistentTopicStatus *to = reinterpret_cast< ::dds::core::status::InconsistentTopicStatus * >(arg);

    to->delegate().v_status(*from);

    return V_RESULT_OK;
}

::dds::core::status::InconsistentTopicStatus
AnyTopicDelegate::inconsistent_topic_status() const
{
    org::opensplice::core::ScopedLock<org::opensplice::core::EntityDelegate> scopedLock(*this);

    ::dds::core::status::InconsistentTopicStatus status;
    u_result uResult = u_topicGetInconsistentTopicStatus(
                           (u_topic)(this->userHandle), TRUE, copy_inconsistent_topic_status, &status);
    ISOCPP_U_RESULT_CHECK_AND_THROW(uResult, "u_topicGetInconsistentTopicStatus failed.");

    return status;
}

v_result
AnyTopicDelegate::copy_all_disposed_topic_status(
    c_voidp info,
    c_voidp arg)
{
    struct v_allDataDisposedInfo *from = (struct v_allDataDisposedInfo *)info;
    ::org::opensplice::core::status::AllDataDisposedTopicStatus *to =
            reinterpret_cast< ::org::opensplice::core::status::AllDataDisposedTopicStatus * >(arg);

    to->delegate().v_status(*from);

    return V_RESULT_OK;
}

::org::opensplice::core::status::AllDataDisposedTopicStatus
AnyTopicDelegate::all_data_disposed_topic_status() const
{
    org::opensplice::core::ScopedLock<org::opensplice::core::EntityDelegate> scopedLock(*this);

    ::org::opensplice::core::status::AllDataDisposedTopicStatus status;
    u_result uResult = u_topicGetAllDataDisposedStatus(
                           (u_topic)(this->userHandle), TRUE, copy_all_disposed_topic_status, &status);
    ISOCPP_U_RESULT_CHECK_AND_THROW(uResult, "u_topicGetAllDataDisposedStatus failed.");

    return status;
}



dds::topic::TAnyTopic<AnyTopicDelegate>
AnyTopicDelegate::wrapper_to_any()
{
    ISOCPP_REPORT_STACK_DELEGATE_BEGIN(this);
    AnyTopicDelegate::ref_type ref =
            OSPL_CXX11_STD_MODULE::dynamic_pointer_cast<AnyTopicDelegate>(this->get_strong_ref());
    dds::topic::AnyTopic any_topic(ref);
    ISOCPP_REPORT_STACK_END();
    return any_topic;
}

dds::topic::TAnyTopic<AnyTopicDelegate>
AnyTopicDelegate::discover_topic(
        const dds::domain::DomainParticipant& dp,
        const std::string& name,
        const dds::core::Duration& timeout)
{
    u_topic uTopic = dp.delegate()->lookup_topic(name, timeout);

    if (uTopic == NULL) {
        return dds::core::null;
    }

    os_char *uTypename = u_topicTypeName(uTopic);
    std::string type_name = uTypename;
    os_free(uTypename);

    u_topicQos uQos;
    u_result uResult = u_topicGetQos(uTopic, &uQos);
    ISOCPP_U_RESULT_CHECK_AND_THROW(uResult, "Failed to get user layer topic qos");

    dds::topic::qos::TopicQos qos;
    qos.delegate().u_qos(uQos);
    u_topicQosFree(uQos);

    ref_type ref(new AnyTopicDelegate(qos, dp, name, type_name, uTopic));
    ref->init(ref);

    return dds::topic::TAnyTopic<AnyTopicDelegate>(ref);
}

void
AnyTopicDelegate::discover_topics(
        const dds::domain::DomainParticipant& dp,
        std::vector<dds::topic::TAnyTopic<AnyTopicDelegate> >& topics,
        uint32_t max_size)
{
    std::vector<u_topic> uTopics;

    dp.delegate()->lookup_topics("", uTopics, max_size);

    topics.clear();
    topics.reserve(uTopics.size());

    for (std::vector<u_topic>::const_iterator it = uTopics.begin(); it != uTopics.end(); ++it) {
        u_topic uTopic = *it;
        os_char *topic_name = u_topicName(uTopic);
        os_char *type_name = u_topicTypeName(uTopic);

        u_topicQos uQos;
        u_result uResult = u_topicGetQos(uTopic, &uQos);
        ISOCPP_U_RESULT_CHECK_AND_THROW(uResult, "Failed to get user layer topic qos");

        dds::topic::qos::TopicQos qos;
        qos.delegate().u_qos(uQos);
        u_topicQosFree(uQos);

        AnyTopicDelegate::ref_type ref(new AnyTopicDelegate(qos, dp, topic_name, type_name, uTopic));
        ref->init(ref);
        os_free(topic_name);
        os_free(type_name);

        topics.push_back(dds::topic::TAnyTopic<AnyTopicDelegate>(ref));
    }
}


}
}
}
