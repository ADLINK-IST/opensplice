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
#ifndef OMG_DDS_SUB_TSAMPLE_INFO_HPP_
#define OMG_DDS_SUB_TSAMPLE_INFO_HPP_

#include <dds/core/Time.hpp>
#include <dds/core/Value.hpp>
#include <dds/core/InstanceHandle.hpp>
#include <dds/sub/GenerationCount.hpp>
#include <dds/sub/Rank.hpp>
#include <dds/sub/status/DataState.hpp>

namespace dds
{
namespace sub
{
template <typename DELEGATE>
class TSampleInfo;
}
}

/**
* The SampleInfo contains information pertaining to the associated Data value
* See \ref DCPS_Modules_Subscription_SampleInfo "SampleInfo" for more information
*/
template <typename DELEGATE>
class dds::sub::TSampleInfo : public dds::core::Value<DELEGATE>
{

public:
    // Required for containers
    TSampleInfo();

public:
    /**
     * Gets the timestamp of the sample. This is the timestamp provided by the
     * DataWriter at the time the sample was produced.
     *
     * @return the timestamp
     */
    const dds::core::Time timestamp() const;

    /**
     * Gets the DataState of the sample.
     *
     * @return the DataState
     */
    const dds::sub::status::DataState state() const;

    /**
     * Gets the GenerationCount of the sample.
     *
     * @return the GenerationCount
     */
    dds::sub::GenerationCount generation_count() const;

    /**
     * Gets the Rank of the sample.
     *
     * @return the Rank
     */
    dds::sub::Rank rank() const;

    /**
     * Gets the valid_data flag. This flag indicates whether there is data
     * associated with the sample. Some samples do not contain data, indicating
     * only a change on the instance_state of the corresponding instance.
     *
     * @return the valid_data flag
     */
    bool valid() const;

    /**
     * Gets the InstanceHandle of the sample.
     *
     * @return the InstanceHandle of the sample
     */
    dds::core::InstanceHandle instance_handle() const;

    /**
     * Gets the publication_handle of the sample.
     *
     * @return the publication_handle
     */
    dds::core::InstanceHandle publication_handle() const;
};

#endif /* OMG_DDS_SUB_TSAMPLE_INFO_HPP_ */
