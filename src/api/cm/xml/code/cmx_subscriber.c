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
#include "cmx__subscriber.h"
#include "cmx_subscriber.h"
#include "cmx__entity.h"
#include "cmx__factory.h"
#include "u_subscriber.h"
#include "u_participant.h"
#include "u_entity.h"
#include "u_observable.h"
#include "v_subscriber.h"
#include "v_subscriberQos.h"
#include "os_heap.h"
#include "os_stdlib.h"
#include <stdio.h>

c_char*
cmx_subscriberNew(
    const c_char* participant,
    const c_char* name,
    const c_char* qos)
{
    u_subscriber sub;
    u_result ur;
    cmx_entity ce;
    c_char* result;
    c_char* context;

    result = NULL;

    ce = cmx_entityClaim(participant);
    if(ce != NULL){
        sub = u_subscriberNew(u_participant(ce->uentity), name, NULL, FALSE);
        if(sub != NULL){
            ur = U_RESULT_OK;
            if ((qos != NULL) && (strlen(qos) > 0)) {
                ur = u_entitySetXMLQos(u_entity(sub), qos);
                context = "u_entitySetXMLQos";
            }
            if (ur == U_RESULT_OK) {
                ur = u_entityEnable(u_entity(sub));
                context = "u_entityEnable";
            }
            if (ur == U_RESULT_OK) {
                ur = cmx_entityRegister(u_object(sub), ce, &result);
                context = "cmx_entityRegister";
            }
            if (ur != U_RESULT_OK) {
                OS_REPORT(OS_ERROR, CM_XML_CONTEXT, 0,
                            "cmx_subscriberNew failed (reason: %s returned %u).",
                            context, ur);
                u_objectFree(u_object(sub));
            }
        }
        cmx_entityRelease(ce);
    }
    return result;
}


c_char*
cmx_subscriberInit(
    v_subscriber entity)
{
    assert(C_TYPECHECK(entity, v_subscriber));
    OS_UNUSED_ARG(entity);

    return (c_char*)(os_strdup("<kind>SUBSCRIBER</kind>"));
}

const c_char*
cmx_subscriberBeginAccess(
    const c_char* subscriber)
{
    cmx_entity ce;
    u_subscriber usubscriber;
    const c_char* result;
    u_result ur;

    result = NULL;

    ce = cmx_entityClaim(subscriber);

    if(ce != NULL){
        usubscriber = u_subscriber(ce->uentity);
        ur = u_subscriberBeginAccess(usubscriber);

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
cmx_subscriberEndAccess(
    const c_char* subscriber)
{
    cmx_entity ce;
    u_subscriber usubscriber;
    const c_char* result;
    u_result ur;

    result = NULL;

    ce = cmx_entityClaim(subscriber);

    if(ce != NULL){
        usubscriber = u_subscriber(ce->uentity);
        ur = u_subscriberEndAccess(usubscriber);

        if(ur == U_RESULT_OK){
            result = CMX_RESULT_OK;
        } else if(ur == U_RESULT_PRECONDITION_NOT_MET){
            result = CMX_RESULT_PRECONDITION_NOT_MET;
        } else {
            result = CMX_RESULT_FAILED;
        }
        cmx_entityRelease(ce);
    } else {
        result = CMX_RESULT_ENTITY_NOT_AVAILABLE;
    }
    return result;
}

c_char*
cmx_subscriberGetDataReaders(
    const c_char* subscriber,
    u_sampleMask mask)
{
    cmx_entity ce;
    u_subscriber usubscriber;
    u_dataReader dataReader;
    c_char *result, *xmlEntity;
    u_result ur;
    c_iter readers;
    c_iter xmlReaders;
    os_size_t length;

    result = NULL;

    ce = cmx_entityClaim(subscriber);

    if(ce != NULL){
        usubscriber = u_subscriber(ce->uentity);
        readers = c_iterNew(NULL);
        ur = u_subscriberGetDataReaderProxies(usubscriber, mask, &readers);

        if(ur == U_RESULT_OK){
            xmlReaders = c_iterNew(NULL);
            length = 0;

            dataReader = u_dataReader(c_iterTakeFirst(readers));

            while(dataReader){
                xmlEntity = NULL;
                ur = cmx_entityRegister(u_object(dataReader), ce->participant, &xmlEntity);

                if(ur == U_RESULT_OK){
                    xmlReaders = c_iterAppend(xmlReaders, xmlEntity);
                    length += strlen(xmlEntity);

                }
                dataReader = u_dataReader(c_iterTakeFirst(readers));
            }
            result = cmx_convertToXMLList(xmlReaders, length);
        }
        c_iterFree(readers);
        cmx_entityRelease(ce);
    }  else {
        printf("1\n");
    }
    return result;
}
