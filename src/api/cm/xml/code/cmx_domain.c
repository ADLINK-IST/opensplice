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
#include "cmx__domain.h"
#include "cmx_domain.h"
#include "cmx__entity.h"
#include "u_partition.h"
#include "v_partition.h"
#include "u_participant.h"
#include "u_entity.h"
#include "u_observable.h"
#include "os_heap.h"
#include <stdio.h>
#include "os_stdlib.h"

c_char*
cmx_domainNew(
    const c_char* participant,
    const c_char* name)
{
    u_partition partition;
    c_char* result = NULL;
    u_result ur;
    cmx_entity ce;

    ce = cmx_entityClaim(participant);
    if (ce != NULL) {
        C_STRUCT(cmx_entityArg) arg;
        partition = u_partitionNew(u_participant(ce->uentity), name, NULL);
        if (partition != NULL) {
            arg.result = NULL;
            arg.create = FALSE;
            arg.entity = cmx_registerObject(u_object(partition), ce);
            if (arg.entity) {
                ur = u_observableAction(u_observable(partition),
                                        cmx_entityNewFromAction,
                                        (c_voidp)&arg);
                if(ur == U_RESULT_OK){
                    result = arg.result;
                }
            }
        }
        cmx_entityRelease(ce);
    }
    return result;
}

c_char*
cmx_domainInit(
    v_partition entity)
{
    assert(C_TYPECHECK(entity, v_partition));
    OS_UNUSED_ARG(entity);
    
    return (c_char*)(os_strdup("<kind>DOMAIN</kind>"));
}
