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
#ifndef OSPL_DDS_SUB_DETAIL_FIND_HPP_
#define OSPL_DDS_SUB_DETAIL_FIND_HPP_

/**
 * @file
 */

// Implementation
#include <string>
#include <vector>

#include <dds/sub/Subscriber.hpp>
#include <dds/sub/status/DataState.hpp>
#include <dds/topic/TopicDescription.hpp>

namespace dds
{
namespace sub
{
namespace detail
{

/** @bug OSPL-1743 No implementation
 * @todo Implementation required - see OSPL-1743
 * @see http://jira.prismtech.com:8080/browse/OSPL-1743 */
template <typename READER, typename FwdIterator>
uint32_t
find(const dds::sub::Subscriber& sub,
     const std::string& topic_name,
     FwdIterator begin, uint32_t max_size)
{
    throw dds::core::UnsupportedError(org::opensplice::core::exception_helper(
            OSPL_CONTEXT_LITERAL("dds::core::UnsupportedError : Method not yet implemented")));
}

/** @bug OSPL-1743 No implementation
 * @todo Implementation required - see OSPL-1743
 * @see http://jira.prismtech.com:8080/browse/OSPL-1743 */
template <typename READER, typename BinIterator>
uint32_t
find(const dds::sub::Subscriber& sub,
     const std::string& topic_name,
     BinIterator begin)
{
    throw dds::core::UnsupportedError(org::opensplice::core::exception_helper(
            OSPL_CONTEXT_LITERAL("dds::core::UnsupportedError : Method not yet implemented")));
}

/** @bug OSPL-1743 No implementation
 * @todo Implementation required - see OSPL-1743
 * @see http://jira.prismtech.com:8080/browse/OSPL-1743 */
template <typename READER, typename T, typename FwdIterator>
uint32_t
find(const dds::sub::Subscriber& sub,
     const dds::topic::TopicDescription<T>& topic_description,
     FwdIterator begin, uint32_t max_size)
{
    throw dds::core::UnsupportedError(org::opensplice::core::exception_helper(
            OSPL_CONTEXT_LITERAL("dds::core::UnsupportedError : Method not yet implemented")));
}

/** @bug OSPL-1743 No implementation
 * @todo Implementation required - see OSPL-1743
 * @see http://jira.prismtech.com:8080/browse/OSPL-1743 */
template <typename READER, typename T, typename BinIterator>
uint32_t
find(const dds::sub::Subscriber& sub,
     const dds::topic::TopicDescription<T>& topic_description,
     BinIterator begin)
{
    throw dds::core::UnsupportedError(org::opensplice::core::exception_helper(
            OSPL_CONTEXT_LITERAL("dds::core::UnsupportedError : Method not yet implemented")));
}

/** @bug OSPL-1743 No implementation
 * @todo Implementation required - see OSPL-1743
 * @see http://jira.prismtech.com:8080/browse/OSPL-1743 */
template <typename READER, typename FwdIterator>
uint32_t
find(const dds::sub::Subscriber& sub,
     const dds::sub::status::DataState& rs,
     FwdIterator begin, uint32_t max_size)
{
    throw dds::core::UnsupportedError(org::opensplice::core::exception_helper(
            OSPL_CONTEXT_LITERAL("dds::core::UnsupportedError : Method not yet implemented")));
}

/** @bug OSPL-1743 No implementation
 * @todo Implementation required - see OSPL-1743
 * @see http://jira.prismtech.com:8080/browse/OSPL-1743 */
template <typename READER, typename BinIterator>
uint32_t
find(const dds::sub::Subscriber& sub,
     const dds::sub::status::DataState& rs,
     BinIterator begin)
{
    throw dds::core::UnsupportedError(org::opensplice::core::exception_helper(
            OSPL_CONTEXT_LITERAL("dds::core::UnsupportedError : Method not yet implemented")));
}

}
}
}

#endif /* OSPL_DDS_SUB_DETAIL_FIND_HPP_ */
