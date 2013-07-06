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
#ifndef OSPL_DDS_TOPIC_DETAIL_MULTITOPIC_HPP_
#define OSPL_DDS_TOPIC_DETAIL_MULTITOPIC_HPP_

/**
 * @file
 */

// Implementation

#include <string>
#include <vector>

#include <dds/core/types.hpp>
#include <dds/domain/DomainParticipant.hpp>
#include <dds/topic/detail/TopicDescription.hpp>
#include <dds/sub/Query.hpp>

namespace dds
{
namespace topic
{
namespace detail
{

#ifdef OMG_DDS_MULTI_TOPIC_SUPPORT

class MultiTopic  : public dds::topic::detail::TopicDescription<T>
{
public:
    MultiTopic(const dds::domain::DomainParticipant& dp,
               const std::string& name,
               const dds::core::Query& query)
        : dds::topic::detail::TopicDescription<T>(dp, name, topic_type_name<T>::value()),
          query_(query) { }

    virtual ~MultiTopic() { }

public:
    const dds::core::Query& query()
    {
        return query_;
    }

    void expression_parameters(const dds::core::StringSeq& params)
    {
        query_.parameters(params.begin(), params.end());
    }

private:
    std::string              subscription_expression_;
    std::vector<std::string> params_;
    dds::core::Query query_;
};

#endif  // OMG_DDS_MULTI_TOPIC_SUPPORT

}
}
}
// End of implementation

#endif /* OSPL_DDS_TOPIC_DETAIL_MULTITOPIC_HPP_ */
