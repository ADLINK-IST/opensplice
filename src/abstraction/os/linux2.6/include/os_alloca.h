
#include "alloca.h"

#ifdef NDEBUG

#define os_alloca(size) alloca(size)
#define os_freea(ptr)

#else

#define os_alloca(size) os_malloc(size)
#define os_freea(ptr) os_free(ptr)

#endif
