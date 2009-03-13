
#ifndef PA_LINUX_ABSTRACT_H
#define PA_LINUX_ABSTRACT_H

#if defined (__cplusplus)
extern "C" {
#endif

/* include OS specific PLATFORM definition file */
#include <endian.h>

#ifdef __LITTLE_ENDIAN
#define PA__LITTLE_ENDIAN
#else
#define PA__BIG_ENDIAN
#endif

#ifdef __x86_64__
#define PA__64BIT
#endif

#if defined (__cplusplus)
}
#endif

#endif /* PA_LINUX_ABSTRACT_H */
