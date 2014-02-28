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

#ifndef OS_POSIX_SEMAPHORE_H
#define OS_POSIX_SEMAPHORE_H

#include <pthread.h>
#if defined (__cplusplus)
extern "C" {
#endif

  /* Apply didn't think it necessary to support anonymous POSIX
     semaphores (created using sem_init), so just map it onto a
     mutex/condvar pair */
  typedef struct os_sem {
    pthread_mutex_t mtx;
    uint32_t value;
    pthread_cond_t cv;
  } os_sem_t;

#if defined (__cplusplus)
}
#endif
#endif /* OS_POSIX_SEMAPHORE_H */
