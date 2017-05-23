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
#ifndef U__OBJECT_H
#define U__OBJECT_H

#include "u_object.h"
#include "u__types.h"
#include "os_if.h"

#ifdef OSPL_BUILD_CORE
#define OS_API OS_API_EXPORT
#else
#define OS_API OS_API_IMPORT
#endif
/* !!!!!!!!NOTE From here no more includes are allowed!!!!!!! */

/* u_objectAlloc allocates and zeros allocSize bytes and initializes the common
 * object attributes from the remaining parameters.  The deinit function is the
 * function invoked by u_objectDeinit and is expected to recursively call the
 * superclass' deinit function, ending at u__objectDeinitW.  The free function is
 * analogous for freeing the memory.
 *
 * These internal deinit/free functions are suffixed with "W" -- the exact suffix
 * is a historical accident without much meaning.  The only non-W functions that
 * ever invokes a W function are u_objectClose and u_objectFree. */
void *
u_objectAlloc(
        size_t allocSize,
        u_kind kind,
        u_deinitFunction_t deinit,
        u_freeFunction_t free)
    __attribute_returns_nonnull__;

u_result
u__objectDeinitW(
    void *_this);

void
u__objectFreeW(
    void *_this);

#undef OS_API

#endif

