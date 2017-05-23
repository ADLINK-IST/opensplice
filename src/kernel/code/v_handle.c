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
#include "v_handle.h"
#include "v_entity.h"
#include "v_public.h"
#include "os_report.h"
#include "os_abstract.h"
#include "os_atomics.h"

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
#define COL(index) ((index) / NROFROW)
#define ROW(index) ((index) % NROFROW)
/*
 * This macro specifies the non zero value as first serial value.
 */
#define MAX_SERIAL   (0x00ffffff)

const v_handle V_HANDLE_NIL = {0, 0, 0};

#define STATUS_DEREGISTERING     0x80000000
#define STATUS_ON_FREELIST       0x40000000
#define STATUS_COUNT_INDEX_MASK  0x0fffffff
#if STATUS_COUNT_INDEX_MASK < NROFCOL * NROFROW - 1
#error "STATUS_COUNT_INDEX_MASK leaves insufficient bits to represent all valid indices"
#endif

static int
new_row(
    v_handleServer server,
    c_ulong col)
{
    if ((server->handleInfos[col] = c_arrayNew(server->handleInfo_type, NROFROW)) != NULL) {
        return 1;
    } else {
        OS_REPORT(OS_FATAL,
                  "kernel::v_handle::v_handleServerRegister",V_RESULT_INTERNAL_ERROR,
                  "Failed to allocate a new list of handles");
        return 0;
    }
}

v_handleServer
v_handleServerNew (
    c_base base)
{
    v_handleServer server;
    c_type type;

    assert(base != NULL);

    type = c_resolve(base,"kernelModuleI::v_handleServer");
    server = c_new(type);
    c_free(type);
    if (server) {
        type = c_resolve(base,"kernelModuleI::v_handleInfoList");
        server->handleInfos = c_arrayNew(type,NROFCOL);
        c_free(type);
        if (server->handleInfos) {
            server->freeListLength = 0;
            server->lastFree = NULL;
            server->firstFree = NULL;
            server->lastIndex = 0;
            server->handleInfo_type = c_resolve(base, "kernelModuleI::v_handleInfo");
            if (!new_row(server, 0)) {
                c_free(server);
                server = NULL;
                OS_REPORT(OS_FATAL,
                          "kernel::v_handle::v_handleServerNew",V_RESULT_INTERNAL_ERROR,
                          "Failed to allocate handle info records");
            } else {
                c_mutexInit(c_getBase(server), &server->mutex);
            }
        } else {
            c_free(server);
            server = NULL;
            OS_REPORT(OS_FATAL,
                      "kernel::v_handle::v_handleServerNew",V_RESULT_INTERNAL_ERROR,
                      "Failed to allocate handle info records");
        }
    } else {
        OS_REPORT(OS_FATAL,
                  "kernel::v_handle::v_handleServerNew",V_RESULT_INTERNAL_ERROR,
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
                  "kernel::v_handle::v_handleServerFree",V_RESULT_ILL_PARAM,
                  "No server specified");
    }
    OS_REPORT(OS_WARNING,
              "kernel::v_handle::v_handleServerFree",V_RESULT_OK,
              "This operation is not yet implemented");
}

c_long
v_handleServerCount(
    v_handleServer server)
{
    if (server == NULL) {
        OS_REPORT(OS_ERROR,
                  "kernel::v_handle::v_handleServerCount",V_RESULT_ILL_PARAM,
                  "No server specified");
    }
    OS_REPORT(OS_WARNING,
              "kernel::v_handle::v_handleServerCount",V_RESULT_OK,
              "This operation is not yet implemented");
    return 0;
}

c_long
v_handleServerClaims(
    v_handleServer server)
{
    if (server == NULL) {
        OS_REPORT(OS_ERROR,
                  "kernel::v_handle::v_handleServerClaims",V_RESULT_ILL_PARAM,
                  "No server specified");
    }
    OS_REPORT(OS_WARNING,
              "kernel::v_handle::v_handleServerClaims",V_RESULT_OK,
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
                  "kernel::v_handle::v_handleServerLookup",V_RESULT_ILL_PARAM,
                  "No server specified");
    }
    if (o == NULL) {
        OS_REPORT(OS_ERROR,
                  "kernel::v_handle::v_handleServerLookup",V_RESULT_ILL_PARAM,
                  "No object specified");
    }
    OS_REPORT(OS_WARNING,
              "kernel::v_handle::v_handleServerLookup",V_RESULT_OK,
              "This operation not yet implemented");
    return NULL;
}

static v_handleInfo *
info_from_idx(
    v_handleServer server,
    c_ulong idx)
{
    v_handleInfo *block, *info;
    block = ((v_handleInfo **) server->handleInfos)[COL(idx)];
    assert(block != NULL);
    info = &block[ROW(idx)];
    return info;
}

static void
init_info(
    v_handleInfo *info)
{
    assert(pa_ld32 (&info->status_count_index) == 0);
    assert(info->serial == 0);
    info->serial = 1;
}

static v_handleInfo *
fresh_info_from_idx(
    v_handleServer server,
    c_ulong idx)
{
    v_handleInfo *info = info_from_idx(server, idx);
    init_info(info);
    return info;
}

static v_handleInfo *
fresh_info_from_new_column(
    v_handleServer server,
    c_ulong idx)
{
    assert(server->handleInfos[COL(idx)] == NULL);
    if (new_row(server, COL(idx))) {
        v_handleInfo *info = info_from_idx(server, idx);
        init_info(info);
        return info;
    } else {
        return NULL;
    }
}

static v_handleInfo *
info_from_freelist(
    v_handleServer server,
    c_ulong *pidx)
{
    c_ulong idx;
    v_handleInfo *info;
    assert(server->firstFree != NULL && server->lastFree != NULL);
    assert(server->freeListLength > 0);

    info = server->firstFree;
    server->firstFree = (void *) info->object_nextFree;
    server->freeListLength--;

    assert(pa_ld32 (&info->status_count_index) & STATUS_ON_FREELIST);
    assert(info->serial != 0);

    idx = pa_ld32 (&info->status_count_index) & STATUS_COUNT_INDEX_MASK;

    assert(0 < idx && idx < NROFROW * NROFCOL);
    assert(info_from_idx(server, idx) == info);
    *pidx = idx;
    return info;
}

v_handle
v_handleServerRegister(
    v_handleServer server,
    c_object o)
{
    v_handle handle;
    v_handleInfo *info;
    c_ulong idx;

    assert(C_TYPECHECK(server,v_handleServer));
    assert(o != NULL);

    c_mutexLock(&server->mutex);

    if (ROW(server->lastIndex) < NROFROW-1) {
        /* Can allocate a fresh index without allocating a new column */
        idx = ++server->lastIndex;
        info = fresh_info_from_idx(server, idx);
    } else if (server->freeListLength < NROFROW && COL(server->lastIndex) < NROFCOL-1) {
        /* Free list is getting short, but we can add another column (this also
           covers the case of an empty freelist) */
        assert(ROW(server->lastIndex) == NROFROW - 1);
        idx = ++server->lastIndex;
        if ((info = fresh_info_from_new_column(server, idx)) == NULL) {
            --server->lastIndex;
            c_mutexUnlock(&server->mutex);
            return V_HANDLE_NIL;
        }
    } else if (server->freeListLength > 0) {
        info = info_from_freelist(server, &idx);
    } else {
        assert(server->firstFree == NULL);
        assert(server->lastIndex == (NROFCOL*NROFROW)-1);
        OS_REPORT(OS_FATAL,
                  "kernel::v_handle::v_handleServerRegister",V_RESULT_OUT_OF_RESOURCES,
                  "The Handle Server ran out of handle space");
        c_mutexUnlock(&server->mutex);
        return V_HANDLE_NIL;
    }

    /* Initialize the handle and associated info record. */
    info->object_nextFree = (c_address) c_keep(o);

    /* memory barrier here will ensure that any accidental
       handleClaim (we haven't advertised the handle, but in case
       somehow it gets fed random garbage) will fail because
       status_count still has ON_FREELIST set until object_nextFree
       properly points to the object. */
    pa_fence ();
    pa_st32 (&info->status_count_index, 0);

    handle.server = (c_address)server;
    handle.index  = idx;
    handle.serial = info->serial;

    c_mutexUnlock(&server->mutex);
    return handle;
}

static v_handleResult
get_server_info (
    v_handleServer *server,
    v_handleInfo **info,
    v_handle handle)
{
    v_handleInfo *block;

    /* Valid index in [1,NROFCOL * NROFROW), so index-1 in [0,NROFCOL*NROFROW-1).
     * Unsigned arithmetic allows using underflow to end up with a single test.
     */
    if (handle.index - 1 >= NROFCOL * NROFROW - 1) {
        OS_REPORT(OS_ERROR, "OSPL-Core", V_HANDLE_ILLEGAL, "Detected invalid handle");
        return V_HANDLE_ILLEGAL;
    }
    *server = v_handleServer ((c_object) handle.server);
    if (*server == NULL) {
        OS_REPORT(OS_ERROR, "OSPL-Core", V_HANDLE_ILLEGAL, "Detected invalid handle");
        return V_HANDLE_ILLEGAL;
    }
    if ((block = ((v_handleInfo**) (*server)->handleInfos)[COL(handle.index)]) == NULL) {
        OS_REPORT(OS_ERROR, "OSPL-Core", V_HANDLE_ILLEGAL, "Detected invalid handle");
        return V_HANDLE_ILLEGAL;
    }
    *info = &block[ROW(handle.index)];
    return V_HANDLE_OK;
}

static void
free_handle (
    v_handleServer server,
    v_handleInfo *info,
    c_ulong idx)
{
    v_public o;

    c_mutexLock(&server->mutex);
    assert (pa_ld32 (&info->status_count_index) == STATUS_DEREGISTERING);
    o = v_public((v_object) info->object_nextFree);

    info->serial = (info->serial == MAX_SERIAL) ? 1 : (info->serial + 1);
    pa_st32 (&info->status_count_index, STATUS_ON_FREELIST | idx);
    info->object_nextFree = (c_address) NULL;

    if (server->firstFree == NULL) {
        assert(server->freeListLength == 0);
        server->firstFree = server->lastFree = info;
    } else {
        v_handleInfo *linfo = server->lastFree;
        assert(server->freeListLength > 0);
        linfo->object_nextFree = (c_address) info;
        server->lastFree = info;
    }
    server->freeListLength++;
    c_mutexUnlock(&server->mutex);

    v_publicDispose(o);
}

static void
release_int (
    v_handleServer server,
    v_handleInfo *info,
    c_ulong idx)
{
    /* GCC atomic builtins imply barriers => whatever had been done
       before release_int will be visible in time.  v_handleDeregister
       marks the handle as DEREGISTERING and relies on release_int to
       put it on the freelist and dispose the object */
    assert (!(pa_ld32 (&info->status_count_index) & STATUS_ON_FREELIST));
    if (pa_dec32_nv (&info->status_count_index) == STATUS_DEREGISTERING) {
        free_handle (server, info, idx);
    }
}

static v_handleResult
claim_int (
    v_handleServer server,
    v_handleInfo *info,
    v_handle handle)
{
    c_ulong status_count;

retry:
    status_count = pa_ld32 (&info->status_count_index);
    if (status_count & (STATUS_DEREGISTERING | STATUS_ON_FREELIST)) {
        /* can't trust serials unless we first lock server->mutex, but
           then again, who cares whether it is EXPIRED or INVALID
           anyway? */
        return V_HANDLE_EXPIRED;
    } else {
        c_ulong new_status_count = status_count + 1;
        if (!pa_cas32 (&info->status_count_index, status_count, new_status_count)) {
            goto retry;
        }
    }

    /* successfully incremented refcount => serial is stable */
    if (handle.serial == info->serial) {
        return V_HANDLE_OK;
    } else {
        /* Incremented refcount, but for the wrong handle => decide
           whether it is expired or illegal, then immediately release
           it */
        v_handleResult result = (handle.serial < info->serial && handle.serial != 0) ? V_HANDLE_EXPIRED : V_HANDLE_ILLEGAL;
        release_int (server, info, handle.index);
        return result;
    }
}

v_handleResult
v_handleDeregister(
    v_handle handle)
{
    v_handleResult result;
    v_handleServer server;
    v_handleInfo *info;
    c_ulong status_count;

    if ((result = get_server_info (&server, &info, handle)) != V_HANDLE_OK) {
        return result;
    }
    if ((result = claim_int (server, info, handle)) != V_HANDLE_OK) {
        return result;
    }

retry:
    status_count = pa_ld32 (&info->status_count_index);
    if (!(status_count & STATUS_DEREGISTERING)) {
        c_ulong new_status_count = status_count | STATUS_DEREGISTERING;
        if (!pa_cas32 (&info->status_count_index, status_count, new_status_count)) {
            goto retry;
        }
    }
    release_int (server, info, handle.index);

    return V_HANDLE_OK;
}

v_handleResult
v_handleClaim (
    v_handle handle,
    v_object *o)
{
    v_handleResult result;
    v_handleServer server;
    v_handleInfo *info;

    if ((result = get_server_info (&server, &info, handle)) != V_HANDLE_OK) {
        *o = NULL;
        return result;
    }
    if ((result = claim_int (server, info, handle)) != V_HANDLE_OK) {
        *o = NULL;
        return result;
    }
    assert (info->object_nextFree != 0);
    *o = (v_object) info->object_nextFree;
    return V_HANDLE_OK;
}

v_handleResult
v_handleRelease (
    v_handle handle)
{
    v_handleResult result;
    v_handleServer server;
    v_handleInfo *info;

    if ((result = get_server_info (&server, &info, handle)) != V_HANDLE_OK) {
        return result;
    }

    /* It must be claimed, therefore the serial can't change and must
       match ... Now what if it wasn't claimed? There's no guarantee
       whatsoever that an issue will be caught, but do some assertions
       anyway. */
    assert ((pa_ld32 (&info->status_count_index) & STATUS_COUNT_INDEX_MASK) > 0);
    assert (!(pa_ld32 (&info->status_count_index) & STATUS_ON_FREELIST));
    assert (handle.serial == info->serial);

    release_int (server, info, handle.index);
    return V_HANDLE_OK;
}
