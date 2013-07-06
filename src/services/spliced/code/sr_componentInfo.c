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

#include "report.h"

#include "sr_componentInfo.h"

#include "u_cfData.h"
#include "kernelModule.h"

#define ATTR_NAME       "name"
#define ATTR_ENABLED    "enabled"
#define ATTR_PRIOKIND   "priority_kind"

#define SCHED_RT   "Realtime"
#define SCHED_TS   "Timeshare"
#define SCHED_DEF  "Default"

#define PRIOKIND_REL "Relative"
#define PRIOKIND_ABS "Absolute"

#define RR_SKIP_STR    "skip"
#define RR_KILL_STR    "kill"
#define RR_RESTART_STR "restart"
#define RR_HALT_STR    "systemhalt"

/**************************************************************
 * Private functions
 **************************************************************/
static c_bool
cfgGetCommand(
    sr_componentInfo ci,
    u_cfElement info)
{
    c_iter iter;
    int      iterLength;
    c_bool r;
    u_cfData d;

    r = TRUE;

    iter = u_cfElementXPath(info, "Command/#text");
    iterLength = c_iterLength(iter);
    d = u_cfData(c_iterTakeFirst(iter));
    if (iterLength == 1) {
        r = u_cfDataStringValue(d, &ci->command);
        u_cfDataFree(d);
    } else {
        OS_REPORT_1(OS_ERROR, OSRPT_CNTXT_SPLICED,
            0, "One <Command> parameter expected for %s", ci->name);
        while (d != NULL) {
            u_cfDataFree(d);
            d = u_cfData(c_iterTakeFirst(iter));
        }
        r = FALSE;
    }
    c_iterFree(iter);

    return r;
}

static c_bool
cfgGetConfiguration(
    sr_componentInfo ci,
    u_cfElement info,
    const char *defaultURI)
{
    c_iter iter;
    int    iterLength;
    c_bool r;
    u_cfData d;

    r = TRUE;
    iter = u_cfElementXPath(info, "Configuration/#text");
    iterLength = c_iterLength(iter);
    d = u_cfData(c_iterTakeFirst(iter));
    if (iterLength == 1) {
        r = u_cfDataStringValue(d, &ci->configuration);
        u_cfDataFree(d);
    } else if (iterLength == 0) {
        ci->configuration = os_strdup(defaultURI);
    } else {
        OS_REPORT_1(OS_ERROR, OSRPT_CNTXT_SPLICED,
            0, "One <Configuration> parameter expected for %s", ci->name);
        ci->configuration = os_strdup(defaultURI);
        while (d != NULL) {
            u_cfDataFree(d);
            d = u_cfData(c_iterTakeFirst(iter));
        }
        r = FALSE;
    }
    c_iterFree(iter);

    return r;
}

static c_bool
cfgGetArguments(
    sr_componentInfo ci,
    u_cfElement info)
{
    c_iter iter;
    int    iterLength;
    c_bool r;
    u_cfData d;

    r = TRUE;
    iter = u_cfElementXPath(info, "Arguments/#text");
    iterLength = c_iterLength(iter);
    d = u_cfData(c_iterTakeFirst(iter));
    if (iterLength == 1) {
        r = u_cfDataStringValue(d, &ci->args);
        u_cfDataFree(d);
    } else if (iterLength == 0) {
        ci->args = os_strdup("");
    } else {
        OS_REPORT_1(OS_ERROR, OSRPT_CNTXT_SPLICED,
            0, "One <Arguments> parameter expected for %s", ci->name);
        ci->args = os_strdup("");
        while (d != NULL) {
            u_cfDataFree(d);
            d = u_cfData(c_iterTakeFirst(iter));
        }
        r = FALSE;
    }
    c_iterFree(iter);

    return r;
}

static c_bool
cfgGetLibrary(
    sr_componentInfo ci,
    u_cfElement info)
{
    c_iter iter;
    int    iterLength;
    c_bool r;
    u_cfData d;

    r = TRUE;
    iter = u_cfElementXPath(info, "Library/#text");
    iterLength = c_iterLength(iter);
    d = u_cfData(c_iterTakeFirst(iter));
    if (iterLength == 1) {
        r = u_cfDataStringValue(d, &ci->library);
        u_cfDataFree(d);
    } else if (iterLength == 0) {
        ci->library = os_strdup("");
    } else {
        OS_REPORT_1(OS_ERROR, OSRPT_CNTXT_SPLICED,
            0, "One <Library> parameter expected for %s", ci->name);
        ci->library = os_strdup("");
        while (d != NULL) {
            u_cfDataFree(d);
            d = u_cfData(c_iterTakeFirst(iter));
        }
        r = FALSE;
    }
    c_iterFree(iter);

    return r;
}

#ifndef INTEGRITY
static c_bool
cfgGetSchedule(
    sr_componentInfo ci,
    u_cfElement info)
{
    c_iter iter;
    int      iterLength;
    c_bool r;
    u_cfData d;
    c_char *str;

    r = TRUE;
    iter = u_cfElementXPath(info, "Scheduling/Class/#text");
    iterLength = c_iterLength(iter);
    d = u_cfData(c_iterTakeFirst(iter));
    if (iterLength == 1) {
        r = u_cfDataStringValue(d, &str);

        if (r == TRUE) {
            if (strncmp(str, SCHED_RT, strlen(SCHED_RT)) == 0) {
                ci->procAttr.schedClass = OS_SCHED_REALTIME;
            } else {
                if (strncmp(str, SCHED_TS, strlen(SCHED_TS)) == 0) {
                    ci->procAttr.schedClass = OS_SCHED_TIMESHARE;
                } else {
                    if (strcmp(str, SCHED_DEF)==0) {
                        ci->procAttr.schedClass = OS_SCHED_DEFAULT;
                    } else {
                        ci->procAttr.schedClass = OS_SCHED_DEFAULT;
                        OS_REPORT_1(OS_WARNING, OSRPT_CNTXT_SPLICED,
                             0, "Incorrect <Scheduling/Class> parameter for %s -> default",
                             ci->name);
                         r = TRUE;
                    }
                }
            }
            os_free(str);
        }
        u_cfDataFree(d);
    } else {
        ci->procAttr.schedClass = OS_SCHED_DEFAULT;
        if (iterLength == 0) {
        } else {
            OS_REPORT_1(OS_ERROR, OSRPT_CNTXT_SPLICED,
                0, "One <Scheduling/Class> parameter expected for %s", ci->name);
            r = FALSE;
        }
        while (d != NULL) {
            u_cfDataFree(d);
            d = u_cfData(c_iterTakeFirst(iter));
        }
    }
    c_iterFree(iter);

    return r;
}

static c_bool
cfgGetPriority(
    sr_componentInfo ci,
    u_cfElement info)
{
    c_iter iter;
    int      iterLength;
    c_bool r;
    u_cfData d;
    c_long prio;

    r = TRUE;
    iter = u_cfElementXPath(info, "Scheduling/Priority/#text");
    iterLength = c_iterLength(iter);
    d = u_cfData(c_iterTakeFirst(iter));
    if (iterLength == 1) {
        r = u_cfDataLongValue(d, &prio);

        if (r == TRUE) {
            ci->procAttr.schedPriority = (os_int32)prio;
        } else {
            OS_REPORT_1(OS_WARNING, OSRPT_CNTXT_SPLICED,
                0, "Incorrect <Scheduling/Priority> parameter for %s -> default", ci->name);
            r = TRUE;
        }
        u_cfDataFree(d);
    } else {
        if (iterLength == 0) {
        } else {
            OS_REPORT_1(OS_ERROR, OSRPT_CNTXT_SPLICED,
                0, "One <Scheduling/Priority> parameter expected for %s", ci->name);
            r = FALSE;
        }
        while (d != NULL) {
            u_cfDataFree(d);
            d = u_cfData(c_iterTakeFirst(iter));
        }
    }
    c_iterFree(iter);

    return r;
}
#endif

static c_bool
cfgGetPriorityKind(
    sr_componentInfo ci,
    u_cfElement info)
{
    c_iter iter;
    int      iterLength;
    c_bool r;
    u_cfElement d;
    c_char * str;

    r = TRUE;
    iter = u_cfElementXPath(info, "Scheduling/Priority");
    iterLength = c_iterLength(iter);
    d = u_cfElement(c_iterTakeFirst(iter));
    if (iterLength == 1) {
        r = u_cfElementAttributeStringValue(d, ATTR_PRIOKIND, &str);

        if (r == TRUE) {
            if (strcmp(str, PRIOKIND_REL) == 0) {
                ci->priorityKind = V_SCHED_PRIO_RELATIVE;
            } else {
                if (strcmp(str, PRIOKIND_ABS) == 0) {
                    ci->priorityKind = V_SCHED_PRIO_ABSOLUTE;
                } else {
                    ci->priorityKind = V_SCHED_PRIO_RELATIVE;
                    OS_REPORT_1(OS_WARNING, OSRPT_CNTXT_SPLICED,
                         0, "Incorrect <Scheduling/Priority[@priority_kind]> attribute for %s -> default",
                         ci->name);
                     r = TRUE;
                }
            }
            os_free(str);
        }
        u_cfElementFree(d);
    } else {
        if (iterLength == 0) {
            ci->priorityKind = V_SCHED_PRIO_RELATIVE;
        } else {
            OS_REPORT_1(OS_ERROR, OSRPT_CNTXT_SPLICED,
                0, "One <Scheduling/Priority[@priority_kind]> parameter expected for %s", ci->name);
            r = FALSE;
        }
        while (d != NULL) {
            u_cfElementFree(d);
            d = u_cfElement(c_iterTakeFirst(iter));
        }
    }
    c_iterFree(iter);

    return r;
}

static c_bool
cfgGetRestartRule(
    sr_componentInfo ci,
    u_cfElement info)
{
    c_iter iter;
    int iterLength;
    c_bool r;
    u_cfData d;
    c_char *str;

    r = TRUE;

    iter = u_cfElementXPath(info, "FailureAction/#text");
    iterLength = c_iterLength(iter);
    d = u_cfData(c_iterTakeFirst(iter));
    if (iterLength == 1) {
        r = u_cfDataStringValue(d, &str);

        if (r == TRUE) {
#ifndef INTEGRITY
            if (strncmp(str, RR_KILL_STR, strlen(RR_KILL_STR)) == 0) {
                ci->restartRule = RR_KILL;
            }
            if (strncmp(str, RR_RESTART_STR, strlen(RR_RESTART_STR)) == 0) {
                ci->restartRule = RR_RESTART;
            }
            if (strncmp(str, RR_HALT_STR, strlen(RR_HALT_STR)) == 0) {
                ci->restartRule = RR_HALT;
            }
#endif
            if (strncmp(str, RR_SKIP_STR, strlen(RR_SKIP_STR)) == 0) {
                ci->restartRule = RR_SKIP;
            }
            if (ci->restartRule == RR_NONE) {
                OS_REPORT_1(OS_WARNING, OSRPT_CNTXT_SPLICED,
                            0,
                            "Incorrect <FailureAction> parameter for %s -> default",
                            ci->name);
                ci->restartRule = RR_SKIP;
            }
            os_free(str);
        }
        u_cfDataFree(d);
    } else {
        if (iterLength == 0) {
            ci->restartRule = RR_SKIP;
        } else {
            OS_REPORT_1(OS_ERROR, OSRPT_CNTXT_SPLICED,
                0, "One <FailureAction> parameter expected for %s", ci->name);
            ci->restartRule = RR_SKIP;
            r = FALSE;
        }
        while (d != NULL) {
            u_cfDataFree(d);
            d = u_cfData(c_iterTakeFirst(iter));
        }
    }
    c_iterFree(iter);

    return r;
}

static void
cfgComponentInfoInit(
    sr_componentInfo ci)
{
#ifndef INTEGRITY
    os_procAttrInit(&ci->procAttr);
#endif
    ci->isService = OS_FALSE;
    ci->restartRule = RR_NONE;
    ci->name = NULL;
    ci->command = NULL;
    ci->configuration = NULL;
    ci->args = NULL;
    ci->library = NULL;
    ci->procId = OS_INVALID_PID;
}

static int
cfgGetServiceComponentInfo(
    sr_componentInfo ci,
    u_cfElement info,
    const char *defaultConfigURI)
{
    c_bool r = FALSE;
    c_bool enabled = TRUE;

    cfgComponentInfoInit(ci);
    ci->isService = OS_TRUE;

    (void)u_cfElementAttributeStringValue(info, ATTR_NAME, &ci->name);
    u_cfElementAttributeBoolValue(info, ATTR_ENABLED, &enabled);

    /* Services have support for name, command, configuration, arguments
     * schedule, priority and restart rule */
    if (enabled) {
        r = cfgGetCommand(ci, info);
        if (r == TRUE) {
            (void)cfgGetConfiguration(ci, info, defaultConfigURI);
            (void)cfgGetArguments(ci, info);
#ifndef INTEGRITY
            (void)cfgGetSchedule(ci, info);
            (void)cfgGetPriority(ci, info);
#endif
            (void)cfgGetPriorityKind(ci, info);
            (void)cfgGetRestartRule(ci, info);
        }
    } else {
        OS_REPORT_1(OS_INFO, OSRPT_CNTXT_SPLICED, 0, "Service %s disabled", ci->name);
    }

    return !((int)r);
}

static int
cfgGetApplicationComponentInfo(
    sr_componentInfo ci,
    u_cfElement info,
    const char *defaultConfigURI)
{
    c_bool r = FALSE;
    c_bool enabled = TRUE;

    cfgComponentInfoInit(ci);

    (void)u_cfElementAttributeStringValue(info, ATTR_NAME, &ci->name);
    u_cfElementAttributeBoolValue(info, ATTR_ENABLED, &enabled);

    /* Applications currently only have name, command, arguments and library supported */
    if (enabled) {
        r = cfgGetCommand(ci, info);
        if (r == TRUE) {
            (void)cfgGetArguments(ci, info);
            (void)cfgGetLibrary(ci, info);
        }
    } else {
        OS_REPORT_1(OS_INFO, OSRPT_CNTXT_SPLICED, 0, "Application %s disabled", ci->name);
    }

    return !((int)r);
}

/**************************************************************
 * constructor/destructor
 **************************************************************/
sr_componentInfo
sr_componentInfoServiceNew(
    u_cfElement info,
    const char *defaultConfigURI)
{
    sr_componentInfo ci;
    int result;

    if (info != NULL) {
        ci = (sr_componentInfo)os_malloc((os_uint32)sizeof(C_STRUCT(sr_componentInfo)));
        if (ci != NULL) {
            result = cfgGetServiceComponentInfo(ci, info, defaultConfigURI);
            if (result) {
                sr_componentInfoFree(ci);
                ci = NULL;
            }
        }
    } else {
        ci = NULL;
    }

    return ci;
}

sr_componentInfo
sr_componentInfoApplicationNew(
    u_cfElement info,
    const char *defaultConfigURI)
{
    sr_componentInfo ci;
    int result;

    if (info != NULL) {
        ci = (sr_componentInfo)os_malloc((os_uint32)sizeof(C_STRUCT(sr_componentInfo)));
        if (ci != NULL) {
            result = cfgGetApplicationComponentInfo(ci, info, defaultConfigURI);
            if (result) {
                sr_componentInfoFree(ci);
                ci = NULL;
            }
        }
    } else {
        ci = NULL;
    }

    return ci;
}

void
sr_componentInfoFree(
    sr_componentInfo componentInfo)
{
    if (componentInfo != NULL) {
        os_free(componentInfo->name);
        os_free(componentInfo->command);
        os_free(componentInfo->configuration);
        os_free(componentInfo->args);
        os_free(componentInfo->library);
        os_free(componentInfo);
    }
}

/**************************************************************
 * Protected functions
 **************************************************************/

/**************************************************************
 * Public functions
 **************************************************************/

