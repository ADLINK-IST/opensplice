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
#ifdef OSPL_2893_COMPILER_BUG
template <typename T>
class ContentFilteredTopic <T, dds::topic::detail::ContentFilteredTopic> : public TopicDescription <T, dds::topic::detail::ContentFilteredTopic>
{
public:
    OMG_DDS_REF_TYPE_T(ContentFilteredTopic, TopicDescription, T, dds::topic::detail::ContentFilteredTopic)
#else
template <typename T, template <typename Q> class DELEGATE>
ContentFilteredTopic<T, DELEGATE>::
#endif
    ContentFilteredTopic(const Topic<T>& topic, const std::string& name, const dds::topic::Filter& filter)
        :
#ifndef OSPL_2893_COMPILER_BUG
        dds::topic::TopicDescription<T, DELEGATE>(new DELEGATE<T>
#else
        dds::topic::TopicDescription<T, dds::topic::detail::ContentFilteredTopic>(new dds::topic::detail::ContentFilteredTopic<T>
#endif
                (topic, name, filter)) { }

#ifdef OSPL_2893_COMPILER_BUG
    virtual
#else
    template <typename T, template <typename Q> class DELEGATE>
    ContentFilteredTopic<T, DELEGATE>::
#endif
    ~ContentFilteredTopic() { }

#ifndef OSPL_2893_COMPILER_BUG
    template <typename T, template <typename Q> class DELEGATE>
#endif
    const std::string&
#ifndef OSPL_2893_COMPILER_BUG
    ContentFilteredTopic<T, DELEGATE>::
#endif
    filter_expression() const
    {
        return this->delegate()->filter_expression();
    }

#ifndef OSPL_2893_COMPILER_BUG
    template <typename T, template <typename Q> class DELEGATE>
#endif
    const dds::core::StringSeq
#ifndef OSPL_2893_COMPILER_BUG
    ContentFilteredTopic<T, DELEGATE>::
#endif
    filter_parameters() const
    {
        return this->delegate()->filter_parameters();
    }

#ifndef OSPL_2893_COMPILER_BUG
    template <typename T, template <typename Q> class DELEGATE>
#endif
    template <typename FWDIterator>
    void
#ifndef OSPL_2893_COMPILER_BUG
    ContentFilteredTopic<T, DELEGATE>::
#endif
    filter_parameters(const FWDIterator& begin, const FWDIterator& end)
    {
        this->delegate()->filter_parameters(begin, end);
    }

#ifndef OSPL_2893_COMPILER_BUG
    template <typename T, template <typename Q> class DELEGATE>
#endif
    const dds::topic::Topic<T>&
#ifndef OSPL_2893_COMPILER_BUG
    ContentFilteredTopic<T, DELEGATE>::
#endif
    topic() const
    {
        return this->delegate()->topic();
    }

#ifdef OSPL_2893_COMPILER_BUG
};
#endif

}
}

// End of implementation

#endif /* OSPL_DDS_TOPIC_TCONTENTFILTEREDTOPIC_HPP_ */
