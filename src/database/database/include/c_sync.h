/*
 *                         Vortex OpenSplice
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR ADLINK
 *   Technology Limited, its affiliated companies and licensors. All rights
 *   reserved.
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

#ifdef OSPL_BUILD_CORE
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

OS_API c_syncResult c_mutexInit     (c_base base, c_mutex *mtx);
OS_API void         c_mutexLock     (c_mutex *mtx);
OS_API c_syncResult c_mutexTryLock  (c_mutex *mtx);
OS_API void         c_mutexUnlock   (c_mutex *mtx);
OS_API void         c_mutexDestroy  (c_mutex *mtx);

#ifdef NDEBUG
typedef os_rwlock c_lock;
#else
typedef struct {
    os_threadId owner;
    os_rwlock lck;
} c_lock;
#endif

OS_API c_syncResult c_lockInit      (c_base base, c_lock *lck);
OS_API void         c_lockRead      (c_lock *lck);
OS_API void         c_lockWrite     (c_lock *lck);
OS_API c_syncResult c_lockTryRead   (c_lock *lck);
OS_API c_syncResult c_lockTryWrite  (c_lock *lck);
OS_API void         c_lockUnlock    (c_lock *lck);
OS_API void         c_lockDestroy   (c_lock *lck);

typedef os_cond c_cond;

OS_API c_syncResult c_condInit      (c_base base, c_cond *cnd, c_mutex *mtx);
OS_API void         c_condWait      (c_cond *cnd, c_mutex *mtx);
OS_API c_syncResult c_condTimedWait (c_cond *cnd, c_mutex *mtx, const os_duration time);
OS_API void         c_condSignal    (c_cond *cnd);
OS_API void         c_condBroadcast (c_cond *cnd);
OS_API void         c_condDestroy   (c_cond *cnd);

typedef struct c_threadId {
    os_threadId value;
} c_threadId;

#undef OS_API

#if defined (__cplusplus)
}
#endif

#endif
