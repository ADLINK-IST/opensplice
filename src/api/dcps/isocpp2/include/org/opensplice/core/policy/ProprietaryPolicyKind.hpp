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


/**
 * @file
 */

#ifndef ORG_OPENSPLICE_CORE_POLICY_PROPRIETARYPOLICYKIND_HPP_
#define ORG_OPENSPLICE_CORE_POLICY_PROPRIETARYPOLICYKIND_HPP_


#include <dds/core/SafeEnumeration.hpp>

namespace org
{
namespace opensplice
{
namespace core
{
namespace policy
{

/*
 * Proprietary policy values
 */
struct InvalidSampleVisibility_def
{
    enum Type
    {
        NO_INVALID_SAMPLES,
        MINIMUM_INVALID_SAMPLES,
        ALL_INVALID_SAMPLES
    };
};
typedef dds::core::safe_enum<InvalidSampleVisibility_def> InvalidSampleVisibility;

struct SchedulingKind_def
{
    enum Type
    {
        SCHEDULE_DEFAULT,
        SCHEDULE_TIMESHARING,
        SCHEDULE_REALTIME
    };
};
typedef dds::core::safe_enum<SchedulingKind_def> SchedulingKind;

struct SchedulingPriorityKind_def
{
    enum Type
    {
        PRIORITY_RELATIVE,
        PRIORITY_ABSOLUTE
    };
};
typedef dds::core::safe_enum<SchedulingPriorityKind_def> SchedulingPriorityKind;

}
}
}
}

#endif /* ORG_OPENSPLICE_CORE_POLICY_PROPRIETARYPOLICYKIND_HPP_ */
