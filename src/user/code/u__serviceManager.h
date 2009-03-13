
#ifndef U__SERVICEMANAGER_H
#define U__SERVICEMANAGER_H

#include "u_serviceManager.h"

#if defined (__cplusplus)
extern "C" {
#endif

v_serviceState u_serviceManagerGetServiceState(u_serviceManager s, const c_char *name);
u_result       u_serviceManagerDeinit         (u_serviceManager s);

#if defined (__cplusplus)
}
#endif

#endif /* U__SERVICEMANAGER_H */
