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

#ifndef ORG_OPENSPLICE_SUB_QOS_SUBSCRIBER_QOS_IMPL_HPP_
#define ORG_OPENSPLICE_SUB_QOS_SUBSCRIBER_QOS_IMPL_HPP_

#include <dds/core/policy/CorePolicy.hpp>
#include <org/opensplice/core/config.hpp>

namespace org
{
namespace opensplice
{
namespace sub
{
namespace qos
{

class OSPL_ISOCPP_IMPL_API SubscriberQosImpl
{
public:
    SubscriberQosImpl();

    SubscriberQosImpl(
        dds::core::policy::Presentation presentation,
        dds::core::policy::Partition partition,
        dds::core::policy::GroupData group_data,
        dds::core::policy::EntityFactory entity_factory);

    ~SubscriberQosImpl();

    void policy(const dds::core::policy::Presentation& presentation);
    void policy(const dds::core::policy::Partition& partition);
    void policy(const dds::core::policy::GroupData& grout_data);
    void policy(const dds::core::policy::EntityFactory& entity_factory);

    template <typename P> const P& policy() const;
    template <typename P> P& policy();
    bool operator ==(const SubscriberQosImpl& other) const
    {
        return other.presentation_ == presentation_ &&
               other.partition_ == partition_ &&
               other.group_data_ == group_data_ &&
               other.entity_factory_ == entity_factory_;
    }
private:
    dds::core::policy::Presentation presentation_;
    dds::core::policy::Partition partition_;
    dds::core::policy::GroupData group_data_;
    dds::core::policy::EntityFactory entity_factory_;
};

template<>
inline const ::dds::core::policy::Presentation&
org::opensplice::sub::qos::SubscriberQosImpl::policy<dds::core::policy::Presentation>() const
{
    return presentation_;
}

template<>
inline const ::dds::core::policy::Partition&
SubscriberQosImpl::policy<dds::core::policy::Partition>() const
{
    return partition_;
}

template<>
inline const ::dds::core::policy::GroupData&
SubscriberQosImpl::policy<dds::core::policy::GroupData>() const
{
    return group_data_;
}

template<>
inline const ::dds::core::policy::EntityFactory&
SubscriberQosImpl::policy<dds::core::policy::EntityFactory>() const
{
    return entity_factory_;
}

}
}
}
}

#endif /* ORG_OPENSPLICE_SUB_QOS_SUBSCRIBER_QOS_IMPL_HPP_ */
