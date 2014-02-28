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

#if !defined NDEBUG
#include "c_metabase.h"
#endif

#include "os_report.h"

#include "v_serviceState.h"
#include "v__serviceState.h"
#include "v_service.h"
#include "v_time.h"
#include "v__observable.h"
#include "v_entity.h"
#include "v_event.h"
#include "v_public.h"
#include "os_report.h"

/**************************************************************
 * constructor/destructor
 **************************************************************/
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
                OS_REPORT(OS_FATAL, "v_serviceState", 0, "Given type not a class");
                assert(0);
            } else {

                if (correctType == FALSE) {
                    OS_REPORT(OS_FATAL, "v_serviceState",
                              0, "Given type does not extend v_serviceState");
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
                          "v_serviceStateNew",0,
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

    v_observableInit(v_observable(serviceState),name, NULL, TRUE);

    c_lockInit(&serviceState->lock, SHARED_LOCK);
    serviceState->stateKind = STATE_NONE;
}


/**************************************************************
 * Protected functions
 **************************************************************/

/**************************************************************
 * Public functions
 **************************************************************/
c_bool
v_serviceStateChangeState(
    v_serviceState serviceState,
    v_serviceStateKind stateKind)
{
    c_bool changed;
    C_STRUCT(v_event) event;

    assert(C_TYPECHECK(serviceState, v_serviceState));

    c_lockWrite(&serviceState->lock);

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
      OS_REPORT_1(OS_ERROR,"Kernel::ServiceState",0,
                  "Unkown state (%d) kind provided.",stateKind);
      assert(FALSE); /* unknown state */
      break;
    }
    if (serviceState->stateKind == stateKind) {
        changed = TRUE;
    } else {
        changed = FALSE;
    }
    c_lockUnlock(&serviceState->lock);

    event.kind = V_EVENT_SERVICESTATE_CHANGED;
    event.source = v_publicHandle(v_public(serviceState));
    event.userData = NULL;
    v_observableNotify(v_observable(serviceState),&event);

    return changed;
}

v_serviceStateKind
v_serviceStateGetKind(
    v_serviceState serviceState)
{
    v_serviceStateKind kind;

    assert(C_TYPECHECK(serviceState, v_serviceState));

    c_lockRead(&serviceState->lock);
    kind = serviceState->stateKind;
    c_lockUnlock(&serviceState->lock);

    return kind;
}

const c_char *
v_serviceStateGetName(
    v_serviceState serviceState)
{
    c_char *name;

    assert(C_TYPECHECK(serviceState, v_serviceState));

    c_lockRead(&serviceState->lock);
    name = v_entityName(serviceState);
    c_lockUnlock(&serviceState->lock);

    return (const c_char *)name;
}

void
v_serviceStateNotify(
    v_serviceState serviceState,
    v_serviceStateKind kind)
{
    v_serviceStateChangeState(serviceState, kind);
}
