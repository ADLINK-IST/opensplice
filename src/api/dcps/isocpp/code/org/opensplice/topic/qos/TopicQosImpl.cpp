/*
*                         OpenSplice DDS
*
*   This software and documentation are Copyright 2006 to 2012 PrismTech
*   Limited and its licensees. All rights reserved. See file:
*
*                     $OSPL_HOME/LICENSE
*
*   for full copyright notice and license terms.
*
*/


/**
 * @file
 */

#include <org/opensplice/topic/qos/TopicQosImpl.hpp>

namespace org { namespace opensplice { namespace topic { namespace qos {

    TopicQosImpl::TopicQosImpl() {  }

    TopicQosImpl::TopicQosImpl(
                   const dds::core::policy::TopicData&              topic_data,
                   const dds::core::policy::Durability&             durability,

#ifdef  OMG_DDS_PERSISTENCE_SUPPORT
                   const dds::core::policy::DurabilityService&      durability_service,
#endif  // OMG_DDS_PERSISTENCE_SUPPORT

                   const dds::core::policy::Deadline&               deadline,
                   const dds::core::policy::LatencyBudget&          budget,
                   const dds::core::policy::Liveliness&             liveliness,
                   const dds::core::policy::Reliability&            reliability,
                   const dds::core::policy::DestinationOrder&       order,
                   const dds::core::policy::History&                history,
                   const dds::core::policy::ResourceLimits&         resources,
                   const dds::core::policy::TransportPriority&      priority,
                   const dds::core::policy::Lifespan&               lifespan,
                   const dds::core::policy::Ownership&              ownership)
      : topic_data_(topic_data),
        durability_(durability),

#ifdef  OMG_DDS_PERSISTENCE_SUPPORT
        durability_service_(durability_service),
#endif  // OMG_DDS_PERSISTENCE_SUPPORT

        deadline_(deadline),
        budget_(budget),
        liveliness_(liveliness),
        reliability_(reliability),
        order_(order),
        history_(history),
        resources_(resources),
        priority_(priority),
        lifespan_(lifespan),
        ownership_(ownership) {}

    TopicQosImpl::~TopicQosImpl() { }


    void TopicQosImpl::policy(const dds::core::policy::TopicData& topic_data) {
      topic_data_ = topic_data;
    }

    void TopicQosImpl::policy(const dds::core::policy::Durability& durability) {
      durability_ = durability;
    }


#ifdef  OMG_DDS_PERSISTENCE_SUPPORT

    void TopicQosImpl::policy(const dds::core::policy::DurabilityService& durability_service) {
      durability_service_ = durability_service;
    }

#endif  // OMG_DDS_PERSISTENCE_SUPPORT


    void TopicQosImpl::policy(const dds::core::policy::Deadline& deadline) {
      deadline_ = deadline;
    }

    void TopicQosImpl::policy(const dds::core::policy::LatencyBudget&  budget) {
      budget_ = budget;
    }


    void TopicQosImpl::policy(const dds::core::policy::Liveliness& liveliness) {
      liveliness_ = liveliness;
    }

    void TopicQosImpl::policy(const dds::core::policy::Reliability& reliability) {
      reliability_ = reliability;
    }
    void TopicQosImpl::policy(const dds::core::policy::DestinationOrder& order) {
      order_ = order;
    }

    void TopicQosImpl::policy(const dds::core::policy::History& history) {
      history_ = history;
    }

    void TopicQosImpl::policy(const dds::core::policy::ResourceLimits& resources) {
      resources_ = resources;
    }
    void TopicQosImpl::policy(const dds::core::policy::TransportPriority& priority) {
      priority_ = priority;
    }
    void TopicQosImpl::policy(const dds::core::policy::Lifespan& lifespan) {
      lifespan_ = lifespan;
    }
    void TopicQosImpl::policy(const dds::core::policy::Ownership& ownership) {
      ownership_ = ownership;
    }


    template<> const dds::core::policy::TopicData&
    TopicQosImpl::policy<dds::core::policy::TopicData>() const {
      return topic_data_;
    }

    template<> const dds::core::policy::Durability&
    TopicQosImpl::policy<dds::core::policy::Durability>() const {
      return durability_;
    }


#ifdef  OMG_DDS_PERSISTENCE_SUPPORT

    template<> const dds::core::policy::DurabilityService&
    TopicQosImpl::policy<dds::core::policy::DurabilityService>() const {
      return durability_service_;
    }

#endif  // OMG_DDS_PERSISTENCE_SUPPORT


    template<> const dds::core::policy::Deadline&
    TopicQosImpl::policy<dds::core::policy::Deadline>() const {
      return deadline_;
    }

    template<> const dds::core::policy::LatencyBudget&
    TopicQosImpl::policy<dds::core::policy::LatencyBudget>() const {
      return budget_;
    }

    template<> const dds::core::policy::Liveliness&
    TopicQosImpl::policy<dds::core::policy::Liveliness>() const {
      return liveliness_;
    }

    template<> const dds::core::policy::Reliability&
    TopicQosImpl::policy<dds::core::policy::Reliability>() const {
      return reliability_;
    }


    template<> const dds::core::policy::DestinationOrder&
    TopicQosImpl::policy<dds::core::policy::DestinationOrder>() const {
      return order_;
    }

    template<> const dds::core::policy::History&
    TopicQosImpl::policy<dds::core::policy::History>() const {
      return history_;
    }


    template<> const dds::core::policy::ResourceLimits&
    TopicQosImpl::policy<dds::core::policy::ResourceLimits>() const {
      return resources_;
    }


    template<> const dds::core::policy::TransportPriority&
    TopicQosImpl::policy<dds::core::policy::TransportPriority>() const {
      return priority_;
    }


    template<> const dds::core::policy::Lifespan&
    TopicQosImpl::policy<dds::core::policy::Lifespan>() const {
      return lifespan_;
    }

    template<> const  dds::core::policy::Ownership&
    TopicQosImpl::policy<dds::core::policy::Ownership>() const {
      return ownership_;
    }



      }
    }
  }
}
