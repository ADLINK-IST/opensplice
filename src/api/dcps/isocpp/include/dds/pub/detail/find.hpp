/*
*                         Vortex OpenSplice
*
 *   This software and documentation are Copyright 2006 to TO_YEAR ADLINK
 *   Technology Limited, its affiliated companies and licensors. All rights
 *   reserved.
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
