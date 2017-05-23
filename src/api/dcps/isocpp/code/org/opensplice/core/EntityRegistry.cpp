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

#include <org/opensplice/core/EntityRegistry.hpp>
#include <dds/domain/DomainParticipant.hpp>
#include <dds/pub/Publisher.hpp>
#include <dds/sub/Subscriber.hpp>

namespace org
{
namespace opensplice
{
namespace core
{

/* DomainParticipant */

template <>
OSPL_ISOCPP_IMPL_API std::map<DDS::DomainParticipant_ptr, dds::core::WeakReference<dds::domain::DomainParticipant> >&
org::opensplice::core::EntityRegistry<DDS::DomainParticipant_ptr, dds::domain::DomainParticipant>::registry()
{
    static std::map<DDS::DomainParticipant_ptr, dds::core::WeakReference<dds::domain::DomainParticipant> > registry_;
    return registry_;
}

/* Publisher */

template <>
OSPL_ISOCPP_IMPL_API std::map<DDS::Publisher_ptr, dds::core::WeakReference<dds::pub::Publisher> >&
org::opensplice::core::EntityRegistry<DDS::Publisher_ptr, dds::pub::Publisher>::registry()
{
    static std::map<DDS::Publisher_ptr, dds::core::WeakReference<dds::pub::Publisher> > registry_;
    return registry_;
}

/* Subscriber */

template <>
OSPL_ISOCPP_IMPL_API std::map<DDS::Subscriber_ptr, dds::core::WeakReference<dds::sub::Subscriber> >&
org::opensplice::core::EntityRegistry<DDS::Subscriber_ptr, dds::sub::Subscriber>::registry()
{
    static std::map<DDS::Subscriber_ptr, dds::core::WeakReference<dds::sub::Subscriber> > registry_;
    return registry_;
}

}
}
}