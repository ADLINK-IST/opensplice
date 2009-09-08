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
#include "u__user.h"
#include "u__kernel.h"
#include "u_entity.h"

#include "os.h"
#include "os_report.h"

#define MAX_KERNEL (128)

C_CLASS(u_kernelAdmin);
C_STRUCT(u_kernelAdmin) { /* protected by global user lock */
    c_long refCount;
    u_kernel kernel;
};

#define u_user(u) ((C_STRUCT(u_user) *)(u))

#define u__userUnlock()     u__unlock(u_user(user))

C_CLASS(u_user);
C_STRUCT(u_user) {
    os_mutex                mutex;
    C_STRUCT(u_kernelAdmin) kernelList[MAX_KERNEL];
    c_long                  kernelCount;
    /* Should only be modified by pa_(in/de)crement! */
    os_uint32               protectCount;
    /* The detachThreadId will have to be set by the detaching thread while
     * holding the user->mutex and be re-set to NULL before releasing the
     * lock. A lock-boundary should thus never be crossed with detachThreadId
     * set to a different value than when the lock was acquired! */
    os_threadId             detachThreadId;
};

/** \brief Detached the user-layer (optionally forced).
 * Normally the detach will be call-counted paired with the user initialize.
 * However, on process-exit, proper cleanup should always be performed. If force
 * is set to TRUE, the detach will be executed, no matter what the reference-
 * count is. Otherwise the detach will do proper call-counting.
 */
static u_result                 u__userDetach(c_bool force);

/** \brief Counter that keeps track of number of times user-layer is initialized */
static os_uint32                _ospl_userInitCount = 0;

/** \brief Reference to heap user-layer admin object.  */
void *                          user = NULL;


static void
u_userExit(void)
{
    os_uint32 initCount;

    initCount = _ospl_userInitCount;

    if(initCount > 0){
        OS_REPORT_2(OS_WARNING, "u_userExit", 0,
              "Forcing user-layer detach while still referenced %d time%s.",
              initCount,
              (initCount > 1) ? "s" : "");
        u__userDetach(TRUE);
    }
}

static u_user
u__userLock(void)
{
    u_user u;
    os_result r = os_resultFail;
    c_bool detaching = FALSE;

    u = u_user(user);
    if (u) {
        if(u->detachThreadId == 0){
            r = os_mutexLock(&u->mutex);
        } else if (u->detachThreadId == os_threadIdSelf()){
            detaching = TRUE;
        }
        /* While waiting on the mutex, a detach can have invalidated the
         * user-admin, so if the reference is still valid, then all went OK,
         * otherwise the lock should be released again and FALSE returned. */
        if(r == os_resultSuccess || detaching){
            if (!user){
                if(r == os_resultSuccess){
                    /* Lock acquired, but user-layer object invalidated, release lock */
                    os_mutexUnlock(&u->mutex);
                }
                /* User-layer object invalidated, return NULL */
                u = NULL;
            }
        } else {
            /* Could not acquire lock, or user-layer is detaching, return NULL */
            u = NULL;
        }

    } /* Else the user-layer is not available, so return NULL */
    return u;
}

static void
u__unlock(u_user u)
{
    if (u != NULL) {
        /* Following check is valid, because while locked, the detachThreadId
         * may not be modified, so iff the condition passed in u__userLock() ,
         * then it will pass too here.
         * Of course the precondition is that the detachThreadId should never be
         * modified across lock boundaries! */
        if(u->detachThreadId == 0){
            os_mutexUnlock(&u->mutex);
        }
    }
}

static u_result
u__userDetach(c_bool force)
{
    u_user u;
    u_result r = U_RESULT_OK;
    os_result mr = os_resultFail;
    u_kernel kernel;
    os_uint32 initCount;
    c_long i;

    initCount = pa_decrement(&_ospl_userInitCount);

    if(initCount == 0 || force){
        u = u__userLock();
        if (u) {
            /* Ever only one thread detaching the user-layer */
            assert(u->detachThreadId == 0);
            u->detachThreadId = os_threadIdSelf();
            for (i = 1; (i <= u->kernelCount); i++) {
                kernel = u->kernelList[i].kernel;
                if (kernel) {
                    u_kernelDetachParticipants(kernel);
                    u_userKernelClose(kernel);
                }
            }

            /* Disable access to user-layer. */
            user = NULL;

            u->detachThreadId = 0;

            /* Unlock the user-layer */
            u__unlock(u);

            /* Destroy the user-layer mutex */
            mr = os_mutexDestroy(&u->mutex);
            if(mr != os_resultSuccess){
                OS_REPORT_1(OS_WARNING,"u_userDetach",0,
                        "User-layer mutex destroy failed: os_result == %d.",
                        mr);
            }

            /* Free the user-object */
            os_free(u);

            /* De-init the OS-abstraction layer */
            os_osExit();
        } else {
            OS_REPORT_2(OS_WARNING,"u_userDetach",0,
                  "User-layer should be initialized (initCount was %d), but user == NULL%s.",
                  initCount + 1,
                  force ? " (forced detach)" : "");

        }
    } else if ((initCount + 1) < initCount){
        /* The 0 boundary is passed, so u_userDetach() is called more often than
         * u_userInitialise(). Therefore undo decrement as nothing happened and
         * warn. */
        initCount = pa_increment(&_ospl_userInitCount);
        OS_REPORT(OS_WARNING, "u_userDetach", 1, "User layer not initialized");
        r = U_RESULT_INTERNAL_ERROR;
    }

    return r;
}

u_result
u_userDetach()
{
    return u__userDetach(FALSE);
}

u_result
u_userInitialise()
{
    u_user u;
    u_result rm = U_RESULT_OK;
    os_mutexAttr mutexAttr;
    os_uint32 initCount;
    void* initUser;

    initCount = pa_increment(&_ospl_userInitCount);
    /* Iff 0 after increment, overflow has occurred. This can only realistically
     * happen when u_userDetach() is called more often than u_userInitialize(). */
    assert(initCount != 0);

    if (initCount == 1) {

        /* Will start allocating the object, so it should currently be empty. */
        assert(user == NULL);
        os_osInit();

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
            u->kernelCount = 0;
            u->protectCount = 0;
            u->detachThreadId = 0;
            os_procAtExit(u_userExit);

            /* This will mark the user-layer initialized */
            user = initUser;
        }
    } else {
        OS_REPORT_1(OS_INFO, "u_userInitialise", 1,
                "User-layer initialization called %d times", initCount);
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
                      "Internal error: User-layer should be initialized (initCount = %d), but user == NULL (waited 100ms).", initCount);
            rm = U_RESULT_INTERNAL_ERROR;
        }
    }
    return rm;
}

u_result
u_userProtect(
    u_entity e)
{
    u_user u;
    u_result r;
    os_result osr;

    /* While theoretically not necessary to have a lock on the user-object for
     * this call, it is an easy way to assure that the protect only succeeds
     * if the user-layer is active. */
    u = u__userLock();
    if( u ){
        osr = os_threadProtect();
        if (osr == os_resultSuccess) {
            pa_increment(&u->protectCount);
            r = U_RESULT_OK;
        } else {
            r = U_RESULT_INTERNAL_ERROR;
        }

        u__userUnlock();
    } else {
        OS_REPORT(OS_ERROR,
                "u_userProtect",0,
                "User layer not initialized.");
        r = U_RESULT_NOT_INITIALISED;
    }
    return r;
}

u_result
u_userUnprotect(
    u_entity e)
{
    u_user u;
    u_result r;
    os_result osr;
    os_uint32 newCount; /* Only used for checking 0-boundary */

    /* The unprotect is not locked on user->mutex on purpose. No unsafe concur-
     * rent code is executed and this way, threads in user-protected space can
     * always leave protected area, even when others are having a lock (i.e.
     * in case of a u_userDetach()). */
    if(user){
        u = u_user(user);
        osr = os_threadUnprotect();
        if (osr == os_resultSuccess) {
            newCount = pa_decrement(&u->protectCount);
            /* Detect passing of 0 boundary (more likely here than with increment) */
            assert(newCount + 1 > newCount);
            r = U_RESULT_OK;
        } else {
            assert(osr == os_resultSuccess);
            r = U_RESULT_INTERNAL_ERROR;
        }
    } else {
        OS_REPORT(OS_ERROR,
                  "u_userUnprotect",0,
                  "User layer not initialized.");
        r = U_RESULT_NOT_INITIALISED;
    }
    return r;
}

c_long
u_userProtectCount()
{
    u_user u;
    c_long count;

    u = u__userLock();
    if ( u ){
        count = u->protectCount;

        u__userUnlock();
    } else {
        count = 0;
    }

    return count;
}


static c_bool
compareAdminUri(
    c_object o,
    c_iterActionArg arg)
{
    u_kernel kernel = (u_kernel)o;
    const c_char *uri = (const c_char *)arg;
    const c_char *kernelUri;
    u_result result;

    if (kernel != NULL) {
        kernelUri = u_kernelUri(kernel);
        if (uri == NULL) {
            if (kernelUri != NULL) {
                uri = "";
                result = (strcmp(kernelUri, uri) == 0);
            } else {
                result = TRUE;
            }
        } else {
            if (kernelUri == NULL) {
                kernelUri = "";
            }
            result = (strcmp(kernelUri, uri) == 0);
        }
    } else {
        result = FALSE;
    }
    return result;
}

u_kernel
u_userKernelNew(
    const c_char *uri)
{
    u_kernel _this = NULL;
    u_kernelAdmin ka;
    u_user u;

    u = u_user(user);

    if(u){
        if (u->kernelCount + 1 < MAX_KERNEL) {
            _this = u_kernelNew(uri);
            if (_this != NULL) {
                u->kernelCount++;
                ka = &u->kernelList[u->kernelCount];
                ka->kernel = _this;
                ka->refCount = 1;
            }
        } else {
            OS_REPORT_1(OS_ERROR,
                    "u_userKernelNew",0,
                    "Max connected Domains (%d) reached!", MAX_KERNEL - 1);
        }
    } else {
        OS_REPORT(OS_ERROR,
                "u_userKernelNew",0,
                "User layer not initialized");
    }

    return _this;
}

/* timeout -1 identifies probe where no error
 * report is expected during normal flow
 */
u_kernel
u_userKernelOpen(
    const c_char *uri,
    c_long timeout)
{
    u_user u;
    u_kernel result;
    u_kernelAdmin ka;
    c_long i;

    result = NULL;

    if (uri == NULL || strlen (uri) == 0) {
        uri = os_getenv ("OSPL_URI");
    }

    u = u__userLock();
    if (u) {
        /* If the kernel is already opened by the process,
           return the kernel object.
           Otherwise open the kernel and add to the administration */
        ka = NULL;
        for (i=1; i<=u->kernelCount; i++) {
            if (compareAdminUri(u->kernelList[i].kernel,(void *)uri)) {
                ka = &u->kernelList[i];
                ka->refCount++;
                result = ka->kernel;
            }
        }
        if (ka == NULL) {
            if (u->kernelCount + 1 < MAX_KERNEL) {
                result = u_kernelOpen(uri, timeout);
                if (result != NULL) {
                    u->kernelCount++;
                    ka = &u->kernelList[u->kernelCount];
                    ka->kernel = result;
                    ka->refCount = 1;
                } else {
                    /* If timeout = -1, don't report */
                    if (timeout >= 0) {
                        if (uri == NULL) {
                            OS_REPORT(OS_ERROR,
                                      "u_userKernelOpen",0,
                                      "Failed to open: The default domain");
                        } else {
                            OS_REPORT_1(OS_ERROR,
                                        "u_userKernelOpen",0,
                                        "Failed to open: %s",uri);
                        }
                    }
                }
            } else {
                OS_REPORT_1(OS_ERROR,
                          "u_userKernelOpen",0,
                          "Max connected Domains (%d) reached!", MAX_KERNEL - 1);
            }
        }

        u__userUnlock();
    } else {
        OS_REPORT(OS_ERROR,
                "u_userKernelOpen",0,
                "User layer not initialized");
    }

    return result;
}

u_result
u_userKernelClose(
    u_kernel kernel)
{
    u_kernelAdmin ka;
    u_user u;
    u_result r;
    c_long i;
    c_bool detach = FALSE;

    u = u__userLock();
    if ( u ){
        r = U_RESULT_ILL_PARAM;
        for (i=1; (i<=u->kernelCount) && (r != U_RESULT_OK); i++) {
            ka = &u->kernelList[i];
            if (ka->kernel == kernel) {
                ka->refCount--;
                if (ka->refCount == 0) {
                    u->kernelList[i].kernel = NULL;
                    detach = TRUE;
                }
                r = U_RESULT_OK;
            }
        }
        u__userUnlock();

        if (detach) {
            u_kernelClose(kernel);
        }
    } else {
        OS_REPORT(OS_ERROR,
                "u_userKernelClose",0,
                "User layer not initialized");
        r = U_RESULT_NOT_INITIALISED;
    }
    return r;
}

u_result
u_userKernelFree(
    u_kernel kernel)
{
    u_kernelAdmin ka;
    u_user u;
    u_result r;
    c_long i;
    c_bool free = FALSE;

    u = u__userLock();
    if ( u ){
        r = U_RESULT_ILL_PARAM;
        for (i=1; (i<=u->kernelCount) && (r != U_RESULT_OK); i++) {
            ka = &u->kernelList[i];
            if (ka->kernel == kernel) {
                u->kernelList[i].kernel = NULL;
                ka->refCount--;
                assert(ka->refCount == 0);
                if (ka->refCount != 0) {
                    OS_REPORT_1(OS_ERROR,
                        "u_userKernelFree",0,
                        "Kernel being freed is still referenced %d time(s) in user-layer, should be 0",
                        ka->refCount);
                }
                /* Always free if found */
                free = TRUE;
                r = U_RESULT_OK;
            }
        }
        u__userUnlock();

        if (free) {
            u_kernelFree(kernel);
        }
    } else {
        OS_REPORT(OS_ERROR,
                "u_userKernelFree",0,
                "User layer not initialized");
        r = U_RESULT_NOT_INITIALISED;
    }
    return r;
}

c_long
u_userServerId(
    v_public o)
{
    v_kernel kernel;
    c_long i, id;
    u_user u;

    u = u_user(user);
    if (u != NULL) {
        kernel = v_objectKernel(o);
        id = 0;
        for (i=1; i<=u->kernelCount; i++) {
            if (u_kernelAddress(u->kernelList[i].kernel) == kernel) {
                id = i << 24;
            }
        }
    }
    return id;
}

c_address
u_userServer(
    c_long id)
{
    u_kernel kernel;
    c_long idx;
    c_address server;
    u_user u;

    u = u_user(user);
    kernel = NULL;
    server = 0;
    if (u != NULL) {
        idx = id >> 24;
        if ((idx > 0) && (idx <= u->kernelCount)) {
            kernel = u->kernelList[idx].kernel;
        }
    }
    if (kernel) {
        server = u_kernelHandleServer(kernel);
    }
    return server;
}

