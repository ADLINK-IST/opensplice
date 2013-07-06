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
#ifndef OSPL_DDS_PUB_FIND_HPP_
#define OSPL_DDS_PUB_FIND_HPP_

/**
 * @file
 */

/*
 * OMG PSM class declaration
 */
#include <spec/dds/pub/find.hpp>

// Implementation

namespace dds
{
namespace pub
{

template <typename WRITER, typename FwdIterator>
uint32_t
find(const dds::pub::Publisher& pub, const std::string& topic_name,
     FwdIterator begin, uint32_t max_size)
{
    return ::dds::pub::detail::find<WRITER, FwdIterator>(pub,
            topic_name,
            begin,
            max_size);
}

template <typename WRITER, typename BinIterator>
uint32_t
find(const dds::pub::Publisher& pub, const std::string& topic_name,
     BinIterator begin)
{
    return ::dds::pub::detail::find<WRITER, BinIterator>(pub,
            topic_name,
            begin);
}

}
}

// End of implementation

#endif /* OSPL_DDS_PUB_FIND_HPP_ */
