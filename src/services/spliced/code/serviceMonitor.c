#include <os.h>

#include <v_event.h>

#include "report.h"
#include "serviceMonitor.h"

C_STRUCT(serviceMonitor) 
{
    spliced spliceDaemon;
    u_serviceManager serviceManager;
};

/**************************************************************
 * Private functions
 **************************************************************/
static c_ulong
serviceMonitorMain(
    u_serviceManager serviceManager,
    c_ulong event,
    c_voidp usrData)
{
    c_iter diedServices;
    c_char *name;
    os_result procCreateResult;
    sr_serviceInfo info;
    c_char *args;
    int argc;

    serviceMonitor this = (serviceMonitor)usrData;

    diedServices = u_serviceManagerGetServices(serviceManager, STATE_DIED);
    name = (c_char *)c_iterTakeFirst(diedServices);
    while (name != NULL) {
        /* check if restart is needed */
        info = splicedGetServiceInfo(this->spliceDaemon, name);
        if (info != NULL) {
            switch (info->restartRule) {
#ifndef INTEGRITY
            case RR_KILL:
                OS_REPORT_1(OS_INFO, OSRPT_CNTXT_SPLICED, 0, 
                    "Service '%s' DIED -> kill", info->name);
#if !defined OS_WIN32_DEFS_H
                os_procDestroy(info->procId, SIGKILL);
#endif
            break;
            case RR_RESTART:
                argc = strlen(info->name)+4+1+strlen(info->configuration)+4+strlen(info->args)+1;
                args = os_malloc(argc);
                if (args) {
                    snprintf(args, argc, "\"%s\" \"%s\" %s", info->name, info->configuration, info->args);
                }
                OS_REPORT_1(OS_INFO, OSRPT_CNTXT_SPLICED, 0, 
                    "Service '%s' DIED -> restart", info->name);
#if !defined OS_WIN32_DEFS_H && !defined INTEGRITY 
                os_procDestroy(info->procId, SIGKILL);
#endif
                procCreateResult = os_procCreate(info->command,
                    info->name, args,
                    &info->procAttr, &info->procId);
                if (procCreateResult != os_resultSuccess) {
                    OS_REPORT_1(OS_ERROR, OSRPT_CNTXT_SPLICED,
                        0, "Could not restart service '%s'",
                        info->name);
                } else {
                    OS_REPORT_1(OS_INFO, OSRPT_CNTXT_SPLICED,
                        0, "Restarted service '%s'",
                        info->name);
                }
                os_free(args);
            break;
            case RR_HALT:
                OS_REPORT_1(OS_INFO, OSRPT_CNTXT_SPLICED, 0,
                    "Service '%s' DIED -> systemhalt", info->name);
                splicedTerminate(this->spliceDaemon);
            break;
#endif
            case RR_SKIP:
                OS_REPORT_1(OS_INFO, OSRPT_CNTXT_SPLICED, 0, 
                    "Service '%s' DIED -> skip", info->name);
            break;
            default:
                if (info->restartRule != RR_SKIP) {
                    OS_REPORT(OS_WARNING, OSRPT_CNTXT_SPLICED,
                              0, "Unknown restart rule, default to 'skip'");
                }
            break;
            }
        } else {
            OS_REPORT_1(OS_ERROR, OSRPT_CNTXT_SPLICED, 0,
                "Unknown service '%s' died", name);
        }
        os_free(name);
        name = (c_char *)c_iterTakeFirst(diedServices);
    }
    c_iterFree(diedServices);

    return event;
}

/**************************************************************
 * constructor/destructor
 **************************************************************/
serviceMonitor
serviceMonitorNew(
    spliced spliceDaemon)
{
    serviceMonitor   this;

    this = (serviceMonitor)os_malloc((os_uint32)C_SIZEOF(serviceMonitor));

    if (this != NULL) {
        this->spliceDaemon = spliceDaemon;
        this->serviceManager = splicedGetServiceManager(this->spliceDaemon);
        u_dispatcherSetEventMask(u_dispatcher(this->serviceManager),
            V_EVENT_SERVICESTATE_CHANGED);
        u_dispatcherInsertListener(u_dispatcher(this->serviceManager), 
                                  (u_dispatcherListener)serviceMonitorMain,
                                  (c_voidp)this);
    }
    return this;
}

void
serviceMonitorFree(
    serviceMonitor this)
{
    if (this != NULL) {
        /* stop listening for services */
        u_dispatcherSetEventMask(u_dispatcher(this->serviceManager), 0);
        u_dispatcherRemoveListener(u_dispatcher(this->serviceManager),
            (u_dispatcherListener)serviceMonitorMain);
        os_free(this);
    }
}

/**************************************************************
 * Protected functions
 **************************************************************/

/**************************************************************
 * Public functions
 **************************************************************/
void
serviceMonitorStop(
    serviceMonitor this)
{
    if (this != NULL) {
        u_dispatcherSetEventMask(u_dispatcher(this->serviceManager), 0);
        u_dispatcherRemoveListener(u_dispatcher(this->serviceManager),
            (u_dispatcherListener)serviceMonitorMain);
    }
}
