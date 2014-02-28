#ifndef OMG_DDS_TOPIC_DISCOVER_HPP_
#define OMG_DDS_TOPIC_DISCOVER_HPP_

/* Copyright 2010, Object Management Group, Inc.
 * Copyright 2010, PrismTech, Inc.
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

#include <dds/domain/DomainParticipant.hpp>

namespace dds
{
namespace topic
{

/**
 * This operation enables the discovery of a Topic<T>, AnyTopic,
 * ContentFilteredTopic<T>, etc. by name.
 *
 * This operation usually results in network look-ups.
 *
 * @see dds::topic::find()
 *
 * @param dp the DomainParticipant
 * @param name the topic name to discover
 * @param timeout the time out
 */
template <typename TOPIC>
TOPIC discover(const dds::domain::DomainParticipant& dp,
               const std::string& name,
               const dds::core::Duration& timeout = dds::core::Duration::infinite());

/**
 * This operation retrieves the list of Topics that have been discovered in the domain
 * and that the application has not indicated should be 'ignored' by means of the
 * dds::topic::ignore operation.
 *
 * This operation usually results in network look-ups.
 *
 * @see dds::topic::find()
 *
 * @param dp the DomainParticipant
 * @param begin a forward iterator pointing to the beginning of a container
 *        in which to insert the topics
 * @param max_size the maximum number of topics to return
 */
template <typename ANYTOPIC, typename FwdIterator>
uint32_t discover(const dds::domain::DomainParticipant& dp, FwdIterator begin, uint32_t max_size);

/**
 * This operation retrieves the list of Topics that have been discovered in the domain
 * and that the application has not indicated should be 'ignored' by means of the
 * dds::topic::ignore operation.
 *
 * This operation usually results in network look-ups.
 *
 * @see dds::topic::find()
 *
 * @param dp the DomainParticipant
 * @param begin a back inserting iterator pointing to the beginning of a container
 *        in which to insert the topics
 */
template <typename ANYTOPIC, typename BinIterator>
uint32_t discover(const dds::domain::DomainParticipant& dp, BinIterator begin);

/**
 * This operation allows an application to instruct the Service to locally ignore
 * a remote domain participant. From that point onwards the Service will locally
 * behave as if the remote participant did not exist. This means it will ignore any
 * Topic, publication, or subscription that originates on that domain participant.
 *
 * @param dp the DomainParticipant
 * @param handle the handle of the DomainParticipant to ignore
 */
void OMG_DDS_API ignore(const dds::domain::DomainParticipant& dp, const dds::core::InstanceHandle& handle);

/**
 * This operation allows an application to instruct the Service to locally ignore
 * a remote domain participant. From that point onwards the Service will locally
 * behave as if the remote participant did not exist. This means it will ignore any
 * Topic, publication, or subscription that originates on that domain participant.
 *
 * @param dp the DomainParticipant
 * @param begin a forward iterator pointing to the beginning of a sequence of
 *        InstanceHandles to ignore
 * @param end a forward iterator pointing to the end of a sequence of
 *        InstanceHandles to ignore
 */
template <typename FwdIterator>
void ignore(const dds::domain::DomainParticipant& dp, FwdIterator begin, FwdIterator end);

}
}

#endif /* OMG_DDS_TOPIC_DISCOVER_HPP_ */
