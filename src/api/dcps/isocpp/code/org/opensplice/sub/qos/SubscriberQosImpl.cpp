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

#include <org/opensplice/sub/qos/SubscriberQosImpl.hpp>

namespace org
{
namespace opensplice
{
namespace sub
{
namespace qos
{

SubscriberQosImpl::SubscriberQosImpl() {  }

SubscriberQosImpl::SubscriberQosImpl(
    dds::core::policy::Presentation presentation,
    dds::core::policy::Partition partition,
    dds::core::policy::GroupData group_data,
    dds::core::policy::EntityFactory entity_factory) :
    presentation_(presentation),
    partition_(partition),
    group_data_(group_data),
    entity_factory_(entity_factory) {  }

org::opensplice::sub::qos::SubscriberQosImpl::~SubscriberQosImpl() { }

void SubscriberQosImpl::policy(const dds::core::policy::Presentation& presentation)
{
    presentation_ = presentation;
}

void SubscriberQosImpl::policy(const dds::core::policy::Partition& partition)
{
    partition_ = partition;
}

void SubscriberQosImpl::policy(const dds::core::policy::GroupData& group_data)
{
    group_data_ = group_data;
}

void SubscriberQosImpl::policy(const dds::core::policy::EntityFactory& entity_factory)
{
    entity_factory_ = entity_factory;
}

}
}
}
}
