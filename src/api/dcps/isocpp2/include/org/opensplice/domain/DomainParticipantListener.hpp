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

#ifndef ORG_OPENSPLICE_DOMAIN_DOMAINPARTICIPANT_LISTENER_HPP_
#define ORG_OPENSPLICE_DOMAIN_DOMAINPARTICIPANT_LISTENER_HPP_

#include <dds/domain/DomainParticipantListener.hpp>
#include <org/opensplice/topic/AnyTopicListener.hpp>


namespace org
{
namespace opensplice
{
namespace domain
{

class OMG_DDS_API DomainParticipantListener :
    public virtual dds::domain::DomainParticipantListener,
    public virtual org::opensplice::topic::AnyTopicListener
{
public:
    virtual ~DomainParticipantListener() { }
};


class OMG_DDS_API NoOpDomainParticipantListener :
    public virtual dds::domain::NoOpDomainParticipantListener,
    public virtual org::opensplice::topic::NoOpAnyTopicListener
{
public:
    virtual ~NoOpDomainParticipantListener()  { }
};

}
}
}

#endif /* ORG_OPENSPLICE_DOMAIN_DOMAINPARTICIPANT_LISTENER_HPP_ */
