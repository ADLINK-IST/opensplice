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

#ifndef ORG_OPENSPLICE_DOMAIN_PARTICIPANT_DELEGATE_HPP_
#define ORG_OPENSPLICE_DOMAIN_PARTICIPANT_DELEGATE_HPP_


// DDS-PSM-Cxx Includes
#include <dds/core/ref_traits.hpp>
#include <dds/core/Time.hpp>
#include <dds/core/InstanceHandle.hpp>
#include <dds/core/status/State.hpp>
#include <dds/core/detail/WeakReferenceImpl.hpp>
#include <dds/core/Entity.hpp>
#include <dds/domain/qos/DomainParticipantQos.hpp>
#include <dds/topic/qos/TopicQos.hpp>
#include <dds/pub/qos/PublisherQos.hpp>
#include <dds/sub/qos/SubscriberQos.hpp>

// OpenSplice Includes
#include <org/opensplice/ForwardDeclarations.hpp>
#include <org/opensplice/core/Mutex.hpp>
#include <org/opensplice/core/EntityDelegate.hpp>
#include <org/opensplice/core/ObjectSet.hpp>
#include <org/opensplice/core/EntitySet.hpp>
#include <org/opensplice/topic/DataRepresentation.hpp>
#include "org/opensplice/domain/Domain.hpp"



#include "u_participant.h"

class OMG_DDS_API org::opensplice::domain::DomainParticipantDelegate :
    public ::org::opensplice::core::EntityDelegate
{
public:
    typedef ::dds::core::smart_ptr_traits< DomainParticipantDelegate >::ref_type ref_type;
    typedef ::dds::core::smart_ptr_traits< DomainParticipantDelegate >::weak_ref_type weak_ref_type;

    DomainParticipantDelegate(uint32_t id,
                              const dds::domain::qos::DomainParticipantQos& qos,
                              dds::domain::DomainParticipantListener *listener,
                              const dds::core::status::StatusMask& mask);

    virtual ~DomainParticipantDelegate();

public:
    void init(ObjectDelegate::weak_ref_type weak_ref);

    void listener(dds::domain::DomainParticipantListener *listener,
                  const ::dds::core::status::StatusMask& mask);
    dds::domain::DomainParticipantListener* listener() const;

    const dds::domain::qos::DomainParticipantQos& qos() const;

    void qos(const dds::domain::qos::DomainParticipantQos& qos);

    uint32_t domain_id();

    void assert_liveliness();

    bool contains_entity(const ::dds::core::InstanceHandle& handle);

    void close();

    dds::core::Time current_time() const;

    dds::topic::qos::TopicQos default_topic_qos() const;
    void default_topic_qos(const dds::topic::qos::TopicQos& qos);

    dds::pub::qos::PublisherQos default_publisher_qos() const;
    void default_publisher_qos(const ::dds::pub::qos::PublisherQos& qos);

    dds::sub::qos::SubscriberQos default_subscriber_qos() const;
    void default_subscriber_qos(const ::dds::sub::qos::SubscriberQos& qos);

    static dds::domain::qos::DomainParticipantQos default_participant_qos();
    static void default_participant_qos(const ::dds::domain::qos::DomainParticipantQos& qos);

    void add_publisher(org::opensplice::core::EntityDelegate& publisher);
    void remove_publisher(org::opensplice::core::EntityDelegate& publisher);

    void add_subscriber(org::opensplice::core::EntityDelegate& subscriber);
    void remove_subscriber(org::opensplice::core::EntityDelegate& subscriber);

    void add_topic(org::opensplice::core::EntityDelegate& topic);
    void remove_topic(org::opensplice::core::EntityDelegate& topic);

    void add_cfTopic(org::opensplice::core::ObjectDelegate& cfTopic);
    void remove_cfTopic(org::opensplice::core::ObjectDelegate& cfTopic);

    org::opensplice::core::EntityDelegate::ref_type
    find_topic(const std::string& topic_name);

    org::opensplice::core::ObjectDelegate::ref_type
    find_cfTopic(const std::string& topic_name);

    u_topic
    lookup_topic(const std::string& topic_name,
                 const dds::core::Duration& timeout);

    void
    lookup_topics(const std::string& type_name,
                  std::vector<u_topic>& topics,
                  uint32_t max_size);

    u_participant registerType(const std::string typeName,
                               const std::string typeDescriptor,
                               org::opensplice::topic::DataRepresentationId_t dataRepresentationId,
                               const std::vector<os_uchar> typeHash,
                               const std::vector<os_uchar> metaData,
                               const std::vector<os_uchar> extentions) const;

    dds::domain::TDomainParticipant<DomainParticipantDelegate>
    wrapper();

    bool is_auto_enable() const;

    virtual void
    listener_notify(ObjectDelegate::ref_type source,
                    uint32_t       triggerMask,
                    void           *eventData,
                    void           *listener);

    org::opensplice::core::EntityDelegate::ref_type
    builtin_subscriber();

    void
    builtin_subscriber(const org::opensplice::core::EntityDelegate::ref_type subscriber);

    void
    delete_historical_data(const std::string& partition_expression,
                           const std::string& topic_expression);

    void
    create_persistent_snapshot(const std::string& partition_expression,
                               const std::string& topic_expression,
                               const std::string& uri);

    std::string
    create_child_name(const std::string& prefix);

    static void
    detach_all_domains(bool block_operations, bool delete_entities);

private:
    static dds::domain::qos::DomainParticipantQos default_participant_qos_;
    static org::opensplice::core::Mutex default_participant_qos_lock_;
    dds::domain::qos::DomainParticipantQos qos_;
    dds::topic::qos::TopicQos default_topic_qos_;
    dds::pub::qos::PublisherQos default_pub_qos_;
    dds::sub::qos::SubscriberQos default_sub_qos_;
    org::opensplice::core::EntitySet publishers;
    org::opensplice::core::EntitySet subscribers;
    org::opensplice::core::EntitySet topics;
    org::opensplice::core::ObjectSet cfTopics;
    org::opensplice::core::EntityDelegate::weak_ref_type builtin_subscriber_;
};

#endif /* ORG_OPENSPLICE_DOMAIN_PARTICIPANT_DELEGATE_HPP_ */
