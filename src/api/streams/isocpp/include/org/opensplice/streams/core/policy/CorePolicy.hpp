#ifndef ORG_OPENSPLICE_STREAMS_CORE_POLICY_CORE_POLICY_IMPL_HPP_
#define ORG_OPENSPLICE_STREAMS_CORE_POLICY_CORE_POLICY_IMPL_HPP_

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

#include <dds/core/Duration.hpp>
namespace org
{
namespace opensplice
{
namespace streams
{
namespace core
{
namespace policy
{

class StreamFlush
{
public:
    StreamFlush() : max_samples_(0) {}

    StreamFlush(const dds::core::Duration& max_delay, uint32_t max_samples) :
        max_delay_(max_delay), max_samples_(max_samples) {}

    void max_delay(const dds::core::Duration& max_delay)
    {
        max_delay_ = max_delay;
    }

    dds::core::Duration max_delay() const
    {
        return max_delay_;
    }

    dds::core::Duration& max_delay()
    {
        return max_delay_;
    }

    void max_samples(uint32_t max_samples)
    {
        max_samples_ = max_samples;
    }

    uint32_t max_samples() const
    {
        return max_samples_;
    }

    uint32_t max_samples()
    {
        return max_samples_;
    }

    bool operator==(const StreamFlush& other) const
    {
        return other.max_delay() == max_delay_ && other.max_samples() == max_samples_;
    }

private:
    dds::core::Duration max_delay_;
    uint32_t max_samples_;
};

}
}
}
}
}

#endif /* ORG_OPENSPLICE_STREAMS_CORE_POLICY_CORE_POLICY_IMPL_HPP_ */
