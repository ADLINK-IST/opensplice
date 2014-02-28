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


/**
 * @file
 */

#ifndef OSPL_DDS_TOPIC_DETAIL_FIND_HPP_
#define OSPL_DDS_TOPIC_DETAIL_FIND_HPP_

#include <string>

#include <dds/topic/Topic.hpp>
#include <org/opensplice/core/EntityRegistry.hpp>

namespace dds
{
namespace topic
{
namespace detail
{

template <typename TOPIC>
TOPIC find(const dds::domain::DomainParticipant& dp, const std::string& topic_name)
{
    (void)dp;
    std::stringstream ss;
    ss << topic_name;
    ss << dp.domain_id();
    TOPIC t =
        org::opensplice::core::EntityRegistry<std::string, TOPIC>::get(ss.str());
    if(t != dds::core::null)
    {
        return t;
    }
    return TOPIC(dds::core::null);
}

}
}
}

#endif /* OSPL_DDS_TOPIC_DETAIL_FIND_HPP_ */
