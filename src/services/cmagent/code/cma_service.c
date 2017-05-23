/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR PrismTech
 *   Limited, its affiliated companies and licensors. All rights reserved.
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
#include "os_version.h"
#include "os_gitrev.h"

#include "cma_service.h"

#include "cma__service.h"
#include "cma__thread.h"
#include "cma__configuration.h"
#include "cma__log.h"

#include "u_user.h"
#include "u_domain.h"
#include "u_service.h"

static int cma_proc_must_exit;

C_STRUCT(cma_service)
{
    C_EXTENDS(cma_object);
    os_char *name;
    u_service service;
    u_waitset ws;
    cma_configuration config;
    os_mutex configLock;
    cma_thread leaseRenewThr;
    os_signalHandlerExitRequestHandle erh;
    os_signalHandlerExceptionHandle eh;
    volatile int terminate;
    os_cond terminateCond;
    os_mutex terminateMtx;
};

static cma_service
cma__serviceNew(
    const os_char *uri,
    const os_char *serviceName) __nonnull((2)) __attribute_malloc__;

static void
cma__serviceDeinit(
    cma_service _this) __nonnull_all__;

static void
cma__serviceMain(
    cma_service _this) __nonnull_all__;

static os_result
cma__exceptionHandler(
    os_callbackArg cbArg,
    void *arg) __nonnull_all__;

static os_result
cma__exitRequestHandler(
    os_callbackArg cbArg,
    void *arg) __nonnull_all__;

static c_bool
cma__serviceChangeState(
    cma_service _this,
    v_serviceStateKind state) __nonnull_all__;

static void
cma__serviceWatchSpliced(
    v_serviceStateKind state,
    c_voidp arg /* cma_service */) __nonnull_all__;

static void*
cma__serviceUpdateLease(
    void *arg) __nonnull_all__;

static os_result
cma__serviceOpenTracingFile(
    cma_service _this) __nonnull_all__;

/***** end static func declarations *****/

OPENSPLICE_SERVICE_ENTRYPOINT(ospl_cmagent, cmagent)
{
    C_STRUCT(cma_logConfig) gc;
    cma_threadStates ts;
    cma_service service;

    os_int32 exitStatus;
    os_char *serviceName;
    os_char *uri;

    u_result uresult;

    exitStatus = EXIT_FAILURE;

    if (argc <= 1) {
        OS_REPORT(OS_WARNING, "Control & Monitoring Agent Service", 0,
            "No service-name supplied, defaulting to '%s'. Usage: '%s <service-name> (<URI/domain-name>)'",
            argv[0], argv[0]);
        serviceName = argv[0];
    }
    if (argc > 1) {
        serviceName = argv[1];
    }
    if (argc > 2) {
        uri = argv[2];
    } else {
        uri = NULL;
    }

    ts = cma_threadStatesAlloc();
    cma_logConfigInit(&gc);

    /* Create threads administration and upgrade the main thread */
    if (cma_threadStatesInit(ts, CMA_MAXTHREADS, &gc) != os_resultSuccess) {
        CMA_FATAL("main", "Failed to initialize threads administration\n");
        goto err_tsinit;
    }
    cma_threadUpgrade(ts);

    service = cma__serviceNew(uri, serviceName);
    if (!service) {
        fprintf(stderr, "Failed to create the service. Is the domain running?\n");
        /* error reported by cma_serviceNew */
        goto err_servicenew;
    }

    cma__serviceChangeState(service, STATE_INITIALISING);

    /* Monitor spliced liveliness */
    uresult = u_serviceWatchSpliceDaemon(service->service, cma__serviceWatchSpliced, (void*)service);
    if (uresult != U_RESULT_OK) {
        /* error reported by u_serviceWatchSpliceDaemon */
        cma__serviceChangeState(service, STATE_TERMINATING);
        goto err_servicewatchspliced;
    }

    /* Run main loop */
    cma__serviceMain(service);

    cma__serviceChangeState(service, STATE_TERMINATING);

    uresult = u_serviceWatchSpliceDaemon(service->service, NULL, (void*)service);
    if (uresult != U_RESULT_OK) {
        goto err_servicewatchspliced;
    }

    exitStatus = EXIT_SUCCESS;

err_servicewatchspliced:
    cma_serviceFree(service);
    CMA_TRACE(("Service terminated\n"));
err_servicenew:
    cma_threadDowngrade();
    cma_threadStatesDeinit(ts);
err_tsinit:
    cma_logConfigDeinit(&gc);
    cma_threadStatesDealloc(ts);

    return exitStatus;
}

cma_service
cma__serviceNew(
    const os_char *uri,
    const os_char *serviceName)
{
    cma_service _this;

    u_result uresult;
    os_result osresult;

    assert(serviceName);

    _this = os_malloc(sizeof(*_this));
    cma__objectInit(cma_object(_this), CMA_OBJECT_SERVICE, (cma_objectDeinitFunc)cma__serviceDeinit);

    uresult = u_userInitialise();
    if (uresult != U_RESULT_OK) {
        CMA_FATAL("cma__serviceNew", "Failed to initialize user-layer (%s)\n",
            u_resultImage(uresult));
        goto err_userinit;
    }

    _this->service = u_serviceNew(uri, U_DOMAIN_ID_ANY, 5, serviceName, V_SERVICETYPE_CMAGENT, NULL, TRUE);
    if (!_this->service) {
        /* error reported by u_serviceNew */
        goto err_ucmagentnew;
    }

    _this->name = os_strdup(serviceName);
    _this->terminate = 0;

    osresult = os_mutexInit(&_this->terminateMtx, NULL);
    if (osresult != os_resultSuccess) {
        CMA_FATAL("cma__serviceNew", "Could not create termination mutex (%s)\n",
            os_resultImage(osresult));
        goto err_mutexinit;
    }

    osresult = os_condInit(&_this->terminateCond, &_this->terminateMtx, NULL);
    if (osresult != os_resultSuccess) {
        CMA_FATAL("cma__serviceNew", "Could not create termination cond (%s)\n",
            os_resultImage(osresult));
        goto err_condinit;
    }

    _this->ws = u_waitsetNew();
    if (!_this->ws) {
        CMA_FATAL("cma__serviceNew", "Could not create waitset.\n");
        goto err_waitsetnew;
    }

    uresult = u_waitsetSetEventMask(_this->ws, V_EVENT_TRIGGER);
    if (uresult != U_RESULT_OK) {
        CMA_FATAL("cma_serviceNew", "Could not set event mask for waitset (%s)\n",
            u_resultImage(uresult));
        goto err_waitsetmask;
    }

    uresult = u_waitsetAttach(_this->ws, u_observable(_this->service), _this->service);
    if (uresult != U_RESULT_OK) {
        CMA_FATAL("cma_serviceNew", "Could not attach service to waitset (%s)\n",
            u_resultImage(uresult));
        goto err_waitsetattach;
    }

    if (!os_serviceGetSingleProcess()) {
        _this->erh = os_signalHandlerRegisterExitRequestCallback(cma__exitRequestHandler, _this);
        _this->eh = os_signalHandlerRegisterExceptionCallback(cma__exceptionHandler, _this);
    } else {
        _this->erh = os_signalHandlerExitRequestHandleNil;
        _this->eh = os_signalHandlerExceptionHandleNil;
    }

    _this->config = cma_configurationNew(_this);
    if (!_this->config) {
        CMA_FATAL("cma_serviceNew",
            "Failed to initialize configuration of service '%s'\n",
            serviceName);
        goto err_confignew;
    }

    if (os_mutexInit(&(_this->configLock), NULL) != os_resultSuccess) {
        CMA_FATAL("cma_serviceNew",
            "Failed to initialize mutex for configuration of service '%s'\n",
            serviceName);
        goto err_configlockinit;
    }

    if (cma__serviceOpenTracingFile(_this) != os_resultSuccess) {
        /* error reported in cma__serivceOpenTracingFile */
        goto err_opentracing;
    }
#if defined(OSPL_INNER_REV) && defined(OSPL_OUTER_REV)
    cma_log(LOG_INFO, "Started Control and Monitoring Agent service"
        " (OpenSplice " OSPL_VERSION_STR ", build " OSPL_INNER_REV_STR "/" OSPL_OUTER_REV_STR ")\n");
#else
    cma_log(LOG_INFO, "Started Control and Monitoring Agent service"
        " (OpenSplice " OSPL_VERSION_STR ", non-PrismTech build)\n");
#endif /* defined(OSPL_INNER_REV) && defined(OSPL_OUTER_REV) */
    cma_configurationPrint(_this->config);

    osresult = cma_threadCreate("LeaseRenew", &(_this->leaseRenewThr), cma__serviceUpdateLease, _this);
    if (osresult != os_resultSuccess) {
        CMA_ERROR("cma_serviceNew", "Failed to create lease renewal thread of service '%s'\n",
            serviceName);
        goto err_leasethr;
    }

    return _this;

err_leasethr:
err_opentracing:
    os_mutexDestroy(&_this->configLock);
err_configlockinit:
    cma_configurationFree(_this->config);
err_confignew:
err_waitsetattach:
err_waitsetmask:
    u_objectFree(u_object(_this->ws));
err_waitsetnew:
    os_condDestroy(&_this->terminateCond);
err_condinit:
    os_mutexDestroy(&_this->terminateMtx);
err_mutexinit:
    os_free(_this->name);
    u_objectFree(u_object(_this->service));
err_ucmagentnew:
err_userinit:
    cma__objectDeinit(cma_object(_this));
    os_free(_this);
    return NULL;
}

static void
cma__serviceDeinit(
    cma_service _this)
{
    cma_objectIsValidKind(_this, CMA_OBJECT_SERVICE);
    assert(_this->terminate);

    /* notify lease-renew-thread of termination */
    os_mutexLock(&_this->terminateMtx);
    os_condBroadcast(&_this->terminateCond);
    os_mutexUnlock(&_this->terminateMtx);

    if (cma_threadJoin(_this->leaseRenewThr, NULL) != os_resultSuccess) {
        CMA_WARNING("cma__serviceDeinit",
            "Failed to join lease-renewal thread\n");
    }

    cma__serviceChangeState(_this, STATE_TERMINATED);

    cma_configurationFree(_this->config);

    os_signalHandlerUnregisterExceptionCallback(_this->eh);
    os_signalHandlerUnregisterExitRequestCallback(_this->erh);

    u_waitsetDetach(_this->ws, u_observable(_this->service));

    u_objectFree(u_object(_this->ws));
    u_objectFree(u_object(_this->service));
    os_free(_this->name);

    os_mutexDestroy(&_this->configLock);
    os_condDestroy(&_this->terminateCond);
    os_mutexDestroy(&_this->terminateMtx);

    cma__objectDeinit(cma_object(_this));
}

struct cma__serviceWaitActionArg {
    cma_service service;
    u_result result;
};

static os_boolean
cma__serviceWaitAction(
    void *userData,
    void *varg)
{
#if 0 /* todo find out what userData and varg are (disabled code is for u_waitsetWaitAction) */
    struct cma__serviceWaitActionArg *arg = varg;
    cma_service _this = arg->service;

    cma_objectIsValidKind(_this, CMA_OBJECT_SERVICE);

    if (v_eventTest(event->kind, V_EVENT_TRIGGER)) {
        CMA_TRACE(("Service received termination trigger\n"));
        _this->terminate = 1;
        arg->result = U_RESULT_OK;
    } else {
        CMA_ERROR("cma__serviceWaitAction",
            "Received unexpected event %d\n",
            (int)event->kind);
        arg->result = U_RESULT_INTERNAL_ERROR;
    }
#else
    OS_UNUSED_ARG(varg);
#endif
    if (userData) {
        CMA_TRACE(("Service waitset notified with user-data %p, kind %s\n",
            userData, u_kindImage(u_objectKind(userData))));
    } else {
        CMA_TRACE(("Service waitset notified with user-data NULL\n"));
    }

    return OS_TRUE;
}

static void
cma__serviceMain(
    cma_service _this)
{
    u_result result;
    struct cma__serviceWaitActionArg arg;

    arg.service = _this;

    cma_log(LOG_INFO, "cma__serviceMain started\n");
    cma__serviceChangeState(_this, STATE_OPERATIONAL);

    while (!_this->terminate) {
        result = u_waitsetWaitAction2(_this->ws, cma__serviceWaitAction, &arg, OS_DURATION_INFINITE);

        if (result == U_RESULT_OK) {
            /* noop */
        } else if (result == U_RESULT_DETACHING) {
            CMA_INFO("cma__serviceMain", "u_waitsetWaitAction detected termination\n");
            _this->terminate = 1;
        } else {
            CMA_ERROR("cma__serviceMain",
                "u_waitsetWaitAction failed (%s)\n",
                u_resultImage(result));
        }
    }

    cma_log(LOG_INFO, "cma__serviceMain finished\n");
}

static os_result
cma__exitRequestHandler(
    os_callbackArg cbArg,
    void *arg)
{
    cma_service _this = cma_service(arg);

    OS_UNUSED_ARG(cbArg);

    assert(_this);

    _this->terminate = 1;
    (void)u_waitsetNotify(_this->ws, NULL);

    return os_resultSuccess;
}

static os_result
cma__exceptionHandler(
    os_callbackArg cbArg,
    void *arg)
{
    cma_service _this = cma_service(arg);

    OS_UNUSED_ARG(cbArg);
    assert(_this);

    (void)cma__serviceChangeState(_this, STATE_DIED);

    return os_resultSuccess;
}

static c_bool
cma__serviceChangeState(
    cma_service _this,
    v_serviceStateKind state)
{
    c_bool stateChanged;

    cma_objectIsValidKind(_this, CMA_OBJECT_SERVICE);
    assert(_this->service);

    stateChanged = u_serviceChangeState(_this->service, state);
    if (stateChanged) {
        /* TODO publish service-status topic? */
    }

    return stateChanged;
}

static void
cma__serviceWatchSpliced(
    v_serviceStateKind state,
    c_voidp arg /* cma_service */)
{
    cma_service _this;

    cma_objectIsValidKind(arg, CMA_OBJECT_SERVICE);

    _this = cma_service(arg);

    switch(state) {
        case STATE_TERMINATING:
        case STATE_TERMINATED:
        case STATE_DIED:
            CMA_TRACE(("cma__serviceWatchSpliced: Spliced is terminating. cmagent service '%s' %s",
                _this->name, _this->terminate ? "is already terminating" : "will terminate too"));
            /* Set the terminate flag  */
            _this->terminate = 1;
            break;
        default:
            /* do nothing */
            break;
    }
}

static void*
cma__serviceUpdateLease(
    void *arg /* cma_service */)
{
    cma_service _this;
    os_duration expiry;
    os_duration sleep;
    u_result uresult;

    _this = cma_service(arg);
    cma_objectIsValidKind(_this, CMA_OBJECT_SERVICE);

    expiry = cma_configurationLeaseExpiryTime(_this->config);
    sleep = cma_configurationLeaseUpdateInterval(_this->config);

    CMA_TRACE(("Started service-lease renewal thread\n"));

    while (!_this->terminate && !cma_proc_must_exit) {
        uresult = u_serviceRenewLease(_this->service, expiry);
        if (uresult != U_RESULT_OK) {
            /* error reported by u_serviceRenewLease */
            _this->terminate = TRUE;
            break;
        }

        /* wait during update-interval, or service termination */
        os_mutexLock(&_this->terminateMtx);
        {
            os_condTimedWait(&_this->terminateCond, &_this->terminateMtx, sleep);
        }
        os_mutexUnlock(&_this->terminateMtx);
    }

    /* this could be removed once waitset triggers on other conditions */
    (void)u_waitsetNotify(_this->ws, NULL);

    expiry = 20*OS_DURATION_SECOND;
    (void)u_serviceRenewLease(_this->service, expiry);

    CMA_TRACE(("Terminating service-lease renewal thread\n"));

    return NULL;
}

static os_result
cma__serviceOpenTracingFile(
    cma_service _this)
{
    os_result result;
    os_char *filename;
    const char *mode;
    cma_logConfig gc = cma_threadLogConfig(cma_threadLookup());

    cma_objectIsValidKind(_this, CMA_OBJECT_SERVICE);

    result = os_resultSuccess;
    filename = cma_configurationTracingFileName(_this->config);
    gc->tracing.categories = cma_configurationTracingCategories(_this->config);

    if(!filename || gc->tracing.categories == LOG_NONE) {
        /* Tracing is not enabled */
        gc->tracing.file = NULL;
        gc->tracing.categories = 0;
    } else if (os_strcasecmp(filename, "stdout") == 0) {
        gc->tracing.file = stdout;
    } else if (os_strcasecmp(filename, "stderr") == 0) {
        gc->tracing.file = stderr;
    } else {
        mode = (cma_configurationTracingAppend(_this->config)) ? "a" : "w";
        gc->tracing.file = fopen(filename, mode);
        if (!gc->tracing.file) {
            const os_char *msg = os_getErrno() ? os_strError(os_getErrno()) : NULL;
            CMA_ERROR("nb_serviceOpenTracingFile",
                "Cannot open tracing logfile '%s' (%s)\n",
                filename, msg ? msg : "unknown reason");
            result = os_resultFail;
        }
    }

    if(result == os_resultSuccess) {
        CMA_TRACE(("Opened CMAgent tracing file '%s'\n", filename));
    }

    return result;
}

const os_char*
cma_serviceName(
    cma_service _this)
{
    cma_objectIsValidKind(_this, CMA_OBJECT_SERVICE);
    return _this->name;
}

u_service
cma_serviceService(
    cma_service _this)
{
    cma_objectIsValidKind(_this, CMA_OBJECT_SERVICE);
    return _this->service;
}
