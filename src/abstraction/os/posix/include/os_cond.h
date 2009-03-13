
#ifndef OS_POSIX_COND_H
#define OS_POSIX_COND_H

#if defined (__cplusplus)
extern "C" {
#endif

#include <pthread.h>

#ifdef OSPL_STRICT_MEM
   typedef struct os_os_cond {
      uint64_t signature; /* Used to identify initialized cond when memory is
                             freed - keep this first in the structure so its
                             so its address is the same as the os_cond */
      pthread_cond_t cond;
   } os_os_cond;
#else
   typedef pthread_cond_t os_os_cond;
#endif


#if defined (__cplusplus)
}
#endif

#endif /* OS_POSIX_COND_H */
