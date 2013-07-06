#ifndef OS_GITREV_H
#define OS_GITREV_H

#include "os_macrostringify.h"

#ifdef OSPL_INNER_REV
#define OSPL_INNER_REV_STR OSPL_STRINGIFY(OSPL_INNER_REV)
#else
#define OSPL_INNER_REV_STR ""
#endif

#ifdef OSPL_OUTER_REV
#define OSPL_OUTER_REV_STR OSPL_STRINGIFY(OSPL_OUTER_REV)

#ifndef OSPL_INNER_REV
#error OSPL_OUTER_REV is defined but OSPL_INNER_REV is not set.
#endif

#else
#define OSPL_OUTER_REV_STR ""
#endif

#endif
