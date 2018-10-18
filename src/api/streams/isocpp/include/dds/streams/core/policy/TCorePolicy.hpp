#ifndef OSPL_DDS_STREAMS_CORE_POLICY_TCOREPOLICY_HPP_
#define OSPL_DDS_STREAMS_CORE_POLICY_TCOREPOLICY_HPP_

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

#include <spec/dds/streams/core/policy/TCorePolicy.hpp>

namespace dds
{
namespace streams
{
namespace core
{
namespace policy
{

template <typename D>
TStreamFlush<D>::TStreamFlush(const dds::core::Duration& max_delay, uint32_t max_samples) :
    dds::core::Value<D>(max_delay, max_samples) {}

template <typename D>
TStreamFlush<D>::TStreamFlush(const TStreamFlush& other) :
    dds::core::Value<D>(other.max_delay(), other.max_samples()) {}

template <typename D>
void TStreamFlush<D>::max_delay(const dds::core::Duration& max_delay)
{
    this->delegate().max_delay(max_delay);
}

template <typename D>
const dds::core::Duration TStreamFlush<D>::max_delay() const
{
    return this->delegate().max_delay();
}

template <typename D>
void TStreamFlush<D>::max_samples(uint32_t max_samples)
{
    this->delegate().max_samples(max_samples);
}

template <typename D>
const uint32_t TStreamFlush<D>::max_samples() const
{
    return this->delegate().max_samples();
}

}
}
}
}

#endif /* OSPL_DDS_STREAMS_CORE_POLICY_TCOREPOLICY_HPP_ */
