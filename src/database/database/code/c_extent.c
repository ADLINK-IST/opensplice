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
    } else {
        _this = c_new(c_getMetaType(type->base,M_EXTENT));
    }
    _this->next = NULL;
    _this->prev = NULL;
    _this->sync = sync;
    if (sync) {
        c_mutexInit(&c_extentSync(_this)->mutex,SHARED_MUTEX);
    }

    assert(type->size > 0);

    /* type is not kept to avoid memory leakage
     * (also see the metadata definition). */
    _this->type = type;
    if (blockSize == 0) {
        _this->cache = NULL;
    } else {
        _this->cache = c_mmCacheCreate(c_baseMM(type->base),
                                       c_memsize(type),
                                       blockSize);
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
        object = c__new(_this->type);
    } else {
        memory = c_mmCacheMalloc(_this->cache);
        object = c_mem2object(memory,_this);
    }
    pa_increment(&_this->type->objectCount);

    c_keep(_this->type);
    c_keep(_this);

    if (_this->sync) {
        result = c_mutexUnlock(&c_extentSync(_this)->mutex);
        assert(result == SYNC_RESULT_SUCCESS);
    }

    return object;
}

void
c_extentFree(
    c_extent _this)
{
    c_syncResult result;

    assert(_this != NULL);
    if (_this->sync) {
        result = c_mutexLock(&c_extentSync(_this)->mutex);
        assert(result == SYNC_RESULT_SUCCESS);
    }

    if (_this->cache != NULL) {
        c_mmCacheDestroy(_this->cache);
        _this->cache = NULL;
    }

    if (_this->sync) {
        result = c_mutexUnlock(&c_extentSync(_this)->mutex);
        assert(result == SYNC_RESULT_SUCCESS);
    }
    c_mutexDestroy(&c_extentSync(_this)->mutex);    
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

    if (_this->cache == NULL) {
        c__free(object);
    } else {
        memory = c_object2mem(object,_this->type);
#ifndef NDEBUG
        memset(memory, 0, c_memsize(_this->type)); /*added*/
#endif
        c_mmCacheFree(_this->cache,memory);
    }
    pa_decrement(&_this->type->objectCount);

    if (_this->sync) {
        result = c_mutexUnlock(&c_extentSync(_this)->mutex);
        assert(result == SYNC_RESULT_SUCCESS);
    }
    c_free(_this->type);
    c_free(_this);/*added*/
}

c_type
c_extentType(
c_extent _this)
{
    assert(_this != NULL);
    return c_keep(_this->type);
}

c_voidp
c_extentMalloc (
    c_extent _this,
    c_long size)
{
    c_voidp memory;
    c_syncResult result;

    assert(_this != NULL);
    assert(size > 0);

    if (_this->sync) {
        result = c_mutexLock(&c_extentSync(_this)->mutex);
        assert(result == SYNC_RESULT_SUCCESS);
    }

    assert(_this->cache == NULL);
    memory = c_mmMalloc(c_baseMM(_this->type->base), size);
    pa_increment(&_this->type->objectCount);

    c_keep(_this->type);
    c_keep(_this);

    if (_this->sync) {
        result = c_mutexUnlock(&c_extentSync(_this)->mutex);
        assert(result == SYNC_RESULT_SUCCESS);
    }

    return memory;
}

c_type
c_extentRegister (
    c_base base)
{
    c_metaObject o;
    c_extent extent;

    /** Create the c_extent meta data type */
    o = c_metaObject(c_new(c_getMetaType(base,M_CLASS)));
    c_baseObject(o)->kind = M_CLASS;
    c_type(o)->base = base;
    c_type(o)->alignment = C_ALIGNMENT(C_STRUCT(c_extent));
    c_type(o)->size = C_SIZEOF(c_extent);

    /* c_type(o)->extent is not assigned in the next call yet, because it
       doesn't exist after the next call. This is corrected in c_baseInit()
    */
    extent = c__new((c_object)o);
    extent->next = NULL;
    extent->prev = NULL;
    extent->sync = FALSE;
    extent->cache = NULL;
    extent->type = c_keep(o);

    c_type(o)->extent = extent;
    return c_type(o);
}
