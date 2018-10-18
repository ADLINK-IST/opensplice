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
#ifndef CMA__UTIL_H_
#define CMA__UTIL_H_

#include "vortex_os.h"

/* cma_time related stuff */
typedef os_int64 cma_time;

#define CMA_TIME_NSEC_FMT "%.9f"
#define CMA_TIME_USEC_FMT "%.6f"
#define CMA_TIME_MSEC_FMT "%.3f"

#define CMA_TIME_TO_REAL(t) ((t) / 1000000000.0f)

/* Get the current time in a 64-bit integer */
cma_time
cma_timeNow(void);

/* Convert os_timeW to cma_time value */
cma_time
cma_time_os_time_conv(
    const os_timeW time) __attribute_const__;

/* Convert cma_time to sec and usec values */
void
cma_time_sec_usec_conv(
    cma_time time,
    int *sec,
    int *usec) __nonnull_all__;

#endif /* CMA__UTIL_H_ */
