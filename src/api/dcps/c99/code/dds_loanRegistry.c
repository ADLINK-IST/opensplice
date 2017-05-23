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
#include "dds_loanRegistry.h"

#include "os_heap.h"
#include "os_stdlib.h"
#include "dds_report.h"

typedef struct LoanEntry_s *LoanEntry;

struct LoanEntry_s {
    LoanEntry next;
    uint32_t length;
    void *data;
};


struct dds_loanRegistry_s {
    os_mutex lock;
    LoanEntry firstLoan;
    LoanEntry cachedLoan;
    uint32_t allocSize;
    DDS_TypeSupportCopyInfo copyInfo;

};

void
dds_loanRegistry_free (
    dds_loanRegistry_t _this)
{
    LoanEntry loan;

    if (_this) {
        while (_this->firstLoan != NULL) {
            loan = _this->firstLoan;
            if (loan->data) {
                DDS_free(loan->data);
                loan->data = NULL;
            }
            _this->firstLoan = loan->next;
            os_free(loan);
        }
        if (_this->cachedLoan) {
            if (_this->cachedLoan->data) {
                DDS_free(_this->cachedLoan->data);
                _this->cachedLoan->data = NULL;
            }
            os_free(_this->cachedLoan);
        }
        DDS_TypeSupportCopyInfo_free(_this->copyInfo);
        os_mutexDestroy(&_this->lock);
        os_free(_this);
    }
}

dds_loanRegistry_t
dds_loanRegistry_new (
    DDS_TypeSupport typeSupport)
{
    dds_loanRegistry_t _this;
    os_result osr;

    _this = os_malloc(sizeof(struct dds_loanRegistry_s));
    osr = os_mutexInit(&_this->lock, NULL);
    if (osr == os_resultSuccess) {
        _this->firstLoan = NULL;
        _this->copyInfo = DDS_TypeSupportCopyInfo_new(typeSupport);
        _this->allocSize = DDS_TypeSupportCopyInfo_alloc_size(_this->copyInfo);
        _this->cachedLoan = os_malloc(sizeof(struct LoanEntry_s));
        _this->cachedLoan->next = NULL;
        _this->cachedLoan->data = NULL;
        _this->cachedLoan->length = 0;
    } else {
        os_free(_this);
        _this = NULL;
    }

    return _this;
}

int
dds_loanRegistry_register (
    dds_loanRegistry_t _this,
    void **buffer,
    uint32_t length)
{
    LoanEntry loan;
    uint32_t i;
    char *ptr;

    assert(_this);
    assert(buffer);
    assert(length > 0);

    os_mutexLock(&_this->lock);

    if (_this->cachedLoan) {
        loan = _this->cachedLoan;
        _this->cachedLoan = NULL;
        if ((loan->length != length) && (loan->length != 0)) {
            DDS_free(loan->data);
            loan->data = NULL;
        }
    } else {
        loan = os_malloc(sizeof(struct LoanEntry_s));
        loan->next = NULL;
        loan->length = 0;
    }
    if (loan->length != length) {
        loan->length = length;
        loan->data = DDS_TypeSupportCopyInfo_alloc_buffer(_this->copyInfo, (DDS_long)length);
        memset(loan->data, 0, _this->allocSize);
    }
    ptr = loan->data;
    for (i = 0; i < loan->length; i++) {
        buffer[i] = ptr;
        ptr += _this->allocSize;
    }

    loan->next = _this->firstLoan;
    _this->firstLoan = loan;

    os_mutexUnlock(&_this->lock);

    return DDS_RETCODE_OK;
}

int
dds_loanRegistry_deregister (
    dds_loanRegistry_t _this,
    void **buffer,
    uint32_t size)
{
    DDS_ReturnCode_t result;
    LoanEntry loan, prev;
    DDS_boolean notFound;

    assert(_this);
    assert(buffer);

    OS_UNUSED_ARG(size);

    os_mutexLock(&_this->lock);

    notFound = TRUE;
    loan = _this->firstLoan;
    prev = NULL;
    while ((loan != NULL) && notFound) {
        notFound = (loan->data != buffer[0]);
        if (notFound) {
            prev = loan;
            loan = loan->next;
        }
    }
    if (loan) {
        if (prev) {
            prev->next = loan->next;
        } else {
            _this->firstLoan = loan->next;
        }
        loan->next = NULL;
        /* Only cache small loans, caching of huge buffers isn't efficient and is potential too intrusive. */
        if (loan->length * _this->allocSize > 10000) {
            DDS_free(loan->data);
            os_free(loan);
        } else {
            if (_this->cachedLoan) {
                if (_this->cachedLoan->data) DDS_free(_this->cachedLoan->data);
                os_free(_this->cachedLoan);
            }
            _this->cachedLoan = loan;
        }

        buffer[0] = NULL;

        result = DDS_RETCODE_OK;
    } else {
        result = DDS_RETCODE_PRECONDITION_NOT_MET;
        DDS_REPORT(result, "Sequence parameter 'buffer' is not registered for this Entity");
    }

    os_mutexUnlock(&_this->lock);

    return result;
}

uint32_t
dds_loanRegistry_typeSize (
     dds_loanRegistry_t _this)
{
    return _this->allocSize;
}

DDS_TypeSupportCopyInfo
dds_loanRegistry_copyInfo (
    dds_loanRegistry_t _this)
{
    return _this->copyInfo;
}
