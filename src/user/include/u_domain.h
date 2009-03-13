#ifndef U_DOMAIN_H
#define U_DOMAIN_H

#if defined (__cplusplus)
extern "C" {
#endif

#include "u_types.h"
#include "os_if.h"

#ifdef OSPL_BUILD_USER
#define OS_API OS_API_EXPORT
#else
#define OS_API OS_API_IMPORT
#endif
/* !!!!!!!!NOTE From here no more includes are allowed!!!!!!! */

#define  u_domain(o)  ((u_domain)(o))

OS_API u_domain u_domainNew  (u_participant p, const c_char *name, v_domainQos qos);
OS_API u_result u_domainInit (u_domain d);
OS_API u_result u_domainFree (u_domain d);
OS_API u_result u_domainDeinit (u_domain d);

#undef OS_API

#if defined (__cplusplus)
}
#endif

#endif
