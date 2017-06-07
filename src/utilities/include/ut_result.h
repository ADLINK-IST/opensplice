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
#ifndef UT_RESULT_H
#define UT_RESULT_H

#if defined (__cplusplus)
extern "C" {
#endif
#include "os_if.h"

#ifdef OSPL_BUILD_CORE
#define OS_API OS_API_EXPORT
#else
#define OS_API OS_API_IMPORT
#endif
/* !!!!!!!!NOTE From here no more includes are allowed!!!!!!! */

typedef enum ut_result_e {
    UT_RESULT_UNDEFINED = OS_RETCODE_ID_UT_RESULT,
    UT_RESULT_OK,
    UT_RESULT_OUT_OF_MEMORY,
    UT_RESULT_WALK_ABORTED,
    UT_RESULT_COUNT,
    UT_RESULT_NOT_IMPLEMENTED,
    UT_RESULT_ILL_PARAM,
    UT_RESULT_ERROR
} ut_result;

OS_API os_int
ut_resultToReturnCode(
    ut_result result);

#undef OS_API

#if defined (__cplusplus)
}
#endif

#endif /* UT_RESULT_H */
