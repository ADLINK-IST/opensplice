/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2009 PrismTech 
 *   Limited and its licensees. All rights reserved. See file:
 *
 *                     $OSPL_HOME/LICENSE 
 *
 *   for full copyright notice and license terms. 
 *
 */

#ifndef OS_DARWIN_MUTEX_H
#define OS_DARWIN_MUTEX_H

#if defined (__cplusplus)
extern "C" {
#endif

#include <pthread.h>
#include <stdint.h>
#include "include/msem.h"

  typedef struct os_os_mutex
  {
#ifdef OSPL_STRICT_MEM
    uint64_t signature; /* Used to identify initialized cond when memory is
			   freed - keep this first in the structure so its
			   so its address is the same as the os_mutex */
#endif
    int shared;
    union {
      pthread_mutex_t mutex;
      __msem_t msem;
    } u;
  } os_os_mutex;

#if defined (__cplusplus)
}
#endif

#endif /* OS_DARWIN_MUTEX_H */
