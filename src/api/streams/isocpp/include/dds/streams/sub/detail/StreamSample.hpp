#ifndef DDS_STREAMS_SUB_DETAIL_STREAMSAMPLE_HPP_
#define DDS_STREAMS_SUB_DETAIL_STREAMSAMPLE_HPP_
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
 * @internal
 * @bug OSPL-2430 Operator equal of 'data' is not available/implemented
 */

namespace dds
{
namespace streams
{
namespace sub
{
namespace detail
{
template <typename T>
class StreamSample
{
public:
    StreamSample() : has_data_copy_(false) { }

    StreamSample(const T& d) :
    has_data_copy_(true), data_copy_(d)
    {
        data_ptr_ = &data_copy_;
    }

    StreamSample(const StreamSample& other) : has_data_copy_(false)
    {
        copy(other);
    }

    StreamSample& operator=(const StreamSample& other)
    {
        return copy(other);
    }

    StreamSample& copy(const StreamSample& other)
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

    /** @internal @bug OSPL-2430 No implementation
        * @todo Implementation required - see OSPL-2430
        * @see http://jira.prismtech.com:8080/browse/OSPL-2430 */
    bool operator ==(const StreamSample& other) const
    {
        return false;
    }

private:
    const T* data_ptr_;
    bool has_data_copy_;
    T data_copy_;
};
}
}
}
}

#endif /* DDS_STREAMS_SUB_DETAIL_STREAMSAMPLE_HPP_ */
