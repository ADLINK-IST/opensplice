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
#ifndef OSPL_DDS_TOPIC_FIND_HPP_
#define OSPL_DDS_TOPIC_FIND_HPP_

/**
 * @file
 */

/*
 * OMG PSM class declaration
 */
#include <spec/dds/topic/find.hpp>
#include <dds/topic/detail/find.hpp>

// Implementation

namespace dds
{
namespace topic
{

template <typename TOPIC>
TOPIC find(const dds::domain::DomainParticipant& dp, const std::string& topic_name)
{
    return ::dds::topic::detail::find<TOPIC>(dp, topic_name);
}

}
}

// End of implementation

#endif /* OSPL_DDS_TOPIC_FIND_HPP_ */
