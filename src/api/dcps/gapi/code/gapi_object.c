/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2013 PrismTech
 *   Limited and its licensees. All rights reserved. See file:
 *
 *                     $OSPL_HOME/LICENSE
 *
 *   for full copyright notice and license terms.
 *
 */
#include "gapi_object.h"
#include "os_report.h"
#include "gapi_typeSupport.h"

#define MAGIC 0x0babe000
#define MAGIC_DELETED 0xfee1dead
#define TRASH_LENGTH (64)

#define IS_TYPE(t1,t2)        (((t1&t2)==t2)?TRUE:FALSE)
#define HEADER_IS_TYPE(h,t)   (IS_TYPE(gapi_handle(h)->kind,t))

#define CHECK_REF (0)

#if CHECK_REF
#define CHECK_REF_DEPTH (32)
static char* CHECK_REF_FILE = NULL;

#define UT_TRACE(msgFormat, ...) do { \
    void *tr[CHECK_REF_DEPTH];\
    char **strs;\
    size_t s,i; \
    FILE* stream; \
    \
    if(!CHECK_REF_FILE){ \
        CHECK_REF_FILE = os_malloc(16); \
        os_sprintf(CHECK_REF_FILE, "heap.log"); \
    } \
    s = backtrace(tr, CHECK_REF_DEPTH);\
    strs = backtrace_symbols(tr, s);\
    stream = fopen(CHECK_REF_FILE, "a");\
    fprintf(stream, msgFormat, __VA_ARGS__);              \
    for (i=0;i<s;i++) fprintf(stream, "%s\n", strs[i]);\
    fprintf(stream, "\n\n\n"); \
    free(strs);\
    fflush(stream);\
    fclose(stream);\
  } while (0)
#else
#define UT_TRACE(msgFormat, ...)
#endif

typedef gapi_boolean (*dealloactorType)(void *);

C_STRUCT(_ObjectRegistry) {
    os_mutex mutex;
    c_iter active;
    void *trash[TRASH_LENGTH];
    int ptr;
};

typedef struct DeleteActionInfo_s {
    gapi_deleteEntityAction action;
    void                    *argument;
} DeleteActionInfo;


C_STRUCT(gapi_handle) {
    unsigned long              magic;
    _ObjectKind       kind;
    os_mutex          mutex;
#ifdef _RWLOCK_
    os_mutex          read;
    unsigned int      count;
#endif
    gapi_boolean      busy;
    os_cond           cv;
    gapi_boolean      beingDeleted;
    dealloactorType   deallocator;
    _ObjectRegistry   registry;
    _Object           object;
    void             *userData;
    DeleteActionInfo  deleteActionInfo;
};

_ObjectRegistry
_ObjectRegistryNew (
    void)
{
    _ObjectRegistry registry;
    os_result osResult;
    os_mutexAttr osMutexAttr;
    int i;

    registry = (_ObjectRegistry) os_malloc(C_SIZEOF(_ObjectRegistry));
    if ( registry != NULL ) {
        registry->active  = NULL;
        registry->ptr = 0;

        osResult = os_mutexAttrInit (&osMutexAttr);
        osMutexAttr.scopeAttr = OS_SCOPE_PRIVATE;
        osResult = os_mutexInit (&registry->mutex, &osMutexAttr);

        for (i = 0; i < TRASH_LENGTH; i++) {
            registry->trash[i] = NULL;
        }
    }

    return registry;
}

void
_ObjectRegistryFree (
    _ObjectRegistry registry)
{
    gapi_handle handle;
    int ptr;

    assert(registry);

    os_mutexLock(&registry->mutex);

    ptr = 0;
    while ( registry->trash[ptr] != NULL ) {
        gapi__free(registry->trash[ptr]);
        registry->trash[ptr] = NULL;
        ptr = (ptr + 1)%TRASH_LENGTH;
    }

    handle = c_iterTakeFirst(registry->active);
    while ( handle != NULL ) {
        handle->registry = NULL;
        gapi__free(handle);
        handle = c_iterTakeFirst(registry->active);
    }
    c_iterFree(registry->active);

    os_mutexUnlock(&registry->mutex);
    os_mutexDestroy(&registry->mutex);

    os_free(registry);
}

void
_ObjectRegistryRegister (
    _ObjectRegistry registry,
    _Object         object)
{
    gapi_handle handle;

    assert(registry);
    assert(object);

    os_mutexLock(&registry->mutex);

    handle = (gapi_handle) object->handle;

    assert(handle);

    handle->registry = registry;
    registry->active = c_iterInsert(registry->active, handle);

    os_mutexUnlock(&registry->mutex);
}

static void
_ObjectRegistryDeregister (
    _ObjectRegistry registry,
    gapi_handle handle)
{
    assert(registry);
    assert(handle);

    os_mutexLock(&registry->mutex);
    c_iterTake(registry->active, handle);
    if (handle->kind != OBJECT_KIND_WAITSET) {
        if (registry->trash[registry->ptr]) {
            UT_TRACE("_ObjectRegistryDeregister(%x,%x) %d freed\n",
                     (unsigned int)registry,
                     (unsigned int)registry->trash[registry->ptr],
                     ((gapi_handle)registry->trash[registry->ptr])->kind);
            gapi__free(registry->trash[registry->ptr]);
        }
        UT_TRACE("_ObjectRegistryDeregister(%x,%x) %d added to trash\n",
                   (unsigned int)registry,
                   (unsigned int)handle,
                   handle->kind);

        registry->trash[registry->ptr] = handle;
        registry->ptr = (registry->ptr+1)%TRASH_LENGTH;
    }
    os_mutexUnlock(&registry->mutex);
}

static gapi_returnCode_t
gapi_handleClaim (
    gapi_handle handle)
{
    gapi_returnCode_t result;
    os_result osr;

    /* this leaky design needs pa_increment. */
    if (handle) {
        if (handle->magic == MAGIC) {
            if ( handle->object != NULL ) {
                osr = os_mutexLock(&handle->mutex);
                if (osr == os_resultSuccess) {
                    if (handle->magic == MAGIC) {
                        result = GAPI_RETCODE_OK;
                    } else {
                        result = GAPI_RETCODE_ALREADY_DELETED;
                    }
                } else {
                    result = GAPI_RETCODE_ALREADY_DELETED;
                }
            } else {
                result = GAPI_RETCODE_ALREADY_DELETED;
            }
        } else {
            if(handle->magic == MAGIC_DELETED)
            {
                result = GAPI_RETCODE_ALREADY_DELETED;
            }
            else
            {
                result = GAPI_RETCODE_BAD_PARAMETER;
            }
        }
    } else {
        result = GAPI_RETCODE_BAD_PARAMETER;
    }
    return result;
}

static gapi_returnCode_t
gapi_handleClaimNotBusy (
    gapi_handle handle)
{
    gapi_returnCode_t result;
    os_result osr;

    /* this leaky design needs pa_increment. */
    if (handle) {
        if (handle->magic == MAGIC) {
            if ( handle->object != NULL ) {
                osr = os_mutexLock(&handle->mutex);
                if (osr == os_resultSuccess) {
                    if (handle->magic == MAGIC) {
                        while ( handle->busy ) {
                            osr = os_condWait(&handle->cv, &handle->mutex);
                            if (osr == os_resultFail)
                            {
                                OS_REPORT(OS_CRITICAL, "gapi_handleClaimNotBusy", 0,
                                          "os_condWait failed - waiting for busy handle");
                                osr = os_mutexUnlock(&handle->mutex);
                                return GAPI_RETCODE_ERROR;
                            }
                        }
                        result = GAPI_RETCODE_OK;
                    } else {
                        result = GAPI_RETCODE_ALREADY_DELETED;
                    }
                } else {
                    result = GAPI_RETCODE_ALREADY_DELETED;
                }
            } else {
                result = GAPI_RETCODE_ALREADY_DELETED;
            }
        } else {
            if(handle->magic == MAGIC_DELETED)
            {
                result = GAPI_RETCODE_ALREADY_DELETED;
            }
            else
            {
                result = GAPI_RETCODE_BAD_PARAMETER;
            }

        }
    } else {
        result = GAPI_RETCODE_BAD_PARAMETER;
    }
    return result;
}

static void
gapi_handleRelease (
    gapi_handle handle)
{
    os_mutexUnlock(&handle->mutex);
}

#ifdef _RWLOCK_
static void
gapi_handleReadClaim (
    gapi_handle handle)
{
    os_mutexLock(&handle->read);
    handle->count++;
    if (handle->count == 1) {
        os_mutexLock(&handle->mutex);
    }
    os_mutexUnlock(&handle->read);
}

static void
gapi_handleReadClaimNotBusy (
    gapi_handle handle)
{
    os_result osr;
    os_mutexLock(&handle->read);
    handle->count++;
    if (handle->count == 1) {
        os_mutexLock(&handle->mutex);
    }
    while ( handle->busy ) {
        osr = os_condWait(&handle->cv, &handle->mutex);
        if (osr == os_resultFail)
        {
            OS_REPORT(OS_CRITICAL, "gapi_handleReadClaimNotBusy", 0,
                      "os_condWait failed - waiting for busy handle");
            break;
        }
    }
    os_mutexUnlock(&handle->read);
}

static void
gapi_handleReadRelease (
    gapi_handle handle)
{
    os_mutexLock(&handle->read);
    handle->count--;
    if (handle->count == 0) {
        os_mutexUnlock(&handle->mutex);
    }
    os_mutexUnlock(&handle->read);
}

#endif

static gapi_boolean
gapi_handleFree (
    void *o)
{
    gapi_handle handle = (gapi_handle) o;
    gapi_deleteEntityAction action = NULL;
    gapi_boolean result = TRUE;
    void *userData = NULL;
    void *actionData = NULL;

    UT_TRACE("gapi_handleFree(%x) %d\n",(unsigned int)handle, handle->kind);
    if (gapi_handleClaim(handle) == GAPI_RETCODE_OK) {

        assert(handle->deallocator);

        if ( handle->userData && handle->deleteActionInfo.action ) {
            action = handle->deleteActionInfo.action;
            userData = handle->userData;
            actionData = handle->deleteActionInfo.argument;
        }

        handle->beingDeleted = TRUE;
        if ( handle->object != NULL ) {
            result = handle->deallocator(handle->object);
            if ((handle->kind == OBJECT_KIND_WAITSET) &&
                (handle->registry != NULL)) {
                _ObjectRegistryDeregister(handle->registry, handle);
                handle->registry = NULL;
            }
            /* Only release _Object when deallocator returned successfully. */
            if (result)
            {
                os_free(handle->object);
                handle->object = NULL;
            }
        }
        if ( result && action ) {
           action(userData, actionData);
        }

        /* Only release handle when deallocator returned successfully. */
        if (result == TRUE) {
            assert(handle->registry == NULL);
            handle->magic  = 0;
            gapi_handleRelease(handle);
            os_condDestroy(&handle->cv);
            os_mutexDestroy(&handle->mutex);
#ifdef _RWLOCK_
            os_mutexDestroy(&handle->read);
#endif
        } else {
            gapi_handleRelease(handle);
        }
    }
    return result;
}

_Object
_ObjectAlloc (
    _ObjectKind     kind,
    long            size,
    gapi_boolean    (*deallocator)(void *))
{
    gapi_handle handle = NULL;
    _Object     object = NULL;

    if ( deallocator != NULL ) {
        handle = (gapi_handle) gapi__malloc(gapi_handleFree, 0, (C_SIZEOF(gapi_handle)));
    } else {
        handle = (gapi_handle) gapi__malloc(NULL, 0, (C_SIZEOF(gapi_handle)));
    }
    if ( handle != NULL ) {
        os_result osResult;
        os_mutexAttr osMutexAttr;
        os_condAttr  osCondAttr;

        handle->magic    = MAGIC;
        handle->kind     = kind;
        handle->registry = NULL;
        handle->userData = NULL;
        handle->busy     = FALSE;
        handle->deleteActionInfo.action   = NULL;
        handle->deleteActionInfo.argument = NULL;

        handle->beingDeleted = FALSE;

        osResult = os_mutexAttrInit (&osMutexAttr);
        osMutexAttr.scopeAttr = OS_SCOPE_PRIVATE;
        osResult = os_mutexInit (&handle->mutex, &osMutexAttr);

        osResult = os_condAttrInit (&osCondAttr);
        osCondAttr.scopeAttr = OS_SCOPE_PRIVATE;
        osResult = os_condInit (&handle->cv, &handle->mutex, &osCondAttr);

#ifdef _RWLOCK_
        osResult = os_mutexInit (&handle->read, &osMutexAttr);
        handle->count = 0;
#endif

        object = (_Object) os_malloc(size);
        if ( object != NULL ) {
            memset(object, 0, size);

            handle->deallocator  = deallocator;

            os_mutexLock(&handle->mutex);

            handle->object   = object;
            object->handle   = (gapi_object)handle;
        } else {
            gapi__free(handle);
        }
    }
    return object;

}

void
_ObjectDelete (
    _Object object)
{
    gapi_handle handle;
    gapi_deleteEntityAction action = NULL;
    void *userData;
    void *actionData;

    assert(object);

    handle = (gapi_handle) object->handle;

    assert(handle);

    handle->object = NULL;
    object->handle = NULL;

    os_free(object);
    handle->object = NULL;

    action = handle->deleteActionInfo.action;
    if ( handle->userData && action ) {
        userData = handle->userData;
        actionData = handle->deleteActionInfo.argument;
        action(userData, actionData);
    }
    handle->userData = NULL;

    handle->magic = MAGIC_DELETED;

    if ( handle->registry != NULL ) {
        _ObjectRegistryDeregister(handle->registry, handle);
    }

    gapi_handleRelease(handle);

    os_condDestroy( &handle->cv );
    os_mutexDestroy( &handle->mutex );
#ifdef _RWLOCK_
    os_mutexDestroy(&handle->read);
#endif

    if ( handle->registry == NULL ) {
        gapi__free(handle);
    }
}

_Object
gapi_objectClaim (
    gapi_object      _this,
    _ObjectKind        kind,
    gapi_returnCode_t *result)
{
    gapi_handle       handle = gapi_handle(_this);
    _Object           object = NULL;
    gapi_returnCode_t retval;

    retval = gapi_handleClaim(handle);

    if (retval == GAPI_RETCODE_OK) {
        if ( HEADER_IS_TYPE(handle,kind) ) {
            if ( handle->object != NULL ) {
                object = handle->object;
                retval = GAPI_RETCODE_OK;
            } else {
                gapi_handleRelease(handle);
                retval = GAPI_RETCODE_ALREADY_DELETED;
            }
        } else {
            gapi_handleRelease(handle);
            retval = GAPI_RETCODE_BAD_PARAMETER;
        }
    }

    if ( result != NULL ) {
        *result = retval;
    }

    return object;
}

_Object
gapi_objectClaimNB (
    gapi_object      _this,
    _ObjectKind        kind,
    gapi_returnCode_t *result)
{
    gapi_handle       handle = gapi_handle(_this);
    _Object           object = NULL;
    gapi_returnCode_t retval;

    retval = gapi_handleClaimNotBusy(handle);

    if (retval == GAPI_RETCODE_OK) {
        if ( HEADER_IS_TYPE(handle,kind) ) {
            if ( handle->object != NULL ) {
                object = handle->object;
                retval = GAPI_RETCODE_OK;
            } else {
                gapi_handleRelease(handle);
                retval = GAPI_RETCODE_ALREADY_DELETED;
            }
        } else {
            gapi_handleRelease(handle);
            retval = GAPI_RETCODE_BAD_PARAMETER;
        }
    }

    if ( result != NULL ) {
        *result = retval;
    }

    return object;
}

_Object
gapi_objectPeek (
    gapi_object _this,
    _ObjectKind kind)
{
    gapi_handle handle = gapi_handle(_this);
    _Object     object = NULL;

    if (gapi_handleClaim(handle) == GAPI_RETCODE_OK) {
        if ( HEADER_IS_TYPE(handle,kind) ) {
            object = handle->object;
        }
        gapi_handleRelease(handle);
    }
    return object;
}

#ifdef _RWLOCK_
_Object
gapi_objectReadClaim (
    gapi_object      _this,
    _ObjectKind        kind,
    gapi_returnCode_t *result)
{
    gapi_handle       handle = gapi_handle(_this);
    _Object           object = NULL;
    gapi_returnCode_t retval = GAPI_RETCODE_BAD_PARAMETER;

    if ( handle != NULL ) {
        if ( handle->magic == MAGIC ) {
            gapi_handleReadClaim(handle);

            if ( HEADER_IS_TYPE(handle,kind) ) {
                if ( handle->object != NULL ) {
                    object = handle->object;
                    retval = GAPI_RETCODE_OK;
                } else {
                    gapi_handleReadRelease(handle);
                    retval = GAPI_RETCODE_ALREADY_DELETED;
                }
            } else {
                gapi_handleReadRelease(handle);
            }
        }
        else
        {
            if(handle->magic == MAGIC_DELETED)
            {
                retval = GAPI_RETCODE_ALREADY_DELETED;
            }
            else
            {
                retval = GAPI_RETCODE_BAD_PARAMETER;
            }
        }
    }

    if ( result != NULL ) {
        *result = retval;
    }

    return object;
}

_Object
gapi_objectReadClaimNB (
    gapi_object      _this,
    _ObjectKind        kind,
    gapi_returnCode_t *result)
{
    gapi_handle       handle = gapi_handle(_this);
    _Object           object = NULL;
    gapi_returnCode_t retval = GAPI_RETCODE_BAD_PARAMETER;

    if ( handle != NULL ) {
        if ( handle->magic == MAGIC ) {
            gapi_handleReadClaimNotBusy(handle);

            if ( HEADER_IS_TYPE(handle,kind) ) {
                if ( handle->object != NULL ) {
                   object = handle->object;
                    retval = GAPI_RETCODE_OK;
                } else {
                    gapi_handleReadRelease(handle);
                    retval = GAPI_RETCODE_ALREADY_DELETED;
                }
            } else {
                gapi_handleReadRelease(handle);
            }
        }
        else
        {
            if(handle->magic == MAGIC_DELETED)
            {
                retval = GAPI_RETCODE_ALREADY_DELETED;
            }
            else
            {
                retval = GAPI_RETCODE_BAD_PARAMETER;
            }
        }
    }

    if ( result != NULL ) {
        *result = retval;
    }

    return object;
}

_Object
gapi_objectReadPeek (
    gapi_object _this,
    _ObjectKind kind)
{
    gapi_handle handle = gapi_handle(_this);
    _Object     object = NULL;

    if ( handle != NULL ) {
        if ( handle->magic == MAGIC ) {
            gapi_handleReadClaim(handle);
            if ( HEADER_IS_TYPE(handle,kind) ) {
                if ( handle->object != NULL ) {
                    object = handle->object;
                }
            }
            gapi_handleReadRelease(handle);
        }
    }

    return object;
}
#endif

/*
 * gapi_objectPeekUnchecked returns the object of to the given handle.
 * This 'unchecked' version, does not check the type of the object, to
 * prevent the need to lock the object temporary.
 */
_Object
gapi_objectPeekUnchecked (
    gapi_object _this)
{
    gapi_handle handle = gapi_handle(_this);
    _Object     object = NULL;

    if ( handle != NULL ) {
        if ( handle->magic == MAGIC ) {
            if ( handle->object != NULL ) {
                object = handle->object;
            }
        }
    }

    return object;
}

gapi_object
_ObjectToHandle (
    _Object object)
{
    gapi_handle handle = NULL;

    if ( object != NULL ) {
        handle = (gapi_handle) object->handle;
        assert(handle);
        if ( handle->magic != MAGIC ) {
            OS_REPORT(OS_WARNING, "_ObjectToHandle", 0, "invalid handle detected");
            handle = NULL;
        }
    }
    return (gapi_object) handle;
}

void
_ObjectClaim (
    _Object object)
{
    gapi_handle handle;

    assert(object);
    handle = (gapi_handle) object->handle;
    assert(handle);
    gapi_handleClaim(handle);
}

void
_ObjectClaimNotBusy (
    _Object object)
{
    gapi_handle handle;

    assert(object);
    handle = (gapi_handle) object->handle;
    assert(handle);
    gapi_handleClaimNotBusy(handle);
}

gapi_object
_ObjectRelease (
    _Object object)
{
    gapi_handle handle = NULL;

    if ( object != NULL ) {
        handle = (gapi_handle) object->handle;
        if (handle) {
            if ( handle->magic == MAGIC ) {
                gapi_handleRelease(handle);
            }
        }
    }
    return (gapi_object) handle;
}

gapi_object
gapi_objectRelease (
    gapi_object _this)
{
    gapi_handle handle = gapi_handle(_this);

    if ( handle != NULL ) {
        if ( handle->magic == MAGIC ) {
            if ( handle->object != NULL ) {
                gapi_handleRelease(handle);
            }
        }
    }

    return handle;
}

#ifdef _RWLOCK_
void
_ObjectReadClaim (
    _Object object)
{
    gapi_handle handle;

    assert(object);
    handle = (gapi_handle) object->handle;
    assert(handle);
    gapi_handleReadClaim(handle);
}

void
_ObjectReadClaimNotBusy (
    _Object object)
{
    gapi_handle handle;

    assert(object);
    handle = (gapi_handle) object->handle;
    assert(handle);
    gapi_handleReadClaimNotBusy(handle);
}

gapi_object
_ObjectReadRelease (
    _Object object)
{
    gapi_handle handle = NULL;

    if ( object != NULL ) {
        handle = (gapi_handle) object->handle;
        assert(handle);
        gapi_handleReadRelease(handle);
    }
    return (gapi_object) handle;
}

gapi_object
gapi_objectReadRelease (
    gapi_object _this)
{
    gapi_handle handle = gapi_handle(_this);

    if ( handle != NULL ) {
        if ( handle->magic == MAGIC ) {
            if ( handle->object != NULL ) {
                gapi_handleReadRelease(handle);
            }
        }
    }

    return handle;
}
#endif

gapi_boolean
_ObjectIsValid (
    _Object object)
{
    gapi_boolean  valid  = FALSE;
    gapi_handle handle = (gapi_handle)object->handle;

    if ( handle != NULL ) {
        if ((handle->magic == MAGIC) &&
            (handle->object != NULL) &&
            (!handle->beingDeleted)) {
                valid = TRUE;
        }
    }
    return valid;
}

void
gapi_object_set_user_data (
    gapi_object _this,
    void *userData,
    gapi_delete_action deleteAction,
    void *deleteActionArg)
{
    gapi_handle handle = (gapi_handle) _this;

    if (gapi_handleClaim(handle) == GAPI_RETCODE_OK) {
        handle->userData = userData;
        handle->deleteActionInfo.action = deleteAction;
        handle->deleteActionInfo.argument = deleteActionArg;
        gapi_handleRelease(handle);
    }
}

void *
gapi_object_get_user_data (
    gapi_object _this)
{
    gapi_handle handle = (gapi_handle) _this;
    void *userData = NULL;

    if (gapi_handleClaim(handle) == GAPI_RETCODE_OK) {
        userData = handle->userData;
        gapi_handleRelease(handle);
    }
    return userData;
}

void *
_ObjectGetUserData (
    _Object object)
{
    gapi_handle handle;

    assert(object);
    assert(object->handle);

    handle = (gapi_handle)object->handle;

    return handle->userData;
}

void
_ObjectSetUserData (
    _Object object,
    void   *userData)
{
    gapi_handle handle;

    assert(object);
    assert(object->handle);

    handle = (gapi_handle)object->handle;

    handle->userData = userData;
}



void
_ObjectSetDeleteAction (
    _Object object,
    gapi_deleteEntityAction action,
    void *actionData)
{
    gapi_handle handle;

    assert(object);
    assert(object->handle);

    handle = (gapi_handle)object->handle;

    handle->deleteActionInfo.action   = action;
    handle->deleteActionInfo.argument = actionData;
}

gapi_boolean
_ObjectGetDeleteAction (
    _Object object,
    gapi_deleteEntityAction *action,
    void **actionData)
{
    gapi_boolean result = FALSE;
    gapi_handle handle;

    assert(object);
    assert(object->handle);

    handle = (gapi_handle)object->handle;

    if ( handle->deleteActionInfo.action ) {
        result = TRUE;
        *action = handle->deleteActionInfo.action;
        *actionData = handle->deleteActionInfo.argument;
    }

    return result;
}

void
_ObjectSetBusy (
    _Object object)
{
    gapi_handle handle;

    assert(object);
    assert(object->handle);

    handle = (gapi_handle)object->handle;

    assert(handle->magic == MAGIC);

    handle->busy = TRUE;
}

void
_ObjectClearBusy (
    _Object object)
{
    gapi_handle handle;

    assert(object);
    assert(object->handle);

    handle = (gapi_handle)object->handle;

    assert(handle->magic == MAGIC);

    if ( handle->busy ) {
        handle->busy = FALSE;
        os_condBroadcast(&handle->cv);
    }
}

void
gapi_objectClearBusy (
    gapi_object _this)
{
    gapi_handle handle = gapi_handle(_this);

    if (gapi_handleClaim(handle) == GAPI_RETCODE_OK) {
        if ( handle->busy ) {
            handle->busy = FALSE;
            os_condBroadcast(&handle->cv);
        }
        gapi_handleRelease(handle);
    }
}

_ObjectKind
gapi_objectGetKind(
    gapi_object _this)
{
    gapi_handle handle = gapi_handle(_this);
    _ObjectKind kind = OBJECT_KIND_UNDEFINED;

    assert(handle);

    if ( handle ) {
        if ( handle->magic == MAGIC ) {
            kind = handle->kind;
        }
    }

    return kind;
}

_ObjectKind
_ObjectGetKind(
    _Object object)
{
    gapi_handle handle;
    _ObjectKind kind = OBJECT_KIND_UNDEFINED;

    assert(object);

    handle = (gapi_handle)object->handle;
    if ( handle ) {
        if ( handle->magic == MAGIC ) {
            kind = handle->kind;
        }
    }

    return kind;
}

os_result
_ObjectWait(
    _Object object,
    os_cond *cv)
{
    gapi_handle handle;
    os_result result;

    assert(object);
    assert(object->handle);
    assert(cv != NULL);

    handle = (gapi_handle)object->handle;

    assert(handle->magic == MAGIC);

    result = os_condWait(cv, &handle->mutex);

    return result;
}

os_result
_ObjectTimedWait(
    _Object object,
    os_cond *cv,
    const os_time *timeout)
{
    gapi_handle handle;
    os_result result;

    assert(object);
    assert(object->handle);
    assert(cv != NULL);

    handle = (gapi_handle)object->handle;

    assert(handle->magic == MAGIC);

    result = os_condTimedWait(cv, &handle->mutex, timeout);

    return result;
}
