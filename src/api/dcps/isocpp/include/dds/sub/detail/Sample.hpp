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
#ifndef OSPL_DDS_SUB_DETAIL_SAMPLE_HPP_
#define OSPL_DDS_SUB_DETAIL_SAMPLE_HPP_

/**
 * @file
 * @internal
 * @bug OSPL-2430 Operator equal of 'data' is not available/implemented
 */

// Implementation
#include <dds/sub/SampleInfo.hpp>

namespace dds
{
namespace sub
{
namespace detail
{
template <typename T>
class Sample
{
public:
    Sample() : has_data_copy_(false), has_info_copy_(false) { }

    Sample(const T& d, const dds::sub::SampleInfo& i) :
        has_data_copy_(true), data_copy_(d), has_info_copy_(true), info_copy_(i)
    {
        data_ptr_ = &data_copy_;
        info_ptr_ = &info_copy_;
    }

    Sample(const Sample& other) : has_data_copy_(false), has_info_copy_(false)
    {
        copy(other);
    }

    Sample& operator=(const Sample& other)
    {
        return copy(other);
    }

    Sample& copy(const Sample& other)
    {
        if(other.has_data_copy_)
        {
            data_copy_ = other.data_copy_;
            data_ptr_ = &data_copy_;
            has_data_copy_ = true;
        }
        else
        {
            data_ptr_ = other.data_ptr_;
        }

        if(other.has_info_copy_)
        {
            info_copy_ = other.info_copy_;
            info_ptr_ = &info_copy_;
            has_info_copy_ = true;
        }
        else
        {
            info_ptr_ = other.info_ptr_;
        }

        return *this;
    }

public:
    const T& data() const
    {
        return *data_ptr_;
    }

    void data(const T& d)
    {
        data_copy_ = d;
        data_ptr_ = &data_copy_;
        has_data_copy_ = true;
    }

    void data(T* d)
    {
        data_ptr_ = d;
    }

    const dds::sub::SampleInfo& info() const
    {
        return *info_ptr_;
    }

    void info(const dds::sub::SampleInfo& i)
    {
        info_copy_ = i;
        info_ptr_ = &info_copy_;
        has_info_copy_ = true;
    }

    void info(dds::sub::SampleInfo* i)
    {
        info_ptr_ = i;
    }

    /** @internal @bug OSPL-2430 No implementation
    * @todo Implementation required - see OSPL-2430
    * @see http://jira.prismtech.com:8080/browse/OSPL-2430 */
    bool operator ==(const Sample& other) const
    {
        return false;
    }

private:
    const T* data_ptr_;
    bool has_data_copy_;
    T data_copy_;
    const dds::sub::SampleInfo* info_ptr_;
    bool has_info_copy_;
    dds::sub::SampleInfo info_copy_;
};
}
}
}

// End of implementation

#endif /* OSPL_DDS_SUB_DETAIL_SAMPLE_HPP_ */
