#ifndef OMG_DDS_SUB_DISCOVERY_HPP_
#define OMG_DDS_SUB_DISCOVERY_HPP_

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

#include <dds/domain/DomainParticipant.hpp>
#include <dds/sub/DataReader.hpp>
#include <dds/topic/BuiltinTopic.hpp>

namespace dds
{
namespace sub
{

/**
 * Ignore subscriptions.
 *
 * @param dp      the DomainParticipant for which the remote
 *                entity will be ignored
 *
 * @param handle  the InstanceHandle of the remote entity that
 *                has to be ignored
 */
void OMG_DDS_API ignore(const dds::domain::DomainParticipant& dp,
                        const dds::core::InstanceHandle& handle);

/**
 * Ignore subscriptions.
 *
 * @param dp      the DomainParticipant for which the remote
 *                entity will be ignored
 *
 * @param handle  the InstanceHandle of the remote entity that
 *                has to be ignored
 */
template <typename FwdIterator>
void ignore(const dds::domain::DomainParticipant& dp, FwdIterator begin, FwdIterator end);


//==========================================================================
//== Discovery Management
/**
 * This operation retrieves the list of publications currently associated
 * with the DataReader; that is, publications that have a matching Topic and
 * compatible QoS that the application has not indicated should be ignored
 * by means of the dds::domain::ignore operation.
 * The handles returned in the 'publication_handles' list are the ones that
 * are used by the DDS implementation to locally identify the corresponding
 * matched DataWriter entities. These handles match the ones that appear
 * in the 'instance_handle' field of the SampleInfo when reading the
 * DCPSPublications builtin topic. The operation may fail if the
 * infrastructure does not locally maintain the connectivity information.
 */
template <typename T>
::dds::core::InstanceHandleSeq
matched_publications(const dds::sub::DataReader<T>& dr);

/**
 * This operation retrieves the list of publications currently associated
 * with the DataReader; that is, publications that have a matching Topic and
 * compatible QoS that the application has not indicated should be ignored
 * by means of the dds::domain::ignore operation.
 * The handles returned in the 'publication_handles' list are the ones that
 * are used by the DDS implementation to locally identify the corresponding
 * matched DataWriter entities. These handles match the ones that appear
 * in the 'instance_handle' field of the SampleInfo when reading the
 * DCPSPublications builtin topic. The operation may fail if the
 * infrastructure does not locally maintain the connectivity information.
 */
template <typename T, typename FwdIterator>
uint32_t
matched_publications(const dds::sub::DataReader<T>& dr,
                     FwdIterator begin, uint32_t max_size);

/**
 * This operation retrieves information on a publication that is currently
 * associated with the DataReader; that is, a subscription with a matching
 * Topic and compatible QoS that the application has not indicated should be
 * ignored by means of the DomainParticipant ignore_subscription operation.
 * The publication handle must correspond to a publication currently
 * associated with the DataReader, otherwise the operation will fail and
 * throw a BadParameterError.
 * The operation matched_subscriptions can be used to find the subscriptions
 * that are currently matched with the DataWriter.
 *
 * The operation may also fail if the infrastructure does not hold the
 * information necessary to fill in the subscription_data.
 * In this case the operation will throw UnsupportedError.
 */
template <typename T>
const dds::topic::PublicationBuiltinTopicData
matched_publications_data(const dds::sub::DataReader<T>& dr,
                          const ::dds::core::InstanceHandle& h);

}
}
#endif /* OMG_DDS_SUB_DISCOVERY_HPP_ */
