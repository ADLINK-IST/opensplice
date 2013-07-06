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

#include "v__serviceManager.h"
#include "v_participant.h"
#include "v_service.h"
#include "v__service.h"
#include "v__serviceState.h"
#include "v__lease.h"
#include "v__observer.h"
#include "v_observable.h"
#include "v_entity.h"
#include "v_public.h"
#include "v_event.h"
#include "v_time.h"
#include "c_collection.h"
#include "c_iterator.h"
#include "os_report.h"

struct collectNamesArg {
    v_serviceStateKind kind;
    c_iter list;
};

/****************************************************************************
 * Private functions
 ****************************************************************************/
static c_bool
collectNames(
    c_object s,
    c_voidp arg)
{
    v_serviceState state = (v_serviceState)s;
    struct collectNamesArg *a = (struct collectNamesArg *)arg;
    v_serviceStateKind kind;
    const c_char *name;

    kind = v_serviceStateGetKind(state);
    if (kind == a->kind) {
        name = v_serviceStateGetName(state);

        a->list = c_iterInsert(a->list, (void *)name);
    }
    return TRUE;
}

/****************************************************************************
 * New/Free
 ****************************************************************************/
static void
v_serviceManagerInit(
    v_serviceManager sm)
{
    v_kernel k;

    assert(sm != NULL);
    assert(C_TYPECHECK(sm, v_serviceManager));

    k = v_objectKernel(sm);

    v_observerInit(v_observer(sm), "ServiceManager", NULL, TRUE);

    c_mutexInit(&sm->mutex, SHARED_MUTEX);
    sm->serviceStates = c_tableNew(v_kernelType(k, K_SERVICESTATE), "name");
}

v_serviceManager
v_serviceManagerNew(
    v_kernel k)
{
    v_serviceManager sm;

    assert(C_TYPECHECK(k, v_kernel));

    sm = v_serviceManager(v_objectNew(k, K_SERVICEMANAGER));
    v_serviceManagerInit(sm);

    return sm;
}


/****************************************************************************
 * Protected functions
 ****************************************************************************/
void
v_serviceManagerNotify(
    v_serviceManager serviceManager,
    v_event event,
    c_voidp userData)
{
    OS_UNUSED_ARG(serviceManager);
    OS_UNUSED_ARG(userData);
    assert(serviceManager != NULL);
    assert(C_TYPECHECK(serviceManager,v_serviceManager));

    /* Only V_EVENT_SERVICESTATE_CHANGED is expected!
       event may be NULL in case of termination.
    */
    if ((event != NULL) && (event->kind != V_EVENT_SERVICESTATE_CHANGED)) {
        OS_REPORT(OS_WARNING, "v_serviceManager", 0, "Unexpected event");
        assert(FALSE);
    }
}

v_serviceState
v_serviceManagerRegister(
    v_serviceManager serviceManager,
    v_service service,
    const c_char *extStateName)
{
    v_kernel kernel;
    v_serviceState state;
    v_serviceState found;

    assert(serviceManager != NULL);
    assert(C_TYPECHECK(serviceManager, v_serviceManager));
    assert(service != NULL);
    assert(C_TYPECHECK(service, v_service));

    kernel = v_objectKernel(serviceManager);
    state = v_serviceStateNew(kernel, v_participantName(service), extStateName);
    if (state != NULL) {
        c_mutexLock(&serviceManager->mutex);

        found = c_insert(serviceManager->serviceStates, state);

        if (found != state) {
            c_free(state);
            state = NULL;
            c_keep(found);
        }
        c_mutexUnlock(&serviceManager->mutex);
        /* observe the state */
        v_observableAddObserver(v_observable(found), v_observer(serviceManager), NULL);
    } else {
        found = NULL;
    }

    return found;
}

v_serviceState
v_serviceManagerGetServiceState(
    v_serviceManager serviceManager,
    const c_char *serviceName)
{
    v_serviceState state;
    c_iter list; 
    c_collection q;
    q_expr expr;
    c_value params[1];

    assert(serviceManager != NULL);
    assert(C_TYPECHECK(serviceManager, v_serviceManager));
    assert(serviceName != NULL);

    expr = (q_expr)q_parse("name like %0");
    params[0] = c_stringValue(c_stringNew(c_getBase(serviceManager), serviceName));

    c_mutexLock(&serviceManager->mutex);
    q = c_queryNew(serviceManager->serviceStates, expr, params);
    q_dispose(expr);
    list = ospl_c_select(q, 0);
    c_free(q);
    c_mutexUnlock(&serviceManager->mutex);

    c_free(params[0].is.String);
    state = c_iterTakeFirst(list);
    assert(c_iterLength(list) == 0);
    c_iterFree(list);

    return state;
}

/****************************************************************************
 * Public functions
 ****************************************************************************/
v_serviceStateKind
v_serviceManagerGetServiceStateKind(
    v_serviceManager serviceManager,
    const c_char *serviceName)
{
    v_serviceState state;
    v_serviceStateKind result;

    assert(serviceManager != NULL);
    assert(C_TYPECHECK(serviceManager, v_serviceManager));
    assert(serviceName != NULL);

    state = v_serviceManagerGetServiceState(serviceManager, serviceName);

    if (state != NULL) {
        result = v_serviceStateGetKind(state);
        c_free(state);
    } else {
        result = STATE_NONE;
    }

    return result;
}

c_iter
v_serviceManagerGetServices(
    v_serviceManager serviceManager,
    v_serviceStateKind kind)
{
    struct collectNamesArg arg;

    assert(serviceManager != NULL);
    assert(C_TYPECHECK(serviceManager, v_serviceManager));

    arg.kind = kind;
    arg.list = c_iterNew(NULL);

    c_mutexLock(&serviceManager->mutex);
    c_walk(serviceManager->serviceStates, collectNames, &arg);
    c_mutexUnlock(&serviceManager->mutex);

    return arg.list;
}

c_bool
v_serviceManagerRemoveService(
        v_serviceManager serviceManager,
        const c_char *serviceName)
{
    v_serviceState state,removed;
    c_bool result = FALSE;
    assert(serviceManager != NULL);
    assert(C_TYPECHECK(serviceManager, v_serviceManager));
    assert(serviceName != NULL);

    state = v_serviceManagerGetServiceState(serviceManager,serviceName);
    if (state) {
        c_mutexLock(&serviceManager->mutex);
        removed = c_remove(serviceManager->serviceStates, state, NULL, NULL);
        if (state != removed) {
            OS_REPORT_1(OS_ERROR, "v_serviceManagerRemoveService", 0,
                      "Could not remove the service %s form the serviceset",serviceName);
        } else {
            result = TRUE;
        }
        c_mutexUnlock(&serviceManager->mutex);
    } else {
        OS_REPORT_1(OS_ERROR, "v_serviceManagerRemoveService", 0,
                   "Could not get the service state for service %s",serviceName);
    }
    return result;

}
