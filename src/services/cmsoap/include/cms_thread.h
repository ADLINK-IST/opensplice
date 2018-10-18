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

/**
 * @file services/cmsoap/include/cms_thread.h
 * 
 * Base class for a thread for the cmsoap service.
 */
#ifndef CMS_THREAD_H
#define CMS_THREAD_H

#include "cms__typebase.h"

#if defined (__cplusplus)
extern "C" {
#endif

#define cms_thread(a) ((cms_thread)(a))


/**
 * Constructs a new thread, but does NOT start it.
 * 
 * @param name The name for the thread.
 * @return The newly created thread.
 */
cms_thread  cms_threadNew       (const c_char* name, os_threadAttr * attr);
                                 
/**
 * Starts the supplied thread in the supplied routine and passes on the
 * supplied arg to the routine.
 * 
 * @param thread The cmsoap thread.
 * @param start_routine The thread routine.
 * @param arg The argument to pass on to the start routine.
 * @return TRUE if the thread was started successfully, FALSE otherwise.
 */
c_bool      cms_threadStart     (cms_thread thread,
                                 void *(* start_routine)(void *),
                                 void *arg);

/**
 * Triggers the thread to terminate, awaits its termination and frees it.
 * 
 * @param thread The thread to terminate and free.
 */
void        cms_threadFree      (cms_thread thread);

#endif
