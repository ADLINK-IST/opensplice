#ifndef OS_WIN32_RWLOCK_H
#define OS_WIN32_RWLOCK_H

#if defined (__cplusplus)
extern "C" {
#endif

#include <os_mutex.h>

typedef os_mutex os_os_rwlock;

#if defined (__cplusplus)
}
#endif

#endif /* OS_WIN32_RWLOCK_H */
