/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR PrismTech
 *   Limited, its affiliated companies and licensors. All rights reserved.
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
#include "cmx__publisher.h"
#include "cmx_publisher.h"
#include "cmx__factory.h"
#include "cmx__entity.h"
#include "u_participant.h"
#include "u_publisher.h"
#include "u_entity.h"
#include "u_observable.h"
#include "v_publisher.h"
#include "os_heap.h"
#include "os_stdlib.h"
#include <stdio.h>

c_char*
cmx_publisherNew(
    const c_char* participant,
    const c_char* name,
    const c_char* qos)
{
    u_publisher pub;
    u_result ur;
    cmx_entity ce;
    c_char* result;
    c_char* context;

    result = NULL;

    ce = cmx_entityClaim(participant);
    if(ce != NULL){
        pub = u_publisherNew(u_participant(ce->uentity), name, NULL, FALSE);
        if(pub != NULL){
            ur = U_RESULT_OK;
            if ((qos != NULL) && (strlen(qos) > 0)) {
                ur = u_entitySetXMLQos(u_entity(pub), qos);
                context = "u_entitySetXMLQos";
            }
            if (ur == U_RESULT_OK) {
                ur = u_entityEnable(u_entity(pub));
                context = "u_entityEnable";
            }
            if (ur == U_RESULT_OK) {
                ur = cmx_entityRegister(u_object(pub), ce, &result);
                context = "cmx_entityRegister";
            }
            if (ur != U_RESULT_OK) {
                OS_REPORT(OS_ERROR, CM_XML_CONTEXT, 0,
                            "cmx_publisherNew failed (reason: %s returned %u).",
                            context, ur);
                u_objectFree(u_object(pub));
            }
        }
        cmx_entityRelease(ce);
    }
    return result;
}

c_char*
cmx_publisherInit(
    v_publisher entity)
{
    assert(C_TYPECHECK(entity, v_publisher));
    OS_UNUSED_ARG(entity);

    return (c_char*)(os_strdup("<kind>PUBLISHER</kind>"));
}

const c_char*
cmx_publisherCoherentBegin(
    const c_char* publisher)
{
    cmx_entity ce;
    u_publisher upublisher;
    const c_char* result;
    u_result ur;

    result = NULL;

    ce = cmx_entityClaim(publisher);

    if(ce != NULL){
        upublisher = u_publisher(ce->uentity);
        ur = u_publisherCoherentBegin(upublisher);

        if(ur == U_RESULT_OK){
            result = CMX_RESULT_OK;
        } else {
            result = CMX_RESULT_FAILED;
        }
        cmx_entityRelease(ce);
    } else {
        result = CMX_RESULT_ENTITY_NOT_AVAILABLE;
    }
    return result;
}

const c_char*
cmx_publisherCoherentEnd(
    const c_char* publisher)
{
    cmx_entity ce;
    u_publisher upublisher;
    const c_char* result;
    u_result ur;

    result = NULL;

    ce = cmx_entityClaim(publisher);

    if(ce != NULL){
        upublisher = u_publisher(ce->uentity);
        ur = u_publisherCoherentEnd(upublisher);

        if(ur == U_RESULT_OK){
            result = CMX_RESULT_OK;
        } else {
            result = CMX_RESULT_FAILED;
        }
        cmx_entityRelease(ce);
    } else {
        result = CMX_RESULT_ENTITY_NOT_AVAILABLE;
    }
    return result;
}
