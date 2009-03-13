#include "u__user.h"

#include "u_spliced.h"
#include "u__types.h"
#include "u__entity.h"
#include "u__kernel.h"
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
u_result
u_splicedClaim(
    u_spliced _this,
    v_spliced *spliced)
{
    u_result result = U_RESULT_OK;

    if ((_this != NULL) && (spliced != NULL)) {
        *spliced = v_spliced(u_entityClaim(u_entity(_this)));
        if (*spliced == NULL) {
            OS_REPORT_2(OS_WARNING, "u_splicedClaim", 0,
                        "Claim Spliced failed. "
                        "<_this = 0x%x, spliced = 0x%x>.",
                         _this, spliced);
            result = U_RESULT_INTERNAL_ERROR;
        }
    } else {
        OS_REPORT_2(OS_ERROR,"u_splicedClaim",0,
                    "Illegal parameter. "
                    "<_this = 0x%x, spliced = 0x%x>.",
                    _this, spliced);
        result = U_RESULT_ILL_PARAM;
    }
    return result;
}

u_result
u_splicedRelease(
    u_spliced _this)
{
    u_result result = U_RESULT_OK;

    if (_this != NULL) {
        result = u_entityRelease(u_entity(_this));
    } else {
        OS_REPORT_1(OS_ERROR,"u_splicedRelease",0,
                    "Illegal parameter. <_this = 0x%x>.", _this);
        result = U_RESULT_ILL_PARAM;
    }
    return result;
}

#ifndef INTEGRITY
#define DAEMON_PATH "Domain/Daemon" 
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
                iter = v_cfElementXPath(service, "MemoryLocking/#text");
                iterLength = c_iterLength(iter);
                if (iterLength == 1) {
                    data = v_cfData(c_iterTakeFirst(iter));
                    c_iterFree(iter);
                    if (u_cfValueScan(v_cfDataValue(data), V_BOOLEAN, &value)) {
                        if (value.is.Boolean) {
                            lock = 1;
                            OS_REPORT(OS_INFO,"lockPages", 0,
                                "Daemon: MemoryLocking enabled for spliced");
                        }
                    } else {
                        OS_REPORT(OS_WARNING,"lockPages", 0,
                            "Failed to retrieve MemoryLocking for Daemon: MemoryLocking disabled");
                    }
                } else {
                    OS_REPORT(OS_INFO,"lockPages", 0,
                        "Splicedaemon: MemoryLocking disabled");
                }
            } else {
                OS_REPORT(OS_WARNING,"lockPages", 0,
                    "Could not get configuration for Daemon (non-existent or too many): MemoryLocking disabled");
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
    u_kernel k)
{
    u_result r;
    v_kernel kk;
    c_iter participants;
    v_spliced spliced;

    r = u_kernelClaim(k,&kk);
    if ((r == U_RESULT_OK) && (kk != NULL)) {
        participants = v_resolveParticipants(kk, V_SPLICED_NAME);
        r = u_kernelRelease(k);
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
    u_kernel k)
{
    u_result result;

    if (spliced != NULL) {
        result = u_serviceInit(u_service(spliced), U_SERVICE_SPLICED, k);
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
    u_spliced spliced)
{
    return u_serviceDeinit(u_service(spliced));
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
    u_kernel k;
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

#if !defined (VXWORKS_RTP) && !defined (__INTEGRITY)
    k = u_userKernelOpen(uri, -1 /* no timeout and no error logging */);
    if (k != NULL) {
        printf("Database opened, opening kernel\n");
        r = u_kernelClaim(k,&kk);
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
            r = u_kernelRelease(k);
        }
        if (otherSpliced) {
            printf("Other splicedaemon running!\n");
            u_userKernelClose(k);
        } else {
            u_kernelFree(k);
            k = u_kernelNew(uri);
            if (k == NULL) {
                printf("Creation of kernel failed!\n");
            } else {
                /* create new proxy to v_spliced object */
                kSpliced = getKernelSplicedaemon(k);
            }
        }
    } else
#endif
    {
	
        k = u_kernelNew(uri);
        if (k == NULL) {
            printf("Creation of kernel failed!\n");
        } else {
            /* create new proxy to v_spliced object */
            kSpliced = getKernelSplicedaemon(k);
        }
    }

    if (kSpliced != NULL) {
        spliced = u_entityAlloc(NULL,u_spliced,kSpliced,TRUE);
        r = u_splicedInit(spliced, k);
        if (r != U_RESULT_OK) {
            u_serviceFree(u_service(spliced));
            OS_REPORT(OS_ERROR,"u_splicedNew",0,
                      "Failed to initialise spliced.");
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
    u_spliced spliced)
{
    u_result r;
    u_kernel kernel;

    if (spliced != NULL) {
        if (u_entity(spliced)->flags & U_ECREATE_INITIALISED) {
            kernel = u_participant(spliced)->kernel;
            r = u_splicedDeinit(spliced);
            os_free(spliced);
    
            /* now close the kernel: should always be the last action of
               this routine!
            */
            /* u_userKernelClose(kernel); in the future */
            u_kernelFree(kernel);
        } else {
            r = u_entityFree(u_entity(spliced));
        }
    } else {
        OS_REPORT(OS_WARNING,"u_splicedFree",0,
                  "The specified Spliced = NIL.");
        r = U_RESULT_OK;
    }
    return r;
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

    r = u_splicedClaim(spliced, &s);
    if ((r == U_RESULT_OK) && (s != NULL)) {
        v_splicedKernelManager(s);
        r = u_splicedRelease(spliced);
    } else {
        OS_REPORT(OS_WARNING, "u_splicedKernelManager", 0, 
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

    r = u_splicedClaim(spliced, &s);
    if ((r == U_RESULT_OK) && (s != NULL)) {
        v_splicedGarbageCollector(s);
        r = u_splicedRelease(spliced);
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

    r = u_splicedClaim(spliced, &s);
    if ((r == U_RESULT_OK) && (s != NULL)) {
        v_splicedPrepareTermination(s);
        r = u_splicedRelease(spliced);
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

    r = u_splicedClaim(spliced, &s);
    if ((r == U_RESULT_OK) && (s != NULL)) {
        started = v_splicedStartHeartbeat(s, period, renewal);
        if (started == FALSE) {
            r = U_RESULT_INTERNAL_ERROR;
            u_splicedRelease(spliced);
        } else {
            r = u_splicedRelease(spliced);
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

    r = u_splicedClaim(spliced, &s);
    if ((r == U_RESULT_OK) && (s != NULL)) {
        stopped = v_splicedStopHeartbeat(s);
        if (stopped == FALSE) {
            r = U_RESULT_INTERNAL_ERROR;
            u_splicedRelease(spliced);
        } else {
            r = u_splicedRelease(spliced);
        }
    } else {
        OS_REPORT(OS_WARNING, "u_splicedStopHeartbeat", 0, 
                  "Could not claim spliced.");
    }
    return r;
}
