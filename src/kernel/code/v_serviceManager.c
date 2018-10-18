/*
 *                         Vortex OpenSplice
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR ADLINK
 *   Technology Limited, its affiliated companies and licensors. All rights
 *   reserved.
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

#include "v__serviceManager.h"
#include "v_participant.h"
#include "v_spliced.h"
#include "v_service.h"
#include "v__service.h"
#include "v__serviceState.h"
#include "v__lease.h"
#include "v__observer.h"
#include "v__observable.h"
#include "v__entity.h"
#include "v_public.h"
#include "v_event.h"
#include "c_collection.h"
#include "c_iterator.h"
#include "os_report.h"

struct collectNamesArg {
    v_serviceStateKind kind;
    c_iter list;
};

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

static void
v_serviceManagerInit(
    _Inout_ v_serviceManager sm)
{
    v_kernel k;

    assert(sm != NULL);
    assert(C_TYPECHECK(sm, v_serviceManager));

    k = v_objectKernel(sm);

    v_entityInit(v_entity(sm), "ServiceManager");
    (void)v_entityEnable(v_entity(sm));

    sm->serviceStates = c_tableNew(v_kernelType(k, K_SERVICESTATE), "name");
}

_Check_return_
_Ret_notnull_
v_serviceManager
v_serviceManagerNew(
    _In_ v_kernel k)
{
    v_serviceManager sm;

    assert(C_TYPECHECK(k, v_kernel));

    sm = v_serviceManager(v_objectNew(k, K_SERVICEMANAGER));
    v_serviceManagerInit(sm);

    return sm;
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
        OSPL_LOCK(serviceManager);

        found = ospl_c_insert(serviceManager->serviceStates, state);

        if (found != state) {
            c_free(state);
            state = NULL;
            c_keep(found);
        } else {
            OSPL_ADD_OBSERVER(state, serviceManager, V_EVENT_SERVICESTATE_CHANGED, NULL);
        }
        OSPL_UNLOCK(serviceManager);
        /* observe the state */
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

    OSPL_LOCK(serviceManager);
    q = c_queryNew(serviceManager->serviceStates, expr, params);
    q_dispose(expr);
    list = ospl_c_select(q, 0);
    c_free(q);
    OSPL_UNLOCK(serviceManager);

    c_free(params[0].is.String);
    state = c_iterTakeFirst(list);
    assert(c_iterLength(list) == 0);
    c_iterFree(list);

    return state;
}

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

    OSPL_LOCK(serviceManager);
    c_walk(serviceManager->serviceStates, collectNames, &arg);
    OSPL_UNLOCK(serviceManager);

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
        OSPL_LOCK(serviceManager);
        removed = c_remove(serviceManager->serviceStates, state, NULL, NULL);
        if (state != removed) {
            OS_REPORT(OS_ERROR, "v_serviceManagerRemoveService", V_RESULT_INTERNAL_ERROR,
                      "Could not remove the service %s form the serviceset",serviceName);
        } else {
            result = TRUE;
        }
        OSPL_UNLOCK(serviceManager);
    } else {
        OS_REPORT(OS_ERROR, "v_serviceManagerRemoveService", V_RESULT_INTERNAL_ERROR,
                   "Could not get the service state for service %s",serviceName);
    }
    return result;

}
