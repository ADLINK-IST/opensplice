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
#include "cmx__factory.h"
#include "cmx__dataReader.h"
#include "cmx_dataReader.h"
#include "cmx__entity.h"
#include "u_observable.h"
#include "u_entity.h"
#include "u_subscriber.h"
#include "u_dataReader.h"
#include "v_dataReader.h"
#include "v_readerQos.h"
#include "os_heap.h"
#include <stdio.h>
#include "os_stdlib.h"

c_char*
cmx_dataReaderNew(
    const c_char* subscriber,
    const c_char* name,
    const c_char* view,
    const c_char* qos)
{
    u_subscriber sub;
    u_dataReader rea;
    c_char* result;
    u_result ur;
    cmx_entity ce;

    ur = U_RESULT_UNDEFINED;
    result = NULL;

    ce = cmx_entityClaim(subscriber);
    if(ce != NULL){
        sub = u_subscriber(ce->uentity);
        if(view != NULL){
            rea = u_dataReaderNew(sub, name,  view, NULL, 0, NULL);
        } else {
            rea = u_dataReaderNew(sub, name,  NULL, NULL, 0, NULL);
        }
        cmx_entityRelease(ce);
        if(rea != NULL){
            ur = U_RESULT_OK;
            if ((qos != NULL) && (strlen(qos) > 0)) {
                ur = u_entitySetXMLQos(u_entity(rea), qos);
            }
            if(ur == U_RESULT_OK){
                if(u_entityEnabled(u_entity(sub))) {
                    u_subscriberQos uqos = NULL;
                    if((ur = u_subscriberGetQos(sub, &uqos)) == U_RESULT_OK) {
                        if(uqos->entityFactory.v.autoenable_created_entities) {
                            ur = u_entityEnable(u_entity(rea));
                        }
                    }
                    if(uqos) {
                        u_subscriberQosFree(uqos);
                    }
                }
            }
            if(ur == U_RESULT_OK){
                ur = cmx_entityRegister(u_object(rea), ce->participant, &result);
            }
        }
    }
    return result;
}

c_char*
cmx_dataReaderInit(
    v_dataReader entity)
{
    assert(C_TYPECHECK(entity, v_dataReader));
    OS_UNUSED_ARG(entity);

    return (c_char*)(os_strdup("<kind>DATAREADER</kind>"));
}

const c_char*
cmx_dataReaderWaitForHistoricalData(
    const c_char* dataReader,
    const os_duration timeout)
{
    u_result ur;
    const c_char* result;
    cmx_entity ce;
    u_dataReader entity;

    ce = cmx_entityClaim(dataReader);

    if(ce != NULL){
        entity = u_dataReader(ce->uentity);
        ur = u_dataReaderWaitForHistoricalData(entity, timeout);

        if(ur == U_RESULT_OK){
            result = CMX_RESULT_OK;
        } else if(ur == U_RESULT_TIMEOUT){
            result = CMX_RESULT_TIMEOUT;
        } else {
            result = CMX_RESULT_FAILED;
        }
        cmx_entityRelease(ce);
    } else {
        result = CMX_RESULT_ENTITY_NOT_AVAILABLE;
    }
    return result;
}
