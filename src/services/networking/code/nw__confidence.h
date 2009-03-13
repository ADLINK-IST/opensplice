#ifndef NW__CONFIDENCE_H
#define NW__CONFIDENCE_H

#include <assert.h>
#include "os_report.h"

#ifndef NDEBUG

#define NW_CONFIDENCE(expr) assert(expr)

#elif defined(OSPL_ENV_TEST)

#define NW_CONFIDENCE_DEFAULT_TYPE     OS_ERROR
#define NW_CONFIDENCE_DEFAULT_CONTEXT  "Networking service"
#define NW_CONFIDENCE_DEFAULT_CODE     0

#define NW_CONFIDENCE(expr)                      \
    if (!(expr)) {                               \
        OS_REPORT(NW_CONFIDENCE_DEFAULT_TYPE,    \
                  NW_CONFIDENCE_DEFAULT_CONTEXT, \
                  NW_CONFIDENCE_DEFAULT_CODE,    \
                  #expr);                        \
    }


#elif defined(OSPL_ENV_RELEASE)

#define NW_CONFIDENCE(expr)

#else

#define NW_CONFIDENCE(expr) assert(expr)

#endif /* NDEBUG */


#endif  /* NW__CONFIDENCE_H */

