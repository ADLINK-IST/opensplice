/*
 *                         Vortex OpenSplice
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR ADLINK
 *   Technology Limited, its affiliated companies and licensors. All rights
 *   reserved.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
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
