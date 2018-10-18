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
#include "os_report.h"

#include "u_user.h"
#include "u__user.h"
#include "u__types.h"
#include "u__object.h"
#include "u__observable.h"
#include "u__entity.h"
#include "u__domain.h"
#include "u__participant.h"
#include "u__serviceManager.h"
#include "u__cfValue.h"
#include "u__service.h"


#include "v_service.h"
#include "v_networking.h"
#include "v_durability.h"
#include "v_cmsoap.h"
#include "v_rnr.h"
#include "v_configuration.h"
#include "v_participant.h"
#include "v_entity.h"
#include "v_event.h"
#include "v_participant.h"
#include "v_leaseManager.h"
#include "v_builtin.h"

#define SERVICE_PATH "Domain/Service[@name='%s']"

struct atExitInfo {
    u_service service;
    u_serviceAtExitAction action;
    void *data;
};

static c_iter atExitInfoList = NULL;
static os_mutex atExitMutex;

static c_ulong
u_serviceSpliceListener(
    const u_observable o,
    const c_ulong event,
    const c_voidp usrData)
{
    v_serviceStateKind kind;
    u_service service = u_service(usrData);

    OS_UNUSED_ARG(o);
    OS_UNUSED_ARG(event);

    assert(service != NULL);

    kind = u_serviceManagerGetServiceStateKind(service->serviceManager, V_SPLICED_NAME);
    if ((kind != STATE_INITIALISING) && (kind != STATE_OPERATIONAL)) {
        service->callback(kind, service->usrData);
    }
    return V_EVENT_SERVICESTATE_CHANGED;
}

static int
lockPages(
    const v_kernel k,
    const c_char *path)
{
    v_configuration cfg;
    v_cfElement root;
    v_cfElement service;
    v_cfData data;
    c_iter iter;
    c_value value;
    int lock = 0;

    assert(k);
    assert(path);

    cfg = v_getConfiguration(k);
    if (cfg != NULL) {
        root = v_configurationGetRoot(cfg);
        if (root != NULL) {
            /*
             * Lookup the Service...
             */
            iter = v_cfElementXPath(root, path);
            if (iter != NULL) {
                if (c_iterLength(iter) > 1) {
                    OS_REPORT(OS_WARNING,"lockPages", U_RESULT_ILL_PARAM,
                                "Multiple (%d) configuration found for service '%s'",
                                 c_iterLength(iter), path);
                }
                service = v_cfElement(c_iterTakeFirst(iter));
                c_iterFree(iter);
            } else {
                service = NULL;
            }
            /*
             * Lookup the Service locking tag...
             */
            if (service != NULL) {
                iter = v_cfElementXPath(service, "Locking/#text");
                if (iter != NULL) {
                    if (c_iterLength(iter) > 1) {
                        OS_REPORT(OS_WARNING,"lockPages", U_RESULT_ILL_PARAM,
                                    "Multiple (%d) locking tags found for service '%s'",
                                     c_iterLength(iter), path);
                    }
                    data = v_cfData(c_iterTakeFirst(iter));
                    if (data != NULL) {
                        if (u_cfValueScan(v_cfDataValue(data), V_BOOLEAN, &value)) {
                            if (value.is.Boolean) {
                                lock = 1;
                            }
                        }
                    }
                    c_iterFree(iter);
                }
            }
            c_free(root);
        }
    }
    return lock;
}


static c_bool
serviceAtExitInfoRemoveCondition(
    void *o,
    c_iterActionArg arg)
{
    struct atExitInfo *info = (struct atExitInfo*)(o);
    u_service _this = u_service(arg);
    return ((_this == info->service) ? TRUE : FALSE);
}


static void
serviceAtExitInfoRemove(
    u_service _this)
{
    struct atExitInfo *info;

    /* See the comment at the start of u_service.h for more
     * information about serviceAtExit functionality.
     */

    os_mutexLock(&atExitMutex);
    info = c_iterTakeAction(atExitInfoList,
                            serviceAtExitInfoRemoveCondition,
                            _this);
    os_mutexUnlock(&atExitMutex);

    if (info != NULL) {
        os_free(info);
    }
}

static u_result
serviceAtExitInfoAdd(
    u_service _this,
    u_serviceAtExitAction action,
    void *privateData)
{
    u_result r = U_RESULT_OUT_OF_MEMORY;
    struct atExitInfo *info;

    /* See the comment at the start of u_service.h for more
     * information about serviceAtExit functionality.
     */

    info = os_malloc(sizeof(struct atExitInfo));

    /* Don't c_keep this service, otherwise it will never be freed.
     * Anyway, it will be removed from the list in the related deinit anyway.
     */
    info->service  = _this;
    info->action   = action;
    info->data     = privateData;
    os_mutexLock(&atExitMutex);
    atExitInfoList = c_iterAppend(atExitInfoList, info);
    if (atExitInfoList != NULL) {
        r = U_RESULT_OK;
    }
    os_mutexUnlock(&atExitMutex);

    return r;
}

static void
serviceAtExitWalkAction(
    void *o,
    c_iterActionArg arg)
{
    struct atExitInfo *info = (struct atExitInfo*)(o);
    OS_UNUSED_ARG(arg);
    /* Call the atExit function of the given service. */
    info->action(info->service, info->data);
}

void
u__serviceInitialise(void)
{
    os_mutexInit(&atExitMutex, NULL);
}

void
u__serviceExit(
    void)
{
    os_duration delay = OS_DURATION_INIT(0, 100000000); /* 100 ms */

    /* See the comment at the start of u_service.h for more
     * information about serviceAtExit functionality. */

    os_mutexLock(&atExitMutex);
    if (atExitInfoList != NULL) {
        /* Notify all services so that they can initiate their termination. */
        c_iterWalk(atExitInfoList, serviceAtExitWalkAction, NULL);

        /* Wait for the services to be terminated. */
        while (c_iterLength(atExitInfoList) > 0) {
            os_mutexUnlock(&atExitMutex);
            ospl_os_sleep(delay);
            os_mutexLock(&atExitMutex);
        }

        c_iterFree(atExitInfoList);
        atExitInfoList = NULL;
    }
    os_mutexUnlock(&atExitMutex);

    /* Never need the mutex anymore. */
    os_mutexDestroy(&atExitMutex);
}

u_result
u__serviceDeinitW(
    void *_vthis)
{
    u_object _this = _vthis;
    u_result r;

    assert(_this != NULL);

    (void)u_observableRemoveListener(u_observable(_this), u_serviceSpliceListener);
    (void)u_objectFree(u_service(_this)->serviceManager);
    r = u__participantDeinitW(_this);
    serviceAtExitInfoRemove(u_service(_this));

    return r;
}

void
u__serviceFreeW(
    void *_this)
{
    u__participantFreeW(_this);
}

u_service
u_serviceNewSpecialized (
    void * (*v_new) (v_kernel kernel, const c_char *name, const c_char *extStateName, v_participantQos qos, c_bool enable, void *arg),
    const c_char *serviceConfigName,
    const os_char *uri,
    const u_domainId_t id,
    const os_int32 timeout, /* in seconds */
    const c_char *name,
    const u_participantQos qos,
    c_bool enable,
    void *arg)
{
    v_kernel kDomain;
    v_service kService;
    v_participantQos kQos;
    u_domain domain;
    u_service service;
    u_result result;

    if ((result = u_domainOpenForService(&domain, uri, id, timeout)) != U_RESULT_OK) {
        goto err_domainOpen;
    }
    if ((result = u_observableWriteClaim(u_observable(domain),(v_public *)(&kDomain), C_MM_RESERVATION_HIGH)) != U_RESULT_OK) {
        goto err_observableWriteClaim;
    }
    if (v_participantQosCheck((v_participantQos)qos) != V_RESULT_OK) {
        goto err_qosCheck;
    }
    if ((kQos = v_participantQosNew(kDomain, (v_participantQos)qos)) == NULL) {
        goto err_qosCheck;
    }
    if (serviceConfigName) {
        v_configuration config;
        if ((config = v_getConfiguration(kDomain)) == NULL) {
            goto err_qosConfig;
        }
        v_configurationGetSchedulingPolicy(config, serviceConfigName, name, &kQos->watchdogScheduling);
    }
    if ((kService = v_new(kDomain, name, NULL, kQos, enable, arg)) == NULL) {
        OS_REPORT(OS_WARNING, "u_serviceNewSpecialized", U_RESULT_INTERNAL_ERROR,
                  "Failed to create Kernel %s Service", serviceConfigName);
        goto err_qosConfig;
    }

    if ((service = u_objectAlloc(sizeof(*service), U_SERVICE, u__serviceDeinitW, u__serviceFreeW)) == NULL) {
        goto err_objectAlloc;
    } else if ((result = u_serviceInit(service, kService, domain)) != U_RESULT_OK) {
        goto err_serviceInit;
    }

    c_free(kQos);
    c_free(kService);
    u_observableRelease(u_observable(domain), C_MM_RESERVATION_HIGH);
    return service;

err_serviceInit:
    u_objectFree(service);
err_objectAlloc:
    v_serviceFree(kService);
err_qosConfig:
    c_free(kQos);
err_qosCheck:
    u_observableRelease(u_observable(domain), C_MM_RESERVATION_HIGH);
err_observableWriteClaim:
    (void)u_domainClose(domain);
err_domainOpen:
    return NULL;
}

struct wrapperArg {
    v_serviceType serviceType;
};

static void *wrapper(v_kernel kernel, const c_char *name, const c_char *extStateName, v_participantQos qos, c_bool enable, void *varg)
{
    struct wrapperArg *arg = varg;
    return v_serviceNew(kernel, name, extStateName, arg->serviceType, qos, enable);
}

u_service
u_serviceNew (
    const os_char *uri,
    const u_domainId_t id,
    const os_int32 timeout, /* in seconds */
    const os_char *name,
    v_serviceType serviceType,
    const u_participantQos qos,
    c_bool enable)
{
    struct wrapperArg arg;
    arg.serviceType = serviceType;
    return u_serviceNewSpecialized(wrapper, NULL, uri, id, timeout, name, qos, enable, &arg);
}

u_result
u_serviceInit(
    u_service _this,
    const v_service ks,
    const u_domain domain)
{
    os_result osr;
    u_result r;
    const c_char *name;
    c_char *path;

    assert(_this != NULL);
    assert(domain != NULL);

    r = u_participantInit(u_participant(_this), v_participant(ks), domain);
    if (r == U_RESULT_OK) {
        name = v_serviceGetName(ks);
        if (name != NULL) {
            path = (c_char *)os_malloc(strlen(SERVICE_PATH) + strlen(name) + 1);
            /* NULL terminator is covered by '%s' in SERVICE_PATH constant */
            os_sprintf(path, SERVICE_PATH, name);
            if (lockPages(v_objectKernel(ks), path)) {
                osr = os_procMLockAll(OS_MEMLOCK_CURRENT|OS_MEMLOCK_FUTURE);
                OS_REPORT(OS_INFO,"u_serviceInit", osr,
                          "Service '%s': Locking enabled",
                          name);
            } else {
                osr = os_resultSuccess;
            }
            os_free(path);
            if (osr != os_resultSuccess) {
                r = U_RESULT_INTERNAL_ERROR;
            }
        }
        if (r == U_RESULT_OK) {
            _this->serviceManager = u_serviceManagerNew(u_participant(_this));
        }
    } else {
        OS_REPORT(OS_ERROR,"u_serviceInit", r,
                  "Initialization of the Participant failed.");
    }

    return r;
}

u_bool
u_serviceChangeState(
    const u_service _this,
    const v_serviceStateKind newState)
{
    u_result result;
    v_service kService;
    u_bool changed = FALSE;

    assert(_this != NULL);

    result = u_observableReadClaim(u_observable(_this), (v_public *)(&kService), C_MM_RESERVATION_NO_CHECK);
    if (result == U_RESULT_OK) {
        assert(kService);
        changed = v_serviceChangeState(kService, newState);
        u_observableRelease(u_observable(_this), C_MM_RESERVATION_NO_CHECK);
    } else {
        OS_REPORT(OS_WARNING, "u_serviceChangeState", result,
                  "Could not claim service.");
    }

    return changed;
}

os_char *
u_serviceGetName(
    const u_service _this)
{
    c_char *name;
    u_result result;
    v_service s;

    assert(_this != NULL);

    name = NULL;

    result = u_observableReadClaim(u_observable(_this), (v_public *)(&s),C_MM_RESERVATION_ZERO);
    if (result == U_RESULT_OK) {
        assert(s);
        name = os_strdup(v_serviceGetName(s));
        u_observableRelease(u_observable(_this), C_MM_RESERVATION_ZERO);
    } else {
        OS_REPORT(OS_WARNING, "u_serviceGetName", result,
                  "Could not claim service.");
    }

    return name;
}

u_result
u_serviceWatchSpliceDaemon(
    const u_service _this,
    const u_serviceSplicedaemonListener listener,
    const void *usrData)
{
    u_result r = U_RESULT_OK;

    assert(_this != NULL);

    if (listener == NULL) {
        r = u_observableRemoveListener(u_observable(_this),u_serviceSpliceListener);
    } else {
        _this->callback = listener;
        _this->usrData = (void *)usrData;
        r = u_observableAddListener(u_observable(_this),
                                    V_EVENT_SERVICESTATE_CHANGED,
                                    u_serviceSpliceListener,
                                    _this);
    }
    return r;
}

u_result
u_serviceRenewLease(
    const u_service _this,
    os_duration leasePeriod)
{
    u_result r;
    v_service kernelService;

    assert(_this != NULL);

    r = u_observableReadClaim(u_observable(_this), (v_public *)(&kernelService), C_MM_RESERVATION_ZERO);
    if(r == U_RESULT_OK)
    {
        assert(kernelService);
        v_serviceRenewLease(kernelService, leasePeriod);
        u_observableRelease(u_observable(_this), C_MM_RESERVATION_ZERO);
    } else {
        OS_REPORT(OS_WARNING,
                  "u_serviceRenewLease", r,
                  "Failed to claim service.");
    }

    return r;
}


u_result
u_serviceAtExit(
    u_service _this,
    u_serviceAtExitAction action,
    void *privateData)
{
    u_result r;
    v_service kernelService;

    assert(_this != NULL);
    assert(action != NULL);

    /* See the comment at the start of u_service.h for more
     * information about serviceAtExit functionality.
     */

    r = u_observableWriteClaim(u_observable(_this), (v_public *)(&kernelService), C_MM_RESERVATION_ZERO);
    if(r == U_RESULT_OK)
    {
        assert(kernelService);
        r = serviceAtExitInfoAdd(_this, action, privateData);
        u_observableRelease(u_observable(_this), C_MM_RESERVATION_ZERO);
    } else {
        OS_REPORT(OS_WARNING,
                  "u_serviceAtExit", r,
                  "Failed to claim service.");
    }

    return r;
}

void u_serviceCheckEnvURI(struct u_service_cmdopts *cmdopts)
{
   if ( !cmdopts->uri )
   {
      char *envuri=os_getenv("OSPL_URI");
      if ( envuri && envuri[0] != '\0')
      {
         cmdopts->uri=os_strdup(envuri);
      }
   }
}

u_domainId_t
u_serviceThreadGetDomainId(void)
{
    return v_kernelThreadInfoGetDomainId();
}

void
u_serviceThreadSetDomainId(
    u_domainId_t domainId)
{
    u_domain domain = NULL;

    if (domainId >= 0) {
        domain = u_userLookupDomain(domainId);
        if (domain) {
            u_domainIdSetThreadSpecific(domain);
            (void)u_domainClose(domain);
        }
    }
}

C_CLASS(u_serviceThreadArguments);
C_STRUCT(u_serviceThreadArguments) {
    u_domainId_t domainId;
    os_threadRoutine start_routine;
    void *arguments;
};


static u_serviceThreadArguments
u_serviceThreadArumentsNew(
    _In_ os_threadRoutine start_routine,
    _In_ void *arguments) __nonnull((1));

static u_serviceThreadArguments
u_serviceThreadArumentsNew(
    os_threadRoutine start_routine,
    void *arguments)
{
    u_serviceThreadArguments args = os_malloc(C_SIZEOF(u_serviceThreadArguments));

    args->domainId = v_kernelThreadInfoGetDomainId();
    args->start_routine = start_routine;
    args->arguments = arguments;

    return args;
}

static void *
u_serviceThreadWrapper(
    _In_ void *args);

static void *
u_serviceThreadWrapper(
    void *args)
{
    u_serviceThreadArguments tinfo = args;
    u_domain domain = NULL;
    os_threadRoutine start_routine;
    void *arguments;

    if (tinfo->domainId >= 0) {
        domain = u_userLookupDomain(tinfo->domainId);
        if (domain) {
            u_domainIdSetThreadSpecific(domain);
            (void)u_domainClose(domain);
        }
    }

    start_routine = tinfo->start_routine;
    arguments = tinfo->arguments;

    os_free(tinfo);

    return start_routine(arguments);
}

os_result
u_serviceThreadCreate(
    _Out_ os_threadId *threadId,
    _In_ const char *name,
    _In_ const os_threadAttr *threadAttr,
    _In_ os_threadRoutine start_routine,
    _In_ void *arg)
{
    u_serviceThreadArguments tinfo;
    os_result r;

    tinfo = u_serviceThreadArumentsNew(start_routine, arg);

    r = os_threadCreate(threadId, name, threadAttr, u_serviceThreadWrapper, tinfo);
    if (r != os_resultSuccess) {
        os_free(tinfo);
    }

    return r;
}

u_result
u_serviceFillNewGroups(
    const u_service _this)
{
    u_result r;
    v_service kservice;

    assert(_this != NULL);

    r = u_observableReadClaim(u_observable(_this), (v_public *)(&kservice), C_MM_RESERVATION_ZERO);
    if(r == U_RESULT_OK) {
        assert(kservice);
        v_serviceFillNewGroups(kservice);
        u_observableRelease(u_observable(_this), C_MM_RESERVATION_ZERO);
    } else {
        OS_REPORT(OS_WARNING, "u_serviceFillNewGroups", r, "Failed to claim service.");
    }
    return r;
}

