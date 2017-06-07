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

#include <org/opensplice/pub/qos/PublisherQosImpl.hpp>

namespace org
{
namespace opensplice
{
namespace pub
{
namespace qos
{

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

void
PublisherQosImpl::policy(const dds::core::policy::Presentation& presentation)
{
    presentation_ = presentation;
}

void
PublisherQosImpl::policy(const dds::core::policy::Partition& partition)
{
    partition_ = partition;
}
void
PublisherQosImpl::policy(const dds::core::policy::GroupData& gdata)
{
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
