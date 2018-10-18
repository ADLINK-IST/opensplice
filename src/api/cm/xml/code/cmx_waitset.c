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
#include "cmx__waitset.h"
#include "cmx__entity.h"
#include "cmx__factory.h"
#include "v_waitset.h"
#include "v_event.h"
#include "v_public.h"
#include "v_dataReaderQuery.h"
#include "u_observable.h"
#include "u_waitset.h"
#include "os_stdlib.h"
#include <stdio.h>

c_char*
cmx_waitsetNew(
    const c_char* participant)
{
    u_waitset waitset;
    c_char* result;

    OS_UNUSED_ARG(participant);

    result = NULL;
    waitset = u_waitsetNew();
    if (waitset) {
        /*
         * C&M thinks that the WaitSet is an Entity. It was in the past, but it isn't anymore.
         * Simulate that a WaitSet is still an entity.
         */
        result = cmx_entityXml((c_string)NULL,
                               (c_address)waitset,
                               (v_handle*)NULL,
                               (c_bool)TRUE,
                               (c_string)STRING_WAITSET_KIND);

        /* Register WaitSet for detaching purposes. */
        cmx_registerObject(u_object(waitset),NULL);
    }
    return result;
}

c_char*
cmx_waitsetInit(
    v_waitset entity)
{
    assert(C_TYPECHECK(entity, v_waitset));
    OS_UNUSED_ARG(entity);

    return (c_char*)(os_strdup("<kind>WAITSET</kind>"));
}

const c_char*
cmx_waitsetAttach(
    const c_char* waitset,
    const c_char* entity)
{
    cmx_entity wce, ce;
    u_result ur;
    const c_char* r;

    wce = cmx_entityClaim(waitset);
    if(wce != NULL){
        ce = cmx_entityClaim(entity);
        if(ce){
            ur = u_waitsetAttach(u_waitset(wce->uentity),
                                 u_observable(ce->uentity),
                                 (c_voidp)ce->uentity);
            if(ur == U_RESULT_OK){
                r = CMX_RESULT_OK;
            } else if(ur == U_RESULT_ILL_PARAM){
                r = CMX_RESULT_ILL_PARAM;
            } else {
                r = CMX_RESULT_FAILED;
            }
            cmx_entityRelease(ce);
        } else {
            r = CMX_RESULT_ILL_PARAM;
        }
        cmx_entityRelease(wce);
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
    cmx_entity wce, ce;
    u_result ur;
    const c_char* r;

    wce = cmx_entityClaim(waitset);
    if(wce != NULL){
        ce = cmx_entityClaim(entity);
        if(ce){
            ur = u_waitsetDetach_s(u_waitset(wce->uentity), u_observable(ce->uentity));
            if(ur == U_RESULT_OK){
                r = CMX_RESULT_OK;
            } else if(ur == U_RESULT_ILL_PARAM){
                r = CMX_RESULT_ILL_PARAM;
            } else {
                r = CMX_RESULT_FAILED;
            }
            cmx_entityRelease(ce);
        } else {
            r = CMX_RESULT_ILL_PARAM;
        }
        cmx_entityRelease(wce);
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
    OS_UNUSED_ARG(o);
    OS_UNUSED_ARG(args);

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
    proceed = FALSE;

    /* Forward the user layer entity to be able to create a proper user layer proxy. */

    r = v_handleClaim(event->source, (c_object)&entity);
    if (r == V_HANDLE_OK) {
        arg->entityArg.entity = cmx_factoryClaimEntity(u_object(event->userData));
        if (arg->entityArg.entity) {
            ue = u_entity(v_entityGetUserData(entity));
            if (ue) {
                if (u_objectKind(u_object(ue)) == U_QUERY) {
                    if (v_dataReaderQueryTest(v_dataReaderQuery(entity),cmx_alwaysTrue,NULL)) {
                        proceed = cmx_entityNewFromWalk(v_public(entity), &arg->entityArg);
                    }
                } else {
                    proceed = cmx_entityNewFromWalk(v_public(entity), &arg->entityArg);
                }
            }
            cmx_factoryReleaseEntity(arg->entityArg.entity);
        } else {
            assert(FALSE);
        }
        r = v_handleRelease(event->source);
        assert(r == V_HANDLE_OK);
        if(proceed == TRUE){
            xmlEntity = arg->entityArg.result;
            arg->list = c_iterInsert(arg->list, xmlEntity);
            arg->length += strlen(xmlEntity);
        }
    }
}

c_char*
cmx_waitsetWait(
    const c_char* waitset)
{
    return cmx_waitsetTimedWait(waitset, OS_DURATION_INFINITE);
}

c_char*
cmx_waitsetTimedWait(
    const c_char* waitset,
    const os_duration t)
{
    cmx_walkEntityArg arg;
    cmx_entity ce;
    c_char* result;
    u_result ur;

    result = NULL;
    ce = cmx_entityClaim(waitset);
    if(ce != NULL){
        arg = cmx_walkEntityArg(os_malloc(C_SIZEOF(cmx_walkEntityArg)));
        if (arg != NULL) {
            arg->length = 0;
            arg->list = NULL;
            arg->entityArg.entity = ce;
            arg->entityArg.create = TRUE;
            arg->entityArg.result = NULL;
            arg->userData = NULL;

            ur = u_waitsetWaitAction(u_waitset(ce->uentity), cmx_waitsetCollect, arg, t);
            if(ur == U_RESULT_OK){
                result = cmx_convertToXMLList(arg->list, arg->length);
            } else if (ur == U_RESULT_TIMEOUT) {
                /* Timeout is also a valid result.
                 * Return a list as result (most likely empty). */
                result = cmx_convertToXMLList(arg->list, arg->length);
            } else {
                c_iterFree(arg->list);
            }
            os_free(arg);
        }
        cmx_entityRelease(ce);
    }
    return result;
}

c_ulong
cmx_waitsetGetEventMask(
    const c_char* waitset)
{
    cmx_entity ce;
    c_ulong mask;

    mask = V_EVENT_UNDEFINED;
    ce = cmx_entityClaim(waitset);
    if(ce != NULL){
        (void)u_waitsetGetEventMask(u_waitset(ce->uentity), &mask);
        cmx_entityRelease(ce);
    }
    return mask;
}

const c_char*
cmx_waitsetSetEventMask(
    const c_char* waitset,
    c_ulong mask)
{
    u_waitset w;
    cmx_entity ce;
    const c_char* result;
    u_result ur;

    ce = cmx_entityClaim(waitset);

    if(ce != NULL){
        w = u_waitset(ce->uentity);
        ur = u_waitsetSetEventMask(w, mask);

        if(ur == U_RESULT_OK){
            result = CMX_RESULT_OK;
        } else if(ur == U_RESULT_ILL_PARAM){
            result = CMX_RESULT_ILL_PARAM;
        } else {
            result = CMX_RESULT_FAILED;
        }
        cmx_entityRelease(ce);
    } else {
        result = CMX_RESULT_ILL_PARAM;
    }
    return result;
}
