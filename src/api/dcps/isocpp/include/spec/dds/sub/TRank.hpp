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
#ifndef OMG_DDS_SUB_TRANK_HPP_
#define OMG_DDS_SUB_TRANK_HPP_

#include <dds/core/Value.hpp>

namespace dds
{
namespace sub
{
template <typename DELEGATE>
class TRank;
}
}

/**
 * The sample_rank and generation_rank available in the SampleInfo are
 * computed based solely on the actual samples in the ordered collection
 * returned by read or take.
 *
 * * The sample_rank indicates the number of samples of the same instance
 *   that follow the current one in the collection.
 *
 * * The generation_rank available in the SampleInfo indicates the difference
 *   in 'generations' between the sample (S) and the Most Recent Sample of the
 *   same instance that appears In the returned Collection (MRSIC). That is, it
 *   counts the number of times the instance transitioned from not-alive to
 *   alive in the time from the reception of the S to the reception of MRSIC.
 *
 * The generation_rank is computed using the formula:
 *
 * generation_rank =
 *      (MRSIC.disposed_generation_count + MRSIC.no_writers_generation_count)
 *      - (S.disposed_generation_count + S.no_writers_generation_count)
 *
 * The absolute_generation_rank available in the SampleInfo indicates the
 * difference in 'generations' between the sample (S) and the Most Recent
 * Sample of the same instance that the middleware has received (MRS). That is,
 * it counts the number of times the instance transitioned from not-alive to
 * alive in the time from the reception of the S to the time when the read or
 * take was called.
 *
 * absolute_generation_rank =
 *      (MRS.disposed_generation_count + MRS.no_writers_generation_count)
 *      - (S.disposed_generation_count + S.no_writers_generation_count)
 */

template <typename DELEGATE>
class dds::sub::TRank : public dds::core::Value<DELEGATE>
{
public:
    /**
     * Creates a Rank instance.
     */
    TRank();

    /**
     * Creates a Rank instance.
     */
    TRank(int32_t sample_rank, int32_t generation_rank, int32_t absolute_generation_rank);

    /**
     * Gets the absolute_generation_rank.
     *
     * @return the absolute_generation_rank
     */
    int32_t absolute_generation() const;

    /**
     * Gets the generation_rank.
     *
     * @return the generation_rank
     */
    inline int32_t generation() const;

    /**
     * Gets the sample_rank.
     *
     * @return the sample_rank
     */
    inline int32_t sample() const;
};

#endif /* OMG_DDS_SUB_TRANK_HPP_ */
