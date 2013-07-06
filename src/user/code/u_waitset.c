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
#include "u_waitset.h"
#include "u_waitsetEvent.h"
#include "u__types.h"
#include "u__entity.h"
#include "u__participant.h"
#include "u__domain.h"
#include "v_waitset.h"
#include "v_observable.h"
#include "v_observer.h"
#include "v_entity.h"
#include "v_event.h"
#include "os_report.h"

#include "u__user.h"

u_waitset
u_waitsetNew(
    u_participant p)
{
    u_waitset _this = NULL;
    u_result result;
    v_entity ke;
    v_participant kp = NULL;

    if (p != NULL) {
        result = u_entityWriteClaim(u_entity(p),(v_entity*)(&kp));
        if (result == U_RESULT_OK) {
            assert(kp);
            ke = v_entity(v_waitsetNew(kp));
            if (ke != NULL) {
                v_observerSetEventMask(v_observer(ke), V_EVENTMASK_ALL);
                _this = u_entityAlloc(p,u_waitset,ke,TRUE);
                if (_this != NULL) {
                    result = u_waitsetInit(_this);
                    if (result != U_RESULT_OK) {
                        OS_REPORT_1(OS_ERROR, "u_waitsetNew", 0,
                                    "Initialisation failed. "
                                    "For Participant (0x%x)", p);
                        u_waitsetFree(_this);
                    }
                } else {
                    OS_REPORT_1(OS_ERROR, "u_waitsetNew", 0,
                                "Create user proxy failed. "
                                "For Participant (0x%x)", p);
                }
                c_free(ke);
            } else {
                OS_REPORT_1(OS_ERROR, "u_waitsetNew", 0,
                            "Create kernel entity failed. "
                            "For Participant (0x%x)", p);
            }
            result = u_entityRelease(u_entity(p));
        } else {
            OS_REPORT_1(OS_WARNING, "u_waitsetNew", 0,
                        "Claim Participant (0x%x) failed.", p);
        }
    } else {
        OS_REPORT(OS_ERROR,"u_waitsetNew",0,
                  "No Participant specified.");
    }
    return _this;
}

u_result
u_waitsetInit(
    u_waitset _this)
{
    u_result result;

    if (_this != NULL) {
        u_entity(_this)->flags |= U_ECREATE_INITIALISED;
        result = U_RESULT_OK;
    } else {
        OS_REPORT(OS_ERROR,"u_waitsetInit",0, "Illegal parameter.");
        result = U_RESULT_ILL_PARAM;
    }
    return result;
}

u_result
u_waitsetFree(
    u_waitset _this)
{
    u_result result;
    c_bool destroy;

    result = u_entityLock(u_entity(_this));
    if (result == U_RESULT_OK) {
        destroy = u_entityDereference(u_entity(_this));
        /* if refCount becomes zero then this call
         * returns true and destruction can take place
         */
        if (destroy) {
            if (u_entityOwner(u_entity(_this))) {
                result = u_waitsetDeinit(_this);
            } else {
                /* This user entity is a proxy, meaning that it is not fully
                 * initialized, therefore only the entity part of the object
                 * can be deinitialized.
                 * It would be better to either introduce a separate proxy
                 * entity for clarity or fully initialize entities and make
                 * them robust against missing information.
                 */
                result = u_entityDeinit(u_entity(_this));
            }
            if (result == U_RESULT_OK) {
                u_entityDealloc(u_entity(_this));
            } else {
                OS_REPORT_2(OS_WARNING,
                            "u_waitsetFree",0,
                            "Operation u_waitsetDeinit failed: "
                            "Waitset = 0x%x, result = %s.",
                            _this, u_resultImage(result));
                u_entityUnlock(u_entity(_this));
            }
        } else {
            u_entityUnlock(u_entity(_this));
        }
    } else {
        OS_REPORT_2(OS_WARNING,
                    "u_waitsetFree",0,
                    "Operation u_entityLock failed: "
                    "Waitset = 0x%x, result = %s.",
                    _this, u_resultImage(result));
    }
    return result;
}

u_result
u_waitsetDeinit(
    u_waitset _this)
{
    u_result result;

    if (_this != NULL) {
        result = u_entityDeinit(u_entity(_this));
    } else {
        OS_REPORT(OS_ERROR,"u_waitsetDeinit",0, "Illegal parameter.");
        result = U_RESULT_ILL_PARAM;
    }
    return result;
}

static void
collectEntities(
    v_waitsetEvent e,
    c_voidp arg)
{
    c_iter *list;

    list = (c_iter *)arg;
    *list = c_iterInsert(*list, e->userData);
}

u_result
u_waitsetNotify(
    u_waitset w,
    c_voidp eventArg)
{
    u_result r = U_RESULT_OK;
    v_waitset kw;

    if (w != NULL) {
        r = u_entityReadClaim(u_entity(w), (v_entity*)(&kw));
        if (r == U_RESULT_OK) {
            assert(kw);
            v_waitsetTrigger(kw, eventArg);
            r = u_entityRelease(u_entity(w));
        }
    }
    return r;
}

u_result
u_waitsetWait(
    u_waitset w,
    c_iter *list)
{
    u_result r;
    v_waitset kw;

    if ((w != NULL) && (list != NULL)) {
        r = u_entityReadClaim(u_entity(w), (v_entity*)(&kw));
        if (r == U_RESULT_OK) {
            assert(kw);
            *list = NULL;
            r = u_resultFromKernel(v_waitsetWait(kw,collectEntities,list));
            u_entityRelease(u_entity(w));
        }
    } else {
        OS_REPORT_2(OS_ERROR, "u_waitsetWait", 0,
                    "Illegal parameter specified (ws=0x"
                    PA_ADDRFMT
                    ",list=0x"
                    PA_ADDRFMT
                    ")",
                    (PA_ADDRCAST)w, (PA_ADDRCAST)list);
        r = U_RESULT_ILL_PARAM;
    }

    return r;
}

u_result
u_waitsetTimedWait(
    u_waitset w,
    const c_time t,
    c_iter *list)
{
    u_result r;
    v_waitset kw;

    if ((w != NULL) && (list != NULL)) {
        r = u_entityReadClaim(u_entity(w), (v_entity*)(&kw));
        if (r == U_RESULT_OK) {
            assert(kw);
            *list = NULL;
            r = u_resultFromKernel(v_waitsetTimedWait(kw,collectEntities,list,t));
            u_entityRelease(u_entity(w));
        }
    } else {
        OS_REPORT_2(OS_ERROR, "u_waitsetTimedWait", 0,
                    "Illegal parameter specified (ws=0x"
                    PA_ADDRFMT
                    ",list=0x"
                    PA_ADDRFMT
                    ")",
                    (PA_ADDRCAST)w, (PA_ADDRCAST)list);
        r = U_RESULT_ILL_PARAM;
    }

    return r;
}

u_result
u_waitsetWaitAction (
    u_waitset w,
    u_waitsetAction action,
    c_voidp arg)
{
    u_result r;
    v_waitset kw;

    if (w != NULL) {
        r = u_entityReadClaim(u_entity(w), (v_entity*)(&kw));
        if (r == U_RESULT_OK) {
            assert(kw);
            r = u_resultFromKernel(v_waitsetWait(kw,action,arg));
            u_entityRelease(u_entity(w));
        }
    }else    {
        OS_REPORT(OS_ERROR, "u_waitsetWaitAction", 0,
                  "Illegal parameter specified.");
        r = U_RESULT_ILL_PARAM;
    }
    return r;
}

u_result
u_waitsetTimedWaitAction (
    u_waitset w,
    u_waitsetAction action,
    c_voidp arg,
    const c_time t)
{
    u_result r;
    v_waitset kw;

    if (w != NULL) {
        r = u_entityReadClaim(u_entity(w), (v_entity*)(&kw));
        if (r == U_RESULT_OK) {
            assert(kw);
            r = u_resultFromKernel(v_waitsetTimedWait(kw,action,arg,t));
            u_entityRelease(u_entity(w));
        }
    } else    {
        OS_REPORT(OS_ERROR, "u_waitsetTimedWaitAction", 0,
                  "Illegal parameter specified.");
        r = U_RESULT_ILL_PARAM;
    }
    return r;
}

static void
collectEvents(
    v_waitsetEvent e,
    c_voidp arg)
{
    c_iter *list;
    u_waitsetEvent ev;
    v_waitsetEventHistoryDelete evd;
    v_waitsetEventHistoryRequest evr;
    v_waitsetEventPersistentSnapshot evps;
    v_waitsetEventConnectWriter ecw;
    list = (c_iter *)arg;

    if(e->kind == V_EVENT_HISTORY_DELETE){
        evd = (v_waitsetEventHistoryDelete)e;
        ev  = u_waitsetHistoryDeleteEventNew(u_entity(e->userData),
                                             e->kind,
                                             evd->partitionExpr,
                                             evd->topicExpr,
                                             evd->deleteTime);
    } else if(e->kind == V_EVENT_HISTORY_REQUEST){
        evr = (v_waitsetEventHistoryRequest)e;
        ev = u_waitsetHistoryRequestEventNew(
                    u_entity(e->userData), e->kind, e->source,
                    evr->request->filter,
                    evr->request->filterParams,
                    evr->request->resourceLimits,
                    evr->request->minSourceTimestamp,
                    evr->request->maxSourceTimestamp);
    } else if(e->kind == V_EVENT_PERSISTENT_SNAPSHOT){
        evps = (v_waitsetEventPersistentSnapshot)e;
        ev = u_waitsetPersistentSnapshotEventNew(
            u_entity(e->userData),
             e->kind,
             e->source,
             evps->request->partitionExpr,
             evps->request->topicExpr,
             evps->request->uri);
    }else if(e->kind == V_EVENT_CONNECT_WRITER) {
        ecw = (v_waitsetEventConnectWriter)e;
        ev = u_waitsetConnectWriterEventNew(
             u_entity(e->userData),
             e->kind,
             e->source,
             ecw->group);
    } else {
        ev = u_waitsetEventNew(u_entity(e->userData), e->kind);
    }
    if (ev) {
        *list = c_iterInsert(*list,ev);
    }
}

u_result
u_waitsetWaitEvents(
    u_waitset w,
    c_iter *list)
{
    u_result r;
    v_waitset kw;

    if ((w != NULL) && (list != NULL)) {
        r = u_entityReadClaim(u_entity(w), (v_entity*)(&kw));
        if (r == U_RESULT_OK) {
            assert(kw);
            *list = NULL;
            r = u_resultFromKernel(v_waitsetWait(kw,collectEvents,list));
            u_entityRelease(u_entity(w));
        }
    }else    {
        OS_REPORT_2(OS_ERROR, "u_waitsetTimedWait", 0,
                    "Illegal parameter specified (ws=0x"
                    PA_ADDRFMT
                    ",list=0x"
                    PA_ADDRFMT
                    ")",
                    (PA_ADDRCAST)w, (PA_ADDRCAST)list);
        r = U_RESULT_ILL_PARAM;
    }
    return r;
}

u_result
u_waitsetTimedWaitEvents(
    u_waitset w,
    const c_time t,
    c_iter *list)
{
    u_result r;
    v_waitset kw;

    if ((w != NULL) && (list != NULL)) {
        r = u_entityReadClaim(u_entity(w), (v_entity*)(&kw));
        if (r == U_RESULT_OK) {
            assert(kw);
            *list = NULL;
            r = u_resultFromKernel(v_waitsetTimedWait(kw,collectEvents,list,t));
            u_entityRelease(u_entity(w));
        }
    }else    {
        OS_REPORT_2(OS_ERROR, "u_waitsetTimedWait", 0,
                    "Illegal parameter specified (ws=0x"
                    PA_ADDRFMT
                    ",list=0x"
                    PA_ADDRFMT
                    ")",
                    (PA_ADDRCAST)w, (PA_ADDRCAST)list);
        r = U_RESULT_ILL_PARAM;
    }
    return r;
}

u_result
u_waitsetAttach(
    u_waitset w,
    u_entity e,
    c_voidp context)
{
    u_result r;
    v_waitset kw;
    v_entity ke;

    if ((w != NULL) && (e != NULL)) {
        r = u_entityWriteClaim(u_entity(w), (v_entity*)(&kw));
        if (r == U_RESULT_OK) {
            assert(kw);
            r = u_entityReadClaim(e, &ke);
            if (r == U_RESULT_OK) {
                v_waitsetAttach(kw,v_observable(ke),context);
                u_entityRelease(e);
            }
            u_entityRelease(u_entity(w));
        } else {
            OS_REPORT(OS_ERROR, "u_waitSetAttach", 0,
                      "Could not claim supplied entity.");
        }
    } else {
        OS_REPORT(OS_ERROR, "u_waitSetAttach", 0,
                  "Illegal parameter specified.");
        r = U_RESULT_ILL_PARAM;
    }
    return r;
}

u_result
u_waitsetDetach(
    u_waitset w,
    u_entity e)
{
    u_result r;
    v_waitset kw;
    v_entity ke;

    if ((w != NULL) && (e != NULL)) {
        r = u_entityReadClaim(u_entity(w), (v_entity*)(&kw));
        if (r == U_RESULT_OK) {
            assert(kw);
            r = u_entityReadClaim(e, &ke);
            if (r == U_RESULT_OK) {
                v_waitsetDetach(kw,v_observable(ke));
                u_entityRelease(e);
            }
            u_entityRelease(u_entity(w));
        } else {
            OS_REPORT(OS_ERROR, "u_waitSetDetach", 0,
                  "Could not claim supplied entity.");
        }
    } else {
        OS_REPORT(OS_ERROR, "u_waitSetDetach", 0,
                  "Illegal parameter specified.");
        r = U_RESULT_ILL_PARAM;
    }
    return r;
}

u_result
u_waitsetGetEventMask(
    u_waitset w,
    c_ulong *eventMask)
{
    v_waitset kw;
    u_result r;

    if ((w != NULL) && (eventMask != NULL)) {
        r = u_entityReadClaim(u_entity(w), (v_entity*)(&kw));
        if (r == U_RESULT_OK) {
            assert(kw);
            if (C_TYPECHECK(kw,v_waitset)) {
                *eventMask = v_observerGetEventMask(v_observer(kw));
            } else {
                OS_REPORT(OS_ERROR, "u_waitGetEventMask", 0,
                          "Class mismatch.");
                r = U_RESULT_CLASS_MISMATCH;
            }
            u_entityRelease(u_entity(w));
        } else {
            OS_REPORT(OS_WARNING, "u_waitGetEventMask", 0,
                      "Could not claim waitset.");
        }
    } else {
        OS_REPORT(OS_ERROR, "u_waitGetEventMask", 0,
                  "Illegal parameter specified.");
        r = U_RESULT_ILL_PARAM;
    }
    return r;
}

u_result
u_waitsetSetEventMask(
    u_waitset w,
    c_ulong eventMask)
{
    v_waitset kw;
    u_result r;

    if (w != NULL) {
        r = u_entityReadClaim(u_entity(w), (v_entity*)(&kw));
        if (r == U_RESULT_OK) {
            assert(kw);
            if (C_TYPECHECK(kw,v_waitset)) {
                v_observerSetEventMask(v_observer(kw), eventMask);
            } else {
                OS_REPORT(OS_ERROR, "u_waitSetEventMask", 0,
                          "Class mismatch.");
                r = U_RESULT_CLASS_MISMATCH;
            }
            u_entityRelease(u_entity(w));
        } else {
            OS_REPORT(OS_WARNING, "u_waitsetSetEventMask", 0,
                      "Could not claim waitset.");
        }
    } else {
        OS_REPORT(OS_ERROR, "u_waitGetEventMask", 0,
                  "Illegal parameter specified.");
        r = U_RESULT_ILL_PARAM;
    }
    return r;
}

