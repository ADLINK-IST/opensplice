#ifndef OMG_TDDS_CORE_COND_GUARD_CONDITION_HPP_
#define OMG_TDDS_CORE_COND_GUARD_CONDITION_HPP_

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

#include <dds/core/cond/TCondition.hpp>


namespace dds
{
namespace core
{
namespace cond
{
template <typename DELEGATE>
class TGuardCondition;
}
}
}

/**
 * A GuardCondition object is a specific Condition whose trigger_value is
 * completely under the control of the application.
 * When first created the trigger_value is set to FALSE.
 * The purpose of the GuardCondition is to provide the means for the
 * application to manually triggering a waitset to stop waiting. This is accomplished by
 * attaching the GuardCondition to the WaitSet and then setting the
 * trigger_value by means of the set trigger_value operation.
 */
template <typename DELEGATE>
class dds::core::cond::TGuardCondition : public TCondition<DELEGATE>
{
public:
    OMG_DDS_REF_TYPE_NODC(TGuardCondition, TCondition, DELEGATE)

public:
    TGuardCondition();

    virtual ~TGuardCondition();

public:

    /**
     * Registers a custom handler with this Condition.
     */
    template <typename Functor>
    void handler(const Functor& func);

    /**
     * Resets the handler for this Condition. After the invocation of this
     * function no handler will be registered with this Condition.
     */
    void reset_handler();

    /**
     * This operation retrieves the trigger_value of the Condition.
     *
     * @return True if the Condition has been triggered.
     */
    void trigger_value(bool value);

    /**
     * This operation retrieves the trigger_value of the Condition.
     *
     * @return True if the Condition has been triggered.
     */
    virtual bool trigger_value() const;

};

#endif /* OMG_TDDS_CORE_GUARD_CONDITION_HPP_ */
