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
#include "cma__util.h"

#define CMA__NSECS_CONST ((cma_time)1000000000)

cma_time
cma_timeNow(void)
{
    os_timeW tv = os_timeWGet();
    return ((cma_time) OS_TIMEW_GET_VALUE(tv));
}

cma_time
cma_time_os_time_conv(
    const os_timeW time)
{
    assert(!OS_TIMEW_ISINFINITE(time));
    assert(!OS_TIMEW_ISINVALID(time));

    return ((cma_time) OS_TIMEW_GET_VALUE(time));
}

void
cma_time_sec_usec_conv(
    cma_time tv,
    int *sec,
    int *usec)
{
    assert(sec);
    assert(usec);

    *sec = (int)(tv / CMA__NSECS_CONST);
    *usec = (int)(tv % CMA__NSECS_CONST) / 1000;
}
