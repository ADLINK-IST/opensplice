#ifndef OMG_DDS_SUB_TCOND_READ_CONDITION_HPP_
#define OMG_DDS_SUB_TCOND_READ_CONDITION_HPP_


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

#include <dds/core/cond/Condition.hpp>
#include <dds/sub/DataReader.hpp>
#include <dds/sub/AnyDataReader.hpp>

namespace dds
{
namespace sub
{
namespace cond
{
template <typename DELEGATE>
class TReadCondition;
}
}
}

/**
 * ReadCondition objects are conditions specifically dedicated
 * to read operations and attached to one DataReader.
 *
 * ReadCondition objects allow an application to specify the data samples
 * it is interested in (by specifying the desired sample states, view states,
 * and instance states). (See the parameter definitions for DataReader's
 * read/take operations.) This allows the middleware to enable the condition
 * only when suitable information is available. They are to be used in
 * conjunction with a WaitSet as normal conditions. More than one
 * ReadCondition may be attached to the same DataReader.
 */
template <typename DELEGATE>
class dds::sub::cond::TReadCondition : public dds::core::cond::TCondition<DELEGATE>
{
public:
    OMG_DDS_REF_TYPE(TReadCondition, dds::core::cond::TCondition, DELEGATE)

public:
    /**
     * Creates a ReadCondition instance.
     *
     * @param dr the DataReader
     * @param status the data state
     */
    template <typename T>
    TReadCondition(const dds::sub::DataReader<T>& dr, const dds::sub::status::DataState& status);

    /**
     * Creates a ReadCondition instance.
     *
     * @param dr the DataReader
     * @param status the data state
     * @param functor the functor to be called when the ReadCondition is triggered
     */
    template <typename T, typename FUN>
    TReadCondition(const dds::sub::DataReader<T>& dr, const dds::sub::status::DataState& status, const FUN& functor);

    /**
     * Creates a ReadCondition instance.
     *
     * @param dr the DataReader encapsulated in an AnyDataReader
     * @param status the data state
     * @param functor the functor to be called when the ReadCondition is triggered
     */
    template <typename FUN>
    TReadCondition(const dds::sub::AnyDataReader& dr, const dds::sub::status::DataState& status, const FUN& functor);

    virtual ~TReadCondition();

public:
    /**
     * This operation returns the set of sample-states that are taken into
     * account to determine the trigger_value of the ReadCondition. These are
     * the sample-states specified when the ReadCondition was created.
     *
     * @return the data state
     */
    const dds::sub::status::DataState state_filter() const;

    /**
     * This operation returns the DataReader associated with the ReadCondition.
     * Note that there is exactly one DataReader associated with each ReadCondition.
     *
     * @return the DataReader
     */
    const dds::sub::AnyDataReader& data_reader() const;

};


#endif /* OMG_DDS_SUB_TCOND_READ_CONDITION_HPP_ */
