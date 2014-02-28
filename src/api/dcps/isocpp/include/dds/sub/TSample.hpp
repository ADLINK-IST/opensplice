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
#ifndef OSPL_DDS_SUB_TSAMPLE_HPP_
#define OSPL_DDS_SUB_TSAMPLE_HPP_

/**
 * @file
 */

/*
 * OMG PSM class declaration
 */
#include <spec/dds/sub/TSample.hpp>

// Implementation
namespace dds
{
namespace sub
{

template <typename T, template <typename Q> class DELEGATE>
Sample<T, DELEGATE>::Sample() : dds::core::Value< DELEGATE<T> >() {}

template <typename T, template <typename Q> class DELEGATE>
Sample<T, DELEGATE>::Sample(const T& data, const SampleInfo& info) : dds::core::Value< DELEGATE<T> >(data, info) { }

template <typename T, template <typename Q> class DELEGATE>
Sample<T, DELEGATE>::Sample(const Sample& other) : dds::core::Value< DELEGATE<T> >(other.delegate()) { }

template <typename T, template <typename Q> class DELEGATE>
const typename Sample<T, DELEGATE>::DataType& Sample<T, DELEGATE>::data() const
{
    return this->delegate().data();
}

template <typename T, template <typename Q> class DELEGATE>
void Sample<T, DELEGATE>::data(const DataType& d)
{
    this->delegate().data(d);
}

template <typename T, template <typename Q> class DELEGATE>
const SampleInfo& Sample<T, DELEGATE>::info() const
{
    return this->delegate().info();
}

template <typename T, template <typename Q> class DELEGATE>
void Sample<T, DELEGATE>::info(const SampleInfo& i)
{
    this->delegate().info(i);
}

}
}
// End of implementation
#endif /* OSPL_DDS_SUB_TSAMPLE_HPP_ */
