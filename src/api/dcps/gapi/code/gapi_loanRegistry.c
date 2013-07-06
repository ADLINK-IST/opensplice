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
#include "os_heap.h"
#include "os_stdlib.h"
#include "c_base.h"
#include "gapi_loanRegistry.h"

C_CLASS(gapi_loan);

C_STRUCT(gapi_loanRegistry) {
    gapi_loan firstLoan;
};

C_STRUCT(gapi_loan) {
    gapi_loan next;
    void *data;
    void *info;
};

gapi_loanRegistry
gapi_loanRegistry_new (
    void)
{
    gapi_loanRegistry _this;

    _this = (gapi_loanRegistry)os_malloc(C_SIZEOF(gapi_loanRegistry));
    _this->firstLoan = NULL;
    return _this;
}

gapi_returnCode_t
gapi_loanRegistry_free (
    gapi_loanRegistry _this)
{
    gapi_returnCode_t result;
    gapi_loan loan;

    if (_this) {
        while (_this->firstLoan != NULL) {
            loan = _this->firstLoan;
            _this->firstLoan = loan->next;
            os_free(loan);
        }
        os_free(_this);
        result = GAPI_RETCODE_OK;
    } else {
        result = GAPI_RETCODE_OK;
    }
    return result;
}

gapi_returnCode_t
gapi_loanRegistry_register (
    gapi_loanRegistry _this,
    void *data_buffer,
    void *info_buffer)
{
    gapi_returnCode_t result;
    gapi_loan loan;

    if (_this) {
        if ( data_buffer && info_buffer ) {
            loan = (gapi_loan)os_malloc(C_SIZEOF(gapi_loan));
            loan->data = data_buffer;
            loan->info = info_buffer;
            loan->next = _this->firstLoan;
            _this->firstLoan = loan;
            result = GAPI_RETCODE_OK;
        } else {
            result = GAPI_RETCODE_PRECONDITION_NOT_MET;
        }
    } else {
        assert(_this);
        result = GAPI_RETCODE_OK;
    }
    return result;
}


gapi_returnCode_t
gapi_loanRegistry_deregister (
    gapi_loanRegistry _this,
    void *data_buffer,
    void *info_buffer)
{
    gapi_returnCode_t result;
    gapi_loan loan, prev;
    gapi_boolean notFound;

    if (_this && data_buffer && info_buffer) {
        notFound = TRUE;
        loan = _this->firstLoan;
        prev = NULL;
        while ((loan != NULL) && notFound) {
            notFound = (loan->data != data_buffer);
            if (notFound) {
                prev = loan;
                loan = loan->next;
            }
        }
        if (loan) {
            if (loan->info == info_buffer) {
                if (prev) {
                prev->next = loan->next;
                } else {
                    _this->firstLoan = loan->next;
                }
                os_free(loan);
                result = GAPI_RETCODE_OK;
            } else {
                result = GAPI_RETCODE_PRECONDITION_NOT_MET;
            }
        } else {
            result = GAPI_RETCODE_PRECONDITION_NOT_MET;
        }
    } else {
        result = GAPI_RETCODE_PRECONDITION_NOT_MET;
    }
    return result;
}

gapi_boolean
gapi_loanRegistry_is_loan (
    gapi_loanRegistry _this,
    void *data_buffer,
    void *info_buffer)
{
    gapi_loan loan;
    gapi_boolean notFound, result;

    if (_this && data_buffer && info_buffer) {
        notFound = TRUE;
        loan = _this->firstLoan;
        while ((loan != NULL) && notFound) {
            notFound = (loan->data == data_buffer);
            if (notFound) {
                loan = loan->next;
            }
        }
        result = (gapi_boolean)((loan) && (loan->info == info_buffer));
    } else {
        result = FALSE;
    }
    return result;
}

gapi_boolean
gapi_loanRegistry_is_empty (
    gapi_loanRegistry _this)
{
    gapi_boolean result;

    if (_this) {
        result = (gapi_boolean)(_this->firstLoan == NULL);
    } else {
        result = TRUE;
    }
    return result;
}
