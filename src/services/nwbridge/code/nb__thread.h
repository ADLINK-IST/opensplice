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
#ifndef NB__THREAD_H_
#define NB__THREAD_H_

#include "nb__object.h"
#include "nb__log.h" /* for nb_logbuf; should come from nb__types.h */

typedef void* (*nb_threadRoutine)(void *arg);

#ifndef NDEBUG
# define nb_thread(o) (assert(nb__objectKind(nb_object(o)) == NB_OBJECT_THREAD), (nb_thread)(o))
#else
# define nb_thread(o) ((nb_thread)(o))
#endif

#define nb_threadStates(o) ((nb_threadStates)(o))

nb_threadStates     nb_threadStatesAlloc(void) __attribute_malloc__
                                               __attribute_returns_nonnull__;

os_result           nb_threadStatesInit(nb_threadStates _this,
                                        unsigned maxthreads,
                                        nb_logConfig gc) __nonnull_all__;

void                nb_threadStatesDeinit(nb_threadStates ts) __nonnull_all__;

void                nb_threadStatesDealloc(nb_threadStates ts) __nonnull_all__;

void                nb_threadUpgrade(nb_threadStates ts) __nonnull_all__;

void                nb_threadDowngrade(void);

nb_thread           nb_threadLookup(void);

nb_logConfig        nb_threadLogConfig(nb_thread thread) __nonnull_all__
                                                         __attribute_returns_nonnull__;

const char *        nb_threadName(nb_thread thread) __nonnull_all__
                                                    __attribute_returns_nonnull__;

nb_logbuf           nb_threadLogbuf(nb_thread _this) __nonnull_all__
                                                     __attribute_returns_nonnull__;

os_result           nb_threadCreate(const char *name,
                                    nb_thread *thread,
                                    const nb_threadRoutine func,
                                    void *arg) __nonnull((1,2,3));

os_result           nb_threadJoin(nb_thread thread,
                                  void **retval ) __nonnull((1));

#endif /* NB__THREAD_H_ */
