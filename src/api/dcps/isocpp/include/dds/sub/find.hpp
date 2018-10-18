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
#ifndef OSPL_DDS_SUB_FIND_HPP_
#define OSPL_DDS_SUB_FIND_HPP_

/**
 * @file
 */

/*
 * OMG PSM class declaration
 */
#include <spec/dds/sub/find.hpp>
#include <dds/sub/detail/find.hpp>

// Implementation

namespace dds
{
namespace sub
{

template <typename READER, typename FwdIterator>
uint32_t
find(const dds::sub::Subscriber& sub,
     const std::string topic_name,
     FwdIterator begin, uint32_t max_size)
{
    return ::dds::sub::detail::find<READER, FwdIterator>(sub,
            topic_name,
            begin,
            max_size);
}

template <typename READER, typename BinIterator>
uint32_t
find(const dds::sub::Subscriber& sub,
     const std::string topic_name,
     BinIterator begin)
{
    return ::dds::sub::detail::find<READER, BinIterator>(sub,
            topic_name,
            begin);
}

template <typename READER, typename T, typename FwdIterator>
uint32_t
find(const dds::sub::Subscriber& sub,
     const dds::topic::TopicDescription<T>& topic_description,
     FwdIterator begin, uint32_t max_size)
{

    OMG_DDS_STATIC_ASSERT((dds::core::is_same<T, typename READER::DataType>::value));
    return ::dds::sub::detail::find<READER, T, FwdIterator>(sub,
            topic_description,
            begin,
            max_size);
}

template <typename READER, typename T, typename BinIterator>
uint32_t
find(const dds::sub::Subscriber& sub,
     const dds::topic::TopicDescription<T>& topic_description,
     BinIterator begin)
{

    OMG_DDS_STATIC_ASSERT((dds::core::is_same<T, typename READER::DataType>::value));
    return ::dds::sub::detail::find<READER, T, BinIterator>(sub,
            topic_description,
            begin);
}


template <typename READER, typename FwdIterator>
uint32_t
find(const dds::sub::Subscriber& sub,
     const dds::sub::status::DataState& rs,
     FwdIterator begin, uint32_t max_size)
{
    return dds::sub::detail::find<READER, FwdIterator>(sub,
            rs,
            begin,
            max_size);
}

template <typename READER, typename BinIterator>
uint32_t
find(const dds::sub::Subscriber& sub,
     const dds::sub::status::DataState& rs,
     BinIterator begin)
{
    return dds::sub::detail::find<READER, BinIterator>(sub, rs, begin);
}


}
}

// End of implementation

#endif /* OSPL_DDS_SUB_FIND_HPP_ */
