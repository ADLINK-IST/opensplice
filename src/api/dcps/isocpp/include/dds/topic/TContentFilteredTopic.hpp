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
#ifndef OSPL_DDS_TOPIC_TCONTENTFILTEREDTOPIC_HPP_
#define OSPL_DDS_TOPIC_TCONTENTFILTEREDTOPIC_HPP_

/**
 * @file
 */

/*
 * OMG PSM class declaration
 */
#include <spec/dds/topic/TContentFilteredTopic.hpp>

// Implementation

namespace dds
{
namespace topic
{

template <typename T, template <typename Q> class DELEGATE>
ContentFilteredTopic<T, DELEGATE>::ContentFilteredTopic(const Topic<T>& topic, const std::string& name, const dds::topic::Filter& filter)
    : dds::topic::TopicDescription<T, DELEGATE>(new DELEGATE<T>(topic, name, filter)) { }

template <typename T, template <typename Q> class DELEGATE>
ContentFilteredTopic<T, DELEGATE>::~ContentFilteredTopic() { }

template <typename T, template <typename Q> class DELEGATE>
const std::string& ContentFilteredTopic<T, DELEGATE>::filter_expression() const
{
    return this->delegate()->filter_expression();
}

template <typename T, template <typename Q> class DELEGATE>
const dds::core::StringSeq ContentFilteredTopic<T, DELEGATE>::filter_parameters() const
{
    return this->delegate()->filter_parameters();
}

template <typename T, template <typename Q> class DELEGATE>
template <typename FWDIterator>
void ContentFilteredTopic<T, DELEGATE>::filter_parameters(const FWDIterator& begin, const FWDIterator& end)
{
    this->delegate()->parameters(begin, end);
}

template <typename T, template <typename Q> class DELEGATE>
const dds::topic::Topic<T>& ContentFilteredTopic<T, DELEGATE>::topic() const
{
    return this->delegate()->topic();
}

}
}

// End of implementation

#endif /* OSPL_DDS_TOPIC_TCONTENTFILTEREDTOPIC_HPP_ */
