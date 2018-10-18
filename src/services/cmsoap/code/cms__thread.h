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
 * @file services/cmsoap/code/cms__thread.h
 * 
 * Supplies all inner methods for the cmsoap thread.
 */
#ifndef CMS__THREAD_H
#define CMS__THREAD_H

#include "cms__typebase.h"
#include "os_thread.h"

#if defined (__cplusplus)
extern "C" {
#endif

/**
 * Initializes the supplied thread with the supplied name.
 * 
 * @param thread The thread to initialize.
 * @param name The name for the thread.
 */
c_bool cms_threadInit (cms_thread thread, const c_char* name, os_threadAttr * attr);

/**
 * Deinitializes the supplied thread.
 * 
 * @param thread The thread to deinitialize.
 */
void cms_threadDeinit (cms_thread thread);

#endif
