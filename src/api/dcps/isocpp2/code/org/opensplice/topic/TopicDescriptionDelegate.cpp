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

#include <org/opensplice/topic/TopicDescriptionDelegate.hpp>

namespace org
{
namespace opensplice
{
namespace topic
{

TopicDescriptionDelegate::TopicDescriptionDelegate(
        const dds::domain::DomainParticipant& dp,
        const std::string& name,
        const std::string& type_name)
    : myParticipant(dp),
      myTopicName(name),
      myTypeName(type_name),
      nrDependents(0)
{
    this->set_domain_id(dp->get_domain_id());
}

TopicDescriptionDelegate::~TopicDescriptionDelegate()
{
}

const std::string&
TopicDescriptionDelegate::name() const
{
    return myTopicName;
}

const std::string&
TopicDescriptionDelegate::type_name() const
{
    return myTypeName;
}

const dds::domain::DomainParticipant&
TopicDescriptionDelegate::domain_participant() const
{
    return myParticipant;
}

void
TopicDescriptionDelegate::incrNrDependents()
{
    nrDependents++;
}

void
TopicDescriptionDelegate::decrNrDependents()
{
    nrDependents--;
}

bool
TopicDescriptionDelegate::hasDependents() const
{
    return (nrDependents > 0);
}


}
}
}
