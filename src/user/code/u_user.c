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
#include "u__user.h"
#include "u__domain.h"
#include "u__types.h"
#include "u__service.h"
#include "u_entity.h"
#include "cfg_parser.h"
#include "cf_config.h"

#include "vortex_os.h"
#include "os_report.h"
#include "os_signalHandler.h"
#include "os_atomics.h"

#define MAX_DOMAINS (64)

#define u_user(u) ((C_STRUCT(u_user) *)(u))

C_CLASS(u_domainAdmin);
C_STRUCT(u_domainAdmin) { /* protected by global user lock */
    u_domain domain;
};

C_CLASS(u_user);
C_STRUCT(u_user) {
    /* The following mutex implements the global user lock.
     * all access to user info is under control of this lock.
     */
    os_mutex mutex;
    os_cond cond;
    /*
     * The domainList attribute holds information about all connected domains.
     * The domainCount attribute specifies the max index in the domainList.
     * So any search range on the list can be limited to 1..domainCount.
     * domains that are detached are removed from the list and the entry in the
     * list is never used again.
     * The value 0 is reserved for the 'no domain' use-case,
     * so if domainCount = 0 then no domains are attached.
     * The reason why domain entries are never reused and why domainCount is not
     * the actual number of domains connected is unclear, it would be more intuitive,
     * maintainable and flexible if entries in the domainList could be reused and the
     * domainCount would reflect the actual number of connected domains.
     * So this is a subject for future improvement.
     */
    C_STRUCT(u_domainAdmin) domainList[MAX_DOMAINS];
    c_long domainCount;
    /* Should only be modified by pa_(in/de)crement! */
    os_uint32 protectCount;
    /* The detachThreadId will have to be set by the detaching thread while
     * holding the user->mutex and be re-set to NULL before releasing the
     * lock. A lock-boundary should thus never be crossed with detachThreadId
     * set to a different value than when the lock was acquired! */
    os_threadId detachThreadId;
    os_uint detached;
    os_boolean detachingDomain;
    /* Indicates whether u_userSetupSignalHandling has been called */
    os_boolean signalHandlingSetup;
};

/** \brief Counter that keeps track of number of times user-layer is initialized.
 *
 * The main purpose of this counter is to ensure that the user-layer is
 * initialized only once.
 */
static pa_uint32_t _ospl_userInitCount = PA_UINT32_INIT(0);

/** \brief Reference to heap user-layer admin object.
 *
 * This reference is only initialized once and implements the root to
 * all user (process) specific information.
 */
void *user = NULL;

/* This method will lock the user-layer and return the reference to the user-layer object if successful.
 * If this method returns NULL then the user-layer is either not initialized or
 * the process is detaching (process termination).
 */
static u_user
u__userLock(void)
{
    u_user u;
    os_result r = os_resultFail;

    u = u_user(user);

    assert(u != NULL);

    r = os_mutexLock_s(&u->mutex);
    if (r != os_resultSuccess) {
        /* The mutex is not valid so apparently the user-layer is either
         * destroyed or in process of destruction. */
        u = NULL;
    } else if (u->detached || (os_threadIdToInteger(u->detachThreadId) != 0 &&
               (os_threadIdToInteger(u->detachThreadId) !=
                os_threadIdToInteger(os_threadIdSelf()))))
    {
        /* Another thread is busy destroying the user-layer or the user-
         * layer is already destroyed. No access is allowed (anymore).
         * The user-layer object will be unlocked and will return null.
         */
        os_mutexUnlock(&u->mutex);
        u = NULL;
    }

    return u;
}

/* This method will unlock the user-layer.
 */
static void
u__userUnlock(void)
{
    u_user u;

    u = u_user(user);

    assert(u != NULL);

    os_mutexUnlock(&u->mutex);
}

static u_result
u__userDetach(
    _In_ os_uint32 flags)
{
    u_user u;
    os_uint32 i;
    u_result result = U_RESULT_OK;
    u_result dresult;

    u = u_user(user);

    os_mutexLock(&u->mutex);
    /* Disable access to user-layer for all other threads except for this thread.
     * Any following user access from other threads is gracefully
     * aborted. */
    if(os_threadIdToInteger(u->detachThreadId) != 0) {
        while(!u->detached) {
            os_condWait(&u->cond, &u->mutex);
        }
    } else {
        u->detachThreadId = os_threadIdSelf();

        /* Unlock the user-layer
         * Part of following code requires to unlock the user object
         * This is allowed now all other threads will abort when
         * trying to claim the lock
         */
        os_mutexUnlock(&u->mutex);
        for (i = 1; (i < MAX_DOMAINS); i++) {
            dresult = u__userDomainDetach(i, flags);
            if(result == U_RESULT_OK) {
                result = dresult;
            }
        }

        /* Now signal other threads waiting on u__userDetach(...) that the
         * work is done. */
        os_mutexLock(&u->mutex);
        u->detached = 1;
        os_condBroadcast(&u->cond);
    }
    os_mutexUnlock(&u->mutex);

    return result;
}

u_result
u__userDomainDetach(
    _In_ os_uint32 idx,
    _In_ os_uint32 flags)
{
    u_bool detach = FALSE;
    u_domain domain;
    u_result r = U_RESULT_OK;
    u_user u;

    if (idx > 0) {
        u = u__userLock();
        if (u) {
            domain = u->domainList[idx].domain;
            if (domain) {
                detach = u_domainSetDetaching(domain, flags);
            }
            u__userUnlock();

            if (detach) {
                r = u_domainDetach(domain);
                if (r != U_RESULT_OK) {
                    OS_REPORT(OS_ERROR,
                            "user::u_user::u__userDetach", r,
                            "Operation u_domainDetach(0x%"PA_PRIxADDR")(%u) for domain \"%s\" (%u) failed."
                            OS_REPORT_NL "result = %s",
                            (os_address)domain, flags, u_domainName(domain), u_domainId(domain),
                            u_resultImage(r));
                }
            } else if (domain) {
                u_domainWaitDetaching(domain);
            }
        }
    }
    return r;
}

static os_ulong_int
u__userGetDetachThreadId(void)
{
    u_user u;
    os_result r = os_resultFail;
    os_ulong_int threadId = 0;
    u = u_user(user);

    assert(u != NULL);

    r = os_mutexLock_s(&u->mutex);
    if (r != os_resultSuccess) {
        /* The mutex is not valid so apparently the user-layer is either
         * destroyed or in process of destruction. Let id be 0*/
    } else {
        threadId = os_threadIdToInteger(u->detachThreadId);
    }
    os_mutexUnlock(&u->mutex);
    return threadId;
}

u_result
u_userDetach(
    _In_ os_uint32 flags)
{
    u_result result = U_RESULT_OK;

    if(!os_serviceGetSingleProcess()) {
        result = u__userDetach(flags);
    }

    return result;
}

static os_result
u__userExceptionCallbackWrapper(
        os_callbackArg exceptionCallbackArg, void *unused)
{
    OS_UNUSED_ARG(unused);

    if ((exceptionCallbackArg.ThreadId == 0) || (exceptionCallbackArg.ThreadId != u__userGetDetachThreadId())) {
        OS_REPORT(OS_ERROR, "u__userExceptionCallbackWrapper", U_RESULT_INTERNAL_ERROR,
                  "Exception occurred, the process now disconnects from the domain");
        u__userDetach(U_USER_BLOCK_OPERATIONS | U_USER_EXCEPTION);
    }
    else {
        OS_REPORT(OS_ERROR, "u__userExceptionCallbackWrapper", U_RESULT_INTERNAL_ERROR,
                  "Exception occurred in u_detach, the exceptionHandler will NOT disconnects the process from the domain");
    }

    return os_resultSuccess;
}

static os_result
u__userExitRequestCallbackWrapper(
    os_callbackArg exitCallbackArg,
    void * unused)
{
    OS_UNUSED_ARG(unused);
    if ((exitCallbackArg.ThreadId == 0) || (exitCallbackArg.ThreadId != u__userGetDetachThreadId())) {
        OS_REPORT(OS_WARNING, "u__userExitRequestCallbackWrapper", U_RESULT_OK,
                  "Received termination request, will detach user-layer from domain.");
        u__userDetach(U_USER_DELETE_ENTITIES);
    }
    else {
        OS_REPORT(OS_WARNING, "u__userExitRequestCallbackWrapper", U_RESULT_OK,
                  "Received termination request from within u_detach process, signalHandler process will NOT detach user-layer from domain.");
    }

    return os_signalHandlerFinishExitRequest(exitCallbackArg);
}

void u_userSetupSignalHandling (c_bool installExitRequestHandler)
{
    u_user u;
    if ((u = u__userLock ()) != NULL) {
        if (!u->signalHandlingSetup) {
            u->signalHandlingSetup = OS_TRUE;
            /* As long as there is no deinit for the user-layer, these handlers
             * cannot be unregistered. The handles are thus explicitly leaked
             * here.
             * TODO: Add cleanup when OSPL-5853 is done. */
            if (!os_serviceGetSingleProcess()) {
                (void) os_signalHandlerNew();
                if (installExitRequestHandler) {
                    (void) os_signalHandlerRegisterExitRequestCallback(u__userExitRequestCallbackWrapper, NULL);
                }
                (void) os_signalHandlerRegisterExceptionCallback(u__userExceptionCallbackWrapper, NULL);
            }
        }
        u__userUnlock();
    }
}

os_int32
u_userGetDomainId(void *arg)
{
    OS_UNUSED_ARG(arg);

    return v_kernelThreadInfoGetDomainId();
}

u_result
u_userInitialise(
    void)
{
    u_user u;
    u_result rm = U_RESULT_OK;
    os_uint32 initCount;
    void* initUser;

    initCount = pa_inc32_nv(&_ospl_userInitCount);
    /* Any participant being an application or service may call this operation but
     * initialization should only be performed once per process.
     * So only initialize when initCount == 1.
     */
    if (initCount == 1) {
        /* Will start allocating the object, so it should currently be empty. */
        assert(user == NULL);

        /* Prepare the OS layer. */
        os_osInit();

        os_reportRegisterDomainCallback(u_userGetDomainId, NULL);

        /* Prepare the configuration layer. */
        if (cfg_parse_init() != CFGPRS_OK) {
            OS_REPORT(OS_ERROR, "u_userInitialise", U_RESULT_INTERNAL_ERROR,
                      "Operation cfg_parse_init() failed.");
            assert(0);
        }

        /* Prepare the service component. */
        u__serviceInitialise();

        /* Use indirection, as user != NULL is a precondition for user-layer
         * functions, so make sure it only holds true when the user-layer is
         * initialized. */
        initUser = os_malloc(sizeof(C_STRUCT(u_user)));

        u = u_user(initUser);
        os_mutexInit(&u->mutex, NULL);
        os_condInit(&u->cond ,&u->mutex, NULL);
        u->domainCount = 0;
        u->protectCount = 0;
        u->detachThreadId = OS_THREAD_ID_NONE;
        u->detached = 0;
        u->detachingDomain = OS_FALSE;
        u->signalHandlingSetup = OS_FALSE;

        memset(u->domainList, 0, sizeof u->domainList);

        /* This will mark the user-layer initialized */
        user = initUser;
    } else {
        if(user == NULL){
            os_duration sleep = OS_DURATION_INIT(0, 100000); /* 100ms */
            /* Another thread is currently initializing the user-layer. Since
             * user != NULL is a precondition for calls after u_userInitialise(),
             * a sleep is performed, to ensure that (if succeeded) successive
             * user-layer calls will also actually pass.*/
            os_sleep(sleep);
        }

        if(user == NULL){
            /* Initialization did not succeed, undo increment and return error */
            initCount = pa_dec32_nv(&_ospl_userInitCount);
            rm = U_RESULT_INTERNAL_ERROR;
            OS_REPORT(OS_ERROR,"u_userInitialise", rm,
                        "Internal error: User-layer should be initialized "
                        "(initCount = %d), but user == NULL (waited 100ms).",
                        initCount);
        }
    }
    return rm;
}

u_result
u_userAddDomain(
    _In_ u_domain domain)
{
    u_domainAdmin ka;
    u_user u;
    u_result result = U_RESULT_OK;
    os_uint32 index;

    assert(domain != NULL);

    u = u__userLock();
    if(u){
        if (u->domainCount + 1 < MAX_DOMAINS) {
            u->domainCount++;
            for (index = 1;(index < MAX_DOMAINS) && (u->domainList[index].domain != NULL); index++) { }
            ka = &u->domainList[index];
            ka->domain = domain;
            u_domainIdSetThreadSpecific(domain);
        } else {
            /* Isn't it possible that if I have added and removed a domain 128 times, that you
             * have a domainList that is empty, but with a count of max?
             * Couldn't there be an empty spot in the list before concluding that we can't handle
             * more domains?
             */
            result = U_RESULT_OUT_OF_MEMORY;
            OS_REPORT(OS_ERROR,
                    "u_userAddDomain",result,
                    "Max connected Domains (%d) reached!", MAX_DOMAINS - 1);
        }
        u__userUnlock();
    } else {
        result = U_RESULT_PRECONDITION_NOT_MET;
        OS_REPORT(OS_ERROR,
                "u_userAddDomain",result,
                "User layer is (being) destroyed");
    }

    return result;
}

u_result
u_userRemoveDomain(
    _In_ u_domain domain)
{
    u_domainAdmin ka;
    u_user u;
    u_result result = U_RESULT_PRECONDITION_NOT_MET;
    os_uint32 i;

    assert(domain != NULL);

    u = u__userLock();
    if(u){
        ka = NULL;
        for (i = 1; (i < MAX_DOMAINS && ka == NULL); i++) {
            if (u->domainList[i].domain == domain) {
                ka = &u->domainList[i];
                ka->domain = NULL;
                u->domainCount--;
                result = U_RESULT_OK;
            }
        }
        u__userUnlock();

        if (result != U_RESULT_OK) {
            OS_REPORT(OS_ERROR,
                "user::u_user::u_userRemoveDomain", result,
                "Domain to be removed not found in user-layer administration: Unknown Domain = 0x%"PA_PRIxADDR".",
                 (os_address)domain);
        }
    }
    return result;
}

u_domainId_t
u_userGetDomainIdFromEnvUri(void)
{
    os_char *uri = NULL;
    u_domainId_t domainId = U_DOMAIN_ID_DEFAULT;
    cf_element platformConfig = NULL;
    cf_element dc = NULL;
    cf_element elemName = NULL;
    cf_data dataName;
    c_value value;
    cfgprs_status r;

    uri = os_getenv ("OSPL_URI");
    r = cfg_parse_ospl (uri, &platformConfig);
    if (r == CFGPRS_OK)
    {
       dc = cf_element (cf_elementChild (platformConfig, CFG_DOMAIN));
       if (dc) {
          elemName = cf_element(cf_elementChild(dc, CFG_ID));
          if (elemName) {
             dataName = cf_data(cf_elementChild(elemName, "#text"));
             if (dataName != NULL) {
                value = cf_dataValue(dataName);
                sscanf(value.is.String, "%d", &domainId);
             }
          }
       }
       cf_elementFree(platformConfig);
    }
    return domainId;
}

u_domain
u_userLookupDomain(
    const u_domainId_t id)
{
    u_user u;
    u_domain domain;
    u_domainAdmin ka;
    c_long i;

    domain = NULL;

    u = u__userLock();
    if (u) {
        for (i=1; (i<MAX_DOMAINS && domain == NULL); i++) {
            ka = &u->domainList[i];
            if (ka->domain) {
                if (u_domainCompareId(ka->domain,id))
                {
                    u_domain d = ka->domain;
                    os_mutexLock(&d->mutex);
                    if (!d->closing) {
                        d->openCount++;
                        domain = d;
                    }
                    os_mutexUnlock(&d->mutex);
                }
            }
        }
        u__userUnlock();
    } else {
        OS_REPORT(OS_ERROR,
                "u_userLookupDomain", U_RESULT_INTERNAL_ERROR,
                "User layer is (being) destroyed");
    }
    return domain;
}

os_uint32
u__userDomainIndex(
    _In_ u_domain domain)
{
    os_uint32 i, idx = 0;
    u_user u;

    assert(domain != NULL);

    u = u__userLock();
    if (u) {
        for (i = 1; (idx == 0) && (i < MAX_DOMAINS); i++) {
            if (domain == u->domainList[i].domain) {
                idx = i;
            }
        }
        u__userUnlock();
    }

    return idx;
}

c_ulong
u_userServerId(
    const v_public o)
{
    v_kernel kernel;
    c_ulong i, id = 0;
    u_user u;

    assert(o != NULL);

    u = user;
    if ( u ) {
        kernel = v_objectKernel(o);
        for (i=1; i<MAX_DOMAINS; i++) {
            if (u->domainList[i].domain) {
                if (u_domainAddress(u->domainList[i].domain) == (os_address)kernel) {
                    id = i << 24;
                }
            }
        }
    }
    return id;
}

c_address
u_userServer(
    _In_ c_ulong id)
{
    u_domain domain;
    c_ulong idx;
    c_address server;
    u_user u;

    domain = NULL;
    server = 0;

    u = user;
    if ( u ) {
        idx = id >> 24;
        if ((idx > 0) && (idx < MAX_DOMAINS)) {
            domain = u->domainList[idx].domain;
        }
        if (domain) {
            server = u_domainHandleServer(domain);
        }
    }
    return server;
}

os_char *
u_userGetProcessName(void)
{
    os_char *name;
    int size;

#define _INITIAL_LENGTH_ (32)

    name = os_malloc(_INITIAL_LENGTH_);
    size = os_procFigureIdentity(name, _INITIAL_LENGTH_);
    if(size >= _INITIAL_LENGTH_){
        /* Output was truncated, only realloc once, since the identity is
         * not changing. */
        name = os_realloc(name, (unsigned) size + 1);
        size = os_procFigureIdentity(name, (unsigned) size + 1);
    }
    /* No else, since fall-through for second os_procFigureIdentity-call */
    if (size < 0){
        /* An error occurred */
        os_free(name);
        name = NULL;
    }
    return name;
}

