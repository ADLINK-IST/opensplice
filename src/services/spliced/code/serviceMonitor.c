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
#include "vortex_os.h"
#include "os_sharedmem.h"
#include "v_group.h"
#include "v_groupSet.h"

#include "v_event.h"

#include "u_observable.h"
#include "u_serviceManager.h"

#include "report.h"
#include "s_shmMonitor.h"
#include "serviceMonitor.h"

C_STRUCT(serviceMonitor)
{
    spliced spliceDaemon;
    u_serviceManager serviceManager;
};

static c_ulong
serviceMonitorMain(
    u_observable o,
    c_ulong event,
    c_voidp usrData)
{
    c_iter diedServices, incompatibleServices;
    c_char *name;
    sr_componentInfo info;
    u_serviceManager serviceManager = (u_serviceManager)o;
    serviceMonitor this = (serviceMonitor)usrData;
    c_bool removed;

    assert(this->serviceManager == serviceManager);

    /* handle the died services */
    diedServices = u_serviceManagerGetServices(serviceManager, STATE_DIED);
    name = (c_char *)c_iterTakeFirst(diedServices);
    while (name != NULL) {
        /* check if restart is needed */
        info = splicedGetServiceInfo(this->spliceDaemon, name);
        if (info != NULL) {
            serviceMonitorProcessDiedservice(this, info);
            removed = u_serviceManagerRemoveService(serviceManager,name);
            if (!removed) {
                OS_REPORT(OS_ERROR, OSRPT_CNTXT_SPLICED, 0,
                   "Could not remove service %s from the serviceset", name);
            }
        } else {
            OS_REPORT(OS_ERROR, OSRPT_CNTXT_SPLICED, 0,
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
            OS_REPORT(OS_INFO, OSRPT_CNTXT_SPLICED, 0,
                "Detected incompatible service '%s' STATE_INCOMPATIBLE_CONFIGURATION -> systemhalt", info->name);
            splicedSignalTerminate(this->spliceDaemon, SPLICED_EXIT_CODE_RECOVERABLE_ERROR, SPLICED_SHM_OK);
            splicedRemoveKnownService(this->spliceDaemon, info->name);
            removed = u_serviceManagerRemoveService(serviceManager,name);
            if (!removed) {
                OS_REPORT(OS_ERROR, OSRPT_CNTXT_SPLICED, 0,
                   "Could not remove incompatible service %s from the serviceset", name);
            }
        } else {
            OS_REPORT(OS_ERROR, OSRPT_CNTXT_SPLICED, 0,
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
    os_duration sleepTime;
    os_int32 dummy = 0;
    int count = 0;
    os_result result;

    /* The child process that terminates remains in a zombie state until it has
     * has os_procCheckStatus called on it (especially true for Posix).  Try for
     * up to 10 seconds.
     */
    sleepTime = 100*OS_DURATION_MILLISECOND;
    while (((result = os_procCheckStatus(info->procId, &dummy)) == os_resultBusy) && (count < 100)) {
        count++;
        ospl_os_sleep(sleepTime);
    }

#if !defined OS_WIN32_DEFS_H
    if (result == os_resultBusy) {
        /* The process didn't quit properly. Kill it and re-wait. */
        os_procDestroy(info->procId, OS_SIGKILL);
        count = 0;
        while (((result = os_procCheckStatus(info->procId, &dummy) == os_resultBusy)) && (count < 100)) {
            count++;
            ospl_os_sleep(sleepTime);
        }
    }
#endif

}

void
serviceMonitorProcessDiedservice(
    serviceMonitor monitor,
    sr_componentInfo info)
{
    c_char *args;
    os_size_t argc;
    os_result procCreateResult;
    c_char *diedscript;
    os_procAttr diedscript_procAttr;
    os_procId diedscript_procId;
    os_result result;
    os_int32 status;
    os_timeM endTime, now;

    assert(monitor);
    assert(monitor->serviceManager);
    assert(monitor->spliceDaemon);
    assert(info);

    /* The service has encountered a terminal error and has not gone through
     * its own formal termination routine, which would normally include
     * deregistering itself from the kernel.  To maintain the correctness of
     * the kernel's user count, we perform a step to decrement that count on
     * behalf of the service that has died.   The detachServiceFromKernel
     * action is performed in all cases except RR_SKIP
     */

    /* This is an undocumented and unsupported feature to assist in debugging.
     * If the OSPL_DIED_SCRIPT environment variable is set then the script
     * pointed to by OSPL_DIED_SCRIPT is executed just before a process
     * is about to die. This could be used for example to create a stack trace
     * of the process just before it dies.
     */
    diedscript = os_getenv("OSPL_DIED_SCRIPT");
    if (diedscript) {
        /* Execute the script and provide the pid of the process as argument */
        argc = 12;
        args = os_malloc(argc);
        snprintf(args, argc, "%d", info->procId);
        os_procAttrInit(&diedscript_procAttr);
        procCreateResult = os_procCreate(diedscript, "diedscript", args, &diedscript_procAttr, &diedscript_procId);
        if (procCreateResult != os_resultSuccess) {
            OS_REPORT(OS_ERROR, "os_procServiceDestroy", 0, "Service %s (%d) is about to die, executing died script '%s' failed", info->name, info->procId, diedscript);
        } else {
            OS_REPORT(OS_INFO, "os_procServiceDestroy", 0, "Service %s (%d) is about to die, executing died script '%s' succeeded", info->name, info->procId, diedscript);
            /* wait until the script has finished or wait max. 5 sec
             * before actually killing the process
             */
            now = os_timeMGet();
            endTime = os_timeMAdd(now, 5*OS_DURATION_SECOND);
            while (os_timeMCompare(now, endTime) == OS_LESS) {
                result = os_procCheckStatus(diedscript_procId, &status);
                if (result != os_resultBusy) {
                    break;
                }
                ospl_os_sleep(100*OS_DURATION_MILLISECOND);
                now = os_timeMGet();
            }
        }
        os_free(args);
    }

    switch (info->restartRule) {
#ifndef INTEGRITY
    case RR_KILL:
        OS_REPORT(OS_INFO, OSRPT_CNTXT_SPLICED, 0,
            "Service '%s' DIED -> kill", info->name);

        waitForDiedService(info);
        splicedRemoveKnownService(monitor->spliceDaemon, info->name);
    break;
    case RR_RESTART:
        argc = strlen(info->name)+4+1+strlen(info->configuration)+4+strlen(info->args)+1;
        args = os_malloc(argc);
        snprintf(args, argc, "\"%s\" \"%s\" %s", info->name, info->configuration, info->args);
        OS_REPORT(OS_INFO, OSRPT_CNTXT_SPLICED, 0, "Service '%s' DIED -> restart", info->name);

        waitForDiedService(info);

        /* Only restart the service when shm is clean, when unclean the
         * domain will terminate.
         */
        if ((s_shmMonitorIsClean(splicedGetShmMonitor(monitor->spliceDaemon))) &&
            (!splicedIsDoingSystemHalt(monitor->spliceDaemon))) {
            result = os_procCreate(info->command, info->name, args, &info->procAttr, &info->procId);
            if (result != os_resultSuccess) {
                OS_REPORT(OS_ERROR, OSRPT_CNTXT_SPLICED,
                        0, "Could not restart service '%s'",
                        info->name);
                splicedRemoveKnownService(monitor->spliceDaemon, info->name);
            } else {
                /* we do not remove the registration of the old user process we just
                 * restarted, as the 'os_sharedMemoryRegisterUserProcess' must ensure
                 * that the registration of the user process can be uniquely identified
                 * by also registrating the creation time of the process for example
                 */
                os_sharedMemoryRegisterUserProcess(splicedGetDomainName(monitor->spliceDaemon), info->procId);
                OS_REPORT(OS_INFO, OSRPT_CNTXT_SPLICED,
                        0, "Restarted service '%s'",
                        info->name);
            }
        } else {
            splicedRemoveKnownService(monitor->spliceDaemon, info->name);
        }
        os_free(args);
    break;
    case RR_HALT:
        OS_REPORT(OS_INFO, OSRPT_CNTXT_SPLICED, 0,
                    "Service '%s' DIED -> systemhalt", info->name);
        splicedSignalTerminate(monitor->spliceDaemon, SPLICED_EXIT_CODE_RECOVERABLE_ERROR, SPLICED_SHM_OK);
        splicedRemoveKnownService(monitor->spliceDaemon, info->name);
    break;
#endif /* INTEGRITY */
    case RR_SKIP:
        OS_REPORT(OS_INFO, OSRPT_CNTXT_SPLICED, 0,
            "Service '%s' DIED -> skip", info->name);
        splicedRemoveKnownService(monitor->spliceDaemon, info->name);
    break;
    default:
        OS_REPORT(OS_WARNING, OSRPT_CNTXT_SPLICED,
                  0, "Unknown restart rule, default to 'skip'");
    break;
    }
}

serviceMonitor
serviceMonitorNew(
    spliced spliceDaemon)
{
    serviceMonitor this;

    this = (serviceMonitor)os_malloc((os_uint32)C_SIZEOF(serviceMonitor));

    if (this != NULL) {
        this->spliceDaemon = spliceDaemon;
        this->serviceManager = splicedGetServiceManager(this->spliceDaemon);
        u_observableAddListener(u_observable(this->serviceManager),
                                V_EVENT_SERVICESTATE_CHANGED,
                                serviceMonitorMain,
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
        u_observableRemoveListener(u_observable(this->serviceManager), serviceMonitorMain);
        os_free(this);
    }
}

void
serviceMonitorStop(
    serviceMonitor this)
{
    if (this != NULL) {
        u_observableRemoveListener(u_observable(this->serviceManager), serviceMonitorMain);
        ut_threadAwake(ut_threadLookupSelf(splicedGetThreads(this->spliceDaemon)));
    }
}
