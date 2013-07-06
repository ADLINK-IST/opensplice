/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2013 PrismTech
 *   Limited and its licensees. All rights reserved. See file:
 *
 *                     $OSPL_HOME/LICENSE
 *
 *   for full copyright notice and license terms.
 *
 */
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

void
serviceMonitorProcessDiedservice(
    u_serviceManager serviceManager,
    sr_componentInfo info);

#if defined (__cplusplus)
}
#endif

#endif /* SERVICEMONITOR_H */
