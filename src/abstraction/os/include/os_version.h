#ifndef OS_VERSION_H
#define OS_VERSION_H

#include "os_macrostringify.h"

#ifndef OSPL_VERSION
#error OSPL_VERSION must be defined.
#endif

#define OSPL_VERSION_STR OSPL_STRINGIFY(OSPL_VERSION)

#endif
