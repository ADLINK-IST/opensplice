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
#ifndef C_SYNC_H
#define C_SYNC_H

#include "os_mutex.h"
#include "os_rwlock.h"
#include "os_cond.h"
#include "os_thread.h"
#include "c_time.h"

#if defined (__cplusplus)
extern "C" {
#endif
#include "os_if.h"

#ifdef OSPL_BUILD_DB
#define OS_API OS_API_EXPORT
#else
#define OS_API OS_API_IMPORT
#endif
/* !!!!!!!!NOTE From here no more includes are allowed!!!!!!! */

typedef os_result c_syncResult;

#define SYNC_RESULT_SUCCESS     (os_resultSuccess)
#define SYNC_RESULT_UNAVAILABLE (os_resultUnavailable)
#define SYNC_RESULT_TIMEOUT     (os_resultTimeout)
#define SYNC_RESULT_BUSY        (os_resultBusy)
#define SYNC_RESULT_INVALID     (os_resultInvalid)
#define SYNC_RESULT_FAIL        (os_resultFail)

#ifdef NDEBUG
typedef os_mutex c_mutex;
#else
typedef struct {
   os_threadId owner;
   os_mutex mtx;
} c_mutex;
#endif

typedef enum {
    SHARED_MUTEX,
    PRIVATE_MUTEX
} c_mutexAttr;

OS_API c_syncResult c_mutexInit     (c_mutex *mtx, const c_mutexAttr attr);
OS_API c_syncResult c_mutexLock     (c_mutex *mtx);
OS_API c_syncResult c_mutexTryLock  (c_mutex *mtx);
OS_API c_syncResult c_mutexUnlock   (c_mutex *mtx);
OS_API c_syncResult c_mutexDestroy  (c_mutex *mtx);

#ifdef NDEBUG
typedef os_rwlock c_lock;
#else
typedef struct {
    os_threadId owner;
    os_rwlock lck;
} c_lock;
#endif

typedef enum {
    SHARED_LOCK,
    PRIVATE_LOCK
} c_lockAttr;

OS_API c_syncResult c_lockInit      (c_lock *lck, const c_lockAttr attr);
OS_API c_syncResult c_lockRead      (c_lock *lck);
OS_API c_syncResult c_lockWrite     (c_lock *lck);
OS_API c_syncResult c_lockTryRead   (c_lock *lck);
OS_API c_syncResult c_lockTryWrite  (c_lock *lck);
OS_API c_syncResult c_lockUnlock    (c_lock *lck);
OS_API c_syncResult c_lockDestroy   (c_lock *lck);

typedef os_cond c_cond;
typedef enum {
    SHARED_COND,
    PRIVATE_COND
} c_condAttr;

OS_API c_syncResult c_condInit      (c_cond *cnd, c_mutex *mtx, const c_condAttr attr);
OS_API c_syncResult c_condWait      (c_cond *cnd, c_mutex *mtx);
OS_API c_syncResult c_condTimedWait (c_cond *cnd, c_mutex *mtx, const c_time time);
OS_API c_syncResult c_condSignal    (c_cond *cnd);
OS_API c_syncResult c_condBroadcast (c_cond *cnd);
OS_API c_syncResult c_condDestroy   (c_cond *cnd);

typedef struct c_threadId {
    os_threadId value;
} c_threadId;

#undef OS_API

#if defined (__cplusplus)
}
#endif

#endif
