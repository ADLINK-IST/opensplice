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

#include <org/opensplice/pub/qos/PublisherQosImpl.hpp>

namespace org { namespace opensplice { namespace pub { namespace qos {

    PublisherQosImpl::PublisherQosImpl() {}

    PublisherQosImpl::PublisherQosImpl(const dds::core::policy::Presentation& presentation,
                       const dds::core::policy::Partition& partition,
                       const dds::core::policy::GroupData& gdata,
                       const dds::core::policy::EntityFactory& factory_policy)
      : presentation_(presentation),
        partition_(partition),
        gdata_(gdata),
        factory_policy_(factory_policy) { }

    PublisherQosImpl::PublisherQosImpl(const PublisherQosImpl& other)
      : presentation_(other.presentation_),
        partition_(other.partition_),
        gdata_(other.gdata_),
        factory_policy_(other.factory_policy_) { }

    template<>
    const dds::core::policy::Presentation&
    PublisherQosImpl::policy<dds::core::policy::Presentation>() const {
      return presentation_;
    }

    template<>
    const dds::core::policy::Partition&
    PublisherQosImpl::policy<dds::core::policy::Partition>() const {
      return partition_;
    }

    template<>
    const dds::core::policy::GroupData&
    PublisherQosImpl::policy<dds::core::policy::GroupData>() const {
      return gdata_;
    }

    template<>
    const dds::core::policy::EntityFactory&
    PublisherQosImpl::policy<dds::core::policy::EntityFactory>() const {
      return factory_policy_;
    }


    void
    PublisherQosImpl::policy(const dds::core::policy::Presentation& presentation) {
      presentation_ = presentation;
    }

    void
    PublisherQosImpl::policy(const dds::core::policy::Partition& partition) {
      partition_ = partition;
    }
    void
    PublisherQosImpl::policy(const dds::core::policy::GroupData& gdata) {
      gdata_ = gdata;
    }

    void PublisherQosImpl::policy(const dds::core::policy::EntityFactory& factory_policy)
    {
      factory_policy_ = factory_policy;
    }

      }
    }
  }
}
