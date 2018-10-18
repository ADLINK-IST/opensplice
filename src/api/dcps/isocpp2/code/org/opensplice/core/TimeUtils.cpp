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

#include <org/opensplice/core/TimeUtils.hpp>
#include <org/opensplice/core/ReportUtils.hpp>
#include "os_abstract.h"

os_timeW
org::opensplice::core::timeUtils::convertTime(
    const dds::core::Time& time,
    os_int64 maxSupportedSeconds)
{
    os_timeW uTime = OS_TIMEW_INVALID;

    if (time == dds::core::Time::invalid()) {
        uTime = OS_TIMEW_INVALID;
    } else if ((time.sec() >= 0) && (time.sec() <= maxSupportedSeconds)) {
        uTime = OS_TIMEW_INIT(time.sec(), time.nanosec());
    } else {
        ISOCPP_THROW_EXCEPTION(ISOCPP_INVALID_ARGUMENT_ERROR, "Specified time is negative or to large: (%" PA_PRId64 ".%09u)",
                               time.sec(), time.nanosec());
    }

    return uTime;
}

os_duration
org::opensplice::core::timeUtils::convertDuration(
    const dds::core::Duration& duration)
{
    os_duration uDuration = OS_DURATION_INFINITE;

    if (duration == dds::core::Duration::infinite()) {
        uDuration = OS_DURATION_INFINITE;
    } else if ((duration.sec() >= 0) && (duration.sec() <= OS_TIME_INFINITE_SEC)) {
        uDuration = OS_DURATION_INIT(duration.sec(), duration.nanosec());
    } else {
        ISOCPP_THROW_EXCEPTION(ISOCPP_INVALID_ARGUMENT_ERROR,
                               "Specified duration is negative or to large: (%" PA_PRId64 ".%09u)",
                               duration.sec(), duration.nanosec());
    }

    return uDuration;
}
