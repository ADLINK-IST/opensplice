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
#include "cmx__waitset.h"
#include "cmx__entity.h"
#include "cmx__factory.h"
#include "v_waitset.h"
#include "v_event.h"
#include "v_public.h"
#include "v_dataReaderQuery.h"
#include "u_waitset.h"
#include "u_waitsetEvent.h"
#include "os_stdlib.h"
#include <stdio.h>

c_char*
cmx_waitsetNew(
    const c_char* participant)
{
    u_participant par;
    u_waitset waitset;
    c_char* result;
    cmx_entityArg arg;
    u_result ur;

    par = u_participant(cmx_entityUserEntity(participant));
    result = NULL;

    if(par){
        waitset = u_waitsetNew(par);

        if(waitset){
            cmx_registerEntity(u_entity(waitset));
            arg = cmx_entityArg(os_malloc(C_SIZEOF(cmx_entityArg)));
            arg->entity = u_entity(waitset);
            arg->create = FALSE;
            arg->participant = NULL;
            arg->result = NULL;
            ur = u_entityAction(u_entity(waitset), cmx_entityNewFromAction, (c_voidp)(arg));

            if(ur == U_RESULT_OK){
                result = arg->result;
                os_free(arg);
            }
        }
    }
    return result;
}

c_char*
cmx_waitsetInit(
    v_waitset entity)
{
    assert(C_TYPECHECK(entity, v_waitset));

    return (c_char*)(os_strdup("<kind>WAITSET</kind>"));
}

const c_char*
cmx_waitsetAttach(
    const c_char* waitset,
    const c_char* entity)
{
    u_waitset w;
    u_entity e;
    u_result ur;
    const c_char* r;

    w = u_waitset(cmx_entityUserEntity(waitset));

    if(w){
        e = u_entity(cmx_entityUserEntity(entity));

        if(e){
            ur = u_waitsetAttach(w, e, (c_voidp) e);

            if(ur == U_RESULT_OK){
                r = CMX_RESULT_OK;
            } else if(ur == U_RESULT_ILL_PARAM){
                r = CMX_RESULT_ILL_PARAM;
            } else {
                r = CMX_RESULT_FAILED;
            }
        } else {
            r = CMX_RESULT_ILL_PARAM;
        }
    } else {
        r = CMX_RESULT_ILL_PARAM;
    }
    return r;
}

const c_char*
cmx_waitsetDetach(
    const c_char* waitset,
    const c_char* entity)
{
    u_waitset w;
    u_entity e;
    u_result ur;
    const c_char* r;

    w = u_waitset(cmx_entityUserEntity(waitset));

    if(w){
        e = u_entity(cmx_entityUserEntity(entity));

        if(e){
            ur = u_waitsetDetach(w, e);

            if(ur == U_RESULT_OK){
                r = CMX_RESULT_OK;
            } else if(ur == U_RESULT_ILL_PARAM){
                r = CMX_RESULT_ILL_PARAM;
            } else {
                r = CMX_RESULT_FAILED;
            }
        } else {
            r = CMX_RESULT_ILL_PARAM;
        }
    } else {
        r = CMX_RESULT_ILL_PARAM;
    }
    return r;
}

/* dummy action as no additional action is not needed for the query evaluation */

c_bool
cmx_alwaysTrue (
    c_object o,
    c_voidp args)
{
    return TRUE;
}

static void
cmx_waitsetCollect(
    v_waitsetEvent event,
    c_voidp args)
{
    cmx_walkEntityArg arg;
    c_bool proceed = FALSE;
    c_char* xmlEntity;
    v_entity entity;
    v_handleResult r;
    u_entity ue;
    arg = cmx_walkEntityArg(args);
    r = v_handleClaim(event->source, (c_object)&entity);
    if (r == V_HANDLE_OK) {
        ue = v_entityGetUserData(entity);
        if (ue) {
            if (u_entityKind(ue) == U_QUERY) {
                if (v_dataReaderQueryTest(v_dataReaderQuery(entity),cmx_alwaysTrue,NULL)) {
                    proceed = cmx_entityNewFromWalk(entity, arg->entityArg);
                }
            } else {
                proceed = cmx_entityNewFromWalk(entity, arg->entityArg);
            }
        }

        r = v_handleRelease(event->source);
        assert(r == V_HANDLE_OK);
        if(proceed == TRUE){
            xmlEntity = arg->entityArg->result;
            arg->list = c_iterInsert(arg->list, xmlEntity);
            arg->length += strlen(xmlEntity);
        }
    }
}

static void
cmx_waitsetWaitAction(
    v_entity entity,
    c_voidp args)
{
    v_waitset kw;

    kw = v_waitset(entity);
    v_waitsetWait(kw,cmx_waitsetCollect, args);
    return;
}

static void
cmx_waitsetTimedWaitAction(
    v_entity entity,
    c_voidp args)
{
    v_waitset kw;
    cmx_walkEntityArg arg;

    arg = cmx_walkEntityArg(args);
    kw = v_waitset(entity);
    v_waitsetTimedWait(kw,cmx_waitsetCollect, args, *((c_time*)(arg->userData)));
    return;
}

c_char*
cmx_waitsetWait(
    const c_char* waitset)
{
    cmx_walkEntityArg arg;
    u_waitset w;
    c_char* result;
    u_result ur;

    result = NULL;
    w = u_waitset(cmx_entityUserEntity(waitset));

    if(w != NULL){
        arg = cmx_walkEntityArg(os_malloc(C_SIZEOF(cmx_walkEntityArg)));
        arg->length = 0;
        arg->list = c_iterNew(NULL);

        arg->entityArg = cmx_entityArg(os_malloc(C_SIZEOF(cmx_entityArg)));
        arg->entityArg->participant = u_entityParticipant(u_entity(w));
        arg->entityArg->create = TRUE;
        arg->entityArg->result = NULL;

        ur = u_entityAction(u_entity(w), cmx_waitsetWaitAction, (c_voidp)arg);

        if(ur == U_RESULT_OK){
            result = cmx_convertToXMLList(arg->list, arg->length);
        } else {
            c_iterFree(arg->list);
        }
        os_free(arg->entityArg);
        os_free(arg);
    }
    return result;
}

c_char*
cmx_waitsetTimedWait(
    const c_char* waitset,
    const c_time t)
{
    cmx_walkEntityArg arg;
    u_waitset w;
    c_char* result;
    u_result ur;

    result = NULL;
    w = u_waitset(cmx_entityUserEntity(waitset));

    if(w != NULL){
        arg = cmx_walkEntityArg(os_malloc(C_SIZEOF(cmx_walkEntityArg)));
        arg->length = 0;
        arg->list = NULL;

        arg->entityArg = cmx_entityArg(os_malloc(C_SIZEOF(cmx_entityArg)));
        arg->entityArg->participant = u_entityParticipant(u_entity(w));
        arg->entityArg->create = TRUE;
        arg->entityArg->result = NULL;
        arg->userData = (c_voidp)(&t);

        ur = u_entityAction(u_entity(w), cmx_waitsetTimedWaitAction, (c_voidp)arg);

        if(ur == U_RESULT_OK){
            result = cmx_convertToXMLList(arg->list, arg->length);
        } else {
            c_iterFree(arg->list);
        }
        os_free(arg->entityArg);
        os_free(arg);
    }
    return result;
}

c_ulong
cmx_waitsetGetEventMask(
    const c_char* waitset)
{
    u_waitset w;
    c_ulong mask;
    u_result ur;

    w = u_waitset(cmx_entityUserEntity(waitset));

    if(w){
        ur = u_waitsetGetEventMask(w, &mask);
    } else {
        mask = V_EVENT_UNDEFINED;
    }
    return mask;
}

const c_char*
cmx_waitsetSetEventMask(
    const c_char* waitset,
    c_ulong mask)
{
    u_waitset w;
    const c_char* result;
    u_result ur;

    w = u_waitset(cmx_entityUserEntity(waitset));

    if(w){
        ur = u_waitsetSetEventMask(w, mask);

        if(ur == U_RESULT_OK){
            result = CMX_RESULT_OK;
        } else if(ur == U_RESULT_ILL_PARAM){
            result = CMX_RESULT_ILL_PARAM;
        } else {
            result = CMX_RESULT_FAILED;
        }
    } else {
        result = CMX_RESULT_ILL_PARAM;
    }
    return result;
}
