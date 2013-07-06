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
#include "s_gc.h"
#include "spliced.h"
#include "s_configuration.h"
#include "s_misc.h"

#include "os.h"
#include "u_user.h"

C_STRUCT(s_garbageCollector) {
    os_threadId id;
    os_mutex mtx;
    os_cond cv;
    int active;
    u_spliced spliced;
};

/**************************************************************
 * Private functions
 **************************************************************/
static void *
garbageCollector(
    void *arg)
{
    s_garbageCollector gc = (s_garbageCollector)arg;
    os_mutexLock(&gc->mtx);
    gc->active++;
    os_condBroadcast(&gc->cv);
    os_mutexUnlock(&gc->mtx);
    u_splicedGarbageCollector(gc->spliced);
    return NULL;
}

/**************************************************************
 * constructor/destructor
 **************************************************************/

/**************************************************************
 * Protected functions
 **************************************************************/
s_garbageCollector
s_garbageCollectorNew(
    spliced daemon)
{
    s_garbageCollector gc;
    s_configuration config;
    os_mutexAttr mtxAttr;
    os_condAttr cvAttr;
    int status;
    os_result osr;

    status = 0;
    gc = os_malloc((os_uint32)C_SIZEOF(s_garbageCollector));
    if (gc) {
        gc->spliced = splicedGetService(daemon);
        gc->active = 0;
        osr = os_mutexAttrInit(&mtxAttr);
        if (osr == os_resultSuccess) {
            mtxAttr.scopeAttr = OS_SCOPE_PRIVATE;
            osr = os_mutexInit(&gc->mtx, &mtxAttr);
        } else {
            status++;
        }
        if (osr == os_resultSuccess) {
            osr = os_condAttrInit(&cvAttr);
            if (osr == os_resultSuccess) {
                cvAttr.scopeAttr = OS_SCOPE_PRIVATE;
                osr = os_condInit(&gc->cv, &gc->mtx, &cvAttr);
            } else {
                os_mutexDestroy(&gc->mtx); /* don't care if this succeeds, already in error situation */
                status++;
            }
            if (osr == os_resultSuccess) {
                config = splicedGetConfiguration(daemon);
                osr = os_threadCreate(&gc->id, S_THREAD_GARBAGE_COLLECTOR, &config->garbageCollectorScheduling, garbageCollector, gc);
                if (osr != os_resultSuccess) {
                    /* don't care if the following statements succeeds, already in error situation */
                    os_mutexDestroy(&gc->mtx);
                    os_condDestroy(&gc->cv);
                    status++;
                }
            }
        } else {
            status++;
        }
    }
    
    if (status && gc) {
        os_free(gc);
        gc = NULL;
    }
    
    return gc;
}

void
s_garbageCollectorFree(
    s_garbageCollector gc)
{
    if (gc) { /* gc might be NULL, when spliced has detected other spliced */
        os_threadWaitExit(gc->id, NULL);
        os_condDestroy(&gc->cv);
        os_mutexDestroy(&gc->mtx);
        os_free(gc);
    }
}

int
s_garbageCollectorWaitForActive(
    s_garbageCollector gc)
{
    int result;
    os_time delay = {2, 0};
    os_result osr;
    
    os_mutexLock(&gc->mtx);
    osr = os_resultSuccess;
    while ((gc->active == 0) && (osr == os_resultSuccess)) {
        osr = os_condTimedWait(&gc->cv, &gc->mtx, &delay);
    }
    result = gc->active;
    os_mutexUnlock(&gc->mtx);
    return result;
}

/**************************************************************
 * Public functions
 **************************************************************/
