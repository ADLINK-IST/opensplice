#ifndef OMG_TDDS_DOMAIN_DOMAIN_PARTICIPANT_HPP_
#define OMG_TDDS_DOMAIN_DOMAIN_PARTICIPANT_HPP_

/* Copyright 2010, Object Management Group, Inc.
 * Copyright 2010, PrismTech, Corp.
 * Copyright 2010, Real-Time Innovations, Inc.
 * All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <string>

#include <dds/core/detail/conformance.hpp>
#include <dds/core/types.hpp>
#include <dds/core/Time.hpp>
#include <dds/core/TEntity.hpp>
#include <dds/core/cond/StatusCondition.hpp>
#include <dds/domain/qos/DomainParticipantQos.hpp>

#include <dds/topic/qos/TopicQos.hpp>

#include <dds/pub/qos/PublisherQos.hpp>
#include <dds/sub/qos/SubscriberQos.hpp>


namespace dds
{
namespace domain
{

template <typename DELEGATE>
class TDomainParticipant;

class DomainParticipantListener;
}
}

/**
 * The DomainParticipant represents the participation of the application on
 * a communication plane that isolates applications running on the same
 * set of physical computers from each other. A domain establishes a virtual
 * network linking all applications that share the same domainId and isolating
 * them from applications running on different domains. In this way, several
 * independent distributed applications can coexist in the same physical
 * network without interfering, or even being aware of each other.
 * See \ref DCPS_Modules_DomainParticipant "Domain Participant" for more information
 */
template <typename DELEGATE>
class dds::domain::TDomainParticipant : public ::dds::core::TEntity<DELEGATE>
{
public:
    typedef dds::domain::DomainParticipantListener Listener;

public:
    /**
     * Creates a new DomainParticipant object. The DomainParticipant signifies
     * that the calling application intends to join the Domain identified by
     * the domain_id argument.
     *
     * The DomainParticipant will be created with the DomainParticipantQos
     * passed as anargument.
     *
     * @param id the id of the domain joined by the new DomainParticipant
     */
    TDomainParticipant(uint32_t id);

    /**
     * Creates a new DomainParticipant object. The DomainParticipant signifies
     * that the calling application intends to join the Domain identified by
     * the domain_id argument.
     *
     * The DomainParticipant will be created with the DomainParticipantQos
     * passed as anargument.
     *
     * @param id the id of the domain joined by the new DomainParticipant
     * @param qos the QoS settings for the new DomainParticipant
     * @param listener the listener
     * @param event_mask the mask defining the events for which the listener
     *  will be notified.
     */
    TDomainParticipant(uint32_t                                          id,
                       const dds::domain::qos::DomainParticipantQos&   qos,
                       dds::domain::DomainParticipantListener*         listener = NULL,
                       const dds::core::status::StatusMask&            event_mask = dds::core::status::StatusMask::none());

public:
    OMG_DDS_BASIC_REF_TYPE(TDomainParticipant, ::dds::core::TEntity, DELEGATE)

public:
    virtual ~TDomainParticipant();

public:

    /**
     * Register a listener with the DomainParticipant.
     * The notifications received by the listener depend on the
     * status mask with which it was registered.
     *
     * @param listener the listener
     * @param event_mask the mask defining the events for which the listener
     *  will be notified.
     */
    void listener(Listener* listener,
                  const ::dds::core::status::StatusMask& event_mask);

    /**
     * Get the listener of this DomainParticipant.
     *
     * @return the listener
     */
    Listener* listener() const;

    /**
     * Gets the DomainParticipantQos setting for this instance.
     *
     * @return the qos
     */
    const dds::domain::qos::DomainParticipantQos& qos() const;

    /**
     * Sets the DomainParticipantQos setting for this instance.
     *
     * @param qos the qos
     */
    void qos(const dds::domain::qos::DomainParticipantQos& qos);

    /**
     * This operation retrieves the domain_id used to create the
     * DomainParticipant. The domain_id identifies the DDS domain
     * to which the DomainParticipant belongs.
     *
     * As described in the introduction to Section 7.1.2.2.1,
     * DomainParticipant Class, on page 22, each DDS domain represents
     * a separate data communication plane isolated from other domains.
     *
     * @return the domain id
     */
    uint32_t domain_id() const;


    /**
     * This operation manually asserts the liveliness of the DataWriter.
     * This is used in combination with the Liveliness QoS policy
     * (see Section 7.1.3, Supported QoS, on page 96) to indicate to the
     * Service that the entity remains active.
     * This operation need only be used if the LIVELINESS setting is either
     * MANUAL_BY_PARTICIPANT or MANUAL_BY_TOPIC. Otherwise, it has no effect.
     *
     * Note: Writing data via the write operation on a DataWriter
     * asserts liveliness on the DataWriter itself and its DomainParticipant.
     * Consequently the use of assert_liveliness is only needed if the
     * application is not writing data regularly.
     */
    void assert_liveliness();


    /**
     * This operation checks whether or not the given a_handle represents
     * an Entity that was created from the DomainParticipant.
     * The containment applies recursively. That is, it applies both to
     * entities (TopicDescription, Publisher, or Subscriber) created directly
     * using the DomainParticipant as well as entities created using a
     * contained Publisher, or Subscriber as the factory, and so forth.
     *
     * @param handle the instance handle for which the containement
     *               relationship has to be checked
     *
     * @return true if the handle belongs to an Entity belonging
     *              to this DomainParticipant
     */
    bool contains_entity(const ::dds::core::InstanceHandle& handle);

    /**
     * This operation returns the current value of the time that the service
     * uses to time-stamp data writes and to set the reception timestamp
     * for the data updates it receives.
     *
     * @return the current time
     */
    dds::core::Time current_time();

    /**
     * Set the QoS associated with this DomainParticipant.
     * @param qos the new DomainParticipant QoS
     */
    dds::domain::qos::DomainParticipantQos& operator << (const dds::domain::qos::DomainParticipantQos& qos);

    /**
     * Get the QoS associated with this DomainParticipant.
     *
     * @param qos the current DomainParticipant QoS
     */
    const TDomainParticipant& operator >> (dds::domain::qos::DomainParticipantQos& qos) const;

public:
    /**
     * Gets the default DomainParticipantQos.
     *
     * @return the default DomainParticipantQos
     */
    static dds::domain::qos::DomainParticipantQos default_participant_qos();

    /**
     * Sets the default DomainParticipantQos.
     *
     * @param qos the default DomainParticipantQos
     */
    static void default_participant_qos(const ::dds::domain::qos::DomainParticipantQos& qos);

    /**
     * Gets the default PublisherQos.
     *
     * @return the default PublisherQos
     */
    dds::pub::qos::PublisherQos default_publisher_qos() const;

    /**
     * Sets the default PublisherQos.
     *
     * @param qos the default PublisherQos
     */
    TDomainParticipant& default_publisher_qos(const ::dds::pub::qos::PublisherQos& qos);

    /**
     * Gets the default SubscriberQos.
     *
     * @return the default SubscriberQos
     */
    dds::sub::qos::SubscriberQos default_subscriber_qos() const;

    /**
     * Sets the default SubscriberQos.
     *
     * @param qos the default SubscriberQos
     */
    TDomainParticipant& default_subscriber_qos(const ::dds::sub::qos::SubscriberQos& qos);

    /**
     * Gets the default TopicQos.
     *
     * @return the default TopicQos
     */
    dds::topic::qos::TopicQos default_topic_qos() const;

    /**
     * Sets the default TopicQos.
     *
     * @param qos the default TopicQos
     */
    TDomainParticipant& default_topic_qos(const dds::topic::qos::TopicQos& qos);

    //=============================================================================
};


#endif /* OMG_TDDS_DOMAIN_DOMAIN_PARTICIPANT_HPP_ */
