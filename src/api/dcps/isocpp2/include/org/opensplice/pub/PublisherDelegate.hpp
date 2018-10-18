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
#ifndef OMG_IDDS_PUB_PUBLISHER_DELEGATE_HPP_
#define OMG_IDDS_PUB_PUBLISHER_DELEGATE_HPP_

#include <dds/core/types.hpp>
#include <dds/core/Duration.hpp>
#include <dds/core/status/State.hpp>
#include <dds/pub/qos/PublisherQos.hpp>
#include <dds/pub/qos/DataWriterQos.hpp>
#include <dds/domain/DomainParticipant.hpp>

#include <org/opensplice/ForwardDeclarations.hpp>
#include <org/opensplice/core/EntityDelegate.hpp>
#include <org/opensplice/core/EntitySet.hpp>
#include <org/opensplice/pub/AnyDataWriterDelegate.hpp>


namespace org
{
namespace opensplice
{
namespace pub
{

class OMG_DDS_API PublisherDelegate : public org::opensplice::core::EntityDelegate
{
public:
    typedef ::dds::core::smart_ptr_traits< PublisherDelegate >::ref_type ref_type;
    typedef ::dds::core::smart_ptr_traits< PublisherDelegate >::weak_ref_type weak_ref_type;

    PublisherDelegate(const dds::domain::DomainParticipant& dp,
                      const dds::pub::qos::PublisherQos& qos,
                      dds::pub::PublisherListener* listener,
                      const dds::core::status::StatusMask& event_mask);

    virtual ~PublisherDelegate();

    void init(ObjectDelegate::weak_ref_type weak_ref);

    void close();

    const dds::pub::qos::PublisherQos& qos() const;
    void qos(const dds::pub::qos::PublisherQos& pqos);

    void default_datawriter_qos(const dds::pub::qos::DataWriterQos& dwqos);
    dds::pub::qos::DataWriterQos default_datawriter_qos() const;

    void suspend_publications();
    void resume_publications();

    void begin_coherent_changes();
    void end_coherent_changes();

    void wait_for_acknowledgments(const dds::core::Duration& max_wait);

    void listener(dds::pub::PublisherListener* listener,
                  const ::dds::core::status::StatusMask& mask);
    dds::pub::PublisherListener* listener() const;

    const dds::domain::DomainParticipant& participant() const;

    bool contains_entity(const ::dds::core::InstanceHandle& handle);

    void add_datawriter(org::opensplice::core::EntityDelegate& datawriter);
    void remove_datawriter(org::opensplice::core::EntityDelegate& datawriter);
    org::opensplice::pub::AnyDataWriterDelegate::ref_type find_datawriter(const std::string& topic_name);

    dds::pub::TPublisher<PublisherDelegate>
    wrapper();

    bool is_auto_enable() const;

    virtual void
    listener_notify(ObjectDelegate::ref_type source,
                    uint32_t       triggerMask,
                    void           *eventData,
                    void           *listener);

private:
    dds::domain::DomainParticipant dp_;
    dds::pub::qos::PublisherQos qos_;
    dds::pub::qos::DataWriterQos default_dwqos_;

    org::opensplice::core::EntitySet writers;
};

}
}
}

#endif /* OMG_IDDS_PUB_PUBLISHER_DELEGATE_HPP_ */
