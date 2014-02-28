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

static c_bool splicedStartedInThisProcess = FALSE;


/**************************************************************
 * Private functions
 **************************************************************/

#ifndef INTEGRITY
#define DAEMON_PATH "Domain/Daemon"
#define DAEMON_PATH_DEPRECATED "Daemon"

static int
checkLockingEnabled(v_cfElement root, const c_char *path, int *lock) {

    v_cfElement service;
    v_cfData data;
    c_iter iter;
    int iterLength;
    int iterLengthRoot;
    c_value value;
    int found= 0;
    iter = v_cfElementXPath(root, path);
    iterLengthRoot = c_iterLength(iter);
    if (iterLengthRoot > 0) {
        service = v_cfElement(c_iterTakeFirst(iter));
        c_iterFree(iter);
        iter = v_cfElementXPath(service, "Locking/#text");
        iterLength = c_iterLength(iter);
        if (iterLength > 0) {
            data = v_cfData(c_iterTakeFirst(iter));
            c_iterFree(iter);
            if (u_cfValueScan(v_cfDataValue(data), V_BOOLEAN, &value)) {
                if (value.is.Boolean) {
                    *lock = 1;
                } else {
                    *lock = 0;
                }
                found = 1;
            }
        }
        if (iterLength > 1 && found) {
            OS_REPORT(OS_WARNING,"lockPages", 0,"Daemon: multiple locking tags found, first one used");
        }
    }
    if (iterLengthRoot > 1) {
        OS_REPORT(OS_WARNING,"lockPages", 0,"Daemon: multiple daemon tags found, first tag used to read locking");
    }
    return found;
}

static int
lockPages(
    v_spliced kSpliced)
{
    v_configuration cfg;
    v_cfElement root;
    int lock;
    int result;
    v_kernel k;

    assert(kSpliced);

    lock = 0;
    k = v_objectKernel(kSpliced);
    assert(k);
    cfg = v_getConfiguration(k);
    if (cfg != NULL) {
        root = v_configurationGetRoot(cfg);
        if (root != NULL) {
            result = checkLockingEnabled(root,DAEMON_PATH,&lock);
            if (result && lock) {
                OS_REPORT(OS_INFO,"lockPages", 0,"Daemon: Locking enabled for spliced");
            } else if (result && !lock) {
                OS_REPORT(OS_INFO,"lockPages", 0,"Daemon: Locking disabled for spliced");
            }
            if (!result) {
                result = checkLockingEnabled(root,DAEMON_PATH_DEPRECATED, &lock);
                if (result && lock) {
                    OS_REPORT(OS_WARNING,"lockPages", 0,"DEPRECATED location for " DAEMON_PATH_DEPRECATED "/Locking location changed to " DAEMON_PATH "/Locking : Locking enabled for spliced");
                } else if (result && !lock) {
                    OS_REPORT(OS_WARNING,"lockPages", 0,"DEPRECATED location for " DAEMON_PATH_DEPRECATED "/Locking location changed to " DAEMON_PATH "/Locking : Locking disabled for spliced");
                }
            }
            c_free(root);
        }
    }
    return lock;
}
#undef DAEMON_PATH
#undef DAEMON_PATH_DEPRECATED
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
    r = u_domainOpen(&domain, uri, -1);

    if (r == U_RESULT_PRECONDITION_NOT_MET)
    {
        /* If the return code is PRECONDITION_NOT_MET it means that
           there is a problem with the report plugins and so the spliced
           should fail to start.  If the return code is 0 it means
           that a domain was not opened or 1 means a domain was opened, in 
           both cases we should continue */
        return spliced;
    }

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
        u_domainFree(domain);
        if (otherSpliced) {
            printf("Other splicedaemon running!\n");
        } else {
            r = u_domainNew(&domain, uri);
            if (r != U_RESULT_OK) {
                printf("Creation of kernel failed! - return code %d\n", r);
            } else {
                /* create new proxy to v_spliced object */
                kSpliced = getKernelSplicedaemon(domain);
            }
        }
    } else
#endif
    {

        r = u_domainNew(&domain, uri);

        if (r != U_RESULT_OK) {
            printf("Creation of kernel failed! Return code %d\n", r);
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
#if 0
                result = u_domainFree(domain);
                if (result != U_RESULT_OK) {
                    OS_REPORT_2(OS_ERROR,
                                "user::u_spliced::u_splicedFree", 0,
                                "Operation u_domainFree(0x%x) failed."
                                OS_REPORT_NL "result = %s.",
                                domain, u_resultImage(result));
                }
#endif
            } else {
                OS_REPORT_2(OS_ERROR,
                            "user::u_spliced::u_splicedFree",0,
                            "Operation u_splicedDeinit(0x%x) failed: "
                            "result = %s.",
                            _this, u_resultImage(result));
                u_entityUnlock(u_entity(_this));
            }
        } else {
            u_entityUnlock(u_entity(_this));
        }
    } else {
        OS_REPORT_2(OS_ERROR,
                    "user::u_spliced::u_splicedFree",0,
                    "Operation u_entityLock(0x%x) failed: "
                    "result = %s.",
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
u_splicedCAndMCommandDispatcherQuit(
   u_spliced spliced)
{
   u_result r;
   v_spliced s;

   r = u_entityReadClaim(u_entity(spliced), (v_entity*)(&s));
   if (r == U_RESULT_OK) {
      assert(s);
      v_splicedCAndMCommandDispatcherQuit(s);
      r = u_entityRelease(u_entity(spliced));
   } else {
      OS_REPORT(OS_WARNING,
                "u_splicedBuiltinCAndMCommandDispatcherQuit", 0,
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

        /* Request shutdown of & wakeup cAndMCommandManager thread */
        v_splicedCAndMCommandDispatcherQuit(s);
        r = u_entityRelease(u_entity(spliced));
    } else {
        OS_REPORT(OS_WARNING, "u_splicedPrepareTermination", 0,
                  "Could not claim spliced.");
    }
    return r;
}

v_leaseManager
u_splicedGetHeartbeatManager(
    u_spliced spliced,
    c_bool create)
{
    v_leaseManager lm = NULL;
    v_spliced s;
    u_result result;

    result = u_entityReadClaim(u_entity(spliced), (v_entity*)(&s));
    if(result == U_RESULT_OK)
    {
        assert(s);
        lm = v_splicedGetHeartbeatManager(s, create);
        u_entityRelease(u_entity(spliced));
    } else {
         OS_REPORT(OS_WARNING, "u_splicedGetHeartbeatManager", 0,
                   "Could not claim spliced.");
    }

    return lm;
}

u_result
u_splicedStartHeartbeat(
    u_spliced spliced,
    v_duration period,
    v_duration renewal,
    c_long priority)
{
    u_result r;
    v_spliced s;
    c_bool started = TRUE;

    r = u_entityReadClaim(u_entity(spliced), (v_entity*)(&s));
    if (r == U_RESULT_OK) {
        assert(s);
        started = v_splicedStartHeartbeat(s, period, renewal, priority);
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

    if(spliced) {
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
            OS_REPORT_1(OS_WARNING, "u_splicedStopHeartbeat", 0,
                      "Could not claim spliced, result was %s.", u_resultImage(r));
        }
    } else {
        r = U_RESULT_ILL_PARAM;
    }
    return r;
}

u_result
u_splicedSetInProcess()
{
    splicedStartedInThisProcess = TRUE;
    return U_RESULT_OK;
}

c_bool
u_splicedInProcess()
{
    return splicedStartedInThisProcess;
}
