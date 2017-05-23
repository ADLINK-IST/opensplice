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
#include "cmx__query.h"
#include "cmx_query.h"
#include "cmx__factory.h"
#include "cmx__entity.h"
#include "u_observable.h"
#include "u_entity.h"
#include "u_reader.h"
#include "u_query.h"
#include "v_dataReaderQuery.h"
#include "os_heap.h"
#include "os_stdlib.h"
#include <stdio.h>

c_char*
cmx_queryNew(
    const c_char* reader,
    const c_char* name,
    const c_char* expression)
{
    u_query que;
    u_result ur;
    cmx_entity ce;
    c_char* result;

    result = NULL;
    ce = cmx_entityClaim(reader);
    if(ce != NULL){
        ur = U_RESULT_OK;
        que = u_queryNew(u_reader(ce->uentity), name, expression, NULL, 0, U_STATE_ANY);
        if(que != NULL){
            ur = cmx_entityRegister(u_object(que), ce->participant, &result);
            if (ur != U_RESULT_OK) {
                OS_REPORT(OS_ERROR, CM_XML_CONTEXT, 0,
                          "cmx_queryNew failed (reason: cmx_entityRegister returned %u).",
                          ur);
                u_objectFree(u_object(que));
            }
        }
        cmx_entityRelease(ce);
    }
    return result;
}

c_char*
cmx_queryInit(
    v_query entity)
{
#if 0
    v_dataReaderQuery query;
#endif
    char buf[512];

#if 0
    query = v_dataReaderQuery(entity);
    if(query->expression){
        if(query->params){
            os_sprintf(buf,
                "<kind>QUERY</kind>"
                "<expression><![CDATA[%s]]></expression>"
                "<params><![CDATA[%s]]></params>"
                "<instanceState>%u</instanceState>"
                "<sampleState>%u</sampleState>"
                "<viewState>%u</viewState>",
                query->expression, query->params,
                query->instanceMask,
                query->sampleMask,
                query->viewMask);
        } else {
            os_sprintf(buf,
                "<kind>QUERY</kind>"
                "<expression><![CDATA[%s]]></expression>"
                "<params></params>"
                "<instanceState>%u</instanceState>"
                "<sampleState>%u</sampleState>"
                "<viewState>%u</viewState>",
                query->expression,
                query->instanceMask,
                query->sampleMask,
                query->viewMask);
        }
    } else {
        if(query->params){
            os_sprintf(buf,
                "<kind>QUERY</kind>"
                "<expression></expression>"
                "<params><![CDATA[%s]]></params>"
                "<instanceState>%u</instanceState>"
                "<sampleState>%u</sampleState>"
                "<viewState>%u</viewState>",
                query->params,
                query->instanceMask,
                query->sampleMask,
                query->viewMask);
        } else {
            os_sprintf(buf,
                "<kind>QUERY</kind>"
                "<expression></expression>"
                "<params></params>"
                "<instanceState>%u</instanceState>"
                "<sampleState>%u</sampleState>"
                "<viewState>%u</viewState>",
                query->instanceMask,
                query->sampleMask,
                query->viewMask);
        }
    }
#else
    OS_UNUSED_ARG(entity);
    os_sprintf(buf,
        "<kind>QUERY</kind>"
        "<expression></expression>"
        "<params></params>"
        "<instanceState></instanceState>"
        "<sampleState></sampleState>"
        "<viewState></viewState>" );
#endif
    return (c_char*)(os_strdup(buf));
}
