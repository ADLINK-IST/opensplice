/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2009 PrismTech
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
#include "u__kernel.h"
#include "u__participant.h"
#include "u__serviceManager.h"
#include "u__cfValue.h"

#include "u_dispatcher.h"
#include "u_service.h"

#include "v_service.h"
#include "v_networking.h"
#include "v_durability.h"
#include "v_cmsoap.h"
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

static u_result
u_serviceClaim(
    u_service _this,
    v_service *service)
{
    u_result result = U_RESULT_OK;

    if ((_this != NULL) && (service != NULL)) {
        *service = v_service(u_entityClaim(u_entity(_this)));
        if (*service == NULL) {
            OS_REPORT_2(OS_WARNING, "u_serviceClaim", 0,
                        "Claim Service failed. "
                        "<_this = 0x%x, service = 0x%x>.",
                         _this, service);
            result = U_RESULT_INTERNAL_ERROR;
        }
    } else {
        OS_REPORT_2(OS_ERROR,"u_serviceClaim",0,
                    "Illegal parameter. "
                    "<_this = 0x%x, service = 0x%x>.",
                    _this, service);
        result = U_RESULT_ILL_PARAM;
    }
    return result;
}

static u_result
u_serviceRelease(
    u_service _this)
{
    u_result result = U_RESULT_OK;

    if (_this != NULL) {
        result = u_entityRelease(u_entity(_this));
    } else {
        OS_REPORT_1(OS_ERROR,"u_serviceRelease",0,
                    "Illegal parameter. <_this = 0x%x>.", _this);
        result = U_RESULT_ILL_PARAM;
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
            sprintf(path, SERVICE_PATH, name);
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
                } else {
                    OS_REPORT_1(OS_INFO,"lockPages", 0,
                        "service '%s': Locking disabled", name);
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
    u_kernel k;
    v_kernel kk;
    v_service ks;
    v_serviceManager sm;
    u_service s;
    u_result r;
    os_result osr;

    ks = NULL;
    k = u_userKernelOpen(uri, timeout);
    if (k == NULL) {
        OS_REPORT(OS_ERROR,"u_serviceNew",0,
                  "Failure to open the kernel");
        return NULL;
    }

	
    s = NULL;
    if (k != NULL) {
        r = u_kernelClaim(k,&kk);
        if ((r == U_RESULT_OK) && (kk != NULL)) {
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
                    case U_SERVICE_NETWORKING:
                        ks = v_service(v_networkingNew(sm, name,
                                                       extendedStateName,
                                                       (v_participantQos)qos));
                        os_procSetTerminationHandler(serviceTermHandler);
                    break;
                    case U_SERVICE_DURABILITY:
                        ks = v_service(v_durabilityNew(sm, name,
                                                       extendedStateName,
                                                       (v_participantQos)qos));
                        os_procSetTerminationHandler(serviceTermHandler);
                    break;
                    case U_SERVICE_CMSOAP:
                        ks = v_service(v_cmsoapNew(sm, name,
                                                   extendedStateName,
                                                   (v_participantQos)qos));
                        os_procSetTerminationHandler(serviceTermHandler);
                    break;
                    case U_SERVICE_INCOGNITO:
                        ks = v_serviceNew(sm, name,
                                          extendedStateName,
                                          (v_participantQos)qos, NULL);
                        os_procSetTerminationHandler(serviceTermHandler);
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
#ifndef INTEGRITY

            } else {
                OS_REPORT(OS_ERROR,"u_serviceNew",0,
                          "Failed to retrieve the Service Manager");
            }
#endif
            if (ks != NULL) {
                s = u_entityAlloc(NULL,u_service,ks,TRUE);
                r = u_serviceInit(s, kind, k);
                if (r != U_RESULT_OK) {
                    OS_REPORT_1(OS_ERROR,"u_serviceNew",0,
                                "Failed to initialize service: %s", name);
                    u_serviceFree(s);
                    s = NULL;
                }
            } else {
                OS_REPORT(OS_WARNING,"u_serviceNew",0,
                          "Failed to retrieve the Service Manager");
            }
            r = u_kernelRelease(k);
        }
    }
    return s;
}

u_result
u_serviceFree(
    u_service service)
{
    u_result r;
    u_kernel kernel;

    if (service != NULL) {

        if (u_entity(service)->flags & U_ECREATE_INITIALISED) {
            kernel = u_participant(service)->kernel;
            r = u_serviceDeinit(service);
            os_free(service);
            u_userKernelClose(kernel);
        } else {
            r = u_entityFree(u_entity(service));
		}
    } else {
        OS_REPORT(OS_WARNING,"u_serviceFree",0,
                  "The specified Service = NIL>");
        r = U_RESULT_OK;
    }
    return r;
}

u_result
u_serviceInit(
    u_service service,
    u_serviceKind kind,
    u_kernel kernel)
{
    u_result r;
    watchSplicedAdmin admin;

    if ((service != NULL) && (kernel != NULL)) {
        admin = watchSplicedAdmin(os_malloc((os_uint32)C_SIZEOF(watchSplicedAdmin)));
        if (admin != NULL) {
            service->serviceKind = kind;
            r = u_participantInit(u_participant(service), kernel);
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
        admin->callback = NULL;
        admin->usrData = NULL;
        if (admin->serviceManager != NULL) {
            u_serviceManagerFree(admin->serviceManager);
        }
        os_free(admin);
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
        r = u_serviceClaim(service, &s);
        if ((r== U_RESULT_OK) && (s != NULL)) {
            result = v_serviceChangeState(s, newState);
            r = u_serviceRelease(service);
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
        result = u_serviceClaim(service, &s);
        if ((result == U_RESULT_OK) && (s != NULL)) {
            name = os_strdup(v_serviceGetName(s));
            u_serviceRelease(service);
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
            u_dispatcherInsertListener(u_dispatcher(service),
                u_serviceSpliceListener, admin);
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
        result = u_serviceClaim(service, &s);
        if ((result == U_RESULT_OK) && (s != NULL)) {
            v_enableStatistics(v_objectKernel(s), categoryName);
            result = u_serviceRelease(service);
        } else {
            OS_REPORT(OS_WARNING, "u_serviceEnableStatistics", 0,
                      "Could not claim service.");
        }
    }
    return result;
}
