/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2009 PrismTech 
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
    type = c_resolve(base,"kernelModule::v_handleInfoList");
    server->handleInfos = c_arrayNew(type,NROFCOL);
    c_free(type);
    server->firstFree = NOHANDLE;
    server->lastIndex = NOHANDLE;
    server->suspended = FALSE;
    c_mutexInit(&server->mutex,SHARED_MUTEX);
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
    if(server->suspended == TRUE) {
        *info = NULL;
        return V_HANDLE_SUSPENDED;
    }
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
        info = &block[row];
        info->serial = SERIALSTART;
        c_mutexInit(&info->mutex,SHARED_MUTEX);
    }

    info->object   = c_keep(o);
    info->nextFree = NOHANDLE;
    info->count    = 0;
    info->freed    = FALSE;

    handle.server  = (c_address)server;
    handle.serial  = info->serial;
    handle.index   = idx;

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
            info->freed = TRUE;
            c_mutexUnlock(&info->mutex);
        } else {
            v_handleInvalidate(handle,info);
        }
    }
    
    return result;
}

v_handleResult
v_handleClaim (
    v_handle handle,
    v_object *o)
{
    v_handleInfo *info;
    v_handleResult result;

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
        *o = (v_object)info->object;
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
        if (info->count == 0) {
            if (info->freed) {
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

