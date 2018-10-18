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
#include "v_dbmsconnect.h"
#include "v_kernel.h"
#include "v_service.h"
#include "v__observable.h"
#include "os_report.h"

v_dbmsconnect
v_dbmsconnectNew(
    v_kernel kernel,
    const c_char *name,
    const c_char *extStateName,
    v_participantQos qos,
    c_bool enable)
{
    v_dbmsconnect dbmsconnect = NULL;
    v_participantQos q;

    assert(C_TYPECHECK(kernel, v_kernel));
    assert(name != NULL);

    q = v_participantQosNew(kernel, (v_participantQos)qos);
    if (q == NULL) {
        OS_REPORT(OS_ERROR, OS_FUNCTION, V_RESULT_INTERNAL_ERROR,
            "DBMSconnect service not created: inconsistent qos");
    } else {
        dbmsconnect = v_dbmsconnect(v_objectNew(kernel, K_DBMSCONNECT));
        if (dbmsconnect) {
            v_serviceInit(v_service(dbmsconnect), name, extStateName, V_SERVICETYPE_DBMSCONNECT, q, enable);

            if (v_service(dbmsconnect)->state == NULL) {
                v_serviceFree(v_service(dbmsconnect));
                dbmsconnect = NULL;
            } else {
                OSPL_ADD_OBSERVER(kernel, dbmsconnect, V_EVENT_NEW_GROUP, NULL);
            }
        } else {
            OS_REPORT(OS_ERROR, OS_FUNCTION, V_RESULT_OUT_OF_MEMORY,
                "DBMSconnect service not created: out of memory");
        }
        c_free(q);
    }

    return dbmsconnect;
}

void
v_dbmsconnectFree(
    v_dbmsconnect _this)
{
    assert(C_TYPECHECK(_this, v_dbmsconnect));
    v_serviceFree(v_service(_this));
}
