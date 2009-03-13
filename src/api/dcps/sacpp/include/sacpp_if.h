#include <os_if.h>

#ifdef OSPL_BUILD_DCPSCCPP
#define SACPP_API OS_API_EXPORT
#else
#define SACPP_API OS_API_IMPORT
#endif
/* !!!!!!!!NOTE From here no more includes are allowed!!!!!!! */
