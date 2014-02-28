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
#ifndef OS_WIN32_SEMAPHORE_H
#define OS_WIN32_SEMAPHORE_H

#include <os_semaphore.h>

#if defined (__cplusplus)
extern "C" {
#endif

#include <winsock2.h>
typedef HANDLE os_sem_t;

#if defined (__cplusplus)
}
#endif
#endif /* OS_WIN32_SEMAPHORE_H */
