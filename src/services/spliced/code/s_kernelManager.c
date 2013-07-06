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
#include "s_kernelManager.h"
#include "s_misc.h"
#include "s_configuration.h"
#include "spliced.h"

#include "os.h"
#include "u_user.h"

C_STRUCT(s_kernelManager) {
    os_threadId id;
    os_mutex mtx;
    os_cond cv;
    int active;
    u_spliced spliced;
    os_threadId resendManager;
    os_threadId cAndMCommandManager;
};

/**************************************************************
 * Private functions
 **************************************************************/
static void *
kernelManager(
    void *arg)
{
    s_kernelManager km = (s_kernelManager)arg;
    os_mutexLock(&km->mtx);
    km->active++;
    os_condBroadcast(&km->cv);
    os_mutexUnlock(&km->mtx);
    u_splicedKernelManager(km->spliced);
    return NULL;
}

/* Resend manager thread for built-in participant */
static void *
resendManager(
    void *arg)
{
    s_kernelManager km = (s_kernelManager)arg;
    os_mutexLock(&km->mtx);
    km->active++;
    os_condBroadcast(&km->cv);
    os_mutexUnlock(&km->mtx);
    u_splicedBuiltinResendManager(km->spliced);
    return NULL;
 
}

static void *
cAndMCommandManager(
    void *arg)
{
    s_kernelManager km = (s_kernelManager)arg;
    os_mutexLock(&km->mtx);
    km->active++;
    os_condBroadcast(&km->cv);
    os_mutexUnlock(&km->mtx);
    u_splicedBuiltinCAndMCommandDispatcher(km->spliced);
    return NULL;
 
}


/**************************************************************
 * constructor/destructor
 **************************************************************/

/**************************************************************
 * Protected functions
 **************************************************************/
s_kernelManager
s_kernelManagerNew(
    spliced daemon)
{
    s_kernelManager km;
    s_configuration config;
    os_mutexAttr mtxAttr;
    os_condAttr cvAttr;
    int status;
    os_result osr;

    status = 0;
    km = os_malloc((os_uint32)C_SIZEOF(s_kernelManager));
    if (km) {
        km->spliced = splicedGetService(daemon);
        km->active = 0;
        osr = os_mutexAttrInit(&mtxAttr);
        if (osr == os_resultSuccess) {
            mtxAttr.scopeAttr = OS_SCOPE_PRIVATE;
            osr = os_mutexInit(&km->mtx, &mtxAttr);
        } else {
            status++;
        }
        if (osr == os_resultSuccess) {
            osr = os_condAttrInit(&cvAttr);
            if (osr == os_resultSuccess) {
                cvAttr.scopeAttr = OS_SCOPE_PRIVATE;
                osr = os_condInit(&km->cv, &km->mtx, &cvAttr);
            } else {
                os_mutexDestroy(&km->mtx); /* don't care if this succeeds, already in error situation */
                status++;
            }
            if (osr == os_resultSuccess) {
                config = splicedGetConfiguration(daemon);
                osr = os_threadCreate(&km->id, 
                            S_THREAD_KERNELMANAGER, &config->kernelManagerScheduling, 
                            kernelManager, km);
                if (osr != os_resultSuccess) {
                    /* don't care if the following statements succeeds, already in error situation */
                    os_mutexDestroy(&km->mtx);
                    os_condDestroy(&km->cv);
                    status++;
                }
            }
            if (osr == os_resultSuccess) {
                config = splicedGetConfiguration(daemon);
                osr = os_threadCreate(&km->resendManager,
                            S_THREAD_RESENDMANAGER, &config->resendManagerScheduling,
                            resendManager, km);
                if (osr != os_resultSuccess) {
                    /* don't care if the following statements succeeds, already in error situation */
                    os_mutexDestroy(&km->mtx);
                    os_condDestroy(&km->cv);
                    status++;
                }
            }
            if (osr == os_resultSuccess ) {
                config = splicedGetConfiguration(daemon);
                if (config->enableCandMCommandThread ) {
                   osr = os_threadCreate(&km->cAndMCommandManager,
                                         S_THREAD_C_AND_M_COMMANDMANAGER,
                                         &config->cAndMCommandScheduling,
                                         cAndMCommandManager, km);
                   if (osr != os_resultSuccess) {
                      /* don't care if the following statements succeeds, already in error situation */
                      os_mutexDestroy(&km->mtx);
                      os_condDestroy(&km->cv);
                      status++;
                   }
                }
            }
        } else {
            status++;
        }
    }
    
    if (status && km) {
        os_free(km);
        km = NULL;
    }
    
    return km;
}

void
s_kernelManagerFree(
    s_kernelManager km)
{
    u_result r;
    v_spliced s;

    if (km) { /* km might be NULL, when spliced has detected other spliced */
        os_threadWaitExit(km->id, NULL);
        os_threadWaitExit(km->resendManager, NULL);
        u_splicedCAndMCommandDispatcherQuit(km->spliced);
        os_threadWaitExit(km->cAndMCommandManager, NULL);
        os_condDestroy(&km->cv);
        os_mutexDestroy(&km->mtx);
        os_free(km);
    }
}

int
s_kernelManagerWaitForActive(
    s_kernelManager km)
{
    int result;
    os_time delay = {1, 0};
    os_time cur;
    os_time start;
    os_result osr;
    
    os_mutexLock(&km->mtx);
    osr = os_resultSuccess;
    cur = os_timeGet();
    start = cur;
    while ((km->active < 2) && (cur.tv_sec - start.tv_sec < 20)) {
        osr = os_condTimedWait(&km->cv, &km->mtx, &delay);
        cur = os_timeGet();
    }
    result = km->active;
    os_mutexUnlock(&km->mtx);
    return result;
}

/**************************************************************
 * Public functions
 **************************************************************/
