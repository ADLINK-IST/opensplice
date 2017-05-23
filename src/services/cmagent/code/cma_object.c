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
#include "cma__object.h"

#include "vortex_os.h"
#include "os_atomics.h"

void
cma__objectInit(
    cma_object _this,
    cma_objectKind kind,
    cma_objectDeinitFunc func)
{
    assert(_this);
    assert(CMA_OBJECT_VALIDKIND(kind));
    assert(func);

#ifndef NDEBUG
    _this->confidence = CMA_OBJECT_CONFIDENCE;
#endif /* NDEBUG */
    pa_st32(&(_this->refCount), 1);
    _this->deinit = func;
    _this->kind = kind;
}

void
cma__objectDeinit(
    cma_object _this)
{
#ifndef NDEBUG
    assert(cma__objectIsValid(_this));
    _this->confidence = 0;
#else
    OS_UNUSED_ARG(_this);
#endif /* NDEBUG */
}

cma_objectKind
cma__objectKind(
    cma_object _this)
{
    assert(cma__objectIsValid(_this));
    return _this->kind;
}

void
cma__objectSetDeinit(
    cma_object _this,
    cma_objectDeinitFunc func)
{
    assert(cma__objectIsValid(_this));

    _this->deinit = func;
}

cma_object
cma__objectKeep(
    cma_object _this)
{
    assert(cma__objectIsValid(_this));

    pa_inc32(&(_this->refCount));

    return _this;
}

void
cma__objectFree(
    cma_object _this)
{
    if (_this) {
        os_uint32 refCount;

        assert(cma__objectIsValid(_this));

        refCount = pa_dec32_nv(&(_this->refCount));
        if (refCount == 0) {
            assert(_this->deinit);
            _this->deinit(_this);
            os_free(_this);
        }
    }
}

#ifndef NDEBUG
c_bool
cma__objectIsValid(
    cma_object _this)
{
    if (!_this) return FALSE;
    if (!(_this->confidence == CMA_OBJECT_CONFIDENCE)) return FALSE;
    if (!CMA_OBJECT_VALIDKIND(_this->kind)) return FALSE;
    return TRUE;
}

c_bool
cma__objectIsValidKind(
    cma_object _this,
    cma_objectKind kind)
{
    if (!cma__objectIsValid(_this)) return FALSE;
    if (cma__objectKind(_this) != kind) return FALSE;
    return TRUE;
}
#endif /* NDEBUG */

const char*
cma__objectKindImage(
    cma_object _this)
{
    const char *image;

    cma_objectIsValid(_this);

#define _CMA_IMAGE_STR(k) ((char*)((os_address)(#k) + strlen("CMA_OBJECT_")))
#define _CMA_IMAGE_STR_CASE(k) case (k): image = _CMA_IMAGE_STR(k); break;
    switch(_this->kind) {
        _CMA_IMAGE_STR_CASE(CMA_OBJECT_SERVICE);
        _CMA_IMAGE_STR_CASE(CMA_OBJECT_THREAD);
        _CMA_IMAGE_STR_CASE(CMA_OBJECT_LOGBUF);
        _CMA_IMAGE_STR_CASE(CMA_OBJECT_CONFIGURATION);
        default:
            image = "(invalid kind)";
            break;
    }
#undef _CMA_IMAGE_STR_CASE
#undef _CMA_IMAGE_STR
    return image;
}
