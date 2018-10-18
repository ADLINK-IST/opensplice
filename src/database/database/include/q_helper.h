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
#ifndef Q_HELPER_H
#define Q_HELPER_H

#if defined (__cplusplus)
extern "C" {
#endif
#include "os_if.h"

#ifdef OSPL_BUILD_CORE
#define OS_API OS_API_EXPORT
#else
#define OS_API OS_API_IMPORT
#endif

OS_API void
q_exprSetText(
    q_expr expr, const c_char* text);

OS_API c_char*
q_exprGetText(
    q_expr expr);

OS_API void
q_exprSetInstanceState(
    q_expr expr,
    c_ulong state);

OS_API void
q_exprSetSampleState(
    q_expr expr,
    c_ulong state);

OS_API void
q_exprSetViewState(
    q_expr expr,
    c_ulong state);

OS_API c_ulong
q_exprGetInstanceState(
    q_expr expr);

OS_API c_ulong
q_exprGetSampleState(
    q_expr expr);

OS_API c_ulong
q_exprGetViewState(
    q_expr expr);

OS_API c_iter
q_exprDeOr(
    q_expr e,
    c_iter list);

#undef OS_API

#if defined (__cplusplus)
}
#endif

#endif /*Q_PARSER_H */
