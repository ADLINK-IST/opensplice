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
#ifndef CPP_DDS_OPENSPLICE_LOANREGISTRY_H
#define CPP_DDS_OPENSPLICE_LOANREGISTRY_H

#include "ccpp.h"
#include "cpp_dcps_if.h"

namespace DDS {
    namespace OpenSplice {

        class Loan;

        /* NOT THREAD SAFE: The caller should enforce thread safety. */
        class OS_API LoanRegistry
        {
        public:
            LoanRegistry();

            virtual ~LoanRegistry();

            DDS::ReturnCode_t
            register_loan(
                void *data_buffer,
                void *info_buffer);

            DDS::ReturnCode_t
            deregister_loan(
                void *data_buffer,
                void *info_buffer);

            DDS::Boolean
            contains_loan(
                void *data_buffer,
                void *info_buffer);

            DDS::Boolean
            is_empty ();

        private:
            Loan*
            list_extract_empty_loan();

            void
            list_insert_loan(Loan *loan);

            void
            list_move_loan_to_end(Loan *loan);

            Loan*
            list_find_loan(
                void *data_buffer,
                void *info_buffer);

            Loan *firstLoan;
            Loan *lastLoan;
        }; /* class DataReader */
    }; /* namespace OpenSplice */
}; /* namespace DDS */

#undef OS_API
#endif /* CPP_DDS_OPENSPLICE_LOANREGISTRY_H */
