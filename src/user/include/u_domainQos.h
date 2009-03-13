#ifndef U_DOMAINQOS_H
#define U_DOMAINQOS_H

#if defined (__cplusplus)
extern "C" {
#endif

#include "u_types.h"
#include "v_kernel.h"
#include "os_if.h"

#ifdef OSPL_BUILD_USER
#define OS_API OS_API_EXPORT
#else
#define OS_API OS_API_IMPORT
#endif
/* !!!!!!!!NOTE From here no more includes are allowed!!!!!!! */

OS_API v_domainQos u_domainQosNew    (v_domainQos tmpl);
OS_API u_result    u_domainQosInit   (v_domainQos q);
OS_API void        u_domainQosDeinit (v_domainQos q);
OS_API void        u_domainQosFree   (v_domainQos q);

#undef OS_API

#if defined (__cplusplus)
}
#endif

#endif /* U_DOMAINQOS_H */
