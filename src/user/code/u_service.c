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
#include "os_report.h"

#include "u_user.h"
#include "u__user.h"
#include "u__types.h"
#include "u__entity.h"
#include "u__domain.h"
#include "u__participant.h"
#include "u__serviceManager.h"
#include "u__cfValue.h"

#include "u_dispatcher.h"
#include "u_service.h"


#include "v_service.h"
#include "v_networking.h"
#include "v_durability.h"
#include "v_cmsoap.h"
#include "v_rnr.h"
#include "v_configuration.h"
#include "v_entity.h"
#include "v_event.h"

#define watchSplicedAdmin(o) ((watchSplicedAdmin)(o))
C_CLASS(watchSplicedAdmin);
C_STRUCT(watchSplicedAdmin) {
    u_serviceManager              serviceManager;
    u_serviceSplicedaemonListener callback;
    c_voidp                       usrData;
};

static u_service callbackService;

/**************************************************************
 * private functions
 **************************************************************/
static os_int32
serviceTermHandler(os_terminationType reason)
{
    os_int32 result = 0;

    if (reason == OS_TERMINATION_NORMAL){
        OS_REPORT_1(OS_WARNING, "serviceTermHandler", 0,
                    "Caught termination request: %d. Please use 'ospl stop' for termination.",
                    reason);
        result = 0;  /* stop termination process */
    } else {   /* OS_TERMINATION_ERROR */
        result = 1;  /* continue termination process */
    }
    return result;
}

static c_ulong
u_serviceSpliceListener(
    u_dispatcher o,
    c_ulong event,
    c_voidp usrData)
{
    v_serviceStateKind kind;
    watchSplicedAdmin admin = watchSplicedAdmin(usrData);

    kind = u_serviceManagerGetServiceStateKind(admin->serviceManager,
                V_SPLICED_NAME);
    if ((kind != STATE_INITIALISING) && (kind != STATE_OPERATIONAL)) {
        admin->callback(kind, admin->usrData);
    }
    return V_EVENT_SERVICESTATE_CHANGED;
}

#ifndef INTEGRITY
#define SERVICE_PATH "Domain/Service[@name='%s']"
static int
lockPages(
    v_kernel k,
    const c_char *name)
{
    v_configuration cfg;
    v_cfElement root;
    v_cfElement service;
    v_cfData data;
    int lock;
    c_char *path;
    c_iter iter;
    int iterLength;
    c_value value;

    assert(k);

    lock = 0;
    cfg = v_getConfiguration(k);
    if (cfg != NULL) {
        root = v_configurationGetRoot(cfg);
        if (root != NULL) {
            path = (c_char *)os_malloc(strlen(SERVICE_PATH) + strlen(name));
            /* NULL terminator is covered by '%s' in SERVICE_PATH constant */
            os_sprintf(path, SERVICE_PATH, name);
            iter = v_cfElementXPath(root, path);
            iterLength = c_iterLength(iter);
            os_free(path);
            if (iterLength == 1) {
                service = v_cfElement(c_iterTakeFirst(iter));
                c_iterFree(iter);
                iter = v_cfElementXPath(service, "Locking/#text");
                iterLength = c_iterLength(iter);
                if (iterLength == 1) {
                    data = v_cfData(c_iterTakeFirst(iter));
                    if (u_cfValueScan(v_cfDataValue(data), V_BOOLEAN, &value)) {
                        if (value.is.Boolean) {
                            lock = 1;
                            OS_REPORT_1(OS_INFO,"lockPages", 0,
                                "service '%s': Locking enabled", name);
                        }
                    } else {
                        OS_REPORT_1(OS_WARNING,"lockPages", 0,
                            "Failed to retrieve Locking for service '%s': Locking disabled", name);
                    }
                }
            } else if (iterLength > 1) {
                OS_REPORT_2(OS_WARNING,"lockPages", 0,
                    "Multiple configuration found for service '%s' (too many: %d): Locking disabled",
                    name, iterLength);
            } else {
                OS_REPORT_1(OS_WARNING,"lockPages", 0,
                    "Could not get configuration for service '%s' (non-existent): Locking disabled",
                    name);
            }
            c_iterFree(iter);
            c_free(root);
        }
    }

    return lock;
}
#undef SERVICE_PATH
#endif

static void
freeKernelServiceObject (
    v_entity e,
    c_voidp argument)
{
    v_serviceFree(v_service(e));
}

static os_result
u__serviceExceptionCallbackWrapper(void)
{
    os_result result = os_resultSuccess;

    /* do not detach when using single process as this does not function correctly for single process*/
    if(!os_serviceGetSingleProcess()) {
        OS_REPORT(OS_ERROR, "u__serviceExceptionCallbackWrapper", 0,
                "Exception occurred, will detach service from domain.");
        /* calling the kernel service free directly because only the kernel administration needs to be removed */
        if (u_entityAction(u_entity(callbackService),freeKernelServiceObject,NULL) != U_RESULT_OK)
            result = os_resultFail;
    }
    return result;
}

/**************************************************************
 * constructor/destructor
 **************************************************************/
u_service
u_serviceNew(
    const c_char *uri,
    c_long timeout,
    const c_char *name,
    const c_char *extendedStateName,
    u_serviceKind kind,
    v_qos qos)
{
    u_domain domain;
    v_kernel kk;
    v_service ks;
    v_serviceManager sm;
    u_service s;
    u_result r;
    os_result osr;
    c_bool serviceTermHandlerRequired = FALSE;

    ks = NULL;
    r = u_domainOpen(&domain, uri, timeout);
    if (r != U_RESULT_OK) {
        OS_REPORT_1(OS_ERROR,"u_serviceNew",0,
                  "Failure to open the kernel - return code %d", r);
        return NULL;
    }

    s = NULL;
    if (domain != NULL) {
        r = u_entityWriteClaim(u_entity(domain),(v_entity*)(&kk));
        if (r == U_RESULT_OK) {
            assert(kk);
            sm = v_getServiceManager(kk);
            if (sm != NULL) {
#ifndef INTEGRITY
                if (lockPages(kk, name)) {
                    osr = os_procMLockAll(OS_MEMLOCK_CURRENT|OS_MEMLOCK_FUTURE);
                } else {
                    osr = os_resultSuccess;
                }
                if (osr == os_resultSuccess) {
#endif
                    switch(kind){
                    case U_SERVICE_DDSI:
                    case U_SERVICE_DDSIE:
                    case U_SERVICE_NETWORKING:
                        ks = v_service(v_networkingNew(sm, name,
                                                       extendedStateName,
                                                       (v_participantQos)qos));
                        serviceTermHandlerRequired = TRUE;
                    break;
                    case U_SERVICE_DURABILITY:
                        ks = v_service(v_durabilityNew(sm, name,
                                                       extendedStateName,
                                                       (v_participantQos)qos));
                        serviceTermHandlerRequired = TRUE;
                    break;
                    case U_SERVICE_CMSOAP:
                        ks = v_service(v_cmsoapNew(sm, name,
                                                   extendedStateName,
                                                   (v_participantQos)qos));
                        serviceTermHandlerRequired = TRUE;
                    break;
                    case U_SERVICE_RNR:
                        ks = v_service(v_rnrNew(sm, name,
                                                extendedStateName,
                                                (v_participantQos)qos));
                        serviceTermHandlerRequired = TRUE;
                    break;
                    case U_SERVICE_DBMSCONNECT:
                    case U_SERVICE_INCOGNITO:
                        ks = v_serviceNew(sm, name,
                                          extendedStateName,
                                          (v_participantQos)qos, NULL);
                        serviceTermHandlerRequired = TRUE;
                    break;
                    case U_SERVICE_SPLICED:
                    break;
                    default:
                        OS_REPORT(OS_WARNING,"u_serviceNew",0,
                                  "Failed to start an unknown service kind");
                    break;
                    }
                } else {
                    OS_REPORT(OS_ERROR,"u_serviceNew",0,
                              "Failed to lock memory pages for current process");
                }

                /* Install the service signal handlers if spliced is not within this
                 * same process.  i.e. only do this if each service is in its own
                 * process so signal handlers won't interfere */
                if (serviceTermHandlerRequired && !u_splicedInProcess()) {
                    os_procSetTerminationHandler(serviceTermHandler);
                }

#ifndef INTEGRITY

            } else {
                OS_REPORT(OS_ERROR,"u_serviceNew",0,
                          "Failed to retrieve the Service Manager");
            }
#endif
            if (ks != NULL) {
                s = u_entityAlloc(NULL,u_service,ks,TRUE);
                r = u_serviceInit(s, kind, domain);
                if (r != U_RESULT_OK) {
                    OS_REPORT_1(OS_ERROR,"u_serviceNew",0,
                                "Failed to initialize service: %s", name);
                    u_serviceFree(s);
                    s = NULL;
                }
                callbackService = s;
                (void) os_signalHandlerSetExceptionCallback(u__serviceExceptionCallbackWrapper);
            } else {
                OS_REPORT(OS_WARNING,"u_serviceNew",0,
                          "Failed to retrieve the Service Manager");
            }
            r = u_entityRelease(u_entity(domain));
        }
    }
    return s;
}

u_result
u_serviceFree(
    u_service _this)
{
    u_result result;
    c_bool destroy;

    result = u_entityLock(u_entity(_this));
    if (result == U_RESULT_OK) {
        destroy = u_entityDereference(u_entity(_this));
        /* if refCount becomes zero then this call
         * returns true and destruction can take place
         */
        if (destroy) {
            if (u_entityOwner(u_entity(_this))) {
                result = u_serviceDeinit(_this);
            } else {
                /* This user entity is a proxy, meaning that it is not fully
                 * initialized, therefore only the entity part of the object
                 * can be deinitialized.
                 * It would be better to either introduce a separate proxy
                 * entity for clarity or fully initialize entities and make
                 * them robust against missing information.
                 */
                result = u_entityDeinit(u_entity(_this));
            }
            if (result == U_RESULT_OK) {
                u_entityDealloc(u_entity(_this));
            } else {
                OS_REPORT_2(OS_WARNING,
                            "u_serviceFree",0,
                            "Operation u_serviceDeinit failed: "
                            "Service = 0x%x, result = %s.",
                            _this, u_resultImage(result));
                u_entityUnlock(u_entity(_this));
            }
        } else {
            u_entityUnlock(u_entity(_this));
        }
    } else {
        OS_REPORT_2(OS_WARNING,
                    "u_serviceFree",0,
                    "Operation u_entityLock failed: "
                    "Service = 0x%x, result = %s.",
                    _this, u_resultImage(result));
    }
    return result;
}

u_result
u_serviceInit(
    u_service service,
    u_serviceKind kind,
    u_domain domain)
{
    u_result r;
    watchSplicedAdmin admin;

    if ((service != NULL) && (domain != NULL)) {
        admin = watchSplicedAdmin(os_malloc((os_uint32)C_SIZEOF(watchSplicedAdmin)));
        service->stt = NULL;
        if (admin != NULL) {
            service->serviceKind = kind;
            r = u_participantInit(u_participant(service), domain);
            if (r == U_RESULT_OK) {
                admin->serviceManager = u_serviceManagerNew(u_participant(service));
                admin->callback = NULL;
                admin->usrData = NULL;
                service->privateData = (c_voidp)admin;
                u_entity(service)->flags |= U_ECREATE_INITIALISED;
            } else {
                OS_REPORT(OS_ERROR,"u_serviceInit",0,
                          "Initialization of the Participant failed.");
            }
        } else {
            service->privateData = NULL;
            OS_REPORT(OS_ERROR,"u_serviceInit",0,
                      "Failed to allocate resources.");
            r = U_RESULT_OUT_OF_MEMORY;
        }
    } else {
        OS_REPORT(OS_ERROR,"u_serviceInit",0,
                  "Illegal parameter.");
        r = U_RESULT_ILL_PARAM;
    }
    return r;
}

u_result
u_serviceDeinit(
    u_service service)
{
    u_result r;
    watchSplicedAdmin admin;
    if (service != NULL) {

        u_dispatcherRemoveListener(u_dispatcher(service),
                                   u_serviceSpliceListener);
        admin = watchSplicedAdmin(service->privateData);
        if (admin) {
            admin->callback = NULL;
            admin->usrData = NULL;
            if (admin->serviceManager != NULL) {
                u_serviceManagerFree(admin->serviceManager);
            }
            os_free(admin);
        }
        service->privateData = NULL;
        r = u_participantDeinit(u_participant(service));
    } else {
        OS_REPORT(OS_ERROR,"u_serviceDeinit",0,
                  "Illegal parameter.");
        r = U_RESULT_ILL_PARAM;
    }
    return r;
}

/**************************************************************
 * Public functions
 **************************************************************/
c_bool
u_serviceChangeState(
    u_service service,
    v_serviceStateKind newState)
{
    u_result r;
    v_service s;
    c_bool result = FALSE;

    if (service != NULL) {
        result = u_entityReadClaim(u_entity(service), (v_entity*)(&s));
        if (result == U_RESULT_OK) {
            assert(s);

            /* start or stop the Termination Monitor Thread */
            if (newState == STATE_TERMINATING) {
                if (service->stt == NULL) {
                    service->stt = u_serviceTerminationThreadNew();
                }
            }
            if (newState == STATE_TERMINATED) {
                if (service->stt != NULL) {
                    r = u_serviceTerminationThreadFree(service->stt);
                    if (r != U_RESULT_OK) {
                        OS_REPORT_1(OS_ERROR, "u_serviceChangeState", 0,
                                   "Failed to clean up the Service Termination Thread for process %d",os_procIdSelf());
                    }
                    service->stt = NULL;
                }
            }
            result = v_serviceChangeState(s, newState);
            r = u_entityRelease(u_entity(service));
        } else {
            OS_REPORT(OS_WARNING, "u_serviceChangeState", 0,
                      "Could not claim service.");
        }
    }

    return result;
}

c_char *
u_serviceGetName(
    u_service service)
{
    c_char *name;
    u_result result;
    v_service s;

    name = NULL;
    if (service != NULL) {
        result = u_entityReadClaim(u_entity(service), (v_entity*)(&s));
        if (result == U_RESULT_OK) {
            assert(s);
            name = os_strdup(v_serviceGetName(s));
            u_entityRelease(u_entity(service));
        } else {
            OS_REPORT(OS_WARNING, "u_serviceGetName", 0,
                      "Could not claim service.");
        }
    }
    return name;
}

u_result
u_serviceWatchSpliceDaemon(
    u_service service,
    u_serviceSplicedaemonListener listener,
    c_voidp usrData)
{
    u_result r;
    watchSplicedAdmin admin;
    c_ulong mask;

    r = U_RESULT_OK;
    if (service == NULL) {
        r = U_RESULT_ILL_PARAM;
    } else {
        admin = watchSplicedAdmin(service->privateData);
        u_dispatcherGetEventMask(u_dispatcher(service), &mask);
        if (listener == NULL) {
            mask &= ~V_EVENT_SERVICESTATE_CHANGED;
            u_dispatcherRemoveListener(u_dispatcher(service),u_serviceSpliceListener);
            admin->callback = NULL;
            admin->usrData = NULL;
        } else {
            admin->callback = listener;
            admin->usrData = usrData;
            mask |= V_EVENT_SERVICESTATE_CHANGED;
            r = u_dispatcherInsertListener(u_dispatcher(service), u_serviceSpliceListener, admin);
        }
        u_dispatcherSetEventMask(u_dispatcher(service), mask);
    }

    return r;
}

u_result
u_serviceEnableStatistics(
    u_service service,
    const char *categoryName)
{
    v_service s;
    u_result result = U_RESULT_UNDEFINED;

    if (service != NULL) {
        result = u_entityReadClaim(u_entity(service), (v_entity*)(&s));
        if (result == U_RESULT_OK) {
            assert(s);
            v_enableStatistics(v_objectKernel(s), categoryName);
            result = u_entityRelease(u_entity(service));
        } else {
            OS_REPORT(OS_WARNING, "u_serviceEnableStatistics", 0,
                      "Could not claim service.");
        }
    }
    return result;
}

u_result
u_serviceRenewLease(
    u_service _this,
    v_duration leasePeriod)
{
    u_result r;
    v_service kernelService;

    if (_this == NULL) {
        r = U_RESULT_ILL_PARAM;
    } else {
        r = u_entityReadClaim(u_entity(_this), (v_entity*)(&kernelService));
        if(r == U_RESULT_OK)
        {
            assert(kernelService);
            v_serviceRenewLease(kernelService, leasePeriod);
            r = u_entityRelease(u_entity(_this));
        } else {
            OS_REPORT(OS_WARNING,
                      "u_serviceRenewLease", 0,
                      "Failed to claim service.");
        }
    }
    return r;
}
