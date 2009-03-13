#ifndef SR_SERVICEINFO_H
#define SR_SERVICEINFO_H

#include "s_types.h"
#include "os.h"

#include <u_cfElement.h>
#include <kernelModule.h>

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
    os_procId           procId;
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
