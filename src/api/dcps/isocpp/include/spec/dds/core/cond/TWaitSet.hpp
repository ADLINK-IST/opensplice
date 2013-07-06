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

namespace dds {
  namespace core {
    namespace cond {
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
 * and could be used to wait on Condition objects associated with different
 * DomainParticipant objects.
 */
template <typename DELEGATE>
class dds::core::cond::TWaitSet : public dds::core::Reference<DELEGATE> {
public:
  typedef std::vector<dds::core::cond::Condition> ConditionSeq;

public:
  OMG_DDS_REF_TYPE_PUBLIC(TWaitSet, dds::core::Reference, DELEGATE)

public:
  ~TWaitSet();

public:
  /**
   * This operation allows an application thread to wait for the occurrence
   * of certain conditions. If none of the conditions attached to the
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
   * already has a thread blocking on it, the operation will raise
   * immediately an exception PreconditionNotMet.
   *
   * The result of the wait operation is the list of all the attached
   * conditions that have a trigger_value of TRUE (i.e., the conditions
   * that unblocked the wait).
   *
   * @param timeout the maximum amount of time for which the wait
   * should block while waiting for a condition to be triggered.
   *
   * @throws PreconditionNotMetException when multiple thread try to invoke
   *        the method concurrently.
   *
   * @return a vector containing the triggered conditions
   */
  const ConditionSeq wait(const dds::core::Duration& timeout);

  /**
   * This operation allows an application thread to wait for the occurrence
   * of certain conditions. If none of the conditions attached to the
   * WaitSet have a trigger_value of TRUE, the wait operation will block
   * suspending the calling thread.
   *
   * It is not allowed for more than one application thread to be waiting
   * on the same WaitSet. If the wait operation is invoked on a WaitSet that
   * already has a thread blocking on it, the operation will raise
   * immediately an exception PreconditionNotMet.
   *
   * The result of the wait operation is the list of all the attached
   * conditions that have a trigger_value of TRUE (i.e., the conditions
   * that unblocked the wait).
   *
   * @throws PreconditionNotMetException when multiple thread try to invoke
   *        the method concurrently.
   *
   * @return a vector containing the triggered conditions
   */
  const ConditionSeq wait();

  /**
   * This operation allows an application thread to wait for the occurrence
   * of certain conditions. If none of the conditions attached to the
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
   * already has a thread blocking on it, the operation will raise
   * immediately an exception PreconditionNotMet.
   *
   * The result of the wait operation is the list of all the attached
   * conditions that have a trigger_value of TRUE (i.e., the conditions
   * that unblocked the wait).
   *
   * @param timeout the maximum amount of time for which the wait
   * should block while waiting for a condition to be triggered.
   *
   * @throws PreconditionNotMetException when multiple thread try to invoke
   *        the method concurrently.
   *
   * @return a vector containing the triggered conditions
   */
  ConditionSeq& wait(ConditionSeq& triggered,
      const dds::core::Duration& timeout);

  /**
   * This operation allows an application thread to wait for the occurrence
   * of certain conditions. If none of the conditions attached to the
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
   * already has a thread blocking on it, the operation will raise
   * immediately an exception PreconditionNotMet.
   *
   * The result of the wait operation is the list of all the attached
   * conditions that have a trigger_value of TRUE (i.e., the conditions
   * that unblocked the wait).
   *
   * @param timeout the maximum amount of time for which the wait
   * should block while waiting for a condition to be triggered.
   *
   * @throws PreconditionNotMetException when multiple thread try to invoke
   *        the method concurrently.
   *
   * @return a vector containing the triggered conditions
   */
  ConditionSeq& wait(ConditionSeq& triggered);

public:
  /**
   * Waits for at least one of the attached conditions to  trigger and then
   * dispatches the events.
   *
   */
  void dispatch();

  /**
   * Waits for at least one of the attached conditions to  trigger and then
   * dispatches the events, or, times out and unblocks.
   *
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
   * trigger_value of TRUE, then attaching the condition will unblock
   * the WaitSet. Adding a Condition that is already attached to the WaitSet
   * has no effect.
   *
   * @param cond the condition to be attached to this waitset.
   */
  TWaitSet& attach_condition(const dds::core::cond::Condition& cond);

  /**
   * Detaches a Condition from the WaitSet. If the Condition was not
   * attached to the WaitSet, the operation will return false.
   *
   * @param cond the condition to detach from this WaitSet
   * @return true if the condition was found and detached, false if the
   *         condition was not part of the WaitSet.
   */
  bool detach_condition(const dds::core::cond::Condition& cond);

  /**
   * This operation retrieves the list of attached conditions.
   *
   * @return the list of attached conditions.
   */
  const ConditionSeq& conditions() const;

  /**
   * This operation retrieves the list of attached conditions.
   *
   * @return the list of attached conditions.
   */
  ConditionSeq& conditions(ConditionSeq& conds) const;
};

#endif /* OMG_TDDS_CORE_WAIT_SET_HPP_ */
