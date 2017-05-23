#ifndef OMG_TDDS_CORE_WAIT_SET_HPP_
#define OMG_TDDS_CORE_WAIT_SET_HPP_

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

#include <vector>

#include <dds/core/types.hpp>
#include <dds/core/Duration.hpp>
#include <dds/core/cond/Condition.hpp>
#include <dds/core/cond/WaitSet.hpp>

namespace dds
{
namespace core
{
namespace cond
{
template <typename DELEGATE>
class TWaitSet;
}
}
}


/**
 * A WaitSet object allows an application to wait until one or more of
 * the attached Condition objects has a trigger_value of TRUE or else
 * until the timeout expires.
 * A WaitSet is not necessarily associated with a single DomainParticipant
 * and could be used to wait for Condition objects associated with different
 * DomainParticipant objects.
 *
 * For more information see \ref DCPS_Modules_Infrastructure_Waitset "Waitset"
 */
template <typename DELEGATE>
class dds::core::cond::TWaitSet : public dds::core::Reference<DELEGATE>
{
public:
    typedef std::vector<dds::core::cond::Condition> ConditionSeq;

public:
    OMG_DDS_REF_TYPE_PUBLIC(TWaitSet, dds::core::Reference, DELEGATE)

public:
    ~TWaitSet();

public:
    /**
     * This operation allows an application thread to wait for the occurrence
     * of certain Conditions. If none of the Conditions attached to the
     * WaitSet have a trigger_value of TRUE, the wait operation will block
     * suspending the calling thread.
     *
     * The wait operation takes a timeout argument that specifies the maximum
     * duration for the wait. If this duration is exceeded and none of
     * the attached Condition objects is true, wait will continue and the
     * returned ConditionSeq will be empty.
     *
     * It is not allowed for more than one application thread to be waiting
     * on the same WaitSet. If the wait operation is invoked on a WaitSet that
     * already has a thread blocking on it, the operation will immediately
     * raise a PreConditionNotMet exception.
     *
     * The result of the wait operation is the list of all the attached
     * Conditions that have a trigger_value of TRUE (i.e., the Conditions
     * that unblocked the wait).
     *
     * @param timeout the maximum amount of time for which the wait
     * should block while waiting for a Condition to be triggered.
     *
     * @throws PreconditionNotMetException when multiple thread try to invoke
     *        the function concurrently.
     *
     * @return a vector containing the triggered Conditions
     */
    const ConditionSeq wait(const dds::core::Duration& timeout);

    /**
     * This operation allows an application thread to wait for the occurrence
     * of certain Conditions. If none of the Conditions attached to the
     * WaitSet have a trigger_value of TRUE, the wait operation will block
     * suspending the calling thread.
     *
     * It is not allowed for more than one application thread to be waiting
     * on the same WaitSet. If the wait operation is invoked on a WaitSet that
     * already has a thread blocking on it, the operation will immediately
     * raise a PreConditionNotMet exception.
     *
     * The result of the wait operation is the list of all the attached
     * Conditions that have a trigger_value of TRUE (i.e., the Conditions
     * that unblocked the wait).
     *
     * @throws PreconditionNotMetException when multiple thread try to invoke
     *        the function concurrently.
     *
     * @return a vector containing the triggered Conditions
     */
    const ConditionSeq wait();

    /**
     * This operation allows an application thread to wait for the occurrence
     * of certain Conditions. If none of the Conditions attached to the
     * WaitSet have a trigger_value of TRUE, the wait operation will block
     * suspending the calling thread.
     *
     * The wait operation takes a timeout argument that specifies the maximum
     * duration for the wait. If this duration is exceeded and none of
     * the attached Condition objects is true, wait will continue and the
     * returned ConditionSeq will be empty.
     *
     * It is not allowed for more than one application thread to be waiting
     * on the same WaitSet. If the wait operation is invoked on a WaitSet that
     * already has a thread blocking on it, the operation will immediately
     * raise a PreConditionNotMet exception.
     *
     * The result of the wait operation is the list of all the attached
     * Conditions that have a trigger_value of TRUE (i.e., the Conditions
     * that unblocked the wait).
     *
     * @param triggered a ConditionSeq in which to put Conditions that were
     * triggered during the wait.
     *
     * @param timeout the maximum amount of time for which the wait
     * should block while waiting for a Condition to be triggered.
     *
     * @throws PreconditionNotMetException when multiple thread try to invoke
     *        the function concurrently.
     *
     * @return a vector containing the triggered Conditions
     */
    ConditionSeq& wait(ConditionSeq& triggered,
                       const dds::core::Duration& timeout);

    /**
     * This operation allows an application thread to wait for the occurrence
     * of certain Conditions. If none of the Conditions attached to the
     * WaitSet have a trigger_value of TRUE, the wait operation will block
     * suspending the calling thread.
     *
     * The wait operation takes a timeout argument that specifies the maximum
     * duration for the wait. If this duration is exceeded and none of
     * the attached Condition objects is true, wait will continue and the
     * returned ConditionSeq will be empty.
     *
     * It is not allowed for more than one application thread to be waiting
     * on the same WaitSet. If the wait operation is invoked on a WaitSet that
     * already has a thread blocking on it, the operation will immediately
     * raise a PreConditionNotMet exception.
     *
     * The result of the wait operation is the list of all the attached
     * Conditions that have a trigger_value of TRUE (i.e., the Conditions
     * that unblocked the wait).
     *
     * @param triggered a ConditionSeq in which to put Conditions that were
     * triggered during the wait.
     *
     * @throws PreconditionNotMetException when multiple thread try to invoke
     *        the function concurrently.
     *
     * @return a vector containing the triggered Conditions
     */
    ConditionSeq& wait(ConditionSeq& triggered);

public:
    /**
     * Waits for at least one of the attached Conditions to trigger and then
     * dispatches the functor associated with the Condition.
     */
    void dispatch();

    /**
     * Waits for at least one of the attached Conditions to trigger and then
     * dispatches the functor associated with the Condition, or, times
     * out and unblocks.
     */
    void dispatch(const dds::core::Duration& timeout);

public:
    /**
     * A synonym for attach_condition.
     */
    TWaitSet& operator +=(const dds::core::cond::Condition& cond);

    /**
     * A synonym for detach_condition.
     */
    TWaitSet& operator -=(const dds::core::cond::Condition& cond);

    /**
     * Attaches a Condition to the WaitSet. It is possible to attach a
     * Condition on a WaitSet that is currently being waited upon
     * (via the wait operation). In this case, if the Condition has a
     * trigger_value of TRUE, then attaching the Condition will unblock
     * the WaitSet. Adding a Condition that is already attached to the WaitSet
     * has no effect.
     *
     * @param cond the Condition to be attached to this WaitSet.
     *
     * @return the WaitSet itself so that attaching Conditions
     * can be chained.
     */
    TWaitSet& attach_condition(const dds::core::cond::Condition& cond);

    /**
     * Detaches a Condition from the WaitSet. If the Condition was not
     * attached to the WaitSet, the operation will return false.
     *
     * @param cond the Condition to detach from this WaitSet
     *
     * @return true if the Condition was found and detached, false if the
     *         Condition was not part of the WaitSet.
     */
    bool detach_condition(const dds::core::cond::Condition& cond);

    /**
     * This operation retrieves the list of attached Conditions.
     *
     * @return the list of attached Conditions.
     */
    const ConditionSeq conditions() const;

    /**
     * This operation retrieves the list of attached Conditions.
     *
     * @param conds a ConditionSeq in which to put the attached
     * Conditions.
     *
     * @return the list of attached Conditions.
     */
    ConditionSeq& conditions(ConditionSeq& conds) const;
};

#endif /* OMG_TDDS_CORE_WAIT_SET_HPP_ */
