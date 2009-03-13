#ifndef U_WRITERQOS_H
#define U_WRITERQOS_H

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

OS_API v_writerQos u_writerQosNew    (v_writerQos tmpl);
OS_API u_result    u_writerQosInit   (v_writerQos q);
OS_API void        u_writerQosDeinit (v_writerQos q);
OS_API void        u_writerQosFree   (v_writerQos q);

#undef OS_API

#if defined (__cplusplus)
}
#endif

#endif /* U_WRITERQOS_H */
