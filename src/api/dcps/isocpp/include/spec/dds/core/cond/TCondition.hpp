#ifndef OMG_TDDS_DDS_CORE_COND_CONDITION_HPP_
#define OMG_TDDS_DDS_CORE_COND_CONDITION_HPP_

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

#include <dds/core/Reference.hpp>


namespace dds
{
namespace core
{
namespace cond
{
template <typename DELEGATE>
class TCondition;
}
}
}
/**
 * A Condition is a root class for all the Conditions that may be attached
 * to a WaitSet. This basic class is specialized in three classes that are
 * known by the middleware: GuardCondition (Section 7.1.2.1.8),
 * StatusCondition (Section 7.1.2.1.9), and ReadCondition (Section 7.1.2.5.8).
 * For more information see \ref DCPS_Modules_Infrastructure_Waitset and
 * \ref DCPS_Modules_Infrastructure_Status
 */
template <typename DELEGATE>
class dds::core::cond::TCondition : public dds::core::Reference<DELEGATE>
{
public:
    OMG_DDS_REF_TYPE(TCondition, dds::core::Reference, DELEGATE)

public:
    virtual ~TCondition();

public:
    /**
     * Dispatches the functors that have been registered with the Condition.
     */
    virtual void dispatch();

    /**
     * This operation retrieves the trigger_value of the Condition.
     *
     * @return True if the Condition has been triggered.
     */
    virtual bool trigger_value() const;

};

#endif /* OMG_TDDS_DDS_CORE_CONDITION_HPP_ */
