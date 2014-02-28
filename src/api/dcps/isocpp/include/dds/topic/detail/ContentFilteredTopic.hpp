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
#ifndef OSPL_DDS_TOPIC_DETAIL_CONTENTFILTEREDTOPIC_HPP_
#define OSPL_DDS_TOPIC_DETAIL_CONTENTFILTEREDTOPIC_HPP_

/**
 * @file
 */

// Implementation

#include <string>
#include <vector>

#include <dds/core/detail/conformance.hpp>
#include <dds/core/types.hpp>
#include <dds/topic/detail/TopicDescription.hpp>
#include <dds/topic/Topic.hpp>
#include <dds/topic/Filter.hpp>

#include <org/opensplice/core/exception_helper.hpp>

namespace dds
{
namespace topic
{
namespace detail
{

#ifdef OMG_DDS_CONTENT_SUBSCRIPTION_SUPPORT

template <typename T>
class ContentFilteredTopic  : public dds::topic::detail::TopicDescription<T>
{
public:
    ContentFilteredTopic(
        const dds::topic::Topic<T>& topic,
        const std::string& name,
        const dds::topic::Filter& filter)
        : dds::topic::detail::TopicDescription<T>(topic.domain_participant(), name, topic.type_name()),
          topic_(topic),
          filter_(filter)
    {
        DDS::StringSeq idl_filter_params;
        idl_filter_params.length(this->filter_.parameters_length());
        DDS::ULong i = 0;
        for(dds::topic::Filter::iterator params = this->filter_.begin();
                params != this->filter_.end();
                ++params)
        {
            idl_filter_params[i++] = (*params).c_str();
        }
        idl_cftopic_ = this->dp_->dp_->create_contentfilteredtopic(this->name_.c_str(),
                       this->topic_->t_,
                       this->filter_.expression().c_str(),
                       idl_filter_params);

        if(DDS::is_nil(this->idl_cftopic_.in()))
        {
            throw dds::core::NullReferenceError(
                org::opensplice::core::exception_helper(
                    OSPL_CONTEXT_LITERAL(
                        "dds::core::NullReferenceError : Unable to create ContentFilteredTopic. "
                        "Nil return from ::create_contentfilteredtopic")));
        }
    }

    virtual ~ContentFilteredTopic()
    {
        DDS::ReturnCode_t result = this->dp_->dp_->delete_contentfilteredtopic(this->idl_cftopic_.in());
        org::opensplice::core::check_and_throw(result, OSPL_CONTEXT_LITERAL("Calling ::delete_contentfilteredtopic"));
    }

public:
    /**
    *  @internal Accessor to return the topic filter.
    * @return The dds::topic::Filter in effect on this topic.
    */
    const dds::topic::Filter& filter() const
    {
        return filter_;
    }

    /**
     *  @internal Sets the filter parameters for this content filtered topic.
     * @param begin The iterator holding the first string param
     * @param end The last item in the string iteration
     */
    template <typename FWIterator>
    void filter_parameters(const FWIterator& begin, const FWIterator& end)
    {
        filter_.parameters(begin, end);
        DDS::StringSeq idl_filter_params;
        idl_filter_params.length(this->filter_.parameters_length());
        DDS::ULong i = 0;
        for(dds::topic::Filter::iterator params = begin;
                params != end;
                ++params)
        {
            idl_filter_params[i++] = (*params).c_str();
        }
        DDS::ReturnCode_t result = idl_cftopic_.in()->set_expression_parameters(idl_filter_params);
        org::opensplice::core::check_and_throw(result, OSPL_CONTEXT_LITERAL("Calling ::set_expression_parameters"));
    }

    const dds::topic::Topic<T>& topic() const
    {
        return topic_;
    }

    /**
    *  @internal The underlying contentfiltered topic. Public so we can access it in the DataReader delegate.
    */
    DDS::ContentFilteredTopic_var idl_cftopic_;

    const std::string& filter_expression() const
    {
        return filter_.expression();
    }

    const dds::core::StringSeq filter_parameters() const
    {
        return dds::core::StringSeq(filter_.begin(), filter_.end());
    }

private:
    dds::topic::Topic<T> topic_;
    dds::topic::Filter filter_;
};

#endif  // OMG_DDS_CONTENT_SUBSCRIPTION_SUPPORT

}
}
}

// End of implementation

#endif /* OSPL_DDS_TOPIC_DETAIL_CONTENTFILTEREDTOPIC_HPP_ */
