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
#include "os.h"
#include "os_sharedmem.h"
#include "v_group.h"
#include "v_groupSet.h"

#include "v_event.h"

#include "report.h"
#include "serviceMonitor.h"

C_STRUCT(serviceMonitor)
{
    spliced spliceDaemon;
    u_serviceManager serviceManager;
};

static void
detachService(
    u_serviceManager serviceManager,
    sr_componentInfo info);




/**************************************************************
 * Private functions
 **************************************************************/
static c_ulong
serviceMonitorMain(
    u_serviceManager serviceManager,
    c_ulong event,
    c_voidp usrData)
{
    c_iter diedServices, incompatibleServices;
    c_char *name;
    sr_componentInfo info;
    serviceMonitor this = (serviceMonitor)usrData;
    c_bool removed;

    /* handle the died services */
    diedServices = u_serviceManagerGetServices(serviceManager, STATE_DIED);
    name = (c_char *)c_iterTakeFirst(diedServices);
    while (name != NULL) {
        /* check if restart is needed */
        info = splicedGetServiceInfo(this->spliceDaemon, name);
        if (info != NULL) {
            serviceMonitorProcessDiedservice(serviceManager, info);
            removed = u_serviceManagerRemoveService(serviceManager,name);
            if (!removed) {
                OS_REPORT_1(OS_ERROR, OSRPT_CNTXT_SPLICED, 0,
                   "Could not remove service %s from the serviceset", name);
            }
        } else {
            OS_REPORT_1(OS_ERROR, OSRPT_CNTXT_SPLICED, 0,
                "Unknown service '%s' died", name);
        }
        os_free(name);
        name = (c_char *)c_iterTakeFirst(diedServices);
    }
    c_iterFree(diedServices);

    /* handle the incompatible services */
    incompatibleServices = u_serviceManagerGetServices(serviceManager, STATE_INCOMPATIBLE_CONFIGURATION);
    name = (c_char *)c_iterTakeFirst(incompatibleServices);
    while (name != NULL) {
        /* check if restart is needed */
        info = splicedGetServiceInfo(this->spliceDaemon, name);
        if (info != NULL) {
            OS_REPORT_1(OS_INFO, OSRPT_CNTXT_SPLICED, 0,
                "Detected incompatible service '%s' STATE_INCOMPATIBLE_CONFIGURATION -> systemhalt", info->name);
            splicedDoSystemHalt(SPLICED_EXIT_CODE_RECOVERABLE_ERROR);
            splicedRemoveKnownService(info->name);
            removed = u_serviceManagerRemoveService(serviceManager,name);
            if (!removed) {
                OS_REPORT_1(OS_ERROR, OSRPT_CNTXT_SPLICED, 0,
                   "Could not remove incompatible service %s from the serviceset", name);
            }
        } else {
            OS_REPORT_1(OS_ERROR, OSRPT_CNTXT_SPLICED, 0,
                "Unknown incompatible service '%s' died", name);
        }
        os_free(name);
        name = (c_char *)c_iterTakeFirst(incompatibleServices);
    }
    c_iterFree(incompatibleServices);

    return event;
}

void
waitForDiedService(
    sr_componentInfo info)
{
    os_time sleepTime;
    os_int32 dummy = 0;
    int count = 0;
    os_result result;

    /* The child process that terminates remains in a zombie state until it has
     * has os_procCheckStatus called on it (especially true for Posix).  Try for
     * up to 10 seconds.
     */
    sleepTime.tv_sec = 0;
    sleepTime.tv_nsec = 100000000;
    while (((result = os_procCheckStatus(info->procId, &dummy) == os_resultBusy)) && (count < 100)) {
        count++;
        os_nanoSleep(sleepTime);
    }
    if (result == os_resultSuccess) {
        info->procId = OS_INVALID_PID;
    }
}

void
detachServiceFromKernel (
    v_entity entity,
    c_voidp args
    )
{
    v_kernel kernel = v_objectKernel(entity);
    OS_UNUSED_ARG(args);
    v_kernelDetach(kernel);
}


static c_bool
removeServiceFromGroup(
    c_object o,
    c_voidp args)
{
    v_group g = v_group(o);
    char * serviceName = (char *)args;
    v_groupRemoveAwareness(g,serviceName);
    return TRUE;
}

/* this is needed so that the group attachedServices and notInterestedServices list count
 * will be in sync with the v_kernelNetworkCount see OSPL-2219*/
void
detachServiceFromGroup(
    v_entity entity,
    c_voidp args
    )
{
    v_kernel kernel = v_objectKernel(entity);
    v_groupSetWalk(kernel->groupSet,removeServiceFromGroup,args);
}

static void
detachService(
    u_serviceManager serviceManager,
    sr_componentInfo info)
{
    u_result result;
    assert(serviceManager);
    assert(info);

    result = u_entityAction(u_entity(serviceManager), detachServiceFromGroup, info->name);
    if (result != U_RESULT_OK) {
        OS_REPORT_1(OS_WARNING, OSRPT_CNTXT_SPLICED, 0,
            "Entity action detach service from groups on service-manager failed (%s)",
            u_resultImage(result));
    }

    result = u_entityAction(u_entity(serviceManager), detachServiceFromKernel, NULL);
    if (result != U_RESULT_OK) {
        OS_REPORT_1(OS_WARNING, OSRPT_CNTXT_SPLICED, 0,
            "Entity action detach service from kernel on service-manager failed (%s)",
            u_resultImage(result));
    }

}

void
serviceMonitorProcessDiedservice(
    u_serviceManager serviceManager,
    sr_componentInfo info)
{
    c_char *args;
    int argc;
    os_result procCreateResult;

    assert(info);

    /* The service has encountered a terminal error and has not gone through
     * its own formal termination routine, which would normally include
     * deregistering itself from the kernel.  To maintain the correctness of
     * the kernel's user count, we perform a step to decrement that count on
     * behalf of the service that has died.   The detachServiceFromKernel
     * action is performed in all cases except RR_SKIP
     */

    switch (info->restartRule) {
#ifndef INTEGRITY
    case RR_KILL:
        OS_REPORT_1(OS_INFO, OSRPT_CNTXT_SPLICED, 0,
            "Service '%s' DIED -> kill", info->name);

#if !defined OS_WIN32_DEFS_H
        os_procDestroy(info->procId, OS_SIGKILL);
#endif
        waitForDiedService(info);
        /* Kill send to service, detach from kernel */
        detachService(serviceManager, info);
        splicedRemoveKnownService(info->name);
    break;
    case RR_RESTART:
        argc = strlen(info->name)+4+1+strlen(info->configuration)+4+strlen(info->args)+1;
        args = os_malloc(argc);
        if (args) {
            snprintf(args, argc, "\"%s\" \"%s\" %s", info->name, info->configuration, info->args);
        }
        OS_REPORT_1(OS_INFO, OSRPT_CNTXT_SPLICED, 0,
            "Service '%s' DIED -> restart", info->name);

        {
            os_time sleepTime;
            sleepTime.tv_sec = 2;
            sleepTime.tv_nsec = 0;
            os_nanoSleep(sleepTime);
        }

#if !defined OS_WIN32_DEFS_H && !defined INTEGRITY
        os_procDestroy(info->procId, OS_SIGKILL);
#endif

        waitForDiedService(info);
        /* Kill send to service, detach from kernel */
        detachService(serviceManager, info);

        procCreateResult = os_procCreate(info->command,
            info->name, args,
            &info->procAttr, &info->procId);
        if (procCreateResult != os_resultSuccess) {
            OS_REPORT_1(OS_ERROR, OSRPT_CNTXT_SPLICED,
                0, "Could not restart service '%s'",
                info->name);
        } else {
            /* we do not remove the registration of the old user process we just
             * restarted, as the 'os_sharedMemoryRegisterUserProcess' must ensure
             * that the registration of the user process can be uniquely identified
             * by also registrating the creation time of the process for example
             */
            os_sharedMemoryRegisterUserProcess(splicedGetDomainName(), info->procId);
            OS_REPORT_1(OS_INFO, OSRPT_CNTXT_SPLICED,
                0, "Restarted service '%s'",
                info->name);
        }
        os_free(args);
    break;
    case RR_HALT:
        OS_REPORT_1(OS_INFO, OSRPT_CNTXT_SPLICED, 0,
            "Service '%s' DIED -> systemhalt", info->name);
        splicedDoSystemHalt(SPLICED_EXIT_CODE_RECOVERABLE_ERROR);
        splicedRemoveKnownService(info->name);
    break;
#endif /* INTEGRITY */
    case RR_SKIP:
        OS_REPORT_1(OS_INFO, OSRPT_CNTXT_SPLICED, 0,
            "Service '%s' DIED -> skip", info->name);
        splicedRemoveKnownService(info->name);
    break;
    default:
        if (info->restartRule != RR_SKIP) {
            OS_REPORT(OS_WARNING, OSRPT_CNTXT_SPLICED,
                      0, "Unknown restart rule, default to 'skip'");
        }
    break;
    }
}

/**************************************************************
 * constructor/destructor
 **************************************************************/
serviceMonitor
serviceMonitorNew(
    spliced spliceDaemon)
{
    serviceMonitor this;
    u_result result = U_RESULT_UNDEFINED;

    this = (serviceMonitor)os_malloc((os_uint32)C_SIZEOF(serviceMonitor));

    if (this != NULL) {
        this->spliceDaemon = spliceDaemon;
        this->serviceManager = splicedGetServiceManager(this->spliceDaemon);

        result = u_dispatcherSetEventMask(u_dispatcher(this->serviceManager),
            V_EVENT_SERVICESTATE_CHANGED);
        if (result != U_RESULT_OK) {
            OS_REPORT_1(OS_ERROR, "serviceMonitorNew", result,
                "Failed to set dispatcher event mask on service-manager (%s)",
                u_resultImage(result));
        } else {
            result = u_dispatcherInsertListener(u_dispatcher(this->serviceManager),
                (u_dispatcherListener)serviceMonitorMain,
                (c_voidp)this);
            if (result != U_RESULT_OK) {
                OS_REPORT_1(OS_ERROR, "serviceMonitorNew", result,
                    "Failed to create service-monitor listener (%s)",
                    u_resultImage(result));
            }
        }
    }

    if (result != U_RESULT_OK) {
        os_free(this);
        this = NULL;
    }

    return this;
}

void
serviceMonitorFree(
    serviceMonitor this)
{
    u_result result;

    if (this != NULL) {
        /* stop listening for services */
        result = u_dispatcherSetEventMask(u_dispatcher(this->serviceManager), 0);
        if (result != U_RESULT_OK) {
            OS_REPORT_1(OS_ERROR, "serviceMonitorFree", result,
                "Failed to unset dispatcher event mask on service-manager (%s)",
                u_resultImage(result));
        }

        result = u_dispatcherRemoveListener(u_dispatcher(this->serviceManager),
            (u_dispatcherListener)serviceMonitorMain);
        if (result != U_RESULT_OK) {
            OS_REPORT_1(OS_ERROR, "serviceMonitorFree", result,
                "Failed to remove service-monitor listener (%s)",
                u_resultImage(result));
        }
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
