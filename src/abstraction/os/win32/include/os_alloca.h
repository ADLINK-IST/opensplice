
#ifndef OS_WIN32_ALLOCA_H
#define OS_WIN32_ALLOCA_H

#define os_alloca(size) os_malloc(size)
#define os_freea(ptr)  os_free(ptr)

#endif /* OS_WIN32_ALLOCA_H */
