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
#include <org/opensplice/core/EntityRegistry.hpp>
#include <org/opensplice/core/RegisterBuiltinTopics.hpp>

namespace dds
{
namespace pub
{
namespace detail
{

template <typename WRITER, typename FwdIterator>
uint32_t
find(const dds::pub::Publisher& pub, const std::string& topic_name,
     FwdIterator begin, int32_t max_size)
{
    DDS::DataWriter_ptr ddsdw = pub->pub_.get()->lookup_datawriter(topic_name.c_str());
    if(ddsdw)
    {
        WRITER dw = org::opensplice::core::EntityRegistry<DDS::DataWriter_ptr, WRITER >::get(ddsdw);
        if(max_size > 0 && dw != dds::core::null)
        {
            *begin = dw;
            return 1;
        }
    }
    return 0;
}


template <typename WRITER, typename BinIterator>
uint32_t
find(const dds::pub::Publisher& pub,
     const std::string& topic_name,
     BinIterator begin)
{
    DDS::DataWriter_ptr ddsdw = pub->pub_.get()->lookup_datawriter(topic_name.c_str());
    if(ddsdw)
    {
        WRITER dw = org::opensplice::core::EntityRegistry<DDS::DataWriter_ptr, WRITER >::get(ddsdw);
        if(dw != dds::core::null)
        {
            *begin = dw;
            return 1;
        }
    }
    return 0;
}

}
}
}

#endif /* OSPL_DDS_PUB_DETAIL_FIND_HPP_ */
