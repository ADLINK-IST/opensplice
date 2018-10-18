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


#include "v__objectLoan.h"
#include "os_report.h"

/************************************************

class v_objectBuffer {
    attribute v_objectBuffer next;
    attribute c_object obj[v_objectBufferLength];
};

class v_objectLoan extends v_objectLoanBuffer {
    attribute c_ulong index;
};

*************************************************/

#define LOANSIZE (v_objectBufferLength)

#define v_objectBuffer(o) (C_CAST((o),v_objectBuffer))

v_objectLoan
v_objectLoanNew (
    v_kernel kernel)
{
    v_objectLoan loan = NULL;

    assert(C_TYPECHECK(kernel,v_kernel));

    loan = v_objectLoan(v_new(kernel, v_kernelType(kernel,K_OBJECTLOAN)));
    if (loan) {
        v_objectBuffer(loan)->next = NULL;
        loan->index = 0;
    } else {
        OS_REPORT(OS_FATAL,
                  "v_objectLoan::v_objectLoanNew",V_RESULT_INTERNAL_ERROR,
                  "Failed to allocate v_objectLoan object.");
        assert(FALSE);
    }
    return loan;
}

void
v_objectLoanDeinit (
    v_objectLoan loan)
{
    OS_UNUSED_ARG(loan);
    assert(C_TYPECHECK(loan,v_objectLoan));
}

void
v_objectLoanRelease (
    v_objectLoan loan)
{
    unsigned int i;

    assert(C_TYPECHECK(loan,v_objectLoan));

    /* This operation frees all loaned objects and all buffers except for the first buffer.
     * The first buffer is kept because this entity will typically need it again.
     * E.g. we don't want to free and allocate a buffer for each read or take operation of
     * a reader, especially when reading one sample at the time.
     */
    while (v_objectBuffer(loan)->next != NULL) {
       v_objectBuffer tmp = v_objectBuffer(loan)->next;
       v_objectBuffer(loan)->next = tmp->next;
       tmp->next = NULL;
       c_free(tmp);
    }

    if (loan->index > LOANSIZE) {
        loan->index = LOANSIZE;
    }
    for (i=0; i<loan->index; i++) {
        c_free(v_objectBuffer(loan)->obj[i]);
        v_objectBuffer(loan)->obj[i] = NULL;
    }
    loan->index = 0;
}

c_object
v_objectLoanInsert (
    v_objectLoan loan,
    c_object object)
{
    v_objectBuffer buf;
    v_kernel kernel;

    assert(loan != NULL);
    assert(object != NULL);

    assert(C_TYPECHECK(loan,v_objectLoan));

    kernel = v_objectKernel(loan);
    if (loan->index >= LOANSIZE) {
        /* head is in next buffer */
        buf = v_objectBuffer(loan)->next;
        buf->obj[loan->index-LOANSIZE] = c_keep(object);
        loan->index++;
        if (loan->index == 2*LOANSIZE) {
            v_objectBuffer(loan)->next = v_objectBuffer(v_new(kernel, v_kernelType(kernel,K_OBJECTBUFFER)));
            v_objectBuffer(loan)->next->next = buf;
            loan->index = LOANSIZE;
        }
    } else {
        /* Head is in this buffer */
        buf = v_objectBuffer(loan);
        buf->obj[loan->index] = c_keep(object);
        loan->index++;
        if (loan->index == LOANSIZE) {
            v_objectBuffer(loan)->next = v_objectBuffer(v_new(kernel, v_kernelType(kernel,K_OBJECTBUFFER)));
            v_objectBuffer(loan)->next->next = NULL;
        }
    }
    return object;
}

v_objectLoan
v_objectLoanSubLoan(
    v_objectLoan _this)
{
    v_objectLoan loan = NULL;
    c_ulong i;

    assert(_this);
    assert(C_TYPECHECK(_this,v_objectLoan));

    /* Try to find an empty loan */
    for (i = 0; i < _this->index; i++) {
        assert(C_TYPECHECK(v_objectBuffer(_this)->obj[i], v_objectLoan));
        loan = v_objectLoan(v_objectBuffer(_this)->obj[i]);
        if (loan->index == 0) {
            break;
        }
    }

    /* Create a new loan and insert into parent */
    if (!loan || loan->index != 0) {
        loan = v_objectLoanInsert(_this, v_objectLoanNew(v_objectKernel(_this)));
        c_free(loan);
    }

    return loan;
}
