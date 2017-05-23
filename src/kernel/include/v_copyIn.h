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
#ifndef V_COPYIN_H
#define V_COPYIN_H

#include "c_typebase.h"

#if defined (__cplusplus)
extern "C" {
#endif

typedef os_uchar v_copyin_result;

#define V_COPYIN_RESULT_INVALID              0
#define V_COPYIN_RESULT_OK                   1
#define V_COPYIN_RESULT_OUT_OF_MEMORY        2

#define V_COPYIN_RESULT_IS_OK(x)             ((x) == V_COPYIN_RESULT_OK)
#define V_COPYIN_RESULT_IS_INVALID(x)        ((x) == V_COPYIN_RESULT_INVALID)
#define V_COPYIN_RESULT_IS_OUT_OF_MEMORY(x)  ((x) == V_COPYIN_RESULT_OUT_OF_MEMORY)

#define V_COPYIN_RESULT_TO_RESULT(r) \
    (V_COPYIN_RESULT_IS_OK(r) ? V_RESULT_OK : V_COPYIN_RESULT_IS_INVALID(r) ? V_RESULT_ILL_PARAM : V_RESULT_OUT_OF_MEMORY)


#if defined (__cplusplus)
}
#endif

#endif /* V_COPYIN_H */
