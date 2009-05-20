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


typedef enum u_userState {
    U_USERSTATE_UNDEFINED,
    U_USERSTATE_ACTIVE,
    U_USERSTATE_DETACHING,
    U_USERSTATE_TERMINATING,
    U_USERSTATE_INACTIVE
} u_userState;

C_CLASS(u_kernelAdmin);
C_STRUCT(u_kernelAdmin) { /* protected by global user lock */
    c_long refCount;
    u_kernel kernel;
};

#define u_user(u) ((C_STRUCT(u_user) *)(u))
C_CLASS(u_user);
C_STRUCT(u_user) {
    os_mutex                mutex;
    c_iter                  kernelList;
    c_long                  protectCount;
    u_userState             state;
    os_threadId             signalThread;
    os_threadId             detachThreadId; /* set when state is detaching! */
};

void *user = NULL;


static void
u_userExit(void)
{
    u_result r;
    u_user u;

    u = u_user(user);
    if (u != NULL) {
        os_mutexLock(&u->mutex);
        switch (u_user(user)->state) {
        case U_USERSTATE_ACTIVE:
        case U_USERSTATE_DETACHING:
        case U_USERSTATE_TERMINATING:
            os_mutexUnlock(&u->mutex);
            r = u_userDetach();
        break;
        default:
            os_mutexUnlock(&u->mutex);
        break;
        }
    } else {
        OS_REPORT(OS_WARNING,"u_userDetach",0,
                  "User layer not initialised.");
    }
}

u_result
u_userDetach()
{
    u_user u;
    u_kernelAdmin ka;
    u_result r = U_RESULT_OK;
    c_iter kernelList;

    if (user != NULL) {
        u = u_user(user);
        os_mutexLock(&u->mutex);
        switch (u->state) {
        case U_USERSTATE_ACTIVE:
            u->state = U_USERSTATE_DETACHING;
            u->detachThreadId = os_threadIdSelf();
            kernelList = u->kernelList;
            u->kernelList = NULL;
            os_mutexUnlock(&u->mutex);
            while ((ka = c_iterTakeFirst(kernelList)) != NULL) {
                u_kernelDetachParticipants(ka->kernel);
                u_kernelClose(ka->kernel);
                os_free(ka);
            }
            c_iterFree(kernelList);
            os_mutexLock(&u->mutex);

            u->state = U_USERSTATE_INACTIVE;
            u->detachThreadId = 0;
        break;
        case U_USERSTATE_DETACHING:
        case U_USERSTATE_TERMINATING:
        break;
        default:
            OS_REPORT(OS_ERROR,"u_userDetach",0,
                      "Internal error: Illegal User layer state detected.");
            r = U_RESULT_INTERNAL_ERROR;
        break;
        }
        os_mutexUnlock(&u->mutex);
    } else {
        OS_REPORT(OS_WARNING,"u_userDetach",0,
                  "User layer not initialised.");
    }
    return r;
}

u_result
u_userInitialise()
{
    u_user u;
    u_result rm = U_RESULT_OK;
    os_mutexAttr mutexAttr;

    if (user == NULL) {
        os_osInit();
        user = os_malloc(sizeof(C_STRUCT(u_user)));
        if (user == NULL) {
            OS_REPORT(OS_ERROR, "u_userInitialise", 0,
                      "Allocation user admin failed: out of memory.");
            rm = U_RESULT_OUT_OF_MEMORY;
        }

        u = u_user(user);
        os_mutexAttrInit(&mutexAttr);
        mutexAttr.scopeAttr = OS_SCOPE_PRIVATE;
        os_mutexInit(&u->mutex,&mutexAttr);
        u->kernelList = c_iterNew(NULL);
        u->protectCount = 0;
        u->detachThreadId = 0;
        u->state = U_USERSTATE_ACTIVE;
        os_procAtExit(u_userExit);
    } else {
        u = u_user(user);
    }
    if (rm == U_RESULT_OK) {
        os_mutexLock(&u->mutex);
        switch (u->state) {
        case U_USERSTATE_DETACHING:
            rm = u_userInitialise();
        break;
        case U_USERSTATE_TERMINATING:
            OS_REPORT(OS_ERROR,"u_userInitialise",0,
                      "Internal error: Try to protect terminating process.");
            rm = U_RESULT_INTERNAL_ERROR;
        break;
        case U_USERSTATE_INACTIVE:
            u->state = U_USERSTATE_ACTIVE;
        case U_USERSTATE_ACTIVE:
        break;
        default:
        break;
        }
        os_mutexUnlock(&u->mutex);
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

    u = u_user(user);
    if (u != NULL) {
        os_mutexLock(&u->mutex);
        switch (u->state) {
        case U_USERSTATE_TERMINATING:
            OS_REPORT(OS_ERROR,"u_userProtect",0,
                      "Internal error: Try to protect terminating process.");
            r = U_RESULT_INTERNAL_ERROR;
        break;
        case U_USERSTATE_ACTIVE:
            osr = os_threadProtect();
            if (osr == os_resultSuccess) {
                u->protectCount++;
                r = U_RESULT_OK;
            } else {
                r = U_RESULT_INTERNAL_ERROR;
            }
        break;
        case U_USERSTATE_DETACHING:
        case U_USERSTATE_INACTIVE:
            if ((u->detachThreadId != 0) &&
                (os_threadIdSelf() != u->detachThreadId)) {
                r = U_RESULT_DETACHING;
            } else {
                osr = os_threadProtect();
                if (osr == os_resultSuccess) {
                    u->protectCount++;
                    r = U_RESULT_OK;
                } else {
                    r = U_RESULT_INTERNAL_ERROR;
                }
            }
        break;
        default:
            OS_REPORT(OS_ERROR,"u_userProtect",0,
                      "Internal error: Illegal user layer state detected.");
            r = U_RESULT_INTERNAL_ERROR;
        break;
        }
        os_mutexUnlock(&u->mutex);
    } else {
        OS_REPORT(OS_ERROR,"u_userProtect",0,
                  "User layer not initialised.");
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

    u = u_user(user);
    if (u != NULL) {
        os_mutexLock(&u->mutex);
        osr = os_threadUnprotect();
        if (osr == os_resultSuccess) {
            u->protectCount--;
            switch (u->state) {
            case U_USERSTATE_TERMINATING:
            case U_USERSTATE_ACTIVE:
            case U_USERSTATE_DETACHING:
            case U_USERSTATE_INACTIVE:
                r = U_RESULT_OK;
            break;
            default:
                OS_REPORT(OS_ERROR,"u_userUnprotect",0,
                          "Internal error: Illegal user layer state detected.");
                r = U_RESULT_INTERNAL_ERROR;
            break;
            }
        } else {
            assert(osr == os_resultSuccess);
            r = U_RESULT_INTERNAL_ERROR;
        }
        os_mutexUnlock(&u->mutex);
    } else {
        OS_REPORT(OS_ERROR,"u_userUnprotect",0,
                  "User layer not initialised.");
        r = U_RESULT_NOT_INITIALISED;
    }
    return r;
}

c_long
u_userProtectCount()
{
    u_user u;
    c_long count;

    u = u_user(user);
    if (u == NULL) {
        count = 0;
    } else {
        os_mutexLock(&u->mutex);
        count = u->protectCount;
        os_mutexUnlock(&u->mutex);
    }
    return count;
}


u_result
u_userAdd(
    u_kernel kernel)
{
    u_user u;
    u_kernelAdmin ka;
    u_result r = U_RESULT_OK;

    u = u_user(user);
    if (u != NULL) {
        ka = (u_kernelAdmin)os_malloc(sizeof(C_STRUCT(u_kernelAdmin)));
        if (ka == NULL) {
            OS_REPORT(OS_ERROR,"u_userAdd",0,
                      "Allocation of Kernel admin failed.");
            r = U_RESULT_INTERNAL_ERROR;
        }
        ka->kernel = kernel;
        ka->refCount = 1;
        os_mutexLock(&u->mutex);
        u->kernelList = c_iterInsert(u->kernelList,ka);
        os_mutexUnlock(&u->mutex);
    } else {
        OS_REPORT(OS_ERROR,"u_userAdd",0,
                  "User layer not initialised");
        r = U_RESULT_NOT_INITIALISED;
    }
    return r;
}

C_CLASS(getAdminKernelArg);
C_STRUCT(getAdminKernelArg) {
    u_kernelAction action;
    u_kernelActionArg arg;
};

static void
getAdminKernel(
    c_voidp arg1,
    c_voidp arg2)
{
    u_kernelAdmin ka = (u_kernelAdmin)arg1;
    getAdminKernelArg a = (getAdminKernelArg)arg2;

    a->action(ka->kernel,a->arg);
}

u_result
u_userWalk(
    u_kernelAction action,
    u_kernelActionArg arg)
{
    C_STRUCT(getAdminKernelArg) a;
    u_user u;
    u_result r = U_RESULT_OK;

    u = u_user(user);
    if (u != NULL) {
        a.action = action;
        a.arg = arg;
        os_mutexLock(&u->mutex);
        c_iterWalk(u->kernelList,getAdminKernel,&a);
        os_mutexUnlock(&u->mutex);
    } else {
        OS_REPORT(OS_ERROR,"u_userWalk",0,
                  "User layer not initialised");
        r = U_RESULT_NOT_INITIALISED;
    }
    return r;
}

static c_bool
compareKernel(
    c_object o,
    c_iterActionArg arg)
{
    u_kernelAdmin ka = (u_kernelAdmin)o;
    u_kernel kernel = (u_kernel)arg;

    return (c_bool)(ka->kernel == kernel);
}

u_result
u_userRemove(
    u_kernel kernel)
{
    u_user u;
    u_kernelAdmin ka;
    u_result r;

    u = u_user(user);
    if (u != NULL) {
        os_mutexLock(&u->mutex);
        ka = c_iterTakeAction(u->kernelList,compareKernel,kernel);
        if (ka == NULL) {
            OS_REPORT(OS_ERROR,"u_userRemove",0,
                      "Internal error: Specified Kernel not found in admin.");
            r = U_RESULT_ILL_PARAM;
        } else {
            os_free(ka);
            r = U_RESULT_OK;
        }
        os_mutexUnlock(&u->mutex);
    } else {
        OS_REPORT(OS_ERROR,"u_userRemove",0,
                  "User layer not initialised");
        r = U_RESULT_NOT_INITIALISED;
    }
    return r;
}

static c_bool
compareAdminUri(
    c_object o,
    c_iterActionArg arg)
{
    u_kernelAdmin ka = (u_kernelAdmin)o;
    const c_char *uri = (const c_char *)arg;
    const c_char *kernelUri;
    u_result result;

    if (ka != NULL) {
        kernelUri = u_kernelUri(ka->kernel);
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
    u_result r;
    u_kernelAdmin ka;

    result = NULL;

    if (uri == NULL || strlen (uri) == 0) {
        uri = os_getenv ("OSPL_URI");
    }

    u = u_user(user);
    if (u != NULL) {
        /* If the kernel is already opened by the process,
           return the kernel object.
           Otherwise open the kernel and add to the administration */
        os_mutexLock(&u->mutex);
        ka = (u_kernelAdmin)c_iterReadAction(u->kernelList,
                                             compareAdminUri,
                                             (void *)uri);
        if (ka != NULL) {
            ka->refCount++;
            result = ka->kernel;
        } else {
            result = u_kernelOpen(uri, timeout);
            if (result != NULL) {
                ka = (u_kernelAdmin)os_malloc(sizeof(C_STRUCT(u_kernelAdmin)));
                if (ka != NULL) {
                    ka->kernel = result;
                    ka->refCount = 1;
                    u->kernelList = c_iterInsert(u->kernelList,ka);
                }
            } else {
                ka = (u_kernelAdmin)c_iterReadAction(u->kernelList,
                                                     compareAdminUri,
                                                     (void *)uri);
                if (ka != NULL) {
                    ka->refCount++;
                    result = ka->kernel;
                } else {
                    /* If timeout = -1, don't report */
                    if (timeout >= 0) {
                        if (uri == NULL) {
                            OS_REPORT(OS_ERROR,"u_userKernelOpen",0,
                                      "Failed to open: The default domain");
                        } else {
                            OS_REPORT_1(OS_ERROR,"u_userKernelOpen",0,
                                        "Failed to open: %s",uri);
                        }
                    }
                }
            }
        }
        os_mutexUnlock(&u->mutex);
    } else {
        OS_REPORT(OS_ERROR,"u_userKernelOpen",0,
                  "User layer not initialised");
        r = U_RESULT_NOT_INITIALISED;
    }

    return result;
}

static c_bool
compareAdminKernel(
    c_object o,
    c_iterActionArg arg)
{
    u_kernelAdmin ka = (u_kernelAdmin)o;
    u_kernel kernel = (u_kernel)arg;

    return ((ka != NULL) && (ka->kernel == kernel));
}

u_result
u_userKernelClose(
    u_kernel kernel)
{
    u_user u;
    u_result r;
    u_kernelAdmin ka;

    if (kernel == NULL) {
        OS_REPORT(OS_WARNING,"u_userKernelClose",0,
                  "No Kernel specified");
        return U_RESULT_OK;
    }
    u = u_user(user);
    if (u != NULL) {
        os_mutexLock(&u->mutex);

        ka = (u_kernelAdmin)c_iterReadAction(u->kernelList,
                                             compareAdminKernel,
                                             kernel);
        os_mutexUnlock(&u->mutex);
        if (ka != NULL) {
            assert(ka->refCount>0);
            ka->refCount--;
            if (ka->refCount == 0) {
                ka = (u_kernelAdmin)c_iterTakeAction(u->kernelList,
                                                     compareAdminKernel,
                                                     kernel);
                u_kernelClose(ka->kernel);
                os_free(ka);
            }
            r = U_RESULT_OK;
        } else {
            OS_REPORT(OS_ERROR,"u_userKernelClose",0,
                      "Internal Error: Specified Kernel not found in admin");
            r = U_RESULT_INTERNAL_ERROR;
        }
    } else {
        OS_REPORT(OS_ERROR,"u_userKernelClose",0,
                  "User layer not initialised");
        r = U_RESULT_NOT_INITIALISED;
    }
    return r;
}

#ifdef WIN32
/* We need this on windows to make sure the main thread of MFC applications
 * calls os_osInit(). Therefore we execute u_userInitialize();
 */
BOOL WINAPI DllMain(
    HINSTANCE hinstDLL,  /* handle to DLL module */
    DWORD fdwReason,     /* reason for calling function */
    LPVOID lpReserved )  /* reserved */
{
    /* Perform actions based on the reason for calling.*/
    switch( fdwReason ) {
    case DLL_PROCESS_ATTACH:
         /* Initialize once for each new process.
            Return FALSE to fail DLL load.
          */
        u_userInitialise();
    break;
    case DLL_THREAD_ATTACH:
         /* Do thread-specific initialization.
          */
    break;
    case DLL_THREAD_DETACH:
         /* Do thread-specific cleanup.
          */
    break;
    case DLL_PROCESS_DETACH:
         /* Perform any necessary cleanup.
          */
    break;
    }
    return TRUE;  /* Successful DLL_PROCESS_ATTACH.*/
}
#endif
