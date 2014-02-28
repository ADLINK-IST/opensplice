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
#ifndef OMG_DDS_SUB_T_GENERATION_COUNT_HPP_
#define OMG_DDS_SUB_T_GENERATION_COUNT_HPP_

#include <dds/core/Value.hpp>

namespace dds
{
namespace sub
{
template <typename DELEGATE>
class TGenerationCount;
}
}

/**
 * For each instance the middleware internally maintains two counts: the
 * disposed_generation_count and no_writers_generation_count, relative to
 * each DataReader:
 *
 * * The disposed_generation_count and no_writers_generation_count are
 *   initialized to zero when the DataReader first detects the presence of
 *   a never-seen-before instance.
 *
 * * The disposed_generation_count is incremented each time the instance_state
 *   of the corresponding instance changes from not_alive_disposed to alive.
 *
 * * The no_writers_generation_count is incremented each time the instance_state
 *   of the corresponding instance changes from not_alive_no_writers to alive.
 *
 * The disposed_generation_count and no_writers_generation_count available in
 * the SampleInfo capture a snapshot of the corresponding counters at the time
 * the sample was received.
 */
template <typename DELEGATE>
class dds::sub::TGenerationCount : public dds::core::Value<DELEGATE>
{
public:
    /**
     * Creates a new GenerationCount instance.
     */
    TGenerationCount();

    /**
     * Creates a new GenerationCount instance.
     *
     * @param disposed_generation_count the disposed_generation_count
     * @param no_writers_generation_count the no_writers_generation_count
     */
    TGenerationCount(int32_t disposed_generation_count, int32_t no_writers_generation_count);

public:
    /**
     * Gets the disposed_generation_count.
     *
     * @return the disposed_generation_count
     */
    int32_t disposed() const;

    /**
     * Gets the no_writers_generation_count.
     *
     * @return the no_writers_generation_count
     */
    inline int32_t no_writers() const;

};

#endif /*  OMG_DDS_SUB_T_GENERATION_COUNT_HPP_ */
