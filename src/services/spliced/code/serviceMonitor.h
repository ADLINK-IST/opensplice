#ifndef SERVICEMONITOR_H
#define SERVICEMONITOR_H

#if defined (__cplusplus)
extern "C" {
#endif

#include "s_types.h"

serviceMonitor
serviceMonitorNew(
    spliced spliceDaemon);
void
serviceMonitorFree(
    serviceMonitor monitor);

void
serviceMonitorStop(
    serviceMonitor monitor);

#if defined (__cplusplus)
}
#endif

#endif /* SERVICEMONITOR_H */
