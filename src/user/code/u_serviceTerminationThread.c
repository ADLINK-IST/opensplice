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

#include "os.h"
#include "os_report.h"

#include "u_user.h"
#include "u__user.h"
#include "u__types.h"
#include "u_service.h"
#include "u_serviceTerminationThread.h"

C_STRUCT(u_serviceTerminationThread) {
    os_cond       terminated;
    os_mutex      mtx;
    os_threadId   sttId;
    c_bool        active;
};

static void *
u_serviceTerminationThreadMain(
    void *arg)
{
    os_result result;
    u_serviceTerminationThread service = (u_serviceTerminationThread)arg;
    os_time delay = {60, 0}; /* wait 60 seconds before forced exit */
    os_mutexLock(&service->mtx);
    result = os_resultSuccess;
    while((service->active) && (result == os_resultSuccess)){
        result = os_condTimedWait(&service->terminated, &service->mtx, &delay);
    }
    os_mutexUnlock(&service->mtx);
    if (result == os_resultTimeout) {
        OS_REPORT_1(OS_ERROR, "u_serviceTerminationThreadMain", 0,
                   "Process %d did not terminate in a normal way forcing exit now, this could be of a possible deadlock", os_procIdToInteger(os_procIdSelf()));
#ifdef _WRS_KERNEL
	exit(1);
#else
        _exit(1);
#endif
    }
    return NULL;
}

u_serviceTerminationThread
u_serviceTerminationThreadNew()
{
    os_result result;
    os_threadAttr osThreadAttr;
    u_serviceTerminationThread this;
    os_mutexAttr mtxAttr;
    os_condAttr cvAttr;

    os_threadAttrInit(&osThreadAttr);
    this = (u_serviceTerminationThread)os_malloc((os_uint32)C_SIZEOF(u_serviceTerminationThread));
    result = os_mutexAttrInit(&mtxAttr);
    if (result == os_resultSuccess) {
        mtxAttr.scopeAttr = OS_SCOPE_PRIVATE;
        result = os_mutexInit(&this->mtx, &mtxAttr);
    } else {
        OS_REPORT(OS_ERROR,"u_serviceTerminationThreadNew",0,
                  "Failed to initiate service mutex.");
    }
    if (result == os_resultSuccess) {
        result = os_condAttrInit(&cvAttr);
        if (result == os_resultSuccess) {
            cvAttr.scopeAttr = OS_SCOPE_PRIVATE;
            result = os_condInit(&this->terminated, &this->mtx, &cvAttr);
        } else {
            os_mutexDestroy(&this->mtx); /* don't care if this succeeds, already in error situation */
            OS_REPORT(OS_ERROR,"u_serviceTerminationThreadNew",0,
                      "Failed to initiate condition variable.");
        }
    }
    this->active = TRUE;

    if (result == os_resultSuccess) {
        result = os_threadCreate(&this->sttId, "Service Termination Thread", &osThreadAttr, u_serviceTerminationThreadMain, this);
        if (result != os_resultSuccess) {
            OS_REPORT_1(OS_ERROR, "u_serviceTerminationThreadNew", 0,
                      "Could not start the Service Termination Thread for process %d", os_procIdToInteger(os_procIdSelf()));
        }
    }
    return this;
}

u_result
u_serviceTerminationThreadFree(
    u_serviceTerminationThread this)
{
    os_result result;
    u_result r;
    r = U_RESULT_OK;

    os_mutexLock(&this->mtx);
    this->active = FALSE;
    result = os_condBroadcast(&this->terminated);
    os_mutexUnlock(&this->mtx);

    result = os_threadWaitExit(this->sttId, NULL);

    os_mutexDestroy(&this->mtx);
    os_condDestroy(&this->terminated);
    os_free(this);
    if (result != os_resultSuccess) {
        r = U_RESULT_INTERNAL_ERROR;
    }

    return r;
}
