/*
 *                         Vortex OpenSplice
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR ADLINK
 *   Technology Limited, its affiliated companies and licensors. All rights
 *   reserved.
 *
 *   Licensed under the ADLINK Software License Agreement Rev 2.7 2nd October
 *   2014 (the "License"); you may not use this file except in compliance with
 *   the License.
 *   You may obtain a copy of the License at:
 *                      $OSPL_HOME/LICENSE
 *
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 */
#ifndef D_THREAD_H
#define D_THREAD_H

#include "os_defs.h"
#include "ut_thread.h"

typedef ut_thread d_thread;

os_result d_threadsInit (d_durability durability);
d_durability d_threadsDurability();
void d_threadsDeinit (void);
os_result d_threadCreate (os_threadId *threadId, const char *name, const os_threadAttr *threadAttr, os_threadRoutine start_routine, void *arg);
d_thread d_threadLookupId (os_threadId tid);
d_thread d_threadLookupSelf (void);
os_result d_threadWaitExit (os_threadId threadId, void **thread_result);

#define d_threadAsleep(thr, sleep_secs)   ut_threadAsleep((ut_thread)thr, sleep_secs)
#define d_threadAwake(thr)                (thr?ut_threadAwake((ut_thread)thr):(void)0)
#define d_sleep(thr, d)                   ut_sleep((ut_thread)thr, d)
#define d_condWait(thr, cv, mtx)          ut_condWait((ut_thread)thr, cv, mtx)
#define d_condTimedWait(thr, cv, mtx, d)  ut_condTimedWait((ut_thread)thr, cv, mtx, d)

#define d_threadName(thr)                 (thr?ut_threadGetName((ut_thread)thr):"anonimous")
const char* d_threadSelfName();

void d_threadLivelinessInit (os_duration min_intv);
os_boolean d_threadLivelinessCheck (os_timeM currentTime, os_duration updateInterval);

#endif /* D_THREAD_H */
