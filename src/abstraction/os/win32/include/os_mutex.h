#ifndef OS_WIN32_MUTEX_H
#define OS_WIN32_MUTEX_H

#if defined (__cplusplus)
extern "C" {
#endif

#include <os_defs.h>

typedef struct mutex {
    os_scopeAttr scope;
    long         id;
    long         lockCount;
} os_os_mutex;

#if defined (__cplusplus)
}
#endif

#endif /* OS_WIN32_MUTEX_H */
