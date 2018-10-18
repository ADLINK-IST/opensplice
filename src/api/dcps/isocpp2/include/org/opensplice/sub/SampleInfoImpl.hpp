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

#ifndef ORG_OPENSPLICE_SUB_SAMPLE_INFO_IMPL_HPP_
#define ORG_OPENSPLICE_SUB_SAMPLE_INFO_IMPL_HPP_

#include <org/opensplice/core/config.hpp>
#include <dds/sub/Rank.hpp>
#include <dds/sub/GenerationCount.hpp>

namespace org
{
namespace opensplice
{
namespace sub
{
class SampleInfoImpl;
}
}
}


class org::opensplice::sub::SampleInfoImpl
{
public:
    SampleInfoImpl() : valid_(false) { }
public:

    inline const dds::core::Time timestamp() const
    {
        return this->source_timestamp_;
    }

    inline void timestamp(const dds::core::Time& t)
    {
        this->source_timestamp_ = t;
    }

    inline const dds::sub::status::DataState state() const
    {
        return this->state_;
    }

    inline void state(const dds::sub::status::DataState& s)
    {
        this->state_ = s;
    }

    inline dds::sub::GenerationCount generation_count() const
    {
        return this->generation_count_;
    }

    inline void generation_count(dds::sub::GenerationCount& c)
    {
        this->generation_count_ = c;
    }

    inline dds::sub::Rank rank() const
    {
        return this->rank_;
    }

    inline void rank(dds::sub::Rank& r)
    {
        this->rank_ = r;
    }

    inline bool valid() const
    {
        return this->valid_;
    }

    inline void valid(bool v)
    {
        this->valid_ = v;
    }

    inline dds::core::InstanceHandle instance_handle() const
    {
        return this->instance_handle_;
    }

    inline void instance_handle(dds::core::InstanceHandle& h)
    {
        this->instance_handle_ = h;
    }

    inline dds::core::InstanceHandle publication_handle() const
    {
        return this->publication_handle_;
    }

    inline void publication_handle(dds::core::InstanceHandle& h)
    {
        this->publication_handle_ = h;
    }

    bool operator==(const SampleInfoImpl& other) const
    {
        return this->source_timestamp_ == other.timestamp()
               && state_is_equal(this->state_, other.state())
               && this->generation_count_ == other.generation_count()
               && this->rank_ == other.rank()
               && this->valid_ == other.valid()
               && this->instance_handle_ == other.instance_handle()
               && this->publication_handle_ == other.publication_handle();
    }


private:
    static bool state_is_equal(
                    const dds::sub::status::DataState& s1,
                    const dds::sub::status::DataState& s2)
    {
        return s1.instance_state() == s1.instance_state()
               && s1.view_state() == s2.view_state()
               && s1.sample_state() == s2.sample_state();
    }

private:
    dds::core::Time source_timestamp_;
    dds::sub::status::DataState state_;
    dds::sub::GenerationCount generation_count_;
    dds::sub::Rank rank_;
    bool valid_;
    dds::core::InstanceHandle instance_handle_;
    dds::core::InstanceHandle publication_handle_;

};


#endif /* ORG_OPENSPLICE_SUB_SAMPLE_INFO_IMPL_HPP_ */
