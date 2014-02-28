#ifndef OS_HOSTTARGET_H
#define OS_HOSTTARGET_H
#include "os_macrostringify.h"

#ifdef OSPL_HOST
#define OSPL_HOST_STR OSPL_STRINGIFY(OSPL_HOST)
#else
#define OSPL_HOST_STR ""
#endif

#ifdef OSPL_TARGET
#define OSPL_TARGET_STR OSPL_STRINGIFY(OSPL_TARGET)
#else
#define OSPL_TARGET_STR ""
#endif

#endif
