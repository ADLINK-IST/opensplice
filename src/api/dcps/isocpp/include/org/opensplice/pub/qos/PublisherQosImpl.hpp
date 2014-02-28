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

#ifndef ORG_OPENSPLICE_PUB_QOS_PUBLISHER_QOS_IMPL_HPP_
#define ORG_OPENSPLICE_PUB_QOS_PUBLISHER_QOS_IMPL_HPP_

#include <dds/core/policy/CorePolicy.hpp>
#include <org/opensplice/core/config.hpp>

namespace org
{
namespace opensplice
{
namespace pub
{
namespace qos
{
class PublisherQosImpl;
}
}
}
}

class OSPL_ISOCPP_IMPL_API org::opensplice::pub::qos::PublisherQosImpl
{
public:
    PublisherQosImpl();

    PublisherQosImpl(const PublisherQosImpl& other);

    PublisherQosImpl(const dds::core::policy::Presentation& presentation,
                     const dds::core::policy::Partition& partition,
                     const dds::core::policy::GroupData& gdata,
                     const dds::core::policy::EntityFactory& factory_policy);

    template <typename POLICY>
    const POLICY& policy() const;

    template <typename POLICY>
    POLICY& policy();

public:
    void policy(const dds::core::policy::Presentation& presentation);
    void policy(const dds::core::policy::Partition& partition);
    void policy(const dds::core::policy::GroupData& gdata);
    void policy(const dds::core::policy::EntityFactory& factory_policy);
    bool operator ==(const PublisherQosImpl& other) const
    {
        return other.presentation_ == presentation_ &&
               other.partition_ == partition_ &&
               other.gdata_ == gdata_ &&
               other.factory_policy_ == factory_policy_;
    }
private:
    dds::core::policy::Presentation          presentation_;
    dds::core::policy::Partition             partition_;
    dds::core::policy::GroupData             gdata_;
    dds::core::policy::EntityFactory   factory_policy_;
};

namespace org
{
namespace opensplice
{
namespace pub
{
namespace qos
{
template<>
inline const dds::core::policy::Presentation&
PublisherQosImpl::policy<dds::core::policy::Presentation>() const
{
    return presentation_;
}

template<>
inline const dds::core::policy::Partition&
PublisherQosImpl::policy<dds::core::policy::Partition>() const
{
    return partition_;
}

template<>
inline const dds::core::policy::GroupData&
PublisherQosImpl::policy<dds::core::policy::GroupData>() const
{
    return gdata_;
}

template<>
inline const dds::core::policy::EntityFactory&
PublisherQosImpl::policy<dds::core::policy::EntityFactory>() const
{
    return factory_policy_;
}
}
}
}
}

#endif /* ORG_OPENSPLICE_PUB_QOS_PUBLISHER_QOS_IMPL_HPP_ */
