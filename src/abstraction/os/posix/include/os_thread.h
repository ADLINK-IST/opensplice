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

#ifndef OS_POSIX_THREAD_H
#define OS_POSIX_THREAD_H

#if defined (__cplusplus)
extern "C" {
#endif

#include <pthread.h>

#define OS_THREAD_ID_NONE (0U)

typedef pthread_t os_os_threadId;

#if defined (__cplusplus)
}
#endif

#endif /* OS_POSIX_THREAD_H */
