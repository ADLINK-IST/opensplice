/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2012 PrismTech
 *   Limited and its licensees. All rights reserved. See file:
 *
 *                     $OSPL_HOME/LICENSE
 *
 *   for full copyright notice and license terms.
 *
 */
#ifndef OSPL_DDS_CORE_COND_TWAITSET_HPP_
#define OSPL_DDS_CORE_COND_TWAITSET_HPP_

/**
 * @file
 */

/*
 * OMG PSM class declaration
 */
#include <spec/dds/core/cond/TWaitSet.hpp>

// Implementation
namespace dds
{
namespace core
{
namespace cond
{

template <typename DELEGATE>
TWaitSet<DELEGATE>::~TWaitSet() { }

template <typename DELEGATE>
const typename TWaitSet<DELEGATE>::ConditionSeq TWaitSet<DELEGATE>::wait(const dds::core::Duration& timeout)
{
    return this->delegate()->wait(timeout);
}

template <typename DELEGATE>
const typename TWaitSet<DELEGATE>::ConditionSeq TWaitSet<DELEGATE>::wait()
{
    return this->delegate()->wait();
}

template <typename DELEGATE>
typename TWaitSet<DELEGATE>::ConditionSeq& TWaitSet<DELEGATE>::wait(ConditionSeq& triggered, const dds::core::Duration& timeout)
{
    return this->delegate()->wait(triggered, timeout);
}

template <typename DELEGATE>
typename TWaitSet<DELEGATE>::ConditionSeq& TWaitSet<DELEGATE>::wait(ConditionSeq& triggered)
{
    return this->delegate()->wait(triggered);
}

template <typename DELEGATE>
void TWaitSet<DELEGATE>::dispatch()
{
    this->delegate()->dispatch();
}

template <typename DELEGATE>
void TWaitSet<DELEGATE>::dispatch(const dds::core::Duration& timeout)
{
    this->delegate()->dispatch(timeout);
}

template <typename DELEGATE>
TWaitSet<DELEGATE>& TWaitSet<DELEGATE>::operator +=(const dds::core::cond::Condition& cond)
{
    this->delegate()->attach_condition(cond);
    return *this;
}

template <typename DELEGATE>
TWaitSet<DELEGATE>& TWaitSet<DELEGATE>::operator -=(const dds::core::cond::Condition& cond)
{
    this->delegate()->detach_condition(cond);
    return *this;
}

template <typename DELEGATE>
TWaitSet<DELEGATE>& TWaitSet<DELEGATE>::attach_condition(const dds::core::cond::Condition& cond)
{
    this->delegate()->attach_condition(cond);
    return *this;
}

template <typename DELEGATE>
bool TWaitSet<DELEGATE>::detach_condition(const dds::core::cond::Condition& cond)
{
    return this->delegate()->detach_condition(cond);
}

template <typename DELEGATE>
const typename TWaitSet<DELEGATE>::ConditionSeq& TWaitSet<DELEGATE>::conditions() const
{
    return this->delegate()->conditions();
}

template <typename DELEGATE>
typename TWaitSet<DELEGATE>::ConditionSeq& TWaitSet<DELEGATE>::conditions(ConditionSeq& conds) const
{
    return this->delegate()->conditions(conds);
}

}
}
}

// End of implementation

#endif /* OSPL_DDS_CORE_COND_TWAITSET_HPP_ */
