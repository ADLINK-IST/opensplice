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
#include "v_qos.h"
#include "v_kernelQos.h"
#include "os_report.h"
#include "v_policy.h"

v_kernelQos
v_kernelQosNew(
    v_kernel kernel,
    v_kernelQos template)
{
    v_kernelQos q;

    assert(kernel != NULL);
    assert(C_TYPECHECK(kernel,v_kernel));

    q = v_kernelQos(v_qosCreate(c_getBase (kernel),V_KERNEL_QOS));

    if (q != NULL) {
        if (template != NULL) {
            q->builtin = template->builtin;
        } else {
            q->builtin.v.enabled = TRUE;
        }
    } else {
        OS_REPORT(OS_ERROR, "v_kernelQosNew", V_RESULT_INTERNAL_ERROR,
            "KernelQos not created: out of memory");
        q = NULL;
    }

    return q;
}
