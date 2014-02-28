#ifndef OMG_DDS_SUB_FIND_HPP_
#define OMG_DDS_SUB_FIND_HPP_

/* Copyright 2010, Object Management Group, Inc.
 * Copyright 2010, PrismTech, Corp.
 * Copyright 2010, Real-Time Innovations, Inc.
 * All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <dds/sub/Subscriber.hpp>
#include <dds/sub/status/DataState.hpp>
#include <dds/topic/TopicDescription.hpp>

namespace dds
{
namespace sub
{

/**
 * Retrieves the built-in Subscriber for the given domain participant.
 *
 * @param dp the domain participant
 * @return the built-in Subscriber
 */
const dds::sub::Subscriber OMG_DDS_API
builtin_subscriber(const dds::domain::DomainParticipant& dp);

/**
 * This function retrieves a previously-created DataReader
 * belonging to the Subscriber that is attached to a Topic with a
 * matching topic_name. If no such DataReader exists, the operation
 * will return an empty container. The use of this
 * operation on the built-in Subscriber allows access to the
 * built-in DataReader entities for the built-in topics.
 *
 * @param sub the Subscriber for which to find a DataReader
 * @param topic_name the topic name to find
 * @param begin a forward iterator pointing to the start of a
 *        container in which to put the DataReaders
 * @param max_size the number of DataReaders to return
 * @return the total number of elements returned. Notice that
 *        at most max_size will be copied using the provided iterator
 *
 */
template <typename READER, typename FwdIterator>
uint32_t
find(const dds::sub::Subscriber& sub,
     const std::string topic_name,
     FwdIterator begin, uint32_t max_size);

/**
 * This function retrieves a previously-created DataReader
 * belonging to the Subscriber that is attached to a Topic with a
 * matching topic_name. If no such DataReader exists, the operation
 * will return an empty container. The use of this
 * operation on the built-in Subscriber allows access to the
 * built-in DataReader entities for the built-in topics.
 *
 * @param sub the Subscriber for which to find a DataReader
 * @param topic_name the topic name to find
 * @param begin a back inserting iterator pointing to the start of a
 *        container in which to put the DataReaders
 * @return the total number of elements returned
 */
template <typename READER, typename BinIterator>
uint32_t
find(const dds::sub::Subscriber& sub,
     const std::string topic_name,
     BinIterator begin);

/**
 * This function retrieves a previously-created DataReader
 * belonging to the Subscriber that is attached to a Topic with a
 * matching topic_name. If no such DataReader exists, the operation
 * will return an empty container. The use of this
 * operation on the built-in Subscriber allows access to the
 * built-in DataReader entities for the built-in topics.
 *
 * @param sub the Subscriber for which to find a DataReader
 * @param topic_description the topic description to find
 * @param begin a forward iterator pointing to the start of a
 *        container in which to put the DataReaders
 * @param max_size the number of DataReaders to return
 * @return the total number of elements returned. Notice that
 *        at most max_size will be copied using the provided iterator
 */
template <typename READER, typename T, typename FwdIterator>
uint32_t
find(const dds::sub::Subscriber& sub,
     const dds::topic::TopicDescription<T>& topic_description,
     FwdIterator begin, uint32_t max_size);

/**
 * This function retrieves a previously-created DataReader
 * belonging to the Subscriber that is attached to a Topic with a
 * matching topic_name. If no such DataReader exists, the operation
 * will return an empty container. The use of this
 * operation on the built-in Subscriber allows access to the
 * built-in DataReader entities for the built-in topics.
 *
 * @param sub the Subscriber for which to find a DataReader
 * @param topic_description the topic description to find
 * @param begin a back inserting iterator pointing to the start of a
 *        container in which to put the DataReaders
 * @return the total number of elements returned
 */
template <typename READER, typename T, typename BinIterator>
uint32_t
find(const dds::sub::Subscriber& sub,
     const dds::topic::TopicDescription<T>& topic_description,
     BinIterator begin);

/**
 * This function retrieves a previously-created DataReader
 * belonging to the Subscriber that is in a specific state.
 *
 * @param sub the Subscriber for which to find a DataReader
 * @param data_state the data_state to find
 * @param begin a forward iterator pointing to the start of a
 *        container in which to put the DataReaders
 * @param max_size the number of DataReaders to return
 * @return the total number of elements returned. Notice that
 *        at most max_size will be copied using the provided iterator
 */
template <typename READER, typename FwdIterator>
uint32_t
find(const dds::sub::Subscriber& sub,
     const dds::sub::status::DataState& data_state,
     FwdIterator begin, uint32_t max_size);

/**
 * This function retrieves a previously-created DataReader
 * belonging to the Subscriber that is in a specific state.
 *
 * @param sub the Subscriber for which to find a DataReader
 * @param data_state the data_state to find
 * @param begin a back inserting iterator pointing to the start
 *        of a container in which to put the DataReaders
 * @return the total number of elements returned
 */
template <typename READER, typename BinIterator>
uint32_t
find(const dds::sub::Subscriber& sub,
     const dds::sub::status::DataState& rs,
     BinIterator begin);

}
}

#endif /* OMG_DDS_SUB_FIND_HPP_ */
