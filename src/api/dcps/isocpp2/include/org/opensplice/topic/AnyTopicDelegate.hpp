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


/**
 * @file
 */

#ifndef ORG_OPENSPLICE_TOPIC_ANYTOPICDELEGATE_HPP_
#define ORG_OPENSPLICE_TOPIC_ANYTOPICDELEGATE_HPP_


#include <dds/core/types.hpp>
#include <org/opensplice/topic/TopicTraits.hpp>
#include <org/opensplice/core/EntityDelegate.hpp>
#include <org/opensplice/topic/TopicDescriptionDelegate.hpp>
#include <org/opensplice/core/status/Status.hpp>

#include <u_topicQos.h>
namespace dds { namespace topic {
template <typename DELEGATE>
class TAnyTopic;
} }


namespace org
{
namespace opensplice
{
namespace topic
{

class OMG_DDS_API AnyTopicDelegate :
    public virtual org::opensplice::core::EntityDelegate,
    public virtual org::opensplice::topic::TopicDescriptionDelegate
{
public:
    typedef ::dds::core::smart_ptr_traits< AnyTopicDelegate >::ref_type ref_type;
    typedef ::dds::core::smart_ptr_traits< AnyTopicDelegate >::weak_ref_type weak_ref_type;

    AnyTopicDelegate(const dds::topic::qos::TopicQos& qos,
                     const dds::domain::DomainParticipant& dp,
                     const std::string& name,
                     const std::string& type_name,
                     u_topic uTopic);

    virtual ~AnyTopicDelegate();

public:
    /* DDS API mirror. */
    dds::topic::qos::TopicQos qos() const;
    void qos(const dds::topic::qos::TopicQos& qos);

    void dispose_all_data(void);

    ::dds::core::status::InconsistentTopicStatus inconsistent_topic_status() const;

    std::string reader_expression() const;

    std::vector<c_value> reader_parameters() const;

    ::org::opensplice::core::status::AllDataDisposedTopicStatus all_data_disposed_topic_status() const;

public:
    dds::topic::TAnyTopic<AnyTopicDelegate> wrapper_to_any();

    void init(ObjectDelegate::weak_ref_type weak_ref);

    void listener_notify(ObjectDelegate::ref_type,
                         uint32_t,
                         void *,
                         void *) {}

    static dds::topic::TAnyTopic<AnyTopicDelegate>
    discover_topic(const dds::domain::DomainParticipant& dp,
                   const std::string& name,
                   const dds::core::Duration& timeout);

    static void
    discover_topics(const dds::domain::DomainParticipant& dp,
                    std::vector<dds::topic::TAnyTopic<AnyTopicDelegate> >& topics,
                    uint32_t max_size);

protected:
    AnyTopicDelegate(const dds::topic::qos::TopicQos& qos,
                     const dds::domain::DomainParticipant& dp,
                     const std::string& name,
                     const std::string& type_name);



    static v_result copy_inconsistent_topic_status(c_voidp info, c_voidp arg);
    static v_result copy_all_disposed_topic_status(c_voidp info, c_voidp arg);

protected:
    dds::topic::qos::TopicQos qos_;

};

}
}
}

#endif /* ORG_OPENSPLICE_TOPIC_ANYTOPICDELEGATE_HPP_ */
