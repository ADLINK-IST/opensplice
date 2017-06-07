#ifndef OMG_DDS_SUB_TSAMPLE_HPP_
#define OMG_DDS_SUB_TSAMPLE_HPP_

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

#include <dds/core/Value.hpp>
#include <dds/sub/SampleInfo.hpp>

namespace dds
{
namespace sub
{
template <typename T, template <typename Q> class DELEGATE>
class Sample;
}
}

/**
 * This class encapsulates the data and info meta-data associated with
 * DDS samples.
 * See \ref DCPS_Modules_Subscription_SampleInfo "DataSample" for more information
 */
template <typename T, template <typename Q> class DELEGATE>
class dds::sub::Sample : public dds::core::Value< DELEGATE<T> >
{
public:
    typedef T DataType;

public:
    /**
     * Create a sample with invalid data.
     */
    Sample();

    /**
     * Creates a Sample instance.
     *
     * @param data the data
     * @param info the sample info
     */
    Sample(const T& data, const SampleInfo& info);

    /**
     * Copies a sample instance.
     *
     * @param other the sample instance to copy
     */
    Sample(const Sample& other);

    /**
     * Gets the data.
     *
     * @return the data
     */
    const DataType& data() const;

    /**
     * Sets the data.
     *
     * @param data the data
     */
    void data(const DataType& data);

    /**
     * Gets the info.
     *
     * @return the info
     */
    const SampleInfo& info() const;

    /**
     * Sets the info.
     *
     * @param info the info
     */
    void info(const SampleInfo& info);
};

#endif /* OMG_DDS_SUB_TSAMPLE_HPP_ */
