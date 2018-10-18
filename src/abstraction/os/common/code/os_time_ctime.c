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

/** \file os/common/code/os_time.c
 *  \brief Time management
 *
 * Implements posix based time management functions
 */

#include <time.h>

/** \brief Translate calendar time into readable string representation
 */
#ifdef _WIN32
#include <windows.h>
os_size_t
os_ctimeW_r(
    os_timeW *t,
    char *buf,
    os_size_t bufsz)
{
    int tz_bias, tz_hours, tz_minutes;
    TIME_ZONE_INFORMATION tzi;
    time_t tt;
    os_size_t result = 0;

    assert(bufsz >= OS_CTIME_R_BUFSIZE);

    if (buf) {
        switch(GetTimeZoneInformation(&tzi)) {
            case TIME_ZONE_ID_STANDARD: tz_bias = tzi.Bias + tzi.StandardBias; break;
            case TIME_ZONE_ID_DAYLIGHT: tz_bias = tzi.Bias + tzi.DaylightBias; break;
            default: tz_bias = tzi.Bias; break;
        }
        tz_bias    = -tz_bias;
        tz_hours   = tz_bias / 60;
        tz_minutes = tz_bias % 60;

        tt = (time_t) OS_TIMEW_GET_SECONDS(*t);
        result = strftime(buf, bufsz, "%Y-%m-%dT%H:%M:%S", localtime(&tt));
        assert(result+6 < bufsz);

        buf[result++] = (tz_hours > 0) ? '+' : '-';
        sprintf_s(&buf[result], 5, "%02d%02d", abs(tz_hours), abs(tz_minutes));
    }
    return result;
}
#else
os_size_t
os_ctimeW_r(
    os_timeW *t,
    char *buf,
    os_size_t bufsz)
{
    os_size_t result = 0;
    time_t tt;

    assert(bufsz >= OS_CTIME_R_BUFSIZE);
    assert(t);

    if (buf) {
        tt = (time_t) OS_TIMEW_GET_SECONDS(*t);
        result = strftime(buf, bufsz, "%Y-%m-%dT%H:%M:%S%z", localtime(&tt));
        assert(result);
    }
    return result;
}
#endif
