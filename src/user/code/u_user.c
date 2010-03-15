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

#define u_user(u) ((C_STRUCT(u_user) *)(u))

C_CLASS(u_kernelAdmin);
C_STRUCT(u_kernelAdmin) { /* protected by global user lock */
    c_long refCount;
    u_kernel kernel;
    /* The keepList holds all kernel references issues by u_userKeep().
     * This list will keep the objects alive until the objects are removed by
     * u_userFree() or when the kernel is detached from the process
     * e.g. when a process terminates.
     */
    c_iter keepList;
    /* The lower and upper address bounds of the kernels memory segment.
     * These values are used to identify if an object belongs to the kernel.
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
     * The kernelList attribute holds information about all connected kernels.
     * The kernelCount attribute specifies the max index in the kernelList.
     * So any search range on the list can be limited to 0..kernelCount.
     * kernels that are detached are removed from the list and the entry in the
     * list is never used again.
     * The reason why kernel entries are never reused and why kernelCount is not
     * the actual number of kernels connected is unclear, it would be more intuitive,
     * maintainable and flexible if entries in the kernelList could be reused and the
     * kernelCount would reflect the actual number of connected kernels.
     * So this is a subject for future improvement.
     */
    C_STRUCT(u_kernelAdmin) kernelList[MAX_KERNEL];
    c_long kernelCount;
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
        } else if ((u->detachThreadId != 0) &&
                   (u->detachThreadId != os_threadIdSelf()))
        {
            /* Another thread is busy destroying the user-layer or the user-
             * layer is already destroyed. No access is allowed (anymore).
             * The user-layer object will be unlocked and will return null.
             */
            os_mutexUnlock(&u->mutex);
            u = NULL;
        }
    } else {
      /* The user-layer is not created or destroyed i.e. non existent, therefore return null.
       */
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
        if ((u->detachThreadId == 0) ||
            (u->detachThreadId == os_threadIdSelf())) {
            os_mutexUnlock(&u->mutex);
        }
    }
}

static void
u_userExit(void)
{
    u_user u;
    u_kernel kernel;
    os_result mr = os_resultFail;
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

        for (i = 1; (i <= u->kernelCount); i++) {
            kernel = u->kernelList[i].kernel;
            if (kernel) {
                u_kernelDetachParticipants(kernel);
                u_userKernelClose(kernel);
            }
        }

        user = NULL;

        /* Destroy the user-layer mutex */
        mr = os_mutexDestroy(&u->mutex);
        if(mr != os_resultSuccess){
            OS_REPORT_1(OS_WARNING,"u_userExit",0,
                        "User-layer mutex destroy failed: os_result == %d.",
                        mr);
        }

        /* Free the user-object */
        os_free(u);

        /* De-init the OS-abstraction layer */
        os_osExit();
    }
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
    u_kernelAdmin ka;
    c_long i;

    if (o) {
        u = u__userLock();
        if (u) {
            /* the user-layer object exists so now find the kernel that holds
             * the given object.
             */
            for (i=1; i <= u->kernelCount; i++) {
                ka = &u->kernelList[i];
                if (ka->kernel) {
                    /* A valid kernel admin exists, now check if the objects
                     * address is in the kernels address range.
                     */
                    if (((c_address)o >= ka->lowerBound) &&
                        ((c_address)o <= ka->upperBound))
                    {
                        c_keep(o);
                        ka->keepList = c_iterInsert(ka->keepList,o);
                        i = u->kernelCount + 1; /* jump out of the loop */
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
    u_kernelAdmin ka;
    c_object found;
    c_long i;

    if (o) {
        u = u__userLock();
        if (u) {
            for (i=1; i <= u->kernelCount; i++) {
                ka = &u->kernelList[i];
                if (ka->kernel) {
                    if (((c_address)o >= ka->lowerBound) &&
                        ((c_address)o <= ka->upperBound))
                    {
                        /* o is in the address range of this kernel.
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
                        i = u->kernelCount + 1; /* jump out of the loop */
                    }
                }
            }
            u__userUnlock();
        }
    }
}

/* This method is depricated, it is only here untill all
 * usage of this method is re3moved from all other files.
 */
u_result
u_userDetach()
{
    return U_RESULT_OK;
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
    os_sharedHandle shm;

    _this = u_kernelNew(uri);
    if(_this) {
        u = u__userLock();
        if(u){
            if (u->kernelCount + 1 < MAX_KERNEL) {
                shm = u_kernelSharedMemoryHandle(_this);
                u->kernelCount++;
                ka = &u->kernelList[u->kernelCount];
                ka->kernel = _this;
                ka->refCount = 1;
                /* The keepList holds all kernel references issues by u_userKeep().
                 * This list will keep the objects alive until the objects are removed by
                 * u_userFree() or when the kernel is detached from the process
                 * e.g. when a process terminates.
                 */
                ka->keepList = NULL;
                ka->lowerBound = (c_address)os_sharedAddress(shm);
                os_sharedSize(shm, (os_uint32*)&ka->upperBound);
                ka->upperBound += ka->lowerBound;
            } else {
                OS_REPORT_1(OS_ERROR,
                        "u_userKernelNew",0,
                        "Max connected Domains (%d) reached!", MAX_KERNEL - 1);
            }
            u__userUnlock();
        } else {
            OS_REPORT(OS_ERROR,
                    "u_userKernelNew",0,
                    "User layer not initialized");
        }
    } /* Fail already reported by u_kernelNew */

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
    os_sharedHandle shm;
    c_long i, firstFreeIndex;

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
        firstFreeIndex = 0;
        for (i=1; i<=u->kernelCount; i++) {
            if (compareAdminUri(u->kernelList[i].kernel,(void *)uri)) {
                ka = &u->kernelList[i];
                ka->refCount++;
                result = ka->kernel;
            } else if ((firstFreeIndex == 0) && (u->kernelList[i].kernel == NULL)) {
                firstFreeIndex = i;
            }
        }
        if (ka == NULL) {
            if ((firstFreeIndex != 0) || (u->kernelCount + 1 < MAX_KERNEL)) {
                result = u_kernelOpen(uri, timeout);
                if (result != NULL) {
                    shm = u_kernelSharedMemoryHandle(result);
                    if (firstFreeIndex != 0) {
                        ka = &u->kernelList[firstFreeIndex];
                    } else {
                        u->kernelCount++;
                        ka = &u->kernelList[u->kernelCount];
                    }
                    ka->kernel = result;
                    ka->refCount = 1;
                    /* The keepList holds all kernel references issues by u_userKeep().
                     * This list will keep the objects alive until the objects are removed by
                     * u_userFree() or when the kernel is detached from the process
                     * e.g. when a process terminates.
                     */
                    ka->keepList = NULL;
                    ka->lowerBound = (c_address)os_sharedAddress(shm);
                    os_sharedSize(shm, (os_uint32*)&ka->upperBound);
                    ka->upperBound += ka->lowerBound;
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
    c_object object;
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
                    object = c_iterTakeFirst(ka->keepList);
                    while (object) {
                        c_free(object);
                        object = c_iterTakeFirst(ka->keepList);
                    }
                    c_iterFree(ka->keepList);
                    ka->keepList = NULL;
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
    c_long i, id = 0;
    u_user u;

    u = u__userLock();
    if ( u ) {
        kernel = v_objectKernel(o);
        for (i=1; i<=u->kernelCount; i++) {
            if (u_kernelAddress(u->kernelList[i].kernel) == kernel) {
                id = i << 24;
            }
        }
        u__userUnlock();
    } else {
        OS_REPORT(OS_ERROR,
                "u_userServerId",0,
                "User layer not initialized");
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

    kernel = NULL;
    server = 0;
    u = u__userLock();
    if ( u ) {
        idx = id >> 24;
        if ((idx > 0) && (idx <= u->kernelCount)) {
            kernel = u->kernelList[idx].kernel;
        }
        u__userUnlock();
    } else {
        OS_REPORT(OS_ERROR,
                "u_userServer",0,
                "User layer not initialized");
    }
    if (kernel) {
        server = u_kernelHandleServer(kernel);
    }
    return server;
}

