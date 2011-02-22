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
#include "v_handle.h"
#include "v_entity.h"
#include "v_public.h"
#include "os_report.h"
#include "os_abstract.h"

/**
 * All handles are kept in a partly preallocated two dimensional array.
 * The following MACRO's are defined wich specify the maximum dimensions of the array in
 * terms of columns and rows. Rows are dynamically allocated when required.
 * Handles are identified by an index and serial number. The index specifies the column and
 * row number, the serial number specifies the generation count af a handle.
 * The generation count is increased everytime a handle is freed, The use of handles where
 * the serial number is older that the current serial value is not granted, these handles
 * are considered invalid.
 */
#define NROFCOL  (4096)
#define NROFROW  (1024)
/**
 * The following two macro's specify the column and row number for a given handle index.
 */
#define COL(index) (index / NROFROW)
#define ROW(index) (index % NROFROW)
/*
 * This MARCRO specifies the value of an illegal handle.
 */
#define NOHANDLE (-1)
/*
 * This MARCRO specifies the non zero value as first serial value.
 */
#define SERIALSTART (1)
#define MAXSERIAL   (0x7fffffff)

const v_handle V_HANDLE_NIL = {0, 0, 0};

#define CHECK_REF (0)

#if CHECK_REF
#define CHECK_REF_TYPE (43)
#define CHECK_REF_DEPTH (64)
static char* CHECK_REF_FILE = NULL;

#define UT_TRACE(msgFormat, ...) do { \
    void *tr[CHECK_REF_DEPTH];\
    char **strs;\
    size_t s,i; \
    FILE* stream; \
    \
    if(!CHECK_REF_FILE){ \
        CHECK_REF_FILE = os_malloc(16); \
        os_sprintf(CHECK_REF_FILE, "handle.log"); \
    } \
    s = backtrace(tr, CHECK_REF_DEPTH);\
    strs = backtrace_symbols(tr, s);\
    stream = fopen(CHECK_REF_FILE, "a");\
    fprintf(stream, msgFormat, __VA_ARGS__);              \
    for (i=0;i<s;i++) fprintf(stream, "%s\n", strs[i]);\
    free(strs);\
    fflush(stream);\
    fclose(stream);\
  } while (0)
#endif

static void
disableHandleServer(
    c_voidp server)
{
    v_handleServerSuspend(server);
}

static void
issueLowMemoryWarning(
    c_voidp arg)
{
#ifdef DDS_1958_CANNOT_CALL_REGISTERED_FUNC_PTR_FROM_DIFF_PROCESS
    os_uint32 warningCount;
    v_handleServer server;

    server = v_handleServer(arg);
    /* dds1958: ES: Check if the warning count is 0 at the moment. If so it
     * means that no warning has been issued. If the value is not 0 however
     * then we do not need to continue and do not need to do any increment
     * and safe out on that code in situations where we get the low memory
     * warning a lot. The idea is that just doing this check (although not
     * a definate yes or no to doing the warning) is in the cases where
     * a warning has already been issued much cheaper then doing the
     * increment and then checking. Only in the situation where the warning
     * is issued for the first time, is this check useless. But that is only
     * 1 time vs many times.
     */
    if(server->lowMemWarningCount == 0)
    {
        /*  increment the warning count
         */
        warningCount = pa_increment(&server->lowMemWarningCount);
        if(warningCount == 1)
        {
            OS_REPORT(OS_WARNING,
                "issueLowMemoryWarning",0,
                "Shared memory is running very low!");

        }
    }
#endif
}

v_handleServer
v_handleServerNew (
    c_base base)
{
    v_handleServer server;
    c_type type;

    assert(base != NULL);

    type = c_resolve(base,"kernelModule::v_handleServer");
    server = c_new(type);
    c_free(type);
    if (server) {
        type = c_resolve(base,"kernelModule::v_handleInfoList");
        server->handleInfos = c_arrayNew(type,NROFCOL);
#ifdef DDS_1958_CANNOT_CALL_REGISTERED_FUNC_PTR_FROM_DIFF_PROCESS
        server->lowMemWarningCount = 0;
#endif
        c_free(type);
        if (server->handleInfos) {
            server->firstFree = NOHANDLE;
            server->lastIndex = NOHANDLE;
            server->suspended = FALSE;
            c_mutexInit(&server->mutex,SHARED_MUTEX);
            c_baseOnOutOfMemory(base, disableHandleServer,server);
            c_baseOnLowOnMemory(base, issueLowMemoryWarning, server);
        } else {
            c_free(server);
            server = NULL;
            OS_REPORT(OS_ERROR,"v_handleServerNew",0,
                      "Failed to allocate handle info records");
        }
    } else {
        OS_REPORT(OS_ERROR,"v_handleServerNew",0,
                  "Failed to allocate handle server");
    }
    return server;
}

void
v_handleServerFree(
    v_handleServer server)
{
    if (server == NULL) {
        OS_REPORT(OS_ERROR,"Kernel HandleServer",0,
                  "v_handleServerFree: no server specified");
    }
    OS_REPORT(OS_WARNING,"Kernel HandleServer",0,
              "v_handleServerFree not yet implemented");
}

c_long
v_handleServerCount(
    v_handleServer server)
{
    if (server == NULL) {
        OS_REPORT(OS_ERROR,"Kernel HandleServer",0,
                  "v_handleServerCount: no server specified");
    }
    OS_REPORT(OS_WARNING,"Kernel HandleServer",0,
              "v_handleServerCount not yet implemented");
    return 0;
}

c_long
v_handleServerClaims(
    v_handleServer server)
{
    if (server == NULL) {
        OS_REPORT(OS_ERROR,"Kernel HandleServer",0,
                           "v_handleServerClaims: no server specified");
    }
    OS_REPORT(OS_WARNING,"Kernel HandleServer",0,
              "v_handleServerClaims not yet implemented");
    return 0;
}

c_iter
v_handleServerLookup(
    v_handleServer server,
    c_object o)
{
    if (server == NULL) {
        OS_REPORT(OS_ERROR,"Kernel HandleServer",0,
                           "v_handleServerLookup: no server specified");
    }
    if (o == NULL) {
        OS_REPORT(OS_ERROR,"Kernel HandleServer",0,
                           "v_handleServerLookup: no object specified");
    }
    OS_REPORT(OS_WARNING,"Kernel HandleServer",0,
              "v_handleServerLookup not yet implemented");
    return NULL;
}

void
v_handleServerSuspend(
    v_handleServer server)
{
    if (server == NULL) {
        OS_REPORT(OS_ERROR,"Kernel HandleServer",0,
                           "v_handleServerSuspend: no server specified");
    } else {
        server->suspended = TRUE;
        OS_REPORT(OS_WARNING,"Kernel HandleServer",0,"v_handleServer is suspended.");
    }
}

void
v_handleServerResume(
    v_handleServer server)
{
    if (server == NULL) {
        OS_REPORT(OS_ERROR,"Kernel HandleServer",0,
                           "v_handleServerResume: no server specified");
    } else {
        server->suspended = FALSE;
        OS_REPORT(OS_WARNING,"Kernel HandleServer",0,"v_handleServer is resumed.");
    }
}

/**
 * \brief The HandleServer get handle info method.
 *
 * This private method retrieves the handle info structure for the specified handle.
 * A infor record is only returned for a valid handle. The returned handle info record is locked
 * before returned so it cannot be modified until the caller unlocks it.
 */
static v_handleResult
v_handleServerInfo(
    v_handle handle,
    v_handleInfo **info)
{
    v_handleServer server;
    c_long idx;
    v_handleInfo *block;
    v_handleResult result;

    server = v_handleServer((c_object)handle.server);
    if (server == NULL) {

        *info = NULL;
        return V_HANDLE_ILLEGAL;
    }
#if 0
    if(server->suspended == TRUE) {
        *info = NULL;
        return V_HANDLE_SUSPENDED;
    }
#endif
    idx = handle.index;
    if ((idx < 0) || (idx > server->lastIndex)) {
        *info = NULL;
        return V_HANDLE_ILLEGAL;
    }
    block = ((v_handleInfo**)server->handleInfos)[COL(idx)];
    *info = &block[ROW(idx)];
    c_mutexLock(&(*info)->mutex);
    if (handle.serial != (*info)->serial) {
        if (handle.serial < (*info)->serial) {
            result = V_HANDLE_EXPIRED;
        } else {
            result = V_HANDLE_ILLEGAL;
        }
        c_mutexUnlock(&(*info)->mutex);
        *info = NULL;
        return result;
    }
    return V_HANDLE_OK;
}

v_handle
v_handleServerRegister(
    v_handleServer server,
    c_object o)
{
    c_type type;
    v_handle handle;
    v_handleInfo *info, *block;
    c_long row, idx;

    assert(C_TYPECHECK(server,v_handleServer));
    assert(o != NULL);

    if(server->suspended == TRUE) {
        /* For now the suspended state means that an unrecoverable error has
         * occured. This implies that from now on any access to the kernel is
         * unsafe and the result is undefined.
         * So because of this and that registering new object are useless
         * skip this action and return NULL.
         */
        return V_HANDLE_NIL;
    }

    c_mutexLock(&server->mutex);

    if (server->firstFree != NOHANDLE) {
        idx = server->firstFree;
        block = ((v_handleInfo**)server->handleInfos)[COL(idx)];
        info = &block[ROW(idx)];
        server->firstFree = info->nextFree;
    } else {
        if (server->lastIndex == ((NROFCOL*NROFROW)-1)) {
            OS_REPORT(OS_ERROR,"Kernel v_handle",0,"Out of handle space");
            c_mutexUnlock(&server->mutex);

            exit(-1);
        }
        if (server->lastIndex == NOHANDLE) {
            server->lastIndex = 0;
        } else {
            server->lastIndex++;
        }
        idx = server->lastIndex;
        row = ROW(idx);
        if (row == 0) {
            type = c_resolve(c_getBase(o),"kernelModule::v_handleInfo");
            server->handleInfos[COL(idx)] = c_arrayNew(type,NROFROW);
        }
        block = ((v_handleInfo**)server->handleInfos)[COL(idx)];
        if (block) {
            info = &block[row];
            info->serial = SERIALSTART;
            c_mutexInit(&info->mutex,SHARED_MUTEX);
        } else {
            OS_REPORT(OS_ERROR,"v_handleServerRegister",0,
                      "Failed to allocate a new list of handles");
        }
    }

    if (info) {
        info->object   = c_keep(o);
        info->nextFree = NOHANDLE;
        info->count    = 0;
        info->freed    = FALSE;

        handle.server  = (c_address)server;
        handle.serial  = info->serial;
        handle.index   = idx;
    } else {
        handle  = V_HANDLE_NIL;
    }

    c_mutexUnlock(&server->mutex);

    return handle;
}

/**
 * This method will invalidate a handle and mark the resources as ready for reuse.
 * Note that the info and handle musr correspond and that info is locked.
 */
static void
v_handleInvalidate (
    v_handle handle,
    v_handleInfo *info)
{
    v_handleServer server;
    c_object entity;

    server = v_handleServer((c_object)handle.server);
    assert(C_TYPECHECK(server,v_handleServer));
    if (server) {
        c_mutexLock(&server->mutex);
        info->nextFree = server->firstFree;
        server->firstFree = handle.index;
        info->serial = (info->serial + 1) % MAXSERIAL;
        entity = info->object;
        info->object = NULL;
        c_mutexUnlock(&server->mutex);
        c_mutexUnlock(&info->mutex);
        v_publicDispose(v_public(entity));
    }
}

v_handleResult
v_handleDeregister(
    v_handle handle)
{
    v_handleInfo *info;
    v_handleResult result;

    result = v_handleServerInfo(handle, &info);
    if (result == V_HANDLE_OK) {
        assert(info != NULL);
        if (info->count > 0) {
            /* The handle can not be invalidated yet because it is in use.
             * so set the free attribute, the v_handleRelease method will
             * check this flag and invalidate the handle as soon as it is
             * no longer used.
             */
            info->freed = TRUE;
            c_mutexUnlock(&info->mutex);
        } else {
            v_handleInvalidate(handle,info);
        }
    }

    return result;
}

v_handleResult
v_handleRenew(
    v_handle *handle)
{
    v_handleServer server;
    v_handleInfo *info;
    c_long idx;
    v_handleInfo *block;

    server = v_handleServer((c_object)handle->server);
    if (server == NULL) {
        return V_HANDLE_ILLEGAL;
    }
    assert(C_TYPECHECK(server,v_handleServer));
    c_mutexLock(&server->mutex);
    if(server->suspended == TRUE) {
        return V_HANDLE_SUSPENDED;
    }
    idx = handle->index;
    if ((idx < 0) || (idx > server->lastIndex)) {
        return V_HANDLE_ILLEGAL;
    }
    block = ((v_handleInfo**)server->handleInfos)[COL(idx)];
    info = &block[ROW(idx)];
    c_mutexLock(&info->mutex);
    c_mutexUnlock(&server->mutex);
    info->count = 0;
    info->serial = (info->serial + 1) % MAXSERIAL;
    handle->serial = info->serial;
    c_mutexUnlock(&info->mutex);

    return V_HANDLE_OK;
}

v_handleResult
v_handleClaim (
    v_handle handle,
    v_object *o)
{
    v_handleInfo *info;
    v_handleResult result;
    v_handleServer server;

    server = v_handleServer((c_object)handle.server);
    if (server == NULL) {
        return V_HANDLE_ILLEGAL;
    }
    if(server->suspended == TRUE) {
        /* For now the suspended state means that an unrecoverable error has
         * occured. This implies that from now on any access to the kernel is
         * unsafe and the result is undefined.
         * So because of this skip this action and return NULL.
         */
        return V_HANDLE_SUSPENDED;
    }

    result = v_handleServerInfo(handle, &info);
    if (result != V_HANDLE_OK) {
        *o = NULL;
        return result;
    }
    if (info->freed) { /* Too late, it is already being freed... */
        result = V_HANDLE_EXPIRED;
        *o = NULL;
    } else {
        info->count++;

#if CHECK_REF
        if (v_object(info->object)->kind == CHECK_REF_TYPE) {
                UT_TRACE("\n\n============ Claim (0x%x): %d -> %d =============\n",
                                info->object, info->count -1, info->count);
        }
#endif
        *o = (v_object)info->object;
        if(*o == NULL)
        {
            OS_REPORT(OS_WARNING, "v_handleClaim", 0,
                        "Unable to obtain kernel entity for handle");
            result = V_HANDLE_ILLEGAL;
        }
    }
    c_mutexUnlock(&info->mutex);
    return result;
}


v_handleResult
v_handleRelease (
    v_handle handle)
{
    v_handleInfo *info;
    v_handleResult result;

    result = v_handleServerInfo(handle, &info);
    if (result == V_HANDLE_OK) {
        assert(info != NULL);
        assert(info->count > 0);
        info->count--;

#if CHECK_REF
        if (v_object(info->object)->kind == CHECK_REF_TYPE) {
                UT_TRACE("\n\n=========== Release (0x%x): %d -> %d ============\n",
                                info->object, info->count+1, info->count);
        }
#endif
        if (info->count == 0) {
            if (info->freed) {
                /* at this point the handle was deregistered but could not
                 * be invalidated because it was claimed, so it must be invalidated now.
                 */
                v_handleInvalidate(handle,info);
            } else {
                c_mutexUnlock(&info->mutex);
            }
        } else {
            c_mutexUnlock(&info->mutex);
        }
    }
    return result;
}

#if CHECK_REF
#undef UT_TRACE
#endif
