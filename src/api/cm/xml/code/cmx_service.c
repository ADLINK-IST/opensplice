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
#include "cmx__service.h"
#include "cmx__entity.h"
#include "cmx__serviceState.h"
#include "u_observable.h"
#include "u_entity.h"
#include "os_heap.h"
#include "v_service.h"
#include "v_entity.h"
#include <stdio.h>
#include "os_stdlib.h"

c_char*
cmx_serviceInit(
    v_service entity)
{
    assert(C_TYPECHECK(entity, v_service));
    OS_UNUSED_ARG(entity);

    return (c_char*)(os_strdup("<kind>SERVICE</kind>"));
}

c_char*
cmx_serviceGetState(
    const c_char* service)
{
    cmx_entity ce;
    c_char* result;

    result = NULL;

    ce = cmx_entityClaim(service);

    if(ce != NULL){
        if (u_observableAction(u_observable(ce->uentity),
                               cmx_serviceActionGetState,
                               &result) != U_RESULT_OK)
        {
            result = NULL;
        }
        cmx_entityRelease(ce);
    }
    return result;
}

void
cmx_serviceActionGetState(
    v_public service,
    c_voidp args)
{
    v_serviceState state;
    c_char** result;
    char *special;

    assert( (v_object(service)->kind == K_SERVICE) ||
            (v_object(service)->kind == K_SPLICED) ||
            (v_object(service)->kind == K_NETWORKING) ||
            (v_object(service)->kind == K_DURABILITY) ||
            (v_object(service)->kind == K_NWBRIDGE) ||
            (v_object(service)->kind == K_RNR)     ||
            (v_object(service)->kind == K_CMSOAP));

    result = (c_char**)args;
    assert(result);

    state = (v_service(service))->state;
    assert(state);

    special = cmx_serviceStateInit(state);
    assert(special);

    /*
     * C&M thinks that the ServiceState is an Entity. Simulate that.
     */
    *result = cmx_entityXml((c_string)  NULL,
                            (c_address) NULL,
                            (v_handle*) &((v_public(state))->handle),
                            (c_bool)    TRUE,
                            (c_string)  special);
    os_free(special);
}

const c_char*
cmx_serviceSetState(
    const c_char* service,
    const c_char* state)
{
    const c_char* result;
    cmx_entity ce;

    OS_UNUSED_ARG(state);

    ce = cmx_entityClaim(service);
    result = CMX_RESULT_FAILED;

    if(ce != NULL){
        /*@todo TODO: Implement state change here whenever this feature is planned.*/
        result = CMX_RESULT_OK;
        cmx_entityRelease(ce);
    }
    return result;
}
