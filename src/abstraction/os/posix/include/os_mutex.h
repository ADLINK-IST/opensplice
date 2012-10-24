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

#ifndef OS_POSIX_MUTEX_H
#define OS_POSIX_MUTEX_H

#if defined (__cplusplus)
extern "C" {
#endif
#include <pthread.h>

#ifdef OSPL_STRICT_MEM
   typedef struct os_os_mutex
   {
      uint64_t signature; /* Used to identify initialized cond when memory is
                             freed - keep this first in the structure so its
                             so its address is the same as the os_mutex */
      pthread_mutex_t mutex;
   } os_os_mutex;
#else
   typedef pthread_mutex_t os_os_mutex;
#endif

#if defined (__cplusplus)
}
#endif

#endif /* OS_POSIX_MUTEX_H */
