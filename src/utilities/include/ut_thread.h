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
#ifndef UT_THREAD_H
#define UT_THREAD_H

#include "os_defs.h"
#include "os_thread.h"
#include "os_mutex.h"
#include "os_cond.h"
#include "os_classbase.h"

#if defined (__cplusplus)
extern "C" {
#endif
#include "os_if.h"
#ifdef OSPL_BUILD_CORE
#define OS_API OS_API_EXPORT
#else
#define OS_API OS_API_IMPORT
#endif


/*
 * An ut_threads object contains information about the threads that were
 * created in that objects' context. It can be used to detect if the related
 * threads make progress or are likely deadlocked.
 */
OS_CLASS(ut_threads);

/*
 * An ut_thread object contains information about a specific thread.
 *
 * An ut_thread should call ut_threadAwake() periodically to let the utility
 * know that the thread is not deadlocked. This 'periodically' means within
 * the interval provided when calling ut_threadsNew().
 *
 * An ut_thread can indicate that it will not call ut_threadAwake() for a
 * certain time by calling ut_threadAsleep(). When this is the case, the
 * utility will not set this thread as deadlocked when it hasn't called
 * threadAwake() within the normal interval during this sleep period.
 *
 * Calling ut_threadAsleep() and ut_threadAwake() is automatically done when
 * the functions ut_nanoSleep(), ut_condWait() and ut_condTimedWait() are
 * called.
 */
OS_CLASS(ut_thread);


/*
 * If a caller is interested in the states of its threads, it can provide
 * a report callback when calling ut_threadsAllIsWell().
 *
 * This is the callback type.
 *
 * Parameters:
 * thread     - The thread related to the given information.
 *              NULL will be provided as last report. If alive is true in that
 *              case, all threads are alive. If only one is not alive, alive
 *              will be FALSE.
 * alive      - When TRUE, the given thread is alive.
 *              Or all threads are alive when thread is NULL and alive is TRUE.
 * changed    - The (thread specific or thread list) information has changed
 *              between current report and previous report.
 * info       - Extra information that ut_threads thinks is interesting.
 * caller     - Name of the caller thread.
 * reportdata - The reportdata provided when calling ut_threadsAllIsWell().
 * userdata   - The userdata provided when creating an ut_threads object.
 *
 * Returns:
 * void
 */
typedef void
(*ut_threadReport)(
        ut_thread thread,
        os_boolean alive,
        os_boolean changed,
        const char *info,
        const char *caller,
        void *reportdata,
        void *userdata);


/*
 * When not knowing when the thread should wake up again, UT_SLEEP_INDEFINITELY
 * can be passed to ut_threadAsleep().
 */
#define UT_SLEEP_INDEFINITELY (0)


/*
 * Create a new threads object with which to manage and check threads.
 *
 * The calling thread is automatically added as an awake thread to the threads
 * object. The thread is added under the assumed name 'selfname'.
 *
 * Parameters:
 * selfname - Assumed name of the calling thread.
 * interval - Max interval in which a thread should indicate that it's awake.
 * message  - The actual report message.
 * caller   - Name of the caller thread.
 * userdata - The userdata provided when creating an ut_threads object.
 *
 * Returns:
 * New threads managing object.
 */
OS_API ut_threads
ut_threadsNew(const char* selfname, os_duration interval, os_uint32 maxthreads, void *userdata);

/*
 * Frees the threads managing object.
 *
 * All related threads should have stopped.
 *
 * Parameters:
 * threads - The threads managing object.
 *
 * Returns:
 * void
 */
OS_API void
ut_threadsFree(ut_threads threads);

/*
 * Overrules the interval set when calling ut_threadsNew.
 *
 * Parameters:
 * threads  - The threads managing object.
 * interval - Max interval in which a thread should indicate that it's awake.
 *
 * Returns:
 * void
 */
OS_API void
ut_threadsSetInterval(ut_threads threads, os_duration interval);

/*
 * Checks the threads that are related to the managing object.
 *
 * All is well when all threads have either announced themselves awake within
 * the required interval (provided when calling ut_threadsNew) or asleep (and
 * the sleep period hasn't finished yet).
 *
 * If a thread hasn't announced itself awake within the interval or the thread
 * hasn't woken up from it's sleep within the indicated time (+interval), then
 * the thread is considered to have deadlocked. And all is not well.
 *
 * It is likely that whoever uses this utility wants to get information about
 * its created threads. For that, the report callback is provided. This will
 * be called with string messages containing thread(s) information. Overall
 * liveliness changes and thread progress changes are always and immediately
 * reported. 'Normal' threads information will be reported in a periodic
 * fashion. The reportInterval indicates the periodic interval of these
 * 'normal' reports.
 *
 * Liveliness checking may only be done on one thread. Limitation is
 * easily lifted: use a CAS loop in check function.
 * For more info on CAS loop see http://en.wikipedia.org/wiki/Compare-and-swap
 *
 * Parameters:
 * threads    - The threads managing object.
 * reportCb   - Report callback
 * reportData - The report data that will be given to the report callback.
 *
 * Returns:
 * TRUE  - All is well.
 * FALSE - One or more threads haven't announced themselves awake within the
 *         required interval. Possibly due to a deadlock.
 */
OS_API os_boolean
ut_threadsAllIsWell(ut_threads threads, ut_threadReport reportCb, void *reportData);

/*
 * Returns the userdata provided when calling ut_threadsNew().
 *
 * Parameters:
 * threads - The threads managing object.
 *
 * Returns:
 * The userdata void pointer.
 */
OS_API void*
ut_threadsUserData(ut_threads threads);

/* Create a new thread associated with the threads management object.
 *
 * Creates a new thread of control that executes concurrently with the calling
 * thread. The new thread applies the function startRoutine passing it arg as
 * first argument.
 *
 * Parameters:
 * threads      - The threads managing object.
 * thread       - The created thread (NULL with a failure).
 * name         - The thread name.
 * threadAttr   - OS thread attributes.
 * startRoutine - Function that will be called by the new thread.
 * arg          - Arguments that are passed to the startRoutine.
 *
 * Returns:
 * void
 */
OS_API void
ut_threadCreate(ut_threads threads, ut_thread *thr, const char *name, const os_threadAttr *threadAttr, os_threadRoutine startRoutine, void *arg);

/*
 * Finds and returns the thread info identified by the OS thread ID.
 *
 * Parameters:
 * threads - The threads managing object.
 * tid     - OS thread ID.
 *
 * Returns:
 * The found thread info.
 * NULL when not found (either by invalid tid or thread was not part of the
 * given threads).
 */
OS_API ut_thread
ut_threadLookupId(ut_threads threads, os_threadId tid);

/*
 * Finds and returns the thread info of the calling thread.
 *
 * Parameters:
 * threads - The threads managing object.
 *
 * Returns:
 * The found thread info.
 * NULL when not found (calling thread was not part of the given threads).
 */
OS_API ut_thread
ut_threadLookupSelf(ut_threads threads);

/*
 * Indicate that the thread will sleep for a number of seconds and thus will
 * not announce itself awake in that period.
 *
 * Parameters:
 * thr - The thread that will sleep.
 * sec - The period the thread will sleep.
 *
 * Returns:
 * void
 */
OS_API void
ut_threadAsleep(ut_thread thr, os_uint32 sec);

/*
 * Indicate that the thread is still awake.
 *
 * Parameters:
 * thr - The thread that is still awake.
 *
 * Returns:
 * void
 */
OS_API void
ut_threadAwake(ut_thread thr);

/*
 * Return the OS thread ID of the given utilities thread.
 *
 * Parameters:
 * thr         - The thread to get the ID from.
 *
 * Returns:
 * os_threadId - The OS thread ID.
 */
OS_API os_threadId
ut_threadGetId(ut_thread thr);

/*
 * Return the name of the given utilities thread.
 *
 * Be aware that the pointer is only valid as long as the thread is running.
 *
 * Parameters:
 * thr         - The thread to get the name from.
 *
 * Returns:
 * const char* - The thread name, provided during ut_threadCreate().
 */
OS_API const char*
ut_threadGetName(ut_thread thr);

/*
 * Suspend the execution of the calling thread for the specified time.
 *
 * It is registered that the thread will not indicate progress while it
 * is sleeping.
 *
 * It returns os_result because it wraps to os_sleep().
 *
 * Parameters:
 * thr              - The thread to sleep
 * delay            - The duration to sleep
 *
 * Returns:
 * os_resultSuccess - The thread is suspended for the specified time
 * os_resultFail    - The thread is not suspended for the specified time
 *                    because of a failure, for example when a negative
 *                    delay is supplied or when the ns-part is not normalized.
 */
OS_API os_result
ut_sleep(ut_thread thr, os_duration delay);

/*
 * Wait for the condition
 *
 * It is registered that the thread will not indicate progress while it
 * is waiting on the condition.
 *
 * It returns os_result because it wraps to os_condWait().
 *
 * Precondition:
 * - mutex is acquired by the calling thread before calling ut_condWait
 *
 * Postcondition:
 * - mutex is still acquired by the calling thread and should be released by it
 *
 * Parameters:
 * thr              - The thread that will wait
 * cv               - The condition variable
 * mtx              - The related mutex
 *
 * Returns:
 * void
 */
OS_API void
ut_condWait(ut_thread thr, os_cond *cv, os_mutex *mtx);

/*
 * Wait for the condition but return when the specified time has expired
 * before the condition is triggered
 *
 * It is registered that the thread will not indicate progress while it
 * is waiting on the condition.
 *
 * It returns os_result because it wraps to ut_condTimedWait().
 *
 * Precondition:
 * - mutex is acquired by the calling thread before calling ut_condTimedWait
 *
 * Postcondition:
 * - mutex is still acquired by the calling thread and should be released by it
 *
 * Parameters:
 * thr              - The thread that will wait
 * cv               - The condition variable
 * mtx              - The related mutex
 * timeout          - The duration it takes for it to timeout
 *
 * Returns:
 * os_resultSuccess - The condition is triggered
 * os_resultTimeout - The condition timed out
 */
OS_API os_result
ut_condTimedWait(ut_thread thr, os_cond *cv, os_mutex *mtx, os_duration timeout);

/*
 * Wait for the termination of the identified thread
 *
 * If the identified thread is still running, wait for its termination else
 * return immediately. In thread_result it returns the exit status of the
 * thread. If thread_result is passed as a NULL pointer, no exit status is
 * returned, but os_threadWaitExit still waits for the thread to terminate.
 *
 * If the thread was found to be not alive during the previous check, this
 * function will expect the thread to have deadlocked and will not wait. It
 * will return os_resultBusy in that case.
 *
 * It returns os_result because it wraps to os_threadWaitExit().
 *
 * Parameters:
 * thr              - The thread to wait for
 * result           - The result value of the thread
 *
 * Returns:
 * os_resultSuccess - The identified thread is not running
 * os_resultBusy    - The given thread was indicated to be not alive
 * os_resultFail    - Not a thread
 */
OS_API os_result
ut_threadWaitExit(ut_thread thr, void **result);

/*
 * Translate the given thread information into a string and copy it into the
 * given buffer.
 *
 * It does not write more than size bytes (including the terminating null byte
 * ('\0')). If the output was truncated due to this limit, then the return
 * value is the number of characters (excluding the terminating null byte)
 * which would have been written to the final string if enough space had been
 * available. Thus, a return value of size or more means that the output was
 * truncated.
 *
 * If an output error (or another problem) is encountered, that prevents this
 * function from translating the thread information, the value 0 is returned.
 *
 * Parameters:
 * thr     - The thread to translate.
 * alive   - The alive state of the thread.
 * info    - Additional information about the thread.
 * buf     - Output buffer.
 * size    - Output buffer size.
 *
 * Returns:
 * 0       - failure
 * other   - Number of (possibly) written characters.
 */
OS_API os_uint32
ut_threadToString(ut_thread thr, os_boolean alive, const char *info, char *buf, os_uint32 size);

#undef OS_API

#if defined (__cplusplus)
}
#endif

#endif /* UT_THREAD_H */
