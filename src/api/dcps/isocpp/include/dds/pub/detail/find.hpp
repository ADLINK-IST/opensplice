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

#ifndef OSPL_DDS_PUB_DETAIL_FIND_HPP_
#define OSPL_DDS_PUB_DETAIL_FIND_HPP_

#include <string>

#include <dds/pub/Publisher.hpp>

namespace dds
{
namespace pub
{
namespace detail
{

/** @bug OSPL-1743 No implementation
 * @todo Implementation required - see OSPL-1743
 * @see http://jira.prismtech.com:8080/browse/OSPL-1743 */
template <typename WRITER, typename FwdIterator>
uint32_t
find(const dds::pub::Publisher& pub, const std::string& topic_name,
     FwdIterator begin, int32_t max_size)
{
    throw dds::core::UnsupportedError(org::opensplice::core::exception_helper(
            OSPL_CONTEXT_LITERAL("dds::core::UnsupportedError : Method not yet implemented")));
}

/** @bug OSPL-1743 No implementation
 * @todo Implementation required - see OSPL-1743
 * @see http://jira.prismtech.com:8080/browse/OSPL-1743 */
template <typename WRITER, typename BinIterator>
uint32_t
find(const dds::pub::Publisher& pub,
     const std::string& topic_name,
     BinIterator begin)
{
    throw dds::core::UnsupportedError(org::opensplice::core::exception_helper(
            OSPL_CONTEXT_LITERAL("dds::core::UnsupportedError : Method not yet implemented")));
}

}
}
}

#endif /* OSPL_DDS_PUB_DETAIL_FIND_HPP_ */
