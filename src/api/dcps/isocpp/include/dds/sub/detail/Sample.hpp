/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2012 PrismTech
 *   Limited and its licensees. All rights reserved. See file:
 *
 *                     $OSPL_HOME/LICENSE
 *
 *   for full copyright notice and license terms.
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
