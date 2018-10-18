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
#include "LoanRegistry.h"
#include "ReportUtils.h"


namespace DDS {
    namespace OpenSplice {
        class Loan
        {
        public:
            Loan(void *d, void *i):
                prev(NULL),
                next(NULL),
                data(d),
                info(i)
            {
            }
            DDS::Boolean empty()
            {
                return ((data == NULL) && (info == NULL));
            }
            DDS::Boolean equals(void *d, void *i)
            {
                return ((data == d) && (info == i));
            }
            void set(void *d, void *i)
            {
                data = d;
                info = i;
            }

            Loan *prev;
            Loan *next;

        private:
            void *data;
            void *info;
        };
    }; /* namespace OpenSplice */
}; /* namespace DDS */


DDS::OpenSplice::LoanRegistry::LoanRegistry() :
    firstLoan(NULL),
    lastLoan(NULL)
{
}

DDS::OpenSplice::LoanRegistry::~LoanRegistry ()
{
    Loan *loan = this->firstLoan;
    Loan *next;
    while (loan) {
        next = loan->next;
        delete loan;
        loan = next;
    }
}

DDS::ReturnCode_t
DDS::OpenSplice::LoanRegistry::register_loan(
    void *data_buffer,
    void *info_buffer)
{
    DDS::ReturnCode_t result = DDS::RETCODE_PRECONDITION_NOT_MET;

    Loan *loan;

    if (data_buffer && info_buffer) {
        /* Try to extract an empty loan from the list. */
        loan = this->list_extract_empty_loan();

        if (loan) {
            /* Insert the buffers into the empty loan. */
            loan->set(data_buffer, info_buffer);
        } else {
            /* No empty loan: create a new one. */
            loan = new Loan(data_buffer, info_buffer);
        }

        /* Add loan to the beginning of the list. */
        this->list_insert_loan(loan);

        result = DDS::RETCODE_OK;
    }

    return result;
}

DDS::ReturnCode_t
DDS::OpenSplice::LoanRegistry::deregister_loan(
    void *data_buffer,
    void *info_buffer)
{
    DDS::ReturnCode_t result = DDS::RETCODE_PRECONDITION_NOT_MET;

    Loan *loan;

    loan = this->list_find_loan(data_buffer, info_buffer);
    if (loan) {
        /* Remove the buffers from this loan. */
        loan->set(NULL, NULL);

        /* Move the now empty loan to the end of the list. */
        this->list_move_loan_to_end(loan);

        result = DDS::RETCODE_OK;
    } else {
        CPP_REPORT(result, "Loan not registered for this DataReader.");
    }

    return result;
}

DDS::Boolean
DDS::OpenSplice::LoanRegistry::contains_loan(
    void *data_buffer,
    void *info_buffer)
{
    return (this->list_find_loan(data_buffer, info_buffer) != NULL);
}

DDS::Boolean
DDS::OpenSplice::LoanRegistry::is_empty ()
{
    DDS::Boolean empty = (DDS::Boolean)true;

    if (this->firstLoan != NULL) {
        /* The empty loans are at the end of the list, meaning that
         * if the first loan is empty, the complete list is empty */
        empty = this->firstLoan->empty();
    }

    return empty;
}

DDS::OpenSplice::Loan*
DDS::OpenSplice::LoanRegistry::list_extract_empty_loan()
{
    Loan *loan = NULL;

    /* Empty loans are at the end. */
    if (this->lastLoan != NULL) {
        if (this->lastLoan->empty()) {
            /* Remember this empty loan. */
            loan = this->lastLoan;

            /* Update the list end (aka lastLoan). */
            if (this->lastLoan == this->firstLoan) {
                /* List is empty now. */
                this->firstLoan = NULL;
                this->lastLoan  = NULL;
            } else {
                this->lastLoan = loan->prev;
                this->lastLoan->next = NULL;
            }

            /* Loan is out of the list. */
            loan->prev = NULL;
            loan->next = NULL;
        }
    }

    return loan;
}

void
DDS::OpenSplice::LoanRegistry::list_insert_loan(
    Loan *loan)
{
    if (this->firstLoan == NULL) {
        /* List was empty. */
        this->firstLoan = loan;
        this->lastLoan  = loan;
    } else {
        /* Insert the loan at the front. */
        loan->next = this->firstLoan;
        this->firstLoan->prev = loan;
        this->firstLoan = loan;
    }
}

void
DDS::OpenSplice::LoanRegistry::list_move_loan_to_end(
    Loan *loan)
{
    if (loan != this->lastLoan) {
        if (loan == this->firstLoan) {
            /* Extract loan from the first position. */
            this->firstLoan = this->firstLoan->next;
            this->firstLoan->prev = NULL;
        } else {
            /* Extract loan from within the list. */
            Loan *prev = loan->prev;
            Loan *next = loan->next;
            prev->next = next;
            next->prev = prev;
        }
        /* Insert loan at the end. */
        loan->next = NULL;
        loan->prev = this->lastLoan;
        this->lastLoan->next = loan;
        this->lastLoan = loan;
    }
}

DDS::OpenSplice::Loan*
DDS::OpenSplice::LoanRegistry::list_find_loan(
    void *data_buffer,
    void *info_buffer)
{
    Loan *loan = NULL;

    if (data_buffer && info_buffer) {
        /* Search until end or loan is found. */
        loan  = this->firstLoan;
        while (loan && !(loan->equals(data_buffer, info_buffer)) ) {

            if (loan->empty()) {
                /* The list ends with all empty loans, which we reached.
                 * So, we can stop searching now. */
                loan = NULL;
            } else {
                loan = loan->next;
            }
        }
    }

    return loan;
}
