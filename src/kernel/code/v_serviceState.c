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

#if !defined NDEBUG
#include "c_metabase.h"
#endif

#include "os_report.h"

#include "v_serviceState.h"
#include "v__serviceState.h"
#include "v_service.h"
#include "v__observable.h"
#include "v_entity.h"
#include "v_event.h"
#include "v_public.h"
#include "os_report.h"

v_serviceState
v_serviceStateNew(
    v_kernel k,
    const c_char *name,
    const c_char *extStateName)
{
    v_serviceState s;
    c_type type;

    assert(C_TYPECHECK(k, v_kernel));

    if (extStateName == NULL) {
        s = v_serviceState(v_objectNew(k, K_SERVICESTATE));
    } else {
        type = c_resolve(c_getBase(c_object(k)), extStateName);
        if (type != NULL) {
#if !defined NDEBUG
            c_type t = type;
            c_bool correctType;
            correctType = FALSE;

            while ((correctType == FALSE) &&
                   (t != NULL) &&
                   (c_baseObject(t)->kind == M_CLASS)) {
                if (strcmp(c_metaObject(t)->name, "v_serviceState") == 0) {
                    correctType = TRUE;
                } else {
                    t = c_type(c_class(t)->extends);
                }
            }

            if ((correctType == FALSE) && (t != NULL)) {
                OS_REPORT(OS_FATAL, "v_serviceState", V_RESULT_ILL_PARAM, "Given type not a class");
                assert(0);
            } else {

                if (correctType == FALSE) {
                    OS_REPORT(OS_FATAL, "v_serviceState",
                              V_RESULT_ILL_PARAM, "Given type does not extend v_serviceState");
                    assert(0);
                }
            }
#endif
            s = v_serviceState(c_new(type));
            if (s) {
                v_objectKind(s) = K_SERVICESTATE;
                v_object(s)->kernel = k;
            } else {
                OS_REPORT(OS_ERROR,
                          "v_serviceStateNew",V_RESULT_INTERNAL_ERROR,
                          "Failed to allocate v_serviceState object.");
                assert(FALSE);
            }
        } else {
            s = NULL;
        }
    }
    if (s != NULL) {
        v_serviceStateInit(s, name);
    }

    return s;
}

void
v_serviceStateInit(
    v_serviceState serviceState,
    const c_char *name)
{
    assert(C_TYPECHECK(serviceState, v_serviceState));

    v_observableInit(v_observable(serviceState));

    serviceState->name = c_stringNew(c_getBase(serviceState),name);

    serviceState->stateKind = STATE_NONE;
}

c_bool
v_serviceStateChangeState(
    v_serviceState serviceState,
    v_serviceStateKind stateKind)
{
    c_bool changed;
    C_STRUCT(v_event) event;

    assert(C_TYPECHECK(serviceState, v_serviceState));

    OSPL_LOCK(serviceState);

    switch (stateKind) {
    case STATE_NONE:
      break;
    case STATE_DIED:
      if ((serviceState->stateKind != STATE_NONE) &&
          (serviceState->stateKind != STATE_TERMINATED))
      {
          serviceState->stateKind = stateKind;
      }
      break;
    case STATE_INITIALISING:
      if ((serviceState->stateKind == STATE_NONE) ||
          (serviceState->stateKind == STATE_DIED)) {
          serviceState->stateKind = stateKind;
      }
      break;
    case STATE_OPERATIONAL:
      if (serviceState->stateKind == STATE_INITIALISING) {
          serviceState->stateKind = stateKind;
      }
      break;
    case STATE_INCOMPATIBLE_CONFIGURATION:
      if ((serviceState->stateKind == STATE_OPERATIONAL) ||
          (serviceState->stateKind == STATE_INITIALISING)) {
          serviceState->stateKind = stateKind;
      }
      break;
    case STATE_TERMINATING:
      if ((serviceState->stateKind == STATE_INITIALISING) ||
          (serviceState->stateKind == STATE_OPERATIONAL)) {
          serviceState->stateKind = stateKind;
      }
      break;
    case STATE_TERMINATED:
      if (serviceState->stateKind == STATE_TERMINATING) {
          serviceState->stateKind = stateKind;
      }
      break;
    default:
      OS_REPORT(OS_ERROR,"Kernel::ServiceState",V_RESULT_ILL_PARAM,
                  "Unkown state (%d) kind provided.",stateKind);
      assert(FALSE); /* unknown state */
      break;
    }
    if (serviceState->stateKind == stateKind) {
        changed = TRUE;
    } else {
        changed = FALSE;
    }

    event.kind = V_EVENT_SERVICESTATE_CHANGED;
    event.source = v_observable(serviceState);
    event.data = NULL;
    event.handled = TRUE;

    OSPL_THROW_EVENT(serviceState, &event);
    OSPL_UNLOCK(serviceState);

    return changed;
}

v_serviceStateKind
v_serviceStateGetKind(
    v_serviceState serviceState)
{
    v_serviceStateKind kind;

    assert(C_TYPECHECK(serviceState, v_serviceState));

    OSPL_LOCK(serviceState);
    kind = serviceState->stateKind;
    OSPL_UNLOCK(serviceState);

    return kind;
}

const c_char *
v_serviceStateGetName(
    v_serviceState serviceState)
{
    c_char *name;

    assert(C_TYPECHECK(serviceState, v_serviceState));

    OSPL_LOCK(serviceState);
    name = serviceState->name;
    OSPL_UNLOCK(serviceState);

    return (const c_char *)name;
}
