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
#ifndef CMA__OBJECT_H_
#define CMA__OBJECT_H_

#include "cma__types.h"

#ifndef NDEBUG
#define CMA_OBJECT_VALIDKIND(k) (((k) > CMA_OBJECT_INVALID) && ((k) < CMA_OBJECT_COUNT))
#define CMA_OBJECT_CONFIDENCE   (3123123123U)
#else
#define CMA_OBJECT_VALIDKIND(k)
#endif /* NDEBUG */

typedef enum cma_objectKind_e {
#ifndef NDEBUG
    CMA_OBJECT_INVALID,
#endif /* NDEBUG */
    CMA_OBJECT_SERVICE,
    CMA_OBJECT_THREAD,
    CMA_OBJECT_LOGBUF,
    CMA_OBJECT_CONFIGURATION /* No comma */
#ifndef NDEBUG
    , CMA_OBJECT_COUNT
#endif /* NDEBUG */
} cma_objectKind;

typedef void (*cma_objectDeinitFunc)(cma_object);

C_STRUCT(cma_object) {
#ifndef NDEBUG
    os_uint32 confidence;
#endif /* NDEBUG */
    pa_uint32_t refCount;
    cma_objectDeinitFunc deinit;
    cma_objectKind kind;
};

#define cma_object(o) ((cma_object)(o))

void
cma__objectInit(
    cma_object _this,
    cma_objectKind kind,
    cma_objectDeinitFunc func) __nonnull_all__;

void
cma__objectDeinit(
    cma_object _this) __nonnull_all__;

cma_objectKind
cma__objectKind(
    cma_object _this) __nonnull_all__;

void
cma__objectSetDeinit(
    cma_object _this,
    cma_objectDeinitFunc func) __nonnull_all__;

cma_object
cma__objectKeep(
    cma_object _this) __attribute_returns_nonnull__ __nonnull_all__;

#define cma_objectKeep(o) cma__objectKeep(cma_object(o))

void
cma__objectFree(
    cma_object _this);

#define cma_objectFree(o) cma__objectFree(cma_object(o))

#ifndef NDEBUG
c_bool
cma__objectIsValid(
    cma_object _this) __attribute_pure__;

c_bool
cma__objectIsValidKind(
    cma_object _this,
    cma_objectKind kind) __attribute_pure__;
#else
#define cma__objectIsValid(o) (TRUE)
#define cma__objectIsValidKind(o, k) (TRUE)
#endif /* NDEBUG */

#define cma_objectIsValid(o) assert(cma__objectIsValid(cma_object(o)))
#define cma_objectIsValidKind(o, k) assert(cma__objectIsValidKind(cma_object(o), k))

const char*
cma__objectKindImage(
    cma_object _this) __nonnull_all__ __attribute_pure__;

#define cma_objectKindImage(o) cma__objectKindImage(cma_object(o))

#endif /* CMA__OBJECT_H_ */
