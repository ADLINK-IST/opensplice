/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2011 PrismTech
 *   Limited and its licensees. All rights reserved. See file:
 *
 *                     $OSPL_HOME/LICENSE 
 *
 *   for full copyright notice and license terms. 
 *
 */
#ifndef SR_SERVICEINFO_H
#define SR_SERVICEINFO_H

#include "s_types.h"
#include "os.h"

#include "u_cfElement.h"
#include "kernelModule.h"

#if defined (__cplusplus)
extern "C" {
#endif

typedef enum sr_restartRule {
    RR_NONE,
    RR_SKIP,
    RR_KILL,
    RR_RESTART,
    RR_HALT
} sr_restartRule;

C_STRUCT(sr_serviceInfo) {
#ifdef OSPL_ENV_SHMT
    os_threadId         procId;
#else
    os_procId           procId;
#endif
    char                *name;
#ifndef INTEGRITY
    os_procAttr         procAttr;
#endif
    v_scheduleKind      priorityKind;
    char                *command;
    char                *configuration;
    char                *args; /* additional arguments, only used during testing */
    sr_restartRule      restartRule;
};

sr_serviceInfo
sr_serviceInfoNew(
    u_cfElement info,
    const char *defaultConfigURI);

void
sr_serviceInfoFree(
    sr_serviceInfo serviceInfo);

#if defined (__cplusplus)
}
#endif

#endif /* SR_SERVICEINFO_H */
