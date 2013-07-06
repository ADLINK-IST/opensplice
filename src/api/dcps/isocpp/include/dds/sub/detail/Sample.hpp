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
    Sample() { }

    Sample(const T& d, const dds::sub::SampleInfo& i)
        : data_(d), info_(i) { }

    Sample(const Sample& other) : data_(other.data()), info_(other.info_) { }

public:
    const T& data() const
    {
        return data_;
    }

    void data(const T& d)
    {
        data_ = d;
    }

    const dds::sub::SampleInfo& info() const
    {
        return info_;
    }
    void info(const dds::sub::SampleInfo& i)
    {
        info_ = i;
    }
    /** @bug OSPL-2430 No implementation
    * @todo Implementation required - see OSPL-2430
    * @see http://jira.prismtech.com:8080/browse/OSPL-2430 */
    bool operator ==(const Sample& other) const
    {
      return false;
    }

private:
    T data_;
    dds::sub::SampleInfo info_;
};
}
}
}

// End of implementation

#endif /* OSPL_DDS_SUB_DETAIL_SAMPLE_HPP_ */
