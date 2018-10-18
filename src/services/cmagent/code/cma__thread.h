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
#ifndef CMA__THREAD_H_
#define CMA__THREAD_H_

#include "cma__object.h"
#include "cma__log.h"

typedef void* (*cma_threadRoutine)(void *arg);

#ifndef NDEBUG
#define cma_thread(o) (assert(cma__objectKind(cma_object(o)) == CMA_OBJECT_THREAD), (cma_thread)(o))
#else
#define cma_thread(o) ((cma_thread)(o))
#endif

#define cma_threadStates(o) ((cma_threadStates)(o))

cma_threadStates
cma_threadStatesAlloc(void) __attribute_malloc__ __attribute_returns_nonnull__;

os_result
cma_threadStatesInit(
    cma_threadStates ts,
    unsigned maxThreads,
    cma_logConfig gc) __nonnull_all__;

void
cma_threadStatesDeinit(
    cma_threadStates ts) __nonnull_all__;

void
cma_threadStatesDealloc(
    cma_threadStates ts) __nonnull_all__;

void
cma_threadUpgrade(
    cma_threadStates ts) __nonnull_all__;

void
cma_threadDowngrade(void);

cma_thread
cma_threadLookup(void);

cma_logConfig
cma_threadLogConfig(
    cma_thread _this) __nonnull_all__ __attribute_returns_nonnull__;

const char*
cma_threadName(
    cma_thread _this) __nonnull_all__ __attribute_returns_nonnull__;

cma_logbuf
cma_threadLogbuf(
    cma_thread _this) __nonnull_all__ __attribute_returns_nonnull__;

os_result
cma_threadCreate(
    const char *name,
    cma_thread *thread,
    const cma_threadRoutine func,
    void *arg) __nonnull((1,2,3));

os_result
cma_threadJoin(
    cma_thread _this,
    void **retval) __nonnull((1));

#endif /* CMA__THREAD_H_ */
