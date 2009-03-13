#include <os_if.h>

#ifdef OS_DCPS_API
#undef OS_DCPS_API
#endif

#ifdef OSPL_BUILD_DCPSCCPP
#define OS_DCPS_API OS_API_EXPORT
#else
#define OS_DCPS_API OS_API_IMPORT
#endif
/* !!!!!!!!NOTE From here no more includes are allowed!!!!!!! */
