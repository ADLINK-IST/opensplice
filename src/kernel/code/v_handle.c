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
 * All handles are kept in a handle pool. Once a handle is allocated in memory it
 * will never be freed, this is because applications may refer to the address at any time
 * and expect to get a notification if the handle has become invalid and not run
 * into a free memory exception. The handle pool is implemented as a partly preallocated
 * two dimensional array. The number of allocated handles will be increased dynamically.
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

#define UT_TRACE(object, old_count, new_count) \
        ut_trace(object, old_count, new_count)

static void
ut_trace(
    v_object object,
    c_ulong old_count,
    c_ulong new_count)
{
    void *tr[CHECK_REF_DEPTH];
    char **strs;
    size_t s,i;
    FILE* stream;
 
    if (v_object(object)->kind == CHECK_REF_TYPE) {
        if(!CHECK_REF_FILE){
            CHECK_REF_FILE = os_malloc(16);
            os_sprintf(CHECK_REF_FILE, "handle.log");
        }
        s = backtrace(tr, CHECK_REF_DEPTH);
        strs = backtrace_symbols(tr, s);
        stream = fopen(CHECK_REF_FILE, "a");
        if (old_count < new_count) {
            fprintf(stream, "\n\nClaim   (0x%x): %d -> %d\n",
                    object, old_count, new_count);
        } else {
            fprintf(stream, "\n\nRelease (0x%x): %d -> %d\n",
                    object, old_count, new_count);
        }
        for (i=0;i<s;i++) fprintf(stream, "%s\n", strs[i]);
        free(strs);
        fflush(stream);
        fclose(stream);
    }
}
#else
#define UT_TRACE(object, old_count, new_count)
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
                      "kernel::v_handle::issueLowMemoryWarning",0,
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
            OS_REPORT(OS_ERROR,
                      "kernel::v_handle::v_handleServerNew",0,
                      "Failed to allocate handle info records");
        }
    } else {
        OS_REPORT(OS_ERROR,
                  "kernel::v_handle::v_handleServerNew",0,
                  "Failed to allocate v_handleServer object");
        assert(FALSE);
    }
    return server;
}

void
v_handleServerFree(
    v_handleServer server)
{
    if (server == NULL) {
        OS_REPORT(OS_ERROR,
                  "kernel::v_handle::v_handleServerFree",0,
                  "No server specified");
    }
    OS_REPORT(OS_WARNING,
              "kernel::v_handle::v_handleServerFree",0,
              "This operation is not yet implemented");
}

c_long
v_handleServerCount(
    v_handleServer server)
{
    if (server == NULL) {
        OS_REPORT(OS_ERROR,
                  "kernel::v_handle::v_handleServerCount",0,
                  "No server specified");
    }
    OS_REPORT(OS_WARNING,
              "kernel::v_handle::v_handleServerCount",0,
              "This operation is not yet implemented");
    return 0;
}

c_long
v_handleServerClaims(
    v_handleServer server)
{
    if (server == NULL) {
        OS_REPORT(OS_ERROR,
                  "kernel::v_handle::v_handleServerClaims",0,
                  "No server specified");
    }
    OS_REPORT(OS_WARNING,
              "kernel::v_handle::v_handleServerClaims",0,
              "This operation is not yet implemented");
    return 0;
}

c_iter
v_handleServerLookup(
    v_handleServer server,
    c_object o)
{
    if (server == NULL) {
        OS_REPORT(OS_ERROR,
                  "kernel::v_handle::v_handleServerLookup",0,
                  "No server specified");
    }
    if (o == NULL) {
        OS_REPORT(OS_ERROR,
                  "kernel::v_handle::v_handleServerLookup",0,
                  "No object specified");
    }
    OS_REPORT(OS_WARNING,
              "kernel::v_handle::v_handleServerLookup",0,
              "This operation not yet implemented");
    return NULL;
}

void
v_handleServerSuspend(
    v_handleServer server)
{
    if (server == NULL) {
        OS_REPORT(OS_ERROR,
                  "kernel::v_handle::v_handleServerSuspend",0,
                  "No server specified");
    } else {
        server->suspended = TRUE;
        OS_REPORT(OS_WARNING,
                  "kernel::v_handle::v_handleServerSuspend",0,
                  "Handle Server is suspended.");
    }
}

void
v_handleServerResume(
    v_handleServer server)
{
    if (server == NULL) {
        OS_REPORT(OS_ERROR,
                  "kernel::v_handle::v_handleServerResume",0,
                  "No server specified");
    } else {
        server->suspended = FALSE;
        OS_REPORT(OS_WARNING,
                  "kernel::v_handle::v_handleServerResume",0,
                  "Handle Server is resumed.");
    }
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
        /* Reuse a previously 'de-registered' handle.
         */
        idx = server->firstFree;
        block = ((v_handleInfo**)server->handleInfos)[COL(idx)];
        info = &block[ROW(idx)];
        server->firstFree = info->nextFree;
    } else {
        if (server->lastIndex == ((NROFCOL*NROFROW)-1)) {
            OS_REPORT(OS_ERROR,
                      "kernel::v_handle::v_handleServerRegister",0,
                      "The Handle Server ran out of handle space");
            c_mutexUnlock(&server->mutex);
            exit(-1);
        }
        /* Increase the last used handle index number for the new handle.
         */
        if (server->lastIndex == NOHANDLE) {
            server->lastIndex = 0;
        } else {
            server->lastIndex++;
        }
        idx = server->lastIndex;

        /* Get the new info record for the handle.
         */
        row = ROW(idx);
        if (row == 0) {
            type = c_resolve(c_getBase(o),"kernelModule::v_handleInfo");
            server->handleInfos[COL(idx)] = c_arrayNew(type,NROFROW);
        }
        block = ((v_handleInfo**)server->handleInfos)[COL(idx)];
        if (block) {
            info = &block[row];
            info->serial = SERIALSTART;
        } else {
            OS_REPORT(OS_ERROR,
                      "kernel::v_handle::v_handleServerRegister",0,
                      "Failed to allocate a new list of handles");
        }
    }

    /* Initialize the handle and associated info record.
     */
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

static void
v_handleServerDeregister (
    v_handleServer server,
    v_handleInfo *info)
{
    c_long idx;
    v_public o;

    c_mutexLock(&server->mutex);
    if (info->freed) {
        info->serial = (info->serial + 1) % MAXSERIAL;
        idx = info->nextFree;
        assert(idx != NOHANDLE);
        info->nextFree = server->firstFree;
        server->firstFree = idx;
        info->freed = FALSE;
        o = v_public(info->object);
        info->object = NULL;
    } else {
        o = NULL;
    }
    c_mutexUnlock(&server->mutex);
    v_publicDispose(o);
}

v_handleResult
v_handleDeregister(
    v_handle handle)
{
    v_handleResult result;
    v_handleServer server;
    v_handleInfo *info, *block;
    c_long idx;
    c_ulong count;

    /* First perform some checks to confirm handle validity.
     */
    server = v_handleServer((c_object)handle.server);
    if (server == NULL) {
#if 0 /* Following test can be re-inserted when ticket scdds2814 is resolved. */
        OS_REPORT(OS_ERROR,
                  "kernel::v_handle::v_handleDeregister",0,
                  "Operation aborted: Handle is invalid."
                  OS_REPORT_NL "Reason: Server address = NULL");
#endif
        return V_HANDLE_ILLEGAL;
    }
    idx = handle.index;
    if ((idx < 0) || (idx > server->lastIndex)) {
        OS_REPORT_1(OS_ERROR,
                    "kernel::v_handle::v_handleDeregister",0,
                    "Operation aborted: Handle is invalid."
                    OS_REPORT_NL "Reason: handle index is out of range (%d).",
                    idx);
        return V_HANDLE_ILLEGAL;
    }

    /* The handle is valid so retrieve the handle's info record.
     */
    result = V_HANDLE_OK;
    block = ((v_handleInfo**)server->handleInfos)[COL(idx)];
    info = &block[ROW(idx)];

    /* Increase the protect count to protect the handle against invalidation
     * and if handle lifecycle is valid then safely set the 'freed' status.
     * Any subsequent operation on this 'freed' handle that decreases the
     * protect count to zero will invalidate this handle.
     */
    count = pa_increment(&info->count);
    assert(count > 0);
    UT_TRACE(info->object, count-1, count);

    c_mutexLock(&server->mutex);
    if (handle.serial == info->serial) {
        /* The lifecycle of the handle is correct so now invalidate the
         * handle.
         */
        if (!info->freed) {
            info->freed = TRUE;
            info->nextFree = idx; /* temporary reminder of the index to be freed */
        }
    } else {
        if (handle.serial < info->serial) {
            result = V_HANDLE_EXPIRED;
        } else {
            result = V_HANDLE_ILLEGAL;
        }
    }
    c_mutexUnlock(&server->mutex);

    count = pa_decrement(&info->count);
    assert(count+1 > count);
    UT_TRACE(info->object, count+1, count);

    /* Accidental dirty read on info->freed is acceptable.
     * Is corrected later on when protected by server mutex.
     */
    if ((count == 0) && (info->freed)) {
        /* At the previous pa_decrement the handle was invalid but
         * no not yet deregistered so deregister now!
         */
        v_handleServerDeregister(server, info);
    }
    return result;
}

v_handleResult
v_handleClaim (
    v_handle handle,
    v_object *o)
{
    v_handleResult result;
    v_handleServer server;
    v_handleInfo *info, *block;
    c_long idx;
    c_ulong count;

    /* First perform some checks to confirm handle validity.
     */
    server = v_handleServer((c_object)handle.server);
    if (server == NULL) {
#if 0 /* Following test can be re-inserted when ticket scdds2814 is resolved. */
        OS_REPORT(OS_ERROR,
                  "kernel::v_handle::v_handleClaim",0,
                  "Operation aborted: Handle is invalid."
                  OS_REPORT_NL "Reason: Server address = NULL");
#endif
        *o = NULL;
        return V_HANDLE_ILLEGAL;
    }
    if(server->suspended == TRUE) {
        /* For now the suspended state means that an unrecoverable error has
         * occured. This implies that from now on any access to the kernel is
         * unsafe and the result is undefined.
         * So because of this skip this action and return NULL.
         */
        *o = NULL;
        return V_HANDLE_SUSPENDED;
    }
    idx = handle.index;
    if ((idx < 0) || (idx > server->lastIndex)) {
        OS_REPORT_1(OS_ERROR,
                    "kernel::v_handle::v_handleClaim",0,
                    "Operation aborted: Handle is invalid."
                    OS_REPORT_NL "Reason: handle index is out of range (%d).",
                    idx);
        *o = NULL;
        return V_HANDLE_ILLEGAL;
    }

    /* The handle is valid so retrieve the handle's info record.
     */
    result = V_HANDLE_OK;
    block = ((v_handleInfo**)server->handleInfos)[COL(idx)];
    info = &block[ROW(idx)];

    /* Increase the protect count to protect the handle against invalidation.
     * The handle can only be invalidated if count is zero.
     */
    count = pa_increment(&info->count);
    assert(count > 0);
    UT_TRACE(info->object, count-1, count);

    if (handle.serial == info->serial) {
        if (info->freed) { /* Too late, it is already being freed... */
            result = V_HANDLE_EXPIRED;
            *o = NULL;
        } else {
            count = pa_increment(&info->count);
            UT_TRACE(info->object, count-1, count);
            *o = (v_object)info->object;
            if(*o == NULL)
            {
                OS_REPORT(OS_WARNING,
                          "kernel::v_handle::v_handleClaim", 0,
                          "Unable to obtain kernel entity for handle");
                result = V_HANDLE_ILLEGAL;
            }
        }
    } else {
        if (handle.serial < info->serial) {
            result = V_HANDLE_EXPIRED;
        } else {
            result = V_HANDLE_ILLEGAL;
        }
        *o = NULL;
    }
    count = pa_decrement(&info->count);
    assert(count+1 > count);
    UT_TRACE(info->object, count+1, count);

    /* Accidental dirty read on info->freed is acceptable.
     * Is corrected later on when protected by server mutex.
     */
    if ((count == 0) && (info->freed)) {
        /* At the previous pa_decrement the handle was invalid but
         * no not yet deregistered so deregister now!
         */
        v_handleServerDeregister(server, info);
    }

    return result;
}

v_handleResult
v_handleRelease (
    v_handle handle)
{
    v_handleResult result;
    v_handleServer server;
    v_handleInfo *info, *block;
    c_long idx;
    c_ulong count;

    /* First perform some checks to confirm handle validity.
     */
    server = v_handleServer((c_object)handle.server);
    if (server == NULL) {
#if 0 /* Following test can be re-inserted when ticket scdds2814 is resolved. */
        OS_REPORT(OS_ERROR,
                  "kernel::v_handle::v_handleRelease",0,
                  "Operation aborted: Handle is invalid."
                  OS_REPORT_NL "Reason: Server address = NULL");
#endif
        return V_HANDLE_ILLEGAL;
    }
    idx = handle.index;
    if ((idx < 0) || (idx > server->lastIndex)) {
        OS_REPORT_1(OS_ERROR,
                    "kernel::v_handle::v_handleRelease",0,
                    "Operation aborted: Handle is invalid."
                    OS_REPORT_NL "Reason: handle index is out of range (%d).",
                    idx);
        return V_HANDLE_ILLEGAL;
    }

    /* The handle is valid so retrieve the handle's info record.
     */
    result = V_HANDLE_OK;
    block = ((v_handleInfo**)server->handleInfos)[COL(idx)];
    info = &block[ROW(idx)];

    /* Increase the protect count to protect the handle against invalidation.
     * The handle can only be invalidated if count is zero.
     */
    count = pa_increment(&info->count);
    assert(count > 0);
    UT_TRACE(info->object, count-1, count);

    if (handle.serial == info->serial) {
        count = pa_decrement(&info->count);
        assert(count+1 > count);
        UT_TRACE(info->object, count+1, count);
    } else {
        if (handle.serial < info->serial) {
            result = V_HANDLE_EXPIRED;
        } else {
            result = V_HANDLE_ILLEGAL;
        }
    }
    count = pa_decrement(&info->count);
    assert(count+1 > count);
    UT_TRACE(info->object, count+1, count);

    /* Accidental dirty read on info->freed is acceptable.
     * Is corrected later on when protected by server mutex.
     */
    if ((count == 0) && (info->freed)) {
        /* At the previous pa_decrement the handle was invalid but
         * no not yet deregistered so deregister now!
         */
        v_handleServerDeregister(server, info);
    }

    return result;
}
