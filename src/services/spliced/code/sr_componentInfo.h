/*
 *                         Vortex OpenSplice
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR ADLINK
 *   Technology Limited, its affiliated companies and licensors. All rights
 *   reserved.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 */
#ifndef SR_COMPONENTINFO_H
#define SR_COMPONENTINFO_H

#include "s_types.h"
#include "vortex_os.h"

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
