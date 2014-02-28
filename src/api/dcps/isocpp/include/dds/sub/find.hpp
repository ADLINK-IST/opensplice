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
