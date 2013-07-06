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
#include "v_qos.h"
#include "v_kernelQos.h"
#include "os_report.h"
#include "v_policy.h"

/**************************************************************
 * private functions
 **************************************************************/

/**************************************************************
 * constructor/destructor
 **************************************************************/
v_kernelQos
v_kernelQosNew(
    v_kernel kernel,
    v_kernelQos template)
{
    v_kernelQos q;

    assert(kernel != NULL);
    assert(C_TYPECHECK(kernel,v_kernel));

    q = v_kernelQos(v_qosCreate(kernel,V_KERNEL_QOS));

    if (q != NULL) {
        if (template != NULL) {
            q->builtin = template->builtin;
        } else {
            q->builtin.enabled = TRUE;
        }
    } else {
        OS_REPORT(OS_ERROR, "v_kernelQosNew", 0,
            "KernelQos not created: out of memory");
        q = NULL; 
    }

    return q;
}

/**************************************************************
 * Protected functions
 **************************************************************/

/**************************************************************
 * Public functions
 **************************************************************/
