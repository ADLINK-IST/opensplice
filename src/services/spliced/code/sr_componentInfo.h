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
#ifndef SR_COMPONENTINFO_H
#define SR_COMPONENTINFO_H

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

/* sr_componentInfo is shared by Domain/Service and Domain/Application */
C_STRUCT(sr_componentInfo) {
    os_threadId         threadId;
    os_procId           procId;
    char                *name;
#ifndef INTEGRITY
    os_procAttr         procAttr;
#endif
    v_schedulePriorityKind priorityKind;
    char                *command;
    char                *configuration;
    char                *args; /* additional arguments, only used during testing */
    char                *library;
    sr_restartRule      restartRule;

    os_boolean          isService; /* flag to discriminate service from application */
};

sr_componentInfo
sr_componentInfoServiceNew(
    u_cfElement info,
    const char *defaultConfigURI
);

sr_componentInfo
sr_componentInfoApplicationNew(
    u_cfElement info,
    const char *defaultConfigURI
);

void
sr_componentInfoFree(
    sr_componentInfo componentInfo);

#if defined (__cplusplus)
}
#endif

#endif /* SR_COMPONENTINFO_H */
