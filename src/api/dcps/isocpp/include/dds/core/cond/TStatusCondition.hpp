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
#ifndef OSPL_DDS_CORE_COND_TSTATUSCONDITION_HPP_
#define OSPL_DDS_CORE_COND_TSTATUSCONDITION_HPP_

/**
 * @file
 */

/*
 * OMG PSM class declaration
 */
#include <spec/dds/core/cond/TStatusCondition.hpp>

// Implementation
namespace dds
{
namespace core
{
namespace cond
{

template <typename DELEGATE>
TStatusCondition<DELEGATE>::TStatusCondition(const dds::core::null_type&) : dds::core::cond::TCondition<DELEGATE>(dds::core::null) { }

template <typename DELEGATE>
TStatusCondition<DELEGATE>::TStatusCondition(const dds::core::Entity& e) : dds::core::cond::TCondition<DELEGATE>(new DELEGATE(e)) { }

template <typename DELEGATE>
template <typename FUN>
TStatusCondition<DELEGATE>::TStatusCondition(const dds::core::Entity& e, const FUN& functor)
    : dds::core::cond::TCondition<DELEGATE>(new DELEGATE(e, functor)) { }

template <typename DELEGATE>
void TStatusCondition<DELEGATE>::enabled_statuses(const dds::core::status::StatusMask& status) const
{
    this->delegate()->enabled_statuses(status);
}

template <typename DELEGATE>
const dds::core::status::StatusMask TStatusCondition<DELEGATE>::enabled_statuses() const
{
    return this->delegate()->enabled_statuses();
}

template <typename DELEGATE>
const dds::core::Entity& TStatusCondition<DELEGATE>::entity() const
{
    return this->delegate()->entity();
}
}
}
}
// End of implementation

#endif /* OSPL_DDS_CORE_COND_TSTATUSCONDITION_HPP_ */
