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
    C_STRUCT(u_kernelAdmin) kernelList[MAX_KERNEL];
    c_long                  kernelCount;
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
        OS_REPORT(OS_WARNING,
                  "u_userDetach",0,
                  "User layer not initialised.");
    }
}

u_result
u_userDetach()
{
    u_user u;
    u_result r = U_RESULT_OK;
    c_long i;
    u_kernel kernel;

    if (user != NULL) {
        u = u_user(user);
        os_mutexLock(&u->mutex);
        switch (u->state) {
        case U_USERSTATE_ACTIVE:
            u->state = U_USERSTATE_DETACHING;
            u->detachThreadId = os_threadIdSelf();
            for (i=1; (i<=u->kernelCount); i++) {
                kernel = u->kernelList[i].kernel;
                if (kernel) {
                    u->kernelList[i].kernel = NULL;
                    os_mutexUnlock(&u->mutex);
                    u_kernelDetachParticipants(kernel);
                    u_kernelClose(kernel);
                    os_mutexLock(&u->mutex);
                }
            }
            u->state = U_USERSTATE_INACTIVE;
            u->detachThreadId = 0;
        break;
        case U_USERSTATE_DETACHING:
        case U_USERSTATE_TERMINATING:
        break;
        default:
            OS_REPORT(OS_ERROR,
                      "u_userDetach",0,
                      "Internal error: Illegal User layer state detected.");
            r = U_RESULT_INTERNAL_ERROR;
        break;
        }
        os_mutexUnlock(&u->mutex);
    } else {
        OS_REPORT(OS_WARNING,
                  "u_userDetach",0,
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
            OS_REPORT(OS_ERROR,
                      "u_userInitialise", 0,
                      "Allocation user admin failed: out of memory.");
            rm = U_RESULT_OUT_OF_MEMORY;
        }

        u = u_user(user);
        os_mutexAttrInit(&mutexAttr);
        mutexAttr.scopeAttr = OS_SCOPE_PRIVATE;
        os_mutexInit(&u->mutex,&mutexAttr);
        u->kernelCount = 0;
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
            OS_REPORT(OS_ERROR,
                      "u_userInitialise",0,
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
            OS_REPORT(OS_ERROR,
                      "u_userProtect",0,
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
            OS_REPORT(OS_ERROR,
                      "u_userProtect",0,
                      "Internal error: Illegal user layer state detected.");
            r = U_RESULT_INTERNAL_ERROR;
        break;
        }
        os_mutexUnlock(&u->mutex);
    } else {
        OS_REPORT(OS_ERROR,
                  "u_userProtect",0,
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
                OS_REPORT(OS_ERROR,
                          "u_userUnprotect",0,
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
        OS_REPORT(OS_ERROR,
                  "u_userUnprotect",0,
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
    u_kernel _this;
    u_kernelAdmin ka;
    u_user u;

    u = u_user(user);

    if (u->kernelCount < MAX_KERNEL) {
        _this = u_kernelNew(uri);
        if (_this != NULL) {
            os_mutexLock(&u->mutex);
            u->kernelCount++;
            ka = &u->kernelList[u->kernelCount];
            ka->kernel = _this;
            ka->refCount = 1;
            os_mutexUnlock(&u->mutex);
        }
    } else {
        _this = NULL;
        OS_REPORT(OS_ERROR,
                  "u_userKernelNew",0,
                  "Max connected Domains reached!");
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

    u = u_user(user);
    if (u != NULL) {
        /* If the kernel is already opened by the process,
           return the kernel object.
           Otherwise open the kernel and add to the administration */
        os_mutexLock(&u->mutex);
        ka = NULL;
        for (i=1; i<=u->kernelCount; i++) {
            if (compareAdminUri(u->kernelList[i].kernel,(void *)uri)) {
                ka = &u->kernelList[i];
                ka->refCount++;
                result = ka->kernel;
            }
        }
        if (ka == NULL) {
            if (u->kernelCount < MAX_KERNEL) {
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
                OS_REPORT(OS_ERROR,
                          "u_userKernelOpen",0,
                          "Max connected Domains reached!");
            }
        }
        os_mutexUnlock(&u->mutex);
    } else {
        OS_REPORT(OS_ERROR,
                  "u_userKernelOpen",0,
                  "User layer not initialised");
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

    u = u_user(user);
    if (u != NULL) {
        os_mutexLock(&u->mutex);
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
        os_mutexUnlock(&u->mutex);
        if (detach) {
            u_kernelClose(kernel);
        }
    } else {
        OS_REPORT(OS_ERROR,
                  "u_userKernelClose", 0,
                  "User layer not initialised");
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

c_long
u_userServer(
    c_long id)
{
    u_kernel kernel;
    c_long idx;
    c_long server;
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
