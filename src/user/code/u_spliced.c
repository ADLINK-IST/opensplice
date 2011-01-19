/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2010 PrismTech
 *   Limited and its licensees. All rights reserved. See file:
 *
 *                     $OSPL_HOME/LICENSE
 *
 *   for full copyright notice and license terms.
 *
 */
#include "u__user.h"

#include "u_spliced.h"
#include "u__types.h"
#include "u__entity.h"
#include "u__domain.h"
#include "u__cfValue.h"

#include "v_entity.h"
#include "v_spliced.h"
#include "v_serviceManager.h"
#include "v_configuration.h"

#include "v_leaseManager.h"
#include "os_report.h"

/**************************************************************
 * Private functions
 **************************************************************/

#ifndef INTEGRITY
#define DAEMON_PATH "Daemon"
static int
lockPages(
    v_spliced kSpliced)
{
    v_configuration cfg;
    v_cfElement root;
    v_cfElement service;
    v_cfData data;
    int lock;
    c_iter iter;
    int iterLength;
    c_value value;
    v_kernel k;

    assert(kSpliced);

    lock = 0;
    k = v_objectKernel(kSpliced);
    assert(k);
    cfg = v_getConfiguration(k);
    if (cfg != NULL) {
        root = v_configurationGetRoot(cfg);
        if (root != NULL) {
            iter = v_cfElementXPath(root, DAEMON_PATH);
            iterLength = c_iterLength(iter);
            if (iterLength == 1) {
                service = v_cfElement(c_iterTakeFirst(iter));
                c_iterFree(iter);
                iter = v_cfElementXPath(service, "Locking/#text");
                iterLength = c_iterLength(iter);
                if (iterLength == 1) {
                    data = v_cfData(c_iterTakeFirst(iter));
                    c_iterFree(iter);
                    if (u_cfValueScan(v_cfDataValue(data), V_BOOLEAN, &value)) {
                        if (value.is.Boolean) {
                            lock = 1;
                            OS_REPORT(OS_INFO,"lockPages", 0,
                                "Daemon: Locking enabled for spliced");
                        }
                    } else {
                        OS_REPORT(OS_WARNING,"lockPages", 0,
                            "Failed to retrieve Locking for Daemon: Locking disabled");
                    }
                } else {
                    OS_REPORT(OS_INFO,"lockPages", 0,
                        "Daemon: Locking disabled");
                    c_iterFree(iter);
                }
            } else {
                c_voidp e;
                OS_REPORT(OS_INFO,"lockPages", 0,
                    "No configuration specified for Daemon. Therefore the default will be used: Locking disabled");
                do{
                    e = c_iterTakeFirst(iter);
                    c_free(e);
                } while(e != NULL);
                c_iterFree(iter);
            }
            c_free(root);
        }
    }

    return lock;
}
#undef DAEMON_PATH
#endif

static v_spliced
getKernelSplicedaemon(
    u_domain k)
{
    u_result r;
    v_kernel kk;
    c_iter participants;
    v_spliced spliced;

    r = u_entityReadClaim(u_entity(k),(v_entity*)(&kk));
    if (r == U_RESULT_OK) {
        assert(kk);
        participants = v_resolveParticipants(kk, V_SPLICED_NAME);
        r = u_entityRelease(u_entity(k));
        assert(c_iterLength(participants) == 1);
        spliced = v_spliced(c_iterTakeFirst(participants));
        c_iterFree(participants);
    } else {
        OS_REPORT(OS_WARNING,"u_splicedNew::getKernelSplicedaemon",0,
                  "Claim Kernel failed.");
        spliced = NULL;
    }
    return spliced;
}

static u_result
u_splicedInit(
    u_spliced spliced,
    u_domain domain)
{
    u_result result;

    if (spliced != NULL) {
        result = u_serviceInit(u_service(spliced), U_SERVICE_SPLICED, domain);
        u_entity(spliced)->flags |= U_ECREATE_INITIALISED;
    } else {
        OS_REPORT(OS_ERROR,"u_splicedInit",0,
                  "Illegal parameter.");
        result = U_RESULT_ILL_PARAM;
    }
    return result;
}

static u_result
u_splicedDeinit(
    u_spliced _this)
{
    u_result r;
    v_spliced s;

    r = u_entityReadClaim(u_entity(_this), (v_entity*)(&s));
    if (r == U_RESULT_OK) {
        assert(s);
        v_splicedCAndMCommandDispatcherQuit(s);
        r = u_entityRelease(u_entity(_this));
    }

    return u_serviceDeinit(u_service(_this));
}

/**************************************************************
 * constructor/destructor
 **************************************************************/

#define SPLICED_NAME "spliced"

u_spliced
u_splicedNew(
    const c_char *uri)
{
    u_result r;
    u_domain domain;
    v_kernel kk;
    v_serviceStateKind sdState;
    v_serviceManager sm;
    c_long nrSec;
    c_bool otherSpliced = FALSE;
    os_time pollDelay = {1, 0};
    v_spliced kSpliced;
    u_spliced spliced;
    os_result osr;

    kSpliced = NULL;
    spliced = NULL;

#if !defined (VXWORKS_RTP) && !defined (__INTEGRITY) && !defined (WINCE)
    domain = u_userFindDomain(uri, -1 /* no timeout and no error logging */);
    if (domain != NULL) {
        printf("Database opened, opening kernel\n");
        r = u_entityWriteClaim(u_entity(domain),(v_entity*)(&kk));
        if ((r == U_RESULT_OK) && (kk != NULL)) {
            sm = v_getServiceManager(kk);

            nrSec = 0;
            do {
                sdState = v_serviceManagerGetServiceStateKind(sm, V_SPLICED_NAME);
                if ((sdState == STATE_TERMINATED) || (sdState == STATE_DIED)) {
                    otherSpliced = FALSE;
                } else {
                    otherSpliced = TRUE;
                }
                nrSec++;
                if (otherSpliced == TRUE) {
                    os_nanoSleep(pollDelay);
                }
            } while (otherSpliced && (nrSec < 4));
            r = u_entityRelease(u_entity(domain));
        }
        u_userKernelClose(domain);
        if (otherSpliced) {
            printf("Other splicedaemon running!\n");
        } else {
            domain = u_userCreateDomain(uri);
            if (domain == NULL) {
                printf("Creation of kernel failed!\n");
            } else {
                /* create new proxy to v_spliced object */
                kSpliced = getKernelSplicedaemon(domain);
            }
        }
    } else
#endif
    {

        domain = u_userCreateDomain(uri);
        if (domain == NULL) {
            printf("Creation of kernel failed!\n");
        } else {
            /* create new proxy to v_spliced object */
            kSpliced = getKernelSplicedaemon(domain);
        }
    }

    if (kSpliced != NULL) {
        spliced = u_entityAlloc(NULL,u_spliced,kSpliced,TRUE);
        r = u_splicedInit(spliced, domain);
        if (r != U_RESULT_OK) {
            u_serviceFree(u_service(spliced));
            OS_REPORT(OS_ERROR,"u_splicedNew",0,
                      "Failed to initialize spliced.");
            spliced = NULL;
        } else {
#ifndef INTEGRITY
            if (lockPages(kSpliced)) {
                osr = os_procMLockAll(OS_MEMLOCK_CURRENT | OS_MEMLOCK_FUTURE);
                if (osr != os_resultSuccess) {
                    OS_REPORT(OS_ERROR,"u_splicedNew",0,
                              "Failed to lock process memory pages");
                    (void)u_splicedFree(spliced);
                    spliced = NULL;
                }
            }
#endif
        }
    }
    return spliced;
}

#undef SPLICED_NAME

u_result
u_splicedFree(
    u_spliced _this)
{
    u_result result;
    u_domain domain;
    c_bool destroy;

    result = u_entityLock(u_entity(_this));
    if (result == U_RESULT_OK) {
        destroy = u_entityDereference(u_entity(_this));
        /* if refCount becomes zero then this call
         * returns true and destruction can take place
         */
        if (destroy) {
            domain = u_participantDomain(u_participant(_this));
            result = u_splicedDeinit(_this);
            if (result == U_RESULT_OK) {
                u_entityDealloc(u_entity(_this));
                u_userDeleteDomain(domain);
            } else {
                OS_REPORT_2(OS_WARNING,
                            "u_splicedFree",0,
                            "Operation u_splicedDeinit failed: "
                            "Spliced = 0x%x, result = %s.",
                            _this, u_resultImage(result));
                u_entityUnlock(u_entity(_this));
            }
        } else {
            u_entityUnlock(u_entity(_this));
        }
    } else {
        OS_REPORT_2(OS_WARNING,
                    "u_splicedFree",0,
                    "Operation u_entityLock failed: "
                    "Spliced = 0x%x, result = %s.",
                    _this, u_resultImage(result));
    }
    return result;
}

/**************************************************************
 * Protected functions
 **************************************************************/

/**************************************************************
 * Public functions
 **************************************************************/
u_result
u_splicedKernelManager(
    u_spliced spliced)
{
    u_result r;
    v_spliced s;

    r = u_entityReadClaim(u_entity(spliced), (v_entity*)(&s));
    if (r == U_RESULT_OK) {
        assert(s);
        v_splicedKernelManager(s);
        r = u_entityRelease(u_entity(spliced));
    } else {
        OS_REPORT(OS_WARNING, "u_splicedKernelManager", 0,
                  "Could not claim spliced.");
    }
    return r;
}

u_result
u_splicedBuiltinResendManager(
    u_spliced spliced)
{
    u_result r;
    v_spliced s;

    r = u_entityReadClaim(u_entity(spliced), (v_entity*)(&s));
    if (r == U_RESULT_OK) {
        assert(s);
        v_splicedBuiltinResendManager(s);
        r = u_entityRelease(u_entity(spliced));
    } else {
        OS_REPORT(OS_WARNING, "u_splicedBuiltinResendManager", 0,
                  "Could not claim spliced.");
    }
    return r;
}

u_result
u_splicedBuiltinCAndMCommandDispatcher(
    u_spliced spliced)
{
    u_result r;
    v_spliced s;

    r = u_entityReadClaim(u_entity(spliced), (v_entity*)(&s));
    if (r == U_RESULT_OK) {
        assert(s);
        v_splicedBuiltinCAndMCommandDispatcher(s);
        r = u_entityRelease(u_entity(spliced));
    } else {
        OS_REPORT(OS_WARNING, "u_splicedBuiltinCAndMCommandDispatcher", 0,
                  "Could not claim spliced.");
    }
    return r;
}

u_result
u_splicedGarbageCollector(
    u_spliced spliced)
{
    u_result r;
    v_spliced s;

    r = u_entityReadClaim(u_entity(spliced), (v_entity*)(&s));
    if (r == U_RESULT_OK) {
        assert(s);
        v_splicedGarbageCollector(s);
        r = u_entityRelease(u_entity(spliced));
    } else {
        OS_REPORT(OS_WARNING, "u_splicedGarbageCollector", 0,
                  "Could not claim spliced.");
    }
    return r;
}

u_result
u_splicedPrepareTermination(
    u_spliced spliced)
{
    u_result r;
    v_spliced s;

    r = u_entityReadClaim(u_entity(spliced), (v_entity*)(&s));
    if (r == U_RESULT_OK) {
        assert(s);
        v_splicedPrepareTermination(s);
        r = u_entityRelease(u_entity(spliced));
    } else {
        OS_REPORT(OS_WARNING, "u_splicedPrepareTermination", 0,
                  "Could not claim spliced.");
    }
    return r;
}

u_result
u_splicedStartHeartbeat(
    u_spliced spliced,
    v_duration period,
    v_duration renewal)
{
    u_result r;
    v_spliced s;
    c_bool started;

    r = u_entityReadClaim(u_entity(spliced), (v_entity*)(&s));
    if (r == U_RESULT_OK) {
        assert(s);
        started = v_splicedStartHeartbeat(s, period, renewal);
        if (started == FALSE) {
            r = U_RESULT_INTERNAL_ERROR;
            u_entityRelease(u_entity(spliced));
        } else {
            r = u_entityRelease(u_entity(spliced));
        }
    } else {
        OS_REPORT(OS_WARNING, "u_splicedStartHeartbeat", 0,
                  "Could not claim spliced.");
    }
    return r;
}

u_result
u_splicedStopHeartbeat(
    u_spliced spliced)
{
    u_result r;
    v_spliced s;
    c_bool stopped;

    r = u_entityReadClaim(u_entity(spliced), (v_entity*)(&s));
    if (r == U_RESULT_OK) {
        assert(s);
        stopped = v_splicedStopHeartbeat(s);
        if (stopped == FALSE) {
            r = U_RESULT_INTERNAL_ERROR;
            u_entityRelease(u_entity(spliced));
        } else {
            r = u_entityRelease(u_entity(spliced));
        }
    } else {
        OS_REPORT(OS_WARNING, "u_splicedStopHeartbeat", 0,
                  "Could not claim spliced.");
    }
    return r;
}

