/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR PrismTech
 *   Limited, its affiliated companies and licensors. All rights reserved.
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
        /* This should be common code. But unfortunately, VS2012 C Runtime contains
         * a bug that causes a crash when using %Z with a buffer that is too small:
         * https://connect.microsoft.com/VisualStudio/feedback/details/782889/
         * So, don't execute strftime with %Z when VS2012 is the compiler. */
#if !(_MSC_VER == 1700)
        result = strftime(buf, bufsz, "%a %b %d %H:%M:%S %Z %Y", localtime(&tt));
#endif

        if(result == 0) {
            /* If not enough room was available, the %Z (time-zone) is left out
             * resulting in the output as expected from ctime_r. */
            result = strftime(buf, bufsz, "%a %b %d %H:%M:%S %Y", localtime(&tt));
            assert(result);
        }
    }
    return result;
}
