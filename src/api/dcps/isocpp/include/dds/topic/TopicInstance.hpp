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
#ifndef OSPL_DDS_TOPIC_TOPICINSTANCE_HPP_
#define OSPL_DDS_TOPIC_TOPICINSTANCE_HPP_

/**
 * @file
 */

/*
 * OMG PSM class declaration
 */
#include <spec/dds/topic/TopicInstance.hpp>

// Implementation

namespace dds
{
namespace topic
{

template <typename T>
TopicInstance<T>::TopicInstance() : h_(dds::core::null) {}

template <typename T>
TopicInstance<T>::TopicInstance(const ::dds::core::InstanceHandle& h)
    : h_(h), sample_() {}

template <typename T>
TopicInstance<T>::TopicInstance(const ::dds::core::InstanceHandle& h, const T& sample)
    : h_(h), sample_(sample) { }

template <typename T>
TopicInstance<T>::operator const ::dds::core::InstanceHandle() const
{
    return h_;
}

template <typename T>
const ::dds::core::InstanceHandle TopicInstance<T>::handle() const
{
    return h_;
}

template <typename T>
void TopicInstance<T>::handle(const ::dds::core::InstanceHandle& h)
{
    h_ = h;
}

template <typename T>
const T& TopicInstance<T>::sample() const
{
    return sample_;
}

template <typename T>
T& TopicInstance<T>::sample()
{
    return sample_;
}

template <typename T>
void TopicInstance<T>::sample(const T& sample)
{
    sample_ = sample;
}

}
}

// End of implementation

#endif /* OSPL_DDS_TOPIC_TOPICINSTANCE_HPP_ */
