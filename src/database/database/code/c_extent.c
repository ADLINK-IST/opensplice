/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2011 PrismTech
 *   Limited and its licensees. All rights reserved. See file:
 *
 *                     $OSPL_HOME/LICENSE
 *
 *   for full copyright notice and license terms.
 *
 */
#include "c__extent.h"
#include "c__base.h"
#include "c_mmbase.h"
#include "os_report.h"
#include "os.h"

c_extent
c_extentNew(
    c_type type,
    const c_long blockSize)
{
    return c_extentSyncNew(type,blockSize,FALSE);
}

c_extent
c_extentSyncNew(
    c_type type,
    const c_long blockSize,
    c_bool sync)
{
    c_extent _this;

    assert(type != NULL);
    assert(blockSize >= 0);

    if (type->size == 0) {
        return NULL;
    }
    if (sync) {
        _this = c_new(c_getMetaType(type->base,M_EXTENTSYNC));
        if (_this) {
            c_baseObject(_this)->kind = M_EXTENTSYNC;
        }
    } else {
        _this = c_new(c_getMetaType(type->base,M_EXTENT));
        if (_this) {
            c_baseObject(_this)->kind = M_EXTENT;
        }
    }
    if (_this) {
        _this->sync = sync;
        c_typeDef(_this)->alias = c_keep(type);
        c_type(_this)->base = type->base;
        c_type(_this)->alignment = type->alignment;
        c_type(_this)->size = type->size;
        c_type(_this)->objectCount = 0;
        c_metaObject(_this)->definedIn = c_metaObject(type->base);
        c_metaObject(_this)->name = NULL;

        if (sync) {
            c_mutexInit(&c_extentSync(_this)->mutex,SHARED_MUTEX);
        }

        if (blockSize == 0) {
            _this->cache = NULL;
        } else {
            _this->cache = c_mmCacheCreate(c_baseMM(type->base),
                                           c_memsize(type),
                                           blockSize);
        }
    }
    return _this;
}

c_object
c_extentCreate(
    c_extent _this)
{
    c_object object;
    c_voidp memory;
    c_syncResult result;

    assert(_this != NULL);

    if (_this->sync) {
        result = c_mutexLock(&c_extentSync(_this)->mutex);
        assert(result == SYNC_RESULT_SUCCESS);
    }

    if (_this->cache == NULL) {
        object = c_new(c_type(_this));
    } else {
        memory = c_mmCacheMalloc(_this->cache);
        object = c_mem2object(memory, c_type(_this));
    }
    pa_increment(&c_typeDef(_this)->alias->objectCount);

    if (_this->sync) {
        result = c_mutexUnlock(&c_extentSync(_this)->mutex);
        assert(result == SYNC_RESULT_SUCCESS);
    }
    return object;
}

/* Do not use cast functions with C_TYPECHECK as
   this function is called when refcount is already 0
*/
void
c_extentFree(
    c_extent _this)
{
    c_syncResult result;

    assert(_this != NULL);

    if (_this->sync) {
        result = c_mutexLock(&((c_extentSync)_this)->mutex);
        assert(result == SYNC_RESULT_SUCCESS);
    }

    if (_this->cache != NULL) {
        c_mmCacheDestroy(_this->cache);
        _this->cache = NULL;
    }

    if (_this->sync) {
        result = c_mutexUnlock(&((c_extentSync)_this)->mutex);
        assert(result == SYNC_RESULT_SUCCESS);
        /* mutex is destroyed through metadata! */
    }
    /* Do not c_free(((c_typeDef)_this)->alias) as it is freed through metadata! */
}

void
c_extentDelete (
    c_extent _this,
    c_object object)
{
    c_voidp memory;
    c_syncResult result;

    assert(_this != NULL);

    if (_this->sync) {
        result = c_mutexLock(&c_extentSync(_this)->mutex);
        assert(result == SYNC_RESULT_SUCCESS);
    }

    assert(_this->cache != NULL);
    if (_this->cache == NULL) {
        c__free(object);
    } else {
        memory = c_object2mem(object);
#ifndef NDEBUG
        memset(memory, 0, c_memsize(c_typeDef(_this)->alias)); /*added*/
#endif
        c_mmCacheFree(_this->cache,memory);
    }
    pa_decrement(&c_typeDef(_this)->alias->objectCount);

    if (_this->sync) {
        result = c_mutexUnlock(&c_extentSync(_this)->mutex);
        assert(result == SYNC_RESULT_SUCCESS);
    }
}

c_type
c_extentType(
    c_extent _this)
{
    assert(_this != NULL);
    return c_keep(c_typeDef(_this)->alias);
}

