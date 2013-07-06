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
