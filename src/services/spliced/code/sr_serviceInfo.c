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

#include "report.h"

#include "sr_serviceInfo.h"

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
    sr_serviceInfo si,
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
        r = u_cfDataStringValue(d, &si->command);
        u_cfDataFree(d);
    } else {
        OS_REPORT_1(OS_ERROR, OSRPT_CNTXT_SPLICED, 
            0, "One <Command> parameter expected for service %s", si->name);
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
    sr_serviceInfo si,
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
        r = u_cfDataStringValue(d, &si->configuration);
        u_cfDataFree(d);
    } else if (iterLength == 0) {
        OS_REPORT_1(OS_INFO, OSRPT_CNTXT_SPLICED, 
            0, "Taking default for <Configuration> parameter service %s", si->name);
        si->configuration = os_strdup(defaultURI);
    } else {
        OS_REPORT_1(OS_ERROR, OSRPT_CNTXT_SPLICED, 
            0, "One <Configuration> parameter expected for service %s", si->name);
        si->configuration = os_strdup(defaultURI);
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
    sr_serviceInfo si,
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
        r = u_cfDataStringValue(d, &si->args);
        u_cfDataFree(d);
    } else if (iterLength == 0) {
        OS_REPORT_1(OS_INFO, OSRPT_CNTXT_SPLICED, 
            0, "Taking default for <Arguments> parameter service %s", si->name);
        si->args = os_strdup("");
    } else {
        OS_REPORT_1(OS_ERROR, OSRPT_CNTXT_SPLICED, 
            0, "One <Arguments> parameter expected for service %s", si->name);
        si->args = os_strdup("");
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
    sr_serviceInfo si,
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
                si->procAttr.schedClass = OS_SCHED_REALTIME;
            } else {
                if (strncmp(str, SCHED_TS, strlen(SCHED_TS)) == 0) {
                    si->procAttr.schedClass = OS_SCHED_TIMESHARE;
                } else {
                    if (strcmp(str, SCHED_DEF)==0) {
                        si->procAttr.schedClass = OS_SCHED_DEFAULT;
                    } else {
                        si->procAttr.schedClass = OS_SCHED_DEFAULT;
                        OS_REPORT_1(OS_WARNING, OSRPT_CNTXT_SPLICED, 
                             0, "Incorrect <Scheduling/Class> parameter for service %s -> default",
                             si->name);
                         r = TRUE;
                    }
                }
            }
            os_free(str);
        }
        u_cfDataFree(d);
    } else {
        si->procAttr.schedClass = OS_SCHED_DEFAULT;
        if (iterLength == 0) {
            OS_REPORT_1(OS_INFO, OSRPT_CNTXT_SPLICED, 
                0, "Taking default for <Scheduling/Class> parameter service %s", si->name);
        } else {
            OS_REPORT_1(OS_ERROR, OSRPT_CNTXT_SPLICED, 
                0, "One <Scheduling/Class> parameter expected for service %s", si->name);
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
    sr_serviceInfo si,
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
            si->procAttr.schedPriority = (os_int32)prio;
        } else {
            OS_REPORT_1(OS_WARNING, OSRPT_CNTXT_SPLICED, 
                0, "Incorrect <Scheduling/Priority> parameter for service %s -> default", si->name);
            r = TRUE;
        }
        u_cfDataFree(d);
    } else {
        if (iterLength == 0) {
            OS_REPORT_1(OS_INFO, OSRPT_CNTXT_SPLICED, 
                0, "Taking default for <Scheduling/Priority> parameter service %s", si->name);
        } else {
            OS_REPORT_1(OS_ERROR, OSRPT_CNTXT_SPLICED, 
                0, "One <Scheduling/Priority> parameter expected for service %s", si->name);
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
    sr_serviceInfo si,
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
                si->priorityKind = V_SCHED_PRIO_RELATIVE;
            } else {
                if (strcmp(str, PRIOKIND_ABS) == 0) {
                    si->priorityKind = V_SCHED_PRIO_ABSOLUTE;
                } else {
                    si->priorityKind = V_SCHED_PRIO_RELATIVE;
                    OS_REPORT_1(OS_WARNING, OSRPT_CNTXT_SPLICED, 
                         0, "Incorrect <Scheduling/Priority[@priority_kind]> attribute for service %s -> default",
                         si->name);
                     r = TRUE;
                }
            }
            os_free(str);
        }
        u_cfElementFree(d);
    } else {
        if (iterLength == 0) {
            si->priorityKind = V_SCHED_PRIO_RELATIVE;
            OS_REPORT_1(OS_INFO, OSRPT_CNTXT_SPLICED, 
                0, "Taking default for <Scheduling/Priority[@priority_kind]> parameter for  service %s", si->name);
        } else {
            OS_REPORT_1(OS_ERROR, OSRPT_CNTXT_SPLICED, 
                0, "One <Scheduling/Priority[@priority_kind]> parameter expected for service %s", si->name);
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
    sr_serviceInfo si,
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
                si->restartRule = RR_KILL;
            }
            if (strncmp(str, RR_RESTART_STR, strlen(RR_RESTART_STR)) == 0) {
                si->restartRule = RR_RESTART;
            }
            if (strncmp(str, RR_HALT_STR, strlen(RR_HALT_STR)) == 0) {
                si->restartRule = RR_HALT;
            }
#endif
            if (strncmp(str, RR_SKIP_STR, strlen(RR_SKIP_STR)) == 0) {
                si->restartRule = RR_SKIP;
            }
            if (si->restartRule == RR_NONE) {
                OS_REPORT_1(OS_WARNING, OSRPT_CNTXT_SPLICED,
                            0,
                            "Incorrect <FailureAction> parameter for service %s -> default",
                            si->name);
                si->restartRule = RR_SKIP;
            }
            os_free(str);
        }
        u_cfDataFree(d);
    } else {
        if (iterLength == 0) {
            OS_REPORT_1(OS_INFO, OSRPT_CNTXT_SPLICED, 
                0, "Taking default for <FailureAction> parameter service %s", si->name);
            si->restartRule = RR_SKIP;
        } else {
            OS_REPORT_1(OS_ERROR, OSRPT_CNTXT_SPLICED, 
                0, "One <FailureAction> parameter expected for service %s", si->name);
            si->restartRule = RR_SKIP;
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

static int
cfgGetInfo(
    sr_serviceInfo si,
    u_cfElement info,
    const char *defaultConfigURI)
{
    c_bool   r = FALSE;
    c_bool   enabled = TRUE;
    
#ifndef INTEGRITY
    os_procAttrInit(&si->procAttr);
#endif
    si->restartRule = RR_NONE;
    si->name = NULL;
    si->command = NULL; 
    si->configuration = NULL;
    si->args = NULL;
    
    (void)u_cfElementAttributeStringValue(info, ATTR_NAME, &si->name);
    u_cfElementAttributeBoolValue(info, ATTR_ENABLED, &enabled);

    if (enabled) {
        r = cfgGetCommand(si, info);

        if (r == TRUE) {
            (void)cfgGetConfiguration(si, info, defaultConfigURI);
            (void)cfgGetArguments(si, info);
#ifndef INTEGRITY
            (void)cfgGetSchedule(si, info);
            (void)cfgGetPriority(si, info);
#endif
            (void)cfgGetPriorityKind(si, info);
            (void)cfgGetRestartRule(si, info);
        }
    } else {
        OS_REPORT_1(OS_INFO, OSRPT_CNTXT_SPLICED, 0, "Service %s disabled", si->name);
    }

    return !((int)r);
}

/**************************************************************
 * constructor/destructor
 **************************************************************/
sr_serviceInfo
sr_serviceInfoNew(
    u_cfElement info,
    const char *defaultConfigURI)
{
    sr_serviceInfo si;
    int result;

    /**
     * Note: We are not able to assign a reasonable value to the member
     * 'procId', since we do not know the type and the OS layer does not
     * provide us with a initial value.
     */
    if (info != NULL) {
        si = (sr_serviceInfo)os_malloc((os_uint32)sizeof(C_STRUCT(sr_serviceInfo)));
        if (si != NULL) {
            result = cfgGetInfo(si, info, defaultConfigURI);
            if (result) {
                sr_serviceInfoFree(si);
                si = NULL;
            }
        }
    } else {
        si = NULL;
    }

    return si;
}

void
sr_serviceInfoFree(
    sr_serviceInfo serviceInfo)
{
    if (serviceInfo != NULL) {
        os_free(serviceInfo->name);
        os_free(serviceInfo->command);
        os_free(serviceInfo->configuration);
        os_free(serviceInfo->args);
        os_free(serviceInfo);
    }
}

/**************************************************************
 * Protected functions
 **************************************************************/

/**************************************************************
 * Public functions
 **************************************************************/

