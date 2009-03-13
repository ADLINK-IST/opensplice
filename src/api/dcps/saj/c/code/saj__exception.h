
#define SAJ_EXCEPTION_TYPE_BAD_PARAM     0
#define SAJ_EXCEPTION_TYPE_NO_MEMORY     1
#define SAJ_EXCEPTION_TYPE_MARSHAL       2
#define SAJ_EXCEPTION_TYPE_BAD_OPERATION 3

#if defined (__cplusplus)
extern "C" {
#endif
#include "os_if.h"

#ifdef OSPL_BUILD_DCPSSAJ
#define OS_API OS_API_EXPORT
#else
#define OS_API OS_API_IMPORT
#endif
/* !!!!!!!!NOTE From here no more includes are allowed!!!!!!! */

OS_API void
saj_exceptionThrow (
    JNIEnv *env,
    int errorCode,
    const char *format,
    ...);

#undef OS_API

#if defined (__cplusplus)
}
#endif

