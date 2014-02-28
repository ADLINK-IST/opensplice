#ifndef OMG_DDS_T_TOPIC_CONTENT_FILTERED_TOPIC_HPP_
#define OMG_DDS_T_TOPIC_CONTENT_FILTERED_TOPIC_HPP_

/* Copyright 2010, Object Management Group, Inc.
 * Copyright 2010, PrismTech, Inc.
 * Copyright 2010, Real-Time Innovations, Inc.
 * All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <vector>

#include <dds/core/detail/conformance.hpp>
#include <dds/core/types.hpp>
#include <dds/topic/Topic.hpp>
#include <dds/topic/Filter.hpp>

#ifdef OMG_DDS_CONTENT_SUBSCRIPTION_SUPPORT

namespace dds
{
namespace topic
{
template <typename T, template <typename Q> class DELEGATE>
class ContentFilteredTopic;
}
}


/**
 * ContentFilteredTopic is a specialization of TopicDescription that allows
 * for content-based subscriptions.
 *
 * ContentFilteredTopic describes a more sophisticated subscription which
 * indicates that the Subscriber does not necessarily want to see all values of each
 * instance published under the Topic. Rather, it only wants to see the values whose
 * contents satisfy certain criteria. Therefore this class must be used to request
 * content-based subscriptions.
 *
 * The selection of the content is done using the SQL based filter with parameters to
 * adapt the filter clause.
 */
template <typename T, template <typename Q> class DELEGATE>
class dds::topic::ContentFilteredTopic : public TopicDescription <T, DELEGATE>
{
public:
    OMG_DDS_REF_TYPE_T(ContentFilteredTopic, TopicDescription, T, DELEGATE)

public:
    /**
     * Creates a ContentFilteredTopic be used as to
     * perform content-based subscriptions.
     * The ContentFilteredTopic only relates to samples published
     * under that Topic, filtered according to their content. The
     * filtering is done by means of evaluating a logical expression
     * that involves the values of some of the data-fields in the sample.
     *
     * @param topic the related Topic
     * @param filter the filter expression
     *
     */
    ContentFilteredTopic(const Topic<T>& topic, const std::string& name, const dds::topic::Filter& filter);

    virtual ~ContentFilteredTopic();

public:
    /**
     * Get the filter expression.
     *
     * @return the filter expression
     */
    const std::string& filter_expression() const;

    /**
     * Get the filter expression parameters.
     *
     * @return the filter parameters as a sequence
     */
    const dds::core::StringSeq filter_parameters() const;

    /**
      * Sets the filter parameters for this content filtered topic.
      *
      * @param begin The iterator holding the first string param
      * @param end The last item in the string iteration
      */
    template <typename FWDIterator>
    void filter_parameters(const FWDIterator& begin, const FWDIterator& end);

    /**
     * Get the typed related topic.
     *
     * @return the typed Topic
     */
    const dds::topic::Topic<T>& topic() const;
};

#endif  // OMG_DDS_CONTENT_SUBSCRIPTION_SUPPORT


#endif /* OMG_DDS_T_TOPIC_CONTENT_FILTERED_TOPIC_HPP_ */
