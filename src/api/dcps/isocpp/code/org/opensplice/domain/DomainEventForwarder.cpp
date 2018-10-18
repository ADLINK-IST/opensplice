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

#include <dds/domain/DomainParticipantListener.hpp>
#include <org/opensplice/domain/DomainEventForwarder.hpp>

namespace org
{
namespace opensplice
{
namespace domain
{

template<>
DomainParticipantEventForwarder<dds::domain::DomainParticipant>::DomainParticipantEventForwarder(
    const dds::domain::DomainParticipant &domain,
    dds::domain::DomainParticipantListener* listener) :
    listener_(listener)
{
    domain_ = dds::core::WeakReference<dds::domain::DomainParticipant>(domain);
}

template<>
DomainParticipantEventForwarder<dds::domain::DomainParticipant>::~DomainParticipantEventForwarder()
{
}

template<>
dds::domain::DomainParticipantListener*
DomainParticipantEventForwarder<dds::domain::DomainParticipant>::listener()
{
    return listener_;
}

}
}
}
