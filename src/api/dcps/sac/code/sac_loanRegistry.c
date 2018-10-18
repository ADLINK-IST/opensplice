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
#include "os_heap.h"
#include "os_stdlib.h"
#include "sac_object.h"
#include "sac_report.h"
#include "sac_loanRegistry.h"
#include "sac_typeSupport.h"
#include "sac_genericCopyOut.h"
#include "dds_dcps_private.h"


C_CLASS(DDS_Loan);

C_STRUCT(DDS_Loan) {
    DDS_Loan next;
    DDS_long length;
    void *data;
    void *info;
};

C_STRUCT(DDS_LoanRegistry) {
    DDS_Loan firstLoan;
    DDS_unsigned_long allocSize; /* the buffer element type size. */
    DDS_allocBuffer allocBuffer;
    DDS_copyCache copyCache;
    DDS_Loan cachedLoan; /* the last freed loan is cached for potential reuse. */
};

DDS_ReturnCode_t
DDS_LoanRegistry_free (
    DDS_LoanRegistry _this)
{
    DDS_ReturnCode_t result;
    DDS_Loan loan;

    if (_this) {
        while (_this->firstLoan != NULL) {
            loan = _this->firstLoan;
            if (loan->data) {
                DDS_free(loan->data);
                loan->data = NULL;
            }
            if (loan->info) {
                os_free(loan->info);
                loan->info = NULL;
            }
            _this->firstLoan = loan->next;
            os_free(loan);
        }
        if (_this->cachedLoan) {
            if (_this->cachedLoan->data) {
                DDS_free(_this->cachedLoan->data);
                _this->cachedLoan->data = NULL;
            }
            if (_this->cachedLoan->info) {
                os_free(_this->cachedLoan->info);
                _this->cachedLoan->info = NULL;
            }
            os_free(_this->cachedLoan);
        }
        os_free(_this);
        result = DDS_RETCODE_OK;
    } else {
        result = DDS_RETCODE_OK;
    }
    return result;
}

DDS_LoanRegistry
DDS_LoanRegistry_new (
    DDS_TypeSupport typeSupport)
{
    DDS_LoanRegistry _this;

    _this = os_malloc(C_SIZEOF(DDS_LoanRegistry));
    _this->firstLoan = NULL;
    _this->allocSize = DDS_TypeSupport_get_alloc_size(typeSupport);
    _this->allocBuffer = DDS_TypeSupport_get_alloc_buffer(typeSupport);
    _this->copyCache = DDS_TypeSupportCopyCache (typeSupport);
    _this->cachedLoan = (DDS_Loan)os_malloc(C_SIZEOF(DDS_Loan));
    _this->cachedLoan->next = NULL;
    _this->cachedLoan->data = NULL;
    _this->cachedLoan->info = NULL;
    _this->cachedLoan->length = 0;
    return _this;
}

DDS_ReturnCode_t
DDS_LoanRegistry_register (
    DDS_LoanRegistry _this,
    _DDS_sequence data_seq,
    DDS_SampleInfoSeq *info_seq,
    DDS_long length)
{
    DDS_Loan loan;
    DDS_long bufferSize;

    assert(_this);
    assert(data_seq);
    assert(info_seq);
    assert(length > 0);
    assert(data_seq->_release == info_seq->_release);
    assert(data_seq->_maximum == info_seq->_maximum);
    assert((data_seq->_buffer != NULL) == (info_seq->_buffer != NULL));

    if (data_seq->_maximum == 0) {
        assert(data_seq->_buffer == NULL);
        if (_this->cachedLoan) {
            loan = _this->cachedLoan;
            _this->cachedLoan = NULL;
            if ((loan->length != length) && (loan->length != 0)) {
                DDS_free(loan->data);
                loan->data = NULL;
                os_free(loan->info);
                loan->info = NULL;
            }
        } else {
            loan = (DDS_Loan)os_malloc(C_SIZEOF(DDS_Loan));
            loan->next = NULL;
            loan->length = 0;
        }
        if (loan->length != length) {
            loan->length = length;
            if (_this->allocBuffer != NULL) {
                loan->data = _this->allocBuffer(length);
            } else {
                loan->data = DDS_copyOutAllocBuffer(_this->copyCache, length);
            }
            bufferSize = length * _this->allocSize;
            memset(loan->data, 0, bufferSize);

            bufferSize = length * sizeof(DDS_SampleInfo);
            loan->info = os_malloc(bufferSize);
            loan->length = length;
        }
        data_seq->_buffer = loan->data;
        data_seq->_release = FALSE;
        data_seq->_maximum = length;
        data_seq->_length  = 0;
        info_seq->_buffer  = loan->info;
        info_seq->_release = FALSE;
        info_seq->_maximum = length;
        info_seq->_length  = 0;

        loan->data = data_seq->_buffer;
        loan->info = info_seq->_buffer;
        loan->next = _this->firstLoan;
        _this->firstLoan = loan;
    } else {
        /* If _maximum != 0, then the application owns the buffers
         * and they aren't loans. Don't register, just return OK. */
        assert(data_seq->_release);
        assert(data_seq->_buffer != NULL);
        assert(data_seq->_maximum >= (DDS_unsigned_long)length);
    }
    return DDS_RETCODE_OK;
}

DDS_ReturnCode_t
DDS_LoanRegistry_deregister (
    DDS_LoanRegistry _this,
    _DDS_sequence data_seq,
    DDS_SampleInfoSeq *info_seq)
{
    DDS_ReturnCode_t result;
    DDS_Loan loan, prev;
    DDS_boolean notFound;

    assert(_this);
    assert(data_seq);
    assert(info_seq);
    assert(data_seq->_release == info_seq->_release);
    assert(data_seq->_maximum == info_seq->_maximum);
    assert((data_seq->_buffer != NULL) == (info_seq->_buffer != NULL));

    notFound = TRUE;
    loan = _this->firstLoan;
    prev = NULL;
    while ((loan != NULL) && notFound) {
        notFound = (loan->data != data_seq->_buffer);
        if (notFound) {
            prev = loan;
            loan = loan->next;
        }
    }
    if (loan) {
        if (loan->info == info_seq->_buffer) {
            if (prev) {
                prev->next = loan->next;
            } else {
                _this->firstLoan = loan->next;
            }
            loan->next = NULL;
            /* Only cache small loans, caching of huge buffers isn't efficient and is potential too intrusive. */
            if (loan->length * _this->allocSize > 10000) {
                DDS_free(loan->data);
                os_free(loan->info);
                os_free(loan);
            } else {
                if (_this->cachedLoan) {
                    if (_this->cachedLoan->data) DDS_free(_this->cachedLoan->data);
                    if (_this->cachedLoan->info) os_free(_this->cachedLoan->info);
                    os_free(_this->cachedLoan);
                }
                _this->cachedLoan = loan;
            }

            data_seq->_length  = 0;
            data_seq->_maximum = 0;
            data_seq->_buffer  = NULL;
            info_seq->_length  = 0;
            info_seq->_maximum = 0;
            info_seq->_buffer  = NULL;

            result = DDS_RETCODE_OK;
        } else {
            result = DDS_RETCODE_PRECONDITION_NOT_MET;
            SAC_REPORT(result, "Sequence parameter 'info_seq' is not registered for this Entity");
        }
    } else {
        result = DDS_RETCODE_PRECONDITION_NOT_MET;
        SAC_REPORT(result, "Sequence parameter 'data_seq' is not registered for this Entity");
    }
    return result;
}

DDS_boolean
DDS_LoanRegistry_is_loan (
    DDS_LoanRegistry _this,
    void *data_buffer,
    void *info_buffer)
{
    DDS_Loan loan;
    DDS_boolean notFound, result;

    if (_this && data_buffer && info_buffer) {
        notFound = TRUE;
        loan = _this->firstLoan;
        while ((loan != NULL) && notFound) {
            notFound = (loan->data == data_buffer);
            if (notFound) {
                loan = loan->next;
            }
        }
        result = (DDS_boolean)((loan) && (loan->info == info_buffer));
    } else {
        result = FALSE;
    }
    return result;
}

DDS_boolean
DDS_LoanRegistry_is_empty (
    DDS_LoanRegistry _this)
{
    DDS_boolean result;

    if (_this) {
        result = (DDS_boolean)(_this->firstLoan == NULL);
    } else {
        result = TRUE;
    }
    return result;
}

DDS_unsigned_long
DDS_LoanRegistry_typeSize (
    DDS_LoanRegistry _this)
{
    return _this->allocSize;
}

