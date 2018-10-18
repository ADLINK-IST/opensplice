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
