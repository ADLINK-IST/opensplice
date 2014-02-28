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
#include "v_handle.h"
#include "v_entity.h"
#include "v_public.h"
#include "os_report.h"
#include "os_abstract.h"

#if defined __sun && (__GNUC__ * 10000 + __GNUC_MINOR__ * 100 + __GNUC_PATCHLEVEL__) < 40100
/* GCC compiler intrinsics are polymorphic, Sun's library is not --
   but we only use them with unsigned 32-bit numbers, hence can easily
   get away with these definitions.

   Also GCC ones are full memory barriers, but each of these
   operations is used only in a certain context, so I believe I can
   get away with some alternative. */
#include <atomic.h>
static inline void __sync_synchronize (void) {
  membar_producer ();
}
static inline int __sync_bool_compare_and_swap (volatile uint32_t *var, uint32_t old, uint32_t new) {
  const int success = atomic_cas_32 (var, old, new) == old;
  membar_consumer ();
  return success;
}
#elif defined _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
static void __sync_synchronize (void) {
  volatile LONG tmp = 0;
  InterlockedExchange (&tmp, 0);
}
static int __sync_bool_compare_and_swap (volatile os_uint32 *var, os_uint32 old, os_uint32 new) {
  return InterlockedCompareExchange (var, new, old) == old;
}
#elif __GNUC__ > 0 && (__GNUC__ * 10000 + __GNUC_MINOR__ * 100 + __GNUC_PATCHLEVEL__) < 40100
static inline int __sync_bool_compare_and_swap (volatile uint32_t *var, uint32_t old, uint32_t new) {
#ifdef __i386
  uint32_t ret;
  __asm__ __volatile__ ("lock; cmpxchgl %2,%1\n" : "=a" (ret), "+m" (*var) : "r" (new), "0" (old) : "memory", "cc");
  return ret == old;
#else
#error "__sync_bool_compare_and_swap workaround: unsupported CPU"
#endif
}
static inline void __sync_synchronize (void) {
#if __x86_64__
  __asm__ __volatile__ ("mfence\n");
#elif defined __i386
  __asm__ __volatile__ ("lock; xorl $0, (%esp)\n");
#else
#error "__sync_synchronize workaround: unsupported CPU"
#endif
}
#endif

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
#define MAX_SERIAL   (0x00ffffff)

const v_handle V_HANDLE_NIL = {0, 0, 0};

#define STATUS_DEREGISTERING 0x80000000
#define STATUS_ON_FREELIST   0x40000000
#define STATUS_COUNT_MASK    0x00ffffff

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
        c_free(type);
        if (server->handleInfos) {
            server->firstFree = NOHANDLE;
            server->lastIndex = NOHANDLE;
            server->suspended = FALSE;
            c_mutexInit(&server->mutex,SHARED_MUTEX);
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

    info = NULL;

    c_mutexLock(&server->mutex);

    if (server->firstFree != NOHANDLE) {
        /* Reuse a previously 'de-registered' handle.
         */
        idx = server->firstFree;
        block = ((v_handleInfo**)server->handleInfos)[COL(idx)];
        info = &block[ROW(idx)];
        assert (info->status_count == STATUS_ON_FREELIST);
        server->firstFree = (c_long) info->object_nextFree;
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
            info->serial = 1;
        } else {
            OS_REPORT(OS_ERROR,
                      "kernel::v_handle::v_handleServerRegister",0,
                      "Failed to allocate a new list of handles");
        }
    }

    /* Initialize the handle and associated info record.
     */
    if (info) {
      info->object_nextFree = (c_address) c_keep(o);
      /* memory barrier here will ensure that any accidental
         handleClaim (we haven't advertised the handle, but in case
         somehow it gets fed random garbage) will fail because
         status_count still has ON_FREELIST set until object_nextFree
         properly points to the object. */
      __sync_synchronize ();
      info->status_count = 0;

      handle.server  = (c_address)server;
      handle.serial  = info->serial;
      handle.index   = idx;
    } else {
        handle  = V_HANDLE_NIL;
    }

    c_mutexUnlock(&server->mutex);

    return handle;
}

static v_handleResult get_server_info (v_handleServer *server, v_handleInfo **info, v_handle handle)
{
  v_handleInfo *block;

  /* Valid serial in [1,MAX_SERIAL] => (serial-1) in [0,MAX_SERIAL-1];
     this guarantees us that an allocated but never yet used entry in
     the handle table (which has serial = 0) will not match handle
     accidentally, and indeed will cause a return code of
     V_HANDLE_ILLEGAL in claim_int. */
  if (handle.serial - 1 >= MAX_SERIAL)
    return V_HANDLE_ILLEGAL;

  /* Valid index in [0,NROFCOL * NROFROW) */
  if (handle.index >= NROFCOL * NROFROW)
    return V_HANDLE_ILLEGAL;

  /* First perform some checks to confirm handle validity. */
  *server = v_handleServer ((c_object) handle.server);
  if (*server == NULL) {
#if 0 /* Following test can be re-inserted when ticket scdds2814 is resolved. */
    OS_REPORT(OS_ERROR,
              "kernel::v_handle::v_handleClaim",0,
              "Operation aborted: Handle is invalid."
              OS_REPORT_NL "Reason: Server address = NULL");
#endif
    return V_HANDLE_ILLEGAL;
  }

  /* The handle is valid so retrieve the handle's info record.
   */
  if ((block = ((v_handleInfo**) (*server)->handleInfos)[COL(handle.index)]) == NULL)
    return V_HANDLE_ILLEGAL;
  *info = &block[ROW(handle.index)];
  return V_HANDLE_OK;
}

static void
v_handleServerDeregister (
    v_handleServer server,
    v_handleInfo *info,
    c_long idx)
{
    v_public o;

    c_mutexLock(&server->mutex);
    assert (info->status_count == STATUS_DEREGISTERING);
    o = v_public((v_object) info->object_nextFree);
    info->serial = (info->serial == MAX_SERIAL) ? 1 : (info->serial + 1);
    info->status_count = STATUS_ON_FREELIST;
    info->object_nextFree = (c_address) server->firstFree;
    server->firstFree = idx;
    c_mutexUnlock(&server->mutex);

    v_publicDispose(o);
}

static void release_int (v_handleServer server, v_handleInfo *info, c_long idx)
{
  /* GCC atomic builtins imply barriers => whatever had been done
     before release_int will be visible in time. */
  assert (!(info->status_count & STATUS_ON_FREELIST));
  if (pa_decrement (&info->status_count) == STATUS_DEREGISTERING)
    v_handleServerDeregister (server, info, idx);
}

static v_handleResult claim_int (v_handleServer server, v_handleInfo *info, v_handle handle)
{
  c_ulong status_count;

 retry:
  status_count = *((volatile c_ulong *) &info->status_count);
  if (status_count & (STATUS_DEREGISTERING | STATUS_ON_FREELIST)) {
    /* can't trust serials unless we first lock server->mutex, but
       then again, who cares whether it is EXPIRED or INVALID
       anyway? */
    return V_HANDLE_EXPIRED;
  } else {
    c_ulong new_status_count = status_count + 1;
    if (!__sync_bool_compare_and_swap (&info->status_count, status_count, new_status_count))
      goto retry;
  }

  /* successfully incremented refcount => serial is stable */
  if (handle.serial == info->serial) {
    return V_HANDLE_OK;
  } else {
    /* Incremented refcount, but for the wrong handle => decide
       whether it is expired or illegal, then immediately release
       it */
    v_handleResult result =
      (handle.serial < info->serial) ? V_HANDLE_EXPIRED : V_HANDLE_ILLEGAL;
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

  if ((result = get_server_info (&server, &info, handle)) != V_HANDLE_OK)
    return result;
  if ((result = claim_int (server, info, handle)) != V_HANDLE_OK)
    return result;

 retry:
  status_count = *((volatile c_ulong *) &info->status_count);
  if (!(status_count & STATUS_DEREGISTERING))
  {
    c_ulong new_status_count = status_count | STATUS_DEREGISTERING;
    if (!__sync_bool_compare_and_swap (&info->status_count, status_count, new_status_count))
      goto retry;
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

  if ((result = get_server_info (&server, &info, handle)) != V_HANDLE_OK)
    return result;

  /* It must be claimed, therefore the serial can't change and must
     match ... Now what if it wasn't claimed? There's no guarantee
     whatsoever that an issue will be caught, but do some assertions
     anyway. */
  assert ((info->status_count & STATUS_COUNT_MASK) > 0);
  assert (!(info->status_count & STATUS_ON_FREELIST));
  assert (handle.serial == info->serial);

  release_int (server, info, handle.index);
  return V_HANDLE_OK;
}
