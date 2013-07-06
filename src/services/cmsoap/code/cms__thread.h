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
void cms_threadInit (cms_thread thread, const c_char* name, os_threadAttr * attr);

/**
 * Deinitializes the supplied thread.
 * 
 * @param thread The thread to deinitialize.
 */
void cms_threadDeinit (cms_thread thread);

#endif
