
#ifndef OS_POSIX_RWLOCK_H
#define OS_POSIX_RWLOCK_H

#if defined (__cplusplus)
extern "C" {
#endif

#include <pthread.h>

typedef pthread_rwlock_t os_os_rwlock;

#if defined (__cplusplus)
}
#endif

#endif /* OS_POSIX_RWLOCK_H */
