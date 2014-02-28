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
#include "u__domain.h"
#include "u__types.h"
#include "u_entity.h"
#include "cfg_parser.h"
#include "cf_config.h"

#include "os.h"
#include "os_report.h"
#include "os_signalHandler.h"

#define MAX_DOMAINS (128)

#define u_user(u) ((C_STRUCT(u_user) *)(u))

C_CLASS(u_domainAdmin);
C_STRUCT(u_domainAdmin) { /* protected by global user lock */
    u_domain domain;
    /* The keepList holds all domain references issues by u_userKeep().
     * This list will keep the objects alive until the objects are removed by
     * u_userFree() or when the domain is detached from the process
     * e.g. when a process terminates.
     */
    c_iter keepList;
    /* The lower and upper address bounds of the domains memory segment.
     * These values are used to identify if an object belongs to the domain.
     */
    c_address lowerBound;
    c_address upperBound;
};

C_CLASS(u_user);
C_STRUCT(u_user) {
    /* The following mutex implements the global user lock.
     * all access to user info is under control of this lock.
     */
    os_mutex mutex;
    /*
     * The domainList attribute holds information about all connected doamins.
     * The domainCount attribute specifies the max index in the domainList.
     * So any search range on the list can be limited to 1..domainCount.
     * domains that are detached are removed from the list and the entry in the
     * list is never used again.
     * The value 0 is reserved for the 'no domain' use-case,
     * so if domainCount = 0 then no doamins are attached.
     * The reason why domain entries are never reused and why domainCount is not
     * the actual number of doamins connected is unclear, it would be more intuitive,
     * maintainable and flexible if entries in the domainList could be reused and the
     * domainCount would reflect the actual number of connected doamins.
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
};

/** \brief Counter that keeps track of number of times user-layer is initialized.
 *
 * The main purpose of this counter is to ensure that the user-layer is
 * initialized only once.
 */
static os_uint32 _ospl_userInitCount = 0;

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
    if (u) {
        r = os_mutexLock(&u->mutex);
        if (r != os_resultSuccess) {
            /* The mutex is not valid so apparently the user-layer is either
             * destroyed or in process of destruction. */
            u = NULL;
        } else if ((os_threadIdToInteger(u->detachThreadId) != 0) &&
                   (os_threadIdToInteger(u->detachThreadId) !=
                    os_threadIdToInteger(os_threadIdSelf())))
        {
            /* Another thread is busy destroying the user-layer or the user-
             * layer is already destroyed. No access is allowed (anymore).
             * The user-layer object will be unlocked and will return null.
             */
            os_mutexUnlock(&u->mutex);
            u = NULL;
        }
    } else {
        /* The user-layer is not created or destroyed i.e. non existent, therefore return null. */
        OS_REPORT(OS_ERROR, "User Layer", 0, "User layer not initialized");
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
    if (u) {
        if ((os_threadIdToInteger(u->detachThreadId) == 0) ||
            (os_threadIdToInteger(u->detachThreadId) ==
             os_threadIdToInteger(os_threadIdSelf()))) {
            os_mutexUnlock(&u->mutex);
        }
    }
}

void
u_userExit(
    void)
{
    u_user u;
    u_domain domain;
    os_result mr = os_resultFail;
    u_result r;
    c_long i;

    u = u__userLock();
    if (u) {
        /* Disable access to user-layer for all other threads except for this thread.
         * Any following user access from other threads is gracefully
         * aborted.
         */
        u->detachThreadId = os_threadIdSelf();
        /* Unlock the user-layer
         * Part of following code requires to unlock the user object
         * This is allowed now all other threads will abort when
         * trying to claim the lock
         */
        u__userUnlock();

        for (i = 1; (i <= u->domainCount); i++) {
            domain = u->domainList[i].domain;
            if (domain) {
                r = u_domainDetachParticipants(domain);
                if (r != U_RESULT_OK) {
                    OS_REPORT_2(OS_ERROR,
                                "user::u_user::u_userExit", 0,
                                "Operation u_domainDetachParticipants(0x%x) failed."
                                OS_REPORT_NL "result = %s",
                                domain, u_resultImage(r));
                } else {
                    r = u_domainFree(domain);
                    if (r != U_RESULT_OK) {
                        OS_REPORT_2(OS_ERROR,
                                    "user::u_user::u_userExit", 0,
                                    "Operation u_domainFree(0x%x) failed."
                                    OS_REPORT_NL "result = %s",
                                    domain, u_resultImage(r));
                    }
                }
            }
        }
        user = NULL;

        /* Destroy the user-layer mutex */
        mr = os_mutexDestroy(&u->mutex);
        if(mr != os_resultSuccess){
            OS_REPORT_1(OS_ERROR,
                        "user::u_user::u_userExit",0,
                        "Operation os_mutexDestroy(0x%x) failed:"
                        OS_REPORT_NL "os_result == %d.",
                        mr);
        }
        /* Free the user-object */
        os_free(u);
    }

    /* Even if access to the user layer is denied, we still need to cleanup
     * the signal handler, which includes waiting for the threads to exit
     * the DDS database */
    os_signalHandlerFree();

    /* De-init the OS-abstraction layer */
    os_osExit();
}

/* This method registers a database object to be managed by the user-layer.
 * Once a process has registered an object it can free its reference.
 * The user-layer will keep the registered object alive until it is deregistered
 * using the u_userFree method.
 * The user-layer will free all references of registered objects on process
 * termination via the installed exit handler (u_userExit).
 */
c_object
u_userKeep(
    c_object o)
{
    u_user u;
    u_domainAdmin ka;
    c_long i;

    if (o) {
        u = u__userLock();
        if (u) {
            /* the user-layer object exists so now find the domain that holds
             * the given object.
             */
            for (i=1; i <= u->domainCount; i++) {
                ka = &u->domainList[i];
                if (ka->domain) {
                    /* A valid domain admin exists, now check if the objects
                     * address is in the domains address range.
                     */
                    if (((c_address)o >= ka->lowerBound) &&
                        ((c_address)o <= ka->upperBound))
                    {
                        c_keep(o);
                        ka->keepList = c_iterInsert(ka->keepList,o);
                        i = u->domainCount + 1; /* jump out of the loop */
                    }
                }
            }
            u__userUnlock();
        }
    }
    return o;
}

void
u_userFree (
    c_object o)
{
    u_user u;
    u_domainAdmin ka;
    c_object found;
    c_long i;

    if (o) {
        u = u__userLock();
        if (u) {
            for (i=1; i <= u->domainCount; i++) {
                ka = &u->domainList[i];
                if (ka->domain) {
                    if (((c_address)o >= ka->lowerBound) &&
                        ((c_address)o <= ka->upperBound))
                    {
                        /* o is in the address range of this domain.
                         * so take it from the keepList.
                         * and free it only if it is actually found in the keepList.
                         */
                        found = c_iterTake(ka->keepList,o);
                        if (found) {
                            c_free(found);
                        } else {
                            OS_REPORT_1(OS_WARNING,"u_userFree",0,
                                       "User tries to free non existing object == 0x%x.",
                                        found);
                        }
                        i = u->domainCount + 1; /* jump out of the loop */
                    }
                }
            }
            u__userUnlock();
        }
    }
}

/* This method is depricated, it is only here untill all
 * usage of this method is removed from all other files.
 */
u_result
u_userDetach()
{
    return U_RESULT_OK;
}

static void
u__userDetach(
    void)
{
    u_user u;
    u_domain domain;
    u_result r;
    c_long i;

    u = u__userLock();
    if (u) {
        /* Disable access to user-layer for all other threads except for this thread.
         * Any following user access from other threads is gracefully
         * aborted.
         */
        u->detachThreadId = os_threadIdSelf();
        /* Unlock the user-layer
         * Part of following code requires to unlock the user object
         * This is allowed now all other threads will abort when
         * trying to claim the lock
         */
        u__userUnlock();

        for (i = 1; (i <= u->domainCount); i++) {
            domain = u->domainList[i].domain;
            if (domain) {
                r = u_domainDetachParticipants(domain);
                if (r != U_RESULT_OK) {
                    OS_REPORT_2(OS_ERROR,
                                "user::u_user::u_userDetach", 0,
                                "Operation u_domainDetachParticipants(0x%x) failed."
                                OS_REPORT_NL "result = %s",
                                domain, u_resultImage(r));
                } else {
                    r = u_domainFree(domain);
                    if (r != U_RESULT_OK) {
                        OS_REPORT_2(OS_ERROR,
                                    "user::u_user::u_userDetach", 0,
                                    "Operation u_domainFree(0x%x) failed."
                                    OS_REPORT_NL "result = %s",
                                    domain, u_resultImage(r));
                    }
                }
            }
        }
        /*        user = NULL;
         * ES: This was set to NULL here by RP, but this causes  errors later on
         * when u_userExit is performed. So commented this out here. As I can
         * not explain why we would need to set it to NULL here.
         */

    }
}

static os_result
u__userExceptionCallbackWrapper(void)
{
    /* do not detach when using single process as this does not function correctly for single process*/
    if(!os_serviceGetSingleProcess()) {
        OS_REPORT(OS_ERROR, "u__userExceptionCallbackWrapper", 0,
                "Exception occurred, will detach user-layer from domain.");
        u__userDetach();
    }
    return os_resultSuccess;
}

static os_result
u__userExitRequestCallbackWrapper(
    os_callbackArg arg)
{
    /* do _exit when using single process as the exit request handler does not function correctly for single process*/
    if(!os_serviceGetSingleProcess()) {
        OS_REPORT(OS_WARNING, "u__userExitRequestCallbackWrapper", 0,
                  "Received termination request, will detach user-layer from domain.");
        u__userDetach();
    } else {
#ifdef _WRS_KERNEL
        exit(0);
#else
        _exit(0);
#endif
    }
    return os_signalHandlerFinishExitRequest(arg);
}

u_result
u_userInitialise(
    void)
{
    u_user u;
    u_result rm = U_RESULT_OK;
    os_mutexAttr mutexAttr;
    os_uint32 initCount;
    void* initUser;
    os_result osResult;
    os_signalHandlerExitRequestCallback exitRequestCallback;
    os_signalHandlerExceptionCallback exceptionCallback;

    initCount = pa_increment(&_ospl_userInitCount);
    /* If initCount == 0 then an overflow has occurred.
     * This can only realistically happen when u_userDetach()
     * is called more often than u_userInitialize().
     */
    assert(initCount != 0);

    os_osInit();
    if (initCount == 1) {
        /* Will start allocating the object, so it should currently be empty. */
        assert(user == NULL);

        /* Use indirection, as user != NULL is a precondition for user-layer
         * functions, so make sure it only holds true when the user-layer is
         * initialized. */
        initUser = os_malloc(sizeof(C_STRUCT(u_user)));
        if (initUser == NULL) {
            /* Initialization failed, so decrement the initialization counter. */
            pa_decrement(&_ospl_userInitCount);
            os_osExit();
            OS_REPORT(OS_ERROR, "u_userInitialise", 0,
                      "Allocation of user admin failed: out of memory.");
            rm = U_RESULT_OUT_OF_MEMORY;
        } else {
            u = u_user(initUser);
            os_mutexAttrInit(&mutexAttr);
            mutexAttr.scopeAttr = OS_SCOPE_PRIVATE;
            os_mutexInit(&u->mutex,&mutexAttr);
            osResult = os_signalHandlerNew();
            if(osResult != os_resultSuccess)
            {
                /* Initialization did not succeed, undo increment and return error */
                initCount = pa_decrement(&_ospl_userInitCount);
                OS_REPORT(OS_ERROR, "u_userInitialise", 0,
                      "Failed to create the signal handler. No proper signal handling can be performed.");
                rm = U_RESULT_INTERNAL_ERROR;
            } else
            {
                exitRequestCallback = os_signalHandlerSetExitRequestCallback(u__userExitRequestCallbackWrapper);
                if(exitRequestCallback && exitRequestCallback != u__userExitRequestCallbackWrapper)
                {
                    initCount = pa_decrement(&_ospl_userInitCount);
                    OS_REPORT(OS_ERROR, "u_userInitialise", 0,
                        "Replaced an exit request callback on the signal handler while this was not expected.");
                    rm = U_RESULT_INTERNAL_ERROR;
                }
                if(rm == U_RESULT_OK){
                    exceptionCallback = os_signalHandlerSetExceptionCallback(u__userExceptionCallbackWrapper);
                    if(exceptionCallback && exceptionCallback != u__userExceptionCallbackWrapper)
                    {
                        initCount = pa_decrement(&_ospl_userInitCount);
                        OS_REPORT(OS_ERROR, "u_userInitialise", 0,
                            "Replaced an exception callback on the signal handler while this was not expected.");
                        rm = U_RESULT_INTERNAL_ERROR;
                    }
                }
                if(rm == U_RESULT_OK)
                {
                    u->domainCount = 0;
                    u->protectCount = 0;
                    u->detachThreadId = OS_THREAD_ID_NONE;

                    /* This will mark the user-layer initialized */
                    user = initUser;
                }
            }
        }
    } else {
        if(user == NULL){
            os_time sleep = {0, 100000}; /* 100ms */
            /* Another thread is currently initializing the user-layer. Since
             * user != NULL is a precondition for calls after u_userInitialise(),
             * a sleep is performed, to ensure that (if succeeded) successive
             * user-layer calls will also actually pass.*/
            os_nanoSleep(sleep);
        }

        if(user == NULL){
            /* Initialization did not succeed, undo increment and return error */
            initCount = pa_decrement(&_ospl_userInitCount);
            OS_REPORT_1(OS_ERROR,"u_userInitialise",0,
                        "Internal error: User-layer should be initialized "
                        "(initCount = %d), but user == NULL (waited 100ms).",
                        initCount);
            rm = U_RESULT_INTERNAL_ERROR;
        }
    }
    return rm;
}

u_result
u_userAddDomain(
    u_domain domain)
{
    u_domainAdmin ka;
    u_user u;
    u_result result;
    os_sharedHandle shm;
    os_result osr;

    if(domain) {
        u = u__userLock();
        if(u){
            if (u->domainCount + 1 < MAX_DOMAINS) {
                shm = u_domainSharedMemoryHandle(domain);
                u->domainCount++;
                ka = &u->domainList[u->domainCount];
                ka->domain = domain;
                /* The keepList holds all domain references issues by
                 * u_userKeep().
                 * This list will keep the objects alive until the objects
                 * are removed by u_userFree() or when the domain is detached
                 * from the process e.g. when a process terminates.
                 */
                ka->keepList = NULL;
                ka->lowerBound = (c_address)os_sharedAddress(shm);
                osr = os_sharedSize(shm, (os_address*)&ka->upperBound);
                if (osr != os_resultSuccess) {
                    OS_REPORT(OS_ERROR,
                            "u_userAddDomain",0,
                            "shared memory size cannot be determined");
                    result = U_RESULT_INTERNAL_ERROR;
                } else {
                    result = U_RESULT_OK;
                }
                ka->upperBound += ka->lowerBound;
            } else {
                OS_REPORT_1(OS_ERROR,
                        "u_userAddDomain",0,
                        "Max connected Domains (%d) reached!", MAX_DOMAINS - 1);
                result = U_RESULT_OUT_OF_MEMORY;
            }
            u__userUnlock();
        } else {
            OS_REPORT(OS_ERROR,
                    "u_userAddDomain",0,
                    "User layer not initialized");
            result = U_RESULT_PRECONDITION_NOT_MET;
        }
    } else {
        OS_REPORT(OS_ERROR,
                  "u_userAddDomain",0,
                  "Invalid Domain specified: Domain = NULL");
        result = U_RESULT_ILL_PARAM;
    } /* Fail already reported by u_domainNew */

    return result;
}

c_long
u_userRemoveDomain(
    u_domain domain)
{
    u_domainAdmin ka;
    u_user u;
    c_long result = -1;
    c_long i;

    if (domain == NULL) {
        OS_REPORT(OS_ERROR,
                  "user::u_user::u_userRemoveDomain", 0,
                  "Illegal parameter: Domain = NULL.");
        return result;
    }

    u = u__userLock();
    if(u){
        ka = NULL;
        for (i=1; (i<=u->domainCount && ka == NULL); i++) {
            if (u->domainList[i].domain == domain) {
                ka = &u->domainList[i];
                result = 0;
                /* Prevent it from being opened again, caller has a reference
                 * and will have to free it through that */
                ka->domain = NULL;
            }
        }
        u__userUnlock();

        if (result < 0) {
            OS_REPORT_1(OS_ERROR,
                "user::u_user::u_userRemoveDomain", 0,
                "Domain to be removed not found in user-layer administration: Unknown Domain = 0x%x.",
                 domain);
        }
    }
    return result;
}

int
u_userGetDomainIdFromEnvUri() {
    char *uri = NULL;
    int domainId = 0;
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
    }
    return domainId;

}

char *
u_userGetDomainNameFromEnvUri() {
    char *uri = NULL;
    char * domainName = NULL;
    cf_element platformConfig = NULL;
    cf_element dc = NULL;
    cf_element elemName = NULL;
    cf_data dataName;
    c_value value;
    cfgprs_status r;

    uri = os_getenv ("OSPL_URI");
    if ( uri != NULL )
    {
       domainName = os_strdup(uri);
    }
    else
    {
       r = cfg_parse_ospl (uri, &platformConfig);
       if (r == CFGPRS_OK)
       {
          dc = cf_element (cf_elementChild (platformConfig, CFG_DOMAIN));
          if (dc)
          {
             elemName = cf_element(cf_elementChild(dc, CFG_NAME));
             if (elemName)
             {
                dataName = cf_data(cf_elementChild(elemName, "#text"));
                if (dataName != NULL)
                {
                   value = cf_dataValue(dataName);
                   domainName = os_strdup(value.is.String);
                }
             }
          }
       }
    }
    return domainName;
}

c_bool
u_userGetSPBFromEnvUri() {
    char *uri = NULL;
    c_bool spb = FALSE;
    cf_element platformConfig = NULL;
    cf_element dc = NULL;
    cf_element elemName = NULL;
    cf_element singleProcess = NULL;
    cf_data elementData = NULL;
    cf_data dataName;
    c_value value;
    cfgprs_status r;

    uri = os_getenv ("OSPL_URI");
    r = cfg_parse_ospl (uri, &platformConfig);
    if (r == CFGPRS_OK)
    {
       dc = cf_element (cf_elementChild (platformConfig, CFG_DOMAIN));
       if (dc) {
      singleProcess = cf_element(cf_elementChild(dc, CFG_SINGLEPROCESS));
      if (singleProcess != NULL) {
         elementData = cf_data(cf_elementChild(singleProcess, "#text"));
         if (elementData != NULL) {
            value = cf_dataValue(elementData);
        if (os_strncasecmp(value.is.String, "TRUE", 4) == 0) {
           /* A SingleProcess value of True implies that Heap is to be used */
          spb = TRUE;
        }
         }
      }
       }
    }
    return spb;

}


/*
 * get the domain name for a given domain id
 * first check the user layer administration
 * if no name is found check the key files for a match
 *
 */
c_char *
u_userDomainIdToDomainName(
    os_int32 id)
{
    c_char *name = NULL;
    os_sharedAttr    shm_attr;
    os_sharedHandle  shm = NULL;
    u_user u;
    u_domainAdmin ka;
    c_long i;
    u = u__userLock();
    /* user does not know the id find it by looking into the uri */
    if (id == MAX_DOMAIN_ID) {
        id = u_userGetDomainIdFromEnvUri();
    }
    if (u) {
        /* the user-layer object exists so now find the domain that holds
         * the given id.
         */
        for (i=1; i <= u->domainCount; i++) {
            ka = &u->domainList[i];
            if (ka && ka->domain && ka->domain->id == id) {
               /* found the domain with the id now get the name */
                if (ka->domain->name) {
                    name = os_strdup(ka->domain->name);
                }
            }
        }
        u__userUnlock();
    }
    if (name == NULL) {
        os_sharedAttrInit(&shm_attr);
        if (u_userGetSPBFromEnvUri()) {
            /* HACK!!! just return the OSPL_URI as name
             * a good solution has to be made for the single process build to match
             * the id and domainName through the os_sharedMemoryGetNameFromId interface.
             */
            if (id == u_userGetDomainIdFromEnvUri()) {
           name = u_userGetDomainNameFromEnvUri();
            }
        } else {

            shm = os_sharedCreateHandle(DOMAIN_NAME, &shm_attr, id);

            if (shm == NULL) {
               OS_REPORT(OS_ERROR,
                         "user::u_domain::u_userDomainIdToDomainName",0,
                         "c_open failed; shared memory open failure!");
            } else {
                /* if this fails name will be NULL */
                os_sharedMemoryGetNameFromId(shm,&name);
                os_sharedDestroyHandle(shm);
        /* As for SPB if we still haven't got the name, just use the URI */
        if (name == NULL && id == u_userGetDomainIdFromEnvUri()) {
            name = u_userGetDomainNameFromEnvUri();
        }
            }
        }
    }
    return name;
}

u_domain
u_userLookupDomain(
    const c_char *uri)
{
    u_user u;
    u_domain domain;
    u_domainAdmin ka;
    c_long i;

    domain = NULL;

    u = u__userLock();
    if (u) {
        if (uri == NULL || strlen (uri) == 0) {
            uri = os_getenv ("OSPL_URI");
            if (uri == NULL) {
                uri = "";
            }
        }
        /* If the domain is already opened by the process,
           return the domain object.
           Otherwise open the domain and add to the administration */
        for (i=1; (i<=u->domainCount && domain == NULL); i++) {
            ka = &u->domainList[i];
            if (u_domainCompareDomainId(ka->domain,(void *)uri))
            {
                domain = ka->domain;
            }
        }
        u__userUnlock();
    } else {
        OS_REPORT(OS_ERROR,
                "u_userLookupDomain",0,
                "User layer not initialized");
    }
    return domain;
}

c_long
u_userServerId(
    v_public o)
{
    v_kernel kernel;
    c_long i, id = 0;
    u_user u;

    u = user;// u__userLock();
    if ( u ) {
        kernel = v_objectKernel(o);
        for (i=1; i<=u->domainCount; i++) {
            if (u_domainAddress(u->domainList[i].domain) == kernel) {
                id = i << 24;
            }
        }
        //u__userUnlock();
    }
    return id;
}

c_address
u_userServer(
    c_long id)
{
    u_domain domain;
    c_long idx;
    c_address server;
    u_user u;

    domain = NULL;
    server = 0;

    u = user;//u__userLock();
    if ( u ) {
        idx = id >> 24;
        if ((idx > 0) && (idx <= u->domainCount)) {
            domain = u->domainList[idx].domain;
        }
        //u__userUnlock();
        if (domain) {
            server = u_domainHandleServer(domain);
        }
    }
    return server;
}

