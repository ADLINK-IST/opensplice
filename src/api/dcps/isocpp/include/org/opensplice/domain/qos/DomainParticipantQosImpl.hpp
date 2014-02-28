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

#ifndef ORG_OPENSPLICE_DOMAIN_QOS_DOMAIN_PARTICIPANT_QOS_IMPL_HPP_
#define ORG_OPENSPLICE_DOMAIN_QOS_DOMAIN_PARTICIPANT_QOS_IMPL_HPP_

#include <dds/core/policy/CorePolicy.hpp>
#include <org/opensplice/core/config.hpp>

namespace org
{
namespace opensplice
{
namespace domain
{
namespace qos
{
class OSPL_ISOCPP_IMPL_API DomainParticipantQosImpl;
}
}
}
}

class org::opensplice::domain::qos::DomainParticipantQosImpl
{
public:
    DomainParticipantQosImpl() { }

    DomainParticipantQosImpl(const ::dds::core::policy::UserData& user_data,
                             const ::dds::core::policy::EntityFactory& entity_factory)
        :  user_data_(user_data),
           entity_factory_(entity_factory)
    { }

    template <typename POLICY>
    const POLICY& policy() const;

    template <typename POLICY>
    POLICY& policy();


    void policy(const ::dds::core::policy::UserData& ud);
    void policy(const ::dds::core::policy::EntityFactory& efp);
    bool operator ==(const DomainParticipantQosImpl& other) const
    {
        return other.user_data_ == user_data_ &&
               other.entity_factory_ == entity_factory_;
    }

private:
    ::dds::core::policy::UserData user_data_;
    ::dds::core::policy::EntityFactory entity_factory_;
};

namespace org
{
namespace opensplice
{
namespace domain
{
namespace qos
{
template<>
inline const ::dds::core::policy::UserData&
DomainParticipantQosImpl::policy<dds::core::policy::UserData> () const
{
    return user_data_;
}

template<>
inline const ::dds::core::policy::EntityFactory&
DomainParticipantQosImpl::policy<dds::core::policy::EntityFactory> () const
{
    return entity_factory_;
}
}
}
}
}

#endif /* ORG_OPENSPLICE_DOMAIN_QOS_DOMAIN_PARTICIPANT_QOS_IMPL_HPP_ */
